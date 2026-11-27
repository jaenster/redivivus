// unknown_tsm.cpp - the Mac shell's Text Services Manager input: the TSM document, the four
// text-services AppleEvent handlers, the input callback, and the text conversions between a
// script and UTF-16 (TEC) the shared chat and text code calls.
//
// 1.14d Intel Mac (DiabloII_macho) 0019dd07..0019e8a1. No binary records this unit's name.
// It owns 003b8d84..8c and 00536ca8..00536cf0; MAC_HandleSuspendResumeEvent (unknown_main.cpp)
// calls its 35-byte activate/deactivate functions out of line; Game/Main.cpp
// (MISC_InitD2IniAndIgnorelist, 0019e8a1) follows. See Source/MacSpecific/README.md.
//
// Mac-only: the guard is spelled `#if !defined(_WIN32) && D2_OS_MAC` (see
// Source/MacSpecific/README.md).
#include "D2Arch.h"

#if !defined(_WIN32) && D2_OS_MAC

#include <Carbon/Carbon.h>
#include <pthread.h>
#include <stdlib.h>

#include <QuickTime/QuickTime.h>
#include <CoreServices/CoreServices.h>
#include <stdio.h>
#include <string.h>

#include "Source/MacSpecific/Source/MacShell/QuickDrawMac.h"

#include "Source/StormMac/H/stpl.h"

typedef int BOOL;
namespace D2Lang::StrTable::strtable {
// 1.14d mac: 002b98b1
uint32_t MISC_GetLanguageCode(void);
}
using D2Lang::StrTable::strtable::MISC_GetLanguageCode;
void MAC_DeactivateTSMDocument(void);
void MAC_ActivateTSMDocument(void);
void MAC_ShutdownTSM(void);
void MAC_DisposeTSMDocument(void);
OSErr MAC_HandleTSMUpdateActiveInputArea(const AppleEvent* pAppleEvent);
OSErr MAC_HandleTSMShowHideInputWindow(const AppleEvent* pAppleEvent, AppleEvent* pReply, SRefCon nRefCon);
OSErr MAC_HandleTSMPosToOffset(const AppleEvent* pAppleEvent, AppleEvent* pReply);
OSErr MAC_HandleTSMOffsetToPos(const AppleEvent* pAppleEvent, AppleEvent* pReply);
void FUN_0019e432(void);

// 1.14d mac: 0019dd07
void FUN_0019dd07(void) {
}

// 1.14d mac: 0019dd0c
// How many characters a run of bytes is, a double-byte character counting as one.
int MAC_CountMultiByteChars(const char* pText, int nBytes) {
    uint32_t nLanguage = MISC_GetLanguageCode();
    short nScript = 3;
    if (nLanguage != 12 && nLanguage != 7) {
        if (nLanguage == 6) {
            nScript = 1;
        } else {
            nScript = 0;
        }
    }
    int nCount = 0;
    while (0 < nBytes) {
        nCount++;
        if (CharacterByteType((Ptr)pText, 0, nScript) == -1) {
            nBytes -= 2;
            pText += sizeof(uint16_t);
        } else {
            nBytes -= 1;
            pText += 1;
        }
    }
    return nCount;
}

// 1.14d mac: 0019dd7d
// The text services 'shiw' handler: nothing to do.
OSErr MAC_HandleTSMShowHideInputWindow(const AppleEvent* pAppleEvent, AppleEvent* pReply, SRefCon nRefCon) {
    return 0;
}

// 1.14d mac: 0019dd84
// The text services 'st2p' handler: a fixed position for the input window.
OSErr MAC_HandleTSMOffsetToPos(const AppleEvent* pAppleEvent, AppleEvent* pReply) {
    short nError = 0;
    Point ptPos;
    Fixed nPointSize;
    DescType nActualType;
    long nOffset;
    long nRefCon;
    Size nActualSize;
    *(uint32_t*)&ptPos = 0x75307530;
    nPointSize = 0x5A0000;
    OSErr nResult = AEGetParamPtr(pAppleEvent, 'ofst', typeLongInteger, &nActualType, &nOffset, 4, &nActualSize);
    if (nResult == 0) {
        nResult = AEGetParamPtr(pAppleEvent, 'refc', typeLongInteger, &nActualType, &nRefCon, 4, &nActualSize);
        if (nResult == 0) {
            nResult = AEPutParamPtr(pReply, 'gpos', typeQDPoint, &ptPos, 4);
            if (nResult == 0) {
                nResult = AEPutParamPtr(pReply, 'ptps', typeFixed, &nPointSize, 4);
                if (nResult == 0) {
                    nResult = AEPutParamPtr(pReply, 'errn', typeShortInteger, &nError, 2);
                }
            }
        }
    }
    return nResult;
}

