// Storm Mac: STRANS.CPP, named by the __FILE__ literal of its frees (1.14d Intel Mac
// 0001d69e). The transparency-record list (HSTRANS): its teardown and the list's
// instantiations, 0001d69e..0001d9b5 (the static initialiser at 0001d960 is this file's).
//
// 0001d9b5..0001de67 sits between this file and SVID.CPP and names neither: the UTF-8 / UTF-16
// converters and the single-byte code page expander, the same module as the Windows
// Source/Storm/Source/SSTR.cpp functions of those names. No __FILE__ literal or neighbouring
// file claims them, so they stay with the file before them (see Source/StormMac/README.md).
#include "Source/StormMac/H/stpl.h"

#if !defined(_WIN32) && D2_OS_MAC

#include "Source/StormMac/H/StormApi.h"

// 1.14d mac: 00010bb3
void SErrReportResourceLeak(const char* szMessage);

// A transparency record: its link first, then the buffer it caches. The size of the record
// is not known: the code that allocates it is not in this file's band.
class STrans : public TSLinkedNode<STrans> {
public:
    uint32_t m_dwUnknown08;  // 0x08: owned buffer (STRANS_DeleteTransRecord moves it to the cache)
    uint32_t m_dwSize;       // 0x0C: its size
};

// The list object's destructor is out of line, so the static initialiser takes its address
// directly (not through a non-lazy pointer, as it would for a weak template member).
class STransList : public TSExplicitList<STrans, 0> {
public:
    ~STransList();
};
STransList::~STransList() {}

// The list's instantiations, by the qualified name the compare tool pairs them with:
// 1.14d mac: 0001d804 STransList::~STransList[D1]
// 1.14d mac: 0001d896 TSLink<STrans>::Unlink
// 1.14d mac: 0001d8d6 TSLink<STrans>::NextLink
// 1.14d mac: 0001d90c TSExplicitList<STrans,0>::UnlinkAll
// 1.14d mac: 0001d960 _GLOBAL__I_a

// 1.14d mac: 003f0e30
STransList g_tTransList;
// 1.14d mac: 003f0e3c
void* g_pTransCacheBuffer;
// 1.14d mac: 003f0e40
uint32_t g_nTransCacheSize;
// 1.14d mac: 003f0e44
void* g_pTransScratch;

// 1.14d mac: 0001d826
// Frees a node and returns the one after it.
STrans* TSList_DeleteNode_STrans(STransList* pList, STrans* pNode) {
    STrans* pNext = pList->Next(pNode);
    pNode->~STrans();
    SMemFree(pNode, "", -2, 0);
    return pNext;
}

// 1.14d mac: 0001d69e
// Takes the record's buffer into the one-slot cache (freeing the one held), then deletes it.
int STRANS_DeleteTransRecord(STrans* pTrans) {
    uint32_t pBuffer = pTrans->m_dwUnknown08;
    if (pBuffer != 0) {
        if (g_pTransCacheBuffer != NULL) {
            SMemFree(g_pTransCacheBuffer, __FILE__, 0x35d, 0);
            pBuffer = pTrans->m_dwUnknown08;
        }
        g_pTransCacheBuffer = (void*)pBuffer;
        g_nTransCacheSize = pTrans->m_dwSize;
        pTrans->m_dwUnknown08 = 0;
        pTrans->m_dwSize = 0;
    }
    TSList_DeleteNode_STrans(&g_tTransList, pTrans);
    return 1;
}

// 1.14d mac: 0001d720
int STransDestroy(void) {
    STrans* pTrans;
    while ((pTrans = g_tTransList.Head()) != NULL) {
        SErrReportResourceLeak("HSTRANS");
        STRANS_DeleteTransRecord(pTrans);
    }
    if (g_pTransScratch != NULL) {
        SMemFree(g_pTransScratch, __FILE__, 0x36f, 0);
        g_pTransScratch = NULL;
    }
    if (g_pTransCacheBuffer != NULL) {
        SMemFree(g_pTransCacheBuffer, __FILE__, 0x373, 0);
        g_pTransCacheBuffer = NULL;
        g_nTransCacheSize = 0;
    }
    return 1;
}

