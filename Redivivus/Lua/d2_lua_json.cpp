// See d2_lua_json.h.
#if D2_FEATURE_MODS
#include "Redivivus/Lua/d2_lua_json.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

namespace {

struct Reader {
    const char* p;
    const char* pBegin;
    const char* pEnd;
    std::string* pError;
    int nDepth = 0;

    bool Fail(const char* pszWhy) {
        int nLine = 1;
        int nColumn = 1;
        for (const char* q = pBegin; q < p && q < pEnd; q++) {
            if (*q == '\n') {
                nLine++;
                nColumn = 1;
            } else {
                nColumn++;
            }
        }
        char szBuf[256];
        snprintf(szBuf, sizeof(szBuf), "line %d, column %d: %s", nLine, nColumn, pszWhy);
        *pError = szBuf;
        return false;
    }

    void SkipSpace() {
        while (p < pEnd && (*p == ' ' || *p == '\t' || *p == '\n' || *p == '\r')) {
            p++;
        }
    }

    bool Literal(const char* pszWord) {
        size_t n = strlen(pszWord);
        if ((size_t)(pEnd - p) < n || memcmp(p, pszWord, n) != 0) {
            return false;
        }
        p += n;
        return true;
    }

    static void PutUtf8(std::string& s, unsigned c) {
        if (c < 0x80) {
            s += (char)c;
        } else if (c < 0x800) {
            s += (char)(0xc0 | (c >> 6));
            s += (char)(0x80 | (c & 0x3f));
        } else if (c < 0x10000) {
            s += (char)(0xe0 | (c >> 12));
            s += (char)(0x80 | ((c >> 6) & 0x3f));
            s += (char)(0x80 | (c & 0x3f));
        } else {
            s += (char)(0xf0 | (c >> 18));
            s += (char)(0x80 | ((c >> 12) & 0x3f));
            s += (char)(0x80 | ((c >> 6) & 0x3f));
            s += (char)(0x80 | (c & 0x3f));
        }
    }

    bool Hex4(unsigned& c) {
        if (pEnd - p < 4) {
            return Fail("truncated \\u escape");
        }
        c = 0;
        for (int i = 0; i < 4; i++) {
            char h = *p++;
            c <<= 4;
            if (h >= '0' && h <= '9') {
                c |= (unsigned)(h - '0');
            } else if (h >= 'a' && h <= 'f') {
                c |= (unsigned)(h - 'a' + 10);
            } else if (h >= 'A' && h <= 'F') {
                c |= (unsigned)(h - 'A' + 10);
            } else {
                return Fail("bad hex digit in \\u escape");
            }
        }
        return true;
    }

    bool String(std::string& s) {
        p++;  // the opening quote
        while (true) {
            if (p >= pEnd) {
                return Fail("unterminated string");
            }
            unsigned char c = (unsigned char)*p;
            if (c == '"') {
                p++;
                return true;
            }
            if (c < 0x20) {
                return Fail("control character in string");
            }
            if (c != '\\') {
                s += (char)c;
                p++;
                continue;
            }
            p++;
            if (p >= pEnd) {
                return Fail("unterminated escape");
            }
            char e = *p++;
            switch (e) {
            case '"': s += '"'; break;
            case '\\': s += '\\'; break;
            case '/': s += '/'; break;
            case 'b': s += '\b'; break;
            case 'f': s += '\f'; break;
            case 'n': s += '\n'; break;
            case 'r': s += '\r'; break;
            case 't': s += '\t'; break;
            case 'u': {
                unsigned u = 0;
                if (!Hex4(u)) {
                    return false;
                }
                if (u >= 0xd800 && u < 0xdc00) {
                    unsigned v = 0;
                    if (pEnd - p < 2 || p[0] != '\\' || p[1] != 'u') {
                        return Fail("lone high surrogate");
                    }
                    p += 2;
                    if (!Hex4(v)) {
                        return false;
                    }
                    if (v < 0xdc00 || v >= 0xe000) {
                        return Fail("bad low surrogate");
                    }
                    u = 0x10000 + ((u - 0xd800) << 10) + (v - 0xdc00);
                } else if (u >= 0xdc00 && u < 0xe000) {
                    return Fail("lone low surrogate");
                }
                PutUtf8(s, u);
                break;
            }
            default:
                p--;
                return Fail("bad escape");
            }
        }
    }

