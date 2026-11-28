// Storm's registry on the Mac, SRegMac.mm (1.14d __FILE__ 0030f7d6.. in SRegSaveData,
// SRegSaveString, SREG_ReadEscapedData, SREG_FreeValueData, SREG_DestroyEntry). 1.14d Intel Mac
// band 0003a696..0003e21d: the SReg exports and the prefs-file code behind them, the stpl.h
// template instantiations the file emitted, and its static initialiser (0003e0a0).
//
// The Windows registry is a set of "Current User", "Local Machine" and "Battle.net" keys; the
// Mac keeps the values in a hash table keyed by "<root>\<subkey>\<value>" and mirrors each root
// and top-level subkey in a com.blizzard.<subkey>.prefs text file. A prefs file is read once, the
// first time a key of it is asked for, and every file the table has values for is written back by
// SREG_FlushPrefsFiles. A value line starts with its type: '#' a number, '$' a string, '@' binary
// data; the data line is %-escaped.
#include "Source/StormMac/H/stpl.h"

#if !defined(_WIN32) && D2_MAC_CARBON

#include "Source/StormMac/H/StormApi.h"

#import <Foundation/Foundation.h>
#include <CoreServices/CoreServices.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#define SREG_PATH_MAX 1024

#define SREG_FLAG_BATTLENET 2
#define SREG_FLAG_MACHINE 4

enum SRegValueType {
    SREG_TYPE_NONE = 0,
    SREG_TYPE_NUMBER = 1,
    SREG_TYPE_STRING = 2,
    SREG_TYPE_DATA = 3,
};


// A value of the registry: its key is "<root>\<subkey>\<value>".
class RegistryEntry {
public:
    uint32_t m_hashval;      // 0x00
    TSLink<RegistryEntry> m_linktoslot;  // 0x04
    TSLink<RegistryEntry> m_linktofull;  // 0x0C
    HASHKEY_STRI m_key;      // 0x14
    int m_type;              // 0x18: SRegValueType
    void* m_data;            // 0x1C: the string or the bytes; the number itself for type 1
    uint32_t m_size;         // 0x20: the byte count of type 3 data

    RegistryEntry() : m_type(SREG_TYPE_NONE), m_data(NULL), m_size(0) {}
    ~RegistryEntry();
};

// A prefs file whose values have been read.
class RegistryFileEntry {
public:
    uint32_t m_hashval;      // 0x00
    TSLink<RegistryFileEntry> m_linktoslot;  // 0x04
    TSLink<RegistryFileEntry> m_linktofull;  // 0x0C
    HASHKEY_STRI m_key;      // 0x14: the prefs file's path
};

// A prefs file being written: the file is created when the first key of its root is
// flushed, and removed again at the end unless a value went into it.
class WriteRegistryEntry {
public:
    uint32_t m_hashval;      // 0x00
    TSLink<WriteRegistryEntry> m_linktoslot;  // 0x04
    TSLink<WriteRegistryEntry> m_linktofull;  // 0x0C
    HASHKEY_STRI m_key;      // 0x14: "<root>\<subkey>"
    FILE* m_file;            // 0x18
    char m_szPath[SREG_PATH_MAX];  // 0x1C
    bool m_bUsed;            // 0x41C

    ~WriteRegistryEntry();
};

// 1.14d mac: 003f5060
static TSHashTable<RegistryEntry, HASHKEY_STRI> s_tRegistry;
// 1.14d mac: 003f5088
static TSHashTable<RegistryFileEntry, HASHKEY_STRI> s_tLoadedFiles;
// 1.14d mac: 003f50b0
// The "Shared" folder of the local users' area; made on first use.
static NSURL* s_pSharedPrefsURL;
// 1.14d mac: 003f50b4
// The user's Preferences folder, found on first use.
static char s_szUserPrefsFolder[SREG_PATH_MAX];

// 1.14d mac: 00397ad8
const char* g_szBlizzardPrefsFolder = "/Blizzard";
// 1.14d mac: 00397adc
const char* g_szBattleNetPrefsFolder = "/Battle.net";

// 1.14d mac: 0032532d
// A hex digit's value, 0 for anything else.
static const uint8_t s_aHexNibble[256] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 0, 0, 0, 0, 0, 0,
    0, 10, 11, 12, 13, 14, 15, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 10, 11, 12, 13, 14, 15,
};

