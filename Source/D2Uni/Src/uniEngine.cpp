// uniEngine.cpp - the Bink cutscene player and the drawing primitives every D2Uni back end shares.
// They all draw into the one 8-bit back buffer (gpUniBackBuffer, gnUniBackBufferWidth bytes per
// row), so the same bodies serve DrawSprocket, QuickDraw and CoreGraphics.
//
// File name inferred (Source/D2Uni/README.md): the first D2Uni object in link order, before
// uniSmack.cpp. The Bink pair may be an object of its own. Windows twins: the unnamed D2DDraw run
// 00512680-005132c0 (ddEngine.cpp in the beta asserts) and the D2Gdi run 006c8930-006c95d0; the
// BINK_* pair sits in ddSmack.cpp. MWCC emits an object's functions last to first, so the
// functions below are in source order, highest address first.
//
// Bodies from the LoD 1.10b classic PEF; differences in other builds are noted where seen.
//
// Classic Mac PowerPC only.
#if !defined(_WIN32) && D2_OS_MAC && D2_ARCH_PPC

#include "Source/D2Uni/Src/uniInternal.h"

// The Bink player's state.
static int gnUniBinkDestY;          // 10434488
static int gnUniBinkDestX;          // 1043448c
static HBINK ghUniBink;             // 10434490

// ppc 1.10b: 10054a70 (1.00 10083e9c, 1.13d 1005aa90, carbon 1.10b 103a3a20, carbon 1.13d 103d3170)
// Instruction for instruction the D2DDraw DrawGroundTile (005132c0): load the tile, shift it by
// the panel, hand it to D2gfx's software floor renderer.
void __fastcall UniDrawGroundTile(D2TileLibraryEntryStrc * pTile, D2GfxLightExStrc * pLight, int nPosX, int nPosY,
                                  int nWorldX, int nWorldY, BYTE nAlpha, int nScreenPanels, void * pTileData) {
    if (!D2CMP::SRC::SpriteCache::SPRITECACHE_AllocEntry(pTile, 1, 0)) {
        return;
    }

    nPosX -= 0x50;
    switch (nScreenPanels) {
        case 1:
            nPosX -= gnUniBackBufferWidth / 4;
            break;
        case 2:
            nPosX += gnUniBackBufferWidth / 4;
            break;
    }

    gpUniHelpers->nfpDrawGroundTile(pTile, nPosX, nPosY, pLight);
}

// ppc 1.10b: 10054a60 (1.00 10083e8c, 1.13d 1005aa80, carbon 1.10b 103a3a10, carbon 1.13d 103d3160)
BOOL __stdcall UniAreBlendedShadowsEnabled() {
    return gpUniSettings->bBlendedShadows;
}

