#include "util/ring_buffer.h"
#include "log.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>

UtilRingBuffer *UtilRingBuffer_new(const unsigned int size)
{
    char *buffer = (char *)malloc(sizeof(char) * size);
    UtilRingBuffer *ringBuffer = (UtilRingBuffer *)malloc(sizeof(UtilRingBuffer));

    if (buffer && ringBuffer)
    {
        ringBuffer->size = size;
        ringBuffer->buffer = buffer;
        ringBuffer->readPosition = 0;
        ringBuffer->writePosition = 0;
        ringBuffer->avaliableToRead = 0;
        return ringBuffer;
    }
    else
    {
        if (buffer)
        {
            free(buffer);
        }
        if (ringBuffer)
        {
            free(ringBuffer);
        }
        ringBuffer = NULL;
    }

    return ringBuffer;
}

void UtilRingBuffer_delete(UtilRingBuffer *ringBuffer)
{
    assert(ringBuffer);
    if (ringBuffer->buffer)
    {
        free(ringBuffer->buffer);
    }
    free(ringBuffer);
}

unsigned int UtilRingBuffer_read(UtilRingBuffer *ringBuffer, char *buffer, const unsigned int size)
{
    assert(ringBuffer);
    assert(buffer);

    unsigned int avaliableToRead = min(UtilRingBuffer_avaliableToRead(ringBuffer), size);
    unsigned int actualRead = 0;

    while (avaliableToRead > 0)
    {
        unsigned int readSize;
        if (ringBuffer->readPosition >= ringBuffer->writePosition)
        {
            readSize = min(ringBuffer->size - ringBuffer->readPosition, avaliableToRead);
        }
        else
        {
            readSize = min(ringBuffer->writePosition - ringBuffer->readPosition, avaliableToRead);
        }
        memcpy(buffer, ringBuffer->buffer + ringBuffer->readPosition, readSize);
        avaliableToRead -= readSize;
        buffer += readSize;
        actualRead += readSize;
        ringBuffer->readPosition += readSize;
        ringBuffer->readPosition %= ringBuffer->size;
        ringBuffer->avaliableToRead -= readSize;
    }
    return actualRead;
}

void UtilRingBuffer_write(UtilRingBuffer *ringBuffer, const char *buffer, const unsigned int size)
{
    assert(ringBuffer);
    assert(buffer);
    assert(size <= ringBuffer->size);

    char *bufferRead = buffer;
    unsigned int pendingSize = size;

    while (pendingSize > 0)
    {
        const unsigned int writeSize = min(pendingSize, ringBuffer->size - ringBuffer->writePosition);
        memcpy(ringBuffer->buffer + ringBuffer->writePosition, bufferRead, writeSize);
        ringBuffer->writePosition += writeSize;
        ringBuffer->writePosition %= ringBuffer->size;
        bufferRead += writeSize;
        pendingSize -= writeSize;
    }

    ringBuffer->avaliableToRead += size;
    ringBuffer->avaliableToRead = min(ringBuffer->avaliableToRead, ringBuffer->size);
}

unsigned int UtilRingBuffer_avaliableToRead(const UtilRingBuffer *ringBuffer)
{
    assert(ringBuffer);
    return ringBuffer->avaliableToRead;
}

unsigned int UtilRingBuffer_avaliableToWrite(const UtilRingBuffer *ringBuffer)
{
    assert(ringBuffer);
    return ringBuffer->size - ringBuffer->avaliableToRead;
}

void UtilRingBuffer_reset(UtilRingBuffer *ringBuffer)
{
    assert(ringBuffer);
    ringBuffer->readPosition = 0;
    ringBuffer->writePosition = 0;
    ringBuffer->avaliableToRead = 0;
}
