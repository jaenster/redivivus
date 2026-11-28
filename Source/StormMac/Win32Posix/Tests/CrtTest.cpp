// The Win32Posix CRT path calls (Win32PosixCrt.h): fopen, stat, _access, _mkdir, _findfirst ...
// with Windows-spelled paths - a drive letter, backslashes, another case, relative to the
// install directory - in a fresh temporary install directory (D2_INSTALL_DIR).
//
//   ninja -C <modern build> d2_win32posix_crt_test && <modern build>/d2_win32posix_crt_test
//
// It reads the host's headers, then the Win32 surface, then Win32PosixCrt.h last - the order
// d2_platform.h gives the game's units - so the names it calls are the redirected ones. It
// exits nonzero when a check fails.
#if !defined(_WIN32) && D2_BACKEND_POSIX

#include <dirent.h>
#include <limits.h>
#include <unistd.h>

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <vector>

#include "Source/StormMac/Win32Posix/Win32PosixHost.h"
#include "Source/StormMac/Win32Posix/Win32PosixCrt.h"

namespace {

int g_nChecks = 0;
int g_nFailures = 0;

void Check(bool bOk, const char* szWhat, int nLine) {
    ++g_nChecks;
    if (!bOk) {
        ++g_nFailures;
        fprintf(stderr, "FAIL line %d: %s\n", nLine, szWhat);
    }
}
#define CHECK(x) Check((x), #x, __LINE__)

std::string g_sDir;      // the install directory, POSIX
std::string g_sCwd;      // the process's directory, where nothing may appear
std::string g_sWinDir;   // the install directory, Windows-spelled, with the separator

bool ExistsPosix(const std::string& sPath) {
    struct stat tStat;
    return (stat)(sPath.c_str(), &tStat) == 0;  // the host's own, unredirected
}

std::string ReadAll(const std::string& sPosixPath) {
    std::string s;
    FILE* p = (fopen)(sPosixPath.c_str(), "rb");
    if (!p) {
        return s;
    }
    char aBuf[256];
    size_t n;
    while ((n = fread(aBuf, 1, sizeof(aBuf), p)) > 0) {
        s.append(aBuf, n);
    }
    fclose(p);
    return s;
}

void TestFopen() {
    // A write with a drive letter and backslashes creates the directories above the leaf.
    FILE* p = fopen((g_sWinDir + "Save\\Chars\\Hero.d2s").c_str(), "wb");
    CHECK(p != nullptr);
    if (p) {
        fputs("stay a while", p);
        fclose(p);
    }
    CHECK(ReadAll(g_sDir + "/Save/Chars/Hero.d2s") == "stay a while");
    // Read back through other spellings: lower case, forward slashes, relative.
    p = fopen((g_sWinDir + "save\\CHARS\\hero.D2S").c_str(), "rb");
    CHECK(p != nullptr);
    if (p) {
        char aBuf[32] = {};
        CHECK(fread(aBuf, 1, sizeof(aBuf), p) == 12 && memcmp(aBuf, "stay a while", 12) == 0);
        fclose(p);
    }
    p = fopen("SAVE/chars/HERO.d2s", "r");
    CHECK(p != nullptr);
    if (p) {
        fclose(p);
    }
    CHECK(fopen("Save\\Chars\\Nothing.d2s", "rb") == nullptr && errno == ENOENT);
    // The write of an existing file under another case takes the existing file.
    p = fopen("save\\chars\\hero.d2s", "ab");
    CHECK(p != nullptr);
    if (p) {
        fputs("!", p);
        fclose(p);
    }
    CHECK(ReadAll(g_sDir + "/Save/Chars/Hero.d2s") == "stay a while!");
    // The Microsoft modes: "at+" is not append-only, "wt" and "rb" are what they say.
    p = fopen("Log.txt", "at+");
    CHECK(p != nullptr);
    if (p) {
        fputs("one\n", p);
        fseek(p, 0, SEEK_SET);
        char aBuf[8] = {};
        CHECK(fgets(aBuf, sizeof(aBuf), p) != nullptr && strcmp(aBuf, "one\n") == 0);
        fclose(p);
    }
    p = fopen("Log.txt", "wt");
    CHECK(p != nullptr);
    if (p) {
        fclose(p);
    }
    CHECK(ReadAll(g_sDir + "/Log.txt").empty());
    // freopen and _fsopen go the same way.
    p = _fsopen("Sh\\Open.txt", "w", 0x40);
    CHECK(p != nullptr && ExistsPosix(g_sDir + "/Sh/Open.txt"));
    if (p) {
        p = freopen("Sh\\Re.txt", "w", p);
        CHECK(p != nullptr && ExistsPosix(g_sDir + "/Sh/Re.txt"));
        if (p) {
            fclose(p);
        }
    }
    // The logs: a relative path lands in the install directory, not in the process's.
    p = fopen("logs\\d2_251101.txt", "w");
    CHECK(p != nullptr);
    if (p) {
        fclose(p);
    }
    CHECK(ExistsPosix(g_sDir + "/logs/d2_251101.txt"));
    CHECK(!ExistsPosix(g_sCwd + "/logs\\d2_251101.txt"));
    CHECK(!ExistsPosix(g_sCwd + "/logs"));
    // And nothing with a backslash in its name was made anywhere.
    for (const std::string& sDir : {g_sDir, g_sCwd}) {
        DIR* pDir = opendir(sDir.c_str());
        bool bBackslash = false;
        while (pDir) {
            struct dirent* pEntry = readdir(pDir);
            if (!pEntry) {
                break;
            }
            bBackslash |= strchr(pEntry->d_name, '\\') != nullptr;
        }
        if (pDir) {
            closedir(pDir);
        }
        CHECK(!bBackslash);
    }
}

void TestStatAccess() {
    struct stat tStat;
    CHECK(stat((g_sWinDir + "SAVE\\chars\\hero.d2s").c_str(), &tStat) == 0 && tStat.st_size == 13);
    CHECK(_stat("save\\Chars", &tStat) == 0 && S_ISDIR(tStat.st_mode));
    CHECK(_stat64("Save\\Chars\\Hero.d2s", &tStat) == 0);
    CHECK(stat("Save\\Missing", &tStat) == -1 && errno == ENOENT);
    CHECK(_access((g_sWinDir + "Save\\CHARS\\HERO.D2S").c_str(), 0) == 0);
    CHECK(_access("Save\\Chars\\Hero.d2s", 6) == 0);
    CHECK(_access("Save\\Chars\\Nope.d2s", 0) == -1 && errno == ENOENT);
}

void TestDirectories() {
    CHECK(_mkdir((g_sWinDir + "Dir1").c_str()) == 0);
    CHECK(_mkdir("dir1") == -1 && errno == EEXIST);
    CHECK(_chdir("DIR1") == 0);
    CHECK(_chdir("Save\\Chars\\Hero.d2s") == -1 && errno == ENOTDIR);
    CHECK(_chdir("Nowhere") == -1);
    CHECK(_rmdir("DIR1") == 0 && !ExistsPosix(g_sDir + "/Dir1"));
    char aBuf[MAX_PATH];
    CHECK(_getcwd(aBuf, sizeof(aBuf)) == aBuf && strcmp(aBuf, g_sWinDir.substr(0, g_sWinDir.size() - 1).c_str()) == 0);
    char aFull[MAX_PATH];
    CHECK(_fullpath(aFull, "save\\..\\Save\\Chars", sizeof(aFull)) == aFull &&
          strcmp(aFull, (g_sWinDir + "Save\\Chars").c_str()) == 0);
    // Rename (the new name's directories are made) and unlink.
    CHECK(rename("Log.txt", "Moved\\Log2.txt") == 0);
    CHECK(ExistsPosix(g_sDir + "/Moved/Log2.txt") && !ExistsPosix(g_sDir + "/Log.txt"));
    CHECK(_unlink("MOVED\\log2.txt") == 0 && !ExistsPosix(g_sDir + "/Moved/Log2.txt"));
    CHECK(_unlink("Moved\\Log2.txt") == -1 && errno == ENOENT);
}

void TestFind() {
    for (const char* szName : {"Find\\A.d2s", "Find\\b.D2S", "Find\\c.txt"}) {
        FILE* p = fopen(szName, "w");
        CHECK(p != nullptr);
        if (p) {
            fputs("xy", p);
            fclose(p);
        }
    }
    CHECK(_mkdir("Find\\Sub") == 0);
    struct _finddata_t tData;
    intptr_t hFind = _findfirst((g_sWinDir + "FIND\\*.d2s").c_str(), &tData);
    CHECK(hFind != -1);
    std::vector<std::string> aNames;
    if (hFind != -1) {
        do {
            aNames.push_back(tData.name);
            CHECK(tData.size == 2 && !(tData.attrib & _A_SUBDIR));
        } while (_findnext(hFind, &tData) == 0);
        CHECK(_findnext(hFind, &tData) == -1);
        CHECK(_findclose(hFind) == 0);
    }
    CHECK(aNames.size() == 2 && aNames[0] == "A.d2s" && aNames[1] == "b.D2S");
    hFind = _findfirst("find\\Sub", &tData);
    CHECK(hFind != -1 && (tData.attrib & _A_SUBDIR) && strcmp(tData.name, "Sub") == 0);
    if (hFind != -1) {
        _findclose(hFind);
    }
    CHECK(_findfirst("Find\\*.zzz", &tData) == -1 && errno == ENOENT);
}

void TestOpen() {
    int fd = _open("Fd\\Raw.bin", _O_CREAT | _O_WRONLY | _O_TRUNC | _O_BINARY, 0644);
    CHECK(fd >= 0);
    if (fd >= 0) {
        CHECK(write(fd, "abc", 3) == 3);
        close(fd);
    }
    CHECK(ReadAll(g_sDir + "/Fd/Raw.bin") == "abc");
    fd = _open("FD\\raw.BIN", _O_RDONLY | _O_BINARY);
    CHECK(fd >= 0);
    if (fd >= 0) {
        close(fd);
    }
    CHECK(_open("Fd\\Raw.bin", _O_CREAT | _O_EXCL | _O_WRONLY) == -1 && errno == EEXIST);
}

void TestSplitMake() {
    char aDrive[_MAX_DRIVE], aDir[_MAX_DIR], aName[_MAX_FNAME], aExt[_MAX_EXT];
    _splitpath("C:\\Diablo II\\Save\\Hero.d2s", aDrive, aDir, aName, aExt);
    CHECK(strcmp(aDrive, "C:") == 0 && strcmp(aDir, "\\Diablo II\\Save\\") == 0 &&
          strcmp(aName, "Hero") == 0 && strcmp(aExt, ".d2s") == 0);
    _splitpath("Hero", aDrive, aDir, aName, aExt);
    CHECK(aDrive[0] == 0 && aDir[0] == 0 && strcmp(aName, "Hero") == 0 && aExt[0] == 0);
    char aPath[MAX_PATH];
    _makepath(aPath, "C", "Save", "Hero", "d2s");
    CHECK(strcmp(aPath, "C:Save\\Hero.d2s") == 0);
}

}  // namespace

