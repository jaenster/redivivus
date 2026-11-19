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

#include "Redivivus/d2_trace.h"

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

#ifdef _MSC_VER
// The Visual C++ 2005 build links the libcmt the original linked, so what is the
// CRT's own function comes from there. The two vector iterators above stay ours: libcmt's are
// `eh vector constructor iterator' (??_L) and `eh vector destructor iterator'
// (??_M), __stdcall, while the tree declares and calls these cdecl.

namespace _Wrappers {
// The C++ EH frame handler of the function that stores it (the frame is
// EHRegistrationNode-shaped: next, handler, state). The tree links that frame
// onto the global ExceptionList, not onto fs:[0], so the system never
// dispatches to them and nothing calls them.
void CRT_ExceptionFilter1(EHExceptionRecord*, EHRegistrationNode*, CONTEXT*, void*) {}
}  // namespace _Wrappers
#endif  // _MSC_VER
