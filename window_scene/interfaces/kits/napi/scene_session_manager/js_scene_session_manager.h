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

#ifndef OHOS_WINDOW_SCENE_JS_SCENE_SESSION_MANAGER_H
#define OHOS_WINDOW_SCENE_JS_SCENE_SESSION_MANAGER_H

#include <map>

#include "interfaces/include/ws_common.h"
#include <native_engine/native_engine.h>
#include <native_engine/native_value.h>
#include "interfaces/kits/napi/scene_session_manager/js_scene_utils.h"
#include "root_scene.h"
#include "session/host/include/scene_session.h"
#include "ability_info.h"
#include "task_scheduler.h"
#include "session_manager/include/scene_session_manager.h"
#include "session_manager/include/ui_effect_manager.h"

namespace OHOS::Rosen {
enum class ListenerFunctionType : uint32_t {
    CREATE_SYSTEM_SESSION_CB,
    CREATE_KEYBOARD_SESSION_CB,
    RECOVER_SCENE_SESSION_CB,
    STATUS_BAR_ENABLED_CHANGE_CB,
    OUTSIDE_DOWN_EVENT_CB,
    SHIFT_FOCUS_CB,
    START_UI_ABILITY_ERROR,
    GESTURE_NAVIGATION_ENABLED_CHANGE_CB,
    CLOSE_TARGET_FLOAT_WINDOW_CB,
    ABILITY_MANAGER_COLLABORATOR_REGISTERED_CB,
    START_PIP_FAILED_CB,
    NOTIFY_APP_USE_CONTROL_LIST_CB,
    WATCH_GESTURE_CONSUME_RESULT_CB,
    WATCH_FOCUS_ACTIVE_CHANGE_CB,
    SET_FOREGROUND_WINDOW_NUM_CB,
    MINIMIZE_BY_WINDOW_ID_CB,
    SCENE_SESSION_DESTRUCT_CB,
    SCENE_SESSION_TRANSFER_TO_TARGET_SCREEN_CB,
    UPDATE_KIOSK_APP_LIST_CB,
    KIOSK_MODE_CHANGE_CB,
    UI_EFFECT_SET_PARAMS_CB,
    UI_EFFECT_ANIMATE_TO_CB,
    VIRTUAL_DENSITY_CHANGE_CB,
    SET_SPECIFIC_SESSION_ZINDEX_CB,
    NOTIFY_SUPPORT_ROTATION_REGISTERED_CB,
    MINIMIZE_ALL_CB,
};

class JsSceneSessionManager final {
public:
    explicit JsSceneSessionManager(napi_env env);
    ~JsSceneSessionManager() = default;

    static napi_value Init(napi_env env, napi_value exportObj);
    static void Finalizer(napi_env env, void* data, void* hint);

