// StormMac Storm Mac/SOURCE/STORM.CPP: the global allocation operators, the Storm shutdown
// chain, the system options, a CRC32, the RSA key calls and the log header writer, 1.14d
// Intel Mac 000095fc..00009d29.
//
// The binary's __FILE__ for the operators is ".../StormMac/Storm Mac/SOURCE/STORM.CPP"
// (lines 35, 40, 46, 50). The functions after StormDestroy up to the SHA-1 unit carry no
// name of their own; they are placed here because nothing between STORM.CPP's last
// __FILE__ and the first SBig function names another file.
#include "../Win32Mac/Source/Win32/Win32Mac.h"

#if !defined(_WIN32) && D2_OS_MAC

#include <string.h>

// ---- what this file calls ----

void* SMemAlloc(uint32_t nBytes, const char* szFile, int nLine, uint32_t dwFlags);
int SMemFree(void* pMem, const char* szFile, int nLine, uint32_t dwFlags);
void SErrSetLastError(DWORD dwErrCode);

void SGdiDestroy(void);
void SVidDestroy(void);
void SDrawDestroy(void);
void SRgnDestroy(void);
void SMsgDestroy(void);
void SNetDestroy(void);
void SEvtDestroy(void);
void SBltDestroy(void);
void SCodeDestroy(void);
void SCmdDestroy(void);
void SCOMP_Destroy(void);
void SFileDestroy(void);
void STransDestroy(void);
// 1.14d mac: 0003602b
void STORM_ShutdownFirst(void);
// 1.14d mac: 00018ae2
void SSYSTEM_SetOption6(uint32_t* pnFirst, uint32_t* pnSecond);

void SBigNew(uint32_t** ppBig);
void SBigDel(uint32_t* pBig);
void SBigFromBinary(uint32_t* pBig, const void* pBytes, uint32_t nBytes);
void SBigPowMod(uint32_t* pResult, uint32_t* pBase, uint32_t* pExponent, uint32_t* pModulus);
void SBigToBinaryBuffer(uint32_t* pBig, uint8_t* pOut, uint32_t nOutSize, uint32_t* pnWritten);

BOOL GetModuleFileName(HANDLE hModule, char* szFilename, DWORD nSize);
BOOL GetUserName(char* szBuffer, size_t* pnSize);
BOOL GetComputerName(char* szBuffer, size_t* pnSize);
LONG InterlockedIncrement(LPLONG pnValue);
void GetLocalTime(void* pSystemTime);

extern const char* g_aszMonthAbbreviation[13];  // 00397050: "", "Jan" ...

typedef void (*STORM_LOGFN)(void* pContext, const char* szFormat, ...);

#define STORM_FILE "/Users/bclemetson/dev/diablo2/Diablo2/Source/DiabloAll/../../../StormMac/Storm Mac/SOURCE/STORM.CPP"

// 1.14d mac: 000095fc
void* operator new(size_t nSize) {
    return SMemAlloc(nSize, STORM_FILE, 35, 0);
}

// 1.14d mac: 00009632
void operator delete(void* pMem) noexcept {
    if (pMem != NULL) {
        SMemFree(pMem, STORM_FILE, 40, 0);
    }
}

// 1.14d mac: 00009671
void operator delete[](void* pMem) noexcept {
    if (pMem != NULL) {
        SMemFree(pMem, STORM_FILE, 46, 0);
    }
}

// 1.14d mac: 000096b0
void* STORM_Alloc(size_t nSize) {
    return SMemAlloc(nSize, STORM_FILE, 50, 0);
}

// 1.14d mac: 000096e6
// Every subsystem's destroy routine, the last-created first. Always returns 1.
BOOL StormDestroy(void) {
    STORM_ShutdownFirst();
    SGdiDestroy();
    SVidDestroy();
    SDrawDestroy();
    SRgnDestroy();
    SMsgDestroy();
    SNetDestroy();
    SEvtDestroy();
    SBltDestroy();
    SCodeDestroy();
    SCmdDestroy();
    SCOMP_Destroy();
    SFileDestroy();
    STransDestroy();
    return 1;
}