// ppc 1.10b: 10054760 (1.00 10083b7c, 1.13d 1005a780, carbon 1.10b 103a3710, carbon 1.13d 103d2e60)
// The same blend modes as D2DDraw/D2Gdi DrawSolidRectEx, through the palette tables. The edges
// are clamped to [0, size], with no inverted-rectangle check (1.14d halts on one).
void __fastcall UniDrawSolidRectEx(int nXStart, int nYStart, int nXEnd, int nYEnd, BYTE nColor, DRAWMODE eDrawMode) {
    int nTableKind;
    BYTE * pTable = UniGetPaletteTable(eDrawMode, &nTableKind);

    int nLeft = nXStart < 0 ? 0 : nXStart;
    if (nLeft >= gnUniBackBufferWidth) {
        nLeft = gnUniBackBufferWidth;
    }
    int nRight = nXEnd < 0 ? 0 : nXEnd;
    if (nRight >= gnUniBackBufferWidth) {
        nRight = gnUniBackBufferWidth;
    }
    int nTop = nYStart < 0 ? 0 : nYStart;
    if (nTop >= gnUniBackBufferHeight) {
        nTop = gnUniBackBufferHeight;
    }
    int nBottom = nYEnd < 0 ? 0 : nYEnd;
    if (nBottom >= gnUniBackBufferHeight) {
        nBottom = gnUniBackBufferHeight;
    }

    int nRows = nBottom - nTop;
    BYTE * pRow = gpUniBackBuffer + nTop * gnUniBackBufferWidth + nLeft;
    int nWidth = nRight - nLeft;
    switch (nTableKind) {
        case 2: {
            // A 256x256 blend table: row = the pixel already there, column = the colour.
            BYTE * pBlend = pTable + nColor;
            while (nRows--) {
                for (int i = 0; i < nWidth; i++) {
                    pRow[i] = pBlend[pRow[i] * 0x100];
                }
                pRow += gnUniBackBufferWidth;
            }
            break;
        }
        case 1:
            // A 256-entry remap of the pixel already there.
            while (nRows--) {
                for (int i = 0; i < nWidth; i++) {
                    pRow[i] = pTable[pRow[i]];
                }
                pRow += gnUniBackBufferWidth;
            }
            break;
        default:
            while (nRows--) {
                FillMemory(pRow, nWidth, nColor);
                pRow += gnUniBackBufferWidth;
            }
            break;
    }
}

// ppc 1.10b: 100546f0 (1.00 10083b14, 1.13d 1005a710, carbon 1.10b 103a36a0, carbon 1.13d 103d2df0)
// Alpha picks the draw mode, as in D2DDraw DrawSolidRectAlpha.
void __fastcall UniDrawSolidRectAlpha(int nXStart, int nYStart, int nXEnd, int nYEnd, DWORD dwColor, BYTE nAlpha) {
    DRAWMODE eDrawMode;
    if (nAlpha >= 0xC0) {
        eDrawMode = 5;
    } else if (nAlpha >= 0x80) {
        eDrawMode = 2;
    } else if (nAlpha >= 0x40) {
        eDrawMode = 1;
    } else if (nAlpha != 0) {
        eDrawMode = 0;
    } else {
        return;
    }

    UniDrawSolidRectEx(nXStart, nYStart, nXEnd, nYEnd, (BYTE)dwColor, eDrawMode);
}

// ppc 1.10b: 10054590 (1.00 100839b0, 1.13d 1005a5b0, carbon 1.10b 103a3540, carbon 1.13d 103d2c90)
// Bresenham, plotting only the points inside the back buffer. nAlpha is ignored.
void __fastcall UniDrawLine(int nXStart, int nYStart, int nXEnd, int nYEnd, DWORD dwColor, BYTE nAlpha) {
    int nDX = nXEnd - nXStart;
    int nDY = nYEnd - nYStart;
    int nStepX = 1;
    if (nDX < 0) {
        nDX = -nDX;
        nStepX = -1;
    }
    int nStepY = 1;
    if (nDY < 0) {
        nDY = -nDY;
        nStepY = -1;
    }

    int nX = nXStart;
    int nY = nYStart;
    if (nX >= 0 && nX < gnUniBackBufferWidth && nY >= 0 && nY < gnUniBackBufferHeight) {
        gpUniBackBuffer[nY * gnUniBackBufferWidth + nX] = (BYTE)dwColor;
    }

    int nError = 0;
    if (nDX < nDY) {
        for (int i = nDY; i > 0; i--) {
            nError += nDX;
            nY += nStepY;
            if (nError > nDY) {
                nX += nStepX;
                nError -= nDY;
            }
            if (nX >= 0 && nX < gnUniBackBufferWidth && nY >= 0 && nY < gnUniBackBufferHeight) {
                gpUniBackBuffer[nY * gnUniBackBufferWidth + nX] = (BYTE)dwColor;
            }
        }
    } else {
        for (int i = nDX; i > 0; i--) {
            nError += nDY;
            nX += nStepX;
            if (nError > nDX) {
                nY += nStepY;
                nError -= nDX;
            }
            if (nX >= 0 && nX < gnUniBackBufferWidth && nY >= 0 && nY < gnUniBackBufferHeight) {
                gpUniBackBuffer[nY * gnUniBackBufferWidth + nX] = (BYTE)dwColor;
            }
        }
    }
}

