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

// winsock2 will include windows.h
#include <winsock2.h>

#include <ntstatus.h>
#include <winnt.h>
#ifdef WIN_NOEXCEPT_UNDEF
#undef WIN_NOEXCEPT
#define WIN_NOEXCEPT
#endif

#include <windns.h>
#include <ws2def.h>
#include <ws2ipdef.h>
#include <ws2spi.h>
#include <ws2tcpip.h>

#include <ip2string.h>
#include <sporder.h>

/* iphlpapi should after windns ws2def ws2ipdef ws2tcpip */
#include <iphlpapi.h>
#include <fltdefs.h>

/* lmjoin should after iphlpapi */
#include <lmjoin.h>
#include <lmserver.h>

/* for pathcch.h */
#define PATHCCH_NO_DEPRECATE
#include <pathcch.h>

#include <d3d9.h>
#include <d3dhal.h>

#include <d3d11.h>
#include <d3d12.h>
#include <windows.graphics.directx.direct3d11.interop.h>
#include <dxgi.h>
#include <dxgi1_3.h>
#include <dxgi1_6.h>
#include <dxva2api.h>
#include <d3d11on12.h>
#include <d3d9on12.h>

#include <bcrypt.h>
#include <bluetoothapis.h>
#include <bluetoothleapis.h>
#include <cfgmgr32.h>
#include <dbghelp.h>
#include <dpapi.h>
#include <dwmapi.h>
#include <esent.h>
#include <evntprov.h>
#include <evntrace.h>
#include <mschapp.h>
#include <ndfapi.h>
#include <powrprof.h>
#include <processthreadsapi.h>
#define CINTERFACE
#include <propvarutil.h>
#undef CINTERFACE
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
