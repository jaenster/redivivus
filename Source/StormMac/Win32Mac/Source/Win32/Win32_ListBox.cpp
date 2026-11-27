// Storm's Win32-on-Mac layer: the list box control, 1.14d Intel Mac band 000270a4..00028a00
// (ListBox_WndProc, registered as the "listbox" class proc, .. LISTBOX_GetItemFromY). The
// original file name is unknown (see Source/StormMac/README.md).
//
// The control's state is a 0x2C-byte block hung on the window as the property
// "__ListBoxData"; the items are a doubly linked list with the text copied after the item
// (or, for an owner-drawn list without strings, just the 32-bit item data).
#include "Win32ListBox.h"

#if !defined(_WIN32) && D2_OS_MAC

#include <string.h>

#define WM_CREATE 0x0001
#define WM_DESTROY 0x0002
#define WM_SIZE 0x0005
#define WM_SETFOCUS 0x0007
#define WM_KILLFOCUS 0x0008
#define WM_ENABLE 0x000A
#define WM_PAINT 0x000F
#define WM_ERASEBKGND 0x0014
#define WM_GETDLGCODE 0x0087
#define WM_COMMAND 0x0111
#define WM_TIMER 0x0113
#define WM_VSCROLL 0x0115
#define WM_GETFONT 0x0031
#define WM_DRAWITEM 0x002B
#define WM_MOUSEMOVE 0x0200
#define WM_LBUTTONDOWN 0x0201
#define WM_LBUTTONUP 0x0202
#define WM_LBUTTONDBLCLK 0x0203

#define LB_ADDSTRING 0x0180
#define LB_INSERTSTRING 0x0181
#define LB_DELETESTRING 0x0182
#define LB_RESETCONTENT 0x0184
#define LB_SETCURSEL 0x0186
#define LB_GETCURSEL 0x0188
#define LB_GETTEXT 0x0189
#define LB_GETTEXTLEN 0x018A
#define LB_GETCOUNT 0x018B
#define LB_SELECTSTRING 0x018C
#define LB_GETTOPINDEX 0x018E
#define LB_FINDSTRING 0x018F
#define LB_SETTABSTOPS 0x0192
#define LB_SETTOPINDEX 0x0197
#define LB_GETITEMDATA 0x0199
#define LB_SETITEMDATA 0x019A
#define LB_SETITEMHEIGHT 0x01A0
#define LB_GETITEMHEIGHT 0x01A1
#define LB_FINDSTRINGEXACT 0x01A2

#define LBS_NOTIFY 0x0001
#define LBS_OWNERDRAWFIXED 0x0010
#define LBS_HASSTRINGS 0x0040
#define LBS_USETABSTOPS 0x0080
#define LBS_NOSEL 0x4000  // tested in the style's second byte

#define LBN_SELCHANGE 1
#define LBN_DBLCLK 2
#define LBN_SETFOCUS 4
#define LBN_KILLFOCUS 5

#define GWL_ID (-12)
#define COLOR_HIGHLIGHT 0x0D
#define COLOR_HIGHLIGHTTEXT 0x0E
#define ETO_OPAQUE 2
#define ETO_CLIPPED 4
#define RGN_AND 1
#define ODT_LISTBOX 2
#define ODA_DRAWENTIRE 1

#define LB_ERR (-1)

struct ListBoxItem {
    ListBoxItem* pNext;   // 0x00
    ListBoxItem* pPrev;   // 0x04
    DWORD dwData;         // 0x08
    char* szText;         // 0x0C  the copy after the item; owner-drawn without strings: the item data
    DWORD nLength;        // 0x10  text bytes with the terminator
    char szBuffer[1];     // 0x14
};

struct ListBoxData {
    DWORD dwStyle;        // 0x00  LBS_*
    LONG nCount;          // 0x04
    LONG nTopIndex;       // 0x08  -1 when empty
    LONG nSelection;      // 0x0C  -1 for none
    LONG nPressed;        // 0x10  the item the button went down on
    LONG nScrollDir;      // 0x14  -1/1 while the mouse is dragged out of the client area, else 0
    UINT nTimerID;        // 0x18  auto-scroll timer
    DWORD dwTabStop;      // 0x1C  LB_SETTABSTOPS
    LONG nItemHeight;     // 0x20
    ListBoxItem* pFirst;  // 0x24
    ListBoxItem* pLast;   // 0x28
};
WIN32MAC_LAYOUT(ListBoxData_size, sizeof(ListBoxData) == 0x2C);

// The creation parameters, from which only the style is read.
struct Win32MacCreateStruct {
    BYTE pad00[0x20];
    DWORD style;          // 0x20
};

struct DRAWITEMSTRUCT {
    UINT CtlType;
    UINT CtlID;
    UINT itemID;
    UINT itemAction;
    UINT itemState;
    HWND hwndItem;
    HDC hDC;
    RECT rcItem;
    DWORD itemData;
};

