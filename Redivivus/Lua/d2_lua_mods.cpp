// See d2_lua_mods.h.
#if D2_FEATURE_MODS
#include "Redivivus/Lua/d2_lua_mods.h"

#include <dirent.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>

#include <algorithm>
#include <map>
#include <set>

#include "Redivivus/Lua/d2_lua_json.h"

bool D2LuaMods_ReadFile(const std::string& sPath, std::string& sOut) {
    FILE* pFile = fopen(sPath.c_str(), "rb");
    if (!pFile) {
        return false;
    }
    sOut.clear();
    char aBuf[8192];
    size_t n;
    while ((n = fread(aBuf, 1, sizeof(aBuf), pFile)) > 0) {
        sOut.append(aBuf, n);
    }
    bool bOk = !ferror(pFile);
    fclose(pFile);
    return bOk;
}

bool D2LuaMods_IsValidName(const std::string& sName) {
    if (sName.empty() || sName.size() > 64) {
        return false;
    }
    for (char c : sName) {
        if (!((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || c == '_' || c == '-')) {
            return false;
        }
    }
    return true;
}

bool D2LuaMods_ParseVersion(const std::string& sText, D2LuaVersion& tOut) {
    tOut = D2LuaVersion();
    size_t nPos = 0;
    for (int i = 0; i < 3; i++) {
        if (nPos >= sText.size() || !(sText[nPos] >= '0' && sText[nPos] <= '9')) {
            return false;
        }
        long nPart = 0;
        while (nPos < sText.size() && sText[nPos] >= '0' && sText[nPos] <= '9') {
            nPart = nPart * 10 + (sText[nPos] - '0');
            if (nPart > 65535) {
                return false;
            }
            nPos++;
        }
        tOut.a[i] = (int)nPart;
        if (nPos == sText.size()) {
            return true;
        }
        if (sText[nPos] != '.' || i == 2) {
            return false;
        }
        nPos++;
    }
    return false;
}

int D2LuaMods_CompareVersions(const D2LuaVersion& a, const D2LuaVersion& b) {
    for (int i = 0; i < 3; i++) {
        if (a.a[i] != b.a[i]) {
            return a.a[i] < b.a[i] ? -1 : 1;
        }
    }
    return 0;
}

static std::string Trim(const std::string& s) {
    size_t b = 0;
    size_t e = s.size();
    while (b < e && (s[b] == ' ' || s[b] == '\t')) {
        b++;
    }
    while (e > b && (s[e - 1] == ' ' || s[e - 1] == '\t')) {
        e--;
    }
    return s.substr(b, e - b);
}

bool D2LuaMods_ParseDependency(const std::string& sText, D2LuaDependency& tOut, std::string& sError) {
    tOut = D2LuaDependency();
    tOut.sText = sText;
    std::string s = Trim(sText);
    if (s.compare(0, 3, "(?)") == 0) {
        tOut.eKind = D2LuaDepKind::HiddenOptional;
        s = Trim(s.substr(3));
    } else if (!s.empty() && (s[0] == '?' || s[0] == '!' || s[0] == '~')) {
        tOut.eKind = s[0] == '?' ? D2LuaDepKind::Optional
                   : s[0] == '!' ? D2LuaDepKind::Incompatible
                                 : D2LuaDepKind::Unordered;
        s = Trim(s.substr(1));
    }
    size_t nEnd = 0;
    while (nEnd < s.size() && s[nEnd] != ' ' && s[nEnd] != '\t' && s[nEnd] != '<' && s[nEnd] != '>' && s[nEnd] != '=') {
        nEnd++;
    }
    tOut.sName = s.substr(0, nEnd);
    if (!D2LuaMods_IsValidName(tOut.sName)) {
        sError = "dependency \"" + sText + "\": bad mod name";
        return false;
    }
    std::string sRest = Trim(s.substr(nEnd));
    if (sRest.empty()) {
        return true;
    }
    if (tOut.eKind == D2LuaDepKind::Incompatible) {
        sError = "dependency \"" + sText + "\": an incompatibility takes no version";
        return false;
    }
    static const struct { const char* psz; D2LuaVersionOp eOp; } s_aOps[] = {
        { "<=", D2LuaVersionOp::LessEqual }, { ">=", D2LuaVersionOp::GreaterEqual }, { "<", D2LuaVersionOp::Less },
        { ">", D2LuaVersionOp::Greater }, { "=", D2LuaVersionOp::Equal },
    };
    for (const auto& tOp : s_aOps) {
        size_t n = strlen(tOp.psz);
        if (sRest.compare(0, n, tOp.psz) == 0) {
            tOut.eOp = tOp.eOp;
            if (!D2LuaMods_ParseVersion(Trim(sRest.substr(n)), tOut.tVersion)) {
                sError = "dependency \"" + sText + "\": bad version";
                return false;
            }
            return true;
        }
    }
    sError = "dependency \"" + sText + "\": expected an operator (< <= = >= >) before the version";
    return false;
}

static bool VersionSatisfies(const D2LuaVersion& tHave, D2LuaVersionOp eOp, const D2LuaVersion& tWant) {
    int c = D2LuaMods_CompareVersions(tHave, tWant);
    switch (eOp) {
    case D2LuaVersionOp::Any: return true;
    case D2LuaVersionOp::Less: return c < 0;
    case D2LuaVersionOp::LessEqual: return c <= 0;
    case D2LuaVersionOp::Equal: return c == 0;
    case D2LuaVersionOp::GreaterEqual: return c >= 0;
    case D2LuaVersionOp::Greater: return c > 0;
    }
    return false;
}

static bool StringField(const D2Json& tRoot, const char* pszKey, bool bRequired, std::string& sOut, std::string& sError) {
    const D2Json* pValue = tRoot.Get(pszKey);
    if (!pValue) {
        if (bRequired) {
            sError = std::string("info.json: \"") + pszKey + "\" is missing";
            return false;
        }
        return true;
    }
    if (pValue->eType != D2Json::String) {
        sError = std::string("info.json: \"") + pszKey + "\" must be a string";
        return false;
    }
    sOut = pValue->sValue;
    return true;
}

bool D2LuaMods_ReadInfo(const std::string& sDir, D2LuaModInfo& tOut, std::string& sError) {
    std::string sText;
    if (!D2LuaMods_ReadFile(sDir + "/info.json", sText)) {
        tOut = D2LuaModInfo();
        sError = "info.json is missing or unreadable";
        return false;
    }
    return D2LuaMods_ParseInfo(sText, sDir, tOut, sError);
}

bool D2LuaMods_ParseInfo(const std::string& sText, const std::string& sDir, D2LuaModInfo& tOut, std::string& sError) {
    tOut = D2LuaModInfo();
    tOut.sPath = sDir;
    D2Json tRoot;
    std::string sJsonError;
    if (!D2Json_Parse(sText.data(), sText.size(), tRoot, sJsonError)) {
        sError = "info.json: " + sJsonError;
        return false;
    }
    if (tRoot.eType != D2Json::Object) {
        sError = "info.json: the top level must be an object";
        return false;
    }
    if (!StringField(tRoot, "name", true, tOut.sName, sError) || !StringField(tRoot, "version", true, tOut.sVersion, sError)
        || !StringField(tRoot, "title", false, tOut.sTitle, sError) || !StringField(tRoot, "author", false, tOut.sAuthor, sError)
        || !StringField(tRoot, "redivivus_version", false, tOut.sRedivivusVersion, sError)) {
        return false;
    }
    if (!D2LuaMods_IsValidName(tOut.sName)) {
        sError = "info.json: name \"" + tOut.sName + "\" is not valid (letters, digits, '_' and '-', at most 64)";
        return false;
    }
    if (!D2LuaMods_ParseVersion(tOut.sVersion, tOut.tVersion)) {
        sError = "info.json: version \"" + tOut.sVersion + "\" is not major.minor.patch";
        return false;
    }
    if (!tOut.sRedivivusVersion.empty()) {
        D2LuaVersion tWant;
        D2LuaVersion tHost;
        D2LuaMods_ParseVersion(D2LUA_REDIVIVUS_VERSION, tHost);
        if (!D2LuaMods_ParseVersion(tOut.sRedivivusVersion, tWant)) {
            sError = "info.json: redivivus_version \"" + tOut.sRedivivusVersion + "\" is not a version";
            return false;
        }
        tWant.a[2] = 0;
        if (D2LuaMods_CompareVersions(tWant, tHost) > 0) {
            sError = "mod '" + tOut.sName + "' needs Redivivus API " + tOut.sRedivivusVersion + ", this host offers "
                   + D2LUA_REDIVIVUS_VERSION;
            return false;
        }
    }
    const D2Json* pDeps = tRoot.Get("dependencies");
    bool bNamesBase = false;
    if (pDeps) {
        if (pDeps->eType != D2Json::Array) {
            sError = "info.json: \"dependencies\" must be an array of strings";
            return false;
        }
        for (const D2Json& tItem : pDeps->aItems) {
            if (tItem.eType != D2Json::String) {
                sError = "info.json: \"dependencies\" must be an array of strings";
                return false;
            }
            D2LuaDependency tDep;
            if (!D2LuaMods_ParseDependency(tItem.sValue, tDep, sError)) {
                sError = "info.json: " + sError;
                return false;
            }
            if (tDep.sName == tOut.sName) {
                sError = "info.json: a mod cannot depend on itself";
                return false;
            }
            bNamesBase = bNamesBase || tDep.sName == "base";
            tOut.aDependencies.push_back(tDep);
        }
    }
    if (tOut.sName != "base" && !bNamesBase) {
        D2LuaDependency tBase;
        tBase.sName = "base";
        tBase.sText = "base (implicit)";
        tOut.aDependencies.insert(tOut.aDependencies.begin(), tBase);
    }
    return true;
}

static bool IsDirectory(const std::string& sPath) {
    struct stat tStat;
    return stat(sPath.c_str(), &tStat) == 0 && S_ISDIR(tStat.st_mode);
}

void D2LuaMods_Discover(const std::vector<std::string>& aRoots, std::vector<D2LuaModInfo>& aOut,
                        std::vector<std::string>& aErrors) {
    std::map<std::string, std::string> tSeen;  // name -> path
    for (const std::string& sRoot : aRoots) {
        DIR* pDir = opendir(sRoot.c_str());
        if (!pDir) {
            aErrors.push_back("cannot open mods directory " + sRoot);
            continue;
        }
        std::vector<std::string> aNames;
        while (struct dirent* pEntry = readdir(pDir)) {
            if (pEntry->d_name[0] != '.') {
                aNames.push_back(pEntry->d_name);
            }
        }
        closedir(pDir);
        std::sort(aNames.begin(), aNames.end());
        for (const std::string& sEntry : aNames) {
            std::string sDir = sRoot + "/" + sEntry;
            if (!IsDirectory(sDir)) {
                continue;
            }
            struct stat tStat;
            if (stat((sDir + "/info.json").c_str(), &tStat) != 0) {
                continue;  // not a mod
            }
            D2LuaModInfo tInfo;
            std::string sError;
            if (!D2LuaMods_ReadInfo(sDir, tInfo, sError)) {
                aErrors.push_back(sDir + ": " + sError);
                continue;
            }
            if (sEntry != tInfo.sName && sEntry != tInfo.sName + "_" + tInfo.sVersion) {
                aErrors.push_back(sDir + ": the directory must be named '" + tInfo.sName + "' or '" + tInfo.sName + "_"
                                  + tInfo.sVersion + "'");
                continue;
            }
            auto it = tSeen.find(tInfo.sName);
            if (it != tSeen.end()) {
                aErrors.push_back("mod '" + tInfo.sName + "' found twice: " + it->second + " and " + sDir);
                continue;
            }
            tSeen[tInfo.sName] = sDir;
            aOut.push_back(tInfo);
        }
    }
}

static const char* OpText(D2LuaVersionOp eOp) {
    switch (eOp) {
    case D2LuaVersionOp::Less: return "<";
    case D2LuaVersionOp::LessEqual: return "<=";
    case D2LuaVersionOp::Equal: return "=";
    case D2LuaVersionOp::GreaterEqual: return ">=";
    case D2LuaVersionOp::Greater: return ">";
    default: return "";
    }
}

static std::string VersionText(const D2LuaVersion& v) {
    char szBuf[48];
    snprintf(szBuf, sizeof(szBuf), "%d.%d.%d", v.a[0], v.a[1], v.a[2]);
    return szBuf;
}

bool D2LuaMods_Resolve(const std::vector<D2LuaModInfo>& aMods, std::vector<D2LuaModInfo>& aOrdered,
                       std::vector<std::string>& aErrors) {
    aOrdered.clear();
    size_t nErrorsBefore = aErrors.size();
    std::map<std::string, size_t> tIndex;
    for (size_t i = 0; i < aMods.size(); i++) {
        if (!tIndex.emplace(aMods[i].sName, i).second) {
            aErrors.push_back("mod '" + aMods[i].sName + "' is listed twice");
        }
    }
    // Ordering edges: aAfter[i] holds the mods i must load after.
    std::vector<std::set<size_t>> aAfter(aMods.size());
    for (size_t i = 0; i < aMods.size(); i++) {
        const D2LuaModInfo& tMod = aMods[i];
        for (const D2LuaDependency& tDep : tMod.aDependencies) {
            auto it = tIndex.find(tDep.sName);
            bool bPresent = it != tIndex.end();
            if (tDep.eKind == D2LuaDepKind::Incompatible) {
                if (bPresent) {
                    aErrors.push_back("mod '" + tMod.sName + "' is incompatible with '" + tDep.sName
                                      + "', and both are enabled");
                }
                continue;
            }
            bool bRequired = tDep.eKind == D2LuaDepKind::Required || tDep.eKind == D2LuaDepKind::Unordered;
            if (!bPresent) {
                if (bRequired) {
                    aErrors.push_back("mod '" + tMod.sName + "' requires '" + tDep.sName + "', which is not installed");
                }
                continue;
            }
            const D2LuaModInfo& tOther = aMods[it->second];
            if (!VersionSatisfies(tOther.tVersion, tDep.eOp, tDep.tVersion)) {
                aErrors.push_back("mod '" + tMod.sName + "' requires '" + tDep.sName + " " + OpText(tDep.eOp) + " "
                                  + VersionText(tDep.tVersion) + "', but version " + tOther.sVersion + " is installed");
                continue;
            }
            if (tDep.eKind != D2LuaDepKind::Unordered) {
                aAfter[i].insert(it->second);
            }
        }
    }
    if (aErrors.size() != nErrorsBefore) {
        return false;
    }
    // Kahn's algorithm, smallest name first among the ready mods.
    std::vector<size_t> aPending(aMods.size());
    std::vector<std::vector<size_t>> aDependents(aMods.size());
    for (size_t i = 0; i < aMods.size(); i++) {
        aPending[i] = aAfter[i].size();
        for (size_t j : aAfter[i]) {
            aDependents[j].push_back(i);
        }
    }
    auto tByName = [&](size_t a, size_t b) { return aMods[a].sName < aMods[b].sName; };
    std::set<size_t, decltype(tByName)> tReady(tByName);
    for (size_t i = 0; i < aMods.size(); i++) {
        if (aPending[i] == 0) {
            tReady.insert(i);
        }
    }
    std::vector<bool> aDone(aMods.size(), false);
    while (!tReady.empty()) {
        size_t i = *tReady.begin();
        tReady.erase(tReady.begin());
        aDone[i] = true;
        aOrdered.push_back(aMods[i]);
        for (size_t d : aDependents[i]) {
            if (--aPending[d] == 0) {
                tReady.insert(d);
            }
        }
    }
    if (aOrdered.size() == aMods.size()) {
        return true;
    }
    // A cycle: walk dependencies among the remaining mods (smallest name first) until one
    // repeats; the repeat closes the cycle.
    size_t nStart = aMods.size();
    for (size_t i = 0; i < aMods.size(); i++) {
        if (!aDone[i] && (nStart == aMods.size() || aMods[i].sName < aMods[nStart].sName)) {
            nStart = i;
        }
    }
    std::vector<size_t> aPath;
    std::vector<int> aOnPath(aMods.size(), -1);
    size_t nAt = nStart;
    while (aOnPath[nAt] < 0) {
        aOnPath[nAt] = (int)aPath.size();
        aPath.push_back(nAt);
        size_t nNext = aMods.size();
        for (size_t j : aAfter[nAt]) {
            if (!aDone[j] && (nNext == aMods.size() || aMods[j].sName < aMods[nNext].sName)) {
                nNext = j;
            }
        }
        nAt = nNext;
    }
    std::string sCycle;
    for (size_t k = (size_t)aOnPath[nAt]; k < aPath.size(); k++) {
        sCycle += aMods[aPath[k]].sName + " -> ";
    }
    sCycle += aMods[nAt].sName;
    aErrors.push_back("dependency cycle: " + sCycle);
    aOrdered.clear();
    return false;
}

// The mods an error of D2LuaMods_Resolve is about: "mod 'x' ..." names x, a cycle names its members.
static std::vector<std::string> Culprits(const std::string& sError) {
    std::vector<std::string> aOut;
    static const char kCycle[] = "dependency cycle: ";
    if (sError.compare(0, 5, "mod '") == 0) {
        size_t nEnd = sError.find('\'', 5);
        if (nEnd != std::string::npos) {
            aOut.push_back(sError.substr(5, nEnd - 5));
        }
    } else if (sError.compare(0, sizeof(kCycle) - 1, kCycle) == 0) {
        std::string sRest = sError.substr(sizeof(kCycle) - 1);
        size_t nPos = 0;
        while (true) {
            size_t nArrow = sRest.find(" -> ", nPos);
            aOut.push_back(sRest.substr(nPos, nArrow == std::string::npos ? std::string::npos : nArrow - nPos));
            if (nArrow == std::string::npos) {
                break;
            }
            nPos = nArrow + 4;
        }
    }
    return aOut;
}

void D2LuaMods_ResolveEnabled(const std::vector<D2LuaModInfo>& aMods, std::vector<D2LuaModInfo>& aOrdered,
                              std::vector<std::string>& aDisabled) {
    std::vector<D2LuaModInfo> aLeft = aMods;
    while (true) {
        std::vector<std::string> aErrors;
        if (D2LuaMods_Resolve(aLeft, aOrdered, aErrors)) {
            return;
        }
        std::set<std::string> tOut;
        for (const std::string& sError : aErrors) {
            for (const std::string& sName : Culprits(sError)) {
                if (tOut.insert(sName).second) {
                    aDisabled.push_back("mod '" + sName + "' is disabled: " + sError);
                }
            }
        }
        std::vector<D2LuaModInfo> aNext;
        for (const D2LuaModInfo& t : aLeft) {
            if (!tOut.count(t.sName)) {
                aNext.push_back(t);
            }
        }
        if (aNext.size() == aLeft.size()) {
            for (const D2LuaModInfo& t : aLeft) {
                aDisabled.push_back("mod '" + t.sName + "' is disabled: the mods cannot be ordered");
            }
            aOrdered.clear();
            return;
        }
        aLeft.swap(aNext);
    }
}
#endif
