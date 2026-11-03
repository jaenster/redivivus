/**
 * Data defined in D2Client/UI/Chat.cpp that other translation units name.
 * Binary: Game.exe
 */

#pragma once

#include "d2_platform.h"

extern int cChatCmdExclaim;
extern char cChatCmdSlash[2];
extern char gaChatInputBuffer;
extern uint32_t gnChatInputTextCursorPosition;
extern int gnChatInputTextLength;
extern uint32_t gnIMEInputMode;
extern uint16_t gwcImeCompositionBuffer[32];
// WCHAR[32] IME composition string (64 bytes, 007bc850..007bc88f), drawn by CHAT_DrawIMEComposition. Filled at 0047ac49 from the GCS_COMPSTR (8) string converted by MultiByteToWideChar, capped at 31 characters; element [31] (007bc88e) is the terminator written at 0047ac5f.
// Zeroed as 16 dwords by StaticInit_6cab30; cleared ([0]=0) at 0047ab98 after the GCS_RESULTSTR (0x800) commit, in INPUT_HandleIMEStartComposition (0047acb8), INPUT_HandleIMEEndComposition (0047ad1c), CHAT_InitIME (0047b112) and CHAT_ShutdownIME (0047b379).
// Provenance: write xrefs in 1.14d; the interior label gpIMECompositionStringEnd (007bc88e) was folded into this array.
extern uint16_t gwcIMECompositionString[32];
