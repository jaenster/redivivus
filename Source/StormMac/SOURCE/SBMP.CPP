// StormMac Storm Mac/SOURCE/SBMP.CPP: PCX, BMP and TGA image decoding, 1.14d Intel Mac
// 0000c0ae..0000d43d.
//
// The binary's __FILE__ is ".../StormMac/Storm Mac/SOURCE/SBMP.CPP" (SBmpLoadImage's
// allocations, lines 0x111 .. 0x466). The functions after SBmpLoadImage up to SCMD.CPP's
// first (the row copy, the TGA helpers, the PCX run-length decoder) are SBMP's: they are
// called from nothing else.
#include "../Win32Mac/Source/Win32/Win32Mac.h"

#if !defined(_WIN32) && D2_OS_MAC

#include "Source/StormMac/H/StormApi.h"

#include <string.h>


struct D2MPQFileStrc;

// The file-local routines are static: each has two callers and stays out of line, and LLVM
// gives an internal function the fast convention (first two arguments in ecx and edx).

// SFileSetFilePointer's move methods, as Win32's.
#ifndef FILE_CURRENT
#define FILE_CURRENT 1
#define FILE_END 2
#endif

// Storm's release-build parameter check: every condition is evaluated into one result, and a
// failure sets ERROR_INVALID_PARAMETER and returns.
#define SBMP_VALIDATE_BEGIN int nValidMask = -1
#define SBMP_VALIDATE(bCondition) if (!(bCondition)) nValidMask = 0
#define SBMP_VALIDATE_END                \
    if (nValidMask == 0) {               \
        SErrSetLastError(0x57);          \
        return 0;                        \
    }
#define SBMP_VALIDATE_END_VOID           \
    if (nValidMask == 0) {               \
        SErrSetLastError(0x57);          \
        return;                          \
    }
#define SBMP_FILE "/Users/bclemetson/dev/diablo2/Diablo2/Source/DiabloAll/../../../StormMac/Storm Mac/SOURCE/SBMP.CPP"

#pragma pack(push, 1)
struct PcxHeader {
    uint8_t byManufacturer;  // 0x00  10
    uint8_t byVersion;       // 0x01  5
    uint8_t byEncoding;      // 0x02
    uint8_t byBitsPerPixel;  // 0x03  per plane
    uint16_t wXMin;          // 0x04
    uint16_t wYMin;          // 0x06
    uint16_t wXMax;          // 0x08
    uint16_t wYMax;          // 0x0A
    uint8_t abyUnused[0x34]; // 0x0C
    uint8_t byReserved;      // 0x40
    uint8_t byPlanes;        // 0x41
    uint16_t wBytesPerLine;  // 0x42
    uint8_t abyFiller[0x3c]; // 0x44
};

struct BmpFileHeader {
    uint16_t wType;       // 'BM'
    uint32_t dwSize;
    uint32_t dwReserved;
    uint32_t dwOffBits;   // 0x0A
};

struct BmpInfoHeader {
    uint32_t dwSize;
    int32_t nWidth;       // 0x04
    int32_t nHeight;      // 0x08
    uint16_t wPlanes;
    uint16_t wBitCount;   // 0x0E
    uint32_t dwCompression;
    uint32_t dwSizeImage; // 0x14
    uint32_t adwRest[4];
};

struct TgaHeader {
    uint8_t byIdLength;       // 0x00
    uint8_t byColorMapType;   // 0x01
    uint8_t byImageType;      // 0x02
    uint16_t wColorMapFirst;  // 0x03
    uint16_t wColorMapLength; // 0x05
    uint8_t byColorMapDepth;  // 0x07
    uint16_t wXOrigin;        // 0x08
    uint16_t wYOrigin;        // 0x0A
    uint16_t wWidth;          // 0x0C
    uint16_t wHeight;         // 0x0E
    uint8_t byPixelDepth;     // 0x10
    union {
        uint8_t byDescriptor; // 0x11
        struct {
            uint8_t nAlphaBits : 4;
            uint8_t bRightToLeft : 1;
            uint8_t bTopToBottom : 1;
            uint8_t nInterleave : 2;
        } tDescriptor;
    };
};
#pragma pack(pop)

static void SBMP_CopyBottomUpRows(uint8_t* pDest, const uint8_t* pSrc, uint32_t nDestSize, int nDestWidth,
                                      int nSrcWidth, int nRows);
