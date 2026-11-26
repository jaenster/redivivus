// string.find, string.match, string.gmatch and string.gsub for mod environments: Lua 5.1
// patterns with the semantics of LuaJIT 2.1's lib_string.c (the vendored copy, which stays
// unmodified), plus a step counter. The stock matcher backtracks inside one C call, where the
// time budget's hook never runs, so ("a"):rep(2000):find(".-.-.-b") runs for minutes. This one
// counts every matcher step and, every kStepsPerCheck steps, asks the host whether the current
// call's deadline has passed; when it has, the call raises the budget error and the host faults
// the mod as it does for any other overrun.
//
// The matcher itself follows lib_string.c (MIT, Copyright (C) 2005-2025 Mike Pall; derived from
// Lua 5.1's lstrlib.c, Copyright (C) 1994-2012 Lua.org, PUC-Rio) step for step, so results,
// captures and error messages are the same. Character classes use LuaJIT's ASCII table, not the C
// locale. Errors are raised with luaL_error, which reports the caller's position as LuaJIT's
// lj_err_caller does.
//
// No function here holds an object with a destructor (errors unwind through these frames).
#if D2_FEATURE_MODS
#include "Redivivus/Lua/d2_lua_internal.h"

#include <stdint.h>
#include <string.h>

#include "lua.hpp"

