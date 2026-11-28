// Win32Posix: the display, the system metrics and the message box on SDL3.
//
// The screen is the primary display's desktop mode, in points. ChangeDisplaySettingsA never
// switches the hardware mode: it records the mode, which GetSystemMetrics and
// EnumDisplaySettingsA report from then on, and a popup window over that whole screen becomes
// a borderless desktop-fullscreen window whose client is scaled to fit (UserWindow.cpp).
// ChangeDisplaySettingsA(NULL, 0) puts the desktop mode back. Every mode is accepted, since any
// size scales; the list EnumDisplaySettingsA gives is SDL's modes plus 640x480, 800x600 and
// 1024x768.
#include "Source/StormMac/Win32Posix/UserInternal.h"

#if !defined(_WIN32) && D2_BACKEND_POSIX

#include <stdio.h>
#include <string.h>

// The record's layout (winuser.h), as Source/D2Win/Src/D2WinMain.h declares it for the game.
struct tagMONITORINFO {
    /* 0x00 */ DWORD cbSize;
    /* 0x04 */ RECT rcMonitor;
    /* 0x14 */ RECT rcWork;
    /* 0x24 */ DWORD dwFlags;
};

namespace Win32Posix {

static void DesktopSize(int* pnWidth, int* pnHeight, int* pnRefresh) {
    *pnWidth = 800;
    *pnHeight = 600;
    *pnRefresh = 60;
    if (!EnsureSdl()) {
        return;
    }
    const SDL_DisplayMode* pMode = SDL_GetDesktopDisplayMode(SDL_GetPrimaryDisplay());
    if (pMode && pMode->w > 0 && pMode->h > 0) {
        *pnWidth = pMode->w;
        *pnHeight = pMode->h;
        *pnRefresh = pMode->refresh_rate > 0.0f ? (int)(pMode->refresh_rate + 0.5f) : 60;
    }
}

void ScreenSize(int* pnWidth, int* pnHeight) {
    State& g = G();
    if (g.bModeChanged) {
        *pnWidth = g.nModeWidth;
        *pnHeight = g.nModeHeight;
        return;
    }
    int nRefresh;
    DesktopSize(pnWidth, pnHeight, &nRefresh);
}

void WorkArea(RECT* pRect) {
    int nWidth, nHeight;
    ScreenSize(&nWidth, &nHeight);
    *pRect = {0, 0, nWidth, nHeight};
    SDL_Rect tUsable;
    if (!G().bModeChanged && EnsureSdl() && SDL_GetDisplayUsableBounds(SDL_GetPrimaryDisplay(), &tUsable)) {
        SDL_Rect tBounds;
        int nX = 0, nY = 0;
        if (SDL_GetDisplayBounds(SDL_GetPrimaryDisplay(), &tBounds)) {
            nX = tBounds.x;
            nY = tBounds.y;
        }
        *pRect = {tUsable.x - nX, tUsable.y - nY, tUsable.x - nX + tUsable.w, tUsable.y - nY + tUsable.h};
    }
}

}  // namespace Win32Posix

using namespace Win32Posix;

