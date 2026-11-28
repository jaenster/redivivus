// Win32Posix: the keyboard on SDL3 (Win32Mac: Win32_Keys.cpp).
//
// An SDL key event becomes WM_KEYDOWN/WM_KEYUP, or WM_SYSKEYDOWN/WM_SYSKEYUP while Alt
// (Option) is held without Ctrl and for F10, with Windows' packing: wParam the virtual key,
// lParam the repeat count (bits 0-15), the PC scan code (16-23), the extended-key bit (24),
// Alt held (29), the previous key state (30) and the transition (31). Letters take the
// virtual key of the letter the layout puts on the key, as Win32Mac's KeyTranslate lookup
// does; every other key its US position. Shift, Ctrl and Alt report VK_SHIFT, VK_CONTROL and
// VK_MENU, with their left and right keys kept apart for GetKeyState.
//
// The Command key is VK_LWIN/VK_RWIN, as in Win32Mac's key table, and it types no characters
// (Win32Mac's TranslateMessage drops them while VK_LWIN is down). On the 1.14d Mac, Cmd-X,
// Cmd-C, Cmd-V and Cmd-A reach the focused edit box through the Edit menu, which
// D2WINEDITBOX_HandleMenuCommand turns into the control characters 0x18, 0x03, 0x16 and
// 0x01; with no menu here TranslateMessage posts those characters itself. Ctrl+letter gives
// the same control characters, as on Windows.
//
// Printable characters come from SDL text input (WM_CHAR, one per Windows-1252 byte, IME
// commits included); TranslateMessage adds the ones a key gives without text input:
// Backspace, Tab, Enter, Escape and the control characters.
#include "Source/StormMac/Win32Posix/UserInternal.h"

#if !defined(_WIN32) && D2_BACKEND_POSIX

#include <string.h>

