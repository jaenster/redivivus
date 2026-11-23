// See d2_media.h.
#include "d2_media.h"

#include <stdio.h>
#include <string.h>

static const int kIoBufferSize = 64 * 1024;

int D2Media::IoRead(void* pOpaque, uint8_t* pBuf, int nSize) {
    D2Media* pThis = (D2Media*)pOpaque;
    if (pThis->m_nSize > 0) {
        int64_t nLeft = pThis->m_nSize - pThis->m_nPos;
        if (nLeft <= 0) return AVERROR_EOF;
        if (nSize > nLeft) nSize = (int)nLeft;
    }
    DWORD nRead = 0;
    if (!ReadFile(pThis->m_hFile, pBuf, (DWORD)nSize, &nRead, nullptr)) return AVERROR(EIO);
    if (nRead == 0) return AVERROR_EOF;
    pThis->m_nPos += nRead;
    return (int)nRead;
}

int64_t D2Media::IoSeek(void* pOpaque, int64_t nOffset, int nWhence) {
    D2Media* pThis = (D2Media*)pOpaque;
    if (nWhence & AVSEEK_SIZE) {
        if (pThis->m_nSize > 0) return pThis->m_nSize;
        LARGE_INTEGER liSize;
        if (!GetFileSizeEx(pThis->m_hFile, &liSize)) return AVERROR(EIO);
        return liSize.QuadPart - pThis->m_nBase;
    }
    int64_t nTarget;
    switch (nWhence & ~AVSEEK_FORCE) {
        case SEEK_SET: nTarget = nOffset; break;
        case SEEK_CUR: nTarget = pThis->m_nPos + nOffset; break;
        case SEEK_END: {
            int64_t nEnd = pThis->m_nSize;
            if (nEnd <= 0) {
                LARGE_INTEGER liSize;
                if (!GetFileSizeEx(pThis->m_hFile, &liSize)) return AVERROR(EIO);
                nEnd = liSize.QuadPart - pThis->m_nBase;
            }
            nTarget = nEnd + nOffset;
            break;
        }
        default: return AVERROR(EINVAL);
    }
    if (nTarget < 0) return AVERROR(EINVAL);
    LARGE_INTEGER liMove;
    liMove.QuadPart = pThis->m_nBase + nTarget;
    if (!SetFilePointerEx(pThis->m_hFile, liMove, nullptr, FILE_BEGIN)) return AVERROR(EIO);
    pThis->m_nPos = nTarget;
    return nTarget;
}

bool D2Media::OpenPath(const char* pszPath, bool bAudio) {
    Close();
    ff = D2FFmpeg_Get();
    if (!ff || !pszPath) return false;
    if (ff->avformat_open_input(&pFormat, pszPath, nullptr, nullptr) < 0) {
        pFormat = nullptr;
        return false;
    }
    return InitStreams(bAudio);
}

bool D2Media::OpenHandle(HANDLE hFile, int64_t nSize, bool bAudio) {
    Close();
    ff = D2FFmpeg_Get();
    if (!ff || !hFile || hFile == INVALID_HANDLE_VALUE) return false;
    LARGE_INTEGER liZero, liPos;
    liZero.QuadPart = 0;
    if (!SetFilePointerEx(hFile, liZero, &liPos, FILE_CURRENT)) return false;
    m_hFile = hFile;
    m_nBase = liPos.QuadPart;
    m_nSize = nSize;
    m_nPos = 0;
    uint8_t* pIoBuf = (uint8_t*)ff->av_malloc(kIoBufferSize);
    if (!pIoBuf) return false;
    m_pIo = ff->avio_alloc_context(pIoBuf, kIoBufferSize, 0, this, IoRead, nullptr, IoSeek);
    if (!m_pIo) {
        ff->av_free(pIoBuf);
        return false;
    }
    pFormat = ff->avformat_alloc_context();
    if (!pFormat) return false;
    pFormat->pb = m_pIo;
    pFormat->flags |= AVFMT_FLAG_CUSTOM_IO;
    if (ff->avformat_open_input(&pFormat, "", nullptr, nullptr) < 0) {
        pFormat = nullptr;  // avformat_open_input frees it on failure
        return false;
    }
    return InitStreams(bAudio);
}

