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

#include "js_scene_session_manager.h"

#include <context.h>
#include <js_runtime_utils.h>
#include "interfaces/include/ws_common.h"
#include "napi_common_want.h"
#include "native_value.h"
#include "pixel_map_napi.h"
#include "session/host/include/pc_fold_screen_manager.h"
#include "session/host/include/scene_persistence.h"
#include "session/host/include/scene_persistent_storage.h"
#include "session/host/include/session.h"
#include "session_manager/include/scene_session_manager.h"
#include <ui_content.h>
#include "want.h"
#include "window_manager_hilog.h"

#include "js_root_scene_session.h"
#include "js_scene_session.h"
#include "js_scene_utils.h"
#include "js_rss_session.h"
#include "js_window_scene_config.h"
#ifdef SOC_PERF_ENABLE
#include "socperf_client.h"
#endif
#ifdef RESOURCE_SCHEDULE_SERVICE_ENABLE
#include "res_sched_client.h"
#endif
#define RETURN_IF_CONVERT_FAIL(env, jsValue, value, paramName, logTag)                                    \
    do {                                                                                                  \
        if (!ConvertFromJsValue(env, jsValue, value)) {                                                   \
            TLOGE(logTag, "Failed to convert parameter to %{public}s", paramName);                        \
            napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM), \
                "Input parameter is missing or invalid"));                                                \
            return NapiGetUndefined(env);                                                                 \
        }                                                                                                 \
    } while (0)

namespace OHOS::Rosen {
using namespace AbilityRuntime;
namespace {
constexpr HiviewDFX::HiLogLabel LABEL = { LOG_CORE, HILOG_DOMAIN_WINDOW, "JsSceneSessionManager" };
constexpr int MIN_ARG_COUNT = 3;
constexpr int DEFAULT_ARG_COUNT = 4;
constexpr int ARG_INDEX_ZERO = 0;
constexpr int ARG_INDEX_ONE = 1;
constexpr int ARG_INDEX_TWO = 2;
constexpr int ARG_INDEX_THREE = 3;
constexpr int ARG_INDEX_FOUR = 4;
constexpr int32_t RESTYPE_RECLAIM = 100001;
const std::string RES_PARAM_RECLAIM_TAG = "reclaimTag";
const std::string CREATE_SYSTEM_SESSION_CB = "createSpecificSession";
const std::string SET_SPECIFIC_SESSION_ZINDEX_CB = "setSpecificWindowZIndex";
const std::string CREATE_KEYBOARD_SESSION_CB = "createKeyboardSession";
const std::string RECOVER_SCENE_SESSION_CB = "recoverSceneSession";
const std::string STATUS_BAR_ENABLED_CHANGE_CB = "statusBarEnabledChange";
const std::string GESTURE_NAVIGATION_ENABLED_CHANGE_CB = "gestureNavigationEnabledChange";
const std::string OUTSIDE_DOWN_EVENT_CB = "outsideDownEvent";
const std::string START_UI_ABILITY_ERROR = "startUIAbilityError";
const std::string ARG_DUMP_HELP = "-h";
const std::string SHIFT_FOCUS_CB = "shiftFocus";
const std::string CLOSE_TARGET_FLOAT_WINDOW_CB = "closeTargetFloatWindow";
const std::string ABILITY_MANAGER_COLLABORATOR_REGISTERED_CB = "abilityManagerCollaboratorRegistered";
const std::string START_PIP_FAILED_CB = "startPiPFailed";
const std::string NOTIFY_APP_USE_CONTROL_LIST_CB = "updateAppUseControl";
const std::string WATCH_GESTURE_CONSUME_RESULT_CB = "watchGestureConsumeResult";
const std::string WATCH_FOCUS_ACTIVE_CHANGE_CB = "watchFocusActiveChange";
const std::string SET_FOREGROUND_WINDOW_NUM_CB = "setForegroundWindowNum";
const std::string MINIMIZE_BY_WINDOW_ID_CB = "minimizeByWindowId";
const std::string SCENE_SESSION_DESTRUCT_CB = "sceneSessionDestruct";
const std::string SCENE_SESSION_TRANSFER_TO_TARGET_SCREEN_CB = "sceneSessionTransferToTargetScreen";
const std::string UPDATE_KIOSK_APP_LIST_CB = "updateKioskAppList";
const std::string KIOSK_MODE_CHANGE_CB = "kioskModeChange";
const std::string NOTIFY_SUPPORT_ROTATION_REGISTERED_CB = "notifySupportRotationRegistered";
const std::string UI_EFFECT_SET_PARAMS_CB = "uiEffectSetParams";
const std::string UI_EFFECT_ANIMATE_TO_CB = "uiEffectAnimateTo";
const std::string VIRTUAL_DENSITY_CHANGE_CB = "virtualDensityChange";
const std::string MINIMIZE_ALL_CB = "minimizeAll";

const std::map<std::string, ListenerFunctionType> ListenerFunctionTypeMap {
    {CREATE_SYSTEM_SESSION_CB,     ListenerFunctionType::CREATE_SYSTEM_SESSION_CB},
    {CREATE_KEYBOARD_SESSION_CB,   ListenerFunctionType::CREATE_KEYBOARD_SESSION_CB},
    {RECOVER_SCENE_SESSION_CB,     ListenerFunctionType::RECOVER_SCENE_SESSION_CB},
    {STATUS_BAR_ENABLED_CHANGE_CB, ListenerFunctionType::STATUS_BAR_ENABLED_CHANGE_CB},
    {OUTSIDE_DOWN_EVENT_CB,        ListenerFunctionType::OUTSIDE_DOWN_EVENT_CB},
    {SHIFT_FOCUS_CB,               ListenerFunctionType::SHIFT_FOCUS_CB},
    {START_UI_ABILITY_ERROR,       ListenerFunctionType::START_UI_ABILITY_ERROR},
    {GESTURE_NAVIGATION_ENABLED_CHANGE_CB,
        ListenerFunctionType::GESTURE_NAVIGATION_ENABLED_CHANGE_CB},
    {CLOSE_TARGET_FLOAT_WINDOW_CB, ListenerFunctionType::CLOSE_TARGET_FLOAT_WINDOW_CB},
    {ABILITY_MANAGER_COLLABORATOR_REGISTERED_CB, ListenerFunctionType::ABILITY_MANAGER_COLLABORATOR_REGISTERED_CB},
    {START_PIP_FAILED_CB,          ListenerFunctionType::START_PIP_FAILED_CB},
    {NOTIFY_APP_USE_CONTROL_LIST_CB, ListenerFunctionType::NOTIFY_APP_USE_CONTROL_LIST_CB},
    {WATCH_GESTURE_CONSUME_RESULT_CB,          ListenerFunctionType::WATCH_GESTURE_CONSUME_RESULT_CB},
    {WATCH_FOCUS_ACTIVE_CHANGE_CB,             ListenerFunctionType::WATCH_FOCUS_ACTIVE_CHANGE_CB},
    {SET_FOREGROUND_WINDOW_NUM_CB,             ListenerFunctionType::SET_FOREGROUND_WINDOW_NUM_CB},
    {MINIMIZE_BY_WINDOW_ID_CB,                 ListenerFunctionType::MINIMIZE_BY_WINDOW_ID_CB},
    {SCENE_SESSION_DESTRUCT_CB,    ListenerFunctionType::SCENE_SESSION_DESTRUCT_CB},
    {SCENE_SESSION_TRANSFER_TO_TARGET_SCREEN_CB,    ListenerFunctionType::SCENE_SESSION_TRANSFER_TO_TARGET_SCREEN_CB},
    {UPDATE_KIOSK_APP_LIST_CB,     ListenerFunctionType::UPDATE_KIOSK_APP_LIST_CB},
    {KIOSK_MODE_CHANGE_CB,         ListenerFunctionType::KIOSK_MODE_CHANGE_CB},
    {NOTIFY_SUPPORT_ROTATION_REGISTERED_CB, ListenerFunctionType::NOTIFY_SUPPORT_ROTATION_REGISTERED_CB},
    {UI_EFFECT_SET_PARAMS_CB,       ListenerFunctionType::UI_EFFECT_SET_PARAMS_CB},
    {UI_EFFECT_ANIMATE_TO_CB,      ListenerFunctionType::UI_EFFECT_ANIMATE_TO_CB},
    {VIRTUAL_DENSITY_CHANGE_CB,   ListenerFunctionType::VIRTUAL_DENSITY_CHANGE_CB},
    {SET_SPECIFIC_SESSION_ZINDEX_CB,     ListenerFunctionType::SET_SPECIFIC_SESSION_ZINDEX_CB},
    {MINIMIZE_ALL_CB,     ListenerFunctionType::MINIMIZE_ALL_CB},
};
} // namespace

napi_value JsSceneSessionManager::Init(napi_env env, napi_value exportObj)
{
    TLOGD(WmsLogTag::DEFAULT, "[NAPI]");
    if (env == nullptr || exportObj == nullptr) {
        WLOGFE("env or exportObj is null!");
        return nullptr;
    }

    std::unique_ptr<JsSceneSessionManager> jsSceneSessionManager = std::make_unique<JsSceneSessionManager>(env);
    napi_wrap(env, exportObj, jsSceneSessionManager.release(), JsSceneSessionManager::Finalizer, nullptr, nullptr);

    napi_set_named_property(env, exportObj, "SessionState", CreateJsSessionState(env));
    napi_set_named_property(env, exportObj, "SessionType", SessionTypeInit(env));
    napi_set_named_property(env, exportObj, "SubWindowModalType", SubWindowModalTypeInit(env));
    napi_set_named_property(env, exportObj, "SceneType", SceneTypeInit(env));
    napi_set_named_property(env, exportObj, "KeyboardGravity", KeyboardGravityInit(env));
    napi_set_named_property(env, exportObj, "KeyboardViewMode", KeyboardViewModeInit(env));
    napi_set_named_property(env, exportObj, "KeyboardFlowLightMode", KeyboardFlowLightModeInit(env));
    napi_set_named_property(env, exportObj, "KeyboardGradientMode", KeyboardGradientModeInit(env));
    napi_set_named_property(env, exportObj, "SessionSizeChangeReason", CreateJsSessionSizeChangeReason(env));
    napi_set_named_property(env, exportObj, "RSUIFirstSwitch", CreateJsRSUIFirstSwitch(env));
    napi_set_named_property(env, exportObj, "StartupVisibility", CreateJsSessionStartupVisibility(env));
    napi_set_named_property(env, exportObj, "WindowVisibility", CreateJsWindowVisibility(env));
    napi_set_named_property(env, exportObj, "ProcessMode", CreateJsSessionProcessMode(env));
    napi_set_named_property(env, exportObj, "PiPControlType", CreateJsSessionPiPControlType(env));
    napi_set_named_property(env, exportObj, "PiPControlStatus", CreateJsSessionPiPControlStatus(env));
    napi_set_named_property(env, exportObj, "Gravity", CreateJsSessionGravity(env));
    napi_set_named_property(env, exportObj, "DragResizeType", CreateJsSessionDragResizeType(env));
    napi_set_named_property(env, exportObj, "RotationChangeType", CreateRotationChangeType(env));
    napi_set_named_property(env, exportObj, "RectType", CreateRectType(env));
    napi_set_named_property(env, exportObj, "WindowAnchor", CreateWindowAnchorType(env));
    napi_set_named_property(env, exportObj, "PixelUnit", CreatePixelUnitType(env));
    napi_set_named_property(env, exportObj, "AnimationType", AnimationTypeInit(env));
    napi_set_named_property(env, exportObj, "WindowTransitionType", WindowTransitionTypeInit(env));
    napi_set_named_property(env, exportObj, "WindowAnimationCurve", WindowAnimationCurveInit(env));
    napi_set_named_property(env, exportObj, "SupportFunctionType", CreateSupportType(env));
    napi_set_named_property(env, exportObj, "WaterfallResidentState", CreateWaterfallResidentState(env));
    napi_set_named_property(env, exportObj, "CompatibleStyleMode", CreateCompatibleStyleMode(env));

    const char* moduleName = "JsSceneSessionManager";
    BindNativeFunction(env, exportObj, "setBehindWindowFilterEnabled",
        moduleName, JsSceneSessionManager::SetBehindWindowFilterEnabled);
    BindNativeFunction(env, exportObj, "getRootSceneSession", moduleName, JsSceneSessionManager::GetRootSceneSession);
    BindNativeFunction(env, exportObj, "requestSceneSession", moduleName, JsSceneSessionManager::RequestSceneSession);
    BindNativeFunction(env, exportObj, "updateSceneSessionWant",
        moduleName, JsSceneSessionManager::UpdateSceneSessionWant);
    BindNativeFunction(env, exportObj, "requestSceneSessionActivation", moduleName,
        JsSceneSessionManager::RequestSceneSessionActivation);
    BindNativeFunction(env, exportObj, "requestSceneSessionBackground", moduleName,
        JsSceneSessionManager::RequestSceneSessionBackground);
    BindNativeFunction(env, exportObj, "requestSceneSessionDestruction", moduleName,
        JsSceneSessionManager::RequestSceneSessionDestruction);
    BindNativeFunction(env, exportObj, "notifyForegroundInteractiveStatus", moduleName,
        JsSceneSessionManager::NotifyForegroundInteractiveStatus);
    BindNativeFunction(env, exportObj, "isSceneSessionValid", moduleName,
        JsSceneSessionManager::IsSceneSessionValid);
    BindNativeFunction(env, exportObj, "on", moduleName, JsSceneSessionManager::RegisterCallback);
    BindNativeFunction(env, exportObj, "getWindowSceneConfig", moduleName,
        JsSceneSessionManager::GetWindowSceneConfig);
    BindNativeFunction(env, exportObj, "getSystemConfig", moduleName,
        JsSceneSessionManager::GetSystemConfig);
    BindNativeFunction(env, exportObj, "updateRotateAnimationConfig", moduleName,
        JsSceneSessionManager::UpdateRotateAnimationConfig);
    BindNativeFunction(env, exportObj, "processBackEvent", moduleName, JsSceneSessionManager::ProcessBackEvent);
    BindNativeFunction(env, exportObj, "checkSceneZOrder", moduleName, JsSceneSessionManager::CheckSceneZOrder);
    BindNativeFunction(env, exportObj, "initUserInfo", moduleName, JsSceneSessionManager::InitUserInfo);
    BindNativeFunction(env, exportObj, "requestSceneSessionByCall", moduleName,
        JsSceneSessionManager::RequestSceneSessionByCall);
    BindNativeFunction(env, exportObj, "startAbilityBySpecified", moduleName,
        JsSceneSessionManager::StartAbilityBySpecified);
    BindNativeFunction(env, exportObj, "startUIAbilityBySCB", moduleName,
        JsSceneSessionManager::StartUIAbilityBySCB);
    BindNativeFunction(env, exportObj, "changeUIAbilityVisibilityBySCB", moduleName,
        JsSceneSessionManager::ChangeUIAbilityVisibilityBySCB);
    BindNativeFunction(env, exportObj, "setVmaCacheStatus", moduleName,
        JsSceneSessionManager::SetVmaCacheStatus);
    BindNativeFunction(env, exportObj, "InitWithRenderServiceAdded", moduleName,
        JsSceneSessionManager::InitWithRenderServiceAdded);
    BindNativeFunction(env, exportObj, "getAllAbilityInfo", moduleName, JsSceneSessionManager::GetAllAbilityInfos);
    BindNativeFunction(env, exportObj, "getBatchAbilityInfos", moduleName, JsSceneSessionManager::GetBatchAbilityInfos);
    BindNativeFunction(env, exportObj, "getAbilityInfo", moduleName, JsSceneSessionManager::GetAbilityInfo);
    BindNativeFunction(env, exportObj, "getAllWindowVisibilityInfos", moduleName,
        JsSceneSessionManager::GetAllWindowVisibilityInfos);
    BindNativeFunction(env, exportObj, "prepareTerminate", moduleName, JsSceneSessionManager::PrepareTerminate);
    BindNativeFunction(env, exportObj, "asyncPrepareTerminate", moduleName,
        JsSceneSessionManager::AsyncPrepareTerminate);
    BindNativeFunction(env, exportObj, "perfRequestEx", moduleName, JsSceneSessionManager::PerfRequestEx);
    BindNativeFunction(env, exportObj, "updateWindowMode", moduleName, JsSceneSessionManager::UpdateWindowMode);
    BindNativeFunction(env, exportObj, "notifySingleHandInfoChange", moduleName,
        JsSceneSessionManager::NotifySingleHandInfoChange);
    BindNativeFunction(env, exportObj, "getSingleHandCompatibleModeConfig", moduleName,
        JsSceneSessionManager::GetSingleHandCompatibleModeConfig);
    BindNativeFunction(env, exportObj, "getRootSceneUIContext", moduleName,
        JsSceneSessionManager::GetRootSceneUIContext);
    BindNativeFunction(env, exportObj, "sendTouchEvent", moduleName, JsSceneSessionManager::SendTouchEvent);
    BindNativeFunction(env, exportObj, "addWindowDragHotArea", moduleName, JsSceneSessionManager::AddWindowDragHotArea);
    BindNativeFunction(env, exportObj, "preloadInLakeApp", moduleName, JsSceneSessionManager::PreloadInLakeApp);
    BindNativeFunction(env, exportObj, "requestFocusStatus", moduleName, JsSceneSessionManager::RequestFocusStatus);
    BindNativeFunction(env, exportObj, "requestAllAppSessionUnfocus", moduleName,
        JsSceneSessionManager::RequestAllAppSessionUnfocus);
    BindNativeFunction(env, exportObj, "setScreenLocked", moduleName, JsSceneSessionManager::SetScreenLocked);
    BindNativeFunction(env, exportObj, "setUserAuthPassed", moduleName, JsSceneSessionManager::SetUserAuthPassed);
    BindNativeFunction(env, exportObj, "updateMaximizeMode", moduleName, JsSceneSessionManager::UpdateMaximizeMode);
    BindNativeFunction(env, exportObj, "reportData", moduleName, JsSceneSessionManager::ReportData);
    BindNativeFunction(env, exportObj, "getRssData", moduleName, JsSceneSessionManager::GetRssData);
    BindNativeFunction(env, exportObj, "registerRssData", moduleName, JsSceneSessionManager::RegisterRssData);
    BindNativeFunction(env, exportObj, "unregisterRssData", moduleName, JsSceneSessionManager::UnregisterRssData);
    BindNativeFunction(env, exportObj, "updateSessionDisplayId", moduleName,
        JsSceneSessionManager::UpdateSessionDisplayId);
    BindNativeFunction(env, exportObj, "notifyStackEmpty", moduleName, JsSceneSessionManager::NotifyStackEmpty);
    BindNativeFunction(env, exportObj, "handleUserSwitch", moduleName, JsSceneSessionManager::HandleUserSwitch);
    BindNativeFunction(env, exportObj, "notifySessionRecoverStatus", moduleName,
        JsSceneSessionManager::NotifySessionRecoverStatus);
    BindNativeFunction(env, exportObj, "setStatusBarDefaultVisibilityPerDisplay", moduleName,
        JsSceneSessionManager::SetStatusBarDefaultVisibilityPerDisplay);
    BindNativeFunction(env, exportObj, "notifyStatusBarShowStatus", moduleName,
        JsSceneSessionManager::NotifyStatusBarShowStatus);
    BindNativeFunction(env, exportObj, "notifyLSStateChange", moduleName,
        JsSceneSessionManager::NotifyLSStateChange);
    BindNativeFunction(env, exportObj, "notifyStatusBarConstantlyShowStatus", moduleName,
        JsSceneSessionManager::NotifyStatusBarConstantlyShowStatus);
    BindNativeFunction(env, exportObj, "notifyAINavigationBarShowStatus", moduleName,
        JsSceneSessionManager::NotifyAINavigationBarShowStatus);
    BindNativeFunction(env, exportObj, "notifyNextAvoidRectInfo", moduleName,
        JsSceneSessionManager::NotifyNextAvoidRectInfo);
    BindNativeFunction(env, exportObj, "updateTitleInTargetPos", moduleName, JsSceneSessionManager::UpdateTitleInTargetPos);
    BindNativeFunction(env, exportObj, "setSystemAnimatedScenes", moduleName,
        JsSceneSessionManager::SetSystemAnimatedScenes);
    BindNativeFunction(env, exportObj, "getSessionSnapshotPixelMap", moduleName,
        JsSceneSessionManager::GetSessionSnapshotPixelMap);
    BindNativeFunction(env, exportObj, "setAppDragResizeType", moduleName,
        JsSceneSessionManager::SetAppDragResizeType);
    BindNativeFunction(env, exportObj, "getSessionSnapshotPixelMapSync", moduleName,
        JsSceneSessionManager::GetSessionSnapshotPixelMapSync);
    BindNativeFunction(env, exportObj, "getCustomDecorHeight", moduleName, JsSceneSessionManager::GetCustomDecorHeight);
    BindNativeFunction(env, exportObj, "switchFreeMultiWindow", moduleName,
        JsSceneSessionManager::SwitchFreeMultiWindow);
    BindNativeFunction(env, exportObj, "getFreeMultiWindowConfig", moduleName,
        JsSceneSessionManager::GetFreeMultiWindowConfig);
    BindNativeFunction(env, exportObj, "getIsLayoutFullScreen", moduleName,
        JsSceneSessionManager::GetIsLayoutFullScreen);
    BindNativeFunction(env, exportObj, "setMaximizeFullScreen", moduleName,
        JsSceneSessionManager::SetMaximizeFullScreen);
    BindNativeFunction(env, exportObj, "getWindowLimits", moduleName,
        JsSceneSessionManager::GetWindowLimits);
    BindNativeFunction(env, exportObj, "setIsDockAutoHide", moduleName,
        JsSceneSessionManager::SetIsDockAutoHide);
    BindNativeFunction(env, exportObj, "notifyEnterRecentTask", moduleName,
        JsSceneSessionManager::NotifyEnterRecentTask);
    BindNativeFunction(env, exportObj, "updateDisplayHookInfo", moduleName,
        JsSceneSessionManager::UpdateDisplayHookInfo);
    BindNativeFunction(env, exportObj, "initScheduleUtils", moduleName,
        JsSceneSessionManager::InitScheduleUtils);
    BindNativeFunction(env, exportObj, "setAppForceLandscapeConfig", moduleName,
        JsSceneSessionManager::SetAppForceLandscapeConfig);
    BindNativeFunction(env, exportObj, "setAppForceLandscapeConfigEnable", moduleName,
        JsSceneSessionManager::SetAppForceLandscapeConfigEnable);
    BindNativeFunction(env, exportObj, "isScbCoreEnabled", moduleName,
        JsSceneSessionManager::IsScbCoreEnabled);
    BindNativeFunction(env, exportObj, "updateAppHookDisplayInfo", moduleName,
        JsSceneSessionManager::UpdateAppHookDisplayInfo);
    BindNativeFunction(env, exportObj, "updateAppHookWindowInfo", moduleName,
        JsSceneSessionManager::UpdateAppHookWindowInfo);
    BindNativeFunction(env, exportObj, "notifyHookOrientationChange", moduleName,
        JsSceneSessionManager::NotifyHookOrientationChange);
    BindNativeFunction(env, exportObj, "refreshPcZOrder", moduleName,
        JsSceneSessionManager::RefreshPcZOrder);
    BindNativeFunction(env, exportObj, "getMaxInstanceCount", moduleName,
        JsSceneSessionManager::GetMaxInstanceCount);
    BindNativeFunction(env, exportObj, "getInstanceCount", moduleName,
        JsSceneSessionManager::GetInstanceCount);
    BindNativeFunction(env, exportObj, "getLastInstanceKey", moduleName,
        JsSceneSessionManager::GetLastInstanceKey);
    BindNativeFunction(env, exportObj, "refreshAppInfo", moduleName,
        JsSceneSessionManager::RefreshAppInfo);
    BindNativeFunction(env, exportObj, "getWindowPid", moduleName,
        JsSceneSessionManager::GetWindowPid);
    BindNativeFunction(env, exportObj, "updatePcFoldScreenStatus", moduleName,
        JsSceneSessionManager::UpdatePcFoldScreenStatus);
    BindNativeFunction(env, exportObj, "updateSystemKeyboardStatus", moduleName,
        JsSceneSessionManager::UpdateSystemKeyboardStatus);
    BindNativeFunction(env, exportObj, "resetPcFoldScreenArrangeRule", moduleName,
        JsSceneSessionManager::ResetPcFoldScreenArrangeRule);
    BindNativeFunction(env, exportObj, "setIsWindowRectAutoSave", moduleName,
        JsSceneSessionManager::SetIsWindowRectAutoSave);
    BindNativeFunction(env, exportObj, "notifyAboveLockScreen", moduleName,
        JsSceneSessionManager::NotifyAboveLockScreen);
    BindNativeFunction(env, exportObj, "setStatusBarAvoidHeight", moduleName,
        JsSceneSessionManager::SetStatusBarAvoidHeight);
    BindNativeFunction(env, exportObj, "cloneWindow", moduleName,
        JsSceneSessionManager::CloneWindow);
    BindNativeFunction(env, exportObj, "registerSingleHandContainerNode", moduleName,
        JsSceneSessionManager::RegisterSingleHandContainerNode);
    BindNativeFunction(env, exportObj, "notifyRotationChange", moduleName,
        JsSceneSessionManager::NotifyRotationChange);
    BindNativeFunction(env, exportObj, "supportFollowParentWindowLayout", moduleName,
        JsSceneSessionManager::SupportFollowParentWindowLayout);
    BindNativeFunction(env, exportObj, "supportFollowRelativePositionToParent", moduleName,
        JsSceneSessionManager::SupportFollowRelativePositionToParent);
    BindNativeFunction(env, exportObj, "updateRsCmdBlockingCount", moduleName,
        JsSceneSessionManager::UpdateRsCmdBlockingCount);
    BindNativeFunction(env, exportObj, "supportZLevel", moduleName,
        JsSceneSessionManager::SupportZLevel);
    BindNativeFunction(env, exportObj, "setSupportFunctionType", moduleName,
        JsSceneSessionManager::SetSupportFunctionType);
    BindNativeFunction(env, exportObj, "supportCreateFloatWindow", moduleName,
        JsSceneSessionManager::SupportCreateFloatWindow);
    BindNativeFunction(env, exportObj, "updateRecentMainSessionList", moduleName,
        JsSceneSessionManager::UpdateRecentMainSessionInfos);
    BindNativeFunction(env, exportObj, "supportSnapshotAllSessionStatus", moduleName,
        JsSceneSessionManager::SupportSnapshotAllSessionStatus);
    BindNativeFunction(env, exportObj, "supportCacheLockedSessionSnapshot", moduleName,
        JsSceneSessionManager::SupportCacheLockedSessionSnapshot);
    BindNativeFunction(env, exportObj, "supportPreloadStartingWindow", moduleName,
        JsSceneSessionManager::SupportPreloadStartingWindow);
    BindNativeFunction(env, exportObj, "preloadStartingWindow", moduleName,
        JsSceneSessionManager::PreloadStartingWindow);
    BindNativeFunction(env, exportObj, "setUIEffectControllerAliveInUI", moduleName,
        JsSceneSessionManager::SetUIEffectControllerAliveInUI);
    BindNativeFunction(env, exportObj, "setPiPSettingSwitchStatus", moduleName,
        JsSceneSessionManager::SetPiPSettingSwitchStatus);
    BindNativeFunction(env, exportObj, "setIsPipEnabled", moduleName,
        JsSceneSessionManager::SetIsPipEnabled);
    BindNativeFunction(env, exportObj, "applyFeatureConfig", moduleName,
        JsSceneSessionManager::ApplyFeatureConfig);
    BindNativeFunction(env, exportObj, "notifySessionTransferToTargetScreenEvent", moduleName,
        JsSceneSessionManager::NotifySessionTransferToTargetScreenEvent);
    BindNativeFunction(env, exportObj, "updateAppBoundSystemTrayStatus", moduleName,
        JsSceneSessionManager::UpdateAppBoundSystemTrayStatus);
    BindNativeFunction(env, exportObj, "getPipDeviceCollaborationPolicy", moduleName,
        JsSceneSessionManager::GetPipDeviceCollaborationPolicy);
    BindNativeFunction(env, exportObj, "notifySupportRotationChange", moduleName,
        JsSceneSessionManager::NotifySupportRotationChange);
    BindNativeFunction(env, exportObj, "setTrayAppListInfo", moduleName,
        JsSceneSessionManager::SetTrayAppListInfo);
    BindNativeFunction(env, exportObj, "getAllJsonProfile", moduleName,
        JsSceneSessionManager::GetAllJsonProfile);
    BindNativeFunction(env, exportObj, "getJsonProfile", moduleName,
        JsSceneSessionManager::GetJsonProfile);
    return NapiGetUndefined(env);
}

JsSceneSessionManager::JsSceneSessionManager(napi_env env) : env_(env)
{
    taskScheduler_ = std::make_shared<MainThreadScheduler>(env);
}

void JsSceneSessionManager::OnCreateSystemSession(const sptr<SceneSession>& sceneSession)
{
    if (sceneSession == nullptr) {
        TLOGI(WmsLogTag::WMS_LIFE, "sceneSession is nullptr");
        return;
    }
    TLOGD(WmsLogTag::WMS_LIFE, "Found callback, id: %{public}d", sceneSession->GetPersistentId());
    wptr<SceneSession> weakSession(sceneSession);
    auto task = [this, weakSession, jsCallBack = GetJSCallback(CREATE_SYSTEM_SESSION_CB), env = env_]() {
        if (jsCallBack == nullptr) {
            TLOGNE(WmsLogTag::WMS_LIFE, "jsCallBack is nullptr");
            return;
        }
        auto specificSession = weakSession.promote();
        if (specificSession == nullptr) {
            TLOGNE(WmsLogTag::WMS_LIFE, "Specific session is nullptr");
            return;
        }
        napi_value jsSceneSessionObj = JsSceneSession::Create(env, specificSession);
        if (jsSceneSessionObj == nullptr) {
            TLOGNE(WmsLogTag::WMS_LIFE, "jsSceneSessionObj is nullptr");
            return;
        }
        napi_value argv[] = {jsSceneSessionObj};
        napi_call_function(env, NapiGetUndefined(env), jsCallBack->GetNapiValue(), ArraySize(argv), argv, nullptr);
    };
    taskScheduler_->PostMainThreadTask(task, "OnCreateSystemSession");
}

void JsSceneSessionManager::OnSetSpecificWindowZIndex(WindowType windowType, int32_t zIndex,
    SetSpecificZIndexReason reason)
{
    TLOGI(WmsLogTag::WMS_FOCUS, "windowType: %{public}d, zIndex: %{public}d, reason: %{public}d",
        windowType, zIndex, reason);
    auto task = [this, windowType, zIndex, reason, jsCallBack = GetJSCallback(SET_SPECIFIC_SESSION_ZINDEX_CB),
        env = env_]() {
        if (jsCallBack == nullptr) {
            TLOGNE(WmsLogTag::WMS_FOCUS, "jsCallBack is nullptr");
            return;
        }
        uint32_t jsSessionType = static_cast<uint32_t>(GetApiType(windowType));
        napi_value argv[] = { CreateJsValue(env, jsSessionType),
                              CreateJsValue(env, zIndex),
                              CreateJsValue(env, reason) };
        napi_status ret = napi_call_function(env, NapiGetUndefined(env), jsCallBack->GetNapiValue(),
            ArraySize(argv), argv, nullptr);
        if (ret != napi_ok) {
            TLOGNE(WmsLogTag::WMS_FOCUS, "OnSetSpecificWindowZIndex:napi call exception ret: %{public}d", ret);
            return;
        }
    };
    taskScheduler_->PostMainThreadTask(task, "OnSetSpecificWindowZIndex, WindowType:" +
        std::to_string(static_cast<uint32_t>(windowType)));
}

void JsSceneSessionManager::OnCreateKeyboardSession(const sptr<SceneSession>& keyboardSession,
    const sptr<SceneSession>& panelSession)
{
    if (keyboardSession == nullptr || panelSession == nullptr) {
        TLOGE(WmsLogTag::WMS_KEYBOARD, "Keyboard or panel session is nullptr");
        return;
    }

    TLOGI(WmsLogTag::WMS_KEYBOARD, "keyboardId: %{public}d, panelId: %{public}d",
        keyboardSession->GetPersistentId(), panelSession->GetPersistentId());
    wptr<SceneSession> weakKeyboardSession(keyboardSession);
    wptr<SceneSession> weakPanelSession(panelSession);
    auto task = [this, weakKeyboardSession, weakPanelSession,
        jsCallBack = GetJSCallback(CREATE_KEYBOARD_SESSION_CB), env = env_]() {
        if (!jsCallBack) {
            TLOGNE(WmsLogTag::WMS_KEYBOARD, "jsCallBack is nullptr");
            return;
        }
        auto keyboardSession = weakKeyboardSession.promote();
        auto panelSession = weakPanelSession.promote();
        if (keyboardSession == nullptr || panelSession == nullptr) {
            TLOGNE(WmsLogTag::WMS_KEYBOARD, "Keyboard or panel session is nullptr");
            return;
        }
        napi_value keyboardSessionObj = JsSceneSession::Create(env, keyboardSession);
        if (keyboardSessionObj == nullptr) {
            TLOGNE(WmsLogTag::WMS_KEYBOARD, "keyboardSessionObj is nullptr");
            return;
        }
        napi_value panelSessionObj = JsSceneSession::Create(env, panelSession);
        if (panelSessionObj == nullptr) {
            TLOGNE(WmsLogTag::WMS_KEYBOARD, "panelSessionObj is nullptr");
            return;
        }
        napi_value argv[] = { keyboardSessionObj, panelSessionObj };
        napi_call_function(env, NapiGetUndefined(env), jsCallBack->GetNapiValue(), ArraySize(argv), argv, nullptr);
    };
    taskScheduler_->PostMainThreadTask(task, "OnCreateKeyboardSession");
}

void JsSceneSessionManager::OnRecoverSceneSession(const sptr<SceneSession>& sceneSession, const SessionInfo& info)
{
    if (sceneSession == nullptr) {
        TLOGE(WmsLogTag::WMS_RECOVER, "sceneSession is nullptr");
        return;
    }

    TLOGD(WmsLogTag::WMS_RECOVER, "[NAPI]");
    wptr<SceneSession> weakSession(sceneSession);
    std::shared_ptr<SessionInfo> sessionInfo = std::make_shared<SessionInfo>(info);
    auto task = [this, weakSession, sessionInfo, jsCallBack = GetJSCallback(RECOVER_SCENE_SESSION_CB), env = env_]() {
        if (jsCallBack == nullptr) {
            TLOGNE(WmsLogTag::WMS_RECOVER, "jsCallBack is nullptr");
            return;
        }
        auto sceneSession = weakSession.promote();
        if (sceneSession == nullptr) {
            TLOGNE(WmsLogTag::WMS_RECOVER, "sceneSession is nullptr");
            return;
        }
        napi_value jsSceneSessionObj = JsSceneSession::Create(env, sceneSession);
        if (jsSceneSessionObj == nullptr) {
            TLOGNE(WmsLogTag::WMS_RECOVER, "jsSceneSessionObj is nullptr");
            return;
        }
        if (sessionInfo == nullptr) {
            TLOGNE(WmsLogTag::WMS_RECOVER, "sessionInfo is nullptr");
            return;
        }
        napi_value jsSessionRecoverInfo =
            CreateJsSessionRecoverInfo(env, *sessionInfo, sceneSession->GetSessionProperty());
        if (jsSessionRecoverInfo == nullptr) {
            TLOGNE(WmsLogTag::WMS_RECOVER, "This target session info is nullptr");
            return;
        }
        napi_value argv[] = { jsSceneSessionObj, jsSessionRecoverInfo };
        napi_call_function(env, NapiGetUndefined(env), jsCallBack->GetNapiValue(), ArraySize(argv), argv, nullptr);
    };
    TLOGD(WmsLogTag::WMS_RECOVER, "post task");
    taskScheduler_->PostMainThreadTask(task, "OnRecoverSceneSession");
}

void JsSceneSessionManager::OnRootSceneBackEvent()
{
    auto task = [rootScene = RootScene::staticRootScene_]() {
        if (rootScene == nullptr ||  rootScene->GetUIContent() == nullptr) {
            TLOGNE(WmsLogTag::WMS_EVENT, "rootScene or UIContent is null");
            return;
        }
        TLOGND(WmsLogTag::WMS_EVENT, "rootScene ProcessBackPressed");
        rootScene->GetUIContent()->ProcessBackPressed();
    };
    taskScheduler_->PostMainThreadTask(task, "OnRootSceneBackEvent");
}

void JsSceneSessionManager::OnStatusBarEnabledUpdate(bool enable, const std::string& bundleName)
{
    TLOGI(WmsLogTag::WMS_MAIN, "enable:%{public}d bundleName:%{public}s", enable, bundleName.c_str());
    auto task = [enable, bundleName, jsCallBack = GetJSCallback(STATUS_BAR_ENABLED_CHANGE_CB), env = env_] {
        if (jsCallBack == nullptr) {
            TLOGNE(WmsLogTag::WMS_EVENT, "jsCallBack is nullptr");
            return;
        }
        napi_value argv[] = {CreateJsValue(env, enable), CreateJsValue(env, bundleName)};
        napi_call_function(env, NapiGetUndefined(env), jsCallBack->GetNapiValue(), ArraySize(argv), argv, nullptr);
    };
    taskScheduler_->PostMainThreadTask(task, "OnStatusBarEnabledUpdate, Enable" + std::to_string(enable));
}

void JsSceneSessionManager::OnGestureNavigationEnabledUpdate(bool enable, const std::string& bundleName,
    GestureBackType type)
{
    TLOGI(WmsLogTag::WMS_MAIN, "enable: %{public}d bundleName: %{public}s", enable, bundleName.c_str());
    auto task =
        [enable, bundleName, type, jsCallBack = GetJSCallback(GESTURE_NAVIGATION_ENABLED_CHANGE_CB), env = env_] {
        if (jsCallBack == nullptr) {
            TLOGNE(WmsLogTag::WMS_MAIN, "jsCallBack is nullptr");
            return;
        }
        napi_value argv[] = {CreateJsValue(env, enable), CreateJsValue(env, bundleName), CreateJsValue(env, type)};
        napi_call_function(env, NapiGetUndefined(env), jsCallBack->GetNapiValue(), ArraySize(argv), argv, nullptr);
    };
    taskScheduler_->PostMainThreadTask(task, "OnGestureNavigationEnabledUpdate" + std::to_string(enable));
}

void JsSceneSessionManager::OnStartUIAbilityError(const uint32_t errorCode)
{
    WLOGFI("[NAPI]");

    auto task = [this, errorCode, jsCallBack = GetJSCallback(START_UI_ABILITY_ERROR), env = env_]() {
        if (jsCallBack == nullptr) {
            TLOGNE(WmsLogTag::WMS_LIFE, "jsCallBack is nullptr");
            return;
        }
        napi_value argv[] = {CreateJsValue(env, errorCode)};
        napi_call_function(env, NapiGetUndefined(env), jsCallBack->GetNapiValue(), ArraySize(argv), argv, nullptr);
    };
    taskScheduler_->PostMainThreadTask(task, "OnStartUIAbilityError, errorCode: " + std::to_string(errorCode));
}

void JsSceneSessionManager::OnOutsideDownEvent(int32_t x, int32_t y)
{
    WLOGFD("[NAPI]");

    auto task = [this, x, y, jsCallBack = GetJSCallback(OUTSIDE_DOWN_EVENT_CB), env = env_]() {
        if (jsCallBack == nullptr) {
            TLOGNE(WmsLogTag::WMS_EVENT, "jsCallBack is nullptr");
            return;
        }
        napi_value objValue = nullptr;
        napi_create_object(env, &objValue);
        if (objValue == nullptr) {
            TLOGNE(WmsLogTag::WMS_EVENT, "Object is null!");
            return;
        }

        napi_set_named_property(env, objValue, "x", CreateJsValue(env_, x));
        napi_set_named_property(env, objValue, "y", CreateJsValue(env_, y));
        napi_value argv[] = {objValue};
        napi_call_function(env, NapiGetUndefined(env), jsCallBack->GetNapiValue(), ArraySize(argv), argv, nullptr);
    };
    taskScheduler_->PostMainThreadTask(task, "OnOutsideDownEvent");
}

void JsSceneSessionManager::OnShiftFocus(int32_t persistentId, DisplayId displayGroupId)
{
    TLOGD(WmsLogTag::WMS_FOCUS, "persistentId: %{public}d, displayGroupId: %{public}" PRIu64,
          persistentId, displayGroupId);

    auto task = [this, persistentId, jsCallBack = GetJSCallback(SHIFT_FOCUS_CB), env = env_, displayGroupId]() {
        if (jsCallBack == nullptr) {
            TLOGNE(WmsLogTag::WMS_FOCUS, "jsCallBack is nullptr");
            return;
        }
        napi_value argv[] = { CreateJsValue(env, persistentId),
                              CreateJsValue(env, static_cast<int64_t>(displayGroupId)) };
        napi_call_function(env, NapiGetUndefined(env), jsCallBack->GetNapiValue(), ArraySize(argv), argv, nullptr);
    };
    taskScheduler_->PostMainThreadTask(task, "OnShiftFocus, PID:" + std::to_string(persistentId));
}

void JsSceneSessionManager::OnAbilityManagerCollaboratorRegistered()
{
    TLOGD(WmsLogTag::WMS_LIFE, "[NAPI]");
    const char* const where = __func__;
    auto task = [jsCallBack = GetJSCallback(ABILITY_MANAGER_COLLABORATOR_REGISTERED_CB), env = env_, where] {
        if (jsCallBack == nullptr) {
            TLOGNE(WmsLogTag::WMS_LIFE, "%{public}s: jsCallBack is nullptr", where);
            return;
        }
        napi_call_function(env, NapiGetUndefined(env), jsCallBack->GetNapiValue(), 0, {}, nullptr);
    };
    taskScheduler_->PostMainThreadTask(task, where);
}

void JsSceneSessionManager::OnStartPiPFailed(DisplayId displayId)
{
    TLOGD(WmsLogTag::WMS_PIP, "[NAPI]");
    auto task = [jsCallBack = GetJSCallback(START_PIP_FAILED_CB), env = env_, displayId] {
        if (jsCallBack == nullptr) {
            TLOGNE(WmsLogTag::WMS_PIP, "jsCallBack is nullptr");
            return;
        }
        napi_value argv[] = { CreateJsValue(env, static_cast<int64_t>(displayId)) };
        napi_call_function(env, NapiGetUndefined(env), jsCallBack->GetNapiValue(), ArraySize(argv), argv, nullptr);
    };
    taskScheduler_->PostMainThreadTask(task, __func__);
}

void JsSceneSessionManager::OnSupportRotationRegistered()
{
    TLOGD(WmsLogTag::WMS_ROTATION, "[NAPI]");
    auto task = [jsCallBack = GetJSCallback(NOTIFY_SUPPORT_ROTATION_REGISTERED_CB), env = env_] {
        if (jsCallBack == nullptr) {
            TLOGNE(WmsLogTag::WMS_ROTATION, "jsCallBack is nullptr");
            return;
        }
        napi_call_function(env, NapiGetUndefined(env), jsCallBack->GetNapiValue(), 0, {}, nullptr);
    };
    taskScheduler_->PostMainThreadTask(task, __func__);
}

void JsSceneSessionManager::ProcessCreateSystemSessionRegister()
{
    NotifyCreateSystemSessionFunc func = [this](const sptr<SceneSession>& session) {
        TLOGNI(WmsLogTag::WMS_SYSTEM, "NotifyCreateSystemSessionFunc");
        this->OnCreateSystemSession(session);
    };
    SceneSessionManager::GetInstance().SetCreateSystemSessionListener(func);
}

/** @note @window.hierarchy */
void JsSceneSessionManager::RegisterSetSpecificWindowZIndexCallback()
{
    NotifySetSpecificWindowZIndexFunc func = [this](WindowType windowType, int32_t zIndex,
        SetSpecificZIndexReason reason) {
        TLOGNI(WmsLogTag::WMS_FOCUS, "set specifc zIndex callback");
        this->OnSetSpecificWindowZIndex(windowType, zIndex, reason);
    };
    SceneSessionManager::GetInstance().SetSpecificWindowZIndexListener(func);
}

void JsSceneSessionManager::ProcessCreateKeyboardSessionRegister()
{
    NotifyCreateKeyboardSessionFunc func = [this](const sptr<SceneSession>& keyboardSession,
        const sptr<SceneSession>& panelSession) {
        this->OnCreateKeyboardSession(keyboardSession, panelSession);
    };
    SceneSessionManager::GetInstance().SetCreateKeyboardSessionListener(func);
}

void JsSceneSessionManager::ProcessStartUIAbilityErrorRegister()
{
    const char* const where = __func__;
    ProcessStartUIAbilityErrorFunc func = [this, where](uint32_t startUIAbilityError) {
        TLOGND(WmsLogTag::WMS_LIFE, "%{public}s called, startUIAbilityError: %{public}d", where, startUIAbilityError);
        this->OnStartUIAbilityError(startUIAbilityError);
    };
    SceneSessionManager::GetInstance().SetStartUIAbilityErrorListener(func);
}

void JsSceneSessionManager::ProcessRecoverSceneSessionRegister()
{
    NotifyRecoverSceneSessionFunc func = [this](const sptr<SceneSession>& session, const SessionInfo& sessionInfo) {
        TLOGND(WmsLogTag::WMS_RECOVER, "RecoverSceneSession");
        this->OnRecoverSceneSession(session, sessionInfo);
    };
    SceneSessionManager::GetInstance().SetRecoverSceneSessionListener(func);
}

void JsSceneSessionManager::ProcessStatusBarEnabledChangeListener()
{
    ProcessStatusBarEnabledChangeFunc func = [this](bool enable, const std::string& bundleName) {
        TLOGND(WmsLogTag::WMS_EVENT, "StatusBarEnabledUpdate");
        this->OnStatusBarEnabledUpdate(enable, bundleName);
    };
    SceneSessionManager::GetInstance().SetStatusBarEnabledChangeListener(func);
}

void JsSceneSessionManager::ProcessGestureNavigationEnabledChangeListener()
{
    auto func = [this](bool enable, const std::string& bundleName, GestureBackType type) {
        this->OnGestureNavigationEnabledUpdate(enable, bundleName, type);
    };
    SceneSessionManager::GetInstance().SetGestureNavigationEnabledChangeListener(func);
}

void JsSceneSessionManager::ProcessOutsideDownEvent()
{
    ProcessOutsideDownEventFunc func = [this](int32_t x, int32_t y) {
        TLOGND(WmsLogTag::WMS_EVENT, "ProcessOutsideDownEvent called");
        this->OnOutsideDownEvent(x, y);
    };
    SceneSessionManager::GetInstance().SetOutsideDownEventListener(func);
}

void JsSceneSessionManager::ProcessShiftFocus()
{
    ProcessShiftFocusFunc func = [this](int32_t persistentId, DisplayId displayGroupId) {
        TLOGND(WmsLogTag::WMS_FOCUS, "ProcessShiftFocus called");
        this->OnShiftFocus(persistentId, displayGroupId);
    };
    NotifySCBAfterUpdateFocusFunc focusedCallback = [this](DisplayId displayId) {
        TLOGND(WmsLogTag::WMS_FOCUS, "scb uicontent focus, displayId: %{public}" PRIu64, displayId);
        const auto& uiContent = rootScene_->GetUIContentByDisplayId(displayId).first;
        if (uiContent == nullptr) {
            TLOGNE(WmsLogTag::WMS_FOCUS, "[WMSComm]uiContent is nullptr");
            return;
        }
        uiContent->Focus();
    };
    NotifySCBAfterUpdateFocusFunc unfocusedCallback = [this](DisplayId displayId) {
        TLOGND(WmsLogTag::WMS_FOCUS, "scb uicontent unfocus, displayId: %{public}" PRIu64, displayId);
        const auto& uiContent = rootScene_->GetUIContentByDisplayId(displayId).first;
        if (uiContent == nullptr) {
            TLOGNE(WmsLogTag::WMS_FOCUS, "[WMSComm]uiContent is nullptr");
            return;
        }
        uiContent->UnFocus();
    };
    NotifyDiffSCBAfterUpdateFocusFunc scbFocusChangeCallback = [this](
        DisplayId prevDisplayId, DisplayId currDisplayId) {
        TLOGND(WmsLogTag::WMS_FOCUS, "scb focus change, prevId: %{public}" PRIu64 " currId: %{public}" PRIu64,
            prevDisplayId, currDisplayId);
        const auto& prevUIContentPair = rootScene_->GetUIContentByDisplayId(prevDisplayId);
        const auto& prevUIContent = prevUIContentPair.first;
        const auto& currUIContent = rootScene_->GetUIContentByDisplayId(currDisplayId).first;
        if (prevUIContentPair.second && prevUIContent == currUIContent) {
            TLOGND(WmsLogTag::WMS_FOCUS, "not need to update focus");
            return;
        }
        if (prevUIContent != nullptr) {
            TLOGND(WmsLogTag::WMS_FOCUS, "scb uicontent unfocus, id: %{public}" PRIu64, prevDisplayId);
            prevUIContent->UnFocus();
        }
        if (currUIContent != nullptr) {
            TLOGND(WmsLogTag::WMS_FOCUS, "scb uicontent focus, id: %{public}" PRIu64, currDisplayId);
            currUIContent->Focus();
        }
    };
    SceneSessionManager::GetInstance().SetShiftFocusListener(func);
    SceneSessionManager::GetInstance().SetSCBFocusedListener(focusedCallback);
    SceneSessionManager::GetInstance().SetSCBUnfocusedListener(unfocusedCallback);
    SceneSessionManager::GetInstance().SetSCBFocusChangeListener(std::move(scbFocusChangeCallback));
}

void JsSceneSessionManager::ProcessAbilityManagerCollaboratorRegistered()
{
    auto func = [this] {
        this->OnAbilityManagerCollaboratorRegistered();
    };
    SceneSessionManager::GetInstance().SetAbilityManagerCollaboratorRegisteredFunc(func);
}

void JsSceneSessionManager::RegisterRootSceneCallbacksOnSSManager()
{
    RegisterDumpRootSceneElementInfoListener();
    RegisterVirtualPixelRatioChangeListener();
    SceneSessionManager::GetInstance().SetRootSceneProcessBackEventFunc([this] {
        TLOGND(WmsLogTag::WMS_EVENT, "rootScene BackEvent");
        this->OnRootSceneBackEvent();
    });
    SceneSessionManager::GetInstance().SetOnFlushUIParamsFunc([] {
        RootScene::staticRootScene_->OnFlushUIParams();
    });
    SceneSessionManager::GetInstance().SetIsRootSceneLastFrameLayoutFinishedFunc([] {
        return RootScene::staticRootScene_->IsLastFrameLayoutFinished();
    });
    SceneSessionManager::GetInstance().RegisterNotifyRootSceneAvoidAreaChangeFunc(
        [](const sptr<AvoidArea>& avoidArea, AvoidAreaType type, const sptr<OccupiedAreaChangeInfo>& info = nullptr) {
        RootScene::staticRootScene_->NotifyAvoidAreaChangeForRoot(avoidArea, type, info);
    });
    SceneSessionManager::GetInstance().RegisterGetRSNodeByStringIDFunc(
        [](const std::string& id) {
        return RootScene::staticRootScene_->GetRSNodeByStringID(id);
    });
    SceneSessionManager::GetInstance().RegisterSetTopWindowBoundaryByIDFunc(
        [](const std::string& id) {
        RootScene::staticRootScene_->SetTopWindowBoundaryByID(id);
    });
    SceneSessionManager::GetInstance().SetHasRootSceneRequestedVsyncFunc([] {
        return RootScene::staticRootScene_->HasRequestedVsync();
    });
    SceneSessionManager::GetInstance().SetRequestVsyncByRootSceneWhenModeChangeFunc(
        [](const std::shared_ptr<VsyncCallback>& vsyncCallback) {
        return RootScene::staticRootScene_->RequestVsync(vsyncCallback);
    });
}

void JsSceneSessionManager::RegisterSSManagerCallbacksOnRootScene()
{
    rootScene_->RegisterUpdateRootSceneRectCallback([](const Rect& rect) {
        if (auto rootSceneSession = SceneSessionManager::GetInstance().GetRootSceneSession()) {
            WSRect rootSceneRect = { 0, 0, rect.width_, rect.height_ };
            rootSceneSession->SetRootSessionRect(rootSceneRect);
        }
    });
    rootScene_->RegisterGetSessionAvoidAreaByTypeCallback([](AvoidAreaType type, bool ignoreVisibility) {
        return SceneSessionManager::GetInstance().GetRootSessionAvoidAreaByType(type, ignoreVisibility);
    });
    rootScene_->RegisterGetStatusBarHeightCallback([]() {
        return SceneSessionManager::GetInstance().GetRootSceneStatusBarHeight();
    });
    rootScene_->RegisterUpdateRootSceneAvoidAreaCallback([] {
        SceneSessionManager::GetInstance().UpdateRootSceneAvoidArea();
    });
    rootScene_->RegisterNotifyWatchFocusActiveChangeCallback([](bool isFocusActive) {
        SceneSessionManager::GetInstance().NotifyWatchFocusActiveChange(isFocusActive);
    });
    if (!Session::IsScbCoreEnabled()) {
        rootScene_->SetFrameLayoutFinishCallback([] {
            SceneSessionManager::GetInstance().NotifyUpdateRectAfterLayout();
            SceneSessionManager::GetInstance().FlushWindowInfoToMMI();
        });
    }
    RootScene::SetOnConfigurationUpdatedCallback([](const std::shared_ptr<AppExecFwk::Configuration>& configuration) {
        SceneSessionManager::GetInstance().OnConfigurationUpdated(configuration);
    });
}

void JsSceneSessionManager::ProcessStartPiPFailedRegister()
{
    SceneSessionManager::GetInstance().SetStartPiPFailedListener([this](DisplayId displayId) {
        TLOGNI(WmsLogTag::WMS_PIP, "NotifyStartPiPFailedFunc");
        this->OnStartPiPFailed(displayId);
    });
}

void JsSceneSessionManager::ProcessSupportRotationRegister()
{
    SceneSessionManager::GetInstance().SetSupportRotationRegisteredListener([this]() {
        TLOGNI(WmsLogTag::WMS_ROTATION, "NotifySupportRotationRegisteredFunc");
        this->OnSupportRotationRegistered();
    });
}

napi_value JsSceneSessionManager::SetBehindWindowFilterEnabled(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::WMS_PC, "[NAPI]");
    JsSceneSessionManager* me = CheckParamsAndGetThis<JsSceneSessionManager>(env, info);
    return (me != nullptr) ? me->OnSetBehindWindowFilterEnabled(env, info) : nullptr;
}

