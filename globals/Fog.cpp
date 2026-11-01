/**
 * Auto-generated global data definitions
 * Binary: Game.exe
 */

#include "globals/globals.h"
#include "Fog/3rdparty/crashy/Report.h"
#include "Fog/D2MemoryManagerStrc.h"
#include "Fog/SMem.h"
#include "Fog/Src/LogManager.h"
#include "Storm/Source/SMem.h"
#include "Storm/Storm.h"


// =============================================================================
// Initialized data (arrays, structs, tables)
// =============================================================================

char cLogFileReadMode[2] = { 'r', '\x00' }; D2_GLOBAL_GUARD(cLogFileReadMode, "cLogFileReadMode");

uint32_t gdwBitBufferSeed[9] = {
    0x0,
    0x1,
    0x3,
    0x7,
    0xf,
    0x1f,
    0x3f,
    0x7f,
    0xff
}; D2_GLOBAL_GUARD(gdwBitBufferSeed, "gdwBitBufferSeed");

D2IniConfigValuesStrc D2IniConfigValues[58] = {
    {
        /* .szAppName = */ "DEBUG",
        /* .szKeyName = */ "CRASHAPP",
        /* .szCmdLineArg = */ "crash",
        /* .nType = */ 0x0,
        /* .field4_0x52 = */ 0x0,
        /* .field5_0x53 = */ 0x0,
        /* .nOffset = */ 0x4,
        /* .nDefaultValue = */ 0x0
    },
    {
        /* .szAppName = */ "VIDEO",
        /* .szKeyName = */ "WINDOW",
        /* .szCmdLineArg = */ "w",
        /* .nType = */ 0x0,
        /* .field4_0x52 = */ 0x0,
        /* .field5_0x53 = */ 0x0,
        /* .nOffset = */ 0x8,
        /* .nDefaultValue = */ 0x0
    },
    {
        /* .szAppName = */ "VIDEO",
        /* .szKeyName = */ "WINDOW",
        /* .szCmdLineArg = */ "window",
        /* .nType = */ 0x0,
        /* .field4_0x52 = */ 0x0,
        /* .field5_0x53 = */ 0x0,
        /* .nOffset = */ 0x8,
        /* .nDefaultValue = */ 0x0
    },
    {
        /* .szAppName = */ "VIDEO",
        /* .szKeyName = */ "WINDOW",
        /* .szCmdLineArg = */ "windowed",
        /* .nType = */ 0x0,
        /* .field4_0x52 = */ 0x0,
        /* .field5_0x53 = */ 0x0,
        /* .nOffset = */ 0x8,
        /* .nDefaultValue = */ 0x0
    },
    {
        /* .szAppName = */ "VIDEO",
        /* .szKeyName = */ "ASPECT",
        /* .szCmdLineArg = */ "nofixaspect",
        /* .nType = */ 0x0,
        /* .field4_0x52 = */ 0x0,
        /* .field5_0x53 = */ 0x0,
        /* .nOffset = */ 0x9,
        /* .nDefaultValue = */ 0x0
    },
    {
        /* .szAppName = */ "VIDEO",
        /* .szKeyName = */ "3DFX",
        /* .szCmdLineArg = */ "3dfx",
        /* .nType = */ 0x0,
        /* .field4_0x52 = */ 0x0,
        /* .field5_0x53 = */ 0x0,
        /* .nOffset = */ 0xa,
        /* .nDefaultValue = */ 0x0
    },
    {
        /* .szAppName = */ "VIDEO",
        /* .szKeyName = */ "OPENGL",
        /* .szCmdLineArg = */ "opengl",
        /* .nType = */ 0x0,
        /* .field4_0x52 = */ 0x0,
        /* .field5_0x53 = */ 0x0,
        /* .nOffset = */ 0xb,
        /* .nDefaultValue = */ 0x0
    },
    {
        /* .szAppName = */ "VIDEO",
        /* .szKeyName = */ "D3D",
        /* .szCmdLineArg = */ "d3d",
        /* .nType = */ 0x0,
        /* .field4_0x52 = */ 0x0,
        /* .field5_0x53 = */ 0x0,
        /* .nOffset = */ 0xd,
        /* .nDefaultValue = */ 0x0
    },
    {
        /* .szAppName = */ "VIDEO",
        /* .szKeyName = */ "RAVE",
        /* .szCmdLineArg = */ "rave",
        /* .nType = */ 0x0,
        /* .field4_0x52 = */ 0x0,
        /* .field5_0x53 = */ 0x0,
        /* .nOffset = */ 0xc,
        /* .nDefaultValue = */ 0x0
    },
    {
        /* .szAppName = */ "VIDEO",
        /* .szKeyName = */ "PERSPECTIVE",
        /* .szCmdLineArg = */ "per",
        /* .nType = */ 0x0,
        /* .field4_0x52 = */ 0x0,
        /* .field5_0x53 = */ 0x0,
        /* .nOffset = */ 0xe,
        /* .nDefaultValue = */ 0x0
    },
    {
        /* .szAppName = */ "VIDEO",
        /* .szKeyName = */ "QUALITY",
        /* .szCmdLineArg = */ "lq",
        /* .nType = */ 0x0,
        /* .field4_0x52 = */ 0x0,
        /* .field5_0x53 = */ 0x0,
        /* .nOffset = */ 0xf,
        /* .nDefaultValue = */ 0x0
    },
    {
        /* .szAppName = */ "VIDEO",
        /* .szKeyName = */ "GAMMA",
        /* .szCmdLineArg = */ "gamma",
        /* .nType = */ 0x1,
        /* .field4_0x52 = */ 0x0,
        /* .field5_0x53 = */ 0x0,
        /* .nOffset = */ 0x10,
        /* .nDefaultValue = */ 0x0
    },
    {
        /* .szAppName = */ "VIDEO",
        /* .szKeyName = */ "VSYNC",
        /* .szCmdLineArg = */ "vsync",
        /* .nType = */ 0x0,
        /* .field4_0x52 = */ 0x0,
        /* .field5_0x53 = */ 0x0,
        /* .nOffset = */ 0x14,
        /* .nDefaultValue = */ 0x0
    },
    {
        /* .szAppName = */ "VIDEO",
        /* .szKeyName = */ "FRAMERATE",
        /* .szCmdLineArg = */ "fr",
        /* .nType = */ 0x1,
        /* .field4_0x52 = */ 0x0,
        /* .field5_0x53 = */ 0x0,
        /* .nOffset = */ 0x15,
        /* .nDefaultValue = */ 0x0
    },
    {
        /* .szAppName = */ "NETWORK",
        /* .szKeyName = */ "SERVERIP",
        /* .szCmdLineArg = */ "s",
        /* .nType = */ 0x2,
        /* .field4_0x52 = */ 0x0,
        /* .field5_0x53 = */ 0x0,
        /* .nOffset = */ 0x37,
        /* .nDefaultValue = */ 0x0
    },
    {
        /* .szAppName = */ "NETWORK",
        /* .szKeyName = */ "GAMETYPE",
        /* .szCmdLineArg = */ "gametype",
        /* .nType = */ 0x1,
        /* .field4_0x52 = */ 0x0,
        /* .field5_0x53 = */ 0x0,
        /* .nOffset = */ 0x19,
        /* .nDefaultValue = */ 0x0
    },
    {
        /* .szAppName = */ "NETWORK",
        /* .szKeyName = */ "ARENA",
        /* .szCmdLineArg = */ "arena",
        /* .nType = */ 0x1,
        /* .field4_0x52 = */ 0x0,
        /* .field5_0x53 = */ 0x0,
        /* .nOffset = */ 0x207,
        /* .nDefaultValue = */ 0x0
    },
    {
        /* .szAppName = */ "NETWORK",
        /* .szKeyName = */ "JOINID",
        /* .szCmdLineArg = */ "joinid",
        /* .nType = */ 0x1,
        /* .field4_0x52 = */ 0x0,
        /* .field5_0x53 = */ 0x0,
        /* .nOffset = */ 0x1d,
        /* .nDefaultValue = */ 0x0
    },
    {
        /* .szAppName = */ "NETWORK",
        /* .szKeyName = */ "GAMENAME",
        /* .szCmdLineArg = */ "gamename",
        /* .nType = */ 0x2,
        /* .field4_0x52 = */ 0x0,
        /* .field5_0x53 = */ 0x0,
        /* .nOffset = */ 0x1f,
        /* .nDefaultValue = */ 0x0
    },
    {
        /* .szAppName = */ "NETWORK",
        /* .szKeyName = */ "BATTLENETIP",
        /* .szCmdLineArg = */ "bn",
        /* .nType = */ 0x2,
        /* .field4_0x52 = */ 0x0,
        /* .field5_0x53 = */ 0x0,
        /* .nOffset = */ 0x4f,
        /* .nDefaultValue = */ 0x0
    },
    {
        /* .szAppName = */ "NETWORK",
        /* .szKeyName = */ "MCPIP",
        /* .szCmdLineArg = */ "mcpip",
        /* .nType = */ 0x2,
        /* .field4_0x52 = */ 0x0,
        /* .field5_0x53 = */ 0x0,
        /* .nOffset = */ 0x67,
        /* .nDefaultValue = */ 0x0
    },
    {
        /* .szAppName = */ "CHARACTER",
        /* .szKeyName = */ "AMAZON",
        /* .szCmdLineArg = */ "ama",
        /* .nType = */ 0x0,
        /* .field4_0x52 = */ 0x0,
        /* .field5_0x53 = */ 0x0,
        /* .nOffset = */ 0x85,
        /* .nDefaultValue = */ 0x1
    },
    {
        /* .szAppName = */ "CHARACTER",
        /* .szKeyName = */ "PALADIN",
        /* .szCmdLineArg = */ "pal",
        /* .nType = */ 0x0,
        /* .field4_0x52 = */ 0x0,
        /* .field5_0x53 = */ 0x0,
        /* .nOffset = */ 0x86,
        /* .nDefaultValue = */ 0x0
    },
    {
        /* .szAppName = */ "CHARACTER",
        /* .szKeyName = */ "SORCERESS",
        /* .szCmdLineArg = */ "sor",
        /* .nType = */ 0x0,
        /* .field4_0x52 = */ 0x0,
        /* .field5_0x53 = */ 0x0,
        /* .nOffset = */ 0x87,
        /* .nDefaultValue = */ 0x0
    },
    {
        /* .szAppName = */ "CHARACTER",
        /* .szKeyName = */ "NECROMANCER",
        /* .szCmdLineArg = */ "nec",
        /* .nType = */ 0x0,
        /* .field4_0x52 = */ 0x0,
        /* .field5_0x53 = */ 0x0,
        /* .nOffset = */ 0x88,
        /* .nDefaultValue = */ 0x0
    },
    {
        /* .szAppName = */ "CHARACTER",
        /* .szKeyName = */ "BARBARIAN",
        /* .szCmdLineArg = */ "bar",
        /* .nType = */ 0x0,
        /* .field4_0x52 = */ 0x0,
        /* .field5_0x53 = */ 0x0,
        /* .nOffset = */ 0x89,
        /* .nDefaultValue = */ 0x0
    },
    {
        /* .szAppName = */ "CHARACTER",
        /* .szKeyName = */ "INVINCIBLE",
        /* .szCmdLineArg = */ "i",
        /* .nType = */ 0x0,
        /* .field4_0x52 = */ 0x0,
        /* .field5_0x53 = */ 0x0,
        /* .nOffset = */ 0x8c,
        /* .nDefaultValue = */ 0x0
    },
    {
        /* .szAppName = */ "CHARACTER",
        /* .szKeyName = */ "NAME",
        /* .szCmdLineArg = */ "name",
        /* .nType = */ 0x2,
        /* .field4_0x52 = */ 0x0,
        /* .field5_0x53 = */ 0x0,
        /* .nOffset = */ 0xbd,
        /* .nDefaultValue = */ 0x0
    },
    {
        /* .szAppName = */ "CHARACTER",
        /* .szKeyName = */ "REALM",
        /* .szCmdLineArg = */ "realm",
        /* .nType = */ 0x2,
        /* .field4_0x52 = */ 0x0,
        /* .field5_0x53 = */ 0x0,
        /* .nOffset = */ 0xd5,
        /* .nDefaultValue = */ 0x0
    },
    {
        /* .szAppName = */ "CHARACTER",
        /* .szKeyName = */ "CTEMP",
        /* .szCmdLineArg = */ "ctemp",
        /* .nType = */ 0x1,
        /* .field4_0x52 = */ 0x0,
        /* .field5_0x53 = */ 0x0,
        /* .nOffset = */ 0x1ed,
        /* .nDefaultValue = */ 0x0
    },
    {
        /* .szAppName = */ "MONSTER",
        /* .szKeyName = */ "NOMONSTERS",
        /* .szCmdLineArg = */ "nm",
        /* .nType = */ 0x0,
        /* .field4_0x52 = */ 0x0,
        /* .field5_0x53 = */ 0x0,
        /* .nOffset = */ 0x1f1,
        /* .nDefaultValue = */ 0x0
    },
    {
        /* .szAppName = */ "MONSTER",
        /* .szKeyName = */ "MONSTERCLASS",
        /* .szCmdLineArg = */ "m",
        /* .nType = */ 0x1,
        /* .field4_0x52 = */ 0x0,
        /* .field5_0x53 = */ 0x0,
        /* .nOffset = */ 0x1f2,
        /* .nDefaultValue = */ 0x0
    },
    {
        /* .szAppName = */ "MONSTER",
        /* .szKeyName = */ "MONSTERINFO",
        /* .szCmdLineArg = */ "minfo",
        /* .nType = */ 0x0,
        /* .field4_0x52 = */ 0x0,
        /* .field5_0x53 = */ 0x0,
        /* .nOffset = */ 0x1f6,
        /* .nDefaultValue = */ 0x0
    },
    {
        /* .szAppName = */ "MONSTER",
        /* .szKeyName = */ "MONSTERDEBUG",
        /* .szCmdLineArg = */ "md",
        /* .nType = */ 0x1,
        /* .field4_0x52 = */ 0x0,
        /* .field5_0x53 = */ 0x0,
        /* .nOffset = */ 0x1f7,
        /* .nDefaultValue = */ 0x0
    },
    {
        /* .szAppName = */ "ITEM",
        /* .szKeyName = */ "RARE",
        /* .szCmdLineArg = */ "rare",
        /* .nType = */ 0x0,
        /* .field4_0x52 = */ 0x0,
        /* .field5_0x53 = */ 0x0,
        /* .nOffset = */ 0x1fb,
        /* .nDefaultValue = */ 0x0
    },
    {
        /* .szAppName = */ "ITEM",
        /* .szKeyName = */ "UNIQUE",
        /* .szCmdLineArg = */ "unique",
        /* .nType = */ 0x0,
        /* .field4_0x52 = */ 0x0,
        /* .field5_0x53 = */ 0x0,
        /* .nOffset = */ 0x1fc,
        /* .nDefaultValue = */ 0x0
    },
    {
        /* .szAppName = */ "INTERFACE",
        /* .szKeyName = */ "ACT",
        /* .szCmdLineArg = */ "act",
        /* .nType = */ 0x1,
        /* .field4_0x52 = */ 0x0,
        /* .field5_0x53 = */ 0x0,
        /* .nOffset = */ 0x1ff,
        /* .nDefaultValue = */ 0x1
    },
    {
        /* .szAppName = */ "DEBUG",
        /* .szKeyName = */ "LOG",
        /* .szCmdLineArg = */ "log",
        /* .nType = */ 0x0,
        /* .field4_0x52 = */ 0x0,
        /* .field5_0x53 = */ 0x0,
        /* .nOffset = */ 0x216,
        /* .nDefaultValue = */ 0x0
    },
    {
        /* .szAppName = */ "DEBUG",
        /* .szKeyName = */ "MSGLOG",
        /* .szCmdLineArg = */ "msglog",
        /* .nType = */ 0x0,
        /* .field4_0x52 = */ 0x0,
        /* .field5_0x53 = */ 0x0,
        /* .nOffset = */ 0x217,
        /* .nDefaultValue = */ 0x0
    },
    {
        /* .szAppName = */ "DEBUG",
        /* .szKeyName = */ "SAFEMODE",
        /* .szCmdLineArg = */ "safe",
        /* .nType = */ 0x0,
        /* .field4_0x52 = */ 0x0,
        /* .field5_0x53 = */ 0x0,
        /* .nOffset = */ 0x218,
        /* .nDefaultValue = */ 0x0
    },
    {
        /* .szAppName = */ "DEBUG",
        /* .szKeyName = */ "NOSAVE",
        /* .szCmdLineArg = */ "nosave",
        /* .nType = */ 0x0,
        /* .field4_0x52 = */ 0x0,
        /* .field5_0x53 = */ 0x0,
        /* .nOffset = */ 0x219,
        /* .nDefaultValue = */ 0x0
    },
    {
        /* .szAppName = */ "DEBUG",
        /* .szKeyName = */ "SEED",
        /* .szCmdLineArg = */ "seed",
        /* .nType = */ 0x1,
        /* .field4_0x52 = */ 0x0,
        /* .field5_0x53 = */ 0x0,
        /* .nOffset = */ 0x21a,
        /* .nDefaultValue = */ 0x0
    },
    {
        /* .szAppName = */ "NETWORK",
        /* .szKeyName = */ "NOPK",
        /* .szCmdLineArg = */ "nopk",
        /* .nType = */ 0x0,
        /* .field4_0x52 = */ 0x0,
        /* .field5_0x53 = */ 0x0,
        /* .nOffset = */ 0x83,
        /* .nDefaultValue = */ 0x0
    },
    {
        /* .szAppName = */ "DEBUG",
        /* .szKeyName = */ "CHEATS",
        /* .szCmdLineArg = */ "cheats",
        /* .nType = */ 0x0,
        /* .field4_0x52 = */ 0x0,
        /* .field5_0x53 = */ 0x0,
        /* .nOffset = */ 0x21e,
        /* .nDefaultValue = */ 0x0
    },
    {
        /* .szAppName = */ "DEBUG",
        /* .szKeyName = */ "TEEN",
        /* .szCmdLineArg = */ "teen",
        /* .nType = */ 0x0,
        /* .field4_0x52 = */ 0x0,
        /* .field5_0x53 = */ 0x0,
        /* .nOffset = */ 0x21f,
        /* .nDefaultValue = */ 0x0
    },
    {
        /* .szAppName = */ "DEBUG",
        /* .szKeyName = */ "NOSOUND",
        /* .szCmdLineArg = */ "ns",
        /* .nType = */ 0x0,
        /* .field4_0x52 = */ 0x0,
        /* .field5_0x53 = */ 0x0,
        /* .nOffset = */ 0x220,
        /* .nDefaultValue = */ 0x0
    },
    {
        /* .szAppName = */ "FILEIO",
        /* .szKeyName = */ "NOPREDLOAD",
        /* .szCmdLineArg = */ "npl",
        /* .nType = */ 0x0,
        /* .field4_0x52 = */ 0x0,
        /* .field5_0x53 = */ 0x0,
        /* .nOffset = */ 0x203,
        /* .nDefaultValue = */ 0x0
    },
    {
        /* .szAppName = */ "FILEIO",
        /* .szKeyName = */ "DIRECT",
        /* .szCmdLineArg = */ "direct",
        /* .nType = */ 0x0,
        /* .field4_0x52 = */ 0x0,
        /* .field5_0x53 = */ 0x0,
        /* .nOffset = */ 0x204,
        /* .nDefaultValue = */ 0x0
    },
    {
        /* .szAppName = */ "FILEIO",
        /* .szKeyName = */ "LOWEND",
        /* .szCmdLineArg = */ "lem",
        /* .nType = */ 0x0,
        /* .field4_0x52 = */ 0x0,
        /* .field5_0x53 = */ 0x0,
        /* .nOffset = */ 0x205,
        /* .nDefaultValue = */ 0x0
    },
    {
        /* .szAppName = */ "DEBUG",
        /* .szKeyName = */ "QuEsTs",
        /* .szCmdLineArg = */ "questall",
        /* .nType = */ 0x0,
        /* .field4_0x52 = */ 0x0,
        /* .field5_0x53 = */ 0x0,
        /* .nOffset = */ 0x221,
        /* .nDefaultValue = */ 0x0
    },
    {
        /* .szAppName = */ "NETWORK",
        /* .szKeyName = */ "COMINT",
        /* .szCmdLineArg = */ "comint",
        /* .nType = */ 0x1,
        /* .field4_0x52 = */ 0x0,
        /* .field5_0x53 = */ 0x0,
        /* .nOffset = */ 0x225,
        /* .nDefaultValue = */ 0x0
    },
    {
        /* .szAppName = */ "NETWORK",
        /* .szKeyName = */ "SKIPTOBNET",
        /* .szCmdLineArg = */ "skiptobnet",
        /* .nType = */ 0x0,
        /* .field4_0x52 = */ 0x0,
        /* .field5_0x53 = */ 0x0,
        /* .nOffset = */ 0x35d,
        /* .nDefaultValue = */ 0x0
    },
    {
        /* .szAppName = */ "NETWORK",
        /* .szKeyName = */ "OPENC",
        /* .szCmdLineArg = */ "openc",
        /* .nType = */ 0x0,
        /* .field4_0x52 = */ 0x0,
        /* .field5_0x53 = */ 0x0,
        /* .nOffset = */ 0x84,
        /* .nDefaultValue = */ 0x0
    },
    {
        /* .szAppName = */ "FILEIO",
        /* .szKeyName = */ "NOCOMPRESS",
        /* .szCmdLineArg = */ "nocompress",
        /* .nType = */ 0x0,
        /* .field4_0x52 = */ 0x0,
        /* .field5_0x53 = */ 0x0,
        /* .nOffset = */ 0x206,
        /* .nDefaultValue = */ 0x0
    },
    {
        /* .szAppName = */ "TXT",
        /* .szKeyName = */ "TXT",
        /* .szCmdLineArg = */ "txt",
        /* .nType = */ 0x0,
        /* .field4_0x52 = */ 0x0,
        /* .field5_0x53 = */ 0x0,
        /* .nOffset = */ 0x215,
        /* .nDefaultValue = */ 0x0
    },
    {
        /* .szAppName = */ "BUILD",
        /* .szKeyName = */ "BUILD",
        /* .szCmdLineArg = */ "build",
        /* .nType = */ 0x0,
        /* .field4_0x52 = */ 0x0,
        /* .field5_0x53 = */ 0x0,
        /* .nOffset = */ 0x223,
        /* .nDefaultValue = */ 0x0
    },
    {
        /* .szAppName = */ "DEBUG",
        /* .szKeyName = */ "NOSOUND",
        /* .szCmdLineArg = */ "nosound",
        /* .nType = */ 0x0,
        /* .field4_0x52 = */ 0x0,
        /* .field5_0x53 = */ 0x0,
        /* .nOffset = */ 0x220,
        /* .nDefaultValue = */ 0x0
    },
    {
        /* .szAppName = */ "DEBUG",
        /* .szKeyName = */ "SOUNDBKG",
        /* .szCmdLineArg = */ "sndbkg",
        /* .nType = */ 0x0,
        /* .field4_0x52 = */ 0x0,
        /* .field5_0x53 = */ 0x0,
        /* .nOffset = */ 0x224,
        /* .nDefaultValue = */ 0x0
    }
}; D2_GLOBAL_GUARD(D2IniConfigValues, "D2IniConfigValues");

