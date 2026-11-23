// Run-time loading of FFmpeg's shared libraries. See d2_ffmpeg.h.
#include "d2_ffmpeg.h"

#include <windows.h>
#include <stdio.h>

#define D2_STR2(x) #x
#define D2_STR(x) D2_STR2(x)

// The DLL file names carry the major version of each library, which the headers
// know. Load order follows the dependency order so each finds its imports already
// present (the loader resolves them from the application directory anyway).
static const char* const g_aszDlls[] = {
    "avutil-" D2_STR(LIBAVUTIL_VERSION_MAJOR) ".dll",
    "swresample-" D2_STR(LIBSWRESAMPLE_VERSION_MAJOR) ".dll",
    "swscale-" D2_STR(LIBSWSCALE_VERSION_MAJOR) ".dll",
    "avcodec-" D2_STR(LIBAVCODEC_VERSION_MAJOR) ".dll",
    "avformat-" D2_STR(LIBAVFORMAT_VERSION_MAJOR) ".dll",
};
enum { DLL_avutil, DLL_swresample, DLL_swscale, DLL_avcodec, DLL_avformat, DLL_COUNT };

static void Note(const char* pszFmt, const char* pszArg) {
    char szLine[512];
    snprintf(szLine, sizeof szLine, pszFmt, pszArg);
    OutputDebugStringA(szLine);
    fputs(szLine, stderr);
}

static const D2FFmpeg* Load() {
    static D2FFmpeg s_table;
    HMODULE ahMod[DLL_COUNT];
    for (int i = 0; i < DLL_COUNT; ++i) {
        ahMod[i] = LoadLibraryA(g_aszDlls[i]);
        if (!ahMod[i]) {
            Note("x64deps: %s not found beside the executable; Bink and Smacker videos are skipped\n",
                 g_aszDlls[i]);
            for (int j = 0; j < i; ++j) FreeLibrary(ahMod[j]);
            return nullptr;
        }
    }
    bool bOk = true;
#define D2_FFMPEG_RESOLVE(dll, name)                                                      \
    s_table.name = reinterpret_cast<decltype(s_table.name)>(                            \
        reinterpret_cast<void*>(GetProcAddress(ahMod[DLL_##dll], #name)));              \
    if (!s_table.name) {                                                                \
        Note("x64deps: FFmpeg entry point %s missing; the DLLs do not match the build\n", #name); \
        bOk = false;                                                                    \
    }
    D2_FFMPEG_FUNCS(D2_FFMPEG_RESOLVE)
#undef D2_FFMPEG_RESOLVE
    if (!bOk) return nullptr;
    s_table.av_log_set_level(AV_LOG_ERROR);
    return &s_table;
}

const D2FFmpeg* D2FFmpeg_Get() {
    static bool s_bTried = false;
    static const D2FFmpeg* s_pTable = nullptr;
    if (!s_bTried) {
        s_bTried = true;
        s_pTable = Load();
    }
    return s_pTable;
}

const char* const* D2FFmpeg_DllNames(int* pnCount) {
    if (pnCount) *pnCount = DLL_COUNT;
    return g_aszDlls;
}
