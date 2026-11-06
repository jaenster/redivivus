/**
 * Data defined in D2Lang/StrTable/strtable.cpp that other translation units name.
 * Binary: Game.exe
 */

#pragma once
#if defined(D2_MODULE_LIBRARIES) && defined(D2_MODULE_STRICT) && !defined(D2_GATE_FROM_ROOT) && !defined(D2_SEES_D2Lang)
#error "D2Lang header included by a module that may not use D2Lang (original DLL import graph; see module-gates.tsv)"
#endif

#include "Source/D2Headers/d2_platform.h"

extern char gaLangAbbreviation_ChineseTraditional[4];
extern char gaLangAbbreviation_Japanese[4];
extern char s_SIN_006e0264[4];
extern char s_HAN_006e0268[4];
extern char gaLangAbbreviation_Korean[4];
extern char gaLangAbbreviation_Cyrillic[4];
extern char s_ITA_006e0274[4];
extern char s_BRA_006e0278[4];
extern char s_POR_006e027c[4];
extern char s_FRA_006e0280[4];
extern char s_DEU_006e0284[4];
extern char gaLangNameFormat_Nominative[4];

namespace D2Lang::StrTable::strtable {
extern void * gpUntranslatedStringCacheHead;
extern uint16_t * * gpStringIndexBase;
extern uint16_t * * gpStringIndexPatchD2;
extern uint16_t * * gpStringIndexExpansion;
extern uint16_t * gpStringBufferBase;
extern uint16_t * gpStringBufferPatchD2;
extern uint16_t * gpStringBufferExpansion;
extern int gnStringTableRefCount;
} // namespace D2Lang::StrTable::strtable