_SERVICE_STATUS lpServiceStatus_007065fc = {
    /* .dwServiceType = */ 0x10,
    /* .dwCurrentState = */ 0x4,
    /* .dwControlsAccepted = */ 0x5,
    /* .dwWin32ExitCode = */ 0x0,
    /* .dwServiceSpecificExitCode = */ 0x0,
    /* .dwCheckPoint = */ 0x0,
    /* .dwWaitHint = */ 0x0
}; D2_GLOBAL_GUARD(lpServiceStatus_007065fc, "lpServiceStatus_007065fc");

char * gaErrorTypeNames[12] = {
    s_No_Error_006ccbc8,
    s_Unknown_Error_006ccbb8,
    s_Out_of_Memory_Error_006ccba4,
    s_File_Read_Error_006ccb94,
    s_File_Write_Error_006ccb80,
    s_Assertion_Failure_006ccb6c,
    s_Halt_006ccb64,
    s_Warning_006ccb5c,
    s_Missing_Dll_006ccb50,
    s_Registry_Error_006ccb40,
    s_Deadlock_006ccb34,
    (char*)0x006ccb30
}; D2_GLOBAL_GUARD(gaErrorTypeNames, "gaErrorTypeNames");

uint8_t gabHuffmanCodeLengths[256] = {
    0x1,
    0x4,
    0x6,
    0x7,
    0x7,
    0x6,
    0x7,
    0x7,
    0x7,
    0x8,
    0x7,
    0x8,
    0x8,
    0x7,
    0x9,
    0x8,
    0x8,
    0x8,
    0x7,
    0x6,
    0x6,
    0x7,
    0x8,
    0x8,
    0x8,
    0x9,
    0x9,
    0xa,
    0xa,
    0x8,
    0x7,
    0x8,
    0x8,
    0xa,
    0xa,
    0xa,
    0xa,
    0x9,
    0xa,
    0xa,
    0x9,
    0xa,
    0xa,
    0xa,
    0xa,
    0xa,
    0xa,
    0xb,
    0x9,
    0x9,
    0xa,
    0xa,
    0xa,
    0xa,
    0xa,
    0xa,
    0xa,
    0xa,
    0xa,
    0x9,
    0xa,
    0xa,
    0xa,
    0xa,
    0x9,
    0xa,
    0xa,
    0x9,
    0xa,
    0x9,
    0xa,
    0xa,
    0xa,
    0xa,
    0xa,
    0xa,
    0x9,
    0x9,
    0xa,
    0xb,
    0x9,
    0x8,
    0x9,
    0xa,
    0xb,
    0x9,
    0x9,
    0xa,
    0x9,
    0xa,
    0xa,
    0xa,
    0xb,
    0xa,
    0xa,
    0xa,
    0xa,
    0xa,
    0x9,
    0x9,
    0x9,
    0xa,
    0xa,
    0x7,
    0x7,
    0x7,
    0xa,
    0x9,
    0x8,
    0x7,
    0xa,
    0xa,
    0xa,
    0xb,
    0xa,
    0xa,
    0xa,
    0xa,
    0xa,
    0xa,
    0xa,
    0xa,
    0xa,
    0xb,
    0xa,
    0xa,
    0xa,
    0xa,
    0x7,
    0xa,
    0xa,
    0xa,
    0xb,
    0xb,
    0xb,
    0xb,
    0xb,
    0xa,
    0xa,
    0xb,
    0xb,
    0xb,
    0xb,
    0xa,
    0x9,
    0xb,
    0xb,
    0xa,
    0xb,
    0x9,
    0x9,
    0x9,
    0xa,
    0xb,
    0xb,
    0xb,
    0xb,
    0xb,
    0xb,
    0xb,
    0xa,
    0xb,
    0xa,
    0xb,
    0xb,
    0xb,
    0xb,
    0xb,
    0xb,
    0xb,
    0xb,
    0xb,
    0xb,
    0xb,
    0xb,
    0xb,
    0xa,
    0xb,
    0xb,
    0xb,
    0xb,
    0xb,
    0xb,
    0xb,
    0xa,
    0xb,
    0xb,
    0xb,
    0xb,
    0xb,
    0xb,
    0xb,
    0xa,
    0xb,
    0xa,
    0xb,
    0xb,
    0xb,
    0xb,
    0xb,
    0xb,
    0xb,
    0xb,
    0xb,
    0xb,
    0xb,
    0xa,
    0xb,
    0xb,
    0xb,
    0xb,
    0xb,
    0xb,
    0xb,
    0xb,
    0xb,
    0xb,
    0xb,
    0xb,
    0xb,
    0xb,
    0xb,
    0xb,
    0xb,
    0xa,
    0xb,
    0xa,
    0xb,
    0xb,
    0xb,
    0xb,
    0xb,
    0xa,
    0xb,
    0xb,
    0xb,
    0xb,
    0xb,
    0xb,
    0xb,
    0xa,
    0xb,
    0xb,
    0xb,
    0xb,
    0xb,
    0xb,
    0xb,
    0xa,
    0xb,
    0xb,
    0xb,
    0xb,
    0xb,
    0xb,
    0x6
}; D2_GLOBAL_GUARD(gabHuffmanCodeLengths, "gabHuffmanCodeLengths");

