// storage: a mod's persistent state as bytes, and back.
//
// What storage may hold (checked on save, with the path of the offending value in the error):
//   values   nil-free plain data: booleans, numbers (NaN and infinities included), strings,
//            tables without a metatable, and handles (d2_lua_api.h), which are saved as their
//            class name and id and come back as handles of the same class (valid or not, as the
//            object is then)
//   keys     strings, numbers (not NaN) and booleans
//   refused  functions, coroutines, other userdata, tables with a metatable, table keys
// Shared tables and cycles are kept: a table reached twice is written once and referenced, so
// the restored graph has the same shape. Each mod's storage is its own graph; tables shared
// between two mods' storage come back as two copies. Tables nest at most 200 deep.
//
// Format, little-endian: a value is a tag byte and its payload.
//   0 false   1 true   2 number (8-byte double)   3 string (u32 length, bytes)
//   4 table (the next id; then key, value pairs; then 6)   5 reference (u32 id)
//   6 end of table   7 handle (u32 length, class name bytes, u64 id)
#if D2_FEATURE_MODS
#include <math.h>
#include <stdint.h>
#include <string.h>

#include <string>

#include "Redivivus/Lua/d2_lua_rt.h"
#include "lua.hpp"

namespace {

enum { kFalse = 0, kTrue = 1, kNumber = 2, kString = 3, kTable = 4, kRef = 5, kEnd = 6, kHandle = 7 };
const int kMaxDepth = 200;

struct Writer {
    lua_State* L;
    std::string* pOut;
    int nSeen;     // stack index: table -> id
    int nNextId;
    char szPath[512];
};

}  // namespace

static void PutU32(std::string* pOut, uint32_t n) {
    for (int i = 0; i < 4; i++) {
        pOut->push_back((char)((n >> (8 * i)) & 0xff));
    }
}

static void PutU64(std::string* pOut, uint64_t n) {
    for (int i = 0; i < 8; i++) {
        pOut->push_back((char)((n >> (8 * i)) & 0xff));
    }
}

static void PutNumber(std::string* pOut, double d) {
    uint64_t nBits;
    memcpy(&nBits, &d, 8);
    PutU64(pOut, nBits);
}

static void PutString(std::string* pOut, const char* p, size_t n) {
    PutU32(pOut, (uint32_t)n);
    pOut->append(p, n);
}

// Append a key to the path for error messages, keeping the old length to restore.
static size_t PushPath(Writer* pW, int nKey) {
    size_t nOld = strlen(pW->szPath);
    size_t nRoom = sizeof(pW->szPath) - nOld;
    lua_State* L = pW->L;
    if (lua_type(L, nKey) == LUA_TSTRING) {
        const char* psz = lua_tostring(L, nKey);
        bool bIdent = (psz[0] < '0' || psz[0] > '9') && psz[0] != '\0';
        for (const char* q = psz; *q && bIdent; q++) {
            bIdent = (*q >= 'a' && *q <= 'z') || (*q >= 'A' && *q <= 'Z') || (*q >= '0' && *q <= '9') || *q == '_';
        }
        snprintf(pW->szPath + nOld, nRoom, bIdent ? ".%s" : "[\"%s\"]", psz);
    } else if (lua_type(L, nKey) == LUA_TNUMBER) {
        snprintf(pW->szPath + nOld, nRoom, "[%.17g]", lua_tonumber(L, nKey));
    } else {
        snprintf(pW->szPath + nOld, nRoom, "[%s]", lua_toboolean(L, nKey) ? "true" : "false");
    }
    return nOld;
}

