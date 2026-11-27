// StormMac Storm Mac/SOURCE/SBMP.CPP: PCX, BMP and TGA image decoding, 1.14d Intel Mac
// 0000c0ae..0000d43d.
//
// The binary's __FILE__ is ".../StormMac/Storm Mac/SOURCE/SBMP.CPP" (SBmpLoadImage's
// allocations, lines 0x111 .. 0x466). The functions after SBmpLoadImage up to SCMD.CPP's
// first (the row copy, the TGA helpers, the PCX run-length decoder) are SBMP's: they are
// called from nothing else.
#include "../Win32Mac/Source/Win32/Win32Mac.h"

#if !defined(_WIN32) && D2_OS_MAC

#include <string.h>

void* SMemAlloc(uint32_t nBytes, const char* szFile, int nLine, uint32_t dwFlags);
int SMemFree(void* pMem, const char* szFile, int nLine, uint32_t dwFlags);
void SMemFill(void* pDest, size_t nSize, uint8_t byValue);
void SErrSetLastError(DWORD dwErrCode);

struct D2MPQFileStrc;
BOOL SFileOpenFile(const char* szFile, D2MPQFileStrc** ppFile);
BOOL SFileReadFile(D2MPQFileStrc* pFile, void* pBuffer, size_t nToRead, size_t* pnRead, void* pOverlapped);
DWORD SFileSetFilePointer(D2MPQFileStrc* pFile, int nDistance, int* pnDistanceHigh, DWORD dwMoveMethod);
DWORD SFileGetFileSize(D2MPQFileStrc* pFile, LPDWORD pdwFileSizeHigh);
BOOL SFileCloseFile(D2MPQFileStrc* pFile);

// File-local routines are not inlined and take their first two arguments in ecx and edx.
#define SBMP_LOCAL static __attribute__((noinline))
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
    uint8_t byDescriptor;     // 0x11
};
#pragma pack(pop)

SBMP_LOCAL void SBMP_CopyBottomUpRows(const void* pSrc, void* pDest, size_t nDestSize, size_t nRowBytes, int nSrcPitch,
                                  int nRows);
SBMP_LOCAL bool SBMP_IsValidTgaHeader(const TgaHeader* pHeader);
SBMP_LOCAL int SBMP_LoadTgaPalette(const TgaHeader* pHeader, const uint8_t* pEntries, int nExpected, uint8_t* pPalette);
SBMP_LOCAL int SBMP_DecodeTgaPixels(const TgaHeader* pHeader, const uint8_t* pSrc, int nSrcSize, uint8_t* pDest,
                                size_t nDestSize);
SBMP_LOCAL void UncompressPcxImage(const uint8_t* pSrc, uint8_t* pDest, uint32_t nDestSize, int nSrcSize,
                               uint32_t nWidth, uint32_t nBytesPerLine, int nPlanes);

