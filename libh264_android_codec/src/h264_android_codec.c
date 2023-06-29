// clang-format off
#include "h264_android_codec/h264_android_codec.h"
#include <stdio.h>
#include <OMXAL/OpenMAXAL.h>
#include <sys/stat.h>
#include "util/util.h"
#include "log.h"
// clang-format on

static const char* TAG = "[H264AndroidCodec]";

char *H264AndroidCodec_getColorFormatName(unsigned int code, char *name)
{
    if (name)
    {
        if (code == XA_COLORFORMAT_YUV420PLANAR)
        {
            name = "YUV420PLANAR";
        }
        else if (code == XA_COLORFORMAT_YUV420SEMIPLANAR)
        {
            name = "YUV420SEMIPLANAR";
        }
        else if (code == XA_COLORFORMAT_YUV422PLANAR)
        {
            name = "YUV422PLANAR";
        }
        else if (code == XA_COLORFORMAT_YUV422SEMIPLANAR)
        {
            name = "YUV422SEMIPLANAR";
        }
    }
    return name;
}

H264AndroidCodec *H264AndroidCodec_new(const H264AndroidCodecOpenOptions options)
{

    H264AndroidCodec *androidCodec = (H264AndroidCodec *)malloc(sizeof(H264AndroidCodec));
    AMediaCodec *mediaCodec = AMediaCodec_createDecoderByType("video/avc");
    AMediaFormat *mediaFormat = AMediaFormat_new();

    if (androidCodec && mediaCodec && mediaFormat)
    {
        AMediaFormat_setString(mediaFormat, "mime", "video/avc");
        AMediaFormat_setInt32(mediaFormat, AMEDIAFORMAT_KEY_WIDTH, options.width);
        AMediaFormat_setInt32(mediaFormat, AMEDIAFORMAT_KEY_HEIGHT, options.height);
        AMediaCodec_configure(mediaCodec, mediaFormat, NULL, NULL, 0);
        androidCodec->mediaCodec = mediaCodec;
        androidCodec->mediaFormat = mediaFormat;
        androidCodec->getOutputBufferCallbackUserData = NULL;
    }
    else
    {
        if (mediaCodec)
        {
            AMediaCodec_delete(mediaCodec);
        }
        if (mediaFormat)
        {
            AMediaFormat_delete(mediaFormat);
        }
        if (androidCodec)
        {
            free(androidCodec);
        }
        androidCodec = NULL;
    }

    return androidCodec;
}

void H264AndroidCodec_start(H264AndroidCodec *androidCodec)
{
    assert(androidCodec);
    AMediaCodec_start(androidCodec->mediaCodec);
}

void H264AndroidCodec_delete(H264AndroidCodec *androidCodec)
{
    assert(androidCodec);
    AMediaFormat_delete(androidCodec->mediaFormat);
    AMediaCodec_delete(androidCodec->mediaCodec);
    free(androidCodec);
}

void H264AndroidCodec_stop(H264AndroidCodec *androidCodec)
{
    assert(androidCodec);
    AMediaCodec_stop(androidCodec->mediaCodec);
}