napi_value JsSceneSessionManager::RegisterCallback(napi_env env, napi_callback_info info)
{
    WLOGFD("[NAPI]");
    JsSceneSessionManager* me = CheckParamsAndGetThis<JsSceneSessionManager>(env, info);
    return (me != nullptr) ? me->OnRegisterCallback(env, info) : nullptr;
}

napi_value JsSceneSessionManager::ProcessBackEvent(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::DEFAULT, "[NAPI]");
    JsSceneSessionManager* me = CheckParamsAndGetThis<JsSceneSessionManager>(env, info);
    return (me != nullptr) ? me->OnProcessBackEvent(env, info) : nullptr;
}

/** @note @window.hierarchy */
napi_value JsSceneSessionManager::CheckSceneZOrder(napi_env env, napi_callback_info info)
{
    WLOGFD("[NAPI]");
    JsSceneSessionManager* me = CheckParamsAndGetThis<JsSceneSessionManager>(env, info);
    return (me != nullptr) ? me->OnCheckSceneZOrder(env, info) : nullptr;
}

napi_value JsSceneSessionManager::InitUserInfo(napi_env env, napi_callback_info info)
{
    TLOGI(WmsLogTag::WMS_MAIN, "[NAPI]");
    JsSceneSessionManager* me = CheckParamsAndGetThis<JsSceneSessionManager>(env, info);
    return (me != nullptr) ? me->OnInitUserInfo(env, info) : nullptr;
}

napi_value JsSceneSessionManager::InitScheduleUtils(napi_env env, napi_callback_info info)
{
    TLOGI(WmsLogTag::WMS_MAIN, "[NAPI]");
    JsSceneSessionManager* me = CheckParamsAndGetThis<JsSceneSessionManager>(env, info);
    return (me != nullptr) ? me->OnInitScheduleUtils(env, info) : nullptr;
}

napi_value JsSceneSessionManager::OnInitScheduleUtils(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::WMS_MAIN, "[NAPI]");
    SceneSessionManager::GetInstance().InitScheduleUtils();
    return NapiGetUndefined(env);
}

void JsSceneSessionManager::Finalizer(napi_env env, void* data, void* hint)
{
    TLOGD(WmsLogTag::DEFAULT, "[NAPI]");
    std::unique_ptr<JsSceneSessionManager>(static_cast<JsSceneSessionManager*>(data));
}

napi_value JsSceneSessionManager::GetRootSceneSession(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::DEFAULT, "[NAPI]");
    JsSceneSessionManager* me = CheckParamsAndGetThis<JsSceneSessionManager>(env, info);
    return (me != nullptr) ? me->OnGetRootSceneSession(env, info) : nullptr;
}

napi_value JsSceneSessionManager::RequestSceneSession(napi_env env, napi_callback_info info)
{
    WLOGFD("[NAPI]");
    JsSceneSessionManager* me = CheckParamsAndGetThis<JsSceneSessionManager>(env, info);
    if (me == nullptr) {
        TLOGW(WmsLogTag::WMS_LIFE, "me is null");
        return nullptr;
    }
    return me->OnRequestSceneSession(env, info);
}

napi_value JsSceneSessionManager::UpdateSceneSessionWant(napi_env env, napi_callback_info info)
{
    WLOGFD("[NAPI]");
    JsSceneSessionManager* me = CheckParamsAndGetThis<JsSceneSessionManager>(env, info);
    return (me != nullptr) ? me->OnUpdateSceneSessionWant(env, info) : nullptr;
}

napi_value JsSceneSessionManager::RequestSceneSessionActivation(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::WMS_LIFE, "[NAPI]");
    JsSceneSessionManager* me = CheckParamsAndGetThis<JsSceneSessionManager>(env, info);
    if (me == nullptr) {
        TLOGW(WmsLogTag::WMS_LIFE, "me is null");
        return nullptr;
    }
    return me->OnRequestSceneSessionActivation(env, info);
}

napi_value JsSceneSessionManager::RequestSceneSessionBackground(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::WMS_LIFE, "[NAPI]");
    JsSceneSessionManager* me = CheckParamsAndGetThis<JsSceneSessionManager>(env, info);
    if (me == nullptr) {
        TLOGW(WmsLogTag::WMS_LIFE, "me is null");
        return nullptr;
    }
    return me->OnRequestSceneSessionBackground(env, info);
}

napi_value JsSceneSessionManager::RequestSceneSessionDestruction(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::WMS_LIFE, "[NAPI]");
    JsSceneSessionManager* me = CheckParamsAndGetThis<JsSceneSessionManager>(env, info);
    if (me == nullptr) {
        TLOGW(WmsLogTag::WMS_LIFE, "me is null");
        return nullptr;
    }
    return me->OnRequestSceneSessionDestruction(env, info);
}

napi_value JsSceneSessionManager::NotifyForegroundInteractiveStatus(napi_env env, napi_callback_info info)
{
    if (Session::IsScbCoreEnabled()) {
        return nullptr;
    }
    WLOGFD("[NAPI]");
    JsSceneSessionManager* me = CheckParamsAndGetThis<JsSceneSessionManager>(env, info);
    return (me != nullptr) ? me->OnNotifyForegroundInteractiveStatus(env, info) : nullptr;
}

napi_value JsSceneSessionManager::IsSceneSessionValid(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::WMS_LIFE, "[NAPI]");
    JsSceneSessionManager* me = CheckParamsAndGetThis<JsSceneSessionManager>(env, info);
    return (me != nullptr) ? me->OnIsSceneSessionValid(env, info) : nullptr;
}

napi_value JsSceneSessionManager::RequestSceneSessionByCall(napi_env env, napi_callback_info info)
{
    WLOGFD("[NAPI]");
    JsSceneSessionManager* me = CheckParamsAndGetThis<JsSceneSessionManager>(env, info);
    return (me != nullptr) ? me->OnRequestSceneSessionByCall(env, info) : nullptr;
}

napi_value JsSceneSessionManager::StartAbilityBySpecified(napi_env env, napi_callback_info info)
{
    WLOGFD("[NAPI]");
    JsSceneSessionManager* me = CheckParamsAndGetThis<JsSceneSessionManager>(env, info);
    return (me != nullptr) ? me->OnStartAbilityBySpecified(env, info) : nullptr;
}

napi_value JsSceneSessionManager::StartUIAbilityBySCB(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::WMS_LIFE, "[NAPI]");
    JsSceneSessionManager* me = CheckParamsAndGetThis<JsSceneSessionManager>(env, info);
    return (me != nullptr) ? me->OnStartUIAbilityBySCB(env, info) : nullptr;
}

napi_value JsSceneSessionManager::ChangeUIAbilityVisibilityBySCB(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::WMS_LIFE, "[NAPI]");
    JsSceneSessionManager* me = CheckParamsAndGetThis<JsSceneSessionManager>(env, info);
    return (me != nullptr) ? me->OnChangeUIAbilityVisibilityBySCB(env, info) : nullptr;
}

napi_value JsSceneSessionManager::GetWindowSceneConfig(napi_env env, napi_callback_info info)
{
    WLOGFD("[NAPI]");
    JsSceneSessionManager* me = CheckParamsAndGetThis<JsSceneSessionManager>(env, info);
    return (me != nullptr) ? me->OnGetWindowSceneConfig(env, info) : nullptr;
}

napi_value JsSceneSessionManager::GetSystemConfig(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::DEFAULT, "in");
    JsSceneSessionManager* me = CheckParamsAndGetThis<JsSceneSessionManager>(env, info);
    return (me != nullptr) ? me->OnGetSystemConfig(env, info) : nullptr;
}

napi_value JsSceneSessionManager::UpdateRotateAnimationConfig(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::WMS_SCB, "[NAPI]");
    JsSceneSessionManager* me = CheckParamsAndGetThis<JsSceneSessionManager>(env, info);
    return (me != nullptr) ? me->OnUpdateRotateAnimationConfig(env, info) : nullptr;
}

napi_value JsSceneSessionManager::SetVmaCacheStatus(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::DEFAULT, "[NAPI]");
    JsSceneSessionManager* me = CheckParamsAndGetThis<JsSceneSessionManager>(env, info);
    return (me != nullptr) ? me->OnSetVmaCacheStatus(env, info) : nullptr;
}

napi_value JsSceneSessionManager::InitWithRenderServiceAdded(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::DEFAULT, "[NAPI]");
    JsSceneSessionManager* me = CheckParamsAndGetThis<JsSceneSessionManager>(env, info);
    return (me != nullptr) ? me->OnInitWithRenderServiceAdded(env, info) : nullptr;
}

napi_value JsSceneSessionManager::GetAllWindowVisibilityInfos(napi_env env, napi_callback_info info)
{
    TLOGI(WmsLogTag::WMS_SCB, "[NAPI]");
    JsSceneSessionManager* me = CheckParamsAndGetThis<JsSceneSessionManager>(env, info);
    return (me != nullptr) ? me->OnGetAllWindowVisibilityInfos(env, info) : nullptr;
}

napi_value JsSceneSessionManager::SetSystemAnimatedScenes(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::DEFAULT, "[NAPI]");
    JsSceneSessionManager* me = CheckParamsAndGetThis<JsSceneSessionManager>(env, info);
    return (me != nullptr) ? me->OnSetSystemAnimatedScenes(env, info) : nullptr;
}

napi_value JsSceneSessionManager::GetAllAbilityInfos(napi_env env, napi_callback_info info)
{
    WLOGFD("[NAPI]");
    JsSceneSessionManager* me = CheckParamsAndGetThis<JsSceneSessionManager>(env, info);
    return (me != nullptr) ? me->OnGetAllAbilityInfos(env, info) : nullptr;
}

napi_value JsSceneSessionManager::GetBatchAbilityInfos(napi_env env, napi_callback_info info)
{
    TLOGI(WmsLogTag::WMS_SCB, "[NAPI]");
    JsSceneSessionManager* me = CheckParamsAndGetThis<JsSceneSessionManager>(env, info);
    return (me != nullptr) ? me->OnGetBatchAbilityInfos(env, info) : nullptr;
}

napi_value JsSceneSessionManager::GetAbilityInfo(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::WMS_SCB, "[NAPI]");
    JsSceneSessionManager* me = CheckParamsAndGetThis<JsSceneSessionManager>(env, info);
    return (me != nullptr) ? me->OnGetAbilityInfo(env, info) : nullptr;
}

napi_value JsSceneSessionManager::PrepareTerminate(napi_env env, napi_callback_info info)
{
    WLOGFD("[NAPI]");
    JsSceneSessionManager* me = CheckParamsAndGetThis<JsSceneSessionManager>(env, info);
    return (me != nullptr) ? me->OnPrepareTerminate(env, info) : nullptr;
}

napi_value JsSceneSessionManager::AsyncPrepareTerminate(napi_env env, napi_callback_info info)
{
    JsSceneSessionManager* me = CheckParamsAndGetThis<JsSceneSessionManager>(env, info);
    return (me != nullptr) ? me->OnAsyncPrepareTerminate(env, info) : nullptr;
}

napi_value JsSceneSessionManager::PerfRequestEx(napi_env env, napi_callback_info info)
{
    WLOGFD("[NAPI]");
    JsSceneSessionManager* me = CheckParamsAndGetThis<JsSceneSessionManager>(env, info);
    return (me != nullptr) ? me->OnPerfRequestEx(env, info) : nullptr;
}

