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

#include <securec.h>

#include <shared_mutex>

#include "display.h"
#include "display_info.h"
#include "display_manager.h"
#include "oh_display_capture.h"
#include "oh_display_manager.h"
#include "oh_display_manager_inner.h"
#include "pixelmap_native_impl.h"
#include "window_manager_hilog.h"

using namespace OHOS;
using namespace Rosen;

namespace OHOS {
namespace {
const std::map<DisplayState, DM_DisplayStateMode> DM_NATIVE_TO_NDK_DISPLAY_STATE_MAP {
    { DisplayState::UNKNOWN,      DM_DisplayStateMode::STATE_UNKNOWN      },
    { DisplayState::OFF,          DM_DisplayStateMode::STATE_OFF          },
    { DisplayState::ON,           DM_DisplayStateMode::STATE_ON           },
    { DisplayState::DOZE,         DM_DisplayStateMode::STATE_DOZE         },
    { DisplayState::DOZE_SUSPEND, DM_DisplayStateMode::STATE_DOZE_SUSPEND },
    { DisplayState::VR,           DM_DisplayStateMode::STATE_VR           },
    { DisplayState::ON_SUSPEND,   DM_DisplayStateMode::STATE_ON_SUSPEND   },
};

const std::map<DM_GraphicCM_ColorSpaceType, DM_ColorSpace> DM_NATIVE_TO_NDK_COLOR_SPACE_TYPE_MAP {
    { DM_GraphicCM_ColorSpaceType::GRAPHIC_CM_COLORSPACE_NONE,     DM_ColorSpace::UNKNOWN },
    { DM_GraphicCM_ColorSpaceType::GRAPHIC_CM_ADOBERGB_FULL,       DM_ColorSpace::ADOBE_RGB },
    { DM_GraphicCM_ColorSpaceType::GRAPHIC_CM_ADOBERGB_LIMIT,      DM_ColorSpace::ADOBE_RGB },
    { DM_GraphicCM_ColorSpaceType::GRAPHIC_CM_BT2020_HLG_FULL,     DM_ColorSpace::BT2020_HLG },
    { DM_GraphicCM_ColorSpaceType::GRAPHIC_CM_BT2020_HLG_LIMIT,    DM_ColorSpace::BT2020_HLG },
    { DM_GraphicCM_ColorSpaceType::GRAPHIC_CM_DISPLAY_BT2020_HLG,  DM_ColorSpace::BT2020_HLG },
    { DM_GraphicCM_ColorSpaceType::GRAPHIC_CM_BT2020_PQ_FULL,      DM_ColorSpace::BT2020_PQ },
    { DM_GraphicCM_ColorSpaceType::GRAPHIC_CM_BT2020_PQ_LIMIT,     DM_ColorSpace::BT2020_PQ },
    { DM_GraphicCM_ColorSpaceType::GRAPHIC_CM_DISPLAY_BT2020_PQ,   DM_ColorSpace::BT2020_PQ },
    { DM_GraphicCM_ColorSpaceType::GRAPHIC_CM_BT601_EBU_FULL,      DM_ColorSpace::BT601_EBU },
    { DM_GraphicCM_ColorSpaceType::GRAPHIC_CM_BT601_EBU_LIMIT,     DM_ColorSpace::BT601_EBU },
    { DM_GraphicCM_ColorSpaceType::GRAPHIC_CM_BT601_SMPTE_C_FULL,  DM_ColorSpace::BT601_SMPTE_C },
    { DM_GraphicCM_ColorSpaceType::GRAPHIC_CM_BT601_SMPTE_C_LIMIT, DM_ColorSpace::BT601_SMPTE_C },
    { DM_GraphicCM_ColorSpaceType::GRAPHIC_CM_BT709_FULL,          DM_ColorSpace::BT709 },
    { DM_GraphicCM_ColorSpaceType::GRAPHIC_CM_BT709_LIMIT,         DM_ColorSpace::BT709 },
    { DM_GraphicCM_ColorSpaceType::GRAPHIC_CM_P3_HLG_FULL,         DM_ColorSpace::P3_HLG },
    { DM_GraphicCM_ColorSpaceType::GRAPHIC_CM_P3_HLG_LIMIT,        DM_ColorSpace::P3_HLG },
    { DM_GraphicCM_ColorSpaceType::GRAPHIC_CM_DISPLAY_P3_HLG,      DM_ColorSpace::P3_HLG },
    { DM_GraphicCM_ColorSpaceType::GRAPHIC_CM_P3_PQ_FULL,          DM_ColorSpace::P3_PQ },
    { DM_GraphicCM_ColorSpaceType::GRAPHIC_CM_P3_PQ_LIMIT,         DM_ColorSpace::P3_PQ },
    { DM_GraphicCM_ColorSpaceType::GRAPHIC_CM_DISPLAY_P3_PQ,       DM_ColorSpace::P3_PQ },
    { DM_GraphicCM_ColorSpaceType::GRAPHIC_CM_P3_FULL,             DM_ColorSpace::DISPLAY_P3 },
    { DM_GraphicCM_ColorSpaceType::GRAPHIC_CM_P3_LIMIT,            DM_ColorSpace::DISPLAY_P3 },
    { DM_GraphicCM_ColorSpaceType::GRAPHIC_CM_DISPLAY_P3_SRGB,     DM_ColorSpace::DISPLAY_P3 },
    { DM_GraphicCM_ColorSpaceType::GRAPHIC_CM_SRGB_FULL,           DM_ColorSpace::SRGB },
    { DM_GraphicCM_ColorSpaceType::GRAPHIC_CM_SRGB_LIMIT,          DM_ColorSpace::SRGB },
    { DM_GraphicCM_ColorSpaceType::GRAPHIC_CM_DISPLAY_SRGB,        DM_ColorSpace::SRGB },
    { DM_GraphicCM_ColorSpaceType::GRAPHIC_CM_LINEAR_SRGB,         DM_ColorSpace::LINEAR_SRGB },
    { DM_GraphicCM_ColorSpaceType::GRAPHIC_CM_LINEAR_BT709,        DM_ColorSpace::LINEAR_SRGB },
    { DM_GraphicCM_ColorSpaceType::GRAPHIC_CM_LINEAR_P3,           DM_ColorSpace::LINEAR_P3 },
    { DM_GraphicCM_ColorSpaceType::GRAPHIC_CM_LINEAR_BT2020,       DM_ColorSpace::LINEAR_BT2020 },
};

const std::map<DM_ScreenHDRFormat, DM_HDRFormat> DM_NATIVE_TO_NDK_HDR_FORMAT_TYPE_MAP {
    { DM_ScreenHDRFormat::NOT_SUPPORT_HDR,        DM_HDRFormat::NONE },
    { DM_ScreenHDRFormat::VIDEO_HLG,              DM_HDRFormat::VIDEO_HLG },
    { DM_ScreenHDRFormat::VIDEO_HDR10,            DM_HDRFormat::VIDEO_HDR10 },
    { DM_ScreenHDRFormat::VIDEO_HDR_VIVID,        DM_HDRFormat::VIDEO_HDR_VIVID },
    { DM_ScreenHDRFormat::IMAGE_HDR_VIVID_DUAL,   DM_HDRFormat::IMAGE_HDR_VIVID_DUAL },
    { DM_ScreenHDRFormat::IMAGE_HDR_VIVID_SINGLE, DM_HDRFormat::IMAGE_HDR_VIVID_SINGLE },
    { DM_ScreenHDRFormat::IMAGE_HDR_ISO_DUAL,     DM_HDRFormat::IMAGE_HDR_ISO_DUAL },
    { DM_ScreenHDRFormat::IMAGE_HDR_ISO_SINGLE,   DM_HDRFormat::IMAGE_HDR_ISO_SINGLE },
};

class AvailableAreaChangeListener : public DisplayManager::IAvailableAreaListener {
public:
    explicit AvailableAreaChangeListener(OH_NativeDisplayManager_AvailableAreaChangeCallback availableAreaChangeFunc)
    {
        innerAvailableAreaChangeFunc_ = availableAreaChangeFunc;
    }

    void OnAvailableAreaChangedByDisplayId(const DMRect& area, DisplayId displayId)
    {
        if (innerAvailableAreaChangeFunc_ == nullptr) {
            TLOGE(WmsLogTag::DMS, "[DMNDK] callback is nullptr");
            return;
        }
        innerAvailableAreaChangeFunc_(displayId);
    }

    OH_NativeDisplayManager_AvailableAreaChangeCallback GetAvailableAreaChangeInnerFunc()
    {
        return innerAvailableAreaChangeFunc_;
    }

private:
    OH_NativeDisplayManager_AvailableAreaChangeCallback innerAvailableAreaChangeFunc_;
};

class DisplayAddListener : public DisplayManager::IDisplayListener {
public:
    explicit DisplayAddListener(OH_NativeDisplayManager_DisplayAddCallback displayAddFunc)
    {
        innerDisplayAddFunc_ = displayAddFunc;
    }

    void OnCreate(DisplayId displayId)
    {
        if (innerDisplayAddFunc_ == nullptr) {
            TLOGE(WmsLogTag::DMS, "[DMNDK] AddListener callback is nullptr");
            return;
        }
        TLOGI(WmsLogTag::DMS, "[DMNDK] AddListener callback displayId=%{public}" PRIu64, displayId);
        innerDisplayAddFunc_(displayId);
    }

    void OnDestroy(DisplayId displayId)
    {
        TLOGI(WmsLogTag::DMS, "[DMNDK] AddListener current not support delete callback.");
    }

    void OnChange(DisplayId displayId)
    {
        TLOGI(WmsLogTag::DMS, "[DMNDK] AddListener current not support Change callback.");
    }

    OH_NativeDisplayManager_DisplayAddCallback GetDisplayAddInnerFunc()
    {
        return innerDisplayAddFunc_;
    }

private:
    OH_NativeDisplayManager_DisplayAddCallback innerDisplayAddFunc_;
};

class DisplayRemoveListener : public DisplayManager::IDisplayListener {
public:
    explicit DisplayRemoveListener(OH_NativeDisplayManager_DisplayRemoveCallback displayRemoveFunc)
    {
        innerDisplayRemoveFunc_ = displayRemoveFunc;
    }

    void OnCreate(DisplayId displayId)
    {
        TLOGI(WmsLogTag::DMS, "[DMNDK] RemoveListener current not support create callback.");
    }

    void OnDestroy(DisplayId displayId)
    {
        if (innerDisplayRemoveFunc_ == nullptr) {
            TLOGE(WmsLogTag::DMS, "[DMNDK] RemoveListener callback is nullptr");
            return;
        }
        TLOGI(WmsLogTag::DMS, "[DMNDK] RemoveListener callback displayId=%{public}" PRIu64, displayId);
        innerDisplayRemoveFunc_(displayId);
    }

