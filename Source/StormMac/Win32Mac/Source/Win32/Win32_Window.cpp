// Storm's Win32-on-Mac layer: window classes, window lifetime, focus and activation,
// 1.14d Intel Mac band 0002eae0..00031a0a (the window manager; the same unit as
// Win32_WindowPaint.cpp, split for size). See Source/StormMac/README.md for how the
// file boundary was pinned.
//
// Windows are heap objects (Win32MacWindow) in a tree under one desktop window. A
// registered class lives in one of 16 table slots.
#include "Win32MacWindow.h"

#if !defined(_WIN32) && D2_OS_MAC

#include <string.h>

#define D2_NOINLINE __attribute__((noinline))

// Carbon's Script Manager and KeyTranslate, declared here because Carbon.h's own
// FillRect, PtInRect, ShowWindow ... clash with the Win32 entry points this file defines.
extern "C" long GetScriptManagerVariable(short nSelector);
extern "C" uint32_t KeyTranslate(const void* pKCHR, uint16_t nKeyCode, uint32_t* pState);
#define smKCHRCache 0x26

#define CW_USEDEFAULT ((int)0x80000000)
#define CS_NOCLOSE 0x0200
#define SC_CLOSE 0xF060
#define SWP_NOSIZE 0x01
#define SWP_NOMOVE 0x02
#define SWP_NOZORDER 0x04
#define SWP_NOACTIVATE 0x10
#define WM_MOVE 0x0003
#define WM_SIZE 0x0005
#define WM_ACTIVATE 0x0006
#define WM_ACTIVATEAPP 0x001C
#define WM_NCACTIVATE 0x0086
#define WM_QUERYNEWPALETTE 0x030F
#define WM_HELP 0x0053
#define WM_ENABLE 0x000A
#define WM_DRAWITEM 0x002B
#define BM_SETCHECK 0x00F1
#define BM_GETSTATE 0x00F2
#define BM_SETSTATE 0x00F3
#define BM_SETSTYLE 0x00F4
#define BM_CLICK 0x00F5
#define WM_GETDLGCODE 0x0087
#define RGN_DIFF 4
#define NULLREGION 1
#define SM_CXSCREEN 0
#define SM_CYSCREEN 1
#define SYSTEM_FONT 13

#define WINDOW_CLASS_COUNT 16

// 1.14d mac: 003979c8
// The next class atom to hand out.
static ATOM s_nNextAtom = 1;
// 1.14d mac: 003f2504
HWND g_hDesktopWnd;
// 1.14d mac: 003f2508
// The scratch region Wnd_CalculateVisibleRgn subtracts through.
static HRGN s_hScratchRgn;
// 1.14d mac: 003f250c
static Win32MacWindowClass s_aWindowClasses[WINDOW_CLASS_COUNT];
// 1.14d mac: 003f480c
// Set once the first window has been told WM_ACTIVATEAPP.
static BYTE s_bAppActivated;

// What Wnd_CalculateVisibleRgn hands to Wnd_Iterate_RemoveWindowFromVisRgn.
struct Win32MacVisRgnContext {
    HRGN hRgn;       // 0x00  the region being reduced
    int nOriginX;    // 0x04  screen position of the window it is for
    int nOriginY;    // 0x08
    HRGN hScratch;   // 0x0C
};

// 1.14d mac: 00322520
// Whether each SW_* command leaves the window shown.
static const int s_anShowVisible[10] = {0, 1, 0, 0, 0, 1, 0, 0, 0, 1};
// 1.14d mac: 00322550
// The SetWindowPos flags each SW_* command stands for.
static const int s_anShowFlags[10] = {0x97, 0x47, 0, 0, 0, 0x47, 0, 0, 0, 0x47};
// 1.14d mac: 003224f0
// WM_GETDLGCODE for each button style.
static const int s_anButtonDlgCode[10] = {0x2020, 0x2010, 0x2000, 0x2000, 0x2040,
                                          0x2000, 0x2000, 0x2000, 0x2000, 0x2040};

// 1.14d mac: 00322578
// The virtual key of the Mac key codes below 0x80 the layout gives no letter.
static const BYTE s_abyMacKeyToVirtualKey[0x80] = {
    0x41, 0x53, 0x44, 0x46, 0x48, 0x47, 0x5a, 0x58, 0x43, 0x56, 0x0,  0x42, 0x51, 0x57, 0x45, 0x52,
    0x59, 0x54, 0x31, 0x32, 0x33, 0x34, 0x36, 0x35, 0xbb, 0x39, 0x37, 0xbd, 0x38, 0x30, 0xdd, 0x4f,
    0x55, 0xdb, 0x49, 0x50, 0xd,  0x4c, 0x4a, 0xde, 0x4b, 0xba, 0xdc, 0xbc, 0xbf, 0x4e, 0x4d, 0xbe,
    0x9,  0x20, 0xc0, 0x8,  0x0,  0x1b, 0x0,  0x5b, 0x10, 0x14, 0x12, 0x11, 0xa1, 0xa5, 0xa3, 0x0,
    0x0,  0x6e, 0x0,  0x6a, 0x0,  0x6b, 0x0,  0x90, 0x0,  0x0,  0x0,  0x6f, 0xd,  0x0,  0x6d, 0x0,
    0x0,  0xdf, 0x60, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x0,  0x68, 0x69, 0x0,  0x0,  0x0,
    0x74, 0x75, 0x76, 0x72, 0x77, 0x78, 0x0,  0x7a, 0x0,  0x2c, 0x0,  0x91, 0x0,  0x79, 0x0,  0x7b,
    0x0,  0x13, 0x2d, 0x24, 0x21, 0x2e, 0x73, 0x23, 0x71, 0x22, 0x70, 0x25, 0x27, 0x28, 0x26, 0x0,
};
// 1.14d mac: 003225f8
// The Mac key code of each Win32 virtual key, 0xFF for none.
static const BYTE s_abyVirtualKeyToMacKey[0x100] = {
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x33, 0x30, 0xff, 0xff, 0x47, 0x24, 0xff, 0xff,
    0x38, 0x3b, 0x3a, 0x71, 0x39, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x35, 0xff, 0xff, 0xff, 0xff,
    0x31, 0x74, 0x79, 0x77, 0x73, 0x7b, 0x7e, 0x7c, 0x7d, 0xff, 0xff, 0xff, 0x69, 0x72, 0x75, 0x72,
    0x1d, 0x12, 0x13, 0x14, 0x15, 0x17, 0x16, 0x1a, 0x1c, 0x19, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0x0, 0xb, 0x8, 0x2, 0xe, 0x3, 0x5, 0x4, 0x22, 0x26, 0x28, 0x25, 0x2e, 0x2d, 0x1f,
    0x23, 0xc, 0xf, 0x1, 0x11, 0x20, 0x9, 0xd, 0x7, 0x10, 0x6, 0x37, 0xff, 0xff, 0xff, 0xff,
    0x52, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59, 0x5b, 0x5c, 0x43, 0x45, 0xff, 0x4e, 0x41, 0x4b,
    0x7a, 0x78, 0x63, 0x76, 0x60, 0x61, 0x62, 0x64, 0x65, 0x6d, 0x67, 0x6f, 0x69, 0x6b, 0x71, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0x47, 0x6b, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0x38, 0x3c, 0x3b, 0x3e, 0x3a, 0x3d, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x29, 0x18, 0x2b, 0x1b, 0x2f, 0x2c,
    0x32, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x21, 0x2a, 0x1e, 0x27, 0x51,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
};

// 1.14d mac: 0002ef30
D2_NOINLINE LRESULT Proc_DesktopWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    if (uMsg != WM_PAINT && uMsg != WM_ERASEBKGND) {
        return 0;
    }
    return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

