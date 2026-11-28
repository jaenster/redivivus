// Win32Posix: the registry, over an .ini file (Win32Mac's SRegMac keeps Storm's registry in the
// Mac preferences; this is the Win32 registry API itself, for the shared code's Windows path).
//
// The file is ~/Library/Application Support/Diablo II/Registry.ini (D2_REGISTRY names another):
// one section per key, by its full path ("HKEY_CURRENT_USER\Software\Blizzard Entertainment\
// Diablo II"), one line per value, typed: name=sz:text, name=dword:0000002a, name=hex:2a 00 ...
// for every other type (with its number, hex(3):...). The default value is named @. A key
// exists when its section or a subkey's does. The file is read once and written back on every
// change, so a crash loses nothing.
#if !defined(_WIN32) && D2_BACKEND_POSIX

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/stat.h>

#include "Source/StormMac/Win32Posix/Win32PosixKernelPriv.h"

using namespace Win32Posix;

namespace {

constexpr DWORD kRegSz = 1;
constexpr DWORD kRegExpandSz = 2;
constexpr DWORD kRegDword = 4;
constexpr LSTATUS kFileNotFound = 2;
constexpr LSTATUS kAccessDenied = 5;
constexpr LSTATUS kInvalidHandle = 6;
constexpr LSTATUS kInvalidParameter = 87;
constexpr LSTATUS kMoreData = 234;

struct KeyObject : Object {
    KeyObject() : Object(OBJECT_REGKEY) {}
    std::string sPath;
};

struct Registry {
    std::mutex tLock;
    IniFile tIni;
    std::string sPath;
};

Registry& Reg() {
    static Registry* pRegistry = [] {
        Registry* p = new Registry;
        const char* szEnv = getenv("D2_REGISTRY");
        if (szEnv && *szEnv) {
            p->sPath = szEnv;
        } else {
            const char* szHome = getenv("HOME");
            p->sPath = std::string(szHome ? szHome : "/tmp") + "/Library/Application Support/Diablo II/Registry.ini";
        }
        for (size_t i = 1; i < p->sPath.size(); ++i) {
            if (p->sPath[i] == '/') {
                mkdir(p->sPath.substr(0, i).c_str(), 0755);
            }
        }
        p->tIni.Load(p->sPath);
        return p;
    }();
    return *pRegistry;
}

// The path of a predefined key or an open one; empty when the handle is neither. The
// predefined keys are 0x8000000n, sign-extended or not.
std::string KeyPath(HKEY hKey) {
    uint64_t n = (uint64_t)(uintptr_t)hKey;
    if ((n >> 32) == 0 || (n >> 32) == 0xFFFFFFFFu) {
        switch ((uint32_t)n) {
            case 0x80000000u: return "HKEY_CLASSES_ROOT";
            case 0x80000001u: return "HKEY_CURRENT_USER";
            case 0x80000002u: return "HKEY_LOCAL_MACHINE";
            case 0x80000003u: return "HKEY_USERS";
            default: break;
        }
    }
    auto pKey = Handle_GetAs<KeyObject>((HANDLE)hKey, OBJECT_REGKEY);
    return pKey ? pKey->sPath : std::string();
}

std::string SubkeyPath(const std::string& sBase, LPCSTR lpSubKey) {
    std::string sPath = sBase;
    if (lpSubKey) {
        std::string sSub = lpSubKey;
        while (!sSub.empty() && sSub[0] == '\\') {
            sSub.erase(0, 1);
        }
        while (!sSub.empty() && sSub.back() == '\\') {
            sSub.pop_back();
        }
        if (!sSub.empty()) {
            sPath += "\\" + sSub;
        }
    }
    return sPath;
}

bool KeyExists(Registry& r, const std::string& sPath) {
    if (sPath.find('\\') == std::string::npos) {
        return true;  // a root
    }
    std::string sPrefix = sPath + "\\";
    for (const IniFile::Section& s : r.tIni.aSections) {
        if (strcasecmp(s.sName.c_str(), sPath.c_str()) == 0 ||
            strncasecmp(s.sName.c_str(), sPrefix.c_str(), sPrefix.size()) == 0) {
            return true;
        }
    }
    return false;
}

LSTATUS OpenKey(const std::string& sPath, HKEY* phkResult) {
    auto pKey = std::make_shared<KeyObject>();
    pKey->sPath = sPath;
    HANDLE h = Handle_Insert(pKey);
    if (!h) {
        return 4;
    }
    *phkResult = (HKEY)h;
    return 0;
}

std::string ValueName(LPCSTR lpValueName) {
    return lpValueName && *lpValueName ? std::string(lpValueName) : std::string("@");
}

std::string Encode(DWORD dwType, const BYTE* pData, DWORD cbData) {
    char sz[16];
    if ((dwType == kRegSz || dwType == kRegExpandSz) && cbData > 0) {
        std::string s((const char*)pData, cbData);
        size_t nNul = s.find('\0');
        if (nNul != std::string::npos && nNul == s.size() - 1) {
            s.pop_back();
            if (s.find_first_of("\r\n") == std::string::npos) {
                return (dwType == kRegSz ? "sz:" : "expand:") + s;
            }
        }
    }
    if (dwType == kRegDword && cbData == sizeof(DWORD)) {
        DWORD dw;
        memcpy(&dw, pData, sizeof(dw));
        snprintf(sz, sizeof(sz), "%08x", dw);
        return std::string("dword:") + sz;
    }
    std::string s = dwType == 3 ? "hex:" : (snprintf(sz, sizeof(sz), "hex(%x):", dwType), std::string(sz));
    for (DWORD i = 0; i < cbData; ++i) {
        snprintf(sz, sizeof(sz), i ? " %02x" : "%02x", pData[i]);
        s += sz;
    }
    return s;
}

bool Decode(const std::string& sValue, DWORD* pdwType, std::vector<BYTE>* pData) {
    pData->clear();
    if (sValue.compare(0, 3, "sz:") == 0 || sValue.compare(0, 7, "expand:") == 0) {
        *pdwType = sValue[0] == 's' ? kRegSz : kRegExpandSz;
        std::string s = sValue.substr(sValue.find(':') + 1);
        pData->assign(s.begin(), s.end());
        pData->push_back(0);
        return true;
    }
    if (sValue.compare(0, 6, "dword:") == 0) {
        *pdwType = kRegDword;
        DWORD dw = (DWORD)strtoul(sValue.c_str() + 6, nullptr, 16);
        pData->resize(sizeof(dw));
        memcpy(pData->data(), &dw, sizeof(dw));
        return true;
    }
    if (sValue.compare(0, 3, "hex") == 0) {
        size_t nColon = sValue.find(':');
        if (nColon == std::string::npos) {
            return false;
        }
        *pdwType = sValue[3] == '(' ? (DWORD)strtoul(sValue.c_str() + 4, nullptr, 16) : 3;  // REG_BINARY
        const char* p = sValue.c_str() + nColon + 1;
        while (*p) {
            char* pEnd;
            unsigned long n = strtoul(p, &pEnd, 16);
            if (pEnd == p) {
                break;
            }
            pData->push_back((BYTE)n);
            p = pEnd;
        }
        return true;
    }
    return false;
}

}  // namespace

