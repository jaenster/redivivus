/* LINK STUB, not RAD's Bink.
 *
 * The 1.14d Mac binary links RAD Game Tools' Bink 1.x (proprietary, no source; nothing of it is
 * vendored here). D2OpenGL's cutscene player references these seven entry points. This file
 * defines them with the Bink 1.x signatures so the Mac objects link, and every one reports
 * failure or nothing: BinkOpenDirectSound returns NULL, the rest return 0. A Mac executable
 * linked against it plays no video.
 *
 * Mac-only: compiled by cmake/mac.cmake (d2_mac_link_deps), never by the Windows build.
 */

typedef void *HBINK;

int BinkSetSoundSystem(void *(*pfnOpen)(unsigned), unsigned nParam) { (void)pfnOpen; (void)nParam; return 0; }
void *BinkOpenDirectSound(unsigned nParam) { (void)nParam; return 0; }
void BinkClose(HBINK hBink) { (void)hBink; }
int BinkDoFrame(HBINK hBink) { (void)hBink; return 0; }
void BinkNextFrame(HBINK hBink) { (void)hBink; }
int BinkWait(HBINK hBink) { (void)hBink; return 0; }
int BinkCopyToBuffer(HBINK hBink, void *pDest, int nDestPitch, unsigned nDestHeight, unsigned nDestX,
                     unsigned nDestY, unsigned nFlags) {
    (void)hBink; (void)pDest; (void)nDestPitch; (void)nDestHeight; (void)nDestX; (void)nDestY; (void)nFlags;
    return 0;
}
