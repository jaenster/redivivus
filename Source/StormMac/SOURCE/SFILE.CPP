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

// A critical section: a recursive pthread mutex.
void CRITSECT_Init(pthread_mutex_t* pMutex);

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
// The completion events of cancelled requests, for the I/O thread to signal.
static TSExplicitList<SFileEventNode, 0> s_tCompletionList;
// 1.14d mac: 003f055c
static TSExplicitList<D2StormAudioStreamRec, 0> s_tStreamList;
// 1.14d mac: 003f0568
static SFileCritSect s_tQueueLock;

// 1.14d mac: 003f059c
static IDirectSound* s_pDirectSound;
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
uint16_t gnSFileSearchLocale;
// 1.14d mac: 003f06d2
static uint8_t s_nSearchPlatform;
// 1.14d mac: 003f06d4
static DWORD s_dwCdThreadId;
// 1.14d mac: 003970c8
static BOOL s_bSeekOptimization = 1;
// 1.14d mac: 00321850
uint8_t gaSFileSignatureRsaModulus[256] = {
    0x77, 0x64, 0xf8, 0x57, 0x1d, 0xfb, 0xb0, 0x09, 0xc4, 0xe6, 0x28, 0x91,
    0x34, 0xe3, 0x55, 0x61, 0x15, 0x8a, 0xe9, 0x07, 0xfc, 0xaa, 0x60, 0xb3,
    0x82, 0xb7, 0xe2, 0xa4, 0x40, 0x15, 0x01, 0x3f, 0xc2, 0x36, 0xa8, 0x9d,
    0x95, 0xd0, 0x54, 0x69, 0xaa, 0xf5, 0xed, 0x5c, 0x7f, 0x21, 0xc5, 0x55,
    0x95, 0x56, 0x5b, 0x2f, 0xc6, 0xdd, 0x2c, 0xbd, 0x74, 0xa3, 0x5a, 0x0d,
    0x70, 0x98, 0x9a, 0x01, 0x36, 0x51, 0x78, 0x71, 0x9b, 0x8e, 0xcb, 0xb8,
    0x84, 0x67, 0x30, 0xf4, 0x43, 0xb3, 0xa3, 0x50, 0xa3, 0xba, 0xa4, 0xf7,
    0xb1, 0x94, 0xe5, 0x5b, 0x95, 0x8b, 0x1a, 0xe4, 0x04, 0x1d, 0xfb, 0xcf,
    0x0e, 0xe6, 0x97, 0x4c, 0xdc, 0xe4, 0x28, 0x7f, 0xb8, 0x58, 0x4a, 0x45,
    0x1b, 0xc8, 0x8c, 0xd0, 0xfd, 0x2e, 0x77, 0xc4, 0x30, 0xd8, 0x3d, 0xd2,
    0xd5, 0xfa, 0xba, 0x9d, 0x1e, 0x02, 0xf6, 0x7b, 0xbe, 0x08, 0x95, 0xcb,
    0xb0, 0x53, 0x3e, 0x1c, 0x41, 0x45, 0xfc, 0x27, 0x6f, 0x63, 0x6a, 0x73,
    0x91, 0xa9, 0x42, 0x00, 0x12, 0x93, 0xf8, 0x5b, 0x83, 0xed, 0x52, 0x77,
    0x4e, 0x38, 0x08, 0x16, 0x23, 0x10, 0x85, 0x4c, 0x0b, 0xa9, 0x8c, 0x9c,
    0x40, 0x4c, 0xaf, 0x6e, 0xa7, 0x89, 0x02, 0xc5, 0x06, 0x96, 0x99, 0x41,
    0xd4, 0x31, 0x03, 0x4a, 0xa9, 0x2b, 0x17, 0x52, 0xdd, 0x5c, 0x4e, 0x5f,
    0x16, 0xc3, 0x81, 0x0f, 0x2e, 0xe2, 0x17, 0x45, 0x2b, 0x7b, 0x65, 0x7a,
    0xa3, 0x18, 0x87, 0xc2, 0xb2, 0xf5, 0xcd, 0x9c, 0xba, 0xcb, 0xde, 0x07,
    0x6f, 0x7c, 0x8b, 0x03, 0x68, 0xe6, 0x3c, 0x5a, 0x2c, 0xae, 0xdc, 0xc3,
    0xc8, 0x38, 0x35, 0x82, 0xda, 0x4d, 0x04, 0xce, 0x9c, 0x68, 0x47, 0x7d,
    0xb4, 0x1d, 0x98, 0x42, 0x8c, 0xf8, 0x27, 0x7e, 0xc8, 0x87, 0xf6, 0x24,
    0xce, 0x7e, 0x06, 0xb1,
};
// 1.14d mac: 00321950
uint8_t gaSFileSignatureRsaExponent[4] = {0x01, 0x00, 0x01, 0x00};

#define SFILE_LOCK() pthread_mutex_lock(&s_tLock.m_tMutex)
#define SFILE_UNLOCK() pthread_mutex_unlock(&s_tLock.m_tMutex)

static int32_t SFILE_GetStreamPosition(D2StormAudioStreamRec* pStream);
int SFILE_s_OpenArchive(D2ArchiveStrc* pArchive, uint32_t nFlags, int32_t bAllowEmpty,
                        D2ArchiveStrc** ppArchiveOut);
static void SFILE_CreateCdThread(void);
struct FSSpec;
static int SFILE_CancelAsyncRequest(HANDLE hBuffer, IDirectSoundBuffer* pStream);
static void SFILE_InitCryptTable(void);
int SFileReadFileEx2(D2MPQFileStrc* pFile, void* pBuffer, uint32_t nBytes, size_t* pnBytesRead,
                            void* pOverlapped, int nFlags, void* pfnComplete);
int SFileCloseFile(D2MPQFileStrc* pFile);
int SFileOpenFileEx(D2ArchiveStrc* pArchive, const char* szFileName, uint32_t dwSearchScope,
                    D2MPQFileStrc** ppFile);

uint32_t SFileAuthenticateArchiveEx(D2ArchiveStrc* pArchive, uint32_t* pAuthResult, void* pModulusBytes,
                                    uint32_t nModulusLen, void* pExponentBytes, uint32_t nExponentLen);

