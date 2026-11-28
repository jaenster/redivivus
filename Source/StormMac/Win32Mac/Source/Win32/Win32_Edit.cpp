// Storm's Win32-on-Mac layer: the built-in edit control, 1.14d Intel Mac 0001ed6c..00021690
// (Edit_WndProc and its helpers). Proc_RegisterBuiltInClasses registers Edit_WndProc as
// the class proc; the control keeps a Win32MacEditData under the "__EditData" property.
// The helpers are static functions of the file: clang gives them its fastcc (the window
// in ecx, the edit data in edx) by itself, so every one of them is plain static here.
// The original file is unknown: no 1.14d string names it (see Source/StormMac/README.md).
#include "Win32Edit.h"

#if !defined(_WIN32) && D2_MAC_CARBON

#include <ctype.h>
#include <string.h>

typedef void (*TIMERPROC)(HWND hWnd, UINT uMsg, UINT nIDEvent, DWORD dwTime);

#define WM_SETFOCUS 0x0007
#define WM_KILLFOCUS 0x0008
#define WM_COMMAND 0x0111
#define EDIT_CARET_TIMER 0x496E5074
#define EDIT_DRAG_TIMER 0x44726167

void* GetProp(HWND hWnd, const char* szName);
BOOL SetProp(HWND hWnd, const char* szName, int nData);
void* RemoveProp(HWND hWnd, const char* szName);
LONG DefWindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
LONG SendMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
HWND GetParent(HWND hWnd);
LONG GetWindowLong(HWND hWnd, int nIndex);
UINT SetTimer(HWND hWnd, UINT nIDEvent, UINT uElapse, TIMERPROC pfnTimerFunc);
BOOL KillTimer(HWND hWnd, UINT nIDEvent);
BOOL InvalidateRect(HWND hWnd, const RECT* pRect, BOOL bErase);
BOOL GetClientRect(HWND hWnd, RECT* pRect);
HWND GetCapture(void);
HWND SetCapture(HWND hWnd);
BOOL ReleaseCapture(void);
HWND SetFocus(HWND hWnd);
HWND GetFocus(void);
HDC BeginPaint(HWND hWnd, PAINTSTRUCT* pPaint);
BOOL EndPaint(HWND hWnd, const PAINTSTRUCT* pPaint);
BOOL MessageBeep(UINT uType);
BOOL Wnd_IsVisibleAndEnabled(HWND hWnd);
DWORD GetSysColor(int nIndex);
DWORD SetTextColor(HDC hDC, DWORD dwColor);
int GetSystemMetrics(int nIndex);
void* GetStockObject(int nIndex);
void GDI_GetFontHeight(HandleHeader** hFont, int* pnHeight);
HDC GetDC(HWND hWnd);
int ReleaseDC(HWND hWnd, HDC hDC);
BOOL GetTextExtentPoint32(HDC hDC, const char* szText, int nLength, SIZE* pSize);
BOOL GetTextExtentExPointA(HDC hDC, const char* szText, int nLength, int nMaxExtent, int* pnFit, int* pDx,
                           SIZE* pSize);
int Text_CalculateLineBreaks(HDC hDC, const char* szText, int nLength, int nMaxWidth, Win32MacTextLine* pLines,
                             int nMaxLines);
BOOL Rect_PinPtInRect(RECT* pRect, int* pX, int* pY);
short GetKeyState(int nVirtKey);
BOOL CLIPBOARD_SetText(const char* szText, int nLength);
int CLIPBOARD_GetTextSize(void);
int CLIPBOARD_GetText(char* pBuffer, int nBufferSize);
void* Priv_AllocClear(size_t nSize);

static const char s_szEditData[] = "__EditData";

static void Edit_On_EM_SETSEL(HWND hWnd, int nStart, int nEnd);
static int Edit_Scroll(HWND hWnd, Win32MacEditData* pEdit, int nDX, int nDY);
static int Edit_VScrollToLine(HWND hWnd, Win32MacEditData* pEdit, int nLine);
static void Edit_SetSelection(HWND hWnd, Win32MacEditData* pEdit, int nStart, int nEnd);
static int Edit_GetCharFromLineIndexXPos(HWND hWnd, Win32MacEditData* pEdit, int nX, int nLine);
static void Edit_DeleteSelection(HWND hWnd, Win32MacEditData* pEdit);
static void Edit_RecalculateLineOffsets(HWND hWnd, Win32MacEditData* pEdit);
static void Edit_ScrollCaretIntoView(HWND hWnd, Win32MacEditData* pEdit, int nChar);
static int Edit_LineFromChar(HWND hWnd, Win32MacEditData* pEdit, int nChar, int* pnX);
static Win32MacEditData* Edit_CreateEditDataBackup(Win32MacEditData* pEdit);
static int Edit_InsertText(HWND hWnd, Win32MacEditData* pEdit, const char* szText, int nLength);
static BOOL Edit_TextFitsInClient(HWND hWnd, Win32MacEditData* pEdit);
static void Edit_RestoreEditDataBackup(HWND hWnd, Win32MacEditData* pEdit,
                                                   Win32MacEditData* pBackup);
static BOOL Edit_CopySelectionToClipboard(Win32MacEditData* pEdit);

// Frees a backup or a destroyed control's data: its line table, then the block.
static void Edit_FreeData(Win32MacEditData* pEdit) {
    if (pEdit->pLines != NULL) {
        Priv_Free(pEdit->pLines);
    }
    Priv_Free(pEdit);
}

