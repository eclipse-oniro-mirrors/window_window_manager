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

#include "js_scene_utils.h"
#include "js_scene_session.h"

#include "session/host/include/session.h"
#include "session_manager/include/scene_session_manager.h"
#include "window_manager_hilog.h"
#include "common/include/session_permission.h"

namespace OHOS::Rosen {
using namespace AbilityRuntime;
namespace {
constexpr HiviewDFX::HiLogLabel LABEL = { LOG_CORE, HILOG_DOMAIN_WINDOW, "JsSceneSession" };
const std::string PENDING_SCENE_CB = "pendingSceneSessionActivation";
const std::string CHANGE_SESSION_VISIBILITY_WITH_STATUS_BAR = "changeSessionVisibilityWithStatusBar";
const std::string SESSION_STATE_CHANGE_CB = "sessionStateChange";
const std::string BUFFER_AVAILABLE_CHANGE_CB = "bufferAvailableChange";
const std::string SESSION_EVENT_CB = "sessionEvent";
const std::string SESSION_RECT_CHANGE_CB = "sessionRectChange";
const std::string SESSION_PIP_CONTROL_STATUS_CHANGE_CB = "sessionPiPControlStatusChange";
const std::string SESSION_AUTO_START_PIP_CB = "autoStartPiP";
const std::string CREATE_SUB_SESSION_CB = "createSpecificSession";
const std::string BIND_DIALOG_TARGET_CB = "bindDialogTarget";
const std::string RAISE_TO_TOP_CB = "raiseToTop";
const std::string RAISE_TO_TOP_POINT_DOWN_CB = "raiseToTopForPointDown";
const std::string CLICK_MODAL_SPECIFIC_WINDOW_OUTSIDE_CB = "clickModalSpecificWindowOutside";
const std::string BACK_PRESSED_CB = "backPressed";
const std::string SESSION_FOCUSABLE_CHANGE_CB = "sessionFocusableChange";
const std::string SESSION_TOUCHABLE_CHANGE_CB = "sessionTouchableChange";
const std::string SESSION_TOP_MOST_CHANGE_CB = "sessionTopmostChange";
const std::string SESSION_MODAL_TYPE_CHANGE_CB = "sessionModalTypeChange";
const std::string CLICK_CB = "click";
const std::string TERMINATE_SESSION_CB = "terminateSession";
const std::string TERMINATE_SESSION_CB_NEW = "terminateSessionNew";
const std::string TERMINATE_SESSION_CB_TOTAL = "terminateSessionTotal";
const std::string UPDATE_SESSION_LABEL_CB = "updateSessionLabel";
const std::string UPDATE_SESSION_ICON_CB = "updateSessionIcon";
const std::string SESSION_EXCEPTION_CB = "sessionException";
const std::string SYSTEMBAR_PROPERTY_CHANGE_CB = "systemBarPropertyChange";
const std::string NEED_AVOID_CB = "needAvoid";
const std::string PENDING_SESSION_TO_FOREGROUND_CB = "pendingSessionToForeground";
const std::string PENDING_SESSION_TO_BACKGROUND_FOR_DELEGATOR_CB = "pendingSessionToBackgroundForDelegator";
const std::string CUSTOM_ANIMATION_PLAYING_CB = "isCustomAnimationPlaying";
const std::string NEED_DEFAULT_ANIMATION_FLAG_CHANGE_CB = "needDefaultAnimationFlagChange";
const std::string SHOW_WHEN_LOCKED_CB = "sessionShowWhenLockedChange";
const std::string REQUESTED_ORIENTATION_CHANGE_CB = "sessionRequestedOrientationChange";
const std::string RAISE_ABOVE_TARGET_CB = "raiseAboveTarget";
const std::string FORCE_HIDE_CHANGE_CB = "sessionForceHideChange";
const std::string WINDOW_DRAG_HOT_AREA_CB = "windowDragHotArea";
const std::string TOUCH_OUTSIDE_CB = "touchOutside";
const std::string SESSIONINFO_LOCKEDSTATE_CHANGE_CB = "sessionInfoLockedStateChange";
const std::string PREPARE_CLOSE_PIP_SESSION = "prepareClosePiPSession";
const std::string LANDSCAPE_MULTI_WINDOW_CB = "landscapeMultiWindow";
const std::string CONTEXT_TRANSPARENT_CB = "contextTransparent";
const std::string KEYBOARD_GRAVITY_CHANGE_CB = "keyboardGravityChange";
const std::string ADJUST_KEYBOARD_LAYOUT_CB = "adjustKeyboardLayout";
const std::string LAYOUT_FULL_SCREEN_CB = "layoutFullScreenChange";
const std::string TITLE_DOCK_HOVER_SHOW_CB = "titleAndDockHoverShowChange";
const std::string RESTORE_MAIN_WINDOW_CB = "restoreMainWindow";
const std::string NEXT_FRAME_LAYOUT_FINISH_CB = "nextFrameLayoutFinish";
const std::string PRIVACY_MODE_CHANGE_CB = "privacyModeChange";
const std::string SESSION_MAIN_WINDOW_TOP_MOST_CHANGE_CB = "sessionMainWindowTopmostChange";
constexpr int ARG_COUNT_3 = 3;
constexpr int ARG_COUNT_4 = 4;
constexpr int ARG_INDEX_0 = 0;
constexpr int ARG_INDEX_1 = 1;
constexpr int ARG_INDEX_2 = 2;
constexpr int ARG_INDEX_3 = 3;

const std::map<std::string, ListenerFuncType> ListenerFuncMap {
    {PENDING_SCENE_CB,                      ListenerFuncType::PENDING_SCENE_CB},
    {CHANGE_SESSION_VISIBILITY_WITH_STATUS_BAR,
        ListenerFuncType::CHANGE_SESSION_VISIBILITY_WITH_STATUS_BAR},
    {SESSION_STATE_CHANGE_CB,               ListenerFuncType::SESSION_STATE_CHANGE_CB},
    {BUFFER_AVAILABLE_CHANGE_CB,            ListenerFuncType::BUFFER_AVAILABLE_CHANGE_CB},
    {SESSION_EVENT_CB,                      ListenerFuncType::SESSION_EVENT_CB},
    {SESSION_RECT_CHANGE_CB,                ListenerFuncType::SESSION_RECT_CHANGE_CB},
    {SESSION_PIP_CONTROL_STATUS_CHANGE_CB,  ListenerFuncType::SESSION_PIP_CONTROL_STATUS_CHANGE_CB},
    {SESSION_AUTO_START_PIP_CB,             ListenerFuncType::SESSION_AUTO_START_PIP_CB},
    {CREATE_SUB_SESSION_CB,                 ListenerFuncType::CREATE_SUB_SESSION_CB},
    {BIND_DIALOG_TARGET_CB,                 ListenerFuncType::BIND_DIALOG_TARGET_CB},
    {RAISE_TO_TOP_CB,                       ListenerFuncType::RAISE_TO_TOP_CB},
    {RAISE_TO_TOP_POINT_DOWN_CB,            ListenerFuncType::RAISE_TO_TOP_POINT_DOWN_CB},
    {CLICK_MODAL_SPECIFIC_WINDOW_OUTSIDE_CB,
        ListenerFuncType::CLICK_MODAL_SPECIFIC_WINDOW_OUTSIDE_CB},
    {BACK_PRESSED_CB,                       ListenerFuncType::BACK_PRESSED_CB},
    {SESSION_FOCUSABLE_CHANGE_CB,           ListenerFuncType::SESSION_FOCUSABLE_CHANGE_CB},
    {SESSION_TOUCHABLE_CHANGE_CB,           ListenerFuncType::SESSION_TOUCHABLE_CHANGE_CB},
    {SESSION_TOP_MOST_CHANGE_CB,            ListenerFuncType::SESSION_TOP_MOST_CHANGE_CB},
    {SESSION_MODAL_TYPE_CHANGE_CB,          ListenerFuncType::SESSION_MODAL_TYPE_CHANGE_CB},
    {CLICK_CB,                              ListenerFuncType::CLICK_CB},
    {TERMINATE_SESSION_CB,                  ListenerFuncType::TERMINATE_SESSION_CB},
    {TERMINATE_SESSION_CB_NEW,              ListenerFuncType::TERMINATE_SESSION_CB_NEW},
    {TERMINATE_SESSION_CB_TOTAL,            ListenerFuncType::TERMINATE_SESSION_CB_TOTAL},
    {SESSION_EXCEPTION_CB,                  ListenerFuncType::SESSION_EXCEPTION_CB},
    {UPDATE_SESSION_LABEL_CB,               ListenerFuncType::UPDATE_SESSION_LABEL_CB},
    {UPDATE_SESSION_ICON_CB,                ListenerFuncType::UPDATE_SESSION_ICON_CB},
    {SYSTEMBAR_PROPERTY_CHANGE_CB,          ListenerFuncType::SYSTEMBAR_PROPERTY_CHANGE_CB},
    {NEED_AVOID_CB,                         ListenerFuncType::NEED_AVOID_CB},
    {PENDING_SESSION_TO_FOREGROUND_CB,      ListenerFuncType::PENDING_SESSION_TO_FOREGROUND_CB},
    {PENDING_SESSION_TO_BACKGROUND_FOR_DELEGATOR_CB,
        ListenerFuncType::PENDING_SESSION_TO_BACKGROUND_FOR_DELEGATOR_CB},
    {CUSTOM_ANIMATION_PLAYING_CB,           ListenerFuncType::CUSTOM_ANIMATION_PLAYING_CB},
    {NEED_DEFAULT_ANIMATION_FLAG_CHANGE_CB, ListenerFuncType::NEED_DEFAULT_ANIMATION_FLAG_CHANGE_CB},
    {SHOW_WHEN_LOCKED_CB,                   ListenerFuncType::SHOW_WHEN_LOCKED_CB},
    {REQUESTED_ORIENTATION_CHANGE_CB,       ListenerFuncType::REQUESTED_ORIENTATION_CHANGE_CB},
    {RAISE_ABOVE_TARGET_CB,                 ListenerFuncType::RAISE_ABOVE_TARGET_CB},
    {FORCE_HIDE_CHANGE_CB,                  ListenerFuncType::FORCE_HIDE_CHANGE_CB},
    {WINDOW_DRAG_HOT_AREA_CB,               ListenerFuncType::WINDOW_DRAG_HOT_AREA_CB},
    {TOUCH_OUTSIDE_CB,                      ListenerFuncType::TOUCH_OUTSIDE_CB},
    {SESSIONINFO_LOCKEDSTATE_CHANGE_CB,     ListenerFuncType::SESSIONINFO_LOCKEDSTATE_CHANGE_CB},
    {PREPARE_CLOSE_PIP_SESSION,             ListenerFuncType::PREPARE_CLOSE_PIP_SESSION},
    {LANDSCAPE_MULTI_WINDOW_CB,             ListenerFuncType::LANDSCAPE_MULTI_WINDOW_CB},
    {CONTEXT_TRANSPARENT_CB,                ListenerFuncType::CONTEXT_TRANSPARENT_CB},
    {KEYBOARD_GRAVITY_CHANGE_CB,            ListenerFuncType::KEYBOARD_GRAVITY_CHANGE_CB},
    {ADJUST_KEYBOARD_LAYOUT_CB,             ListenerFuncType::ADJUST_KEYBOARD_LAYOUT_CB},
    {LAYOUT_FULL_SCREEN_CB,                 ListenerFuncType::LAYOUT_FULL_SCREEN_CB},
    {TITLE_DOCK_HOVER_SHOW_CB,              ListenerFuncType::TITLE_DOCK_HOVER_SHOW_CB},
    {NEXT_FRAME_LAYOUT_FINISH_CB,           ListenerFuncType::NEXT_FRAME_LAYOUT_FINISH_CB},
    {PRIVACY_MODE_CHANGE_CB,                ListenerFuncType::PRIVACY_MODE_CHANGE_CB},
    {RESTORE_MAIN_WINDOW_CB,                ListenerFuncType::RESTORE_MAIN_WINDOW_CB},
    {PRIVACY_MODE_CHANGE_CB,           ListenerFuncType::PRIVACY_MODE_CHANGE_CB},
    {SESSION_MAIN_WINDOW_TOP_MOST_CHANGE_CB, ListenerFuncType::SESSION_MAIN_WINDOW_TOP_MOST_CHANGE_CB},
};

const std::vector<std::string> g_syncGlobalPositionPermission {
    "Recent",
};
} // namespace

std::map<int32_t, napi_ref> JsSceneSession::jsSceneSessionMap_;

napi_value CreateJsPiPControlStatusObject(napi_env env, PiPControlStatusInfo controlStatusInfo)
{
    TLOGI(WmsLogTag::WMS_PIP, "in");
    napi_value objValue = nullptr;
    napi_create_object(env, &objValue);
    if (objValue == nullptr) {
        TLOGE(WmsLogTag::WMS_PIP, "objValue is nullptr");
        return NapiGetUndefined(env);
    }
    uint32_t controlType = static_cast<uint32_t>(controlStatusInfo.controlType);
    int32_t status = static_cast<int32_t>(controlStatusInfo.status);
    napi_set_named_property(env, objValue, "controlType", CreateJsValue(env, controlType));
    napi_set_named_property(env, objValue, "status", CreateJsValue(env, status));
    return objValue;
}

napi_value CreateJsPiPControlEnableObject(napi_env env, PiPControlEnableInfo controlEnableInfo)
{
    TLOGI(WmsLogTag::WMS_PIP, "in");
    napi_value objValue = nullptr;
    napi_create_object(env, &objValue);
    if (objValue == nullptr) {
        TLOGE(WmsLogTag::WMS_PIP, "objValue is nullptr");
        return NapiGetUndefined(env);
    }
    uint32_t controlType = static_cast<uint32_t>(controlEnableInfo.controlType);
    int32_t enabled = static_cast<int32_t>(controlEnableInfo.enabled);
    napi_set_named_property(env, objValue, "controlType", CreateJsValue(env, controlType));
    napi_set_named_property(env, objValue, "enabled", CreateJsValue(env, enabled));
    return objValue;
}

static napi_value CreatePipTemplateInfo(napi_env env, const sptr<SceneSession>& session)
{
    TLOGI(WmsLogTag::WMS_PIP, "in");
    napi_value pipTemplateInfoValue = nullptr;
    napi_create_object(env, &pipTemplateInfoValue);
    napi_set_named_property(env, pipTemplateInfoValue, "pipTemplateType",
        CreateJsValue(env, session->GetPiPTemplateInfo().pipTemplateType));
    napi_set_named_property(env, pipTemplateInfoValue, "priority",
        CreateJsValue(env, session->GetPiPTemplateInfo().priority));
    napi_value controlArrayValue = nullptr;
    std::vector<std::uint32_t> controlGroups = session->GetPiPTemplateInfo().controlGroup;
    napi_create_array_with_length(env, controlGroups.size(), &controlArrayValue);
    auto index = 0;
    for (const auto& controlGroup : controlGroups) {
        napi_set_element(env, controlArrayValue, index++, CreateJsValue(env, controlGroup));
    }
    napi_set_named_property(env, pipTemplateInfoValue, "controlGroup", controlArrayValue);
    napi_value controlStatusArrayValue = nullptr;
    std::vector<PiPControlStatusInfo> controlStatusInfoList = session->GetPiPTemplateInfo().pipControlStatusInfoList;
    napi_create_array_with_length(env, controlStatusInfoList.size(), &controlStatusArrayValue);
    auto controlStatusIndex = 0;
    for (const auto& controlStatus : controlStatusInfoList) {
        napi_set_element(env, controlStatusArrayValue, controlStatusIndex++,
            CreateJsPiPControlStatusObject(env, controlStatus));
    }
    napi_set_named_property(env, pipTemplateInfoValue, "pipControlStatusInfoList", controlStatusArrayValue);
    napi_value controlEnableArrayValue = nullptr;
    std::vector<PiPControlEnableInfo> controlEnableInfoList = session->GetPiPTemplateInfo().pipControlEnableInfoList;
    napi_create_array_with_length(env, controlEnableInfoList.size(), &controlEnableArrayValue);
    auto controlEnableIndex = 0;
    for (const auto& controlEnableInfo : controlEnableInfoList) {
        napi_set_element(env, controlEnableArrayValue, controlEnableIndex++,
            CreateJsPiPControlEnableObject(env, controlEnableInfo));
    }
    napi_set_named_property(env, pipTemplateInfoValue, "pipControlEnableInfoList", controlEnableArrayValue);
    return pipTemplateInfoValue;
}

static void SetWindowSize(napi_env env, napi_value objValue, const sptr<SceneSession>& session)
{
    auto abilityInfo = session->GetSessionInfo().abilityInfo;
    if (!abilityInfo) {
        return;
    }
    uint32_t value = 0;
    auto metadata = abilityInfo->metadata;
    for (auto item : metadata) {
        if (item.name == "ohos.ability.window.width") {
            if (GetIntValueFromString(item.value, value) == WSError::WS_OK) {
                TLOGI(WmsLogTag::WMS_LAYOUT, "ohos.ability.window.width = %{public}d", value);
                napi_set_named_property(env, objValue, "windowWidth", CreateJsValue(env, value));
            }
        } else if (item.name == "ohos.ability.window.height") {
            if (GetIntValueFromString(item.value, value) == WSError::WS_OK) {
                TLOGI(WmsLogTag::WMS_LAYOUT, "ohos.ability.window.height = %{public}d", value);
                napi_set_named_property(env, objValue, "windowHeight", CreateJsValue(env, value));
            }
        }
    }
}

napi_value JsSceneSession::Create(napi_env env, const sptr<SceneSession>& session)
{
    napi_value objValue = nullptr;
    napi_create_object(env, &objValue);
    if (objValue == nullptr || session == nullptr) {
        WLOGFE("[NAPI]Object or session is null!");
        return NapiGetUndefined(env);
    }

    sptr<JsSceneSession> jsSceneSession = sptr<JsSceneSession>::MakeSptr(env, session);
    jsSceneSession->IncStrongRef(nullptr);
    napi_wrap(env, objValue, jsSceneSession.GetRefPtr(), JsSceneSession::Finalizer, nullptr, nullptr);
    napi_set_named_property(env, objValue, "persistentId",
        CreateJsValue(env, static_cast<int32_t>(session->GetPersistentId())));
    napi_set_named_property(env, objValue, "parentId",
        CreateJsValue(env, static_cast<int32_t>(session->GetParentPersistentId())));
    napi_set_named_property(env, objValue, "type",
        CreateJsValue(env, static_cast<uint32_t>(GetApiType(session->GetWindowType()))));
    napi_set_named_property(env, objValue, "isAppType", CreateJsValue(env, session->IsFloatingWindowAppType()));
    napi_set_named_property(env, objValue, "pipTemplateInfo", CreatePipTemplateInfo(env, session));
    napi_set_named_property(env, objValue, "keyboardGravity",
        CreateJsValue(env, static_cast<int32_t>(session->GetKeyboardGravity())));
    napi_set_named_property(env, objValue, "isTopmost",
        CreateJsValue(env, static_cast<int32_t>(session->IsTopmost())));
    napi_set_named_property(env, objValue, "isMainWindowTopmost",
        CreateJsValue(env, static_cast<int32_t>(session->IsMainWindowTopmost())));
    napi_set_named_property(env, objValue, "subWindowModalType",
        CreateJsValue(env, static_cast<int32_t>(session->GetSubWindowModalType())));
    napi_set_named_property(env, objValue, "appInstanceKey",
        CreateJsValue(env, session->GetSessionInfo().appInstanceKey_));
    SetWindowSize(env, objValue, session);
    sptr<WindowSessionProperty> sessionProperty = session->GetSessionProperty();
    if (sessionProperty != nullptr) {
        napi_set_named_property(env, objValue, "screenId",
            CreateJsValue(env, static_cast<int32_t>(sessionProperty->GetDisplayId())));
    } else {
        napi_set_named_property(env, objValue, "screenId",
            CreateJsValue(env, static_cast<int32_t>(0)));
        TLOGE(WmsLogTag::WMS_LIFE, "sessionProperty is nullptr!");
    }
    const char* moduleName = "JsSceneSession";
    BindNativeMethod(env, objValue, moduleName);
    BindNativeMethodForKeyboard(env, objValue, moduleName);
    BindNativeMethodForCompatiblePcMode(env, objValue, moduleName);
    BindNativeMethodForSCBSystemSession(env, objValue, moduleName);
    BindNativeMethodForFocus(env, objValue, moduleName);
    napi_ref jsRef = nullptr;
    napi_status status = napi_create_reference(env, objValue, 1, &jsRef);
    if (status != napi_ok) {
        WLOGFE("get ref failed");
    }
    jsSceneSessionMap_[session->GetPersistentId()] = jsRef;
    BindNativeFunction(env, objValue, "updateSizeChangeReason", moduleName, JsSceneSession::UpdateSizeChangeReason);
    return objValue;
}

void JsSceneSession::BindNativeMethod(napi_env env, napi_value objValue, const char* moduleName)
{
    BindNativeFunction(env, objValue, "on", moduleName, JsSceneSession::RegisterCallback);
    BindNativeFunction(env, objValue, "updateNativeVisibility", moduleName, JsSceneSession::UpdateNativeVisibility);
    BindNativeFunction(env, objValue, "setShowRecent", moduleName, JsSceneSession::SetShowRecent);
    BindNativeFunction(env, objValue, "setZOrder", moduleName, JsSceneSession::SetZOrder);
    BindNativeFunction(env, objValue, "setTouchable", moduleName, JsSceneSession::SetTouchable);
    BindNativeFunction(env, objValue, "setRectChangeBySystem", moduleName, JsSceneSession::SetRectChangeBySystem);
    BindNativeFunction(env, objValue, "setSystemActive", moduleName, JsSceneSession::SetSystemActive);
    BindNativeFunction(env, objValue, "setPrivacyMode", moduleName, JsSceneSession::SetPrivacyMode);
    BindNativeFunction(env, objValue, "setSystemSceneOcclusionAlpha",
        moduleName, JsSceneSession::SetSystemSceneOcclusionAlpha);
    BindNativeFunction(env, objValue, "setSystemSceneForceUIFirst",
        moduleName, JsSceneSession::SetSystemSceneForceUIFirst);
    BindNativeFunction(env, objValue, "setFloatingScale", moduleName, JsSceneSession::SetFloatingScale);
    BindNativeFunction(env, objValue, "setIsMidScene", moduleName, JsSceneSession::SetIsMidScene);
    BindNativeFunction(env, objValue, "setScale", moduleName, JsSceneSession::SetScale);
    BindNativeFunction(env, objValue, "setWindowLastSafeRect", moduleName, JsSceneSession::SetWindowLastSafeRect);
    BindNativeFunction(env, objValue, "setMovable", moduleName, JsSceneSession::SetMovable);
    BindNativeFunction(env, objValue, "setSplitButtonVisible", moduleName, JsSceneSession::SetSplitButtonVisible);
    BindNativeFunction(env, objValue, "setOffset", moduleName, JsSceneSession::SetOffset);
    BindNativeFunction(env, objValue, "setExitSplitOnBackground", moduleName,
        JsSceneSession::SetExitSplitOnBackground);
    BindNativeFunction(env, objValue, "setWaterMarkFlag", moduleName, JsSceneSession::SetWaterMarkFlag);
    BindNativeFunction(env, objValue, "setPipActionEvent", moduleName, JsSceneSession::SetPipActionEvent);
    BindNativeFunction(env, objValue, "setPiPControlEvent", moduleName, JsSceneSession::SetPiPControlEvent);
    BindNativeFunction(env, objValue, "notifyDisplayStatusBarTemporarily", moduleName,
        JsSceneSession::NotifyDisplayStatusBarTemporarily);
    BindNativeFunction(env, objValue, "setTemporarilyShowWhenLocked", moduleName,
        JsSceneSession::SetTemporarilyShowWhenLocked);
    BindNativeFunction(env, objValue, "setSkipDraw", moduleName, JsSceneSession::SetSkipDraw);
    BindNativeFunction(env, objValue, "setSkipSelfWhenShowOnVirtualScreen", moduleName,
        JsSceneSession::SetSkipSelfWhenShowOnVirtualScreen);
    BindNativeFunction(env, objValue, "setCompatibleModeInPc", moduleName,
        JsSceneSession::SetCompatibleModeInPc);
    BindNativeFunction(env, objValue, "setUniqueDensityDpiFromSCB", moduleName,
        JsSceneSession::SetUniqueDensityDpiFromSCB);
    BindNativeFunction(env, objValue, "setBlankFlag", moduleName, JsSceneSession::SetBlankFlag);
    BindNativeFunction(env, objValue, "setBufferAvailableCallbackEnable", moduleName,
        JsSceneSession::SetBufferAvailableCallbackEnable);
    BindNativeFunction(env, objValue, "syncDefaultRequestedOrientation", moduleName,
        JsSceneSession::SyncDefaultRequestedOrientation);
    BindNativeFunction(env, objValue, "setIsPcAppInPad", moduleName,
        JsSceneSession::SetIsPcAppInPad);
    BindNativeFunction(env, objValue, "setCompatibleModeEnableInPad", moduleName,
        JsSceneSession::SetCompatibleModeEnableInPad);
    BindNativeFunction(env, objValue, "setStartingWindowExitAnimationFlag", moduleName,
        JsSceneSession::SetStartingWindowExitAnimationFlag);
}

void JsSceneSession::BindNativeMethodForKeyboard(napi_env env, napi_value objValue, const char* moduleName)
{
    BindNativeFunction(env, objValue, "setSCBKeepKeyboard", moduleName, JsSceneSession::SetSCBKeepKeyboard);
    BindNativeFunction(env, objValue, "requestHideKeyboard", moduleName, JsSceneSession::RequestHideKeyboard);
    BindNativeFunction(env, objValue, "openKeyboardSyncTransaction", moduleName,
        JsSceneSession::OpenKeyboardSyncTransaction);
    BindNativeFunction(env, objValue, "closeKeyboardSyncTransaction", moduleName,
        JsSceneSession::CloseKeyboardSyncTransaction);
    BindNativeFunction(env, objValue, "notifyTargetScreenWidthAndHeight", moduleName,
        JsSceneSession::NotifyTargetScreenWidthAndHeight);
}

void JsSceneSession::BindNativeMethodForCompatiblePcMode(napi_env env, napi_value objValue, const char* moduleName)
{
    BindNativeFunction(env, objValue, "setAppSupportPhoneInPc", moduleName,
        JsSceneSession::SetAppSupportPhoneInPc);
    BindNativeFunction(env, objValue, "setCompatibleWindowSizeInPc", moduleName,
        JsSceneSession::SetCompatibleWindowSizeInPc);
    BindNativeFunction(env, objValue, "compatibleFullScreenRecover", moduleName,
        JsSceneSession::CompatibleFullScreenRecover);
    BindNativeFunction(env, objValue, "compatibleFullScreenMinimize", moduleName,
        JsSceneSession::CompatibleFullScreenMinimize);
    BindNativeFunction(env, objValue, "compatibleFullScreenClose", moduleName,
        JsSceneSession::CompatibleFullScreenClose);
}

void JsSceneSession::BindNativeMethodForSCBSystemSession(napi_env env, napi_value objValue, const char* moduleName)
{
    BindNativeFunction(env, objValue, "syncScenePanelGlobalPosition", moduleName,
        JsSceneSession::SyncScenePanelGlobalPosition);
    BindNativeFunction(env, objValue, "unSyncScenePanelGlobalPosition", moduleName,
        JsSceneSession::UnSyncScenePanelGlobalPosition);
}

void JsSceneSession::BindNativeMethodForFocus(napi_env env, napi_value objValue, const char* moduleName)
{
    BindNativeFunction(env, objValue, "setFocusable", moduleName, JsSceneSession::SetFocusable);
    BindNativeFunction(env, objValue, "setFocusableOnShow", moduleName, JsSceneSession::SetFocusableOnShow);
    BindNativeFunction(env, objValue, "setSystemFocusable", moduleName, JsSceneSession::SetSystemFocusable);
    BindNativeFunction(env, objValue, "setSystemSceneBlockingFocus", moduleName,
        JsSceneSession::SetSystemSceneBlockingFocus);
}

JsSceneSession::JsSceneSession(napi_env env, const sptr<SceneSession>& session)
    : env_(env), weakSession_(session), persistentId_(session->GetPersistentId())
{
    auto sessionchangeCallback = sptr<SceneSession::SessionChangeCallback>::MakeSptr();
    session->RegisterSessionChangeCallback(sessionchangeCallback);
    sessionchangeCallback->clearCallbackFunc_ = [weakThis = wptr(this)](bool needRemove) {
        if (!needRemove) {
            TLOGND(WmsLogTag::WMS_LIFE, "clearCallbackFunc needRemove is false");
            return;
        }
        auto jsSceneSession = weakThis.promote();
        if (!jsSceneSession) {
            TLOGNE(WmsLogTag::WMS_LIFE, "clearCallbackFunc jsSceneSession is null");
            return;
        }
        jsSceneSession->ClearCbMap();
    };
    sessionchangeCallback_ = sessionchangeCallback;

    taskScheduler_ = std::make_shared<MainThreadScheduler>(env);
    TLOGI(WmsLogTag::WMS_LIFE, "created, id:%{public}d", persistentId_);
}

JsSceneSession::~JsSceneSession()
{
    TLOGI(WmsLogTag::WMS_LIFE, "destroyed, id:%{public}d", persistentId_);
    auto session = weakSession_.promote();
    if (session == nullptr) {
        TLOGI(WmsLogTag::WMS_LIFE, "session is nullptr, id:%{public}d", persistentId_);
        return;
    }
    session->UnregisterSessionChangeListeners();
    SceneSessionManager::GetInstance().UnregisterCreateSubSessionListener(session->GetPersistentId());
}

void JsSceneSession::ProcessPendingSceneSessionActivationRegister()
{
    NotifyPendingSessionActivationFunc func = [weakThis = wptr(this)](SessionInfo& info) {
        auto jsSceneSession = weakThis.promote();
        if (!jsSceneSession) {
            TLOGE(WmsLogTag::WMS_LIFE, "ProcessPendingSceneSessionActivationRegister jsSceneSession is null");
            return;
        }
        jsSceneSession->PendingSessionActivation(info);
    };
    auto session = weakSession_.promote();
    if (session == nullptr) {
        TLOGE(WmsLogTag::WMS_LIFE, "session is nullptr, id:%{public}d", persistentId_);
        return;
    }
    session->SetPendingSessionActivationEventListener(func);
    TLOGD(WmsLogTag::WMS_LIFE, "success");
}

void JsSceneSession::ProcessWindowDragHotAreaRegister()
{
    WLOGFI("[NAPI]");
    NotifyWindowDragHotAreaFunc func = [weakThis = wptr(this)](
        DisplayId displayId, uint32_t type, const SizeChangeReason reason) {
        auto jsSceneSession = weakThis.promote();
        if (!jsSceneSession) {
            TLOGE(WmsLogTag::WMS_LIFE, "ProcessWindowDragHotAreaRegister jsSceneSession is null");
            return;
        }
        jsSceneSession->OnWindowDragHotArea(displayId, type, reason);
    };
    auto session = weakSession_.promote();
    if (session == nullptr) {
        WLOGFE("session is nullptr, id:%{public}d", persistentId_);
        return;
    }
    session->SetWindowDragHotAreaListener(func);
}

void JsSceneSession::OnWindowDragHotArea(DisplayId displayId, uint32_t type, const SizeChangeReason reason)
{
    WLOGFI("[NAPI]");

    auto session = weakSession_.promote();
    if (session == nullptr) {
        WLOGFE("session is nullptr, id:%{public}d", persistentId_);
        return;
    }
    WSRect rect = session->GetSessionTargetRect();
    auto task = [weakThis = wptr(this), persistentId = persistentId_, env = env_, displayId, type, reason, rect] {
        auto jsSceneSession = weakThis.promote();
        if (!jsSceneSession || jsSceneSessionMap_.find(persistentId) == jsSceneSessionMap_.end()) {
            TLOGNE(WmsLogTag::WMS_LAYOUT, "OnWindowDragHotArea jsSceneSession id:%{public}d has been destroyed",
                persistentId);
            return;
        }
        auto jsCallBack = jsSceneSession->GetJSCallback(WINDOW_DRAG_HOT_AREA_CB);
        if (!jsCallBack) {
            TLOGNE(WmsLogTag::WMS_LAYOUT, "[NAPI]jsCallBack is nullptr");
            return;
        }
        napi_value jsHotAreaDisplayId = CreateJsValue(env, static_cast<int64_t>(displayId));
        if (jsHotAreaDisplayId == nullptr) {
            TLOGNE(WmsLogTag::WMS_LAYOUT, "[NAPI]jsHotAreaDisplayId is nullptr");
            return;
        }
        napi_value jsHotAreaType = CreateJsValue(env, type);
        if (jsHotAreaType == nullptr) {
            TLOGNE(WmsLogTag::WMS_LAYOUT, "[NAPI]jsHotAreaType is nullptr");
            return;
        }
        napi_value jsHotAreaReason = CreateJsValue(env, reason);
        if (jsHotAreaReason == nullptr) {
            TLOGNE(WmsLogTag::WMS_LAYOUT, "[NAPI]jsHotAreaReason is nullptr");
            return;
        }
        napi_value jsHotAreaRect = CreateJsSessionRect(env, rect);
        if (jsHotAreaRect == nullptr) {
            TLOGNE(WmsLogTag::WMS_LAYOUT, "[NAPI]jsHotAreaRect is nullptr");
            return;
        }
        napi_value argv[] = {jsHotAreaDisplayId, jsHotAreaType, jsHotAreaReason, jsHotAreaRect};
        napi_call_function(env, NapiGetUndefined(env), jsCallBack->GetNapiValue(), ArraySize(argv), argv, nullptr);
    };
    taskScheduler_->PostMainThreadTask(task, "OnWindowDragHotArea");
}

void JsSceneSession::ProcessSessionInfoLockedStateChangeRegister()
{
    NotifySessionInfoLockedStateChangeFunc func = [weakThis = wptr(this)](bool lockedState) {
        auto jsSceneSession = weakThis.promote();
        if (!jsSceneSession) {
            TLOGE(WmsLogTag::WMS_LIFE, "ProcessSessionInfoLockedStateChangeRegister jsSceneSession is null");
            return;
        }
        jsSceneSession->OnSessionInfoLockedStateChange(lockedState);
    };
    auto session = weakSession_.promote();
    if (session == nullptr) {
        WLOGFE("session is nullptr, id:%{public}d", persistentId_);
        return;
    }
    session->SetSessionInfoLockedStateChangeListener(func);
    WLOGFD("success");
}

void JsSceneSession::ProcessLandscapeMultiWindowRegister()
{
    TLOGD(WmsLogTag::WMS_MULTI_WINDOW, "in");
    auto sessionchangeCallback = sessionchangeCallback_.promote();
    if (sessionchangeCallback == nullptr) {
        TLOGE(WmsLogTag::WMS_MULTI_WINDOW, "sessionchangeCallback is nullptr");
        return;
    }
    sessionchangeCallback->onSetLandscapeMultiWindowFunc_ = [weakThis = wptr(this)](bool isLandscapeMultiWindow) {
        auto jsSceneSession = weakThis.promote();
        if (!jsSceneSession) {
            TLOGE(WmsLogTag::WMS_LIFE, "ProcessLandscapeMultiWindowRegister jsSceneSession is null");
            return;
        }
        jsSceneSession->SetLandscapeMultiWindow(isLandscapeMultiWindow);
    };
    TLOGD(WmsLogTag::WMS_MULTI_WINDOW, "success");
}

void JsSceneSession::SetLandscapeMultiWindow(bool isLandscapeMultiWindow)
{
    TLOGI(WmsLogTag::WMS_MULTI_WINDOW, "[NAPI]isLandscapeMultiWindow: %{public}u",
        isLandscapeMultiWindow);

    auto task = [weakThis = wptr(this), persistentId = persistentId_, isLandscapeMultiWindow, env = env_] {
        auto jsSceneSession = weakThis.promote();
        if (!jsSceneSession || jsSceneSessionMap_.find(persistentId) == jsSceneSessionMap_.end()) {
            TLOGE(WmsLogTag::WMS_LIFE, "SetLandscapeMultiWindow jsSceneSession id:%{public}d has been destroyed",
                persistentId);
            return;
        }
        auto jsCallBack = jsSceneSession->GetJSCallback(LANDSCAPE_MULTI_WINDOW_CB);
        if (!jsCallBack) {
            WLOGFE("[NAPI]jsCallBack is nullptr");
            return;
        }
        napi_value jsSessionLandscapeMultiWindowObj = CreateJsValue(env, isLandscapeMultiWindow);
        napi_value argv[] = {jsSessionLandscapeMultiWindowObj};
        napi_call_function(env, NapiGetUndefined(env), jsCallBack->GetNapiValue(), ArraySize(argv), argv, nullptr);
    };
    taskScheduler_->PostMainThreadTask(task,
        "SetLandscapeMultiWindow, isLandscapeMultiWindow:" + std::to_string(isLandscapeMultiWindow));
}

void JsSceneSession::ProcessKeyboardGravityChangeRegister()
{
    auto session = weakSession_.promote();
    if (session == nullptr) {
        WLOGFE("session is nullptr, id:%{public}d", persistentId_);
        return;
    }
    NotifyKeyboardGravityChangeFunc func = [weakThis = wptr(this)](SessionGravity gravity) {
        auto jsSceneSession = weakThis.promote();
        if (!jsSceneSession) {
            TLOGE(WmsLogTag::WMS_LIFE, "ProcessKeyboardGravityChangeRegister jsSceneSession is null");
            return;
        }
        jsSceneSession->OnKeyboardGravityChange(gravity);
    };
    session->SetKeyboardGravityChangeCallback(func);
    TLOGI(WmsLogTag::WMS_KEYBOARD, "success");
}

void JsSceneSession::OnKeyboardGravityChange(SessionGravity gravity)
{
    auto task = [weakThis = wptr(this), persistentId = persistentId_, gravity, env = env_] {
        auto jsSceneSession = weakThis.promote();
        if (!jsSceneSession || jsSceneSessionMap_.find(persistentId) == jsSceneSessionMap_.end()) {
            TLOGE(WmsLogTag::WMS_LIFE, "OnKeyboardGravityChange jsSceneSession id:%{public}d has been destroyed",
                persistentId);
            return;
        }
        auto jsCallBack = jsSceneSession->GetJSCallback(KEYBOARD_GRAVITY_CHANGE_CB);
        if (!jsCallBack) {
            WLOGFE("[NAPI]jsCallBack is nullptr");
            return;
        }
        napi_value gravityObj = CreateJsValue(env, gravity);
        napi_value argv[] = {gravityObj};
        napi_call_function(env, NapiGetUndefined(env), jsCallBack->GetNapiValue(), ArraySize(argv), argv, nullptr);
        TLOGD(WmsLogTag::WMS_KEYBOARD, "Napi call gravity success, gravity: %{public}u", gravity);
    };
    taskScheduler_->PostMainThreadTask(task, "OnKeyboardGravityChange: gravity " +
        std::to_string(static_cast<int>(gravity)));
}

void JsSceneSession::ProcessAdjustKeyboardLayoutRegister()
{
    NotifyKeyboardLayoutAdjustFunc func = [weakThis = wptr(this)](const KeyboardLayoutParams& params) {
        auto jsSceneSession = weakThis.promote();
        if (!jsSceneSession) {
            TLOGE(WmsLogTag::WMS_LIFE, "ProcessAdjustKeyboardLayoutRegister jsSceneSession is null");
            return;
        }
        jsSceneSession->OnAdjustKeyboardLayout(params);
    };
    auto session = weakSession_.promote();
    if (session == nullptr) {
        TLOGE(WmsLogTag::WMS_KEYBOARD, "session is nullptr");
        return;
    }
    session->SetAdjustKeyboardLayoutCallback(func);
    TLOGI(WmsLogTag::WMS_KEYBOARD, "success");
}

void JsSceneSession::ProcessLayoutFullScreenChangeRegister()
{
    auto sessionchangeCallback = sessionchangeCallback_.promote();
    if (sessionchangeCallback == nullptr) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "sessionchangeCallback is nullptr");
        return;
    }
    sessionchangeCallback->onLayoutFullScreenChangeFunc_ = [weakThis = wptr(this)](bool isLayoutFullScreen) {
        auto jsSceneSession = weakThis.promote();
        if (!jsSceneSession) {
            TLOGE(WmsLogTag::WMS_LIFE, "ProcessLayoutFullScreenChangeRegister jsSceneSession is null");
            return;
        }
        jsSceneSession->OnLayoutFullScreenChange(isLayoutFullScreen);
    };
    TLOGI(WmsLogTag::WMS_LAYOUT, "success");
}

