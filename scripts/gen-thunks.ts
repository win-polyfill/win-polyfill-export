/// <reference types="./c-preprocessor.d.ts" />

import fs from "fs/promises";
import path from "path";
import { getFiles, readJson } from "./file-util";
import {
  SuffixArrayLoaded,
  SuffixArrayRange,
  binary_search_range,
} from "./suffix-array";
import { handle_preprocess } from "./compiler";

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
  // shell32
  "DuplicateString",
];

const ExportData = [] as string[];

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

let AsciiCode__ = "_".codePointAt(0) ?? 0;
let AsciiCode_A = "A".codePointAt(0) ?? 0;
let AsciiCode_Z = "Z".codePointAt(0) ?? 0;
let AsciiCode_a = "a".codePointAt(0) ?? 0;
let AsciiCode_z = "z".codePointAt(0) ?? 0;
let AsciiCode_0 = "0".codePointAt(0) ?? 0;
let AsciiCode_9 = "9".codePointAt(0) ?? 0;

//  32 0x20 Space
// 126 0x7E ~
// 127 0x7F DEL
for (let ch = 32; ch < 127; ch += 1) {
  if (ch == "_".codePointAt(0)) {
    continue;
  }
  if (ch == AsciiCode__) continue;
  if (AsciiCode_0 <= ch && ch <= AsciiCode_9) continue;
  if (AsciiCode_a <= ch && ch <= AsciiCode_z) continue;
  if (AsciiCode_A <= ch && ch <= AsciiCode_Z) continue;
  // FunctionPrefixTable.push(ch);
}

let FallbackNames = {
  DsGetDcClose: "DsGetDcCloseW",
  InterlockedPushListSList: "InterlockedPushListSListEx",
  GetEnvironmentStringsA: "GetEnvironmentStrings",
  CMP_WaitNoPendingInstallEvents: "CM_WaitNoPendingInstallEvents",
  SystemFunction036: "RtlGenRandom",
  SystemFunction040: "RtlEncryptMemory",
  SystemFunction041: "RtlDecryptMemory",
  GdiEntry1: "DdCreateDirectDrawObject",
  GdiEntry2: "DdQueryDirectDrawObject",
  GdiEntry3: "DdDeleteDirectDrawObject",
  GdiEntry4: "DdCreateSurfaceObject",
  GdiEntry5: "DdDeleteSurfaceObject",
  GdiEntry6: "DdResetVisrgn",
  GdiEntry7: "DdGetDC",
  GdiEntry8: "DdReleaseDC",
  GdiEntry9: "DdCreateDIBSection",
  GdiEntry10: "DdReenableDirectDrawObject",
  GdiEntry11: "DdAttachSurface",
  GdiEntry12: "DdUnattachSurface",
  GdiEntry13: "DdQueryDisplaySettingsUniqueness",
  GdiEntry14: "DdGetDxHandle",
  GdiEntry15: "DdSetGammaRamp",
  GdiEntry16: "DdSwapTextureHandles",
  GdiEntry17: "DdChangeSurfacePointer",
} as { [id: string]: string };

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
function removeComments(string: string) {
  //Takes a string of code, not an actual function.
  return string.replace(/\/\*[\s\S]*?\*\/|(?<=[^:])\/\/.*|^\/\/.*/g, "").trim(); //Strip comments
}

function strip_comment(ba: string) {
  ba = ba.replaceAll("/* [in] */", "_In_");
  ba = ba.replaceAll("/* in */", "_In_");
  ba = ba.replaceAll("/* [out] */", "_Out_");
  ba = ba.replaceAll("/* out */", "_Out_");
  ba = ba.replaceAll("/* [optional][in] */", "_In_opt_");
  ba = ba.replaceAll("/* [annotation][out] */", " ");
  ba = ba.replaceAll("/* [out][annotation] */", " ");
  ba = ba.replaceAll("/* [annotation][in] */", " ");
  ba = ba.replaceAll("  ", " ");
  ba = removeComments(ba);
  let stringList = ba.split("\n").filter((x) => x.trim().length > 0);
  stringList = stringList.map((x) => x.trimEnd());
  ba = stringList.join("\n");
  return ba;
}