// 1.14d mac: 0001ed6c
LONG Edit_WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    LONG lResult;
    Win32MacEditData* pEdit = (Win32MacEditData*)GetProp(hWnd, s_szEditData);
    if (pEdit == NULL && uMsg != 1) {
        return DefWindowProc(hWnd, uMsg, wParam, lParam);
    }

    switch (uMsg) {
    case 1: {  // WM_CREATE
        if (hWnd == NULL || lParam == 0) {
            return -1;
        }
        Win32MacEditData* pNew = (Win32MacEditData*)Priv_Alloc(sizeof(Win32MacEditData));
        if (pNew == NULL) {
            RemoveProp(hWnd, s_szEditData);
            return -1;
        }
        DWORD dwStyle = ((DWORD*)lParam)[8];
        pNew->dwStyle = dwStyle;
        pNew->nLimit = 30000;
        pNew->nSelCaret = 0;
        pNew->bCaretOn = 0;
        pNew->nSelAnchor = 0;
        pNew->nLength = 0;
        pNew->nScrollX = 0;
        pNew->bChanged = 0;
        pNew->nDragX = 0;
        pNew->n20 = 0;
        pNew->nDragY = 0;
        pNew->nTopLine = 0;
        pNew->nLines = 0;
        pNew->nLineHeight = 0;
        pNew->pLines = NULL;
        pNew->pText = NULL;
        pNew->nCapacity = (dwStyle & ES_MULTILINE) ? 0x1000 : 0x100;
        pNew->pText = (char*)Priv_AllocClear(pNew->nCapacity);
        if (pNew->pText == NULL) {
            RemoveProp(hWnd, s_szEditData);
            lResult = -1;
        } else {
            int* pLine = (int*)Priv_Alloc(sizeof(Win32MacTextLine));
            pNew->pLines = (Win32MacTextLine*)pLine;
            if (pLine == NULL) {
                RemoveProp(hWnd, s_szEditData);
                lResult = -1;
            } else {
                pLine[1] = 0;
                pLine[0] = 0;
                pLine[3] = 0;
                pLine[2] = 0;
                pLine[4] = 0;
                pNew->nLines = 1;
                BOOL bSet = SetProp(hWnd, s_szEditData, (int)(intptr_t)pNew);
                lResult = bSet ? 0 : -1;
                if (bSet) {
                    return lResult;
                }
                RemoveProp(hWnd, s_szEditData);
            }
        }
        if (pNew->pText != NULL) {
            Priv_Free(pNew->pText);
        }
        Edit_FreeData(pNew);
        return lResult;
    }
    case 2: {  // WM_DESTROY
        Win32MacEditData* pOld = (Win32MacEditData*)RemoveProp(hWnd, s_szEditData);
        if (pOld == NULL) {
            return 0;
        }
        if (pOld->pLines != NULL) {
            Priv_Free(pOld->pLines);
        }
        if (pOld->pText != NULL) {
            Priv_Free(pOld->pText);
        }
        Priv_Free(pOld);
        return 0;
    }
    case 7: {  // WM_SETFOCUS
        pEdit = (Win32MacEditData*)GetProp(hWnd, s_szEditData);
        lResult = DefWindowProc(hWnd, WM_SETFOCUS, wParam, lParam);
        DWORD dwID = GetWindowLong(hWnd, -12);
        HWND hParent = GetParent(hWnd);
        SendMessage(hParent, WM_COMMAND, (dwID & 0xFFFF) | 0x1000000, (LPARAM)hWnd);
        if ((pEdit->dwStyle & ES_READONLY) == 0 && pEdit->nSelAnchor == pEdit->nSelCaret) {
            SetTimer(hWnd, EDIT_CARET_TIMER, 500, NULL);
        }
        return lResult;
    }
    case 8: {  // WM_KILLFOCUS
        pEdit = (Win32MacEditData*)GetProp(hWnd, s_szEditData);
        KillTimer(hWnd, EDIT_CARET_TIMER);
        bool bWasOn = pEdit->bCaretOn != 0;
        if (bWasOn) {
            pEdit->bCaretOn = 0;
        }
        if (pEdit->nSelAnchor == pEdit->nSelCaret) {
            if (bWasOn) {
                InvalidateRect(hWnd, NULL, 1);
            }
        } else {
            Edit_SetSelection(hWnd, pEdit, -1, -1);
            InvalidateRect(hWnd, NULL, 1);
        }
        lResult = DefWindowProc(hWnd, WM_KILLFOCUS, wParam, lParam);
        DWORD dwID = GetWindowLong(hWnd, -12);
        HWND hParent = GetParent(hWnd);
        SendMessage(hParent, WM_COMMAND, (dwID & 0xFFFF) | 0x2000000, (LPARAM)hWnd);
        return lResult;
    }
    case 0xC: {  // WM_SETTEXT
        pEdit = (Win32MacEditData*)GetProp(hWnd, s_szEditData);
        pEdit->pText[0] = 0;
        pEdit->nLength = 0;
        Edit_SetSelection(hWnd, pEdit, 0, 0);
        pEdit->bChanged = 1;
        int nLength = strlen((const char*)lParam);
        lResult = Edit_InsertText(hWnd, pEdit, (const char*)lParam, nLength);
        if ((pEdit->dwStyle & ES_MULTILINE) != 0) {
            Edit_SetSelection(hWnd, pEdit, 0, 0);
        }
        Edit_RecalculateLineOffsets(hWnd, pEdit);
        Edit_ScrollCaretIntoView(hWnd, pEdit, pEdit->nSelAnchor);
        InvalidateRect(hWnd, NULL, 1);
        return lResult;
    }
    case 0xD: {  // WM_GETTEXT
        pEdit = (Win32MacEditData*)GetProp(hWnd, s_szEditData);
        lResult = 0;
        if (pEdit->pText != NULL) {
            lResult = wParam - 1;
            if (pEdit->nLength <= (int)(wParam - 1)) {
                lResult = pEdit->nLength;
            }
            memcpy((void*)lParam, pEdit->pText, lResult);
            ((char*)lParam)[lResult] = 0;
        }
        return lResult;
    }
    case 0xE:  // WM_GETTEXTLENGTH
        pEdit = (Win32MacEditData*)GetProp(hWnd, s_szEditData);
        lResult = 0;
        if (pEdit->pText != NULL) {
            lResult = pEdit->nLength;
        }
        return lResult;
    case 0xF: {  // WM_PAINT
        pEdit = (Win32MacEditData*)GetProp(hWnd, s_szEditData);
        PAINTSTRUCT tPaint;
        HDC hDC = BeginPaint(hWnd, &tPaint);
        char* pText = NULL;
        bool bHaveText = false;
        if ((pEdit->dwStyle & ES_PASSWORD) == 0) {
            pText = pEdit->pText;
            bHaveText = pText != NULL;
        } else {
            size_t nLength = pEdit->nLength;
            pText = (char*)Priv_Alloc(nLength);
            if (pText != NULL) {
                if (nLength != 0) {
                    memset(pText, 0xA5, nLength);
                }
                bHaveText = true;
            }
        }
        if (bHaveText) {
            int nAnchor = pEdit->nSelAnchor;
            int nCaret = pEdit->nSelCaret;
            HWND hFocus = GetFocus();
            int nSelStart = nAnchor;
            if (nCaret < nAnchor) {
                nSelStart = nCaret;
            }
            int nSelEnd = nCaret;
            if (nCaret <= nAnchor) {
                nSelEnd = nAnchor;
            }
            if (hFocus != hWnd) {
                nSelStart = nSelEnd;
            }
            BOOL bEnabled = Wnd_IsVisibleAndEnabled(hWnd);
            DWORD dwOldColor = bEnabled;
            if (!bEnabled) {
                dwOldColor = SetTextColor(hDC, GetSysColor(0x11));
                nSelStart = nSelEnd;
            }
            RECT rcLine;
            RECT rcSeg;
            rcLine.left = -pEdit->nScrollX;
            rcLine.top = 0;
            int nLineHeight = pEdit->nLineHeight;
            int nVisible = 1;
            int nTop = 0;
            if ((pEdit->dwStyle & ES_MULTILINE) != 0) {
                rcLine.bottom = nLineHeight;
                GetClientRect(hWnd, &rcSeg);
                nVisible = (rcSeg.bottom - rcSeg.top) / pEdit->nLineHeight;
                nLineHeight = rcLine.bottom;
                nTop = rcLine.top;
            }
            int nFirst = pEdit->nTopLine;
            int nCount = pEdit->nLines - nFirst;
            if (nVisible <= nCount) {
                nCount = nVisible;
            }
            rcLine.bottom = (nCount - 1) * pEdit->nLineHeight;
            rcLine.top = nTop + rcLine.bottom;
            rcLine.bottom = rcLine.bottom + nLineHeight;
            int nLast = nFirst - 1 + nCount;
            if (pEdit->nTopLine <= nLast) {
                bool bCaretDrawn = false;
                for (;;) {
                    Win32MacTextLine* pLine = &pEdit->pLines[nLast];
                    int nStart = pLine->nStart;
                    int nLineLen = pLine->nLength;
                    rcLine.right = pLine->nWidth + (-pEdit->nScrollX);
                    const char* pLineText = pText + nStart;
                    int nLineEnd = nLineLen + nStart;
                    bool bDrawnCaret;
                    if (nSelStart < nSelEnd) {
                        SIZE tSize;
                        rcSeg.top = rcLine.top;
                        int nEnd1 = nSelStart;
                        if (nLineEnd <= nSelStart) {
                            nEnd1 = nLineEnd;
                        }
                        int nLen1 = nEnd1 - nStart;
                        int nX = 0;
                        rcSeg.bottom = rcLine.bottom;
                        if (nLen1 != 0 && nStart <= nEnd1) {
                            GetTextExtentPoint32(hDC, pLineText, nLen1, &tSize);
                            rcSeg.left = -pEdit->nScrollX;
                            rcSeg.right = -pEdit->nScrollX + tSize.cx;
                            GDI_DrawLine(hDC, &rcSeg, NULL, pLineText, nLen1, 0, 0xFFFFFFFF, 0xFFFFFFFF);
                            nX = tSize.cx;
                        }
                        int nEnd2 = nSelEnd;
                        if (nLineEnd <= nSelEnd) {
                            nEnd2 = nLineEnd;
                        }
                        int nBegin2 = nSelStart;
                        if (nSelStart < nStart) {
                            nBegin2 = nStart;
                        }
                        if (nEnd2 - nBegin2 != 0 && nBegin2 <= nEnd2) {
                            GetTextExtentPoint32(hDC, pLineText, nEnd2 - nStart, &tSize);
                            rcSeg.left = nX + (-pEdit->nScrollX);
                            rcSeg.right = -pEdit->nScrollX + tSize.cx;
                            GDI_DrawLine(hDC, &rcSeg, NULL, pText + nBegin2, nEnd2 - nBegin2, 1, 0xFFFFFFFF,
                                         0xFFFFFFFF);
                            nX = tSize.cx;
                        }
                        int nBegin3 = nStart;
                        if (nStart <= nSelEnd) {
                            nBegin3 = nSelEnd;
                        }
                        bDrawnCaret = false;
                        if (nLineEnd - nBegin3 != 0 && nBegin3 <= nLineEnd) {
                            GetTextExtentPoint32(hDC, pLineText, nLineLen, &tSize);
                            rcSeg.left = nX + (-pEdit->nScrollX);
                            rcSeg.right = -pEdit->nScrollX + tSize.cx;
                            bDrawnCaret = false;
                            GDI_DrawLine(hDC, &rcSeg, NULL, pText + nBegin3, nLineEnd - nBegin3, 0,
                                         0xFFFFFFFF, 0xFFFFFFFF);
                        }
                    } else {
                        DWORD dwCaret = 0xFFFFFFFF;
                        RECT rcCaret;
                        RECT* pRect = &rcLine;
                        if (nSelStart >= nStart) {
                            if (!bCaretDrawn && pEdit->bCaretOn != 0 && nSelStart <= nLineEnd &&
                                (dwCaret = nSelStart - nStart, nLineEnd == nSelStart)) {
                                rcCaret.left = rcLine.left;
                                rcCaret.top = rcLine.top;
                                rcCaret.right = rcLine.right + 1;
                                rcCaret.bottom = rcLine.bottom;
                                pRect = &rcCaret;
                            }
                        }
                        GDI_DrawLine(hDC, pRect, NULL, pLineText, nLineLen, 0, dwCaret, 0xFFFFFFFF);
                        bDrawnCaret = dwCaret != 0xFFFFFFFF;
                    }
                    rcLine.top -= pEdit->nLineHeight;
                    rcLine.bottom -= pEdit->nLineHeight;
                    nLast--;
                    if (nLast < pEdit->nTopLine) {
                        break;
                    }
                    bCaretDrawn = bCaretDrawn || bDrawnCaret;
                }
            }
            if (pText != pEdit->pText) {
                Priv_Free(pText);
            }
            if (!bEnabled) {
                SetTextColor(hDC, dwOldColor);
            }
        }
        EndPaint(hWnd, &tPaint);
        lResult = 0;
        if (pEdit->bChanged != 0) {
            HWND hParent = GetParent(hWnd);
            if (hParent != NULL) {
                DWORD dwID = GetWindowLong(hWnd, -12);
                SendMessage(hParent, WM_COMMAND, (dwID & 0xFFFF) | 0x3000000, (LPARAM)hWnd);
            }
            pEdit->bChanged = 0;
        }
        return lResult;
    }
    case 0x30: {  // WM_SETFONT
        pEdit = (Win32MacEditData*)GetProp(hWnd, s_szEditData);
        lResult = DefWindowProc(hWnd, 0x30, wParam, lParam);
        if (wParam == 0) {
            wParam = DefWindowProc(hWnd, 0x31, 0, 0);
            if (wParam == 0) {
                wParam = (WPARAM)GetStockObject(0xD);
            }
        }
        LONG nHeight;
        GDI_GetFontHeight((HandleHeader**)wParam, &nHeight);
        pEdit->nLineHeight = nHeight;
        Edit_RecalculateLineOffsets(hWnd, pEdit);
        InvalidateRect(hWnd, NULL, 1);
        return lResult;
    }
    case 0x87:  // WM_GETDLGCODE
        pEdit = (Win32MacEditData*)GetProp(hWnd, s_szEditData);
        return (pEdit->dwStyle & ES_MULTILINE) | 0x89;
    case 0xB0: {  // EM_GETSEL
        pEdit = (Win32MacEditData*)GetProp(hWnd, s_szEditData);
        int nStart = pEdit->nSelAnchor;
        int nEnd = pEdit->nSelCaret;
        int nMax = nEnd;
        if (nEnd <= nStart) {
            nMax = nStart;
        }
        if (nEnd < nStart) {
            nStart = nEnd;
        }
        if (wParam != 0) {
            *(int*)wParam = nStart;
        }
        if (lParam != 0) {
            *(int*)lParam = nMax;
        }
        return nStart & 0xFFFF;
    }
    case 0xB1:  // EM_SETSEL
        Edit_On_EM_SETSEL(hWnd, wParam, lParam);
        return 0;
    case 0xBA:  // EM_GETLINECOUNT
        pEdit = (Win32MacEditData*)GetProp(hWnd, s_szEditData);
        return pEdit->nLines;
    case 0xC2: {  // EM_REPLACESEL
        pEdit = (Win32MacEditData*)GetProp(hWnd, s_szEditData);
        if (pEdit == NULL || lParam == 0 || *(const char*)lParam == '\0') {
            return 0;
        }
        Edit_DeleteSelection(hWnd, pEdit);
        int nLength = strlen((const char*)lParam);
        Edit_InsertText(hWnd, pEdit, (const char*)lParam, nLength);
        Edit_RecalculateLineOffsets(hWnd, pEdit);
        Edit_ScrollCaretIntoView(hWnd, pEdit, pEdit->nSelAnchor);
        InvalidateRect(hWnd, NULL, 1);
        return 0;
    }
    case 0xC5:  // EM_LIMITTEXT
        pEdit = (Win32MacEditData*)GetProp(hWnd, s_szEditData);
        if (pEdit != NULL) {
            if (wParam == 0) {
                wParam = (pEdit->dwStyle << 29) | 0x7FFFFFFF;
            }
            pEdit->nLimit = wParam;
        }
        return 0;
    case 0xCE:  // EM_GETFIRSTVISIBLELINE
        pEdit = (Win32MacEditData*)GetProp(hWnd, s_szEditData);
        return pEdit->nTopLine;
    case 0x100: {  // WM_KEYDOWN
        pEdit = (Win32MacEditData*)GetProp(hWnd, s_szEditData);
        short nShift = GetKeyState(0x10);
        GetKeyState(0x11);
        int nAnchor = pEdit->nSelAnchor;
        int nCaret = pEdit->nSelCaret;
        int nSelMax = nCaret;
        if (nCaret <= nAnchor) {
            nSelMax = nAnchor;
        }
        int nSelMin = nAnchor;
        if (nCaret < nAnchor) {
            nSelMin = nCaret;
        }
        int nUnhandled = -1;
        int nNewAnchor = nAnchor;
        int nNewCaret = nCaret;
        int nLength = pEdit->nLength;
        const char* pText = pEdit->pText;
        if (wParam < 0x21) {
            if (wParam == 8) {  // VK_BACK
                if (nCaret == nAnchor && nCaret > 0) {
                    int nPrev = nAnchor - 1;
                    int nTo = nPrev;
                    if (nPrev > 0 && pText[nAnchor - 2] == '\r' && (nTo = nAnchor - 2, pText[nPrev] != '\n')) {
                        nTo = nPrev;
                    }
                    pEdit->nSelCaret = nTo;
                }
                Edit_DeleteSelection(hWnd, pEdit);
                Edit_RecalculateLineOffsets(hWnd, pEdit);
                int nScrollTo = pEdit->nSelAnchor - 5;
                if (nScrollTo < 0) {
                    nScrollTo = 0;
                }
                Edit_ScrollCaretIntoView(hWnd, pEdit, nScrollTo);
                InvalidateRect(hWnd, NULL, 1);
                return 0;
            }
        } else if (wParam - 0x21 <= 0xD) {
            RECT rcClient;
            int nResult;
            switch (wParam) {
            case 0x21:  // VK_PRIOR
                if ((pEdit->dwStyle & ES_MULTILINE) != 0) {
                    GetClientRect(hWnd, &rcClient);
                    int nPage = (rcClient.bottom - rcClient.top) / pEdit->nLineHeight;
                    int nStep = nPage - 1;
                    int nDelta = -1;
                    if (0 < nStep) {
                        nDelta = 1 - nPage;
                    }
                    nResult = Edit_Scroll(hWnd, pEdit, 0, nDelta);
                    goto scrolled;
                }
                nUnhandled = 0;
                nNewCaret = nCaret;
                break;
            case 0x22:  // VK_NEXT
                if ((pEdit->dwStyle & ES_MULTILINE) != 0) {
                    GetClientRect(hWnd, &rcClient);
                    int nStep = (rcClient.bottom - rcClient.top) / pEdit->nLineHeight - 1;
                    int nDelta = 1;
                    if (0 < nStep) {
                        nDelta = nStep;
                    }
                    nResult = Edit_Scroll(hWnd, pEdit, 0, nDelta);
                    goto scrolled;
                }
                nUnhandled = 0;
                nNewCaret = nCaret;
                break;
            case 0x23:  // VK_END
                if ((pEdit->dwStyle & ES_MULTILINE) != 0) {
                    nResult = Edit_VScrollToLine(hWnd, pEdit, pEdit->nLines);
                    goto scrolled;
                }
                nUnhandled = 0;
                if (nShift < 0) {
                    nNewCaret = nLength;
                } else {
                    if (nCaret == nAnchor) {
                        nSelMax = nLength;
                    }
                    nNewAnchor = nSelMax;
                    nNewCaret = nNewAnchor;
                }
                break;
            case 0x24:  // VK_HOME
                if ((pEdit->dwStyle & ES_MULTILINE) != 0) {
                    nResult = Edit_VScrollToLine(hWnd, pEdit, 0);
                    goto scrolled;
                }
                nUnhandled = 0;
                if (nShift >= 0) {
                    if (nCaret == nAnchor) {
                        nNewAnchor = 0;
                        nNewCaret = 0;
                    } else {
                        nNewAnchor = nSelMin;
                        nNewCaret = nSelMin;
                    }
                } else {
                    nNewCaret = 0;
                }
                break;
            case 0x25:  // VK_LEFT
                if (nShift < 0) {
                    nUnhandled = 0;
                    if (nCaret < 1) {
                        break;
                    }
                    int nPrev = nCaret - 1;
                    if (0 < nPrev) {
                        int nTo = nPrev;
                        if (pText[nCaret - 2] == '\r' && (nTo = nCaret - 2, pText[nPrev] != '\n')) {
                            nTo = nPrev;
                        }
                        nNewCaret = nTo;
                    } else {
                        nNewCaret = nPrev;
                    }
                    break;
                }
                nUnhandled = 0;
                if (nCaret == nAnchor) {
                    if (0 < nAnchor) {
                        int nPrev = nAnchor - 1;
                        if (nPrev < 1 || pText[nAnchor - 2] != '\r') {
                            nNewAnchor = nPrev;
                            nNewCaret = nNewAnchor;
                            break;
                        }
                        nNewAnchor = nAnchor - 2;
                        if (pText[nPrev] != '\n') {
                            nNewAnchor = nPrev;
                        }
                    }
                    nNewCaret = nNewAnchor;
                } else {
                    nNewAnchor = nSelMin;
                    nNewCaret = nSelMin;
                }
                break;
            case 0x26:  // VK_UP
                if ((pEdit->dwStyle & ES_MULTILINE) == 0) {
                    nUnhandled = 0;
                    if (nShift >= 0) {
                        if (nCaret == nAnchor) {
                            nNewAnchor = 0;
                            nNewCaret = 0;
                        } else {
                            nNewAnchor = nSelMin;
                            nNewCaret = nSelMin;
                        }
                    } else {
                        nNewCaret = 0;
                    }
                    break;
                }
                {
                    int nX;
                    int nLine = Edit_LineFromChar(hWnd, pEdit, nCaret, &nX);
                    nUnhandled = 0;
                    int nTarget = 0;
                    if (0 < nLine) {
                        nTarget = Edit_GetCharFromLineIndexXPos(hWnd, pEdit, nX, nLine - 1);
                    }
                    nNewCaret = nTarget;
                    if (nShift >= 0) {
                        if (nCaret == nAnchor) {
                            nNewAnchor = nTarget;
                        } else {
                            nNewAnchor = nSelMin;
                            nNewCaret = nSelMin;
                        }
                    }
                }
                break;
            case 0x27:  // VK_RIGHT
                if (nShift >= 0) {
                    nUnhandled = 0;
                    if (nCaret == nAnchor) {
                        if (nLength <= nAnchor) {
                            nNewCaret = nNewAnchor;
                            break;
                        }
                        int nNext = nAnchor + 1;
                        if (nNext < nLength && pText[nAnchor] == '\r' && pText[nAnchor + 1] == '\n') {
                            nNext = nAnchor + 2;
                        }
                        nNewAnchor = nNext;
                        nNewCaret = nNewAnchor;
                    } else {
                        nNewAnchor = nSelMax;
                        nNewCaret = nSelMax;
                    }
                    break;
                }
                nUnhandled = 0;
                if (nCaret < nLength) {
                    int nNext = nCaret + 1;
                    if (nLength <= nNext) {
                        nNewCaret = nNext;
                    } else {
                        nNewCaret = nNext;
                        if (pText[nCaret] == '\r' && pText[nCaret + 1] == '\n') {
                            nNewCaret = nCaret + 2;
                        }
                    }
                }
                break;
            case 0x28:  // VK_DOWN
                if ((pEdit->dwStyle & ES_MULTILINE) == 0) {
                    nUnhandled = 0;
                    if (nShift < 0) {
                        nNewCaret = nLength;
                    } else {
                        if (nCaret == nAnchor) {
                            nSelMax = nLength;
                        }
                        nNewAnchor = nSelMax;
                        nNewCaret = nNewAnchor;
                    }
                    break;
                }
                {
                    int nX;
                    int nLine = Edit_LineFromChar(hWnd, pEdit, nCaret, &nX);
                    int nTarget = nLength;
                    if (nLine < pEdit->nLines - 1) {
                        nTarget = Edit_GetCharFromLineIndexXPos(hWnd, pEdit, nX, nLine + 1);
                    }
                    nNewCaret = nTarget;
                    nUnhandled = 0;
                    if (nShift >= 0) {
                        if (nCaret == nAnchor) {
                            nSelMax = nNewCaret;
                        }
                        nNewAnchor = nSelMax;
                        nNewCaret = nNewAnchor;
                    }
                }
                break;
            case 0x2E:  // VK_DELETE
                if (nCaret == nAnchor && nCaret < nLength) {
                    int nNext = nAnchor + 1;
                    if (nNext < nLength && pText[nAnchor] == '\r' && pText[nAnchor + 1] == '\n') {
                        nNext = nAnchor + 2;
                    }
                    pEdit->nSelCaret = nNext;
                }
                Edit_DeleteSelection(hWnd, pEdit);
                Edit_RecalculateLineOffsets(hWnd, pEdit);
                Edit_ScrollCaretIntoView(hWnd, pEdit, pEdit->nSelAnchor);
                InvalidateRect(hWnd, NULL, 1);
                return 0;
            default:
                break;
            scrolled:
                if (nResult == 0) {
                    return 0;
                }
                InvalidateRect(hWnd, NULL, 1);
                return 0;
            }
        }
        if (nNewAnchor != pEdit->nSelAnchor || nNewCaret != pEdit->nSelCaret) {
            Edit_SetSelection(hWnd, pEdit, nNewAnchor, nNewCaret);
            Edit_ScrollCaretIntoView(hWnd, pEdit, nNewCaret);
            InvalidateRect(hWnd, NULL, 1);
        }
        lResult = 0;
        if (nUnhandled != 0) {
            lResult = DefWindowProc(hWnd, uMsg, wParam, lParam);
        }
        return lResult;
    }
    case 0x102: {  // WM_CHAR
        pEdit = (Win32MacEditData*)GetProp(hWnd, s_szEditData);
        short nCtrl = GetKeyState(0x11);
        short nWin = GetKeyState(0x5B);
        char cKey = (char)wParam;
        if ((short)(nWin | nCtrl) >= 0) {
            DWORD dwStyle = pEdit->dwStyle;
            lResult = (unsigned short)(nWin | nCtrl);
            if ((dwStyle & ES_READONLY) == 0) {
                int nChar = cKey;
                if (isprint(nChar) || ((dwStyle & ES_WANTRETURN) != 0 && nChar == 0xD)) {
                    Win32MacEditData* pBackup = Edit_CreateEditDataBackup(pEdit);
                    lResult = (LONG)pBackup;
                    if (pBackup != NULL) {
                        char szKeyChars[2];
                        int nKeyLength = 1;
                        if (nChar == 0xD) {
                            szKeyChars[1] = 10;
                            nKeyLength = 2;
                        }
                        szKeyChars[0] = cKey;
                        Edit_DeleteSelection(hWnd, pEdit);
                        Edit_InsertText(hWnd, pEdit, szKeyChars, nKeyLength);
                        Edit_RecalculateLineOffsets(hWnd, pEdit);
                        if (!Edit_TextFitsInClient(hWnd, pEdit)) {
                            MessageBeep(0x10);
                            Edit_RestoreEditDataBackup(hWnd, pEdit, pBackup);
                            lResult = -1;
                        } else {
                            Edit_ScrollCaretIntoView(hWnd, pEdit, pEdit->nSelAnchor);
                            InvalidateRect(hWnd, NULL, 1);
                            lResult = 0;
                        }
                        if (pBackup->pLines != NULL) {
                            Priv_Free(pBackup->pLines);
                        }
                        Priv_Free(pBackup);
                    }
                }
            }
            return lResult;
        }
        lResult = cKey;
        if (cKey == 0x41 || cKey == 0x61) {
            lResult = 0;
            Edit_On_EM_SETSEL(hWnd, 0, -1);
            return lResult;
        }
        if (cKey == 0x43 || cKey == 0x63) {
            return SendMessage(hWnd, 0x301, 0, 0);
        }
        if (cKey == 0x56 || cKey == 0x76) {
            return SendMessage(hWnd, 0x302, 0, 0);
        }
        if (cKey == 0x58 || cKey == 0x78) {
            return SendMessage(hWnd, 0x300, 0, 0);
        }
        return lResult;
    }
    case 0x113: {  // WM_TIMER
        pEdit = (Win32MacEditData*)GetProp(hWnd, s_szEditData);
        if (wParam == EDIT_DRAG_TIMER) {
            if (GetCapture() != hWnd) {
                return 0;
            }
            if (Edit_Scroll(hWnd, pEdit, pEdit->nDragX, pEdit->nDragY) == 0) {
                return 0;
            }
        } else {
            if (wParam != EDIT_CARET_TIMER) {
                return -1;
            }
            if (pEdit->nSelAnchor != pEdit->nSelCaret) {
                return 0;
            }
            pEdit->bCaretOn = pEdit->bCaretOn == 0;
        }
        InvalidateRect(hWnd, NULL, 1);
        return 0;
    }
    case 0x115: {  // WM_VSCROLL
        pEdit = (Win32MacEditData*)GetProp(hWnd, s_szEditData);
        if ((pEdit->dwStyle & ES_MULTILINE) == 0) {
            return -1;
        }
        UINT uCode = wParam & 0xFFFF;
        if (uCode > 8) {
            return 0;
        }
        int nResult;
        switch (uCode) {
        case 0:
            nResult = Edit_Scroll(hWnd, pEdit, 0, -1);
            break;
        case 1:
            nResult = Edit_Scroll(hWnd, pEdit, 0, 1);
            break;
        case 4:
        case 5:
        case 8:
            nResult = Edit_VScrollToLine(hWnd, pEdit, wParam >> 16);
            break;
        default:
            return 0;
        }
        if (nResult == 0) {
            return 0;
        }
        InvalidateRect(hWnd, NULL, 1);
        return 0;
    }
    case 0x200: {  // WM_MOUSEMOVE
        pEdit = (Win32MacEditData*)GetProp(hWnd, s_szEditData);
        if (GetCapture() != hWnd) {
            return 0;
        }
        pEdit->nDragX = 0;
        pEdit->nDragY = 0;
        RECT rcClient;
        GetClientRect(hWnd, &rcClient);
        int nX = (short)lParam;
        int nY = lParam >> 16;
        int nDirY = -1;
        int nDirX = -1;
        if (rcClient.left <= nX) {
            nDirX = rcClient.right <= nX;
        }
        if (rcClient.top <= nY) {
            nDirY = rcClient.bottom <= nY;
        }
        int nPos;
        if ((pEdit->dwStyle & ES_MULTILINE) == 0) {
            nPos = 0;
            if (nDirY >= 0) {
                if (nDirY < 1) {
                    if (nDirX == 0) {
                        nPos = Edit_GetCharFromLineIndexXPos(hWnd, pEdit, nX, 0);
                    } else {
                        pEdit->nDragX = nDirX;
                        nPos = pEdit->nSelCaret;
                    }
                } else {
                    nPos = pEdit->nLength;
                }
            }
            if (nPos == pEdit->nSelCaret) {
                return 0;
            }
            Edit_SetSelection(hWnd, pEdit, -1, nPos);
        } else {
            if (nDirY != 0) {
                pEdit->nDragY = nDirY;
                return 0;
            }
            if (nDirX < 0) {
                rcClient.left = rcClient.left + 1;
                nX = rcClient.left;
            } else if (0 < nDirX) {
                nX = rcClient.right - 2;
            }
            int nLine = 0;
            if (1 < pEdit->nLines) {
                int nPinY = nY;
                int nPinX = nX;
                RECT rcPin;
                GetClientRect(hWnd, &rcPin);
                Rect_PinPtInRect(&rcPin, &nPinX, &nPinY);
                int nWanted = nPinY / pEdit->nLineHeight + pEdit->nTopLine;
                nLine = pEdit->nLines - 1;
                if (nWanted <= pEdit->nLines - 1) {
                    nLine = nWanted;
                }
            }
            nPos = Edit_GetCharFromLineIndexXPos(hWnd, pEdit, nX, nLine);
            if (nPos == pEdit->nSelCaret) {
                return 0;
            }
            Edit_SetSelection(hWnd, pEdit, -1, nPos);
        }
        InvalidateRect(hWnd, NULL, 1);
        return 0;
    }
    case 0x201: {  // WM_LBUTTONDOWN
        pEdit = (Win32MacEditData*)GetProp(hWnd, s_szEditData);
        SetFocus(hWnd);
        SetCapture(hWnd);
        pEdit->nDragX = 0;
        pEdit->nDragY = 0;
        int nX = (short)lParam;
        int nY = lParam >> 16;
        int nLine = 0;
        if ((pEdit->dwStyle & ES_MULTILINE) != 0 && 1 < pEdit->nLines) {
            int nPinY = nY;
            int nPinX = nX;
            RECT rcPin;
            GetClientRect(hWnd, &rcPin);
            Rect_PinPtInRect(&rcPin, &nPinX, &nPinY);
            int nWanted = nPinY / pEdit->nLineHeight + pEdit->nTopLine;
            nLine = pEdit->nLines - 1;
            if (nWanted <= pEdit->nLines - 1) {
                nLine = nWanted;
            }
        }
        int nPos = Edit_GetCharFromLineIndexXPos(hWnd, pEdit, nX, nLine);
        int nStart = nPos;
        if ((wParam & 4) != 0) {
            nStart = -1;
        }
        Edit_SetSelection(hWnd, pEdit, nStart, nPos);
        InvalidateRect(hWnd, NULL, 1);
        SetTimer(hWnd, EDIT_DRAG_TIMER, 0x32, NULL);
        return 0;
    }
    case 0x202:  // WM_LBUTTONUP
        pEdit = (Win32MacEditData*)GetProp(hWnd, s_szEditData);
        KillTimer(hWnd, EDIT_DRAG_TIMER);
        if (GetCapture() == hWnd) {
            ReleaseCapture();
        }
        return 0;
    case 0x203:   // WM_LBUTTONDBLCLK
    case 0x3A6:
        Edit_On_EM_SETSEL(hWnd, 0, -1);
        return 0;
    case 0x300: {  // WM_CUT
        pEdit = (Win32MacEditData*)GetProp(hWnd, s_szEditData);
        lResult = 0;
        if (Edit_CopySelectionToClipboard(pEdit)) {
            Edit_DeleteSelection(hWnd, pEdit);
            Edit_RecalculateLineOffsets(hWnd, pEdit);
            Edit_ScrollCaretIntoView(hWnd, pEdit, pEdit->nSelAnchor);
            InvalidateRect(hWnd, NULL, 1);
            lResult = 1;
        }
        return lResult;
    }
    case 0x301:  // WM_COPY
        pEdit = (Win32MacEditData*)GetProp(hWnd, s_szEditData);
        return Edit_CopySelectionToClipboard(pEdit) != 0;
    case 0x302: {  // WM_PASTE
        pEdit = (Win32MacEditData*)GetProp(hWnd, s_szEditData);
        int nSize = CLIPBOARD_GetTextSize();
        if (nSize < 1) {
            return 0;
        }
        char* pBuffer = (char*)Priv_Alloc(nSize);
        if (pBuffer == NULL) {
            return 0;
        }
        Win32MacEditData* pBackup = Edit_CreateEditDataBackup(pEdit);
        if (pBackup == NULL) {
            return 0;
        }
        CLIPBOARD_GetText(pBuffer, nSize);
        Edit_DeleteSelection(hWnd, pEdit);
        int nLength = strlen(pBuffer);
        Edit_InsertText(hWnd, pEdit, pBuffer, nLength);
        Edit_RecalculateLineOffsets(hWnd, pEdit);
        if (!Edit_TextFitsInClient(hWnd, pEdit)) {
            MessageBeep(0x10);
            Edit_RestoreEditDataBackup(hWnd, pEdit, pBackup);
            lResult = 0;
        } else {
            Edit_ScrollCaretIntoView(hWnd, pEdit, pEdit->nSelAnchor);
            InvalidateRect(hWnd, NULL, 1);
            lResult = 1;
        }
        Edit_FreeData(pBackup);
        return lResult;
    }
    case 0x303:  // WM_CLEAR
        pEdit = (Win32MacEditData*)GetProp(hWnd, s_szEditData);
        if (pEdit->nSelAnchor == pEdit->nSelCaret) {
            return 1;
        }
        Edit_DeleteSelection(hWnd, pEdit);
        Edit_RecalculateLineOffsets(hWnd, pEdit);
        Edit_ScrollCaretIntoView(hWnd, pEdit, pEdit->nSelAnchor);
        InvalidateRect(hWnd, NULL, 1);
        return 1;
    default:
        return DefWindowProc(hWnd, uMsg, wParam, lParam);
    }
}

