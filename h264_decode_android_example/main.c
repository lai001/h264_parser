// clang-format off
#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>
#include <pthread.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <OMXAL/OpenMAXAL.h>
#include <media/NdkMediaCodec.h>

#include "h264_android_codec/h264_android_codec.h"
#include "h264_parse/h264_parser.h"
#include "util/util.h"
#include "log.h"
#include "util/util_buffer_block.h"

#include "h264_android_codec/decode_async.h"
// clang-format on

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
