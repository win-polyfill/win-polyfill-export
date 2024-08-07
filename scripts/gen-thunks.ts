import fs from "fs/promises";
import path from "path";
import { getFiles, readJson } from "./file-util";
import {
  SuffixArrayLoaded,
  SuffixArrayRange,
  binary_search_range,
  binary_search,
} from "./suffix-array";

interface ExportItem {
  Ordinal: number;
  Name: string;
  ExportByOrdinal: boolean;
  VirtualAddress: number;
  ForwardedName: string;
}

const ApisToIgnore = [
  // ntdll
  // inlined functions
  "NtGetTickCount",
  "NtCurrentTeb",
  // fake exported functions
  "NtPullTransaction",
  "RtlEnclaveCallDispatch",

  // These are not real function, should be filtered out by script
  // TODO:
  "OutOfMemory",
  "KiRaiseUserExceptionDispatcher",
  "KiUserCallbackDispatcher",
  "KiUserExceptionDispatcher",
  "LdrSetDllManifestProber",
  "RtlEqualLuid",

  "RtlpTimeFieldsToTime",
  "RtlpTimeToTimeFields",
  "EtwEnumerateTraceGuids",

  // kernel32
  "ResetState",
  "ResolveDelayLoadedAPI",
  "ResolveDelayLoadsFromDll",
  "DelayLoadFailureHook",

  // user32
  "DrawFrame",

  // DLL apis present in all DLLs
  "DllGetClassObject",
  "DllCanUnloadNow",
  "DllInstall",
  "DllRegisterServer",
  "DllUnregisterServer",
  "DllGetVersion",
  "GetProxyDllInfo",
  "LdrSystemDllInitBlock",

  // fake exported
  "InjectKeyboardInput",
  "InjectMouseInput",

  //banned functions
  "StrNCpyA",
  "StrNCpyW",
  "StrCpyNA",
  "CDefFolderMenu_Create",
  "GetNumaAvailableMemory",

  // dbghelp not function decls
  "dbghelp",
  "dh",
  "sym",
  "symsrv",
  "block",
  "lm",

  // esent not function decls
  "ese",
  "esent",
  "JetAttachDatabase3",
  "JetCreateDatabase3",
  "JetBeginSurrogateBackup",
  "JetEndSurrogateBackup",
  "JetPrereadIndexRange",

  // gdi32 member function
  "GetTransform",
  "GetCertificate",
  "GetCertificateSize",
  "fpClosePrinter",

  // gdi32 non-function

  // pdh non-function
  "PdhLogServiceControlA",
  "PdhLogServiceControlW",

  // iphlpapi banned >=vista
  "AllocateAndGetTcpExTableFromStack",
  "AllocateAndGetUdpExTableFromStack",

  // setupapi non-function
  "ExtensionPropSheetPageProc",

  // ws2_32 suffix
  "WEP",
  // shell32
  "DuplicateString",
];

//  32 0x20 Space
// 126 0x7E ~
// 127 0x7F DEL

const FunctionPrefixTable: number[] = [];
FunctionPrefixTable.push(" ".codePointAt(0) ?? 0);
FunctionPrefixTable.push("\r".codePointAt(0) ?? 0);
FunctionPrefixTable.push("\n".codePointAt(0) ?? 0);
FunctionPrefixTable.push("\t".codePointAt(0) ?? 0);
FunctionPrefixTable.push("\v".codePointAt(0) ?? 0);
FunctionPrefixTable.push("*".codePointAt(0) ?? 0);
FunctionPrefixTable.push(")".codePointAt(0) ?? 0);
FunctionPrefixTable.push("(".codePointAt(0) ?? 0);
FunctionPrefixTable.push(";".codePointAt(0) ?? 0);
FunctionPrefixTable.push("}".codePointAt(0) ?? 0);

interface FunctionArg {
  type: string;
  name: string;
}

