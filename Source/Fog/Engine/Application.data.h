/**
 * Data defined in Fog/Engine/Application.cpp that other translation units name.
 * Binary: Game.exe
 */

#pragma once
#if defined(D2_MODULE_LIBRARIES) && defined(D2_MODULE_STRICT) && !defined(D2_GATE_FROM_ROOT) && !defined(D2_SEES_Fog)
#error "Fog header included by a module that may not use Fog (original DLL import graph; see module-gates.tsv)"
#endif

#include "Source/D2Headers/d2_platform.h"
#include "Source/globals/globals.h"
#include "Source/D2Hell/D2Hell.h"
#include "Source/Fog/D2MemoryManagerStrc.h"
#include "Source/Fog/D2PoolManagerStrc.h"
#include "Source/Fog/D2QServerStrc.h"
#include "Source/Fog/Memory.h"
#include "Source/Fog/Save.h"
#include "Source/Fog/Source/SFile.h"
#include "Source/Fog/Source/SSComp.h"
#include "Source/Fog/Src/LogManager.h"
#include "3rdparty/Storm/3rdparty/zlib/deflate.h"
#include "3rdparty/Storm/H/stpl.h"
#include "3rdparty/Storm/Source/SCMD.h"
#include "3rdparty/Storm/Source/SComp.h"
#include "3rdparty/Storm/Source/SErr.h"
#include "3rdparty/Storm/Source/SMem.h"
#include "3rdparty/Storm/Storm.h"
#include "Source/D2Headers/d2_platform.h"

extern uint32_t gaEnumProcessModuleHandles[1028];
extern uint8_t IMAGE_RESOURCE_DIRECTORY_00995000[16];
// Rsrc_Icon_1_409 Size of resource: 0x2e8 bytes
extern uint8_t Rsrc_Icon_1_409[744];
// Rsrc_Icon_2_409 Size of resource: 0x128 bytes
extern uint8_t Rsrc_Icon_2_409[296];
// Rsrc_Icon_3_409 Size of resource: 0x2e8 bytes
extern uint8_t Rsrc_Icon_3_409[744];
// Rsrc_Icon_4_409 Size of resource: 0x128 bytes
extern uint8_t Rsrc_Icon_4_409[296];
// Rsrc_Icon_5_409 Size of resource: 0x8a8 bytes
extern uint8_t Rsrc_Icon_5_409[2216];
// Rsrc_Icon_6_409 Size of resource: 0x128 bytes
extern uint8_t Rsrc_Icon_6_409[296];
// Rsrc_GroupIcon_65_409 Size of resource: 0x22 bytes
extern uint8_t Rsrc_GroupIcon_65_409[34];
// Rsrc_GroupIcon_66_409 Size of resource: 0x22 bytes
extern uint8_t Rsrc_GroupIcon_66_409[34];
// Rsrc_GroupIcon_67_409 Size of resource: 0x22 bytes
extern uint8_t Rsrc_GroupIcon_67_409[34];
// Rsrc_Version_1_409 Size of resource: 0x454 bytes
extern uint8_t Rsrc_Version_1_409[92];
// Rsrc_Manifest_1_409 Size of resource: 0x56 bytes
extern char Rsrc_Manifest_1_409[1745];
extern void * ExceptionList;
extern void * SubSystemTib;
extern void * FiberData;
extern void * ArbitraryUserPointer;
extern void * Self;
extern void * EnvironmentPointer;
extern CLIENT_ID ClientId;
extern void * ActiveRpcHandle;
extern void * ThreadLocalStoragePointer;
