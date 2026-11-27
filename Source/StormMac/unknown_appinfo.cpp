// StormMac: application information, Internet Config, MoreFiles helpers and the user and
// machine names, 1.14d Intel Mac 00033cef..00034740 (GetFileTypeAndCreatorForPath ..
// GetDialogItemRect), followed by the file-saving and start-up code up to 00035545.
// The original file name is not recorded: no 1.14d string or static initialiser names it.
#include "Source/StormMac/Win32Mac/Source/Win32/Win32Mac.h"

#if !defined(_WIN32) && D2_OS_MAC

#include <Carbon/Carbon.h>
#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// The 10.7 SDK headers this layer builds against do not declare these QuickDraw calls.
extern "C" {
void InitCursor(void);
void GetPort(GrafPtr* pPort);
SInt16 GetPortTextFont(CGrafPtr pPort);
Style GetPortTextFace(CGrafPtr pPort);
SInt16 GetPortTextMode(CGrafPtr pPort);
SInt16 GetPortTextSize(CGrafPtr pPort);
void TextFont(SInt16 nFont);
void TextFace(StyleParameter nFace);
void TextMode(SInt16 nMode);
void TextSize(SInt16 nSize);
}

// 1.14d mac: 00033cef
// Maps the path's extension to a Mac file type; for some extensions the type depends on the
// creator, which may be rewritten. 'Temp' when there is no path or the extension is unknown.
DWORD GetFileTypeAndCreatorForPath(LPCSTR szPath, DWORD* pdwCreator) {
    if (szPath == NULL) {
        return 'Temp';
    }
    const char* szExtension = strrchr(szPath, '.');
    if (szExtension != NULL) {
        DWORD dwExtension = 0;
        int nIndex = 0;
        do {
            if ((unsigned char)szExtension[nIndex + 1] == 0) {
                break;
            }
            dwExtension = (dwExtension << 8) | toupper((unsigned char)szExtension[nIndex + 1]);
            nIndex++;
        } while (nIndex < 4);

        DWORD dwType;
        DWORD dwCreator;
        if (dwExtension > 0x4C4F46) {
            if (dwExtension < 0x4D5051) {
                if (dwExtension == 0x4C4F47) {
                    *pdwCreator = 'ttxt';
                    return 'TEXT';
                }
                return 'Temp';
            }
            if (dwExtension < 0x544D50) {
                if (dwExtension != 0x4D5051) {
                    return 'Temp';
                }
                dwCreator = *pdwCreator;
                if (dwCreator < 0x576F5721) {
                    if (dwCreator < 0x57617233) {
                        if (dwCreator < 0x5732424E && dwCreator < 0x53576172) {
                            if (dwCreator == 'Dbl2') {
                                return 'D2pq';
                            }
                            if (dwCreator == 'Dblo') {
                                return 'Dmpq';
                            }
                            if (dwCreator == 'FrTh') {
                                *pdwCreator = 'War3';
                                return 'Wmpq';
                            }
                        }
                    } else if (dwCreator == 'War3') {
                        return 'Wmpq';
                    }
                } else if (dwCreator == 'WoW!') {
                    return 'W!pq';
                }
                return 'Smpq';
            }
            if (dwExtension > 0x574148) {
                if (dwExtension != 0x574149) {
                    return 'Temp';
                }
                dwCreator = *pdwCreator;
                dwType = 'Wwai';
            } else {
                if (dwExtension < 0x545854) {
                    if (dwExtension != 0x544D50) {
                        return 'Temp';
                    }
                    dwCreator = *pdwCreator;
                    if (dwCreator > 0x57617232 || dwCreator > 0x5732424D || dwCreator > 0x53576171) {
                        return 'Temp';
                    }
                    if (dwCreator == 'Dbl2') {
                        return 'D2tt';
                    }
                    if (dwCreator != 'Dblo') {
                        if (dwCreator != 'FrTh') {
                            return 'Temp';
                        }
                        *pdwCreator = 'War3';
                        return 'Temp';
                    }
                    return 'DTmp';
                }
                if (dwExtension < 0x573347) {
                    if (dwExtension == 0x545854) {
                        *pdwCreator = 'ttxt';
                        return 'TEXT';
                    }
                    return 'Temp';
                }
                if (dwExtension > 0x573357) {
                    if (dwExtension != 0x573358) {
                        return 'Temp';
                    }
                    dwCreator = *pdwCreator;
                    dwType = 'Ww3x';
                } else if (dwExtension == 0x573347) {
                    dwCreator = *pdwCreator;
                    dwType = 'Ww3g';
                } else if (dwExtension == 0x57334D) {
                    dwCreator = *pdwCreator;
                    dwType = 'Ww3m';
                } else {
                    if (dwExtension != 0x57334E) {
                        return 'Temp';
                    }
                    dwCreator = *pdwCreator;
                    dwType = 'Ww3n';
                }
            }
        } else {
            if (dwExtension != 0x424E49) {
                return 'Temp';
            }
            dwCreator = *pdwCreator;
            dwType = 'Wmpq';
        }
        if (dwCreator == 'War3') {
            return dwType;
        }
        if (dwCreator == 'FrTh') {
            *pdwCreator = 'War3';
            return dwType;
        }
    }
    return 'Temp';
}