// 1.14d mac: 00020a74
// EM_SETSEL: start -1 clears the selection, 0..-1 selects everything.
static void Edit_On_EM_SETSEL(HWND hWnd, int nStart, int nEnd) {
    Win32MacEditData* pEdit = (Win32MacEditData*)GetProp(hWnd, s_szEditData);
    if (nStart == -1) {
        nStart = 0;
        nEnd = 0;
    } else if (nStart == 0 && nEnd == -1) {
        nEnd = pEdit->nLength;
        nStart = 0;
    }
    Edit_SetSelection(hWnd, pEdit, nStart, nEnd);
    Edit_RecalculateLineOffsets(hWnd, pEdit);
    Edit_ScrollCaretIntoView(hWnd, pEdit, pEdit->nSelAnchor);
    InvalidateRect(hWnd, NULL, 1);
}

// 1.14d mac: 00020aff
// Scrolls a single-line control sideways by 4 pixels in the direction of nDX (the selection
// end follows) and a multiline one by nDY lines; non-zero when something moved.
static int Edit_Scroll(HWND hWnd, Win32MacEditData* pEdit, int nDX, int nDY) {
    int bScrolled = 0;
    DWORD dwStyle;
    if (nDX != 0 && ((dwStyle = pEdit->dwStyle) & (ES_MULTILINE | ES_AUTOHSCROLL)) == ES_AUTOHSCROLL) {
        int nScrollX = pEdit->nScrollX;
        int nPos;
        RECT rcClient;
        if (nDX < 1) {
            nScrollX -= 4;
            if (nScrollX < 0) {
                nScrollX = 0;
            }
            int nLine = 0;
            nPos = 0;
            if ((dwStyle & ES_MULTILINE) != 0 && 1 < pEdit->nLines) {
                int nY = 0;
                int nX = -4;
                RECT rcPin;
                GetClientRect(hWnd, &rcPin);
                Rect_PinPtInRect(&rcPin, &nX, &nY);
                nPos = nY / pEdit->nLineHeight + pEdit->nTopLine;
                nLine = pEdit->nLines - 1;
                if (nPos <= pEdit->nLines - 1) {
                    nLine = nPos;
                }
            }
            nPos = Edit_GetCharFromLineIndexXPos(hWnd, pEdit, -4, nLine);
        } else {
            GetClientRect(hWnd, &rcClient);
            int nEdge = rcClient.right + 4;
            int nLine = 0;
            if ((pEdit->dwStyle & ES_MULTILINE) != 0 && 1 < pEdit->nLines) {
                int nY = 0;
                int nX = nEdge;
                RECT rcPin;
                GetClientRect(hWnd, &rcPin);
                Rect_PinPtInRect(&rcPin, &nX, &nY);
                int nWanted = nY / pEdit->nLineHeight + pEdit->nTopLine;
                nLine = pEdit->nLines - 1;
                if (nWanted <= pEdit->nLines - 1) {
                    nLine = nWanted;
                }
            }
            nScrollX += 4;
            nPos = Edit_GetCharFromLineIndexXPos(hWnd, pEdit, nEdge, nLine);
            if (nPos == pEdit->nLength) {
                nLine = 0;
                if ((pEdit->dwStyle & ES_MULTILINE) != 0 && 1 < pEdit->nLines) {
                    int nY = 0;
                    int nX = rcClient.right;
                    RECT rcPin;
                    GetClientRect(hWnd, &rcPin);
                    Rect_PinPtInRect(&rcPin, &nX, &nY);
                    int nWanted = nY / pEdit->nLineHeight + pEdit->nTopLine;
                    nLine = pEdit->nLines - 1;
                    if (nWanted <= pEdit->nLines - 1) {
                        nLine = nWanted;
                    }
                }
                int nOther = Edit_GetCharFromLineIndexXPos(hWnd, pEdit, rcClient.right, nLine);
                if (nPos == nOther) {
                    nScrollX = pEdit->nScrollX;
                }
            }
        }
        if (nScrollX != pEdit->nScrollX || nPos != pEdit->nSelCaret) {
            pEdit->nScrollX = nScrollX;
            Edit_SetSelection(hWnd, pEdit, -1, nPos);
            bScrolled = 1;
        }
    }
    if (nDY != 0 && (pEdit->dwStyle & ES_MULTILINE) != 0) {
        bScrolled = Edit_VScrollToLine(hWnd, pEdit, nDY + pEdit->nTopLine);
    }
    return bScrolled;
}

