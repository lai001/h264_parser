// clang-format off
#include "h264_android_codec/decode_async.h"
#include <assert.h>
#include <string.h>
#include "util/util_buffer_block.h"
// clang-format on

H264AndroidDecodeAsyncContext *H264AndroidDecodeAsyncContext_new(H264AndroidCodecOpenOptions options, char *filePath)
{
    H264AndroidDecodeAsyncContext *context =
        (H264AndroidDecodeAsyncContext *)malloc(sizeof(H264AndroidDecodeAsyncContext));
    DecodeAsyncQueue *queue = Queue_new(NULL);
    char *contextFilePath = (char *)malloc(strlen(filePath) * sizeof(char) + 1);
    if (context && queue && contextFilePath)
    {
        sem_init(&context->bin_sem, 0, 5);
        pthread_mutex_init(&context->queueMutex, NULL);
        pthread_mutex_init(&context->isStopMutex, NULL);
        context->queue = queue;
        context->androidCodec = NULL;
        context->h264Parser = NULL;
        context->isStop = 1;
        context->options = options;
        strcpy(contextFilePath, filePath);
        context->filePath = contextFilePath;
    }
    else
    {
        if (contextFilePath)
        {
            free(contextFilePath);
        }

        if (queue)
        {
            free(queue);
        }
        if (context)
        {
            free(context);
        }
        context = NULL;
    }
    return context;
}

void H264AndroidDecodeAsyncContext_delete(H264AndroidDecodeAsyncContext *context)
{
    assert(context);
    if (context->filePath)
    {
        free(context->filePath);
    }
    H264AndroidDecodeAsyncContext_stop(context);
    pthread_mutex_lock(&context->queueMutex);
    Queue_delete(context->queue);
    pthread_mutex_unlock(&context->queueMutex);
    free(context);
}

static void getOutputBuffer(H264AndroidDecodeAsyncContext *context, char *decodedBuffer, const size_t decodedBufferSize)
{
    assert(context);
    sem_wait(&context->bin_sem);

    pthread_mutex_lock(&context->queueMutex);
    if (context->queue)
    {
        DecodeAsyncNode *node = Node_new(decodedBuffer, decodedBufferSize);
        Queue_enqueue(context->queue, node);
    }
    pthread_mutex_unlock(&context->queueMutex);
}

static void getOutputParseBuffer(H264AndroidDecodeAsyncContext *context, const char *buffer, const size_t bufferSize,
                                 int *isStop)
{
    pthread_mutex_lock(&context->isStopMutex);
    *isStop = context->isStop;
    pthread_mutex_unlock(&context->isStopMutex);
    if (*isStop)
    {
    }
    else
    {
        H264AndroidCodec_queueInputBuffer(context->androidCodec, buffer, bufferSize, 2000);
    }
}

static void H264AndroidDecodeAsyncContext_runable(H264AndroidDecodeAsyncContext *context)
{
    assert(context);
    H264AndroidCodec *androidCodec = H264AndroidCodec_new(context->options);
    H264Parser *h264Parser = H264Parser_new(context->filePath, 5 * 1024 * 1024);
    if (androidCodec && h264Parser)
    {
        context->androidCodec = androidCodec;
        context->h264Parser = h264Parser;
        H264AndroidCodec_setOutputBufferCallback(androidCodec, context, getOutputBuffer);

        H264Parser_setOutputBufferCallback(h264Parser, context, getOutputParseBuffer);

        H264AndroidCodec_start(androidCodec);
        H264Parser_start(h264Parser);

        H264AndroidCodec_stop(androidCodec);
        H264AndroidCodec_delete(androidCodec);

        H264Parser_delete(h264Parser);

        context->androidCodec = NULL;
        context->h264Parser = NULL;
    }
    else
    {
        if (androidCodec)
        {
            H264AndroidCodec_delete(androidCodec);
        }
        if (h264Parser)
        {
            H264Parser_delete(h264Parser);
        }
    }

    pthread_mutex_lock(&context->isStopMutex);
    context->isStop = 1;
    pthread_mutex_unlock(&context->isStopMutex);
}

