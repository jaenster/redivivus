// Storm Mac's SFILE.CPP: the MPQ archive and file layer with its asynchronous reader (a
// thread that serves CD reads and the DirectSound streams), 1.14d Intel Mac build
// (DiabloII_macho). __FILE__ is ".../StormMac/Storm Mac/SOURCE/SFILE.CPP" (the SMemAlloc
// calls). The Windows Storm's SFile.cpp is the same code on Win32 types.
//
// Records and lists: every record (archive, file, stream, request) starts with its list
// link, so the lists' link offset is 0. All of it runs under one lock (s_tLock).
#include "Source/StormMac/SOURCE/SFILE.h"

#if !defined(_WIN32) && D2_OS_MAC

#include <ctype.h>
#include <pthread.h>
#include <stdio.h>
#include <string.h>

#define SFILE_FILE "/Users/bclemetson/dev/diablo2/Diablo2/Source/DiabloAll/../../../StormMac/Storm Mac/SOURCE/SFILE.CPP"
#define SFILE_STATIC static __attribute__((noinline))

// A critical section: a recursive pthread mutex.
class SFileCritSect {
public:
    pthread_mutex_t m_tMutex;

    SFileCritSect();
    ~SFileCritSect();
};

template<class T>
class SFileList : public TSExplicitList<T, 0> {};

// The statics, in the order the file's static initialiser builds them (00017700).
// 1.14d mac: 003f0500
static TSExplicitList<D2ArchiveStrc, 0> s_tArchiveList;
// 1.14d mac: 003f050c
static TSExplicitList<D2StormAsyncOpStrc, 0> s_tRequestQueue;
// 1.14d mac: 003f0518
static SFileCritSect s_tLock;
// 1.14d mac: 003f0544
static TSExplicitList<D2MPQFileStrc, 0> s_tFileList;
// 1.14d mac: 003f0550
// Requests that only carry a completion event: cancelling a request leaves one behind to signal it.
static TSExplicitList<D2StormAsyncOpStrc, 0> s_tCompletionList;
// 1.14d mac: 003f055c
static TSExplicitList<D2StormAudioStreamRec, 0> s_tStreamList;
// 1.14d mac: 003f0568
static SFileCritSect s_tQueueLock;

// 1.14d mac: 003f059c
static void* s_pDirectSound;
// 1.14d mac: 003f05a0
static uint32_t s_dwDirectAccessFlags;
// 1.14d mac: 003f05a8
static void* s_pWaveBuffer;
// 1.14d mac: 003f05ac
static HANDLE s_hCdEvent;
// 1.14d mac: 003f05b0
static uint8_t s_bCdThreadDraining;
// 1.14d mac: 003f05b4
static HANDLE s_hCdThread;
// 1.14d mac: 003f05b8
static D2StormAsyncOpStrc* s_pCurrentRequest;
// 1.14d mac: 003f05bc
static uint8_t s_bCdThreadExit;
// 1.14d mac: 003f05c0
static uint32_t* s_pCryptTable;
// 1.14d mac: 003f05c4
static char s_szBasePath[260];
// 1.14d mac: 003f06c8
static uint32_t s_nAsyncBudget;
// 1.14d mac: 003f06cc
static uint32_t s_nIoErrorMode;
// 1.14d mac: 003f06d0
static uint16_t s_nSearchLocale;
// 1.14d mac: 003f06d2
static uint8_t s_nSearchPlatform;
// 1.14d mac: 003f06d4
static DWORD s_dwCdThreadId;
// 1.14d mac: 003970c8
static BOOL s_bSeekOptimization = 1;
// 1.14d mac: 00321850
extern uint8_t gaSFileSignatureRsaModulus[256];
// 1.14d mac: 00321950
extern uint8_t gaSFileSignatureRsaExponent[4];

#define SFILE_LOCK() pthread_mutex_lock(&s_tLock.m_tMutex)
#define SFILE_UNLOCK() pthread_mutex_unlock(&s_tLock.m_tMutex)

SFILE_STATIC int32_t SFILE_GetStreamPosition(D2StormAudioStreamRec* pStream);
SFILE_STATIC int SFILE_CancelAsyncRequest(HANDLE hBuffer, IDirectSoundBuffer* pStream);
SFILE_STATIC void SFILE_InitCryptTable(void);
static int SFileReadFileEx2(D2MPQFileStrc* pFile, void* pBuffer, uint32_t nBytes, size_t* pnBytesRead,
                            void* pOverlapped, int nFlags, void* pfnComplete);
int SFileCloseFile(D2MPQFileStrc* pFile);
int SFileOpenFileEx(D2ArchiveStrc* pArchive, const char* szFileName, uint32_t dwSearchScope,
                    D2MPQFileStrc** ppFile);