// Advances a 16.16 fixed-point accumulator; returns the whole steps taken and keeps the fraction.
static inline int UniStepFixed(int & nAccumulator, int nDelta) {
    nAccumulator += nDelta;
    if (nAccumulator > 0xFFFF || nAccumulator < -0xFFFF) {
        int nWhole = nAccumulator / 0x10000; // rounds toward zero, as the srawi/addze pair does
        nAccumulator -= nWhole * 0x10000;
        return nWhole;
    }
    return 0;
}

// ppc 1.10b: 100543a0 (1.00 100837dc, 1.13d 1005a3c0, carbon 1.10b 103a3350, carbon 1.13d 103d2aa0)
// A line by 16.16 fixed-point steps: the length is the integer square root (found by counting up),
// each step adds dx/len and dy/len. Points outside the buffer are skipped until the line enters
// it; once inside, it stops at the first point that leaves.
static void sUniDrawRectLine(const POINT * pFrom, const POINT * pTo, BYTE nColor) {
    int nX = pFrom->x;
    int nY = pFrom->y;
    int nDX = pTo->x - nX;
    int nDY = pTo->y - nY;
    if (nDX == 0 && nDY == 0) {
        return;
    }

    int nLength = 0;
    while (nLength * nLength < nDX * nDX + nDY * nDY) {
        nLength++;
    }

    int nStepX = (nDX << 16) / nLength;
    int nStepY = (nDY << 16) / nLength;
    int nFracX = 0;
    int nFracY = 0;
    int nStep = 0;
    for (;;) {
        if (nY >= 0 && nY < gnUniBackBufferHeight && nX >= 0 && nX < gnUniBackBufferWidth) {
            BYTE * pPixel = gpUniBackBuffer + nY * gnUniBackBufferWidth + nX;
            *pPixel = nColor;
            for (;;) {
                if (++nStep >= nLength) {
                    return;
                }
                int nMoveX = UniStepFixed(nFracX, nStepX);
                nX += nMoveX;
                pPixel += nMoveX;
                if (nX < 0 || nX >= gnUniBackBufferWidth) {
                    return;
                }
                int nMoveY = UniStepFixed(nFracY, nStepY);
                nY += nMoveY;
                pPixel += nMoveY * gnUniBackBufferWidth;
                if (nY < 0 || nY >= gnUniBackBufferHeight) {
                    return;
                }
                *pPixel = nColor;
            }
        }

        nX += UniStepFixed(nFracX, nStepX);
        nY += UniStepFixed(nFracY, nStepY);
        if (nStep >= nLength) {
            return;
        }
        nStep++;
    }
}

// ppc 1.10b: 10054290 (1.00 100836a8, 1.13d 1005a250, carbon 1.10b 103a3240, carbon 1.13d 103d2930)
// The automap's diamond: the four edge midpoints joined.
void __fastcall UniDrawRect(RECT * pRect, BYTE nColor) {
    int nWidth = pRect->right - pRect->left;
    int nHeight = pRect->bottom - pRect->top;
    if (nWidth == 0 || nHeight == 0) {
        return;
    }

    int nMidX = pRect->left + (int)((unsigned)nWidth >> 1);
    int nMidY = pRect->top + (int)((unsigned)nHeight >> 1);
    POINT tTop = {nMidX, pRect->top};
    POINT tRight = {pRect->right, nMidY};
    POINT tBottom = {nMidX, pRect->bottom};
    POINT tLeft = {pRect->left, nMidY};
    sUniDrawRectLine(&tTop, &tRight, nColor);
    sUniDrawRectLine(&tRight, &tBottom, nColor);
    sUniDrawRectLine(&tBottom, &tLeft, nColor);
    sUniDrawRectLine(&tLeft, &tTop, nColor);
}