static int SBMP_IsValidTgaHeader(const TgaHeader* pHeader);
static int SBMP_LoadTgaPalette(const TgaHeader* pHeader, const uint8_t* pEntries, int nExpected, uint8_t* pPalette);
static int SBMP_DecodeTgaPixels(const TgaHeader* pHeader, const uint8_t* pSrc, int nSrcSize, uint8_t* pDest,
                                size_t nDestSize);
static void UncompressPcxImage(uint8_t* pDest, const uint8_t* pSrc, uint32_t nDestSize, int nSrcSize,
                               uint32_t nWidth, uint32_t nBytesPerLine, int nPlanes);

// The three decoders SBmpDecodeImage dispatches to; each is inlined into it.
static int DecodeBmpMem(const uint8_t* pData, uint8_t* pPalette, uint8_t* pbPixels, uint32_t nPixelBufSize,
                        uint32_t* pnWidth, uint32_t* pnHeight, uint32_t* pnBpp) {
    const BmpFileHeader* pFile = (const BmpFileHeader*)pData;
    const BmpInfoHeader* pInfo = (const BmpInfoHeader*)(pData + 0x0e);
    if (pnWidth != NULL) {
        *pnWidth = pInfo->nWidth;
    }
    if (pnHeight != NULL) {
        *pnHeight = pInfo->nHeight;
    }
    if (pnBpp != NULL) {
        *pnBpp = pInfo->wBitCount;
    }
    if (pInfo->wBitCount > 8) {
        pPalette = NULL;
    }
    if (pPalette != NULL) {
        // the BGRX palette follows the headers, 0x36 bytes in
        const uint8_t* pQuads = pData + 0x36;
        for (int i = 0; i < 0x100; i++) {
            pPalette[i * 4] = pQuads[i * 4 + 2];
            pPalette[i * 4 + 1] = pQuads[i * 4 + 1];
            pPalette[i * 4 + 2] = pQuads[i * 4];
            pPalette[i * 4 + 3] = 0;
        }
    }
    if (pbPixels != NULL) {
        uint32_t nOffset = pFile->dwOffBits - 0x0e - 0x28 - (pPalette != NULL ? 0x400 : 0);
        int nDestWidth = (pInfo->nWidth * pInfo->wBitCount) >> 3;
        int nSrcWidth = nDestWidth;
        if (nSrcWidth & 3) {
            nSrcWidth += 4 - (nSrcWidth & 3);
        }
        SBMP_CopyBottomUpRows(pbPixels, pData + nOffset, nPixelBufSize, nDestWidth, nSrcWidth, pInfo->nHeight);
    }
    return 1;
}

static int DecodePcxMem(const uint8_t* pData, uint32_t nDataSize, uint8_t* pPalette, uint8_t* pbPixels,
                        uint32_t nPixelBufSize, uint32_t* pnWidth, uint32_t* pnHeight, uint32_t* pnBpp) {
    const PcxHeader* pHeader = (const PcxHeader*)pData;
    int nWidth = pHeader->wXMax + 1 - pHeader->wXMin;
    int nHeight = pHeader->wYMax + 1 - pHeader->wYMin;
    int nBpp = pHeader->byBitsPerPixel * pHeader->byPlanes;
    if (pnWidth != NULL) {
        *pnWidth = nWidth;
    }
    if (pnHeight != NULL) {
        *pnHeight = nHeight;
    }
    if (pnBpp != NULL) {
        *pnBpp = nBpp;
    }
    if (pbPixels != NULL) {
        uint32_t nPixelBytes = ((uint32_t)(nBpp + 7) >> 3) * (nWidth * nHeight);
        // an 8-bit image ends in a marker byte and the 768-byte palette
        UncompressPcxImage(pbPixels, pData + sizeof(PcxHeader), nPixelBufSize < nPixelBytes ? nPixelBufSize : nPixelBytes,
                           nDataSize - sizeof(PcxHeader) - (nBpp == 8 ? 0x301 : 0), nWidth, pHeader->wBytesPerLine,
                           pHeader->byPlanes);
    }
    if (pPalette != NULL && nBpp == 8) {
        const uint8_t* pRgb = pData + nDataSize - 0x300;
        for (int i = 0; i < 0x100; i++) {
            pPalette[i * 4] = pRgb[i * 3];
            pPalette[i * 4 + 1] = pRgb[i * 3 + 1];
            pPalette[i * 4 + 2] = pRgb[i * 3 + 2];
            pPalette[i * 4 + 3] = 0;
        }
    }
    return 1;
}

