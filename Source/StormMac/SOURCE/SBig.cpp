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
#include <new>

void* SMemAlloc(uint32_t nBytes, const char* szFile, int nLine, uint32_t dwFlags);
int SMemFree(void* pMem, const char* szFile, int nLine, uint32_t dwFlags);
void* SMemReAlloc(void* pMem, uint32_t nBytes, const char* szFile, int nLine, uint32_t dwFlags);

// File-local routines. Internal linkage is in the binary: they take their first two arguments in
// ecx/edx (LLVM's fastcc, given only to internal functions) and each sits right after its first
// caller (clang defers a static definition to its first use). Yet they stay out of line with one
// caller, which plain `static` at -Os does not do (tried: seven of them inline); the source-level
// cause is not recoverable, so noinline stands for it.
#define SBIG_LOCAL static __attribute__((noinline))
#define SBIG_FILE "/Users/bclemetson/dev/diablo2/Diablo2/Source/DiabloAll/../../../StormMac/Storm Mac/SOURCE/SBig.cpp"

struct BigBuffer {
    uint32_t nCapacity;   // 0x00  m_alloc
    uint32_t nCount;      // 0x04  m_count
    uint32_t* pData;      // 0x08  m_data
    uint32_t nGrowStep;   // 0x0C  m_chunk
    uint32_t nStartWord;  // 0x10  m_offset: the number's lowest word

    BigBuffer() : nCapacity(0), nCount(0), pData(NULL), nGrowStep(0), nStartWord(0) {}
    ~BigBuffer() {
        if (pData != NULL) {
            SMemFree(pData, "nothing", -2, 0);
        }
    }
};

struct ByteArray {
    uint32_t nCapacity;
    uint32_t nCount;
    uint8_t* pData;
    uint32_t nGrowStep;

    ByteArray() : nCapacity(0), nCount(0), pData(NULL), nGrowStep(0) {}
    ~ByteArray() {
        if (pData != NULL) {
            SMemFree(pData, "nothing", -2, 0);
        }
    }
};

struct BigStack {
    BigBuffer atBuffers[16];
    int32_t nDepth;  // 0x140: buffers in use