// ppc 1.10b: 10054190 (1.00 1008358c, 1.13d 1005a0f0, carbon 1.10b 103a3140, carbon 1.13d 103d27d0)
// The rectangle's outline, corner to corner.
void __fastcall UniDrawRectEx(RECT * pRect, BYTE nColor) {
    if (pRect->right == pRect->left || pRect->bottom == pRect->top) {
        return;
    }

    POINT tTopLeft = {pRect->left, pRect->top};
    POINT tTopRight = {pRect->right, pRect->top};
    POINT tBottomRight = {pRect->right, pRect->bottom};
    POINT tBottomLeft = {pRect->left, pRect->bottom};
    sUniDrawRectLine(&tTopLeft, &tTopRight, nColor);
    sUniDrawRectLine(&tTopRight, &tBottomRight, nColor);
    sUniDrawRectLine(&tBottomRight, &tBottomLeft, nColor);
    sUniDrawRectLine(&tBottomLeft, &tTopLeft, nColor);
}

// ppc 1.10b: 10054030 (1.00 10083428, 1.13d 10059f00, carbon 1.10b 103a2fe0, carbon 1.13d 103d25e0)
// Clips the rectangle in place, then fills it.
void __fastcall UniDrawSolidRect(RECT * pRect, BYTE nColor) {
    if (pRect->right < 0 || pRect->left > gnUniBackBufferWidth || pRect->bottom < 0 ||
        pRect->top > gnUniBackBufferHeight) {
        return;
    }

    if (pRect->left < 0) {
        pRect->left = 0;
    }
    if (pRect->right > gnUniBackBufferWidth) {
        pRect->right = gnUniBackBufferWidth;
    }
    if (pRect->top < 0) {
        pRect->top = 0;
    }
    if (pRect->bottom > gnUniBackBufferHeight) {
        pRect->bottom = gnUniBackBufferHeight;
    }

    int nWidth = pRect->right - pRect->left;
    int nRows = pRect->bottom - pRect->top;
    if (nWidth == 0 || nRows == 0) {
        return;
    }

    BYTE * pRow = gpUniBackBuffer + pRect->top * gnUniBackBufferWidth + pRect->left;
    while (nRows--) {
        for (int i = 0; i < nWidth; i++) {
            pRow[i] = nColor;
        }
        pRow += gnUniBackBufferWidth;
    }
}

// ppc 1.10b: 10053fc0 (1.00 100833b8, 1.13d 10059e90, carbon 1.10b 103a2f70, carbon 1.13d 103d2570)
void __fastcall UniDrawSolidSquare(POINT * pCenter, BYTE nSize, BYTE nColor) {
    if (nSize == 0) {
        return;
    }

    int nHalf = (BYTE)(nSize - 1);
    RECT tRect;
    SetRect(&tRect, pCenter->x - nHalf, pCenter->y - nHalf, pCenter->x + nHalf + 1, pCenter->y + nHalf + 1);
    UniDrawSolidRect(&tRect, nColor);
}

// ppc 1.10b: 10053f50 (1.00 10083350, 1.13d 10059dd0, carbon 1.10b 103a2f00, carbon 1.13d 103d24b0)
// bPartial leaves the bottom 47 rows (the control panel) alone. 1.13d grows this function
// (0x70 -> 0xc0 bytes); not reconstructed here.
void __fastcall UniClearScreen(BOOL bPartial) {
    if (!bPartial) {
        ZeroMemory(gpUniBackBuffer, gnUniBackBufferWidth * gnUniBackBufferHeight);
    } else {
        ZeroMemory(gpUniBackBuffer, gnUniBackBufferWidth * (gnUniBackBufferHeight - 0x2F));
    }
}

