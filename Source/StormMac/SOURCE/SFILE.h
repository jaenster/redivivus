// Storm Mac's SFILE.CPP: the MPQ archive and file layer, 1.14d Intel Mac build (DiabloII_macho).
// The records are the Windows Storm's (Source/Storm/Source/SFile.h) with the Win32-on-Mac
// layer's types: a HANDLE is the layer's 32-bit index, a list's end is marked with the
// terminator's address | 1 (StormMac/H/stpl.h).
//
// Mac-only: see Win32Mac/Source/Win32/Win32Mac.h for why the guard is spelled
// `#if !defined(_WIN32) && D2_OS_MAC`.
#pragma once

#include "D2Arch.h"

#if !defined(_WIN32) && D2_OS_MAC

#pragma GCC visibility push(hidden)
#include "Source/StormMac/H/stpl.h"
#pragma GCC visibility pop
#include "Source/StormMac/Win32Mac/Source/Win32/Win32Mac.h"

struct D2MPQFileStrc;
struct D2ArchiveStrc;
struct D2StormAudioStreamRec;
struct D2StormAsyncOpStrc;
struct D2SsigContextStrc;
// The Mac DirectSound shim's buffer (Storm::IDirectSoundBuffer); only the calls this file makes.
// The layer implements each method as a free function taking the object first
// (Win32_DirectSound.cpp, Win32MacDirectSound.h); the members here are those functions: this
// file's calls pass fewer arguments than the layer declares, so they bind to the layer's symbols.
struct IDirectSoundBuffer {
    int SetVolume(int nVolume) __asm__("__Z28IDirectSoundBuffer_SetVolumeP18IDirectSoundBufferi");
    int SetPan(int nPan) __asm__("__Z25IDirectSoundBuffer_SetPanP18IDirectSoundBufferi");
    int SetCurrentPosition(DWORD dwPosition)
        __asm__("__Z37IDirectSoundBuffer_SetCurrentPositionP18IDirectSoundBufferj");
    int GetCurrentPosition(DWORD* pdwPlayCursor, DWORD* pdwWriteCursor)
        __asm__("__Z37IDirectSoundBuffer_GetCurrentPositionP18IDirectSoundBufferPjS1_");
    int Release() __asm__("__Z26IDirectSoundBuffer_ReleaseP18IDirectSoundBuffer");
    int Lock(DWORD dwOffset, DWORD dwBytes, void** ppvAudioPtr1, DWORD* pdwAudioBytes1, void** ppvAudioPtr2,
             DWORD* pdwAudioBytes2, DWORD dwFlags) __asm__("__Z23IDirectSoundBuffer_LockP18IDirectSoundBufferiiPiS1_S1_S1_j");
    int Unlock() __asm__("__Z25IDirectSoundBuffer_UnlockP18IDirectSoundBufferPvjS1_j");
    int Stop() __asm__("__Z23IDirectSoundBuffer_StopP18IDirectSoundBuffer");
    int Play(DWORD dwReserved1, DWORD dwPriority, DWORD dwFlags)
        __asm__("__Z23IDirectSoundBuffer_PlayP18IDirectSoundBufferjjj");
};

struct D2MPQHashEntryStrc {
    uint32_t dwHashA;       // 0x00
    uint32_t dwHashB;       // 0x04
    uint16_t wLocale;       // 0x08
    uint16_t wPlatform;     // 0x0A
    uint32_t dwBlockIndex;  // 0x0C
};

struct D2MPQHeaderStrc {
    uint32_t dwMagic;            // 0x00  'MPQ\x1a'
    uint32_t dwHeaderSize;       // 0x04
    uint32_t dwArchiveSize;      // 0x08
    uint16_t wFormatVersion;     // 0x0C
    uint16_t wSectorSizeShift;   // 0x0E
    uint32_t dwHashTablePos;     // 0x10
    uint32_t dwBlockTablePos;    // 0x14
    uint32_t dwHashTableSize;    // 0x18
    uint32_t dwBlockTableSize;   // 0x1C
};

// The block table entry as held in memory: the 0x10-byte disk entry plus the attributes
// (file time, CRC).
struct D2MPQBlockEntryStrc {
    uint32_t dwFilePos;           // 0x00
    uint32_t dwCompressedSize;    // 0x04
    uint32_t dwFileSize;          // 0x08
    uint32_t dwFlags;             // 0x0C  0xff00 compressed, 0x10000 encrypted, 0x80000000 exists
    FILETIME ftFileTime;          // 0x10
    uint32_t dwCRC32;             // 0x18
};

