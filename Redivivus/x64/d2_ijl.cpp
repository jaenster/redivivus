// The Intel JPEG Library (ijl11.dll) for the 64-bit build: the three entry
// points the screenshot writer calls (D2WinMain.cpp D2WIN_SaveScreenshotToFile,
// 004fa010): ijlInit, ijlWrite(IJL_JFILE_WRITEWHOLEIMAGE), ijlFree. The JPEG is
// encoded by stb_image_write (3rdparty/stb).
//
// What the game hands over: DIBBytes with 3 bytes per pixel in R,G,B order
// (DIBColor = IJL_RGB), rows padded to 4 bytes (DIBPadBytes), DIBWidth x
// DIBHeight with DIBHeight positive, quality 100, the file name in JPGFile.
// The renderers fill that buffer from their back buffer top row first
// (DirectDraw.cpp GetBackBuffer 675, Windowed.cpp 238), and the original's
// screenshots come out upright - so a positive DIBHeight is a top-down buffer
// here, and a negative one is read bottom-up.
#include "d2_x64deps.h"

#include <stdlib.h>
#include <string.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#define STB_IMAGE_WRITE_STATIC
#define STBI_WRITE_NO_STDIO
#include "stb_image_write.h"

#if defined(_WIN32)
#include <windows.h>
#else
// The modern macOS build (cmake/mac-modern.cmake): the same file calls, on Win32Posix, whose
// surface spells no file constants (the game passes their values).
#include "Source/D2Headers/d2_platform.h"
#define GENERIC_WRITE 0x40000000u
#define CREATE_ALWAYS 2
#define FILE_ATTRIBUTE_NORMAL 0x80
#endif

namespace {

struct FileSink {
    HANDLE hFile;
    bool bFailed;
};

void WriteToFile(void* pContext, void* pData, int nSize) {
    FileSink* pSink = (FileSink*)pContext;
    DWORD nWritten = 0;
    if (pSink->bFailed || nSize <= 0) return;
    if (!WriteFile(pSink->hFile, pData, (DWORD)nSize, &nWritten, nullptr) || (int)nWritten != nSize) {
        pSink->bFailed = true;
    }
}

}  // namespace

extern "C" {

// IJL fills the structure with its defaults; what the game then sets is what
// ijlWrite reads, so the only state to establish is "initialised".
int32_t __stdcall ijlInit(void* pJpegCoreProps) {
    JPEG_CORE_PROPERTIES_HEAD* p = (JPEG_CORE_PROPERTIES_HEAD*)pJpegCoreProps;
    if (!p) return IJL_INVALID_JPEG_PROPERTIES;
    p->DIBChannels = 3;
    p->DIBColor = IJL_BGR;
    p->JPGChannels = 3;
    p->jquality = 75;
    return IJL_OK;
}

int32_t __stdcall ijlFree(void* pJpegCoreProps) {
    return pJpegCoreProps ? IJL_OK : IJL_INVALID_JPEG_PROPERTIES;
}

int32_t __stdcall ijlWrite(void* pJpegCoreProps, int32_t nWriteOption) {
    JPEG_CORE_PROPERTIES_HEAD* p = (JPEG_CORE_PROPERTIES_HEAD*)pJpegCoreProps;
    if (!p || nWriteOption != IJL_JFILE_WRITEWHOLEIMAGE) return IJL_INVALID_JPEG_PROPERTIES;
    if (!p->JPGFile || !*p->JPGFile) return IJL_INVALID_FILENAME;
    if (!p->DIBBytes || p->DIBWidth == 0 || p->DIBHeight == 0) return IJL_INVALID_JPEG_PROPERTIES;
    uint32_t nChannels = p->DIBChannels ? p->DIBChannels : 3;
    if (nChannels != 3 && nChannels != 1) return IJL_INVALID_JPEG_PROPERTIES;

    uint32_t nWidth = p->DIBWidth;
    bool bBottomUp = p->DIBHeight < 0;
    uint32_t nHeight = (uint32_t)(bBottomUp ? -p->DIBHeight : p->DIBHeight);
    size_t nStride = (size_t)nWidth * nChannels + p->DIBPadBytes;

    // stb takes a tightly packed top-down RGB image; rebuild one when the rows are
    // padded, bottom-up, or BGR.
    bool bSwap = nChannels == 3 && p->DIBColor == IJL_BGR;
    const uint8_t* pPixels = p->DIBBytes;
    uint8_t* pPacked = nullptr;
    if (p->DIBPadBytes || bBottomUp || bSwap) {
        pPacked = (uint8_t*)malloc((size_t)nWidth * nHeight * nChannels);
        if (!pPacked) return IJL_INVALID_JPEG_PROPERTIES;
        for (uint32_t y = 0; y < nHeight; ++y) {
            const uint8_t* pSrc = p->DIBBytes + (size_t)(bBottomUp ? nHeight - 1 - y : y) * nStride;
            uint8_t* pDst = pPacked + (size_t)y * nWidth * nChannels;
            if (bSwap) {
                for (uint32_t x = 0; x < nWidth; ++x) {
                    pDst[x * 3 + 0] = pSrc[x * 3 + 2];
                    pDst[x * 3 + 1] = pSrc[x * 3 + 1];
                    pDst[x * 3 + 2] = pSrc[x * 3 + 0];
                }
            } else {
                memcpy(pDst, pSrc, (size_t)nWidth * nChannels);
            }
        }
        pPixels = pPacked;
    }

    int nQuality = p->jquality;
    if (nQuality < 1) nQuality = 1;
    if (nQuality > 100) nQuality = 100;

    FileSink sink;
    sink.hFile = CreateFileA(p->JPGFile, GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
    sink.bFailed = false;
    if (sink.hFile == INVALID_HANDLE_VALUE) {
        free(pPacked);
        return IJL_ERR_FILECREATE;
    }
    int nOk = stbi_write_jpg_to_func(WriteToFile, &sink, (int)nWidth, (int)nHeight, (int)nChannels, pPixels, nQuality);
    CloseHandle(sink.hFile);
    free(pPacked);
    if (!nOk || sink.bFailed) {
        DeleteFileA(p->JPGFile);
        return IJL_ERR_FILECREATE;
    }
    p->JPGWidth = nWidth;
    p->JPGHeight = (int)nHeight;
    p->JPGChannels = nChannels;
    return IJL_OK;
}

}  // extern "C"
