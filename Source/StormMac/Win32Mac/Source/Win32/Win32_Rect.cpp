// Storm's Win32-on-Mac layer: the RECT functions, 1.14d Intel Mac 0002b1fe..0002b500.
// The original file is unknown: no binary string names it. The run sits between
// Win32_Process.cpp (ends with WSOCK_CloseSocket, 0002b1f5) and the resource code.
#include "Win32Text.h"

#if !defined(_WIN32) && D2_OS_MAC

// 1.14d mac: 0002b1fe
// Clamps the point into the rectangle, right and bottom inclusive; true when it moved.
bool Rect_PinPtInRect(RECT* pRect, int* pX, int* pY) {
    int nY = *pY;
    int nX = *pX;
    bool bMoved = true;
    if (pRect->left <= nX) {
        int nRight = pRect->right;
        if (nRight < nX) {
            nX = nRight;
        }
        bMoved = nRight < *pX;
    }
    int nOutY = pRect->top;
    bool bMovedY = true;
    if (pRect->top <= nY) {
        nOutY = nY;
        bMovedY = bMoved;
        if (pRect->bottom < nY) {
            bMovedY = true;
            nOutY = pRect->bottom;
        }
    }
    *pX = nX;
    *pY = nOutY;
    return bMovedY;
}

// 1.14d mac: 0002b25d
BOOL SetRect(RECT* pRect, int nLeft, int nTop, int nRight, int nBottom) {
    if (pRect == NULL) {
        return 0;
    }
    pRect->left = nLeft;
    pRect->top = nTop;
    pRect->right = nRight;
    pRect->bottom = nBottom;
    return 1;
}

// 1.14d mac: 0002b28d
BOOL SetRectEmpty(RECT* pRect) {
    pRect->top = 0;
    pRect->left = 0;
    pRect->bottom = 0;
    pRect->right = 0;
    return 1;
}

// 1.14d mac: 0002b2b5
BOOL CopyRect(RECT* pDest, RECT* pSrc) {
    BOOL bResult = 0;
    if (pDest != NULL && pSrc != NULL) {
        *pDest = *pSrc;
        bResult = 1;
    }
    return bResult;
}

// 1.14d mac: 0002b2e1
// Writes the intersection into pDest (when not NULL); an empty RECT when there is none.
BOOL IntersectRect(RECT* pDest, RECT* pRect1, RECT* pRect2) {
    int nLeft = 0, nTop = 0, nRight = 0, nBottom = 0;
    BOOL bResult = 0;
    if (pRect1 != NULL && pRect2 != NULL) {
        int nLeft1 = pRect1->left, nRight1 = pRect1->right;
        int nTop1 = pRect1->top, nBottom1 = pRect1->bottom;
        int nLeft2 = pRect2->left, nRight2 = pRect2->right;
        int nTop2 = pRect2->top, nBottom2 = pRect2->bottom;
        if (nLeft1 < nRight1 && nTop1 < nBottom1 && nLeft2 < nRight2 && nTop2 < nBottom2) {
            nRight = nRight1 <= nRight2 ? nRight1 : nRight2;
            nLeft = nLeft1 < nLeft2 ? nLeft2 : nLeft1;
            nBottom = nBottom1 <= nBottom2 ? nBottom1 : nBottom2;
            nTop = nTop1 < nTop2 ? nTop2 : nTop1;
            if (nTop < nBottom && nLeft < nRight) {
                bResult = 1;
            } else {
                nLeft = nTop = nRight = nBottom = 0;
            }
        }
    }
    if (pDest != NULL) {
        pDest->left = nLeft;
        pDest->top = nTop;
        pDest->right = nRight;
        pDest->bottom = nBottom;
    }
    return bResult;
}

// 1.14d mac: 0002b378
// The part of pRect1 pRect2 does not cover; trimmed only when the overlap spans a full
// edge of pRect1. Nonzero when the result is not empty.
BOOL SubtractRect(RECT* pDest, RECT* pRect1, RECT* pRect2) {
    BOOL bResult = 0;
    int nLeft, nTop, nRight, nBottom;
    if (pRect1 == NULL || pRect2 == NULL || pRect1->right <= pRect1->left || pRect1->bottom <= pRect1->top) {
        nBottom = 0;
        nRight = 0;
        nTop = 0;
        nLeft = 0;
    } else {
        RECT aInter;
        if (!IntersectRect(&aInter, pRect1, pRect2)) {
            nBottom = pRect1->bottom;
            nRight = pRect1->right;
            nLeft = pRect1->left;
            nTop = pRect1->top;
            bResult = 1;
        } else {
            nLeft = pRect1->left;
            if (aInter.left == pRect1->left && aInter.top == pRect1->top && aInter.right == pRect1->right &&
                aInter.bottom == pRect1->bottom) {
                nBottom = nRight = nTop = nLeft = 0;
                goto done;
            }
            nBottom = pRect1->bottom;
            nTop = pRect1->top;
            nRight = pRect1->right;
            if (aInter.top == nTop && aInter.bottom == nBottom) {
                bResult = 1;
                if (aInter.left == nLeft) {
                    nLeft = aInter.right;
                    goto done;
                }
                if (aInter.right == nRight) {
                    nRight = aInter.left;
                }
            } else if (aInter.left == nLeft && aInter.right == nRight && aInter.top != nTop &&
                       aInter.bottom == nBottom) {
                nBottom = aInter.top;
            }
            bResult = 1;
        }
    }
done:
    if (pDest != NULL) {
        pDest->left = nLeft;
        pDest->top = nTop;
        pDest->right = nRight;
        pDest->bottom = nBottom;
    }
    return bResult;
}

// 1.14d mac: 0002b485
BOOL OffsetRect(RECT* pRect, int nDX, int nDY) {
    if (pRect == NULL) {
        return 0;
    }
    pRect->left += nDX;
    pRect->top += nDY;
    pRect->right += nDX;
    pRect->bottom += nDY;
    return 1;
}

// 1.14d mac: 0002b4ab
BOOL IsRectEmpty(RECT* pRect) {
    BOOL bResult = 1;
    if (pRect != NULL) {
        bool bEmpty = true;
        if (pRect->left < pRect->right) {
            bEmpty = pRect->top >= pRect->bottom;
        }
        bResult = bEmpty;
    }
    return bResult;
}

// 1.14d mac: 0002b4d1
// Right and bottom edges exclusive.
BOOL PtInRect(RECT* pRect, POINT tPoint) {
    if (pRect == NULL) {
        return 0;
    }
    if (tPoint.x < pRect->left || tPoint.x >= pRect->right || tPoint.y < pRect->top) {
        return 0;
    }
    return tPoint.y < pRect->bottom;
}

#endif  // D2_OS_MAC