LSTATUS RegOpenKeyExA(HKEY hKey, LPCSTR lpSubKey, DWORD ulOptions, DWORD samDesired, HKEY* phkResult) {
    (void)ulOptions;
    (void)samDesired;
    std::string sBase = KeyPath(hKey);
    if (sBase.empty()) {
        return kInvalidHandle;
    }
    if (!phkResult) {
        return kInvalidParameter;
    }
    Registry& r = Reg();
    std::lock_guard<std::mutex> tGuard(r.tLock);
    std::string sPath = SubkeyPath(sBase, lpSubKey);
    if (!KeyExists(r, sPath)) {
        return kFileNotFound;
    }
    return OpenKey(sPath, phkResult);
}

LSTATUS RegCreateKeyExA(HKEY hKey, LPCSTR lpSubKey, DWORD Reserved, LPSTR lpClass, DWORD dwOptions, DWORD samDesired,
                        LPSECURITY_ATTRIBUTES lpSecurityAttributes, HKEY* phkResult, LPDWORD lpdwDisposition) {
    (void)Reserved;
    (void)lpClass;
    (void)dwOptions;
    (void)samDesired;
    (void)lpSecurityAttributes;
    std::string sBase = KeyPath(hKey);
    if (sBase.empty()) {
        return kInvalidHandle;
    }
    if (!phkResult) {
        return kInvalidParameter;
    }
    Registry& r = Reg();
    std::lock_guard<std::mutex> tGuard(r.tLock);
    std::string sPath = SubkeyPath(sBase, lpSubKey);
    bool bExisted = KeyExists(r, sPath);
    if (!bExisted) {
        r.tIni.Get(sPath.c_str());
        if (!r.tIni.Save(r.sPath)) {
            return kAccessDenied;
        }
    }
    if (lpdwDisposition) {
        *lpdwDisposition = bExisted ? 2 : 1;  // REG_OPENED_EXISTING_KEY, REG_CREATED_NEW_KEY
    }
    return OpenKey(sPath, phkResult);
}