namespace Win32Posix {

struct KeyRow {
    SDL_Scancode nScancode;
    BYTE nVk;
    BYTE nScan;  // PC scan code set 1
    bool bExtended;
};

static const KeyRow s_aKeys[] = {
    {SDL_SCANCODE_A, 'A', 0x1E, false}, {SDL_SCANCODE_B, 'B', 0x30, false}, {SDL_SCANCODE_C, 'C', 0x2E, false},
    {SDL_SCANCODE_D, 'D', 0x20, false}, {SDL_SCANCODE_E, 'E', 0x12, false}, {SDL_SCANCODE_F, 'F', 0x21, false},
    {SDL_SCANCODE_G, 'G', 0x22, false}, {SDL_SCANCODE_H, 'H', 0x23, false}, {SDL_SCANCODE_I, 'I', 0x17, false},
    {SDL_SCANCODE_J, 'J', 0x24, false}, {SDL_SCANCODE_K, 'K', 0x25, false}, {SDL_SCANCODE_L, 'L', 0x26, false},
    {SDL_SCANCODE_M, 'M', 0x32, false}, {SDL_SCANCODE_N, 'N', 0x31, false}, {SDL_SCANCODE_O, 'O', 0x18, false},
    {SDL_SCANCODE_P, 'P', 0x19, false}, {SDL_SCANCODE_Q, 'Q', 0x10, false}, {SDL_SCANCODE_R, 'R', 0x13, false},
    {SDL_SCANCODE_S, 'S', 0x1F, false}, {SDL_SCANCODE_T, 'T', 0x14, false}, {SDL_SCANCODE_U, 'U', 0x16, false},
    {SDL_SCANCODE_V, 'V', 0x2F, false}, {SDL_SCANCODE_W, 'W', 0x11, false}, {SDL_SCANCODE_X, 'X', 0x2D, false},
    {SDL_SCANCODE_Y, 'Y', 0x15, false}, {SDL_SCANCODE_Z, 'Z', 0x2C, false},
    {SDL_SCANCODE_1, '1', 0x02, false}, {SDL_SCANCODE_2, '2', 0x03, false}, {SDL_SCANCODE_3, '3', 0x04, false},
    {SDL_SCANCODE_4, '4', 0x05, false}, {SDL_SCANCODE_5, '5', 0x06, false}, {SDL_SCANCODE_6, '6', 0x07, false},
    {SDL_SCANCODE_7, '7', 0x08, false}, {SDL_SCANCODE_8, '8', 0x09, false}, {SDL_SCANCODE_9, '9', 0x0A, false},
    {SDL_SCANCODE_0, '0', 0x0B, false},
    {SDL_SCANCODE_RETURN, 0x0D, 0x1C, false}, {SDL_SCANCODE_ESCAPE, 0x1B, 0x01, false},
    {SDL_SCANCODE_BACKSPACE, 0x08, 0x0E, false}, {SDL_SCANCODE_TAB, 0x09, 0x0F, false},
    {SDL_SCANCODE_SPACE, 0x20, 0x39, false}, {SDL_SCANCODE_MINUS, 0xBD, 0x0C, false},
    {SDL_SCANCODE_EQUALS, 0xBB, 0x0D, false}, {SDL_SCANCODE_LEFTBRACKET, 0xDB, 0x1A, false},
    {SDL_SCANCODE_RIGHTBRACKET, 0xDD, 0x1B, false}, {SDL_SCANCODE_BACKSLASH, 0xDC, 0x2B, false},
    {SDL_SCANCODE_NONUSHASH, 0xDC, 0x2B, false}, {SDL_SCANCODE_SEMICOLON, 0xBA, 0x27, false},
    {SDL_SCANCODE_APOSTROPHE, 0xDE, 0x28, false}, {SDL_SCANCODE_GRAVE, 0xC0, 0x29, false},
    {SDL_SCANCODE_COMMA, 0xBC, 0x33, false}, {SDL_SCANCODE_PERIOD, 0xBE, 0x34, false},
    {SDL_SCANCODE_SLASH, 0xBF, 0x35, false}, {SDL_SCANCODE_CAPSLOCK, 0x14, 0x3A, false},
    {SDL_SCANCODE_F1, 0x70, 0x3B, false}, {SDL_SCANCODE_F2, 0x71, 0x3C, false}, {SDL_SCANCODE_F3, 0x72, 0x3D, false},
    {SDL_SCANCODE_F4, 0x73, 0x3E, false}, {SDL_SCANCODE_F5, 0x74, 0x3F, false}, {SDL_SCANCODE_F6, 0x75, 0x40, false},
    {SDL_SCANCODE_F7, 0x76, 0x41, false}, {SDL_SCANCODE_F8, 0x77, 0x42, false}, {SDL_SCANCODE_F9, 0x78, 0x43, false},
    {SDL_SCANCODE_F10, 0x79, 0x44, false}, {SDL_SCANCODE_F11, 0x7A, 0x57, false},
    {SDL_SCANCODE_F12, 0x7B, 0x58, false}, {SDL_SCANCODE_F13, 0x7C, 0x64, false},
    {SDL_SCANCODE_F14, 0x7D, 0x65, false}, {SDL_SCANCODE_F15, 0x7E, 0x66, false},
    {SDL_SCANCODE_F16, 0x7F, 0x67, false}, {SDL_SCANCODE_F17, 0x80, 0x68, false},
    {SDL_SCANCODE_F18, 0x81, 0x69, false}, {SDL_SCANCODE_F19, 0x82, 0x6A, false},
    {SDL_SCANCODE_PRINTSCREEN, 0x2C, 0x37, true}, {SDL_SCANCODE_SCROLLLOCK, 0x91, 0x46, false},
    {SDL_SCANCODE_PAUSE, 0x13, 0x45, false}, {SDL_SCANCODE_INSERT, 0x2D, 0x52, true},
    {SDL_SCANCODE_HOME, 0x24, 0x47, true}, {SDL_SCANCODE_PAGEUP, 0x21, 0x49, true},
    {SDL_SCANCODE_DELETE, 0x2E, 0x53, true}, {SDL_SCANCODE_END, 0x23, 0x4F, true},
    {SDL_SCANCODE_PAGEDOWN, 0x22, 0x51, true}, {SDL_SCANCODE_RIGHT, 0x27, 0x4D, true},
    {SDL_SCANCODE_LEFT, 0x25, 0x4B, true}, {SDL_SCANCODE_DOWN, 0x28, 0x50, true},
    {SDL_SCANCODE_UP, 0x26, 0x48, true},
    // The Mac keypad's Clear key sits where Num Lock does; Win32Mac gives it VK_NUMLOCK.
    {SDL_SCANCODE_NUMLOCKCLEAR, 0x90, 0x45, true},
    {SDL_SCANCODE_KP_DIVIDE, 0x6F, 0x35, true}, {SDL_SCANCODE_KP_MULTIPLY, 0x6A, 0x37, false},
    {SDL_SCANCODE_KP_MINUS, 0x6D, 0x4A, false}, {SDL_SCANCODE_KP_PLUS, 0x6B, 0x4E, false},
    {SDL_SCANCODE_KP_ENTER, 0x0D, 0x1C, true}, {SDL_SCANCODE_KP_1, 0x61, 0x4F, false},
    {SDL_SCANCODE_KP_2, 0x62, 0x50, false}, {SDL_SCANCODE_KP_3, 0x63, 0x51, false},
    {SDL_SCANCODE_KP_4, 0x64, 0x4B, false}, {SDL_SCANCODE_KP_5, 0x65, 0x4C, false},
    {SDL_SCANCODE_KP_6, 0x66, 0x4D, false}, {SDL_SCANCODE_KP_7, 0x67, 0x47, false},
    {SDL_SCANCODE_KP_8, 0x68, 0x48, false}, {SDL_SCANCODE_KP_9, 0x69, 0x49, false},
    {SDL_SCANCODE_KP_0, 0x60, 0x52, false}, {SDL_SCANCODE_KP_PERIOD, 0x6E, 0x53, false},
    {SDL_SCANCODE_KP_EQUALS, 0xBB, 0x59, false}, {SDL_SCANCODE_NONUSBACKSLASH, 0xE2, 0x56, false},
    {SDL_SCANCODE_APPLICATION, 0x5D, 0x5D, true},
    {SDL_SCANCODE_LCTRL, kVkControl, 0x1D, false}, {SDL_SCANCODE_RCTRL, kVkControl, 0x1D, true},
    {SDL_SCANCODE_LSHIFT, kVkShift, 0x2A, false}, {SDL_SCANCODE_RSHIFT, kVkShift, 0x36, false},
    {SDL_SCANCODE_LALT, kVkMenu, 0x38, false}, {SDL_SCANCODE_RALT, kVkMenu, 0x38, true},
    {SDL_SCANCODE_LGUI, kVkLWin, 0x5B, true}, {SDL_SCANCODE_RGUI, kVkRWin, 0x5C, true},
};

static const KeyRow* FindKey(SDL_Scancode nScancode) {
    for (const KeyRow& row : s_aKeys) {
        if (row.nScancode == nScancode) {
            return &row;
        }
    }
    return nullptr;
}

BYTE VirtualKeyFromSdl(SDL_Scancode nScancode, SDL_Keycode nKey) {
    if (nKey >= 'a' && nKey <= 'z') {
        return (BYTE)(nKey - 'a' + 'A');
    }
    const KeyRow* pRow = FindKey(nScancode);
    return pRow ? pRow->nVk : 0;
}

BYTE ScanCodeFromSdl(SDL_Scancode nScancode, bool* pbExtended) {
    const KeyRow* pRow = FindKey(nScancode);
    *pbExtended = pRow && pRow->bExtended;
    return pRow ? pRow->nScan : 0;
}

// The left or right form of a modifier, which GetKeyState also answers for.
static BYTE SideKey(SDL_Scancode nScancode) {
    switch (nScancode) {
    case SDL_SCANCODE_LSHIFT:
        return kVkLShift;
    case SDL_SCANCODE_RSHIFT:
        return kVkRShift;
    case SDL_SCANCODE_LCTRL:
        return kVkLControl;
    case SDL_SCANCODE_RCTRL:
        return kVkRControl;
    case SDL_SCANCODE_LALT:
        return kVkLMenu;
    case SDL_SCANCODE_RALT:
        return kVkRMenu;
    default:
        return 0;
    }
}

static bool AsyncDown(BYTE nVk) {
    return (G().abAsyncState[nVk] & 0x80) != 0;
}

void SetAsyncKey(BYTE nVk, bool bDown) {
    State& g = G();
    if (bDown) {
        if (!(g.abAsyncState[nVk] & 0x80)) {
            g.abAsyncPressed[nVk] = 1;
        }
        g.abAsyncState[nVk] |= 0x80;
    } else {
        g.abAsyncState[nVk] &= (BYTE)~0x80;
    }
}

// The left or right modifier a key message stands for, read back from its scan code and
// extended bit as Windows does: Shift by scan code, Ctrl and Alt by the extended bit.
static BYTE SideKeyFromLParam(BYTE nVk, LPARAM lParam) {
    BYTE nScan = (BYTE)((uint32_t)lParam >> 16);
    bool bExtended = ((uint32_t)lParam >> 24) & 1;
    switch (nVk) {
    case kVkShift:
        return nScan == 0x36 ? kVkRShift : kVkLShift;
    case kVkControl:
        return bExtended ? kVkRControl : kVkLControl;
    case kVkMenu:
        return bExtended ? kVkRMenu : kVkLMenu;
    default:
        return 0;
    }
}

static BYTE OtherSide(BYTE nSide) {
    return (BYTE)(((nSide - kVkLShift) ^ 1) + kVkLShift);
}

void OnSdlKey(const SDL_KeyboardEvent& tKey) {
    State& g = G();
    BYTE nVk = VirtualKeyFromSdl(tKey.scancode, tKey.key);
    if (!nVk) {
        return;
    }
    HWND hWnd = g.hFocus ? g.hFocus : WndFromSdlId(tKey.windowID);
    if (!hWnd) {
        hWnd = MainWindow();
    }
    bool bDown = tKey.down;
    BYTE nSide = SideKey(tKey.scancode);
    bool bWasDown = tKey.repeat || AsyncDown(nSide ? nSide : nVk);
    if (nSide) {
        SetAsyncKey(nSide, bDown);
        SetAsyncKey(nVk, bDown || AsyncDown(OtherSide(nSide)));
    } else {
        SetAsyncKey(nVk, bDown);
    }
    bool bAlt = AsyncDown(kVkMenu);
    bool bCtrl = AsyncDown(kVkControl);
    bool bSys = (bAlt && !bCtrl) || nVk == kVkF10 || (nVk == kVkMenu && !bCtrl);
    if (!bDown && nVk == kVkMenu) {
        bSys = !bCtrl;  // releasing Alt itself
    }
    bool bExtended;
    BYTE nScan = ScanCodeFromSdl(tKey.scancode, &bExtended);
    uint32_t nLParam = 1u | ((uint32_t)nScan << 16) | (bExtended ? 1u << 24 : 0u);
    if (bAlt) {
        nLParam |= 1u << 29;
    }
    if (bWasDown || !bDown) {
        nLParam |= 1u << 30;
    }
    if (!bDown) {
        nLParam |= 1u << 31;
    }
    UINT uMsg = bDown ? (bSys ? kWmSysKeyDown : kWmKeyDown) : (bSys ? kWmSysKeyUp : kWmKeyUp);
    if (bDown) {
        g.nLastKeyLParam = (LPARAM)nLParam;
    }
    Post(hWnd, uMsg, nVk, (LPARAM)nLParam);
}

void OnSdlText(const SDL_TextInputEvent& tText) {
    State& g = G();
    if (!tText.text || AsyncDown(kVkLWin) || AsyncDown(kVkRWin)) {
        return;
    }
    HWND hWnd = g.hFocus ? g.hFocus : WndFromSdlId(tText.windowID);
    if (!hWnd) {
        return;
    }
    char szAnsi[256];
    size_t nLen = Utf8ToAnsi(tText.text, szAnsi, sizeof(szAnsi));
    for (size_t i = 0; i < nLen; i++) {
        BYTE nChar = (BYTE)szAnsi[i];
        if (nChar >= 0x20 && nChar != 0x7F) {
            Post(hWnd, kWmChar, nChar, g.nLastKeyLParam ? g.nLastKeyLParam : 1);
        }
    }
}

// A key message taken off the queue updates GetKeyState's table.
void ApplyKeyMessage(const MSG& tMsg) {
    State& g = G();
    bool bDown;
    switch (tMsg.message) {
    case kWmKeyDown:
    case kWmSysKeyDown:
        bDown = true;
        break;
    case kWmKeyUp:
    case kWmSysKeyUp:
        bDown = false;
        break;
    default:
        return;
    }
    BYTE nVk = (BYTE)tMsg.wParam;
    BYTE nSide = SideKeyFromLParam(nVk, tMsg.lParam);
    auto set = [&](BYTE n, bool b) {
        if (b) {
            if (!(g.abKeyState[n] & 0x80)) {
                g.abKeyState[n] ^= 0x01;  // the toggle flips on each press
            }
            g.abKeyState[n] |= 0x80;
        } else {
            g.abKeyState[n] &= (BYTE)~0x80;
        }
    };
    if (nSide) {
        set(nSide, bDown);
        set(nVk, bDown || (g.abKeyState[OtherSide(nSide)] & 0x80));
    } else {
        set(nVk, bDown);
    }
}

void ResetKeys() {
    State& g = G();
    for (int i = 0; i < 256; i++) {
        if (i == kVkCapital || i == kVkNumLock || i == kVkScroll) {
            continue;
        }
        g.abKeyState[i] &= (BYTE)~0x80;
        g.abAsyncState[i] &= (BYTE)~0x80;
    }
}

WPARAM MouseKeyFlags() {
    State& g = G();
    WPARAM n = 0;
    if (g.abAsyncState[kVkLButton] & 0x80) n |= kMkLButton;
    if (g.abAsyncState[kVkRButton] & 0x80) n |= kMkRButton;
    if (g.abAsyncState[kVkMButton] & 0x80) n |= kMkMButton;
    if (g.abAsyncState[kVkXButton1] & 0x80) n |= kMkXButton1;
    if (g.abAsyncState[kVkXButton2] & 0x80) n |= kMkXButton2;
    if (g.abAsyncState[kVkShift] & 0x80) n |= kMkShift;
    if (g.abAsyncState[kVkControl] & 0x80) n |= kMkControl;
    return n;
}

}  // namespace Win32Posix

