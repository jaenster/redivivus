// Win32Posix: the clipboard on SDL3. CF_TEXT only, in Windows-1252 on the game's side and
// UTF-8 on SDL's. SetClipboardData copies the text out at once: the game frees its own
// handle afterwards (D2WINEDITBOX_SetClipboardText). GetClipboardData hands out a handle the
// clipboard owns, as Windows does; it is freed when the clipboard is emptied or read again.
#include "Source/StormMac/Win32Posix/UserInternal.h"

#if !defined(_WIN32) && D2_BACKEND_POSIX

#include <string.h>

using namespace Win32Posix;

static void DropClipboardData() {
    State& g = G();
    if (g.hClipboardData) {
        GlobalFree(g.hClipboardData);
        g.hClipboardData = nullptr;
    }
}

BOOL OpenClipboard(HWND hWndNewOwner) {
    (void)hWndNewOwner;
    State& g = G();
    if (g.bClipboardOpen || !EnsureSdl()) {
        SetLastError(5);  // ERROR_ACCESS_DENIED
        return FALSE;
    }
    g.bClipboardOpen = true;
    return TRUE;
}

BOOL CloseClipboard(void) {
    State& g = G();
    if (!g.bClipboardOpen) {
        SetLastError(1418);  // ERROR_CLIPBOARD_NOT_OPEN
        return FALSE;
    }
    g.bClipboardOpen = false;
    return TRUE;
}

BOOL EmptyClipboard(void) {
    State& g = G();
    if (!g.bClipboardOpen) {
        SetLastError(1418);  // ERROR_CLIPBOARD_NOT_OPEN
        return FALSE;
    }
    DropClipboardData();
    SDL_ClearClipboardData();
    return TRUE;
}

BOOL IsClipboardFormatAvailable(UINT format) {
    return format == CF_TEXT && EnsureSdl() && SDL_HasClipboardText();
}

HANDLE GetClipboardData(UINT uFormat) {
    State& g = G();
    if (!g.bClipboardOpen || uFormat != CF_TEXT) {
        return nullptr;
    }
    char* szUtf8 = SDL_GetClipboardText();
    if (!szUtf8) {
        return nullptr;
    }
    size_t nSize = strlen(szUtf8) + 1;
    DropClipboardData();
    HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, nSize);
    char* szAnsi = hMem ? (char*)GlobalLock(hMem) : nullptr;
    if (szAnsi) {
        Utf8ToAnsi(szUtf8, szAnsi, nSize);
        GlobalUnlock(hMem);
        g.hClipboardData = hMem;
    } else if (hMem) {
        GlobalFree(hMem);
        hMem = nullptr;
    }
    SDL_free(szUtf8);
    return hMem;
}

HANDLE SetClipboardData(UINT uFormat, HANDLE hMem) {
    State& g = G();
    if (!g.bClipboardOpen || uFormat != CF_TEXT || !hMem) {
        return nullptr;
    }
    const char* szAnsi = (const char*)GlobalLock(hMem);
    if (!szAnsi) {
        return nullptr;
    }
    size_t nSize = strlen(szAnsi) * 3 + 1;
    char* szUtf8 = (char*)SDL_malloc(nSize);
    if (!szUtf8) {
        GlobalUnlock(hMem);
        return nullptr;
    }
    AnsiToUtf8(szAnsi, szUtf8, nSize);
    GlobalUnlock(hMem);
    bool bSet = SDL_SetClipboardText(szUtf8);
    SDL_free(szUtf8);
    return bSet ? hMem : nullptr;
}

#endif  // !defined(_WIN32) && D2_BACKEND_POSIX
