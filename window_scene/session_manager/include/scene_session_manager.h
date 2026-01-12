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

#ifndef OHOS_ROSEN_WINDOW_SCENE_SCENE_SESSION_MANAGER_H
#define OHOS_ROSEN_WINDOW_SCENE_SCENE_SESSION_MANAGER_H

#include <cstdint>
#include <unordered_map>
#include <mutex>
#include <shared_mutex>

#ifndef OHOS_BUILD_ENABLE_SECURITY_COMPONENT // pointer_envent.h
#define OHOS_BUILD_ENABLE_SECURITY_COMPONENT
#endif
#ifndef SUPPORT_SCREEN
#define SUPPORT_SCREEN
#endif

#include "ability_info.h"
#include "agent_death_recipient.h"
#include "app_debug_listener_interface.h"
#include "app_mgr_client.h"
#include "application_state_observer_stub.h"
#include "common/include/task_scheduler.h"
#include "display_change_info.h"
#include "display_change_listener.h"
#include "display_info.h"
#include "future.h"
#include "include/core/SkRegion.h"
#include "interfaces/include/ws_common.h"
#include "mission_snapshot.h"
#include "res_common.h"
#include "scb_session_handler.h"
#include "scene_session_converter.h"
#include "screen_fold_data.h"
#include "screen_session_manager_client.h"
#include "session/host/include/keyboard_session.h"
#include "session/host/include/session.h"
#include "session/host/include/root_scene_session.h"
#include "session_listener_controller.h"
#include "ffrt_queue_helper.h"
#include "session_manager/include/window_manager_lru.h"
#include "session_manager/include/zidl/scene_session_manager_stub.h"
#include "thread_safety_annotations.h"
#include "transaction/rs_interfaces.h"
#include "window_focus_controller.h"
#include "window_scene_config.h"
#include "wm_single_instance.h"
#include "zidl/session_lifecycle_listener_interface.h"
#include "zidl/session_router_stack_listener.h"
#include "zidl/pip_change_listener.h"

namespace OHOS::AAFwk {
class SessionInfo;
} // namespace OHOS::AAFwk

namespace OHOS::AppExecFwk {
class IBundleMgr;
struct AbilityInfo;
struct BundleInfo;
class LauncherService;
struct JsonProfileInfo;
} // namespace OHOS::AppExecFwk

namespace OHOS::Global::Resource {
class ResourceManager;
enum ColorMode;
} // namespace OHOS::Global::Resource

namespace OHOS::Rosen {
class RSNode;
namespace AncoConsts {
    constexpr const char* ANCO_MISSION_ID = "ohos.anco.param.missionId";
    constexpr const char* ANCO_SESSION_ID = "ohos.anco.param.sessionId";
}
constexpr const char* IS_CALL_BY_SCB = "isCallBySCB";

struct SCBAbilityInfo {
    AppExecFwk::AbilityInfo abilityInfo_;
    uint32_t sdkVersion_;
    std::string codePath_;
    bool isAbilityHook_;
    bool isForceRotate_;
};

struct SessionIdentityInfo {
    std::string bundleName_;
    std::string moduleName_;
    std::string abilityName_;
    int32_t appIndex_ = 0;
    std::string instanceKey_;
    uint32_t windowType_ = static_cast<uint32_t>(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    bool isAtomicService_ = false;
    std::string specifiedFlag_;
};

class SceneSession;
struct SecSurfaceInfo;
class RSUIExtensionData;
class AccessibilityWindowInfo;
class UnreliableWindowInfo;
class ScbDumpSubscriber;
class StartingWindowRdbManager;
struct StartingWindowRdbItemKey;
class IUIEffectController;
class IUIEffectControllerClient;

using NotifyCreateSystemSessionFunc = std::function<void(const sptr<SceneSession>& session)>;
using NotifyCreateKeyboardSessionFunc = std::function<void(const sptr<SceneSession>& keyboardSession,
    const sptr<SceneSession>& panelSession)>;
using NotifyCreateSubSessionFunc = std::function<void(const sptr<SceneSession>& session)>;
using NotifyRecoverSceneSessionFunc =
    std::function<void(const sptr<SceneSession>& session, const SessionInfo& sessionInfo)>;
using ProcessStatusBarEnabledChangeFunc = std::function<void(bool enable, const std::string& bundleName)>;
using ProcessGestureNavigationEnabledChangeFunc = std::function<void(bool enable, const std::string& bundleName,
    GestureBackType type)>;
using ProcessOutsideDownEventFunc = std::function<void(int32_t x, int32_t y)>;
using ProcessShiftFocusFunc = std::function<void(int32_t persistentId, DisplayId displayGroupId)>;
using NotifySetFocusSessionFunc = std::function<void(const sptr<SceneSession>& session)>;
using DumpRootSceneElementInfoFunc = std::function<void(const sptr<SceneSession>& session,
    const std::vector<std::string>& params, std::vector<std::string>& infos)>;
using WindowChangedFunc = std::function<void(int32_t persistentId, WindowUpdateType type)>;
using TraverseFunc = std::function<bool(const sptr<SceneSession>& session)>;
using CmpFunc = std::function<bool(std::pair<int32_t, sptr<SceneSession>>& lhs,
    std::pair<int32_t, sptr<SceneSession>>& rhs)>;
using ProcessStartUIAbilityErrorFunc = std::function<void(int32_t startUIAbilityError)>;
using NotifySCBAfterUpdateFocusFunc = std::function<void(DisplayId displayId)>;
using NotifyDiffSCBAfterUpdateFocusFunc = std::function<void(DisplayId prevDisplayId, DisplayId currDisplayId)>;
using FlushWindowInfoTask = std::function<void()>;
using ProcessVirtualPixelRatioChangeFunc = std::function<void(float density, const Rect& rect)>;
using DumpUITreeFunc = std::function<void(std::string& dumpInfo)>;
using RootSceneProcessBackEventFunc = std::function<void()>;
using ProcessCloseTargetFloatWindowFunc = std::function<void(const std::string& bundleName)>;
using AbilityManagerCollaboratorRegisteredFunc = std::function<void()>;
using OnFlushUIParamsFunc = std::function<void()>;
using IsRootSceneLastFrameLayoutFinishedFunc = std::function<bool()>;
using NotifyStartPiPFailedFunc = std::function<void(DisplayId displayId)>;
using NotifyAppUseControlListFunc =
    std::function<void(ControlAppType type, int32_t userId, const std::vector<AppUseControlInfo>& controlList)>;
using NotifyRootSceneAvoidAreaChangeFunc = std::function<void(const sptr<AvoidArea>& avoidArea, AvoidAreaType type,
    const sptr<OccupiedAreaChangeInfo>& info)>;
using NotifySupportRotationRegisteredFunc = std::function<void()>;
using NotifyWatchGestureConsumeResultFunc = std::function<void(int32_t keyCode, bool isConsumed)>;
using NotifyWatchFocusActiveChangeFunc = std::function<void(bool isActive)>;
using GetRSNodeByStringIDFunc = std::function<std::shared_ptr<Rosen::RSNode>(const std::string& id)>;
using SetTopWindowBoundaryByIDFunc = std::function<void(const std::string& id)>;
using SetForegroundWindowNumFunc = std::function<void(uint32_t windowNum)>;
using MinimizeByWindowIdFunc = std::function<void(const std::vector<int32_t>& windowIds)>;
using NotifySceneSessionDestructFunc = std::function<void(int32_t persistentId)>;
using NotifyTransferSessionToTargetScreenFunc = std::function<void(const TransferSessionInfo& info)>;
using HasRootSceneRequestedVsyncFunc = std::function<bool()>;
using RequestVsyncByRootSceneWhenModeChangeFunc =
    std::function<void(const std::shared_ptr<VsyncCallback>& vsyncCallback)>;
using UpdateKioskAppListFunc = std::function<void(const std::vector<std::string>& kioskAppList)>;
using KioskModeChangeFunc = std::function<void(bool isKioskMode, int32_t persistentId)>;
using NotifySessionRecoverStateChangeFunc = std::function<void(const SessionRecoverState& state,
    const sptr<WindowSessionProperty>& property)>;
using NotifyRecoverStateChangeFunc = std::function<void(const RecoverState& state)>;
using FindScenePanelRsNodeByZOrderFunc = std::function<std::shared_ptr<Rosen::RSNode>(DisplayId screenId,
    uint32_t targetZOrder)>;
using NotifyAppProcessDiedFunc = std::function<void(const AppExecFwk::ProcessData& processData)>;
using ConvertSystemConfigFunc = std::function<void(const std::string& configItem)>;
using NotifyVirtualPixelChangeFunc = std::function<void(float density, DisplayId displayId)>;
using NotifySetSpecificWindowZIndexFunc = std::function<void(WindowType windowType, int32_t zIndex,
    SetSpecificZIndexReason reason)>;
class AppAnrListener : public IRemoteStub<AppExecFwk::IAppDebugListener> {
public:
    void OnAppDebugStarted(const std::vector<AppExecFwk::AppDebugInfo>& debugInfos) override;
    void OnAppDebugStoped(const std::vector<AppExecFwk::AppDebugInfo>& debugInfos) override;
};

class DisplayChangeListener : public IDisplayChangeListener {
public:
    void OnDisplayStateChange(DisplayId defaultDisplayId, sptr<DisplayInfo> displayInfo,
        const std::map<DisplayId, sptr<DisplayInfo>>& displayInfoMap, DisplayStateChangeType type) override;
    void OnScreenshot(DisplayId displayId) override;
    void OnImmersiveStateChange(ScreenId screenId, bool& immersive) override;
    void OnGetSurfaceNodeIdsFromMissionIds(std::vector<uint64_t>& missionIds,
        std::vector<uint64_t>& surfaceNodeIds, const std::vector<uint32_t>& needWindowTypeList = {},
        bool isNeedForceCheck = false) override;
    void OnSetSurfaceNodeIds(DisplayId displayId, const std::vector<uint64_t>& surfaceNodeIds) override;
    void OnVirtualScreenDisconnected(DisplayId displayId) override;

    /*
     * Fold Screen Status Change Report
     */
    void OnScreenFoldStatusChanged(const std::vector<std::string>& screenFoldInfo) override;
};

class ScreenConnectionChangeListener : public IScreenConnectionChangeListener {
public:
    void OnScreenConnected(const sptr<ScreenSession>& screenSession) override;
    void OnScreenDisconnected(const sptr<ScreenSession>& screenSession) override;
};

class AppStateObserver : public AppExecFwk::ApplicationStateObserverStub {
public:
    void OnProcessDied(const AppExecFwk::ProcessData& processData) override;
    void RegisterProcessDiedNotifyFunc(NotifyAppProcessDiedFunc&& func);

private:
    NotifyAppProcessDiedFunc procDiedCallback_;
};

class SceneSessionManager : public SceneSessionManagerStub {
WM_DECLARE_SINGLE_INSTANCE_BASE(SceneSessionManager)
public:
    friend class AnomalyDetection;
    bool IsSessionVisible(const sptr<SceneSession>& session) const;
    bool IsSessionVisibleForeground(const sptr<SceneSession>& session) const;

    void SetRootSceneContext(const std::weak_ptr<AbilityRuntime::Context>& contextWeak);
    sptr<RootSceneSession> GetRootSceneSession();
    sptr<SceneSession> GetSceneSession(int32_t persistentId);
    bool CheckAndGetAbilityInfoByWant(const std::shared_ptr<AAFwk::Want>& want, AppExecFwk::AbilityInfo& abilityInfo);
    sptr<SceneSession> GetMainParentSceneSession(int32_t persistentId,
        const std::map<int32_t, sptr<SceneSession>>& sessionMap);
    void PostFlushWindowInfoTask(FlushWindowInfoTask&& task, const std::string& taskName, const int delayTime);

    sptr<SceneSession> GetSceneSessionByIdentityInfo(const SessionIdentityInfo& info);
    sptr<SceneSession> GetSceneSessionByType(WindowType type);
    std::vector<sptr<SceneSession>> GetSceneSessionByBundleName(const std::string& bundleName);

    WSError CreateAndConnectSpecificSession(const sptr<ISessionStage>& sessionStage,
        const sptr<IWindowEventChannel>& eventChannel, const std::shared_ptr<RSSurfaceNode>& surfaceNode,
        sptr<WindowSessionProperty> property, int32_t& persistentId, sptr<ISession>& session,
        SystemSessionConfig& systemConfig, sptr<IRemoteObject> token = nullptr) override;
    WSError DestroyAndDisconnectSpecificSession(const int32_t persistentId) override;
    WSError DestroyAndDisconnectSpecificSessionWithDetachCallback(const int32_t persistentId,
        const sptr<IRemoteObject>& callback) override;
    void SetCreateSystemSessionListener(const NotifyCreateSystemSessionFunc& func);
    void SetCreateKeyboardSessionListener(const NotifyCreateKeyboardSessionFunc& func);
    void SetStatusBarEnabledChangeListener(const ProcessStatusBarEnabledChangeFunc& func);
    void SetStartUIAbilityErrorListener(const ProcessStartUIAbilityErrorFunc& func);
    void SetGestureNavigationEnabledChangeListener(const ProcessGestureNavigationEnabledChangeFunc& func);
    void SetDumpRootSceneElementInfoListener(const DumpRootSceneElementInfoFunc& func);
    void SetOutsideDownEventListener(const ProcessOutsideDownEventFunc& func);
    void SetShiftFocusListener(const ProcessShiftFocusFunc& func);
    void SetSCBFocusedListener(const NotifySCBAfterUpdateFocusFunc& func);
    void SetSCBUnfocusedListener(const NotifySCBAfterUpdateFocusFunc& func);
    void SetSCBFocusChangeListener(const NotifyDiffSCBAfterUpdateFocusFunc&& func);
    void SetDumpUITreeFunc(const DumpUITreeFunc& func);
    void SetFindScenePanelRsNodeByZOrderFunc(FindScenePanelRsNodeByZOrderFunc&& func);
    const AppWindowSceneConfig& GetWindowSceneConfig() const;
    void ApplyFeatureConfig(const std::unordered_map<std::string, std::string>& configMap);
    WMError GetAllJsonProfile(AppExecFwk::ProfileType profileType, int32_t userId,
        std::vector<AppExecFwk::JsonProfileInfo>& jsonProfileInfos);
    WMError GetJsonProfile(AppExecFwk::ProfileType profileType, const std::string& bundleName,
        const std::string& moduleName, int32_t userId, std::string& profileInfo);

    /*
     * Window Recover
     */
    WSError RecoverAndConnectSpecificSession(const sptr<ISessionStage>& sessionStage,
        const sptr<IWindowEventChannel>& eventChannel, const std::shared_ptr<RSSurfaceNode>& surfaceNode,
        sptr<WindowSessionProperty> property, sptr<ISession>& session, sptr<IRemoteObject> token = nullptr) override;
    WSError RecoverAndReconnectSceneSession(const sptr<ISessionStage>& sessionStage,
        const sptr<IWindowEventChannel>& eventChannel, const std::shared_ptr<RSSurfaceNode>& surfaceNode,
        sptr<ISession>& session, sptr<WindowSessionProperty> property = nullptr,
        sptr<IRemoteObject> token = nullptr) override;
    WSError GetBatchAbilityInfos(const std::vector<std::string>& bundleNames, int32_t userId,
        std::vector<SCBAbilityInfo>& scbAbilityInfos);
    void SetRecoverSceneSessionListener(const NotifyRecoverSceneSessionFunc& func);
    void UpdateRecoveredSessionInfo(const std::vector<int32_t>& recoveredPersistentIds);
    void NotifyRecoveringFinished();
    bool IsInputEventEnabled() const;
    void SetEnableInputEvent(bool enabled);
    void SetAlivePersistentIds(const std::vector<int32_t>& alivePersistentIds);

