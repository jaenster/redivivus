// STUB HEADER - Redivivus-owned.
//
// What the shared tree headers (Source/D2Headers/d2_platform.h) expect from the C library that
// a Mac C library supplies differently: the MSVC-only errno_t, and FILE, which d2_platform.h
// would otherwise define as MSVC's struct _iobuf over the Mac library's own. Included first by
// the one D2Rave unit that includes a shared tree header (raveRenderer.cpp).
//
// Classic Mac PowerPC only. See README.md beside this file.
#pragma once

#if !defined(_WIN32) && D2_OS_MAC && D2_ARCH_PPC

#define _FILE_DEFINED 1
typedef int errno_t;

#endif // !defined(_WIN32) && D2_OS_MAC && D2_ARCH_PPC