// 1.14d mac: 0002eae0
// What a window does with the messages nothing else claims.
D2_NOINLINE LRESULT DefWindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        case WM_CREATE:
            return 1;
        case WM_ACTIVATE:
            if ((wParam & 0xFFFF) - 1 < 2) {
                SetFocus(hWnd);
            }
            return 0;
        case WM_ENABLE:
            InvalidateRect(hWnd, 0, 1);
            break;
        case WM_SETTEXT: {
            if (hWnd == 0) {
                return 0;
            }
            hWnd->cMnemonic = 0;
            if (hWnd->szText != 0) {
                Priv_Free(hWnd->szText);
                hWnd->szText = 0;
            }
            const char* szText = (const char*)lParam;
            if (szText != 0 && *szText != 0) {
                hWnd->szText = (char*)Priv_Alloc(strlen(szText) + 1);
                strcpy(hWnd->szText, szText);
                hWnd->cMnemonic = 0;
                const char* pAmp = strchr(hWnd->szText, '&');
                if (pAmp != 0 && pAmp[1] != 0 && pAmp[1] != '&') {
                    hWnd->cMnemonic = pAmp[1];
                }
            }
            return 1;
        }
        case WM_GETTEXT: {
            char* szBuf = (char*)lParam;
            if (szBuf != 0 && (int)wParam > 0) {
                *szBuf = 0;
                if (hWnd != 0 && hWnd->szText != 0 && *hWnd->szText != 0) {
                    strncpy(szBuf, hWnd->szText, wParam - 1);
                    szBuf[wParam - 1] = 0;
                    return strlen(szBuf) + 1;
                }
            }
            return 0;
        }
        case WM_GETTEXTLENGTH:
            if (hWnd != 0 && hWnd->szText != 0 && *hWnd->szText != 0) {
                return strlen(hWnd->szText);
            }
            return 0;
        case WM_PAINT: {
            PAINTSTRUCT tPaint;
            BeginPaint(hWnd, &tPaint);
            EndPaint(hWnd, &tPaint);
            break;
        }
        case WM_CLOSE:
            DestroyWindow(hWnd);
            break;
        case WM_ERASEBKGND:
            if (hWnd != 0) {
                RECT rcClient;
                GetClientRect(hWnd, &rcClient);
                FillRect((HDC)wParam, &rcClient);
            }
            return 1;
        case WM_SHOWWINDOW:
            if (lParam != 0 && (((hWnd->dwStyle & WS_VISIBLE) != 0) == (wParam == 0))) {
                ShowWindow(hWnd, wParam != 0 ? 5 : 0);
            }
            break;
        case WM_SETFONT:
            if (hWnd == 0) {
                return 0;
            }
            if (wParam == 0) {
                wParam = (int)GetStockObject(SYSTEM_FONT);
            }
            hWnd->hFont = wParam;
            return 0;
        case WM_GETFONT:
            if (hWnd == 0) {
                return 0;
            }
            {
                int hFont = hWnd->hFont;
                return hFont != (int)GetStockObject(SYSTEM_FONT) ? hFont : 0;
            }
        case WM_NCCREATE:
            return 1;
        case WM_NCACTIVATE:
            return 1;
        case WM_KEYDOWN:
            if (wParam == 0x2D || wParam == 0x70) {
                HWND hActive = GetActiveWindow();
                if (hActive != 0) {
                    SendMessage(hActive, WM_HELP, 0, 0);
                }
                return 0;
            }
            break;
        case WM_SYSKEYDOWN:
            if (wParam == 0x73) {
                HWND hTop = Wnd_GetTopParent(hWnd);
                if ((hTop->pClass->style & CS_NOCLOSE) == 0) {
                    PostMessage(hTop, WM_SYSCOMMAND, SC_CLOSE, 0);
                }
            }
            break;
        case WM_SYSCOMMAND:
            if ((wParam & 0xFFF0) == SC_CLOSE) {
                return SendMessage(hWnd, WM_CLOSE, 0, 0);
            }
            break;
    }
    return 0;
}

// 1.14d mac: 0002ef4a
D2_NOINLINE LRESULT Proc_ButtonWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    DWORD dwStyle = hWnd->dwStyle;
    LRESULT lResult = 0;
    WPARAM wNotify;
    HWND hParent;

    if (uMsg < 0x200) {
        if (uMsg >= 0xF0) {
            switch (uMsg) {
                case 0xF0:
                case 0xF1:
                    return 0;
                case BM_GETSTATE:
                    return GetProp(hWnd, "__ButtonIsPressed") != 0;
                case BM_SETSTATE: {
                    int bPressed = GetProp(hWnd, "__ButtonIsPressed");
                    if (bPressed == 0 && wParam != 0) {
                        SetProp(hWnd, "__ButtonIsPressed", 1);
                    } else {
                        if (wParam != 0 || bPressed == 0) {
                            return 0;
                        }
                        RemoveProp(hWnd, "__ButtonIsPressed");
                    }
                    InvalidateRect(hWnd, 0, 1);
                    return 0;
                }
                case BM_SETSTYLE:
                    hWnd->dwStyle = dwStyle | 0xF;
                    return 0;
                case BM_CLICK:
                    SendMessage(hWnd, WM_LBUTTONDOWN, 0, 0);
                    SendMessage(hWnd, WM_LBUTTONUP, 0, 0);
                    return 0;
                default:
                    return DefWindowProc(hWnd, uMsg, wParam, lParam);
            }
        }
        if (uMsg > WM_NCHITTEST + 2) {
            if (uMsg == WM_GETDLGCODE) {
                return (dwStyle & 0xF) < 10 ? s_anButtonDlgCode[dwStyle & 0xF] : 0x2000;
            }
            return DefWindowProc(hWnd, uMsg, wParam, lParam);
        }
        if (uMsg < WM_SETFOCUS) {
            if (uMsg != WM_DESTROY) {
                return DefWindowProc(hWnd, uMsg, wParam, lParam);
            }
            RemoveProp(hWnd, "__ButtonIsPressed");
            return DefWindowProc(hWnd, WM_DESTROY, wParam, lParam);
        }
        switch (uMsg) {
            case WM_SETFOCUS:
                if ((dwStyle & 0x4000) != 0) {
                    wNotify = hWnd->nID;
                    hParent = GetParent(hWnd);
                    SendMessage(hParent, WM_COMMAND, (wNotify & 0xFFFF) | 0x60000, (LPARAM)hWnd);
                }
                return 0;
            case WM_KILLFOCUS:
                if ((dwStyle & 0x4000) != 0) {
                    wNotify = hWnd->nID;
                    hParent = GetParent(hWnd);
                    SendMessage(hParent, WM_COMMAND, (wNotify & 0xFFFF) | 0x70000, (LPARAM)hWnd);
                }
                return 0;
            case WM_SETTEXT: {
                LRESULT lText = DefWindowProc(hWnd, WM_SETTEXT, wParam, lParam);
                if (lText != 0) {
                    InvalidateRect(hWnd, 0, 1);
                    lResult = lText;
                }
                return lResult;
            }
            case WM_PAINT: {
                PAINTSTRUCT tPaint;
                HDC hDC = BeginPaint(hWnd, &tPaint);
                if (hDC != 0 && hWnd != 0) {
                    DWORD dwType = dwStyle & 0xF;
                    if (dwType > 1) {
                        if (dwType == 0xB) {
                            DRAWITEMSTRUCT tItem;
                            tItem.CtlType = 4;
                            tItem.CtlID = hWnd->nID;
                            tItem.itemID = 0;
                            tItem.itemAction = 1;
                            tItem.itemState = 0;
                            tItem.hwndItem = hWnd;
                            tItem.itemData = 0;
                            tItem.hDC = hDC;
                            GetClientRect(hWnd, &tItem.rcItem);
                            if (GetFocus() == hWnd) {
                                tItem.itemState |= 0x10;
                            }
                            if ((dwStyle & 0x8000000) != 0) {
                                tItem.itemState |= 4;
                            }
                            hParent = GetParent(hWnd);
                            SendMessage(hParent, WM_DRAWITEM, hWnd->nID, (LPARAM)&tItem);
                        } else if (dwType == 9) {
                            fp_fatal("Only painting of owner-drawn buttons has been implemented");
                        } else {
                            fp_fatal("Proc_PaintButton: Unknown button style");
                        }
                    }
                }
                EndPaint(hWnd, &tPaint);
                return 0;
            }
            default:
                return DefWindowProc(hWnd, uMsg, wParam, lParam);
        }
    }
    if (uMsg - WM_MOUSEMOVE > 3) {
        return DefWindowProc(hWnd, uMsg, wParam, lParam);
    }
    int nMouseX = (short)lParam;
    int nMouseY = (int)lParam >> 16;
    switch (uMsg) {
        case WM_MOUSEMOVE:
            if (GetCapture() == hWnd) {
                RECT rcClient;
                GetClientRect(hWnd, &rcClient);
                BOOL bInside = PtInRect(&rcClient, nMouseX, nMouseY);
                SendMessage(hWnd, BM_SETSTATE, bInside != 0, 0);
            }
            return 0;
        case WM_LBUTTONDOWN:
            SendMessage(hWnd, BM_SETSTATE, 1, 0);
            SetFocus(hWnd);
            SetCapture(hWnd);
            return 0;
        case WM_LBUTTONUP: {
            if (GetCapture() != hWnd) {
                return 0;
            }
            ReleaseCapture();
            SendMessage(hWnd, BM_SETSTATE, 0, 0);
            RECT rcButton;
            GetClientRect(hWnd, &rcButton);
            if (PtInRect(&rcButton, nMouseX, nMouseY) == 0) {
                return 0;
            }
            if ((dwStyle & 0xF) == 9) {
                SendMessage(hWnd, BM_SETCHECK, 1, 0);
            }
            wNotify = hWnd->nID;
            hParent = GetParent(hWnd);
            SendMessage(hParent, WM_COMMAND, wNotify & 0xFFFF, (LPARAM)hWnd);
            return 0;
        }
        case WM_LBUTTONDBLCLK:
            wNotify = hWnd->nID;
            hParent = GetParent(hWnd);
            SendMessage(hParent, WM_COMMAND, (wNotify & 0xFFFF) | 0x50000, (LPARAM)hWnd);
            return 0;
    }
    return lResult;
}

// 1.14d mac: 0002f793
D2_NOINLINE LRESULT Proc_StaticWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    LRESULT lResult = 0;
    if (uMsg < WM_MOUSEMOVE) {
        if (uMsg - WM_SETFOCUS < 2) {
            InvalidateRect(hWnd, 0, 1);
        } else if (uMsg == WM_SETTEXT) {
            LRESULT lText = DefWindowProc(hWnd, WM_SETTEXT, wParam, lParam);
            if (lText != 0) {
                InvalidateRect(hWnd, 0, 1);
                lResult = lText;
            }
        } else if (uMsg == WM_PAINT) {
            PAINTSTRUCT tPaint;
            BeginPaint(hWnd, &tPaint);
            EndPaint(hWnd, &tPaint);
        } else {
            lResult = DefWindowProc(hWnd, uMsg, wParam, lParam);
        }
    } else if (uMsg - WM_MOUSEMOVE >= 3) {
        lResult = DefWindowProc(hWnd, uMsg, wParam, lParam);
    }
    return lResult;
}

