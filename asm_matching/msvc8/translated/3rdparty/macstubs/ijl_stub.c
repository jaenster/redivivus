/* LINK STUB, not Intel's JPEG Library (ijl; proprietary, nothing of it is vendored here).
 *
 * D2WIN_SaveScreenshotToFile, a shared unit, writes its JPEG through ijl11.dll on Windows. The
 * Mac link only needs its three references to resolve. Each returns a non-zero IJLERR, so the
 * screenshot path reports an error instead of writing a file.
 *
 * Mac-only: compiled by cmake/mac.cmake (d2_mac_link_deps), never by the Windows build.
 */

#define IJL_STUB_FAILURE 1

int ijlInit(void *pJpegCoreProps) { (void)pJpegCoreProps; return IJL_STUB_FAILURE; }
int ijlFree(void *pJpegCoreProps) { (void)pJpegCoreProps; return IJL_STUB_FAILURE; }
int ijlWrite(void *pJpegCoreProps, int nIoType) { (void)pJpegCoreProps; (void)nIoType; return IJL_STUB_FAILURE; }
