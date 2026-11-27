// StormMac Storm Mac/SOURCE/SERR.CPP: Storm's error reporting (the error dialog and log,
// message-source facilities, resource-leak reports), 1.14d Intel Mac 0000fec0..00011040.
//
// No __FILE__ of its own is used here; the file is pinned by the file map (SErrGetErrorStr
// ... SErrSuppressErrors) and by its static initialiser at 00011040, which ends it. The
// first functions, from SERR_SetDisplayErrorCallback, follow SCOMP.CPP's static
// initialiser (0000fe80).
#include "../Win32Mac/Source/Win32/Win32Mac.h"
#pragma GCC visibility push(hidden)
#include "../H/stpl.h"
#pragma GCC visibility pop

#if !defined(_WIN32) && D2_OS_MAC

#include <stdarg.h>
#include <stdio.h>
#include <string.h>

// ---- what this file calls ----

int SMemFree(void* pMem, const char* szFile, int nLine, uint32_t dwFlags);
void* SMsgGetDefaultWindow(void);
void* StormGetInstance(void);
void SLogFlushAll(void);
uint32_t SStrCopy(char* szDest, const char* szSource, int32_t nMaxChars);
uint32_t SStrLen(const char* szString);
char* SStrChrR(const char* szString, char cChar);
void SStrPack(char* szDest, const char* szSource, uint32_t nMaxChars);
int SStrPrintf(char* szDest, size_t nMaxChars, const char* szFormat, ...);
int SRegLoadValue(const char* szKey, const char* szValue, uint32_t dwFlags, int32_t* pnValue);

void SetLastError(DWORD dwErrCode);
DWORD GetLastError(void);
LONG InterlockedIncrement(LPLONG pnValue);
LONG InterlockedDecrement(LPLONG pnValue);
void* GetProcessHeap(void);
void* HeapAlloc(void* hHeap, DWORD dwFlags, size_t nBytes);
BOOL HeapFree(void* hHeap, DWORD dwFlags, void* pMem);
void InitializeCriticalSection(CRITICAL_SECTION* pSection);
void DeleteCriticalSection(CRITICAL_SECTION* pSection);
void EnterCriticalSection(CRITICAL_SECTION* pSection);
void LeaveCriticalSection(CRITICAL_SECTION* pSection);
BOOL GetModuleFileName(HANDLE hModule, char* szFilename, DWORD nSize);
HANDLE FindFirstFile(const char* szFileName, void* pFindData);
BOOL FindClose(HANDLE hFind);
void OutputDebugString(const char* szOutputString);
BOOL IsDebuggerPresent(void);
int IsWindow(void* hWnd);
BOOL IsWindowVisible(void* hWnd);
int MessageBox(void* hWnd, const char* szText, const char* szCaption, UINT nType);
void ExitProcess(UINT nExitCode);
int LoadString(void* hInstance, UINT nId, char* szBuffer, int nBufferMax);
int FormatMessage(DWORD dwFlags, const void* pSource, DWORD dwMessageId, DWORD dwLanguageId, char* szBuffer);
BOOL CreateDirectory(const char* szPath, void* pSecurity);
DWORD GetFileAttributes(const char* szFileName);
HANDLE CreateFile(const char* szFileName, DWORD dwAccess, DWORD dwShare, void* pSecurity, DWORD dwCreation,
                  DWORD dwFlags, HANDLE hTemplate);
BOOL WriteFile(HANDLE hFile, const void* pBuffer, size_t nBytes, int* pnWritten, void* pOverlapped);
void GetLocalTime(void* pSystemTime);
void LogComputerInfoHeader(uint32_t dwFlags, void (*pfnLog)(HANDLE, const char*, ...), void* pLogContext,
                           const char* szHeader, const uint16_t* pTime);
void STORM_Nop(void);
extern "C" void DebugStr(const unsigned char* pMessage);
// Out of line in SFILE.CPP's neighbourhood.
void CRITSECT_Init(pthread_mutex_t* pMutex);
// The destructor SFileCritSect::~SFileCritSect[D2] (SFILE.CPP, 00016a72), which the statics register with __cxa_atexit.
void CRITSECT_Destroy(pthread_mutex_t* pMutex) __attribute__((visibility("hidden"))) __asm__("__ZN13SFileCritSectD2Ev");

extern "C" int __cxa_atexit(void (*pfnDestructor)(void*), void* pObject, void* pDso);
extern "C" void* __dso_handle;

