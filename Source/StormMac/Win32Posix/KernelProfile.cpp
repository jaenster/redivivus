// Win32Posix: the private profile calls (D2.ini and the like) over an .ini file read the way
// Windows reads one. Win32Mac has none: the Mac game kept its settings in the registry stand-in
// (SRegMac) alone.
//
// A file name without a directory is looked for in the install directory, where Windows would
// look in its own. A value has its surrounding blanks and one pair of matching quotes removed.
// With no section name the call lists the sections, with no key the section's keys, each
// terminated and the list closed by a second terminator, as Windows lists them. Every call reads
// the file afresh, so an edit made while the game runs is seen.
#if !defined(_WIN32) && D2_BACKEND_POSIX

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

#include "Source/StormMac/Win32Posix/Win32PosixKernelPriv.h"

namespace {

std::string Trim(const std::string& s) {
    size_t nStart = s.find_first_not_of(" \t\r\n");
    if (nStart == std::string::npos) {
        return std::string();
    }
    size_t nEnd = s.find_last_not_of(" \t\r\n");
    return s.substr(nStart, nEnd - nStart + 1);
}

}  // namespace

namespace Win32Posix {

bool IniFile::Load(const std::string& sPath) {
    aPreamble.clear();
    aSections.clear();
    FILE* pFile = fopen(sPath.c_str(), "rb");
    if (!pFile) {
        return false;
    }
    std::string sText;
    char aBuffer[4096];
    size_t n;
    while ((n = fread(aBuffer, 1, sizeof(aBuffer), pFile)) > 0) {
        sText.append(aBuffer, n);
    }
    fclose(pFile);
    size_t nPos = 0;
    while (nPos < sText.size()) {
        size_t nEnd = sText.find('\n', nPos);
        if (nEnd == std::string::npos) {
            nEnd = sText.size();
        }
        std::string sLine = sText.substr(nPos, nEnd - nPos);
        if (!sLine.empty() && sLine.back() == '\r') {
            sLine.pop_back();
        }
        nPos = nEnd + 1;
        std::string sTrim = Trim(sLine);
        if (!sTrim.empty() && sTrim[0] == '[') {
            size_t nClose = sTrim.find(']');
            Section tSection;
            tSection.sName = Trim(sTrim.substr(1, nClose == std::string::npos ? std::string::npos : nClose - 1));
            aSections.push_back(tSection);
            continue;
        }
        Entry tEntry;
        tEntry.sLine = sLine;
        size_t nEq = sTrim.find('=');
        if (!sTrim.empty() && sTrim[0] != ';' && nEq != std::string::npos && nEq > 0) {
            tEntry.sKey = Trim(sTrim.substr(0, nEq));
            std::string sValue = Trim(sTrim.substr(nEq + 1));
            if (sValue.size() >= 2 && (sValue[0] == '"' || sValue[0] == '\'') && sValue.back() == sValue[0]) {
                sValue = sValue.substr(1, sValue.size() - 2);
            }
            tEntry.sValue = sValue;
        }
        (aSections.empty() ? aPreamble : aSections.back().aEntries).push_back(tEntry);
    }
    return true;
}

bool IniFile::Save(const std::string& sPath) const {
    std::string sText;
    for (const Entry& e : aPreamble) {
        sText += e.sLine + "\r\n";
    }
    for (const Section& s : aSections) {
        sText += "[" + s.sName + "]\r\n";
        for (const Entry& e : s.aEntries) {
            sText += (e.sKey.empty() ? e.sLine : e.sKey + "=" + e.sValue) + "\r\n";
        }
    }
    std::string sTemp = sPath + ".tmp";
    FILE* pFile = fopen(sTemp.c_str(), "wb");
    if (!pFile) {
        return false;
    }
    bool bOk = fwrite(sText.data(), 1, sText.size(), pFile) == sText.size();
    bOk = fclose(pFile) == 0 && bOk;
    return bOk && rename(sTemp.c_str(), sPath.c_str()) == 0;
}

IniFile::Section* IniFile::Find(const char* szSection) {
    for (Section& s : aSections) {
        if (strcasecmp(s.sName.c_str(), szSection) == 0) {
            return &s;
        }
    }
    return nullptr;
}

IniFile::Section& IniFile::Get(const char* szSection) {
    if (Section* p = Find(szSection)) {
        return *p;
    }
    Section tSection;
    tSection.sName = szSection;
    aSections.push_back(tSection);
    return aSections.back();
}

IniFile::Entry* IniFile::Find(Section& tSection, const char* szKey) {
    for (Entry& e : tSection.aEntries) {
        if (!e.sKey.empty() && strcasecmp(e.sKey.c_str(), szKey) == 0) {
            return &e;
        }
    }
    return nullptr;
}

}  // namespace Win32Posix

using namespace Win32Posix;

