#include "h264_parse/h264_parser.h"
#include "h264_parse/h264_nal_header.h"
#include "log.h"
#include "util/util.h"
#include "util/util_buffer_block.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

static void enqueue(char buffer[4], char value)
{
    for (size_t i = 0; i < 3; i++)
    {
        buffer[i] = buffer[i + 1];
    }
    buffer[3] = value;
}

H264Parser *H264Parser_new(const char *filePath, const unsigned int maxBufferSize)
{
    assert(filePath);

    H264ParseReader *reader = H264ParseReader_new(filePath, maxBufferSize);
    H264Parser *parser = (H264Parser *)malloc(sizeof(H264Parser));

    if (reader && parser)
    {
        parser->reader = reader;
        parser->h264ParserCallbackUserData = NULL;
    }
    else
    {
        if (reader)
        {
            H264ParseReader_delete(reader);
        }
        if (parser)
        {
            free(parser);
        }
        parser = NULL;
    }

    return parser;
}

void H264Parser_delete(H264Parser *h264Parser)
{
    assert(h264Parser);
    if (h264Parser->reader)
    {
        H264ParseReader_delete(h264Parser->reader);
    }
    free(h264Parser);
}

void H264Parser_start(H264Parser *h264Parser)
{
    assert(h264Parser);

    const unsigned int bufferSize = 1 * 1024 * 1024;
    char *buffer = (char *)malloc(sizeof(char) * bufferSize);
    if (buffer == NULL)
    {
        return;
    }

    unsigned int totalActualBufferRead = 0;
    unsigned int position = 0;

    char core[4] = {0, 0, 0, 0};
    char capacity = 0;

    UtilBufferBlock *block = UtilBufferBlock_new(0);

    int startIndex = -1;
    int endIndex = -1;

    int isStop = 0;

    while (H264ParseReader_isReadable(h264Parser->reader))
    {
        unsigned int actualBufferRead = H264ParseReader_read(h264Parser->reader, buffer, bufferSize);
        totalActualBufferRead += actualBufferRead;

        UtilBufferBlock_increaseSize(block, actualBufferRead);
        UtilBufferBlock_copyFrom(block, UtilBufferBlock_getSize(block) - actualBufferRead, buffer, bufferSize);

        // log_debug("bufferSize: %i, actualBufferRead: %i, totalActualBufferRead: %i", bufferSize, actualBufferRead,
        //           totalActualBufferRead);

        for (unsigned int i = 0; i < actualBufferRead; i++)
        {
            enqueue(core, *(buffer + i));
            capacity = min(capacity + 1, 4);

            if (capacity > 2)
            {
                char startCodeSize = 0;

                if (H264Parser_isStartCode(core, &startCodeSize))
                {
                    if (endIndex == -1 && startIndex == -1)
                    {
                        startIndex = i + 1 - startCodeSize;
                    }
                    else if (endIndex == -1 && startIndex > -1)
                    {
                        endIndex = UtilBufferBlock_getSize(block) - (actualBufferRead - i) - startCodeSize;
                    }
                    else
                    {
                        assert(false);
                    }
                    if (endIndex > -1 && startIndex > -1)
                    {

                        // char a = *UtilBufferBlock_getBuffer(saveBlock, 0);
                        // H264NALHeader header;
                        // H264NALHeader_initialization(&header, a);

                        // log_trace("nal_unit_type: %s", ENALUnitType_getName(header.nal_unit_type));

                        if (h264Parser->getOutputBufferCallback)
                        {
                            UtilBufferBlock *saveBlock = UtilBufferBlock_new(endIndex - startIndex + 1);
                            UtilBufferBlock_copyFrom(saveBlock, 0, UtilBufferBlock_getBuffer(block, startIndex),
                                                     UtilBufferBlock_getSize(saveBlock));
                            h264Parser->getOutputBufferCallback(h264Parser->h264ParserCallbackUserData,
                                                                UtilBufferBlock_getBuffer(saveBlock, 0),
                                                                UtilBufferBlock_getSize(saveBlock), &isStop);
                            UtilBufferBlock_delete(saveBlock);
                        }
                        log_debug("startIndex: 0x%x, endIndex: 0x%x", startIndex, endIndex);
                        startIndex = endIndex + 1;
                        endIndex = -1;
                    }
                    capacity = 0;
                    // log_debug("Find start code. position: 0x%x , startCodeSize: %d", position, startCodeSize);
                }
            }
            position += 1;
        }
    }

    UtilBufferBlock_delete(block);
    free(buffer);
}

H264_PARSE_API void H264Parser_setOutputBufferCallback(H264Parser *h264Parser, void *userData,
                                                       H264ParserGetOutputBufferCallback callback)
{
    assert(h264Parser);
    h264Parser->h264ParserCallbackUserData = userData;
    h264Parser->getOutputBufferCallback = callback;
}

int H264Parser_isStartCode(char buffer[4], char *outSize)
{
    char startCode[4] = {0, 0, 0, 1};
    char startCode1[3] = {0, 0, 1};
    if (memcmp(buffer, startCode, 4) == 0)
    {
        if (outSize)
        {
            *outSize = 4;
        }
        return 1;
    }
    if (memcmp(buffer + 1, startCode1, 3) == 0)
    {
        if (outSize)
        {
            *outSize = 3;
        }
        return 1;
    }
    return 0;
}
