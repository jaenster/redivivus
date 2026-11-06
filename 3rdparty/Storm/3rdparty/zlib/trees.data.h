/**
 * Data defined in Storm/3rdparty/zlib/trees.cpp that other translation units name.
 * Binary: Game.exe
 */

#pragma once
#if defined(D2_MODULE_LIBRARIES) && defined(D2_MODULE_STRICT) && !defined(D2_GATE_FROM_ROOT) && !defined(D2_SEES_Storm)
#error "Storm header included by a module that may not use Storm (original DLL import graph; see module-gates.tsv)"
#endif

#include "Source/D2Headers/d2_platform.h"
#include "Source/globals/globals.h"
#include "Source/D2Hell/D2Hell.h"
#include "3rdparty/Storm/3rdparty/zlib/deflate.h"
#include "3rdparty/Storm/H/stpl.h"
#include "3rdparty/Storm/Source/SCMD.h"
#include "3rdparty/Storm/Source/SComp.h"
#include "3rdparty/Storm/Source/SErr.h"
#include "3rdparty/Storm/Source/SMem.h"
#include "3rdparty/Storm/Storm.h"
#include "Source/D2Headers/d2_platform.h"
#include "3rdparty/Storm/3rdparty/zlib/trees.h"

// Forward declarations
struct ct_data;

extern D2ZlibExtraLBitsBlOrderStrc gZlibExtraLBitsBlOrder;
extern int gaZlibExtraDBits[30];
extern int gaZlibExtraBlBits[19];
extern ct_data gaZlibStaticLTree[288];
extern ct_data gZlibStaticDTreeDistCode[30];
extern uint8_t _dist_code[512];
extern uint8_t gaZlibLengthCodeTable[256];