// 1.14d mac: 0002f500
// Registers the classes every window needs: the desktop, Button, Static, Listbox, Edit.
D2_NOINLINE void Proc_RegisterBuiltInClasses(int hInstance) {
    WNDCLASSEX tClass;

    tClass.cbSize = sizeof(WNDCLASSEX);
    tClass.style = 0x10000000;
    tClass.lpfnWndProc = Proc_DesktopWndProc;
    tClass.cbClsExtra = 0;
    tClass.cbWndExtra = 0;
    tClass.hInstance = hInstance;
    tClass.hIcon = 0;
    tClass.hCursor = 0;
    tClass.hbrBackground = 0;
    tClass.lpszMenuName = "";
    tClass.lpszClassName = "Wnd_Desktop";
    tClass.hIconSm = 0;
    RegisterClassEx(&tClass);

    tClass.cbSize = sizeof(WNDCLASSEX);
    tClass.style = 8;
    tClass.lpfnWndProc = Proc_ButtonWndProc;
    tClass.lpszClassName = "Button";
    RegisterClassEx(&tClass);

    tClass.cbSize = sizeof(WNDCLASSEX);
    tClass.style = 0;
    tClass.lpfnWndProc = Proc_StaticWndProc;
    tClass.lpszClassName = "Static";
    RegisterClassEx(&tClass);

    tClass.cbSize = sizeof(WNDCLASSEX);
    tClass.style = 8;
    tClass.lpfnWndProc = ListBox_WndProc;
    tClass.lpszClassName = "Listbox";
    RegisterClassEx(&tClass);

    tClass.cbSize = sizeof(WNDCLASSEX);
    tClass.style = 8;
    tClass.lpfnWndProc = Edit_WndProc;
    tClass.lpszClassName = "Edit";
    RegisterClassEx(&tClass);
}

// 1.14d mac: 0002fa20
// The registered class of that name, case-insensitively, or NULL.
D2_NOINLINE Win32MacWindowClass* Wnd_FindClassByName(LPCSTR szClassName) {
    for (UINT i = 0; i < WINDOW_CLASS_COUNT; i++) {
        Win32MacWindowClass* pClass = &s_aWindowClasses[i];
        if (pClass->nUsed != 0 && _strnicmp(pClass->szClassName, szClassName, 0x100) == 0) {
            return pClass;
        }
    }
    return 0;
}

// 1.14d mac: 0002faf8
// A free slot of the class table with cbClsExtra bytes of class storage, or NULL.
D2_NOINLINE Win32MacWindowClass* Wnd_NewWndClass(int nClsExtra) {
    for (UINT i = 0; i < WINDOW_CLASS_COUNT; i++) {
        Win32MacWindowClass* pClass = &s_aWindowClasses[i];
        if (pClass->nUsed == 0) {
            pClass->cbClsExtra = nClsExtra;
            if (nClsExtra < 1) {
                pClass->pClsExtra = 0;
            } else {
                pClass->pClsExtra = Priv_AllocClear(nClsExtra);
            }
            return pClass;
        }
    }
    return 0;
}

// 1.14d mac: 0002f888
D2_NOINLINE HWND Wnd_CreateDesktopWindow(int hInstance, Win32MacWindowClass* pClass) {
    HWND hWnd = (HWND)Priv_AllocClear(pClass->cbWndExtra + sizeof(Win32MacWindow));
    if (hWnd != 0) {
        int nWidth = GetSystemMetrics(SM_CXSCREEN);
        int nHeight = GetSystemMetrics(SM_CYSCREEN);
        hWnd->n04 = 0;
        hWnd->n08 = 0;
        hWnd->pClass = pClass;
        hWnd->pfnWndProc = pClass->pfnWndProc;
        hWnd->dwExStyle = 0;
        hWnd->szClassName = pClass->szClassName;
        char* szText = (char*)Priv_Alloc(8);
        hWnd->szText = szText;
        strcpy(szText, "Desktop");
        hWnd->dwStyle = 0x12000000;
        hWnd->n2C = 0;
        hWnd->nY = 0;
        hWnd->nWidth = nWidth;
        hWnd->nHeight = nHeight;
        hWnd->nX = 0;
        hWnd->nID = 0;
        hWnd->hInstance = hInstance;
        hWnd->n10 = 0;
        hWnd->cMnemonic = 0;
        hWnd->rcWindow.left = 0;
        hWnd->rcWindow.top = 0;
        hWnd->pParent = 0;
        hWnd->pOwner = 0;
        hWnd->pLastChild = 0;
        hWnd->pFirstChild = 0;
        hWnd->rcWindow.right = nWidth;
        hWnd->rcWindow.bottom = nHeight;
        hWnd->hUpdateRgn = CreateRectRgn(0, 0, nWidth, nHeight);
        hWnd->nNeedsPaint = 1;
        g_pMsgState->bPaintPending++;
        hWnd->n78 = 1;
        hWnd->hFont = (int)GetStockObject(SYSTEM_FONT);
    }
    return hWnd;
}

// 1.14d mac: 0002f9e0
D2_NOINLINE void Wnd_InitWnd(int hInstance) {
    Proc_RegisterBuiltInClasses(hInstance);
    Win32MacWindowClass* pClass = Wnd_FindClassByName("Wnd_Desktop");
    g_hDesktopWnd = Wnd_CreateDesktopWindow(hInstance, pClass);
}

// 1.14d mac: 0002fad7
D2_NOINLINE void Wnd_FreeWnd(HWND hWnd) {
    if (hWnd->szText != 0) {
        Priv_Free(hWnd->szText);
    }
    Priv_Free(hWnd);
}

// 1.14d mac: 0002fa76
// Tears the window system down.
D2_NOINLINE void Wnd_Shutdown(void) {
    if (g_hDesktopWnd != 0) {
        if (g_hDesktopWnd->hUpdateRgn != 0) {
            DeleteObject(g_hDesktopWnd->hUpdateRgn);
            g_hDesktopWnd->hUpdateRgn = 0;
        }
        Wnd_FreeWnd(g_hDesktopWnd);
    }
    if (s_hScratchRgn != 0) {
        DeleteObject(s_hScratchRgn);
        s_hScratchRgn = 0;
    }
}

// 1.14d mac: 0002fb57
D2_NOINLINE BOOL Wnd_IsSelfOrDescendant(HWND hWndFind, HWND hWndRoot, int nDepth) {
    if (hWndFind == hWndRoot) {
        return 1;
    }
    for (HWND hChild = hWndRoot->pFirstChild; hChild != 0; hChild = hChild->pNextSibling) {
        if (Wnd_IsSelfOrDescendant(hWndFind, hChild, nDepth + 1) != 0) {
            return 1;
        }
    }
    return 0;
}

// 1.14d mac: 0002fb9f
// Tells each ancestor a child window was created or destroyed, unless the window opted out.
D2_NOINLINE BOOL Wnd_SendParentNotify(HWND hWnd, WPARAM wEvent, int nChildID, LPARAM lParam) {
    if (g_hDesktopWnd == hWnd) {
        return 0;
    }
    while (hWnd != 0) {
        if ((hWnd->dwStyle & WS_CHILD) == 0) {
            return 0;
        }
        if ((hWnd->dwExStyle & 4) != 0) {
            continue;
        }
        hWnd = hWnd->pParent;
        SendMessage(hWnd, WM_PARENTNOTIFY, 0, lParam);
    }
    return 0;
}

// 1.14d mac: 0002fe9b
// The ancestor whose parent is the desktop.
D2_NOINLINE HWND Wnd_GetTopParent(HWND hWnd) {
    while (hWnd != 0) {
        HWND hParent = hWnd->pParent;
        if (hParent == g_hDesktopWnd) {
            return hWnd;
        }
        hWnd = hParent;
    }
    return 0;
}

// 1.14d mac: 0002ffdd
D2_NOINLINE HWND GetDesktopWindow(void) {
    return g_hDesktopWnd;
}

// 1.14d mac: 0002ffee
D2_NOINLINE LRESULT CallWindowProc(WNDPROC pfnWndProc, HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    if (pfnWndProc != 0 && Wnd_IsSelfOrDescendant(hWnd, g_hDesktopWnd, 0) != 0) {
        return pfnWndProc(hWnd, uMsg, wParam, lParam);
    }
    return 0;
}

// 1.14d mac: 00030049
D2_NOINLINE BOOL IsWindow(HWND hWnd) {
    return Wnd_IsSelfOrDescendant(hWnd, g_hDesktopWnd, 0);
}

// 1.14d mac: 00030077
D2_NOINLINE ATOM RegisterClassEx(const WNDCLASSEX* pClassEx) {
    if (Wnd_FindClassByName(pClassEx->lpszClassName) != 0) {
        return 0;
    }
    Win32MacWindowClass* pClass = Wnd_NewWndClass(pClassEx->cbClsExtra);
    if (pClass == 0) {
        return 0;
    }
    ATOM nAtom = s_nNextAtom;
    s_nNextAtom = nAtom + 1;
    pClass->nUsed = nAtom;
    pClass->cbSize = pClassEx->cbSize;
    pClass->style = pClassEx->style;
    pClass->pfnWndProc = pClassEx->lpfnWndProc;
    pClass->cbWndExtra = pClassEx->cbWndExtra;
    pClass->hInstance = pClassEx->hInstance;
    pClass->hIcon = pClassEx->hIcon;
    pClass->hCursor = pClassEx->hCursor;
    pClass->hbrBackground = pClassEx->hbrBackground;
    if (pClassEx->lpszMenuName == 0) {
        pClass->szMenuName[0] = 0;
    } else {
        strncpy(pClass->szMenuName, pClassEx->lpszMenuName, 0x100);
    }
    if (pClassEx->lpszClassName == 0) {
        pClass->szClassName[0] = 0;
    } else {
        strncpy(pClass->szClassName, pClassEx->lpszClassName, 0x100);
    }
    if (pClass->pfnWndProc == 0) {
        pClass->pfnWndProc = DefWindowProc;
    }
    return nAtom;
}

