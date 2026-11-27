// StormMac Storm Mac/SOURCE/SBig.cpp: Storm's big-number arithmetic, 1.14d Intel Mac
// 0000a629..0000bf46.
//
// A big number (BigData, 0x168 bytes) is a result buffer, a stack of sixteen scratch
// buffers with a depth counter, and a byte array the binary form is built in. Every
// buffer is a growable array of dwords with a start word (BigBuffer, 0x14 bytes). The
// exports (SBigAdd ...) forward to the BigBuffer routines with the scratch stack.
//
// SBigNew's __FILE__ is ".../StormMac/Storm Mac/SOURCE/SBig.cpp", line 0x51e. The run
// before it, from SBigAdd, is the exported functions in alphabetical order; they and the
// BigBuffer routines and array templates after them are placed here up to SBLT.CPP's
// first function.
#include "../Win32Mac/Source/Win32/Win32Mac.h"

#if !defined(_WIN32) && D2_OS_MAC

#include <string.h>

void* SMemAlloc(uint32_t nBytes, const char* szFile, int nLine, uint32_t dwFlags);
int SMemFree(void* pMem, const char* szFile, int nLine, uint32_t dwFlags);
void* SMemReAlloc(void* pMem, uint32_t nBytes, const char* szFile, int nLine, uint32_t dwFlags);

#define SBIG_FILE "/Users/bclemetson/dev/diablo2/Diablo2/Source/DiabloAll/../../../StormMac/Storm Mac/SOURCE/SBig.cpp"

struct BigBuffer {
    uint32_t nCapacity;   // 0x00  m_alloc
    uint32_t nCount;      // 0x04  m_count
    uint32_t* pData;      // 0x08  m_data
    uint32_t nGrowStep;   // 0x0C  m_chunk
    uint32_t nStartWord;  // 0x10  m_offset: the number's lowest word
};

struct ByteArray {
    uint32_t nCapacity;
    uint32_t nCount;
    uint8_t* pData;
    uint32_t nGrowStep;
};

struct BigStack {
    BigBuffer atBuffers[16];
    int32_t nDepth;  // 0x140: buffers in use
};

struct BigData {
    BigBuffer tResult;  // 0x000
    BigStack tStack;    // 0x014
    ByteArray tBytes;   // 0x158: the binary form
};

typedef char BigBuffer_size[sizeof(BigBuffer) == 0x14 ? 1 : -1];
typedef char BigData_size[sizeof(BigData) == 0x168 ? 1 : -1];

void BigBuffer_SetZero(BigBuffer* pBuffer);
void TSBaseArray_Ui_Copy(BigBuffer* pArray, uint32_t nCount, uint32_t* pSrc);
void SBIG_GrowToIndexNoZero(BigBuffer* pBuffer, int32_t nIndex);
void SBIG_TrimLeadingZeros(BigBuffer* pBuffer);
void TSFixedArray_Ui_Set(BigBuffer* pArray, int nCount, const uint32_t* pSrc);
void SBIG_BigBuffer_GetWord(BigStack* pStack, BigBuffer* pDest, BigBuffer* pSource);
void BigBuffer_Add(BigBuffer* pDest, BigBuffer* pSourceA, BigBuffer* pSourceB);
int32_t BigBuffer_Compare(BigBuffer* pA, BigBuffer* pB);
void BigBuffer_Div(BigBuffer* pQuotient, BigBuffer* pRemainder, BigBuffer* pDividend, BigBuffer* pDivisor,
                          BigStack* pStack);
void BigBuffer_FromBinary(BigBuffer* pResult, const void* pSrcBytes, uint32_t nByteCount);
void BigBuffer_Mul(BigBuffer* pResult, BigBuffer* pA, BigBuffer* pB, BigStack* pStack);
void BigBuffer_MulMod(BigBuffer* pResult, BigBuffer* pA, BigBuffer* pB, BigBuffer* pModulus,
                             BigStack* pStack);
void BigBuffer_PowMod(BigBuffer* pResult, BigBuffer* pBase, BigBuffer* pExponent, BigBuffer* pModulus,
                             BigStack* pStack);
void BigBuffer_Shl(BigBuffer* pResult, BigBuffer* pSource, uint32_t nShiftBits);
void BigBuffer_Shr(BigBuffer* pResult, BigBuffer* pSource, uint32_t nShiftBits);
void BigBuffer_Square(BigBuffer* pResult, BigBuffer* pInput, BigStack* pStack);
void BigBuffer_Sub(BigBuffer* pResult, BigBuffer* pLhs, BigBuffer* pRhs);
void BigBuffer_Div_Ux(uint32_t* pnRemainder, BigBuffer* pQuotient, BigBuffer* pDividend, uint32_t nDivisorLo,
                             uint32_t nDivisorHi);
void BigBuffer_Mul_Ux(BigBuffer* pDest, BigBuffer* pSource, uint32_t nMultiplierLo, uint32_t nMultiplierHi);
void SBIG_ToBinaryAppend(ByteArray* pBytes, BigBuffer* pBig);

// ---- the exports ----

// 1.14d mac: 0000a629
void SBigAdd(BigData* pResult, BigData* pBigA, BigData* pBigB) {
    BigBuffer_Add(&pResult->tResult, &pBigA->tResult, &pBigB->tResult);
}