    void OnChange(DisplayId displayId)
    {
        TLOGI(WmsLogTag::DMS, "[DMNDK] RemoveListener current not support Change callback.");
    }

    OH_NativeDisplayManager_DisplayRemoveCallback GetDisplayRemoveInnerFunc()
    {
        return innerDisplayRemoveFunc_;
    }

private:
    OH_NativeDisplayManager_DisplayRemoveCallback innerDisplayRemoveFunc_;
};

std::shared_mutex availableAreaChangeMutex;
std::unordered_map<uint32_t, sptr<AvailableAreaChangeListener>> availableAreaChangeListenerMap;
std::shared_mutex displayAddMutex;
std::unordered_map<uint32_t, sptr<DisplayAddListener>> displayAddListenerMap;
std::shared_mutex displayRemoveMutex;
std::unordered_map<uint32_t, sptr<DisplayRemoveListener>> displayRemoveListenerMap;

bool CheckAvailableAreaChangeHasRegistered(const sptr<AvailableAreaChangeListener>& availableAreaChangeListener)
{
    std::unique_lock<std::shared_mutex> lock(availableAreaChangeMutex);
    for (const auto& iter : availableAreaChangeListenerMap) {
        if (iter.second->GetAvailableAreaChangeInnerFunc() ==
            availableAreaChangeListener->GetAvailableAreaChangeInnerFunc()) {
            return true;
        }
    }
    return false;
}

bool CheckDisplayAddHasRegistered(const sptr<DisplayAddListener>& displayAddListener)
{
    std::unique_lock<std::shared_mutex> lock(displayAddMutex);
    for (const auto& iter : displayAddListenerMap) {
        if (iter.second->GetDisplayAddInnerFunc() == displayAddListener->GetDisplayAddInnerFunc()) {
            return true;
        }
    }
    return false;
}

bool CheckDisplayRemoveHasRegistered(const sptr<DisplayRemoveListener>& displayRemoveListener)
{
    std::unique_lock<std::shared_mutex> lock(displayRemoveMutex);
    for (const auto& iter : displayRemoveListenerMap) {
        if (iter.second->GetDisplayRemoveInnerFunc() == displayRemoveListener->GetDisplayRemoveInnerFunc()) {
            return true;
        }
    }
    return false;
}
}
}

class OH_DisplayModeChangeListener : public DisplayManager::IDisplayModeListener {
private:
    OH_NativeDisplayManager_FoldDisplayModeChangeCallback innerDisplayModeChangeFunc_;
public:
    explicit OH_DisplayModeChangeListener(OH_NativeDisplayManager_FoldDisplayModeChangeCallback displayModeChangeFunc)
    {
        innerDisplayModeChangeFunc_ = displayModeChangeFunc;
    }
    void OnDisplayModeChanged(FoldDisplayMode displayMode)
    {
        if (innerDisplayModeChangeFunc_ == NULL) {
            TLOGI(WmsLogTag::DMS, "[DMNDK] callback is null");
            return;
        }
        TLOGI(WmsLogTag::DMS, "[DMNDK] displayMode callback displayMode=%{public}d", displayMode);
        innerDisplayModeChangeFunc_(static_cast<NativeDisplayManager_FoldDisplayMode>(displayMode));
    }
};

class OH_DisplayChangeListener : public DisplayManager::IDisplayListener {
public:
    explicit OH_DisplayChangeListener(OH_NativeDisplayManager_DisplayChangeCallback displayChangeFunc)
    {
        innerDisplayChangeFunc_ = displayChangeFunc;
    }
    void OnCreate(DisplayId)
    {
        TLOGI(WmsLogTag::DMS, "[DMNDK] current not support create callback.");
    }
    void OnDestroy(DisplayId)
    {
        TLOGI(WmsLogTag::DMS, "[DMNDK] current not support delete callback.");
    }
    void OnChange(DisplayId displayId)
    {
        if (innerDisplayChangeFunc_ == NULL) {
            TLOGI(WmsLogTag::DMS, "[DMNDK] callback is null");
            return;
        }
        TLOGD(WmsLogTag::DMS, "[DMNDK] callback displayId=%{public}" PRIu64, displayId);
        innerDisplayChangeFunc_(static_cast<uint64_t>(displayId));
    }
private:
    OH_NativeDisplayManager_DisplayChangeCallback innerDisplayChangeFunc_;
};

static sptr<DisplayInfo> OH_GetDefaultDisplayInfo()
{
    sptr<Display> defaultDisplay = DisplayManager::GetInstance().GetDefaultDisplaySync();
    if (defaultDisplay == nullptr) {
        TLOGE(WmsLogTag::DMS, "[DMNDK] get default display null.");
        return nullptr;
    }
    auto info = defaultDisplay->GetDisplayInfo();
    if (info == nullptr) {
        TLOGE(WmsLogTag::DMS, "[DMNDK] get default display info null.");
        return nullptr;
    }
    return info;
}

NativeDisplayManager_ErrorCode OH_NativeDisplayManager_GetDefaultDisplayId(uint64_t *displayId)
{
    if (displayId == nullptr) {
        TLOGE(WmsLogTag::DMS, "[DMNDK] input displayId null.");
        return NativeDisplayManager_ErrorCode::DISPLAY_MANAGER_ERROR_INVALID_PARAM;
    }
    auto displayInfo = OH_GetDefaultDisplayInfo();
    if (displayInfo == nullptr) {
        TLOGE(WmsLogTag::DMS, "[DMNDK] get default display info null.");
        return NativeDisplayManager_ErrorCode::DISPLAY_MANAGER_ERROR_SYSTEM_ABNORMAL;
    }
    *displayId = displayInfo->GetDisplayId();
    return NativeDisplayManager_ErrorCode::DISPLAY_MANAGER_OK;
}

NativeDisplayManager_ErrorCode OH_NativeDisplayManager_GetDefaultDisplayWidth(int32_t *displayWidth)
{
    if (displayWidth == nullptr) {
        TLOGE(WmsLogTag::DMS, "[DMNDK] input displayWidth null.");
        return NativeDisplayManager_ErrorCode::DISPLAY_MANAGER_ERROR_INVALID_PARAM;
    }
    auto displayInfo = OH_GetDefaultDisplayInfo();
    if (displayInfo == nullptr) {
        TLOGE(WmsLogTag::DMS, "[DMNDK] get default display info null.");
        return NativeDisplayManager_ErrorCode::DISPLAY_MANAGER_ERROR_SYSTEM_ABNORMAL;
    }
    *displayWidth = displayInfo->GetWidth();
    return NativeDisplayManager_ErrorCode::DISPLAY_MANAGER_OK;
}

NativeDisplayManager_ErrorCode OH_NativeDisplayManager_GetDefaultDisplayHeight(int32_t *displayHeight)
{
    if (displayHeight == nullptr) {
        TLOGE(WmsLogTag::DMS, "[DMNDK] input displayHeight null.");
        return NativeDisplayManager_ErrorCode::DISPLAY_MANAGER_ERROR_INVALID_PARAM;
    }
    auto displayInfo = OH_GetDefaultDisplayInfo();
    if (displayInfo == nullptr) {
        TLOGE(WmsLogTag::DMS, "[DMNDK] get default display info null.");
        return NativeDisplayManager_ErrorCode::DISPLAY_MANAGER_ERROR_SYSTEM_ABNORMAL;
    }
    *displayHeight = displayInfo->GetHeight();
    return NativeDisplayManager_ErrorCode::DISPLAY_MANAGER_OK;
}

NativeDisplayManager_ErrorCode OH_NativeDisplayManager_GetDefaultDisplayRotation(
    NativeDisplayManager_Rotation *displayRotation)
{
    if (displayRotation == nullptr) {
        TLOGE(WmsLogTag::DMS, "[DMNDK] input displayRotation null.");
        return NativeDisplayManager_ErrorCode::DISPLAY_MANAGER_ERROR_INVALID_PARAM;
    }
    auto displayInfo = OH_GetDefaultDisplayInfo();
    if (displayInfo == nullptr) {
        TLOGE(WmsLogTag::DMS, "[DMNDK] get default display info null.");
        return NativeDisplayManager_ErrorCode::DISPLAY_MANAGER_ERROR_SYSTEM_ABNORMAL;
    }
    *displayRotation = static_cast<NativeDisplayManager_Rotation>(displayInfo->GetRotation());
    return NativeDisplayManager_ErrorCode::DISPLAY_MANAGER_OK;
}

NativeDisplayManager_ErrorCode OH_NativeDisplayManager_GetDefaultDisplayOrientation(
    NativeDisplayManager_Orientation *displayOrientation)
{
    if (displayOrientation == nullptr) {
        TLOGE(WmsLogTag::DMS, "[DMNDK] input displayOrientation null.");
        return NativeDisplayManager_ErrorCode::DISPLAY_MANAGER_ERROR_INVALID_PARAM;
    }
    auto displayInfo = OH_GetDefaultDisplayInfo();
    if (displayInfo == nullptr) {
        TLOGE(WmsLogTag::DMS, "[DMNDK] get default display info null.");
        return NativeDisplayManager_ErrorCode::DISPLAY_MANAGER_ERROR_SYSTEM_ABNORMAL;
    }
    *displayOrientation = static_cast<NativeDisplayManager_Orientation>(displayInfo->GetDisplayOrientation());
    return NativeDisplayManager_ErrorCode::DISPLAY_MANAGER_OK;
}

NativeDisplayManager_ErrorCode OH_NativeDisplayManager_GetDefaultDisplayVirtualPixelRatio(float *virtualPixel)
{
    if (virtualPixel == nullptr) {
        TLOGE(WmsLogTag::DMS, "[DMNDK] input virtualPixel null.");
        return NativeDisplayManager_ErrorCode::DISPLAY_MANAGER_ERROR_INVALID_PARAM;
    }
    auto displayInfo = OH_GetDefaultDisplayInfo();
    if (displayInfo == nullptr) {
        TLOGE(WmsLogTag::DMS, "[DMNDK] get default display info null.");
        return NativeDisplayManager_ErrorCode::DISPLAY_MANAGER_ERROR_SYSTEM_ABNORMAL;
    }
    *virtualPixel = displayInfo->GetVirtualPixelRatio();
    return NativeDisplayManager_ErrorCode::DISPLAY_MANAGER_OK;
}

NativeDisplayManager_ErrorCode OH_NativeDisplayManager_GetDefaultDisplayRefreshRate(uint32_t *refreshRate)
{
    if (refreshRate == nullptr) {
        TLOGE(WmsLogTag::DMS, "[DMNDK] input refreshRate null.");
        return NativeDisplayManager_ErrorCode::DISPLAY_MANAGER_ERROR_INVALID_PARAM;
    }
    auto displayInfo = OH_GetDefaultDisplayInfo();
    if (displayInfo == nullptr) {
        TLOGE(WmsLogTag::DMS, "[DMNDK] get default display info null.");
        return NativeDisplayManager_ErrorCode::DISPLAY_MANAGER_ERROR_SYSTEM_ABNORMAL;
    }
    *refreshRate = displayInfo->GetRefreshRate();
    return NativeDisplayManager_ErrorCode::DISPLAY_MANAGER_OK;
}