// 1.14d mac: 00030162
D2_NOINLINE ATOM RegisterClass(const WNDCLASS* pClass) {
    WNDCLASSEX tClassEx;
    tClassEx.cbSize = sizeof(WNDCLASSEX);
    tClassEx.style = pClass->style;
    tClassEx.lpfnWndProc = pClass->lpfnWndProc;
    tClassEx.cbClsExtra = pClass->cbClsExtra;
    tClassEx.cbWndExtra = pClass->cbWndExtra;
    tClassEx.hInstance = pClass->hInstance;
    tClassEx.hIcon = pClass->hIcon;
    tClassEx.hCursor = pClass->hCursor;
    tClassEx.hbrBackground = pClass->hbrBackground;
    tClassEx.lpszMenuName = pClass->lpszMenuName;
    tClassEx.lpszClassName = pClass->lpszClassName;
    tClassEx.hIconSm = 0;
    return RegisterClassEx(&tClassEx);
}

// 1.14d mac: 000301c7
D2_NOINLINE BOOL UnregisterClass(LPCSTR szClassName) {
    Win32MacWindowClass* pClass = Wnd_FindClassByName(szClassName);
    if (pClass == 0) {
        return 0;
    }
    if (pClass->pClsExtra != 0) {
        Priv_Free(pClass->pClsExtra);
        pClass->pClsExtra = 0;
    }
    pClass->nUsed = 0;
    return 1;
}

// 1.14d mac: 00030791
static void Wnd_UnlinkFromParent(HWND hWnd) {
    if (hWnd == 0) {
        return;
    }
    HWND hParent = hWnd->pParent;
    if (hParent == 0) {
        return;
    }
    if (hParent->pFirstChild == hWnd) {
        hParent->pFirstChild = hWnd->pNextSibling;
    } else {
        hWnd->pPrevSibling->pNextSibling = hWnd->pNextSibling;
    }
    if (hParent->pLastChild == hWnd) {
        hParent->pLastChild = hWnd->pPrevSibling;
    } else {
        hWnd->pNextSibling->pPrevSibling = hWnd->pPrevSibling;
    }
}

// 1.14d mac: 0003020c
D2_NOINLINE BOOL DestroyWindow(HWND hWnd) {
    HWND hDesktop = g_hDesktopWnd;
    if (Wnd_IsSelfOrDescendant(hWnd, hDesktop, 0) == 0) {
        return 0;
    }
    if (hWnd->pParent != 0 && Wnd_IsSelfOrDescendant(hWnd->pParent, hDesktop, 0) != 0) {
        Wnd_SendParentNotify(hWnd, 2, hWnd->n08, (LPARAM)hWnd);
    }
    ShowWindow(hWnd, 0);
    Win32MacMsgState* pState = g_pMsgState;
    Msg_RemoveMsgsForHWnd(pState, hWnd);
    Msg_RemoveMsgsForHWnd(MSGSTATE_INPUT_RING(pState), hWnd);
    if (pState->hWndFocus == hWnd) {
        HWND hAncestor = hWnd->pParent;
        while (hAncestor != 0) {
            SetFocus(hAncestor);
            if (hAncestor == GetFocus()) {
                break;
            }
            hAncestor = hAncestor->pParent;
        }
        if (pState->hWndFocus == 0) {
            pState->hWndFocus = g_hDesktopWnd;
        }
    }
    if (pState->hWndActive == hWnd) {
        pState->hWndActive = 0;
        for (HWND hTop = g_hDesktopWnd->pFirstChild; hTop != 0; hTop = hTop->pNextSibling) {
            if (hTop != hWnd) {
                SetActiveWindow(hTop);
                break;
            }
        }
    }
    if (pState->hWndCapture == hWnd) {
        pState->hWndCapture = 0;
    }
    SendMessage(hWnd, WM_DESTROY, 0, 0);
    SendMessage(hWnd, WM_NCDESTROY, 0, 0);
    HWND hChild = hWnd->pFirstChild;
    while (hChild != 0) {
        HWND hNext = hChild->pNextSibling;
        DestroyWindow(hChild);
        hChild = hNext;
    }
    hWnd->pFirstChild = 0;
    hWnd->pLastChild = 0;
    HWND hOwned = g_hDesktopWnd->pFirstChild;
    while (hOwned != 0) {
        HWND hNext = hOwned->pNextSibling;
        if (hOwned->pOwner == hWnd) {
            DestroyWindow(hOwned);
        }
        hOwned = hNext;
    }
    if (hWnd->pParent != 0) {
        Wnd_UnlinkFromParent(hWnd);
    }
    DeleteObject(hWnd->hUpdateRgn);
    Wnd_FreeWnd(hWnd);
    return 1;
}

// 1.14d mac: 00030408
D2_NOINLINE BOOL ShowWindow(HWND hWnd, UINT nCmdShow) {
    if (hWnd == 0) {
        return 0;
    }
    DWORD dwStyle = hWnd->dwStyle;
    BOOL bWasVisible = (dwStyle >> 28) & 1;
    BYTE uFlags = 0;
    if (nCmdShow < 10) {
        uFlags = s_anShowFlags[nCmdShow];
        if (s_anShowVisible[nCmdShow] != bWasVisible) {
            SendMessage(hWnd, WM_SHOWWINDOW, s_anShowVisible[nCmdShow], 0);
            dwStyle = hWnd->dwStyle;
        }
    }
    BYTE uSwp = uFlags | SWP_NOACTIVATE;
    if ((dwStyle & WS_CHILD) == 0) {
        uSwp = uFlags;
    }
    SetWindowPos(hWnd, 0, 0, 0, 0, 0, uSwp);
    return bWasVisible;
}

// 1.14d mac: 000304b5
D2_NOINLINE HWND SetFocus(HWND hWnd) {
    Win32MacMsgState* pState = g_pMsgState;
    HWND hOldFocus = pState->hWndFocus;
    HWND hPrev = 0;
    if (hOldFocus != 0) {
        hPrev = hOldFocus;
    }
    if (hWnd == 0) {
        hWnd = g_hDesktopWnd;
    }
    if (hWnd != 0 && hWnd != hPrev) {
        for (HWND hCheck = hWnd; hCheck != 0; hCheck = hCheck->pParent) {
            if ((hCheck->dwStyle & 0x28000000) != 0) {
                return hPrev;
            }
        }
        pState->hWndFocus = hWnd;
        HWND hFocus = pState->hWndFocus;
        if (hFocus != g_hDesktopWnd) {
            HWND hTop;
            do {
                hTop = 0;
                if (hFocus == 0) {
                    break;
                }
                HWND hParent = hFocus->pParent;
                hTop = hFocus;
                hFocus = hParent;
            } while (hFocus != g_hDesktopWnd);
            if (hTop != pState->hWndActive) {
                SetActiveWindow(hTop);
            }
        }
        if (hOldFocus != 0) {
            SendMessage(hOldFocus, WM_KILLFOCUS, (WPARAM)hWnd, 0);
        }
        HWND hNewFocus = g_pMsgState->hWndFocus;
        if (hNewFocus != 0) {
            SendMessage(hNewFocus, WM_SETFOCUS, (WPARAM)hPrev, 0);
        }
    }
    return hPrev;
}

// 1.14d mac: 000305a7
D2_NOINLINE HWND GetFocus(void) {
    HWND hResult = 0;
    if (g_pMsgState != 0) {
        HWND hFocus = g_pMsgState->hWndFocus;
        if (hFocus != 0 && Wnd_IsSelfOrDescendant(hFocus, g_hDesktopWnd, 0) != 0) {
            hResult = hFocus;
        }
    }
    return hResult;
}

// 1.14d mac: 000305f6
D2_NOINLINE HWND SetActiveWindow(HWND hWnd) {
    Win32MacMsgState* pState = g_pMsgState;
    HWND hOldActive = pState->hWndActive;
    HWND hPrev = 0;
    if (hOldActive != 0) {
        hPrev = hOldActive;
    }
    if (hPrev != hWnd && hWnd != 0 && hWnd->pParent == g_hDesktopWnd && (hWnd->dwStyle & WS_DISABLED) == 0 &&
        (hWnd->dwStyle & WS_VISIBLE) != 0) {
        if (hOldActive != 0) {
            if (SendMessage(hOldActive, WM_NCACTIVATE, 0, 0) == 0) {
                return hPrev;
            }
            SendMessage(hOldActive, WM_ACTIVATE, 0, (LPARAM)hWnd);
            pState = g_pMsgState;
        }
        pState->hWndActive = hWnd;
        SendMessage(hWnd, WM_QUERYNEWPALETTE, 0, 0);
        if ((hWnd->dwStyle & WS_CHILD) == 0) {
            SetWindowPos(hWnd, 0, 0, 0, 0, 0, 0x13);
        }
        SendMessage(hWnd, WM_NCACTIVATE, 1, 0);
        SendMessage(hWnd, WM_ACTIVATE, 1, (LPARAM)hPrev);
        HWND hFocus = g_pMsgState->hWndFocus;
        if (hFocus != 0) {
            if (hFocus != g_hDesktopWnd) {
                HWND hTop;
                do {
                    hTop = 0;
                    if (hFocus == 0) {
                        break;
                    }
                    HWND hParent = hFocus->pParent;
                    hTop = hFocus;
                    hFocus = hParent;
                } while (hFocus != g_hDesktopWnd);
                if (hWnd == hTop) {
                    return hPrev;
                }
            }
            SetFocus(hWnd);
        }
    }
    return hPrev;
}