// 1.14d mac: 0000a645
void BigBuffer_Add(BigBuffer* pDest, BigBuffer* pSourceA, BigBuffer* pSourceB) {
    uint32_t nCarry = 0;
    int i = 0;
    while (true) {
        if (nCarry == 0 && pSourceA->nCount <= i + pSourceA->nStartWord &&
            pSourceB->nCount <= pSourceB->nStartWord + i) {
            SBIG_GrowToIndexNoZero(pDest, i);
            return;
        }
        uint32_t nIndexA = pSourceA->nStartWord + i;
        uint32_t nWordA = 0;
        if (nIndexA < pSourceA->nCount) {
            nWordA = pSourceA->pData[nIndexA];
        }
        uint32_t nIndexB = pSourceB->nStartWord + i;
        uint32_t nWordB = 0;
        if (nIndexB < pSourceB->nCount) {
            nWordB = pSourceB->pData[nIndexB];
        }
        TSBaseArray_Ui_Copy(pDest, pDest->nStartWord + i, (uint32_t*)1);
        pDest->pData[pDest->nStartWord + i] = nWordA + nCarry + nWordB;
        nCarry = (uint32_t)((nWordA + nCarry) < nWordA) + (uint32_t)((nWordA + nCarry + nWordB) < nWordB);
        i++;
    }
}

// 1.14d mac: 0000a71d
// Only the SHA-1 code calls it (a forwarder onto BigBuffer_Compare).
int32_t SBIG_CompareBigBuffers_ForSha1(BigData* pBigA, BigData* pBigB) {
    return BigBuffer_Compare(&pBigA->tResult, &pBigB->tResult);
}

// 1.14d mac: 0000a72c
int32_t BigBuffer_Compare(BigBuffer* pA, BigBuffer* pB) {
    uint32_t nCountB = pB->nCount;
    uint32_t nIndexB = pB->nStartWord;
    uint32_t nIndexA = pA->nStartWord;
    int32_t nResult = 0;
    while (true) {
        uint32_t nWordA;
        if (nIndexA < pA->nCount) {
            nWordA = pA->pData[nIndexA];
        } else {
            nWordA = 0;
            if (nCountB <= nIndexB) {
                return nResult;
            }
        }
        uint32_t nWordB = 0;
        if (nIndexB < nCountB) {
            nWordB = pB->pData[nIndexB];
        }
        if (nWordA != nWordB) {
            uint32_t nHigherB = 0;
            nWordA = 0;
            if (nIndexA < pA->nCount) {
                nWordA = pA->pData[nIndexA];
            }
            if (nIndexB < nCountB) {
                nHigherB = pB->pData[nIndexB];
            }
            nResult = -1;
            if (nHigherB < nWordA) {
                nResult = 1;
            }
        }
        nIndexA++;
        nIndexB++;
    }
}

// 1.14d mac: 0000a7b6
// The array of each buffer is freed, then the object itself.
void SBigDel(BigData* pBig) {
    if (pBig != NULL) {
        if (pBig->tBytes.pData != NULL) {
            SMemFree(pBig->tBytes.pData, "nothing", -2, 0);
        }
        for (int i = 16; i != 0; i--) {
            if (pBig->tStack.atBuffers[i - 1].pData != NULL) {
                SMemFree(pBig->tStack.atBuffers[i - 1].pData, "nothing", -2, 0);
            }
        }
        if (pBig->tResult.pData != NULL) {
            SMemFree(pBig->tResult.pData, "nothing", -2, 0);
        }
        operator delete(pBig);
    }
}

