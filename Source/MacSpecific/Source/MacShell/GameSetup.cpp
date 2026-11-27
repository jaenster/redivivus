// GameSetup.cpp - the Mac shell's game set-up: the system check (OS version, memory, threads,
// Sound Manager, DrawSprocket, disk space), the game archives, the CD key and the game disc,
// and the allocator it hands the set-up code.
//
// 1.14d Intel Mac (DiabloII_macho) 0019bed3..0019c7e4. Pinned by the 1.14d __FILE__
// ".../Diablo2/Source/DiabloAll/../MacSpecific/Source/Mac Shell/GameSetup.cpp" (0031a4da),
// passed to SMemAlloc/SMemFree by GAMESETUP_Alloc (line 463) and GAMESETUP_Free (line 486).
// FUN_0019bed3 takes the addresses of GAMESETUP_Alloc, GAMESETUP_Free and FUN_0019c7d9 with
// direct references (a function of another unit is reached through a non-lazy pointer), so it
// opens this unit: it calls MAC_GetDefaultDisplayMode, which clang would inline in one unit,
// out of line, and the unit has no jump table, so nothing aligns its start. It ends at the
// padding to 0019c7e4. See Source/MacSpecific/README.md.
//
// Mac-only: the guard is spelled `#if !defined(_WIN32) && D2_OS_MAC` (see
// Source/MacSpecific/README.md).
#include "D2Arch.h"

#if !defined(_WIN32) && D2_OS_MAC

#include <Carbon/Carbon.h>
#include <pthread.h>
#include <stdlib.h>

#include <QuickTime/QuickTime.h>
#include <CoreServices/CoreServices.h>
#include <stdio.h>
#include <string.h>

#include "Source/MacSpecific/Source/MacShell/QuickDrawMac.h"

#include "Source/StormMac/H/stpl.h"

typedef int BOOL;
// 1.14d mac: 002edc4a
BOOL D2CheckExpansionGame(void);
// 1.14d mac: 00018c44
int SMemFree(void* pMem, const char* szFile, int nLine, uint32_t dwFlags);
// 1.14d mac: 0019be00
void MAC_GetDefaultDisplayMode(int* pnWidth, int* pnHeight, int* pnBitDepth);
// 1.14d mac: 0002a6a3
uint32_t GetTempPathFreeSpace(void);
// 1.14d mac: 002eff85
void LOGFILE_WriteStringToFile(const char* szFormat, ...);
// 1.14d mac: 00033f9a
int GetApplicationInformation(void* pName, FSSpec* pFileSpec, uint32_t* pdwSignature, uint64_t* pPSN);
// 1.14d mac: 002ef3d4
void SFILE_InitializeFileSystem(int nArg1, int nArg2);
// 1.14d mac: 000136ea
int SFileFileExistsEx(void* hArchive, const char* szFileName, uint32_t dwSearchScope);
// 1.14d mac: 00014226
int SFileOpenArchive(const char* szArchiveName, int nPriority, uint32_t dwFlags, void** phArchive);
// 1.14d mac: 0001239c
int SFileCloseArchive(void* hArchive);
// 1.14d mac: 0001e564
int FPMessageBox(const char* szText, const char* szCaption, uint32_t dwType);
// 1.14d mac: 00304096
// Bink's BinkSetMemory (third party)
void BINK_SetMemory(void* (*pfnAlloc)(size_t), void (*pfnFree)(void*)) __asm__("_BinkSetMemory");
// 1.14d mac: 0005ad4d
void BNMESSQUEUE_Shutdown(void);
// The Battle.net client's callback table; only the two slots GAMESETUP calls are laid out.
struct BNetClientCallbacks {
    void (*pfnSetLoggingFlag)(void* pFlag);
    uint8_t unk04[0x90];
    int (*pfnDecodeCDKeys)(void);
};
// 1.14d mac: 0005ad31
BNetClientCallbacks* BNCLIENT_GetCallbacks(void);
// 1.14d mac: 0019ced5
void MAC_DisableMenus(void);
extern "C" OSStatus DSpStartup(void);
extern "C" uint32_t DSpGetVersion(void);
void* GAMESETUP_Alloc(size_t nSize);
void GAMESETUP_Free(void* pMemory);
int FUN_0019c7d9(void);
// The configuration Game/Main.cpp parses the command line into (its D2IniConfigStrc, 0x3CD
// bytes); only the bytes the shell uses are laid out.
struct MacDisplayState {
    uint8_t unk00[8];
    bool bFullScreen;
    uint8_t unk09;
    uint8_t b0A;
    uint8_t b0B;
    uint8_t b0C;
    uint8_t b0D;
    uint8_t unk0E[0x211 - 0x0E];
    void* pfnHook __attribute__((packed));
    uint8_t unk215[0x3CD - 0x215];
};
static_assert(sizeof(void*) != 4 || offsetof(MacDisplayState, pfnHook) == 0x211, "MacDisplayState::pfnHook");
static_assert(sizeof(void*) != 4 || sizeof(MacDisplayState) == 0x3CD, "MacDisplayState");
// 1.14d mac: 005c8660
extern MacDisplayState g_tMacDisplayState;
// 1.14d mac: 005c8a30
extern int g_nMacAppMode;
// 1.14d mac: 0019aad0
bool MAC_PromptForGameDisc(void);