// Process serial number, name, signature and bundle file spec of the running application,
// cached by the first call.
// 1.14d mac: 003f4830
static BOOL s_bAppInfoCached;
// 1.14d mac: 003f4834
static DWORD s_dwAppSignature;
// 1.14d mac: 003f4838
static ProcessSerialNumber s_tAppPSN;
// 1.14d mac: 003f4840
static FSSpec s_tAppSpec;
// 1.14d mac: 003f4886
static Str255 s_szAppName;

// 1.14d mac: 00033f9a
int GetApplicationInformation(void* pName, FSSpec* pFileSpec, DWORD* pdwSignature, uint64_t* pPSN) {
    int nErr;
    if (!s_bAppInfoCached) {
        nErr = GetCurrentProcess(&s_tAppPSN);
        if (nErr != 0) {
            return nErr;
        }
        ProcessInfoRec tInfo;
        tInfo.processInfoLength = sizeof(tInfo);
        tInfo.processName = s_szAppName;
        tInfo.processAppSpec = &s_tAppSpec;
        nErr = GetProcessInformation(&s_tAppPSN, &tInfo);
        if (nErr != 0) {
            return nErr;
        }
        FSRef tBundle;
        FSSpec tSpec;
        if (GetProcessBundleLocation(&s_tAppPSN, &tBundle) == 0) {
            if (FSGetCatalogInfo(&tBundle, 0, NULL, NULL, &tSpec, NULL) == 0) {
                memcpy(&s_tAppSpec, &tSpec, sizeof(FSSpec));
            }
        }
        s_dwAppSignature = tInfo.processSignature;
        s_bAppInfoCached = 1;
    }
    if (pdwSignature != NULL) {
        *pdwSignature = s_dwAppSignature;
    }
    if (pPSN != NULL) {
        *pPSN = *(uint64_t*)&s_tAppPSN;
    }
    if (pFileSpec != NULL) {
        memcpy(pFileSpec, &s_tAppSpec, sizeof(FSSpec));
    }
    nErr = 0;
    if (pName != NULL) {
        memmove(pName, s_szAppName, s_szAppName[0] + 1);
    }
    return nErr;
}


// Internet Config instance, started on first use with the application's signature.
// 1.14d mac: 003f48c8
static BOOL s_bICStarted;
// 1.14d mac: 003f48cc
static ICInstance s_pICInstance;

// 1.14d mac: 000344af
void ReleaseICInstance(void);

// 1.14d mac: 00034428
ICInstance GetICInstance(void) {
    if (!s_bICStarted) {
        s_bICStarted = 1;
        DWORD dwSignature = 0x3F3F3F3F;
        GetApplicationInformation(NULL, NULL, &dwSignature, NULL);
        if (ICStart(&s_pICInstance, dwSignature) == 0) {
            atexit(ReleaseICInstance);
        } else {
            s_pICInstance = NULL;
        }
    }
    return s_pICInstance;
}

// 1.14d mac: 000344af
void ReleaseICInstance(void) {
    if (s_pICInstance != NULL) {
        ICStop(s_pICInstance);
        s_pICInstance = NULL;
    }
}

