// Drives the 64-bit Bink, Smacker and IJL shims the way the game does, without
// the game: opens a .bik through a file HANDLE (BINKFILEHANDLE, as Storm hands
// it over) and a .smk by path, decodes every frame through Wait/DoFrame/Copy/
// NextFrame, writes a few frames as PNG, and writes a JPEG through ijlWrite.
//
//   x64deps_test <out-dir> [--bik file.bik] [--smk file.smk] [--paced] [--frames n,n,...]
//
// Without --paced the loop ignores BinkWait/SmackWait and decodes as fast as it
// can; with it the loop paces as the game does and the report shows the wall
// time against the stream's nominal duration, which is the A/V sync evidence.
// FFmpeg's DLLs must sit beside the executable (docs/building.md). Exit code 0
// means every requested step produced its file.
#include "Redivivus/x64/d2_x64deps.h"

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#define STB_IMAGE_WRITE_STATIC
#include "stb_image_write.h"

static const char* g_pszOut = ".";
static bool g_bPaced = false;
static int g_aDumpFrames[8] = {1, 10, 50, -1};
static int g_nFailures = 0;

static void Fail(const char* pszFmt, const char* pszArg = "") {
    printf("FAIL %s%s\n", pszFmt, pszArg);
    ++g_nFailures;
}

static bool WantFrame(uint32_t nFrame, uint32_t nLast) {
    for (int i = 0; g_aDumpFrames[i] > 0; ++i) {
        if ((uint32_t)g_aDumpFrames[i] == nFrame) return true;
    }
    return nFrame == nLast;
}

static void WritePng(const char* pszName, uint32_t nFrame, const uint8_t* pRgb, int nW, int nH) {
    char szPath[MAX_PATH];
    snprintf(szPath, sizeof szPath, "%s\\%s-%03u.png", g_pszOut, pszName, nFrame);
    if (!stbi_write_png(szPath, nW, nH, 3, pRgb, nW * 3)) Fail("png write ", szPath);
    else printf("wrote %s\n", szPath);
}

// --- Bink ---------------------------------------------------------------
static void TestBink(const char* pszFile) {
    HANDLE hFile = CreateFileA(pszFile, GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, 0, nullptr);
    if (hFile == INVALID_HANDLE_VALUE) return Fail("open ", pszFile);
    if (!BinkSetSoundSystem(BinkOpenDirectSound, 0)) Fail("BinkSetSoundSystem");
    // The game's "resolution mode 2" open: twice the height, as for the 640x240 logo clips.
    BINK* pBink = (BINK*)BinkOpen(hFile, BINKFILEHANDLE | BINKCOPY2XH);
    if (!pBink) {
        CloseHandle(hFile);
        return Fail("BinkOpen returned NULL for ", pszFile);
    }
    printf("bink %s: %ux%u, %u frames, FrameNum=%u\n", pszFile, pBink->Width, pBink->Height, pBink->Frames, pBink->FrameNum);
    int nPitch = (int)pBink->Width * 4;
    uint8_t* pSurface = (uint8_t*)calloc((size_t)nPitch * pBink->Height, 1);
    uint8_t* pRgb = (uint8_t*)malloc((size_t)pBink->Width * pBink->Height * 3);
    ULONGLONG nStart = GetTickCount64();
    uint32_t nDecoded = 0;
    for (;;) {
        if (g_bPaced && BinkWait(pBink)) {
            Sleep(1);
            continue;
        }
        BinkDoFrame(pBink);
        BinkCopyToBuffer(pBink, pSurface, nPitch, pBink->Height, 0, 0, BINKSURFACE32);
        ++nDecoded;
        if (WantFrame(pBink->FrameNum, pBink->Frames)) {
            for (uint32_t i = 0; i < pBink->Width * pBink->Height; ++i) {
                pRgb[i * 3 + 0] = pSurface[i * 4 + 2];
                pRgb[i * 3 + 1] = pSurface[i * 4 + 1];
                pRgb[i * 3 + 2] = pSurface[i * 4 + 0];
            }
            WritePng("bink", pBink->FrameNum, pRgb, (int)pBink->Width, (int)pBink->Height);
        }
        if (pBink->FrameNum == pBink->Frames) break;
        BinkNextFrame(pBink);
    }
    ULONGLONG nElapsed = GetTickCount64() - nStart;
    printf("bink: decoded %u frames in %llu ms%s\n", nDecoded, nElapsed, g_bPaced ? " (paced)" : "");
    if (nDecoded != pBink->Frames) Fail("bink frame count mismatch");
    // Not black: a decoded picture has more than one value in it.
    uint8_t nFirst = pSurface[0];
    bool bVaries = false;
    for (size_t i = 0; i < (size_t)nPitch * pBink->Height && !bVaries; ++i) bVaries = pSurface[i] != nFirst;
    if (!bVaries) Fail("bink last frame is flat");
    BinkClose(pBink);
    CloseHandle(hFile);
    free(pSurface);
    free(pRgb);
}