interface FunctionInfo {
  conv: string;
  key: string;
  return_type: string;
  has_dllexport: boolean;
  has_dllimport: boolean;
  is_vaarg: boolean;
  is_data: boolean;
  args: FunctionArg[];
  argsMerged: string[];
  body: string;
  decls: string;
}

const CONV_LIST = ["__stdcall", "__cdecl", "__fastcall"];
const PrimitiveTypesC = [
  "void",
  "short",
  "int",
  "char",
  "const",
  "unsigned",
  "long",
  "wchar_t",
];

const PrimitiveTypePrefixes = ["struct", "enum", "union"];

function SplitType(ba: string): string[] {
  let type = "";
  let typeList = [];
  for (let i = 0; i < ba.length; i += 1) {
    let ch = ba[i];
    if (" \v\t\r\n*[]".indexOf(ch) >= 0) {
      if (type.length > 0) {
        typeList.push(type);
        type = "";
      }
      if ("*[]".indexOf(ch) >= 0) {
        typeList.push(ch);
      }
      continue;
    }
    type += ch;
  }
  if (type.length > 0) {
    typeList.push(type);
  }
  return typeList;
}

function find_function_ret_type(ba: string, info: Partial<FunctionInfo>) {
  info.conv = "__cdecl";
  info.return_type = undefined;
  info.has_dllexport = false;
  info.has_dllimport = false;

  let lines = SplitType(ba);
  for (let k = lines.length - 1; k > 0; k -= 1) {
    let line = lines[k];
    if (line === "") continue;
    if (line === "__declspec(dllexport)") {
      info.has_dllexport = true;
      continue;
    }
    if (line === "__declspec(dllimport)") {
      info.has_dllimport = true;
      continue;
    }
    if (line === "__declspec(allocator)") {
      continue;
    }
    if (info.return_type === undefined) {
      if (CONV_LIST.indexOf(line) >= 0) {
        info.conv = line;
        continue;
      }
      if (line === "(" || line === "return") return undefined;
      info.return_type = line;
    } else {
      if (info.return_type === "*") {
        info.return_type = line + info.return_type;
        continue;
      }

      if (
        PrimitiveTypesC.indexOf(line) >= 0 ||
        PrimitiveTypePrefixes.indexOf(line) >= 0
      ) {
        info.return_type = line + " " + info.return_type;
        continue;
      }
      break;
    }
  }
}

function find_function_tail(key: string, ba: string) {
  let OpenParenthesesCount = 0;
  let CloseParenthesesCount = 0;
  ba = ba.replaceAll("const IID &", "REFIID");
  ba = ba.replaceAll("const IID * const", "REFIID");
  ba = ba.replaceAll("const GUID * const", "REFGUID");

  ba = ba.replaceAll("const PROPVARIANT * const", "REFPROPVARIANT");
  ba = ba.replaceAll("const VARIANT * const", "REFVARIANT");
  ba = ba.replaceAll("const PROPERTYKEY * const", "REFPROPERTYKEY");
  ba = ba.replaceAll("const KNOWNFOLDERID * const", "REFKNOWNFOLDERID");
  let OpenParenthesesIndex = -1;

  for (let i = 0; ; i += 1) {
    if (" \t\n\r\v".indexOf(ba[i]) < 0) {
      if (ba[i] === "(") {
        OpenParenthesesCount = 1;
        OpenParenthesesIndex = i;
      }
      if (ba[i] === ";") {
        return ";";
      }
      break;
    }
  }
  let TailFound: string | undefined = undefined;
  if (OpenParenthesesCount === 1) {
    // only start with "(" need to be handled
    for (let i = OpenParenthesesIndex + 1; i < ba.length; i += 1) {
      if (ba[i] === "(") OpenParenthesesCount += 1;
      if (ba[i] === ")") CloseParenthesesCount += 1;
      if (OpenParenthesesCount > 1 || CloseParenthesesCount > 1) {
        // console.log(`${key} with ParenthesesCount:${OpenParenthesesCount}`);
        return undefined;
      }

      if (OpenParenthesesCount === CloseParenthesesCount) {
        TailFound = ba.substring(OpenParenthesesIndex + 1, i);
        break;
      }

      if (ba[i] === ";" || ba[i] === "{" || ba[i] === "}") {
        // early exit
        break;
      }
    }
  }
  return TailFound;
}

