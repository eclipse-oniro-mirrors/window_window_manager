/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#ifndef OHOS_ROSEN_SESSION_MANAGER_AGENT_CONTROLLER_H
#define OHOS_ROSEN_SESSION_MANAGER_AGENT_CONTROLLER_H
#include <mutex>

#include "client_agent_container.h"
#include "window_manager.h"
#include "wm_single_instance.h"
#include "zidl/window_manager_agent_interface.h"
#include "window_visibility_info.h"
#include "window_drawing_content_info.h"
#include "window_pid_visibility_info.h"

namespace OHOS {
namespace Rosen {
class SessionManagerAgentController {
WM_DECLARE_SINGLE_INSTANCE_BASE(SessionManagerAgentController)
public:
    WMError RegisterWindowManagerAgent(const sptr<IWindowManagerAgent>& windowManagerAgent,
        WindowManagerAgentType type, int32_t pid);
    WMError UnregisterWindowManagerAgent(const sptr<IWindowManagerAgent>& windowManagerAgent,
        WindowManagerAgentType type, int32_t pid);

    void UpdateCameraFloatWindowStatus(uint32_t accessTokenId, bool isShowing);
    void UpdateFocusChangeInfo(const sptr<FocusChangeInfo>& focusChangeInfo, bool focused);
    void UpdateDisplayGroupInfo(DisplayGroupId displayGroupId, DisplayId displayId, bool isAdd);
    void UpdateWindowModeTypeInfo(WindowModeType type);
    void NotifyAccessibilityWindowInfo(const std::vector<sptr<AccessibilityWindowInfo>>& infos,
        WindowUpdateType type);
    void NotifyWaterMarkFlagChangedResult(bool hasWaterMark);
    void UpdateWindowVisibilityInfo(const std::vector<sptr<WindowVisibilityInfo>>& windowVisibilityInfos);
    void UpdateVisibleWindowNum(const std::vector<VisibleWindowNumInfo>& visibleWindowNumInfo);
    void UpdateWindowDrawingContentInfo(const std::vector<sptr<WindowDrawingContentInfo>>& windowDrawingContentInfos);
    void UpdateCameraWindowStatus(uint32_t accessTokenId, bool isShowing);
    void NotifyGestureNavigationEnabledResult(bool enable);
    void NotifyWindowStyleChange(WindowStyleType type);
    void NotifyCallingWindowDisplayChanged(const CallingWindowInfo& callingWindowInfo);
    void NotifyWindowPidVisibilityChanged(const sptr<WindowPidVisibilityInfo>& info);
    void NotifyWindowSystemBarPropertyChange(WindowType type, const SystemBarProperty& systemBarProperty);
    void UpdatePiPWindowStateChanged(const std::string& bundleName, bool isForeground);
    void NotifyWindowPropertyChange(uint32_t propertyDirtyFlags, const WindowInfoList& windowInfoList);
    void NotifySupportRotationChange(const SupportRotationInfo& supportRotationInfo);

private:
    SessionManagerAgentController()
    {
        smAgentContainer_.SetAgentDeathCallback(([this](const sptr<IRemoteObject>& remoteObject) {
            DoAfterAgentDeath(remoteObject);
        }));
    }
    virtual ~SessionManagerAgentController() = default;
    void DoAfterAgentDeath(const sptr<IRemoteObject>& remoteObject);

    ClientAgentContainer<IWindowManagerAgent, WindowManagerAgentType> smAgentContainer_;
    std::map<int32_t, std::map<WindowManagerAgentType, sptr<IWindowManagerAgent>>> windowManagerPidAgentMap_;
    std::map<sptr<IRemoteObject>, std::pair<int32_t, WindowManagerAgentType>> windowManagerAgentPairMap_;
    std::mutex windowManagerAgentPidMapMutex_;
    WindowManagementMode windowManagementMode_ { WindowManagementMode::UNDEFINED };
};
}
}
#endif // OHOS_ROSEN_WINDOW_MANAGER_AGENT_CONTROLLER_H