void JsSceneSession::OnLayoutFullScreenChange(bool isLayoutFullScreen)
{
    auto task = [weakThis = wptr(this), persistentId = persistentId_, isLayoutFullScreen, env = env_] {
        auto jsSceneSession = weakThis.promote();
        if (!jsSceneSession || jsSceneSessionMap_.find(persistentId) == jsSceneSessionMap_.end()) {
            TLOGE(WmsLogTag::WMS_LIFE, "OnLayoutFullScreenChange jsSceneSession id:%{public}d has been destroyed",
                persistentId);
            return;
        }
        auto jsCallBack = jsSceneSession->GetJSCallback(LAYOUT_FULL_SCREEN_CB);
        if (!jsCallBack) {
            TLOGE(WmsLogTag::WMS_LAYOUT, "OnLayoutFullScreenChange jsCallBack is nullptr");
            return;
        }
        napi_value paramsObj = CreateJsValue(env, isLayoutFullScreen);
        napi_value argv[] = {paramsObj};
        napi_call_function(env, NapiGetUndefined(env), jsCallBack->GetNapiValue(), ArraySize(argv), argv, nullptr);
    };
    taskScheduler_->PostMainThreadTask(task, "OnLayoutFullScreenChange");
}

void JsSceneSession::ProcessTitleAndDockHoverShowChangeRegister()
{
    auto session = weakSession_.promote();
    if (session == nullptr) {
        TLOGE(WmsLogTag::WMS_IMMS, "session is nullptr, id:%{public}d", persistentId_);
        return;
    }
    NotifyTitleAndDockHoverShowChangeFunc func = [weakThis = wptr(this)](
        bool isTitleHoverShown, bool isDockHoverShown) {
        auto jsSceneSession = weakThis.promote();
        if (!jsSceneSession) {
            TLOGNE(WmsLogTag::WMS_IMMS, "jsSceneSession is null");
            return;
        }
        jsSceneSession->OnTitleAndDockHoverShowChange(isTitleHoverShown, isDockHoverShown);
    };
    session->SetTitleAndDockHoverShowChangeCallback(func);
    TLOGI(WmsLogTag::WMS_IMMS, "Register success, persistent id %{public}d", persistentId_);
}

void JsSceneSession::OnTitleAndDockHoverShowChange(bool isTitleHoverShown, bool isDockHoverShown)
{
    auto task = [weakThis = wptr(this), persistentId = persistentId_, isTitleHoverShown, isDockHoverShown, env = env_] {
        auto jsSceneSession = weakThis.promote();
        if (!jsSceneSession || jsSceneSessionMap_.find(persistentId) == jsSceneSessionMap_.end()) {
            TLOGNE(WmsLogTag::WMS_IMMS, "jsSceneSession id:%{public}d has been destroyed",
                persistentId);
            return;
        }
        auto jsCallBack = jsSceneSession->GetJSCallback(TITLE_DOCK_HOVER_SHOW_CB);
        if (!jsCallBack) {
            TLOGNE(WmsLogTag::WMS_IMMS, "jsCallBack is nullptr");
            return;
        }
        napi_value jsObjTitle = CreateJsValue(env, isTitleHoverShown);
        napi_value jsObjDock = CreateJsValue(env, isDockHoverShown);
        napi_value argv[] = {jsObjTitle, jsObjDock};
        napi_call_function(env, NapiGetUndefined(env), jsCallBack->GetNapiValue(), ArraySize(argv), argv, nullptr);
    };
    taskScheduler_->PostMainThreadTask(task, __func__);
}

void JsSceneSession::ProcessRestoreMainWindowRegister()
{
    auto session = weakSession_.promote();
    if (session == nullptr) {
        TLOGE(WmsLogTag::WMS_LIFE, "session is nullptr, id:%{public}d", persistentId_);
        return;
    }
    NotifyRestoreMainWindowFunc func = [weakThis = wptr(this)] {
        auto jsSceneSession = weakThis.promote();
        if (!jsSceneSession) {
            TLOGNE(WmsLogTag::WMS_LIFE, "%{public}s jsSceneSession is null", __func__);
            return;
        }
        jsSceneSession->RestoreMainWindow();
    };
    session->SetRestoreMainWindowCallback(func);
    TLOGI(WmsLogTag::WMS_LIFE, "success");
}

void JsSceneSession::RestoreMainWindow()
{
    const char* const funcName = __func__;
    auto task = [weakThis = wptr(this), persistentId = persistentId_, env = env_, funcName] {
        auto jsSceneSession = weakThis.promote();
        if (!jsSceneSession || jsSceneSessionMap_.find(persistentId) == jsSceneSessionMap_.end()) {
            TLOGNE(WmsLogTag::WMS_LIFE, "%{public}s jsSceneSession id:%{public}d has been destroyed",
                funcName, persistentId);
            return;
        }
        auto jsCallBack = jsSceneSession->GetJSCallback(RESTORE_MAIN_WINDOW_CB);
        if (!jsCallBack) {
            TLOGNE(WmsLogTag::WMS_LIFE, "%{public}s jsCallBack is nullptr", funcName);
            return;
        }
        napi_call_function(env, NapiGetUndefined(env), jsCallBack->GetNapiValue(), 0, {}, nullptr);
    };
    taskScheduler_->PostMainThreadTask(task, funcName);
}

void JsSceneSession::OnAdjustKeyboardLayout(const KeyboardLayoutParams& params)
{
    auto task = [weakThis = wptr(this), persistentId = persistentId_, params, env = env_] {
        auto jsSceneSession = weakThis.promote();
        if (!jsSceneSession || jsSceneSessionMap_.find(persistentId) == jsSceneSessionMap_.end()) {
            TLOGE(WmsLogTag::WMS_LIFE, "OnAdjustKeyboardLayout jsSceneSession id:%{public}d has been destroyed",
                persistentId);
            return;
        }
        auto jsCallBack = jsSceneSession->GetJSCallback(ADJUST_KEYBOARD_LAYOUT_CB);
        if (!jsCallBack) {
            TLOGE(WmsLogTag::WMS_KEYBOARD, "OnAdjustKeyboardLayout jsCallBack is nullptr");
            return;
        }
        napi_value keyboardLayoutParamsObj = CreateJsKeyboardLayoutParams(env, params);
        if (keyboardLayoutParamsObj == nullptr) {
            TLOGE(WmsLogTag::WMS_KEYBOARD, "OnAdjustKeyboardLayout this keyboard layout params obj is nullptr");
        }
        napi_value argv[] = {keyboardLayoutParamsObj};
        napi_call_function(env, NapiGetUndefined(env), jsCallBack->GetNapiValue(), ArraySize(argv), argv, nullptr);
    };
    taskScheduler_->PostMainThreadTask(task, "OnAdjustKeyboardLayout");
}

void JsSceneSession::OnSessionInfoLockedStateChange(bool lockedState)
{
    WLOGFI("[NAPI]state: %{public}u", lockedState);

    auto task = [weakThis = wptr(this), persistentId = persistentId_, lockedState, env = env_] {
        auto jsSceneSession = weakThis.promote();
        if (!jsSceneSession || jsSceneSessionMap_.find(persistentId) == jsSceneSessionMap_.end()) {
            TLOGE(WmsLogTag::WMS_LIFE, "OnSessionInfoLockedStateChange jsSceneSession id:%{public}d has been destroyed",
                persistentId);
            return;
        }
        auto jsCallBack = jsSceneSession->GetJSCallback(SESSIONINFO_LOCKEDSTATE_CHANGE_CB);
        if (!jsCallBack) {
            WLOGFE("[NAPI]jsCallBack is nullptr");
            return;
        }
        napi_value jsSessionInfoLockedStateObj = CreateJsValue(env, lockedState);
        napi_value argv[] = {jsSessionInfoLockedStateObj};
        napi_call_function(env, NapiGetUndefined(env), jsCallBack->GetNapiValue(), ArraySize(argv), argv, nullptr);
    };
    taskScheduler_->PostMainThreadTask(task, "OnSessionInfoLockedStateChange: state " + std::to_string(lockedState));
}

void JsSceneSession::ClearCbMap()
{
    const char* const where = __func__;
    auto task = [weakThis = wptr(this), where] {
        auto jsSceneSession = weakThis.promote();
        if (!jsSceneSession) {
            TLOGNE(WmsLogTag::WMS_LIFE, "%{public}s: jsSceneSession is null", where);
            return;
        }
        TLOGNI(WmsLogTag::WMS_LIFE, "%{public}s: persistent id %{public}d", where, jsSceneSession->persistentId_);
        {
            HITRACE_METER_FMT(HITRACE_TAG_WINDOW_MANAGER, "JsSceneSession clear jsCbMap");
            std::unique_lock<std::shared_mutex> lock(jsSceneSession->jsCbMapMutex_);
            jsSceneSession->jsCbMap_.clear();
        }
        // delete native reference
        if (auto iter = jsSceneSessionMap_.find(jsSceneSession->persistentId_); iter != jsSceneSessionMap_.end()) {
            napi_delete_reference(jsSceneSession->env_, iter->second);
            jsSceneSessionMap_.erase(iter);
        } else {
            TLOGNE(WmsLogTag::WMS_LIFE, "%{public}s: delete ref failed, %{public}d",
                   where, jsSceneSession->persistentId_);
        }
    };
    taskScheduler_->PostMainThreadTask(task, "ClearCbMap PID:" + std::to_string(persistentId_));
}

void JsSceneSession::ProcessSessionDefaultAnimationFlagChangeRegister()
{
    auto session = weakSession_.promote();
    if (session == nullptr) {
        WLOGFE("session is nullptr, id:%{public}d", persistentId_);
        return;
    }
    session->RegisterDefaultAnimationFlagChangeCallback([weakThis = wptr(this)](bool isNeedDefaultAnimationFlag) {
        auto jsSceneSession = weakThis.promote();
        if (!jsSceneSession) {
            TLOGE(WmsLogTag::WMS_LIFE, "ProcessSessionDefaultAnimationFlagChangeRegister jsSceneSession is null");
            return;
        }
        jsSceneSession->OnDefaultAnimationFlagChange(isNeedDefaultAnimationFlag);
    });
}

void JsSceneSession::OnDefaultAnimationFlagChange(bool isNeedDefaultAnimationFlag)
{
    WLOGFI("[NAPI]flag: %{public}u", isNeedDefaultAnimationFlag);

    auto task = [weakThis = wptr(this), persistentId = persistentId_, isNeedDefaultAnimationFlag, env = env_] {
        auto jsSceneSession = weakThis.promote();
        if (!jsSceneSession || jsSceneSessionMap_.find(persistentId) == jsSceneSessionMap_.end()) {
            TLOGE(WmsLogTag::WMS_LIFE, "OnDefaultAnimationFlagChange jsSceneSession id:%{public}d has been destroyed",
                persistentId);
            return;
        }
        auto jsCallBack = jsSceneSession->GetJSCallback(NEED_DEFAULT_ANIMATION_FLAG_CHANGE_CB);
        if (!jsCallBack) {
            WLOGFE("[NAPI]jsCallBack is nullptr");
            return;
        }
        napi_value jsSessionDefaultAnimationFlagObj = CreateJsValue(env, isNeedDefaultAnimationFlag);
        napi_value argv[] = {jsSessionDefaultAnimationFlagObj};
        napi_call_function(env, NapiGetUndefined(env), jsCallBack->GetNapiValue(), ArraySize(argv), argv, nullptr);
    };
    std::string info = "OnDefaultAnimationFlagChange, flag:" + std::to_string(isNeedDefaultAnimationFlag);
    taskScheduler_->PostMainThreadTask(task, info);
}

void JsSceneSession::ProcessChangeSessionVisibilityWithStatusBarRegister()
{
    NotifyChangeSessionVisibilityWithStatusBarFunc func = [weakThis = wptr(this)](SessionInfo& info, bool visible) {
        auto jsSceneSession = weakThis.promote();
        if (!jsSceneSession) {
            TLOGE(WmsLogTag::WMS_LIFE, "ProcessChangeSessionVisibilityWithStatusBarRegister jsSceneSession is null");
            return;
        }
        jsSceneSession->ChangeSessionVisibilityWithStatusBar(info, visible);
    };
    auto session = weakSession_.promote();
    if (session == nullptr) {
        WLOGFE("session is nullptr, id:%{public}d", persistentId_);
        return;
    }
    session->SetChangeSessionVisibilityWithStatusBarEventListener(func);
    WLOGFD("success");
}

void JsSceneSession::ProcessSessionStateChangeRegister()
{
    NotifySessionStateChangeFunc func = [weakThis = wptr(this)](const SessionState& state) {
        auto jsSceneSession = weakThis.promote();
        if (!jsSceneSession) {
            TLOGE(WmsLogTag::WMS_LIFE, "ProcessSessionStateChangeRegister jsSceneSession is null");
            return;
        }
        jsSceneSession->OnSessionStateChange(state);
    };
    auto session = weakSession_.promote();
    if (session == nullptr) {
        WLOGFE("session is nullptr, id:%{public}d", persistentId_);
        return;
    }
    session->SetSessionStateChangeListenser(func);
    WLOGFD("success");
}

void JsSceneSession::ProcessBufferAvailableChangeRegister()
{
    NotifyBufferAvailableChangeFunc func = [weakThis = wptr(this)](const bool isAvailable) {
        auto jsSceneSession = weakThis.promote();
        if (!jsSceneSession) {
            TLOGE(WmsLogTag::WMS_LIFE, "ProcessBufferAvailableChangeRegister jsSceneSession is null");
            return;
        }
        jsSceneSession->OnBufferAvailableChange(isAvailable);
    };
    auto session = weakSession_.promote();
    if (session == nullptr) {
        WLOGFE("session is nullptr, id:%{public}d", persistentId_);
        return;
    }
    session->SetBufferAvailableChangeListener(func);
    WLOGFD("success");
}

void JsSceneSession::ProcessCreateSubSessionRegister()
{
    NotifyCreateSubSessionFunc func = [weakThis = wptr(this)](const sptr<SceneSession>& sceneSession) {
        auto jsSceneSession = weakThis.promote();
        if (!jsSceneSession) {
            TLOGE(WmsLogTag::WMS_LIFE, "ProcessCreateSubSessionRegister jsSceneSession is null");
            return;
        }
        jsSceneSession->OnCreateSubSession(sceneSession);
    };
    auto session = weakSession_.promote();
    if (session == nullptr) {
        WLOGFE("session is nullptr, id:%{public}d", persistentId_);
        return;
    }
    SceneSessionManager::GetInstance().RegisterCreateSubSessionListener(session->GetPersistentId(), func);
    WLOGFD("success, id: %{public}d", session->GetPersistentId());
}

