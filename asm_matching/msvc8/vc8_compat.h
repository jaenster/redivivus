// vc8_compat.h - forced first include of every oracle compile (cl /FI).
//
// The tree is written for a C++17 compiler; the original was built with Visual
// C++ 2005. This header supplies, for VC8, the handful of C++11/17 spellings the
// generator emits that have an exact C++03 equivalent. Nothing here may change
// generated code: each definition expands to what VC8 compiles the original
// construct to. What cannot be expressed as a macro is rewritten per file by
// vc8_translate.py instead (nested namespaces, packed structs, = default).
#pragma once

// A null pointer constant. VC8's `0` is the same constant; only overload
// resolution could tell them apart, and the tree overloads nothing on it.
#define nullptr 0

// d2_enums.h's only use: namespace-scope constants, which `const` gives the
// same internal linkage and constant-expression status.
#define constexpr const

// C++11 static_assert as a C++03 negative-array typedef. Same check, no code.
#define VC8_CAT2(a, b) a##b
#define VC8_CAT(a, b) VC8_CAT2(a, b)
#define static_assert(c, m) typedef char VC8_CAT(vc8_static_assert_, __COUNTER__)[(c) ? 1 : -1]

#define thread_local __declspec(thread)

// The Windows version the headers declare for. Unset, the 2003 SDK hides
// IsDebuggerPresent, TrackMouseEvent and other NT4+ APIs the tree calls. XP is
// the era's floor; a struct whose size depends on it would show in the audit
// as a differing cbSize constant.
#define _WIN32_WINNT 0x0501
#define WINVER 0x0501

// The time-stamp-counter intrinsic (<intrin.h> in VC8).
extern "C" unsigned __int64 __rdtsc(void);
#pragma intrinsic(__rdtsc)

// GCC's frame/return-address builtins, level 0 only (the only level the tree
// uses). The return address is MSVC's own intrinsic; the frame address is EBP.
#define __builtin_frame_address(level) vc8_frame_address()
static __forceinline void* vc8_frame_address(void) { void* p; __asm { mov p, ebp } return p; }
extern "C" void* _ReturnAddress(void);
#pragma intrinsic(_ReturnAddress)
#define __builtin_return_address(level) _ReturnAddress()

// d2_platform.h's _MSC_VER path of D2_Cpuid; VC8 declares it only in <intrin.h>,
// which would bring hundreds of declarations the tree redeclares its own way.
extern "C" void __cpuid(int cpuInfo[4], int infoType);
#pragma intrinsic(__cpuid)

// d2_platform.h re-declares mingw's socket types (psdk_inc/_ip_types.h) so that
// sockaddr_in.sin_addr is the 32-bit word Ghidra models. Under the Platform SDK
// winsock2.h declares them itself; claiming the guard skips the mingw copy.
#define _MINGW_IP_TYPES_H

// Winsock linkage. Game.exe calls ws2_32 through the linker's import stubs
// (`call 006b012c`, a `jmp [__imp_closesocket]`), which is what a declaration
// WITHOUT dllimport produces; the Platform SDK declares every Winsock function
// dllimport (`call [__imp_closesocket]`). Only crashy (3rdparty/crashy, 0x4016c0)
// calls through the IAT directly, and flags.tsv gives it the SDK's linkage back.
#ifndef WINSOCK_API_LINKAGE
#define WINSOCK_API_LINKAGE
#endif

// The tree declares `typedef int _EXCEPTION_DISPOSITION` (d2_enums.h); VC8's
// excpt.h declares an enum of that tag. Rename the CRT's while the Windows
// headers are read; vc8_after_windows.h drops the rename again.
#define _EXCEPTION_DISPOSITION vc8_EXCEPTION_DISPOSITION

// C4481: `override` is a VC8 extension keyword. C4480: so is an enum's
// underlying type. Both are what the tree means; neither changes code.
#pragma warning(disable: 4480 4481)

// C++11 <cmath> additions the tree uses. Ghidra lowers an x87 FISTP/FRNDINT
// (round in the current rounding mode) to std::nearbyint; this is that.
namespace std {
inline double nearbyint(double x) { double r; __asm { fld x } __asm { frndint } __asm { fstp r } return r; }
inline bool isnan(double x) { return x != x; }
}

