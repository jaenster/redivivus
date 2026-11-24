// Redivivus' own code, not recovered from any Blizzard binary.
//
// The dedicated server's console log: one timestamped line per call on stdout, flushed at
// once so a supervisor or a pipe sees it as it happens. The engine keeps its own log files
// through Fog; this is only what the host itself reports.
#pragma once

#include <cstdarg>
#include <cstdio>
#include <ctime>

namespace Redivivus::Server {

inline void ServerLogV(const char * szFormat, va_list args) {
    char szTime[32];
    const time_t tNow = time(nullptr);
    const struct tm * pNow = localtime(&tNow);
    if (!pNow || !strftime(szTime, sizeof(szTime), "%Y-%m-%d %H:%M:%S", pNow)) {
        szTime[0] = '\0';
    }
    printf("%s d2server: ", szTime);
    vprintf(szFormat, args);
    putchar('\n');
    fflush(stdout);
}

inline void ServerLog(const char * szFormat, ...) {
    va_list args;
    va_start(args, szFormat);
    ServerLogV(szFormat, args);
    va_end(args);
}

}  // namespace Redivivus::Server
