/**
 * Data defined in D2Client/UI/SkillDesc.cpp that other translation units name.
 * Binary: Game.exe
 */

#pragma once
#if defined(D2_MODULE_LIBRARIES) && defined(D2_MODULE_STRICT) && !defined(D2_GATE_FROM_ROOT) && !defined(D2_SEES_D2Client)
#error "D2Client header included by a module that may not use D2Client (original DLL import graph; see module-gates.tsv)"
#endif
#if defined(D2_MODULE_LIBRARIES) && defined(D2_MODULE_STRICT) && !defined(D2_GATE_FROM_ROOT) && !defined(D2_MODULE_D2Client) && !defined(D2_LOADS_D2Client)
#error "internal D2Client header included from another module; include D2Client's public header (see module-gates.tsv)"
#endif

#include "Source/D2Headers/d2_platform.h"

// Forward declarations
struct D2SkillStrc;
struct D2SkillsTxt;
struct D2UnitStrc;
typedef void (__fastcall *D2SkillDescDamageFunc)(D2UnitStrc * pUnit, D2SkillStrc * pSkill, D2SkillsTxt * pSkillsTxt, int32_t nSkillLevel, int32_t nXLeft, int32_t nY, int32_t nXRight);

extern D2SkillDescDamageFunc gapSkillDamageDescFuncTable[36];
