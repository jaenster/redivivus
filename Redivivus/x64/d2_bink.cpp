// Bink (binkw32.dll) for the 64-bit build: the ten entry points the game calls,
// decoding through FFmpeg's bink/binkaudio (d2_media.h).
//
// The game's loop (DirectDraw.cpp BINK_PlayVideoFile, 005137e0, and its D3D and
// Glide twins): BinkOpen on the MPQ file handle Storm positioned, then until
// FrameNum == Frames: BinkWait says whether the frame is due; BinkDoFrame
// decodes it; BinkCopyToBuffer writes it into the locked surface in the
// surface's pixel format at the centring offset; BinkNextFrame advances. The
// shim keeps those semantics: FrameNum is 1-based and counts the frame DoFrame
// will decode next, Wait follows the audio clock while the track plays, and the
// 2X open flags double the reported Width/Height and the copy scales to it, as
// the 640x240 "BlizNorth640x240.bik" relies on to fill 640x480.
#include "d2_x64deps.h"
#include "d2_media.h"

#include <ddraw.h>
#include <new>
#include <stdlib.h>
#include <string.h>

namespace {

struct BinkFile {
    BINK pub;  // the public head: Width, Height, Frames, FrameNum
    D2Media media;
    bool bFrameReady = false;     // pFrame holds the frame numbered FrameNum
    uint32_t nScaleX = 1;
    uint32_t nScaleY = 1;
    SwsContext* pSws = nullptr;
    AVPixelFormat swsFormat = AV_PIX_FMT_NONE;
};

bool g_bSoundSystem = false;

// The destination format of a BINKSURFACE constant, and its bytes per pixel.
// 8P is Bink's dithered palette output, which no D2 surface in 1.14d is; the
// nearest single-plane stand-in is grey, kept so a copy into such a surface at
// least shows the picture.
bool SurfaceFormat(uint32_t nType, AVPixelFormat* pFmt, int* pBpp) {
    switch (nType & BINKSURFACEMASK) {
        case BINKSURFACE8P:   *pFmt = AV_PIX_FMT_GRAY8;    *pBpp = 1; return true;
        case BINKSURFACE24:   *pFmt = AV_PIX_FMT_BGR24;    *pBpp = 3; return true;
        case BINKSURFACE24R:  *pFmt = AV_PIX_FMT_RGB24;    *pBpp = 3; return true;
        case BINKSURFACE32:
        case BINKSURFACE32A:  *pFmt = AV_PIX_FMT_BGRA;     *pBpp = 4; return true;
        case BINKSURFACE32R:
        case BINKSURFACE32RA: *pFmt = AV_PIX_FMT_RGBA;     *pBpp = 4; return true;
        case BINKSURFACE555:
        case BINKSURFACE5551: *pFmt = AV_PIX_FMT_RGB555LE; *pBpp = 2; return true;
        case BINKSURFACE565:  *pFmt = AV_PIX_FMT_RGB565LE; *pBpp = 2; return true;
        case BINKSURFACEYUY2: *pFmt = AV_PIX_FMT_YUYV422;  *pBpp = 2; return true;
        case BINKSURFACEUYVY: *pFmt = AV_PIX_FMT_UYVY422;  *pBpp = 2; return true;
        default: return false;  // 4444, 655, 664, planar YV12: nothing in the tree asks for them
    }
}

}  // namespace