// --- Smacker --------------------------------------------------------------
static void TestSmack(const char* pszFile) {
    Smack* pSmk = (Smack*)SmackOpen(pszFile, SMACKTRACKS, SMACKAUTOEXTRA);
    if (!pSmk) return Fail("SmackOpen returned NULL for ", pszFile);
    printf("smack %s: %ux%u, %u frames, %u ms/frame\n", pszFile, pSmk->Width, pSmk->Height, pSmk->Frames, pSmk->MSPerFrame);
    // The GDI renderer's use: 8-bit indices, pitch = width, palette rebuilt on NewPalette.
    uint8_t* pIndices = (uint8_t*)calloc((size_t)pSmk->Width * pSmk->Height, 1);
    uint8_t* pRgb = (uint8_t*)malloc((size_t)pSmk->Width * pSmk->Height * 3);
    uint8_t aPal[768] = {};
    SmackToBuffer(pSmk, 0, 0, pSmk->Width, pSmk->Height, pIndices, 0);
    ULONGLONG nStart = GetTickCount64();
    uint32_t nDecoded = 0, nPaletteChanges = 0;
    for (;;) {
        if (g_bPaced && SmackWait(pSmk)) {
            Sleep(1);
            continue;
        }
        SmackDoFrame(pSmk);
        if (pSmk->NewPalette) {
            memcpy(aPal, pSmk->Palette, sizeof(aPal));
            ++nPaletteChanges;
        }
        ++nDecoded;
        if (WantFrame(pSmk->FrameNum + 1, pSmk->Frames)) {
            for (uint32_t i = 0; i < pSmk->Width * pSmk->Height; ++i) {
                memcpy(pRgb + i * 3, aPal + pIndices[i] * 3, 3);
            }
            WritePng("smack", pSmk->FrameNum, pRgb, (int)pSmk->Width, (int)pSmk->Height);
        }
        if (pSmk->FrameNum == pSmk->Frames) break;  // the game's end test
        SmackNextFrame(pSmk);
    }
    ULONGLONG nElapsed = GetTickCount64() - nStart;
    printf("smack: %u DoFrame calls, %u palette changes, %llu ms%s\n", nDecoded, nPaletteChanges, nElapsed,
           g_bPaced ? " (paced)" : "");
    if (nDecoded != pSmk->Frames + 1) Fail("smack frame count mismatch");
    if (!nPaletteChanges) Fail("smack never published a palette");
    SmackClose(pSmk);

    // The Direct3D renderer's use: 16-bit 555 into a 1024-byte-pitch texture.
    pSmk = (Smack*)SmackOpen(pszFile, SMACKTRACKS, SMACKAUTOEXTRA);
    if (!pSmk) {
        free(pIndices);
        free(pRgb);
        return Fail("second SmackOpen failed");
    }
    uint32_t nPitch = 0x400;
    uint8_t* pTex = (uint8_t*)calloc((size_t)nPitch * pSmk->Height, 1);
    SmackToBuffer(pSmk, 0, 0, nPitch, pSmk->Height, pTex, SMACKBUFFER555);
    SmackDoFrame(pSmk);
    for (uint32_t y = 0; y < pSmk->Height; ++y) {
        for (uint32_t x = 0; x < pSmk->Width && x < nPitch / 2; ++x) {
            uint16_t v = ((uint16_t*)(pTex + y * nPitch))[x];
            uint8_t* pOut = pRgb + ((size_t)y * pSmk->Width + x) * 3;
            pOut[0] = (uint8_t)(((v >> 10) & 31) << 3);
            pOut[1] = (uint8_t)(((v >> 5) & 31) << 3);
            pOut[2] = (uint8_t)((v & 31) << 3);
        }
    }
    WritePng("smack555", 0, pRgb, (int)pSmk->Width, (int)pSmk->Height);
    SmackClose(pSmk);
    free(pTex);
    free(pIndices);
    free(pRgb);
}

