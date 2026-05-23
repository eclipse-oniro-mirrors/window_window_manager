/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#ifndef RS_BASE_SCREEN_TYPES
#define RS_BASE_SCREEN_TYPES

#include <cstdint>
#include "pixel_map.h"

namespace OHOS {
namespace Rosen {
// Logical ScreenId for both physical and virtual screen.
using ScreenId = uint64_t;

// Screen Physical Id provided by hdi-backend.
using ScreenPhysicalId = uint32_t;

constexpr ScreenId INVALID_SCREEN_ID = ~(static_cast<ScreenId>(0));

constexpr int32_t INVALID_BACKLIGHT_VALUE = -1;

constexpr uint32_t LAYER_COMPOSITION_CAPACITY = 12;

constexpr uint32_t DEFAULT_SKIP_FRAME_INTERVAL = 1;

constexpr uint32_t INVALID_EXPECTED_REFRESH_RATE = UINT32_MAX;

constexpr int32_t SCREEN_ROTATION_NUM = 4;

constexpr int32_t RS_ROTATION_0 = 0;
constexpr int32_t RS_ROTATION_90 = 90;
constexpr int32_t RS_ROTATION_180 = 180;
constexpr int32_t RS_ROTATION_270 = 270;
constexpr int32_t RS_ROTATION_360 = 360;

constexpr uint32_t MAX_BLACK_LIST_NUM = 1024;
constexpr uint32_t MAX_WHITE_LIST_NUM = 1024;

inline constexpr ScreenId ToScreenId(ScreenPhysicalId physicalId)
{
    return static_cast<ScreenId>(physicalId);
}

inline constexpr ScreenPhysicalId ToScreenPhysicalId(ScreenId id)
{
    return static_cast<ScreenPhysicalId>(id);
}

enum class ScreenEvent : uint8_t {
    CONNECTED,
    DISCONNECTED,
    UNKNOWN,
};

enum class ScreenChangeReason : uint8_t {
    DEFAULT = 0,
    HWCDEAD = 1,
    PROCESS_DISCONNECTED = 2,
};

enum class ScreenRotation : uint32_t {
    ROTATION_0 = 0,
    ROTATION_90,
    ROTATION_180,
    ROTATION_270,
    INVALID_SCREEN_ROTATION,
};

/*
 * @brief Enumerates screen constraint types, the frame to be displayed on screen at a given time.
 */
enum class ScreenConstraintType : uint32_t {
    CONSTRAINT_NONE = 0,      /**< No constraint */
    CONSTRAINT_ABSOLUTE,      /**< Absolute timestamp */
    CONSTRAINT_RELATIVE,      /**< Relative timestamp */
    CONSTRAINT_ADAPTIVE,      /**< Adaptive vsync mode */
};

enum class ScreenScaleMode : uint32_t {
    FILL_MODE = 0,
    UNISCALE_MODE,
    INVALID_MODE,
};

typedef enum : uint32_t {
    BUILT_IN_TYPE_SCREEN = 0,
    EXTERNAL_TYPE_SCREEN,
    VIRTUAL_TYPE_SCREEN,
    UNKNOWN_TYPE_SCREEN,
} RSScreenType;

typedef enum : uint32_t {
    POWER_STATUS_ON = 0,
    POWER_STATUS_STANDBY,
    POWER_STATUS_SUSPEND,
    POWER_STATUS_OFF,
    POWER_STATUS_OFF_FAKE,
    POWER_STATUS_ON_ADVANCED,
    POWER_STATUS_OFF_ADVANCED,
    POWER_STATUS_DOZE,
    POWER_STATUS_DOZE_SUSPEND,
    POWER_STATUS_BUTT,
    INVALID_POWER_STATUS,
} ScreenPowerStatus;

enum class PanelPowerStatus : uint32_t {
    PANEL_POWER_STATUS_ON = 0,
    PANEL_POWER_STATUS_OFF,
    INVALID_PANEL_POWER_STATUS,
};

typedef enum : uint32_t {
    DISP_INTF_HDMI = 0,
    DISP_INTF_LCD,
    DISP_INTF_BT1120,
    DISP_INTF_BT656,
    DISP_INTF_YPBPR,
    DISP_INTF_RGB,
    DISP_INTF_CVBS,
    DISP_INTF_SVIDEO,
    DISP_INTF_VGA,
    DISP_INTF_MIPI,
    DISP_INTF_PANEL,
    DISP_INTF_BUTT,
    DISP_INVALID,
} ScreenInterfaceType;

typedef enum : uint32_t {
    COLOR_GAMUT_INVALID = UINT32_MAX,
    COLOR_GAMUT_NATIVE = 0,
    COLOR_GAMUT_STANDARD_BT601,
    COLOR_GAMUT_STANDARD_BT709,
    COLOR_GAMUT_DCI_P3,
    COLOR_GAMUT_SRGB,
    COLOR_GAMUT_ADOBE_RGB,
    COLOR_GAMUT_DISPLAY_P3,
    COLOR_GAMUT_BT2020,
    COLOR_GAMUT_BT2100_PQ,
    COLOR_GAMUT_BT2100_HLG,
    COLOR_GAMUT_DISPLAY_BT2020,
} ScreenColorGamut;

typedef enum : uint32_t {
    GAMUT_MAP_CONSTANT = 0,
    GAMUT_MAP_EXTENSION,
    GAMUT_MAP_HDR_CONSTANT,
    GAMUT_MAP_HDR_EXTENSION,
} ScreenGamutMap;

typedef enum : uint32_t {
    NOT_SUPPORT_HDR = 0,
    VIDEO_HLG,
    VIDEO_HDR10,
    VIDEO_HDR_VIVID,
    IMAGE_HDR_VIVID_DUAL,
    IMAGE_HDR_VIVID_SINGLE,
    IMAGE_HDR_ISO_DUAL,
    IMAGE_HDR_ISO_SINGLE,
} ScreenHDRFormat;

typedef enum : uint32_t {
    MATAKEY_RED_PRIMARY_X = 0,
    MATAKEY_RED_PRIMARY_Y = 1,
    MATAKEY_GREEN_PRIMARY_X = 2,
    MATAKEY_GREEN_PRIMARY_Y = 3,
    MATAKEY_BLUE_PRIMARY_X = 4,
    MATAKEY_BLUE_PRIMARY_Y = 5,
    MATAKEY_WHITE_PRIMARY_X = 6,
    MATAKEY_WHITE_PRIMARY_Y = 7,
    MATAKEY_MAX_LUMINANCE = 8,
    MATAKEY_MIN_LUMINANCE = 9,
    MATAKEY_MAX_CONTENT_LIGHT_LEVEL = 10,
    MATAKEY_MAX_FRAME_AVERAGE_LIGHT_LEVEL = 11,
    MATAKEY_HDR10_PLUS = 12,
    MATAKEY_HDR_VIVID = 13,
} ScreenHDRMetadataKey;

/*
 * @brief Defines the screen HDR metadata.
 */
typedef struct {
    ScreenHDRMetadataKey key;
    float value;
} ScreenHDRMetaData;

typedef enum : uint32_t {
    SUCCESS = 0,
    SCREEN_NOT_FOUND,
    RS_CONNECTION_ERROR,
    SURFACE_NOT_UNIQUE,
    RENDER_SERVICE_NULL,
    INVALID_ARGUMENTS,
    WRITE_PARCEL_ERR,
    HDI_ERROR,
    SCREEN_MANAGER_NULL,
    BLACKLIST_IS_EMPTY,
    SET_RATE_ERROR,
    VIRTUAL_SCREEN,
    READ_PARCEL_ERR,
    IPC_ERROR,
    HDI_ERR_NOT_SUPPORT,
    MAIN_THREAD_NULL,
} StatusCode;

typedef enum : uint32_t {
    VIRTUAL_SCREEN_PLAY = 0,
    VIRTUAL_SCREEN_PAUSE,
    VIRTUAL_SCREEN_INVALID_STATUS,
} VirtualScreenStatus;

enum class DualScreenStatus : uint64_t {
    DUAL_SCREEN_EXIT = 0,
    DUAL_SCREEN_ENTER,
    DUAL_SCREEN_STATUS_BUTT,
};

/*
 * @brief Enumerates screen color key types supported by hardware acceleration.
 */
typedef enum {
    SCREEN_CKEY_NONE = 0,      /**< No color key */
    SCREEN_CKEY_SRC,           /**< Source color key */
    SCREEN_CKEY_DST,           /**< Destination color key */
    SCREEN_CKEY_BUTT           /**< Null operation */
} ScreenColorKey;

/*
 * @brief Enumerates screen mirror types supported by hardware acceleration.
 */
typedef enum {
    SCREEN_MIRROR_NONE = 0,      /**< No mirror */
    SCREEN_MIRROR_LR,            /**< Left and right mirrors */
    SCREEN_MIRROR_TB,            /**< Top and bottom mirrors */
    SCREEN_MIRROR_BUTT           /**< Null operation */
} ScreenMirrorType;

/*
 * @brief Enumerates the color data spaces.
 */
typedef enum {
    SCREEN_COLOR_DATA_SPACE_UNKNOWN = 0,
    SCREEN_GAMUT_BT601 = 0x00000001,
    SCREEN_GAMUT_BT709 = 0x00000002,
    SCREEN_GAMUT_DCI_P3 = 0x00000003,
    SCREEN_GAMUT_SRGB = 0x00000004,
    SCREEN_GAMUT_ADOBE_RGB = 0x00000005,
    SCREEN_GAMUT_DISPLAY_P3 = 0x00000006,
    SCREEN_GAMUT_BT2020 = 0x00000007,
    SCREEN_GAMUT_BT2100_PQ = 0x00000008,
    SCREEN_GAMUT_BT2100_HLG = 0x00000009,
    SCREEN_GAMUT_DISPLAY_BT2020 = 0x0000000a,
    SCREEN_TRANSFORM_FUNC_UNSPECIFIED = 0x00000100,
    SCREEN_TRANSFORM_FUNC_LINEAR = 0x00000200,
    SCREEN_TRANSFORM_FUNC_SRGB = 0x00000300,
    SCREEN_TRANSFORM_FUNC_SMPTE_170M = 0x00000400,
    SCREEN_TRANSFORM_FUNC_GM2_2 = 0x00000500,
    SCREEN_TRANSFORM_FUNC_GM2_6 = 0x00000600,
    SCREEN_TRANSFORM_FUNC_GM2_8 = 0x00000700,
    SCREEN_TRANSFORM_FUNC_ST2084 = 0x00000800,
    SCREEN_TRANSFORM_FUNC_HLG = 0x00000900,
    SCREEN_PRECISION_UNSPECIFIED = 0x00010000,
    SCREEN_PRECISION_FULL = 0x00020000,
    SCREEN_PRESION_LIMITED = 0x00030000,
    SCREEN_PRESION_EXTENDED = 0x00040000,
    SCREEN_BT601_SMPTE170M_FULL = SCREEN_GAMUT_BT601 | SCREEN_TRANSFORM_FUNC_SMPTE_170M | SCREEN_PRECISION_FULL,
    SCREEN_BT601_SMPTE170M_LIMITED = SCREEN_GAMUT_BT601 | SCREEN_TRANSFORM_FUNC_SMPTE_170M | SCREEN_PRESION_LIMITED,
    SCREEN_BT709_LINEAR_FULL = SCREEN_GAMUT_BT709 | SCREEN_TRANSFORM_FUNC_LINEAR | SCREEN_PRECISION_FULL,
    SCREEN_BT709_LINEAR_EXTENDED = SCREEN_GAMUT_BT709 | SCREEN_TRANSFORM_FUNC_LINEAR | SCREEN_PRESION_EXTENDED,
    SCREEN_BT709_SRGB_FULL = SCREEN_GAMUT_BT709 | SCREEN_TRANSFORM_FUNC_SRGB | SCREEN_PRECISION_FULL,
    SCREEN_BT709_SRGB_EXTENDED = SCREEN_GAMUT_BT709 | SCREEN_TRANSFORM_FUNC_SRGB | SCREEN_PRESION_EXTENDED,
    SCREEN_BT709_SMPTE170M_LIMITED = SCREEN_GAMUT_BT709 | SCREEN_TRANSFORM_FUNC_SMPTE_170M | SCREEN_PRESION_LIMITED,
    SCREEN_DCI_P3_LINEAR_FULL = SCREEN_GAMUT_DCI_P3 | SCREEN_TRANSFORM_FUNC_LINEAR | SCREEN_PRECISION_FULL,
    SCREEN_DCI_P3_GAMMA26_FULL = SCREEN_GAMUT_DCI_P3 | SCREEN_TRANSFORM_FUNC_GM2_6 | SCREEN_PRECISION_FULL,
    SCREEN_DISPLAY_P3_LINEAR_FULL = SCREEN_GAMUT_DISPLAY_P3 | SCREEN_TRANSFORM_FUNC_LINEAR | SCREEN_PRECISION_FULL,
    SCREEN_DCI_P3_SRGB_FULL = SCREEN_GAMUT_DCI_P3 | SCREEN_TRANSFORM_FUNC_SRGB | SCREEN_PRECISION_FULL,
    SCREEN_ADOBE_RGB_GAMMA22_FULL = SCREEN_GAMUT_ADOBE_RGB | SCREEN_TRANSFORM_FUNC_GM2_2 | SCREEN_PRECISION_FULL,
    SCREEN_BT2020_LINEAR_FULL = SCREEN_GAMUT_BT2020 | SCREEN_TRANSFORM_FUNC_LINEAR | SCREEN_PRECISION_FULL,
    SCREEN_BT2020_SRGB_FULL = SCREEN_GAMUT_BT2020 | SCREEN_TRANSFORM_FUNC_SRGB | SCREEN_PRECISION_FULL,
    SCREEN_BT2020_SMPTE170M_FULL = SCREEN_GAMUT_BT2020 | SCREEN_TRANSFORM_FUNC_SMPTE_170M | SCREEN_PRECISION_FULL,
    SCREEN_BT2020_ST2084_FULL = SCREEN_GAMUT_BT2020 | SCREEN_TRANSFORM_FUNC_ST2084 | SCREEN_PRECISION_FULL,
    SCREEN_BT2020_HLG_FULL = SCREEN_GAMUT_BT2020 | SCREEN_TRANSFORM_FUNC_HLG | SCREEN_PRECISION_FULL,
    SCREEN_BT2020_ST2084_LIMITED = SCREEN_GAMUT_BT2020 | SCREEN_TRANSFORM_FUNC_ST2084 | SCREEN_PRESION_LIMITED,
} ScreenColorDataSpace;

// get the underlying type of an enum value.
template<typename EnumType>
inline constexpr typename std::underlying_type<EnumType>::type ECast(EnumType t)
{
    return static_cast<typename std::underlying_type<EnumType>::type>(t);
}

inline int32_t ScreenRotationMapping(ScreenRotation screenCorrection)
{
    return screenCorrection == ScreenRotation::INVALID_SCREEN_ROTATION ?
        RS_ROTATION_0 : static_cast<int32_t>(screenCorrection) * RS_ROTATION_90;
}

typedef enum {
    SKIP_FRAME_BY_INTERVAL,
    SKIP_FRAME_BY_REFRESH_RATE,
    SKIP_FRAME_BY_ACTIVE_REFRESH_RATE,
} SkipFrameStrategy;

typedef enum : uint32_t {
    DISPLAY_CONNECTION_TYPE_INTERNAL = 0,
    DISPLAY_CONNECTION_TYPE_EXTERNAL,
    INVALID_DISPLAY_CONNECTION_TYPE,
} ScreenConnectionType;

using GraphicCM_ColorSpaceType = enum {
    GRAPHIC_CM_COLORSPACE_NONE,

    /* COLORPRIMARIES_BT601_P | (TRANSFUNC_BT709 << 8) | (MATRIX_BT601_P << 16) | (RANGE_FULL << 21) */
    GRAPHIC_CM_BT601_EBU_FULL      = 2 | (1 << 8) | (2 << 16) | (1 << 21),
    /* COLORPRIMARIES_BT601_N | (TRANSFUNC_BT709 << 8) | (MATRIX_BT601_N << 16) | (RANGE_FULL << 21) */
    GRAPHIC_CM_BT601_SMPTE_C_FULL  = 3 | (1 << 8) | (3 << 16) | (1 << 21),
    /* COLORPRIMARIES_BT709   | (TRANSFUNC_BT709 << 8) | (MATRIX_BT709   << 16) | (RANGE_FULL << 21) */
    GRAPHIC_CM_BT709_FULL          = 1 | (1 << 8) | (1 << 16) | (1 << 21),
    /* COLORPRIMARIES_BT2020  | (TRANSFUNC_HLG   << 8) | (MATRIX_BT2020  << 16) | (RANGE_FULL << 21) */
    GRAPHIC_CM_BT2020_HLG_FULL     = 4 | (5 << 8) | (4 << 16) | (1 << 21),
    /* COLORPRIMARIES_BT2020  | (TRANSFUNC_PQ    << 8) | (MATRIX_BT2020  << 16) | (RANGE_FULL << 21) */
    GRAPHIC_CM_BT2020_PQ_FULL      = 4 | (4 << 8) | (4 << 16) | (1 << 21),

    /* COLORPRIMARIES_BT601_P | (TRANSFUNC_BT709 << 8) | (MATRIX_BT601_P << 16) | (RANGE_LIMITED << 21) */
    GRAPHIC_CM_BT601_EBU_LIMIT     = 2 | (1 << 8) | (2 << 16) | (2 << 21),
    /* COLORPRIMARIES_BT601_N | (TRANSFUNC_BT709 << 8) | (MATRIX_BT601_N << 16) | (RANGE_LIMITED << 21) */
    GRAPHIC_CM_BT601_SMPTE_C_LIMIT = 3 | (1 << 8) | (3 << 16) | (2 << 21),
    /* COLORPRIMARIES_BT709   | (TRANSFUNC_BT709 << 8) | (MATRIX_BT709   << 16) | (RANGE_LIMITED << 21) */
    GRAPHIC_CM_BT709_LIMIT         = 1 | (1 << 8) | (1 << 16) | (2 << 21),
    /* COLORPRIMARIES_BT2020  | (TRANSFUNC_HLG   << 8) | (MATRIX_BT2020  << 16) | (RANGE_LIMITED << 21) */
    GRAPHIC_CM_BT2020_HLG_LIMIT    = 4 | (5 << 8) | (4 << 16) | (2 << 21),
    /* COLORPRIMARIES_BT2020  | (TRANSFUNC_PQ    << 8) | (MATRIX_BT2020  << 16) | (RANGE_LIMITED << 21) */
    GRAPHIC_CM_BT2020_PQ_LIMIT     = 4 | (4 << 8) | (4 << 16) | (2 << 21),

    /* COLORPRIMARIES_SRGB     | (TRANSFUNC_SRGB     << 8) | (MATRIX_BT601_N  << 16) | (RANGE_FULL << 21) */
    GRAPHIC_CM_SRGB_FULL           = 1 | (2 << 8) | (3 << 16) | (1 << 21),
    /* COLORPRIMARIES_P3_D65   | (TRANSFUNC_SRGB     << 8) | (MATRIX_P3       << 16) | (RANGE_FULL << 21) */
    GRAPHIC_CM_P3_FULL             = 6 | (2 << 8) | (3 << 16) | (1 << 21),
    /* COLORPRIMARIES_P3_D65   | (TRANSFUNC_HLG      << 8) | (MATRIX_P3       << 16) | (RANGE_FULL << 21) */
    GRAPHIC_CM_P3_HLG_FULL         = 6 | (5 << 8) | (3 << 16) | (1 << 21),
    /* COLORPRIMARIES_P3_D65   | (TRANSFUNC_PQ       << 8) | (MATRIX_P3       << 16) | (RANGE_FULL << 21) */
    GRAPHIC_CM_P3_PQ_FULL          = 6 | (4 << 8) | (3 << 16) | (1 << 21),
    /* COLORPRIMARIES_ADOBERGB | (TRANSFUNC_ADOBERGB << 8) | (MATRIX_ADOBERGB << 16) | (RANGE_FULL << 21) */
    GRAPHIC_CM_ADOBERGB_FULL       = 23 | (6 << 8) | (0 << 16) | (1 << 21),

    /* COLORPRIMARIES_SRGB     | (TRANSFUNC_SRGB     << 8) | (MATRIX_BT601_N  << 16) | (RANGE_LIMITED << 21) */
    GRAPHIC_CM_SRGB_LIMIT          = 1 | (2 << 8) | (3 << 16) | (2 << 21),
    /* COLORPRIMARIES_P3_D65   | (TRANSFUNC_SRGB     << 8) | (MATRIX_P3       << 16) | (RANGE_LIMITED << 21) */
    GRAPHIC_CM_P3_LIMIT            = 6 | (2 << 8) | (3 << 16) | (2 << 21),
    /* COLORPRIMARIES_P3_D65   | (TRANSFUNC_HLG      << 8) | (MATRIX_P3       << 16) | (RANGE_LIMITED << 21) */
    GRAPHIC_CM_P3_HLG_LIMIT        = 6 | (5 << 8) | (3 << 16) | (2 << 21),
    /* COLORPRIMARIES_P3_D65   | (TRANSFUNC_PQ       << 8) | (MATRIX_P3       << 16) | (RANGE_LIMITED << 21) */
    GRAPHIC_CM_P3_PQ_LIMIT         = 6 | (4 << 8) | (3 << 16) | (2 << 21),
    /* COLORPRIMARIES_ADOBERGB | (TRANSFUNC_ADOBERGB << 8) | (MATRIX_ADOBERGB << 16) | (RANGE_LIMITED << 21) */
    GRAPHIC_CM_ADOBERGB_LIMIT      = 23 | (6 << 8) | (0 << 16) | (2 << 21),

    /* COLORPRIMARIES_SRGB   | (TRANSFUNC_LINEAR << 8) */
    GRAPHIC_CM_LINEAR_SRGB         = 1 | (3 << 8),
    /* equal to GRAPHIC_CM_LINEAR_SRGB */
    GRAPHIC_CM_LINEAR_BT709        = 1 | (3 << 8),
    /* COLORPRIMARIES_P3_D65 | (TRANSFUNC_LINEAR << 8) */
    GRAPHIC_CM_LINEAR_P3           = 6 | (3 << 8),
    /* COLORPRIMARIES_BT2020 | (TRANSFUNC_LINEAR << 8) */
    GRAPHIC_CM_LINEAR_BT2020       = 4 | (3 << 8),

    /* equal to GRAPHIC_CM_SRGB_FULL */
    GRAPHIC_CM_DISPLAY_SRGB        = 1 | (2 << 8) | (3 << 16) | (1 << 21),
    /* equal to GRAPHIC_CM_P3_FULL */
    GRAPHIC_CM_DISPLAY_P3_SRGB     = 6 | (2 << 8) | (3 << 16) | (1 << 21),
    /* equal to GRAPHIC_CM_P3_HLG_FULL */
    GRAPHIC_CM_DISPLAY_P3_HLG      = 6 | (5 << 8) | (3 << 16) | (1 << 21),
    /* equal to GRAPHIC_CM_P3_PQ_FULL */
    GRAPHIC_CM_DISPLAY_P3_PQ       = 6 | (4 << 8) | (3 << 16) | (1 << 21),
    /* COLORPRIMARIES_BT2020   | (TRANSFUNC_SRGB << 8)     | (MATRIX_BT2020 << 16)   | (RANGE_FULL << 21) */
    GRAPHIC_CM_DISPLAY_BT2020_SRGB = 4 | (2 << 8) | (4 << 16) | (1 << 21),
    /* equal to GRAPHIC_CM_BT2020_HLG_FULL */
    GRAPHIC_CM_DISPLAY_BT2020_HLG  = 4 | (5 << 8) | (4 << 16) | (1 << 21),
    /* equal to GRAPHIC_CM_BT2020_PQ_FULL */
    GRAPHIC_CM_DISPLAY_BT2020_PQ   = 4 | (4 << 8) | (4 << 16) | (1 << 21)
};

using GraphicPixelFormat = enum {
    GRAPHIC_PIXEL_FMT_CLUT8 = 0,                    /**< CLUT8 format */
    GRAPHIC_PIXEL_FMT_CLUT1 = 1,                    /**< CLUT1 format */
    GRAPHIC_PIXEL_FMT_CLUT4 = 2,                    /**< CLUT4 format */
    GRAPHIC_PIXEL_FMT_RGB_565 = 3,                  /**< RGB565 format */
    GRAPHIC_PIXEL_FMT_RGBA_5658 = 4,                /**< RGBA5658 format */
    GRAPHIC_PIXEL_FMT_RGBX_4444 = 5,                /**< RGBX4444 format */
    GRAPHIC_PIXEL_FMT_RGBA_4444 = 6,                /**< RGBA4444 format */
    GRAPHIC_PIXEL_FMT_RGB_444 = 7,                  /**< RGB444 format */
    GRAPHIC_PIXEL_FMT_RGBX_5551 = 8,                /**< RGBX5551 format */
    GRAPHIC_PIXEL_FMT_RGBA_5551 = 9,                /**< RGBA5551 format */
    GRAPHIC_PIXEL_FMT_RGB_555 = 10,                 /**< RGB555 format */
    GRAPHIC_PIXEL_FMT_RGBX_8888 = 11,               /**< RGBX8888 format */
    GRAPHIC_PIXEL_FMT_RGBA_8888 = 12,               /**< RGBA8888 format */
    GRAPHIC_PIXEL_FMT_RGB_888 = 13,                 /**< RGB888 format */
    GRAPHIC_PIXEL_FMT_BGR_565 = 14,                 /**< BGR565 format */
    GRAPHIC_PIXEL_FMT_BGRX_4444 = 15,               /**< BGRX4444 format */
    GRAPHIC_PIXEL_FMT_BGRA_4444 = 16,               /**< BGRA4444 format */
    GRAPHIC_PIXEL_FMT_BGRX_5551 = 17,               /**< BGRX5551 format */
    GRAPHIC_PIXEL_FMT_BGRA_5551 = 18,               /**< BGRA5551 format */
    GRAPHIC_PIXEL_FMT_BGRX_8888 = 19,               /**< BGRX8888 format */
    GRAPHIC_PIXEL_FMT_BGRA_8888 = 20,               /**< BGRA8888 format */
    GRAPHIC_PIXEL_FMT_YUV_422_I = 21,               /**< YUV422 interleaved format */
    GRAPHIC_PIXEL_FMT_YCBCR_422_SP = 22,            /**< YCBCR422 semi-planar format */
    GRAPHIC_PIXEL_FMT_YCRCB_422_SP = 23,            /**< YCRCB422 semi-planar format */
    GRAPHIC_PIXEL_FMT_YCBCR_420_SP = 24,            /**< YCBCR420 semi-planar format */
    GRAPHIC_PIXEL_FMT_YCRCB_420_SP = 25,            /**< YCRCB420 semi-planar format */
    GRAPHIC_PIXEL_FMT_YCBCR_422_P = 26,             /**< YCBCR422 planar format */
    GRAPHIC_PIXEL_FMT_YCRCB_422_P = 27,             /**< YCRCB422 planar format */
    GRAPHIC_PIXEL_FMT_YCBCR_420_P = 28,             /**< YCBCR420 planar format */
    GRAPHIC_PIXEL_FMT_YCRCB_420_P = 29,             /**< YCRCB420 planar format */
    GRAPHIC_PIXEL_FMT_YUYV_422_PKG = 30,            /**< YUYV422 packed format */
    GRAPHIC_PIXEL_FMT_UYVY_422_PKG = 31,            /**< UYVY422 packed format */
    GRAPHIC_PIXEL_FMT_YVYU_422_PKG = 32,            /**< YVYU422 packed format */
    GRAPHIC_PIXEL_FMT_VYUY_422_PKG = 33,            /**< VYUY422 packed format */
    GRAPHIC_PIXEL_FMT_RGBA_1010102 = 34,            /**< RGBA_1010102 packed format */
    GRAPHIC_PIXEL_FMT_YCBCR_P010 = 35,              /**< YCBCR420 semi-planar 10bit packed format */
    GRAPHIC_PIXEL_FMT_YCRCB_P010 = 36,              /**< YCRCB420 semi-planar 10bit packed format */
    GRAPHIC_PIXEL_FMT_RAW10 = 37,                   /**< Raw 10bit packed format */
    GRAPHIC_PIXEL_FMT_BLOB = 38,                    /**< BLOB format */
    GRAPHIC_PIXEL_FMT_RGBA16_FLOAT = 39,            /**< RGBA16 float format */
    GRAPHIC_PIXEL_FMT_Y8 = 40,                      /**< Y8 format */
    GRAPHIC_PIXEL_FMT_Y16 = 41,                     /**< Y16 format */
    GRAPHIC_PIXEL_FMT_RGBA_R16G16 = 42,
    GRAPHIC_PIXEL_FMT_RGBA_1010108 = 43,
    GRAPHIC_PIXEL_FMT_VENDER_MASK = 0X7FFF0000,     /**< vendor mask format */
    GRAPHIC_PIXEL_FMT_BUTT = 0X7FFFFFFF             /**< Invalid pixel format */
};

enum HdrStatus : uint32_t {
    NO_HDR = 0x0000,
    HDR_PHOTO = 0x0001,
    HDR_VIDEO = 0x0010,
    AI_HDR_VIDEO_GTM = 0x0100,
    HDR_EFFECT = 0x1000,
    AI_HDR_VIDEO_GAINMAP = 0x10000,
    HDR_UICOMPONENT = 0x100000,
};
} // namespace Rosen
} // namespace OHOS

#endif // RS_BASE_SCREEN_TYPES
