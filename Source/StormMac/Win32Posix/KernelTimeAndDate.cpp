// Win32Posix: the clocks, FILETIME and SYSTEMTIME, the time zone and the date format (Win32Mac's
// Win32_TimeAndDate.cpp).
//
// GetTickCount and timeGetTime are milliseconds of the monotonic clock, wrapping at 32 bits as
// Windows' do. The performance counter runs at 10 MHz, the rate Windows has given it since
// Windows 10, so its LowPart wraps in minutes as the game expects of it, not in seconds. A
// FILETIME is 100 ns units from 1601-01-01 UTC; the calendar arithmetic is the proleptic
// Gregorian one Windows uses, so it does not depend on the range of time_t.
#if !defined(_WIN32) && D2_BACKEND_POSIX

#include <stdio.h>
#include <string.h>
#include <time.h>

#include "Source/StormMac/Win32Posix/Win32PosixKernelPriv.h"

namespace {

constexpr int64_t kTicksPerSecond = 10000000;
constexpr int64_t kUnixEpochTicks = 116444736000000000LL;  // 1970-01-01 in FILETIME units

uint64_t ToU64(const FILETIME& t) {
    return ((uint64_t)t.dwHighDateTime << 32) | t.dwLowDateTime;
}

FILETIME FromU64(uint64_t n) {
    FILETIME t;
    t.dwLowDateTime = (DWORD)n;
    t.dwHighDateTime = (DWORD)(n >> 32);
    return t;
}

// Days from 1970-01-01 of a civil date, and back (Howard Hinnant's algorithms).
int64_t DaysFromCivil(int64_t y, unsigned m, unsigned d) {
    y -= m <= 2;
    const int64_t era = (y >= 0 ? y : y - 399) / 400;
    const unsigned yoe = (unsigned)(y - era * 400);
    const unsigned doy = (153 * (m + (m > 2 ? -3 : 9)) + 2) / 5 + d - 1;
    const unsigned doe = yoe * 365 + yoe / 4 - yoe / 100 + doy;
    return era * 146097 + (int64_t)doe - 719468;
}

void CivilFromDays(int64_t z, int64_t* py, unsigned* pm, unsigned* pd) {
    z += 719468;
    const int64_t era = (z >= 0 ? z : z - 146096) / 146097;
    const unsigned doe = (unsigned)(z - era * 146097);
    const unsigned yoe = (doe - doe / 1460 + doe / 36524 - doe / 146096) / 365;
    const unsigned doy = doe - (365 * yoe + yoe / 4 - yoe / 100);
    const unsigned mp = (5 * doy + 2) / 153;
    *pd = doy - (153 * mp + 2) / 5 + 1;
    *pm = mp < 10 ? mp + 3 : mp - 9;
    *py = (int64_t)yoe + era * 400 + (*pm <= 2);
}

int64_t NowUnixTicks() {
    struct timespec t;
    clock_gettime(CLOCK_REALTIME, &t);
    return (int64_t)t.tv_sec * kTicksPerSecond + t.tv_nsec / 100;
}

void SystemTimeFromTm(const struct tm& tm, long nNanoseconds, SYSTEMTIME* pTime) {
    pTime->wYear = (WORD)(tm.tm_year + 1900);
    pTime->wMonth = (WORD)(tm.tm_mon + 1);
    pTime->wDayOfWeek = (WORD)tm.tm_wday;
    pTime->wDay = (WORD)tm.tm_mday;
    pTime->wHour = (WORD)tm.tm_hour;
    pTime->wMinute = (WORD)tm.tm_min;
    pTime->wSecond = (WORD)tm.tm_sec;
    pTime->wMilliseconds = (WORD)(nNanoseconds / 1000000);
}

}  // namespace

namespace Win32Posix {

FILETIME FileTime_FromTimespec(const struct timespec& tTime) {
    return FromU64((uint64_t)(kUnixEpochTicks + (int64_t)tTime.tv_sec * kTicksPerSecond + tTime.tv_nsec / 100));
}

struct timespec FileTime_ToTimespec(const FILETIME& tFileTime) {
    int64_t n = (int64_t)ToU64(tFileTime) - kUnixEpochTicks;
    int64_t nSeconds = n >= 0 ? n / kTicksPerSecond : -((-n + kTicksPerSecond - 1) / kTicksPerSecond);
    struct timespec t;
    t.tv_sec = (time_t)nSeconds;
    t.tv_nsec = (long)((n - nSeconds * kTicksPerSecond) * 100);
    return t;
}

}  // namespace Win32Posix

using namespace Win32Posix;

extern "C" DWORD GetTickCount() {
    return (DWORD)(clock_gettime_nsec_np(CLOCK_MONOTONIC) / 1000000);
}

DWORD timeGetTime(void) {
    return GetTickCount();
}

