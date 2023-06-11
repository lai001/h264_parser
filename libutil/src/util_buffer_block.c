#include "util/util_buffer_block.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>

UtilBufferBlock *UtilBufferBlock_new(const unsigned int size)
{
    char *buffer = (char *)malloc(sizeof(char) * size);
    UtilBufferBlock *block = (UtilBufferBlock *)malloc(sizeof(UtilBufferBlock));

    if (buffer && block)
    {
        block->buffer = buffer;
        block->size = size;
    }
    else
    {
        if (buffer)
        {
            free(buffer);
        }
        if (block)
        {
            free(block);
        }
        block = NULL;
    }

    return block;
}

void UtilBufferBlock_delete(UtilBufferBlock *block)
{
    assert(block);
    if (block->buffer)
    {
        free(block->buffer);
    }
    free(block);
}

int UtilBufferBlock_resize(UtilBufferBlock *block, const unsigned int size)
{
    assert(block);
    if (size == block->size)
    {
        return 1;
    }
    char *newBuffer = (char *)realloc(block->buffer, size);
    if (newBuffer)
    {
        block->buffer = newBuffer;
        block->size = size;
        return 1;
    }
    else
    {
        return 0;
    }
}

int UtilBufferBlock_increaseSize(UtilBufferBlock *block, const unsigned int size)
{
    assert(block);
    if (size == 0)
    {
        return 1;
    }
    char *newBuffer = (char *)realloc(block->buffer, size + block->size);
    if (newBuffer)
    {
        block->buffer = newBuffer;
        block->size += size;
        return 1;
    }
    else
    {
        return 0;
    }
}

unsigned int UtilBufferBlock_getSize(UtilBufferBlock *block)
{
    assert(block);
    return block->size;
}

void UtilBufferBlock_reset(UtilBufferBlock *block)
{
    assert(block);
    memset(block->buffer, 0, block->size);
}

const char *UtilBufferBlock_getBuffer(UtilBufferBlock *block, const unsigned int offset)
{
    assert(block);
    return block->buffer + offset;
}

unsigned int UtilBufferBlock_copyFrom(UtilBufferBlock *block, const unsigned int targetOffset, const char *sourceBuffer,
                                      const unsigned int sourceBufferSize)
{
    assert(block);
    assert(targetOffset < block->size);
    const unsigned int actualCopy = min(block->size - targetOffset, sourceBufferSize);
    if (actualCopy > 0)
    {
        memcpy(block->buffer + targetOffset, sourceBuffer, actualCopy);
    }
    return actualCopy;
}

unsigned int UtilBufferBlock_copyTo(UtilBufferBlock *block, const unsigned int sourceOffset,
                                    const unsigned int sourceSize, const char *targetBuffer,
                                    const unsigned int targetBufferSize)
{
    assert(block);
    if (sourceSize == 0)
    {
        return 0;
    }
    const unsigned int actualCopy = min(sourceSize, targetBufferSize);
    memcpy(block->buffer + sourceOffset, targetBuffer, actualCopy);
    return actualCopy;
}
