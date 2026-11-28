// Storm's SLog, 1.14d Intel Mac build (DiabloII_macho): the log files behind HSLOG handles.
// Band 00017c32..00018b0b. The original is SLOG.CPP (the Windows build's assert text names
// it); the Windows twin is Source/Storm/Source/SLog.cpp, from which this differs by the
// Win32-on-Mac layer under it, a log file name with a Mac volume prefix, and the record
// carrying its file name (so a log can be opened late).
//
// A log is a record in one of four lists (slot = handle & 3), each behind its own critical
// section; LockLog returns it with the slot's section held. Text is built in the record's
// 64 KiB buffer and written when it passes 0xC000 bytes.
#include "Source/StormMac/Win32Mac/Source/Win32/Win32Mac.h"

#if !defined(_WIN32) && D2_MAC_CARBON

#include "Source/StormMac/H/StormApi.h"

#include <stdarg.h>
#include <stdio.h>
#include <string.h>

// ---- the Win32 layer and Storm entry points this file calls ----
// 1.14d mac: 0002af3a
void EnterCriticalSection(LPCRITICAL_SECTION pCriticalSection);
// 1.14d mac: 0002af43
void LeaveCriticalSection(LPCRITICAL_SECTION pCriticalSection);
// 1.14d mac: 0002aeda
void InitializeCriticalSection(LPCRITICAL_SECTION pCriticalSection);
// 1.14d mac: 0002af4c
void DeleteCriticalSection(LPCRITICAL_SECTION pCriticalSection);
// 1.14d mac: 00028cab
LPVOID VirtualAlloc(LPVOID lpAddress, size_t dwSize, DWORD flAllocationType, DWORD flProtect);
// 1.14d mac: 00028d2e
BOOL VirtualFree(LPVOID lpAddress, size_t dwSize, DWORD dwFreeType);
// 1.14d mac: 00021fc6
BOOL WriteFile(HANDLE hFile, LPCVOID lpBuffer, DWORD nNumberOfBytesToWrite, LPDWORD lpNumberOfBytesWritten,
               OVERLAPPED* lpOverlapped);
// 1.14d mac: 00022082
BOOL FlushFileBuffers(HANDLE hFile);
// 1.14d mac: 00021ad7
HANDLE CreateFile(LPCSTR lpFileName, DWORD dwDesiredAccess, DWORD dwShareMode, LPVOID lpSecurityAttributes,
                  DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes, HANDLE hTemplateFile);
// 1.14d mac: 000220ca
DWORD SetFilePointer(HANDLE hFile, LONG lDistanceToMove, LPLONG lpDistanceToMoveHigh, DWORD dwMoveMethod);
// 1.14d mac: 000226f8
BOOL CreateDirectory(char* lpPathName, LPVOID lpSecurityAttributes);
// 1.14d mac: 0002a16f
HANDLE GetModuleHandle(LPCSTR szModuleName);
// 1.14d mac: 0002a0ea
DWORD GetModuleFileName(HANDLE hModule, char* szFileName, DWORD nSize);
// 1.14d mac: 0002d178
void OutputDebugString(LPCSTR szOutputString);
// 1.14d mac: 00010bb3
void SErrReportResourceLeak(const char* szType);

// 1.14d mac: 0002c392
void GetLocalTime(SYSTEMTIME* lpSystemTime);

#define GENERIC_WRITE 0x40000000
#define FILE_SHARE_READ 0x1
#define FILE_SHARE_WRITE 0x2
#define CREATE_ALWAYS 2
#define OPEN_ALWAYS 4
#define FILE_ATTRIBUTE_NORMAL 0x80
#define FILE_END 2
#define ERROR_INVALID_PARAMETER 0x57
#define MAX_PATH 260

#define SLOG_SLOTS 4
#define SLOG_BUFFER_SIZE 0x10000
#define SLOG_FLUSH_THRESHOLD 0xC000
#define SLOG_FLAG_OPEN_NOW 0x1
#define SLOG_FLAG_NO_NAME 0x2
#define SLOG_FLAG_APPEND 0x4

