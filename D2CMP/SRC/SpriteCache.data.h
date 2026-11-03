/**
 * Data defined in D2CMP/SRC/SpriteCache.cpp that other translation units name.
 * Binary: Game.exe
 */

#pragma once

#include "d2_platform.h"
#include "D2CMP/SRC/LRUCache.h"

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