    BigStack() : nDepth(0) {}
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
void TSFixedArray_Ui_Set(BigBuffer* pArray, uint32_t nCount, const uint32_t* pSrc);
void SBIG_BigBuffer_GetWord(BigStack* pStack, BigBuffer* pDest, BigBuffer* pSource);
// The buffer's element access, inlined everywhere: word i above the start word. Reading a word
// past the count gives 0; writing one first grows the array to hold it.
static inline bool BigBuffer_IsUsed(const BigBuffer* pBuffer, uint32_t i) {
    return i + pBuffer->nStartWord < pBuffer->nCount;
}

static inline uint32_t BigBuffer_Get(const BigBuffer* pBuffer, uint32_t i) {
    return BigBuffer_IsUsed(pBuffer, i) ? pBuffer->pData[pBuffer->nStartWord + i] : 0;
}

// Moves the start word, making sure the words below it exist.
static inline void BigBuffer_SetOffset(BigBuffer* pBuffer, uint32_t nOffset) {
    pBuffer->nStartWord = nOffset;
    if (nOffset != 0) {
        TSBaseArray_Ui_Copy(pBuffer, nOffset - 1, (uint32_t*)1);
    }
}

static inline uint32_t& BigBuffer_At(BigBuffer* pBuffer, uint32_t i) {
    TSBaseArray_Ui_Copy(pBuffer, pBuffer->nStartWord + i, (uint32_t*)1);
    return pBuffer->pData[pBuffer->nStartWord + i];
}

SBIG_LOCAL void BigBuffer_Add(BigBuffer* pDest, BigBuffer* pSourceA, BigBuffer* pSourceB);
SBIG_LOCAL int32_t BigBuffer_Compare(BigBuffer* pA, BigBuffer* pB);
SBIG_LOCAL void BigBuffer_Div(BigBuffer* pQuotient, BigBuffer* pRemainder, BigBuffer* pDividend, BigBuffer* pDivisor,
                          BigStack* pStack);
SBIG_LOCAL void BigBuffer_FromBinary(BigBuffer* pResult, const void* pSrcBytes, uint32_t nByteCount);
SBIG_LOCAL void BigBuffer_Mul(BigBuffer* pResult, BigBuffer* pA, BigBuffer* pB, BigStack* pStack);
SBIG_LOCAL void BigBuffer_MulMod(BigBuffer* pResult, BigBuffer* pA, BigBuffer* pB, BigBuffer* pModulus,
                             BigStack* pStack);
SBIG_LOCAL void BigBuffer_PowMod(BigBuffer* pResult, BigBuffer* pBase, BigBuffer* pExponent, BigBuffer* pModulus,
                             BigStack* pStack);
SBIG_LOCAL void BigBuffer_Shl(BigBuffer* pResult, BigBuffer* pSource, uint32_t nShiftBits);
SBIG_LOCAL void BigBuffer_Shr(BigBuffer* pResult, BigBuffer* pSource, uint32_t nShiftBits);
SBIG_LOCAL void BigBuffer_Square(BigBuffer* pResult, BigBuffer* pInput, BigStack* pStack);
SBIG_LOCAL void BigBuffer_Sub(BigBuffer* pResult, BigBuffer* pLhs, BigBuffer* pRhs);
SBIG_LOCAL void BigBuffer_Div_Ux(BigBuffer* pQuotient, uint32_t* pnRemainder, BigBuffer* pDividend, uint64_t nDivisor);
SBIG_LOCAL void BigBuffer_Mul_Ux(BigBuffer* pDest, BigBuffer* pSource, uint64_t nMultiplier);
SBIG_LOCAL void SBIG_ToBinaryAppend(ByteArray* pBytes, BigBuffer* pBig);

// ---- the exports ----

// 1.14d mac: 0000a629
void SBigAdd(BigData* pResult, BigData* pBigA, BigData* pBigB) {
    BigBuffer_Add(&pResult->tResult, &pBigA->tResult, &pBigB->tResult);
}

// 1.14d mac: 0000a645
SBIG_LOCAL void BigBuffer_Add(BigBuffer* pDest, BigBuffer* pSourceA, BigBuffer* pSourceB) {
    uint64_t nCarry = 0;
    uint32_t i;
    for (i = 0; nCarry != 0 || BigBuffer_IsUsed(pSourceA, i) || BigBuffer_IsUsed(pSourceB, i); i++) {
        nCarry = nCarry + BigBuffer_Get(pSourceA, i) + BigBuffer_Get(pSourceB, i);
        BigBuffer_At(pDest, i) = (uint32_t)nCarry;
        nCarry >>= 32;
    }
    SBIG_GrowToIndexNoZero(pDest, i);
}

// 1.14d mac: 0000a71d
// Only the SHA-1 code calls it (a forwarder onto BigBuffer_Compare).
int32_t SBIG_CompareBigBuffers_ForSha1(BigData* pBigA, BigData* pBigB) {
    return BigBuffer_Compare(&pBigA->tResult, &pBigB->tResult);
}

// 1.14d mac: 0000a72c
SBIG_LOCAL int32_t BigBuffer_Compare(BigBuffer* pA, BigBuffer* pB) {
    int32_t nResult = 0;
    for (uint32_t i = 0; BigBuffer_IsUsed(pA, i) || BigBuffer_IsUsed(pB, i); i++) {
        if (BigBuffer_Get(pA, i) != BigBuffer_Get(pB, i)) {
            nResult = BigBuffer_Get(pA, i) > BigBuffer_Get(pB, i) ? 1 : -1;
        }
    }
    return nResult;
}

// 1.14d mac: 0000a7b6
// The array of each buffer is freed, then the object itself.
void SBigDel(BigData* pBig) {
    delete pBig;
}

// 1.14d mac: 0000a87e
// quotient and remainder of dividend / divisor; the scratch stack holds the scaled copies.
SBIG_LOCAL void BigBuffer_Div(BigBuffer* pQuotient, BigBuffer* pRemainder, BigBuffer* pDividend, BigBuffer* pDivisor,
                          BigStack* pStack) {
    SBIG_TrimLeadingZeros(pDividend);
    SBIG_TrimLeadingZeros(pDivisor);
    uint32_t nDividendLen = pDividend->nCount - pDividend->nStartWord;
    uint32_t nDivisorLen = pDivisor->nCount - pDivisor->nStartWord;
    if (nDivisorLen > nDividendLen) {
        if (pRemainder != pDividend) {
            TSFixedArray_Ui_Set(pRemainder, pDividend->nCount, pDividend->pData);
        }
        pRemainder->nGrowStep = pDividend->nGrowStep;
        pRemainder->nStartWord = pDividend->nStartWord;
        BigBuffer_SetZero(pQuotient);
    } else if (nDivisorLen <= 1) {
        BigBuffer_Div_Ux(pQuotient, &BigBuffer_At(pRemainder, 0), pDividend, BigBuffer_Get(pDivisor, 0));
        SBIG_GrowToIndexNoZero(pRemainder, 1);
    } else {
        BigBuffer* pWork = &pStack->atBuffers[pStack->nDepth++];
        BigBuffer* pScaledDivisor = &pStack->atBuffers[pStack->nDepth++];
        BigBuffer* pProduct = &pStack->atBuffers[pStack->nDepth++];
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
        BigBuffer_Shl(pWork, pDividend, nShift);
        BigBuffer_Shl(pScaledDivisor, pDivisor, nShift);
        uint32_t nTopWord = nDivisorLen - 1;
        uint32_t nEstimator = BigBuffer_At(pScaledDivisor, nTopWord) + 1;
        uint32_t nQuotientLen = nDividendLen + 1 - nDivisorLen;
        SBIG_GrowToIndexNoZero(pQuotient, nQuotientLen);
        // each quotient digit is found with both the quotient and the work buffer windowed at it
        for (uint32_t i = nQuotientLen; i-- > 0;) {
            BigBuffer_SetOffset(pQuotient, i);
            BigBuffer_SetOffset(pWork, i);
            if (nEstimator == 0) {
                BigBuffer_At(pQuotient, 0) = BigBuffer_At(pWork, nDivisorLen);
            } else {
                BigBuffer_At(pQuotient, 0) =
                    (BigBuffer_At(pWork, nTopWord) + ((uint64_t)BigBuffer_At(pWork, nDivisorLen) << 32)) / nEstimator;
            }
            if (BigBuffer_At(pQuotient, 0) != 0) {
                BigBuffer_Mul_Ux(pProduct, pScaledDivisor, BigBuffer_At(pQuotient, 0));
                BigBuffer_Sub(pWork, pWork, pProduct);
            }
            while (BigBuffer_At(pWork, nDivisorLen) != 0 || BigBuffer_Compare(pWork, pScaledDivisor) >= 0) {
                BigBuffer_At(pQuotient, 0)++;
                BigBuffer_Sub(pWork, pWork, pScaledDivisor);
            }
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
SBIG_LOCAL void BigBuffer_FromBinary(BigBuffer* pResult, const void* pSrcBytes, uint32_t nByteCount) {
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
SBIG_LOCAL void BigBuffer_Mul(BigBuffer* pResult, BigBuffer* pA, BigBuffer* pB, BigStack* pStack) {
    BigBuffer* pArray = pResult;
    if (pResult == pA || pResult == pB) {
        pArray = &pStack->atBuffers[pStack->nDepth++];
    }
    BigBuffer_SetZero(pArray);
    for (uint32_t i = 0; BigBuffer_IsUsed(pA, i); i++) {
        uint64_t nCarry = 0;
        uint32_t j;
        for (j = 0; BigBuffer_IsUsed(pB, j); j++) {
            nCarry = (uint64_t)BigBuffer_Get(pA, i) * BigBuffer_Get(pB, j) + nCarry + BigBuffer_At(pArray, i + j);
            BigBuffer_At(pArray, i + j) = (uint32_t)nCarry;
            nCarry >>= 32;
        }
        BigBuffer_At(pArray, i + j) = (uint32_t)nCarry;
    }
    SBIG_BigBuffer_GetWord(pStack, pResult, pArray);
}

// 1.14d mac: 0000afda
// result = a * b mod modulus.
SBIG_LOCAL void BigBuffer_MulMod(BigBuffer* pResult, BigBuffer* pA, BigBuffer* pB, BigBuffer* pModulus,
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
    void* pMem = SMemAlloc(sizeof(BigData), SBIG_FILE, 0x51e, 0);
    *ppBigData = pMem ? new (pMem) BigData : NULL;
}

// 1.14d mac: 0000b0fd
void SBigPowMod(BigData* pResult, BigData* pBase, BigData* pExponent, BigData* pModulus) {
    BigBuffer_PowMod(&pResult->tResult, &pBase->tResult, &pExponent->tResult, &pModulus->tResult, &pResult->tStack);
}

// 1.14d mac: 0000b127
// result = base ^ exponent mod modulus, two exponent bits at a time over the powers
// base^1 .. base^3.
SBIG_LOCAL void BigBuffer_PowMod(BigBuffer* pResult, BigBuffer* pBase, BigBuffer* pExponent, BigBuffer* pModulus,
                             BigStack* pStack) {
    SBIG_TrimLeadingZeros(pExponent);
    if (pExponent->nCount - pExponent->nStartWord == 0) {
        SBIG_GrowToIndexNoZero(pResult, 1);
        BigBuffer_At(pResult, 0) = 1;
        return;
    }
    int nDepth = pStack->nDepth;
    pStack->nDepth = nDepth + 3;
    BigBuffer* pQuotientSink = &pStack->atBuffers[nDepth];
    BigBuffer* pSquare = &pStack->atBuffers[nDepth + 1];
    BigBuffer* pCube = &pStack->atBuffers[nDepth + 2];
    BigBuffer* pAccumulator = pResult;
    if (pResult == pBase || pResult == pExponent || pResult == pModulus) {
        pAccumulator = &pStack->atBuffers[pStack->nDepth++];
    }
    BigBuffer_MulMod(pSquare, pBase, pBase, pModulus, pStack);
    BigBuffer_MulMod(pCube, pSquare, pBase, pModulus, pStack);
    BigBuffer* apPowers[3] = {pBase, pSquare, pCube};
    SBIG_GrowToIndexNoZero(pAccumulator, 1);
    BigBuffer_At(pAccumulator, 0) = 1;
    for (uint32_t i = pExponent->nCount - pExponent->nStartWord; i > 0; i--) {
        uint32_t dwExpWord = BigBuffer_Get(pExponent, i - 1);
        uint32_t nBits = 32;
        if (i == pExponent->nCount - pExponent->nStartWord) {
            while (dwExpWord < 0x40000000) {
                dwExpWord <<= 2;
                nBits -= 2;
            }
        }
        for (uint32_t j = 0; j < nBits; j += 2) {
            BigBuffer_Square(pAccumulator, pAccumulator, pStack);
            BigBuffer_Div(pQuotientSink, pAccumulator, pAccumulator, pModulus, pStack);
            BigBuffer_Square(pAccumulator, pAccumulator, pStack);
            BigBuffer_Div(pQuotientSink, pAccumulator, pAccumulator, pModulus, pStack);
            if (dwExpWord >> 30 != 0) {
                BigBuffer_MulMod(pAccumulator, pAccumulator, apPowers[(dwExpWord >> 30) - 1], pModulus, pStack);
            }
            dwExpWord <<= 2;
        }
    }
    SBIG_BigBuffer_GetWord(pStack, pResult, pAccumulator);
    pStack->nDepth = pStack->nDepth - 3;
}

// 1.14d mac: 0000b38f
SBIG_LOCAL void BigBuffer_Shl(BigBuffer* pResult, BigBuffer* pSource, uint32_t nShiftBits) {
    uint32_t nWords = nShiftBits / 32;
    uint32_t nBits = nShiftBits % 32;
    uint32_t nCount = pSource->nCount - pSource->nStartWord + nWords + 1;
    for (uint32_t i = nCount; i-- > 0;) {
        uint32_t nWord = i >= nWords ? BigBuffer_Get(pSource, i - nWords) << nBits : 0;
        if (i > nWords && nBits != 0) {
            nWord += BigBuffer_Get(pSource, i - nWords - 1) >> (32 - nBits);
        }
        BigBuffer_At(pResult, i) = nWord;
    }
    SBIG_GrowToIndexNoZero(pResult, nCount);
    SBIG_TrimLeadingZeros(pResult);
}

// 1.14d mac: 0000b487
SBIG_LOCAL void BigBuffer_Shr(BigBuffer* pResult, BigBuffer* pSrc, uint32_t nShiftBits) {
    uint32_t nWordShift = nShiftBits >> 5;
    uint32_t nSrcCount = pSrc->nCount;
    int nSrcBase = pSrc->nStartWord;
    uint32_t nIndex = nSrcBase + nWordShift;
    int i = 0;
    if (nIndex < nSrcCount) {
        uint32_t byShift = nShiftBits & 0x1f;
        do {
            uint32_t dwWord = pSrc->pData[nIndex] >> byShift;
            if ((nShiftBits & 0x1f) != 0) {
                uint32_t dwUpper = 0;
                if (i + 1 + nSrcBase + nWordShift < nSrcCount) {
                    dwUpper = pSrc->pData[nSrcBase + nWordShift + i + 1];
                }
                dwWord = (dwUpper << (0x20U - byShift)) + dwWord;
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
SBIG_LOCAL void BigBuffer_Square(BigBuffer* pResult, BigBuffer* pInput, BigStack* pStack) {
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
            uint32_t iInner = 0;
            uint64_t nCarry = 0;
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
                if (iInner >= iOuter) {
                    nProduct = 0;
                }
                uint64_t nAddend = nProduct + nCarry;
                TSBaseArray_Ui_Copy(pArray, pArray->nStartWord + iDest, (uint32_t*)1);
                uint64_t nSum = nAddend + nProductPlusOld;
                pArray->pData[iDest + pArray->nStartWord] = (uint32_t)nSum;
                nCarryHigh = (uint32_t)(nSum >> 32);
                bool bOverflow = nSum < nAddend;
                nCarry = (uint64_t)nCarryHigh | ((uint64_t)bOverflow << 32);
                if (iInner + 1 > iOuter) {
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
SBIG_LOCAL void BigBuffer_Sub(BigBuffer* pResult, BigBuffer* pLhs, BigBuffer* pRhs) {
    int64_t nBorrow = 0;
    uint32_t i;
    for (i = 0; BigBuffer_IsUsed(pLhs, i) || BigBuffer_IsUsed(pRhs, i); i++) {
        nBorrow = nBorrow + BigBuffer_Get(pLhs, i) - BigBuffer_Get(pRhs, i);
        BigBuffer_At(pResult, i) = (uint32_t)nBorrow;
        nBorrow = (int32_t)(nBorrow >> 32);
    }
    SBIG_GrowToIndexNoZero(pResult, i);
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
SBIG_LOCAL void SBIG_ToBinaryAppend(ByteArray* pBytes, BigBuffer* pBig) {
    for (uint32_t i = 0; i < (pBig->nCount - pBig->nStartWord) * 4; i++) {
        uint32_t dwValue = BigBuffer_Get(pBig, i / 4) >> (8 * i & 0x18);
        if (dwValue != 0 || i / 4 + 1 < pBig->nCount - pBig->nStartWord) {
            TSGrowableArray_Uc_Reserve(pBytes, 1, 1);
            uint8_t* pByteData = pBytes->pData;
            uint32_t nUsed = pBytes->nCount;
            pBytes->nCount = nUsed + 1;
            pByteData[nUsed] = (uint8_t)dwValue;
        }
    }
}

// 1.14d mac: 0000b92f
// pQuotient = pDividend / divisor, the remainder to *pnRemainder; the divisor is 64 bits.
SBIG_LOCAL void BigBuffer_Div_Ux(BigBuffer* pQuotient, uint32_t* pnRemainder, BigBuffer* pDividend, uint64_t nDivisor) {
    SBIG_GrowToIndexNoZero(pQuotient, pDividend->nCount - pDividend->nStartWord);
    uint64_t nData = 0;
    uint32_t i = pDividend->nCount - pDividend->nStartWord;
    while (i-- > 0) {
        nData = (nData << 32) + BigBuffer_Get(pDividend, i);
        BigBuffer_At(pQuotient, i) = (uint32_t)(nData / nDivisor);
        nData %= nDivisor;
    }
    SBIG_TrimLeadingZeros(pQuotient);
    *pnRemainder = (uint32_t)nData;
}

// 1.14d mac: 0000ba3f
SBIG_LOCAL void BigBuffer_Mul_Ux(BigBuffer* pDest, BigBuffer* pSource, uint64_t nMultiplier) {
    uint64_t nCarry = 0;
    uint32_t i;
    for (i = 0; nCarry != 0 || BigBuffer_IsUsed(pSource, i); i++) {
        nCarry = nCarry + BigBuffer_Get(pSource, i) * nMultiplier;
        BigBuffer_At(pDest, i) = (uint32_t)nCarry;
        nCarry >>= 32;
    }
    SBIG_GrowToIndexNoZero(pDest, i);
}

void TSBaseArray_Uc_ReallocData(ByteArray* pArray, uint32_t nNewMax);

// 1.14d mac: 0000bb08
// Room for nCount more bytes; the chunk is a power of two up to 0x100.
void TSGrowableArray_Uc_Reserve(ByteArray* pArray, int nCount, int bRound) {
    uint32_t nNewMax = pArray->nCount + nCount;
    if (nNewMax > pArray->nCapacity) {
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
                new (&pArray->pData[i]) uint8_t(pBlock[i]);
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
    if (nNewMax > pArray->nCapacity) {
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
        pArray->pData = (uint32_t*)SMemAlloc(nNewMax * sizeof(uint32_t), "nothing", -2, 0);
        if (pBlock != NULL) {
            uint32_t nKeep = pArray->nCount;
            if (nNewMax < nKeep) {
                nKeep = nNewMax;
            }
            for (uint32_t i = 0; i < nKeep; i++) {
                new (&pArray->pData[i]) uint32_t(pBlock[i]);
            }
            SMemFree(pBlock, "nothing", -2, 0);
        }
    }
}

// 1.14d mac: 0000bdc8
// Makes nCount a valid index: grows the count to nCount + 1; a non-null pSrc zeroes the new words.
// The new words are then constructed in place (a trivial constructor: the loop is left empty).
void TSBaseArray_Ui_Copy(BigBuffer* pArray, uint32_t nCount, uint32_t* pSrc) {
    if (nCount >= pArray->nCount) {
        uint32_t nNewCount = nCount + 1;
        TSGrowableArray_Ui_Reserve(pArray, nNewCount - pArray->nCount, 1);
        if (pSrc != NULL) {
            size_t n = (nNewCount - pArray->nCount) * 4;
            if (0 < (int)n) {
                bzero(pArray->pData + pArray->nCount, n);
            }
        }
        for (uint32_t i = pArray->nCount; i <= nCount; i++) {
            new (&pArray->pData[i]) uint32_t;
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
void TSFixedArray_Ui_Set(BigBuffer* pArray, uint32_t nCount, const uint32_t* pSrc) {
    TSFixedArray_Ui_ReallocData(pArray, nCount);
    for (uint32_t i = 0; i < nCount; i++) {
        new (&pArray->pData[i]) uint32_t(pSrc[i]);
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
