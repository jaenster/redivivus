// Win32Posix: the prelude of a Win32Posix unit that needs the host's own headers beside the
// Win32 surface. Include the host headers first, then this, then nothing from the host.
//
// The Win32 surface (d2_platform.h, which reads Win32Posix.h) and the host's headers agree on
// every name but two: Winsock's u_long is 32 bits where <sys/types.h> has unsigned long, and
// Winsock's fd_set is a count and an array of sockets where <sys/select.h> has a bit set. In
// a unit that reads both, the Win32 two are renamed Win32Posix_u_long and Win32Posix_fd_set.
// The renamed u_long is the same type, but a function declared with fd_set mangles with
// its struct name, so a unit that defines an entry point taking fd_set (select,
// __WSAFDIsSet) must not use this prelude.
//
// The C++ library headers the layer uses are read here, ahead of the surface: some of them
// reach <sys/types.h> on their own.
#pragma once

#if !defined(_WIN32) && D2_BACKEND_POSIX

#include <sys/types.h>
#include <sys/select.h>
#include <atomic>
#include <chrono>
#include <condition_variable>
#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

// The host's fd_set macros (and its FD_SETSIZE, 1024 where Winsock's is 64) step aside for
// the surface's and come back after it.
#pragma push_macro("FD_SETSIZE")
#pragma push_macro("FD_ZERO")
#pragma push_macro("FD_ISSET")
#pragma push_macro("FD_SET")
#pragma push_macro("FD_CLR")
#undef FD_SETSIZE
#undef FD_ZERO
#undef FD_ISSET
#undef FD_SET
#undef FD_CLR
#define u_long Win32Posix_u_long
#define fd_set Win32Posix_fd_set
// The layer's own units are the ones the CRT redirects land on: fopen and stat stay the host's.
#define WIN32POSIX_NO_CRT_REDIRECT 1
#include "Source/D2Headers/d2_platform.h"
#undef WIN32POSIX_NO_CRT_REDIRECT
#undef u_long
#undef fd_set
#undef FD_SETSIZE
#undef FD_ZERO
#undef FD_ISSET
#undef FD_SET
#undef FD_CLR
#pragma pop_macro("FD_CLR")
#pragma pop_macro("FD_SET")
#pragma pop_macro("FD_ISSET")
#pragma pop_macro("FD_ZERO")
#pragma pop_macro("FD_SETSIZE")

#endif  // !defined(_WIN32) && D2_BACKEND_POSIX