// 1.14d mac: 00031618
void* GetProp(HWND hWnd, LPCSTR szString);
// 1.14d mac: 00031666
BOOL SetProp(HWND hWnd, LPCSTR szString, void* hData);
// 1.14d mac: 000316f2
void* RemoveProp(HWND hWnd, LPCSTR szString);
// 1.14d mac: 00030bdf
HWND GetCapture(void);
// 1.14d mac: 00030c00
HWND SetCapture(HWND hWnd);
// 1.14d mac: 00030c56
BOOL ReleaseCapture(void);
// 1.14d mac: 000304b5
HWND SetFocus(HWND hWnd);
// 1.14d mac: 0002fdc5
BOOL GetClientRect(HWND hWnd, RECT* pRect);
// 1.14d mac: 00031060
BOOL GetWindowRect(HWND hWnd, RECT* pRect);
// 1.14d mac: 00030cfd
BOOL SetWindowPos(HWND hWnd, HWND hWndInsertAfter, int X, int Y, int cx, int cy, UINT uFlags);
// 1.14d mac: 00030b93
HWND GetParent(HWND hWnd);
// 1.14d mac: 0003156b
LONG GetWindowLong(HWND hWnd, int nIndex);
// 1.14d mac: 000295f5
LONG SendMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
// 1.14d mac: 00029755
BOOL KillTimer(HWND hWnd, UINT nIDEvent);
// 1.14d mac: 000296ce
UINT SetTimer(HWND hWnd, UINT nIDEvent, UINT uElapse, void* pfnTimerFunc);
// 1.14d mac: 000312bf
BOOL InvalidateRect(HWND hWnd, const RECT* pRect, BOOL bErase);
// 1.14d mac: 000313af
HDC BeginPaint(HWND hWnd, PAINTSTRUCT* pPaint);
// 1.14d mac: 000314f9
BOOL EndPaint(HWND hWnd, const PAINTSTRUCT* pPaint);
// 1.14d mac: 0002eae0
LONG DefWindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
// 1.14d mac: 00024419
HGDIOBJ SelectObject(HDC hDC, HGDIOBJ hObject);
// 1.14d mac: 000249a9
COLORREF GetSysColor(int nIndex);
// 1.14d mac: 00024694
COLORREF SetTextColor(HDC hDC, COLORREF crColor);
// 1.14d mac: 000247a8
COLORREF SetBkColor(HDC hDC, COLORREF crColor);
// 1.14d mac: 0002bbc3
BOOL ExtTextOut(HDC hDC, int X, int Y, UINT fuOptions, const RECT* pRect, LPCSTR szString, UINT nCount);
// 1.14d mac: 00022da9
HRGN CreateRectRgn(int nLeft, int nTop, int nRight, int nBottom);
// 1.14d mac: 00023e6c
int GetClipRgn(HDC hDC, HRGN hRgn);
// 1.14d mac: 000241a5
BOOL SetRectRgn(HRGN hRgn, int nLeft, int nTop, int nRight, int nBottom);
// 1.14d mac: 00023f14
int CombineRgn(HRGN hRgnDest, HRGN hRgnSrc1, HRGN hRgnSrc2, int nMode);
// 1.14d mac: 000243f2
int SelectClipRgn(HDC hDC, HRGN hRgn);
// 1.14d mac: 00022f56
BOOL DeleteObject(HGDIOBJ hObject);
// 1.14d mac: 00033a24
int _stricmp(const char* szA, const char* szB);
// 1.14d mac: 00033a6f
int _strnicmp(const char* szA, const char* szB, size_t nCount);

static const char s_szListBoxData[] = "__ListBoxData";

static ListBoxItem* ListBox_GetItemByIndex(ListBoxData* pData, LONG nIndex);
static LONG ListBox_On_WM_LBUTTONDOWN(HWND hWnd, LPARAM lParam);
static LONG ListBox_On_LB_SETCURSEL(HWND hWnd, LONG nIndex);
static LONG ListBox_On_LB_SETTOPINDEX(HWND hWnd, LONG nIndex);
static int ListBox_FindString(HWND hWnd, int nStart, const char* szFind, int bExact);
static BOOL SetSelection(HWND hWnd, ListBoxData* pData, LONG nIndex, BOOL bNotify);
static void LISTBOX_InvalidateItem(HWND hWnd, ListBoxData* pData, LONG nIndex);
static int LISTBOX_DeleteString(HWND hWnd, ListBoxData* pData, LONG nIndex);
static int LISTBOX_InsertString(ListBoxData* pData, LONG nIndex, const void* pText, size_t nLength);
static BOOL LISTBOX_MoveSelection(HWND hWnd, ListBoxData* pData, LONG nDelta);
static int LISTBOX_GetItemFromY(HWND hWnd, ListBoxData* pData, int nY);