// One log, 0x10124 bytes (VirtualAlloc'd by LockLog).
struct SLogRecord {
    uint32_t nHandle;                   // 0x000  slot = nHandle & 3
    SLogRecord* pNext;                  // 0x004  next record in the slot's list
    char szFileName[MAX_PATH];          // 0x008
    HANDLE hFile;                       // 0x10C  0 until the file is open
    uint32_t dwFlags;                   // 0x110  SLOG_FLAG_*
    uint32_t nUsed;                     // 0x114  bytes in acBuffer
    uint32_t nLineStart;                // 0x118  start of the current entry (OutputDebugString from here)
    int32_t nIndent;                    // 0x11C
    BOOL bTimestamps;                   // 0x120
    char acBuffer[SLOG_BUFFER_SIZE];    // 0x124
};
typedef char SLogRecord_size[(sizeof(SLogRecord) == 0x10124) ? 1 : -1];

// 1.14d mac: 003f06fc
static char s_bInitialized;
// 1.14d mac: 003f0700
static uint32_t s_nLastHandle;
// 1.14d mac: 003f0704
static CRITICAL_SECTION s_aCritSect[SLOG_SLOTS];
// 1.14d mac: 003f07b4
static SLogRecord* s_apHead[SLOG_SLOTS];
// 1.14d mac: 003f07c4
static CRITICAL_SECTION s_tDirCritSect;
// 1.14d mac: 003f07f0
// The directory a bare log file name goes in; empty: the executable's.
static char s_szLogDirectory[MAX_PATH];
// 1.14d mac: 003f08f4
static char s_szTimeStamp[64];
// 1.14d mac: 003f0934
static uint32_t s_nTimeStampLength;
// 1.14d mac: 003f0938
static uint32_t s_dwLastTick;
// 1.14d mac: 003f093c
uint32_t g_dwLogSetting1;
// 1.14d mac: 003f0940
uint32_t g_dwLogSetting2;

// 1.14d mac: 00554e08
// Nonzero: log text also goes to OutputDebugString.
extern uint32_t gbCompressionDebugOutput;

static SLogRecord* LockLog(uint32_t nHandle, uint32_t* pnSlotOut, int32_t bAllocate);
static void SLOG_FlushLog(SLogRecord* pLog);
static BOOL OpenLogFile(const char* szFileName, HANDLE* pnFileHandle, uint32_t dwFlags);
static BOOL SLOG_EnsureFileOpen(SLogRecord* pLog);
static void SLOG_OutputTime(SLogRecord* pLog, int32_t bHasData);
static void SLOG_WriteIndentSpaces(SLogRecord* pLog);
static int STORM_GetMacPathPrefixLength(char* szPath);

// 1.14d mac: 00017c32
void Storm::Source::SLog::SLogClose(uint32_t nHandle) {
    if (s_bInitialized == 1) {
        uint32_t nSlot;
        SLogRecord* pLog = LockLog(nHandle, &nSlot, 0);
        if (pLog) {
            if (pLog->hFile) {
                SLOG_FlushLog(pLog);
                CloseHandle(pLog->hFile);
            }
            SLogRecord** ppLink = &s_apHead[nSlot];
            SLogRecord* pCur;
            while ((pCur = *ppLink) != NULL) {
                if (pCur == pLog) {
                    *ppLink = pCur->pNext;
                    VirtualFree(pLog, 0, MEM_RELEASE);
                    break;
                }
                ppLink = &pCur->pNext;
            }
            LeaveCriticalSection(&s_aCritSect[nSlot]);
        }
    }
}

// 1.14d mac: 00017ccf
static SLogRecord* LockLog(uint32_t nHandle, uint32_t* pnSlotOut, int32_t bAllocate) {
    if (nHandle == 0) {
        *pnSlotOut = 0xFFFFFFFF;
        return NULL;
    }
    uint32_t nSlot = nHandle & 3;
    EnterCriticalSection(&s_aCritSect[nSlot]);
    *pnSlotOut = nSlot;
    SLogRecord** ppNext = &s_apHead[nSlot];
    SLogRecord* pLog;
    while ((pLog = *ppNext) != NULL) {
        if (pLog->nHandle == nHandle) {
            return pLog;
        }
        ppNext = &pLog->pNext;
    }
    if (bAllocate) {
        pLog = (SLogRecord*)VirtualAlloc(NULL, sizeof(SLogRecord), MEM_COMMIT, PAGE_READWRITE);
        *ppNext = pLog;
        if (pLog) {
            pLog->nHandle = nHandle;
            pLog->pNext = NULL;
            pLog->szFileName[0] = '\0';
            pLog->hFile = 0;
            pLog->nLineStart = 0;
            pLog->nUsed = 0;
            return pLog;
        }
    }
    LeaveCriticalSection(&s_aCritSect[nSlot]);
    *pnSlotOut = 0xFFFFFFFF;
    return NULL;
}

