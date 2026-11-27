// StormMac, the display and cursor driver table, 1.14d Intel Mac 00036830..000371f9
// (MACCURSOR_ShowDisplayCursor .. MACDISPLAY_FindBestDisplayMode).
//
// The original file name is not recorded: no 1.14d string names it. The functions are
// the slots of one driver table (g_tDefaultDriver, the only driver there is) plus the
// registration calls the renderer module makes against it (D2OpenGL imports
// MACCURSOR_ShowDisplayCursor, MACCURSOR_HideDisplayCursor, MACCURSOR_RestoreCursor,
// MACCURSOR_SetCursorProcs, MACCURSOR_InitCursor and MACDISPLAY_FindBestDisplayMode by
// these names). See Source/StormMac/README.md.
#include "Win32Mac/Source/Win32/Win32Mac.h"

#if !defined(_WIN32) && D2_OS_MAC

#include <ApplicationServices/ApplicationServices.h>
#include <CoreFoundation/CoreFoundation.h>
#include <Carbon/Carbon.h>
#include <string.h>

// 1.14d mac: 00029a30
void Priv_Free(void* pMem);

extern "C" void InitCursor(void);

void MACCURSOR_ShowDisplayCursor(void);
void MACCURSOR_HideDisplayCursor(void);

typedef void (*CursorProc)(void);
typedef bool (*SuspendProc)(void);
typedef bool (*CursorPosProc)(int32_t nX, int32_t nY);
typedef bool (*GuardProc)(bool bFlag);
typedef bool (*GetCursorPosProc)(int32_t*, int32_t*);

struct DisplayRect {
    int32_t aEdge[4];
};

// The mode record the renderer hands to Init. The byte at 0x11 is the caps Init reports
// back; the dword at 0x10 is read after it is written.
struct DisplayModeInfo {
    DisplayRect tMode;  // 0x00
    union {
        int32_t nDepthAndCaps;  // 0x10
        struct {
            int8_t cDepth;  // 0x10
            uint8_t bCaps;  // 0x11
        };
    };
};

struct DisplayBounds {
    DisplayRect tRect;  // 0x00
    int32_t nDepth;     // 0x10
    int32_t nFlag;      // 0x14
};

struct DisplayDriver {
    int32_t nUnused0;                                                // 0x00
    int32_t nUnused4;                                                // 0x04
    DisplayRect tMode;                                               // 0x08
    int32_t nDepth;                                                  // 0x18
    DisplayBounds tBounds;                                           // 0x1c
    bool (*pfnInit)(DisplayModeInfo*, DisplayBounds*, void*, void*); // 0x34
    bool (*pfnShutdown)(void);                                       // 0x38
    int (*pfnSlot3C)(void);                                          // 0x3c
    int (*pfnSlot40)(void);                                          // 0x40
    int (*pfnSlot44)(void);                                          // 0x44
    int (*pfnSlot48)(void);                                          // 0x48
    bool (*pfnGetCursorPos)(int32_t*, int32_t*);                     // 0x4c
    int (*pfnSlot50)(void);                                          // 0x50
    bool (*pfnSlot54)(int32_t, int32_t);                             // 0x54
    int (*pfnSlot58)(void);                                          // 0x58
    bool (*pfnSetCursorPos)(int32_t, int32_t);                       // 0x5c
    int32_t (*pfnHideCursor)(void);                                  // 0x60
    int32_t (*pfnShowCursor)(void);                                  // 0x64
    int32_t (*pfnGetShowCount)(void);                                // 0x68
    int32_t (*pfnRestoreCursor)(void);                               // 0x6c
    bool (*pfnSuspendCursorProcs)(void);                             // 0x70
    bool (*pfnResumeCursorProcs)(void);                              // 0x74
    int (*pfnSlot78)(void);                                          // 0x78
    int (*pfnSlot7C)(void);                                          // 0x7c
    int (*pfnSlot80)(void);                                          // 0x80
    int (*pfnSlot84)(void);                                          // 0x84
    int (*pfnSlot88)(void);                                          // 0x88
    bool (*pfnGetBounds)(DisplayBounds*);                            // 0x8c
    int (*pfnSlot90)(void);                                          // 0x90
    int (*pfnSlot94)(void);                                          // 0x94
    int (*pfnSlot98)(void);                                          // 0x98
    bool (*pfnSetPaletteEntries)(const uint32_t*, int, int);         // 0x9c
    bool (*pfnGetPaletteEntries)(uint32_t*, int, int);               // 0xa0
    void (*pfnSlotA4)(void);                                         // 0xa4
    int (*pfnSlotA8)(void);                                          // 0xa8
    bool (*pfnGetContext)(void**);                                   // 0xac
    void* aReservedB0[4];                                            // 0xb0
    bool (*pfnSetSuspendCallbacks)(SuspendProc, SuspendProc);        // 0xc0
    bool (*pfnSetCursorProcs)(CursorProc, CursorProc, CursorProc);   // 0xc4
    void* aReservedC8[2];                                            // 0xc8
    int (*pfnSlotD0)(void);                                          // 0xd0
    void (*pfnSetGuardFlag)(bool);                                   // 0xd4
    bool (*pfnGetGuardFlag)(void);                                   // 0xd8
};

