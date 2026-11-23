// Smacker (smackw32.dll) for the 64-bit build: the six entry points the game
// calls, decoding through FFmpeg's smackvid/smackaud (d2_media.h).
//
// The game's loop (Windowed.cpp DecodeSmacker 006c9710 and the DDraw, D3D and
// Glide renderers' copies): SmackOpen(path, SMACKTRACKS, SMACKAUTOEXTRA);
// SmackToBuffer once, naming the buffer every later SmackDoFrame writes into,
// 8-bit indices (flags 0) or 16-bit 555 (0x80000000); then per frame SmackWait,
// SmackDoFrame, a palette rebuild when NewPalette is set, and either
// SmackNextFrame or - once FrameNum == Frames - close. FrameNum is 0-based, the
// frame DoFrame decodes next; NextFrame past the last frame leaves it at Frames,
// where DoFrame redraws the last picture and the game's test ends the playback.
#include "d2_x64deps.h"
#include "d2_media.h"

#include <new>
#include <stdlib.h>
#include <string.h>

namespace {

struct SmackFile {
    Smack pub;  // the RAD header the callers index
    D2Media media;
    bool bFrameReady = false;
    bool bHavePalette = false;
    uint32_t aPalette[256] = {};  // the decoder's palette, 0xAARRGGBB, for change detection
    // SmackToBuffer's destination.
    uint8_t* pDest = nullptr;
    uint32_t nDestLeft = 0;
    uint32_t nDestTop = 0;
    uint32_t nDestPitch = 0;
    uint32_t nDestHeight = 0;
    uint32_t nDestFlags = 0;
};

// Writes the current PAL8 frame into the destination the caller named.
void Blit(SmackFile* pSmk) {
    AVFrame* pFrame = pSmk->media.pFrame;
    if (!pSmk->pDest || !pFrame || !pFrame->data[0]) return;
    uint32_t nW = pSmk->pub.Width;
    uint32_t nH = pSmk->pub.Height;
    if ((uint32_t)pFrame->width < nW) nW = (uint32_t)pFrame->width;
    if ((uint32_t)pFrame->height < nH) nH = (uint32_t)pFrame->height;
    bool b16 = (pSmk->nDestFlags & SMACKBUFFER16) != 0;
    bool b565 = (pSmk->nDestFlags & SMACKBUFFER16) == SMACKBUFFER565;
    bool bReversed = (pSmk->nDestFlags & SMACKBUFFERREVERSED) != 0;
    uint16_t a16[256];
    if (b16) {
        for (int i = 0; i < 256; ++i) {
            uint32_t c = pSmk->aPalette[i];
            uint32_t r = (c >> 16) & 0xFF, g = (c >> 8) & 0xFF, b = c & 0xFF;
            a16[i] = b565 ? (uint16_t)((r >> 3) << 11 | (g >> 2) << 5 | (b >> 3))
                          : (uint16_t)((r >> 3) << 10 | (g >> 3) << 5 | (b >> 3));
        }
    }
    for (uint32_t y = 0; y < nH; ++y) {
        uint32_t nRow = pSmk->nDestTop + y;
        if (bReversed) {
            if (pSmk->nDestHeight == 0 || nRow >= pSmk->nDestHeight) break;
            nRow = pSmk->nDestHeight - 1 - nRow;
        }
        const uint8_t* pSrc = pFrame->data[0] + (size_t)y * pFrame->linesize[0];
        uint8_t* pRow = pSmk->pDest + (size_t)nRow * pSmk->nDestPitch;
        if (b16) {
            uint16_t* pOut = (uint16_t*)pRow + pSmk->nDestLeft;
            for (uint32_t x = 0; x < nW; ++x) pOut[x] = a16[pSrc[x]];
        } else {
            memcpy(pRow + pSmk->nDestLeft, pSrc, nW);
        }
    }
}

// Takes the frame's palette into the public header; NewPalette says whether it
// changed. FFmpeg hands PAL8 palettes as 256 native-endian 0xAARRGGBB words in
// data[1]; the SDK publishes 8-bit R,G,B triplets.
void TakePalette(SmackFile* pSmk) {
    AVFrame* pFrame = pSmk->media.pFrame;
    if (!pFrame || !pFrame->data[1]) {
        pSmk->pub.NewPalette = 0;
        return;
    }
    const uint32_t* pPal = (const uint32_t*)pFrame->data[1];
    bool bChanged = !pSmk->bHavePalette || memcmp(pPal, pSmk->aPalette, sizeof pSmk->aPalette) != 0;
    if (bChanged) {
        memcpy(pSmk->aPalette, pPal, sizeof pSmk->aPalette);
        for (int i = 0; i < 256; ++i) {
            pSmk->pub.Palette[i * 3 + 0] = (uint8_t)(pPal[i] >> 16);
            pSmk->pub.Palette[i * 3 + 1] = (uint8_t)(pPal[i] >> 8);
            pSmk->pub.Palette[i * 3 + 2] = (uint8_t)pPal[i];
        }
        pSmk->bHavePalette = true;
    }
    pSmk->pub.NewPalette = bChanged ? 1 : 0;
}

// The SMK header words the SDK publishes and FFmpeg's demuxer does not: the
// frame count at 12 (without the ring frame), the rate at 16 (positive: ms per
// frame; negative: 10-microsecond units per frame; 0: 10 fps) and the flags at
// 20 (bit 0: a ring frame follows the last frame, for seamless looping).
struct SmkHeader {
    uint32_t nFrames;
    uint32_t nMsPerFrame;
    uint32_t nFlags;
};

bool ReadHeader(HANDLE hFile, SmkHeader* pOut) {
    uint8_t a[24];
    DWORD nRead = 0;
    LARGE_INTEGER liZero, liPos;
    liZero.QuadPart = 0;
    if (!SetFilePointerEx(hFile, liZero, &liPos, FILE_CURRENT)) return false;
    bool bOk = ReadFile(hFile, a, sizeof a, &nRead, nullptr) && nRead == sizeof a && a[0] == 'S' && a[1] == 'M' && a[2] == 'K';
    SetFilePointerEx(hFile, liPos, nullptr, FILE_BEGIN);
    if (!bOk) return false;
    auto rd = [&](int i) { return (uint32_t)(a[i] | a[i + 1] << 8 | a[i + 2] << 16 | (uint32_t)a[i + 3] << 24); };
    pOut->nFrames = rd(12);
    int32_t nRate = (int32_t)rd(16);
    pOut->nMsPerFrame = nRate > 0 ? (uint32_t)nRate : nRate < 0 ? (uint32_t)(-(int64_t)nRate / 100) : 100;
    pOut->nFlags = rd(20);
    return true;
}

}  // namespace