void JsSceneSession::ProcessBindDialogTargetRegister()
{
    auto sessionchangeCallback = sessionchangeCallback_.promote();
    if (sessionchangeCallback == nullptr) {
        TLOGE(WmsLogTag::WMS_DIALOG, "sessionchangeCallback is nullptr");
        return;
    }
    sessionchangeCallback->onBindDialogTarget_ = [weakThis = wptr(this)](const sptr<SceneSession>& sceneSession) {
        auto jsSceneSession = weakThis.promote();
        if (!jsSceneSession) {
            TLOGE(WmsLogTag::WMS_LIFE, "ProcessBindDialogTargetRegister jsSceneSession is null");
            return;
        }
        jsSceneSession->OnBindDialogTarget(sceneSession);
    };
    TLOGD(WmsLogTag::WMS_DIALOG, "success");
}

void JsSceneSession::ProcessSessionRectChangeRegister()
{
    NotifySessionRectChangeFunc func = [weakThis = wptr(this)](const WSRect& rect,
        const SizeChangeReason reason, const DisplayId displayId = DISPLAY_ID_INVALID) {
        auto jsSceneSession = weakThis.promote();
        if (!jsSceneSession) {
            TLOGE(WmsLogTag::WMS_LIFE, "ProcessSessionRectChangeRegister jsSceneSession is null");
            return;
        }
        jsSceneSession->OnSessionRectChange(rect, reason, displayId);
    };
    auto session = weakSession_.promote();
    if (session == nullptr) {
        WLOGFE("session is nullptr, id:%{public}d", persistentId_);
        return;
    }
    session->SetSessionRectChangeCallback(func);
    WLOGFD("success");
}

void JsSceneSession::ProcessSessionPiPControlStatusChangeRegister()
{
    NotifySessionPiPControlStatusChangeFunc func = [weakThis = wptr(this)](
        WsPiPControlType controlType, WsPiPControlStatus status) {
        auto jsSceneSession = weakThis.promote();
        if (!jsSceneSession) {
            TLOGE(WmsLogTag::WMS_LIFE, "ProcessSessionPiPControlStatusChangeRegister jsSceneSession is null");
            return;
        }
        jsSceneSession->OnSessionPiPControlStatusChange(controlType, status);
    };
    auto session = weakSession_.promote();
    if (session == nullptr) {
        TLOGE(WmsLogTag::WMS_PIP, "session is nullptr, id:%{public}d", persistentId_);
        return;
    }
    session->SetSessionPiPControlStatusChangeCallback(func);
    TLOGI(WmsLogTag::WMS_PIP, "success");
}

void JsSceneSession::ProcessAutoStartPiPStatusChangeRegister()
{
    auto session = weakSession_.promote();
    if (session == nullptr) {
        TLOGE(WmsLogTag::WMS_PIP, "session is nullptr, id:%{public}d", persistentId_);
        return;
    }
    NotifyAutoStartPiPStatusChangeFunc func = [weakThis = wptr(this)](bool isAutoStart) {
        auto jsSceneSession = weakThis.promote();
        if (!jsSceneSession) {
            TLOGNE(WmsLogTag::WMS_PIP, "jsSceneSession is null");
            return;
        }
        jsSceneSession->OnAutoStartPiPStatusChange(isAutoStart);
    };
    session->SetAutoStartPiPStatusChangeCallback(func);
    TLOGI(WmsLogTag::WMS_PIP, "success");
}

/** @note @window.hierarchy */
void JsSceneSession::ProcessRaiseToTopRegister()
{
    auto sessionchangeCallback = sessionchangeCallback_.promote();
    if (sessionchangeCallback == nullptr) {
        WLOGFE("sessionchangeCallback is nullptr");
        return;
    }
    sessionchangeCallback->onRaiseToTop_ = [weakThis = wptr(this)] {
        auto jsSceneSession = weakThis.promote();
        if (!jsSceneSession) {
            TLOGE(WmsLogTag::WMS_LIFE, "ProcessRaiseToTopRegister jsSceneSession is null");
            return;
        }
        jsSceneSession->OnRaiseToTop();
    };
    WLOGFD("success");
}

/** @note @window.hierarchy */
void JsSceneSession::ProcessRaiseToTopForPointDownRegister()
{
    NotifyRaiseToTopForPointDownFunc func = [weakThis = wptr(this)]() {
        auto jsSceneSession = weakThis.promote();
        if (!jsSceneSession) {
            TLOGE(WmsLogTag::WMS_LIFE, "ProcessRaiseToTopForPointDownRegister jsSceneSession is null");
            return;
        }
        jsSceneSession->OnRaiseToTopForPointDown();
    };
    auto session = weakSession_.promote();
    if (session == nullptr) {
        WLOGFE("session is nullptr, id:%{public}d", persistentId_);
        return;
    }
    session->SetRaiseToAppTopForPointDownFunc(func);
    WLOGFD("success");
}

void JsSceneSession::ProcessClickModalSpecificWindowOutsideRegister()
{
    NotifyClickModalSpecificWindowOutsideFunc func = [weakThis = wptr(this)] {
        auto jsSceneSession = weakThis.promote();
        if (!jsSceneSession) {
            TLOGE(WmsLogTag::WMS_LIFE, "ProcessClickModalSpecificWindowOutsideRegister jsSceneSession is null");
            return;
        }
        jsSceneSession->OnClickModalSpecificWindowOutside();
    };
    auto session = weakSession_.promote();
    if (session == nullptr) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "session is nullptr, id:%{public}d", persistentId_);
        return;
    }
    session->SetClickModalSpecificWindowOutsideListener(func);
    TLOGD(WmsLogTag::WMS_LAYOUT, "success");
}

/** @note @window.hierarchy */
void JsSceneSession::ProcessRaiseAboveTargetRegister()
{
    auto sessionchangeCallback = sessionchangeCallback_.promote();
    if (sessionchangeCallback == nullptr) {
        WLOGFE("sessionchangeCallback is nullptr");
        return;
    }
    sessionchangeCallback->onRaiseAboveTarget_ = [weakThis = wptr(this)](int32_t subWindowId) {
        auto jsSceneSession = weakThis.promote();
        if (!jsSceneSession) {
            TLOGE(WmsLogTag::WMS_LIFE, "ProcessRaiseAboveTargetRegister jsSceneSession is null");
            return;
        }
        jsSceneSession->OnRaiseAboveTarget(subWindowId);
    };
    WLOGFD("success");
}

void JsSceneSession::ProcessSessionEventRegister()
{
    auto sessionchangeCallback = sessionchangeCallback_.promote();
    if (sessionchangeCallback == nullptr) {
        WLOGFE("sessionchangeCallback is nullptr");
        return;
    }
    sessionchangeCallback->OnSessionEvent_ = [weakThis = wptr(this)](uint32_t eventId, const SessionEventParam& param) {
        auto jsSceneSession = weakThis.promote();
        if (!jsSceneSession) {
            TLOGE(WmsLogTag::WMS_LIFE, "ProcessSessionEventRegister jsSceneSession is null");
            return;
        }
        jsSceneSession->OnSessionEvent(eventId, param);
    };
    WLOGFD("success");
}

void JsSceneSession::ProcessTerminateSessionRegister()
{
    TLOGD(WmsLogTag::WMS_LIFE, "in");
    NotifyTerminateSessionFunc func = [weakThis = wptr(this)](const SessionInfo& info) {
        auto jsSceneSession = weakThis.promote();
        if (!jsSceneSession) {
            TLOGE(WmsLogTag::WMS_LIFE, "ProcessTerminateSessionRegister jsSceneSession is null");
            return;
        }
        jsSceneSession->TerminateSession(info);
    };
    auto session = weakSession_.promote();
    if (session == nullptr) {
        TLOGE(WmsLogTag::WMS_LIFE, "session is nullptr, id:%{public}d", persistentId_);
        return;
    }
    session->SetTerminateSessionListener(func);
    TLOGD(WmsLogTag::WMS_LIFE, "success");
}

void JsSceneSession::ProcessTerminateSessionRegisterNew()
{
    TLOGD(WmsLogTag::WMS_LIFE, "in");
    NotifyTerminateSessionFuncNew func = [weakThis = wptr(this)](
        const SessionInfo& info, bool needStartCaller, bool isFromBroker) {
        auto jsSceneSession = weakThis.promote();
        if (!jsSceneSession) {
            TLOGE(WmsLogTag::WMS_LIFE, "ProcessTerminateSessionRegisterNew jsSceneSession is null");
            return;
        }
        jsSceneSession->TerminateSessionNew(info, needStartCaller, isFromBroker);
    };
    auto session = weakSession_.promote();
    if (session == nullptr) {
        TLOGE(WmsLogTag::WMS_LIFE, "session is nullptr, id:%{public}d", persistentId_);
        return;
    }
    session->SetTerminateSessionListenerNew(func);
    TLOGD(WmsLogTag::WMS_LIFE, "success");
}

void JsSceneSession::ProcessTerminateSessionRegisterTotal()
{
    TLOGD(WmsLogTag::WMS_LIFE, "in");
    NotifyTerminateSessionFuncTotal func = [weakThis = wptr(this)](
        const SessionInfo& info, TerminateType terminateType) {
        auto jsSceneSession = weakThis.promote();
        if (!jsSceneSession) {
            TLOGE(WmsLogTag::WMS_LIFE, "ProcessTerminateSessionRegisterTotal jsSceneSession is null");
            return;
        }
        jsSceneSession->TerminateSessionTotal(info, terminateType);
    };
    auto session = weakSession_.promote();
    if (session == nullptr) {
        TLOGE(WmsLogTag::WMS_LIFE, "session is nullptr, id:%{public}d", persistentId_);
        return;
    }
    session->SetTerminateSessionListenerTotal(func);
    TLOGD(WmsLogTag::WMS_LIFE, "success");
}

void JsSceneSession::ProcessPendingSessionToForegroundRegister()
{
    TLOGD(WmsLogTag::WMS_LIFE, "in");
    NotifyPendingSessionToForegroundFunc func = [weakThis = wptr(this)](const SessionInfo& info) {
        auto jsSceneSession = weakThis.promote();
        if (!jsSceneSession) {
            TLOGE(WmsLogTag::WMS_LIFE, "ProcessPendingSessionToForegroundRegister jsSceneSession is null");
            return;
        }
        jsSceneSession->PendingSessionToForeground(info);
    };
    auto session = weakSession_.promote();
    if (session == nullptr) {
        TLOGE(WmsLogTag::WMS_LIFE, "session is nullptr, id:%{public}d", persistentId_);
        return;
    }
    session->SetPendingSessionToForegroundListener(func);
    TLOGD(WmsLogTag::WMS_LIFE, "success");
}

void JsSceneSession::ProcessPendingSessionToBackgroundForDelegatorRegister()
{
    TLOGD(WmsLogTag::WMS_LIFE, "in");
    NotifyPendingSessionToBackgroundForDelegatorFunc func = [weakThis = wptr(this)](const SessionInfo& info,
        bool shouldBackToCaller) {
        auto jsSceneSession = weakThis.promote();
        if (!jsSceneSession) {
            TLOGNE(WmsLogTag::WMS_LIFE, "jsSceneSession is null");
            return;
        }
        jsSceneSession->PendingSessionToBackgroundForDelegator(info, shouldBackToCaller);
    };
    auto session = weakSession_.promote();
    if (session == nullptr) {
        TLOGE(WmsLogTag::WMS_LIFE, "session is nullptr, id:%{public}d", persistentId_);
        return;
    }
    session->SetPendingSessionToBackgroundForDelegatorListener(func);
    TLOGD(WmsLogTag::WMS_LIFE, "success");
}

void JsSceneSession::ProcessSessionExceptionRegister()
{
    WLOGFD("in");
    NotifySessionExceptionFunc func = [weakThis = wptr(this)](const SessionInfo& info, bool needRemoveSession) {
        auto jsSceneSession = weakThis.promote();
        if (!jsSceneSession) {
            TLOGE(WmsLogTag::WMS_LIFE, "ProcessSessionExceptionRegister jsSceneSession is null");
            return;
        }
        jsSceneSession->OnSessionException(info, needRemoveSession);
    };
    auto session = weakSession_.promote();
    if (session == nullptr) {
        WLOGFE("session is nullptr, id:%{public}d", persistentId_);
        return;
    }
    session->SetSessionExceptionListener(func, true);
    WLOGFD("success");
}

/** @note @window.hierarchy */
void JsSceneSession::ProcessSessionTopmostChangeRegister()
{
    auto sessionchangeCallback = sessionchangeCallback_.promote();
    if (sessionchangeCallback == nullptr) {
        TLOGE(WmsLogTag::WMS_HIERARCHY, "sessionchangeCallback is nullptr");
        return;
    }
    sessionchangeCallback->onSessionTopmostChange_ = [weakThis = wptr(this)](bool topmost) {
        auto jsSceneSession = weakThis.promote();
        if (!jsSceneSession) {
            TLOGE(WmsLogTag::WMS_LIFE, "ProcessSessionTopmostChangeRegister jsSceneSession is null");
            return;
        }
        jsSceneSession->OnSessionTopmostChange(topmost);
    };
    auto session = weakSession_.promote();
    if (session == nullptr) {
        TLOGE(WmsLogTag::WMS_HIERARCHY, "session is nullptr, id:%{public}d", persistentId_);
        return;
    }
    sessionchangeCallback->onSessionTopmostChange_(session->IsTopmost());
    TLOGD(WmsLogTag::WMS_HIERARCHY, "success");
}

/** @note @window.hierarchy */
void JsSceneSession::ProcessMainWindowTopmostChangeRegister()
{
    NotifyMainWindowTopmostChangeFunc func = [weakThis = wptr(this)](bool isTopmost) {
        auto jsSceneSession = weakThis.promote();
        if (!jsSceneSession) {
            TLOGNE(WmsLogTag::WMS_HIERARCHY, "jsSceneSession is null");
            return;
        }
        jsSceneSession->OnMainWindowTopmostChange(isTopmost);
    };
    auto session = weakSession_.promote();
    if (session == nullptr) {
        TLOGE(WmsLogTag::WMS_HIERARCHY, "session is nullptr, id:%{public}d", persistentId_);
        return;
    }
    session->SetMainWindowTopmostChangeCallback(func);
    TLOGD(WmsLogTag::WMS_HIERARCHY, "register success");
}

void JsSceneSession::ProcessSessionModalTypeChangeRegister()
{
    const char* const where = __func__;
    NotifySessionModalTypeChangeFunc func =
        [weakThis = wptr(this), where](SubWindowModalType subWindowModalType) {
        auto jsSceneSession = weakThis.promote();
        if (!jsSceneSession) {
            TLOGNE(WmsLogTag::WMS_LIFE, "%{public}s jsSceneSession is null", where);
            return;
        }
        jsSceneSession->OnSessionModalTypeChange(subWindowModalType);
    };
    auto session = weakSession_.promote();
    if (session == nullptr) {
        TLOGE(WmsLogTag::WMS_HIERARCHY, "session is nullptr, persistent id:%{public}d", persistentId_);
        return;
    }
    session->SetSessionModalTypeChangeCallback(func);
    TLOGD(WmsLogTag::WMS_HIERARCHY, "register success, persistent id:%{public}d", persistentId_);
}

void JsSceneSession::ProcessSessionFocusableChangeRegister()
{
    NotifySessionFocusableChangeFunc func = [weakThis = wptr(this)](bool isFocusable) {
        auto jsSceneSession = weakThis.promote();
        if (!jsSceneSession) {
            TLOGE(WmsLogTag::WMS_LIFE, "ProcessSessionFocusableChangeRegister jsSceneSession is null");
            return;
        }
        jsSceneSession->OnSessionFocusableChange(isFocusable);
    };
    auto session = weakSession_.promote();
    if (session == nullptr) {
        WLOGFE("[WMSComm]session is nullptr, id:%{public}d", persistentId_);
        return;
    }
    session->SetSessionFocusableChangeListener(func);
    TLOGD(WmsLogTag::WMS_FOCUS, "success");
}

void JsSceneSession::ProcessSessionTouchableChangeRegister()
{
    NotifySessionTouchableChangeFunc func = [weakThis = wptr(this)](bool touchable) {
        auto jsSceneSession = weakThis.promote();
        if (!jsSceneSession) {
            TLOGE(WmsLogTag::WMS_LIFE, "ProcessSessionTouchableChangeRegister jsSceneSession is null");
            return;
        }
        jsSceneSession->OnSessionTouchableChange(touchable);
    };
    auto session = weakSession_.promote();
    if (session == nullptr) {
        WLOGFE("session is nullptr, id:%{public}d", persistentId_);
        return;
    }
    session->SetSessionTouchableChangeListener(func);
    WLOGFD("success");
}

void JsSceneSession::ProcessClickRegister()
{
    NotifyClickFunc func = [weakThis = wptr(this)](bool requestFocus) {
        auto jsSceneSession = weakThis.promote();
        if (!jsSceneSession) {
            TLOGE(WmsLogTag::WMS_LIFE, "ProcessClickRegister jsSceneSession is null");
            return;
        }
        jsSceneSession->OnClick(requestFocus);
    };
    auto session = weakSession_.promote();
    if (session == nullptr) {
        WLOGFE("session is nullptr, id:%{public}d", persistentId_);
        return;
    }
    session->SetClickListener(func);
    WLOGFD("success");
}

void JsSceneSession::ProcessContextTransparentRegister()
{
    NotifyContextTransparentFunc func = [weakThis = wptr(this)]() {
        auto jsSceneSession = weakThis.promote();
        if (!jsSceneSession) {
            TLOGE(WmsLogTag::WMS_LIFE, "ProcessContextTransparentRegister jsSceneSession is null");
            return;
        }
        jsSceneSession->OnContextTransparent();
    };
    auto session = weakSession_.promote();
    if (session == nullptr) {
        WLOGFE("session is nullptr, id:%{public}d", persistentId_);
        return;
    }
    session->SetContextTransparentFunc(func);
    WLOGFD("success");
}

void JsSceneSession::OnSessionEvent(uint32_t eventId, const SessionEventParam& param)
{
    WLOGFI("[NAPI]eventId: %{public}d", eventId);

    auto task = [weakThis = wptr(this), persistentId = persistentId_, eventId, param, env = env_] {
        auto jsSceneSession = weakThis.promote();
        if (!jsSceneSession || jsSceneSessionMap_.find(persistentId) == jsSceneSessionMap_.end()) {
            TLOGE(WmsLogTag::WMS_LIFE, "OnSessionEvent jsSceneSession id:%{public}d has been destroyed",
                persistentId);
            return;
        }
        auto jsCallBack = jsSceneSession->GetJSCallback(SESSION_EVENT_CB);
        if (jsCallBack == nullptr) {
            WLOGFE("[NAPI]jsCallBack is nullptr");
            return;
        }
        napi_value jsSessionStateObj = CreateJsValue(env, eventId);
        napi_value jsSessionParamObj = CreateJsSessionEventParam(env, param);
        napi_value argv[] = {jsSessionStateObj, jsSessionParamObj};
        napi_call_function(env, NapiGetUndefined(env), jsCallBack->GetNapiValue(), ArraySize(argv), argv, nullptr);
    };
    std::unique_ptr<NapiAsyncTask::ExecuteCallback> execute = nullptr;
    taskScheduler_->PostMainThreadTask(task, "OnSessionEvent, EventId:" + std::to_string(eventId));
}

void JsSceneSession::ProcessBackPressedRegister()
{
    NotifyBackPressedFunc func = [weakThis = wptr(this)](bool needMoveToBackground) {
        auto jsSceneSession = weakThis.promote();
        if (!jsSceneSession) {
            TLOGE(WmsLogTag::WMS_LIFE, "ProcessBackPressedRegister jsSceneSession is null");
            return;
        }
        jsSceneSession->OnBackPressed(needMoveToBackground);
    };
    auto session = weakSession_.promote();
    if (session == nullptr) {
        WLOGFE("session is nullptr, id:%{public}d", persistentId_);
        return;
    }
    session->SetBackPressedListenser(func);
}

void JsSceneSession::ProcessSystemBarPropertyChangeRegister()
{
    auto session = weakSession_.promote();
    if (session == nullptr) {
        TLOGE(WmsLogTag::WMS_IMMS, "session is null, id:%{public}d", persistentId_);
        return;
    }
    session->RegisterSystemBarPropertyChangeCallback([weakThis = wptr(this)](
        const std::unordered_map<WindowType, SystemBarProperty>& propertyMap) {
        auto jsSceneSession = weakThis.promote();
        if (!jsSceneSession) {
            TLOGNE(WmsLogTag::WMS_LIFE, "jsSceneSession is null");
            return;
        }
        jsSceneSession->OnSystemBarPropertyChange(propertyMap);
    });
}

void JsSceneSession::ProcessNeedAvoidRegister()
{
    auto sessionchangeCallback = sessionchangeCallback_.promote();
    if (sessionchangeCallback == nullptr) {
        TLOGE(WmsLogTag::WMS_IMMS, "sessionchangeCallback is nullptr");
        return;
    }
    sessionchangeCallback->OnNeedAvoid_ = [weakThis = wptr(this)](bool status) {
        auto jsSceneSession = weakThis.promote();
        if (!jsSceneSession) {
            TLOGE(WmsLogTag::WMS_LIFE, "ProcessNeedAvoidRegister jsSceneSession is null");
            return;
        }
        jsSceneSession->OnNeedAvoid(status);
    };
    TLOGD(WmsLogTag::WMS_IMMS, "success");
}

void JsSceneSession::ProcessIsCustomAnimationPlaying()
{
    auto sessionchangeCallback = sessionchangeCallback_.promote();
    if (sessionchangeCallback == nullptr) {
        WLOGFE("sessionchangeCallback is nullptr");
        return;
    }
    sessionchangeCallback->onIsCustomAnimationPlaying_ = [weakThis = wptr(this)](bool status) {
        auto jsSceneSession = weakThis.promote();
        if (!jsSceneSession) {
            TLOGE(WmsLogTag::WMS_LIFE, "ProcessIsCustomAnimationPlaying jsSceneSession is null");
            return;
        }
        jsSceneSession->OnIsCustomAnimationPlaying(status);
    };
    WLOGFD("success");
}

void JsSceneSession::ProcessShowWhenLockedRegister()
{
    auto sessionchangeCallback = sessionchangeCallback_.promote();
    if (sessionchangeCallback == nullptr) {
        WLOGFE("sessionchangeCallback is nullptr");
        return;
    }
    sessionchangeCallback->OnShowWhenLocked_ = [weakThis = wptr(this)](bool showWhenLocked) {
        auto jsSceneSession = weakThis.promote();
        if (!jsSceneSession) {
            TLOGE(WmsLogTag::WMS_LIFE, "ProcessShowWhenLockedRegister jsSceneSession is null");
            return;
        }
        jsSceneSession->OnShowWhenLocked(showWhenLocked);
    };
    auto session = weakSession_.promote();
    if (session == nullptr) {
        WLOGFE("session is nullptr, id:%{public}d", persistentId_);
        return;
    }
    sessionchangeCallback->OnShowWhenLocked_(session->GetShowWhenLockedFlagValue());
    WLOGFD("success");
}

void JsSceneSession::ProcessRequestedOrientationChange()
{
    auto sessionchangeCallback = sessionchangeCallback_.promote();
    if (sessionchangeCallback == nullptr) {
        WLOGFE("sessionchangeCallback is nullptr");
        return;
    }
    sessionchangeCallback->OnRequestedOrientationChange_ = [weakThis = wptr(this)](uint32_t orientation) {
        auto jsSceneSession = weakThis.promote();
        if (!jsSceneSession) {
            TLOGE(WmsLogTag::WMS_LIFE, "ProcessRequestedOrientationChange jsSceneSession is null");
            return;
        }
        jsSceneSession->OnReuqestedOrientationChange(orientation);
    };
    WLOGFD("success");
}

void JsSceneSession::ProcessForceHideChangeRegister()
{
    auto sessionchangeCallback = sessionchangeCallback_.promote();
    if (sessionchangeCallback == nullptr) {
        WLOGFE("sessionchangeCallback is nullptr");
        return;
    }
    sessionchangeCallback->OnForceHideChange_ = [weakThis = wptr(this)](bool hide) {
        auto jsSceneSession = weakThis.promote();
        if (!jsSceneSession) {
            TLOGE(WmsLogTag::WMS_LIFE, "ProcessForceHideChangeRegister jsSceneSession is null");
            return;
        }
        jsSceneSession->OnForceHideChange(hide);
    };
    WLOGFD("success");
}

void JsSceneSession::OnForceHideChange(bool hide)
{
    WLOGFI("[NAPI]hide: %{public}u", hide);

    auto task = [weakThis = wptr(this), persistentId = persistentId_, hide, env = env_] {
        auto jsSceneSession = weakThis.promote();
        if (!jsSceneSession || jsSceneSessionMap_.find(persistentId) == jsSceneSessionMap_.end()) {
            TLOGE(WmsLogTag::WMS_LIFE, "OnForceHideChange jsSceneSession id:%{public}d has been destroyed",
                persistentId);
            return;
        }
        auto jsCallBack = jsSceneSession->GetJSCallback(FORCE_HIDE_CHANGE_CB);
        if (!jsCallBack) {
            WLOGFE("[NAPI]jsCallBack is nullptr");
            return;
        }
        napi_value jsSessionForceHideObj = CreateJsValue(env, hide);
        napi_value argv[] = {jsSessionForceHideObj};
        napi_call_function(env, NapiGetUndefined(env), jsCallBack->GetNapiValue(), ArraySize(argv), argv, nullptr);
    };
    taskScheduler_->PostMainThreadTask(task, "OnForceHideChange, hide:" + std::to_string(hide));
}

void JsSceneSession::ProcessTouchOutsideRegister()
{
    auto sessionchangeCallback = sessionchangeCallback_.promote();
    if (sessionchangeCallback == nullptr) {
        WLOGFE("sessionchangeCallback is nullptr");
        return;
    }
    sessionchangeCallback->OnTouchOutside_ = [weakThis = wptr(this)] {
        auto jsSceneSession = weakThis.promote();
        if (!jsSceneSession) {
            TLOGE(WmsLogTag::WMS_LIFE, "ProcessTouchOutsideRegister jsSceneSession is null");
            return;
        }
        jsSceneSession->OnTouchOutside();
    };
    WLOGFD("success");
}

void JsSceneSession::OnTouchOutside()
{
    WLOGFI("[NAPI]");
    auto task = [weakThis = wptr(this), persistentId = persistentId_, env = env_] {
        auto jsSceneSession = weakThis.promote();
        if (!jsSceneSession || jsSceneSessionMap_.find(persistentId) == jsSceneSessionMap_.end()) {
            TLOGE(WmsLogTag::WMS_LIFE, "OnTouchOutside jsSceneSession id:%{public}d has been destroyed",
                persistentId);
            return;
        }
        auto jsCallBack = jsSceneSession->GetJSCallback(TOUCH_OUTSIDE_CB);
        if (!jsCallBack) {
            WLOGFE("[NAPI]jsCallBack is nullptr");
            return;
        }
        napi_value argv[] = {};
        napi_call_function(env, NapiGetUndefined(env), jsCallBack->GetNapiValue(), 0, argv, nullptr);
    };
    taskScheduler_->PostMainThreadTask(task);
}

