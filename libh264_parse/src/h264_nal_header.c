#include "h264_parse/h264_nal_header.h"
#include <assert.h>

void H264NALHeader_initialization(H264NALHeader *header, const unsigned char code)
{
    assert(header);
    header->code = code;
    header->forbidden_zero_bit = (code >> 7) & 0b00000001;
    header->nal_ref_idc = (code >> 5) & 0b00000011;
    header->nal_unit_type = code & 0b00011111;
}

const char *ENALUnitType_getName(ENALUnitType type)
{
    static const char *Unused = "Unused";
    static const char *SLICE = "SLICE";
    static const char *DPA = "DPA";
    static const char *DPB = "DPB";
    static const char *DPC = "DPC";
    static const char *IDR = "IDR";
    static const char *SEI = "SEI";
    static const char *SPS = "SPS";
    static const char *PPS = "PPS";
    static const char *AUD = "AUD";
    static const char *EOSEQ = "EOSEQ";
    static const char *EOSTREAM = "EOSTREAM";
    static const char *FILL = "FILL";

    switch (type)
    {
    case ENALUnitType_Unused:
        return Unused;
        break;

    case ENALUnitType_SLICE:
        return SLICE;
        break;

    case ENALUnitType_DPA:
        return DPA;
        break;

    case ENALUnitType_DPB:
        return DPB;
        break;

    case ENALUnitType_DPC:
        return DPC;
        break;

    case ENALUnitType_IDR:
        return IDR;
        break;

    case ENALUnitType_SEI:
        return SEI;
        break;

    case ENALUnitType_SPS:
        return SPS;
        break;

    case ENALUnitType_PPS:
        return PPS;
        break;

    case ENALUnitType_AUD:
        return AUD;
        break;

    case ENALUnitType_EOSEQ:
        return EOSEQ;
        break;

    case ENALUnitType_EOSTREAM:
        return EOSTREAM;
        break;

    case ENALUnitType_FILL:
        return FILL;
        break;

    default:
        assert(0);
        break;
    }
}