int GetSystemMetrics(int nIndex) {
    int nWidth, nHeight;
    switch (nIndex) {
    case 0:  // SM_CXSCREEN
    case 78:  // SM_CXVIRTUALSCREEN
        ScreenSize(&nWidth, &nHeight);
        return nWidth;
    case 1:  // SM_CYSCREEN
    case 79:  // SM_CYVIRTUALSCREEN
        ScreenSize(&nWidth, &nHeight);
        return nHeight;
    case 16: {  // SM_CXFULLSCREEN
        RECT rc;
        WorkArea(&rc);
        return rc.right - rc.left;
    }
    case 17: {  // SM_CYFULLSCREEN
        RECT rc;
        WorkArea(&rc);
        return rc.bottom - rc.top;
    }
    case 2:  // SM_CXVSCROLL
    case 3:  // SM_CYHSCROLL
        return 16;
    case 11:  // SM_CXICON
    case 12:  // SM_CYICON
    case 13:  // SM_CXCURSOR
    case 14:  // SM_CYCURSOR
        return 32;
    case 19:  // SM_MOUSEPRESENT
    case 75:  // SM_MOUSEWHEELPRESENT
        return 1;
    case 36:  // SM_CXDOUBLECLK
    case 37:  // SM_CYDOUBLECLK
        return 4;
    case 43:  // SM_CMOUSEBUTTONS
        return 5;
    case 80: {  // SM_CMONITORS
        int nCount = 1;
        if (EnsureSdl()) {
            SDL_DisplayID* pDisplays = SDL_GetDisplays(&nCount);
            SDL_free(pDisplays);
        }
        return nCount > 0 ? nCount : 1;
    }
    // SM_CYCAPTION 4, SM_CXBORDER 5, SM_CYBORDER 6, SM_CXDLGFRAME 7, SM_CYDLGFRAME 8,
    // SM_CYMENU 15, SM_CXFRAME 32, SM_CYFRAME 33: no frame (AdjustWindowRectEx).
    default:
        return 0;
    }
}

// dmFields: DM_BITSPERPEL 0x40000, DM_PELSWIDTH 0x80000, DM_PELSHEIGHT 0x100000,
// DM_DISPLAYFREQUENCY 0x400000.
static void FillMode(DEVMODEA* pMode, int nWidth, int nHeight, int nRefresh) {
    WORD wDriverExtra = pMode->dmDriverExtra;
    memset(pMode, 0, sizeof(*pMode));
    pMode->dmSize = sizeof(*pMode);
    pMode->dmDriverExtra = wDriverExtra;
    pMode->dmSpecVersion = 0x0401;
    pMode->dmFields = 0x40000 | 0x80000 | 0x100000 | 0x400000;
    pMode->dmBitsPerPel = 32;
    pMode->dmPelsWidth = (DWORD)nWidth;
    pMode->dmPelsHeight = (DWORD)nHeight;
    pMode->dmDisplayFrequency = (DWORD)nRefresh;
}

// iModeNum: ENUM_CURRENT_SETTINGS (DWORD)-1, ENUM_REGISTRY_SETTINGS (DWORD)-2, else an index.
BOOL EnumDisplaySettingsA(LPCSTR lpszDeviceName, DWORD iModeNum, DEVMODEA* lpDevMode) {
    (void)lpszDeviceName;
    if (!lpDevMode) {
        return FALSE;
    }
    State& g = G();
    int nWidth, nHeight, nRefresh;
    DesktopSize(&nWidth, &nHeight, &nRefresh);
    if (iModeNum == 0xFFFFFFFFu) {
        if (g.bModeChanged) {
            FillMode(lpDevMode, g.nModeWidth, g.nModeHeight, g.nModeRefresh);
        } else {
            FillMode(lpDevMode, nWidth, nHeight, nRefresh);
        }
        return TRUE;
    }
    if (iModeNum == 0xFFFFFFFEu) {
        FillMode(lpDevMode, nWidth, nHeight, nRefresh);
        return TRUE;
    }
    struct Mode {
        int nWidth, nHeight, nRefresh;
    };
    Mode aModes[256];
    uint32_t nModes = 0;
    auto add = [&](int w, int h, int r) {
        for (uint32_t i = 0; i < nModes; i++) {
            if (aModes[i].nWidth == w && aModes[i].nHeight == h && aModes[i].nRefresh == r) {
                return;
            }
        }
        if (nModes < SDL_arraysize(aModes)) {
            aModes[nModes++] = {w, h, r};
        }
    };
    add(640, 480, nRefresh);
    add(800, 600, nRefresh);
    add(1024, 768, nRefresh);
    if (EnsureSdl()) {
        int nCount = 0;
        SDL_DisplayMode** ppModes = SDL_GetFullscreenDisplayModes(SDL_GetPrimaryDisplay(), &nCount);
        for (int i = 0; ppModes && i < nCount; i++) {
            int nModeRefresh = ppModes[i]->refresh_rate > 0.0f ? (int)(ppModes[i]->refresh_rate + 0.5f) : nRefresh;
            add(ppModes[i]->w, ppModes[i]->h, nModeRefresh);
        }
        SDL_free(ppModes);
    }
    add(nWidth, nHeight, nRefresh);
    if (iModeNum >= nModes) {
        return FALSE;
    }
    const Mode& m = aModes[iModeNum];
    FillMode(lpDevMode, m.nWidth, m.nHeight, m.nRefresh);
    return TRUE;
}

