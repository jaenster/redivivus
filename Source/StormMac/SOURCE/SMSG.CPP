// Storm's SMsg, 1.14d Intel Mac build (DiabloII_macho): window messages routed through
// SEvt. Band 00018cdb..00019532, ended by the file's static initialiser (000194e0).
// The Windows twin is Source/Storm/WindowHandle.cpp.
//
// A message handler is an SEvt handler under one of five event types, keyed by the window.
// Every window that has had a handler registered or removed is tracked in a list; deleting
// the window drops its handlers of all five types.
#include "Source/StormMac/H/stpl.h"

#if !defined(_WIN32) && D2_MAC_CARBON

#include "Source/StormMac/H/StormApi.h"

#include <stdint.h>

// 1.14d mac: 00011280
int SEvtDispatch(uint32_t dwType, uint32_t dwSubType, uint32_t dwId, void* pData);
// 1.14d mac: 0001183c
int SEvtRegisterHandler(uint32_t dwType, uint32_t dwSubType, uint32_t dwId, int nFlags, void (*pfnHandler)(void*));
// 1.14d mac: 00011c21
int SEvtUnregisterHandler(uint32_t dwType, uint32_t dwSubType, uint32_t dwId, void (*pfnHandler)(void*));
// 1.14d mac: 000115a5
int SEvtUnregisterType(uint32_t dwType, uint32_t dwSubType);
// 1.14d mac: 0001169c
int SEvtPushState(uint32_t dwType, uint32_t dwParam);
// 1.14d mac: 00011504
int SEvtPopState(uint32_t dwType, uint32_t dwParam);
// 1.14d mac: 000110bc
int SEvtBreakHandlerChain(void* pData);

#define SMSG_EVENT_MESSAGE 0x534D5347  // 'SMSG': any message, keyed by the message
#define SMSG_EVENT_COMMAND 0x534D5348  // 'SMSH': WM_COMMAND, keyed by the command
#define SMSG_EVENT_SYSCOMMAND 0x534D5349  // 'SMSI': WM_SYSCOMMAND
#define SMSG_EVENT_KEYDOWN 0x534D534A  // 'SMSJ': WM_KEYDOWN, keyed by the key
#define SMSG_EVENT_KEYUP 0x534D534B  // 'SMSK': WM_KEYUP

#define WM_NCDESTROY 0x0082
#define WM_KEYDOWN 0x0100
#define WM_KEYUP 0x0101
#define WM_COMMAND 0x0111
#define WM_SYSCOMMAND 0x0112
#define ERROR_INVALID_PARAMETER 0x57

// A tracked window, 0xC bytes: the list link, then the window.
struct SMsgWindowRecord {
    TSLink<SMsgWindowRecord> m_link;  // 0x00
    void* hWnd;                       // 0x08
};

// What SEvtDispatch hands a message handler.
struct SMsgEventData {
    void* hWnd;         // 0x00
    uint32_t nMsg;      // 0x04
    uint32_t nWParam;   // 0x08
    int32_t nLParam;    // 0x0C
    uint32_t nCmdCode;  // 0x10  the notification code of a WM_COMMAND
    uint32_t dwUnused;  // 0x14
    int32_t bHandled;   // 0x18
    int32_t nResult;    // 0x1C
};

// 1.14d mac: 003f0950
static TSLinkList<SMsgWindowRecord> s_tWindowList;
// 1.14d mac: 003f095c
// The window messages are currently dispatched for.
void* g_pActiveWindow;
// 1.14d mac: 003f0960
int g_nActiveWindowState[4];

// The instantiations clang-425 emitted into this file's object (stpl.h):
// 1.14d mac: 0001929a TSLinkList<SMsgWindowRecord>::~TSLinkList[D1]
// 1.14d mac: 000192bc TSLinkList<SMsgWindowRecord>::UnlinkAll
// 1.14d mac: 00019304 TSLink<SMsgWindowRecord>::Unlink
// 1.14d mac: 00019344 TSLink<SMsgWindowRecord>::NextLink
// 1.14d mac: 0001937a TSLinkList<SMsgWindowRecord>::NewNode
// 1.14d mac: 000193f0 TSLinkList<SMsgWindowRecord>::LinkNode
// 1.14d mac: 00019464 TSLinkList<SMsgWindowRecord>::DeleteNode
// 1.14d mac: 000194e0 _GLOBAL__I_a

static void SMSG_DeleteWindow(void* pWindow);
static SMsgWindowRecord* SMSG_FindWindow(void* hWnd);
static int SMSG_InternalRegister(uint32_t dwEventType, void* pWindow, uint32_t dwKey, void* pfnHandler);
static int SMSG_InternalUnregister(uint32_t dwEventType, void* pWindow, uint32_t dwKey, void* pfnHandler);

// 1.14d mac: 00018cdb
int Storm::SMsgBreakHandlerChain(void* pData) {
    return SEvtBreakHandlerChain(pData);
}