function ParseFunctionArgs(tailFound: string, info: FunctionInfo) {
  if (tailFound === ";") {
    // it's data
    info.is_data = true;
  } else {
    info.is_data = false;
    for (let arg of tailFound.split(",")) {
      let argTrimmed = arg.trim();
      if (argTrimmed.length === 0) continue;
      if (argTrimmed === "void") continue;
      let typeList = SplitType(argTrimmed);
      let name: string = typeList.pop()!;
      if (name === "]") {
        typeList.pop();
        name = typeList.pop()!;
        typeList.push("[]");
      } else if (name === "*") {
        typeList.push(name);
        name = "";
      }
      if (PrimitiveTypePrefixes.indexOf(typeList[typeList.length - 1]) >= 0) {
        typeList.push(name);
        name = "";
      } else if (PrimitiveTypesC.indexOf(name) >= 0) {
        typeList.push(name);
        name = "";
      } else if (typeList.length === 0) {
        typeList.push(name);
        name = "";
      }

      let typeFinal: string = typeList.join(" ");
      let funcArg: FunctionArg = {
        type: typeFinal,
        name: name,
      };
      info.args?.push(funcArg);
      info.argsMerged?.push(`${funcArg.type}`);
      // info.argsMerged?.push(argTrimmed);
    }
  }
  let args = info.args!;
  if (args.length > 0 && args[args.length - 1].type === "...") {
    info.is_vaarg = true;
  }
  info.body = `(${info.argsMerged?.join(", ")})`;
}

function ParseFunctionInfo(
  dllname: string,
  arch: "x86" | "x64",
  sa: SuffixArrayLoaded,
  sa_full: SuffixArrayLoaded,
  key: string,
  matchedIndex: number[]
): FunctionInfo | undefined {
  matchedIndex.sort((ia, ib) => {
    let offsetA = sa.index[ia];
    let offsetB = sa.index[ib];
    return offsetA - offsetB;
  });
  let infos = [] as FunctionInfo[];
  for (let i = 0; i < matchedIndex.length; i++) {
    let indexFound = matchedIndex[i];
    let offset = sa.index[indexFound];
    let tailOffset = offset + key.length + 1;
    if (key === "CreateThread") {
      i += 0;
    }
    let suffix = sa.content.subarray(tailOffset, tailOffset + 4096).toString();
    let prefix = sa.content.subarray(offset - 4096, offset).toString();
    let tailFoundCurrent = find_function_tail(key, suffix);
    if (tailFoundCurrent !== undefined) {
      let info: Partial<FunctionInfo> = {
        key: key,
        has_dllexport: false,
        has_dllimport: false,
        is_vaarg: false,
        is_data: false,
        args: [],
        argsMerged: [],
        body: "",
      };
      ParseFunctionArgs(tailFoundCurrent, info as FunctionInfo);
      find_function_ret_type(prefix, info);
      infos.push(info as FunctionInfo);
    }
  }
  function get_value(x: FunctionInfo) {
    let v = 0;
    if (x.has_dllexport || x.has_dllimport) {
      v |= 1 << 0;
    }
    if (!x.is_data) {
      v |= 1 << 1;
    }
    if (x.return_type !== undefined) {
      v |= 1 << 2;
    }
    return v;
  }
  infos.sort((a, b) => {
    return get_value(b) - get_value(a);
  });
  let info = infos[0];
  if (info === undefined) {
    let foundCount = 0;
    for (let prefix_char of FunctionPrefixTable) {
      for (let suffix_char of FunctionPrefixTable) {
        let keyFull = `${String.fromCodePoint(
          prefix_char
        )}${key}${String.fromCodePoint(suffix_char)}`;
        let keyToSearch = Buffer.from(keyFull);
        let range = binary_search(sa_full, keyToSearch);
        if (range.high >= range.low) {
          foundCount += range.high - range.low + 1;
        }
      }
    }
    if (foundCount > 0) {
      console.log(`Can not find function parameters for ${arch}:${key}`);
    }
    return undefined;
  }
  if (info.return_type === undefined) {
    console.log(`Can not find return type for ${arch}:${key}`);
    process.exit();
  }
  let declsepc = " ";
  if (info.has_dllexport) {
    declsepc = " __declspec(dllexport) ";
  } else if (info.has_dllimport) {
    declsepc = " __declspec(dllimport) ";
  } else {
    if (dllname === "ntdll") {
      if (info.conv !== "__cdecl")
        console.log(`no __declspec ntdll for ${arch}:${key}`);
    } else {
      if (arch === "x86") {
        // TODO: make sure all function have __declspec(dllimport) when needed
        // console.log(`no __declspec ntdll for ${arch}:${key}`);
      }
    }
  }
  if (info.is_data) {
    info.decls = `EXTERN_C${declsepc}${info.return_type} ${key};`;
  } else {
    info.decls = `EXTERN_C${declsepc}${info.return_type} ${info.conv} ${key}${info.body};`;
  }
  return info as FunctionInfo;
}