// 1.14d mac: 0003411f
// Parses the URL, checks that a helper preference exists for its scheme, optionally asks
// the callback whether to go on, and launches it. 0 on success, else a small status.
int LaunchURL(char* szURL, int nLength, int (*pfnConfirm)(void*), void* pArg) {
    Str255 szHelperKey = "\pHelper\xA5";
    int nResult = 1;
    Handle hURL = NULL;
    if (szURL != NULL && nLength > 0) {
        ICInstance pIC = GetICInstance();
        nResult = 2;
        if (pIC != NULL) {
            hURL = NewHandle(1);
            nResult = 4;
            if (hURL == NULL) {
                return nResult;
            }
            long nStart = 0;
            long nEnd = nLength;
            nResult = 4;
            unsigned int uErr = ICParseURL(pIC, "\p", szURL, nLength, &nStart, &nEnd, hURL);
            if (uErr < 0xFFFFFD5F) {
                if (uErr == 0) {
                    long nURLSize = GetHandleSize(hURL);
                    long nColon = 0;
                    for (;;) {
                        if (nColon >= nURLSize) {
                            nResult = 3;
                            goto done;
                        }
                        if ((*hURL)[nColon++] == ':') {
                            nColon--;
                            break;
                        }
                    }
                    if (nColon > 0 && nColon < nURLSize) {
                        memmove(szHelperKey + 8, *hURL, nColon);
                        szHelperKey[0] += (unsigned char)nColon;
                        ICAttr nAttr;
                        ICAppSpec tHelper;
                        long nPrefSize = sizeof(tHelper);
                        if (ICGetPref(pIC, szHelperKey, &nAttr, &tHelper, &nPrefSize) == 0) {
                            if (pfnConfirm != NULL && pfnConfirm(pArg) != 0) {
                                nResult = 6;
                                goto done;
                            }
                            nStart = 0;
                            nEnd = nURLSize;
                            uErr = ICLaunchURL(pIC, "\p", *hURL, nURLSize, &nStart, &nEnd);
                            if (uErr < 0xFFFFEC6C) {
                                nResult = 0;
                                if (uErr != 0) {
                                    nResult = 5;
                                }
                            } else if (uErr < 0xFFFFFD5F) {
                                nResult = (uErr == 0xFFFFEC6C) ? 3 : 5;
                            } else if (uErr > 0xFFFFFFCD) {
                                nResult = (uErr == 0xFFFFFFCE) ? 1 : 5;
                            } else if (uErr < 0xFFFFFF94) {
                                if (uErr < 0xFFFFFDA7) {
                                    switch (uErr) {
                                    case 0xFFFFFD5F:
                                        nResult = 1;
                                        break;
                                    case 0xFFFFFD62:
                                    case 0xFFFFFD64:
                                        nResult = 2;
                                        break;
                                    case 0xFFFFFD66:
                                        nResult = 3;
                                        break;
                                    default:
                                        nResult = 5;
                                        break;
                                    }
                                } else if (uErr != 0xFFFFFDA7) {
                                    nResult = 5;
                                }
                            } else if (uErr != 0xFFFFFF94) {
                                nResult = 5;
                            }
                            goto done;
                        }
                    }
                    nResult = 3;
                }
            } else if (uErr < 0xFFFFFDA7) {
                nResult = (uErr == 0xFFFFFD5F) ? 1 : 5;
            } else if (uErr != 0xFFFFFDA7 && uErr != 0xFFFFFF94) {
                nResult = (uErr == 0xFFFFFFCE) ? 1 : 5;
            }
        }
    }
done:
    DisposeHandle(hURL);
    return nResult;
}

// 1.14d mac: 00554e48
// The exit hook fp_fatal calls when the report says to quit; none means ExitToShell.
extern void (*g_pfnFatalExitHook)(void);
char FPvfatal(const char* szFormat, va_list pArgs);

// 1.14d mac: 00034519
// Reports a formatted fatal error; when the report says to quit, calls the exit hook or
// exits to the shell.
void fp_fatal(const char* szFormat, ...) {
    va_list pArgs;
    va_start(pArgs, szFormat);
    char bFatal = FPvfatal(szFormat, pArgs);
    va_end(pArgs);
    if (bFatal) {
        if (g_pfnFatalExitHook == NULL) {
            ExitToShell();
        }
        g_pfnFatalExitHook();
    }
}

// 1.14d mac: 000344dc
// ResolveAliasFileWithMountFlags, the no-UI flag mapped to the mount flags.
int MacResolveAliasFile(FSSpec* pFileSpec, Boolean bResolveChains, Boolean* pbTargetIsFolder, Boolean* pbWasAliased,
                        char bAllowUI) {
    return ResolveAliasFileWithMountFlags(pFileSpec, bResolveChains, pbTargetIsFolder, pbWasAliased,
                                          bAllowUI == 0);
}

