// Win32Posix: the device context handles and the GDI objects the game asks for (Win32Mac:
// Win32_GDI.cpp). Nothing is drawn through GDI: an HDC names its window (or the screen) and
// the renderer draws through SDL. The stock objects and the images are handles only.
#include "Source/StormMac/Win32Posix/UserInternal.h"

#if !defined(_WIN32) && D2_BACKEND_POSIX

using namespace Win32Posix;

// GetDC(NULL) is the screen's DC.
HDC GetDC(HWND hWnd) {
    if (!hWnd) {
        return (HDC)MakeHandle(kHdcTag, kDesktopSlot);
    }
    if (!WndFromHandle(hWnd)) {
        return nullptr;
    }
    return (HDC)MakeHandle(kHdcTag, HandleSlot(hWnd));
}

int ReleaseDC(HWND hWnd, HDC hDC) {
    (void)hWnd;
    return HandleHasTag(hDC, kHdcTag) ? 1 : 0;
}

// WHITE_BRUSH 0 ... DEFAULT_GUI_FONT 17, DC_BRUSH 18, DC_PEN 19.
HGDIOBJ GetStockObject(int i) {
    if (i < 0 || i > 19) {
        return nullptr;
    }
    return (HGDIOBJ)MakeHandle(kHstockTag, (uint32_t)i + 1);
}

DWORD GdiSetBatchLimit(DWORD dw) {
    static DWORD s_dwLimit = 20;
    DWORD dwOld = s_dwLimit;
    s_dwLimit = dw ? dw : 20;
    return dwOld;
}

HICON LoadIconA(HINSTANCE hInstance, LPCSTR lpIconName) {
    (void)hInstance;
    uint32_t nId = (uintptr_t)lpIconName < 0x10000 ? (uint32_t)(uintptr_t)lpIconName : 32512;
    return (HICON)MakeHandle(kHiconTag, nId);
}

// type: IMAGE_BITMAP 0, IMAGE_ICON 1, IMAGE_CURSOR 2. The bundle carries the application icon,
// so an icon resource is a handle the window code can pass around and nothing more.
HANDLE LoadImageA(HINSTANCE hInst, LPCSTR name, UINT type, int cx, int cy, UINT fuLoad) {
    (void)hInst;
    (void)cx;
    (void)cy;
    (void)fuLoad;
    uint32_t nId = (uintptr_t)name < 0x10000 ? (uint32_t)(uintptr_t)name : 0;
    switch (type) {
    case 1:
        return (HANDLE)MakeHandle(kHiconTag, nId);
    case 2:
        return (HANDLE)MakeHandle(kHcursorTag, 32512);
    default:
        return (HANDLE)MakeHandle(kHimageTag, nId);
    }
}

#endif  // !defined(_WIN32) && D2_BACKEND_POSIX
