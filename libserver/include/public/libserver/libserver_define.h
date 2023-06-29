#ifndef LIBSERVER_DEFINE_H
#define LIBSERVER_DEFINE_H

#ifdef _WIN32
#ifdef LIBSERVER_BUILD_DLL_EXPORT
#define LIBSERVER_API __declspec(dllexport)
#elif (defined LIBSERVER_DLL)
#define LIBSERVER_API __declspec(dllimport)
#else
#define LIBSERVER_API
#endif
#else
#define LIBSERVER_API
#endif

#endif