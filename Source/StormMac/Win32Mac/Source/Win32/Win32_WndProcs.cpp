// Storm's Win32-on-Mac layer: DefWindowProc and the window procedures of the built-in
// classes (desktop, Button, Static) with their registration, 1.14d Intel Mac
// 0002eae0..0002f888. A unit of its own ahead of the window manager (Win32_Window.cpp):
// RegisterClassEx reaches DefWindowProc through a non-lazy pointer, and the button
// procedure calls the window manager's leaf accessors (GetCapture, GetClientRect) out of
// line. No binary records the file name (see Source/StormMac/README.md).
#include "Win32MacWindow.h"

#if !defined(_WIN32) && D2_OS_MAC

#include <string.h>

#define CS_NOCLOSE 0x0200
#define SC_CLOSE 0xF060
#define WM_ACTIVATE 0x0006
#define WM_NCACTIVATE 0x0086
#define WM_HELP 0x0053
#define WM_ENABLE 0x000A
#define WM_DRAWITEM 0x002B
#define BM_SETCHECK 0x00F1
#define BM_GETSTATE 0x00F2
#define BM_SETSTATE 0x00F3
#define BM_SETSTYLE 0x00F4
#define BM_CLICK 0x00F5
#define WM_GETDLGCODE 0x0087
#define SYSTEM_FONT 13

// 1.14d mac: 003224f0
// WM_GETDLGCODE for each button style.
static const int s_anButtonDlgCode[10] = {0x2020, 0x2010, 0x2000, 0x2000, 0x2040,
                                          0x2000, 0x2000, 0x2000, 0x2000, 0x2040};

// 1.14d mac: 0002ef30
LRESULT Proc_DesktopWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    if (uMsg != WM_PAINT && uMsg != WM_ERASEBKGND) {
        return 0;
    }
    return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

// 1.14d mac: 0002eae0
// What a window does with the messages nothing else claims.
LRESULT DefWindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
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
LRESULT Proc_ButtonWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
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
LRESULT Proc_StaticWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    LRESULT lResult = 0;
    switch (uMsg) {
        case WM_SETFOCUS:
        case WM_KILLFOCUS:
            InvalidateRect(hWnd, 0, 1);
            break;
        case WM_SETTEXT: {
            LRESULT lText = DefWindowProc(hWnd, WM_SETTEXT, wParam, lParam);
            if (lText != 0) {
                InvalidateRect(hWnd, 0, 1);
                lResult = lText;
            }
            break;
        }
        case WM_PAINT: {
            PAINTSTRUCT tPaint;
            BeginPaint(hWnd, &tPaint);
            EndPaint(hWnd, &tPaint);
            break;
        }
        case WM_MOUSEMOVE:
        case WM_LBUTTONDOWN:
        case WM_LBUTTONUP:
            break;
        default:
            lResult = DefWindowProc(hWnd, uMsg, wParam, lParam);
            break;
    }
    return lResult;
}

// 1.14d mac: 0002f500
// Registers the classes every window needs: the desktop, Button, Static, Listbox, Edit.
void Proc_RegisterBuiltInClasses(int hInstance) {
    WNDCLASSEX tClass;

    memset(&tClass, 0, sizeof(tClass));
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

    memset(&tClass, 0, sizeof(tClass));
    tClass.cbSize = sizeof(WNDCLASSEX);
    tClass.style = 8;
    tClass.lpfnWndProc = Proc_ButtonWndProc;
    tClass.cbClsExtra = 0;
    tClass.cbWndExtra = 0;
    tClass.hInstance = hInstance;
    tClass.hIcon = 0;
    tClass.hCursor = 0;
    tClass.hbrBackground = 0;
    tClass.lpszMenuName = "";
    tClass.lpszClassName = "Button";
    tClass.hIconSm = 0;
    RegisterClassEx(&tClass);

    memset(&tClass, 0, sizeof(tClass));
    tClass.cbSize = sizeof(WNDCLASSEX);
    tClass.style = 0;
    tClass.lpfnWndProc = Proc_StaticWndProc;
    tClass.cbClsExtra = 0;
    tClass.cbWndExtra = 0;
    tClass.hInstance = hInstance;
    tClass.hIcon = 0;
    tClass.hCursor = 0;
    tClass.hbrBackground = 0;
    tClass.lpszMenuName = "";
    tClass.lpszClassName = "Static";
    tClass.hIconSm = 0;
    RegisterClassEx(&tClass);

    memset(&tClass, 0, sizeof(tClass));
    tClass.cbSize = sizeof(WNDCLASSEX);
    tClass.style = 8;
    tClass.lpfnWndProc = ListBox_WndProc;
    tClass.cbClsExtra = 0;
    tClass.cbWndExtra = 0;
    tClass.hInstance = hInstance;
    tClass.hIcon = 0;
    tClass.hCursor = 0;
    tClass.hbrBackground = 0;
    tClass.lpszMenuName = "";
    tClass.lpszClassName = "Listbox";
    tClass.hIconSm = 0;
    RegisterClassEx(&tClass);

    memset(&tClass, 0, sizeof(tClass));
    tClass.cbSize = sizeof(WNDCLASSEX);
    tClass.style = 8;
    tClass.lpfnWndProc = Edit_WndProc;
    tClass.cbClsExtra = 0;
    tClass.cbWndExtra = 0;
    tClass.hInstance = hInstance;
    tClass.hIcon = 0;
    tClass.hCursor = 0;
    tClass.hbrBackground = 0;
    tClass.lpszMenuName = "";
    tClass.lpszClassName = "Edit";
    tClass.hIconSm = 0;
    RegisterClassEx(&tClass);
}

#endif