// The cursor's three procs: InitCursor itself, and the show/hide procs the cursor
// counter drives (CGDisplayShowCursor/HideCursor by default).
// 1.14d mac: 003979d4
CursorProc g_pfnInitCursor = InitCursor;
// 1.14d mac: 003979d8
CursorProc g_pfnShowCursorProc = MACCURSOR_ShowDisplayCursor;
// 1.14d mac: 003979dc
CursorProc g_pfnHideCursorProc = MACCURSOR_HideDisplayCursor;

extern DisplayDriver g_tDefaultDriver;
// 1.14d mac: 00397abc
DisplayDriver* g_pDisplayDriver = &g_tDefaultDriver;

// 1.14d mac: 00554e4c
SuspendProc g_pfnSuspendCallback;
// 1.14d mac: 00554e50
SuspendProc g_pfnResumeCallback;
// 1.14d mac: 003f4950
GuardProc g_pfnGuardProc;
// 1.14d mac: 003f4954
CursorPosProc g_pfnSetCursorPosProc;
// 1.14d mac: 003f4958
GetCursorPosProc g_pfnGetCursorPosProc;
// 1.14d mac: 003f495c
static bool g_bCursorProcsSuspended;
// 1.14d mac: 003f4960
void* g_pCursorContext;
// 1.14d mac: 003f4964
int32_t g_nCursorShowCount;
// 1.14d mac: 003f4968
uint32_t g_aPalette[256];
// 1.14d mac: 003f4d68
static bool g_bGuardFlag;

// 1.14d mac: 00036830
// CGDisplayShowCursor(0): the default show proc.
void MACCURSOR_ShowDisplayCursor(void) {
    CGDisplayShowCursor(0);
}

// 1.14d mac: 00036847
// CGDisplayHideCursor(0): the default hide proc.
void MACCURSOR_HideDisplayCursor(void) {
    CGDisplayHideCursor(0);
}

// 1.14d mac: 0003685e
bool MACDISPLAY_Init(DisplayModeInfo* pModeInfo, DisplayBounds* pBounds, void* pContext, void* pfnCallback) {
    g_bCursorProcsSuspended = false;
    g_pCursorContext = pContext;
    g_nCursorShowCount = 0;
    g_pfnInitCursor();
    memset(g_aPalette, 0, sizeof(g_aPalette));
    DisplayDriver* pDriver;
    if (pBounds != NULL) {
        pDriver = g_pDisplayDriver;
        pDriver->tBounds = *pBounds;
    } else {
        pDriver = g_pDisplayDriver;
        pDriver->tBounds.tRect.aEdge[0] = pModeInfo->tMode.aEdge[0];
        pDriver->tBounds.tRect.aEdge[1] = pModeInfo->tMode.aEdge[1];
        pDriver->tBounds.tRect.aEdge[2] = pModeInfo->tMode.aEdge[2];
        pDriver->tBounds.tRect.aEdge[3] = pModeInfo->tMode.aEdge[3];
        pDriver->tBounds.nDepth = pModeInfo->cDepth;
        pDriver->tBounds.nFlag = 0;
        pDriver = g_pDisplayDriver;
    }
    pModeInfo->bCaps = (pDriver->tBounds.nFlag == 0) | 2;
    pDriver = g_pDisplayDriver;
    pDriver->nDepth = pModeInfo->nDepthAndCaps;
    pDriver->tMode = pModeInfo->tMode;
    return true;
}

// 1.14d mac: 00036938
bool MACDISPLAY_Shutdown(void) {
    g_bCursorProcsSuspended = false;
    g_pCursorContext = NULL;
    g_nCursorShowCount = 0;
    g_pfnInitCursor();
    return true;
}

// 1.14d mac: 0003696f
int MACDISPLAY_Slot3C(void) {
    return 0;
}

// 1.14d mac: 00036976
int MACDISPLAY_Slot40(void) {
    return 1;
}

