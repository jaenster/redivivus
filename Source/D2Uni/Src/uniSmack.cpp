// uniSmack.cpp - Smacker movies and the 468x60 PCX banners (D2Multi's ad slot) for every D2Uni
// back end: decode into a private 8-bit buffer, remap the movie palette onto the game palette,
// copy into the back buffer.
//
// The one D2Uni file name the binary attests: "uniSmack.cpp" is the __FILE__ these functions
// pass to Fog's allocator and Storm's SMemFree (1.10b string 103f5614). Windows twins: D2DDraw
// ddSmack.cpp and D2Gdi gdiSmack.cpp.
//
// MWCC emits an object's functions last to first; the __FILE__ line numbers agree (the decoder's
// allocations are lines 95 and 112, the free shared by every teardown is line 256), so the
// functions below are in source order, highest address first. UniCloseSmacker's body is inlined
// into the decoder and the player.
//
// Classic Mac PowerPC, and the Win32Posix build (D2_BACKEND_POSIX) over the SDL3 back end.
#if !defined(_WIN32) && ((D2_MAC_CARBON && D2_ARCH_PPC) || D2_BACKEND_POSIX)

#include "Source/D2Uni/Src/uniInternal.h"
#include "Source/D2Multi/D2Multi.h"

BOOL gbUniSmackPaletteReady;        // 1055dc20
BYTE gabUniSmackRemap[256];         // 1055dc24

static BYTE * gpUniSmackBuffer;     // 10434494, the decoded frame or banner

// The banner the 'p' mode expects: 468x60, 8 bits.
enum {
    UNISMACK_BANNER_WIDTH = 468,
    UNISMACK_BANNER_HEIGHT = 60,
};

// Nearest game-palette index for one RGB colour, by the luminance-weighted distance
// 77*dR^2 + 151*dG^2 + 28*dB^2 against the 8-bit top halves of the colour table. Written out in
// both callers in the binary (unrolled by two); one helper here.
static BYTE UniSmackNearestColor(const BYTE * pRGB) {
    const ColorSpec * pTable = (*ghUniColorTable)->ctTable;
    int nBest = 0x7FFFFFFF;
    BYTE nBestIndex = 0;
    for (int i = 0; i < 256; i++) {
        int nDR = (pTable[i].rgb.red >> 8) - pRGB[0];
        int nDG = (pTable[i].rgb.green >> 8) - pRGB[1];
        int nDB = (pTable[i].rgb.blue >> 8) - pRGB[2];
        int nDistance = nDR * nDR * 77 + nDG * nDG * 151 + nDB * nDB * 28;
        if (nDistance < nBest) {
            nBest = nDistance;
            nBestIndex = (BYTE)i;
        }
    }
    return nBestIndex;
}

