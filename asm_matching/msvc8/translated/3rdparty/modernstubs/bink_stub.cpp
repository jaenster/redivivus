// LINK STUB, not RAD's Bink, for the modern build.
//
// RAD Game Tools' Bink is proprietary and nothing of it is vendored. D2OpenGL's cutscene player
// (BinkCutscene.cpp) opens a video with BinkOpen and plays it with the calls below. Here
// BinkOpen finds no video, so OGL_PlayVideoFile goes on as for a missing file and the game
// skips the cutscene; the other entry points are never reached with a video, and do nothing.
// The signatures are d2_platform.h's.
//
// Modern build only: compiled by cmake/mac-modern.cmake (d2_modern_link_deps).
#if !defined(_WIN32) && D2_BACKEND_POSIX

#include "Source/D2Headers/d2_platform.h"

extern "C" {

void* __stdcall BinkOpen(void* hFile, uint32_t dwFlags) {
    (void)hFile;
    (void)dwFlags;
    return nullptr;
}

void __stdcall BinkClose(void* pBink) {
    (void)pBink;
}

int32_t __stdcall BinkDoFrame(void* pBink) {
    (void)pBink;
    return 0;
}

void __stdcall BinkNextFrame(void* pBink) {
    (void)pBink;
}

int32_t __stdcall BinkWait(void* pBink) {
    (void)pBink;
    return 0;
}

int32_t __stdcall BinkCopyToBuffer(void* pBink, void* pDest, int32_t nDestPitch, uint32_t nDestHeight, uint32_t nDestX,
                                   uint32_t nDestY, uint32_t dwFlags) {
    (void)pBink;
    (void)pDest;
    (void)nDestPitch;
    (void)nDestHeight;
    (void)nDestX;
    (void)nDestY;
    (void)dwFlags;
    return 0;
}

// No sound system to open: BinkSetSoundSystem reports failure, and Bink plays silently.
int32_t __stdcall BinkSetSoundSystem(void*(__stdcall* pfnOpen)(uint32_t), uint32_t dwParam) {
    (void)pfnOpen;
    (void)dwParam;
    return 0;
}

void* __stdcall BinkOpenDirectSound(uint32_t dwParam) {
    (void)dwParam;
    return nullptr;
}

}  // extern "C"

#endif  // !defined(_WIN32) && D2_BACKEND_POSIX
