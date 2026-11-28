// crashy's report file on POSIX, for the modern build. Not Blizzard's crashy.
//
// Blizzard's crash reporter (3rdParty/crashy) has no public source; the Windows build carries
// the parts of it Game.exe holds (3rdparty/crashy), on the Win32 file calls. This is the
// report file alone, with the same behaviour on the POSIX file calls: Open makes
// <temp>/Blizzard Error/<YYYY-MM-DD HH.MM.SS>/ and opens the named file in it, Printf writes
// to it, AddAttachment records a path (copying the file into the report directory first when
// asked), Close and the destructor close it. isDebuggerPresent asks the kernel layer. There
// is no error-reporter process to launch: Fog's POSIX crash path (ErrorManager.cpp,
// CRASH_FillupCrashTxt) names the report in the log instead.
//
// The report's paths are POSIX paths; an attachment's path arrives Windows-spelled, as the
// game spells every path, and is mapped by the kernel layer.
//
// Modern build only: compiled by cmake/mac-modern.cmake (d2_modern_link_deps).
#if !defined(_WIN32) && D2_BACKEND_POSIX

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>

#include "Source/StormMac/Win32Posix/Win32PosixKernelPriv.h"

#include "3rdparty/crashy/Report.h"
#include "3rdparty/crashy/crashyRoot.h"
#include "3rdparty/crashy/crashy.globals.h"

namespace {

// The report directory's parent: the temporary directory's "Blizzard Error", as on Windows.
std::string ReportRoot() {
    const char* szTmp = getenv("TMPDIR");
    std::string s = szTmp && *szTmp ? szTmp : "/tmp";
    while (s.size() > 1 && s.back() == '/') {
        s.pop_back();
    }
    return s + "/Blizzard Error";
}

bool MakeDirs(const std::string& sPath) {
    for (size_t i = 1; i <= sPath.size(); ++i) {
        if (i == sPath.size() || sPath[i] == '/') {
            if (mkdir(sPath.substr(0, i).c_str(), 0755) != 0 && errno != EEXIST) {
                return false;
            }
        }
    }
    return true;
}

bool CopyFileBytes(const char* szFrom, const char* szTo) {
    int nIn = open(szFrom, O_RDONLY);
    if (nIn < 0) {
        return false;
    }
    int nOut = open(szTo, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (nOut < 0) {
        close(nIn);
        return false;
    }
    char aBuffer[16384];
    ssize_t nRead;
    bool bOk = true;
    while ((nRead = read(nIn, aBuffer, sizeof(aBuffer))) > 0) {
        if (write(nOut, aBuffer, (size_t)nRead) != nRead) {
            bOk = false;
            break;
        }
    }
    close(nIn);
    close(nOut);
    return bOk && nRead == 0;
}

// Copies s into a report path slot, as crashy's bounded copies do: false when it does not fit.
bool StorePath(char* pSlot, size_t nSlotSize, const std::string& s) {
    if (s.size() + 1 > nSlotSize) {
        return false;
    }
    memcpy(pSlot, s.c_str(), s.size() + 1);
    return true;
}

}  // namespace

namespace Fog::_3rdparty::crashy {

// The thread the crash path treats as the main one (Fog/Debug.cpp), set by the last of the
// game's static initialisers (Redivivus/d2_static_init.cpp), as 3rdparty/crashy/crashyRoot.cpp
// and Source/globals/crashy.cpp hold them on Windows.
uint32_t dwCurrentThreadId = 0;

void __stdcall SetCurrentThreadId() {
    dwCurrentThreadId = GetCurrentThreadId();
}

::Report* __thiscall ScalarDeletingDestructor(::Report* pThis, uint32_t nFlags) {
    Destructor(pThis);
    if (nFlags & 1) {
        ::operator delete(pThis);
    }
    return pThis;
}

namespace {
ReportVtable gReportVtable = {ScalarDeletingDestructor};
}

::Report* __thiscall Constructor(::Report* pThis) {
    pThis->_vfptr = &gReportVtable;
    pThis->pFile = nullptr;
    pThis->nAttachmentCount = 0;
    pThis->szReportDir[0] = '\0';
    pThis->szFilePath[0] = '\0';
    return pThis;
}

void __thiscall Destructor(::Report* pThis) {
    if (pThis->pFile) {
        fclose(pThis->pFile);
        pThis->pFile = nullptr;
    }
}

int8_t __thiscall Open(::Report* pThis, char* szFileName) {
    time_t tNow = time(nullptr);
    struct tm tLocal;
    localtime_r(&tNow, &tLocal);
    char szStamp[32];
    strftime(szStamp, sizeof(szStamp), "%Y-%m-%d %H.%M.%S", &tLocal);
    std::string sDir = ReportRoot() + "/" + szStamp;
    if (!MakeDirs(sDir) || !StorePath(pThis->szReportDir, sizeof(pThis->szReportDir), sDir) ||
        !StorePath(pThis->szFilePath, sizeof(pThis->szFilePath), sDir + "/" + szFileName)) {
        return 0;
    }
    pThis->pFile = fopen(pThis->szFilePath, "w");
    return pThis->pFile != nullptr;
}

int8_t __thiscall Close(::Report* pThis) {
    if (pThis->pFile) {
        fclose(pThis->pFile);
        pThis->pFile = nullptr;
    }
    return 1;
}

bool __thiscall IsOpen(::Report* pThis) {
    return pThis->pFile != nullptr;
}

int8_t __thiscall AddAttachment(::Report* pThis, char* szPath, int8_t bCopyToReportDir) {
    const int nSlots = (int)(sizeof(pThis->aszAttachmentPaths) / sizeof(pThis->aszAttachmentPaths[0]));
    if (pThis->nAttachmentCount >= nSlots || !szPath) {
        return 0;
    }
    std::string sPath = Win32Posix::Path_ToPosix(szPath);
    if (bCopyToReportDir && pThis->szReportDir[0]) {
        size_t nSlash = sPath.find_last_of('/');
        std::string sCopy = std::string(pThis->szReportDir) + "/" +
                            (nSlash == std::string::npos ? sPath : sPath.substr(nSlash + 1));
        if (!CopyFileBytes(sPath.c_str(), sCopy.c_str())) {
            return 0;
        }
        sPath = sCopy;
    }
    if (!StorePath(pThis->aszAttachmentPaths[pThis->nAttachmentCount], sizeof(pThis->aszAttachmentPaths[0]), sPath)) {
        return 0;
    }
    pThis->nAttachmentCount++;
    return 1;
}

int32_t Printf(::Report* pThis, char* szFormat, ...) {
    if (!pThis->pFile) {
        return 0;
    }
    va_list tArgs;
    va_start(tArgs, szFormat);
    int nWritten = vfprintf(pThis->pFile, szFormat, tArgs);
    va_end(tArgs);
    return nWritten;
}

uint32_t __stdcall isDebuggerPresent() {
    return IsDebuggerPresent() ? 1 : 0;
}

}  // namespace Fog::_3rdparty::crashy

#endif  // !defined(_WIN32) && D2_BACKEND_POSIX