// 1.14d mac: 00030b23
// Puts hWnd into hParent's child list after nInsertAfter (1 = at the bottom; 0 = on top
// except that HWND_NOTOPMOST (-2) on the desktop goes behind the topmost windows).
static void Wnd_LinkToParent(HWND hWnd, HWND hParent, int nInsertAfter) {
    HWND hPrev;
    if (nInsertAfter == 1) {
        hPrev = hParent->pLastChild;
    } else if (nInsertAfter == -2 && g_hDesktopWnd == hParent) {
        HWND* ppLink = &hParent->pFirstChild;
        for (;;) {
            hPrev = *ppLink;
            if (hPrev == 0) {
                hPrev = 0;
                break;
            }
            ppLink = &hPrev->pNextSibling;
            if ((hPrev->dwExStyle & 8) == 0) {
                break;
            }
        }
    } else {
        hPrev = 0;
    }
    HWND* ppNext = hPrev == 0 ? &hParent->pFirstChild : &hPrev->pNextSibling;
    HWND hNext = *ppNext;
    hWnd->pPrevSibling = hPrev;
    if (hPrev == 0) {
        hParent->pFirstChild = hWnd;
    } else {
        hPrev->pNextSibling = hWnd;
    }
    hWnd->pNextSibling = hNext;
    if (hNext == 0) {
        hParent->pLastChild = hWnd;
    } else {
        hNext->pPrevSibling = hWnd;
    }
    hWnd->pParent = hParent;
}

// 1.14d mac: 000307c9
HWND CreateWindowEx(DWORD dwExStyle, LPCSTR szClassName, LPCSTR szWindowName, DWORD dwStyle, int nX, int nY, int nWidth,
                    int nHeight, HWND hWndParent, int hMenu, int hInstance, int lpParam) {
    if (nWidth == CW_USEDEFAULT) {
        nWidth = 640;
        nHeight = 480;
    }
    if (nX == CW_USEDEFAULT) {
        nX = 0;
        nY = 0;
    }
    DWORD dwIsChild = dwStyle & WS_CHILD;
    int nClampedHeight = nHeight;
    if (dwIsChild == 0) {
        int nScreenHeight = GetSystemMetrics(SM_CYSCREEN);
        if (nHeight + nY > nScreenHeight) {
            nClampedHeight = nScreenHeight - nY;
        }
    }
    HWND hDesktop = g_hDesktopWnd;
    HWND hLinkParent;
    if (hWndParent == 0) {
        hLinkParent = 0;
        if (dwIsChild != 0) {
            return 0;
        }
    } else {
        hLinkParent = hWndParent;
        if (hWndParent == hDesktop) {
            hLinkParent = 0;
        }
        if (Wnd_IsSelfOrDescendant(hWndParent, hDesktop, 0) == 0) {
            return 0;
        }
        if (hLinkParent != 0 && dwIsChild == 0) {
            if ((hLinkParent->dwStyle & WS_CHILD) != 0) {
                return 0;
            }
            HWND hTop = hDesktop->pFirstChild;
            while (hTop != 0 && hTop != hLinkParent) {
                hTop = hTop->pNextSibling;
            }
            if (hTop == 0) {
                return 0;
            }
        }
    }
    Win32MacWindowClass* pClass = Wnd_FindClassByName(szClassName);
    if (pClass == 0) {
        return 0;
    }
    HWND hWnd = (HWND)Priv_AllocClear(pClass->cbWndExtra + sizeof(Win32MacWindow));
    if (hWnd == 0) {
        return 0;
    }
    hWnd->n04 = 0;
    hWnd->n08 = dwIsChild != 0 ? hMenu : 0;
    hWnd->pClass = pClass;
    hWnd->pfnWndProc = pClass->pfnWndProc;
    hWnd->dwExStyle = dwExStyle;
    hWnd->szClassName = pClass->szClassName;
    hWnd->szText = 0;
    hWnd->dwStyle = dwStyle & ~WS_VISIBLE;
    hWnd->n2C = nX;
    hWnd->nY = nY;
    hWnd->nWidth = nWidth;
    hWnd->nHeight = nClampedHeight;
    hWnd->nX = (int)hWndParent;
    hWnd->nID = hMenu;
    hWnd->hInstance = hInstance;
    hWnd->n10 = lpParam;
    hWnd->cMnemonic = 0;
    HWND hOwner;
    if (dwIsChild == 0) {
        hOwner = hLinkParent;
        hLinkParent = g_hDesktopWnd;
        if (g_hDesktopWnd == 0) {
            hOwner = 0;
        }
    } else {
        hOwner = 0;
    }
    hWnd->rcWindow.left = nX;
    hWnd->rcWindow.top = nY;
    hWnd->rcWindow.right = nWidth + nX;
    hWnd->rcWindow.bottom = nClampedHeight + nY;
    hWnd->hUpdateRgn = CreateRectRgn(0, 0, 0, 0);
    hWnd->nNeedsPaint = 0;
    hWnd->n78 = 0;
    hWnd->hFont = (int)GetStockObject(SYSTEM_FONT);
    if (hLinkParent != 0) {
        Wnd_LinkToParent(hWnd, hLinkParent, 0);
    }
    hWnd->pOwner = hOwner;
    SendMessage(hWnd, WM_NCCREATE, 0, (LPARAM)&hWnd->n10);
    SendMessage(hWnd, WM_CREATE, 0, (LPARAM)&hWnd->n10);
    if (szWindowName != 0) {
        SendMessage(hWnd, WM_SETTEXT, 0, (LPARAM)szWindowName);
    }
    SendMessage(hWnd, WM_SIZE, 0,
                ((hWnd->rcWindow.bottom - hWnd->rcWindow.top) << 16) | ((hWnd->rcWindow.right & 0xFFFF) - hWnd->rcWindow.left));
    SendMessage(hWnd, WM_MOVE, 0, 0);
    Wnd_SendParentNotify(hWnd, 1, hWnd->n08, (LPARAM)hWnd);
    hWnd->n10 = 0;
    if ((dwStyle & WS_VISIBLE) != 0) {
        ShowWindow(hWnd, 5);
    }
    if (s_bAppActivated == 0) {
        SendMessage(hWnd, WM_ACTIVATEAPP, 1, 0);
        s_bAppActivated = 1;
    }
    return hWnd;
}

// 1.14d mac: 00030b93
D2_NOINLINE HWND GetParent(HWND hWnd) {
    HWND hResult = 0;
    if (Wnd_IsSelfOrDescendant(hWnd, g_hDesktopWnd, 0) != 0) {
        hResult = (hWnd->dwStyle & WS_CHILD) != 0 ? hWnd->pParent : hWnd->pOwner;
    }
    return hResult;
}

// 1.14d mac: 00030bdf
D2_NOINLINE HWND GetCapture(void) {
    if (g_pMsgState == 0) {
        return 0;
    }
    return g_pMsgState->hWndCapture;
}

// 1.14d mac: 00030c00
D2_NOINLINE HWND SetCapture(HWND hWnd) {
    Win32MacMsgState* pState = g_pMsgState;
    HWND hPrev = 0;
    if (pState != 0) {
        hPrev = pState->hWndCapture;
        pState->hWndCapture = hWnd;
        if (hPrev != 0 && hPrev != hWnd) {
            SendMessage(hPrev, 0x215, 0, (LPARAM)hWnd);
        }
    }
    return hPrev;
}

// 1.14d mac: 00030c56
D2_NOINLINE BOOL ReleaseCapture(void) {
    Win32MacMsgState* pState = g_pMsgState;
    if (pState == 0) {
        return 0;
    }
    HWND hPrev = pState->hWndCapture;
    pState->hWndCapture = 0;
    if (hPrev != 0) {
        SendMessage(hPrev, 0x215, 0, 0);
    }
    return 1;
}

// 1.14d mac: 00030cb2
D2_NOINLINE HWND GetActiveWindow(void) {
    HWND hActive = g_pMsgState->hWndActive;
    HWND hResult = 0;
    if (hActive != 0 && Wnd_IsSelfOrDescendant(hActive, g_hDesktopWnd, 0) != 0) {
        hResult = hActive;
    }
    return hResult;
}

// 1.14d mac: 0002fdc5
D2_NOINLINE BOOL GetClientRect(HWND hWnd, RECT* pRect) {
    if (hWnd == 0 || pRect == 0) {
        return 0;
    }
    pRect->left = 0;
    pRect->top = 0;
    pRect->right = hWnd->rcWindow.right - hWnd->rcWindow.left;
    pRect->bottom = hWnd->rcWindow.bottom - hWnd->rcWindow.top;
    return 1;
}

// 1.14d mac: 0002fec2
D2_NOINLINE BOOL ClientToScreen(HWND hWnd, POINT* pPoint) {
    if (pPoint == 0) {
        return 0;
    }
    int nX = pPoint->x;
    int nY = pPoint->y;
    for (; hWnd != 0; hWnd = hWnd->pParent) {
        nY += hWnd->rcWindow.top;
        nX += hWnd->rcWindow.left;
    }
    pPoint->x = nX;
    pPoint->y = nY;
    return 1;
}