// 1.14d mac: 00020d0a
// Makes nLine (clamped) the top line; tells the parent (EN_VSCROLL) when it changed.
static int Edit_VScrollToLine(HWND hWnd, Win32MacEditData* pEdit, int nLine) {
    int bChanged = 0;
    int nClamped = 0;
    if (-1 < nLine) {
        nClamped = pEdit->nLines - 1;
        if (nLine < nClamped) {
            nClamped = nLine;
        }
    }
    if (nClamped != pEdit->nTopLine) {
        pEdit->nTopLine = nClamped;
        HWND hParent = GetParent(hWnd);
        bChanged = 1;
        if (hParent != NULL) {
            DWORD dwID = GetWindowLong(hWnd, -12);
            SendMessage(hParent, WM_COMMAND, (dwID & 0xFFFF) | 0x6020000, (LPARAM)hWnd);
        }
    }
    return bChanged;
}

// 1.14d mac: 00020d81
// -1 for the start keeps the current one, -1 for the end puts it on the start; both are
// clamped to the text. Restarts the caret blink when the selection became empty or not.
static void Edit_SetSelection(HWND hWnd, Win32MacEditData* pEdit, int nStart, int nEnd) {
    if (nStart == -1) {
        nStart = pEdit->nSelAnchor;
    }
    if (nEnd == -1) {
        nEnd = nStart;
    }
    int nLength = pEdit->nLength;
    if (nLength < nEnd) {
        nEnd = nLength;
    }
    pEdit->nSelCaret = nEnd;
    if (nLength < nStart) {
        nStart = nLength;
    }
    pEdit->nSelAnchor = nStart;
    if ((pEdit->dwStyle & ES_READONLY) == 0) {
        if (GetFocus() == hWnd && pEdit->bCaretOn != (int)(nEnd == nStart)) {
            pEdit->bCaretOn = nEnd == nStart;
            KillTimer(hWnd, EDIT_CARET_TIMER);
            if (nEnd == nStart) {
                SetTimer(hWnd, EDIT_CARET_TIMER, 500, NULL);
            }
        }
    }
}

