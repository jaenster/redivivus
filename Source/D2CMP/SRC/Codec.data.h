/**
 * Data defined in D2CMP/SRC/Codec.cpp that other translation units name.
 * Binary: Game.exe
 */

#pragma once
#if defined(D2_MODULE_LIBRARIES) && defined(D2_MODULE_STRICT) && !defined(D2_GATE_FROM_ROOT) && !defined(D2_SEES_D2CMP)
#error "D2CMP header included by a module that may not use D2CMP (original DLL import graph; see module-gates.tsv)"
#endif

#include "Source/D2Headers/d2_platform.h"
#include "Source/D2CMP/SRC/Codec.h"

// Forward declarations
struct D2CodecPixelBufferEntryStrc;

extern uint8_t gaCodecFrameWorkBuffer2[262144];
extern D2CodecPixelBufferEntryStrc gaCodecFrameDecodeBuffer[5625];
