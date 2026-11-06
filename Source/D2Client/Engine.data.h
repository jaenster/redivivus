/**
 * Data defined in D2Client/Engine.cpp that other translation units name.
 * Binary: Game.exe
 */

#pragma once
#if defined(D2_MODULE_LIBRARIES) && defined(D2_MODULE_STRICT) && !defined(D2_GATE_FROM_ROOT) && !defined(D2_SEES_D2Client)
#error "D2Client header included by a module that may not use D2Client (original DLL import graph; see module-gates.tsv)"
#endif

#include "Source/D2Headers/d2_platform.h"
#include "Source/D2Client/UI/ui.h"

// Forward declarations
struct D2ArchiveStrc;
struct D2UIFlagInitStrc;
struct D2UIMouseMsgStrc;
typedef void (__stdcall *fnD2InitUIFlagFunction)(D2UIMouseMsgStrc * param_1);

extern D2UIFlagInitStrc gsUIEngineFlagInit[4];
extern D2UIFlagInitStrc gsUIEngineFlagInit2[4];
// @data IMG_popupok_DiscPrompt
// @wasnamed IMG_popupok
// @evidence Loaded at 004606b4 and drawn by DrawPopupOk(1) from
// LOADER_DrawInsertDiscScreen / fpGameLoopFunction.
// @caveat both this and 007a5244 are loaded from the IDENTICAL path Data\Global\UI\Bigmenu\popupok
// with identical flags (004606a8 and 004606b9, ECX=0x6d6a14 both times). They are two independent
// handles to the same asset; only the consumer distinguishes them, and that is what these names
// record. Nothing in the binary makes them semantically different.
extern D2ArchiveStrc * IMG_popupok_DiscPrompt;
extern D2ArchiveStrc * IMG_okcancelbtn;
// @data IMG_popupok_ConnLost
// @wasnamed IMG_popupok
// @evidence Loaded at 004606c5 and drawn by DrawPopupOk(0) from
// LOADER_DrawConnectionLostScreen. See the caveat on 007a523c.
extern D2ArchiveStrc * IMG_popupok_ConnLost;