// 1.14d mac: 00020e18
// The character on the line nearest to the x position (the scroll offset added).
static int Edit_GetCharFromLineIndexXPos(HWND hWnd, Win32MacEditData* pEdit, int nX, int nLine) {
    int nLineStart = pEdit->pLines[nLine].nStart;
    int nLength = pEdit->pLines[nLine].nLength;
    int* pDx = (int*)Priv_Alloc(nLength * 4 + 4);
    int nResult = 0;
    if (pDx != NULL) {
        HDC hDC = GetDC(hWnd);
        nResult = 0;
        if (hDC != NULL) {
            SIZE tSize;
            GetTextExtentExPointA(hDC, pEdit->pText + nLineStart, nLength, 0, NULL, pDx, &tSize);
            pDx[nLength] = tSize.cx;
            int nTarget = nX + pEdit->nScrollX;
            nResult = nLength;
            if (nTarget < tSize.cx) {
                int nIndex = 0;
                int nCurrent;
                int nNextDx;
                do {
                    nCurrent = nIndex;
                    nResult = -1;
                    if (nLength <= nCurrent) {
                        goto done;
                    }
                    nNextDx = pDx[nCurrent + 1];
                    nIndex = nCurrent + 1;
                } while (nNextDx < nTarget);
                nResult = nCurrent;
                if (-1 < nCurrent && nNextDx - nTarget < nTarget - pDx[nCurrent]) {
                    nResult = nCurrent + 1;
                }
            }
        done:
            nResult += nLineStart;
            ReleaseDC(hWnd, hDC);
        }
        Priv_Free(pDx);
    }
    return nResult;
}