float sinTable_f32[512] = {
    0.0,
    0.01227154,
    0.02454123,
    0.03680722,
    0.04906768,
    0.06132074,
    0.07356457,
    0.08579732,
    0.09801714,
    0.1102222,
    0.1224107,
    0.1345807,
    0.1467305,
    0.1588582,
    0.1709619,
    0.1830399,
    0.1950903,
    0.2071114,
    0.2191013,
    0.2310581,
    0.2429802,
    0.2548657,
    0.2667128,
    0.2785197,
    0.2902847,
    0.3020059,
    0.3136818,
    0.3253103,
    0.3368899,
    0.3484187,
    0.3598951,
    0.3713172,
    0.3826835,
    0.3939920,
    0.4052413,
    0.4164296,
    0.4275551,
    0.4386162,
    0.4496113,
    0.4605387,
    0.4713967,
    0.4821838,
    0.4928982,
    0.5035384,
    0.5141028,
    0.5245897,
    0.5349976,
    0.5453250,
    0.5555702,
    0.5657318,
    0.5758082,
    0.5857978,
    0.5956993,
    0.6055111,
    0.6152316,
    0.6248595,
    0.6343933,
    0.6438316,
    0.6531729,
    0.6624158,
    0.6715590,
    0.6806010,
    0.6895406,
    0.6983762,
    0.7071068,
    0.7157308,
    0.7242471,
    0.7326543,
    0.7409511,
    0.7491364,
    0.7572089,
    0.7651673,
    0.7730105,
    0.7807372,
    0.7883465,
    0.7958369,
    0.8032076,
    0.8104572,
    0.8175848,
    0.8245893,
    0.8314697,
    0.8382247,
    0.8448536,
    0.8513552,
    0.8577286,
    0.8639728,
    0.8700870,
    0.8760701,
    0.8819213,
    0.8876396,
    0.8932243,
    0.8986745,
    0.9039893,
    0.9091680,
    0.9142098,
    0.9191139,
    0.9238796,
    0.9285061,
    0.9329928,
    0.9373390,
    0.9415441,
    0.9456074,
    0.9495282,
    0.9533061,
    0.9569404,
    0.9604305,
    0.9637761,
    0.9669765,
    0.9700313,
    0.9729400,
    0.9757022,
    0.9783174,
    0.9807853,
    0.9831055,
    0.9852777,
    0.9873014,
    0.9891765,
    0.9909027,
    0.9924796,
    0.9939070,
    0.9951847,
    0.9963126,
    0.9972904,
    0.9981181,
    0.9987954,
    0.9993224,
    0.9996988,
    0.9999247,
    1,
    0.9999247,
    0.9996988,
    0.9993224,
    0.9987954,
    0.9981181,
    0.9972904,
    0.9963126,
    0.9951847,
    0.9939070,
    0.9924795,
    0.9909026,
    0.9891765,
    0.9873014,
    0.9852777,
    0.9831055,
    0.9807853,
    0.9783174,
    0.9757021,
    0.9729400,
    0.9700313,
    0.9669765,
    0.9637761,
    0.9604305,
    0.9569403,
    0.9533060,
    0.9495282,
    0.9456073,
    0.9415441,
    0.9373390,
    0.9329928,
    0.9285061,
    0.9238795,
    0.9191138,
    0.9142097,
    0.9091679,
    0.9039893,
    0.8986744,
    0.8932243,
    0.8876396,
    0.8819212,
    0.8760701,
    0.8700870,
    0.8639728,
    0.8577286,
    0.8513551,
    0.8448535,
    0.8382246,
    0.8314696,
    0.8245893,
    0.8175848,
    0.8104572,
    0.8032075,
    0.7958369,
    0.7883464,
    0.7807372,
    0.7730104,
    0.7651672,
    0.7572088,
    0.7491363,
    0.7409511,
    0.7326542,
    0.7242470,
    0.7157308,
    0.7071067,
    0.6983762,
    0.6895405,
    0.6806009,
    0.6715589,
    0.6624157,
    0.6531728,
    0.6438315,
    0.6343932,
    0.6248595,
    0.6152315,
    0.6055110,
    0.5956993,
    0.5857978,
    0.5758081,
    0.5657318,
    0.5555702,
    0.5453249,
    0.5349976,
    0.5245896,
    0.5141027,
    0.5035383,
    0.4928981,
    0.4821837,
    0.4713967,
    0.4605387,
    0.4496113,
    0.4386162,
    0.4275550,
    0.4164295,
    0.4052413,
    0.3939920,
    0.3826834,
    0.3713171,
    0.3598950,
    0.3484186,
    0.3368898,
    0.3253102,
    0.3136817,
    0.3020059,
    0.2902846,
    0.2785196,
    0.2667127,
    0.2548656,
    0.2429801,
    0.2310580,
    0.2191012,
    0.2071113,
    0.1950902,
    0.1830398,
    0.1709618,
    0.1588581,
    0.1467304,
    0.1345806,
    0.1224106,
    0.1102221,
    0.09801706,
    0.08579723,
    0.07356448,
    0.06132065,
    0.04906759,
    0.03680713,
    0.02454114,
    0.01227145,
    -8.742278E-8,
    -0.01227163,
    -0.02454132,
    -0.03680731,
    -0.04906776,
    -0.06132083,
    -0.07356466,
    -0.08579740,
    -0.09801723,
    -0.1102223,
    -0.1224108,
    -0.1345808,
    -0.1467306,
    -0.1588582,
    -0.1709620,
    -0.1830400,
    -0.1950904,
    -0.2071115,
    -0.2191013,
    -0.2310582,
    -0.2429803,
    -0.2548658,
    -0.2667128,
    -0.2785198,
    -0.2902848,
    -0.3020060,
    -0.3136818,
    -0.3253104,
    -0.3368900,
    -0.3484188,
    -0.3598951,
    -0.3713173,
    -0.3826835,
    -0.3939921,
    -0.4052414,
    -0.4164296,
    -0.4275552,
    -0.4386163,
    -0.4496114,
    -0.4605388,
    -0.4713968,
    -0.4821839,
    -0.4928983,
    -0.5035385,
    -0.5141028,
    -0.5245898,
    -0.5349977,
    -0.5453251,
    -0.5555703,
    -0.5657319,
    -0.5758083,
    -0.5857980,
    -0.5956994,
    -0.6055111,
    -0.6152317,
    -0.6248596,
    -0.6343934,
    -0.6438316,
    -0.6531729,
    -0.6624159,
    -0.6715590,
    -0.6806011,
    -0.6895406,
    -0.6983763,
    -0.7071068,
    -0.7157309,
    -0.7242472,
    -0.7326543,
    -0.7409512,
    -0.7491364,
    -0.7572089,
    -0.7651674,
    -0.7730106,
    -0.7807373,
    -0.7883465,
    -0.7958370,
    -0.8032076,
    -0.8104573,
    -0.8175849,
    -0.8245894,
    -0.8314697,
    -0.8382248,
    -0.8448536,
    -0.8513553,
    -0.8577287,
    -0.8639729,
    -0.8700870,
    -0.8760701,
    -0.8819213,
    -0.8876397,
    -0.8932244,
    -0.8986745,
    -0.9039893,
    -0.9091680,
    -0.9142098,
    -0.9191139,
    -0.9238796,
    -0.9285061,
    -0.9329928,
    -0.9373391,
    -0.9415441,
    -0.9456074,
    -0.9495282,
    -0.9533061,
    -0.9569404,
    -0.9604306,
    -0.9637761,
    -0.9669765,
    -0.9700313,
    -0.9729400,
    -0.9757022,
    -0.9783174,
    -0.9807853,
    -0.9831055,
    -0.9852777,
    -0.9873014,
    -0.9891765,
    -0.9909027,
    -0.9924796,
    -0.9939070,
    -0.9951847,
    -0.9963126,
    -0.9972905,
    -0.9981181,
    -0.9987954,
    -0.9993224,
    -0.9996988,
    -0.9999247,
    -1,
    -0.9999247,
    -0.9996988,
    -0.9993224,
    -0.9987954,
    -0.9981181,
    -0.9972904,
    -0.9963126,
    -0.9951847,
    -0.9939070,
    -0.9924795,
    -0.9909026,
    -0.9891765,
    -0.9873014,
    -0.9852776,
    -0.9831055,
    -0.9807853,
    -0.9783173,
    -0.9757021,
    -0.9729399,
    -0.9700312,
    -0.9669765,
    -0.9637761,
    -0.9604305,
    -0.9569403,
    -0.9533060,
    -0.9495282,
    -0.9456073,
    -0.9415440,
    -0.9373389,
    -0.9329928,
    -0.9285060,
    -0.9238795,
    -0.9191138,
    -0.9142097,
    -0.9091679,
    -0.9039893,
    -0.8986744,
    -0.8932242,
    -0.8876396,
    -0.8819212,
    -0.8760700,
    -0.8700869,
    -0.8639728,
    -0.8577285,
    -0.8513551,
    -0.8448535,
    -0.8382246,
    -0.8314695,
    -0.8245892,
    -0.8175848,
    -0.8104571,
    -0.8032075,
    -0.7958368,
    -0.7883464,
    -0.7807372,
    -0.7730104,
    -0.7651672,
    -0.7572088,
    -0.7491363,
    -0.7409510,
    -0.7326542,
    -0.7242470,
    -0.7157307,
    -0.7071066,
    -0.6983761,
    -0.6895404,
    -0.6806009,
    -0.6715589,
    -0.6624157,
    -0.6531727,
    -0.6438314,
    -0.6343932,
    -0.6248594,
    -0.6152315,
    -0.6055109,
    -0.5956992,
    -0.5857977,
    -0.5758080,
    -0.5657317,
    -0.5555701,
    -0.5453249,
    -0.5349975,
    -0.5245895,
    -0.5141026,
    -0.5035383,
    -0.4928980,
    -0.4821836,
    -0.4713966,
    -0.4605386,
    -0.4496112,
    -0.4386161,
    -0.4275549,
    -0.4164294,
    -0.4052412,
    -0.3939919,
    -0.3826833,
    -0.3713170,
    -0.3598949,
    -0.3484185,
    -0.3368897,
    -0.3253101,
    -0.3136816,
    -0.3020058,
    -0.2902845,
    -0.2785195,
    -0.2667126,
    -0.2548655,
    -0.2429800,
    -0.2310579,
    -0.2191011,
    -0.2071112,
    -0.1950902,
    -0.1830397,
    -0.1709617,
    -0.1588580,
    -0.1467303,
    -0.1345805,
    -0.1224105,
    -0.1102220,
    -0.09801697,
    -0.08579714,
    -0.07356439,
    -0.06132056,
    -0.04906750,
    -0.03680705,
    -0.02454105,
    -0.01227136
}; D2_GLOBAL_GUARD(sinTable_f32, "sinTable_f32");