bool D2Media::InitStreams(bool bAudio) {
    if (ff->avformat_find_stream_info(pFormat, nullptr) < 0) return false;
    for (unsigned i = 0; i < pFormat->nb_streams; ++i) {
        AVCodecParameters* pPar = pFormat->streams[i]->codecpar;
        if (pPar->codec_type == AVMEDIA_TYPE_VIDEO && nVideoStream < 0) nVideoStream = (int)i;
        if (pPar->codec_type == AVMEDIA_TYPE_AUDIO && nAudioStream < 0 && bAudio) nAudioStream = (int)i;
    }
    if (nVideoStream < 0) return false;

    AVStream* pVs = pFormat->streams[nVideoStream];
    const AVCodec* pVCodec = ff->avcodec_find_decoder(pVs->codecpar->codec_id);
    if (!pVCodec) return false;
    pVideo = ff->avcodec_alloc_context3(pVCodec);
    if (!pVideo || ff->avcodec_parameters_to_context(pVideo, pVs->codecpar) < 0) return false;
    if (ff->avcodec_open2(pVideo, pVCodec, nullptr) < 0) return false;
    nWidth = (uint32_t)pVs->codecpar->width;
    nHeight = (uint32_t)pVs->codecpar->height;
    nFrames = pVs->nb_frames > 0 ? (uint32_t)pVs->nb_frames : 0;
    frameRate = pVs->avg_frame_rate.num > 0 ? pVs->avg_frame_rate : pVs->r_frame_rate;
    if (frameRate.num <= 0 || frameRate.den <= 0) {
        // Fall back on the stream clock: one tick per frame is how both demuxers set it.
        frameRate.num = pVs->time_base.den;
        frameRate.den = pVs->time_base.num > 0 ? pVs->time_base.num : 1;
    }

    if (nAudioStream >= 0) {
        AVStream* pAs = pFormat->streams[nAudioStream];
        const AVCodec* pACodec = ff->avcodec_find_decoder(pAs->codecpar->codec_id);
        if (pACodec) {
            m_pAudio = ff->avcodec_alloc_context3(pACodec);
            if (m_pAudio && ff->avcodec_parameters_to_context(m_pAudio, pAs->codecpar) >= 0 &&
                ff->avcodec_open2(m_pAudio, pACodec, nullptr) >= 0) {
                m_nPcmRate = m_pAudio->sample_rate;
                m_nPcmChannels = m_pAudio->ch_layout.nb_channels > 2 ? 2 : m_pAudio->ch_layout.nb_channels;
                if (m_nPcmChannels < 1) m_nPcmChannels = 1;
                AVChannelLayout outLayout;
                ff->av_channel_layout_default(&outLayout, m_nPcmChannels);
                int rc = ff->swr_alloc_set_opts2(&m_pSwr, &outLayout, AV_SAMPLE_FMT_S16, m_nPcmRate,
                                                 &m_pAudio->ch_layout, m_pAudio->sample_fmt,
                                                 m_pAudio->sample_rate, 0, nullptr);
                ff->av_channel_layout_uninit(&outLayout);
                if (rc < 0 || ff->swr_init(m_pSwr) < 0 || !m_pcm.Open(m_nPcmRate, m_nPcmChannels)) {
                    // Audio is not worth failing the video for.
                    if (m_pSwr) ff->swr_free(&m_pSwr);
                    ff->avcodec_free_context(&m_pAudio);
                    nAudioStream = -1;
                }
            } else {
                if (m_pAudio) ff->avcodec_free_context(&m_pAudio);
                nAudioStream = -1;
            }
        } else {
            nAudioStream = -1;
        }
        if (nAudioStream >= 0) m_pAudioFrame = ff->av_frame_alloc();
    }

    pFrame = ff->av_frame_alloc();
    m_pPacket = ff->av_packet_alloc();
    return pFrame && m_pPacket;
}

