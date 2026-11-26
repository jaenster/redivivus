// See d2_lua_test_world.h. Built into the host test only.
#if D2_FEATURE_MODS && D2_LUA_HOST_TEST
#include "Redivivus/Lua/test/d2_lua_test_world.h"

#include <map>
#include <memory>

#include "Redivivus/Lua/test/generated/d2_lua_api_test.gen.h"

static std::map<uint64_t, std::unique_ptr<TestUnitObj>> s_tUnits;
static uint64_t s_nNextId = 1;

TestUnitObj* TestUnitTraits::Resolve(uint64_t nId) {
    auto it = s_tUnits.find(nId);
    return it == s_tUnits.end() ? nullptr : it->second.get();
}

uint64_t TestUnitTraits::IdOf(TestUnitObj* pUnit) {
    return pUnit->nId;
}

TestUnitObj* TestWorld_Add(const char* pszName, TestUnitKind eKind, double nHealth) {
    std::unique_ptr<TestUnitObj> pUnit(new TestUnitObj);
    pUnit->nId = s_nNextId++;
    pUnit->sName = pszName;
    pUnit->eKind = eKind;
    pUnit->nHealth = nHealth;
    TestUnitObj* pRaw = pUnit.get();
    s_tUnits[pRaw->nId] = std::move(pUnit);
    return pRaw;
}

bool TestWorld_Remove(uint64_t nId) {
    return s_tUnits.erase(nId) != 0;
}

void TestWorld_Clear() {
    s_tUnits.clear();
    s_nNextId = 1;
}

size_t TestWorld_Count() {
    return s_tUnits.size();
}

// --- the natives test/api/test.d.ts binds -----------------------------------------------------------
// Called from Lua: no object with a destructor lives across a call that may raise.

lua_Integer TestUnit_GetId(D2LuaCtx& ctx, TestUnitObj* self) {
    (void)ctx;
    return (lua_Integer)self->nId;
}

const char* TestUnit_GetName(D2LuaCtx& ctx, TestUnitObj* self) {
    (void)ctx;
    return self->sName.c_str();
}

TestUnitKind TestUnit_GetKind(D2LuaCtx& ctx, TestUnitObj* self) {
    (void)ctx;
    return self->eKind;
}

double TestUnit_GetHealth(D2LuaCtx& ctx, TestUnitObj* self) {
    (void)ctx;
    return self->nHealth;
}

void TestUnit_SetHealth(D2LuaCtx& ctx, TestUnitObj* self, double nValue) {
    (void)ctx;
    self->nHealth = nValue;
}

bool TestUnit_Damage(D2LuaCtx& ctx, TestUnitObj* self, double nAmount) {
    self->nHealth -= nAmount;
    bool bKilled = self->nHealth <= 0;
    if (!bKilled) {
        D2LuaRaise_testUnitDamaged(ctx.L, self, nAmount, false);
        return false;
    }
    // Out of the world first, so handlers see the handle as invalid; freed after the event.
    auto it = s_tUnits.find(self->nId);
    TestUnitObj* pDead = it->second.release();
    s_tUnits.erase(it);
    D2LuaRaise_testUnitDamaged(ctx.L, pDead, nAmount, true);
    delete pDead;
    return true;
}

TestUnitObj* TestUnit_FindNearest(D2LuaCtx& ctx, TestUnitObj* self, D2LuaOpt<TestUnitKind> tKind) {
    (void)ctx;
    TestUnitObj* pBest = nullptr;
    uint64_t nBest = 0;
    for (auto& tEntry : s_tUnits) {
        TestUnitObj* pUnit = tEntry.second.get();
        if (pUnit == self || (tKind.bHas && pUnit->eKind != tKind.tValue)) {
            continue;
        }
        uint64_t nDistance = pUnit->nId > self->nId ? pUnit->nId - self->nId : self->nId - pUnit->nId;
        if (!pBest || nDistance < nBest) {
            pBest = pUnit;
            nBest = nDistance;
        }
    }
    return pBest;
}

TestUnitObj* TestWorld_GetUnit(D2LuaCtx& ctx, lua_Integer nId) {
    (void)ctx;
    return TestUnitTraits::Resolve((uint64_t)nId);
}

TestUnitObj* TestWorld_Spawn(D2LuaCtx& ctx, const char* pszName, TestUnitKind eKind, double nHealth) {
    (void)ctx;
    return TestWorld_Add(pszName, eKind, nHealth);
}

double TestRegistry_DamageFormula(D2LuaCtx& ctx, TestUnitObj* pUnit, double nAmount) {
    (void)ctx;
    (void)pUnit;
    return nAmount / 2;
}
#endif
