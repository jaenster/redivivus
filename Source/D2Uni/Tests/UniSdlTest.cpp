// D2Uni on SDL3: the renderer table's test. It drives UniFunctionTable the way D2gfx does
// (SetOption 8, Initialize, InitPerspective, CreateWindow on a Win32Posix window), sets a
// palette and the palette tables, draws primitives and a DC6 sprite through the slots, presents,
// and checks three things: the 8-bit back buffer against what each slot must have written (the
// sprite against a decoder of its own), the screenshot slot's RGB against the palette, and the
// presented ARGB pixels read back from the SDL renderer, at 1:1 and scaled into a larger window.
//
// Headless: SDL_VIDEODRIVER=dummy ./d2_uni_test   (the default when --live is not given)
// Live:     ./d2_uni_test --live [seconds] [shot.bmp]   a real window, animated, vsync on; the
//           last presented frame is read back into shot.bmp. Metal keeps no presented frame to
//           read: SDL_RENDER_DRIVER=software gives the shot (the same conversion and scaling).
// D2UNI_TEST_DC6=<file.dc6> draws that sprite in place of the built-in one; D2UNI_TEST_PAL=<pal.dat>
// (256 B, G, R triplets, as the game's act palettes) is the live run's palette.
//
// The test links D2Uni, D2CMP's CelCmp and FastCmp (the sprite blitter the slots call) and the
// Win32Posix layer; the few game calls those reach on the tested paths are supplied here, and
// every other symbol they reference is a stub that halts if it is ever reached.
#if !defined(_WIN32) && D2_BACKEND_POSIX

#include <SDL3/SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "Source/D2Uni/Src/uniInternal.h"
#include "Source/D2CMP/Src/CelCmp.h"
#include "Source/D2CMP/Src/SpriteCache.h"
#include "Source/D2Win/DC6.h"
#include "Source/D2Win/DC6Block.h"

// ---- the game calls the tested paths reach ----

namespace D2CMP::SRC::SpriteCache {
// The real one's path for a sprite whose DC6 is already loaded.
int __stdcall SPRITECACHE_GetOrLoadSprite(D2GfxDataStrc * pGfxData, int32_t bCheckOnly, int32_t bLoadAsCel) {
    (void)bCheckOnly;
    (void)bLoadAsCel;
    if (!pGfxData || !pGfxData->pDC6) {
        return 0;
    }
    pGfxData->pDC6Block = D2CMP::SRC::CelCmp::CELCMP_GetDC6BlockFromGfxData(pGfxData);
    return 1;
}
BOOL __stdcall SPRITECACHE_AllocEntry(D2TileLibraryEntryStrc * pTile, uint32_t dwFlags, uint32_t dwReserved) {
    (void)pTile;
    (void)dwFlags;
    (void)dwReserved;
    return 0;
}
} // namespace D2CMP::SRC::SpriteCache

namespace D2Hell {
int32_t D2MEM_FindLowestSetBit(uint32_t dwMask) {
    return dwMask ? __builtin_ctz(dwMask) : -1;
}
} // namespace D2Hell

namespace Fog::Src::ErrorManager {
void * __stdcall GetInstructionPointer() {
    return __builtin_return_address(0);
}
void ERROR_UnrecoverableInternalError_Halt(char * szMessage, void * nAddress, int32_t nLine) {
    fprintf(stderr, "halt: %s at %p line 0x%x\n", szMessage, nAddress, nLine);
    abort();
}
} // namespace Fog::Src::ErrorManager

// ---- the rest of what CelCmp, FastCmp and D2Uni reference: never reached by the tested paths ----
// Named by their linker symbols, so the test links whole (a symbol left to dynamic lookup that is
// taken by address fails at load). The four data symbols are zeroed storage.

[[noreturn]] static void Unreached(const char * szSymbol) {
    fprintf(stderr, "unreached: %s\n", szSymbol);
    abort();
}

#define UNI_TEST_UNREACHED(id, sym)                                                                \
    extern "C" void id() __asm__(sym);                                                             \
    extern "C" void id() {                                                                         \
        Unreached(sym);                                                                            \
    }
