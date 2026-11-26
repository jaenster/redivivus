#include "Redivivus/d2_trace.h"
// Hand-written CRT / intrinsic support layer.
//
// Every definition here stands in for a function that IS in Game.exe but is
// dropped before emission by `excludePatterns` (`/^compiler(::.*)?$/`,
// `/^_Wrappers$/` in ghidra-reconstruct/run.ts) and that the target's own C
// runtime does not supply under the name the tree calls. A libcmt function the
// tree calls by its real name binds to that runtime instead and has no entry here. The generator never writes this file, so it
// carries no merge risk and no debt (regen-workflow.md, option 3).
//
// Rules obeyed here, from docs/regen-workflow.md:
//   - No fabricated ABI. Every body below is backed by the disassembly or the
//     decompilation of the named address in
//     Diablo2Lod /windows/lod/1.14d/Game.exe, quoted in Redivivus/README.md.
//   - Where the ABI could not be honestly reproduced at the emitted call site,
//     the symbol is LEFT UNDEFINED rather than stubbed. See README.md.
//   - No shim exists only to make a count fall.
//
// Symbols deliberately NOT defined here (see README.md for each):
//   _Wrappers::CRT_ExceptionFilter1            - real game code (a C++ EH frame
//                                                handler), excluded only by
//                                                namespace
//
// The x87 math helpers (_CIpow ... _CIsqrt, floor) and ATL's AtlThrow need no
// entry: the generator spells their calls as the source function (pow, floor,
// AtlThrow), which the toolchain or d2_platform.h supplies.

#include <cmath>
#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>

#ifndef _MSC_VER
// ---------------------------------------------------------------------------
// Functions whose generated prototype is already concrete.
// ---------------------------------------------------------------------------

// 00684af3 is libcmt's _set_se_translator (libmap lib-exact). The tree calls it
// by that name; Visual C++ links libcmt's own. mingw's msvcrt import library
// exports no _set_se_translator, and GCC code never reaches an MSVC translator
// anyway, so here the platform header's `_set_se_translator` lands on this slot:
// the get/set/return-previous semantics and the per-thread storage, without the
// dispatch, which needs the MSVC SEH layer.
static thread_local void* g_pSeTranslator = nullptr;
extern "C" void* D2_SetSeTranslator(void* pTranslator) {
  void* pPrevious = g_pSeTranslator;
  g_pSeTranslator = pTranslator;
  return pPrevious;
}

// ---------------------------------------------------------------------------
// SEH and /GS.
// ---------------------------------------------------------------------------

// 00684f50, libcmt's _except_handler4 (libmap lib-exact). This is the MSVC SEH
// frame handler; mingw-GCC does not provide it, and msvcrt.dll exports only
// _except_handler4_common. There is no honest reimplementation - it walks an
// MSVC scope table this tree does not build.
//
// It is referenced ONLY by address, never called: the EH4 frames the generator
// cannot yet spell as D2_SEH_TRY (their __except blocks are funclets the decompile
// lacks: Fog/Engine/Application.cpp, Fog/Src/LogManager.cpp) only store it into a
// local SEH registration record, and the emitted code never links that record
// onto fs:[0]. The link needs the address; nothing
// reaches the body.
//
// STATED PLAINLY: this is a stub, not a handler. Any path that ever calls it
// dies at the abort() below. It does not dispatch, unwind, or filter.
extern "C" int _except_handler4(struct _EXCEPTION_RECORD* pRecord, EH4_EXCEPTION_REGISTRATION* pRegistration,
                                 CONTEXT* pContext, void* pDispatcherContext) {
  (void)pRecord;
  (void)pRegistration;
  (void)pContext;
  (void)pDispatcherContext;
  D2Trace("ABORT called");
    abort();
}
#endif  // !_MSC_VER

#if defined(_MSC_VER) && defined(_M_X64)
// The same stub for a Visual Studio x64 build: x64 SEH is table-driven and its CRT has
// no _except_handler4 to link, while the x86 libcmt has the real one. Referenced by
// address only, as above; any call dies at the abort().
extern "C" int _except_handler4(struct _EXCEPTION_RECORD* pRecord, EH4_EXCEPTION_REGISTRATION* pRegistration,
                                 CONTEXT* pContext, void* pDispatcherContext) {
  (void)pRecord;
  (void)pRegistration;
  (void)pContext;
  (void)pDispatcherContext;
  D2Trace("ABORT called");
  abort();
}
#endif
