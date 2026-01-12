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

#ifndef OHOS_WINDOW_MANAGER_LITE_INTERFACE_H
#define OHOS_WINDOW_MANAGER_LITE_INTERFACE_H

#include <iremote_broker.h>

#include "interfaces/include/ws_common.h"
#include "window_property.h"
#include "window_transition_info.h"
#include "zidl/window_manager_agent_interface.h"

namespace OHOS {
namespace MMI {
class KeyEvent;
}
namespace Rosen {

class IWindowManagerLite : public IRemoteBroker {
public:
    DECLARE_INTERFACE_DESCRIPTOR(u"OHOS.IWindowManagerLite");
    // do not need enum
    virtual WMError RegisterWindowManagerAgent(WindowManagerAgentType type,
        const sptr<IWindowManagerAgent>& windowManagerAgent) = 0;
    virtual WMError UnregisterWindowManagerAgent(WindowManagerAgentType type,
        const sptr<IWindowManagerAgent>& windowManagerAgent) = 0;
    virtual WMError GetVisibilityWindowInfo(std::vector<sptr<WindowVisibilityInfo>>& infos) { return WMError::WM_OK; };
    virtual WMError UpdateScreenLockStatusForApp(
        const std::string& bundleName, bool isRelease) { return WMError::WM_OK; }
    virtual void GetFocusWindowInfo(FocusChangeInfo& focusInfo, DisplayId displayId = DEFAULT_DISPLAY_ID) = 0;
    virtual void GetAllGroupInfo(std::unordered_map<DisplayId, DisplayGroupId>& displayId2GroupIdMap,
                                 std::vector<sptr<FocusChangeInfo>>& allFocusInfoList) = 0;
    virtual WMError CheckWindowId(int32_t windowId, int32_t& pid) = 0;
    virtual WMError CheckUIExtensionCreation(int32_t windowId, uint32_t tokenId, const AppExecFwk::ElementName& element,
        AppExecFwk::ExtensionAbilityType extensionAbilityType, int32_t& pid) = 0;
    virtual WMError GetWindowModeType(WindowModeType& windowModeType) { return WMError::WM_OK; }
    virtual WMError GetMainWindowInfos(int32_t topNum, std::vector<MainWindowInfo>& topNInfo) = 0;
    virtual WMError UpdateAnimationSpeedWithPid(pid_t pid, float speed) { return WMError::WM_OK; }
    virtual WMError GetCallingWindowInfo(CallingWindowInfo& callingWindowInfo) = 0;
    virtual WMError GetAllMainWindowInfos(std::vector<MainWindowInfo>& infos) = 0;
    virtual WMError GetMainWindowInfoByToken(const sptr<IRemoteObject>& abilityToken, MainWindowInfo& windowInfo) = 0;
    virtual WMError ClearMainSessions(const std::vector<int32_t>& persistentIds,
        std::vector<int32_t>& clearFailedIds) = 0;
    virtual WSError RaiseWindowToTop(int32_t persistentId) { return WSError::WS_OK; }
    virtual WMError GetWindowStyleType(WindowStyleType& windowStyleType) = 0;
    virtual WMError TerminateSessionByPersistentId(int32_t persistentId) = 0;
    virtual WMError CloseTargetFloatWindow(const std::string& bundleName) = 0;
    virtual WMError CloseTargetPiPWindow(const std::string& bundleName) = 0;
    virtual WMError GetCurrentPiPWindowInfo(std::string& bundleName) = 0;
    virtual WMError GetRootMainWindowId(int32_t persistentId, int32_t& hostWindowId) = 0;
    virtual WMError GetAccessibilityWindowInfo(std::vector<sptr<AccessibilityWindowInfo>>& infos) = 0;
    virtual WMError SetGlobalDragResizeType(DragResizeType dragResizeType) { return WMError::WM_OK; }
    virtual WMError GetGlobalDragResizeType(DragResizeType& dragResizeType) { return WMError::WM_OK; }
    virtual WMError SetAppDragResizeType(const std::string& bundleName,
        DragResizeType dragResizeType) { return WMError::WM_OK; }
    virtual WMError GetAppDragResizeType(const std::string& bundleName,
        DragResizeType& dragResizeType) { return WMError::WM_OK; }
    virtual WMError SetAppKeyFramePolicy(const std::string& bundleName,
        const KeyFramePolicy& keyFramePolicy) { return WMError::WM_OK; }
    virtual WMError ListWindowInfo(const WindowInfoOption& windowInfoOption,
        std::vector<sptr<WindowInfo>>& infos) { return WMError::WM_ERROR_DEVICE_NOT_SUPPORT; }
    virtual WMError RegisterWindowPropertyChangeAgent(WindowInfoKey windowInfoKey, uint32_t interestInfo,
        const sptr<IWindowManagerAgent>& windowManagerAgent) { return WMError::WM_ERROR_DEVICE_NOT_SUPPORT; }
    virtual WMError UnregisterWindowPropertyChangeAgent(WindowInfoKey windowInfoKey, uint32_t interestInfo,
        const sptr<IWindowManagerAgent>& windowManagerAgent) { return WMError::WM_ERROR_DEVICE_NOT_SUPPORT; }
    virtual WMError RecoverWindowPropertyChangeFlag(uint32_t observedFlags, uint32_t interestedFlags)
    {
        return WMError::WM_ERROR_DEVICE_NOT_SUPPORT;
    }
    virtual WSError SendPointerEventForHover(const std::shared_ptr<MMI::PointerEvent>& pointerEvent)
        { return WSError::WS_ERROR_DEVICE_NOT_SUPPORT; }
    virtual WMError GetDisplayIdByWindowId(const std::vector<uint64_t>& windowIds,
        std::unordered_map<uint64_t, DisplayId>& windowDisplayIdMap) = 0;
};
}
}
#endif // OHOS_WINDOW_MANAGER_LITE_INTERFACE_H
