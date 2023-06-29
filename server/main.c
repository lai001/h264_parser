#include "libserver/libserver.h"
#include <assert.h>

static void callback(void *userData, char *buffer, const size_t bufferSize)
{
    static int imageIndex = 0;
    imageIndex += 1;
    log_trace("[Server] imageIndex: %d", imageIndex);
}

int main(int argc, char **argv)
{
    LibServer *server = LibServer_new("localhost", 9999);
    assert(server);
    LibServer_setOutputBufferCallback(server, NULL, callback);
    LibServer_start(server);
    LibServer_delete(server);
    return 0;
}