// 1.14d mac: 0000a87e
// quotient and remainder of dividend / divisor; the scratch stack holds the scaled copies.
void BigBuffer_Div(BigBuffer* pQuotient, BigBuffer* pRemainder, BigBuffer* pDividend, BigBuffer* pDivisor,
                          BigStack* pStack) {
    SBIG_TrimLeadingZeros(pDividend);
    SBIG_TrimLeadingZeros(pDivisor);
    uint32_t nDivisorLen = pDivisor->nCount - pDivisor->nStartWord;
    uint32_t nDividendLen = pDividend->nCount - pDividend->nStartWord;
    if (nDividendLen < nDivisorLen) {
        if (pRemainder != pDividend) {
            TSFixedArray_Ui_Set(pRemainder, pDividend->nCount, pDividend->pData);
        }
        pRemainder->nGrowStep = pDividend->nGrowStep;
        pRemainder->nStartWord = pDividend->nStartWord;
        BigBuffer_SetZero(pQuotient);
    } else if (nDivisorLen < 2) {
        TSBaseArray_Ui_Copy(pRemainder, pRemainder->nStartWord, (uint32_t*)1);
        uint32_t nDivisor = 0;
        if (pDivisor->nStartWord < pDivisor->nCount) {
            nDivisor = pDivisor->pData[pDivisor->nStartWord];
        }
        BigBuffer_Div_Ux(pRemainder->pData + pRemainder->nStartWord, pQuotient, pDividend, nDivisor, 0);
        SBIG_GrowToIndexNoZero(pRemainder, 1);
    } else {
        int nTop = pStack->nDepth;
        pStack->nDepth = nTop + 3;
        // the shift that puts the divisor's top bit at the top of a word
        uint32_t nShift = 0x1f;
        for (int nWord = pDivisor->nCount - pDivisor->nStartWord; nWord != 0; nWord--) {
            uint32_t nIndex = nWord - 1 + pDivisor->nStartWord;
            if (nIndex < pDivisor->nCount && pDivisor->pData[nIndex] != 0) {
                uint32_t nBit = 0x40;
                uint32_t dwMask = 0x80000000;
                bool bFound = false;
                while (nBit != 0x20) {
                    nBit--;
                    uint32_t dwTest = pDivisor->pData[nIndex] & dwMask;
                    dwMask >>= 1;
                    if (dwTest != 0) {
                        nShift = (nBit & 0x1f) ^ 0x1f;
                        bFound = true;
                        break;
                    }
                }
                if (bFound) {
                    break;
                }
            }
        }
        BigBuffer* pWork = &pStack->atBuffers[nTop];
        BigBuffer* pScaledDivisor = &pStack->atBuffers[nTop + 1];
        BigBuffer* pProduct = &pStack->atBuffers[nTop + 2];
        BigBuffer_Shl(pWork, pDividend, nShift);
        BigBuffer_Shl(pScaledDivisor, pDivisor, nShift);
        int nTopWord = nDivisorLen - 1;
        TSBaseArray_Ui_Copy(pScaledDivisor, pScaledDivisor->nStartWord - 1 + nDivisorLen, (uint32_t*)1);
        uint32_t nTopDivisor = pScaledDivisor->pData[pScaledDivisor->nStartWord + nTopWord];
        int nQuotientLen = nDividendLen + 1;
        SBIG_GrowToIndexNoZero(pQuotient, nQuotientLen - nDivisorLen);
        uint32_t nStep = nQuotientLen - nDivisorLen;
        if (nStep != 0) {
            uint32_t nEstimator = nTopDivisor + 1;
            do {
                uint32_t nNext = nStep - 1;
                pQuotient->nStartWord = nNext;
                if (nNext == 0) {
                    pWork->nStartWord = 0;
                    nStep = 0;
                } else {
                    TSBaseArray_Ui_Copy(pQuotient, nStep - 2, (uint32_t*)1);
                    pWork->nStartWord = nNext;
                    TSBaseArray_Ui_Copy(pWork, nStep - 2, (uint32_t*)1);
                    nStep = pWork->nStartWord;
                }
                if (nEstimator == 0) {
                    TSBaseArray_Ui_Copy(pWork, nStep + nDivisorLen, (uint32_t*)1);
                    uint32_t nDigit = pWork->pData[pWork->nStartWord + nDivisorLen];
                    TSBaseArray_Ui_Copy(pQuotient, pQuotient->nStartWord, (uint32_t*)1);
                    pQuotient->pData[pQuotient->nStartWord] = nDigit;
                } else {
                    TSBaseArray_Ui_Copy(pWork, nStep + nTopWord, (uint32_t*)1);
                    uint32_t nLow = pWork->pData[pWork->nStartWord + nTopWord];
                    TSBaseArray_Ui_Copy(pWork, pWork->nStartWord + nDivisorLen, (uint32_t*)1);
                    uint32_t nHigh = pWork->pData[pWork->nStartWord + nDivisorLen];
                    TSBaseArray_Ui_Copy(pQuotient, pQuotient->nStartWord, (uint32_t*)1);
                    uint32_t* pDigits = pQuotient->pData;
                    uint32_t nDigitIndex = pQuotient->nStartWord;
                    pDigits[nDigitIndex] = (uint32_t)((((uint64_t)nHigh << 32) | nLow) / nEstimator);
                }
                TSBaseArray_Ui_Copy(pQuotient, pQuotient->nStartWord, (uint32_t*)1);
                if (pQuotient->pData[pQuotient->nStartWord] != 0) {
                    TSBaseArray_Ui_Copy(pQuotient, pQuotient->nStartWord, (uint32_t*)1);
                    BigBuffer_Mul_Ux(pProduct, pScaledDivisor, pQuotient->pData[pQuotient->nStartWord], 0);
                    BigBuffer_Sub(pWork, pWork, pProduct);
                }
                while (true) {
                    TSBaseArray_Ui_Copy(pWork, pWork->nStartWord + nDivisorLen, (uint32_t*)1);
                    if (pWork->pData[pWork->nStartWord + nDivisorLen] == 0 &&
                        BigBuffer_Compare(pWork, pScaledDivisor) < 0) {
                        break;
                    }
                    TSBaseArray_Ui_Copy(pQuotient, pQuotient->nStartWord, (uint32_t*)1);
                    pQuotient->pData[pQuotient->nStartWord]++;
                    BigBuffer_Sub(pWork, pWork, pScaledDivisor);
                }
                nStep = nNext;
            } while (nStep != 0);
        }
        BigBuffer_Shr(pRemainder, pWork, nShift);
        SBIG_TrimLeadingZeros(pRemainder);
        pStack->nDepth = pStack->nDepth - 3;
    }
}

// 1.14d mac: 0000ace5
void SBigFromBinary(BigData* pDstBignum, const void* pSrcBytes, uint32_t nByteCount) {
    BigBuffer_FromBinary(&pDstBignum->tResult, pSrcBytes, nByteCount);
}

// 1.14d mac: 0000ad01
// Little-endian bytes into words.
void BigBuffer_FromBinary(BigBuffer* pResult, const void* pSrcBytes, uint32_t nByteCount) {
    BigBuffer_SetZero(pResult);
    for (uint32_t i = 0; i != nByteCount; i++) {
        uint8_t byValue = ((const uint8_t*)pSrcBytes)[i];
        uint32_t nWordIndex = i >> 2;
        uint32_t dwCurWord;
        if ((i & 3) == 0) {
            dwCurWord = 0;
        } else {
            TSBaseArray_Ui_Copy(pResult, pResult->nStartWord + nWordIndex, (uint32_t*)1);
            dwCurWord = pResult->pData[pResult->nStartWord + nWordIndex];
        }
        TSBaseArray_Ui_Copy(pResult, pResult->nStartWord + nWordIndex, (uint32_t*)1);
        pResult->pData[nWordIndex + pResult->nStartWord] = ((uint32_t)byValue << ((i & 3) << 3)) + dwCurWord;
    }
}

// 1.14d mac: 0000ada4
void SBigFromUnsigned(BigData* pDstBignum, uint32_t dwValue) {
    TSBaseArray_Ui_Copy(&pDstBignum->tResult, pDstBignum->tResult.nStartWord, (uint32_t*)1);
    pDstBignum->tResult.pData[pDstBignum->tResult.nStartWord] = dwValue;
    SBIG_GrowToIndexNoZero(&pDstBignum->tResult, 1);
}

// 1.14d mac: 0000ade7
void SBigMul(BigData* pResult, BigData* pBigA, BigData* pBigB) {
    BigBuffer_Mul(&pResult->tResult, &pBigA->tResult, &pBigB->tResult, &pResult->tStack);
}