// ppc 1.10b: 10054f30 (1.00 1008436c, 1.13d 1005af50, carbon 1.10b 103a3ee0, carbon 1.13d 103d3630)
// 's' opens a Smacker movie (through its FSSpec) and decodes the first frame; 'p' loads a PCX
// banner through Storm and remaps its palette. On any failure the context is closed again.
void __fastcall UniDecodeSmacker(char * szFile, D2SmackerContextStrc * pContext, int nMode) {
#if !D2_BACKEND_POSIX
    FSSpec tSpec;
#endif
    BYTE abPalette[1024];
    void * pFileData;
    DWORD dwFileSize;
    uint32_t nWidth;
    uint32_t nHeight;
    uint32_t nBpp;

    pContext->eMode = (char)nMode;
    if (nMode == 's') {
        if (pContext->hSmack) {
            return;
        }
#if D2_BACKEND_POSIX
        {
            // SmackOpen takes the path, as on Windows.
            pContext->hSmack = SmackOpen(szFile, SMACKTRACKS, 0xFFFFFFFF);
#else
        if (FilepathToFSSpec(&tSpec, szFile, 0) == 0) {
            // The Mac SmackOpen takes the FSSpec where Windows takes a path.
            pContext->hSmack = SmackOpen((const char *)&tSpec, SMACKTRACKS, 0xFFFFFFFF);
#endif
            Smack * pSmack = (Smack *)pContext->hSmack;
            if (pSmack) {
                gpUniSmackBuffer = (BYTE *)Fog::Memory::AllocClientMemory(pSmack->Width * pSmack->Height,
                                                                          (char *)"uniSmack.cpp", 95, 0);
                ZeroMemory(gpUniSmackBuffer, pSmack->Width * pSmack->Height);
                SmackToBuffer(pContext->hSmack, 0, 0, pSmack->Width, pSmack->Height, gpUniSmackBuffer, 0);
                gbUniSmackPaletteReady = 0;
                return;
            }
        }
    } else if (nMode == 'p') {
        gpUniSmackBuffer = (BYTE *)Fog::Memory::AllocClientMemory(UNISMACK_BANNER_WIDTH * UNISMACK_BANNER_HEIGHT,
                                                                  (char *)"uniSmack.cpp", 112, 0);
        ZeroMemory(gpUniSmackBuffer, UNISMACK_BANNER_WIDTH * UNISMACK_BANNER_HEIGHT);
        pFileData = nullptr;
        dwFileSize = 0;
        if (Storm::Source::SFile::SFileLoadFileEx(nullptr, szFile, &pFileData, &dwFileSize, 0, 3, nullptr) &&
            pFileData) {
            int bDecoded = Storm::Source::SBMP::SBmpDecodeImage(2, pFileData, dwFileSize, abPalette, gpUniSmackBuffer,
                                                                UNISMACK_BANNER_WIDTH * UNISMACK_BANNER_HEIGHT,
                                                                &nWidth, &nHeight, &nBpp);
            Storm::Source::SMem::SMemFree(pFileData, (char *)"uniSmack.cpp", 147, 0);
            if (bDecoded && nWidth == UNISMACK_BANNER_WIDTH && nHeight == UNISMACK_BANNER_HEIGHT && nBpp == 8) {
                // PCX palette entries are 4 bytes (R, G, B, unused).
                for (int i = 0; i < 256; i++) {
                    gabUniSmackRemap[i] = UniSmackNearestColor(&abPalette[i * 4]);
                }
                return;
            }
        }
    } else {
        return;
    }

    // UniCloseSmacker, inlined.
    if (gpUniSmackBuffer) {
        Fog::Memory::FreeClientMemory(gpUniSmackBuffer, (char *)"uniSmack.cpp", 256, 0);
        gpUniSmackBuffer = nullptr;
    }
    if (pContext->eMode == 's' && pContext->hSmack) {
        SmackClose(pContext->hSmack);
        pContext->hSmack = nullptr;
    }
    pContext->eMode = 0;
}

static void sUniSmackBlit(D2SmackerContextStrc * pContext, uint32_t nWidth, int nHeight);

// ppc 1.10b: 10054cd0 (1.00 10083fa4, 1.13d 1005acf0, carbon 1.10b 103a3c80, carbon 1.13d 103d33d0)
// Advances a movie one frame when its timer allows, rebuilding the remap when the frame brings a
// new palette, and copies the current frame (or the banner) into the back buffer unless the
// surface is down. When the last frame is reached the movie is closed (UniCloseSmacker inlined)
// and the blit below still reads the closed movie's size, as the binary does.
void __fastcall UniPlayerSmacker(D2SmackerContextStrc * pContext) {
    if (pContext->eMode == 's') {
        Smack * pSmack = (Smack *)pContext->hSmack;
        if (!pSmack) {
            return;
        }

        if (!SmackWait(pSmack)) {
            if (pSmack->NewPalette) {
                // Smacker palette entries are 3 bytes.
                for (int i = 0; i < 256; i++) {
                    gabUniSmackRemap[i] = UniSmackNearestColor(&pSmack->Palette[i * 3]);
                }
                gbUniSmackPaletteReady = 1;
            }

            SmackDoFrame(pSmack);
            if (pSmack->FrameNum == pSmack->Frames) {
                if (gpUniSmackBuffer) {
                    Fog::Memory::FreeClientMemory(gpUniSmackBuffer, (char *)"uniSmack.cpp", 256, 0);
                    gpUniSmackBuffer = nullptr;
                }
                if (pContext->eMode == 's' && pContext->hSmack) {
                    SmackClose(pContext->hSmack);
                    pContext->hSmack = nullptr;
                }
                pContext->eMode = 0;
            } else {
                SmackNextFrame(pSmack);
            }
        }

        if (!gbUniSurfaceReleased && gbUniSmackPaletteReady) {
            sUniSmackBlit(pContext, pSmack->Width, pSmack->Height);
        }
    } else if (pContext->eMode == 'p' && !gbUniSurfaceReleased) {
        sUniSmackBlit(pContext, UNISMACK_BANNER_WIDTH, UNISMACK_BANNER_HEIGHT);
    }
}

// ppc 1.10b: 10054c40 (1.00 10083f2c, 1.13d 1005ac60, carbon 1.10b 103a3bf0, carbon 1.13d 103d3340)
void __fastcall UniCloseSmacker(D2SmackerContextStrc * pContext) {
    if (gpUniSmackBuffer) {
        Fog::Memory::FreeClientMemory(gpUniSmackBuffer, (char *)"uniSmack.cpp", 256, 0);
        gpUniSmackBuffer = nullptr;
    }
    if (pContext->eMode == 's' && pContext->hSmack) {
        SmackClose(pContext->hSmack);
        pContext->hSmack = nullptr;
    }
    pContext->eMode = 0;
}

// ppc 1.10b: 10054b30 (1.13d 1005ab50, carbon 1.10b 103a3ae0, carbon 1.13d 103d3230)
// Copies the buffer through the remap into the back buffer, the image's bottom-left corner at the
// context's (nX, nY).
static void sUniSmackBlit(D2SmackerContextStrc * pContext, uint32_t nWidth, int nHeight) {
    if (!gpUniSmackBuffer) {
        return;
    }

    BYTE * pDest = gpUniBackBuffer + gnUniBackBufferWidth * (pContext->nY - nHeight) + pContext->nX;
    const BYTE * pSrc = gpUniSmackBuffer;
    while (nHeight--) {
        for (uint32_t i = 0; i < nWidth; i++) {
            pDest[i] = gabUniSmackRemap[pSrc[i]];
        }
        pSrc += nWidth;
        pDest += gnUniBackBufferWidth;
    }
}

#endif // !defined(_WIN32) && ((D2_MAC_CARBON && D2_ARCH_PPC) || D2_BACKEND_POSIX)
