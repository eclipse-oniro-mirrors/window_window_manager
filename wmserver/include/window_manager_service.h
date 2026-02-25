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

#ifndef OHOS_WINDOW_MANAGER_SERVICE_H
#define OHOS_WINDOW_MANAGER_SERVICE_H

#include <input_window_monitor.h>
#include <map>
#include <nocopyable.h>
#include <system_ability.h>
#include <transaction/rs_interfaces.h>
#include <vector>
#include <window_manager_service_handler_stub.h>

#include "atomic_map.h"
#include "display_change_listener.h"
#include "drag_controller.h"
#include "event_handler.h"
#include "freeze_controller.h"
#include "perform_reporter.h"
#include "singleton_delegator.h"
#include "snapshot_controller.h"
#include "struct_multimodal.h"
#include "window_common_event.h"
#include "window_controller.h"
#include "window_dumper.h"
#include "window_group_mgr.h"
#include "window_manager_config.h"
#include "window_root.h"
#include "window_system_effect.h"
#include "wm_common_inner.h"
#include "wm_single_instance.h"
#include "zidl/window_manager_stub.h"

namespace OHOS {
namespace Rosen {
class DisplayChangeListener : public IDisplayChangeListener {
public:
    virtual void OnDisplayStateChange(DisplayId defaultDisplayId, sptr<DisplayInfo> displayInfo,
        const std::map<DisplayId, sptr<DisplayInfo>>& displayInfoMap, DisplayStateChangeType type) override;
    virtual void OnScreenshot(DisplayId displayId) override;
};

class WindowInfoQueriedListener : public IWindowInfoQueriedListener {
public:
    virtual void HasPrivateWindow(DisplayId id, bool& hasPrivateWindow) override;
};

class WindowManagerServiceHandler : public AAFwk::WindowManagerServiceHandlerStub {
public:
    virtual void NotifyWindowTransition(
        sptr<AAFwk::AbilityTransitionInfo> from, sptr<AAFwk::AbilityTransitionInfo> to, bool& animaEnabled) override;
    int32_t GetFocusWindow(sptr<IRemoteObject>& abilityToken) override;
    virtual void StartingWindow(
        sptr<AAFwk::AbilityTransitionInfo> info, std::shared_ptr<Media::PixelMap> pixelMap, uint32_t bgColor) override;
    virtual void StartingWindow(
        sptr<AAFwk::AbilityTransitionInfo> info, std::shared_ptr<Media::PixelMap> pixelMap) override;
    virtual void CancelStartingWindow(sptr<IRemoteObject> abilityToken) override;
    virtual void NotifyAnimationAbilityDied(sptr<AAFwk::AbilityTransitionInfo> info) override;
    virtual int32_t MoveMissionsToForeground(const std::vector<int32_t>& missionIds, int32_t topMissionId) override;
    virtual int32_t MoveMissionsToBackground(
        const std::vector<int32_t>& missionIds, std::vector<int32_t>& result) override;
};

class RSUIDirector;
class WindowManagerService : public SystemAbility, public WindowManagerStub {
friend class DisplayChangeListener;
friend class WindowManagerServiceHandler;
DECLARE_SYSTEM_ABILITY(WindowManagerService);
WM_DECLARE_SINGLE_INSTANCE_BASE(WindowManagerService);

public:
    using Task = std::function<void()>;
    void OnStart() override;
    void OnStop() override;
    void OnAddSystemAbility(int32_t systemAbilityId, const std::string& deviceId) override;
    void SetWindowInputEventConsumer();
    int Dump(int fd, const std::vector<std::u16string>& args) override;

