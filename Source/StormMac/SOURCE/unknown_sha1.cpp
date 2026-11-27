// StormMac Storm Mac/SOURCE: the SHA-1 class, 1.14d Intel Mac 00009d2e..0000a56e
// (Sha1::Pump, Initialize, Append, Finalize).
//
// The original file name is unknown: no __FILE__ names it. The PPC Storm 1.10 Beta
// exports Sha1::Pump/Append/Finalize as members of a class of their own; 1.14d lays them
// out between STORM.CPP's last function and SBig.cpp's first, so they are kept as a unit
// of their own (see Source/StormMac/README.md).
#include "../Win32Mac/Source/Win32/Win32Mac.h"

#if !defined(_WIN32) && D2_OS_MAC

#include <string.h>

// 1.14d mac: 00397084
// The padding block: 0x80, then zeros. Defined here: Finalize addresses it directly (lea),
// not through a non-lazy pointer as it would an object of another unit.
static const uint8_t g_abySha1Padding[64] = {0x80};

// The context: the bit count (low, high), the five state words, the 64-byte block buffer.
struct Sha1 {
    uint64_t qwBitCount;      // 0x00
    uint32_t aState[5];       // 0x08
    uint8_t abyBlock[64];     // 0x1C
};

static inline uint32_t Rol(uint32_t dwValue, int nBits) {
    return dwValue << nBits | dwValue >> (32 - nBits);
}

// 1.14d mac: 00009d2e
// One 512-bit block into the state. External (a class member in the PPC build): the original
// calls it with its arguments on the stack, not in ecx/edx as a file-local function.
void Sha1_Pump(uint32_t* pState, const uint8_t* pBlock) {
    uint32_t aMsgSchedule[80];
    for (int i = 0; i < 16; i++) {
        aMsgSchedule[i] = (uint32_t)pBlock[i * 4 + 3] | (uint32_t)pBlock[i * 4 + 2] << 8 |
                          (uint32_t)pBlock[i * 4 + 1] << 16 | (uint32_t)pBlock[i * 4] << 24;
    }
    for (int i = 0; i < 64; i++) {
        aMsgSchedule[i + 16] = Rol(aMsgSchedule[i + 8] ^ aMsgSchedule[i + 13] ^ aMsgSchedule[i + 2] ^ aMsgSchedule[i], 1);
    }
    uint32_t a = pState[0];
    uint32_t b = pState[1];
    uint32_t c = pState[2];
    uint32_t d = pState[3];
    uint32_t e = pState[4];
    for (int i = 0; i < 80; i++) {
        uint32_t dwFunction;
        uint32_t dwConstant;
        if (i < 20) {
            dwFunction = (~b & d) | (b & c);
            dwConstant = 0x5a827999;
        } else if (i < 40) {
            dwFunction = b ^ c ^ d;
            dwConstant = 0x6ed9eba1;
        } else if (i < 60) {
            dwFunction = ((c | d) & b) | (c & d);
            dwConstant = 0x8f1bbcdc;
        } else {
            dwFunction = b ^ c ^ d;
            dwConstant = 0xca62c1d6;
        }
        uint32_t dwTemp = aMsgSchedule[i] + dwConstant + Rol(a, 5) + dwFunction + e;
        aMsgSchedule[i] = 0;
        e = d;
        d = c;
        c = Rol(b, 30);
        b = a;
        a = dwTemp;
    }
    pState[0] += a;
    pState[1] += b;
    pState[2] += c;
    pState[3] += d;
    pState[4] += e;
}

// 1.14d mac: 0000a480
void Sha1_Initialize(Sha1* pContext) {
    pContext->qwBitCount = 0;
    pContext->aState[0] = 0x67452301;
    pContext->aState[1] = 0xefcdab89;
    pContext->aState[2] = 0x98badcfe;
    pContext->aState[3] = 0x10325476;
    pContext->aState[4] = 0xc3d2e1f0;
}

// 1.14d mac: 0000a4b8
void Sha1_Append(Sha1* pContext, const uint8_t* pData, uint32_t nSize) {
    uint32_t i = (uint32_t)(pContext->qwBitCount >> 3) & 0x3f;
    pContext->qwBitCount += (uint64_t)nSize << 3;
    if (i != 0) {
        nSize += i;
        pData -= i;
        if (nSize >= 0x40) {
            for (; i < 0x40; i++) {
                pContext->abyBlock[i] = pData[i];
            }
            Sha1_Pump(pContext->aState, pContext->abyBlock);
            pData += sizeof(pContext->abyBlock);
            nSize -= 0x40;
            i = 0;
        }
    }
    while (nSize >= 0x40) {
        Sha1_Pump(pContext->aState, pData);
        pData += sizeof(pContext->abyBlock);
        nSize -= 0x40;
    }
    for (; i < nSize; i++) {
        pContext->abyBlock[i] = pData[i];
    }
}

// 1.14d mac: 0000a56e
void Sha1_Finalize(Sha1* pContext, uint8_t* pDigest) {
    uint8_t abyLength[8];
    uint32_t dwLow = (uint32_t)pContext->qwBitCount;
    uint32_t dwHigh = (uint32_t)(pContext->qwBitCount >> 32);
    uint32_t dwShifted = dwLow;
    for (int i = 8; i > 0; i--) {
        abyLength[i - 1] = (uint8_t)dwShifted;
        dwShifted = dwShifted >> 8 | dwHigh << 24;
        dwHigh >>= 8;
    }
    Sha1_Append(pContext, g_abySha1Padding, (0x37 - (dwLow >> 3) & 0x3f) + 1);
    Sha1_Append(pContext, abyLength, 8);
    for (int nWord = 0; nWord != 5; nWord++) {
        uint32_t dwWord = pContext->aState[nWord];
        for (int nByte = 3; nByte >= 0; nByte--) {
            pDigest[nWord * 4 + nByte] = (uint8_t)dwWord;
            dwWord >>= 8;
        }
    }
}

#endif  // D2_OS_MAC