// 1.14d mac: 00036980
int MACDISPLAY_Slot44(void) {
    return 1;
}

// 1.14d mac: 0003698a
int MACDISPLAY_Slot48(void) {
    return 0;
}

// 1.14d mac: 00036991
// The position comes from the registered proc; no proc, no position.
bool MACCURSOR_GetCursorPos(int32_t* pX, int32_t* pY) {
    if (g_pfnGetCursorPosProc == NULL) {
        return false;
    }
    return g_pfnGetCursorPosProc(pX, pY);
}

// 1.14d mac: 000369c2
int MACDISPLAY_Slot50(void) {
    return 0;
}

// 1.14d mac: 000369c9
bool MACDISPLAY_Slot54(int32_t nA, int32_t nB) {
    if (g_pfnGetCursorPosProc == NULL) {
        return false;
    }
    return ((bool (*)(int32_t, int32_t))g_pfnGetCursorPosProc)(nA, nB);
}

// 1.14d mac: 000369fa
int MACDISPLAY_Slot58(void) {
    return 0;
}

// 1.14d mac: 00036a01
bool MACCURSOR_SetCursorPos(int32_t nX, int32_t nY) {
    if (g_pfnSetCursorPosProc == NULL) {
        return false;
    }
    return g_pfnSetCursorPosProc(nX, nY);
}

// 1.14d mac: 00036a32
// Counts a hide; the hide proc runs on the 0 to -1 step unless the procs are suspended.
int32_t MACCURSOR_HideCursor(void) {
    int32_t nCount = g_nCursorShowCount;
    if (nCount == 0) {
        nCount = 0;
        if (!g_bCursorProcsSuspended) {
            g_pfnHideCursorProc();
            nCount = g_nCursorShowCount;
        }
    }
    g_nCursorShowCount = nCount - 1;
    return nCount - 1;
}

// 1.14d mac: 00036a6c
// ShowCursor semantics: the show proc runs when the count reaches 0.
int32_t MACCURSOR_ShowCursor(void) {
    g_nCursorShowCount++;
    int32_t nCount = g_nCursorShowCount;
    if (g_nCursorShowCount == 0 && !g_bCursorProcsSuspended) {
        g_pfnShowCursorProc();
        nCount = g_nCursorShowCount;
    }
    return nCount;
}

// 1.14d mac: 00036aa4
int32_t MACCURSOR_GetShowCount(void) {
    return g_nCursorShowCount;
}

// 1.14d mac: 00036ab5
int32_t MACCURSOR_RestoreCursor(void) {
    bool bSuspended = g_bCursorProcsSuspended;
    if (!bSuspended) {
        g_pfnInitCursor();
        if (g_nCursorShowCount < 0) {
            g_pfnHideCursorProc();
        }
    }
    return g_nCursorShowCount;
}

// 1.14d mac: 00036aee
bool MACCURSOR_SuspendCursorProcs(void) {
    if (!g_bCursorProcsSuspended) {
        if (g_pfnSuspendCallback != NULL && !g_pfnSuspendCallback()) {
            return false;
        }
        g_pfnInitCursor();
        g_bCursorProcsSuspended = true;
    }
    return true;
}

// 1.14d mac: 00036b2d
bool MACCURSOR_ResumeCursorProcs(void) {
    if (g_bCursorProcsSuspended) {
        if (g_pfnResumeCallback != NULL && !g_pfnResumeCallback()) {
            return false;
        }
        g_bCursorProcsSuspended = false;
        g_pfnInitCursor();
        if (g_nCursorShowCount < 0) {
            g_pfnHideCursorProc();
        }
    }
    return true;
}

// 1.14d mac: 00036b7e
int MACDISPLAY_Slot78(void) {
    return 0;
}

// 1.14d mac: 00036b85
int MACDISPLAY_Slot7C(void) {
    return 0;
}

// 1.14d mac: 00036b8c
int MACDISPLAY_Slot80(void) {
    return 0;
}

// 1.14d mac: 00036b93
int MACDISPLAY_Slot84(void) {
    return 0;
}

// 1.14d mac: 00036b9a
int MACDISPLAY_Slot88(void) {
    return 0;
}

// 1.14d mac: 00036ba1
// The driver's bounds record; true when the bounds flag is set.
bool MACDISPLAY_GetBounds(DisplayBounds* pOut) {
    DisplayDriver* pDriver = g_pDisplayDriver;
    *pOut = pDriver->tBounds;
    return pDriver->tBounds.nFlag != 0;
}

