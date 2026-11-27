// StormMac Win32 Mac/Source/Win32/Win32_Region.cpp: the layer's own regions, 1.14d Intel Mac
// 00031b0d..00032b50 (NewEmptyStripRgn .. Rgn_AddRectToStripRgn).
//
// A region is a "strip region": a list of rects sorted into horizontal bands, held in one
// heap block, named by a 1-based handle into a Handle-sized table of block pointers. The
// window manager (Win32_Window.cpp) builds visible and update regions out of them.
// The original name of this file is not recorded; it sits between the key-code helpers of
// the window manager and the DirectX shims, see Source/StormMac/README.md.
#include "Win32Mac.h"

#if !defined(_WIN32) && D2_OS_MAC

#include <CoreServices/CoreServices.h>
#include <string.h>

// 1.14d mac: 0001e45c
Handle MemAllocHandle(Size nSize);
// 1.14d mac: 0001e48a
void MemFreeHandle(Handle hMem);

typedef Win32MacRect StripRect;

#define D2_FASTCALL __attribute__((fastcall))

// The region block: bounds, rect count, a lock count the walkers bump, capacity (a multiple
// of 8 rects) and the rects.
struct StripRgn {
    StripRect tBounds;    // 0x00
    int nRects;           // 0x10
    int nLocks;           // 0x14
    int nCapacity;        // 0x18
    StripRect aRects[1];  // 0x1c
};

typedef void (*pfnStripRgnBandProc)(int hRgn, int nTop, int nHeight, StripRect* pBand1, StripRect* pBand1Last,
                                    StripRect* pBand2, StripRect* pBand2Last);

// 1.14d mac: 003f4810
static int s_nRegionSlots;
// 1.14d mac: 003f4814
static Handle s_hRegions;
// 1.14d mac: 003f4818
static int s_hScratchRgn;

int D2_FASTCALL Rgn_NewStripRgn(int nCapacity);
static void Rgn_ResizeStripRgn(int hRgn, int nCapacity);
static void Rgn_CompactStripRgn(int hRgn);
static void Rgn_AddRectToStripRgn(int hRgn, const StripRect* pRect);
static void Rgn_CombineRgns(int hDest, int hSrc1, int hSrc2, pfnStripRgnBandProc pfnBandProc);
static void Rgn_UnionBandProc(int, int, int, StripRect*, StripRect*, StripRect*, StripRect*);
static void Rgn_IntersectBandProc(int, int, int, StripRect*, StripRect*, StripRect*, StripRect*);
static void Rgn_SubtractBandProc(int, int, int, StripRect*, StripRect*, StripRect*, StripRect*);

static inline StripRgn* Rgn_Lock(int hRgn) {
    StripRgn* pRgn = 0;
    if (hRgn != 0) {
        pRgn = ((StripRgn**)*s_hRegions)[hRgn - 1];
        pRgn->nLocks++;
    }
    return pRgn;
}

static inline void Rgn_Unlock(int hRgn) {
    if (hRgn != 0) {
        ((StripRgn**)*s_hRegions)[hRgn - 1]->nLocks--;
    }
}

// 1.14d mac: 00031b0d
int NewEmptyStripRgn(void) {
    return Rgn_NewStripRgn(0);
}

// 1.14d mac: 00031b18
// Finds a free slot in the handle table (creating it, or growing it by 32 slots) and
// allocates a region with room for nCapacity rects. 0 on failure.
int D2_FASTCALL Rgn_NewStripRgn(int nCapacity) {
    if (s_hRegions == 0) {
        s_nRegionSlots = 32;
        s_hRegions = MemAllocHandle(32 * sizeof(void*));
        if (s_hRegions != 0) {
            void** pSlots = (void**)*s_hRegions;
            for (int i = 0; i < 32; i++) {
                pSlots[i] = 0;
            }
        }
    }
    int nSlot;
    int nSlots;
    do {
        nSlots = s_nRegionSlots;
        for (nSlot = 0; nSlot < nSlots && ((void**)*s_hRegions)[nSlot] != 0; nSlot++) {
        }
        if (nSlots == nSlot) {
            SetHandleSize(s_hRegions, nSlots * 4 + 128);
            if (MemError() != 0) {
                return 0;
            }
            nSlots = s_nRegionSlots + 32;
            for (int i = s_nRegionSlots; i < nSlots; i++) {
                ((void**)*s_hRegions)[i] = 0;
            }
            s_nRegionSlots = nSlots;
        }
    } while (nSlots == nSlot);
    StripRgn* pRgn = (StripRgn*)MemAlloc(nCapacity * sizeof(StripRect) + 0x1c, 0x100);
    if (pRgn == 0) {
        return 0;
    }
    pRgn->nCapacity = nCapacity;
    ((void**)*s_hRegions)[nSlot] = pRgn;
    return nSlot + 1;
}