struct D2ArchiveStrc {
    TSLink<D2ArchiveStrc> m_link;            // 0x000  pPrev, pNext
    char szArchivePath[260];                 // 0x008
    HANDLE hFile;                            // 0x10C
    int nArchiveType;                        // 0x110  0 disk, 1 and 3 read by the I/O thread (3: on a CD)
    int nPriority;                           // 0x114  the list is in descending priority
    D2MPQFileStrc* pLastReadFile;            // 0x118  file whose aligned run is cached in pSectorBuffer
    uint32_t nLastReadOffset;                // 0x11C
    uint32_t nSectorSize;                    // 0x120
    void* pSectorBuffer;                     // 0x124
    uint32_t nLastReadResult;                // 0x128
    uint32_t nArchiveOffset;                 // 0x12C
    uint32_t nArchiveFileSize;               // 0x130
    D2MPQHeaderStrc* pMpqHeader;             // 0x134
    D2MPQBlockEntryStrc* pBlockTable;        // 0x138
    D2MPQHashEntryStrc* pHashTable;          // 0x13C
    uint32_t nFilePos;                       // 0x140  where the last checked read ended
    int nRefCount;                           // 0x144
    D2MPQFileStrc* pArchiveFile;             // 0x148  the archive opened as a file (closed with it)

    D2ArchiveStrc() {}
    ~D2ArchiveStrc();
};

struct D2MPQFileStrc {
    TSLink<D2MPQFileStrc> m_link;        // 0x00  pPrev, pNext
    char szFileName[260];                    // 0x008
    char* szFilePath;                        // 0x10C
    HANDLE hFile;                            // 0x110
    D2ArchiveStrc* pArchive;                 // 0x114
    D2MPQBlockEntryStrc* pBlockEntry;        // 0x118
    uint32_t nDecryptKey;                    // 0x11C
    uint32_t nFilePos;                       // 0x120
    uint32_t nLastFilePos;                   // 0x124
    uint32_t nSectorCount;                   // 0x128
    uint32_t* pSectorTable;                  // 0x12C
    BOOL bSectorsLoaded;                     // 0x130
    BOOL bStreamActive;                      // 0x134  a DirectSound stream reads this file
    void* pReadBuffer;                       // 0x138
    uint32_t nBytesRead;                     // 0x13C
    uint32_t nBufferSize;                    // 0x140
    uint32_t nRefCount;                      // 0x144
    uint32_t dwComputedCRC;                  // 0x148
    BOOL bChecksumEnabled;                   // 0x14C
    uint32_t nChecksumState;                 // 0x150
    uint32_t nExpectedFileSize;              // 0x154

    D2MPQFileStrc() : szFilePath(NULL), hFile(NULL), pArchive(NULL), pSectorTable(NULL), pReadBuffer(NULL) {}
    ~D2MPQFileStrc();
};

struct D2StormAudioStreamRec {
    TSLink<D2StormAudioStreamRec> m_link;        // 0x00  pPrev, pNext
    D2MPQFileStrc* pMpqFile;                 // 0x08
    uint32_t nWritePos;                      // 0x0C
    uint32_t dwAvgBytesPerSec;               // 0x10
    uint32_t nNoCleanupFlag;                 // 0x14
    int nStreamType;                         // 0x18
    uint32_t nBytesPastEnd;                  // 0x1C
    uint32_t nDataChunkOffset;               // 0x20
    uint32_t nDataChunkSize;                 // 0x24
    uint32_t nInitialPos;                    // 0x28
    uint32_t nCurrentPos;                    // 0x2C
    uint32_t nBytePos;                       // 0x30
    int lVolume;                             // 0x34
    int lPan;                                // 0x38
    IDirectSoundBuffer* pStream;             // 0x3C
    uint32_t nBufferSize;                    // 0x40
    uint32_t bAllocatedStream;               // 0x44
    uint8_t bIs16Bit;                        // 0x48
    uint8_t pad49[3];                        // 0x49
    uint32_t nRefCount;                      // 0x4C

    D2StormAudioStreamRec() {}
    ~D2StormAudioStreamRec();
};

struct D2StormAsyncOpStrc {
    TSLink<D2StormAsyncOpStrc> m_link;        // 0x00  pPrev, pNext
    HANDLE hCompletionEvent;                 // 0x08
    D2MPQFileStrc* pFile;                    // 0x0C
    int nFilePos;                            // 0x10
    uint32_t nSeekFilePos;                   // 0x14
    int dwDueTick;                           // 0x18
    int nPriority;                           // 0x1C
    void* pChunkWritePtr;                    // 0x20
    void* pReadBufferBase;                   // 0x24
    IDirectSoundBuffer* pStream;             // 0x28  the DirectSound buffer the request fills
    uint32_t nStreamPos;                     // 0x2C
    D2StormAudioStreamRec* pParentStream;    // 0x30
    int nBytesToRead;                        // 0x34
    uint32_t bAutoFree;                      // 0x38
    uint32_t nChunkBytesRead;                // 0x3C
    void* pCompletion;                       // 0x40