// 1.14d mac: 000270a4
// Registered by Proc_RegisterBuiltInClasses as the listbox class proc.
LONG ListBox_WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    ListBoxData* pData = (ListBoxData*)GetProp(hWnd, s_szListBoxData);
    if (pData == NULL && uMsg != WM_CREATE) {
        return DefWindowProc(hWnd, uMsg, wParam, lParam);
    }
    LONG nResult;
    switch (uMsg) {
    case WM_MOUSEMOVE: {
        pData = (ListBoxData*)GetProp(hWnd, s_szListBoxData);
        nResult = 0;
        if (pData->nTimerID != 0 && GetCapture() == hWnd) {
            int nY = (LPARAM)lParam >> 16;
            RECT rcClient;
            GetClientRect(hWnd, &rcClient);
            if (nY < rcClient.top) {
                pData->nScrollDir = -1;
            } else if (nY < rcClient.bottom) {
                pData->nScrollDir = 0;
                int nItem = LISTBOX_GetItemFromY(hWnd, pData, nY);
                if (nItem > -1) {
                    SetSelection(hWnd, pData, nItem, 1);
                }
            } else {
                pData->nScrollDir = 1;
            }
        }
        return nResult;
    }
    case WM_LBUTTONDOWN:
        return ListBox_On_WM_LBUTTONDOWN(hWnd, lParam);
    case WM_LBUTTONUP:
        pData = (ListBoxData*)GetProp(hWnd, s_szListBoxData);
        if (pData->nTimerID != 0 && GetCapture() == hWnd) {
            ReleaseCapture();
            KillTimer(hWnd, pData->nTimerID);
            pData->nTimerID = 0;
            pData->nScrollDir = 0;
        }
        return 0;
    case WM_LBUTTONDBLCLK: {
        pData = (ListBoxData*)GetProp(hWnd, s_szListBoxData);
        LONG nPressed = pData->nPressed;
        nResult = ListBox_On_WM_LBUTTONDOWN(hWnd, lParam);
        if ((pData->dwStyle & LBS_NOTIFY) && nPressed != -1 && nPressed == pData->nPressed) {
            HWND hParent = GetParent(hWnd);
            UINT uID = GetWindowLong(hWnd, GWL_ID);
            SendMessage(hParent, WM_COMMAND, (uID & 0xFFFF) | (LBN_DBLCLK << 16), (LPARAM)hWnd);
        }
        return nResult;
    }
    case WM_TIMER:
        pData = (ListBoxData*)GetProp(hWnd, s_szListBoxData);
        nResult = -1;
        if (GetCapture() == hWnd && pData->nTimerID == wParam) {
            nResult = 0;
            if (pData->nScrollDir != 0) {
                LISTBOX_MoveSelection(hWnd, pData, pData->nScrollDir);
            }
        }
        return nResult;
    case WM_VSCROLL:
        pData = (ListBoxData*)GetProp(hWnd, s_szListBoxData);
        switch (wParam & 0xFFFF) {
        case 0:
            if (pData->nTopIndex != -1) {
                if (pData->nTopIndex > 0) {
                    ListBox_On_LB_SETTOPINDEX(hWnd, pData->nTopIndex - 1);
                }
                return 0;
            }
            break;
        case 1:
            if (pData->nTopIndex != -1) {
                if (pData->nCount > 0 && pData->nTopIndex < pData->nCount - 1) {
                    ListBox_On_LB_SETTOPINDEX(hWnd, pData->nTopIndex + 1);
                }
                return 0;
            }
            break;
        case 4:
        case 5:
        case 8:
            ListBox_On_LB_SETTOPINDEX(hWnd, wParam >> 16);
            return 0;
        default:
            return 0;
        }
        ListBox_On_LB_SETTOPINDEX(hWnd, 0);
        return 0;
    case 0x100: {  // WM_KEYDOWN
        pData = (ListBoxData*)GetProp(hWnd, s_szListBoxData);
        nResult = -1;
        LONG nDelta = -1;
        RECT rcClient;
        switch (wParam) {
        case 0x21:  // VK_PRIOR
            GetClientRect(hWnd, &rcClient);
            nDelta = 1 - (rcClient.bottom - rcClient.top) / pData->nItemHeight;
            break;
        case 0x22:  // VK_NEXT
            GetClientRect(hWnd, &rcClient);
            nDelta = (rcClient.bottom - rcClient.top) / pData->nItemHeight - 1;
            break;
        case 0x23:  // VK_END
            nDelta = pData->nCount - 1;
            if (pData->nCount < 1) {
                nDelta = 0;
            }
            break;
        case 0x24:  // VK_HOME
            nDelta = 0;
            if (pData->nCount > 0) {
                nDelta = 1 - pData->nCount;
            }
            break;
        case 0x26:  // VK_UP
            break;
        case 0x28:  // VK_DOWN
            nDelta = 1;
            break;
        default:
            return nResult;
        }
        if (nDelta != 0) {
            BOOL bMoved = LISTBOX_MoveSelection(hWnd, pData, nDelta);
            nResult = -(bMoved == 0);
        }
        return nResult;
    }
    case 0x87:  // WM_GETDLGCODE
        return 1;
    case WM_CREATE: {
        const Win32MacCreateStruct* pCreate = (const Win32MacCreateStruct*)lParam;
        if (hWnd == NULL || pCreate == NULL) {
            return -1;
        }
        ListBoxData* pNew = (ListBoxData*)Priv_Alloc(sizeof(ListBoxData));
        if (pNew == NULL) {
            return -1;
        }
        pNew->dwStyle = pCreate->style;
        pNew->nCount = 0;
        pNew->nTopIndex = -1;
        pNew->nSelection = -1;
        pNew->nPressed = -1;
        pNew->nScrollDir = 0;
        pNew->nTimerID = 0;
        pNew->dwTabStop = 8;
        pNew->nItemHeight = 0x10;
        pNew->pFirst = NULL;
        pNew->pLast = NULL;
        return -(SetProp(hWnd, s_szListBoxData, pNew) == 0);
    }
    case WM_DESTROY: {
        ListBoxData* pOld = (ListBoxData*)RemoveProp(hWnd, s_szListBoxData);
        if (pOld != NULL) {
            if (pOld->nTimerID != 0) {
                KillTimer(hWnd, pOld->nTimerID);
                pOld->nTimerID = 0;
            }
            for (LONG n = pOld->nCount; n != 0; n--) {
                LISTBOX_DeleteString(hWnd, pOld, 0);
            }
            Priv_Free(pOld);
        }
        return 0;
    }
    case WM_SIZE: {
        pData = (ListBoxData*)GetProp(hWnd, s_szListBoxData);
        RECT rcClient;
        GetClientRect(hWnd, &rcClient);
        int nHeight = rcClient.bottom - rcClient.top;
        if (pData->nItemHeight < nHeight && nHeight % pData->nItemHeight != 0) {
            RECT rcWindow;
            GetWindowRect(hWnd, &rcWindow);
            SetWindowPos(hWnd, NULL, 0, 0, rcWindow.right - rcWindow.left,
                         (rcWindow.bottom - rcWindow.top) - nHeight % pData->nItemHeight, 0x16);
        }
        return 0;
    }
    case WM_SETFOCUS:
    case WM_KILLFOCUS: {
        GetProp(hWnd, s_szListBoxData);
        HWND hParent = GetParent(hWnd);
        UINT uID = GetWindowLong(hWnd, GWL_ID);
        UINT uNotify = uMsg == WM_SETFOCUS ? LBN_SETFOCUS : LBN_KILLFOCUS;
        SendMessage(hParent, WM_COMMAND, (uID & 0xFFFF) | (uNotify << 16), (LPARAM)hWnd);
        return 0;
    }
    case WM_ENABLE:
        GetProp(hWnd, s_szListBoxData);
        InvalidateRect(hWnd, NULL, 1);
        return 0;
    case WM_PAINT: {
        pData = (ListBoxData*)GetProp(hWnd, s_szListBoxData);
        PAINTSTRUCT tPaint;
        HDC hDC = BeginPaint(hWnd, &tPaint);
        HGDIOBJ hOldFont = NULL;
        HGDIOBJ hFont = (HGDIOBJ)SendMessage(hWnd, WM_GETFONT, 0, 0);
        if (hFont != NULL) {
            hOldFont = SelectObject(hDC, hFont);
        }
        RECT rcItem;
        GetClientRect(hWnd, &rcItem);
        if (pData->nCount > 0 && pData->nTopIndex >= 0) {
            LONG nTop = pData->nTopIndex;
            RECT rcClient;
            GetClientRect(hWnd, &rcClient);
            LONG nItemHeight = pData->nItemHeight;
            LONG nCount = pData->nCount;
            DRAWITEMSTRUCT tDraw;
            tDraw.CtlType = ODT_LISTBOX;
            tDraw.CtlID = GetWindowLong(hWnd, GWL_ID);
            tDraw.itemAction = ODA_DRAWENTIRE;
            tDraw.hwndItem = hWnd;
            LONG nLast = nTop - 1 + (rcClient.bottom - rcClient.top) / nItemHeight;
            if (nLast > nCount - 1) {
                nLast = nCount - 1;
            }
            tDraw.hDC = hDC;
            HRGN hItemRgn;
            HRGN hOldClipRgn;
            if (!(pData->dwStyle & LBS_OWNERDRAWFIXED)) {
                hOldClipRgn = NULL;
                hItemRgn = NULL;
            } else {
                hItemRgn = CreateRectRgn(0, 0, 0, 0);
                hOldClipRgn = CreateRectRgn(0, 0, 0, 0);
                if (hOldClipRgn != NULL) {
                    GetClipRgn(hDC, hOldClipRgn);
                }
            }
            for (LONG nRow = 0; nTop + nRow <= nLast; nRow++) {
                LONG nItemTop = pData->nItemHeight * nRow;
                LONG nItemBottom = pData->nItemHeight + nItemTop;
                LONG nIndex = nTop + nRow;
                rcItem.top = nItemTop;
                rcItem.bottom = nItemBottom;
                ListBoxItem* pItem = ListBox_GetItemByIndex(pData, nIndex);
                if (!(pData->dwStyle & LBS_OWNERDRAWFIXED)) {
                    COLORREF crBk;
                    COLORREF crText;
                    if (nIndex == pData->nSelection) {
                        crBk = SetBkColor(hDC, GetSysColor(COLOR_HIGHLIGHT));
                        crText = SetTextColor(hDC, GetSysColor(COLOR_HIGHLIGHTTEXT));
                    } else {
                        crText = SetTextColor(hDC, 0xFFFFFF);
                        crBk = SetBkColor(hDC, 0);
                    }
                    ExtTextOut(hDC, rcItem.left, rcItem.top, ETO_OPAQUE | ETO_CLIPPED, &rcItem, pItem->szText,
                               strlen(pItem->szText));
                    SetTextColor(hDC, crText);
                    SetBkColor(hDC, crBk);
                } else {
                    tDraw.itemState = nIndex == pData->nSelection;
                    tDraw.rcItem = rcItem;
                    tDraw.itemData = pItem->dwData;
                    tDraw.itemID = nIndex;
                    SetRectRgn(hItemRgn, rcItem.left, nItemTop, rcItem.right, pData->nItemHeight + nItemTop);
                    CombineRgn(hItemRgn, hItemRgn, hOldClipRgn, RGN_AND);
                    SelectClipRgn(hDC, hItemRgn);
                    SendMessage(GetParent(hWnd), WM_DRAWITEM, tDraw.CtlID, (LPARAM)&tDraw);
                }
            }
            if (hOldClipRgn != NULL) {
                SelectClipRgn(hDC, hOldClipRgn);
                DeleteObject(hOldClipRgn);
            }
            if (hItemRgn != NULL) {
                DeleteObject(hItemRgn);
            }
        }
        if (hOldFont != NULL) {
            SelectObject(hDC, hOldFont);
        }
        EndPaint(hWnd, &tPaint);
        return 0;
    }
    case WM_ERASEBKGND:
        GetProp(hWnd, s_szListBoxData);
        DefWindowProc(hWnd, WM_ERASEBKGND, wParam, lParam);
        return 1;
    case LB_ADDSTRING: {
        pData = (ListBoxData*)GetProp(hWnd, s_szListBoxData);
        size_t nLength = strlen((const char*)lParam) + 1;
        nResult = LB_ERR;
        int nIndex = LISTBOX_InsertString(pData, -1, (const char*)lParam, nLength);
        if (nIndex != -1) {
            InvalidateRect(hWnd, NULL, 1);
            nResult = nIndex;
        }
        return nResult;
    }
    case LB_INSERTSTRING: {
        pData = (ListBoxData*)GetProp(hWnd, s_szListBoxData);
        LONG nCount = pData->nCount;
        size_t nLength = strlen((const char*)lParam) + 1;
        if ((LONG)wParam > nCount - 1 || nCount == 0 || (LONG)wParam < -1) {
            wParam = -1;
        }
        nResult = LB_ERR;
        int nIndex = LISTBOX_InsertString(pData, wParam, (const char*)lParam, nLength);
        if (nIndex != -1) {
            InvalidateRect(hWnd, NULL, 1);
            nResult = nIndex;
        }
        return nResult;
    }
    case LB_DELETESTRING: {
        pData = (ListBoxData*)GetProp(hWnd, s_szListBoxData);
        nResult = LB_ERR;
        if ((LONG)wParam < 0 || pData->nCount <= (LONG)wParam) {
            return nResult;
        }
        int nRemaining = LISTBOX_DeleteString(hWnd, pData, wParam);
        if (nRemaining == -1) {
            return nResult;
        }
        InvalidateRect(hWnd, NULL, 1);
        return nRemaining;
    }
    case LB_RESETCONTENT:
        pData = (ListBoxData*)GetProp(hWnd, s_szListBoxData);
        for (LONG n = pData->nCount; n != 0; n--) {
            LISTBOX_DeleteString(hWnd, pData, 0);
        }
        InvalidateRect(hWnd, NULL, 1);
        return 0;
    case LB_SETCURSEL:
        return ListBox_On_LB_SETCURSEL(hWnd, wParam);
    case LB_GETCURSEL:
        pData = (ListBoxData*)GetProp(hWnd, s_szListBoxData);
        nResult = LB_ERR;
        if (pData->nSelection >= 0 && pData->nSelection < pData->nCount) {
            nResult = pData->nSelection;
        }
        return nResult;
    case LB_GETTEXT: {
        pData = (ListBoxData*)GetProp(hWnd, s_szListBoxData);
        nResult = LB_ERR;
        if ((LONG)wParam < 0 || pData->nCount <= (LONG)wParam) {
            return nResult;
        }
        ListBoxItem* pItem = ListBox_GetItemByIndex(pData, wParam);
        if (pItem == NULL || lParam == 0) {
            return nResult;
        }
        if ((pData->dwStyle & (LBS_HASSTRINGS | LBS_OWNERDRAWFIXED)) == LBS_OWNERDRAWFIXED) {
            *(DWORD*)lParam = pItem->dwData;
            return 4;
        }
        if (pItem->szText == NULL) {
            return nResult;
        }
        strcpy((char*)lParam, pItem->szText);
        return strlen((const char*)lParam);
    }
    case LB_GETTEXTLEN: {
        pData = (ListBoxData*)GetProp(hWnd, s_szListBoxData);
        nResult = LB_ERR;
        if ((LONG)wParam < 0 || pData->nCount <= (LONG)wParam) {
            return nResult;
        }
        ListBoxItem* pItem = ListBox_GetItemByIndex(pData, wParam);
        if (pItem == NULL) {
            return nResult;
        }
        nResult = 4;
        if ((pData->dwStyle & (LBS_HASSTRINGS | LBS_OWNERDRAWFIXED)) == LBS_OWNERDRAWFIXED) {
            return nResult;
        }
        char* szText = pItem->szText;
        nResult = LB_ERR;
        if (szText == NULL) {
            return nResult;
        }
        return strlen(szText);
    }
    case LB_GETCOUNT:
        pData = (ListBoxData*)GetProp(hWnd, s_szListBoxData);
        nResult = LB_ERR;
        if (pData->nCount > 0) {
            nResult = pData->nCount;
        }
        return nResult;
    case LB_SELECTSTRING: {
        int nIndex = ListBox_FindString(hWnd, wParam, (const char*)lParam, 0);
        nResult = LB_ERR;
        if (nIndex != -1) {
            ListBox_On_LB_SETCURSEL(hWnd, nIndex);
            nResult = nIndex;
        }
        return nResult;
    }
    case LB_GETTOPINDEX:
        pData = (ListBoxData*)GetProp(hWnd, s_szListBoxData);
        nResult = LB_ERR;
        if (pData->nCount < 1) {
            return nResult;
        }
        return pData->nTopIndex;
    case LB_FINDSTRING:
        // Falls into the default: the result of the search is dropped.
        ListBox_FindString(hWnd, wParam, (const char*)lParam, 0);
        return DefWindowProc(hWnd, uMsg, wParam, lParam);
    case LB_FINDSTRINGEXACT:
        ListBox_FindString(hWnd, wParam, (const char*)lParam, 1);
        return DefWindowProc(hWnd, uMsg, wParam, lParam);
    case LB_SETTABSTOPS:
        pData = (ListBoxData*)GetProp(hWnd, s_szListBoxData);
        nResult = LB_ERR;
        if (pData->dwStyle & LBS_USETABSTOPS) {
            pData->dwTabStop = *(DWORD*)lParam;
            InvalidateRect(hWnd, NULL, 1);
            nResult = 1;
        }
        return nResult;
    case LB_SETTOPINDEX:
        return ListBox_On_LB_SETTOPINDEX(hWnd, wParam);
    case LB_GETITEMDATA: {
        pData = (ListBoxData*)GetProp(hWnd, s_szListBoxData);
        nResult = LB_ERR;
        if ((LONG)wParam < 0 || pData->nCount <= (LONG)wParam) {
            return nResult;
        }
        ListBoxItem* pItem = ListBox_GetItemByIndex(pData, wParam);
        if (pItem == NULL) {
            return nResult;
        }
        return pItem->dwData;
    }
    case LB_SETITEMDATA: {
        pData = (ListBoxData*)GetProp(hWnd, s_szListBoxData);
        nResult = LB_ERR;
        if ((LONG)wParam > -1 && (LONG)wParam < pData->nCount) {
            ListBoxItem* pItem = ListBox_GetItemByIndex(pData, wParam);
            if (pItem != NULL) {
                pItem->dwData = lParam;
                nResult = 0;
                if ((pData->dwStyle & (LBS_HASSTRINGS | LBS_OWNERDRAWFIXED)) == LBS_OWNERDRAWFIXED) {
                    pItem->szText = (char*)lParam;
                }
            }
        }
        return nResult;
    }
    case LB_SETITEMHEIGHT:
        pData = (ListBoxData*)GetProp(hWnd, s_szListBoxData);
        if (pData->nItemHeight != lParam) {
            pData->nItemHeight = lParam;
            InvalidateRect(hWnd, NULL, 1);
        }
        return 0;
    case LB_GETITEMHEIGHT:
        pData = (ListBoxData*)GetProp(hWnd, s_szListBoxData);
        return pData->nItemHeight;
    default:
        return DefWindowProc(hWnd, uMsg, wParam, lParam);
    }
}