// STORM.CPP's option words: [1] non-zero when the debugger-only leak output is on,
// [5] non-zero when no error log is written.
extern uint32_t gnSSystemOption1;
extern BOOL gbSSystemShutdownInProgress;

// 1.14d mac: 003dfb14
// The application's extra-information writer for the error log, or null.
void (*g_pfnErrorLogExtraInfo)(char* szBuffer, uint32_t nBufferSize);

// 1.14d mac: 003cf71c
// The message-string table the resource strings fall back to.
const char* const gaSErrMessageStrings[12] = {
    "ERROR #%u (0x%08x)",
    "This application has encountered a critical error:\n\n%s\n",
    "Program:\t%s\n",
    "File:\t%s\nLine:\t%d\n",
    "Function:\t%s\n",
    "Object:\t%s\n",
    "Handle:\t%s\n",
    "Expr:\t%s\n\n",
    "\n%s\n\n",
    "Press OK to terminate the application.",
    "Do you wish to keep running anyway?\n\tOK:\tKeep running\n\tCancel:\tTerminate application",
    "File:\t%s\n",
};

typedef void (*pfnBlizzardError)(const char* szTitle, const char* szText, char** ppExtra, int nUnused, int nFlags,
                                 bool bFatal);
typedef int (*pfnSErrHandler)(uint32_t nErrorCode, const char* szErrorText, const char* szStackTrace,
                              int32_t nTraceFlags, const char* szCustomMsg);

// ---- the intrusive list the registered handlers live on ----

// The log mutex: made by the static initialiser, which registers its destructor straight from
// Storm's own CRITSECT_Destroy.
class SErrCritSect : public pthread_mutex_t {
public:
    SErrCritSect() {
        CRITSECT_Init(this);
        __cxa_atexit((void (*)(void*))CRITSECT_Destroy, this, &__dso_handle);
    }
};

// A registered error handler: the link, then the callback.
struct SErrHandlerNode : TSLinkedNode<SErrHandlerNode> {
    pfnSErrHandler pfnHandler;
};

// A message source: the facility number, its module and the next one.
struct SErrFacility {
    uint16_t wFacility;     // 0x00
    uint16_t wPad;          // 0x02
    void* hModule;          // 0x04
    SErrFacility* pNext;    // 0x08
};

// ---- the module's state ----

// 1.14d mac: 003f02ac
static TSExplicitList<SErrHandlerNode, 0> s_tHandlers;
// 1.14d mac: 003f02e4
static pfnBlizzardError s_pfnDisplayError;
// 1.14d mac: 003f02e8
static char s_bFacilitiesRegistered;
// 1.14d mac: 003f02ec
static SErrFacility* s_pFacilities;
// 1.14d mac: 003970c4
// -1 until the critical section is made; the first incrementer to reach 0 makes it.
static LONG s_nCriticalSectionState = -1;
// 1.14d mac: 003f02f0
static CRITICAL_SECTION s_tCriticalSection;
// 1.14d mac: 003f02b8
static SErrCritSect s_tLogMutex;
// 1.14d mac: 003f0328
static char s_bLeakOptionRead;
// 1.14d mac: 003f032c
static int32_t s_nLeakOption;
// 1.14d mac: 003f03b4
static char s_szStringBuffer[0x100];
// 1.14d mac: 003f02fc
BOOL gbSuppressErrors;
// 1.14d mac: 003f02f8
bool gbSSystemErrorDialogActive;
// 1.14d mac: 003f0300
DWORD gdwStormLastErrorCode;

// 1.14d mac: 0000fec0
void SERR_SetDisplayErrorCallback(pfnBlizzardError pfnCallback) {
    s_pfnDisplayError = pfnCallback;
}

// 1.14d mac: 0000fed5
BOOL SErrDestroy(void) {
    s_tHandlers.DeleteAll();
    s_bFacilitiesRegistered = 0;
    while (s_pFacilities != NULL) {
        SErrFacility* pNext = s_pFacilities->pNext;
        HeapFree(GetProcessHeap(), 0, s_pFacilities);
        s_pFacilities = pNext;
    }
    if (s_nCriticalSectionState != -1) {
        DeleteCriticalSection(&s_tCriticalSection);
        s_nCriticalSectionState = -1;
    }
    return 1;
}

// 1.14d mac: 0000ff5d
// Makes the critical section on first use, then enters it.
static void SERR_InternalEnterCriticalSection(void) {
    if (InterlockedIncrement(&s_nCriticalSectionState) == 0) {
        InitializeCriticalSection(&s_tCriticalSection);
    } else {
        InterlockedDecrement(&s_nCriticalSectionState);
    }
    EnterCriticalSection(&s_tCriticalSection);
}

