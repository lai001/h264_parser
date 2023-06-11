#include "h264_parse/h264_parse_reader.h"
#include "log.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>

static void h264ParseReaderFill(H264ParseReader *reader, const unsigned int bufferSize)
{
    assert(reader);
    char *buffer = (char *)malloc(sizeof(char) * bufferSize);

    if (buffer)
    {
        size_t newLen = fread(buffer, sizeof(char), bufferSize, reader->file);

        if (feof(reader->file))
        {
            const long filePosition = ftell(reader->file);
            UtilRingBuffer_write(reader->ringBuffer, buffer, newLen);
        }
        else if (ferror(reader->file))
        {
        }
        else
        {
            UtilRingBuffer_write(reader->ringBuffer, buffer, newLen);
        }
        free(buffer);
    }
}

H264ParseReader *H264ParseReader_new(const char *filePath, const unsigned int maxBufferSize)
{
    assert(filePath);
    FILE *readerFile = fopen(filePath, "rb");
    char *readerFilePath = (char *)malloc((strlen(filePath) + 1) * sizeof(char));
    UtilRingBuffer *ringBuffer = UtilRingBuffer_new(maxBufferSize);
    H264ParseReader *reader = (H264ParseReader *)malloc(sizeof(H264ParseReader));
    long readerFileSize = -1;

    if (readerFile)
    {
        if (fseek(readerFile, 0, SEEK_END) == 0)
        {
            readerFileSize = ftell(readerFile);
            if (fseek(readerFile, 0, SEEK_SET) != 0)
            {
                readerFileSize = -1;
            }
        }
    }

    if (readerFile && readerFilePath && reader && ringBuffer && readerFileSize > 0)
    {
        strcpy(readerFilePath, filePath);
        reader->fileSize = readerFileSize;
        reader->filePath = readerFilePath;
        reader->file = readerFile;
        reader->ringBuffer = ringBuffer;
    }
    else
    {
        if (readerFile)
        {
            fclose(readerFile);
        }
        if (readerFilePath)
        {
            free(readerFilePath);
        }
        if (ringBuffer)
        {
            UtilRingBuffer_delete(ringBuffer);
        }
        if (reader)
        {
            free(reader);
        }
        reader = NULL;
    }

    return reader;
}

void H264ParseReader_delete(H264ParseReader *reader)
{
    assert(reader);
    if (reader->file)
    {
        fclose(reader->file);
    }
    if (reader->filePath)
    {
        free(reader->filePath);
    }
    if (reader->ringBuffer)
    {
        UtilRingBuffer_delete(reader->ringBuffer);
    }
    free(reader);
}

unsigned int H264ParseReader_read(H264ParseReader *reader, char *buffer, const unsigned int size)
{
    assert(reader);
    h264ParseReaderFill(reader, size);
    return UtilRingBuffer_read(reader->ringBuffer, buffer, size);
}

H264_PARSE_API int H264ParseReader_isReadable(H264ParseReader *reader)
{
    assert(reader);

    if (ftell(reader->file) < reader->fileSize)
    {
        return 1;
    }
    else
    {
        if (UtilRingBuffer_avaliableToRead(reader->ringBuffer) > 0)
        {
            return 1;
        }
        else
        {
            return 0;
        }
    }
}

H264_PARSE_API long H264ParseReader_getFileSize(H264ParseReader *reader)
{
    assert(reader);
    return reader->fileSize;
}
