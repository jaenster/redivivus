/**
 * Data defined in Fog/Debug.cpp that other translation units name.
 * Binary: Game.exe
 */

#pragma once
#if defined(D2_MODULE_LIBRARIES) && defined(D2_MODULE_STRICT) && !defined(D2_GATE_FROM_ROOT) && !defined(D2_SEES_Fog)
#error "Fog header included by a module that may not use Fog (original DLL import graph; see module-gates.tsv)"
#endif
#if defined(D2_MODULE_LIBRARIES) && defined(D2_MODULE_STRICT) && !defined(D2_GATE_FROM_ROOT) && !defined(D2_MODULE_Fog) && !defined(D2_LOADS_Fog)
#error "internal Fog header included from another module; include Fog's public header (see module-gates.tsv)"
#endif

#include "Source/D2Headers/d2_platform.h"

// Forward declarations
struct D2CrashDumpRequestStrc;
typedef BOOL (__stdcall *pfnSymCleanup)(HANDLE hProcess);
#if !(defined(_WIN64) || defined(__LP64__))
typedef BOOL (__stdcall *pfnSymGetLineFromAddr)(HANDLE hProcess, DWORD dwAddr, void * pdwDisplacement, void * pLine);
#else
// 64-bit dbghelp exports only the *64 entry points (DWORD64 addresses).
typedef BOOL (__stdcall *pfnSymGetLineFromAddr)(HANDLE hProcess, DWORD64 dwAddr, void * pdwDisplacement, void * pLine);
#endif
#if !(defined(_WIN64) || defined(__LP64__))
typedef BOOL (__stdcall *pfnSymGetModuleInfo)(HANDLE hProcess, DWORD dwAddr, void * pModuleInfo);
#else
// 64-bit dbghelp exports only the *64 entry points (DWORD64 addresses).
typedef BOOL (__stdcall *pfnSymGetModuleInfo)(HANDLE hProcess, DWORD64 dwAddr, void * pModuleInfo);
#endif
#if !(defined(_WIN64) || defined(__LP64__))
typedef BOOL (__stdcall *pfnSymGetSymFromAddr)(HANDLE hProcess, DWORD_PTR dwAddr, void * pDisplacement, void * pSymbol);
#else
// 64-bit dbghelp exports only the *64 entry points (DWORD64 addresses).
typedef BOOL (__stdcall *pfnSymGetSymFromAddr)(HANDLE hProcess, DWORD64 dwAddr, void * pDisplacement, void * pSymbol);
#endif
typedef BOOL (__stdcall *pfnSymInitialize)(HANDLE hProcess, char * UserSearchPath, BOOL fInvadeProcess);
#if !(defined(_WIN64) || defined(__LP64__))
typedef DWORD (__stdcall *pfnSymGetModuleBase)(HANDLE hProcess, DWORD dwAddr);
#else
// 64-bit dbghelp exports only the *64 entry points (DWORD64 addresses).
typedef DWORD64 (__stdcall *pfnSymGetModuleBase)(HANDLE hProcess, DWORD64 dwAddr);
#endif
typedef DWORD (__stdcall *pfnSymGetOptions)();
#if !(defined(_WIN64) || defined(__LP64__))
typedef DWORD (__stdcall *pfnSymLoadModule)(HANDLE hProcess, HANDLE hFile, char * ImageName, char * ModuleName, DWORD_PTR BaseOfDll, DWORD SizeOfDll);
#else
// 64-bit dbghelp exports only the *64 entry points (DWORD64 addresses).
typedef DWORD64 (__stdcall *pfnSymLoadModule)(HANDLE hProcess, HANDLE hFile, char * ImageName, char * ModuleName, DWORD64 BaseOfDll, DWORD SizeOfDll);
#endif
typedef DWORD (__stdcall *pfnSymSetOptions)(DWORD SymOptions);
#if !(defined(_WIN64) || defined(__LP64__))
typedef void * (__stdcall *pfnSymFunctionTableAccess)(HANDLE hProcess, DWORD AddrBase);
#else
// 64-bit dbghelp exports only the *64 entry points (DWORD64 addresses).
typedef void * (__stdcall *pfnSymFunctionTableAccess)(HANDLE hProcess, DWORD64 AddrBase);
#endif
typedef BOOL (__stdcall *pfnStackWalk)(DWORD MachineType, HANDLE hProcess, HANDLE hThread, void * StackFrame, void * ContextRecord, void * ReadMemoryRoutine, pfnSymFunctionTableAccess FunctionTableAccessRoutine, pfnSymGetModuleBase GetModuleBaseRoutine, void * TranslateAddress);

extern D2CrashDumpRequestStrc * pCrashDumpInfo;
// lpCriticalSection parameter of InitializeCriticalSection
// lpCriticalSection parameter of LeaveCriticalSection
// lpCriticalSection parameter of EnterCriticalSection
extern _RTL_CRITICAL_SECTION lpCriticalSection_0098a6f0;
extern char szCustomSymbolSearchPath[32768];
extern char gszCrashLogTempDir[1024];

namespace Fog::Debug {
extern HMODULE pDbgHelpOrImageHlp;
extern pfnSymCleanup pSymCleanup;
extern pfnSymFunctionTableAccess pSymFunctionTableAccess;
extern pfnSymGetLineFromAddr pSymGetLineFromAddr;
extern pfnSymGetModuleBase pSymGetModuleBase;
extern pfnSymGetModuleInfo pSymGetModuleInfo;
extern pfnSymGetOptions pSymGetOptions;
extern pfnSymGetSymFromAddr pSymGetSymFromAddr;
extern pfnSymInitialize pSymInitialize;
extern pfnSymLoadModule pSymLoadModule;
extern pfnSymSetOptions pSymSetOptions;
extern pfnStackWalk pStackWalk;
extern BOOL bBetterStackCrawls;
} // namespace Fog::Debug
