// Streaming PCM output for the video shims, on waveOut.
//
// The originals played their audio through DirectSound (BinkOpenDirectSound,
// Smacker's SmackSoundUseDirectSound). What the game observes of that is only
// that the sound plays in step with the frames; the device route is private to
// the DLL. waveOut gives the shim the one thing it needs for that - a sample
// clock (waveOutGetPosition) the frame pacing can follow - with far less state
// than a DirectSound streaming buffer, and it mixes with the game's own
// DirectSound output on every Windows since XP.
#pragma once

#include <stdint.h>
#include <windows.h>
#include <mmsystem.h>

class D2PcmOut {
public:
    D2PcmOut() {}
    ~D2PcmOut() { Close(); }
    D2PcmOut(const D2PcmOut&) = delete;
    D2PcmOut& operator=(const D2PcmOut&) = delete;

    // 16-bit signed interleaved PCM at this rate and channel count.
    bool Open(int nSampleRate, int nChannels);
    void Close();
    bool IsOpen() const { return m_hWave != nullptr; }

    // Queues a block; returns false when the device refused it.
    bool Write(const int16_t* pSamples, int nFrames);

    // Samples played so far, as milliseconds; -1 when nothing has been queued
    // or the device is closed.
    double PositionMs() const;
    // True while queued blocks are still playing.
    bool IsPlaying() const;

private:
    struct Block {
        WAVEHDR hdr;
        Block* pNext;
    };
    void Reap();

    HWAVEOUT m_hWave = nullptr;
    int m_nRate = 0;
    int m_nChannels = 0;
    Block* m_pBlocks = nullptr;
    uint64_t m_nQueuedFrames = 0;
};