// 1.14d mac: 00031ce9
// Makes a region of the one rect (empty when the rect is), compacted.
void Rgn_SetStripRgnRect(int hRgn, const StripRect* pRect) {
    if (hRgn == 0 || pRect == 0) {
        return;
    }
    if (pRect->right - pRect->left > 0 && pRect->bottom - pRect->top > 0) {
        StripRgn* pRgn = ((StripRgn**)*s_hRegions)[hRgn - 1];
        if (pRgn->nRects == 0) {
            Rgn_ResizeStripRgn(hRgn, 1);
            pRgn = ((StripRgn**)*s_hRegions)[hRgn - 1];
        }
        pRgn->nLocks++;
        pRgn->tBounds = *pRect;
        pRgn->nRects = 1;
        pRgn->aRects[0] = *pRect;
    } else {
        StripRgn* pRgn = ((StripRgn**)*s_hRegions)[hRgn - 1];
        pRgn->nLocks++;
        memset(&pRgn->tBounds, 0, sizeof(StripRect));
        pRgn->nRects = 0;
    }
    ((StripRgn**)*s_hRegions)[hRgn - 1]->nLocks--;
    Rgn_CompactStripRgn(hRgn);
}

// 1.14d mac: 00031dca
void DisposeStripRgn(int hRgn) {
    if (hRgn != 0) {
        void* pMem = ((void**)*s_hRegions)[hRgn - 1];
        if (pMem != 0) {
            MemFree(pMem, 0);
            ((void**)*s_hRegions)[hRgn - 1] = 0;
        }
    }
}

// 1.14d mac: 00031e16
int CountStripRgnRects(int hRgn) {
    if (hRgn == 0) {
        return 0;
    }
    return ((StripRgn**)*s_hRegions)[hRgn - 1]->nRects;
}

// 1.14d mac: 00031e3b
// Copies up to nMaxRects rects out; returns how many.
int GetStripRgnRects(int hRgn, StripRect* pRects, int nMaxRects) {
    int nCopied = 0;
    bool bValid = (pRects != 0) & (hRgn != 0);
    if (nMaxRects > 0 && bValid) {
        StripRgn* pRgn = Rgn_Lock(hRgn);
        int nCount = pRgn->nRects;
        if (nCount > 0) {
            for (nCopied = 0; nCopied < nMaxRects && nCopied < nCount; nCopied++) {
                pRects[nCopied] = pRgn->aRects[nCopied];
            }
        }
        Rgn_Unlock(hRgn);
    }
    return nCopied;
}

// 1.14d mac: 00031ed2
void GetStripRgnBoundingRect(int hRgn, StripRect* pRect) {
    if (hRgn != 0 && pRect != 0) {
        StripRgn* pRgn = Rgn_Lock(hRgn);
        *pRect = pRgn->tBounds;
        Rgn_Unlock(hRgn);
    }
}

// 1.14d mac: 00031f17
bool IsStripRgnEmpty(int hRgn) {
    bool bEmpty = true;
    if (hRgn != 0) {
        bEmpty = ((StripRgn**)*s_hRegions)[hRgn - 1]->nRects == 0;
    }
    return bEmpty;
}