// 1.14d mac: 00017da7
static void SLOG_FlushLog(SLogRecord* pLog) {
    DWORD dwBytesWritten;
    if (pLog->nUsed) {
        WriteFile(pLog->hFile, pLog->acBuffer, pLog->nUsed, &dwBytesWritten, NULL);
        FlushFileBuffers(pLog->hFile);
        pLog->nUsed = 0;
        pLog->nLineStart = 0;
    }
}

// 1.14d mac: 00017e0d
BOOL Storm::Source::SLog::SLogCreate(const char* szFileName, uint32_t dwFlags, int* pLogHandle) {
    bool bEmpty;
    if (!pLogHandle || (bEmpty = *szFileName == '\0', *pLogHandle = 0, bEmpty) || !szFileName) {
        SErrSetLastError(ERROR_INVALID_PARAMETER);
        return 0;
    }
    const char* szName = "";
    uint32_t dwUsedFlags = dwFlags & ~SLOG_FLAG_OPEN_NOW;
    if (!(dwFlags & SLOG_FLAG_NO_NAME)) {
        szName = szFileName;
        dwUsedFlags = dwFlags;
    }
    HANDLE hFile = 0;
    if ((dwUsedFlags & SLOG_FLAG_OPEN_NOW) && !OpenLogFile(szName, &hFile, dwUsedFlags)) {
        return 0;
    }
    uint32_t nHandle = ++s_nLastHandle;
    *pLogHandle = nHandle;
    uint32_t nSlot;
    SLogRecord* pLog = LockLog(nHandle, &nSlot, 1);
    if (!pLog) {
        *pLogHandle = 0;
        return 0;
    }
    pLog->hFile = hFile;
    SStrCopy(pLog->szFileName, szName, MAX_PATH);
    pLog->dwFlags = dwUsedFlags;
    pLog->bTimestamps = 1;
    pLog->nIndent = 0;
    LeaveCriticalSection(&s_aCritSect[nSlot]);
    return 1;
}