LSTATUS RegCloseKey(HKEY hKey) {
    if (Handle_GetAs<KeyObject>((HANDLE)hKey, OBJECT_REGKEY)) {
        Handle_Remove((HANDLE)hKey);
        return 0;
    }
    return KeyPath(hKey).empty() ? kInvalidHandle : 0;
}

LSTATUS RegQueryValueExA(HKEY hKey, LPCSTR lpValueName, LPDWORD lpReserved, LPDWORD lpType, LPBYTE lpData,
                         LPDWORD lpcbData) {
    (void)lpReserved;
    std::string sPath = KeyPath(hKey);
    if (sPath.empty()) {
        return kInvalidHandle;
    }
    if (lpData && !lpcbData) {
        return kInvalidParameter;
    }
    Registry& r = Reg();
    std::lock_guard<std::mutex> tGuard(r.tLock);
    IniFile::Section* pSection = r.tIni.Find(sPath.c_str());
    IniFile::Entry* pEntry = pSection ? IniFile::Find(*pSection, ValueName(lpValueName).c_str()) : nullptr;
    DWORD dwType;
    std::vector<BYTE> aData;
    if (!pEntry || !Decode(pEntry->sValue, &dwType, &aData)) {
        return kFileNotFound;
    }
    if (lpType) {
        *lpType = dwType;
    }
    if (!lpcbData) {
        return 0;
    }
    DWORD cbNeeded = (DWORD)aData.size();
    if (lpData && *lpcbData < cbNeeded) {
        *lpcbData = cbNeeded;
        return kMoreData;
    }
    if (lpData) {
        memcpy(lpData, aData.data(), cbNeeded);
    }
    *lpcbData = cbNeeded;
    return 0;
}

LSTATUS RegSetValueExA(HKEY hKey, LPCSTR lpValueName, DWORD Reserved, DWORD dwType, const BYTE* lpData, DWORD cbData) {
    (void)Reserved;
    std::string sPath = KeyPath(hKey);
    if (sPath.empty()) {
        return kInvalidHandle;
    }
    if (!lpData && cbData) {
        return kInvalidParameter;
    }
    Registry& r = Reg();
    std::lock_guard<std::mutex> tGuard(r.tLock);
    IniFile::Section& tSection = r.tIni.Get(sPath.c_str());
    std::string sName = ValueName(lpValueName);
    std::string sValue = Encode(dwType, lpData, cbData);
    if (IniFile::Entry* pEntry = IniFile::Find(tSection, sName.c_str())) {
        pEntry->sValue = sValue;
    } else {
        IniFile::Entry tEntry;
        tEntry.sKey = sName;
        tEntry.sValue = sValue;
        tSection.aEntries.push_back(tEntry);
    }
    return r.tIni.Save(r.sPath) ? 0 : kAccessDenied;
}

LSTATUS RegDeleteValueA(HKEY hKey, LPCSTR lpValueName) {
    std::string sPath = KeyPath(hKey);
    if (sPath.empty()) {
        return kInvalidHandle;
    }
    Registry& r = Reg();
    std::lock_guard<std::mutex> tGuard(r.tLock);
    IniFile::Section* pSection = r.tIni.Find(sPath.c_str());
    if (!pSection) {
        return kFileNotFound;
    }
    std::string sName = ValueName(lpValueName);
    auto& a = pSection->aEntries;
    for (size_t i = 0; i < a.size(); ++i) {
        if (!a[i].sKey.empty() && strcasecmp(a[i].sKey.c_str(), sName.c_str()) == 0) {
            a.erase(a.begin() + (long)i);
            return r.tIni.Save(r.sPath) ? 0 : kAccessDenied;
        }
    }
    return kFileNotFound;
}

LSTATUS RegFlushKey(HKEY hKey) {
    if (KeyPath(hKey).empty()) {
        return kInvalidHandle;
    }
    Registry& r = Reg();
    std::lock_guard<std::mutex> tGuard(r.tLock);
    return r.tIni.Save(r.sPath) ? 0 : kAccessDenied;
}

#endif  // !defined(_WIN32) && D2_BACKEND_POSIX
