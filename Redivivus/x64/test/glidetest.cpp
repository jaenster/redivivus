// 64-bit smoke test of a glide3x.dll: resolves the decorated names the way
// Source/D2Glide/Renderer/Glide.cpp does, opens a 640x480 Glide context on a
// window, and keeps drawing (a colour ramp through grBufferClear, a gradient
// through the LFB) until killed. Writes glidetest.log beside the exe.
#include <windows.h>
#include <stdio.h>
#include <stdint.h>

typedef uint32_t FxU32;
typedef int32_t FxI32;
typedef FxU32 GrContext_t;
struct GrLfbInfo_t { FxI32 size; void* lfbPtr; FxU32 strideInBytes; FxI32 writeMode; FxI32 origin; };

static void (__stdcall *grGlideInit)();
static void (__stdcall *grGlideShutdown)();
static void (__stdcall *grSstSelect)(int);
static GrContext_t (__stdcall *grSstWinOpen)(FxU32 hWnd, FxI32 res, FxI32 refresh, FxI32 cformat, FxI32 origin, int nCol, int nAux);
static void (__stdcall *grSstWinClose)(GrContext_t);
static void (__stdcall *grBufferClear)(FxU32 color, FxU32 alpha, FxU32 depth);
static void (__stdcall *grBufferSwap)(FxU32 interval);
static FxI32 (__stdcall *grLfbLock)(FxI32 type, FxI32 buffer, FxI32 writeMode, FxI32 origin, FxI32 pixelPipeline, GrLfbInfo_t* info);
static FxI32 (__stdcall *grLfbUnlock)(FxI32 type, FxI32 buffer);
static const char* (__stdcall *grGetString)(FxU32 pname);

static FILE* g_log;
#define LOG(...) do { fprintf(g_log, __VA_ARGS__); fputc('\n', g_log); fflush(g_log); } while (0)

template <class T> static bool Resolve(HMODULE h, T& fn, const char* name) {
    fn = (T)(void*)GetProcAddress(h, name);
    LOG("%-28s %s", name, fn ? "ok" : "MISSING");
    return fn != nullptr;
}

static LRESULT CALLBACK WndProc(HWND h, UINT m, WPARAM w, LPARAM l) {
    if (m == WM_DESTROY) { PostQuitMessage(0); return 0; }
    return DefWindowProcA(h, m, w, l);
}

int WINAPI WinMain(HINSTANCE hInst, HINSTANCE, LPSTR, int) {
    g_log = fopen("glidetest.log", "w");
    if (!g_log) return 1;
    LOG("glidetest %u-bit", (unsigned)(sizeof(void*) * 8));
    HMODULE hGlide = LoadLibraryA("glide3x.dll");
    LOG("LoadLibrary(glide3x.dll): %p", (void*)hGlide);
    if (!hGlide) return 2;
    bool ok = true;
    ok &= Resolve(hGlide, grGlideInit, "_grGlideInit@0");
    ok &= Resolve(hGlide, grGlideShutdown, "_grGlideShutdown@0");
    ok &= Resolve(hGlide, grSstSelect, "_grSstSelect@4");
    ok &= Resolve(hGlide, grSstWinOpen, "_grSstWinOpen@28");
    ok &= Resolve(hGlide, grSstWinClose, "_grSstWinClose@4");
    ok &= Resolve(hGlide, grBufferClear, "_grBufferClear@12");
    ok &= Resolve(hGlide, grBufferSwap, "_grBufferSwap@4");
    ok &= Resolve(hGlide, grLfbLock, "_grLfbLock@24");
    ok &= Resolve(hGlide, grLfbUnlock, "_grLfbUnlock@8");
    ok &= Resolve(hGlide, grGetString, "_grGetString@4");
    if (!ok) return 3;

    WNDCLASSA wc = {};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInst;
    wc.lpszClassName = "GlideTest64";
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    RegisterClassA(&wc);
    RECT rc = {0, 0, 640, 480};
    AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, FALSE);
    HWND hWnd = CreateWindowA("GlideTest64", "Glide 64-bit smoke test", WS_OVERLAPPEDWINDOW | WS_VISIBLE,
                              100, 100, rc.right - rc.left, rc.bottom - rc.top, nullptr, nullptr, hInst, nullptr);
    LOG("window %p", (void*)hWnd);

    grGlideInit();
    grSstSelect(0);
    LOG("vendor=%s renderer=%s version=%s", grGetString(0x1), grGetString(0x2), grGetString(0x3));
    // GR_RESOLUTION_640x480 = 7, GR_REFRESH_60Hz = 3, GR_COLORFORMAT_ARGB = 0,
    // GR_ORIGIN_UPPER_LEFT = 0, two colour buffers, one aux buffer - the game's own call.
    GrContext_t ctx = grSstWinOpen((FxU32)(uintptr_t)hWnd, 7, 3, 0, 0, 2, 1);
    LOG("grSstWinOpen -> %u", ctx);
    if (!ctx) return 4;

    int nFrame = 0, nLfbOk = 0;
    MSG msg;
    for (;;) {
        while (PeekMessageA(&msg, nullptr, 0, 0, PM_REMOVE)) {
            if (msg.message == WM_QUIT) goto done;
            TranslateMessage(&msg);
            DispatchMessageA(&msg);
        }
        grBufferClear(0x00202060 + ((nFrame & 63) << 2), 0, 0);
        GrLfbInfo_t info = {};
        info.size = sizeof info;
        // GR_LFB_WRITE_ONLY = 1, GR_BUFFER_BACKBUFFER = 1, GR_LFBWRITEMODE_565 = 0
        if (grLfbLock(1, 1, 0, 0, 0, &info) && info.lfbPtr) {
            for (int y = 100; y < 380; ++y) {
                uint16_t* row = (uint16_t*)((uint8_t*)info.lfbPtr + y * info.strideInBytes);
                for (int x = 120; x < 520; ++x) {
                    unsigned r = (x - 120) * 31 / 400, g = (y - 100) * 63 / 280, b = (nFrame >> 2) & 31;
                    row[x] = (uint16_t)(r << 11 | g << 5 | b);
                }
            }
            grLfbUnlock(1, 1);
            ++nLfbOk;
        }
        grBufferSwap(1);
        if (++nFrame % 60 == 0) LOG("frame %d lfb ok %d", nFrame, nLfbOk);
        Sleep(16);
    }
done:
    grSstWinClose(ctx);
    grGlideShutdown();
    LOG("closed after %d frames", nFrame);
    return 0;
}
