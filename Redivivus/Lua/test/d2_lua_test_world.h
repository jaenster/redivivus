// A stand-in world of units for the host test, bound through test/api/test.d.ts. It stands where
// the engine's units will: the handle traits and natives here are the shape Unit's will have.
#pragma once

#if D2_FEATURE_MODS
#include <stdint.h>

#include <string>

#include "Redivivus/Lua/d2_lua_api.h"

enum class TestUnitKind { Monster = 0, Npc = 1, Player = 2 };

struct TestUnitObj {
    uint64_t nId;
    std::string sName;
    TestUnitKind eKind;
    double nHealth;
};

struct TestUnitTraits {
    typedef TestUnitObj Object;
    static Object* Resolve(uint64_t nId);
    static uint64_t IdOf(Object* pUnit);
};

// Host-side control of the world, for the test.
TestUnitObj* TestWorld_Add(const char* pszName, TestUnitKind eKind, double nHealth);
// Remove a unit (its handles become invalid). False when there was none.
bool TestWorld_Remove(uint64_t nId);
void TestWorld_Clear();
size_t TestWorld_Count();
#endif