uint32_t SErrGetErrorStr(uint32_t dwErrCode, char* szBuffer, size_t nBufferSize);

// 1.14d mac: 0001083c
// The resource string for an index, else the built-in text.
static const char* SERR_GetString(int32_t nErrorIndex) {
    void* hInstance = StormGetInstance();
    const char* szString = s_szStringBuffer;
    if (LoadString(hInstance, nErrorIndex + 0x5100, s_szStringBuffer, 0x100) == 0) {
        szString = gaSErrMessageStrings[nErrorIndex];
    }
    return szString;
}

// 1.14d mac: 0001088b
static void WriteErrorLog(const char* pszText);

// 1.14d mac: 0000ffaa
int SErrDisplayError(uint32_t nErrorCode, const char* pStackTrace, int32_t nTraceFlags, const char* pCustomMsg,
                     int32_t nMessageFlags, UINT nExitCode) {
    char szModuleFile[260];
    char szModuleName[260];
    char szErrorText[256];
    char szFullMessage[2048];
    struct {  // WIN32_FIND_DATA
        char abHeader[0x2c];
        char szFileName[260];
        char szAlternate[16];
    } tFindData;
    char szCallbackTitle[260];

    if (gbSuppressErrors != 0 || gbSSystemErrorDialogActive) {
        return 0;
    }
    gbSSystemErrorDialogActive = true;
    SERR_InternalEnterCriticalSection();
    SLogFlushAll();
    szModuleFile[0] = '\0';
    szModuleName[0] = '\0';
    GetModuleFileName(0, szModuleFile, 0x104);
    bzero(&tFindData, sizeof(tFindData));
    HANDLE hFind = FindFirstFile(szModuleFile, (char*)&tFindData);
    if (hFind != 0) {
        FindClose(hFind);
    }
    SStrCopy(szModuleName, tFindData.szFileName, 0x104);
    if (SStrChrR(szModuleName, '.') != NULL) {
        *SStrChrR(szModuleName, '.') = '\0';
    }
    char* pSlash = SStrChrR(szModuleFile, '\\');
    if (pSlash != NULL && pSlash[1] != '\0') {
        memmove(szModuleFile, pSlash + 1, strlen(pSlash + 1) + 1);
    }
    szErrorText[0] = '\0';
    SErrGetErrorStr(nErrorCode, szErrorText, 0x100);
    if (szErrorText[0] == '\0') {
        SStrPrintf(szErrorText, 0x100, SERR_GetString(0), nErrorCode & 0xffff, nErrorCode);
    }
    char* pCursor;
    bool bHasTrace;
    if (pStackTrace == NULL) {
        pCursor = szFullMessage;
        bHasTrace = false;
    } else {
        pCursor = szFullMessage + SStrCopy(szFullMessage, pStackTrace, 0x800);
        bHasTrace = true;
    }
    if (0 < nTraceFlags) {
        SStrPrintf(pCursor, szErrorText - pCursor, "(%u)", nTraceFlags);
        pCursor += SStrLen(pCursor);
    }
    SStrPrintf(pCursor, szErrorText - pCursor, " : error %lu: ", nErrorCode & 0xffff);
    uint32_t nLength = SStrLen(pCursor);
    SStrCopy(pCursor + nLength, szErrorText, (int32_t)(szErrorText - (pCursor + nLength)));
    OutputDebugString(szFullMessage);
    SStrPrintf(szFullMessage, 0x800, SERR_GetString(1), szErrorText);
    uint32_t nUsed = SStrLen(szFullMessage);
    SStrPrintf(szFullMessage + nUsed, 0x800 - nUsed, SERR_GetString(2), szModuleFile);
    int nOffset = SStrLen(szFullMessage + nUsed) + nUsed;
    pCursor = szFullMessage + nOffset;
    if (bHasTrace && *pStackTrace != '\0') {
        int32_t nStringIndex;
        const char* szFormat;
        switch (nTraceFlags) {
        case -5:
            nStringIndex = 0xe;
            szFormat = SERR_GetString(nStringIndex);
            break;
        case -4:
            nStringIndex = 0xb;
            szFormat = SERR_GetString(nStringIndex);
            break;
        case -3:
            nStringIndex = 6;
            szFormat = SERR_GetString(nStringIndex);
            break;
        case -2:
            nStringIndex = 5;
            szFormat = SERR_GetString(nStringIndex);
            break;
        case -1:
            nStringIndex = 4;
            szFormat = SERR_GetString(nStringIndex);
            break;
        default:
            szFormat = SERR_GetString(3);
            break;
        }
        SStrPrintf(pCursor, 0x800 - nOffset, szFormat, pStackTrace);
        pCursor = szFullMessage + SStrLen(pCursor) + nOffset;
    }
    const char* szCustom = "";
    int32_t nTailIndex;
    if (nErrorCode == 0x85100000) {
        if (pCustomMsg != NULL) {
            szCustom = pCustomMsg;
        }
        nTailIndex = 7;
    } else {
        if (pCustomMsg != NULL) {
            szCustom = pCustomMsg;
        }
        nTailIndex = 8;
    }
    SStrPrintf(pCursor, szErrorText - pCursor, SERR_GetString(nTailIndex), szCustom);
    uint32_t nTailLength = SStrLen(pCursor);
    if (s_pfnDisplayError != NULL) {
        const char* szTrace = "";
        if (bHasTrace) {
            szTrace = pStackTrace;
        }
        SStrPrintf(szCallbackTitle, 0x104, "%s %s:%i", szErrorText, szTrace, nTraceFlags);
        s_pfnDisplayError(szCallbackTitle, szFullMessage, NULL, 0, nTraceFlags, false);
    }
    pCursor += nTailLength;
    // the buttons: the debugger changes the prompt and the break result
    int nBreakResult;
    int nButton;
    int nMsgResult;
    uint32_t nIcon;
    uint32_t nStyle;
    if (!IsDebuggerPresent()) {
        if (nMessageFlags == 0) {
            SStrCopy(pCursor, SERR_GetString(9), (int32_t)(szErrorText - pCursor));
            nButton = 0;
            nMsgResult = 1;
            nIcon = 0x10;
            nStyle = 0x52000;
            nBreakResult = 0;
        } else {
            SStrCopy(pCursor, SERR_GetString(10), (int32_t)(szErrorText - pCursor));
            nBreakResult = 0;
            nButton = 1;
            nMsgResult = 2;
            nIcon = 0x30;
            nStyle = 0x52101;
        }
    } else if (nMessageFlags == 0) {
        SStrCopy(pCursor, SERR_GetString(0xc), (int32_t)(szErrorText - pCursor));
        nBreakResult = 6;
        nButton = 0;
        nMsgResult = 7;
        nIcon = 0x10;
        nStyle = 0x52104;
    } else {
        SStrCopy(pCursor, SERR_GetString(0xd), (int32_t)(szErrorText - pCursor));
        nBreakResult = 6;
        nButton = 2;
        nMsgResult = 7;
        nIcon = 0x30;
        nStyle = 0x52103;
    }
    if (nButton != 0) {
        nMsgResult = nButton;
    }
    SErrHandlerNode* pHandler = s_tHandlers.Head();
    bool bHandled = false;
    if (pHandler != NULL) {
        const char* szHandlerMsg = "";
        if (pCustomMsg != NULL) {
            szHandlerMsg = pCustomMsg;
        }
        do {
            if (pHandler->pfnHandler(nErrorCode, szErrorText, pStackTrace, nTraceFlags, szHandlerMsg) == 0) {
                bHandled = true;
                break;
            }
            pHandler = pHandler->m_link.m_next;
        } while (!TSLink<SErrHandlerNode>::IsTerminator(pHandler));
    }
    if (!bHandled && s_pfnDisplayError == NULL) {
        void* hWnd = SMsgGetDefaultWindow();
        if (hWnd == NULL || IsWindow(hWnd) == 0 || IsWindowVisible(hWnd) == 0) {
            hWnd = NULL;
        }
        nMsgResult = MessageBox(hWnd, szFullMessage, szModuleName, nStyle | nIcon);
    }
    LeaveCriticalSection(&s_tCriticalSection);
    if (gbSSystemShutdownInProgress == 0) {
        pthread_mutex_lock(&s_tLogMutex);
        pthread_mutex_unlock(&s_tLogMutex);
        *pCursor = '\0';
        WriteErrorLog(szFullMessage);
    }
    if (nMsgResult == nBreakResult) {
        DebugStr((const unsigned char*)"\pSErr break");
    } else if (nMsgResult != nButton) {
        gbSuppressErrors = 1;
        ExitProcess(nExitCode);
        gbSSystemErrorDialogActive = false;
        return 0;
    }
    gbSSystemErrorDialogActive = false;
    return 1;
}