// 1.14d mac: 0019dea7
// The text services 'p2st' handler: offset 0 in the active region.
OSErr MAC_HandleTSMPosToOffset(const AppleEvent* pAppleEvent, AppleEvent* pReply) {
    DescType nActualType;
    long nRefCon;
    Point ptPos;
    Size nActualSize;
    OSErr nResult = AEGetParamPtr(pAppleEvent, 'refc', typeLongInteger, &nActualType, &nRefCon, 4, &nActualSize);
    if (nResult == 0) {
        nResult = AEGetParamPtr(pAppleEvent, 'cpos', typeQDPoint, &nActualType, &ptPos, 4, &nActualSize);
        if (nResult == 0) {
            long nOffset = 0;
            nResult = AEPutParamPtr(pReply, 'ofst', typeLongInteger, &nOffset, 4);
            if (nResult == 0) {
                short nRegionClass = 2;
                nResult = AEPutParamPtr(pReply, 'rgnc', typeShortInteger, &nRegionClass, 2);
            }
        }
    }
    return nResult;
}

void MAC_ConvertTextToUnicode(const char* pSrc, char* wszDest, int nSrcLength, int nDestSize, int nScript);

// 1.14d mac: 003b8d84
// Where the text services' input goes: the text, whether it is committed, and the highlight range.
static void (*s_pfnTSMInput)(const char*, bool, int*) = (void (*)(const char*, bool, int*))FUN_0019dd07;

// 1.14d mac: 0019df93
// The text services 'updt' handler: the active input area's text converted to Unicode and
// handed to the input callback with its highlight range.
OSErr MAC_HandleTSMUpdateActiveInputArea(const AppleEvent* pAppleEvent) {
    // The descriptors start zeroed (not typeNull) and each has its handle cleared again right
    // before AEGetParamDesc fills it: the asm stores both.
    AEDesc tHiliteDesc = {0, NULL};
    AEDesc tUpdateDesc = {0, NULL};
    AEDesc tTextDesc = {0, NULL};
    DescType nActualType;
    long nFixedLength;
    long nRefCon;
    Size nActualSize;
    long nPinRange;
    int aHiliteRange[2];
    UniChar wszText[256];
    Ptr pText = NULL;
    Ptr pHiliteRanges = NULL;
    aHiliteRange[1] = 0;
    aHiliteRange[0] = 0;
    bzero(wszText, sizeof(wszText));
    OSErr nResult = AEGetParamPtr(pAppleEvent, 'refc', typeLongInteger, &nActualType, &nRefCon, 4, &nActualSize);
    if (nResult == 0) {
        tTextDesc.dataHandle = NULL;
        nResult = AEGetParamDesc(pAppleEvent, 'kdat', typeText, &tTextDesc);
        if (nResult == 0) {
            AEGetParamPtr(pAppleEvent, 'fixl', typeLongInteger, &nActualType, &nFixedLength, 4, &nActualSize);
            tHiliteDesc.dataHandle = NULL;
            AEGetParamDesc(pAppleEvent, 'hrng', typeTextRangeArray, &tHiliteDesc);
            tUpdateDesc.dataHandle = NULL;
            AEGetParamDesc(pAppleEvent, 'udng', typeTextRangeArray, &tUpdateDesc);
            AEGetParamPtr(pAppleEvent, 'pnrg', typeTextRange, &nActualType, &nPinRange, 4, &nActualSize);
            Size nTextSize = AEGetDescDataSize(&tTextDesc);
            pText = NewPtr(nTextSize);
            nResult = MemError();
            if (nResult == 0 && (nResult = AEGetDescData(&tTextDesc, pText, nTextSize)) == 0) {
                if (nFixedLength == -1) {
                    nFixedLength = nTextSize;
                }
                if (tHiliteDesc.dataHandle != NULL) {
                    Size nRangesSize = AEGetDescDataSize(&tHiliteDesc);
                    pHiliteRanges = NewPtr(nRangesSize);
                    nResult = MemError();
                    if (nResult != 0 || (nResult = AEGetDescData(&tHiliteDesc, pHiliteRanges, nRangesSize)) != 0) {
                        goto cleanup;
                    }
                    TextRangeArray* pArray = (TextRangeArray*)pHiliteRanges;
                    TextRange* pRange = pArray->fRange;
                    for (int i = pArray->fNumOfRanges; 0 < i; i--) {
                        if (pRange->fHiliteStyle == 5) {
                            aHiliteRange[0] = MAC_CountMultiByteChars(pText, pRange->fStart);
                            aHiliteRange[1] = MAC_CountMultiByteChars(pText, pRange->fEnd);
                            break;
                        }
                        pRange++;
                    }
                }
                MAC_ConvertTextToUnicode(pText, (char*)wszText, nTextSize < 0x100 ? nTextSize : 0xFF, 0x200, -0x80);
                s_pfnTSMInput((const char*)wszText, nFixedLength == nTextSize, aHiliteRange);
                nResult = 0;
            }
            goto cleanup;
        }
    }
cleanup:
    AEDisposeDesc(&tTextDesc);
    AEDisposeDesc(&tHiliteDesc);
    AEDisposeDesc(&tUpdateDesc);
    DisposePtr(pHiliteRanges);
    DisposePtr(pText);
    return nResult;
}

