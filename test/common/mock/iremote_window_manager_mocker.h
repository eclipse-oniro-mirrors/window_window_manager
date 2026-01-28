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

#ifndef OHOS_ROSEN_TEST_COMMON_MOCK_IREMOTE_WINDOW_MANAGER_MOCKER
#define OHOS_ROSEN_TEST_COMMON_MOCK_IREMOTE_WINDOW_MANAGER_MOCKER

#include <gmock/gmock.h>
#include <iremote_object.h>
#include <iremote_stub.h>
#include "window_manager_hilog.h"
#include "wm_common.h"
#include "zidl/window_manager_agent_interface.h"
#include "zidl/window_manager_interface.h"

namespace OHOS::Rosen {
class IRemoteWindowManagerMocker : public IRemoteStub<IWindowManager> {
public:
    IRemoteWindowManagerMocker() = default;
    ~IRemoteWindowManagerMocker() = default;

    // virtual (func) = 0 in IWindowManager
    MOCK_METHOD(WMError, CreateWindow, (sptr<IWindow>&, sptr<WindowProperty>&, const std::shared_ptr<RSSurfaceNode>&,
        uint32_t&, sptr<IRemoteObject>), (override));
    MOCK_METHOD(WMError, AddWindow, (sptr<WindowProperty>& property), (override));
    MOCK_METHOD(WMError, RemoveWindow, (uint32_t windowId, bool isFromInnerkits), (override));
    MOCK_METHOD(WMError, DestroyWindow, (uint32_t, bool), (override));
    MOCK_METHOD(WMError, RequestFocus, (uint32_t windowId), (override));
    MOCK_METHOD(AvoidArea, GetAvoidAreaByType, (uint32_t windowId, AvoidAreaType type, const Rect& rect), (override));
    MOCK_METHOD(WMError, GetTopWindowId, (uint32_t mainWinId, uint32_t& topWinId), (override));
    MOCK_METHOD(void, NotifyServerReadyToMoveOrDrag, (uint32_t, sptr<WindowProperty>&,
        sptr<MoveDragProperty>&), (override));
    MOCK_METHOD(void, ProcessPointDown, (uint32_t windowId, bool isPointDown), (override));
    MOCK_METHOD(void, ProcessPointUp, (uint32_t windowId), (override));
    MOCK_METHOD(WMError, MinimizeAllAppWindows, (DisplayId displayId), (override));
    MOCK_METHOD(WMError, ToggleShownStateForAllAppWindows, (), (override));
    MOCK_METHOD(WMError, SetWindowLayoutMode, (WindowLayoutMode mode), (override));
    MOCK_METHOD(WMError, NotifyScreenshotEvent, (ScreenshotEventType type), (override));
    MOCK_METHOD(WMError, UpdateProperty, (sptr<WindowProperty>&, PropertyChangeAction, bool), (override));
    MOCK_METHOD(WMError, SetWindowGravity, (uint32_t windowId, WindowGravity gravity, uint32_t percent), (override));
    MOCK_METHOD(WMError, GetAccessibilityWindowInfo, (std::vector<sptr<AccessibilityWindowInfo>>& infos), (override));
    MOCK_METHOD(WMError, GetUnreliableWindowInfo, (int32_t, std::vector<sptr<UnreliableWindowInfo>>&), (override));
    MOCK_METHOD(WMError, GetVisibilityWindowInfo, (std::vector<sptr<WindowVisibilityInfo>>& infos), (override));
    MOCK_METHOD(WMError, SetWindowAnimationController, (const sptr<RSIWindowAnimationController>&), (override));
    MOCK_METHOD(WMError, GetSystemConfig, (SystemConfig& systemConfig), (override));
    MOCK_METHOD(WMError, NotifyWindowTransition, (sptr<WindowTransitionInfo>&, sptr<WindowTransitionInfo>&,
        bool), (override));
    MOCK_METHOD(WMError, GetModeChangeHotZones, (DisplayId displayId, ModeChangeHotZones& hotZones), (override));
    MOCK_METHOD(WMError, UpdateAvoidAreaListener, (uint32_t windowId, bool haveListener), (override));
    MOCK_METHOD(WMError, UpdateRsTree, (uint32_t windowId, bool isAdd), (override));
    MOCK_METHOD(WMError, BindDialogTarget, (uint32_t& windowId, sptr<IRemoteObject> targetToken), (override));
    MOCK_METHOD(WMError, RaiseToAppTop, (uint32_t windowId), (override));
    MOCK_METHOD(std::shared_ptr<Media::PixelMap>, GetSnapshot, (int32_t windowId), (override));
    MOCK_METHOD(WMError, SetGestureNavigationEnabled, (bool enable), (override));
    MOCK_METHOD(void, DispatchKeyEvent, (uint32_t windowId, std::shared_ptr<MMI::KeyEvent> event), (override));
    MOCK_METHOD(WMError, GetWindowAnimationTargets, (std::vector<uint32_t> missionIds,
        std::vector<sptr<RSWindowAnimationTarget>>& targets), (override));
    MOCK_METHOD(void, SetMaximizeMode, (MaximizeMode maximizeMode), (override));
    MOCK_METHOD(MaximizeMode, GetMaximizeMode, (), (override));
    MOCK_METHOD(void, GetFocusWindowInfo, (FocusChangeInfo&, DisplayId), (override));
    MOCK_METHOD(void, GetFocusWindowInfoByAbilityToken, (FocusChangeInfo&, const sptr<IRemoteObject>&), (override));
    MOCK_METHOD(void, MinimizeWindowsByLauncher, (std::vector<uint32_t> windowIds, bool isAnimated,
        sptr<RSIWindowAnimationFinishedCallback>& finishCallback), (override));
    MOCK_METHOD(void, SetAnchorAndScale, (int32_t x, int32_t y, float scale), (override));
    MOCK_METHOD(void, SetAnchorOffset, (int32_t deltaX, int32_t deltaY), (override));
    MOCK_METHOD(void, OffWindowZoom, (), (override));

    // func mocked and will be tested
    MOCK_METHOD(WMError, RegisterWindowManagerAgent, (WindowManagerAgentType type,
        const sptr<IWindowManagerAgent>& windowManagerAgent), (override));
    MOCK_METHOD(WMError, UnregisterWindowManagerAgent, (WindowManagerAgentType type,
        const sptr<IWindowManagerAgent>& windowManagerAgent), (override));
    MOCK_METHOD(WMError, RegisterWindowPropertyChangeAgent, (WindowInfoKey windowInfoKey, uint32_t interestInfo,
        const sptr<IWindowManagerAgent>& windowManagerAgent), (override));
};
} // namespace OHOS::Rosen
#endif