    static napi_value SetBehindWindowFilterEnabled(napi_env env, napi_callback_info info);
    static napi_value GetRootSceneSession(napi_env env, napi_callback_info info);
    static napi_value ChangeUIAbilityVisibilityBySCB(napi_env env, napi_callback_info info);
    static napi_value RegisterCallback(napi_env env, napi_callback_info info);
    static napi_value GetWindowSceneConfig(napi_env env, napi_callback_info info);
    static napi_value GetSystemConfig(napi_env env, napi_callback_info info);
    static napi_value UpdateRotateAnimationConfig(napi_env env, napi_callback_info info);
    static napi_value ProcessBackEvent(napi_env env, napi_callback_info info);
    static napi_value CheckSceneZOrder(napi_env env, napi_callback_info info);
    static napi_value SetVmaCacheStatus(napi_env env, napi_callback_info info);
    static napi_value InitWithRenderServiceAdded(napi_env env, napi_callback_info info);
    static napi_value GetAllWindowVisibilityInfos(napi_env env, napi_callback_info info);
    static napi_value GetAllAbilityInfos(napi_env env, napi_callback_info info);
    static napi_value GetBatchAbilityInfos(napi_env env, napi_callback_info info);
    static napi_value GetAbilityInfo(napi_env env, napi_callback_info info);
    static napi_value PrepareTerminate(napi_env env, napi_callback_info info);
    static napi_value AsyncPrepareTerminate(napi_env env, napi_callback_info info);
    static napi_value PerfRequestEx(napi_env env, napi_callback_info info);
    static napi_value UpdateWindowMode(napi_env env, napi_callback_info info);
    static napi_value NotifySingleHandInfoChange(napi_env env, napi_callback_info info);
    static napi_value GetSingleHandCompatibleModeConfig(napi_env env, napi_callback_info info);
    static napi_value GetRootSceneUIContext(napi_env env, napi_callback_info info);
    static napi_value SendTouchEvent(napi_env env, napi_callback_info info);
    static napi_value AddWindowDragHotArea(napi_env env, napi_callback_info info);
    static napi_value PreloadInLakeApp(napi_env env, napi_callback_info info);
    static napi_value RequestFocusStatus(napi_env env, napi_callback_info info);
    static napi_value RequestAllAppSessionUnfocus(napi_env env, napi_callback_info info);
    static napi_value SetScreenLocked(napi_env env, napi_callback_info info);
    static napi_value SetUserAuthPassed(napi_env env, napi_callback_info info);
    static napi_value UpdateTitleInTargetPos(napi_env env, napi_callback_info info);
    static napi_value UpdateMaximizeMode(napi_env env, napi_callback_info info);
    static napi_value ReportData(napi_env env, napi_callback_info info);
    static napi_value GetRssData(napi_env env, napi_callback_info info);
    static napi_value RegisterRssData(napi_env env, napi_callback_info info);
    static napi_value UnregisterRssData(napi_env env, napi_callback_info info);
    static napi_value NotifySessionRecoverStatus(napi_env env, napi_callback_info info);
    static napi_value UpdateSessionDisplayId(napi_env env, napi_callback_info info);
    static napi_value NotifyStackEmpty(napi_env env, napi_callback_info info);
    static napi_value SetSystemAnimatedScenes(napi_env env, napi_callback_info info);
    static napi_value GetSessionSnapshotPixelMap(napi_env env, napi_callback_info info);
    static napi_value GetSessionSnapshotPixelMapSync(napi_env env, napi_callback_info info);
    static napi_value SetAppDragResizeType(napi_env env, napi_callback_info info);
    static napi_value GetCustomDecorHeight(napi_env env, napi_callback_info info);
    static napi_value NotifyEnterRecentTask(napi_env env, napi_callback_info info);
    static napi_value NotifySCBRecentStateChange(napi_env env, napi_callback_info info);
    static napi_value UpdateDisplayHookInfo(napi_env env, napi_callback_info info);
    static napi_value UpdateAppHookDisplayInfo(napi_env env, napi_callback_info info);
    static napi_value UpdateAppHookWindowInfo(napi_env env, napi_callback_info info);
    static napi_value NotifyHookOrientationChange(napi_env env, napi_callback_info info);
    static napi_value InitScheduleUtils(napi_env env, napi_callback_info info);
    static napi_value SetAppForceLandscapeConfig(napi_env env, napi_callback_info info);
    static napi_value SwitchFreeMultiWindow(napi_env env, napi_callback_info info);
    static napi_value GetFreeMultiWindowConfig(napi_env env, napi_callback_info info);
    static napi_value IsScbCoreEnabled(napi_env env, napi_callback_info info);
    static napi_value RefreshPcZOrder(napi_env env, napi_callback_info info);
    static napi_value GetWindowPid(napi_env env, napi_callback_info info);
    static napi_value UpdatePcFoldScreenStatus(napi_env env, napi_callback_info info);
    static napi_value UpdateSystemKeyboardStatus(napi_env env, napi_callback_info info);
    static napi_value ResetPcFoldScreenArrangeRule(napi_env env, napi_callback_info info);
    static napi_value SetIsWindowRectAutoSave(napi_env env, napi_callback_info info);
    static napi_value NotifyAboveLockScreen(napi_env env, napi_callback_info info);
    static napi_value CloneWindow(napi_env env, napi_callback_info info);
    static napi_value RegisterSingleHandContainerNode(napi_env env, napi_callback_info info);
    static napi_value NotifyRotationChange(napi_env env, napi_callback_info info);
    static napi_value SupportFollowParentWindowLayout(napi_env env, napi_callback_info info);
    static napi_value SupportFollowRelativePositionToParent(napi_env env, napi_callback_info info);
    static napi_value UpdateRsCmdBlockingCount(napi_env env, napi_callback_info info);
    static napi_value SupportZLevel(napi_env env, napi_callback_info info);
    static napi_value SetSupportFunctionType(napi_env env, napi_callback_info info);
    static napi_value GetApplicationInfo(napi_env env, napi_callback_info info);
    static napi_value SetUIEffectControllerAliveInUI(napi_env env, napi_callback_info info);
    static napi_value SupportCreateFloatWindow(napi_env env, napi_callback_info info);
    static napi_value ApplyFeatureConfig(napi_env env, napi_callback_info info);
    static napi_value NotifySupportRotationChange(napi_env env, napi_callback_info info);
    static napi_value GetAllJsonProfile(napi_env env, napi_callback_info info);
    static napi_value GetJsonProfile(napi_env env, napi_callback_info info);
    static napi_value SetAppForceLandscapeConfigEnable(napi_env env, napi_callback_info info);

