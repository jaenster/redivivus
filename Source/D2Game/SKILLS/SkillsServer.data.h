/**
 * Data defined in D2Game/SKILLS/SkillsServer.cpp that other translation units name.
 * Binary: Game.exe
 */

#pragma once

#include "d2_platform.h"

// Forward declarations
struct D2UnitStrc;
struct SGAMEDATA;
typedef BOOL (__fastcall *D2SkillSrvDoFunc)(::SGAMEDATA * pGame, D2UnitStrc * pAttacker, eD2Skills eD2Skill, int nSkillLvl);

extern D2SkillSrvDoFunc SKILLSRVDOFUNCS[191];
