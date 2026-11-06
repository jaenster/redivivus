/* Hand-written configuration for i686-w64-mingw32 (32-bit Windows, PE/COFF).
   Replaces the autoconf-generated config.h; only what pecoff.c, dwarf.c,
   fileline.c, posix.c, read.c, alloc.c, sort.c and state.c consult. */

#define HAVE_ATOMIC_FUNCTIONS 1
#define HAVE_SYNC_FUNCTIONS 1
#define HAVE_DECL_STRNLEN 1
#define HAVE_DECL__PGMPTR 0
#define HAVE_INTTYPES_H 1
#define HAVE_STDINT_H 1
#define HAVE_STDLIB_H 1
#define HAVE_STRING_H 1
#define HAVE_SYS_STAT_H 1
#define HAVE_SYS_TYPES_H 1
#define HAVE_UNISTD_H 1
#define HAVE_WINDOWS_H 1
/* HAVE_TLHELP32_H is left undefined on purpose: with it, the first lookup opens
   and parses every loaded DLL. Only the executable carries DWARF. */
#define STDC_HEADERS 1

#ifndef _GNU_SOURCE
# define _GNU_SOURCE 1
#endif