char gszLogFilePath[260] = {
    'D',
    '2',
    '\x00',
    '\x00',
    '\x00',
    '\x00',
    '\x00',
    '\x00',
    '\x00',
    '\x00',
    '\x00',
    '\x00',
    '\x00',
    '\x00',
    '\x00',
    '\x00',
    '\x00',
    '\x00',
    '\x00',
    '\x00',
    '\x00',
    '\x00',
    '\x00',
    '\x00',
    '\x00',
    '\x00',
    '\x00',
    '\x00',
    '\x00',
    '\x00',
    '\x00',
    '\x00',
    '\x00',
    '\x00',
    '\x00',
    '\x00',
    '\x00',
    '\x00',
    '\x00',
    '\x00',
    '\x00',
    '\x00',
    '\x00',
    '\x00',
    '\x00',
    '\x00',
    '\x00',
    '\x00',
    '\x00',
    '\x00',
    '\x00',
    '\x00',
    '\x00',
    '\x00',
    '\x00',
    '\x00',
    '\x00',
    '\x00',
    '\x00',
    '\x00',
    '\x00',
    '\x00',
    '\x00',
    '\x00',
    '\x00',
    '\x00',
    '\x00',
    '\x00',
    '\x00',
    '\x00',
    '\x00',
    '\x00',
    '\x00',
    '\x00',
    '\x00',
    '\x00',
    '\x00',
    '\x00',
    '\x00',
    '\x00',
    '\x00',
    '\x00',
    '\x00',
    '\x00',
    '\x00',
    '\x00',
    '\x00',
    '\x00',
    '\x00',
    '\x00',
    '\x00',
    '\x00',
    '\x00',
    '\x00',
    '\x00',
    '\x00',
    '\x00',
    '\x00',
    '\x00',
    '\x00',
    '\x00',
    '\x00',
    '\x00',
    '\x00',
    '\x00',
    '\x00',
    '\x00',
    '\x00',
    '\x00',
    '\x00',
    '\x00',
    '\x00',
    '\x00',
    '\x00',
    '\x00',
    '\x00',
    '\x00',
    '\x00',
    '\x00',
    '\x00',
    '\x00',
    '\x00',
    '\x00',
    '\x00',
    '\x00',
    '\x00',
    '\x00',
    '\x00',
    '\x00',
    '\x00',
    '\x00',
    '\x00',
    '\x00',
    '\x00',
    '\x00',
    '\x00',
    '\x00',
    '\x00',
    '\x00',
    '\x00',
    '\x00',
    '\x00',
    '\x00',
    '\x00',
    '\x00',
    '\x00',
    '\x00',
    '\x00',
    '\x00',
    '\x00',
    '\x00',
    '\x00',
    '\x00',
    '\x00',
    '\x00',
    '\x00',
    '\x00',
    '\x00',
    '\x00',
    '\x00',
    '\x00',
    '\x00',
    '\x00',
    '\x00',
    '\x00',
    '\x00',
    '\x00',
    '\x00',
    '\x00',
    '\x00',
    '\x00',
    '\x00',
    '\x00',
    '\x00',
    '\x00',
    '\x00',
    '\x00',
    '\x00',
    '\x00',
    '\x00',
    '\x00',
    '\x00',
    '\x00',
    '\x00',
    '\x00',
    '\x00',
    '\x00',
    '\x00',
    '\x00',
    '\x00',
    '\x00',
    '\x00',
    '\x00',
    '\x00',
    '\x00',
    '\x00',
    '\x00',
    '\x00',
    '\x00',
    '\x00',
    '\x00',
    '\x00',
    '\x00',
    '\x00',
    '\x00',
    '\x00',
    '\x00',
    '\x00',
    '\x00',
    '\x00',
    '\x00',
    '\x00',
    '\x00',
    '\x00',
    '\x00',
    '\x00',
    '\x00',
    '\x00',
    '\x00',
    '\x00',
    '\x00',
    '\x00',
    '\x00',
    '\x00',
    '\x00',
    '\x00',
    '\x00',
    '\x00',
    '\x00',
    '\x00',
    '\x00',
    '\x00',
    '\x00',
    '\x00',
    '\x00',
    '\x00',
    '\x00',
    '\x00',
    '\x00',
    '\x00',
    '\x00',
    '\x00',
    '\x00',
    '\x00',
    '\x00',
    '\x00',
    '\x00',
    '\x00',
    '\x00',
    '\x00',
    '\x00',
    '\x00',
    '\x00',
    '\x00',
    '\x00',
    '\x00',
    '\x00',
    '\x00',
    '\x00',
    '\x00'
}; D2_GLOBAL_GUARD(gszLogFilePath, "gszLogFilePath");