// 1.14d mac: 00020f10
static void Edit_DeleteSelection(HWND hWnd, Win32MacEditData* pEdit) {
    int nSelMin = pEdit->nSelAnchor;
    int nSelEnd = pEdit->nSelCaret;
    int nSelMax = nSelEnd;
    if (nSelEnd <= nSelMin) {
        nSelMax = nSelMin;
    }
    if (nSelEnd < nSelMin) {
        nSelMin = nSelEnd;
    }
    if (nSelMin < nSelMax) {
        memmove(pEdit->pText + nSelMin, pEdit->pText + nSelMax, (1 - nSelMax) + pEdit->nLength);
        pEdit->nLength = pEdit->nLength + (nSelMin - nSelMax);
        Edit_SetSelection(hWnd, pEdit, nSelMin, nSelMin);
        pEdit->bChanged = 1;
    }
}

// 1.14d mac: 00020f78
// Rebuilds the line table: one entry for a single-line control (with its pixel width),
// the wrapped lines for a multiline one.
static void Edit_RecalculateLineOffsets(HWND hWnd, Win32MacEditData* pEdit) {
    HDC hDC = GetDC(hWnd);
    if (hDC != NULL) {
        if (pEdit->pLines != NULL) {
            Priv_Free(pEdit->pLines);
            pEdit->pLines = NULL;
            pEdit->nLines = 0;
        }
        if ((pEdit->dwStyle & ES_MULTILINE) == 0) {
            SIZE tSize;
            Win32MacTextLine* pLine = (Win32MacTextLine*)Priv_Alloc(sizeof(Win32MacTextLine));
            pEdit->pLines = pLine;
            if (pLine != NULL) {
                GetTextExtentPoint32(hDC, pEdit->pText, pEdit->nLength, &tSize);
                pLine = pEdit->pLines;
                pLine->nStart = 0;
                int nLength = pEdit->nLength;
                pLine->nVisibleLength = nLength;
                pLine->nVisibleWidth = tSize.cx;
                pLine->nLength = nLength;
                pLine->nWidth = tSize.cx;
                pEdit->nLines = 1;
            }
        } else {
            RECT rcClient;
            GetClientRect(hWnd, &rcClient);
            int nMaxLines = Text_CalculateLineBreaks(hDC, pEdit->pText, pEdit->nLength,
                                                     rcClient.right - rcClient.left, NULL, 0);
            Win32MacTextLine* pLines = (Win32MacTextLine*)Priv_Alloc(nMaxLines * sizeof(Win32MacTextLine));
            if (pLines != NULL) {
                pEdit->nLines = Text_CalculateLineBreaks(hDC, pEdit->pText, pEdit->nLength,
                                                         rcClient.right - rcClient.left, pLines, nMaxLines);
                pEdit->pLines = pLines;
            }
        }
        ReleaseDC(hWnd, hDC);
    }
}