using namespace Win32Posix;

// The state as of the last message taken off the queue: bit 15 down, bit 0 toggled.
SHORT GetKeyState(int nVirtKey) {
    if (nVirtKey < 0 || nVirtKey > 0xFF) {
        return 0;
    }
    BYTE n = G().abKeyState[nVirtKey];
    return (SHORT)(((n & 0x80) ? 0xFF80 : 0) | (n & 0x01));
}

// The state now (as of the last pump): bit 15 down, bit 0 pressed since the last call.
SHORT GetAsyncKeyState(int vKey) {
    if (vKey < 0 || vKey > 0xFF) {
        return 0;
    }
    State& g = G();
    PumpSdl();
    SHORT nResult = (SHORT)(((g.abAsyncState[vKey] & 0x80) ? 0x8000 : 0) | (g.abAsyncPressed[vKey] ? 1 : 0));
    g.abAsyncPressed[vKey] = 0;
    return nResult;
}

// WM_KEYDOWN/WM_SYSKEYDOWN to WM_CHAR/WM_SYSCHAR for the keys that type a control character;
// the characters text input already gives are not repeated. The character goes to the front
// of the queue, so it is the next message, as on Windows.
BOOL TranslateMessage(const MSG* lpMsg) {
    if (!lpMsg || (lpMsg->message != kWmKeyDown && lpMsg->message != kWmSysKeyDown)) {
        return FALSE;
    }
    State& g = G();
    BYTE nVk = (BYTE)lpMsg->wParam;
    bool bCtrl = (g.abKeyState[kVkControl] & 0x80) != 0;
    bool bCmd = (g.abKeyState[kVkLWin] & 0x80) || (g.abKeyState[kVkRWin] & 0x80);
    BYTE nChar = 0;
    if (bCmd) {
        switch (nVk) {
        case 'X':
            nChar = 0x18;
            break;
        case 'C':
            nChar = 0x03;
            break;
        case 'V':
            nChar = 0x16;
            break;
        case 'A':
            nChar = 0x01;
            break;
        case kVkReturn:
            nChar = 0x0D;
            break;
        }
    } else if (bCtrl && nVk >= 'A' && nVk <= 'Z') {
        nChar = (BYTE)(nVk - 'A' + 1);
    } else if (bCtrl && nVk == 0xDB) {
        nChar = 0x1B;  // Ctrl+[
    } else {
        switch (nVk) {
        case kVkBack:
            nChar = 0x08;
            break;
        case kVkTab:
            nChar = 0x09;
            break;
        case kVkReturn:
            nChar = bCtrl ? 0x0A : 0x0D;
            break;
        case kVkEscape:
            nChar = 0x1B;
            break;
        }
    }
    if (!nChar) {
        return FALSE;
    }
    MSG tChar = *lpMsg;
    tChar.message = lpMsg->message == kWmSysKeyDown ? kWmSysChar : kWmChar;
    tChar.wParam = nChar;
    tChar.lParam = lpMsg->lParam;
    EnqueueFront(tChar);
    return TRUE;
}