// 1.14d mac: 0000ae0a
// Schoolbook multiplication; into a scratch buffer when the result aliases an operand.
void BigBuffer_Mul(BigBuffer* pResult, BigBuffer* pA, BigBuffer* pB, BigStack* pStack) {
    BigBuffer* pArray = pResult;
    if (pResult == pA || pResult == pB) {
        pArray = &pStack->atBuffers[pStack->nDepth++];
    }
    BigBuffer_SetZero(pArray);
    uint32_t nCountA = pA->nCount;
    uint32_t nStartA = pA->nStartWord;
    if (nStartA < nCountA) {
        int iOuter = 0;
        uint32_t nCarry;
        do {
            uint32_t nCountB = pB->nCount;
            uint32_t nStartB = pB->nStartWord;
            int iInner = 0;
            nCarry = 0;
            if (nStartB < nCountB) {
                while (true) {
                    uint32_t nWordA = 0;
                    if (nStartA + iOuter < nCountA) {
                        nWordA = pA->pData[nStartA + iOuter];
                    }
                    uint32_t nWordB = 0;
                    if (nStartB + iInner < nCountB) {
                        nWordB = pB->pData[nStartB + iInner];
                    }
                    int iDest = iOuter + iInner;
                    uint64_t nProduct = (uint64_t)nWordB * (uint64_t)nWordA;
                    uint32_t nLow = (uint32_t)nProduct;
                    uint32_t nSum = nLow + nCarry;
                    TSBaseArray_Ui_Copy(pArray, pArray->nStartWord + iDest, (uint32_t*)1);
                    uint32_t* pDest = &pArray->pData[pArray->nStartWord + iDest];
                    uint32_t nOld = *pDest;
                    nCarry = (uint32_t)(nProduct >> 32) + (uint32_t)(nSum < nLow) + (uint32_t)((nSum + nOld) < nSum);
                    TSBaseArray_Ui_Copy(pArray, pArray->nStartWord + iDest, (uint32_t*)1);
                    pArray->pData[iDest + pArray->nStartWord] = nSum + nOld;
                    nCountB = pB->nCount;
                    nStartB = pB->nStartWord;
                    iInner++;
                    if (nCountB <= iInner + nStartB) {
                        break;
                    }
                    nCountA = pA->nCount;
                    nStartA = pA->nStartWord;
                }
            }
            TSBaseArray_Ui_Copy(pArray, pArray->nStartWord + iInner + iOuter, (uint32_t*)1);
            pArray->pData[iInner + iOuter + pArray->nStartWord] = nCarry;
            nCountA = pA->nCount;
            nStartA = pA->nStartWord;
            iOuter++;
        } while (nStartA + iOuter < nCountA);
    }
    SBIG_BigBuffer_GetWord(pStack, pResult, pArray);
}

// 1.14d mac: 0000afda
// result = a * b mod modulus.
void BigBuffer_MulMod(BigBuffer* pResult, BigBuffer* pA, BigBuffer* pB, BigBuffer* pModulus,
                             BigStack* pStack) {
    int nDepth = pStack->nDepth;
    pStack->nDepth = nDepth + 1;
    BigBuffer* pTemp = &pStack->atBuffers[nDepth];
    BigBuffer_Mul(pTemp, pA, pB, pStack);
    BigBuffer_Div(pTemp, pResult, pTemp, pModulus, pStack);
    pStack->nDepth = pStack->nDepth - 1;
}

// 1.14d mac: 0000b032
void SBigNew(BigData** ppBigData) {
    BigData* pBigNum = (BigData*)SMemAlloc(sizeof(BigData), SBIG_FILE, 0x51e, 0);
    BigData* pResult = NULL;
    if (pBigNum != NULL) {
        pBigNum->tResult.nCount = 0;
        pBigNum->tResult.nCapacity = 0;
        pBigNum->tResult.nGrowStep = 0;
        pBigNum->tResult.pData = NULL;
        pBigNum->tResult.nStartWord = 0;
        for (int i = 0; i < 16; i++) {
            BigBuffer* pBuffer = &pBigNum->tStack.atBuffers[i];
            pBuffer->nCount = 0;
            pBuffer->nCapacity = 0;
            pBuffer->nGrowStep = 0;
            pBuffer->pData = NULL;
            pBuffer->nStartWord = 0;
        }
        pBigNum->tStack.nDepth = 0;
        pBigNum->tBytes.nCount = 0;
        pBigNum->tBytes.nCapacity = 0;
        pBigNum->tBytes.nGrowStep = 0;
        pBigNum->tBytes.pData = NULL;
        pResult = pBigNum;
    }
    *ppBigData = pResult;
}

// 1.14d mac: 0000b0fd
void SBigPowMod(BigData* pResult, BigData* pBase, BigData* pExponent, BigData* pModulus) {
    BigBuffer_PowMod(&pResult->tResult, &pBase->tResult, &pExponent->tResult, &pModulus->tResult, &pResult->tStack);
}

