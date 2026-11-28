// Target of this build, for the hand-maintained code that has to differ between
// targets (register names in a CONTEXT, the SEH frame handler, pointer-width
// shims, the Mac-only Storm layer). Generated code never tests these: it is one
// source for every target.
//
// Operating system, exactly one defined, to 1:
//   D2_OS_WIN     Windows (mingw, VC8)
//   D2_OS_MAC     Mac OS X (Apple clang; 1.14d Mac shipped i386 built with clang-425)
//
// The Mac's layer, beside D2_OS_MAC (Source/StormMac/README.md), from the toolchain:
//   D2_MAC_CARBON     the 1.14d build's Carbon, Win32Mac and StormMac layer (cmake/mac.cmake)
//   D2_BACKEND_POSIX  the Win32Posix layer
//
// Architecture, exactly one defined, to 1:
//   D2_ARCH_X86   32-bit x86: the 1.14d Game.exe target (mingw i686, VC8) and the
//                 1.14d Intel Mac build
//   D2_ARCH_X64   64-bit x86-64 (mingw x86_64, the modern Intel Mac build)
//   D2_ARCH_ARM64 64-bit ARM (the modern Apple silicon Mac build); in hand-maintained code
//                 it takes the x86-64 branch wherever that branch is about pointer width
//   D2_ARCH_PPC   32-bit PowerPC: the Mac builds before the Intel move
//
// Byte order: D2_BIG_ENDIAN is defined, to 1, on PowerPC only. The file, wire and
// archive formats are little-endian on every target, so big-endian code swaps.
//
// Any other target is a hard error: nothing in the tree has been checked against it.
//
// Shared headers do not include this file: a new macro in a header many VC8 TUs
// see shifts symbol numbering in unrelated functions. A shared header tests the
// compiler's own macros (_WIN64, __APPLE__) directly; a .cpp may include this.
#pragma once

#if defined(_WIN32)
#define D2_OS_WIN 1
#elif defined(__APPLE__) && defined(__MACH__)
#define D2_OS_MAC 1
#else
#error "D2Arch.h: unsupported target operating system (expected Windows or Mac OS X)"
#endif

#if defined(_M_IX86) || defined(__i386__)
#define D2_ARCH_X86 1
#elif defined(_M_X64) || defined(__x86_64__)
#define D2_ARCH_X64 1
#elif (defined(__ppc__) || defined(__POWERPC__) || defined(_M_PPC)) && !defined(__ppc64__)
#define D2_ARCH_PPC 1
#define D2_BIG_ENDIAN 1
#elif defined(__aarch64__) || defined(__arm64__)
#define D2_ARCH_ARM64 1
#else
#error "D2Arch.h: unsupported target architecture (expected x86, x86-64, ARM64 or PowerPC)"
#endif