    WMError CreateWindow(sptr<IWindow>& window, sptr<WindowProperty>& property,
        const std::shared_ptr<RSSurfaceNode>& surfaceNode,
        uint32_t& windowId, sptr<IRemoteObject> token) override;
    WMError AddWindow(sptr<WindowProperty>& property) override;
    WMError RemoveWindow(uint32_t windowId, bool isFromInnerkits) override;
    WMError NotifyWindowTransition(sptr<WindowTransitionInfo>& from, sptr<WindowTransitionInfo>& to,
        bool isFromClient = false) override;
    void NotifyAnimationAbilityDied(sptr<WindowTransitionInfo> info);
    WMError DestroyWindow(uint32_t windowId, bool onlySelf = false) override;
    WMError RequestFocus(uint32_t windowId) override;
    AvoidArea GetAvoidAreaByType(uint32_t windowId, AvoidAreaType avoidAreaType,
        const Rect& rect = Rect::EMPTY_RECT) override;
    void NotifyServerReadyToMoveOrDrag(uint32_t windowId, sptr<WindowProperty>& windowProperty,
        sptr<MoveDragProperty>& moveDragProperty) override;
    void ProcessPointDown(uint32_t windowId, bool isPointDown) override;
    void ProcessPointUp(uint32_t windowId) override;
    WMError GetTopWindowId(uint32_t mainWinId, uint32_t& topWinId) override;
    WMError MinimizeAllAppWindows(DisplayId displayId, int32_t excludeWindowId = 0) override;
    WMError ToggleShownStateForAllAppWindows() override;
    WMError SetWindowLayoutMode(WindowLayoutMode mode) override;
    WMError NotifyScreenshotEvent(ScreenshotEventType type) override;
    WMError UpdateProperty(sptr<WindowProperty>& windowProperty, PropertyChangeAction action,
        bool isAsyncTask = false) override;
    WMError SetWindowGravity(uint32_t windowId, WindowGravity gravity, uint32_t percent) override;
    WMError GetAccessibilityWindowInfo(std::vector<sptr<AccessibilityWindowInfo>>& infos) override;
    WMError GetUnreliableWindowInfo(int32_t windowId, std::vector<sptr<UnreliableWindowInfo>>& infos) override;
    WMError GetVisibilityWindowInfo(std::vector<sptr<WindowVisibilityInfo>>& infos) override;
    WMError RaiseToAppTop(uint32_t windowId) override;
    std::shared_ptr<Media::PixelMap> GetSnapshot(int32_t windowId) override;
    WMError SetGestureNavigationEnabled(bool enable) override;
    void DispatchKeyEvent(uint32_t windowId, std::shared_ptr<MMI::KeyEvent> event) override;
    void NotifyDumpInfoResult(const std::vector<std::string>& info) override;
    WMError GetWindowAnimationTargets(std::vector<uint32_t> missionIds,
        std::vector<sptr<RSWindowAnimationTarget>>& targets) override;
    WMError RegisterWindowManagerAgent(WindowManagerAgentType type,
        const sptr<IWindowManagerAgent>& windowManagerAgent) override;
    WMError UnregisterWindowManagerAgent(WindowManagerAgentType type,
        const sptr<IWindowManagerAgent>& windowManagerAgent) override;

    WMError SetWindowAnimationController(const sptr<RSIWindowAnimationController>& controller) override;
    WMError GetSystemConfig(SystemConfig& systemConfig) override;
    WMError GetModeChangeHotZones(DisplayId displayId, ModeChangeHotZones& hotZones) override;
    WMError UpdateAvoidAreaListener(uint32_t windowId, bool haveAvoidAreaListener) override;
    void StartingWindow(sptr<WindowTransitionInfo> info, std::shared_ptr<Media::PixelMap> pixelMap,
        bool isColdStart, uint32_t bkgColor = 0xffffffff);
    void CancelStartingWindow(sptr<IRemoteObject> abilityToken);
    WMError MoveMissionsToForeground(const std::vector<int32_t>& missionIds, int32_t topMissionId);
    WMError MoveMissionsToBackground(const std::vector<int32_t>& missionIds,
        std::vector<int32_t>& result);
    void MinimizeWindowsByLauncher(std::vector<uint32_t> windowIds, bool isAnimated,
        sptr<RSIWindowAnimationFinishedCallback>& finishCallback) override;
    WMError UpdateRsTree(uint32_t windowId, bool isAdd) override;
    void OnScreenshot(DisplayId displayId);
    void OnAccountSwitched(int accountId);
    WMError BindDialogTarget(uint32_t& windowId, sptr<IRemoteObject> targetToken) override;
    void HasPrivateWindow(DisplayId displayId, bool& hasPrivateWindow);
    void NotifyWindowClientPointUp(uint32_t windowId, const std::shared_ptr<MMI::PointerEvent>& pointerEvent);