#define UNI_TEST_DATA(id, sym)                                                                     \
    extern "C" unsigned char id[256] __asm__(sym);                                                 \
    alignas(16) unsigned char id[256] = {};

UNI_TEST_UNREACHED(UniTestUnreached0, "__ZN11TSHashTableI7CELLIST12HASHKEY_NONEE3PtrEjPS1_")
UNI_TEST_UNREACHED(UniTestUnreached1, "__ZN11TSHashTableI7CELLIST12HASHKEY_NONEE6UnlinkEPS0_")
UNI_TEST_UNREACHED(UniTestUnreached2, "__ZN16TSHashTableReuseI7CELLIST12HASHKEY_NONELi0EE10DestructorEv")
UNI_TEST_UNREACHED(UniTestUnreached3, "__ZN16TSHashTableReuseI7CELLIST12HASHKEY_NONELi0EE11ConstructorEv")
UNI_TEST_UNREACHED(UniTestUnreached4, "__ZN3Fog6Memory16FreeClientMemoryEPvPcii")
UNI_TEST_UNREACHED(UniTestUnreached5, "__ZN3Fog6Memory17AllocClientMemoryEmPcii")
UNI_TEST_UNREACHED(UniTestUnreached6, "__ZN3Fog6Source5SFile19GetUseDirectCommandEv")
UNI_TEST_UNREACHED(UniTestUnreached7, "__ZN5D2CMP22SUBTILE_CountRunLengthEPcS0_iPiPj")
UNI_TEST_UNREACHED(UniTestUnreached8, "__ZN5D2CMP29SUBTILE_CountRunLengthForByteEPcS0_ciPiPj")
UNI_TEST_UNREACHED(UniTestUnreached9, "__ZN5D2CMP3SRC11SpriteCache32SPRITECACHE_SetFreeEntryCallbackEPFvPvE")
UNI_TEST_UNREACHED(UniTestUnreached10, "__ZN5D2CMP3SRC3Raw19RAW_CreateCelFramesEP7CELLISTP3DC6")
UNI_TEST_UNREACHED(UniTestUnreached11, "__ZN5D2Win3Src9D2WinFont20SetAndReturnLastFontEi")
UNI_TEST_UNREACHED(UniTestUnreached12, "__ZN5D2Win3Src9D2WinFont9DRAW_textEPtiiii")
UNI_TEST_UNREACHED(UniTestUnreached13, "__ZN5Storm6Source4SBMP15SBmpDecodeImageEiPvjS2_PhiPjS4_S4_")
UNI_TEST_UNREACHED(UniTestUnreached14, "__ZN5Storm6Source4SMem8SMemFreeEPvPcij")
UNI_TEST_UNREACHED(UniTestUnreached15, "__ZN5Storm6Source4SMem9SMemAllocEjPcjj")
UNI_TEST_UNREACHED(UniTestUnreached16, "__ZN5Storm6Source5SFile15SFileLoadFileExEP13D2ArchiveStrcPcPPvPjjjP11_OVERLAPPED")
UNI_TEST_UNREACHED(UniTestUnreached17, "__ZN5Storm6Source5SFile15SFileOpenFileExEP13D2ArchiveStrcPcjPPv")
UNI_TEST_UNREACHED(UniTestUnreached18, "__ZN6D2Hell14D2MEM_CopyRowsEPcPhiiij")
UNI_TEST_UNREACHED(UniTestUnreached19, "__ZN6D2Lang7Unicode6UNISYS25STRING_ConvertCharToWcharEPtPci")
UNI_TEST_UNREACHED(UniTestUnreached20, "__ZN6TSListI9TCELENTRY9TSGetLinkIS0_EE5ClearEv")
UNI_TEST_UNREACHED(UniTestUnreached21, "_BinkClose")
UNI_TEST_UNREACHED(UniTestUnreached22, "_BinkCopyToBuffer")
UNI_TEST_UNREACHED(UniTestUnreached23, "_BinkDoFrame")
UNI_TEST_UNREACHED(UniTestUnreached24, "_BinkNextFrame")
UNI_TEST_UNREACHED(UniTestUnreached25, "_BinkOpen")
UNI_TEST_UNREACHED(UniTestUnreached26, "_BinkOpenDirectSound")
UNI_TEST_UNREACHED(UniTestUnreached27, "_BinkSetSoundSystem")
UNI_TEST_UNREACHED(UniTestUnreached28, "_BinkWait")
UNI_TEST_UNREACHED(UniTestUnreached29, "_SmackClose")
UNI_TEST_UNREACHED(UniTestUnreached30, "_SmackDoFrame")
UNI_TEST_UNREACHED(UniTestUnreached31, "_SmackNextFrame")
UNI_TEST_UNREACHED(UniTestUnreached32, "_SmackOpen")
UNI_TEST_UNREACHED(UniTestUnreached33, "_SmackToBuffer")
UNI_TEST_UNREACHED(UniTestUnreached34, "_SmackWait")
UNI_TEST_DATA(gUniTestData0, "__ZN17TSExportTableSyncI7CELLISTP10HCELLIST__P16HLOCKEDCELLIST__9CCritSectLi0EE7vftableE")
UNI_TEST_DATA(gUniTestData1, "__ZN5D2CMP3SRC6CelCmp19gCelCmpCacheHandlerE")
UNI_TEST_DATA(gUniTestData2, "__ZN5D2CMP3SRC6CelCmp26lpCriticalSection_008f03ccE")
UNI_TEST_DATA(gUniTestData3, "__ZN5D2CMP3SRC6CelCmp26s___AUTCELENTRY___00741f14E")

