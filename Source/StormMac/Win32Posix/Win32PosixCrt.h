// Win32Posix: the Microsoft C runtime's path functions, over the kernel's path mapping.
//
// The shared game code opens files with the C runtime (fopen, stat, _fullpath ...) and hands it
// Windows paths: "C:\...", backslashes, names whose case is whatever the original wrote. The
// host C library takes none of that - fopen("D2\\logs\\x.txt") makes a file with backslashes in
// its name in the current directory. These are the CRT's calls, taking the same paths the
// Win32 file calls take (Win32Posix::Path_ToPosix: drive letters dropped, either separator,
// relative paths against the install directory, case-insensitive matching, the MPQ directory
// for reads of the game's archives), so the shared code is not edited.
//
// d2_platform.h reads this last, after every system header: the names the host C library
// declares itself (fopen, freopen, stat, rename) are redirected with function-like macros, so
// a declaration or a plain mention of them is untouched; the names only the Microsoft CRT has
// are declared here as they are. A file opened for writing or appending gets its directory
// created as needed (the game's logs and saves are opened that way beneath directories that
// do not exist yet).
//
// Not redirected: remove, which the C++ library's algorithm of the same name makes unsafe to
// macro, and which the shared code does not call (it deletes through DeleteFileA).
// The text mode's CR LF translation is not done: a file is written as the host writes it.
#if !defined(_WIN32) && D2_BACKEND_POSIX

// Read twice by a unit that wants the redirects after Win32PosixHost.h (which reads
// d2_platform.h with them off, so the kernel's own fopen and stat stay the host's): the
// declarations once, the macros each time they are not suppressed.
#ifndef WIN32POSIX_CRT_DECLARED
#define WIN32POSIX_CRT_DECLARED

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/stat.h>

// io.h and fcntl.h: the flags as the Microsoft CRT numbers them (the host's differ).
#ifndef _O_RDONLY
#  define _O_RDONLY 0x0000
#  define _O_WRONLY 0x0001
#  define _O_RDWR 0x0002
#  define _O_APPEND 0x0008
#  define _O_NOINHERIT 0x0080
#  define _O_CREAT 0x0100
#  define _O_TRUNC 0x0200
#  define _O_EXCL 0x0400
#  define _O_TEXT 0x4000
#  define _O_BINARY 0x8000
#endif

// io.h: the _findfirst record (size and times 32-bit, as _finddata_t is on Windows).
struct _finddata_t {
    unsigned attrib;
    int time_create;
    int time_access;
    int time_write;
    unsigned size;
    char name[260];
};
#define _A_NORMAL 0x00
#define _A_RDONLY 0x01
#define _A_HIDDEN 0x02
#define _A_SYSTEM 0x04
#define _A_SUBDIR 0x10
#define _A_ARCH 0x20

#define _MAX_PATH 260
#define _MAX_DRIVE 3
#define _MAX_DIR 256
#define _MAX_FNAME 256
#define _MAX_EXT 256

// Behind the redirecting macros below: stdio.h's.
FILE* Win32Posix_fopen(const char* szPath, const char* szMode);
FILE* Win32Posix_freopen(const char* szPath, const char* szMode, FILE* pStream);
int Win32Posix_stat(const char* szPath, struct stat* pStat);
int Win32Posix_rename(const char* szOld, const char* szNew);

// stdio.h, io.h, direct.h, stdlib.h: the Microsoft CRT's own.
FILE* _fsopen(const char* szPath, const char* szMode, int nShare);
int _open(const char* szPath, int nFlags, int nMode = 0);
int _access(const char* szPath, int nMode);
int _stat(const char* szPath, struct stat* pStat);
int _stat64(const char* szPath, struct stat* pStat);
int _mkdir(const char* szPath);
int _rmdir(const char* szPath);
int _unlink(const char* szPath);
int _chmod(const char* szPath, int nMode);
int _chdir(const char* szPath);
char* _getcwd(char* szBuffer, int nMaxLength);
intptr_t _findfirst(const char* szFileSpec, struct _finddata_t* pFindData);
int _findnext(intptr_t hFind, struct _finddata_t* pFindData);
int _findclose(intptr_t hFind);
void _splitpath(const char* szPath, char* szDrive, char* szDir, char* szFname, char* szExt);
void _makepath(char* szPath, const char* szDrive, const char* szDir, const char* szFname, const char* szExt);

#endif  // WIN32POSIX_CRT_DECLARED

#if !defined(WIN32POSIX_NO_CRT_REDIRECT) && !defined(WIN32POSIX_CRT_REDIRECTED)
#define WIN32POSIX_CRT_REDIRECTED
#define fopen(szPath, szMode) Win32Posix_fopen(szPath, szMode)
#define freopen(szPath, szMode, pStream) Win32Posix_freopen(szPath, szMode, pStream)
#define stat(szPath, pStat) Win32Posix_stat(szPath, pStat)
#define rename(szOld, szNew) Win32Posix_rename(szOld, szNew)
#endif

#endif  // !defined(_WIN32) && D2_BACKEND_POSIX