// 1.14d mac: 00036bdc
int MACDISPLAY_Slot90(void) {
    return 1;
}

// 1.14d mac: 00036be6
int MACDISPLAY_Slot94(void) {
    return 0;
}

// 1.14d mac: 00036bed
int MACDISPLAY_Slot98(void) {
    return 0;
}

// 1.14d mac: 00036bf4
bool MACDISPLAY_SetPaletteEntries(const uint32_t* pEntries, int nCount, int nFirst) {
    int nBytes = nCount * 4;
    if (nBytes > 0) {
        memmove(&g_aPalette[nFirst], &pEntries[nFirst], nBytes);
    }
    return true;
}

// 1.14d mac: 00036c36
bool MACDISPLAY_GetPaletteEntries(uint32_t* pEntries, int nCount, int nFirst) {
    int nBytes = nCount * 4;
    if (nBytes > 0) {
        memmove(&pEntries[nFirst], &g_aPalette[nFirst], nBytes);
    }
    return true;
}

// 1.14d mac: 00036c78
void MACDISPLAY_SlotA4(void) {
}

// 1.14d mac: 00036c7d
int MACDISPLAY_SlotA8(void) {
    return 0;
}

// 1.14d mac: 00036c84
bool MACDISPLAY_GetContext(void** ppContext) {
    *ppContext = g_pCursorContext;
    return true;
}

// 1.14d mac: 00036c9f
bool MACCURSOR_SetSuspendCallbacks(SuspendProc pfnSuspend, SuspendProc pfnResume) {
    g_pfnSuspendCallback = pfnSuspend;
    g_pfnResumeCallback = pfnResume;
    return true;
}

// 1.14d mac: 00036cc1
bool MACCURSOR_SetCursorProcs(CursorProc pfnInit, CursorProc pfnShow, CursorProc pfnHide) {
    g_pfnInitCursor = pfnInit;
    g_pfnShowCursorProc = pfnShow;
    g_pfnHideCursorProc = pfnHide;
    return true;
}

// 1.14d mac: 00036cec
int MACDISPLAY_SlotD0(void) {
    return 0;
}

// 1.14d mac: 00036cf3
void MACCURSOR_SetGuardFlag(bool bFlag) {
    g_bGuardFlag = bFlag;
}

// 1.14d mac: 00036d07
bool MACCURSOR_GetGuardFlag(void) {
    return g_bGuardFlag;
}

// The only driver: the slots above, in table order. Init writes into it.
// 1.14d mac: 003979e0
DisplayDriver g_tDefaultDriver = {
    0, 0, {{1, 1, 1, 1}}, 0x108, {{{1, 1, 1, 8}}, 1, 0},
    MACDISPLAY_Init, MACDISPLAY_Shutdown, MACDISPLAY_Slot3C, MACDISPLAY_Slot40, MACDISPLAY_Slot44,
    MACDISPLAY_Slot48, MACCURSOR_GetCursorPos, MACDISPLAY_Slot50, MACDISPLAY_Slot54, MACDISPLAY_Slot58,
    MACCURSOR_SetCursorPos, MACCURSOR_HideCursor, MACCURSOR_ShowCursor, MACCURSOR_GetShowCount,
    MACCURSOR_RestoreCursor, MACCURSOR_SuspendCursorProcs, MACCURSOR_ResumeCursorProcs,
    MACDISPLAY_Slot78, MACDISPLAY_Slot7C, MACDISPLAY_Slot80, MACDISPLAY_Slot84, MACDISPLAY_Slot88,
    MACDISPLAY_GetBounds, MACDISPLAY_Slot90, MACDISPLAY_Slot94, MACDISPLAY_Slot98,
    MACDISPLAY_SetPaletteEntries, MACDISPLAY_GetPaletteEntries, MACDISPLAY_SlotA4, MACDISPLAY_SlotA8,
    MACDISPLAY_GetContext, {0, 0, 0, 0}, MACCURSOR_SetSuspendCallbacks, MACCURSOR_SetCursorProcs, {0, 0},
    MACDISPLAY_SlotD0, MACCURSOR_SetGuardFlag, MACCURSOR_GetGuardFlag,
};

// 1.14d mac: 00036d19
// Mode 0 and 3 are the only drivers there are; both end on the default one.
bool MACDISPLAY_SelectDriver(int nMode) {
    g_pDisplayDriver->pfnShutdown();
    g_pDisplayDriver = &g_tDefaultDriver;
    if (nMode != 3) {
        if (nMode != 0) {
            return false;
        }
        g_pDisplayDriver = &g_tDefaultDriver;
    }
    g_tDefaultDriver.pfnRestoreCursor();
    return true;
}

