#ifndef LIBSERVER_H
#define LIBSERVER_H

#include <pthread.h>
#include "h264_android_codec/h264_android_codec.h"
#include "hv/hbase.h"
#include "hv/hloop.h"
#include "hv/hsocket.h"
#include "libserver/libserver_define.h"
#include "log.h"
#include "util/util.h"

#ifdef __cplusplus
extern "C"
{
#endif

    typedef void (*LibServer_GetOutputBufferCallback)(void *userData, char *buffer, const size_t bufferSize);

    typedef struct LIBSERVER_API LibServerVolatile
    {
        UtilRingBuffer *ringBuffer;
        H264AndroidCodec *codec;
        hloop_t *loop;
        hio_t *upstreamIO;
    } LibServerVolatile;

    typedef struct LIBSERVER_API LibServer
    {
        pthread_mutex_t serverVolatileMutex;
        LibServerVolatile serverVolatile;
        LibServer_GetOutputBufferCallback getOutputBufferCallback;
        void *userdata;
        char *host;
        int port;
    } LibServer;

    LIBSERVER_API LibServer *LibServer_new(const char *host, const int port);

    LIBSERVER_API void LibServer_delete(LibServer *server);

    LIBSERVER_API void LibServer_start(LibServer *server);

    LIBSERVER_API void LibServer_stop(LibServer *server);

    LIBSERVER_API void LibServer_setOutputBufferCallback(LibServer *server, void *userdata,
                                                         LibServer_GetOutputBufferCallback callback);

    LIBSERVER_API void LibServerVolatile_initialization(LibServerVolatile *serverVolatile, LibServer *server,
                                                        const char *host, const int port,
                                                        H264AndroidCodecOpenOptions options);

    LIBSERVER_API void LibServerVolatile_deinitialization(LibServerVolatile *serverVolatile);

#ifdef __cplusplus
}
#endif

#endif