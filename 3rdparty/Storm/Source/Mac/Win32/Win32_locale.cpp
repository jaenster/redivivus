// Storm's Win32-on-Mac layer: the short-date 'itl0' and the time zone, 1.14d Intel
// Mac band 0002c5bb..0002c670. See Win32Mac.h for what is and is not known about the
// original file boundaries. A file starts at 0002c5bb: GetDateFormat calls
// DATE_CreateShortDateIntl, which is tiny and has no other caller, out of line, which
// clang-425 at -Os would not do within one file.
#include "Win32Mac.h"

#if !defined(_WIN32) && D2_OS_MAC

#include <CoreServices/CoreServices.h>

typedef struct _TIME_ZONE_INFORMATION {
    LONG Bias;                    // 0x00  minutes to add to local time for UTC
    BYTE aRest[0xAC - 0x04];      // 0x04  names and daylight rules: not filled in
} TIME_ZONE_INFORMATION;

// 1.14d mac: 0002c5bb
// An 'itl0' for DateString's short form: US separators, month/day/year, 12-hour.
Handle DATE_CreateShortDateIntl(void) {
    Handle hIntl = NewHandle(sizeof(Intl0Rec));
    HLock(hIntl);
    // Four bytes at a time, in reading order: decimalPt thousSep listSep currSym1,
    // currSym2 currSym3 currFmt dateOrder (mdy), shrtDateFmt dateSep timeCycle timeFmt,
    // mornStr, eveStr, timeSep time1Suff-time3Suff, time4Suff-time7Suff, time8Suff intl0Vers.
    UInt32* pIntl = (UInt32*)*hIntl;
    pIntl[0] = EndianU32_BtoN('.,;$');
    pIntl[1] = EndianU32_BtoN('  \0\0');
    pIntl[2] = EndianU32_BtoN('\0/\xFF`');
    pIntl[3] = EndianU32_BtoN(' AM\0');
    pIntl[4] = EndianU32_BtoN(' PM\0');
    pIntl[5] = EndianU32_BtoN(':   ');
    pIntl[6] = EndianU32_BtoN('    ');
    pIntl[7] = EndianU32_BtoN(' \0\0\0');
    return hIntl;
}

// 1.14d mac: 0002c617
// Only the bias, from the machine location's GMT delta (24-bit signed seconds).
DWORD GetTimeZoneInformation(TIME_ZONE_INFORMATION* lpTimeZoneInformation) {
    DWORD dwResult = 0xFFFFFFFF;
    if (lpTimeZoneInformation != NULL) {
        MachineLocation tLocation;
        ReadLocation(&tLocation);
        LONG nDelta = tLocation.u.gmtDelta & 0xFFFFFF;
        if (nDelta & 0x800000) {
            nDelta |= 0xFF000000;
        }
        lpTimeZoneInformation->Bias = nDelta / -60;
        dwResult = 0;
    }
    return dwResult;
}

#endif  // D2_OS_MAC
