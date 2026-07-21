/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

#include "display_manager_service_inner.h"

#include <cinttypes>
#include <iservice_registry.h>

#include "display_manager_service.h"
#include "window_manager_hilog.h"

namespace OHOS::Rosen {
WM_IMPLEMENT_SINGLE_INSTANCE(DisplayManagerServiceInner)

DisplayId DisplayManagerServiceInner::GetDefaultDisplayId() const
{
    auto defaultDisplayInfo = DisplayManagerService::GetInstance().GetDefaultDisplayInfo();
    if (defaultDisplayInfo == nullptr) {
        TLOGE(WmsLogTag::DMS, "GetDefaultDisplayId, defaultDisplayInfo is nullptr.");
        return DISPLAY_ID_INVALID;
    }
    return defaultDisplayInfo->GetDisplayId();
}

sptr<DisplayInfo> DisplayManagerServiceInner::GetDisplayById(DisplayId displayId) const
{
    sptr<DisplayInfo> display = DisplayManagerService::GetInstance().GetDisplayInfoById(displayId);
    if (display == nullptr) {
        TLOGE(WmsLogTag::DMS, "GetDisplayById can not find corresponding display!\n");
    }
    return display;
}

sptr<DisplayInfo> DisplayManagerServiceInner::GetDefaultDisplay() const
{
    return DisplayManagerService::GetInstance().GetDefaultDisplayInfo();
}

std::vector<DisplayId> DisplayManagerServiceInner::GetAllDisplayIds() const
{
    return DisplayManagerService::GetInstance().GetAllDisplayIds();
}

void DisplayManagerServiceInner::RegisterWindowInfoQueriedListener(const sptr<IWindowInfoQueriedListener>& listener)
{
    DisplayManagerService::GetInstance().RegisterWindowInfoQueriedListener(listener);
}

std::vector<sptr<DisplayInfo>> DisplayManagerServiceInner::GetAllDisplays() const
{
    std::vector<sptr<DisplayInfo>> res;
    auto displayIds = GetAllDisplayIds();
    for (auto displayId : displayIds) {
        sptr<DisplayInfo> display = DisplayManagerService::GetInstance().GetDisplayInfoById(displayId);
        if (display != nullptr) {
            res.emplace_back(display);
        } else {
            TLOGE(WmsLogTag::DMS, "GetAllDisplays display %" PRIu64" nullptr!", displayId);
        }
    }
    return res;
}

void DisplayManagerServiceInner::UpdateRSTree(DisplayId displayId, DisplayId parentDisplayId,
    std::shared_ptr<RSSurfaceNode>& surfaceNode, bool isAdd, bool isMultiDisplay)
{
    DisplayManagerService::GetInstance().UpdateRSTree(displayId, parentDisplayId, surfaceNode, isAdd, isMultiDisplay);
}

sptr<ScreenInfo> DisplayManagerServiceInner::GetScreenInfoByDisplayId(DisplayId displayId) const
{
    auto displayInfo = DisplayManagerService::GetInstance().GetDisplayInfoById(displayId);
    if (displayInfo == nullptr) {
        TLOGE(WmsLogTag::DMS, "can not get display.");
        return nullptr;
    }
    return DisplayManagerService::GetInstance().GetScreenInfoById(displayInfo->GetScreenId());
}

ScreenId DisplayManagerServiceInner::GetScreenGroupIdByDisplayId(DisplayId displayId) const
{
    auto displayInfo = DisplayManagerService::GetInstance().GetDisplayInfoById(displayId);
    if (displayInfo == nullptr) {
        TLOGE(WmsLogTag::DMS, "can not get display.");
        return INVALID_SCREEN_ID;
    }
    return DisplayManagerService::GetInstance().GetScreenGroupIdByScreenId(displayInfo->GetScreenId());
}

sptr<SupportedScreenModes> DisplayManagerServiceInner::GetScreenModesByDisplayId(DisplayId displayId) const
{
    const sptr<ScreenInfo> screenInfo = GetScreenInfoByDisplayId(displayId);
    if (screenInfo == nullptr) {
        TLOGE(WmsLogTag::DMS, "can not get display.");
        return nullptr;
    }
    auto modes = screenInfo->GetModes();
    auto id = screenInfo->GetModeId();
    if (id >= modes.size()) {
        TLOGE(WmsLogTag::DMS, "can not get screenMode.");
        return nullptr;
    }
    return modes[id];
}

std::shared_ptr<Media::PixelMap> DisplayManagerServiceInner::GetDisplaySnapshot(DisplayId displayId,
    DmErrorCode* errorCode, bool isUseDma, bool isCaptureFullOfScreen) const
{
    return DisplayManagerService::GetInstance().GetDisplaySnapshot(displayId, errorCode, isUseDma,
        isCaptureFullOfScreen);
}

void DisplayManagerServiceInner::RegisterDisplayChangeListener(sptr<IDisplayChangeListener> listener)
{
    DisplayManagerService::GetInstance().RegisterDisplayChangeListener(listener);
}

DMError DisplayManagerServiceInner::SetOrientationFromWindow(DisplayId displayId, Orientation orientation,
    bool withAnimation)
{
    auto displayInfo = GetDisplayById(displayId);
    if (displayInfo == nullptr) {
        return DMError::DM_ERROR_NULLPTR;
    }
    return DisplayManagerService::GetInstance().
        SetOrientationFromWindow(displayInfo->GetScreenId(), orientation, withAnimation);
}

bool DisplayManagerServiceInner::SetRotationFromWindow(DisplayId displayId, Rotation targetRotation, bool withAnimation)
{
    auto displayInfo = GetDisplayById(displayId);
    if (displayInfo == nullptr) {
        return false;
    }
    return DisplayManagerService::GetInstance().
        SetRotationFromWindow(displayInfo->GetScreenId(), targetRotation, withAnimation);
}

void DisplayManagerServiceInner::SetGravitySensorSubscriptionEnabled()
{
    DisplayManagerService::GetInstance().SetGravitySensorSubscriptionEnabled();
}

sptr<CutoutInfo> DisplayManagerServiceInner::GetCutoutInfo(DisplayId displayId) const
{
    return DisplayManagerService::GetInstance().GetCutoutInfo(displayId);
}

void DisplayManagerServiceInner::NotifyPrivateWindowStateChanged(bool hasPrivate)
{
    return DisplayManagerService::GetInstance().NotifyPrivateWindowStateChanged(hasPrivate);
}
} // namespace OHOS::Rosen