RegistryEntry::~RegistryEntry() {
    if ((uint32_t)(m_type - SREG_TYPE_STRING) < 2 && m_data != NULL) {
        SMemFree(m_data, __FILE__, 105, 0);
    }
}

WriteRegistryEntry::~WriteRegistryEntry() {
    if (m_file != NULL) {
        fclose(m_file);
    }
    if (!m_bUsed) {
        unlink(m_szPath);
    }
}

void SREG_FreeValueData(RegistryEntry* pEntry);
static void SREG_LoadPrefsFile(const char* szPrefsPath);

// 1.14d mac: 0003b53a
void SREG_GetUserPrefsFolder() {
    static int s_nFound = (s_szUserPrefsFolder[0] = 0, [] {
        FSRef tFolder;
        if (FSFindFolder(kOnAppropriateDisk, kPreferencesFolderType, true, &tFolder) == noErr) {
            FSRefMakePath(&tFolder, (UInt8*)s_szUserPrefsFolder, SREG_PATH_MAX);
        }
        return 1;
    }());
    (void)s_nFound;
}

// 1.14d mac: 0003b5ed
char* SREG_GetSharedPrefsFolder() {
    NSAutoreleasePool* pPool = [[NSAutoreleasePool alloc] init];
    if (s_pSharedPrefsURL == nil) {
        NSFileManager* pManager = [NSFileManager defaultManager];
        NSURL* pUsers = [pManager URLForDirectory:NSUserDirectory inDomain:NSLocalDomainMask appropriateForURL:nil create:NO error:nil];
        NSURL* pShared = [NSURL URLWithString:@"Shared/" relativeToURL:pUsers];
        NSString* pPath = [pShared path];
        if ([pManager createDirectoryAtPath:pPath withIntermediateDirectories:YES attributes:nil error:nil]) {
            s_pSharedPrefsURL = [pShared retain];
        }
    }
    [pPool release];
    if (s_pSharedPrefsURL == nil) {
        return NULL;
    }
    return (char*)[[NSFileManager defaultManager] fileSystemRepresentationWithPath:[s_pSharedPrefsURL path]];
}

// "<root>\<subkey>\<value>" and the prefs file it lives in, from the SReg flags.
// 1.14d mac: 0003a76f
static void SREG_BuildPrefsPaths(const char* szSubKey, const char* szValueName, uint32_t dwFlags, char* szKeyPath,
                                 char* szPrefsPath) {
    char szKeyRoot[SREG_PATH_MAX];
    const char* szRoot;
    const char* szFolder;
    const char* szSubFolder;
    SStrCopy(szKeyRoot, szSubKey, SREG_PATH_MAX);
    char* pSeparator = SStrChr(szKeyRoot, '\\');
    if (pSeparator) {
        *pSeparator = '\0';
    }
    if (dwFlags & SREG_FLAG_BATTLENET) {
        szFolder = SREG_GetSharedPrefsFolder();
        szRoot = "Battle.net";
        szSubFolder = g_szBattleNetPrefsFolder;
    } else if (dwFlags & SREG_FLAG_MACHINE) {
        szFolder = SREG_GetSharedPrefsFolder();
        szRoot = "Local Machine";
        szSubFolder = g_szBlizzardPrefsFolder;
    } else {
        SREG_GetUserPrefsFolder();
        szFolder = s_szUserPrefsFolder;
        szRoot = "Current User";
        szSubFolder = "";
    }
    SStrPrintf(szKeyPath, SREG_PATH_MAX, "%s\\%s\\%s", szRoot, szSubKey, szValueName);
    SStrPrintf(szPrefsPath, SREG_PATH_MAX, "%s%s/com.blizzard.%s.prefs", szFolder, szSubFolder, szKeyRoot);
}

// The value, reading its prefs file first if it is not known yet.
// 1.14d mac: 0003a89e
static RegistryEntry* SREG_LookupValue(const char* szKeyPath, const char* szPrefsPath) {
    RegistryEntry* pEntry = s_tRegistry.Ptr(szKeyPath);
    if (pEntry == NULL) {
        SREG_LoadPrefsFile(szPrefsPath);
        pEntry = s_tRegistry.Ptr(szKeyPath);
    }
    return pEntry;
}

