#ifndef H264_DEFINE_H
#define H264_DEFINE_H

#ifdef _WIN32
#ifdef H264_PARSE_BUILD_DLL_EXPORT
#define H264_PARSE_API __declspec(dllexport)
#elif (defined H264_PARSE_DLL)
#define H264_PARSE_API __declspec(dllimport)
#else
#define H264_PARSE_API
#endif
#else
#define H264_PARSE_API
#endif

#endif