extern "C" {

void* __stdcall BinkOpen(void* hFile, uint32_t dwFlags) {
    if (!D2FFmpeg_Get()) return nullptr;
    BinkFile* pBink = new (std::nothrow) BinkFile;
    if (!pBink) return nullptr;
    memset(&pBink->pub, 0, sizeof pBink->pub);

    bool bOpened;
    if (dwFlags & BINKFILEHANDLE) {
        // The stream's length is in its own header: bytes 4..7 hold the file size
        // less eight. Storm's handle may be the whole MPQ archive, so this bound
        // is what stops the demuxer reading past the video.
        HANDLE h = (HANDLE)hFile;
        LARGE_INTEGER liZero, liPos;
        liZero.QuadPart = 0;
        int64_t nSize = 0;
        if (SetFilePointerEx(h, liZero, &liPos, FILE_CURRENT)) {
            uint8_t aHeader[8];
            DWORD nRead = 0;
            if (ReadFile(h, aHeader, 8, &nRead, nullptr) && nRead == 8 && aHeader[0] == 'B' &&
                aHeader[1] == 'I' && aHeader[2] == 'K') {
                nSize = (int64_t)(aHeader[4] | aHeader[5] << 8 | aHeader[6] << 16 | (uint32_t)aHeader[7] << 24) + 8;
            }
            SetFilePointerEx(h, liPos, nullptr, FILE_BEGIN);
        }
        bOpened = pBink->media.OpenHandle(h, nSize, g_bSoundSystem);
    } else {
        bOpened = pBink->media.OpenPath((const char*)hFile, g_bSoundSystem);
    }
    if (!bOpened || pBink->media.nWidth == 0 || pBink->media.nHeight == 0) {
        delete pBink;
        return nullptr;
    }

    switch (dwFlags & 0x70000000u) {
        case BINKCOPY2XH:
        case BINKCOPY2XHI:  pBink->nScaleY = 2; break;
        case BINKCOPY2XW:   pBink->nScaleX = 2; break;
        case BINKCOPY2XWH:
        case BINKCOPY2XWHI: pBink->nScaleX = pBink->nScaleY = 2; break;
        default: break;
    }
    pBink->pub.Width = pBink->media.nWidth * pBink->nScaleX;
    pBink->pub.Height = pBink->media.nHeight * pBink->nScaleY;
    pBink->pub.Frames = pBink->media.nFrames;
    pBink->pub.FrameNum = 1;
    if (pBink->pub.Frames == 0) {
        // No count in the container: the loop's end test needs one. Count by
        // duration; a short count only ends the video a frame early.
        AVStream* pVs = pBink->media.pFormat->streams[pBink->media.nVideoStream];
        if (pVs->duration > 0 && pVs->time_base.num > 0) {
            pBink->pub.Frames = (uint32_t)(pVs->duration * pVs->time_base.num * pBink->media.frameRate.num /
                                           ((int64_t)pVs->time_base.den * pBink->media.frameRate.den));
        }
        if (pBink->pub.Frames == 0) pBink->pub.Frames = 1;
    }
    return pBink;
}

void __stdcall BinkClose(void* pHandle) {
    BinkFile* pBink = (BinkFile*)pHandle;
    if (!pBink) return;
    if (pBink->pSws) pBink->media.ff->sws_freeContext(pBink->pSws);
    delete pBink;
}

// 1 while frame FrameNum is not yet due. The first frame is always due: that
// is what starts the clock.
int32_t __stdcall BinkWait(void* pHandle) {
    BinkFile* pBink = (BinkFile*)pHandle;
    if (!pBink || !pBink->media.Started()) return 0;
    double fDueMs = (double)(pBink->pub.FrameNum - 1) * 1000.0 * pBink->media.frameRate.den / pBink->media.frameRate.num;
    return pBink->media.ClockMs() < fDueMs ? 1 : 0;
}

// Decodes frame FrameNum. Returns 0; Bink's non-zero means it skipped the frame
// to catch up, which this shim never does (the game does not look).
int32_t __stdcall BinkDoFrame(void* pHandle) {
    BinkFile* pBink = (BinkFile*)pHandle;
    if (!pBink) return 0;
    if (!pBink->bFrameReady) {
        int rc = pBink->media.NextVideoFrame();
        if (rc > 0) pBink->bFrameReady = true;
        // At end of stream the last decoded frame stays; FrameNum reaching Frames ends the loop.
        if (!pBink->media.Started()) pBink->media.Start();
    }
    return 0;
}

void __stdcall BinkNextFrame(void* pHandle) {
    BinkFile* pBink = (BinkFile*)pHandle;
    if (!pBink) return;
    pBink->bFrameReady = false;
    if (pBink->pub.FrameNum < pBink->pub.Frames) {
        pBink->pub.FrameNum++;
    } else {
        // Bink loops to frame 1 here; the game has closed by then. Hold.
        pBink->pub.FrameNum = pBink->pub.Frames;
    }
}

int32_t __stdcall BinkCopyToBuffer(void* pHandle, void* pDest, int32_t nDestPitch, uint32_t nDestHeight,
                                   uint32_t nDestX, uint32_t nDestY, uint32_t dwFlags) {
    BinkFile* pBink = (BinkFile*)pHandle;
    if (!pBink || !pDest || !pBink->media.pFrame || !pBink->media.pFrame->data[0]) return 0;
    AVPixelFormat fmt;
    int nBpp;
    if (!SurfaceFormat(dwFlags, &fmt, &nBpp)) return 0;
    const D2FFmpeg* ff = pBink->media.ff;
    AVFrame* pSrc = pBink->media.pFrame;
    if (pBink->pSws && pBink->swsFormat != fmt) {
        ff->sws_freeContext(pBink->pSws);
        pBink->pSws = nullptr;
    }
    if (!pBink->pSws) {
        // Pixel doubling for the 2X modes, as Bink does; no filtering at 1:1 either.
        pBink->pSws = ff->sws_getContext(pSrc->width, pSrc->height, (AVPixelFormat)pSrc->format,
                                         (int)pBink->pub.Width, (int)pBink->pub.Height, fmt,
                                         SWS_POINT, nullptr, nullptr, nullptr);
        pBink->swsFormat = fmt;
        if (!pBink->pSws) return 0;
    }
    uint32_t nRows = pBink->pub.Height;
    if (nDestHeight && nDestHeight < nRows) nRows = nDestHeight;
    uint8_t* pOut = (uint8_t*)pDest + (int64_t)nDestY * nDestPitch + (int64_t)nDestX * nBpp;
    uint8_t* apDst[4] = {pOut, nullptr, nullptr, nullptr};
    int aDstStride[4] = {nDestPitch, 0, 0, 0};
    // sws_scale writes output rows [0, sliceH) scaled from the source slice; a
    // shorter destination is cut by handing it fewer source rows.
    int nSrcRows = pSrc->height;
    if (nRows < pBink->pub.Height) nSrcRows = (int)((int64_t)nRows * pSrc->height / pBink->pub.Height);
    ff->sws_scale(pBink->pSws, pSrc->data, pSrc->linesize, 0, nSrcRows, apDst, aDstStride);
    return 0;
}

// BinkSetSoundSystem(BinkOpenDirectSound, 0): turn the audio track on. The
// device route is the shim's (d2_pcmout.h), so the opener is only a token.
int32_t __stdcall BinkSetSoundSystem(void* (__stdcall* pfnOpen)(uint32_t), uint32_t dwParam) {
    if (!pfnOpen) {
        g_bSoundSystem = false;
        return 0;
    }
    g_bSoundSystem = pfnOpen(dwParam) != nullptr;
    return g_bSoundSystem ? 1 : 0;
}

void* __stdcall BinkOpenDirectSound(uint32_t dwParam) {
    (void)dwParam;
    static int s_nToken;
    return &s_nToken;
}

// The BINKSURFACE constant for a DirectDraw surface's pixel format, read the way
// Bink did: IDirectDrawSurface::GetPixelFormat (the same vtable slot in every
// surface interface version).
uint32_t __stdcall BinkDDSurfaceType(void* pDDSurface) {
    if (!pDDSurface) return (uint32_t)-1;
    LPDIRECTDRAWSURFACE pSurface = (LPDIRECTDRAWSURFACE)pDDSurface;
    DDPIXELFORMAT ddpf;
    memset(&ddpf, 0, sizeof ddpf);
    ddpf.dwSize = sizeof ddpf;
    if (FAILED(IDirectDrawSurface_GetPixelFormat(pSurface, &ddpf))) return (uint32_t)-1;
    if (ddpf.dwFlags & DDPF_PALETTEINDEXED8) return BINKSURFACE8P;
    if (ddpf.dwFlags & DDPF_FOURCC) {
        if (ddpf.dwFourCC == MAKEFOURCC('Y', 'U', 'Y', '2')) return BINKSURFACEYUY2;
        if (ddpf.dwFourCC == MAKEFOURCC('U', 'Y', 'V', 'Y')) return BINKSURFACEUYVY;
        if (ddpf.dwFourCC == MAKEFOURCC('Y', 'V', '1', '2')) return BINKSURFACEYV12;
        return (uint32_t)-1;
    }
    if (!(ddpf.dwFlags & DDPF_RGB)) return (uint32_t)-1;
    bool bAlpha = (ddpf.dwFlags & DDPF_ALPHAPIXELS) != 0;
    switch (ddpf.dwRGBBitCount) {
        case 32:
            if (ddpf.dwRBitMask == 0x00FF0000) return bAlpha ? BINKSURFACE32A : BINKSURFACE32;
            if (ddpf.dwRBitMask == 0x000000FF) return bAlpha ? BINKSURFACE32RA : BINKSURFACE32R;
            return BINKSURFACE32;
        case 24:
            return ddpf.dwRBitMask == 0x00FF0000 ? BINKSURFACE24 : BINKSURFACE24R;
        case 16:
            if (ddpf.dwGBitMask == 0x07E0) return BINKSURFACE565;
            if (ddpf.dwGBitMask == 0x03E0) return bAlpha ? BINKSURFACE5551 : BINKSURFACE555;
            if (ddpf.dwGBitMask == 0x00F0) return BINKSURFACE4444;
            if (ddpf.dwGBitMask == 0x07C0) return BINKSURFACE655;
            if (ddpf.dwGBitMask == 0x03F0) return BINKSURFACE664;
            return BINKSURFACE565;
        default:
            return (uint32_t)-1;
    }
}

}  // extern "C"