// 1.14d mac: 0000b127
// result = base ^ exponent mod modulus, two exponent bits at a time over the powers
// base^1 .. base^3.
void BigBuffer_PowMod(BigBuffer* pResult, BigBuffer* pBase, BigBuffer* pExponent, BigBuffer* pModulus,
                             BigStack* pStack) {
    SBIG_TrimLeadingZeros(pExponent);
    if (pExponent->nCount == pExponent->nStartWord) {
        SBIG_GrowToIndexNoZero(pResult, 1);
        TSBaseArray_Ui_Copy(pResult, pResult->nStartWord, (uint32_t*)1);
        pResult->pData[pResult->nStartWord] = 1;
    } else {
        int nDepth = pStack->nDepth;
        pStack->nDepth = nDepth + 3;
        BigBuffer* pQuotientSink = &pStack->atBuffers[nDepth];
        BigBuffer* pSquare = &pStack->atBuffers[nDepth + 1];
        BigBuffer* pCube = &pStack->atBuffers[nDepth + 2];
        BigBuffer* pAccumulator = pResult;
        if (pResult == pBase || pResult == pExponent || pResult == pModulus) {
            pStack->nDepth = nDepth + 4;
            pAccumulator = &pStack->atBuffers[nDepth + 3];
        }
        BigBuffer_MulMod(pSquare, pBase, pBase, pModulus, pStack);
        BigBuffer_MulMod(pCube, pSquare, pBase, pModulus, pStack);
        BigBuffer* apPowers[4] = {pQuotientSink, pBase, pSquare, pCube};
        SBIG_GrowToIndexNoZero(pAccumulator, 1);
        TSBaseArray_Ui_Copy(pAccumulator, pAccumulator->nStartWord, (uint32_t*)1);
        pAccumulator->pData[pAccumulator->nStartWord] = 1;
        uint32_t nCount = pExponent->nCount;
        uint32_t nStart = pExponent->nStartWord;
        int nWord = nCount - nStart;
        while (nWord != 0) {
            uint32_t nIndex = (nStart - 1) + nWord;
            uint32_t dwExpWord = 0;
            if (nIndex < nCount) {
                dwExpWord = pExponent->pData[nIndex];
            }
            uint32_t nBits = 0x20;
            bool bTopWord = nWord == (int)(nCount - nStart);
            nWord--;
            bool bRun = true;
            if (bTopWord && dwExpWord < 0x40000000) {
                int nLeading = -0x20;
                do {
                    nLeading += 2;
                    dwExpWord <<= 2;
                } while (dwExpWord < 0x40000000);
                if (nLeading != 0) {
                    nBits = -nLeading;
                } else {
                    bRun = false;
                }
            }
            if (bRun) {
                for (uint32_t i = 0; i < nBits; i += 2) {
                    BigBuffer_Square(pAccumulator, pAccumulator, pStack);
                    BigBuffer_Div(pQuotientSink, pAccumulator, pAccumulator, pModulus, pStack);
                    BigBuffer_Square(pAccumulator, pAccumulator, pStack);
                    BigBuffer_Div(pQuotientSink, pAccumulator, pAccumulator, pModulus, pStack);
                    if (dwExpWord >> 0x1e != 0) {
                        BigBuffer_MulMod(pAccumulator, pAccumulator, apPowers[dwExpWord >> 0x1e], pModulus, pStack);
                    }
                    dwExpWord <<= 2;
                }
            }
            if (nWord == 0) {
                break;
            }
            nCount = pExponent->nCount;
            nStart = pExponent->nStartWord;
        }
        SBIG_BigBuffer_GetWord(pStack, pResult, pAccumulator);
        pStack->nDepth = pStack->nDepth - 3;
    }
}

// 1.14d mac: 0000b38f
void BigBuffer_Shl(BigBuffer* pResult, BigBuffer* pSource, uint32_t nShiftBits) {
    uint32_t nWordShift = nShiftBits >> 5;
    int nIndex = (nWordShift + 1 + pSource->nCount) - pSource->nStartWord;
    if (nIndex != 0) {
        int iWord = pSource->nCount - pSource->nStartWord;
        uint8_t nBitShift = nShiftBits & 0x1f;
        int iNext;
        do {
            uint32_t nDest = nWordShift + iWord;
            uint32_t dwWord = 0;
            if (nWordShift <= nDest) {
                uint32_t nSrc = pSource->nStartWord + iWord;
                if (nSrc < pSource->nCount) {
                    dwWord = pSource->pData[nSrc];
                }
                dwWord <<= nBitShift;
            }
            if (nWordShift < nDest && (nShiftBits & 0x1f) != 0) {
                uint32_t dwLower = 0;
                if ((uint32_t)(iWord - 1 + pSource->nStartWord) < pSource->nCount) {
                    dwLower = pSource->pData[pSource->nStartWord + iWord - 1];
                }
                dwWord += dwLower >> ((0x20U - nBitShift) & 0x1f);
            }
            TSBaseArray_Ui_Copy(pResult, pResult->nStartWord + nDest, (uint32_t*)1);
            pResult->pData[nDest + pResult->nStartWord] = dwWord;
            iNext = (nWordShift - 1) + iWord;
            iWord--;
        } while (iNext != -1);
    }
    SBIG_GrowToIndexNoZero(pResult, nIndex);
    SBIG_TrimLeadingZeros(pResult);
}

// 1.14d mac: 0000b487
void BigBuffer_Shr(BigBuffer* pResult, BigBuffer* pSrc, uint32_t nShiftBits) {
    uint32_t nWordShift = nShiftBits >> 5;
    uint32_t nSrcCount = pSrc->nCount;
    int nSrcBase = pSrc->nStartWord;
    uint32_t nIndex = nSrcBase + nWordShift;
    int i = 0;
    if (nIndex < nSrcCount) {
        uint8_t byShift = nShiftBits & 0x1f;
        do {
            uint32_t dwWord = pSrc->pData[nIndex] >> byShift;
            if ((nShiftBits & 0x1f) != 0) {
                uint32_t dwUpper = 0;
                if (i + 1 + nSrcBase + nWordShift < nSrcCount) {
                    dwUpper = pSrc->pData[nSrcBase + nWordShift + i + 1];
                }
                dwWord = (dwUpper << ((0x20U - byShift) & 0x1f)) + dwWord;
            }
            TSBaseArray_Ui_Copy(pResult, pResult->nStartWord + i, (uint32_t*)1);
            pResult->pData[pResult->nStartWord + i] = dwWord;
            nSrcCount = pSrc->nCount;
            nSrcBase = pSrc->nStartWord;
            nIndex = i + 1 + nWordShift + nSrcBase;
            i++;
        } while ((uint32_t)(i + nWordShift + nSrcBase) < nSrcCount);
    }
    SBIG_GrowToIndexNoZero(pResult, i);
}