// 1.14d mac: 0000c0ae
// Format 0: by the magic bytes; 1: BMP; 2: PCX; 3: TGA. The palette is 256 four-byte
// entries.
int SBmpDecodeImage(int nFormat, const void* pData, uint32_t nDataSize, uint8_t* pPalette, uint8_t* pbPixels,
                    int nPixelBufSize, uint32_t* pnWidth, uint32_t* pnHeight, uint32_t* pnBpp) {
    const uint8_t* pBytes = (const uint8_t*)pData;
    if (pnWidth != NULL) {
        *pnWidth = 0;
    }
    if (pnHeight != NULL) {
        *pnHeight = 0;
    }
    if (pnBpp != NULL) {
        *pnBpp = 0;
    }
    if (nDataSize == 0 || pData == NULL || (nPixelBufSize == 0 && pbPixels != NULL)) {
        SErrSetLastError(0x57);
        return 0;
    }
    bool bPcx = false;
    bool bBmp = false;
    if (nFormat == 0 && 1 < nDataSize) {
        if (*(const int16_t*)pData == 0x50a) {
            bPcx = true;
        } else if (*(const int16_t*)pData == 0x4d42) {
            bBmp = true;
        }
    } else if (nFormat != 3) {
        if (nFormat == 2) {
            bPcx = true;
        } else if (nFormat == 1) {
            bBmp = true;
        } else {
            return 0;
        }
    }
    if (bPcx) {
        const PcxHeader* pHeader = (const PcxHeader*)pData;
        uint32_t nWidth = (pHeader->wXMax + 1) - (uint32_t)pHeader->wXMin;
        if (pnWidth != NULL) {
            *pnWidth = nWidth;
        }
        uint32_t nHeight = (pHeader->wYMax + 1) - (uint32_t)pHeader->wYMin;
        if (pnHeight != NULL) {
            *pnHeight = nHeight;
        }
        uint32_t nBpp = (uint32_t)pHeader->byPlanes * (uint32_t)pHeader->byBitsPerPixel;
        if (pnBpp != NULL) {
            *pnBpp = nBpp;
        }
        if (pbPixels != NULL) {
            int nTrailer = 0;
            if (nBpp == 8) {
                nTrailer = -0x301;
            }
            uint32_t nPixelBytes = ((nBpp + 7) >> 3) * nHeight * nWidth;
            if ((uint32_t)nPixelBufSize <= nPixelBytes && nPixelBytes - nPixelBufSize != 0) {
                nPixelBytes = nPixelBufSize;
            }
            UncompressPcxImage(pBytes + 0x80, pbPixels, nPixelBytes, (nDataSize - 0x80) + nTrailer, nWidth,
                               pHeader->wBytesPerLine, pHeader->byPlanes);
        }
        if (pPalette == NULL || nBpp != 8) {
            return 1;
        }
        // the 768 palette bytes at the end of the file
        const uint8_t* pEntry = pBytes + nDataSize - 0x300;
        for (int i = 0; i != 0x100; i++) {
            pPalette[i * 4] = pEntry[0];
            pPalette[i * 4 + 1] = pEntry[1];
            pPalette[i * 4 + 2] = pEntry[2];
            pPalette[i * 4 + 3] = 0;
            pEntry += 3;
        }
        return 1;
    }
    if (bBmp) {
        const BmpFileHeader* pFile = (const BmpFileHeader*)pData;
        const BmpInfoHeader* pInfo = (const BmpInfoHeader*)(pBytes + 0x0e);
        if (pnWidth != NULL) {
            *pnWidth = pInfo->nWidth;
        }
        if (pnHeight != NULL) {
            *pnHeight = pInfo->nHeight;
        }
        uint16_t wBitCount = pInfo->wBitCount;
        if (pnBpp != NULL) {
            *pnBpp = wBitCount;
        }
        if (8 < wBitCount) {
            pPalette = NULL;
        }
        if (pPalette != NULL) {
            // the BGRX palette follows the headers, 0x36 bytes in
            for (int i = 0; i != 0x100; i++) {
                pPalette[i * 4] = pBytes[i * 4 + 0x38];
                pPalette[i * 4 + 1] = pBytes[i * 4 + 0x37];
                pPalette[i * 4 + 2] = pBytes[i * 4 + 0x36];
                pPalette[i * 4 + 3] = 0;
            }
        }
        if (pbPixels == NULL) {
            return 1;
        }
        int nHeaderBytes = -0x36;
        if (pPalette != NULL) {
            nHeaderBytes = -0x436;
        }
        uint32_t nRowBytes = (int)((uint32_t)pInfo->wBitCount * pInfo->nWidth) >> 3;
        uint32_t nPitch = nRowBytes;
        if ((nRowBytes & 3) != 0) {
            nPitch = (nRowBytes + 4) - (nRowBytes & 3);
        }
        SBMP_CopyBottomUpRows(pBytes + nHeaderBytes + pFile->dwOffBits, pbPixels, nPixelBufSize, nRowBytes, nPitch,
                              pInfo->nHeight);
        return 1;
    }
    // TGA
    int bResult = 0;
    if (0x11 < nDataSize && SBMP_IsValidTgaHeader((const TgaHeader*)pData)) {
        const TgaHeader* pHeader = (const TgaHeader*)pData;
        uint8_t byDepth = pHeader->byPixelDepth;
        uint16_t wHeight = pHeader->wHeight;
        int nPaletteBytes = 0;
        if (pHeader->byColorMapType != 0) {
            nPaletteBytes = ((pHeader->byColorMapDepth + 7) >> 3) * (uint32_t)pHeader->wColorMapLength;
        }
        int nPixelBytes = 1;
        if (pHeader->byImageType < 9) {
            nPixelBytes = ((byDepth + 7) >> 3) * (uint32_t)wHeight * (uint32_t)pHeader->wWidth;
        }
        bResult = 0;
        if ((uint32_t)pHeader->byIdLength + nPixelBytes + 0x12 + nPaletteBytes <= nDataSize) {
            if (pnWidth != NULL) {
                *pnWidth = pHeader->wWidth;
            }
            if (pnHeight != NULL) {
                *pnHeight = wHeight;
            }
            if (pnBpp != NULL) {
                *pnBpp = byDepth;
            }
            if ((pPalette == NULL || pHeader->byColorMapType == 0 ||
                 SBMP_LoadTgaPalette(pHeader, pBytes + pHeader->byIdLength + 0x12, nPaletteBytes, pPalette) != 0) &&
                (pbPixels == NULL ||
                 SBMP_DecodeTgaPixels(pHeader, pBytes + pHeader->byIdLength + nPaletteBytes + 0x12,
                                      nDataSize - (pHeader->byIdLength + nPaletteBytes + 0x12), pbPixels,
                                      nPixelBufSize) != 0)) {
                bResult = 1;
            }
        }
    }
    return bResult;
}