static void WriteValue(Writer* pW, int nIndex, int nDepth, bool bKey) {
    lua_State* L = pW->L;
    nIndex = nIndex < 0 ? lua_gettop(L) + nIndex + 1 : nIndex;
    switch (lua_type(L, nIndex)) {
    case LUA_TBOOLEAN:
        pW->pOut->push_back((char)(lua_toboolean(L, nIndex) ? kTrue : kFalse));
        return;
    case LUA_TNUMBER: {
        double d = lua_tonumber(L, nIndex);
        if (bKey && d != d) {
            luaL_error(L, "storage: cannot save a NaN table key (at %s)", pW->szPath);
        }
        pW->pOut->push_back((char)kNumber);
        PutNumber(pW->pOut, d);
        return;
    }
    case LUA_TSTRING: {
        size_t n = 0;
        const char* p = lua_tolstring(L, nIndex, &n);
        pW->pOut->push_back((char)kString);
        PutString(pW->pOut, p, n);
        return;
    }
    case LUA_TUSERDATA: {
        const char* pszClass = nullptr;
        uint64_t nId = 0;
        if (!bKey && D2LuaHandle_Inspect(L, nIndex, &pszClass, &nId)) {
            pW->pOut->push_back((char)kHandle);
            PutString(pW->pOut, pszClass, strlen(pszClass));
            PutU64(pW->pOut, nId);
            return;
        }
        break;
    }
    case LUA_TTABLE: {
        if (bKey) {
            break;
        }
        lua_pushvalue(L, nIndex);
        lua_rawget(L, pW->nSeen);
        if (!lua_isnil(L, -1)) {
            pW->pOut->push_back((char)kRef);
            PutU32(pW->pOut, (uint32_t)lua_tointeger(L, -1));
            lua_pop(L, 1);
            return;
        }
        lua_pop(L, 1);
        if (lua_getmetatable(L, nIndex)) {
            luaL_error(L, "storage: cannot save a table with a metatable (at %s); storage holds plain data", pW->szPath);
        }
        if (nDepth >= kMaxDepth) {
            luaL_error(L, "storage: tables nest deeper than %d (at %s)", kMaxDepth, pW->szPath);
        }
        lua_pushvalue(L, nIndex);
        lua_pushinteger(L, pW->nNextId++);
        lua_rawset(L, pW->nSeen);
        pW->pOut->push_back((char)kTable);
        luaL_checkstack(L, 6, "storage");
        lua_pushnil(L);
        while (lua_next(L, nIndex)) {
            int nKey = lua_gettop(L) - 1;
            size_t nOld = PushPath(pW, nKey);
            WriteValue(pW, nKey, nDepth + 1, true);
            WriteValue(pW, nKey + 1, nDepth + 1, false);
            pW->szPath[nOld] = '\0';
            lua_pop(L, 1);
        }
        pW->pOut->push_back((char)kEnd);
        return;
    }
    default:
        break;
    }
    if (bKey) {
        luaL_error(L, "storage: cannot save a %s as a table key (at %s); keys are strings, numbers or booleans",
                   luaL_typename(L, nIndex), pW->szPath);
    }
    luaL_error(L, "storage: cannot save a %s (at %s); storage holds plain data and handles", luaL_typename(L, nIndex),
               pW->szPath);
}

void D2LuaStorage_Write(lua_State* L, int nIndex, std::string* pOut, const char* pszMod) {
    nIndex = nIndex < 0 ? lua_gettop(L) + nIndex + 1 : nIndex;
    Writer tW;
    tW.L = L;
    tW.pOut = pOut;
    lua_newtable(L);
    tW.nSeen = lua_gettop(L);
    tW.nNextId = 1;
    if (pszMod[0]) {
        snprintf(tW.szPath, sizeof(tW.szPath), "storage of mod '%s': storage", pszMod);
    } else {
        snprintf(tW.szPath, sizeof(tW.szPath), "storage");
    }
    WriteValue(&tW, nIndex, 0, false);
    lua_pop(L, 1);
}

// --- reading -------------------------------------------------------------------------------------

namespace {

struct Reader {
    lua_State* L;
    const std::string* pIn;
    size_t* pnPos;
    int nTables;  // stack index: id -> table
    int nNextId;
    int nInto;  // stack index of the table the root table is read into (emptied first), or 0
};

}  // namespace

static unsigned char GetByte(Reader* pR) {
    if (*pR->pnPos >= pR->pIn->size()) {
        luaL_error(pR->L, "storage: corrupt save (truncated)");
    }
    return (unsigned char)(*pR->pIn)[(*pR->pnPos)++];
}