// 1.14d mac: 00031f41
// Reallocates the block for nCapacity rects rounded up to a multiple of 8, keeping what
// fits. When the allocation fails the region is freed and its slot cleared.
static void Rgn_ResizeStripRgn(int hRgn, int nCapacity) {
    if (hRgn == 0) {
        return;
    }
    int nBlocks = (nCapacity + 7) / 8;
    int nRounded = nBlocks * 8;
    StripRgn* pOld = ((StripRgn**)*s_hRegions)[hRgn - 1];
    int nOldCapacity = pOld->nCapacity;
    if (nOldCapacity == nRounded) {
        return;
    }
    int nBytes = nBlocks * 128 | 0x1c;
    StripRgn* pNew = (StripRgn*)MemAlloc(nBytes, 0x100);
    if (pNew != 0) {
        size_t nCopy = nOldCapacity * sizeof(StripRect) + 0x1c;
        if ((int)nCopy > nBytes) {
            nCopy = nBytes;
        }
        if ((int)nCopy > 0) {
            memmove(pNew, pOld, nCopy);
        }
    }
    MemFree(pOld, 0);
    ((void**)*s_hRegions)[hRgn - 1] = pNew;
    if (pNew != 0) {
        pNew->nCapacity = nRounded;
    }
}

// 1.14d mac: 00032025
// Shrinks the block when the rect count is below the capacity.
static void Rgn_CompactStripRgn(int hRgn) {
    int* pnCount = (int*)offsetof(StripRgn, nRects);
    int* pnCapacity = (int*)offsetof(StripRgn, nCapacity);
    if (hRgn != 0) {
        StripRgn* pRgn = ((StripRgn**)*s_hRegions)[hRgn - 1];
        pnCount = &pRgn->nRects;
        pnCapacity = &pRgn->nCapacity;
    }
    int nCount = *pnCount;
    int nCapacity = *pnCapacity;
    if (nCapacity > nCount) {
        Rgn_ResizeStripRgn(hRgn, nCount);
    }
}

// 1.14d mac: 00032060
void Rgn_SetEmptyStripRgn(int hRgn) {
    if (hRgn != 0) {
        StripRgn* pRgn = ((StripRgn**)*s_hRegions)[hRgn - 1];
        memset(&pRgn->tBounds, 0, sizeof(StripRect));
        pRgn->nRects = 0;
        Rgn_CompactStripRgn(hRgn);
    }
}

// 1.14d mac: 000320a6
// Copies the bounds and every rect of hSrc into hDest.
void CopyStripRgn(int hDest, int hSrc) {
    if (hDest == 0 || hSrc == 0) {
        return;
    }
    StripRgn* pSrc = Rgn_Lock(hSrc);
    int nCount = pSrc->nRects;
    Rgn_ResizeStripRgn(hDest, nCount);
    StripRgn* pDest = Rgn_Lock(hDest);
    pDest->tBounds = pSrc->tBounds;
    pDest->nRects = nCount;
    for (int i = 0; i < nCount; i++) {
        pDest->aRects[i] = pSrc->aRects[i];
    }
    Rgn_Unlock(hDest);
    Rgn_Unlock(hSrc);
}

// 1.14d mac: 0003215e
void Rgn_UnionStripRgn(int hDest, int hSrc1, int hSrc2) {
    if (hDest == 0 || hSrc1 == 0 || hSrc2 == 0) {
        return;
    }
    if (hSrc1 == hSrc2) {
        CopyStripRgn(hDest, hSrc1);
        return;
    }
    if (((StripRgn**)*s_hRegions)[hSrc1 - 1]->nRects == 0) {
        CopyStripRgn(hDest, hSrc2);
    } else if (((StripRgn**)*s_hRegions)[hSrc2 - 1]->nRects == 0) {
        CopyStripRgn(hDest, hSrc1);
    } else {
        Rgn_CombineRgns(hDest, hSrc1, hSrc2, Rgn_UnionBandProc);
    }
}

// The next rect of a band after pRect, or NULL at its end: the band is the rects that
// share one top, and pLast is the region's last rect.
static inline StripRect* NextInBand(StripRect* pRect, StripRect* pLast) {
    if (pRect == pLast) {
        return 0;
    }
    StripRect* pNext = pRect + 1;
    return pNext->top == pRect->top ? pNext : 0;
}

// The first rect of the band after pRect's, or NULL after the last band.
static inline StripRect* NextBand(StripRect* pRect, StripRect* pLast) {
    LONG nTop = pRect->top;
    StripRect* pNext;
    do {
        pNext = pRect == pLast ? 0 : pRect + 1;
        pRect = pNext;
    } while (pNext != 0 && pNext->top == nTop);
    return pNext;
}