// 1.14d mac: 00028224
// A left-button press: unless the list ignores selection, stops the scroll timer, selects the
// item under the mouse and on success takes focus and capture and starts the auto-scroll timer.
static LONG ListBox_On_WM_LBUTTONDOWN(HWND hWnd, LPARAM lParam) {
    ListBoxData* pData = (ListBoxData*)GetProp(hWnd, s_szListBoxData);
    LONG nResult = -1;
    if (!((pData->dwStyle >> 8) & (LBS_NOSEL >> 8))) {
        if (pData->nTimerID != 0) {
            KillTimer(hWnd, pData->nTimerID);
            pData->nTimerID = 0;
        }
        pData->nScrollDir = 0;
        pData->nPressed = -1;
        int nItem = LISTBOX_GetItemFromY(hWnd, pData, lParam >> 16);
        if (nItem != -1) {
            if (SetSelection(hWnd, pData, nItem, 1)) {
                pData->nPressed = nItem;
                SetFocus(hWnd);
                SetCapture(hWnd);
                pData->nTimerID = SetTimer(hWnd, 0, 100, NULL);
                nResult = 0;
            }
        }
    }
    return nResult;
}

// 1.14d mac: 000282fa
static LONG ListBox_On_LB_SETCURSEL(HWND hWnd, LONG nIndex) {
    ListBoxData* pData = (ListBoxData*)GetProp(hWnd, s_szListBoxData);
    if (nIndex == -1 || (nIndex > -1 && nIndex < pData->nCount)) {
        return SetSelection(hWnd, pData, nIndex, 0) ? 0 : -1;
    }
    return -1;
}