static int DecodeTgaMem(const uint8_t* pData, uint32_t nDataSize, uint8_t* pPalette, uint8_t* pbPixels,
                        uint32_t nPixelBufSize, uint32_t* pnWidth, uint32_t* pnHeight, uint32_t* pnBpp) {
    const TgaHeader* pHeader = (const TgaHeader*)pData;
    if (nDataSize < sizeof(TgaHeader) || !SBMP_IsValidTgaHeader(pHeader)) {
        return 0;
    }
    int nBpp = pHeader->byPixelDepth;
    int nHeight = pHeader->wHeight;
    int nWidth = pHeader->wWidth;
    int nPaletteBytes = 0;
    if (pHeader->byColorMapType != 0) {
        nPaletteBytes = ((uint32_t)(pHeader->byColorMapDepth + 7) >> 3) * pHeader->wColorMapLength;
    }
    // run-length images are measured as they decode
    int nPixelBytes = 1;
    if (pHeader->byImageType <= 8) {
        nPixelBytes = ((uint32_t)(nBpp + 7) >> 3) * (nHeight * nWidth);
    }
    if (pHeader->byIdLength + sizeof(TgaHeader) + nPaletteBytes + nPixelBytes > nDataSize) {
        return 0;
    }
    if (pnWidth != NULL) {
        *pnWidth = nWidth;
    }
    if (pnHeight != NULL) {
        *pnHeight = nHeight;
    }
    if (pnBpp != NULL) {
        *pnBpp = nBpp;
    }
    if (pPalette != NULL && pHeader->byColorMapType != 0 &&
        !SBMP_LoadTgaPalette(pHeader, pData + sizeof(TgaHeader) + pHeader->byIdLength, nPaletteBytes, pPalette)) {
        return 0;
    }
    if (pbPixels != NULL) {
        uint32_t nOffset = pHeader->byIdLength + sizeof(TgaHeader) + nPaletteBytes;
        if (!SBMP_DecodeTgaPixels(pHeader, pData + nOffset, nDataSize - nOffset, pbPixels, nPixelBufSize)) {
            return 0;
        }
    }
    return 1;
}

// 1.14d mac: 0000c0ae
// Format 0: by the magic bytes; 1: BMP; 2: PCX; 3: TGA. The palette is 256 four-byte
// entries.
int Storm::Source::SBMP::SBmpDecodeImage(int nFormat, const void* pData, uint32_t nDataSize, void* pPaletteArg, uint8_t* pbPixels,
                    int nPixelBufSize, uint32_t* pnWidth, uint32_t* pnHeight, uint32_t* pnBpp) {
    uint8_t* pPalette = (uint8_t*)pPaletteArg;
    if (pnWidth != NULL) {
        *pnWidth = 0;
    }
    if (pnHeight != NULL) {
        *pnHeight = 0;
    }
    if (pnBpp != NULL) {
        *pnBpp = 0;
    }
    SBMP_VALIDATE_BEGIN;
    SBMP_VALIDATE(pData != NULL);
    SBMP_VALIDATE(nDataSize != 0);
    SBMP_VALIDATE(nPixelBufSize != 0 || pbPixels == NULL);
    SBMP_VALIDATE_END;
    if (nFormat == 0 && nDataSize >= sizeof(uint16_t)) {
        // by the signature; anything else is taken for a TGA, which has none
        switch (*(const uint16_t*)pData) {
        case 0x4d42:
            nFormat = 1;
            break;
        case 0x050a:
            nFormat = 2;
            break;
        default:
            nFormat = 3;
            break;
        }
    }
    switch (nFormat) {
    case 1:
        return DecodeBmpMem((const uint8_t*)pData, pPalette, pbPixels, nPixelBufSize, pnWidth, pnHeight, pnBpp);
    case 2:
        return DecodePcxMem((const uint8_t*)pData, nDataSize, pPalette, pbPixels, nPixelBufSize, pnWidth, pnHeight,
                            pnBpp);
    case 3:
        return DecodeTgaMem((const uint8_t*)pData, nDataSize, pPalette, pbPixels, nPixelBufSize, pnWidth, pnHeight,
                            pnBpp);
    default:
        return 0;
    }
}

