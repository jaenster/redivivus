/**
 * Data defined in D2CMP/SRC/SpriteCache.cpp that other translation units name.
 * Binary: Game.exe
 */

#pragma once
#if defined(D2_MODULE_LIBRARIES) && defined(D2_MODULE_STRICT) && !defined(D2_GATE_FROM_ROOT) && !defined(D2_SEES_D2CMP)
#error "D2CMP header included by a module that may not use D2CMP (original DLL import graph; see module-gates.tsv)"
#endif
#if defined(D2_MODULE_LIBRARIES) && defined(D2_MODULE_STRICT) && !defined(D2_GATE_FROM_ROOT) && !defined(D2_MODULE_D2CMP) && !defined(D2_LOADS_D2CMP)
#error "internal D2CMP header included from another module; include D2CMP's public header (see module-gates.tsv)"
#endif

#include "Source/D2Headers/d2_platform.h"

// Forward declarations
struct D2LRUCacheNodeStrc;
struct D2LRUCacheStrc;
typedef void (__fastcall *pfnLruFreeEntry)(void * pEntry);

extern char gszExtDc6[5];
extern char gszExtDcc[5];
extern char gszDirObjects[20];
extern pfnLruFreeEntry gpfnSpriteCacheFreeEntry;
extern BYTE gaCelDataHashTable[4096];
extern D2LRUCacheStrc gCelDataCacheLru;
