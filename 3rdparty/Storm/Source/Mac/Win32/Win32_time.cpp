// Storm's Win32-on-Mac layer: FILETIME and SYSTEMTIME on Mac dates, date strings and
// the time zone, 1.14d Intel Mac band 0002c143..0002c670. See Win32Mac.h for what is and is not known about the
// original file boundaries.
//
// A Mac date is seconds since 1904-01-01; a FILETIME counts 100 ns since 1601-01-01.
// The two epochs are 0x0153B281E0FB4000 FILETIME units apart.
#include "Win32Mac.h"

#if !defined(_WIN32) && D2_OS_MAC

#include <CoreServices/CoreServices.h>
#include <string.h>

#define MAC_EPOCH_AS_FILETIME 0x0153B281E0FB4000ULL
#define FILETIME_UNITS_PER_SECOND 10000000

typedef struct _SYSTEMTIME {
    WORD wYear;
    WORD wMonth;
    WORD wDayOfWeek;
    WORD wDay;
    WORD wHour;
    WORD wMinute;
    WORD wSecond;
    WORD wMilliseconds;
} SYSTEMTIME;

// 1.14d mac: 003f2280
// The Mac clock read once, at the first call, and the millisecond timer at that
// moment; later times advance from them by the timer.
static unsigned long s_nClockBaseSeconds;
// 1.14d mac: 003f2284
static DWORD s_dwClockBaseMilliseconds;

static void GetCurrentTime(SYSTEMTIME* lpSystemTime, BOOL bUniversal);

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

#endif  // D2_OS_MAC