// 1.14d mac: 0000b555
// Squaring: each cross product is added twice, the diagonal once.
void BigBuffer_Square(BigBuffer* pResult, BigBuffer* pInput, BigStack* pStack) {
    BigBuffer* pArray = pResult;
    if (pResult == pInput) {
        pArray = &pStack->atBuffers[pStack->nDepth++];
    }
    BigBuffer_SetZero(pArray);
    uint32_t nCount = pInput->nCount;
    uint32_t nStart = pInput->nStartWord;
    if (nStart < nCount) {
        uint32_t iOuter = 0;
        uint32_t nCarryHigh;
        do {
            uint64_t nCarry = 0;
            uint32_t iInner = 0;
            while (true) {
                uint32_t nWordOuter = 0;
                if (nStart + iOuter < nCount) {
                    nWordOuter = pInput->pData[nStart + iOuter];
                }
                uint32_t nWordInner = 0;
                if (nStart + iInner < nCount) {
                    nWordInner = pInput->pData[nStart + iInner];
                }
                int iDest = iOuter + iInner;
                uint64_t nProduct = (uint64_t)nWordInner * (uint64_t)nWordOuter;
                TSBaseArray_Ui_Copy(pArray, pArray->nStartWord + iDest, (uint32_t*)1);
                uint64_t nProductPlusOld = nProduct + (uint64_t)pArray->pData[pArray->nStartWord + iDest];
                if (iOuter <= iInner) {
                    nProduct = 0;
                }
                uint64_t nAddend = nProduct + nCarry;
                TSBaseArray_Ui_Copy(pArray, pArray->nStartWord + iDest, (uint32_t*)1);
                uint64_t nSum = nAddend + nProductPlusOld;
                pArray->pData[iDest + pArray->nStartWord] = (uint32_t)nSum;
                nCarryHigh = (uint32_t)(nSum >> 32);
                bool bOverflow = nSum < nAddend;
                nCarry = (uint64_t)nCarryHigh | ((uint64_t)bOverflow << 32);
                if (iOuter < iInner + 1) {
                    break;
                }
                nCount = pInput->nCount;
                nStart = pInput->nStartWord;
                iInner++;
            }
            TSBaseArray_Ui_Copy(pArray, iInner + 1 + pArray->nStartWord + iOuter, (uint32_t*)1);
            pArray->pData[pArray->nStartWord + iOuter + 1 + iInner] = nCarryHigh;
            nCount = pInput->nCount;
            nStart = pInput->nStartWord;
            iOuter++;
        } while (nStart + iOuter < nCount);
    }
    SBIG_BigBuffer_GetWord(pStack, pResult, pArray);
}

// 1.14d mac: 0000b731
void SBigSub(BigData* pResult, BigData* pBigA, BigData* pBigB) {
    BigBuffer_Sub(&pResult->tResult, &pBigA->tResult, &pBigB->tResult);
}

// 1.14d mac: 0000b74d
void BigBuffer_Sub(BigBuffer* pResult, BigBuffer* pLhs, BigBuffer* pRhs) {
    uint32_t nBorrowLo = 0;
    int32_t nBorrowHi = 0;
    int i = 0;
    while (true) {
        uint32_t nIndexLhs = pLhs->nStartWord + i;
        uint32_t nWordLhs;
        if (nIndexLhs < pLhs->nCount) {
            nWordLhs = pLhs->pData[nIndexLhs];
        } else {
            nWordLhs = 0;
            if (pRhs->nCount <= pRhs->nStartWord + i) {
                SBIG_GrowToIndexNoZero(pResult, i);
                return;
            }
        }
        uint32_t nIndexRhs = pRhs->nStartWord + i;
        uint32_t nWordRhs = 0;
        if (nIndexRhs < pRhs->nCount) {
            nWordRhs = pRhs->pData[nIndexRhs];
        }
        TSBaseArray_Ui_Copy(pResult, pResult->nStartWord + i, (uint32_t*)1);
        pResult->pData[pResult->nStartWord + i] = (nWordLhs + nBorrowLo) - nWordRhs;
        nBorrowLo = (nBorrowHi + (uint32_t)((nWordLhs + nBorrowLo) < nWordLhs)) - (uint32_t)(nWordLhs + nBorrowLo < nWordRhs);
        nBorrowHi = (int32_t)nBorrowLo >> 31;
        i++;
    }
}

// 1.14d mac: 0000b819
void SBigToBinaryBuffer(BigData* pBigNum, void* pDest, uint32_t nMaxBytes, size_t* pnSize) {
    pBigNum->tBytes.nCount = 0;
    SBIG_ToBinaryAppend(&pBigNum->tBytes, &pBigNum->tResult);
    if (pBigNum->tBytes.nCount < nMaxBytes) {
        nMaxBytes = pBigNum->tBytes.nCount;
    }
    if (0 < (int)nMaxBytes) {
        memmove(pDest, pBigNum->tBytes.pData, nMaxBytes);
    }
    if (pnSize != NULL) {
        *pnSize = nMaxBytes;
    }
}

void TSGrowableArray_Uc_Reserve(ByteArray* pArray, int nCount, int bRound);

// 1.14d mac: 0000b878
// The bytes of the number, least significant first; zero bytes of the top word are not
// appended.
void SBIG_ToBinaryAppend(ByteArray* pBytes, BigBuffer* pBig) {
    uint32_t nCount = pBig->nCount;
    int nStart = pBig->nStartWord;
    if (((nCount - nStart) & 0x3fffffff) != 0) {
        uint8_t nBitShift = 0;
        uint32_t nByte = 0;
        do {
            uint32_t nIndex = nStart + (nByte >> 2);
            uint32_t dwWord = 0;
            if (nIndex < nCount) {
                dwWord = pBig->pData[nIndex];
            }
            dwWord = dwWord >> (nBitShift & 0x18);
            if (dwWord != 0 || (nByte >> 2) + 1 < nCount - nStart) {
                TSGrowableArray_Uc_Reserve(pBytes, 1, 1);
                uint32_t nUsed = pBytes->nCount;
                pBytes->nCount = nUsed + 1;
                pBytes->pData[nUsed] = (uint8_t)dwWord;
                nCount = pBig->nCount;
                nStart = pBig->nStartWord;
            }
            nBitShift += 8;
            nByte++;
        } while (nByte < (nCount - nStart) * 4);
    }
}