namespace {

const int kMaxCaptures = 32;  // LUA_MAXCAPTURES
const int kMaxDepth = 200;    // LJ_MAX_XLEVEL
const unsigned kStepsPerCheck = 1024;
const ptrdiff_t kCapUnfinished = -1;
const ptrdiff_t kCapPosition = -2;
const char kEsc = '%';

struct MatchState {
    const char* pSrcInit;
    const char* pSrcEnd;
    lua_State* L;
    int nLevel;
    int nDepth;
    unsigned nSteps;
    struct {
        const char* pInit;
        ptrdiff_t nLen;
    } aCapture[kMaxCaptures];
};

// LuaJIT's lj_char classes: ASCII only, bytes 128..255 belong to no class.
enum { kCntrl = 1, kSpace = 2, kPunct = 4, kDigit = 8, kXdigit = 16, kUpper = 32, kLower = 64 };
const int kAlpha = kLower | kUpper;
const int kAlnum = kAlpha | kDigit;
const int kGraph = kAlnum | kPunct;

int CharBits(int c) {
    if (c < 0 || c >= 128) {
        return 0;
    }
    int nBits = 0;
    if (c < 32 || c == 127) {
        nBits |= kCntrl;
    }
    if (c == ' ' || (c >= '\t' && c <= '\r')) {
        nBits |= kSpace;
    }
    if (c >= '0' && c <= '9') {
        nBits |= kDigit | kXdigit;
    }
    if ((c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F')) {
        nBits |= kXdigit;
    }
    if (c >= 'A' && c <= 'Z') {
        nBits |= kUpper;
    }
    if (c >= 'a' && c <= 'z') {
        nBits |= kLower;
    }
    if (c > 32 && c < 127 && !(nBits & kAlnum)) {
        nBits |= kPunct;
    }
    return nBits;
}

}  // namespace

static inline int UChar(char c) {
    return (unsigned char)c;
}

static void Step(MatchState* pMs) {
    if (++pMs->nSteps % kStepsPerCheck == 0 && D2LuaHost_DeadlinePassed(pMs->L)) {
        luaL_error(pMs->L, "time budget exceeded");
    }
}

static int CheckCapture(MatchState* pMs, int l) {
    l -= '1';
    if (l < 0 || l >= pMs->nLevel || pMs->aCapture[l].nLen == kCapUnfinished) {
        luaL_error(pMs->L, "invalid capture index");
    }
    return l;
}

static int CaptureToClose(MatchState* pMs) {
    int nLevel = pMs->nLevel;
    for (nLevel--; nLevel >= 0; nLevel--) {
        if (pMs->aCapture[nLevel].nLen == kCapUnfinished) {
            return nLevel;
        }
    }
    luaL_error(pMs->L, "invalid pattern capture");
    return 0;
}

static const char* ClassEnd(MatchState* pMs, const char* p) {
    switch (*p++) {
    case kEsc:
        if (*p == '\0') {
            luaL_error(pMs->L, "malformed pattern (ends with '%%')");
        }
        return p + 1;
    case '[':
        if (*p == '^') {
            p++;
        }
        do {
            if (*p == '\0') {
                luaL_error(pMs->L, "malformed pattern (missing ']')");
            }
            if (*(p++) == kEsc && *p != '\0') {
                p++;
            }
        } while (*p != ']');
        return p + 1;
    default:
        return p;
    }
}

static int MatchClass(int c, int cl) {
    if ((cl & 0xc0) == 0x40) {
        int nClass = 0;
        switch (cl | 0x20) {
        case 'a': nClass = kAlpha; break;
        case 'c': nClass = kCntrl; break;
        case 'd': nClass = kDigit; break;
        case 'g': nClass = kGraph; break;
        case 'l': nClass = kLower; break;
        case 'p': nClass = kPunct; break;
        case 's': nClass = kSpace; break;
        case 'u': nClass = kUpper; break;
        case 'w': nClass = kAlnum; break;
        case 'x': nClass = kXdigit; break;
        default: break;
        }
        if (nClass) {
            int bIs = (CharBits(c) & nClass) != 0;
            return (cl & 0x20) ? bIs : !bIs;
        }
        if (cl == 'z') {
            return c == 0;
        }
        if (cl == 'Z') {
            return c != 0;
        }
    }
    return cl == c;
}

static int MatchBracketClass(int c, const char* p, const char* ec) {
    int bSig = 1;
    if (*(p + 1) == '^') {
        bSig = 0;
        p++;
    }
    while (++p < ec) {
        if (*p == kEsc) {
            p++;
            if (MatchClass(c, UChar(*p))) {
                return bSig;
            }
        } else if (*(p + 1) == '-' && p + 2 < ec) {
            p += 2;
            if (UChar(*(p - 2)) <= c && c <= UChar(*p)) {
                return bSig;
            }
        } else if (UChar(*p) == c) {
            return bSig;
        }
    }
    return !bSig;
}

static int SingleMatch(int c, const char* p, const char* ep) {
    switch (*p) {
    case '.': return 1;
    case kEsc: return MatchClass(c, UChar(*(p + 1)));
    case '[': return MatchBracketClass(c, p, ep - 1);
    default: return UChar(*p) == c;
    }
}

static const char* Match(MatchState* pMs, const char* s, const char* p);

static const char* MatchBalance(MatchState* pMs, const char* s, const char* p) {
    if (*p == 0 || *(p + 1) == 0) {
        luaL_error(pMs->L, "unbalanced pattern");
    }
    if (*s != *p) {
        return nullptr;
    }
    int b = *p;
    int e = *(p + 1);
    int nCont = 1;
    while (++s < pMs->pSrcEnd) {
        Step(pMs);
        if (*s == e) {
            if (--nCont == 0) {
                return s + 1;
            }
        } else if (*s == b) {
            nCont++;
        }
    }
    return nullptr;
}

static const char* MaxExpand(MatchState* pMs, const char* s, const char* p, const char* ep) {
    ptrdiff_t i = 0;
    while (s + i < pMs->pSrcEnd && SingleMatch(UChar(*(s + i)), p, ep)) {
        Step(pMs);
        i++;
    }
    while (i >= 0) {
        const char* pRes = Match(pMs, s + i, ep + 1);
        if (pRes) {
            return pRes;
        }
        i--;
    }
    return nullptr;
}

static const char* MinExpand(MatchState* pMs, const char* s, const char* p, const char* ep) {
    for (;;) {
        const char* pRes = Match(pMs, s, ep + 1);
        if (pRes) {
            return pRes;
        } else if (s < pMs->pSrcEnd && SingleMatch(UChar(*s), p, ep)) {
            s++;
        } else {
            return nullptr;
        }
    }
}

static const char* StartCapture(MatchState* pMs, const char* s, const char* p, ptrdiff_t nWhat) {
    int nLevel = pMs->nLevel;
    if (nLevel >= kMaxCaptures) {
        luaL_error(pMs->L, "too many captures");
    }
    pMs->aCapture[nLevel].pInit = s;
    pMs->aCapture[nLevel].nLen = nWhat;
    pMs->nLevel = nLevel + 1;
    const char* pRes = Match(pMs, s, p);
    if (!pRes) {
        pMs->nLevel--;
    }
    return pRes;
}

static const char* EndCapture(MatchState* pMs, const char* s, const char* p) {
    int l = CaptureToClose(pMs);
    pMs->aCapture[l].nLen = s - pMs->aCapture[l].pInit;
    const char* pRes = Match(pMs, s, p);
    if (!pRes) {
        pMs->aCapture[l].nLen = kCapUnfinished;
    }
    return pRes;
}

static const char* MatchCapture(MatchState* pMs, const char* s, int l) {
    l = CheckCapture(pMs, l);
    size_t nLen = (size_t)pMs->aCapture[l].nLen;
    if ((size_t)(pMs->pSrcEnd - s) >= nLen && memcmp(pMs->aCapture[l].pInit, s, nLen) == 0) {
        return s + nLen;
    }
    return nullptr;
}

static const char* Match(MatchState* pMs, const char* s, const char* p) {
    if (++pMs->nDepth > kMaxDepth) {
        luaL_error(pMs->L, "pattern too complex");
    }
init:
    Step(pMs);
    switch (*p) {
    case '(':
        if (*(p + 1) == ')') {
            s = StartCapture(pMs, s, p + 2, kCapPosition);
        } else {
            s = StartCapture(pMs, s, p + 1, kCapUnfinished);
        }
        break;
    case ')':
        s = EndCapture(pMs, s, p + 1);
        break;
    case kEsc:
        switch (*(p + 1)) {
        case 'b':
            s = MatchBalance(pMs, s, p + 2);
            if (!s) {
                break;
            }
            p += 4;
            goto init;
        case 'f': {
            p += 2;
            if (*p != '[') {
                luaL_error(pMs->L, "missing '[' after '%%f' in pattern");
            }
            const char* ep = ClassEnd(pMs, p);
            char cPrevious = (s == pMs->pSrcInit) ? '\0' : *(s - 1);
            if (MatchBracketClass(UChar(cPrevious), p, ep - 1) || !MatchBracketClass(UChar(*s), p, ep - 1)) {
                s = nullptr;
                break;
            }
            p = ep;
            goto init;
        }
        default:
            if (*(p + 1) >= '0' && *(p + 1) <= '9') {
                s = MatchCapture(pMs, s, UChar(*(p + 1)));
                if (!s) {
                    break;
                }
                p += 2;
                goto init;
            }
            goto dflt;
        }
        break;
    case '\0':
        break;
    case '$':
        if (*(p + 1) != '\0') {
            goto dflt;
        }
        if (s != pMs->pSrcEnd) {
            s = nullptr;
        }
        break;
    default:
    dflt: {
        const char* ep = ClassEnd(pMs, p);
        int m = s < pMs->pSrcEnd && SingleMatch(UChar(*s), p, ep);
        switch (*ep) {
        case '?': {
            const char* pRes;
            if (m && (pRes = Match(pMs, s + 1, ep + 1)) != nullptr) {
                s = pRes;
                break;
            }
            p = ep + 1;
            goto init;
        }
        case '*':
            s = MaxExpand(pMs, s, p, ep);
            break;
        case '+':
            s = m ? MaxExpand(pMs, s + 1, p, ep) : nullptr;
            break;
        case '-':
            s = MinExpand(pMs, s, p, ep);
            break;
        default:
            if (m) {
                s++;
                p = ep;
                goto init;
            }
            s = nullptr;
            break;
        }
        break;
    }
    }
    pMs->nDepth--;
    return s;
}

static void PushOneCapture(MatchState* pMs, int i, const char* s, const char* e) {
    if (i >= pMs->nLevel) {
        if (i == 0) {
            lua_pushlstring(pMs->L, s, (size_t)(e - s));
        } else {
            luaL_error(pMs->L, "invalid capture index");
        }
    } else {
        ptrdiff_t l = pMs->aCapture[i].nLen;
        if (l == kCapUnfinished) {
            luaL_error(pMs->L, "unfinished capture");
        }
        if (l == kCapPosition) {
            lua_pushinteger(pMs->L, pMs->aCapture[i].pInit - pMs->pSrcInit + 1);
        } else {
            lua_pushlstring(pMs->L, pMs->aCapture[i].pInit, (size_t)l);
        }
    }
}

static int PushCaptures(MatchState* pMs, const char* s, const char* e) {
    int nLevels = (pMs->nLevel == 0 && s) ? 1 : pMs->nLevel;
    luaL_checkstack(pMs->L, nLevels, "too many captures");
    for (int i = 0; i < nLevels; i++) {
        PushOneCapture(pMs, i, s, e);
    }
    return nLevels;
}

static void InitMatch(MatchState* pMs, lua_State* L, const char* pSrc, size_t nSrc) {
    pMs->L = L;
    pMs->pSrcInit = pSrc;
    pMs->pSrcEnd = pSrc + nSrc;
    pMs->nLevel = 0;
    pMs->nDepth = 0;
    pMs->nSteps = 0;
}

// lj_str_haspattern
static int HasPattern(const char* p, size_t n) {
    for (size_t i = 0; i < n; i++) {
        int c = UChar(p[i]);
        if ((CharBits(c) & kPunct) && strchr("^$*+?.([%-", c)) {
            return 1;
        }
    }
    return 0;
}

// lj_str_find, counted: the first occurrence of p in s, or null.
static const char* PlainFind(MatchState* pMs, const char* s, const char* p, size_t nS, size_t nP) {
    if (nP == 0) {
        return s;
    }
    if (nP > nS) {
        return nullptr;
    }
    const char* pLast = s + (nS - nP);
    for (const char* q = s; q <= pLast; q++) {
        Step(pMs);
        if (*q == *p && memcmp(q, p, nP) == 0) {
            return q;
        }
    }
    return nullptr;
}

static int FindAux(lua_State* L, int bFind) {
    size_t nS = 0;
    size_t nP = 0;
    const char* pS = luaL_checklstring(L, 1, &nS);
    const char* pP = luaL_checklstring(L, 2, &nP);
    int32_t nStart = (int32_t)luaL_optinteger(L, 3, 1);
    if (nStart < 0) {
        nStart += (int32_t)nS;
    } else {
        nStart--;
    }
    if (nStart < 0) {
        nStart = 0;
    }
    size_t nSt = (size_t)nStart;
    if (nSt > nS) {
        nSt = nS;
    }
    MatchState tMs;
    InitMatch(&tMs, L, pS, nS);
    if (bFind && (lua_toboolean(L, 4) || !HasPattern(pP, nP))) {
        const char* q = PlainFind(&tMs, pS + nSt, pP, nS - nSt, nP);
        if (q) {
            lua_pushinteger(L, (lua_Integer)(q - pS) + 1);
            lua_pushinteger(L, (lua_Integer)(q - pS) + (lua_Integer)nP);
            return 2;
        }
    } else {
        const char* pStr = pS + nSt;
        int bAnchor = 0;
        if (*pP == '^') {
            pP++;
            bAnchor = 1;
        }
        do {
            tMs.nLevel = 0;
            tMs.nDepth = 0;
            const char* q = Match(&tMs, pStr, pP);
            if (q) {
                if (bFind) {
                    lua_pushinteger(L, (lua_Integer)(pStr - pS) + 1);
                    lua_pushinteger(L, (lua_Integer)(q - pS));
                    return PushCaptures(&tMs, nullptr, nullptr) + 2;
                }
                return PushCaptures(&tMs, pStr, q);
            }
        } while (pStr++ < tMs.pSrcEnd && !bAnchor);
    }
    lua_pushnil(L);
    return 1;
}

static int Lua_Find(lua_State* L) {
    return FindAux(L, 1);
}

static int Lua_Match(lua_State* L) {
    return FindAux(L, 0);
}

// Upvalues: the string, the pattern, the position to continue from.
static int Lua_GmatchAux(lua_State* L) {
    size_t nS = 0;
    const char* pS = lua_tolstring(L, lua_upvalueindex(1), &nS);
    const char* pP = lua_tostring(L, lua_upvalueindex(2));
    size_t nPos = (size_t)lua_tointeger(L, lua_upvalueindex(3));
    MatchState tMs;
    InitMatch(&tMs, L, pS, nS);
    for (const char* pSrc = pS + nPos; pSrc <= tMs.pSrcEnd; pSrc++) {
        tMs.nLevel = 0;
        tMs.nDepth = 0;
        const char* e = Match(&tMs, pSrc, pP);
        if (e) {
            size_t nNew = (size_t)(e - pS);
            if (e == pSrc) {
                nNew++;
            }
            lua_pushinteger(L, (lua_Integer)nNew);
            lua_replace(L, lua_upvalueindex(3));
            return PushCaptures(&tMs, pSrc, e);
        }
    }
    return 0;
}

static int Lua_Gmatch(lua_State* L) {
    luaL_checkstring(L, 1);
    luaL_checkstring(L, 2);
    lua_settop(L, 2);
    lua_pushinteger(L, 0);
    lua_pushcclosure(L, Lua_GmatchAux, 3);
    return 1;
}

static void AddS(MatchState* pMs, luaL_Buffer* pB, const char* s, const char* e) {
    size_t l = 0;
    const char* pNew = lua_tolstring(pMs->L, 3, &l);
    for (size_t i = 0; i < l; i++) {
        if (pNew[i] != kEsc) {
            luaL_addchar(pB, pNew[i]);
        } else {
            i++;
            if (!(pNew[i] >= '0' && pNew[i] <= '9')) {
                luaL_addchar(pB, pNew[i]);
            } else if (pNew[i] == '0') {
                luaL_addlstring(pB, s, (size_t)(e - s));
            } else {
                PushOneCapture(pMs, pNew[i] - '1', s, e);
                luaL_addvalue(pB);
            }
        }
    }
}

static void AddValue(MatchState* pMs, luaL_Buffer* pB, const char* s, const char* e) {
    lua_State* L = pMs->L;
    switch (lua_type(L, 3)) {
    case LUA_TNUMBER:
    case LUA_TSTRING:
        AddS(pMs, pB, s, e);
        return;
    case LUA_TFUNCTION: {
        lua_pushvalue(L, 3);
        int n = PushCaptures(pMs, s, e);
        lua_call(L, n, 1);
        break;
    }
    case LUA_TTABLE:
        PushOneCapture(pMs, 0, s, e);
        lua_gettable(L, 3);
        break;
    }
    if (!lua_toboolean(L, -1)) {
        lua_pop(L, 1);
        lua_pushlstring(L, s, (size_t)(e - s));
    } else if (!lua_isstring(L, -1)) {
        luaL_error(L, "invalid replacement value (a %s)", luaL_typename(L, -1));
    }
    luaL_addvalue(pB);
}

static int Lua_Gsub(lua_State* L) {
    size_t nSrc = 0;
    const char* pSrc = luaL_checklstring(L, 1, &nSrc);
    const char* p = luaL_checkstring(L, 2);
    int nTr = lua_type(L, 3);
    int nMax = luaL_optint(L, 4, (int)(nSrc + 1));
    int bAnchor = (*p == '^') ? (p++, 1) : 0;
    int n = 0;
    if (!(nTr == LUA_TNUMBER || nTr == LUA_TSTRING || nTr == LUA_TFUNCTION || nTr == LUA_TTABLE)) {
        luaL_argerror(L, 3, "string/function/table expected");
    }
    MatchState tMs;
    luaL_Buffer tB;
    luaL_buffinit(L, &tB);
    InitMatch(&tMs, L, pSrc, nSrc);
    while (n < nMax) {
        tMs.nLevel = 0;
        tMs.nDepth = 0;
        const char* e = Match(&tMs, pSrc, p);
        if (e) {
            n++;
            AddValue(&tMs, &tB, pSrc, e);
        }
        if (e && e > pSrc) {
            pSrc = e;
        } else if (pSrc < tMs.pSrcEnd) {
            luaL_addchar(&tB, *pSrc++);
        } else {
            break;
        }
        if (bAnchor) {
            break;
        }
    }
    luaL_addlstring(&tB, pSrc, (size_t)(tMs.pSrcEnd - pSrc));
    luaL_pushresult(&tB);
    lua_pushinteger(L, n);
    return 2;
}

void D2LuaPattern_Install(lua_State* L, int nStringTable) {
    static const luaL_Reg s_aFunctions[] = {
        { "find", Lua_Find }, { "match", Lua_Match }, { "gmatch", Lua_Gmatch }, { "gsub", Lua_Gsub },
    };
    nStringTable = nStringTable < 0 ? lua_gettop(L) + nStringTable + 1 : nStringTable;
    for (size_t i = 0; i < sizeof(s_aFunctions) / sizeof(s_aFunctions[0]); i++) {
        lua_pushcfunction(L, s_aFunctions[i].func);
        lua_setfield(L, nStringTable, s_aFunctions[i].name);
    }
}
#endif
