#ifndef H264_ANDROID_CODEC_H
#define H264_ANDROID_CODEC_H

// clang-format off
#include "h264_android_codec_define.h"
#include <media/NdkMediaCodec.h>
// clang-format on

#ifdef __cplusplus
extern "C"
{
#endif

    typedef void (*GetOutputBufferCallback)(void *userData, char *buffer, const size_t bufferSize);

    typedef struct H264_ANDROID_CODEC_API H264AndroidCodec
    {
        AMediaCodec *mediaCodec;
        AMediaFormat *mediaFormat;
        void *getOutputBufferCallbackUserData;
        GetOutputBufferCallback getOutputBufferCallback;
    } H264AndroidCodec;

    typedef struct H264_ANDROID_CODEC_API H264AndroidCodecOpenOptions
    {
        unsigned int width;
        unsigned int height;
    } H264AndroidCodecOpenOptions;

    H264_ANDROID_CODEC_API H264AndroidCodec *H264AndroidCodec_new(const H264AndroidCodecOpenOptions options);
    H264_ANDROID_CODEC_API void H264AndroidCodec_delete(H264AndroidCodec *androidCodec);

    H264_ANDROID_CODEC_API void H264AndroidCodec_start(H264AndroidCodec *androidCodec);
    H264_ANDROID_CODEC_API void H264AndroidCodec_stop(H264AndroidCodec *androidCodec);

    H264_ANDROID_CODEC_API int H264AndroidCodec_queueInputBuffer(H264AndroidCodec *androidCodec, char *buffer,
                                                                 const unsigned int bufferSize,
                                                                 const long int timeoutUs);

    H264_ANDROID_CODEC_API void H264AndroidCodec_setOutputBufferCallback(
        H264AndroidCodec *androidCodec, void *getOutputBufferCallbackUserData,
        GetOutputBufferCallback getOutputBufferCallback);

    H264_ANDROID_CODEC_API char *H264AndroidCodec_getColorFormatName(unsigned int code, char *name);

#ifdef __cplusplus
}
#endif

#endif