    /*
     * Window Input Event
     */
    void RegisterWatchGestureConsumeResultCallback(NotifyWatchGestureConsumeResultFunc&& func);
    WMError NotifyWatchGestureConsumeResult(int32_t keyCode, bool isConsumed) override;
    void RegisterWatchFocusActiveChangeCallback(NotifyWatchFocusActiveChangeFunc&& func);
    WMError NotifyWatchFocusActiveChange(bool isActive) override;
    void RegisterFlushWindowInfoCallback();
    void FlushWindowInfoToMMI(const bool forceFlush = false);
    void SendCancelEventBeforeEraseSession(const sptr<SceneSession>& sceneSession);
    void BuildCancelPointerEvent(const std::shared_ptr<MMI::PointerEvent>& pointerEvent, int32_t fingerId,
                                 int32_t action, int32_t wid);

    /*
     * Window Rotate Animation
     */
    void UpdateRotateAnimationConfig(const RotateAnimationConfig& config);
    void CloseSyncTransaction(std::function<void()> func);
    const std::vector<sptr<SceneSession>> GetActiveSceneSessionCopy();

    WSError ProcessBackEvent();
    WSError BindDialogSessionTarget(uint64_t persistentId, sptr<IRemoteObject> targetToken) override;
    WMError SetGestureNavigationEnabled(bool enable) override;
    WMError RegisterWindowManagerAgent(WindowManagerAgentType type,
        const sptr<IWindowManagerAgent>& windowManagerAgent) override;
    WMError UnregisterWindowManagerAgent(WindowManagerAgentType type,
        const sptr<IWindowManagerAgent>& windowManagerAgent) override;

    /*
     * Dump
     */
    WSError GetAllSessionDumpInfo(std::string& info);
    WSError GetAllSessionDumpDetailInfo(std::string& info);
    WSError GetSpecifiedSessionDumpInfo(std::string& dumpInfo, const std::vector<std::string>& params,
        const std::string& strId);
    WSError GetSCBDebugDumpInfo(std::string&& cmd, std::string& dumpInfo);
    WSError GetSessionDumpInfo(const std::vector<std::string>& params, std::string& info) override;
    WSError DumpSessionAll(std::vector<std::string>& infos) override;
    WSError DumpSessionWithId(int32_t persistentId, std::vector<std::string>& infos) override;

    /*
     * Window Focus
     */
    WSError SetFocusedSessionId(const int32_t persistentId, const DisplayId displayId);
    int32_t GetFocusedSessionId(DisplayId displayId = DEFAULT_DISPLAY_ID) const;
    DisplayId GetDisplayGroupId(DisplayId displayId = DEFAULT_DISPLAY_ID) const;
    std::vector<std::pair<DisplayId, int32_t>> GetAllFocusedSessionList() const;
    FocusChangeReason GetFocusChangeReason() const { return focusChangeReason_; }
    WMError RequestFocusStatus(int32_t persistentId, bool isFocused, bool byForeground = true,
        FocusChangeReason reason = FocusChangeReason::DEFAULT) override;
    WMError RequestFocusStatusBySCB(int32_t persistentId, bool isFocused, bool byForeground = true,
        FocusChangeReason reason = FocusChangeReason::DEFAULT, DisplayId displayId = DISPLAY_ID_INVALID);
    WMError RequestFocusStatusBySA(int32_t persistentId, bool isFocused = true,
        bool byForeground = true, FocusChangeReason reason = FocusChangeReason::SA_REQUEST) override;
    void RequestAllAppSessionUnfocus();
    WSError ShiftAppWindowFocus(int32_t sourcePersistentId, int32_t targetPersistentId) override;
    void GetFocusWindowInfo(FocusChangeInfo& focusInfo, DisplayId displayId = DEFAULT_DISPLAY_ID) override;
    void GetFocusWindowInfoByAbilityToken(FocusChangeInfo& focusInfo,
        const sptr<IRemoteObject>& abilityToken) override;
    void GetAllGroupInfo(std::unordered_map<DisplayId, DisplayGroupId>& displayId2GroupIdMap,
                         std::vector<sptr<FocusChangeInfo>>& allFocusInfoList) override;
    WSError GetFocusSessionToken(sptr<IRemoteObject>& token, DisplayId displayId = DEFAULT_DISPLAY_ID) override;
    WSError GetFocusSessionElement(AppExecFwk::ElementName& element, DisplayId displayId = DEFAULT_DISPLAY_ID) override;
    WSError IsFocusWindowParent(const sptr<IRemoteObject>& token, bool& isParent);
    WSError AddFocusGroup(DisplayGroupId displayGroupId, DisplayId displayId);
    WSError RemoveFocusGroup(DisplayGroupId displayGroupId, DisplayId displayId);
    WSError SendPointerEventForHover(const std::shared_ptr<MMI::PointerEvent>& pointerEvent);

    /*
     * Window Hierarchy
     */
    WSError SetSpecificWindowZIndex(WindowType windowType, int32_t zIndex) override;
    WSError ResetSpecificWindowZIndex(int32_t pid) override;
    void SetSpecificWindowZIndexListener(const NotifySetSpecificWindowZIndexFunc& func);

    WSError UpdateWindowMode(int32_t persistentId, int32_t windowMode);
    WSError SendTouchEvent(const std::shared_ptr<MMI::PointerEvent>& pointerEvent, uint32_t zIndex);
    WSError RaiseWindowToTop(int32_t persistentId) override;
    void SetScreenLocked(const bool isScreenLocked);
    bool IsScreenLocked() const;
    void ConfigSupportZLevel();

    /*
     * Multi User
     */
    WSError InitUserInfo(int32_t userId, std::string& fileDir);
    void HandleUserSwitch(const UserSwitchEventType type, const bool isUserActive);
    int32_t GetCurrentUserId() const;

    void StartWindowInfoReportLoop();

    void NotifyCompleteFirstFrameDrawing(int32_t persistentId);
    void NotifySessionMovedToFront(int32_t persistentId);
    WSError SetSessionLabel(const sptr<IRemoteObject>& token, const std::string& label) override;
    WSError SetSessionIcon(const sptr<IRemoteObject>& token, const std::shared_ptr<Media::PixelMap>& icon) override;
    WSError SetSessionIconInner(const sptr<IRemoteObject>& token, const std::shared_ptr<Media::PixelMap>& icon);
    WSError SetSessionIconForThirdParty(const sptr<IRemoteObject>& token, const std::shared_ptr<Media::PixelMap>& icon);
    WSError IsValidSessionIds(const std::vector<int32_t>& sessionIds, std::vector<bool>& results) override;
    void HandleTurnScreenOn(const sptr<SceneSession>& sceneSession);
    void HandleKeepScreenOn(const sptr<SceneSession>& sceneSession, bool requireLock,
        const std::string& screenLockPrefix, std::shared_ptr<PowerMgr::RunningLock>& screenLock);
    void InitWithRenderServiceAdded();

    WSError RegisterSessionListener(const sptr<ISessionListener>& listener) override;
    WSError UnRegisterSessionListener(const sptr<ISessionListener>& listener) override;
    WSError GetSessionInfos(const std::string& deviceId, int32_t numMax,
        std::vector<SessionInfoBean>& sessionInfos) override;
    WSError GetMainWindowStatesByPid(int32_t pid, std::vector<MainWindowState>& windowStates);
    WSError GetSessionInfo(const std::string& deviceId, int32_t persistentId, SessionInfoBean& sessionInfo) override;
    WSError GetSessionInfoByContinueSessionId(const std::string& continueSessionId,
        SessionInfoBean& sessionInfo) override;
    WSError GetAllAbilityInfos(const AAFwk::Want& want, int32_t userId,
        std::vector<SCBAbilityInfo>& scbAbilityInfos);
    WSError GetAbilityInfo(const std::string& bundleName, const std::string& moduleName,
        const std::string& abilityName, int32_t userId, SCBAbilityInfo& scbAbilityInfo);
    WSError PrepareTerminate(int32_t persistentId, bool& isPrepareTerminate);

    WSError UpdateSessionTouchOutsideListener(int32_t& persistentId, bool haveListener) override;
    WSError GetSessionSnapshot(const std::string& deviceId, int32_t persistentId,
                               SessionSnapshot& snapshot, bool isLowResolution) override;
    WMError GetSessionSnapshotById(int32_t persistentId, SessionSnapshot& snapshot) override;
    WSError SetVmaCacheStatus(bool flag);
    WSError GetUIContentRemoteObj(int32_t persistentId, sptr<IRemoteObject>& uiContentRemoteObj) override;
    WMError GetRootUIContentRemoteObj(DisplayId displayId, sptr<IRemoteObject>& uiContentRemoteObj) override;
    WSError SetSessionContinueState(const sptr<IRemoteObject>& token, const ContinueState& continueState) override;
    WSError LockSession(int32_t sessionId) override;
    WSError UnlockSession(int32_t sessionId) override;
    WMError GetTopWindowId(uint32_t mainWinId, uint32_t& topWinId) override;
    WMError GetParentMainWindowId(int32_t windowId, int32_t& mainWindowId) override;

    /*
     * PC Window
     */
    WMError IsPcWindow(bool& isPcWindow) override;
    WMError IsFreeMultiWindow(bool& isFreeMultiWindow) override;
    WMError IsPcOrPadFreeMultiWindowMode(bool& isPcOrPadFreeMultiWindowMode) override;
    WMError IsWindowRectAutoSave(const std::string& key, bool& enabled, int persistentId) override;
    void SetIsWindowRectAutoSave(const std::string& key, bool enabled,
        const std::string& abilityKey, bool isSaveBySpecifiedFlag);
    int32_t ChangeUIAbilityVisibilityBySCB(const sptr<SceneSession>& sceneSession, bool visibility,
        bool isNewWant, bool isFromClient = true);
    WMError ShiftAppWindowPointerEvent(int32_t sourcePersistentId, int32_t targetPersistentId,
        int32_t fingerId) override;
    void SetFocusedSessionDisplayIdIfNeeded(sptr<SceneSession>& newSession);
    WSError StartOrMinimizePcAppInPadUIAbilityBySCB(const sptr<SceneSession>& sceneSession, bool isBackground);
    std::vector<std::string> trayAppList_;
    WMError GetWindowLimits(int32_t windowId, WindowLimits& windowLimits);
    void RegisterVirtualPixelChangeCallback(NotifyVirtualPixelChangeFunc&& func);
    NotifyVirtualPixelChangeFunc onVirtualPixelChangeCallback_;
    void ConfigDockAutoHide(bool isDockAutoHide);
    void SchedulePcAppInPadLifecycle(bool isBackground);
    void SetTrayAppList(std::vector<std::string>&& trayAppList);

    /*
     * Sub Window
     */
    WMError SetParentWindow(int32_t subWindowId, int32_t newParentWindowId) override;

    std::map<int32_t, sptr<SceneSession>>& GetSessionMapByScreenId(ScreenId id);
    void UpdatePrivateStateAndNotify(uint32_t persistentId);
    void InitPersistentStorage();
    void OnOutsideDownEvent(int32_t x, int32_t y);
    WSError GetDisplayGroupIdFromSceneSession(const sptr<SceneSession>& session, DisplayId& displayGroupId) const;
    bool IsSameDisplayGroupId(const sptr<SceneSession>& session, const DisplayId touchDisplayGroupId) const;
    void NotifySessionTouchOutside(int32_t persistentId, DisplayId displayId);

    WMError GetAccessibilityWindowInfo(std::vector<sptr<AccessibilityWindowInfo>>& infos) override;
    WMError ConvertToRelativeCoordinateExtended(const Rect& rect, Rect& newRect, DisplayId& newDisplayId) override;
    WMError GetUnreliableWindowInfo(int32_t windowId,
        std::vector<sptr<UnreliableWindowInfo>>& infos) override;
    WSError SetWindowFlags(const sptr<SceneSession>& sceneSession, const sptr<WindowSessionProperty>& property);

    void OnScreenshot(DisplayId displayId);
    void NotifyDumpInfoResult(const std::vector<std::string>& info) override;
    void SetVirtualPixelRatioChangeListener(const ProcessVirtualPixelRatioChangeFunc& func);
    void ProcessVirtualPixelRatioChange(DisplayId defaultDisplayId, sptr<DisplayInfo> displayInfo,
        const std::map<DisplayId, sptr<DisplayInfo>>& displayInfoMap, DisplayStateChangeType type);
    void ProcessUpdateRotationChange(DisplayId defaultDisplayId, sptr<DisplayInfo> displayInfo,
        const std::map<DisplayId, sptr<DisplayInfo>>& displayInfoMap, DisplayStateChangeType type);

    void SetRootSceneProcessBackEventFunc(const RootSceneProcessBackEventFunc& processBackEventFunc);
    void RegisterWindowChanged(const WindowChangedFunc& func);
    WSError CloneWindow(int32_t fromPersistentId, int32_t toPersistentId, bool needOffScreen = true);

    /*
     * Collaborator
     */
    void SetAbilityManagerCollaboratorRegisteredFunc(const AbilityManagerCollaboratorRegisteredFunc& func);
    WSError RegisterIAbilityManagerCollaborator(int32_t type,
        const sptr<AAFwk::IAbilityManagerCollaborator>& impl) override;
    WSError UnregisterIAbilityManagerCollaborator(int32_t type) override;
    void ClearAllCollaboratorSessions();

    WMError CheckWindowId(int32_t windowId, int32_t& pid) override;
    void GetSceneSessionPrivacyModeBundles(DisplayId displayId,
        std::unordered_map<DisplayId, std::unordered_set<std::string>>& privacyBundles);
    BrokerStates CheckIfReuseSession(SessionInfo& sessionInfo);
    bool CheckCollaboratorType(int32_t type);
    sptr<SceneSession> FindSessionByAffinity(const std::string& affinity);
    void AddWindowDragHotArea(DisplayId displayId, uint32_t type, WSRect& area);
    void PreloadInLakeApp(const std::string& bundleName);
    WSError UpdateMaximizeMode(int32_t persistentId, bool isMaximize);
    WSError UpdateSessionDisplayId(int32_t persistentId, uint64_t screenId);
    void RegisterClientDisplayIdChangeNotifyManagerFunc(const sptr<SceneSession>& sceneSession);
    WSError NotifyStackEmpty(int32_t persistentId);
    void NotifySessionUpdate(const SessionInfo& sessionInfo, ActionType type,
        ScreenId fromScreenId = SCREEN_ID_INVALID);
    WMError GetSurfaceNodeIdsFromMissionIds(std::vector<uint64_t>& missionIds,
        std::vector<uint64_t>& surfaceNodeIds, const std::vector<uint32_t>& needWindowTypeList = {},
        bool isNeedForceCheck = false);
    WMError GetSurfaceNodeIdsFromSubSession(
        const sptr<SceneSession>& sceneSession, std::vector<uint64_t>& surfaceNodeIds);
    WMError UpdateSubSessionBlackList(const sptr<SceneSession>& sceneSession);
    WMError RemoveSessionFromBlackList(const sptr<SceneSession>& sceneSession);
    WMError SetSurfaceNodeIds(DisplayId displayId, const std::vector<uint64_t>& surfaceNodeIds);
    WMError OnVirtualScreenDisconnected(DisplayId displayId);
    WSError UpdateTitleInTargetPos(int32_t persistentId, bool isShow, int32_t height);

