// D2FlamingLogo.cpp - the animated flaming Diablo II logo of the Mac setup and about
// dialogs: thirty 440x160 8-bit frames from one PCX strip (and a mask strip), drawn into a
// dialog item every two ticks.
//
// 1.14d Intel Mac (DiabloII_macho) 0019f2b6..0019f900. Pinned by the 1.14d __FILE__
// ".../Diablo2/Source/DiabloAll/../MacSpecific/Source/Mac Shell/D2FlamingLogo.cpp"
// (0031a9d3), passed to SMemAlloc/SMemFree by FLAMINGLOGO_Init and FLAMINGLOGO_Shutdown.
// The Baal AI follows at 0019f900, so the file ends there; where it starts is not proven
// (0019f18a and 0019f1c4 before it use no file name; they are kept in unknown_util.cpp).
//
// Mac-only: the guard is spelled `#if !defined(_WIN32) && D2_OS_MAC` (see
// Source/MacSpecific/README.md).
#include "D2Arch.h"

#if !defined(_WIN32) && D2_OS_MAC

#include <Carbon/Carbon.h>
#include <QuickTime/QuickTime.h>

#include "Source/MacSpecific/Source/MacShell/QuickDrawMac.h"

#include "Source/StormMac/H/stpl.h"

typedef int BOOL;

// 1.14d mac: 0000c421
int SBmpLoadImage(const char* szFileName, void* pPalette, void* pBuffer, uint32_t nBufferSize, int* pnWidth,
                  int* pnHeight, int* pnBitDepth);
// 1.14d mac: 00014226
int SFileOpenArchive(const char* szArchiveName, int nPriority, uint32_t dwFlags, void** phArchive);
// 1.14d mac: 0001239c
int SFileCloseArchive(void* hArchive);
// 1.14d mac: 00034740
void GetDialogItemRect(DialogRef pDialog, DialogItemIndex nItem, Rect* pRect);
// 1.14d mac: 002edc4a
BOOL D2CheckExpansionGame(void);

void FLAMINGLOGO_Shutdown(void);

enum {
    FLAMINGLOGO_WIDTH = 440,
    FLAMINGLOGO_HEIGHT = 160,
    FLAMINGLOGO_FRAMES = 30,
    FLAMINGLOGO_FRAME_SIZE = FLAMINGLOGO_WIDTH * FLAMINGLOGO_HEIGHT,
    FLAMINGLOGO_STRIP_SIZE = FLAMINGLOGO_FRAME_SIZE * FLAMINGLOGO_FRAMES,
};

// The logo kinds: the QuickDraw strips of the classic and expansion game, and their Carbon
// (masked) versions. Only the Carbon ones are drawn in 1.14d.
enum {
    FLAMINGLOGO_CLASSIC,
    FLAMINGLOGO_EXPANSION,
    FLAMINGLOGO_CARBON_CLASSIC,
    FLAMINGLOGO_CARBON_EXPANSION,
};
// 1.14d mac: 003e0ee8
static const char* s_szLogoFiles[4] = {
    "Data\\Local\\MacUI\\D2Logo.pcx",
    "Data\\Local\\MacUI\\D2ELogo.pcx",
    "Data\\Local\\MacUI\\D2LogoCarbon.pcx",
    "Data\\Local\\MacUI\\D2ELogoCarbon.pcx",
};
// 1.14d mac: 003e0ef8
// Not static: FLAMINGLOGO_Init reads the mask name before its SMemAlloc call, which clang only
// keeps there when the table is not an internal never-written (so constant) array.
const char* s_szLogoMaskFiles[4] = {
    0,
    0,
    "Data\\Local\\MacUI\\D2LogoCarbonMask.pcx",
    "Data\\Local\\MacUI\\D2ELogoCarbonMask.pcx",
};
// 1.14d mac: 00331f30
// The 'clut' resources of the strips.
static const int s_nLogoCTables[4] = {0xC80, 0xC81, 0xC82, 0xC83};
// 1.14d mac: 00331f40
static const Rect s_rcFrame = {0, 0, FLAMINGLOGO_HEIGHT, FLAMINGLOGO_WIDTH};

// 1.14d mac: 00536cf4
static DialogRef s_pDialog;
// 1.14d mac: 00536cf8
static int s_nDialogItem;
// 1.14d mac: 00536cfc
static uint8_t* s_pFrames;
// 1.14d mac: 00536d00
static uint8_t* s_pMasks;
// 1.14d mac: 00536d04
static CTabHandle s_hCTable;
// 1.14d mac: 00536d08
static CTabHandle s_hMaskCTable;
// 1.14d mac: 00536d0c
static uint32_t s_dwNextTick;
// 1.14d mac: 00536d10
static uint32_t s_nFrame;

// A GWorld over one frame of a strip, disposed when it goes out of scope.
struct FlamingLogoGWorld {
    GWorldPtr m_pWorld;

    FlamingLogoGWorld() : m_pWorld(0) {}
    ~FlamingLogoGWorld() {
        if (m_pWorld)
            DisposeGWorld(m_pWorld);
    }
};

