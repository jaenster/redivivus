// StormMac Win32 Mac/Source/Win32/Win32_TimeAndDate.cpp: the tick count, the
// performance counter, FILETIME and SYSTEMTIME on Mac dates, date strings and the time
// zone, 1.14d Intel Mac 0002c0ee..0002c670 (GetTickCount .. GetTimeZoneInformation).
//
// Pinned by the PPC Storm 1.10 Beta: its __sinit_Win32_TimeAndDate_cpp closes, read last
// to first as MWCC emits a file, GetCurrentTime, GetTickCount, timeGetTime,
// QueryPerformanceCounter, QueryPerformanceFrequency, FileTimeToMacDateTime,
// MacDateTimeToFileTime, FileTimeToLocalFileTime, FileTimeToSystemTime,
// SystemTimeToFileTime, GetSystemTimeAsFileTime, CompareFileTime, GetLocalTime,
// GetSystemTime, GetDateFormat, GetTimeFormat, GetTimeZoneInformation: 1.14d's run from
// 0002c0ee to Win32_utils.cpp at 0002c670, in the same order. The folder is that of
// Win32_utils.cpp, the one Win32_ file whose 1.14d path is in the binary.
//
// A Mac date is seconds since 1904-01-01; a FILETIME counts 100 ns since 1601-01-01.
// The two epochs are 0x0153B281E0FB4000 FILETIME units apart.
#include "Win32Mac.h"

#if !defined(_WIN32) && D2_OS_MAC

#include <CoreServices/CoreServices.h>
#include <string.h>

#define MAC_EPOCH_AS_FILETIME 0x0153B281E0FB4000ULL
#define FILETIME_UNITS_PER_SECOND 10000000

// 1.14d mac: 003f2280
// The Mac clock read once, at the first call, and the millisecond timer at that
// moment; later times advance from them by the timer.
static unsigned long s_nClockBaseSeconds;
// 1.14d mac: 003f2284
static DWORD s_dwClockBaseMilliseconds;

static void GetCurrentTime(SYSTEMTIME* lpSystemTime, BOOL bUniversal);

// 1.14d mac: 0002c0ee
DWORD GetTickCount(void) {
    return FPMilliseconds();
}

// 1.14d mac: 0002c0f7
DWORD TIME_GetMilliseconds(void) {
    return FPMilliseconds();
}

// 1.14d mac: 0002c100
BOOL QueryPerformanceCounter(LARGE_INTEGER* lpPerformanceCount) {
    LARGE_INTEGER tCounter;
    MAC_GetMicroseconds64(&tCounter.LowPart);
    lpPerformanceCount->LowPart = tCounter.LowPart;
    lpPerformanceCount->HighPart = tCounter.HighPart;
    return 1;
}

// 1.14d mac: 0002c129
// The counter is in microseconds.
BOOL QueryPerformanceFrequency(LARGE_INTEGER* lpFrequency) {
    lpFrequency->LowPart = 1000000;
    lpFrequency->HighPart = 0;
    return 1;
}

// 1.14d mac: 0002c143
// The third argument is only checked for null.
BOOL FileTimeToMacDateTime(const FILETIME* pFileTime, UInt32* pnMacDate, void* pUnused) {
    BOOL bResult = 0;
    if (pFileTime != NULL && pnMacDate != NULL && pUnused != NULL) {
        uint64_t nTime = ((uint64_t)pFileTime->dwHighDateTime << 32) | pFileTime->dwLowDateTime;
        *pnMacDate = (nTime - MAC_EPOCH_AS_FILETIME) / FILETIME_UNITS_PER_SECOND;
        bResult = 1;
    }
    return bResult;
}

// 1.14d mac: 0002c199
BOOL MacDateTimeToFileTime(const UInt32* pnMacDate, FILETIME* pFileTime) {
    BOOL bResult = 0;
    if (pnMacDate != NULL && pFileTime != NULL) {
        uint64_t nTime = (uint64_t)*pnMacDate * FILETIME_UNITS_PER_SECOND + MAC_EPOCH_AS_FILETIME;
        pFileTime->dwLowDateTime = (DWORD)nTime;
        pFileTime->dwHighDateTime = (DWORD)(nTime >> 32);
        bResult = 1;
    }
    return bResult;
}

