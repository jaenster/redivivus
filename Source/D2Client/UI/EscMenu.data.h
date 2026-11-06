/**
 * Data defined in D2Client/UI/EscMenu.cpp that other translation units name.
 * Binary: Game.exe
 */

#pragma once

#include "d2_platform.h"
#include "D2Client/UI/EscMenu.h"

// Forward declarations
struct D2EscMenuUIFlagStrc;

// 38 records of {nEnabled,nHideable,nSavedState}. Split into three labels, so opening the ESC menu wrote 456 bytes past a 4-byte object and read its per-flag config from garbage.
extern D2EscMenuUIFlagStrc gaEscMenuUIFlags[38];