int H264AndroidCodec_getOutputBuffer(H264AndroidCodec *androidCodec, char **buffer, size_t *outputBufferSize,
                                     const long int timeoutUs)
{
    assert(androidCodec);
    *buffer = NULL;
    *outputBufferSize = 0;

    ssize_t outputBufferIndex;
    do
    {
        AMediaCodecBufferInfo mediaCodecBufferInfo;
        outputBufferIndex = AMediaCodec_dequeueOutputBuffer(androidCodec->mediaCodec, &mediaCodecBufferInfo, 2000);

        log_trace("%s outputBufferIndex : %d", TAG, outputBufferIndex);

        if (outputBufferIndex >= 0)
        {
            int colorFormat;
            int framebufferSize = 0;
            {
                int frameWidth, frameHeight;
                AMediaFormat *format = AMediaCodec_getOutputFormat(androidCodec->mediaCodec);
                AMediaFormat_getInt32(format, AMEDIAFORMAT_KEY_WIDTH, &frameWidth);
                AMediaFormat_getInt32(format, AMEDIAFORMAT_KEY_HEIGHT, &frameHeight);
                AMediaFormat_getInt32(format, AMEDIAFORMAT_KEY_COLOR_FORMAT, &colorFormat);
                framebufferSize = mediaCodecBufferInfo.size;
            }

            *buffer = AMediaCodec_getOutputBuffer(androidCodec->mediaCodec, outputBufferIndex, outputBufferSize);
            char colorFormatName[256];
            log_trace("%s framebufferSize: %d, color format name: %s", TAG, framebufferSize,
                      H264AndroidCodec_getColorFormatName(colorFormat, colorFormatName));

            AMediaCodec_releaseOutputBuffer(androidCodec->mediaCodec, outputBufferIndex, false);
            return 1;
        }
        else if (outputBufferIndex == AMEDIACODEC_INFO_OUTPUT_FORMAT_CHANGED)
        {
            int frameWidth, frameHeight;
            AMediaFormat *format = AMediaCodec_getOutputFormat(androidCodec->mediaCodec);
            AMediaFormat_getInt32(format, "width", &frameWidth);
            AMediaFormat_getInt32(format, "height", &frameHeight);
            int32_t colorFormat;

            AMediaFormat_getInt32(format, AMEDIAFORMAT_KEY_COLOR_FORMAT, &colorFormat);

            log_trace("%s AMEDIACODEC_INFO_OUTPUT_FORMAT_CHANGED", TAG);
        }
        else if (outputBufferIndex == AMEDIACODEC_INFO_OUTPUT_BUFFERS_CHANGED)
        {
            log_trace("%s AMEDIACODEC_INFO_OUTPUT_BUFFERS_CHANGED", TAG);
        }
        else if (outputBufferIndex == AMEDIACODEC_INFO_TRY_AGAIN_LATER)
        {
            log_trace("%s AMEDIACODEC_INFO_TRY_AGAIN_LATER", TAG);
        }
        else
        {
            log_trace("%s Unknow", TAG);
        }
    } while (outputBufferIndex > 0);

    return 0;
}

int H264AndroidCodec_queueInputBuffer(H264AndroidCodec *androidCodec, char *buffer, const unsigned int bufferSize,
                                      const long int timeoutUs)
{
    assert(androidCodec);
    assert(buffer);
    assert(bufferSize > 0);
    ssize_t inputBufferIndex = AMediaCodec_dequeueInputBuffer(androidCodec->mediaCodec, timeoutUs);
    if (inputBufferIndex >= 0)
    {
        size_t intputBufferSize;
        char *mediaCodecGetInputBuffer =
            (char *)AMediaCodec_getInputBuffer(androidCodec->mediaCodec, inputBufferIndex, &intputBufferSize);
        if (mediaCodecGetInputBuffer && intputBufferSize > 0)
        {
            log_trace("%s inputBufferIndex: %d, intputBufferSize: %d", TAG, inputBufferIndex, intputBufferSize);
            memcpy(mediaCodecGetInputBuffer, buffer, min(bufferSize, intputBufferSize));
            AMediaCodec_queueInputBuffer(androidCodec->mediaCodec, inputBufferIndex, 0,
                                         min(bufferSize, intputBufferSize), 0, 0);

            char *decodedBuffer;
            size_t decodedBufferSize;
            H264AndroidCodec_getOutputBuffer(androidCodec, &decodedBuffer, &decodedBufferSize, 2000);
            if (decodedBuffer && decodedBufferSize > 0)
            {
                if (androidCodec->getOutputBufferCallback)
                {
                    androidCodec->getOutputBufferCallback(androidCodec->getOutputBufferCallbackUserData, decodedBuffer,
                                                          decodedBufferSize);
                }
            }
        }
        else
        {
            return 0;
        }
    }
    else
    {
        return 0;
    }
}

void H264AndroidCodec_setOutputBufferCallback(H264AndroidCodec *androidCodec, void *getOutputBufferCallbackUserData,
                                              GetOutputBufferCallback getOutputBufferCallback)
{
    assert(androidCodec);
    androidCodec->getOutputBufferCallback = getOutputBufferCallback;
    androidCodec->getOutputBufferCallbackUserData = getOutputBufferCallbackUserData;
}