NativeDisplayManager_ErrorCode OH_NativeDisplayManager_GetDefaultDisplayDensityDpi(int32_t *densityDpi)
{
    if (densityDpi == nullptr) {
        TLOGE(WmsLogTag::DMS, "[DMNDK] input densityDpi null.");
        return NativeDisplayManager_ErrorCode::DISPLAY_MANAGER_ERROR_INVALID_PARAM;
    }
    auto displayInfo = OH_GetDefaultDisplayInfo();
    if (displayInfo == nullptr) {
        TLOGE(WmsLogTag::DMS, "[DMNDK] get default display info null.");
        return NativeDisplayManager_ErrorCode::DISPLAY_MANAGER_ERROR_SYSTEM_ABNORMAL;
    }
    *densityDpi = displayInfo->GetVirtualPixelRatio() * DOT_PER_INCH;
    return NativeDisplayManager_ErrorCode::DISPLAY_MANAGER_OK;
}

NativeDisplayManager_ErrorCode OH_NativeDisplayManager_GetDefaultDisplayDensityPixels(float *densityPixels)
{
    if (densityPixels == nullptr) {
        TLOGE(WmsLogTag::DMS, "[DMNDK] input densityPixels null.");
        return NativeDisplayManager_ErrorCode::DISPLAY_MANAGER_ERROR_INVALID_PARAM;
    }
    auto displayInfo = OH_GetDefaultDisplayInfo();
    if (displayInfo == nullptr) {
        TLOGE(WmsLogTag::DMS, "[DMNDK] get default display info null.");
        return NativeDisplayManager_ErrorCode::DISPLAY_MANAGER_ERROR_SYSTEM_ABNORMAL;
    }
    *densityPixels = displayInfo->GetVirtualPixelRatio();
    return NativeDisplayManager_ErrorCode::DISPLAY_MANAGER_OK;
}

NativeDisplayManager_ErrorCode OH_NativeDisplayManager_GetDefaultDisplayScaledDensity(float *scaledDensity)
{
    if (scaledDensity == nullptr) {
        TLOGE(WmsLogTag::DMS, "[DMNDK] input scaledDensity null.");
        return NativeDisplayManager_ErrorCode::DISPLAY_MANAGER_ERROR_INVALID_PARAM;
    }
    auto displayInfo = OH_GetDefaultDisplayInfo();
    if (displayInfo == nullptr) {
        TLOGE(WmsLogTag::DMS, "[DMNDK] get default display info null.");
        return NativeDisplayManager_ErrorCode::DISPLAY_MANAGER_ERROR_SYSTEM_ABNORMAL;
    }
    *scaledDensity = displayInfo->GetVirtualPixelRatio();
    return NativeDisplayManager_ErrorCode::DISPLAY_MANAGER_OK;
}

NativeDisplayManager_ErrorCode OH_NativeDisplayManager_GetDefaultDisplayDensityXdpi(float *xDpi)
{
    if (xDpi == nullptr) {
        TLOGE(WmsLogTag::DMS, "[DMNDK] input xDpi null.");
        return NativeDisplayManager_ErrorCode::DISPLAY_MANAGER_ERROR_INVALID_PARAM;
    }
    auto displayInfo = OH_GetDefaultDisplayInfo();
    if (displayInfo == nullptr) {
        TLOGE(WmsLogTag::DMS, "[DMNDK] get default display info null.");
        return NativeDisplayManager_ErrorCode::DISPLAY_MANAGER_ERROR_SYSTEM_ABNORMAL;
    }
    *xDpi = displayInfo->GetXDpi();
    return NativeDisplayManager_ErrorCode::DISPLAY_MANAGER_OK;
}

NativeDisplayManager_ErrorCode OH_NativeDisplayManager_GetDefaultDisplayDensityYdpi(float *yDpi)
{
    if (yDpi == nullptr) {
        TLOGE(WmsLogTag::DMS, "[DMNDK] input yDpi null.");
        return NativeDisplayManager_ErrorCode::DISPLAY_MANAGER_ERROR_INVALID_PARAM;
    }
    auto displayInfo = OH_GetDefaultDisplayInfo();
    if (displayInfo == nullptr) {
        TLOGE(WmsLogTag::DMS, "[DMNDK] get default display info null.");
        return NativeDisplayManager_ErrorCode::DISPLAY_MANAGER_ERROR_SYSTEM_ABNORMAL;
    }
    *yDpi = displayInfo->GetYDpi();
    return NativeDisplayManager_ErrorCode::DISPLAY_MANAGER_OK;
}

static void OH_SetDisplayRect(const DMRect &dmRect, NativeDisplayManager_Rect *displayRectItem)
{
    displayRectItem->left = dmRect.posX_;
    displayRectItem->top = dmRect.posY_;
    displayRectItem->width = dmRect.width_;
    displayRectItem->height = dmRect.height_;
}

static NativeDisplayManager_Rect* OH_CreateBoundingRects(const std::vector<DMRect> &boundingRects)
{
    int32_t boundSize = static_cast<int32_t>(boundingRects.size());
    NativeDisplayManager_Rect *ohBoundingRects =
        static_cast<NativeDisplayManager_Rect*>(malloc(sizeof(NativeDisplayManager_Rect) * boundSize));
    if (ohBoundingRects == NULL) {
        TLOGE(WmsLogTag::DMS, "[DMNDK] memory failed.");
        return NULL;
    }
    auto retMemset = memset_s(ohBoundingRects, sizeof(NativeDisplayManager_Rect) * boundSize, 0,
        sizeof(NativeDisplayManager_Rect) * boundSize);
    if (retMemset != 0) {
        free(ohBoundingRects);
        ohBoundingRects = NULL;
        TLOGE(WmsLogTag::DMS, "[DMNDK] memset failed.");
        return NULL;
    }
    TLOGI(WmsLogTag::DMS, "[DMNDK] bounding size:%{public}d.", boundSize);
    for (int i = 0; i < boundSize; i++) {
        OH_SetDisplayRect(boundingRects[i], (ohBoundingRects + i));
    }
    return ohBoundingRects;
}

static void OH_SetWaterfallDisplayAreaRects(const WaterfallDisplayAreaRects &waterArea,
    NativeDisplayManager_CutoutInfo *ohCutoutInfo)
{
    if (ohCutoutInfo == NULL) {
        TLOGE(WmsLogTag::DMS, "[DMNDK] ohCutoutInfo is null.");
        return;
    }
    TLOGI(WmsLogTag::DMS, "[DMNDK] set waterfall Area.");
    OH_SetDisplayRect(waterArea.left, &(ohCutoutInfo->waterfallDisplayAreaRects.left));
    OH_SetDisplayRect(waterArea.top, &(ohCutoutInfo->waterfallDisplayAreaRects.top));
    OH_SetDisplayRect(waterArea.right, &(ohCutoutInfo->waterfallDisplayAreaRects.right));
    OH_SetDisplayRect(waterArea.bottom, &(ohCutoutInfo->waterfallDisplayAreaRects.bottom));
}

static NativeDisplayManager_CutoutInfo* OH_CreateCutoutInfoObject(const sptr<CutoutInfo> &cutoutInfo)
{
    NativeDisplayManager_CutoutInfo *ohCutoutInfo =
        static_cast<NativeDisplayManager_CutoutInfo*>(malloc(sizeof(NativeDisplayManager_CutoutInfo)));
    if (ohCutoutInfo == NULL) {
        TLOGE(WmsLogTag::DMS, "[DMNDK] memory failed.");
        return NULL;
    }
    auto retMemset = memset_s(ohCutoutInfo, sizeof(NativeDisplayManager_CutoutInfo), 0,
        sizeof(NativeDisplayManager_CutoutInfo));
    if (retMemset != 0) {
        free(ohCutoutInfo);
        ohCutoutInfo = NULL;
        TLOGE(WmsLogTag::DMS, "[DMNDK] memset failed.");
        return NULL;
    }
    std::vector<DMRect> boundingRects = cutoutInfo->GetBoundingRects();
    WaterfallDisplayAreaRects waterRects = cutoutInfo->GetWaterfallDisplayAreaRects();
    ohCutoutInfo->boundingRectsLength = static_cast<int32_t>(boundingRects.size());
    TLOGI(WmsLogTag::DMS, "[DMNDK] boundingRectsLength=%{public}d.", ohCutoutInfo->boundingRectsLength);
    ohCutoutInfo->boundingRects = OH_CreateBoundingRects(boundingRects);
    if (ohCutoutInfo->boundingRects == NULL) {
        TLOGE(WmsLogTag::DMS, "[DMNDK] create bounding rects failed.");
        free(ohCutoutInfo);
        ohCutoutInfo = NULL;
        return NULL;
    }
    OH_SetWaterfallDisplayAreaRects(waterRects, ohCutoutInfo);
    return ohCutoutInfo;
}

NativeDisplayManager_ErrorCode OH_NativeDisplayManager_CreateDefaultDisplayCutoutInfo(
    NativeDisplayManager_CutoutInfo **cutoutInfo)
{
    if (cutoutInfo == NULL) {
        TLOGE(WmsLogTag::DMS, "[DMNDK] input cutoutInfo null.");
        return NativeDisplayManager_ErrorCode::DISPLAY_MANAGER_ERROR_INVALID_PARAM;
    }
    TLOGI(WmsLogTag::DMS, "[DMNDK] get display cutout info.");
    sptr<Display> defaultDisplay = DisplayManager::GetInstance().GetDefaultDisplaySync();
    if (defaultDisplay == nullptr) {
        TLOGE(WmsLogTag::DMS, "[DMNDK] get cutout info (display) null.");
        return NativeDisplayManager_ErrorCode::DISPLAY_MANAGER_ERROR_SYSTEM_ABNORMAL;
    }
    sptr<CutoutInfo> cutoutInfoInner = defaultDisplay->GetCutoutInfo();
    if (cutoutInfoInner == nullptr) {
        TLOGE(WmsLogTag::DMS, "[DMNDK] get cutout info (from display) null.");
        return NativeDisplayManager_ErrorCode::DISPLAY_MANAGER_ERROR_SYSTEM_ABNORMAL;
    }
    *cutoutInfo = OH_CreateCutoutInfoObject(cutoutInfoInner);
    if (*cutoutInfo == NULL) {
        TLOGE(WmsLogTag::DMS, "[DMNDK] convert cutout info null.");
        return NativeDisplayManager_ErrorCode::DISPLAY_MANAGER_ERROR_SYSTEM_ABNORMAL;
    }
    TLOGI(WmsLogTag::DMS, "[DMNDK] get display cutout info success.");
    return NativeDisplayManager_ErrorCode::DISPLAY_MANAGER_OK;
}