    /*
     * PC Window
     */
    static napi_value GetWindowLimits(napi_env env, napi_callback_info info);
    static napi_value SetIsDockAutoHide(napi_env env, napi_callback_info info);
    static napi_value SetTrayAppListInfo(napi_env env, napi_callback_info info);

    /*
     * Multi Instance
     */
    static napi_value GetMaxInstanceCount(napi_env env, napi_callback_info info);
    static napi_value GetInstanceCount(napi_env env, napi_callback_info info);
    static napi_value GetLastInstanceKey(napi_env env, napi_callback_info info);
    static napi_value RefreshAppInfo(napi_env env, napi_callback_info info);

    /*
     * Window Immersive
     */
    static napi_value NotifyAINavigationBarShowStatus(napi_env env, napi_callback_info info);
    static napi_value NotifyNextAvoidRectInfo(napi_env env, napi_callback_info info);
    static napi_value GetIsLayoutFullScreen(napi_env env, napi_callback_info info);
    static napi_value SetMaximizeFullScreen(napi_env env, napi_callback_info info);
    static napi_value SetStatusBarDefaultVisibilityPerDisplay(napi_env env, napi_callback_info info);
    static napi_value NotifyStatusBarShowStatus(napi_env env, napi_callback_info info);
    static napi_value NotifyLSStateChange(napi_env env, napi_callback_info info);
    static napi_value NotifyStatusBarConstantlyShowStatus(napi_env env, napi_callback_info info);
    static napi_value SetStatusBarAvoidHeight(napi_env env, napi_callback_info info);

    /*
     * Multi User
     */
    static napi_value InitUserInfo(napi_env env, napi_callback_info info);
    static napi_value HandleUserSwitch(napi_env env, napi_callback_info info);

    /*
     * Window Pattern
     */
    static napi_value SupportSnapshotAllSessionStatus(napi_env env, napi_callback_info info);
    static napi_value SupportCacheLockedSessionSnapshot(napi_env env, napi_callback_info info);
    static napi_value SupportPreloadStartingWindow(napi_env env, napi_callback_info info);
    static napi_value PreloadStartingWindow(napi_env env, napi_callback_info info);

