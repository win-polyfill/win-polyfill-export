#pragma once

#define _WIN32_WINNT 0x0A01
#define WINNT
#define WIN_NOEXCEPT_UNDEF

#define _CRT_FUNCTIONS_REQUIRED 1
#define _STL_COMPILER_PREPROCESSOR 1
#define _NO_CRT_STDIO_INLINE
#define _CTYPE_DISABLE_MACROS
#define _CRT_SECURE_CPP_OVERLOAD_SECURE_NAMES 0

#define DECLSPEC_NORETURN
#define WINBASE_DECLARE_RESTORE_LAST_ERROR
#define FE_IME
#define _IMAGEHLP_SOURCE_

#define PHNT_RTL_BYTESWAP
#define PHNT_ENABLE_ALL

#define abs abs_none
#define memchr memchr_none
#define strchr strchr_none
#define strpbrk strpbrk_none
#define strrchr strrchr_none
#define strstr strstr_none
#define wcschr wcschr_none
#define wcspbrk wcspbrk_none
#define wcsrchr wcsrchr_none
#define wcsstr wcsstr_none

#include <sal.h>

#include <math.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define NTSTATUS_DEFINED
typedef enum : int32_t
{
    NTSTATUS__XXX_SOMETHING
} NTSTATUS;
typedef NTSTATUS *PNTSTATUS;
#define NT_SUCCESS(Status) (((NTSTATUS)(Status)) >= 0)

#define _NTRTL_FWD_H
#include "win-polyfill-core/nt.h"

// The COM apis should at the beggning
#define RPCPROXY_ENABLE_CPP_NO_CINTERFACE
#include <callobj.h>
#include <messagedispatcherapi.h>
#include <rpcproxy.h>

// dxva2
#include <dxvahd.h>
#include <highlevelmonitorconfigurationapi.h>
#include <lowlevelmonitorconfigurationapi.h>
#include <opmapi.h>

// ncrypt
#include <ncrypt.h>
#include <ncryptprotect.h>

// mf
// dvn -> ks -> ksmedia -> mf
#include <dvp.h>

#include <ks.h>

#include <ksmedia.h>

#include <evr.h>
#include <mfapi.h>
#include <mfcontentdecryptionmodule.h>
#include <mfd3d12.h>
#include <mfidl.h>
#include <mfreadwrite.h>
#include <wmcontainer.h>

#include <aclapi.h>
#include <appmgmt.h>
#include <appmodel.h>
#include <devquery.h>
#include <errhandlingapi.h>
#include <evntcons.h>
#include <i_cryptasn1tls.h>
#include <ime.h>
#include <memoryapi.h>
#include <msacmdlg.h>
#include <mssip.h>
#include <ncrypt.h>
#include <olectl.h>
#include <oobenotification.h>
#include <perflib.h>
#include <processsnapshot.h>
#include <sddl.h>
#include <swdevice.h>
#include <tlhelp32.h>
#include <wct.h>
#include <wincred.h>
#include <windowsceip.h>
#include <winnls32.h>
#include <winsafer.h>

// shcore
#include <featurestagingapi.h>
#include <isolatedapplauncher.h>
#include <shcore.h>

#include <d3dkmthk.h>
#include <usp10.h>
#include <winddi.h>
#include <wingdi.h>
#include <winppi.h>

#include <ddrawint.h>
typedef PDD_HALINFO LPDDHALINFO;
typedef PDD_CALLBACKS LPDDHAL_DDCALLBACKS;
typedef PDD_PALETTECALLBACKS LPDDHAL_DDPALETTECALLBACKS;
typedef PDD_SURFACECALLBACKS LPDDHAL_DDSURFACECALLBACKS;
typedef PDD_VIDEOPORTCALLBACKS LPDDHAL_DDVIDEOPORTCALLBACKS;
typedef PDD_COLORCONTROLCALLBACKS LPDDHAL_DDCOLORCONTROLCALLBACKS;
typedef PDD_KERNELCALLBACKS LPDDHAL_DDKERNELCALLBACKS;
typedef PDD_MOTIONCOMPCALLBACKS LPDDHAL_DDMOTIONCOMPCALLBACKS;
typedef PDD_MISCELLANEOUS2CALLBACKS LPDDHAL_DDMISCELLANEOUS2CALLBACKS;
typedef PDD_D3DBUFCALLBACKS LPDDHAL_D3DBUFCALLBACKS;
typedef PDD_NTCALLBACKS LPDDHAL_DDNTCALLBACKS;

typedef struct _DDHAL_DDEXEBUFCALLBACKS FAR *LPDDHAL_DDEXEBUFCALLBACKS;
typedef struct _VIDMEM FAR *LPVIDMEM;
#include <ddrawgdi.h>

#include <dsgetdc.h>
#include <dsrole.h>
#include <icmpapi.h>


#include "gen-exports-fixes.h"
