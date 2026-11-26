// The link definitions the 64-bit build has to spell differently.
//
// Redivivus/d2_link_support.s defines _Wrappers::CRT_ExceptionFilter1 as an
// i686 stub (the decorated symbol, `xorl %eax,%eax; ret`); that file is x86
// assembly and is left out of the 64-bit build (CMakeLists.txt). The handler is
// the C++ EH frame handler VC8 stored into the SEH frame of ErrorManager.cpp's
// halt (00682ea0, `__frame0.pSehHandler`). The tree links that frame onto its
// own `ExceptionList` variable, not onto the thread's chain, so no dispatcher
// ever calls it on either target; this is the same no-op, as a C++ definition.
#include "D2Arch.h"

#if !D2_ARCH_X64
#error "d2_crt_x64.cpp is for the 64-bit build only; the 32-bit build has d2_link_support.s"
#endif

#include <windows.h>

struct EHExceptionRecord;
struct EHRegistrationNode;

namespace _Wrappers {
void CRT_ExceptionFilter1(EHExceptionRecord*, EHRegistrationNode*, CONTEXT*, void*) {}
}  // namespace _Wrappers

// VC8's _rotl16 intrinsic (ROL r16, imm8). On x86-64 mingw's winnt.h declares it
// as a CRT function, so d2_platform.h's inline stands aside and this is the
// definition.
extern "C" unsigned short __cdecl _rotl16(unsigned short value, unsigned char shift) {
    return (unsigned short)((value << shift) | (value >> (16 - shift)));
}

// Fog's LOGMGR_GetTime32 and D2Common's SEED_GetTime32 copy VC8's inline time(),
// which calls _time32. The UCRT exports it; the x86-64 msvcrt.dll import library
// of older mingw-w64 (the distro packages CI uses) does not, so the import slot
// is filled here from _time64, which every msvcrt exports.
#include <time.h>
#if !defined(_UCRT)
static __time32_t __cdecl d2_time32(__time32_t* pTime) {
    __time32_t nTime = (__time32_t)_time64(nullptr);
    if (pTime) *pTime = nTime;
    return nTime;
}
extern "C" __time32_t(__cdecl* __imp__time32)(__time32_t*) = d2_time32;
#endif