// 1.14d mac: 00010d08
// Appends a message source to the facility list.
static void SERR_AddStormFacility(uint16_t wModuleId) {
    SErrFacility** ppTail = &s_pFacilities;
    while (*ppTail != NULL) {
        ppTail = &(*ppTail)->pNext;
    }
    SErrFacility* pNode = (SErrFacility*)HeapAlloc(GetProcessHeap(), 4, 0xc);
    *ppTail = pNode;
    pNode->wFacility = wModuleId;
    (*ppTail)->hModule = StormGetInstance();
    (*ppTail)->pNext = NULL;
}

// 1.14d mac: 0001071c
uint32_t SErrGetErrorStr(uint32_t dwErrCode, char* szBuffer, size_t nBufferSize) {
    if (nBufferSize == 0 || szBuffer == NULL) {
        gdwStormLastErrorCode = 0x57;
        SetLastError(0x57);
        return 0;
    }
    SERR_InternalEnterCriticalSection();
    if (s_bFacilitiesRegistered == 0) {
        s_bFacilitiesRegistered = 1;
        SERR_AddStormFacility(0x510);
        SERR_AddStormFacility(0x876);
        SERR_AddStormFacility(0x878);
    }
    SErrFacility** ppLink = &s_pFacilities;
    void* hModule;
    while (true) {
        SErrFacility* pFacility = *ppLink;
        hModule = NULL;
        if (pFacility == NULL) {
            break;
        }
        ppLink = &pFacility->pNext;
        if (pFacility->wFacility == ((dwErrCode >> 16) & 0xfff)) {
            hModule = pFacility->hModule;
            break;
        }
    }
    LeaveCriticalSection(&s_tCriticalSection);
    *szBuffer = '\0';
    if (dwErrCode == 8) {
        SStrPrintf(szBuffer, nBufferSize, "%s", "There is not enough free memory available to continue.");
        return 1;
    }
    DWORD dwFlags = 0x1000;
    if (hModule != NULL) {
        dwFlags = 0x800;
    }
    return FormatMessage(dwFlags, hModule, dwErrCode, 0x400, szBuffer) != 0;
}