// ppc 1.10b: 10053f40 (1.00 1008334c, 1.13d 10059dc0, carbon 1.10b 103a2ef0, carbon 1.13d 103d24a0)
// Empty. Called with UniSpriteSetResolution when a back end activates its surface.
void UniEngineSetResolution() {
}

// ppc 1.10b: 10053f30 (1.00 10083348, 1.13d 10059db0, carbon 1.10b 103a2ee0, carbon 1.13d 103d2490)
// Empty. Called after every back end clears its surface state (the teardown twin of
// UniEngineSetResolution).
void UniEngineReset() {
}

// ppc 1.10b: 10053e80 (1.00 10083298, 1.13d 10059d00, carbon 1.10b 103a2e30, carbon 1.13d 103d23e0)
// The debug text slot: format, widen, draw in D2Win font 13 twelve pixels lower. The Windows twin
// draws with its own debug font (ddDebugFont.cpp); the Mac goes through D2Win.
void __fastcall UniDrawString(int nPosX, int nPosY, char * szFormat, va_list va) {
    char szBuffer[512];
    Unicode wszBuffer[512];
    vsprintf(szBuffer, szFormat, va);
    Unicode::toUnicode(wszBuffer, szBuffer, 512);
    int nOldFont = D2Win_SetTextFont(13);
    D2Win_DrawText(wszBuffer, nPosX, nPosY + 12, 0, 0);
    D2Win_SetTextFont(nOldFont);
}

// ppc 1.10b: 10053e70 (1.00 10083294, 1.13d 10059cf0, carbon 1.10b 103a2e20, carbon 1.13d 103d23d0)
void __fastcall UniDrawLight(DWORD * pLight, DWORD * pPlayerLight, int nPosX, int nPosY) {
}

// ppc 1.10b: 10053ca0 (1.00 10083098, 1.13d 10059b20, carbon 1.10b 103a2c50, carbon 1.13d 103d2200)
// Decodes a frame into the destination, then presents the bounding box of the frame's dirty
// rectangles (centred on the screen) through the back end. The last frame ends the movie.
void UniBinkRenderFrame(BYTE * pDest, int nDestPitch, uint32_t dwSurfaceType) {
    BinkDoFrame(ghUniBink);
    BinkCopyToBuffer(ghUniBink, pDest, nDestPitch, ghUniBink->Height, gnUniBinkDestX, gnUniBinkDestY, dwSurfaceType);
    BinkGetRects(ghUniBink, dwSurfaceType);

    int16_t nTop = (int16_t)((gnUniScreenHeight - (int)ghUniBink->Height) >> 1);
    int16_t nLeft = (int16_t)((gnUniScreenWidth - (int)ghUniBink->Width) >> 1);
    // Start inverted (top/left at the far edge, bottom/right at the near one) and grow.
    Rect tDirty;
    tDirty.top = (int16_t)((gnUniScreenHeight + (int)ghUniBink->Height) >> 1);
    tDirty.left = (int16_t)((gnUniScreenWidth + (int)ghUniBink->Width) >> 1);
    tDirty.bottom = nTop;
    tDirty.right = nLeft;
    for (int i = 0; i < ghUniBink->NumRects; i++) {
        const BINKRECT & tRect = ghUniBink->FrameRects[i];
        int16_t nRectTop = nTop + (int16_t)tRect.Top;
        int16_t nRectLeft = nLeft + (int16_t)tRect.Left;
        int16_t nRectBottom = nRectTop + (int16_t)tRect.Height;
        int16_t nRectRight = nRectLeft + (int16_t)tRect.Width;
        if (nRectTop < tDirty.top) {
            tDirty.top = nRectTop;
        }
        if (nRectLeft < tDirty.left) {
            tDirty.left = nRectLeft;
        }
        if (nRectBottom > tDirty.bottom) {
            tDirty.bottom = nRectBottom;
        }
        if (nRectRight > tDirty.right) {
            tDirty.right = nRectRight;
        }
    }

    if (tDirty.left < tDirty.right && tDirty.top < tDirty.bottom) {
        UniBinkBlit(&tDirty);
    }

    if (ghUniBink->FrameNum == ghUniBink->Frames) {
        gbUniCutsceneQuit = 1;
    } else {
        BinkNextFrame(ghUniBink);
    }
}