void H264AndroidDecodeAsyncContext_start(H264AndroidDecodeAsyncContext *context)
{
    assert(context);
    H264AndroidDecodeAsyncContext_stop(context);

    // pthread_mutex_lock(&context->queueMutex);
    // Queue_delete(context->queue);
    // pthread_mutex_unlock(&context->queueMutex);

    pthread_mutex_lock(&context->isStopMutex);
    context->isStop = 0;
    pthread_mutex_unlock(&context->isStopMutex);
    pthread_create(&context->tid, NULL, H264AndroidDecodeAsyncContext_runable, context);
    pthread_detach(context->tid);
    // pthread_join(tid, NULL);
}

void H264AndroidDecodeAsyncContext_stop(H264AndroidDecodeAsyncContext *context)
{
    assert(context);
    pthread_mutex_lock(&context->isStopMutex);
    context->isStop = 1;
    pthread_mutex_unlock(&context->isStopMutex);
}

int H264AndroidDecodeAsyncContext_isStop(H264AndroidDecodeAsyncContext *context)
{
    assert(context);
    int isStop = 0;
    pthread_mutex_lock(&context->isStopMutex);
    isStop = context->isStop;
    pthread_mutex_unlock(&context->isStopMutex);
    return isStop;
}

DecodeAsyncNode *H264AndroidDecodeAsyncContext_dequeue(H264AndroidDecodeAsyncContext *context)
{
    assert(context);
    sem_post(&context->bin_sem);
    DecodeAsyncNode *node = NULL;
    pthread_mutex_lock(&context->queueMutex);
    if (context->queue)
    {
        node = Queue_dequeue(context->queue);
    }
    pthread_mutex_unlock(&context->queueMutex);
    return node;
}

void H264AndroidDecodeAsyncContext_deleteNode(H264AndroidDecodeAsyncContext *context, DecodeAsyncNode *node)
{
    assert(context);
    assert(node);
    Node_delete(node);
}

DecodeAsyncNode *Node_new(const char *buffer, const unsigned int bufferSize)
{
    assert(buffer);
    UtilBufferBlock *block = UtilBufferBlock_new(bufferSize);
    DecodeAsyncNode *node = (DecodeAsyncNode *)malloc(sizeof(DecodeAsyncNode));
    if (block && node)
    {
        node->block = block;
        node->next = NULL;
        UtilBufferBlock_copyFrom(block, 0, buffer, bufferSize);
    }
    else
    {
        if (node)
        {
            free(node);
        }
        if (block)
        {
            UtilBufferBlock_delete(block);
        }
        block = NULL;
    }
    return node;
}

DecodeAsyncNode *Node_delete(DecodeAsyncNode *node)
{
    assert(node);
    DecodeAsyncNode *next = node->next;
    if (node->block)
    {
        UtilBufferBlock_delete(node->block);
    }
    free(node);
    return next;
}

void Node_getBufferAndSize(DecodeAsyncNode *node, char **buffer, unsigned int *bufferSize)
{
    assert(node);
    if (*buffer)
    {
        *buffer = UtilBufferBlock_getBuffer(node->block, 0);
    }
    if (bufferSize)
    {
        *bufferSize = UtilBufferBlock_getSize(node->block);
    }
}

DecodeAsyncQueue *Queue_new(DecodeAsyncNode *head)
{
    DecodeAsyncQueue *queue = (DecodeAsyncQueue *)malloc(sizeof(DecodeAsyncQueue));
    if (queue)
    {
        if (head)
        {
            queue->head = head;
            queue->tail = head;
            queue->size = 1;
        }
        else
        {
            queue->head = NULL;
            queue->tail = NULL;
            queue->size = 0;
        }
    }
    else
    {
    }
    return queue;
}

void Queue_delete(DecodeAsyncQueue *queue)
{
    assert(queue);
    while (queue->head != NULL)
    {
        DecodeAsyncNode *nextNode = queue->head->next;
        Node_delete(queue->head);
        queue->head = nextNode;
    }
}

void Queue_enqueue(DecodeAsyncQueue *queue, DecodeAsyncNode *node)
{
    assert(queue);
    assert(node);

    if (queue->size < 1)
    {
        queue->head = node;
        queue->size = 1;
    }
    else
    {
        queue->tail->next = node;
        queue->size += 1;
    }
    queue->tail = node;
}

DecodeAsyncNode *Queue_dequeue(DecodeAsyncQueue *queue)
{
    assert(queue);
    if (queue->size > 0)
    {
        queue->size -= 1;
        DecodeAsyncNode *ret = queue->head;
        queue->head = ret->next;
        return ret;
    }
    else
    {
        return NULL;
    }
}