// MoreFiles routines.
// 1.14d mac: 00034556
// PBHGetVInfoSync with the name buffer used only as input.
short GetVolumeInfoNoName(ConstStr255Param pVolName, short nVRefNum, HParamBlockRec* pParamBlock) {
    short nResult = -50;
    if (pParamBlock != NULL) {
        Str255 abVolName;
        pParamBlock->volumeParam.ioVRefNum = nVRefNum;
        if (pVolName == NULL) {
            pParamBlock->volumeParam.ioNamePtr = NULL;
            pParamBlock->volumeParam.ioVolIndex = 0;
        } else {
            memmove(abVolName, pVolName, *pVolName + 1);
            pParamBlock->volumeParam.ioNamePtr = abVolName;
            pParamBlock->volumeParam.ioVolIndex = -1;
        }
        nResult = PBHGetVInfoSync(pParamBlock);
        pParamBlock->volumeParam.ioNamePtr = NULL;
    }
    return nResult;
}

// 1.14d mac: 000345f0
// PBGetCatInfoSync for vRefNum/dirID/name without returning the name.
short GetCatInfoNoName(short nVRefNum, long dwDirID, ConstStr255Param pName, CInfoPBRec* pParamBlock) {
    Str31 abEmptyName;
    if (pName == NULL || *pName == 0) {
        abEmptyName[0] = 0;
        pParamBlock->dirInfo.ioNamePtr = abEmptyName;
        pParamBlock->dirInfo.ioFDirIndex = -1;
    } else {
        pParamBlock->dirInfo.ioNamePtr = (StringPtr)pName;
        pParamBlock->dirInfo.ioFDirIndex = 0;
    }
    pParamBlock->dirInfo.ioVRefNum = nVRefNum;
    pParamBlock->dirInfo.ioDrDirID = dwDirID;
    short nResult = PBGetCatInfoSync(pParamBlock);
    pParamBlock->dirInfo.ioNamePtr = NULL;
    return nResult;
}

// 1.14d mac: 00034666
short DetermineVRefNum(ConstStr255Param pVolName, short nVRefNum, short* pRealVRefNum) {
    HParamBlockRec tParamBlock;
    short nResult = GetVolumeInfoNoName(pVolName, nVRefNum, &tParamBlock);
    if (nResult == 0) {
        *pRealVRefNum = tParamBlock.volumeParam.ioVRefNum;
    }
    return nResult;
}

// 1.14d mac: 000346a3
short UnmountVolume(ConstStr255Param pVolName, short nVRefNum) {
    HParamBlockRec tParamBlock;
    short nResult = GetVolumeInfoNoName(pVolName, nVRefNum, &tParamBlock);
    if (nResult == 0) {
        tParamBlock.volumeParam.ioNamePtr = NULL;
        nResult = PBUnmountVol((ParamBlockRec*)&tParamBlock);
    }
    return nResult;
}

// 1.14d mac: 000346e7
// Catalog info: the isDirectory flag (attrib bit 4) and the directory ID.
short GetDirectoryID(short nVRefNum, long dwDirID, ConstStr255Param pName, long* pDirID, Boolean* pbIsDirectory) {
    CInfoPBRec tParamBlock;
    short nResult = GetCatInfoNoName(nVRefNum, dwDirID, pName, &tParamBlock);
    if (nResult == 0) {
        Boolean bIsDirectory = (tParamBlock.hFileInfo.ioFlAttrib >> 4) & 1;
        *pbIsDirectory = bIsDirectory;
        if (bIsDirectory) {
            tParamBlock.hFileInfo.ioDirID = tParamBlock.dirInfo.ioDrDirID;
        }
        *pDirID = tParamBlock.hFileInfo.ioDirID;
    }
    return nResult;
}

// 1.14d mac: 00034740
// GetDialogItem for the item's rect only.
void GetDialogItemRect(DialogPtr pDialog, short nItem, Rect* pRect) {
    if (pRect != NULL) {
        DialogItemType nItemType;
        Handle hItem;
        GetDialogItem(pDialog, nItem, &nItemType, &hItem, pRect);
    }
}

// 1.14d mac: 003f48d0
static long s_nSystemVersion;