// 1.14d mac: 000210bd
// Brings the character into view: vertically for a multiline control, by the text extent
// for a single-line autoscrolling one.
static void Edit_ScrollCaretIntoView(HWND hWnd, Win32MacEditData* pEdit, int nChar) {
    DWORD dwStyle = pEdit->dwStyle;
    if ((dwStyle & (ES_MULTILINE | ES_AUTOVSCROLL)) == (ES_MULTILINE | ES_AUTOVSCROLL)) {
        int nLine = Edit_LineFromChar(hWnd, pEdit, nChar, NULL);
        int nTop = pEdit->nTopLine;
        int nTarget = nLine;
        int nArg = nLine;
        if (nTop <= nLine) {
            int nVisible = 1;
            RECT rcClient;
            if ((pEdit->dwStyle & ES_MULTILINE) != 0) {
                GetClientRect(hWnd, &rcClient);
                nVisible = (rcClient.bottom - rcClient.top) / pEdit->nLineHeight;
                nTop = pEdit->nTopLine;
            }
            nTarget = (nLine - nVisible) + -1;
            nArg = nVisible;
            if (nTarget <= nTop) {
                return;
            }
        }
        Edit_VScrollToLine(hWnd, pEdit, nTarget);
        return;
    }
    if ((dwStyle & ES_AUTOHSCROLL) == 0) {
        return;
    }
    int* pDx = (int*)Priv_Alloc(pEdit->nLength << 2);
    if (pDx == NULL) {
        return;
    }
    HDC hDC = GetDC(hWnd);
    if (hDC != NULL) {
        int nScrollX = pEdit->nScrollX;
        RECT rcClient;
        SIZE tSize;
        GetClientRect(hWnd, &rcClient);
        GetTextExtentExPointA(hDC, pEdit->pText, pEdit->nLength, 0, NULL, pDx, &tSize);
        int nX = tSize.cx;
        if (nChar < pEdit->nLength) {
            nX = pDx[nChar];
        }
        if (nX < nScrollX) {
            pEdit->nScrollX = nX;
        } else if ((rcClient.right - rcClient.left) - 1 + nScrollX < nX) {
            nX = nX - ((rcClient.right - rcClient.left) - 1);
            pEdit->nScrollX = nX;
        }
        ReleaseDC(hWnd, hDC);
    }
    Priv_Free(pDx);
}

