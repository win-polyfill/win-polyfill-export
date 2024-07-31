#pragma once
#include <sdkddkver.h>

#define _CRT_STDIO_ARBITRARY_WIDE_SPECIFIERS

#ifndef _WINSOCK_DEPRECATED_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#endif

#define _WINSOCKAPI_
#define PSAPI_VERSION 1

#define INITKNOWNFOLDERS

#include <intrin.h>
#include <strsafe.h>

#ifndef UMDF_USING_NTSTATUS
#define UMDF_USING_NTSTATUS
#endif

#include <winsock2.h>

#include <windows.h>

#include <ip2string.h>
#include <windns.h>
#include <ws2def.h>
#include <ws2ipdef.h>
#include <ws2tcpip.h>

/* iphlpapi should after windns ws2def ws2ipdef ws2tcpip */
#include <iphlpapi.h>

/* lmjoin should after iphlpapi */
#include <lmjoin.h>

#include <ntstatus.h>
#include <winnt.h>

/* for pathcch.h */
#define PATHCCH_NO_DEPRECATE
#include <pathcch.h>

#include <d3d9.h>
#include <d3dhal.h>

#include <bcrypt.h>
#include <bluetoothleapis.h>
#include <cfgmgr32.h>
#include <d3d11.h>
#include <d3d12.h>
#include <dbghelp.h>
#include <dpapi.h>
#include <dwmapi.h>
#include <dxgi.h>
#include <dxva2api.h>
#include <esent.h>
#include <evntprov.h>
#include <evntrace.h>
#include <mfapi.h>
#include <mfidl.h>
#include <mfreadwrite.h>
#include <mschapp.h>
#include <ncrypt.h>
#include <ndfapi.h>
#include <powrprof.h>
#include <processthreadsapi.h>
#include <propvarutil.h>
#include <psapi.h>
#include <roapi.h>
#include <roerrorapi.h>
#include <setupapi.h>
#include <shellapi.h>
#include <shellscalingapi.h>
#include <shlobj.h>
#include <shlwapi.h>
#include <threadpoolapiset.h>
#include <timezoneapi.h>
#include <uiautomation.h>
#include <usb.h>
#include <userenv.h>
#include <uxtheme.h>
#include <werapi.h>
#include <winevt.h>
#include <winhttp.h>
#include <winnls.h>
#include <winstring.h>
#include <winusb.h>
#ifdef __cplusplus
#include <dwrite.h>
#include <dwrite_3.h>
#endif