// The system option values, 1.14d mac 00554df8..00554e08.
// 1.14d mac: 00554dfc
uint32_t s_dwSystemOption1;
// 1.14d mac: 00554e00
uint32_t s_dwSystemOption2;
// 1.14d mac: 00554df8
uint32_t s_dwSystemOption3;
// 1.14d mac: 00554e04
uint32_t s_dwSystemOption4;
// 1.14d mac: 00554e08
uint32_t s_dwSystemOption5;

// 1.14d mac: 0000973c
BOOL SSYSTEM_SetSystemOption(uint32_t nOptionId, uint32_t* pValue, int32_t nValueSize) {
    SErrSetLastError(0x57);
    BOOL bResult = 0;
    if (pValue != NULL) {
        switch (nOptionId) {
        case 1:
            if (nValueSize != 4) {
                return bResult;
            }
            s_dwSystemOption1 = *pValue;
            break;
        case 2:
            if (nValueSize != 4) {
                return bResult;
            }
            {
                uint32_t dwOption2 = *pValue;
                if (dwOption2 == 0) {
                    return bResult;
                }
                // only a power of two
                if (((dwOption2 - 1) ^ dwOption2) != dwOption2 * 2 - 1) {
                    return bResult;
                }
                s_dwSystemOption2 = dwOption2;
            }
            break;
        case 3:
            if (nValueSize != 4) {
                return bResult;
            }
            s_dwSystemOption3 = *pValue;
            break;
        case 4:
            if (nValueSize != 4) {
                return bResult;
            }
            s_dwSystemOption4 = *pValue;
            break;
        case 5:
            if (nValueSize != 4) {
                return bResult;
            }
            s_dwSystemOption5 = *pValue;
            break;
        case 6: {
            if (nValueSize != 8) {
                return bResult;
            }
            uint32_t nFirst = pValue[0];
            uint32_t nSecond = pValue[1];
            SSYSTEM_SetOption6(&nFirst, &nSecond);
            break;
        }
        }
        SErrSetLastError(0);
        bResult = 1;
    }
    return bResult;
}

// 1.14d mac: 00320958
extern const uint32_t g_adwCrc32Table[256];

// 1.14d mac: 00009830
// Flag 1: start from 0 instead of the running value's complement; 2: process the data;
// 4: complement the result.
void STORM_Crc32(const uint8_t* pData, int nSize, uint32_t* pCrc, uint32_t dwFlags) {
    uint32_t dwCrc = 0;
    if (pCrc != NULL) {
        dwCrc = *pCrc;
    }
    uint32_t dwValue = ~-(uint32_t)((dwFlags & 1) == 0) | dwCrc;
    if ((dwFlags & 2) != 0) {
        for (; nSize != 0; nSize--) {
            dwValue = dwValue >> 8 ^ g_adwCrc32Table[(dwValue & 0xff) ^ *pData];
            pData++;
        }
    }
    if (pCrc != NULL) {
        *pCrc = (int32_t)(dwFlags << 29) >> 31 ^ dwValue;
    }
}

// A key is a pair of big numbers: the modulus, then the exponent.
// 1.14d mac: 0000989a
// Crypt::RSA::Prepare
void CryptRSA_Prepare(uint32_t** ppKey, const void* pModulus, uint32_t nModulusLen, const void* pExponent,
                      uint32_t nExponentLen) {
    SBigFromBinary(ppKey[0], pModulus, nModulusLen);
    SBigFromBinary(ppKey[1], pExponent, nExponentLen);
}

uint8_t* SSIGNATURE_RsaExportZeroPadded(uint32_t** ppBig, uint8_t* pOutBuf, uint32_t nBufLen);

// 1.14d mac: 000098dc
// Crypt::RSA::Process: data = data ^ exponent mod modulus, in place.
void CryptRSA_Process(uint32_t** pKey, uint8_t* pData, uint32_t nSize) {
    uint32_t* pInput;
    uint32_t* pResult[2];
    SBigNew(&pInput);
    SBigFromBinary(pInput, pData, nSize);
    SBigNew(pResult);
    SBigPowMod(pResult[0], pInput, pKey[1], pKey[0]);
    SSIGNATURE_RsaExportZeroPadded(pResult, pData, nSize);
    SBigDel(pResult[0]);
    SBigDel(pInput);
}

