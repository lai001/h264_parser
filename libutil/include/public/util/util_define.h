#ifndef Util_DEFINE_H
#define Util_DEFINE_H

#ifdef _WIN32
#ifdef Util_BUILD_DLL_EXPORT
#define Util_API __declspec(dllexport)
#elif (defined Util_DLL)
#define Util_API __declspec(dllimport)
#else
#define Util_API
#endif
#else
#define Util_API
#endif

#ifndef max
#define max(a, b) (((a) > (b)) ? (a) : (b))
#endif
#ifndef min
#define min(a, b) (((a) < (b)) ? (a) : (b))
#endif

#endif