    void SetAnchorAndScale(int32_t x, int32_t y, float scale) override;
    void SetAnchorOffset(int32_t deltaX, int32_t deltaY) override;
    void OffWindowZoom() override;
    void PostAsyncTask(Task task, const std::string& taskName = "WMSTask", uint32_t delay = 0);
    void SetMaximizeMode(MaximizeMode maximizeMode) override;
    MaximizeMode GetMaximizeMode() override;
    void GetFocusWindowInfo(FocusChangeInfo& focusInfo, DisplayId displayId = DEFAULT_DISPLAY_ID) override;
    void GetAllGroupInfo(std::unordered_map<DisplayId, DisplayGroupId>& displayId2GroupIdMap,
                         std::vector<sptr<FocusChangeInfo>>& allFocusInfoList) override {};

protected:
    WindowManagerService();
    virtual ~WindowManagerService() = default;

private:
    bool Init();
    void InitWithAbilityManagerServiceAdded();
    void InitWithRanderServiceAdded();
    void WindowVisibilityChangeCallback(std::shared_ptr<RSOcclusionData> occlusionData);
    void OnWindowEvent(Event event, const sptr<IRemoteObject>& remoteObject);
    void NotifyDisplayStateChange(DisplayId defaultDisplayId, sptr<DisplayInfo> displayInfo,
        const std::map<DisplayId, sptr<DisplayInfo>>& displayInfoMap, DisplayStateChangeType type);
    WMError GetFocusWindowInfo(sptr<IRemoteObject>& abilityToken, DisplayId displayId = DEFAULT_DISPLAY_ID);
    bool CheckSystemWindowPermission(const sptr<WindowProperty>& property) const;
    bool CheckAnimationPermission(const sptr<WindowProperty>& property) const;
    void LoadWindowParameter();
    void ConfigureWindowManagerService();
    void PostVoidSyncTask(Task task, const std::string& taskName = "WMSTask");
    template<typename SyncTask, typename Return = std::invoke_result_t<SyncTask>>
    Return PostSyncTask(SyncTask&& task, const std::string& taskName = "WMSTask")
    {
        Return ret;
        std::function<void()> syncTask([&ret, &task]() {ret = task();});
        if (handler_) {
            handler_->PostSyncTask(syncTask, "wms:" + taskName, AppExecFwk::EventQueue::Priority::IMMEDIATE);
        }
        return ret;
    }
    void ConfigHotZones(const std::vector<int>& hotZones);
    void ConfigDecor(const WindowManagerConfig::ConfigItem& decorConfig);
    void ConfigWindowAnimation(const WindowManagerConfig::ConfigItem& animeConfig);
    void ConfigKeyboardAnimation(const WindowManagerConfig::ConfigItem& animeConfig);
    void ConfigStartingWindowAnimation(const WindowManagerConfig::ConfigItem& animeConfig);
    RSAnimationTimingCurve CreateCurve(const WindowManagerConfig::ConfigItem& curveConfig);
    void CreateKeyboardCurve(const WindowManagerConfig::ConfigItem& config,
        AnimationConfig::KeyboardAnimation& animateConfig, KeyboardAnimationCurve& sysCurveConfig);
    void RecordShowTimeEvent(int64_t costTime);
    void ConfigWindowEffect(const WindowManagerConfig::ConfigItem& effectConfig);
    bool ConfigAppWindowCornerRadius(const WindowManagerConfig::ConfigItem& item, float& out);
    bool ConfigAppWindowShadow(const WindowManagerConfig::ConfigItem& shadowConfig, WindowShadowParameters& outShadow);

    /*
     * RS Client Multi Instance
     */
    void InitRSUIDirector();

    static inline SingletonDelegator<WindowManagerService> delegator;
    std::string name_ = "WindowManagerService";
    AtomicMap<uint32_t, uint32_t> accessTokenIdMaps_;
    sptr<WindowRoot> windowRoot_;
    sptr<WindowController> windowController_;
    sptr<InputWindowMonitor> inputWindowMonitor_;
    sptr<SnapshotController> snapshotController_;
    sptr<WindowManagerServiceHandler> wmsHandler_;
    sptr<DragController> dragController_;
    sptr<FreezeController> freezeDisplayController_;
    sptr<WindowDumper> windowDumper_;
    sptr<WindowGroupMgr> windowGroupMgr_;
    SystemConfig systemConfig_;
    ModeChangeHotZonesConfig hotZonesConfig_ { false, 0, 0, 0 };
    std::shared_ptr<WindowCommonEvent> windowCommonEvent_;
    std::shared_ptr<AppExecFwk::EventRunner> runner_ = nullptr;
    std::shared_ptr<AppExecFwk::EventHandler> handler_ = nullptr;
    RSInterfaces& rsInterface_;
    bool startingOpen_ = true;
    std::shared_ptr<RSUIDirector> rsUiDirector_;
    std::shared_ptr<PerformReporter> windowShowPerformReport_;
    MaximizeMode maximizeMode_ = MaximizeMode::MODE_RECOVER;
};
} // namespace Rosen
} // namespace OHOS
#endif // OHOS_WINDOW_MANAGER_SERVICE_H