// 1.14d mac: 0003a696
int Storm::Source::SReg::SRegLoadData(char* szSubKey, char* szValueName, int nFlags, uint8_t* pbData, uint32_t dwBufSize,
                 uint32_t* pdwDataLen) {
    char szKeyPath[SREG_PATH_MAX];
    char szPrefsPath[SREG_PATH_MAX];
    char cSubKey = *szSubKey;
    if (*szValueName == '\0' || szValueName == NULL || cSubKey == '\0' || szSubKey == NULL) {
        SErrSetLastError(0x57);
        return 0;
    }
    SREG_BuildPrefsPaths(szSubKey, szValueName, nFlags, szKeyPath, szPrefsPath);
    RegistryEntry* pEntry = SREG_LookupValue(szKeyPath, szPrefsPath);
    if (pEntry == NULL || pEntry->m_type != SREG_TYPE_DATA) {
        return 0;
    }
    uint32_t dwSize = pEntry->m_size;
    if (dwBufSize != 0 && pbData != NULL) {
        if (dwBufSize < dwSize) {
            dwSize = dwBufSize;
        }
        if ((int)dwSize > 0) {
            memmove(pbData, pEntry->m_data, dwSize);
        }
    }
    if (pdwDataLen != NULL) {
        *pdwDataLen = dwSize;
    }
    return 1;
}

// 1.14d mac: 0003a8e2
int Storm::Source::SReg::SRegLoadString(const char* szSubKey, const char* szValueName, int nFlags, char* szOutBuf, uint32_t dwOutBufSize) {
    char szKeyPath[SREG_PATH_MAX];
    char szPrefsPath[SREG_PATH_MAX];
    char cValueName = *szValueName;
    char cSubKey = *szSubKey;
    if (dwOutBufSize == 0 || szOutBuf == NULL || cValueName == '\0' || szValueName == NULL || cSubKey == '\0' ||
        szSubKey == NULL) {
        SErrSetLastError(0x57);
        return 0;
    }
    SREG_BuildPrefsPaths(szSubKey, szValueName, nFlags, szKeyPath, szPrefsPath);
    RegistryEntry* pEntry = SREG_LookupValue(szKeyPath, szPrefsPath);
    if (pEntry == NULL || pEntry->m_type != SREG_TYPE_STRING) {
        return 0;
    }
    SStrCopy(szOutBuf, (const char*)pEntry->m_data, dwOutBufSize);
    return 1;
}

// 1.14d mac: 0003a9a0
int Storm::Source::SReg::SRegLoadValue(const char* szSubKey, const char* szValueName, int nFlags, int32_t* pnValue) {
    char szKeyPath[SREG_PATH_MAX];
    char szPrefsPath[SREG_PATH_MAX];
    if (pnValue == NULL || *szValueName == '\0' || szValueName == NULL || *szSubKey == '\0' || szSubKey == NULL) {
        SErrSetLastError(0x57);
        return 0;
    }
    SREG_BuildPrefsPaths(szSubKey, szValueName, nFlags, szKeyPath, szPrefsPath);
    RegistryEntry* pEntry = SREG_LookupValue(szKeyPath, szPrefsPath);
    if (pEntry == NULL || pEntry->m_type != SREG_TYPE_NUMBER) {
        return 0;
    }
    *pnValue = (int32_t)(intptr_t)pEntry->m_data;
    return 1;
}

// The value to write to: found, its old data dropped, or made.
// 1.14d mac: 0003ab38
static RegistryEntry* SREG_FindOrCreateValue(const char* szKeyPath, const char* szPrefsPath) {
    RegistryEntry* pEntry = SREG_LookupValue(szKeyPath, szPrefsPath);
    if (pEntry == NULL) {
        pEntry = s_tRegistry.New(szKeyPath, 0, 0);
    } else {
        SREG_FreeValueData(pEntry);
    }
    return pEntry;
}

// 1.14d mac: 0003aa47
int Storm::Source::SReg::SRegSaveData(char* szSubKey, const char* szValueName, uint32_t dwFlags, uint8_t* pbData, uint32_t dwDataLen) {
    char szKeyPath[SREG_PATH_MAX];
    char szPrefsPath[SREG_PATH_MAX];
    char cSubKey = *szSubKey;
    if (*szValueName == '\0' || szValueName == NULL || cSubKey == '\0' || szSubKey == NULL) {
        SErrSetLastError(0x57);
        return 0;
    }
    SREG_BuildPrefsPaths(szSubKey, szValueName, dwFlags, szKeyPath, szPrefsPath);
    RegistryEntry* pEntry = SREG_FindOrCreateValue(szKeyPath, szPrefsPath);
    pEntry->m_type = SREG_TYPE_DATA;
    pEntry->m_data = SMemAlloc(dwDataLen, __FILE__, 578, 0);
    pEntry->m_size = dwDataLen;
    if ((int)dwDataLen > 0) {
        memmove(pEntry->m_data, pbData, dwDataLen);
    }
    return 1;
}