// 1.14d mac: 0002c1ca
// Adds the machine's offset from GMT, the 24-bit signed gmtDelta of its location.
BOOL FileTimeToLocalFileTime(const FILETIME* lpFileTime, FILETIME* lpLocalFileTime) {
    BOOL bResult = 0;
    if (lpFileTime != NULL && lpLocalFileTime != NULL) {
        MachineLocation tLocation;
        ReadLocation(&tLocation);
        long nDelta = tLocation.u.gmtDelta & 0xFFFFFF;
        if (tLocation.u.gmtDelta & 0x800000) {
            nDelta = tLocation.u.gmtDelta | 0xFF000000;
        }
        long nBias = -nDelta;
        uint64_t nTime = ((uint64_t)lpFileTime->dwHighDateTime << 32) | lpFileTime->dwLowDateTime;
        nTime -= (int64_t)nBias * FILETIME_UNITS_PER_SECOND;
        lpLocalFileTime->dwLowDateTime = (DWORD)nTime;
        lpLocalFileTime->dwHighDateTime = (DWORD)(nTime >> 32);
        bResult = 1;
    }
    return bResult;
}

// 1.14d mac: 0002c229
// The Mac counts weekdays from 1 (Sunday), Win32 from 0.
BOOL FileTimeToSystemTime(const FILETIME* lpFileTime, SYSTEMTIME* lpSystemTime) {
    BOOL bResult = 0;
    if (lpFileTime != NULL && lpSystemTime != NULL) {
        uint64_t nTime = ((uint64_t)lpFileTime->dwHighDateTime << 32) | lpFileTime->dwLowDateTime;
        DateTimeRec tDate;
        SecondsToDate((nTime - MAC_EPOCH_AS_FILETIME) / FILETIME_UNITS_PER_SECOND, &tDate);
        lpSystemTime->wYear = tDate.year;
        lpSystemTime->wMonth = tDate.month;
        lpSystemTime->wDayOfWeek = tDate.dayOfWeek - 1;
        lpSystemTime->wDay = tDate.day;
        lpSystemTime->wHour = tDate.hour;
        lpSystemTime->wMinute = tDate.minute;
        lpSystemTime->wSecond = tDate.second;
        lpSystemTime->wMilliseconds = 0;
        bResult = 1;
    }
    return bResult;
}

// 1.14d mac: 0002c2c8
BOOL SystemTimeToFileTime(const SYSTEMTIME* lpSystemTime, FILETIME* lpFileTime) {
    BOOL bResult = 0;
    if (lpSystemTime != NULL && lpFileTime != NULL) {
        unsigned long nSeconds;
        DateTimeRec tDate;
        tDate.year = lpSystemTime->wYear;
        tDate.month = lpSystemTime->wMonth;
        tDate.day = lpSystemTime->wDay;
        tDate.hour = lpSystemTime->wHour;
        tDate.minute = lpSystemTime->wMinute;
        tDate.second = lpSystemTime->wSecond;
        tDate.dayOfWeek = lpSystemTime->wDayOfWeek + 1;
        DateToSeconds(&tDate, &nSeconds);
        uint64_t nTime = (uint64_t)nSeconds * FILETIME_UNITS_PER_SECOND + MAC_EPOCH_AS_FILETIME;
        lpFileTime->dwLowDateTime = (DWORD)nTime;
        lpFileTime->dwHighDateTime = (DWORD)(nTime >> 32);
        bResult = 1;
    }
    return bResult;
}

// 1.14d mac: 0002c34d
void GetSystemTime(SYSTEMTIME* lpSystemTime) {
    if (lpSystemTime != NULL) {
        GetCurrentTime(lpSystemTime, 1);
    }
}