interface MergeResult {
  decls: string;
}

// DbgPrintReturnControlC not present in phnt
async function exportFiles(
  mergeResult: MergeResult,
  FunctionPrefixRange: SuffixArrayRange[],
  arch: "x86" | "x64",
  dllname: string,
  gensDirArch: string,
  filesExports: string[],
  sa: SuffixArrayLoaded,
  sa_full: SuffixArrayLoaded,
  keysToIgnore: Set<string>,
  keysToPushPop: string[]
) {
  let funcCountMap = new Map<string, number>();
  let funcMap = new Map<string, FunctionInfo>();
  let fileCount = 0;
  for (let f of filesExports) {
    let basename = path.basename(f);
    if (basename.startsWith(dllname)) {
      // console.log(basename)
      fileCount += 1;
      let ExportList: ExportItem[] = [];
      try {
        ExportList = (await readJson(f)).Exports as ExportItem[];
      } catch (ex) {}
      for (let e of ExportList) {
        if (!e.ExportByOrdinal) {
          let trimFunc = e.Name.trim();
          if (trimFunc.indexOf("@") >= 0) continue;
          if (trimFunc.length > 0) {
            let oldCount = funcCountMap.get(trimFunc) ?? 0;
            funcCountMap.set(trimFunc, oldCount + 1);
          }
        }
      }
    }
  }
  if (fileCount <= 0)
    throw new Error(`The fileCount:${fileCount} for ${dllname} should >= 1`);

  let allKeys = Array.from(funcCountMap.keys()).sort();
  for (const key of allKeys) {
    if (keysToIgnore.has(key)) {
      funcCountMap.set(key, 0);
      continue;
    }
    let keyBuffer = Buffer.from(key, "utf-8");
    let matchedIndex = [] as number[];
    for (const prefixRange of FunctionPrefixRange) {
      let currentRange = prefixRange;
      for (let i = 0; i < keyBuffer.length; i += 1) {
        currentRange = binary_search_range(
          sa,
          keyBuffer[i],
          i + 1,
          currentRange
        );
      }
      for (let i = currentRange.low; i <= currentRange.high; i += 1) {
        let contentIndex = sa.index[i];
        let offsetBegin = contentIndex + 1 + keyBuffer.length;
        let functionMatched =
          FunctionPrefixTable.indexOf(sa.content[offsetBegin]) >= 0;
        if (functionMatched) {
          matchedIndex.push(i);
        }
      }
    }

    const info = ParseFunctionInfo(
      dllname,
      arch,
      sa,
      sa_full,
      key,
      matchedIndex
    );
    if (info === undefined) {
      if (matchedIndex.length > 0) {
        console.log(`Can not find body for ${arch}:${key}`);
      }
      funcCountMap.set(key, 0);
    } else {
      funcMap.set(key, info);
      mergeResult.decls = `${mergeResult.decls}${info.decls}\n`;
    }
  }
  let KeysJoin: string[] = [];
  let KeysDelta: string[] = [];
  for (let key of allKeys) {
    keysToPushPop.push(key);
    let value = funcCountMap.get(key) ?? 0;
    if (value > 0) {
      let info = funcMap.get(key)!;
      let define_thunks = "DEFINE_THUNK";
      if (info.is_data) define_thunks = "DEFINE_THUNK_DATA";
      else if (info.is_vaarg) define_thunks = "DEFINE_THUNK_VAARG";
      if (value == fileCount) {
        KeysJoin.push(`${define_thunks}(${dllname}, ${key})`);
      } else {
        KeysDelta.push(`${define_thunks}(${dllname}, ${key})`);
      }
    }
    keysToIgnore.add(key);
  }
  KeysJoin.push("");
  KeysDelta.push("");
  await fs.writeFile(
    path.join(gensDirArch, `${dllname}_basic.h`),
    KeysJoin.join("\r\n")
  );
  await fs.writeFile(
    path.join(gensDirArch, `${dllname}_delta.h`),
    KeysDelta.join("\r\n")
  );

  let KeysUnion: string[] = [];
  KeysUnion.push(`#include "${dllname}_basic.h"`);
  KeysUnion.push(`#include "${dllname}_delta.h"`);
  KeysUnion.push("");
  await fs.writeFile(
    path.join(gensDirArch, `${dllname}_full.h`),
    KeysUnion.join("\r\n")
  );
}

