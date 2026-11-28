// The MPQ directory's lookup order (Win32PosixKernelPriv.h, MpqDir): D2_MPQ_DIR, the directory
// holding the .app, the install directory, the folder kept in Win32Posix.ini, and the fallback
// to the install directory; and MpqDir_Store, the folder picker's choice kept for the next
// launch. MpqDir is worked out once per process, so each case is a child process: this test's
// own executable, copied into a scratch Diablo II.app and started with the case's environment.
// No window, no SDL.
//
//   ninja -C <modern build> d2_mpqdir_test && <modern build>/d2_mpqdir_test
#if !defined(_WIN32) && D2_BACKEND_POSIX

#include <limits.h>
#include <mach-o/dyld.h>
#include <spawn.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>

#include <string>
#include <vector>

#include "Source/StormMac/Win32Posix/Win32PosixKernelPriv.h"

extern char** environ;

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

std::string g_sRoot;    // the scratch directory, real path
std::string g_sApp;     // the copied test executable inside <root>/beside/Diablo II.app
std::string g_sResult;  // where a child writes its answer

void Touch(const std::string& sPath) {
    FILE* pFile = fopen(sPath.c_str(), "wb");
    if (pFile) {
        fclose(pFile);
    }
}

std::string ReadAll(const std::string& sPath) {
    std::string s;
    FILE* pFile = fopen(sPath.c_str(), "rb");
    if (pFile) {
        char aBuffer[4096];
        size_t n = fread(aBuffer, 1, sizeof(aBuffer), pFile);
        s.assign(aBuffer, n);
        fclose(pFile);
    }
    return s;
}

// Runs the bundled copy as a child with the case's install directory and D2_MPQ_DIR (empty:
// unset); `store` makes the child call MpqDir_Store(sStore) first. The child's MpqDir().
std::string RunCase(const std::string& sInstall, const std::string& sMpqEnv, const std::string& sStore = "") {
    unlink(g_sResult.c_str());
    std::vector<std::string> aEnv = {"D2_INSTALL_DIR=" + sInstall, "D2_MPQDIR_TEST_OUT=" + g_sResult,
                                     "HOME=" + g_sRoot};
    if (!sMpqEnv.empty()) {
        aEnv.push_back("D2_MPQ_DIR=" + sMpqEnv);
    }
    if (!sStore.empty()) {
        aEnv.push_back("D2_MPQDIR_TEST_STORE=" + sStore);
    }
    std::vector<char*> aEnvp;
    for (std::string& s : aEnv) {
        aEnvp.push_back(&s[0]);
    }
    aEnvp.push_back(nullptr);
    char szChild[] = "child";
    char* aArgv[] = {&g_sApp[0], szChild, nullptr};
    pid_t nPid;
    if (posix_spawn(&nPid, g_sApp.c_str(), nullptr, nullptr, aArgv, aEnvp.data()) != 0) {
        return "<spawn failed>";
    }
    int nStatus = 0;
    waitpid(nPid, &nStatus, 0);
    return ReadAll(g_sResult);
}

int Child() {
    const char* szStore = getenv("D2_MPQDIR_TEST_STORE");
    if (szStore && !Win32Posix::MpqDir_Store(szStore)) {
        return 1;
    }
    FILE* pFile = fopen(getenv("D2_MPQDIR_TEST_OUT"), "wb");
    if (!pFile) {
        return 1;
    }
    fputs(Win32Posix::MpqDir().c_str(), pFile);
    fclose(pFile);
    return 0;
}

}  // namespace

int main(int argc, char** argv) {
    if (argc > 1 && strcmp(argv[1], "child") == 0) {
        return Child();
    }
    char szTemplate[] = "/tmp/win32posix-mpqdir-XXXXXX";
    char* szDir = mkdtemp(szTemplate);
    char szReal[PATH_MAX];
    if (!szDir || !realpath(szDir, szReal)) {
        perror("mkdtemp");
        return 2;
    }
    g_sRoot = szReal;
    g_sResult = g_sRoot + "/result.txt";
    const std::string sBeside = g_sRoot + "/beside";
    const std::string sInstall = g_sRoot + "/install";
    const std::string sPicked = g_sRoot + "/picked";
    const std::string sEnv = g_sRoot + "/env";
    for (const std::string& s : {sBeside, sBeside + "/Diablo II.app", sBeside + "/Diablo II.app/Contents",
                                 sBeside + "/Diablo II.app/Contents/MacOS", sInstall, sPicked, sEnv}) {
        mkdir(s.c_str(), 0755);
    }
    char szSelf[PATH_MAX];
    uint32_t nSelf = sizeof(szSelf);
    if (_NSGetExecutablePath(szSelf, &nSelf) != 0) {
        return 2;
    }
    g_sApp = sBeside + "/Diablo II.app/Contents/MacOS/Diablo II";
    std::string sCopy = "cp '" + std::string(szSelf) + "' '" + g_sApp + "'";
    if (system(sCopy.c_str()) != 0) {
        return 2;
    }
    Touch(sPicked + "/D2DATA.MPQ");  // matched without case

    // Nothing anywhere: the install directory.
    CHECK(RunCase(sInstall, "") == sInstall);
    // D2_MPQ_DIR first, data or not.
    CHECK(RunCase(sInstall, sEnv) == sEnv);
    // The picker's choice: kept in Win32Posix.ini and read back on the next launch.
    CHECK(RunCase(sInstall, "", sPicked) == sPicked);
    CHECK(Win32Posix::HasGameData(sPicked) && !Win32Posix::HasGameData(sInstall));
    CHECK(ReadAll(sInstall + "/Win32Posix.ini").find("MpqDir=" + sPicked) != std::string::npos);
    CHECK(RunCase(sInstall, "") == sPicked);
    // The install directory with the data comes before the kept choice.
    Touch(sInstall + "/d2data.mpq");
    CHECK(RunCase(sInstall, "") == sInstall);
    // The directory holding the .app comes before both.
    Touch(sBeside + "/d2data.mpq");
    CHECK(RunCase(sInstall, "") == sBeside);
    // And D2_MPQ_DIR before that.
    CHECK(RunCase(sInstall, sEnv) == sEnv);

    printf("mpqdir: %d checks, %d failed (%s)\n", g_nChecks, g_nFailures, g_sRoot.c_str());
    if (g_nFailures == 0) {
        std::string sCommand = "rm -rf '" + g_sRoot + "'";
        if (system(sCommand.c_str()) != 0) {
            fprintf(stderr, "could not remove %s\n", g_sRoot.c_str());
        }
    }
    return g_nFailures ? 1 : 0;
}

#endif  // !defined(_WIN32) && D2_BACKEND_POSIX