char s___AUMessage_LogManager___00708124[28] = {
    '.',
    '?',
    'A',
    'U',
    'M',
    'e',
    's',
    's',
    'a',
    'g',
    'e',
    '@',
    'L',
    'o',
    'g',
    'M',
    'a',
    'n',
    'a',
    'g',
    'e',
    'r',
    '@',
    '@',
    '\x00',
    '\x00',
    '\x00',
    '\x00'
}; D2_GLOBAL_GUARD(s___AUMessage_LogManager___00708124, "s___AUMessage_LogManager___00708124");

SERVICE_STATUS_HANDLE ghServiceCtrlHandler = nullptr; D2_GLOBAL_GUARD(ghServiceCtrlHandler, "ghServiceCtrlHandler");

_RTL_CRITICAL_SECTION lpCriticalSection_0074c830 = {
    /* .DebugInfo = */ nullptr,
    /* .LockCount = */ 0x0,
    /* .RecursionCount = */ 0x0,
    /* .OwningThread = */ nullptr,
    /* .LockSemaphore = */ nullptr,
    /* .SpinCount = */ 0x0
}; D2_GLOBAL_GUARD(lpCriticalSection_0074c830, "lpCriticalSection_0074c830");

_RTL_CRITICAL_SECTION lpCriticalSection_0074d848 = {}; D2_GLOBAL_GUARD(lpCriticalSection_0074d848, "lpCriticalSection_0074d848");

_SYSTEM_INFO gSystemInfo = {}; D2_GLOBAL_GUARD(gSystemInfo, "gSystemInfo");

char gszCrashReportProcessPath[2048] = {}; D2_GLOBAL_GUARD(gszCrashReportProcessPath, "gszCrashReportProcessPath");

uint32_t gaHuffmanFastDecodeTable[256] = {}; D2_GLOBAL_GUARD(gaHuffmanFastDecodeTable, "gaHuffmanFastDecodeTable");

uint8_t gabHuffmanNextTable[256] = {}; D2_GLOBAL_GUARD(gabHuffmanNextTable, "gabHuffmanNextTable");

uint64_t gaServerSentPacketByteCounts[256] = {}; D2_GLOBAL_GUARD(gaServerSentPacketByteCounts, "gaServerSentPacketByteCounts");

// also labelled: gnPacketLogWriteOffset0, gnPacketLogWriteOffset1
unsigned long long gnSentPacketRawBytes = 0; D2_GLOBAL_GUARD(gnSentPacketRawBytes, "gnSentPacketRawBytes");

// also labelled: gnPacketLogWriteOffset2, gnPacketLogWriteOffset3
unsigned long long gnSentPacketWireBytes = 0; D2_GLOBAL_GUARD(gnSentPacketWireBytes, "gnSentPacketWireBytes");

// also labelled: gdwPacketLogSequenceNum, gnPacketLogSequenceNumHigh
unsigned long long gnSentPacketHeaderBytes = 0; D2_GLOBAL_GUARD(gnSentPacketHeaderBytes, "gnSentPacketHeaderBytes");

_RTL_CRITICAL_SECTION lpCriticalSection_0075cfbc = {}; D2_GLOBAL_GUARD(lpCriticalSection_0075cfbc, "lpCriticalSection_0075cfbc");

char cp_0075d040[256] = {}; D2_GLOBAL_GUARD(cp_0075d040, "cp_0075d040");

char gLogFileNameArray[20][16] = {}; D2_GLOBAL_GUARD(gLogFileNameArray, "gLogFileNameArray");

_SYSTEMTIME gLogRealmStartupLocalTime = {}; D2_GLOBAL_GUARD(gLogRealmStartupLocalTime, "gLogRealmStartupLocalTime");