// 1.14d mac: 0000c421
// The same decoders over a file opened through SFile, the format picked from its first two
// bytes.
int SBmpLoadImage(const char* szFile, uint8_t* pPalette, uint8_t* pbPixels, uint32_t nPixelBufSize, uint32_t* pnWidth,
                  uint32_t* pnHeight, uint32_t* pnBpp) {
    if (pnWidth != NULL) {
        *pnWidth = 0;
    }
    if (pnHeight != NULL) {
        *pnHeight = 0;
    }
    if (pnBpp != NULL) {
        *pnBpp = 0;
    }
    SBMP_VALIDATE_BEGIN;
    SBMP_VALIDATE(szFile != NULL);
    SBMP_VALIDATE(*szFile != '\0');
    SBMP_VALIDATE(nPixelBufSize != 0 || pbPixels == NULL);
    SBMP_VALIDATE_END;
    D2MPQFileStrc* pFile;
    if (!SFileOpenFile(szFile, &pFile)) {
        return 0;
    }
    int nFormat = 0;
    uint16_t wSignature;
    if (SFileReadFile(pFile, &wSignature, sizeof(wSignature), NULL, NULL)) {
        SFileSetFilePointer(pFile, -2, NULL, FILE_CURRENT);
        // anything but the two signatures is taken for a TGA, which has none
        switch (wSignature) {
        case 0x4d42:
            nFormat = 1;
            break;
        case 0x050a:
            nFormat = 2;
            break;
        default:
            nFormat = 3;
            break;
        }
    }
    int bResult = 0;
    switch (nFormat) {
    case 1: {
        BmpFileHeader tFileHeader;
        if (!SFileReadFile(pFile, &tFileHeader, sizeof(BmpFileHeader), NULL, NULL)) {
            break;
        }
        BmpInfoHeader tInfoHeader;
        if (!SFileReadFile(pFile, &tInfoHeader, sizeof(BmpInfoHeader), NULL, NULL)) {
            break;
        }
        if (pnWidth != NULL) {
            *pnWidth = tInfoHeader.nWidth;
        }
        if (pnHeight != NULL) {
            *pnHeight = tInfoHeader.nHeight;
        }
        if (pnBpp != NULL) {
            *pnBpp = tInfoHeader.wBitCount;
        }
        if (tInfoHeader.wBitCount > 8) {
            pPalette = NULL;
        }
        if (pPalette != NULL) {
            // BGRX quads into RGBX entries
            uint8_t abyQuads[0x400];
            if (!SFileReadFile(pFile, abyQuads, sizeof(abyQuads), NULL, NULL)) {
                break;
            }
            for (int i = 0; i < 0x100; i++) {
                pPalette[i * 4] = abyQuads[i * 4 + 2];
                pPalette[i * 4 + 1] = abyQuads[i * 4 + 1];
                pPalette[i * 4 + 2] = abyQuads[i * 4];
                pPalette[i * 4 + 3] = 0;
            }
        }
        if (pbPixels != NULL) {
            SFileSetFilePointer(pFile,
                                tFileHeader.dwOffBits - sizeof(BmpFileHeader) - 0x28 - (pPalette != NULL ? 0x400 : 0),
                                NULL, FILE_CURRENT);
            uint32_t nRead = 0;
            uint8_t* pBuffer = (uint8_t*)SMemAlloc(tInfoHeader.dwSizeImage, SBMP_FILE, 0x1fa, 0);
            SFileReadFile(pFile, pBuffer, tInfoHeader.dwSizeImage, &nRead, NULL);
            int nDestWidth = (tInfoHeader.nWidth * tInfoHeader.wBitCount) >> 3;
            int nSrcWidth = nDestWidth;
            if (nSrcWidth & 3) {
                nSrcWidth += 4 - (nSrcWidth & 3);
            }
            SBMP_CopyBottomUpRows(pbPixels, pBuffer, nPixelBufSize, nDestWidth, nSrcWidth, tInfoHeader.nHeight);
            SMemFree(pBuffer, SBMP_FILE, 0x206, 0);
            if (nRead < tInfoHeader.dwSizeImage) {
                break;
            }
        }
        bResult = 1;
        break;
    }
    case 2: {
        PcxHeader tHeader;
        if (!SFileReadFile(pFile, &tHeader, sizeof(PcxHeader), NULL, NULL)) {
            break;
        }
        int nWidth = tHeader.wXMax + 1 - tHeader.wXMin;
        if (pnWidth != NULL) {
            *pnWidth = nWidth;
        }
        if (pnHeight != NULL) {
            *pnHeight = tHeader.wYMax + 1 - tHeader.wYMin;
        }
        int nBpp = tHeader.byPlanes * tHeader.byBitsPerPixel;
        if (pnBpp != NULL) {
            *pnBpp = nBpp;
        }
        bool bPalette = pPalette != NULL && nBpp == 8;
        if (pbPixels != NULL) {
            // an 8-bit image ends in a marker byte and the 768-byte palette
            uint32_t nBytes = SFileGetFileSize(pFile, NULL) - SFileSetFilePointer(pFile, 0, NULL, FILE_CURRENT) -
                              (nBpp == 8 ? 0x300 : 0);
            uint8_t* pBuffer = (uint8_t*)SMemAlloc(nBytes, SBMP_FILE, 0x466, 0);
            SFileReadFile(pFile, pBuffer, nBytes, NULL, NULL);
            UncompressPcxImage(pbPixels, pBuffer, nPixelBufSize, nBytes - (nBpp == 8), nWidth, tHeader.wBytesPerLine,
                               tHeader.byPlanes);
            SMemFree(pBuffer, SBMP_FILE, 0x46f, 0);
        } else if (bPalette) {
            SFileSetFilePointer(pFile, -0x300, NULL, FILE_END);
        }
        if (bPalette) {
            uint8_t abyRgb[0x300];
            SFileReadFile(pFile, abyRgb, sizeof(abyRgb), NULL, NULL);
            for (int i = 0; i < 0x100; i++) {
                pPalette[i * 4] = abyRgb[i * 3];
                pPalette[i * 4 + 1] = abyRgb[i * 3 + 1];
                pPalette[i * 4 + 2] = abyRgb[i * 3 + 2];
                pPalette[i * 4 + 3] = 0;
            }
        }
        bResult = 1;
        break;
    }
    case 3: {
        TgaHeader tHeader;
        uint32_t nRead;
        if (!SFileReadFile(pFile, &tHeader, sizeof(TgaHeader), &nRead, NULL) || nRead != sizeof(TgaHeader) ||
            !SBMP_IsValidTgaHeader(&tHeader)) {
            break;
        }
        int nBpp = tHeader.byPixelDepth;
        int nHeight = tHeader.wHeight;
        int nWidth = tHeader.wWidth;
        int nPaletteBytes = 0;
        if (tHeader.byColorMapType != 0) {
            nPaletteBytes = ((uint32_t)(tHeader.byColorMapDepth + 7) >> 3) * tHeader.wColorMapLength;
        }
        // run-length images are measured as they decode
        int nPixelBytes = 1;
        if (tHeader.byImageType <= 8) {
            nPixelBytes = ((uint32_t)(nBpp + 7) >> 3) * (nHeight * nWidth);
        }
        if (tHeader.byIdLength + sizeof(TgaHeader) + nPaletteBytes + nPixelBytes > SFileGetFileSize(pFile, NULL)) {
            break;
        }
        if (pnWidth != NULL) {
            *pnWidth = nWidth;
        }
        if (pnHeight != NULL) {
            *pnHeight = nHeight;
        }
        if (pnBpp != NULL) {
            *pnBpp = nBpp;
        }
        SFileSetFilePointer(pFile, tHeader.byIdLength, NULL, FILE_CURRENT);
        if (pPalette != NULL && tHeader.byColorMapType != 0) {
            uint8_t abyEntries[0x400];
            SFileReadFile(pFile, abyEntries, nPaletteBytes, NULL, NULL);
            if (!SBMP_LoadTgaPalette(&tHeader, abyEntries, nPaletteBytes, pPalette)) {
                break;
            }
        }
        if (pbPixels != NULL) {
            uint32_t nBytes = SFileGetFileSize(pFile, NULL) - sizeof(TgaHeader) - nPaletteBytes - tHeader.byIdLength;
            uint8_t* pBuffer = (uint8_t*)SMemAlloc(nBytes, SBMP_FILE, 0x111, 0);
            SFileReadFile(pFile, pBuffer, nBytes, NULL, NULL);
            if (!SBMP_DecodeTgaPixels(&tHeader, pBuffer, nBytes, pbPixels, nPixelBufSize)) {
                SMemFree(pBuffer, SBMP_FILE, 0x118, 0);
                break;
            }
            SMemFree(pBuffer, SBMP_FILE, 0x11b, 0);
        }
        bResult = 1;
        break;
    }
    }
    SFileCloseFile(pFile);
    return bResult;
}