void JsSceneSession::ProcessFrameLayoutFinishRegister()
{
    NotifyFrameLayoutFinishFunc func = [weakThis = wptr(this)]() {
        auto jsSceneSession = weakThis.promote();
        if (!jsSceneSession) {
            TLOGE(WmsLogTag::WMS_LIFE, "ProcessFrameLayoutFinishRegister jsSceneSession is null");
            return;
        }
        jsSceneSession->NotifyFrameLayoutFinish();
    };
    auto session = weakSession_.promote();
    if (session == nullptr) {
        TLOGI(WmsLogTag::WMS_MULTI_WINDOW, "session is nullptr");
        return;
    }
    session->SetFrameLayoutFinishListener(func);
    TLOGD(WmsLogTag::WMS_MULTI_WINDOW, "success");
}

void JsSceneSession::NotifyFrameLayoutFinish()
{
    TLOGI(WmsLogTag::WMS_MULTI_WINDOW, "[NAPI]");
    auto session = weakSession_.promote();
    if (session == nullptr) {
        TLOGE(WmsLogTag::WMS_MULTI_WINDOW, "session is nullptr");
        return;
    }
    auto task = [weakThis = wptr(this), persistentId = persistentId_, env = env_] {
        auto jsSceneSession = weakThis.promote();
        if (!jsSceneSession || jsSceneSessionMap_.find(persistentId) == jsSceneSessionMap_.end()) {
            TLOGE(WmsLogTag::WMS_MULTI_WINDOW, "jsSceneSession id:%{public}d has been destroyed", persistentId);
            return;
        }
        auto jsCallBack = jsSceneSession->GetJSCallback(NEXT_FRAME_LAYOUT_FINISH_CB);
        if (!jsCallBack) {
            TLOGE(WmsLogTag::WMS_MULTI_WINDOW, "[NAPI]jsCallBack is nullptr");
            return;
        }
        napi_value argv[] = {};
        napi_call_function(env, NapiGetUndefined(env), jsCallBack->GetNapiValue(), 0, argv, nullptr);
    };
    taskScheduler_->PostMainThreadTask(task, "NotifyFrameLayoutFinish");
}

void JsSceneSession::ProcessPrivacyModeChangeRegister()
{
    NotifyPrivacyModeChangeFunc func = [this](bool isPrivacyMode) {
        this->NotifyPrivacyModeChange(isPrivacyMode);
    };
    auto session = weakSession_.promote();
    if (session == nullptr) {
        TLOGE(WmsLogTag::WMS_SCB, "session is nullptr");
        return;
    }
    session->SetPrivacyModeChangeNotifyFunc(func);
}

void JsSceneSession::NotifyPrivacyModeChange(bool isPrivacyMode)
{
    TLOGI(WmsLogTag::WMS_SCB, "isPrivacyMode:%{public}d, id:%{public}d", isPrivacyMode, persistentId_);
    auto task = [this, isPrivacyMode, env = env_]() {
        auto jsCallback = this->GetJSCallback(PRIVACY_MODE_CHANGE_CB);
        if (!jsCallback) {
            TLOGE(WmsLogTag::WMS_SCB, "jsCallback is nullptr");
            return;
        }
        napi_value jsSessionPrivacyMode = CreateJsValue(env, isPrivacyMode);
        napi_value argv[] = { jsSessionPrivacyMode };
        napi_call_function(env, NapiGetUndefined(env), jsCallback->GetNapiValue(), ArraySize(argv), argv, nullptr);
    };
    taskScheduler_->PostMainThreadTask(task, "NotifyPrivacyModeChange");
}

void JsSceneSession::Finalizer(napi_env env, void* data, void* hint)
{
    WLOGFI("[NAPI]");
    auto jsSceneSession = static_cast<JsSceneSession*>(data);
    if (jsSceneSession == nullptr) {
        TLOGE(WmsLogTag::WMS_LIFE, "JsSceneSession is nullptr");
        return;
    }
    jsSceneSession->DecStrongRef(nullptr);
}

napi_value JsSceneSession::RegisterCallback(napi_env env, napi_callback_info info)
{
    WLOGFD("[NAPI]");
    JsSceneSession* me = CheckParamsAndGetThis<JsSceneSession>(env, info);
    return (me != nullptr) ? me->OnRegisterCallback(env, info) : nullptr;
}

napi_value JsSceneSession::UpdateNativeVisibility(napi_env env, napi_callback_info info)
{
    if (Session::IsScbCoreEnabled()) {
        return nullptr;
    }
    WLOGFI("[NAPI]");
    JsSceneSession* me = CheckParamsAndGetThis<JsSceneSession>(env, info);
    return (me != nullptr) ? me->OnUpdateNativeVisibility(env, info) : nullptr;
}

napi_value JsSceneSession::SetPrivacyMode(napi_env env, napi_callback_info info)
{
    WLOGFI("[NAPI]");
    JsSceneSession* me = CheckParamsAndGetThis<JsSceneSession>(env, info);
    return (me != nullptr) ? me->OnSetPrivacyMode(env, info) : nullptr;
}

napi_value JsSceneSession::SetSkipSelfWhenShowOnVirtualScreen(napi_env env, napi_callback_info info)
{
    WLOGFI("[NAPI]");
    JsSceneSession* me = CheckParamsAndGetThis<JsSceneSession>(env, info);
    return (me != nullptr) ? me->OnSetSkipSelfWhenShowOnVirtualScreen(env, info) : nullptr;
}

napi_value JsSceneSession::SetSystemSceneOcclusionAlpha(napi_env env, napi_callback_info info)
{
    WLOGFI("[NAPI]");
    JsSceneSession* me = CheckParamsAndGetThis<JsSceneSession>(env, info);
    return (me != nullptr) ? me->OnSetSystemSceneOcclusionAlpha(env, info) : nullptr;
}

napi_value JsSceneSession::SetSystemSceneForceUIFirst(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::DEFAULT, "[NAPI]");
    JsSceneSession* me = CheckParamsAndGetThis<JsSceneSession>(env, info);
    return (me != nullptr) ? me->OnSetSystemSceneForceUIFirst(env, info) : nullptr;
}

napi_value JsSceneSession::SetFocusable(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::WMS_FOCUS, "[NAPI]");
    JsSceneSession* me = CheckParamsAndGetThis<JsSceneSession>(env, info);
    return (me != nullptr) ? me->OnSetFocusable(env, info) : nullptr;
}

napi_value JsSceneSession::SetFocusableOnShow(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::WMS_FOCUS, "[NAPI]");
    JsSceneSession* me = CheckParamsAndGetThis<JsSceneSession>(env, info);
    return (me != nullptr) ? me->OnSetFocusableOnShow(env, info) : nullptr;
}

napi_value JsSceneSession::SetSystemFocusable(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::WMS_FOCUS, "[NAPI]");
    JsSceneSession* me = CheckParamsAndGetThis<JsSceneSession>(env, info);
    return (me != nullptr) ? me->OnSetSystemFocusable(env, info) : nullptr;
}

napi_value JsSceneSession::SetSystemSceneBlockingFocus(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::WMS_FOCUS, "[NAPI]");
    JsSceneSession* me = CheckParamsAndGetThis<JsSceneSession>(env, info);
    return (me != nullptr) ? me->OnSetSystemSceneBlockingFocus(env, info) : nullptr;
}

napi_value JsSceneSession::UpdateSizeChangeReason(napi_env env, napi_callback_info info)
{
    WLOGFD("[NAPI]");
    JsSceneSession* me = CheckParamsAndGetThis<JsSceneSession>(env, info);
    return (me != nullptr) ? me->OnUpdateSizeChangeReason(env, info) : nullptr;
}

napi_value JsSceneSession::OpenKeyboardSyncTransaction(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::WMS_KEYBOARD, "[NAPI]");
    JsSceneSession* me = CheckParamsAndGetThis<JsSceneSession>(env, info);
    return (me != nullptr) ? me->OnOpenKeyboardSyncTransaction(env, info) : nullptr;
}

napi_value JsSceneSession::CloseKeyboardSyncTransaction(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::WMS_KEYBOARD, "[NAPI]");
    JsSceneSession* me = CheckParamsAndGetThis<JsSceneSession>(env, info);
    return (me != nullptr) ? me->OnCloseKeyboardSyncTransaction(env, info) : nullptr;
}

napi_value JsSceneSession::NotifyTargetScreenWidthAndHeight(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::WMS_KEYBOARD, "[NAPI]");
    JsSceneSession* me = CheckParamsAndGetThis<JsSceneSession>(env, info);
    return (me != nullptr) ? me->OnNotifyTargetScreenWidthAndHeight(env, info) : nullptr;
}

napi_value JsSceneSession::SetShowRecent(napi_env env, napi_callback_info info)
{
    WLOGFI("[NAPI]");
    JsSceneSession* me = CheckParamsAndGetThis<JsSceneSession>(env, info);
    return (me != nullptr) ? me->OnSetShowRecent(env, info) : nullptr;
}

/** @note @window.hierarchy */
napi_value JsSceneSession::SetZOrder(napi_env env, napi_callback_info info)
{
    if (Session::IsScbCoreEnabled()) {
        return nullptr;
    }
    WLOGFD("[NAPI]");
    JsSceneSession* me = CheckParamsAndGetThis<JsSceneSession>(env, info);
    return (me != nullptr) ? me->OnSetZOrder(env, info) : nullptr;
}

napi_value JsSceneSession::SetTouchable(napi_env env, napi_callback_info info)
{
    JsSceneSession* me = CheckParamsAndGetThis<JsSceneSession>(env, info);
    return (me != nullptr) ? me->OnSetTouchable(env, info): nullptr;
}

napi_value JsSceneSession::SetRectChangeBySystem(napi_env env, napi_callback_info info)
{
    JsSceneSession* me = CheckParamsAndGetThis<JsSceneSession>(env, info);
    return (me != nullptr) ? me->OnSetRectChangeBySystem(env, info): nullptr;
}

napi_value JsSceneSession::SetSystemActive(napi_env env, napi_callback_info info)
{
    JsSceneSession* me = CheckParamsAndGetThis<JsSceneSession>(env, info);
    return (me != nullptr) ? me->OnSetSystemActive(env, info): nullptr;
}

napi_value JsSceneSession::SetFloatingScale(napi_env env, napi_callback_info info)
{
    WLOGFI("[NAPI]");
    JsSceneSession* me = CheckParamsAndGetThis<JsSceneSession>(env, info);
    return (me != nullptr) ? me->OnSetFloatingScale(env, info) : nullptr;
}

napi_value JsSceneSession::SetIsMidScene(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::WMS_MULTI_WINDOW, "[NAPI]");
    JsSceneSession* me = CheckParamsAndGetThis<JsSceneSession>(env, info);
    return (me != nullptr) ? me->OnSetIsMidScene(env, info) : nullptr;
}

napi_value JsSceneSession::SetSCBKeepKeyboard(napi_env env, napi_callback_info info)
{
    WLOGFI("[NAPI]");
    JsSceneSession* me = CheckParamsAndGetThis<JsSceneSession>(env, info);
    return (me != nullptr) ? me->OnSetSCBKeepKeyboard(env, info) : nullptr;
}

napi_value JsSceneSession::SetOffset(napi_env env, napi_callback_info info)
{
    if (Session::IsScbCoreEnabled()) {
        return nullptr;
    }
    WLOGFI("[NAPI]");
    JsSceneSession *me = CheckParamsAndGetThis<JsSceneSession>(env, info);
    return (me != nullptr) ? me->OnSetOffset(env, info) : nullptr;
}

napi_value JsSceneSession::SetExitSplitOnBackground(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::WMS_MULTI_WINDOW, "[NAPI]");
    JsSceneSession* me = CheckParamsAndGetThis<JsSceneSession>(env, info);
    return (me != nullptr) ? me->OnSetExitSplitOnBackground(env, info) : nullptr;
}

napi_value JsSceneSession::SetWaterMarkFlag(napi_env env, napi_callback_info info)
{
    JsSceneSession* me = CheckParamsAndGetThis<JsSceneSession>(env, info);
    return (me != nullptr) ? me->OnSetWaterMarkFlag(env, info) : nullptr;
}

napi_value JsSceneSession::SetPipActionEvent(napi_env env, napi_callback_info info)
{
    TLOGI(WmsLogTag::WMS_PIP, "[NAPI]");
    JsSceneSession* me = CheckParamsAndGetThis<JsSceneSession>(env, info);
    return (me != nullptr) ? me->OnSetPipActionEvent(env, info) : nullptr;
}

napi_value JsSceneSession::SetPiPControlEvent(napi_env env, napi_callback_info info)
{
    TLOGI(WmsLogTag::WMS_PIP, "[NAPI]");
    JsSceneSession* me = CheckParamsAndGetThis<JsSceneSession>(env, info);
    return (me != nullptr) ? me->OnSetPiPControlEvent(env, info) : nullptr;
}

napi_value JsSceneSession::NotifyDisplayStatusBarTemporarily(napi_env env, napi_callback_info info)
{
    JsSceneSession* me = CheckParamsAndGetThis<JsSceneSession>(env, info);
    return (me != nullptr) ? me->OnNotifyDisplayStatusBarTemporarily(env, info) : nullptr;
}

napi_value JsSceneSession::SetTemporarilyShowWhenLocked(napi_env env, napi_callback_info info)
{
    TLOGI(WmsLogTag::WMS_SCB, "[NAPI]");
    JsSceneSession *me = CheckParamsAndGetThis<JsSceneSession>(env, info);
    return (me != nullptr) ? me->OnSetTemporarilyShowWhenLocked(env, info) : nullptr;
}

napi_value JsSceneSession::SetSkipDraw(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::WMS_SCB, "[NAPI]");
    JsSceneSession *me = CheckParamsAndGetThis<JsSceneSession>(env, info);
    return (me != nullptr) ? me->OnSetSkipDraw(env, info) : nullptr;
}

napi_value JsSceneSession::SetCompatibleModeInPc(napi_env env, napi_callback_info info)
{
    TLOGI(WmsLogTag::WMS_SCB, "[NAPI]");
    JsSceneSession *me = CheckParamsAndGetThis<JsSceneSession>(env, info);
    return (me != nullptr) ? me->OnSetCompatibleModeInPc(env, info) : nullptr;
}

napi_value JsSceneSession::SetAppSupportPhoneInPc(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::WMS_SCB, "[NAPI]");
    JsSceneSession* me = CheckParamsAndGetThis<JsSceneSession>(env, info);
    return (me != nullptr) ? me->OnSetAppSupportPhoneInPc(env, info) : nullptr;
}

napi_value JsSceneSession::SetCompatibleWindowSizeInPc(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::WMS_SCB, "[NAPI]");
    JsSceneSession* me = CheckParamsAndGetThis<JsSceneSession>(env, info);
    return (me != nullptr) ? me->OnSetCompatibleWindowSizeInPc(env, info) : nullptr;
}

napi_value JsSceneSession::SetUniqueDensityDpiFromSCB(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::WMS_SCB, "[NAPI]");
    JsSceneSession *me = CheckParamsAndGetThis<JsSceneSession>(env, info);
    return (me != nullptr) ? me->OnSetUniqueDensityDpiFromSCB(env, info) : nullptr;
}

napi_value JsSceneSession::SetBlankFlag(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::WMS_SCB, "[NAPI]");
    JsSceneSession *me = CheckParamsAndGetThis<JsSceneSession>(env, info);
    return (me != nullptr) ? me->OnSetBlankFlag(env, info) : nullptr;
}

napi_value JsSceneSession::SetBufferAvailableCallbackEnable(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::WMS_SCB, "[NAPI]");
    JsSceneSession *me = CheckParamsAndGetThis<JsSceneSession>(env, info);
    return (me != nullptr) ? me->OnSetBufferAvailableCallbackEnable(env, info) : nullptr;
}

napi_value JsSceneSession::SyncDefaultRequestedOrientation(napi_env env, napi_callback_info info)
{
    TLOGI(WmsLogTag::WMS_SCB, "[NAPI]");
    JsSceneSession* me = CheckParamsAndGetThis<JsSceneSession>(env, info);
    return (me != nullptr) ? me->OnSyncDefaultRequestedOrientation(env, info) : nullptr;
}

napi_value JsSceneSession::SetIsPcAppInPad(napi_env env, napi_callback_info info)
{
    TLOGI(WmsLogTag::WMS_SCB, "[NAPI]");
    JsSceneSession* me = CheckParamsAndGetThis<JsSceneSession>(env, info);
    return (me != nullptr) ? me->OnSetIsPcAppInPad(env, info) : nullptr;
}

napi_value JsSceneSession::SetCompatibleModeEnableInPad(napi_env env, napi_callback_info info)
{
    TLOGI(WmsLogTag::WMS_SCB, "[NAPI]");
    JsSceneSession* me = CheckParamsAndGetThis<JsSceneSession>(env, info);
    return (me != nullptr) ? me->OnSetCompatibleModeEnableInPad(env, info) : nullptr;
}

napi_value JsSceneSession::SetStartingWindowExitAnimationFlag(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::WMS_SCB, "[NAPI]");
    JsSceneSession *me = CheckParamsAndGetThis<JsSceneSession>(env, info);
    return (me != nullptr) ? me->OnSetStartingWindowExitAnimationFlag(env, info) : nullptr;
}

napi_value JsSceneSession::CompatibleFullScreenRecover(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::WMS_SCB, "[NAPI]");
    JsSceneSession *me = CheckParamsAndGetThis<JsSceneSession>(env, info);
    return (me != nullptr) ? me->OnCompatibleFullScreenRecover(env, info) : nullptr;
}

napi_value JsSceneSession::CompatibleFullScreenMinimize(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::WMS_SCB, "[NAPI]");
    JsSceneSession *me = CheckParamsAndGetThis<JsSceneSession>(env, info);
    return (me != nullptr) ? me->OnCompatibleFullScreenMinimize(env, info) : nullptr;
}

napi_value JsSceneSession::CompatibleFullScreenClose(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::WMS_SCB, "[NAPI]");
    JsSceneSession *me = CheckParamsAndGetThis<JsSceneSession>(env, info);
    return (me != nullptr) ? me->OnCompatibleFullScreenClose(env, info) : nullptr;
}

napi_value JsSceneSession::SyncScenePanelGlobalPosition(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::WMS_SCB, "[NAPI]");
    JsSceneSession* me = CheckParamsAndGetThis<JsSceneSession>(env, info);
    return (me != nullptr) ? me->OnSyncScenePanelGlobalPosition(env, info) : nullptr;
}

napi_value JsSceneSession::UnSyncScenePanelGlobalPosition(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::WMS_SCB, "[NAPI]");
    JsSceneSession* me = CheckParamsAndGetThis<JsSceneSession>(env, info);
    return (me != nullptr) ? me->OnUnSyncScenePanelGlobalPosition(env, info) : nullptr;
}

bool JsSceneSession::IsCallbackRegistered(napi_env env, const std::string& type, napi_value jsListenerObject)
{
    HITRACE_METER_FMT(HITRACE_TAG_WINDOW_MANAGER, "JsSceneSession::IsCallbackRegistered[%s]", type.c_str());
    std::shared_lock<std::shared_mutex> lock(jsCbMapMutex_);
    if (jsCbMap_.empty() || jsCbMap_.find(type) == jsCbMap_.end()) {
        return false;
    }

    for (auto iter = jsCbMap_.begin(); iter != jsCbMap_.end(); ++iter) {
        bool isEquals = false;
        napi_strict_equals(env, jsListenerObject, iter->second->GetNapiValue(), &isEquals);
        if (isEquals) {
            WLOGFE("[NAPI] %{public}s has already been registered", type.c_str());
            return true;
        }
    }
    return false;
}

napi_value JsSceneSession::OnRegisterCallback(napi_env env, napi_callback_info info)
{
    size_t argc = 4;
    napi_value argv[4] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc < 2) { // 2: params num
        WLOGFE("[NAPI]Argc is invalid: %{public}zu", argc);
        return NapiGetUndefined(env);
    }
    std::string cbType;
    if (!ConvertFromJsValue(env, argv[0], cbType)) {
        WLOGFE("[NAPI]Failed to convert parameter to callbackType");
        return NapiGetUndefined(env);
    }
    napi_value value = argv[1];
    if (value == nullptr || !NapiIsCallable(env, value)) {
        WLOGFE("[NAPI]Invalid argument");
        return NapiGetUndefined(env);
    }
    auto iterFunctionType = ListenerFuncMap.find(cbType);
    if (iterFunctionType == ListenerFuncMap.end()) {
        WLOGFE("[NAPI]callback type is not supported, type = %{public}s", cbType.c_str());
        return NapiGetUndefined(env);
    }
    ListenerFuncType listenerFuncType = iterFunctionType->second;
    if (IsCallbackRegistered(env, cbType, value)) {
        WLOGFE("[NAPI]callback is registered, type = %{public}s", cbType.c_str());
        return NapiGetUndefined(env);
    }
    auto session = weakSession_.promote();
    if (session == nullptr) {
        WLOGFE("[NAPI]session is nullptr, id:%{public}d", persistentId_);
        return NapiGetUndefined(env);
    }

    std::shared_ptr<NativeReference> callbackRef;
    napi_ref result = nullptr;
    napi_create_reference(env, value, 1, &result);
    callbackRef.reset(reinterpret_cast<NativeReference*>(result));
    {
        HITRACE_METER_FMT(HITRACE_TAG_WINDOW_MANAGER, "JsSceneSession set jsCbMap[%s]", cbType.c_str());
        std::unique_lock<std::shared_mutex> lock(jsCbMapMutex_);
        jsCbMap_[cbType] = callbackRef;
    }
    ProcessRegisterCallback(listenerFuncType);
    WLOGFD("[NAPI] end, type = %{public}s", cbType.c_str());
    return NapiGetUndefined(env);
}

void JsSceneSession::ProcessRegisterCallback(ListenerFuncType listenerFuncType)
{
    switch (static_cast<uint32_t>(listenerFuncType)) {
        case static_cast<uint32_t>(ListenerFuncType::PENDING_SCENE_CB):
            ProcessPendingSceneSessionActivationRegister();
            break;
        case static_cast<uint32_t>(ListenerFuncType::RESTORE_MAIN_WINDOW_CB):
            ProcessRestoreMainWindowRegister();
            break;
        case static_cast<uint32_t>(ListenerFuncType::CHANGE_SESSION_VISIBILITY_WITH_STATUS_BAR):
            ProcessChangeSessionVisibilityWithStatusBarRegister();
            break;
        case static_cast<uint32_t>(ListenerFuncType::SESSION_STATE_CHANGE_CB):
            ProcessSessionStateChangeRegister();
            break;
        case static_cast<uint32_t>(ListenerFuncType::BUFFER_AVAILABLE_CHANGE_CB):
            ProcessBufferAvailableChangeRegister();
            break;
        case static_cast<uint32_t>(ListenerFuncType::SESSION_EVENT_CB):
            ProcessSessionEventRegister();
            break;
        case static_cast<uint32_t>(ListenerFuncType::SESSION_RECT_CHANGE_CB):
            ProcessSessionRectChangeRegister();
            break;
        case static_cast<uint32_t>(ListenerFuncType::SESSION_PIP_CONTROL_STATUS_CHANGE_CB):
            ProcessSessionPiPControlStatusChangeRegister();
            break;
        case static_cast<uint32_t>(ListenerFuncType::SESSION_AUTO_START_PIP_CB):
            ProcessAutoStartPiPStatusChangeRegister();
            break;
        case static_cast<uint32_t>(ListenerFuncType::CREATE_SUB_SESSION_CB):
            ProcessCreateSubSessionRegister();
            break;
        case static_cast<uint32_t>(ListenerFuncType::BIND_DIALOG_TARGET_CB):
            ProcessBindDialogTargetRegister();
            break;
        case static_cast<uint32_t>(ListenerFuncType::RAISE_TO_TOP_CB):
            ProcessRaiseToTopRegister();
            break;
        case static_cast<uint32_t>(ListenerFuncType::RAISE_TO_TOP_POINT_DOWN_CB):
            ProcessRaiseToTopForPointDownRegister();
            break;
        case static_cast<uint32_t>(ListenerFuncType::CLICK_MODAL_SPECIFIC_WINDOW_OUTSIDE_CB):
            ProcessClickModalSpecificWindowOutsideRegister();
            break;
        case static_cast<uint32_t>(ListenerFuncType::BACK_PRESSED_CB):
            ProcessBackPressedRegister();
            break;
        case static_cast<uint32_t>(ListenerFuncType::SESSION_FOCUSABLE_CHANGE_CB):
            ProcessSessionFocusableChangeRegister();
            break;
        case static_cast<uint32_t>(ListenerFuncType::SESSION_TOUCHABLE_CHANGE_CB):
            ProcessSessionTouchableChangeRegister();
            break;
        case static_cast<uint32_t>(ListenerFuncType::SESSION_TOP_MOST_CHANGE_CB):
            ProcessSessionTopmostChangeRegister();
            break;
        case static_cast<uint32_t>(ListenerFuncType::SESSION_MAIN_WINDOW_TOP_MOST_CHANGE_CB):
            ProcessMainWindowTopmostChangeRegister();
            break;
        case static_cast<uint32_t>(ListenerFuncType::SESSION_MODAL_TYPE_CHANGE_CB):
            ProcessSessionModalTypeChangeRegister();
            break;
        case static_cast<uint32_t>(ListenerFuncType::CLICK_CB):
            ProcessClickRegister();
            break;
        case static_cast<uint32_t>(ListenerFuncType::TERMINATE_SESSION_CB):
            ProcessTerminateSessionRegister();
            break;
        case static_cast<uint32_t>(ListenerFuncType::TERMINATE_SESSION_CB_NEW):
            ProcessTerminateSessionRegisterNew();
            break;
        case static_cast<uint32_t>(ListenerFuncType::TERMINATE_SESSION_CB_TOTAL):
            ProcessTerminateSessionRegisterTotal();
            break;
        case static_cast<uint32_t>(ListenerFuncType::SESSION_EXCEPTION_CB):
            ProcessSessionExceptionRegister();
            break;
        case static_cast<uint32_t>(ListenerFuncType::UPDATE_SESSION_LABEL_CB):
            ProcessUpdateSessionLabelRegister();
            break;
        case static_cast<uint32_t>(ListenerFuncType::UPDATE_SESSION_ICON_CB):
            ProcessUpdateSessionIconRegister();
            break;
        case static_cast<uint32_t>(ListenerFuncType::SYSTEMBAR_PROPERTY_CHANGE_CB):
            ProcessSystemBarPropertyChangeRegister();
            break;
        case static_cast<uint32_t>(ListenerFuncType::NEED_AVOID_CB):
            ProcessNeedAvoidRegister();
            break;
        case static_cast<uint32_t>(ListenerFuncType::PENDING_SESSION_TO_FOREGROUND_CB):
            ProcessPendingSessionToForegroundRegister();
            break;
        case static_cast<uint32_t>(ListenerFuncType::PENDING_SESSION_TO_BACKGROUND_FOR_DELEGATOR_CB):
            ProcessPendingSessionToBackgroundForDelegatorRegister();
            break;
        case static_cast<uint32_t>(ListenerFuncType::CUSTOM_ANIMATION_PLAYING_CB):
            ProcessIsCustomAnimationPlaying();
            break;
        case static_cast<uint32_t>(ListenerFuncType::NEED_DEFAULT_ANIMATION_FLAG_CHANGE_CB):
            ProcessSessionDefaultAnimationFlagChangeRegister();
            break;
        case static_cast<uint32_t>(ListenerFuncType::SHOW_WHEN_LOCKED_CB):
            ProcessShowWhenLockedRegister();
            break;
        case static_cast<uint32_t>(ListenerFuncType::REQUESTED_ORIENTATION_CHANGE_CB):
            ProcessRequestedOrientationChange();
            break;
        case static_cast<uint32_t>(ListenerFuncType::RAISE_ABOVE_TARGET_CB):
            ProcessRaiseAboveTargetRegister();
            break;
        case static_cast<uint32_t>(ListenerFuncType::FORCE_HIDE_CHANGE_CB):
            ProcessForceHideChangeRegister();
            break;
        case static_cast<uint32_t>(ListenerFuncType::WINDOW_DRAG_HOT_AREA_CB):
            ProcessWindowDragHotAreaRegister();
            break;
        case static_cast<uint32_t>(ListenerFuncType::TOUCH_OUTSIDE_CB):
            ProcessTouchOutsideRegister();
            break;
        case static_cast<uint32_t>(ListenerFuncType::SESSIONINFO_LOCKEDSTATE_CHANGE_CB):
            ProcessSessionInfoLockedStateChangeRegister();
            break;
        case static_cast<uint32_t>(ListenerFuncType::PREPARE_CLOSE_PIP_SESSION):
            ProcessPrepareClosePiPSessionRegister();
            break;
        case static_cast<uint32_t>(ListenerFuncType::LANDSCAPE_MULTI_WINDOW_CB):
            ProcessLandscapeMultiWindowRegister();
            break;
        case static_cast<uint32_t>(ListenerFuncType::CONTEXT_TRANSPARENT_CB):
            ProcessContextTransparentRegister();
            break;
        case static_cast<uint32_t>(ListenerFuncType::KEYBOARD_GRAVITY_CHANGE_CB):
            ProcessKeyboardGravityChangeRegister();
            break;
        case static_cast<uint32_t>(ListenerFuncType::ADJUST_KEYBOARD_LAYOUT_CB):
            ProcessAdjustKeyboardLayoutRegister();
            break;
        case static_cast<uint32_t>(ListenerFuncType::LAYOUT_FULL_SCREEN_CB):
            ProcessLayoutFullScreenChangeRegister();
            break;
        case static_cast<uint32_t>(ListenerFuncType::TITLE_DOCK_HOVER_SHOW_CB):
            ProcessTitleAndDockHoverShowChangeRegister();
            break;
        case static_cast<uint32_t>(ListenerFuncType::NEXT_FRAME_LAYOUT_FINISH_CB):
            ProcessFrameLayoutFinishRegister();
            break;
        case static_cast<uint32_t>(ListenerFuncType::PRIVACY_MODE_CHANGE_CB):
            ProcessPrivacyModeChangeRegister();
            break;
        default:
            break;
    }
}