// 1.14d mac: 0000c421
// The same decoders over a file: opened through SFile, the format picked from its first
// two bytes, the rest read in pieces.
bool SBmpLoadImage(const char* szFile, uint8_t* pPalette, uint8_t* pPixels, size_t nPixelBufSize, uint32_t* pnWidth,
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
    if (*szFile == '\0' || szFile == NULL || (nPixelBufSize == 0 && pPixels != NULL)) {
        SErrSetLastError(0x57);
        return false;
    }
    D2MPQFileStrc* pFile;
    if (SFileOpenFile(szFile, &pFile) == 0) {
        return false;
    }
    bool bResult = false;
    int16_t nMagic;
    if (SFileReadFile(pFile, &nMagic, 2, NULL, NULL) != 0) {
        SFileSetFilePointer(pFile, -2, NULL, 1);
        if (nMagic == 0x50a) {
            PcxHeader tHeader;
            if (SFileReadFile(pFile, &tHeader, 0x80, NULL, NULL) != 0) {
                uint32_t nWidth = (tHeader.wXMax + 1) - (uint32_t)tHeader.wXMin;
                if (pnWidth != NULL) {
                    *pnWidth = nWidth;
                }
                if (pnHeight != NULL) {
                    *pnHeight = (tHeader.wYMax + 1) - (uint32_t)tHeader.wYMin;
                }
                uint32_t nBpp = (uint32_t)tHeader.byBitsPerPixel * (uint32_t)tHeader.byPlanes;
                if (pnBpp != NULL) {
                    *pnBpp = nBpp;
                }
                bool bReadPalette = false;
                if (pPixels == NULL) {
                    bResult = true;
                    if (nBpp == 8 && pPalette != NULL) {
                        SFileSetFilePointer(pFile, -0x300, NULL, 2);
                        bReadPalette = true;
                    }
                } else {
                    DWORD dwSize = SFileGetFileSize(pFile, NULL);
                    DWORD dwPos = SFileSetFilePointer(pFile, 0, NULL, 1);
                    int nTrailer = 0;
                    if (nBpp == 8) {
                        nTrailer = -0x300;
                    }
                    size_t nCompressed = nTrailer + (dwSize - dwPos);
                    void* pBuffer = SMemAlloc(nCompressed, SBMP_FILE, 0x466, 0);
                    SFileReadFile(pFile, pBuffer, nCompressed, NULL, NULL);
                    UncompressPcxImage((const uint8_t*)pBuffer, pPixels, nPixelBufSize, nCompressed - (nBpp == 8),
                                       nWidth, tHeader.wBytesPerLine, tHeader.byPlanes);
                    SMemFree(pBuffer, SBMP_FILE, 0x46f, 0);
                    bResult = true;
                    if (nBpp == 8 && pPalette != NULL) {
                        bReadPalette = true;
                    }
                }
                if (bReadPalette) {
                    uint8_t abyEntries[0x300];
                    SFileReadFile(pFile, abyEntries, 0x300, NULL, NULL);
                    bResult = true;
                    for (int i = 0; i != 0x100; i++) {
                        pPalette[i * 4] = abyEntries[i * 3];
                        pPalette[i * 4 + 1] = abyEntries[i * 3 + 1];
                        pPalette[i * 4 + 2] = abyEntries[i * 3 + 2];
                        pPalette[i * 4 + 3] = 0;
                    }
                }
            }
        } else if (nMagic == 0x4d42) {
            BmpFileHeader tFileHeader;
            BmpInfoHeader tInfoHeader;
            // the file header is 14 bytes: read into a buffer of its own
            uint8_t abyFileHeader[0xe];
            if (SFileReadFile(pFile, abyFileHeader, 0xe, NULL, NULL) != 0 &&
                SFileReadFile(pFile, &tInfoHeader, 0x28, NULL, NULL) != 0) {
                memcpy(&tFileHeader, abyFileHeader, 0xe);
                if (pnWidth != NULL) {
                    *pnWidth = tInfoHeader.nWidth;
                }
                if (pnHeight != NULL) {
                    *pnHeight = tInfoHeader.nHeight;
                }
                if (pnBpp != NULL) {
                    *pnBpp = tInfoHeader.wBitCount;
                }
                bResult = false;
                if (8 < tInfoHeader.wBitCount) {
                    pPalette = NULL;
                }
                bool bOk = true;
                if (pPalette != NULL) {
                    uint8_t abyPalette[0x400];
                    if (SFileReadFile(pFile, abyPalette, 0x400, NULL, NULL) == 0) {
                        bOk = false;
                    } else {
                        for (int i = 0; i != 0x100; i++) {
                            pPalette[i * 4] = abyPalette[i * 4 + 2];
                            pPalette[i * 4 + 1] = abyPalette[i * 4 + 1];
                            pPalette[i * 4 + 2] = abyPalette[i * 4];
                            pPalette[i * 4 + 3] = 0;
                        }
                    }
                }
                if (bOk) {
                    bResult = true;
                    if (pPixels != NULL) {
                        int nHeaderBytes = -0x36;
                        if (pPalette != NULL) {
                            nHeaderBytes = -0x436;
                        }
                        SFileSetFilePointer(pFile, nHeaderBytes + tFileHeader.dwOffBits, NULL, 1);
                        size_t nRead = 0;
                        void* pBuffer = SMemAlloc(tInfoHeader.dwSizeImage, SBMP_FILE, 0x1fa, 0);
                        SFileReadFile(pFile, pBuffer, tInfoHeader.dwSizeImage, &nRead, NULL);
                        uint32_t nRowBytes = (int)(tInfoHeader.wBitCount * tInfoHeader.nWidth) >> 3;
                        uint32_t nPitch = nRowBytes;
                        if ((nRowBytes & 3) != 0) {
                            nPitch = (nRowBytes + 4) - (nRowBytes & 3);
                        }
                        SBMP_CopyBottomUpRows(pBuffer, pPixels, nPixelBufSize, nRowBytes, nPitch, tInfoHeader.nHeight);
                        SMemFree(pBuffer, SBMP_FILE, 0x206, 0);
                        bResult = !(nRead < tInfoHeader.dwSizeImage);
                    }
                }
            }
        } else {
            TgaHeader tHeader;
            size_t nRead;
            if (SFileReadFile(pFile, &tHeader, 0x12, &nRead, NULL) != 0 && nRead == 0x12 &&
                SBMP_IsValidTgaHeader(&tHeader) != 0) {
                uint32_t nPaletteBytes = 0;
                if (tHeader.byColorMapType != 0) {
                    nPaletteBytes = (uint32_t)tHeader.wColorMapLength * ((tHeader.byColorMapDepth + 7) >> 3);
                }
                int nPixelBytes = 1;
                if (tHeader.byImageType < 9) {
                    nPixelBytes = ((tHeader.byPixelDepth + 7) >> 3) * (uint32_t)tHeader.wHeight * (uint32_t)tHeader.wWidth;
                }
                uint32_t nIdLength = tHeader.byIdLength;
                bResult = false;
                if (nIdLength + 0x12 + nPixelBytes + nPaletteBytes <= SFileGetFileSize(pFile, NULL)) {
                    if (pnWidth != NULL) {
                        *pnWidth = tHeader.wWidth;
                    }
                    if (pnHeight != NULL) {
                        *pnHeight = tHeader.wHeight;
                    }
                    if (pnBpp != NULL) {
                        *pnBpp = tHeader.byPixelDepth;
                    }
                    SFileSetFilePointer(pFile, tHeader.byIdLength, NULL, 1);
                    bool bOk = true;
                    if (pPalette == NULL || tHeader.byColorMapType == 0) {
                        bResult = true;
                    } else {
                        uint8_t abyPalette[0x400];
                        SFileReadFile(pFile, abyPalette, nPaletteBytes, NULL, NULL);
                        bResult = SBMP_LoadTgaPalette(&tHeader, abyPalette, nPaletteBytes, pPalette) != 0;
                        bOk = bResult;
                    }
                    if (bOk && pPixels != NULL) {
                        size_t nPixelSize = ((SFileGetFileSize(pFile, NULL) - 0x12 - nPaletteBytes)) - tHeader.byIdLength;
                        uint8_t* pBuffer = (uint8_t*)SMemAlloc(nPixelSize, SBMP_FILE, 0x111, 0);
                        SFileReadFile(pFile, pBuffer, nPixelSize, NULL, NULL);
                        if (SBMP_DecodeTgaPixels(&tHeader, pBuffer, nPixelSize, pPixels, nPixelBufSize) != 0) {
                            SMemFree(pBuffer, SBMP_FILE, 0x11b, 0);
                            bResult = true;
                        } else {
                            SMemFree(pBuffer, SBMP_FILE, 0x118, 0);
                            bResult = false;
                        }
                    }
                }
            }
        }
    }
    SFileCloseFile(pFile);
    return bResult;
}