// 1.14d mac: 0001204e
uint32_t SFileAuthenticateArchiveEx(D2ArchiveStrc* pArchive, uint32_t* pAuthResult, void* pModulusBytes,
                                    uint32_t nModulusLen, void* pExponentBytes, uint32_t nExponentLen) {
    uint32_t nDummy;
    uint32_t* pnResult = pAuthResult ? pAuthResult : &nDummy;
    *pnResult = 0;
    D2SsigContextStrc* pContext = NULL;
    bool bCustom = pModulusBytes != NULL || pExponentBytes != NULL;
    SSignatureVerifyStream_Begin(&pContext, bCustom ? nModulusLen : 0x100, bCustom ? nExponentLen : 4);
    const void* pModulus = bCustom ? pModulusBytes : gaSFileSignatureRsaModulus;
    const void* pExponent = bCustom ? pExponentBytes : gaSFileSignatureRsaExponent;

    SFILE_LOCK();
    uint8_t* pData = (uint8_t*)SMemAlloc(0x10000, SFILE_FILE, 0xa7f, 0x10000000);
    DWORD dwSavedPos = SetFilePointer(pArchive->hFile, 0, NULL, 1);
    if (dwSavedPos != 0xFFFFFFFF && SetFilePointer(pArchive->hFile, pArchive->nArchiveOffset, NULL, 0) != 0xFFFFFFFF) {
        uint32_t nRemaining = pArchive->pMpqHeader->dwArchiveSize + SSignatureVerifyStream_GetSignatureLength(pContext);
        while (nRemaining) {
            size_t nRead = 0;
            uint32_t nChunk = nRemaining > 0x10000 ? 0x10000 : nRemaining;
            if (!ReadFile(pArchive->hFile, pData, nChunk, (LPDWORD)&nRead, 0) || nRead != nChunk) {
                goto done;
            }
            SSignatureVerifyStream_ProvideData(pContext, pData, nChunk);
            nRemaining -= nChunk;
        }
        SetFilePointer(pArchive->hFile, dwSavedPos, NULL, 0);
    }
done:
    SMemFree(pData, SFILE_FILE, 0xaa5, 0);
    SFILE_UNLOCK();
    int nVerified = SSignatureVerifyStream_Finish(pContext, pModulus, pExponent);
    *pnResult = (nVerified != 0) * 4 + 1;
    return nVerified != 0;
}

// 1.14d mac: 00012264
void SFILE_CancelAsyncRequestNoReturn(int32_t nParam) {
    if (nParam != 0) {
        SFILE_CancelAsyncRequest(nParam, NULL);
        return;
    }
    SErrSetLastError(0x57);
}

// 1.14d mac: 0001228d
// Drops every queued request that reads into hBuffer or fills pStream, and leaves a request
// carrying the dropped one's completion event so that the I/O thread signals it.
SFILE_STATIC int SFILE_CancelAsyncRequest(HANDLE hBuffer, IDirectSoundBuffer* pStream) {
    int nCancelled = 0;
    if (pStream != NULL || hBuffer != 0) {
        SFILE_LOCK();
        nCancelled = 0;
        D2StormAsyncOpStrc* pRequest = s_tRequestQueue.Head();
        while (pRequest != NULL) {
            if ((hBuffer == 0 || pRequest->pReadBufferBase != (void*)hBuffer) &&
                (pStream == NULL || pRequest->pStream != pStream)) {
                pRequest = s_tRequestQueue.Next(pRequest);
            } else {
                HANDLE hEvent = pRequest->hCompletionEvent;
                if (hEvent != 0) {
                    D2StormAsyncOpStrc* pSignal = s_tCompletionList.NewNode(STORM_LIST_LINK_BEFORE, 0, 0);
                    pSignal->hCompletionEvent = hEvent;
                }
                pRequest = s_tRequestQueue.DeleteNode(pRequest);
                nCancelled = 1;
            }
        }
        SFILE_UNLOCK();
    }
    return nCancelled;
}

// 1.14d mac: 0001239c
int SFileCloseArchive(D2ArchiveStrc* pArchive) {
    SFILE_LOCK();
    if (pArchive->pArchiveFile != NULL) {
        SFileCloseFile(pArchive->pArchiveFile);
    }
    s_tArchiveList.UnlinkNode(pArchive);
    if (--pArchive->nRefCount == 0) {
        delete pArchive;
    }
    SFILE_UNLOCK();
    return 1;
}

// 1.14d mac: 00012401
int SFileCloseFile(D2MPQFileStrc* pFile) {
    SFILE_LOCK();
    s_tFileList.UnlinkNode(pFile);
    if (pFile != NULL) {
        if (--pFile->nRefCount == 0) {
            delete pFile;
        }
    }
    SFILE_UNLOCK();
    return 1;
}

// 1.14d mac: 000128ed
BOOL SFileDdaEnd(D2MPQFileStrc* pFile) {
    SFILE_LOCK();
    D2StormAudioStreamRec* pStream = s_tStreamList.Head();
    while (pStream != NULL) {
        D2StormAudioStreamRec* pNextStream = s_tStreamList.Next(pStream);
        if (pStream->pMpqFile == pFile) {
            if (s_pCurrentRequest != NULL && s_pCurrentRequest->pStream != NULL &&
                s_pCurrentRequest->pStream == pStream->pStream) {
                s_pCurrentRequest->pStream = NULL;
            }
            SFILE_CancelAsyncRequest(0, pStream->pStream);
            s_tStreamList.UnlinkNode(pStream);
            if (--pStream->nRefCount == 0) {
                delete pStream;
            }
        }
        pStream = pNextStream;
    }
    pFile->pStream = NULL;
    SFILE_UNLOCK();
    return 1;
}