// 1.14d mac: 0000b92f
// pQuotient = pDividend / divisor, the remainder to *pnRemainder; the divisor is 64 bits.
void BigBuffer_Div_Ux(uint32_t* pnRemainder, BigBuffer* pQuotient, BigBuffer* pDividend, uint32_t nDivisorLo,
                             uint32_t nDivisorHi) {
    SBIG_GrowToIndexNoZero(pQuotient, pDividend->nCount - pDividend->nStartWord);
    uint32_t nCount = pDividend->nCount;
    int nStart = pDividend->nStartWord;
    uint32_t nRemainder = 0;
    int iWord = nCount - nStart;
    uint64_t nDivisor = ((uint64_t)nDivisorHi << 32) | nDivisorLo;
    if (iWord != 0) {
        while (true) {
            uint32_t nWord = 0;
            if ((uint32_t)(nStart - 1 + iWord) < nCount) {
                nWord = pDividend->pData[nStart + iWord - 1];
            }
            TSBaseArray_Ui_Copy(pQuotient, iWord - 1 + pQuotient->nStartWord, (uint32_t*)1);
            uint64_t nNumerator = ((uint64_t)nRemainder << 32) | nWord;
            pQuotient->pData[pQuotient->nStartWord + iWord - 1] = (uint32_t)(nNumerator / nDivisor);
            nRemainder = (uint32_t)(nNumerator % nDivisor);
            if (iWord == 1) {
                break;
            }
            iWord--;
            nCount = pDividend->nCount;
            nStart = pDividend->nStartWord;
        }
    }
    SBIG_TrimLeadingZeros(pQuotient);
    *pnRemainder = nRemainder;
}

// 1.14d mac: 0000ba3f
void BigBuffer_Mul_Ux(BigBuffer* pDest, BigBuffer* pSource, uint32_t nMultiplierLo, uint32_t nMultiplierHi) {
    uint32_t nCarry = 0;
    int i = 0;
    while (true) {
        if (nCarry == 0 && pSource->nCount <= i + pSource->nStartWord) {
            SBIG_GrowToIndexNoZero(pDest, i);
            return;
        }
        uint32_t nIndex = pSource->nStartWord + i;
        uint32_t nWord = 0;
        if (nIndex < pSource->nCount) {
            nWord = pSource->pData[nIndex];
        }
        TSBaseArray_Ui_Copy(pDest, pDest->nStartWord + i, (uint32_t*)1);
        uint64_t nProduct = (uint64_t)nWord * (uint64_t)nMultiplierLo + (((uint64_t)(nWord * nMultiplierHi) << 32) | nCarry);
        pDest->pData[pDest->nStartWord + i] = (uint32_t)nProduct;
        nCarry = (uint32_t)(nProduct >> 32);
        i++;
    }
}

void TSBaseArray_Uc_ReallocData(ByteArray* pArray, uint32_t nNewMax);

// 1.14d mac: 0000bb08
// Room for nCount more bytes; the chunk is a power of two up to 0x100.
void TSGrowableArray_Uc_Reserve(ByteArray* pArray, int nCount, int bRound) {
    uint32_t nNewMax = pArray->nCount + nCount;
    if (pArray->nCapacity < nNewMax) {
        if (bRound != 0) {
            uint32_t nGranularity = pArray->nGrowStep;
            if (pArray->nGrowStep == 0) {
                nGranularity = nNewMax;
                if (nNewMax < 0x100) {
                    uint32_t nPrevious;
                    do {
                        nPrevious = nGranularity;
                        nGranularity = nPrevious - 1 & nPrevious;
                    } while (nGranularity != 0);
                    nGranularity = 1;
                    if (1 < nPrevious) {
                        nGranularity = nPrevious;
                    }
                } else {
                    pArray->nGrowStep = 0x100;
                    nGranularity = 0x100;
                }
            }
            if (nNewMax % nGranularity != 0) {
                nNewMax = (nGranularity + nNewMax) - nNewMax % nGranularity;
            }
        }
        TSBaseArray_Uc_ReallocData(pArray, nNewMax);
    }
}

// 1.14d mac: 0000bb7c
void TSBaseArray_Uc_ReallocData(ByteArray* pArray, uint32_t nNewMax) {
    uint8_t* pBlock = pArray->pData;
    pArray->nCapacity = nNewMax;
    pArray->pData = (uint8_t*)SMemReAlloc(pBlock, nNewMax, "nothing", -2, 0x10);
    if (pArray->pData == NULL) {
        pArray->pData = (uint8_t*)SMemAlloc(nNewMax, "nothing", -2, 0);
        if (pBlock != NULL) {
            uint32_t nKeep = pArray->nCount;
            if (nNewMax < nKeep) {
                nKeep = nNewMax;
            }
            for (uint32_t i = 0; i < nKeep; i++) {
                pArray->pData[i] = pBlock[i];
            }
            SMemFree(pBlock, "nothing", -2, 0);
        }
    }
}

void TSGrowableArray_Ui_Reserve(BigBuffer* pArray, int nCount, int bRound);

// 1.14d mac: 0000bc46
void SBIG_GrowToIndexNoZero(BigBuffer* pBuffer, int32_t nIndex) {
    uint32_t nNewCount = pBuffer->nStartWord + nIndex;
    if (pBuffer->nCount < nNewCount) {
        TSGrowableArray_Ui_Reserve(pBuffer, nNewCount - pBuffer->nCount, 1);
    }
    pBuffer->nCount = nNewCount;
}