    /*
     * PiP Window
     */
    static napi_value SetPiPSettingSwitchStatus(napi_env env, napi_callback_info info);
    static napi_value SetIsPipEnabled(napi_env env, napi_callback_info info);
    static napi_value GetPipDeviceCollaborationPolicy(napi_env env, napi_callback_info info);

private:
    napi_value OnSetBehindWindowFilterEnabled(napi_env env, napi_callback_info info);
    napi_value OnRegisterCallback(napi_env env, napi_callback_info info);
    napi_value OnGetRootSceneSession(napi_env env, napi_callback_info info);
    napi_value OnRequestSceneSession(napi_env env, napi_callback_info info);
    napi_value OnUpdateSceneSessionWant(napi_env env, napi_callback_info info);
    napi_value OnRequestSceneSessionActivation(napi_env env, napi_callback_info info);
    napi_value OnRequestSceneSessionBackground(napi_env env, napi_callback_info info);
    napi_value OnRequestSceneSessionDestruction(napi_env env, napi_callback_info info);
    napi_value OnNotifyForegroundInteractiveStatus(napi_env env, napi_callback_info info);
    napi_value OnIsSceneSessionValid(napi_env env, napi_callback_info info);
    napi_value OnRequestSceneSessionByCall(napi_env env, napi_callback_info info);
    napi_value OnStartAbilityBySpecified(napi_env env, napi_callback_info info);
    napi_value OnStartUIAbilityBySCB(napi_env env, napi_callback_info info);
    napi_value OnChangeUIAbilityVisibilityBySCB(napi_env env, napi_callback_info info);
    napi_value OnGetWindowSceneConfig(napi_env env, napi_callback_info info);
    napi_value OnGetSystemConfig(napi_env env, napi_callback_info info);
    napi_value OnUpdateRotateAnimationConfig(napi_env env, napi_callback_info info);
    napi_value OnProcessBackEvent(napi_env env, napi_callback_info info);
    napi_value OnCheckSceneZOrder(napi_env env, napi_callback_info info);
    napi_value OnSetVmaCacheStatus(napi_env env, napi_callback_info info);
    napi_value OnInitWithRenderServiceAdded(napi_env env, napi_callback_info info);
    napi_value OnGetAllWindowVisibilityInfos(napi_env env, napi_callback_info info);
    napi_value OnGetAllAbilityInfos(napi_env env, napi_callback_info info);
    napi_value OnGetAbilityInfo(napi_env env, napi_callback_info info);
    napi_value OnPrepareTerminate(napi_env env, napi_callback_info info);
    napi_value OnAsyncPrepareTerminate(napi_env env, napi_callback_info info);
    napi_value OnPerfRequestEx(napi_env env, napi_callback_info info);
    napi_value OnUpdateWindowMode(napi_env env, napi_callback_info info);
    napi_value OnNotifySingleHandInfoChange(napi_env env, napi_callback_info info);
    napi_value OnGetSingleHandCompatibleModeConfig(napi_env env, napi_callback_info info);
    napi_value OnGetRootSceneUIContext(napi_env env, napi_callback_info info);
    napi_value OnSendTouchEvent(napi_env env, napi_callback_info info);
    napi_value OnAddWindowDragHotArea(napi_env env, napi_callback_info info);
    napi_value OnPreloadInLakeApp(napi_env env, napi_callback_info info);
    napi_value OnRequestFocusStatus(napi_env env, napi_callback_info info);
    void DoRequestFocusStatus(int32_t persistentId, bool isFocused, bool byForeground,
        FocusChangeReason reason, DisplayId displayId);
    napi_value OnRequestAllAppSessionUnfocus(napi_env env, napi_callback_info info);
    napi_value OnSetScreenLocked(napi_env env, napi_callback_info info);
    napi_value OnSetUserAuthPassed(napi_env env, napi_callback_info info);
    napi_value OnUpdateMaximizeMode(napi_env env, napi_callback_info info);
    napi_value OnReportData(napi_env env, napi_callback_info info);
    napi_value OnGetRssData(napi_env env, napi_callback_info info);
    napi_value OnRegisterRssData(napi_env env, napi_callback_info info);
    napi_value OnUnregisterRssData(napi_env env, napi_callback_info info);
    napi_value OnUpdateSessionDisplayId(napi_env env, napi_callback_info info);
    napi_value OnNotifyStackEmpty(napi_env env, napi_callback_info info);
    napi_value OnUpdateTitleInTargetPos(napi_env env, napi_callback_info info);
    napi_value OnSetSystemAnimatedScenes(napi_env env, napi_callback_info info);
    napi_value OnGetSessionSnapshotPixelMap(napi_env env, napi_callback_info info);
    napi_value OnGetSessionSnapshotPixelMapSync(napi_env env, napi_callback_info info);
    napi_value OnSetAppDragResizeType(napi_env env, napi_callback_info info);
    napi_value OnGetCustomDecorHeight(napi_env env, napi_callback_info info);
    napi_value OnSwitchFreeMultiWindow(napi_env env, napi_callback_info info);
    napi_value OnGetFreeMultiWindowConfig(napi_env env, napi_callback_info info);
    napi_value OnNotifyEnterRecentTask(napi_env env, napi_callback_info info);
    napi_value OnNotifySCBRecentStateChange(napi_env env, napi_callback_info info);
    napi_value OnUpdateDisplayHookInfo(napi_env env, napi_callback_info info);
    napi_value OnUpdateAppHookDisplayInfo(napi_env env, napi_callback_info info);
    napi_value OnUpdateAppHookWindowInfo(napi_env env, napi_callback_info info);
    napi_value OnNotifyHookOrientationChange(napi_env env, napi_callback_info info);
    napi_value OnInitScheduleUtils(napi_env env, napi_callback_info info);
    napi_value OnSetAppForceLandscapeConfig(napi_env env, napi_callback_info info);
    napi_value OnIsScbCoreEnabled(napi_env env, napi_callback_info info);
    napi_value OnRefreshPcZOrder(napi_env env, napi_callback_info info);
    napi_value OnGetWindowPid(napi_env env, napi_callback_info info);
    napi_value OnUpdatePcFoldScreenStatus(napi_env env, napi_callback_info info);
    napi_value OnUpdateSystemKeyboardStatus(napi_env env, napi_callback_info info);
    napi_value OnResetPcFoldScreenArrangeRule(napi_env env, napi_callback_info info);
    napi_value OnSetIsWindowRectAutoSave(napi_env env, napi_callback_info info);
    napi_value OnNotifyAboveLockScreen(napi_env env, napi_callback_info info);
    napi_value OnCloneWindow(napi_env env, napi_callback_info info);
    napi_value OnRegisterSingleHandContainerNode(napi_env env, napi_callback_info info);
    napi_value OnNotifyRotationChange(napi_env env, napi_callback_info info);
    napi_value OnSupportFollowParentWindowLayout(napi_env env, napi_callback_info info);
    napi_value OnSupportFollowRelativePositionToParent(napi_env env, napi_callback_info info);
    napi_value OnUpdateRsCmdBlockingCount(napi_env env, napi_callback_info info);
    napi_value OnSupportZLevel(napi_env env, napi_callback_info info);
    napi_value OnSetSupportFunctionType(napi_env env, napi_callback_info info);
    napi_value OnUpdateRecentMainSessionInfos(napi_env env, napi_callback_info info);
    napi_value OnApplyFeatureConfig(napi_env env, napi_callback_info info);
    napi_value OnNotifySupportRotationChange(napi_env env, napi_callback_info info);
    napi_value OnGetAllJsonProfile(napi_env env, napi_callback_info info);
    napi_value OnGetJsonProfile(napi_env env, napi_callback_info info);
    napi_value OnSetAppForceLandscapeConfigEnable(napi_env env, napi_callback_info info);
    