// dwFlags: CDS_TEST 2, CDS_FULLSCREEN 4. DISP_CHANGE_SUCCESSFUL 0, DISP_CHANGE_BADMODE -2.
LONG ChangeDisplaySettingsA(DEVMODEA* lpDevMode, DWORD dwFlags) {
    State& g = G();
    if (!lpDevMode) {
        if (g.bModeChanged) {
            g.bModeChanged = false;
            UpdateAllFullscreen();
        }
        return 0;
    }
    int nWidth, nHeight, nRefresh;
    DesktopSize(&nWidth, &nHeight, &nRefresh);
    if (lpDevMode->dmFields & 0x80000) {
        nWidth = (int)lpDevMode->dmPelsWidth;
    } else if (g.bModeChanged) {
        nWidth = g.nModeWidth;
    }
    if (lpDevMode->dmFields & 0x100000) {
        nHeight = (int)lpDevMode->dmPelsHeight;
    } else if (g.bModeChanged) {
        nHeight = g.nModeHeight;
    }
    if (lpDevMode->dmFields & 0x400000) {
        nRefresh = (int)lpDevMode->dmDisplayFrequency;
    }
    if (nWidth <= 0 || nHeight <= 0) {
        return -2;
    }
    if (dwFlags & 2) {
        return 0;
    }
    g.bModeChanged = true;
    g.nModeWidth = nWidth;
    g.nModeHeight = nHeight;
    g.nModeRefresh = nRefresh;
    UpdateAllFullscreen();
    return 0;
}

HMONITOR MonitorFromWindow(HWND hwnd, DWORD dwFlags) {
    (void)hwnd;
    (void)dwFlags;
    return (HMONITOR)MakeHandle(kHmonitorTag, 1);
}

// MONITORINFOF_PRIMARY 1.
BOOL GetMonitorInfoA(HMONITOR hMonitor, struct tagMONITORINFO* lpmi) {
    if (!HandleHasTag(hMonitor, kHmonitorTag) || !lpmi) {
        return FALSE;
    }
    int nWidth, nHeight;
    ScreenSize(&nWidth, &nHeight);
    lpmi->rcMonitor = {0, 0, nWidth, nHeight};
    WorkArea(&lpmi->rcWork);
    lpmi->dwFlags = 1;
    return TRUE;
}

// SPI_GETSCREENSAVEACTIVE 0x10, SPI_SETSCREENSAVEACTIVE 0x11, SPI_GETWORKAREA 0x30.
BOOL SystemParametersInfoA(UINT uiAction, UINT uiParam, PVOID pvParam, UINT fWinIni) {
    (void)fWinIni;
    switch (uiAction) {
    case 0x30:
        if (!pvParam) {
            return FALSE;
        }
        WorkArea((RECT*)pvParam);
        return TRUE;
    case 0x10:
        if (!pvParam) {
            return FALSE;
        }
        *(BOOL*)pvParam = EnsureSdl() ? SDL_ScreenSaverEnabled() : TRUE;
        return TRUE;
    case 0x11:
        if (EnsureSdl()) {
            if (uiParam) {
                SDL_EnableScreenSaver();
            } else {
                SDL_DisableScreenSaver();
            }
        }
        return TRUE;
    default:
        return FALSE;
    }
}