int gLogFileSlotMutex[20] = {}; D2_GLOBAL_GUARD(gLogFileSlotMutex, "gLogFileSlotMutex");

uint32_t gLogFileLastAccessTimeArray[20] = {}; D2_GLOBAL_GUARD(gLogFileLastAccessTimeArray, "gLogFileLastAccessTimeArray");

char gLogFilePathArray[20][260] = {}; D2_GLOBAL_GUARD(gLogFilePathArray, "gLogFilePathArray");

FILE * globalOpenFileHandleArray[20] = {}; D2_GLOBAL_GUARD(globalOpenFileHandleArray, "globalOpenFileHandleArray");

uint32_t gLogFileLastWriteTimeArray[20] = {}; D2_GLOBAL_GUARD(gLogFileLastWriteTimeArray, "gLogFileLastWriteTimeArray");

CRITICAL_SECTION gLogMessageQueueCS = {}; D2_GLOBAL_GUARD(gLogMessageQueueCS, "gLogMessageQueueCS");

// also labelled: FileHandlesCriticalSection_Maybe
_RTL_CRITICAL_SECTION gLogFileCS = {}; D2_GLOBAL_GUARD(gLogFileCS, "gLogFileCS");

D2StormListHead gLogMessageQueueHead = {}; D2_GLOBAL_GUARD(gLogMessageQueueHead, "gLogMessageQueueHead");

D2HighResTimestampStrc gHighResTimerInitTimestamp = D2HighResTimestampStrc(); D2_GLOBAL_GUARD(gHighResTimerInitTimestamp, "gHighResTimerInitTimestamp");

CRITICAL_SECTION gHighResTimerCS = {}; D2_GLOBAL_GUARD(gHighResTimerCS, "gHighResTimerCS");

char gszSystemGlobalString[1024] = {}; D2_GLOBAL_GUARD(gszSystemGlobalString, "gszSystemGlobalString");

_RTL_CRITICAL_SECTION lpCriticalSection_0098a6c4 = {}; D2_GLOBAL_GUARD(lpCriticalSection_0098a6c4, "lpCriticalSection_0098a6c4");

_RTL_CRITICAL_SECTION lpCriticalSection_0098a6f0 = {}; D2_GLOBAL_GUARD(lpCriticalSection_0098a6f0, "lpCriticalSection_0098a6f0");

char szCustomSymbolSearchPath[32768] = {}; D2_GLOBAL_GUARD(szCustomSymbolSearchPath, "szCustomSymbolSearchPath");

char gszCrashLogTempDir[1024] = {}; D2_GLOBAL_GUARD(gszCrashLogTempDir, "gszCrashLogTempDir");

uint32_t gaEnumProcessModuleHandles[1028] = {}; D2_GLOBAL_GUARD(gaEnumProcessModuleHandles, "gaEnumProcessModuleHandles");

// =============================================================================
// Initialized scalars
// =============================================================================

_SERVICE_STATUS gszCommandLineLocaleCode = {}; D2_GLOBAL_GUARD(gszCommandLineLocaleCode, "gszCommandLineLocaleCode");
uint32_t gpServerMemHeap = 0x1; D2_GLOBAL_GUARD(gpServerMemHeap, "gpServerMemHeap");
char * gnExcelRecordFieldNotFound = (char *)0x006ccb30; D2_GLOBAL_GUARD(gnExcelRecordFieldNotFound, "gnExcelRecordFieldNotFound");
uint32_t nTokenBucketMask = 0xf; D2_GLOBAL_GUARD(nTokenBucketMask, "nTokenBucketMask");
uint32_t gnQServerIpConnCountModulo = 0x11; D2_GLOBAL_GUARD(gnQServerIpConnCountModulo, "gnQServerIpConnCountModulo");
uint32_t gnQServerHacklistBucketCount = 0x1; D2_GLOBAL_GUARD(gnQServerHacklistBucketCount, "gnQServerHacklistBucketCount");
HANDLE ghCurrentProcessHandle = INVALID_HANDLE_VALUE; D2_GLOBAL_GUARD(ghCurrentProcessHandle, "ghCurrentProcessHandle");
HINSTANCE WinMain_hInstance = (HINSTANCE)0x00000000; D2_GLOBAL_GUARD(WinMain_hInstance, "WinMain_hInstance");
BOOL D2Ini_bUseDirect = 0x0; D2_GLOBAL_GUARD(D2Ini_bUseDirect, "D2Ini_bUseDirect");
uint32_t g_nProjectId = 0; D2_GLOBAL_GUARD(g_nProjectId, "g_nProjectId");
char * szProduct = 0; D2_GLOBAL_GUARD(szProduct, "szProduct");
char * g_szApplicationName = 0; D2_GLOBAL_GUARD(g_szApplicationName, "g_szApplicationName");
char * g_szModuleName = 0; D2_GLOBAL_GUARD(g_szModuleName, "g_szModuleName");
char * g_szMilestone = 0; D2_GLOBAL_GUARD(g_szMilestone, "g_szMilestone");
char * g_szBuildNumber = 0; D2_GLOBAL_GUARD(g_szBuildNumber, "g_szBuildNumber");
char * g_szPlatform = 0; D2_GLOBAL_GUARD(g_szPlatform, "g_szPlatform");
char * g_szPrimaryAssignment = 0; D2_GLOBAL_GUARD(g_szPrimaryAssignment, "g_szPrimaryAssignment");
char * g_szAssignments = 0; D2_GLOBAL_GUARD(g_szAssignments, "g_szAssignments");
int gbSystemInfoInitialized = 0; D2_GLOBAL_GUARD(gbSystemInfoInitialized, "gbSystemInfoInitialized");
uint32_t g_eOSType = 0; D2_GLOBAL_GUARD(g_eOSType, "g_eOSType");
int fGameVersion = 0; D2_GLOBAL_GUARD(fGameVersion, "fGameVersion");
uint32_t g_dwTotalPhysicalMemory = 0; D2_GLOBAL_GUARD(g_dwTotalPhysicalMemory, "g_dwTotalPhysicalMemory");
uint32_t g_dwErrorState = 0; D2_GLOBAL_GUARD(g_dwErrorState, "g_dwErrorState");
uint32_t gbFogErrorManagerInitialized = 0; D2_GLOBAL_GUARD(gbFogErrorManagerInitialized, "gbFogErrorManagerInitialized");
// also labelled: gpAsyncDataQueueBuffer
int gnAsyncDataPriorityDeferDepth = 0; D2_GLOBAL_GUARD(gnAsyncDataPriorityDeferDepth, "gnAsyncDataPriorityDeferDepth");
// also labelled: gpAsyncDataLoadBuffer
D2AsyncData_Line0x128 * gpAsyncDataDeferredList = 0; D2_GLOBAL_GUARD(gpAsyncDataDeferredList, "gpAsyncDataDeferredList");
uint32_t gbAsyncDataInitialized = 0; D2_GLOBAL_GUARD(gbAsyncDataInitialized, "gbAsyncDataInitialized");
uint32_t gnAsyncDataHandleBufferSize = 0; D2_GLOBAL_GUARD(gnAsyncDataHandleBufferSize, "gnAsyncDataHandleBufferSize");
D2AsyncDataStrc * pD2AsyncDataStrc = 0; D2_GLOBAL_GUARD(pD2AsyncDataStrc, "pD2AsyncDataStrc");
uint32_t gnDbgSymInitRefcount = 0; D2_GLOBAL_GUARD(gnDbgSymInitRefcount, "gnDbgSymInitRefcount");
int gLogMessageQueueCount = 0; D2_GLOBAL_GUARD(gLogMessageQueueCount, "gLogMessageQueueCount");
int gLogMessageQueueTotalBytes = 0; D2_GLOBAL_GUARD(gLogMessageQueueTotalBytes, "gLogMessageQueueTotalBytes");
HANDLE gLogWriterThreadHandle = 0; D2_GLOBAL_GUARD(gLogWriterThreadHandle, "gLogWriterThreadHandle");
uint32_t gLogManagerInitialized = 0; D2_GLOBAL_GUARD(gLogManagerInitialized, "gLogManagerInitialized");
uint32_t gLogWriterThreadStopFlag = 0; D2_GLOBAL_GUARD(gLogWriterThreadStopFlag, "gLogWriterThreadStopFlag");
LPDWORD gLogWriterThreadId = 0; D2_GLOBAL_GUARD(gLogWriterThreadId, "gLogWriterThreadId");
SMemHeapStatsStrc gaSMemHeapStats[4096] = {}; D2_GLOBAL_GUARD(gaSMemHeapStats, "gaSMemHeapStats");
int g_SMemLastTickCount = 0; D2_GLOBAL_GUARD(g_SMemLastTickCount, "g_SMemLastTickCount");
uint32_t gnD3DBinkVideoYOffset = 0; D2_GLOBAL_GUARD(gnD3DBinkVideoYOffset, "gnD3DBinkVideoYOffset");
uint32_t gnD3DBinkVideoXOffset = 0; D2_GLOBAL_GUARD(gnD3DBinkVideoXOffset, "gnD3DBinkVideoXOffset");
void * gpBinkVideoD3D = 0; D2_GLOBAL_GUARD(gpBinkVideoD3D, "gpBinkVideoD3D");
uint32_t gnBinkDDSurfaceType = 0; D2_GLOBAL_GUARD(gnBinkDDSurfaceType, "gnBinkDDSurfaceType");
uint32_t gbBinkVideoShouldStop = 0; D2_GLOBAL_GUARD(gbBinkVideoShouldStop, "gbBinkVideoShouldStop");
uint32_t gbQServerUseDedicatedIoWorkers = 0; D2_GLOBAL_GUARD(gbQServerUseDedicatedIoWorkers, "gbQServerUseDedicatedIoWorkers");
unsigned int gdwHacklistLastLogTime = {}; D2_GLOBAL_GUARD(gdwHacklistLastLogTime, "gdwHacklistLastLogTime");
HMODULE hDbgHelpDll = 0; D2_GLOBAL_GUARD(hDbgHelpDll, "hDbgHelpDll");
pfnSymGetModuleInfo64 pSymGetModuleInfo64 = (pfnSymGetModuleInfo64)0; D2_GLOBAL_GUARD(pSymGetModuleInfo64, "pSymGetModuleInfo64");
pfnSymGetSymFromAddr64 pSymGetSymFromAddr64 = (pfnSymGetSymFromAddr64)0; D2_GLOBAL_GUARD(pSymGetSymFromAddr64, "pSymGetSymFromAddr64");
pfnSymSetSearchPath pSymSetSearchPath = (pfnSymSetSearchPath)0; D2_GLOBAL_GUARD(pSymSetSearchPath, "pSymSetSearchPath");
pfnSymUnloadModule64 pSymUnloadModule64 = (pfnSymUnloadModule64)0; D2_GLOBAL_GUARD(pSymUnloadModule64, "pSymUnloadModule64");
pfnSymFunctionTableAccess64 pSymFunctionTableAccess64 = (pfnSymFunctionTableAccess64)0; D2_GLOBAL_GUARD(pSymFunctionTableAccess64, "pSymFunctionTableAccess64");
pfnSymGetModuleBase64 pSymGetModuleBase64 = (pfnSymGetModuleBase64)0; D2_GLOBAL_GUARD(pSymGetModuleBase64, "pSymGetModuleBase64");
pfnStackWalk64 pStackWalk64 = (pfnStackWalk64)0; D2_GLOBAL_GUARD(pStackWalk64, "pStackWalk64");
pfnEnumerateLoadedModules64 pEnumerateLoadedModules64 = (pfnEnumerateLoadedModules64)0; D2_GLOBAL_GUARD(pEnumerateLoadedModules64, "pEnumerateLoadedModules64");
pfnSymInitialize pSymInitializeApi = (pfnSymInitialize)0; D2_GLOBAL_GUARD(pSymInitializeApi, "pSymInitializeApi");
pfnSymGetOptions pSymGetOptionsApi = (pfnSymGetOptions)0; D2_GLOBAL_GUARD(pSymGetOptionsApi, "pSymGetOptionsApi");
uint8_t bDebugSymbolsInitialized = 0; D2_GLOBAL_GUARD(bDebugSymbolsInitialized, "bDebugSymbolsInitialized");
uint8_t bEnableCrashDumps = 0; D2_GLOBAL_GUARD(bEnableCrashDumps, "bEnableCrashDumps");
pfnSymSetOptions pSymSetOptionsApi = (pfnSymSetOptions)0; D2_GLOBAL_GUARD(pSymSetOptionsApi, "pSymSetOptionsApi");
pfnSymCleanup pSymCleanupApi = (pfnSymCleanup)0; D2_GLOBAL_GUARD(pSymCleanupApi, "pSymCleanupApi");
HANDLE hCrashDumpCompleteEvent = 0; D2_GLOBAL_GUARD(hCrashDumpCompleteEvent, "hCrashDumpCompleteEvent");
HANDLE hCrashDumpRequestEvent = 0; D2_GLOBAL_GUARD(hCrashDumpRequestEvent, "hCrashDumpRequestEvent");
pfnSymLoadModule64 pSymLoadModule64 = (pfnSymLoadModule64)0; D2_GLOBAL_GUARD(pSymLoadModule64, "pSymLoadModule64");
pfnMiniDumpWriteDump pMiniDumpWriteDumpApi = (pfnMiniDumpWriteDump)0; D2_GLOBAL_GUARD(pMiniDumpWriteDumpApi, "pMiniDumpWriteDumpApi");
void * pCrashDumpInfo = 0; D2_GLOBAL_GUARD(pCrashDumpInfo, "pCrashDumpInfo");
pfnSymGetLineFromAddr64 pSymGetLineFromAddr64 = (pfnSymGetLineFromAddr64)0; D2_GLOBAL_GUARD(pSymGetLineFromAddr64, "pSymGetLineFromAddr64");
pfnSymFindFileInPath pSymFindFileInPath = (pfnSymFindFileInPath)0; D2_GLOBAL_GUARD(pSymFindFileInPath, "pSymFindFileInPath");
pfnFogExceptionFilter gpCurrentExceptionFilter = (pfnFogExceptionFilter)0; D2_GLOBAL_GUARD(gpCurrentExceptionFilter, "gpCurrentExceptionFilter");
void * gpCurrentExceptionContext = 0; D2_GLOBAL_GUARD(gpCurrentExceptionContext, "gpCurrentExceptionContext");