    /*
     * PC Window
     */
    napi_value OnGetWindowLimits(napi_env env, napi_callback_info info);
    void RegisterVirtualPixelRatioChangeCallback();
    void OnVirtualPixelChange(float density, DisplayId displayId);
    napi_value OnSetIsDockAutoHide(napi_env env, napi_callback_info info);
    napi_value OnHandleTrayAppChange(napi_env env, napi_callback_info info);

    /*
     * Multi Instance
     */
    napi_value OnGetMaxInstanceCount(napi_env env, napi_callback_info info);
    napi_value OnGetInstanceCount(napi_env env, napi_callback_info info);
    napi_value OnGetLastInstanceKey(napi_env env, napi_callback_info info);
    napi_value OnRefreshAppInfo(napi_env env, napi_callback_info info);

    /*
     * Window Immersive
     */
    napi_value OnNotifyAINavigationBarShowStatus(napi_env env, napi_callback_info info);
    napi_value OnNotifyNextAvoidRectInfo(napi_env env, napi_callback_info info);
    napi_value OnGetIsLayoutFullScreen(napi_env env, napi_callback_info info);
    napi_value OnSetMaximizeFullScreen(napi_env env, napi_callback_info info);
    napi_value OnSetStatusBarDefaultVisibilityPerDisplay(napi_env env, napi_callback_info info);
    napi_value OnNotifyStatusBarShowStatus(napi_env env, napi_callback_info info);
    napi_value OnNotifyLSStateChange(napi_env env, napi_callback_info info);
    napi_value OnNotifyStatusBarConstantlyShowStatus(napi_env env, napi_callback_info info);
    napi_value OnSetStatusBarAvoidHeight(napi_env env, napi_callback_info info);