// 1.14d mac: 0003477a
// CSCopyUserName, with a Gestalt('sysv') check that adds a CFRetain on 10.0.
CFStringRef MacCopyUserName(Boolean bUseShortName) {
    if (s_nSystemVersion == 0) {
        Gestalt(gestaltSystemVersion, &s_nSystemVersion);
    }
    UInt32 nSystemVersion = s_nSystemVersion;
    CFStringRef pUserName = CSCopyUserName(bUseShortName);
    if (nSystemVersion > 0xFFF && pUserName != NULL && s_nSystemVersion == 0x1000) {
        CFRetain(pUserName);
    }
    return pUserName;
}

// 1.14d mac: 000347e7
// CSCopyMachineName, or on 10.0.x ComputerName from the SystemConfiguration preferences.
CFTypeRef MacCopyMachineName(void) {
    if (s_nSystemVersion == 0) {
        Gestalt(gestaltSystemVersion, &s_nSystemVersion);
    }
    CFTypeRef pMachineName;
    if ((UInt32)s_nSystemVersion < 0x1000 || (UInt32)s_nSystemVersion > 0x100F) {
        pMachineName = CSCopyMachineName();
    } else {
        pMachineName = NULL;
        CFURLRef pUrl = CFURLCreateWithFileSystemPath(NULL, CFSTR("/var/db/SystemConfiguration/preferences.xml"),
                                                      kCFURLPOSIXPathStyle, false);
        if (pUrl != NULL) {
            CFDataRef pData;
            if (CFURLCreateDataAndPropertiesFromResource(NULL, pUrl, &pData, NULL, NULL, NULL)) {
                CFPropertyListRef pPlist = CFPropertyListCreateFromXMLData(kCFAllocatorSystemDefault, pData, 0, NULL);
                if (pPlist != NULL) {
                    CFDictionaryRef pDict = (CFDictionaryRef)CFDictionaryGetValue((CFDictionaryRef)pPlist, CFSTR("System"));
                    if (pDict != NULL) {
                        pDict = (CFDictionaryRef)CFDictionaryGetValue(pDict, CFSTR("System"));
                        if (pDict != NULL) {
                            pMachineName = CFDictionaryGetValue(pDict, CFSTR("ComputerName"));
                            if (pMachineName != NULL) {
                                CFRetain(pMachineName);
                            }
                        }
                    }
                    CFRelease(pPlist);
                }
                CFRelease(pData);
            }
            CFRelease(pUrl);
        }
    }
    return pMachineName;
}


// The application's vRefNum and directory, taken at start-up (an FSSpec whose name is empty).
// 1.14d mac: 003f48d4
static FSSpec s_tAppLocation;

// 1.14d mac: 00035246
void InitAppLocation(FSSpec* pAppLocation) {
    short nResult = GetApplicationInformation(NULL, pAppLocation, NULL, NULL);
    if (nResult != 0) {
        pAppLocation->parID = 0;
        pAppLocation->vRefNum = 0;
        pAppLocation->name[0] = 0;
    }
}

// 1.14d mac: 00035290
// Static initialiser of the application location.
void StaticInitAppLocation(void) {
    InitAppLocation(&s_tAppLocation);
}

// 'REV:' trailer, the last 32 bytes of a buffer: signature at -0x20, creation date -0x1c,
// flags -0x18, file type -0x14, creator -0x10, date -0x0c, time -0x08, all big-endian.
static DWORD ReadBE32(const BYTE* pData) {
    DWORD dwValue = *(const DWORD*)pData;
    return (dwValue << 24) | ((dwValue & 0xFF00) << 8) | ((dwValue >> 8) & 0xFF00) | (dwValue >> 24);
}