int main() {
    char aTemplate[] = "/tmp/d2crttestXXXXXX";
    char* szTmp = mkdtemp(aTemplate);
    if (!szTmp) {
        perror("mkdtemp");
        return 2;
    }
    char aReal[PATH_MAX];
    g_sDir = realpath(szTmp, aReal) ? aReal : szTmp;
    g_sDir += "/install";
    setenv("D2_INSTALL_DIR", g_sDir.c_str(), 1);
    char aCwd[PATH_MAX];
    g_sCwd = getcwd(aCwd, sizeof(aCwd)) ? aCwd : ".";
    char aWin[MAX_PATH];
    GetModuleFileNameA(nullptr, aWin, sizeof(aWin));  // makes the install directory
    g_sDir = [] {
        char aBuf[PATH_MAX];
        return std::string(realpath(getenv("D2_INSTALL_DIR"), aBuf) ? aBuf : getenv("D2_INSTALL_DIR"));
    }();
    g_sWinDir = "C:" + g_sDir + "/";
    for (char& c : g_sWinDir) {
        if (c == '/') {
            c = '\\';
        }
    }
    TestFopen();
    TestStatAccess();
    TestDirectories();
    TestFind();
    TestOpen();
    TestSplitMake();
    printf("%d checks, %d failures\n", g_nChecks, g_nFailures);
    std::string sCleanup = "rm -rf '" + std::string(szTmp) + "'";
    if (g_nFailures == 0) {
        (void)system(sCleanup.c_str());
    }
    return g_nFailures ? 1 : 0;
}

#endif  // !defined(_WIN32) && D2_BACKEND_POSIX
