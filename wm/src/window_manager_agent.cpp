/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#include "window_manager_agent.h"
#include "foundation/window/window_manager/interfaces/innerkits/wm/window_manager.h"
#include "window_manager_hilog.h"
#include "wm_common.h"

namespace OHOS {
namespace Rosen {
WindowManagerAgent::WindowManagerAgent(const int32_t userId) : userId_(userId) {}

void WindowManagerAgent::UpdateFocusChangeInfo(const sptr<FocusChangeInfo>& focusChangeInfo, bool focused)
{
    TLOGD(WmsLogTag::WMS_FOCUS, "userId_=%{public}d", userId_);
    WindowManager::GetInstance(userId_).UpdateFocusChangeInfo(focusChangeInfo, focused);
}

void WindowManagerAgent::UpdateWindowModeTypeInfo(WindowModeType type)
{
    TLOGD(WmsLogTag::WMS_SYSTEM, "userId_=%{public}d", userId_);
    WindowManager::GetInstance(userId_).UpdateWindowModeTypeInfo(type);
}

void WindowManagerAgent::UpdateSystemBarRegionTints(DisplayId displayId, const SystemBarRegionTints& tints)
{
    TLOGD(WmsLogTag::WMS_SYSTEM, "userId_=%{public}d", userId_);
    WindowManager::GetInstance(userId_).UpdateSystemBarRegionTints(displayId, tints);
}

void WindowManagerAgent::NotifyAccessibilityWindowInfo(const std::vector<sptr<AccessibilityWindowInfo>>& infos,
    WindowUpdateType type)
{
    TLOGD(WmsLogTag::WMS_MAIN, "userId_=%{public}d", userId_);
    WindowManager::GetInstance(userId_).NotifyAccessibilityWindowInfo(infos, type);
}

void WindowManagerAgent::UpdateWindowVisibilityInfo(const std::vector<sptr<WindowVisibilityInfo>>& visibilityInfos)
{
    TLOGD(WmsLogTag::WMS_ATTRIBUTE, "userId_=%{public}d", userId_);
    WindowManager::GetInstance(userId_).UpdateWindowVisibilityInfo(visibilityInfos);
}

void WindowManagerAgent::UpdateWindowDrawingContentInfo(
    const std::vector<sptr<WindowDrawingContentInfo>>& windowDrawingContentInfos)
{
    TLOGD(WmsLogTag::WMS_SYSTEM, "userId_=%{public}d", userId_);
    WindowManager::GetInstance(userId_).UpdateWindowDrawingContentInfo(windowDrawingContentInfos);
}

void WindowManagerAgent::UpdateCameraFloatWindowStatus(uint32_t accessTokenId, bool isShowing)
{
    TLOGD(WmsLogTag::WMS_SYSTEM, "userId_=%{public}d", userId_);
    WindowManager::GetInstance(userId_).UpdateCameraFloatWindowStatus(accessTokenId, isShowing);
}

void WindowManagerAgent::NotifyWaterMarkFlagChangedResult(bool showWaterMark)
{
    TLOGD(WmsLogTag::DEFAULT, "userId_=%{public}d", userId_);
    WindowManager::GetInstance(userId_).NotifyWaterMarkFlagChangedResult(showWaterMark);
}

void WindowManagerAgent::UpdateVisibleWindowNum(
    const std::vector<VisibleWindowNumInfo>& visibleWindowNumInfo)
{
    TLOGD(WmsLogTag::WMS_MAIN, "userId_=%{public}d", userId_);
    WindowManager::GetInstance(userId_).UpdateVisibleWindowNum(visibleWindowNumInfo);
}

void WindowManagerAgent::NotifyGestureNavigationEnabledResult(bool enable)
{
    TLOGD(WmsLogTag::DEFAULT, "userId_=%{public}d", userId_);
    WindowManager::GetInstance(userId_).NotifyGestureNavigationEnabledResult(enable);
}

void WindowManagerAgent::NotifyWindowStyleChange(WindowStyleType type)
{
    TLOGD(WmsLogTag::WMS_MAIN, "userId_=%{public}d", userId_);
    WindowManager::GetInstance(userId_).NotifyWindowStyleChange(type);
}

void WindowManagerAgent::NotifyWindowSystemBarPropertyChange(
    WindowType type, const SystemBarProperty& systemBarProperty)
{
    TLOGD(WmsLogTag::WMS_IMMS, "userId_=%{public}d", userId_);
    WindowManager::GetInstance(userId_).NotifyWindowSystemBarPropertyChange(type, systemBarProperty);
}

void WindowManagerAgent::NotifyWindowPidVisibilityChanged(const sptr<WindowPidVisibilityInfo>& info)
{
    TLOGD(WmsLogTag::WMS_LIFE, "userId_=%{public}d", userId_);
    WindowManager::GetInstance(userId_).NotifyWindowPidVisibilityChanged(info);
}

void WindowManagerAgent::NotifyWindowPropertyChange(uint32_t propertyDirtyFlags,
    const WindowInfoList& windowInfoList)
{
    TLOGD(WmsLogTag::WMS_ATTRIBUTE, "userId_=%{public}d", userId_);
    WindowManager::GetInstance().NotifyWindowPropertyChange(propertyDirtyFlags, windowInfoList);
}

void WindowManagerAgent::NotifySupportRotationChange(const SupportRotationInfo& supportRotationInfo)
{
    TLOGD(WmsLogTag::WMS_ROTATION, "userId_=%{public}d", userId_);
    WindowManager::GetInstance(userId_).NotifySupportRotationChange(supportRotationInfo);
}
} // namespace Rosen
} // namespace OHOS
