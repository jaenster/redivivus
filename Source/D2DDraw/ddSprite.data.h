/**
 * Data defined in D2DDraw/ddSprite.cpp that other translation units name.
 * Binary: Game.exe
 */

#pragma once
#if defined(D2_MODULE_LIBRARIES) && defined(D2_MODULE_STRICT) && !defined(D2_GATE_FROM_ROOT) && !defined(D2_SEES_D2DDraw)
#error "D2DDraw header included by a module that may not use D2DDraw (original DLL import graph; see module-gates.tsv)"
#endif
#if defined(D2_MODULE_LIBRARIES) && defined(D2_MODULE_STRICT) && !defined(D2_GATE_FROM_ROOT) && !defined(D2_MODULE_D2DDraw) && !defined(D2_LOADS_D2DDraw)
#error "internal D2DDraw header included from another module; include D2DDraw's public header (see module-gates.tsv)"
#endif

#include "Source/D2Headers/d2_platform.h"

extern uint32_t gnDDrawSurfaceXMin;
extern uint32_t gnDDrawRenderWidth;
