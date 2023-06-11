#include <stdio.h>

#include "hv/hbase.h"
#include "hv/hloop.h"
#include "hv/hsocket.h"

#include "h264_parse/h264_parser.h"
#include "log.h"
#include "util/util.h"

static void getOutputBuffer(void *userData, const char *buffer, const size_t bufferSize)
{
    static int fileSuffix = 0;
    char filePath[1024];
    Util_tryCreateFolder("./nalu");
    sprintf(filePath, "./nalu/nalu_%i.bin", fileSuffix);
    fileSuffix += 1;
    log_trace("filePath: %s", filePath);
    Util_saveBufferToFile(buffer, bufferSize, filePath);
}

static void doParse()
{
    H264Parser *h264Parser = H264Parser_new("./assets/output.264", 5 * 1024 * 1024);
    H264Parser_setOutputBufferCallback(h264Parser, NULL, getOutputBuffer);
    H264Parser_start(h264Parser);
    H264Parser_delete(h264Parser);
}

int main(int argc, char **argv)
{
    doParse();
    return 0;
}
