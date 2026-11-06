/**
 * Data defined in D2Client/UI/Automap.cpp that other translation units name.
 * Binary: Game.exe
 */

#pragma once

#include "d2_platform.h"

// Forward declarations
struct DC6;

namespace D2Client::UI::Automap {
extern int nAutoMapCenters;
extern int nAutoMapParty;
extern uint32_t AutoMapPartyNames;
extern uint32_t MiniMapType;
extern void * pClientAutomapCellActual;
extern size_t nAutomapCellListSize;
extern DC6 * pAutoMapDC6[4];
extern int nAutoMapDC6FrameCount[4];
extern int AutoMapMoveX;
extern int AutoMapMoveY;
extern int nAutoMapFade;
extern BOOL bAutoMapFade;
extern int AUTOMAP_TextHeightOffset;
extern int gnAutomapClipLeft;
extern int gnAutomapClipTop;
extern int gnAutomapClipRight;
extern int gnAutomapClipBottom;
extern int nAutomapCenterY;
extern int gnAutomapCenterX;
extern int AutoMapLeft;
extern uint32_t AutoMapLeft2;
extern uint32_t AutoMapLeft3;
} // namespace D2Client::UI::Automap
