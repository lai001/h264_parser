#include "util/util.h"
#include <assert.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/syscall.h>

int Util_isLittleEndian()
{
    int n = 1;
    if (*(char *)&n == 1)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

int Util_tryCreateFolder(char *dir)
{
    if (0 == access(dir, 0))
    {
        return 0;
    }
    else
    {
        if (0 == mkdir(dir, 0777))
        {
            return 0;
        }
        else
        {
            return 1;
        }
    }
}

int Util_saveBufferToFile(const char *buffer, const unsigned int bufferSize, const char *filePath)
{
    assert(buffer);
    assert(filePath);

    FILE *fp = fopen(filePath, "wb");
    if (fp)
    {
        size_t write = fwrite(buffer, sizeof(char), bufferSize, fp);
        int status = fclose(fp);
        return status;
    }
    return 0;
}

char *Util_readBufferFromFile(char *filePath, long *outFileSize)
{
    FILE *frameFile = fopen(filePath, "rb");
    // log_trace("filePath: %s", filePath);
    assert(frameFile);
    fseek(frameFile, 0, SEEK_END);
    long fileSize = ftell(frameFile);
    fseek(frameFile, 0, SEEK_SET);
    char *fileBuffer = (char *)malloc(sizeof(char) * fileSize);
    fread(fileBuffer, sizeof(char), fileSize, frameFile);
    fclose(frameFile);
    if (outFileSize)
    {
        *outFileSize = fileSize;
    }
    return fileBuffer;
}

/**
 * https://github.com/qemu/qemu/blob/2e3408b3cc7de4e87a9adafc8c19bfce3abec947/util/oslib-posix.c#L105
 */
int Util_getCurrentThreadID()
{
#if defined(__linux__)
    return syscall(SYS_gettid);
#elif defined(__FreeBSD__)
    long tid;
    thr_self(&tid);
    return (int)tid;
#elif defined(__NetBSD__)
    return _lwp_self();
#elif defined(__OpenBSD__)
    return getthrid();
#else
    return getpid();
#endif
}
