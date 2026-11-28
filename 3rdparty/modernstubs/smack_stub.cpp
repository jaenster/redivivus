// LINK STUB, not RAD's Smacker, for the modern build.
//
// RAD Game Tools' Smacker is proprietary and nothing of it is vendored. D2Uni's movie slot
// (uniSmack.cpp, UniDecodeSmacker and UniPlayerSmacker) opens a Smacker file with SmackOpen and
// plays it with the calls below. Here SmackOpen finds no movie, so UniDecodeSmacker leaves the
// context without a handle and UniPlayerSmacker returns at once: the game skips the movie. The
// other entry points are never reached without a handle, and do nothing.
// The signatures are d2_platform.h's.
//
// Modern build only: compiled by cmake/mac-modern.cmake (d2_modern_link_deps).
#if !defined(_WIN32) && D2_BACKEND_POSIX

#include "Source/D2Headers/d2_platform.h"

extern "C" {

void* __stdcall SmackOpen(const char* szName, uint32_t dwFlags, uint32_t dwExtraBuf) {
    (void)szName;
    (void)dwFlags;
    (void)dwExtraBuf;
    return nullptr;
}

void __stdcall SmackClose(void* pSmack) {
    (void)pSmack;
}

int32_t __stdcall SmackDoFrame(void* pSmack) {
    (void)pSmack;
    return 0;
}

void __stdcall SmackNextFrame(void* pSmack) {
    (void)pSmack;
}

// Nonzero: not yet time for a frame. Never reached without a handle.
int32_t __stdcall SmackWait(void* pSmack) {
    (void)pSmack;
    return 1;
}

void __stdcall SmackToBuffer(void* pSmack, uint32_t nLeft, uint32_t nTop, uint32_t nPitch, uint32_t nDestHeight,
                             void* pBuffer, uint32_t dwFlags) {
    (void)pSmack;
    (void)nLeft;
    (void)nTop;
    (void)nPitch;
    (void)nDestHeight;
    (void)pBuffer;
    (void)dwFlags;
}

}  // extern "C"

#endif  // !defined(_WIN32) && D2_BACKEND_POSIX
