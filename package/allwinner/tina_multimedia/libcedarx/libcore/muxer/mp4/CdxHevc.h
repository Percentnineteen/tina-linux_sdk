#ifndef CDX_HEVC_H
#define CDX_HEVC_H

#include <stdint.h>
#include "ByteIOContext.h"
#define AW_INPUT_BUFFER_PADDING_SIZE 32

enum HEVCNALUnitType {
    HEVC_NAL_TYPE_TRAIL_N    = 0,
    HEVC_NAL_TYPE_TRAIL_R    = 1,
    HEVC_NAL_TYPE_TSA_N      = 2,
    HEVC_NAL_TYPE_TSA_R      = 3,
    HEVC_NAL_TYPE_STSA_N     = 4,
    HEVC_NAL_TYPE_STSA_R     = 5,
    HEVC_NAL_TYPE_RADL_N     = 6,
    HEVC_NAL_TYPE_RADL_R     = 7,
    HEVC_NAL_TYPE_RASL_N     = 8,
    HEVC_NAL_TYPE_RASL_R     = 9,
    HEVC_NAL_TYPE_BLA_W_LP   = 16,
    HEVC_NAL_TYPE_BLA_W_RADL = 17,
    HEVC_NAL_TYPE_BLA_N_LP   = 18,
    HEVC_NAL_TYPE_IDR_W_RADL = 19,
    HEVC_NAL_TYPE_IDR_N_LP   = 20,
    HEVC_NAL_TYPE_CRA_NUT    = 21,
    HEVC_NAL_TYPE_VPS        = 32,
    HEVC_NAL_TYPE_SPS        = 33,
    HEVC_NAL_TYPE_PPS        = 34,
    HEVC_NAL_TYPE_AUD        = 35,
    HEVC_NAL_TYPE_EOS_NUT    = 36,
    HEVC_NAL_TYPE_EOB_NUT    = 37,
    HEVC_NAL_TYPE_FD_NUT     = 38,
    HEVC_NAL_TYPE_SEI_PREFIX = 39,
    HEVC_NAL_TYPE_SEI_SUFFIX = 40,
};

enum HEVCSliceType {
    HEVC_SLICE_TYPE_B = 0,
    HEVC_SLICE_TYPE_P = 1,
    HEVC_SLICE_TYPE_I = 2,
};

/**
 * 7.4.2.1
 */
#define HEVC_MAX_NUM_SUB_LAYERS 7
#define HEVC_MAX_NUM_VPS_COUNT 16
#define HEVC_MAX_NUM_SPS_COUNT 32
#define HEVC_MAX_NUM_PPS_COUNT 256
#define HEVC_MAX_NUM_SHORT_TERM_RPS_COUNT 64
#define HEVC_MAX_NUM_CU_SIZE 128

#define HEVC_MAX_NUM_REFS 16
#define HEVC_MAX_NUM_DPB_SIZE 16 // A.4.1

#define HEVC_MAX_NUM_LOG2_CTB_SIZE 6

cdx_int32 hvccWriteSpsPps(ByteIOContext *pb, cdx_uint8 *data,
                       cdx_int32 size, cdx_int32 PsArrayCompleteness);

cdx_uint32 hvccGetSpsPpsSize(cdx_uint8 *data, cdx_int32 size, cdx_int32 PsArrayCompleteness);

#endif /* CDX_HEVC_H */