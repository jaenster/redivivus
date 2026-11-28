// Storm's Win32-on-Mac layer: path walking and fopen, 1.14d Intel Mac 000360c2..00036440
// (PathToFSSpecCreateDirs, FOpenResolveMacPath). The original file name is unknown; the
// folder is that of the layer (see Source/StormMac/README.md).
#include "Win32Mac.h"

#if !defined(_WIN32) && D2_MAC_CARBON

#include <Carbon/Carbon.h>
#include <stdio.h>
#include <string.h>

// 1.14d mac: 000360c2
// Walks the path's components with FSMakeFSSpec and PBGetCatInfo, creating the missing
// directories when bCreateDirs is set. Returns the leaf's FSSpec, whether it exists, and on
// failure the stage that failed and the component.
int PathToFSSpecCreateDirs(short nVRefNum, long dwDirID, const char* szPath, char cSeparator, char bCreateDirs,
                           FSSpec* pFSSpec, unsigned char* pbExists, int* pnFailStage, char* szFailComponent,
                           int nFailComponentSize) {
    short nResult = -50;
    int nFailStage;
    char szComponent[255];
    if (szPath != NULL && *szPath != '\0' && pFSSpec != NULL) {
        if (pbExists != NULL) {
            *pbExists = 0;
        }
        long dwCurDirID = dwDirID;
        for (;;) {
            while (*szPath == cSeparator) {
                szPath++;
            }
            const char* pSeparator = strchr(szPath, cSeparator);
            size_t nLen;
            if (pSeparator == NULL) {
                nLen = strlen(szPath);
            } else {
                nLen = pSeparator - szPath;
            }
            if (nLen > 0xFE) {
                nLen = 0xFE;
            }
            strncpy(szComponent, szPath, nLen);
            szComponent[nLen] = '\0';
            if (pSeparator == NULL) {
                Str255 abLeaf;
                memcpy(abLeaf + 1, szPath, nLen);
                abLeaf[0] = (unsigned char)nLen;
                short nErr = FSMakeFSSpec(nVRefNum, dwCurDirID, abLeaf, pFSSpec);
                if (nErr != 0) {
                    nFailStage = 6;
                    nResult = nErr;
                    if (nErr != fnfErr) {
                        goto fail;
                    }
                }
                nResult = 0;
                if (pbExists != NULL) {
                    *pbExists = nErr == 0;
                }
                break;
            }
            Str255 abDir;
            FSSpec tDirSpec;
            memcpy(abDir + 1, szPath, nLen);
            abDir[0] = (unsigned char)nLen;
            nResult = FSMakeFSSpec(nVRefNum, dwCurDirID, abDir, &tDirSpec);
            szPath = pSeparator;
            if (nResult == 0) {
                CInfoPBRec tInfo;
                Str63 abName;
                memcpy(abName, tDirSpec.name, tDirSpec.name[0] + 1);
                tInfo.dirInfo.ioFDirIndex = 0;
                tInfo.dirInfo.ioVRefNum = tDirSpec.vRefNum;
                tInfo.dirInfo.ioDrDirID = tDirSpec.parID;
                tInfo.dirInfo.ioNamePtr = abName;
                nResult = PBGetCatInfoSync(&tInfo);
                if (nResult != 0) {
                    nFailStage = 5;
                    goto fail;
                }
                dwCurDirID = tInfo.dirInfo.ioDrDirID;
                if ((tInfo.dirInfo.ioFlAttrib & 0x10) == 0) {
                    nFailStage = 4;
                    nResult = dupFNErr;
                    goto fail;
                }
                continue;
            }
            nFailStage = 2;
            if (nResult != fnfErr || (nResult = dirNFErr, bCreateDirs == 0)) {
                goto fail;
            }
            nResult = FSpDirCreate(&tDirSpec, smSystemScript, &dwCurDirID);
            if (nResult == 0) {
                continue;
            }
            nFailStage = 3;
            goto fail;
        }
        return nResult;
    fail:
        if (pnFailStage != NULL) {
            *pnFailStage = nFailStage;
        }
        if (szFailComponent != NULL && nFailComponentSize > 0) {
            strncpy(szFailComponent, szComponent, nFailComponentSize - 1);
            szFailComponent[nFailComponentSize - 1] = '\0';
        }
    }
    return nResult;
}

// Converts a Mac path token to a POSIX path; non-zero on failure.
int PATH_ToPosixPath(const char* szPath, char* szOut, int nOutSize);

// 1.14d mac: 000363ca
// fopen; a Mac path token is converted to a POSIX path first (NULL when that fails).
FILE* FOpenResolveMacPath(const char* szPath, const char* szMode) {
    char szPosixPath[1024];
    const char* szOpen = szPath;
    if (PATH_IsMacPathToken(szPath)) {
        szOpen = szPosixPath;
        if (PATH_ToPosixPath(szPath, szPosixPath, 0x400) != 0) {
            return NULL;
        }
    }
    return fopen(szOpen, szMode);
}

#endif  // D2_OS_MAC