// 1.14d mac: 000099a0
// The big number as bytes, the rest of the buffer zeroed.
uint8_t* SSIGNATURE_RsaExportZeroPadded(uint32_t** ppBig, uint8_t* pOutBuf, uint32_t nBufLen) {
    uint32_t nWritten;
    SBigToBinaryBuffer(*ppBig, pOutBuf, nBufLen, &nWritten);
    if (nWritten < nBufLen && 0 < (int)(nBufLen - nWritten)) {
        bzero(pOutBuf + nWritten, nBufLen - nWritten);
    }
    return pOutBuf;
}

// 1.14d mac: 003f0234
// Nonzero while a header is being written: a nested or concurrent call writes nothing.
static LONG s_nLogHeaderBusy;

// 1.14d mac: 000099ee
// Flags: 1 keep the defaults, 2 the title, 4 the executable, 8 the time, 0x10 the
// computer, 0x20 the user. With flag 1 or none, a title adds 0x3e and no title 0x3c.
void LogComputerInfoHeader(uint32_t dwFlags, STORM_LOGFN pfnLog, void* pLogContext, const char* szHeader,
                           const uint16_t* pTime) {
    if (InterlockedIncrement(&s_nLogHeaderBusy) == 1) {
        uint32_t dwEffectiveFlags;
        uint16_t aLocalTime[8];
        char szBuffer[0x4e + 1 + 0x100 - 0x4e];
        size_t nBufferSize;
        if ((dwFlags == 0 || (dwEffectiveFlags = dwFlags, (dwFlags & 1) != 0)) &&
            (dwEffectiveFlags = dwFlags | 0x3c, szHeader != NULL) &&
            (dwEffectiveFlags = dwFlags | 0x3e, *szHeader == '\0')) {
            dwEffectiveFlags = dwFlags | 0x3c;
        }
        memset(szBuffer, '=', 0x4e);
        szBuffer[0x4e] = 0;
        pfnLog(pLogContext, "%s", szBuffer);
        if ((dwEffectiveFlags & 2) != 0 && szHeader != NULL && *szHeader != '\0') {
            pfnLog(pLogContext, "%s", szHeader);
            pfnLog(pLogContext, "");
        }
        if ((dwEffectiveFlags & 4) != 0) {
            GetModuleFileName(0, szBuffer, 0x104);
            pfnLog(pLogContext, "%-10s%s", "Exe:", szBuffer);
        }
        if ((dwEffectiveFlags & 8) != 0) {
            if (pTime == NULL) {
                pTime = aLocalTime;
                GetLocalTime(aLocalTime);
            }
            uint16_t nHour = pTime[4];
            uint16_t nHour12;
            if (nHour < 13) {
                nHour12 = 12;
                if (nHour != 0) {
                    nHour12 = nHour;
                }
            } else {
                nHour12 = nHour - 12;
            }
            int nMeridiem = 'P';
            if (nHour < 12) {
                nMeridiem = 'A';
            }
            pfnLog(pLogContext, "%-10s%3s %2d, %4d %2d:%02d:%02d.%03d %cM", "Time:", g_aszMonthAbbreviation[pTime[1]],
                   pTime[3], pTime[0], nHour12, pTime[5], pTime[6], pTime[7], nMeridiem);
        }
        if ((dwEffectiveFlags & 0x20) != 0) {
            nBufferSize = 0x101;
            if (!GetUserName(szBuffer, &nBufferSize)) {
                strcpy(szBuffer, "<unknown>");
            }
            pfnLog(pLogContext, "%-10s%s", "User:", szBuffer);
        }
        if ((dwEffectiveFlags & 0x10) != 0) {
            nBufferSize = 0x10;
            if (!GetComputerName(szBuffer, &nBufferSize)) {
                strcpy(szBuffer, "<unknown>");
            }
            pfnLog(pLogContext, "%-10s%s", "Computer:", szBuffer);
        }
        memset(szBuffer, '-', 0x4e);
        szBuffer[0x4e] = 0;
        pfnLog(pLogContext, "%s", szBuffer);
    }
    InterlockedDecrement(&s_nLogHeaderBusy);
}

// 1.14d mac: 00009d29
// An empty function.
void STORM_Nop(void) {
}

#endif  // D2_OS_MAC
