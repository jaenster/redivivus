// Storm Mac: SSignature.cpp, named by the __FILE__ literal of the verify stream's data buffer
// (1.14d Intel Mac 0001d458, SMemFree line 0xd4). The streaming RSA/SHA-1 signature verifier:
// begin, feed data, finish. 1.14d Mac band 0001d302..0001d5f8. Windows'
// Source/Storm/Source/SSignature.cpp is the same module.
//
// The stream keeps the last nBufferSize bytes (the "SIGN" magic and the signature) in a window
// and hashes everything that falls out of it.
#include "Source/StormMac/H/stpl.h"

#if !defined(_WIN32) && D2_OS_MAC

#include <alloca.h>
#include <string.h>

class Sha1;
struct BigData;

// The verify stream's context. Its first fields are Windows' D2SsigContextStrc's.
struct SsigContext {
    uint32_t nModulusLen;     // 0x00: RSA modulus length in bytes, also the signature length
    uint32_t nExponentLen;    // 0x04
    uint32_t nBytesBuffered;  // 0x08: bytes in pDataBuffer
    uint32_t nBufferSize;     // 0x0C: nModulusLen + 4, the "SIGN" magic and the signature
    uint8_t* pDataBuffer;     // 0x10
    uint32_t aSha1State[23];  // 0x14: the Sha1 object
};

// 1.14d mac: 00009632
void* operator new(size_t nBytes);
void Sha1_Initialize(uint32_t* pSha1);
void Sha1_Append(uint32_t* pSha1, const void* pData, uint32_t nBytes);
void Sha1_Finalize(uint32_t* pSha1, void* pDigest);
void SBigNew(BigData** ppBig);
void SBigDel(BigData* pBig);
void CryptRSA_Prepare(BigData** pRsa, void* pModulus, uint32_t nModulusLen, void* pExponent, uint32_t nExponentLen);
void CryptRSA_Process(BigData** pRsa, void* pData, uint32_t nLen);

// 1.14d mac: 0001d302
void SSignatureVerifyStream_Begin(SsigContext** ppContext, uint32_t nModulusLen, uint32_t nExponentLen) {
    SsigContext* pContext = (SsigContext*)operator new(sizeof(SsigContext));
    *ppContext = pContext;
    pContext->nModulusLen = nModulusLen;
    pContext->nExponentLen = nExponentLen;
    pContext->nBytesBuffered = 0;
    pContext->nBufferSize = nModulusLen + 4;
    pContext->pDataBuffer = (uint8_t*)SMemAlloc(pContext->nBufferSize, __FILE__, 0x57, 8);
    Sha1_Initialize(pContext->aSha1State);
}

// 1.14d mac: 0001d374
uint32_t SSignatureVerifyStream_GetSignatureLength(SsigContext* pContext) {
    return pContext->nBufferSize;
}

// 1.14d mac: 0001d37f
void SSignatureVerifyStream_ProvideData(SsigContext* pContext, uint8_t* pData, size_t nDataLen) {
    uint32_t nBuffered = pContext->nBytesBuffered;
    int nRemaining = nDataLen - pContext->nBufferSize;
    size_t nNewBuffered;
    if (nRemaining < 0) {
        int nFlush = nRemaining + nBuffered;
        if (0 < nFlush) {
            Sha1_Append(pContext->aSha1State, pContext->pDataBuffer, nFlush);
            nNewBuffered = pContext->nBytesBuffered - nFlush;
            pContext->nBytesBuffered = nNewBuffered;
            if (0 < (int)nNewBuffered) {
                memmove(pContext->pDataBuffer, pContext->pDataBuffer + nFlush, nNewBuffered);
            }
        }
        if (0 < (int)nDataLen) {
            memmove(pContext->pDataBuffer + pContext->nBytesBuffered, pData, nDataLen);
        }
        nNewBuffered = nDataLen + pContext->nBytesBuffered;
    } else {
        if (nBuffered != 0) {
            Sha1_Append(pContext->aSha1State, pContext->pDataBuffer, nBuffered);
        }
        if (0 < nRemaining) {
            Sha1_Append(pContext->aSha1State, pData, nRemaining);
        }
        nNewBuffered = pContext->nBufferSize;
        if (0 < (int)nNewBuffered) {
            memmove(pContext->pDataBuffer, pData + nRemaining, nNewBuffered);
            nNewBuffered = pContext->nBufferSize;
        }
    }
    pContext->nBytesBuffered = nNewBuffered;
}

// 1.14d mac: 0001d458
// Verifies the signature in the window against the SHA-1 of the data that left it, then frees
// the buffer and the context.
int SSignatureVerifyStream_Finish(SsigContext* pContext, void* pModulus, void* pExponent) {
    int bVerified = 0;
    if (pContext->nBytesBuffered == pContext->nBufferSize) {
        if (*(int*)pContext->pDataBuffer == 0x5349474e) {
            uint32_t nSigLen = pContext->nModulusLen;
            BigData* aRsa[2];
            uint8_t* pHash = (uint8_t*)alloca(nSigLen);
            memset(pHash, 0xbb, nSigLen);
            pHash[nSigLen - 1] = 0x0b;
            Sha1_Finalize(pContext->aSha1State, pHash);
            uint8_t* pSig = (uint8_t*)alloca(pContext->nModulusLen);
            if (0 < (int)pContext->nModulusLen) {
                memmove(pSig, pContext->pDataBuffer + 4, pContext->nModulusLen);
            }
            SBigNew(&aRsa[0]);
            SBigNew(&aRsa[1]);
            CryptRSA_Prepare(aRsa, pModulus, pContext->nModulusLen, pExponent, pContext->nExponentLen);
            CryptRSA_Process(aRsa, pSig, pContext->nModulusLen);
            int nCmp = memcmp(pSig, pHash, pContext->nModulusLen);
            SBigDel(aRsa[1]);
            SBigDel(aRsa[0]);
            bVerified = nCmp == 0;
        }
        SMemFree(pContext->pDataBuffer, __FILE__, 0xd4, 0);
        if (pContext == NULL) {
            return bVerified;
        }
    } else {
        SMemFree(pContext->pDataBuffer, __FILE__, 0xd4, 0);
    }
    operator delete(pContext);
    return bVerified;
}

#endif  // D2_OS_MAC
