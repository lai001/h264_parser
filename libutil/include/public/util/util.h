#ifndef UTIL_H
#define UTIL_H

#include "util_define.h"

#ifndef max
#define max(a, b) (((a) > (b)) ? (a) : (b))
#endif
#ifndef min
#define min(a, b) (((a) < (b)) ? (a) : (b))
#endif

#ifdef __cplusplus
extern "C"
{
#endif

    Util_API int Util_isLittleEndian();

    Util_API int Util_tryCreateFolder(char *dir);

    Util_API int Util_saveBufferToFile(const char *buffer, const unsigned int bufferSize, const char *filePath);

    Util_API char *Util_readBufferFromFile(char *filePath, long *outFileSize);

#ifdef __cplusplus
}
#endif

#endif
