// Win32Posix: the scripted run - input and screenshots from a file, for a run with nobody at the
// keyboard (a test, a bounded run on a desktop the harness cannot capture or drive). Ours: no
// Windows or Mac build has it, and it does nothing unless D2_POSIX_SCRIPT names a file.
//
// One step a line, at a time in milliseconds since the first message pump; '#' starts a comment.
//   <ms> move <x> <y>           the mouse to a point of the main window's client area
//   <ms> click <x> <y> [right]  move there, then press and release the button
//   <ms> down <x> <y> [right]   move there and press the button (held: walking, a drag)
//   <ms> up <x> <y> [right]     move there and release it
//   <ms> type <text>            the text as typed, through SDL text input (WM_CHAR)
//   <ms> key <name>             press and release a key, by its SDL name ("Return", "Escape", "A")
//   <ms> keydown <name>         press a key and keep it down ("Left Shift" to attack in place)
//   <ms> keyup <name>           release it
//   <ms> shot <path>            save the next presented frame as a BMP (the renderer's side:
//                               Win32Posix_TakeShotRequest)
//   <ms> focus lost|gained      the window loses or regains the keyboard focus, as switching
//                               to another application and back does
//   <ms> quit                   ask the main window to close, as Cmd-Q does
// The file is read again while the game runs: a line appended later as `+<ms> <verb> ...` runs
// that long after it is read, so a run can be driven step by step (append, wait for the shot,
// look, append). At most kMaxSteps steps are kept. The input goes through the same translation
// as SDL's own events, so it reaches the game as the window messages a person's would.
#if !defined(_WIN32) && D2_BACKEND_POSIX

#include "Source/StormMac/Win32Posix/UserInternal.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