// 1.14d mac: 0000ccb4
// Rows from the last source row up, packed into the destination until it is full.
SBMP_LOCAL void SBMP_CopyBottomUpRows(const void* pSrc, void* pDest, size_t nDestSize, size_t nRowBytes, int nSrcPitch,
                                  int nRows) {
    if (nRows == 0 || nSrcPitch == 0 || nRowBytes == 0 || nDestSize == 0 || pSrc == NULL || pDest == NULL) {
        SErrSetLastError(0x57);
        return;
    }
    const uint8_t* pSrcRow = (const uint8_t*)pSrc + (nRows - 1) * nSrcPitch;
    uint8_t* pDestCursor = (uint8_t*)pDest;
    do {
        size_t nCopy = (intptr_t)pDest + nDestSize - (intptr_t)pDestCursor;
        if ((int)nRowBytes < (int)nCopy) {
            nCopy = nRowBytes;
        }
        nRows--;
        if (nCopy == 0) {
            return;
        }
        if (0 < (int)nCopy) {
            memmove(pDestCursor, pSrcRow, nCopy);
        }
        pSrcRow -= nSrcPitch;
        pDestCursor += nCopy;
    } while (nRows != 0);
}

// 1.14d mac: 0000cd40
// Image types 1, 2, 9 and 10; colour-mapped 8-bit with a 24 or 32 bit map, or true colour
// 24 or 32 bit with no map.
SBMP_LOCAL bool SBMP_IsValidTgaHeader(const TgaHeader* pHeader) {
    if (10 < pHeader->byImageType) {
        return false;
    }
    if ((0x606U >> (pHeader->byImageType & 0x1f) & 1) == 0) {
        return false;
    }
    if (pHeader->byColorMapType == 1) {
        if (pHeader->byPixelDepth != 8) {
            return false;
        }
        if (0x100 < (int)pHeader->wColorMapFirst + (int)pHeader->wColorMapLength) {
            return false;
        }
        if (pHeader->byColorMapDepth != 0x18 && pHeader->byColorMapDepth != 0x20) {
            return false;
        }
    } else {
        if (pHeader->byColorMapType != 0) {
            return false;
        }
        if (pHeader->byPixelDepth == 0x20) {
            if ((pHeader->byDescriptor & 0xf) != 8) {
                return false;
            }
        } else if (pHeader->byPixelDepth == 0x18) {
            if ((pHeader->byDescriptor & 0xf) != 0) {
                return false;
            }
        } else {
            return false;
        }
    }
    return pHeader->byDescriptor < 0x40;
}