// 1.14d mac: 000129bc
DWORD SFileSetFilePointer(D2MPQFileStrc* pMPQFile, int32_t nDistanceToMove, int* pnDistanceToMoveHigh,
                          DWORD dwMoveMethod) {
    if (pnDistanceToMoveHigh != NULL && *pnDistanceToMoveHigh != 0) {
        SErrSetLastError(0x57);
        return 0xFFFFFFFF;
    }
    SFILE_LOCK();
    if (pMPQFile->hFile != 0) {
        SFILE_UNLOCK();
        return SetFilePointer(pMPQFile->hFile, nDistanceToMove, NULL, dwMoveMethod);
    }
    uint32_t nFileSize;
    if (dwMoveMethod == 0) {
        pMPQFile->nFilePos = nDistanceToMove;
    } else if (dwMoveMethod == 1 || dwMoveMethod == 2) {
        nFileSize = dwMoveMethod == 1 ? pMPQFile->nFilePos : pMPQFile->pBlockEntry->dwFileSize;
        if (nDistanceToMove < 0 && nFileSize < (uint32_t)-nDistanceToMove) {
            pMPQFile->nFilePos = 0;
            nDistanceToMove = 0;
        } else {
            nDistanceToMove += nFileSize;
            pMPQFile->nFilePos = nDistanceToMove;
        }
    } else {
        nDistanceToMove = pMPQFile->nFilePos;
    }
    uint32_t nLast = pMPQFile->pBlockEntry->dwFileSize - 1;
    if ((uint32_t)nDistanceToMove < nLast) {
        nLast = nDistanceToMove;
    }
    pMPQFile->nFilePos = nLast;
    pMPQFile->nBytesRead = 0;
    pMPQFile->nBufferSize = 0;
    SFILE_UNLOCK();
    return nLast;
}

// 1.14d mac: 00013080
// Walks a RIFF-style stream of (id, size) pairs to the chunk with the given id.
static __attribute__((noinline, used)) uint32_t SFILE_FindChunk(D2MPQFileStrc* pMPQFile, int32_t nChunkId, int* pChunkSize) {
    int32_t aEntry[2];
    while (true) {
        if (!SFileReadFileEx2(pMPQFile, aEntry, 8, NULL, NULL, 0, NULL)) {
            return 0;
        }
        if (aEntry[0] == nChunkId) {
            break;
        }
        if (SFileSetFilePointer(pMPQFile, aEntry[1], NULL, 1) == 0xFFFFFFFF) {
            return 0;
        }
    }
    pChunkSize[0] = aEntry[1];
    DWORD dwPos = SFileSetFilePointer(pMPQFile, 0, NULL, 1);
    pChunkSize[1] = dwPos;
    return dwPos != 0xFFFFFFFF;
}

// 1.14d mac: 00013132
static DWORD SFILE_CdThreadProc(LPVOID pParam);
SFILE_STATIC void SFILE_CreateCdThread(void) {
    if (s_hCdThread == 0) {
        s_hCdEvent = CreateEventA(0, 0, 0, NULL);
        s_hCdThread = CreateThread(0, 0, SFILE_CdThreadProc, 0, 0, &s_dwCdThreadId, "CdThreadProc");
        SetThreadPriority(s_hCdThread, 1);
    }
}

// 1.14d mac: 000131d7
BOOL SFileDdaDestroy(void) {
    SFILE_LOCK();
    s_bCdThreadDraining = 1;
    SFILE_UNLOCK();
    while (s_hCdThread != 0) {
        if ((s_pCurrentRequest == NULL && s_tRequestQueue.Head() == NULL) ||
            WaitForSingleObject(s_hCdThread, 10) != WAIT_TIMEOUT) {
            break;
        }
    }
    SFILE_LOCK();
    D2StormAudioStreamRec* pStream = s_tStreamList.Head();
    while (pStream != NULL) {
        D2StormAudioStreamRec* pNextStream = s_tStreamList.Next(pStream);
        s_tStreamList.UnlinkNode(pStream);
        if (--pStream->nRefCount == 0) {
            delete pStream;
        }
        pStream = pNextStream;
    }
    s_pDirectSound = NULL;
    s_bCdThreadDraining = 0;
    SFILE_UNLOCK();
    return 1;
}

// 1.14d mac: 000132d6
BOOL SFileDdaGetPos(D2MPQFileStrc* pFile, uint32_t* pdwCurrentPos, uint32_t* pdwEndPos) {
    SFILE_LOCK();
    BOOL nResult = 0;
    uint32_t nSize;
    uint32_t nPosition;
    D2StormAudioStreamRec* pStream = s_tStreamList.Head();
    while (true) {
        if (pStream == NULL) {
            SErrSetLastError(0x85100072);
            nSize = 0;
            nPosition = 0;
            break;
        }
        if (pStream->pMpqFile == pFile) {
            nSize = pStream->nDataChunkSize;
            nResult = 1;
            nPosition = 0;
            if ((uint32_t)pStream->nStreamType > 1) {
                if (pStream->nStreamType == 4) {
                    SErrSetLastError(0x85100072);
                    nResult = 0;
                    nPosition = nSize;
                } else {
                    SFILE_GetStreamPosition(pStream);
                    nPosition = pStream->nCurrentPos;
                }
            }
            break;
        }
        pStream = s_tStreamList.Next(pStream);
    }
    if (pdwCurrentPos != NULL) {
        *pdwCurrentPos = nPosition;
    }
    if (pdwEndPos != NULL) {
        *pdwEndPos = nSize;
    }
    SFILE_UNLOCK();
    return nResult;
}

// 1.14d mac: 000133a1
SFILE_STATIC int32_t SFILE_GetStreamPosition(D2StormAudioStreamRec* pStream) {
    DWORD dwPlayCursor = 0;
    DWORD dwWriteCursor = 0;
    int nResult = pStream->pStream->GetCurrentPosition(&dwPlayCursor, &dwWriteCursor);
    DWORD dwResult = 0xFFFFFFFF;
    if (nResult >= 0) {
        int nAhead = pStream->nBytePos % pStream->nBufferSize - dwPlayCursor;
        int nWrap = -(int)pStream->nBufferSize;
        if (0 < nAhead) {
            nWrap = 0;
        }
        uint32_t nValidBytes = (pStream->nBytePos - nAhead) + nWrap;
        if (pStream->nDataChunkSize <= nValidBytes) {
            nValidBytes = pStream->nDataChunkSize;
        }
        pStream->nCurrentPos = nValidBytes;
        dwResult = dwPlayCursor;
    }
    return dwResult;
}