napi_value JsSceneSession::OnUpdateNativeVisibility(napi_env env, napi_callback_info info)
{
    size_t argc = 4;
    napi_value argv[4] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc < 1) { // 1: params num
        WLOGFE("[NAPI]Argc is invalid: %{public}zu", argc);
        return NapiGetUndefined(env);
    }
    bool visible = false;
    if (!ConvertFromJsValue(env, argv[0], visible)) {
        WLOGFE("[NAPI]Failed to convert parameter to bool");
        return NapiGetUndefined(env);
    }
    auto session = weakSession_.promote();
    if (session == nullptr) {
        WLOGFE("[NAPI]session is nullptr, id:%{public}d", persistentId_);
        return NapiGetUndefined(env);
    }
    session->UpdateNativeVisibility(visible);
    WLOGFI("[NAPI] end");
    return NapiGetUndefined(env);
}

napi_value JsSceneSession::OnSetPrivacyMode(napi_env env, napi_callback_info info)
{
    size_t argc = 4;
    napi_value argv[4] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc < 1) { // 1: params num
        WLOGFE("[NAPI]Argc is invalid: %{public}zu", argc);
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    bool isPrivacy = false;
    if (!ConvertFromJsValue(env, argv[0], isPrivacy)) {
        WLOGFE("[NAPI]Failed to convert parameter to bool");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    auto session = weakSession_.promote();
    if (session == nullptr) {
        WLOGFE("[NAPI]session is nullptr, id:%{public}d", persistentId_);
        return NapiGetUndefined(env);
    }
    session->SetPrivacyMode(isPrivacy);
    SceneSessionManager::GetInstance().UpdatePrivateStateAndNotify(session->GetPersistentId());
    WLOGFI("[NAPI] end");
    return NapiGetUndefined(env);
}

napi_value JsSceneSession::OnSetSystemSceneOcclusionAlpha(napi_env env, napi_callback_info info)
{
    size_t argc = 4;
    napi_value argv[4] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc < 1) { // 1: params num
        WLOGFE("[NAPI]Argc is invalid: %{public}zu", argc);
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    double alpha = 0.f;
    if (!ConvertFromJsValue(env, argv[0], alpha)) {
        WLOGFE("[NAPI]Failed to convert parameter to bool");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    auto session = weakSession_.promote();
    if (session == nullptr) {
        WLOGFE("[NAPI]session is nullptr, id:%{public}d", persistentId_);
        return NapiGetUndefined(env);
    }
    session->SetSystemSceneOcclusionAlpha(alpha);
    WLOGFI("[NAPI] end");
    return NapiGetUndefined(env);
}

napi_value JsSceneSession::OnSetSystemSceneForceUIFirst(napi_env env, napi_callback_info info)
{
    size_t argc = ARG_COUNT_4;
    napi_value argv[ARG_COUNT_4] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc < 1) { // 1: params num
        TLOGE(WmsLogTag::DEFAULT, "[NAPI]Argc is invalid: %{public}zu", argc);
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    bool forceUIFirst = false;
    if (!ConvertFromJsValue(env, argv[0], forceUIFirst)) {
        TLOGE(WmsLogTag::DEFAULT, "[NAPI]Failed to convert parameter to forceUIFirst");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    auto session = weakSession_.promote();
    if (session == nullptr) {
        TLOGE(WmsLogTag::DEFAULT, "[NAPI]session is nullptr, id:%{public}d", persistentId_);
        return NapiGetUndefined(env);
    }
    session->SetSystemSceneForceUIFirst(forceUIFirst);
    TLOGD(WmsLogTag::DEFAULT, "[NAPI] end");
    return NapiGetUndefined(env);
}

napi_value JsSceneSession::OnSetFocusable(napi_env env, napi_callback_info info)
{
    size_t argc = 4;
    napi_value argv[4] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc < 1) { // 1: params num
        WLOGFE("[NAPI]Argc is invalid: %{public}zu", argc);
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    bool isFocusable = false;
    if (!ConvertFromJsValue(env, argv[0], isFocusable)) {
        WLOGFE("[NAPI]Failed to convert parameter to bool");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    auto session = weakSession_.promote();
    if (session == nullptr) {
        WLOGFE("[NAPI]session is nullptr, id:%{public}d", persistentId_);
        return NapiGetUndefined(env);
    }
    session->SetFocusable(isFocusable);
    TLOGD(WmsLogTag::WMS_FOCUS, "[NAPI] end");
    return NapiGetUndefined(env);
}

napi_value JsSceneSession::OnSetFocusableOnShow(napi_env env, napi_callback_info info)
{
    size_t argc = ARG_COUNT_4;
    napi_value argv[ARG_COUNT_4] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc < 1) { // 1: params num
        TLOGE(WmsLogTag::WMS_FOCUS, "[NAPI]Argc is invalid: %{public}zu", argc);
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    bool isFocusableOnShow = true;
    if (!ConvertFromJsValue(env, argv[0], isFocusableOnShow)) {
        TLOGE(WmsLogTag::WMS_FOCUS, "[NAPI]Failed to convert parameter to bool");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    auto session = weakSession_.promote();
    if (session == nullptr) {
        TLOGE(WmsLogTag::WMS_FOCUS, "[NAPI]session is nullptr, id:%{public}d", persistentId_);
        return NapiGetUndefined(env);
    }
    session->SetFocusableOnShow(isFocusableOnShow);
    TLOGD(WmsLogTag::WMS_FOCUS, "[NAPI] end");
    return NapiGetUndefined(env);
}

napi_value JsSceneSession::OnSetSystemFocusable(napi_env env, napi_callback_info info)
{
    size_t argc = ARG_COUNT_4;
    napi_value argv[ARG_COUNT_4] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc < 1) { // 1: params num
        TLOGE(WmsLogTag::WMS_FOCUS, "[NAPI]Argc is invalid: %{public}zu", argc);
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    bool systemFocusable = false;
    if (!ConvertFromJsValue(env, argv[0], systemFocusable)) {
        TLOGE(WmsLogTag::WMS_FOCUS, "[NAPI]Failed to convert parameter to bool");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    auto session = weakSession_.promote();
    if (session == nullptr) {
        TLOGE(WmsLogTag::WMS_FOCUS, "[NAPI]session is nullptr, id:%{public}d", persistentId_);
        return NapiGetUndefined(env);
    }
    session->SetSystemFocusable(systemFocusable);
    TLOGD(WmsLogTag::WMS_FOCUS, "[NAPI] end");
    return NapiGetUndefined(env);
}

napi_value JsSceneSession::OnSetSystemSceneBlockingFocus(napi_env env, napi_callback_info info)
{
    size_t argc = 4;
    napi_value argv[4] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc < 1) { // 1: params num
        WLOGFE("[NAPI]Argc is invalid: %{public}zu", argc);
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    bool blocking = false;
    if (!ConvertFromJsValue(env, argv[0], blocking)) {
        WLOGFE("[NAPI]Failed to convert parameter to bool");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    auto session = weakSession_.promote();
    if (session == nullptr) {
        WLOGFE("[NAPI]session is nullptr, id:%{public}d", persistentId_);
        return NapiGetUndefined(env);
    }
    session->SetSystemSceneBlockingFocus(blocking);
    WLOGFI("[NAPI] end");
    return NapiGetUndefined(env);
}

napi_value JsSceneSession::OnUpdateSizeChangeReason(napi_env env, napi_callback_info info)
{
    size_t argc = 4;
    napi_value argv[4] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc < 1) { // 1: params num
        WLOGFE("[NAPI]Argc is invalid: %{public}zu", argc);
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    SizeChangeReason reason = SizeChangeReason::UNDEFINED;
    if (!ConvertFromJsValue(env, argv[0], reason)) {
        WLOGFE("[NAPI]Failed to convert parameter to bool");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    auto session = weakSession_.promote();
    if (session == nullptr) {
        WLOGFE("[NAPI]session is nullptr, id:%{public}d", persistentId_);
        return NapiGetUndefined(env);
    }
    session->UpdateSizeChangeReason(reason);
    WLOGFI("[NAPI] reason: %{public}u end", reason);
    return NapiGetUndefined(env);
}

napi_value JsSceneSession::OnOpenKeyboardSyncTransaction(napi_env env, napi_callback_info info)
{
    auto session = weakSession_.promote();
    if (session == nullptr) {
        TLOGE(WmsLogTag::WMS_KEYBOARD, "[NAPI] session is nullptr, id:%{public}d", persistentId_);
        return NapiGetUndefined(env);
    }
    session->OpenKeyboardSyncTransaction();
    return NapiGetUndefined(env);
}

napi_value JsSceneSession::OnCloseKeyboardSyncTransaction(napi_env env, napi_callback_info info)
{
    size_t argc = ARG_COUNT_4;
    napi_value argv[ARG_COUNT_4] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc < 3) { // 3: params num
        TLOGE(WmsLogTag::WMS_KEYBOARD, "[NAPI]Argc is invalid: %{public}zu", argc);
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }

    WSRect keyboardPanelRect = { 0, 0, 0, 0 };
    napi_value nativeObj = argv[ARG_INDEX_0];
    if (nativeObj == nullptr || !ConvertRectInfoFromJs(env, nativeObj, keyboardPanelRect)) {
        TLOGE(WmsLogTag::WMS_KEYBOARD, "[NAPI]Failed to convert parameter to keyboardPanelRect");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }

    bool isKeyboardShow = false;
    if (!ConvertFromJsValue(env, argv[ARG_INDEX_1], isKeyboardShow)) {
        TLOGE(WmsLogTag::WMS_KEYBOARD, "[NAPI]Failed to convert parameter to bool");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }

    bool isRotating = false;
    if (!ConvertFromJsValue(env, argv[ARG_INDEX_2], isRotating)) {
        TLOGE(WmsLogTag::WMS_KEYBOARD, "[NAPI]Failed to convert parameter to bool");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }

    auto session = weakSession_.promote();
    if (session == nullptr) {
        TLOGE(WmsLogTag::WMS_KEYBOARD, "[NAPI]session is nullptr, id:%{public}d", persistentId_);
        return NapiGetUndefined(env);
    }
    session->CloseKeyboardSyncTransaction(keyboardPanelRect, isKeyboardShow, isRotating);
    return NapiGetUndefined(env);
}

napi_value JsSceneSession::OnNotifyTargetScreenWidthAndHeight(napi_env env, napi_callback_info info)
{
    size_t argc = ARG_COUNT_4;
    napi_value argv[ARG_COUNT_4] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc < ARG_COUNT_3) {
        TLOGE(WmsLogTag::WMS_KEYBOARD, "[NAPI]Argc is invalid: %{public}zu", argc);
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }

    bool isScreenAngleMismatch = false;
    if (!ConvertFromJsValue(env, argv[ARG_INDEX_0], isScreenAngleMismatch)) {
        TLOGE(WmsLogTag::WMS_KEYBOARD, "[NAPI]Failed to convert parameter to bool");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }

    uint32_t screenWidth = 0;
    if (!ConvertFromJsValue(env, argv[ARG_INDEX_1], screenWidth)) {
        TLOGE(WmsLogTag::WMS_KEYBOARD, "[NAPI]Failed to convert parameter to uint32_t");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }

    uint32_t screenHeight = 0;
    if (!ConvertFromJsValue(env, argv[ARG_INDEX_2], screenHeight)) {
        TLOGE(WmsLogTag::WMS_KEYBOARD, "[NAPI]Failed to convert parameter to uint32_t");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }

    auto session = weakSession_.promote();
    if (session == nullptr) {
        TLOGE(WmsLogTag::WMS_KEYBOARD, "[NAPI]session is nullptr, id:%{public}d", persistentId_);
        return NapiGetUndefined(env);
    }
    session->NotifyTargetScreenWidthAndHeight(isScreenAngleMismatch, screenWidth, screenHeight);
    return NapiGetUndefined(env);
}

void JsSceneSession::OnCreateSubSession(const sptr<SceneSession>& sceneSession)
{
    if (sceneSession == nullptr) {
        WLOGFE("[NAPI]sceneSession is nullptr");
        return;
    }

    TLOGI(WmsLogTag::WMS_LIFE, "[NAPI]OnCreateSubSession, id: %{public}d, parentId: %{public}d",
        sceneSession->GetPersistentId(), sceneSession->GetParentPersistentId());
    wptr<SceneSession> weakSession(sceneSession);
    auto task = [weakThis = wptr(this), persistentId = persistentId_, weakSession, env = env_] {
        auto jsSceneSession = weakThis.promote();
        if (!jsSceneSession || jsSceneSessionMap_.find(persistentId) == jsSceneSessionMap_.end()) {
            TLOGE(WmsLogTag::WMS_LIFE, "OnCreateSubSession jsSceneSession id:%{public}d has been destroyed",
                persistentId);
            return;
        }
        auto jsCallBack = jsSceneSession->GetJSCallback(CREATE_SUB_SESSION_CB);
        if (jsCallBack == nullptr) {
            WLOGFE("[NAPI]jsCallBack is nullptr");
            return;
        }
        auto specificSession = weakSession.promote();
        if (specificSession == nullptr) {
            TLOGE(WmsLogTag::WMS_LIFE, "[NAPI]root session or target session or env is nullptr");
            return;
        }
        napi_value jsSceneSessionObj = Create(env, specificSession);
        if (jsSceneSessionObj == nullptr) {
            TLOGE(WmsLogTag::WMS_LIFE, "[NAPI]jsSceneSessionObj or jsCallBack is nullptr");
            return;
        }
        TLOGI(WmsLogTag::WMS_LIFE, "CreateJsSceneSessionObject success, id: %{public}d, parentId: %{public}d",
            specificSession->GetPersistentId(), specificSession->GetParentPersistentId());
        napi_value argv[] = {jsSceneSessionObj};
        napi_call_function(env, NapiGetUndefined(env), jsCallBack->GetNapiValue(), ArraySize(argv), argv, nullptr);
    };
    std::string info = "OnCreateSpecificSession PID:" + std::to_string(sceneSession->GetPersistentId());
    taskScheduler_->PostMainThreadTask(task, info);
}

void JsSceneSession::OnBindDialogTarget(const sptr<SceneSession>& sceneSession)
{
    if (sceneSession == nullptr) {
        WLOGFI("[NAPI]sceneSession is nullptr");
        return;
    }

    TLOGI(WmsLogTag::WMS_DIALOG, "id: %{public}d", sceneSession->GetPersistentId());

    wptr<SceneSession> weakSession(sceneSession);
    auto task = [weakThis = wptr(this), persistentId = persistentId_, weakSession, env = env_] {
        auto jsSceneSession = weakThis.promote();
        if (!jsSceneSession || jsSceneSessionMap_.find(persistentId) == jsSceneSessionMap_.end()) {
            TLOGE(WmsLogTag::WMS_LIFE, "OnBindDialogTarget jsSceneSession id:%{public}d has been destroyed",
                persistentId);
            return;
        }
        auto jsCallBack = jsSceneSession->GetJSCallback(BIND_DIALOG_TARGET_CB);
        if (jsCallBack == nullptr) {
            WLOGFE("[NAPI]jsCallBack is nullptr");
            return;
        }
        auto specificSession = weakSession.promote();
        if (specificSession == nullptr) {
            WLOGFE("[NAPI]root session or target session or env is nullptr");
            return;
        }
        napi_value jsSceneSessionObj = Create(env, specificSession);
        if (jsSceneSessionObj == nullptr) {
            WLOGFE("[NAPI]jsSceneSessionObj or jsCallBack is nullptr");
            return;
        }
        TLOGI(WmsLogTag::WMS_DIALOG, "CreateJsObject success, id: %{public}d", specificSession->GetPersistentId());
        napi_value argv[] = {jsSceneSessionObj};
        napi_call_function(env, NapiGetUndefined(env), jsCallBack->GetNapiValue(), ArraySize(argv), argv, nullptr);
    };
    taskScheduler_->PostMainThreadTask(task, "OnBindDialogTarget, PID:" +
        std::to_string(sceneSession->GetPersistentId()));
}

void JsSceneSession::OnSessionStateChange(const SessionState& state)
{
    auto session = weakSession_.promote();
    if (session == nullptr) {
        TLOGW(WmsLogTag::WMS_LIFE, "session is nullptr, id:%{public}d", persistentId_);
        return;
    }

    TLOGI(WmsLogTag::WMS_LIFE, "id: %{public}d, state: %{public}d", session->GetPersistentId(), state);
    auto task = [weakThis = wptr(this), persistentId = persistentId_, state, env = env_] {
        auto jsSceneSession = weakThis.promote();
        if (!jsSceneSession || jsSceneSessionMap_.find(persistentId) == jsSceneSessionMap_.end()) {
            TLOGE(WmsLogTag::WMS_LIFE, "OnSessionStateChange jsSceneSession id:%{public}d has been destroyed",
                persistentId);
            return;
        }
        auto jsCallBack = jsSceneSession->GetJSCallback(SESSION_STATE_CHANGE_CB);
        if (!jsCallBack) {
            WLOGFE("[NAPI]jsCallBack is nullptr");
            return;
        }
        napi_value jsSessionStateObj = CreateJsValue(env, state);
        napi_value argv[] = {jsSessionStateObj};
        napi_call_function(env, NapiGetUndefined(env), jsCallBack->GetNapiValue(), ArraySize(argv), argv, nullptr);
    };
    taskScheduler_->PostMainThreadTask(task, "OnSessionStateChange, state:" + std::to_string(static_cast<int>(state)));
}

void JsSceneSession::OnBufferAvailableChange(const bool isBufferAvailable)
{
    WLOGFD("[NAPI]state: %{public}u", isBufferAvailable);
    auto task = [weakThis = wptr(this), persistentId = persistentId_, isBufferAvailable, env = env_] {
        auto jsSceneSession = weakThis.promote();
        if (!jsSceneSession || jsSceneSessionMap_.find(persistentId) == jsSceneSessionMap_.end()) {
            TLOGE(WmsLogTag::WMS_LIFE, "OnBufferAvailableChange jsSceneSession id:%{public}d has been destroyed",
                persistentId);
            return;
        }
        auto jsCallBack = jsSceneSession->GetJSCallback(BUFFER_AVAILABLE_CHANGE_CB);
        if (!jsCallBack) {
            WLOGFE("[NAPI]jsCallBack is nullptr");
            return;
        }
        napi_value jsBufferAvailableObj = CreateJsValue(env, isBufferAvailable);
        napi_value argv[] = { jsBufferAvailableObj };
        napi_call_function(env, NapiGetUndefined(env), jsCallBack->GetNapiValue(), ArraySize(argv), argv, nullptr);
    };
    taskScheduler_->PostMainThreadTask(task, "OnBufferAvailableChange");
}

/** @note @window.layout */
void JsSceneSession::OnSessionRectChange(const WSRect& rect, const SizeChangeReason reason, const DisplayId displayId)
{
    if (reason != SizeChangeReason::MOVE && reason != SizeChangeReason::PIP_RESTORE && rect.IsEmpty()) {
        WLOGFD("Rect is empty, there is no need to notify");
        return;
    }
    WLOGFD("[NAPI]");
    auto task = [weakThis = wptr(this), persistentId = persistentId_, rect, displayId, reason, env = env_] {
        auto jsSceneSession = weakThis.promote();
        if (!jsSceneSession || jsSceneSessionMap_.find(persistentId) == jsSceneSessionMap_.end()) {
            TLOGNE(WmsLogTag::WMS_LAYOUT, "OnSessionRectChange jsSceneSession id:%{public}d has been destroyed",
                persistentId);
            return;
        }
        auto jsCallBack = jsSceneSession->GetJSCallback(SESSION_RECT_CHANGE_CB);
        if (!jsCallBack) {
            TLOGNE(WmsLogTag::WMS_LAYOUT, "[NAPI]jsCallBack is nullptr");
            return;
        }
        napi_value jsSessionRect = CreateJsSessionRect(env, rect);
        napi_value jsSizeChangeReason = CreateJsValue(env, static_cast<int32_t>(reason));
        napi_value jsDisplayId = CreateJsValue(env, static_cast<int32_t>(displayId));
        napi_value argv[] = {jsSessionRect, jsSizeChangeReason, jsDisplayId};
        napi_call_function(env, NapiGetUndefined(env), jsCallBack->GetNapiValue(), ArraySize(argv), argv, nullptr);
    };
    std::string rectInfo = "OnSessionRectChange [" + std::to_string(rect.posX_) + "," + std::to_string(rect.posY_)
        + "], [" + std::to_string(rect.width_) + ", " + std::to_string(rect.height_);
    taskScheduler_->PostMainThreadTask(task, rectInfo);
}

void JsSceneSession::OnSessionPiPControlStatusChange(WsPiPControlType controlType, WsPiPControlStatus status)
{
    TLOGI(WmsLogTag::WMS_PIP, "controlType:%{public}u, status:%{public}d", controlType, status);
    auto task = [weakThis = wptr(this), persistentId = persistentId_, controlType, status, env = env_] {
        auto jsSceneSession = weakThis.promote();
        if (!jsSceneSession || jsSceneSessionMap_.find(persistentId) == jsSceneSessionMap_.end()) {
            TLOGE(WmsLogTag::WMS_LIFE, "OnSessionPiPControlStatusChange jsSceneSession id:%{public}d has been"
                " destroyed", persistentId);
            return;
        }
        auto jsCallBack = jsSceneSession->GetJSCallback(SESSION_PIP_CONTROL_STATUS_CHANGE_CB);
        if (!jsCallBack) {
            TLOGE(WmsLogTag::WMS_PIP, "[NAPI]jsCallBack is nullptr");
            return;
        }
        napi_value controlTypeValue = CreateJsValue(env, controlType);
        napi_value controlStatusValue = CreateJsValue(env, status);
        napi_value argv[] = {controlTypeValue, controlStatusValue};
        napi_call_function(env, NapiGetUndefined(env), jsCallBack->GetNapiValue(), ArraySize(argv), argv, nullptr);
    };
    taskScheduler_->PostMainThreadTask(task, __func__);
}

void JsSceneSession::OnAutoStartPiPStatusChange(bool isAutoStart)
{
    TLOGI(WmsLogTag::WMS_PIP, "isAutoStart:%{public}u", isAutoStart);
    auto task = [weakThis = wptr(this), persistentId = persistentId_, isAutoStart, env = env_] {
        auto jsSceneSession = weakThis.promote();
        if (!jsSceneSession || jsSceneSessionMap_.find(persistentId) == jsSceneSessionMap_.end()) {
            TLOGNE(WmsLogTag::WMS_PIP, "jsSceneSession id:%{public}d has been destroyed", persistentId);
            return;
        }
        auto jsCallBack = jsSceneSession->GetJSCallback(SESSION_AUTO_START_PIP_CB);
        if (!jsCallBack) {
            TLOGNE(WmsLogTag::WMS_PIP, "[NAPI]jsCallBack is nullptr");
            return;
        }
        napi_value isAutoStartValue = CreateJsValue(env, isAutoStart);
        napi_value argv[] = {isAutoStartValue};
        napi_call_function(env, NapiGetUndefined(env), jsCallBack->GetNapiValue(), ArraySize(argv), argv, nullptr);
    };
    taskScheduler_->PostMainThreadTask(task, __func__);
}

/** @note @window.hierarchy */
void JsSceneSession::OnRaiseToTop()
{
    WLOGFI("[NAPI]");
    auto task = [weakThis = wptr(this), persistentId = persistentId_, env = env_] {
        auto jsSceneSession = weakThis.promote();
        if (!jsSceneSession || jsSceneSessionMap_.find(persistentId) == jsSceneSessionMap_.end()) {
            TLOGE(WmsLogTag::WMS_HIERARCHY, "OnRaiseToTop jsSceneSession id:%{public}d has been destroyed",
                persistentId);
            return;
        }
        auto jsCallBack = jsSceneSession->GetJSCallback(RAISE_TO_TOP_CB);
        if (!jsCallBack) {
            WLOGFE("[NAPI]jsCallBack is nullptr");
            return;
        }
        napi_value argv[] = {};
        napi_call_function(env, NapiGetUndefined(env), jsCallBack->GetNapiValue(), 0, argv, nullptr);
    };
    taskScheduler_->PostMainThreadTask(task, "OnRaiseToTop");
}

/** @note @window.hierarchy */
void JsSceneSession::OnRaiseToTopForPointDown()
{
    WLOGFI("[NAPI]");
    auto task = [weakThis = wptr(this), persistentId = persistentId_, env = env_] {
        auto jsSceneSession = weakThis.promote();
        if (!jsSceneSession || jsSceneSessionMap_.find(persistentId) == jsSceneSessionMap_.end()) {
            TLOGE(WmsLogTag::WMS_HIERARCHY, "OnRaiseToTopForPointDown jsSceneSession id:%{public}d has been destroyed",
                persistentId);
            return;
        }
        auto jsCallBack = jsSceneSession->GetJSCallback(RAISE_TO_TOP_POINT_DOWN_CB);
        if (!jsCallBack) {
            WLOGFE("[NAPI]jsCallBack is nullptr");
            return;
        }
        napi_value argv[] = {};
        napi_call_function(env, NapiGetUndefined(env), jsCallBack->GetNapiValue(), 0, argv, nullptr);
    };
    taskScheduler_->PostMainThreadTask(task, "OnRaiseToTopForPointDown");
}

void JsSceneSession::OnClickModalSpecificWindowOutside()
{
    TLOGI(WmsLogTag::WMS_LAYOUT, "[NAPI]");
    auto task = [weakThis = wptr(this), persistentId = persistentId_, env = env_] {
        auto jsSceneSession = weakThis.promote();
        if (!jsSceneSession || jsSceneSessionMap_.find(persistentId) == jsSceneSessionMap_.end()) {
            TLOGE(WmsLogTag::WMS_LIFE, "OnClickModalSpecificWindowOutside jsSceneSession id:%{public}d has been"
                " destroyed", persistentId);
            return;
        }
        auto jsCallBack = jsSceneSession->GetJSCallback(CLICK_MODAL_SPECIFIC_WINDOW_OUTSIDE_CB);
        if (!jsCallBack) {
            TLOGE(WmsLogTag::WMS_LAYOUT, "[NAPI]jsCallBack is nullptr");
            return;
        }
        napi_value argv[] = {};
        napi_call_function(env, NapiGetUndefined(env), jsCallBack->GetNapiValue(), 0, argv, nullptr);
    };
    taskScheduler_->PostMainThreadTask(task, "OnClickModalSpecificWindowOutside");
}

void JsSceneSession::OnRaiseAboveTarget(int32_t subWindowId)
{
    WLOGFI("[NAPI]");
    auto task = [weakThis = wptr(this), persistentId = persistentId_, env = env_, subWindowId] {
        auto jsSceneSession = weakThis.promote();
        if (!jsSceneSession || jsSceneSessionMap_.find(persistentId) == jsSceneSessionMap_.end()) {
            TLOGE(WmsLogTag::WMS_LIFE, "OnRaiseAboveTarget jsSceneSession id:%{public}d has been destroyed",
                persistentId);
            return;
        }
        auto jsCallBack = jsSceneSession->GetJSCallback(RAISE_ABOVE_TARGET_CB);
        if (!jsCallBack) {
            WLOGFE("[NAPI]jsCallBack is nullptr");
            return;
        }
        napi_value jsSceneSessionObj = CreateJsValue(env, subWindowId);
        if (jsSceneSessionObj == nullptr) {
            WLOGFE("[NAPI]jsSceneSessionObj is nullptr");
            return;
        }
        napi_value argv[] = {CreateJsError(env, 0), jsSceneSessionObj};
        napi_call_function(env, NapiGetUndefined(env), jsCallBack->GetNapiValue(), ArraySize(argv), argv, nullptr);
    };
    taskScheduler_->PostMainThreadTask(task, "OnRaiseAboveTarget: " + std::to_string(subWindowId));
}

void JsSceneSession::OnSessionFocusableChange(bool isFocusable)
{
    TLOGD(WmsLogTag::WMS_FOCUS, "[NAPI]state: %{public}u", isFocusable);
    auto task = [weakThis = wptr(this), persistentId = persistentId_, isFocusable, env = env_] {
        auto jsSceneSession = weakThis.promote();
        if (!jsSceneSession || jsSceneSessionMap_.find(persistentId) == jsSceneSessionMap_.end()) {
            TLOGE(WmsLogTag::WMS_LIFE, "OnSessionFocusableChange jsSceneSession id:%{public}d has been destroyed",
                persistentId);
            return;
        }
        auto jsCallBack = jsSceneSession->GetJSCallback(SESSION_FOCUSABLE_CHANGE_CB);
        if (!jsCallBack) {
            WLOGFE("[NAPI]jsCallBack is nullptr");
            return;
        }
        napi_value jsSessionFocusableObj = CreateJsValue(env, isFocusable);
        napi_value argv[] = {jsSessionFocusableObj};
        napi_call_function(env, NapiGetUndefined(env), jsCallBack->GetNapiValue(), ArraySize(argv), argv, nullptr);
    };
    taskScheduler_->PostMainThreadTask(task, "OnSessionFocusableChange, state:" + std::to_string(isFocusable));
}

void JsSceneSession::OnSessionTouchableChange(bool touchable)
{
    WLOGFI("[NAPI]state: %{public}u", touchable);
    auto task = [weakThis = wptr(this), persistentId = persistentId_, touchable, env = env_] {
        auto jsSceneSession = weakThis.promote();
        if (!jsSceneSession || jsSceneSessionMap_.find(persistentId) == jsSceneSessionMap_.end()) {
            TLOGE(WmsLogTag::WMS_LIFE, "OnSessionTouchableChange jsSceneSession id:%{public}d has been destroyed",
                persistentId);
            return;
        }
        auto jsCallBack = jsSceneSession->GetJSCallback(SESSION_TOUCHABLE_CHANGE_CB);
        if (!jsCallBack) {
            WLOGFE("[NAPI]jsCallBack is nullptr");
            return;
        }
        napi_value jsSessionTouchableObj = CreateJsValue(env, touchable);
        napi_value argv[] = {jsSessionTouchableObj};
        napi_call_function(env, NapiGetUndefined(env), jsCallBack->GetNapiValue(), ArraySize(argv), argv, nullptr);
    };
    taskScheduler_->PostMainThreadTask(task, "OnSessionTouchableChange: state " + std::to_string(touchable));
}

/** @note @window.hierarchy */
void JsSceneSession::OnSessionTopmostChange(bool topmost)
{
    TLOGI(WmsLogTag::WMS_HIERARCHY, "[NAPI]State: %{public}u", topmost);
    auto task = [weakThis = wptr(this), persistentId = persistentId_, topmost, env = env_] {
        auto jsSceneSession = weakThis.promote();
        if (!jsSceneSession || jsSceneSessionMap_.find(persistentId) == jsSceneSessionMap_.end()) {
            TLOGE(WmsLogTag::WMS_HIERARCHY, "OnSessionTopmostChange jsSceneSession id:%{public}d has been destroyed",
                persistentId);
            return;
        }
        auto jsCallBack = jsSceneSession->GetJSCallback(SESSION_TOP_MOST_CHANGE_CB);
        if (!jsCallBack) {
            TLOGE(WmsLogTag::WMS_HIERARCHY, "[NAPI]jsCallBack is nullptr");
            return;
        }
        napi_value jsSessionTouchableObj = CreateJsValue(env, topmost);
        napi_value argv[] = {jsSessionTouchableObj};
        napi_call_function(env, NapiGetUndefined(env), jsCallBack->GetNapiValue(), ArraySize(argv), argv, nullptr);
    };
    taskScheduler_->PostMainThreadTask(task, "OnSessionTopmostChange: state " + std::to_string(topmost));
}

/** @note @window.hierarchy */
void JsSceneSession::OnMainWindowTopmostChange(bool isTopmost)
{
    TLOGD(WmsLogTag::WMS_HIERARCHY, "[NAPI]isTopmost: %{public}u", isTopmost);
    auto task = [weakThis = wptr(this), persistentId = persistentId_, isTopmost, env = env_] {
        auto jsSceneSession = weakThis.promote();
        if (!jsSceneSession || jsSceneSessionMap_.find(persistentId) == jsSceneSessionMap_.end()) {
            TLOGNE(WmsLogTag::WMS_HIERARCHY, "jsSceneSession id:%{public}d has been destroyed", persistentId);
            return;
        }
        auto jsCallBack = jsSceneSession->GetJSCallback(SESSION_MAIN_WINDOW_TOP_MOST_CHANGE_CB);
        if (!jsCallBack) {
            TLOGNE(WmsLogTag::WMS_HIERARCHY, "[NAPI]jsCallBack is nullptr");
            return;
        }
        napi_value jsMainWindowTopmostObj = CreateJsValue(env, isTopmost);
        napi_value argv[] = {jsMainWindowTopmostObj};
        napi_call_function(env, NapiGetUndefined(env), jsCallBack->GetNapiValue(), ArraySize(argv), argv, nullptr);
    };
    taskScheduler_->PostMainThreadTask(task,
        "OnMainWindowTopmostChange: " + std::to_string(isTopmost));
}

void JsSceneSession::OnSessionModalTypeChange(SubWindowModalType subWindowModalType)
{
    const char* const where = __func__;
    auto task = [weakThis = wptr(this), persistentId = persistentId_, subWindowModalType, env = env_, where] {
        auto jsSceneSession = weakThis.promote();
        if (!jsSceneSession || jsSceneSessionMap_.find(persistentId) == jsSceneSessionMap_.end()) {
            TLOGNE(WmsLogTag::WMS_LIFE, "%{public}s jsSceneSession id:%{public}d has been destroyed",
                where, persistentId);
            return;
        }
        auto jsCallBack = jsSceneSession->GetJSCallback(SESSION_MODAL_TYPE_CHANGE_CB);
        if (!jsCallBack) {
            TLOGNE(WmsLogTag::WMS_HIERARCHY, "%{public}s jsCallBack is nullptr", where);
            return;
        }
        napi_value jsSessionModalTypeObj = CreateJsValue(env, subWindowModalType);
        napi_value argv[] = {jsSessionModalTypeObj};
        napi_call_function(env, NapiGetUndefined(env), jsCallBack->GetNapiValue(), ArraySize(argv), argv, nullptr);
    };
    taskScheduler_->PostMainThreadTask(task,
        "OnSessionModalTypeChange: " + std::to_string(static_cast<uint32_t>(subWindowModalType)));
}

void JsSceneSession::OnClick(bool requestFocus)
{
    TLOGD(WmsLogTag::WMS_FOCUS, "[NAPI] id: %{public}d, requestFocus: %{public}u", persistentId_, requestFocus);
    auto task = [weakThis = wptr(this), persistentId = persistentId_, requestFocus, env = env_] {
        auto jsSceneSession = weakThis.promote();
        if (!jsSceneSession || jsSceneSessionMap_.find(persistentId) == jsSceneSessionMap_.end()) {
            TLOGE(WmsLogTag::WMS_LIFE, "OnClick jsSceneSession id:%{public}d has been destroyed",
                persistentId);
            return;
        }
        auto jsCallBack = jsSceneSession->GetJSCallback(CLICK_CB);
        if (!jsCallBack) {
            WLOGFE("[NAPI]jsCallBack is nullptr");
            return;
        }
        napi_value jsRequestFocusObj = CreateJsValue(env, requestFocus);
        napi_value argv[] = {jsRequestFocusObj};
        napi_call_function(env, NapiGetUndefined(env), jsCallBack->GetNapiValue(), ArraySize(argv), argv, nullptr);
    };
    taskScheduler_->PostMainThreadTask(task, "OnClick: requestFocus" + std::to_string(requestFocus));
}

void JsSceneSession::OnContextTransparent()
{
    WLOGFD("[NAPI]");
    auto task = [weakThis = wptr(this), persistentId = persistentId_, env = env_] {
        auto jsSceneSession = weakThis.promote();
        if (!jsSceneSession || jsSceneSessionMap_.find(persistentId) == jsSceneSessionMap_.end()) {
            TLOGE(WmsLogTag::WMS_LIFE, "OnContextTransparent jsSceneSession id:%{public}d has been destroyed",
                persistentId);
            return;
        }
        auto jsCallBack = jsSceneSession->GetJSCallback(CONTEXT_TRANSPARENT_CB);
        if (!jsCallBack) {
            WLOGFE("[NAPI]jsCallBack is nullptr");
            return;
        }
        napi_value argv[] = {};
        napi_call_function(env, NapiGetUndefined(env), jsCallBack->GetNapiValue(), 0, argv, nullptr);
    };
    taskScheduler_->PostMainThreadTask(task, "OnContextTransparent");
}

void JsSceneSession::ChangeSessionVisibilityWithStatusBar(SessionInfo& info, bool visible)
{
    WLOGI("[NAPI]: bundleName %{public}s, moduleName %{public}s, "
        "abilityName %{public}s, appIndex %{public}d, reuse %{public}d, visible %{public}d",
        info.bundleName_.c_str(), info.moduleName_.c_str(), info.abilityName_.c_str(),
        info.appIndex_, info.reuse, visible);
    auto session = weakSession_.promote();
    if (session == nullptr) {
        WLOGFE("session is nullptr, id:%{public}d", persistentId_);
        return;
    }
    std::shared_ptr<SessionInfo> sessionInfo = std::make_shared<SessionInfo>(info);
    auto task = [weakThis = wptr(this), sessionInfo, visible]() {
        auto jsSceneSession = weakThis.promote();
        if (!jsSceneSession) {
            TLOGE(WmsLogTag::WMS_LIFE, "ChangeSessionVisibilityWithStatusBar jsSceneSession is null");
            return;
        }
        jsSceneSession->ChangeSessionVisibilityWithStatusBarInner(sessionInfo, visible);
    };
    taskScheduler_->PostMainThreadTask(task, "ChangeSessionVisibilityWithStatusBar, visible:" +
        std::to_string(visible));
}

void JsSceneSession::ChangeSessionVisibilityWithStatusBarInner(std::shared_ptr<SessionInfo> sessionInfo, bool visible)
{
    std::shared_ptr<NativeReference> jsCallBack = GetJSCallback(CHANGE_SESSION_VISIBILITY_WITH_STATUS_BAR);
    if (!jsCallBack) {
        WLOGFE("[NAPI]jsCallBack is nullptr");
        return;
    }
    if (sessionInfo == nullptr) {
        WLOGFE("[NAPI]sessionInfo is nullptr");
        return;
    }
    napi_value jsSessionInfo = CreateJsSessionInfo(env_, *sessionInfo);
    if (jsSessionInfo == nullptr) {
        WLOGFE("[NAPI]target session info is nullptr");
        return;
    }
    napi_value visibleNapiV = CreateJsValue(env_, visible);
    napi_value argv[] = {jsSessionInfo, visibleNapiV};
    napi_call_function(env_, NapiGetUndefined(env_),
        jsCallBack->GetNapiValue(), ArraySize(argv), argv, nullptr);
}

sptr<SceneSession> JsSceneSession::GenSceneSession(SessionInfo& info)
{
    sptr<SceneSession> sceneSession = nullptr;
    if (info.persistentId_ == 0) {
        auto result = SceneSessionManager::GetInstance().CheckIfReuseSession(info);
        if (result == BrokerStates::BROKER_NOT_START) {
            TLOGE(WmsLogTag::WMS_LIFE, "[NAPI]BrokerStates not started");
            return nullptr;
        }
        if (info.reuse || info.isAtomicService_) {
            TLOGI(WmsLogTag::WMS_LIFE, "session need to be reusesd.");
            if (SceneSessionManager::GetInstance().CheckCollaboratorType(info.collaboratorType_)) {
                sceneSession = SceneSessionManager::GetInstance().FindSessionByAffinity(info.sessionAffinity);
            } else {
                SessionIdentityInfo identityInfo = { info.bundleName_, info.moduleName_, info.abilityName_,
                    info.appIndex_, info.appInstanceKey_, info.windowType_, info.isAtomicService_ };
                sceneSession = SceneSessionManager::GetInstance().GetSceneSessionByIdentityInfo(identityInfo);
            }
        }
        if (sceneSession == nullptr) {
            TLOGI(WmsLogTag::WMS_LIFE, "SceneSession not exist, request a new one.");
            sceneSession = SceneSessionManager::GetInstance().RequestSceneSession(info);
            if (sceneSession == nullptr) {
                TLOGE(WmsLogTag::WMS_LIFE, "RequestSceneSession return nullptr");
                return nullptr;
            }
        } else {
            sceneSession->SetSessionInfo(info);
        }
        info.persistentId_ = sceneSession->GetPersistentId();
        sceneSession->SetSessionInfoPersistentId(sceneSession->GetPersistentId());
    } else {
        sceneSession = SceneSessionManager::GetInstance().GetSceneSession(info.persistentId_);
        if (sceneSession == nullptr) {
            TLOGE(WmsLogTag::WMS_LIFE, "GetSceneSession return nullptr");
            sceneSession = SceneSessionManager::GetInstance().RequestSceneSession(info);
            if (sceneSession == nullptr) {
                TLOGE(WmsLogTag::WMS_LIFE, "retry RequestSceneSession return nullptr");
                return nullptr;
            }
            info.persistentId_ = sceneSession->GetPersistentId();
            sceneSession->SetSessionInfoPersistentId(sceneSession->GetPersistentId());
        } else {
            sceneSession->SetSessionInfo(info);
        }
    }
    return sceneSession;
}

void JsSceneSession::PendingSessionActivation(SessionInfo& info)
{
    TLOGI(WmsLogTag::WMS_LIFE, "[NAPI]bundleName %{public}s, moduleName %{public}s, abilityName %{public}s, "
        "appIndex %{public}d, reuse %{public}d", info.bundleName_.c_str(), info.moduleName_.c_str(),
        info.abilityName_.c_str(), info.appIndex_, info.reuse);
    auto sceneSession = GenSceneSession(info);
    if (sceneSession == nullptr) {
        TLOGE(WmsLogTag::WMS_LIFE, "GenSceneSession failed");
        return;
    }

    if (info.want != nullptr) {
        auto focusedOnShow = info.want->GetBoolParam(AAFwk::Want::PARAM_RESV_WINDOW_FOCUSED, true);
        sceneSession->SetFocusedOnShow(focusedOnShow);
    } else {
        sceneSession->SetFocusedOnShow(true);
    }

    auto callerSession = SceneSessionManager::GetInstance().GetSceneSession(info.callerPersistentId_);
    if (callerSession != nullptr) {
        TLOGI(WmsLogTag::WMS_SCB,
            "isCalledRightlyByCallerId result is: %{public}d", false);
        info.isCalledRightlyByCallerId_ = false;
    }
    std::shared_ptr<SessionInfo> sessionInfo = std::make_shared<SessionInfo>(info);
    auto task = [weakThis = wptr(this), sessionInfo] {
        auto jsSceneSession = weakThis.promote();
        if (jsSceneSession == nullptr) {
            TLOGE(WmsLogTag::WMS_LIFE, "PendingSessionActivation JsSceneSession is null");
            return;
        }
        jsSceneSession->PendingSessionActivationInner(sessionInfo);
    };
    sceneSession->PostLifeCycleTask(task, "PendingSessionActivation", LifeCycleTaskType::START);
    if (info.fullScreenStart_) {
        sceneSession->NotifySessionFullScreen(true);
    }
}

void JsSceneSession::PendingSessionActivationInner(std::shared_ptr<SessionInfo> sessionInfo)
{
    auto task = [weakThis = wptr(this), persistentId = persistentId_, sessionInfo, env_ref = env_] {
        auto jsSceneSession = weakThis.promote();
        if (!jsSceneSession || jsSceneSessionMap_.find(persistentId) == jsSceneSessionMap_.end()) {
            TLOGE(WmsLogTag::WMS_LIFE, "PendingSessionActivationInner jsSceneSession id:%{public}d has been destroyed",
                persistentId);
            return;
        }
        auto jsCallBack = jsSceneSession->GetJSCallback(PENDING_SCENE_CB);
        if (!jsCallBack) {
            TLOGE(WmsLogTag::WMS_LIFE, "[NAPI]jsCallBack is nullptr");
            return;
        }
        if (sessionInfo == nullptr) {
            TLOGE(WmsLogTag::WMS_LIFE, "[NAPI]sessionInfo is nullptr");
            return;
        }
        napi_value jsSessionInfo = CreateJsSessionInfo(env_ref, *sessionInfo);
        if (jsSessionInfo == nullptr) {
            TLOGE(WmsLogTag::WMS_LIFE, "[NAPI]target session info is nullptr");
            return;
        }
        napi_value argv[] = {jsSessionInfo};
        TLOGI(WmsLogTag::WMS_LIFE, "[NAPI]PendingSessionActivationInner task success, id:%{public}d",
            sessionInfo->persistentId_);
        napi_call_function(env_ref, NapiGetUndefined(env_ref),
            jsCallBack->GetNapiValue(), ArraySize(argv), argv, nullptr);
    };
    taskScheduler_->PostMainThreadTask(task, "PendingSessionActivationInner");
}

void JsSceneSession::OnBackPressed(bool needMoveToBackground)
{
    WLOGFI("[NAPI] %{public}d", needMoveToBackground);
    auto task = [weakThis = wptr(this), persistentId = persistentId_, needMoveToBackground, env = env_] {
        auto jsSceneSession = weakThis.promote();
        if (!jsSceneSession || jsSceneSessionMap_.find(persistentId) == jsSceneSessionMap_.end()) {
            TLOGE(WmsLogTag::WMS_LIFE, "OnBackPressed jsSceneSession id:%{public}d has been destroyed",
                persistentId);
            return;
        }
        auto jsCallBack = jsSceneSession->GetJSCallback(BACK_PRESSED_CB);
        if (!jsCallBack) {
            WLOGFE("[NAPI]jsCallBack is nullptr");
            return;
        }
        napi_value jsNeedMoveToBackgroundObj = CreateJsValue(env, needMoveToBackground);
        napi_value argv[] = {jsNeedMoveToBackgroundObj};
        napi_call_function(env, NapiGetUndefined(env), jsCallBack->GetNapiValue(), ArraySize(argv), argv, nullptr);
    };
    taskScheduler_->PostMainThreadTask(task, "OnBackPressed:" + std::to_string(needMoveToBackground));
}

void JsSceneSession::TerminateSession(const SessionInfo& info)
{
    TLOGI(WmsLogTag::WMS_LIFE, "bundle:%{public}s, ability:%{public}s, id:%{public}d",
        info.bundleName_.c_str(), info.abilityName_.c_str(), info.persistentId_);

    std::shared_ptr<SessionInfo> sessionInfo = std::make_shared<SessionInfo>(info);
    auto task = [weakThis = wptr(this), persistentId = persistentId_, sessionInfo, env = env_] {
        auto jsSceneSession = weakThis.promote();
        if (!jsSceneSession || jsSceneSessionMap_.find(persistentId) == jsSceneSessionMap_.end()) {
            TLOGE(WmsLogTag::WMS_LIFE, "TerminateSession jsSceneSession id:%{public}d has been destroyed",
                persistentId);
            return;
        }
        auto jsCallBack = jsSceneSession->GetJSCallback(TERMINATE_SESSION_CB);
        if (!jsCallBack) {
            TLOGE(WmsLogTag::WMS_LIFE, "[NAPI]jsCallBack is nullptr");
            return;
        }
        if (sessionInfo == nullptr) {
            TLOGE(WmsLogTag::WMS_LIFE, "[NAPI]sessionInfo is nullptr");
            return;
        }
        napi_value jsSessionInfo = CreateJsSessionInfo(env, *sessionInfo);
        if (jsSessionInfo == nullptr) {
            TLOGE(WmsLogTag::WMS_LIFE, "[NAPI]target session info is nullptr");
            return;
        }
        napi_value argv[] = {jsSessionInfo};
        napi_call_function(env, NapiGetUndefined(env), jsCallBack->GetNapiValue(), ArraySize(argv), argv, nullptr);
    };
    taskScheduler_->PostMainThreadTask(task, "TerminateSession name:" + info.abilityName_);
}

void JsSceneSession::TerminateSessionNew(const SessionInfo& info, bool needStartCaller, bool isFromBroker)
{
    TLOGI(WmsLogTag::WMS_LIFE, "[NAPI]bundleName = %{public}s, abilityName = %{public}s",
        info.bundleName_.c_str(), info.abilityName_.c_str());
    bool needRemoveSession = false;
    if (!needStartCaller && !isFromBroker) {
        needRemoveSession = true;
    }
    auto task = [weakThis = wptr(this), persistentId = persistentId_, needStartCaller, needRemoveSession, env = env_] {
        auto jsSceneSession = weakThis.promote();
        if (!jsSceneSession || jsSceneSessionMap_.find(persistentId) == jsSceneSessionMap_.end()) {
            TLOGE(WmsLogTag::WMS_LIFE, "TerminateSessionNew jsSceneSession id:%{public}d has been destroyed",
                persistentId);
            return;
        }
        auto jsCallBack = jsSceneSession->GetJSCallback(TERMINATE_SESSION_CB_NEW);
        if (!jsCallBack) {
            TLOGE(WmsLogTag::WMS_LIFE, "[NAPI]jsCallBack is nullptr");
            return;
        }
        napi_value jsNeedStartCaller = CreateJsValue(env, needStartCaller);
        if (jsNeedStartCaller == nullptr) {
            TLOGE(WmsLogTag::WMS_LIFE, "[NAPI]jsNeedStartCaller is nullptr");
            return;
        }
        napi_value jsNeedRemoveSession = CreateJsValue(env, needRemoveSession);
        if (jsNeedRemoveSession == nullptr) {
            TLOGE(WmsLogTag::WMS_LIFE, "[NAPI]jsNeedRemoveSession is nullptr");
            return;
        }
        napi_value argv[] = {jsNeedStartCaller, jsNeedRemoveSession};
        napi_call_function(env, NapiGetUndefined(env), jsCallBack->GetNapiValue(), ArraySize(argv), argv, nullptr);
    };
    taskScheduler_->PostMainThreadTask(task, "TerminateSessionNew, name:" + info.abilityName_);
}

void JsSceneSession::TerminateSessionTotal(const SessionInfo& info, TerminateType terminateType)
{
    TLOGI(WmsLogTag::WMS_LIFE, "[NAPI]bundleName = %{public}s, abilityName = %{public}s, terminateType = %{public}d",
        info.bundleName_.c_str(), info.abilityName_.c_str(), static_cast<int32_t>(terminateType));

    auto task = [weakThis = wptr(this), persistentId = persistentId_, terminateType, env = env_] {
        auto jsSceneSession = weakThis.promote();
        if (!jsSceneSession || jsSceneSessionMap_.find(persistentId) == jsSceneSessionMap_.end()) {
            TLOGE(WmsLogTag::WMS_LIFE, "TerminateSessionTotal jsSceneSession id:%{public}d has been destroyed",
                persistentId);
            return;
        }
        auto jsCallBack = jsSceneSession->GetJSCallback(TERMINATE_SESSION_CB_TOTAL);
        if (!jsCallBack) {
            TLOGE(WmsLogTag::WMS_LIFE, "[NAPI]jsCallBack is nullptr");
            return;
        }
        napi_value jsTerminateType = CreateJsValue(env, static_cast<int32_t>(terminateType));
        if (jsTerminateType == nullptr) {
            TLOGE(WmsLogTag::WMS_LIFE, "[NAPI]jsTerminateType is nullptr");
            return;
        }
        napi_value argv[] = {jsTerminateType};
        napi_call_function(env, NapiGetUndefined(env), jsCallBack->GetNapiValue(), ArraySize(argv), argv, nullptr);
    };
    taskScheduler_->PostMainThreadTask(task, "TerminateSessionTotal:name:" + info.abilityName_);
}

void JsSceneSession::UpdateSessionLabel(const std::string& label)
{
    WLOGFI("[NAPI]");
    auto task = [weakThis = wptr(this), persistentId = persistentId_, label, env = env_] {
        auto jsSceneSession = weakThis.promote();
        if (!jsSceneSession || jsSceneSessionMap_.find(persistentId) == jsSceneSessionMap_.end()) {
            TLOGE(WmsLogTag::WMS_LIFE, "UpdateSessionLabel jsSceneSession id:%{public}d has been destroyed",
                persistentId);
            return;
        }
        auto jsCallBack = jsSceneSession->GetJSCallback(UPDATE_SESSION_LABEL_CB);
        if (!jsCallBack) {
            WLOGFE("[NAPI]jsCallBack is nullptr");
            return;
        }
        napi_value jsLabel = CreateJsValue(env, label);
        if (jsLabel == nullptr) {
            WLOGFE("[NAPI]this target jsLabel is nullptr");
            return;
        }
        napi_value argv[] = {jsLabel};
        napi_call_function(env, NapiGetUndefined(env), jsCallBack->GetNapiValue(), ArraySize(argv), argv, nullptr);
    };
    taskScheduler_->PostMainThreadTask(task, "UpdateSessionLabel");
}

void JsSceneSession::ProcessUpdateSessionLabelRegister()
{
    WLOGFD("in");
    NofitySessionLabelUpdatedFunc func = [weakThis = wptr(this)](const std::string& label) {
        auto jsSceneSession = weakThis.promote();
        if (!jsSceneSession) {
            TLOGE(WmsLogTag::WMS_LIFE, "ProcessUpdateSessionLabelRegister jsSceneSession is null");
            return;
        }
        jsSceneSession->UpdateSessionLabel(label);
    };
    auto session = weakSession_.promote();
    if (session == nullptr) {
        WLOGFE("session is nullptr, id:%{public}d", persistentId_);
        return;
    }
    session->SetUpdateSessionLabelListener(func);
    WLOGFD("success");
}

void JsSceneSession::ProcessUpdateSessionIconRegister()
{
    WLOGFD("in");
    NofitySessionIconUpdatedFunc func = [weakThis = wptr(this)](const std::string& iconPath) {
        auto jsSceneSession = weakThis.promote();
        if (!jsSceneSession) {
            TLOGE(WmsLogTag::WMS_LIFE, "ProcessUpdateSessionIconRegister jsSceneSession is null");
            return;
        }
        jsSceneSession->UpdateSessionIcon(iconPath);
    };
    auto session = weakSession_.promote();
    if (session == nullptr) {
        WLOGFE("session is nullptr, id:%{public}d", persistentId_);
        return;
    }
    session->SetUpdateSessionIconListener(func);
    WLOGFD("success");
}

void JsSceneSession::UpdateSessionIcon(const std::string& iconPath)
{
    WLOGFI("[NAPI]");
    auto task = [weakThis = wptr(this), persistentId = persistentId_, iconPath, env = env_] {
        auto jsSceneSession = weakThis.promote();
        if (!jsSceneSession || jsSceneSessionMap_.find(persistentId) == jsSceneSessionMap_.end()) {
            TLOGE(WmsLogTag::WMS_LIFE, "UpdateSessionIcon jsSceneSession id:%{public}d has been destroyed",
                persistentId);
            return;
        }
        auto jsCallBack = jsSceneSession->GetJSCallback(UPDATE_SESSION_ICON_CB);
        if (!jsCallBack) {
            WLOGFE("[NAPI]jsCallBack is nullptr");
            return;
        }
        napi_value jsIconPath = CreateJsValue(env, iconPath);
        if (jsIconPath == nullptr) {
            WLOGFE("[NAPI]this target jsIconPath is nullptr");
            return;
        }
        napi_value argv[] = {jsIconPath};
        napi_call_function(env, NapiGetUndefined(env), jsCallBack->GetNapiValue(), ArraySize(argv), argv, nullptr);
    };
    taskScheduler_->PostMainThreadTask(task, "UpdateSessionIcon");
}

void JsSceneSession::OnSessionException(const SessionInfo& info, bool needRemoveSession)
{
    TLOGI(WmsLogTag::WMS_LIFE, "[NAPI]bundleName = %{public}s, abilityName = %{public}s",
        info.bundleName_.c_str(), info.abilityName_.c_str());

    std::shared_ptr<SessionInfo> sessionInfo = std::make_shared<SessionInfo>(info);
    auto task = [weakThis = wptr(this), persistentId = persistentId_, sessionInfo, needRemoveSession, env = env_] {
        auto jsSceneSession = weakThis.promote();
        if (!jsSceneSession || jsSceneSessionMap_.find(persistentId) == jsSceneSessionMap_.end()) {
            TLOGE(WmsLogTag::WMS_LIFE, "OnSessionException jsSceneSession id:%{public}d has been destroyed",
                persistentId);
            return;
        }
        auto jsCallBack = jsSceneSession->GetJSCallback(SESSION_EXCEPTION_CB);
        if (!jsCallBack) {
            TLOGE(WmsLogTag::WMS_LIFE, "[NAPI]jsCallBack is nullptr");
            return;
        }
        if (sessionInfo == nullptr) {
            TLOGE(WmsLogTag::WMS_LIFE, "[NAPI]sessionInfo is nullptr");
            return;
        }
        napi_value jsSessionInfo = CreateJsSessionInfo(env, *sessionInfo);
        napi_value jsNeedRemoveSession = CreateJsValue(env, needRemoveSession);
        if (jsSessionInfo == nullptr) {
            TLOGE(WmsLogTag::WMS_LIFE, "[NAPI]target session info is nullptr");
            return;
        }
        napi_value argv[] = {jsSessionInfo, jsNeedRemoveSession};
        napi_call_function(env, NapiGetUndefined(env), jsCallBack->GetNapiValue(), ArraySize(argv), argv, nullptr);
    };
    taskScheduler_->PostMainThreadTask(task, "OnSessionException, name" + info.bundleName_);
}

void JsSceneSession::PendingSessionToForeground(const SessionInfo& info)
{
    TLOGI(WmsLogTag::WMS_LIFE, "[NAPI]bundleName = %{public}s, abilityName = %{public}s",
        info.bundleName_.c_str(), info.abilityName_.c_str());

    std::shared_ptr<SessionInfo> sessionInfo = std::make_shared<SessionInfo>(info);
    auto task = [weakThis = wptr(this), persistentId = persistentId_, sessionInfo, env = env_] {
        auto jsSceneSession = weakThis.promote();
        if (!jsSceneSession || jsSceneSessionMap_.find(persistentId) == jsSceneSessionMap_.end()) {
            TLOGE(WmsLogTag::WMS_LIFE, "PendingSessionToForeground jsSceneSession id:%{public}d has been destroyed",
                persistentId);
            return;
        }
        auto jsCallBack = jsSceneSession->GetJSCallback(PENDING_SESSION_TO_FOREGROUND_CB);
        if (!jsCallBack) {
            TLOGE(WmsLogTag::WMS_LIFE, "[NAPI]jsCallBack is nullptr");
            return;
        }
        if (sessionInfo == nullptr) {
            TLOGE(WmsLogTag::WMS_LIFE, "[NAPI]sessionInfo is nullptr");
            return;
        }
        napi_value jsSessionInfo = CreateJsSessionInfo(env, *sessionInfo);
        if (jsSessionInfo == nullptr) {
            TLOGE(WmsLogTag::WMS_LIFE, "[NAPI]target session info is nullptr");
            return;
        }
        napi_value argv[] = {jsSessionInfo};
        napi_call_function(env, NapiGetUndefined(env), jsCallBack->GetNapiValue(), ArraySize(argv), argv, nullptr);
    };
    taskScheduler_->PostMainThreadTask(task, "PendingSessionToForeground:" + info.bundleName_);
}

void JsSceneSession::PendingSessionToBackgroundForDelegator(const SessionInfo& info, bool shouldBackToCaller)
{
    TLOGI(WmsLogTag::WMS_LIFE,
        "[NAPI]bundleName = %{public}s, abilityName = %{public}s, shouldBackToCaller = %{public}d",
        info.bundleName_.c_str(), info.abilityName_.c_str(), shouldBackToCaller);

    std::shared_ptr<SessionInfo> sessionInfo = std::make_shared<SessionInfo>(info);
    auto task = [weakThis = wptr(this), persistentId = persistentId_, sessionInfo, env = env_, shouldBackToCaller] {
        auto jsSceneSession = weakThis.promote();
        if (!jsSceneSession || jsSceneSessionMap_.find(persistentId) == jsSceneSessionMap_.end()) {
            TLOGNE(WmsLogTag::WMS_LIFE, "jsSceneSession id:%{public}d has been destroyed", persistentId);
            return;
        }
        auto jsCallBack = jsSceneSession->GetJSCallback(PENDING_SESSION_TO_BACKGROUND_FOR_DELEGATOR_CB);
        if (!jsCallBack) {
            TLOGNE(WmsLogTag::WMS_LIFE, "[NAPI]jsCallBack is nullptr");
            return;
        }
        napi_value jsSessionInfo = CreateJsSessionInfo(env, *sessionInfo);
        napi_value jsShouldBackToCaller = CreateJsValue(env, shouldBackToCaller);
        if (jsSessionInfo == nullptr) {
            TLOGNE(WmsLogTag::WMS_LIFE, "[NAPI]target session info is nullptr");
            return;
        }
        napi_value argv[] = {jsSessionInfo, jsShouldBackToCaller};
        napi_call_function(env, NapiGetUndefined(env), jsCallBack->GetNapiValue(), ArraySize(argv), argv, nullptr);
    };
    taskScheduler_->PostMainThreadTask(task, "PendingSessionToBackgroundForDelegator, name:" + info.bundleName_);
}

void JsSceneSession::OnSystemBarPropertyChange(const std::unordered_map<WindowType, SystemBarProperty>& propertyMap)
{
    TLOGD(WmsLogTag::WMS_IMMS, "[NAPI]");
    auto task = [weakThis = wptr(this), persistentId = persistentId_, propertyMap, env = env_] {
        auto jsSceneSession = weakThis.promote();
        if (!jsSceneSession || jsSceneSessionMap_.find(persistentId) == jsSceneSessionMap_.end()) {
            TLOGE(WmsLogTag::WMS_LIFE, "OnSystemBarPropertyChange jsSceneSession id:%{public}d has been destroyed",
                persistentId);
            return;
        }
        auto jsCallBack = jsSceneSession->GetJSCallback(SYSTEMBAR_PROPERTY_CHANGE_CB);
        if (jsCallBack == nullptr) {
            WLOGFE("[NAPI]jsCallBack is nullptr");
            return;
        }
        napi_value jsSessionStateObj = CreateJsSystemBarPropertyArrayObject(env, propertyMap);
        if (jsSessionStateObj == nullptr) {
            TLOGE(WmsLogTag::WMS_IMMS, "[NAPI]jsSessionStateObj is nullptr");
            return;
        }
        napi_value argv[] = {jsSessionStateObj};
        napi_call_function(env, NapiGetUndefined(env), jsCallBack->GetNapiValue(), ArraySize(argv), argv, nullptr);
    };
    taskScheduler_->PostMainThreadTask(task, "OnSystemBarPropertyChange");
}

void JsSceneSession::OnNeedAvoid(bool status)
{
    TLOGD(WmsLogTag::WMS_IMMS, "[NAPI] %{public}d", status);
    auto task = [weakThis = wptr(this), persistentId = persistentId_, needAvoid = status, env = env_] {
        auto jsSceneSession = weakThis.promote();
        if (!jsSceneSession || jsSceneSessionMap_.find(persistentId) == jsSceneSessionMap_.end()) {
            TLOGE(WmsLogTag::WMS_LIFE, "OnNeedAvoid jsSceneSession id:%{public}d has been destroyed",
                persistentId);
            return;
        }
        auto jsCallBack = jsSceneSession->GetJSCallback(NEED_AVOID_CB);
        if (jsCallBack == nullptr) {
            WLOGFE("[NAPI]jsCallBack is nullptr");
            return;
        }
        napi_value jsSessionStateObj = CreateJsValue(env, needAvoid);
        napi_value argv[] = {jsSessionStateObj};
        napi_call_function(env, NapiGetUndefined(env), jsCallBack->GetNapiValue(), ArraySize(argv), argv, nullptr);
    };
    taskScheduler_->PostMainThreadTask(task, "OnNeedAvoid:" + std::to_string(status));
}

void JsSceneSession::OnIsCustomAnimationPlaying(bool status)
{
    WLOGFI("[NAPI] %{public}d", status);
    auto task = [weakThis = wptr(this), persistentId = persistentId_, isPlaying = status, env = env_] {
        auto jsSceneSession = weakThis.promote();
        if (!jsSceneSession || jsSceneSessionMap_.find(persistentId) == jsSceneSessionMap_.end()) {
            TLOGE(WmsLogTag::WMS_LIFE, "OnIsCustomAnimationPlaying jsSceneSession id:%{public}d has been destroyed",
                persistentId);
            return;
        }
        auto jsCallBack = jsSceneSession->GetJSCallback(CUSTOM_ANIMATION_PLAYING_CB);
        if (jsCallBack == nullptr) {
            WLOGFE("[NAPI]jsCallBack is nullptr");
            return;
        }
        napi_value jsSessionStateObj = CreateJsValue(env, isPlaying);
        napi_value argv[] = {jsSessionStateObj};
        napi_call_function(env, NapiGetUndefined(env), jsCallBack->GetNapiValue(), ArraySize(argv), argv, nullptr);
    };
    taskScheduler_->PostMainThreadTask(task, "OnIsCustomAnimationPlaying:" + std::to_string(status));
}

void JsSceneSession::OnShowWhenLocked(bool showWhenLocked)
{
    WLOGFI("[NAPI] %{public}d", showWhenLocked);
    auto task = [weakThis = wptr(this), persistentId = persistentId_, flag = showWhenLocked, env = env_] {
        auto jsSceneSession = weakThis.promote();
        if (!jsSceneSession || jsSceneSessionMap_.find(persistentId) == jsSceneSessionMap_.end()) {
            TLOGE(WmsLogTag::WMS_LIFE, "OnShowWhenLocked jsSceneSession id:%{public}d has been destroyed",
                persistentId);
            return;
        }
        auto jsCallBack = jsSceneSession->GetJSCallback(SHOW_WHEN_LOCKED_CB);
        if (jsCallBack == nullptr) {
            WLOGFE("[NAPI]jsCallBack is nullptr");
            return;
        }
        napi_value jsSessionStateObj = CreateJsValue(env, flag);
        napi_value argv[] = {jsSessionStateObj};
        napi_call_function(env, NapiGetUndefined(env), jsCallBack->GetNapiValue(), ArraySize(argv), argv, nullptr);
    };
    taskScheduler_->PostMainThreadTask(task, "OnShowWhenLocked:" +std::to_string(showWhenLocked));
}

void JsSceneSession::OnReuqestedOrientationChange(uint32_t orientation)
{
    WLOGFI("[NAPI] %{public}u", orientation);

    auto task = [weakThis = wptr(this), persistentId = persistentId_, rotation = orientation, env = env_] {
        auto jsSceneSession = weakThis.promote();
        if (!jsSceneSession || jsSceneSessionMap_.find(persistentId) == jsSceneSessionMap_.end()) {
            TLOGE(WmsLogTag::WMS_LIFE, "OnReuqestedOrientationChange jsSceneSession id:%{public}d has been destroyed",
                persistentId);
            return;
        }
        auto jsCallBack = jsSceneSession->GetJSCallback(REQUESTED_ORIENTATION_CHANGE_CB);
        if (jsCallBack == nullptr) {
            WLOGFE("[NAPI]jsCallBack is nullptr");
            return;
        }
        napi_value jsSessionRotationObj = CreateJsValue(env, rotation);
        napi_value argv[] = {jsSessionRotationObj};
        napi_call_function(env, NapiGetUndefined(env), jsCallBack->GetNapiValue(), ArraySize(argv), argv, nullptr);
        WLOGFI("[NAPI]change rotation success %{public}u", rotation);
    };
    taskScheduler_->PostMainThreadTask(task, "OnReuqestedOrientationChange:orientation" +std::to_string(orientation));
}

napi_value JsSceneSession::OnSetShowRecent(napi_env env, napi_callback_info info)
{
    auto session = weakSession_.promote();
    if (session == nullptr) {
        WLOGFE("[NAPI]session is null, id:%{public}d", persistentId_);
        return NapiGetUndefined(env);
    }
    size_t argc = 4;
    napi_value argv[4] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    bool showRecent = true;
    if (argc == ARGC_ONE && GetType(env, argv[0]) == napi_boolean) {
        if (!ConvertFromJsValue(env, argv[0], showRecent)) {
            WLOGFE("[NAPI]Failed to convert parameter to bool");
            return NapiGetUndefined(env);
        }
    }
    session->SetShowRecent(showRecent);
    return NapiGetUndefined(env);
}

napi_value JsSceneSession::OnSetZOrder(napi_env env, napi_callback_info info)
{
    size_t argc = 4;
    napi_value argv[4] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc != 1) { // 1: params num
        WLOGFE("[NAPI]Argc is invalid: %{public}zu", argc);
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    uint32_t zOrder;
    if (!ConvertFromJsValue(env, argv[0], zOrder)) {
        WLOGFE("[NAPI]Failed to convert parameter to uint32_t");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    auto session = weakSession_.promote();
    if (session == nullptr) {
        WLOGFW("[NAPI]session is null, id:%{public}d", persistentId_);
        return NapiGetUndefined(env);
    }
    session->SetZOrder(zOrder);
    return NapiGetUndefined(env);
}

napi_value JsSceneSession::OnSetFloatingScale(napi_env env, napi_callback_info info)
{
    size_t argc = 4;
    napi_value argv[4] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc < 1) { // 1: params num
        WLOGFE("[NAPI]Argc is invalid: %{public}zu", argc);
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    double_t floatingScale = 1.0;
    if (!ConvertFromJsValue(env, argv[0], floatingScale)) {
        WLOGFE("[NAPI]Failed to convert parameter to bool");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    auto session = weakSession_.promote();
    if (session == nullptr) {
        WLOGFE("[NAPI]session is nullptr, id:%{public}d", persistentId_);
        return NapiGetUndefined(env);
    }
    session->SetFloatingScale(static_cast<float_t>(floatingScale));
    return NapiGetUndefined(env);
}

napi_value JsSceneSession::OnSetIsMidScene(napi_env env, napi_callback_info info)
{
    size_t argc = ARGC_FOUR;
    napi_value argv[ARGC_FOUR] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc < ARGC_ONE) {
        TLOGE(WmsLogTag::WMS_MULTI_WINDOW, "[NAPI]Argc is invalid: %{public}zu", argc);
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }

    bool isMidScene = false;
    if (!ConvertFromJsValue(env, argv[0], isMidScene)) {
        TLOGE(WmsLogTag::WMS_MULTI_WINDOW, "[NAPI]Failed to convert parameter to isMidScene");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }

    auto session = weakSession_.promote();
    if (session == nullptr) {
        TLOGE(WmsLogTag::WMS_MULTI_WINDOW, "[NAPI]session is nullptr, id:%{public}d", persistentId_);
        return NapiGetUndefined(env);
    }
    session->SetIsMidScene(isMidScene);
    return NapiGetUndefined(env);
}

napi_value JsSceneSession::OnSetSCBKeepKeyboard(napi_env env, napi_callback_info info)
{
    size_t argc = 4;
    napi_value argv[4] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc < 1) {
        WLOGFE("[NAPI]Argc is invalid: %{public}zu", argc);
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    bool scbKeepKeyboardFlag = false;
    if (!ConvertFromJsValue(env, argv[0], scbKeepKeyboardFlag)) {
        WLOGFE("[NAPI]Failed to convert parameter to bool");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    auto session = weakSession_.promote();
    if (session == nullptr) {
        WLOGFE("[NAPI]session is nullptr, id:%{public}d", persistentId_);
        return NapiGetUndefined(env);
    }
    session->SetSCBKeepKeyboard(scbKeepKeyboardFlag);
    return NapiGetUndefined(env);
}

napi_value JsSceneSession::OnSetOffset(napi_env env, napi_callback_info info)
{
    size_t argc = 4;
    napi_value argv[4] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc != 2) {
        WLOGFE("[NAPI]Argc count is invalid: %{public}zu", argc);
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
                                      "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }

    double offsetX = 0.0f;
    if (!ConvertFromJsValue(env, argv[0], offsetX)) {
        WLOGFE("[NAPI]Failed to convert parameter to double");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
                                      "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }

    double offsetY = 0.0f;
    if (!ConvertFromJsValue(env, argv[1], offsetY)) {
        WLOGFE("[NAPI]Failed to convert parameter to double");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
                                      "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }

    auto session = weakSession_.promote();
    if (session == nullptr) {
        WLOGFE("[NAPI]session is nullptr, id:%{public}d", persistentId_);
        return NapiGetUndefined(env);
    }
    session->SetOffset(static_cast<float>(offsetX), static_cast<float>(offsetY));
    return NapiGetUndefined(env);
}

napi_value JsSceneSession::OnSetExitSplitOnBackground(napi_env env, napi_callback_info info)
{
    size_t argc = ARGC_FOUR;
    napi_value argv[ARGC_FOUR] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc != 1) {
        TLOGE(WmsLogTag::WMS_MULTI_WINDOW, "[NAPI]Argc count is invalid: %{public}zu", argc);
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
                                      "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    bool isExitSplitOnBackground = false;
    if (!ConvertFromJsValue(env, argv[0], isExitSplitOnBackground)) {
        TLOGE(WmsLogTag::WMS_MULTI_WINDOW, "[NAPI]Failed to convert parameter to bool");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
                                      "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    auto session = weakSession_.promote();
    if (session == nullptr) {
        TLOGE(WmsLogTag::WMS_MULTI_WINDOW, "[NAPI]session is nullptr, id:%{public}d", persistentId_);
        return NapiGetUndefined(env);
    }
    session->SetExitSplitOnBackground(isExitSplitOnBackground);
    return NapiGetUndefined(env);
}

napi_value JsSceneSession::OnSetWaterMarkFlag(napi_env env, napi_callback_info info)
{
    size_t argc = ARGC_FOUR;
    napi_value argv[ARGC_FOUR] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc < ARGC_ONE) {
        TLOGE(WmsLogTag::WMS_SCB, "[NAPI]Argc is invalid: %{public}zu", argc);
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    bool isWaterMarkAdded = false;
    if (!ConvertFromJsValue(env, argv[0], isWaterMarkAdded)) {
        TLOGE(WmsLogTag::WMS_SCB, "[NAPI]Failed to convert parameter to bool");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    auto session = weakSession_.promote();
    if (session == nullptr) {
        TLOGE(WmsLogTag::WMS_SCB, "[NAPI]session is nullptr, id:%{public}d", persistentId_);
        return NapiGetUndefined(env);
    }
    uint32_t currFlag = session->GetSessionProperty()->GetWindowFlags();
    if (isWaterMarkAdded) {
        currFlag = currFlag | static_cast<uint32_t>(WindowFlag::WINDOW_FLAG_WATER_MARK);
    } else {
        currFlag = currFlag & ~(static_cast<uint32_t>(WindowFlag::WINDOW_FLAG_WATER_MARK));
    }
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    property->SetSystemCalling(true);
    property->SetWindowFlags(currFlag);
    SceneSessionManager::GetInstance().SetWindowFlags(session, property);
    return NapiGetUndefined(env);
}

void JsSceneSession::ProcessPrepareClosePiPSessionRegister()
{
    auto sessionchangeCallback = sessionchangeCallback_.promote();
    if (sessionchangeCallback == nullptr) {
        TLOGE(WmsLogTag::WMS_PIP, "sessionchangeCallback is nullptr");
        return;
    }
    sessionchangeCallback->onPrepareClosePiPSession_ = [weakThis = wptr(this)] {
        auto jsSceneSession = weakThis.promote();
        if (!jsSceneSession) {
            TLOGE(WmsLogTag::WMS_LIFE, "OnPrepareClosePiPSession jsSceneSession is null");
            return;
        }
        jsSceneSession->OnPrepareClosePiPSession();
    };
    TLOGD(WmsLogTag::WMS_PIP, "success");
}

void JsSceneSession::OnPrepareClosePiPSession()
{
    TLOGI(WmsLogTag::WMS_PIP, "[NAPI]");
    auto task = [weakThis = wptr(this), persistentId = persistentId_, env = env_] {
        auto jsSceneSession = weakThis.promote();
        if (!jsSceneSession || jsSceneSessionMap_.find(persistentId) == jsSceneSessionMap_.end()) {
            TLOGE(WmsLogTag::WMS_LIFE, "OnPrepareClosePiPSession jsSceneSession id:%{public}d has been destroyed",
                persistentId);
            return;
        }
        auto jsCallBack = jsSceneSession->GetJSCallback(PREPARE_CLOSE_PIP_SESSION);
        if (!jsCallBack) {
            TLOGE(WmsLogTag::WMS_PIP, "[NAPI]jsCallBack is nullptr");
            return;
        }
        napi_value argv[] = {};
        napi_call_function(env, NapiGetUndefined(env), jsCallBack->GetNapiValue(), 0, argv, nullptr);
    };
    taskScheduler_->PostMainThreadTask(task, "OnPrepareClosePiPSession");
}

napi_value JsSceneSession::OnSetSystemActive(napi_env env, napi_callback_info info)
{
    size_t argc = 4;
    napi_value argv[4] = { nullptr };
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc < 1) {
       WLOGFE("[NAPI]argc is invalid : %{public}zu", argc);
       napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
       "Input parameter is missing or invalid"));
       return NapiGetUndefined(env);
    }
    bool scbSystemActive = false;
    if (!ConvertFromJsValue(env, argv[0], scbSystemActive)){
        WLOGFE("[NAPI] Failed to convert parameter to bool");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
        "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    auto session = weakSession_.promote();
    if (session == nullptr) {
        WLOGFE("[NAPI] session is null, id:%{public}d", persistentId_);
        return NapiGetUndefined(env);
    }
    session->SetSystemActive(scbSystemActive);
    return NapiGetUndefined(env);
}

napi_value JsSceneSession::OnSetTouchable(napi_env env, napi_callback_info info)
{
    size_t argc = 4;
    napi_value argv[4] = { nullptr };
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc != 1) {
        WLOGFE("[NAPI] Argc is invalid: %{public}zu", argc);
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input Parameter is missing or invalid" ));
        return NapiGetUndefined(env);
    }

    bool touchable = false;
    if (!ConvertFromJsValue(env, argv[0], touchable)) {
      WLOGFE("[NAPI] Failed to  convert parameter to touchable");
      napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
        "Input parameter is missing or invalid" ));
      return NapiGetUndefined(env);
    }

    auto session = weakSession_.promote();
    if (session == nullptr) {
        WLOGFE("[NAPI] session is null, id:%{public}d", persistentId_);
        return NapiGetUndefined(env);
    }

    session->SetSystemTouchable(touchable);
    return NapiGetUndefined(env);
}

napi_value JsSceneSession::OnSetRectChangeBySystem(napi_env env, napi_callback_info info)
{
    TLOGI(WmsLogTag::WMS_EVENT, "[NAPI]");
    size_t argc = ARG_COUNT_4;
    napi_value argv[ARG_COUNT_4] = { nullptr };
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc != 1) {
        WLOGFE("[NAPI] Argc is invalid: %{public}zu", argc);
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input Parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }

    bool rectChangeBySystem = false;
    if (!ConvertFromJsValue(env, argv[0], rectChangeBySystem)) {
        WLOGFE("[NAPI] Failed to  convert parameter to rectChangeBySystem");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }

    auto session = weakSession_.promote();
    if (session == nullptr) {
        WLOGFE("[NAPI] session is null, id:%{public}d", persistentId_);
        return NapiGetUndefined(env);
    }

    session->SetRectChangeBySystem(rectChangeBySystem);
    return NapiGetUndefined(env);
}

napi_value JsSceneSession::SetScale(napi_env env, napi_callback_info info)
{
    if (Session::IsScbCoreEnabled()) {
        return nullptr;
    }
    WLOGI("[NAPI]SetScale");
    JsSceneSession* me = CheckParamsAndGetThis<JsSceneSession>(env, info);
    return (me != nullptr) ? me->OnSetScale(env, info) : nullptr;
}

napi_value JsSceneSession::OnSetScale(napi_env env, napi_callback_info info)
{
    size_t argc = ARG_COUNT_4;
    napi_value argv[ARG_COUNT_4] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc < ARG_COUNT_4) { // ARG_COUNT_4: params num
        WLOGFE("[NAPI]Argc is invalid: %{public}zu", argc);
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    double_t scaleX = 1.0;
    if (!ConvertFromJsValue(env, argv[ARG_INDEX_0], scaleX)) {
        WLOGFE("[NAPI]Failed to convert parameter to scaleX");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    double_t scaleY = 1.0;
    if (!ConvertFromJsValue(env, argv[ARG_INDEX_1], scaleY)) {
        WLOGFE("[NAPI]Failed to convert parameter to scaleY");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    double_t pivotX = 0.0;
    if (!ConvertFromJsValue(env, argv[ARG_INDEX_2], pivotX)) {
        WLOGFE("[NAPI]Failed to convert parameter to pivotX");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    double_t pivotY = 0.0;
    if (!ConvertFromJsValue(env, argv[ARG_INDEX_3], pivotY)) {
        WLOGFE("[NAPI]Failed to convert parameter to pivotY");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    auto session = weakSession_.promote();
    if (session == nullptr) {
        WLOGFE("[NAPI]session is nullptr, id:%{public}d", persistentId_);
        return NapiGetUndefined(env);
    }
    session->SetScale(static_cast<float_t>(scaleX), static_cast<float_t>(scaleY), static_cast<float_t>(pivotX),
        static_cast<float_t>(pivotY));
    return NapiGetUndefined(env);
}

napi_value JsSceneSession::SetWindowLastSafeRect(napi_env env, napi_callback_info info)
{
    TLOGI(WmsLogTag::WMS_LAYOUT, "[NAPI]");
    JsSceneSession* me = CheckParamsAndGetThis<JsSceneSession>(env, info);
    return (me != nullptr) ? me->OnSetWindowLastSafeRect(env, info) : nullptr;
}

napi_value JsSceneSession::OnSetWindowLastSafeRect(napi_env env, napi_callback_info info)
{
    size_t argc = ARG_COUNT_4;
    napi_value argv[ARG_COUNT_4] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc < ARG_COUNT_4) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "[NAPI]Argc is invalid: %{public}zu", argc);
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    int32_t left = 0;
    if (!ConvertFromJsValue(env, argv[ARG_INDEX_0], left)) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "[NAPI]Failed to convert parameter to left");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    int32_t top = 0;
    if (!ConvertFromJsValue(env, argv[ARG_INDEX_1], top)) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "[NAPI]Failed to convert parameter to top");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    int32_t width = 0;
    if (!ConvertFromJsValue(env, argv[ARG_INDEX_2], width)) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "[NAPI]Failed to convert parameter to width");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    int32_t height = 0;
    if (!ConvertFromJsValue(env, argv[ARG_INDEX_3], height)) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "[NAPI]Failed to convert parameter to height");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    auto session = weakSession_.promote();
    if (session == nullptr) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "[NAPI]session is nullptr, id:%{public}d", persistentId_);
        return NapiGetUndefined(env);
    }
    WSRect lastRect = { left, top, width, height };
    session->SetSessionRequestRect(lastRect);
    return NapiGetUndefined(env);
}

napi_value JsSceneSession::SetMovable(napi_env env, napi_callback_info info)
{
    JsSceneSession* me = CheckParamsAndGetThis<JsSceneSession>(env, info);
    return (me != nullptr) ? me->OnSetMovable(env, info) : nullptr;
}

napi_value JsSceneSession::OnSetMovable(napi_env env, napi_callback_info info)
{
    size_t argc = ARGC_FOUR;
    napi_value argv[ARGC_FOUR] = { nullptr };
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc != ARGC_ONE) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "[NAPI] Argc is invalid: %{public}zu", argc);
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input Parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }

    bool movable = true;
    if (!ConvertFromJsValue(env, argv[0], movable)) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "[NAPI] Failed to convert parameter to movable");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }

    auto session = weakSession_.promote();
    if (session == nullptr) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "[NAPI] session is nullptr, id:%{public}d", persistentId_);
        return NapiGetUndefined(env);
    }
    session->SetMovable(movable);
    return NapiGetUndefined(env);
}