// uType: the buttons in the low nibble (MB_OK 0, MB_OKCANCEL 1, MB_ABORTRETRYIGNORE 2,
// MB_YESNOCANCEL 3, MB_YESNO 4, MB_RETRYCANCEL 5), the icon in the next (MB_ICONHAND 0x10,
// MB_ICONQUESTION 0x20, MB_ICONEXCLAMATION 0x30, MB_ICONASTERISK 0x40), the default button
// in the next (MB_DEFBUTTON2 0x100, MB_DEFBUTTON3 0x200). Returns IDOK 1, IDCANCEL 2,
// IDABORT 3, IDRETRY 4, IDIGNORE 5, IDYES 6 or IDNO 7. With no display to show it on (the
// dummy and offscreen drivers) the text goes to stderr and the default button is taken.
extern "C" int MessageBoxA(HWND hWnd, LPCSTR lpText, LPCSTR lpCaption, UINT uType) {
    static const int s_aButtonSets[6][3] = {
        {1, 0, 0}, {1, 2, 0}, {3, 4, 5}, {6, 7, 2}, {6, 7, 0}, {4, 2, 0},
    };
    static const char* const s_aszNames[8] = {"", "OK", "Cancel", "Abort", "Retry", "Ignore", "Yes", "No"};
    UINT nSet = uType & 0xF;
    if (nSet > 5) {
        nSet = 0;
    }
    int nButtons = 0;
    while (nButtons < 3 && s_aButtonSets[nSet][nButtons]) {
        nButtons++;
    }
    int nDefault = (int)((uType >> 8) & 0xF);
    if (nDefault >= nButtons) {
        nDefault = 0;
    }
    int nDefaultId = s_aButtonSets[nSet][nDefault];

    char szText[2048], szCaption[512];
    AnsiToUtf8(lpText ? lpText : "", szText, sizeof(szText));
    AnsiToUtf8(lpCaption ? lpCaption : "Error", szCaption, sizeof(szCaption));
    const char* szDriver = G().bSdlReady ? SDL_GetCurrentVideoDriver() : SDL_GetHint(SDL_HINT_VIDEO_DRIVER);
    if (szDriver && (strcmp(szDriver, "dummy") == 0 || strcmp(szDriver, "offscreen") == 0)) {
        fprintf(stderr, "MessageBox [%s]: %s -> %s\n", szCaption, szText, s_aszNames[nDefaultId]);
        return nDefaultId;
    }

    SDL_MessageBoxButtonData aButtons[3];
    for (int i = 0; i < nButtons; i++) {
        int nId = s_aButtonSets[nSet][i];
        aButtons[i].flags = 0;
        if (i == nDefault) {
            aButtons[i].flags |= SDL_MESSAGEBOX_BUTTON_RETURNKEY_DEFAULT;
        }
        if (nId == 2 || (nButtons == 1 && nId == 1)) {
            aButtons[i].flags |= SDL_MESSAGEBOX_BUTTON_ESCAPEKEY_DEFAULT;
        }
        aButtons[i].buttonID = nId;
        aButtons[i].text = s_aszNames[nId];
    }
    SDL_MessageBoxData tBox;
    SDL_zero(tBox);
    UINT nIcon = uType & 0xF0;
    tBox.flags = nIcon == 0x10 ? SDL_MESSAGEBOX_ERROR : nIcon == 0x30 ? SDL_MESSAGEBOX_WARNING : SDL_MESSAGEBOX_INFORMATION;
    tBox.flags |= SDL_MESSAGEBOX_BUTTONS_LEFT_TO_RIGHT;
    tBox.window = Win32Posix_GetSdlWindow(hWnd);
    tBox.title = szCaption;
    tBox.message = szText;
    tBox.numbuttons = nButtons;
    tBox.buttons = aButtons;
    int nPressed = -1;
    if (!SDL_ShowMessageBox(&tBox, &nPressed) || nPressed < 0) {
        fprintf(stderr, "MessageBox [%s]: %s (%s)\n", szCaption, szText, SDL_GetError());
        return nDefaultId;
    }
    return nPressed;
}

#endif  // !defined(_WIN32) && D2_BACKEND_POSIX
