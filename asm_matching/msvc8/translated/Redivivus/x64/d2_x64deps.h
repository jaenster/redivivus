// The three licensed DLLs Game.exe used but did not contain - Bink (binkw32.dll),
// Smacker (smackw32.dll) and the Intel JPEG Library (ijl11.dll) - exist only as
// 32-bit binaries. The 64-bit build defines their entry points here, with the
// signatures the tree declares in Source/D2Headers/d2_platform.h and the struct
// layouts the callers read (BINK: globals.h; Smack: D2Gdi.h; JPEG_CORE_PROPERTIES:
// D2WinMain.h). The 32-bit build never compiles any of this: it links the import
// libraries in 3rdparty/implibs against the original DLLs.
//
// Bink and Smacker decode through FFmpeg's shared libraries, loaded at run time
// (d2_ffmpeg.h); without them BinkOpen/SmackOpen return NULL and the game goes on
// as it does for a missing video file. IJL writes through stb_image_write.
#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// --- Bink -----------------------------------------------------------------
// The callers read the first four fields; the rest of RAD's BINK header stays
// private to the shim. FrameNum is 1-based: the frame BinkDoFrame decodes next.
struct BINK {
    uint32_t Width;
    uint32_t Height;
    uint32_t Frames;
    uint32_t FrameNum;
};

// BinkOpen flags the game passes (DirectDraw.cpp BINK_PlayVideoFile, 005137e0).
#define BINKFILEHANDLE   0x00800000u  // `name` is a Win32 file HANDLE at the video's offset
#define BINKCOPY2XH      0x10000000u  // decode at twice the height
#define BINKCOPY2XHI     0x20000000u  // (interlaced variant; same output size)
#define BINKCOPY2XW      0x30000000u  // twice the width
#define BINKCOPY2XWH     0x40000000u  // both
#define BINKCOPY2XWHI    0x50000000u
#define BINKCOPYNOSCALING 0x70000000u
#define BINKCOPYALL      0x80000000u  // CopyToBuffer: copy every pixel, not just the changed ones

// BinkCopyToBuffer destination formats (also what BinkDDSurfaceType returns).
#define BINKSURFACE8P    0
#define BINKSURFACE24    1
#define BINKSURFACE24R   2
#define BINKSURFACE32    3
#define BINKSURFACE32R   4
#define BINKSURFACE32A   5
#define BINKSURFACE32RA  6
#define BINKSURFACE4444  7
#define BINKSURFACE5551  8
#define BINKSURFACE555   9
#define BINKSURFACE565   10
#define BINKSURFACE655   11
#define BINKSURFACE664   12
#define BINKSURFACEYUY2  13
#define BINKSURFACEUYVY  14
#define BINKSURFACEYV12  15
#define BINKSURFACEMASK  15

void* __stdcall BinkOpen(void* hFile, uint32_t dwFlags);
void __stdcall BinkClose(void* pBink);
int32_t __stdcall BinkDoFrame(void* pBink);
void __stdcall BinkNextFrame(void* pBink);
int32_t __stdcall BinkWait(void* pBink);
int32_t __stdcall BinkCopyToBuffer(void* pBink, void* pDest, int32_t nDestPitch, uint32_t nDestHeight,
                                   uint32_t nDestX, uint32_t nDestY, uint32_t dwFlags);
int32_t __stdcall BinkSetSoundSystem(void* (__stdcall* pfnOpen)(uint32_t), uint32_t dwParam);
void* __stdcall BinkOpenDirectSound(uint32_t dwParam);
uint32_t __stdcall BinkDDSurfaceType(void* pDDSurface);

