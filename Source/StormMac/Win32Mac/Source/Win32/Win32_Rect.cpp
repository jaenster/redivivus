// Storm's Win32-on-Mac layer: the RECT functions, 1.14d Intel Mac 0002b1fe..0002b500.
// The original file is unknown: no binary string names it. The run sits between
// Win32_Process.cpp (ends with WSOCK_CloseSocket, 0002b1f5) and the resource code.
#include "Win32Text.h"

#if !defined(_WIN32) && D2_MAC_CARBON

// 1.14d mac: 0002b1fe
// Clamps the point into the rectangle, right and bottom inclusive; true when it moved.
BOOL Rect_PinPtInRect(RECT* pRect, int* pX, int* pY) {
    int nY = *pY;
    int nOutX = pRect->left;
    BOOL bMovedX = 1;
    int nX = *pX;
    if (nX >= nOutX) {
        int nRight = pRect->right;
        bMovedX = nX > nRight;
        if (nX > nRight) {
            nX = nRight;
        }
        nOutX = nX;
    }
    int nOutY = pRect->top;
    BOOL bMoved = 1;
    if (nY >= nOutY) {
        int nBottom = pRect->bottom;
        bMoved = nY > nBottom ? 1 : bMovedX;
        if (nY > nBottom) {
            nY = nBottom;
        }
        nOutY = nY;
    }
    *pX = nOutX;
    *pY = nOutY;
    return bMoved;
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
BOOL IntersectRect(RECT* pDest, const RECT* pRect1, const RECT* pRect2) {
    int nLeft, nTop, nRight, nBottom;
    BOOL bResult;
    int nLeft1, nRight1, nTop1, nBottom1, nLeft2, nRight2, nTop2, nBottom2;
    if (pRect1 != NULL && pRect2 != NULL) {
        nLeft1 = pRect1->left, nRight1 = pRect1->right;
        if (nLeft1 < nRight1) {
            nTop1 = pRect1->top, nBottom1 = pRect1->bottom;
            if (nTop1 < nBottom1) {
                nLeft2 = pRect2->left, nRight2 = pRect2->right;
                if (nLeft2 < nRight2) {
                    nTop2 = pRect2->top, nBottom2 = pRect2->bottom;
                    if (nTop2 < nBottom2) {
                        nRight = nRight1 <= nRight2 ? nRight1 : nRight2;
                        nLeft = nLeft1 < nLeft2 ? nLeft2 : nLeft1;
                        nBottom = nBottom1 <= nBottom2 ? nBottom1 : nBottom2;
                        nTop = nTop1 < nTop2 ? nTop2 : nTop1;
                        if (nTop < nBottom && nLeft < nRight) {
                            bResult = 1;
                            goto store;
                        }
                    }
                }
            }
        }
    }
    nLeft = nTop = nRight = nBottom = 0;
    bResult = 0;
store:
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
BOOL SubtractRect(RECT* pDest, const RECT* pRect1, const RECT* pRect2) {
    BOOL bResult = 0;
    int nLeft, nTop, nRight, nBottom;
    if (pRect1 == NULL || pRect2 == NULL || pRect1->left >= pRect1->right || pRect1->top >= pRect1->bottom) {
        nBottom = nRight = nTop = nLeft = 0;
    } else {
        RECT aInter;
        if (!IntersectRect(&aInter, pRect1, pRect2)) {
            nBottom = pRect1->bottom;
            nRight = pRect1->right;
            nLeft = pRect1->left;
            nTop = pRect1->top;
            bResult = 1;
        } else if ((nLeft = pRect1->left, aInter.left == nLeft) && aInter.top == pRect1->top &&
                   aInter.right == pRect1->right && aInter.bottom == pRect1->bottom) {
            nBottom = nRight = nTop = nLeft = 0;
        } else {
            nBottom = pRect1->bottom;
            nTop = pRect1->top;
            nRight = pRect1->right;
            bResult = 1;
            if (aInter.top == nTop && aInter.bottom == nBottom) {
                if (aInter.left == nLeft) {
                    nLeft = aInter.right;
                } else if (aInter.right == nRight) {
                    nRight = aInter.left;
                }
            } else if (aInter.left == nLeft && aInter.right == nRight) {
                if (aInter.top == nTop) {
                    nTop = aInter.bottom;
                } else if (aInter.bottom == nBottom) {
                    nBottom = aInter.top;
                }
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
    POINT* pPoint = &tPoint;
    if (pPoint->x < pRect->left || pPoint->x >= pRect->right) {
        return 0;
    }
    if (pPoint->y < pRect->top) {
        return 0;
    }
    return pPoint->y < pRect->bottom;
}

#endif  // D2_OS_MAC
