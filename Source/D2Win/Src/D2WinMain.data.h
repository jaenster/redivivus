/**
 * Data defined in D2Win/Src/D2WinMain.cpp that other translation units name.
 * Binary: Game.exe
 */

#pragma once
#if defined(D2_MODULE_LIBRARIES) && defined(D2_MODULE_STRICT) && !defined(D2_GATE_FROM_ROOT) && !defined(D2_SEES_D2Win)
#error "D2Win header included by a module that may not use D2Win (original DLL import graph; see module-gates.tsv)"
#endif

#include "Source/D2Headers/d2_platform.h"
#include "Source/globals/globals.h"

// Forward declarations
struct D2UIMouseMsgStrc;
typedef void (__stdcall *D2WinWndHandlerFn)(D2UIMouseMsgStrc * pMsg);
typedef void (__stdcall *pfnVoidNoArgCallback)();

extern D2WinWndHandlerStrc gaUIWindowHandlers[33];
extern uint32_t gnInitialFormState;
extern pfnVoidNoArgCallback gpfnExitCallback;
