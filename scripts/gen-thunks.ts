import fs from "fs/promises";
import path from "path";
import { getFiles, readJson } from "./file-util";
import {
  SuffixArrayLoaded,
  SuffixArrayRange,
  binary_search_range,
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

  // TODO: redirection
  "K32EmptyWorkingSet",
  "K32EnumDeviceDrivers",
  "K32EnumPageFilesA",
  "K32EnumPageFilesW",
  "K32EnumProcessModules",
  "K32EnumProcessModulesEx",
  "K32EnumProcesses",
  "K32GetDeviceDriverBaseNameA",
  "K32GetDeviceDriverBaseNameW",
  "K32GetDeviceDriverFileNameA",
  "K32GetDeviceDriverFileNameW",
  "K32GetMappedFileNameA",
  "K32GetMappedFileNameW",
  "K32GetModuleBaseNameA",
  "K32GetModuleBaseNameW",
  "K32GetModuleFileNameExA",
  "K32GetModuleFileNameExW",
  "K32GetModuleInformation",
  "K32GetPerformanceInfo",
  "K32GetProcessImageFileNameA",
  "K32GetProcessImageFileNameW",
  "K32GetProcessMemoryInfo",
  "K32GetWsChanges",
  "K32GetWsChangesEx",
  "K32InitializeProcessForWsWatch",
  "K32QueryWorkingSet",
  "K32QueryWorkingSetEx",

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
];

const ExportData = [
  "RtlNtdllName",
  "RtlDosPathSeperatorsString",
  "RtlAlternateDosPathSeperatorString",
  "RtlNtPathSeperatorString",
  "NlsAnsiCodePage",
  "NlsMbCodePageTag",
  "NlsMbOemCodePageTag",
];

// These are vaargs or forced cdecl apis
const VaArgApis = [
  // ntdll vaarg
  "DbgPrint",
  "DbgPrintEx",
  "DbgPrintReturnControlC",
  "RtlInitializeSidEx",
  "_snprintf",
  "swprintf",
  "_snwprintf",
  "sprintf",
  "sscanf",
  "_snprintf_s",
  "_snscanf_s",
  "_snwprintf_s",
  "_snwscanf_s",
  "_swprintf",
  "sprintf_s",
  "sscanf_s",
  "swprintf_s",
  "swscanf_s",
  // advapi32 vaarg
  "TraceMessage",
  // shell32 vaarg
  "ShellMessageBoxA",
  "ShellMessageBoxW",
  // user32 vaarg
  "wsprintfA",
  "wsprintfW",
  // netapi32 vaarg
  "RxRemoteApi",
  // setupapi vaarg
  "SetupWriteTextLog",
  "SetupWriteTextLogError",
  // shlwapi
  "wnsprintfA",
  "wnsprintfW",
];

//  32 0x20 Space
// 126 0x7E ~
// 127 0x7F DEL
for (let ch = 32; ch < 127; ch += 1) {
  if (ch == "_".codePointAt(0)) {
    continue;
  }
  let AsciiCode__ = "_".codePointAt(0) ?? 0;
  let AsciiCode_A = "A".codePointAt(0) ?? 0;
  let AsciiCode_Z = "Z".codePointAt(0) ?? 0;
  let AsciiCode_a = "a".codePointAt(0) ?? 0;
  let AsciiCode_z = "z".codePointAt(0) ?? 0;
  let AsciiCode_0 = "0".codePointAt(0) ?? 0;
  let AsciiCode_9 = "9".codePointAt(0) ?? 0;
  if (ch == AsciiCode__) continue;
  if (AsciiCode_0 <= ch && ch <= AsciiCode_9) continue;
  if (AsciiCode_a <= ch && ch <= AsciiCode_z) continue;
  if (AsciiCode_A <= ch && ch <= AsciiCode_Z) continue;
  // FunctionPrefixTable.push(ch);
}
const FunctionPrefixTable: number[] = [];
FunctionPrefixTable.push(" ".codePointAt(0) ?? 0);
FunctionPrefixTable.push("\r".codePointAt(0) ?? 0);
FunctionPrefixTable.push("\n".codePointAt(0) ?? 0);
FunctionPrefixTable.push("\t".codePointAt(0) ?? 0);
FunctionPrefixTable.push("*".codePointAt(0) ?? 0);
FunctionPrefixTable.push(")".codePointAt(0) ?? 0);
FunctionPrefixTable.push("(".codePointAt(0) ?? 0);
FunctionPrefixTable.push(";".codePointAt(0) ?? 0);
FunctionPrefixTable.push(",".codePointAt(0) ?? 0);

const FunctionPrefixRange: SuffixArrayRange[] = [];

let AllMatchedCount = 0;