NativeDisplayManager_ErrorCode OH_NativeDisplayManager_DestroyDefaultDisplayCutoutInfo(
    NativeDisplayManager_CutoutInfo *cutoutInfo)
{
    if (cutoutInfo == NULL) {
        TLOGE(WmsLogTag::DMS, "[DMNDK] input cutoutInfo null pointer.");
        return NativeDisplayManager_ErrorCode::DISPLAY_MANAGER_ERROR_INVALID_PARAM;
    }
    if (cutoutInfo->boundingRects != NULL) {
        free(cutoutInfo->boundingRects);
        cutoutInfo->boundingRects = NULL;
    }
    free(cutoutInfo);
    cutoutInfo = NULL;
    TLOGI(WmsLogTag::DMS, "[DMNDK] destroy cutoutInfo end.");
    return NativeDisplayManager_ErrorCode::DISPLAY_MANAGER_OK;
}

bool OH_NativeDisplayManager_IsFoldable()
{
    bool isFoldable = DisplayManager::GetInstance().IsFoldable();
    TLOGI(WmsLogTag::DMS, "[DMNDK] get display isFoldable=%{public}d.", isFoldable);
    return isFoldable;
}

NativeDisplayManager_ErrorCode OH_NativeDisplayManager_GetFoldDisplayMode(
    NativeDisplayManager_FoldDisplayMode *foldDisplayMode)
{
    TLOGI(WmsLogTag::DMS, "[DMNDK] get fold display mode.");
    if (foldDisplayMode == NULL) {
        TLOGE(WmsLogTag::DMS, "[DMNDK] input foldDisplayMode null.");
        return NativeDisplayManager_ErrorCode::DISPLAY_MANAGER_ERROR_INVALID_PARAM;
    }
    if (!DisplayManager::GetInstance().IsFoldable()) {
        TLOGE(WmsLogTag::DMS, "[DMNDK] device is not foldable.");
        return NativeDisplayManager_ErrorCode::DISPLAY_MANAGER_ERROR_DEVICE_NOT_SUPPORTED;
    }
    FoldDisplayMode foldMode = DisplayManager::GetInstance().GetFoldDisplayModeForExternal();
    switch (foldMode) {
        case FoldDisplayMode::SUB:
            *foldDisplayMode = NativeDisplayManager_FoldDisplayMode::DISPLAY_MANAGER_FOLD_DISPLAY_MODE_SUB;
            break;
        case FoldDisplayMode::MAIN:
            *foldDisplayMode = NativeDisplayManager_FoldDisplayMode::DISPLAY_MANAGER_FOLD_DISPLAY_MODE_MAIN;
            break;
        case FoldDisplayMode::FULL:
            *foldDisplayMode = NativeDisplayManager_FoldDisplayMode::DISPLAY_MANAGER_FOLD_DISPLAY_MODE_FULL;
            break;
        case FoldDisplayMode::COORDINATION:
            *foldDisplayMode = NativeDisplayManager_FoldDisplayMode::DISPLAY_MANAGER_FOLD_DISPLAY_MODE_COORDINATION;
            break;
        default:
            *foldDisplayMode = NativeDisplayManager_FoldDisplayMode::DISPLAY_MANAGER_FOLD_DISPLAY_MODE_UNKNOWN;
            break;
    }
    TLOGI(WmsLogTag::DMS, "[DMNDK] current fold display mode: %{public}d.", *foldDisplayMode);
    return NativeDisplayManager_ErrorCode::DISPLAY_MANAGER_OK;
}

std::shared_mutex foldChangeMutex;
std::map<uint32_t, OH_NativeDisplayManager_FoldDisplayModeChangeCallback> g_foldChangeCallbackMap;
std::map<uint32_t, sptr<DisplayManager::IDisplayModeListener>> g_foldDisplayModeChangeListenerMap;

bool CheckFoldChangeHasRegistered(OH_NativeDisplayManager_FoldDisplayModeChangeCallback displayModeChangeCallback)
{
    if (g_foldChangeCallbackMap.empty()) {
        return false;
    }
    for (auto iter : g_foldChangeCallbackMap) {
        if (iter.second == displayModeChangeCallback) {
            return true;
        }
    }
    return false;
}

NativeDisplayManager_ErrorCode OH_NativeDisplayManager_RegisterFoldDisplayModeChangeListener(
    OH_NativeDisplayManager_FoldDisplayModeChangeCallback displayModeChangeCallback, uint32_t *listenerIndex)
{
    TLOGI(WmsLogTag::DMS, "[DMNDK] register fold display mode change listener.");
    if (displayModeChangeCallback == NULL || listenerIndex == NULL) {
        TLOGE(WmsLogTag::DMS, "[DMNDK] input params null.");
        return NativeDisplayManager_ErrorCode::DISPLAY_MANAGER_ERROR_INVALID_PARAM;
    }
    if (!DisplayManager::GetInstance().IsFoldable()) {
        TLOGE(WmsLogTag::DMS, "[DMNDK] device is not foldable.");
        return NativeDisplayManager_ErrorCode::DISPLAY_MANAGER_ERROR_DEVICE_NOT_SUPPORTED;
    }
    std::unique_lock<std::shared_mutex> lock(foldChangeMutex);
    if (CheckFoldChangeHasRegistered(displayModeChangeCallback)) {
        TLOGE(WmsLogTag::DMS, "[DMNDK] input params error (has registered).");
        return NativeDisplayManager_ErrorCode::DISPLAY_MANAGER_ERROR_INVALID_PARAM;
    }
    sptr<DisplayManager::IDisplayModeListener> displayModeListener =
        sptr<OH_DisplayModeChangeListener>::MakeSptr(displayModeChangeCallback);
    if (displayModeListener == nullptr) {
        TLOGE(WmsLogTag::DMS, "[DMNDK] display mode listener MakeSptr fail.");
        return NativeDisplayManager_ErrorCode::DISPLAY_MANAGER_ERROR_SYSTEM_ABNORMAL;
    }
    static std::atomic<uint32_t> registerCount = 1;
    DMError ret = DisplayManager::GetInstance().RegisterDisplayModeListener(displayModeListener);
    if (ret != DMError::DM_OK) {
        TLOGE(WmsLogTag::DMS, "[DMNDK] display mode listener register failed ret=%{public}d.", ret);
        return NativeDisplayManager_ErrorCode::DISPLAY_MANAGER_ERROR_SYSTEM_ABNORMAL;
    }
    *listenerIndex = registerCount++;
    g_foldChangeCallbackMap.emplace(*listenerIndex, displayModeChangeCallback);
    g_foldDisplayModeChangeListenerMap.emplace(*listenerIndex, displayModeListener);
    TLOGI(WmsLogTag::DMS, "[DMNDK] register fold change success and listenerIndex= %{public}d.", *listenerIndex);
    return NativeDisplayManager_ErrorCode::DISPLAY_MANAGER_OK;
}

NativeDisplayManager_ErrorCode OH_NativeDisplayManager_UnregisterFoldDisplayModeChangeListener(uint32_t listenerIndex)
{
    TLOGI(WmsLogTag::DMS, "[DMNDK] unregister fold display mode change listener %{public}d.", listenerIndex);
    if (!DisplayManager::GetInstance().IsFoldable()) {
        TLOGE(WmsLogTag::DMS, "[DMNDK] unregister fail(device is not foldable).");
        return NativeDisplayManager_ErrorCode::DISPLAY_MANAGER_ERROR_DEVICE_NOT_SUPPORTED;
    }
    std::unique_lock<std::shared_mutex> lock(foldChangeMutex);
    auto iter = g_foldDisplayModeChangeListenerMap.find(listenerIndex);
    if (iter == g_foldDisplayModeChangeListenerMap.end()) {
        TLOGE(WmsLogTag::DMS, "[DMNDK] unregister listener fail(not find register info).");
        return NativeDisplayManager_ErrorCode::DISPLAY_MANAGER_ERROR_INVALID_PARAM;
    }
    DMError ret = DMError::DM_OK;
    if (iter->second != nullptr) {
        ret = DisplayManager::GetInstance().UnregisterDisplayModeListener(iter->second);
        g_foldDisplayModeChangeListenerMap.erase(listenerIndex);
        g_foldChangeCallbackMap.erase(listenerIndex);
        TLOGI(WmsLogTag::DMS, "[DMNDK] unregister listener ert=%{public}d", ret);
    }
    return ret == DMError::DM_OK ? NativeDisplayManager_ErrorCode::DISPLAY_MANAGER_OK :
        NativeDisplayManager_ErrorCode::DISPLAY_MANAGER_ERROR_SYSTEM_ABNORMAL;
}

std::shared_mutex displayChangeMutex;
std::map<uint32_t, OH_NativeDisplayManager_DisplayChangeCallback> g_displayChangeCallbackMap;
std::map<uint32_t, sptr<DisplayManager::IDisplayListener>> g_displayChangeListenerMap;

bool CheckDisplayChangeHasRegistered(OH_NativeDisplayManager_DisplayChangeCallback displayChangeCallback)
{
    if (g_displayChangeCallbackMap.empty()) {
        return false;
    }
    for (auto iter : g_displayChangeCallbackMap) {
        if (iter.second == displayChangeCallback) {
            return true;
        }
    }
    return false;
}

NativeDisplayManager_ErrorCode OH_NativeDisplayManager_RegisterDisplayChangeListener(
    OH_NativeDisplayManager_DisplayChangeCallback displayChangeCallback, uint32_t *listenerIndex)
{
    TLOGI(WmsLogTag::DMS, "[DMNDK] register display change listener.");
    if (displayChangeCallback == NULL || listenerIndex == NULL) {
        TLOGE(WmsLogTag::DMS, "[DMNDK] register fail(input params null).");
        return NativeDisplayManager_ErrorCode::DISPLAY_MANAGER_ERROR_INVALID_PARAM;
    }
    std::unique_lock<std::shared_mutex> lock(displayChangeMutex);
    if (CheckDisplayChangeHasRegistered(displayChangeCallback)) {
        TLOGE(WmsLogTag::DMS, "[DMNDK] input params error (has registered).");
        return NativeDisplayManager_ErrorCode::DISPLAY_MANAGER_ERROR_INVALID_PARAM;
    }
    sptr<DisplayManager::IDisplayListener> displayListener =
        sptr<OH_DisplayChangeListener>::MakeSptr(displayChangeCallback);
    if (displayListener == nullptr) {
        TLOGE(WmsLogTag::DMS, "[DMNDK] register display change MakeSptr fail.");
        return NativeDisplayManager_ErrorCode::DISPLAY_MANAGER_ERROR_SYSTEM_ABNORMAL;
    }
    static std::atomic<uint32_t> registerCount = 1;
    DMError ret = DisplayManager::GetInstance().RegisterDisplayListener(displayListener);
    if (ret != DMError::DM_OK) {
        TLOGE(WmsLogTag::DMS, "[DMNDK] register failed ret=%{public}d.", ret);
        return NativeDisplayManager_ErrorCode::DISPLAY_MANAGER_ERROR_SYSTEM_ABNORMAL;
    }
    *listenerIndex = registerCount++;
    g_displayChangeCallbackMap.emplace(*listenerIndex, displayChangeCallback);
    g_displayChangeListenerMap.emplace(*listenerIndex, displayListener);
    TLOGI(WmsLogTag::DMS, "[DMNDK] register listenerIndex= %{public}d.", *listenerIndex);
    return NativeDisplayManager_ErrorCode::DISPLAY_MANAGER_OK;
}

