// clang-format off
#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>
#include <pthread.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <OMXAL/OpenMAXAL.h>
#include <media/NdkMediaCodec.h>

#include "hv/hbase.h"
#include "hv/hloop.h"
#include "hv/hsocket.h"

#include "h264_android_codec/h264_android_codec.h"
#include "h264_parse/h264_parser.h"
#include "util/util.h"
#include "log.h"
#include "util/util_buffer_block.h"

#include "h264_android_codec/decode_async.h"
// clang-format on

static void onClose(hio_t *io)
{
    log_trace("onClose fd=%d error=%d\n", hio_fd(io), hio_error(io));
}

static void onRead(hio_t *io, void *buf, int readbytes)
{
    log_trace("onRead fd=%d readbytes=%d\n", hio_fd(io), readbytes);
    char localaddrstr[SOCKADDR_STRLEN] = {0};
    char peeraddrstr[SOCKADDR_STRLEN] = {0};
    log_trace("[%s] <=> [%s]\n", SOCKADDR_STR(hio_localaddr(io), localaddrstr),
              SOCKADDR_STR(hio_peeraddr(io), peeraddrstr));
    log_trace("< %.*s", readbytes, (char *)buf);
    //// echo
    // log_trace("> %.*s", readbytes, (char *)buf);
    // hio_write(io, buf, readbytes);
}

static void onAccept(hio_t *io)
{
    log_trace("onAccept connfd=%d\n", hio_fd(io));
    char localaddrstr[SOCKADDR_STRLEN] = {0};
    char peeraddrstr[SOCKADDR_STRLEN] = {0};
    log_trace("accept connfd=%d [%s] <= [%s]\n", hio_fd(io), SOCKADDR_STR(hio_localaddr(io), localaddrstr),
              SOCKADDR_STR(hio_peeraddr(io), peeraddrstr));
    hio_setcb_close(io, onClose);
    hio_setcb_read(io, onRead);
    hio_read_start(io);
}

static int runServer()
{
    hloop_t *loop = hloop_new(0);
    hio_t *listenio = hio_create_socket(loop, "localhost", 9999, HIO_TYPE_SOCK_STREAM, HIO_SERVER_SIDE);
    if (listenio == NULL)
    {
        return -1;
    };
    hio_setcb_accept(listenio, onAccept);
    if (hio_accept(listenio) != 0)
    {
        return -1;
    }
    log_trace("listenfd=%d\n", hio_fd(listenio));

    char localaddrstr[SOCKADDR_STRLEN] = {0};
    log_trace(" [%s]\n", SOCKADDR_STR(hio_localaddr(listenio), localaddrstr));

    hloop_run(loop);
    hloop_free(&loop);
    return 0;
}

int main(int argc, char **argv)
{
    H264AndroidCodecOpenOptions options;
    options.width = 1920;
    options.height = 1080;
    H264AndroidDecodeAsyncContext *context = H264AndroidDecodeAsyncContext_new(options, "./assets/output.264");
    H264AndroidDecodeAsyncContext_start(context);

    while (H264AndroidDecodeAsyncContext_isStop(context) == 0)
    {
        usleep(17 * 1000);
        DecodeAsyncNode *node = H264AndroidDecodeAsyncContext_dequeue(context);

        if (node)
        {
            char *decodedBuffer;
            unsigned int decodedBufferSize = 0;
            Node_getBufferAndSize(node, &decodedBuffer, &decodedBufferSize);
            static int yuvDataIndex = 0;
            char filePath[256];
            Util_tryCreateFolder("./yuv");
            sprintf(filePath, "./yuv/yuv_%d.yuv", yuvDataIndex);
            log_trace("filePath: %s , decodedBufferSize: %d\n", filePath, decodedBufferSize);
            FILE *fp = fopen(filePath, "wb");
            if (fp)
            {
                fwrite(decodedBuffer, 1, decodedBufferSize, fp);
                fclose(fp);
            }
            yuvDataIndex += 1;
            H264AndroidDecodeAsyncContext_deleteNode(context, node);
        }
    }
    H264AndroidDecodeAsyncContext_delete(context);
    return 0;
}
