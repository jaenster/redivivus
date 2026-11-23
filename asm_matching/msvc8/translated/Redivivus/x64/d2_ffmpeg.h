// FFmpeg, loaded at run time. The Bink and Smacker shims decode with
// libavformat/libavcodec (bink, binkaudio, smackvid, smackaud), convert with
// libswscale/libswresample. Nothing links against FFmpeg: the DLLs are found
// beside the executable with LoadLibrary, each entry point with GetProcAddress,
// so the tree stays MIT and FFmpeg stays what its LGPL asks of it - a separate,
// replaceable library. The headers are a compile-time dependency only
// (REDIVIVUS_FFMPEG_INCLUDE); they must come from the same major versions as the
// DLLs that will be present, because struct layouts are read through them.
//
// D2FFmpeg_Get() returns the function table, or nullptr when any DLL or symbol is
// missing. The result is cached; a missing FFmpeg is reported once in the trace.
#pragma once

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavformat/avio.h>
#include <libavutil/avutil.h>
#include <libavutil/channel_layout.h>
#include <libavutil/frame.h>
#include <libavutil/imgutils.h>
#include <libavutil/log.h>
#include <libavutil/mem.h>
#include <libavutil/opt.h>
#include <libswresample/swresample.h>
#include <libswscale/swscale.h>
}

// X(dll, name): every FFmpeg function the shims call, by the DLL that exports it.
#define D2_FFMPEG_FUNCS(X)                   \
    X(avutil, av_frame_alloc)                \
    X(avutil, av_frame_free)                 \
    X(avutil, av_frame_unref)                \
    X(avutil, av_malloc)                     \
    X(avutil, av_free)                       \
    X(avutil, av_log_set_level)              \
    X(avutil, av_strerror)                   \
    X(avutil, av_get_bytes_per_sample)       \
    X(avutil, av_channel_layout_default)     \
    X(avutil, av_channel_layout_uninit)      \
    X(swresample, swr_alloc_set_opts2)       \
    X(swresample, swr_init)                  \
    X(swresample, swr_convert)               \
    X(swresample, swr_free)                  \
    X(swscale, sws_getContext)               \
    X(swscale, sws_scale)                    \
    X(swscale, sws_freeContext)              \
    X(avcodec, avcodec_find_decoder)         \
    X(avcodec, avcodec_alloc_context3)       \
    X(avcodec, avcodec_parameters_to_context) \
    X(avcodec, avcodec_open2)                \
    X(avcodec, avcodec_send_packet)          \
    X(avcodec, avcodec_receive_frame)        \
    X(avcodec, avcodec_free_context)         \
    X(avcodec, av_packet_alloc)              \
    X(avcodec, av_packet_free)               \
    X(avcodec, av_packet_unref)              \
    X(avformat, avformat_alloc_context)      \
    X(avformat, avformat_free_context)       \
    X(avformat, avformat_open_input)         \
    X(avformat, avformat_close_input)        \
    X(avformat, avformat_find_stream_info)   \
    X(avformat, av_read_frame)               \
    X(avformat, avio_alloc_context)          \
    X(avformat, avio_context_free)

struct D2FFmpeg {
#define D2_FFMPEG_MEMBER(dll, name) decltype(&::name) name;
    D2_FFMPEG_FUNCS(D2_FFMPEG_MEMBER)
#undef D2_FFMPEG_MEMBER
};

const D2FFmpeg* D2FFmpeg_Get();

// The DLL names this build was compiled against, for diagnostics and docs.
const char* const* D2FFmpeg_DllNames(int* pnCount);