// 1.14d mac: 000321d6
// Walks the bands of two regions top to bottom, calls pfnBandProc for each stretch where
// the pair of bands is constant into a scratch region, merges vertically adjacent bands
// that came out identical and copies the scratch into hDest.
static void Rgn_CombineRgns(int hDest, int hSrc1, int hSrc2, pfnStripRgnBandProc pfnBandProc) {
    if (s_hScratchRgn == 0) {
        s_hScratchRgn = Rgn_NewStripRgn(0);
    } else {
        Rgn_SetEmptyStripRgn(s_hScratchRgn);
    }
    if (s_hScratchRgn == 0) {
        return;
    }
    StripRgn* pRgn1 = Rgn_Lock(hSrc1);
    StripRgn* pRgn2 = Rgn_Lock(hSrc2);
    StripRect* pBand2Last = &pRgn2->aRects[pRgn2->nRects - 1];
    StripRect* pBand1Last = &pRgn1->aRects[pRgn1->nRects - 1];
    StripRect* pBand1 = pRgn1->aRects;
    StripRect* pBand2 = pRgn2->aRects;
    bool bStarted = false;
    int nBottom = hSrc1;
    do {
        // A band of the first region that starts before the second one does.
        while (pBand1 != 0) {
            int nTop = pBand1->top;
            if (nTop < nBottom && bStarted) {
                nTop = nBottom;
            }
            int nEnd;
            if (pBand2 == 0) {
                nBottom = pBand1->bottom;
                nEnd = nBottom;
            } else {
                nEnd = pBand2->top;
                if (nTop >= nEnd) {
                    break;
                }
                nBottom = pBand1->bottom;
                if (nBottom > nEnd) {
                    nBottom = nEnd;
                }
                nEnd = nBottom;
            }
            pfnBandProc(s_hScratchRgn, nTop, nEnd - nTop, pBand1, pBand1Last, 0, pBand2Last);
            break;
        }
        bool bBoth = false;
        StripRect* pNext1 = pBand1;
        if (pBand2 != 0) {
            // A band of the second region that starts before the first one does.
            int nTop = pBand2->top;
            if (nTop < nBottom && bStarted) {
                nTop = nBottom;
            }
            if (pBand1 == 0 || nTop < pBand1->top) {
                int nEnd = pBand2->bottom;
                if (pBand1 != 0 && pBand1->top < nEnd) {
                    nEnd = pBand1->top;
                }
                nBottom = nEnd;
                pfnBandProc(s_hScratchRgn, nTop, nBottom - nTop, 0, pBand1Last, pBand2, pBand2Last);
            }
            if (pBand1 == 0) {
                pNext1 = 0;
            } else {
                // The overlap of the two bands.
                if (pBand2->top < pBand1->bottom) {
                    int nTop1 = pBand1->top;
                    int nBottom2 = pBand2->bottom;
                    if (nTop1 < nBottom2) {
                        int nOverlapTop = pBand2->top <= nTop1 ? nTop1 : pBand2->top;
                        nBottom = nBottom2;
                        if (pBand1->bottom <= nBottom2) {
                            nBottom = pBand1->bottom;
                        }
                        pfnBandProc(s_hScratchRgn, nOverlapTop, nBottom - nOverlapTop, pBand1, pBand1Last, pBand2,
                                    pBand2Last);
                    }
                }
                pNext1 = pBand1;
                if (pBand1->bottom == nBottom) {
                    pNext1 = NextBand(pBand1, pBand1Last);
                }
            }
            bStarted = true;
            pBand1 = pNext1;
            if (pBand2->bottom == nBottom) {
                pBand2 = NextBand(pBand2, pBand2Last);
            }
            (void)bBoth;
        } else {
            if (pBand1 != 0) {
                pNext1 = pBand1;
                if (pBand1->bottom == nBottom) {
                    pNext1 = NextBand(pBand1, pBand1Last);
                }
            }
            bStarted = true;
            pBand1 = pNext1;
        }
    } while (pBand1 != 0 || pBand2 != 0);

    Rgn_Unlock(hSrc1);
    Rgn_Unlock(hSrc2);

    // Merge a band into the one above it when they are the same rects and touch; the
    // merged band's rects are marked empty and squeezed out at the end.
    int hScratch = s_hScratchRgn;
    StripRgn* pScratch = Rgn_Lock(hScratch);
    if (pScratch->nRects > 1) {
        StripRect* pFirst = pScratch->aRects;
        StripRect* pLast = &pScratch->aRects[pScratch->nRects - 1];
        bool bMerged = false;
        StripRect* pPrev = pFirst;
        StripRect* pCur = pFirst;
        for (;;) {
            // The first non-empty rect of the next band.
            LONG nCurTop;
            LONG nCurLeft;
            LONG nCurRight;
            for (;;) {
                for (;;) {
                    pCur = pCur == pLast ? 0 : pCur + 1;
                    if (pCur == 0) {
                        goto compact;
                    }
                    nCurLeft = pCur->left;
                    nCurRight = pCur->right;
                    if (nCurRight != nCurLeft) {
                        break;
                    }
                }
                nCurTop = pCur->top;
                if (nCurTop != pPrev->top) {
                    break;
                }
            }
            // Does it continue the previous band exactly?
            StripRect* pPrevRect;
            StripRect* pCurRect;
            if (nCurTop != pPrev->bottom || nCurLeft != pPrev->left) {
                pPrev = pCur;
                continue;
            }
            pPrevRect = pPrev;
            pCurRect = pCur;
            bool bSame = false;
            for (;;) {
                if (nCurRight != pPrevRect->right) {
                    break;
                }
                StripRect* pCurNext = 0;
                if (pCurRect != pLast) {
                    pCurNext = pCurRect + 1;
                    if (pCurNext->top != pCurRect->top) {
                        pCurNext = 0;
                    }
                }
                if (pPrevRect[1].right == pPrevRect[1].left) {
                    bSame = pCurNext == 0;
                    break;
                }
                StripRect* pPrevNext = pPrevRect + 1;
                if (pPrevRect[1].top != pPrevRect->top) {
                    pPrevNext = 0;
                }
                if (pCurNext == 0 || pPrevNext == 0) {
                    bSame = (pCurNext == 0) == (pPrevNext == 0);
                    break;
                }
                if (pCurNext->left != pPrevNext->left) {
                    break;
                }
                nCurRight = pCurNext->right;
                pPrevRect = pPrevNext;
                pCurRect = pCurNext;
            }
            if (!bSame) {
                pPrev = pCur;
                continue;
            }
            // Same: stretch the previous band over this one.
            LONG nCurBottom = pCur->bottom;
            pPrev->left = nCurLeft;
            pPrev->bottom = nCurBottom;
            pCur->right = pCur->left;
            bMerged = true;
            StripRect* pExtend = pPrev + 1;
            StripRect* pWalk = pCur;
            while (pWalk != pLast) {
                StripRect* pNextRect = pWalk + 1;
                if (pNextRect->top != nCurTop) {
                    break;
                }
                pWalk = pNextRect;
                pExtend->bottom = pExtend->bottom - pWalk->top + pWalk->bottom;
                pWalk->right = pWalk->left;
                pExtend++;
            }
        }
    compact:
        if (bMerged) {
            StripRect* pHole = 0;
            StripRect* pRect = pFirst;
            for (;;) {
                StripRect* pNextHole;
                if (pRect->right == pRect->left) {
                    pNextHole = pHole == 0 ? pRect : pHole;
                } else {
                    pNextHole = 0;
                    if (pHole != 0) {
                        *pHole = *pRect;
                        pNextHole = pHole + 1;
                    }
                }
                bool bEnd = pRect == pLast;
                pRect++;
                pHole = pNextHole;
                if (bEnd) {
                    break;
                }
            }
            if (pHole != 0) {
                pScratch->nRects = (int)(pHole - pFirst);
            }
        }
    }
    Rgn_Unlock(hScratch);
    CopyStripRgn(hDest, hScratch);
}