// 1.14d mac: 0000ccb4
// Rows from the last source row up, packed into the destination until it is full.
static void SBMP_CopyBottomUpRows(uint8_t* pDest, const uint8_t* pSrc, uint32_t nDestSize, int nDestWidth,
                                      int nSrcWidth, int nRows) {
    SBMP_VALIDATE_BEGIN;
    SBMP_VALIDATE(pDest != NULL);
    SBMP_VALIDATE(pSrc != NULL);
    SBMP_VALIDATE(nDestSize != 0);
    SBMP_VALIDATE(nDestWidth != 0);
    SBMP_VALIDATE(nSrcWidth != 0);
    SBMP_VALIDATE(nRows != 0);
    SBMP_VALIDATE_END_VOID;
    pSrc += (nRows - 1) * nSrcWidth;
    uint8_t* pDestEnd = pDest + nDestSize;
    while (nRows--) {
        int nCopy = pDestEnd - pDest;
        if (nCopy > nDestWidth) {
            nCopy = nDestWidth;
        }
        if (nCopy == 0) {
            break;
        }
        if (nCopy > 0) {
            memmove(pDest, pSrc, nCopy);
        }
        pDest += nCopy;
        pSrc -= nSrcWidth;
    }
}

// 1.14d mac: 0000cd40
// Image types 1, 2, 9 and 10; colour-mapped 8-bit with a 24 or 32 bit map, or true colour
// 24 or 32 bit with no map.
static int SBMP_IsValidTgaHeader(const TgaHeader* pHeader) {
    bool bValid = false;
    switch (pHeader->byImageType) {
    case 1:
    case 2:
    case 9:
    case 10:
        if ((pHeader->byColorMapType == 0 && ((pHeader->byPixelDepth == 0x18 && (pHeader->byDescriptor & 0xf) == 0) ||
                                              (pHeader->byPixelDepth == 0x20 && (pHeader->byDescriptor & 0xf) == 8))) ||
            (pHeader->byColorMapType == 1 && pHeader->byPixelDepth == 8 &&
             (int)pHeader->wColorMapFirst + (int)pHeader->wColorMapLength <= 0x100 &&
             (pHeader->byColorMapDepth == 0x18 || pHeader->byColorMapDepth == 0x20))) {
            // no interleaving
            bValid = pHeader->byDescriptor < 0x40;
        }
        break;
    }
    return bValid;
}