// 1.14d mac: 00028353
// Clamps the new top item to the list and to the last page that still fills the window.
static LONG ListBox_On_LB_SETTOPINDEX(HWND hWnd, LONG nIndex) {
    ListBoxData* pData = (ListBoxData*)GetProp(hWnd, s_szListBoxData);
    LONG nCount = pData->nCount;
    LONG nTop = 0;
    if (nIndex > -1) {
        if (nCount == 0) {
            return -1;
        }
        nTop = nIndex;
        if (nCount - 1 <= nIndex) {
            nTop = nCount - 1;
        }
        if (nTop < 0) {
            return -1;
        }
    }
    LONG nResult = -1;
    if (nTop < nCount) {
        RECT rcClient;
        GetClientRect(hWnd, &rcClient);
        nResult = 0;
        LONG nLastPage = nCount - (rcClient.bottom - rcClient.top) / pData->nItemHeight;
        if (nLastPage < 0) {
            nLastPage = 0;
        }
        if (nTop <= nLastPage) {
            nLastPage = nTop;
        }
        if (nLastPage != pData->nTopIndex) {
            pData->nTopIndex = nLastPage;
            InvalidateRect(hWnd, NULL, 1);
            nResult = 0;
        }
    }
    return nResult;
}

// 1.14d mac: 000283fc
// The item at an index; -1 is the last one.
static ListBoxItem* ListBox_GetItemByIndex(ListBoxData* pData, LONG nIndex) {
    ListBoxItem* pItem = NULL;
    if (pData != NULL && nIndex > -2 && nIndex < pData->nCount) {
        if (nIndex == -1) {
            pItem = pData->pLast;
        } else {
            pItem = pData->pFirst;
            for (; nIndex != 0; nIndex--) {
                pItem = pItem->pNext;
            }
        }
    }
    return pItem;
}

