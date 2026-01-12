
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

#include "window_manager_agent_lite.h"
#include "foundation/window/window_manager/interfaces/innerkits/wm/window_manager_lite.h"
#include "singleton_container.h"
#include "wm_common.h"

namespace OHOS {
namespace Rosen {
void WindowManagerAgentLite::UpdateFocusChangeInfo(const sptr<FocusChangeInfo>& focusChangeInfo, bool focused)
{
    SingletonContainer::Get<WindowManagerLite>().UpdateFocusChangeInfo(focusChangeInfo, focused);
}

void WindowManagerAgentLite::UpdateDisplayGroupInfo(DisplayGroupId displayGroupId, DisplayId displayId, bool isAdd)
{
    SingletonContainer::Get<WindowManagerLite>().UpdateDisplayGroupInfo(displayGroupId, displayId, isAdd);
}

void WindowManagerAgentLite::UpdateWindowVisibilityInfo(const std::vector<sptr<WindowVisibilityInfo>>& visibilityInfos)
{
    SingletonContainer::Get<WindowManagerLite>().UpdateWindowVisibilityInfo(visibilityInfos);
}

void WindowManagerAgentLite::UpdateWindowDrawingContentInfo(
    const std::vector<sptr<WindowDrawingContentInfo>>& windowDrawingContentInfos)
{
    SingletonContainer::Get<WindowManagerLite>().UpdateWindowDrawingContentInfo(windowDrawingContentInfos);
}

void WindowManagerAgentLite::UpdateWindowModeTypeInfo(WindowModeType type)
{
    SingletonContainer::Get<WindowManagerLite>().UpdateWindowModeTypeInfo(type);
}

void WindowManagerAgentLite::UpdateCameraWindowStatus(uint32_t accessTokenId, bool isShowing)
{
    SingletonContainer::Get<WindowManagerLite>().UpdateCameraWindowStatus(accessTokenId, isShowing);
}

void WindowManagerAgentLite::NotifyWindowStyleChange(WindowStyleType type)
{
    SingletonContainer::Get<WindowManagerLite>().NotifyWindowStyleChange(type);
}

void WindowManagerAgentLite::NotifyCallingWindowDisplayChanged(const CallingWindowInfo& callingWindowInfo)
{
    SingletonContainer::Get<WindowManagerLite>().NotifyCallingWindowDisplayChanged(callingWindowInfo);
}

void WindowManagerAgentLite::UpdatePiPWindowStateChanged(const std::string& bundleName, bool isForeground)
{
    SingletonContainer::Get<WindowManagerLite>().UpdatePiPWindowStateChanged(bundleName, isForeground);
}

void WindowManagerAgentLite::NotifyWindowPropertyChange(uint32_t propertyDirtyFlags,
    const WindowInfoList& windowInfoList)
{
    SingletonContainer::Get<WindowManagerLite>().NotifyWindowPropertyChange(propertyDirtyFlags, windowInfoList);
}

void WindowManagerAgentLite::NotifyAccessibilityWindowInfo(const std::vector<sptr<AccessibilityWindowInfo>>& infos,
    WindowUpdateType type)
{
    SingletonContainer::Get<WindowManagerLite>().NotifyAccessibilityWindowInfo(infos, type);
}
} // namespace Rosen
} // namespace OHOS
