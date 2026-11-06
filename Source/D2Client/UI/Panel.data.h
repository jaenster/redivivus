/**
 * Data defined in D2Client/UI/Panel.cpp that other translation units name.
 * Binary: Game.exe
 */

#pragma once
#if defined(D2_MODULE_LIBRARIES) && defined(D2_MODULE_STRICT) && !defined(D2_GATE_FROM_ROOT) && !defined(D2_SEES_D2Client)
#error "D2Client header included by a module that may not use D2Client (original DLL import graph; see module-gates.tsv)"
#endif

#include "Source/D2Headers/d2_platform.h"

// Forward declarations
struct DC6;

namespace D2Client::UI::Panel {
extern DC6 * runbutton;
extern DC6 * ctrlpnl_popbelt;
extern DC6 * GemSocket;
extern DC6 * _00BorderFrame;
extern DC6 * CtrlPnl7;
extern DC6 * skillpoints;
extern DC6 * Invwarn;
extern DC6 * hostilepic;
extern DC6 * menupanel;
extern DC6 * DuroIcon;
extern DC6 * InvChar;
extern DC6 * Hlthmana;
extern DC6 * menubutton;
extern DC6 * _00CtrlPnl7;
extern DC6 * Arrow;
extern DC6 * btn;
extern DC6 * overlap;
extern BOOL bShowHpText;
extern BOOL bShowmpText;
} // namespace D2Client::UI::Panel
