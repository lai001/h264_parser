#include "libserver/libserver.h"

typedef struct MyUserData
{
    H264AndroidCodec *codec;
    LibServer *server;
} MyUserData;

static MyUserData *MyUserData_new(H264AndroidCodec *codec, LibServer *server)
{
    MyUserData *data = (MyUserData *)malloc(sizeof(MyUserData));
    if (data)
    {
        data->codec = codec;
        data->server = server;
    }
    return data;
}

static void MyUserData_delete(MyUserData *data)
{
    assert(data);
    free(data);
}

static void MyUserData_decodedBufferCallback(void *inUserData, char *buffer, const size_t bufferSize)
{
    MyUserData *userData = (MyUserData *)inUserData;
    assert(userData);
    assert(userData->server);
    if (userData->server->getOutputBufferCallback)
    {
        userData->server->getOutputBufferCallback(userData->server->userdata, buffer, bufferSize);
    }
}

static void LibServer_onClose(hio_t *io)
{
    log_trace("[Server] onClose fd=%d error=%d\n", hio_fd(io), hio_error(io));
}

static void LibServer_onRead(hio_t *io, void *buf, int readbytes)
{
    log_trace("[Server] onRead fd=%d readbytes=%d\n", hio_fd(io), readbytes);
    MyUserData *userdata = (MyUserData *)hevent_userdata(io);
    assert(userdata);
    assert(userdata->server);
    assert(userdata->codec);

    H264AndroidCodec_queueInputBuffer(userdata->codec, buf, readbytes, 2000);
    // log_trace("[Server] < %.*s", readbytes, (char *)buf);
    //// echo
    // log_trace("> %.*s", readbytes, (char *)buf);
    // hio_write(io, buf, readbytes);
}

static void LibServer_onAccept(hio_t *io)
{
    log_trace("[Server] onAccept connfd=%d\n", hio_fd(io));
    char localaddrstr[SOCKADDR_STRLEN] = {0};
    char peeraddrstr[SOCKADDR_STRLEN] = {0};
    log_trace("[Server] accept connfd=%d [%s] <= [%s]\n", hio_fd(io), SOCKADDR_STR(hio_localaddr(io), localaddrstr),
              SOCKADDR_STR(hio_peeraddr(io), peeraddrstr));
    hio_setcb_close(io, LibServer_onClose);
    hio_setcb_read(io, LibServer_onRead);
    hio_read_start(io);
}

LibServer *LibServer_new(const char *host, const int port)
{
    LibServer *server = (LibServer *)malloc(sizeof(LibServer));
    char *serverHost = (char *)malloc(strlen(host) + 1);
    if (server && serverHost)
    {
        strcpy(serverHost, host);
        server->host = serverHost;
        server->port = port;
        server->userdata = NULL;
        server->getOutputBufferCallback = NULL;

        server->serverVolatile.loop = NULL;
        server->serverVolatile.upstreamIO = NULL;
        server->serverVolatile.ringBuffer = NULL;
        server->serverVolatile.codec = NULL;

        pthread_mutex_init(&server->serverVolatileMutex, NULL);
    }
    else
    {
        if (server)
        {
            free(server);
        }
        if (serverHost)
        {
            free(serverHost);
        }

        server = NULL;
    }
    return server;
}

void LibServer_delete(LibServer *server)
{
    assert(server);
    free(server);
}

void LibServer_start(LibServer *server)
{
    assert(server);
    H264AndroidCodecOpenOptions options;
    options.width = 1080;
    options.height = 1920;
    LibServerVolatile_initialization(&server->serverVolatile, server, server->host, server->port, options);
}

void LibServer_stop(LibServer *server)
{
    assert(server);
    pthread_mutex_lock(&server->serverVolatileMutex);
    if (server->serverVolatile.loop)
    {
        hloop_stop(server->serverVolatile.loop);
    }
    pthread_mutex_unlock(&server->serverVolatileMutex);
}

void LibServer_setOutputBufferCallback(LibServer *server, void *userdata, LibServer_GetOutputBufferCallback callback)
{
    assert(server);
    server->getOutputBufferCallback = callback;
    server->userdata = userdata;
}

void LibServerVolatile_initialization(LibServerVolatile *serverVolatile, LibServer *server, const char *host,
                                      const int port, H264AndroidCodecOpenOptions options)
{
    assert(serverVolatile);

    UtilRingBuffer *ringBuffer = UtilRingBuffer_new(1024 * 1024 * 5);
    H264AndroidCodec *codec = H264AndroidCodec_new(options);
    hloop_t *loop = hloop_new(0);
    hio_t *upstreamIO = hio_create_socket(loop, host, port, HIO_TYPE_SOCK_STREAM, HIO_SERVER_SIDE);

    pthread_mutex_lock(&server->serverVolatileMutex);
    serverVolatile->loop = loop;
    pthread_mutex_unlock(&server->serverVolatileMutex);
    serverVolatile->upstreamIO = upstreamIO;
    serverVolatile->ringBuffer = ringBuffer;
    serverVolatile->codec = codec;

    if (ringBuffer && codec && loop && upstreamIO && hio_accept(upstreamIO) == 0)
    {
        MyUserData *userdata = MyUserData_new(codec, server);
        if (userdata)
        {
            H264AndroidCodec_setOutputBufferCallback(codec, userdata, MyUserData_decodedBufferCallback);
            H264AndroidCodec_start(codec);
            hevent_set_userdata(upstreamIO, userdata);
            hio_setcb_accept(upstreamIO, LibServer_onAccept);
            hloop_run(loop);
            MyUserData_delete(userdata);
        }
    }

    LibServerVolatile_deinitialization(serverVolatile);
}

void LibServerVolatile_deinitialization(LibServerVolatile *serverVolatile)
{
    assert(serverVolatile);

    if (serverVolatile->loop)
    {
        hloop_free(serverVolatile->loop);
    }
    if (serverVolatile->ringBuffer)
    {
        UtilRingBuffer_delete(serverVolatile->ringBuffer);
    }
    if (serverVolatile->codec)
    {
        H264AndroidCodec_delete(serverVolatile->codec);
    }

    serverVolatile->loop = NULL;
    serverVolatile->upstreamIO = NULL;
    serverVolatile->ringBuffer = NULL;
    serverVolatile->codec = NULL;
}