napi_value JsSceneSessionManager::UpdateWindowMode(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::WMS_LAYOUT, "[NAPI]");
    JsSceneSessionManager* me = CheckParamsAndGetThis<JsSceneSessionManager>(env, info);
    return (me != nullptr) ? me->OnUpdateWindowMode(env, info) : nullptr;
}

napi_value JsSceneSessionManager::NotifySingleHandInfoChange(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::WMS_LAYOUT, "[NAPI]");
    JsSceneSessionManager* me = CheckParamsAndGetThis<JsSceneSessionManager>(env, info);
    return (me != nullptr) ? me->OnNotifySingleHandInfoChange(env, info) : nullptr;
}

napi_value JsSceneSessionManager::GetSingleHandCompatibleModeConfig(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::WMS_LAYOUT, "[NAPI]");
    JsSceneSessionManager* me = CheckParamsAndGetThis<JsSceneSessionManager>(env, info);
    return (me != nullptr) ? me->OnGetSingleHandCompatibleModeConfig(env, info) : nullptr;
}

napi_value JsSceneSessionManager::AddWindowDragHotArea(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::WMS_LAYOUT, "[NAPI]");
    JsSceneSessionManager* me = CheckParamsAndGetThis<JsSceneSessionManager>(env, info);
    return (me != nullptr) ? me->OnAddWindowDragHotArea(env, info) : nullptr;
}

napi_value JsSceneSessionManager::GetRootSceneUIContext(napi_env env, napi_callback_info info)
{
    WLOGFD("[NAPI]");
    JsSceneSessionManager* me = CheckParamsAndGetThis<JsSceneSessionManager>(env, info);
    return (me != nullptr) ? me->OnGetRootSceneUIContext(env, info) : nullptr;
}

napi_value JsSceneSessionManager::SendTouchEvent(napi_env env, napi_callback_info info)
{
    TLOGI(WmsLogTag::WMS_EVENT, "in");
    JsSceneSessionManager* me = CheckParamsAndGetThis<JsSceneSessionManager>(env, info);
    return (me != nullptr) ? me->OnSendTouchEvent(env, info) : nullptr;
}

napi_value JsSceneSessionManager::PreloadInLakeApp(napi_env env, napi_callback_info info)
{
    WLOGFD("[NAPI]");
    JsSceneSessionManager* me = CheckParamsAndGetThis<JsSceneSessionManager>(env, info);
    return (me != nullptr) ? me->OnPreloadInLakeApp(env, info) : nullptr;
}

napi_value JsSceneSessionManager::RequestFocusStatus(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::WMS_FOCUS, "[NAPI]");
    JsSceneSessionManager* me = CheckParamsAndGetThis<JsSceneSessionManager>(env, info);
    return (me != nullptr) ? me->OnRequestFocusStatus(env, info) : nullptr;
}

napi_value JsSceneSessionManager::RequestAllAppSessionUnfocus(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::WMS_FOCUS, "[NAPI]");
    JsSceneSessionManager* me = CheckParamsAndGetThis<JsSceneSessionManager>(env, info);
    return (me != nullptr) ? me->OnRequestAllAppSessionUnfocus(env, info) : nullptr;
}

napi_value JsSceneSessionManager::SetScreenLocked(napi_env env, napi_callback_info info)
{
    WLOGFI("[NAPI]");
    JsSceneSessionManager* me = CheckParamsAndGetThis<JsSceneSessionManager>(env, info);
    return (me != nullptr) ? me->OnSetScreenLocked(env, info) : nullptr;
}

napi_value JsSceneSessionManager::SetUserAuthPassed(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::WMS_LIFE, "[NAPI]");
    JsSceneSessionManager* me = CheckParamsAndGetThis<JsSceneSessionManager>(env, info);
    return (me != nullptr) ? me->OnSetUserAuthPassed(env, info) : nullptr;
}

napi_value JsSceneSessionManager::UpdateMaximizeMode(napi_env env, napi_callback_info info)
{
    WLOGFI("[NAPI]");
    JsSceneSessionManager* me = CheckParamsAndGetThis<JsSceneSessionManager>(env, info);
    return (me != nullptr) ? me->OnUpdateMaximizeMode(env, info) : nullptr;
}

napi_value JsSceneSessionManager::ReportData(napi_env env, napi_callback_info info)
{
    WLOGFD("[NAPI]");
    JsSceneSessionManager* me = CheckParamsAndGetThis<JsSceneSessionManager>(env, info);
    return (me != nullptr) ? me->OnReportData(env, info) : nullptr;
}

napi_value JsSceneSessionManager::GetRssData(napi_env env, napi_callback_info info)
{
    WLOGFD("[NAPI]");
    JsSceneSessionManager* me = CheckParamsAndGetThis<JsSceneSessionManager>(env, info);
    return (me != nullptr) ? me->OnGetRssData(env, info) : nullptr;
}

napi_value JsSceneSessionManager::RegisterRssData(napi_env env, napi_callback_info info)
{
    WLOGFD("[NAPI]");
    JsSceneSessionManager* me = CheckParamsAndGetThis<JsSceneSessionManager>(env, info);
    return (me != nullptr) ? me->OnRegisterRssData(env, info) : nullptr;
}

napi_value JsSceneSessionManager::UnregisterRssData(napi_env env, napi_callback_info info)
{
    WLOGFD("[NAPI]");
    JsSceneSessionManager* me = CheckParamsAndGetThis<JsSceneSessionManager>(env, info);
    return (me != nullptr) ? me->OnUnregisterRssData(env, info) : nullptr;
}

napi_value JsSceneSessionManager::UpdateSessionDisplayId(napi_env env, napi_callback_info info)
{
    WLOGFI("[NAPI]");
    JsSceneSessionManager* me = CheckParamsAndGetThis<JsSceneSessionManager>(env, info);
    return (me != nullptr) ? me->OnUpdateSessionDisplayId(env, info) : nullptr;
}

napi_value JsSceneSessionManager::NotifyStackEmpty(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::WMS_LIFE, "[NAPI]");
    JsSceneSessionManager* me = CheckParamsAndGetThis<JsSceneSessionManager>(env, info);
    return (me != nullptr) ? me->OnNotifyStackEmpty(env, info) : nullptr;
}

napi_value JsSceneSessionManager::HandleUserSwitch(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::WMS_MULTI_USER, "[NAPI]");
    JsSceneSessionManager* me = CheckParamsAndGetThis<JsSceneSessionManager>(env, info);
    return (me != nullptr) ? me->OnHandleUserSwitch(env, info) : nullptr;
}

napi_value JsSceneSessionManager::SetStatusBarDefaultVisibilityPerDisplay(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::WMS_IMMS, "[NAPI]");
    JsSceneSessionManager* me = CheckParamsAndGetThis<JsSceneSessionManager>(env, info);
    return (me != nullptr) ? me->OnSetStatusBarDefaultVisibilityPerDisplay(env, info) : nullptr;
}

napi_value JsSceneSessionManager::NotifyStatusBarShowStatus(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::WMS_IMMS, "[NAPI]");
    JsSceneSessionManager* me = CheckParamsAndGetThis<JsSceneSessionManager>(env, info);
    return (me != nullptr) ? me->OnNotifyStatusBarShowStatus(env, info) : nullptr;
}

napi_value JsSceneSessionManager::NotifyLSStateChange(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::WMS_IMMS, "[NAPI]");
    JsSceneSessionManager* me = CheckParamsAndGetThis<JsSceneSessionManager>(env, info);
    return (me != nullptr) ? me->OnNotifyLSStateChange(env, info) : nullptr;
}

napi_value JsSceneSessionManager::NotifyStatusBarConstantlyShowStatus(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::WMS_IMMS, "[NAPI]");
    JsSceneSessionManager* me = CheckParamsAndGetThis<JsSceneSessionManager>(env, info);
    return (me != nullptr) ? me->OnNotifyStatusBarConstantlyShowStatus(env, info) : nullptr;
}

napi_value JsSceneSessionManager::NotifyAINavigationBarShowStatus(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::WMS_IMMS, "[NAPI]");
    JsSceneSessionManager* me = CheckParamsAndGetThis<JsSceneSessionManager>(env, info);
    return (me != nullptr) ? me->OnNotifyAINavigationBarShowStatus(env, info) : nullptr;
}

napi_value JsSceneSessionManager::NotifyNextAvoidRectInfo(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::WMS_IMMS, "[NAPI]");
    JsSceneSessionManager* me = CheckParamsAndGetThis<JsSceneSessionManager>(env, info);
    return (me != nullptr) ? me->OnNotifyNextAvoidRectInfo(env, info) : nullptr;
}

napi_value JsSceneSessionManager::NotifySessionRecoverStatus(napi_env env, napi_callback_info info)
{
    WLOGFI("[NAPI]");
    JsSceneSessionManager* me = CheckParamsAndGetThis<JsSceneSessionManager>(env, info);
    return (me != nullptr) ? me->OnNotifySessionRecoverStatus(env, info) : nullptr;
}

napi_value JsSceneSessionManager::UpdateTitleInTargetPos(napi_env env, napi_callback_info info)
{
    WLOGFI("[NAPI]");
    JsSceneSessionManager* me = CheckParamsAndGetThis<JsSceneSessionManager>(env, info);
    return (me != nullptr) ? me->OnUpdateTitleInTargetPos(env, info) : nullptr;
}

napi_value JsSceneSessionManager::GetSessionSnapshotPixelMap(napi_env env, napi_callback_info info)
{
    WLOGFI("[NAPI]");
    JsSceneSessionManager* me = CheckParamsAndGetThis<JsSceneSessionManager>(env, info);
    return (me != nullptr) ? me->OnGetSessionSnapshotPixelMap(env, info) : nullptr;
}

napi_value JsSceneSessionManager::GetSessionSnapshotPixelMapSync(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::WMS_MAIN, "[NAPI]");
    JsSceneSessionManager* me = CheckParamsAndGetThis<JsSceneSessionManager>(env, info);
    return (me != nullptr) ? me->OnGetSessionSnapshotPixelMapSync(env, info) : nullptr;
}

napi_value JsSceneSessionManager::SetAppDragResizeType(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::WMS_LAYOUT, "[NAPI]");
    JsSceneSessionManager* me = CheckParamsAndGetThis<JsSceneSessionManager>(env, info);
    return (me != nullptr) ? me->OnSetAppDragResizeType(env, info) : nullptr;
}

napi_value JsSceneSessionManager::SwitchFreeMultiWindow(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::WMS_LAYOUT_PC, "[NAPI]");
    JsSceneSessionManager* me = CheckParamsAndGetThis<JsSceneSessionManager>(env, info);
    return (me != nullptr) ? me->OnSwitchFreeMultiWindow(env, info) : nullptr;
}

napi_value JsSceneSessionManager::GetIsLayoutFullScreen(napi_env env, napi_callback_info info)
{
    WLOGFI("[NAPI]");
    JsSceneSessionManager* me = CheckParamsAndGetThis<JsSceneSessionManager>(env, info);
    return (me != nullptr) ? me->OnGetIsLayoutFullScreen(env, info) : nullptr;
}

napi_value JsSceneSessionManager::SetMaximizeFullScreen(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::WMS_LAYOUT_PC, "[NAPI]");
    JsSceneSessionManager* me = CheckParamsAndGetThis<JsSceneSessionManager>(env, info);
    return (me != nullptr) ? me->OnSetMaximizeFullScreen(env, info) : nullptr;
}

napi_value JsSceneSessionManager::GetWindowLimits(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::WMS_LAYOUT_PC, "[NAPI]");
    JsSceneSessionManager* me = CheckParamsAndGetThis<JsSceneSessionManager>(env, info);
    return (me != nullptr) ? me->OnGetWindowLimits(env, info) : nullptr;
}

napi_value JsSceneSessionManager::SetIsDockAutoHide(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::WMS_LAYOUT_PC, "[NAPI]");
    JsSceneSessionManager* me = CheckParamsAndGetThis<JsSceneSessionManager>(env, info);
    return (me != nullptr) ? me->OnSetIsDockAutoHide(env, info) : nullptr;
}

napi_value JsSceneSessionManager::SetTrayAppListInfo(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::WMS_LIFE, "[NAPI]");
    JsSceneSessionManager* me = CheckParamsAndGetThis<JsSceneSessionManager>(env, info);
    return (me != nullptr) ? me->OnHandleTrayAppChange(env, info) : nullptr;
}

napi_value JsSceneSessionManager::GetFreeMultiWindowConfig(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::WMS_LAYOUT_PC, "[NAPI]");
    JsSceneSessionManager* me = CheckParamsAndGetThis<JsSceneSessionManager>(env, info);
    return (me != nullptr) ? me->OnGetFreeMultiWindowConfig(env, info) : nullptr;
}

napi_value JsSceneSessionManager::NotifyEnterRecentTask(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::WMS_IMMS, "[NAPI]");
    JsSceneSessionManager* me = CheckParamsAndGetThis<JsSceneSessionManager>(env, info);
    return (me != nullptr) ? me->OnNotifyEnterRecentTask(env, info) : nullptr;
}

napi_value JsSceneSessionManager::UpdateDisplayHookInfo(napi_env env, napi_callback_info info)
{
    TLOGI(WmsLogTag::WMS_LAYOUT, "[NAPI]");
    JsSceneSessionManager* me = CheckParamsAndGetThis<JsSceneSessionManager>(env, info);
    return (me != nullptr) ? me->OnUpdateDisplayHookInfo(env, info) : nullptr;
}

napi_value JsSceneSessionManager::UpdateAppHookDisplayInfo(napi_env env, napi_callback_info info)
{
    TLOGI(WmsLogTag::WMS_LAYOUT, "[NAPI]");
    JsSceneSessionManager* me = CheckParamsAndGetThis<JsSceneSessionManager>(env, info);
    return (me != nullptr) ? me->OnUpdateAppHookDisplayInfo(env, info) : nullptr;
}

napi_value JsSceneSessionManager::UpdateAppHookWindowInfo(napi_env env, napi_callback_info info)
{
    TLOGI(WmsLogTag::WMS_LAYOUT, "[NAPI]");
    JsSceneSessionManager* me = CheckParamsAndGetThis<JsSceneSessionManager>(env, info);
    return (me != nullptr) ? me->OnUpdateAppHookWindowInfo(env, info) : nullptr;
}


napi_value JsSceneSessionManager::NotifyHookOrientationChange(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::WMS_COMPAT, "[NAPI]");
    JsSceneSessionManager* me = CheckParamsAndGetThis<JsSceneSessionManager>(env, info);
    return (me != nullptr) ? me->OnNotifyHookOrientationChange(env, info) : NapiGetUndefined(env);
}

napi_value JsSceneSessionManager::IsScbCoreEnabled(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::WMS_PIPELINE, "[NAPI]");
    JsSceneSessionManager* me = CheckParamsAndGetThis<JsSceneSessionManager>(env, info);
    return (me != nullptr) ? me->OnIsScbCoreEnabled(env, info) : nullptr;
}

napi_value JsSceneSessionManager::GetMaxInstanceCount(napi_env env, napi_callback_info info)
{
    JsSceneSessionManager* me = CheckParamsAndGetThis<JsSceneSessionManager>(env, info);
    if (me == nullptr) {
        TLOGW(WmsLogTag::WMS_LIFE, "me is null");
        return nullptr;
    }
    return me->OnGetMaxInstanceCount(env, info);
}

napi_value JsSceneSessionManager::GetInstanceCount(napi_env env, napi_callback_info info)
{
    JsSceneSessionManager* me = CheckParamsAndGetThis<JsSceneSessionManager>(env, info);
    if (me == nullptr) {
        TLOGW(WmsLogTag::WMS_LIFE, "me is null");
        return nullptr;
    }
    return me->OnGetInstanceCount(env, info);
}

napi_value JsSceneSessionManager::GetLastInstanceKey(napi_env env, napi_callback_info info)
{
    JsSceneSessionManager* me = CheckParamsAndGetThis<JsSceneSessionManager>(env, info);
    if (me == nullptr) {
        TLOGW(WmsLogTag::WMS_LIFE, "me is null");
        return nullptr;
    }
    return me->OnGetLastInstanceKey(env, info);
}

napi_value JsSceneSessionManager::RefreshAppInfo(napi_env env, napi_callback_info info)
{
    JsSceneSessionManager* me = CheckParamsAndGetThis<JsSceneSessionManager>(env, info);
    if (me == nullptr) {
        TLOGW(WmsLogTag::WMS_LIFE, "me is null");
        return nullptr;
    }
    return me->OnRefreshAppInfo(env, info);
}

napi_value JsSceneSessionManager::GetWindowPid(napi_env env, napi_callback_info info)
{
    JsSceneSessionManager* me = CheckParamsAndGetThis<JsSceneSessionManager>(env, info);
    if (me == nullptr) {
        TLOGW(WmsLogTag::WMS_SCB, "me is null");
        return nullptr;
    }
    return me->OnGetWindowPid(env, info);
}

napi_value JsSceneSessionManager::UpdatePcFoldScreenStatus(napi_env env, napi_callback_info info)
{
    JsSceneSessionManager* me = CheckParamsAndGetThis<JsSceneSessionManager>(env, info);
    return (me != nullptr) ? me->OnUpdatePcFoldScreenStatus(env, info) : nullptr;
}

napi_value JsSceneSessionManager::UpdateSystemKeyboardStatus(napi_env env, napi_callback_info info)
{
    JsSceneSessionManager* me = CheckParamsAndGetThis<JsSceneSessionManager>(env, info);
    return (me != nullptr) ? me->OnUpdateSystemKeyboardStatus(env, info) : nullptr;
}

napi_value JsSceneSessionManager::ResetPcFoldScreenArrangeRule(napi_env env, napi_callback_info info)
{
    JsSceneSessionManager* me = CheckParamsAndGetThis<JsSceneSessionManager>(env, info);
    return (me != nullptr) ? me->OnResetPcFoldScreenArrangeRule(env, info) : nullptr;
}

napi_value JsSceneSessionManager::RefreshPcZOrder(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::WMS_LAYOUT_PC, "[NAPI]");
    JsSceneSessionManager* me = CheckParamsAndGetThis<JsSceneSessionManager>(env, info);
    return (me != nullptr) ? me->OnRefreshPcZOrder(env, info) : nullptr;
}

napi_value JsSceneSessionManager::SetIsWindowRectAutoSave(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::WMS_MAIN, "[NAPI]");
    JsSceneSessionManager* me = CheckParamsAndGetThis<JsSceneSessionManager>(env, info);
    return (me != nullptr) ? me->OnSetIsWindowRectAutoSave(env, info) : nullptr;
}

napi_value JsSceneSessionManager::NotifyAboveLockScreen(napi_env env, napi_callback_info info)
{
    JsSceneSessionManager* me = CheckParamsAndGetThis<JsSceneSessionManager>(env, info);
    if (me == nullptr) {
        TLOGW(WmsLogTag::WMS_SCB, "me is null");
        return nullptr;
    }
    return me->OnNotifyAboveLockScreen(env, info);
}

napi_value JsSceneSessionManager::SetStatusBarAvoidHeight(napi_env env, napi_callback_info info)
{
    JsSceneSessionManager* me = CheckParamsAndGetThis<JsSceneSessionManager>(env, info);
    return (me != nullptr) ? me->OnSetStatusBarAvoidHeight(env, info) : nullptr;
}

napi_value JsSceneSessionManager::CloneWindow(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::WMS_PC, "[NAPI]");
    JsSceneSessionManager* me = CheckParamsAndGetThis<JsSceneSessionManager>(env, info);
    return (me != nullptr) ? me->OnCloneWindow(env, info) : nullptr;
}

napi_value JsSceneSessionManager::RegisterSingleHandContainerNode(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::WMS_LAYOUT, "[NAPI]");
    JsSceneSessionManager* me = CheckParamsAndGetThis<JsSceneSessionManager>(env, info);
    return (me != nullptr) ? me->OnRegisterSingleHandContainerNode(env, info) : nullptr;
}

napi_value JsSceneSessionManager::NotifyRotationChange(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::WMS_ROTATION, "[NAPI]");
    JsSceneSessionManager* me = CheckParamsAndGetThis<JsSceneSessionManager>(env, info);
    return (me != nullptr) ? me->OnNotifyRotationChange(env, info) : nullptr;
}

napi_value JsSceneSessionManager::SupportZLevel(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::WMS_HIERARCHY, "[NAPI]");
    JsSceneSessionManager* me = CheckParamsAndGetThis<JsSceneSessionManager>(env, info);
    return (me != nullptr) ? me->OnSupportZLevel(env, info) : nullptr;
}

napi_value JsSceneSessionManager::SetSupportFunctionType(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::WMS_KEYBOARD, "[NAPI]");
    JsSceneSessionManager* me = CheckParamsAndGetThis<JsSceneSessionManager>(env, info);
    return (me != nullptr) ? me->OnSetSupportFunctionType(env, info) : nullptr;
}

napi_value JsSceneSessionManager::ApplyFeatureConfig(napi_env env, napi_callback_info info)
{
    TLOGI(WmsLogTag::WMS_MAIN, "[NAPI]");
    JsSceneSessionManager* me = CheckParamsAndGetThis<JsSceneSessionManager>(env, info);
    return (me != nullptr) ? me->OnApplyFeatureConfig(env, info) : nullptr;
}