    bool Number(double& n) {
        const char* q = p;
        if (q < pEnd && *q == '-') {
            q++;
        }
        if (q >= pEnd || !(*q >= '0' && *q <= '9')) {
            return Fail("bad number");
        }
        if (*q == '0') {
            q++;
        } else {
            while (q < pEnd && *q >= '0' && *q <= '9') {
                q++;
            }
        }
        if (q < pEnd && *q == '.') {
            q++;
            if (q >= pEnd || !(*q >= '0' && *q <= '9')) {
                return Fail("bad number");
            }
            while (q < pEnd && *q >= '0' && *q <= '9') {
                q++;
            }
        }
        if (q < pEnd && (*q == 'e' || *q == 'E')) {
            q++;
            if (q < pEnd && (*q == '+' || *q == '-')) {
                q++;
            }
            if (q >= pEnd || !(*q >= '0' && *q <= '9')) {
                return Fail("bad number");
            }
            while (q < pEnd && *q >= '0' && *q <= '9') {
                q++;
            }
        }
        std::string sNum(p, q);
        n = strtod(sNum.c_str(), nullptr);
        p = q;
        return true;
    }

    bool Value(D2Json& v) {
        SkipSpace();
        if (p >= pEnd) {
            return Fail("unexpected end of input");
        }
        if (++nDepth > 200) {
            return Fail("nested too deeply");
        }
        bool bOk = true;
        switch (*p) {
        case '{': {
            v.eType = D2Json::Object;
            p++;
            SkipSpace();
            if (p < pEnd && *p == '}') {
                p++;
                break;
            }
            while (true) {
                SkipSpace();
                if (p >= pEnd || *p != '"') {
                    return Fail("expected a member name");
                }
                std::string sKey;
                if (!String(sKey)) {
                    return false;
                }
                SkipSpace();
                if (p >= pEnd || *p != ':') {
                    return Fail("expected ':'");
                }
                p++;
                v.aMembers.emplace_back(std::move(sKey), D2Json());
                if (!Value(v.aMembers.back().second)) {
                    return false;
                }
                SkipSpace();
                if (p < pEnd && *p == ',') {
                    p++;
                    continue;
                }
                if (p < pEnd && *p == '}') {
                    p++;
                    break;
                }
                return Fail("expected ',' or '}'");
            }
            break;
        }
        case '[': {
            v.eType = D2Json::Array;
            p++;
            SkipSpace();
            if (p < pEnd && *p == ']') {
                p++;
                break;
            }
            while (true) {
                v.aItems.emplace_back();
                if (!Value(v.aItems.back())) {
                    return false;
                }
                SkipSpace();
                if (p < pEnd && *p == ',') {
                    p++;
                    continue;
                }
                if (p < pEnd && *p == ']') {
                    p++;
                    break;
                }
                return Fail("expected ',' or ']'");
            }
            break;
        }
        case '"':
            v.eType = D2Json::String;
            bOk = String(v.sValue);
            break;
        case 't':
            v.eType = D2Json::Bool;
            v.bValue = true;
            bOk = Literal("true") || Fail("bad literal");
            break;
        case 'f':
            v.eType = D2Json::Bool;
            bOk = Literal("false") || Fail("bad literal");
            break;
        case 'n':
            v.eType = D2Json::Null;
            bOk = Literal("null") || Fail("bad literal");
            break;
        default:
            v.eType = D2Json::Number;
            bOk = Number(v.nValue);
            break;
        }
        nDepth--;
        return bOk;
    }
};

}  // namespace

const D2Json* D2Json::Get(const char* pszKey) const {
    for (const auto& tMember : aMembers) {
        if (tMember.first == pszKey) {
            return &tMember.second;
        }
    }
    return nullptr;
}

bool D2Json_Parse(const char* pText, size_t nLen, D2Json& tOut, std::string& sError) {
    Reader tReader;
    tReader.p = pText;
    tReader.pBegin = pText;
    tReader.pEnd = pText + nLen;
    tReader.pError = &sError;
    // A UTF-8 byte order mark is tolerated.
    if (nLen >= 3 && memcmp(pText, "\xef\xbb\xbf", 3) == 0) {
        tReader.p += 3;
    }
    tOut = D2Json();
    if (!tReader.Value(tOut)) {
        return false;
    }
    tReader.SkipSpace();
    if (tReader.p != tReader.pEnd) {
        return tReader.Fail("trailing characters after the value");
    }
    return true;
}
#endif