// 1.14d mac: 0000cdbe
// The colour map into a 256-entry palette, the rest of it zeroed.
static int SBMP_LoadTgaPalette(const TgaHeader* pHeader, const uint8_t* pEntries, int nExpected, uint8_t* pPalette) {
    uint16_t nLength = pHeader->wColorMapLength;
    uint32_t nDepth = pHeader->byColorMapDepth + 7;
    int nBytesPerEntry = nDepth / 8;
    if (nLength * nBytesPerEntry != nExpected) {
        return 0;
    }
    uint16_t nFirst = pHeader->wColorMapFirst;
    int nIndex = nFirst;
    if (nFirst != 0) {
        bzero(pPalette, nIndex * 4);
    }
    if (nLength != 0) {
        int nEnd = nIndex + nLength;
        const uint8_t* pSrc = pEntries + 3;
        do {
            pPalette[nIndex * 4] = pSrc[-1];
            pPalette[nIndex * 4 + 1] = pSrc[-2];
            pPalette[nIndex * 4 + 2] = pSrc[-3];
            uint8_t byAlpha = 0;
            if (nDepth >= 0x20) {
                byAlpha = *pSrc;
            }
            pPalette[nIndex * 4 + 3] = byAlpha;
            pSrc += nBytesPerEntry;
            nIndex++;
        } while (nIndex < nEnd);
    }
    int nRest = (0x100 - nIndex) * 4;
    if (nRest > 0) {
        bzero(pPalette + nIndex * 4, nRest);
    }
    return 1;
}