NativeDisplayManager_ErrorCode OH_NativeDisplayManager_UnregisterDisplayChangeListener(uint32_t listenerIndex)
{
    TLOGI(WmsLogTag::DMS, "[DMNDK] unregister display change listener %{public}d.", listenerIndex);
    std::unique_lock<std::shared_mutex> lock(displayChangeMutex);
    auto iter = g_displayChangeListenerMap.find(listenerIndex);
    if (iter == g_displayChangeListenerMap.end()) {
        TLOGE(WmsLogTag::DMS, "[DMNDK] unregister fail(not find register info).");
        return NativeDisplayManager_ErrorCode::DISPLAY_MANAGER_ERROR_INVALID_PARAM;
    }
    DMError ret = DMError::DM_OK;
    if (iter->second != nullptr) {
        ret = DisplayManager::GetInstance().UnregisterDisplayListener(iter->second);
        g_displayChangeListenerMap.erase(listenerIndex);
        g_displayChangeCallbackMap.erase(listenerIndex);
        TLOGI(WmsLogTag::DMS, "[DMNDK] unregister listener ret=%{public}d", ret);
    }
    return ret == DMError::DM_OK ? NativeDisplayManager_ErrorCode::DISPLAY_MANAGER_OK :
        NativeDisplayManager_ErrorCode::DISPLAY_MANAGER_ERROR_SYSTEM_ABNORMAL;
}

static void NativeDisplayManager_SetColorSpace(NativeDisplayManager_DisplayInfo *displayInfo, sptr<DisplayInfo> info)
{
    std::vector<uint32_t> colorSpaces = info->GetColorSpaces();
    if (colorSpaces.empty()) {
        TLOGW(WmsLogTag::DMS, "[DMNDK] colorSpaces is empty displayId=%{public}u", displayInfo->id);
        return;
    }
    displayInfo->colorSpace = (NativeDisplayManager_DisplayColorSpace*)malloc(
        sizeof(NativeDisplayManager_DisplayColorSpace));
    if (displayInfo->colorSpace == nullptr) {
        TLOGE(WmsLogTag::DMS, "[DMNDK] malloc color space failed");
        return;
    }
    auto retMemset = memset_s(displayInfo->colorSpace, sizeof(NativeDisplayManager_DisplayColorSpace), 0,
        sizeof(NativeDisplayManager_DisplayColorSpace));
    if (retMemset != EOK) {
        TLOGE(WmsLogTag::DMS, "[DMNDK] memset color space failed");
        DISPLAY_MANAGER_FREE_MEMORY(displayInfo->colorSpace);
        return;
    }
    displayInfo->colorSpace->colorSpaceLength = static_cast<uint32_t>(colorSpaces.size());
    displayInfo->colorSpace->colorSpaces = (uint32_t*)malloc(sizeof(uint32_t) * colorSpaces.size());
    if (displayInfo->colorSpace->colorSpaces == nullptr) {
        TLOGE(WmsLogTag::DMS, "[DMNDK] malloc color spaces failed");
        DISPLAY_MANAGER_FREE_MEMORY(displayInfo->colorSpace);
        return;
    }
    retMemset = memset_s(displayInfo->colorSpace->colorSpaces, sizeof(uint32_t) * colorSpaces.size(), 0,
        sizeof(uint32_t) * colorSpaces.size());
    if (retMemset != EOK) {
        TLOGE(WmsLogTag::DMS, "[DMNDK] memset color spaces failed");
        DISPLAY_MANAGER_FREE_MEMORY(displayInfo->colorSpace->colorSpaces);
        DISPLAY_MANAGER_FREE_MEMORY(displayInfo->colorSpace);
        return;
    }

    uint32_t colorLoop = 0;
    for (const auto colorSpace : colorSpaces) {
        DM_GraphicCM_ColorSpaceType colorSpaceValue = static_cast<DM_GraphicCM_ColorSpaceType>(colorSpace);
        if (DM_NATIVE_TO_NDK_COLOR_SPACE_TYPE_MAP.find(colorSpaceValue) ==
            DM_NATIVE_TO_NDK_COLOR_SPACE_TYPE_MAP.end()) {
            TLOGW(WmsLogTag::DMS, "[DMNDK] color spaces[%{public}d] not in map.", colorSpace);
            continue;
        }
        displayInfo->colorSpace->colorSpaces[colorLoop] =
            static_cast<uint32_t>(DM_NATIVE_TO_NDK_COLOR_SPACE_TYPE_MAP.at(colorSpaceValue));
        colorLoop++;
    }
    TLOGI(WmsLogTag::DMS, "[DMNDK] color spaces count:%{public}u.", colorLoop);
}

static void NativeDisplayManager_SetHdrFormat(NativeDisplayManager_DisplayInfo *displayInfo, sptr<DisplayInfo> info)
{
    std::vector<uint32_t> hdrFormats = info->GetHdrFormats();
    if (hdrFormats.empty()) {
        TLOGW(WmsLogTag::DMS, "[DMNDK] hdrFormats is empty displayId=%{public}u", displayInfo->id);
        return;
    }
    displayInfo->hdrFormat = (NativeDisplayManager_DisplayHdrFormat*)malloc(
        sizeof(NativeDisplayManager_DisplayHdrFormat));
    if (displayInfo->hdrFormat == nullptr) {
        TLOGE(WmsLogTag::DMS, "[DMNDK] malloc hdr format failed");
        return;
    }
    auto retMemset = memset_s(displayInfo->hdrFormat, sizeof(NativeDisplayManager_DisplayHdrFormat), 0,
        sizeof(NativeDisplayManager_DisplayHdrFormat));
    if (retMemset != EOK) {
        TLOGE(WmsLogTag::DMS, "[DMNDK] memset hdr format failed");
        DISPLAY_MANAGER_FREE_MEMORY(displayInfo->hdrFormat);
        return;
    }
    displayInfo->hdrFormat->hdrFormatLength = static_cast<uint32_t>(hdrFormats.size());
    displayInfo->hdrFormat->hdrFormats = (uint32_t*)malloc(sizeof(uint32_t) * hdrFormats.size());
    if (displayInfo->hdrFormat->hdrFormats == nullptr) {
        TLOGE(WmsLogTag::DMS, "[DMNDK] malloc hdr format failed");
        DISPLAY_MANAGER_FREE_MEMORY(displayInfo->hdrFormat);
        return;
    }
    retMemset = memset_s(displayInfo->hdrFormat->hdrFormats, sizeof(uint32_t) * hdrFormats.size(), 0,
        sizeof(uint32_t) * hdrFormats.size());
    if (retMemset != EOK) {
        TLOGE(WmsLogTag::DMS, "[DMNDK] memset hdr format failed");
        DISPLAY_MANAGER_FREE_MEMORY(displayInfo->hdrFormat->hdrFormats);
        DISPLAY_MANAGER_FREE_MEMORY(displayInfo->hdrFormat);
        return;
    }

    uint32_t hdrLoop = 0;
    for (const auto hdrFormat : hdrFormats) {
        DM_ScreenHDRFormat hdrFormatValue = static_cast<DM_ScreenHDRFormat>(hdrFormat);
        if (DM_NATIVE_TO_NDK_HDR_FORMAT_TYPE_MAP.find(hdrFormatValue) == DM_NATIVE_TO_NDK_HDR_FORMAT_TYPE_MAP.end()) {
            TLOGW(WmsLogTag::DMS, "[DMNDK] hdr format[%{public}d] not in map.", hdrFormat);
            continue;
        }
        displayInfo->hdrFormat->hdrFormats[hdrLoop] =
            static_cast<uint32_t>(DM_NATIVE_TO_NDK_HDR_FORMAT_TYPE_MAP.at(hdrFormatValue));
        hdrLoop++;
    }
    TLOGI(WmsLogTag::DMS, "[DMNDK] hdr format count:%{public}u", hdrLoop);
}

static void NativeDisplayManager_SetDisplayInfo(NativeDisplayManager_DisplayInfo *displayInfo,
    sptr<DisplayInfo> info)
{
    displayInfo->id = static_cast<uint32_t>(info->GetDisplayId());
    displayInfo->width = info->GetWidth();
    displayInfo->height = info->GetHeight();
    displayInfo->orientation = static_cast<NativeDisplayManager_Orientation>(info->GetDisplayOrientation());
    displayInfo->rotation = static_cast<NativeDisplayManager_Rotation>(info->GetRotation());
    displayInfo->refreshRate = info->GetRefreshRate();
    displayInfo->availableWidth = info->GetAvailableWidth();
    displayInfo->availableHeight = info->GetAvailableHeight();
    displayInfo->physicalWidth = info->GetPhysicalWidth();
    displayInfo->physicalHeight = info->GetPhysicalHeight();
    displayInfo->densityDPI = info->GetVirtualPixelRatio() * DOT_PER_INCH;
    displayInfo->densityPixels = info->GetVirtualPixelRatio();
    displayInfo->scaledDensity = info->GetVirtualPixelRatio();
    displayInfo->xDPI = info->GetXDpi();
    displayInfo->yDPI = info->GetYDpi();
    displayInfo->isAlive = info->GetAliveStatus();
    if (DM_NATIVE_TO_NDK_DISPLAY_STATE_MAP.find(info->GetDisplayState()) != DM_NATIVE_TO_NDK_DISPLAY_STATE_MAP.end()) {
        displayInfo->state = static_cast<NativeDisplayManager_DisplayState>(
            DM_NATIVE_TO_NDK_DISPLAY_STATE_MAP.at(info->GetDisplayState()));
    } else {
        displayInfo->state = static_cast<NativeDisplayManager_DisplayState>(DM_DisplayStateMode::STATE_UNKNOWN);
    }
    /* color space */
    NativeDisplayManager_SetColorSpace(displayInfo, info);
    /* hdr format */
    NativeDisplayManager_SetHdrFormat(displayInfo, info);
    TLOGI(WmsLogTag::DMS, "[DMNDK] set display id[%{public}u] finish.", displayInfo->id);
}