extern "C" BOOL QueryPerformanceCounter(LARGE_INTEGER* lpPerformanceCount) {
    lpPerformanceCount->QuadPart = (LONGLONG)(clock_gettime_nsec_np(CLOCK_UPTIME_RAW) / 100);
    return TRUE;
}

extern "C" BOOL QueryPerformanceFrequency(LARGE_INTEGER* lpFrequency) {
    lpFrequency->QuadPart = kTicksPerSecond;
    return TRUE;
}

void GetSystemTime(LPSYSTEMTIME lpSystemTime) {
    struct timespec t;
    clock_gettime(CLOCK_REALTIME, &t);
    struct tm tm;
    gmtime_r(&t.tv_sec, &tm);
    SystemTimeFromTm(tm, t.tv_nsec, lpSystemTime);
}

void GetLocalTime(LPSYSTEMTIME lpSystemTime) {
    struct timespec t;
    clock_gettime(CLOCK_REALTIME, &t);
    struct tm tm;
    localtime_r(&t.tv_sec, &tm);
    SystemTimeFromTm(tm, t.tv_nsec, lpSystemTime);
}

LONG CompareFileTime(const FILETIME* lpFileTime1, const FILETIME* lpFileTime2) {
    uint64_t a = ToU64(*lpFileTime1), b = ToU64(*lpFileTime2);
    return a < b ? -1 : a > b ? 1 : 0;
}