// 1.14d mac: 00018ce4
int SMsgDestroy(void) {
    while (!TSLink<SMsgWindowRecord>::IsTerminator(s_tWindowList.m_terminator.m_next)) {
        SMSG_DeleteWindow(s_tWindowList.m_terminator.m_next->hWnd);
    }
    return 1;
}

// 1.14d mac: 00018d1b
static void SMSG_DeleteWindow(void* pWindow) {
    if (g_pActiveWindow == pWindow) {
        g_pActiveWindow = NULL;
        g_nActiveWindowState[1] = 0;
        g_nActiveWindowState[0] = 0;
        g_nActiveWindowState[3] = 0;
        g_nActiveWindowState[2] = 0;
    }
    SMsgWindowRecord* pRecord = SMSG_FindWindow(pWindow);
    if (pRecord) {
        SEvtUnregisterType(SMSG_EVENT_MESSAGE, (uint32_t)pWindow);
        SEvtUnregisterType(SMSG_EVENT_COMMAND, (uint32_t)pWindow);
        SEvtUnregisterType(SMSG_EVENT_SYSCOMMAND, (uint32_t)pWindow);
        SEvtUnregisterType(SMSG_EVENT_KEYUP, (uint32_t)pWindow);
        SEvtUnregisterType(SMSG_EVENT_KEYDOWN, (uint32_t)pWindow);
        s_tWindowList.DeleteNode(pRecord);
    }
}

// 1.14d mac: 00018ddd
int Storm::WindowHandle::SMsgDispatchMessage(Win32MacWindow* pWindow, uint32_t nMsg, uint32_t nWParam, int32_t nLParam, int* pbHandled,
                        int* pnResult) {
    SMsgEventData tData;
    if (pbHandled) {
        *pbHandled = 0;
    }
    if (pnResult) {
        *pnResult = 0;
    }
    tData.hWnd = pWindow;
    tData.nMsg = nMsg;
    tData.nWParam = nWParam;
    tData.nLParam = nLParam;
    tData.nCmdCode = 0;
    if (nMsg == WM_COMMAND) {
        tData.nCmdCode = nWParam >> 16;
    }
    tData.bHandled = 0;
    tData.nResult = 0;
    void* pTarget = pWindow;
    do {
        SEvtDispatch(SMSG_EVENT_MESSAGE, (uint32_t)pTarget, nMsg, &tData);
        uint32_t dwEventType;
        uint32_t nKey;
        if (nMsg < WM_COMMAND) {
            if (nMsg == WM_KEYDOWN) {
                dwEventType = SMSG_EVENT_KEYDOWN;
                nKey = nWParam;
                SEvtDispatch(dwEventType, (uint32_t)pTarget, nKey, &tData);
            } else if (nMsg == WM_KEYUP) {
                dwEventType = SMSG_EVENT_KEYUP;
                nKey = nWParam;
                SEvtDispatch(dwEventType, (uint32_t)pTarget, nKey, &tData);
            }
        } else {
            if (nMsg == WM_SYSCOMMAND) {
                dwEventType = SMSG_EVENT_SYSCOMMAND;
                nKey = nWParam;
                SEvtDispatch(dwEventType, (uint32_t)pTarget, nKey, &tData);
            } else if (nMsg == WM_COMMAND) {
                dwEventType = SMSG_EVENT_COMMAND;
                nKey = nWParam & 0xFFFF;
                SEvtDispatch(dwEventType, (uint32_t)pTarget, nKey, &tData);
            }
        }
        // A window message is also offered to the active window's handlers.
        bool bOther = pTarget != g_pActiveWindow;
        if (pTarget == NULL || bOther) {
            if (nMsg == WM_NCDESTROY) {
                SMSG_DeleteWindow(pWindow);
            }
            if (pbHandled) {
                *pbHandled = tData.bHandled;
            }
            if (pnResult) {
                *pnResult = tData.nResult;
            }
            return 1;
        }
        pTarget = NULL;
    } while (true);
}

// 1.14d mac: 00018f40
void* SMsgGetDefaultWindow(void) {
    return g_pActiveWindow;
}

// 1.14d mac: 00018f51
int Storm::WindowHandle::SMsgPopRegisterState(Win32MacWindow* pParam) {
    uint32_t dwParam = (uint32_t)pParam;
    SEvtPopState(SMSG_EVENT_COMMAND, dwParam);
    SEvtPopState(SMSG_EVENT_SYSCOMMAND, dwParam);
    SEvtPopState(SMSG_EVENT_KEYDOWN, dwParam);
    SEvtPopState(SMSG_EVENT_KEYUP, dwParam);
    SEvtPopState(SMSG_EVENT_MESSAGE, dwParam);
    return 1;
}

