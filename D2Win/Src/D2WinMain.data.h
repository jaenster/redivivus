/**
 * Data defined in D2Win/Src/D2WinMain.cpp that other translation units name.
 * Binary: Game.exe
 */

#pragma once

#include "d2_platform.h"
#include "globals/globals.h"

// Forward declarations
struct D2ControlStrc;
struct D2UIMouseMsgStrc;
struct D2WinEditBox;
typedef LRESULT (__stdcall *WndProcB)(HWND param_1, uint32_t uMsg, uint32_t param_3, LPARAM param_4);
typedef void (__stdcall *D2WinWndHandlerFn)(D2UIMouseMsgStrc * pMsg);
typedef void (__stdcall *pfnVoidNoArgCallback)();

extern D2WinWndHandlerStrc gaUIWindowHandlers[33];
extern uint32_t gnCursorDC6FrameNumber;
extern HWND ghwndD2GFX_CachedWindow;
extern uint32_t gnWindowInitState;
extern BOOL gbWindowed;
extern int gnResolutionMode;
extern int gnFormControlCount;
extern uint32_t gbGfxWindowCreated;
extern uint32_t gnInitialFormState;
extern pfnVoidNoArgCallback gpfnExitCallback;

namespace D2Client::Forms {
extern D2ControlStrc * pFormUnknown;
extern D2ControlStrc * pFormSecond;
extern D2WinEditBox * pFormTextAndEditBox;
extern D2ControlStrc * pFormFourth;
extern D2ControlStrc * pFormImage;
} // namespace D2Client::Forms

namespace D2Client::GAME::Record {
extern WndProcB pWndProcB;
} // namespace D2Client::GAME::Record