// No IME context: the system's input method commits through text input, which arrives as
// WM_CHAR, and the composition stays in the system's own window.
HKL GetKeyboardLayout(DWORD idThread) {
    (void)idThread;
    return (HKL)(uintptr_t)0x04090409;  // en-US
}

HIMC ImmGetContext(HWND hWnd) {
    (void)hWnd;
    return nullptr;
}

BOOL ImmReleaseContext(HWND hWnd, HIMC hIMC) {
    (void)hWnd;
    (void)hIMC;
    return TRUE;
}

LONG ImmGetCompositionStringA(HIMC hIMC, DWORD dwIndex, LPVOID lpBuf, DWORD dwBufLen) {
    (void)hIMC;
    (void)dwIndex;
    (void)lpBuf;
    (void)dwBufLen;
    return 0;
}

DWORD ImmGetCandidateListA(HIMC hIMC, DWORD deIndex, LPCANDIDATELIST lpCandList, DWORD dwBufLen) {
    (void)hIMC;
    (void)deIndex;
    (void)lpCandList;
    (void)dwBufLen;
    return 0;
}

DWORD ImmGetCandidateListCountA(HIMC hIMC, LPDWORD lpdwListCount) {
    (void)hIMC;
    if (lpdwListCount) {
        *lpdwListCount = 0;
    }
    return 0;
}

BOOL ImmGetOpenStatus(HIMC hIMC) {
    (void)hIMC;
    return FALSE;
}

BOOL ImmSetOpenStatus(HIMC hIMC, BOOL fOpen) {
    (void)hIMC;
    (void)fOpen;
    return FALSE;
}

BOOL ImmGetConversionStatus(HIMC hIMC, LPDWORD lpfdwConversion, LPDWORD lpfdwSentence) {
    (void)hIMC;
    if (lpfdwConversion) {
        *lpfdwConversion = 0;
    }
    if (lpfdwSentence) {
        *lpfdwSentence = 0;
    }
    return FALSE;
}

BOOL ImmSetConversionStatus(HIMC hIMC, DWORD fdwConversion, DWORD fdwSentence) {
    (void)hIMC;
    (void)fdwConversion;
    (void)fdwSentence;
    return FALSE;
}

BOOL ImmSimulateHotKey(HWND hWnd, DWORD dwHotKeyID) {
    (void)hWnd;
    (void)dwHotKeyID;
    return FALSE;
}

BOOL ImmIsIME(HKL hKL) {
    (void)hKL;
    return FALSE;
}

#endif  // !defined(_WIN32) && D2_BACKEND_POSIX
