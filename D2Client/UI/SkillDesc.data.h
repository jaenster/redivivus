/**
 * Data defined in D2Client/UI/SkillDesc.cpp that other translation units name.
 * Binary: Game.exe
 */

#pragma once

#include "d2_platform.h"

// Forward declarations
struct D2SkillStrc;
struct D2SkillsTxt;
struct D2UnitStrc;
typedef void (__fastcall *D2SkillDescDamageFunc)(D2UnitStrc * pUnit, D2SkillStrc * pSkill, D2SkillsTxt * pSkillsTxt, int32_t nSkillLevel, int32_t nXLeft, int32_t nY, int32_t nXRight);

extern D2SkillDescDamageFunc gapSkillDamageDescFuncTable[36];
