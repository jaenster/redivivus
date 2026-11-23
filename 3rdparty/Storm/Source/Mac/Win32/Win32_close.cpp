// Storm's Win32-on-Mac layer: CloseHandle, 1.14d Intel Mac band 00025660..000257bc.
// See Win32Mac.h for what is and is not known about the original file boundaries.
//
// CloseHandle calls GetWin32HandlePtr, the next function in the binary, out of
// line; clang-425 at -Os inlines it into anything in its own TU. So a source file
// ends here and the handle table's begins at 000257bc.
#include "Win32Mac.h"

#if !defined(_WIN32) && D2_OS_MAC

#include <CoreServices/CoreServices.h>

// 1.14d mac: 00025660
// Drops a reference; the last one releases what the type holds and frees the slot.
BOOL CloseHandle(HANDLE hHandle) {
    if (hHandle == 0) {
        SetLastError(0x57);
        return 0;
    }
    Win32HandleObject* pObject = GetWin32HandlePtr(hHandle);
    if (pObject == NULL) {
        SetLastError(6);
        return 0;
    }
    if (InterlockedDecrement(&pObject->nRefCount) != 0) {
        return 1;
    }
    switch (pObject->dwType) {
        case WIN32MAC_HANDLE_EVENT:
            Event_Close(hHandle);
            pObject = GetWin32HandlePtr(hHandle);
            if (pObject->szEventName != NULL) {
                MemFree(pObject->szEventName, 0);
            }
            break;
        case WIN32MAC_HANDLE_FILE: {
            Event_Close(hHandle);
            Win32HandleObject* pFile = GetWin32HandlePtr(hHandle);
            if (pFile->bResourceFork) {
                CloseResFile(pFile->nRefNum);
            } else {
                FSClose(pFile->nRefNum);
            }
            SetFileTimeAtClose(hHandle);
            if (pFile->bWritable) {
                FlushVol(NULL, pFile->tSpec.vRefNum);
            }
            break;
        }
        case WIN32MAC_HANDLE_MODULE:
            Event_Close(hHandle);
            pObject = GetWin32HandlePtr(hHandle);
            if (pObject->szModulePath != NULL) {
                MemFree(pObject->szModulePath, 0);
            }
            break;
        case 5:
            if (pObject->pType5Data != NULL) {
                MemFree(pObject->pType5Data, 0);
            }
            break;
        case WIN32MAC_HANDLE_THREAD:
            Event_Close(hHandle);
            break;
        default:
            fp_fatal("bad case in CloseHandle");
            break;
    }
    FreeWin32Handle(hHandle);
    return 1;
}

#endif  // D2_OS_MAC