    /*
     * Window Immersive
     */
    WSError GetIsLayoutFullScreen(bool& isLayoutFullScreen);
    WSError SetMaximizeFullScreen(int32_t persistentId, bool isMaximizeFullScreen);
    WSError UpdateSessionAvoidAreaListener(int32_t persistentId, bool haveListener) override;
    void RegisterNotifyRootSceneAvoidAreaChangeFunc(NotifyRootSceneAvoidAreaChangeFunc&& func);
    void UpdateRootSceneAvoidArea();
    bool GetImmersiveState(ScreenId screenId);
    WSError NotifyStatusBarShowStatus(int32_t persistentId, bool isVisible);
    void UpdateAvoidAreaForLSStateChange(int32_t curState, int32_t preState);
    void NotifyStatusBarConstantlyShow(DisplayId displayId, bool isVisible);
    void GetStatusBarConstantlyShow(DisplayId displayId, bool& isVisible) const;
    WSError NotifyAINavigationBarShowStatus(bool isVisible, WSRect barArea, uint64_t displayId);
    WSError NotifyNextAvoidRectInfo(AvoidAreaType type,
        const WSRect& portraitRect, const WSRect& landspaceRect, DisplayId displayId);
    WSError GetNextAvoidRectInfo(DisplayId displayId, AvoidAreaType type,
        std::pair<WSRect, WSRect>& nextSystemBarAvoidAreaRectInfo);
    WSRect GetAINavigationBarArea(uint64_t displayId, bool ignoreVisibility = false);
    void ClearDisplayStatusBarTemporarilyFlags();
    AvoidArea GetRootSessionAvoidAreaByType(AvoidAreaType type, bool ignoreVisibility = false);
    uint32_t GetRootSceneStatusBarHeight() const;
    void SetOnFlushUIParamsFunc(OnFlushUIParamsFunc&& func);
    void SetIsRootSceneLastFrameLayoutFinishedFunc(IsRootSceneLastFrameLayoutFinishedFunc&& func);
    void SetStatusBarDefaultVisibilityPerDisplay(DisplayId displayId, bool visible);
    bool GetStatusBarDefaultVisibilityByDisplayId(DisplayId displayId);
    void SetStatusBarAvoidHeight(DisplayId displayId, int32_t height);
    void GetStatusBarAvoidHeight(DisplayId displayId, WSRect& barArea);

    WSError NotifyWindowExtensionVisibilityChange(int32_t pid, int32_t uid, bool visible) override;
    void DealwithVisibilityChange(const std::vector<std::pair<uint64_t, WindowVisibilityState>>& visibilityChangeInfos,
    const std::vector<std::pair<uint64_t, WindowVisibilityState>>& currVisibleData);
    void NotifyUpdateRectAfterLayout();
    void FlushUIParams(ScreenId screenId, std::unordered_map<int32_t, SessionUIParam>&& uiParams);
    WSError UpdateSessionWindowVisibilityListener(int32_t persistentId, bool haveListener) override;
    WMError UpdateSessionOcclusionStateListener(int32_t persistentId, bool haveListener) override;
    WMError SetSystemAnimatedScenes(SystemAnimatedSceneType sceneType, bool isRegularAnimation = false);

    std::shared_ptr<Media::PixelMap> GetSessionSnapshotPixelMap(const int32_t persistentId, const float scaleParam,
        const SnapshotNodeType snapshotNode = SnapshotNodeType::DEFAULT_NODE, bool needSnapshot = true);
    WMError GetVisibilityWindowInfo(std::vector<sptr<WindowVisibilityInfo>>& infos) override;
    const std::map<int32_t, sptr<SceneSession>> GetSceneSessionMap();
    void GetAllSceneSession(std::vector<sptr<SceneSession>>& sceneSessions);
    void GetAllWindowVisibilityInfos(std::vector<std::pair<int32_t, uint32_t>>& windowVisibilityInfos);

    /*
     * Keyboard Window
     */
    void RequestInputMethodCloseKeyboard(int32_t persistentId);
    void GetKeyboardOccupiedAreaWithRotation(
        int32_t persistentId, Rotation rotation, std::vector<std::pair<bool, WSRect>>& avoidAreas);
    void ReportKeyboardCreateException(sptr<SceneSession>& keyboardSession);
    void ConfigSupportFunctionType(SupportFunctionType funcType);

    /*
     * UIExtension
     */
    uint32_t GetLockScreenZOrder();
    WMError CheckUIExtensionCreation(int32_t windowId, uint32_t tokenId, const AppExecFwk::ElementName& element,
        AppExecFwk::ExtensionAbilityType extensionAbilityType, int32_t& pid);
    void OnNotifyAboveLockScreen(const std::vector<int32_t>& windowIds);
    void AddExtensionWindowStageToSCB(const sptr<ISessionStage>& sessionStage, const sptr<IRemoteObject>& token,
        uint64_t surfaceNodeId, int64_t startModalExtensionTimeStamp, bool isConstrainedModal = false) override;
    void RemoveExtensionWindowStageFromSCB(const sptr<ISessionStage>& sessionStage,
        const sptr<IRemoteObject>& token, bool isConstrainedModal = false) override;
    void UpdateModalExtensionRect(const sptr<IRemoteObject>& token, Rect rect) override;
    void ProcessModalExtensionPointDown(const sptr<IRemoteObject>& token, int32_t posX, int32_t posY) override;
    WSError AddOrRemoveSecureSession(int32_t persistentId, bool shouldHide) override;
    WSError CheckExtWindowFlagsPermission(ExtensionWindowFlags& actions) const;
    WSError UpdateExtWindowFlags(const sptr<IRemoteObject>& token, uint32_t extWindowFlags,
        uint32_t extWindowActions) override;
    void CheckSceneZOrder();
    WSError GetHostWindowRect(int32_t hostWindowId, Rect& rect) override;
    WSError GetHostGlobalScaledRect(int32_t hostWindowId, Rect& globalScaledRect) override;
    WMError GetCallingWindowWindowStatus(uint32_t callingWindowId, WindowStatus& windowStatus) override;
    WMError GetCallingWindowRect(uint32_t callingWindowId, Rect& rect) override;
    WMError GetWindowModeType(WindowModeType& windowModeType) override;
    WMError GetWindowIdsByCoordinate(DisplayId displayId, int32_t windowNumber,
        int32_t x, int32_t y, std::vector<int32_t>& windowIds) override;
    void ChangeWindowRectYInVirtualDisplay(DisplayId& displayId, int32_t& y);
    WSError UseImplicitAnimation(int32_t hostWindowId, bool useImplicit) override;

    int32_t ReclaimPurgeableCleanMem();
    void OnBundleUpdated(const std::string& bundleName, int userId);
    void OnConfigurationUpdated(const std::shared_ptr<AppExecFwk::Configuration>& configuration);

    std::shared_ptr<TaskScheduler> GetTaskScheduler() { return taskScheduler_; }

    int32_t GetCustomDecorHeight(int32_t persistentId);

    /*
     * Free Multi Window
     */
    WSError SwitchFreeMultiWindow(bool enable);
    WSError GetFreeMultiWindowEnableState(bool& enable) override;
    WMError SetForegroundWindowNum(uint32_t windowNum) override;
    void RegisterSetForegroundWindowNumCallback(SetForegroundWindowNumFunc&& func);

    const SystemSessionConfig& GetSystemSessionConfig() const;
    WSError NotifyEnterRecentTask(bool enterRecent);
    WMError UpdateDisplayHookInfo(int32_t uid, uint32_t width, uint32_t height, float_t density, bool enable);
    WMError UpdateAppHookDisplayInfo(int32_t uid, const HookInfo& hookInfo, bool enable);
    WMError NotifyHookOrientationChange(int32_t persistentId);
    void InitScheduleUtils();
    void ProcessDisplayScale(sptr<DisplayInfo>& displayInfo);
    WMError GetRootMainWindowId(int32_t persistentId, int32_t& hostWindowId);

    /*
     * Move Drag
     */
    WMError SetGlobalDragResizeType(DragResizeType dragResizeType) override;
    WMError GetGlobalDragResizeType(DragResizeType& dragResizeType) override;
    WMError SetAppDragResizeType(const std::string& bundleName, DragResizeType dragResizeType) override;
    WMError GetAppDragResizeType(const std::string& bundleName, DragResizeType& dragResizeType) override;
    WMError SetAppDragResizeTypeInner(const std::string& bundleName, DragResizeType dragResizeType);
    WMError SetAppKeyFramePolicy(const std::string& bundleName, const KeyFramePolicy& keyFramePolicy) override;
    KeyFramePolicy GetAppKeyFramePolicy(const std::string& bundleName);

    /*
     * Window Layout
     */
    SingleHandTransform GetNormalSingleHandTransform() const;
    SingleHandScreenInfo GetSingleHandScreenInfo() const;
    WSRect GetOriginRect() const;
    WSRect GetSingleHandRect() const;
    void NotifySingleHandInfoChange(SingleHandScreenInfo singleHandScreenInfo, WSRect originRect,
        WSRect singleHandRect);
    void RegisterGetRSNodeByStringIDFunc(GetRSNodeByStringIDFunc&& func);
    void RegisterSetTopWindowBoundaryByIDFunc(SetTopWindowBoundaryByIDFunc&& func);
    void RegisterSingleHandContainerNode(const std::string& stringId);
    const SingleHandCompatibleModeConfig& GetSingleHandCompatibleModeConfig() const;
    void ConfigSupportFollowParentWindowLayout();
    void ConfigSupportFollowRelativePositionToParent();
    void SetHasRootSceneRequestedVsyncFunc(HasRootSceneRequestedVsyncFunc&& func);
    void SetRequestVsyncByRootSceneWhenModeChangeFunc(RequestVsyncByRootSceneWhenModeChangeFunc&& func);
    WMError UpdateWindowModeByIdForUITest(int32_t windowId, int32_t updateMode) override;
    WMError UpdateAppHookWindowInfo(const std::string& bundleName, const HookWindowInfo& hookWindowInfo);
    HookWindowInfo GetAppHookWindowInfo(const std::string& bundleName);
    void UpdateAppHookWindowInfoWhenSwitchFreeMultiWindow(bool isOpenFreeMultiWindow);
    void UpdateRsCmdBlockingCount(bool enable);
    int32_t GetOrResetRsCmdBlockingCount();
    void RegisterGetRsCmdBlockingCountFunc(const sptr<SceneSession>& sceneSession);
    void RunAfterNVsyncs(uint32_t vsyncCount, OnCallback&& callback);

    /*
     * Window Property
     */
    WMError UpdateScreenLockStatusForApp(const std::string& bundleName, bool isRelease) override;
    void DealwithDrawingContentChange(const std::vector<std::pair<uint64_t, bool>>& drawingContentChangeInfo);
    WMError ListWindowInfo(const WindowInfoOption& windowInfoOption, std::vector<sptr<WindowInfo>>& infos) override;
    WMError RegisterWindowPropertyChangeAgent(WindowInfoKey windowInfoKey,
        uint32_t interestInfo, const sptr<IWindowManagerAgent>& windowManagerAgent) override;
    WMError UnregisterWindowPropertyChangeAgent(WindowInfoKey windowInfoKey,
        uint32_t interestInfo, const sptr<IWindowManagerAgent>& windowManagerAgent) override;
    WMError RecoverWindowPropertyChangeFlag(uint32_t observedFlags, uint32_t interestedFlags) override;
    WMError GetAllWindowLayoutInfo(DisplayId displayId, std::vector<sptr<WindowLayoutInfo>>& infos) override;
    WMError GetGlobalWindowMode(DisplayId displayId, GlobalWindowMode& globalWinMode) override;
    WMError GetTopNavDestinationName(int32_t windowId, std::string& topNavDestName) override;
    WMError SetWatermarkImageForApp(const std::shared_ptr<Media::PixelMap>& pixelMap,
        std::string& watermarkName) override;
    WMError RecoverWatermarkImageForApp(const std::string& watermarkName) override;
    void SetSkipSelfWhenShowOnVirtualScreen(uint64_t surfaceNodeId, bool isSkip);
    WMError AddSkipSelfWhenShowOnVirtualScreenList(const std::vector<int32_t>& persistentIds) override;
    WMError RemoveSkipSelfWhenShowOnVirtualScreenList(const std::vector<int32_t>& persistentIds) override;
    WMError SetScreenPrivacyWindowTagSwitch(
        uint64_t screenId, const std::vector<std::string>& privacyWindowTags, bool enable) override;
    WMError NotifyBrightnessModeChange(const std::string& brightnessMode);
    void NotifyOnAttachToFrameNode(const sptr<Session>& session);
    WMError NotifyScreenshotEvent(ScreenshotEventType type) override;
    WMError UpdateSessionScreenshotAppEventListener(int32_t persistentId, bool haveListener);
    WMError AddSessionBlackList(const std::unordered_set<std::string>& bundleNames,
        const std::unordered_set<std::string>& privacyWindowTags) override;
    WMError RemoveSessionBlackList(const std::unordered_set<std::string>& bundleNames,
        const std::unordered_set<std::string>& privacyWindowTags) override;
    void UpdateSessionPrivacyForSuperFold(const sptr<SceneSession>& sceneSession, DisplayId displayId,
        std::unordered_map<DisplayId, std::unordered_set<std::string>>& privacyBundles);
    void RegisterBrightnessDataChangeListener();
    void UnregisterBrightnessDataChangeListener();
    void SubscribePowerManagerServiceSa();

    /*
     * Multi Window
     */
    void SetCloseTargetFloatWindowFunc(const ProcessCloseTargetFloatWindowFunc& func);
    WMError CloseTargetFloatWindow(const std::string& bundleName);

    /*
     * Fold Screen Status Change Report
     */
    WMError ReportScreenFoldStatusChange(const std::vector<std::string>& screenFoldInfo);

    void SetBehindWindowFilterEnabled(bool enabled);
    void UpdateSecSurfaceInfo(std::shared_ptr<RSUIExtensionData> secExtensionData, uint64_t userId);
    void UpdateConstrainedModalUIExtInfo(std::shared_ptr<RSUIExtensionData> constrainedModalUIExtData, uint64_t userId);
    WSError SetAppForceLandscapeConfig(const std::string& bundleName, AppForceLandscapeConfig& config);
    WSError SetAppForceLandscapeConfigEnable(const std::string& bundleName, bool enableForceLandscape);
    AppForceLandscapeConfig GetAppForceLandscapeConfig(const std::string& bundleName);
    bool GetAppForceLandscapeConfigEnable(const std::string& bundleName);
    WMError GetWindowStyleType(WindowStyleType& windowStyletype) override;
    WMError GetProcessSurfaceNodeIdByPersistentId(const int32_t pid,
        const std::vector<int32_t>& persistentIds, std::vector<uint64_t>& surfaceNodeIds) override;

    /*
     * Window ZOrder: PC
     */
    void RefreshPcZOrderList(uint32_t startZOrder, std::vector<int32_t>&& persistentIds);