// 1.14d mac: 0002fef3
D2_NOINLINE BOOL ScreenToClient(HWND hWnd, POINT* pPoint) {
    if (pPoint == 0) {
        return 0;
    }
    int nX = pPoint->x;
    int nY = pPoint->y;
    for (; hWnd != 0; hWnd = hWnd->pParent) {
        nY -= hWnd->rcWindow.top;
        nX -= hWnd->rcWindow.left;
    }
    pPoint->x = nX;
    pPoint->y = nY;
    return 1;
}

// 1.14d mac: 00031060
D2_NOINLINE BOOL GetWindowRect(HWND hWnd, RECT* pRect) {
    if (hWnd == 0 || pRect == 0) {
        return 0;
    }
    pRect->left = 0;
    pRect->top = 0;
    pRect->right = hWnd->rcWindow.right - hWnd->rcWindow.left;
    pRect->bottom = hWnd->rcWindow.bottom - hWnd->rcWindow.top;
    ClientToScreen(hWnd, (POINT*)pRect);
    ClientToScreen(hWnd, (POINT*)&pRect->right);
    return 1;
}

// 1.14d mac: 0002fdfe
// Takes a visible, opaque window's screen rectangle out of the region being built.
// 1 when nothing is left.
D2_NOINLINE BOOL Wnd_Iterate_RemoveWindowFromVisRgn(HWND hWnd, Win32MacVisRgnContext* pContext) {
    if ((hWnd->dwStyle & WS_VISIBLE) == 0 || (hWnd->dwExStyle & 0x20) != 0) {
        return 0;
    }
    RECT rcWindow;
    GetWindowRect(hWnd, &rcWindow);
    OffsetRect(&rcWindow, -pContext->nOriginX, -pContext->nOriginY);
    SetRectRgn(pContext->hScratch, rcWindow.left, rcWindow.top, rcWindow.right, rcWindow.bottom);
    return CombineRgn(pContext->hRgn, pContext->hRgn, pContext->hScratch, RGN_DIFF) == NULLREGION;
}

// 1.14d mac: 0002fbf8
// hRgn becomes the part of the window that shows: its client area less everything in front.
D2_NOINLINE void Wnd_CalculateVisibleRgn(HWND hWnd, HRGN hRgn, int bClipChildren) {
    int nOriginY = 0;
    int nOriginX = 0;
    SetRectRgn(hRgn, 0, 0, 0, 0);
    for (HWND hCheck = hWnd; hCheck != 0;) {
        DWORD dwStyle = hCheck->dwStyle;
        hCheck = hCheck->pParent;
        if ((dwStyle & WS_VISIBLE) == 0) {
            return;
        }
    }
    if (hWnd == 0) {
        SetRectRgn(hRgn, 0, 0, nOriginX, nOriginY);
        nOriginY = 0;
        nOriginX = 0;
    } else {
        SetRectRgn(hRgn, 0, 0, hWnd->rcWindow.right - hWnd->rcWindow.left, hWnd->rcWindow.bottom - hWnd->rcWindow.top);
        nOriginX = 0;
        nOriginY = 0;
        for (HWND hUp = hWnd; hUp != 0; hUp = hUp->pParent) {
            nOriginY += hUp->rcWindow.top;
            nOriginX += hUp->rcWindow.left;
        }
    }
    if (s_hScratchRgn == 0) {
        s_hScratchRgn = CreateRectRgn(0, 0, 0, 0);
        if (s_hScratchRgn == 0) {
            return;
        }
    }
    Win32MacVisRgnContext tContext;
    tContext.hRgn = hRgn;
    tContext.nOriginX = nOriginX;
    tContext.nOriginY = nOriginY;
    tContext.hScratch = s_hScratchRgn;
    if (hWnd->pPrevSibling != 0 && (hWnd->dwStyle & (WS_CHILD | WS_CLIPSIBLINGS)) != WS_CHILD) {
        HWND hSibling = hWnd->pPrevSibling;
        while (hSibling != 0) {
            HWND hNext = hSibling->pPrevSibling;
            if (Wnd_Iterate_RemoveWindowFromVisRgn(hSibling, &tContext) != 0) {
                return;
            }
            hSibling = hNext;
        }
    }
    if (hWnd->pParent != 0) {
        HWND hAncestor = hWnd->pParent;
        do {
            HWND hAbove = hAncestor->pParent;
            HWND hSibling = hAncestor->pPrevSibling;
            while (hSibling != 0) {
                HWND hNext = hSibling->pPrevSibling;
                if (Wnd_Iterate_RemoveWindowFromVisRgn(hSibling, &tContext) != 0) {
                    return;
                }
                hSibling = hNext;
            }
            hAncestor = hAbove;
        } while (hAncestor != 0);
    }
    if (bClipChildren != 0 && hWnd->pFirstChild != 0 && (hWnd->dwStyle & WS_CLIPCHILDREN) != 0) {
        HWND hChild = hWnd->pFirstChild;
        while (hChild != 0) {
            HWND hNext = hChild->pNextSibling;
            if (Wnd_Iterate_RemoveWindowFromVisRgn(hChild, &tContext) != 0) {
                return;
            }
            hChild = hNext;
        }
    }
}

// 1.14d mac: 0002ff24
// The child of hWndParent that contains the point (in the parent's client space),
// the deepest one with bRecurse; bVisibleOnly skips hidden and disabled windows.
D2_NOINLINE HWND Wnd_ChildFromPoint(HWND hWndParent, int nX, int nY, int bRecurse, int bVisibleOnly) {
    if (hWndParent == 0) {
        return 0;
    }
    for (HWND hChild = hWndParent->pFirstChild; hChild != 0; hChild = hChild->pNextSibling) {
        if (bVisibleOnly == 0 ||
            ((hChild->dwStyle & WS_VISIBLE) != 0 && (hChild->dwStyle & (WS_VISIBLE | WS_DISABLED)) != (WS_VISIBLE | WS_DISABLED))) {
            if (PtInRect(&hChild->rcWindow, nX, nY) != 0) {
                if (bRecurse == 0) {
                    return hChild;
                }
                HWND hDeeper = Wnd_ChildFromPoint(hChild, nX - hChild->rcWindow.left, nY - hChild->rcWindow.top, bRecurse,
                                                  bVisibleOnly);
                return hDeeper != 0 ? hDeeper : hChild;
            }
        }
    }
    return 0;
}

// 1.14d mac: 00030fce
D2_NOINLINE HWND WindowFromPoint(POINT pt) {
    HWND hResult = 0;
    if (g_hDesktopWnd != 0) {
        hResult = Wnd_ChildFromPoint(g_hDesktopWnd, pt.x, pt.y, 1, 1);
        if (hResult == 0) {
            hResult = 0;
            if (PtInRect(&g_hDesktopWnd->rcWindow, pt.x, pt.y) != 0) {
                hResult = g_hDesktopWnd;
            }
            if (hResult == 0) {
                return 0;
            }
        }
    }
    return hResult;
}

// What the invalidation helpers pass down the tree: the rectangle, the scratch region
// they build the piece in, and how each window takes it.
struct Win32MacInvalidateArgs {
    RECT rcInvalid;        // 0x00
    HRGN hRgn;             // 0x10
    int bErase;            // 0x14
    int bPaintNow;         // 0x18
    int bAllChildren;      // 0x1C  also the children of a WS_CLIPCHILDREN window
};

// 1.14d mac: 0003137a
static void Wnd_SendPaintIfPending(HWND hWnd) {
    if (hWnd != 0 && hWnd->nNeedsPaint != 0) {
        SendMessage(hWnd, WM_PAINT, 0, 0);
    }
}

// 1.14d mac: 00031a0a
// Adds hRgn to the window's update region.
static void Wnd_AddToUpdateRegion(HWND hWnd, HRGN hRgn, int bErase, int bPaintNow) {
    if ((hWnd->dwStyle & WS_VISIBLE) != 0) {
        int nType = CombineRgn(hWnd->hUpdateRgn, hWnd->hUpdateRgn, hRgn, 2);
        if (nType != NULLREGION && hWnd->nNeedsPaint == 0) {
            hWnd->nNeedsPaint = 1;
            g_pMsgState->bPaintPending++;
        }
    }
    if (bErase != 0) {
        hWnd->n78 = 1;
    }
    if (bPaintNow != 0) {
        Wnd_SendPaintIfPending(hWnd);
    }
}

// 1.14d mac: 000318bf
D2_NOINLINE BOOL Wnd_InvalidateRectRecursive(HWND hWnd, Win32MacInvalidateArgs* pArgs) {
    if ((hWnd->dwStyle & WS_VISIBLE) != 0) {
        RECT rc = pArgs->rcInvalid;
        OffsetRect(&rc, -hWnd->rcWindow.left, -hWnd->rcWindow.top);
        RECT rcClient;
        rcClient.left = 0;
        rcClient.top = 0;
        rcClient.right = hWnd->rcWindow.right - hWnd->rcWindow.left;
        rcClient.bottom = hWnd->rcWindow.bottom - hWnd->rcWindow.top;
        if (IntersectRect(&rc, &rc, &rcClient) != 0) {
            SetRectRgn(pArgs->hRgn, rc.left, rc.top, rc.right, rc.bottom);
            Wnd_AddToUpdateRegion(hWnd, pArgs->hRgn, pArgs->bErase, pArgs->bPaintNow);
            if (hWnd->pFirstChild != 0 && (pArgs->bAllChildren != 0 || (hWnd->dwStyle & WS_CLIPCHILDREN) == 0)) {
                Win32MacInvalidateArgs tChild;
                tChild.rcInvalid = rc;
                tChild.hRgn = pArgs->hRgn;
                tChild.bErase = pArgs->bErase;
                tChild.bPaintNow = pArgs->bPaintNow;
                tChild.bAllChildren = pArgs->bAllChildren;
                HWND hChild = hWnd->pFirstChild;
                while (hChild != 0) {
                    HWND hNext = hChild->pNextSibling;
                    if (Wnd_InvalidateRectRecursive(hChild, &tChild) != 0) {
                        return 1;
                    }
                    hChild = hNext;
                }
            }
        }
    }
    return 0;
}

