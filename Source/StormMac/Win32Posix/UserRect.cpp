// Win32Posix: rectangles (Win32Mac: Win32_Rect.cpp). Empty means right <= left or
// bottom <= top; the right and bottom edges are outside, as on Windows.
#include "Source/StormMac/Win32Posix/UserInternal.h"

#if !defined(_WIN32) && D2_BACKEND_POSIX

void SetRect(LPRECT lprc, int left, int top, int right, int bottom) {
    if (lprc) {
        *lprc = {left, top, right, bottom};
    }
}

BOOL SetRectEmpty(LPRECT lprc) {
    if (!lprc) {
        return FALSE;
    }
    *lprc = {0, 0, 0, 0};
    return TRUE;
}

BOOL CopyRect(LPRECT lprcDst, const RECT* lprcSrc) {
    if (!lprcDst || !lprcSrc) {
        return FALSE;
    }
    *lprcDst = *lprcSrc;
    return TRUE;
}

BOOL IsRectEmpty(const RECT* lprc) {
    return !lprc || lprc->right <= lprc->left || lprc->bottom <= lprc->top;
}

BOOL EqualRect(const RECT* lprc1, const RECT* lprc2) {
    return lprc1 && lprc2 && lprc1->left == lprc2->left && lprc1->top == lprc2->top &&
           lprc1->right == lprc2->right && lprc1->bottom == lprc2->bottom;
}

BOOL PtInRect(const RECT* lprc, POINT pt) {
    return lprc && pt.x >= lprc->left && pt.x < lprc->right && pt.y >= lprc->top && pt.y < lprc->bottom;
}

BOOL OffsetRect(LPRECT lprc, int dx, int dy) {
    if (!lprc) {
        return FALSE;
    }
    lprc->left += dx;
    lprc->right += dx;
    lprc->top += dy;
    lprc->bottom += dy;
    return TRUE;
}

BOOL InflateRect(LPRECT lprc, int dx, int dy) {
    if (!lprc) {
        return FALSE;
    }
    lprc->left -= dx;
    lprc->right += dx;
    lprc->top -= dy;
    lprc->bottom += dy;
    return TRUE;
}

BOOL IntersectRect(LPRECT lprcDst, const RECT* lprcSrc1, const RECT* lprcSrc2) {
    if (!lprcDst || !lprcSrc1 || !lprcSrc2) {
        return FALSE;
    }
    RECT rc = {lprcSrc1->left > lprcSrc2->left ? lprcSrc1->left : lprcSrc2->left,
               lprcSrc1->top > lprcSrc2->top ? lprcSrc1->top : lprcSrc2->top,
               lprcSrc1->right < lprcSrc2->right ? lprcSrc1->right : lprcSrc2->right,
               lprcSrc1->bottom < lprcSrc2->bottom ? lprcSrc1->bottom : lprcSrc2->bottom};
    if (IsRectEmpty(&rc)) {
        SetRectEmpty(lprcDst);
        return FALSE;
    }
    *lprcDst = rc;
    return TRUE;
}

BOOL UnionRect(LPRECT lprcDst, const RECT* lprcSrc1, const RECT* lprcSrc2) {
    if (!lprcDst || !lprcSrc1 || !lprcSrc2) {
        return FALSE;
    }
    bool bEmpty1 = IsRectEmpty(lprcSrc1), bEmpty2 = IsRectEmpty(lprcSrc2);
    if (bEmpty1 && bEmpty2) {
        SetRectEmpty(lprcDst);
        return FALSE;
    }
    if (bEmpty1 || bEmpty2) {
        *lprcDst = bEmpty1 ? *lprcSrc2 : *lprcSrc1;
        return TRUE;
    }
    *lprcDst = {lprcSrc1->left < lprcSrc2->left ? lprcSrc1->left : lprcSrc2->left,
                lprcSrc1->top < lprcSrc2->top ? lprcSrc1->top : lprcSrc2->top,
                lprcSrc1->right > lprcSrc2->right ? lprcSrc1->right : lprcSrc2->right,
                lprcSrc1->bottom > lprcSrc2->bottom ? lprcSrc1->bottom : lprcSrc2->bottom};
    return TRUE;
}

#endif  // !defined(_WIN32) && D2_BACKEND_POSIX
