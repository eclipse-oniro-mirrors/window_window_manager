/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef OHOS_OH_NATIVE_DISPLAY_MANAGER_INNER_H
#define OHOS_OH_NATIVE_DISPLAY_MANAGER_INNER_H

#include <cstdint>
#include <map>
#include <set>

#include "dm_common.h"

namespace OHOS {
namespace Rosen {
enum class DM_DisplayStateMode : uint32_t {
    STATE_UNKNOWN = 0,
    STATE_OFF,
    STATE_ON,
    STATE_DOZE,
    STATE_DOZE_SUSPEND,
    STATE_VR,
    STATE_ON_SUSPEND
};

enum class DM_ColorSpace : uint32_t {
    UNKNOWN = 0,
    ADOBE_RGB = 1,
    BT2020_HLG = 2,
    BT2020_PQ = 3,
    BT601_EBU = 4,
    BT601_SMPTE_C = 5,
    BT709 = 6,
    P3_HLG = 7,
    P3_PQ = 8,
    DISPLAY_P3 = 9,
    SRGB = 10,
    LINEAR_SRGB = 11,
    LINEAR_P3 = 12,
    LINEAR_BT2020 = 13,
};

enum class DM_HDRFormat : uint32_t {
    NONE = 0,
    VIDEO_HLG = 1,
    VIDEO_HDR10 = 2,
    VIDEO_HDR_VIVID = 3,
    IMAGE_HDR_VIVID_DUAL = 4,
    IMAGE_HDR_VIVID_SINGLE = 5,
    IMAGE_HDR_ISO_DUAL = 6,
    IMAGE_HDR_ISO_SINGLE = 7,
};

enum class DM_GraphicCM_ColorSpaceType : uint32_t {
    GRAPHIC_CM_COLORSPACE_NONE,

    // COLORPRIMARIES_BT601_P | (TRANSFUNC_BT709 << 8) | (MATRIX_BT601_P << 16) | (RANGE_FULL << 21)
    GRAPHIC_CM_BT601_EBU_FULL = 2 | (1 << 8) | (2 << 16) | (1 << 21),

    // COLORPRIMARIES_BT601_N | (TRANSFUNC_BT709 << 8) | (MATRIX_BT601_N << 16) | (RANGE_FULL << 21)
    GRAPHIC_CM_BT601_SMPTE_C_FULL = 3 | (1 << 8) | (3 << 16) | (1 << 21),

    // COLORPRIMARIES_BT709 | (TRANSFUNC_BT709 << 8) | (MATRIX_BT709 << 16) | (RANGE_FULL << 21)
    GRAPHIC_CM_BT709_FULL = 1 | (1 << 8) | (1 << 16) | (1 << 21),

    // COLORPRIMARIES_BT2020 | (TRANSFUNC_HLG << 8) | (MATRIX_BT2020 << 16) | (RANGE_FULL << 21)
    GRAPHIC_CM_BT2020_HLG_FULL = 4 | (5 << 8) | (4 << 16) | (1 << 21),

    // COLORPRIMARIES_BT2020 | (TRANSFUNC_PQ << 8) | (MATRIX_BT2020 << 16) | (RANGE_FULL << 21)
    GRAPHIC_CM_BT2020_PQ_FULL = 4 | (4 << 8) | (4 << 16) | (1 << 21),

    // COLORPRIMARIES_BT601_P | (TRANSFUNC_BT709 << 8) | (MATRIX_BT601_P << 16) | (RANGE_LIMITED << 21)
    GRAPHIC_CM_BT601_EBU_LIMIT = 2 | (1 << 8) | (2 << 16) | (2 << 21),

    // COLORPRIMARIES_BT601_N | (TRANSFUNC_BT709 << 8) | (MATRIX_BT601_N << 16) | (RANGE_LIMITED << 21)
    GRAPHIC_CM_BT601_SMPTE_C_LIMIT = 3 | (1 << 8) | (3 << 16) | (2 << 21),

    // COLORPRIMARIES_BT709 | (TRANSFUNC_BT709 << 8) | (MATRIX_BT709 << 16) | (RANGE_LIMITED << 21)
    GRAPHIC_CM_BT709_LIMIT = 1 | (1 << 8) | (1 << 16) | (2 << 21),

    // COLORPRIMARIES_BT2020 | (TRANSFUNC_HLG << 8) | (MATRIX_BT2020 << 16) | (RANGE_LIMITED << 21)
    GRAPHIC_CM_BT2020_HLG_LIMIT = 4 | (5 << 8) | (4 << 16) | (2 << 21),

    // COLORPRIMARIES_BT2020 | (TRANSFUNC_PQ << 8) | (MATRIX_BT2020 << 16) | (RANGE_LIMITED << 21)
    GRAPHIC_CM_BT2020_PQ_LIMIT = 4 | (4 << 8) | (4 << 16) | (2 << 21),

    // COLORPRIMARIES_SRGB | (TRANSFUNC_SRGB << 8) | (MATRIX_BT601_N << 16) | (RANGE_FULL << 21)
    GRAPHIC_CM_SRGB_FULL = 1 | (2 << 8) | (3 << 16) | (1 << 21),

    // COLORPRIMARIES_P3_D65 | (TRANSFUNC_SRGB << 8) | (MATRIX_P3 << 16) | (RANGE_FULL << 21)
    GRAPHIC_CM_P3_FULL = 6 | (2 << 8) | (3 << 16) | (1 << 21),