// 1.14d mac: 0000ce80
// Raw (types 1, 2) or run-length (9, 10) pixels, placed by the descriptor's origin bits.
static int SBMP_DecodeTgaPixels(const TgaHeader* pHeader, const uint8_t* pSrc, int nSrcSize, uint8_t* pDest,
                                size_t nDestSize) {
    int nBytesPerPixel = (uint32_t)(pHeader->byPixelDepth + 7) >> 3;
    uint8_t byDescriptor = pHeader->byDescriptor;
    bool bRightToLeft = pHeader->tDescriptor.bRightToLeft;
    bool bTopToBottom = pHeader->tDescriptor.bTopToBottom;
    int nPixelStep = bRightToLeft ? -nBytesPerPixel : nBytesPerPixel;
    int nWidth = pHeader->wWidth;
    int nHeight = pHeader->wHeight;
    int nImageSize = nWidth * nHeight * nBytesPerPixel;
    if (nImageSize > (int)nDestSize) {
        nImageSize = nDestSize;
    }
    int nRowAdjust;
    if (bTopToBottom) {
        nRowAdjust = (nBytesPerPixel - nPixelStep) * nWidth;
    } else {
        nRowAdjust = -nWidth * (nPixelStep + nBytesPerPixel);
    }
    if (bRightToLeft) {
        pDest += (nWidth - 1) * nBytesPerPixel;
    }
    if (!bTopToBottom) {
        pDest += (nHeight - 1) * nWidth * nBytesPerPixel;
    }
    // one pixel of the source at pDest, then step along the row (and to the next row at its end)
#define SBMP_PUT_PIXEL(pPixel)                                    \
    do {                                                          \
        switch (nBytesPerPixel) {                                 \
        case 3:                                                   \
            pDest[2] = (pPixel)[2];                               \
        case 2:                                                   \
            pDest[1] = (pPixel)[1];                               \
        case 1:                                                   \
            pDest[0] = (pPixel)[0];                               \
            break;                                                \
        case 4:                                                   \
            *(uint32_t*)pDest = *(const uint32_t*)(pPixel);       \
        }                                                         \
        if (--nColumn == 0) {                                     \
            nColumn = nWidth;                                     \
            pDest += nRowAdjust + nPixelStep;                     \
        } else {                                                  \
            pDest += nPixelStep;                                  \
        }                                                         \
    } while (0)
    int nColumn = nWidth;
    if (pHeader->byImageType == 9 || pHeader->byImageType == 10) {
        int nPixelsLeft = nImageSize / nBytesPerPixel;
        if (nPixelsLeft != 0 && nSrcSize > 0) {
            const uint8_t* pEnd = pSrc + nSrcSize;
            do {
                uint8_t byPacket = *pSrc;
                const uint8_t* pData = pSrc + 1;
                int nCount;
                if (!(byPacket & 0x80)) {
                    // raw packet
                    nCount = byPacket + 1;
                    if (nCount > nPixelsLeft) {
                        nCount = nPixelsLeft;
                    }
                    if (pSrc + nCount * nBytesPerPixel + 1 > pEnd) {
                        return 0;
                    }
                    for (int i = 0; i < nCount; i++) {
                        SBMP_PUT_PIXEL(pData);
                        pData += nBytesPerPixel;
                    }
                    pSrc += nCount * nBytesPerPixel + 1;
                } else {
                    // run packet
                    nCount = (byPacket & 0x7f) + 1;
                    if (nCount > nPixelsLeft) {
                        nCount = nPixelsLeft;
                    }
                    if (pSrc + nBytesPerPixel + 1 > pEnd) {
                        return 0;
                    }
                    for (int i = 0; i < nCount; i++) {
                        SBMP_PUT_PIXEL(pData);
                    }
                    pSrc += nBytesPerPixel + 1;
                }
                nPixelsLeft -= nCount;
            } while (nPixelsLeft != 0 && pSrc < pEnd);
        }
    } else if (pHeader->byImageType == 1 || pHeader->byImageType == 2) {
        if (nImageSize > nSrcSize) {
            return 0;
        }
        if ((byDescriptor & 0x30) == 0x20) {
            if (nImageSize > 0) {
                memmove(pDest, pSrc, nImageSize);
            }
        } else {
            for (int nPixels = nImageSize / nBytesPerPixel; nPixels != 0; nPixels--) {
                SBMP_PUT_PIXEL(pSrc);
                pSrc += nBytesPerPixel;
            }
        }
    } else {
        return 0;
    }
#undef SBMP_PUT_PIXEL
    return 1;
}