// --- IJL ------------------------------------------------------------------
// A 0x4e40-byte block as the game allocates, zeroed, filled the way
// D2WIN_SaveScreenshotToFile fills it: RGB rows padded to 4 bytes, quality 100.
static void TestIjl() {
    const int nW = 203, nH = 120;  // odd width: exercises DIBPadBytes
    uint32_t nPad = ((nW * 3 + 3) & ~3u) - nW * 3;
    size_t nStride = (size_t)nW * 3 + nPad;
    uint8_t* pDib = (uint8_t*)calloc(nStride * nH, 1);
    for (int y = 0; y < nH; ++y) {
        for (int x = 0; x < nW; ++x) {
            uint8_t* p = pDib + y * nStride + x * 3;
            p[0] = (uint8_t)(x * 255 / nW);        // red grows to the right
            p[1] = (uint8_t)(y * 255 / nH);        // green grows downward
            p[2] = (uint8_t)((x / 16 + y / 16) & 1 ? 200 : 40);  // blue checker
        }
    }
    uint8_t* pProps = (uint8_t*)calloc(0x4e40, 1);
    JPEG_CORE_PROPERTIES_HEAD* p = (JPEG_CORE_PROPERTIES_HEAD*)pProps;
    char szPath[MAX_PATH];
    snprintf(szPath, sizeof szPath, "%s\\ijl-screenshot.jpg", g_pszOut);
    if (ijlInit(p) != IJL_OK) Fail("ijlInit");
    p->DIBBytes = pDib;
    p->DIBColor = IJL_RGB;
    p->DIBPadBytes = nPad;
    p->JPGFile = szPath;
    p->jquality = 100;
    p->DIBWidth = nW;
    p->DIBHeight = nH;
    p->JPGWidth = nW;
    p->JPGHeight = nH;
    int rc = ijlWrite(p, IJL_JFILE_WRITEWHOLEIMAGE);
    if (rc != IJL_OK) Fail("ijlWrite");
    if (ijlFree(p) != IJL_OK) Fail("ijlFree");
    HANDLE h = CreateFileA(szPath, GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, 0, nullptr);
    if (h == INVALID_HANDLE_VALUE) {
        Fail("jpeg not written ", szPath);
    } else {
        uint8_t aMagic[3] = {};
        DWORD nRead = 0;
        ReadFile(h, aMagic, 3, &nRead, nullptr);
        DWORD nSize = GetFileSize(h, nullptr);
        CloseHandle(h);
        if (nRead != 3 || aMagic[0] != 0xFF || aMagic[1] != 0xD8 || aMagic[2] != 0xFF) Fail("jpeg magic wrong");
        printf("wrote %s (%lu bytes)\n", szPath, (unsigned long)nSize);
    }
    free(pProps);
    free(pDib);
}

int main(int argc, char** argv) {
    const char* pszBik = nullptr;
    const char* pszSmk = nullptr;
    if (argc < 2) {
        fprintf(stderr, "usage: x64deps_test <out-dir> [--bik f] [--smk f] [--paced] [--frames n,n,...]\n");
        return 2;
    }
    g_pszOut = argv[1];
    setvbuf(stdout, nullptr, _IONBF, 0);
    for (int i = 2; i < argc; ++i) {
        if (!strcmp(argv[i], "--bik") && i + 1 < argc) pszBik = argv[++i];
        else if (!strcmp(argv[i], "--smk") && i + 1 < argc) pszSmk = argv[++i];
        else if (!strcmp(argv[i], "--paced")) g_bPaced = true;
        else if (!strcmp(argv[i], "--frames") && i + 1 < argc) {
            int n = 0;
            for (char* pTok = strtok(argv[++i], ","); pTok && n < 7; pTok = strtok(nullptr, ",")) g_aDumpFrames[n++] = atoi(pTok);
            g_aDumpFrames[n] = -1;
        }
    }
    CreateDirectoryA(g_pszOut, nullptr);
    if (pszBik) TestBink(pszBik);
    if (pszSmk) TestSmack(pszSmk);
    TestIjl();
    printf("%s: %d failure(s)\n", g_nFailures ? "FAILED" : "OK", g_nFailures);
    return g_nFailures ? 1 : 0;
}