    /*
     * PiP Window
     */
    WMError CloseTargetPiPWindow(const std::string& bundleName);
    WMError GetCurrentPiPWindowInfo(std::string& bundleName);
    WMError GetPiPSettingSwitchStatus(bool& switchStatus) override;
    WMError GetIsPipEnabled(bool& isPipEnabled) override;
    void SetPiPSettingSwitchStatus(bool switchStatus);
    void SetIsPipEnabled(bool isPipEnabled);
    void SetStartPiPFailedListener(NotifyStartPiPFailedFunc&& func);
    bool GetPipDeviceCollaborationPolicy(int32_t screenId);
    WMError SetPipEnableByScreenId(int32_t screenId, bool enabled);
    WMError UnsetPipEnableByScreenId(int32_t screenId);
    WMError RegisterPipChgListenerByScreenId(int32_t screenId, const sptr<IPipChangeListener>& listener);
    WMError UnregisterPipChgListenerByScreenId(int32_t screenId);

    /*
     * Window Rotation
     */
    void SetSupportRotationRegisteredListener(NotifySupportRotationRegisteredFunc&& func);

     /*
     * FloatingBall Window
     */
    WMError GetFbPanelWindowId(uint32_t& windowId);

    /*
     * Window Watermark
     */
    WMError SetProcessWatermark(int32_t pid, const std::string& watermarkName, bool isEnabled) override;

    /*
     * Window Snapshot
     */
    WMError SkipSnapshotForAppProcess(int32_t pid, bool skip) override;
    WMError SkipSnapshotByUserIdAndBundleNames(int32_t userId,
        const std::vector<std::string>& bundleNameList) override;

    /*
     * Multi Instance
     */
    uint32_t GetMaxInstanceCount(const std::string& bundleName);
    uint32_t GetInstanceCount(const std::string& bundleName);
    std::string GetLastInstanceKey(const std::string& bundleName);
    void RefreshAppInfo(const std::string& bundleName);

    /*
     * Screen Manager
     */
    WMError GetDisplayIdByWindowId(const std::vector<uint64_t>& windowIds,
        std::unordered_map<uint64_t, DisplayId>& windowDisplayIdMap) override;

    /*
     * Specific Window
     */
    void RegisterCreateSubSessionListener(int32_t persistentId, const NotifyCreateSubSessionFunc& func);
    void RegisterBindDialogTargetListener(const sptr<SceneSession>& session, NotifyBindDialogSessionFunc&& func);
    void UnregisterSpecificSessionCreateListener(int32_t persistentId);
    WMError HasFloatingWindowForeground(const sptr<IRemoteObject>& abilityToken,
        bool& hasOrNot) override;

    /*
     * Window Lifecycle
     */
    void GetMainSessionByBundleNameAndAppIndex(
        const std::string& bundleName, int32_t appIndex, std::vector<sptr<SceneSession>>& mainSessions);
    WSError NotifyAppUseControlList(
        ControlAppType type, int32_t userId, const std::vector<AppUseControlInfo>& controlList);
    void NotifyAppUseControlListInner(
        ControlAppType type, int32_t userId, const std::vector<AppUseControlInfo>& controlList);
    sptr<SceneSession> GetSessionForAppUseControl(const AppUseControlInfo& appUseControlInfo);
    void RegisterNotifyAppUseControlListCallback(NotifyAppUseControlListFunc&& func);
    WMError MinimizeMainSession(const std::string& bundleName, int32_t appIndex, int32_t userId);
    sptr<SceneSession> RequestSceneSession(const SessionInfo& sessionInfo,
        sptr<WindowSessionProperty> property = nullptr);
    void UpdateSceneSessionWant(const SessionInfo& sessionInfo);
    WSError RequestSceneSessionActivation(const sptr<SceneSession>& sceneSession, bool isNewActive,
        int32_t requestId = DEFAULT_REQUEST_FROM_SCB_ID);
    WSError RequestSceneSessionBackground(const sptr<SceneSession>& sceneSession, const bool isDelegator = false,
        const bool isToDesktop = false, const bool isSaveSnapshot = true,
        LifeCycleChangeReason reason = LifeCycleChangeReason::DEFAULT);
    WSError RequestSceneSessionDestruction(const sptr<SceneSession>& sceneSession, bool needRemoveSession = true,
        bool isSaveSnapshot = true, const bool isForceClean = false, bool isUserRequestedExit = false,
        LifeCycleChangeReason reason = LifeCycleChangeReason::DEFAULT);
    WSError RequestSceneSessionDestructionInner(sptr<SceneSession>& sceneSession,
        sptr<AAFwk::SessionInfo> sceneSessionInfo, const bool needRemoveSession, const bool isForceClean = false,
        bool isUserRequestedExit = false, LifeCycleChangeReason reason = LifeCycleChangeReason::DEFAULT);
    void NotifyForegroundInteractiveStatus(const sptr<SceneSession>& sceneSession, bool interactive);
    WSError RequestSceneSessionByCall(const sptr<SceneSession>& sceneSession,
        int32_t requestId = DEFAULT_REQUEST_FROM_SCB_ID);
    void StartAbilityBySpecified(const SessionInfo& sessionInfo);
    void NotifyWindowStateErrorFromMMI(int32_t pid, int32_t persistentId);
    void RemoveLifeCycleTaskByPersistentId(int32_t persistentId, const LifeCycleTaskType taskType);
    WSError PendingSessionToForeground(const sptr<IRemoteObject>& token,
        int32_t windowMode = DEFAULT_INVALID_WINDOW_MODE) override;
    WSError PendingSessionToBackground(const sptr<IRemoteObject>& token, const BackgroundParams& params);
    WSError PendingSessionToBackgroundForDelegator(const sptr<IRemoteObject>& token,
        bool shouldBackToCaller = true) override;
    WSError TerminateSessionNew(
        const sptr<AAFwk::SessionInfo> info, bool needStartCaller, bool isFromBroker = false) override;
    WSError ClearSession(int32_t persistentId) override;
    WSError ClearAllSessions() override;
    WSError MoveSessionsToForeground(const std::vector<int32_t>& sessionIds, int32_t topSessionId) override;
    WSError MoveSessionsToBackground(const std::vector<int32_t>& sessionIds, std::vector<int32_t>& result) override;
    WMError MinimizeAllAppWindows(DisplayId displayId, int32_t excludeWindowId = 0) override;
    int32_t StartUIAbilityBySCB(sptr<AAFwk::SessionInfo>& abilitySessionInfo, sptr<SceneSession>& sceneSession);
    int32_t StartUIAbilityBySCB(sptr<SceneSession>& sceneSessions);
    WMError GetMainWindowInfos(int32_t topNum, std::vector<MainWindowInfo>& topNInfo);
    WMError GetAllMainWindowInfo(std::vector<sptr<MainWindowInfo>>& infos) override;
    WMError GetMainWindowInfoByToken(const sptr<IRemoteObject>& abilityToken, MainWindowInfo& windowInfo) const;
    WMError GetMainWindowSnapshot(const std::vector<int32_t>& windowIds, const WindowSnapshotConfiguration& config,
        const sptr<IRemoteObject>& callback) override;
    WMError GetCallingWindowInfo(CallingWindowInfo& callingWindowInfo);
    void NotifyDisplayIdChanged(int32_t persistentId, uint64_t displayId);
    WMError GetAllMainWindowInfos(std::vector<MainWindowInfo>& infos) const;
    WMError ClearMainSessions(const std::vector<int32_t>& persistentIds, std::vector<int32_t>& clearFailedIds);
    WMError TerminateSessionByPersistentId(int32_t persistentId);
    void SetUserAuthPassed(bool isUserAuthPassed);
    bool IsUserAuthPassed() const;
    void GetMainSessionByAbilityInfo(const AbilityInfoBase& abilityInfo,
        std::vector<sptr<SceneSession>>& mainSessions) const;
    WMError LockSessionByAbilityInfo(const AbilityInfoBase& abilityInfo, bool isLock);
    WMError RegisterSessionLifecycleListener(const sptr<ISessionLifecycleListener>& listener,
        const std::vector<int32_t>& persistentIdList);
    WMError RegisterSessionLifecycleListener(const sptr<ISessionLifecycleListener>& listener,
        const std::vector<std::string>& bundleNameList);
    WMError UnregisterSessionLifecycleListener(const sptr<ISessionLifecycleListener>& listener);
    bool IsMainWindowByPersistentId(int32_t persistentId);
    WMError MinimizeByWindowId(const std::vector<int32_t>& windowIds) override;
    void RegisterMinimizeByWindowIdCallback(MinimizeByWindowIdFunc&& func);
    WMError UpdateAnimationSpeedWithPid(pid_t pid, float speed);
    void RegisterSceneSessionDestructCallback(NotifySceneSessionDestructFunc&& func);
    void RegisterTransferSessionToTargetScreenCallback(NotifyTransferSessionToTargetScreenFunc&& func);
    WMError NotifyTransferSessionToTargetScreen(const TransferSessionInfo& info);
    void NotifySessionTransferToTargetScreenEvent(const int32_t persistentId,
        const uint32_t resultCode, const uint64_t fromScreenid, const uint64_t toScreenId,
        LifeCycleChangeReason reason = LifeCycleChangeReason::DEFAULT);
    WSError GetRecentMainSessionInfoList(std::vector<RecentSessionInfo>& recentSessionInfoList);
    void UpdateRecentMainSessionInfos(const std::vector<int32_t>& recentMainSessionIdList);
    sptr<SceneSession> GetMainSessionByPersistentId(int32_t persistentId) const;
    WMError GetRouterStackInfo(int32_t persistentId, const sptr<ISessionRouterStackListener>& listener);
    WMError CreateNewInstanceKey(const std::string& bundleName, std::string& instanceKey);
    WMError RemoveInstanceKey(const std::string& bundleName, const std::string& instanceKey);
    void AddRequestTaskInfo(sptr<SceneSession> sceneSession, int32_t requestId);
    std::shared_ptr<AAFwk::Want> GetRequestWantFromTaskInfoMap(int32_t persistentId, int32_t requestId);
    void RemoveRequestTaskInfo(int32_t persistentId, int32_t requestId);
    void ClearRequestTaskInfo(int32_t persistentId);
    void RefreshAllAppUseControlMap(const AppUseControlInfo& appUseControlInfo, ControlAppType type);
    void NotifyAmsPendingSessionWhenFail(uint32_t resultCode, std::string resultMessage,
        int32_t requestId, int32_t persistentId);
    WSError PendingSessionToBackgroundByPersistentId(const int32_t persistentId, bool shouldBackToCaller = true);
    WMError UpdateKioskAppList(const std::vector<std::string>& kioskAppList);
    WMError EnterKioskMode(const sptr<IRemoteObject>& token);
    WMError ExitKioskMode();
    void ConfigSupportCreateFloatWindow();
    void RegisterGetStartWindowConfigCallback(const sptr<SceneSession>& sceneSession);
    void RegisterUpdateKioskAppListCallback(UpdateKioskAppListFunc&& func);
    void RegisterKioskModeChangeCallback(KioskModeChangeFunc&& func);
    bool IsAppBoundSystemTray(int32_t callingPid, uint32_t callingToken, const std::string &instanceKey);
    void UpdateAppBoundSystemTrayStatus(const std::string &key, int32_t pid, bool enabled);
    void RegisterIsAppBoundSystemTrayFunc(const sptr<SceneSession>& sceneSession);

    /*
     * Window Pattern
     */
    void SetDelayRemoveSnapshot(bool delayRemoveSnapshot);
    bool GetDelayRemoveSnapshot() const;
    void InitSnapshotCache();
    void VisitSnapshotFromCache(int32_t persistentId);
    void PutSnapshotToCache(int32_t persistentId);
    void RemoveSnapshotFromCache(int32_t persistentId);
    void UpdateAllStartingWindowRdb();
    void GetStartupPage(const SessionInfo& sessionInfo, StartingWindowInfo& startingWindowInfo);
    bool CheckAndGetPreLoadResourceId(const StartingWindowInfo& startingWindowInfo, uint32_t& resId);
    std::shared_ptr<Media::PixelMap> GetPreLoadStartingWindow(const SessionInfo& sessionInfo);
    void RemovePreLoadStartingWindowFromMap(const SessionInfo& sessionInfo);
    WSError RegisterSaveSnapshotFunc(const sptr<SceneSession>& sceneSession);
    std::shared_ptr<Media::PixelMap> GetPixelMap(uint32_t resourceId,
        std::shared_ptr<AppExecFwk::AbilityInfo> abilityInfo, bool needCrop = false);
    WMError SetStartWindowBackgroundColor(const std::string& moduleName, const std::string& abilityName,
        uint32_t color, int32_t uid) override;
    void ConfigSupportSnapshotAllSessionStatus();
    void ConfigSupportCacheLockedSessionSnapshot();
    void ConfigSupportPreloadStartingWindow();
    void PreLoadStartingWindow(sptr<SceneSession> sceneSession);
    bool IsSyncLoadStartingWindow() { return syncLoadStartingWindow_; };
    bool IsDmaReclaimEnabled() { return enableDmaReclaim_; };

    /*
     * Window Animation
     */
    WMError AnimateTo(int32_t windowId, const WindowAnimationProperty& animationProperty,
        const WindowAnimationOption& animationOption) override;
    void NotifySupportRotationChange(const SupportRotationInfo& supportRotationInfo);
    WMError NotifySupportRotationRegistered() override;

    std::vector<sptr<SceneSession>> GetSceneSessions(ScreenId screenId);
    WMError UpdateScreenLockState(int32_t persistentId);

    WMError UpdateOutline(const sptr<IRemoteObject>& remoteObject, const OutlineParams& outlineParams) override;

protected:
    SceneSessionManager();
    virtual ~SceneSessionManager();

private:
    std::atomic<bool> enterRecent_ { false };
    bool isKeyboardPanelEnabled_ = false;
    bool isTrayAppForeground_ = false;
    std::unordered_map<std::string, ConvertSystemConfigFunc> convertConfigMap_;
    static sptr<SceneSessionManager> CreateInstance();
    static inline bool isNotCurrentScreen(sptr<SceneSession> sceneSession, ScreenId screenId)
    {
        return sceneSession->GetSessionInfo().screenId_ != screenId &&
               sceneSession->GetSessionInfo().screenId_ != SCREEN_ID_INVALID;
    }
    void Init();
    void RegisterAppListener();
    bool IsPrepareTerminateEnabled() const;
    void InitPrepareTerminateConfig();
    void LoadWindowSceneXml();
    void LoadWindowParameter();
    void ConfigWindowSceneXml();
    void ConfigWindowSceneXml(const WindowSceneConfig::ConfigItem& config);
    void ConfigWindowEffect(const WindowSceneConfig::ConfigItem& effectConfig,
        AppWindowSceneConfig& AppWindowSceneConfig);
    void ConfigWindowImmersive(const WindowSceneConfig::ConfigItem& immersiveConfig);
    void ConfigKeyboardAnimation(const WindowSceneConfig::ConfigItem& animationConfig);
    void ConfigDefaultKeyboardAnimation(KeyboardSceneAnimationConfig& animationIn,
        KeyboardSceneAnimationConfig& animationOut);
    bool ConfigAppWindowCornerRadius(const WindowSceneConfig::ConfigItem& item, float& out);
    bool ConfigAppWindowShadow(const WindowSceneConfig::ConfigItem& shadowConfig, WindowShadowConfig& outShadow);
    bool ConfigStatusBar(const WindowSceneConfig::ConfigItem& config, StatusBarConfig& statusBarConfig);
    void ConfigSystemUIStatusBar(const WindowSceneConfig::ConfigItem& statusBarConfig);
    void ConfigDecor(const WindowSceneConfig::ConfigItem& decorConfig, bool mainConfig = true);
    void ConfigWindowAnimation(const WindowSceneConfig::ConfigItem& windowAnimationConfig);
    void ConfigStartingWindowAnimation(const WindowSceneConfig::ConfigItem& startingWindowConfig);
    bool ConfigWindowLayout(const WindowSceneConfig::ConfigItem& windowLayoutConfig);
    bool ConfigMoveDrag(const WindowSceneConfig::ConfigItem& moveDragConfig);
    bool ConfigMoveResample(const WindowSceneConfig::ConfigItem& moveResampleConfig);
    void ConfigWindowSizeLimits();
    void ConfigMainWindowSizeLimits(const WindowSceneConfig::ConfigItem& mainWindowSizeConifg);
    void ConfigSubWindowSizeLimits(const WindowSceneConfig::ConfigItem& subWindowSizeConifg);
    void ConfigDialogWindowSizeLimits(const WindowSceneConfig::ConfigItem& dialogWindowSizeConifg);
    void ConfigSnapshotScale();
    void ConfigFreeMultiWindow();
    void LoadFreeMultiWindowConfig(bool enable);
    void DoUpdateSceneSessionWant(const SessionInfo& sessionInfo);

