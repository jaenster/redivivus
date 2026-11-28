// Storm's Win32-on-Mac layer: the fatal-error and message-box helpers (FP*), 1.14d Intel
// Mac 0001e482..0001e94c. Between the heap and the Carbon helpers of the same band; the
// original file is unknown: no 1.14d string names it (see Source/StormMac/README.md).
#include "Win32Mac.h"

#if !defined(_WIN32) && D2_MAC_CARBON

#include <Carbon/Carbon.h>
#include <stdarg.h>
#include <stdio.h>

// 1.14d mac: 0001e564
int FPMessageBox(const char* szText, const char* szCaption, DWORD dwFlags);
// 1.14d mac: 0001e94c
void MAC_ConvertNewlinesToReturns(CFStringRef* pString);
// 1.14d mac: 0001eafe
unsigned char* c2pstr(char* szString);
// 1.14d mac: 00036d5e
void MACCURSOR_CallGuardProc(bool bEnable);

// 1.14d mac: 003f0e6c
// Nesting count: above zero a fatal error goes to the debugger and not to a box.
static int16_t s_nFatalToDebugger;
// 1.14d mac: 003f0e6e
// Nesting count: above zero the debugger path skips DebugStr.
static int16_t s_nFatalSkipDebugStr;
// 1.14d mac: 003f0e70
// Called before the alert is shown.
static void (*s_pfnMessageBoxCallback)(void);

// 1.14d mac: 0001e482
void FPSetFatalToDebugger(int bEnable) {
    s_nFatalToDebugger += bEnable ? 1 : -1;
}

// 1.14d mac: 0001e49d
void FPSetFatalSkipDebugStr(int bEnable) {
    s_nFatalSkipDebugStr += bEnable ? 1 : -1;
}

// 1.14d mac: 0001e4b8
BOOL FPvfatal(const char* szFormat, va_list args) {
    unsigned char szMessage[256];
    vsprintf((char*)szMessage, szFormat, args);
    if (s_nFatalToDebugger < 1) {
        FPMessageBox((char*)szMessage, "Fatal Error", 0x10);
    } else {
        c2pstr((char*)szMessage);
        if (s_nFatalSkipDebugStr < 1) {
            DebugStr(szMessage);
        }
    }
    return 1;
}

// 1.14d mac: 0001e550
void FPSetMessageBoxCallback(void (*pfnCallback)(void)) {
    s_pfnMessageBoxCallback = pfnCallback;
}