// ---- the checks ----

static int s_nChecks;
static int s_nFailures;

#define CHECK(cond, ...)                                                                           \
    do {                                                                                           \
        s_nChecks++;                                                                               \
        if (!(cond)) {                                                                             \
            s_nFailures++;                                                                         \
            fprintf(stderr, "FAIL %s:%d: %s: ", __FILE__, __LINE__, #cond);                       \
            fprintf(stderr, __VA_ARGS__);                                                          \
            fputc('\n', stderr);                                                                   \
        }                                                                                          \
    } while (0)

// ---- D2gfx's side: the helpers and the settings ----

static void * s_pYBufferBits;
static int s_nYBufferWidth;
static int s_nYBufferHeight;
static void __fastcall TestFillYBufferTable(void * ppvBits, int nWidth, int nHeight, int a4) {
    (void)a4;
    s_pYBufferBits = ppvBits;
    s_nYBufferWidth = nWidth;
    s_nYBufferHeight = nHeight;
}

// ---- the sprite: a DC6 file image, widened as D2CMP's loader widens it ----

// On disk (D2CMP CelCmp.cpp): a 0x18-byte header, one 32-bit offset per frame, and per frame a
// 0x20-byte block header, the RLE rows (bottom row first; 0x80 ends a row, 0x80|n skips n, n < 0x80
// copies n bytes) and a 3-byte terminator. Little-endian, as the files are.
enum {
    kDc6FileHeader = 0x18,
    kDc6FileBlockHeader = 0x20,
    kDc6FileTerminator = 3,
};

static uint32_t ReadLe32(const uint8_t * p) {
    return (uint32_t)p[0] | (uint32_t)p[1] << 8 | (uint32_t)p[2] << 16 | (uint32_t)p[3] << 24;
}

static void WriteLe32(uint8_t * p, uint32_t n) {
    p[0] = (uint8_t)n;
    p[1] = (uint8_t)(n >> 8);
    p[2] = (uint8_t)(n >> 16);
    p[3] = (uint8_t)(n >> 24);
}

// A 24x20 frame: a diamond of colours 0x20 + row, transparent around it, offset (2, -1).
static uint8_t * BuildDc6File(size_t * pnSize) {
    const int nWidth = 24;
    const int nHeight = 20;
    uint8_t aRle[4096];
    int nRle = 0;
    for (int nRow = 0; nRow < nHeight; nRow++) {
        int nHalf = nRow < nHeight / 2 ? nRow + 1 : nHeight - nRow;
        int nLeft = nWidth / 2 - nHalf;
        if (nLeft > 0) {
            aRle[nRle++] = (uint8_t)(0x80 | nLeft);
        }
        aRle[nRle++] = (uint8_t)(nHalf * 2);
        for (int i = 0; i < nHalf * 2; i++) {
            aRle[nRle++] = (uint8_t)(0x20 + nRow + (i & 1) * 0x40);
        }
        aRle[nRle++] = 0x80;
    }
    size_t nSize = kDc6FileHeader + 4 + kDc6FileBlockHeader + (size_t)nRle + kDc6FileTerminator;
    uint8_t * pFile = (uint8_t *)calloc(1, nSize);
    WriteLe32(pFile + 0x00, 6);          // nVersion
    WriteLe32(pFile + 0x04, 1);          // nFlags
    WriteLe32(pFile + 0x08, 0);          // nFormat
    WriteLe32(pFile + 0x0c, 0xEEEEEEEE); // nTerm
    WriteLe32(pFile + 0x10, 1);          // nDirections
    WriteLe32(pFile + 0x14, 1);          // nFrames
    WriteLe32(pFile + 0x18, kDc6FileHeader + 4);
    uint8_t * pBlock = pFile + kDc6FileHeader + 4;
    WriteLe32(pBlock + 0x00, 0);         // nFlip
    WriteLe32(pBlock + 0x04, nWidth);
    WriteLe32(pBlock + 0x08, nHeight);
    WriteLe32(pBlock + 0x0c, 2);         // nOffsetX
    WriteLe32(pBlock + 0x10, (uint32_t)-1); // nOffsetY
    WriteLe32(pBlock + 0x1c, (uint32_t)nRle);
    memcpy(pBlock + kDc6FileBlockHeader, aRle, (size_t)nRle);
    memset(pBlock + kDc6FileBlockHeader + nRle, 0xEE, kDc6FileTerminator);
    *pnSize = nSize;
    return pFile;
}

// The native image the renderers draw: the header with its pointer table, then native blocks.
static DC6 * WidenDc6(const uint8_t * pFile, size_t nFileSize) {
    if (nFileSize < kDc6FileHeader || ReadLe32(pFile) != 6) {
        return nullptr;
    }
    uint32_t nDirections = ReadLe32(pFile + 0x10);
    uint32_t nFrames = ReadLe32(pFile + 0x14);
    uint32_t nBlocks = nDirections * nFrames;
    size_t nAlloc = sizeof(DC6) + nBlocks * sizeof(DC6Block *);
    for (uint32_t i = 0; i < nBlocks; i++) {
        const uint8_t * pSrc = pFile + ReadLe32(pFile + kDc6FileHeader + 4 * i);
        nAlloc += sizeof(DC6Block) + ReadLe32(pSrc + 0x1c) + kDc6FileTerminator;
    }
    DC6 * pDc6 = (DC6 *)calloc(1, nAlloc);
    pDc6->nVersion = (int)ReadLe32(pFile + 0x00);
    pDc6->nFlags = (int)ReadLe32(pFile + 0x04);
    pDc6->nFormat = (int)ReadLe32(pFile + 0x08);
    pDc6->nDirections = (int)nDirections;
    pDc6->nFrames = (int)nFrames;
    uint8_t * pCursor = (uint8_t *)&pDc6->pBlocks[nBlocks];
    for (uint32_t i = 0; i < nBlocks; i++) {
        const uint8_t * pSrc = pFile + ReadLe32(pFile + kDc6FileHeader + 4 * i);
        DC6Block * pBlock = (DC6Block *)pCursor;
        pBlock->nFlip = (int)ReadLe32(pSrc + 0x00);
        pBlock->nWidth = (int)ReadLe32(pSrc + 0x04);
        pBlock->nHeight = (int)ReadLe32(pSrc + 0x08);
        pBlock->nOffsetX = (int)ReadLe32(pSrc + 0x0c);
        pBlock->nOffsetY = (int)ReadLe32(pSrc + 0x10);
        pBlock->nAllocSize = (int)ReadLe32(pSrc + 0x14);
        pBlock->nNextBlock = 0;
        pBlock->nLength = (int)ReadLe32(pSrc + 0x1c);
        memcpy(pBlock->data, pSrc + kDc6FileBlockHeader, (size_t)pBlock->nLength + kDc6FileTerminator);
        pDc6->pBlocks[i] = pBlock;
        pCursor += sizeof(DC6Block) + (size_t)pBlock->nLength + kDc6FileTerminator;
    }
    // The renderers draw flags-1 (relocated) images of version 6 without raw data.
    pDc6->nFlags &= ~4;
    return pDc6;
}

// The frame decoded independently into pDest (pitch nPitch) with its bottom-left at (nX, nY).
static void DecodeFrame(const DC6Block * pBlock, uint8_t * pDest, int nPitch, int nX, int nY, int nHeight) {
    const uint8_t * p = (const uint8_t *)pBlock->data;
    const uint8_t * pEnd = p + pBlock->nLength;
    int nCol = 0;
    int nRow = nY;
    int nStep = (pBlock->nFlip & 1) ? 1 : -1;
    while (p < pEnd) {
        uint8_t b = *p++;
        if (b == 0x80) {
            nRow += nStep;
            nCol = 0;
        } else if (b & 0x80) {
            nCol += b & 0x7F;
        } else {
            for (int i = 0; i < b; i++) {
                int nPx = nX + nCol++;
                if (nRow >= 0 && nRow < nHeight && nPx >= 0 && nPx < nPitch) {
                    pDest[nRow * nPitch + nPx] = p[i];
                }
            }
            p += b;
        }
    }
}

// ---- the scene ----

static PALETTEENTRY s_aPalette[256];
static BYTE s_abRemap[256];
static BYTE s_abBlend[256 * 256];
static BYTE * s_apTables[72];

static BYTE s_abPalFile[768];
static bool s_bPalFile;

static void BuildPalette(int nShift) {
    if (s_bPalFile) {
        for (int i = 0; i < 256; i++) {
            s_aPalette[i].peBlue = s_abPalFile[i * 3 + 0];
            s_aPalette[i].peGreen = s_abPalFile[i * 3 + 1];
            s_aPalette[i].peRed = s_abPalFile[i * 3 + 2];
            s_aPalette[i].peFlags = 0;
        }
        return;
    }
    for (int i = 0; i < 256; i++) {
        s_aPalette[i].peRed = (BYTE)(i + nShift);
        s_aPalette[i].peGreen = (BYTE)(255 - i);
        s_aPalette[i].peBlue = (BYTE)(i * 7 + nShift * 3);
        s_aPalette[i].peFlags = 0;
    }
}

static void DrawScene(D2RendererFunctionsStrc * pTable, D2GfxDataStrc * pSprite, int nFrame) {
    pTable->nfpBeginScene(1, 0, 0, 0);
    pTable->nfpClearScreen(0);
    RECT tSolid = {10, 10, 50, 30};
    pTable->nfpDrawSolidRect(&tSolid, 7);
    pTable->nfpDrawLine(0, 100, 99, 100, 9, 0xFF);
    RECT tOutline = {200, 200, 260, 240};
    pTable->nfpDrawRectEx(&tOutline, 11);
    pTable->nfpDrawSolidRectEx(300, 10, 340, 50, 0, 3); // remap of the pixel there
    pTable->nfpDrawSolidRectEx(400, 10, 440, 50, 5, 0); // blend of colour 5 over the pixel there
    POINT tCentre = {500, 400};
    pTable->nfpDrawSolidSquare(&tCentre, 4, 13);
    pTable->nfpDrawImage(pSprite, 320 + nFrame % 200, 300, 0xFF, 5, nullptr);
    pTable->fpEndScene1();
    pTable->fpEndScene2();
}

// The pixel the presented frame has at (nX, nY) of the renderer's output, as ARGB8888.
static bool ReadPresented(SDL_Window * pWindow, SDL_Surface ** ppOut) {
    SDL_Renderer * pRenderer = SDL_GetRenderer(pWindow);
    SDL_Surface * pRead = pRenderer ? SDL_RenderReadPixels(pRenderer, nullptr) : nullptr;
    if (!pRead) {
        return false;
    }
    *ppOut = SDL_ConvertSurface(pRead, SDL_PIXELFORMAT_ARGB8888);
    SDL_DestroySurface(pRead);
    return *ppOut != nullptr;
}

static uint32_t PixelAt(SDL_Surface * pSurface, int nX, int nY) {
    return *(const uint32_t *)((const uint8_t *)pSurface->pixels + (size_t)nY * (size_t)pSurface->pitch + (size_t)nX * 4);
}

static uint32_t Argb(BYTE nIndex) {
    return 0xFF000000u | (uint32_t)s_aPalette[nIndex].peRed << 16 | (uint32_t)s_aPalette[nIndex].peGreen << 8 |
           s_aPalette[nIndex].peBlue;
}

static LRESULT __stdcall TestWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    return DefWindowProcA(hWnd, uMsg, wParam, lParam);
}