const baseDir = path.join(__dirname, "..");
const docsDir = path.join(baseDir, "docs");
const gensDir = path.join(baseDir, "gens");

export async function genWin32Headers() {
  let pathNtDirs = path.join(baseDir, "..", "win-polyfill-phnt");
  let filesPhnt = await getFiles(pathNtDirs);
  let filesWindows = await getFiles(
    "C:/Program Files (x86)/Windows Kits/10/Include/10.0.26100.0",
    true
  );
  filesWindows = await getFiles(
    "C:/work/study/runtimes/msvcrt/Windows/Include",
    true
  );
  let filesMSVC = await getFiles(
    "C:/Program Files/Microsoft Visual Studio/2022/Community/VC/Tools/MSVC/14.40.33807/include",
    true
  );
  filesMSVC = await getFiles("C:/work/study/runtimes/msvcrt/VC/include", true);

  let filesHeaders: string[] = Array.prototype.concat(
    filesPhnt.sort(),
    filesWindows.sort(),
    filesMSVC.sort()
  );
  let totalSize = 0;
  let buffers: Buffer[] = [];
  for (let f of filesHeaders) {
    if (f.endsWith(".h")) {
      let content = await fs.readFile(f);
      totalSize += content.length;
      buffers.push(content);
    }
  }
  let buf = Buffer.concat(buffers);
  console.log(buffers.length, totalSize);
  await fs.writeFile(path.join(docsDir, "win32-api-merged.h.txt"), buf);
  return buf;
}