// 1.14d mac: 0000cdbe
// The colour map into a 256-entry palette, the rest of it zeroed.
SBMP_LOCAL int SBMP_LoadTgaPalette(const TgaHeader* pHeader, const uint8_t* pEntries, int nExpected, uint8_t* pPalette) {
    uint16_t nLength = pHeader->wColorMapLength;
    uint32_t nDepth = pHeader->byColorMapDepth + 7;
    uint32_t nBytesPerEntry = nDepth >> 3;
    int bResult = 0;
    if (nLength * nBytesPerEntry - nExpected == 0) {
        uint32_t nIndex = pHeader->wColorMapFirst;
        if (pHeader->wColorMapFirst != 0) {
            bzero(pPalette, nIndex * 4);
        }
        if (nLength != 0) {
            uint32_t nEnd = nLength + nIndex;
            const uint8_t* pSrc = pEntries + 3;
            uint32_t nNext = nIndex + 1;
            do {
                pPalette[nIndex * 4] = pSrc[-1];
                pPalette[nIndex * 4 + 1] = pSrc[-2];
                pPalette[nIndex * 4 + 2] = pSrc[-3];
                uint8_t byAlpha = 0;
                if (0x1f < nDepth) {
                    byAlpha = *pSrc;
                }
                pPalette[nIndex * 4 + 3] = byAlpha;
                pSrc += nBytesPerEntry;
                nIndex++;
            } while ((int)nIndex < (int)nEnd);
            nIndex = nNext;
            if (nNext <= nEnd) {
                nIndex = nEnd;
            }
        }
        size_t n = (0x100 - nIndex) * 4;
        bResult = 1;
        if (0 < (int)n) {
            bzero(pPalette + nIndex * 4, n);
        }
    }
    return bResult;
}