// ---- the UTF converters and the code page expander (0001d9b5..0001de67) ----

// The lead byte's first-byte mark by sequence length (index 1..6).
// 1.14d mac: 00321a64
static const uint32_t gdwUTF8FirstByteMark[7] = { 0, 0, 0xC0, 0xE0, 0xF0, 0xF8, 0xFC };
// How many continuation bytes follow a lead byte; continuation bytes count 0.
// 1.14d mac: 00321a80
static const uint8_t gaUTF8ByteCountTable[256] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
    3, 3, 3, 3, 3, 3, 3, 3, 4, 4, 4, 4, 5, 5, 5, 5,
};
// 1.14d mac: 00321b80
static const int gdwUTF8ConversionTab[6] = { 0x0, 0x3080, 0xe2080, 0x3c82080, -0x5f7df80, -0x7df7df80 };
// Windows-1252 as Unicode: the Latin-1 table of the Storm code pages.
// 1.14d mac: 00321b98
uint16_t gszUnicodeLatin1_1[256] = {
    0x0000, 0x0001, 0x0002, 0x0003, 0x0004, 0x0005, 0x0006, 0x0007,
    0x0008, 0x0009, 0x000a, 0x000b, 0x000c, 0x000d, 0x000e, 0x000f,
    0x0010, 0x0011, 0x0012, 0x0013, 0x0014, 0x0015, 0x0016, 0x0017,
    0x0018, 0x0019, 0x001a, 0x001b, 0x001c, 0x001d, 0x001e, 0x001f,
    0x0020, 0x0021, 0x0022, 0x0023, 0x0024, 0x0025, 0x0026, 0x0027,
    0x0028, 0x0029, 0x002a, 0x002b, 0x002c, 0x002d, 0x002e, 0x002f,
    0x0030, 0x0031, 0x0032, 0x0033, 0x0034, 0x0035, 0x0036, 0x0037,
    0x0038, 0x0039, 0x003a, 0x003b, 0x003c, 0x003d, 0x003e, 0x003f,
    0x0040, 0x0041, 0x0042, 0x0043, 0x0044, 0x0045, 0x0046, 0x0047,
    0x0048, 0x0049, 0x004a, 0x004b, 0x004c, 0x004d, 0x004e, 0x004f,
    0x0050, 0x0051, 0x0052, 0x0053, 0x0054, 0x0055, 0x0056, 0x0057,
    0x0058, 0x0059, 0x005a, 0x005b, 0x005c, 0x005d, 0x005e, 0x005f,
    0x0060, 0x0061, 0x0062, 0x0063, 0x0064, 0x0065, 0x0066, 0x0067,
    0x0068, 0x0069, 0x006a, 0x006b, 0x006c, 0x006d, 0x006e, 0x006f,
    0x0070, 0x0071, 0x0072, 0x0073, 0x0074, 0x0075, 0x0076, 0x0077,
    0x0078, 0x0079, 0x007a, 0x007b, 0x007c, 0x007d, 0x007e, 0x007f,
    0x20ac, 0xffff, 0x201a, 0x0192, 0x201e, 0x2026, 0x2020, 0x2021,
    0x02c6, 0x2030, 0x0160, 0x2039, 0x0152, 0xffff, 0x017d, 0xffff,
    0xffff, 0x2018, 0x2019, 0x201c, 0x201d, 0x2022, 0x2013, 0x2014,
    0x02dc, 0x2122, 0x0161, 0x203a, 0x0153, 0xffff, 0x017e, 0x0178,
    0x00a0, 0x00a1, 0x00a2, 0x00a3, 0x00a4, 0x00a5, 0x00a6, 0x00a7,
    0x00a8, 0x00a9, 0x00aa, 0x00ab, 0x00ac, 0x00ad, 0x00ae, 0x00af,
    0x00b0, 0x00b1, 0x00b2, 0x00b3, 0x00b4, 0x00b5, 0x00b6, 0x00b7,
    0x00b8, 0x00b9, 0x00ba, 0x00bb, 0x00bc, 0x00bd, 0x00be, 0x00bf,
    0x00c0, 0x00c1, 0x00c2, 0x00c3, 0x00c4, 0x00c5, 0x00c6, 0x00c7,
    0x00c8, 0x00c9, 0x00ca, 0x00cb, 0x00cc, 0x00cd, 0x00ce, 0x00cf,
    0x00d0, 0x00d1, 0x00d2, 0x00d3, 0x00d4, 0x00d5, 0x00d6, 0x00d7,
    0x00d8, 0x00d9, 0x00da, 0x00db, 0x00dc, 0x00dd, 0x00de, 0x00df,
    0x00e0, 0x00e1, 0x00e2, 0x00e3, 0x00e4, 0x00e5, 0x00e6, 0x00e7,
    0x00e8, 0x00e9, 0x00ea, 0x00eb, 0x00ec, 0x00ed, 0x00ee, 0x00ef,
    0x00f0, 0x00f1, 0x00f2, 0x00f3, 0x00f4, 0x00f5, 0x00f6, 0x00f7,
    0x00f8, 0x00f9, 0x00fa, 0x00fb, 0x00fc, 0x00fd, 0x00fe, 0x00ff,
};