// 1.14d mac: 0019bed3
// Checks the system (OS version, memory, threads, Sound Manager, DrawSprocket, disk space),
// then the game archives and the CD key, and asks for the game disc. 1 when the game may start.
Boolean FUN_0019bed3(void) {
    int nWidth;
    int nHeight;
    int nBitDepth;
    Str255 szError = {0};
    long nResponse;
    bool bRequirementsMet;
    MAC_GetDefaultDisplayMode(&nWidth, &nHeight, &nBitDepth);
    if (Gestalt(gestaltSystemVersion, &nResponse) == 0 && (nResponse & 0xFFFF) > 0x80F) {
        if ((nResponse & 0xFFFF) < 0x1012) {
            bRequirementsMet = false;
            if ((nResponse & 0xFFFF) < 0x1000) {
                GetIndString(szError, 201, 1);
            } else {
                GetIndString(szError, 201, 2);
            }
        } else if (Gestalt(gestaltPhysicalRAMSize, &nResponse) == 0 && (uint32_t)nResponse > 0x3FFFFFF) {
            if (Gestalt(gestaltThreadMgrAttr, &nResponse) == 0 && (nResponse & 5) == 5) {
                NumVersion tSndVersion = SndSoundManagerVersion();
                if ((*(uint32_t*)&tSndVersion >> 16) < 0x310) {
                    bRequirementsMet = false;
                    GetIndString(szError, 200, 4);
                } else {
                    uint32_t nDSpVersion = DSpGetVersion();
                    if ((nDSpVersion >> 16) < 0x173) {
                        LOGFILE_WriteStringToFile("CheckSystemRequirements: *** DSpGetVersion returned %.4x.", nDSpVersion >> 16);
                        bRequirementsMet = false;
                        GetIndString(szError, 200, 6);
                    } else {
                        short nErr = DSpStartup();
                        if (nErr == 0) {
                            FSSpec tAppSpec;
                            if (GetApplicationInformation(NULL, &tAppSpec, NULL, NULL) == 0) {
                                HParamBlockRec tParams;
                                memset(&tParams, 0, sizeof(tParams));
                                tParams.volumeParam.ioVRefNum = tAppSpec.vRefNum;
                                if (PBHGetVInfoSync(&tParams) == 0 && (tParams.volumeParam.ioVAtrb & 0x80800000) == 0) {
                                    if (GetTempPathFreeSpace() > 0x27FF) {
                                        bRequirementsMet = true;
                                        if ((uint32_t)(tParams.volumeParam.ioVFrBlk * tParams.volumeParam.ioVAlBlkSiz) < 0x2800) {
                                            bRequirementsMet = false;
                                            GetIndString(szError, 200, 10);
                                        }
                                    } else {
                                        bRequirementsMet = false;
                                        GetIndString(szError, 200, 9);
                                    }
                                } else {
                                    bRequirementsMet = false;
                                    GetIndString(szError, 200, 8);
                                }
                            } else {
                                bRequirementsMet = false;
                                GetIndString(szError, 200, 13);
                            }
                        } else {
                            LOGFILE_WriteStringToFile("CheckSystemRequirements: *** DSpStartup returned %d.", (int)nErr);
                            bRequirementsMet = false;
                            GetIndString(szError, 200, 6);
                        }
                    }
                }
            } else {
                bRequirementsMet = false;
                GetIndString(szError, 200, 3);
            }
        } else {
            bRequirementsMet = false;
            GetIndString(szError, 200, 11);
        }
    } else {
        bRequirementsMet = false;
        GetIndString(szError, 200, 1);
    }
    if (szError[0] != '\0') {
        ParamText(szError, "\p", "\p", "\p");
        StopAlert(0x211, NULL);
    }
    if (!bRequirementsMet) {
        return 0;
    }
    EnterMovies();
    SFILE_InitializeFileSystem(0, 0);
    void* hSounds = NULL;
    void* hExpansion = NULL;
    void* hPatch = NULL;
    void* hGraphics = NULL;
    void* hGameData = NULL;
    BOOL bExpansion = D2CheckExpansionGame();
    const char* szReason;
    if (!SFileOpenArchive("Diablo II Sounds", 0, 0, &hSounds)) {
        szReason = "missing Diablo II Sounds";
    fail:
        LOGFILE_WriteStringToFile("CheckCDKey: *** Failed: %s.", szReason);
        GetIndString(szError, 200, 13);
        ParamText(szError, "\p", "\p", "\p");
        StopAlert(0x211, NULL);
    } else {
        if (bExpansion) {
            if (!SFileOpenArchive("Diablo II Expansion Data", 0, 0, &hExpansion)) {
                szReason = "missing Diablo II Expansion Data";
                goto fail;
            }
            if (!SFileOpenArchive("Diablo II Graphics", 0, 0, &hGraphics)) {
                szReason = "missing Diablo II Graphics";
                goto fail;
            }
        }
        SFileOpenArchive("Diablo II Patch", 0, 0, &hPatch);
        SFileOpenArchive("Diablo II Game Data", 0, 0, &hGameData);
        BNetClientCallbacks* pCallbacks = BNCLIENT_GetCallbacks();
        *(BOOL*)&g_tMacDisplayState = bExpansion;
        pCallbacks->pfnSetLoggingFlag(&g_tMacDisplayState);
        if (!pCallbacks->pfnDecodeCDKeys()) {
            szReason = "invalid or missing CD-key";
            goto fail;
        }
        BNMESSQUEUE_Shutdown();
        SFileCloseArchive(hSounds);
        if (hExpansion != NULL) {
            SFileCloseArchive(hExpansion);
        }
        if (hPatch != NULL) {
            SFileCloseArchive(hPatch);
        }
        if (hGraphics != NULL) {
            SFileCloseArchive(hGraphics);
        }
        if (hGameData != NULL) {
            SFileCloseArchive(hGameData);
        }
        BINK_SetMemory((void* (*)(size_t))GAMESETUP_Alloc, GAMESETUP_Free);
        g_nMacAppMode = 4;
        g_tMacDisplayState.b0B = 1;
        g_tMacDisplayState.pfnHook = (void*)FUN_0019c7d9;
        MAC_DisableMenus();
        if (MAC_PromptForGameDisc()) {
            BOOL bGameExpansion = D2CheckExpansionGame();
            void* hPatchArchive = NULL;
            const char* szText;
            if (!SFileOpenArchive("Diablo II Patch", 0, 0, &hPatchArchive)) {
                szText = "The file \"Diablo II Patch\" cannot be opened.";
            } else {
                int nHasMarker = SFileFileExistsEx(hPatchArchive, "Data\\IAmExpansion", 0);
                SFileCloseArchive(hPatchArchive);
                if (bGameExpansion == (nHasMarker == 1)) {
                    return 1;
                }
                szText = "An incompatible Diablo II Patch data file has been detected. You have classic Diablo II installed, "
                         "but the patch data is for use with Diablo II: Lord of Destruction.";
                if (bGameExpansion) {
                    szText = "An incompatible Diablo II Patch data file has been detected. You have Diablo II: Lord of "
                             "Destruction installed, but the patch data is for use with classic Diablo II.";
                }
            }
            LOGFILE_WriteStringToFile("PatchMPQMatchesGameType: *** %s.", szText);
            GetIndString(szError, 200, 13);
            char szErrorText[256];
            CopyPascalStringToC(szError, szErrorText);
            FPMessageBox(szText, szErrorText, 0x20000010);
        }
    }
    return 0;
}

// 1.14d mac: 0019c760
// The allocator handed to the set-up code: never NULL, -1 on failure.
void* GAMESETUP_Alloc(size_t nSize) {
    void* pMemory = SMemAlloc(nSize, __FILE__, 463, 0);
    return pMemory ? pMemory : (void*)-1;
}

// 1.14d mac: 0019c79f
void GAMESETUP_Free(void* pMemory) {
    if (pMemory)
        SMemFree(pMemory, __FILE__, 486, 0);
}

// 1.14d mac: 0019c7d9
int FUN_0019c7d9(void) {
    return 1;
}

#endif  // D2_OS_MAC