// 1.14d mac: 00032689
// Band proc: the x-intervals of both bands merged, overlapping and touching ones joined.
static void Rgn_UnionBandProc(int hRgn, int nTop, int nHeight, StripRect* pBand1, StripRect* pBand1Last,
                              StripRect* pBand2, StripRect* pBand2Last) {
    StripRect tRect = {0, 0, 0, 0};
    LONG nLeft = 0;
    LONG nRight = 0;
    for (;;) {
        StripRect* pPick = pBand1;
        if (pBand1 == 0 || (pBand2 != 0 && pBand2->left <= pBand1->left)) {
            pPick = pBand2;
        }
        if (nRight == nLeft || nRight < pPick->left) {
            if (nRight != nLeft && nRight - nLeft >= 0) {
                Rgn_AddRectToStripRgn(hRgn, &tRect);
            }
            nLeft = pPick->left;
            tRect.top = nTop;
            nRight = pPick->right;
            tRect.left = nLeft;
            tRect.right = nRight;
            tRect.bottom = nHeight + nTop;
        } else if (nRight < pPick->right) {
            tRect.top = nTop;
            nRight = pPick->right;
            tRect.left = nLeft;
            tRect.right = nRight;
            tRect.bottom = nHeight + nTop;
        }
        if (pPick == pBand1) {
            if (pBand1 == pBand1Last) {
                pBand1 = 0;
            } else {
                LONG nBandTop = pBand1->top;
                pBand1++;
                if (nBandTop != pBand1->top) {
                    pBand1 = 0;
                }
            }
        } else if (pBand2 == pBand2Last) {
            pBand2 = 0;
        } else {
            LONG nBandTop = pBand2->top;
            pBand2++;
            if (nBandTop != pBand2->top) {
                pBand2 = 0;
            }
        }
        if (pBand1 == 0 && pBand2 == 0) {
            if (nRight != nLeft && nRight - nLeft >= 0) {
                Rgn_AddRectToStripRgn(hRgn, &tRect);
            }
            return;
        }
    }
}

