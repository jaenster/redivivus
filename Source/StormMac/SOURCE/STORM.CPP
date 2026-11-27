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
BOOL Storm_ReturnTrue(void);
// 1.14d mac: 00018ae2
void SLogSetSettings(const uint32_t* pdwSetting1, const uint32_t* pdwSetting2);

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

// 1.14d mac: 00397050
// Thirteen four-byte strings in place: "", then the three-letter months.
extern const char g_aszMonthAbbreviation[13][4];
const char g_aszMonthAbbreviation[13][4] = {"", "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};

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
    Storm_ReturnTrue();
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

// The system option values, 1.14d mac 00554df8..00554e0c, in the order the options are numbered
// (3, 1, 2, 4, 5), then the flag SErrDisplayError reads that nothing in the Mac build sets.
// 1.14d mac: 00554df8
uint32_t gnSSystemOption3;
// 1.14d mac: 00554dfc
uint32_t gnSSystemOption1;
// 1.14d mac: 00554e00
uint32_t gnSSystemWaveBufferSize;
// 1.14d mac: 00554e04
uint32_t gnSSystemAsyncReadSize;
// 1.14d mac: 00554e08
uint32_t gbCompressionDebugOutput;
// 1.14d mac: 00554e0c
BOOL gbSSystemShutdownInProgress;

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
            gnSSystemOption1 = *pValue;
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
                gnSSystemWaveBufferSize = dwOption2;
            }
            break;
        case 3:
            if (nValueSize != 4) {
                return bResult;
            }
            gnSSystemOption3 = *pValue;
            break;
        case 4:
            if (nValueSize != 4) {
                return bResult;
            }
            gnSSystemAsyncReadSize = *pValue;
            break;
        case 5:
            if (nValueSize != 4) {
                return bResult;
            }
            gbCompressionDebugOutput = *pValue;
            break;
        case 6: {
            if (nValueSize != 8) {
                return bResult;
            }
            uint32_t nFirst = pValue[0];
            uint32_t nSecond = pValue[1];
            SLogSetSettings(&nFirst, &nSecond);
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
const uint32_t g_adwCrc32Table[256] = {
    0x00000000, 0x77073096, 0xee0e612c, 0x990951ba, 0x076dc419, 0x706af48f,
    0xe963a535, 0x9e6495a3, 0x0edb8832, 0x79dcb8a4, 0xe0d5e91e, 0x97d2d988,
    0x09b64c2b, 0x7eb17cbd, 0xe7b82d07, 0x90bf1d91, 0x1db71064, 0x6ab020f2,
    0xf3b97148, 0x84be41de, 0x1adad47d, 0x6ddde4eb, 0xf4d4b551, 0x83d385c7,
    0x136c9856, 0x646ba8c0, 0xfd62f97a, 0x8a65c9ec, 0x14015c4f, 0x63066cd9,
    0xfa0f3d63, 0x8d080df5, 0x3b6e20c8, 0x4c69105e, 0xd56041e4, 0xa2677172,
    0x3c03e4d1, 0x4b04d447, 0xd20d85fd, 0xa50ab56b, 0x35b5a8fa, 0x42b2986c,
    0xdbbbc9d6, 0xacbcf940, 0x32d86ce3, 0x45df5c75, 0xdcd60dcf, 0xabd13d59,
    0x26d930ac, 0x51de003a, 0xc8d75180, 0xbfd06116, 0x21b4f4b5, 0x56b3c423,
    0xcfba9599, 0xb8bda50f, 0x2802b89e, 0x5f058808, 0xc60cd9b2, 0xb10be924,
    0x2f6f7c87, 0x58684c11, 0xc1611dab, 0xb6662d3d, 0x76dc4190, 0x01db7106,
    0x98d220bc, 0xefd5102a, 0x71b18589, 0x06b6b51f, 0x9fbfe4a5, 0xe8b8d433,
    0x7807c9a2, 0x0f00f934, 0x9609a88e, 0xe10e9818, 0x7f6a0dbb, 0x086d3d2d,
    0x91646c97, 0xe6635c01, 0x6b6b51f4, 0x1c6c6162, 0x856530d8, 0xf262004e,
    0x6c0695ed, 0x1b01a57b, 0x8208f4c1, 0xf50fc457, 0x65b0d9c6, 0x12b7e950,
    0x8bbeb8ea, 0xfcb9887c, 0x62dd1ddf, 0x15da2d49, 0x8cd37cf3, 0xfbd44c65,
    0x4db26158, 0x3ab551ce, 0xa3bc0074, 0xd4bb30e2, 0x4adfa541, 0x3dd895d7,
    0xa4d1c46d, 0xd3d6f4fb, 0x4369e96a, 0x346ed9fc, 0xad678846, 0xda60b8d0,
    0x44042d73, 0x33031de5, 0xaa0a4c5f, 0xdd0d7cc9, 0x5005713c, 0x270241aa,
    0xbe0b1010, 0xc90c2086, 0x5768b525, 0x206f85b3, 0xb966d409, 0xce61e49f,
    0x5edef90e, 0x29d9c998, 0xb0d09822, 0xc7d7a8b4, 0x59b33d17, 0x2eb40d81,
    0xb7bd5c3b, 0xc0ba6cad, 0xedb88320, 0x9abfb3b6, 0x03b6e20c, 0x74b1d29a,
    0xead54739, 0x9dd277af, 0x04db2615, 0x73dc1683, 0xe3630b12, 0x94643b84,
    0x0d6d6a3e, 0x7a6a5aa8, 0xe40ecf0b, 0x9309ff9d, 0x0a00ae27, 0x7d079eb1,
    0xf00f9344, 0x8708a3d2, 0x1e01f268, 0x6906c2fe, 0xf762575d, 0x806567cb,
    0x196c3671, 0x6e6b06e7, 0xfed41b76, 0x89d32be0, 0x10da7a5a, 0x67dd4acc,
    0xf9b9df6f, 0x8ebeeff9, 0x17b7be43, 0x60b08ed5, 0xd6d6a3e8, 0xa1d1937e,
    0x38d8c2c4, 0x4fdff252, 0xd1bb67f1, 0xa6bc5767, 0x3fb506dd, 0x48b2364b,
    0xd80d2bda, 0xaf0a1b4c, 0x36034af6, 0x41047a60, 0xdf60efc3, 0xa867df55,
    0x316e8eef, 0x4669be79, 0xcb61b38c, 0xbc66831a, 0x256fd2a0, 0x5268e236,
    0xcc0c7795, 0xbb0b4703, 0x220216b9, 0x5505262f, 0xc5ba3bbe, 0xb2bd0b28,
    0x2bb45a92, 0x5cb36a04, 0xc2d7ffa7, 0xb5d0cf31, 0x2cd99e8b, 0x5bdeae1d,
    0x9b64c2b0, 0xec63f226, 0x756aa39c, 0x026d930a, 0x9c0906a9, 0xeb0e363f,
    0x72076785, 0x05005713, 0x95bf4a82, 0xe2b87a14, 0x7bb12bae, 0x0cb61b38,
    0x92d28e9b, 0xe5d5be0d, 0x7cdcefb7, 0x0bdbdf21, 0x86d3d2d4, 0xf1d4e242,
    0x68ddb3f8, 0x1fda836e, 0x81be16cd, 0xf6b9265b, 0x6fb077e1, 0x18b74777,
    0x88085ae6, 0xff0f6a70, 0x66063bca, 0x11010b5c, 0x8f659eff, 0xf862ae69,
    0x616bffd3, 0x166ccf45, 0xa00ae278, 0xd70dd2ee, 0x4e048354, 0x3903b3c2,
    0xa7672661, 0xd06016f7, 0x4969474d, 0x3e6e77db, 0xaed16a4a, 0xd9d65adc,
    0x40df0b66, 0x37d83bf0, 0xa9bcae53, 0xdebb9ec5, 0x47b2cf7f, 0x30b5ffe9,
    0xbdbdf21c, 0xcabac28a, 0x53b39330, 0x24b4a3a6, 0xbad03605, 0xcdd70693,
    0x54de5729, 0x23d967bf, 0xb3667a2e, 0xc4614ab8, 0x5d681b02, 0x2a6f2b94,
    0xb40bbe37, 0xc30c8ea1, 0x5a05df1b, 0x2d02ef8d,
};

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
