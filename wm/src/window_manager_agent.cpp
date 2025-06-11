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

#include "window_manager_agent.h"
#include "foundation/window/window_manager/interfaces/innerkits/wm/window_manager.h"
#include "singleton_container.h"
#include "wm_common.h"

namespace OHOS {
namespace Rosen {
void WindowManagerAgent::UpdateFocusChangeInfo(const sptr<FocusChangeInfo>& focusChangeInfo, bool focused)
{
    SingletonContainer::Get<WindowManager>().UpdateFocusChangeInfo(focusChangeInfo, focused);
}

void WindowManagerAgent::UpdateWindowModeTypeInfo(WindowModeType type)
{
    SingletonContainer::Get<WindowManager>().UpdateWindowModeTypeInfo(type);
}

void WindowManagerAgent::UpdateSystemBarRegionTints(DisplayId displayId, const SystemBarRegionTints& tints)
{
    SingletonContainer::Get<WindowManager>().UpdateSystemBarRegionTints(displayId, tints);
}

void WindowManagerAgent::NotifyAccessibilityWindowInfo(const std::vector<sptr<AccessibilityWindowInfo>>& infos,
    WindowUpdateType type)
{
    SingletonContainer::Get<WindowManager>().NotifyAccessibilityWindowInfo(infos, type);
}

void WindowManagerAgent::UpdateWindowVisibilityInfo(const std::vector<sptr<WindowVisibilityInfo>>& visibilityInfos)
{
    SingletonContainer::Get<WindowManager>().UpdateWindowVisibilityInfo(visibilityInfos);
}

void WindowManagerAgent::UpdateWindowDrawingContentInfo(
    const std::vector<sptr<WindowDrawingContentInfo>>& windowDrawingContentInfos)
{
    SingletonContainer::Get<WindowManager>().UpdateWindowDrawingContentInfo(windowDrawingContentInfos);
}

void WindowManagerAgent::UpdateCameraFloatWindowStatus(uint32_t accessTokenId, bool isShowing)
{
    SingletonContainer::Get<WindowManager>().UpdateCameraFloatWindowStatus(accessTokenId, isShowing);
}

void WindowManagerAgent::NotifyWaterMarkFlagChangedResult(bool showWaterMark)
{
    SingletonContainer::Get<WindowManager>().NotifyWaterMarkFlagChangedResult(showWaterMark);
}

void WindowManagerAgent::UpdateVisibleWindowNum(
    const std::vector<VisibleWindowNumInfo>& visibleWindowNumInfo)
{
    SingletonContainer::Get<WindowManager>().UpdateVisibleWindowNum(visibleWindowNumInfo);
}

void WindowManagerAgent::NotifyGestureNavigationEnabledResult(bool enable)
{
    SingletonContainer::Get<WindowManager>().NotifyGestureNavigationEnabledResult(enable);
}

void WindowManagerAgent::NotifyWindowStyleChange(WindowStyleType type)
{
    SingletonContainer::Get<WindowManager>().NotifyWindowStyleChange(type);
}

void WindowManagerAgent::NotifyWindowPidVisibilityChanged(const sptr<WindowPidVisibilityInfo>& info)
{
    SingletonContainer::Get<WindowManager>().NotifyWindowPidVisibilityChanged(info);
}

void WindowManagerAgent::NotifyWindowPropertyChange(uint32_t propertyDirtyFlags,
    const std::vector<std::unordered_map<WindowInfoKey, std::any>>& windowInfoList)
{
    SingletonContainer::Get<WindowManager>().NotifyWindowPropertyChange(propertyDirtyFlags, windowInfoList);
}
} // namespace Rosen
} // namespace OHOS