// 1.14d mac: 0002c364
LONG CompareFileTime(const FILETIME* lpFileTime1, const FILETIME* lpFileTime2) {
    if (lpFileTime1->dwHighDateTime > lpFileTime2->dwHighDateTime) {
        return 1;
    }
    if (lpFileTime1->dwHighDateTime < lpFileTime2->dwHighDateTime) {
        return -1;
    }
    if (lpFileTime1->dwLowDateTime > lpFileTime2->dwLowDateTime) {
        return 1;
    }
    if (lpFileTime1->dwLowDateTime < lpFileTime2->dwLowDateTime) {
        return -1;
    }
    return 0;
}

// 1.14d mac: 0002c3a6
// The current time as a SYSTEMTIME, universal (the GMT delta taken off) or local.
static void GetCurrentTime(SYSTEMTIME* lpSystemTime, BOOL bUniversal) {
    if (s_nClockBaseSeconds == 0) {
        GetDateTime(&s_nClockBaseSeconds);
        s_dwClockBaseMilliseconds = FPMilliseconds();
    }
    DWORD dwElapsed = FPMilliseconds() - s_dwClockBaseMilliseconds;
    unsigned long nSeconds = dwElapsed / 1000 + s_nClockBaseSeconds;
    MachineLocation tLocation;
    DateTimeRec tDate;
    if (bUniversal) {
        ReadLocation(&tLocation);
        long nDelta = tLocation.u.gmtDelta & 0xFFFFFF;
        if (tLocation.u.gmtDelta & 0x800000) {
            nDelta = tLocation.u.gmtDelta | 0xFF000000;
        }
        SecondsToDate(nSeconds - nDelta, &tDate);
    } else {
        SecondsToDate(nSeconds, &tDate);
    }
    lpSystemTime->wYear = tDate.year;
    lpSystemTime->wMonth = tDate.month;
    lpSystemTime->wDayOfWeek = tDate.dayOfWeek - 1;
    lpSystemTime->wDay = tDate.day;
    lpSystemTime->wHour = tDate.hour;
    lpSystemTime->wMinute = tDate.minute;
    lpSystemTime->wSecond = tDate.second;
    lpSystemTime->wMilliseconds = dwElapsed % 1000;
}

// 1.14d mac: 0002c392
void GetLocalTime(SYSTEMTIME* lpSystemTime) {
    if (lpSystemTime != NULL) {
        GetCurrentTime(lpSystemTime, 0);
    }
}

#define DATE_SHORTDATE 0x00000001
#define DATE_LONGDATE 0x00000002

// 1.14d mac: 0001eb1c
char* p2cstr(unsigned char* pString);

Handle DATE_CreateShortDateIntl(void);

// 1.14d mac: 0002c4a3
// The date as the Mac formats it: DATE_SHORTDATE with the layer's own 'itl0' (US
// order, '/'), DATE_LONGDATE as longDate, otherwise abbrevDate. lpFormat and the
// locale are not used. Returns 0, not the length.
int GetDateFormat(DWORD Locale, DWORD dwFlags, const SYSTEMTIME* lpDate, LPCSTR lpFormat, char* lpDateStr,
                  int cchDate) {
    char szDate[256];
    unsigned long nSeconds;
    DateTimeRec tDate;
    Handle hIntl;
    DateForm nForm;
    if (dwFlags & DATE_SHORTDATE) {
        hIntl = DATE_CreateShortDateIntl();
        nForm = shortDate;
    } else {
        if (dwFlags & DATE_LONGDATE) {
            nForm = longDate;
        } else {
            nForm = abbrevDate;
        }
        hIntl = NULL;
    }
    tDate.year = lpDate->wYear;
    tDate.month = lpDate->wMonth;
    tDate.day = lpDate->wDay;
    tDate.hour = lpDate->wHour;
    tDate.minute = lpDate->wMinute;
    tDate.second = lpDate->wSecond;
    tDate.dayOfWeek = lpDate->wDayOfWeek + 1;
    DateToSeconds(&tDate, &nSeconds);
    DateString(nSeconds, nForm, (unsigned char*)szDate, hIntl);
    if (hIntl != NULL) {
        DisposeHandle(hIntl);
    }
    p2cstr((unsigned char*)szDate);
    strncpy(lpDateStr, szDate, cchDate);
    lpDateStr[cchDate - 1] = '\0';
    return 0;
}

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