// 1.14d mac: 0003abad
int Storm::Source::SReg::SRegSaveString(char* szSubKey, const char* szValueName, int nFlags, char* szValue) {
    char szKeyPath[SREG_PATH_MAX];
    char szPrefsPath[SREG_PATH_MAX];
    if (szValue == NULL || *szValueName == '\0' || szValueName == NULL || *szSubKey == '\0' || szSubKey == NULL) {
        SErrSetLastError(0x57);
        return 0;
    }
    uint32_t dwLength = SStrLen(szValue);
    SREG_BuildPrefsPaths(szSubKey, szValueName, nFlags, szKeyPath, szPrefsPath);
    RegistryEntry* pEntry = SREG_FindOrCreateValue(szKeyPath, szPrefsPath);
    pEntry->m_type = SREG_TYPE_STRING;
    size_t nBytes = dwLength + 1;
    pEntry->m_data = SMemAlloc(nBytes, __FILE__, 608, 0);
    if ((int)nBytes > 0) {
        memmove(pEntry->m_data, szValue, nBytes);
    }
    return 1;
}

// 1.14d mac: 0003acba
int Storm::Source::SReg::SRegSaveValue(char* szSubKey, const char* szValueName, int nFlags, int32_t nValue) {
    char szKeyPath[SREG_PATH_MAX];
    char szPrefsPath[SREG_PATH_MAX];
    char cSubKey = *szSubKey;
    if (*szValueName == '\0' || szValueName == NULL || cSubKey == '\0' || szSubKey == NULL) {
        SErrSetLastError(0x57);
        return 0;
    }
    SREG_BuildPrefsPaths(szSubKey, szValueName, nFlags, szKeyPath, szPrefsPath);
    RegistryEntry* pEntry = SREG_FindOrCreateValue(szKeyPath, szPrefsPath);
    pEntry->m_type = SREG_TYPE_NUMBER;
    pEntry->m_data = (void*)(intptr_t)nValue;
    return 1;
}

// 1.14d mac: 0003ad51
int Storm::Source::SReg::SRegDeleteValue(const char* szSubKey, const char* szValueName, uint32_t dwFlags) {
    char szKeyPath[SREG_PATH_MAX];
    char szPrefsPath[SREG_PATH_MAX];
    SREG_BuildPrefsPaths(szSubKey, szValueName, dwFlags, szKeyPath, szPrefsPath);
    SREG_LoadPrefsFile(szPrefsPath);
    RegistryEntry* pEntry = s_tRegistry.Ptr(szKeyPath);
    if (pEntry != NULL) {
        SREG_FreeValueData(pEntry);
    }
    return 1;
}

// Out of line in 1.14d although its one remaining call site would inline it: the callee takes
// ecx/edx (fastcc, so static), hence a second caller in this unit was dead-stripped at link.
static __attribute__((noinline)) void SREG_ReadEscapedData(FILE* pFile, RegistryEntry* pEntry, bool bAppendTerminator);

// The values of a prefs file, once.
// 1.14d mac: 0003ade0
static void SREG_LoadPrefsFile(const char* szPrefsPath) {
    char szLine[SREG_PATH_MAX];
    if (s_tLoadedFiles.Ptr(szPrefsPath) == NULL) {
        FILE* pFile = fopen(szPrefsPath, "r");
        if (pFile != NULL) {
            for (;;) {
                int nType = fgetc(pFile);
                uint32_t nLength = 0;
                do {
                    int nChar = fgetc(pFile);
                    if (nChar == EOF) {
                        fclose(pFile);
                        goto done;
                    }
                    if (nChar == '\n') {
                        szLine[nLength] = '\0';
                        break;
                    }
                    szLine[nLength++] = (char)nChar;
                } while (nLength < SREG_PATH_MAX);
                if (s_tRegistry.Ptr(szLine) != NULL) {
                    continue;
                }
                RegistryEntry* pEntry = s_tRegistry.New(szLine, 0, 0);
                bool bAppendTerminator;
                if ((char)nType == '@') {
                    pEntry->m_type = SREG_TYPE_DATA;
                    bAppendTerminator = false;
                } else {
                    if ((char)nType != '$') {
                        if ((char)nType == '#') {
                            pEntry->m_type = SREG_TYPE_NUMBER;
                            fscanf(pFile, "%ld\n", (long*)&pEntry->m_data);
                        }
                        continue;
                    }
                    pEntry->m_type = SREG_TYPE_STRING;
                    bAppendTerminator = true;
                }
                SREG_ReadEscapedData(pFile, pEntry, bAppendTerminator);
            }
        }
    done:
        s_tLoadedFiles.New(szPrefsPath, 0, 0);
    }
}

