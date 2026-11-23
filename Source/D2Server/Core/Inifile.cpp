// D2Server CORE\Inifile.cpp: realms.ini, read from the directory of the module.
// Reconstructed from the 1.00 D2Server.dll (2000-05-26); not part of the 1.14d byte
// match; not linked into Game.exe yet.
//
// realms.ini holds one [realm] section per realm. The section that applies is the
// first whose accept list covers this machine's address; reading stops at the next
// section after it.

#include <mbstring.h>

#include "Source/D2Server/Core/Inifile.h"
#include "Source/D2Server/Core/WinMain.h"
#include "Source/D2Server/D2ServerImports.h"

namespace D2Server::Core::Inifile {

using namespace D2Server::Imports;

enum {
    INI_KEYWORD_NONE,
    INI_KEYWORD_REALM,
    INI_KEYWORD_NAME,
    INI_KEYWORD_DESC,
    INI_KEYWORD_ADMINPASS,
    INI_KEYWORD_PUBLICPASS,
    INI_KEYWORD_BNETIP,
    INI_KEYWORD_MCPIP,
    INI_KEYWORD_CHARSERV,
    INI_KEYWORD_ACCEPT,
    INI_KEYWORD_MULTICLIENT,
    INI_KEYWORD_ANNOUNCE,
};

#pragma pack(push, 1)
struct IniKeywordEntry {
    int32_t nKeywordId;             // 0x00
    const char * pszKeyword;        // 0x04
};
#pragma pack(pop)

static const IniKeywordEntry g_aIniKeywords[11] = {                  // 1.00 D2Server: 10023b98
    { INI_KEYWORD_REALM, "realm" },
    { INI_KEYWORD_NAME, "name" },
    { INI_KEYWORD_DESC, "desc" },
    { INI_KEYWORD_ADMINPASS, "adminpass" },
    { INI_KEYWORD_PUBLICPASS, "publicpass" },
    { INI_KEYWORD_BNETIP, "bnetip" },
    { INI_KEYWORD_MCPIP, "mcpip" },
    { INI_KEYWORD_CHARSERV, "charserv" },
    { INI_KEYWORD_ACCEPT, "accept" },
    { INI_KEYWORD_MULTICLIENT, "multiclient" },
    { INI_KEYWORD_ANNOUNCE, "announce" },
};
static const int g_nIniKeywords = 11;                                 // 1.00 D2Server: 10023bf0

// The network-order mask of each accept-list prefix length, 0..32.
static const uint8_t g_aPrefixMasks[33][4] = {                        // 1.00 D2Server: 10023bf4
    { 0x00, 0x00, 0x00, 0x00 }, { 0x80, 0x00, 0x00, 0x00 }, { 0xC0, 0x00, 0x00, 0x00 }, { 0xE0, 0x00, 0x00, 0x00 },
    { 0xF0, 0x00, 0x00, 0x00 }, { 0xF8, 0x00, 0x00, 0x00 }, { 0xFC, 0x00, 0x00, 0x00 }, { 0xFE, 0x00, 0x00, 0x00 },
    { 0xFF, 0x00, 0x00, 0x00 }, { 0xFF, 0x80, 0x00, 0x00 }, { 0xFF, 0xC0, 0x00, 0x00 }, { 0xFF, 0xE0, 0x00, 0x00 },
    { 0xFF, 0xF0, 0x00, 0x00 }, { 0xFF, 0xF8, 0x00, 0x00 }, { 0xFF, 0xFC, 0x00, 0x00 }, { 0xFF, 0xFE, 0x00, 0x00 },
    { 0xFF, 0xFF, 0x00, 0x00 }, { 0xFF, 0xFF, 0x80, 0x00 }, { 0xFF, 0xFF, 0xC0, 0x00 }, { 0xFF, 0xFF, 0xE0, 0x00 },
    { 0xFF, 0xFF, 0xF0, 0x00 }, { 0xFF, 0xFF, 0xF8, 0x00 }, { 0xFF, 0xFF, 0xFC, 0x00 }, { 0xFF, 0xFF, 0xFE, 0x00 },
    { 0xFF, 0xFF, 0xFF, 0x00 }, { 0xFF, 0xFF, 0xFF, 0x80 }, { 0xFF, 0xFF, 0xFF, 0xC0 }, { 0xFF, 0xFF, 0xFF, 0xE0 },
    { 0xFF, 0xFF, 0xFF, 0xF0 }, { 0xFF, 0xFF, 0xFF, 0xF8 }, { 0xFF, 0xFF, 0xFF, 0xFC }, { 0xFF, 0xFF, 0xFF, 0xFE },
    { 0xFF, 0xFF, 0xFF, 0xFF },
};

// The value delimiters besides blanks: none (it is never filled in).
char g_szIniWhitespaceChars[8] = {};                                  // 1.00 D2Server: 10026d40
static char s_szKeywordDelims[] = "=";                                // 1.00 D2Server: 100245c8
static char s_szListDelims[] = ",";                                   // 1.00 D2Server: 10024270

static RealmIniInfo g_tRealmIni = {};                                 // 1.00 D2Server: 10026d48
static int32_t g_bInifileValid = 0;                                   // 1.00 D2Server: 10028e30

// 1.00 D2Server: 10005ba0
void __fastcall GetModuleDir(char * szPath) {
    GetModuleFileNameA(nullptr, szPath, MAX_PATH);
    szPath[MAX_PATH - 1] = '\0';
    while (*szPath && SStrChr(szPath + 1, '\\')) {
        szPath = SStrChr(szPath + 1, '\\');
    }
    *szPath = '\0';
}

// Reads a double-quoted value into pOut (the quotes are optional at the end of the
// line); returns the text after it, blanks skipped.
static char * IniReadQuoted(char * p, char * pOut) {
    p = SkipWhitespace(p, g_szIniWhitespaceChars);
    while (*p && *p != '"') {
        ++p;
    }
    if (*p == '"') {
        ++p;
    }
    while (*p && *p != '"') {
        *pOut++ = *p++;
    }
    if (*p == '"') {
        ++p;
    }
    *pOut = '\0';
    return SkipWhitespace(p, g_szIniWhitespaceChars);
}

// True when szNumber reads as a non-negative number no larger than nMax; *pnOut gets it.
static bool IniReadNumber(const char * szNumber, int nMax, int * pnOut) {
    int bDigit = szNumber[0] == '-' ? isdigit(szNumber[1]) : isdigit(szNumber[0]);
    if (!bDigit) {
        return false;
    }
    *pnOut = atoi(szNumber);
    return *pnOut >= 0 && *pnOut <= nMax;
}

// The VC6 build inlines the token readers below; they are the two helpers that follow
// ParseIniFile (SkipWhitespace, ParseAcceptLine), with the delimiters of each use.
// 1.00 D2Server: 10005cf0
int __cdecl ParseIniFile() {
    char szPath[MAX_PATH];
    char szLine[0x400];
    char szToken[0x400];
    char szNumber[0x400];
    char szPrefix[0x400];
    int bRealmSelected = 0;
    int nLine = 0;

    GetModuleDir(szPath);
    strcat(szPath, "\\");
    strcat(szPath, "realms.ini");
    WinMain::PostStatusMessage(1, (char *)"[INIFILE]  Opening intialization file: %s", szPath);
    FILE * pIniFile = fopen(szPath, "rt");
    if (!pIniFile) {
        WinMain::PostStatusMessage(1, (char *)"[INIFILE]  Unable to open initialization file: %s", szPath);
        return 0;
    }
    uint32_t dwLocalIp = inet_addr(FOG_SOCKET_GetPeerName());
    if (dwLocalIp == INADDR_NONE) {
        WinMain::PostStatusMessage(1, (char *)"[INIFILE]  Unable to get local ip address.");
        return 0;
    }
    memset(&g_tRealmIni, 0, sizeof(g_tRealmIni));

    while (!feof(pIniFile) && fgets(szLine, sizeof(szLine), pIniFile)) {
        ++nLine;
        char * p = ParseAcceptLine(szLine, szToken, s_szKeywordDelims);
        if (szToken[0] == '\0' || szToken[0] == ';') {
            continue;
        }
        char * pKeyword = szToken;
        if (szToken[0] == '[') {
            if (szToken[1] == '\0') {
                p = ParseAcceptLine(p, szToken, s_szKeywordDelims);
            } else {
                pKeyword = szToken + 1;
            }
            char * pClose = strchr(pKeyword, ']');
            if (!pClose) {
                WinMain::PostStatusMessage(1, (char *)"[INIFILE]  Error parsing line (%d) -- Missing close bracket in section header.", nLine);
                continue;
            }
            *pClose = '\0';
        }
        int nKeyword = INI_KEYWORD_NONE;
        for (int i = 0; i < g_nIniKeywords; ++i) {
            if (_mbscoll((const unsigned char *)pKeyword, (const unsigned char *)g_aIniKeywords[i].pszKeyword) == 0) {
                nKeyword = g_aIniKeywords[i].nKeywordId;
                break;
            }
        }

        switch (nKeyword) {
        case INI_KEYWORD_REALM:
            if (bRealmSelected) {
                goto done;
            }
            memset(&g_tRealmIni, 0, sizeof(g_tRealmIni));
            WinMain::PostStatusMessage(1, g_szIniWhitespaceChars);
            WinMain::PostStatusMessage(1, (char *)"[INIFILE]  KEYWORD:REALM");
            break;
        case INI_KEYWORD_NAME:
            IniReadQuoted(p, pKeyword);
            if (*pKeyword == '\0') {
                WinMain::PostStatusMessage(1, (char *)"[INIFILE]  Error parsing line (%d)  -- Unable to read realm name.", nLine);
            } else {
                SStrCopy(g_tRealmIni.szName, pKeyword, sizeof(g_tRealmIni.szName));
                WinMain::PostStatusMessage(1, (char *)"[INIFILE]  KEYWORD:NAME         VALUE:%s", g_tRealmIni.szName);
            }
            break;
        case INI_KEYWORD_DESC:
            IniReadQuoted(p, pKeyword);
            if (*pKeyword == '\0') {
                WinMain::PostStatusMessage(1, (char *)"[INIFILE]  Error parsing line (%d)  -- Unable to read realm description.", nLine);
            } else {
                SStrCopy(g_tRealmIni.szDescription, pKeyword, sizeof(g_tRealmIni.szDescription));
                WinMain::PostStatusMessage(1, (char *)"[INIFILE]  KEYWORD:DESC         VALUE:%s", g_tRealmIni.szDescription);
            }
            break;
        case INI_KEYWORD_ADMINPASS:
            IniReadQuoted(p, pKeyword);
            if (*pKeyword == '\0') {
                WinMain::PostStatusMessage(1, (char *)"[INIFILE]  Error parsing line (%d)  -- Unable to read admin password.", nLine);
            } else {
                SStrCopy(g_tRealmIni.szAdminPass, pKeyword, sizeof(g_tRealmIni.szAdminPass));
                WinMain::PostStatusMessage(1, (char *)"[INIFILE]  KEYWORD:ADMINPASS    VALUE:%s", g_tRealmIni.szAdminPass);
            }
            break;
        case INI_KEYWORD_PUBLICPASS:
            IniReadQuoted(p, pKeyword);
            if (*pKeyword == '\0') {
                // The binary reports the admin password here too.
                WinMain::PostStatusMessage(1, (char *)"[INIFILE]  Error parsing line (%d)  -- Unable to read admin password.", nLine);
            } else {
                SStrCopy(g_tRealmIni.szPublicPass, pKeyword, sizeof(g_tRealmIni.szPublicPass));
                WinMain::PostStatusMessage(1, (char *)"[INIFILE]  KEYWORD:PUBLICPASS   VALUE:%s", g_tRealmIni.szPublicPass);
            }
            break;
        case INI_KEYWORD_BNETIP:
            IniReadQuoted(p, pKeyword);
            if (*pKeyword == '\0') {
                WinMain::PostStatusMessage(1, (char *)"[INIFILE]  Error parsing line (%d)  -- Unable to read battle net ip.", nLine);
            } else {
                SStrCopy(g_tRealmIni.szBnetIp, pKeyword, sizeof(g_tRealmIni.szBnetIp));
                WinMain::PostStatusMessage(1, (char *)"[INIFILE]  KEYWORD:BNETIP       VALUE:%s", g_tRealmIni.szBnetIp);
            }
            break;
        case INI_KEYWORD_MCPIP: {
            ParseAcceptLine(p, pKeyword, g_szIniWhitespaceChars);
            uint32_t dwIp;
            if (*pKeyword == '\0' || (dwIp = inet_addr(pKeyword)) == INADDR_NONE) {
                WinMain::PostStatusMessage(1, (char *)"[INIFILE]  Error parsing line (%d)  -- Unable to read MCP ip.", nLine);
            } else {
                SStrCopy(g_tRealmIni.szMcpIp, pKeyword, sizeof(g_tRealmIni.szMcpIp));
                g_tRealmIni.dwMcpIpAddr = dwIp;
                in_addr tAddr;
                tAddr.s_addr = dwIp;
                WinMain::PostStatusMessage(1, (char *)"[INIFILE]  KEYWORD:MCPIP        VALUE:%s", inet_ntoa(tAddr));
            }
            break;
        }
        case INI_KEYWORD_CHARSERV: {
            BOOL bAny = FALSE;
            p = ParseAcceptLine(p, pKeyword, s_szListDelims);
            if (*pKeyword == '\0') {
                WinMain::PostStatusMessage(1, (char *)"[INIFILE]  Error parsing line (%d)  -- Unable to read battle net ip.", nLine);
                break;
            }
            while (*pKeyword != '\0') {
                if (*pKeyword == ';') {
                    if (!bAny) {
                        WinMain::PostStatusMessage(1, (char *)"[INIFILE]  Error parsing line (%d)  -- Unable to read battle net ip.", nLine);
                    }
                    break;
                }
                uint32_t dwIp = inet_addr(pKeyword);
                if (dwIp == INADDR_NONE) {
                    WinMain::PostStatusMessage(1, (char *)"[INIFILE]  Error parsing line (%d)  -- Unable to read character server ip.", nLine);
                    if (!bAny) {
                        WinMain::PostStatusMessage(1, (char *)"[INIFILE]  Error parsing line (%d)  -- Unable to read battle net ip.", nLine);
                    }
                    break;
                }
                bAny = TRUE;
                if (g_tRealmIni.nCharServers > 2) {
                    WinMain::PostStatusMessage(1, (char *)"[INIFILE]  Error parsing line (%d)  -- Exceeded maximum # of character servers.", nLine);
                    break;
                }
                g_tRealmIni.aCharServerIps[g_tRealmIni.nCharServers] = dwIp;
                ++g_tRealmIni.nCharServers;
                in_addr tAddr;
                tAddr.s_addr = dwIp;
                WinMain::PostStatusMessage(1, (char *)"[INIFILE]  KEYWORD:CSIP(%d)      VALUE:%s", g_tRealmIni.nCharServers - 1, inet_ntoa(tAddr));
                p = ParseAcceptLine(p, pKeyword, s_szListDelims);
            }
            break;
        }
        case INI_KEYWORD_ACCEPT: {
            BOOL bAny = FALSE;
            p = ParseAcceptLine(p, pKeyword, s_szListDelims);
            if (*pKeyword == '\0') {
                WinMain::PostStatusMessage(1, (char *)"[INIFILE]  Error parsing line (%d)  -- Unable to read battle net ip.", nLine);
                break;
            }
            while (*pKeyword != '\0') {
                int nPrefix = 32;
                if (*pKeyword == ';') {
                    if (!bAny) {
                        WinMain::PostStatusMessage(1, (char *)"[INIFILE]  Error parsing line (%d)  -- Unable to read battle net ip.", nLine);
                    }
                    break;
                }
                char * pSlash = strchr(pKeyword, '/');
                uint32_t dwIp = INADDR_NONE;
                if (pSlash) {
                    *pSlash = '\0';
                    SkipWhitespace(ParseAcceptLine(pSlash + 1, szPrefix, g_szIniWhitespaceChars), g_szIniWhitespaceChars);
                    if (!IniReadNumber(szPrefix, 32, &nPrefix)) {
                        goto bad_accept;
                    }
                }
                dwIp = inet_addr(pKeyword);
                if (dwIp == INADDR_NONE) {
                bad_accept:
                    WinMain::PostStatusMessage(1, (char *)"[INIFILE]  Error parsing line (%d)  -- Unable to read acceptlist ip.", nLine);
                    if (!bAny) {
                        WinMain::PostStatusMessage(1, (char *)"[INIFILE]  Error parsing line (%d)  -- Unable to read battle net ip.", nLine);
                    }
                    break;
                }
                bAny = TRUE;
                {
                    in_addr tAddr;
                    tAddr.s_addr = dwIp;
                    if (g_tRealmIni.nAcceptIps < 256) {
                        int i = 0;
                        for (; i < g_tRealmIni.nAcceptIps; ++i) {
                            if (g_tRealmIni.aAcceptIps[i].dwIpAddr == dwIp) {
                                break;
                            }
                        }
                        if (i == g_tRealmIni.nAcceptIps) {
                            g_tRealmIni.aAcceptIps[g_tRealmIni.nAcceptIps].dwIpAddr = dwIp;
                            g_tRealmIni.aAcceptIps[g_tRealmIni.nAcceptIps].nPrefixLen = nPrefix;
                            ++g_tRealmIni.nAcceptIps;
                        }
                    } else {
                        WinMain::PostStatusMessage(1, (char *)"Unable to add ip (%s) to accept list - maximum accept list size is %d", inet_ntoa(tAddr), 256);
                    }
                    WinMain::PostStatusMessage(1, (char *)"[INIFILE]  KEYWORD:ACCEPT       VALUE:%s/%d", inet_ntoa(tAddr), nPrefix);
                }
                p = ParseAcceptLine(p, pKeyword, s_szListDelims);
                for (int i = 0; i < g_tRealmIni.nAcceptIps; ++i) {
                    uint32_t dwMask;
                    memcpy(&dwMask, g_aPrefixMasks[g_tRealmIni.aAcceptIps[i].nPrefixLen], sizeof(dwMask));
                    if ((dwMask & dwLocalIp) == g_tRealmIni.aAcceptIps[i].dwIpAddr) {
                        bRealmSelected = 1;
                        WinMain::PostStatusMessage(1, (char *)"[INIFILE]  Selected realm.");
                        break;
                    }
                }
            }
            break;
        }
        case INI_KEYWORD_MULTICLIENT: {
            int nMulti;
            ParseAcceptLine(p, szNumber, g_szIniWhitespaceChars);
            if (!IniReadNumber(szNumber, 0x400, &nMulti)) {
                WinMain::PostStatusMessage(1, (char *)"[INIFILE]  Error parsing line (%d)  -- Invalid value for MULTICLIENT key.", nLine);
            } else {
                g_tRealmIni.nMaxMultiClient = nMulti;
                WinMain::PostStatusMessage(1, (char *)"[INIFILE]  KEYWORD:MULTICLIENT  VALUE:%d.", nMulti);
            }
            break;
        }
        case INI_KEYWORD_ANNOUNCE:
            IniReadQuoted(SkipWhitespace(p, g_szIniWhitespaceChars), pKeyword);
            if (*pKeyword == '\0') {
                WinMain::PostStatusMessage(1, (char *)"[INIFILE]  Error parsing line (%d)  -- Unable to read announcement.", nLine);
            } else if (g_tRealmIni.nAnnouncements < 20) {
                SStrCopy(g_tRealmIni.aAnnouncements[g_tRealmIni.nAnnouncements], pKeyword, 0x100);
                ++g_tRealmIni.nAnnouncements;
                WinMain::PostStatusMessage(1, (char *)"[INIFILE]  KEYWORD:ANNOUNCE     VALUE:%s", pKeyword);
            } else {
                WinMain::PostStatusMessage(1, (char *)"[INIFILE]  Error parsing line (%d)  -- Exceeded maximum # of announcements.", nLine);
            }
            break;
        default:
            WinMain::PostStatusMessage(1, (char *)"[INIFILE]  Error parsing line (%d) -- Unknown keyword (%s).", nLine, pKeyword);
            break;
        }
    }

done:
    fclose(pIniFile);
    WinMain::PostStatusMessage(1, (char *)"[INIFILE]  Succesfully read intialization file: %s", szPath);
    if (!bRealmSelected) {
        WinMain::PostStatusMessage(1, (char *)"[INIFILE]  Error -- Unable to find realm for this machine.");
        return 0;
    }
    if (g_tRealmIni.szName[0] == '\0') {
        WinMain::PostStatusMessage(1, (char *)"[INIFILE]  Error -- Realm info didn't contain name of realm.");
        return 0;
    }
    if (g_tRealmIni.szBnetIp[0] == '\0') {
        WinMain::PostStatusMessage(1, (char *)"[INIFILE]  Error -- Realm info didn't contain battle net ip.");
        return 0;
    }
    if (g_tRealmIni.nAcceptIps <= 0) {
        WinMain::PostStatusMessage(1, (char *)"[INIFILE]  Error -- Realm info missing accept list.");
        return 0;
    }
    if (g_tRealmIni.nCharServers <= 0) {
        WinMain::PostStatusMessage(1, (char *)"[INIFILE]  Error -- Realm info missing character server list.");
        return 0;
    }
    g_bInifileValid = 1;
    return 1;
}

// 1.00 D2Server: 10007390
char * __fastcall SkipWhitespace(char * pStr, char * pDelimiters) {
    while (*pStr && (isspace(*pStr) || strchr(pDelimiters, *pStr))) {
        ++pStr;
    }
    return pStr;
}

// The next token of pLine, lowercased, into pOut; the blanks and delimiters around it
// are skipped.
// 1.00 D2Server: 100073e0
char * __fastcall ParseAcceptLine(char * pLine, char * pOut, char * pDelimiters) {
    while (*pLine && (isspace(*pLine) || strchr(pDelimiters, *pLine))) {
        ++pLine;
    }
    while (*pLine && !isspace(*pLine) && !strchr(pDelimiters, *pLine)) {
        *pOut++ = (char)tolower(*pLine++);
    }
    *pOut = '\0';
    while (*pLine && (isspace(*pLine) || strchr(pDelimiters, *pLine))) {
        ++pLine;
    }
    return pLine;
}

// 1.00 D2Server: 10007560
char * __stdcall GetMcpIp() {
    return g_tRealmIni.szMcpIp;
}

// 1.00 D2Server: 10007580
int __stdcall GetMaxMultiClient() {
    return g_tRealmIni.nMaxMultiClient ? g_tRealmIni.nMaxMultiClient : 1;
}

// 1.00 D2Server: 100075a0
int __stdcall GetCharServerCount() {
    return g_tRealmIni.nCharServers;
}

// 1.00 D2Server: 100075c0
uint32_t __fastcall GetCharServerIp(int nIndex) {
    if (nIndex >= 0 && nIndex < 3) {
        return g_tRealmIni.aCharServerIps[nIndex];
    }
    FOG_DisplayAssert("i >= 0 && i < MAX_CHAR_SERV", "C:\\D2\\Source\\D2Server\\CORE\\Inifile.cpp", 0x30A);
    _exit(-1);
}

// Inserts a line into the announcement list at nIndex.
// 1.00 D2Server: 10007770
BOOL __fastcall StatusLog_InsertLine(int nIndex, char * szLine) {
    if (nIndex < 0 || nIndex > g_tRealmIni.nAnnouncements || g_tRealmIni.nAnnouncements >= 20) {
        return FALSE;
    }
    if (nIndex < g_tRealmIni.nAnnouncements - 1) {
        memmove(g_tRealmIni.aAnnouncements[nIndex + 1], g_tRealmIni.aAnnouncements[nIndex], (g_tRealmIni.nAnnouncements - nIndex) * 0x100);
    }
    SStrCopy(g_tRealmIni.aAnnouncements[nIndex], szLine, 0x100);
    ++g_tRealmIni.nAnnouncements;
    return TRUE;
}

} // namespace D2Server::Core::Inifile