// 1.14d mac: 0000d1f8
// PCX run-length data into pixels. Each scanline holds nPlanes planes of nBytesPerLine
// bytes, the planes written interleaved (the last plane first); stops at either end.
static void UncompressPcxImage(uint8_t* pDest, const uint8_t* pSrc, uint32_t nDestSize, int nSrcSize,
                               uint32_t nWidth, uint32_t nBytesPerLine, int nPlanes) {
    int nRowStride = nPlanes * nWidth;
    uint8_t* pDestEnd = pDest + nDestSize;
    const uint8_t* pSrcEnd = pSrc + nSrcSize;
    int nPlane = nPlanes - 1;
    uint32_t nColumn = 0;
    while (pDest + nColumn < pDestEnd && pSrc < pSrcEnd) {
        uint8_t byValue = *pSrc++;
        if (byValue >= 0xc0) {
            uint32_t nRun = byValue & 0x3f;
            if (nRun >= (uint32_t)(pDestEnd - pDest)) {
                nRun = pDestEnd - pDest;
            }
            byValue = *pSrc++;
            do {
                if (nWidth <= nColumn) {
                    // padding past the visible width
                    uint32_t nSkip = nBytesPerLine - nColumn;
                    if (nRun < nSkip) {
                        nSkip = nRun;
                    }
                    nColumn += nSkip;
                    if (nBytesPerLine <= nColumn) {
                        nColumn -= nBytesPerLine;
                        if (--nPlane < 0) {
                            pDest += nRowStride;
                            nPlane = nPlanes - 1;
                        }
                    }
                    nRun -= nSkip;
                    if (nRun == 0) {
                        break;
                    }
                }
                uint32_t nFill = nWidth - nColumn;
                if (nRun < nFill) {
                    nFill = nRun;
                }
                if (nPlanes == 3) {
                    int nBytes = pDestEnd - pDest;
                    if ((int)(nFill * 3) < nBytes) {
                        nBytes = nFill * 3;
                    }
                    if (0 < nBytes) {
                        int nPos = nColumn * 3 + nPlane;
                        int nStop = nBytes + nPos;
                        do {
                            pDest[nPos] = byValue;
                            nPos += 3;
                        } while (nPos < nStop);
                    }
                } else if (nPlanes == 1) {
                    SMemFill(pDest + nColumn, nFill, byValue);
                } else {
                    int nBytes = pDestEnd - pDest;
                    if ((int)(nFill * nPlanes) < nBytes) {
                        nBytes = nFill * nPlanes;
                    }
                    if (0 < nBytes) {
                        int nPos = nColumn * nPlanes + nPlane;
                        int nStop = nBytes + nPos;
                        do {
                            pDest[nPos] = byValue;
                            nPos += nPlanes;
                        } while (nPos < nStop);
                    }
                }
                nColumn += nFill;
                if (nBytesPerLine <= nColumn) {
                    nColumn -= nBytesPerLine;
                    if (--nPlane < 0) {
                        pDest += nRowStride;
                        nPlane = nPlanes - 1;
                    }
                }
                nRun -= nFill;
            } while (nRun != 0);
        } else {
            // a single pixel
            if (nColumn < nWidth) {
                if (nPlanes == 3) {
                    uint8_t* pPixel = pDest + nColumn * 3 + nPlane;
                    if (pPixel < pDestEnd) {
                        *pPixel = byValue;
                    }
                } else if (nPlanes == 1) {
                    pDest[nColumn] = byValue;
                } else {
                    uint8_t* pPixel = pDest + nColumn * nPlanes + nPlane;
                    if (pPixel < pDestEnd) {
                        *pPixel = byValue;
                    }
                }
            }
            nColumn++;
            if (nBytesPerLine <= nColumn) {
                nColumn -= nBytesPerLine;
                if (--nPlane < 0) {
                    pDest += nRowStride;
                    nPlane = nPlanes - 1;
                }
            }
        }
    }
}

#endif  // D2_OS_MAC