// 1.14d mac: 0003494a
// Creates the file the FSSpec names (an existing one is accepted), opens its data or
// resource fork, writes the buffer, sets the end of file and returns the OSErr (-1 on a
// short write). Writing the data fork with bParseTrailer set, a 'REV:' trailer supplies
// type, creator, dates and flags; without one type and creator come from the path.
int SaveFSSpecFromMem(FSSpec* pFSSpec, const BYTE* pBuffer, int nSize, char bResourceFork, char bParseTrailer) {
    short nResult;
    short nRefNum;
    DWORD dwTime = 0;
    DWORD dwDate = 0;
    DWORD dwCreationDate = 0;
    DWORD dwTrailerFlags = 0;
    DWORD dwFileType;
    DWORD dwCreator;
    bool bHasTrailer;
    int nWriteSize;
    unsigned char* pName = pFSSpec->name;

    HRstFLock(pFSSpec->vRefNum, pFSSpec->parID, pName);
    if (nSize < 0x20 || bParseTrailer != 1 || bResourceFork != 0 ||
        ReadBE32(pBuffer + nSize - 0x20) != 'REV:') {
        char szFileName[64];
        CopyPascalStringToC(pName, szFileName);
        dwCreator = 0x3F3F3F3F;
        dwFileType = GetFileTypeAndCreatorForPath(szFileName, &dwCreator);
        if (dwFileType == 'Temp') {
            dwFileType = 0x3F3F3F3F;
            dwCreator = 0x3F3F3F3F;
        }
        bHasTrailer = false;
        nWriteSize = nSize;
    } else {
        dwTrailerFlags = ReadBE32(pBuffer + nSize - 0x18);
        dwCreationDate = ReadBE32(pBuffer + nSize - 0x1C);
        dwTime = ReadBE32(pBuffer + nSize - 8) & 0xFFFFFF;
        dwDate = ReadBE32(pBuffer + nSize - 0xC) & 0xFFFFFF;
        dwCreator = ReadBE32(pBuffer + nSize - 0x10);
        dwFileType = ReadBE32(pBuffer + nSize - 0x14);
        BYTE bFlags = (BYTE)dwTrailerFlags;
        if ((bFlags & 2) == 0) {
            dwFileType = 0x3F3F3F3F;
        }
        nWriteSize = nSize - 0x20;
        if ((bFlags & 1) == 0) {
            nWriteSize = nSize;
        }
        if ((bFlags & 2) == 0) {
            dwCreator = 0x3F3F3F3F;
        }
        bHasTrailer = true;
    }
    nResult = FSpCreate(pFSSpec, dwCreator, dwFileType, -1);
    if (nResult == 0 || nResult == dupFNErr) {
        if (bResourceFork == 1) {
            nResult = FSpOpenRF(pFSSpec, fsRdWrPerm, &nRefNum);
        } else {
            nResult = FSpOpenDF(pFSSpec, fsRdWrPerm, &nRefNum);
        }
        if (nResult == 0) {
            long nBytesWritten = nWriteSize;
            nResult = FSWrite(nRefNum, &nBytesWritten, pBuffer);
            if (nResult == 0 && (nResult = SetEOF(nRefNum, nWriteSize)) == 0) {
                nResult = 0;
                if (nWriteSize != nBytesWritten) {
                    nResult = -1;
                }
            }
            FSClose(nRefNum);
            CInfoPBRec tInfo;
            if (bHasTrailer) {
                if ((dwTrailerFlags & 0xE) == 0) {
                    return nResult;
                }
                memset(&tInfo, 0, sizeof(tInfo));
                Str63 abName;
                memcpy(abName, pName, pName[0] + 1);
                tInfo.hFileInfo.ioVRefNum = pFSSpec->vRefNum;
                tInfo.hFileInfo.ioDirID = pFSSpec->parID;
                tInfo.hFileInfo.ioFDirIndex = 0;
                tInfo.hFileInfo.ioNamePtr = abName;
                if (PBGetCatInfoSync(&tInfo) != 0) {
                    return nResult;
                }
                if ((dwTrailerFlags & 2) != 0) {
                    tInfo.hFileInfo.ioFlFndrInfo.fdType = dwFileType;
                    tInfo.hFileInfo.ioFlFndrInfo.fdCreator = dwCreator;
                }
                if ((dwTrailerFlags & 4) != 0) {
                    DateTimeRec tDate;
                    UInt32 nSeconds;
                    tDate.year = (short)(dwDate >> 16) + 1900;
                    tDate.month = (dwDate >> 8) & 0xFF;
                    tDate.day = dwDate & 0xFF;
                    tDate.hour = (short)(dwTime >> 16);
                    tDate.minute = (dwTime >> 8) & 0xFF;
                    tDate.second = dwTime & 0xFF;
                    tDate.dayOfWeek = 0;
                    DateToSeconds(&tDate, &nSeconds);
                    tInfo.hFileInfo.ioFlCrDat = dwCreationDate;
                    tInfo.hFileInfo.ioFlMdDat = nSeconds;
                    tInfo.hFileInfo.ioFlBkDat = nSeconds;
                }
                if ((dwTrailerFlags & 8) != 0) {
                    tInfo.hFileInfo.ioFlFndrInfo.fdFlags = (short)(dwTrailerFlags >> 16);
                }
                tInfo.hFileInfo.ioDirID = pFSSpec->parID;
                PBSetCatInfoSync(&tInfo);
            } else {
                if (dwFileType == 0x3F3F3F3F) {
                    return nResult;
                }
                memset(&tInfo, 0, sizeof(tInfo));
                Str63 abName;
                memcpy(abName, pName, pName[0] + 1);
                tInfo.hFileInfo.ioVRefNum = pFSSpec->vRefNum;
                tInfo.hFileInfo.ioDirID = pFSSpec->parID;
                tInfo.hFileInfo.ioFDirIndex = 0;
                tInfo.hFileInfo.ioNamePtr = abName;
                if (PBGetCatInfoSync(&tInfo) != 0) {
                    return nResult;
                }
                tInfo.hFileInfo.ioFlFndrInfo.fdType = dwFileType;
                tInfo.hFileInfo.ioDirID = pFSSpec->parID;
                tInfo.hFileInfo.ioFlFndrInfo.fdCreator = dwCreator;
                PBSetCatInfoSync(&tInfo);
            }
        }
    }
    return nResult;
}