static void SREG_WriteEscapedData(FILE* pFile, const uint8_t* pData, int nCount);

// Every value, to the prefs file of its root and subkey.
// 1.14d mac: 0003afe5
void SREG_FlushPrefsFiles() {
    TSHashTable<WriteRegistryEntry, HASHKEY_STRI> tFiles;
    char szFileKey[SREG_PATH_MAX];
    char szFolder[SREG_PATH_MAX];
    for (RegistryEntry* pEntry = s_tRegistry.m_fulllist.Head(); pEntry != NULL && ((intptr_t)pEntry & 1) == 0;
         pEntry = s_tRegistry.m_fulllist.Next(pEntry)) {
        const char* szText = pEntry->m_key.m_str;
        char* pSeparator = SStrChr(szText, '\\');
        if (pSeparator == NULL) {
            continue;
        }
        pSeparator = SStrChr(pSeparator + 1, '\\');
        if (pSeparator == NULL) {
            continue;
        }
        SStrCopy(szFileKey, szText, (int)(1 - (intptr_t)szText + (intptr_t)pSeparator));
        WriteRegistryEntry* pFile = tFiles.Ptr(szFileKey);
        if (pFile == NULL) {
            const char* szBase;
            const char* szSubFolder;
            mode_t nMode;
            if (szFileKey[0] == 'B') {
                szBase = SREG_GetSharedPrefsFolder();
                szSubFolder = g_szBattleNetPrefsFolder;
                nMode = 0777;
            } else if (szFileKey[0] == 'L') {
                szBase = SREG_GetSharedPrefsFolder();
                szSubFolder = g_szBlizzardPrefsFolder;
                nMode = 0777;
            } else if (szFileKey[0] == 'C') {
                nMode = 0755;
                SREG_GetUserPrefsFolder();
                szSubFolder = "";
                szBase = s_szUserPrefsFolder;
            } else {
                continue;
            }
            char* pRoot = SStrChr(szFileKey, '\\');
            pFile = tFiles.New(szFileKey, 0, 0);
            SStrPrintf(szFolder, SREG_PATH_MAX, "%s%s", szBase, szSubFolder);
            SStrPrintf(pFile->m_szPath, SREG_PATH_MAX, "%s/com.blizzard.%s.prefs", szFolder, pRoot + 1);
            NSFileManager* pManager = [NSFileManager defaultManager];
            NSString* pFolder = [pManager stringWithFileSystemRepresentation:szFolder length:strlen(szFolder)];
            if ([pManager createDirectoryAtPath:pFolder withIntermediateDirectories:YES attributes:nil error:nil]) {
                chmod(szFolder, nMode);
                pFile->m_bUsed = false;
                pFile->m_file = fopen(pFile->m_szPath, "w");
            }
        }
        int nType = pEntry->m_type;
        if (nType == SREG_TYPE_NONE || pFile->m_file == NULL) {
            continue;
        }
        pFile->m_bUsed = true;
        if (nType == SREG_TYPE_DATA) {
            fprintf(pFile->m_file, "%c%s\n", '@', szText);
            SREG_WriteEscapedData(pFile->m_file, (const uint8_t*)pEntry->m_data, pEntry->m_size);
        } else if (nType == SREG_TYPE_STRING) {
            fprintf(pFile->m_file, "%c%s\n", '$', szText);
            SREG_WriteEscapedData(pFile->m_file, (const uint8_t*)pEntry->m_data, SStrLen((const char*)pEntry->m_data));
        } else if (nType == SREG_TYPE_NUMBER) {
            fprintf(pFile->m_file, "%c%s\n%ld\n", '#', szText, (long)pEntry->m_data);
        }
    }
    tFiles.InternalClear(0);
}

// A byte the data line escapes: a control character or a byte above 0x7F.
static inline bool SREG_IsControlByte(uint8_t byChar) {
    return byChar < ' ' || (char)byChar < 0;
}