napi_value JsSceneSessionManager::OnApplyFeatureConfig(napi_env env, napi_callback_info info)
{
    size_t argc = ARGC_ONE;
    napi_value argv[ARGC_ONE] = { nullptr };
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc < ARGC_ONE) {
        TLOGE(WmsLogTag::WMS_MAIN, "Argc is invalid: %{public}zu", argc);
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    std::unordered_map<std::string, std::string> configMap;
    if (!ConvertStringMapFromJs(env, argv[ARG_INDEX_ZERO], configMap)) {
        TLOGE(WmsLogTag::WMS_MAIN, "Failed to convert parameter to configMap");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    SceneSessionManager::GetInstance().ApplyFeatureConfig(configMap);
    return NapiGetUndefined(env);
}

bool JsSceneSessionManager::IsCallbackRegistered(napi_env env, const std::string& type, napi_value jsListenerObject)
{
    HITRACE_METER_FMT(HITRACE_TAG_WINDOW_MANAGER, "JsSceneSessionManager::IsCallbackRegistered[%s]", type.c_str());
    std::shared_lock<std::shared_mutex> lock(jsCbMapMutex_);
    if (jsCbMap_.empty() || jsCbMap_.find(type) == jsCbMap_.end()) {
        return false;
    }

    for (auto iter = jsCbMap_.begin(); iter != jsCbMap_.end(); ++iter) {
        bool isEquals = false;
        napi_strict_equals(env, jsListenerObject, iter->second->GetNapiValue(), &isEquals);
        if (isEquals) {
            WLOGFE("Method %{public}s has already been registered", type.c_str());
            return true;
        }
    }
    return false;
}

napi_value JsSceneSessionManager::SupportFollowParentWindowLayout(napi_env env, napi_callback_info info)
{
    TLOGI(WmsLogTag::WMS_ROTATION, "[NAPI]");
    JsSceneSessionManager* me = CheckParamsAndGetThis<JsSceneSessionManager>(env, info);
    return (me != nullptr) ? me->OnSupportFollowParentWindowLayout(env, info) : nullptr;
}

napi_value JsSceneSessionManager::SupportFollowRelativePositionToParent(napi_env env, napi_callback_info info)
{
    TLOGI(WmsLogTag::WMS_ROTATION, "[NAPI]");
    JsSceneSessionManager* me = CheckParamsAndGetThis<JsSceneSessionManager>(env, info);
    return (me != nullptr) ? me->OnSupportFollowRelativePositionToParent(env, info) : nullptr;
}

napi_value JsSceneSessionManager::UpdateRsCmdBlockingCount(napi_env env, napi_callback_info info)
{
    TLOGI(WmsLogTag::WMS_LAYOUT, "[NAPI]");
    JsSceneSessionManager* me = CheckParamsAndGetThis<JsSceneSessionManager>(env, info);
    return (me != nullptr) ? me->OnUpdateRsCmdBlockingCount(env, info) : nullptr;
}

napi_value JsSceneSessionManager::OnRegisterCallback(napi_env env, napi_callback_info info)
{
    size_t argc = 4;
    napi_value argv[4] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc < ARGC_TWO) {
        WLOGFE("Argc is invalid: %{public}zu", argc);
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    std::string cbType;
    if (!ConvertFromJsValue(env, argv[0], cbType)) {
        WLOGFE("Failed to convert parameter to callbackType");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    napi_value value = argv[1];
    if (value == nullptr || !NapiIsCallable(env, value)) {
        WLOGFE("Callback is nullptr or not callable");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    if (IsCallbackRegistered(env, cbType, value)) {
        return NapiGetUndefined(env);
    }
    auto iterFuncType = ListenerFunctionTypeMap.find(cbType);
    if (iterFuncType == ListenerFunctionTypeMap.end()) {
        TLOGE(WmsLogTag::WMS_MAIN, "Failed to find function handler! type=%{public}s", cbType.c_str());
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    ListenerFunctionType listenerFunctionType = iterFuncType->second;
    ProcessRegisterCallback(listenerFunctionType);
    std::shared_ptr<NativeReference> callbackRef;
    napi_ref result = nullptr;
    napi_create_reference(env, value, 1, &result);
    callbackRef.reset(reinterpret_cast<NativeReference*>(result));
    {
        HITRACE_METER_FMT(HITRACE_TAG_WINDOW_MANAGER, "JsSceneSessionManager set jsCbMap[%s]", cbType.c_str());
        std::unique_lock<std::shared_mutex> lock(jsCbMapMutex_);
        jsCbMap_[cbType] = callbackRef;
    }
    WLOGFD("End, type=%{public}s", cbType.c_str());
    return NapiGetUndefined(env);
}

void JsSceneSessionManager::ProcessRegisterCallback(ListenerFunctionType listenerFunctionType)
{
    switch (listenerFunctionType) {
        case ListenerFunctionType::CREATE_SYSTEM_SESSION_CB:
            ProcessCreateSystemSessionRegister();
            break;
        case ListenerFunctionType::CREATE_KEYBOARD_SESSION_CB:
            ProcessCreateKeyboardSessionRegister();
            break;
        case ListenerFunctionType::RECOVER_SCENE_SESSION_CB:
            ProcessRecoverSceneSessionRegister();
            break;
        case ListenerFunctionType::STATUS_BAR_ENABLED_CHANGE_CB:
            ProcessStatusBarEnabledChangeListener();
            break;
        case ListenerFunctionType::OUTSIDE_DOWN_EVENT_CB:
            ProcessOutsideDownEvent();
            break;
        case ListenerFunctionType::SHIFT_FOCUS_CB:
            ProcessShiftFocus();
            break;
        case ListenerFunctionType::START_UI_ABILITY_ERROR:
            ProcessStartUIAbilityErrorRegister();
            break;
        case ListenerFunctionType::GESTURE_NAVIGATION_ENABLED_CHANGE_CB:
            ProcessGestureNavigationEnabledChangeListener();
            break;
        case ListenerFunctionType::CLOSE_TARGET_FLOAT_WINDOW_CB:
            ProcessCloseTargetFloatWindow();
            break;
        case ListenerFunctionType::ABILITY_MANAGER_COLLABORATOR_REGISTERED_CB:
            ProcessAbilityManagerCollaboratorRegistered();
            break;
        case ListenerFunctionType::START_PIP_FAILED_CB:
            ProcessStartPiPFailedRegister();
            break;
        case ListenerFunctionType::NOTIFY_APP_USE_CONTROL_LIST_CB:
            RegisterNotifyAppUseControlListCallback();
            break;
        case ListenerFunctionType::WATCH_GESTURE_CONSUME_RESULT_CB:
            RegisterWatchGestureConsumeResultCallback();
            break;
        case ListenerFunctionType::WATCH_FOCUS_ACTIVE_CHANGE_CB:
            RegisterWatchFocusActiveChangeCallback();
            break;
        case ListenerFunctionType::SET_FOREGROUND_WINDOW_NUM_CB:
            RegisterSetForegroundWindowNumCallback();
            break;
        case ListenerFunctionType::MINIMIZE_BY_WINDOW_ID_CB:
            RegisterMinimizeByWindowIdCallback();
            break;
        case ListenerFunctionType::SCENE_SESSION_DESTRUCT_CB:
            RegisterSceneSessionDestructCallback();
            break;
        case ListenerFunctionType::SCENE_SESSION_TRANSFER_TO_TARGET_SCREEN_CB:
            RegisterTransferSessionToTargetScreenCallback();
            break;
        case ListenerFunctionType::UPDATE_KIOSK_APP_LIST_CB:
            RegisterUpdateKioskAppListCallback();
            break;
        case ListenerFunctionType::KIOSK_MODE_CHANGE_CB:
            RegisterKioskModeChangeCallback();
            break;
        case ListenerFunctionType::UI_EFFECT_SET_PARAMS_CB:
            RegisterUIEffectSetParamsCallback();
            break;
        case ListenerFunctionType::UI_EFFECT_ANIMATE_TO_CB:
            RegisterUIEffectAnimateToCallback();
            break;
        case ListenerFunctionType::VIRTUAL_DENSITY_CHANGE_CB:
            RegisterVirtualPixelRatioChangeCallback();
            break;
        case ListenerFunctionType::SET_SPECIFIC_SESSION_ZINDEX_CB:
            RegisterSetSpecificWindowZIndexCallback();
            break;
        case ListenerFunctionType::NOTIFY_SUPPORT_ROTATION_REGISTERED_CB:
            ProcessSupportRotationRegister();
            break;
        case ListenerFunctionType::MINIMIZE_ALL_CB:
            RegisterMinimizeAllCallback();
            break;
        default:
            break;
    }
}

napi_value JsSceneSessionManager::OnProcessBackEvent(napi_env env, napi_callback_info info)
{
    SceneSessionManager::GetInstance().ProcessBackEvent();
    return NapiGetUndefined(env);
}

napi_value JsSceneSessionManager::OnCheckSceneZOrder(napi_env env, napi_callback_info info)
{
    SceneSessionManager::GetInstance().CheckSceneZOrder();
    return NapiGetUndefined(env);
}

static napi_value CreateWindowModes(napi_env env,
    const std::vector<AppExecFwk::SupportWindowMode>& windowModes)
{
    napi_value arrayValue = nullptr;
    napi_create_array_with_length(env, windowModes.size(), &arrayValue);
    if (arrayValue == nullptr) {
        TLOGE(WmsLogTag::WMS_SCB, "Failed to create napi array");
        return NapiGetUndefined(env);
    }
    auto index = 0;
    for (const auto& windowMode : windowModes) {
        napi_set_element(env, arrayValue, index++, CreateJsValue(env, static_cast<int32_t>(windowMode)));
    }
    return arrayValue;
}

static napi_value CreateApplicationInfo(napi_env env, const AppExecFwk::AbilityInfo& abilityInfo)
{
    napi_value objValue = nullptr;
    napi_create_object(env, &objValue);
    if (objValue == nullptr) {
        TLOGE(WmsLogTag::WMS_MAIN, "CreateObject failed");
        return NapiGetUndefined(env);
    }
    napi_set_named_property(env, objValue, "startMode", CreateJsValue(env, abilityInfo.applicationInfo.startMode));
    return objValue;
}

static napi_value CreateWindowSize(napi_env env, const AppExecFwk::AbilityInfo& abilityInfo)
{
    napi_value objValue = nullptr;
    napi_create_object(env, &objValue);
    if (objValue == nullptr) {
        WLOGFE("CreateObject failed");
        return NapiGetUndefined(env);
    }
    napi_set_named_property(env, objValue, "maxWindowRatio", CreateJsValue(env, abilityInfo.maxWindowRatio));
    napi_set_named_property(env, objValue, "minWindowRatio", CreateJsValue(env, abilityInfo.minWindowRatio));
    napi_set_named_property(env, objValue, "maxWindowWidth", CreateJsValue(env, abilityInfo.maxWindowWidth));
    napi_set_named_property(env, objValue, "minWindowWidth", CreateJsValue(env, abilityInfo.minWindowWidth));
    napi_set_named_property(env, objValue, "maxWindowHeight", CreateJsValue(env, abilityInfo.maxWindowHeight));
    napi_set_named_property(env, objValue, "minWindowHeight", CreateJsValue(env, abilityInfo.minWindowHeight));
    return objValue;
}

static napi_value CreateAbilityItemInfo(napi_env env, const AppExecFwk::AbilityInfo& abilityInfo)
{
    napi_value objValue = nullptr;
    napi_create_object(env, &objValue);
    if (objValue == nullptr) {
        WLOGFE("CreateObject failed");
        return NapiGetUndefined(env);
    }
    napi_set_named_property(env, objValue, "appIconId", CreateJsValue(env, abilityInfo.iconId));
    napi_set_named_property(env, objValue, "appLabelId", CreateJsValue(env, abilityInfo.labelId));
    napi_set_named_property(env, objValue, "bundleName", CreateJsValue(env, abilityInfo.bundleName));
    napi_set_named_property(env, objValue, "moduleName", CreateJsValue(env, abilityInfo.moduleName));
    napi_set_named_property(env, objValue, "name", CreateJsValue(env, abilityInfo.name));
    napi_set_named_property(env, objValue, "launchType",
        CreateJsValue(env, static_cast<int32_t>(abilityInfo.launchMode)));
    napi_set_named_property(env, objValue, "supportWindowModes", CreateWindowModes(env, abilityInfo.windowModes));
    napi_set_named_property(env, objValue, "windowSize", CreateWindowSize(env, abilityInfo));
    napi_set_named_property(env, objValue, "orientation",
        CreateJsValue(env, static_cast<int32_t>(abilityInfo.orientation)));
    napi_set_named_property(env, objValue, "excludeFromSession", CreateJsValue(env, abilityInfo.excludeFromMissions));
    napi_set_named_property(env, objValue, "unclearableSession", CreateJsValue(env, abilityInfo.unclearableMission));
    napi_set_named_property(env, objValue, "continuable", CreateJsValue(env, abilityInfo.continuable));
    napi_set_named_property(env, objValue, "removeSessionAfterTerminate",
        CreateJsValue(env, abilityInfo.removeMissionAfterTerminate));
    napi_set_named_property(env, objValue, "preferMultiWindowOrientation",
        CreateJsValue(env, abilityInfo.preferMultiWindowOrientation));
    napi_set_named_property(env, objValue, "isForceRotate",
        CreateJsValue(env, abilityInfo.applicationInfo.isForceRotate));
    napi_set_named_property(env, objValue, "applicationInfo", CreateApplicationInfo(env, abilityInfo));
    return objValue;
}

static napi_value CreateSCBAbilityInfo(napi_env env, const SCBAbilityInfo& scbAbilityInfo)
{
    napi_value objValue = nullptr;
    napi_create_object(env, &objValue);
    if (objValue == nullptr) {
        WLOGFE("CreateObject failed");
        return NapiGetUndefined(env);
    }
    napi_set_named_property(env, objValue, "abilityItemInfo", CreateAbilityItemInfo(env, scbAbilityInfo.abilityInfo_));
    napi_set_named_property(env, objValue, "sdkVersion", CreateJsValue(env, scbAbilityInfo.sdkVersion_));
    napi_set_named_property(env, objValue, "codePath", CreateJsValue(env, scbAbilityInfo.codePath_));
    napi_set_named_property(
        env, objValue, "isAbilityHook", CreateJsValue(env, scbAbilityInfo.isAbilityHook_));
    return objValue;
}

static napi_value CreateAbilityInfos(napi_env env, const std::vector<SCBAbilityInfo>& scbAbilityInfos)
{
    napi_value arrayValue = nullptr;
    napi_create_array_with_length(env, scbAbilityInfos.size(), &arrayValue);
    if (arrayValue == nullptr) {
        TLOGE(WmsLogTag::WMS_SCB, "Failed to create napi array");
        return NapiGetUndefined(env);
    }
    auto index = 0;
    for (const auto& scbAbilityInfo : scbAbilityInfos) {
        napi_set_element(env, arrayValue, index++, CreateSCBAbilityInfo(env, scbAbilityInfo));
    }
    return arrayValue;
}

napi_value JsSceneSessionManager::OnGetAllAbilityInfos(napi_env env, napi_callback_info info)
{
    size_t argc = 4;
    napi_value argv[4] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc < ARGC_TWO || argc > ARGC_THREE) {
        WLOGFE("Argc is invalid: %{public}zu", argc);
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    AAFwk::Want want;
    bool ret = OHOS::AppExecFwk::UnwrapWant(env, argv[0], want);
    if (!ret) {
        WLOGFE("Failed to convert parameter to want");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    int32_t userId;
    if (!ConvertFromJsValue(env, argv[1], userId)) {
        WLOGFE("Failed to convert parameter to userId");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    auto errCode = std::make_shared<int32_t>(static_cast<int32_t>(WSErrorCode::WS_OK));
    auto scbAbilityInfos = std::make_shared<std::vector<SCBAbilityInfo>>();
    auto execute = [want, userId, infos = scbAbilityInfos, errCode]() {
        auto code = WS_JS_TO_ERROR_CODE_MAP.at(
            SceneSessionManager::GetInstance().GetAllAbilityInfos(want, userId, *infos));
        *errCode = static_cast<int32_t>(code);
    };
    auto complete = [errCode, infos = scbAbilityInfos]
        (napi_env env, NapiAsyncTask& task, int32_t status) {
        if (*errCode != static_cast<int32_t>(WSErrorCode::WS_OK)) {
            std::string errMsg = "invalid params can not get All AbilityInfos!";
            task.RejectWithCustomize(env, CreateJsValue(env, *errCode), CreateJsValue(env, errMsg));
            return;
        }
        task.ResolveWithCustomize(env, CreateJsValue(env, static_cast<int32_t>(WSErrorCode::WS_OK)),
            CreateAbilityInfos(env, *infos));
    };
    napi_value result = nullptr;
    napi_value callback = (argc == 2) ? nullptr : argv[2];
    NapiAsyncTask::Schedule("JsSceneSessionManager::OnGetAllAbilityInfos",
        env, CreateAsyncTaskWithLastParam(env, callback, std::move(execute), std::move(complete), &result));
    return result;
}

napi_value JsSceneSessionManager::OnGetBatchAbilityInfos(napi_env env, napi_callback_info info)
{
    size_t argc = DEFAULT_ARG_COUNT;
    napi_value argv[DEFAULT_ARG_COUNT] = { nullptr };
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc != ARGC_TWO) {
        TLOGE(WmsLogTag::DEFAULT, "Argc is invalid: %{public}zu", argc);
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    int32_t userId;
    if (!ConvertFromJsValue(env, argv[0], userId)) {
        TLOGE(WmsLogTag::DEFAULT, "Failed to convert parameter to userId");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    std::vector<std::string> bundleNames;
    if (!ParseArrayStringValue(env, argv[ARG_INDEX_ONE], bundleNames)) {
        TLOGE(WmsLogTag::DEFAULT, "Failed to convert parameter to bundleNames");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    auto errCode = std::make_shared<WSErrorCode>(WSErrorCode::WS_OK);
    auto scbAbilityInfos = std::make_shared<std::vector<SCBAbilityInfo>>();
    auto execute = [bundleNames = std::move(bundleNames), userId, infos = scbAbilityInfos, errCode] {
        *errCode = WS_JS_TO_ERROR_CODE_MAP.at(
            SceneSessionManager::GetInstance().GetBatchAbilityInfos(bundleNames, userId, *infos));
    };
    auto complete = [errCode, infos = scbAbilityInfos](napi_env env, NapiAsyncTask& task, int32_t status) {
        if (*errCode != WSErrorCode::WS_OK) {
            std::string errMsg = "invalid params can not get batch AbilityInfos!";
            task.RejectWithCustomize(env, CreateJsValue(env, *errCode), CreateJsValue(env, errMsg));
            return;
        }
        task.ResolveWithCustomize(env, CreateJsValue(env, static_cast<int32_t>(WSErrorCode::WS_OK)),
            CreateAbilityInfos(env, *infos));
    };
    napi_value result = nullptr;
    napi_value callback = nullptr;
    NapiAsyncTask::Schedule("JsSceneSessionManager::OnGetBatchAbilityInfos",
        env, CreateAsyncTaskWithLastParam(env, callback, std::move(execute), std::move(complete), &result));
    return result;
}

napi_value JsSceneSessionManager::OnGetAbilityInfo(napi_env env, napi_callback_info info)
{
    size_t argc = DEFAULT_ARG_COUNT;
    napi_value argv[DEFAULT_ARG_COUNT] = { nullptr };
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc != ARGC_FOUR) {
        TLOGE(WmsLogTag::DEFAULT, "Argc is invalid: %{public}zu", argc);
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    std::string bundleName;
    if (!ConvertFromJsValue(env, argv[0], bundleName)) {
        TLOGE(WmsLogTag::DEFAULT, "Failed to convert parameter to bundleName");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    std::string moduleName;
    if (!ConvertFromJsValue(env, argv[ARG_INDEX_ONE], moduleName)) {
        TLOGE(WmsLogTag::DEFAULT, "Failed to convert parameter to moduleName");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    std::string abilityName;
    if (!ConvertFromJsValue(env, argv[ARG_INDEX_TWO], abilityName)) {
        TLOGE(WmsLogTag::DEFAULT, "Failed to convert parameter to abilityName");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    int32_t userId = 0;
    if (!ConvertFromJsValue(env, argv[ARG_INDEX_THREE], userId)) {
        TLOGE(WmsLogTag::DEFAULT, "Failed to convert parameter to userId");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    SCBAbilityInfo scbAbilityInfo;
    WSErrorCode ret = WS_JS_TO_ERROR_CODE_MAP.at(SceneSessionManager::GetInstance().GetAbilityInfo(
        bundleName, moduleName, abilityName, userId, scbAbilityInfo));
    if (ret != WSErrorCode::WS_OK) {
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_STATE_ABNORMALLY),
            "System is abnormal"));
        return NapiGetUndefined(env);
    }
    return CreateSCBAbilityInfo(env, scbAbilityInfo);
}

napi_value JsSceneSessionManager::OnInitUserInfo(napi_env env, napi_callback_info info)
{
    size_t argc = 4;
    napi_value argv[4] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc < ARGC_TWO) {
        TLOGE(WmsLogTag::WMS_MAIN, "Argc is invalid: %{public}zu", argc);
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    int32_t userId;
    if (!ConvertFromJsValue(env, argv[0], userId)) { // 1: params num
        TLOGE(WmsLogTag::WMS_MAIN, "Failed to convert parameter to userId");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    std::string fileDir;
    if (!ConvertFromJsValue(env, argv[1], fileDir)) { // 2: params num
        TLOGE(WmsLogTag::WMS_MAIN, "NAPI]Failed to convert parameter to fileDir");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    WSErrorCode ret =
        WS_JS_TO_ERROR_CODE_MAP.at(SceneSessionManager::GetInstance().InitUserInfo(userId, fileDir));
    if (ret != WSErrorCode::WS_OK) {
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_STATE_ABNORMALLY),
            "System is abnormal"));
    }
    return NapiGetUndefined(env);
}

void JsSceneSessionManager::RegisterDumpRootSceneElementInfoListener()
{
    DumpRootSceneElementInfoFunc func = [this](const sptr<SceneSession>& session,
        const std::vector<std::string>& params, std::vector<std::string>& infos) {
        if (session == nullptr) {
            return;
        }
        const auto& uiContent = rootScene_->
            GetUIContentByDisplayId(session->GetSessionProperty()->GetDisplayId()).first;
        if (params.size() == 1 && params[0] == ARG_DUMP_HELP) { // 1: params num
            Ace::UIContent::ShowDumpHelp(infos);
            TLOGND(WmsLogTag::WMS_EVENT, "Dump ArkUI help info");
        } else if (uiContent != nullptr) {
            uiContent->DumpInfo(params, infos);
            TLOGND(WmsLogTag::WMS_EVENT, "Dump ArkUI element info");
        }
    };
    SceneSessionManager::GetInstance().SetDumpRootSceneElementInfoListener(func);
}

void JsSceneSessionManager::RegisterVirtualPixelRatioChangeListener()
{
    ProcessVirtualPixelRatioChangeFunc func = [this](float density, const Rect& rect) {
        WLOGFI("VirtualPixelRatioChangeListener %{public}d,%{public}d,%{public}d,%{public}d; %{public}f",
            rect.posX_, rect.posY_, rect.width_, rect.height_, density);
        RootScene::staticRootScene_->SetDisplayDensity(density);
        RootScene::staticRootScene_->UpdateViewportConfig(rect, WindowSizeChangeReason::UNDEFINED);
    };
    SceneSessionManager::GetInstance().SetVirtualPixelRatioChangeListener(func);
}

napi_value JsSceneSessionManager::OnGetRootSceneSession(napi_env env, napi_callback_info info)
{
    WLOGFI("in");
    sptr<RootSceneSession> rootSceneSession = SceneSessionManager::GetInstance().GetRootSceneSession();
    if (rootSceneSession == nullptr) {
        napi_throw(env,
            CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_STATE_ABNORMALLY), "System is abnormal"));
        return NapiGetUndefined(env);
    }
    if (rootScene_ == nullptr) {
        rootScene_ = sptr<RootScene>::MakeSptr();
    }
    RootScene::staticRootScene_ = rootScene_;
    rootSceneSession->SetLoadContentFunc([rootScene = rootScene_]
        (const std::string& contentUrl, napi_env env, napi_value storage, AbilityRuntime::Context* context) {
            rootScene->LoadContent(contentUrl, env, storage, context);
            ScenePersistentStorage::InitDir(context->GetPreferencesDir());
            SceneSessionManager::GetInstance().InitPersistentStorage();
            SceneSessionManager::GetInstance().UpdateAllStartingWindowRdb();
        });
    rootSceneSession->SetGetUIContentFunc([rootScene = rootScene_](DisplayId displayId) -> Ace::UIContent* {
        const auto& uiContentPair = rootScene->GetUIContentByDisplayId(displayId);
        TLOGNI(WmsLogTag::WMS_ATTRIBUTE, "found=%{public}d, displayId: %{public}" PRIu64,
            uiContentPair.second, displayId);
        if (uiContentPair.second) {
            return uiContentPair.first;
        }
        return nullptr;
    });
    RegisterRootSceneCallbacksOnSSManager();
    RegisterSSManagerCallbacksOnRootScene();
    napi_value jsRootSceneSessionObj = JsRootSceneSession::Create(env, rootSceneSession);
    if (jsRootSceneSessionObj == nullptr) {
        WLOGFE("jsRootSceneSessionObj is nullptr");
        napi_throw(env,
            CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_STATE_ABNORMALLY), "System is abnormal"));
        return NapiGetUndefined(env);
    }
    return jsRootSceneSessionObj;
}

napi_value JsSceneSessionManager::OnRequestSceneSession(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::WMS_LIFE, "in");
    WSErrorCode errCode = WSErrorCode::WS_OK;
    size_t argc = 4;
    napi_value argv[4] = { nullptr };
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc < ARGC_ONE) {
        WLOGFE("Argc is invalid: %{public}zu", argc);
        errCode = WSErrorCode::WS_ERROR_INVALID_PARAM;
    }

    SessionInfo sessionInfo;
    AAFwk::Want want;
    if (errCode == WSErrorCode::WS_OK) {
        napi_value nativeObj = argv[0];
        if (nativeObj == nullptr) {
            WLOGFE("Failed to convert object to session info");
            errCode = WSErrorCode::WS_ERROR_INVALID_PARAM;
        } else if (!ConvertSessionInfoFromJs(env, nativeObj, sessionInfo)) {
            WLOGFE("Failed to get session info from js object");
            errCode = WSErrorCode::WS_ERROR_INVALID_PARAM;
        }
    }
    if (errCode == WSErrorCode::WS_ERROR_INVALID_PARAM) {
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    if (argc == ARGC_TWO && GetType(env, argv[1]) != napi_undefined) {
        OHOS::AppExecFwk::UnwrapWant(env, argv[1], want);
        sessionInfo.want = std::make_shared<AAFwk::Want>(want);
    }

    TLOGI(WmsLogTag::WMS_MAIN, "[%{public}s, %{public}s, %{public}s], sceneType: %{public}d, errCode=%{public}d",
        sessionInfo.bundleName_.c_str(), sessionInfo.moduleName_.c_str(), sessionInfo.abilityName_.c_str(),
        static_cast<uint32_t>(sessionInfo.sceneType_), errCode);
    sptr<SceneSession> sceneSession = SceneSessionManager::GetInstance().RequestSceneSession(sessionInfo);
    if (sceneSession == nullptr) {
        napi_throw(env,
            CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_STATE_ABNORMALLY), "System is abnormal"));
        return NapiGetUndefined(env);
    } else {
        napi_value jsSceneSessionObj = JsSceneSession::Create(env, sceneSession);
        if (jsSceneSessionObj == nullptr) {
            WLOGFE("jsSceneSessionObj is nullptr");
            napi_throw(env, CreateJsError(
                env, static_cast<int32_t>(WSErrorCode::WS_ERROR_STATE_ABNORMALLY), "System is abnormal"));
        }
        return jsSceneSessionObj;
    }
}

napi_value JsSceneSessionManager::OnUpdateSceneSessionWant(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::WMS_MAIN, "in");
    WSErrorCode errCode = WSErrorCode::WS_OK;
    size_t argc = 4;
    napi_value argv[4] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc < ARGC_TWO) {
        WLOGFE("Argc is invalid: %{public}zu", argc);
        errCode = WSErrorCode::WS_ERROR_INVALID_PARAM;
    }

    SessionInfo sessionInfo;
    AAFwk::Want want;
    if (errCode == WSErrorCode::WS_OK) {
        napi_value nativeObj = argv[0];
        if (nativeObj == nullptr) {
            TLOGE(WmsLogTag::WMS_MAIN, "Failed to convert to session info");
            errCode = WSErrorCode::WS_ERROR_INVALID_PARAM;
        } else if (!ConvertSessionInfoFromJs(env, nativeObj, sessionInfo)) {
            TLOGE(WmsLogTag::WMS_MAIN, "Failed to get session info from js object");
            errCode = WSErrorCode::WS_ERROR_INVALID_PARAM;
        } else if (!OHOS::AppExecFwk::UnwrapWant(env, argv[1], want)) {
            TLOGE(WmsLogTag::WMS_MAIN, "Failed to get want from js object");
            errCode = WSErrorCode::WS_ERROR_INVALID_PARAM;
        }
    }
    if (errCode == WSErrorCode::WS_ERROR_INVALID_PARAM) {
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    sessionInfo.want = std::make_shared<AAFwk::Want>(want);

    TLOGI(WmsLogTag::WMS_MAIN, "[%{public}s, %{public}s, %{public}s, %{public}d]",
        sessionInfo.bundleName_.c_str(), sessionInfo.moduleName_.c_str(), sessionInfo.abilityName_.c_str(),
        sessionInfo.persistentId_);
    SceneSessionManager::GetInstance().UpdateSceneSessionWant(sessionInfo);
    return NapiGetUndefined(env);
}

napi_value JsSceneSessionManager::OnRequestSceneSessionActivation(napi_env env, napi_callback_info info)
{
    TLOGI(WmsLogTag::WMS_LIFE, "in");
    size_t argc = 4;
    napi_value argv[4] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc < ARGC_TWO) {
        TLOGE(WmsLogTag::WMS_LIFE, "Argc is invalid: %{public}zu", argc);
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM), "InputInvalid"));
        return NapiGetUndefined(env);
    }
    napi_value jsSceneSessionObj = argv[0];
    if (jsSceneSessionObj == nullptr) {
        TLOGE(WmsLogTag::WMS_LIFE, "Failed to get js session object");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM), "InputInvalid"));
        return NapiGetUndefined(env);
    }
    void* pointerResult = nullptr;
    napi_unwrap(env, jsSceneSessionObj, &pointerResult);
    auto jsSceneSession = static_cast<JsSceneSession*>(pointerResult);
    if (jsSceneSession == nullptr) {
        TLOGE(WmsLogTag::WMS_LIFE, "Failed to get session from js object");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM), "InputInvalid"));
        return NapiGetUndefined(env);
    }
    sptr<SceneSession> sceneSession = jsSceneSession->GetNativeSession();
    if (sceneSession == nullptr) {
        TLOGE(WmsLogTag::WMS_LIFE, "sceneSession is nullptr");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_SYSTEM_ABNORMALLY),
            "sceneSession is nullptr"));
        return NapiGetUndefined(env);
    }

    bool isNewActive = true;
    ConvertFromJsValue(env, argv[1], isNewActive);
    int32_t requestId = DEFAULT_REQUEST_FROM_SCB_ID;
    ConvertFromJsValue(env, argv[ARG_INDEX_TWO], requestId);
    SceneSessionManager::GetInstance().RequestSceneSessionActivation(sceneSession, isNewActive, requestId);
    return NapiGetUndefined(env);
}

napi_value JsSceneSessionManager::OnRequestSceneSessionBackground(napi_env env, napi_callback_info info)
{
    TLOGI(WmsLogTag::WMS_LIFE, "in");
    WSErrorCode errCode = WSErrorCode::WS_OK;
    size_t argc = 5;
    napi_value argv[5] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc < ARGC_ONE) {
        TLOGE(WmsLogTag::WMS_LIFE, "Argc is invalid: %{public}zu", argc);
        errCode = WSErrorCode::WS_ERROR_INVALID_PARAM;
    }

    sptr<SceneSession> sceneSession = nullptr;
    if (errCode == WSErrorCode::WS_OK) {
        napi_value jsSceneSessionObj = argv[0];
        if (jsSceneSessionObj == nullptr) {
            TLOGE(WmsLogTag::WMS_LIFE, "Failed to get js scene session object");
            errCode = WSErrorCode::WS_ERROR_INVALID_PARAM;
        } else {
            void* pointerResult = nullptr;
            napi_unwrap(env, jsSceneSessionObj, &pointerResult);
            auto jsSceneSession = static_cast<JsSceneSession*>(pointerResult);
            if (jsSceneSession == nullptr) {
                TLOGE(WmsLogTag::WMS_LIFE, "Failed to get scene session from js object");
                errCode = WSErrorCode::WS_ERROR_INVALID_PARAM;
            } else {
                sceneSession = jsSceneSession->GetNativeSession();
            }
        }
    }
    if (sceneSession == nullptr) {
        TLOGE(WmsLogTag::WMS_LIFE, "sceneSession is nullptr");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_SYSTEM_ABNORMALLY),
            "sceneSession is nullptr"));
        return NapiGetUndefined(env);
    }

    if (errCode == WSErrorCode::WS_ERROR_INVALID_PARAM) {
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }

    bool isDelegator = false;
    if (argc >= ARGC_TWO && GetType(env, argv[ARGC_ONE]) == napi_boolean) {
        ConvertFromJsValue(env, argv[ARGC_ONE], isDelegator);
        TLOGI(WmsLogTag::WMS_LIFE, "isDelegator: %{public}u", isDelegator);
    }

    bool isToDesktop = false;
    if (argc >= ARGC_THREE && GetType(env, argv[ARGC_TWO]) == napi_boolean) {
        ConvertFromJsValue(env, argv[ARGC_TWO], isToDesktop);
        TLOGI(WmsLogTag::WMS_LIFE, "isToDesktop: %{public}u", isToDesktop);
    }

    bool isSaveSnapshot = true;
    if (argc >= ARGC_FOUR && GetType(env, argv[ARGC_THREE]) == napi_boolean) {
        ConvertFromJsValue(env, argv[ARGC_THREE], isSaveSnapshot);
        TLOGI(WmsLogTag::WMS_LIFE, "isSaveSnapshot: %{public}u", isSaveSnapshot);
    }

    LifeCycleChangeReason reason = LifeCycleChangeReason::DEFAULT;
    if (argc >= ARGC_FIVE) {
        ConvertFromJsValue(env, argv[ARGC_FOUR], reason);
        TLOGI(WmsLogTag::WMS_LIFE, "backgroundReason: %{public}u", reason);
    }

    SceneSessionManager::GetInstance().RequestSceneSessionBackground(sceneSession, isDelegator, isToDesktop,
        isSaveSnapshot, reason);
    return NapiGetUndefined(env);
}

napi_value JsSceneSessionManager::OnRequestSceneSessionDestruction(napi_env env, napi_callback_info info)
{
    TLOGI(WmsLogTag::WMS_LIFE, "in");
    WSErrorCode errCode = WSErrorCode::WS_OK;
    size_t argc = 6;
    napi_value argv[6] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc < ARGC_ONE) {
        TLOGE(WmsLogTag::WMS_LIFE, "Argc is invalid: %{public}zu", argc);
        errCode = WSErrorCode::WS_ERROR_INVALID_PARAM;
    }

    sptr<SceneSession> sceneSession = nullptr;
    JsSceneSession* jsSceneSession;
    if (errCode == WSErrorCode::WS_OK) {
        napi_value jsSceneSessionObj = argv[0];
        if (jsSceneSessionObj == nullptr) {
            TLOGE(WmsLogTag::WMS_LIFE, "Failed to get js scene session object");
            errCode = WSErrorCode::WS_ERROR_INVALID_PARAM;
        } else {
            void* pointerResult = nullptr;
            napi_unwrap(env, jsSceneSessionObj, &pointerResult);
            jsSceneSession = static_cast<JsSceneSession*>(pointerResult);
            if (jsSceneSession == nullptr) {
                TLOGE(WmsLogTag::WMS_LIFE, "Failed to get session from js object");
                errCode = WSErrorCode::WS_ERROR_INVALID_PARAM;
            } else {
                sceneSession = jsSceneSession->GetNativeSession();
                SetIsClearSession(env, jsSceneSessionObj, sceneSession);
            }
        }
    }
    if (sceneSession == nullptr) {
        TLOGE(WmsLogTag::WMS_LIFE, "sceneSession is nullptr");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_SYSTEM_ABNORMALLY),
            "sceneSession is nullptr"));
        return NapiGetUndefined(env);
    }

    bool needRemoveSession = false;
    if (argc >= ARGC_TWO && GetType(env, argv[ARGC_ONE]) == napi_boolean) {
        RETURN_IF_CONVERT_FAIL(env, argv[ARGC_ONE], needRemoveSession, "needRemoveSession", WmsLogTag::WMS_LIFE);
    }

    bool isSaveSnapshot = true;
    if (argc >= ARGC_THREE && GetType(env, argv[ARGC_TWO]) == napi_boolean) {
        RETURN_IF_CONVERT_FAIL(env, argv[ARGC_TWO], isSaveSnapshot, "isSaveSnapshot", WmsLogTag::WMS_LIFE);
    }

    bool isForceClean = false;
    if (argc >= ARGC_FOUR && GetType(env, argv[ARGC_THREE]) == napi_boolean) {
        RETURN_IF_CONVERT_FAIL(env, argv[ARGC_THREE], isForceClean, "isForceClean", WmsLogTag::WMS_LIFE);
    }

    bool isUserRequestedExit = false;
    if (argc >= ARGC_FIVE && GetType(env, argv[ARGC_FOUR]) == napi_boolean) {
        RETURN_IF_CONVERT_FAIL(env, argv[ARGC_FOUR], isUserRequestedExit, "isUserRequestedExit", WmsLogTag::WMS_LIFE);
    }

    LifeCycleChangeReason terminateReason = LifeCycleChangeReason::DEFAULT;
    if (argc >= 6) {
        RETURN_IF_CONVERT_FAIL(env, argv[ARGC_FIVE], terminateReason, "terminateReason", WmsLogTag::WMS_LIFE);
    }

    if (errCode == WSErrorCode::WS_ERROR_INVALID_PARAM) {
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }

    SceneSessionManager::GetInstance().RequestSceneSessionDestruction(sceneSession,
        needRemoveSession, isSaveSnapshot, isForceClean, isUserRequestedExit, terminateReason);
    return NapiGetUndefined(env);
}

napi_value JsSceneSessionManager::OnNotifyForegroundInteractiveStatus(napi_env env, napi_callback_info info)
{
    TLOGI(WmsLogTag::WMS_LIFE, "in");
    size_t argc = 4;
    napi_value argv[4] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc < ARGC_TWO) {
        WLOGFE("Argc is invalid: %{public}zu", argc);
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM), "InputInvalid"));
        return NapiGetUndefined(env);
    }

    napi_value jsSceneSessionObj = argv[0];
    if (jsSceneSessionObj == nullptr) {
        WLOGFE("Failed to get js scene session object");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM), "InputInvalid"));
        return NapiGetUndefined(env);
    }
    void* pointerResult = nullptr;
    napi_unwrap(env, jsSceneSessionObj, &pointerResult);
    auto jsSceneSession = static_cast<JsSceneSession*>(pointerResult);
    if (jsSceneSession == nullptr) {
        WLOGFE("Failed to get scene session from js object");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM), "InputInvalid"));
        return NapiGetUndefined(env);
    }
    sptr<SceneSession> sceneSession = jsSceneSession->GetNativeSession();
    if (sceneSession == nullptr) {
        WLOGFE("sceneSession is nullptr");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_SYSTEM_ABNORMALLY),
            "sceneSession is nullptr"));
        return NapiGetUndefined(env);
    }

    bool interactive = true;
    ConvertFromJsValue(env, argv[1], interactive);
    SceneSessionManager::GetInstance().NotifyForegroundInteractiveStatus(sceneSession, interactive);
    return NapiGetUndefined(env);
}

napi_value JsSceneSessionManager::OnIsSceneSessionValid(napi_env env, napi_callback_info info)
{
    TLOGI(WmsLogTag::WMS_LIFE, "in");
    size_t argc = 4;
    napi_value argv[4] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc < ARGC_ONE) {
        TLOGE(WmsLogTag::WMS_LIFE, "Argc is invalid: %{public}zu", argc);
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM), "InputInvalid"));
        return NapiGetUndefined(env);
    }
    napi_value jsSceneSessionObj = argv[0];
    if (jsSceneSessionObj == nullptr) {
        TLOGE(WmsLogTag::WMS_LIFE, "Failed to get js scene session object");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM), "InputInvalid"));
        return NapiGetUndefined(env);
    }
    void* pointerResult = nullptr;
    napi_unwrap(env, jsSceneSessionObj, &pointerResult);
    auto jsSceneSession = static_cast<JsSceneSession*>(pointerResult);
    if (jsSceneSession == nullptr) {
        TLOGE(WmsLogTag::WMS_LIFE, "Failed to get session from js object");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM), "InputInvalid"));
        return NapiGetUndefined(env);
    }
    napi_value result = nullptr;
    sptr<SceneSession> sceneSession = jsSceneSession->GetNativeSession();
    napi_get_boolean(env, sceneSession != nullptr, &result);
    return result;
}

void JsSceneSessionManager::SetIsClearSession(napi_env env, napi_value jsSceneSessionObj,
    sptr<SceneSession>& sceneSession)
{
    if (sceneSession == nullptr) {
        WLOGFE("sceneSession is null");
        return;
    }
    napi_value jsOperatorType = nullptr;
    napi_get_named_property(env, jsSceneSessionObj, "operatorType", &jsOperatorType);
    if (GetType(env, jsOperatorType) != napi_undefined) {
        int32_t operatorType = -1;
        if (ConvertFromJsValue(env, jsOperatorType, operatorType)) {
            TLOGD(WmsLogTag::DEFAULT, "operatorType: %{public}d", operatorType);
            if (operatorType == SessionOperationType::TYPE_CLEAR) {
                sceneSession->SetSessionInfoIsClearSession(true);
            }
        }
    }
}

napi_value JsSceneSessionManager::OnRequestSceneSessionByCall(napi_env env, napi_callback_info info)
{
    TLOGI(WmsLogTag::WMS_LIFE, "in");
    WSErrorCode errCode = WSErrorCode::WS_OK;
    size_t argc = 4;
    napi_value argv[4] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc < ARGC_TWO) {
        WLOGFE("Argc is invalid: %{public}zu", argc);
        errCode = WSErrorCode::WS_ERROR_INVALID_PARAM;
    }

    sptr<SceneSession> sceneSession = nullptr;
    if (errCode == WSErrorCode::WS_OK) {
        napi_value jsSceneSessionObj = argv[0];
        if (jsSceneSessionObj == nullptr) {
            WLOGFE("Failed to get js scene session object");
            errCode = WSErrorCode::WS_ERROR_INVALID_PARAM;
        } else {
            void* pointerResult = nullptr;
            napi_unwrap(env, jsSceneSessionObj, &pointerResult);
            auto jsSceneSession = static_cast<JsSceneSession*>(pointerResult);
            if (jsSceneSession == nullptr) {
                WLOGFE("Failed to get scene session from js object");
                errCode = WSErrorCode::WS_ERROR_INVALID_PARAM;
            } else {
                sceneSession = jsSceneSession->GetNativeSession();
            }
        }
    }
    if (sceneSession == nullptr) {
        WLOGFE("sceneSession is nullptr");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_SYSTEM_ABNORMALLY),
            "sceneSession is nullptr"));
        return NapiGetUndefined(env);
    }

    if (errCode == WSErrorCode::WS_ERROR_INVALID_PARAM) {
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }

    int32_t requestId = DEFAULT_REQUEST_FROM_SCB_ID;
    ConvertFromJsValue(env, argv[1], requestId);
    SceneSessionManager::GetInstance().RequestSceneSessionByCall(sceneSession, requestId);
    return NapiGetUndefined(env);
}

napi_value JsSceneSessionManager::OnSetBehindWindowFilterEnabled(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::DEFAULT, "in");
    size_t argc = ARGC_FOUR;
    napi_value argv[ARGC_FOUR] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc != ARGC_ONE) {
        TLOGD(WmsLogTag::WMS_PC, "Argc is invalid: %{public}zu", argc);
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
        "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    bool behindWindowFilterEnabled = false;
    if (!ConvertFromJsValue(env, argv[0], behindWindowFilterEnabled)) {
        TLOGD(WmsLogTag::WMS_PC, "Faile to convert parameter to behindWindowFilterEnabled.");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is invalid."));
        return NapiGetUndefined(env);
    }
    SceneSessionManager::GetInstance().SetBehindWindowFilterEnabled(behindWindowFilterEnabled);
    return NapiGetUndefined(env);
}

