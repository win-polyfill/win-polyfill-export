/*++
Copyright (c) Microsoft Corporation.
Licensed under the MIT license.

Module Name:
- conapi.h

Abstract:
- This module contains the internal structures and definitions used by the console server.

Author:
- Therese Stowell (ThereseS) 12-Nov-1990

Revision History:
--*/

#pragma once
// these should be in precomp but aren't being picked up.
#define STATUS_SHARING_VIOLATION         ((NTSTATUS)0xC0000043L)

#include <wincon.h>

#include <ntcsrdll.h>
#include <csrmsg.h>

#include "conmsgl1.h"
#include "conmsgl2.h"
#include "conmsgl3.h"

#include "condrv.h"
#include "ConIoSrv.h"

#include "ntcon.h"


typedef struct _CONSOLE_API_MSG
{
    // From here down is the actual packet data sent/received.
    CD_IO_DESCRIPTOR Descriptor;
    union
    {
        struct
        {
            CD_CREATE_OBJECT_INFORMATION CreateObject;
            CONSOLE_CREATESCREENBUFFER_MSG CreateScreenBuffer;
        };
        struct
        {
            CONSOLE_MSG_HEADER msgHeader;
            union
            {
                CONSOLE_MSG_BODY_L1 consoleMsgL1;
                CONSOLE_MSG_BODY_L2 consoleMsgL2;
                CONSOLE_MSG_BODY_L3 consoleMsgL3;
            } u;
        };
    };
    // End packet data

    // DO NOT PUT MORE FIELDS DOWN HERE.
    // The tail end of this structure will have a console driver packet
    // copied over it and it will overwrite any fields declared here.

} CONSOLE_API_MSG, *PCONSOLE_API_MSG, * const PCCONSOLE_API_MSG;

typedef enum _SrvApiNumber {
    SrvWriteConsole = 542, // 4.0 to 6.0
} SrvApiNumber;

typedef struct _CONSOLE_API_MSG_XP {
    PORT_MESSAGE h;
    PCSR_CAPTURE_HEADER CaptureBuffer;
    ULONG ApiNumber;
    ULONG ReturnValue;
    ULONG Reserved;
    union {
        CONSOLE_GETCONSOLEINPUT_MSG GetConsoleInput;
        CONSOLE_WRITECONSOLEINPUT_MSG WriteConsoleInput;
        CONSOLE_READCONSOLEOUTPUT_MSG ReadConsoleOutput;
        CONSOLE_WRITECONSOLEOUTPUT_MSG WriteConsoleOutput;
        CONSOLE_READCONSOLEOUTPUTSTRING_MSG ReadConsoleOutputString;
        CONSOLE_WRITECONSOLEOUTPUTSTRING_MSG WriteConsoleOutputString;
        CONSOLE_FILLCONSOLEOUTPUT_MSG FillConsoleOutput;
        CONSOLE_MODE_MSG GetConsoleMode;
        CONSOLE_GETNUMBEROFFONTS_MSG GetNumberOfConsoleFonts;
        CONSOLE_GETNUMBEROFINPUTEVENTS_MSG GetNumberOfConsoleInputEvents;
        CONSOLE_GETCURSORINFO_MSG GetConsoleCursorInfo;
        CONSOLE_GETMOUSEINFO_MSG GetConsoleMouseInfo;
        CONSOLE_GETFONTINFO_MSG GetConsoleFontInfo;
        CONSOLE_GETFONTSIZE_MSG GetConsoleFontSize;
        CONSOLE_MODE_MSG SetConsoleMode;
        CONSOLE_GETLARGESTWINDOWSIZE_MSG GetLargestConsoleWindowSize;
        CONSOLE_SETSCREENBUFFERSIZE_MSG SetConsoleScreenBufferSize;
        CONSOLE_SETCURSORPOSITION_MSG SetConsoleCursorPosition;
        CONSOLE_SETCURSORINFO_MSG SetConsoleCursorInfo;
        CONSOLE_SETWINDOWINFO_MSG SetConsoleWindowInfo;
        CONSOLE_SCROLLSCREENBUFFER_MSG ScrollConsoleScreenBuffer;
        CONSOLE_SETTEXTATTRIBUTE_MSG SetConsoleTextAttribute;
        CONSOLE_SETFONT_MSG SetConsoleFont;
        CONSOLE_SETICON_MSG SetConsoleIcon;
        CONSOLE_READCONSOLE_MSG ReadConsole;
        CONSOLE_WRITECONSOLE_MSG_XP WriteConsole;
        CONSOLE_GETTITLE_MSG GetConsoleTitle;
        CONSOLE_SETTITLE_MSG SetConsoleTitle;
        CONSOLE_CREATESCREENBUFFER_MSG CreateConsoleScreenBuffer;
        CONSOLE_INVALIDATERECT_MSG InvalidateConsoleBitmapRect;
        CONSOLE_VDM_MSG VDMConsoleOperation;
        CONSOLE_SETCURSOR_MSG SetConsoleCursor;
        CONSOLE_SHOWCURSOR_MSG ShowConsoleCursor;
        CONSOLE_MENUCONTROL_MSG ConsoleMenuControl;
        CONSOLE_SETPALETTE_MSG SetConsolePalette;
        CONSOLE_SETDISPLAYMODE_MSG SetConsoleDisplayMode;
        CONSOLE_REGISTERVDM_MSG RegisterConsoleVDM;
        CONSOLE_GETHARDWARESTATE_MSG GetConsoleHardwareState;
        CONSOLE_SETHARDWARESTATE_MSG SetConsoleHardwareState;
        CONSOLE_GETDISPLAYMODE_MSG GetConsoleDisplayMode;
        CONSOLE_ADDALIAS_MSG AddConsoleAliasW;
        CONSOLE_GETALIAS_MSG GetConsoleAliasW;
        CONSOLE_GETALIASESLENGTH_MSG GetConsoleAliasesLengthW;
        CONSOLE_GETALIASEXESLENGTH_MSG GetConsoleAliasExesLengthW;
        CONSOLE_GETALIASES_MSG GetConsoleAliasesW;
        CONSOLE_GETALIASEXES_MSG GetConsoleAliasExesW;
        CONSOLE_EXPUNGECOMMANDHISTORY_MSG ExpungeConsoleCommandHistoryW;
        CONSOLE_SETNUMBEROFCOMMANDS_MSG SetConsoleNumberOfCommandsW;
        CONSOLE_GETCOMMANDHISTORYLENGTH_MSG GetConsoleCommandHistoryLengthW;
        CONSOLE_GETCOMMANDHISTORY_MSG GetConsoleCommandHistoryW;
        CONSOLE_GETCP_MSG GetConsoleCP;
        CONSOLE_SETCP_MSG SetConsoleCP;
        CONSOLE_SETKEYSHORTCUTS_MSG SetConsoleKeyShortcuts;
        CONSOLE_SETMENUCLOSE_MSG SetConsoleMenuClose;
        CONSOLE_CTRLEVENT_MSG GenerateConsoleCtrlEvent;
        CONSOLE_GETKEYBOARDLAYOUTNAME_MSG GetKeyboardLayoutName;
        CONSOLE_GETCONSOLEWINDOW_MSG GetConsoleWindow;
        CONSOLE_LANGID_MSG GetConsoleLangId;
        CONSOLE_GETSELECTIONINFO_MSG GetConsoleSelectionInfo;
        CONSOLE_GETCONSOLEPROCESSLIST_MSG GetConsoleProcessList;
    } u;
} CONSOLE_API_MSG_XP, *PCONSOLE_API_MSG_XP;