static NativeDisplayManager_ErrorCode NativeDisplayManager_SetDisplaysInfo(const std::vector<sptr<Display>>& displays,
    NativeDisplayManager_DisplayInfo *displaysInfo)
{
    uint32_t i = 0;
    for (auto& display : displays) {
        if (display == nullptr) {
            TLOGE(WmsLogTag::DMS, "[DMNDK] get display null.");
            continue;
        }
        auto info = display->GetDisplayInfo();
        if (info == nullptr) {
            TLOGE(WmsLogTag::DMS, "[DMNDK] get display id[%{public}" PRIu64"] info null.", display->GetId());
            continue;
        }
        auto errCode = memset_s(displaysInfo[i].name, sizeof(displaysInfo[i].name), 0, sizeof(displaysInfo[i].name));
        if (errCode != EOK) {
            TLOGE(WmsLogTag::DMS, "[DMNDK] memset display info fail.");
            return NativeDisplayManager_ErrorCode::DISPLAY_MANAGER_ERROR_SYSTEM_ABNORMAL;
        }
        int ret = memcpy_s(displaysInfo[i].name, sizeof(displaysInfo[i].name) - 1, info->GetName().c_str(),
                           info->GetName().size());
        if (ret != EOK) {
            TLOGE(WmsLogTag::DMS, "[DMNDK] failed to memcpy name.");
            return NativeDisplayManager_ErrorCode::DISPLAY_MANAGER_ERROR_SYSTEM_ABNORMAL;
        }
        NativeDisplayManager_SetDisplayInfo(displaysInfo + i, info);
        i++;
    }
    return NativeDisplayManager_ErrorCode::DISPLAY_MANAGER_OK;
}

static void NativeDisplayManager_DestroyDisplaysInfoInner(uint32_t displaysLength,
    NativeDisplayManager_DisplayInfo *displaysInfo)
{
    if (displaysLength == 0 || displaysInfo == nullptr) {
        TLOGE(WmsLogTag::DMS, "[DMNDK] param is null.");
        return;
    }
    for (uint32_t i = 0; i < displaysLength; i++) {
        NativeDisplayManager_DisplayInfo displayItem = displaysInfo[i];
        if (displayItem.colorSpace != nullptr) {
            if (displayItem.colorSpace->colorSpaces != nullptr) {
                DISPLAY_MANAGER_FREE_MEMORY(displayItem.colorSpace->colorSpaces);
            }
            DISPLAY_MANAGER_FREE_MEMORY(displayItem.colorSpace);
        }
        if (displayItem.hdrFormat != nullptr) {
            if (displayItem.hdrFormat->hdrFormats != nullptr) {
                DISPLAY_MANAGER_FREE_MEMORY(displayItem.hdrFormat->hdrFormats);
            }
            DISPLAY_MANAGER_FREE_MEMORY(displayItem.hdrFormat);
        }
    }
    DISPLAY_MANAGER_FREE_MEMORY(displaysInfo);
}

NativeDisplayManager_ErrorCode OH_NativeDisplayManager_CreateAllDisplays(
    NativeDisplayManager_DisplaysInfo **allDisplays)
{
    if (allDisplays == nullptr) {
        TLOGE(WmsLogTag::DMS, "[DMNDK] param is null.");
        return NativeDisplayManager_ErrorCode::DISPLAY_MANAGER_ERROR_INVALID_PARAM;
    }
    /* displays is not null make sure by GetAllDisplays*/
    std::vector<sptr<Display>> displays = DisplayManager::GetInstance().GetAllDisplays();
    if (displays.empty()) {
        TLOGE(WmsLogTag::DMS, "[DMNDK] displays is empty.");
        return NativeDisplayManager_ErrorCode::DISPLAY_MANAGER_ERROR_SYSTEM_ABNORMAL;
    }
    NativeDisplayManager_DisplaysInfo *displaysInner =
        (NativeDisplayManager_DisplaysInfo*)malloc(sizeof(NativeDisplayManager_DisplaysInfo));
    if (displaysInner == nullptr) {
        TLOGE(WmsLogTag::DMS, "[DMNDK] malloc displays inner failed.");
        return NativeDisplayManager_ErrorCode::DISPLAY_MANAGER_ERROR_SYSTEM_ABNORMAL;
    }
    int32_t retMemset = memset_s(displaysInner, sizeof(NativeDisplayManager_DisplaysInfo), 0,
        sizeof(NativeDisplayManager_DisplaysInfo));
    if (retMemset != EOK) {
        TLOGE(WmsLogTag::DMS, "[DMNDK] memset displays failed.");
        DISPLAY_MANAGER_FREE_MEMORY(displaysInner);
        return NativeDisplayManager_ErrorCode::DISPLAY_MANAGER_ERROR_SYSTEM_ABNORMAL;
    }
    size_t displaySize = displays.size();
    displaysInner->displaysLength = static_cast<uint32_t>(displaySize);
    NativeDisplayManager_DisplayInfo *displaysInfo =
        (NativeDisplayManager_DisplayInfo*)malloc(sizeof(NativeDisplayManager_DisplayInfo) * displaySize);
    if (displaysInfo == nullptr) {
        TLOGE(WmsLogTag::DMS, "[DMNDK] malloc displaysInfo failed.");
        DISPLAY_MANAGER_FREE_MEMORY(displaysInner);
        return NativeDisplayManager_ErrorCode::DISPLAY_MANAGER_ERROR_SYSTEM_ABNORMAL;
    }
    retMemset = memset_s(displaysInfo, sizeof(NativeDisplayManager_DisplayInfo) * displaySize, 0,
        sizeof(NativeDisplayManager_DisplayInfo) * displaySize);
    NativeDisplayManager_ErrorCode setRet = NativeDisplayManager_ErrorCode::DISPLAY_MANAGER_OK;
    if (retMemset == EOK) {
        setRet = NativeDisplayManager_SetDisplaysInfo(displays, displaysInfo);
    }
    if (retMemset != EOK || setRet != NativeDisplayManager_ErrorCode::DISPLAY_MANAGER_OK) {
        TLOGE(WmsLogTag::DMS, "[DMNDK] memset or set displaysInfo failed setRet=%{public}d.", setRet);
        NativeDisplayManager_DestroyDisplaysInfoInner(displaysInner->displaysLength, displaysInfo);
        DISPLAY_MANAGER_FREE_MEMORY(displaysInner);
        return NativeDisplayManager_ErrorCode::DISPLAY_MANAGER_ERROR_SYSTEM_ABNORMAL;
    }
    displaysInner->displaysInfo = displaysInfo;
    *allDisplays = displaysInner;
    return setRet;
}

void OH_NativeDisplayManager_DestroyAllDisplays(NativeDisplayManager_DisplaysInfo *allDisplays)
{
    if (allDisplays == nullptr) {
        TLOGI(WmsLogTag::DMS, "[DMNDK] param is null.");
        return;
    }
    if (allDisplays->displaysInfo == nullptr) {
        DISPLAY_MANAGER_FREE_MEMORY(allDisplays);
        return;
    }
    NativeDisplayManager_DestroyDisplaysInfoInner(allDisplays->displaysLength, allDisplays->displaysInfo);
    DISPLAY_MANAGER_FREE_MEMORY(allDisplays);
}

static NativeDisplayManager_DisplayInfo* NativeDisplayManager_FillDisplayInfo(sptr<Display> display,
    NativeDisplayManager_ErrorCode *errCode)
{
    sptr<DisplayInfo> info = display->GetDisplayInfo();
    if (info == nullptr) {
        TLOGE(WmsLogTag::DMS, "[DMNDK] get display info null.");
        *errCode = NativeDisplayManager_ErrorCode::DISPLAY_MANAGER_ERROR_SYSTEM_ABNORMAL;
        return nullptr;
    }
    NativeDisplayManager_DisplayInfo *displayInner =
        (NativeDisplayManager_DisplayInfo*)malloc(sizeof(NativeDisplayManager_DisplayInfo));
    if (displayInner == nullptr) {
        TLOGE(WmsLogTag::DMS, "[DMNDK] malloc display info null.");
        *errCode = NativeDisplayManager_ErrorCode::DISPLAY_MANAGER_ERROR_SYSTEM_ABNORMAL;
        return nullptr;
    }
    auto retMemset = memset_s(displayInner, sizeof(NativeDisplayManager_DisplayInfo), 0,
        sizeof(NativeDisplayManager_DisplayInfo));
    if (retMemset != EOK) {
        TLOGE(WmsLogTag::DMS, "[DMNDK] memset display info null.");
        DISPLAY_MANAGER_FREE_MEMORY(displayInner);
        *errCode = NativeDisplayManager_ErrorCode::DISPLAY_MANAGER_ERROR_SYSTEM_ABNORMAL;
        return nullptr;
    }
    int ret = memcpy_s(displayInner->name, sizeof(displayInner->name) - 1, info->GetName().c_str(),
                       info->GetName().size());
    if (ret != EOK) {
        TLOGE(WmsLogTag::DMS, "[DMNDK] memcpy display name failed.");
        DISPLAY_MANAGER_FREE_MEMORY(displayInner);
        *errCode = NativeDisplayManager_ErrorCode::DISPLAY_MANAGER_ERROR_SYSTEM_ABNORMAL;
        return nullptr;
    }
    NativeDisplayManager_SetDisplayInfo(displayInner, info);
    *errCode = NativeDisplayManager_ErrorCode::DISPLAY_MANAGER_OK;
    return displayInner;
}

NativeDisplayManager_ErrorCode OH_NativeDisplayManager_CreateDisplayById(uint32_t id,
    NativeDisplayManager_DisplayInfo **displayInfo)
{
    if (displayInfo == nullptr) {
        TLOGE(WmsLogTag::DMS, "[DMNDK] param is null.");
        return NativeDisplayManager_ErrorCode::DISPLAY_MANAGER_ERROR_INVALID_PARAM;
    }
    sptr<Display> display = DisplayManager::GetInstance().GetDisplayById(static_cast<DisplayId>(id));
    if (display == nullptr) {
        TLOGE(WmsLogTag::DMS, "[DMNDK] get display by id null.");
        return NativeDisplayManager_ErrorCode::DISPLAY_MANAGER_ERROR_SYSTEM_ABNORMAL;
    }
    TLOGI(WmsLogTag::DMS, "[DMNDK] get display id[%{public}" PRIu64"] info", display->GetId());
    NativeDisplayManager_ErrorCode errorCode = NativeDisplayManager_ErrorCode::DISPLAY_MANAGER_OK;
    *displayInfo = NativeDisplayManager_FillDisplayInfo(display, &errorCode);
    return errorCode;
}