// 1.14d mac: 00013418
BOOL SFileDdaInitialize(void* pDirectSound) {
    if (pDirectSound == NULL) {
        SErrSetLastError(0x57);
    } else {
        s_pDirectSound = pDirectSound;
    }
    return pDirectSound != NULL;
}

// 1.14d mac: 0001344b
int SFileDestroy(void) {
    SFileDdaDestroy();
    if (s_hCdThread != 0) {
        s_bCdThreadExit = 1;
        SetEvent(s_hCdEvent);
        WaitForSingleObject(s_hCdThread, 1000);
        if (s_hCdEvent != 0) {
            CloseHandle(s_hCdEvent);
            s_hCdEvent = 0;
        }
        if (s_hCdThread != 0) {
            CloseHandle(s_hCdThread);
            s_hCdThread = 0;
        }
    }
    D2StormAsyncOpStrc* pCurrent = s_pCurrentRequest;
    if (pCurrent != NULL) {
        delete pCurrent;
        s_pCurrentRequest = NULL;
    }
    D2StormAsyncOpStrc* pRequest;
    while ((pRequest = s_tRequestQueue.Head()) != NULL) {
        delete pRequest;
    }
    D2StormAudioStreamRec* pStream = s_tStreamList.Head();
    while (pStream != NULL) {
        SErrReportResourceLeak("AUDIOSTREAM");
        delete pStream;
        pStream = s_tStreamList.Head();
    }
    D2MPQFileStrc* pFile;
    while ((pFile = s_tFileList.Head()) != NULL) {
        SErrReportNamedResourceLeak("HSFILE", pFile->szFileName);
        delete pFile;
    }
    D2ArchiveStrc* pArchive = s_tArchiveList.Head();
    while (pArchive != NULL) {
        SErrReportNamedResourceLeak("HSARCHIVE", pArchive->szArchivePath);
        delete pArchive;
        pArchive = s_tArchiveList.Head();
    }
    if (s_pCryptTable != NULL) {
        SMemFree(s_pCryptTable, SFILE_FILE, 0xd6d, 0);
        s_pCryptTable = NULL;
    }
    if (s_pWaveBuffer != NULL) {
        SMemFree(s_pWaveBuffer, SFILE_FILE, 0xd79, 0);
        s_pWaveBuffer = NULL;
    }
    return 1;
}

// 1.14d mac: 000136bd
int32_t SFileEnableDirectAccess(BOOL bUseDirect) {
    s_dwDirectAccessFlags = bUseDirect;
    return 1;
}

// 1.14d mac: 000136d6
void SFileEnableSeekOptimization(BOOL bEnable) {
    s_bSeekOptimization = bEnable;
}

SFILE_STATIC void SFILE_ConvertRelativePathName(const char* szFileName, char* szOutPath, int32_t bStripDir);
SFILE_STATIC uint32_t SFILE_FindFileOnDisk(const char* szFileName, uint8_t bSearchFlags, char* szOutPath);
SFILE_STATIC D2MPQBlockEntryStrc* GetFileBlockEntry(D2ArchiveStrc* pArchive, const char* szFileName, uint32_t dwFlags,
                                              D2ArchiveStrc** ppArchiveOut);
SFILE_STATIC int SFILE_GetHashTableIndex(uint32_t nUnused, const char* szFileName, D2ArchiveStrc* pArchive,
                                   uint16_t nLocale, uint8_t nPlatform);

// 1.14d mac: 000136ea
int SFileFileExistsEx(D2ArchiveStrc* pArchive, const char* szFileName, uint32_t dwFlags) {
    char szDiskPath[260];
    int nResult;
    if (szFileName == NULL || *szFileName == '\0') {
        SErrSetLastError(0x57);
        return 0;
    }
    SFILE_LOCK();
    SFILE_InitCryptTable();
    if ((dwFlags & 3) == 0 || SFILE_FindFileOnDisk(szFileName, (uint8_t)dwFlags, szDiskPath) == 0) {
        nResult = GetFileBlockEntry(pArchive, szFileName, dwFlags, NULL) != NULL;
    } else {
        nResult = 2;
    }
    SFILE_UNLOCK();
    return nResult;
}

// 1.14d mac: 000137af
// The MPQ crypt table: 5 x 256 dwords from the generator seed 0x100001.
SFILE_STATIC void SFILE_InitCryptTable(void) {
    if (s_pCryptTable == NULL) {
        uint32_t* pTable = (uint32_t*)SMemAlloc(0x1400, SFILE_FILE, 0x31e, 0);
        s_pCryptTable = pTable;
        uint32_t dwSeed = 0x100001;
        for (int nIndex = 0; nIndex != 0x100; nIndex++) {
            uint32_t* pEntry = pTable;
            for (int nRow = 5; nRow != 0; nRow--) {
                uint32_t dwHigh = (dwSeed * 0x7d + 3) % 0x2aaaab;
                dwSeed = (dwHigh * 0x7d + 3) % 0x2aaaab;
                *pEntry = dwHigh * 0x10000 | (dwSeed & 0xffff);
                pEntry += 0x100;
            }
            pTable++;
        }
    }
}

