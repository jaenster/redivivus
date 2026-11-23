// Target architecture of this build, for the hand-maintained code that has to
// differ between the 32-bit and the 64-bit executable (register names in a
// CONTEXT, the SEH frame handler, pointer-width shims). Generated code never
// tests these: it is one source for both targets.
//
//   D2_ARCH_X86   32-bit x86, the 1.14d Game.exe target (mingw i686, VC8)
//   D2_ARCH_X64   64-bit x86-64 (mingw x86_64)
//
// Exactly one is defined, to 1. Any other target is a hard error: nothing in the
// tree has been checked against it.
#pragma once

#if defined(_M_IX86) || defined(__i386__)
#define D2_ARCH_X86 1
#elif defined(_M_X64) || defined(__x86_64__)
#define D2_ARCH_X64 1
#else
#error "D2Arch.h: unsupported target architecture (expected x86 or x86-64)"
#endif