    std::tuple<std::string, std::vector<float>> CreateCurve(const WindowSceneConfig::ConfigItem& curveConfig);
    void LoadKeyboardAnimation(const WindowSceneConfig::ConfigItem& item, KeyboardSceneAnimationConfig& config);
    sptr<SceneSession::SpecificSessionCallback> CreateSpecificSessionCallback();
    sptr<KeyboardSession::KeyboardSessionCallback> CreateKeyboardSessionCallback();
    void FillSessionInfo(sptr<SceneSession>& sceneSession);
    std::shared_ptr<AppExecFwk::AbilityInfo> QueryAbilityInfoFromBMS(const int32_t uId, const std::string& bundleName,
        const std::string& abilityName, const std::string& moduleName, bool isAtomicServiceFreeInstall = false);
    std::vector<sptr<SceneSession>> GetSubSceneSession(int32_t parentWindowId);
    void RemoveDuplicateSubSession(const std::vector<std::pair<uint64_t, WindowVisibilityState>>& visibilityChangeInfo,
        std::vector<sptr<SceneSession>>& subSessions);
    void SetSessionVisibilityInfo(const sptr<SceneSession>& session, WindowVisibilityState visibleState,
        std::vector<sptr<WindowVisibilityInfo>>& windowVisibilityInfos, std::string& visibilityInfo);
    void UpdateSubWindowVisibility(const sptr<SceneSession>& session, WindowVisibilityState visibleState,
        const std::vector<std::pair<uint64_t, WindowVisibilityState>>& visibilityChangeInfo,
        std::vector<sptr<WindowVisibilityInfo>>& windowVisibilityInfos, std::string& visibilityInfo,
        const std::vector<std::pair<uint64_t, WindowVisibilityState>>& currVisibleData);
    bool GetSessionRSVisible(const sptr<Session>& session,
        const std::vector<std::pair<uint64_t, WindowVisibilityState>>& currVisibleData,
        WindowVisibilityState& sessionVisibleState);
    std::string GetFloatWidth(const int width, float value);

    /*
     * Window Pipeline
     */
    void ProcessFocusZOrderChange(uint32_t dirty);
    void PostProcessFocus();
    void PostProcessProperty(uint32_t dirty);

    /*
     * Window Lifecycle
     */
    bool isUserAuthPassed_ {false};
    NotifySceneSessionDestructFunc onSceneSessionDestruct_;
    NotifyTransferSessionToTargetScreenFunc onTransferSessionToTargetScreen_;
    std::unordered_map<std::string, std::unordered_map<ControlAppType, ControlInfo>> allAppUseControlMap_;
    std::unordered_map<int32_t, std::shared_ptr<RequestTaskInfo>> requestTaskInfoMap;
    std::mutex requestTaskInfoMapMutex_;
    sptr<SceneSession> GetSceneSessionBySessionInfo(const SessionInfo& sessionInfo);
    void CreateRootSceneSession();
    void RegisterRootSceneSession();
    void InitSceneSession(sptr<SceneSession>& sceneSession, const SessionInfo& sessionInfo,
        const sptr<WindowSessionProperty>& property) REQUIRES(SCENE_GUARD);
    void InitFbWindow(const sptr<SceneSession>& sceneSession, const sptr<WindowSessionProperty>& property);
    void RegisterSessionExceptionFunc(const sptr<SceneSession>& sceneSession);
    void NotifySessionForeground(const sptr<SceneSession>& session, uint32_t reason, bool withAnimation);
    void NotifySessionBackground(const sptr<SceneSession>& session, uint32_t reason, bool withAnimation,
        bool isFromInnerkits);
    std::vector<AppExecFwk::SupportWindowMode> ExtractSupportWindowModeFromMetaData(
        const std::shared_ptr<AppExecFwk::AbilityInfo>& abilityInfo);
    std::vector<AppExecFwk::SupportWindowMode> ParseWindowModeFromMetaData(
        const std::string& supportModesInFreeMultiWindow);
    WSError ClearSession(sptr<SceneSession> sceneSession);
    bool IsSessionClearable(sptr<SceneSession> sceneSession);
    void GetAllClearableSessions(std::vector<sptr<SceneSession>>& sessionVector);
    sptr<AAFwk::SessionInfo> SetAbilitySessionInfo(const sptr<SceneSession>& sceneSession,
        int32_t requestId = DEFAULT_REQUEST_FROM_SCB_ID, bool useRequestTaskInfo = false);
    void ResetWantInfo(const sptr<SceneSession>& sceneSession);
    void ResetSceneSessionInfoWant(const sptr<AAFwk::SessionInfo>& sceneSessionInfo);
    int32_t StartUIAbilityBySCBTimeoutCheck(const sptr<SceneSession>& sceneSession,
        const sptr<AAFwk::SessionInfo>& abilitySessionInfo,
        const uint32_t& windowStateChangeReason, bool& isColdStart);
    void ResetSessionInfoAfterStartUIAbility(const sptr<SceneSession>& sceneSession);
    sptr<SceneSession> GetHookedSessionByModuleName(const SessionInfo& sessionInfo);
    void RegisterHookSceneSessionActivationFunc(const sptr<SceneSession>& sceneSession);
    void SetSessionInfoStartWindowType(const sptr<SceneSession>& sceneSession);
    void RegisterSceneSessionDestructNotifyManagerFunc(const sptr<SceneSession>& sceneSession);
    void ResetSceneMissionInfo(const sptr<AAFwk::SessionInfo>& abilitySessionInfo);
    void UpdateAbilityHookState(sptr<SceneSession>& sceneSession, bool isAbilityHook);
    void CloseAllFd(std::shared_ptr<AAFwk::Want>& want);
    WMError CheckWindowIds(
        const std::vector<int32_t>& windowIds, const sptr<IRemoteObject>& callback);
    void RegisterWindowStateErrorCallbackToMMI();
    std::unordered_map<std::string, std::unordered_set<int32_t>> appsWithBoundSystemTrayMap_;

    /*
     * Window Focus
     */
    std::mutex highlightIdsMutex_;
    std::unordered_set<int32_t> highlightIds_;
    std::vector<std::pair<int32_t, sptr<SceneSession>>> GetSceneSessionVector(CmpFunc cmp);
    void TraverseSessionTree(TraverseFunc func, bool isFromTopToBottom);
    void TraverseSessionTreeFromTopToBottom(TraverseFunc func);
    void TraverseSessionTreeFromBottomToTop(TraverseFunc func);

    /*
     * Window Focus
     */
    WSError RequestSessionFocus(int32_t persistentId, bool byForeground = true,
        FocusChangeReason reason = FocusChangeReason::DEFAULT);
    WSError RequestSessionFocusImmediately(int32_t persistentId, bool blockNotifyUntilVisible = true);
    WSError RequestSessionUnfocus(int32_t persistentId, FocusChangeReason reason = FocusChangeReason::DEFAULT,
        DisplayId displayId = DISPLAY_ID_INVALID);
    WSError RequestAllAppSessionUnfocusInner();
    WSError RequestSessionFocusCheck(const sptr<SceneSession>& sceneSession, const sptr<FocusGroup>& focusGroup,
        int32_t persistentId, bool byForeground, FocusChangeReason reason);
    WSError RequestFocusBasicCheck(int32_t persistentId, const sptr<FocusGroup>& focusGroup);
    bool CheckLastFocusedAppSessionFocus(const sptr<SceneSession>& focusedSession,
        const sptr<SceneSession>& nextSession);
    WSError RequestFocusSpecificCheck(DisplayId displayId, const sptr<SceneSession>& sceneSession, bool byForeground,
        FocusChangeReason reason = FocusChangeReason::DEFAULT);
    bool CheckTopmostWindowFocus(const sptr<SceneSession>& focusedSession, const sptr<SceneSession>& sceneSession);
    bool CheckRequestFocusImmediately(const sptr<SceneSession>& sceneSession);
    bool CheckRequestFocusSubWindowImmediately(const sptr<SceneSession>& sceneSession);
    bool CheckFocusIsDownThroughBlockingType(const sptr<SceneSession>& requestSceneSession,
        const sptr<SceneSession>& focusedSession, bool includingAppSession);
    bool CheckClickFocusIsDownThroughFullScreen(const sptr<SceneSession>& focusedSession,
        const sptr<SceneSession>& sceneSession, FocusChangeReason reason);
    bool IsBlockingFocusWindowType(const sptr<SceneSession>& sceneSession) const;
    bool IsParentSessionVisible(const sptr<SceneSession>& session);
    sptr<SceneSession> GetNextFocusableSession(DisplayId displayId, int32_t persistentId);
    sptr<SceneSession> GetTopFloatingSession(DisplayId displayGroupId, int32_t persistentId);
    sptr<SceneSession> GetNextFocusableSessionWhenFloatWindowExist(DisplayId displayGroupId, int32_t persistentId);
    sptr<SceneSession> GetTopNearestBlockingFocusSession(DisplayId displayId, uint32_t zOrder,
        bool includingAppSession);
    sptr<SceneSession> GetTopFocusableNonAppSession();
    bool CheckBlockingFocus(const sptr<SceneSession>& session, bool includingAppSession);
    WSError ShiftFocus(DisplayId displayId, const sptr<SceneSession>& nextSession, bool isProactiveUnfocus,
        FocusChangeReason reason = FocusChangeReason::DEFAULT);
    void UpdateFocusStatus(DisplayId displayId, const sptr<SceneSession>& sceneSession, bool isFocused,
        const sptr<FocusNotifyInfo>& focusNotifyInfo);
    void NotifyFocusStatus(const sptr<SceneSession>& sceneSession, bool isFocused, const sptr<FocusGroup>& focusGroup,
        const sptr<FocusNotifyInfo>& focusNotifyInfo);
    sptr<FocusNotifyInfo> GetFocusNotifyInfo(DisplayId displayId, const sptr<SceneSession>& nextSession);
    int32_t NotifyRssThawApp(const int32_t uid, const std::string& bundleName, const std::string& reason);
    void NotifyFocusStatusByMission(const sptr<SceneSession>& prevSession, const sptr<SceneSession>& currSession);
    void NotifyUnFocusedByMission(const sptr<SceneSession>& sceneSession);
    void NotifyFocusedByMission(const sptr<SceneSession>& sceneSession);
    bool MissionChanged(const sptr<SceneSession>& prevSession, const sptr<SceneSession>& currSession);
    std::string GetAllSessionFocusInfo();
    void RegisterRequestFocusStatusNotifyManagerFunc(const sptr<SceneSession>& sceneSession);
    void ProcessUpdateLastFocusedAppId(const std::vector<std::pair<uint32_t, uint32_t>>& zOrderList);
    WSError ProcessModalTopmostRequestFocusImmediately(const sptr<SceneSession>& sceneSession);
    WSError ProcessSubWindowRequestFocusImmediately(const sptr<SceneSession>& sceneSession);
    WSError ProcessDialogRequestFocusImmediately(const sptr<SceneSession>& sceneSession);

    void RegisterGetStateFromManagerFunc(sptr<SceneSession>& sceneSession);
    void RegisterSessionChangeByActionNotifyManagerFunc(sptr<SceneSession>& sceneSession);

    WSError DestroyDialogWithMainWindow(const sptr<SceneSession>& sceneSession) REQUIRES(SCENE_GUARD);
    sptr<SceneSession> FindMainWindowWithToken(sptr<IRemoteObject> targetToken);
    WSError UpdateParentSessionForDialog(const sptr<SceneSession>& sceneSession, sptr<WindowSessionProperty> property);
    void UpdateCameraFloatWindowStatus(uint32_t accessTokenId, bool isShowing);
    void UpdateFocusableProperty(int32_t persistentId);
    WMError UpdateTopmostProperty(const sptr<WindowSessionProperty>& property, const sptr<SceneSession>& sceneSession);
    std::vector<sptr<SceneSession>> GetSceneSessionVectorByTypeAndDisplayId(WindowType type, uint64_t displayId);
    std::vector<sptr<SceneSession>> GetSceneSessionVectorByType(WindowType type);
    void UpdateOccupiedAreaIfNeed(int32_t persistentId);
    void NotifyMMIWindowPidChange(int32_t windowId, bool startMoving);
    void UpdateHighlightStatus(DisplayId displayId, const sptr<SceneSession>& preSceneSession,
        const sptr<SceneSession>& currSceneSession, bool isProactiveUnfocus);
    void SetHighlightSessionIds(const sptr<SceneSession>& sceneSession, bool needBlockHighlightNotify,
        int64_t timeStamp);
    void AddHighlightSessionIds(const sptr<SceneSession>& sceneSession, bool needBlockHighlightNotify);
    void RemoveHighlightSessionIds(const sptr<SceneSession>& sceneSession);
    std::string GetHighlightIdsStr();

    /*
     * PC Window
     */
    WMError ShiftAppWindowPointerEventInner(
        int32_t sourceWindowId, int32_t targetWindowId, DisplayId targetDisplayId, int32_t fingerId);

    /*
     * Window Animation
     */
    WMError CreateUIEffectController(const sptr<IUIEffectControllerClient>& controllerClient,
        sptr<IUIEffectController>& controller, int32_t& controllerId) override;

    /*
     * Sub Window
     */
    WMError SetParentWindowInner(const sptr<SceneSession>& subSession,
        const sptr<SceneSession>& oldParentSession, const sptr<SceneSession>& newParentSession);

    /*
     * Window Immersive
     */
    void UpdateAvoidSessionAvoidArea(WindowType type);
    void UpdateNormalSessionAvoidArea(int32_t persistentId, const sptr<SceneSession>& sceneSession, bool& needUpdate);
    void UpdateAvoidArea(int32_t persistentId);
    void UpdateRootSceneSessionAvoidArea(int32_t persistentId, bool& needUpdate);
    void NotifyWindowSystemBarPropertyChange(WindowType type, const SystemBarProperty& systemBarProperty);

