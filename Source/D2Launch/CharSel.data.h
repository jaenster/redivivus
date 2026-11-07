/**
 * Data defined in D2Launch/CharSel.cpp that other translation units name.
 * Binary: Game.exe
 */

#pragma once
#if defined(D2_MODULE_LIBRARIES) && defined(D2_MODULE_STRICT) && !defined(D2_GATE_FROM_ROOT) && !defined(D2_SEES_D2Launch)
#error "D2Launch header included by a module that may not use D2Launch (original DLL import graph; see module-gates.tsv)"
#endif
#if defined(D2_MODULE_LIBRARIES) && defined(D2_MODULE_STRICT) && !defined(D2_GATE_FROM_ROOT) && !defined(D2_MODULE_D2Launch) && !defined(D2_LOADS_D2Launch)
#error "internal D2Launch header included from another module; include D2Launch's public header (see module-gates.tsv)"
#endif

#include "Source/D2Headers/d2_platform.h"

// Forward declarations
struct D2WinButton;

extern char PreferredClosedRealm[256];
extern uint32_t gnCharSelSelectedIndex;
extern ::D2WinButton * gpCharSelScreenControl;
extern char PreferedClosedRealm2[256];
extern uint32_t gbCharSelectInProgress;
extern int nCharacterSelectionIndex;
extern uint32_t gnCharSelLoadedCharCount;
extern uint32_t gnCharSelCreateNewButtonClickCount;