namespace D2Common::Drlg::Maze {

} // namespace D2Common::Drlg::Maze

namespace Fog::Src::Excel {

} // namespace Fog::Src::Excel

namespace D2Client::Warden::NWardenClient {

} // namespace D2Client::Warden::NWardenClient

namespace D2Client::Warden {

} // namespace D2Client::Warden

namespace D2Client::UI {

} // namespace D2Client::UI

namespace D2Client::UI::Button::ButtonImplementation {

} // namespace D2Client::UI::Button::ButtonImplementation

namespace D2Client::UI::Button::ButtonWrapper {

} // namespace D2Client::UI::Button::ButtonWrapper

namespace D2Client::UI::Editbox::EditBoxImplementation {

} // namespace D2Client::UI::Editbox::EditBoxImplementation

namespace D2Client::UI::Editbox::EditBoxWrapper {

} // namespace D2Client::UI::Editbox::EditBoxWrapper

namespace D2Client::UI::Spinner::SpinnerImplementation {

} // namespace D2Client::UI::Spinner::SpinnerImplementation

namespace D2Client::UI::Spinner::SpinnerWrapper {

} // namespace D2Client::UI::Spinner::SpinnerWrapper

namespace D2Client::UNIT::Object {

} // namespace D2Client::UNIT::Object

namespace D2Client::UI::ListBox::ListBoxWrapper {

} // namespace D2Client::UI::ListBox::ListBoxWrapper

namespace D2Client::UI::ListBox::ListBoxImplementation {

} // namespace D2Client::UI::ListBox::ListBoxImplementation

namespace D2Win::_FONT {

} // namespace D2Win::_FONT

namespace D2Game::GAME::Event::Dispatch {

} // namespace D2Game::GAME::Event::Dispatch

namespace D2Game::GAME::Event::Dispatch::Player {

} // namespace D2Game::GAME::Event::Dispatch::Player

namespace D2Game::GAME::Event::Dispatch::Object {

} // namespace D2Game::GAME::Event::Dispatch::Object

namespace D2Game::GAME::Event::Dispatch::Monster {

} // namespace D2Game::GAME::Event::Dispatch::Monster

namespace D2Common::Chat::Ignorelist::ChatIgnoreList {

} // namespace D2Common::Chat::Ignorelist::ChatIgnoreList

namespace D2Common::Chat::Ignorelist::IgnoreList {

} // namespace D2Common::Chat::Ignorelist::IgnoreList

namespace D2Common::Drlg::Outdoors {

} // namespace D2Common::Drlg::Outdoors

namespace Define_the_symbol__ATL_MIXED {

} // namespace Define_the_symbol__ATL_MIXED

namespace Fog::_3rdparty::crashy {

// =============================================================================
// Initialized data (arrays, structs, tables)
// =============================================================================

void * vftable[1] = { (void*)&Fog::_3rdparty::crashy::ScalarDeletingDestructor }; D2_GLOBAL_GUARD(vftable, "Fog::_3rdparty::crashy::vftable");

} // namespace Fog::_3rdparty::crashy

namespace Storm::Source::SFile {

} // namespace Storm::Source::SFile

namespace Class::FILEREC {

} // namespace Class::FILEREC

namespace Class::EVENTREC {

} // namespace Class::EVENTREC

namespace Class::REQUEST {

} // namespace Class::REQUEST

namespace Class::AUDIOSTREAM {

} // namespace Class::AUDIOSTREAM

namespace Class::HANDLER {

} // namespace Class::HANDLER

namespace D2Client::CharScreen {

} // namespace D2Client::CharScreen

namespace D2Client::UI::Automap {

} // namespace D2Client::UI::Automap

namespace D2Game::GAME::Rendering {

} // namespace D2Game::GAME::Rendering

namespace D2Client::UI::Help {

} // namespace D2Client::UI::Help

namespace D2Client::UI::npcmenu {

} // namespace D2Client::UI::npcmenu

namespace D2Client::NPC {

} // namespace D2Client::NPC

namespace D2Client::UNIT::UnitSnd {

} // namespace D2Client::UNIT::UnitSnd

namespace D2Client::UI::ui {

} // namespace D2Client::UI::ui

namespace D2Game::Quests::QUESTS::Quest40 {

} // namespace D2Game::Quests::QUESTS::Quest40

namespace D2Game::Quests::QUESTS::A5Q1 {

} // namespace D2Game::Quests::QUESTS::A5Q1

namespace D2Game::Quests::QUESTS::A5Q2 {

} // namespace D2Game::Quests::QUESTS::A5Q2

namespace D2Game::Quests::QUESTS::A5Q3 {

} // namespace D2Game::Quests::QUESTS::A5Q3