// 1.14d mac: 000311e9
// Invalidates pRect (the window's own client space; the whole window for NULL).
static void Wnd_InvalidateWndRect(HWND hWnd, const RECT* pRect, int bErase, int bPaintNow, int bAllChildren) {
    if ((hWnd->dwStyle & WS_VISIBLE) == 0) {
        return;
    }
    if (s_hScratchRgn == 0) {
        s_hScratchRgn = CreateRectRgn(0, 0, 0, 0);
        if (s_hScratchRgn == 0) {
            return;
        }
    }
    Win32MacInvalidateArgs tArgs;
    tArgs.hRgn = s_hScratchRgn;
    tArgs.bErase = bErase;
    tArgs.bPaintNow = bPaintNow;
    tArgs.bAllChildren = bAllChildren;
    if (pRect == 0) {
        tArgs.rcInvalid = hWnd->rcWindow;
    } else {
        tArgs.rcInvalid = *pRect;
        OffsetRect(&tArgs.rcInvalid, hWnd->rcWindow.left, hWnd->rcWindow.top);
    }
    Wnd_InvalidateRectRecursive(hWnd, &tArgs);
}

// 1.14d mac: 000310be
// Invalidates pRect of the window's parent (pRect is in the parent's space) and the
// windows in front of hWnd that it also covers.
static void Wnd_InvalidateInParent(HWND hWnd, const RECT* pRect) {
    HWND hParent = hWnd->pParent;
    if ((hParent->dwStyle & WS_VISIBLE) == 0) {
        return;
    }
    if ((hParent->dwStyle & WS_CLIPCHILDREN) == 0) {
        Wnd_InvalidateWndRect(hParent, pRect, 1, 0, 0);
        return;
    }
    if (s_hScratchRgn == 0) {
        s_hScratchRgn = CreateRectRgn(0, 0, 0, 0);
        if (s_hScratchRgn == 0) {
            return;
        }
    }
    Win32MacInvalidateArgs tArgs;
    tArgs.bErase = 1;
    tArgs.bPaintNow = 0;
    tArgs.bAllChildren = 1;
    tArgs.rcInvalid = *pRect;
    tArgs.hRgn = s_hScratchRgn;
    SetRectRgn(s_hScratchRgn, pRect->left, pRect->top, pRect->right, pRect->bottom);
    Wnd_AddToUpdateRegion(hParent, tArgs.hRgn, 1, 0);
    HWND hSibling = hWnd->pNextSibling;
    while (hSibling != 0) {
        HWND hNext = hSibling->pNextSibling;
        if (Wnd_InvalidateRectRecursive(hSibling, &tArgs) != 0) {
            return;
        }
        hSibling = hNext;
    }
}

// 1.14d mac: 00031874
// Marks hWnd, if hOwner owns it, and everything it owns in turn. 1 when it did.
D2_NOINLINE BOOL Wnd_MarkOwnedWindows(HWND hWnd, HWND hOwner) {
    if (hWnd->pOwner != hOwner) {
        return 0;
    }
    hWnd->n7C = 1;
    HWND hSibling = hWnd->pParent->pFirstChild;
    while (hSibling != 0) {
        HWND hNext = hSibling->pNextSibling;
        if (Wnd_MarkOwnedWindows(hSibling, hWnd) != 0) {
            return 1;
        }
        hSibling = hNext;
    }
    return 0;
}

// 1.14d mac: 000317ac
// Moves hWnd and the windows it owns to after hAfter. 1 when hAfter is itself one of them
// (nothing moves), 0 when they moved.
static BOOL Wnd_ReorderOwnedWindows(HWND hWnd, HWND hAfter) {
    hWnd->n7C = 1;
    HWND hSibling = hWnd->pParent->pFirstChild;
    while (hSibling != 0) {
        HWND hNext = hSibling->pNextSibling;
        if (Wnd_MarkOwnedWindows(hSibling, hWnd) != 0) {
            break;
        }
        hSibling = hNext;
    }
    if (hAfter == 0 || hAfter->n7C == 0) {
        HWND hMarked = hWnd->pParent->pLastChild;
        while (hMarked != 0) {
            HWND hPrev = hMarked->pPrevSibling;
            if (hMarked->n7C != 0) {
                Wnd_InvalidateInParent(hMarked, &hMarked->rcWindow);
                Wnd_UnlinkFromParent(hMarked);
                Wnd_LinkToParent(hMarked, hMarked->pParent, (int)hAfter);
                Wnd_InvalidateWndRect(hMarked, 0, 1, 0, 1);
                hMarked->n7C = 0;
            }
            hMarked = hPrev;
        }
        return 0;
    }
    HWND hClear = hWnd->pParent->pFirstChild;
    while (hClear != 0) {
        hClear->n7C = 0;
        hClear = hClear->pNextSibling;
    }
    return 1;
}

// 1.14d mac: 00030cfd
D2_NOINLINE BOOL SetWindowPos(HWND hWnd, HWND hWndInsertAfter, int nX, int nY, int cx, int cy, BYTE uFlags) {
    BOOL bResult = 0;
    if (hWnd == 0) {
        return 0;
    }
    DWORD dwStyle = hWnd->dwStyle;
    UINT nFlags = uFlags;
    if ((dwStyle & WS_VISIBLE) != 0) {
        nFlags &= ~0x40u;
    } else {
        nFlags &= ~0x80u;
        if ((uFlags & 0x40) == 0) {
            nFlags |= 8;
        }
    }
    UINT nNoActivate = nFlags | SWP_NOACTIVATE;
    Win32MacMsgState* pState = g_pMsgState;
    if (hWnd == pState->hWndActive) {
        nFlags = nNoActivate;
    }
    HWND hParent = hWnd->pParent;
    if (hParent != g_hDesktopWnd) {
        nFlags = nNoActivate;
    }
    int nLeft = hWnd->rcWindow.left;
    if (hWnd->rcWindow.right - nLeft == cx) {
        nFlags |= hWnd->rcWindow.bottom - hWnd->rcWindow.top == cy;
    }
    if (nLeft == nX && hWnd->rcWindow.top == nY) {
        nFlags |= SWP_NOMOVE;
    }
    if (hWnd == (HWND)-2 && (hWnd->dwExStyle & 8) == 0) {
        nFlags |= SWP_NOZORDER;
    }
    HWND hAfter = hWndInsertAfter;
    if ((dwStyle & WS_CHILD) != 0) {
        hAfter = (UINT)hWndInsertAfter <= (UINT)-3 ? hWndInsertAfter : 0;
    }
    if ((nFlags & SWP_NOZORDER) == 0) {
        if ((UINT)hAfter == 1) {
            hAfter = hParent->pLastChild;
        } else {
            hAfter = (UINT)hAfter <= (UINT)-3 ? hAfter : 0;
        }
        if ((UINT)hAfter >= 2 && (UINT)hAfter + 2 >= 2) {
            if (hAfter->pParent != hParent) {
                nFlags |= SWP_NOZORDER;
            }
            if (hAfter->pNextSibling == hWnd) {
                nFlags |= SWP_NOZORDER;
            }
        }
    }
    RECT* pRect = &hWnd->rcWindow;
    if ((nFlags & (SWP_NOSIZE | SWP_NOMOVE)) != (SWP_NOSIZE | SWP_NOMOVE)) {
        RECT rcNew = *pRect;
        if ((nFlags & SWP_NOSIZE) == 0) {
            rcNew.right = cx + rcNew.left;
            rcNew.bottom = cy + rcNew.top;
        }
        if ((nFlags & SWP_NOMOVE) == 0) {
            OffsetRect(&rcNew, nX - rcNew.left, nY - rcNew.top);
        }
        RECT rcUncovered;
        if (SubtractRect(&rcUncovered, pRect, &rcNew) != 0) {
            Wnd_InvalidateInParent(hWnd, &rcUncovered);
        }
        *pRect = rcNew;
        Wnd_InvalidateWndRect(hWnd, 0, 1, 0, 1);
    }
    if ((nFlags & SWP_NOZORDER) == 0) {
        bool bInPlace = hAfter != 0 ? hWnd->pPrevSibling == hAfter : hWnd->pPrevSibling == 0;
        if (!bInPlace) {
            HWND hTop = hWnd;
            while (hTop->pOwner != 0) {
                hTop = hTop->pOwner;
            }
            if (hTop == hWnd || Wnd_ReorderOwnedWindows(hTop, hAfter) == 0) {
                Wnd_ReorderOwnedWindows(hWnd, hAfter);
            }
        }
    }
    if ((nFlags & 0x40) != 0) {
        if ((hWnd->dwStyle & WS_VISIBLE) == 0) {
            hWnd->dwStyle |= WS_VISIBLE;
            Wnd_InvalidateWndRect(hWnd, 0, 1, 0, 1);
        }
    } else if ((nFlags & 0x80) != 0 && (hWnd->dwStyle & WS_VISIBLE) != 0) {
        hWnd->dwStyle &= ~WS_VISIBLE;
        Wnd_ClearUpdateRegionTree(hWnd, 0);
        Wnd_InvalidateInParent(hWnd, pRect);
        if (hWnd == g_pMsgState->hWndActive) {
            for (HWND hNext = g_hDesktopWnd->pFirstChild; hNext != 0; hNext = hNext->pNextSibling) {
                if ((hNext->dwStyle & WS_VISIBLE) != 0 && hNext != hWnd) {
                    SetActiveWindow(hNext);
                    break;
                }
            }
        }
    }
    bResult = 1;
    if ((nFlags & SWP_NOACTIVATE) == 0) {
        SetActiveWindow(hWnd);
    }
    return bResult;
}