// 1.14d mac: 0001dad0
// Accepts tab, line feed, carriage return, printable ASCII and well-formed 2 to 4 byte
// sequences (no overlongs, no surrogates, nothing past U+10FFFF).
int UTF8_IsValidString(char* szString) {
    uint8_t* p = (uint8_t*)szString;
    if (p == NULL) {
        return 0;
    }
    for (;;) {
        uint8_t c = *p;
        if ((uint8_t)(c - '\t') < 2 || c == '\r') {
            p++;
        } else if (c == 0) {
            return 1;
        } else if ((uint8_t)(c - 0x20) <= 0x5e) {
            p++;
        } else if ((uint8_t)(c - 0xC2) <= 0x1d) {
            if (p[1] < 0x80 || p[1] > 0xBF) {
                return 0;
            }
            p += 2;
        } else if (c == 0xE0) {
            if (p[1] < 0xA0 || p[1] > 0xBF || p[2] < 0x80 || p[2] > 0xBF) {
                return 0;
            }
            p += 3;
        } else if (c == 0xED) {
            if (p[1] < 0x80 || p[1] > 0x9F || p[2] < 0x80 || p[2] > 0xBF) {
                return 0;
            }
            p += 3;
        } else if ((uint8_t)(c - 0xE1) <= 0xe) {
            if (p[1] < 0x80 || p[1] > 0xBF || p[2] < 0x80 || p[2] > 0xBF) {
                return 0;
            }
            p += 3;
        } else if (c == 0xF0) {
            if (p[1] < 0x90 || p[1] > 0xBF || p[2] < 0x80 || p[2] > 0xBF || p[3] < 0x80 || p[3] > 0xBF) {
                return 0;
            }
            p += 4;
        } else if ((uint8_t)(c - 0xF1) <= 2) {
            if (p[1] < 0x80 || p[1] > 0xBF || p[2] < 0x80 || p[2] > 0xBF || p[3] < 0x80 || p[3] > 0xBF) {
                return 0;
            }
            p += 4;
        } else if (c == 0xF4) {
            if (p[1] < 0x80 || p[1] > 0x8F || p[2] < 0x80 || p[2] > 0xBF || p[3] < 0x80 || p[3] > 0xBF) {
                return 0;
            }
            p += 4;
        } else {
            return 0;
        }
    }
}