// 1.14d mac: 00028505
static int ListBox_CompareItemData(DWORD dwItemData, DWORD dwFindData) {
    return dwItemData == dwFindData;
}

// 1.14d mac: 00028516
static int ListBox_CompareStringExact(const char* szItem, const char* szFind) {
    return _stricmp(szItem, szFind) == 0;
}

// 1.14d mac: 0002853b
static int ListBox_CompareStringPrefix(const char* szItem, const char* szPrefix) {
    return _strnicmp(szItem, szPrefix, strlen(szPrefix)) == 0;
}

// 1.14d mac: 00028427
// Searches from the item after nStart, wrapping round; the index or -1.
static int ListBox_FindString(HWND hWnd, int nStart, const char* szFind, int bExact) {
    ListBoxData* pData = (ListBoxData*)GetProp(hWnd, s_szListBoxData);
    if (pData != NULL && pData->nCount > 0) {
        int nFirst = nStart + 1;
        int (*pfnMatch)(const char*, const char*);
        if ((pData->dwStyle & (LBS_HASSTRINGS | LBS_OWNERDRAWFIXED)) == LBS_OWNERDRAWFIXED) {
            pfnMatch = (int (*)(const char*, const char*))ListBox_CompareItemData;
        } else {
            pfnMatch = ListBox_CompareStringExact;
            if (bExact == 0) {
                pfnMatch = ListBox_CompareStringPrefix;
            }
        }
        int nIndex = nFirst;
        for (ListBoxItem* pItem = ListBox_GetItemByIndex(pData, nFirst); pItem != NULL; pItem = pItem->pNext) {
            if (pfnMatch(pItem->szText, szFind)) {
                if (nIndex != -1) {
                    return nIndex;
                }
                break;
            }
            nIndex++;
        }
        if (nStart > -1) {
            ListBoxItem* pItem = pData->pFirst;
            if (pItem != NULL) {
                nIndex = 0;
                do {
                    if (pfnMatch(pItem->szText, szFind)) {
                        return nIndex;
                    }
                    nIndex++;
                } while (nIndex < nFirst && (pItem = pItem->pNext) != NULL);
            }
        }
    }
    return -1;
}