// 1.14d mac: 00034f87
// Walks the Pascal path at each ':', creating the directories FSMakeFSSpec reports missing,
// then saves the buffer.
int SaveFileFromMemCreateDirs(const unsigned char* pPath, const BYTE* pBuffer, int nSize, char bResourceFork,
                              char bUseTempFolder, char bParseTrailer) {
    short nVRefNum;
    long dwDirID;
    FSSpec tSpec;
    short nResult;
    if (bUseTempFolder == 0) {
        nVRefNum = s_tAppLocation.vRefNum;
        dwDirID = s_tAppLocation.parID;
    } else {
        nResult = FindFolder(kOnSystemDisk, kTemporaryFolderType, true, &nVRefNum, &dwDirID);
        if (nResult != 0) {
            return nResult;
        }
    }
    unsigned char nPathLen = *pPath;
    if (nPathLen > 1) {
        size_t nPos = 2;
        do {
            if (pPath[nPos] == ':' && pPath[nPos - 1] != ':') {
                unsigned char abPartialPath[256];
                memcpy(abPartialPath, pPath, nPos);
                abPartialPath[0] = (unsigned char)(nPos - 1);
                short nErr = FSMakeFSSpec(nVRefNum, dwDirID, abPartialPath, &tSpec);
                if (nErr == fnfErr) {
                    long dwNewDirID;
                    nErr = FSpDirCreate(&tSpec, smSystemScript, &dwNewDirID);
                    nResult = -120;
                    if (nErr == fnfErr) {
                        return nResult;
                    }
                }
                nResult = nErr;
                if (nResult != 0) {
                    goto check;
                }
            }
            nPos++;
        } while ((int)nPos <= (int)nPathLen);
    }
    nResult = FSMakeFSSpec(nVRefNum, dwDirID, pPath, &tSpec);
check:
    if (nResult == 0 || nResult == fnfErr) {
        nResult = SaveFSSpecFromMem(&tSpec, pBuffer, nSize, bResourceFork, bParseTrailer);
    }
    return nResult;
}

int PathToFSSpecCreateDirs(short nVRefNum, long dwDirID, const char* szPath, char cSeparator, char bCreateDirs,
                           FSSpec* pFSSpec, unsigned char* pbExists, int* pnFailStage, char* szFailComponent,
                           int nFailComponentSize);

// 1.14d mac: 0003513f
// Saves a buffer under a backslash-separated Windows path, creating its directories.
int SaveWinPathFileFromMem(const char* szPath, const BYTE* pBuffer, int nSize, char bResourceFork,
                           char bUseTempFolder, char bParseTrailer) {
    short nVRefNum;
    long dwDirID;
    FSSpec tSpec;
    short nResult;
    if (bUseTempFolder == 0) {
        nVRefNum = s_tAppLocation.vRefNum;
        dwDirID = s_tAppLocation.parID;
    } else {
        nResult = FindFolder(kOnSystemDisk, kTemporaryFolderType, true, &nVRefNum, &dwDirID);
        if (nResult != 0) {
            return nResult;
        }
    }
    nResult = PathToFSSpecCreateDirs(nVRefNum, dwDirID, szPath, '\\', 1, &tSpec, NULL, NULL, NULL, 0);
    if (nResult == 0 || nResult == fnfErr) {
        nResult = SaveFSSpecFromMem(&tSpec, pBuffer, nSize, bResourceFork, bParseTrailer);
    }
    return nResult;
}

