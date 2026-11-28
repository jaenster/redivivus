// Win32Posix: what the kernel units share among themselves and with the other Win32Posix units
// (User, Sockets, DirectSound) - the handle table, the waitable objects, the path mapping and
// the error mapping. Not part of the surface the game sees.
//
// A HANDLE is a 32-bit value - a multiple of 4 from 0x100 up, as Windows hands them out - that
// indexes the table below; it is never a pointer. The table holds a reference to the object,
// so a handle closed on one thread while another waits on it stays valid for the waiter.
//
// Paths: the game sees Windows paths. The POSIX path /a/b is "C:\a\b" to it; any drive letter
// maps back to the root, separators are either slash, and a relative path is relative to the
// install directory. A component that does not exist under its own spelling is matched
// without case, as NTFS would.
//
// A unit reads the host headers it needs first, then this (see Win32PosixHost.h).
#pragma once

#if !defined(_WIN32) && D2_BACKEND_POSIX

#include "Source/StormMac/Win32Posix/Win32PosixHost.h"

namespace Win32Posix {

enum ObjectType : DWORD {
    OBJECT_NONE = 0,
    OBJECT_FILE,
    OBJECT_FIND,
    OBJECT_EVENT,
    OBJECT_THREAD,
    OBJECT_MAPPING,
    OBJECT_MODULE,
    OBJECT_REGKEY,
    OBJECT_COMPLETION_PORT,
    OBJECT_OTHER,
};

// A kernel object. Waitable objects keep bSignaled under the one dispatcher lock
// (DispatcherLock), so a wait on several objects sees them all at one instant.
struct Object {
    explicit Object(ObjectType eType) : eType(eType) {}
    virtual ~Object() {}
    const ObjectType eType;
    bool bWaitable = false;
    bool bSignaled = false;
    bool bManualReset = true;
};

// The handle table. Insert returns 0 when it is full.
HANDLE Handle_Insert(std::shared_ptr<Object> pObject);
std::shared_ptr<Object> Handle_Get(HANDLE hHandle);
template <class T> std::shared_ptr<T> Handle_GetAs(HANDLE hHandle, ObjectType eType) {
    std::shared_ptr<Object> p = Handle_Get(hHandle);
    if (!p || p->eType != eType) {
        return nullptr;
    }
    return std::static_pointer_cast<T>(p);
}
bool Handle_Remove(HANDLE hHandle);
// The table value as a DWORD (the handle values are 32-bit by construction).
DWORD Handle_Value(HANDLE hHandle);

// The dispatcher: every waitable object's state changes under it, and every change wakes the
// waiters.
std::mutex& DispatcherLock();
void Dispatcher_Signal(Object* pObject, bool bSignaled);  // takes the lock

// The thread-local last error, by its Windows code.
void SetError(DWORD dwError);
DWORD ErrorFromErrno(int nErrno);

// The user's home directory, and the directory applications keep their data in:
// ~/Library/Application Support on macOS, XDG_DATA_HOME (default ~/.local/share) elsewhere.
// POSIX paths without the trailing slash.
std::string HomeDir();
std::string AppSupportDir();
// The directories: the install directory (D2_INSTALL_DIR, default
// AppSupportDir()/Diablo II), and the one the MPQs are read from (D2_MPQ_DIR,
// else MpqDir in [Paths] of Win32Posix.ini in the install directory, else the install
// directory). Both are POSIX paths without the trailing slash, created on first use.
const std::string& InstallDir();
const std::string& MpqDir();
// The POSIX path of a Windows one, matched without case where its own spelling is missing; a
// component nothing matches (a file about to be created) keeps the spelling given.
std::string Path_ToPosix(const char* szWinPath);
// The POSIX path of a Windows one for a read: as Path_ToPosix, and an .mpq file the install
// directory does not hold is looked for in the MPQ directory.
std::string Path_ToPosixForRead(const char* szWinPath);
// The Windows path the game sees for a POSIX one.
std::string Path_ToWindows(const std::string& sPosixPath);

// The FILETIME (100 ns since 1601) of a POSIX time.
FILETIME FileTime_FromTimespec(const struct timespec& tTime);
struct timespec FileTime_ToTimespec(const FILETIME& tFileTime);

// The exit code of a thread handle, STILL_ACTIVE (0x103) while it runs; false when the handle
// is not a thread.
bool Thread_ExitCode(HANDLE hThread, LPDWORD pdwExitCode);

// The file descriptor behind a file handle, -1 when it is not one.
int File_Descriptor(HANDLE hFile);

// An .ini file as GetPrivateProfileString reads it: sections and keys matched without case,
// comments and blank lines kept for the write back. The registry's file is one too.
struct IniFile {
    struct Entry {
        std::string sKey;  // empty for a line that is not key=value
        std::string sValue;
        std::string sLine;  // the line as read, for the others
    };
    struct Section {
        std::string sName;
        std::vector<Entry> aEntries;
    };
    std::vector<Entry> aPreamble;  // the lines before the first section
    std::vector<Section> aSections;

    bool Load(const std::string& sPath);  // false (and empty) when there is no such file
    bool Save(const std::string& sPath) const;
    Section* Find(const char* szSection);
    Section& Get(const char* szSection);  // created at the end when missing
    static Entry* Find(Section& tSection, const char* szKey);
};

// Queues a completion on a port, as PostQueuedCompletionStatus does: the Sockets units post
// their finished overlapped operations through it.
bool CompletionPort_Post(HANDLE hPort, DWORD dwBytes, ULONG_PTR nKey, LPOVERLAPPED pOverlapped);
// The port and key a handle was associated with by CreateIoCompletionPort; false when none.
bool CompletionPort_Lookup(HANDLE hFile, HANDLE* phPort, ULONG_PTR* pnKey);

}  // namespace Win32Posix

#endif  // !defined(_WIN32) && D2_BACKEND_POSIX