namespace {

std::string ProfilePath(LPCSTR lpFileName) {
    std::string sName = lpFileName ? lpFileName : "win.ini";
    if (sName.find_first_of("\\/") == std::string::npos) {
        return InstallDir() + "/" + sName;
    }
    return Path_ToPosix(sName.c_str());
}

std::mutex& ProfileLock() {
    static std::mutex* pLock = new std::mutex;
    return *pLock;
}

// Copies a list of terminated names, closed by a second terminator, cut to the buffer.
DWORD CopyList(const std::vector<std::string>& aNames, LPSTR lpOut, DWORD nSize) {
    if (nSize < 2) {
        if (nSize == 1) {
            lpOut[0] = '\0';
        }
        return 0;
    }
    DWORD n = 0;
    for (const std::string& s : aNames) {
        if (n + s.size() + 2 > nSize) {
            size_t nFit = nSize - n - 2;
            memcpy(lpOut + n, s.c_str(), nFit);
            n += (DWORD)nFit;
            lpOut[n] = '\0';
            lpOut[n + 1] = '\0';
            return nSize - 2;
        }
        memcpy(lpOut + n, s.c_str(), s.size() + 1);
        n += (DWORD)s.size() + 1;
    }
    lpOut[n] = '\0';
    if (n == 0) {
        lpOut[1] = '\0';
    }
    return n;
}

}  // namespace

DWORD GetPrivateProfileStringA(LPCSTR lpAppName, LPCSTR lpKeyName, LPCSTR lpDefault, LPSTR lpReturnedString,
                               DWORD nSize, LPCSTR lpFileName) {
    if (!lpReturnedString || nSize == 0) {
        return 0;
    }
    IniFile tIni;
    {
        std::lock_guard<std::mutex> tGuard(ProfileLock());
        tIni.Load(ProfilePath(lpFileName));
    }
    if (!lpAppName) {
        std::vector<std::string> aNames;
        for (const IniFile::Section& s : tIni.aSections) {
            aNames.push_back(s.sName);
        }
        return CopyList(aNames, lpReturnedString, nSize);
    }
    IniFile::Section* pSection = tIni.Find(lpAppName);
    if (!lpKeyName) {
        std::vector<std::string> aNames;
        if (pSection) {
            for (const IniFile::Entry& e : pSection->aEntries) {
                if (!e.sKey.empty()) {
                    aNames.push_back(e.sKey);
                }
            }
        }
        return CopyList(aNames, lpReturnedString, nSize);
    }
    IniFile::Entry* pEntry = pSection ? IniFile::Find(*pSection, lpKeyName) : nullptr;
    std::string sValue;
    if (pEntry) {
        sValue = pEntry->sValue;
    } else {
        sValue = lpDefault ? lpDefault : "";
        size_t nEnd = sValue.find_last_not_of(' ');
        sValue.resize(nEnd == std::string::npos ? 0 : nEnd + 1);
    }
    if (sValue.size() >= nSize) {
        memcpy(lpReturnedString, sValue.c_str(), nSize - 1);
        lpReturnedString[nSize - 1] = '\0';
        SetError(234);  // ERROR_MORE_DATA
        return nSize - 1;
    }
    memcpy(lpReturnedString, sValue.c_str(), sValue.size() + 1);
    return (DWORD)sValue.size();
}

// The value's leading decimal digits, signed, as Windows reads them; the default when the key
// is missing.
UINT GetPrivateProfileIntA(LPCSTR lpAppName, LPCSTR lpKeyName, INT nDefault, LPCSTR lpFileName) {
    char szValue[64];
    char szMissing[] = "\x01";
    GetPrivateProfileStringA(lpAppName, lpKeyName, szMissing, szValue, sizeof(szValue), lpFileName);
    if (strcmp(szValue, szMissing) == 0) {
        return (UINT)nDefault;
    }
    return (UINT)strtol(szValue, nullptr, 10);
}

// A null key removes the section, a null string the key.
BOOL WritePrivateProfileStringA(LPCSTR lpAppName, LPCSTR lpKeyName, LPCSTR lpString, LPCSTR lpFileName) {
    if (!lpAppName) {
        SetError(87);
        return FALSE;
    }
    std::lock_guard<std::mutex> tGuard(ProfileLock());
    std::string sPath = ProfilePath(lpFileName);
    IniFile tIni;
    tIni.Load(sPath);
    if (!lpKeyName) {
        for (size_t i = 0; i < tIni.aSections.size(); ++i) {
            if (strcasecmp(tIni.aSections[i].sName.c_str(), lpAppName) == 0) {
                tIni.aSections.erase(tIni.aSections.begin() + (long)i);
                break;
            }
        }
    } else if (!lpString) {
        if (IniFile::Section* pSection = tIni.Find(lpAppName)) {
            auto& a = pSection->aEntries;
            for (size_t i = 0; i < a.size(); ++i) {
                if (!a[i].sKey.empty() && strcasecmp(a[i].sKey.c_str(), lpKeyName) == 0) {
                    a.erase(a.begin() + (long)i);
                    break;
                }
            }
        }
    } else {
        IniFile::Section& tSection = tIni.Get(lpAppName);
        if (IniFile::Entry* pEntry = IniFile::Find(tSection, lpKeyName)) {
            pEntry->sValue = lpString;
        } else {
            IniFile::Entry tEntry;
            tEntry.sKey = lpKeyName;
            tEntry.sValue = lpString;
            // after the section's last key, ahead of the blank lines that close it
            auto& a = tSection.aEntries;
            size_t nAt = a.size();
            while (nAt > 0 && a[nAt - 1].sKey.empty() && Trim(a[nAt - 1].sLine).empty()) {
                --nAt;
            }
            a.insert(a.begin() + (long)nAt, tEntry);
        }
    }
    if (!tIni.Save(sPath)) {
        SetError(5);
        return FALSE;
    }
    return TRUE;
}

#endif  // !defined(_WIN32) && D2_BACKEND_POSIX