napi_value JsSceneSession::SetSplitButtonVisible(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::WMS_LAYOUT, "[NAPI]");
    JsSceneSession* me = CheckParamsAndGetThis<JsSceneSession>(env, info);
    return (me != nullptr) ? me->OnSetSplitButtonVisible(env, info) : nullptr;
}

napi_value JsSceneSession::OnSetSplitButtonVisible(napi_env env, napi_callback_info info)
{
    size_t argc = ARGC_FOUR;
    napi_value argv[ARGC_FOUR] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc < ARGC_ONE) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "[NAPI]Argc count is invalid: %{public}zu", argc);
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
                                      "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    bool isVisible = true;
    if (!ConvertFromJsValue(env, argv[0], isVisible)) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "[NAPI]Failed to convert parameter to bool");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
                                      "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }

    auto session = weakSession_.promote();
    if (session == nullptr) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "[NAPI]session is nullptr, id:%{public}d", persistentId_);
        return NapiGetUndefined(env);
    }
    session->SetSplitButtonVisible(isVisible);
    return NapiGetUndefined(env);
}

napi_value JsSceneSession::RequestHideKeyboard(napi_env env, napi_callback_info info)
{
    WLOGFD("[NAPI]");
    JsSceneSession* me = CheckParamsAndGetThis<JsSceneSession>(env, info);
    return (me != nullptr) ? me->OnRequestHideKeyboard(env, info) : nullptr;
}

