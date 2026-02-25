
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

#include "window_manager_agent_lite.h"
#include "foundation/window/window_manager/interfaces/innerkits/wm/window_manager_lite.h"
#include "window_manager_hilog.h"
#include "wm_common.h"

namespace OHOS {
namespace Rosen {
WindowManagerAgentLite::WindowManagerAgentLite(const int32_t userId) : userId_(userId) {}

void WindowManagerAgentLite::UpdateFocusChangeInfo(const sptr<FocusChangeInfo>& focusChangeInfo, bool focused)
{
    TLOGD(WmsLogTag::WMS_FOCUS, "userId_=%{public}d", userId_);
    WindowManagerLite::GetInstance(userId_).UpdateFocusChangeInfo(focusChangeInfo, focused);
}

void WindowManagerAgentLite::UpdateDisplayGroupInfo(DisplayGroupId displayGroupId, DisplayId displayId, bool isAdd)
{
    TLOGD(WmsLogTag::WMS_FOCUS, "userId_=%{public}d", userId_);
    WindowManagerLite::GetInstance(userId_).UpdateDisplayGroupInfo(displayGroupId, displayId, isAdd);
}

void WindowManagerAgentLite::UpdateWindowVisibilityInfo(const std::vector<sptr<WindowVisibilityInfo>>& visibilityInfos)
{
    TLOGD(WmsLogTag::WMS_ATTRIBUTE, "userId_=%{public}d", userId_);
    WindowManagerLite::GetInstance(userId_).UpdateWindowVisibilityInfo(visibilityInfos);
}

void WindowManagerAgentLite::UpdateWindowDrawingContentInfo(
    const std::vector<sptr<WindowDrawingContentInfo>>& windowDrawingContentInfos)
{
    TLOGD(WmsLogTag::WMS_SYSTEM, "userId_=%{public}d", userId_);
    WindowManagerLite::GetInstance(userId_).UpdateWindowDrawingContentInfo(windowDrawingContentInfos);
}

void WindowManagerAgentLite::UpdateWindowModeTypeInfo(WindowModeType type)
{
    TLOGD(WmsLogTag::WMS_SYSTEM, "userId_=%{public}d", userId_);
    WindowManagerLite::GetInstance(userId_).UpdateWindowModeTypeInfo(type);
}

void WindowManagerAgentLite::UpdateCameraWindowStatus(uint32_t accessTokenId, bool isShowing)
{
    TLOGD(WmsLogTag::WMS_SYSTEM, "userId_=%{public}d", userId_);
    WindowManagerLite::GetInstance(userId_).UpdateCameraWindowStatus(accessTokenId, isShowing);
}

void WindowManagerAgentLite::NotifyWindowStyleChange(WindowStyleType type)
{
    TLOGD(WmsLogTag::WMS_MAIN, "userId_=%{public}d", userId_);
    WindowManagerLite::GetInstance(userId_).NotifyWindowStyleChange(type);
}

void WindowManagerAgentLite::NotifyCallingWindowDisplayChanged(const CallingWindowInfo& callingWindowInfo)
{
    TLOGD(WmsLogTag::WMS_KEYBOARD, "userId_=%{public}d", userId_);
    WindowManagerLite::GetInstance(userId_).NotifyCallingWindowDisplayChanged(callingWindowInfo);
}

void WindowManagerAgentLite::UpdatePiPWindowStateChanged(const std::string& bundleName, bool isForeground)
{
    TLOGD(WmsLogTag::WMS_PIP, "userId_=%{public}d", userId_);
    WindowManagerLite::GetInstance(userId_).UpdatePiPWindowStateChanged(bundleName, isForeground);
}

void WindowManagerAgentLite::NotifyWindowPropertyChange(uint32_t propertyDirtyFlags,
    const WindowInfoList& windowInfoList)
{
    TLOGD(WmsLogTag::WMS_ATTRIBUTE, "userId_=%{public}d", userId_);
    WindowManagerLite::GetInstance(userId_).NotifyWindowPropertyChange(propertyDirtyFlags, windowInfoList);
}

void WindowManagerAgentLite::NotifyAccessibilityWindowInfo(const std::vector<sptr<AccessibilityWindowInfo>>& infos,
    WindowUpdateType type)
{
    TLOGD(WmsLogTag::WMS_ATTRIBUTE, "userId_=%{public}d", userId_);
    WindowManagerLite::GetInstance(userId_).NotifyAccessibilityWindowInfo(infos, type);
}
} // namespace Rosen
} // namespace OHOS