    WSError IsLastFrameLayoutFinished(bool& isLayoutFinished);
    void HandleSpecificSystemBarProperty(WindowType type, const sptr<WindowSessionProperty>& property,
        const sptr<SceneSession>& sceneSession);

    sptr<AppExecFwk::IBundleMgr> GetBundleManager();
    std::shared_ptr<Global::Resource::ResourceManager> GetResourceManager(const AppExecFwk::AbilityInfo& abilityInfo,
        Global::Resource::ColorMode colorMode = Global::Resource::ColorMode::COLOR_MODE_NOT_SET);

    bool CheckIsRemote(const std::string& deviceId);
    bool GetLocalDeviceId(std::string& localDeviceId);
    std::string AnonymizeDeviceId(const std::string& deviceId);
    int GetRemoteSessionInfos(const std::string& deviceId, int32_t numMax,
                              std::vector<SessionInfoBean>& sessionInfos);
    int GetRemoteSessionInfo(const std::string& deviceId, int32_t persistentId, SessionInfoBean& sessionInfo);
    WSError GetTotalUITreeInfo(std::string& dumpInfo);

    void PerformRegisterInRequestSceneSession(sptr<SceneSession>& sceneSession);
    WSError RequestSceneSessionActivationInner(sptr<SceneSession>& sceneSession, bool isNewActive,
        int32_t requestId = DEFAULT_REQUEST_FROM_SCB_ID) REQUIRES(SCENE_GUARD);
    WSError SetBrightness(const sptr<SceneSession>& sceneSession, float brightness);
    void PostBrightnessTask(float brightness);
    WSError UpdateBrightness(int32_t persistentId);
    void SetDisplayBrightness(float brightness);
    float GetDisplayBrightness() const;
    void HandleHideNonSystemFloatingWindows(const sptr<WindowSessionProperty>& property,
        const sptr<SceneSession>& sceneSession);
    void UpdateForceHideState(const sptr<SceneSession>& sceneSession, const sptr<WindowSessionProperty>& property,
        bool add);
    void NotifyWindowInfoChange(int32_t persistentId, WindowUpdateType type);
    void NotifyWindowInfoChangeFromSession(int32_t persistentid);
    bool FillWindowInfo(std::vector<sptr<AccessibilityWindowInfo>>& infos,
        const sptr<SceneSession>& sceneSession);
    std::vector<std::pair<uint64_t, WindowVisibilityState>> GetWindowVisibilityChangeInfo(
        std::vector<std::pair<uint64_t, WindowVisibilityState>>& currVisibleData);
    void GetWindowLayerChangeInfo(std::shared_ptr<RSOcclusionData> occlusionData,
        std::vector<std::pair<uint64_t, WindowVisibilityState>>& currVisibleData,
        std::vector<std::pair<uint64_t, bool>>& currDrawingContentData);
    void WindowLayerInfoChangeCallback(std::shared_ptr<RSOcclusionData> occlusiontionData);
    sptr<SceneSession> SelectSesssionFromMap(const uint64_t& surfaceId);
    void WindowDestroyNotifyVisibility(const sptr<SceneSession>& sceneSession);
    void RegisterSessionSnapshotFunc(const sptr<SceneSession>& sceneSession);

    /*
     * Window Property
     */
    std::unordered_map<std::string, std::unordered_set<int32_t>> releasedScreenLockMap_;
    std::vector<std::pair<uint64_t, bool>> GetWindowDrawingContentChangeInfo(
        const std::vector<std::pair<uint64_t, bool>>& currDrawingContentData);
    bool GetPreWindowDrawingState(uint64_t surfaceId, bool currentWindowDrawing, int32_t& pid);
    bool GetProcessDrawingState(uint64_t surfaceId, int32_t pid);
    void UpdateWindowDrawingData(uint64_t surfaceId, int32_t pid, int32_t uid);
    bool GetSpecifiedDrawingData(uint64_t surfaceId, int32_t& pid, int32_t& uid);
    void RemoveSpecifiedDrawingData(uint64_t surfaceId);
    WMError ReleaseScreenLockForApp(const std::string& bundleName);
    WMError RelockScreenLockForApp(const std::string& bundleName);
    void GetAllSessionsToReleaseScreenLock(
        std::vector<sptr<SceneSession>>& sessionsToReleaseScreenLock, const std::string& bundleName);
    bool FilterForListWindowInfo(const WindowInfoOption& windowInfoOption,
        const sptr<SceneSession>& sceneSession) const;
    void FilterForGetAllWindowLayoutInfo(DisplayId displayId, bool isVirtualDisplay,
        std::vector<sptr<SceneSession>>& filteredSessions);
    bool IsGetWindowLayoutInfoNeeded(const sptr<SceneSession>& session) const;
    int32_t GetFoldLowerScreenPosY() const;
    bool IsSessionInSpecificDisplay(const sptr<SceneSession>& session, DisplayId displayId) const;
    DisplayId UpdateSpecificSessionClientDisplayId(const sptr<WindowSessionProperty>& property);
    void SetSkipEventOnCastPlusInner(int32_t windowId, bool isSkip);
    void UpdateSessionDisplayIdBySessionInfo(sptr<SceneSession> sceneSession, const SessionInfo& sessionInfo);
    void CheckFloatWindowIsAnco(pid_t pid, const sptr<SceneSession>& newSession);
    bool IsNeedUpdateBrightness(int32_t persistentId, float brightness);
    void RegisterSessionPropertyChangeNotifyManagerFunc(const sptr<SceneSession>& sceneSession);
    void NotifySessionPropertyChangeFromSession(int32_t persistentId, WindowInfoKey windowInfoKey);
    WMError GetRootUIContentRemoteObjInner(DisplayId displayId, sptr<IRemoteObject>& uiContentRemoteObj);

    /*
     * Window Rotate Animation
     */
    void RegisterAcquireRotateAnimationConfigFunc(const sptr<SceneSession>& sceneSession);

    void RegisterVisibilityChangedDetectFunc(const sptr<SceneSession>& sceneSession);
    void NotifySessionForCallback(const sptr<SceneSession>& sceneSession, const bool needRemoveSession,
        LifeCycleChangeReason reason = LifeCycleChangeReason::DEFAULT);
    void AddClientDeathRecipient(const sptr<ISessionStage>& sessionStage, const sptr<SceneSession>& sceneSession);
    void DestroySpecificSession(const sptr<IRemoteObject>& remoteObject);
    bool GetExtensionWindowIds(const sptr<IRemoteObject>& token, int32_t& persistentId, int32_t& parentId);
    void DestroyExtensionSession(const sptr<IRemoteObject>& remoteExtSession, bool isConstrainedModal = false);
    void EraseSceneSessionMapById(int32_t persistentId);
    void EraseSceneSessionAndMarkDirtyLocked(int32_t persistentId);
    WSError GetAbilityInfosFromBundleInfo(const std::vector<AppExecFwk::BundleInfo>& bundleInfos,
        std::vector<SCBAbilityInfo>& scbAbilityInfos, int32_t userId = 0);
    void GetOrientationFromResourceManager(AppExecFwk::AbilityInfo& abilityInfo);
    void UpdatePrivateStateAndNotifyForAllScreens();

    void ClosePipWindowIfExist(WindowType type);
    void NotifySessionAINavigationBarChange(int32_t persistentId);
    void ReportWindowProfileInfos();
    std::string FillWindowProfileInfo(const OHOS::sptr<OHOS::Rosen::SceneSession>& currSession, int32_t focusWindowId);
    void UpdateCameraWindowStatus(uint32_t accessTokenId, bool isShowing);
    void GetAllSceneSessionForAccessibility(std::vector<sptr<SceneSession>>& sceneSessionList);
    void FillAccessibilityInfo(std::vector<sptr<SceneSession>>& sceneSessionList,
        std::vector<sptr<AccessibilityWindowInfo>>& accessibilityInfo);
    void FilterSceneSessionCovered(std::vector<sptr<SceneSession>>& sceneSessionList);
    bool SubtractIntersectArea(std::shared_ptr<SkRegion>& unaccountedSpace, const sptr<SceneSession>& sceneSession);
    void NotifyAllAccessibilityInfo();
    void RegisterSecSurfaceInfoListener();
    void RegisterConstrainedModalUIExtInfoListener();
    void UpdatePiPWindowStateChanged(const std::string& bundleName, bool isForeground);

    /*
     * UIExtension
     */
    void DestroyUIServiceExtensionSubWindow(const sptr<SceneSession>& sceneSession);
    WSError CheckSubSessionStartedByExtension(const sptr<IRemoteObject>& token,
        const sptr<WindowSessionProperty>& property);
    void SetExtensionSubSessionDisplayId(const sptr<WindowSessionProperty>& property,
        const sptr<ISessionStage>& sessionStage);
    void ReportSubWindowCreationFailure(const int32_t& pid, const std::string& abilityName,
        const std::string& parentBundleName, const std::string& hostBundleName);

    /*
     * Multi User
     */
    bool IsPcSceneSessionLifecycle(const sptr<SceneSession>& sceneSession);
    bool IsNeedChangeLifeCycleOnUserSwitch(const sptr<SceneSession>& sceneSession, int32_t pid);
    WSError StartOrMinimizeUIAbilityBySCB(const sptr<SceneSession>& sceneSession, bool isUserActive);
    void ProcessUIAbilityOnUserSwitch(bool isUserActive);
    void HandleUserSwitching(bool isUserActive);
    void HandleUserSwitched(bool isUserActive);
    FlushWindowInfoTask CreateDelayedFlushWindowInfoToMMITask();
    void StartDelayedFlushWindowInfoToMMITask();
    void StopDelayedFlushWindowInfoToMMITask();

    /*
     * Window Recover
     */
    bool IsWindowSupportCacheForRecovering(const sptr<SceneSession>& sceneSession,
        const sptr<WindowSessionProperty>& property);
    void CacheSpecificSessionForRecovering(const sptr<SceneSession>& sceneSession,
        const sptr<WindowSessionProperty>& property);
    void RecoverCachedSubSession(int32_t persistentId);
    void RecoverCachedDialogSession(int32_t persistentId);
    void RemoveFailRecoveredSession();
    void ClearUnrecoveredSessions(const std::vector<int32_t>& recoveredPersistentIds) REQUIRES(SCENE_GUARD);
    void RecoverSessionInfo(const sptr<WindowSessionProperty>& property);
    bool IsNeedRecover(const int32_t persistentId);
    WSError CheckSessionPropertyOnRecovery(const sptr<WindowSessionProperty>& property, bool isSpecificSession);
    void UpdateRecoverPropertyForSuperFold(const sptr<WindowSessionProperty>& property);
    void RegisterSessionRecoverStateChangeListener();
    void RegisterRecoverStateChangeListener();
    void OnSessionRecoverStateChange(const SessionRecoverState& state,
        const sptr<WindowSessionProperty>& property);
    void OnRecoverStateChange(const RecoverState& state);
    NotifySessionRecoverStateChangeFunc sessionRecoverStateChangeFunc_;
    NotifyRecoverStateChangeFunc recoverStateChangeFunc_;

    /*
     * Gesture Back
     */
    void UpdateGestureBackEnabled(int32_t curId, int32_t nextId = INVALID_SESSION_ID);
    std::unordered_set<int32_t> gestureBackEnableWindowIdSet_; // ONLY Accessed on OS_sceneSession thread

    /*
     * Window Input Event
     */
    NotifyWatchGestureConsumeResultFunc onWatchGestureConsumeResultFunc_;
    NotifyWatchFocusActiveChangeFunc onWatchFocusActiveChangeFunc_;

    sptr<RootSceneSession> rootSceneSession_;
    std::weak_ptr<AbilityRuntime::Context> rootSceneContextWeak_;
    mutable std::shared_mutex sceneSessionMapMutex_;
    std::map<int32_t, sptr<SceneSession>> sceneSessionMap_;
    std::map<int32_t, sptr<SceneSession>> systemTopSceneSessionMap_;
    std::map<int32_t, sptr<SceneSession>> nonSystemFloatSceneSessionMap_;
    sptr<ScbSessionHandler> scbSessionHandler_;
    std::shared_ptr<SessionListenerController> listenerController_;
    std::unordered_map<sptr<IRemoteObject>, int32_t, SptrHash<IRemoteObject>> remoteObjectMap_;

    /*
     * UIExtension
     */
    std::unordered_map<sptr<IRemoteObject>, sptr<IRemoteObject>, SptrHash<IRemoteObject>> remoteExtSessionMap_;
    std::unordered_map<sptr<IRemoteObject>,
        ExtensionWindowAbilityInfo, SptrHash<IRemoteObject>> extSessionInfoMap_;
    std::unordered_map<int32_t, ExtensionWindowFlags> extWindowFlagsMap_;
    ExtensionWindowFlags combinedExtWindowFlags_ { 0 };
    std::atomic_bool shouldHideNonSecureFloatingWindows_ { false };
    std::atomic_bool specialExtWindowHasPrivacyMode_ { false };
    bool lastWaterMarkShowState_ { false };
    sptr<AgentDeathRecipient> extensionDeath_ = new AgentDeathRecipient(
        [this](const sptr<IRemoteObject>& remoteExtSession) { this->DestroyExtensionSession(remoteExtSession); });

    std::set<int32_t> touchOutsideListenerSessionSet_;
    std::set<int32_t> windowVisibilityListenerSessionSet_;
    std::unordered_set<int32_t> occlusionStateListenerSessionSet_;

    NotifyCreateSystemSessionFunc createSystemSessionFunc_;
    NotifyCreateKeyboardSessionFunc createKeyboardSessionFunc_;
    ProcessStatusBarEnabledChangeFunc statusBarEnabledChangeFunc_;
    ProcessGestureNavigationEnabledChangeFunc gestureNavigationEnabledChangeFunc_;
    ProcessOutsideDownEventFunc outsideDownEventFunc_;
    ProcessShiftFocusFunc shiftFocusFunc_;
    NotifySCBAfterUpdateFocusFunc notifySCBAfterFocusedFunc_;
    NotifySCBAfterUpdateFocusFunc notifySCBAfterUnfocusedFunc_;
    NotifyDiffSCBAfterUpdateFocusFunc notifyDiffSCBAfterUnfocusedFunc_;
    ProcessStartUIAbilityErrorFunc startUIAbilityErrorFunc_;
    DumpRootSceneElementInfoFunc dumpRootSceneFunc_;
    DumpUITreeFunc dumpUITreeFunc_;
    ProcessVirtualPixelRatioChangeFunc processVirtualPixelRatioChangeFunc_ = nullptr;
    ProcessCloseTargetFloatWindowFunc closeTargetFloatWindowFunc_;
    SetForegroundWindowNumFunc setForegroundWindowNumFunc_;
    MinimizeByWindowIdFunc minimizeByWindowIdFunc_;
    FindScenePanelRsNodeByZOrderFunc findScenePanelRsNodeByZOrderFunc_;
    AppWindowSceneConfig appWindowSceneConfig_;

    /*
     * Window Recover
     */
    bool recoveringFinished_ = false;
    NotifyRecoverSceneSessionFunc recoverSceneSessionFunc_;
    std::set<int32_t> failRecoveredPersistentIdSet_;

    /*
     * Window Rotate Animation
     */
    RotateAnimationConfig rotateAnimationConfig_;
    NotifySupportRotationRegisteredFunc supportRotationRegisteredListener_;

    /*
     * PiP Window
     */
    NotifyStartPiPFailedFunc startPiPFailedFunc_;