// --- Smacker --------------------------------------------------------------
// The RAD SmackTag header as the callers index it: Width/Height/Frames at the
// front, NewPalette at 0x68, Palette (256 RGB triplets) at 0x6C, FrameNum at 0x374.
// FrameNum is 0-based; it reaches Frames after the last frame has been advanced
// past, which is what the game's `FrameNum == Frames` close test waits for.
struct Smack {
    uint32_t Version;           // 0x000
    uint32_t Width;             // 0x004
    uint32_t Height;            // 0x008
    uint32_t Frames;            // 0x00C
    uint32_t MSPerFrame;        // 0x010
    uint32_t SmackerType;       // 0x014  bit 0: ring frame (the file loops)
    uint32_t LargestInTrack[7]; // 0x018
    uint32_t tablesize;         // 0x034
    uint32_t codesize;          // 0x038
    uint32_t absize;            // 0x03C
    uint32_t detailsize;        // 0x040
    uint32_t typesize;          // 0x044
    uint32_t TrackType[7];      // 0x048
    uint32_t extra;             // 0x064
    uint32_t NewPalette;        // 0x068  set when this frame changed the palette
    uint8_t Palette[772];       // 0x06C  R,G,B per entry, 256 entries (4 bytes spare)
    uint32_t PalType;           // 0x370
    uint32_t FrameNum;          // 0x374
    uint32_t FrameSize;         // 0x378
    uint32_t SndSize;           // 0x37C
};

#define SMACKFILEHANDLE  0x01000u
#define SMACKTRACKS      0xFE000u    // play every audio track
#define SMACKAUTOEXTRA   0xFFFFFFFFu // SmackOpen extrabuf: let the library size it
// SmackToBuffer flags.
#define SMACKBUFFERREVERSED 0x00000001u // bottom-up destination
#define SMACKBUFFER555      0x80000000u
#define SMACKBUFFER565      0xC0000000u
#define SMACKBUFFER16       0xC0000000u // mask

void* __stdcall SmackOpen(const char* szName, uint32_t dwFlags, uint32_t dwExtraBuf);
void __stdcall SmackClose(void* pSmack);
int32_t __stdcall SmackDoFrame(void* pSmack);
void __stdcall SmackNextFrame(void* pSmack);
int32_t __stdcall SmackWait(void* pSmack);
void __stdcall SmackToBuffer(void* pSmack, uint32_t nLeft, uint32_t nTop, uint32_t nPitch, uint32_t nDestHeight,
                             void* pBuffer, uint32_t dwFlags);

// --- Intel JPEG Library ---------------------------------------------------
// The head of JPEG_CORE_PROPERTIES as D2WinMain.h lays it out; the 0x4e40-byte
// struct the game zeroes continues with IJL's private tables, which the shim does
// not touch. Field order and types are what fix the offsets on both targets.
struct JPEG_CORE_PROPERTIES_HEAD {
    int UseJPEGPROPERTIES;
    uint8_t* DIBBytes;
    uint32_t DIBWidth;
    int DIBHeight;
    uint32_t DIBPadBytes;
    uint32_t DIBChannels;
    int DIBColor;
    int DIBSubsampling;
    char* JPGFile;
    uint8_t* JPGBytes;
    uint32_t JPGSizeBytes;
    uint32_t JPGWidth;
    int JPGHeight;
    uint32_t JPGChannels;
    int JPGColor;
    int JPGSubsampling;
    uint32_t JPGThumbWidth;
    int JPGThumbHeight;
    int cconversion_reqd;
    int upsampling_reqd;
    int jquality;
};

#define IJL_OK                 0
#define IJL_INVALID_JPEG_PROPERTIES (-2)
#define IJL_INVALID_FILENAME   (-6)
#define IJL_ERR_FILECREATE     (-19)
#define IJL_RGB  1
#define IJL_BGR  2
#define IJL_JFILE_WRITEWHOLEIMAGE 8

int32_t __stdcall ijlInit(void* pJpegCoreProps);
int32_t __stdcall ijlFree(void* pJpegCoreProps);
int32_t __stdcall ijlWrite(void* pJpegCoreProps, int32_t nWriteOption);

#ifdef __cplusplus
}
#endif