    /*
     * Multi User
     */
    napi_value OnInitUserInfo(napi_env env, napi_callback_info info);
    napi_value OnHandleUserSwitch(napi_env env, napi_callback_info info);

    void OnRootSceneBackEvent();
    void OnStatusBarEnabledUpdate(bool enable, const std::string& bundleName);
    void OnGestureNavigationEnabledUpdate(bool enable, const std::string& bundleName, GestureBackType type);
    void OnCreateSystemSession(const sptr<SceneSession>& sceneSession);
    void OnCreateKeyboardSession(const sptr<SceneSession>& keyboardSession, const sptr<SceneSession>& panelSession);
    void OnOutsideDownEvent(int32_t x, int32_t y);
    void OnStartUIAbilityError(const uint32_t errorCode);
    void OnShiftFocus(int32_t persistentId, DisplayId displayGroupId);
    void ProcessCreateSystemSessionRegister();
    void ProcessCreateKeyboardSessionRegister();
    void ProcessStatusBarEnabledChangeListener();
    void ProcessGestureNavigationEnabledChangeListener();
    void ProcessStartUIAbilityErrorRegister();
    void ProcessOutsideDownEvent();
    void ProcessShiftFocus();
    void ProcessRegisterCallback(ListenerFunctionType listenerFunctionType);
    bool IsCallbackRegistered(napi_env env, const std::string& type, napi_value jsListenerObject);
    void RegisterDumpRootSceneElementInfoListener();
    void RegisterVirtualPixelRatioChangeListener();
    void SetIsClearSession(napi_env env, napi_value jsSceneSessionObj, sptr<SceneSession>& sceneSession);
    void OnCloseTargetFloatWindow(const std::string& bundleName);
    void ProcessCloseTargetFloatWindow();
    std::shared_ptr<NativeReference> GetJSCallback(const std::string& functionName);
    void ProcessAbilityManagerCollaboratorRegistered();
    void OnAbilityManagerCollaboratorRegistered();
    void RegisterRootSceneCallbacksOnSSManager();
    void RegisterSSManagerCallbacksOnRootScene();
    void OnNotifyAppUseControlList(
        ControlAppType type, int32_t userId, const std::vector<AppUseControlInfo>& controlList);
    void RegisterNotifyAppUseControlListCallback();
    void RegisterSetForegroundWindowNumCallback();
    void OnSetForegroundWindowNum(uint32_t windowNum);
    void RegisterMinimizeByWindowIdCallback();
    void OnMinimizeByWindowId(const std::vector<int32_t>& windowIds);
    void OnUpdateKioskAppListCallback(const std::vector<std::string>& kioskAppList);
    void RegisterUpdateKioskAppListCallback();
    void OnKioskModeChangeCallback(bool isKioskMode, int32_t persistentId);
    void RegisterKioskModeChangeCallback();

    /*
     * Window Recover
     */
    napi_value OnNotifySessionRecoverStatus(napi_env env, napi_callback_info info);
    napi_value OnGetBatchAbilityInfos(napi_env env, napi_callback_info info);
    void OnRecoverSceneSession(const sptr<SceneSession>& sceneSession, const SessionInfo& sessionInfo);
    void ProcessRecoverSceneSessionRegister();

    /*
     * PiP Window
     */
    void OnStartPiPFailed(DisplayId displayId);
    void ProcessStartPiPFailedRegister();
    napi_value OnSetPiPSettingSwitchStatus(napi_env env, napi_callback_info info);
    napi_value OnSetIsPipEnabled(napi_env env, napi_callback_info info);
    napi_value OnGetPipDeviceCollaborationPolicy(napi_env env, napi_callback_info info);