// 1.14d mac: 003b8d8c
// The script the TEC converter below was made for; none until the first conversion.
static int s_nTECScript = -0x80;
// 1.14d mac: 00536cf0
static TECObjectRef s_hTECConverter;

// 1.14d mac: 0019e2fb
// Converts text in a script (the language's by default, -0x80) to Unicode, with a terminator.
void MAC_ConvertTextToUnicode(const char* pSrc, char* wszDest, int nSrcLength, int nDestSize, int nScript) {
    ByteCount nOutLength;
    ByteCount nSrcRead;
    TextEncoding nEncoding;
    if (nScript == -0x80) {
        uint32_t nLanguage = MISC_GetLanguageCode();
        nScript = 3;
        if (nLanguage != 12 && nLanguage != 7) {
            if (nLanguage == 6) {
                nScript = 1;
            } else {
                nScript = 0;
            }
        }
    }
    if (nScript != s_nTECScript) {
        if (s_hTECConverter != NULL) {
            TECDisposeConverter(s_hTECConverter);
            s_hTECConverter = NULL;
        }
        s_nTECScript = nScript;
        UpgradeScriptInfoToTextEncoding((short)nScript, kTextLanguageDontCare, kTextRegionDontCare, NULL, &nEncoding);
        TextEncoding nUnicodeEncoding = CreateTextEncoding(kTextEncodingUnicodeDefault, 0, 0);
        TECCreateConverter(&s_hTECConverter, nEncoding, nUnicodeEncoding);
    }
    TECConvertText(s_hTECConverter, (ConstTextPtr)pSrc, nSrcLength, &nSrcRead, (TextPtr)wszDest, nDestSize - 2, &nOutLength);
    ByteCount nConverted = nOutLength;
    TECFlushText(s_hTECConverter, (TextPtr)wszDest + nOutLength, (nDestSize - 2) - nOutLength, &nOutLength);
    *(uint16_t*)(wszDest + nConverted + nOutLength) = 0;
}

// 1.14d mac: 00536ca8
// The Script Manager variable 8 as the shell found it.
static long s_nSavedScriptVariable;
// 1.14d mac: 00536ce8
// The text services document, while there is one.
static TSMDocumentID s_hTSMDocument;

// 1.14d mac: 0019e432
// Takes the Script Manager's variable 8 down to 0, installs the four text services AppleEvent
// handlers and drops any text services document.
void FUN_0019e432(void) {
    s_nSavedScriptVariable = GetScriptManagerVariable(8);
    SetScriptManagerVariable(8, 0);
    static AEEventHandlerUPP s_pfnUpdate = NewAEEventHandlerUPP((AEEventHandlerProcPtr)MAC_HandleTSMUpdateActiveInputArea);
    static AEEventHandlerUPP s_pfnShowHide = NewAEEventHandlerUPP((AEEventHandlerProcPtr)MAC_HandleTSMShowHideInputWindow);
    static AEEventHandlerUPP s_pfnPosToOffset = NewAEEventHandlerUPP((AEEventHandlerProcPtr)MAC_HandleTSMPosToOffset);
    static AEEventHandlerUPP s_pfnOffsetToPos = NewAEEventHandlerUPP((AEEventHandlerProcPtr)MAC_HandleTSMOffsetToPos);
    AEInstallEventHandler('tsvc', 'updt', s_pfnUpdate, 0, false);
    AEInstallEventHandler('tsvc', 'shiw', s_pfnShowHide, 0, false);
    AEInstallEventHandler('tsvc', 'p2st', s_pfnPosToOffset, 0, false);
    AEInstallEventHandler('tsvc', 'st2p', s_pfnOffsetToPos, 0, false);
    MAC_DisposeTSMDocument();
}