// 1.14d mac: 000312b5
D2_NOINLINE BOOL AdjustWindowRectEx(void* pRect, DWORD dwStyle, BOOL bMenu, DWORD dwExStyle) {
    return 1;
}

// 1.14d mac: 000312bf
D2_NOINLINE BOOL InvalidateRect(HWND hWnd, const RECT* pRect, BOOL bErase) {
    RECT rcInvalid;
    int bPaintNow = 0;
    if (hWnd == 0) {
        bPaintNow = 1;
        hWnd = g_hDesktopWnd;
        if (hWnd == 0) {
            return 0;
        }
        pRect = 0;
    }
    if (pRect != 0) {
        rcInvalid = *pRect;
    } else {
        rcInvalid.left = 0;
        rcInvalid.top = 0;
        rcInvalid.right = hWnd->rcWindow.right - hWnd->rcWindow.left;
        rcInvalid.bottom = hWnd->rcWindow.bottom - hWnd->rcWindow.top;
    }
    Wnd_InvalidateWndRect(hWnd, &rcInvalid, bErase, bPaintNow, 1);
    return 1;
}

// 1.14d mac: 0003135a
D2_NOINLINE BOOL UpdateWindow(HWND hWnd) {
    if (hWnd != 0) {
        Wnd_SendPaintIfPending(hWnd);
        return 1;
    }
    return 0;
}

// 1.14d mac: 0003149c
static void Wnd_ClearUpdateRegion(HWND hWnd) {
    SetRectRgn(hWnd->hUpdateRgn, 0, 0, 0, 0);
    if (hWnd->nNeedsPaint != 0) {
        g_pMsgState->bPaintPending--;
        hWnd->nNeedsPaint = 0;
    }
}

// 1.14d mac: 000313af
D2_NOINLINE HDC BeginPaint(HWND hWnd, PAINTSTRUCT* pPaint) {
    HDC hDC = 0;
    if (hWnd != 0 && pPaint != 0) {
        hDC = GetDC(hWnd);
        if (hDC != 0) {
            ExtSelectClipRgn(hDC, hWnd->hUpdateRgn, 1);
            pPaint->hdc = hDC;
            pPaint->fErase = 1;
            if (hWnd->n78 != 0) {
                LRESULT lErase = SendMessage(hWnd, WM_ERASEBKGND, (WPARAM)hDC, 0);
                pPaint->fErase = lErase == 0;
                hWnd->n78 = 0;
            }
            GetClipBox(hDC, &pPaint->rcPaint);
            memset(pPaint->rgbReserved, 0, sizeof(pPaint->rgbReserved));
        }
        Wnd_ClearUpdateRegion(hWnd);
    }
    return hDC;
}

// 1.14d mac: 000314f9
D2_NOINLINE BOOL EndPaint(HWND hWnd, const PAINTSTRUCT* pPaint) {
    ReleaseDC(hWnd, pPaint->hdc);
    return 1;
}

// 1.14d mac: 0003151d
D2_NOINLINE BOOL IsWindowVisible(HWND hWnd) {
    if (hWnd == 0) {
        return 0;
    }
    return (hWnd->dwStyle >> 28) & 1;
}

// 1.14d mac: 00031539
D2_NOINLINE BYTE Wnd_IsVisibleAndEnabled(HWND hWnd) {
    BYTE bVisible = 0;
    bool bEnabled = false;
    if (hWnd != 0) {
        DWORD dwStyle = hWnd->dwStyle;
        bEnabled = (dwStyle & WS_DISABLED) == 0;
        bVisible = (dwStyle & 0x10000000) >> 28;
    }
    return bEnabled & bVisible;
}

// 1.14d mac: 00031564
D2_NOINLINE int Wnd_ReturnZero(void) {
    return 0;
}

// 1.14d mac: 0003156b
D2_NOINLINE int GetWindowLong(HWND hWnd, int nIndex) {
    if (hWnd == 0) {
        return 0;
    }
    if ((UINT)nIndex < 0xFFFFFFF0u) {
        if (nIndex == -21) {
            return hWnd->n04;
        }
        if (nIndex == -20) {
            return hWnd->dwExStyle;
        }
    } else if ((UINT)nIndex < 0xFFFFFFF4u) {
        if (nIndex == -16) {
            return hWnd->dwStyle;
        }
    } else {
        switch (nIndex) {
            case -12:
                return hWnd->n08;
            case -8:
                return (int)hWnd->pParent;
            case -6:
                return hWnd->hInstance;
            case -4:
                if (hWnd->pfnWndProc != 0) {
                    return (int)hWnd->pfnWndProc;
                }
                return (int)hWnd->pClass->pfnWndProc;
        }
    }
    if (nIndex >= 0 && (UINT)nIndex <= (UINT)hWnd->pClass->cbClsExtra - 4) {
        return *(int*)((BYTE*)hWnd + nIndex + 0x8C);
    }
    return 0;
}

// 1.14d mac: 00031618
D2_NOINLINE int GetProp(HWND hWnd, LPCSTR szName) {
    if (hWnd == 0 || szName == 0 || *szName == 0) {
        return 0;
    }
    Win32MacWindowProp* pProp = hWnd->pProps;
    while (pProp != 0) {
        if (strcmp(szName, pProp->szName) == 0) {
            return pProp->nValue;
        }
        pProp = pProp->pNext;
    }
    return 0;
}

// 1.14d mac: 00031666
D2_NOINLINE BOOL SetProp(HWND hWnd, LPCSTR szName, int nValue) {
    if (hWnd == 0 || szName == 0 || *szName == 0) {
        return 0;
    }
    Win32MacWindowProp* pProp = hWnd->pProps;
    while (pProp != 0) {
        if (strcmp(szName, pProp->szName) == 0) {
            pProp->nValue = nValue;
            return 1;
        }
        pProp = pProp->pNext;
    }
    pProp = (Win32MacWindowProp*)Priv_Alloc(strlen(szName) + 0xC);
    strcpy(pProp->szName, szName);
    pProp->nValue = nValue;
    pProp->pNext = hWnd->pProps;
    hWnd->pProps = pProp;
    return 1;
}

// 1.14d mac: 000316f2
D2_NOINLINE int RemoveProp(HWND hWnd, LPCSTR szName) {
    if (hWnd == 0 || szName == 0 || *szName == 0) {
        return 0;
    }
    Win32MacWindowProp* pPrev = 0;
    Win32MacWindowProp* pProp = hWnd->pProps;
    while (pProp != 0) {
        if (strcmp(szName, pProp->szName) == 0) {
            int nValue = pProp->nValue;
            if (pPrev == 0) {
                hWnd->pProps = pProp->pNext;
            } else {
                pPrev->pNext = pProp->pNext;
            }
            Priv_Free(pProp);
            return nValue;
        }
        pPrev = pProp;
        pProp = pProp->pNext;
    }
    return 0;
}

// 1.14d mac: 00031766
// Clears the update region of the window and of every window below it.
D2_NOINLINE BOOL Wnd_ClearUpdateRegionTree(HWND hWnd, int nReserved) {
    Wnd_ClearUpdateRegion(hWnd);
    HWND hChild = hWnd->pFirstChild;
    while (hChild != 0) {
        HWND hNext = hChild->pNextSibling;
        if (Wnd_ClearUpdateRegionTree(hChild, 0) != 0) {
            return 1;
        }
        hChild = hNext;
    }
    return 0;
}

// 1.14d mac: 00031a84
// The Win32 virtual key of a Mac key code: the letter the keyboard layout gives it, else a
// fixed table.
D2_NOINLINE UINT Win32VirtualKey_From_MacScanCode(BYTE nKeyCode) {
    uint32_t nKeyState = 0;
    void* pKCHR = (void*)GetScriptManagerVariable(smKCHRCache);
    uint32_t nResult = KeyTranslate(pKCHR, nKeyCode, &nKeyState);
    if ((nResult & 0xFF) - 0x61 < 0x1A || ((nResult >>= 16), (nResult & 0xFF) - 0x61 < 0x1A)) {
        nResult += 0xE0;
    } else {
        nResult = 0;
        if ((char)nKeyCode >= 0) {
            nResult = s_abyMacKeyToVirtualKey[nKeyCode];
        }
    }
    return nResult & 0xFF;
}

// 1.14d mac: 00031af6
D2_NOINLINE BYTE Key_VirtualKeyToMacKeyCode(BYTE nIndex) {
    return s_abyVirtualKeyToMacKey[nIndex];
}

#endif  // D2_OS_MAC
