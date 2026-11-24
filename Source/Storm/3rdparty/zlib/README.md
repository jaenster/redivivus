# zlib 1.2.8

Storm links zlib 1.2.8 (`inflateInit_` checks the version string "1.2.8"). Where this tree's
function bodies here are the genuine zlib 1.2.8 sources, they are altered copies: the C was
run through the preprocessor and its identifiers renamed to this tree's spelling (types, data,
parameters, cross-unit calls), with casts where the tree's types differ from zlib's. Each such
body is the one VC8 compiles to the bytes of Diablo II 1.14d. The other bodies are generated.

Configuration, read from the 1.14d code: the default zconf.h for Win32 (STDC, FAR empty, no
Z_SOLO), BYFOUR crc32 with its little- and big-endian paths, the static tables (no
DYNAMIC_CRC_TABLE, no BUILDFIXED), gzip support (no NO_GZIP), no FASTEST, no ZLIB_WINAPI, no
DEBUG; zlibCompileFlags() is 0x55. Every external function is `__fastcall`, as `/Gr` makes it, and the compiler inlines static helpers as `/Ob2` does.

Source: https://zlib.net/fossils/zlib-1.2.8.tar.gz,
SHA-256 36658cb768a54c1d4dec43c3116c27ed893e88b02ecfcb44f2166f9c0b7f2a0d.

## Licence

```

  Copyright (C) 1995-2013 Jean-loup Gailly and Mark Adler

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you must not
     claim that you wrote the original software. If you use this software
     in a product, an acknowledgment in the product documentation would be
     appreciated but is not required.
  2. Altered source versions must be plainly marked as such, and must not be
     misrepresented as being the original software.
  3. This notice may not be removed or altered from any source distribution.

  Jean-loup Gailly        Mark Adler
  jloup@gzip.org          madler@alumni.caltech.edu
```
