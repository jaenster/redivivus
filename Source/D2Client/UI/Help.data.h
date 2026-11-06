/**
 * Data defined in D2Client/UI/Help.cpp that other translation units name.
 * Binary: Game.exe
 */

#pragma once
#if defined(D2_MODULE_LIBRARIES) && defined(D2_MODULE_STRICT) && !defined(D2_GATE_FROM_ROOT) && !defined(D2_SEES_D2Client)
#error "D2Client header included by a module that may not use D2Client (original DLL import graph; see module-gates.tsv)"
#endif

#include "Source/D2Headers/d2_platform.h"

// Forward declarations
struct DC6;

extern RGBQUAD nColor;

namespace D2Client::UI::Help {
extern BOOL bSeenHelpMenu;
extern DC6 * pHelpBorder;
extern DC6 * HelpWhiteBullet;
extern DC6 * HelpYellowBullet;
extern int32_t gbMiniPanel;
} // namespace D2Client::UI::Help
