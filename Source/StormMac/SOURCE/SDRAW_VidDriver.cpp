// Storm Mac: SDRAW_VidDriver.CPP, named by the __FILE__ literal of SDrawDestroy's free
// (1.14d Intel Mac 00036035, line 0xd8). Only the teardown is reconstructed here.
#include "Source/StormMac/H/stpl.h"

#if !defined(_WIN32) && D2_OS_MAC

// 1.14d mac: 0003602b
int Storm_ReturnTrue(void);
// 1.14d mac: 0001790c
int SGdiDestroy(void);
// 1.14d mac: 0001de67
int SVidDestroy(void);

// The driver's buffer and the state around it.
// 1.14d mac: 003979d0
uint32_t g_dwDriverActive;
// 1.14d mac: 003f491c
uint32_t g_dwDriverWidth;
// 1.14d mac: 003f4920
void* g_pDriverBuffer;
// 1.14d mac: 003f4924
uint32_t g_dwDriverHeight;
// 1.14d mac: 003f4928
uint32_t g_dwDriverPitch;

// 1.14d mac: 00036035
int SDrawDestroy(void) {
    uint32_t bFirst = Storm_ReturnTrue();
    uint32_t bGdi = SGdiDestroy();
    uint32_t bVid = SVidDestroy();
    g_dwDriverActive = 0;
    if (g_pDriverBuffer != NULL) {
        SMemFree(g_pDriverBuffer, __FILE__, 0xd8, 0);
        g_pDriverBuffer = NULL;
    }
    g_dwDriverWidth = 0;
    g_dwDriverHeight = 0;
    g_dwDriverPitch = 0;
    return bVid & bGdi & bFirst & 1;
}

#endif  // D2_OS_MAC