// The bytes of a value on one line, a run of control characters and '%' inside %..% as hex.
// 1.14d mac: 0003b786
static void SREG_WriteEscapedData(FILE* pFile, const uint8_t* pData, int nCount) {
    if (nCount != 0) {
        bool bInEscape = false;
        do {
            uint8_t byChar = *pData;
            uint32_t nOut = byChar;
            if (nOut != '%' || bInEscape) {
                if (bInEscape) {
                    if (byChar == '%' || SREG_IsControlByte(byChar) ||
                        (nCount != 1 && (SREG_IsControlByte(pData[1]) || pData[1] == '%'))) {
                        goto hex;
                    }
                    fputc('%', pFile);
                    bInEscape = false;
                } else if (byChar == '%' || SREG_IsControlByte(byChar)) {
                    fputc('%', pFile);
                    bInEscape = true;
                hex:
                    fputc("0123456789ABCDEF"[byChar >> 4], pFile);
                    nOut = "0123456789ABCDEF"[nOut & 0xF];
                }
            } else {
                fputc('%', pFile);
                nOut = '%';
            }
            fputc(nOut, pFile);
            nCount--;
            pData++;
        } while (nCount != 0);
        if (bInEscape) {
            fputc('%', pFile);
        }
    }
    fputc('\n', pFile);
}

// Writes the values out and drops them.
// 1.14d mac: 0003b8c9
void SREG_Destroy() {
    SREG_FlushPrefsFiles();
    RegistryEntry* pEntry = s_tRegistry.m_fulllist.Head();
    while (!TSLink<RegistryEntry>::IsTerminator(pEntry)) {
        RegistryEntry* pNext = s_tRegistry.m_fulllist.Next(pEntry);
        s_tRegistry.DeleteNode(pEntry);
        pEntry = pNext;
    }
    s_tLoadedFiles.InternalClear(0);
}

// The value line of a prefs file: %..% runs are hex digits for bytes; the buffer grows a byte at a time.
// 1.14d mac: 0003b98f
static void SREG_ReadEscapedData(FILE* pFile, RegistryEntry* pEntry, bool bAppendTerminator) {
    TSGrowableArray<uint8_t> tBuffer;
    bool bInEscape = false;
    bool bHighNibble;
    bool bGotHexDigit;
    for (;;) {
        bHighNibble = true;
        bGotHexDigit = false;
        for (;;) {
            int nChar = fgetc(pFile);
            if (nChar == '\n' || nChar == EOF) {
                goto finish;
            }
            if (nChar == '%') {
                if (!bGotHexDigit && bInEscape) {
                    tBuffer.Reserve(1, 1);
                    tBuffer.m_data[tBuffer.m_count++] = '%';
                }
                bInEscape ^= 1;
                break;
            }
            if (bInEscape) {
                uint8_t byNibble = s_aHexNibble[nChar];
                if (bHighNibble) {
                    tBuffer.Reserve(1, 1);
                    tBuffer.m_data[tBuffer.m_count++] = byNibble << 4;
                    bHighNibble = false;
                } else {
                    tBuffer.m_data[tBuffer.m_count - 1] |= byNibble;
                    bHighNibble = true;
                }
                bGotHexDigit = true;
            } else {
                tBuffer.Reserve(1, 1);
                tBuffer.m_data[tBuffer.m_count++] = (uint8_t)nChar;
            }
        }
    }
finish:
    if (bAppendTerminator) {
        tBuffer.Reserve(1, 1);
        tBuffer.m_data[tBuffer.m_count++] = 0;
    }
    pEntry->m_size = tBuffer.m_count;
    pEntry->m_data = SMemAlloc(tBuffer.m_count, __FILE__, 206, 0);
    if ((int)tBuffer.m_count > 0) {
        memmove(pEntry->m_data, tBuffer.m_data, tBuffer.m_count);
    }
}

// Strings and data own their bytes.
// 1.14d mac: 0003bc80
void SREG_FreeValueData(RegistryEntry* pEntry) {
    if ((uint32_t)(pEntry->m_type - SREG_TYPE_STRING) < 2 && pEntry->m_data != NULL) {
        SMemFree(pEntry->m_data, __FILE__, 112, 0);
    }
    pEntry->m_type = SREG_TYPE_NONE;
    pEntry->m_data = NULL;
    pEntry->m_size = 0;
}