    SystemSessionConfig systemConfig_;
    float snapshotScale_ = 0.5;
    int32_t brightnessSessionId_ = INVALID_SESSION_ID;
    float displayBrightness_ = UNDEFINED_BRIGHTNESS;
    bool isScreenLocked_ { false };
    bool isPrepareTerminateEnable_ { false };

    /*
     * Window Focus
     */
    sptr<WindowFocusController> windowFocusController_;
    FocusChangeReason focusChangeReason_ = FocusChangeReason::DEFAULT;
    bool needBlockNotifyFocusStatusUntilForeground_ { false };
    bool needBlockNotifyUnfocusStatus_ { false };

    /*
     * Window Hierarchy
     */
    NotifySetSpecificWindowZIndexFunc setSpecificWindowZIndexFunc_;
    std::unordered_map<WindowType, int32_t> specificZIndexByPidMap_;
    std::mutex specificZIndexByPidMapMutex_;

    /*
     * DFX
     */
    bool openDebugTrace_ { false };

    std::atomic<bool> enableInputEvent_ = true;
    std::vector<int32_t> alivePersistentIds_ = {};
    std::vector<VisibleWindowNumInfo> lastInfo_ = {};
    std::shared_mutex lastInfoMutex_;

    std::shared_ptr<AppExecFwk::EventHandler> mainHandler_;
    std::shared_ptr<TaskScheduler> taskScheduler_;
    sptr<AppExecFwk::IBundleMgr> bundleMgr_;
    sptr<AppAnrListener> appAnrListener_;
    sptr<AppExecFwk::LauncherService> launcherService_;
    std::shared_mutex startingWindowMapMutex_;
    const size_t MAX_CACHE_COUNT = 100;
    std::map<std::string, std::map<std::string, StartingWindowInfo>> startingWindowMap_;
    std::shared_mutex preLoadstartingWindowMapMutex_;
    std::unordered_map<std::string, std::shared_ptr<Media::PixelMap>> preLoadStartingWindowMap_;
    std::shared_mutex startingWindowColorFromAppMapMutex_;
    std::unordered_map<std::string, std::unordered_map<std::string, uint32_t>> startingWindowColorFromAppMap_;
    std::unordered_map<std::string, AppForceLandscapeConfig> appForceLandscapeMap_;
    std::shared_mutex appForceLandscapeMutex_;

    std::mutex privacyBundleMapMutex_;
    std::unordered_map<DisplayId, std::unordered_set<std::string>> privacyBundleMap_;

    WindowModeType lastWindowModeType_ { WindowModeType::WINDOW_MODE_OTHER };

    /*
     * Multi User
     */
    static constexpr int32_t DEFAULT_USERID = -1;
    std::atomic<int32_t> currentUserId_ { DEFAULT_USERID };
    std::atomic<bool> isUserBackground_ { false };

    std::atomic<int32_t> flushWindowInfoCount_ { 0 };
    std::atomic<bool> isDelayFlushWindowInfoMode_ { false };

    // displayRegionMap_ stores the screen display area for AccessibilityNotification,
    // the read and write operations must be performed in the same thread, current is in task thread.
    std::unordered_map<DisplayId, std::shared_ptr<SkRegion>> displayRegionMap_;
    std::shared_ptr<SkRegion> GetDisplayRegion(DisplayId displayId);
    void UpdateDisplayRegion(const sptr<DisplayInfo>& displayInfo);

    std::shared_ptr<AppExecFwk::EventRunner> eventLoop_;
    std::shared_ptr<AppExecFwk::EventHandler> eventHandler_;
    bool isReportTaskStart_ = false;
    std::vector<std::pair<uint64_t, WindowVisibilityState> > lastVisibleData_;
    RSInterfaces& rsInterface_;
    void RegisterSessionStateChangeNotifyManagerFunc(sptr<SceneSession>& sceneSession);
    void RegisterSessionInfoChangeNotifyManagerFunc(sptr<SceneSession>& sceneSession);
    void RegisterDisplayIdChangedNotifyManagerFunc(const sptr<SceneSession>& sceneSession);
    void OnSessionStateChange(int32_t persistentId, const SessionState& state) REQUIRES(SCENE_GUARD);
    void ProcessFocusWhenForeground(sptr<SceneSession>& sceneSession);
    void ProcessFocusWhenForegroundScbCore(sptr<SceneSession>& sceneSession);
    void ProcessSubSessionForeground(sptr<SceneSession>& sceneSession);
    void ProcessSubSessionBackground(sptr<SceneSession>& sceneSession);
    sptr<SceneSession> FindSessionByToken(const sptr<IRemoteObject>& token,
        WindowType type = WindowType::APP_MAIN_WINDOW_BASE);

    void CheckAndNotifyWaterMarkChangedResult();
    WSError NotifyWaterMarkFlagChangedResult(bool hasWaterMark);
    void ProcessPreload(const AppExecFwk::AbilityInfo& abilityInfo) const;
    WindowChangedFunc WindowChangedFunc_;
    sptr<AgentDeathRecipient> windowDeath_ = new AgentDeathRecipient(
        [this](const sptr<IRemoteObject>& remoteObject) { this->DestroySpecificSession(remoteObject); });

    int GetRemoteSessionSnapshotInfo(const std::string& deviceId, int32_t sessionId,
                                     AAFwk::MissionSnapshot& sessionSnapshot);

    /*
     * Collaborator
     */
    AbilityManagerCollaboratorRegisteredFunc abilityManagerCollaboratorRegisteredFunc_;
    const int32_t BROKER_UID = 5557;
    const int32_t BROKER_RESERVE_UID = 5005;
    std::shared_mutex collaboratorMapLock_;
    std::unordered_map<int32_t, sptr<AAFwk::IAbilityManagerCollaborator>> collaboratorMap_;
    std::atomic<int64_t> containerStartAbilityTime_ { 0 };
    sptr<AgentDeathRecipient> collaboratorDeathRecipient_;
    BrokerStates NotifyStartAbility(
        int32_t collaboratorType, const SessionInfo& sessionInfo, int32_t persistentId = 0);
    void NotifySessionCreate(const sptr<SceneSession> sceneSession, const SessionInfo& sessionInfo);
    void NotifyLoadAbility(int32_t collaboratorType, sptr<AAFwk::SessionInfo> abilitySessionInfo,
        std::shared_ptr<AppExecFwk::AbilityInfo> abilityInfo);
    void NotifyUpdateSessionInfo(const sptr<SceneSession> sceneSession);
    void NotifyClearSession(int32_t collaboratorType, int32_t persistentId);
    void NotifyMoveSessionToForeground(int32_t collaboratorType, int32_t persistentId);
    bool PreHandleCollaboratorStartAbility(sptr<SceneSession>& sceneSession, int32_t persistentId = 0);
    bool PreHandleCollaborator(sptr<SceneSession>& sceneSession, int32_t persistentId = 0);
    void NotifyCollaboratorAfterStart(sptr<SceneSession>& sceneSession, sptr<AAFwk::SessionInfo>& sceneSessionInfo);
    void UpdateCollaboratorSessionWant(sptr<SceneSession>& session, int32_t persistentId = 0);
    sptr<AAFwk::IAbilityManagerCollaborator> GetCollaboratorByType(int32_t collaboratorType);
    void GetCollaboratorAbilityInfos(const std::vector<AppExecFwk::BundleInfo>& bundleInfos,
        std::vector<SCBAbilityInfo>& scbAbilityInfos, int32_t userId);

    std::vector<uint64_t> skipSurfaceNodeIds_;
    std::atomic_bool processingFlushUIParams_ { false };

    /*
     * PiP Window
     */
    std::mutex pipSettingSwitchMutex_;
    uint64_t pipWindowSurfaceId_ = 0;
    bool pipSwitchStatus_ = true;
    bool pipIsPipEnabled_ = false;
    std::shared_mutex screenPipEnabledMapLock_;
    std::unordered_map<int32_t, bool> screenPipEnabledMap_;
    std::shared_mutex pipChgListenerMapMutex_;
    std::map<int32_t, sptr<IPipChangeListener>> pipChgListenerMap_;
    bool CheckPiPPriority(const PiPTemplateInfo& pipTemplateInfo, DisplayId displayId = 0);
    std::string GetScreenName(int32_t persistentId);
    bool IsEnablePiPCreate(const sptr<WindowSessionProperty>& property);
    bool IsPiPForbidden(const sptr<WindowSessionProperty>& property, const WindowType& type);
    bool IsLastPiPWindowVisible(uint64_t surfaceId, WindowVisibilityState lastVisibilityState);
    void NotifyPiPWindowVisibleChange(bool isScreenLocked);
    void NotifyMulScreenPipStart(const sptr<WindowSessionProperty>& property, WindowType type);

    /*
     * Floating ball
     */
    WSError IsFloatingBallValid(const sptr<SceneSession>& parentSession);

    void DestroySubSession(const sptr<SceneSession>& sceneSession);
    void DestroyToastSession(const sptr<SceneSession>& sceneSession);
    void NotifyCreateSubSession(int32_t persistentId, sptr<SceneSession> session, uint32_t windowFlags = 0);
    void NotifyCreateToastSession(int32_t persistentId, sptr<SceneSession> session);
    void NotifySessionUnfocusedToClient(int32_t persistentId);
    void NotifyCreateSpecificSession(sptr<SceneSession> session,
        sptr<WindowSessionProperty> property, const WindowType& type);
    sptr<SceneSession> CreateSceneSession(const SessionInfo& sessionInfo, sptr<WindowSessionProperty> property);

    /*
     * Keyboard
     */
    void CreateKeyboardPanelSession(sptr<SceneSession> keyboardSession);
    sptr<SceneSession> RequestKeyboardPanelSession(const std::string& panelName, uint64_t displayId);
    sptr<SceneSession> GetKeyboardSession(DisplayId displayId, bool isSystemKeyboard);
    void HandleKeyboardAvoidChange(const sptr<SceneSession>& sceneSession, DisplayId displayId,
        SystemKeyboardAvoidChangeReason reason);
    void UpdateKeyboardAvoidAreaActive(bool systemKeyboardAvoidAreaActive);

    /*
     * Specific Window
     */
    void ClearSpecificSessionRemoteObjectMap(int32_t persistentId);
    WSError DestroyAndDisconnectSpecificSessionInner(const int32_t persistentId);

    WSError GetAppMainSceneSession(int32_t persistentId, sptr<SceneSession>& sceneSession);
    void CalculateCombinedExtWindowFlags();
    void UpdateSpecialExtWindowFlags(int32_t persistentId, ExtensionWindowFlags flags, ExtensionWindowFlags actions);
    void HideNonSecureFloatingWindows();
    void HideNonSecureSubWindows(const sptr<SceneSession>& sceneSession);
    WSError HandleSecureSessionShouldHide(const sptr<SceneSession>& sceneSession);
    bool CheckSystemWindowPermission(const sptr<WindowSessionProperty>& property);
    bool CheckModalSubWindowPermission(const sptr<WindowSessionProperty>& property);

    /*
     * Window Snapshot
     */
    void SetSessionSnapshotSkipForAppProcess(const sptr<SceneSession>& sceneSession) REQUIRES(SCENE_GUARD);
    void SetSessionSnapshotSkipForAppBundleName(const sptr<SceneSession>& sceneSession) REQUIRES(SCENE_GUARD);

    void HandleSpecialExtWindowFlagsChange(int32_t persistentId, ExtensionWindowFlags extWindowFlags,
        ExtensionWindowFlags extWindowActions);
    void ProcessWindowModeType();
    WindowModeType CheckWindowModeType();
    void NotifyRSSWindowModeTypeUpdate();
    void CacVisibleWindowNum();
    bool IsVectorSame(const std::vector<VisibleWindowNumInfo>& lastInfo,
        const std::vector<VisibleWindowNumInfo>& currentInfo);
    bool IsKeyboardForeground();
    WindowStatus GetWindowStatus(WindowMode mode, SessionState sessionState,
        const sptr<WindowSessionProperty>& property);
    void DeleteStateDetectTask();
    bool JudgeNeedNotifyPrivacyInfo(DisplayId displayId, const std::unordered_set<std::string>& privacyBundles);

    /*
     * Window Visibility
     */
    bool NotifyVisibleChange(int32_t persistentId);

    /*
     * Fold Screen Status Change Report
     */
    WMError MakeScreenFoldData(const std::vector<std::string>& screenFoldInfo, ScreenFoldData& screenFoldData);
    WMError CheckAndReportScreenFoldStatus(ScreenFoldData& data);
    WMError ReportScreenFoldStatus(const ScreenFoldData& data);
    void RecoveryVisibilityPidCount(int32_t pid) REQUIRES(SCENE_GUARD);

    /*
     * Window Watermark
     */
    bool SetSessionWatermarkForAppProcess(const sptr<SceneSession>& sceneSession);
    std::vector<NodeId> GetSessionNodeIdsAndWatermarkNameByPid(int32_t pid, std::string& watermarkName);
    void SetWatermarkForSession(const sptr<SceneSession>& session);
    void ClearWatermarkForSession(const sptr<SceneSession>& session);
    void ClearProcessRecordWhenAppExit(const AppExecFwk::ProcessData& processData);

    /*
     * Window Layout
     */
    struct RsCmdBlockingFlag {
        int32_t count = 0;
        int64_t startTime = 0;
    };
    RsCmdBlockingFlag rsCmdBlockingFlag_;
    std::shared_ptr<VsyncCallback> vsyncCallback_ = nullptr;
    std::shared_ptr<VsyncStation> vsyncStation_ = nullptr;
    SingleHandTransform singleHandTransform_;
    WSRect originRect_;
    WSRect singleHandRect_;
    SingleHandScreenInfo singleHandScreenInfo_;
    GetRSNodeByStringIDFunc getRSNodeByStringIDFunc_;
    SetTopWindowBoundaryByIDFunc setTopWindowBoundaryByIDFunc_;
    SingleHandCompatibleModeConfig singleHandCompatibleModeConfig_;
    std::unordered_set<std::string> appsWithDeduplicatedWindowStatus_;
    std::shared_mutex appHookWindowInfoMapMutex_;
    std::unordered_map<std::string, HookWindowInfo> appHookWindowInfoMap_;
    void InitVsyncStation();
    void BindVsyncStation(const sptr<SceneSession>& sceneSession);
    bool GetDisplaySizeById(DisplayId displayId, int32_t& displayWidth, int32_t& displayHeight);
    void UpdateSessionWithFoldStateChange(DisplayId displayId, SuperFoldStatus status, SuperFoldStatus prevStatus);
    void ConfigSingleHandCompatibleMode(const WindowSceneConfig::ConfigItem& configItem);
    void ConfigAppsWithDeduplicatedWindowStatus();
    void SetWindowStatusDeduplicationBySystemConfig(const SessionInfo& sessionInfo, SystemSessionConfig& systemConfig);
    HasRootSceneRequestedVsyncFunc hasRootSceneRequestedVsyncFunc_;
    RequestVsyncByRootSceneWhenModeChangeFunc requestVsyncByRootSceneWhenModeChangeFunc_;
    WSError HasRootSceneRequestedVsync(bool& hasRootSceneRequestedVsync);
    WSError RequestVsyncByRootSceneWhenModeChange(const std::shared_ptr<VsyncCallback>& vsyncCallback);