// 1.14d mac: 0002856e
// Changes the selection (-1 clears it), scrolls it into view and invalidates the rows it
// touches; with bNotify and LBS_NOTIFY the parent gets LBN_SELCHANGE.
static BOOL SetSelection(HWND hWnd, ListBoxData* pData, LONG nIndex, BOOL bNotify) {
    if (nIndex < -1) {
        return 0;
    }
    if (pData->nCount <= nIndex) {
        return 0;
    }
    LONG nOld = pData->nSelection;
    if (nOld == nIndex) {
        return 1;
    }
    pData->nSelection = nIndex;
    bool bOldRow = true;
    if (nIndex >= 0) {
        if (pData->nTopIndex <= nIndex) {
            goto visible;
        }
        goto scroll;
    }
    goto oldrow;
visible : {
    RECT rcClient;
    GetClientRect(hWnd, &rcClient);
    LONG nVisible = (rcClient.bottom - rcClient.top) / pData->nItemHeight - 1;
    if (nIndex <= pData->nTopIndex + nVisible) {
        LISTBOX_InvalidateItem(hWnd, pData, nIndex);
        goto oldrow;
    }
    nIndex = nIndex - nVisible;
}
scroll:
    pData->nTopIndex = nIndex;
    InvalidateRect(hWnd, NULL, 1);
    InvalidateRect(hWnd, NULL, 1);
    bOldRow = false;
oldrow:
    if (bOldRow && nOld > -1) {
        LISTBOX_InvalidateItem(hWnd, pData, nOld);
    }
    if (bNotify && (pData->dwStyle & LBS_NOTIFY)) {
        HWND hParent = GetParent(hWnd);
        UINT uID = GetWindowLong(hWnd, GWL_ID);
        SendMessage(hParent, WM_COMMAND, (uID & 0xFFFF) | (LBN_SELCHANGE << 16), (LPARAM)hWnd);
    }
    return 1;
}

// 1.14d mac: 00028681
// Invalidates an item's row if it is in the visible range.
static void LISTBOX_InvalidateItem(HWND hWnd, ListBoxData* pData, LONG nIndex) {
    if (pData->nCount > 0) {
        LONG nRow = nIndex - pData->nTopIndex;
        if (nRow > -1) {
            RECT rcClient;
            GetClientRect(hWnd, &rcClient);
            if (nRow < (rcClient.bottom - rcClient.top) / pData->nItemHeight) {
                RECT rcRow;
                GetClientRect(hWnd, &rcRow);
                rcRow.top = nRow * pData->nItemHeight + rcRow.top;
                rcRow.bottom = rcRow.top + pData->nItemHeight;
                InvalidateRect(hWnd, &rcRow, 1);
            }
        }
    }
}