napi_value JsSceneSessionManager::OnStartAbilityBySpecified(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::WMS_LIFE, "in");
    WSErrorCode errCode = WSErrorCode::WS_OK;
    size_t argc = 4;
    napi_value argv[4] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc < 1) {
        WLOGFE("Argc is invalid: %{public}zu", argc);
        errCode = WSErrorCode::WS_ERROR_INVALID_PARAM;
    }

    SessionInfo sessionInfo;
    if (errCode == WSErrorCode::WS_OK) {
        napi_value nativeObj = argv[0];
        if (nativeObj == nullptr) {
            WLOGFE("Failed to convert object to session info");
            errCode = WSErrorCode::WS_ERROR_INVALID_PARAM;
        } else if (!ConvertSessionInfoFromJs(env, nativeObj, sessionInfo)) {
            WLOGFE("Failed to get session info from js object");
            errCode = WSErrorCode::WS_ERROR_INVALID_PARAM;
        }
    }

    if (errCode == WSErrorCode::WS_ERROR_INVALID_PARAM) {
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }

    AAFwk::Want want;
    if (OHOS::AppExecFwk::UnwrapWant(env, argv[1], want)) {
        sessionInfo.want = std::make_shared<AAFwk::Want>(want);
    }

    WLOGFI("[%{public}s, %{public}s, %{public}s], errCode=%{public}d",
        sessionInfo.bundleName_.c_str(), sessionInfo.moduleName_.c_str(), sessionInfo.abilityName_.c_str(), errCode);
    SceneSessionManager::GetInstance().StartAbilityBySpecified(sessionInfo);
    return NapiGetUndefined(env);
}

napi_value JsSceneSessionManager::OnStartUIAbilityBySCB(napi_env env, napi_callback_info info)
{
    TLOGI(WmsLogTag::WMS_LIFE, "in");
    WSErrorCode errCode = WSErrorCode::WS_OK;
    size_t argc = 4;
    napi_value argv[4] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc < 1) {
        WLOGFE("Argc is invalid: %{public}zu", argc);
        errCode = WSErrorCode::WS_ERROR_INVALID_PARAM;
    }

    sptr<SceneSession> sceneSession = nullptr;
    if (errCode == WSErrorCode::WS_OK) {
        napi_value nativeObj = argv[0];
        if (nativeObj == nullptr) {
            WLOGFE("Failed to convert object to session info");
            errCode = WSErrorCode::WS_ERROR_INVALID_PARAM;
        } else {
            void* pointerResult = nullptr;
            napi_unwrap(env, nativeObj, &pointerResult);
            auto jsSceneSession = static_cast<JsSceneSession*>(pointerResult);
            if (jsSceneSession == nullptr) {
                errCode = WSErrorCode::WS_ERROR_INVALID_PARAM;
            } else {
                sceneSession = jsSceneSession->GetNativeSession();
            }
        }
    }
    if (sceneSession == nullptr) {
        WLOGFE("sceneSession is nullptr");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_SYSTEM_ABNORMALLY),
            "sceneSession is nullptr"));
        return NapiGetUndefined(env);
    }

    if (errCode == WSErrorCode::WS_ERROR_INVALID_PARAM) {
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }

    SceneSessionManager::GetInstance().StartUIAbilityBySCB(sceneSession);
    return NapiGetUndefined(env);
}

napi_value JsSceneSessionManager::OnChangeUIAbilityVisibilityBySCB(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::WMS_LIFE, "in");
    size_t argc = DEFAULT_ARG_COUNT;
    napi_value argv[DEFAULT_ARG_COUNT] = { nullptr };
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc < 1 || argv[ARG_INDEX_ZERO] == nullptr) {
        TLOGE(WmsLogTag::WMS_LIFE, "Argc is invalid: %{public}zu or nativeObj is nullptr", argc);
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }

    void* result = nullptr;
    napi_unwrap(env, argv[ARG_INDEX_ZERO], &result);
    auto jsSceneSession = static_cast<JsSceneSession*>(result);
    if (jsSceneSession == nullptr) {
        TLOGE(WmsLogTag::WMS_LIFE, "jsSceneSession is nullptr");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "jsSceneSession is nullptr"));
        return NapiGetUndefined(env);
    }

    sptr<SceneSession> sceneSession = jsSceneSession->GetNativeSession();
    if (sceneSession == nullptr) {
        TLOGE(WmsLogTag::WMS_LIFE, "sceneSession is nullptr");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_SYSTEM_ABNORMALLY),
            "sceneSession is nullptr"));
        return NapiGetUndefined(env);
    }

    bool visibility = true;
    ConvertFromJsValue(env, argv[ARG_INDEX_ONE], visibility);
    bool isFromClient = true;
    ConvertFromJsValue(env, argv[ARG_INDEX_TWO], isFromClient);
    bool isNewWant = false;
    ConvertFromJsValue(env, argv[ARG_INDEX_THREE], isNewWant);

    SceneSessionManager::GetInstance().ChangeUIAbilityVisibilityBySCB(
        sceneSession, visibility, isNewWant, isFromClient);
    return NapiGetUndefined(env);
}

napi_value JsSceneSessionManager::OnGetWindowSceneConfig(napi_env env, napi_callback_info info)
{
    WLOGFD("in");
    const AppWindowSceneConfig& windowSceneConfig = SceneSessionManager::GetInstance().GetWindowSceneConfig();
    napi_value jsWindowSceneConfigObj = JsWindowSceneConfig::CreateWindowSceneConfig(env, windowSceneConfig);
    if (jsWindowSceneConfigObj == nullptr) {
        WLOGFE("jsWindowSceneConfigObj is nullptr");
        napi_throw(env, CreateJsError(env,
            static_cast<int32_t>(WSErrorCode::WS_ERROR_STATE_ABNORMALLY), "System is abnormal"));
    }
    return jsWindowSceneConfigObj;
}

napi_value JsSceneSessionManager::OnGetSystemConfig(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::DEFAULT, "in");
    const SystemSessionConfig& systemConfig = SceneSessionManager::GetInstance().GetSystemSessionConfig();
    napi_value jsSystemConfigObj = JsWindowSceneConfig::CreateSystemConfig(env, systemConfig);
    if (jsSystemConfigObj == nullptr) {
        TLOGE(WmsLogTag::DEFAULT, "jsSystemConfigObj is nullptr");
        napi_throw(env, CreateJsError(env,
            static_cast<int32_t>(WSErrorCode::WS_ERROR_STATE_ABNORMALLY), "System is abnormal"));
    }
    return jsSystemConfigObj;
}

napi_value JsSceneSessionManager::OnUpdateRotateAnimationConfig(napi_env env, napi_callback_info info)
{
    size_t argc = ARGC_FOUR;
    napi_value argv[ARGC_FOUR] = { nullptr };
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc != ARGC_ONE) {
        TLOGE(WmsLogTag::WMS_SCB, "Argc is invalid: %{public}zu", argc);
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    RotateAnimationConfig rotateAnimationConfig;
    if (!ConvertRotateAnimationConfigFromJs(env, argv[0], rotateAnimationConfig)) {
        TLOGE(WmsLogTag::WMS_SCB, "Failed to convert parameter to rotateAnimationConfig.");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is invalid."));
        return NapiGetUndefined(env);
    }
    SceneSessionManager::GetInstance().UpdateRotateAnimationConfig(rotateAnimationConfig);
    return NapiGetUndefined(env);
}

napi_value JsSceneSessionManager::OnSetVmaCacheStatus(napi_env env, napi_callback_info info)
{
    size_t argc = ARGC_FOUR;
    napi_value argv[ARGC_FOUR] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc != ARGC_ONE) {
        WLOGFE("Argc is invalid: %{public}zu", argc);
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
        "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    bool flag = false;
    if (!ConvertFromJsValue(env, argv[0], flag)) {
        WLOGFE("Faile to convert parameter to flag.");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is invalid."));
        return NapiGetUndefined(env);
    }
    SceneSessionManager::GetInstance().SetVmaCacheStatus(flag);
    return NapiGetUndefined(env);
}

napi_value JsSceneSessionManager::OnInitWithRenderServiceAdded(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::DEFAULT, "in");
    SceneSessionManager::GetInstance().InitWithRenderServiceAdded();
    return NapiGetUndefined(env);
}

static napi_value CreateJsWindowVisibilityInfo(napi_env env,
    const std::pair<int32_t, uint32_t>& visibilityInfo)
{
    napi_value objValue = nullptr;
    napi_create_object(env, &objValue);
    if (objValue == nullptr) {
        TLOGE(WmsLogTag::WMS_SCB, "failed to create napi object");
        return NapiGetUndefined(env);
    }
    napi_set_named_property(env, objValue, "windowId",
        CreateJsValue(env, static_cast<int32_t>(visibilityInfo.first)));
    napi_set_named_property(env, objValue, "visibility",
        CreateJsValue(env, static_cast<int32_t>(visibilityInfo.second)));
    return objValue;
}

static napi_value CreateJsWindowVisibilityInfoArray(napi_env env,
    const std::vector<std::pair<int32_t, uint32_t>>& visibilityInfos)
{
    napi_value arrayValue = nullptr;
    napi_create_array_with_length(env, visibilityInfos.size(), &arrayValue);
    if (arrayValue == nullptr) {
        TLOGE(WmsLogTag::WMS_SCB, "failed to create napi array");
        return NapiGetUndefined(env);
    }
    auto index = 0;
    for (const auto& visibilityInfo : visibilityInfos) {
        napi_set_element(env, arrayValue, index++, CreateJsWindowVisibilityInfo(env, visibilityInfo));
    }
    return arrayValue;
}

napi_value JsSceneSessionManager::OnGetAllWindowVisibilityInfos(napi_env env, napi_callback_info info)
{
    auto windowVisibilityInfos = std::make_shared<std::vector<std::pair<int32_t, uint32_t>>>();

    auto execute = [infos = windowVisibilityInfos]() {
        SceneSessionManager::GetInstance().GetAllWindowVisibilityInfos(*infos);
    };
    auto complete = [infos = windowVisibilityInfos]
        (napi_env env, NapiAsyncTask& task, int32_t status) {
        task.ResolveWithCustomize(env, CreateJsValue(env, static_cast<int32_t>(WSErrorCode::WS_OK)),
            CreateJsWindowVisibilityInfoArray(env, *infos));
    };

    napi_value result = nullptr;
    napi_value callback = nullptr;
    NapiAsyncTask::Schedule("JsSceneSessionManager::OnGetAllWindowVisibilityInfos",
        env, CreateAsyncTaskWithLastParam(env, callback, std::move(execute), std::move(complete), &result));
    return result;
}

napi_value JsSceneSessionManager::OnSetSystemAnimatedScenes(napi_env env, napi_callback_info info)
{
    size_t argc = 4;
    napi_value argv[4] = { nullptr };
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc < 1) {
        WLOGFE("Argc is invalid: %{public}zu", argc);
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
        "Input parameter is missing."));
        return NapiGetUndefined(env);
    }
    uint32_t sceneCode;
    if (!ConvertFromJsValue(env, argv[0], sceneCode)) {
        WLOGFE("Faile to convert parameter to sceneCode.");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is invalid."));
        return NapiGetUndefined(env);
    }
    bool isRegularAnimation = false;
    if (argc >= ARGC_TWO && !ConvertFromJsValue(env, argv[1], isRegularAnimation)) {
        TLOGE(WmsLogTag::WMS_SCB, "Failed to convert parameter to regular animation.");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is invalid."));
        return NapiGetUndefined(env);
    }

    SystemAnimatedSceneType sceneType = static_cast<SystemAnimatedSceneType>(sceneCode);
    WMError ret = SceneSessionManager::GetInstance().SetSystemAnimatedScenes(sceneType, isRegularAnimation);
    if (ret != WMError::WM_OK) {
        WmErrorCode wmErrorCode = WM_JS_TO_ERROR_CODE_MAP.at(ret);
        WLOGFE("Failed, return %{public}d", wmErrorCode);
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(wmErrorCode),
            "Set system animated scene failed."));
        return NapiGetUndefined(env);
    }
    WLOGFI("Success");
    return NapiGetUndefined(env);
}

napi_value JsSceneSessionManager::OnPrepareTerminate(napi_env env, napi_callback_info info)
{
    TLOGI(WmsLogTag::WMS_LIFE, "in");
    size_t argc = 4;
    napi_value argv[4] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc < 1) {
        WLOGFE("Argc is invalid: %{public}zu", argc);
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    int32_t persistentId;
    if (!ConvertFromJsValue(env, argv[0], persistentId)) {
        WLOGFE("Failed to convert parameter to persistentId");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    bool isPrepareTerminate = false;
    SceneSessionManager::GetInstance().PrepareTerminate(persistentId, isPrepareTerminate);
    napi_value result = nullptr;
    napi_get_boolean(env, isPrepareTerminate, &result);
    return result;
}

napi_value JsSceneSessionManager::OnAsyncPrepareTerminate(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::WMS_LIFE, "in");
    size_t argc = ARGC_FOUR;
    napi_value argv[ARGC_FOUR] = { nullptr };
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc != ARGC_TWO) {
        TLOGE(WmsLogTag::WMS_LIFE, "Argc is invalid: %{public}zu", argc);
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    int32_t persistentId = 0;
    if (!ConvertFromJsValue(env, argv[ARG_INDEX_ZERO], persistentId)) {
        TLOGE(WmsLogTag::WMS_LIFE, "Failed to convert parameter to persistentId");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    auto isPrepareTerminate = std::make_shared<bool>(false);
    auto execute = [persistentId, isPrepareTerminate] {
        SceneSessionManager::GetInstance().PrepareTerminate(persistentId, *isPrepareTerminate);
    };
    auto complete = [isPrepareTerminate](napi_env env, NapiAsyncTask& task, int32_t status) {
        task.ResolveWithCustomize(env, CreateJsValue(env, static_cast<int32_t>(WSErrorCode::WS_OK)),
            CreateJsValue(env, *isPrepareTerminate));
    };
    napi_value result = nullptr;
    napi_value callback = argv[ARG_INDEX_ONE];
    NapiAsyncTask::Schedule("JsSceneSessionManager::OnAsyncPrepareTerminate", env,
        CreateAsyncTaskWithLastParam(env, callback, std::move(execute), std::move(complete), &result));
    return result;
}

napi_value JsSceneSessionManager::OnPerfRequestEx(napi_env env, napi_callback_info info)
{
    WLOGFD("in");
#ifdef SOC_PERF_ENABLE
    size_t argc = 4;
    napi_value argv[4] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc < ARGC_TWO) {
        WLOGFE("Argc is invalid: %{public}zu", argc);
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    int32_t cmdId;
    bool onOffTag = false;
    if (!ConvertFromJsValue(env, argv[0], cmdId) || !ConvertFromJsValue(env, argv[1], onOffTag)) {
        WLOGFE("Failed to convert parameter to cmdId or onOffTag");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    std::string msg;
    if (argc == ARGC_THREE) {
        if (!ConvertFromJsValue(env, argv[ARGC_TWO], msg)) {
            WLOGFE("Failed to convert parameter to cmd msg");
            napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
                "Input parameter is missing or invalid"));
            return NapiGetUndefined(env);
        }
    }
    OHOS::SOCPERF::SocPerfClient::GetInstance().PerfRequestEx(cmdId, onOffTag, msg);
    WLOGFD("Success cmdId: %{public}d onOffTag: %{public}u msg:%{public}s",
        cmdId, static_cast<uint32_t>(onOffTag), msg.c_str());
#endif
    return NapiGetUndefined(env);
}

napi_value JsSceneSessionManager::OnUpdateWindowMode(napi_env env, napi_callback_info info)
{
    size_t argc = 4;
    napi_value argv[4] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc < ARGC_TWO) {
        WLOGFE("Argc is invalid: %{public}zu", argc);
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    int32_t persistentId;
    if (!ConvertFromJsValue(env, argv[0], persistentId)) {
        WLOGFE("Failed to convert parameter to persistentId");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    int32_t windowMode;
    if (!ConvertFromJsValue(env, argv[1], windowMode)) {
        WLOGFE("Failed to convert parameter to windowMode");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    SceneSessionManager::GetInstance().UpdateWindowMode(persistentId, windowMode);
    return NapiGetUndefined(env);
}

napi_value JsSceneSessionManager::OnNotifySingleHandInfoChange(napi_env env, napi_callback_info info)
{
    size_t argc = DEFAULT_ARG_COUNT;
    napi_value argv[DEFAULT_ARG_COUNT] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc != ARGC_THREE) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "Argc is invalid: %{public}zu", argc);
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    SingleHandScreenInfo singleHandScreenInfo;
    if (!ConvertSingleHandScreenInfoFromJs(env, argv[ARG_INDEX_ZERO], singleHandScreenInfo)) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "Failed to convert parameter to singleHandScreenInfo");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    WSRect originRect;
    if (!ConvertSessionRectInfoFromJs(env, argv[ARG_INDEX_ONE], originRect)) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "Failed to convert parameter to originRect");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    WSRect singleHandRect;
    if (!ConvertSessionRectInfoFromJs(env, argv[ARG_INDEX_TWO], singleHandRect)) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "Failed to convert parameter to singleHandRect");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    SceneSessionManager::GetInstance().NotifySingleHandInfoChange(singleHandScreenInfo, originRect, singleHandRect);
    return NapiGetUndefined(env);
}

napi_value JsSceneSessionManager::OnGetSingleHandCompatibleModeConfig(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::WMS_LAYOUT, "in");
    const auto& singleHandCompatibleModeConfig =
        SceneSessionManager::GetInstance().GetSingleHandCompatibleModeConfig();
    napi_value jsSingleHandCompatibleModeConfigObj =
        JsWindowSceneConfig::CreateSingleHandCompatibleConfig(env, singleHandCompatibleModeConfig);
    if (jsSingleHandCompatibleModeConfigObj == nullptr) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "jsSingleHandCompatibleModeConfigObj is nullptr");
        napi_throw(env, CreateJsError(env,
            static_cast<int32_t>(WSErrorCode::WS_ERROR_STATE_ABNORMALLY), "System is abnormal"));
        return NapiGetUndefined(env);
    }
    return jsSingleHandCompatibleModeConfigObj;
}

napi_value JsSceneSessionManager::OnAddWindowDragHotArea(napi_env env, napi_callback_info info)
{
    size_t argc = 4;
    napi_value argv[4] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc < ARGC_THREE) {
        WLOGFE("Argc is invalid: %{public}zu", argc);
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    int64_t displayId;
    if (!ConvertFromJsValue(env, argv[0], displayId)) {
        WLOGFE("Failed to convert parameter to displayId");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    uint32_t type;
    if (!ConvertFromJsValue(env, argv[ARG_INDEX_ONE], type)) {
        WLOGFE("Failed to convert parameter to type");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    WSRect area;
    if (argv[ARG_INDEX_TWO] == nullptr || !ConvertRectInfoFromJs(env, argv[ARG_INDEX_TWO], area)) {
        WLOGFE("Failed to convert parameter to area");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }

    SceneSessionManager::GetInstance().AddWindowDragHotArea(displayId, type, area);
    return NapiGetUndefined(env);
}

napi_value JsSceneSessionManager::OnGetRootSceneUIContext(napi_env env, napi_callback_info info)
{
    size_t argc = 4;
    napi_value argv[4] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc >= 1) {
        WLOGFE("Argc is invalid: %{public}zu, expect zero params", argc);
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WmErrorCode::WM_ERROR_INVALID_PARAM)));
        return NapiGetUndefined(env);
    }

    if (RootScene::staticRootScene_ == nullptr) {
        WLOGFE("Root scene is nullptr");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WmErrorCode::WM_ERROR_STATE_ABNORMALLY)));
        return NapiGetUndefined(env);
    }

    const auto& uiContent = RootScene::staticRootScene_->GetUIContent();
    if (uiContent == nullptr) {
        WLOGFE("uiContent is nullptr");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WmErrorCode::WM_ERROR_STATE_ABNORMALLY)));
        return NapiGetUndefined(env);
    }
    napi_value uiContext = uiContent->GetUINapiContext();
    if (uiContext == nullptr) {
        WLOGFE("uiContext obtained from jsEngine is nullptr");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WmErrorCode::WM_ERROR_STATE_ABNORMALLY)));
        return NapiGetUndefined(env);
    }
    WLOGFD("success");
    return uiContext;
}

