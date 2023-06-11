#ifndef H264_ANDROID_CODEC_DEFINE_H
#define H264_ANDROID_CODEC_DEFINE_H

#ifdef _WIN32
#ifdef H264_ANDROID_CODEC_BUILD_DLL_EXPORT
#define H264_ANDROID_CODEC_API __declspec(dllexport)
#elif (defined H264_ANDROID_CODEC_DLL)
#define H264_ANDROID_CODEC_API __declspec(dllimport)
#else
#define H264_ANDROID_CODEC_API
#endif
#else
#define H264_ANDROID_CODEC_API
#endif

#endif