// 1.14d mac: 0002120d
// The line a character is on; with pnX the extent of the line's text before it.
static int Edit_LineFromChar(HWND hWnd, Win32MacEditData* pEdit, int nChar, int* pnX) {
    int nLine;
    int nLines = pEdit->nLines;
    if ((pEdit->dwStyle & ES_MULTILINE) == 0 || nLines < 2) {
        nLine = 0;
    } else {
        Win32MacTextLine* pLines = pEdit->pLines;
        nLine = nLines - 1;
        if (nChar < pLines[nLine].nStart) {
            nLine = -1;
            do {
                pLines++;
                if (nLines <= nLine + 2) {
                    nLine = 0;
                    break;
                }
                nLine++;
            } while (pLines->nStart <= nChar);
        }
    }
    if (pnX != NULL) {
        SIZE tSize;
        HDC hDC = GetDC(hWnd);
        int nStart = pEdit->pLines[nLine].nStart;
        GetTextExtentPoint32(hDC, pEdit->pText + nStart, nChar - nStart, &tSize);
        ReleaseDC(hWnd, hDC);
        *pnX = tSize.cx;
    }
    return nLine;
}

// 1.14d mac: 000212b7
// A copy of the header, the line table and the text, in one block with the text last.
static Win32MacEditData* Edit_CreateEditDataBackup(Win32MacEditData* pEdit) {
    Win32MacEditData* pBackup = (Win32MacEditData*)Priv_Alloc(pEdit->nCapacity + sizeof(Win32MacEditData));
    if (pBackup != NULL) {
        Win32MacTextLine* pLines = (Win32MacTextLine*)Priv_Alloc(pEdit->nLines * sizeof(Win32MacTextLine));
        if (pLines == NULL) {
            Priv_Free(pBackup);
            pBackup = NULL;
        } else {
            *pBackup = *pEdit;
            memcpy(pLines, pEdit->pLines, pEdit->nLines * sizeof(Win32MacTextLine));
            pBackup->pLines = pLines;
            pBackup->pText = (char*)(pBackup + 1);
            memcpy(pBackup + 1, pEdit->pText, pEdit->nCapacity);
        }
    }
    return pBackup;
}

// 1.14d mac: 00021343
// Inserts at the start of the selection, cut to the control's limit, growing the text
// block as needed; the caret goes after it. 1 when anything went in.
static int Edit_InsertText(HWND hWnd, Win32MacEditData* pEdit, const char* szText, int nLength) {
    int nPos = pEdit->nSelAnchor;
    if (pEdit->nSelCaret < pEdit->nSelAnchor) {
        nPos = pEdit->nSelCaret;
    }
    int nOld = pEdit->nLength;
    if (pEdit->nLimit < nOld + nLength) {
        nLength = pEdit->nLimit - (nOld + nLength) + nLength;
        if (nLength < 0) {
            nLength = 0;
        }
    }
    int nResult = 0;
    if (0 < nLength) {
        int nEnd = nLength + nPos;
        int nNeeded = nOld + 1 + nLength;
        char* pText;
        if (pEdit->nCapacity < nNeeded) {
            pText = (char*)Priv_Alloc(nNeeded);
            if (pText == NULL) {
                return 0;
            }
            memcpy(pText, pEdit->pText, pEdit->nCapacity);
            Priv_Free(pEdit->pText);
            pEdit->pText = pText;
            pEdit->nCapacity = nNeeded;
        } else {
            pText = pEdit->pText;
        }
        memmove(pText + nEnd, pText + nPos, (nOld + 1) - nPos);
        memcpy(pEdit->pText + nPos, szText, nLength);
        pEdit->nLength = pEdit->nLength + nLength;
        Edit_SetSelection(hWnd, pEdit, nEnd, nEnd);
        pEdit->bChanged = 1;
        nResult = 1;
    }
    return nResult;
}

// 1.14d mac: 00021443
// Single line: the text is narrower than the client; multiline: the lines fit its height.
static BOOL Edit_TextFitsInClient(HWND hWnd, Win32MacEditData* pEdit) {
    DWORD dwStyle = pEdit->dwStyle;
    BOOL bFits = 1;
    if ((dwStyle & ES_MULTILINE) == 0) {
        if ((dwStyle & ES_AUTOHSCROLL) == 0) {
            HDC hDC = GetDC(hWnd);
            if (hDC != NULL) {
                RECT rcClient;
                SIZE tSize;
                GetClientRect(hWnd, &rcClient);
                GetTextExtentPoint32(hDC, pEdit->pText, pEdit->nLength, &tSize);
                bFits = tSize.cx < rcClient.right - rcClient.left;
                ReleaseDC(hWnd, hDC);
            }
        }
    } else if ((dwStyle & ES_AUTOVSCROLL) == 0) {
        RECT rcClient;
        int nLines = pEdit->nLines;
        GetClientRect(hWnd, &rcClient);
        bFits = nLines <= (rcClient.bottom - rcClient.top) / pEdit->nLineHeight;
    }
    return bFits;
}

// 1.14d mac: 000214f2
// Puts a backup back, resizing the line table and text block when they differ, and
// restarts the blink.
static void Edit_RestoreEditDataBackup(HWND hWnd, Win32MacEditData* pEdit,
                                                   Win32MacEditData* pBackup) {
    char* pText = pEdit->pText;
    int nCapacity = pEdit->nCapacity;
    int nLines = pEdit->nLines;
    Win32MacTextLine* pLines = pEdit->pLines;
    if (nLines != pBackup->nLines) {
        Priv_Free(pLines);
        nLines = pBackup->nLines;
        pLines = (Win32MacTextLine*)Priv_Alloc(nLines * sizeof(Win32MacTextLine));
    }
    if (nCapacity < pBackup->nCapacity) {
        Priv_Free(pText);
        nCapacity = pBackup->nCapacity;
        pText = (char*)Priv_Alloc(nCapacity);
    }
    memcpy(pEdit, pBackup, offsetof(Win32MacEditData, pText));
    pEdit->nLines = nLines;
    pEdit->pLines = pLines;
    pEdit->nCapacity = nCapacity;
    pEdit->pText = pText;
    memcpy(pLines, pBackup->pLines, pBackup->nLines * sizeof(Win32MacTextLine));
    memcpy(pEdit->pText, pBackup->pText, pBackup->nCapacity);
    KillTimer(hWnd, EDIT_CARET_TIMER);
    KillTimer(hWnd, EDIT_DRAG_TIMER);
    if ((pEdit->dwStyle & ES_READONLY) == 0 && pEdit->nSelAnchor == pEdit->nSelCaret) {
        SetTimer(hWnd, EDIT_CARET_TIMER, 500, NULL);
    }
}

// 1.14d mac: 0002160c
// Puts the selection on the scrap; a password control puts bullets (0xA5) instead.
static BOOL Edit_CopySelectionToClipboard(Win32MacEditData* pEdit) {
    if (pEdit == NULL) {
        return 0;
    }
    int nSelStart = pEdit->nSelAnchor;
    int nSelEnd = pEdit->nSelCaret;
    int nMin = nSelStart;
    if (nSelEnd < nSelStart) {
        nMin = nSelEnd;
    }
    int nMax = nSelEnd;
    if (nSelEnd <= nSelStart) {
        nMax = nSelStart;
    }
    int nSize = nMax - nMin;
    if (0 < nSize) {
        char* pText = pEdit->pText;
        char* szBullets;
        if ((pEdit->dwStyle & ES_PASSWORD) == 0 || (szBullets = (char*)Priv_Alloc(nSize)) == NULL) {
            CLIPBOARD_SetText(pText + nMin, nSize);
        } else {
            int nHigh = nSelStart;
            if (nSelStart <= nSelEnd) {
                nHigh = nSelEnd;
            }
            int nLow = ~nSelEnd;
            if (~nSelEnd < ~nSelStart) {
                nLow = ~nSelStart;
            }
            memset(szBullets, 0xA5, nHigh + 1 + nLow);
            CLIPBOARD_SetText(szBullets, nSize);
            Priv_Free(szBullets);
        }
        return 1;
    }
    return 0;
}

#endif  // D2_OS_MAC