// 1.14d mac: 00036d5e
bool MACCURSOR_CallGuardProc(bool bFlag) {
    if (g_pfnGuardProc == NULL) {
        return false;
    }
    return g_pfnGuardProc(bFlag);
}

// 1.14d mac: 00036d8b
void MACCURSOR_RegisterGuardProc(GuardProc pfnGuard) {
    g_pfnGuardProc = pfnGuard;
}

// 1.14d mac: 00036d9f
void MACCURSOR_RegisterSetCursorPosProc(CursorPosProc pfnSetCursorPos) {
    g_pfnSetCursorPosProc = pfnSetCursorPos;
}

// 1.14d mac: 00036db3
void MACCURSOR_RegisterGetCursorPosProc(GetCursorPosProc pfnGetCursorPos) {
    g_pfnGetCursorPosProc = pfnGetCursorPos;
}

struct AccelNode {
    AccelNode* pNext;
};

// 1.14d mac: 00036fd3
// Frees a singly linked list of accelerator tables.
void Accel_DeleteAcceleratorList(AccelNode* pList) {
    while (pList != NULL) {
        AccelNode* pNext = pList->pNext;
        Priv_Free(pList);
        pList = pNext;
    }
}

// 1.14d mac: 00036ff5
void MACCURSOR_InitCursor(void) {
    InitCursor();
}

// 1.14d mac: 00036ffe
// The mode with the smallest (area over the request, plus 100 for a stretch mismatch, plus the
// refresh difference) among the modes at least nWidth x nHeight whose pixel encoding names
// nBits characters and holds no digit or 'F'. Without bAcceptClosest only an exact fit returns.
CGDisplayModeRef MACDISPLAY_FindBestDisplayMode(CGDirectDisplayID nDisplay, uint32_t nWidth, uint32_t nHeight,
                                                uint32_t nBits, uint32_t nRefreshHz, uint8_t bStretched,
                                                uint8_t bAcceptClosest) {
    CFMutableCharacterSetRef hSet =
        CFCharacterSetCreateMutableCopy(kCFAllocatorDefault, CFCharacterSetGetPredefined(kCFCharacterSetDecimalDigit));
    CFCharacterSetAddCharactersInString(hSet, CFSTR("F"));
    CFArrayRef hModes = CGDisplayCopyAllDisplayModes(nDisplay, NULL);
    int nBestScore = 0x7fffffff;
    CGDisplayModeRef hBest = NULL;
    if (hModes != NULL) {
        CFIndex nModes = CFArrayGetCount(hModes);
        hBest = NULL;
        if (nModes > 0) {
            int nArea = nHeight * nWidth;
            for (CFIndex i = 0; i != nModes; i++) {
                CGDisplayModeRef hMode = (CGDisplayModeRef)CFArrayGetValueAtIndex(hModes, i);
                CFStringRef hEncoding = CGDisplayModeCopyPixelEncoding(hMode);
                CFIndex nLength = CFStringGetLength(hEncoding);
                CFRange tRange = CFRangeMake(0, nLength);
                if (!CFStringFindCharacterFromSet(hEncoding, hSet, tRange, 0, &tRange) && (uint32_t)nLength == nBits) {
                    uint32_t nModeWidth = CGDisplayModeGetWidth(hMode);
                    uint32_t nModeHeight = CGDisplayModeGetHeight(hMode);
                    if (nModeWidth >= nWidth && nModeHeight >= nHeight) {
                        uint32_t nFlags = CGDisplayModeGetIOFlags(hMode);
                        int64_t nRate = (int64_t)CGDisplayModeGetRefreshRate(hMode);
                        int nRateDiff = (int)nRefreshHz - (int)nRate;
                        if (nRateDiff < 0) {
                            nRateDiff = -nRateDiff;
                        }
                        int nStretchPenalty = (uint32_t)bStretched != ((nFlags >> 11) & 1) ? 100 : 0;
                        int nScore = nModeHeight * nModeWidth - nArea + nStretchPenalty + nRateDiff;
                        if (nScore < nBestScore) {
                            nBestScore = nScore;
                            hBest = hMode;
                        }
                    }
                }
            }
        }
    }
    CGDisplayModeRef hResult = (bAcceptClosest || nBestScore == 0) ? hBest : NULL;
    CFRetain(hResult);
    CFRelease(hModes);
    CFRelease(hSet);
    return hResult;
}

#endif  // !defined(_WIN32) && D2_OS_MAC
