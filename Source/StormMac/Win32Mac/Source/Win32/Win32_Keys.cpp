// Storm's Win32-on-Mac layer: the keyboard, 1.14d Intel Mac band 00026d14..000270a4. A table
// of 256 key states (a repeat count per virtual key) is kept from the Carbon key events; the
// original file name is unknown (see Source/StormMac/README.md).
#include "Win32ListBox.h"

#if !defined(_WIN32) && D2_MAC_CARBON

#include <Carbon/Carbon.h>


// 1.14d mac: 003f0f20
// The key map GetAsyncKeyState read last (GetKeys: 128 bits).
static BYTE s_abKeyMap[16];
// 1.14d mac: 003f0f30
// Per virtual key: how often it has repeated while down, 0 when up.
static WORD s_awKeyState[256];
// 1.14d mac: 0039729c
// Nonzero: Command-key chords are not turned into WM_SYS* messages, except for ':', '=' and 'v'.
DWORD g_dwSysKeyFilter = 1;

// 1.14d mac: 00026d14
// Turns a Mac key event into the Win32 key message, in place.
void Keys_PrepareKeyMsg(MSG* pMsg, BOOL bUpdateState, BOOL bKeyDown, BYTE byMacKeyCode, BYTE byCharCode,
                        BOOL bAltDown) {
    BYTE byChar = byCharCode;
    if (byCharCode > 0x1F || byMacKeyCode > 0x2F) {
        byChar = s_awKeyState[VK_CONTROL] != 0 ? 0 : byCharCode;
    }
    switch (byMacKeyCode) {
    case 0x4C:
        byChar = 0x0D;
        break;
    case 0x60: case 0x61: case 0x62: case 0x63: case 0x64: case 0x65: case 0x67: case 0x69:
    case 0x6B: case 0x6D: case 0x6F: case 0x71: case 0x72: case 0x73: case 0x74: case 0x75:
    case 0x76: case 0x77: case 0x78: case 0x79: case 0x7A: case 0x7B: case 0x7C: case 0x7D:
    case 0x7E:
        byChar = 0;
        break;
    }
    UINT uVirtKey = Win32VirtualKey_From_MacScanCode(byMacKeyCode) & 0xFF;
    WORD wPrevState = s_awKeyState[uVirtKey];
    UINT uLParam;
    if (!bKeyDown) {
        uLParam = 1;
        if (bUpdateState) {
            s_awKeyState[uVirtKey] = 0;
        }
    } else {
        uLParam = 0x10000;
        if (wPrevState < 0xFF && bUpdateState != 0) {
            s_awKeyState[uVirtKey] = wPrevState + 1;
        }
    }
    uLParam |= byChar << 8;
    // Extended keys: the cursor and function keys.
    if (byMacKeyCode < 0x72) {
        UINT uBit = byMacKeyCode - 0x3D;
        if (uBit <= 0xF && ((0xC003u >> uBit) & 1)) {
            uLParam |= 0x80;
        }
    } else {
        UINT uBit = byMacKeyCode - 0x72;
        if (uBit <= 0xC && ((0x1EAFu >> uBit) & 1)) {
            uLParam |= 0x80;
        }
    }
    UINT uMsg;
    if (!bAltDown) {
        uMsg = (!bKeyDown) | WM_KEYDOWN;
    } else if (g_dwSysKeyFilter == 0 || byMacKeyCode == ':' || byMacKeyCode == '=' || byMacKeyCode == 'v') {
        uLParam |= 4;
        uMsg = (!bKeyDown) | WM_SYSKEYDOWN;
    } else {
        uMsg = 0;
    }
    pMsg->hwnd = GetFocus();
    if (pMsg->hwnd == NULL) {
        pMsg->hwnd = GetDesktopWindow();
    }
    pMsg->message = uMsg;
    pMsg->wParam = uVirtKey;
    pMsg->lParam = (wPrevState != 0) * 2 | uLParam;
}

// 1.14d mac: 00026f58
int GetKeyState(int nVirtKey) {
    if (s_awKeyState[nVirtKey] != 0) {
        return (nVirtKey == VK_CAPITAL) | -0x80;
    }
    return 0;
}

// 1.14d mac: 00026f82
short GetAsyncKeyState(BYTE byVirtKey) {
    BYTE byMacKey = Key_VirtualKeyToMacKeyCode(byVirtKey);
    BYTE byBit = byMacKey & 7;
    UINT byByte = byMacKey >> 3;
    BYTE abKeyMap[16];
    GetKeys(*(KeyMap*)abKeyMap);
    WORD wResult = 0;
    if ((abKeyMap[byByte] >> byBit) & 1) {
        wResult = ((s_abKeyMap[byByte] & (1 << byBit)) != 0) | 0x8000;
    }
    SMemCopy(s_abKeyMap, abKeyMap, sizeof(s_abKeyMap));
    return (short)wResult;
}

// 1.14d mac: 00027028
// WM_KEYDOWN or WM_SYSKEYDOWN with a character becomes WM_CHAR or WM_SYSCHAR.
BOOL TranslateMessage(const MSG* pMsg) {
    BOOL bTranslated = 0;
    if ((pMsg->message | 4) == WM_SYSKEYDOWN && (s_awKeyState[VK_LWIN] == 0 || pMsg->wParam == 0x0D)) {
        LPARAM lParam = pMsg->lParam;
        UINT uChar = lParam >> 8;
        if ((BYTE)uChar != 0) {
            PostMessage(pMsg->hwnd, (pMsg->message != WM_KEYDOWN) * 4 + WM_CHAR, (BYTE)uChar, lParam);
            bTranslated = 1;
        }
    }
    return bTranslated;
}

#endif  // D2_OS_MAC