let FunctionDecl = /[\r\n\t\s]+[;\(].*/;
let TailCharsMatched = [];

// DbgPrintReturnControlC not present in phnt
async function exportFiles(
  dllname: string,
  gensDir: string,
  filesExports: string[],
  sa: SuffixArrayLoaded,
  keysToIgnore: Set<string>,
  keysToPushPop: string[]
) {
  let funcMap = new Map<string, number>();
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
          if (trimFunc.length > 0) {
            let oldCount = funcMap.get(trimFunc) ?? 0;
            funcMap.set(trimFunc, oldCount + 1);
          }
        }
      }
    }
  }
  if (fileCount <= 0)
    throw new Error(`The fileCount:${fileCount} for ${dllname} should >= 1`);

  let allKeys = Array.from(funcMap.keys()).sort();
  for (const key of allKeys) {
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
        if ("AddAccessAllowedAceEx" == keyBuffer.toString("utf8")) {
          contentIndex = sa.index[i];
        }
        let offsetBegin = contentIndex + 1 + keyBuffer.length;
        if (FunctionPrefixTable.indexOf(sa.content[offsetBegin]) >= 0) {
          matchedIndex.push(i);
          break;
        }
      }
      if (matchedIndex.length > 0) break;
    }
    if (matchedIndex.length == 0) {
      funcMap.set(key, 0);
    } else {
      if (matchedIndex.length > 10) {
        console.log(`matchedIndex length ${matchedIndex.length}`);
      }
      let indexFound = matchedIndex[0];
      let offset = sa.index[indexFound];
      let matched = sa.content.subarray(offset, offset + 128) as Buffer;
      let matched_str = matched.toString("utf-8");
      offset += 1;
      // console.log(matched_str);
    }
  }
  let KeysJoin: string[] = [];
  let KeysDelta: string[] = [];
  for (let key of allKeys) {
    keysToPushPop.push(key);
    if (keysToIgnore.has(key)) continue;
    let value = funcMap.get(key) ?? 0;
    let define_thunks = "DEFINE_THUNK";
    if (ExportData.indexOf(key) >= 0) define_thunks = "DEFINE_THUNK_DATA";
    else if (VaArgApis.indexOf(key) >= 0) define_thunks = "DEFINE_THUNK_VAARG";
    if (value > 0) {
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
    path.join(gensDir, `${dllname}_basic.h`),
    KeysJoin.join("\r\n")
  );
  await fs.writeFile(
    path.join(gensDir, `${dllname}_delta.h`),
    KeysDelta.join("\r\n")
  );

  let KeysUnion: string[] = [];
  KeysUnion.push(`#include "${dllname}_basic.h"`);
  KeysUnion.push(`#include "${dllname}_delta.h"`);
  KeysUnion.push("");
  await fs.writeFile(
    path.join(gensDir, `${dllname}_full.h`),
    KeysUnion.join("\r\n")
  );
}

const baseDir = path.join(__dirname, "..");
const docsDir = path.join(baseDir, "docs");

export async function genWin32Headers() {
  let pathNtDirs = path.join(baseDir, "..", "win-polyfill-phnt");
  let filesPhnt = await getFiles(pathNtDirs);
  let filesWindows = await getFiles(
    "C:/Program Files (x86)/Windows Kits/10/Include/10.0.26100.0",
    true
  );

  let filesHeaders: string[] = Array.prototype.concat(
    filesPhnt.sort(),
    filesWindows.sort()
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

async function MergeFile(sa: SuffixArrayLoaded, arch: "x86" | "x64") {
  let keysToIgnore = new Set<string>(ApisToIgnore);
  let docsDirArch = path.join(docsDir, "gens", arch);
  let filesExports = await getFiles(docsDirArch);
  let gensDir = path.join(baseDir, "gens", arch);
  let keysToPushPop: string[] = [];
  for (let name of DllNameList) {
    await exportFiles(
      name,
      gensDir,
      filesExports,
      sa,
      keysToIgnore,
      keysToPushPop
    );
  }
  let push_path = path.join(gensDir, `win32_api_push.h`);
  if (false)
    await fs.writeFile(
      push_path,
      keysToPushPop.map((x) => `#define ${x} ${x}_none`).join("\r\n")
    );
  else await fs.rm(push_path, { force: true });
  await fs.writeFile(
    path.join(gensDir, `win32_api_pop.h`),
    keysToPushPop.map((x) => `#undef ${x}`).join("\r\n")
  );
}

// await genWin32Headers();

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

async function allMerge() {
  let sa = await load();
  let range = { low: 0, high: sa.content.length - 1 };
  for (let prefix_char of FunctionPrefixTable) {
    FunctionPrefixRange.push(binary_search_range(sa, prefix_char, 0, range));
  }
  await MergeFile(sa, "x86");
  await MergeFile(sa, "x64");
  console.log(AllMatchedCount);
}

allMerge();
