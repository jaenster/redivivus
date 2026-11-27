// MacShell.h - the Mac shell entry points the shared game code calls from its D2_OS_MAC blocks.
//
// Mac-only: the guard is spelled `#if !defined(_WIN32) && D2_OS_MAC`.
#pragma once

#if !defined(_WIN32) && D2_OS_MAC

#include <stdio.h>

// 1.14d mac: 0019cf88
// Takes the pending Carbon events and posts them as Win32 messages.
void MAC_ProcessEvents(void);
// 1.14d mac: 0019ce5a
void MAC_EnableMenus(void);
// The text services' input hook and document (unknown_tsm.cpp).
void MAC_SetTSMInputCallback(void (*pfnCallback)(const char*, bool, int*));
void MAC_CreateTSMDocument(void);
// Between a text script (-0x80: the game language's) and UTF-16 (unknown_tsm.cpp).
void MAC_ConvertTextToUnicode(const char* pSrc, char* wszDest, int nSrcLength, int nDestSize, int nScript);
void MAC_ConvertUnicodeToText(const char* wszSrc, char* szDest, int nSrcChars, int nDestSize, int nScript);
// 1.14d mac: 0019cb78
void MAC_ApplyDisplayModuleSettings(void);
// 1.14d mac: 0019c7e4
void MAC_ResetVideoSettingsAndExit(void);
// 1.14d mac: 0030a904
bool MAC_GetSharedSupportPath(char* szBuffer, unsigned int nMaxLength);
// 1.14d mac: 0030a84a
bool MAC_SpawnProcess(const char* szPath);
// 1.14d mac: 002b7854
bool MACSEC_IsBlizzardSigned(const char* szPath);
// 1.14d mac: 000363ca
FILE* FOpenResolveMacPath(const char* szPath, const char* szMode);
// Fog's Blizzard support folder (Mac: the Application Support path), the Mac twin of
// SFILE_GetBlizzardAppDataPath.
namespace Fog::Src::ErrorManager {
bool FOG_MacGetBlizzardSupportPath(char* szBuffer, size_t nMaxLength);
}
using Fog::Src::ErrorManager::FOG_MacGetBlizzardSupportPath;

#endif
