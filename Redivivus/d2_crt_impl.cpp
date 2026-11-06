// Real implementations for CRT entry points the generator will not emit.
//
// These live in Ghidra's `compiler` / `_Wrappers` namespaces, which the
// reconstruction excludes because they are Microsoft's statically-linked CRT
// rather than Diablo II's code. They still have to exist for the executable to
// link, and a stub that returns does the wrong thing at run time. Each one below
// is the actual behaviour, not a placeholder; what remains stubbed in
// d2_link_support.s is what genuinely cannot be expressed here (MSVC's SEH
// unwinder, which mingw does not share an exception model with).

#include <windows.h>
#include <string.h>
#include <stdlib.h>

#include "support/d2_trace.h"

#ifndef _MSC_VER
// The CRT's strlen behind Ghidra's _Wrappers name.
namespace _Wrappers {
uint32_t CRT_StrLen(char* pText) { return pText ? (uint32_t)strlen(pText) : 0u; }

// Registered with atexit; the original body only clears the security cookie
// state, which has no counterpart here.
void CRT_SecurityCookieStub2(void) {}
}  // namespace _Wrappers

// __ftol2 takes its argument on the x87 stack and returns it truncated toward
// zero in EDX:EAX. That register contract cannot be written in portable C++, so
// this reads ST(0) directly. fistpll truncates when the rounding mode is set to
// chop, which is what the CRT routine arranges.
extern "C" uint64_t __ftol2(void) {
    long long r;
    unsigned short cw, cwChop;
    __asm__ __volatile__(
        "fnstcw %1\n\t"
        "movw %1, %%ax\n\t"
        "orw $0x0C00, %%ax\n\t"     // rounding control = truncate
        "movw %%ax, %2\n\t"
        "fldcw %2\n\t"
        "fistpll %0\n\t"
        "fldcw %1"
        : "=m"(r), "=m"(cw), "=m"(cwChop)
        :
        : "ax");
    return (uint64_t)r;
}

#endif  // !_MSC_VER

// MSVC's array construct/destruct helpers. Each walks `nCount` elements of
// `nElementSize` bytes and calls the per-element constructor or destructor, which
// take the element in ECX (MSVC types them __thiscall, the same ABI). The
// real ones also unwind already-built elements if a constructor throws, which is
// what the constructor's extra pfnDestructor argument is for; nothing in the
// reconstruction throws out of these, so the loop is the whole behaviour.
//
// Destruction runs back to front, as the CRT does.
extern "C" void _eh_vector_constructor_iterator_(void* pArray, uint32_t nElementSize,
                                                int nCount, void (__fastcall *pfnConstructor)(void*),
                                                void (__fastcall *pfnDestructor)(void*)) {
    (void)pfnDestructor;
    if (!pArray || !pfnConstructor) return;
    char* p = (char*)pArray;
    for (int i = 0; i < nCount; ++i) pfnConstructor(p + (size_t)i * nElementSize);
}

extern "C" void _eh_vector_destructor_iterator_(void* pArray, uint32_t nElementSize,
                                               int nCount, void (__fastcall *pfnDestructor)(void*)) {
    if (!pArray || !pfnDestructor) return;
    char* p = (char*)pArray;
    for (int i = nCount - 1; i >= 0; --i) pfnDestructor(p + (size_t)i * nElementSize);
}

// __CIsqrt is deliberately NOT implemented here. It receives its argument in
// ST(0) from the caller and returns through it, and GCC's x87 stack tracking
// makes that contract easy to get subtly wrong. It stays stubbed in
// d2_link_support.s until it is worth doing properly.

#ifdef _MSC_VER
// The Visual C++ 2005 build links the libcmt the original linked, so what is the
// CRT's own function comes from there. __ftol2 is libcmt's, and defining it
// here would collide with it. The two vector iterators above stay ours: libcmt's are
// `eh vector constructor iterator' (??_L) and `eh vector destructor iterator'
// (??_M), __stdcall, while the tree declares and calls these cdecl.

// libcmt's __CIsqrt takes ST(0) and returns in it; the tree declares and calls
// __CIsqrt as a cdecl function of a stack argument, a different symbol (one more
// leading underscore). VC8's long double is double.
extern "C" long double __CIsqrt(long double x) { return sqrt((double)x); }

namespace _Wrappers {
// The C++ EH frame handlers of the functions that store them (the frame is
// EHRegistrationNode-shaped: next, handler, state). The tree links that frame
// onto the global ExceptionList, not onto fs:[0], so the system never
// dispatches to them and nothing calls them.
void CRT_ExceptionFilter1(EHExceptionRecord*, EHRegistrationNode*, CONTEXT*, void*) {}
void CRT_ExceptionFilter2(EHExceptionRecord*, EHRegistrationNode*, CONTEXT*, void*) {}
void CRT_ExceptionFilter3(EHExceptionRecord*, EHRegistrationNode*, CONTEXT*, void*) {}
}  // namespace _Wrappers
#endif  // _MSC_VER