// TODO: handle `unsigned char` `unsigned char*` `unsigned long`
let API_LIST_STDCALL = [
  "WINAPI",
  "NTAPI",
  "STDAPICALLTYPE",
  "APIENTRY",
  "CALLBACK",
  "WMIAPI",
  "EVNTAPI",
  "NET_API_FUNCTION",
  "__stdcall",
  "IMAGEAPI",
  "JET_API",
  "NETIOAPI_API_",
  "__RPC_USER",
  "STDMETHODCALLTYPE",
  "WSAAPI",
  "WSPAPI",
  "PASCAL",
  "DWRITE_EXPORT",
];

let API_LIST_CDECL = ["STDAPIVCALLTYPE", "__cdecl", "__CRTDECL", "WINAPIV"];

let TYPE_LIST = [
  "DWORD",
  "BOOL",
  "unsigned",
  "LONG64",
  "NTSTATUS",
  "ULONG",
  "CONFIGRET",
  "int",
  "HRESULT",
  "FEATURE_ENABLED_STATE",
  "UINT32",
  "void",
];

let HRESULT_TYPE_LIST = [
  "STDAPI",
  "EVRPUBLIC",
  "WINOLEAPI",
  "WINOLEAUTAPI",
  "SHSTDAPI",
  "SHFOLDERAPI",
  "DWMAPI",
  "PSSTDAPI",
  "THEMEAPI",
  "LWSTDAPI",
];

let API_LIST_SUB = [
  // cdecl
  "LWSTDAPIV_(",
  // stdcall
  "SHSTDAPI_(",
  "LWSTDAPI_(",
  "WINOLEAPI_(",
  "DWMAPI_(",
  "PSSTDAPI_(",
  "THEMEAPI_(",
];

function join_pointers(lines: string[]) {
  for (let i = lines.length - 1; i >= 0; i -= 1) {
    let line_i = lines[i];
    if (line_i.startsWith("*") || line_i.startsWith("const *")) {
      lines[i - 1] = lines[i - 1] + " " + lines[i];
      lines[i] = "";
    }
  }
  return lines.filter(function (e) {
    return e.trim().length > 0;
  });
}

function split_return_type_lines(ba: string) {
  ba = ba.replaceAll("unsigned long long *", "PUINT64 ");
  ba = ba.replaceAll("unsigned long long ", "UINT64 ");
  ba = ba.replaceAll("long long *", "PINT64 ");
  ba = ba.replaceAll("long long ", "INT64 ");

  ba = ba.replaceAll("unsigned int *", "PUINT32 ");
  ba = ba.replaceAll("unsigned int ", "UINT32 ");

  ba = ba.replaceAll("unsigned short *", "PUINT16 ");
  ba = ba.replaceAll("unsigned short ", "UINT16 ");

  ba = ba.replaceAll("unsigned char * ", "PUINT8 ");
  ba = ba.replaceAll("unsigned char ", "UINT8 ");

  ba = ba.replaceAll("unsigned long ", "ULONG ");
  ba = ba.replaceAll("_CONST_RETURN", "const");

  let lines = [] as string[];
  let chars = "";
  for (let i = ba.length - 1; i >= 0; i -= 1) {
    if (ba[i] === "*") {
      if (chars.length > 0) lines.unshift(chars);
      lines.unshift("*");
      continue;
    }

    if (" \r\n\v\t".indexOf(ba[i]) >= 0) {
      if (chars.length > 0) {
        if (chars !== "FAR" && !chars.startsWith("#")) {
          lines.unshift(chars);
        }
        chars = "";
      }
      continue;
    }
    if (ba[i] === "(") {
      if (chars.length > 0) lines.unshift(chars);
      chars = "";
      continue;
    }
    chars = ba[i] + chars;
    if (ba[i] === ")") {
      let OpenParenthesesCount = 0;
      let CloseParenthesesCount = 1;
      let j = i - 1;
      for (; j >= 0; j -= 1) {
        chars = ba[j] + chars;
        if (ba[j] === "(") OpenParenthesesCount += 1;
        if (ba[j] === ")") CloseParenthesesCount += 1;
        if (OpenParenthesesCount === CloseParenthesesCount) {
          break;
        }
      }
      i = j;
    }
  }
  if (chars.length > 0) lines.unshift(chars);
  return lines;
}

// let lines = split_return_type_lines("abc\r\nWINOLEAPI_(_Ret_opt_ _Post_writable_byte_size_(cb)  __drv_allocatesMem(Mem) _Check_return_ LPVOID)");

interface ReturnTypeInfo {
  type: string;
  conv: string;
}