// ppc 1.10b: 10053a30 (1.00 10082e44, 1.13d 100598e0, carbon 1.10b 103a29e0, carbon 1.13d 103d1fc0)
// The D2DDraw BINK_PlayVideoFile loop on the Mac event queue: open from the MPQ (direct when
// Fog says so), pick the BinkOpen flags by resolution, centre on the screen, and decode until the
// movie ends or a mouse or key press (OSEventAvail) asks to quit. As on Windows, a quit request
// lets 500 ms of frames play out first.
void UniPlayBinkFile(char * szFile, int nResolution, BYTE * pDest, int nDestPitch, uint32_t dwSurfaceType,
                     fnBinkFrameCallback pfnFrame) {
    HANDLE hFile;
    int16_t nRefNum;
    EventRecord tEvent;

    gbUniCutsceneQuit = 0;
    if (ghUniBink) {
        return;
    }

    FlushEvents(everyEvent, 0);
    BinkSetSoundSystem(BinkOpenSoundManager, 0);
    uint32_t dwOpenMode = Fog::GetUseDirectCommand() ? 5 : 4;
    Storm::Source::SFile::SFileOpenFileEx(nullptr, szFile, dwOpenMode, &hFile);
    if (!hFile) {
        LogPrintf("UniPlayBinkFile: *** Couldn't load Bink movie \"%s\".", szFile);
    } else {
        FileRefFromHANDLE(hFile, &nRefNum);
        uint32_t dwOpenFlags = BINKFILEHANDLE;
        switch (nResolution) {
            case 1:
                dwOpenFlags = 0x30800000;
                break;
            case 2:
                dwOpenFlags = 0x10800000;
                break;
            case 3:
                dwOpenFlags = 0x40800000;
                break;
        }

        // The Mac BinkOpen takes the file's refNum where Windows passes a HANDLE.
        ghUniBink = (HBINK)BinkOpen((void *)(intptr_t)nRefNum, dwOpenFlags);
        if (!ghUniBink) {
            LogPrintf("UniPlayBinkFile: *** Bink can't load \"%s\".", szFile);
        } else {
            LogPrintf("UniPlayBinkFile: Playing Bink movie \"%s\".", szFile);
            gnUniBinkDestX = (gnUniScreenWidth - (int)ghUniBink->Width) >> 1;
            if (gnUniBinkDestX & 3) {
                gnUniBinkDestX = (gnUniBinkDestX | 3) + 1;
            }
            gnUniBinkDestY = (gnUniScreenHeight - (int)ghUniBink->Height) >> 1;

            DWORD dwStart = GetTickCount();
            while (!gbUniCutsceneQuit || GetTickCount() < dwStart + 500) {
                if (pfnFrame) {
                    pfnFrame();
                }
                if (OSEventAvail(everyEvent & ~(mUpMask | keyUpMask), &tEvent)) {
                    gbUniCutsceneQuit = 1;
                    FlushEvents(0x3E, 0); // mouse down/up, key down/up, auto-key
                } else if (!BinkWait(ghUniBink)) {
                    UniBinkRenderFrame(pDest, nDestPitch, dwSurfaceType);
                }
            }
        }
    }

    if (ghUniBink) {
        BinkClose(ghUniBink);
        ghUniBink = nullptr;
    }
    if (hFile) {
        CloseHandle(hFile);
    }
}

#endif // !defined(_WIN32) && D2_OS_MAC && D2_ARCH_PPC
