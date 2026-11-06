/**
 * Data defined in D2Win/Src/D2WinMain.cpp that other translation units name.
 * Binary: Game.exe
 */

#pragma once

#include "d2_platform.h"
#include "globals/globals.h"

// Forward declarations
struct D2UIMouseMsgStrc;
typedef void (__stdcall *D2WinWndHandlerFn)(D2UIMouseMsgStrc * pMsg);
typedef void (__stdcall *pfnVoidNoArgCallback)();

extern D2WinWndHandlerStrc gaUIWindowHandlers[33];
extern uint32_t gnInitialFormState;
extern pfnVoidNoArgCallback gpfnExitCallback;