// The button captions are CFString constants per script (the layer localises to French,
// German and Japanese; everything else is English).
// 1.14d mac: 0001e564
// MessageBox on a Carbon standard alert; returns the IDxxx of the button hit.
int FPMessageBox(const char* szText, const char* szCaption, DWORD dwFlags) {
    CFStringRef cfYes, cfNo, cfIgnore, cfAbort, cfRetry, cfCancel;
    CFStringEncoding nEncoding = 0x500;
    SInt32 nScript = GetScriptManagerVariable(smRegionCode);
    if (nScript == verFrance || nScript == verFrCanada || nScript == verFrSwiss) {
        cfYes = CFSTR("Oui");
        cfNo = CFSTR("Non");
        cfIgnore = CFSTR("Ignorer");
        cfAbort = CFSTR("A");
        cfRetry = CFSTR("Nouvelle tentative");
        cfCancel = CFSTR("Annuler");
    } else if (nScript == verGermany) {
        cfYes = CFSTR("Ja");
        cfNo = CFSTR("Nein");
        cfIgnore = CFSTR("Ignorieren");
        cfRetry = CFSTR("Wiederholung");
        cfAbort = CFSTR("Abbrechen");
        cfCancel = cfAbort;
    } else if (nScript == verJapan) {
        nEncoding = 0xA01;
        cfYes = CFSTR("はい");
        cfNo = CFSTR("いいえ");
        cfIgnore = CFSTR("無視");
        cfAbort = CFSTR("中止");
        cfRetry = CFSTR("リトライ");
        cfCancel = CFSTR("キャンセル");
    } else {
        cfYes = CFSTR("Yes");
        cfNo = CFSTR("No");
        cfIgnore = CFSTR("Ignore");
        cfAbort = CFSTR("Abort");
        cfRetry = CFSTR("Retry");
        cfCancel = CFSTR("Cancel");
    }

    CFStringEncoding nUseEncoding = (dwFlags & 0x10000) ? kCFStringEncodingUTF8 : nEncoding;
    if (s_pfnMessageBoxCallback != NULL) {
        s_pfnMessageBoxCallback();
    }
    MACCURSOR_CallGuardProc(true);

    const char* szExplanation = szText;
    if (szCaption == NULL || *szCaption == '\0') {
        szExplanation = "";
        szCaption = szText;
    }
    CFStringRef cfMessage = CFStringCreateWithCString(NULL, szCaption, nUseEncoding);
    CFStringRef cfExplain = CFStringCreateWithCString(NULL, szExplanation, nUseEncoding);
    MAC_ConvertNewlinesToReturns(&cfMessage);
    MAC_ConvertNewlinesToReturns(&cfExplain);

    AlertType nAlertType = kAlertNoteAlert;
    if ((dwFlags & 0xF0) == 0x30) {
        nAlertType = kAlertCautionAlert;
    } else if ((dwFlags & 0xF0) == 0x10) {
        nAlertType = kAlertStopAlert;
    }
    FlushEvents(8, 0);

    UInt32 nType = dwFlags & 0xF;
    UInt16 nCancelButton = 0;
    CFStringRef cfDefault = CFSTR("OK");
    CFStringRef cfOther = NULL;
    CFStringRef cfCancelText = NULL;
    UInt16 nDefaultButton = 1;
    switch (nType) {
    case 0:
        break;
    case 1:
        nCancelButton = 2;
        cfCancelText = cfCancel;
        break;
    case 2:
        nDefaultButton = 2;
        cfDefault = cfAbort;
        cfOther = cfIgnore;
        cfCancelText = cfRetry;
        if ((dwFlags & 0x100) == 0 && (nDefaultButton = 3, (dwFlags & 0x200) == 0)) {
            nDefaultButton = 1;
        }
        break;
    case 3:
        nDefaultButton = 2;
        cfDefault = cfYes;
        cfOther = cfCancel;
        cfCancelText = cfNo;
        if ((dwFlags & 0x100) == 0 && (nDefaultButton = 3, (dwFlags & 0x200) == 0)) {
            nDefaultButton = 1;
        }
        break;
    case 4:
        nDefaultButton = ((dwFlags >> 8) & 1) + 1;
        cfDefault = cfYes;
        cfCancelText = cfNo;
        break;
    case 5:
        nDefaultButton = ((dwFlags >> 8) & 1) + 1;
        cfDefault = cfRetry;
        cfCancelText = cfCancel;
        break;
    default:
        fp_fatal("Unknown MessageBox type = %08x", nType);
        return 0;
    }

    AlertStdCFStringAlertParamRec tParams;
    tParams.version = 1;
    tParams.movable = true;
    tParams.helpButton = false;
    tParams.defaultText = cfDefault;
    tParams.cancelText = cfCancelText;
    tParams.otherText = cfOther;
    tParams.defaultButton = nDefaultButton;
    tParams.cancelButton = nCancelButton;
    tParams.position = 0;
    tParams.flags = 0;
    DialogRef pAlert;
    DialogItemIndex nItemHit;
    CreateStandardAlert(nAlertType, cfMessage, cfExplain, &tParams, &pAlert);
    RunStandardAlert(pAlert, NULL, &nItemHit);
    CFRelease(cfMessage);
    CFRelease(cfExplain);
    if ((dwFlags & 0x20000000) == 0) {
        MACCURSOR_CallGuardProc(false);
    }

    UInt32 nResult = 1;
    switch (nType) {
    case 1:
        nResult = (nItemHit != 1) + 1;
        break;
    case 2:
        nResult = 3;
        if (nItemHit != 1) {
            nResult = (nItemHit == 2) ? 4 : 5;
        }
        break;
    case 3:
        nResult = 6;
        if (nItemHit != 1) {
            nResult = (nItemHit == 2) ? 7 : 2;
        }
        break;
    case 4:
        nResult = (nItemHit != 1) | 6;
        break;
    case 5:
        nResult = (nItemHit == 1) * 2 + 2;
        break;
    }
    return nResult;
}

#endif  // D2_OS_MAC
