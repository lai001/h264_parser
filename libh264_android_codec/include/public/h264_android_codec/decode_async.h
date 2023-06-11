#ifndef DECODE_ASYNC_H
#define DECODE_ASYNC_H

// clang-format off
#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>
#include <pthread.h>
#include <sys/stat.h>
#include <sys/syscall.h>

#include "h264_android_codec_define.h"
#include "h264_android_codec/h264_android_codec.h"
#include "h264_parse/h264_parser.h"
#include "util/util.h"
#include "log.h"
#include "util/util_buffer_block.h"
// clang-format on

#ifdef __cplusplus
extern "C"
{
#endif

    typedef struct H264_ANDROID_CODEC_API DecodeAsyncNode
    {
        UtilBufferBlock *block;
        struct DecodeAsyncNode *next;
    } DecodeAsyncNode;

    typedef struct H264_ANDROID_CODEC_API DecodeAsyncQueue
    {
        struct DecodeAsyncNode *head;
        struct DecodeAsyncNode *tail;
        unsigned int size;
    } DecodeAsyncQueue;

    typedef struct H264_ANDROID_CODEC_API H264AndroidDecodeAsyncContext
    {
        sem_t bin_sem;
        pthread_t tid;
        pthread_mutex_t queueMutex;
        DecodeAsyncQueue *queue;

        H264AndroidCodec *androidCodec;
        H264Parser *h264Parser;

        pthread_mutex_t isStopMutex;
        int isStop;

        H264AndroidCodecOpenOptions options;
        char *filePath;
    } H264AndroidDecodeAsyncContext;

    H264_ANDROID_CODEC_API H264AndroidDecodeAsyncContext *H264AndroidDecodeAsyncContext_new(
        H264AndroidCodecOpenOptions options, char *filePath);

    H264_ANDROID_CODEC_API void H264AndroidDecodeAsyncContext_delete(H264AndroidDecodeAsyncContext *context);

    H264_ANDROID_CODEC_API void H264AndroidDecodeAsyncContext_start(H264AndroidDecodeAsyncContext *context);

    H264_ANDROID_CODEC_API void H264AndroidDecodeAsyncContext_stop(H264AndroidDecodeAsyncContext *context);

    H264_ANDROID_CODEC_API int H264AndroidDecodeAsyncContext_isStop(H264AndroidDecodeAsyncContext *context);

    H264_ANDROID_CODEC_API DecodeAsyncNode *H264AndroidDecodeAsyncContext_dequeue(
        H264AndroidDecodeAsyncContext *context);

    H264_ANDROID_CODEC_API void H264AndroidDecodeAsyncContext_deleteNode(H264AndroidDecodeAsyncContext *context,
                                                                         DecodeAsyncNode *node);

    H264_ANDROID_CODEC_API DecodeAsyncNode *Node_new(const char *buffer, const unsigned int bufferSize);

    H264_ANDROID_CODEC_API DecodeAsyncNode *Node_delete(DecodeAsyncNode *node);

    H264_ANDROID_CODEC_API void Node_getBufferAndSize(DecodeAsyncNode *node, char **buffer, unsigned int *bufferSize);

    H264_ANDROID_CODEC_API DecodeAsyncQueue *Queue_new(DecodeAsyncNode *head);

    H264_ANDROID_CODEC_API void Queue_delete(DecodeAsyncQueue *queue);

    H264_ANDROID_CODEC_API void Queue_enqueue(DecodeAsyncQueue *queue, DecodeAsyncNode *node);

    H264_ANDROID_CODEC_API DecodeAsyncNode *Queue_dequeue(DecodeAsyncQueue *queue);

#ifdef __cplusplus
}
#endif

#endif