// 1.14d mac: 00010d65
// A formatted line and CRLF to the log file.
void SERR_WriteLogLineF(HANDLE hFile, const char* szFormat, ...) {
    char szBuffer[2048];
    va_list tArgs;
    int nWritten;
    va_start(tArgs, szFormat);
    const char* szUsed = "";
    if (szFormat != NULL) {
        szUsed = szFormat;
    }
    vsnprintf(szBuffer, 0x7fd, szUsed, tArgs);
    va_end(tArgs);
    szBuffer[0x7fd] = 0;
    SStrPack(szBuffer, "\r\n", 0x800);
    WriteFile(hFile, szBuffer, strlen(szBuffer), &nWritten, 0);
}

// 1.14d mac: 00010e19
// CRLF, then the text with every bare LF made a CRLF.
static void SERR_WriteLogTextCRLF(HANDLE hFile, const char* szText) {
    size_t nSize = strlen(szText);
    char szBuffer[nSize * 2 + 2];
    int nWritten;
    int nLength = 2;
    szBuffer[0] = '\r';
    szBuffer[1] = '\n';
    char cPrev = '\0';
    for (; nSize != 0; nSize--) {
        char cChar = *szText;
        if (cChar == '\n' && cPrev != '\r') {
            szBuffer[nLength++] = '\r';
        }
        szBuffer[nLength++] = cChar;
        szText++;
        cPrev = cChar;
    }
    szBuffer[nLength] = '\0';
    WriteFile(hFile, szBuffer, nLength, &nWritten, 0);
}