namespace Win32Posix {

namespace {

// The C++ library's containers are not used here: they read <sys/types.h>, whose u_long and
// fd_set are not the Win32 surface's (UserInternal.h).
constexpr size_t kMaxSteps = 2048;
constexpr uint64_t kRereadMs = 200;

struct Step {
    uint64_t nAtMs;
    char szVerb[16];
    char szArg[512];  // the path or key name
    int nX, nY;
    bool bRight;
};

struct Script {
    bool bLoaded;
    Step aSteps[kMaxSteps];
    size_t nSteps;
    size_t nNext;
    uint64_t nStartMs;
    char szShotPath[512];  // asked for, not yet taken
    char szPath[1024];     // the script, read again for appended steps
    long nReadOffset;      // the file offset read up to (whole lines only)
    uint64_t nLastReadMs;
};

Script& S() {
    static Script tScript;
    return tScript;
}

// Reads the whole lines from the read offset on; `+<ms>` times are taken from nNowMs.
void ReadSteps(Script& s, uint64_t nNowMs) {
    FILE* pFile = fopen(s.szPath, "r");
    if (!pFile) {
        return;
    }
    if (fseek(pFile, s.nReadOffset, SEEK_SET) != 0) {
        fclose(pFile);
        return;
    }
    char szLine[1024];
    while (fgets(szLine, sizeof(szLine), pFile)) {
        size_t nLineLen = strlen(szLine);
        if (nLineLen == 0 || szLine[nLineLen - 1] != '\n') {
            break;  // a line still being written: read it next time
        }
        s.nReadOffset += (long)nLineLen;
        char* pHash = strchr(szLine, '#');
        if (pHash) {
            *pHash = '\0';
        }
        const char* pText = szLine;
        while (*pText == ' ' || *pText == '\t') {
            pText++;
        }
        bool bRelative = *pText == '+';
        if (bRelative) {
            pText++;
        }
        unsigned long long nAt;
        char szVerb[32], szRest[960] = "";
        int nRead = sscanf(pText, "%llu %15s %959[^\n]", &nAt, szVerb, szRest);
        if (nRead < 2 || s.nSteps == kMaxSteps) {
            continue;
        }
        Step& t = s.aSteps[s.nSteps++];
        memset(&t, 0, sizeof(t));
        t.nAtMs = bRelative ? nNowMs + nAt : nAt;
        snprintf(t.szVerb, sizeof(t.szVerb), "%s", szVerb);
        if (!strcmp(t.szVerb, "move") || !strcmp(t.szVerb, "click") || !strcmp(t.szVerb, "down") ||
            !strcmp(t.szVerb, "up")) {
            char szButton[16] = "";
            sscanf(szRest, "%d %d %15s", &t.nX, &t.nY, szButton);
            t.bRight = strcmp(szButton, "right") == 0;
        } else {
            snprintf(t.szArg, sizeof(t.szArg), "%s", szRest);
            size_t nLen = strlen(t.szArg);
            while (nLen && (t.szArg[nLen - 1] == ' ' || t.szArg[nLen - 1] == '\r')) {
                t.szArg[--nLen] = '\0';
            }
        }
    }
    fclose(pFile);
}

void Load(Script& s) {
    s.bLoaded = true;
    const char* szPath = getenv("D2_POSIX_SCRIPT");
    if (!szPath || !*szPath) {
        return;
    }
    snprintf(s.szPath, sizeof(s.szPath), "%s", szPath);
    FILE* pFile = fopen(szPath, "r");
    if (!pFile) {
        fprintf(stderr, "Win32Posix: script %s: cannot open\n", szPath);
        s.szPath[0] = '\0';
        return;
    }
    fclose(pFile);
    s.nStartMs = SDL_GetTicks();
    ReadSteps(s, 0);
    fprintf(stderr, "Win32Posix: script %s: %zu steps\n", szPath, s.nSteps);
}

void Mouse(Wnd* pWnd, const Step& t, bool bButton, bool bDown) {
    float fX, fY;
    ClientToSdl(pWnd, t.nX, t.nY, &fX, &fY);
    SDL_Event tEvent;
    SDL_zero(tEvent);
    if (!bButton) {
        tEvent.type = SDL_EVENT_MOUSE_MOTION;
        tEvent.motion.windowID = pWnd->nSdlId;
        tEvent.motion.x = fX;
        tEvent.motion.y = fY;
    } else {
        tEvent.type = bDown ? SDL_EVENT_MOUSE_BUTTON_DOWN : SDL_EVENT_MOUSE_BUTTON_UP;
        tEvent.button.windowID = pWnd->nSdlId;
        tEvent.button.button = t.bRight ? SDL_BUTTON_RIGHT : SDL_BUTTON_LEFT;
        tEvent.button.down = bDown;
        tEvent.button.clicks = 1;
        tEvent.button.x = fX;
        tEvent.button.y = fY;
    }
    TranslateSdlEvent(tEvent);
}

void Text(Wnd* pWnd, const Step& t) {
    SDL_Event tEvent;
    SDL_zero(tEvent);
    tEvent.type = SDL_EVENT_TEXT_INPUT;
    tEvent.text.windowID = pWnd->nSdlId;
    tEvent.text.text = t.szArg;
    TranslateSdlEvent(tEvent);
}

void Key(Wnd* pWnd, const Step& t, bool bPress, bool bRelease) {
    SDL_Keycode nKey = SDL_GetKeyFromName(t.szArg);
    SDL_Scancode nScan = SDL_GetScancodeFromName(t.szArg);
    if (nKey == SDLK_UNKNOWN && nScan == SDL_SCANCODE_UNKNOWN) {
        fprintf(stderr, "Win32Posix: script: no key named \"%s\"\n", t.szArg);
        return;
    }
    for (int nPass = 0; nPass < 2; nPass++) {
        bool bDown = nPass == 0;
        if ((bDown && !bPress) || (!bDown && !bRelease)) {
            continue;
        }
        SDL_Event tEvent;
        SDL_zero(tEvent);
        tEvent.type = bDown ? SDL_EVENT_KEY_DOWN : SDL_EVENT_KEY_UP;
        tEvent.key.windowID = pWnd->nSdlId;
        tEvent.key.key = nKey;
        tEvent.key.scancode = nScan;
        tEvent.key.down = bDown;
        TranslateSdlEvent(tEvent);
    }
}

}  // namespace

void RunScript() {
    Script& s = S();
    if (!s.bLoaded) {
        Load(s);
    }
    uint64_t nNow = SDL_GetTicks() - s.nStartMs;
    if (s.szPath[0] && nNow - s.nLastReadMs >= kRereadMs) {
        s.nLastReadMs = nNow;
        ReadSteps(s, nNow);
    }
    while (s.nNext < s.nSteps && s.aSteps[s.nNext].nAtMs <= nNow) {
        const Step& t = s.aSteps[s.nNext++];
        Wnd* pWnd = WndFromHandle(MainWindow());
        fprintf(stderr, "Win32Posix: script %llu ms: %s %s\n", (unsigned long long)nNow, t.szVerb, t.szArg);
        if (!strcmp(t.szVerb, "shot")) {
            snprintf(s.szShotPath, sizeof(s.szShotPath), "%s", t.szArg);
        } else if (!strcmp(t.szVerb, "quit")) {
            SDL_Event tEvent;
            SDL_zero(tEvent);
            tEvent.type = SDL_EVENT_QUIT;
            TranslateSdlEvent(tEvent);
        } else if (!pWnd) {
            fprintf(stderr, "Win32Posix: script: no main window yet\n");
        } else if (!strcmp(t.szVerb, "move")) {
            Mouse(pWnd, t, false, false);
        } else if (!strcmp(t.szVerb, "click")) {
            Mouse(pWnd, t, false, false);
            Mouse(pWnd, t, true, true);
            Mouse(pWnd, t, true, false);
        } else if (!strcmp(t.szVerb, "down") || !strcmp(t.szVerb, "up")) {
            Mouse(pWnd, t, false, false);
            Mouse(pWnd, t, true, t.szVerb[0] == 'd');
        } else if (!strcmp(t.szVerb, "type")) {
            Text(pWnd, t);
        } else if (!strcmp(t.szVerb, "key")) {
            Key(pWnd, t, true, true);
        } else if (!strcmp(t.szVerb, "keydown")) {
            Key(pWnd, t, true, false);
        } else if (!strcmp(t.szVerb, "keyup")) {
            Key(pWnd, t, false, true);
        } else if (!strcmp(t.szVerb, "focus")) {
            SDL_Event tEvent;
            SDL_zero(tEvent);
            tEvent.type = strcmp(t.szArg, "lost") ? SDL_EVENT_WINDOW_FOCUS_GAINED : SDL_EVENT_WINDOW_FOCUS_LOST;
            tEvent.window.windowID = pWnd->nSdlId;
            TranslateSdlEvent(tEvent);
        }
    }
}

}  // namespace Win32Posix

using namespace Win32Posix;

bool Win32Posix_TakeShotRequest(char* szPath, size_t nSize) {
    Script& s = S();
    if (!s.szShotPath[0] || nSize == 0) {
        return false;
    }
    snprintf(szPath, nSize, "%s", s.szShotPath);
    s.szShotPath[0] = '\0';
    return true;
}

#endif  // !defined(_WIN32) && D2_BACKEND_POSIX
