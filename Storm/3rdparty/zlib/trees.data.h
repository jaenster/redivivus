/**
 * Data defined in Storm/3rdparty/zlib/trees.cpp that other translation units name.
 * Binary: Game.exe
 */

#pragma once

#include "d2_platform.h"
#include "globals/globals.h"
#include "Storm/3rdparty/zlib/trees.h"

// Forward declarations
struct ct_data;

extern D2ZlibExtraLBitsBlOrderStrc gZlibExtraLBitsBlOrder;
extern int gaZlibExtraDBits[30];
extern int gaZlibExtraBlBits[19];
extern ct_data gaZlibStaticLTree[288];
extern ct_data gZlibStaticDTreeDistCode[30];
extern uint8_t _dist_code[512];
extern uint8_t gaZlibLengthCodeTable[256];