// 1.14d mac: 0001088b
// "Errors\<date> Error.txt" beside the executable, or in the executable's own folder when
// that cannot be made.
static void WriteErrorLog(const char* pszText) {
    char szModulePath[260];
    char szPath[260];
    char szFileName[260];
    uint16_t aLocalTime[8];
    GetLocalTime(aLocalTime);
    GetModuleFileName(0, szModulePath, 0x104);
    SStrCopy(szPath, szModulePath, 0x104);
    char* pEnd = SStrChrR(szPath, '\\');
    if (pEnd == NULL) {
        pEnd = szPath + SStrLen(szPath);
    } else {
        pEnd[1] = '\0';
    }
    SStrPack(szPath, "Errors\\", 0x104);
    if (!CreateDirectory(szPath, NULL)) {
        DWORD dwError = GetLastError();
        bool bUsable = false;
        if (dwError == 5 || dwError == 0xb7) {
            DWORD dwAttributes = GetFileAttributes(szPath);
            bUsable = dwAttributes != 0xffffffff && (dwAttributes & 0x10) != 0;
        }
        if (!bUsable) {
            pEnd[1] = '\0';
        }
    }
    sprintf(szFileName, "%04d-%02d-%02d %02d.%02d.%02d %s.txt", aLocalTime[0], aLocalTime[1], aLocalTime[3],
            aLocalTime[4], aLocalTime[5], aLocalTime[6], "Error");
    SStrPack(szPath, szFileName, 0x104);
    HANDLE hFile = CreateFile(szPath, 0x40000000, 3, 0, 2, 0x80, 0);
    if (hFile != 0) {
        LogComputerInfoHeader(1, SERR_WriteLogLineF, (void*)hFile, "Error", aLocalTime);
        SERR_WriteLogTextCRLF(hFile, pszText);
        if (g_pfnErrorLogExtraInfo != NULL) {
            // a run-time sized array: 1.14d moves esp down 0x1450 here (lea esi, [ebx - 0x1450]; mov esp, esi)
            uint32_t nExtraInfoSize = 0x1450;
            char szExtraInfo[nExtraInfoSize];
            szExtraInfo[0] = '\0';
            g_pfnErrorLogExtraInfo(szExtraInfo, nExtraInfoSize);
            if (szExtraInfo[0] != '\0') {
                SERR_WriteLogTextCRLF(hFile, szExtraInfo);
            }
        }
        STORM_Nop();
        CloseHandle(hFile);
    }
}

// 1.14d mac: 00010b81
void SErrSuppressErrors(BOOL bSuppress) {
    gbSuppressErrors = bSuppress;
}

// 1.14d mac: 00010b98
void SErrSetLastError(DWORD dwErrCode) {
    gdwStormLastErrorCode = dwErrCode;
    SetLastError(dwErrCode);
}

void SErrReportNamedResourceLeak(const char* szMessage, const char* szText);

// 1.14d mac: 00010bb3
void SErrReportResourceLeak(const char* szMessage) {
    SErrReportNamedResourceLeak(szMessage, NULL);
}

// 1.14d mac: 00010bd1
// "%s (%s)": raised as an error when the "Debug Memory" option is on, else just logged.
void SErrReportNamedResourceLeak(const char* szMessage, const char* szText) {
    char szDescription[256];
    char szLine[200];
    if (s_bLeakOptionRead == '\0') {
        s_bLeakOptionRead = 1;
        SRegLoadValue("Internal", "Debug Memory", 0, &s_nLeakOption);
    }
    if (s_nLeakOption != 0) {
        if (szText == NULL || *szText == '\0') {
            SStrPrintf(szDescription, 0x100, "%s", szMessage);
        } else {
            SStrPrintf(szDescription, 0x100, "%s (%s)", szMessage, szText);
        }
        if (gnSSystemOption1 == 0) {
            SErrDisplayError(0x8510007f, szDescription, -3, NULL, 1, 1);
        } else {
            SStrPrintf(szLine, 200, "Storm Error : handle never released -- %s\n", szDescription);
            OutputDebugString(szLine);
        }
    }
}

template void TSLink<SErrHandlerNode>::Unlink();
template TSLink<SErrHandlerNode>* TSLink<SErrHandlerNode>::NextLink(int);
template void TSExplicitList<SErrHandlerNode, 0>::UnlinkAll();
template void TSExplicitList<SErrHandlerNode, 0>::DeleteAll();
template TSExplicitList<SErrHandlerNode, 0>::~TSExplicitList();

// 1.14d mac: 00010eca TSExplicitList<SErrHandlerNode,0>::~TSExplicitList[D2]
// 1.14d mac: 00010eec TSExplicitList<SErrHandlerNode,0>::DeleteAll
// 1.14d mac: 00010f74 TSLink<SErrHandlerNode>::Unlink
// 1.14d mac: 00010fb4 TSLink<SErrHandlerNode>::NextLink
// 1.14d mac: 00010fea TSExplicitList<SErrHandlerNode,0>::UnlinkAll
// 1.14d mac: 00011040 _GLOBAL__I_a

#endif  // D2_OS_MAC
