#ifndef RING_BUFFER_H
#define RING_BUFFER_H

#include "util_define.h"

#ifdef __cplusplus
extern "C"
{
#endif

    typedef struct Util_API UtilRingBuffer
    {
        unsigned int writePosition;
        unsigned int readPosition;
        unsigned int avaliableToRead;
        unsigned int size;
        char *buffer;
    } UtilRingBuffer;

    Util_API UtilRingBuffer *UtilRingBuffer_new(const unsigned int size);
    Util_API void UtilRingBuffer_delete(UtilRingBuffer *ringBuffer);
    Util_API unsigned int UtilRingBuffer_read(UtilRingBuffer *ringBuffer, char *buffer, const unsigned int size);
    Util_API void UtilRingBuffer_write(UtilRingBuffer *ringBuffer, const char *buffer, const unsigned int size);
    Util_API unsigned int UtilRingBuffer_avaliableToRead(const UtilRingBuffer *ringBuffer);
    Util_API unsigned int UtilRingBuffer_avaliableToWrite(const UtilRingBuffer *ringBuffer);
    Util_API void UtilRingBuffer_reset(UtilRingBuffer *ringBuffer);

#ifdef __cplusplus
}
#endif

#endif