// 1.14d mac: 0001d9b5
// The Windows SSTR.cpp algorithm over a signed 16-bit input: the code unit is sign-extended
// (movsx), so the surrogate pair test and the 4..6 byte forms fold away and a negative unit
// becomes U+FFFD in two bytes.
int Storm::Source::SSTR::UTF16_ToUTF8(uint8_t* pbUTF8, int nOutMaxChars, uint16_t* pszUTF16Out, int nInBytes, int* pnCharsWritten, int* pnBytesRead) {
    int16_t* pInEnd;
    if (nInBytes == 0x7fffffff) {
        pInEnd = (int16_t*)-1;
    } else {
        pInEnd = (int16_t*)pszUTF16Out + nInBytes;
    }
    uint8_t* pOutCur = pbUTF8;
    int16_t* pInCur = (int16_t*)pszUTF16Out;
    int nResult;
    while (pInCur < pInEnd) {
        uint32_t uCodepoint = *pInCur;
        int nUnits = 1;
        if (uCodepoint >= 0xd800 && uCodepoint <= 0xdbff) {
            if (pInCur + 1 >= pInEnd) {
                break;
            }
            uint32_t uSurrPair = pInCur[1];
            if (uSurrPair >= 0xdc00 && uSurrPair <= 0xdfff) {
                uCodepoint = ((uCodepoint - 0xd7f7) << 10) + uSurrPair;
                nUnits = 2;
            }
        }
        if (uCodepoint < 0x80) {
            nResult = 1;
            if (uCodepoint == 0) {
                if (pOutCur < pbUTF8 + nOutMaxChars) {
                    *pOutCur++ = 0;
                    nResult = 0;
                }
                goto report_counts;
            }
        } else if (uCodepoint < 0x800) {
            nResult = 2;
        } else if (uCodepoint < 0x10000) {
            nResult = 3;
        } else if (uCodepoint < 0x200000) {
            nResult = 4;
        } else if (uCodepoint < 0x4000000) {
            nResult = 5;
        } else if (uCodepoint <= 0x7fffffff) {
            nResult = 6;
        } else {
            uCodepoint = 0xfffd;
            nResult = 2;
        }
        uint8_t* pEnd = pOutCur + nResult;
        if (pEnd > pbUTF8 + nOutMaxChars) {
            goto report_counts;
        }
        switch (nResult) {
        case 6:
            *--pEnd = (uint8_t)((uCodepoint & 0x3f) | 0x80);
            uCodepoint >>= 6;
        case 5:
            *--pEnd = (uint8_t)((uCodepoint & 0x3f) | 0x80);
            uCodepoint >>= 6;
        case 4:
            *--pEnd = (uint8_t)((uCodepoint & 0x3f) | 0x80);
            uCodepoint >>= 6;
        case 3:
            *--pEnd = (uint8_t)((uCodepoint & 0x3f) | 0x80);
            uCodepoint >>= 6;
        case 2:
            *--pEnd = (uint8_t)((uCodepoint & 0x3f) | 0x80);
            uCodepoint >>= 6;
        case 1:
            *--pEnd = (uint8_t)(uCodepoint | gdwUTF8FirstByteMark[nResult]);
        }
        pInCur += nUnits;
        pOutCur = pEnd + nResult;
    }
    nResult = -1;
report_counts:
    if (pnBytesRead != NULL) {
        *pnBytesRead = pInCur - (int16_t*)pszUTF16Out;
    }
    if (pnCharsWritten != NULL) {
        *pnCharsWritten = pOutCur - pbUTF8;
    }
    return nResult;
}