namespace D2Game::Quests::QUESTS::A5Q4 {

} // namespace D2Game::Quests::QUESTS::A5Q4

namespace D2Game::Quests::QUESTS::A5Q5 {

} // namespace D2Game::Quests::QUESTS::A5Q5

namespace D2Game::Quests::QUESTS::A5Q6 {

} // namespace D2Game::Quests::QUESTS::A5Q6

namespace D2Game::Quests::QUESTS::Quest37 {

} // namespace D2Game::Quests::QUESTS::Quest37

namespace D2Game::Quests::QUESTS::A1Q0 {

} // namespace D2Game::Quests::QUESTS::A1Q0

namespace D2Game::Quests::QUESTS::A1Q1 {

} // namespace D2Game::Quests::QUESTS::A1Q1

namespace D2Game::Quests::QUESTS::A1Q2 {

} // namespace D2Game::Quests::QUESTS::A1Q2

namespace D2Game::Quests::QUESTS::A1Q3 {

} // namespace D2Game::Quests::QUESTS::A1Q3

namespace D2Game::Quests::QUESTS::A1Q4 {

} // namespace D2Game::Quests::QUESTS::A1Q4

namespace D2Game::Quests::QUESTS::A1Q5 {

} // namespace D2Game::Quests::QUESTS::A1Q5

namespace D2Game::Quests::QUESTS::A1Q6 {

} // namespace D2Game::Quests::QUESTS::A1Q6

namespace D2Game::Quests::QUESTS::A1Q7 {

} // namespace D2Game::Quests::QUESTS::A1Q7

namespace D2Game::Quests::QUESTS::Quest38 {

} // namespace D2Game::Quests::QUESTS::Quest38

namespace D2Game::Quests::QUESTS::A2Q0 {

} // namespace D2Game::Quests::QUESTS::A2Q0

namespace D2Game::Quests::QUESTS::A2Q1 {

} // namespace D2Game::Quests::QUESTS::A2Q1

namespace D2Game::Quests::QUESTS::A2Q2 {

} // namespace D2Game::Quests::QUESTS::A2Q2

namespace D2Game::Quests::QUESTS::A2Q3 {

} // namespace D2Game::Quests::QUESTS::A2Q3

namespace D2Game::Quests::QUESTS::A2Q4 {

} // namespace D2Game::Quests::QUESTS::A2Q4

namespace D2Game::Quests::QUESTS::A2Q5 {

} // namespace D2Game::Quests::QUESTS::A2Q5

namespace D2Game::Quests::QUESTS::A2Q6 {

} // namespace D2Game::Quests::QUESTS::A2Q6

namespace D2Game::Quests::QUESTS::A2Q7 {

} // namespace D2Game::Quests::QUESTS::A2Q7

namespace D2Game::Quests::QUESTS::A2Q8 {

} // namespace D2Game::Quests::QUESTS::A2Q8

namespace D2Common::Monsters::MONSTERS::Umod {

} // namespace D2Common::Monsters::MONSTERS::Umod

namespace D2Game::Quests::QUESTS::A4Q0 {

} // namespace D2Game::Quests::QUESTS::A4Q0

namespace D2Game::Quests::QUESTS::A4Q1 {

} // namespace D2Game::Quests::QUESTS::A4Q1

namespace D2Game::Quests::QUESTS::A4Q2 {

} // namespace D2Game::Quests::QUESTS::A4Q2

namespace D2Game::Quests::QUESTS::A4Q3 {

} // namespace D2Game::Quests::QUESTS::A4Q3

namespace D2Game::Quests::QUESTS::A4Q4 {

} // namespace D2Game::Quests::QUESTS::A4Q4

namespace D2Game::Quests::QUESTS::Quest39 {

} // namespace D2Game::Quests::QUESTS::Quest39

namespace D2Game::Quests::QUESTS::A3Q0 {

} // namespace D2Game::Quests::QUESTS::A3Q0

namespace D2Game::Quests::QUESTS::A3Q1 {

} // namespace D2Game::Quests::QUESTS::A3Q1

namespace D2Game::Quests::QUESTS::A3Q2 {

} // namespace D2Game::Quests::QUESTS::A3Q2

namespace D2Game::Quests::QUESTS::A3Q3 {

} // namespace D2Game::Quests::QUESTS::A3Q3

namespace D2Game::Quests::QUESTS::A3Q4 {

} // namespace D2Game::Quests::QUESTS::A3Q4

namespace D2Game::Quests::QUESTS::A3Q5 {

} // namespace D2Game::Quests::QUESTS::A3Q5

namespace D2Game::Quests::QUESTS::A3Q6 {

} // namespace D2Game::Quests::QUESTS::A3Q6

namespace D2Client::Waypoint {

} // namespace D2Client::Waypoint

namespace D2Common::Drlg::DrlgGrid {

} // namespace D2Common::Drlg::DrlgGrid

namespace Fog::Memory {

// =============================================================================
// Initialized data (arrays, structs, tables)
// =============================================================================

D2MemoryManagerStrc pGlobalPoolSystem = {}; D2_GLOBAL_GUARD(pGlobalPoolSystem, "Fog::Memory::pGlobalPoolSystem");

} // namespace Fog::Memory

namespace Fog::Debug {

// =============================================================================
// Initialized scalars
// =============================================================================

HMODULE pDbgHelpOrImageHlp = 0; D2_GLOBAL_GUARD(pDbgHelpOrImageHlp, "Fog::Debug::pDbgHelpOrImageHlp");
pfnSymCleanup pSymCleanup = (pfnSymCleanup)0; D2_GLOBAL_GUARD(pSymCleanup, "Fog::Debug::pSymCleanup");
pfnSymFunctionTableAccess pSymFunctionTableAccess = (pfnSymFunctionTableAccess)0; D2_GLOBAL_GUARD(pSymFunctionTableAccess, "Fog::Debug::pSymFunctionTableAccess");
pfnSymGetLineFromAddr pSymGetLineFromAddr = (pfnSymGetLineFromAddr)0; D2_GLOBAL_GUARD(pSymGetLineFromAddr, "Fog::Debug::pSymGetLineFromAddr");
pfnSymGetModuleBase pSymGetModuleBase = (pfnSymGetModuleBase)0; D2_GLOBAL_GUARD(pSymGetModuleBase, "Fog::Debug::pSymGetModuleBase");
pfnSymGetModuleInfo pSymGetModuleInfo = (pfnSymGetModuleInfo)0; D2_GLOBAL_GUARD(pSymGetModuleInfo, "Fog::Debug::pSymGetModuleInfo");
pfnSymGetOptions pSymGetOptions = (pfnSymGetOptions)0; D2_GLOBAL_GUARD(pSymGetOptions, "Fog::Debug::pSymGetOptions");
pfnSymGetSymFromAddr pSymGetSymFromAddr = (pfnSymGetSymFromAddr)0; D2_GLOBAL_GUARD(pSymGetSymFromAddr, "Fog::Debug::pSymGetSymFromAddr");
pfnSymInitialize pSymInitialize = (pfnSymInitialize)0; D2_GLOBAL_GUARD(pSymInitialize, "Fog::Debug::pSymInitialize");
pfnSymLoadModule pSymLoadModule = (pfnSymLoadModule)0; D2_GLOBAL_GUARD(pSymLoadModule, "Fog::Debug::pSymLoadModule");
pfnSymSetOptions pSymSetOptions = (pfnSymSetOptions)0; D2_GLOBAL_GUARD(pSymSetOptions, "Fog::Debug::pSymSetOptions");
pfnStackWalk pStackWalk = (pfnStackWalk)0; D2_GLOBAL_GUARD(pStackWalk, "Fog::Debug::pStackWalk");
BOOL bBetterStackCrawls = 0; D2_GLOBAL_GUARD(bBetterStackCrawls, "Fog::Debug::bBetterStackCrawls");

} // namespace Fog::Debug

namespace D2Client::MCP {

} // namespace D2Client::MCP

namespace D2Client::Bnet {

} // namespace D2Client::Bnet

namespace D2Game::Save {

} // namespace D2Game::Save

namespace D2Game::GameData {

} // namespace D2Game::GameData

namespace D2Client::GAME::Record {

} // namespace D2Client::GAME::Record

namespace D2Client::UI::Dialog {

} // namespace D2Client::UI::Dialog

namespace D2Client::_Selection {

} // namespace D2Client::_Selection

namespace D2Client::Cursor {

} // namespace D2Client::Cursor

namespace D2Client::Draw::Weather {

} // namespace D2Client::Draw::Weather

namespace D2Client::Draw::LightMap {

} // namespace D2Client::Draw::LightMap

namespace D2Client::GAME::View {

} // namespace D2Client::GAME::View

namespace D2Client::GAME::RosterPets {

} // namespace D2Client::GAME::RosterPets

namespace D2Client::GAME::Roster {

} // namespace D2Client::GAME::Roster

namespace D2Client::UI::Inv {

} // namespace D2Client::UI::Inv

namespace D2Client::UI::Hireables {

} // namespace D2Client::UI::Hireables

namespace D2Client::UI::Panel {

} // namespace D2Client::UI::Panel

namespace D2Client::UI::Party {

} // namespace D2Client::UI::Party

namespace D2Client::_Interaction {

} // namespace D2Client::_Interaction

namespace D2Client::Merc {

} // namespace D2Client::Merc

namespace D2Client::Mouse {

} // namespace D2Client::Mouse

namespace D2Client::Forms {

} // namespace D2Client::Forms

namespace D2Client::_CdKey {

} // namespace D2Client::_CdKey

namespace D2Direct3D::Renderer {

} // namespace D2Direct3D::Renderer
