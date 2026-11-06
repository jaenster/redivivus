#include "support/d2_trace.h"
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
//     Diablo2Lod /windows/lod/1.14d/Game.exe, quoted in support/README.md.
//   - Where the ABI could not be honestly reproduced at the emitted call site,
//     the symbol is LEFT UNDEFINED rather than stubbed. See README.md.
//   - No shim exists only to make a count fall.
//
// Symbols deliberately NOT defined here (see README.md for each):
//   __ftol2, __CIsqrt                          - operand lives in ST(0) and the
//                                                emitted call sites pass nothing
//   _Wrappers::CRT_StrLen, _Wrappers::gCmdLineHelpText
//                                              - string DATA typed as FUNCTION
//   _Wrappers::CRT_ExceptionFilter1/2/3, _Wrappers::CRT_SecurityCookieStub2
//   compiler::PKWARE_explode/implode, compiler::CRT_GetItemDataAndFlags
//                                              - real game code, excluded only
//                                                by namespace

#include <cmath>
#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>

// ---------------------------------------------------------------------------
// The five MSVC x87 __CI* intrinsics.
//
// Their operands travel in the x87 stack registers, NOT on the C stack:
//   006886c0 CRT_CICos    __CIcos    ST(0) -> ST(0)   FCOS at 0068872d
//   00688590 CRT_CISin    __CIsin    ST(0) -> ST(0)
//   00687ac0 CRT_CILog    __CIlog    ST(0) -> ST(0)   FLDLN2/FXCH/FYL2X @00687b3f
//   00687ea0 CRT_CILog10  __CIlog10  ST(0) -> ST(0)   FLDLG2 form
//   00687c10 CRT_CIPow    __CIpow    ST(1),ST(0) -> ST(0)  FYL2X+F2XM1 @00687c6d
//
// Aliasing these to cos/sin/log/log10/pow AT THE BINARY'S ABI would read
// garbage. That is not what happens here: Ghidra (v791) models the operands as
// parameters with ST0/ST1 custom storage, so the decompiler threads the values
// through and the emitted call sites are ordinary C calls carrying the operand
// as a `long double` argument - e.g. D2Common/Drlg/Environment.cpp:83
// `fFloatResult = CRT_CICos(fVar1);`. The definitions below therefore match the
// prototypes the call sites were compiled against, and the x87 stack never
// enters the picture. mingw's `long double` is the same 80-bit x87 type as
// Ghidra's `float10`.
//
// CRT_CIPow's parameter order is load-bearing: Ghidra records fBase in ST(1)
// and fExponent in ST(0), fixed from the FXCH at 00687c4e. powl(base, exp).
// ---------------------------------------------------------------------------
extern "C" long double CRT_CICos(long double x) { return cosl(x); }
extern "C" long double CRT_CISin(long double x) { return sinl(x); }
extern "C" long double CRT_CILog(long double x) { return logl(x); }
extern "C" long double CRT_CILog10(long double x) { return log10l(x); }
extern "C" long double CRT_CIPow(long double base, long double exponent) {
  return powl(base, exponent);
}

#ifndef _MSC_VER
// ---------------------------------------------------------------------------
// A CRT function whose generated prototype in d2_platform.h is the
// emitter's unknown-signature fallback `T NAME(...)`. A variadic declaration
// with no named parameter cannot be defined portably, and re-declaring the name
// with a concrete signature in this TU would collide with the forced include.
// Both are avoided by giving the real definition a private C++ name and an asm
// label carrying the exact object-file symbol (i386 PE user-label prefix `_`).
// On i386 cdecl a fixed-arity definition is call-compatible with the variadic
// declaration: all arguments go on the stack and the caller cleans up.
// ---------------------------------------------------------------------------

// 00683080 `float10 CRT_Floor(double fParam)` __cdecl. Its x87 fallback jumps to
// 0068ea71, which Ghidra's FID matches as __floor_default (VS2005 Release,
// single match); the SSE2 path at 006830c0 is the branch-free exponent-field
// floor. Its only caller, Fog/Source/SFile.cpp:614-615 (SFILE_JulianDayNumber),
// is a floor consumer. Both call sites discard the result.
extern "C" double d2_CRT_Floor(double x) __asm__("_CRT_Floor");
extern "C" double d2_CRT_Floor(double x) { return floor(x); }

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

// 006b1a30 - NOT a return-value helper. __stdcall, does not return, one
// statement: `__CxxThrowException_8(&dwParam, &DAT_007020b4)`. The ThrowInfo at
// 007020b4 names one catchable type at 007020cc with sizeOrOffset 4 and a
// TypeDescriptor at 0074c0a8 whose decorated name is
// `.?AVCAtlException@ATL@@`. So the function throws a 4-byte ATL::CAtlException
// carrying the HRESULT: it is ATL's AtlThrow. The only call site,
// D2Direct3D/Renderer/Direct3D.cpp:596 `CRT_ReturnValue(0x80004005)` guarded by
// a null test, is the ATLENSURE shape.
//
// mingw has no ATL, so the exception type is declared here to the shape the
// type descriptor gives - a 4-byte object holding the HRESULT - and thrown.
// Nothing in the tree catches it (the MSVC EH frames are not emitted), so in
// practice this terminates. That matches the binary's own "does not return".
namespace ATL {
struct CAtlException {
  long m_hr;
};
}  // namespace ATL

extern "C" unsigned int CRT_ReturnValue(unsigned int dwHResult) {
  throw ATL::CAtlException{static_cast<long>(dwHResult)};
}

// ---------------------------------------------------------------------------
// SEH and /GS.
// ---------------------------------------------------------------------------

// 00684f50, libcmt's _except_handler4 (libmap lib-exact). This is the MSVC SEH
// frame handler; mingw-GCC does not provide it, and msvcrt.dll exports only
// _except_handler4_common. There is no honest reimplementation - it walks an
// MSVC scope table this tree does not build.
//
// It is referenced ONLY by address, never called: Fog/Debug.cpp:191,
// Fog/Engine/Application.cpp:1516 and :1642, Fog/Src/LogManager.cpp:1880 all
// only store it into a local SEH registration record, and the emitted code
// never links that record onto fs:[0]. The link needs the address; nothing
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
#else  // _MSC_VER
// ---------------------------------------------------------------------------
// The Visual C++ 2005 build links the libcmt the original linked. Where a
// CRT_ name above is that CRT function itself, the symbol resolves to libcmt's
// own object: each alternatename applies only while the name is otherwise
// undefined, and each target is call-compatible with the tree's declaration.
// The runtime functions the tree calls by their libcmt names need none.
// ---------------------------------------------------------------------------

// 00683080: floor, SSE2 path and __floor_default fallback, as libcmt's floor.
#pragma comment(linker, "/alternatename:_CRT_Floor=_floor")

// 006b1a30: ATL's AtlThrow, which throws an ATL::CAtlException holding the
// HRESULT. The toolchain has no ATL; the class is declared to the shape the
// original's type descriptor gives (.?AVCAtlException@ATL@@, 4 bytes).
namespace ATL {
class CAtlException {
 public:
  long m_hr;
};
}  // namespace ATL

extern "C" unsigned int CRT_ReturnValue(unsigned int dwHResult) {
  ATL::CAtlException tException;
  tException.m_hr = static_cast<long>(dwHResult);
  throw tException;
}
#endif  // _MSC_VER