// 1.14d mac: 0019f2b6
// Loads the strips (opening the game MPQs when the files are not loose) and the colour
// tables for the dialog item that shows the logo; on failure everything is released again.
void FLAMINGLOGO_Init(DialogRef pDialog, int nItem) {
    void* hGameArchive = 0;
    void* hPatchArchive = 0;
    void* hExpansionArchive = 0;
    int nWidth, nHeight, nBitDepth;

    BOOL bExpansion = D2CheckExpansionGame();
    s_pDialog = pDialog;
    s_nDialogItem = nItem;
    s_pFrames = (uint8_t*)SMemAlloc(FLAMINGLOGO_STRIP_SIZE, __FILE__, 87, 8);
    int nLogo = bExpansion ? FLAMINGLOGO_CARBON_EXPANSION : FLAMINGLOGO_CARBON_CLASSIC;
    const char* szLogo = s_szLogoFiles[nLogo];
    int nCTable = s_nLogoCTables[nLogo];
    if (!SBmpLoadImage(szLogo, 0, s_pFrames, FLAMINGLOGO_STRIP_SIZE, &nWidth, &nHeight, &nBitDepth)) {
        if (!SFileOpenArchive("Diablo II Game Data", 1000, 0, &hGameArchive))
            goto fail;
        SFileOpenArchive("Diablo II Patch", 1000, 0, &hPatchArchive);
        if (D2CheckExpansionGame())
            SFileOpenArchive("Diablo II Expansion Data", 1000, 0, &hExpansionArchive);
        if (!SBmpLoadImage(szLogo, 0, s_pFrames, FLAMINGLOGO_STRIP_SIZE, &nWidth, &nHeight, &nBitDepth))
            goto close;
    }
    switch (nLogo) {
    case FLAMINGLOGO_CARBON_CLASSIC:
    case FLAMINGLOGO_CARBON_EXPANSION: {
        const char* szMask = s_szLogoMaskFiles[nLogo];
        s_pMasks = (uint8_t*)SMemAlloc(FLAMINGLOGO_STRIP_SIZE, __FILE__, 114, 8);
        if (!SBmpLoadImage(szMask, 0, s_pMasks, FLAMINGLOGO_STRIP_SIZE, &nWidth, &nHeight, &nBitDepth)) {
            SMemFree(s_pMasks, __FILE__, 125, 0);
            s_pMasks = 0;
        }
        break;
    }
    }
close:
    if (hGameArchive)
        SFileCloseArchive(hGameArchive);
    if (hPatchArchive)
        SFileCloseArchive(hPatchArchive);
    if (hExpansionArchive)
        SFileCloseArchive(hExpansionArchive);
    s_hCTable = GetCTable((short)nCTable);
    if (s_hCTable) {
        s_hMaskCTable = GetCTable(40);
        if (s_hMaskCTable) {
            s_dwNextTick = TickCount() + 2;
            return;
        }
    }
fail:
    FLAMINGLOGO_Shutdown();
}

// 1.14d mac: 0019f551
void FLAMINGLOGO_Shutdown(void) {
    if (s_hCTable) {
        DisposeCTable(s_hCTable);
        s_hCTable = 0;
    }
    if (s_hMaskCTable) {
        DisposeCTable(s_hMaskCTable);
        s_hMaskCTable = 0;
    }
    s_pDialog = 0;
    s_nDialogItem = 0;
    if (s_pFrames) {
        SMemFree(s_pFrames, __FILE__, 279, 0);
        s_pFrames = 0;
    }
    if (s_pMasks) {
        SMemFree(s_pMasks, __FILE__, 285, 0);
        s_pMasks = 0;
    }
}

// 1.14d mac: 0019f61c
// Every second tick (never more than ten frames behind): the next frame, copied into the
// dialog item, through the mask when there is one.
void FLAMINGLOGO_DrawFrame(ControlRef) {
    if (!s_pFrames)
        return;
    uint32_t dwTicks = TickCount();
    if (dwTicks <= s_dwNextTick)
        return;
    uint32_t dwNextTick = dwTicks > s_dwNextTick + 22 ? dwTicks : s_dwNextTick + 2;
    FlamingLogoGWorld frame;
    FlamingLogoGWorld mask;
    s_dwNextTick = dwNextTick;
    s_nFrame = (s_nFrame + 1) % FLAMINGLOGO_FRAMES;
    if (QTNewGWorldFromPtr(&frame.m_pWorld, 8, &s_rcFrame, s_hCTable, 0, 0, s_pFrames + s_nFrame * FLAMINGLOGO_FRAME_SIZE,
                           FLAMINGLOGO_WIDTH))
        return;
    if (s_pMasks && QTNewGWorldFromPtr(&mask.m_pWorld, 8, &s_rcFrame, s_hMaskCTable, 0, 0,
                                       s_pMasks + s_nFrame * FLAMINGLOGO_FRAME_SIZE, FLAMINGLOGO_WIDTH))
        return;

    Rect rcItem;
    GrafPtr pSavedPort;
    RGBColor rgbFore, rgbBack;
    GetDialogItemRect(s_pDialog, s_nDialogItem, &rcItem);
    GetPort(&pSavedPort);
    SetPortWindowPort(GetDialogWindow(s_pDialog));
    GetForeColor(&rgbFore);
    GetBackColor(&rgbBack);
    ForeColor(blackColor);
    BackColor(whiteColor);
    if (mask.m_pWorld) {
        EraseRect(&rcItem);
        CopyDeepMask(GetPortBitMapForCopyBits(frame.m_pWorld), GetPortBitMapForCopyBits(mask.m_pWorld),
                     GetPortBitMapForCopyBits(GetDialogPort(s_pDialog)), &s_rcFrame, &s_rcFrame, &rcItem, srcCopy, 0);
    } else {
        CopyBits(GetPortBitMapForCopyBits(frame.m_pWorld), GetPortBitMapForCopyBits(GetDialogPort(s_pDialog)),
                 &s_rcFrame, &rcItem, srcCopy, 0);
    }
    RGBForeColor(&rgbFore);
    RGBBackColor(&rgbBack);
    SetPort(pSavedPort);
}

#endif  // D2_OS_MAC
