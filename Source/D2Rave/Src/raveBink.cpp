// raveBink.cpp - Bink cutscene playback for the RAVE renderer.
//
// File name inferred (the Windows renderers keep the same code in D2Glide/Src/glBink.cpp-style
// units); the unit itself is measured: it ends with __sinit_raveBink_cpp at 102b1738. See
// Source/D2Rave/README.md.

#include "Source/D2Rave/Src/raveInternal.h"

#if !defined(_WIN32) && D2_MAC_CARBON && D2_ARCH_PPC

namespace D2Rave {

namespace {
// The movie state lives in TOC-relative statics, not on the stack.
int snRaveBinkY;         // 1037c718
unsigned int snRaveBinkX; // 1037c71c
HBINK shRaveBink;         // 1037c720
int sbRaveBinkDone;       // 1037c724
} // namespace

// Plays one Bink movie straight into the DrawSprocket front buffer, centred, until it ends or a
// mouse/key event arrives; keeps looping for at least 500 ms after the end.
// mac ppc classic/1.09d: 102b1478 (1.00: 10077088)
void RavePlayBinkFile(const char * szFile, int nMode, uint8_t * pDest, int nPitch, void (*pfnFrame)()) {
    sbRaveBinkDone = 0;
    if (shRaveBink == nullptr) {
        void * hFile;
        int16_t nRefNum[2];
        EventRecord event;

        FlushEvents(everyEvent, 0);
        BinkSetSoundSystem(BinkOpenSoundManager, 0);
        SFileOpenFileEx(nullptr, szFile, GetUseDirectCommand() ? 5 : 4, &hFile);
        if (hFile == nullptr) {
            LOGFILE_WriteStringToFile("RavePlayBinkFile: *** Couldn't load Bink movie \"%s\".", szFile);
        } else {
            FileRefFromHANDLE(hFile, nRefNum);
            uint32_t dwFlags = 0x800000;
            if (nMode == 2) {
                dwFlags = 0x10800000;
            } else if (nMode < 2) {
                if (nMode > 0) {
                    dwFlags = 0x30800000;
                }
            } else if (nMode < 4) {
                dwFlags = 0x40800000;
            }
            shRaveBink = BinkOpen(nRefNum[0], dwFlags);
            if (shRaveBink == nullptr) {
                LOGFILE_WriteStringToFile("RavePlayBinkFile: *** Bink can't load \"%s\".", szFile);
            } else {
                LOGFILE_WriteStringToFile("RavePlayBinkFile: Playing Bink movie \"%s\".", szFile);
                snRaveBinkX = (uint32_t)(gnRaveScreenWidth - shRaveBink->Width) >> 1;
                if (snRaveBinkX & 3) {
                    snRaveBinkX = (snRaveBinkX | 3) + 1;
                }
                snRaveBinkY = (uint32_t)(gnRaveScreenHeight - shRaveBink->Height) >> 1;
                uint32_t dwStart = GetTickCount();
                while (sbRaveBinkDone == 0 || GetTickCount() < dwStart + 500) {
                    if (pfnFrame) {
                        pfnFrame();
                    }
                    if (OSEventAvail(0xffeb, &event)) {
                        sbRaveBinkDone = 1;
                        FlushEvents(mDownMask | mUpMask | keyDownMask | keyUpMask | autoKeyMask, 0);
                    } else if (BinkWait(shRaveBink) == 0) {
                        BinkDoFrame(shRaveBink);
                        BinkCopyToBuffer(shRaveBink, pDest, nPitch, shRaveBink->Height, snRaveBinkX, snRaveBinkY,
                                         gnRaveBinkSurfaceType);
                        if (shRaveBink->FrameNum == shRaveBink->Frames) {
                            sbRaveBinkDone = 1;
                        } else {
                            BinkNextFrame(shRaveBink);
                        }
                    }
                }
            }
        }
        if (shRaveBink != nullptr) {
            BinkClose(shRaveBink);
            shRaveBink = nullptr;
        }
        if (hFile != nullptr) {
            CloseHandle(hFile);
        }
    }
    FlushEvents(mDownMask | mUpMask | keyDownMask | keyUpMask | autoKeyMask, 0);
}

} // namespace D2Rave

#endif // !defined(_WIN32) && D2_MAC_CARBON && D2_ARCH_PPC