    /*
     * Window Snapshot
     */
    std::unordered_set<int32_t> snapshotSkipPidSet_ GUARDED_BY(SCENE_GUARD); // ONLY Accessed on OS_sceneSession thread
    std::unordered_set<std::string> snapshotSkipBundleNameSet_ GUARDED_BY(SCENE_GUARD);

    uint32_t sessionMapDirty_ { 0 };
    std::condition_variable nextFlushCompletedCV_;
    std::mutex nextFlushCompletedMutex_;
    RootSceneProcessBackEventFunc rootSceneProcessBackEventFunc_ = nullptr;

    /*
     * Window Watermark
     */
    std::unordered_map<int32_t, std::string> processWatermarkPidMap_; // ONLY Accessed on OS_sceneSession thread
    std::unordered_map<int32_t, std::string> appWatermarkPidMap_;

    /*
     * Dump
     */
    std::shared_ptr<ScbDumpSubscriber> scbDumpSubscriber_;
    RunnableFuture<std::vector<std::string>> dumpInfoFuture_;
    void DumpSessionInfo(const sptr<SceneSession>& session, std::ostringstream& oss);
    void DumpFocusInfo(std::ostringstream& oss);
    void DumpSessionElementInfo(const sptr<SceneSession>& session,
        const std::vector<std::string>& params, std::string& dumpInfo);
    void DumpAllSessionFocusableInfo(int32_t persistentId);

    /*
     * Screen Manager
     */
    bool IsInDefaultScreen(const sptr<SceneSession>& sceneSession);

    /*
     * Window Mode Type
     */
    bool IsNeedSkipWindowModeTypeCheck(const sptr<SceneSession>& sceneSession, bool isSmallFold);

    /*
     * Window Immersive
     */
    NotifyRootSceneAvoidAreaChangeFunc onNotifyAvoidAreaChangeForRootFunc_;
    OnFlushUIParamsFunc onFlushUIParamsFunc_;
    IsRootSceneLastFrameLayoutFinishedFunc isRootSceneLastFrameLayoutFinishedFunc_;
    std::unordered_map<uint64_t, bool> isAINavigationBarVisible_;
    std::shared_mutex currAINavigationBarAreaMapMutex_;
    std::map<uint64_t, WSRect> currAINavigationBarAreaMap_;
    std::mutex nextAvoidRectInfoMapMutex_;
    std::unordered_map<AvoidAreaType, std::unordered_map<DisplayId, std::pair<WSRect, WSRect>>> nextAvoidRectInfoMap_;
    std::unordered_map<DisplayId, bool> statusBarDefaultVisibilityPerDisplay_;
    std::set<int32_t> avoidAreaListenerSessionSet_;
    static constexpr int32_t INVALID_STATUS_BAR_AVOID_HEIGHT = -1;
    std::unordered_map<DisplayId, int32_t> statusBarAvoidHeight_;
    std::unordered_map<DisplayId, bool> statusBarConstantlyShowMap_;
    std::mutex lastSystemBarPropertyMapMutex_;
    std::unordered_map<WindowType, SystemBarProperty> lastSystemBarPropertyMap_;
    bool GetLSState() const { return isLSState_; }
    void SetLSState(bool isLSState) { isLSState_ = isLSState; }
    bool isLSState_ = false;

    struct SessionInfoList {
        int32_t uid_;
        std::string bundleName_;
        std::string abilityName_;
        std::string moduleName_;
        bool operator==(const SessionInfoList& list) const
        {
            return this->uid_ == list.uid_ && this->bundleName_ == list.bundleName_ &&
                this->abilityName_ == list.abilityName_ && this->moduleName_ == list.moduleName_;
        }
        friend struct SessionHasher;
    };
    struct SessionHasher {
        size_t operator()(const SessionInfoList& sessionInfo) const
        {
            return std::hash<int32_t>()(sessionInfo.uid_) + std::hash<std::string>()(sessionInfo.bundleName_) +
                std::hash<std::string>()(sessionInfo.abilityName_) + std::hash<std::string>()(sessionInfo.moduleName_);
        }
    };
    std::unordered_map<SessionInfoList, std::shared_ptr<AppExecFwk::AbilityInfo>, SessionHasher> abilityInfoMap_;

    /*
     * Window Property
     */
    void UpdateDarkColorModeToRS();
    struct DrawingSessionInfo {
        int32_t pid_ = 0;
        int32_t uid_ = 0;
    };
    uint32_t observedFlags_ = 0;
    uint32_t interestedFlags_ = 0;
    std::unordered_map<uint64_t, DrawingSessionInfo> lastDrawingSessionInfoMap_;
    std::unordered_set<int32_t> screenshotAppEventListenerSessionSet_;
    void NotifyWindowPropertyChangeByWindowInfoKey(
        const sptr<SceneSession>& sceneSession, WindowInfoKey windowInfoKey);
    void NotifyWindowPropertyChange(ScreenId screenId);
    void PackWindowPropertyChangeInfo(const sptr<SceneSession>& sceneSession,
        std::unordered_map<WindowInfoKey, WindowChangeInfoType>& windowPropertyChangeInfo);
    WMError AddSessionBlackList(const std::vector<sptr<SceneSession>>& sceneSessionList,
        const std::unordered_set<std::string>& privacyWindowTags);
    WMError RemoveSessionBlackList(const std::vector<sptr<SceneSession>>& sceneSessionList,
        const std::unordered_set<std::string>& privacyWindowTags);
    void AddskipSurfaceNodeIdSet(int32_t windowId, std::unordered_set<uint64_t>& skipSurfaceNodeIdSet);
    WMError FlushSessionBlackListInfoMapWhenAdd();
    WMError FlushSessionBlackListInfoMapWhenAdd(ScreenId screenId);
    WMError FlushSessionBlackListInfoMapWhenRemove();
    WMError FlushSessionBlackListInfoMapWhenRemove(ScreenId screenId);
    void UpdateVirtualScreenBlackList(ScreenId screenId);
    void AddSkipSurfaceNodeWhenAttach(int32_t windowId, const std::string& bundleName, uint64_t surfaceNodeId);
    std::unordered_map<std::string, std::unordered_set<std::string>> bundleRSBlackListConfigMap_;
    inline static const std::string WMS_DEFAULT = "WMS_DEFAULT";
    sptr<AppExecFwk::IAppMgr> appMgrClient_ = nullptr;
    sptr<AppStateObserver> appStateObserver_ = nullptr;
    void RegisterAppStateObserver();

    struct SessionBlackListInfo {
        int32_t windowId = INVALID_SESSION_ID;
        std::string privacyWindowTag = WMS_DEFAULT;
    };
    struct SessionBlackListEqual  {
        bool operator()(const SessionBlackListInfo& left, const SessionBlackListInfo& right) const
        {
            return left.windowId == right.windowId && left.privacyWindowTag == right.privacyWindowTag;
        }
    };
    struct SessionBlackListHasher  {
        size_t operator()(const SessionBlackListInfo& info) const
        {
            return std::hash<int32_t>()(info.windowId) + std::hash<std::string>()(info.privacyWindowTag);
        }
    };
    using SessionBlackListInfoSet =
        std::unordered_set<SessionBlackListInfo, SessionBlackListHasher, SessionBlackListEqual>;
    SessionBlackListInfoSet sessionRSBlackListConfigSet_;
    std::unordered_map<DisplayId, SessionBlackListInfoSet> sessionBlackListInfoMap_;

    void RemoveSessionFromBlackListInfoSet(
        const sptr<SceneSession>& sceneSession, SessionBlackListInfoSet& sessionBlackListInfoSet);

    struct ScreenBlackListInfo {
        std::string privacyWindowTag = "";
    };
    struct ScreenBlackListEqual {
        bool operator()(const ScreenBlackListInfo& left, const ScreenBlackListInfo& right) const
        {
            return left.privacyWindowTag == right.privacyWindowTag;
        }
    };
    struct ScreenBlackListHasher {
        size_t operator()(const ScreenBlackListInfo& info) const
        {
            return std::hash<std::string>()(info.privacyWindowTag);
        }
    };
    using ScreenBlackListInfoSet =
        std::unordered_set<ScreenBlackListInfo, ScreenBlackListHasher, ScreenBlackListEqual>;
    std::unordered_map<ScreenId, ScreenBlackListInfoSet> screenRSBlackListConfigMap_;

    /*
     * Move Drag
     */
    std::mutex dragResizeTypeMutex_;
    DragResizeType globalDragResizeType_ = DragResizeType::RESIZE_TYPE_UNDEFINED;
    std::unordered_map<std::string, DragResizeType> appDragResizeTypeMap_;
    void GetEffectiveDragResizeType(DragResizeType& dragResizeType);
    WMError GetAppDragResizeTypeInner(const std::string& bundleName, DragResizeType& dragResizeType);
    std::mutex keyFrameMutex_;
    std::unordered_map<std::string, KeyFramePolicy> appKeyFramePolicyMap_;

    /*
     * Specific Window
     */
    std::unordered_map<int32_t, NotifyCreateSubSessionFunc> createSubSessionFuncMap_;
    std::unordered_map<int32_t, std::vector<sptr<SceneSession>>> recoverSubSessionCacheMap_;
    std::unordered_map<int32_t, NotifyBindDialogSessionFunc> bindDialogTargetFuncMap_;
    std::unordered_map<int32_t, std::vector<sptr<SceneSession>>> recoverDialogSessionCacheMap_;

    /*
     * PC Window
     */
    //Whether to save window rect
    std::unordered_map<std::string, bool> isWindowRectAutoSaveMap_;
    //Whether to save rect according to specifiedFlag
    std::unordered_map<std::string, bool> isSaveBySpecifiedFlagMap_;
    std::shared_ptr<FoldScreenStatusChangeCallback> foldChangeCallback_;

    /*
     * Window Lifecycle
     */
    NotifyAppUseControlListFunc notifyAppUseControlListFunc_;
    std::unordered_map<int32_t, int32_t> visibleWindowCountMap_ GUARDED_BY(SCENE_GUARD);
    std::unordered_set<std::string> sessionLockedStateCacheSet_;
    std::shared_ptr<FfrtQueueHelper> ffrtQueueHelper_ = nullptr;
    std::vector<RecentSessionInfo> recentMainSessionInfoList_;
    UpdateKioskAppListFunc updateKioskAppListFunc_;
    KioskModeChangeFunc kioskModeChangeFunc_;
    std::vector<std::string> kioskAppListCache_;
    bool isKioskMode_ = false;
    int32_t kioskAppPersistentId_ = INVALID_SESSION_ID;

    /*
     * Window Pattern
     */
    std::shared_mutex startingWindowFollowAppMapMutex_;
    std::unordered_map<std::string, std::unordered_set<std::string>> startingWindowFollowAppMap_;
    std::unordered_set<std::string> emptyStartupResource_;
    std::atomic<bool> delayRemoveSnapshot_ = false;
    bool syncLoadStartingWindow_ = false;
    bool enableDmaReclaim_ = false;
    void InitWindowPattern();
    void InitStartingWindow();
    void InitDmaReclaimParam();
    void InitStartingWindowRdb(const std::string& rdbPath);
    bool GetStartingWindowInfoFromCache(const SessionInfo& sessionInfo, StartingWindowInfo& startingWindowInfo,
        bool isDark);
    uint32_t UpdateCachedColorToAppSet(const std::string& bundleName, const std::string& moduleName,
        const std::string& abilityName, StartingWindowInfo& startingWindowInfo);
    bool GetStartingWindowInfoFromRdb(const SessionInfo& sessionInfo, StartingWindowInfo& startingWindowInfo,
        bool darkMode);
    bool GetStartWindowColorFollowApp(const SessionInfo& sessionInfo);
    void ClearStartWindowColorFollowApp(const std::string& bundleName);
    bool GetPathInfoFromResource(const std::shared_ptr<Global::Resource::ResourceManager> resourceMgr,
        bool hapPathEmpty, uint32_t resourceId, std::string& path);
    bool CheckStartWindowColorFollowApp(const AppExecFwk::AbilityInfo& abilityInfo,
        const std::shared_ptr<Global::Resource::ResourceManager>& resourceMgr);
    bool GetStartupPageFromResource(const AppExecFwk::AbilityInfo& abilityInfo, StartingWindowInfo& startingWindowInfo,
        Global::Resource::ColorMode defaultColorMode = Global::Resource::ColorMode::COLOR_MODE_NOT_SET,
        bool appColorModeChanged = false,
        Global::Resource::ColorMode appColorMode = Global::Resource::ColorMode::COLOR_MODE_NOT_SET);
    void GetBundleStartingWindowInfos(bool isDark, const AppExecFwk::BundleInfo& bundleInfo,
        std::vector<std::pair<StartingWindowRdbItemKey, StartingWindowInfo>>& outValues);
    void CacheStartingWindowInfo(const std::string& bundleName, const std::string& moduleName,
        const std::string& abilityName, const StartingWindowInfo& startingWindowInfo, bool isDark);
    std::shared_ptr<StartingWindowRdbManager> startingWindowRdbMgr_;
    std::unique_ptr<LruCache> snapshotLruCache_;
    std::size_t snapshotCapacity_ = 0;
    bool GetIconFromDesk(const SessionInfo& sessionInfo, std::string& startupPagePath) const;
    bool GetIsDarkFromConfiguration(const std::string& appColorMode);
    bool needCloseSync_ = false;
    std::function<void()> closeSyncFunc_ = nullptr;
    WMError SetImageForRecent(uint32_t imgResourceId, ImageFit imageFit, int32_t persistentId) override;
    WMError SetImageForRecentPixelMap(const std::shared_ptr<Media::PixelMap>& pixelMap, ImageFit imageFit,
        int32_t persistentId) override;
    WMError RemoveImageForRecent(int32_t persistentId) override;
    void GetCropInfoByDisplaySize(const Media::ImageInfo& imageInfo, Media::DecodeOptions& decodeOpts);

    RecoverState recoverState_ = RecoverState::RECOVER_END;
    OutlineParams recoverOutlineParams_;
    bool needRecoverOutline_ = false;
    sptr<IRemoteObject> outlineRemoteObject_;
    sptr<AgentDeathRecipient> outlineRemoteDeath_ = new AgentDeathRecipient(
        [this](const sptr<IRemoteObject>& remoteObject) { DeleteAllOutline(remoteObject); });
    void UpdateOutlineInner(const sptr<IRemoteObject>& remoteObject, const OutlineParams& outlineParams);
    bool NeedOutline(int32_t persistentId, const std::vector<int32_t>& persistentIdList);
    void AddOutlineRemoteDeathRecipient(const sptr<IRemoteObject>& remoteObject);
    void DeleteAllOutline(const sptr<IRemoteObject>& remoteObject);
    bool CacheOutlineParamsIfNeed(const OutlineParams& outlineParams);

    std::string GetCallerSessionColorMode(const SessionInfo& sessionInfo);
    void NotifySessionScreenLockedChange(bool isScreenLocked);
    void SetBufferAvailable(sptr<SceneSession>& sceneSession);

    /*
     * Compatible Mode
     */
    void NotifyIsFullScreenInForceSplitMode(uint32_t uid, bool isFullScreen);
    std::unordered_set<uint32_t> fullScreenInForceSplitUidSet_;
    void RegisterForceSplitEnableListenerIfMainWindow(const sptr<SceneSession>& sceneSession);
};
} // namespace OHOS::Rosen

#endif // OHOS_ROSEN_WINDOW_SCENE_SCENE_SESSION_MANAGER_H