// The tree's __thiscall free functions and pointer types reach VC8 as
// `__fastcall (A1, int, ...)` (vc8_translate.py; the keyword is accepted on a
// pointer type but not on a free function). A call goes through a plain C
// cast to the __thiscall pointer type instead of passing the unused int:
// same callee (receiver in ECX, the rest on the stack, popped by the callee),
// and the caller loads no EDX. `vc8_thiscall(f)` is that cast,
// `((R (__thiscall*)(A1, ...))(f))`, with the type worked out by the
// compiler: VC8 has no decltype, so the type is deduced inside sizeof and
// read back through a numbered nested class (the VC8 typeof technique Boost
// used; vc8_tc::extract), each call site taking its own number from
// __COUNTER__. A cast, not a call: through an inline function VC8 loads a
// vtable slot into EAX where the original, and the cast, use EDX. Both
// sizeof operands are unevaluated, so `f` is evaluated once, by the cast.
// The second one checks that the recorded __fastcall type is f's own.
#ifdef __cplusplus
namespace vc8_tc {
template<int N> struct id {};
struct none {};
template<class ID, class T = none> struct extract;
template<class ID> struct extract<ID, none> { template<bool> struct impl; typedef impl<true> type; };
template<class ID, class T> struct extract : extract<ID, none> {
    template<> struct impl<true> { typedef T type; };
    template<bool> struct impl;
    typedef impl<true> type;
};
template<class TC, class FC> struct sig { typedef TC tc; typedef FC fc; };
template<int N, class T> struct reg : extract<id<N>, T> { typedef char (*sized)[N]; };
template<int N> struct get { typedef typename extract<id<N> >::type::type::tc tc; typedef typename extract<id<N> >::type::type::fc fc; };
template<class F> char same(F);
template<int N, class R, class A1> typename reg<N, sig<R (__thiscall*)(A1), R (__fastcall*)(A1, int)> >::sized enc(R (__fastcall*)(A1, int));
template<int N, class R, class A1, class A2> typename reg<N, sig<R (__thiscall*)(A1, A2), R (__fastcall*)(A1, int, A2)> >::sized enc(R (__fastcall*)(A1, int, A2));
template<int N, class R, class A1, class A2, class A3> typename reg<N, sig<R (__thiscall*)(A1, A2, A3), R (__fastcall*)(A1, int, A2, A3)> >::sized enc(R (__fastcall*)(A1, int, A2, A3));
template<int N, class R, class A1, class A2, class A3, class A4> typename reg<N, sig<R (__thiscall*)(A1, A2, A3, A4), R (__fastcall*)(A1, int, A2, A3, A4)> >::sized enc(R (__fastcall*)(A1, int, A2, A3, A4));
template<int N, class R, class A1, class A2, class A3, class A4, class A5> typename reg<N, sig<R (__thiscall*)(A1, A2, A3, A4, A5), R (__fastcall*)(A1, int, A2, A3, A4, A5)> >::sized enc(R (__fastcall*)(A1, int, A2, A3, A4, A5));
template<int N, class R, class A1, class A2, class A3, class A4, class A5, class A6> typename reg<N, sig<R (__thiscall*)(A1, A2, A3, A4, A5, A6), R (__fastcall*)(A1, int, A2, A3, A4, A5, A6)> >::sized enc(R (__fastcall*)(A1, int, A2, A3, A4, A5, A6));
template<int N, class R, class A1, class A2, class A3, class A4, class A5, class A6, class A7> typename reg<N, sig<R (__thiscall*)(A1, A2, A3, A4, A5, A6, A7), R (__fastcall*)(A1, int, A2, A3, A4, A5, A6, A7)> >::sized enc(R (__fastcall*)(A1, int, A2, A3, A4, A5, A6, A7));
template<int N, class R, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8> typename reg<N, sig<R (__thiscall*)(A1, A2, A3, A4, A5, A6, A7, A8), R (__fastcall*)(A1, int, A2, A3, A4, A5, A6, A7, A8)> >::sized enc(R (__fastcall*)(A1, int, A2, A3, A4, A5, A6, A7, A8));
template<int N, class R, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9> typename reg<N, sig<R (__thiscall*)(A1, A2, A3, A4, A5, A6, A7, A8, A9), R (__fastcall*)(A1, int, A2, A3, A4, A5, A6, A7, A8, A9)> >::sized enc(R (__fastcall*)(A1, int, A2, A3, A4, A5, A6, A7, A8, A9));
template<int N, class R, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10> typename reg<N, sig<R (__thiscall*)(A1, A2, A3, A4, A5, A6, A7, A8, A9, A10), R (__fastcall*)(A1, int, A2, A3, A4, A5, A6, A7, A8, A9, A10)> >::sized enc(R (__fastcall*)(A1, int, A2, A3, A4, A5, A6, A7, A8, A9, A10));
template<int N, class R, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10, class A11> typename reg<N, sig<R (__thiscall*)(A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11), R (__fastcall*)(A1, int, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11)> >::sized enc(R (__fastcall*)(A1, int, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11));
template<int N, class R, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10, class A11, class A12> typename reg<N, sig<R (__thiscall*)(A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12), R (__fastcall*)(A1, int, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12)> >::sized enc(R (__fastcall*)(A1, int, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12));
}
#define vc8_thiscall(f) vc8_thiscall_n_(f, __COUNTER__)
#define vc8_thiscall_n_(f, n) vc8_thiscall_(f, (n) + 1)
#define vc8_thiscall_(f, n) (sizeof(*::vc8_tc::enc<n>(f)), sizeof(::vc8_tc::same< ::vc8_tc::get<n>::fc>(f)), \
    (::vc8_tc::get<n>::tc)(f))
#endif

// GCC's spelling of MSVC's _alloca intrinsic. malloc.h is included last, so
// that every rename above is in force before any CRT header is read.
#include <malloc.h>
#define __builtin_alloca _alloca