napi_value JsSceneSession::OnRequestHideKeyboard(napi_env env, napi_callback_info info)
{
    WLOGFI("[NAPI]");
    auto session = weakSession_.promote();
    if (session == nullptr) {
        WLOGFE("[NAPI]session is nullptr, id:%{public}d", persistentId_);
        return NapiGetUndefined(env);
    }
    session->RequestHideKeyboard();
    return NapiGetUndefined(env);
}

napi_value JsSceneSession::OnSetPipActionEvent(napi_env env, napi_callback_info info)
{
    size_t argc = 4;
    napi_value argv[4] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc < 1) {
        TLOGE(WmsLogTag::WMS_PIP, "[NAPI]Argc count is invalid: %{public}zu", argc);
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
                                      "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    std::string action;
    if (!ConvertFromJsValue(env, argv[0], action)) {
        TLOGE(WmsLogTag::WMS_PIP, "[NAPI]Failed to convert parameter to string");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
                                      "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    int32_t status = -1;
    if (argc > 1) {
        if (!ConvertFromJsValue(env, argv[1], status)) {
            TLOGE(WmsLogTag::WMS_PIP, "[NAPI]Failed to convert parameter to int");
            napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
                                          "Input parameter is missing or invalid"));
            return NapiGetUndefined(env);
        }
    }

    auto session = weakSession_.promote();
    if (session == nullptr) {
        TLOGE(WmsLogTag::WMS_PIP, "[NAPI]session is nullptr, id:%{public}d", persistentId_);
        return NapiGetUndefined(env);
    }
    session->SetPipActionEvent(action, status);
    return NapiGetUndefined(env);
}