void D2Media::Close() {
    if (ff) {
        if (m_pPcmBuf) ff->av_free(m_pPcmBuf);
        if (m_pSwr) ff->swr_free(&m_pSwr);
        if (m_pAudioFrame) ff->av_frame_free(&m_pAudioFrame);
        if (pFrame) ff->av_frame_free(&pFrame);
        if (m_pPacket) ff->av_packet_free(&m_pPacket);
        if (m_pAudio) ff->avcodec_free_context(&m_pAudio);
        if (pVideo) ff->avcodec_free_context(&pVideo);
        if (pFormat) ff->avformat_close_input(&pFormat);
        if (m_pIo) {
            ff->av_free(m_pIo->buffer);
            ff->avio_context_free(&m_pIo);
        }
    }
    m_pcm.Close();
    m_pPcmBuf = nullptr;
    m_nPcmBufFrames = 0;
    m_pSwr = nullptr;
    m_pAudioFrame = nullptr;
    pFrame = nullptr;
    m_pPacket = nullptr;
    m_pAudio = nullptr;
    pVideo = nullptr;
    pFormat = nullptr;
    m_pIo = nullptr;
    m_hFile = nullptr;
    nVideoStream = nAudioStream = -1;
    nWidth = nHeight = nFrames = 0;
    bEof = false;
    m_bStarted = false;
    m_fLastAudioMs = -1.0;
}

void D2Media::PumpAudio(AVPacket* pPacket) {
    if (!m_pAudio || ff->avcodec_send_packet(m_pAudio, pPacket) < 0) return;
    while (ff->avcodec_receive_frame(m_pAudio, m_pAudioFrame) >= 0) {
        int nSamples = m_pAudioFrame->nb_samples;
        if (nSamples > m_nPcmBufFrames) {
            if (m_pPcmBuf) ff->av_free(m_pPcmBuf);
            m_nPcmBufFrames = nSamples + 256;
            m_pPcmBuf = (int16_t*)ff->av_malloc((size_t)m_nPcmBufFrames * m_nPcmChannels * sizeof(int16_t));
            if (!m_pPcmBuf) {
                m_nPcmBufFrames = 0;
                ff->av_frame_unref(m_pAudioFrame);
                return;
            }
        }
        uint8_t* apOut[1] = {(uint8_t*)m_pPcmBuf};
        int nOut = ff->swr_convert(m_pSwr, apOut, m_nPcmBufFrames,
                                   (const uint8_t**)m_pAudioFrame->extended_data, nSamples);
        if (nOut > 0) m_pcm.Write(m_pPcmBuf, nOut);
        ff->av_frame_unref(m_pAudioFrame);
    }
}

int D2Media::NextVideoFrame() {
    if (!pFormat || bEof) return 0;
    for (;;) {
        // A frame the decoder already holds?
        int rc = ff->avcodec_receive_frame(pVideo, pFrame);
        if (rc >= 0) return 1;
        if (rc != AVERROR(EAGAIN) && rc != AVERROR_EOF) return -1;
        if (rc == AVERROR_EOF) {
            bEof = true;
            return 0;
        }
        rc = ff->av_read_frame(pFormat, m_pPacket);
        if (rc < 0) {
            // Drain, then stop.
            ff->avcodec_send_packet(pVideo, nullptr);
            if (m_pAudio) PumpAudio(nullptr);
            rc = ff->avcodec_receive_frame(pVideo, pFrame);
            bEof = true;
            return rc >= 0 ? 1 : 0;
        }
        if (m_pPacket->stream_index == nVideoStream) {
            rc = ff->avcodec_send_packet(pVideo, m_pPacket);
            ff->av_packet_unref(m_pPacket);
            if (rc < 0 && rc != AVERROR(EAGAIN)) return -1;
        } else if (m_pPacket->stream_index == nAudioStream) {
            PumpAudio(m_pPacket);
            ff->av_packet_unref(m_pPacket);
        } else {
            ff->av_packet_unref(m_pPacket);
        }
    }
}

void D2Media::Start() {
    if (m_bStarted) return;
    m_bStarted = true;
    m_nWallStart = GetTickCount64();
}

double D2Media::ClockMs() {
    ULONGLONG nNow = GetTickCount64();
    if (m_pcm.IsOpen()) {
        if (m_pcm.IsPlaying()) {
            double fPos = m_pcm.PositionMs();
            if (fPos >= 0.0) {
                m_fLastAudioMs = fPos;
                m_nWallAtLastAudio = nNow;
                return fPos;
            }
        } else if (m_fLastAudioMs >= 0.0) {
            return m_fLastAudioMs + (double)(nNow - m_nWallAtLastAudio);
        }
    }
    if (!m_bStarted) return 0.0;
    return (double)(nNow - m_nWallStart);
}