void TSBaseArray_Ui_ReallocData(BigBuffer* pArray, uint32_t nNewMax);

// 1.14d mac: 0000bc80
// The chunk is a power of two up to 0x40.
void TSGrowableArray_Ui_Reserve(BigBuffer* pArray, int nCount, int bRound) {
    uint32_t nNewMax = pArray->nCount + nCount;
    if (pArray->nCapacity < nNewMax) {
        if (bRound != 0) {
            uint32_t nGranularity = pArray->nGrowStep;
            if (pArray->nGrowStep == 0) {
                nGranularity = nNewMax;
                if (nNewMax < 0x40) {
                    uint32_t nPrevious;
                    do {
                        nPrevious = nGranularity;
                        nGranularity = nPrevious - 1 & nPrevious;
                    } while (nGranularity != 0);
                    nGranularity = 1;
                    if (1 < nPrevious) {
                        nGranularity = nPrevious;
                    }
                } else {
                    pArray->nGrowStep = 0x40;
                    nGranularity = 0x40;
                }
            }
            if (nNewMax % nGranularity != 0) {
                nNewMax = (nGranularity + nNewMax) - nNewMax % nGranularity;
            }
        }
        TSBaseArray_Ui_ReallocData(pArray, nNewMax);
    }
}

// 1.14d mac: 0000bcf2
void TSBaseArray_Ui_ReallocData(BigBuffer* pArray, uint32_t nNewMax) {
    uint32_t* pBlock = pArray->pData;
    pArray->nCapacity = nNewMax;
    pArray->pData = (uint32_t*)SMemReAlloc(pBlock, nNewMax * sizeof(uint32_t), "nothing", -2, 0x10);
    if (pArray->pData == NULL) {
        uint32_t* pNew = (uint32_t*)SMemAlloc(nNewMax * sizeof(uint32_t), "nothing", -2, 0);
        pArray->pData = pNew;
        if (pBlock != NULL) {
            uint32_t nKeep = pArray->nCount;
            if (nNewMax < nKeep) {
                nKeep = nNewMax;
            }
            for (uint32_t i = 0; i < nKeep; i++) {
                pNew[i] = pBlock[i];
            }
            SMemFree(pBlock, "nothing", -2, 0);
        }
    }
}

// 1.14d mac: 0000bdc8
// Makes nCount a valid index: grows the count to nCount + 1; a non-null pSrc zeroes the new words.
void TSBaseArray_Ui_Copy(BigBuffer* pArray, uint32_t nCount, uint32_t* pSrc) {
    if (pArray->nCount <= nCount) {
        uint32_t nNewCount = nCount + 1;
        TSGrowableArray_Ui_Reserve(pArray, nNewCount - pArray->nCount, 1);
        if (pSrc != NULL) {
            size_t n = (nNewCount - pArray->nCount) * 4;
            if (0 < (int)n) {
                bzero(pArray->pData + pArray->nCount, n);
            }
        }
        pArray->nCount = nNewCount;
    }
}

// 1.14d mac: 0000be34
// Drops the zero words at the top.
void SBIG_TrimLeadingZeros(BigBuffer* pBuffer) {
    uint32_t nCount = pBuffer->nCount;
    uint32_t nStart = pBuffer->nStartWord;
    while (nCount != nStart && pBuffer->pData[nCount - 1] == 0) {
        bool bWrapped = nCount == 0;
        nCount--;
        if (bWrapped) {
            TSGrowableArray_Ui_Reserve(pBuffer, -1, 1);
            nStart = pBuffer->nStartWord;
        }
        pBuffer->nCount = nCount;
    }
}

// 1.14d mac: 0000be84
// The number is zero: no words past the start.
void BigBuffer_SetZero(BigBuffer* pBuffer) {
    uint32_t nStart = pBuffer->nStartWord;
    if (pBuffer->nCount < nStart) {
        TSGrowableArray_Ui_Reserve(pBuffer, nStart - pBuffer->nCount, 1);
    }
    pBuffer->nCount = nStart;
}

void TSFixedArray_Ui_ReallocData(BigBuffer* pArray, int nCount);

// 1.14d mac: 0000bebc
// Takes the scratch buffer's number into the destination and pops the stack, unless the
// destination is the scratch buffer itself.
void SBIG_BigBuffer_GetWord(BigStack* pStack, BigBuffer* pDest, BigBuffer* pSource) {
    if (pDest != pSource) {
        TSFixedArray_Ui_Set(pDest, pSource->nCount, pSource->pData);
        pDest->nGrowStep = pSource->nGrowStep;
        pDest->nStartWord = pSource->nStartWord;
        pStack->nDepth = pStack->nDepth - 1;
    }
}

// 1.14d mac: 0000bf02
void TSFixedArray_Ui_Set(BigBuffer* pArray, int nCount, const uint32_t* pSrc) {
    TSFixedArray_Ui_ReallocData(pArray, nCount);
    if (nCount != 0) {
        uint32_t* pDest = pArray->pData;
        for (int nRemaining = nCount; nRemaining != 0; nRemaining--) {
            if (pDest != NULL) {
                *pDest = *pSrc;
            }
            pSrc++;
            pDest++;
        }
    }
    pArray->nCount = nCount;
}

// 1.14d mac: 0000bf46
void TSFixedArray_Ui_ReallocData(BigBuffer* pArray, int nCount) {
    pArray->nCapacity = nCount;
    if (pArray->pData != NULL || nCount != 0) {
        pArray->pData = (uint32_t*)SMemReAlloc(pArray->pData, nCount * sizeof(uint32_t), "nothing", -2, 0);
    }
}

#endif  // D2_OS_MAC