// The template instantiations and compiler-made functions this file's object holds
// (stpl.h), by the qualified name the compare tool pairs them with:
// 1.14d mac: 0003bbb0 TSHashTable<RegistryEntry,HASHKEY_STRI>::~TSHashTable[D1]
// 1.14d mac: 0003bbba TSHashTable<RegistryFileEntry,HASHKEY_STRI>::~TSHashTable[D1]
// 1.14d mac: 0003bbc4 TSHashTable<RegistryEntry,HASHKEY_STRI>::Ptr
// 1.14d mac: 0003bce0 TSHashTable<RegistryEntry,HASHKEY_STRI>::DeleteNode
// 1.14d mac: 0003bd30 TSHashTable<WriteRegistryEntry,HASHKEY_STRI>::Ptr
// 1.14d mac: 0003bdec TSHashTable<WriteRegistryEntry,HASHKEY_STRI>::~TSHashTable[D1]
// 1.14d mac: 0003bdf6 TSHashTable<RegistryFileEntry,HASHKEY_STRI>::InternalClear
// 1.14d mac: 0003beb2 TSExplicitList<RegistryFileEntry,-572662307>::UnlinkAll
// 1.14d mac: 0003befe TSLink<RegistryFileEntry>::Unlink
// 1.14d mac: 0003bf3e TSLink<RegistryFileEntry>::NextLink
// 1.14d mac: 0003bf74 TSHashTable<WriteRegistryEntry,HASHKEY_STRI>::InternalClear
// 1.14d mac: 0003c030 TSExplicitList<WriteRegistryEntry,-572662307>::UnlinkAll
// 1.14d mac: 0003c07c TSLink<WriteRegistryEntry>::Unlink
// 1.14d mac: 0003c0bc TSLink<WriteRegistryEntry>::NextLink
// 1.14d mac: 0003c0f2 TSHashTable<WriteRegistryEntry,HASHKEY_STRI>::InternalNewNode
// 1.14d mac: 0003c176 TSHashTable<WriteRegistryEntry,HASHKEY_STRI>::InitializeTableSlots
// 1.14d mac: 0003c1e2 TSHashTable<WriteRegistryEntry,HASHKEY_STRI>::MonitorFullness
// 1.14d mac: 0003c2b6 TSExplicitList<WriteRegistryEntry,-572662307>::LinkNode
// 1.14d mac: 0003c338 TSHashTable<WriteRegistryEntry,HASHKEY_STRI>::GrowListArray
// 1.14d mac: 0003c4fc WriteRegistryEntry::~WriteRegistryEntry[D2]
// 1.14d mac: 0003c57c TSGrowableArray<TSExplicitList<WriteRegistryEntry,-572662307>>::SetCount
// 1.14d mac: 0003c628 TSGrowableArray<TSExplicitList<WriteRegistryEntry,-572662307>>::Reserve
// 1.14d mac: 0003c69a TSGrowableArray<TSExplicitList<WriteRegistryEntry,-572662307>>::ReallocData
// 1.14d mac: 0003c7e2 TSHashTable<WriteRegistryEntry,HASHKEY_STRI>::~TSHashTable[D2]
// 1.14d mac: 0003c85a TSHashTable<WriteRegistryEntry,HASHKEY_STRI>::InternalDelete
// 1.14d mac: 0003c89c TSHashTable<WriteRegistryEntry,HASHKEY_STRI>::InternalNew
// 1.14d mac: 0003c8c8 TSHashTable<WriteRegistryEntry,HASHKEY_STRI>::~TSHashTable[D0]
// 1.14d mac: 0003c8e2 TSHashTable<WriteRegistryEntry,HASHKEY_STRI>::Destroy
// 1.14d mac: 0003c930 TSGrowableArray<TSExplicitList<WriteRegistryEntry,-572662307>>::~TSGrowableArray[D2]
// 1.14d mac: 0003c9b0 TSExplicitList<WriteRegistryEntry,-572662307>::NewNode
// 1.14d mac: 0003ca3e TSHashTable<RegistryEntry,HASHKEY_STRI>::Unlink
// 1.14d mac: 0003ca68 TSLink<RegistryEntry>::Unlink
// 1.14d mac: 0003caa8 TSLink<RegistryEntry>::NextLink
// 1.14d mac: 0003cade RegistryEntry::~RegistryEntry[D2]
// 1.14d mac: 0003cbb8 TSHashTable<RegistryFileEntry,HASHKEY_STRI>::~TSHashTable[D2]
// 1.14d mac: 0003cc30 TSHashTable<RegistryFileEntry,HASHKEY_STRI>::InternalDelete
// 1.14d mac: 0003ccae TSHashTable<RegistryFileEntry,HASHKEY_STRI>::InternalNew
// 1.14d mac: 0003ccda TSHashTable<RegistryFileEntry,HASHKEY_STRI>::~TSHashTable[D0]
// 1.14d mac: 0003ccf4 TSHashTable<RegistryFileEntry,HASHKEY_STRI>::Destroy
// 1.14d mac: 0003cd42 TSGrowableArray<TSExplicitList<RegistryFileEntry,-572662307>>::~TSGrowableArray[D2]
// 1.14d mac: 0003cdc2 TSExplicitList<RegistryFileEntry,-572662307>::NewNode
// 1.14d mac: 0003ce4e TSExplicitList<RegistryFileEntry,-572662307>::LinkNode
// 1.14d mac: 0003ced0 TSHashTable<RegistryEntry,HASHKEY_STRI>::~TSHashTable[D2]
// 1.14d mac: 0003cf48 TSHashTable<RegistryEntry,HASHKEY_STRI>::InternalClear
// 1.14d mac: 0003d004 TSHashTable<RegistryEntry,HASHKEY_STRI>::InternalDelete
// 1.14d mac: 0003d046 TSHashTable<RegistryEntry,HASHKEY_STRI>::InternalNew
// 1.14d mac: 0003d072 TSHashTable<RegistryEntry,HASHKEY_STRI>::~TSHashTable[D0]
// 1.14d mac: 0003d08c TSHashTable<RegistryEntry,HASHKEY_STRI>::Destroy
// 1.14d mac: 0003d0da TSGrowableArray<TSExplicitList<RegistryEntry,-572662307>>::~TSGrowableArray[D2]
// 1.14d mac: 0003d15a TSExplicitList<RegistryEntry,-572662307>::NewNode
// 1.14d mac: 0003d1fc TSExplicitList<RegistryEntry,-572662307>::LinkNode
// 1.14d mac: 0003d27e TSExplicitList<RegistryEntry,-572662307>::UnlinkAll
// 1.14d mac: 0003d2ca TSHashTable<RegistryFileEntry,HASHKEY_STRI>::Ptr
// 1.14d mac: 0003d386 TSHashTable<RegistryFileEntry,HASHKEY_STRI>::InternalNewNode
// 1.14d mac: 0003d40a TSHashTable<RegistryFileEntry,HASHKEY_STRI>::InitializeTableSlots
// 1.14d mac: 0003d476 TSHashTable<RegistryFileEntry,HASHKEY_STRI>::MonitorFullness
// 1.14d mac: 0003d54a TSHashTable<RegistryFileEntry,HASHKEY_STRI>::GrowListArray
// 1.14d mac: 0003d70e TSGrowableArray<TSExplicitList<RegistryFileEntry,-572662307>>::SetCount
// 1.14d mac: 0003d7ba TSGrowableArray<TSExplicitList<RegistryFileEntry,-572662307>>::Reserve
// 1.14d mac: 0003d82c TSGrowableArray<TSExplicitList<RegistryFileEntry,-572662307>>::ReallocData
// 1.14d mac: 0003d974 TSGrowableArray<unsignedchar>::Reserve
// 1.14d mac: 0003d9e8 TSGrowableArray<unsignedchar>::ReallocData
// 1.14d mac: 0003dab2 TSHashTable<RegistryEntry,HASHKEY_STRI>::InternalNewNode
// 1.14d mac: 0003db36 TSHashTable<RegistryEntry,HASHKEY_STRI>::InitializeTableSlots
// 1.14d mac: 0003dba2 TSHashTable<RegistryEntry,HASHKEY_STRI>::MonitorFullness
// 1.14d mac: 0003dc76 TSHashTable<RegistryEntry,HASHKEY_STRI>::GrowListArray
// 1.14d mac: 0003de3a TSGrowableArray<TSExplicitList<RegistryEntry,-572662307>>::SetCount
// 1.14d mac: 0003dee6 TSGrowableArray<TSExplicitList<RegistryEntry,-572662307>>::Reserve
// 1.14d mac: 0003df58 TSGrowableArray<TSExplicitList<RegistryEntry,-572662307>>::ReallocData
// 1.14d mac: 0003e0a0 _GLOBAL__I_a

#endif  // D2_OS_MAC
