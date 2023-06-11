#ifndef UTIL_BUFFER_BLOCK_H
#define UTIL_BUFFER_BLOCK_H

#include "util_define.h"

#ifdef __cplusplus
extern "C"
{
#endif

    typedef struct Util_API UtilBufferBlock
    {
        char *buffer;
        unsigned int size;
    } UtilBufferBlock;

    Util_API UtilBufferBlock *UtilBufferBlock_new(const unsigned int size);

    Util_API void UtilBufferBlock_delete(UtilBufferBlock *block);

    Util_API int UtilBufferBlock_resize(UtilBufferBlock *block, const unsigned int size);

    Util_API int UtilBufferBlock_increaseSize(UtilBufferBlock *block, const unsigned int size);

    Util_API unsigned int UtilBufferBlock_getSize(UtilBufferBlock *block);

    Util_API void UtilBufferBlock_reset(UtilBufferBlock *block);

    Util_API const char *UtilBufferBlock_getBuffer(UtilBufferBlock *block, const unsigned int offset);

    Util_API unsigned int UtilBufferBlock_copyFrom(UtilBufferBlock *block, const unsigned int targetOffset,
                                                   const char *sourceBuffer, const unsigned int sourceBufferSize);

    Util_API unsigned int UtilBufferBlock_copyTo(UtilBufferBlock *block, const unsigned int sourceOffset,
                                                 const unsigned int sourceSize, const char *targetBuffer,
                                                 const unsigned int targetBufferSize);

#ifdef __cplusplus
}
#endif

#endif
