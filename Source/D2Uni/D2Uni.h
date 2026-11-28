// D2Uni.h - the public interface of D2Uni, the classic Mac PowerPC software renderer: its
// renderer function table. D2gfx picks it from the mode array as mode 2 (DISPLAYTYPE_SOFTWARE).
//
// The table type is the shared D2RendererFunctionsStrc of Source/D2gfx/D2gfx.public.h - the one
// the Windows renderers fill. The shipped Mac table is that table with three Mac-only slots
// inserted after slot 10 (57 slots: Mac 0-10 = shared 0-10, Mac 11-13 Mac-only, Mac 14-56 =
// shared 11-53; classic 1.00 has 56, without DrawClippedImage). Those three are declared here as
// their own record, D2MacRendererSlotsStrc, rather than forking the shared type; a Mac D2gfx
// reads both. Names are inferred (Source/D2Uni/README.md).
//
// Classic Mac PowerPC only.
#pragma once

#if !defined(_WIN32) && D2_MAC_CARBON && D2_ARCH_PPC

struct D2RendererFunctionsStrc;
struct EventRecord;
struct Point;

// Mac slots 11-13: no Windows twin. ProcessEvent is handed every Mac event; GlobalToLocal and
// LocalToGlobal translate the mouse between the screen and the game surface.
struct D2MacRendererSlotsStrc {
    /* Mac 11 */ unsigned char (*nfpProcessEvent)(EventRecord * pEvent);
    /* Mac 12 */ int (*nfpGlobalToLocal)(Point * pPoint);
    /* Mac 13 */ int (*nfpLocalToGlobal)(Point * pPoint);
};

// 1.10b classic: the 57-slot Mac table is at 104421c0 (classic 1.00 102edcd8, classic 1.13d
// 1047b32c, Carbon 1.10b 1047da08, Carbon 1.13d 104b877c).
extern D2RendererFunctionsStrc UniFunctionTable;
extern D2MacRendererSlotsStrc UniMacFunctionTable;

#endif // !defined(_WIN32) && D2_MAC_CARBON && D2_ARCH_PPC