// 1.14d mac: 0001387f
SFILE_STATIC uint32_t SFILE_FindFileOnDisk(const char* szFileName, uint8_t bSearchFlags, char* szOutPath) {
    if ((bSearchFlags & 3) == 0) {
        return 0;
    }
    if (bSearchFlags & 2) {
        SFILE_ConvertRelativePathName(szFileName, szOutPath, 1);
        uint32_t dwAttributes = GetFileAttributes(szOutPath);
        if (((dwAttributes >> 4) & 1) == 0) {
            return ((dwAttributes >> 4) & 1) ^ 1;
        }
    }
    if (bSearchFlags & 1) {
        if (*szFileName == '\\' || strstr(szFileName, ":\\") != NULL || strstr(szFileName, "\\\\") != NULL) {
            SStrCopy(szOutPath, szFileName, 0x104);
        } else {
            SFILE_ConvertRelativePathName(szFileName, szOutPath, 0);
        }
        return (GetFileAttributes(szOutPath) >> 4 & 1) ^ 1;
    }
    return 0;
}


// 1.14d mac: 00013955
// The block table entry of a file in the given archive, or in the first open archive that has it.
SFILE_STATIC D2MPQBlockEntryStrc* GetFileBlockEntry(D2ArchiveStrc* pArchive, const char* szFileName, uint32_t dwFlags,
                                              D2ArchiveStrc** ppArchiveOut) {
    int nHashIndex;
    DWORD dwErrCode;
    if (ppArchiveOut != NULL) {
        *ppArchiveOut = NULL;
    }
    if (pArchive == NULL) {
        pArchive = s_tArchiveList.Head();
        if (pArchive != NULL && s_pCryptTable != NULL) {
            while (pArchive != NULL) {
                nHashIndex = SFILE_GetHashTableIndex(s_nSearchLocale, szFileName, pArchive, s_nSearchLocale, s_nSearchPlatform);
                if (nHashIndex != -1) {
                    goto found;
                }
                pArchive = s_tArchiveList.Next(pArchive);
            }
        }
    } else {
        nHashIndex = SFILE_GetHashTableIndex(s_nSearchLocale, szFileName, pArchive, s_nSearchLocale, s_nSearchPlatform);
        if (nHashIndex != -1) {
        found:
            int nBlockIndex = pArchive->pHashTable[nHashIndex].dwBlockIndex;
            uint32_t dwBlockFlags = pArchive->pBlockTable[nBlockIndex].dwFlags;
            if ((int)dwBlockFlags < 0 && ((dwFlags & 4) == 0 || (dwBlockFlags & 0x1ff00) == 0)) {
                D2MPQBlockEntryStrc* pBlock = pArchive->pBlockTable + nBlockIndex;
                if (pBlock == NULL) {
                    return NULL;
                }
                if (ppArchiveOut != NULL) {
                    *ppArchiveOut = pArchive;
                }
                return pBlock;
            }
            dwErrCode = 0x3ee;
            goto fail;
        }
    }
    dwErrCode = 2;
fail:
    SErrSetLastError(dwErrCode);
    return NULL;
}

// 1.14d mac: 00013a6f
SFILE_STATIC void SFILE_BuildDefaultBasePath(void) {
    HANDLE hModule = GetModuleHandle(NULL);
    GetModuleFileName(hModule, s_szBasePath, 0x104);
    char* pLastSlash = SStrChrR(s_szBasePath, '\\');
    if (pLastSlash != NULL) {
        *pLastSlash = '\0';
    }
    SStrPack(s_szBasePath, "\\", 0x104);
}

// 1.14d mac: 00013adb
int SFileGetFileName(D2MPQFileStrc* pFile, char* szText, int32_t nBufferSize) {
    if (szText == NULL) {
        SErrSetLastError(0x57);
    } else {
        *szText = '\0';
        SFILE_LOCK();
        SStrCopy(szText, pFile->szFileName, nBufferSize);
        SFILE_UNLOCK();
    }
    return szText != NULL;
}

// 1.14d mac: 00013b40
DWORD SFileGetFileSize(D2MPQFileStrc* pMPQFile, LPDWORD pdwFileSizeHigh) {
    DWORD dwSize;
    if (pdwFileSizeHigh != NULL) {
        *pdwFileSizeHigh = 0;
    }
    SFILE_LOCK();
    if (pMPQFile->hFile == 0) {
        dwSize = pMPQFile->pBlockEntry->dwFileSize;
        SFILE_UNLOCK();
    } else {
        SFILE_UNLOCK();
        dwSize = GetFileSize(pMPQFile->hFile, pdwFileSizeHigh);
    }
    return dwSize;
}

uint32_t SFileLoadFileEx2(D2ArchiveStrc* pArchive, const char* szFileName, void** ppOutBuffer, DWORD* pdwOutSize,
                          size_t nExtraPadBytes, uint32_t nSearchScope, int32_t nReadCtx, int nReadFlags);

// 1.14d mac: 00013bad
uint32_t SFileLoadFile(const char* szFileName, void** ppOutBuffer, DWORD* pdwOutSize, size_t nExtraPadBytes,
                       int32_t nReadCtx) {
    uint32_t nDiskScope = (s_dwDirectAccessFlags & 1) + 2;
    if ((s_dwDirectAccessFlags & 2) == 0) {
        nDiskScope = s_dwDirectAccessFlags & 1;
    }
    uint32_t nNoSource = !(s_dwDirectAccessFlags != 0 || s_tArchiveList.Head() != NULL);
    return SFileLoadFileEx2(NULL, szFileName, ppOutBuffer, pdwOutSize, nExtraPadBytes, nNoSource | nDiskScope,
                            nReadCtx, 0);
}