// 1.14d mac: 00032754
void Rgn_IntersectStripRgn(int hDest, int hSrc1, int hSrc2) {
    if (hDest == 0 || hSrc1 == 0 || hSrc2 == 0) {
        return;
    }
    if (hSrc1 == hSrc2) {
        CopyStripRgn(hDest, hSrc1);
        return;
    }
    if (((StripRgn**)*s_hRegions)[hSrc1 - 1]->nRects == 0 || ((StripRgn**)*s_hRegions)[hSrc2 - 1]->nRects == 0) {
        Rgn_SetEmptyStripRgn(hDest);
        return;
    }
    Rgn_CombineRgns(hDest, hSrc1, hSrc2, Rgn_IntersectBandProc);
}

// 1.14d mac: 000327ca
// Band proc: the x-intervals the two bands share.
static void Rgn_IntersectBandProc(int hRgn, int nTop, int nHeight, StripRect* pBand1, StripRect* pBand1Last,
                                  StripRect* pBand2, StripRect* pBand2Last) {
    if (pBand1 == 0 || pBand2 == 0) {
        return;
    }
    StripRect tRect;
    for (;;) {
        LONG nRight1 = pBand1->right;
        for (;;) {
            LONG nRight2 = pBand2->right;
            tRect.right = nRight2;
            if (nRight1 <= nRight2) {
                tRect.right = nRight1;
            }
            tRect.left = pBand2->left;
            if (pBand2->left <= pBand1->left) {
                tRect.left = pBand1->left;
            }
            if (tRect.left < tRect.right) {
                tRect.top = nTop;
                tRect.bottom = nHeight + nTop;
                Rgn_AddRectToStripRgn(hRgn, &tRect);
                nRight2 = pBand2->right;
                nRight1 = pBand1->right;
            }
            if (nRight1 < nRight2) {
                break;
            }
            if (pBand2 == pBand2Last) {
                return;
            }
            LONG nBandTop = pBand2->top;
            pBand2++;
            if (nBandTop != pBand2->top) {
                pBand2 = 0;
            }
            if (pBand2 == 0) {
                return;
            }
        }
        if (pBand1 == pBand1Last) {
            break;
        }
        LONG nBandTop = pBand1->top;
        pBand1++;
        if (nBandTop != pBand1->top) {
            pBand1 = 0;
        }
        if (pBand1 == 0 || pBand2 == 0) {
            return;
        }
    }
}

// 1.14d mac: 00032887
void Rgn_SubtractStripRgn(int hDest, int hSrc1, int hSrc2) {
    if (hDest == 0 || hSrc1 == 0 || hSrc2 == 0) {
        return;
    }
    if (hSrc1 == hSrc2 || ((StripRgn**)*s_hRegions)[hSrc1 - 1]->nRects == 0) {
        Rgn_SetEmptyStripRgn(hDest);
        return;
    }
    if (((StripRgn**)*s_hRegions)[hSrc2 - 1]->nRects == 0) {
        CopyStripRgn(hDest, hSrc1);
        return;
    }
    Rgn_CombineRgns(hDest, hSrc1, hSrc2, Rgn_SubtractBandProc);
}