    // COLORPRIMARIES_P3_D65 | (TRANSFUNC_HLG << 8) | (MATRIX_P3 << 16) | (RANGE_FULL << 21)
    GRAPHIC_CM_P3_HLG_FULL = 6 | (5 << 8) | (3 << 16) | (1 << 21),

    // COLORPRIMARIES_P3_D65 | (TRANSFUNC_PQ << 8) | (MATRIX_P3 << 16) | (RANGE_FULL << 21)
    GRAPHIC_CM_P3_PQ_FULL = 6 | (4 << 8) | (3 << 16) | (1 << 21),

    // COLORPRIMARIES_ADOBERGB | (TRANSFUNC_ADOBERGB << 8) | (MATRIX_ADOBERGB << 16) | (RANGE_FULL << 21)
    GRAPHIC_CM_ADOBERGB_FULL = 23 | (6 << 8) | (0 << 16) | (1 << 21),

    // COLORPRIMARIES_SRGB | (TRANSFUNC_SRGB << 8) | (MATRIX_BT601_N << 16) | (RANGE_LIMITED << 21)
    GRAPHIC_CM_SRGB_LIMIT = 1 | (2 << 8) | (3 << 16) | (2 << 21),

    // COLORPRIMARIES_P3_D65 | (TRANSFUNC_SRGB << 8) | (MATRIX_P3 << 16) | (RANGE_LIMITED << 21)
    GRAPHIC_CM_P3_LIMIT = 6 | (2 << 8) | (3 << 16) | (2 << 21),

    // COLORPRIMARIES_P3_D65 | (TRANSFUNC_HLG << 8) | (MATRIX_P3 << 16) | (RANGE_LIMITED << 21)
    GRAPHIC_CM_P3_HLG_LIMIT = 6 | (5 << 8) | (3 << 16) | (2 << 21),

    // COLORPRIMARIES_P3_D65 | (TRANSFUNC_PQ << 8) | (MATRIX_P3 << 16) | (RANGE_LIMITED << 21)
    GRAPHIC_CM_P3_PQ_LIMIT = 6 | (4 << 8) | (3 << 16) | (2 << 21),

    // COLORPRIMARIES_ADOBERGB | (TRANSFUNC_ADOBERGB << 8) | (MATRIX_ADOBERGB << 16) | (RANGE_LIMITED << 21)
    GRAPHIC_CM_ADOBERGB_LIMIT = 23 | (6 << 8) | (0 << 16) | (2 << 21),

    // COLORPRIMARIES_SRGB | (TRANSFUNC_LINEAR << 8)
    GRAPHIC_CM_LINEAR_SRGB = 1 | (3 << 8),

    // equal to GRAPHIC_CM_LINEAR_SRGB
    GRAPHIC_CM_LINEAR_BT709 = 1 | (3 << 8),

    // COLORPRIMARIES_P3_D65 | (TRANSFUNC_LINEAR << 8)
    GRAPHIC_CM_LINEAR_P3 = 6 | (3 << 8),

    // COLORPRIMARIES_BT2020 | (TRANSFUNC_LINEAR << 8)
    GRAPHIC_CM_LINEAR_BT2020 = 4 | (3 << 8),

    // equal to GRAPHIC_CM_SRGB_FULL
    GRAPHIC_CM_DISPLAY_SRGB = 1 | (2 << 8) | (3 << 16) | (1 << 21),

    // equal to GRAPHIC_CM_P3_FULL
    GRAPHIC_CM_DISPLAY_P3_SRGB = 6 | (2 << 8) | (3 << 16) | (1 << 21),

    // equal to GRAPHIC_CM_P3_HLG_FULL
    GRAPHIC_CM_DISPLAY_P3_HLG = 6 | (5 << 8) | (3 << 16) | (1 << 21),

    // equal to GRAPHIC_CM_P3_PQ_FULL
    GRAPHIC_CM_DISPLAY_P3_PQ = 6 | (4 << 8) | (3 << 16) | (1 << 21),

    // COLORPRIMARIES_BT2020 | (TRANSFUNC_SRGB << 8) | (MATRIX_BT2020 << 16) | (RANGE_FULL << 21)
    GRAPHIC_CM_DISPLAY_BT2020_SRGB = 4 | (2 << 8) | (4 << 16) | (1 << 21),

    // equal to GRAPHIC_CM_BT2020_HLG_FULL
    GRAPHIC_CM_DISPLAY_BT2020_HLG = 4 | (5 << 8) | (4 << 16) | (1 << 21),

    // equal to GRAPHIC_CM_BT2020_PQ_FULL
    GRAPHIC_CM_DISPLAY_BT2020_PQ = 4 | (4 << 8) | (4 << 16) | (1 << 21)
};

enum class DM_ScreenHDRFormat : uint32_t {
    NOT_SUPPORT_HDR = 0,
    VIDEO_HLG,
    VIDEO_HDR10,
    VIDEO_HDR_VIVID,
    IMAGE_HDR_VIVID_DUAL,
    IMAGE_HDR_VIVID_SINGLE,
    IMAGE_HDR_ISO_DUAL,
    IMAGE_HDR_ISO_SINGLE,
};

#define DISPLAY_MANAGER_FREE_MEMORY(ptr) \
    do { \
        if ((ptr)) { \
            free((ptr)); \
            (ptr) = NULL; \
        } \
    } while (0)

}  // namespace Rosen
}  // namespace OHOS
#endif  // OHOS_OH_NATIVE_DISPLAY_MANAGER_INNER_H