// 1.14d mac: 00011fec
int SFileAuthenticateArchive(D2ArchiveStrc* pArchive, uint32_t* pAuthResult) {
    if (SFileAuthenticateArchiveEx(pArchive, pAuthResult, NULL, 0, NULL, 0)) {
        return 1;
    }
    if (pAuthResult) {
        *pAuthResult = 0;
    }
    SErrSetLastError(0x4dc);
    return 0;
}

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
static int SFILE_CancelAsyncRequest(HANDLE hBuffer, IDirectSoundBuffer* pStream) {
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
                    SFileEventNode* pSignal = s_tCompletionList.NewNode(STORM_LIST_LINK_BEFORE, 0, 0);
                    pSignal->hEvent = hEvent;
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
        D2MPQFileStrc* pStreamFile = pStream->pMpqFile;
        D2StormAudioStreamRec* pNextStream = s_tStreamList.Next(pStream);
        if (pStreamFile == pFile) {
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
    pFile->bStreamActive = 0;
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
    } else {
        if (dwMoveMethod == 1) {
            nFileSize = pMPQFile->nFilePos;
        } else if (dwMoveMethod == 2) {
            nFileSize = pMPQFile->pBlockEntry->dwFileSize;
        } else {
            nDistanceToMove = pMPQFile->nFilePos;
            goto seeked;
        }
        if (nDistanceToMove < 0 && nFileSize < (uint32_t)-nDistanceToMove) {
            pMPQFile->nFilePos = 0;
            nDistanceToMove = 0;
        } else {
            nDistanceToMove += nFileSize;
            pMPQFile->nFilePos = nDistanceToMove;
        }
    }
seeked:
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
static uint32_t SFILE_FindChunk(D2MPQFileStrc* pMPQFile, int32_t nChunkId, int* pChunkSize) {
    // A chunk header, read whole: a struct, not an array (1.14d has no stack guard here).
    struct {
        int32_t nId;
        int32_t nSize;
    } tEntry;
    while (true) {
        if (!SFileReadFileEx2(pMPQFile, &tEntry, sizeof(tEntry), NULL, NULL, 0, NULL)) {
            return 0;
        }
        if (tEntry.nId == nChunkId) {
            break;
        }
        if (SFileSetFilePointer(pMPQFile, tEntry.nSize, NULL, 1) == 0xFFFFFFFF) {
            return 0;
        }
    }
    pChunkSize[0] = tEntry.nSize;
    DWORD dwPos = SFileSetFilePointer(pMPQFile, 0, NULL, 1);
    pChunkSize[1] = dwPos;
    return dwPos != 0xFFFFFFFF;
}

static DWORD SFILE_CdThreadProc(LPVOID pParam);
// 1.14d mac: 00013132
static void SFILE_CreateCdThread(void) {
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
static int32_t SFILE_GetStreamPosition(D2StormAudioStreamRec* pStream) {
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
        if (nValidBytes >= pStream->nDataChunkSize) {
            nValidBytes = pStream->nDataChunkSize;
        }
        pStream->nCurrentPos = nValidBytes;
        dwResult = dwPlayCursor;
    }
    return dwResult;
}

// 1.14d mac: 00013418
BOOL SFileDdaInitialize(IDirectSound* pDirectSound) {
    if (pDirectSound == NULL) {
        SErrSetLastError(0x57);
        return 0;
    }
    s_pDirectSound = pDirectSound;
    return 1;
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

static void SFILE_ConvertRelativePathName(const char* szFileName, char* szOutPath, int32_t bStripDir);
static uint32_t SFILE_FindFileOnDisk(const char* szFileName, uint32_t dwFlags, char* szOutPath);
static D2MPQBlockEntryStrc* GetFileBlockEntry(D2ArchiveStrc* pArchive, const char* szFileName, uint32_t dwFlags,
                                              D2ArchiveStrc** ppArchiveOut);
static int SFILE_GetHashTableIndex(uint32_t nUnused, const char* szFileName, D2ArchiveStrc* pArchive,
                                   uint16_t nLocale, uint8_t nPlatform);

// 1.14d mac: 000136ea
int SFileFileExistsEx(D2ArchiveStrc* pArchive, const char* szFileName, uint32_t dwFlags) {
    char szDiskPath[260];
    int nResult;
    if (*szFileName == '\0' || szFileName == NULL) {
        SErrSetLastError(0x57);
        return 0;
    }
    SFILE_LOCK();
    SFILE_InitCryptTable();
    if ((dwFlags & 3) == 0 || SFILE_FindFileOnDisk(szFileName, dwFlags, szDiskPath) == 0) {
        nResult = GetFileBlockEntry(pArchive, szFileName, dwFlags, NULL) != NULL;
    } else {
        nResult = 2;
    }
    SFILE_UNLOCK();
    return nResult;
}

// 1.14d mac: 000137af
// The MPQ crypt table: 5 x 256 dwords from the generator seed 0x100001.
static void SFILE_InitCryptTable(void) {
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
static uint32_t SFILE_FindFileOnDisk(const char* szFileName, uint32_t dwFlags, char* szOutPath) {
    uint32_t nFound = 0;
    if ((dwFlags & 3) != 0) {
        if (dwFlags & 2) {
            SFILE_ConvertRelativePathName(szFileName, szOutPath, 1);
            uint32_t dwAttributes = GetFileAttributes(szOutPath) >> 4;
            uint32_t nIsDirectory = dwAttributes & 1;
            if ((dwAttributes & 1) == 0) {
                nFound = nIsDirectory ^ 1;
                goto done;
            }
        }
        if (dwFlags & 1) {
            if (*szFileName == '\\' || strstr(szFileName, ":\\") != NULL || strstr(szFileName, "\\\\") != NULL) {
                SStrCopy(szOutPath, szFileName, 0x104);
            } else {
                SFILE_ConvertRelativePathName(szFileName, szOutPath, 0);
            }
            nFound = ((GetFileAttributes(szOutPath) >> 4) & 1) ^ 1;
        }
    }
done:
    return nFound;
}

// 1.14d mac: 00013955
// The block table entry of a file in the given archive, or in the first open archive that has it.
static D2MPQBlockEntryStrc* GetFileBlockEntry(D2ArchiveStrc* pArchive, const char* szFileName, uint32_t dwFlags,
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
                nHashIndex = SFILE_GetHashTableIndex(gnSFileSearchLocale, szFileName, pArchive, gnSFileSearchLocale, s_nSearchPlatform);
                if (nHashIndex != -1) {
                    goto found;
                }
                pArchive = s_tArchiveList.Next(pArchive);
            }
        }
    } else {
        nHashIndex = SFILE_GetHashTableIndex(gnSFileSearchLocale, szFileName, pArchive, gnSFileSearchLocale, s_nSearchPlatform);
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
void SFILE_BuildDefaultBasePath(void) {
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
        return 0;
    }
    *szText = '\0';
    SFILE_LOCK();
    SStrCopy(szText, pFile->szFileName, nBufferSize);
    SFILE_UNLOCK();
    return 1;
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
    uint32_t nNoSource = !((s_dwDirectAccessFlags != 0) | !s_tArchiveList.IsEmpty());
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
static void SFILE_ConvertRelativePathName(const char* szFileName, char* szOutPath, int32_t bStripDir) {
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
static void SFILE_Decrypt(uint32_t* pData, uint32_t nByteLen, uint32_t nSeed) {
    uint32_t nKey = 0xEEEEEEEE;
    uint32_t* pTable = s_pCryptTable;
    for (uint32_t nCount = nByteLen >> 2; nCount != 0; nCount--) {
        nKey += pTable[0x400 + (nSeed & 0xff)];
        uint32_t nDecrypted = (nKey + nSeed) ^ *pData;
        *pData++ = nDecrypted;
        nKey = nDecrypted + 3 + (nKey << 5) + nKey;
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
static BOOL SFILE_ReadFileChecked(int32_t nFileOffset, uint32_t* pFilePosOut, HANDLE hFile, void* pOutputBuf,
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
static BOOL SFILE_SyncReadFile(D2MPQFileStrc* pFileHandle, int32_t nFileOffset, void* pOutputBuf,
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
    uint32_t nNoSource = !((s_dwDirectAccessFlags != 0) | !s_tArchiveList.IsEmpty());
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
        return 0;
    }
    SMemFree(pFile, SFILE_FILE, 0x12ee, 0);
    return 1;
}

static uint32_t SFILE_InternalReadAligned(D2MPQFileStrc* pMPQFile, uint32_t nAlignedOffset, void* pBuffer,
                                          uint32_t nBytes);

// 1.14d mac: 000155db
static uint32_t SFILE_InternalReadAlignedSector(D2MPQFileStrc* pMPQFile, uint32_t nAlignedOffset) {
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
static uint32_t SFILE_DecompressReadCallback(SFileDecompressSource* pState, void** ppData) {
    *ppData = pState->pData;
    return pState->nSize;
}

// 1.14d mac: 00015c32
static int SFILE_DecompressWriteCallback(SFileDecompressSink* pOutput, void* pData, size_t nSize) {
    memcpy(pOutput->pBuffer + pOutput->nOffset, pData, nSize);
    pOutput->nOffset += nSize;
    return 0;
}

// 1.14d mac: 00015c64
// Probes the archive's hash table for the three hashes of a name: the exact locale and
// platform wins; a neutral entry is the fallback.
static int SFILE_GetHashTableIndex(uint32_t nUnused, const char* szFileName, D2ArchiveStrc* pArchive, uint16_t nLocale,
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

// 1.14d mac: 00017278 D2ArchiveStrc::~D2ArchiveStrc[D2]
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

// 1.14d mac: 0001737c D2MPQFileStrc::~D2MPQFileStrc[D2]
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

// 1.14d mac: 00016a72 SFileCritSect::~SFileCritSect[D2]
SFileCritSect::~SFileCritSect() {
    pthread_mutex_destroy(&m_tMutex);
}

typedef void (*PFNSFILEOPENCALLBACK)(const char* szFileName, void* pParam);
// 1.14d mac: 003f0594
static PFNSFILEOPENCALLBACK s_pfnOpenCallback;
// 1.14d mac: 003f0598
static void* s_pOpenCallbackParam;

// The key a name hashes to under hash type 3 (the name past any drive and directory).
static uint32_t SFILE_HashFileKey(const char* szName) {
    uint32_t dwKey = 0x7fed7fed;
    if (szName != NULL) {
        int32_t nSeed2 = 0xEEEEEEEE;
        for (; *szName; szName++) {
            uint32_t nChar = toupper(*szName) & 0xff;
            dwKey = (dwKey + nSeed2) ^ s_pCryptTable[0x300 + nChar];
            nSeed2 = dwKey + 3 + nSeed2 * 0x21 + nChar;
        }
    }
    return dwKey;
}

// 1.14d mac: 00013dd3
// Opens a file from the disk when the flags allow and it is there, else from the archives.
// Returns 2 for a disk file, 1 for an archive file, 0 on failure; with flag 4 the handle is
// the raw file (positioned on the data for an archive file) instead of a record.
int SFileOpenFileEx(D2ArchiveStrc* hArchive, const char* szFileName, uint32_t dwFlags, D2MPQFileStrc** ppFile) {
    char szDiskPath[260];
    D2ArchiveStrc* pArchive;
    uint32_t nResult;
    if (ppFile != NULL) {
        *ppFile = NULL;
        if (*szFileName != '\0' && szFileName != NULL) {
            if (s_pfnOpenCallback != NULL) {
                s_pfnOpenCallback(szFileName, s_pOpenCallbackParam);
            }
            SFILE_LOCK();
            SFILE_InitCryptTable();
            bool bFromArchive = true;
            if ((dwFlags & 3) != 0) {
                if (SFILE_FindFileOnDisk(szFileName, dwFlags, szDiskPath) == 0) {
                    bFromArchive = true;
                } else {
                    HANDLE hDisk = CreateFile(szDiskPath, 0x80000000, 1, 0, 3, 0x8000000, 0);
                    if (hDisk == 0) {
                        bFromArchive = true;
                    } else {
                        bFromArchive = false;
                        void* pHandle = (void*)hDisk;
                        if ((dwFlags & 4) == 0) {
                            D2MPQFileStrc* pFile = s_tFileList.NewNode(STORM_LIST_LINK_AFTER, 0, 0);
                            SStrCopy(pFile->szFileName, szFileName, 0x104);
                            pFile->szFilePath = SStrDup(szDiskPath);
                            pFile->hFile = hDisk;
                            pFile->nRefCount = 1;
                            pFile->bChecksumEnabled = 0;
                            pHandle = pFile;
                        }
                        *ppFile = (D2MPQFileStrc*)pHandle;
                        nResult = 2;
                    }
                }
            }
            if (bFromArchive) {
                D2MPQBlockEntryStrc* pBlockEntry = GetFileBlockEntry(hArchive, szFileName, dwFlags, &pArchive);
                nResult = 0;
                if (pBlockEntry != NULL) {
                    if ((dwFlags & 4) == 0) {
                        const char* szName = szFileName;
                        char* pColon = SStrChrR(szName, ':');
                        if (pColon != NULL) {
                            szName = pColon + 1;
                        }
                        char* pSlash = SStrChrR(szName, '\\');
                        if (pSlash != NULL) {
                            szName = pSlash + 1;
                        }
                        uint32_t dwFileKey = SFILE_HashFileKey(szName);
                        uint32_t nFileSize = pBlockEntry->dwFileSize;
                        if (pBlockEntry->dwFlags & 0x20000) {
                            dwFileKey = ((dwFileKey + pBlockEntry->dwFilePos) - pArchive->nArchiveOffset) ^ nFileSize;
                        }
                        uint32_t* pSectorOffsets = NULL;
                        uint32_t nSectors = ((nFileSize - 1) + pArchive->nSectorSize) / pArchive->nSectorSize;
                        if ((uint8_t)(pBlockEntry->dwFlags >> 8) != 0) {
                            pSectorOffsets = (uint32_t*)SMemAlloc(nSectors * 4 + 4, SFILE_FILE, 0x1118, 0);
                        }
                        D2MPQFileStrc* pFile = s_tFileList.NewNode(STORM_LIST_LINK_AFTER, 0, 0);
                        SStrCopy(pFile->szFileName, szFileName, 0x104);
                        pFile->hFile = 0;
                        pFile->pArchive = pArchive;
                        pFile->pBlockEntry = pBlockEntry;
                        pFile->nDecryptKey = dwFileKey;
                        pFile->nSectorCount = nSectors;
                        pFile->pSectorTable = pSectorOffsets;
                        pFile->pReadBuffer = SMemAlloc(0x1000, SFILE_FILE, 0x1123, 0);
                        pFile->nRefCount = 1;
                        pFile->bChecksumEnabled = pBlockEntry->dwCRC32 != 0;
                        pFile->nChecksumState = 1;
                        if (pArchive != NULL) {
                            pArchive->nRefCount++;
                        }
                        *ppFile = pFile;
                        nResult = 1;
                    } else {
                        HANDLE hRaw = CreateFile(pArchive->szArchivePath, 0x80000000, 1, 0, 3, 0x8000000, 0);
                        *ppFile = (D2MPQFileStrc*)hRaw;
                        nResult = hRaw != 0;
                        if (hRaw == 0) {
                            SErrSetLastError(2);
                        } else {
                            SetFilePointer(hRaw, pBlockEntry->dwFilePos, NULL, 0);
                        }
                    }
                }
            }
            SFILE_UNLOCK();
            return nResult;
        }
    }
    SErrSetLastError(0x57);
    return 0;
}

// 1.14d mac: 00014226
int SFileOpenArchive(const char* szArchiveName, int nPriority, uint32_t dwFlags, D2ArchiveStrc** phArchive) {
    uint16_t aSpec[38];
    char szPath[260];
    if (phArchive != NULL) {
        *phArchive = NULL;
        if (*szArchiveName != '\0' && szArchiveName != NULL) {
            SFILE_LOCK();
            SFILE_InitCryptTable();
            SStrCopy(szPath, szArchiveName, 0x104);
            szPath[259] = '\0';
            if (GetFileAttributes(szPath) & 0x10) {
                if (*szArchiveName != '\\' && strstr(szArchiveName, ":\\") == NULL &&
                    strstr(szArchiveName, "\\\\") == NULL) {
                    SFILE_ConvertRelativePathName(szArchiveName, szPath, 0);
                } else {
                    SStrCopy(szPath, szArchiveName, 0x104);
                }
            }
            uint32_t bOnCd = 0;
            if (FilepathToFSSpec((FSSpec*)aSpec, szPath, 0) == 0) {
                bOnCd = STORM_IsCDVolume(aSpec[0]) & 0xff;
            }
            if ((dwFlags & 1) == 0 || bOnCd != 0) {
                HANDLE hFile = CreateFile(szPath, 0x80000000, 1, 0, 3, 0, 0);
                if (hFile != 0) {
                    D2ArchiveStrc* pArchive = (D2ArchiveStrc*)SMemAlloc(sizeof(D2ArchiveStrc), "", -2, 8);
                    if (pArchive != NULL) {
                        new (pArchive) D2ArchiveStrc;
                    }
                    SStrCopy(pArchive->szArchivePath, szPath, 0x104);
                    pArchive->hFile = hFile;
                    pArchive->nPriority = nPriority;
                    pArchive->nRefCount = 1;
                    if (bOnCd == 0) {
                        pArchive->nArchiveType = (dwFlags & 2) == 0 ? 0 : 1;
                    } else {
                        pArchive->nArchiveType = 3;
                    }
                    pArchive->pMpqHeader = (D2MPQHeaderStrc*)SMemAlloc(0x20, SFILE_FILE, 0xf95, 0);
                    pArchive->nArchiveOffset = 0;
                    pArchive->nArchiveFileSize = GetFileSize(hFile, NULL);
                    int nResult = SFILE_s_OpenArchive(pArchive, dwFlags, bOnCd, phArchive);
                    SFILE_UNLOCK();
                    return nResult;
                }
            } else {
                SErrSetLastError(0xf);
            }
            SFILE_UNLOCK();
            return 0;
        }
    }
    SErrSetLastError(0x57);
    return 0;
}

// 1.14d mac: 000145e7
// Finds the MPQ header (every 0x200 bytes from the start), reads and decrypts the hash and
// block tables, links the archive by priority and reads its (attributes) file.
int SFILE_s_OpenArchive(D2ArchiveStrc* pArchive, uint32_t nFlags, int32_t bAllowEmpty,
                        D2ArchiveStrc** ppArchiveOut) {
    uint8_t* pScanBuffer = (uint8_t*)SMemAlloc(0x1000, SFILE_FILE, 0xfaa, 0);
    uint32_t nOffset = pArchive->nArchiveOffset;
    uint32_t nBufferPos = 0;
    uint32_t nBufferLen = 0;
    uint32_t nBytesRead;
    while (true) {
        if (nBufferLen + nBufferPos <= nOffset) {
            SetFilePointer(pArchive->hFile, nOffset, NULL, 0);
            nBytesRead = 0;
            ReadFile(pArchive->hFile, pScanBuffer, 0x1000, &nBytesRead, 0);
            if (nBytesRead < 0x20 ||
                (nBufferPos = nOffset, nBufferLen = nBytesRead, nOffset = pArchive->nArchiveOffset,
                 pArchive->nArchiveFileSize <= pArchive->nArchiveOffset + 0x20)) {
                if (pArchive != NULL) {
                    delete pArchive;
                }
                SErrSetLastError(0x8510006c);
                SMemFree(pScanBuffer, SFILE_FILE, 0xfc7, 0);
                return 0;
            }
        }
        memcpy(pArchive->pMpqHeader, pScanBuffer + (nOffset - nBufferPos), sizeof(D2MPQHeaderStrc));
        if (pArchive->pMpqHeader->dwMagic == 0x1a51504d && pArchive->pMpqHeader->dwHeaderSize > 0x1f) {
            break;
        }
        nOffset = pArchive->nArchiveOffset + 0x200;
        pArchive->nArchiveOffset = nOffset;
    }
    SMemFree(pScanBuffer, SFILE_FILE, 0xfee, 0);

    D2MPQHeaderStrc* pHeader = pArchive->pMpqHeader;
    pArchive->nSectorSize = 0x200 << (pHeader->wSectorSizeShift & 0x1f);
    pArchive->pSectorBuffer = SMemAlloc(pArchive->nSectorSize, SFILE_FILE, 0xff3, 0);
    pArchive->pHashTable = (D2MPQHashEntryStrc*)SMemAlloc(pHeader->dwHashTableSize << 4, SFILE_FILE, 0xffc, 0);
    SFILE_ReadFileChecked(pHeader->dwHashTablePos + pArchive->nArchiveOffset, &pArchive->nFilePos, pArchive->hFile,
                          pArchive->pHashTable, pHeader->dwHashTableSize << 4, &nBytesRead, pArchive->szArchivePath);
    SFILE_Decrypt((uint32_t*)pArchive->pHashTable, pHeader->dwHashTableSize << 4,
                  SFILE_HashFileKey("(hash table)"));

    pArchive->pBlockTable =
        (D2MPQBlockEntryStrc*)SMemAlloc(pHeader->dwBlockTableSize * 0x1c, SFILE_FILE, 0x101b, 0);
    SFILE_ReadFileChecked(pHeader->dwBlockTablePos + pArchive->nArchiveOffset, &pArchive->nFilePos, pArchive->hFile,
                          pArchive->pBlockTable, pHeader->dwBlockTableSize << 4, &nBytesRead,
                          pArchive->szArchivePath);
    SFILE_Decrypt((uint32_t*)pArchive->pBlockTable, pHeader->dwBlockTableSize << 4,
                  SFILE_HashFileKey("(block table)"));

    // The disk entries are 0x10 bytes; spread them out to make room for the attributes.
    D2MPQBlockEntryStrc* pBlocks = pArchive->pBlockTable;
    uint32_t nBlocks = pHeader->dwBlockTableSize;
    int nBlock = nBlocks - 1;
    D2MPQBlockEntryStrc* pDest = pBlocks + nBlock;
    D2MPQBlockEntryStrc* pFirst = pDest;
    if (nBlocks > 1) {
        void* pSource = (char*)pBlocks + nBlock * 0x10;
        do {
            MoveMemory(pDest, pSource, 0x10);
            pDest->ftFileTime.dwHighDateTime = 0;
            pDest->ftFileTime.dwLowDateTime = 0;
            pDest->dwCRC32 = 0;
            pDest--;
            pSource = (char*)pSource - 0x10;
            nBlock--;
            pFirst = pBlocks;
        } while (nBlock != 0);
    }
    if (nBlocks != 0) {
        pFirst->ftFileTime.dwHighDateTime = 0;
        pFirst->ftFileTime.dwLowDateTime = 0;
        pFirst->dwCRC32 = 0;
    }
    if (pArchive->nArchiveOffset != 0) {
        nBlocks = pHeader->dwBlockTableSize;
        for (uint32_t i = 0; i < nBlocks; i++) {
            if (pBlocks[i].dwFilePos != 0) {
                pBlocks[i].dwFilePos += pArchive->nArchiveOffset;
                nBlocks = pHeader->dwBlockTableSize;
            }
        }
    }

    // Higher priority first.
    D2ArchiveStrc* pRef = NULL;
    for (D2ArchiveStrc* pNext = s_tArchiveList.Head(); pNext != NULL; pNext = s_tArchiveList.Next(pNext)) {
        if (pNext->nPriority <= pArchive->nPriority) {
            pRef = pNext;
            break;
        }
    }
    s_tArchiveList.LinkNode(pArchive, STORM_LIST_LINK_BEFORE, pRef);
    if (pArchive->nArchiveType == 0) {
        if ((nFlags & 1) != 0 && bAllowEmpty == 0) {
            SFileCloseArchive(pArchive);
            SErrSetLastError(0xf);
            return 0;
        }
    } else {
        SFILE_CreateCdThread();
    }
    *ppArchiveOut = pArchive;
    SFILE_UNLOCK();

    // The (attributes) file: CRCs and file times for the blocks.
    D2MPQBlockEntryStrc* pTable = pArchive->pBlockTable;
    int nCount = pArchive->pMpqHeader->dwBlockTableSize;
    D2MPQFileStrc* pAttrFile;
    if (SFileOpenFileEx(pArchive, "(attributes)", 0, &pAttrFile) != 0) {
        DWORD nSize = SFileGetFileSize(pAttrFile, NULL);
        uint8_t* pAttr = (uint8_t*)SMemAlloc(nSize, SFILE_FILE, 0x55f, 0);
        if (SFileReadFileEx2(pAttrFile, pAttr, nSize, NULL, NULL, 0, NULL) != 0 && nSize > 7) {
            DWORD nExpected = 8;
            uint32_t nHasCrc = ((uint32_t*)pAttr)[1] & 1;
            if (nHasCrc != 0) {
                nExpected = nCount * 4 + 8;
            }
            uint32_t nHasTime = ((uint32_t*)pAttr)[1] & 2;
            if (nHasTime != 0) {
                nExpected += nCount * 8;
            }
            if (nSize == nExpected) {
                uint32_t* pCrc = (uint32_t*)(pAttr + 8);
                if (nHasCrc != 0 && nCount != 0) {
                    for (int i = 0; i != nCount; i++) {
                        pTable[i].dwCRC32 = pCrc[i];
                    }
                    pCrc = (uint32_t*)(pAttr + nCount * 4 + 8);
                }
                if (nHasTime != 0 && nCount != 0) {
                    FILETIME* pTime = (FILETIME*)pCrc;
                    for (int i = 0; i != nCount; i++) {
                        pTable[i].ftFileTime = pTime[i];
                    }
                }
            }
        }
        if (pAttr != NULL) {
            SMemFree(pAttr, SFILE_FILE, 0x57f, 0);
        }
        SFileCloseFile(pAttrFile);
    }
    SFILE_LOCK();
    return 1;
}

SFileCritSect::SFileCritSect() {
    CRITSECT_Init(&m_tMutex);
}

// 1.14d mac: 000176a0
void CRITSECT_Init(pthread_mutex_t* pMutex) {
    pthread_mutexattr_t tAttr;
    pthread_mutexattr_init(&tAttr);
    pthread_mutexattr_settype(&tAttr, PTHREAD_MUTEX_RECURSIVE);
    pthread_mutex_init(pMutex, &tAttr);
    pthread_mutexattr_destroy(&tAttr);
}

// What a caller hands SFileReadFileEx2 for an asynchronous read: where in the file to read
// (0x08) and the event to signal when it is done (0x10).
struct SFileOverlapped {
    uint8_t pad00[8];
    uint32_t nOffset;  // 0x08
    uint8_t pad0c[4];
    HANDLE hEvent;     // 0x10
};

static D2StormAsyncOpStrc* IssueRequest(D2MPQFileStrc* pFile, uint32_t nFileOffset, void* pChunkWritePtr,
                                              void* pReadBufferBase, IDirectSoundBuffer* pStream,
                                              uint32_t nStreamPos, D2StormAudioStreamRec* pParentStream,
                                              uint32_t nBytesToRead, int32_t nTimeoutMs, int32_t nPriority,
                                              HANDLE hCompletionEvent, BOOL bAutoFree, BOOL bQueueImmediately,
                                              void* pCompletion);
static uint32_t SFILE_InternalReadFile(uint32_t nUnused, uint32_t nOffset, D2MPQFileStrc* pFile,
                                             void* pDestination, uint32_t nBytes);

// 1.14d mac: 00012ac1
// Reads from an open file. A synchronous read of a disk file goes straight to the disk. An
// archive file is read through its read-ahead buffer; what is left is read directly or, for a
// CD or an asynchronous read, queued in chunks for the I/O thread, which is waited for unless
// the read is asynchronous.
int SFileReadFileEx2(D2MPQFileStrc* hFile, void* pBuffer, uint32_t nBytesToRead, size_t* pnBytesRead,
                     void* pOverlappedArg, int nPriority, void* ppCompletion) {
    SFileOverlapped* pOverlapped = (SFileOverlapped*)pOverlappedArg;
    uint8_t* pDest = (uint8_t*)pBuffer;
    if (pnBytesRead != NULL) {
        *pnBytesRead = 0;
    }
    if (pBuffer == NULL) {
        SErrSetLastError(0x57);
        return 0;
    }
    SFILE_LOCK();
    if (nBytesToRead == 0) {
        if (pOverlapped != NULL && pOverlapped->hEvent != 0) {
            SetEvent(pOverlapped->hEvent);
        }
        SFILE_UNLOCK();
        return 1;
    }
    if (hFile->hFile != 0 && pOverlapped == NULL) {
        BOOL bResult = SFILE_SyncReadFile(hFile, -1, pBuffer, nBytesToRead, (uint32_t*)pnBytesRead);
        SFILE_UNLOCK();
        return bResult;
    }
    if (hFile->pReadBuffer != NULL) {
        uint32_t nConsumed = hFile->nBytesRead;
        if (nConsumed < hFile->nBufferSize && pOverlapped == NULL) {
            uint32_t nCached = hFile->nBufferSize - nConsumed;
            if (nBytesToRead < nCached) {
                nCached = nBytesToRead;
            }
            if (0 < (int)nCached) {
                memmove(pBuffer, (uint8_t*)hFile->pReadBuffer + nConsumed, nCached);
                nConsumed = hFile->nBytesRead;
            }
            hFile->nFilePos = hFile->nFilePos + nCached;
            hFile->nBytesRead = nConsumed + nCached;
            if (pnBytesRead != NULL) {
                *pnBytesRead = nCached;
            }
            if (nCached == nBytesToRead) {
                if (hFile->bChecksumEnabled != 0 && hFile->nChecksumState == 4 &&
                    hFile->dwComputedCRC != hFile->pBlockEntry->dwCRC32) {
                    SErrDisplayError(0x85100083, hFile->szFileName, -4, NULL, 0, 1);
                }
                SFILE_UNLOCK();
                return 1;
            }
            nBytesToRead -= nCached;
            pDest = (uint8_t*)pBuffer + nCached;
        }
    }
    uint32_t nReadLen = nBytesToRead;
    uint8_t* pReadBase = pDest;
    if (pOverlapped == NULL) {
        uint32_t nSector = hFile->pArchive->nSectorSize;
        uint32_t nToBoundary = nSector - ((nSector - 1) & hFile->nFilePos);
        uint32_t nLimit = 0x1000;
        if (nToBoundary < 0x1001) {
            nLimit = nToBoundary;
        }
        if (nBytesToRead < nLimit) {
            pReadBase = (uint8_t*)hFile->pReadBuffer;
            nReadLen = nLimit;
        }
    }
    if (nReadLen == 0) {
        if (pOverlapped != NULL && pOverlapped->hEvent != 0) {
            SetEvent(pOverlapped->hEvent);
        }
        SFILE_UNLOCK();
        return 1;
    }
    uint32_t* pPos = pOverlapped != NULL ? &pOverlapped->nOffset : &hFile->nFilePos;
    uint32_t nOffset = *pPos;
    D2ArchiveStrc* pArchive = hFile->pArchive;
    if (hFile->hFile == 0 && pArchive->nArchiveType == 0) {
        nReadLen = SFILE_InternalReadFile(nReadLen, nOffset, hFile, pReadBase, nReadLen);
        if (hFile->bChecksumEnabled != 0 && hFile->nChecksumState == 2 &&
            hFile->nExpectedFileSize == hFile->pBlockEntry->dwFileSize) {
            STORM_Crc32(NULL, 0, &hFile->dwComputedCRC, 4);
            hFile->nChecksumState = 4;
        }
        if (pOverlapped != NULL && pOverlapped->hEvent != 0) {
            SetEvent(pOverlapped->hEvent);
        }
    } else {
        uint32_t nChunkSize = 0x100000;
        if (pArchive != NULL &&
            ((uint32_t)(pArchive->nArchiveType - 2) < 2 || (pArchive->nArchiveType == 1 && pOverlapped != NULL))) {
            nChunkSize = s_nDataChunkSize;
        }
        SFILE_CreateCdThread();
        HANDLE hEvent = pOverlapped != NULL ? pOverlapped->hEvent : CreateEventA(0, 1, 0, NULL);
        uint32_t nRequests = ((nReadLen - 1) + nChunkSize) / nChunkSize;
        D2StormAsyncOpStrc* pRequest = NULL;
        uint32_t nQueued = 0;
        if (nRequests != 0) {
            int32_t nReqPriority = pOverlapped != NULL ? nPriority : 1;
            int32_t nTimeoutMs = pOverlapped == NULL ? 0 : 0x7fffffff;
            uint32_t nPos = 0;
            for (uint32_t nIndex = 0; nIndex < nRequests; nIndex++) {
                uint32_t nThis = nChunkSize < nReadLen ? nChunkSize : nReadLen;
                bool bLast = nIndex == nRequests - 1;
                nQueued += bLast ? 0 : nThis;
                pRequest = IssueRequest(hFile, nOffset + nPos, pReadBase + nPos, pReadBase, NULL, 0, NULL, nThis,
                                        nTimeoutMs, nReqPriority, bLast ? hEvent : 0, pOverlapped != NULL || !bLast,
                                        bLast, bLast ? ppCompletion : NULL);
                nReadLen -= nChunkSize;
                nPos += nChunkSize;
            }
        }
        SFILE_UNLOCK();
        if (pOverlapped != NULL) {
            return 1;
        }
        WaitForSingleObject(hEvent, 0xffffffff);
        SFILE_LOCK();
        CloseHandle(hEvent);
        nReadLen = nQueued + pRequest->nChunkBytesRead;
        if (pRequest != NULL) {
            delete pRequest;
        }
    }
    uint32_t nGot = nReadLen;
    if (nBytesToRead < nReadLen) {
        nGot = nBytesToRead;
    }
    if (pReadBase != pDest) {
        if (0 < (int)nGot) {
            memmove(pDest, pReadBase, nGot);
        }
        hFile->nBytesRead = nGot;
        hFile->nBufferSize = nReadLen;
    }
    if (pOverlapped == NULL) {
        hFile->nFilePos += nGot;
    }
    if (pnBytesRead != NULL) {
        *pnBytesRead = *pnBytesRead + nGot;
    }
    if (nGot != nBytesToRead) {
        SErrSetLastError(0x26);
        SFILE_UNLOCK();
        return 0;
    }
    if (hFile->bChecksumEnabled != 0 && hFile->nChecksumState == 4 &&
        hFile->dwComputedCRC != hFile->pBlockEntry->dwCRC32) {
        SErrDisplayError(0x85100083, hFile->szFileName, -4, NULL, 0, 1);
    }
    SFILE_UNLOCK();
    return 1;
}

// 1.14d mac: 00014e4e
// Sets the priority of the queued requests that read into a buffer. A request that becomes
// urgent absorbs the request just before it when the two are one contiguous read.
void SFilePrioritizeRequest(void* pReadBufferBase, int nPriority) {
    if (pReadBufferBase == NULL) {
        SErrSetLastError(0x57);
        return;
    }
    SFILE_LOCK();
    D2StormAsyncOpStrc* pRequest = s_tRequestQueue.Head();
    D2StormAsyncOpStrc* pPrevRequest = NULL;
    while (pRequest != NULL) {
        D2StormAsyncOpStrc* pMatched = pPrevRequest;
        if (pRequest->pReadBufferBase == pReadBufferBase) {
            int nOldPriority = pRequest->nPriority;
            pRequest->nPriority = nPriority;
            pMatched = pRequest;
            if (0 < nPriority && pPrevRequest != NULL && nOldPriority < 1 &&
                pPrevRequest->pReadBufferBase == pReadBufferBase) {
                if (pPrevRequest->nBytesToRead + pPrevRequest->nFilePos == pRequest->nFilePos &&
                    pPrevRequest->nBytesToRead + pRequest->nBytesToRead < 0x100001 && pPrevRequest->bAutoFree != 0 &&
                    pRequest->nChunkBytesRead == 0 && pPrevRequest->nChunkBytesRead == 0 &&
                    pPrevRequest->pStream == NULL && pRequest->pStream == NULL &&
                    pPrevRequest->pParentStream == NULL && pRequest->pParentStream == NULL &&
                    pPrevRequest->hCompletionEvent == 0 && pPrevRequest->pCompletion == NULL) {
                    pRequest->nFilePos = pPrevRequest->nFilePos;
                    pRequest->nSeekFilePos = pPrevRequest->nSeekFilePos;
                    pRequest->dwDueTick = pPrevRequest->dwDueTick;
                    pRequest->pChunkWritePtr = pPrevRequest->pChunkWritePtr;
                    pRequest->nBytesToRead = pRequest->nBytesToRead + pPrevRequest->nBytesToRead;
                    delete pPrevRequest;
                }
            }
        }
        pRequest = s_tRequestQueue.Next(pRequest);
        pPrevRequest = pMatched;
    }
    SFILE_UNLOCK();
    if (0 < nPriority) {
        SetEvent(s_hCdEvent);
    }
}

// 1.14d mac: 0001505d
// Queues a read for the I/O thread.
static D2StormAsyncOpStrc* IssueRequest(D2MPQFileStrc* pFile, uint32_t nFileOffset, void* pChunkWritePtr,
                                              void* pReadBufferBase, IDirectSoundBuffer* pStream,
                                              uint32_t nStreamPos, D2StormAudioStreamRec* pParentStream,
                                              uint32_t nBytesToRead, int32_t nTimeoutMs, int32_t nPriority,
                                              HANDLE hCompletionEvent, BOOL bAutoFree, BOOL bQueueImmediately,
                                              void* pCompletion) {
    D2StormAsyncOpStrc* pRequest;
    if (pChunkWritePtr == NULL && pStream == NULL) {
        SErrSetLastError(0x57);
        pRequest = NULL;
    } else {
        SFILE_LOCK();
        pRequest = s_tRequestQueue.NewNode(STORM_LIST_LINK_BEFORE, 0, 0);
        uint32_t nTick = GetTickCount();
        pRequest->hCompletionEvent = hCompletionEvent;
        pRequest->pFile = pFile;
        pRequest->nFilePos = nFileOffset;
        if (pFile->hFile == 0) {
            nFileOffset = (nFileOffset >> 1) + pFile->pBlockEntry->dwFilePos;
        }
        pRequest->nSeekFilePos = nFileOffset;
        pRequest->dwDueTick = nTick + nTimeoutMs;
        pRequest->nPriority = nPriority;
        pRequest->pChunkWritePtr = pChunkWritePtr;
        pRequest->pReadBufferBase = pReadBufferBase;
        pRequest->pStream = pStream;
        pRequest->nStreamPos = nStreamPos;
        pRequest->pParentStream = pParentStream;
        pRequest->nBytesToRead = nBytesToRead;
        pRequest->bAutoFree = bAutoFree;
        pRequest->pCompletion = pCompletion;
        if (pFile != NULL) {
            pFile->nRefCount++;
        }
        if (pParentStream != NULL) {
            pParentStream->nRefCount++;
        }
        SFILE_UNLOCK();
        if (bQueueImmediately != 0) {
            SetEvent(s_hCdEvent);
        }
    }
    return pRequest;
}

// 1.14d mac: 0001516c
// Reads from an archive file the sector way: the unaligned head and tail through the
// archive's one-sector cache, the aligned middle straight into the destination; the file's
// CRC runs along while the reads are in order.
static uint32_t SFILE_InternalReadFile(uint32_t nUnused, uint32_t nOffset, D2MPQFileStrc* pFile,
                                             void* pDestination, uint32_t nBytes) {
    uint8_t* pOutput = (uint8_t*)pDestination;
    uint32_t nFileSize = pFile->pBlockEntry->dwFileSize;
    uint32_t nResult = 0;
    uint32_t nRemaining = nFileSize - nOffset;
    if (nOffset <= nFileSize && nRemaining != 0) {
        if (nBytes < nRemaining) {
            nRemaining = nBytes;
        }
        if (pFile->bChecksumEnabled != 0 && nOffset == 0) {
            STORM_Crc32(NULL, 0, &pFile->dwComputedCRC, 1);
            pFile->nExpectedFileSize = 0;
            pFile->nChecksumState = 2;
        }
        D2ArchiveStrc* pArchive = pFile->pArchive;
        uint32_t nSector = pArchive->nSectorSize;
        uint32_t nHead;
        uint8_t* pDest;
        if ((nOffset & (nSector - 1)) == 0) {
            nHead = 0;
            pDest = pOutput;
        } else {
            if (pFile->bChecksumEnabled != 0 && pFile->nChecksumState != 1 && pFile->nExpectedFileSize != nOffset) {
                pFile->nChecksumState = 1;
                nSector = pArchive->nSectorSize;
            }
            uint32_t nSectorBytes = SFILE_InternalReadAlignedSector(pFile, -nSector & nOffset);
            uint32_t nSectorSize = pFile->pArchive->nSectorSize;
            uint32_t nInSector = (nSectorSize - 1) & nOffset;
            nHead = nSectorSize - nInSector;
            if (nRemaining < nHead) {
                nHead = nRemaining;
            }
            if (0 < (int)nHead) {
                memmove(pOutput, (uint8_t*)pFile->pArchive->pSectorBuffer + nInSector, nHead);
            }
            if (pFile->bChecksumEnabled != 0 && pFile->nChecksumState == 2) {
                STORM_Crc32(pOutput, nHead, &pFile->dwComputedCRC, 2);
                pFile->nExpectedFileSize = pFile->nExpectedFileSize + nHead;
            }
            pArchive = pFile->pArchive;
            if (nSectorBytes != pArchive->nSectorSize || (nRemaining -= nHead, nRemaining == 0)) {
                uint32_t nSkip = (pArchive->nSectorSize - 1) & nOffset;
                uint32_t nAvailable = nSectorBytes - nSkip;
                if (nSectorBytes < nSkip) {
                    return 0;
                }
                if (nAvailable <= nHead) {
                    return nAvailable;
                }
                return nHead;
            }
            nOffset += nHead;
            pDest = pOutput + nHead;
        }
        uint32_t nMiddle = 0;
        uint32_t nTail = 0;
        if (nRemaining != 0) {
            if (pFile->bChecksumEnabled != 0 && pFile->nChecksumState != 1 && pFile->nExpectedFileSize != nOffset) {
                pFile->nChecksumState = 1;
                nSector = pArchive->nSectorSize;
            }
            nMiddle = -nSector & nRemaining;
            uint32_t nRead = SFILE_InternalReadAligned(pFile, nOffset, pDest, nMiddle);
            if (pFile->bChecksumEnabled != 0 && pFile->nChecksumState == 2) {
                STORM_Crc32(pDest, nRead, &pFile->dwComputedCRC, 2);
                pFile->nExpectedFileSize = pFile->nExpectedFileSize + nRead;
            }
            if (nMiddle != nRead || (nRemaining -= nMiddle, nRemaining == 0)) {
                return nRead + nHead;
            }
            if (pFile->bChecksumEnabled != 0 && pFile->nChecksumState != 1 &&
                pFile->nExpectedFileSize != nOffset + nMiddle) {
                pFile->nChecksumState = 1;
            }
            nTail = SFILE_InternalReadAlignedSector(pFile, nOffset + nMiddle);
            if (0 < (int)nRemaining) {
                memmove(pDest + nMiddle, pFile->pArchive->pSectorBuffer, nRemaining);
            }
            if (nRemaining < nTail) {
                nTail = nRemaining;
            }
            if (pFile->bChecksumEnabled != 0 && pFile->nChecksumState == 2) {
                STORM_Crc32(pDest + nMiddle, nTail, &pFile->dwComputedCRC, 2);
                pFile->nExpectedFileSize = pFile->nExpectedFileSize + nTail;
            }
        }
        nResult = nMiddle + nHead + nTail;
    }
    return nResult;
}

// 1.14d mac: 00017484 D2StormAudioStreamRec::~D2StormAudioStreamRec[D2]
D2StormAudioStreamRec::~D2StormAudioStreamRec() {
    if (pMpqFile != NULL) {
        if (--pMpqFile->nRefCount == 0) {
            delete pMpqFile;
        }
    }
    if (pStream != NULL && bAllocatedStream != 0) {
        pStream->Release();
    }
    m_link.Unlink();
}

// 1.14d mac: 000174ec D2StormAsyncOpStrc::~D2StormAsyncOpStrc[D2]
D2StormAsyncOpStrc::~D2StormAsyncOpStrc() {
    if (pFile != NULL) {
        if (--pFile->nRefCount == 0) {
            delete pFile;
        }
    }
    if (pParentStream != NULL) {
        if (--pParentStream->nRefCount == 0) {
            delete pParentStream;
        }
    }
    m_link.Unlink();
}

// 1.14d mac: 003f05a4
static uint32_t s_nWaveChunkSize;
// The Storm option block (SSystem): the third dword is the size of one wave streaming chunk,
// the fourth asks for reads cut at sector boundaries.
extern uint32_t gnSSystemWaveBufferSize;
extern uint32_t gnSSystemAsyncReadSize;

struct SFileDsBufferDesc {
    uint32_t dwSize;
    uint32_t dwFlags;
    uint32_t dwBufferBytes;
    uint32_t dwReserved;
    void* pWaveFormat;
};

struct SFileWaveFormat {
    uint16_t wFormatTag;
    uint16_t nChannels;
    uint32_t nSamplesPerSec;
    uint32_t nAvgBytesPerSec;
    uint16_t nBlockAlign;
    uint16_t wBitsPerSample;
    uint16_t cbSize;
};

// 1.14d mac: 00012465
// Starts streaming a RIFF wave file out of an MPQ through a DirectSound buffer, the caller's
// or one made here.
BOOL SFileDdaBeginEx(D2MPQFileStrc* hFile, uint32_t dwBufferBytes, uint32_t dwFlags, uint32_t dwInitialPos,
                     int32_t lVolume, int32_t lPan, IDirectSoundBuffer* lpDSB) {
    IDirectSoundBuffer* pBuffer = lpDSB;
    DWORD dwErrCode;
    if (dwBufferBytes == 0) {
        dwErrCode = 0x57;
    } else if (s_pDirectSound == NULL) {
        dwErrCode = 0x85100071;
    } else {
        SFILE_LOCK();
        if (hFile->bStreamActive != 0) {
            SFileDdaEnd(hFile);
        }
        if (((s_dwDirectAccessFlags | dwFlags) & 1) == 0 && (hFile->hFile != 0 || hFile->pArchive == NULL)) {
            SErrSetLastError(0x8510006f);
            SFILE_UNLOCK();
            return 0;
        }
        s_nWaveChunkSize = gnSSystemWaveBufferSize;
        if (s_nWaveChunkSize == 0) {
            gnSSystemWaveBufferSize = 0x4000;
            s_nWaveChunkSize = 0x4000;
        }
        if (s_pWaveBuffer == NULL) {
            s_pWaveBuffer = SMemAlloc(s_nWaveChunkSize, SFILE_FILE, 0xba3, 0);
        }
        D2ArchiveStrc* pArchive = hFile->pArchive;
        if (pArchive != NULL && (uint32_t)pArchive->nArchiveType < 2) {
            pArchive->nArchiveType = 2;
        }
        SFILE_UNLOCK();
        uint32_t nChunk = s_nWaveChunkSize;
        uint32_t nRemainder = (nChunk - 1) & dwBufferBytes;
        if (nRemainder != 0) {
            dwBufferBytes = (dwBufferBytes + nChunk) - nRemainder;
        }
        SFileSetFilePointer(hFile, 0, NULL, 0);
        uint32_t nBufferSize = nChunk * 2;
        if (nChunk * 2 <= dwBufferBytes) {
            nBufferSize = dwBufferBytes;
        }
        int32_t aRiffHeader[3];
        if (SFileReadFileEx2(hFile, aRiffHeader, 0xc, NULL, NULL, 0, NULL) != 0 && aRiffHeader[0] == 0x46464952 &&
            aRiffHeader[2] == 0x45564157) {
            int aChunk[2];
            aChunk[1] = 0;
            aChunk[0] = 0;
            uint32_t nFound = SFILE_FindChunk(hFile, 0x20746d66, aChunk);
            uint32_t nFormatSize = aChunk[0];
            SFileWaveFormat tFormat;
            if (nFound != 0 && nFormatSize > 0xf &&
                SFileReadFileEx2(hFile, &tFormat, 0x10, NULL, NULL, 0, NULL) != 0 &&
                SFileSetFilePointer(hFile, nFormatSize - 0x10, NULL, 1) != 0xFFFFFFFF) {
                tFormat.cbSize = 0;
                nFound = SFILE_FindChunk(hFile, 0x61746164, aChunk);
                int nDataOffset = aChunk[1];
                uint32_t nDataSize = aChunk[0];
                if (nFound != 0) {
                    uint32_t nStartPos = dwInitialPos == 0 ? 0 : dwInitialPos % nDataSize;
                    bool bOwnBuffer = pBuffer == NULL;
                    if (bOwnBuffer) {
                        SFileDsBufferDesc tDesc;
                        tDesc.dwReserved = 0;
                        tDesc.dwFlags = (dwFlags & 0x60) | 0x80;
                        tDesc.dwSize = 0x14;
                        tDesc.pWaveFormat = &tFormat;
                        tDesc.dwBufferBytes = nBufferSize;
                        s_pDirectSound->CreateSoundBuffer(&tDesc, &pBuffer);
                        if (pBuffer == NULL) {
                            return 0;
                        }
                    }
                    if (lVolume != 0 && lVolume != 0x7fffffff) {
                        pBuffer->SetVolume(lVolume);
                    }
                    if (lPan != 0 && lPan != 0x7fffffff) {
                        pBuffer->SetPan(lPan);
                    }
                    SFILE_LOCK();
                    D2StormAudioStreamRec* pStream = s_tStreamList.NewNode(STORM_LIST_LINK_BEFORE, 0, 0);
                    pStream->pMpqFile = hFile;
                    pStream->nBufferSize = nBufferSize;
                    pStream->dwAvgBytesPerSec = tFormat.nAvgBytesPerSec;
                    pStream->nNoCleanupFlag = (dwFlags >> 0x12) & 1;
                    pStream->nStreamType = 0;
                    pStream->nDataChunkOffset = nDataOffset;
                    pStream->nDataChunkSize = nDataSize;
                    pStream->nInitialPos = nStartPos;
                    pStream->nCurrentPos = nStartPos;
                    pStream->nBytePos = nStartPos;
                    pStream->lVolume = lVolume;
                    pStream->lPan = lPan;
                    pStream->pStream = pBuffer;
                    pStream->bAllocatedStream = bOwnBuffer;
                    pStream->bIs16Bit = (tFormat.wBitsPerSample == 8) << 7;
                    pStream->nRefCount = 1;
                    if (hFile != NULL) {
                        hFile->nRefCount++;
                    }
                    pBuffer->SetCurrentPosition(0);
                    hFile->bStreamActive = 1;
                    SFILE_UNLOCK();
                    SFILE_CreateCdThread();
                    SetEvent(s_hCdEvent);
                    return 1;
                }
            }
        }
        dwErrCode = 0xd;
    }
    SErrSetLastError(dwErrCode);
    return 0;
}

typedef blast_out PFNBLASTWRITE;
typedef blast_in PFNBLASTREAD;

// What the blast decompressor's callbacks share: the destination (buffer, running offset) and
// the source (data at 0x0C, size at 0x14).
struct SFileBlastContext {
    uint8_t* pOutput;     // 0x00
    int nOutOffset;       // 0x04
    uint32_t nOutLimit;   // 0x08
    void* pInput;         // 0x0C
    uint32_t n10;         // 0x10
    uint32_t nInSize;     // 0x14
};

// 1.14d mac: 00015661
// Reads whole sectors, starting on a sector boundary, into pDstBuf: loads the sector offset table
// of a compressed file on first use, decrypts, and decompresses sector by sector. Returns the
// bytes made.
static uint32_t SFILE_InternalReadAligned(D2MPQFileStrc* pFile, uint32_t nFileOffset, void* pDestination,
                                                uint32_t nReadLen) {
    uint8_t* pDstBuf = (uint8_t*)pDestination;
    D2ArchiveStrc* pArchive = pFile->pArchive;
    uint32_t nResult;
    if ((nFileOffset & (pArchive->nSectorSize - 1)) != 0) {
        SErrSetLastError(0x57);
        return 0;
    }
    nResult = 0;
    if (nReadLen == 0) {
        return 0;
    }
    D2MPQBlockEntryStrc* pBlock = pFile->pBlockEntry;
    uint8_t cCompress = (uint8_t)(pBlock->dwFlags >> 8);
    if (cCompress != 0 && pFile->bSectorsLoaded == 0) {
        uint32_t nTableRead = 0;
        if (!SFILE_ReadFileChecked(pBlock->dwFilePos, &pArchive->nFilePos, pArchive->hFile, pFile->pSectorTable,
                                   pFile->nSectorCount * 4 + 4, &nTableRead, pArchive->szArchivePath)) {
            return 0;
        }
        if ((pFile->pBlockEntry->dwFlags & 0x10000) != 0) {
            SFILE_Decrypt(pFile->pSectorTable, nTableRead, pFile->nDecryptKey - 1);
        }
        uint32_t nIndex = 0;
        while (true) {
            if (pFile->nSectorCount <= nIndex) {
                pFile->bSectorsLoaded = 1;
                pBlock = pFile->pBlockEntry;
                cCompress = (uint8_t)(pBlock->dwFlags >> 8);
                break;
            }
            if (pFile->pSectorTable[nIndex] > pFile->pSectorTable[nIndex + 1]) {
                return 0;
            }
            nIndex++;
        }
    }

    uint8_t* pReadBuffer = pDstBuf;
    uint32_t nToRead = nReadLen;
    uint32_t nBaseOffset = nFileOffset;
    if (cCompress != 0) {
        uint32_t nSector = pFile->pArchive->nSectorSize;
        nBaseOffset = pFile->pSectorTable[nFileOffset / nSector];
        nToRead = pFile->pSectorTable[(nReadLen + nFileOffset) / nSector] - nBaseOffset;
        pReadBuffer = (uint8_t*)SMemAlloc(nToRead + 4, SFILE_FILE, 0x360, 0x10000000);
        pBlock = pFile->pBlockEntry;
    }
    nResult = nReadLen;
    uint32_t nBytesGot = 0;
    pArchive = pFile->pArchive;
    SFILE_ReadFileChecked(pBlock->dwFilePos + nBaseOffset, &pArchive->nFilePos, pArchive->hFile, pReadBuffer,
                          nToRead, &nBytesGot, pArchive->szArchivePath);
    uint32_t dwFlags = pFile->pBlockEntry->dwFlags;
    if (nBytesGot < nToRead) {
        if ((uint8_t)(dwFlags >> 8) == 0) {
            nResult = nBytesGot;
        } else {
            uint32_t nSector = pFile->pArchive->nSectorSize;
            nResult = 0;
            uint32_t nIndex = nFileOffset / nSector + 1;
            while (nIndex <= pFile->nSectorCount) {
                if (nBytesGot < pFile->pSectorTable[nIndex] - nBaseOffset) {
                    break;
                }
                nResult += nSector;
                nIndex++;
            }
        }
    }
    if ((dwFlags & 0x10000) != 0) {
        uint32_t nSectors = pFile->nSectorCount;
        uint32_t nSector = pFile->pArchive->nSectorSize;
        uint32_t nIndex = nFileOffset / nSector;
        if (nIndex < nSectors) {
            int nPos = 0;
            int nCountdown = -(int)(((nResult - 1) + nSector) / nSector);
            do {
                if (nCountdown == 0) {
                    break;
                }
                uint32_t nLength;
                if ((uint8_t)(pFile->pBlockEntry->dwFlags >> 8) == 0) {
                    uint32_t nSectorSize = pFile->pArchive->nSectorSize;
                    uint32_t nLeft = nResult - nPos;
                    if (nSectorSize < nLeft) {
                        nLeft = nSectorSize;
                    }
                    nLength = nLeft;
                    if (nIndex == nSectors - 1) {
                        uint32_t nTail = (nSectorSize - 1) & pFile->pBlockEntry->dwFileSize;
                        if (nTail != 0) {
                            nLength = nTail;
                            if (nLeft < nTail) {
                                nLength = nLeft;
                            }
                        }
                    }
                } else {
                    nLength = pFile->pSectorTable[nIndex + 1] - pFile->pSectorTable[nIndex];
                }
                SFILE_Decrypt((uint32_t*)(pReadBuffer + nPos), nLength & 0xfffffffc, pFile->nDecryptKey + nIndex);
                nPos += nLength;
                nCountdown++;
                nIndex++;
                nSectors = pFile->nSectorCount;
            } while (nIndex < nSectors);
        }
        dwFlags = pFile->pBlockEntry->dwFlags;
    }
    if ((uint8_t)(dwFlags >> 8) != 0) {
        uint32_t nSectors = pFile->nSectorCount;
        uint32_t nSector = pFile->pArchive->nSectorSize;
        uint32_t nIndex = nFileOffset / nSector;
        if (nIndex < nSectors) {
            int nCountdown = -(int)(((nResult - 1) + nSector) / nSector);
            int nSrcPos = 0;
            int nDstPos = 0;
            do {
                if (nCountdown == 0) {
                    break;
                }
                uint32_t nCompressedLen = pFile->pSectorTable[nIndex + 1] - pFile->pSectorTable[nIndex];
                bool bLast = nIndex == nSectors - 1;
                nIndex++;
                D2ArchiveStrc* pMpq;
                uint32_t nExpected;
                if (bLast) {
                    pMpq = pFile->pArchive;
                    nExpected = pMpq->nSectorSize;
                    uint32_t nFileSize = pFile->pBlockEntry->dwFileSize;
                    if (nFileSize == 0 || (nFileSize & (nExpected - 1)) != 0) {
                        nExpected = (nExpected - 1) & nFileSize;
                    }
                } else {
                    pMpq = pFile->pArchive;
                    nExpected = pMpq->nSectorSize;
                }
                if (nCompressedLen < nExpected) {
                    uint32_t nMethod = pFile->pBlockEntry->dwFlags & 0xff00;
                    if (nMethod == 0x200) {
                        if (pFile->bChecksumEnabled != 0 && pFile->nChecksumState != 1 &&
                            0x3f < pReadBuffer[nSrcPos]) {
                            pFile->nChecksumState = 1;
                        }
                        uint32_t nDestSize = nExpected;
                        if (SCompDecompress(pDstBuf + nDstPos, &nDestSize, pReadBuffer + nSrcPos, nCompressedLen,
                                            pMpq->szArchivePath) == 0) {
                            SErrDisplayError(0x85100083, pFile->pArchive->szArchivePath, -4, NULL, 0, 1);
                        }
                    } else if (nMethod == 0x100) {
                        pthread_mutex_lock(&s_tQueueLock.m_tMutex);
                        SFileBlastContext tContext;
                        tContext.pInput = pReadBuffer + nSrcPos;
                        tContext.nOutOffset = 0;
                        tContext.nOutLimit = 0xffffffff;
                        tContext.n10 = 0;
                        tContext.pOutput = pDstBuf + nDstPos;
                        tContext.nInSize = nCompressedLen;
                        blast((PFNBLASTREAD)SFILE_DecompressReadCallback, &tContext,
                              (PFNBLASTWRITE)SFILE_DecompressWriteCallback, &tContext);
                        pthread_mutex_unlock(&s_tQueueLock.m_tMutex);
                    }
                } else if (pReadBuffer != pDstBuf && 0 < (int)nExpected) {
                    memmove(pDstBuf + nDstPos, pReadBuffer + nSrcPos, nExpected);
                }
                nSrcPos += nCompressedLen;
                nDstPos += nExpected;
                nCountdown++;
                nSectors = pFile->nSectorCount;
            } while (nIndex < nSectors);
        }
    }
    if (pReadBuffer != pDstBuf) {
        SMemFree(pReadBuffer, SFILE_FILE, 0x3cd, 0);
    }
    return nResult;
}

// What a request's completion pointer names: the function to call when a read is done.
struct SFileCompletion {
    void (*pfnComplete)(void* pBuffer, int nBytes, SFileCompletion* pCompletion);
};

// 1.14d mac: 00015e0c
// The I/O thread. Each pass signals the events left by cancelled requests, tops up the
// DirectSound streams (starting, refilling and finishing them), then picks the next request
// by urgency (a stream about to run dry, an urgent read, one that is due, else the nearest
// on the disc) and performs it, pacing plain reads to the byte budget.
static DWORD SFILE_CdThreadProc(LPVOID pParam) {
    uint32_t nLastTick = GetTickCount();
    if (s_bCdThreadExit != 0) {
        s_bCdThreadExit = 0;
        return 0;
    }
    D2ArchiveStrc* pLastArchive = NULL;
    uint32_t dwLastReadPos = 0;
    while (true) {
        uint32_t nNow = GetTickCount();
        uint32_t nDelay = 0;
        if (s_nAsyncBudget != 0) {
            nDelay = (s_nDataChunkSize * 1000) / s_nAsyncBudget;
        }
        SFILE_LOCK();
        SFileEventNode* pSignal = s_tCompletionList.Head();
        while (pSignal != NULL) {
            SetEvent(pSignal->hEvent);
            pSignal = s_tCompletionList.DeleteNode(pSignal);
        }
        if (s_bCdThreadDraining == 0) {
            D2StormAudioStreamRec* pStream = s_tStreamList.Head();
            while (pStream != NULL) {
                int nType = pStream->nStreamType;
                if (nType == 3) {
                    goto stream_end;
                }
                if (nType == 2) {
                    if (pStream->nDataChunkSize + pStream->nDataChunkOffset <= pStream->pMpqFile->nFilePos) {
                        goto stream_end;
                    }
                    goto stream_refill;
                }
                goto stream_start;
            stream_end:
                if (pStream->nNoCleanupFlag == 0) {
                    int nPos = SFILE_GetStreamPosition(pStream);
                    if (nPos != -1) {
                        if (pStream->nCurrentPos < pStream->nDataChunkSize) {
                            if (s_nWaveChunkSize <= (uint32_t)(nPos - pStream->nWritePos) % pStream->nBufferSize) {
                                DWORD nLockSize = pStream->nBufferSize - pStream->nWritePos;
                                if (s_nWaveChunkSize < nLockSize) {
                                    nLockSize = s_nWaveChunkSize;
                                }
                                void* pLockPtr = NULL;
                                int nLock = pStream->pStream->Lock(pStream->nWritePos, nLockSize, &pLockPtr, &nLockSize,
                                                                   NULL, NULL, 0);
                                if (nLock >= 0) {
                                    memset(pLockPtr, pStream->bIs16Bit, nLockSize);
                                    pStream->pStream->Unlock();
                                    pStream->nBytePos = pStream->nBytePos + nLockSize;
                                    pStream->nBytesPastEnd = pStream->nBytesPastEnd + nLockSize;
                                    uint32_t nWrite = pStream->nWritePos + nLockSize;
                                    pStream->nWritePos = nWrite;
                                    if (pStream->nBufferSize <= nWrite) {
                                        pStream->nWritePos = nWrite - pStream->nBufferSize;
                                    }
                                }
                            }
                            pStream->nStreamType = 3;
                        } else {
                            pStream->pStream->Stop();
                            pStream->nStreamType = 4;
                        }
                    }
                } else {
                    pStream->pMpqFile->nFilePos = pStream->nDataChunkOffset;
                    pStream->nBytePos = 0;
                    pStream->nBytesPastEnd = 0;
                    nType = pStream->nStreamType;
                    goto stream_dispatch;
                }
                goto stream_next;
            stream_start:
                nType = pStream->nStreamType;
            stream_dispatch:
                if (nType == 0) {
                    uint32_t nInitialPos = pStream->nInitialPos;
                    uint32_t nFileOffset = pStream->nDataChunkOffset + nInitialPos;
                    D2MPQFileStrc* pFile = pStream->pMpqFile;
                    pFile->nFilePos = nFileOffset;
                    uint32_t nTotal = pStream->nDataChunkSize - nInitialPos;
                    if (pStream->nBufferSize < nTotal) {
                        nTotal = pStream->nBufferSize;
                    }
                    uint32_t nDone = 0;
                    if (gnSSystemAsyncReadSize != 0) {
                        nDone = s_nWaveChunkSize;
                        if (nTotal <= s_nWaveChunkSize) {
                            nDone = nTotal;
                        }
                        uint32_t nMisalign = (pFile->pArchive->nSectorSize - 1) & (nDone + nFileOffset);
                        if (nMisalign == 0 || nDone <= nMisalign) {
                            nMisalign = 0;
                        }
                        nDone -= nMisalign;
                        IssueRequest(pFile, nFileOffset, NULL, NULL, pStream->pStream, 0, pStream, nDone, 0, -2, 0, 1,
                                     0, NULL);
                        pFile = pStream->pMpqFile;
                        nFileOffset = pFile->nFilePos + nDone;
                        pFile->nFilePos = nFileOffset;
                    }
                    if (nDone < nTotal) {
                        int nDelayMs = 0;
                        do {
                            uint32_t nThis = nTotal - nDone;
                            if (s_nWaveChunkSize < nThis) {
                                nThis = s_nWaveChunkSize;
                            }
                            IssueRequest(pFile, nFileOffset, NULL, NULL, pStream->pStream, nDone, pStream, nThis,
                                         nDelayMs, -2, 0, 1, 0, NULL);
                            pFile = pStream->pMpqFile;
                            nFileOffset = nThis + pFile->nFilePos;
                            pFile->nFilePos = nFileOffset;
                            nDelayMs += (s_nWaveChunkSize * 1000) / pStream->dwAvgBytesPerSec;
                            nDone += s_nWaveChunkSize;
                        } while (nDone < nTotal);
                    }
                    pStream->nStreamType = 1;
                    pStream->nWritePos = nDone % pStream->nBufferSize;
                } else if (nType == 2) {
                    goto stream_refill;
                }
                goto stream_next;
            stream_refill: {
                int nPos = SFILE_GetStreamPosition(pStream);
                if (nPos != -1) {
                    uint32_t nWrite = pStream->nWritePos;
                    uint32_t nBufSize = pStream->nBufferSize;
                    if (s_nWaveChunkSize <= (uint32_t)(nPos - nWrite) % nBufSize) {
                        uint32_t nLock = nBufSize - nWrite;
                        if (s_nWaveChunkSize < nLock) {
                            nLock = s_nWaveChunkSize;
                        }
                        D2MPQFileStrc* pFile = pStream->pMpqFile;
                        uint32_t nFilePos = pFile->nFilePos;
                        uint32_t nLength = (pStream->nDataChunkSize + pStream->nDataChunkOffset) - nFilePos;
                        if (nLock < nLength) {
                            nLength = nLock;
                        }
                        if (gnSSystemAsyncReadSize != 0) {
                            uint32_t nTail = (pFile->pArchive->nSectorSize - 1) & (nLength + nFilePos);
                            if (nTail != 0 && nTail < nLength) {
                                nLength -= nTail;
                            }
                        }
                        IssueRequest(pFile, nFilePos, NULL, NULL, pStream->pStream, nWrite, pStream, nLength,
                                     ((nBufSize + s_nWaveChunkSize * -2) * 1000) / pStream->dwAvgBytesPerSec, -2, 0, 1,
                                     0, NULL);
                        pStream->pMpqFile->nFilePos += nLength;
                        uint32_t nNewWrite = pStream->nWritePos + s_nWaveChunkSize;
                        pStream->nWritePos = nNewWrite;
                        if (pStream->nBufferSize <= nNewWrite) {
                            pStream->nWritePos = nNewWrite - pStream->nBufferSize;
                        }
                    }
                }
            }
            stream_next:
                pStream = s_tStreamList.Next(pStream);
            }
        }

        // Pick the next request.
        int nNow2 = GetTickCount();
        D2StormAsyncOpStrc* pRequest = s_tRequestQueue.Head();
        D2StormAsyncOpStrc* pBest = NULL;
        if (pRequest != NULL) {
            uint32_t nBestSeek = 0xffffffff;
            int nBest = 0x7fffffff;
            bool bDue = false;
            bool bPriority = false;
            bool bStream = false;
            do {
                TSLink<D2StormAsyncOpStrc>* pPrevLink = pRequest->m_link.m_prevlink;
                D2StormAsyncOpStrc* pPrev =
                    pPrevLink == &s_tRequestQueue.m_terminator ? NULL : (D2StormAsyncOpStrc*)pPrevLink;
                if (pRequest->pReadBufferBase == NULL || pPrev == NULL ||
                    pRequest->pReadBufferBase != pPrev->pReadBufferBase) {
                    int nDue = pRequest->dwDueTick - nNow2;
                    if (nDue < 1000 && pRequest->pParentStream != NULL && (!bStream || nDue < nBest)) {
                        bStream = true;
                        nBest = nDue;
                        pBest = pRequest;
                    }
                    if (!bStream) {
                        int nRequestPriority = pRequest->nPriority;
                        if (0 < nRequestPriority && (!bPriority || nDue < nBest)) {
                            bPriority = true;
                            nBest = nDue;
                            pBest = pRequest;
                        }
                        if (!bPriority) {
                            if (nDue < 1000 && nDue < nBest) {
                                bDue = true;
                                pBest = pRequest;
                                nBest = nDue;
                            }
                            if (!bDue && (pBest == NULL || pBest->nPriority <= nRequestPriority)) {
                                uint32_t nSeek = 0;
                                if (s_bSeekOptimization != 0) {
                                    D2ArchiveStrc* pArchive = pRequest->pFile->pArchive;
                                    uint32_t nBase = 0;
                                    if (pArchive != NULL) {
                                        nBase = dwLastReadPos - pArchive->nSectorSize;
                                        if (dwLastReadPos < pArchive->nSectorSize || nBase == 0) {
                                            nBase = 0;
                                        }
                                    }
                                    nSeek = 0xfffffffe;
                                    if (pArchive == pLastArchive) {
                                        uint32_t nSeekPos = pRequest->nSeekFilePos;
                                        nSeek = nSeekPos - nBase;
                                        if (nSeekPos < nBase) {
                                            nSeek = nSeekPos + 0x8000000;
                                        }
                                    }
                                }
                                if (pBest != NULL && nRequestPriority <= pBest->nPriority && nBestSeek <= nSeek) {
                                    if (nSeek != nBestSeek) {
                                        goto next_request;
                                    }
                                    if (nBest <= nDue) {
                                        goto next_request;
                                    }
                                }
                                nBest = nDue;
                                pBest = pRequest;
                                nBestSeek = nSeek;
                            }
                        }
                    }
                }
            next_request:
                pRequest = s_tRequestQueue.Next(pRequest);
            } while (pRequest != NULL);
            if (pBest != NULL) {
                if (!bPriority && !bStream && !bDue && nNow - nLastTick < nDelay) {
                    nDelay = (nLastTick - nNow) + nDelay;
                    SFILE_UNLOCK();
                    goto wait;
                }
                dwLastReadPos = pBest->nSeekFilePos;
                pLastArchive = pBest->pFile->pArchive;
                s_pCurrentRequest = pBest;
                s_tRequestQueue.UnlinkNode(pBest);
                SFILE_UNLOCK();

                // Perform it.
                D2StormAsyncOpStrc* pCurrent = s_pCurrentRequest;
                void** ppDest = &pCurrent->pChunkWritePtr;
                if (pCurrent->pStream != NULL) {
                    ppDest = &s_pWaveBuffer;
                }
                void* pDest = *ppDest;
                if (pDest == NULL) {
                    pCurrent->nChunkBytesRead = 0;
                } else {
                    D2MPQFileStrc* pFile = pCurrent->pFile;
                    if (pFile->hFile == 0) {
                        uint32_t nRead = SFILE_InternalReadFile((uint32_t)(uintptr_t)pDest, pCurrent->nFilePos, pFile,
                                                                pDest, pCurrent->nBytesToRead);
                        pCurrent = s_pCurrentRequest;
                        pCurrent->nChunkBytesRead = nRead;
                        pFile = pCurrent->pFile;
                        if (pFile->bChecksumEnabled != 0 && pFile->nChecksumState == 2 &&
                            pFile->nExpectedFileSize == pFile->pBlockEntry->dwFileSize) {
                            STORM_Crc32(NULL, 0, &pFile->dwComputedCRC, 4);
                            pCurrent = s_pCurrentRequest;
                            pCurrent->pFile->nChecksumState = 4;
                        }
                    } else {
                        SFILE_SyncReadFile(pFile, pCurrent->nFilePos, pDest, pCurrent->nBytesToRead,
                                           &pCurrent->nChunkBytesRead);
                        pCurrent = s_pCurrentRequest;
                    }
                }
                if (pCurrent->pStream != NULL) {
                    if (pCurrent->nChunkBytesRead == 0) {
                        if (pCurrent->nBytesToRead != 0) {
                            pCurrent->pStream->Stop();
                            if (s_pCurrentRequest->pParentStream != NULL) {
                                s_pCurrentRequest->pParentStream->nStreamType = 4;
                            }
                        }
                    } else {
                        SFILE_LOCK();
                        DWORD nLockedSize;
                        int nLock = pCurrent->pStream->Lock(pCurrent->nStreamPos, s_nWaveChunkSize,
                                                            &pCurrent->pChunkWritePtr, &nLockedSize, NULL, NULL, 0);
                        if (nLock >= 0 && nLockedSize == s_nWaveChunkSize) {
                            uint32_t nCopy = pCurrent->nChunkBytesRead;
                            if (nLockedSize <= pCurrent->nChunkBytesRead) {
                                nCopy = nLockedSize;
                            }
                            if (0 < (int)nCopy) {
                                memmove(pCurrent->pChunkWritePtr, s_pWaveBuffer, nCopy);
                            }
                            if (nCopy < nLockedSize) {
                                memset((uint8_t*)pCurrent->pChunkWritePtr + nCopy, pCurrent->pParentStream->bIs16Bit,
                                       nLockedSize - nCopy);
                            }
                            pCurrent->pStream->Unlock();
                            D2StormAudioStreamRec* pParent = pCurrent->pParentStream;
                            uint32_t nBytePos = (pCurrent->nFilePos - pParent->nDataChunkOffset) + nLockedSize;
                            pParent->nBytePos = nBytePos;
                            uint32_t nDataSize = pParent->nDataChunkSize;
                            if (nDataSize <= nBytePos && nBytePos - nDataSize != 0) {
                                pParent->nBytesPastEnd = nBytePos - nDataSize;
                            }
                            if (pParent->nStreamType == 1) {
                                if (0x10000 < nDataSize) {
                                    nDataSize = 0x10000;
                                }
                                if (pParent->nBufferSize <= nDataSize) {
                                    nDataSize = pParent->nBufferSize;
                                }
                                if (nDataSize <= (pCurrent->nFilePos - pParent->nDataChunkOffset) + pCurrent->nBytesToRead) {
                                    pParent->pStream->Play(0, 0, 1);
                                    pCurrent->pParentStream->nStreamType = 2;
                                }
                            }
                        }
                        SFILE_UNLOCK();
                    }
                }
                SFileCompletion* pCompletion = (SFileCompletion*)s_pCurrentRequest->pCompletion;
                if (pCompletion != NULL && pCompletion->pfnComplete != NULL) {
                    D2MPQFileStrc* pFile = s_pCurrentRequest->pFile;
                    if (pFile->bChecksumEnabled != 0 && pFile->nChecksumState == 4 &&
                        pFile->dwComputedCRC != pFile->pBlockEntry->dwCRC32) {
                        SErrDisplayError(0x85100083, pFile->szFileName, -4, NULL, 0, 1);
                        pCompletion = (SFileCompletion*)s_pCurrentRequest->pCompletion;
                    }
                    pCompletion->pfnComplete(s_pCurrentRequest->pReadBufferBase,
                                             (int)s_pCurrentRequest->pChunkWritePtr +
                                                 (s_pCurrentRequest->nChunkBytesRead -
                                                  (int)s_pCurrentRequest->pReadBufferBase),
                                             pCompletion);
                }
                HANDLE hEvent = s_pCurrentRequest->hCompletionEvent;
                SFILE_LOCK();
                D2StormAsyncOpStrc* pDone = s_pCurrentRequest;
                if (pDone->bAutoFree != 0 && pDone != NULL) {
                    delete pDone;
                }
                s_pCurrentRequest = NULL;
                SFILE_UNLOCK();
                if (hEvent != 0) {
                    SetEvent(hEvent);
                }
                nLastTick = nNow;
                if (s_bCdThreadExit == 1) {
                    s_bCdThreadExit = 0;
                    return 0;
                }
                continue;
            }
        }
        // Nothing to do: sleep a quarter second or until woken.
        SFILE_UNLOCK();
        nDelay = 0xfa;
    wait:
        WaitForSingleObject(s_hCdEvent, nDelay);
        if (s_bCdThreadExit != 0) {
            s_bCdThreadExit = 0;
            return 0;
        }
    }
}

// The out-of-line instantiations of stpl.h's list templates this file's object holds, one
// set per record type (archive, request, file, completion event, stream), by the qualified
// name the compare tool pairs them with. The calls inside the file inline them; the originals
// are weak copies the linker kept from here.

template void TSLink<D2ArchiveStrc>::Unlink();
template TSLink<D2ArchiveStrc>* TSLink<D2ArchiveStrc>::NextLink(int);
template void TSExplicitList<D2ArchiveStrc,0>::UnlinkAll();
template void TSExplicitList<D2ArchiveStrc,0>::LinkNode(D2ArchiveStrc*, uint32_t, D2ArchiveStrc*);
template D2ArchiveStrc* TSExplicitList<D2ArchiveStrc,0>::NewNode(uint32_t, uint32_t, uint32_t);
template D2ArchiveStrc* TSExplicitList<D2ArchiveStrc,0>::DeleteNode(D2ArchiveStrc*);
template TSExplicitList<D2ArchiveStrc,0>::~TSExplicitList();

template void TSLink<D2StormAsyncOpStrc>::Unlink();
template TSLink<D2StormAsyncOpStrc>* TSLink<D2StormAsyncOpStrc>::NextLink(int);
template void TSExplicitList<D2StormAsyncOpStrc,0>::UnlinkAll();
template void TSExplicitList<D2StormAsyncOpStrc,0>::LinkNode(D2StormAsyncOpStrc*, uint32_t, D2StormAsyncOpStrc*);
template D2StormAsyncOpStrc* TSExplicitList<D2StormAsyncOpStrc,0>::NewNode(uint32_t, uint32_t, uint32_t);
template D2StormAsyncOpStrc* TSExplicitList<D2StormAsyncOpStrc,0>::DeleteNode(D2StormAsyncOpStrc*);
template TSExplicitList<D2StormAsyncOpStrc,0>::~TSExplicitList();

template void TSLink<D2MPQFileStrc>::Unlink();
template TSLink<D2MPQFileStrc>* TSLink<D2MPQFileStrc>::NextLink(int);
template void TSExplicitList<D2MPQFileStrc,0>::UnlinkAll();
template void TSExplicitList<D2MPQFileStrc,0>::LinkNode(D2MPQFileStrc*, uint32_t, D2MPQFileStrc*);
template D2MPQFileStrc* TSExplicitList<D2MPQFileStrc,0>::NewNode(uint32_t, uint32_t, uint32_t);
template D2MPQFileStrc* TSExplicitList<D2MPQFileStrc,0>::DeleteNode(D2MPQFileStrc*);
template TSExplicitList<D2MPQFileStrc,0>::~TSExplicitList();

template void TSLink<SFileEventNode>::Unlink();
template TSLink<SFileEventNode>* TSLink<SFileEventNode>::NextLink(int);
template void TSExplicitList<SFileEventNode,0>::UnlinkAll();
template void TSExplicitList<SFileEventNode,0>::LinkNode(SFileEventNode*, uint32_t, SFileEventNode*);
template SFileEventNode* TSExplicitList<SFileEventNode,0>::NewNode(uint32_t, uint32_t, uint32_t);
template SFileEventNode* TSExplicitList<SFileEventNode,0>::DeleteNode(SFileEventNode*);
template TSExplicitList<SFileEventNode,0>::~TSExplicitList();

template void TSLink<D2StormAudioStreamRec>::Unlink();
template TSLink<D2StormAudioStreamRec>* TSLink<D2StormAudioStreamRec>::NextLink(int);
template void TSExplicitList<D2StormAudioStreamRec,0>::UnlinkAll();
template void TSExplicitList<D2StormAudioStreamRec,0>::LinkNode(D2StormAudioStreamRec*, uint32_t, D2StormAudioStreamRec*);
template D2StormAudioStreamRec* TSExplicitList<D2StormAudioStreamRec,0>::NewNode(uint32_t, uint32_t, uint32_t);
template D2StormAudioStreamRec* TSExplicitList<D2StormAudioStreamRec,0>::DeleteNode(D2StormAudioStreamRec*);
template TSExplicitList<D2StormAudioStreamRec,0>::~TSExplicitList();

// 1.14d mac: 00016a2e TSExplicitList<D2ArchiveStrc,0>::~TSExplicitList[D2]
// 1.14d mac: 000170de TSExplicitList<D2ArchiveStrc,0>::UnlinkAll
// 1.14d mac: 00016d72 TSLink<D2ArchiveStrc>::Unlink
// 1.14d mac: 00016db2 TSLink<D2ArchiveStrc>::NextLink
// 1.14d mac: 00016b6a TSExplicitList<D2ArchiveStrc,0>::LinkNode
// 1.14d mac: 00016a50 TSExplicitList<D2StormAsyncOpStrc,0>::~TSExplicitList[D2]
// 1.14d mac: 00017020 TSExplicitList<D2StormAsyncOpStrc,0>::UnlinkAll
// 1.14d mac: 00017068 TSLink<D2StormAsyncOpStrc>::Unlink
// 1.14d mac: 000170a8 TSLink<D2StormAsyncOpStrc>::NextLink
// 1.14d mac: 0001719c TSExplicitList<D2StormAsyncOpStrc,0>::LinkNode
// 1.14d mac: 00017126 TSExplicitList<D2StormAsyncOpStrc,0>::NewNode
// 1.14d mac: 00017210 TSExplicitList<D2StormAsyncOpStrc,0>::DeleteNode
// 1.14d mac: 00016a8e TSExplicitList<D2MPQFileStrc,0>::~TSExplicitList[D2]
// 1.14d mac: 00016fd8 TSExplicitList<D2MPQFileStrc,0>::UnlinkAll
// 1.14d mac: 00016cfc TSLink<D2MPQFileStrc>::Unlink
// 1.14d mac: 00016d3c TSLink<D2MPQFileStrc>::NextLink
// 1.14d mac: 00016c88 TSExplicitList<D2MPQFileStrc,0>::LinkNode
// 1.14d mac: 00016bde TSExplicitList<D2MPQFileStrc,0>::NewNode
// 1.14d mac: 00016ab0 TSExplicitList<SFileEventNode,0>::~TSExplicitList[D2]
// 1.14d mac: 00016f1a TSExplicitList<SFileEventNode,0>::UnlinkAll
// 1.14d mac: 00016f62 TSLink<SFileEventNode>::Unlink
// 1.14d mac: 00016fa2 TSLink<SFileEventNode>::NextLink
// 1.14d mac: 0001762c TSExplicitList<SFileEventNode,0>::LinkNode
// 1.14d mac: 000175b6 TSExplicitList<SFileEventNode,0>::NewNode
// 1.14d mac: 00017546 TSExplicitList<SFileEventNode,0>::DeleteNode
// 1.14d mac: 00016ad2 TSExplicitList<D2StormAudioStreamRec,0>::~TSExplicitList[D2]
// 1.14d mac: 00016ed2 TSExplicitList<D2StormAudioStreamRec,0>::UnlinkAll
// 1.14d mac: 00016de8 TSLink<D2StormAudioStreamRec>::Unlink
// 1.14d mac: 00016e28 TSLink<D2StormAudioStreamRec>::NextLink
// 1.14d mac: 00016e5e TSExplicitList<D2StormAudioStreamRec,0>::LinkNode
// 1.14d mac: 00016af4 TSExplicitList<D2StormAudioStreamRec,0>::NewNode
// 1.14d mac: 00017700 _GLOBAL__I_a

#endif  // D2_OS_MAC
