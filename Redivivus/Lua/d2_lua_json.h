// A small JSON reader for the files the Lua host reads itself: a mod's info.json and the source
// maps TypeScriptToLua writes. Strict RFC 8259 input, UTF-8 passed through, \u escapes decoded
// to UTF-8 (surrogate pairs joined). Never called from inside Lua.
#pragma once

#if D2_FEATURE_MODS
#include <string>
#include <utility>
#include <vector>

struct D2Json {
    enum Type { Null, Bool, Number, String, Array, Object };
    Type eType = Null;
    bool bValue = false;
    double nValue = 0;
    std::string sValue;
    std::vector<D2Json> aItems;                             // Array
    std::vector<std::pair<std::string, D2Json>> aMembers;  // Object, in file order

    // The member called pszKey of an object, or null.
    const D2Json* Get(const char* pszKey) const;
};

// Parse text into tOut. On failure returns false and sets sError to "line L, column C: why".
bool D2Json_Parse(const char* pText, size_t nLen, D2Json& tOut, std::string& sError);
#endif