// 1.14d mac: 00013c35
uint32_t SFileLoadFileEx2(D2ArchiveStrc* pArchive, const char* szFileName, void** ppOutBuffer, DWORD* pdwOutSize,
                          size_t nExtraPadBytes, uint32_t nSearchScope, int32_t nReadCtx, int nReadFlags) {
    if (ppOutBuffer != NULL && (*ppOutBuffer = NULL, szFileName != NULL)) {
        if (pdwOutSize != NULL) {
            *pdwOutSize = 0;
        }
        D2MPQFileStrc* pFile = NULL;
        if (SFileOpenFileEx(pArchive, szFileName, nSearchScope, &pFile) != 0) {
            DWORD dwFileSize = SFileGetFileSize(pFile, NULL);
            void* pBuffer = SMemAlloc(dwFileSize + nExtraPadBytes, SFILE_FILE, 0xf2d, 0);
            if (SFileReadFileEx2(pFile, pBuffer, dwFileSize, NULL, (void*)nReadCtx, nReadFlags, NULL) == 0) {
                if (pBuffer != NULL) {
                    SMemFree(pBuffer, SFILE_FILE, 0xf43, 0);
                }
            } else {
                if (0 < (int)nExtraPadBytes) {
                    bzero((char*)pBuffer + dwFileSize, nExtraPadBytes);
                }
                *ppOutBuffer = pBuffer;
                if (pdwOutSize != NULL) {
                    *pdwOutSize = dwFileSize;
                }
            }
        }
        if (pFile != NULL) {
            SFileCloseFile(pFile);
        }
        return *ppOutBuffer != NULL;
    }
    SErrSetLastError(0x57);
    return 0;
}

// 1.14d mac: 00013d8b
uint32_t SFileLoadFileEx(D2ArchiveStrc* pArchive, const char* szFileName, void** ppOutBuffer, DWORD* pdwOutSize,
                         size_t nExtraPadBytes, uint32_t nSearchScope, int32_t nReadCtx) {
    return SFileLoadFileEx2(pArchive, szFileName, ppOutBuffer, pdwOutSize, nExtraPadBytes, nSearchScope, nReadCtx, 0);
}

// 1.14d mac: 0001453f
SFILE_STATIC void SFILE_ConvertRelativePathName(const char* szFileName, char* szOutPath, int32_t bStripDir) {
    char szPath[260];
    if (s_szBasePath[0] == '\0') {
        SFILE_BuildDefaultBasePath();
    }
    if (bStripDir != 0) {
        char* pLastSlash = SStrChrR(szFileName, '\\');
        if (pLastSlash != NULL) {
            szFileName = pLastSlash + 1;
        }
    }
    sprintf(szPath, "%s%s", s_szBasePath, szFileName);
    PATH_MakeFullPath(szOutPath, szPath, 0x104);
}

// 1.14d mac: 00014d7d
SFILE_STATIC void SFILE_Decrypt(uint32_t* pData, uint32_t nByteLen, uint32_t nSeed) {
    uint32_t* pTable = s_pCryptTable;
    uint32_t nKey = 0xEEEEEEEE;
    for (uint32_t nCount = nByteLen >> 2; nCount != 0; nCount--) {
        nKey += pTable[0x400 + (nSeed & 0xff)];
        uint32_t nDecrypted = (nKey + nSeed) ^ *pData;
        *pData++ = nDecrypted;
        nKey = nDecrypted + 3 + nKey * 0x21;
        nSeed = ((~nSeed) << 0x15) + 0x11111111 | (nSeed >> 0xb);
    }
}
typedef int (*PFNSFILEREADERROR)(const char* szFileName, DWORD dwLastError, int nRetryCount);

// 1.14d mac: 003970cc
static uint32_t s_nDataChunkSize = 0x20000;
// 1.14d mac: 003970d0
static int32_t s_nReadErrorMode = 2;
// 1.14d mac: 003f06cc
static PFNSFILEREADERROR s_pfnReadError;

// 1.14d mac: 00014c64
SFILE_STATIC BOOL SFILE_ReadFileChecked(int32_t nFileOffset, uint32_t* pFilePosOut, HANDLE hFile, void* pOutputBuf,
                                  uint32_t nBytesToRead, uint32_t* pBytesRead, const char* szFileName) {
    int nRetryCount = 0;
    do {
        do {
            while (true) {
                if (nFileOffset != -1) {
                    SetFilePointer(hFile, nFileOffset, NULL, 0);
                }
                int nRead = ReadFile(hFile, pOutputBuf, nBytesToRead, pBytesRead, 0);
                *pFilePosOut = *pBytesRead + nFileOffset;
                if (nRead != 0) {
                    if (*pBytesRead == nBytesToRead) {
                        return 1;
                    }
                    SErrSetLastError(0x26);
                    return 0;
                }
                DWORD dwLastError = GetLastError();
                if (s_nReadErrorMode != 2) {
                    if (s_nReadErrorMode == 0) {
                        return 0;
                    }
                    if (s_nReadErrorMode == 1) {
                        if (s_pfnReadError == NULL) {
                            return 0;
                        }
                        int nRetry = s_pfnReadError(szFileName, dwLastError, nRetryCount);
                        nRetryCount++;
                        if (nRetry == 0) {
                            return 0;
                        }
                    }
                    break;
                }
                SErrDisplayError(dwLastError, szFileName, -4, NULL, 0, 1);
            }
        } while (true);
    } while (true);
}

