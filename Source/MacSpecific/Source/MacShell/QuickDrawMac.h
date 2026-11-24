// The QuickDraw calls the Mac shell makes. The 10.7 SDK's headers no longer declare
// QuickDraw (the library still exports it, and the 1.14d binary imports it), so the
// prototypes are written out here from the 10.6 SDK's QuickDraw.h. Not an original header.
#pragma once

#include "D2Arch.h"

#if !defined(_WIN32) && D2_OS_MAC

#include <Carbon/Carbon.h>

extern "C" {
void GetPort(GrafPtr* pPort);
void SetPort(GrafPtr pPort);
void GetForeColor(RGBColor* pColor);
void GetBackColor(RGBColor* pColor);
void RGBForeColor(const RGBColor* pColor);
void RGBBackColor(const RGBColor* pColor);
void ForeColor(long nColor);
void BackColor(long nColor);
void EraseRect(const Rect* pRect);
const BitMap* GetPortBitMapForCopyBits(CGrafPtr pPort);
void CopyBits(const BitMap* pSrc, const BitMap* pDst, const Rect* pSrcRect, const Rect* pDstRect, short nMode,
              RgnHandle hMaskRgn);
void CopyDeepMask(const BitMap* pSrc, const BitMap* pMask, const BitMap* pDst, const Rect* pSrcRect,
                  const Rect* pMaskRect, const Rect* pDstRect, short nMode, RgnHandle hMaskRgn);
CTabHandle GetCTable(short nCTableId);
void DisposeCTable(CTabHandle hCTable);
void DisposeGWorld(GWorldPtr pWorld);
}

enum {
    blackColor = 33,
    whiteColor = 30,
};

#endif  // D2_OS_MAC