BOOL SystemTimeToFileTime(const SYSTEMTIME* lpSystemTime, LPFILETIME lpFileTime) {
    const SYSTEMTIME& s = *lpSystemTime;
    static const unsigned char kDays[12] = {31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    bool bLeap = (s.wYear % 4 == 0 && s.wYear % 100 != 0) || s.wYear % 400 == 0;
    if (s.wYear < 1601 || s.wYear > 30827 || s.wMonth < 1 || s.wMonth > 12 || s.wDay < 1 ||
        s.wDay > (s.wMonth == 2 && !bLeap ? 28 : kDays[s.wMonth - 1]) || s.wHour > 23 || s.wMinute > 59 ||
        s.wSecond > 59 || s.wMilliseconds > 999) {
        SetError(87);
        return FALSE;
    }
    int64_t nDays = DaysFromCivil(s.wYear, s.wMonth, s.wDay);
    int64_t nSeconds = nDays * 86400 + s.wHour * 3600 + s.wMinute * 60 + s.wSecond;
    *lpFileTime = FromU64((uint64_t)(kUnixEpochTicks + nSeconds * kTicksPerSecond + s.wMilliseconds * 10000LL));
    return TRUE;
}

BOOL FileTimeToSystemTime(const FILETIME* lpFileTime, LPSYSTEMTIME lpSystemTime) {
    uint64_t n = ToU64(*lpFileTime);
    if (n >= 0x8000000000000000ULL) {
        SetError(87);
        return FALSE;
    }
    int64_t nUnix = (int64_t)n - kUnixEpochTicks;
    int64_t nSeconds = nUnix >= 0 ? nUnix / kTicksPerSecond : -((-nUnix + kTicksPerSecond - 1) / kTicksPerSecond);
    int64_t nRest = nUnix - nSeconds * kTicksPerSecond;
    int64_t nDays = nSeconds >= 0 ? nSeconds / 86400 : -((-nSeconds + 86399) / 86400);
    int64_t nDaySecond = nSeconds - nDays * 86400;
    int64_t y;
    unsigned m, d;
    CivilFromDays(nDays, &y, &m, &d);
    lpSystemTime->wYear = (WORD)y;
    lpSystemTime->wMonth = (WORD)m;
    lpSystemTime->wDay = (WORD)d;
    lpSystemTime->wDayOfWeek = (WORD)(((nDays % 7) + 11) % 7);  // 1970-01-01 was a Thursday
    lpSystemTime->wHour = (WORD)(nDaySecond / 3600);
    lpSystemTime->wMinute = (WORD)(nDaySecond / 60 % 60);
    lpSystemTime->wSecond = (WORD)(nDaySecond % 60);
    lpSystemTime->wMilliseconds = (WORD)(nRest / 10000);
    return TRUE;
}

// The offset the local zone has at that instant, daylight saving included.
BOOL FileTimeToLocalFileTime(const FILETIME* lpFileTime, LPFILETIME lpLocalFileTime) {
    struct timespec t = FileTime_ToTimespec(*lpFileTime);
    struct tm tm;
    localtime_r(&t.tv_sec, &tm);
    *lpLocalFileTime = FromU64(ToU64(*lpFileTime) + (int64_t)tm.tm_gmtoff * kTicksPerSecond);
    return TRUE;
}

DWORD GetTimeZoneInformation(struct _TIME_ZONE_INFORMATION* lpTimeZoneInformation) {
    tzset();
    memset(lpTimeZoneInformation, 0, sizeof(*lpTimeZoneInformation));
    time_t nNow = time(nullptr);
    struct tm tm;
    localtime_r(&nNow, &tm);
    long nStandardOffset = -timezone;  // seconds east of UTC, out of daylight saving
    lpTimeZoneInformation->Bias = (LONG)(-nStandardOffset / 60);
    lpTimeZoneInformation->StandardBias = 0;
    lpTimeZoneInformation->DaylightBias = daylight ? -60 : 0;
    for (int i = 0; i < 31 && tzname[0] && tzname[0][i]; ++i) {
        lpTimeZoneInformation->StandardName[i] = (WCHAR)(unsigned char)tzname[0][i];
    }
    for (int i = 0; i < 31 && tzname[1] && tzname[1][i]; ++i) {
        lpTimeZoneInformation->DaylightName[i] = (WCHAR)(unsigned char)tzname[1][i];
    }
    if (!daylight) {
        return 0;  // TIME_ZONE_ID_UNKNOWN
    }
    return tm.tm_isdst > 0 ? 2 : 1;  // TIME_ZONE_ID_DAYLIGHT, TIME_ZONE_ID_STANDARD
}

constexpr int kQuotePair = 2;  // '' in a picture

// The date pictures of the user's locale are the en-US ones (KernelProcess.cpp gives the
// locale): short M/d/yyyy, long dddd, MMMM d, yyyy.
int GetDateFormatA(LCID Locale, DWORD dwFlags, const SYSTEMTIME* lpDate, LPCSTR lpFormat, LPSTR lpDateStr, int cchDate) {
    (void)Locale;
    static const char* const kDayNames[7] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday",
                                             "Saturday"};
    static const char* const kMonthNames[12] = {"January", "February", "March",     "April",   "May",      "June",
                                                "July",    "August",   "September", "October", "November", "December"};
    SYSTEMTIME tDate;
    if (lpDate) {
        FILETIME tCheck;
        if (!SystemTimeToFileTime(lpDate, &tCheck)) {
            return 0;
        }
        FileTimeToSystemTime(&tCheck, &tDate);  // the day of the week from the date itself
    } else {
        GetLocalTime(&tDate);
    }
    const char* szPicture = lpFormat ? lpFormat : (dwFlags & 0x2) ? "dddd, MMMM d, yyyy" : "M/d/yyyy";
    std::string sOut;
    for (const char* p = szPicture; *p;) {
        char c = *p;
        if (c == '\'' && p[1] == '\'') {  // '' is a quote
            sOut += '\'';
            p += kQuotePair;
            continue;
        }
        if (c == '\'') {  // quoted text, in which '' is a quote too
            ++p;
            while (*p) {
                if (*p == '\'') {
                    if (p[1] == '\'') {
                        sOut += '\'';
                        p += kQuotePair;
                        continue;
                    }
                    ++p;
                    break;
                }
                sOut += *p++;
            }
            continue;
        }
        if (c != 'd' && c != 'M' && c != 'y' && c != 'g') {
            sOut += *p++;
            continue;
        }
        int n = 0;
        while (p[n] == c) {
            ++n;
        }
        p += n;
        char sz[32];
        if (c == 'd') {
            if (n == 1) snprintf(sz, sizeof(sz), "%u", tDate.wDay);
            else if (n == 2) snprintf(sz, sizeof(sz), "%02u", tDate.wDay);
            else if (n == 3) snprintf(sz, sizeof(sz), "%.3s", kDayNames[tDate.wDayOfWeek]);
            else snprintf(sz, sizeof(sz), "%s", kDayNames[tDate.wDayOfWeek]);
        } else if (c == 'M') {
            if (n == 1) snprintf(sz, sizeof(sz), "%u", tDate.wMonth);
            else if (n == 2) snprintf(sz, sizeof(sz), "%02u", tDate.wMonth);
            else if (n == 3) snprintf(sz, sizeof(sz), "%.3s", kMonthNames[tDate.wMonth - 1]);
            else snprintf(sz, sizeof(sz), "%s", kMonthNames[tDate.wMonth - 1]);
        } else if (c == 'y') {
            if (n == 1) snprintf(sz, sizeof(sz), "%u", tDate.wYear % 100);
            else if (n == 2) snprintf(sz, sizeof(sz), "%02u", tDate.wYear % 100);
            else snprintf(sz, sizeof(sz), "%u", tDate.wYear);
        } else {
            snprintf(sz, sizeof(sz), "A.D.");
        }
        sOut += sz;
    }
    int nNeeded = (int)sOut.size() + 1;
    if (cchDate == 0) {
        return nNeeded;
    }
    if (cchDate < nNeeded) {
        SetError(122);  // ERROR_INSUFFICIENT_BUFFER
        return 0;
    }
    memcpy(lpDateStr, sOut.c_str(), (size_t)nNeeded);
    return nNeeded;
}

#endif  // !defined(_WIN32) && D2_BACKEND_POSIX
