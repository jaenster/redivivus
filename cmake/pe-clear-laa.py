#!/usr/bin/env python3
"""Clear IMAGE_FILE_LARGE_ADDRESS_AWARE in a PE file's COFF header, in place.

GNU ld cannot leave the flag off a PE32+ image (--disable-large-address-aware is i386
only), so REDIVIVUS_X64_LOWMEM_SMOKE clears it after the link. Without the flag Windows
and wine keep a 64-bit process below 2 GB.
"""
import struct
import sys

LARGE_ADDRESS_AWARE = 0x0020

path = sys.argv[1]
with open(path, "r+b") as f:
    f.seek(0x3C)
    (pe,) = struct.unpack("<I", f.read(4))
    f.seek(pe)
    if f.read(4) != b"PE\0\0":
        sys.exit(f"{path}: not a PE image")
    f.seek(pe + 4 + 18)  # COFF header: Machine..SizeOfOptionalHeader, then Characteristics
    (chars,) = struct.unpack("<H", f.read(2))
    f.seek(pe + 4 + 18)
    f.write(struct.pack("<H", chars & ~LARGE_ADDRESS_AWARE))
