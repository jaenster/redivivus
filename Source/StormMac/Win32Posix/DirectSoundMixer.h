// Win32Posix: the DirectSound mixer's own calls (DirectSound.cpp), beside the COM interface the
// game uses. Not part of the surface the game sees: the tests drive the mixer through these
// with the audio device paused, so the mixed samples can be checked.
#pragma once

#if !defined(_WIN32) && D2_BACKEND_POSIX

struct IDirectSound;

namespace Win32PosixDirectSound {

// Stops (or restarts) the device pulling samples; the buffers keep their state.
void PauseDevice(IDirectSound* pDirectSound, bool bPause);
// Mixes nFrames stereo float frames into pOut (2 * nFrames floats) at the mix rate, advancing
// every playing buffer as the device would.
void Mix(IDirectSound* pDirectSound, float* pOut, int nFrames);
// The mix rate: the primary buffer's format, 22050 until SetFormat sets it.
int MixRate(IDirectSound* pDirectSound);

}  // namespace Win32PosixDirectSound

#endif  // !defined(_WIN32) && D2_BACKEND_POSIX