function find_function_ret_type(
  arch: "x86" | "x64",
  ba: string,
  firstChar: string
): ReturnTypeInfo | undefined {
  let lines = [] as string[];
  let return_conv = "__stdcall" as "__stdcall" | "__cdecl" | "__fastcall";
  let return_type: string | undefined = undefined;
  do {
    if (firstChar === ",") {
      lines = ba.split(/[,\(\r\n]/g).filter(function (e) {
        return e !== "(" && e !== "," && e.trim().length > 0;
      });
      lines = join_pointers(lines.map((x) => x.trim()));
      let api_type = lines[lines.length - 1];
      if (api_type === "_ACRTIMP") {
        let return_policy = lines[lines.length - 2];
        if (return_policy === "__RETURN_POLICY_DST") {
          return_type = lines[lines.length - 3];
        } else if (
          return_policy === "__DEFINE_CPP_OVERLOAD_STANDARD_NFUNC_0_2_SIZE"
        ) {
          return_type = "size_t";
        } else {
          console.log(`Failed for return_policy: ${return_policy}`);
          process.exit(-1);
        }
      } else if (api_type === "__EMPTY_DECLSPEC") {
        return_type = lines[lines.length - 3];
      }
      break;
    } else {
      lines = split_return_type_lines(ba);
      lines = join_pointers(lines);
      let api_type = lines[lines.length - 1];
      if (api_type === "FASTCALL") {
        if (arch === "x86") {
          return_conv = "__fastcall";
        } else {
          return_conv = "__cdecl";
        }
        return_type = lines[lines.length - 2];
        break;
      }

      if (API_LIST_STDCALL.indexOf(api_type) >= 0) {
        let inline_token = lines[lines.length - 3];
        if (inline_token === "inline") {
          break;
        }
        return_type = lines[lines.length - 2];
        break;
      }
      if (API_LIST_CDECL.indexOf(api_type) >= 0) {
        let inline_token = lines[lines.length - 3];
        if (inline_token === "inline") {
          break;
        }
        return_type = lines[lines.length - 2];
        return_conv = "__cdecl";
        break;
      }
      if (TYPE_LIST.indexOf(api_type) >= 0) {
        return_conv = "__cdecl";
        return_type = api_type;
        break;
      }
      if (api_type.startsWith("STDAPI_(")) {
        return_type = api_type.substring(
          "STDAPI_(".length,
          api_type.length - 1
        );
        break;
      }
      if (HRESULT_TYPE_LIST.indexOf(api_type) >= 0) {
        return_type = "HRESULT";
        break;
      }
      if (api_type.startsWith("NOT_BUILD_WINDOWS_DEPRECATE_NDFAPI_STDAPI(")) {
        return_type = "HRESULT";
        break;
      }

      if (api_type === "NETIOAPI_API") {
        return_type = "NETIO_STATUS";
        break;
      }
      if (api_type === "PFAPIENTRY") {
        return_type = "DWORD";
        break;
      }
      if (api_type === "PDH_FUNCTION") {
        return_type = "PDH_STATUS";
        break;
      }
      for (let i = 0; i < API_LIST_SUB.length; i += 1) {
        let item = API_LIST_SUB[i];
        if (api_type.startsWith(item)) {
          if (i < 1) {
            return_conv = "__cdecl";
          }
          return_type = api_type.substring(item.length, api_type.length - 1);
          break;
        }
      }
      // do not add more
    }
  } while (0);
  if (return_type === undefined) {
    return undefined;
  }
  return {
    conv: return_conv,
    type: return_type,
  };
}

interface ParameterInfo {
  firstChar: string;
  tail: string;
}

async function find_function_tail(
  key: string,
  ba: string
): Promise<ParameterInfo | undefined> {
  let OpenParenthesesCount = 0;
  let CloseParenthesesCount = 0;
  let TailFound: string | undefined = undefined;
  let firstCharNonSpace = "";
  let i = 0;
  ba = strip_comment(ba);
  for (;;) {
    if (" \t\n\r\v".indexOf(ba[i]) < 0) {
      if (ba[i] === "(" || ba[i] === ",") {
        OpenParenthesesCount = 1;
        firstCharNonSpace = ba[i];
        i += 1;
        break;
      } else if (ba[i] === ")") {
        OpenParenthesesCount = 0;
        firstCharNonSpace = ba[i];
        i += 1;
        break;
      }
      return TailFound;
    }
    i += 1;
  }
  let beginOffset = i;
  for (; i < ba.length; i += 1) {
    if (ba[i] === "(") OpenParenthesesCount += 1;
    if (ba[i] === ")") CloseParenthesesCount += 1;
    if (
      ba[i] === ";" ||
      (OpenParenthesesCount > 0 &&
        OpenParenthesesCount == CloseParenthesesCount)
    ) {
      if (ba[i] === ")") i += 1;
      if (firstCharNonSpace == ")") {
        TailFound = ba.substring(beginOffset, i);
      } else {
        TailFound = "(" + ba.substring(beginOffset, i);
      }
      break;
    }
    if (ba[i] === "{" || ba[i] === "}") return undefined;
  }
  if (
    OpenParenthesesCount == CloseParenthesesCount &&
    TailFound !== undefined
  ) {
    let TailFoundFinal = await handle_preprocess(TailFound);
    TailFoundFinal = TailFoundFinal.trim();
    if (TailFoundFinal.indexOf("&") >= 0) {
      // console.warn(`${key} have &`)
      return undefined;
    }
    return {
      firstChar: firstCharNonSpace,
      tail: TailFoundFinal,
    };
  }
  return undefined;
}

interface MergeResult {
  decls: string
};

// DbgPrintReturnControlC not present in phnt
async function exportFiles(
  mergeResult: MergeResult,
  arch: "x86" | "x64",
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
    const finalKey = (FallbackNames[key] ?? key) as string;
    let keyBuffer = Buffer.from(finalKey, "utf-8");
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
        let functionMatchedAnsi =
          sa.content[offsetBegin] === AsciiCode_A &&
          FunctionPrefixTable.indexOf(sa.content[offsetBegin + 1]) >= 0;
        if (functionMatched || functionMatchedAnsi) {
          matchedIndex.push(i);
        }
      }
    }
    if (keysToIgnore.has(key)) matchedIndex = [];
    let tailFound: ParameterInfo | undefined;
    let returnTypeFound: ReturnTypeInfo | undefined;
    for (let i = 0; i < matchedIndex.length; i++) {
      let indexFound = matchedIndex[i];
      let offset = sa.index[indexFound];
      let tailOffset = offset + keyBuffer.length + 1;
      if (sa.content[tailOffset] === AsciiCode_A) {
        tailOffset += 1;
      }
      //let matched = sa.content.subarray(offset - 5, tailOffset) as Buffer;
      if (key === "SHPropStgReadMultiple") {
        i += 0;
      }
      let suffix = sa.content
        .subarray(tailOffset, tailOffset + 2048)
        .toString();
      let prefix = sa.content.subarray(offset - 256, offset).toString();
      let tailFoundCurrent = await find_function_tail(key, suffix);
      if (tailFoundCurrent !== undefined) {
        tailFound = tailFoundCurrent;
        returnTypeFound = find_function_ret_type(
          arch,
          prefix,
          tailFoundCurrent.firstChar
        );
        if (returnTypeFound != undefined) break;
      }
    }
    if (tailFound === undefined) {
      funcMap.set(key, 0);
    } else {
      if (returnTypeFound === undefined) {
        console.log(`Can not find return type for ${key}`);
        process.exit();
      }
      // console.log(`${returnTypeFound}`);
      mergeResult.decls = mergeResult.decls + `${returnTypeFound.type} ${returnTypeFound.conv} ${key}${tailFound.tail};\n`
    }
  }
  let KeysJoin: string[] = [];
  let KeysDelta: string[] = [];
  for (let key of allKeys) {
    keysToPushPop.push(key);
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

async function MergeFile(sa: SuffixArrayLoaded, mergeResult: MergeResult, arch: "x86" | "x64") {
  let keysToIgnore = new Set<string>(ApisToIgnore);
  let docsDirArch = path.join(docsDir, "gens", arch);
  let filesExports = await getFiles(docsDirArch);
  let gensDir = path.join(baseDir, "gens", arch);
  let keysToPushPop: string[] = [];
  for (let name of DllNameList) {
    await exportFiles(
      mergeResult,
      arch,
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

async function load(): Promise<SuffixArrayLoaded> {
  // await genWin32Headers();
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
  let mergeResult: MergeResult = {
    decls: ''
  };
  // await MergeFile(sa, "x86");
  await MergeFile(sa, mergeResult, "x64");
  await fs.writeFile('decs.c', mergeResult.decls)
}

allMerge();
