#include "hv/hbase.h"
#include "hv/hloop.h"
#include "hv/hsocket.h"

#include "log.h"

typedef struct MyUserData
{
    hio_t *serverIO;
    char *path;
} MyUserData;

MyUserData *MyUserData_new(const char *path)
{
    assert(path);
    char *userdataPath = (char *)malloc(strlen(path) + 1);
    MyUserData *userdata = (MyUserData *)malloc(sizeof(MyUserData));

    if (userdataPath && userdata)
    {
        strcpy(userdataPath, path);
        userdata->path = userdataPath;
    }
    else
    {
        if (userdataPath)
        {
            free(userdataPath);
        }
        if (userdata)
        {
            free(userdata);
        }
        userdata = NULL;
    }

    return userdata;
}

void MyUserData_delete(MyUserData *userdata)
{
    assert(userdata);
    if (userdata->path)
    {
        free(userdata->path);
    }
    free(userdata);
}

static void onRead(hio_t *io, void *buf, int readbytes)
{
    log_trace("[Client] onRead fd=%d readbytes=%d\n", hio_fd(io), readbytes);
}

static void onUpstreamConnect(hio_t *upstreamIO)
{
    log_trace("[Client] onUpstreamConnect");

    MyUserData *userdata = hevent_userdata(upstreamIO);
    assert(userdata);

    FILE *fp = fopen(userdata->path, "rb");

    if (fp)
    {
        log_trace("[Client] open file.");
        int bufferSize = 1024 * 8;
        char *buffer = (char *)malloc(sizeof(char) * bufferSize);
        assert(buffer);
        while (1)
        {
            usleep(500 * 1000);
            size_t newLen = fread(buffer, sizeof(char), bufferSize, fp);
            if (feof(fp))
            {
                log_trace("[Client] end of file.");
                break;
            }
            else if (ferror(fp))
            {
                log_trace("[Client] read file error.");
                break;
            }
            else
            {
                log_trace("[Client] send buffer to server.");
                hio_write(upstreamIO, buffer, newLen);
            }
        }
        free(buffer);
    }
}

static int runClient(MyUserData *userData)
{
    hloop_t *loop = hloop_new(0);
    hio_t *upstreamIO = hio_create_socket(loop, "localhost", 9999, HIO_TYPE_SOCK_STREAM, HIO_CLIENT_SIDE);
    if (upstreamIO == NULL)
    {
        return -1;
    };
    userData->serverIO = upstreamIO;
    hevent_set_userdata(upstreamIO, userData);
    // hio_setcb_read(upstreamIO, hio_write_upstream);
    hio_setcb_read(upstreamIO, onRead);
    hio_setcb_close(upstreamIO, hio_close_upstream);
    hio_setcb_connect(upstreamIO, onUpstreamConnect);

    hio_connect(upstreamIO);
    log_trace("[Client] connect");

    hloop_run(loop);
    hloop_free(&loop);

    return 0;
}

int main(int argc, char **argv)
{
    MyUserData *userdata = MyUserData_new("./assets/output.264");
    assert(userdata);
    runClient(userdata);
    MyUserData_delete(userdata);
    return 0;
}