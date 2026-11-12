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
typedef BOOL (__stdcall *pfnSymCleanup)(HANDLE hProcess);
typedef BOOL (__stdcall *pfnSymGetLineFromAddr)(HANDLE hProcess, DWORD dwAddr, void * pdwDisplacement, void * pLine);
typedef BOOL (__stdcall *pfnSymGetModuleInfo)(HANDLE hProcess, DWORD dwAddr, void * pModuleInfo);
typedef BOOL (__stdcall *pfnSymGetSymFromAddr)(HANDLE hProcess, DWORD dwAddr, void * pDisplacement, void * pSymbol);
typedef BOOL (__stdcall *pfnSymInitialize)(HANDLE hProcess, char * UserSearchPath, BOOL fInvadeProcess);
typedef DWORD (__stdcall *pfnSymGetModuleBase)(HANDLE hProcess, DWORD dwAddr);
typedef DWORD (__stdcall *pfnSymGetOptions)();
typedef DWORD (__stdcall *pfnSymLoadModule)(HANDLE hProcess, HANDLE hFile, char * ImageName, char * ModuleName, DWORD BaseOfDll, DWORD SizeOfDll);
typedef DWORD (__stdcall *pfnSymSetOptions)(DWORD SymOptions);
typedef void * (__stdcall *pfnSymFunctionTableAccess)(HANDLE hProcess, DWORD AddrBase);
typedef BOOL (__stdcall *pfnStackWalk)(DWORD MachineType, HANDLE hProcess, HANDLE hThread, void * StackFrame, void * ContextRecord, void * ReadMemoryRoutine, pfnSymFunctionTableAccess FunctionTableAccessRoutine, pfnSymGetModuleBase GetModuleBaseRoutine, void * TranslateAddress);

// lpCriticalSection parameter of InitializeCriticalSection
// lpCriticalSection parameter of LeaveCriticalSection
// lpCriticalSection parameter of EnterCriticalSection
extern _RTL_CRITICAL_SECTION lpCriticalSection_0098a6f0;
extern char szCustomSymbolSearchPath[32768];
extern char gszCrashLogTempDir[1024];

namespace Fog { namespace Debug {
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
}} // namespace Fog::Debug