// 1.14d mac: 00014ffd
SFILE_STATIC BOOL SFILE_SyncReadFile(D2MPQFileStrc* pFileHandle, int32_t nFileOffset, void* pOutputBuf,
                               uint32_t nBytesToRead, uint32_t* pBytesRead) {
    uint32_t aResult[2];
    aResult[1] = 0xFFFFFFFF;
    aResult[0] = 0;
    SFILE_ReadFileChecked(nFileOffset, &aResult[1], pFileHandle->hFile, pOutputBuf, nBytesToRead, &aResult[0],
                          pFileHandle->szFileName);
    if (pBytesRead != NULL) {
        *pBytesRead = aResult[0];
    }
    return aResult[0] == nBytesToRead;
}

// 1.14d mac: 00014de3
int32_t SFileOpenFile(const char* szFileName, D2MPQFileStrc** ppFile) {
    uint32_t nDiskScope = (s_dwDirectAccessFlags & 1) + 2;
    if ((s_dwDirectAccessFlags & 2) == 0) {
        nDiskScope = s_dwDirectAccessFlags & 1;
    }
    uint32_t nNoSource = !(s_dwDirectAccessFlags != 0 || s_tArchiveList.Head() != NULL);
    return SFileOpenFileEx(NULL, szFileName, nNoSource | nDiskScope, ppFile);
}

// 1.14d mac: 00014fbb
int SFileReadFile(D2MPQFileStrc* pFile, void* pBuffer, uint32_t nBytesToRead, size_t* pnBytesRead,
                  void* pOverlapped) {
    return SFileReadFileEx2(pFile, pBuffer, nBytesToRead, pnBytesRead, pOverlapped, 0, NULL);
}

// 1.14d mac: 0001545d
void SFileSetAsyncBudget(uint32_t nBudgetBytesPerSec) {
    s_nAsyncBudget = nBudgetBytesPerSec;
}

// 1.14d mac: 00015471
int32_t SFileSetBasePath(const char* szText) {
    if (szText == NULL) {
        SErrSetLastError(0x57);
        return 0;
    }
    if (*szText == '\0') {
        s_szBasePath[0] = '\0';
    } else {
        char cLastChar = szText[SStrLen(szText) - 1];
        if (0x104 < SStrLen(szText) + 1 + (cLastChar != '\\')) {
            SErrSetLastError(0xa1);
            return 0;
        }
        SFILE_LOCK();
        SStrCopy(s_szBasePath, szText, 0x104);
        if (cLastChar != '\\') {
            SStrPack(s_szBasePath, "\\", 0x104);
        }
        SFILE_UNLOCK();
    }
    return 1;
}

// 1.14d mac: 0001553b
void SFileSetDataChunkSize(uint32_t nChunkSize) {
    if ((nChunkSize & (nChunkSize - 1)) == 0) {
        s_nDataChunkSize = nChunkSize;
    } else {
        SErrSetLastError(0x57);
    }
}

// 1.14d mac: 0001556a
BOOL SFileSetIoErrorMode(int32_t nErrorMode, void* pfnErrorHandler) {
    s_nReadErrorMode = nErrorMode;
    s_pfnReadError = (PFNSFILEREADERROR)pfnErrorHandler;
    return 1;
}

// 1.14d mac: 0001558c
int32_t SFileUnloadFile(void* pFile) {
    if (pFile == NULL) {
        SErrSetLastError(0x57);
    } else {
        SMemFree(pFile, SFILE_FILE, 0x12ee, 0);
    }
    return pFile != NULL;
}

SFILE_STATIC uint32_t SFILE_InternalReadAligned(D2MPQFileStrc* pMPQFile, uint32_t nAlignedOffset, void* pBuffer,
                                          uint32_t nBytes);

// 1.14d mac: 000155db
SFILE_STATIC uint32_t SFILE_InternalReadAlignedSector(D2MPQFileStrc* pMPQFile, uint32_t nAlignedOffset) {
    D2ArchiveStrc* pArchive = pMPQFile->pArchive;
    if (pArchive->pLastReadFile == pMPQFile && pArchive->nLastReadOffset == nAlignedOffset) {
        return pArchive->nLastReadResult;
    }
    uint32_t nBytes = pArchive->nSectorSize;
    uint32_t nFileSize = pMPQFile->pBlockEntry->dwFileSize;
    if (nFileSize < nBytes + nAlignedOffset && (pMPQFile->pBlockEntry->dwFlags & 0xff00) == 0) {
        nBytes = nFileSize - nAlignedOffset;
    }
    pArchive->pLastReadFile = pMPQFile;
    pArchive = pMPQFile->pArchive;
    pArchive->nLastReadOffset = nAlignedOffset;
    uint32_t nResult = SFILE_InternalReadAligned(pMPQFile, nAlignedOffset, pArchive->pSectorBuffer, nBytes);
    pMPQFile->pArchive->nLastReadResult = nResult;
    return nResult;
}

// What the decompression callbacks work on: a source (pointer at 0x0C, count at 0x14) and a
// destination (buffer, running offset).
struct SFileDecompressSource {
    uint8_t pad00[0x0C];
    void* pData;         // 0x0C
    uint8_t pad10[4];
    uint32_t nSize;      // 0x14
};
struct SFileDecompressSink {
    uint8_t* pBuffer;    // 0x00
    int nOffset;         // 0x04
};

