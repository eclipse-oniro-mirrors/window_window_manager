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

namespace OHOS::Rosen {
enum class ListenerFunctionType : uint32_t {
    CREATE_SYSTEM_SESSION_CB,
    CREATE_KEYBOARD_SESSION_CB,
    RECOVER_SCENE_SESSION_CB,
    STATUS_BAR_ENABLED_CHANGE_CB,
    OUTSIDE_DOWN_EVENT_CB,
    SHIFT_FOCUS_CB,
    CALLING_WINDOW_ID_CHANGE_CB,
    START_UI_ABILITY_ERROR,
    GESTURE_NAVIGATION_ENABLED_CHANGE_CB,
    ABILITY_MANAGER_COLLABORATOR_REGISTERED_CB,
};

class JsSceneSessionManager final {
public:
    explicit JsSceneSessionManager(napi_env env);
    ~JsSceneSessionManager() = default;

    static napi_value Init(napi_env env, napi_value exportObj);
    static void Finalizer(napi_env env, void* data, void* hint);

    static napi_value GetRootSceneSession(napi_env env, napi_callback_info info);
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
    static napi_value ChangeUIAbilityVisibilityBySCB(napi_env env, napi_callback_info info);
    static napi_value RegisterCallback(napi_env env, napi_callback_info info);
    static napi_value GetWindowSceneConfig(napi_env env, napi_callback_info info);
    static napi_value UpdateRotateAnimationConfig(napi_env env, napi_callback_info info);
    static napi_value ProcessBackEvent(napi_env env, napi_callback_info info);
    static napi_value CheckSceneZOrder(napi_env env, napi_callback_info info);
    static napi_value UpdateFocus(napi_env env, napi_callback_info info);
    static napi_value InitUserInfo(napi_env env, napi_callback_info info);
    static napi_value GetSessionSnapshotFilePath(napi_env env, napi_callback_info info);
    static napi_value SetVmaCacheStatus(napi_env env, napi_callback_info info);
    static napi_value InitWithRenderServiceAdded(napi_env env, napi_callback_info info);
    static napi_value GetAllWindowVisibilityInfos(napi_env env, napi_callback_info info);
    static napi_value GetAllAbilityInfos(napi_env env, napi_callback_info info);
    static napi_value GetBatchAbilityInfos(napi_env env, napi_callback_info info);
    static napi_value PrepareTerminate(napi_env env, napi_callback_info info);
    static napi_value PerfRequestEx(napi_env env, napi_callback_info info);
    static napi_value UpdateWindowMode(napi_env env, napi_callback_info info);
    static napi_value GetRootSceneUIContext(napi_env env, napi_callback_info info);
    static napi_value SendTouchEvent(napi_env env, napi_callback_info info);
    static napi_value AddWindowDragHotArea(napi_env env, napi_callback_info info);
    static napi_value PreloadInLakeApp(napi_env env, napi_callback_info info);
    static napi_value RequestFocusStatus(napi_env env, napi_callback_info info);
    static napi_value RequestAllAppSessionUnfocus(napi_env env, napi_callback_info info);
    static napi_value SetScreenLocked(napi_env env, napi_callback_info info);
    static napi_value UpdateTitleInTargetPos(napi_env env, napi_callback_info info);
    static napi_value UpdateMaximizeMode(napi_env env, napi_callback_info info);
    static napi_value ReportData(napi_env env, napi_callback_info info);
    static napi_value GetRssData(napi_env env, napi_callback_info info);
    static napi_value RegisterRssData(napi_env env, napi_callback_info info);
    static napi_value UnregisterRssData(napi_env env, napi_callback_info info);
    static napi_value NotifyStatusBarShowStatus(napi_env env, napi_callback_info info);
    static napi_value NotifyAINavigationBarShowStatus(napi_env env, napi_callback_info info);
    static napi_value NotifySessionRecoverStatus(napi_env env, napi_callback_info info);
    static napi_value UpdateSessionDisplayId(napi_env env, napi_callback_info info);
    static napi_value NotifyStackEmpty(napi_env env, napi_callback_info info);
    static napi_value NotifySwitchingUser(napi_env env, napi_callback_info info);
    static napi_value SetSystemAnimatedScenes(napi_env env, napi_callback_info info);
    static napi_value GetSessionSnapshotPixelMap(napi_env env, napi_callback_info info);
    static napi_value GetCustomDecorHeight(napi_env env, napi_callback_info info);
    static napi_value NotifyEnterRecentTask(napi_env env, napi_callback_info info);
    static napi_value UpdateDisplayHookInfo(napi_env env, napi_callback_info info);
    static napi_value UpdateAppHookDisplayInfo(napi_env env, napi_callback_info info);
    static napi_value InitScheduleUtils(napi_env env, napi_callback_info info);
    static napi_value SetAppForceLandscapeConfig(napi_env env, napi_callback_info info);
    static napi_value SwitchFreeMultiWindow(napi_env env, napi_callback_info info);
    static napi_value GetFreeMultiWindowConfig(napi_env env, napi_callback_info info);
    static napi_value GetIsLayoutFullScreen(napi_env env, napi_callback_info info);
    static napi_value IsScbCoreEnabled(napi_env env, napi_callback_info info);

private:
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
    napi_value OnUpdateRotateAnimationConfig(napi_env env, napi_callback_info info);
    napi_value OnProcessBackEvent(napi_env env, napi_callback_info info);
    napi_value OnCheckSceneZOrder(napi_env env, napi_callback_info info);
    napi_value OnUpdateFocus(napi_env env, napi_callback_info info);
    napi_value OnInitUserInfo(napi_env env, napi_callback_info info);
    napi_value OnGetSessionSnapshotFilePath(napi_env env, napi_callback_info info);
    napi_value OnSetVmaCacheStatus(napi_env env, napi_callback_info info);
    napi_value OnInitWithRenderServiceAdded(napi_env env, napi_callback_info info);
    napi_value OnGetAllWindowVisibilityInfos(napi_env env, napi_callback_info info);
    napi_value OnGetAllAbilityInfos(napi_env env, napi_callback_info info);
    napi_value OnGetBatchAbilityInfos(napi_env env, napi_callback_info info);
    napi_value OnPrepareTerminate(napi_env env, napi_callback_info info);
    napi_value OnPerfRequestEx(napi_env env, napi_callback_info info);
    napi_value OnUpdateWindowMode(napi_env env, napi_callback_info info);
    napi_value OnGetRootSceneUIContext(napi_env env, napi_callback_info info);
    napi_value OnSendTouchEvent(napi_env env, napi_callback_info info);
    napi_value OnAddWindowDragHotArea(napi_env env, napi_callback_info info);
    napi_value OnPreloadInLakeApp(napi_env env, napi_callback_info info);
    napi_value OnRequestFocusStatus(napi_env env, napi_callback_info info);
    napi_value OnRequestAllAppSessionUnfocus(napi_env env, napi_callback_info info);
    napi_value OnSetScreenLocked(napi_env env, napi_callback_info info);
    napi_value OnUpdateMaximizeMode(napi_env env, napi_callback_info info);
    napi_value OnReportData(napi_env env, napi_callback_info info);
    napi_value OnGetRssData(napi_env env, napi_callback_info info);
    napi_value OnRegisterRssData(napi_env env, napi_callback_info info);
    napi_value OnUnregisterRssData(napi_env env, napi_callback_info info);
    napi_value OnNotifySessionRecoverStatus(napi_env env, napi_callback_info info);
    napi_value OnUpdateSessionDisplayId(napi_env env, napi_callback_info info);
    napi_value OnNotifyStackEmpty(napi_env env, napi_callback_info info);
    napi_value OnNotifySwitchingUser(napi_env env, napi_callback_info info);
    napi_value OnNotifyStatusBarShowStatus(napi_env env, napi_callback_info info);
    napi_value OnNotifyAINavigationBarShowStatus(napi_env env, napi_callback_info info);
    napi_value OnUpdateTitleInTargetPos(napi_env env, napi_callback_info info);
    napi_value OnSetSystemAnimatedScenes(napi_env env, napi_callback_info info);
    napi_value OnGetSessionSnapshotPixelMap(napi_env env, napi_callback_info info);
    napi_value OnGetCustomDecorHeight(napi_env env, napi_callback_info info);
    napi_value OnSwitchFreeMultiWindow(napi_env env, napi_callback_info info);
    napi_value OnGetFreeMultiWindowConfig(napi_env env, napi_callback_info info);
    napi_value OnGetIsLayoutFullScreen(napi_env env, napi_callback_info info);
    napi_value OnNotifyEnterRecentTask(napi_env env, napi_callback_info info);
    napi_value OnUpdateDisplayHookInfo(napi_env env, napi_callback_info info);
    napi_value OnUpdateAppHookDisplayInfo(napi_env env, napi_callback_info info);
    napi_value OnInitScheduleUtils(napi_env env, napi_callback_info info);
    napi_value OnSetAppForceLandscapeConfig(napi_env env, napi_callback_info info);
    napi_value OnIsScbCoreEnabled(napi_env env, napi_callback_info info);

