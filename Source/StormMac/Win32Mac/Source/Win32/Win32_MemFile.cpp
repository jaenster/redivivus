// Storm's Win32-on-Mac layer: memory files, 1.14d Intel Mac 00036440..00036830
// (GetMemFile2Ptr .. UnLockMemFile2). A memory file is a 0x9C-byte header and a data block;
// in the tracked mode (s_bMemFile2Tracked) the headers sit on a queue ordered by last use
// and a file whose data was released is read back from its backing file when it is locked.
// The original file name is unknown; the folder is that of the layer.
#include "Win32Mac.h"

#if !defined(_WIN32) && D2_MAC_CARBON

#include <Carbon/Carbon.h>
#include <string.h>

// One memory file, 0x9C bytes. The queue links (QElem) sit at the start.
struct MemFile2 {
    QElem tLink;                  // 0x00  the queue element (only qLink and qType are used)
    BYTE pad0A[0x48 - sizeof(QElem)];
    char szBackingFile[0x40];     // 0x48  empty when there is none
    void* pData;                  // 0x88  NULL when released
    LONG nLockCount;              // 0x8C
    LONG nSize;                   // 0x90
    LONG nSequence;               // 0x94  last-use order
    DWORD dwFlags;                // 0x98  1 in use, 4 zero-filled
};

WIN32MAC_LAYOUT(MemFile2_size, sizeof(MemFile2) == 0x9C);
WIN32MAC_LAYOUT(MemFile2_data, offsetof(MemFile2, pData) == 0x88);

int FPMessageBox(const char* szText, const char* szCaption, UINT uType);
void ExitProcess(UINT uExitCode);
BOOL DeleteFile(const char* szFileName);
HANDLE CreateFile(const char* szFileName, DWORD dwAccess, DWORD dwShare, void* pSecurity, DWORD dwDisposition,
                  DWORD dwFlags, HANDLE hTemplate);
BOOL ReadFile(HANDLE hFile, void* pBuffer, DWORD nBytesToRead, DWORD* pnBytesRead, OVERLAPPED* pOverlapped);

// 1.14d mac: 003f492c
BYTE s_bMemFile2Tracked;
// 1.14d mac: 003f4930
LONG s_nMemFile2Sequence;
// 1.14d mac: 003f4934
LONG s_nMemFile2Total;
// 1.14d mac: 003f4938
LONG s_nMemFile2Resident;
// 1.14d mac: 003f493c
LONG s_nMemFile2Locked;
// 1.14d mac: 003f4940
QHdr s_tMemFile2Queue;

// 1.14d mac: 00036440
// In the tracked mode the data pointer of the header, else the argument itself.
void* GetMemFile2Ptr(void* hMemFile) {
    if (s_bMemFile2Tracked != 0) {
        hMemFile = ((MemFile2*)hMemFile)->pData;
    }
    return hMemFile;
}

// 1.14d mac: 0003645d
void* AllocateMemFile2(int nSize, BYTE bZero) {
    void* pHeader;
    if (s_bMemFile2Tracked == 0) {
        pHeader = MemAlloc(nSize, (DWORD)bZero << 10);
    } else {
        pHeader = NULL;
        if (nSize > 0) {
            DWORD dwFlags = 0x100;
            if (bZero != 0) {
                dwFlags = 0x500;
            }
            pHeader = MemAlloc(0x9C, 0);
            void* pData = MemAlloc(nSize, dwFlags);
            if (pHeader == NULL || pData == NULL) {
                FPMessageBox("Out of memory: AllocateMemFile2", "ERROR", 0x10);
                ExitProcess(0xFFFFFFFF);
            } else {
                MemFile2* pFile = (MemFile2*)pHeader;
                SMemZero(pHeader, sizeof(MemFile2));
                pFile->pData = pData;
                pFile->dwFlags = (DWORD)bZero << 2 | pFile->dwFlags | 1;
                pFile->nSize = nSize;
                s_nMemFile2Total += nSize;
                s_nMemFile2Resident += pFile->nSize;
                s_nMemFile2Sequence++;
                pFile->nSequence = s_nMemFile2Sequence;
                Enqueue((QElemPtr)pHeader, &s_tMemFile2Queue);
            }
        }
    }
    return pHeader;
}

// 1.14d mac: 00036580
// Tracked: nothing while locked; frees the data, deletes the backing file, updates the
// totals and takes the header off the queue.
void FreeMemFile2(void* hMemFile) {
    if (s_bMemFile2Tracked != 0) {
        MemFile2* pFile = (MemFile2*)hMemFile;
        if (pFile == NULL) {
            return;
        }
        if (pFile->nLockCount != 0) {
            return;
        }
        if (pFile->pData != NULL) {
            MemFree(pFile->pData, 0);
            s_nMemFile2Resident -= pFile->nSize;
        }
        if (pFile->szBackingFile[0] != '\0') {
            DeleteFile(pFile->szBackingFile);
        }
        s_nMemFile2Total -= pFile->nSize;
        Dequeue((QElemPtr)hMemFile, &s_tMemFile2Queue);
    }
    MemFree(hMemFile, 0);
}

// 1.14d mac: 0003661b
// Counts a lock and returns the data pointer. The first lock takes the header off the
// queue and, when the data was released, allocates it again and reads it back from the
// backing file.
void* LockMemFile2(void* hMemFile) {
    MemFile2* pFile = (MemFile2*)hMemFile;
    if (s_bMemFile2Tracked != 0 && pFile != NULL) {
        LONG nLocks = pFile->nLockCount;
        pFile->nLockCount = nLocks + 1;
        if (nLocks == 0) {
            s_nMemFile2Locked += pFile->nSize;
            Dequeue((QElemPtr)pFile, &s_tMemFile2Queue);
            if (pFile->pData == NULL) {
                pFile->pData = MemAlloc(pFile->nSize, (pFile->dwFlags & 4) << 8);
                if (pFile->pData == NULL) {
                    FPMessageBox("Out of memory: LockMemFile2", "ERROR", 0x10);
                    ExitProcess(0xFFFFFFFF);
                } else {
                    s_nMemFile2Resident += pFile->nSize;
                    if (pFile->szBackingFile[0] != '\0') {
                        HANDLE hFile = CreateFile(pFile->szBackingFile, 0x80000000, 1, NULL, 3, 0x8000000, 0);
                        if (hFile != 0) {
                            DWORD nBytesRead;
                            ReadFile(hFile, pFile->pData, pFile->nSize, &nBytesRead, NULL);
                            CloseHandle(hFile);
                            if (pFile->nSize != (LONG)nBytesRead) {
                                FPMessageBox("File read error: LockMemFile2", "ERROR", 0x10);
                                ExitProcess(0xFFFFFFFF);
                            }
                        }
                    }
                }
            }
        }
        return pFile->pData;
    }
    return hMemFile;
}

// 1.14d mac: 000367c0
// Takes a lock off; the last one stamps a new use number and puts the header back on the queue.
void UnLockMemFile2(void* hMemFile) {
    MemFile2* pFile = (MemFile2*)hMemFile;
    if (s_bMemFile2Tracked != 0 && pFile != NULL && pFile->nLockCount != 0) {
        LONG nLocks = pFile->nLockCount - 1;
        pFile->nLockCount = nLocks;
        if (nLocks == 0) {
            s_nMemFile2Sequence++;
            pFile->nSequence = s_nMemFile2Sequence;
            Enqueue((QElemPtr)pFile, &s_tMemFile2Queue);
            pFile->dwFlags |= 1;
            s_nMemFile2Locked -= pFile->nSize;
        }
    }
}

#endif  // D2_OS_MAC