// 1.14d mac: 00015c1f
SFILE_STATIC uint32_t SFILE_DecompressReadCallback(SFileDecompressSource* pState, void** ppData) {
    *ppData = pState->pData;
    return pState->nSize;
}

// 1.14d mac: 00015c32
SFILE_STATIC int SFILE_DecompressWriteCallback(SFileDecompressSink* pOutput, void* pData, size_t nSize) {
    memcpy(pOutput->pBuffer + pOutput->nOffset, pData, nSize);
    pOutput->nOffset += nSize;
    return 0;
}

// 1.14d mac: 00015c64
// Probes the archive's hash table for the three hashes of a name: the exact locale and
// platform wins; a neutral entry is the fallback.
SFILE_STATIC int SFILE_GetHashTableIndex(uint32_t nUnused, const char* szFileName, D2ArchiveStrc* pArchive, uint16_t nLocale,
                                   uint8_t nPlatform) {
    uint32_t nStartIndex = 0x7fed7fed;
    uint32_t dwHashA = 0x7fed7fed;
    uint32_t dwHashB = 0x7fed7fed;
    if (szFileName != NULL) {
        int32_t nSeed2 = 0xEEEEEEEE;
        for (const char* p = szFileName; *p; p++) {
            uint32_t nChar = toupper(*p) & 0xff;
            nStartIndex = (nStartIndex + nSeed2) ^ s_pCryptTable[nChar];
            nSeed2 = nStartIndex + 3 + nSeed2 * 0x21 + nChar;
        }
        nSeed2 = 0xEEEEEEEE;
        for (const char* p = szFileName; *p; p++) {
            uint32_t nChar = toupper(*p) & 0xff;
            dwHashA = (dwHashA + nSeed2) ^ s_pCryptTable[0x100 + nChar];
            nSeed2 = dwHashA + 3 + nSeed2 * 0x21 + nChar;
        }
        nSeed2 = 0xEEEEEEEE;
        for (const char* p = szFileName; *p; p++) {
            uint32_t nChar = toupper(*p) & 0xff;
            dwHashB = (dwHashB + nSeed2) ^ s_pCryptTable[0x200 + nChar];
            nSeed2 = dwHashB + 3 + nSeed2 * 0x21 + nChar;
        }
    }
    D2MPQHashEntryStrc* pTable = pArchive->pHashTable;
    uint32_t dwMask = pArchive->pMpqHeader->dwHashTableSize - 1;
    nStartIndex &= dwMask;
    uint32_t nFallbackIndex = 0xFFFFFFFF;
    uint32_t nIndex = nStartIndex;
    while (true) {
        D2MPQHashEntryStrc* pEntry = &pTable[nIndex];
        if (pEntry->dwBlockIndex == 0xFFFFFFFF) {
            return nFallbackIndex;
        }
        if (pEntry->dwHashA == dwHashA && pEntry->dwHashB == dwHashB && pEntry->dwBlockIndex != 0xFFFFFFFE) {
            int16_t nEntryLocale = pEntry->wLocale;
            bool bTryFallback = true;
            if (nEntryLocale == (int16_t)nLocale) {
                if ((char)pEntry->wPlatform == (char)nPlatform) {
                    return nIndex;
                }
            } else if (nEntryLocale != 0 && nEntryLocale != (int16_t)nLocale) {
                bTryFallback = false;
            }
            if (bTryFallback) {
                char cPlatform = (char)pEntry->wPlatform;
                if (cPlatform == '\0' || cPlatform == (char)nPlatform) {
                    nFallbackIndex = nIndex;
                }
            }
        }
        nIndex = (nIndex + 1) & dwMask;
        if (nIndex == nStartIndex) {
            return nFallbackIndex;
        }
    }
}

// The records' destructors (the Free...Record functions): each unlinks the record, once by
// hand and once in the link member's own destructor.

// 1.14d mac: 00017278
D2ArchiveStrc::~D2ArchiveStrc() {
    if (hFile != 0 && pArchiveFile == NULL) {
        CloseHandle(hFile);
    }
    if (pSectorBuffer != NULL) {
        SMemFree(pSectorBuffer, SFILE_FILE, 0xf0, 0);
    }
    if (pMpqHeader != NULL) {
        SMemFree(pMpqHeader, SFILE_FILE, 0xf1, 0);
    }
    if (pBlockTable != NULL) {
        SMemFree(pBlockTable, SFILE_FILE, 0xf8, 0);
    }
    if (pHashTable != NULL) {
        SMemFree(pHashTable, SFILE_FILE, 0xf9, 0);
    }
    m_link.Unlink();
}

// 1.14d mac: 0001737c
D2MPQFileStrc::~D2MPQFileStrc() {
    if (hFile != 0) {
        CloseHandle(hFile);
    }
    if (pArchive != NULL) {
        pArchive->pLastReadFile = NULL;
    }
    if (pSectorTable != NULL) {
        SMemFree(pSectorTable, SFILE_FILE, 0x124, 0);
    }
    if (pReadBuffer != NULL) {
        SMemFree(pReadBuffer, SFILE_FILE, 0x125, 0);
    }
    if (pArchive != NULL) {
        if (--pArchive->nRefCount == 0) {
            delete pArchive;
        }
    }
    if (szFilePath != NULL) {
        SMemFree(szFilePath, SFILE_FILE, 0x127, 0);
    }
    m_link.Unlink();
}

// 1.14d mac: 00016a72 (the mutex destructor)
SFileCritSect::~SFileCritSect() {
    pthread_mutex_destroy(&m_tMutex);
}
#endif  // D2_OS_MAC