// 1.14d mac: 0001dbca
int Storm::Source::SSTR::UTF8_ToUTF16(short* pszUTF16Out, int nOutMaxChars, uint8_t* pbUTF8In, int nInBytes, int* pnCharsWritten, int* pnBytesRead) {
    if (!UTF8_IsValidString((char*)pbUTF8In)) {
        return -1;
    }
    uint8_t* pInEnd;
    if (nInBytes == 0x7fffffff) {
        pInEnd = (uint8_t*)-1;
    } else {
        pInEnd = pbUTF8In + nInBytes;
    }
    uint8_t* pInCur = pbUTF8In;
    short* pOutCur = pszUTF16Out;
    int nResult;
    while (pInCur < pInEnd) {
        uint32_t uByteCount = gaUTF8ByteCountTable[*pInCur];
        if (pInCur + uByteCount >= pInEnd) {
            nResult = -1 - uByteCount;
            goto report_counts;
        }
        int nUsed = 0;
        int nAccum = 0;
        switch (uByteCount) {
        case 5:
            nUsed = 1;
            nAccum = *pInCur << 6;
        case 4:
            nAccum = (nAccum + pInCur[nUsed++]) << 6;
        case 3:
            nAccum = (nAccum + pInCur[nUsed++]) << 6;
        case 2:
            nAccum = (nAccum + pInCur[nUsed++]) << 6;
        case 1:
            nAccum = (nAccum + pInCur[nUsed++]) << 6;
        case 0:
            nAccum += pInCur[nUsed++];
        }
        uint32_t uCodepoint = nAccum - gdwUTF8ConversionTab[uByteCount];
        if (pOutCur >= pszUTF16Out + nOutMaxChars) {
        output_full:
            nResult = 1;
            goto report_counts;
        }
        if (uCodepoint <= 0xffff) {
            *pOutCur++ = (short)uCodepoint;
            if (uCodepoint == 0) {
                nResult = 0;
                goto report_counts;
            }
        } else if (uCodepoint > 0x10ffff) {
            *pOutCur++ = (short)0xfffd;
        } else {
            if (pOutCur + 1 >= pszUTF16Out + nOutMaxChars) {
                goto output_full;
            }
            pOutCur[0] = (short)((uCodepoint - 0x10000) >> 10) - 0x2800;
            pOutCur[1] = ((uCodepoint - 0x10000) & 0x3ff) | 0xdc00;
            pOutCur += 2;
        }
        pInCur += nUsed;
    }
    nResult = -1;
report_counts:
    if (pnBytesRead != NULL) {
        *pnBytesRead = pInCur - pbUTF8In;
    }
    if (pnCharsWritten != NULL) {
        *pnCharsWritten = pOutCur - pszUTF16Out;
    }
    return nResult;
}

// Kept out of line with the table as an argument: the Windows twin (Storm SSTR.cpp) has three
// callers (Latin1, Latin2, Korean) and the Mac callee takes the table in ecx, so the original TU
// had them too and the linker dead-stripped two. With only Latin1 here clang would inline it.
static int TRANSTBL_SingleByteToUnicode(const uint16_t* pXlatTbl, uint16_t* pDest, uint8_t* pbSrc, int nCount) __attribute__((noinline));
// 1.14d mac: 0001de00
// Expands bytes through a 256-entry table, stopping at the terminating zero (which is
// written when there is room) or at nCount.
static int TRANSTBL_SingleByteToUnicode(const uint16_t* pXlatTbl, uint16_t* pDest, uint8_t* pbSrc, int nCount) {
    if (nCount == 0) {
        return 0;
    }
    uint8_t bCur = *pbSrc;
    uint16_t* pOut = pDest;
    if (bCur != 0) {
        int nLeft = -nCount;
        do {
            nLeft++;
            pbSrc++;
            *pOut++ = pXlatTbl[bCur];
            bCur = *pbSrc;
            if (bCur == 0) {
                break;
            }
        } while (nLeft != 0);
        if (nLeft == 0) {
            return pOut - pDest;
        }
    }
    *pOut++ = 0;
    return pOut - pDest;
}

// 1.14d mac: 0001ddd4
int Storm::Source::SSTR::TRANSTBL_Latin1ToUnicode(uint16_t* pDest, uint8_t* pbSrc, int nCount) {
    return TRANSTBL_SingleByteToUnicode(gszUnicodeLatin1_1, pDest, pbSrc, nCount);
}

#endif  // D2_OS_MAC