int main(int argc, char ** argv) {
    bool bLive = argc > 1 && strcmp(argv[1], "--live") == 0;
    int nSeconds = bLive && argc > 2 ? atoi(argv[2]) : 5;
    const char * szShot = bLive && argc > 3 ? argv[3] : nullptr;
    if (!bLive) {
        SDL_SetHint(SDL_HINT_VIDEO_DRIVER, "dummy");
    }

    // The sprite.
    size_t nFileSize = 0;
    uint8_t * pFile = nullptr;
    const char * szDc6 = getenv("D2UNI_TEST_DC6");
    if (szDc6) {
        FILE * f = fopen(szDc6, "rb");
        if (f) {
            fseek(f, 0, SEEK_END);
            nFileSize = (size_t)ftell(f);
            fseek(f, 0, SEEK_SET);
            pFile = (uint8_t *)malloc(nFileSize);
            nFileSize = fread(pFile, 1, nFileSize, f);
            fclose(f);
        }
        CHECK(pFile != nullptr, "cannot read %s", szDc6);
    }
    if (!pFile) {
        pFile = BuildDc6File(&nFileSize);
    }
    DC6 * pDc6 = WidenDc6(pFile, nFileSize);
    CHECK(pDc6 != nullptr, "not a DC6 image");
    if (!pDc6) {
        return 1;
    }
    D2GfxDataStrc tSprite = {};
    tSprite.pDC6 = pDc6;

    // The window D2gfx would make.
    WNDCLASSA tClass = {};
    tClass.lpfnWndProc = TestWndProc;
    tClass.lpszClassName = "D2UniTest";
    CHECK(RegisterClassA(&tClass) != 0, "RegisterClassA");

    D2RendererFunctionsStrc * pTable = &UniFunctionTable;
    static D2GfxSettingsStrc s_tSettings = {};
    s_tSettings.bVSync = bLive ? 1 : 0;
    static ::D2GfxHelperStrc s_tHelpers = {};
    s_tHelpers.nfpFillYBufferTable = (::fpFillYBufferTable)TestFillYBufferTable;

    CHECK(pTable->nfpSetOption(8, 1), "SetOption 8");
    CHECK(gnUniMode == UNIMODE_SDL, "gnUniMode %d", gnUniMode);
    CHECK(pTable->nfpInitialize(nullptr), "Initialize (UniDetect)");
    CHECK(pTable->nfpInitPerspective(&s_tSettings, &s_tHelpers), "InitPerspective (UniInit)");
    CHECK(ghUniColorTable && (*ghUniColorTable)->ctSize == 255, "the colour table");

    HWND hWnd = CreateWindowExA(0, "D2UniTest", "D2Uni on SDL3", 0xcb0000, 0, 0, 640, 480, nullptr, nullptr,
                                nullptr, nullptr);
    CHECK(hWnd != nullptr, "CreateWindowExA");
    ShowWindow(hWnd, 1);
    CHECK(pTable->nfpCreateWindow(hWnd, 0), "CreateWindow (UniCreateSurface)");
    int nWidth = 0, nHeight = 0;
    pTable->nfpGetScreenSize(&nWidth, &nHeight);
    CHECK(nWidth == 640 && nHeight == 480, "screen %dx%d", nWidth, nHeight);
    CHECK(gnUniBackBufferDepth == 8 && gnUniBackBufferWidth == 640, "back buffer %d bits, %d wide",
          gnUniBackBufferDepth, gnUniBackBufferWidth);

    BuildPalette(0);
    pTable->nfpSetPalette(s_aPalette);
    for (int i = 0; i < 256; i++) {
        s_abRemap[i] = (BYTE)(200 + i);
        for (int j = 0; j < 256; j++) {
            s_abBlend[i * 256 + j] = (BYTE)(i + j);
        }
    }
    s_apTables[0x41] = s_abRemap; // draw mode 3
    s_apTables[2] = s_abBlend;    // draw mode 0
    pTable->nfpSetPaletteTable(s_apTables);
    CHECK(gapUniPaletteTables[0x41] == s_abRemap && gapUniPaletteTables[2] == s_abBlend &&
              gapUniPaletteTables[71] == nullptr,
          "the palette tables are copied pointer by pointer");

    SDL_Window * pWindow = Win32Posix_GetSdlWindow(hWnd);
    CHECK(pWindow != nullptr, "the SDL window");

    if (bLive) {
        const char * szPal = getenv("D2UNI_TEST_PAL");
        FILE * f = szPal ? fopen(szPal, "rb") : nullptr;
        if (f) {
            s_bPalFile = fread(s_abPalFile, 1, sizeof(s_abPalFile), f) == sizeof(s_abPalFile);
            fclose(f);
        }
        Uint64 nEnd = SDL_GetTicks() + (Uint64)nSeconds * 1000;
        int nFrame = 0;
        MSG tMsg;
        while (SDL_GetTicks() < nEnd) {
            while (PeekMessageA(&tMsg, nullptr, 0, 0, PM_REMOVE)) {
                DispatchMessageA(&tMsg);
            }
            BuildPalette(nFrame);
            pTable->nfpSetPalette(s_aPalette);
            DrawScene(pTable, &tSprite, nFrame++);
        }
        if (szShot) {
            SDL_Surface * pShot = nullptr;
            CHECK(ReadPresented(pWindow, &pShot) && SDL_SaveBMP(pShot, szShot), "the shot: %s", SDL_GetError());
            if (pShot) {
                printf("shot: %dx%d -> %s\n", pShot->w, pShot->h, szShot);
                SDL_DestroySurface(pShot);
            }
        }
        printf("live: %d frames in %d s (%s)\n", nFrame, nSeconds,
               SDL_GetRendererName(SDL_GetRenderer(pWindow)));
    } else {
        DrawScene(pTable, &tSprite, 0);

        // The 8-bit back buffer, slot by slot.
        CHECK(s_pYBufferBits == nullptr, "EndScene1 drops D2gfx's row table");
        const BYTE * pBack = gpUniBackBuffer;
        auto At = [&](int nX, int nY) { return pBack[nY * gnUniBackBufferWidth + nX]; };
        CHECK(At(10, 10) == 7 && At(49, 29) == 7 && At(50, 30) == 0 && At(9, 10) == 0, "DrawSolidRect");
        CHECK(At(0, 100) == 9 && At(99, 100) == 9 && At(100, 100) == 0, "DrawLine");
        CHECK(At(200, 200) == 11 && At(259, 200) == 11 && At(200, 239) == 11 && At(230, 220) == 0, "DrawRectEx");
        CHECK(At(300, 10) == 200 && At(339, 49) == 200 && At(340, 10) == 0, "DrawSolidRectEx through the remap");
        CHECK(At(400, 10) == 5 && At(439, 49) == 5, "DrawSolidRectEx through the blend table");
        CHECK(At(497, 397) == 13 && At(503, 403) == 13 && At(504, 404) == 0, "DrawSolidSquare");

        // The sprite against the independent decode.
        static BYTE s_abExpected[640 * 480];
        memset(s_abExpected, 0, sizeof(s_abExpected));
        const DC6Block * pBlock = pDc6->pBlocks[0];
        DecodeFrame(pBlock, s_abExpected, 640, 320 + pBlock->nOffsetX, 300 + pBlock->nOffsetY, 480);
        int nSpriteBad = 0, nSpritePixels = 0;
        for (int y = 0; y < 480; y++) {
            for (int x = 0; x < 640; x++) {
                if (s_abExpected[y * 640 + x]) {
                    nSpritePixels++;
                    nSpriteBad += At(x, y) != s_abExpected[y * 640 + x];
                }
            }
        }
        CHECK(nSpritePixels > 0 && nSpriteBad == 0, "DrawImage: %d of %d sprite pixels differ", nSpriteBad,
              nSpritePixels);

        // The screenshot slot: 24-bit rows through the colour table.
        static BYTE s_abRgb[640 * 480 * 3];
        CHECK(pTable->nfpGetBackBuffer(s_abRgb), "GetBackBuffer");
        const BYTE * pRgb = &s_abRgb[(10 * 640 + 10) * 3];
        CHECK(pRgb[0] == s_aPalette[7].peRed && pRgb[1] == s_aPalette[7].peGreen && pRgb[2] == s_aPalette[7].peBlue,
              "GetBackBuffer RGB %02x%02x%02x", pRgb[0], pRgb[1], pRgb[2]);

        // The presented frame, 1:1.
        SDL_Surface * pShot = nullptr;
        CHECK(ReadPresented(pWindow, &pShot), "SDL_RenderReadPixels: %s", SDL_GetError());
        if (pShot) {
            CHECK(pShot->w == 640 && pShot->h == 480, "output %dx%d", pShot->w, pShot->h);
            int nBad = 0;
            for (int y = 0; y < 480 && pShot->h == 480; y++) {
                for (int x = 0; x < 640 && pShot->w == 640; x++) {
                    nBad += PixelAt(pShot, x, y) != Argb(At(x, y));
                }
            }
            CHECK(nBad == 0, "%d presented pixels are not the palette's ARGB of the back buffer", nBad);
            SDL_DestroySurface(pShot);
        }

        // The palette changes, the frame follows.
        BuildPalette(40);
        pTable->nfpSetPalette(s_aPalette);
        DrawScene(pTable, &tSprite, 0);
        if (ReadPresented(pWindow, &pShot)) {
            CHECK(PixelAt(pShot, 10, 10) == Argb(7), "after SetPalette %08x", PixelAt(pShot, 10, 10));
            SDL_DestroySurface(pShot);
        }

        // Scaled: a window 2.25 by 2 times the client, so the frame is 2x, centred.
        SDL_SetWindowSize(pWindow, 1440, 960);
        MSG tMsg;
        while (PeekMessageA(&tMsg, nullptr, 0, 0, PM_REMOVE)) {
            DispatchMessageA(&tMsg);
        }
        int nVx = 0, nVy = 0, nVw = 0, nVh = 0;
        CHECK(Win32Posix_GetViewport(hWnd, &nVx, &nVy, &nVw, &nVh), "Win32Posix_GetViewport");
        printf("viewport in a 1440x960 window: %d,%d %dx%d\n", nVx, nVy, nVw, nVh);
        CHECK(nVw * 480 == nVh * 640, "the viewport keeps 4:3 (%dx%d)", nVw, nVh);
        DrawScene(pTable, &tSprite, 0);
        if (ReadPresented(pWindow, &pShot)) {
            CHECK(pShot->w == 1440 && pShot->h == 960, "scaled output %dx%d", pShot->w, pShot->h);
            if (pShot->w == 1440 && pShot->h == 960 && nVw > 0 && nVh > 0) {
                int nBad = 0;
                for (int y = 0; y < 480; y += 7) {
                    for (int x = 0; x < 640; x += 5) {
                        // the centre of the game pixel's block on screen
                        int sx = nVx + (2 * x + 1) * nVw / 1280;
                        int sy = nVy + (2 * y + 1) * nVh / 960;
                        nBad += PixelAt(pShot, sx, sy) != Argb(At(x, y));
                    }
                }
                CHECK(nBad == 0, "%d sampled scaled pixels differ", nBad);
                if (nVx > 0) {
                    CHECK(PixelAt(pShot, nVx / 2, 480) == 0xFF000000u, "the bar left of the frame is black: %08x",
                          PixelAt(pShot, nVx / 2, 480));
                }
            }
            SDL_DestroySurface(pShot);
        }
    }

    CHECK(pTable->nfpDestroyWindow(), "DestroyWindow (UniCloseSurface)");
    CHECK(gpUniBackBuffer == nullptr && gnUniScreenWidth == 0, "the surface is down");
    CHECK(pTable->nfpRelease(), "Release");
    DestroyWindow(hWnd);
    free(pDc6);
    free(pFile);

    printf("%d checks, %d failures\n", s_nChecks, s_nFailures);
    return s_nFailures ? 1 : 0;
}

#endif // !defined(_WIN32) && D2_BACKEND_POSIX