// 1.14d mac: 0019e5e9
// Fixes, deactivates and deletes the text services document; the keyboard script goes back to
// the system's.
void MAC_DisposeTSMDocument(void) {
    if (s_hTSMDocument != 0) {
        FixTSMDocument(s_hTSMDocument);
        DeactivateTSMDocument(s_hTSMDocument);
        DeleteTSMDocument(s_hTSMDocument);
        s_hTSMDocument = 0;
    }
    KeyScript(0);
    KeyScript(-12);
}

// 1.14d mac: 0019e64a
// Where the text services' input goes; the empty handler for none.
void MAC_SetTSMInputCallback(void (*pfnCallback)(const char*, bool, int*)) {
    void (*pfnInput)(const char*, bool, int*) = (void (*)(const char*, bool, int*))FUN_0019dd07;
    if (pfnCallback != NULL) {
        pfnInput = pfnCallback;
    }
    s_pfnTSMInput = pfnInput;
}

// 1.14d mac: 0019e669
void MAC_DeactivateTSMDocument(void) {
    if (s_hTSMDocument != 0) {
        DeactivateTSMDocument(s_hTSMDocument);
    }
}

// 1.14d mac: 0019e68c
void MAC_ActivateTSMDocument(void) {
    if (s_hTSMDocument != 0) {
        ActivateTSMDocument(s_hTSMDocument);
    }
}

// 1.14d mac: 0019e6af
// Makes the text services document, active and without the input window, and turns the
// keyboard scripts on.
void MAC_CreateTSMDocument(void) {
    if (s_hTSMDocument == 0) {
        OSType nInterfaceType = 'tsvc';
        NewTSMDocument(1, &nInterfaceType, &s_hTSMDocument, 'TSM!');
        ActivateTSMDocument(s_hTSMDocument);
        UseInputWindow(s_hTSMDocument, false);
    }
    KeyScript(-7);
    KeyScript(-3);
}

// 1.14d mac: 0019e736
void MAC_ShutdownTSM(void) {
    MAC_DisposeTSMDocument();
    SetScriptManagerVariable(8, s_nSavedScriptVariable);
}

// 1.14d mac: 003b8d88
// The script the converter below was made for.
static int s_nTECScriptToText = -0x80;
// 1.14d mac: 00536cec
static TECObjectRef s_hTECConverterToText;

// 1.14d mac: 0019e764
// The reverse of MAC_ConvertTextToUnicode: UTF-16 to a script, with a one-byte terminator.
void MAC_ConvertUnicodeToText(const char* wszSrc, char* szDest, int nSrcChars, int nDestSize, int nScript) {
    ByteCount nOutLength;
    ByteCount nSrcRead;
    TextEncoding nEncoding;
    if (nScript == -0x80) {
        uint32_t nLanguage = MISC_GetLanguageCode();
        nScript = 3;
        if (nLanguage != 12 && nLanguage != 7) {
            if (nLanguage == 6) {
                nScript = 1;
            } else {
                nScript = 0;
            }
        }
    }
    if (nScript != s_nTECScriptToText) {
        if (s_hTECConverterToText != NULL) {
            TECDisposeConverter(s_hTECConverterToText);
            s_hTECConverterToText = NULL;
        }
        s_nTECScriptToText = nScript;
        TextEncoding nUnicodeEncoding = CreateTextEncoding(kTextEncodingUnicodeDefault, 0, 0);
        UpgradeScriptInfoToTextEncoding((short)s_nTECScriptToText, kTextLanguageDontCare, kTextRegionDontCare, NULL, &nEncoding);
        TECCreateConverter(&s_hTECConverterToText, nUnicodeEncoding, nEncoding);
    }
    TECConvertText(s_hTECConverterToText, (ConstTextPtr)wszSrc, nSrcChars * 2, &nSrcRead, (TextPtr)szDest, nDestSize - 1, &nOutLength);
    ByteCount nConverted = nOutLength;
    TECFlushText(s_hTECConverterToText, (TextPtr)szDest + nOutLength, (nDestSize - 1) - nOutLength, &nOutLength);
    szDest[nConverted + nOutLength] = 0;
}

#endif  // D2_OS_MAC
