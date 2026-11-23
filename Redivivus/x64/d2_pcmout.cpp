// See d2_pcmout.h.
#include "d2_pcmout.h"

#include <stdlib.h>
#include <string.h>

bool D2PcmOut::Open(int nSampleRate, int nChannels) {
    Close();
    if (nSampleRate <= 0 || nChannels <= 0) return false;
    WAVEFORMATEX wfx;
    memset(&wfx, 0, sizeof wfx);
    wfx.wFormatTag = WAVE_FORMAT_PCM;
    wfx.nChannels = (WORD)nChannels;
    wfx.nSamplesPerSec = (DWORD)nSampleRate;
    wfx.wBitsPerSample = 16;
    wfx.nBlockAlign = (WORD)(nChannels * 2);
    wfx.nAvgBytesPerSec = wfx.nSamplesPerSec * wfx.nBlockAlign;
    if (waveOutOpen(&m_hWave, WAVE_MAPPER, &wfx, 0, 0, CALLBACK_NULL) != MMSYSERR_NOERROR) {
        m_hWave = nullptr;
        return false;
    }
    m_nRate = nSampleRate;
    m_nChannels = nChannels;
    m_nQueuedFrames = 0;
    return true;
}

void D2PcmOut::Close() {
    if (!m_hWave) return;
    waveOutReset(m_hWave);
    while (m_pBlocks) {
        Block* pBlock = m_pBlocks;
        m_pBlocks = pBlock->pNext;
        waveOutUnprepareHeader(m_hWave, &pBlock->hdr, sizeof pBlock->hdr);
        free(pBlock->hdr.lpData);
        free(pBlock);
    }
    waveOutClose(m_hWave);
    m_hWave = nullptr;
    m_nQueuedFrames = 0;
}

// Frees the blocks the device has finished with. The list keeps queue order, so
// finished blocks are at the front; stop at the first one still playing.
void D2PcmOut::Reap() {
    while (m_pBlocks && (m_pBlocks->hdr.dwFlags & WHDR_DONE)) {
        Block* pBlock = m_pBlocks;
        m_pBlocks = pBlock->pNext;
        waveOutUnprepareHeader(m_hWave, &pBlock->hdr, sizeof pBlock->hdr);
        free(pBlock->hdr.lpData);
        free(pBlock);
    }
}

bool D2PcmOut::Write(const int16_t* pSamples, int nFrames) {
    if (!m_hWave || nFrames <= 0) return false;
    Reap();
    size_t nBytes = (size_t)nFrames * m_nChannels * sizeof(int16_t);
    Block* pBlock = (Block*)calloc(1, sizeof(Block));
    if (!pBlock) return false;
    pBlock->hdr.lpData = (LPSTR)malloc(nBytes);
    if (!pBlock->hdr.lpData) {
        free(pBlock);
        return false;
    }
    memcpy(pBlock->hdr.lpData, pSamples, nBytes);
    pBlock->hdr.dwBufferLength = (DWORD)nBytes;
    if (waveOutPrepareHeader(m_hWave, &pBlock->hdr, sizeof pBlock->hdr) != MMSYSERR_NOERROR ||
        waveOutWrite(m_hWave, &pBlock->hdr, sizeof pBlock->hdr) != MMSYSERR_NOERROR) {
        waveOutUnprepareHeader(m_hWave, &pBlock->hdr, sizeof pBlock->hdr);
        free(pBlock->hdr.lpData);
        free(pBlock);
        return false;
    }
    Block** ppTail = &m_pBlocks;
    while (*ppTail) ppTail = &(*ppTail)->pNext;
    *ppTail = pBlock;
    m_nQueuedFrames += (uint64_t)nFrames;
    return true;
}

double D2PcmOut::PositionMs() const {
    if (!m_hWave || !m_nQueuedFrames) return -1.0;
    MMTIME mmt;
    mmt.wType = TIME_SAMPLES;
    if (waveOutGetPosition(m_hWave, &mmt, sizeof mmt) != MMSYSERR_NOERROR) return -1.0;
    if (mmt.wType == TIME_SAMPLES) return mmt.u.sample * 1000.0 / m_nRate;
    if (mmt.wType == TIME_BYTES) return mmt.u.cb * 1000.0 / ((double)m_nRate * m_nChannels * 2);
    if (mmt.wType == TIME_MS) return (double)mmt.u.ms;
    return -1.0;
}

bool D2PcmOut::IsPlaying() const {
    for (const Block* pBlock = m_pBlocks; pBlock; pBlock = pBlock->pNext) {
        if (!(pBlock->hdr.dwFlags & WHDR_DONE)) return true;
    }
    return false;
}