// 1.14d mac: 0000ce80
// Raw (types 1, 2) or run-length (9, 10) pixels, placed by the descriptor's origin bits.
SBMP_LOCAL int SBMP_DecodeTgaPixels(const TgaHeader* pHeader, const uint8_t* pSrc, int nSrcSize, uint8_t* pDest,
                                size_t nDestSize) {
    uint32_t nBytesPerPixel = (pHeader->byPixelDepth + 7) >> 3;
    uint8_t byDescriptor = pHeader->byDescriptor;
    uint32_t nPixelStep = -nBytesPerPixel;
    if ((byDescriptor & 0x10) == 0) {
        nPixelStep = nBytesPerPixel;
    }
    uint32_t nWidth = pHeader->wWidth;
    size_t nImageSize = pHeader->wHeight * nWidth * nBytesPerPixel;
    if (nImageSize - nDestSize != 0 && (int)nDestSize <= (int)nImageSize) {
        nImageSize = nDestSize;
    }
    int nRowAdjust;
    if ((byDescriptor & 0x20) == 0) {
        nRowAdjust = -nWidth * (nPixelStep + nBytesPerPixel);
    } else {
        nRowAdjust = (nBytesPerPixel - nPixelStep) * nWidth;
    }
    if ((byDescriptor & 0x10) != 0) {
        pDest += (nWidth - 1) * nBytesPerPixel;
    }
    if ((byDescriptor & 0x20) == 0) {
        pDest += (pHeader->wHeight - 1) * nWidth * nBytesPerPixel;
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
        uint32_t nLeft = nColumn - 1;                             \
        nColumn = nLeft;                                          \
        if (nLeft == 0) {                                         \
            nColumn = nWidth;                                     \
        }                                                         \
        pDest += (nLeft == 0 ? nRowAdjust : 0) + nPixelStep;      \
    } while (0)
    uint32_t nColumn = nWidth;
    if ((uint8_t)(pHeader->byImageType - 9) < 2) {
        uint32_t nPixelsLeft = (int)nImageSize / (int)nBytesPerPixel;
        if (nPixelsLeft != 0 && 0 < nSrcSize) {
            const uint8_t* pEnd = pSrc + nSrcSize;
            const uint8_t* pNext;
            do {
                const uint8_t* pData = pSrc + 1;
                uint32_t nPacket = *pSrc;
                uint32_t nCount;
                if ((nPacket & 0x80) == 0) {
                    nCount = nPacket + 1;
                    if ((int)nPixelsLeft < (int)(nPacket + 1)) {
                        nCount = nPixelsLeft;
                    }
                    if (pEnd < pSrc + nCount * nBytesPerPixel + 1) {
                        return 0;
                    }
                    for (uint32_t i = 0; i < nCount; i++) {
                        SBMP_PUT_PIXEL(pData);
                        pData += nBytesPerPixel;
                    }
                    pNext = pSrc + nCount * nBytesPerPixel + 1;
                } else {
                    nCount = (nPacket & 0x7f) + 1;
                    if ((int)nPixelsLeft < (int)nCount) {
                        nCount = nPixelsLeft;
                    }
                    pNext = pSrc + nBytesPerPixel + 1;
                    if (pEnd < pNext) {
                        return 0;
                    }
                    for (uint32_t i = 0; i < nCount; i++) {
                        SBMP_PUT_PIXEL(pData);
                    }
                }
                nPixelsLeft -= nCount;
                pSrc = pNext;
            } while (nPixelsLeft != 0 && pNext < pEnd);
        }
    } else {
        if (1 < (uint8_t)(pHeader->byImageType - 1)) {
            return 0;
        }
        if (nSrcSize < (int)nImageSize) {
            return 0;
        }
        if ((byDescriptor & 0x30) == 0x20) {
            if ((int)nImageSize < 1) {
                return 1;
            }
            memmove(pDest, pSrc, nImageSize);
        } else {
            int nPixels = (int)nImageSize / (int)nBytesPerPixel;
            const uint8_t* pPixel = pSrc;
            for (; nPixels != 0; nPixels--) {
                SBMP_PUT_PIXEL(pPixel);
                pPixel += nBytesPerPixel;
            }
        }
    }
#undef SBMP_PUT_PIXEL
    return 1;
}

// 1.14d mac: 0000d1f8
// PCX run-length data into pixels. Each scanline holds nPlanes planes of nBytesPerLine
// bytes, the planes written interleaved (the last plane first); stops at either end.
SBMP_LOCAL void UncompressPcxImage(const uint8_t* pSrc, uint8_t* pDest, uint32_t nDestSize, int nSrcSize,
                               uint32_t nWidth, uint32_t nBytesPerLine, int nPlanes) {
    int nRowStride = nPlanes * nWidth;
    uint8_t* pDestEnd = pDest + nDestSize;
    const uint8_t* pSrcEnd = pSrc + nSrcSize;
    int nPlane = nPlanes - 1;
    uint32_t nColumn = 0;
    for (;;) {
        if (pDestEnd <= pDest + nColumn || pSrcEnd <= pSrc) {
            return;
        }
        uint8_t byValue = *pSrc++;
        uint32_t nRun = 1;
        if (0xbf < byValue) {
            nRun = byValue & 0x3f;
            if ((int)(pDestEnd - pDest) <= (int)nRun) {
                nRun = pDestEnd - pDest;
            }
            byValue = *pSrc++;
        }
        while (nRun != 0) {
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
        }
    }
}

#endif  // D2_OS_MAC