// 1.14d mac: 000286f8
// Unlinks the item and fixes the count, the selection and the top index; the remaining
// count, or -1.
static int LISTBOX_DeleteString(HWND hWnd, ListBoxData* pData, LONG nIndex) {
    int nResult = -1;
    if (pData != NULL && nIndex > -1 && nIndex < pData->nCount) {
        LONG nCount = pData->nCount;
        ListBoxItem* pItem = ListBox_GetItemByIndex(pData, nIndex);
        if (pItem != NULL) {
            if (pItem == pData->pFirst) {
                pData->pFirst = pItem->pNext;
            }
            if (pItem == pData->pLast) {
                pData->pLast = pItem->pPrev;
            }
            if (pItem->pPrev != NULL) {
                pItem->pPrev->pNext = pItem->pNext;
            }
            if (pItem->pNext != NULL) {
                pItem->pNext->pPrev = pItem->pPrev;
            }
            nCount--;
            pData->nCount = nCount;
            nResult = pData->nSelection;
            if (nResult == nIndex) {
                pData->nSelection = -1;
            } else if (nIndex < nResult) {
                pData->nSelection = nResult - 1;
            }
            if (nCount == 0) {
                pData->nTopIndex = -1;
                nResult = 0;
            } else {
                RECT rcClient;
                GetClientRect(hWnd, &rcClient);
                LONG nTop = pData->nTopIndex;
                if (nTop < 1) {
                    nResult = pData->nCount;
                } else {
                    nResult = pData->nCount;
                    if (nResult - 1 < nTop - 1 + (rcClient.bottom - rcClient.top) / pData->nItemHeight) {
                        pData->nTopIndex = nTop;
                    }
                }
            }
        }
    }
    return nResult;
}

// 1.14d mac: 000287d5
// Links a new item at the index (-1 appends) and returns that index, or -1. The item's text
// is a copy; an owner-drawn list without strings keeps the data value itself.
static int LISTBOX_InsertString(ListBoxData* pData, LONG nIndex, const void* pText, size_t nLength) {
    int nResult = -1;
    if (pData != NULL && nIndex > -2) {
        if (nIndex < pData->nCount) {
            ListBoxItem* pPrev;
            if (nIndex == -1) {
                pPrev = pData->pLast;
            } else {
                pPrev = NULL;
                if (nIndex > 0) {
                    pPrev = ListBox_GetItemByIndex(pData, nIndex - 1);
                }
            }
            ListBoxItem* pItem;
            if ((pData->dwStyle & (LBS_HASSTRINGS | LBS_OWNERDRAWFIXED)) == LBS_OWNERDRAWFIXED) {
                pItem = (ListBoxItem*)Priv_Alloc(0x18);
                if (pItem != NULL) {
                    pItem->szText = (char*)pText;
                    pItem->dwData = (DWORD)pText;
                    pItem->nLength = 0;
                }
            } else {
                pItem = (ListBoxItem*)Priv_Alloc(nLength + 0x14);
                if (pItem != NULL) {
                    pItem->dwData = 0;
                    pItem->szText = pItem->szBuffer;
                    pItem->nLength = nLength;
                    memcpy(pItem->szBuffer, pText, nLength);
                }
            }
            nResult = -1;
            if (pItem != NULL) {
                if (pPrev == NULL) {
                    pItem->pPrev = NULL;
                    pItem->pNext = pData->pFirst;
                    if (pData->pFirst != NULL) {
                        pData->pFirst->pPrev = pItem;
                    }
                    pData->pFirst = pItem;
                    if (pData->pLast == NULL) {
                        pData->pLast = pItem;
                    }
                } else {
                    pItem->pPrev = pPrev;
                    pItem->pNext = pPrev->pNext;
                    if (pPrev->pNext == NULL) {
                        pData->pLast = pItem;
                    } else {
                        pPrev->pNext->pPrev = pItem;
                    }
                    pPrev->pNext = pItem;
                }
                nResult = pData->nCount;
                pData->nCount = nResult + 1;
                if (nResult == 0) {
                    pData->nTopIndex = 0;
                }
                if (nIndex > -1) {
                    nResult = nIndex;
                    if (nIndex <= pData->nSelection) {
                        pData->nSelection = pData->nSelection + 1;
                    }
                }
            }
        }
    }
    return nResult;
}

// 1.14d mac: 0002890b
// Moves the selection by nDelta, clamped to the list; whether it changed.
static BOOL LISTBOX_MoveSelection(HWND hWnd, ListBoxData* pData, LONG nDelta) {
    LONG nCount = pData->nCount;
    if (nCount > 0) {
        LONG nOld = pData->nSelection;
        LONG nNew = nDelta + nOld;
        if (nOld < 0) {
            nNew = 0;
        }
        LONG nClamped = 0;
        if (nNew > -1) {
            nCount = nCount - 1;
            nClamped = nCount;
            if (nNew <= nCount) {
                nClamped = nNew;
            }
        }
        if (nClamped != nOld) {
            return SetSelection(hWnd, pData, nClamped, 1) != 0;
        }
    }
    return false;
}

// 1.14d mac: 0002895e
// The item under a client Y, clamped to the client area; -1 when past the last item.
static int LISTBOX_GetItemFromY(HWND hWnd, ListBoxData* pData, int nY) {
    int nResult;
    if (pData->nCount < 1) {
        nResult = -1;
    } else {
        RECT rcClient;
        GetClientRect(hWnd, &rcClient);
        if (nY <= rcClient.top) {
            nY = rcClient.top;
        }
        int nLimit = rcClient.bottom - 1;
        if (nY <= nLimit) {
            nLimit = nY;
        }
        int nIndex = (nLimit - rcClient.top) / pData->nItemHeight + pData->nTopIndex;
        nResult = -1;
        if (nIndex <= pData->nCount - 1) {
            nResult = nIndex;
        }
    }
    return nResult;
}

#endif  // D2_OS_MAC