    /*
     * Window Animation
     */
    void RegisterUIEffectSetParamsCallback();
    void OnUIEffectSetParams(int32_t id, sptr<UIEffectParams> param);
    void RegisterUIEffectAnimateToCallback();
    void OnUIEffectAnimateTo(int32_t id, sptr<UIEffectParams> param, sptr<WindowAnimationOption> option,
        sptr<WindowAnimationOption> interruptOption);
    napi_value OnSetUIEffectControllerAliveInUI(napi_env env, napi_callback_info info);

    /*
     * Window Input Event
     */
    void RegisterWatchGestureConsumeResultCallback();
    void OnWatchGestureConsumeResult(int32_t keyCode, bool isConsumed);
    void RegisterWatchFocusActiveChangeCallback();
    void OnWatchFocusActiveChange(bool isActive);

    /*
     * Window Lifecycle
     */
    static napi_value RequestSceneSession(napi_env env, napi_callback_info info);
    static napi_value UpdateSceneSessionWant(napi_env env, napi_callback_info info);
    static napi_value RequestSceneSessionActivation(napi_env env, napi_callback_info info);
    static napi_value RequestSceneSessionBackground(napi_env env, napi_callback_info info);
    static napi_value RequestSceneSessionDestruction(napi_env env, napi_callback_info info);
    static napi_value NotifyForegroundInteractiveStatus(napi_env env, napi_callback_info info);
    static napi_value IsSceneSessionValid(napi_env env, napi_callback_info info);
    static napi_value RequestSceneSessionByCall(napi_env env, napi_callback_info info);
    static napi_value StartAbilityBySpecified(napi_env env, napi_callback_info info);
    static napi_value StartUIAbilityBySCB(napi_env env, napi_callback_info info);
    napi_value OnGetApplicationInfo(napi_env env, napi_callback_info info);
    napi_value OnSupportCreateFloatWindow(napi_env env, napi_callback_info info);
    void RegisterSceneSessionDestructCallback();
    void OnSceneSessionDestruct(int32_t persistentId);
    void RegisterTransferSessionToTargetScreenCallback();
    void OnTransferSessionToTargetScreen(const TransferSessionInfo& info);
    static napi_value NotifySessionTransferToTargetScreenEvent(napi_env env, napi_callback_info info);
    napi_value OnNotifySessionTransferToTargetScreenEvent(napi_env env, napi_callback_info info);
    static napi_value UpdateRecentMainSessionInfos(napi_env env, napi_callback_info info);
    static napi_value UpdateAppBoundSystemTrayStatus(napi_env env, napi_callback_info info);
    napi_value OnUpdateAppBoundSystemTrayStatus(napi_env env, napi_callback_info info);
    void RegisterMinimizeAllCallback();
    void OnMinimizeAll(DisplayId displayId, int32_t excludeWindowId);

    /*
     * Window Hierarchy
     */
    void RegisterSetSpecificWindowZIndexCallback();
    void OnSetSpecificWindowZIndex(WindowType windowType, int32_t zIndex, SetSpecificZIndexReason reason);

    napi_env env_;
    std::shared_mutex jsCbMapMutex_;
    std::map<std::string, std::shared_ptr<NativeReference>> jsCbMap_;

    sptr<RootScene> rootScene_;
    std::shared_ptr<MainThreadScheduler> taskScheduler_;

    /*
     * Window Rotation
     */
    std::unordered_map<int32_t, RotationChangeResult> GetRotationChangeResult(
        const std::vector<sptr<SceneSession>>& activeSceneSessionMapCopy,
        const RotationChangeInfo& rotationChangeInfo, bool isRestrictNotify = false);
    void ProcessSupportRotationRegister();
    void OnSupportRotationRegistered();

    /*
     * Window Pattern
     */
    napi_value OnSupportSnapshotAllSessionStatus(napi_env env, napi_callback_info info);
    napi_value OnSupportCacheLockedSessionSnapshot(napi_env env, napi_callback_info info);
    napi_value OnSupportPreloadStartingWindow(napi_env env, napi_callback_info info);
    napi_value OnPreloadStartingWindow(napi_env env, napi_callback_info info);
};
} // namespace OHOS::Rosen

#endif // OHOS_WINDOW_SCENE_JS_SCENE_SESSION_MANAGER_H