// 1.14d mac: 000352af
// Saves the current port's text font, face, mode and size.
void GetTextState(short* pTextState) {
    if (pTextState != NULL) {
        GrafPtr pPort;
        GetPort(&pPort);
        if (pPort != NULL) {
            pTextState[0] = GetPortTextFont(pPort);
            pTextState[1] = GetPortTextFace(pPort) & 0xFF;
            pTextState[2] = GetPortTextMode(pPort);
            pTextState[3] = GetPortTextSize(pPort);
        }
    }
}

// 1.14d mac: 00035310
void SetTextState(const short* pTextState) {
    if (pTextState != NULL) {
        TextFont(pTextState[0]);
        TextFace(pTextState[1]);
        TextMode(pTextState[2]);
        TextSize(pTextState[3]);
    }
}

// 1.14d mac: 00035351
// A volume with a drive number that is locked, sized between 300 and 850 MiB and, by its
// volume parameters, a CD: 1, else 0.
BOOL STORM_IsCDVolume(unsigned short nVRefNum) {
    HParamBlockRec tVolInfo;
    unsigned char abVolName[256];
    char szVolName[256];
    memset(&tVolInfo, 0, sizeof(tVolInfo));
    tVolInfo.volumeParam.ioVRefNum = nVRefNum;
    tVolInfo.volumeParam.ioNamePtr = abVolName;
    BOOL bResult = 0;
    if (PBHGetVInfoSync(&tVolInfo) == 0) {
        CopyPascalStringToC(abVolName, szVolName);
        if (tVolInfo.volumeParam.ioVDrvInfo > 0 && (tVolInfo.volumeParam.ioVAtrb & 0x80800000) != 0 &&
            (tVolInfo.volumeParam.ioVAlBlkSiz >> 16) * tVolInfo.volumeParam.ioVNmAlBlks + 0xED400000 < 0x22600001) {
            GetVolParmsInfoBuffer tParms;
            HParamBlockRec tVolParms;
            memset(&tVolParms, 0, sizeof(tVolParms));
            tVolParms.ioParam.ioVRefNum = nVRefNum;
            tVolParms.ioParam.ioBuffer = (Ptr)&tParms;
            tVolParms.ioParam.ioReqCount = 0x20;
            PBHGetVolParmsSync(&tVolParms);
            if (tParms.vMVersion < 3 || (tParms.vMExtendedAttributes & 1) != 0) {
                bResult = 1;
            }
        }
    }
    return bResult;
}

void Mem_SetDefaultAllocators(void);
void WIN32MAC_Initialize(void);
void SLOG_InitCriticalSections(void);
void SMemInitialize(void);

// 1.14d mac: 000354bd
void StormMacInit(void) {
    InitCursor();
    FlushEvents(0xFFFF, 0);
    Mem_SetDefaultAllocators();
    WIN32MAC_Initialize();
    SLOG_InitCriticalSections();
    SMemInitialize();
}

void FPSetFatalToDebugger(int bValue);
void FPSetFatalSkipDebugStr(int bValue);
void SErrSuppressErrors(int bValue);
void StormDestroy(void);
void SREG_Destroy(void);
void SMemDestroy(void);
void SErrDestroy(void);
void SLogDestroy(void);
void WinDestroy(void);
void FUN_0001e320(void);
// 1.14d mac: 000354f4
void MyRuntimeTerminate(void) {
    FPSetFatalToDebugger(1);
    FPSetFatalSkipDebugStr(1);
    SErrSuppressErrors(1);
    StormDestroy();
    SREG_Destroy();
    SMemDestroy();
    SErrDestroy();
    SLogDestroy();
    WinDestroy();
    FUN_0001e320();
}

HANDLE GetModuleHandle(LPCSTR szModuleName);

// 1.14d mac: 00035545
void* StormGetInstance(void) {
    return (void*)GetModuleHandle(NULL);
}

#endif  // D2_OS_MAC
