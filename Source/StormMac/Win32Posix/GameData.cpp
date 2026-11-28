// Win32Posix: finding the game data on the first run (Win32PosixShell.h, POSIX_LocateGameData).
// The game ships without its MPQs: they come from the player's own Diablo II install. The
// kernel looks in D2_MPQ_DIR, beside the .app, in the install directory and in the folder picked
// on an earlier run (Win32PosixKernelPriv.h, MpqDir); when none holds d2data.mpq, the player is
// asked for the folder once, and the choice is kept in Win32Posix.ini for the next launch.
#if !defined(_WIN32) && D2_BACKEND_POSIX

#include <SDL3/SDL.h>
#include <stdio.h>

#include <string>

#include "Source/StormMac/Win32Posix/Win32PosixKernelPriv.h"

using namespace Win32Posix;

namespace {

constexpr const char* kTitle = "Diablo II";

struct FolderChoice {
    bool bDone = false;
    bool bFailed = false;
    std::string sFolder;  // empty when the player cancelled
};

void SDLCALL OnFolderChosen(void* pUser, const char* const* pszFiles, int nFilter) {
    (void)nFilter;
    FolderChoice* pChoice = (FolderChoice*)pUser;
    if (pszFiles == nullptr) {
        pChoice->bFailed = true;
    } else if (pszFiles[0] != nullptr) {
        pChoice->sFolder = pszFiles[0];
    }
    pChoice->bDone = true;
}

// The native folder picker, run to its answer on this (the main) thread.
FolderChoice AskForFolder() {
    FolderChoice tChoice;
    SDL_PropertiesID hProps = SDL_CreateProperties();
    SDL_SetStringProperty(hProps, SDL_PROP_FILE_DIALOG_TITLE_STRING,
                          "Choose your Diablo II folder (the one with d2data.mpq)");
    SDL_SetStringProperty(hProps, SDL_PROP_FILE_DIALOG_ACCEPT_STRING, "Use This Folder");
    std::string sStart = HomeDir();
    SDL_SetStringProperty(hProps, SDL_PROP_FILE_DIALOG_LOCATION_STRING, sStart.c_str());
    SDL_ShowFileDialogWithProperties(SDL_FILEDIALOG_OPENFOLDER, OnFolderChosen, &tChoice, hProps);
    while (!tChoice.bDone) {
        SDL_Event tEvent;
        if (SDL_WaitEventTimeout(&tEvent, 100) && tEvent.type == SDL_EVENT_QUIT) {
            break;
        }
    }
    SDL_DestroyProperties(hProps);
    return tChoice;
}

}  // namespace

int POSIX_LocateGameData(void) {
    if (HasGameData(MpqDir())) {
        return 1;
    }
    fprintf(stderr, "Win32Posix: no d2data.mpq in %s; asking for the game folder\n", MpqDir().c_str());
    if (!SDL_InitSubSystem(SDL_INIT_VIDEO | SDL_INIT_EVENTS)) {
        fprintf(stderr, "Win32Posix: no folder picker (%s)\n", SDL_GetError());
        return 0;
    }
    const char* szNotFound = nullptr;
    for (;;) {
        if (szNotFound) {
            SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_WARNING, kTitle, szNotFound, nullptr);
        }
        FolderChoice tChoice = AskForFolder();
        if (tChoice.bFailed) {
            fprintf(stderr, "Win32Posix: the folder picker failed (%s)\n", SDL_GetError());
        }
        if (tChoice.sFolder.empty()) {
            SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_INFORMATION, kTitle,
                                     "Diablo II needs the game files from your own Diablo II install "
                                     "(the folder with d2data.mpq, d2exp.mpq and the other .mpq files).\n\n"
                                     "Place that folder next to Diablo II.app, put the .mpq files in "
                                     "~/Library/Application Support/Diablo II, or choose the folder the "
                                     "next time the game starts.",
                                     nullptr);
            return 0;
        }
        if (!HasGameData(tChoice.sFolder)) {
            szNotFound = "That folder has no d2data.mpq. Choose the folder your Diablo II is installed in.";
            continue;
        }
        if (!MpqDir_Store(tChoice.sFolder)) {
            fprintf(stderr, "Win32Posix: could not keep the game folder in Win32Posix.ini\n");
        }
        fprintf(stderr, "Win32Posix: game data in %s\n", MpqDir().c_str());
        return 1;
    }
}

#endif  // !defined(_WIN32) && D2_BACKEND_POSIX