// 1.14d mac: 00017f20
// A bare name goes under the log directory or the executable's; every directory on the path
// is created; the file is created, or with SLOG_FLAG_APPEND opened and positioned at its end.
static BOOL OpenLogFile(const char* szFileName, HANDLE* pnFileHandle, uint32_t dwFlags) {
    char szFullPath[MAX_PATH];
    char szDirPath[MAX_PATH];
    if (!szFileName || *szFileName == '\0') {
        *pnFileHandle = 0;
        return 0;
    }
    if (szFileName[1] != ':' && !SStrChr(szFileName, '\\')) {
        EnterCriticalSection(&s_tDirCritSect);
        if (s_szLogDirectory[0] == '\0') {
            GetModuleFileName(GetModuleHandle(NULL), szFullPath, MAX_PATH);
            char* pSlash = SStrChrR(szFullPath, '\\');
            if (pSlash) {
                *pSlash = '\0';
            }
            SStrPack(szFullPath, "\\", MAX_PATH);
            SStrPack(szFullPath, szFileName, MAX_PATH);
        } else {
            uint32_t nLen = SStrCopy(szFullPath, s_szLogDirectory, MAX_PATH);
            SStrCopy(szFullPath + nLen, szFileName, MAX_PATH - nLen);
        }
        LeaveCriticalSection(&s_tDirCritSect);
        szFileName = szFullPath;
    }
    // New files are text files of Storm's creator while the log is created.
    DWORD dwSavedCreator = g_dwNewFileCreator;
    DWORD dwSavedType = g_dwNewFileType;
    g_dwNewFileCreator = 'ttxt';
    g_dwNewFileType = 'ttro';
    if (!szFileName) {
        SErrSetLastError(ERROR_INVALID_PARAMETER);
    } else {
        SStrCopy(szDirPath, szFileName, MAX_PATH);
        char* pEnd = SStrChrR(szDirPath, '\\');
        char* pAltEnd = SStrChrR(szDirPath, '/');
        if (pEnd < pAltEnd) {
            pEnd = pAltEnd;
        }
        if (pEnd) {
            int nPrefix = STORM_GetMacPathPrefixLength(szDirPath);
            if (pEnd < szDirPath + nPrefix) {
                szDirPath[nPrefix] = '\0';
            } else {
                pEnd[1] = '\0';
            }
        }
        int nPrefix = STORM_GetMacPathPrefixLength(szDirPath);
        for (char* p = SStrChr(szDirPath + nPrefix, '/'); p; p = SStrChr(p + 1, '/')) {
            *p = '\\';
        }
        for (char* p = SStrChr(szDirPath + nPrefix, '\\'); p; p = SStrChr(p + 1, '\\')) {
            *p = '\0';
            CreateDirectory(szDirPath, NULL);
            *p = '\\';
        }
        CreateDirectory(szDirPath, NULL);
    }
    HANDLE hFile = CreateFile(szFileName, GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL,
                              ((dwFlags & SLOG_FLAG_APPEND) >> 1) + CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
    *pnFileHandle = hFile;
    BOOL bOpened = hFile != 0;
    if ((dwFlags & SLOG_FLAG_APPEND) && bOpened) {
        SetFilePointer(hFile, 0, NULL, FILE_END);
    }
    g_dwNewFileType = dwSavedType;
    g_dwNewFileCreator = dwSavedCreator;
    return bOpened;
}

// 1.14d mac: 0001829f
void SLogDestroy(void) {
    SLogFlushAll();
    for (int nSlot = 0; nSlot < SLOG_SLOTS; nSlot++) {
        EnterCriticalSection(&s_aCritSect[nSlot]);
        while (s_apHead[nSlot]) {
            SLogClose(s_apHead[nSlot]->nHandle);
            SErrReportResourceLeak("HSLOG");
        }
        LeaveCriticalSection(&s_aCritSect[nSlot]);
    }
    s_bInitialized = 0;
    for (int nSlot = 0; nSlot < SLOG_SLOTS; nSlot++) {
        DeleteCriticalSection(&s_aCritSect[nSlot]);
    }
    DeleteCriticalSection(&s_tDirCritSect);
}

// 1.14d mac: 00018339
void Storm::Source::SLog::SLogFlushAll(void) {
    for (int nSlot = 0; nSlot < SLOG_SLOTS; nSlot++) {
        EnterCriticalSection(&s_aCritSect[nSlot]);
        for (SLogRecord* pLog = s_apHead[nSlot]; pLog; pLog = pLog->pNext) {
            if (pLog->hFile) {
                SLOG_FlushLog(pLog);
            }
        }
        LeaveCriticalSection(&s_aCritSect[nSlot]);
    }
}

// 1.14d mac: 0001839a
void SLogDump(uint32_t nHandle, const uint8_t* pData, uint32_t nLength) {
    uint32_t nSlot;
    SLogRecord* pLog = LockLog(nHandle, &nSlot, 0);
    if (!pLog) {
        return;
    }
    if (SLOG_EnsureFileOpen(pLog)) {
        for (uint32_t nRow = 0; nRow < nLength; nRow += 8) {
            SLOG_OutputTime(pLog, 0);
            SLOG_WriteIndentSpaces(pLog);
            sprintf(pLog->acBuffer + pLog->nUsed, "%04lx: ", (unsigned long)nRow);
            pLog->nUsed += SStrLen(pLog->acBuffer + pLog->nUsed);
            for (uint32_t nCol = nRow; nCol < nRow + 8; nCol++) {
                if (nCol < nLength) {
                    sprintf(pLog->acBuffer + pLog->nUsed, "%02x ", (unsigned)pData[nCol]);
                    pLog->nUsed += SStrLen(pLog->acBuffer + pLog->nUsed);
                } else {
                    pLog->nUsed += SStrCopy(pLog->acBuffer + pLog->nUsed, "   ", 0x7FFFFFFF);
                }
                if ((nCol & 3) == 3) {
                    pLog->nUsed += SStrCopy(pLog->acBuffer + pLog->nUsed, "  ", 0x7FFFFFFF);
                }
            }
            for (uint32_t nCol = nRow; nCol < nRow + 8; nCol++) {
                char cByte = 0;
                if (nCol < nLength) {
                    cByte = pData[nCol];
                    if (cByte >= ' ' && cByte != 0x7F) {
                        pLog->acBuffer[pLog->nUsed] = cByte;
                        pLog->acBuffer[pLog->nUsed + 1] = '\0';
                        pLog->nUsed += SStrLen(pLog->acBuffer + pLog->nUsed);
                        goto printed;
                    }
                }
                pLog->nUsed += SStrCopy(pLog->acBuffer + pLog->nUsed, cByte != 0 ? "." : " ", 0x7FFFFFFF);
            printed:
                if ((nCol & 7) == 3) {
                    pLog->nUsed += SStrCopy(pLog->acBuffer + pLog->nUsed, " ", 0x7FFFFFFF);
                }
            }
            pLog->acBuffer[pLog->nUsed + 2] = '\0';
            pLog->acBuffer[pLog->nUsed] = '\r';
            pLog->acBuffer[pLog->nUsed + 1] = '\n';
            pLog->nUsed += 2;
        }
        if (gbCompressionDebugOutput) {
            OutputDebugString(pLog->acBuffer + pLog->nLineStart);
        }
        pLog->nLineStart = pLog->nUsed;
        if (pLog->nUsed > SLOG_FLUSH_THRESHOLD - 1) {
            SLOG_FlushLog(pLog);
        }
    }
    LeaveCriticalSection(&s_aCritSect[nSlot]);
}

// 1.14d mac: 00018659
// Opens the record's file when it has none; failing, the record forgets its name.
static BOOL SLOG_EnsureFileOpen(SLogRecord* pLog) {
    BOOL bResult = 1;
    if (pLog->hFile == 0) {
        if (!OpenLogFile(pLog->szFileName, &pLog->hFile, pLog->dwFlags)) {
            pLog->szFileName[0] = '\0';
            bResult = 0;
        }
    }
    return bResult;
}

// 1.14d mac: 0001869c
static void SLOG_OutputTime(SLogRecord* pLog, int32_t bHasData) {
    if (pLog->bTimestamps) {
        DWORD dwTick = GetTickCount();
        if (dwTick != s_dwLastTick) {
            s_dwLastTick = dwTick;
            SYSTEMTIME tTime;
            GetLocalTime(&tTime);
            sprintf(s_szTimeStamp, "%u/%u %02u:%02u:%02u.%03u  ", (unsigned)tTime.wMonth, (unsigned)tTime.wDay,
                    (unsigned)tTime.wHour, (unsigned)tTime.wMinute, (unsigned)tTime.wSecond,
                    (unsigned)tTime.wMilliseconds);
            s_nTimeStampLength = SStrLen(s_szTimeStamp);
        }
        char* pOut = pLog->acBuffer + pLog->nUsed;
        if (bHasData) {
            SMemCopy(pOut, s_szTimeStamp, s_nTimeStampLength + 1);
        } else {
            memset(pOut, ' ', s_nTimeStampLength);
            pLog->acBuffer[pLog->nUsed + s_nTimeStampLength] = '\0';
        }
        pLog->nUsed += s_nTimeStampLength;
    }
}

// 1.14d mac: 000187b1
static void SLOG_WriteIndentSpaces(SLogRecord* pLog) {
    int32_t nIndent = pLog->nIndent;
    if (nIndent > 0) {
        int32_t nSpaces = nIndent >= 0x81 ? 0x80 : nIndent;
        SMemFill(pLog->acBuffer + pLog->nUsed, nSpaces, ' ');
        pLog->acBuffer[pLog->nUsed + nSpaces] = '\0';
        pLog->nUsed += nSpaces;
    }
}

// 1.14d mac: 00018810
void Storm::Source::SLog::SLogInitialize(void) {
    if (!s_bInitialized) {
        for (int nSlot = 0; nSlot < SLOG_SLOTS; nSlot++) {
            InitializeCriticalSection(&s_aCritSect[nSlot]);
        }
        InitializeCriticalSection(&s_tDirCritSect);
        s_bInitialized = 1;
    }
}

// 1.14d mac: 00018860
BOOL SLogIsInitialized(void) {
    return s_bInitialized;
}

// 1.14d mac: 00018872
// Sets the indent; returns the old one.
uint32_t SLogSetAbsIndent(uint32_t nHandle, uint32_t nIndent) {
    uint32_t nSlot;
    SLogRecord* pLog = LockLog(nHandle, &nSlot, 0);
    uint32_t nOld = 0;
    if (pLog) {
        nOld = pLog->nIndent;
        pLog->nIndent = nIndent;
        LeaveCriticalSection(&s_aCritSect[nSlot]);
    }
    return nOld;
}

// 1.14d mac: 000188c7
// Moves the indent by a delta; returns the old one.
uint32_t Storm::Source::SLog::SLogSetIndent(uint32_t nHandle, int32_t nDelta) {
    uint32_t nSlot;
    SLogRecord* pLog = LockLog(nHandle, &nSlot, 0);
    uint32_t nOld = 0;
    if (pLog) {
        nOld = pLog->nIndent;
        pLog->nIndent = nDelta + nOld;
        LeaveCriticalSection(&s_aCritSect[nSlot]);
    }
    return nOld;
}

// 1.14d mac: 0001891e
void SLogSetTimestamp(uint32_t nHandle, uint32_t bTimestamps) {
    uint32_t nSlot;
    SLogRecord* pLog = LockLog(nHandle, &nSlot, 0);
    if (pLog) {
        pLog->bTimestamps = bTimestamps;
        LeaveCriticalSection(&s_aCritSect[nSlot]);
    }
}

// 1.14d mac: 00018965
void SLogPend(uint32_t nHandle, const char* szFormat, va_list pArgList) {
    uint32_t nSlot;
    SLogRecord* pLog = LockLog(nHandle, &nSlot, 0);
    if (!pLog) {
        return;
    }
    if (SLOG_EnsureFileOpen(pLog)) {
        SLOG_OutputTime(pLog, 1);
        SLOG_WriteIndentSpaces(pLog);
        vsprintf(pLog->acBuffer + pLog->nUsed, szFormat, pArgList);
        pLog->nUsed += SStrLen(pLog->acBuffer + pLog->nUsed);
        pLog->acBuffer[pLog->nUsed + 2] = '\0';
        pLog->acBuffer[pLog->nUsed] = '\r';
        pLog->acBuffer[pLog->nUsed + 1] = '\n';
        pLog->nUsed += 2;
        if (gbCompressionDebugOutput) {
            OutputDebugString(pLog->acBuffer + pLog->nLineStart);
        }
        pLog->nLineStart = pLog->nUsed;
        if (pLog->nUsed > SLOG_FLUSH_THRESHOLD - 1) {
            SLOG_FlushLog(pLog);
        }
    }
    LeaveCriticalSection(&s_aCritSect[nSlot]);
}

// 1.14d mac: 00018a6f
void Storm::Source::SLog::SLogWrite(uint32_t nHandle, const char* szFormat, ...) {
    va_list pArgs;
    va_start(pArgs, szFormat);
    SLogPend(nHandle, szFormat, pArgs);
    va_end(pArgs);
}

// 1.14d mac: 00018a96
// The length of a leading "\{MacPathNN}\" token (to and with the separator after it), else 0.
static int STORM_GetMacPathPrefixLength(char* szPath) {
    if (SStrCmpI(szPath, "\\{MacPath", 8) == 0) {
        return SStrChr(szPath, '}') + 2 - szPath;
    }
    return 0;
}

// 1.14d mac: 00018ae2
// Sets either of two logging settings; a null pointer leaves it as it is.
void SLogSetSettings(const uint32_t* pdwSetting1, const uint32_t* pdwSetting2) {
    if (pdwSetting1) {
        g_dwLogSetting1 = *pdwSetting1;
    }
    if (pdwSetting2) {
        g_dwLogSetting2 = *pdwSetting2;
    }
}

#endif  // D2_OS_MAC