NativeDisplayManager_ErrorCode OH_NativeDisplayManager_CreatePrimaryDisplay(
    NativeDisplayManager_DisplayInfo **displayInfo)
{
    if (displayInfo == nullptr) {
        TLOGE(WmsLogTag::DMS, "[DMNDK] param is null.");
        return NativeDisplayManager_ErrorCode::DISPLAY_MANAGER_ERROR_INVALID_PARAM;
    }
    sptr<Display> display = DisplayManager::GetInstance().GetPrimaryDisplaySync();
    if (display == nullptr) {
        TLOGE(WmsLogTag::DMS, "[DMNDK] get primary display is null.");
        return NativeDisplayManager_ErrorCode::DISPLAY_MANAGER_ERROR_SYSTEM_ABNORMAL;
    }
    TLOGI(WmsLogTag::DMS, "[DMNDK] get primary display id[%{public}" PRIu64"].", display->GetId());
    NativeDisplayManager_ErrorCode errorCode = NativeDisplayManager_ErrorCode::DISPLAY_MANAGER_OK;
    *displayInfo = NativeDisplayManager_FillDisplayInfo(display, &errorCode);
    return errorCode;
}

void OH_NativeDisplayManager_DestroyDisplay(NativeDisplayManager_DisplayInfo *displayInfo)
{
    if (displayInfo == nullptr) {
        TLOGE(WmsLogTag::DMS, "[DMNDK] free display info is null.");
        return;
    }
    if (displayInfo->colorSpace != nullptr) {
        if (displayInfo->colorSpace->colorSpaces != nullptr) {
            DISPLAY_MANAGER_FREE_MEMORY(displayInfo->colorSpace->colorSpaces);
        }
        DISPLAY_MANAGER_FREE_MEMORY(displayInfo->colorSpace);
    }
    if (displayInfo->hdrFormat != nullptr) {
        if (displayInfo->hdrFormat->hdrFormats != nullptr) {
            DISPLAY_MANAGER_FREE_MEMORY(displayInfo->hdrFormat->hdrFormats);
        }
        DISPLAY_MANAGER_FREE_MEMORY(displayInfo->hdrFormat);
    }
    DISPLAY_MANAGER_FREE_MEMORY(displayInfo);
}

NativeDisplayManager_ErrorCode OH_NativeDisplayManager_CaptureScreenPixelmap(uint32_t displayId,
    OH_PixelmapNative **pixelMap)
{
    if (pixelMap == nullptr) {
        TLOGE(WmsLogTag::DMS, "[DMNDK] pixelMap is null.");
        return NativeDisplayManager_ErrorCode::DISPLAY_MANAGER_ERROR_INVALID_PARAM;
    }
    CaptureOption option;
    option.displayId_ = static_cast<DisplayId>(displayId);
    option.isNeedNotify_ = true;
    DmErrorCode errCode = DmErrorCode::DM_OK;
    std::shared_ptr<Media::PixelMap> captureImage = DisplayManager::GetInstance().GetScreenCapture(option, &errCode);

    if (errCode == DmErrorCode::DM_ERROR_INVALID_PARAM) {
        TLOGE(WmsLogTag::DMS, "[DMNDK] param error.");
        return NativeDisplayManager_ErrorCode::DISPLAY_MANAGER_ERROR_INVALID_PARAM;
    }
    if (errCode == DmErrorCode::DM_ERROR_DEVICE_NOT_SUPPORT) {
        TLOGE(WmsLogTag::DMS, "[DMNDK] not support.");
        return NativeDisplayManager_ErrorCode::DISPLAY_MANAGER_ERROR_DEVICE_NOT_SUPPORTED;
    }
    if (errCode == DmErrorCode::DM_ERROR_NO_PERMISSION) {
        TLOGE(WmsLogTag::DMS, "[DMNDK] pixelMap no permission.");
        return NativeDisplayManager_ErrorCode::DISPLAY_MANAGER_ERROR_NO_PERMISSION;
    }
    if (captureImage == nullptr) {
        TLOGE(WmsLogTag::DMS, "[DMNDK] pixelMap is null.");
        return NativeDisplayManager_ErrorCode::DISPLAY_MANAGER_ERROR_SYSTEM_ABNORMAL;
    }
    *pixelMap = new OH_PixelmapNative(captureImage);
    if (*pixelMap == nullptr) {
        TLOGE(WmsLogTag::DMS, "[DMNDK] pixelMap convert pixelMapNative null.");
        return NativeDisplayManager_ErrorCode::DISPLAY_MANAGER_ERROR_SYSTEM_ABNORMAL;
    }
    TLOGI(WmsLogTag::DMS, "[DMNDK] get screen capture end.");
    return NativeDisplayManager_ErrorCode::DISPLAY_MANAGER_OK;
}

NativeDisplayManager_ErrorCode OH_NativeDisplayManager_RegisterAvailableAreaChangeListener(
    OH_NativeDisplayManager_AvailableAreaChangeCallback availableAreaChangeCallback, uint32_t* listenerIndex)
{
    if (availableAreaChangeCallback == nullptr || listenerIndex == nullptr) {
        TLOGE(WmsLogTag::DMS, "[DMNDK] input params null.");
        return NativeDisplayManager_ErrorCode::DISPLAY_MANAGER_ERROR_ILLEGAL_PARAM;
    }
    sptr<AvailableAreaChangeListener> availableAreaChangeListener =
        sptr<AvailableAreaChangeListener>::MakeSptr(availableAreaChangeCallback);
    if (CheckAvailableAreaChangeHasRegistered(availableAreaChangeListener)) {
        TLOGE(WmsLogTag::DMS, "[DMNDK] input params error (has registered).");
        return NativeDisplayManager_ErrorCode::DISPLAY_MANAGER_OK;
    }
    sptr<DisplayManager::IAvailableAreaListener> displayAvailableAreaListener = availableAreaChangeListener;

    static std::atomic<uint32_t> registerCount = 1;
    DMError ret = DisplayManager::GetInstance().RegisterAvailableAreaListener(displayAvailableAreaListener);
    if (ret != DMError::DM_OK) {
        TLOGE(WmsLogTag::DMS, "[DMNDK] register failed ret=%{public}d.", ret);
        return NativeDisplayManager_ErrorCode::DISPLAY_MANAGER_ERROR_SYSTEM_ABNORMAL;
    }
    *listenerIndex = registerCount++;
    std::unique_lock<std::shared_mutex> lock(availableAreaChangeMutex);
    availableAreaChangeListenerMap.emplace(*listenerIndex, availableAreaChangeListener);
    TLOGI(WmsLogTag::DMS, "[DMNDK] register listenerIndex=%{public}d.", *listenerIndex);
    return NativeDisplayManager_ErrorCode::DISPLAY_MANAGER_OK;
}

NativeDisplayManager_ErrorCode OH_NativeDisplayManager_UnregisterAvailableAreaChangeListener(uint32_t listenerIndex)
{
    std::unique_lock<std::shared_mutex> lock(availableAreaChangeMutex);
    auto iter = availableAreaChangeListenerMap.find(listenerIndex);
    if (iter == availableAreaChangeListenerMap.end()) {
        TLOGE(WmsLogTag::DMS, "[DMNDK] unregister listener fail(not find register info).");
        return NativeDisplayManager_ErrorCode::DISPLAY_MANAGER_ERROR_ILLEGAL_PARAM;
    }
    DMError ret = DMError::DM_OK;
    if (iter->second != nullptr) {
        ret = DisplayManager::GetInstance().UnregisterAvailableAreaListener(iter->second);
        availableAreaChangeListenerMap.erase(listenerIndex);
        TLOGI(WmsLogTag::DMS, "[DMNDK] unregister listener ret=%{public}d", ret);
    }
    return ret == DMError::DM_OK ? NativeDisplayManager_ErrorCode::DISPLAY_MANAGER_OK :
        NativeDisplayManager_ErrorCode::DISPLAY_MANAGER_ERROR_SYSTEM_ABNORMAL;
}

NativeDisplayManager_ErrorCode OH_NativeDisplayManager_RegisterDisplayAddListener(
    OH_NativeDisplayManager_DisplayAddCallback displayAddCallback, uint32_t* listenerIndex)
{
    if (displayAddCallback == nullptr || listenerIndex == nullptr) {
        TLOGE(WmsLogTag::DMS, "[DMNDK] register fail(input params null).");
        return NativeDisplayManager_ErrorCode::DISPLAY_MANAGER_ERROR_ILLEGAL_PARAM;
    }
    sptr<DisplayAddListener> displayAddListener =
        sptr<DisplayAddListener>::MakeSptr(displayAddCallback);
    if (CheckDisplayAddHasRegistered(displayAddListener)) {
        TLOGE(WmsLogTag::DMS, "[DMNDK] input params error (has registered).");
        return NativeDisplayManager_ErrorCode::DISPLAY_MANAGER_OK;
    }
    sptr<DisplayManager::IDisplayListener> displayListener = displayAddListener;

    static std::atomic<uint32_t> registerCount = 1;
    DMError ret = DisplayManager::GetInstance().RegisterDisplayListener(displayListener);
    if (ret != DMError::DM_OK) {
        TLOGE(WmsLogTag::DMS, "[DMNDK] register failed ret=%{public}d.", ret);
        return NativeDisplayManager_ErrorCode::DISPLAY_MANAGER_ERROR_SYSTEM_ABNORMAL;
    }
    *listenerIndex = registerCount++;
    std::unique_lock<std::shared_mutex> lock(displayAddMutex);
    displayAddListenerMap.emplace(*listenerIndex, displayAddListener);
    TLOGI(WmsLogTag::DMS, "[DMNDK] register listenerIndex= %{public}d.", *listenerIndex);
    return NativeDisplayManager_ErrorCode::DISPLAY_MANAGER_OK;
}

NativeDisplayManager_ErrorCode OH_NativeDisplayManager_UnregisterDisplayAddListener(uint32_t listenerIndex)
{
    std::unique_lock<std::shared_mutex> lock(displayAddMutex);
    auto iter = displayAddListenerMap.find(listenerIndex);
    if (iter == displayAddListenerMap.end()) {
        TLOGE(WmsLogTag::DMS, "[DMNDK] unregister fail(not find register info).");
        return NativeDisplayManager_ErrorCode::DISPLAY_MANAGER_ERROR_ILLEGAL_PARAM;
    }
    DMError ret = DMError::DM_OK;
    if (iter->second != nullptr) {
        ret = DisplayManager::GetInstance().UnregisterDisplayListener(iter->second);
        displayAddListenerMap.erase(listenerIndex);
        TLOGI(WmsLogTag::DMS, "[DMNDK] unregister listener ret=%{public}d", ret);
    }
    return ret == DMError::DM_OK ? NativeDisplayManager_ErrorCode::DISPLAY_MANAGER_OK :
        NativeDisplayManager_ErrorCode::DISPLAY_MANAGER_ERROR_SYSTEM_ABNORMAL;
}

