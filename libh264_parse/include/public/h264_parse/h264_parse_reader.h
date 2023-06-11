#ifndef H264_PARSE_READER_H
#define H264_PARSE_READER_H

#include <stdio.h>

#include "h264_parse/h264_define.h"
#include "util/ring_buffer.h"

#ifdef __cplusplus
extern "C"
{
#endif

    typedef struct H264_PARSE_API H264ParseReader
    {
        char *filePath;
        FILE *file;
        long fileSize;
        UtilRingBuffer *ringBuffer;
    } H264ParseReader;

    H264_PARSE_API H264ParseReader *H264ParseReader_new(const char *filePath, const unsigned int maxBufferSize);
    H264_PARSE_API void H264ParseReader_delete(H264ParseReader *reader);
    H264_PARSE_API unsigned int H264ParseReader_read(H264ParseReader *reader, char *buffer, const unsigned int size);
    H264_PARSE_API int H264ParseReader_isReadable(H264ParseReader *reader);
    H264_PARSE_API long H264ParseReader_getFileSize(H264ParseReader *reader);

#ifdef __cplusplus
}
#endif

#endif
