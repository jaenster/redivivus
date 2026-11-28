// StormMac, Win32 Mac layer: the private allocator and the clipboard text, 1.14d Intel
// Mac 00029968..00029b68 (Priv_Alloc .. CLIPBOARD_GetText).
// The original file is unknown: no 1.14d string or initialiser names it, and in the
// named PPC Storm 1.10 Beta (Storm_(Carbon), traceback names) this code is a translation
// unit without a static initialiser, so it carries no file name there either.
// In the PPC Storm 1.10 Beta the Priv_ functions (Priv_Alloc, Priv_AllocClear, Priv_Free,
// Priv_PutTextOnClipboard, Priv_GetClipboardTextBufferSize, Priv_GetTextFromClipboard) are
// linked between the message code and Win32_Process.cpp, as in 1.14d. They are not in
// Win32_Process.cpp in 1.14d: with ExitProcess's body in the same file clang-425 compiles
// Priv_Alloc's failure path differently (tried).
#include "Win32Mac.h"

#if !defined(_WIN32) && D2_MAC_CARBON

#include <Carbon/Carbon.h>
#include <dlfcn.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>

// 1.14d mac: 0002b1d1
int MessageBox(void* hWnd, LPCSTR szText, LPCSTR szCaption, UINT uType);
// 1.14d mac: 0002a41f
void ExitProcess(UINT uExitCode);

// 1.14d mac: 00029968
// Allocates or ends the process: an allocation the layer cannot do without.
void* Priv_Alloc(size_t nSize) {
    void* pMem = MemAlloc(nSize, 0);
    if (pMem == NULL) {
        MessageBox(NULL, "Out of Memory", "", 0x30);
        ExitProcess(0xFFFFFFFF);
    }
    return pMem;
}

// 1.14d mac: 000299cc
void* Priv_AllocClear(size_t nSize) {
    void* pMem = MemAlloc(nSize, 0x100);
    if (pMem == NULL) {
        MessageBox(NULL, "Out of Memory", "", 0x30);
        ExitProcess(0xFFFFFFFF);
    }
    return pMem;
}

// 1.14d mac: 00029a30
void Priv_Free(void* pMem) {
    MemFree(pMem, 0);
}

// 1.14d mac: 00029a4e
// Replaces the scrap with the text as flavor 'TEXT'.
BOOL CLIPBOARD_SetText(const char* szText, int nLength) {
    bool bResult = false;
    if (nLength > 0 && szText != NULL && *szText != '\0') {
        ClearCurrentScrap();
        ScrapRef hScrap;
        GetCurrentScrap(&hScrap);
        bResult = PutScrapFlavor(hScrap, 'TEXT', 0, nLength, szText) == noErr;
    }
    return bResult;
}

// 1.14d mac: 00029aad
// The 'TEXT' scrap's size plus its terminator, 0 when there is none.
int CLIPBOARD_GetTextSize(void) {
    ScrapRef hScrap;
    Size nSize = 0;
    GetCurrentScrap(&hScrap);
    GetScrapFlavorSize(hScrap, 'TEXT', &nSize);
    return nSize > 0 ? nSize + 1 : 0;
}

// 1.14d mac: 00029af1
// Copies the 'TEXT' scrap out with a terminator: its size plus one, 0 when there is
// none, -1 when the buffer is too small.
int CLIPBOARD_GetText(char* pBuffer, int nBufferSize) {
    ScrapRef hScrap;
    Size nSize = 0;
    GetCurrentScrap(&hScrap);
    GetScrapFlavorSize(hScrap, 'TEXT', &nSize);
    int nResult = 0;
    if (nSize != 0) {
        nResult = -1;
        if (nSize < nBufferSize) {
            GetScrapFlavorData(hScrap, 'TEXT', &nSize, pBuffer);
            pBuffer[nSize] = '\0';
            nResult = nSize + 1;
        }
    }
    return nResult;
}

#endif  // D2_OS_MAC