NativeDisplayManager_ErrorCode OH_NativeDisplayManager_RegisterDisplayRemoveListener(
    OH_NativeDisplayManager_DisplayRemoveCallback displayRemoveCallback, uint32_t* listenerIndex)
{
    if (displayRemoveCallback == nullptr || listenerIndex == nullptr) {
        TLOGE(WmsLogTag::DMS, "[DMNDK] register fail(input params null).");
        return NativeDisplayManager_ErrorCode::DISPLAY_MANAGER_ERROR_ILLEGAL_PARAM;
    }
    sptr<DisplayRemoveListener> displayRemoveListener =
        sptr<DisplayRemoveListener>::MakeSptr(displayRemoveCallback);
    if (CheckDisplayRemoveHasRegistered(displayRemoveListener)) {
        TLOGE(WmsLogTag::DMS, "[DMNDK] input params error (has registered).");
        return NativeDisplayManager_ErrorCode::DISPLAY_MANAGER_OK;
    }
    sptr<DisplayManager::IDisplayListener> displayListener = displayRemoveListener;

    static std::atomic<uint32_t> registerCount = 1;
    DMError ret = DisplayManager::GetInstance().RegisterDisplayListener(displayListener);
    if (ret != DMError::DM_OK) {
        TLOGE(WmsLogTag::DMS, "[DMNDK] register failed ret=%{public}d.", ret);
        return NativeDisplayManager_ErrorCode::DISPLAY_MANAGER_ERROR_SYSTEM_ABNORMAL;
    }
    *listenerIndex = registerCount++;
    std::unique_lock<std::shared_mutex> lock(displayRemoveMutex);
    displayRemoveListenerMap.emplace(*listenerIndex, displayRemoveListener);
    TLOGI(WmsLogTag::DMS, "[DMNDK] register listenerIndex= %{public}d.", *listenerIndex);
    return NativeDisplayManager_ErrorCode::DISPLAY_MANAGER_OK;
}

NativeDisplayManager_ErrorCode OH_NativeDisplayManager_UnregisterDisplayRemoveListener(uint32_t listenerIndex)
{
    std::unique_lock<std::shared_mutex> lock(displayRemoveMutex);
    auto iter = displayRemoveListenerMap.find(listenerIndex);
    if (iter == displayRemoveListenerMap.end()) {
        TLOGE(WmsLogTag::DMS, "[DMNDK] unregister fail(not find register info).");
        return NativeDisplayManager_ErrorCode::DISPLAY_MANAGER_ERROR_ILLEGAL_PARAM;
    }
    DMError ret = DMError::DM_OK;
    if (iter->second != nullptr) {
        ret = DisplayManager::GetInstance().UnregisterDisplayListener(iter->second);
        displayRemoveListenerMap.erase(listenerIndex);
        TLOGI(WmsLogTag::DMS, "[DMNDK] unregister listener ret=%{public}d", ret);
    }
    return ret == DMError::DM_OK ? NativeDisplayManager_ErrorCode::DISPLAY_MANAGER_OK :
        NativeDisplayManager_ErrorCode::DISPLAY_MANAGER_ERROR_SYSTEM_ABNORMAL;
}

NativeDisplayManager_ErrorCode OH_NativeDisplayManager_CreateAvailableArea(
    uint64_t displayId, NativeDisplayManager_Rect** availableArea)
{
    if (availableArea == nullptr) {
        TLOGE(WmsLogTag::DMS, "[DMNDK] input availableArea null.");
        return NativeDisplayManager_ErrorCode::DISPLAY_MANAGER_ERROR_ILLEGAL_PARAM;
    }
    int64_t displayCheck = static_cast<int64_t>(displayId);
    if (displayCheck < 0) {
        TLOGE(WmsLogTag::DMS, "[DMNDK] input display illegal.");
        return NativeDisplayManager_ErrorCode::DISPLAY_MANAGER_ERROR_ILLEGAL_PARAM;
    }
    sptr<Display> display = DisplayManager::GetInstance().GetDisplayById(static_cast<DisplayId>(displayId));
    if (display == nullptr) {
        TLOGE(WmsLogTag::DMS, "[DMNDK] display is  null, id %{public}" PRIu64" ", displayId);
        return NativeDisplayManager_ErrorCode::DISPLAY_MANAGER_ERROR_SYSTEM_ABNORMAL;
    }
    NativeDisplayManager_Rect* availableAreaInfo =
        static_cast<NativeDisplayManager_Rect*>(malloc(sizeof(NativeDisplayManager_Rect)));
    if (availableAreaInfo == NULL) {
        TLOGE(WmsLogTag::DMS, "[DMNDK] memory failed.");
        return NativeDisplayManager_ErrorCode::DISPLAY_MANAGER_ERROR_SYSTEM_ABNORMAL;
    }

    auto retMemset = memset_s(availableAreaInfo, sizeof(NativeDisplayManager_Rect), 0,
        sizeof(NativeDisplayManager_Rect));
    if (retMemset != EOK) {
        free(availableAreaInfo);
        availableAreaInfo = nullptr;
        TLOGE(WmsLogTag::DMS, "[DMNDK] memset failed.");
        return NativeDisplayManager_ErrorCode::DISPLAY_MANAGER_ERROR_SYSTEM_ABNORMAL;
    }

    DMRect displayAvailableArea = DMRect::NONE();
    display->GetAvailableArea(displayAvailableArea);
    TLOGI(WmsLogTag::DMS, "[DMNDK] posX_=%{public}d posY_=%{public}d width_=%{public}d height_=%{public}d",
        displayAvailableArea.posX_, displayAvailableArea.posY_,
        displayAvailableArea.width_, displayAvailableArea.height_);
    OH_SetDisplayRect(displayAvailableArea, availableAreaInfo);
    *availableArea = availableAreaInfo;
    return NativeDisplayManager_ErrorCode::DISPLAY_MANAGER_OK;
}

NativeDisplayManager_ErrorCode OH_NativeDisplayManager_DestroyAvailableArea(NativeDisplayManager_Rect* availableArea)
{
    if (availableArea == nullptr) {
        TLOGE(WmsLogTag::DMS, "[DMNDK] input availableArea null.");
        return NativeDisplayManager_ErrorCode::DISPLAY_MANAGER_ERROR_ILLEGAL_PARAM;
    }
    free(availableArea);
    availableArea = nullptr;
    TLOGI(WmsLogTag::DMS, "[DMNDK] destroy availableArea end");
    return  NativeDisplayManager_ErrorCode::DISPLAY_MANAGER_OK;
}

NativeDisplayManager_ErrorCode OH_NativeDisplayManager_GetDisplaySourceMode(
    uint64_t displayId, NativeDisplayManager_SourceMode* sourceMode)
{
    if (sourceMode == nullptr) {
        TLOGE(WmsLogTag::DMS, "[DMNDK] input sourceMode null.");
        return NativeDisplayManager_ErrorCode::DISPLAY_MANAGER_ERROR_ILLEGAL_PARAM;
    }
    int64_t displayCheck = static_cast<int64_t>(displayId);
    if (displayCheck < 0) {
        TLOGE(WmsLogTag::DMS, "[DMNDK] input display illegal.");
        return NativeDisplayManager_ErrorCode::DISPLAY_MANAGER_ERROR_ILLEGAL_PARAM;
    }
    sptr<Display> display = DisplayManager::GetInstance().GetDisplayById(static_cast<DisplayId>(displayId));
    if (display == nullptr) {
        TLOGE(WmsLogTag::DMS, "[DMNDK] display is  null, id %{public}" PRIu64" ", displayId);
        return NativeDisplayManager_ErrorCode::DISPLAY_MANAGER_ERROR_SYSTEM_ABNORMAL;
    }
    sptr<DisplayInfo> displayInfo = display->GetDisplayInfo();
    if (displayInfo == nullptr) {
        TLOGE(WmsLogTag::DMS, "[DMNDK] get displayInfo null.");
        return NativeDisplayManager_ErrorCode::DISPLAY_MANAGER_ERROR_SYSTEM_ABNORMAL;
    }
    DisplaySourceMode getSourceMode = displayInfo->GetDisplaySourceMode();
    TLOGI(WmsLogTag::DMS, "[DMNDK] getSourceMode = %{public}d", getSourceMode);
    *sourceMode = static_cast<NativeDisplayManager_SourceMode>(getSourceMode);
    return NativeDisplayManager_ErrorCode::DISPLAY_MANAGER_OK;
}

NativeDisplayManager_ErrorCode OH_NativeDisplayManager_GetDisplayPosition(uint64_t displayId, int32_t* x, int32_t* y)
{
    if (x == nullptr || y == nullptr) {
        TLOGE(WmsLogTag::DMS, "[DMNDK] input x or y is null.");
        return NativeDisplayManager_ErrorCode::DISPLAY_MANAGER_ERROR_ILLEGAL_PARAM;
    }
    int64_t displayCheck = static_cast<int64_t>(displayId);
    if (displayCheck < 0) {
        TLOGE(WmsLogTag::DMS, "[DMNDK] input display illegal.");
        return NativeDisplayManager_ErrorCode::DISPLAY_MANAGER_ERROR_ILLEGAL_PARAM;
    }
    sptr<Display> display = DisplayManager::GetInstance().GetDisplayById(static_cast<DisplayId>(displayId));
    if (display == nullptr) {
        TLOGE(WmsLogTag::DMS, "[DMNDK] display is  null, id %{public}" PRIu64" ", displayId);
        return NativeDisplayManager_ErrorCode::DISPLAY_MANAGER_ERROR_SYSTEM_ABNORMAL;
    }
    sptr<DisplayInfo> displayInfo = display->GetDisplayInfo();
    if (displayInfo == nullptr) {
        TLOGE(WmsLogTag::DMS, "[DMNDK] get displayInfo null.");
        return NativeDisplayManager_ErrorCode::DISPLAY_MANAGER_ERROR_SYSTEM_ABNORMAL;
    }
    DisplaySourceMode getSourceMode = displayInfo->GetDisplaySourceMode();
    if (getSourceMode == DisplaySourceMode::MAIN || getSourceMode == DisplaySourceMode::EXTEND) {
        *x = displayInfo->GetX();
        *y = displayInfo->GetY();
        return NativeDisplayManager_ErrorCode::DISPLAY_MANAGER_OK;
    } else {
        TLOGE(WmsLogTag::DMS, "[DMNDK] just main and extend has x, y.");
        return NativeDisplayManager_ErrorCode::DISPLAY_MANAGER_ERROR_ILLEGAL_PARAM;
    }
}