extern "C" {

void* __stdcall SmackOpen(const char* szName, uint32_t dwFlags, uint32_t dwExtraBuf) {
    (void)dwExtraBuf;
    if (!szName || !D2FFmpeg_Get()) return nullptr;
    SmackFile* pSmk = new (std::nothrow) SmackFile;
    if (!pSmk) return nullptr;
    memset(&pSmk->pub, 0, sizeof pSmk->pub);
    bool bAudio = (dwFlags & SMACKTRACKS) != 0;
    SmkHeader hdr = {};
    bool bOpened;
    if (dwFlags & SMACKFILEHANDLE) {
        HANDLE h = (HANDLE)(uintptr_t)szName;
        bOpened = ReadHeader(h, &hdr) && pSmk->media.OpenHandle(h, 0, bAudio);
    } else {
        HANDLE h = CreateFileA(szName, GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, 0, nullptr);
        bOpened = h != INVALID_HANDLE_VALUE && ReadHeader(h, &hdr);
        if (h != INVALID_HANDLE_VALUE) CloseHandle(h);
        bOpened = bOpened && pSmk->media.OpenPath(szName, bAudio);
    }
    if (!bOpened || pSmk->media.nWidth == 0 || pSmk->media.nHeight == 0 ||
        pSmk->media.pVideo->pix_fmt != AV_PIX_FMT_PAL8) {
        delete pSmk;
        return nullptr;
    }
    // The header words the file carries and the callers may read.
    pSmk->pub.Version = 'S' | 'M' << 8 | 'K' << 16 | '2' << 24;
    pSmk->pub.Width = pSmk->media.nWidth;
    pSmk->pub.Height = pSmk->media.nHeight;
    pSmk->pub.Frames = hdr.nFrames ? hdr.nFrames : 1;
    pSmk->pub.MSPerFrame = hdr.nMsPerFrame ? hdr.nMsPerFrame : 100;
    pSmk->pub.SmackerType = hdr.nFlags;
    pSmk->pub.PalType = 0;
    pSmk->pub.FrameNum = 0;
    return pSmk;
}

void __stdcall SmackClose(void* pHandle) {
    delete (SmackFile*)pHandle;
}

int32_t __stdcall SmackWait(void* pHandle) {
    SmackFile* pSmk = (SmackFile*)pHandle;
    if (!pSmk || !pSmk->media.Started()) return 0;
    double fDueMs = (double)pSmk->pub.FrameNum * 1000.0 * pSmk->media.frameRate.den / pSmk->media.frameRate.num;
    return pSmk->media.ClockMs() < fDueMs ? 1 : 0;
}

// Decodes frame FrameNum into the SmackToBuffer destination and publishes its
// palette. Returns 0; Smacker's non-zero flagged a dropped frame.
int32_t __stdcall SmackDoFrame(void* pHandle) {
    SmackFile* pSmk = (SmackFile*)pHandle;
    if (!pSmk) return 0;
    if (!pSmk->bFrameReady) {
        int rc = pSmk->media.NextVideoFrame();
        pSmk->bFrameReady = true;
        if (!pSmk->media.Started()) pSmk->media.Start();
        if (rc > 0) TakePalette(pSmk);
        else pSmk->pub.NewPalette = 0;
    } else {
        pSmk->pub.NewPalette = 0;
    }
    Blit(pSmk);
    return 0;
}

void __stdcall SmackNextFrame(void* pHandle) {
    SmackFile* pSmk = (SmackFile*)pHandle;
    if (!pSmk) return;
    pSmk->bFrameReady = false;
    if (pSmk->pub.FrameNum < pSmk->pub.Frames) pSmk->pub.FrameNum++;
}

void __stdcall SmackToBuffer(void* pHandle, uint32_t nLeft, uint32_t nTop, uint32_t nPitch, uint32_t nDestHeight,
                             void* pBuffer, uint32_t dwFlags) {
    SmackFile* pSmk = (SmackFile*)pHandle;
    if (!pSmk) return;
    pSmk->pDest = (uint8_t*)pBuffer;
    pSmk->nDestLeft = nLeft;
    pSmk->nDestTop = nTop;
    pSmk->nDestPitch = nPitch;
    pSmk->nDestHeight = nDestHeight;
    pSmk->nDestFlags = dwFlags;
}

}  // extern "C"