    D2StormAsyncOpStrc() {}
    ~D2StormAsyncOpStrc();
};
// What cancelling a request leaves for the I/O thread to signal.
struct SFileEventNode : TSLinkedNode<SFileEventNode> {  // m_link at 0x00
    HANDLE hEvent;                  // 0x08
};

typedef char D2MPQ_size_block[(sizeof(D2MPQBlockEntryStrc) == 0x1c) ? 1 : -1];
typedef char D2MPQ_size_archive[(sizeof(D2ArchiveStrc) == 0x14c) ? 1 : -1];
typedef char D2MPQ_size_file[(sizeof(D2MPQFileStrc) == 0x158) ? 1 : -1];
typedef char D2MPQ_size_stream[(sizeof(D2StormAudioStreamRec) == 0x50) ? 1 : -1];
typedef char D2MPQ_size_async[(sizeof(D2StormAsyncOpStrc) == 0x44) ? 1 : -1];

// The Mac DirectSound shim's device (Storm::IDirectSound).
struct IDirectSound {
    int CreateSoundBuffer(const void* pDescription, IDirectSoundBuffer** ppBuffer)
        __asm__("__Z30IDirectSound_CreateSoundBufferP12IDirectSoundPKjPP18IDirectSoundBufferPv");
};

// ---- the entry points of the other Storm files this one calls ----
int SCompDecompress(void* pDest, uint32_t* pnDestSize, const void* pSource, uint32_t nSourceSize,
                    const char* szArchivePath);
extern "C" {
#include "3rdparty/blast/blast.h"
}
void SErrSetLastError(DWORD dwErrCode);
void SErrReportResourceLeak(const char* szType);
void SErrReportNamedResourceLeak(const char* szType, const char* szName);
char* SStrChrR(const char* szString, char cChar);
int SStrCopy(char* szDest, const char* szSource, uint32_t nMaxChars);
int SStrPack(char* szDest, const char* szSource, uint32_t nMaxChars);
void SSignatureVerifyStream_Begin(D2SsigContextStrc** ppContext, uint32_t nModulusLen, uint32_t nExponentLen);
uint32_t SSignatureVerifyStream_GetSignatureLength(D2SsigContextStrc* pContext);
void SSignatureVerifyStream_ProvideData(D2SsigContextStrc* pContext, const void* pData, uint32_t nSize);
int SSignatureVerifyStream_Finish(D2SsigContextStrc* pContext, const void* pModulus, const void* pExponent);

void SErrDisplayError(DWORD dwErrCode, const char* szFileName, int nLine, char* szDescription, int nFlags,
                      int nRecoverable);
uint32_t SStrLen(const char* szString);
// mode 1 starts a CRC, 2 adds data, 4 finishes it
void STORM_Crc32(const uint8_t* pData, uint32_t nSize, uint32_t* pCrc, int nMode);
char* SStrDup(const char* szString);
uint8_t STORM_IsCDVolume(int16_t nVRefNum);

// ---- the Win32 layer entry points this file calls (Win32Mac.h has the rest) ----
HANDLE CreateFile(LPCSTR lpFileName, DWORD dwDesiredAccess, DWORD dwShareMode, LPVOID lpSecurityAttributes,
                  DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes, HANDLE hTemplateFile);
void MoveMemory(void* pDest, const void* pSource, size_t nSize);
DWORD SetFilePointer(HANDLE hFile, LONG lDistanceToMove, LPLONG lpDistanceToMoveHigh, DWORD dwMoveMethod);
BOOL ReadFile(HANDLE hFile, LPVOID lpBuffer, DWORD nNumberOfBytesToRead, LPDWORD lpNumberOfBytesRead,
              LPVOID lpOverlapped);
DWORD GetFileSize(HANDLE hFile, LPDWORD lpFileSizeHigh);
DWORD GetFileAttributes(LPCSTR lpFileName);
HANDLE GetModuleHandle(LPCSTR szModuleName);
DWORD GetModuleFileName(HANDLE hModule, char* szFileName, DWORD nSize);
char* PATH_MakeFullPath(char* szFullPath, LPCSTR szPath, DWORD nBufferSize);
HANDLE CreateEventA(LPVOID lpEventAttributes, BOOL bManualReset, BOOL bInitialState, LPCSTR lpName);
BOOL SetEvent(HANDLE hEvent);
DWORD GetLastError(void);
HANDLE CreateThread(LPVOID lpThreadAttributes, size_t dwStackSize, LPTHREAD_START_ROUTINE pfnStart,
                    LPVOID lpParameter, DWORD dwCreationFlags, LPDWORD lpThreadId, LPCSTR szName);
BOOL SetThreadPriority(HANDLE hThread, int nPriority);

#endif  // D2_OS_MAC