// 1.14d mac: 000328fd
// Band proc: the parts of the first band's x-intervals the second band does not cover.
static void Rgn_SubtractBandProc(int hRgn, int nTop, int nHeight, StripRect* pBand1, StripRect* pBand1Last,
                                 StripRect* pBand2, StripRect* pBand2Last) {
    if (pBand1 == 0) {
        return;
    }
    StripRect* pPrev = 0;
    StripRect tRect;
    do {
        for (;;) {
            StripRect* pCover = pBand2;
            if (pPrev == 0 || (tRect.left = pPrev->right, pPrev->right <= pBand1->left)) {
                tRect.left = pBand1->left;
            }
            tRect.right = pBand1->right;
            if (pCover != 0 && pCover->left <= tRect.right) {
                tRect.right = pCover->left;
            }
            if (tRect.left < tRect.right) {
                tRect.top = nTop;
                tRect.bottom = nHeight + nTop;
                Rgn_AddRectToStripRgn(hRgn, &tRect);
            }
            pBand2 = 0;
            if (pCover == 0 || (pBand2 = pCover, pBand1->right < pCover->right)) {
                break;
            }
            pBand2 = 0;
            pPrev = pCover;
            if (pCover != pBand2Last) {
                pBand2 = pCover + 1;
                if (pCover->top != pBand2->top) {
                    pBand2 = 0;
                }
            }
        }
        if (pBand1 == pBand1Last) {
            return;
        }
        LONG nBandTop = pBand1->top;
        pBand1++;
        if (nBandTop != pBand1->top) {
            pBand1 = 0;
        }
    } while (pBand1 != 0);
}

// 1.14d mac: 000329b1
void Rgn_DestroyAllRegions(void) {
    Handle hMem = s_hRegions;
    if (s_nRegionSlots > 0) {
        int i = 0;
        int nSlots = s_nRegionSlots;
        do {
            void* pMem = ((void**)*hMem)[i];
            if (pMem != 0) {
                MemFree(pMem, 0);
                ((void**)*s_hRegions)[i] = 0;
                hMem = s_hRegions;
                nSlots = s_nRegionSlots;
            }
            i++;
        } while (i < nSlots);
    }
    MemFreeHandle(hMem);
    s_hRegions = 0;
    s_nRegionSlots = 0;
}

// 1.14d mac: 00032a2d
// Appends a rect (growing the block) and widens the bounds.
static void Rgn_AddRectToStripRgn(int hRgn, const StripRect* pRect) {
    StripRgn* pRgn = Rgn_Lock(hRgn);
    int nCapacity = pRgn->nCapacity;
    if (nCapacity <= pRgn->nRects + 1) {
        if (hRgn == 0) {
            pRgn = 0;
            Rgn_ResizeStripRgn(0, nCapacity + 1);
        } else {
            Rgn_Unlock(hRgn);
            Rgn_ResizeStripRgn(hRgn, nCapacity + 1);
            pRgn = Rgn_Lock(hRgn);
        }
    }
    int nCount = pRgn->nRects;
    if (nCount == 0) {
        pRgn->tBounds = *pRect;
    } else {
        LONG nLeft = pRgn->tBounds.left <= pRect->left ? pRgn->tBounds.left : pRect->left;
        LONG nTop = pRgn->tBounds.top <= pRect->top ? pRgn->tBounds.top : pRect->top;
        LONG nRight = pRgn->tBounds.right >= pRect->right ? pRgn->tBounds.right : pRect->right;
        LONG nBottom = pRgn->tBounds.bottom >= pRect->bottom ? pRgn->tBounds.bottom : pRect->bottom;
        pRgn->tBounds.left = nLeft;
        pRgn->tBounds.top = nTop;
        pRgn->tBounds.right = nRight;
        pRgn->tBounds.bottom = nBottom;
    }
    pRgn->aRects[nCount] = *pRect;
    pRgn->nRects++;
    Rgn_Unlock(hRgn);
}

#endif  // D2_OS_MAC