napi_value JsSceneSessionManager::OnSendTouchEvent(napi_env env, napi_callback_info info)
{
    size_t argc = 4;
    napi_value argv[4] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc < ARGC_TWO || argc > ARGC_THREE) {
        TLOGE(WmsLogTag::WMS_EVENT, "Argc is invalid: %{public}zu", argc);
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    napi_value nativeObj = argv[0];
    if (nativeObj == nullptr) {
        TLOGE(WmsLogTag::WMS_EVENT, "Failed to convert object");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    auto pointerEvent = MMI::PointerEvent::Create();
    if (pointerEvent == nullptr) {
        TLOGE(WmsLogTag::WMS_EVENT, "Failed to create pointer event");
        return NapiGetUndefined(env);
    }
    if (!ConvertPointerEventFromJs(env, nativeObj, *pointerEvent)) {
        TLOGE(WmsLogTag::WMS_EVENT, "Failed to convert pointer event");
        return NapiGetUndefined(env);
    }
    uint32_t zIndex;
    if (!ConvertFromJsValue(env, argv[1], zIndex)) {
        TLOGE(WmsLogTag::WMS_EVENT, "Failed to convert parameter to zIndex");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    uint32_t identity = static_cast<uint32_t>(SendTouchAction::ACTION_NORMAL);
    if (argc == ARGC_THREE && !ConvertFromJsValue(env, argv[ARGC_TWO], identity)) {
        TLOGE(WmsLogTag::WMS_EVENT, "Failed to convert parameter to identity");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    if (identity == static_cast<uint32_t>(SendTouchAction::ACTION_NOT_RECEIVE_PULL_CANCEL)) {
        pointerEvent->AddFlag(MMI::PointerEvent::EVENT_FLAG_SIMULATE_LEFT_RIGHT_ANTI_AXIS_MOVE);
    }
    SceneSessionManager::GetInstance().SendTouchEvent(pointerEvent, zIndex);
    return NapiGetUndefined(env);
}

napi_value JsSceneSessionManager::OnPreloadInLakeApp(napi_env env, napi_callback_info info)
{
    size_t argc = 4;
    napi_value argv[4] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc != ARGC_ONE) {
        WLOGFE("Argc is invalid: %{public}zu", argc);
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    std::string bundleName;
    if (!ConvertFromJsValue(env, argv[0], bundleName)) {
        WLOGFE("Failed to convert parameter to bundleName");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    auto localScheduler = SceneSessionManager::GetInstance().GetTaskScheduler();
    auto preloadTask = [bundleName = std::move(bundleName)] {
        SceneSessionManager::GetInstance().PreloadInLakeApp(bundleName);
    };
    localScheduler->PostAsyncTask(preloadTask, __func__);
    return NapiGetUndefined(env);
}

napi_value JsSceneSessionManager::OnRequestFocusStatus(napi_env env, napi_callback_info info)
{
    size_t argc = 5;
    napi_value argv[5] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc < MIN_ARG_COUNT) {
        TLOGE(WmsLogTag::WMS_FOCUS, "Argc is invalid: %{public}zu", argc);
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    int32_t persistentId;
    if (!ConvertFromJsValue(env, argv[0], persistentId)) {
        TLOGE(WmsLogTag::WMS_FOCUS, "Failed to convert parameter to persistentId");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    bool isFocused = false;
    if (!ConvertFromJsValue(env, argv[ARG_INDEX_ONE], isFocused)) {
        TLOGE(WmsLogTag::WMS_FOCUS, "Failed to convert parameter to isFocused");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    bool byForeground = false;
    if (!ConvertFromJsValue(env, argv[ARG_INDEX_TWO], byForeground)) {
        TLOGE(WmsLogTag::WMS_FOCUS, "Failed to convert parameter to byForeground");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    FocusChangeReason reason = FocusChangeReason::DEFAULT;
    if (argc > MIN_ARG_COUNT && !ConvertFromJsValue(env, argv[ARG_INDEX_THREE], reason)) {
        TLOGE(WmsLogTag::WMS_FOCUS, "Failed to convert parameter to reason");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    int64_t displayIdValue = -1;
    if (argc > DEFAULT_ARG_COUNT && !ConvertFromJsValue(env, argv[ARG_INDEX_FOUR], displayIdValue)) {
        TLOGE(WmsLogTag::WMS_FOCUS, "Failed to convert parameter to displayId");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    DisplayId displayId = displayIdValue < 0 ? DISPLAY_ID_INVALID : static_cast<DisplayId>(displayIdValue);
    DoRequestFocusStatus(persistentId, isFocused, byForeground, reason, displayId);
    return NapiGetUndefined(env);
}

void JsSceneSessionManager::DoRequestFocusStatus(int32_t persistentId, bool isFocused, bool byForeground,
    FocusChangeReason reason, DisplayId displayId)
{
    TLOGD(WmsLogTag::WMS_FOCUS, "Id: %{public}d, isFocused: %{public}d, byForeground: %{public}d, "
        "reason: %{public}d, displayId: %{public}" PRIu64, persistentId, isFocused, byForeground, reason, displayId);
    if (Session::IsScbCoreEnabled()) {
        SceneSessionManager::GetInstance().RequestFocusStatusBySCB(persistentId, isFocused, byForeground, reason,
            displayId);
    } else {
        SceneSessionManager::GetInstance().RequestFocusStatus(persistentId, isFocused, byForeground, reason);
    }
}

napi_value JsSceneSessionManager::OnRequestAllAppSessionUnfocus(napi_env env, napi_callback_info info)
{
    size_t argc = 4;
    napi_value argv[4] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc >= ARGC_ONE) {
        WLOGFE("Argc is invalid: %{public}zu", argc);
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    SceneSessionManager::GetInstance().RequestAllAppSessionUnfocus();
    return NapiGetUndefined(env);
}

napi_value JsSceneSessionManager::OnSetScreenLocked(napi_env env, napi_callback_info info)
{
    size_t argc = 4;
    napi_value argv[4] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc != ARGC_ONE) {
        WLOGFE("Argc is invalid: %{public}zu", argc);
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    bool isScreenLocked = false;
    if (!ConvertFromJsValue(env, argv[0], isScreenLocked)) {
        WLOGFE("Failed to convert parameter to isScreenLocked");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    SceneSessionManager::GetInstance().SetScreenLocked(isScreenLocked);
    return NapiGetUndefined(env);
}

napi_value JsSceneSessionManager::OnSetUserAuthPassed(napi_env env, napi_callback_info info)
{
    size_t argc = ARGC_FOUR;
    napi_value argv[ARGC_FOUR] = { nullptr };
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc != ARGC_ONE) {
        TLOGE(WmsLogTag::WMS_LIFE, "Argc is invalid: %{public}zu", argc);
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    bool isUserAuthPassed = false;
    if (!ConvertFromJsValue(env, argv[0], isUserAuthPassed)) {
        TLOGE(WmsLogTag::WMS_LIFE, "Failed to convert parameter to isUserAuthPassed");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    SceneSessionManager::GetInstance().SetUserAuthPassed(isUserAuthPassed);
    return NapiGetUndefined(env);
}

napi_value JsSceneSessionManager::OnUpdateMaximizeMode(napi_env env, napi_callback_info info)
{
    size_t argc = 4;
    napi_value argv[4] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc < ARGC_TWO) {
        WLOGFE("Argc is invalid: %{public}zu", argc);
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    int32_t persistentId;
    if (!ConvertFromJsValue(env, argv[0], persistentId)) {
        WLOGFE("Failed to convert parameter to persistentId");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    bool isMaximize = false;
    if (!ConvertFromJsValue(env, argv[1], isMaximize)) {
        WLOGFE("Failed to convert parameter to isMaximize");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    SceneSessionManager::GetInstance().UpdateMaximizeMode(persistentId, isMaximize);
    return NapiGetUndefined(env);
}

napi_value JsSceneSessionManager::OnUpdateSessionDisplayId(napi_env env, napi_callback_info info)
{
    size_t argc = 6;
    napi_value argv[6] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc < ARGC_TWO) {
        WLOGFE("Argc is invalid: %{public}zu", argc);
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    int32_t persistentId;
    if (!ConvertFromJsValue(env, argv[0], persistentId)) {
        WLOGFE("Failed to convert parameter to persistentId");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    int32_t screenId;
    if (!ConvertFromJsValue(env, argv[1], screenId)) {
        WLOGFE("Failed to convert parameter to screenId");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    SceneSessionManager::GetInstance().UpdateSessionDisplayId(persistentId, screenId);
    return NapiGetUndefined(env);
}

napi_value JsSceneSessionManager::OnNotifyStackEmpty(napi_env env, napi_callback_info info)
{
    size_t argc = ARGC_FOUR;
    napi_value argv[ARGC_FOUR] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc != ARGC_ONE) {
        TLOGE(WmsLogTag::WMS_LIFE, "Argc is invalid: %{public}zu", argc);
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    int32_t persistentId ;
    if (!ConvertFromJsValue(env, argv[0], persistentId)) {
        TLOGE(WmsLogTag::WMS_LIFE, "Failed to convert parameter to displayId");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    WSErrorCode ret =
        WS_JS_TO_ERROR_CODE_MAP.at(SceneSessionManager::GetInstance().NotifyStackEmpty(persistentId));
    if (ret != WSErrorCode::WS_OK) {
        TLOGE(WmsLogTag::WMS_LIFE, "Notify stack empty failed");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_STATE_ABNORMALLY),
            "System is abnormal"));
    }
    return NapiGetUndefined(env);
}

napi_value JsSceneSessionManager::OnHandleUserSwitch(napi_env env, napi_callback_info info)
{
    size_t argc = DEFAULT_ARG_COUNT;
    napi_value argv[DEFAULT_ARG_COUNT] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc != ARGC_TWO) {
        TLOGE(WmsLogTag::WMS_MULTI_USER, "Argc is invalid: %{public}zu", argc);
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
                                      "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    uint32_t eventTypeValue = 0;
    if (!ConvertFromJsValue(env, argv[ARG_INDEX_ZERO], eventTypeValue)) {
        TLOGE(WmsLogTag::WMS_MULTI_USER, "Failed to convert parameter to eventTypeValue");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
                                      "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    UserSwitchEventType eventType = static_cast<UserSwitchEventType>(eventTypeValue);
    bool isUserActive = true;
    if (!ConvertFromJsValue(env, argv[ARG_INDEX_ONE], isUserActive)) {
        TLOGE(WmsLogTag::WMS_MULTI_USER, "Failed to convert parameter to isUserActive");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
                                      "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    SceneSessionManager::GetInstance().HandleUserSwitch(eventType, isUserActive);
    return NapiGetUndefined(env);
}

napi_value JsSceneSessionManager::OnSetStatusBarDefaultVisibilityPerDisplay(napi_env env, napi_callback_info info)
{
    size_t argc = ARGC_FOUR;
    napi_value argv[ARGC_FOUR] = { nullptr };
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc < ARGC_TWO) {
        TLOGE(WmsLogTag::WMS_IMMS, "Argc is invalid: %{public}zu", argc);
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    int64_t displayId = -1;
    if (!ConvertFromJsValue(env, argv[0], displayId)) {
        TLOGE(WmsLogTag::WMS_IMMS, "Failed to convert parameter to displayId");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    if (displayId < 0) {
        TLOGE(WmsLogTag::WMS_IMMS, "Failed to convert parameter to displayId");
        return NapiGetUndefined(env);
    }
    bool visible = false;
    if (!ConvertFromJsValue(env, argv[1], visible)) {
        TLOGE(WmsLogTag::WMS_IMMS, "Failed to convert parameter to visible");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    SceneSessionManager::GetInstance().SetStatusBarDefaultVisibilityPerDisplay(
        static_cast<DisplayId>(displayId), visible);
    return NapiGetUndefined(env);
}

napi_value JsSceneSessionManager::OnNotifyStatusBarShowStatus(napi_env env, napi_callback_info info)
{
    size_t argc = ARGC_FOUR;
    napi_value argv[ARGC_FOUR] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc < ARGC_TWO) {
        TLOGE(WmsLogTag::WMS_IMMS, "Argc is invalid: %{public}zu", argc);
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    int32_t persistentId = 0;
    if (!ConvertFromJsValue(env, argv[0], persistentId)) {
        TLOGE(WmsLogTag::WMS_IMMS, "Failed to convert parameter to persistentId");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    if (persistentId <= 0) {
        TLOGE(WmsLogTag::WMS_IMMS, "Failed to convert parameter to persistentId");
        return NapiGetUndefined(env);
    }
    bool isVisible = false;
    if (!ConvertFromJsValue(env, argv[1], isVisible)) {
        TLOGE(WmsLogTag::WMS_IMMS, "Failed to convert parameter to isVisible");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    SceneSessionManager::GetInstance().NotifyStatusBarShowStatus(persistentId, isVisible);
    return NapiGetUndefined(env);
}

napi_value JsSceneSessionManager::OnNotifyLSStateChange(napi_env env, napi_callback_info info)
{
    size_t argc = ARGC_FOUR;
    napi_value argv[ARGC_FOUR] = { nullptr };
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc < ARGC_TWO) {
        TLOGE(WmsLogTag::WMS_IMMS, "Argc is invalid: %{public}zu", argc);
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    int32_t curState = 0;
    if (!ConvertFromJsValue(env, argv[ARG_INDEX_ZERO], curState)) {
        TLOGE(WmsLogTag::WMS_IMMS, "Failed to convert parameter to curState");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    int32_t preState = 0;
    if (!ConvertFromJsValue(env, argv[ARG_INDEX_ONE], preState)) {
        TLOGE(WmsLogTag::WMS_IMMS, "Failed to convert parameter to preState");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    SceneSessionManager::GetInstance().UpdateAvoidAreaForLSStateChange(curState, preState);
    return NapiGetUndefined(env);
}

napi_value JsSceneSessionManager::OnNotifyStatusBarConstantlyShowStatus(napi_env env, napi_callback_info info)
{
    size_t argc = ARGC_FOUR;
    napi_value argv[ARGC_FOUR] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc < ARGC_TWO) {
        TLOGE(WmsLogTag::WMS_IMMS, "Argc is invalid: %{public}zu", argc);
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    int64_t screenId = -1;
    if (!ConvertFromJsValue(env, argv[0], screenId)) {
        TLOGE(WmsLogTag::WMS_IMMS, "Failed to convert parameter to screenId");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    bool isVisible = false;
    if (!ConvertFromJsValue(env, argv[1], isVisible)) {
        TLOGE(WmsLogTag::WMS_IMMS, "Failed to convert parameter to isVisible");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    SceneSessionManager::GetInstance().NotifyStatusBarConstantlyShow(static_cast<uint64_t>(screenId), isVisible);
    return NapiGetUndefined(env);
}

napi_value JsSceneSessionManager::OnNotifyAINavigationBarShowStatus(napi_env env, napi_callback_info info)
{
    size_t argc = 4;
    napi_value argv[4] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc < ARGC_THREE) {
        WLOGFE("Argc is invalid: %{public}zu", argc);
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    bool isVisible = false;
    if (!ConvertFromJsValue(env, argv[0], isVisible)) {
        WLOGFE("Failed to convert parameter to isVisible");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    WSRect barArea;
    if (argv[1] == nullptr || !ConvertRectInfoFromJs(env, argv[1], barArea)) {
        WLOGFE("Failed to convert parameter to barArea");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    int64_t displayId = -1;
    if (!ConvertFromJsValue(env, argv[ARG_INDEX_TWO], displayId)) {
        TLOGE(WmsLogTag::WMS_LIFE, "Failed to convert parameter to displayId");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    SceneSessionManager::GetInstance().NotifyAINavigationBarShowStatus(
        isVisible, barArea, static_cast<uint64_t>(displayId));
    return NapiGetUndefined(env);
}

napi_value JsSceneSessionManager::OnNotifyNextAvoidRectInfo(napi_env env, napi_callback_info info)
{
    size_t argc = DEFAULT_ARG_COUNT;
    napi_value argv[ARG_INDEX_FOUR] = { nullptr };
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc < ARGC_THREE) {
        TLOGE(WmsLogTag::WMS_IMMS, "Argc is invalid: %{public}zu", argc);
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    int32_t type = 0;
    if (!ConvertFromJsValue(env, argv[ARG_INDEX_ZERO], type) ||
        JS_SESSION_TO_WINDOW_TYPE_MAP.find(static_cast<JsSessionType>(type)) == JS_SESSION_TO_WINDOW_TYPE_MAP.end()) {
        TLOGE(WmsLogTag::WMS_IMMS, "Failed to convert parameter to type");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    auto winType = JS_SESSION_TO_WINDOW_TYPE_MAP.at(static_cast<JsSessionType>(type));
    if (winType != WindowType::WINDOW_TYPE_STATUS_BAR && winType != WindowType::WINDOW_TYPE_NAVIGATION_INDICATOR) {
        TLOGE(WmsLogTag::WMS_IMMS, "Input parameter is missing or invalid");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    auto avoidType = (winType == WindowType::WINDOW_TYPE_STATUS_BAR) ? AvoidAreaType::TYPE_SYSTEM :
                                                                       AvoidAreaType::TYPE_NAVIGATION_INDICATOR;
    int64_t displayId = -1;
    if (!ConvertFromJsValue(env, argv[ARG_INDEX_ONE], displayId)) {
        TLOGE(WmsLogTag::WMS_IMMS, "Failed to convert parameter to displayId");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    WSRect portraitRect;
    if (argv[ARG_INDEX_TWO] == nullptr || !ConvertRectInfoFromJs(env, argv[ARG_INDEX_TWO], portraitRect)) {
        TLOGE(WmsLogTag::WMS_IMMS, "Failed to convert parameter to landspaceRect");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    WSRect landspaceRect;
    if (argv[ARG_INDEX_THREE] == nullptr || !ConvertRectInfoFromJs(env, argv[ARG_INDEX_THREE], landspaceRect)) {
        TLOGE(WmsLogTag::WMS_IMMS, "Failed to convert parameter to landspaceRect");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    SceneSessionManager::GetInstance().NotifyNextAvoidRectInfo(avoidType, portraitRect,
        landspaceRect, static_cast<uint64_t>(displayId));
    return NapiGetUndefined(env);
}

napi_value JsSceneSessionManager::OnNotifySessionRecoverStatus(napi_env env, napi_callback_info info)
{
    size_t argc = 4;
    napi_value argv[4] = { nullptr };
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc != ARGC_TWO) {
        WLOGFE("Argc is invalid: %{public}zu", argc);
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    bool isRecovering = false;
    if (!ConvertFromJsValue(env, argv[0], isRecovering)) {
        WLOGFE("Failed to convert parameter to isRecovering");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    WLOGFD("IsRecovering: %{public}u", isRecovering);
    // Recovered sessions persistentId list as second argument
    std::vector<int32_t> recoveredPersistentIds;
    if (!ConvertInt32ArrayFromJs(env, argv[1], recoveredPersistentIds)) {
        WLOGFE("Failed to convert recovered persistentId array");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    if (!isRecovering) {
        // Sceneboard recover finished
        SceneSessionManager::GetInstance().NotifyRecoveringFinished();
        SceneSessionManager::GetInstance().UpdateRecoveredSessionInfo(recoveredPersistentIds);
    } else {
        SceneSessionManager::GetInstance().SetAlivePersistentIds(recoveredPersistentIds);
    }
    SceneSessionManager::GetInstance().SetEnableInputEvent(!isRecovering);

    return NapiGetUndefined(env);
}

napi_value JsSceneSessionManager::OnUpdateTitleInTargetPos(napi_env env, napi_callback_info info)
{
    size_t argc = 4;
    napi_value argv[4] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc < 3) {
        WLOGFE("Argc is invalid: %{public}zu", argc);
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    int32_t persistentId;
    if (!ConvertFromJsValue(env, argv[0], persistentId)) {
        WLOGFE("Failed to convert parameter to persistentId");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    bool isShow = false;
    if (!ConvertFromJsValue(env, argv[1], isShow)) {
        WLOGFE("Failed to convert parameter to isShow");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    int32_t height;
    if (!ConvertFromJsValue(env, argv[2], height)) {
        WLOGFE("Failed to convert parameter to height");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    SceneSessionManager::GetInstance().UpdateTitleInTargetPos(persistentId, isShow, height);
    return NapiGetUndefined(env);
}

napi_value JsSceneSessionManager::OnReportData(napi_env env, napi_callback_info info)
{
    size_t argc = 4;
    napi_value argv[4] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc < ARG_INDEX_THREE) { // ReportData args must be greater than three
        WLOGFE("Argc is invalid: %{public}zu", argc);
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    int32_t resType;
    if (!ConvertFromJsValue(env, argv[0], resType)) {
        WLOGFE("Failed to convert parameter to resType");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    int32_t value;
    if (!ConvertFromJsValue(env, argv[ARG_INDEX_ONE], value)) { // second args is int value
        WLOGFE("Failed to convert parameter to value");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }

    std::unordered_map<std::string, std::string> mapPayload;
    if (!ConvertStringMapFromJs(env, argv[ARG_INDEX_TWO], mapPayload)) {
        WLOGFE("Failed to convert parameter to pauloadPid");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    mapPayload["srcPid"] = std::to_string(getprocpid());
    if (resType == RESTYPE_RECLAIM) {
        std::string reclaimTag = mapPayload[RES_PARAM_RECLAIM_TAG];
        WLOGFI("handle reclaim type, reclaimTag=%{public}s", reclaimTag.c_str());
        if (reclaimTag == "true") {
            auto retId = SceneSessionManager::GetInstance().ReclaimPurgeableCleanMem();
            WLOGFI("ReclaimPurgeableCleanMem finished, retId:%{public}d", retId);
            return NapiGetUndefined(env);
        }
    }
#ifdef RESOURCE_SCHEDULE_SERVICE_ENABLE
    OHOS::ResourceSchedule::ResSchedClient::GetInstance().ReportData(resType, value, mapPayload);
#endif
    return NapiGetUndefined(env);
}

napi_value JsSceneSessionManager::OnGetRssData(napi_env env, napi_callback_info info)
{
#ifdef RESOURCE_SCHEDULE_SERVICE_ENABLE
    size_t argc = DEFAULT_ARG_COUNT;
    napi_value argv[DEFAULT_ARG_COUNT] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc < ARG_INDEX_TWO) { // OnGetRssData args must be greater than two
        WLOGFE("Argc is invalid: %{public}zu", argc);
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    int32_t resType;
    if (!ConvertFromJsValue(env, argv[0], resType)) { // first args is int value
        WLOGFE("Failed to convert parameter to resType");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    nlohmann::json payload;
    if (!ConvertJsonFromJs(env, argv[ARG_INDEX_ONE], payload)) {
        WLOGFE("Failed to convert parameter to payload");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    static std::string pid = std::to_string(getprocpid());
    payload["srcPid"] = pid;
    nlohmann::json reply;
    ResourceSchedule::ResSchedClient::GetInstance().ReportSyncEvent(resType, 0, payload, reply);
    return RssSession::DealRssReply(env, payload, reply);
#endif
    return NapiGetUndefined(env);
}

napi_value JsSceneSessionManager::OnRegisterRssData(napi_env env, napi_callback_info info)
{
#ifdef RESOURCE_SCHEDULE_SERVICE_ENABLE
    return RssSession::RegisterRssData(env, info);
#endif
    return NapiGetUndefined(env);
}

napi_value JsSceneSessionManager::OnUnregisterRssData(napi_env env, napi_callback_info info)
{
#ifdef RESOURCE_SCHEDULE_SERVICE_ENABLE
    return RssSession::UnregisterRssData(env, info);
#endif
    return NapiGetUndefined(env);
}

napi_value JsSceneSessionManager::SupportSnapshotAllSessionStatus(napi_env env, napi_callback_info info)
{
    TLOGI(WmsLogTag::WMS_PATTERN, "[NAPI]");
    JsSceneSessionManager* me = CheckParamsAndGetThis<JsSceneSessionManager>(env, info);
    return (me != nullptr) ? me->OnSupportSnapshotAllSessionStatus(env, info) : nullptr;
}

napi_value JsSceneSessionManager::OnSupportSnapshotAllSessionStatus(napi_env env, napi_callback_info info)
{
    SceneSessionManager::GetInstance().ConfigSupportSnapshotAllSessionStatus();
    return NapiGetUndefined(env);
}

napi_value JsSceneSessionManager::SupportCacheLockedSessionSnapshot(napi_env env, napi_callback_info info)
{
    TLOGI(WmsLogTag::WMS_PATTERN, "[NAPI]");
    JsSceneSessionManager* me = CheckParamsAndGetThis<JsSceneSessionManager>(env, info);
    return (me != nullptr) ? me->OnSupportCacheLockedSessionSnapshot(env, info) : nullptr;
}

napi_value JsSceneSessionManager::OnSupportCacheLockedSessionSnapshot(napi_env env, napi_callback_info info)
{
    SceneSessionManager::GetInstance().ConfigSupportCacheLockedSessionSnapshot();
    return NapiGetUndefined(env);
}

napi_value JsSceneSessionManager::SupportPreloadStartingWindow(napi_env env, napi_callback_info info)
{
    TLOGI(WmsLogTag::WMS_PATTERN, "[NAPI]");
    JsSceneSessionManager* me = CheckParamsAndGetThis<JsSceneSessionManager>(env, info);
    return (me != nullptr) ? me->OnSupportPreloadStartingWindow(env, info) : nullptr;
}

napi_value JsSceneSessionManager::OnSupportPreloadStartingWindow(napi_env env, napi_callback_info info)
{
    SceneSessionManager::GetInstance().ConfigSupportPreloadStartingWindow();
    return NapiGetUndefined(env);
}

napi_value JsSceneSessionManager::PreloadStartingWindow(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::WMS_LIFE, "[NAPI]");
    JsSceneSessionManager* me = CheckParamsAndGetThis<JsSceneSessionManager>(env, info);
    return (me != nullptr) ? me->OnPreloadStartingWindow(env, info) : nullptr;
}

napi_value JsSceneSessionManager::OnPreloadStartingWindow(napi_env env, napi_callback_info info)
{
    HITRACE_METER_FMT(HITRACE_TAG_WINDOW_MANAGER, "JsSceneSessionManager::OnPreloadStartingWindow");
    WSErrorCode errCode = WSErrorCode::WS_OK;
    size_t argc = 4;
    napi_value argv[4] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc < ARGC_ONE) {
        TLOGE(WmsLogTag::WMS_PATTERN, "Argc is invalid: %{public}zu", argc);
        errCode = WSErrorCode::WS_ERROR_INVALID_PARAM;
    }

    sptr<SceneSession> sceneSession = nullptr;
    if (errCode == WSErrorCode::WS_OK) {
        napi_value jsSceneSessionObj = argv[0];
        if (jsSceneSessionObj == nullptr) {
            TLOGE(WmsLogTag::WMS_PATTERN, "Failed to get js scene session object");
            errCode = WSErrorCode::WS_ERROR_INVALID_PARAM;
        } else {
            void* pointerResult = nullptr;
            napi_unwrap(env, jsSceneSessionObj, &pointerResult);
            auto jsSceneSession = static_cast<JsSceneSession*>(pointerResult);
            if (jsSceneSession == nullptr) {
                TLOGE(WmsLogTag::WMS_PATTERN, "Failed to get scene session from js object");
                errCode = WSErrorCode::WS_ERROR_INVALID_PARAM;
            } else {
                sceneSession = jsSceneSession->GetNativeSession();
            }
        }
    }
    if (sceneSession == nullptr) {
        TLOGE(WmsLogTag::WMS_PATTERN, "sceneSession is nullptr");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_SYSTEM_ABNORMALLY),
            "sceneSession is nullptr"));
        return NapiGetUndefined(env);
    }

    if (errCode == WSErrorCode::WS_ERROR_INVALID_PARAM) {
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }

    SceneSessionManager::GetInstance().PreLoadStartingWindow(sceneSession);
    return NapiGetUndefined(env);
}

napi_value JsSceneSessionManager::SupportCreateFloatWindow(napi_env env, napi_callback_info info)
{
    TLOGI(WmsLogTag::WMS_LIFE, "[NAPI]");
    JsSceneSessionManager* me = CheckParamsAndGetThis<JsSceneSessionManager>(env, info);
    return (me != nullptr) ? me->OnSupportCreateFloatWindow(env, info) : nullptr;
}

napi_value JsSceneSessionManager::OnSupportCreateFloatWindow(napi_env env, napi_callback_info info)
{
    SceneSessionManager::GetInstance().ConfigSupportCreateFloatWindow();
    return NapiGetUndefined(env);
}

napi_value JsSceneSessionManager::OnGetSessionSnapshotPixelMap(napi_env env, napi_callback_info info)
{
    size_t argc = 4;
    napi_value argv[4] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc < ARGC_TWO) {
        WLOGFE("Argc is invalid: %{public}zu", argc);
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    int32_t persistentId;
    if (!ConvertFromJsValue(env, argv[0], persistentId)) {
        WLOGFE("Failed to convert parameter to persistentId");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    double scaleValue;
    if (!ConvertFromJsValue(env, argv[1], scaleValue)) {
        WLOGFE("Failed to convert parameter to scaleValue");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    SnapshotNodeType snapshotNode = SnapshotNodeType::DEFAULT_NODE;
    if (argc > ARGC_TWO && !ConvertFromJsValue(env, argv[ARG_INDEX_TWO], snapshotNode)) {
        TLOGE(WmsLogTag::WMS_PATTERN, "Failed to convert parameter to snapshotNode");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    if (snapshotNode < SnapshotNodeType::DEFAULT_NODE || snapshotNode > SnapshotNodeType::APP_NODE) {
        TLOGE(WmsLogTag::WMS_PATTERN, "SnapshotNodeType invalid:%{public}d", snapshotNode);
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    bool needSnapshot = true;
    if (argc > ARGC_THREE && !ConvertFromJsValue(env, argv[ARG_INDEX_THREE], needSnapshot)) {
        TLOGE(WmsLogTag::WMS_PATTERN, "Failed to convert parameter to needSnapshot");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }

    float scaleParam = GreatOrEqual(scaleValue, 0.0f) && LessOrEqual(scaleValue, 1.0f) ?
        static_cast<float>(scaleValue) : 0.0f;
    std::shared_ptr<std::shared_ptr<Media::PixelMap>> pixelPtr = std::make_shared<std::shared_ptr<Media::PixelMap>>();
    NapiAsyncTask::ExecuteCallback execute = [persistentId, scaleParam, pixelPtr, snapshotNode, needSnapshot]() {
        if (pixelPtr == nullptr) {
            return;
        }
        *pixelPtr = SceneSessionManager::GetInstance().GetSessionSnapshotPixelMap(
            persistentId, scaleParam, snapshotNode, needSnapshot);
    };
    NapiAsyncTask::CompleteCallback complete =
        [persistentId, scaleParam, pixelPtr](napi_env env, NapiAsyncTask& task, int32_t status) {
            if (pixelPtr == nullptr) {
                WLOGE("pixelMap ptr not exist");
                task.Reject(env, CreateJsError(env, static_cast<int32_t>(WmErrorCode::WM_ERROR_STATE_ABNORMALLY)));
                return;
            }
            napi_value nativeData = nullptr;
            if (*pixelPtr) {
                nativeData = Media::PixelMapNapi::CreatePixelMap(env, *pixelPtr);
            }
            if (nativeData) {
                WLOGD("pixelmap W x H=%{public}d x %{public}d", (*pixelPtr)->GetWidth(), (*pixelPtr)->GetHeight());
                task.Resolve(env, nativeData);
            } else {
                WLOGE("Create native pixelmap fail");
                task.Reject(env, CreateJsError(env, static_cast<int32_t>(WmErrorCode::WM_ERROR_STATE_ABNORMALLY)));
            }
        };
    napi_value result = nullptr;
    napi_value lastParam = argv[1];
    NapiAsyncTask::ScheduleHighQos("JsSceneSessionManager::OnGetSessionSnapshotPixelMap",
        env, CreateAsyncTaskWithLastParam(env, lastParam, std::move(execute), std::move(complete), &result));
    return result;
}

napi_value JsSceneSessionManager::OnGetSessionSnapshotPixelMapSync(napi_env env, napi_callback_info info)
{
    size_t argc = ARGC_FOUR;
    napi_value argv[ARGC_FOUR] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc < ARGC_TWO) {
        TLOGE(WmsLogTag::WMS_MAIN, "Argc is invalid: %{public}zu", argc);
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    int32_t persistentId = -1;
    if (!ConvertFromJsValue(env, argv[0], persistentId)) {
        TLOGE(WmsLogTag::WMS_MAIN, "Failed to convert parameter to persistentId");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    double scaleValue = 0.0;
    if (!ConvertFromJsValue(env, argv[1], scaleValue)) {
        TLOGE(WmsLogTag::WMS_MAIN, "Failed to convert parameter to scaleValue");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    SnapshotNodeType snapshotNode = SnapshotNodeType::DEFAULT_NODE;
    if (argc > ARGC_TWO && !ConvertFromJsValue(env, argv[ARG_INDEX_TWO], snapshotNode)) {
        TLOGE(WmsLogTag::WMS_PATTERN, "Failed to convert parameter to snapshotNode");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    if (snapshotNode < SnapshotNodeType::DEFAULT_NODE || snapshotNode > SnapshotNodeType::APP_NODE) {
        TLOGE(WmsLogTag::WMS_PATTERN, "SnapshotNodeType invalid:%{public}d", snapshotNode);
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    bool needSnapshot = true;
    if (argc > ARGC_THREE && !ConvertFromJsValue(env, argv[ARG_INDEX_THREE], needSnapshot)) {
        TLOGE(WmsLogTag::WMS_PATTERN, "Failed to convert parameter to needSnapshot");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    float scaleParam = GreatOrEqual(scaleValue, 0.0f) && LessOrEqual(scaleValue, 1.0f) ?
        static_cast<float>(scaleValue) : 0.0f;
    std::shared_ptr<Media::PixelMap> pixelPtr =
        SceneSessionManager::GetInstance().GetSessionSnapshotPixelMap(
            persistentId, scaleParam, snapshotNode, needSnapshot);
    if (pixelPtr == nullptr) {
        TLOGE(WmsLogTag::WMS_MAIN, "Failed to create pixlePtr");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_STATE_ABNORMALLY),
            "System is abnormal"));
        return NapiGetUndefined(env);
    }
    napi_value pixelMapObj = Media::PixelMapNapi::CreatePixelMap(env, pixelPtr);
    if (pixelMapObj == nullptr) {
        TLOGE(WmsLogTag::WMS_MAIN, "Failed to create pixel map object");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_STATE_ABNORMALLY),
            "System is abnormal"));
        return NapiGetUndefined(env);
    }
    return pixelMapObj;
}

napi_value JsSceneSessionManager::OnSetAppDragResizeType(napi_env env, napi_callback_info info)
{
    size_t argc = ARGC_FOUR;
    napi_value argv[ARGC_FOUR] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc < ARGC_TWO) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "Argc is invalid: %{public}zu", argc);
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    std::string bundleName;
    if (!ConvertFromJsValue(env, argv[0], bundleName)) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "Failed to convert parameter to bundleName");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    DragResizeType dragResizeType;
    if (!ConvertDragResizeTypeFromJs(env, argv[ARGC_ONE], dragResizeType)) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "Failed to convert parameter to dragResizeType");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    TLOGD(WmsLogTag::WMS_LAYOUT, "dragResizeType: %{public}d, bundleName: %{public}s",
        dragResizeType, bundleName.c_str());
    WMError err = SceneSessionManager::GetInstance().SetAppDragResizeTypeInner(bundleName, dragResizeType);
    if (err != WMError::WM_OK) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "Failed to call set method");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_STATE_ABNORMALLY),
            "System is abnormal"));
        return NapiGetUndefined(env);
    }
    return NapiGetUndefined(env);
}

napi_value JsSceneSessionManager::GetCustomDecorHeight(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::WMS_DECOR, "[NAPI]");
    JsSceneSessionManager* me = CheckParamsAndGetThis<JsSceneSessionManager>(env, info);
    return (me != nullptr) ? me->OnGetCustomDecorHeight(env, info) : nullptr;
}

napi_value JsSceneSessionManager::OnGetCustomDecorHeight(napi_env env, napi_callback_info info)
{
    size_t argc = 4;
    napi_value argv[4] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc < 1) {
        TLOGE(WmsLogTag::WMS_DECOR, "Argc is invalid: %{public}zu", argc);
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    int32_t persistentId;
    if (!ConvertFromJsValue(env, argv[0], persistentId)) {
        TLOGE(WmsLogTag::WMS_DECOR, "Failed to convert parameter to persistentId");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    int32_t customDecorHeight = SceneSessionManager::GetInstance().GetCustomDecorHeight(persistentId);
    napi_value result = nullptr;
    napi_create_int32(env, customDecorHeight, &result);
    return result;
}

napi_value JsSceneSessionManager::OnSwitchFreeMultiWindow(napi_env env, napi_callback_info info)
{
    size_t argc = 4;
    napi_value argv[4] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc < ARGC_ONE) {
        TLOGE(WmsLogTag::WMS_LAYOUT_PC, "Argc is invalid: %{public}zu", argc);
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    bool enable;
    if (!ConvertFromJsValue(env, argv[0], enable)) {
        TLOGE(WmsLogTag::WMS_LAYOUT_PC, "Failed to convert parameter to intoFreeMultiWindow bool value");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    SceneSessionManager::GetInstance().SwitchFreeMultiWindow(enable);
    return NapiGetUndefined(env);
}

napi_value JsSceneSessionManager::OnGetFreeMultiWindowConfig(napi_env env, napi_callback_info info)
{
    auto systemConfig = SceneSessionManager::GetInstance().GetSystemSessionConfig();
    return JsWindowSceneConfig::CreateFreeMultiWindowConfig(env, systemConfig);
}

napi_value JsSceneSessionManager::OnGetIsLayoutFullScreen(napi_env env, napi_callback_info info)
{
    bool isLayoutFullScreen = false;
    SceneSessionManager::GetInstance().GetIsLayoutFullScreen(isLayoutFullScreen);
    napi_value result = nullptr;
    napi_get_boolean(env, isLayoutFullScreen, &result);
    return result;
}

napi_value JsSceneSessionManager::OnSetMaximizeFullScreen(napi_env env, napi_callback_info info)
{
    size_t argc = ARGC_TWO;
    napi_value argv[ARGC_TWO] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc < ARGC_TWO) {
        TLOGE(WmsLogTag::WMS_LAYOUT_PC, "Argc is invalid: %{public}zu", argc);
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    uint32_t persistentId = 0;
    if (!ConvertFromJsValue(env, argv[0], persistentId)) {
        TLOGE(WmsLogTag::WMS_LAYOUT_PC, "Failed to convert parameter to persistentId");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    bool isMaximizeFullScreen;
    if (!ConvertFromJsValue(env, argv[1], isMaximizeFullScreen)) {
        TLOGE(WmsLogTag::WMS_LAYOUT_PC, "Failed to convert parameter to isMaximizeFullScreen bool value");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    SceneSessionManager::GetInstance().SetMaximizeFullScreen(static_cast<int32_t>(persistentId), isMaximizeFullScreen);
    napi_value result = nullptr;
    napi_get_boolean(env, isMaximizeFullScreen, &result);
    return result;
}

napi_value JsSceneSessionManager::OnGetWindowLimits(napi_env env, napi_callback_info info)
{
    size_t argc = 4;
    napi_value argv[4] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc < ARGC_ONE) {
        TLOGE(WmsLogTag::WMS_LAYOUT_PC, "Argc is invalid: %{public}zu", argc);
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    int32_t windowId;
    if (!ConvertFromJsValue(env, argv[0], windowId)) {
        TLOGE(WmsLogTag::WMS_LAYOUT_PC, "Failed to convert parameter to windowId");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    WindowLimits windowLimits;
    WMError ret = SceneSessionManager::GetInstance().GetWindowLimits(windowId, windowLimits);
    if (ret != WMError::WM_OK) {
        WmErrorCode wmErrorCode = WM_JS_TO_ERROR_CODE_MAP.at(ret);
        TLOGE(WmsLogTag::WMS_LAYOUT_PC, "Get window limits failed, return %{public}d", wmErrorCode);
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(wmErrorCode), "Get window limits failed."));
        return NapiGetUndefined(env);
    }
    napi_value jsWindowLimitsObj = JsWindowSceneConfig::CreateWindowLimits(env, windowLimits);
    if (jsWindowLimitsObj == nullptr) {
        TLOGE(WmsLogTag::WMS_LAYOUT_PC, "jsWindowLimitsObj is nullptr");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_STATE_ABNORMALLY),
            "Get window limits failed."));
        return NapiGetUndefined(env);
    }
    return jsWindowLimitsObj;
}

napi_value JsSceneSessionManager::OnSetIsDockAutoHide(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value argv[1] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc < ARGC_ONE) {
        TLOGE(WmsLogTag::WMS_LAYOUT_PC, "Argc is invalid: %{public}zu", argc);
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    bool isDockAutoHide = false;
    if (!ConvertFromJsValue(env, argv[0], isDockAutoHide)) {
        TLOGE(WmsLogTag::WMS_LAYOUT_PC, "Failed to convert parameter to isDockAutoHide bool value");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    SceneSessionManager::GetInstance().ConfigDockAutoHide(isDockAutoHide);
    return NapiGetUndefined(env);
}

napi_value JsSceneSessionManager::OnHandleTrayAppChange(napi_env env, napi_callback_info info)
{
    size_t argc = DEFAULT_ARG_COUNT;
    napi_value argv[DEFAULT_ARG_COUNT] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc != ARGC_ONE) {
        TLOGE(WmsLogTag::WMS_LIFE, "Argc is invalid: %{public}zu", argc);
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
                                      "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    std::string trayAppListStr = "";
    if (!ConvertFromJsValue(env, argv[ARG_INDEX_ZERO], trayAppListStr)) {
        TLOGE(WmsLogTag::WMS_LIFE, "Failed to convert parameter to trayAppList");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
                                      "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    std::stringstream trayAppListStream(trayAppListStr);
    std::string item;
    std::vector<std::string> trayAppList;
    while (std::getline(trayAppListStream, item, '|')) {
        if (!item.empty())  {
            trayAppList.push_back(item);
        }
    }
    SceneSessionManager::GetInstance().SetTrayAppList(std::move(trayAppList));
    return NapiGetUndefined(env);
}

napi_value JsSceneSessionManager::OnNotifyEnterRecentTask(napi_env env, napi_callback_info info)
{
    size_t argc = 4;
    napi_value argv[4] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);

    if (argc < ARGC_ONE) {
        TLOGE(WmsLogTag::WMS_IMMS, "Argc is invalid: %{public}zu", argc);
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    bool enterRecent = false;
    if (!ConvertFromJsValue(env, argv[0], enterRecent)) {
        TLOGE(WmsLogTag::WMS_IMMS, "Failed to convert parameter to enterRecent");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }

    SceneSessionManager::GetInstance().NotifyEnterRecentTask(enterRecent);
    return NapiGetUndefined(env);
}

std::shared_ptr<NativeReference> JsSceneSessionManager::GetJSCallback(const std::string& functionName)
{
    HITRACE_METER_FMT(HITRACE_TAG_WINDOW_MANAGER, "JsSceneSessionManager::GetJSCallback[%s]", functionName.c_str());
    std::shared_ptr<NativeReference> jsCallBack = nullptr;
    std::shared_lock<std::shared_mutex> lock(jsCbMapMutex_);
    auto iter = jsCbMap_.find(functionName);
    if (iter == jsCbMap_.end()) {
        TLOGE(WmsLogTag::DEFAULT, "Can't find callback %{public}s", functionName.c_str());
        return jsCallBack;
    }
    jsCallBack = iter->second;
    if (jsCallBack == nullptr) {
        TLOGE(WmsLogTag::DEFAULT, "Find function %{public}s, but callback is nullptr!", functionName.c_str());
    }
    return jsCallBack;
}

napi_value JsSceneSessionManager::OnUpdateDisplayHookInfo(napi_env env, napi_callback_info info)
{
    size_t argc = 5;
    napi_value argv[5] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);

    if (argc < ARGC_FIVE) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "Argc is invalid: %{public}zu", argc);
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }

    int32_t uid = 0;
    if (!ConvertFromJsValue(env, argv[0], uid)) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "Failed to convert parameter to uid");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }

    uint32_t width;
    if (!ConvertFromJsValue(env, argv[ARGC_ONE], width)) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "Failed to convert parameter to width");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }

    uint32_t height;
    if (!ConvertFromJsValue(env, argv[ARGC_TWO], height)) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "Failed to convert parameter to height");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }

    double_t density = 1.0;
    if (!ConvertFromJsValue(env, argv[ARGC_THREE], density)) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "Failed to convert parameter to density");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }

    bool enable;
    if (!ConvertFromJsValue(env, argv[ARGC_FOUR], enable)) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "Failed to convert parameter to enable");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    SceneSessionManager::GetInstance().UpdateDisplayHookInfo(uid, width, height, static_cast<float_t>(density), enable);
    return NapiGetUndefined(env);
}

napi_value JsSceneSessionManager::OnUpdateAppHookDisplayInfo(napi_env env, napi_callback_info info)
{
    size_t argc = 3;
    napi_value argv[3] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);

    if (argc < ARGC_THREE) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "Argc is invalid: %{public}zu", argc);
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }

    int32_t uid = 0;
    if (!ConvertFromJsValue(env, argv[0], uid)) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "Failed to convert parameter to uid");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }

    HookInfo hookInfo;
    if (argv[1] == nullptr || !ConvertHookInfoFromJs(env, argv[1], hookInfo)) {
        WLOGFE("Failed to convert parameter to hookInfo");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }

    bool enable = false;
    if (!ConvertFromJsValue(env, argv[ARGC_TWO], enable)) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "Failed to convert parameter to enable");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    SceneSessionManager::GetInstance().UpdateAppHookDisplayInfo(uid, hookInfo, enable);
    return NapiGetUndefined(env);
}

napi_value JsSceneSessionManager::OnUpdateAppHookWindowInfo(napi_env env, napi_callback_info info)
{
    size_t argc = ARGC_TWO;
    napi_value argv[ARGC_TWO] = { nullptr };
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);

    if (argc < ARGC_TWO) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "Argc is invalid: %{public}zu", argc);
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }

    std::string bundleName;
    if (!ConvertFromJsValue(env, argv[ARG_INDEX_ZERO], bundleName)) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "Failed to convert parameter to bundleName");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }

    HookWindowInfo hookWindowInfo{};
    if (!ConvertHookWindowInfoFromJs(env, argv[ARG_INDEX_ONE], hookWindowInfo)) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "Failed to convert parameter to hookWindowInfo");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    SceneSessionManager::GetInstance().UpdateAppHookWindowInfo(bundleName, hookWindowInfo);
    return NapiGetUndefined(env);
}

napi_value JsSceneSessionManager::OnNotifyHookOrientationChange(napi_env env, napi_callback_info info)
{
    size_t argc = ARGC_ONE;
    napi_value argv[ARGC_ONE] = { nullptr };
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);

    if (argc != ARGC_ONE) {
        TLOGE(WmsLogTag::WMS_COMPAT, "Argc is invalid: %{public}zu", argc);
        return NapiGetUndefined(env);
    }

    int32_t persistentId = 0;
    if (!ConvertFromJsValue(env, argv[0], persistentId)) {
        TLOGE(WmsLogTag::WMS_COMPAT, "Failed to convert parameter to persistentId");
        return NapiGetUndefined(env);
    }
    SceneSessionManager::GetInstance().NotifyHookOrientationChange(persistentId);
    return NapiGetUndefined(env);
}

napi_value JsSceneSessionManager::SetAppForceLandscapeConfig(napi_env env, napi_callback_info info)
{
    JsSceneSessionManager* me = CheckParamsAndGetThis<JsSceneSessionManager>(env, info);
    return (me != nullptr) ? me->OnSetAppForceLandscapeConfig(env, info) : nullptr;
}

napi_value JsSceneSessionManager::OnSetAppForceLandscapeConfig(napi_env env, napi_callback_info info)
{
    size_t argc = ARGC_FOUR;
    napi_value argv[ARGC_FOUR] = { nullptr };
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc != OHOS::Rosen::ARGC_FOUR) {
        TLOGE(WmsLogTag::DEFAULT, "Argc is invalid: %{public}zu", argc);
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }

    std::string bundleName;
    if (!ConvertFromJsValue(env, argv[ARG_INDEX_ZERO], bundleName)) {
        TLOGE(WmsLogTag::DEFAULT, "Failed to convert parameter to bundleName");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }

    AppForceLandscapeConfig config;
    napi_value jsMode = nullptr;
    napi_get_named_property(env, argv[ARG_INDEX_ONE], "mode", &jsMode);
    RETURN_IF_CONVERT_FAIL(env, jsMode, config.mode_, "mode", WmsLogTag::DEFAULT);
    napi_value jsSupportSplit = nullptr;
    napi_get_named_property(env, argv[ARG_INDEX_ONE], "supportSplit", &jsSupportSplit);
    RETURN_IF_CONVERT_FAIL(env, jsSupportSplit, config.supportSplit_, "supportSplit", WmsLogTag::DEFAULT);
    napi_value jsIgnoreOrient = nullptr;
    napi_get_named_property(env, argv[ARG_INDEX_ONE], "ignoreOrientation", &jsIgnoreOrient);
    RETURN_IF_CONVERT_FAIL(env, jsIgnoreOrient, config.ignoreOrientation_, "ignoreOrientation",
        WmsLogTag::DEFAULT);
    napi_value jsContainsSysConfig = nullptr;
    napi_get_named_property(env, argv[ARG_INDEX_ONE], "containsSysConfig", &jsContainsSysConfig);
    RETURN_IF_CONVERT_FAIL(env, jsContainsSysConfig, config.containsSysConfig_, "containsSysConfig",
        WmsLogTag::DEFAULT);
    napi_value jsContainsAppConfig = nullptr;
    napi_get_named_property(env, argv[ARG_INDEX_ONE], "containsAppConfig", &jsContainsAppConfig);
    RETURN_IF_CONVERT_FAIL(env, jsContainsAppConfig, config.containsAppConfig_, "containsAppConfig",
        WmsLogTag::DEFAULT);
    napi_value jsIsSysRouter = nullptr;
    napi_get_named_property(env, argv[ARG_INDEX_TWO], "isRouter", &jsIsSysRouter);
    ConvertFromJsValue(env, jsIsSysRouter, config.isSysRouter_);
    napi_value jsSysConfigJsonStr = nullptr;
    napi_get_named_property(env, argv[ARG_INDEX_TWO], "configJsonStr", &jsSysConfigJsonStr);
    ConvertFromJsValue(env, jsSysConfigJsonStr, config.sysConfigJsonStr_);
    napi_value jsSysHomePage = nullptr;
    napi_get_named_property(env, argv[ARG_INDEX_TWO], "homePage", &jsSysHomePage);
    ConvertFromJsValue(env, jsSysHomePage, config.sysHomePage_);
    napi_value jsIsAppRouter = nullptr;
    napi_get_named_property(env, argv[ARG_INDEX_THREE], "isRouter", &jsIsAppRouter);
    ConvertFromJsValue(env, jsIsAppRouter, config.isAppRouter_);
    napi_value jsAppConfigJsonStr = nullptr;
    napi_get_named_property(env, argv[ARG_INDEX_THREE], "configJsonStr", &jsAppConfigJsonStr);
    ConvertFromJsValue(env, jsAppConfigJsonStr, config.appConfigJsonStr_);

    TLOGI(WmsLogTag::DEFAULT, "SetAppForceLandscapeConfig bundleName: %{public}s, mode: %{public}d, "
        "supportSplit: %{public}d, ignoreOrientation: %{public}d, containsSysConfig: %{public}d, "
        "containsAppConfig: %{public}d, isSysRouter: %{public}d, sysConfigJsonStr: %{public}s,"
        "sysHomePage: %{public}s, isAppRouter: %{public}d, appConfigJsonStr: %{public}s",
        bundleName.c_str(), config.mode_, config.supportSplit_, config.ignoreOrientation_,
        config.containsSysConfig_, config.containsAppConfig_, config.isSysRouter_,
        config.sysConfigJsonStr_.c_str(), config.sysHomePage_.c_str(), config.isAppRouter_,
        config.appConfigJsonStr_.c_str());
    SceneSessionManager::GetInstance().SetAppForceLandscapeConfig(bundleName, config);
    return NapiGetUndefined(env);
}

napi_value JsSceneSessionManager::SetAppForceLandscapeConfigEnable(napi_env env, napi_callback_info info)
{
    JsSceneSessionManager* me = CheckParamsAndGetThis<JsSceneSessionManager>(env, info);
    return (me != nullptr) ? me->OnSetAppForceLandscapeConfigEnable(env, info) : nullptr;
}

napi_value JsSceneSessionManager::OnSetAppForceLandscapeConfigEnable(napi_env env, napi_callback_info info)
{
    size_t argc = ARGC_TWO;
    napi_value argv[ARGC_TWO] = { nullptr };
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc != OHOS::Rosen::ARGC_TWO) {
        TLOGE(WmsLogTag::DEFAULT, "Argc is invalid: %{public}zu", argc);
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }

    std::string bundleName;
    if (!ConvertFromJsValue(env, argv[ARG_INDEX_ZERO], bundleName)) {
        TLOGE(WmsLogTag::DEFAULT, "Failed to convert parameter to bundleName");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }

    bool enableForceSplit = false;
    if (GetType(env, argv[ARG_INDEX_ONE]) == napi_boolean) {
        RETURN_IF_CONVERT_FAIL(env, argv[ARG_INDEX_ONE], enableForceSplit, "enableForceSplit", WmsLogTag::DEFAULT);
    }

    TLOGI(WmsLogTag::DEFAULT, "SetAppForceLandscapeConfigEnable bundleName: %{public}s, enable: %{public}d",
        bundleName.c_str(), enableForceSplit);

    SceneSessionManager::GetInstance().SetAppForceLandscapeConfigEnable(bundleName, enableForceSplit);
    return NapiGetUndefined(env);
}
napi_value JsSceneSessionManager::OnIsScbCoreEnabled(napi_env env, napi_callback_info info)
{
    size_t argc = 4;
    napi_value argv[4] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc >= 1) {
        TLOGE(WmsLogTag::WMS_PIPELINE, "Argc is invalid: %{public}zu", argc);
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    napi_value result = nullptr;
    napi_get_boolean(env, Session::IsScbCoreEnabled(), &result);
    return result;
}

napi_value JsSceneSessionManager::OnNotifyAboveLockScreen(napi_env env, napi_callback_info info)
{
    size_t argc = ARGC_FOUR;
    napi_value argv[ARGC_FOUR] = { nullptr };
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc != ARGC_ONE) {
        TLOGE(WmsLogTag::WMS_UIEXT, "Argc is invalid: %{public}zu", argc);
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }

    std::vector<int32_t> windowIds;
    if (!ConvertInt32ArrayFromJs(env, argv[0], windowIds)) {
        TLOGE(WmsLogTag::WMS_UIEXT, "Failed to convert windowIds");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }

    TLOGI(WmsLogTag::WMS_UIEXT, "UIExtOnLock: window list size: %{public}zu", windowIds.size());
    SceneSessionManager::GetInstance().OnNotifyAboveLockScreen(windowIds);
    return NapiGetUndefined(env);
}

napi_value JsSceneSessionManager::OnCloneWindow(napi_env env, napi_callback_info info)
{
    size_t argc = ARGC_THREE;
    napi_value argv[ARGC_THREE] = { nullptr };
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc < ARGC_TWO) {
        TLOGE(WmsLogTag::WMS_PC, "Argc is invalid: %{public}zu", argc);
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    int32_t fromPersistentId = -1;
    if (!ConvertFromJsValue(env, argv[0], fromPersistentId)) {
        TLOGE(WmsLogTag::WMS_PC, "Failed to convert parameter fromPersistentId");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    int32_t toPersistentId = -1;
    if (!ConvertFromJsValue(env, argv[ARGC_ONE], toPersistentId)) {
        TLOGE(WmsLogTag::WMS_PC, "Failed to convert parameter toPersistentId");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    TLOGI(WmsLogTag::WMS_PC, "from:%{public}d to:%{public}d", fromPersistentId, toPersistentId);
    if (argc >= ARGC_THREE && GetType(env, argv[ARGC_TWO]) == napi_boolean) {
        bool needOffScreen = true;
        ConvertFromJsValue(env, argv[ARGC_TWO], needOffScreen);
        SceneSessionManager::GetInstance().CloneWindow(fromPersistentId, toPersistentId, needOffScreen);
    } else {
        SceneSessionManager::GetInstance().CloneWindow(fromPersistentId, toPersistentId);
    }
    return NapiGetUndefined(env);
}

napi_value JsSceneSessionManager::OnRefreshPcZOrder(napi_env env, napi_callback_info info)
{
    size_t argc = 4;
    napi_value argv[4] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc < ARGC_TWO) {
        TLOGE(WmsLogTag::WMS_LAYOUT_PC, "Argc is invalid: %{public}zu", argc);
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    uint32_t startZOrder;
    if (!ConvertFromJsValue(env, argv[0], startZOrder)) {
        TLOGE(WmsLogTag::WMS_LAYOUT_PC, "Failed to convert start Z order to %{public}d", startZOrder);
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    std::vector<int32_t> persistentIds;
    if (!ConvertInt32ArrayFromJs(env, argv[1], persistentIds)) {
        TLOGE(WmsLogTag::WMS_LAYOUT_PC, "Failed to convert persistentIds");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    SceneSessionManager::GetInstance().RefreshPcZOrderList(startZOrder, std::move(persistentIds));
    return NapiGetUndefined(env);
}

napi_value JsSceneSessionManager::OnGetMaxInstanceCount(napi_env env, napi_callback_info info)
{
    size_t argc = 4;
    napi_value argv[4] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc != ARGC_ONE) {
        TLOGE(WmsLogTag::WMS_LIFE, "Argc is invalid: %{public}zu", argc);
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    std::string bundleName;
    if (!ConvertFromJsValue(env, argv[0], bundleName)) {
        TLOGE(WmsLogTag::WMS_LIFE, "Failed to convert parameter to bundleName");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    napi_value result = nullptr;
    napi_create_uint32(env, SceneSessionManager::GetInstance().GetMaxInstanceCount(bundleName), &result);
    return result;
}

napi_value JsSceneSessionManager::OnGetInstanceCount(napi_env env, napi_callback_info info)
{
    size_t argc = 4;
    napi_value argv[4] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc != ARGC_ONE) {
        TLOGE(WmsLogTag::WMS_LIFE, "Argc is invalid: %{public}zu", argc);
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    std::string bundleName;
    if (!ConvertFromJsValue(env, argv[0], bundleName)) {
        TLOGE(WmsLogTag::WMS_LIFE, "Failed to convert parameter to bundleName");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    napi_value result = nullptr;
    napi_create_uint32(env, SceneSessionManager::GetInstance().GetInstanceCount(bundleName), &result);
    return result;
}

napi_value JsSceneSessionManager::OnGetLastInstanceKey(napi_env env, napi_callback_info info)
{
    size_t argc = 4;
    napi_value argv[4] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc != ARGC_ONE) {
        TLOGE(WmsLogTag::WMS_LIFE, "Argc is invalid: %{public}zu", argc);
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    std::string bundleName;
    if (!ConvertFromJsValue(env, argv[0], bundleName)) {
        TLOGE(WmsLogTag::WMS_LIFE, "Failed to convert parameter to bundleName");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    std::string instanceKey = SceneSessionManager::GetInstance().GetLastInstanceKey(bundleName);
    napi_value result = nullptr;
    napi_create_string_utf8(env, instanceKey.c_str(), instanceKey.length(), &result);
    return result;
}

napi_value JsSceneSessionManager::OnRefreshAppInfo(napi_env env, napi_callback_info info)
{
    size_t argc = ARGC_FOUR;
    napi_value argv[ARGC_FOUR] = { nullptr };
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc != ARGC_ONE) {
        TLOGE(WmsLogTag::WMS_LIFE, "Argc is invalid: %{public}zu", argc);
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    std::string bundleName;
    if (!ConvertFromJsValue(env, argv[0], bundleName)) {
        TLOGE(WmsLogTag::WMS_LIFE, "Failed to convert parameter to bundleName");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    SceneSessionManager::GetInstance().RefreshAppInfo(bundleName);
    return NapiGetUndefined(env);
}

napi_value JsSceneSessionManager::OnGetWindowPid(napi_env env, napi_callback_info info)
{
    size_t argc = 4;
    napi_value argv[4] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc < ARGC_ONE) {
        TLOGE(WmsLogTag::WMS_SCB, "Argc is invalid: %{public}zu", argc);
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    int32_t windowId;
    if (!ConvertFromJsValue(env, argv[0], windowId)) {
        TLOGE(WmsLogTag::WMS_SCB, "Failed to convert parameter to windowId");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    int32_t pid = INVALID_PID;
    WMError ret = SceneSessionManager::GetInstance().CheckWindowId(windowId, pid);
    if (ret != WMError::WM_OK) {
        WmErrorCode wmErrorCode = WM_JS_TO_ERROR_CODE_MAP.at(ret);
        TLOGE(WmsLogTag::WMS_SCB, "Get window pid failed, return %{public}d", wmErrorCode);
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(wmErrorCode), "Get window pid failed."));
        return NapiGetUndefined(env);
    }
    napi_value result = nullptr;
    napi_create_int32(env, pid, &result);
    return result;
}

napi_value JsSceneSessionManager::OnUpdatePcFoldScreenStatus(napi_env env, napi_callback_info info)
{
    size_t argc = ARGC_FIVE;
    napi_value argv[ARGC_FIVE] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);

    if (argc < ARGC_FIVE) {
        TLOGE(WmsLogTag::WMS_PC, "Argc is invalid: %{public}zu", argc);
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }

    int64_t displayId { INVALID_SCREEN_ID };
    if (!ConvertFromJsValue(env, argv[0], displayId)) {
        TLOGE(WmsLogTag::WMS_PC, "Failed to convert parameter to displayId");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }

    uint32_t statusNum;
    if (!ConvertFromJsValue(env, argv[ARG_INDEX_ONE], statusNum)) {
        TLOGE(WmsLogTag::WMS_PC, "Failed to convert parameter to status");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    SuperFoldStatus status = static_cast<SuperFoldStatus>(statusNum);

    WSRect defaultDisplayRect;
    if (argv[ARG_INDEX_TWO] == nullptr || !ConvertRectInfoFromJs(env, argv[ARG_INDEX_TWO], defaultDisplayRect)) {
        TLOGE(WmsLogTag::WMS_PC, "Failed to convert parameter to display rect");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }

    WSRect virtualDisplayRect;
    if (argv[ARG_INDEX_THREE] == nullptr || !ConvertRectInfoFromJs(env, argv[ARG_INDEX_THREE], virtualDisplayRect)) {
        TLOGE(WmsLogTag::WMS_PC, "Failed to convert parameter to virtual rect");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }

    WSRect foldCreaseRect;
    if (argv[ARG_INDEX_FOUR] == nullptr || !ConvertRectInfoFromJs(env, argv[ARG_INDEX_FOUR], foldCreaseRect)) {
        TLOGE(WmsLogTag::WMS_PC, "Failed to convert parameter to fold crease rect");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }

    PcFoldScreenManager::GetInstance().UpdateFoldScreenStatus(displayId, status,
        defaultDisplayRect, virtualDisplayRect, foldCreaseRect);
    return NapiGetUndefined(env);
}

napi_value JsSceneSessionManager::OnUpdateSystemKeyboardStatus(napi_env env, napi_callback_info info)
{
    size_t argc = ARGC_FOUR;
    napi_value argv[ARGC_FOUR] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);

    if (argc < ARGC_ONE) {
        TLOGE(WmsLogTag::WMS_KEYBOARD, "Argc is invalid: %{public}zu", argc);
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }

    bool hasSystemKeyboard = false;
    if (argv[0] == nullptr || !ConvertFromJsValue(env, argv[0], hasSystemKeyboard)) {
        TLOGE(WmsLogTag::WMS_KEYBOARD, "Failed to convert parameter to bool");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }

    PcFoldScreenManager::GetInstance().UpdateSystemKeyboardStatus(hasSystemKeyboard);
    return NapiGetUndefined(env);
}

napi_value JsSceneSessionManager::OnResetPcFoldScreenArrangeRule(napi_env env, napi_callback_info info)
{
    size_t argc = ARGC_FOUR;
    napi_value argv[ARGC_FOUR] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);

    if (argc < ARGC_ONE) {
        TLOGE(WmsLogTag::WMS_PC, "Argc is invalid: %{public}zu", argc);
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }

    WSRect rect;
    if (argv[0] == nullptr || !ConvertRectInfoFromJs(env, argv[0], rect)) {
        TLOGE(WmsLogTag::WMS_PC, "Failed to convert parameter to rect");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }

    PcFoldScreenManager::GetInstance().ResetArrangeRule(rect);
    return NapiGetUndefined(env);
}

void JsSceneSessionManager::OnCloseTargetFloatWindow(const std::string& bundleName)
{
    TLOGD(WmsLogTag::WMS_MULTI_WINDOW, "");
    auto task = [this, bundleName, jsCallBack = GetJSCallback(CLOSE_TARGET_FLOAT_WINDOW_CB), env = env_] {
        if (jsCallBack == nullptr) {
            TLOGNE(WmsLogTag::WMS_MULTI_WINDOW, "jsCallBack is nullptr");
            return;
        }
        napi_value jsBundleNameObj = CreateJsValue(env, bundleName);
        napi_value argv[] = {jsBundleNameObj};
        napi_call_function(env, NapiGetUndefined(env), jsCallBack->GetNapiValue(), ArraySize(argv), argv, nullptr);
    };
    taskScheduler_->PostMainThreadTask(task, "OnCloseTargetFloatWindow bundleName:" + bundleName);
}

void JsSceneSessionManager::ProcessCloseTargetFloatWindow()
{
    ProcessCloseTargetFloatWindowFunc func = [this](const std::string& bundleName) {
        TLOGND(WmsLogTag::WMS_MULTI_WINDOW, "ProcessCloseTargetFloatWindow. bundleName:%{public}s", bundleName.c_str());
        this->OnCloseTargetFloatWindow(bundleName);
    };
    SceneSessionManager::GetInstance().SetCloseTargetFloatWindowFunc(func);
}

static napi_value CreateAppUseControlInfos(
    napi_env env, const std::vector<AppUseControlInfo>& controlList)
{
    napi_value arrayValue = nullptr;
    napi_create_array_with_length(env, controlList.size(), &arrayValue);
    if (arrayValue == nullptr) {
        TLOGE(WmsLogTag::WMS_LIFE, "Failed to create napi array");
        return NapiGetUndefined(env);
    }
    int32_t index = 0;
    for (const auto& appUseControlInfo : controlList) {
        napi_value objValue = nullptr;
        napi_create_object(env, &objValue);
        if (objValue == nullptr) {
            TLOGE(WmsLogTag::WMS_LIFE, "failed to create napi object");
            return NapiGetUndefined(env);
        }
        napi_set_named_property(env, objValue, "bundleName", CreateJsValue(env, appUseControlInfo.bundleName_));
        napi_set_named_property(env, objValue, "appIndex", CreateJsValue(env, appUseControlInfo.appIndex_));
        napi_set_named_property(env, objValue, "isNeedControl", CreateJsValue(env, appUseControlInfo.isNeedControl_));
        napi_set_named_property(env, objValue, "isControlRecentOnly",
            CreateJsValue(env, appUseControlInfo.isControlRecentOnly_));
        napi_set_element(env, arrayValue, index++, objValue);
    }
    return arrayValue;
}

void JsSceneSessionManager::RegisterNotifyAppUseControlListCallback()
{
    TLOGI(WmsLogTag::WMS_LIFE, "in");
    SceneSessionManager::GetInstance().RegisterNotifyAppUseControlListCallback(
        [this](ControlAppType type, int32_t userId, const std::vector<AppUseControlInfo>& controlList) {
            this->OnNotifyAppUseControlList(type, userId, controlList);
        });
}

napi_value JsSceneSessionManager::OnSetIsWindowRectAutoSave(napi_env env, napi_callback_info info)
{
    size_t argc = DEFAULT_ARG_COUNT;
    napi_value argv[DEFAULT_ARG_COUNT] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc != DEFAULT_ARG_COUNT) {
        TLOGE(WmsLogTag::WMS_MAIN, "Argc is invalid: %{public}zu", argc);
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    std::string key;
    if (!ConvertFromJsValue(env, argv[ARG_INDEX_ZERO], key)) {
        TLOGE(WmsLogTag::WMS_MAIN, "Failed to convert key to %{public}s", key.c_str());
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    bool enabled = false;
    if (!ConvertFromJsValue(env, argv[ARG_INDEX_ONE], enabled)) {
        TLOGE(WmsLogTag::WMS_MAIN, "Failed to convert enabled to %{public}d", enabled);
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    std::string abilityKey;
    if (!ConvertFromJsValue(env, argv[ARG_INDEX_TWO], abilityKey)) {
        TLOGE(WmsLogTag::WMS_MAIN, "Failed to convert abilityKey to %{public}s", abilityKey.c_str());
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    bool isSaveBySpecifiedFlag = false;
    if (!ConvertFromJsValue(env, argv[ARG_INDEX_THREE], isSaveBySpecifiedFlag)) {
        TLOGE(WmsLogTag::WMS_MAIN, "Failed to convert isSaveBySpecifiedFlag to %{public}d", isSaveBySpecifiedFlag);
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    SceneSessionManager::GetInstance().SetIsWindowRectAutoSave(key, enabled, abilityKey, isSaveBySpecifiedFlag);
    return NapiGetUndefined(env);
}

void JsSceneSessionManager::OnNotifyAppUseControlList(
    ControlAppType type, int32_t userId, const std::vector<AppUseControlInfo>& controlList)
{
    TLOGI(WmsLogTag::WMS_LIFE, "in");
    taskScheduler_->PostMainThreadTask([this, type, userId, controlList,
        jsCallBack = GetJSCallback(NOTIFY_APP_USE_CONTROL_LIST_CB), env = env_] {
        if (jsCallBack == nullptr) {
            TLOGNE(WmsLogTag::WMS_LIFE, "jsCallBack is nullptr");
            return;
        }
        napi_value typeValue = CreateJsValue(env, static_cast<uint8_t>(type));
        napi_value userIdValue = CreateJsValue(env, userId);
        napi_value controlListValue = CreateAppUseControlInfos(env, controlList);
        napi_value argv[] = { typeValue, userIdValue, controlListValue };
        napi_call_function(env, NapiGetUndefined(env), jsCallBack->GetNapiValue(), ArraySize(argv), argv, nullptr);
    }, __func__);
}

void JsSceneSessionManager::RegisterWatchGestureConsumeResultCallback()
{
    SceneSessionManager::GetInstance().RegisterWatchGestureConsumeResultCallback(
        [this](int32_t keyCode, bool isConsumed) {
            TLOGND(WmsLogTag::WMS_EVENT, "RegisterWatchGestureConsumeResultCallback called");
            this->OnWatchGestureConsumeResult(keyCode, isConsumed);
        });
}

void JsSceneSessionManager::OnWatchGestureConsumeResult(int32_t keyCode, bool isConsumed)
{
    TLOGD(WmsLogTag::WMS_EVENT, "in");
    taskScheduler_->PostMainThreadTask([this, keyCode, isConsumed,
        jsCallBack = GetJSCallback(WATCH_GESTURE_CONSUME_RESULT_CB), env = env_] {
        if (jsCallBack == nullptr) {
            TLOGNE(WmsLogTag::WMS_EVENT, "jsCallBack is nullptr");
            return;
        }
        napi_value objValue = nullptr;
        napi_create_object(env, &objValue);
        if (objValue == nullptr) {
            TLOGNE(WmsLogTag::WMS_EVENT, "jsCallBack is nullptr");
            return;
        }
        napi_set_named_property(env, objValue, "keyCode", CreateJsValue(env, keyCode));
        napi_set_named_property(env, objValue, "isConsumed", CreateJsValue(env, isConsumed));
        napi_value argv[] = { objValue };
        napi_call_function(env, NapiGetUndefined(env), jsCallBack->GetNapiValue(), ArraySize(argv), argv, nullptr);
    }, __func__);
}

void JsSceneSessionManager::RegisterWatchFocusActiveChangeCallback()
{
    SceneSessionManager::GetInstance().RegisterWatchFocusActiveChangeCallback([this](bool isActive) {
        TLOGND(WmsLogTag::WMS_EVENT, "RegisterWatchFocusActiveChangeCallback called");
        this->OnWatchFocusActiveChange(isActive);
    });
}

void JsSceneSessionManager::OnWatchFocusActiveChange(bool isActive)
{
    TLOGD(WmsLogTag::WMS_EVENT, "in");
    taskScheduler_->PostMainThreadTask([this, isActive,
        jsCallBack = GetJSCallback(WATCH_FOCUS_ACTIVE_CHANGE_CB), env = env_] {
        if (jsCallBack == nullptr) {
            TLOGNE(WmsLogTag::WMS_EVENT, "jsCallBack is nullptr");
            return;
        }
        napi_value isActiveValue = CreateJsValue(env, isActive);
        napi_value argv[] = { isActiveValue };
        napi_call_function(env, NapiGetUndefined(env), jsCallBack->GetNapiValue(), ArraySize(argv), argv, nullptr);
    }, __func__);
}

void JsSceneSessionManager::RegisterSetForegroundWindowNumCallback()
{
    SceneSessionManager::GetInstance().RegisterSetForegroundWindowNumCallback([this](uint32_t windowNum) {
        TLOGND(WmsLogTag::WMS_PC, "RegisterSetForegroundWindowNumCallback called");
        this->OnSetForegroundWindowNum(windowNum);
    });
}

void JsSceneSessionManager::OnSetForegroundWindowNum(uint32_t windowNum)
{
    TLOGD(WmsLogTag::WMS_PC, "in");
    taskScheduler_->PostMainThreadTask([this, windowNum,
        jsCallBack = GetJSCallback(SET_FOREGROUND_WINDOW_NUM_CB), env = env_] {
        if (jsCallBack == nullptr) {
            TLOGNE(WmsLogTag::WMS_PC, "jsCallBack is nullptr");
            return;
        }
        napi_value windowNumValue = CreateJsValue(env, windowNum);
        napi_value argv[] = { windowNumValue };
        napi_call_function(env, NapiGetUndefined(env), jsCallBack->GetNapiValue(), ArraySize(argv), argv, nullptr);
    }, __func__);
}

void JsSceneSessionManager::RegisterMinimizeByWindowIdCallback()
{
    SceneSessionManager::GetInstance().RegisterMinimizeByWindowIdCallback(
        [this](const std::vector<int32_t>& windowIds) {
        TLOGND(WmsLogTag::WMS_PC, "RegisterMinimizeByWindowIdCallback called");
        this->OnMinimizeByWindowId(windowIds);
    });
}

static napi_value CreateWindowIds(napi_env env, const std::vector<int32_t>& windowIds)
{
    napi_value arrayValue = nullptr;
    napi_create_array_with_length(env, windowIds.size(), &arrayValue);
    if (arrayValue == nullptr) {
        TLOGE(WmsLogTag::WMS_PC, "Failed to create napi array");
        return NapiGetUndefined(env);
    }
    auto index = 0;
    for (const auto& windowId : windowIds) {
        napi_set_element(env, arrayValue, index++, CreateJsValue(env, static_cast<int32_t>(windowId)));
    }
    return arrayValue;
}

void JsSceneSessionManager::OnMinimizeByWindowId(const std::vector<int32_t>& windowIds)
{
    TLOGD(WmsLogTag::WMS_PC, "in");
    taskScheduler_->PostMainThreadTask([this, windowIds,
        jsCallBack = GetJSCallback(MINIMIZE_BY_WINDOW_ID_CB), env = env_] {
        if (jsCallBack == nullptr) {
            TLOGNE(WmsLogTag::WMS_PC, "jsCallBack is nullptr");
            return;
        }
        napi_value windowIdList = CreateWindowIds(env, windowIds);
        napi_value argv[] = { windowIdList };
        napi_call_function(env, NapiGetUndefined(env), jsCallBack->GetNapiValue(), ArraySize(argv), argv, nullptr);
    }, __func__);
}

napi_value JsSceneSessionManager::OnSetStatusBarAvoidHeight(napi_env env, napi_callback_info info)
{
    size_t argc = ARGC_FOUR;
    napi_value argv[ARGC_FOUR] = { nullptr };
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc < ARGC_ONE) {
        TLOGE(WmsLogTag::WMS_IMMS, "Argc is invalid: %{public}zu", argc);
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    int64_t displayId = -1;
    if (!ConvertFromJsValue(env, argv[0], displayId) || displayId < 0) {
        TLOGE(WmsLogTag::WMS_IMMS, "Failed to convert parameter to displayId");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    int32_t height = 0;
    if (!ConvertFromJsValue(env, argv[1], height)) {
        TLOGE(WmsLogTag::WMS_IMMS, "Failed to convert parameter to height");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    SceneSessionManager::GetInstance().SetStatusBarAvoidHeight(static_cast<uint64_t>(displayId), height);
    return NapiGetUndefined(env);
}

napi_value JsSceneSessionManager::OnRegisterSingleHandContainerNode(napi_env env, napi_callback_info info)
{
    size_t argc = DEFAULT_ARG_COUNT;
    napi_value argv[DEFAULT_ARG_COUNT] = { nullptr };
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc < ARGC_ONE) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "Argc is invalid: %{public}zu", argc);
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    std::string stringId;
    if (!ConvertFromJsValue(env, argv[ARG_INDEX_ZERO], stringId)) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "Failed to convert parameter to stringId");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    SceneSessionManager::GetInstance().RegisterSingleHandContainerNode(stringId);
    return NapiGetUndefined(env);
}

std::unordered_map<int32_t, RotationChangeResult> JsSceneSessionManager::GetRotationChangeResult(
    const std::vector<sptr<SceneSession>>& activeSceneSessionMapCopy, const RotationChangeInfo& rotationChangeInfo,
    bool isRestrictNotify)
{
    std::unordered_map<int32_t, RotationChangeResult> rotationChangeResultMap;
    for (const auto& curSession : activeSceneSessionMapCopy) {
        if (curSession == nullptr) {
            TLOGE(WmsLogTag::WMS_ROTATION, "sesison is nullptr");
            continue;
        }
        RotationChangeResult rotationChangeResult =
            curSession->NotifyRotationChange(rotationChangeInfo, isRestrictNotify);
        if (rotationChangeResult.windowRect_.width_ != 0 && rotationChangeResult.windowRect_.height_ != 0) {
            rotationChangeResultMap[curSession->GetPersistentId()] = rotationChangeResult;
        }
    }
    return rotationChangeResultMap;
}

napi_value JsSceneSessionManager::OnNotifyRotationChange(napi_env env, napi_callback_info info)
{
    size_t argc = ARGC_FOUR;
    napi_value argv[ARGC_FOUR] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc != ARGC_TWO) {
        TLOGE(WmsLogTag::WMS_ROTATION, "Argc count is invalid: %{public}zu", argc);
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
                                      "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    RotationChangeInfo rotationChangeInfo;
    napi_value rotationChangeInfoObj = argv[0];
    if (rotationChangeInfoObj == nullptr) {
        TLOGE(WmsLogTag::WMS_ROTATION, "Failed to get rotationChangeInfoObj");
        return NapiGetUndefined(env);
    }
    if (!ConvertInfoFromJsValue(env, rotationChangeInfoObj, rotationChangeInfo)) {
        TLOGE(WmsLogTag::WMS_ROTATION, "Failed to convert parameter to rotationChangeInfo");
        return NapiGetUndefined(env);
    }
    bool isRestrictNotify = false;
    if (!ConvertFromJsValue(env, argv[1], isRestrictNotify)) {
        TLOGE(WmsLogTag::WMS_ROTATION, "Failed to convert parameter to isRestrictNotify");
        return NapiGetUndefined(env);
    }
    TLOGI(WmsLogTag::WMS_ROTATION, "info type: %{public}d, rect: [%{public}d, %{public}d, %{public}d, %{public}d], "
        "isRestrictNotify: %{public}d",
        static_cast<uint32_t>(rotationChangeInfo.type_), rotationChangeInfo.displayRect_.posX_,
        rotationChangeInfo.displayRect_.posY_, rotationChangeInfo.displayRect_.width_,
        rotationChangeInfo.displayRect_.height_, isRestrictNotify);

    std::vector<sptr<SceneSession>> activeSceneSessionMapCopy =
        SceneSessionManager::GetInstance().GetActiveSceneSessionCopy();
    if (activeSceneSessionMapCopy.empty()) {
        TLOGE(WmsLogTag::WMS_ROTATION, "activeSceneSessionMapCopy empty");
        return NapiGetUndefined(env);
    }
    std::unordered_map<int32_t, RotationChangeResult> rotationChangeResultMap =
        GetRotationChangeResult(activeSceneSessionMapCopy, rotationChangeInfo, isRestrictNotify);
    napi_value rotationChangeResultObj = CreateResultMapToJsValue(env, rotationChangeResultMap);
    if (rotationChangeResultObj == nullptr) {
        TLOGE(WmsLogTag::WMS_ROTATION, "Failed to convert rotationChangeResult to js value");
        return NapiGetUndefined(env);
    }
    return rotationChangeResultObj;
}

void JsSceneSessionManager::RegisterSceneSessionDestructCallback()
{
    SceneSessionManager::GetInstance().RegisterSceneSessionDestructCallback([this](int32_t persistentId) {
        this->OnSceneSessionDestruct(persistentId);
    });
}

void JsSceneSessionManager::RegisterUIEffectSetParamsCallback()
{
    UIEffectManager::GetInstance().RegisterUIEffectSetParamsCallback(
        [this](int32_t id, sptr<UIEffectParams> param) {
        this->OnUIEffectSetParams(id, param);
    });
}

void JsSceneSessionManager::RegisterUIEffectAnimateToCallback()
{
    UIEffectManager::GetInstance().RegisterUIEffectAnimateToCallback(
        [this](int32_t id, sptr<UIEffectParams> param, sptr<WindowAnimationOption> options,
            sptr<WindowAnimationOption> interruptOption) {
            this->OnUIEffectAnimateTo(id, param, options, interruptOption);
    });
}

void JsSceneSessionManager::OnUIEffectSetParams(int32_t id, sptr<UIEffectParams> param)
{
    const char* const where = __func__;
    taskScheduler_->PostMainThreadTask(
        [this, id, param, where, jsCallBack = GetJSCallback(UI_EFFECT_SET_PARAMS_CB), env = env_] {
            if (jsCallBack == nullptr || param == nullptr) {
                TLOGNE(WmsLogTag::WMS_ANIMATION, "%{public}s:jsCallBack or param is nullptr", where);
                return;
            }
            napi_value jsParam = nullptr;
            if (napi_status status = param->ConvertToJsValue(env, jsParam); status != napi_status::napi_ok) {
                TLOGNE(WmsLogTag::WMS_ANIMATION, "OnUIEffectSetParams trans failed with code %{public}d", status);
                return;
            }
            napi_value argv[] = { CreateJsValue(env, id), jsParam };
            napi_call_function(env, NapiGetUndefined(env), jsCallBack->GetNapiValue(), ArraySize(argv), argv, nullptr);
        }, __func__ + std::to_string(id));
}

void JsSceneSessionManager::OnUIEffectAnimateTo(int32_t id, sptr<UIEffectParams> param,
    sptr<WindowAnimationOption> option, sptr<WindowAnimationOption> interruptOption)
{
    const char* const where = __func__;
    taskScheduler_->PostMainThreadTask(
        [this, id, param, option, interruptOption, where,
            jsCallBack = GetJSCallback(UI_EFFECT_ANIMATE_TO_CB), env = env_] {
            if (jsCallBack == nullptr || param == nullptr || option == nullptr) {
                TLOGNE(WmsLogTag::WMS_ANIMATION, "%{public}s:jsCallBack or param or option is nullptr", where);
                return;
            }
            napi_value jsParam = nullptr;
            if (napi_status status = param->ConvertToJsValue(env, jsParam); status != napi_status::napi_ok) {
                TLOGNE(WmsLogTag::WMS_ANIMATION, "%{public}s:trans failed with code %{public}d", where, status);
                return;
            }
            napi_value jsOption = ConvertWindowAnimationOptionToJsValue(env, *option);
            if (!jsOption) {
                TLOGNE(WmsLogTag::WMS_ANIMATION, "%{public}s:jsOption is nullptr", where);
                return;
            }
            if (interruptOption) {
                napi_value jsInterruptOption = ConvertWindowAnimationOptionToJsValue(env, *interruptOption);
                if (!jsInterruptOption) {
                    TLOGNE(WmsLogTag::WMS_ANIMATION, "%{public}s:js interrupt option is nullptr", where);
                    return;
                }
                napi_value argv[] = { CreateJsValue(env, id), jsParam, jsOption, jsInterruptOption};
                napi_call_function(env, NapiGetUndefined(env), jsCallBack->GetNapiValue(),
                    ArraySize(argv), argv, nullptr);
                return;
            }
            napi_value argv[] = { CreateJsValue(env, id), jsParam, jsOption};
            napi_call_function(env, NapiGetUndefined(env), jsCallBack->GetNapiValue(), ArraySize(argv), argv, nullptr);
        }, __func__ + std::to_string(id));
}

void JsSceneSessionManager::OnSceneSessionDestruct(int32_t persistentId)
{
    taskScheduler_->PostMainThreadTask(
        [this, persistentId, jsCallBack = GetJSCallback(SCENE_SESSION_DESTRUCT_CB), env = env_] {
            if (jsCallBack == nullptr) {
                TLOGNE(WmsLogTag::WMS_LIFE, "jsCallBack is nullptr");
                return;
            }
            napi_value argv[] = { CreateJsValue(env, persistentId) };
            napi_call_function(env, NapiGetUndefined(env), jsCallBack->GetNapiValue(), ArraySize(argv), argv, nullptr);
        }, "OnSceneSessionDestruct, perisistentId: " + std::to_string(persistentId));
}

void JsSceneSessionManager::RegisterTransferSessionToTargetScreenCallback()
{
    SceneSessionManager::GetInstance().RegisterTransferSessionToTargetScreenCallback(
        [this](const TransferSessionInfo& info) {
            this->OnTransferSessionToTargetScreen(info);
    });
}

void JsSceneSessionManager::OnTransferSessionToTargetScreen(const TransferSessionInfo& info)
{
    TLOGI(WmsLogTag::WMS_LIFE, "in");
    const char* const where = __func__;
    taskScheduler_->PostMainThreadTask(
        [info, where, jsCallBack = GetJSCallback(SCENE_SESSION_TRANSFER_TO_TARGET_SCREEN_CB), env = env_] {
            if (jsCallBack == nullptr) {
                TLOGNE(WmsLogTag::WMS_LIFE, "%{public}s:jsCallBack is nullptr", where);
                return;
            }
            napi_value persistentId = CreateJsValue(env, info.persistentId);
            napi_value toScreenId = CreateJsValue(env, info.toScreenId);
            napi_value wantParams = OHOS::AppExecFwk::WrapWantParams(env, info.wantParams);
            napi_value argv[] = { persistentId, toScreenId, wantParams };
            napi_call_function(env, NapiGetUndefined(env), jsCallBack->GetNapiValue(), ArraySize(argv), argv, nullptr);
            SceneSessionManager::GetInstance().UpdateScreenLockState(info.persistentId);
        }, __func__);
}

napi_value JsSceneSessionManager::NotifySessionTransferToTargetScreenEvent(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::WMS_LIFE, "[NAPI]");
    JsSceneSessionManager* me = CheckParamsAndGetThis<JsSceneSessionManager>(env, info);
    return (me != nullptr) ? me->OnNotifySessionTransferToTargetScreenEvent(env, info) : nullptr;
}

napi_value JsSceneSessionManager::OnNotifySessionTransferToTargetScreenEvent(napi_env env, napi_callback_info info)
{
    size_t argc = ARGC_FIVE;
    napi_value argv[ARGC_FIVE] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc != ARGC_FIVE) {
        TLOGE(WmsLogTag::WMS_LIFE, "Argc is invalid: %{public}zu", argc);
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    int32_t persistentId;
    if (!ConvertFromJsValue(env, argv[ARG_INDEX_ZERO], persistentId)) {
        TLOGE(WmsLogTag::WMS_LIFE, "Failed to convert parameter to persistentId");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    uint32_t resultCode;
    if (!ConvertFromJsValue(env, argv[ARG_INDEX_ONE], resultCode)) {
        TLOGE(WmsLogTag::WMS_LIFE, "Failed to convert parameter to resultCode");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    int64_t fromScreenId;
    if (!ConvertFromJsValue(env, argv[ARG_INDEX_TWO], fromScreenId) || fromScreenId < 0) {
        TLOGE(WmsLogTag::WMS_LIFE, "Failed to convert parameter to fromScreenId");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    int64_t toScreenId;
    if (!ConvertFromJsValue(env, argv[ARG_INDEX_THREE], toScreenId) || toScreenId < 0) {
        TLOGE(WmsLogTag::WMS_LIFE, "Failed to convert parameter to toScreenId");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }

    LifeCycleChangeReason transferReason = LifeCycleChangeReason::DEFAULT;
    if (!ConvertFromJsValue(env, argv[ARG_INDEX_FOUR], transferReason)) {
        TLOGE(WmsLogTag::WMS_LIFE, "Failed to convert parameter to transferReason");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    TLOGI(WmsLogTag::WMS_LIFE, "persistentId: %{public}d, resultCode: %{public}d, "
        "fromScreenId: %{public}" PRId64 " , toScreenId: %{public}" PRId64 ", transferReason: %{public}u",
        persistentId, resultCode, fromScreenId, toScreenId, transferReason);
    SceneSessionManager::GetInstance().NotifySessionTransferToTargetScreenEvent(
        persistentId, resultCode, static_cast<uint64_t>(fromScreenId), static_cast<uint64_t>(toScreenId));
    return NapiGetUndefined(env);
}

napi_value JsSceneSessionManager::UpdateAppBoundSystemTrayStatus(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::WMS_MAIN, "[NAPI]");
    JsSceneSessionManager* me = CheckParamsAndGetThis<JsSceneSessionManager>(env, info);
    return (me != nullptr) ? me->OnUpdateAppBoundSystemTrayStatus(env, info) : nullptr;
}

napi_value JsSceneSessionManager::OnUpdateAppBoundSystemTrayStatus(napi_env env, napi_callback_info info)
{
    size_t argc = ARGC_THREE;
    napi_value argv[ARGC_THREE] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc != ARGC_THREE) {
        TLOGE(WmsLogTag::WMS_MAIN, "Argc is invalid: %{public}zu", argc);
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    std::string key;
    if (!ConvertFromJsValue(env, argv[ARG_INDEX_ZERO], key)) {
        TLOGE(WmsLogTag::WMS_MAIN, "Failed to convert key to %{public}s", key.c_str());
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    int32_t pid;
    if (!ConvertFromJsValue(env, argv[ARG_INDEX_ONE], pid)) {
        TLOGE(WmsLogTag::WMS_MAIN, "Failed to convert pid to %{public}d", pid);
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    bool enabled = false;
    if (!ConvertFromJsValue(env, argv[ARG_INDEX_TWO], enabled)) {
        TLOGE(WmsLogTag::WMS_MAIN, "Failed to convert enabled to %{public}d", enabled);
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    SceneSessionManager::GetInstance().UpdateAppBoundSystemTrayStatus(key, pid, enabled);
    return NapiGetUndefined(env);
}

napi_value JsSceneSessionManager::OnSupportFollowParentWindowLayout(napi_env env, napi_callback_info info)
{
    SceneSessionManager::GetInstance().ConfigSupportFollowParentWindowLayout();
    return NapiGetUndefined(env);
}

napi_value JsSceneSessionManager::OnUpdateRsCmdBlockingCount(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::WMS_LAYOUT, "in");
    size_t argc = ARGC_FOUR;
    napi_value argv[ARGC_FOUR] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc != ARGC_ONE) {
        TLOGD(WmsLogTag::WMS_LAYOUT, "Argc is invalid: %{public}zu", argc);
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
        "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    bool enable = false;
    if (!ConvertFromJsValue(env, argv[0], enable)) {
        TLOGD(WmsLogTag::WMS_LAYOUT, "Faile to convert parameter");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is invalid."));
        return NapiGetUndefined(env);
    }
    SceneSessionManager::GetInstance().UpdateRsCmdBlockingCount(enable);
    return NapiGetUndefined(env);
}

napi_value JsSceneSessionManager::OnSupportFollowRelativePositionToParent(napi_env env, napi_callback_info info)
{
    SceneSessionManager::GetInstance().ConfigSupportFollowRelativePositionToParent();
    return NapiGetUndefined(env);
}

napi_value JsSceneSessionManager::OnSupportZLevel(napi_env env, napi_callback_info info)
{
    SceneSessionManager::GetInstance().ConfigSupportZLevel();
    return NapiGetUndefined(env);
}

napi_value JsSceneSessionManager::OnSetSupportFunctionType(napi_env env, napi_callback_info info)
{
    size_t argc = ARGC_ONE;
    napi_value argv[ARGC_ONE] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc != ARGC_ONE) {
        TLOGE(WmsLogTag::WMS_KEYBOARD, "Argc count is invalid: %{public}zu", argc);
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    SupportFunctionType funcType;
    uint32_t funcTypeRawValue = 0;
    napi_value funcTypeObj = argv[0];
    if (funcTypeObj == nullptr) {
        TLOGE(WmsLogTag::WMS_KEYBOARD, "Failed to get funcTypeObj");
        return NapiGetUndefined(env);
    }
    if (napi_get_value_uint32(env, funcTypeObj, &funcTypeRawValue) != napi_ok) {
        TLOGE(WmsLogTag::WMS_KEYBOARD, "Failed to get funcTypeRawValue");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    funcType = static_cast<SupportFunctionType>(funcTypeRawValue);
    SceneSessionManager::GetInstance().ConfigSupportFunctionType(funcType);
    return NapiGetUndefined(env);
}

napi_value JsSceneSessionManager::UpdateRecentMainSessionInfos(napi_env env, napi_callback_info info)
{
    JsSceneSessionManager* me = CheckParamsAndGetThis<JsSceneSessionManager>(env, info);
    return (me != nullptr) ? me->OnUpdateRecentMainSessionInfos(env, info) : nullptr;
}

napi_value JsSceneSessionManager::OnUpdateRecentMainSessionInfos(napi_env env, napi_callback_info info)
{
    size_t argc = ARGC_FOUR;
    napi_value argv[ARGC_FOUR] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc != ARGC_ONE) {
        TLOGE(WmsLogTag::WMS_LIFE, "Argc count is invalid: %{public}zu", argc);
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    std::vector<int32_t> recentMainSessionIdList;
    if (!ConvertInt32ArrayFromJs(env, argv[0], recentMainSessionIdList)) {
        TLOGE(WmsLogTag::WMS_LIFE, "Failed to convert parameter to recentMainSessionIdList");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    SceneSessionManager::GetInstance().UpdateRecentMainSessionInfos(recentMainSessionIdList);
    return NapiGetUndefined(env);
}

napi_value JsSceneSessionManager::SetUIEffectControllerAliveInUI(napi_env env, napi_callback_info info)
{
    JsSceneSessionManager* me = CheckParamsAndGetThis<JsSceneSessionManager>(env, info);
    return (me != nullptr) ? me->OnSetUIEffectControllerAliveInUI(env, info) : nullptr;
}

napi_value JsSceneSessionManager::OnSetUIEffectControllerAliveInUI(napi_env env, napi_callback_info info)
{
    TLOGI(WmsLogTag::WMS_LIFE, "in");
    size_t argc = DEFAULT_ARG_COUNT;
    napi_value argv[DEFAULT_ARG_COUNT] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc != ARGC_TWO) {
        TLOGE(WmsLogTag::WMS_LIFE, "Argc count is invalid: %{public}zu", argc);
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    int32_t id = -1;
    if (!ConvertFromJsValue(env, argv[ARG_INDEX_ZERO], id)) {
        TLOGE(WmsLogTag::WMS_ANIMATION, "Failed to convert enabled to %{public}d", id);
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    bool alive = false;
    if (!ConvertFromJsValue(env, argv[ARG_INDEX_ONE], alive)) {
        TLOGE(WmsLogTag::WMS_ANIMATION, "Failed to convert enabled to %{public}d", alive);
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    UIEffectManager::GetInstance().SetUIEffectControllerAliveState(id, alive);
    return NapiGetUndefined(env);
}

void JsSceneSessionManager::RegisterUpdateKioskAppListCallback()
{
    TLOGI(WmsLogTag::WMS_LIFE, "in");
    SceneSessionManager::GetInstance().RegisterUpdateKioskAppListCallback(
        [this](const std::vector<std::string>& kioskAppList) { this->OnUpdateKioskAppListCallback(kioskAppList); });
}

static napi_value CreateUpdateKioskAppList(napi_env env, const std::vector<std::string>& kioskAppList)
{
    napi_value arrayValue = nullptr;
    napi_create_array_with_length(env, kioskAppList.size(), &arrayValue);
    if (arrayValue == nullptr) {
        TLOGE(WmsLogTag::WMS_LIFE, "Failed to create napi array");
        return NapiGetUndefined(env);
    }
    int32_t index = 0;
    for (const auto& bundleName : kioskAppList) {
        napi_value jsBundleNameObj = CreateJsValue(env, bundleName);
        napi_set_element(env, arrayValue, index++, jsBundleNameObj);
    }
    return arrayValue;
}

void JsSceneSessionManager::OnUpdateKioskAppListCallback(const std::vector<std::string>& kioskAppList)
{
    TLOGI(WmsLogTag::WMS_LIFE, "in");
    taskScheduler_->PostMainThreadTask([this, kioskAppList,
        jsCallBack = GetJSCallback(UPDATE_KIOSK_APP_LIST_CB), env = env_] {
            if (jsCallBack == nullptr) {
                TLOGNE(WmsLogTag::WMS_LIFE, "jsCallBack is nullptr");
                return;
            }
            napi_value kioskAppListValue = CreateUpdateKioskAppList(env, kioskAppList);
            napi_value argv[] = { kioskAppListValue };
            napi_call_function(env, NapiGetUndefined(env), jsCallBack->GetNapiValue(), ArraySize(argv), argv, nullptr);
        }, __func__);
}

void JsSceneSessionManager::OnKioskModeChangeCallback(bool isKioskMode, int32_t persistentId)
{
    taskScheduler_->PostMainThreadTask([this, isKioskMode, persistentId,
        jsCallBack = GetJSCallback(KIOSK_MODE_CHANGE_CB), env = env_] {
            if (jsCallBack == nullptr) {
                TLOGNE(WmsLogTag::WMS_LIFE, "jsCallBack is nullptr");
                return;
            }
            napi_value isKioskModeValue = CreateJsValue(env, isKioskMode);
            napi_value persistentIdValue = CreateJsValue(env, persistentId);
            napi_value argv[] = { isKioskModeValue, persistentIdValue };
            napi_call_function(env, NapiGetUndefined(env), jsCallBack->GetNapiValue(), ArraySize(argv), argv, nullptr);
        }, __func__);
}

void JsSceneSessionManager::RegisterKioskModeChangeCallback()
{
    TLOGI(WmsLogTag::WMS_LIFE, "in");
    SceneSessionManager::GetInstance().RegisterKioskModeChangeCallback(
        [this](bool isKioskMode, int32_t persistentId) { this->OnKioskModeChangeCallback(isKioskMode, persistentId); });
}

napi_value JsSceneSessionManager::SetPiPSettingSwitchStatus(napi_env env, napi_callback_info info)
{
    TLOGI(WmsLogTag::WMS_PIP, "in");
    JsSceneSessionManager* me = CheckParamsAndGetThis<JsSceneSessionManager>(env, info);
    return (me != nullptr) ? me->OnSetPiPSettingSwitchStatus(env, info) : nullptr;
}

napi_value JsSceneSessionManager::OnSetPiPSettingSwitchStatus(napi_env env, napi_callback_info info)
{
    size_t argc = ARGC_FOUR;
    napi_value argv[ARGC_FOUR] = { nullptr };
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc != ARGC_ONE) {
        TLOGE(WmsLogTag::WMS_PIP, "Argc is invalid: %{public}zu", argc);
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    bool switchStatus = true;
    if (!ConvertFromJsValue(env, argv[0], switchStatus)) {
        TLOGE(WmsLogTag::WMS_PIP, "Failed to convert parameter to switchStatus");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    SceneSessionManager::GetInstance().SetPiPSettingSwitchStatus(switchStatus);
    return NapiGetUndefined(env);
}

napi_value JsSceneSessionManager::SetIsPipEnabled(napi_env env, napi_callback_info info)
{
    TLOGI(WmsLogTag::WMS_PIP, "in");
    JsSceneSessionManager* me = CheckParamsAndGetThis<JsSceneSessionManager>(env, info);
    return (me != nullptr) ? me->OnSetIsPipEnabled(env, info) : nullptr;
}
 
napi_value JsSceneSessionManager::OnSetIsPipEnabled(napi_env env, napi_callback_info info)
{
    size_t argc = ARGC_ONE;
    napi_value argv[ARGC_ONE] = { nullptr };
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc != ARGC_ONE) {
        TLOGE(WmsLogTag::WMS_PIP, "Argc is invalid: %{public}zu", argc);
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    bool isPipEnabled  = false;
    if (!ConvertFromJsValue(env, argv[0], isPipEnabled)) {
        TLOGE(WmsLogTag::WMS_PIP, "Failed to convert parameter to isPipEnabled ");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    SceneSessionManager::GetInstance().SetIsPipEnabled(isPipEnabled);
    return NapiGetUndefined(env);
}

napi_value JsSceneSessionManager::GetPipDeviceCollaborationPolicy(napi_env env, napi_callback_info info)
{
    TLOGI(WmsLogTag::WMS_PIP, "in");
    JsSceneSessionManager* me = CheckParamsAndGetThis<JsSceneSessionManager>(env, info);
    return (me != nullptr) ? me->OnGetPipDeviceCollaborationPolicy(env, info) : nullptr;
}

napi_value JsSceneSessionManager::OnGetPipDeviceCollaborationPolicy(napi_env env, napi_callback_info info)
{
    size_t argc = ARGC_ONE;
    napi_value argv[ARGC_ONE] = { nullptr };
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc != ARGC_ONE) {
        TLOGE(WmsLogTag::WMS_PIP, "Argc is invalid: %{public}zu", argc);
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    int32_t screenId = -1;
    if (!ConvertFromJsValue(env, argv[ARG_INDEX_ZERO], screenId)) {
        TLOGE(WmsLogTag::WMS_PIP, "Failed to convert parameter to screenId, %{public}d", screenId);
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    TLOGI(WmsLogTag::WMS_PIP, "screenId  %{public}d", screenId);
    bool isPipEnabled = SceneSessionManager::GetInstance().GetPipDeviceCollaborationPolicy(screenId);
    return CreateJsValue(env, isPipEnabled);
}

void JsSceneSessionManager::RegisterVirtualPixelRatioChangeCallback()
{
    SceneSessionManager::GetInstance().RegisterVirtualPixelChangeCallback(
        [this](float density, DisplayId displayId) {
            this->OnVirtualPixelChange(density, displayId);
    });
}

void JsSceneSessionManager::OnVirtualPixelChange(float density, DisplayId displayId)
{
    taskScheduler_->PostMainThreadTask(
        [this, density, displayId, jsCallBack = GetJSCallback(VIRTUAL_DENSITY_CHANGE_CB), env = env_] {
            if (jsCallBack == nullptr) {
                TLOGNE(WmsLogTag::WMS_LIFE, "jsCallBack is nullptr");
                return;
            }
            napi_value densityValue = CreateJsValue(env, density);
            napi_value displayIdValue = CreateJsValue(env, static_cast<int64_t>(displayId));
            napi_value argv[] = { densityValue, displayIdValue };
            napi_call_function(env, NapiGetUndefined(env), jsCallBack->GetNapiValue(), ArraySize(argv), argv, nullptr);
        }, __func__);
}

napi_value JsSceneSessionManager::NotifySupportRotationChange(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::WMS_ROTATION, "[NAPI]");
    JsSceneSessionManager* me = CheckParamsAndGetThis<JsSceneSessionManager>(env, info);
    return (me != nullptr) ? me->OnNotifySupportRotationChange(env, info) : nullptr;
}

napi_value JsSceneSessionManager::OnNotifySupportRotationChange(napi_env env, napi_callback_info info)
{
    size_t argc = ARGC_FOUR;
    napi_value argv[ARGC_FOUR] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc != ARGC_ONE) {
        TLOGE(WmsLogTag::WMS_ROTATION, "Argc count is invalid: %{public}zu", argc);
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
                                      "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    SupportRotationInfo supportRotationInfo;
    napi_value supportRotationInfoObj = argv[0];
    if (supportRotationInfoObj == nullptr) {
        TLOGE(WmsLogTag::WMS_ROTATION, "Failed to get supportRotationInfoObj");
        return NapiGetUndefined(env);
    }
    if (!ConvertSupportRotationInfoFromJsValue(env, supportRotationInfoObj, supportRotationInfo)) {
        TLOGE(WmsLogTag::WMS_ROTATION, "Failed to convert parameter to supportRotationInfo");
        return NapiGetUndefined(env);
    }
    TLOGI(WmsLogTag::WMS_ROTATION, "persistentId: %{public}d, container size: %{public}zu, scene size: %{public}zu",
        supportRotationInfo.persistentId_, supportRotationInfo.containerSupportRotation_.size(),
        supportRotationInfo.sceneSupportRotation_.size());
    SceneSessionManager::GetInstance().NotifySupportRotationChange(supportRotationInfo);
    return NapiGetUndefined(env);
}

napi_value JsSceneSessionManager::GetAllJsonProfile(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::WMS_COMPAT, "[NAPI]");
    JsSceneSessionManager* me = CheckParamsAndGetThis<JsSceneSessionManager>(env, info);
    return (me != nullptr) ? me->OnGetAllJsonProfile(env, info) : nullptr;
}

napi_value JsSceneSessionManager::GetJsonProfile(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::WMS_COMPAT, "[NAPI]");
    JsSceneSessionManager* me = CheckParamsAndGetThis<JsSceneSessionManager>(env, info);
    return (me != nullptr) ? me->OnGetJsonProfile(env, info) : nullptr;
}

napi_value JsSceneSessionManager::OnGetAllJsonProfile(napi_env env, napi_callback_info info)
{
    size_t argc = ARGC_TWO;
    napi_value argv[ARGC_TWO] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc != ARGC_TWO) {
        TLOGE(WmsLogTag::WMS_COMPAT, "Argc count is invalid: %{public}zu", argc);
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    int32_t profileTypeId = -1;
    AppExecFwk::ProfileType profileType;
    if (!ConvertFromJsValue(env, argv[ARG_INDEX_ZERO], profileTypeId) ||
        profileTypeId > std::numeric_limits<uint8_t>::max() || profileTypeId < std::numeric_limits<int8_t>::min()) {
        TLOGE(WmsLogTag::WMS_COMPAT, "Failed to convert parameter to profileTypeId");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    } else {
        profileType = static_cast<AppExecFwk::ProfileType>(static_cast<int8_t>(profileTypeId));
    }
    int32_t userId = -1;
    if (!ConvertFromJsValue(env, argv[ARG_INDEX_ONE], userId)) {
        TLOGE(WmsLogTag::WMS_COMPAT, "Failed to convert parameter to userId");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    std::vector<AppExecFwk::JsonProfileInfo> profileInfos = {};
    SceneSessionManager::GetInstance().GetAllJsonProfile(profileType, userId, profileInfos);

    napi_value jsProfileInfoArray = nullptr;
    napi_create_array(env, &jsProfileInfoArray);
    for (size_t i = 0; i < profileInfos.size(); ++i) {
        napi_value jsProfileInfo = nullptr;
        napi_create_object(env, &jsProfileInfo);
        if (jsProfileInfo == nullptr) {
            TLOGE(WmsLogTag::WMS_COMPAT, "Object is null!");
            return NapiGetUndefined(env);
        }
        napi_set_named_property(env, jsProfileInfo, "profileType", CreateJsValue(env, profileInfos[i].profileType));
        napi_set_named_property(env, jsProfileInfo, "bundleName", CreateJsValue(env, profileInfos[i].bundleName));
        napi_set_named_property(env, jsProfileInfo, "moduleName", CreateJsValue(env, profileInfos[i].moduleName));
        napi_set_named_property(env, jsProfileInfo, "profile", CreateJsValue(env, profileInfos[i].profile));
        napi_set_element(env, jsProfileInfoArray, i, jsProfileInfo);
    }
    return jsProfileInfoArray;
}

napi_value JsSceneSessionManager::OnGetJsonProfile(napi_env env, napi_callback_info info)
{
    size_t argc = ARGC_FOUR;
    napi_value argv[ARGC_FOUR] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc != ARGC_FOUR) {
        TLOGE(WmsLogTag::WMS_COMPAT, "Argc count is invalid: %{public}zu", argc);
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    int32_t profileTypeId = -1;
    AppExecFwk::ProfileType profileType;
    if (!ConvertFromJsValue(env, argv[ARG_INDEX_ZERO], profileTypeId) ||
        profileTypeId > std::numeric_limits<uint8_t>::max() ||
        profileTypeId < std::numeric_limits<int8_t>::min()) {
        TLOGE(WmsLogTag::WMS_COMPAT, "Failed to convert parameter to profileTypeId");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    } else {
        profileType = static_cast<AppExecFwk::ProfileType>(static_cast<int8_t>(profileTypeId));
    }

    std::string bundleName = "";
    if (!ConvertFromJsValue(env, argv[ARG_INDEX_ONE], bundleName)) {
        TLOGE(WmsLogTag::WMS_COMPAT, "Failed to convert parameter to bundleName");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }

    std::string moduleName = "";
    if (!ConvertFromJsValue(env, argv[ARG_INDEX_TWO], moduleName)) {
        TLOGE(WmsLogTag::WMS_COMPAT, "Failed to convert parameter to moduleName");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }

    int32_t userId = -1;
    if (!ConvertFromJsValue(env, argv[ARG_INDEX_THREE], userId)) {
        TLOGE(WmsLogTag::WMS_COMPAT, "Failed to convert parameter to userId");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }

    std::string profileInfo = "";
    SceneSessionManager::GetInstance().GetJsonProfile(profileType, bundleName, moduleName, userId, profileInfo);
    napi_value result = nullptr;
    napi_create_string_utf8(env, profileInfo.c_str(), profileInfo.length(), &result);
    return result;
}

void JsSceneSessionManager::RegisterMinimizeAllCallback()
{
    TLOGND(WmsLogTag::WMS_LIFE, "RegisterMinimizeAllCallback called");
    SceneSessionManager::GetInstance().RegisterMinimizeAllCallback(
        [this](DisplayId displayId, int32_t excludeWindowId) {
        this->OnMinimizeAll(displayId, excludeWindowId);
    });
}

void JsSceneSessionManager::OnMinimizeAll(DisplayId displayId, int32_t excludeWindowId)
{
    TLOGNI(WmsLogTag::WMS_LIFE, "in");
    const char* const where = __func__;
    taskScheduler_->PostMainThreadTask([this, where, displayId, excludeWindowId,
        jsCallBack = GetJSCallback(MINIMIZE_ALL_CB), env = env_] {
        if (jsCallBack == nullptr) {
            TLOGNE(WmsLogTag::WMS_LIFE, "%{public}s, jsCallBack is nullptr", where);
            return;
        }
        napi_value jsDisplayIdObj = CreateJsNumber(env, static_cast<int64_t>(displayId));
        napi_value jsExcludeWindowIdObj = CreateJsValue(env, excludeWindowId);
        napi_value argv[] = { jsDisplayIdObj, jsExcludeWindowIdObj };
        napi_call_function(env, NapiGetUndefined(env), jsCallBack->GetNapiValue(), ArraySize(argv), argv, nullptr);
        }, __func__);
}
} // namespace OHOS::Rosen