    void OnRootSceneBackEvent();
    void OnStatusBarEnabledUpdate(bool enable, const std::string& bundleName);
    void OnGestureNavigationEnabledUpdate(bool enable, const std::string& bundleName);
    void OnCreateSystemSession(const sptr<SceneSession>& sceneSession);
    void OnCreateKeyboardSession(const sptr<SceneSession>& keyboardSession, const sptr<SceneSession>& panelSession);
    void OnRecoverSceneSession(const sptr<SceneSession>& sceneSession, const SessionInfo& sessionInfo);
    void OnOutsideDownEvent(int32_t x, int32_t y);
    void OnStartUIAbilityError(const uint32_t errorCode);
    void OnShiftFocus(int32_t persistentId);
    void OnCallingSessionIdChange(uint32_t callingSessionId);
    void ProcessCreateSystemSessionRegister();
    void ProcessCreateKeyboardSessionRegister();
    void ProcessRecoverSceneSessionRegister();
    void ProcessStatusBarEnabledChangeListener();
    void ProcessGestureNavigationEnabledChangeListener();
    void ProcessStartUIAbilityErrorRegister();
    void ProcessOutsideDownEvent();
    void ProcessShiftFocus();
    void ProcessCallingSessionIdChangeRegister();
    void ProcessRegisterCallback(ListenerFunctionType listenerFunctionType);
    bool IsCallbackRegistered(napi_env env, const std::string& type, napi_value jsListenerObject);
    void RegisterDumpRootSceneElementInfoListener();
    void RegisterVirtualPixelRatioChangeListener();
    void SetIsClearSession(napi_env env, napi_value jsSceneSessionObj, sptr<SceneSession>& sceneSession);
    std::shared_ptr<NativeReference> GetJSCallback(const std::string& functionName);
    void ProcessAbilityManagerCollaboratorRegistered();
    void OnAbilityManagerCollaboratorRegistered();

    napi_env env_;
    std::shared_mutex jsCbMapMutex_;
    std::map<std::string, std::shared_ptr<NativeReference>> jsCbMap_;

    sptr<RootScene> rootScene_;
    std::shared_ptr<MainThreadScheduler> taskScheduler_;
};
} // namespace OHOS::Rosen

#endif // OHOS_WINDOW_SCENE_JS_SCENE_SESSION_MANAGER_H