static uint64_t GetU(Reader* pR, int nBytes) {
    uint64_t n = 0;
    for (int i = 0; i < nBytes; i++) {
        n |= (uint64_t)GetByte(pR) << (8 * i);
    }
    return n;
}

static void PushString(Reader* pR) {
    uint64_t n = GetU(pR, 4);
    if (n > pR->pIn->size() - *pR->pnPos) {
        luaL_error(pR->L, "storage: corrupt save (string length)");
    }
    lua_pushlstring(pR->L, pR->pIn->data() + *pR->pnPos, (size_t)n);
    *pR->pnPos += (size_t)n;
}

// Push the next value; returns false for the end-of-table marker (nothing pushed).
static bool ReadValue(Reader* pR, int nDepth) {
    lua_State* L = pR->L;
    luaL_checkstack(L, 4, "storage");
    unsigned char nTag = GetByte(pR);
    switch (nTag) {
    case kFalse:
    case kTrue:
        lua_pushboolean(L, nTag == kTrue);
        return true;
    case kNumber: {
        uint64_t nBits = GetU(pR, 8);
        double d;
        memcpy(&d, &nBits, 8);
        lua_pushnumber(L, d);
        return true;
    }
    case kString:
        PushString(pR);
        return true;
    case kTable: {
        if (nDepth >= kMaxDepth) {
            luaL_error(L, "storage: corrupt save (nested too deep)");
        }
        if (nDepth == 0 && pR->nInto) {
            // The root goes into the table the caller holds, so references to it stay good.
            lua_pushvalue(L, pR->nInto);
            lua_pushnil(L);
            while (lua_next(L, -2)) {
                lua_pop(L, 1);
                lua_pushvalue(L, -1);
                lua_pushnil(L);
                lua_rawset(L, -4);
            }
        } else {
            lua_newtable(L);
        }
        int nTable = lua_gettop(L);
        lua_pushvalue(L, nTable);
        lua_rawseti(L, pR->nTables, pR->nNextId++);
        while (ReadValue(pR, nDepth + 1)) {
            if (lua_isnil(L, -1) || (lua_type(L, -1) == LUA_TNUMBER && lua_tonumber(L, -1) != lua_tonumber(L, -1))) {
                luaL_error(L, "storage: corrupt save (bad key)");
            }
            if (!ReadValue(pR, nDepth + 1)) {
                luaL_error(L, "storage: corrupt save (key without value)");
            }
            lua_rawset(L, nTable);
        }
        return true;
    }
    case kRef: {
        uint64_t nId = GetU(pR, 4);
        if (nId == 0 || nId >= (uint64_t)pR->nNextId) {
            luaL_error(L, "storage: corrupt save (bad reference)");
        }
        lua_rawgeti(L, pR->nTables, (int)nId);
        return true;
    }
    case kEnd:
        return false;
    case kHandle: {
        PushString(pR);
        uint64_t nId = GetU(pR, 8);
        const char* pszClass = lua_tostring(L, -1);
        if (!D2LuaHandle_PushByName(L, pszClass, nId)) {
            luaL_error(L, "storage: the save holds a %s handle, a class this state does not have", pszClass);
        }
        lua_remove(L, -2);
        return true;
    }
    default:
        luaL_error(L, "storage: corrupt save (unknown tag %d)", (int)nTag);
        return false;
    }
}

void D2LuaStorage_Read(lua_State* L, const std::string* pIn, size_t* pnPos, int nInto) {
    Reader tR;
    tR.L = L;
    tR.pIn = pIn;
    tR.pnPos = pnPos;
    tR.nInto = nInto ? (nInto < 0 ? lua_gettop(L) + nInto + 1 : nInto) : 0;
    lua_newtable(L);
    tR.nTables = lua_gettop(L);
    tR.nNextId = 1;
    if (!ReadValue(&tR, 0)) {
        luaL_error(L, "storage: corrupt save (unexpected end of table)");
    }
    lua_remove(L, tR.nTables);
}
#endif
