#ifndef H264_PARSER_H
#define H264_PARSER_H

#include "h264_define.h"
#include "h264_nal_header.h"
#include "h264_parse_reader.h"

#ifdef __cplusplus
extern "C"
{
#endif

    typedef struct H264_PARSE_API H264NALUBlock
    {
        H264NALHeader header;
    } H264NALBlock;

    typedef void (*H264ParserGetOutputBufferCallback)(void *userData, const char *buffer, const size_t bufferSize,
                                                      int *isStop);

    typedef struct H264_PARSE_API H264Parser
    {
        H264ParseReader *reader;
        void *h264ParserCallbackUserData;
        H264ParserGetOutputBufferCallback getOutputBufferCallback;
    } H264Parser;

    H264_PARSE_API H264Parser *H264Parser_new(const char *filePath, const unsigned int maxBufferSize);
    H264_PARSE_API void H264Parser_delete(H264Parser *h264Parser);
    H264_PARSE_API void H264Parser_start(H264Parser *h264Parser);

    H264_PARSE_API void H264Parser_setOutputBufferCallback(H264Parser *h264Parser, void *userData,
                                                           H264ParserGetOutputBufferCallback callback);

    H264_PARSE_API int H264Parser_isStartCode(char buffer[4], char *outSize);

#ifdef __cplusplus
}
#endif

#endif
