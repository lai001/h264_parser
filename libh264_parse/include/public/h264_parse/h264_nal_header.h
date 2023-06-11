#ifndef H264_NAL_HEADER_H
#define H264_NAL_HEADER_H

#include "h264_define.h"

#ifdef __cplusplus
extern "C"
{
#endif

    typedef enum H264_PARSE_API ENALUnitType
    {
        ENALUnitType_Unused = 0,
        ENALUnitType_SLICE = 1,
        ENALUnitType_DPA = 2,
        ENALUnitType_DPB = 3,
        ENALUnitType_DPC = 4,
        ENALUnitType_IDR = 5,
        ENALUnitType_SEI = 6,
        ENALUnitType_SPS = 7,
        ENALUnitType_PPS = 8,
        ENALUnitType_AUD = 9,
        ENALUnitType_EOSEQ = 10,
        ENALUnitType_EOSTREAM = 11,
        ENALUnitType_FILL = 12,
    } ENALUnitType;

    typedef enum H264_PARSE_API ENALUnitPriorityType
    {
        ENALUnitPriorityType_DISPOSABLE = 0,
        ENALUnitPriorityType_LOW = 1,
        ENALUnitPriorityType_HIGH = 2,
        ENALUnitPriorityType_HIGHEST = 3
    } ENALUnitPriorityType;

    typedef struct H264_PARSE_API H264NALHeader
    {
        unsigned char code;
        unsigned char forbidden_zero_bit;
        unsigned char nal_ref_idc;
        ENALUnitType nal_unit_type;
    } H264NALHeader;

    H264_PARSE_API void H264NALHeader_initialization(H264NALHeader *header, const unsigned char code);
    H264_PARSE_API const char *ENALUnitType_getName(ENALUnitType type);

#ifdef __cplusplus
}
#endif

#endif