// 1.14d mac: 00018fb6
int Storm::WindowHandle::SMsgPushRegisterState(Win32MacWindow* pParam) {
    uint32_t dwParam = (uint32_t)pParam;
    SEvtPushState(SMSG_EVENT_COMMAND, dwParam);
    SEvtPushState(SMSG_EVENT_SYSCOMMAND, dwParam);
    SEvtPushState(SMSG_EVENT_KEYDOWN, dwParam);
    SEvtPushState(SMSG_EVENT_KEYUP, dwParam);
    SEvtPushState(SMSG_EVENT_MESSAGE, dwParam);
    return 1;
}

// 1.14d mac: 0001901b
void Storm::WindowHandle::SMsgRegisterCommand(Win32MacWindow* hWindow, uint32_t nCommandId, void (*pfnHandler)(void*)) {
    SMSG_InternalRegister(SMSG_EVENT_COMMAND, hWindow, nCommandId, (void*)pfnHandler);
}

// 1.14d mac: 00019040
static int SMSG_InternalRegister(uint32_t dwEventType, void* pWindow, uint32_t dwKey, void* pfnHandler) {
    if (!pfnHandler) {
        SErrSetLastError(ERROR_INVALID_PARAMETER);
        return 0;
    }
    if (!SMSG_FindWindow(pWindow)) {
        SMsgWindowRecord* pRecord = s_tWindowList.NewNode(STORM_LIST_LINK_BEFORE, 0, 0);
        pRecord->hWnd = pWindow;
    }
    return SEvtRegisterHandler(dwEventType, (uint32_t)pWindow, dwKey, 0, (void (*)(void*))pfnHandler);
}

// 1.14d mac: 000190c9
void Storm::WindowHandle::SMsgRegisterKeyDown(Win32MacWindow* hWindow, uint32_t nKey, void (*pfnHandler)(void*)) {
    SMSG_InternalRegister(SMSG_EVENT_KEYDOWN, hWindow, nKey, (void*)pfnHandler);
}

// 1.14d mac: 000190ee
void Storm::WindowHandle::SMsgRegisterKeyUp(Win32MacWindow* hWindow, uint32_t nKey, void (*pfnHandler)(void*)) {
    SMSG_InternalRegister(SMSG_EVENT_KEYUP, hWindow, nKey, (void*)pfnHandler);
}

// 1.14d mac: 00019113
void Storm::WindowHandle::SMsgRegisterMessage(Win32MacWindow* hWindow, uint32_t nMessage, void (*pfnHandler)(void*)) {
    SMSG_InternalRegister(SMSG_EVENT_MESSAGE, hWindow, nMessage, (void*)pfnHandler);
}

// 1.14d mac: 00019138
void Storm::WindowHandle::SMsgUnregisterCommand(Win32MacWindow* hWindow, uint32_t nCommandId, void (*pfnHandler)(void*)) {
    SMSG_InternalUnregister(SMSG_EVENT_COMMAND, hWindow, nCommandId, (void*)pfnHandler);
}

// 1.14d mac: 0001915d
static int SMSG_InternalUnregister(uint32_t dwEventType, void* pWindow, uint32_t dwKey, void* pfnHandler) {
    if (!SMSG_FindWindow(pWindow)) {
        SMsgWindowRecord* pRecord = s_tWindowList.NewNode(STORM_LIST_LINK_BEFORE, 0, 0);
        pRecord->hWnd = pWindow;
    }
    return SEvtUnregisterHandler(dwEventType, (uint32_t)pWindow, dwKey, (void (*)(void*))pfnHandler);
}

// 1.14d mac: 000191c6
void Storm::WindowHandle::SMsgUnregisterKeyDown(Win32MacWindow* hWindow, uint32_t nKey, void (*pfnHandler)(void*)) {
    SMSG_InternalUnregister(SMSG_EVENT_KEYDOWN, hWindow, nKey, (void*)pfnHandler);
}

// 1.14d mac: 000191eb
void Storm::WindowHandle::SMsgUnregisterKeyUp(Win32MacWindow* hWindow, uint32_t nKey, void (*pfnHandler)(void*)) {
    SMSG_InternalUnregister(SMSG_EVENT_KEYUP, hWindow, nKey, (void*)pfnHandler);
}

// 1.14d mac: 00019210
void Storm::WindowHandle::SMsgUnregisterMessage(Win32MacWindow* hWindow, uint32_t nMessage, void (*pfnHandler)(void*)) {
    SMSG_InternalUnregister(SMSG_EVENT_MESSAGE, hWindow, nMessage, (void*)pfnHandler);
}

// 1.14d mac: 00019235
static SMsgWindowRecord* SMSG_FindWindow(void* hWnd) {
    SMsgWindowRecord* pFound = NULL;
    for (SMsgWindowRecord* pRecord = s_tWindowList.Head(); !TSLink<SMsgWindowRecord>::IsTerminator(pRecord);
         pRecord = s_tWindowList.Link(pRecord)->m_next) {
        if (pRecord->hWnd == hWnd) {
            pFound = pRecord;
            break;
        }
    }
    return pFound;
}

#endif  // D2_OS_MAC