napi_value JsSceneSession::OnSetPiPControlEvent(napi_env env, napi_callback_info info)
{
    size_t argc = ARG_COUNT_4;
    napi_value argv[ARG_COUNT_4] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc < 1) {
        TLOGE(WmsLogTag::WMS_PIP, "[NAPI]Argc count is invalid: %{public}zu", argc);
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    auto controlType = WsPiPControlType::VIDEO_PLAY_PAUSE;
    if (!ConvertFromJsValue(env, argv[0], controlType)) {
        TLOGE(WmsLogTag::WMS_PIP, "[NAPI]Failed to convert parameter to int");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    auto status = WsPiPControlStatus::PLAY;
    if (argc > 1) {
        if (!ConvertFromJsValue(env, argv[1], status)) {
            TLOGE(WmsLogTag::WMS_PIP, "[NAPI]Failed to convert parameter to int");
            napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
                "Input parameter is missing or invalid"));
            return NapiGetUndefined(env);
        }
    }

    auto session = weakSession_.promote();
    if (session == nullptr) {
        TLOGE(WmsLogTag::WMS_PIP, "[NAPI]session is nullptr, id:%{public}d", persistentId_);
        return NapiGetUndefined(env);
    }
    session->SetPiPControlEvent(controlType, status);
    return NapiGetUndefined(env);
}

napi_value JsSceneSession::OnNotifyDisplayStatusBarTemporarily(napi_env env, napi_callback_info info)
{
    auto session = weakSession_.promote();
    if (session == nullptr) {
        TLOGE(WmsLogTag::WMS_IMMS, "[NAPI]session is nullptr, id:%{public}d", persistentId_);
        return NapiGetUndefined(env);
    }
    size_t argc = ARGC_FOUR;
    napi_value argv[ARGC_FOUR] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    bool isTempDisplay = false;
    if (argc == ARGC_ONE && GetType(env, argv[0]) == napi_boolean) {
        if (!ConvertFromJsValue(env, argv[0], isTempDisplay)) {
            TLOGE(WmsLogTag::WMS_IMMS, "[NAPI]failed to convert parameter to bool");
            return NapiGetUndefined(env);
        }
    }
    session->SetIsDisplayStatusBarTemporarily(isTempDisplay);

    TLOGI(WmsLogTag::WMS_IMMS, "id:%{public}u name:%{public}s isTempDisplay:%{public}u",
        session->GetPersistentId(), session->GetWindowName().c_str(), isTempDisplay);
    return NapiGetUndefined(env);
}

sptr<SceneSession> JsSceneSession::GetNativeSession() const
{
    return weakSession_.promote();
}

napi_value JsSceneSession::OnSetTemporarilyShowWhenLocked(napi_env env, napi_callback_info info)
{
    size_t argc = 4;
    napi_value argv[4] = { nullptr };
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc < 1) {
        TLOGE(WmsLogTag::WMS_SCB, "[NAPI]argc is invalid : %{public}zu", argc);
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    bool isTemporarilyShowWhenLocked = false;
    if (!ConvertFromJsValue(env, argv[0], isTemporarilyShowWhenLocked)) {
        TLOGE(WmsLogTag::WMS_SCB, "[NAPI] Failed to convert parameter to bool");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    auto session = weakSession_.promote();
    if (session == nullptr) {
        TLOGE(WmsLogTag::WMS_SCB, "[NAPI] session_ is null");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    session->SetTemporarilyShowWhenLocked(isTemporarilyShowWhenLocked);
    return NapiGetUndefined(env);
}

napi_value JsSceneSession::OnSetSkipDraw(napi_env env, napi_callback_info info)
{
    size_t argc = 4;
    napi_value argv[4] = { nullptr };
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc < 1) {
        TLOGE(WmsLogTag::WMS_SCB, "[NAPI]argc is invalid : %{public}zu", argc);
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    bool skip = false;
    if (!ConvertFromJsValue(env, argv[0], skip)) {
        TLOGE(WmsLogTag::WMS_SCB, "[NAPI] Failed to convert parameter to bool");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    auto session = weakSession_.promote();
    if (session == nullptr) {
        TLOGE(WmsLogTag::WMS_SCB, "[NAPI] session_ is null");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    session->SetSkipDraw(skip);
    return NapiGetUndefined(env);
}

std::shared_ptr<NativeReference> JsSceneSession::GetJSCallback(const std::string& functionName)
{
    HITRACE_METER_FMT(HITRACE_TAG_WINDOW_MANAGER, "JsSceneSession::GetJSCallback[%s]", functionName.c_str());
    std::shared_ptr<NativeReference> jsCallBack = nullptr;
    std::shared_lock<std::shared_mutex> lock(jsCbMapMutex_);
    auto iter = jsCbMap_.find(functionName);
    if (iter == jsCbMap_.end()) {
        TLOGE(WmsLogTag::DEFAULT, "%{public}s callback not found!", functionName.c_str());
    } else {
        jsCallBack = iter->second;
    }
    return jsCallBack;
}

napi_value JsSceneSession::OnSetSkipSelfWhenShowOnVirtualScreen(napi_env env, napi_callback_info info)
{
    size_t argc = 4;
    napi_value argv[4] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc < 1) { // 1: params num
        WLOGFE("[NAPI]Argc is invalid: %{public}zu", argc);
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    bool isSkip = false;
    if (!ConvertFromJsValue(env, argv[0], isSkip)) {
        WLOGFE("[NAPI]Failed to convert parameter to bool");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    auto session = weakSession_.promote();
    if (session == nullptr) {
        WLOGFE("[NAPI]session is nullptr, id:%{public}d", persistentId_);
        return NapiGetUndefined(env);
    }
    session->SetSkipSelfWhenShowOnVirtualScreen(isSkip);
    WLOGFI("[NAPI] end");
    return NapiGetUndefined(env);
}

napi_value JsSceneSession::OnSetCompatibleModeInPc(napi_env env, napi_callback_info info)
{
    size_t argc = 4;
    napi_value argv[4] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);

    if (argc < ARGC_TWO) {
        TLOGE(WmsLogTag::WMS_SCB, "[NAPI]Argc is invalid: %{public}zu", argc);
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }

    bool enable = false;
    if (!ConvertFromJsValue(env, argv[0], enable)) {
        TLOGE(WmsLogTag::WMS_SCB, "[NAPI]Failed to convert parameter to enable");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }

    bool isSupportDragInPcCompatibleMode = false;
    if (!ConvertFromJsValue(env, argv[ARGC_ONE], isSupportDragInPcCompatibleMode)) {
        TLOGE(WmsLogTag::WMS_SCB, "[NAPI]Failed to convert parameter to isSupportDragInPcCompatibleMode");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }

    auto session = weakSession_.promote();
    if (session == nullptr) {
        TLOGE(WmsLogTag::WMS_SCB, "[NAPI]session is nullptr, id:%{public}d", persistentId_);
        return NapiGetUndefined(env);
    }
    session->SetCompatibleModeInPc(enable, isSupportDragInPcCompatibleMode);
    return NapiGetUndefined(env);
}

napi_value JsSceneSession::OnSetAppSupportPhoneInPc(napi_env env, napi_callback_info info)
{
    size_t argc = ARGC_FOUR;
    napi_value argv[ARGC_FOUR] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc < ARGC_ONE) {
        TLOGE(WmsLogTag::WMS_SCB, "[NAPI]Argc is invalid: %{public}zu", argc);
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    bool isSupportPhone = false;
    if (!ConvertFromJsValue(env, argv[0], isSupportPhone)) {
        TLOGE(WmsLogTag::WMS_SCB, "[NAPI]Failed to convert parameter to isSupportPhone");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    auto session = weakSession_.promote();
    if (session == nullptr) {
        TLOGE(WmsLogTag::WMS_SCB, "[NAPI]session is nullptr, id:%{public}d", persistentId_);
        return NapiGetUndefined(env);
    }
    session->SetAppSupportPhoneInPc(isSupportPhone);
    return NapiGetUndefined(env);
}

napi_value JsSceneSession::OnSetCompatibleWindowSizeInPc(napi_env env, napi_callback_info info)
{
    size_t argc = ARGC_FOUR;
    napi_value argv[ARGC_FOUR] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc < ARGC_FOUR) {
        TLOGE(WmsLogTag::WMS_SCB, "[NAPI]Argc is invalid: %{public}zu", argc);
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    int32_t portraitWidth = 0;
    if (!ConvertFromJsValue(env, argv[ARG_INDEX_0], portraitWidth)) {
        TLOGE(WmsLogTag::WMS_SCB, "[NAPI]Failed to convert parameter to portraitWidth");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    int32_t portraitHeight = 0;
    if (!ConvertFromJsValue(env, argv[ARG_INDEX_1], portraitHeight)) {
        TLOGE(WmsLogTag::WMS_SCB, "[NAPI]Failed to convert parameter to portraitHeight");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    int32_t landscapeWidth = 0;
    if (!ConvertFromJsValue(env, argv[ARG_INDEX_2], landscapeWidth)) {
        TLOGE(WmsLogTag::WMS_SCB, "[NAPI]Failed to convert parameter to landscapeWidth");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    int32_t landscapeHeight = 0;
    if (!ConvertFromJsValue(env, argv[ARG_INDEX_3], landscapeHeight)) {
        TLOGE(WmsLogTag::WMS_SCB, "[NAPI]Failed to convert parameter to landscapeHeight");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    auto session = weakSession_.promote();
    if (session == nullptr) {
        TLOGE(WmsLogTag::WMS_SCB, "[NAPI]session is nullptr, id:%{public}d", persistentId_);
        return NapiGetUndefined(env);
    }
    session->SetCompatibleWindowSizeInPc(portraitWidth, portraitHeight, landscapeWidth, landscapeHeight);
    return NapiGetUndefined(env);
}

napi_value JsSceneSession::OnSetCompatibleModeEnableInPad(napi_env env, napi_callback_info info)
{
    size_t argc = ARGC_FOUR;
    napi_value argv[ARGC_FOUR] = { nullptr };
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc != ARGC_ONE) {
        TLOGE(WmsLogTag::WMS_SCB, "[NAPI]Argc is invalid: %{public}zu", argc);
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }

    bool enable = false;
    if (!ConvertFromJsValue(env, argv[ARG_INDEX_0], enable)) {
        TLOGE(WmsLogTag::WMS_SCB, "[NAPI]Failed to convert parameter to portraitWidth");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }

    auto session = weakSession_.promote();
    if (session == nullptr) {
        TLOGE(WmsLogTag::WMS_SCB, "[NAPI]session is nullptr, id:%{public}d", persistentId_);
        return NapiGetUndefined(env);
    }
    session->SetCompatibleModeEnableInPad(enable);
    return NapiGetUndefined(env);
}

napi_value JsSceneSession::OnSetUniqueDensityDpiFromSCB(napi_env env, napi_callback_info info)
{
    size_t argc = ARG_COUNT_4;
    napi_value argv[ARG_COUNT_4] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    bool paramValidFlag = true;
    bool useUnique = false;
    uint32_t densityDpi = 0;
    std::string errMsg = "";
    if (argc < ARGC_TWO) {
        TLOGE(WmsLogTag::WMS_SCB, "Params not match %{public}zu", argc);
        errMsg = "Invalid args count, need two arg!";
        paramValidFlag = false;
    } else {
        if (!ConvertFromJsValue(env, argv[0], useUnique)) {
            TLOGE(WmsLogTag::WMS_SCB, "Failed to convert parameter to useUnique");
            errMsg = "Failed to convert parameter to useUnique";
            paramValidFlag = false;
        }
        if (paramValidFlag && !ConvertFromJsValue(env, argv[1], densityDpi)) {
            TLOGE(WmsLogTag::WMS_SCB, "Failed to convert parameter to densityDpi");
            errMsg = "Failed to convert parameter to densityDpi";
            paramValidFlag = false;
        }
    }
    if (!paramValidFlag) {
        TLOGE(WmsLogTag::WMS_SCB, "paramValidFlag error");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM), errMsg));
        return NapiGetUndefined(env);
    }

    auto session = weakSession_.promote();
    if (!session) {
        TLOGE(WmsLogTag::WMS_SCB, "[NAPI] session is nullptr, id:%{public}d", persistentId_);
        return NapiGetUndefined(env);
    }
    session->SetUniqueDensityDpi(useUnique, densityDpi);
    return NapiGetUndefined(env);
}

napi_value JsSceneSession::OnSetBlankFlag(napi_env env, napi_callback_info info)
{
    size_t argc = ARGC_FOUR;
    napi_value argv[ARGC_FOUR] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);

    if (argc < ARGC_ONE) {
        TLOGE(WmsLogTag::WMS_SCB, "[NAPI]Argc is invalid: %{public}zu", argc);
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }

    bool isAddBlank = false;
    if (!ConvertFromJsValue(env, argv[0], isAddBlank)) {
        TLOGE(WmsLogTag::WMS_SCB, "[NAPI]Failed to convert parameter to isAddBlank");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }

    auto session = weakSession_.promote();
    if (session == nullptr) {
        TLOGE(WmsLogTag::WMS_SCB, "[NAPI]session is nullptr, id:%{public}d", persistentId_);
        return NapiGetUndefined(env);
    }
    session->SetBlankFlag(isAddBlank);
    return NapiGetUndefined(env);
}

napi_value JsSceneSession::OnSetBufferAvailableCallbackEnable(napi_env env, napi_callback_info info)
{
    size_t argc = ARGC_FOUR;
    napi_value argv[ARGC_FOUR] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);

    if (argc < ARGC_ONE) {
        TLOGE(WmsLogTag::WMS_SCB, "[NAPI]Argc is invalid: %{public}zu", argc);
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }

    bool enable = false;
    if (!ConvertFromJsValue(env, argv[0], enable)) {
        TLOGE(WmsLogTag::WMS_SCB, "[NAPI]Failed to convert parameter to enable");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }

    auto session = weakSession_.promote();
    if (session == nullptr) {
        TLOGE(WmsLogTag::WMS_SCB, "[NAPI]session is nullptr, id:%{public}d", persistentId_);
        return NapiGetUndefined(env);
    }
    session->SetBufferAvailableCallbackEnable(enable);
    return NapiGetUndefined(env);
}

napi_value JsSceneSession::OnSyncDefaultRequestedOrientation(napi_env env, napi_callback_info info)
{
    size_t argc = ARGC_FOUR;
    napi_value argv[ARGC_FOUR] = { nullptr };
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc != ARGC_ONE) {
        TLOGE(WmsLogTag::WMS_SCB, "[NAPI]Argc is invalid: %{public}zu", argc);
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    uint32_t defaultRequestedOrientation = 0;
    if (!ConvertFromJsValue(env, argv[0], defaultRequestedOrientation)) {
        TLOGE(WmsLogTag::WMS_SCB, "[NAPI]Failed to convert parameter to defaultRequestedOrientation");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    auto session = weakSession_.promote();
    if (session == nullptr) {
        TLOGE(WmsLogTag::WMS_SCB, "[NAPI]session is nullptr, id:%{public}d", persistentId_);
        return NapiGetUndefined(env);
    }
    session->SetDefaultRequestedOrientation(static_cast<Orientation>(defaultRequestedOrientation));
    return NapiGetUndefined(env);
}

napi_value JsSceneSession::OnSetIsPcAppInPad(napi_env env, napi_callback_info info)
{
    size_t argc = ARGC_FOUR;
    napi_value argv[ARGC_FOUR] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc < ARGC_ONE) {
        TLOGE(WmsLogTag::WMS_SCB, "[NAPI]Argc is invalid: %{public}zu", argc);
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
                                      "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    bool enable = false;
    if (!ConvertFromJsValue(env, argv[0], enable)) {
        TLOGE(WmsLogTag::WMS_SCB, "[NAPI]Failed to convert parameter to enable");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
                                      "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    auto session = weakSession_.promote();
    if (session == nullptr) {
        TLOGE(WmsLogTag::WMS_SCB, "[NAPI]session is nullptr, id:%{public}d", persistentId_);
        return NapiGetUndefined(env);
    }
    session->SetIsPcAppInPad(enable);
    return NapiGetUndefined(env);
}

napi_value JsSceneSession::OnSetStartingWindowExitAnimationFlag(napi_env env, napi_callback_info info)
{
    size_t argc = ARGC_FOUR;
    napi_value argv[ARGC_FOUR] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);

    if (argc != ARGC_ONE) {
        TLOGE(WmsLogTag::WMS_SCB, "[NAPI]Argc is invalid: %{public}zu", argc);
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }

    bool enable = true;
    if (!ConvertFromJsValue(env, argv[0], enable)) {
        TLOGE(WmsLogTag::WMS_SCB, "[NAPI]Failed to convert parameter to enable");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }

    auto session = weakSession_.promote();
    if (session == nullptr) {
        TLOGE(WmsLogTag::WMS_SCB, "[NAPI]session is null, id:%{public}d", persistentId_);
        return NapiGetUndefined(env);
    }
    session->SetStartingWindowExitAnimationFlag(enable);
    return NapiGetUndefined(env);
}

napi_value JsSceneSession::OnCompatibleFullScreenRecover(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::WMS_SCB, "[NAPI]");
    auto session = weakSession_.promote();
    if (session == nullptr) {
        TLOGE(WmsLogTag::WMS_SCB, "[NAPI]session is nullptr, id:%{public}d", persistentId_);
        return NapiGetUndefined(env);
    }
    session->CompatibleFullScreenRecover();
    return NapiGetUndefined(env);
}

napi_value JsSceneSession::OnCompatibleFullScreenMinimize(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::WMS_SCB, "[NAPI]");
    auto session = weakSession_.promote();
    if (session == nullptr) {
        TLOGE(WmsLogTag::WMS_SCB, "[NAPI]session is nullptr, id:%{public}d", persistentId_);
        return NapiGetUndefined(env);
    }
    session->CompatibleFullScreenMinimize();
    return NapiGetUndefined(env);
}

napi_value JsSceneSession::OnCompatibleFullScreenClose(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::WMS_SCB, "[NAPI]");
    auto session = weakSession_.promote();
    if (session == nullptr) {
        TLOGE(WmsLogTag::WMS_SCB, "[NAPI]session is nullptr, id:%{public}d", persistentId_);
        return NapiGetUndefined(env);
    }
    session->CompatibleFullScreenClose();
    return NapiGetUndefined(env);
}

napi_value JsSceneSession::OnSyncScenePanelGlobalPosition(napi_env env, napi_callback_info info)
{
    size_t argc = ARGC_FOUR;
    napi_value argv[ARGC_FOUR] = { nullptr };
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);

    if (argc != ARGC_ONE) {
        TLOGE(WmsLogTag::WMS_SCB, "[NAPI]Argc is invalid: %{public}zu", argc);
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }

    std::string reason;
    if (!ConvertFromJsValue(env, argv[0], reason)) {
        TLOGE(WmsLogTag::WMS_PIPELINE, "[NAPI]Failed to convert parameter to sync reason");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    auto it = std::find_if(g_syncGlobalPositionPermission.begin(), g_syncGlobalPositionPermission.end(),
        [reason](const std::string& permission) { return permission.find(reason) != std::string::npos; });
    if (it == g_syncGlobalPositionPermission.end()) {
        TLOGE(WmsLogTag::WMS_PIPELINE, "[NAPI]called reason:%{public}s is not permitted", reason.c_str());
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_NO_PERMISSION),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    TLOGI(WmsLogTag::WMS_PIPELINE, "[NAPI]called reason:%{public}s", reason.c_str());
    auto session = weakSession_.promote();
    if (session == nullptr) {
        TLOGE(WmsLogTag::WMS_SCB, "[NAPI]session is nullptr, id:%{public}d", persistentId_);
        return NapiGetUndefined(env);
    }
    session->SyncScenePanelGlobalPosition(true);
    return NapiGetUndefined(env);
}

napi_value JsSceneSession::OnUnSyncScenePanelGlobalPosition(napi_env env, napi_callback_info info)
{
    size_t argc = ARGC_FOUR;
    napi_value argv[ARGC_FOUR] = { nullptr };
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);

    if (argc != ARGC_ONE) {
        TLOGE(WmsLogTag::WMS_SCB, "[NAPI]Argc is invalid: %{public}zu", argc);
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }

    std::string reason;
    if (!ConvertFromJsValue(env, argv[0], reason)) {
        TLOGE(WmsLogTag::WMS_PIPELINE, "[NAPI]Failed to convert parameter to un sync reason");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    auto it = std::find_if(g_syncGlobalPositionPermission.begin(), g_syncGlobalPositionPermission.end(),
        [reason](const std::string& permission) { return permission.find(reason) != std::string::npos; });
    if (it == g_syncGlobalPositionPermission.end()) {
        TLOGE(WmsLogTag::WMS_PIPELINE, "[NAPI]called reason:%{public}s is not permitted", reason.c_str());
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_NO_PERMISSION),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    TLOGI(WmsLogTag::WMS_PIPELINE, "[NAPI]called reason:%{public}s", reason.c_str());
    auto session = weakSession_.promote();
    if (session == nullptr) {
        TLOGE(WmsLogTag::WMS_SCB, "[NAPI]session is nullptr, id:%{public}d", persistentId_);
        return NapiGetUndefined(env);
    }
    session->SyncScenePanelGlobalPosition(false);
    return NapiGetUndefined(env);
}

} // namespace OHOS::Rosen
