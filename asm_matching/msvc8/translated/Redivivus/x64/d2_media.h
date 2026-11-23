// One demuxed, decoded media file - what the Bink and Smacker shims share.
//
// A file opens from a path or from a Win32 HANDLE positioned at the video's
// first byte (Storm hands Bink the MPQ archive's handle seeked to the stored
// file, SFile.cpp 3873: flag 4). Video packets decode into `pFrame`; audio
// packets are converted to 16-bit PCM and queued to the sound device as they
// are met, which keeps them in step with the frames because both formats
// interleave each frame's audio with its video.
//
// ClockMs() is the playback clock the pacing follows: the sound device's
// position while audio plays, continued on the wall clock when it has drained
// or the file has no audio.
#pragma once

#include "d2_ffmpeg.h"
#include "d2_pcmout.h"

#include <stdint.h>
#include <windows.h>

class D2Media {
public:
    D2Media() {}
    ~D2Media() { Close(); }
    D2Media(const D2Media&) = delete;
    D2Media& operator=(const D2Media&) = delete;

    // bAudio: decode and play the audio track(s); the first track is played.
    bool OpenPath(const char* pszPath, bool bAudio);
    // nSize bounds the stream; 0 reads to the end of the file.
    bool OpenHandle(HANDLE hFile, int64_t nSize, bool bAudio);
    void Close();

    // Decodes the next video frame into pFrame. 1: a frame is ready; 0: end of
    // stream (pFrame keeps the last one); -1: decode error.
    int NextVideoFrame();

    // Starts the clock at the first decoded frame.
    void Start();
    bool Started() const { return m_bStarted; }
    double ClockMs();

    const D2FFmpeg* ff = nullptr;
    AVFormatContext* pFormat = nullptr;
    AVCodecContext* pVideo = nullptr;
    AVFrame* pFrame = nullptr;      // the current video frame
    int nVideoStream = -1;
    int nAudioStream = -1;
    uint32_t nWidth = 0;
    uint32_t nHeight = 0;
    uint32_t nFrames = 0;           // the container's count, 0 when it has none
    AVRational frameRate = {0, 1};
    bool bEof = false;

private:
    bool InitStreams(bool bAudio);
    void PumpAudio(AVPacket* pPacket);
    static int IoRead(void* pOpaque, uint8_t* pBuf, int nSize);
    static int64_t IoSeek(void* pOpaque, int64_t nOffset, int nWhence);

    AVIOContext* m_pIo = nullptr;
    HANDLE m_hFile = nullptr;
    int64_t m_nBase = 0;
    int64_t m_nSize = 0;
    int64_t m_nPos = 0;
    AVCodecContext* m_pAudio = nullptr;
    AVFrame* m_pAudioFrame = nullptr;
    AVPacket* m_pPacket = nullptr;
    SwrContext* m_pSwr = nullptr;
    D2PcmOut m_pcm;
    int m_nPcmRate = 0;
    int m_nPcmChannels = 0;
    int16_t* m_pPcmBuf = nullptr;
    int m_nPcmBufFrames = 0;
    bool m_bStarted = false;
    ULONGLONG m_nWallStart = 0;
    double m_fLastAudioMs = -1.0;
    ULONGLONG m_nWallAtLastAudio = 0;
};