const DllNameList = [
  "ntdll",
  "kernel32",

  "user32",
  "shell32",
  "advapi32",
  "cfgmgr32",
  "crypt32",
  "dbghelp",
  "esent",
  "gdi32",
  "iphlpapi",
  "netapi32",
  "ole32",

  "pdh",
  "powrprof",
  "psapi",
  "setupapi",
  "shlwapi",
  "userenv",
  "version",
  "winhttp",
  "ws2_32",

  // Not present on Windows 2000
  "bcrypt",
  "bcryptprimitives",
  "bluetoothapis",
  "d3d11",
  "d3d12",
  "d3d9",
  "dwmapi",
  "dwrite",
  "dxgi",
  "dxva2",
  "mf",
  "mfplat",
  "mfreadwrite",
  "ncrypt",
  "ndfapi",
  "propsys",
  "shcore",
  "uiautomationcore",
  "uxtheme",
  "wevtapi",
  "winusb",
  "zipfldr",
];

async function MergeFile(
  sa_full: SuffixArrayLoaded,
  sa: SuffixArrayLoaded,
  FunctionPrefixRange: SuffixArrayRange[],
  mergeResult: MergeResult,
  arch: "x86" | "x64"
) {
  let keysToIgnore = new Set<string>(ApisToIgnore);

  let docsDirArch = path.join(docsDir, "gens", arch);
  let filesExports = await getFiles(docsDirArch);
  let gensDirArch = path.join(gensDir, arch);
  let keysToPushPop: string[] = [];
  for (let name of DllNameList) {
    await exportFiles(
      mergeResult,
      FunctionPrefixRange,
      arch,
      name,
      gensDirArch,
      filesExports,
      sa,
      sa_full,
      keysToIgnore,
      keysToPushPop
    );
  }
  let push_path = path.join(gensDirArch, `win32_api_push.h`);
  if (false)
    await fs.writeFile(
      push_path,
      keysToPushPop.map((x) => `#define ${x} ${x}_none`).join("\r\n")
    );
  else await fs.rm(push_path, { force: true });
  await fs.writeFile(
    path.join(gensDirArch, `win32_api_pop.h`),
    keysToPushPop.map((x) => `#undef ${x}`).join("\r\n")
  );
}

async function load(): Promise<SuffixArrayLoaded> {
  let cacheDir = path.join(docsDir, "cache");
  let win32Headers = await fs.readFile(
    path.join(cacheDir, "win32-api-merged.h.txt")
  );
  let su_index_raw = await fs.readFile(
    path.join(cacheDir, "win32-api-merged.h.index.txt")
  );
  let su_index = new Uint32Array(su_index_raw.buffer);

  let loaded: SuffixArrayLoaded = {
    index: su_index,
    content: win32Headers,
  };
  return loaded;
}

async function MergePreprocessorFile(
  sa_full: SuffixArrayLoaded,
  arch: "x86" | "x64"
) {
  let gensDirArch = path.join(gensDir, arch);
  let su_content = await fs.readFile(
    path.join(gensDirArch, "gen-exports-decls.h")
  );
  let su_index_raw = await fs.readFile(
    path.join(gensDirArch, "gen-exports-decls.h.index.txt")
  );
  let su_index = new Uint32Array(su_index_raw.buffer);
  let sa: SuffixArrayLoaded = {
    index: su_index,
    content: su_content,
  };
  const FunctionPrefixRange: SuffixArrayRange[] = [];
  for (let prefix_char of FunctionPrefixTable) {
    let range = { low: 0, high: sa.content.length - 1 };
    FunctionPrefixRange.push(binary_search_range(sa, prefix_char, 0, range));
  }
  let mergeResult: MergeResult = {
    decls: "",
  };
  await MergeFile(sa_full, sa, FunctionPrefixRange, mergeResult, arch);
  await fs.writeFile(
    path.join(gensDirArch, "gen-exports-decls.inc.h"),
    mergeResult.decls
  );
}

async function allMergePreproces() {
  // await genWin32Headers();
  let sa_full = await load();
  await MergePreprocessorFile(sa_full, "x86");
  await MergePreprocessorFile(sa_full, "x64");
}

allMergePreproces();
