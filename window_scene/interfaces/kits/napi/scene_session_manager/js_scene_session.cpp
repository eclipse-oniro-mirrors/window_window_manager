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
#include "js_window_scene_config.h"
#include "napi_common_want.h"
#include "pixel_map_napi.h"
#include "session/host/include/ability_info_manager.h"
#include "session/host/include/session.h"
#include "session_manager/include/scene_session_manager.h"
#include "window_manager_hilog.h"
#include "common/include/session_permission.h"
#include "napi_common_want.h"

namespace OHOS::Rosen {
using namespace AbilityRuntime;
namespace {
constexpr HiviewDFX::HiLogLabel LABEL = { LOG_CORE, HILOG_DOMAIN_WINDOW, "JsSceneSession" };
const std::string PENDING_SCENE_CB = "pendingSceneSessionActivation";
const std::string CHANGE_SESSION_VISIBILITY_WITH_STATUS_BAR = "changeSessionVisibilityWithStatusBar";
const std::string SESSION_STATE_CHANGE_CB = "sessionStateChange";
const std::string UPDATE_TRANSITION_ANIMATION_CB = "updateTransitionAnimation";
const std::string BUFFER_AVAILABLE_CHANGE_CB = "bufferAvailableChange";
const std::string SESSION_EVENT_CB = "sessionEvent";
const std::string SESSION_RECT_CHANGE_CB = "sessionRectChange";
const std::string SESSION_WINDOW_LIMITS_CHANGE_CB = "sessionWindowLimitsChange";
const std::string FLOATING_BALL_UPDATE_CB = "updateFbTemplateInfo";
const std::string FLOATING_BALL_STOP_CB = "prepareRemoveFb";
const std::string FLOATING_BALL_RESTORE_MAIN_WINDOW_CB = "restoreFbMainWindow";
const std::string WINDOW_MOVING_CB = "windowMoving";
const std::string SESSION_PIP_CONTROL_STATUS_CHANGE_CB = "sessionPiPControlStatusChange";
const std::string SESSION_AUTO_START_PIP_CB = "autoStartPiP";
const std::string UPDATE_PIP_TEMPLATE_INFO_CB = "updatePiPTemplateInfo";
const std::string SET_PIP_PARENT_WINDOWID_CB = "setPiPParentWindowId";
const std::string CREATE_SUB_SESSION_CB = "createSpecificSession";
const std::string CLEAR_SUB_SESSION_CB = "clearSubSession";
const std::string WINDOW_ANCHOR_INFO_CHANGE_CB = "windowAnchorInfoChange";
const std::string FOLLOW_PARENT_RECT_CB = "followParentRect";
const std::string BIND_DIALOG_TARGET_CB = "bindDialogTarget";
const std::string RAISE_TO_TOP_CB = "raiseToTop";
const std::string RAISE_TO_TOP_POINT_DOWN_CB = "raiseToTopForPointDown";
const std::string CLICK_MODAL_WINDOW_OUTSIDE_CB = "clickModalWindowOutside";
const std::string BACK_PRESSED_CB = "backPressed";
const std::string SESSION_FOCUSABLE_CHANGE_CB = "sessionFocusableChange";
const std::string SESSION_TOUCHABLE_CHANGE_CB = "sessionTouchableChange";
const std::string SESSION_TOP_MOST_CHANGE_CB = "sessionTopmostChange";
const std::string SUB_MODAL_TYPE_CHANGE_CB = "subModalTypeChange";
const std::string MAIN_MODAL_TYPE_CHANGE_CB = "mainModalTypeChange";
const std::string THROW_SLIP_ANIMATION_STATE_CHANGE_CB = "throwSlipAnimationStateChange";
const std::string FULLSCREEN_WATERFALL_MODE_CHANGE_CB = "fullScreenWaterfallModeChange";
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
const std::string PENDING_SESSION_TO_BACKGROUND_CB = "pendingSessionToBackground";
const std::string PENDING_SESSION_TO_BACKGROUND_FOR_DELEGATOR_CB = "pendingSessionToBackgroundForDelegator";
const std::string CUSTOM_ANIMATION_PLAYING_CB = "isCustomAnimationPlaying";
const std::string NEED_DEFAULT_ANIMATION_FLAG_CHANGE_CB = "needDefaultAnimationFlagChange";
const std::string SHOW_WHEN_LOCKED_CB = "sessionShowWhenLockedChange";
const std::string REQUESTED_ORIENTATION_CHANGE_CB = "sessionRequestedOrientationChange";
const std::string RAISE_ABOVE_TARGET_CB = "raiseAboveTarget";
const std::string RAISE_MAIN_WINDOW_ABOVE_TARGET_CB = "raiseMainWindowAboveTarget";
const std::string FORCE_HIDE_CHANGE_CB = "sessionForceHideChange";
const std::string WINDOW_DRAG_HOT_AREA_CB = "windowDragHotArea";
const std::string TOUCH_OUTSIDE_CB = "touchOutside";
const std::string SESSIONINFO_LOCKEDSTATE_CHANGE_CB = "sessionInfoLockedStateChange";
const std::string PREPARE_CLOSE_PIP_SESSION = "prepareClosePiPSession";
const std::string LANDSCAPE_MULTI_WINDOW_CB = "landscapeMultiWindow";
const std::string CONTEXT_TRANSPARENT_CB = "contextTransparent";
const std::string ADJUST_KEYBOARD_LAYOUT_CB = "adjustKeyboardLayout";
const std::string LAYOUT_FULL_SCREEN_CB = "layoutFullScreenChange";
const std::string DEFAULT_DENSITY_ENABLED_CB = "defaultDensityEnabled";
const std::string WINDOW_SHADOW_ENABLE_CHANGE_CB = "windowShadowEnableChange";
const std::string TITLE_DOCK_HOVER_SHOW_CB = "titleAndDockHoverShowChange";
const std::string RESTORE_MAIN_WINDOW_CB = "restoreMainWindow";
const std::string RESTORE_FLOAT_MAIN_WINDOW_CB = "restoreFloatMainWindow";
const std::string NEXT_FRAME_LAYOUT_FINISH_CB = "nextFrameLayoutFinish";
const std::string MAIN_WINDOW_TOP_MOST_CHANGE_CB = "mainWindowTopmostChange";
const std::string SET_WINDOW_RECT_AUTO_SAVE_CB = "setWindowRectAutoSave";
const std::string UPDATE_APP_USE_CONTROL_CB = "updateAppUseControl";
const std::string PRIVACY_MODE_CHANGE_CB = "privacyModeChange";
const std::string SESSION_DISPLAY_ID_CHANGE_CB = "sessionDisplayIdChange";
const std::string SET_SUPPORT_WINDOW_MODES_CB = "setSupportWindowModes";
const std::string SESSION_LOCK_STATE_CHANGE_CB = "sessionLockStateChange";
const std::string UPDATE_SESSION_LABEL_AND_ICON_CB = "updateSessionLabelAndIcon";
const std::string SESSION_GET_TARGET_ORIENTATION_CONFIG_INFO_CB = "sessionGetTargetOrientationConfigInfo";
const std::string KEYBOARD_STATE_CHANGE_CB = "keyboardStateChange";
const std::string KEYBOARD_EFFECT_OPTION_CHANGE_CB = "keyboardEffectOptionChange";
const std::string SET_WINDOW_CORNER_RADIUS_CB = "setWindowCornerRadius";
const std::string HIGHLIGHT_CHANGE_CB = "highlightChange";
const std::string SET_PARENT_SESSION_CB = "setParentSession";
const std::string UPDATE_FLAG_CB = "updateFlag";
const std::string Z_LEVEL_CHANGE_CB = "zLevelChange";
const std::string UPDATE_FOLLOW_SCREEN_CHANGE_CB = "sessionUpdateFollowScreenChange";
const std::string USE_IMPLICITANIMATION_CB = "useImplicitAnimationChange";
const std::string SET_WINDOW_SHADOWS_CB = "setWindowShadows";
const std::string SET_SUB_WINDOW_SOURCE_CB = "setSubWindowSource";
const std::string ANIMATE_TO_CB = "animateToTargetProperty";
const std::string BATCH_PENDING_SCENE_ACTIVE_CB = "batchPendingSceneSessionsActivation";
const std::string SCENE_OUTLINE_PARAMS_CHANGE_CB = "sceneOutlineParamsChange";
const std::string RESTART_APP_CB = "restartApp";
const std::string CALLING_SESSION_ID_CHANGE_CB = "callingWindowIdChange";
const std::string ROTATION_LOCK_CHANGE_CB = "rotationLockChange";
const std::string SNAPSHOT_SKIP_CHANGE_CB = "snapshotSkipChange";
const std::string COMPATIBLE_MODE_CHANGE_CB = "compatibleModeChange";

constexpr int ARG_COUNT_1 = 1;
constexpr int ARG_COUNT_2 = 2;
constexpr int ARG_COUNT_3 = 3;
constexpr int ARG_COUNT_4 = 4;
constexpr int ARG_COUNT_6 = 6;
constexpr int ARG_COUNT_7 = 7;
constexpr int ARG_INDEX_0 = 0;
constexpr int ARG_INDEX_1 = 1;
constexpr int ARG_INDEX_2 = 2;
constexpr int ARG_INDEX_3 = 3;
constexpr int ARG_INDEX_4 = 4;
constexpr int ARG_INDEX_5 = 5;
constexpr int ARG_INDEX_6 = 6;
constexpr uint32_t DISALLOW_ACTIVATION_ISOLATE_VERSION = 20;

const std::map<std::string, ListenerFuncType> ListenerFuncMap {
    {PENDING_SCENE_CB,                      ListenerFuncType::PENDING_SCENE_CB},
    {CHANGE_SESSION_VISIBILITY_WITH_STATUS_BAR,
        ListenerFuncType::CHANGE_SESSION_VISIBILITY_WITH_STATUS_BAR},
    {SESSION_STATE_CHANGE_CB,               ListenerFuncType::SESSION_STATE_CHANGE_CB},
    {UPDATE_TRANSITION_ANIMATION_CB,        ListenerFuncType::UPDATE_TRANSITION_ANIMATION_CB},
    {BUFFER_AVAILABLE_CHANGE_CB,            ListenerFuncType::BUFFER_AVAILABLE_CHANGE_CB},
    {SESSION_EVENT_CB,                      ListenerFuncType::SESSION_EVENT_CB},
    {SESSION_RECT_CHANGE_CB,                ListenerFuncType::SESSION_RECT_CHANGE_CB},
    {SESSION_WINDOW_LIMITS_CHANGE_CB,       ListenerFuncType::SESSION_WINDOW_LIMITS_CHANGE_CB},
    {SESSION_PIP_CONTROL_STATUS_CHANGE_CB,  ListenerFuncType::SESSION_PIP_CONTROL_STATUS_CHANGE_CB},
    {SESSION_AUTO_START_PIP_CB,             ListenerFuncType::SESSION_AUTO_START_PIP_CB},
    {CREATE_SUB_SESSION_CB,                 ListenerFuncType::CREATE_SUB_SESSION_CB},
    {CLEAR_SUB_SESSION_CB,                  ListenerFuncType::CLEAR_SUB_SESSION_CB},
    {BIND_DIALOG_TARGET_CB,                 ListenerFuncType::BIND_DIALOG_TARGET_CB},
    {RAISE_TO_TOP_CB,                       ListenerFuncType::RAISE_TO_TOP_CB},
    {RAISE_TO_TOP_POINT_DOWN_CB,            ListenerFuncType::RAISE_TO_TOP_POINT_DOWN_CB},
    {CLICK_MODAL_WINDOW_OUTSIDE_CB,         ListenerFuncType::CLICK_MODAL_WINDOW_OUTSIDE_CB},
    {BACK_PRESSED_CB,                       ListenerFuncType::BACK_PRESSED_CB},
    {SESSION_FOCUSABLE_CHANGE_CB,           ListenerFuncType::SESSION_FOCUSABLE_CHANGE_CB},
    {SESSION_TOUCHABLE_CHANGE_CB,           ListenerFuncType::SESSION_TOUCHABLE_CHANGE_CB},
    {SESSION_TOP_MOST_CHANGE_CB,            ListenerFuncType::SESSION_TOP_MOST_CHANGE_CB},
    {SUB_MODAL_TYPE_CHANGE_CB,              ListenerFuncType::SUB_MODAL_TYPE_CHANGE_CB},
    {MAIN_MODAL_TYPE_CHANGE_CB,             ListenerFuncType::MAIN_MODAL_TYPE_CHANGE_CB},
    {THROW_SLIP_ANIMATION_STATE_CHANGE_CB,  ListenerFuncType::THROW_SLIP_ANIMATION_STATE_CHANGE_CB},
    {FULLSCREEN_WATERFALL_MODE_CHANGE_CB,   ListenerFuncType::FULLSCREEN_WATERFALL_MODE_CHANGE_CB},
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
    {PENDING_SESSION_TO_BACKGROUND_CB,      ListenerFuncType::PENDING_SESSION_TO_BACKGROUND_CB},
    {PENDING_SESSION_TO_BACKGROUND_FOR_DELEGATOR_CB,
        ListenerFuncType::PENDING_SESSION_TO_BACKGROUND_FOR_DELEGATOR_CB},
    {CUSTOM_ANIMATION_PLAYING_CB,           ListenerFuncType::CUSTOM_ANIMATION_PLAYING_CB},
    {NEED_DEFAULT_ANIMATION_FLAG_CHANGE_CB, ListenerFuncType::NEED_DEFAULT_ANIMATION_FLAG_CHANGE_CB},
    {SHOW_WHEN_LOCKED_CB,                   ListenerFuncType::SHOW_WHEN_LOCKED_CB},
    {REQUESTED_ORIENTATION_CHANGE_CB,       ListenerFuncType::REQUESTED_ORIENTATION_CHANGE_CB},
    {RAISE_ABOVE_TARGET_CB,                 ListenerFuncType::RAISE_ABOVE_TARGET_CB},
    {RAISE_MAIN_WINDOW_ABOVE_TARGET_CB,     ListenerFuncType::RAISE_MAIN_WINDOW_ABOVE_TARGET_CB},
    {FORCE_HIDE_CHANGE_CB,                  ListenerFuncType::FORCE_HIDE_CHANGE_CB},
    {WINDOW_DRAG_HOT_AREA_CB,               ListenerFuncType::WINDOW_DRAG_HOT_AREA_CB},
    {TOUCH_OUTSIDE_CB,                      ListenerFuncType::TOUCH_OUTSIDE_CB},
    {SESSIONINFO_LOCKEDSTATE_CHANGE_CB,     ListenerFuncType::SESSIONINFO_LOCKEDSTATE_CHANGE_CB},
    {PREPARE_CLOSE_PIP_SESSION,             ListenerFuncType::PREPARE_CLOSE_PIP_SESSION},
    {LANDSCAPE_MULTI_WINDOW_CB,             ListenerFuncType::LANDSCAPE_MULTI_WINDOW_CB},
    {CONTEXT_TRANSPARENT_CB,                ListenerFuncType::CONTEXT_TRANSPARENT_CB},
    {ADJUST_KEYBOARD_LAYOUT_CB,             ListenerFuncType::ADJUST_KEYBOARD_LAYOUT_CB},
    {LAYOUT_FULL_SCREEN_CB,                 ListenerFuncType::LAYOUT_FULL_SCREEN_CB},
    {DEFAULT_DENSITY_ENABLED_CB,            ListenerFuncType::DEFAULT_DENSITY_ENABLED_CB},
    {WINDOW_SHADOW_ENABLE_CHANGE_CB,        ListenerFuncType::WINDOW_SHADOW_ENABLE_CHANGE_CB},
    {TITLE_DOCK_HOVER_SHOW_CB,              ListenerFuncType::TITLE_DOCK_HOVER_SHOW_CB},
    {NEXT_FRAME_LAYOUT_FINISH_CB,           ListenerFuncType::NEXT_FRAME_LAYOUT_FINISH_CB},
    {PRIVACY_MODE_CHANGE_CB,                ListenerFuncType::PRIVACY_MODE_CHANGE_CB},
    {RESTORE_MAIN_WINDOW_CB,                ListenerFuncType::RESTORE_MAIN_WINDOW_CB},
    {RESTORE_FLOAT_MAIN_WINDOW_CB,          ListenerFuncType::RESTORE_FLOAT_MAIN_WINDOW_CB},
    {MAIN_WINDOW_TOP_MOST_CHANGE_CB,        ListenerFuncType::MAIN_WINDOW_TOP_MOST_CHANGE_CB},
    {SET_WINDOW_RECT_AUTO_SAVE_CB,          ListenerFuncType::SET_WINDOW_RECT_AUTO_SAVE_CB},
    {UPDATE_APP_USE_CONTROL_CB,             ListenerFuncType::UPDATE_APP_USE_CONTROL_CB},
    {PRIVACY_MODE_CHANGE_CB,                ListenerFuncType::PRIVACY_MODE_CHANGE_CB},
    {SESSION_DISPLAY_ID_CHANGE_CB,          ListenerFuncType::SESSION_DISPLAY_ID_CHANGE_CB},
    {SET_SUPPORT_WINDOW_MODES_CB,           ListenerFuncType::SET_SUPPORT_WINDOW_MODES_CB},
    {SESSION_GET_TARGET_ORIENTATION_CONFIG_INFO_CB,
        ListenerFuncType::SESSION_GET_TARGET_ORIENTATION_CONFIG_INFO_CB},
    {WINDOW_MOVING_CB,                      ListenerFuncType::WINDOW_MOVING_CB},
    {SESSION_LOCK_STATE_CHANGE_CB,          ListenerFuncType::SESSION_LOCK_STATE_CHANGE_CB},
    {UPDATE_SESSION_LABEL_AND_ICON_CB,      ListenerFuncType::UPDATE_SESSION_LABEL_AND_ICON_CB},
    {KEYBOARD_STATE_CHANGE_CB,              ListenerFuncType::KEYBOARD_STATE_CHANGE_CB},
    {KEYBOARD_EFFECT_OPTION_CHANGE_CB,          ListenerFuncType::KEYBOARD_EFFECT_OPTION_CHANGE_CB},
    {SET_WINDOW_CORNER_RADIUS_CB,           ListenerFuncType::SET_WINDOW_CORNER_RADIUS_CB},
    {HIGHLIGHT_CHANGE_CB,                   ListenerFuncType::HIGHLIGHT_CHANGE_CB},
    {FOLLOW_PARENT_RECT_CB,                 ListenerFuncType::FOLLOW_PARENT_RECT_CB},
    {SET_PARENT_SESSION_CB,                 ListenerFuncType::SET_PARENT_SESSION_CB},
    {UPDATE_FLAG_CB,                        ListenerFuncType::UPDATE_FLAG_CB},
    {Z_LEVEL_CHANGE_CB,                     ListenerFuncType::Z_LEVEL_CHANGE_CB},
    {UPDATE_PIP_TEMPLATE_INFO_CB,           ListenerFuncType::UPDATE_PIP_TEMPLATE_INFO_CB},
    {SET_PIP_PARENT_WINDOWID_CB,            ListenerFuncType::SET_PIP_PARENT_WINDOWID_CB},
    {UPDATE_FOLLOW_SCREEN_CHANGE_CB,        ListenerFuncType::UPDATE_FOLLOW_SCREEN_CHANGE_CB},
    {USE_IMPLICITANIMATION_CB,              ListenerFuncType::USE_IMPLICIT_ANIMATION_CB},
    {WINDOW_ANCHOR_INFO_CHANGE_CB,          ListenerFuncType::WINDOW_ANCHOR_INFO_CHANGE_CB},
    {SET_WINDOW_SHADOWS_CB,                 ListenerFuncType::SET_WINDOW_SHADOWS_CB},
    {SET_SUB_WINDOW_SOURCE_CB,              ListenerFuncType::SET_SUB_WINDOW_SOURCE_CB},
    {ANIMATE_TO_CB,                         ListenerFuncType::ANIMATE_TO_CB},
    {BATCH_PENDING_SCENE_ACTIVE_CB,         ListenerFuncType::BATCH_PENDING_SCENE_ACTIVE_CB},
    {FLOATING_BALL_UPDATE_CB,               ListenerFuncType::FLOATING_BALL_UPDATE_CB},
    {FLOATING_BALL_STOP_CB,                 ListenerFuncType::FLOATING_BALL_STOP_CB},
    {FLOATING_BALL_RESTORE_MAIN_WINDOW_CB,      ListenerFuncType::FLOATING_BALL_RESTORE_MAIN_WINDOW_CB},
    {SCENE_OUTLINE_PARAMS_CHANGE_CB,        ListenerFuncType::SCENE_OUTLINE_PARAMS_CHANGE_CB},
    {RESTART_APP_CB,                        ListenerFuncType::RESTART_APP_CB},
    {CALLING_SESSION_ID_CHANGE_CB,          ListenerFuncType::CALLING_SESSION_ID_CHANGE_CB},
    {ROTATION_LOCK_CHANGE_CB,               ListenerFuncType::ROTATION_LOCK_CHANGE_CB},
    {SNAPSHOT_SKIP_CHANGE_CB,               ListenerFuncType::SNAPSHOT_SKIP_CHANGE_CB},
    {COMPATIBLE_MODE_CHANGE_CB,             ListenerFuncType::COMPATIBLE_MODE_CHANGE_CB},
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

static napi_value CreatePipTemplateInfo(napi_env env, const PiPTemplateInfo& pipTemplateInfo)
{
    TLOGI(WmsLogTag::WMS_PIP, "in");
    napi_value pipTemplateInfoValue = nullptr;
    napi_create_object(env, &pipTemplateInfoValue);
    napi_set_named_property(env, pipTemplateInfoValue, "pipTemplateType",
        CreateJsValue(env, pipTemplateInfo.pipTemplateType));
    napi_set_named_property(env, pipTemplateInfoValue, "priority",
        CreateJsValue(env, pipTemplateInfo.priority));
    napi_set_named_property(env, pipTemplateInfoValue, "defaultWindowSizeType",
        CreateJsValue(env, pipTemplateInfo.defaultWindowSizeType));
    napi_set_named_property(env, pipTemplateInfoValue, "cornerAdsorptionEnabled",
        CreateJsValue(env, pipTemplateInfo.cornerAdsorptionEnabled));
    napi_value controlArrayValue = nullptr;
    std::vector<std::uint32_t> controlGroups = pipTemplateInfo.controlGroup;
    napi_create_array_with_length(env, controlGroups.size(), &controlArrayValue);
    auto index = 0;
    for (const auto& controlGroup : controlGroups) {
        napi_set_element(env, controlArrayValue, index++, CreateJsValue(env, controlGroup));
    }
    napi_set_named_property(env, pipTemplateInfoValue, "controlGroup", controlArrayValue);
    napi_value controlStatusArrayValue = nullptr;
    std::vector<PiPControlStatusInfo> controlStatusInfoList = pipTemplateInfo.pipControlStatusInfoList;
    napi_create_array_with_length(env, controlStatusInfoList.size(), &controlStatusArrayValue);
    auto controlStatusIndex = 0;
    for (const auto& controlStatus : controlStatusInfoList) {
        napi_set_element(env, controlStatusArrayValue, controlStatusIndex++,
            CreateJsPiPControlStatusObject(env, controlStatus));
    }
    napi_set_named_property(env, pipTemplateInfoValue, "pipControlStatusInfoList", controlStatusArrayValue);
    napi_value controlEnableArrayValue = nullptr;
    std::vector<PiPControlEnableInfo> controlEnableInfoList = pipTemplateInfo.pipControlEnableInfoList;
    napi_create_array_with_length(env, controlEnableInfoList.size(), &controlEnableArrayValue);
    auto controlEnableIndex = 0;
    for (const auto& controlEnableInfo : controlEnableInfoList) {
        napi_set_element(env, controlEnableArrayValue, controlEnableIndex++,
            CreateJsPiPControlEnableObject(env, controlEnableInfo));
    }
    napi_set_named_property(env, pipTemplateInfoValue, "pipControlEnableInfoList", controlEnableArrayValue);
    return pipTemplateInfoValue;
}

static napi_value CreateFbTemplateInfo(napi_env env, const FloatingBallTemplateInfo& fbTemplateInfo)
{
    TLOGI(WmsLogTag::WMS_SYSTEM, "template %{public}d", fbTemplateInfo.template_);
    napi_value fbTemplateInfoValue = nullptr;
    napi_create_object(env, &fbTemplateInfoValue);
    napi_set_named_property(env, fbTemplateInfoValue, "template",
        CreateJsValue(env, fbTemplateInfo.template_));
    napi_set_named_property(env, fbTemplateInfoValue, "title",
        CreateJsValue(env, fbTemplateInfo.title_));
    napi_set_named_property(env, fbTemplateInfoValue, "content",
        CreateJsValue(env, fbTemplateInfo.content_));
    napi_set_named_property(env, fbTemplateInfoValue, "backgroundColor",
        CreateJsValue(env, fbTemplateInfo.backgroundColor_));

    if (fbTemplateInfo.icon_ != nullptr) {
        napi_value jsIcon = Media::PixelMapNapi::CreatePixelMap(env, fbTemplateInfo.icon_);
        if (jsIcon == nullptr) {
            TLOGNE(WmsLogTag::WMS_SYSTEM, "icon is nullptr");
            return nullptr;
        }
        napi_set_named_property(env, fbTemplateInfoValue, "icon", jsIcon);
    }
    return fbTemplateInfoValue;
}

static void ParseMetadataConfiguration(napi_env env, napi_value objValue, const sptr<SceneSession>& session)
{
    auto abilityInfo = session->GetSessionInfo().abilityInfo;
    if (!abilityInfo) {
        TLOGW(WmsLogTag::WMS_LAYOUT_PC, "abilityInfo is nullptr");
        return;
    }
    uint32_t value = 0;
    for (const auto& item : abilityInfo->metadata) {
        if (item.name == "ohos.ability.window.width") {
            if (GetIntValueFromString(item.value, value) == WSError::WS_OK) {
                TLOGI(WmsLogTag::WMS_LAYOUT_PC, "ohos.ability.window.width=%{public}d", value);
                napi_set_named_property(env, objValue, "windowWidth", CreateJsValue(env, value));
            }
        } else if (item.name == "ohos.ability.window.height") {
            if (GetIntValueFromString(item.value, value) == WSError::WS_OK) {
                TLOGI(WmsLogTag::WMS_LAYOUT_PC, "ohos.ability.window.height=%{public}d", value);
                napi_set_named_property(env, objValue, "windowHeight", CreateJsValue(env, value));
            }
        } else if (item.name == "ohos.ability.window.left") {
            if (item.value.size() > 0) {
                TLOGI(WmsLogTag::WMS_LAYOUT_PC, "ohos.ability.window.left=%{public}s", item.value.c_str());
                napi_set_named_property(env, objValue, "windowLeft", CreateJsValue(env, item.value));
            }
        } else if (item.name == "ohos.ability.window.top") {
            if (item.value.size() > 0) {
                TLOGI(WmsLogTag::WMS_LAYOUT_PC, "ohos.ability.window.top=%{public}s", item.value.c_str());
                napi_set_named_property(env, objValue, "windowTop", CreateJsValue(env, item.value));
            }
        } else if (item.name == "ohos.ability.window.isMaximize") {
            TLOGI(WmsLogTag::WMS_LAYOUT_PC, "ohos.ability.window.isMaximize=%{public}s", item.value.c_str());
            bool isMaximize = (item.value == "true");
            napi_set_named_property(env, objValue, "isMaximize", CreateJsValue(env, isMaximize));
        } else if (item.name == "ohos.ability.window.isRightAngle") {
            TLOGI(WmsLogTag::WMS_LAYOUT_PC, "ohos.ability.window.isRightAngle=%{public}s", item.value.c_str());
            bool isRightAngle = (item.value == "true");
            napi_set_named_property(env, objValue, "isRightAngle", CreateJsValue(env, isRightAngle));
        }
    }
}

napi_value JsSceneSession::GetJsPanelSessionObj(napi_env env, const sptr<SceneSession>& session)
{
    if (session->GetWindowType() != WindowType::WINDOW_TYPE_KEYBOARD_PANEL) {
        TLOGD(WmsLogTag::WMS_KEYBOARD, "This is not panel session");
        return nullptr;
    }
    int32_t persistentId = session->GetPersistentId();
    auto iter = jsSceneSessionMap_.find(persistentId);
    if (iter != jsSceneSessionMap_.end()) {
        const auto& ref = iter->second;
        if (ref != nullptr) {
            TLOGI(WmsLogTag::WMS_KEYBOARD, "Js panel session exists already and reuse it, id: %{public}d",
                  persistentId);
            napi_value panelSessionObj = nullptr;
            napi_get_reference_value(env, ref, &panelSessionObj);
            return panelSessionObj;
        }
    }
    TLOGI(WmsLogTag::WMS_KEYBOARD, "Not find js panel session");
    return nullptr;
}

napi_value JsSceneSession::Create(napi_env env, const sptr<SceneSession>& session)
{
    if (session == nullptr) {
        TLOGE(WmsLogTag::DEFAULT, "session is null");
        return NapiGetUndefined(env);
    }
    napi_value objValue = JsSceneSession::GetJsPanelSessionObj(env, session);
    if (objValue != nullptr) {
        return objValue;
    }
    napi_create_object(env, &objValue);
    if (objValue == nullptr) {
        TLOGE(WmsLogTag::DEFAULT, "objValue is null");
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
    napi_set_named_property(env, objValue, "pipTemplateInfo",
        CreatePipTemplateInfo(env, session->GetPiPTemplateInfo()));
    napi_set_named_property(env, objValue, "fbTemplateInfo",
        CreateFbTemplateInfo(env, session->GetFbTemplateInfo()));
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
    napi_set_named_property(env, objValue, "isSystemKeyboard",
        CreateJsValue(env, static_cast<int32_t>(session->IsSystemKeyboard())));
    napi_set_named_property(env, objValue, "bundleName",
        CreateJsValue(env, session->GetSessionInfo().bundleName_));
    napi_set_named_property(env, objValue, "startWindowType",
        CreateJsValue(env, session->GetSessionInfo().startWindowType_));
    napi_set_named_property(env, objValue, "windowName",
        CreateJsValue(env, session->GetWindowName()));
    napi_set_named_property(env, objValue, "zLevel",
        CreateJsValue(env, static_cast<int32_t>(session->GetSubWindowZLevel())));
    napi_set_named_property(env, objValue, "subWindowOutlineEnabled",
        CreateJsValue(env, session->IsSubWindowOutlineEnabled()));
    napi_set_named_property(env, objValue, "requestOrientation",
        CreateJsValue(env, session->GetSessionInfo().specificSessionRequestOrientation_));
    ParseMetadataConfiguration(env, objValue, session);
    sptr<WindowSessionProperty> sessionProperty = session->GetSessionProperty();
    if (sessionProperty != nullptr) {
        napi_set_named_property(env, objValue, "screenId",
            CreateJsValue(env, static_cast<int32_t>(sessionProperty->GetDisplayId())));
        napi_set_named_property(env, objValue, "isUIExtFirstSubWindow",
            CreateJsValue(env, static_cast<int32_t>(sessionProperty->GetIsUIExtFirstSubWindow())));
        napi_set_named_property(env, objValue, "zIndex",
            CreateJsValue(env, static_cast<int32_t>(sessionProperty->GetZIndex())));
    } else {
        napi_set_named_property(env, objValue, "screenId",
            CreateJsValue(env, static_cast<int32_t>(SCREEN_ID_INVALID)));
        napi_set_named_property(env, objValue, "zIndex",
            CreateJsValue(env, static_cast<int32_t>(SPECIFIC_ZINDEX_INVALID)));
        TLOGE(WmsLogTag::WMS_LIFE, "sessionProperty is nullptr!");
    }
    const char* moduleName = "JsSceneSession";
    BindNativeMethod(env, objValue, moduleName);
    BindNativeMethodForKeyboard(env, objValue, moduleName);
    BindNativeMethodForCompatiblePcMode(env, objValue, moduleName);
    BindNativeMethodForPcAppInPadNormal(env, objValue, moduleName);
    BindNativeMethodForSCBSystemSession(env, objValue, moduleName);
    BindNativeMethodForFocus(env, objValue, moduleName);
    BindNativeMethodForWaterfall(env, objValue, moduleName);
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
    BindNativeFunction(env, objValue, "activateDragBySystem", moduleName, JsSceneSession::ActivateDragBySystem);
    BindNativeFunction(env, objValue, "on", moduleName, JsSceneSession::RegisterCallback);
    BindNativeFunction(env, objValue, "updateNativeVisibility", moduleName, JsSceneSession::UpdateNativeVisibility);
    BindNativeFunction(env, objValue, "setShowRecent", moduleName, JsSceneSession::SetShowRecent);
    BindNativeFunction(env, objValue, "setZOrder", moduleName, JsSceneSession::SetZOrder);
    BindNativeFunction(env, objValue, "getZOrder", moduleName, JsSceneSession::GetZOrder);
    BindNativeFunction(env, objValue, "getUid", moduleName, JsSceneSession::GetUid);
    BindNativeFunction(env, objValue, "setTouchable", moduleName, JsSceneSession::SetTouchable);
    BindNativeFunction(env, objValue, "setWindowInputType", moduleName, JsSceneSession::SetWindowInputType);
    BindNativeFunction(env, objValue, "setExpandInputFlag", moduleName, JsSceneSession::SetExpandInputFlag);
    BindNativeFunction(env, objValue, "setSystemActive", moduleName, JsSceneSession::SetSystemActive);
    BindNativeFunction(env, objValue, "setPrivacyMode", moduleName, JsSceneSession::SetPrivacyMode);
    BindNativeFunction(env, objValue, "setSystemSceneOcclusionAlpha",
        moduleName, JsSceneSession::SetSystemSceneOcclusionAlpha);
    BindNativeFunction(env, objValue, "resetOcclusionAlpha", moduleName, JsSceneSession::ResetOcclusionAlpha);
    BindNativeFunction(env, objValue, "setSystemSceneForceUIFirst",
        moduleName, JsSceneSession::SetSystemSceneForceUIFirst);
    BindNativeFunction(env, objValue, "setUIFirstSwitch", moduleName, JsSceneSession::SetUIFirstSwitch);
    BindNativeFunction(env, objValue, "markSystemSceneUIFirst",
        moduleName, JsSceneSession::MarkSystemSceneUIFirst);
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
    BindNativeFunction(env, objValue, "notifyPipOcclusionChange", moduleName, JsSceneSession::NotifyPipOcclusionChange);
    BindNativeFunction(env, objValue, "notifyPipSizeChange", moduleName, JsSceneSession::NotifyPipSizeChange);
    BindNativeFunction(env, objValue, "notifyPipActiveStatusChange", moduleName,
        JsSceneSession::NotifyPipActiveWindowStatusChange);
    BindNativeFunction(env, objValue, "notifyDisplayStatusBarTemporarily", moduleName,
        JsSceneSession::NotifyDisplayStatusBarTemporarily);
    BindNativeFunction(env, objValue, "setTemporarilyShowWhenLocked", moduleName,
        JsSceneSession::SetTemporarilyShowWhenLocked);
    BindNativeFunction(env, objValue, "setSkipDraw", moduleName, JsSceneSession::SetSkipDraw);
    BindNativeFunction(env, objValue, "setSkipSelfWhenShowOnVirtualScreen", moduleName,
        JsSceneSession::SetSkipSelfWhenShowOnVirtualScreen);
    BindNativeFunction(env, objValue, "setSkipEventOnCastPlus", moduleName, JsSceneSession::SetSkipEventOnCastPlus);
    BindNativeFunction(env, objValue, "setUniqueDensityDpiFromSCB", moduleName,
        JsSceneSession::SetUniqueDensityDpiFromSCB);
    BindNativeFunction(env, objValue, "setBlank", moduleName, JsSceneSession::SetBlank);
    BindNativeFunction(env, objValue, "removeBlank", moduleName, JsSceneSession::RemoveBlank);
    BindNativeFunction(env, objValue, "addSnapshot", moduleName, JsSceneSession::AddSnapshot);
    BindNativeFunction(env, objValue, "removeSnapshot", moduleName, JsSceneSession::RemoveSnapshot);
    BindNativeFunction(env, objValue, "setBufferAvailableCallbackEnable", moduleName,
        JsSceneSession::SetBufferAvailableCallbackEnable);
    BindNativeFunction(env, objValue, "syncDefaultRequestedOrientation", moduleName,
        JsSceneSession::SyncDefaultRequestedOrientation);
    BindNativeFunction(env, objValue, "setIsPcAppInPad", moduleName,
        JsSceneSession::SetIsPcAppInPad);
    BindNativeFunction(env, objValue, "setStartingWindowExitAnimationFlag", moduleName,
        JsSceneSession::SetStartingWindowExitAnimationFlag);
    BindNativeFunction(env, objValue, "setWindowEnableDragBySystem", moduleName,
        JsSceneSession::SetWindowEnableDragBySystem);
    BindNativeFunction(env, objValue, "setNeedSyncSessionRect", moduleName,
        JsSceneSession::SetNeedSyncSessionRect);
    BindNativeFunction(env, objValue, "setIsPendingToBackgroundState", moduleName,
        JsSceneSession::SetIsPendingToBackgroundState);
    BindNativeFunction(env, objValue, "setIsActivatedAfterScreenLocked", moduleName,
        JsSceneSession::SetIsActivatedAfterScreenLocked);
    BindNativeFunction(env, objValue, "setLabel", moduleName, JsSceneSession::SetLabel);
    BindNativeFunction(env, objValue, "setFrameGravity", moduleName,
        JsSceneSession::SetFrameGravity);
    BindNativeFunction(env, objValue, "setUseStartingWindowAboveLocked", moduleName,
        JsSceneSession::SetUseStartingWindowAboveLocked);
    BindNativeFunction(env, objValue, "saveSnapshotSync", moduleName,
        JsSceneSession::SaveSnapshotSync);
    BindNativeFunction(env, objValue, "saveSnapshotAsync", moduleName,
        JsSceneSession::SaveSnapshotAsync);
    BindNativeFunction(env, objValue, "setBorderUnoccupied", moduleName,
        JsSceneSession::SetBorderUnoccupied);
    BindNativeFunction(env, objValue, "setEnableAddSnapshot", moduleName,
        JsSceneSession::SetEnableAddSnapshot);
    BindNativeFunction(env, objValue, "setFreezeImmediately", moduleName,
        JsSceneSession::SetFreezeImmediately);
    BindNativeFunction(env, objValue, "throwSlipDirectly", moduleName,
        JsSceneSession::ThrowSlipDirectly);
    BindNativeFunction(env, objValue, "sendContainerModalEvent", moduleName, JsSceneSession::SendContainerModalEvent);
    BindNativeFunction(env, objValue, "setColorSpace", moduleName, JsSceneSession::SetColorSpace);
    BindNativeFunction(env, objValue, "setSnapshotSkip", moduleName, JsSceneSession::SetSnapshotSkip);
    BindNativeFunction(env, objValue, "addSidebarBlur", moduleName, JsSceneSession::AddSidebarBlur);
    BindNativeFunction(env, objValue, "setSidebarBlur", moduleName, JsSceneSession::SetSidebarBlur);
    BindNativeFunction(env, objValue, "notifyRotationProperty", moduleName, JsSceneSession::NotifyRotationProperty);
    BindNativeFunction(env, objValue, "notifyPageRotationIsIgnored", moduleName,
        JsSceneSession::NotifyPageRotationIsIgnored);
    BindNativeFunction(env, objValue, "setCurrentRotation", moduleName, JsSceneSession::SetCurrentRotation);
    BindNativeFunction(env, objValue, "setSidebarBlurMaximize", moduleName, JsSceneSession::SetSidebarBlurMaximize);
    BindNativeFunction(env, objValue, "toggleCompatibleMode", moduleName, JsSceneSession::ToggleCompatibleMode);
    BindNativeFunction(env, objValue, "requestSpecificSessionClose", moduleName,
        JsSceneSession::RequestSpecificSessionClose);
    BindNativeFunction(env, objValue, "sendFbActionEvent", moduleName, JsSceneSession::SendFbActionEvent);
    BindNativeFunction(env, objValue, "setPcAppInpadSpecificSystemBarInvisible",
        moduleName, JsSceneSession::SetPcAppInpadSpecificSystemBarInvisible);
    BindNativeFunction(env, objValue, "setPcAppInpadOrientationLandscape",
        moduleName, JsSceneSession::SetPcAppInpadOrientationLandscape);
    BindNativeFunction(env, objValue, "setIsPcAppInpadCompatibleMode",
        moduleName, JsSceneSession::SetPcAppInpadCompatibleMode);
    BindNativeFunction(env, objValue, "updateSceneAnimationConfig", moduleName,
        JsSceneSession::UpdateSceneAnimationConfig);
    BindNativeFunction(env, objValue, "setMobileAppInPadLayoutFullScreen",
        moduleName, JsSceneSession::SetMobileAppInPadLayoutFullScreen);
}

void JsSceneSession::BindNativeMethodForKeyboard(napi_env env, napi_value objValue, const char* moduleName)
{
    BindNativeFunction(env, objValue, "setSCBKeepKeyboard", moduleName, JsSceneSession::SetSCBKeepKeyboard);
    BindNativeFunction(env, objValue, "requestHideKeyboard", moduleName, JsSceneSession::RequestHideKeyboard);
    BindNativeFunction(env, objValue, "openKeyboardSyncTransaction", moduleName,
        JsSceneSession::OpenKeyboardSyncTransaction);
    BindNativeFunction(env, objValue, "closeKeyboardSyncTransaction", moduleName,
        JsSceneSession::CloseKeyboardSyncTransaction);
    BindNativeFunction(env, objValue, "notifyKeyboardAnimationCompleted", moduleName,
        JsSceneSession::NotifyKeyboardAnimationCompleted);
    BindNativeFunction(env, objValue, "callingWindowStateChange", moduleName,
        JsSceneSession::CallingWindowStateChange);
}

void JsSceneSession::BindNativeMethodForCompatiblePcMode(napi_env env, napi_value objValue, const char* moduleName)
{
    BindNativeFunction(env, objValue, "setAppSupportPhoneInPc", moduleName,
        JsSceneSession::SetAppSupportPhoneInPc);
}

void JsSceneSession::BindNativeMethodForPcAppInPadNormal(napi_env env, napi_value objValue, const char* moduleName)
{
    BindNativeFunction(env, objValue, "pcAppInPadNormalClose", moduleName,
        JsSceneSession::PcAppInPadNormalClose);
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
    BindNativeFunction(env, objValue, "setExclusivelyHighlighted", moduleName,
        JsSceneSession::SetExclusivelyHighlighted);
}

void JsSceneSession::BindNativeMethodForWaterfall(napi_env env, napi_value objValue, const char* moduleName)
{
    BindNativeFunction(env, objValue, "maskSupportEnterWaterfallMode", moduleName,
        JsSceneSession::MaskSupportEnterWaterfallMode);
    BindNativeFunction(env, objValue, "updateFullScreenWaterfallMode", moduleName,
        JsSceneSession::UpdateFullScreenWaterfallMode);
}

JsSceneSession::JsSceneSession(napi_env env, const sptr<SceneSession>& session)
    : env_(env), weakSession_(session), persistentId_(session->GetPersistentId()),
      taskScheduler_(std::make_shared<MainThreadScheduler>(env))
{
    session->RegisterClearCallbackMapCallback([weakThis = wptr(this)](bool needRemove) {
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
    });

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
    SceneSessionManager::GetInstance().UnregisterSpecificSessionCreateListener(session->GetPersistentId());
}

void JsSceneSession::ProcessPendingSceneSessionActivationRegister()
{
    auto session = weakSession_.promote();
    if (session == nullptr) {
        TLOGE(WmsLogTag::WMS_LIFE, "session is nullptr, id:%{public}d", persistentId_);
        return;
    }
    const char* const where = __func__;
    session->SetPendingSessionActivationEventListener([weakThis = wptr(this), where](SessionInfo& info) {
        auto jsSceneSession = weakThis.promote();
        if (!jsSceneSession) {
            TLOGNE(WmsLogTag::WMS_LIFE, "%{public}s jsSceneSession is null", where);
            return;
        }
        jsSceneSession->PendingSessionActivation(info);
    });
    TLOGD(WmsLogTag::WMS_LIFE, "success");
}

void JsSceneSession::ProcessBatchPendingSceneSessionsActivationRegister()
{
    auto session = weakSession_.promote();
    if (session == nullptr) {
        TLOGE(WmsLogTag::WMS_LIFE, "session is nullptr, id:%{public}d", persistentId_);
        return;
    }
    const char* const where = __func__;
    session->SetBatchPendingSessionsActivationEventListener(
        [weakThis = wptr(this), where](std::vector<std::shared_ptr<SessionInfo>>& sessionInfos,
        const std::vector<std::shared_ptr<PendingSessionActivationConfig>>& configs) {
        auto jsSceneSession = weakThis.promote();
        if (!jsSceneSession) {
            TLOGNE(WmsLogTag::WMS_LIFE, "%{public}s jsSceneSession is null", where);
            return;
        }
        jsSceneSession->BatchPendingSessionsActivation(sessionInfos, configs);
    });
    TLOGD(WmsLogTag::WMS_LIFE, "success");
}

void JsSceneSession::ProcessWindowDragHotAreaRegister()
{
    TLOGD(WmsLogTag::DEFAULT, "[NAPI]");
    NotifyWindowDragHotAreaFunc func = [weakThis = wptr(this)](
        DisplayId displayId, uint32_t type, SizeChangeReason reason) {
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

void JsSceneSession::OnWindowDragHotArea(DisplayId displayId, uint32_t type, SizeChangeReason reason)
{
    TLOGD(WmsLogTag::DEFAULT, "[NAPI]");

    auto session = weakSession_.promote();
    if (session == nullptr) {
        WLOGFE("session is nullptr, id:%{public}d", persistentId_);
        return;
    }
    WSRect rect = session->GetSessionTargetRectByDisplayId(displayId);
    const char* const where = __func__;
    auto task = [weakThis = wptr(this), persistentId = persistentId_, env = env_,
                 displayId, type, reason, rect, where] {
        auto jsSceneSession = weakThis.promote();
        if (!jsSceneSession || jsSceneSessionMap_.find(persistentId) == jsSceneSessionMap_.end()) {
            TLOGNE(WmsLogTag::WMS_LAYOUT, "%{public}s: jsSceneSession id:%{public}d has been destroyed",
                where, persistentId);
            return;
        }
        auto jsCallBack = jsSceneSession->GetJSCallback(WINDOW_DRAG_HOT_AREA_CB);
        if (!jsCallBack) {
            TLOGNE(WmsLogTag::WMS_LAYOUT, "jsCallBack is nullptr");
            return;
        }
        napi_value jsHotAreaDisplayId = CreateJsValue(env, static_cast<int64_t>(displayId));
        if (jsHotAreaDisplayId == nullptr) {
            TLOGNE(WmsLogTag::WMS_LAYOUT, "jsHotAreaDisplayId is nullptr");
            return;
        }
        napi_value jsHotAreaType = CreateJsValue(env, type);
        if (jsHotAreaType == nullptr) {
            TLOGNE(WmsLogTag::WMS_LAYOUT, "jsHotAreaType is nullptr");
            return;
        }
        napi_value jsHotAreaReason = CreateJsValue(env, reason);
        if (jsHotAreaReason == nullptr) {
            TLOGNE(WmsLogTag::WMS_LAYOUT, "jsHotAreaReason is nullptr");
            return;
        }
        napi_value jsHotAreaRect = CreateJsSessionRect(env, rect);
        if (jsHotAreaRect == nullptr) {
            TLOGNE(WmsLogTag::WMS_LAYOUT, "jsHotAreaRect is nullptr");
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
            TLOGNE(WmsLogTag::WMS_LIFE, "ProcessSessionInfoLockedStateChangeRegister jsSceneSession is null");
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
    TLOGD(WmsLogTag::DEFAULT, "success");
}

void JsSceneSession::ProcessLandscapeMultiWindowRegister()
{
    TLOGD(WmsLogTag::WMS_MULTI_WINDOW, "in");
    auto session = weakSession_.promote();
    if (session == nullptr) {
        TLOGE(WmsLogTag::WMS_MULTI_WINDOW, "session is nullptr");
        return;
    }
    session->RegisterSetLandscapeMultiWindowFunc([weakThis = wptr(this)](bool isLandscapeMultiWindow) {
        auto jsSceneSession = weakThis.promote();
        if (!jsSceneSession) {
            TLOGNE(WmsLogTag::WMS_LIFE, "jsSceneSession is null");
            return;
        }
        jsSceneSession->SetLandscapeMultiWindow(isLandscapeMultiWindow);
    });
    TLOGD(WmsLogTag::WMS_MULTI_WINDOW, "success");
}

void JsSceneSession::SetLandscapeMultiWindow(bool isLandscapeMultiWindow)
{
    TLOGI(WmsLogTag::WMS_MULTI_WINDOW, "isLandscapeMultiWindow: %{public}u",
        isLandscapeMultiWindow);

    auto task = [weakThis = wptr(this), persistentId = persistentId_, isLandscapeMultiWindow, env = env_] {
        auto jsSceneSession = weakThis.promote();
        if (!jsSceneSession || jsSceneSessionMap_.find(persistentId) == jsSceneSessionMap_.end()) {
            TLOGNE(WmsLogTag::WMS_LIFE, "SetLandscapeMultiWindow jsSceneSession id:%{public}d has been destroyed",
                persistentId);
            return;
        }
        auto jsCallBack = jsSceneSession->GetJSCallback(LANDSCAPE_MULTI_WINDOW_CB);
        if (!jsCallBack) {
            TLOGNE(WmsLogTag::WMS_LIFE, "jsCallBack is nullptr");
            return;
        }
        napi_value jsSessionLandscapeMultiWindowObj = CreateJsValue(env, isLandscapeMultiWindow);
        napi_value argv[] = {jsSessionLandscapeMultiWindowObj};
        napi_call_function(env, NapiGetUndefined(env), jsCallBack->GetNapiValue(), ArraySize(argv), argv, nullptr);
    };
    taskScheduler_->PostMainThreadTask(task,
        "SetLandscapeMultiWindow, isLandscapeMultiWindow:" + std::to_string(isLandscapeMultiWindow));
}

void JsSceneSession::ProcessAdjustKeyboardLayoutRegister()
{
    NotifyKeyboardLayoutAdjustFunc func = [weakThis = wptr(this)](const KeyboardLayoutParams& params) {
        auto jsSceneSession = weakThis.promote();
        if (!jsSceneSession) {
            TLOGNE(WmsLogTag::WMS_LIFE, "ProcessAdjustKeyboardLayoutRegister jsSceneSession is null");
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
    auto session = weakSession_.promote();
    if (session == nullptr) {
        TLOGE(WmsLogTag::WMS_LAYOUT_PC, "session is nullptr, id:%{public}d", persistentId_);
        return;
    }
    const char* const where = __func__;
    auto layoutFullScreenChangeCallback = [weakThis = wptr(this), where](bool isLayoutFullScreen) {
        auto jsSceneSession = weakThis.promote();
        if (!jsSceneSession) {
            TLOGNE(WmsLogTag::WMS_LAYOUT_PC, "%{public}s jsSceneSession is null", where);
            return;
        }
        jsSceneSession->OnLayoutFullScreenChange(isLayoutFullScreen);
    };
    session->RegisterLayoutFullScreenChangeCallback(layoutFullScreenChangeCallback);
    TLOGD(WmsLogTag::WMS_LAYOUT_PC, "success");
}

void JsSceneSession::OnLayoutFullScreenChange(bool isLayoutFullScreen)
{
    const char* const where = __func__;
    auto task = [weakThis = wptr(this), persistentId = persistentId_, isLayoutFullScreen, env = env_, where] {
        auto jsSceneSession = weakThis.promote();
        if (!jsSceneSession || jsSceneSessionMap_.find(persistentId) == jsSceneSessionMap_.end()) {
            TLOGNE(WmsLogTag::WMS_LAYOUT_PC, "%{public}s jsSceneSession id: %{public}d has been destroyed",
                where, persistentId);
            return;
        }
        auto jsCallBack = jsSceneSession->GetJSCallback(LAYOUT_FULL_SCREEN_CB);
        if (!jsCallBack) {
            TLOGNE(WmsLogTag::WMS_LAYOUT_PC, "%{public}s jsCallBack is nullptr", where);
            return;
        }
        napi_value paramsObj = CreateJsValue(env, isLayoutFullScreen);
        napi_value argv[] = {paramsObj};
        napi_call_function(env, NapiGetUndefined(env), jsCallBack->GetNapiValue(), ArraySize(argv), argv, nullptr);
    };
    taskScheduler_->PostMainThreadTask(task, where);
}

void JsSceneSession::ProcessDefaultDensityEnabledRegister()
{
    auto session = weakSession_.promote();
    if (session == nullptr) {
        TLOGE(WmsLogTag::WMS_LIFE, "session is nullptr, id:%{public}d", persistentId_);
        return;
    }
    session->RegisterDefaultDensityEnabledCallback([weakThis = wptr(this)](bool isDefaultDensityEnabled) {
        auto jsSceneSession = weakThis.promote();
        if (!jsSceneSession) {
            TLOGNE(WmsLogTag::WMS_LIFE, "ProcessDefaultDensityEnabledRegister jsSceneSession is null");
            return;
        }
        jsSceneSession->OnDefaultDensityEnabled(isDefaultDensityEnabled);
    });
    TLOGI(WmsLogTag::WMS_ATTRIBUTE, "success");
}

void JsSceneSession::OnDefaultDensityEnabled(bool isDefaultDensityEnabled)
{
    auto task = [weakThis = wptr(this), persistentId = persistentId_, isDefaultDensityEnabled, env = env_] {
        auto jsSceneSession = weakThis.promote();
        if (!jsSceneSession || jsSceneSessionMap_.find(persistentId) == jsSceneSessionMap_.end()) {
            TLOGNE(WmsLogTag::WMS_LIFE, "OnDefaultDensityEnabled jsSceneSession id:%{public}d has been destroyed",
                persistentId);
            return;
        }
        auto jsCallBack = jsSceneSession->GetJSCallback(DEFAULT_DENSITY_ENABLED_CB);
        if (!jsCallBack) {
            TLOGNE(WmsLogTag::WMS_LAYOUT, "OnDefaultDensityEnabled jsCallBack is nullptr");
            return;
        }
        napi_value paramsObj = CreateJsValue(env, isDefaultDensityEnabled);
        napi_value argv[] = {paramsObj};
        napi_call_function(env, NapiGetUndefined(env), jsCallBack->GetNapiValue(), ArraySize(argv), argv, nullptr);
    };
    taskScheduler_->PostMainThreadTask(task, "OnDefaultDensityEnabled");
}

void JsSceneSession::ProcessWindowShadowEnableChangeRegister()
{
    auto session = weakSession_.promote();
    if (session == nullptr) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "session: %{public}d is null", persistentId_);
        return;
    }
    const char* const where = __func__;
    session->RegisterWindowShadowEnableChangeCallback([weakThis = wptr(this), where](bool isEnabled) {
        auto jsSceneSession = weakThis.promote();
        if (!jsSceneSession) {
            TLOGNE(WmsLogTag::WMS_ATTRIBUTE, "%{public}s jsSceneSession is null", where);
            return;
        }
        jsSceneSession->OnWindowShadowEnableChange(isEnabled);
    });
    TLOGI(WmsLogTag::WMS_ATTRIBUTE, "success");
}

void JsSceneSession::OnWindowShadowEnableChange(bool isEnabled)
{
    const char* const where = __func__;
    auto task = [weakThis = wptr(this), persistentId = persistentId_, isEnabled, env = env_, where] {
        auto jsSceneSession = weakThis.promote();
        if (!jsSceneSession || jsSceneSessionMap_.find(persistentId) == jsSceneSessionMap_.end()) {
            TLOGNE(WmsLogTag::WMS_ATTRIBUTE, "%{public}s jsSceneSession: %{public}d is null", where, persistentId);
            return;
        }
        auto jsCallBack = jsSceneSession->GetJSCallback(WINDOW_SHADOW_ENABLE_CHANGE_CB);
        if (!jsCallBack) {
            TLOGNE(WmsLogTag::WMS_ATTRIBUTE, "%{public}s jsCallBack is nullptr", where);
            return;
        }
        napi_value paramsObj = CreateJsValue(env, isEnabled);
        if (paramsObj == nullptr) {
            TLOGNE(WmsLogTag::WMS_ATTRIBUTE, "%{public}s params obj is null", where);
            return;
        }
        napi_value argv[] = {paramsObj};
        napi_call_function(env, NapiGetUndefined(env), jsCallBack->GetNapiValue(), ArraySize(argv), argv, nullptr);
    };
    taskScheduler_->PostMainThreadTask(task, __func__);
}

void JsSceneSession::ProcessTitleAndDockHoverShowChangeRegister()
{
    auto session = weakSession_.promote();
    if (session == nullptr) {
        TLOGE(WmsLogTag::WMS_LAYOUT_PC, "session is nullptr, id:%{public}d", persistentId_);
        return;
    }
    const char* const funcName = __func__;
    session->SetTitleAndDockHoverShowChangeCallback([weakThis = wptr(this), funcName](
        bool isTitleHoverShown, bool isDockHoverShown) {
        auto jsSceneSession = weakThis.promote();
        if (!jsSceneSession) {
            TLOGNE(WmsLogTag::WMS_LAYOUT_PC, "%{public}s jsSceneSession is null", funcName);
            return;
        }
        jsSceneSession->OnTitleAndDockHoverShowChange(isTitleHoverShown, isDockHoverShown);
    });
    TLOGD(WmsLogTag::WMS_LAYOUT_PC, "Register success, persistent id %{public}d", persistentId_);
}

void JsSceneSession::OnTitleAndDockHoverShowChange(bool isTitleHoverShown, bool isDockHoverShown)
{
    const char* const funcName = __func__;
    auto task = [weakThis = wptr(this), persistentId = persistentId_, isTitleHoverShown, isDockHoverShown,
        env = env_, funcName] {
        auto jsSceneSession = weakThis.promote();
        if (!jsSceneSession || jsSceneSessionMap_.find(persistentId) == jsSceneSessionMap_.end()) {
            TLOGNE(WmsLogTag::WMS_LAYOUT_PC, "%{public}s jsSceneSession id:%{public}d has been destroyed",
                funcName, persistentId);
            return;
        }
        auto jsCallBack = jsSceneSession->GetJSCallback(TITLE_DOCK_HOVER_SHOW_CB);
        if (!jsCallBack) {
            TLOGNE(WmsLogTag::WMS_LAYOUT_PC, "%{public}s jsCallBack is nullptr", funcName);
            return;
        }
        napi_value jsObjTitle = CreateJsValue(env, isTitleHoverShown);
        napi_value jsObjDock = CreateJsValue(env, isDockHoverShown);
        napi_value argv[] = {jsObjTitle, jsObjDock};
        napi_call_function(env, NapiGetUndefined(env), jsCallBack->GetNapiValue(), ArraySize(argv), argv, nullptr);
    };
    taskScheduler_->PostMainThreadTask(task, funcName);
}

void JsSceneSession::ProcessUseImplicitAnimationChangeRegister()
{
    auto session = weakSession_.promote();
    if (session == nullptr) {
        TLOGE(WmsLogTag::WMS_PC, "session is nullptr, id:%{public}d", persistentId_);
        return;
    }
    const char* const funcName = __func__;
    session->RegisterUseImplicitAnimationChangeCallback([weakThis = wptr(this), funcName](bool useImplicit) {
        auto jsSceneSession = weakThis.promote();
        if (!jsSceneSession) {
            TLOGNE(WmsLogTag::WMS_PC, "%{public}s jsSceneSession is null", funcName);
            return;
        }
        jsSceneSession->OnUseImplicitAnimationChange(useImplicit);
    });
    TLOGD(WmsLogTag::WMS_PC, "Register success, persistent id %{public}d", persistentId_);
}

void JsSceneSession::OnUseImplicitAnimationChange(bool useImplicit)
{
    const char* const funcName = __func__;
    auto task = [weakThis = wptr(this), persistentId = persistentId_, useImplicit,
        env = env_, funcName] {
        auto jsSceneSession = weakThis.promote();
        if (!jsSceneSession || jsSceneSessionMap_.find(persistentId) == jsSceneSessionMap_.end()) {
            TLOGNE(WmsLogTag::WMS_PC, "%{public}s jsSceneSession id:%{public}d has been destroyed",
                funcName, persistentId);
            return;
        }
        auto jsCallBack = jsSceneSession->GetJSCallback(USE_IMPLICITANIMATION_CB);
        if (!jsCallBack) {
            TLOGNE(WmsLogTag::WMS_PC, "%{public}s jsCallBack is nullptr", funcName);
            return;
        }
        napi_value jsObjUseImplicit = CreateJsValue(env, useImplicit);
        napi_value argv[] = {jsObjUseImplicit};
        napi_call_function(env, NapiGetUndefined(env), jsCallBack->GetNapiValue(), ArraySize(argv), argv, nullptr);
    };
    taskScheduler_->PostMainThreadTask(task, funcName);
}

void JsSceneSession::ProcessRestoreMainWindowRegister()
{
    auto session = weakSession_.promote();
    if (session == nullptr) {
        TLOGE(WmsLogTag::WMS_LAYOUT_PC, "session is nullptr, id:%{public}d", persistentId_);
        return;
    }
    const char* const funcName = __func__;
    session->SetRestoreMainWindowCallback([weakThis = wptr(this), funcName]
        (bool isAppSupportPhoneInPc, int32_t callingPid, uint32_t callingToken) {
        auto jsSceneSession = weakThis.promote();
        if (!jsSceneSession) {
            TLOGNE(WmsLogTag::WMS_LAYOUT_PC, "%{public}s jsSceneSession is null", funcName);
            return;
        }
        jsSceneSession->RestoreMainWindow(isAppSupportPhoneInPc, callingPid, callingToken);
    });
    TLOGD(WmsLogTag::WMS_LAYOUT_PC, "success");
}

void JsSceneSession::RestoreMainWindow(bool isAppSupportPhoneInPc, int32_t callingPid, uint32_t callingToken)
{
    const char* const funcName = __func__;
    auto task = [weakThis = wptr(this), persistentId = persistentId_, env = env_, funcName,
        isAppSupportPhoneInPc, callingPid, callingToken] {
        auto jsSceneSession = weakThis.promote();
        if (!jsSceneSession || jsSceneSessionMap_.find(persistentId) == jsSceneSessionMap_.end()) {
            TLOGNE(WmsLogTag::WMS_LAYOUT_PC, "%{public}s jsSceneSession id:%{public}d has been destroyed",
                funcName, persistentId);
            return;
        }
        auto jsCallBack = jsSceneSession->GetJSCallback(RESTORE_MAIN_WINDOW_CB);
        if (!jsCallBack) {
            TLOGNE(WmsLogTag::WMS_LAYOUT_PC, "%{public}s jsCallBack is nullptr", funcName);
            return;
        }
        napi_value jsIsAppSupportPhoneInPc = CreateJsValue(env, isAppSupportPhoneInPc);
        napi_value jsCallingPid = CreateJsValue(env, callingPid);
        napi_value jsCallingToken = CreateJsValue(env, callingToken);
        napi_value argv[] = {jsIsAppSupportPhoneInPc, jsCallingPid, jsCallingToken};
        napi_call_function(env, NapiGetUndefined(env), jsCallBack->GetNapiValue(), ArraySize(argv), argv, nullptr);
    };
    taskScheduler_->PostMainThreadTask(task, funcName);
}

void JsSceneSession::ProcessRestoreFloatMainWindowRegister()
{
    auto session = weakSession_.promote();
    if (session == nullptr) {
        TLOGE(WmsLogTag::WMS_LIFE, "session is nullptr, id:%{public}d", persistentId_);
        return;
    }
    const char* const funcName = __func__;
    session->SetRestoreFloatMainWindowCallback([weakThis = wptr(this), funcName]
        (const std::shared_ptr<AAFwk::WantParams>& wantParameters) {
        auto jsSceneSession = weakThis.promote();
        if (!jsSceneSession) {
            TLOGE(WmsLogTag::WMS_LIFE, "%{public}s jsSceneSession is null", funcName);
            return;
        }
        jsSceneSession->RestoreFloatMainWindow(wantParameters);
    });
    TLOGD(WmsLogTag::WMS_LIFE, "success");
}

void JsSceneSession::RestoreFloatMainWindow(const std::shared_ptr<AAFwk::WantParams>& wantParameters)
{
    const char* const funcName = __func__;
    auto task = [weakThis = wptr(this), persistentId = persistentId_, env = env_, wantParameters, funcName] {
        auto jsSceneSession = weakThis.promote();
        if (!jsSceneSession || jsSceneSessionMap_.find(persistentId) == jsSceneSessionMap_.end()) {
            TLOGE(WmsLogTag::WMS_LIFE, "%{public}s jsSceneSession id:%{public}d has been destroyed",
                funcName, persistentId);
            return;
        }
        auto jsCallBack = jsSceneSession->GetJSCallback(RESTORE_FLOAT_MAIN_WINDOW_CB);
        if (!jsCallBack) {
            TLOGE(WmsLogTag::WMS_LIFE, "%{public}s jsCallBack is nullptr", funcName);
            return;
        }
        napi_value jsWantParams = OHOS::AppExecFwk::WrapWantParams(env, *wantParameters);
        napi_value argv[] = {jsWantParams};
        napi_call_function(env, NapiGetUndefined(env), jsCallBack->GetNapiValue(), ArraySize(argv), argv, nullptr);
    };
    taskScheduler_->PostMainThreadTask(task, funcName);
}

void JsSceneSession::OnAdjustKeyboardLayout(const KeyboardLayoutParams& params)
{
    auto task = [weakThis = wptr(this), persistentId = persistentId_, params, env = env_] {
        auto jsSceneSession = weakThis.promote();
        if (!jsSceneSession || jsSceneSessionMap_.find(persistentId) == jsSceneSessionMap_.end()) {
            TLOGNE(WmsLogTag::WMS_LIFE, "OnAdjustKeyboardLayout jsSceneSession id:%{public}d has been destroyed",
                persistentId);
            return;
        }
        auto jsCallBack = jsSceneSession->GetJSCallback(ADJUST_KEYBOARD_LAYOUT_CB);
        if (!jsCallBack) {
            TLOGNE(WmsLogTag::WMS_KEYBOARD, "OnAdjustKeyboardLayout jsCallBack is nullptr");
            return;
        }
        napi_value keyboardLayoutParamsObj = CreateJsKeyboardLayoutParams(env, params);
        if (keyboardLayoutParamsObj == nullptr) {
            TLOGNE(WmsLogTag::WMS_KEYBOARD, "OnAdjustKeyboardLayout this keyboard layout params obj is nullptr");
        }
        napi_value argv[] = {keyboardLayoutParamsObj};
        napi_call_function(env, NapiGetUndefined(env), jsCallBack->GetNapiValue(), ArraySize(argv), argv, nullptr);
    };
    taskScheduler_->PostMainThreadTask(task, "OnAdjustKeyboardLayout");
}

void JsSceneSession::OnSessionInfoLockedStateChange(bool lockedState)
{
    TLOGI(WmsLogTag::DEFAULT, "%{public}u", lockedState);

    const char* const where = __func__;
    auto task = [weakThis = wptr(this), persistentId = persistentId_, lockedState, env = env_, where] {
        auto jsSceneSession = weakThis.promote();
        if (!jsSceneSession || jsSceneSessionMap_.find(persistentId) == jsSceneSessionMap_.end()) {
            TLOGNE(WmsLogTag::WMS_LIFE, "%{public}s: jsSceneSession id:%{public}d has been destroyed",
                where, persistentId);
            return;
        }
        auto jsCallBack = jsSceneSession->GetJSCallback(SESSIONINFO_LOCKEDSTATE_CHANGE_CB);
        if (!jsCallBack) {
            TLOGNE(WmsLogTag::WMS_LIFE, "jsCallBack is nullptr");
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
            TLOGNE(WmsLogTag::WMS_LIFE, "ProcessSessionDefaultAnimationFlagChangeRegister jsSceneSession is null");
            return;
        }
        jsSceneSession->OnDefaultAnimationFlagChange(isNeedDefaultAnimationFlag);
    });
}

void JsSceneSession::OnDefaultAnimationFlagChange(bool isNeedDefaultAnimationFlag)
{
    TLOGI(WmsLogTag::DEFAULT, "%{public}u", isNeedDefaultAnimationFlag);

    auto task = [weakThis = wptr(this), persistentId = persistentId_, isNeedDefaultAnimationFlag, env = env_] {
        auto jsSceneSession = weakThis.promote();
        if (!jsSceneSession || jsSceneSessionMap_.find(persistentId) == jsSceneSessionMap_.end()) {
            TLOGNE(WmsLogTag::WMS_LIFE, "OnDefaultAnimationFlagChange jsSceneSession id:%{public}d has been destroyed",
                persistentId);
            return;
        }
        auto jsCallBack = jsSceneSession->GetJSCallback(NEED_DEFAULT_ANIMATION_FLAG_CHANGE_CB);
        if (!jsCallBack) {
            TLOGNE(WmsLogTag::WMS_LIFE, "jsCallBack is nullptr");
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
    auto session = weakSession_.promote();
    if (session == nullptr) {
        WLOGFE("session is nullptr, id:%{public}d", persistentId_);
        return;
    }
    session->SetChangeSessionVisibilityWithStatusBarEventListener(
        [weakThis = wptr(this), where = __func__](const SessionInfo& info, bool visible) {
        auto jsSceneSession = weakThis.promote();
        if (!jsSceneSession) {
            TLOGNE(WmsLogTag::WMS_LIFE, "%{public}s jsSceneSession is null", where);
            return;
        }
        jsSceneSession->ChangeSessionVisibilityWithStatusBar(info, visible);
    });
    TLOGD(WmsLogTag::DEFAULT, "success");
}

void JsSceneSession::ProcessSessionStateChangeRegister()
{
    auto session = weakSession_.promote();
    if (session == nullptr) {
        WLOGFE("session is nullptr, id:%{public}d", persistentId_);
        return;
    }
    session->SetSessionStateChangeListenser([weakThis = wptr(this), where = __func__](const SessionState& state) {
        auto jsSceneSession = weakThis.promote();
        if (!jsSceneSession) {
            TLOGNE(WmsLogTag::WMS_LIFE, "%{public}s jsSceneSession is null", where);
            return;
        }
        jsSceneSession->OnSessionStateChange(state);
    });
    TLOGD(WmsLogTag::DEFAULT, "success");
}

void JsSceneSession::ProcessUpdateTransitionAnimationRegister()
{
    auto session = weakSession_.promote();
    if (session == nullptr) {
        TLOGE(WmsLogTag::WMS_ANIMATION, "session is nullptr, id:%{public}d", persistentId_);
        return;
    }
    session->SetTransitionAnimationCallback([weakThis = wptr(this), where = __func__](const WindowTransitionType& type,
        const TransitionAnimation& animation) {
        auto jsSceneSession = weakThis.promote();
        if (!jsSceneSession) {
            TLOGNE(WmsLogTag::WMS_ANIMATION, "%{public}s jsSceneSession is null", where);
            return;
        }
        jsSceneSession->OnUpdateTransitionAnimation(type, animation);
    });
    TLOGD(WmsLogTag::WMS_ANIMATION, "success");
}

void JsSceneSession::ProcessBufferAvailableChangeRegister()
{
    auto session = weakSession_.promote();
    if (session == nullptr) {
        WLOGFE("session is nullptr, id:%{public}d", persistentId_);
        return;
    }
    session->SetBufferAvailableChangeListener([weakThis = wptr(this), where = __func__]
        (const bool isAvailable, bool startWindowInvisible) {
        auto jsSceneSession = weakThis.promote();
        if (!jsSceneSession) {
            TLOGNE(WmsLogTag::WMS_LIFE, "%{public}s jsSceneSession is null", where);
            return;
        }
        jsSceneSession->OnBufferAvailableChange(isAvailable, startWindowInvisible);
    });
    TLOGD(WmsLogTag::DEFAULT, "success");
}

void JsSceneSession::ProcessCreateSubSessionRegister()
{
    auto session = weakSession_.promote();
    if (session == nullptr) {
        WLOGFE("session is nullptr, id:%{public}d", persistentId_);
        return;
    }
    SceneSessionManager::GetInstance().RegisterCreateSubSessionListener(session->GetPersistentId(),
        [weakThis = wptr(this)](const sptr<SceneSession>& sceneSession) {
            auto jsSceneSession = weakThis.promote();
            if (!jsSceneSession) {
                TLOGNE(WmsLogTag::WMS_LIFE, "ProcessCreateSubSessionRegister jsSceneSession is null");
                return;
            }
            jsSceneSession->OnCreateSubSession(sceneSession);
        });
    TLOGD(WmsLogTag::DEFAULT, "success, id: %{public}d", session->GetPersistentId());
}

void JsSceneSession::ProcessClearSubSessionRegister()
{
    auto session = weakSession_.promote();
    if (session == nullptr) {
        TLOGE(WmsLogTag::WMS_LIFE, "session is nullptr, id:%{public}d", persistentId_);
        return;
    }
    session->SetClearSubSessionCallback([weakThis = wptr(this)](const int32_t subPersistentId) {
        auto jsSceneSession = weakThis.promote();
        if (!jsSceneSession) {
            TLOGNE(WmsLogTag::WMS_LIFE, "ProcessClearSubSessionRegister jsSceneSession is null");
            return;
        }
        jsSceneSession->OnClearSubSession(subPersistentId);
    });
    TLOGI(WmsLogTag::WMS_LIFE, "success");
}

void JsSceneSession::ProcessBindDialogTargetRegister()
{
    auto session = weakSession_.promote();
    if (session == nullptr) {
        TLOGE(WmsLogTag::WMS_DIALOG, "session is nullptr, id:%{public}d", persistentId_);
        return;
    }
    SceneSessionManager::GetInstance().RegisterBindDialogTargetListener(session,
        [weakThis = wptr(this)](const sptr<SceneSession>& sceneSession) {
            auto jsSceneSession = weakThis.promote();
            if (!jsSceneSession) {
                TLOGNE(WmsLogTag::WMS_LIFE, "jsSceneSession is null");
                return;
            }
            jsSceneSession->OnBindDialogTarget(sceneSession);
        });
    TLOGD(WmsLogTag::WMS_DIALOG, "success");
}

void JsSceneSession::ProcessSessionRectChangeRegister()
{
    auto session = weakSession_.promote();
    if (session == nullptr) {
        WLOGFE("session is nullptr, id:%{public}d", persistentId_);
        return;
    }
    session->SetSessionRectChangeCallback([weakThis = wptr(this)](const WSRect& rect,
        SizeChangeReason reason, DisplayId displayId = DISPLAY_ID_INVALID,
        const RectAnimationConfig& rectAnimationConfig = {}) {
        auto jsSceneSession = weakThis.promote();
        if (!jsSceneSession) {
            TLOGNE(WmsLogTag::WMS_LIFE, "ProcessSessionRectChangeRegister jsSceneSession is null");
            return;
        }
        jsSceneSession->OnSessionRectChange(rect, reason, displayId, rectAnimationConfig);
    });
    TLOGD(WmsLogTag::DEFAULT, "success");
}

void JsSceneSession::ProcessSessionWindowLimitsChangeRegister()
{
    auto session = weakSession_.promote();
    if (session == nullptr) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "session is nullptr, id:%{public}d", persistentId_);
        return;
    }
    session->SetSessionWindowLimitsChangeCallback([weakThis = wptr(this)](const WindowLimits& windowLimits) {
        auto jsSceneSession = weakThis.promote();
        if (!jsSceneSession) {
            TLOGNE(WmsLogTag::WMS_LAYOUT, "ProcessSessionWindowLimitsChangeRegister jsSceneSession is null");
            return;
        }
        jsSceneSession->OnSessionWindowLimitsChange(windowLimits);
    });
    TLOGD(WmsLogTag::WMS_LAYOUT, "success");
}

void JsSceneSession::ProcessFloatingBallUpdateRegister()
{
    auto session = weakSession_.promote();
    if (session == nullptr) {
        TLOGE(WmsLogTag::WMS_LIFE, "session is nullptr, id:%{public}d", persistentId_);
        return;
    }
    session->SetFloatingBallUpdateCallback([weakThis = wptr(this)](const FloatingBallTemplateInfo& fbTemplateInfo) {
        auto jsSceneSession = weakThis.promote();
        if (!jsSceneSession) {
            TLOGE(WmsLogTag::WMS_LIFE, "ProcessFloatingBallUpdateRegister jsSceneSession is null");
            return;
        }
        jsSceneSession->OnFloatingBallUpdate(fbTemplateInfo);
    });
    TLOGD(WmsLogTag::WMS_LIFE, "success");
}

void JsSceneSession::ProcessFloatingBallStopRegister()
{
    auto session = weakSession_.promote();
    if (session == nullptr) {
        TLOGE(WmsLogTag::WMS_LIFE, "session is nullptr, id:%{public}d", persistentId_);
        return;
    }
    session->SetFloatingBallStopCallback([weakThis = wptr(this)]() {
        auto jsSceneSession = weakThis.promote();
        if (!jsSceneSession) {
            TLOGE(WmsLogTag::WMS_LIFE, "ProcessFloatingBallStopRegister jsSceneSession is null");
            return;
        }
        jsSceneSession->OnFloatingBallStop();
    });
    TLOGD(WmsLogTag::WMS_LIFE, "success");
}

void JsSceneSession::ProcessFloatingBallRestoreMainWindowRegister()
{
    auto session = weakSession_.promote();
    if (session == nullptr) {
        TLOGE(WmsLogTag::WMS_LIFE, "session is nullptr, id:%{public}d", persistentId_);
        return;
    }
    session->SetFloatingBallRestoreMainWindowCallback([weakThis = wptr(this)](
        const std::shared_ptr<AAFwk::Want>& want) {
        auto jsSceneSession = weakThis.promote();
        if (!jsSceneSession) {
            TLOGE(WmsLogTag::WMS_LIFE, "ProcessFloatingBallRestoreMainWindowRegister jsSceneSession is null");
            return;
        }
        jsSceneSession->OnFloatingBallRestoreMainWindow(want);
    });
    TLOGD(WmsLogTag::WMS_LIFE, "success");
}

void JsSceneSession::ProcessSessionDisplayIdChangeRegister()
{
    auto session = weakSession_.promote();
    if (session == nullptr) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "session is nullptr, id:%{public}d", persistentId_);
        return;
    }
    session->SetSessionDisplayIdChangeCallback([weakThis = wptr(this), where = __func__](uint64_t displayId) {
        auto jsSceneSession = weakThis.promote();
        if (!jsSceneSession) {
            TLOGNE(WmsLogTag::WMS_LAYOUT, "%{public}s jsSceneSession is null", where);
            return;
        }
        jsSceneSession->OnSessionDisplayIdChange(displayId);
    });
}

void JsSceneSession::ProcessWindowMovingRegister()
{
    auto session = weakSession_.promote();
    if (session == nullptr) {
        TLOGE(WmsLogTag::WMS_DECOR, "session is nullptr, id:%{public}d", persistentId_);
        return;
    }
    session->SetWindowMovingCallback([weakThis = wptr(this)](DisplayId displayId, int32_t pointerX, int32_t pointerY) {
        auto jsSceneSession = weakThis.promote();
        if (!jsSceneSession) {
            TLOGNE(WmsLogTag::WMS_DECOR, "ProcessWindowMovingRegister jsSceneSession is null");
            return;
        }
        jsSceneSession->OnWindowMoving(displayId, pointerX, pointerY);
    });
    TLOGI(WmsLogTag::WMS_DECOR, "success");
}

void JsSceneSession::ProcessSessionPiPControlStatusChangeRegister()
{
    auto session = weakSession_.promote();
    if (session == nullptr) {
        TLOGE(WmsLogTag::WMS_PIP, "session is nullptr, id:%{public}d", persistentId_);
        return;
    }
    session->SetSessionPiPControlStatusChangeCallback([weakThis = wptr(this), where = __func__](
        WsPiPControlType controlType, WsPiPControlStatus status) {
        auto jsSceneSession = weakThis.promote();
        if (!jsSceneSession) {
            TLOGNE(WmsLogTag::WMS_LIFE, "%{public}s jsSceneSession is null", where);
            return;
        }
        jsSceneSession->OnSessionPiPControlStatusChange(controlType, status);
    });
    TLOGI(WmsLogTag::WMS_PIP, "success");
}

void JsSceneSession::ProcessAutoStartPiPStatusChangeRegister()
{
    auto session = weakSession_.promote();
    if (session == nullptr) {
        TLOGE(WmsLogTag::WMS_PIP, "session is nullptr, id:%{public}d", persistentId_);
        return;
    }
    session->SetAutoStartPiPStatusChangeCallback([weakThis = wptr(this)](bool isAutoStart, uint32_t priority,
        uint32_t width, uint32_t height) {
        auto jsSceneSession = weakThis.promote();
        if (!jsSceneSession) {
            TLOGNE(WmsLogTag::WMS_PIP, "jsSceneSession is null");
            return;
        }
        jsSceneSession->OnAutoStartPiPStatusChange(isAutoStart, priority, width, height);
    });
    TLOGI(WmsLogTag::WMS_PIP, "success");
}

void JsSceneSession::ProcessUpdatePiPTemplateInfoRegister()
{
    auto session = weakSession_.promote();
    if (session == nullptr) {
        TLOGE(WmsLogTag::WMS_PIP, "session is nullptr, id:%{public}d", persistentId_);
        return;
    }
    session->SetUpdatePiPTemplateInfoCallback([weakThis = wptr(this)](PiPTemplateInfo& pipTemplateInfo) {
        auto jsSceneSession = weakThis.promote();
        if (!jsSceneSession) {
            TLOGNE(WmsLogTag::WMS_PIP, "jsSceneSession is null");
            return;
        }
        jsSceneSession->OnUpdatePiPTemplateInfo(pipTemplateInfo);
    });
    TLOGI(WmsLogTag::WMS_PIP, "success");
}

void JsSceneSession::ProcessSetPiPParentWindowIdRegister()
{
    auto session = weakSession_.promote();
    if (session == nullptr) {
        TLOGE(WmsLogTag::WMS_PIP, "session is nullptr, id:%{public}d", persistentId_);
        return;
    }
    session->SetPipParentWindowIdCallback([weakThis = wptr(this)](uint32_t windowId) {
        auto jsSceneSession = weakThis.promote();
        if (!jsSceneSession) {
            TLOGNE(WmsLogTag::WMS_PIP, "jsSceneSession is null");
            return;
        }
        jsSceneSession->OnSetPiPParentWindowId(windowId);
    });
    TLOGI(WmsLogTag::WMS_PIP, "success");
}

/** @note @window.hierarchy */
void JsSceneSession::ProcessRaiseToTopRegister()
{
    auto session = weakSession_.promote();
    if (session == nullptr) {
        TLOGE(WmsLogTag::WMS_LIFE, "session is nullptr, id:%{public}d", persistentId_);
        return;
    }
    session->RegisterRaiseToTopCallback([weakThis = wptr(this)] {
        auto jsSceneSession = weakThis.promote();
        if (!jsSceneSession) {
            TLOGNE(WmsLogTag::WMS_LIFE, "jsSceneSession is null");
            return;
        }
        jsSceneSession->OnRaiseToTop();
    });
    TLOGD(WmsLogTag::WMS_HIERARCHY, "success");
}

/** @note @window.hierarchy */
void JsSceneSession::ProcessRaiseToTopForPointDownRegister()
{
    auto session = weakSession_.promote();
    if (session == nullptr) {
        WLOGFE("session is nullptr, id:%{public}d", persistentId_);
        return;
    }
    session->SetRaiseToAppTopForPointDownFunc([weakThis = wptr(this)]() {
        auto jsSceneSession = weakThis.promote();
        if (!jsSceneSession) {
            TLOGNE(WmsLogTag::WMS_LIFE, "ProcessRaiseToTopForPointDownRegister jsSceneSession is null");
            return;
        }
        jsSceneSession->OnRaiseToTopForPointDown();
    });
    TLOGD(WmsLogTag::DEFAULT, "success");
}

void JsSceneSession::ProcessClickModalWindowOutsideRegister()
{
    auto session = weakSession_.promote();
    if (session == nullptr) {
        TLOGE(WmsLogTag::WMS_LIFE, "session is nullptr, id:%{public}d", persistentId_);
        return;
    }
    const char* const where = __func__;
    session->SetClickModalWindowOutsideListener([where, weakThis = wptr(this)] {
        auto jsSceneSession = weakThis.promote();
        if (!jsSceneSession) {
            TLOGNE(WmsLogTag::WMS_LIFE, "%{public}s jsSceneSession is null", where);
            return;
        }
        jsSceneSession->OnClickModalWindowOutside();
    });
    TLOGD(WmsLogTag::WMS_LAYOUT, "success");
}

/** @note @window.hierarchy */
void JsSceneSession::ProcessRaiseAboveTargetRegister()
{
    auto session = weakSession_.promote();
    if (session == nullptr) {
        TLOGE(WmsLogTag::WMS_LIFE, "session is nullptr, id:%{public}d", persistentId_);
        return;
    }
    session->RegisterRaiseAboveTargetCallback([weakThis = wptr(this)](int32_t subWindowId) {
        auto jsSceneSession = weakThis.promote();
        if (!jsSceneSession) {
            TLOGNE(WmsLogTag::WMS_LIFE, "jsSceneSession is null");
            return;
        }
        jsSceneSession->OnRaiseAboveTarget(subWindowId);
    });
    TLOGD(WmsLogTag::WMS_HIERARCHY, "success");
}

/** @note @window.hierarchy */
void JsSceneSession::ProcessRaiseMainWindowAboveTargetRegister()
{
    auto session = weakSession_.promote();
    if (session == nullptr) {
        TLOGE(WmsLogTag::WMS_LIFE, "session is nullptr, id:%{public}d", persistentId_);
        return;
    }
    session->RegisterRaiseMainWindowAboveTargetCallback([weakThis = wptr(this)](int32_t targetId) {
        auto jsSceneSession = weakThis.promote();
        if (!jsSceneSession) {
            TLOGNE(WmsLogTag::WMS_LIFE, "jsSceneSession is null");
            return;
        }
        jsSceneSession->OnRaiseMainWindowAboveTarget(targetId);
    });
    TLOGD(WmsLogTag::WMS_HIERARCHY, "success");
}

void JsSceneSession::ProcessSessionEventRegister()
{
    auto session = weakSession_.promote();
    if (session == nullptr) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "session is null, id:%{public}d", persistentId_);
        return;
    }
    session->RegisterSessionEventCallback([weakThis = wptr(this)](uint32_t eventId, const SessionEventParam& param) {
        auto jsSceneSession = weakThis.promote();
        if (!jsSceneSession) {
            TLOGNE(WmsLogTag::WMS_LAYOUT, "ProcessSessionEventRegister jsSceneSession is null");
            return;
        }
        jsSceneSession->OnSessionEvent(eventId, param);
    });
    TLOGD(WmsLogTag::WMS_LAYOUT, "success");
}

void JsSceneSession::ProcessTerminateSessionRegister()
{
    TLOGD(WmsLogTag::WMS_LIFE, "in");
    auto session = weakSession_.promote();
    if (session == nullptr) {
        TLOGE(WmsLogTag::WMS_LIFE, "session is nullptr, id:%{public}d", persistentId_);
        return;
    }
    const char* const where = __func__;
    session->SetTerminateSessionListener([weakThis = wptr(this), persistentId = persistentId_, where](
        const SessionInfo& info) {
        auto jsSceneSession = weakThis.promote();
        if (!jsSceneSession) {
            TLOGNE(WmsLogTag::WMS_LIFE, "%{public}s jsSceneSession is null", where);
            SceneSessionManager::GetInstance().RemoveLifeCycleTaskByPersistentId(
                persistentId, LifeCycleTaskType::STOP);
            return;
        }
        jsSceneSession->TerminateSession(info);
    });
    TLOGD(WmsLogTag::WMS_LIFE, "success");
}

void JsSceneSession::ProcessTerminateSessionRegisterNew()
{
    TLOGD(WmsLogTag::WMS_LIFE, "in");
    auto session = weakSession_.promote();
    if (session == nullptr) {
        TLOGE(WmsLogTag::WMS_LIFE, "session is nullptr, id:%{public}d", persistentId_);
        return;
    }
    const char* const where = __func__;
    session->SetTerminateSessionListenerNew([weakThis = wptr(this), persistentId = persistentId_, where](
        const SessionInfo& info, bool needStartCaller, bool isFromBroker, bool isForceClean) {
        auto jsSceneSession = weakThis.promote();
        if (!jsSceneSession) {
            TLOGNE(WmsLogTag::WMS_LIFE, "%{public}s jsSceneSession is null", where);
            SceneSessionManager::GetInstance().RemoveLifeCycleTaskByPersistentId(
                persistentId, LifeCycleTaskType::STOP);
            return;
        }
        jsSceneSession->TerminateSessionNew(info, needStartCaller, isFromBroker, isForceClean);
    });
    TLOGD(WmsLogTag::WMS_LIFE, "success");
}

void JsSceneSession::ProcessTerminateSessionRegisterTotal()
{
    TLOGD(WmsLogTag::WMS_LIFE, "in");
    auto session = weakSession_.promote();
    if (session == nullptr) {
        TLOGE(WmsLogTag::WMS_LIFE, "session is nullptr, id:%{public}d", persistentId_);
        return;
    }
    const char* const where = __func__;
    session->SetTerminateSessionListenerTotal([weakThis = wptr(this), where](
        const SessionInfo& info, TerminateType terminateType) {
        auto jsSceneSession = weakThis.promote();
        if (!jsSceneSession) {
            TLOGNE(WmsLogTag::WMS_LIFE, "%{public}s jsSceneSession is null", where);
            return;
        }
        jsSceneSession->TerminateSessionTotal(info, terminateType);
    });
    TLOGD(WmsLogTag::WMS_LIFE, "success");
}

void JsSceneSession::ProcessPendingSessionToForegroundRegister()
{
    TLOGD(WmsLogTag::WMS_LIFE, "in");
    auto session = weakSession_.promote();
    if (session == nullptr) {
        TLOGE(WmsLogTag::WMS_LIFE, "session is nullptr, id:%{public}d", persistentId_);
        return;
    }
    const char* const where = __func__;
    session->SetPendingSessionToForegroundListener([weakThis = wptr(this), where](const SessionInfo& info) {
        auto jsSceneSession = weakThis.promote();
        if (!jsSceneSession) {
            TLOGNE(WmsLogTag::WMS_LIFE, "%{public}s jsSceneSession is null", where);
            return;
        }
        jsSceneSession->PendingSessionToForeground(info);
    });
    TLOGD(WmsLogTag::WMS_LIFE, "success");
}

void JsSceneSession::ProcessPendingSessionToBackgroundRegister()
{
    TLOGD(WmsLogTag::WMS_LIFE, "in");
    auto session = weakSession_.promote();
    if (session == nullptr) {
        TLOGE(WmsLogTag::WMS_LIFE, "session is nullptr, id:%{public}d", persistentId_);
        return;
    }
    const char* const where = __func__;
    session->SetPendingSessionToBackgroundListener([weakThis = wptr(this), where](
        const SessionInfo& info, const BackgroundParams& params) {
        auto jsSceneSession = weakThis.promote();
        if (!jsSceneSession) {
            TLOGNE(WmsLogTag::WMS_LIFE, "%{public}s jsSceneSession is null", where);
            return;
        }
        jsSceneSession->PendingSessionToBackground(info, params);
    });
    TLOGD(WmsLogTag::WMS_LIFE, "success");
}

void JsSceneSession::ProcessPendingSessionToBackgroundForDelegatorRegister()
{
    TLOGD(WmsLogTag::WMS_LIFE, "in");
    auto session = weakSession_.promote();
    if (session == nullptr) {
        TLOGE(WmsLogTag::WMS_LIFE, "session is nullptr, id:%{public}d", persistentId_);
        return;
    }
    const char* const where = __func__;
    session->SetPendingSessionToBackgroundForDelegatorListener([weakThis = wptr(this), where](
        const SessionInfo& info, bool shouldBackToCaller) {
        auto jsSceneSession = weakThis.promote();
        if (!jsSceneSession) {
            TLOGNE(WmsLogTag::WMS_LIFE, "%{public}s jsSceneSession is null", where);
            return;
        }
        jsSceneSession->PendingSessionToBackgroundForDelegator(info, shouldBackToCaller);
    });
    TLOGD(WmsLogTag::WMS_LIFE, "success");
}

void JsSceneSession::ProcessSessionExceptionRegister()
{
    TLOGD(WmsLogTag::WMS_LIFE, "in");
    auto session = weakSession_.promote();
    if (session == nullptr) {
        TLOGE(WmsLogTag::WMS_LIFE, "session is nullptr, id:%{public}d", persistentId_);
        return;
    }
    const char* const where = __func__;
    session->SetSessionExceptionListener([weakThis = wptr(this), persistentId = persistentId_, where](
        const SessionInfo& info, const ExceptionInfo& exceptionInfo, bool startFail) {
        auto jsSceneSession = weakThis.promote();
        if (!jsSceneSession) {
            TLOGNE(WmsLogTag::WMS_LIFE, "%{public}s jsSceneSession is null", where);
            SceneSessionManager::GetInstance().RemoveLifeCycleTaskByPersistentId(
                persistentId, LifeCycleTaskType::STOP);
            return;
        }
        jsSceneSession->OnSessionException(info, exceptionInfo, startFail);
    }, true);
}

/** @note @window.hierarchy */
void JsSceneSession::ProcessSessionTopmostChangeRegister()
{
    auto session = weakSession_.promote();
    if (session == nullptr) {
        TLOGE(WmsLogTag::WMS_LIFE, "session is nullptr, id:%{public}d", persistentId_);
        return;
    }
    session->RegisterSessionTopmostChangeCallback([weakThis = wptr(this)](bool topmost) {
        auto jsSceneSession = weakThis.promote();
        if (!jsSceneSession) {
            TLOGNE(WmsLogTag::WMS_LIFE, "jsSceneSession is null");
            return;
        }
        jsSceneSession->OnSessionTopmostChange(topmost);
    });
    TLOGD(WmsLogTag::WMS_HIERARCHY, "success");
}

/** @note @window.hierarchy */
void JsSceneSession::ProcessMainWindowTopmostChangeRegister()
{
    auto session = weakSession_.promote();
    if (session == nullptr) {
        TLOGE(WmsLogTag::WMS_HIERARCHY, "session is nullptr, id:%{public}d", persistentId_);
        return;
    }
    const char* const where = __func__;
    session->SetMainWindowTopmostChangeCallback([weakThis = wptr(this), where](bool isTopmost) {
        auto jsSceneSession = weakThis.promote();
        if (!jsSceneSession) {
            TLOGNE(WmsLogTag::WMS_HIERARCHY, "%{public}s jsSceneSession is null", where);
            return;
        }
        jsSceneSession->OnMainWindowTopmostChange(isTopmost);
    });
    TLOGD(WmsLogTag::WMS_HIERARCHY, "register success");
}

/** @note @window.hierarchy */
void JsSceneSession::ProcessSubWindowZLevelChangeRegister()
{
    auto session = weakSession_.promote();
    if (session == nullptr) {
        TLOGE(WmsLogTag::WMS_HIERARCHY, "session is nullptr, persistentId:%{public}d", persistentId_);
        return;
    }
    session->RegisterSubSessionZLevelChangeCallback([weakThis = wptr(this)](int32_t zLevel) {
        auto jsSceneSession = weakThis.promote();
        if (!jsSceneSession) {
            TLOGNE(WmsLogTag::WMS_LIFE, "jsSceneSession is null");
            return;
        }
        jsSceneSession->OnSubSessionZLevelChange(zLevel);
    });
    TLOGD(WmsLogTag::WMS_HIERARCHY, "register success, persistentId:%{public}d", persistentId_);
}

void JsSceneSession::ProcessSubModalTypeChangeRegister()
{
    auto session = weakSession_.promote();
    if (session == nullptr) {
        TLOGE(WmsLogTag::WMS_HIERARCHY, "session is nullptr, persistentId:%{public}d", persistentId_);
        return;
    }
    const char* const where = __func__;
    session->RegisterSubModalTypeChangeCallback([weakThis = wptr(this), where](SubWindowModalType subWindowModalType) {
        auto jsSceneSession = weakThis.promote();
        if (!jsSceneSession) {
            TLOGNE(WmsLogTag::WMS_LIFE, "%{public}s jsSceneSession is null", where);
            return;
        }
        jsSceneSession->OnSubModalTypeChange(subWindowModalType);
    });
    TLOGD(WmsLogTag::WMS_HIERARCHY, "register success, persistentId:%{public}d", persistentId_);
}

void JsSceneSession::ProcessMainModalTypeChangeRegister()
{
    auto session = weakSession_.promote();
    if (session == nullptr) {
        TLOGE(WmsLogTag::WMS_LIFE, "session is nullptr, persistentId:%{public}d", persistentId_);
        return;
    }
    const char* const where = __func__;
    session->RegisterMainModalTypeChangeCallback([weakThis = wptr(this), where](bool isModal) {
        auto jsSceneSession = weakThis.promote();
        if (!jsSceneSession) {
            TLOGNE(WmsLogTag::WMS_LIFE, "%{public}s jsSceneSession is null", where);
            return;
        }
        jsSceneSession->OnMainModalTypeChange(isModal);
    });
    TLOGD(WmsLogTag::WMS_HIERARCHY, "register success, persistentId:%{public}d", persistentId_);
}

void JsSceneSession::RegisterThrowSlipAnimationStateChangeCallback()
{
    auto session = weakSession_.promote();
    if (session == nullptr) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "session is nullptr, persistent id: %{public}d", persistentId_);
        return;
    }
    const char* const where = __func__;
    session->RegisterThrowSlipAnimationStateChangeCallback(
        [weakThis = wptr(this), where](bool isAnimating, bool isFullScreen) {
        auto jsSceneSession = weakThis.promote();
        if (!jsSceneSession) {
            TLOGNE(WmsLogTag::WMS_LAYOUT_PC, "%{public}s jsSceneSession is null", where);
            return;
        }
        jsSceneSession->OnThrowSlipAnimationStateChange(isAnimating, isFullScreen);
    });
    TLOGD(WmsLogTag::WMS_LAYOUT_PC, "register success, persistent id: %{public}d", persistentId_);
}

void JsSceneSession::RegisterFullScreenWaterfallModeChangeCallback()
{
    auto session = weakSession_.promote();
    if (session == nullptr) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "session is nullptr, persistent id: %{public}d", persistentId_);
        return;
    }
    const char* const where = __func__;
    session->RegisterFullScreenWaterfallModeChangeCallback([weakThis = wptr(this), where](bool isWaterfallMode) {
        auto jsSceneSession = weakThis.promote();
        if (!jsSceneSession) {
            TLOGNE(WmsLogTag::WMS_LAYOUT, "%{public}s jsSceneSession is null", where);
            return;
        }
        jsSceneSession->OnFullScreenWaterfallModeChange(isWaterfallMode);
    });
    TLOGD(WmsLogTag::WMS_LAYOUT, "register success, persistent id: %{public}d", persistentId_);
}

void JsSceneSession::ProcessSessionFocusableChangeRegister()
{
    NotifySessionFocusableChangeFunc func = [weakThis = wptr(this)](bool isFocusable) {
        auto jsSceneSession = weakThis.promote();
        if (!jsSceneSession) {
            TLOGNE(WmsLogTag::WMS_LIFE, "ProcessSessionFocusableChangeRegister jsSceneSession is null");
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
            TLOGNE(WmsLogTag::WMS_LIFE, "ProcessSessionTouchableChangeRegister jsSceneSession is null");
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
    TLOGD(WmsLogTag::DEFAULT, "success");
}

void JsSceneSession::ProcessClickRegister()
{
    NotifyClickFunc func = [weakThis = wptr(this)](bool requestFocus, bool isClick) {
        auto jsSceneSession = weakThis.promote();
        if (!jsSceneSession) {
            TLOGNE(WmsLogTag::WMS_LIFE, "ProcessClickRegister jsSceneSession is null");
            return;
        }
        jsSceneSession->OnClick(requestFocus, isClick);
    };
    auto session = weakSession_.promote();
    if (session == nullptr) {
        WLOGFE("session is nullptr, id:%{public}d", persistentId_);
        return;
    }
    session->SetClickListener(func);
    TLOGD(WmsLogTag::DEFAULT, "success");
}

void JsSceneSession::ProcessContextTransparentRegister()
{
    NotifyContextTransparentFunc func = [weakThis = wptr(this)]() {
        auto jsSceneSession = weakThis.promote();
        if (!jsSceneSession) {
            TLOGNE(WmsLogTag::WMS_LIFE, "ProcessContextTransparentRegister jsSceneSession is null");
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
    TLOGD(WmsLogTag::DEFAULT, "success");
}

void JsSceneSession::OnSessionEvent(uint32_t eventId, const SessionEventParam& param)
{
    TLOGI(WmsLogTag::DEFAULT, "%{public}d", eventId);

    auto task = [weakThis = wptr(this), persistentId = persistentId_, eventId, param, env = env_] {
        auto jsSceneSession = weakThis.promote();
        if (!jsSceneSession || jsSceneSessionMap_.find(persistentId) == jsSceneSessionMap_.end()) {
            TLOGNE(WmsLogTag::WMS_LIFE, "OnSessionEvent jsSceneSession id:%{public}d has been destroyed",
                persistentId);
            return;
        }
        auto jsCallBack = jsSceneSession->GetJSCallback(SESSION_EVENT_CB);
        if (jsCallBack == nullptr) {
            TLOGNE(WmsLogTag::WMS_LIFE, "jsCallBack is nullptr");
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
    auto session = weakSession_.promote();
    if (session == nullptr) {
        TLOGE(WmsLogTag::WMS_LIFE, "session is nullptr, id:%{public}d", persistentId_);
        return;
    }
    const char* const where = __func__;
    session->SetBackPressedListenser([weakThis = wptr(this), where](bool needMoveToBackground) {
        auto jsSceneSession = weakThis.promote();
        if (!jsSceneSession) {
            TLOGNE(WmsLogTag::WMS_LIFE, "%{public}s jsSceneSession is null", where);
            return;
        }
        jsSceneSession->OnBackPressed(needMoveToBackground);
    });
}

void JsSceneSession::ProcessSystemBarPropertyChangeRegister()
{
    auto session = weakSession_.promote();
    if (session == nullptr) {
        TLOGE(WmsLogTag::WMS_IMMS, "session is null, win %{public}d", persistentId_);
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
    auto session = weakSession_.promote();
    if (session == nullptr) {
        TLOGE(WmsLogTag::WMS_IMMS, "session is null, win %{public}d", persistentId_);
        return;
    }
    session->RegisterNeedAvoidCallback([weakThis = wptr(this)](bool status) {
        auto jsSceneSession = weakThis.promote();
        if (!jsSceneSession) {
            TLOGNE(WmsLogTag::WMS_IMMS, "jsSceneSession is null");
            return;
        }
        jsSceneSession->OnNeedAvoid(status);
    });
}

void JsSceneSession::ProcessIsCustomAnimationPlaying()
{
    auto session = weakSession_.promote();
    if (session == nullptr) {
        TLOGE(WmsLogTag::WMS_LIFE, "session is nullptr");
        return;
    }
    session->RegisterIsCustomAnimationPlayingCallback([weakThis = wptr(this)](bool status) {
        auto jsSceneSession = weakThis.promote();
        if (!jsSceneSession) {
            TLOGNE(WmsLogTag::WMS_LIFE, "jsSceneSession is null");
            return;
        }
        jsSceneSession->OnIsCustomAnimationPlaying(status);
    });
}

void JsSceneSession::ProcessShowWhenLockedRegister()
{
    auto session = weakSession_.promote();
    if (session == nullptr) {
        TLOGE(WmsLogTag::WMS_LIFE, "session is nullptr, id:%{public}d", persistentId_);
        return;
    }
    const char* const where = __func__;
    session->RegisterShowWhenLockedCallback([weakThis = wptr(this), where](bool showWhenLocked) {
        auto jsSceneSession = weakThis.promote();
        if (!jsSceneSession) {
            TLOGNE(WmsLogTag::WMS_LIFE, "%{public}s jsSceneSession is null", where);
            return;
        }
        jsSceneSession->OnShowWhenLocked(showWhenLocked);
    });
    TLOGD(WmsLogTag::WMS_LIFE, "success");
}

void JsSceneSession::ProcessRequestedOrientationChange()
{
    auto session = weakSession_.promote();
    if (session == nullptr) {
        TLOGE(WmsLogTag::WMS_LIFE, "session is nullptr");
        return;
    }
    session->RegisterRequestedOrientationChangeCallback(
        [weakThis = wptr(this)](uint32_t orientation, bool needAnimation) {
            auto jsSceneSession = weakThis.promote();
            if (!jsSceneSession) {
                TLOGNE(WmsLogTag::WMS_LIFE, "jsSceneSession is null");
                return;
            }
            jsSceneSession->OnReuqestedOrientationChange(orientation, needAnimation);
        });
}

void JsSceneSession::ProcessGetTargetOrientationConfigInfoRegister()
{
    auto session = weakSession_.promote();
    if (session == nullptr) {
        TLOGE(WmsLogTag::WMS_ROTATION, "session is nullptr");
        return;
    }
    const char* const where = __func__;
    session->SetSessionGetTargetOrientationConfigInfoCallback(
        [where, weakThis = wptr(this)](uint32_t targetOrientation) {
        auto jsSceneSession = weakThis.promote();
        if (!jsSceneSession) {
            TLOGNE(WmsLogTag::WMS_ROTATION, "jsSceneSession is null");
            return;
        }
        jsSceneSession->OnGetTargetOrientationConfigInfo(targetOrientation);
    });
}

void JsSceneSession::ProcessForceHideChangeRegister()
{
    auto session = weakSession_.promote();
    if (session == nullptr) {
        TLOGE(WmsLogTag::WMS_LIFE, "session is nullptr, id:%{public}d", persistentId_);
        return;
    }
    const char* const where = __func__;
    session->RegisterForceHideChangeCallback([weakThis = wptr(this), where](bool hide) {
        auto jsSceneSession = weakThis.promote();
        if (!jsSceneSession) {
            TLOGNE(WmsLogTag::WMS_LIFE, "%{public}s jsSceneSession is null", where);
            return;
        }
        jsSceneSession->OnForceHideChange(hide);
    });
    TLOGD(WmsLogTag::WMS_LIFE, "success");
}

void JsSceneSession::OnForceHideChange(bool hide)
{
    TLOGI(WmsLogTag::DEFAULT, "%{public}u", hide);

    auto task = [weakThis = wptr(this), persistentId = persistentId_, hide, env = env_] {
        auto jsSceneSession = weakThis.promote();
        if (!jsSceneSession || jsSceneSessionMap_.find(persistentId) == jsSceneSessionMap_.end()) {
            TLOGNE(WmsLogTag::WMS_LIFE, "OnForceHideChange jsSceneSession id:%{public}d has been destroyed",
                persistentId);
            return;
        }
        auto jsCallBack = jsSceneSession->GetJSCallback(FORCE_HIDE_CHANGE_CB);
        if (!jsCallBack) {
            TLOGNE(WmsLogTag::WMS_LIFE, "jsCallBack is nullptr");
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
    auto session = weakSession_.promote();
    if (session == nullptr) {
        TLOGE(WmsLogTag::WMS_LIFE, "session is nullptr");
        return;
    }
    session->RegisterTouchOutsideCallback([weakThis = wptr(this)] {
        auto jsSceneSession = weakThis.promote();
        if (!jsSceneSession) {
            TLOGNE(WmsLogTag::WMS_LIFE, "jsSceneSession is null");
            return;
        }
        jsSceneSession->OnTouchOutside();
    });
}

void JsSceneSession::OnTouchOutside()
{
    TLOGD(WmsLogTag::DEFAULT, "[NAPI]");
    auto task = [weakThis = wptr(this), persistentId = persistentId_, env = env_] {
        auto jsSceneSession = weakThis.promote();
        if (!jsSceneSession || jsSceneSessionMap_.find(persistentId) == jsSceneSessionMap_.end()) {
            TLOGNE(WmsLogTag::WMS_LIFE, "OnTouchOutside jsSceneSession id:%{public}d has been destroyed",
                persistentId);
            return;
        }
        auto jsCallBack = jsSceneSession->GetJSCallback(TOUCH_OUTSIDE_CB);
        if (!jsCallBack) {
            TLOGNE(WmsLogTag::WMS_LIFE, "jsCallBack is nullptr");
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
            TLOGNE(WmsLogTag::WMS_LIFE, "ProcessFrameLayoutFinishRegister jsSceneSession is null");
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
            TLOGNE(WmsLogTag::WMS_MULTI_WINDOW, "jsSceneSession id:%{public}d has been destroyed", persistentId);
            return;
        }
        auto jsCallBack = jsSceneSession->GetJSCallback(NEXT_FRAME_LAYOUT_FINISH_CB);
        if (!jsCallBack) {
            TLOGNE(WmsLogTag::WMS_MULTI_WINDOW, "jsCallBack is nullptr");
            return;
        }
        napi_value argv[] = {};
        napi_call_function(env, NapiGetUndefined(env), jsCallBack->GetNapiValue(), 0, argv, nullptr);
    };
    taskScheduler_->PostMainThreadTask(task, "NotifyFrameLayoutFinish");
}

void JsSceneSession::ProcessPrivacyModeChangeRegister()
{
    auto session = weakSession_.promote();
    if (session == nullptr) {
        TLOGE(WmsLogTag::WMS_SCB, "session is nullptr");
        return;
    }
    session->SetPrivacyModeChangeNotifyFunc([weakThis = wptr(this)](bool isPrivacyMode) {
        auto jsSceneSession = weakThis.promote();
        if (!jsSceneSession) {
            TLOGNE(WmsLogTag::WMS_SCB, "jsSceneSession is null");
            return;
        }
        jsSceneSession->NotifyPrivacyModeChange(isPrivacyMode);
    });
}

void JsSceneSession::NotifyPrivacyModeChange(bool isPrivacyMode)
{
    TLOGI(WmsLogTag::WMS_SCB, "isPrivacyMode:%{public}d, id:%{public}d", isPrivacyMode, persistentId_);
    auto task = [weakThis = wptr(this), isPrivacyMode, env = env_]() {
        auto jsSceneSession = weakThis.promote();
        if (!jsSceneSession) {
            TLOGNE(WmsLogTag::WMS_SCB, "jsSceneSession is null");
            return;
        }
        auto jsCallback = jsSceneSession->GetJSCallback(PRIVACY_MODE_CHANGE_CB);
        if (!jsCallback) {
            TLOGNE(WmsLogTag::WMS_SCB, "jsCallback is nullptr");
            return;
        }
        napi_value jsIsPrivacyModeValue = CreateJsValue(env, isPrivacyMode);
        napi_value argv[] = { jsIsPrivacyModeValue };
        napi_call_function(env, NapiGetUndefined(env), jsCallback->GetNapiValue(), ArraySize(argv), argv, nullptr);
    };
    taskScheduler_->PostMainThreadTask(task, __func__);
}

void JsSceneSession::Finalizer(napi_env env, void* data, void* hint)
{
    TLOGD(WmsLogTag::DEFAULT, "Finalizer");
    auto jsSceneSession = static_cast<JsSceneSession*>(data);
    if (jsSceneSession == nullptr) {
        TLOGE(WmsLogTag::WMS_LIFE, "JsSceneSession is nullptr");
        return;
    }
    jsSceneSession->DecStrongRef(nullptr);
}

napi_value JsSceneSession::RegisterCallback(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::DEFAULT, "[NAPI]");
    JsSceneSession* me = CheckParamsAndGetThis<JsSceneSession>(env, info);
    return (me != nullptr) ? me->OnRegisterCallback(env, info) : nullptr;
}

napi_value JsSceneSession::UpdateNativeVisibility(napi_env env, napi_callback_info info)
{
    if (Session::IsScbCoreEnabled()) {
        return nullptr;
    }
    TLOGD(WmsLogTag::DEFAULT, "[NAPI]");
    JsSceneSession* me = CheckParamsAndGetThis<JsSceneSession>(env, info);
    return (me != nullptr) ? me->OnUpdateNativeVisibility(env, info) : nullptr;
}

napi_value JsSceneSession::SetPrivacyMode(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::DEFAULT, "[NAPI]");
    JsSceneSession* me = CheckParamsAndGetThis<JsSceneSession>(env, info);
    return (me != nullptr) ? me->OnSetPrivacyMode(env, info) : nullptr;
}

napi_value JsSceneSession::SetSkipSelfWhenShowOnVirtualScreen(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::DEFAULT, "[NAPI]");
    JsSceneSession* me = CheckParamsAndGetThis<JsSceneSession>(env, info);
    return (me != nullptr) ? me->OnSetSkipSelfWhenShowOnVirtualScreen(env, info) : nullptr;
}

napi_value JsSceneSession::SetSkipEventOnCastPlus(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::WMS_ATTRIBUTE, "[NAPI]");
    JsSceneSession* me = CheckParamsAndGetThis<JsSceneSession>(env, info);
    return (me != nullptr) ? me->OnSetSkipEventOnCastPlus(env, info) : nullptr;
}

napi_value JsSceneSession::SetSystemSceneOcclusionAlpha(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::DEFAULT, "[NAPI]");
    JsSceneSession* me = CheckParamsAndGetThis<JsSceneSession>(env, info);
    return (me != nullptr) ? me->OnSetSystemSceneOcclusionAlpha(env, info) : nullptr;
}

napi_value JsSceneSession::ResetOcclusionAlpha(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::WMS_ATTRIBUTE, "[NAPI]");
    JsSceneSession* me = CheckParamsAndGetThis<JsSceneSession>(env, info);
    return (me != nullptr) ? me->OnResetOcclusionAlpha(env, info) : nullptr;
}

napi_value JsSceneSession::SetSystemSceneForceUIFirst(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::DEFAULT, "[NAPI]");
    JsSceneSession* me = CheckParamsAndGetThis<JsSceneSession>(env, info);
    return (me != nullptr) ? me->OnSetSystemSceneForceUIFirst(env, info) : nullptr;
}

napi_value JsSceneSession::SetUIFirstSwitch(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::DEFAULT, "[NAPI]");
    JsSceneSession* me = CheckParamsAndGetThis<JsSceneSession>(env, info);
    return (me != nullptr) ? me->OnSetUIFirstSwitch(env, info) : nullptr;
}

napi_value JsSceneSession::MarkSystemSceneUIFirst(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::DEFAULT, "[NAPI]");
    JsSceneSession* me = CheckParamsAndGetThis<JsSceneSession>(env, info);
    return (me != nullptr) ? me->OnMarkSystemSceneUIFirst(env, info) : nullptr;
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

napi_value JsSceneSession::SetExclusivelyHighlighted(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::WMS_FOCUS, "[NAPI]");
    JsSceneSession* me = CheckParamsAndGetThis<JsSceneSession>(env, info);
    return (me != nullptr) ? me->OnSetExclusivelyHighlighted(env, info) : nullptr;
}

napi_value JsSceneSession::MaskSupportEnterWaterfallMode(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::WMS_LAYOUT_PC, "[NAPI]");
    JsSceneSession* me = CheckParamsAndGetThis<JsSceneSession>(env, info);
    return (me != nullptr) ? me->OnMaskSupportEnterWaterfallMode(env, info) : nullptr;
}

napi_value JsSceneSession::UpdateFullScreenWaterfallMode(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::WMS_LAYOUT_PC, "in");
    JsSceneSession* me = CheckParamsAndGetThis<JsSceneSession>(env, info);
    return (me != nullptr) ? me->OnUpdateFullScreenWaterfallMode(env, info) : nullptr;
}

napi_value JsSceneSession::UpdateSizeChangeReason(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::WMS_LAYOUT, "[NAPI]");
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

napi_value JsSceneSession::NotifyKeyboardAnimationCompleted(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::WMS_KEYBOARD, "[NAPI]");
    JsSceneSession* me = CheckParamsAndGetThis<JsSceneSession>(env, info);
    return (me != nullptr) ? me->OnNotifyKeyboardAnimationCompleted(env, info) : nullptr;
}

napi_value JsSceneSession::CallingWindowStateChange(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::WMS_KEYBOARD, "[NAPI]");
    JsSceneSession* me = CheckParamsAndGetThis<JsSceneSession>(env, info);
    return (me != nullptr) ? me->OnCallingWindowStateChange(env, info) : nullptr;
}

napi_value JsSceneSession::SetShowRecent(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::DEFAULT, "Enter");
    JsSceneSession* me = CheckParamsAndGetThis<JsSceneSession>(env, info);
    return (me != nullptr) ? me->OnSetShowRecent(env, info) : nullptr;
}

/** @note @window.hierarchy */
napi_value JsSceneSession::SetZOrder(napi_env env, napi_callback_info info)
{
    if (Session::IsScbCoreEnabled()) {
        return nullptr;
    }
    TLOGD(WmsLogTag::DEFAULT, "[NAPI]");
    JsSceneSession* me = CheckParamsAndGetThis<JsSceneSession>(env, info);
    return (me != nullptr) ? me->OnSetZOrder(env, info) : nullptr;
}

napi_value JsSceneSession::GetZOrder(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::WMS_PC, "[NAPI]");
    JsSceneSession* me = CheckParamsAndGetThis<JsSceneSession>(env, info);
    return (me != nullptr) ? me->OnGetZOrder(env, info) : nullptr;
}

napi_value JsSceneSession::GetUid(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::WMS_LAYOUT, "[NAPI]");
    JsSceneSession* me = CheckParamsAndGetThis<JsSceneSession>(env, info);
    return (me != nullptr) ? me->OnGetUid(env, info) : nullptr;
}

napi_value JsSceneSession::SetTouchable(napi_env env, napi_callback_info info)
{
    JsSceneSession* me = CheckParamsAndGetThis<JsSceneSession>(env, info);
    return (me != nullptr) ? me->OnSetTouchable(env, info): nullptr;
}

napi_value JsSceneSession::SetWindowInputType(napi_env env, napi_callback_info info)
{
    JsSceneSession* me = CheckParamsAndGetThis<JsSceneSession>(env, info);
    return (me != nullptr) ? me->OnSetWindowInputType(env, info): nullptr;
}

napi_value JsSceneSession::SetExpandInputFlag(napi_env env, napi_callback_info info)
{
    JsSceneSession* me = CheckParamsAndGetThis<JsSceneSession>(env, info);
    return (me != nullptr) ? me->OnSetExpandInputFlag(env, info): nullptr;
}

napi_value JsSceneSession::SetSystemActive(napi_env env, napi_callback_info info)
{
    JsSceneSession* me = CheckParamsAndGetThis<JsSceneSession>(env, info);
    return (me != nullptr) ? me->OnSetSystemActive(env, info): nullptr;
}

napi_value JsSceneSession::SetFloatingScale(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::DEFAULT, "[NAPI]");
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
    TLOGD(WmsLogTag::DEFAULT, "[NAPI]");
    JsSceneSession* me = CheckParamsAndGetThis<JsSceneSession>(env, info);
    return (me != nullptr) ? me->OnSetSCBKeepKeyboard(env, info) : nullptr;
}

napi_value JsSceneSession::SetOffset(napi_env env, napi_callback_info info)
{
    if (Session::IsScbCoreEnabled()) {
        return nullptr;
    }
    TLOGD(WmsLogTag::DEFAULT, "[NAPI]");
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

napi_value JsSceneSession::NotifyPipOcclusionChange(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::WMS_PIP, "[NAPI]");
    JsSceneSession* me = CheckParamsAndGetThis<JsSceneSession>(env, info);
    return (me != nullptr) ? me->OnNotifyPipOcclusionChange(env, info) : nullptr;
}

napi_value JsSceneSession::NotifyPipSizeChange(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::WMS_PIP, "[NAPI]");
    JsSceneSession* me = CheckParamsAndGetThis<JsSceneSession>(env, info);
    return (me != nullptr) ? me->OnNotifyPipSizeChange(env, info) : nullptr;
}

napi_value JsSceneSession::NotifyPipActiveWindowStatusChange(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::WMS_PIP, "[NAPI]");
    JsSceneSession* me = CheckParamsAndGetThis<JsSceneSession>(env, info);
    return (me != nullptr) ? me->OnNotifyPiPActiveStatusChange(env, info) : nullptr;
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

napi_value JsSceneSession::ToggleCompatibleMode(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::WMS_COMPAT, "[NAPI]");
    JsSceneSession* me = CheckParamsAndGetThis<JsSceneSession>(env, info);
    return (me != nullptr) ? me->OnToggleCompatibleMode(env, info) : nullptr;
}

napi_value JsSceneSession::SetAppSupportPhoneInPc(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::WMS_COMPAT, "[NAPI]");
    JsSceneSession* me = CheckParamsAndGetThis<JsSceneSession>(env, info);
    return (me != nullptr) ? me->OnSetAppSupportPhoneInPc(env, info) : nullptr;
}

napi_value JsSceneSession::SetUniqueDensityDpiFromSCB(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::WMS_SCB, "[NAPI]");
    JsSceneSession *me = CheckParamsAndGetThis<JsSceneSession>(env, info);
    return (me != nullptr) ? me->OnSetUniqueDensityDpiFromSCB(env, info) : nullptr;
}

napi_value JsSceneSession::SetBlank(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::WMS_SCB, "[NAPI]");
    JsSceneSession* me = CheckParamsAndGetThis<JsSceneSession>(env, info);
    return (me != nullptr) ? me->OnSetBlank(env, info) : nullptr;
}

napi_value JsSceneSession::RemoveBlank(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::WMS_SCB, "[NAPI]");
    JsSceneSession* me = CheckParamsAndGetThis<JsSceneSession>(env, info);
    return (me != nullptr) ? me->OnRemoveBlank(env, info) : nullptr;
}

/*
 * AddSnapshot and RemoveSnapshot must be in pair
 * be in good control of the time RemoveSnapshot execute, do not rely on first frame callback
 */
napi_value JsSceneSession::AddSnapshot(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::WMS_PATTERN, "[NAPI]");
    JsSceneSession* me = CheckParamsAndGetThis<JsSceneSession>(env, info);
    return (me != nullptr) ? me->OnAddSnapshot(env, info) : nullptr;
}

napi_value JsSceneSession::RemoveSnapshot(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::WMS_PATTERN, "[NAPI]");
    JsSceneSession* me = CheckParamsAndGetThis<JsSceneSession>(env, info);
    return (me != nullptr) ? me->OnRemoveSnapshot(env, info) : nullptr;
}

napi_value JsSceneSession::SetBufferAvailableCallbackEnable(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::WMS_SCB, "[NAPI]");
    JsSceneSession* me = CheckParamsAndGetThis<JsSceneSession>(env, info);
    return (me != nullptr) ? me->OnSetBufferAvailableCallbackEnable(env, info) : nullptr;
}

napi_value JsSceneSession::SyncDefaultRequestedOrientation(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::WMS_SCB, "[NAPI]");
    JsSceneSession* me = CheckParamsAndGetThis<JsSceneSession>(env, info);
    return (me != nullptr) ? me->OnSyncDefaultRequestedOrientation(env, info) : nullptr;
}

napi_value JsSceneSession::SetIsPcAppInPad(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::WMS_COMPAT, "[NAPI]");
    JsSceneSession* me = CheckParamsAndGetThis<JsSceneSession>(env, info);
    return (me != nullptr) ? me->OnSetIsPcAppInPad(env, info) : nullptr;
}

napi_value JsSceneSession::SetStartingWindowExitAnimationFlag(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::WMS_SCB, "[NAPI]");
    JsSceneSession *me = CheckParamsAndGetThis<JsSceneSession>(env, info);
    return (me != nullptr) ? me->OnSetStartingWindowExitAnimationFlag(env, info) : nullptr;
}

napi_value JsSceneSession::PcAppInPadNormalClose(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::WMS_COMPAT, "[NAPI]");
    JsSceneSession *me = CheckParamsAndGetThis<JsSceneSession>(env, info);
    return (me != nullptr) ? me->OnPcAppInPadNormalClose(env, info) : nullptr;
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

napi_value JsSceneSession::SetWindowEnableDragBySystem(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::WMS_SCB, "[NAPI]");
    JsSceneSession* me = CheckParamsAndGetThis<JsSceneSession>(env, info);
    return (me != nullptr) ? me->OnSetWindowEnableDragBySystem(env, info) : nullptr;
}

napi_value JsSceneSession::ActivateDragBySystem(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::WMS_LAYOUT, "[NAPI]");
    JsSceneSession* me = CheckParamsAndGetThis<JsSceneSession>(env, info);
    return (me != nullptr) ? me->OnActivateDragBySystem(env, info) : nullptr;
}

napi_value JsSceneSession::SetNeedSyncSessionRect(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::WMS_PIPELINE, "[NAPI]");
    JsSceneSession* me = CheckParamsAndGetThis<JsSceneSession>(env, info);
    return (me != nullptr) ? me->OnSetNeedSyncSessionRect(env, info) : nullptr;
}

napi_value JsSceneSession::SetIsPendingToBackgroundState(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::WMS_SCB, "[NAPI]");
    JsSceneSession* me = CheckParamsAndGetThis<JsSceneSession>(env, info);
    return (me != nullptr) ? me->OnSetIsPendingToBackgroundState(env, info) : nullptr;
}

napi_value JsSceneSession::SetLabel(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::WMS_LAYOUT_PC, "[NAPI]");
    JsSceneSession* me = CheckParamsAndGetThis<JsSceneSession>(env, info);
    return (me != nullptr) ? me->OnSetLabel(env, info) : nullptr;
}

napi_value JsSceneSession::SetIsActivatedAfterScreenLocked(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::WMS_SCB, "[NAPI]");
    JsSceneSession* me = CheckParamsAndGetThis<JsSceneSession>(env, info);
    return (me != nullptr) ? me->OnSetIsActivatedAfterScreenLocked(env, info) : nullptr;
}

napi_value JsSceneSession::SetFrameGravity(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::WMS_LAYOUT_PC, "[NAPI]");
    JsSceneSession* me = CheckParamsAndGetThis<JsSceneSession>(env, info);
    return (me != nullptr) ? me->OnSetFrameGravity(env, info) : nullptr;
}

napi_value JsSceneSession::SetUseStartingWindowAboveLocked(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::WMS_SCB, "[NAPI]");
    JsSceneSession* me = CheckParamsAndGetThis<JsSceneSession>(env, info);
    return (me != nullptr) ? me->OnSetUseStartingWindowAboveLocked(env, info) : nullptr;
}

napi_value JsSceneSession::SaveSnapshotSync(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::WMS_SCB, "[NAPI]");
    JsSceneSession* me = CheckParamsAndGetThis<JsSceneSession>(env, info);
    return (me != nullptr) ? me->OnSaveSnapshotSync(env, info) : nullptr;
}

napi_value JsSceneSession::SaveSnapshotAsync(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::WMS_SCB, "[NAPI]");
    JsSceneSession* me = CheckParamsAndGetThis<JsSceneSession>(env, info);
    return (me != nullptr) ? me->OnSaveSnapshotAsync(env, info) : nullptr;
}

napi_value JsSceneSession::SetBorderUnoccupied(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::WMS_SCB, "[NAPI]");
    JsSceneSession* me = CheckParamsAndGetThis<JsSceneSession>(env, info);
    return (me != nullptr) ? me->OnSetBorderUnoccupied(env, info) : nullptr;
}

napi_value JsSceneSession::SetEnableAddSnapshot(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::WMS_SCB, "[NAPI]");
    JsSceneSession* me = CheckParamsAndGetThis<JsSceneSession>(env, info);
    return (me != nullptr) ? me->OnSetEnableAddSnapshot(env, info) : nullptr;
}

napi_value JsSceneSession::SetFreezeImmediately(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::DEFAULT, "in");
    JsSceneSession* me = CheckParamsAndGetThis<JsSceneSession>(env, info);
    return (me != nullptr) ? me->OnSetFreezeImmediately(env, info) : nullptr;
}

napi_value JsSceneSession::ThrowSlipDirectly(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::WMS_LAYOUT_PC, "in");
    JsSceneSession* me = CheckParamsAndGetThis<JsSceneSession>(env, info);
    return (me != nullptr) ? me->OnThrowSlipDirectly(env, info) : nullptr;
}

napi_value JsSceneSession::SendContainerModalEvent(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::WMS_EVENT, "[NAPI]");
    JsSceneSession* me = CheckParamsAndGetThis<JsSceneSession>(env, info);
    return (me != nullptr) ? me->OnSendContainerModalEvent(env, info) : nullptr;
}

napi_value JsSceneSession::SetColorSpace(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::WMS_ATTRIBUTE, "[NAPI]");
    JsSceneSession* me = CheckParamsAndGetThis<JsSceneSession>(env, info);
    return (me != nullptr) ? me->OnSetColorSpace(env, info) : nullptr;
}

napi_value JsSceneSession::NotifyRotationProperty(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::WMS_ROTATION, "[NAPI]");
    JsSceneSession* me = CheckParamsAndGetThis<JsSceneSession>(env, info);
    return (me != nullptr) ? me->OnNotifyRotationProperty(env, info) : nullptr;
}

napi_value JsSceneSession::NotifyPageRotationIsIgnored(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::WMS_ROTATION, "[NAPI]");
    JsSceneSession* me = CheckParamsAndGetThis<JsSceneSession>(env, info);
    return (me != nullptr) ? me->OnNotifyPageRotationIsIgnored(env, info) : nullptr;
}

napi_value JsSceneSession::SetSnapshotSkip(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::WMS_ATTRIBUTE, "[NAPI]");
    JsSceneSession* me = CheckParamsAndGetThis<JsSceneSession>(env, info);
    return (me != nullptr) ? me->OnSetSnapshotSkip(env, info) : nullptr;
}

napi_value JsSceneSession::AddSidebarBlur(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::WMS_PC, "[NAPI]");
    JsSceneSession* me = CheckParamsAndGetThis<JsSceneSession>(env, info);
    return (me != nullptr) ? me->OnAddSidebarBlur(env, info) : nullptr;
}

napi_value JsSceneSession::SetSidebarBlur(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::WMS_PC, "[NAPI]");
    JsSceneSession* me = CheckParamsAndGetThis<JsSceneSession>(env, info);
    return (me != nullptr) ? me->OnSetSidebarBlur(env, info) : nullptr;
}

napi_value JsSceneSession::SetCurrentRotation(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::WMS_ROTATION, "[NAPI]");
    JsSceneSession* me = CheckParamsAndGetThis<JsSceneSession>(env, info);
    return (me != nullptr) ? me->OnSetCurrentRotation(env, info) : nullptr;
}

napi_value JsSceneSession::SetSidebarBlurMaximize(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::WMS_PC, "[NAPI]");
    JsSceneSession* me = CheckParamsAndGetThis<JsSceneSession>(env, info);
    return (me != nullptr) ? me->OnSetSidebarBlurMaximize(env, info) : nullptr;
}

napi_value JsSceneSession::SendFbActionEvent(napi_env env, napi_callback_info info)
{
    TLOGI(WmsLogTag::WMS_SYSTEM, "[NAPI]");
    JsSceneSession* me = CheckParamsAndGetThis<JsSceneSession>(env, info);
    return (me != nullptr) ? me->OnSendFbActionEvent(env, info) : nullptr;
}

napi_value JsSceneSession::SetPcAppInpadCompatibleMode(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::WMS_PC, "[NAPI]");
    JsSceneSession* me = CheckParamsAndGetThis<JsSceneSession>(env, info);
    return (me != nullptr) ? me->OnSetPcAppInpadCompatibleMode(env, info) : nullptr;
}

napi_value JsSceneSession::SetMobileAppInPadLayoutFullScreen(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::WMS_PC, "[NAPI]");
    JsSceneSession* me = CheckParamsAndGetThis<JsSceneSession>(env, info);
    return (me != nullptr) ? me->OnSetMobileAppInPadLayoutFullScreen(env, info) : nullptr;
}

napi_value JsSceneSession::SetPcAppInpadSpecificSystemBarInvisible(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::WMS_PC, "[NAPI]");
    JsSceneSession* me = CheckParamsAndGetThis<JsSceneSession>(env, info);
    return (me != nullptr) ? me->OnSetPcAppInpadSpecificSystemBarInvisible(env, info) : nullptr;
}

napi_value JsSceneSession::SetPcAppInpadOrientationLandscape(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::WMS_PC, "[NAPI]");
    JsSceneSession* me = CheckParamsAndGetThis<JsSceneSession>(env, info);
    return (me != nullptr) ? me->OnSetPcAppInpadOrientationLandscape(env, info) : nullptr;
}

napi_value JsSceneSession::UpdateSceneAnimationConfig(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::WMS_PC, "[NAPI]");
    JsSceneSession* me = CheckParamsAndGetThis<JsSceneSession>(env, info);
    return (me != nullptr) ? me->OnUpdateSceneAnimationConfig(env, info) : nullptr;
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
            WLOGFE("%{public}s has already been registered", type.c_str());
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
        WLOGFE("Argc is invalid: %{public}zu", argc);
        return NapiGetUndefined(env);
    }
    std::string cbType;
    if (!ConvertFromJsValue(env, argv[0], cbType)) {
        WLOGFE("Failed to convert parameter to callbackType");
        return NapiGetUndefined(env);
    }
    napi_value value = argv[1];
    if (value == nullptr || !NapiIsCallable(env, value)) {
        WLOGFE("Invalid argument");
        return NapiGetUndefined(env);
    }
    auto iterFunctionType = ListenerFuncMap.find(cbType);
    if (iterFunctionType == ListenerFuncMap.end()) {
        WLOGFE("callback type is not supported, type=%{public}s", cbType.c_str());
        return NapiGetUndefined(env);
    }
    ListenerFuncType listenerFuncType = iterFunctionType->second;
    if (IsCallbackRegistered(env, cbType, value)) {
        WLOGFE("callback is registered, type=%{public}s", cbType.c_str());
        return NapiGetUndefined(env);
    }
    auto session = weakSession_.promote();
    if (session == nullptr) {
        WLOGFE("session is nullptr, id:%{public}d", persistentId_);
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
    TLOGD(WmsLogTag::DEFAULT, "end, type=%{public}s", cbType.c_str());
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
        case static_cast<uint32_t>(ListenerFuncType::RESTORE_FLOAT_MAIN_WINDOW_CB):
            ProcessRestoreFloatMainWindowRegister();
            break;
        case static_cast<uint32_t>(ListenerFuncType::CHANGE_SESSION_VISIBILITY_WITH_STATUS_BAR):
            ProcessChangeSessionVisibilityWithStatusBarRegister();
            break;
        case static_cast<uint32_t>(ListenerFuncType::SESSION_STATE_CHANGE_CB):
            ProcessSessionStateChangeRegister();
            break;
        case static_cast<uint32_t>(ListenerFuncType::UPDATE_TRANSITION_ANIMATION_CB):
            ProcessUpdateTransitionAnimationRegister();
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
        case static_cast<uint32_t>(ListenerFuncType::SESSION_WINDOW_LIMITS_CHANGE_CB):
            ProcessSessionWindowLimitsChangeRegister();
            break;
        case static_cast<uint32_t>(ListenerFuncType::WINDOW_MOVING_CB):
            ProcessWindowMovingRegister();
            break;
        case static_cast<uint32_t>(ListenerFuncType::SESSION_DISPLAY_ID_CHANGE_CB):
            ProcessSessionDisplayIdChangeRegister();
            break;
        case static_cast<uint32_t>(ListenerFuncType::SESSION_PIP_CONTROL_STATUS_CHANGE_CB):
            ProcessSessionPiPControlStatusChangeRegister();
            break;
        case static_cast<uint32_t>(ListenerFuncType::SESSION_AUTO_START_PIP_CB):
            ProcessAutoStartPiPStatusChangeRegister();
            break;
        case static_cast<uint32_t>(ListenerFuncType::UPDATE_PIP_TEMPLATE_INFO_CB):
            ProcessUpdatePiPTemplateInfoRegister();
            break;
        case static_cast<uint32_t>(ListenerFuncType::SET_PIP_PARENT_WINDOWID_CB):
            ProcessSetPiPParentWindowIdRegister();
            break;
        case static_cast<uint32_t>(ListenerFuncType::CREATE_SUB_SESSION_CB):
            ProcessCreateSubSessionRegister();
            break;
        case static_cast<uint32_t>(ListenerFuncType::CLEAR_SUB_SESSION_CB):
            ProcessClearSubSessionRegister();
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
        case static_cast<uint32_t>(ListenerFuncType::CLICK_MODAL_WINDOW_OUTSIDE_CB):
            ProcessClickModalWindowOutsideRegister();
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
        case static_cast<uint32_t>(ListenerFuncType::MAIN_WINDOW_TOP_MOST_CHANGE_CB):
            ProcessMainWindowTopmostChangeRegister();
            break;
        case static_cast<uint32_t>(ListenerFuncType::Z_LEVEL_CHANGE_CB):
            ProcessSubWindowZLevelChangeRegister();
            break;
        case static_cast<uint32_t>(ListenerFuncType::SUB_MODAL_TYPE_CHANGE_CB):
            ProcessSubModalTypeChangeRegister();
            break;
        case static_cast<uint32_t>(ListenerFuncType::MAIN_MODAL_TYPE_CHANGE_CB):
            ProcessMainModalTypeChangeRegister();
            break;
        case static_cast<uint32_t>(ListenerFuncType::THROW_SLIP_ANIMATION_STATE_CHANGE_CB):
            RegisterThrowSlipAnimationStateChangeCallback();
            break;
        case static_cast<uint32_t>(ListenerFuncType::FULLSCREEN_WATERFALL_MODE_CHANGE_CB):
            RegisterFullScreenWaterfallModeChangeCallback();
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
        case static_cast<uint32_t>(ListenerFuncType::PENDING_SESSION_TO_BACKGROUND_CB):
            ProcessPendingSessionToBackgroundRegister();
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
        case static_cast<uint32_t>(ListenerFuncType::RAISE_MAIN_WINDOW_ABOVE_TARGET_CB):
            ProcessRaiseMainWindowAboveTargetRegister();
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
        case static_cast<uint32_t>(ListenerFuncType::ADJUST_KEYBOARD_LAYOUT_CB):
            ProcessAdjustKeyboardLayoutRegister();
            break;
        case static_cast<uint32_t>(ListenerFuncType::LAYOUT_FULL_SCREEN_CB):
            ProcessLayoutFullScreenChangeRegister();
            break;
        case static_cast<uint32_t>(ListenerFuncType::DEFAULT_DENSITY_ENABLED_CB):
            ProcessDefaultDensityEnabledRegister();
            break;
        case static_cast<uint32_t>(ListenerFuncType::WINDOW_SHADOW_ENABLE_CHANGE_CB):
            ProcessWindowShadowEnableChangeRegister();
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
        case static_cast<uint32_t>(ListenerFuncType::SET_WINDOW_RECT_AUTO_SAVE_CB):
            ProcessSetWindowRectAutoSaveRegister();
            break;
        case static_cast<uint32_t>(ListenerFuncType::UPDATE_APP_USE_CONTROL_CB):
            RegisterUpdateAppUseControlCallback();
            break;
        case static_cast<uint32_t>(ListenerFuncType::SET_SUPPORT_WINDOW_MODES_CB):
            ProcessSetSupportedWindowModesRegister();
            break;
        case static_cast<uint32_t>(ListenerFuncType::SESSION_LOCK_STATE_CHANGE_CB):
            ProcessSessionLockStateChangeRegister();
            break;
        case static_cast<uint32_t>(ListenerFuncType::UPDATE_SESSION_LABEL_AND_ICON_CB):
            ProcessUpdateSessionLabelAndIconRegister();
            break;
        case static_cast<uint32_t>(ListenerFuncType::SESSION_GET_TARGET_ORIENTATION_CONFIG_INFO_CB):
            ProcessGetTargetOrientationConfigInfoRegister();
            break;
        case static_cast<uint32_t>(ListenerFuncType::KEYBOARD_STATE_CHANGE_CB):
            ProcessKeyboardStateChangeRegister();
            break;
        case static_cast<uint32_t>(ListenerFuncType::KEYBOARD_EFFECT_OPTION_CHANGE_CB):
            ProcessKeyboardEffectOptionChangeRegister();
            break;
        case static_cast<uint32_t>(ListenerFuncType::HIGHLIGHT_CHANGE_CB):
            ProcessSetHighlightChangeRegister();
            break;
        case static_cast<uint32_t>(ListenerFuncType::WINDOW_ANCHOR_INFO_CHANGE_CB):
            ProcessWindowAnchorInfoChangeRegister();
            break;
        case static_cast<uint32_t>(ListenerFuncType::SET_WINDOW_CORNER_RADIUS_CB):
            ProcessSetWindowCornerRadiusRegister();
            break;
        case static_cast<uint32_t>(ListenerFuncType::FOLLOW_PARENT_RECT_CB):
            ProcessFollowParentRectRegister();
            break;
        case static_cast<uint32_t>(ListenerFuncType::SET_PARENT_SESSION_CB):
            ProcessSetParentSessionRegister();
            break;
        case static_cast<uint32_t>(ListenerFuncType::UPDATE_FLAG_CB):
            ProcessUpdateFlagRegister();
            break;
        case static_cast<uint32_t>(ListenerFuncType::UPDATE_FOLLOW_SCREEN_CHANGE_CB):
            ProcessSessionUpdateFollowScreenChange();
            break;
        case static_cast<uint32_t>(ListenerFuncType::USE_IMPLICIT_ANIMATION_CB):
            ProcessUseImplicitAnimationChangeRegister();
            break;
        case static_cast<uint32_t>(ListenerFuncType::SET_WINDOW_SHADOWS_CB):
            ProcessSetWindowShadowsRegister();
            break;
        case static_cast<uint32_t>(ListenerFuncType::SET_SUB_WINDOW_SOURCE_CB):
            ProcessSetSubWindowSourceRegister();
            break;
        case static_cast<uint32_t>(ListenerFuncType::ANIMATE_TO_CB):
            ProcessAnimateToTargetPropertyRegister();
            break;
        case static_cast<uint32_t>(ListenerFuncType::BATCH_PENDING_SCENE_ACTIVE_CB):
            ProcessBatchPendingSceneSessionsActivationRegister();
            break;
        case static_cast<uint32_t>(ListenerFuncType::FLOATING_BALL_UPDATE_CB):
            ProcessFloatingBallUpdateRegister();
            break;
        case static_cast<uint32_t>(ListenerFuncType::FLOATING_BALL_STOP_CB):
            ProcessFloatingBallStopRegister();
            break;
        case static_cast<uint32_t>(ListenerFuncType::FLOATING_BALL_RESTORE_MAIN_WINDOW_CB):
            ProcessFloatingBallRestoreMainWindowRegister();
            break;
        case static_cast<uint32_t>(ListenerFuncType::SCENE_OUTLINE_PARAMS_CHANGE_CB):
            ProcessSceneOutlineParamsChangeRegister();
            break;
        case static_cast<uint32_t>(ListenerFuncType::RESTART_APP_CB):
            ProcessRestartAppRegister();
            break;
        case static_cast<uint32_t>(ListenerFuncType::CALLING_SESSION_ID_CHANGE_CB):
            ProcessCallingSessionIdChangeRegister();
            break;
        case static_cast<uint32_t>(ListenerFuncType::ROTATION_LOCK_CHANGE_CB):
            ProcessRotationLockChangeRegister();
            break;
        case static_cast<uint32_t>(ListenerFuncType::SNAPSHOT_SKIP_CHANGE_CB):
            ProcessSnapshotSkipChangeRegister();
            break;
        case static_cast<uint32_t>(ListenerFuncType::COMPATIBLE_MODE_CHANGE_CB):
            ProcessCompatibleModeChangeRegister();
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
        WLOGFE("Argc is invalid: %{public}zu", argc);
        return NapiGetUndefined(env);
    }
    bool visible = false;
    if (!ConvertFromJsValue(env, argv[0], visible)) {
        WLOGFE("Failed to convert parameter to bool");
        return NapiGetUndefined(env);
    }
    auto session = weakSession_.promote();
    if (session == nullptr) {
        WLOGFE("session is nullptr, id:%{public}d", persistentId_);
        return NapiGetUndefined(env);
    }
    session->UpdateNativeVisibility(visible);
    TLOGD(WmsLogTag::DEFAULT, "end");
    return NapiGetUndefined(env);
}

napi_value JsSceneSession::OnSetPrivacyMode(napi_env env, napi_callback_info info)
{
    size_t argc = 4;
    napi_value argv[4] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc < 1) { // 1: params num
        WLOGFE("Argc is invalid: %{public}zu", argc);
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    bool isPrivacy = false;
    if (!ConvertFromJsValue(env, argv[0], isPrivacy)) {
        WLOGFE("Failed to convert parameter to bool");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    auto session = weakSession_.promote();
    if (session == nullptr) {
        WLOGFE("session is nullptr, id:%{public}d", persistentId_);
        return NapiGetUndefined(env);
    }
    session->SetPrivacyMode(isPrivacy);
    SceneSessionManager::GetInstance().UpdatePrivateStateAndNotify(session->GetPersistentId());
    TLOGD(WmsLogTag::DEFAULT, "end");
    return NapiGetUndefined(env);
}

napi_value JsSceneSession::OnSetSystemSceneOcclusionAlpha(napi_env env, napi_callback_info info)
{
    size_t argc = 4;
    napi_value argv[4] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc < 1) { // 1: params num
        WLOGFE("Argc is invalid: %{public}zu", argc);
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    double alpha = 0.f;
    if (!ConvertFromJsValue(env, argv[0], alpha)) {
        WLOGFE("Failed to convert parameter to bool");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    auto session = weakSession_.promote();
    if (session == nullptr) {
        WLOGFE("session is nullptr, id:%{public}d", persistentId_);
        return NapiGetUndefined(env);
    }
    session->SetSystemSceneOcclusionAlpha(alpha);
    TLOGD(WmsLogTag::DEFAULT, "end");
    return NapiGetUndefined(env);
}

napi_value JsSceneSession::OnResetOcclusionAlpha(napi_env env, napi_callback_info info)
{
    auto session = weakSession_.promote();
    if (session == nullptr) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "session is nullptr, id:%{public}d", persistentId_);
        return NapiGetUndefined(env);
    }
    session->ResetOcclusionAlpha();
    TLOGD(WmsLogTag::WMS_ATTRIBUTE, "end");
    return NapiGetUndefined(env);
}

napi_value JsSceneSession::OnSetSystemSceneForceUIFirst(napi_env env, napi_callback_info info)
{
    size_t argc = ARG_COUNT_4;
    napi_value argv[ARG_COUNT_4] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc < 1) { // 1: params num
        TLOGE(WmsLogTag::DEFAULT, "Argc is invalid: %{public}zu", argc);
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    bool forceUIFirst = false;
    if (!ConvertFromJsValue(env, argv[0], forceUIFirst)) {
        TLOGE(WmsLogTag::DEFAULT, "Failed to convert parameter to forceUIFirst");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    auto session = weakSession_.promote();
    if (session == nullptr) {
        TLOGE(WmsLogTag::DEFAULT, "session is nullptr, id:%{public}d", persistentId_);
        return NapiGetUndefined(env);
    }
    session->SetSystemSceneForceUIFirst(forceUIFirst);
    TLOGD(WmsLogTag::DEFAULT, "end");
    return NapiGetUndefined(env);
}

napi_value JsSceneSession::OnSetUIFirstSwitch(napi_env env, napi_callback_info info)
{
    size_t argc = ARG_COUNT_4;
    napi_value argv[ARG_COUNT_4] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc < ARG_COUNT_1) {
        TLOGE(WmsLogTag::DEFAULT, "Argc is invalid: %{public}zu", argc);
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }

    RSUIFirstSwitch uiFirstSwitch;
    if (!ConvertFromJsValue(env, argv[ARG_INDEX_0], uiFirstSwitch)) {
        TLOGE(WmsLogTag::DEFAULT, "Failed to convert parameter to uiFirstSwitch");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    auto session = weakSession_.promote();
    if (session == nullptr) {
        TLOGE(WmsLogTag::DEFAULT, "session is nullptr, id:%{public}d", persistentId_);
        return NapiGetUndefined(env);
    }
    session->SetUIFirstSwitch(uiFirstSwitch);
    return NapiGetUndefined(env);
}

napi_value JsSceneSession::OnMarkSystemSceneUIFirst(napi_env env, napi_callback_info info)
{
    size_t argc = ARG_COUNT_4;
    napi_value argv[ARG_COUNT_4] = { nullptr };
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc < ARGC_TWO) {
        TLOGE(WmsLogTag::DEFAULT, "Argc is invalid: %{public}zu", argc);
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    bool isForced = false;
    if (!ConvertFromJsValue(env, argv[ARG_INDEX_0], isForced)) {
        TLOGE(WmsLogTag::DEFAULT, "Failed to convert parameter to isForced");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    bool isUIFirstEnabled = false;
    if (!ConvertFromJsValue(env, argv[ARG_INDEX_1], isUIFirstEnabled)) {
        TLOGE(WmsLogTag::DEFAULT, "Failed to convert parameter to isUIFirstEnabled");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    auto session = weakSession_.promote();
    if (session == nullptr) {
        TLOGE(WmsLogTag::DEFAULT, "session is nullptr, id:%{public}d", persistentId_);
        return NapiGetUndefined(env);
    }
    session->MarkSystemSceneUIFirst(isForced, isUIFirstEnabled);
    return NapiGetUndefined(env);
}

napi_value JsSceneSession::OnSetFocusable(napi_env env, napi_callback_info info)
{
    size_t argc = 4;
    napi_value argv[4] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc < 1) { // 1: params num
        WLOGFE("Argc is invalid: %{public}zu", argc);
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    bool isFocusable = false;
    if (!ConvertFromJsValue(env, argv[0], isFocusable)) {
        WLOGFE("Failed to convert parameter to bool");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    auto session = weakSession_.promote();
    if (session == nullptr) {
        WLOGFE("session is nullptr, id:%{public}d", persistentId_);
        return NapiGetUndefined(env);
    }
    session->SetFocusable(isFocusable);
    TLOGD(WmsLogTag::WMS_FOCUS, "end");
    return NapiGetUndefined(env);
}

napi_value JsSceneSession::OnSetFocusableOnShow(napi_env env, napi_callback_info info)
{
    size_t argc = ARG_COUNT_4;
    napi_value argv[ARG_COUNT_4] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc < 1) { // 1: params num
        TLOGE(WmsLogTag::WMS_FOCUS, "Argc is invalid: %{public}zu", argc);
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    bool isFocusableOnShow = true;
    if (!ConvertFromJsValue(env, argv[0], isFocusableOnShow)) {
        TLOGE(WmsLogTag::WMS_FOCUS, "Failed to convert parameter to bool");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    auto session = weakSession_.promote();
    if (session == nullptr) {
        TLOGE(WmsLogTag::WMS_FOCUS, "session is nullptr, id:%{public}d", persistentId_);
        return NapiGetUndefined(env);
    }
    session->SetFocusableOnShow(isFocusableOnShow);
    TLOGD(WmsLogTag::WMS_FOCUS, "end");
    return NapiGetUndefined(env);
}

napi_value JsSceneSession::OnSetSystemFocusable(napi_env env, napi_callback_info info)
{
    size_t argc = ARG_COUNT_4;
    napi_value argv[ARG_COUNT_4] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc < 1) { // 1: params num
        TLOGE(WmsLogTag::WMS_FOCUS, "Argc is invalid: %{public}zu", argc);
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    bool systemFocusable = false;
    if (!ConvertFromJsValue(env, argv[0], systemFocusable)) {
        TLOGE(WmsLogTag::WMS_FOCUS, "Failed to convert parameter to bool");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    auto session = weakSession_.promote();
    if (session == nullptr) {
        TLOGE(WmsLogTag::WMS_FOCUS, "session is nullptr, id:%{public}d", persistentId_);
        return NapiGetUndefined(env);
    }
    session->SetSystemFocusable(systemFocusable);
    TLOGD(WmsLogTag::WMS_FOCUS, "end");
    return NapiGetUndefined(env);
}

napi_value JsSceneSession::OnSetSystemSceneBlockingFocus(napi_env env, napi_callback_info info)
{
    size_t argc = 4;
    napi_value argv[4] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc < 1) { // 1: params num
        WLOGFE("Argc is invalid: %{public}zu", argc);
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    bool blocking = false;
    if (!ConvertFromJsValue(env, argv[0], blocking)) {
        WLOGFE("Failed to convert parameter to bool");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    auto session = weakSession_.promote();
    if (session == nullptr) {
        WLOGFE("session is nullptr, id:%{public}d", persistentId_);
        return NapiGetUndefined(env);
    }
    session->SetSystemSceneBlockingFocus(blocking);
    TLOGD(WmsLogTag::DEFAULT, "end");
    return NapiGetUndefined(env);
}

napi_value JsSceneSession::OnSetExclusivelyHighlighted(napi_env env, napi_callback_info info)
{
    size_t argc = ARG_COUNT_4;
    napi_value argv[ARG_COUNT_4] = { nullptr };
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc != ARG_COUNT_1) {
        TLOGE(WmsLogTag::WMS_FOCUS, "Argc is invalid: %{public}zu", argc);
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    bool isExclusivelyHighlighted = true;
    if (!ConvertFromJsValue(env, argv[ARG_INDEX_0], isExclusivelyHighlighted)) {
        TLOGE(WmsLogTag::WMS_FOCUS, "Failed to convert parameter to isExclusivelyHighlighted");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    auto session = weakSession_.promote();
    if (session == nullptr) {
        TLOGE(WmsLogTag::WMS_FOCUS, "session is nullptr, id: %{public}d", persistentId_);
        return NapiGetUndefined(env);
    }
    session->SetExclusivelyHighlighted(isExclusivelyHighlighted);
    TLOGD(WmsLogTag::WMS_FOCUS, "end");
    return NapiGetUndefined(env);
}

napi_value JsSceneSession::OnMaskSupportEnterWaterfallMode(napi_env env, napi_callback_info info)
{
    auto session = weakSession_.promote();
    if (session == nullptr) {
        TLOGE(WmsLogTag::WMS_LAYOUT_PC, "session is nullptr, id:%{public}d", persistentId_);
        return NapiGetUndefined(env);
    }
    session->MaskSupportEnterWaterfallMode();
    return NapiGetUndefined(env);
}

napi_value JsSceneSession::OnUpdateFullScreenWaterfallMode(napi_env env, napi_callback_info info)
{
    size_t argc = ARG_COUNT_4;
    napi_value argv[ARG_COUNT_4] = { nullptr };
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc != ARG_INDEX_1) {
        TLOGE(WmsLogTag::WMS_LAYOUT_PC, "Argc is invalid: %{public}zu", argc);
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    bool isWaterfallMode = false;
    if (!ConvertFromJsValue(env, argv[ARG_INDEX_0], isWaterfallMode)) {
        TLOGE(WmsLogTag::WMS_LAYOUT_PC, "Failed to convert parameter to isWaterfallMode");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    auto session = weakSession_.promote();
    if (session == nullptr) {
        TLOGE(WmsLogTag::WMS_LAYOUT_PC, "session is nullptr, id:%{public}d", persistentId_);
        return NapiGetUndefined(env);
    }
    session->UpdateFullScreenWaterfallMode(isWaterfallMode);
    return NapiGetUndefined(env);
}

napi_value JsSceneSession::OnUpdateSizeChangeReason(napi_env env, napi_callback_info info)
{
    size_t argc = 4;
    napi_value argv[4] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc < 1) { // 1: params num
        TLOGE(WmsLogTag::WMS_LAYOUT, "Argc is invalid: %{public}zu", argc);
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    SizeChangeReason reason = SizeChangeReason::UNDEFINED;
    if (!ConvertFromJsValue(env, argv[0], reason)) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "Failed to convert parameter to bool");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    auto session = weakSession_.promote();
    if (session == nullptr) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "session is nullptr, id:%{public}d", persistentId_);
        return NapiGetUndefined(env);
    }
    session->UpdateSizeChangeReason(reason);
    TLOGD(WmsLogTag::WMS_LAYOUT, "%{public}u", reason);
    return NapiGetUndefined(env);
}

napi_value JsSceneSession::OnOpenKeyboardSyncTransaction(napi_env env, napi_callback_info info)
{
    auto session = weakSession_.promote();
    if (session == nullptr) {
        TLOGE(WmsLogTag::WMS_KEYBOARD, "session is nullptr, id:%{public}d", persistentId_);
        return NapiGetUndefined(env);
    }
    session->OpenKeyboardSyncTransaction();
    return NapiGetUndefined(env);
}

napi_value JsSceneSession::OnCloseKeyboardSyncTransaction(napi_env env, napi_callback_info info)
{
    size_t argc = ARG_COUNT_3;
    napi_value argv[ARG_COUNT_3] = { nullptr };
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc < ARG_COUNT_3) {
        TLOGE(WmsLogTag::WMS_KEYBOARD, "Argc is invalid: %{public}zu", argc);
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }

    KeyboardBaseInfo keyboardBaseInfo;
    if (!HandleCloseKeyboardSyncTransactionKeyboardBaseInfo(env, argv, ARG_INDEX_0, keyboardBaseInfo)) {
        TLOGE(WmsLogTag::WMS_KEYBOARD, "Failed to convert parameter to keyboardBaseInfo");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }

    KeyboardAnimationRectConfig keyboardAnimationRectConfig;
    if (!HandleCloseKeyboardSyncTransactionKeyboardAnimationRectConfig(env, argv, ARG_INDEX_1,
        keyboardAnimationRectConfig)) {
        TLOGE(WmsLogTag::WMS_KEYBOARD, "Failed to convert parameter to keyboardAnimationRectConfig");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }

    CallingWindowInfoData callingWindowInfoData;
    if (!HandleCallingWindowInfoData(env, argv, ARG_INDEX_2, callingWindowInfoData)) {
        TLOGE(WmsLogTag::WMS_KEYBOARD, "Failed to convert parameter to callingWindowInfoData");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    auto session = weakSession_.promote();
    if (session == nullptr) {
        TLOGE(WmsLogTag::WMS_KEYBOARD, "session is nullptr, id:%{public}d", persistentId_);
        return NapiGetUndefined(env);
    }

    WindowAnimationInfo animationInfo;
    animationInfo.beginRect = keyboardAnimationRectConfig.beginRect;
    animationInfo.endRect = keyboardAnimationRectConfig.endRect;
    animationInfo.animated =  keyboardAnimationRectConfig.animated;
    animationInfo.callingId = keyboardBaseInfo.callingId;
    animationInfo.isGravityChanged = keyboardBaseInfo.isGravityChanged;

    session->CloseKeyboardSyncTransaction(keyboardBaseInfo.keyboardPanelRect, keyboardBaseInfo.isKeyboardShow,
        animationInfo, callingWindowInfoData);
    return NapiGetUndefined(env);
}

bool JsSceneSession::HandleCloseKeyboardSyncTransactionKeyboardBaseInfo(napi_env env,
    napi_value argv[], int index, KeyboardBaseInfo& keyboardBaseInfo)
{
    napi_value nativeObj = argv[index];
    if (nativeObj == nullptr || !ConvertKeyboardBaseInfoFromJs(env, nativeObj, keyboardBaseInfo)) {
        TLOGE(WmsLogTag::WMS_KEYBOARD, "Failed to convert parameter to keyboardBaseInfo");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return false;
    }
    return true;
}

bool JsSceneSession::HandleCloseKeyboardSyncTransactionKeyboardAnimationRectConfig(napi_env env,
    napi_value argv[], int index, KeyboardAnimationRectConfig& keyboardAnimationRectConfig)
{
    napi_value nativeObj = argv[index];
    if (nativeObj == nullptr || !ConvertKeyboardAnimationRectConfigFromJs(env, nativeObj,
        keyboardAnimationRectConfig)) {
        TLOGE(WmsLogTag::WMS_KEYBOARD, "Failed to convert parameter to keyboardAnimationRectConfig");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return false;
    }
    return true;
}

bool JsSceneSession::HandleCallingWindowInfoData(napi_env env,
    napi_value argv[], int index, CallingWindowInfoData& callingWindowInfoData)
{
    napi_value nativeObj = argv[index];
    if (nativeObj == nullptr || !ConvertCallingWindowInfoDataFromJs(env, nativeObj, callingWindowInfoData)) {
        TLOGE(WmsLogTag::WMS_KEYBOARD, "Failed to convert parameter to callingWindowInfoData");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return false;
    }
    return true;
}

napi_value JsSceneSession::OnCallingWindowStateChange(napi_env env, napi_callback_info info)
{
    size_t argc = ARG_COUNT_1;
    napi_value argv[ARG_COUNT_1] = { nullptr };
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc < ARG_COUNT_1) {
        TLOGE(WmsLogTag::WMS_KEYBOARD, "Argc is invalid: %{public}zu", argc);
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }

    CallingWindowInfoData callingWindowInfoData;
    if (!HandleCallingWindowInfoData(env, argv, ARG_INDEX_0, callingWindowInfoData)) {
        TLOGE(WmsLogTag::WMS_KEYBOARD, "Failed to convert parameter to callingWindowInfoData");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }

    auto session = weakSession_.promote();
    if (session == nullptr) {
        TLOGE(WmsLogTag::WMS_KEYBOARD, "session is nullptr, id:%{public}d", persistentId_);
        return NapiGetUndefined(env);
    }

    session->CallingWindowStateChange(callingWindowInfoData);
    return NapiGetUndefined(env);
}

napi_value JsSceneSession::OnNotifyKeyboardAnimationCompleted(napi_env env, napi_callback_info info)
{
    size_t argc = ARG_COUNT_4;
    napi_value argv[ARG_COUNT_4] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc < ARG_COUNT_4) {
        TLOGE(WmsLogTag::WMS_KEYBOARD, "Argc is invalid: %{public}zu", argc);
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }

    uint32_t callingId = 0;
    if (!ConvertFromJsValue(env, argv[ARG_INDEX_0], callingId)) {
        TLOGE(WmsLogTag::WMS_KEYBOARD, "Failed to convert parameter to uint32_t");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }

    bool isShowAnimation = false;
    if (!ConvertFromJsValue(env, argv[ARG_INDEX_1], isShowAnimation)) {
        TLOGE(WmsLogTag::WMS_KEYBOARD, "Failed to convert parameter to bool");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }

    WSRect beginRect;
    napi_value nativeObj = argv[ARG_INDEX_2];
    if (nativeObj == nullptr || !ConvertSessionRectInfoFromJs(env, nativeObj, beginRect)) {
        TLOGE(WmsLogTag::WMS_KEYBOARD, "Failed to convert parameter to beginRect");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }

    WSRect endRect;
    nativeObj = argv[ARG_INDEX_3];
    if (nativeObj == nullptr || !ConvertSessionRectInfoFromJs(env, nativeObj, endRect)) {
        TLOGE(WmsLogTag::WMS_KEYBOARD, "Failed to convert parameter to endRect");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }

    sptr<SceneSession> callingSession = SceneSessionManager::GetInstance().GetSceneSession(callingId);
    if (callingSession == nullptr) {
        TLOGI(WmsLogTag::WMS_KEYBOARD, "callingSession is null, id: %{public}d", callingId);
        return NapiGetUndefined(env);
    }
    callingSession->NotifyKeyboardAnimationCompleted(isShowAnimation, beginRect, endRect);
    return NapiGetUndefined(env);
}

void JsSceneSession::OnCreateSubSession(const sptr<SceneSession>& sceneSession)
{
    if (sceneSession == nullptr) {
        WLOGFE("sceneSession is nullptr");
        return;
    }

    TLOGI(WmsLogTag::WMS_LIFE, "id: %{public}d, parentId: %{public}d",
        sceneSession->GetPersistentId(), sceneSession->GetParentPersistentId());
    wptr<SceneSession> weakSession(sceneSession);
    auto task = [weakThis = wptr(this), persistentId = persistentId_, weakSession, env = env_] {
        auto jsSceneSession = weakThis.promote();
        if (!jsSceneSession || jsSceneSessionMap_.find(persistentId) == jsSceneSessionMap_.end()) {
            TLOGNE(WmsLogTag::WMS_LIFE, "OnCreateSubSession jsSceneSession id:%{public}d has been destroyed",
                persistentId);
            return;
        }
        auto jsCallBack = jsSceneSession->GetJSCallback(CREATE_SUB_SESSION_CB);
        if (jsCallBack == nullptr) {
            TLOGNE(WmsLogTag::WMS_LIFE, "jsCallBack is nullptr");
            return;
        }
        auto specificSession = weakSession.promote();
        if (specificSession == nullptr) {
            TLOGNE(WmsLogTag::WMS_LIFE, "root session or target session or env is nullptr");
            return;
        }
        napi_value jsSceneSessionObj = Create(env, specificSession);
        if (jsSceneSessionObj == nullptr) {
            TLOGNE(WmsLogTag::WMS_LIFE, "jsSceneSessionObj or jsCallBack is nullptr");
            return;
        }
        TLOGNI(WmsLogTag::WMS_LIFE, "CreateJsSceneSessionObject success, id: %{public}d, parentId: %{public}d",
            specificSession->GetPersistentId(), specificSession->GetParentPersistentId());
        napi_value argv[] = {jsSceneSessionObj};
        napi_call_function(env, NapiGetUndefined(env), jsCallBack->GetNapiValue(), ArraySize(argv), argv, nullptr);
    };
    std::string info = "OnCreateSpecificSession PID:" + std::to_string(sceneSession->GetPersistentId());
    taskScheduler_->PostMainThreadTask(task, info);
}

void JsSceneSession::OnClearSubSession(int32_t subPersistentId)
{
    auto task = [weakThis = wptr(this), persistentId = persistentId_, subPersistentId, env = env_] {
        auto jsSceneSession = weakThis.promote();
        if (!jsSceneSession || jsSceneSessionMap_.find(persistentId) == jsSceneSessionMap_.end()) {
            TLOGNE(WmsLogTag::WMS_LIFE, "OnClearSubSession jsSceneSession id:%{public}d has been destroyed",
                persistentId);
            return;
        }
        auto jsCallBack = jsSceneSession->GetJSCallback(CLEAR_SUB_SESSION_CB);
        if (jsCallBack == nullptr) {
            TLOGNE(WmsLogTag::WMS_LIFE, "jsCallBack is nullptr");
            return;
        }
        napi_value jsSubPersistentId = CreateJsValue(env, subPersistentId);
        napi_value argv[] = {jsSubPersistentId};
        napi_call_function(env, NapiGetUndefined(env), jsCallBack->GetNapiValue(), ArraySize(argv), argv, nullptr);
    };
    std::string info = "OnClearSubSession id:" + std::to_string(subPersistentId);
    taskScheduler_->PostMainThreadTask(task, info);
}

void JsSceneSession::OnBindDialogTarget(const sptr<SceneSession>& sceneSession)
{
    if (sceneSession == nullptr) {
        TLOGD(WmsLogTag::DEFAULT, "sceneSession is nullptr");
        return;
    }

    TLOGI(WmsLogTag::WMS_DIALOG, "id: %{public}d", sceneSession->GetPersistentId());

    wptr<SceneSession> weakSession(sceneSession);
    auto task = [weakThis = wptr(this), persistentId = persistentId_, weakSession, env = env_] {
        auto jsSceneSession = weakThis.promote();
        if (!jsSceneSession || jsSceneSessionMap_.find(persistentId) == jsSceneSessionMap_.end()) {
            TLOGNE(WmsLogTag::WMS_LIFE, "OnBindDialogTarget jsSceneSession id:%{public}d has been destroyed",
                persistentId);
            return;
        }
        auto jsCallBack = jsSceneSession->GetJSCallback(BIND_DIALOG_TARGET_CB);
        if (jsCallBack == nullptr) {
            TLOGNE(WmsLogTag::WMS_LIFE, "jsCallBack is nullptr");
            return;
        }
        auto specificSession = weakSession.promote();
        if (specificSession == nullptr) {
            TLOGNE(WmsLogTag::WMS_LIFE, "root session or target session or env is nullptr");
            return;
        }
        napi_value jsSceneSessionObj = Create(env, specificSession);
        if (jsSceneSessionObj == nullptr) {
            TLOGNE(WmsLogTag::WMS_LIFE, "jsSceneSessionObj or jsCallBack is nullptr");
            return;
        }
        TLOGNI(WmsLogTag::WMS_DIALOG, "CreateJsObject success, id: %{public}d", specificSession->GetPersistentId());
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
            TLOGNE(WmsLogTag::WMS_LIFE, "OnSessionStateChange jsSceneSession id:%{public}d has been destroyed",
                persistentId);
            return;
        }
        auto jsCallBack = jsSceneSession->GetJSCallback(SESSION_STATE_CHANGE_CB);
        if (!jsCallBack) {
            TLOGNE(WmsLogTag::WMS_LIFE, "jsCallBack is nullptr");
            return;
        }
        napi_value jsSessionStateObj = CreateJsValue(env, state);
        napi_value argv[] = {jsSessionStateObj};
        napi_call_function(env, NapiGetUndefined(env), jsCallBack->GetNapiValue(), ArraySize(argv), argv, nullptr);
    };
    taskScheduler_->PostMainThreadTask(task, "OnSessionStateChange, state:" + std::to_string(static_cast<int>(state)));
}

void JsSceneSession::OnUpdateTransitionAnimation(const WindowTransitionType& type, const TransitionAnimation& animation)
{
    auto session = weakSession_.promote();
    if (session == nullptr) {
        TLOGE(WmsLogTag::WMS_ANIMATION, "session is nullptr, id:%{public}d", persistentId_);
        return;
    }

    TLOGI(WmsLogTag::WMS_ANIMATION, "id: %{public}d, type: %{public}d", session->GetPersistentId(), type);
    auto task = [weakThis = wptr(this), persistentId = persistentId_, type, animation, env = env_] {
        auto jsSceneSession = weakThis.promote();
        if (!jsSceneSession || jsSceneSessionMap_.find(persistentId) == jsSceneSessionMap_.end()) {
            TLOGNE(WmsLogTag::WMS_ANIMATION,
                "OnUpdateTransitionAnimation jsSceneSession id:%{public}d has been destroyed", persistentId);
            return;
        }
        auto jsCallBack = jsSceneSession->GetJSCallback(UPDATE_TRANSITION_ANIMATION_CB);
        if (!jsCallBack) {
            TLOGNE(WmsLogTag::WMS_ANIMATION, "jsCallBack is nullptr");
            return;
        }
        napi_value jsTransitionTypeObj = CreateJsValue(env, type);
        napi_value jsTransitionAnimationObj = ConvertTransitionAnimationToJsValue(env,
            std::make_shared<TransitionAnimation>(animation));
        napi_value argv[] = {jsTransitionTypeObj, jsTransitionAnimationObj};
        napi_call_function(env, NapiGetUndefined(env), jsCallBack->GetNapiValue(), ArraySize(argv), argv, nullptr);
    };
    taskScheduler_->PostMainThreadTask(task, "OnUpdateTransitionAnimation, type:" +
        std::to_string(static_cast<int>(type)));
}

void JsSceneSession::OnBufferAvailableChange(const bool isBufferAvailable, bool startWindowInvisible)
{
    TLOGD(WmsLogTag::DEFAULT, "state: %{public}u, %{public}u", isBufferAvailable, startWindowInvisible);
    auto task = [weakThis = wptr(this), persistentId = persistentId_,
                isBufferAvailable, startWindowInvisible, env = env_] {
        auto jsSceneSession = weakThis.promote();
        if (!jsSceneSession || jsSceneSessionMap_.find(persistentId) == jsSceneSessionMap_.end()) {
            TLOGNE(WmsLogTag::WMS_LIFE, "OnBufferAvailableChange jsSceneSession id:%{public}d has been destroyed",
                persistentId);
            return;
        }
        auto jsCallBack = jsSceneSession->GetJSCallback(BUFFER_AVAILABLE_CHANGE_CB);
        if (!jsCallBack) {
            TLOGNE(WmsLogTag::WMS_LIFE, "jsCallBack is nullptr");
            return;
        }
        napi_value jsBufferAvailableObj = CreateJsValue(env, isBufferAvailable);
        napi_value jsStartWindowInvisibleObj = CreateJsValue(env, startWindowInvisible);
        napi_value argv[] = { jsBufferAvailableObj, jsStartWindowInvisibleObj };
        napi_call_function(env, NapiGetUndefined(env), jsCallBack->GetNapiValue(), ArraySize(argv), argv, nullptr);
    };
    taskScheduler_->PostMainThreadTask(task, "OnBufferAvailableChange");
}

/** @note @window.layout */
void JsSceneSession::OnSessionRectChange(const WSRect& rect, SizeChangeReason reason, DisplayId displayId,
    const RectAnimationConfig& rectAnimationConfig)
{
    if (!IsMoveToOrDragMove(reason) && reason != SizeChangeReason::PIP_RESTORE && rect.IsEmpty()) {
        TLOGD(WmsLogTag::DEFAULT, "Rect is empty, there is no need to notify");
        return;
    }
    TLOGD(WmsLogTag::DEFAULT, "[NAPI]");
    auto task = [weakThis = wptr(this), persistentId = persistentId_, rect, displayId, reason,
        rectAnimationConfig, env = env_] {
        auto jsSceneSession = weakThis.promote();
        if (!jsSceneSession || jsSceneSessionMap_.find(persistentId) == jsSceneSessionMap_.end()) {
            TLOGNE(WmsLogTag::WMS_LAYOUT, "OnSessionRectChange jsSceneSession id:%{public}d has been destroyed",
                persistentId);
            return;
        }
        auto jsCallBack = jsSceneSession->GetJSCallback(SESSION_RECT_CHANGE_CB);
        if (!jsCallBack) {
            TLOGNE(WmsLogTag::WMS_LAYOUT, "jsCallBack is nullptr");
            return;
        }
        napi_value jsSessionRect = CreateJsSessionRect(env, rect);
        napi_value jsSizeChangeReason = CreateJsValue(env, static_cast<int32_t>(reason));
        napi_value jsDisplayId = CreateJsValue(env, static_cast<int32_t>(displayId));
        napi_value jsAnimationConfigObj = CreateJsRectAnimationConfig(env, rectAnimationConfig);
        napi_value argv[] = { jsSessionRect, jsSizeChangeReason, jsDisplayId, jsAnimationConfigObj };
        napi_call_function(env, NapiGetUndefined(env), jsCallBack->GetNapiValue(), ArraySize(argv), argv, nullptr);
    };
    std::string rectInfo = "OnSessionRectChange [" + std::to_string(rect.posX_) + "," + std::to_string(rect.posY_)
        + "], [" + std::to_string(rect.width_) + ", " + std::to_string(rect.height_);
    taskScheduler_->PostMainThreadTask(task, rectInfo);
}

/** @note @window.layout */
void JsSceneSession::OnSessionWindowLimitsChange(const WindowLimits& windowLimits)
{
    TLOGD(WmsLogTag::WMS_LAYOUT, "[NAPI]");
    auto task = [weakThis = wptr(this), persistentId = persistentId_, windowLimits, env = env_, funcName = __func__] {
        auto jsSceneSession = weakThis.promote();
        if (!jsSceneSession || jsSceneSessionMap_.find(persistentId) == jsSceneSessionMap_.end()) {
            TLOGNE(WmsLogTag::WMS_LAYOUT, "%{public}s: jsSceneSession id:%{public}d has been destroyed",
                funcName, persistentId);
            return;
        }
        auto jsCallBack = jsSceneSession->GetJSCallback(SESSION_WINDOW_LIMITS_CHANGE_CB);
        if (!jsCallBack) {
            TLOGNE(WmsLogTag::WMS_LAYOUT, "%{public}s: jsCallBack is nullptr", funcName);
            return;
        }
        napi_value jsSessionWindowLimits = JsWindowSceneConfig::CreateWindowLimits(env, windowLimits);
        napi_value argv[] = { jsSessionWindowLimits };
        napi_call_function(env, NapiGetUndefined(env), jsCallBack->GetNapiValue(), ArraySize(argv), argv, nullptr);
    };
    std::string windowLimitsInfo = std::string(__func__) + " [" + std::to_string(windowLimits.maxWidth_) + ", " +
        std::to_string(windowLimits.maxHeight_) + ", " + std::to_string(windowLimits.minWidth_) + ", " +
        std::to_string(windowLimits.minHeight_) + ", " + std::to_string(static_cast<uint32_t>(windowLimits.pixelUnit_))
        + "] id:" + std::to_string(persistentId_);
    taskScheduler_->PostMainThreadTask(task, windowLimitsInfo);
}

void JsSceneSession::OnFloatingBallUpdate(const FloatingBallTemplateInfo& fbTemplateInfo)
{
    auto task = [weakThis = wptr(this), persistentId = persistentId_, fbTemplateInfo, env = env_] {
        auto jsSceneSession = weakThis.promote();
        if (!jsSceneSession || jsSceneSessionMap_.find(persistentId) == jsSceneSessionMap_.end()) {
            TLOGNE(WmsLogTag::WMS_LAYOUT, "OnFloatingBallUpdate jsSceneSession id:%{public}d has been destroyed",
                persistentId);
            return;
        }
        auto jsCallBack = jsSceneSession->GetJSCallback(FLOATING_BALL_UPDATE_CB);
        if (!jsCallBack) {
            TLOGNE(WmsLogTag::WMS_LAYOUT, "jsCallBack is nullptr");
            return;
        }
        napi_value jsTemplate = CreateJsValue(env, fbTemplateInfo.template_);
        if (jsTemplate == nullptr) {
            TLOGNE(WmsLogTag::WMS_MAIN, "jsTemplate is nullptr");
            return;
        }
        napi_value jsTitle = CreateJsValue(env, fbTemplateInfo.title_);
        if (jsTitle == nullptr) {
            TLOGNE(WmsLogTag::WMS_MAIN, "jsTitle is nullptr");
            return;
        }
        napi_value jsContent = CreateJsValue(env, fbTemplateInfo.content_);
        if (jsContent == nullptr) {
            TLOGNE(WmsLogTag::WMS_MAIN, "jsContent is nullptr");
            return;
        }
        napi_value jsBackgroundColor = CreateJsValue(env, fbTemplateInfo.backgroundColor_);
        if (jsBackgroundColor == nullptr) {
            TLOGNE(WmsLogTag::WMS_MAIN, "jsBackgroundColor is nullptr");
            return;
        }
        napi_value jsIcon = Media::PixelMapNapi::CreatePixelMap(env, fbTemplateInfo.icon_);
        if (jsIcon == nullptr) {
            TLOGNE(WmsLogTag::WMS_MAIN, "icon is nullptr");
            return;
        }
        napi_value fbTemplateInfoValue = nullptr;
        napi_create_object(env, &fbTemplateInfoValue);
        napi_set_named_property(env, fbTemplateInfoValue, "template", jsTemplate);
        napi_set_named_property(env, fbTemplateInfoValue, "title", jsTitle);
        napi_set_named_property(env, fbTemplateInfoValue, "content", jsContent);
        napi_set_named_property(env, fbTemplateInfoValue, "backgroundColor", jsBackgroundColor);
        napi_set_named_property(env, fbTemplateInfoValue, "icon", jsIcon);
        napi_value argv[] = {fbTemplateInfoValue};
        napi_call_function(env, NapiGetUndefined(env), jsCallBack->GetNapiValue(), ArraySize(argv), argv, nullptr);
    };
    taskScheduler_->PostMainThreadTask(task, __func__);
}

void JsSceneSession::OnFloatingBallStop()
{
    TLOGND(WmsLogTag::WMS_LAYOUT, "[NAPI]");
    auto task = [weakThis = wptr(this), persistentId = persistentId_, env = env_] {
        auto jsSceneSession = weakThis.promote();
        if (!jsSceneSession || jsSceneSessionMap_.find(persistentId) == jsSceneSessionMap_.end()) {
            TLOGNE(WmsLogTag::WMS_LAYOUT, "OnFloatingBallStop jsSceneSession id:%{public}d has been destroyed",
                persistentId);
            return;
        }
        auto jsCallBack = jsSceneSession->GetJSCallback(FLOATING_BALL_STOP_CB);
        if (!jsCallBack) {
            TLOGNE(WmsLogTag::WMS_LAYOUT, "jsCallBack is nullptr");
            return;
        }
        napi_value argv[] = {};
        napi_call_function(env, NapiGetUndefined(env), jsCallBack->GetNapiValue(), 0, argv, nullptr);
    };
    taskScheduler_->PostMainThreadTask(task, __func__);
}

void JsSceneSession::OnFloatingBallRestoreMainWindow(const std::shared_ptr<AAFwk::Want>& want)
{
    TLOGND(WmsLogTag::WMS_LAYOUT, "[NAPI]");
    auto task = [weakThis = wptr(this), want, persistentId = persistentId_, env = env_] {
        auto jsSceneSession = weakThis.promote();
        if (!jsSceneSession || jsSceneSessionMap_.find(persistentId) == jsSceneSessionMap_.end()) {
            TLOGNE(WmsLogTag::WMS_LAYOUT,
                "OnFloatingBallRestoreMainWindow jsSceneSession id:%{public}d has been destroyed", persistentId);
            return;
        }
        auto jsCallBack = jsSceneSession->GetJSCallback(FLOATING_BALL_RESTORE_MAIN_WINDOW_CB);
        if (!jsCallBack) {
            TLOGNE(WmsLogTag::WMS_LAYOUT, "jsCallBack is nullptr");
            return;
        }
        napi_value jsWant = AppExecFwk::WrapWant(env, *want);
        if (jsWant == nullptr) {
            TLOGNE(WmsLogTag::WMS_LAYOUT, "null jsWant");
            return;
        }
        napi_value argv[] = {jsWant};
        napi_call_function(env, NapiGetUndefined(env), jsCallBack->GetNapiValue(), ArraySize(argv), argv, nullptr);
    };
    taskScheduler_->PostMainThreadTask(task, __func__);
}

void JsSceneSession::OnWindowMoving(DisplayId displayId, int32_t pointerX, int32_t pointerY)
{
    auto task = [weakThis = wptr(this), persistentId = persistentId_, displayId, pointerX, pointerY, env = env_] {
        auto jsSceneSession = weakThis.promote();
        if (!jsSceneSession || jsSceneSessionMap_.find(persistentId) == jsSceneSessionMap_.end()) {
            TLOGNE(WmsLogTag::WMS_DECOR, "jsSceneSession id:%{public}d has been destroyed",
                persistentId);
            return;
        }
        auto jsCallBack = jsSceneSession->GetJSCallback(WINDOW_MOVING_CB);
        if (!jsCallBack) {
            TLOGNE(WmsLogTag::WMS_DECOR, "jsCallBack is nullptr");
            return;
        }
        napi_value jsDisplayId = CreateJsValue(env, static_cast<int64_t>(displayId));
        napi_value jsPointerX = CreateJsValue(env, pointerX);
        napi_value jsPointerY = CreateJsValue(env, pointerY);
        napi_value argv[] = {jsDisplayId, jsPointerX, jsPointerY};
        napi_call_function(env, NapiGetUndefined(env), jsCallBack->GetNapiValue(), ArraySize(argv), argv, nullptr);
    };
    taskScheduler_->PostMainThreadTask(task, "OnWindowMoving");
}

void JsSceneSession::OnSessionDisplayIdChange(uint64_t displayId)
{
    auto task = [weakThis = wptr(this), displayId, persistentId = persistentId_, env = env_] {
        auto jsSceneSession = weakThis.promote();
        if (!jsSceneSession || jsSceneSessionMap_.find(persistentId) == jsSceneSessionMap_.end()) {
            TLOGNE(WmsLogTag::WMS_LAYOUT, "OnSessionDisplayIdChange jsSceneSession id:%{public}d has been destroyed",
                persistentId);
            return;
        }
        auto jsCallBack = jsSceneSession->GetJSCallback(SESSION_DISPLAY_ID_CHANGE_CB);
        if (!jsCallBack) {
            TLOGNE(WmsLogTag::WMS_LAYOUT, "jsCallBack is nullptr");
            return;
        }
        napi_value jsSessionDisplayIdObj = CreateJsValue(env, static_cast<int32_t>(displayId));
        napi_value argv[] = { jsSessionDisplayIdObj };
        napi_call_function(env, NapiGetUndefined(env), jsCallBack->GetNapiValue(), ArraySize(argv), argv, nullptr);
    };
    taskScheduler_->PostMainThreadTask(task, __func__);
}

void JsSceneSession::OnSessionPiPControlStatusChange(WsPiPControlType controlType, WsPiPControlStatus status)
{
    TLOGI(WmsLogTag::WMS_PIP, "controlType:%{public}u, status:%{public}d", controlType, status);
    auto task = [weakThis = wptr(this), persistentId = persistentId_, controlType, status, env = env_] {
        auto jsSceneSession = weakThis.promote();
        if (!jsSceneSession || jsSceneSessionMap_.find(persistentId) == jsSceneSessionMap_.end()) {
            TLOGNE(WmsLogTag::WMS_LIFE, "OnSessionPiPControlStatusChange jsSceneSession id:%{public}d has been"
                " destroyed", persistentId);
            return;
        }
        auto jsCallBack = jsSceneSession->GetJSCallback(SESSION_PIP_CONTROL_STATUS_CHANGE_CB);
        if (!jsCallBack) {
            TLOGNE(WmsLogTag::WMS_PIP, "jsCallBack is nullptr");
            return;
        }
        napi_value controlTypeValue = CreateJsValue(env, controlType);
        napi_value controlStatusValue = CreateJsValue(env, status);
        napi_value argv[] = {controlTypeValue, controlStatusValue};
        napi_call_function(env, NapiGetUndefined(env), jsCallBack->GetNapiValue(), ArraySize(argv), argv, nullptr);
    };
    taskScheduler_->PostMainThreadTask(task, __func__);
}

void JsSceneSession::OnAutoStartPiPStatusChange(bool isAutoStart, uint32_t priority, uint32_t width, uint32_t height)
{
    TLOGI(WmsLogTag::WMS_PIP, "isAutoStart:%{public}u priority:%{public}u width:%{public}u height:%{public}u",
        isAutoStart, priority, width, height);
    auto task = [weakThis = wptr(this), persistentId = persistentId_, isAutoStart, priority, width, height,
        env = env_] {
        auto jsSceneSession = weakThis.promote();
        if (!jsSceneSession || jsSceneSessionMap_.find(persistentId) == jsSceneSessionMap_.end()) {
            TLOGNE(WmsLogTag::WMS_PIP, "jsSceneSession id:%{public}d has been destroyed", persistentId);
            return;
        }
        auto jsCallBack = jsSceneSession->GetJSCallback(SESSION_AUTO_START_PIP_CB);
        if (!jsCallBack) {
            TLOGNE(WmsLogTag::WMS_PIP, "jsCallBack is nullptr");
            return;
        }
        napi_value isAutoStartValue = CreateJsValue(env, isAutoStart);
        napi_value priorityValue = CreateJsValue(env, priority);
        napi_value widthValue = CreateJsValue(env, width);
        napi_value heightValue = CreateJsValue(env, height);
        napi_value argv[] = {isAutoStartValue, priorityValue, widthValue, heightValue};
        napi_call_function(env, NapiGetUndefined(env), jsCallBack->GetNapiValue(), ArraySize(argv), argv, nullptr);
    };
    taskScheduler_->PostMainThreadTask(task, __func__);
}

void JsSceneSession::OnUpdatePiPTemplateInfo(PiPTemplateInfo& pipTemplateInfo)
{
    TLOGI(WmsLogTag::WMS_PIP, "pipTemplateType: %{public}u, priority: %{public}d, "
        "defaultWindowSizeType: %{public}d, cornerAdsorptionEnabled: %{public}d",
        pipTemplateInfo.pipTemplateType, pipTemplateInfo.priority, pipTemplateInfo.defaultWindowSizeType,
        pipTemplateInfo.cornerAdsorptionEnabled);
    auto task = [weakThis = wptr(this), persistentId = persistentId_,
        pipTemplateInfo = std::move(pipTemplateInfo), env = env_] {
        auto jsSceneSession = weakThis.promote();
        if (!jsSceneSession || jsSceneSessionMap_.find(persistentId) == jsSceneSessionMap_.end()) {
            TLOGNE(WmsLogTag::WMS_PIP, "jsSceneSession id:%{public}d has been destroyed", persistentId);
            return;
        }
        auto jsCallBack = jsSceneSession->GetJSCallback(UPDATE_PIP_TEMPLATE_INFO_CB);
        if (!jsCallBack) {
            TLOGNE(WmsLogTag::WMS_PIP, "jsCallBack is nullptr");
            return;
        }
        napi_value pipTemplateInfoValue = CreatePipTemplateInfo(env, pipTemplateInfo);
        napi_value argv[] = {pipTemplateInfoValue};
        napi_call_function(env, NapiGetUndefined(env), jsCallBack->GetNapiValue(), ArraySize(argv), argv, nullptr);
    };
    taskScheduler_->PostMainThreadTask(task, __func__);
}

void JsSceneSession::OnSetPiPParentWindowId(uint32_t windowId)
{
    TLOGI(WmsLogTag::WMS_PIP, "PiPParentWindowId: %{public}u", windowId);
    auto task = [weakThis = wptr(this), persistentId = persistentId_,
        windowId, env = env_] {
        auto jsSceneSession = weakThis.promote();
        if (!jsSceneSession || jsSceneSessionMap_.find(persistentId) == jsSceneSessionMap_.end()) {
            TLOGNE(WmsLogTag::WMS_PIP, "jsSceneSession id:%{public}d has been destroyed", persistentId);
            return;
        }
        auto jsCallBack = jsSceneSession->GetJSCallback(SET_PIP_PARENT_WINDOWID_CB);
        if (!jsCallBack) {
            TLOGNE(WmsLogTag::WMS_PIP, "jsCallBack is nullptr");
            return;
        }
        napi_value parentWindowIdValue = CreateJsValue(env, windowId);
        napi_value argv[] = {parentWindowIdValue};
        napi_call_function(env, NapiGetUndefined(env), jsCallBack->GetNapiValue(), ArraySize(argv), argv, nullptr);
    };
    taskScheduler_->PostMainThreadTask(task, __func__);
}

/** @note @window.hierarchy */
void JsSceneSession::OnRaiseToTop()
{
    TLOGD(WmsLogTag::DEFAULT, "[NAPI]");
    auto task = [weakThis = wptr(this), persistentId = persistentId_, env = env_] {
        auto jsSceneSession = weakThis.promote();
        if (!jsSceneSession || jsSceneSessionMap_.find(persistentId) == jsSceneSessionMap_.end()) {
            TLOGE(WmsLogTag::WMS_HIERARCHY, "OnRaiseToTop jsSceneSession id:%{public}d has been destroyed",
                persistentId);
            return;
        }
        auto jsCallBack = jsSceneSession->GetJSCallback(RAISE_TO_TOP_CB);
        if (!jsCallBack) {
            WLOGFE("jsCallBack is nullptr");
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
    TLOGD(WmsLogTag::DEFAULT, "[NAPI]");
    auto task = [weakThis = wptr(this), persistentId = persistentId_, env = env_] {
        auto jsSceneSession = weakThis.promote();
        if (!jsSceneSession || jsSceneSessionMap_.find(persistentId) == jsSceneSessionMap_.end()) {
            TLOGNE(WmsLogTag::WMS_HIERARCHY, "OnRaiseToTopForPointDown jsSceneSession id:%{public}d has been destroyed",
                persistentId);
            return;
        }
        auto jsCallBack = jsSceneSession->GetJSCallback(RAISE_TO_TOP_POINT_DOWN_CB);
        if (!jsCallBack) {
            TLOGNE(WmsLogTag::WMS_HIERARCHY, "jsCallBack is nullptr");
            return;
        }
        napi_value argv[] = {};
        napi_call_function(env, NapiGetUndefined(env), jsCallBack->GetNapiValue(), 0, argv, nullptr);
    };
    taskScheduler_->PostMainThreadTask(task, "OnRaiseToTopForPointDown");
}

void JsSceneSession::OnClickModalWindowOutside()
{
    TLOGD(WmsLogTag::WMS_PC, "[NAPI]");
    const char* const where = __func__;
    auto task = [weakThis = wptr(this), persistentId = persistentId_, env = env_, where] {
        auto jsSceneSession = weakThis.promote();
        if (!jsSceneSession || jsSceneSessionMap_.find(persistentId) == jsSceneSessionMap_.end()) {
            TLOGNE(WmsLogTag::WMS_LIFE, "%{public}s jsSceneSession id:%{public}d has been destroyed",
                where, persistentId);
            return;
        }
        auto jsCallBack = jsSceneSession->GetJSCallback(CLICK_MODAL_WINDOW_OUTSIDE_CB);
        if (!jsCallBack) {
            TLOGNE(WmsLogTag::WMS_PC, "%{public}s jsCallBack is nullptr", where);
            return;
        }
        napi_value argv[] = {};
        napi_call_function(env, NapiGetUndefined(env), jsCallBack->GetNapiValue(), 0, argv, nullptr);
    };
    taskScheduler_->PostMainThreadTask(task, __func__);
}

void JsSceneSession::OnRaiseAboveTarget(int32_t subWindowId)
{
    TLOGD(WmsLogTag::DEFAULT, "[NAPI]");
    auto task = [weakThis = wptr(this), persistentId = persistentId_, env = env_, subWindowId] {
        auto jsSceneSession = weakThis.promote();
        if (!jsSceneSession || jsSceneSessionMap_.find(persistentId) == jsSceneSessionMap_.end()) {
            TLOGNE(WmsLogTag::WMS_LIFE, "OnRaiseAboveTarget jsSceneSession id:%{public}d has been destroyed",
                persistentId);
            return;
        }
        auto jsCallBack = jsSceneSession->GetJSCallback(RAISE_ABOVE_TARGET_CB);
        if (!jsCallBack) {
            TLOGNE(WmsLogTag::WMS_LIFE, "jsCallBack is nullptr");
            return;
        }
        napi_value jsSceneSessionObj = CreateJsValue(env, subWindowId);
        if (jsSceneSessionObj == nullptr) {
            TLOGNE(WmsLogTag::WMS_LIFE, "jsSceneSessionObj is nullptr");
            return;
        }
        napi_value argv[] = {CreateJsError(env, 0), jsSceneSessionObj};
        napi_call_function(env, NapiGetUndefined(env), jsCallBack->GetNapiValue(), ArraySize(argv), argv, nullptr);
    };
    taskScheduler_->PostMainThreadTask(task, "OnRaiseAboveTarget: " + std::to_string(subWindowId));
}

void JsSceneSession::OnRaiseMainWindowAboveTarget(int32_t targetId)
{
    TLOGD(WmsLogTag::DEFAULT, "[NAPI]");
    auto task = [weakThis = wptr(this), persistentId = persistentId_, env = env_, targetId] {
        auto jsSceneSession = weakThis.promote();
        if (!jsSceneSession || jsSceneSessionMap_.find(persistentId) == jsSceneSessionMap_.end()) {
            TLOGNE(WmsLogTag::WMS_LIFE, "OnRaiseMainWindowAboveTarget jsSceneSession id:%{public}d has been destroyed",
                persistentId);
            return;
        }
        auto jsCallBack = jsSceneSession->GetJSCallback(RAISE_MAIN_WINDOW_ABOVE_TARGET_CB);
        if (!jsCallBack) {
            TLOGNE(WmsLogTag::WMS_LIFE, "jsCallBack is nullptr");
            return;
        }
        napi_value jsSceneSessionObj = CreateJsValue(env, targetId);
        if (jsSceneSessionObj == nullptr) {
            TLOGNE(WmsLogTag::WMS_LIFE, "jsSceneSessionObj is nullptr");
            return;
        }
        napi_value argv[] = {CreateJsError(env, 0), jsSceneSessionObj};
        napi_call_function(env, NapiGetUndefined(env), jsCallBack->GetNapiValue(), ArraySize(argv), argv, nullptr);
    };
    taskScheduler_->PostMainThreadTask(task, "OnRaiseMainWindowAboveTarget: " + std::to_string(targetId));
}

void JsSceneSession::OnSessionFocusableChange(bool isFocusable)
{
    TLOGD(WmsLogTag::WMS_FOCUS, "state: %{public}u", isFocusable);
    auto task = [weakThis = wptr(this), persistentId = persistentId_, isFocusable, env = env_] {
        auto jsSceneSession = weakThis.promote();
        if (!jsSceneSession || jsSceneSessionMap_.find(persistentId) == jsSceneSessionMap_.end()) {
            TLOGNE(WmsLogTag::WMS_LIFE, "OnSessionFocusableChange jsSceneSession id:%{public}d has been destroyed",
                persistentId);
            return;
        }
        auto jsCallBack = jsSceneSession->GetJSCallback(SESSION_FOCUSABLE_CHANGE_CB);
        if (!jsCallBack) {
            TLOGNE(WmsLogTag::WMS_LIFE, "jsCallBack is nullptr");
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
    TLOGI(WmsLogTag::DEFAULT, "%{public}u", touchable);
    auto task = [weakThis = wptr(this), persistentId = persistentId_, touchable, env = env_] {
        auto jsSceneSession = weakThis.promote();
        if (!jsSceneSession || jsSceneSessionMap_.find(persistentId) == jsSceneSessionMap_.end()) {
            TLOGNE(WmsLogTag::WMS_LIFE, "OnSessionTouchableChange jsSceneSession id:%{public}d has been destroyed",
                persistentId);
            return;
        }
        auto jsCallBack = jsSceneSession->GetJSCallback(SESSION_TOUCHABLE_CHANGE_CB);
        if (!jsCallBack) {
            TLOGNE(WmsLogTag::WMS_LIFE, "jsCallBack is nullptr");
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
    TLOGI(WmsLogTag::WMS_HIERARCHY, "State: %{public}u", topmost);
    auto task = [weakThis = wptr(this), persistentId = persistentId_, topmost, env = env_] {
        auto jsSceneSession = weakThis.promote();
        if (!jsSceneSession || jsSceneSessionMap_.find(persistentId) == jsSceneSessionMap_.end()) {
            TLOGNE(WmsLogTag::WMS_HIERARCHY, "OnSessionTopmostChange jsSceneSession id:%{public}d has been destroyed",
                persistentId);
            return;
        }
        auto jsCallBack = jsSceneSession->GetJSCallback(SESSION_TOP_MOST_CHANGE_CB);
        if (!jsCallBack) {
            TLOGNE(WmsLogTag::WMS_HIERARCHY, "jsCallBack is nullptr");
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
    TLOGD(WmsLogTag::WMS_HIERARCHY, "isTopmost: %{public}u", isTopmost);
    const char* const where = __func__;
    taskScheduler_->PostMainThreadTask([weakThis = wptr(this), persistentId = persistentId_,
        isTopmost, env = env_, where] {
        auto jsSceneSession = weakThis.promote();
        if (!jsSceneSession || jsSceneSessionMap_.find(persistentId) == jsSceneSessionMap_.end()) {
            TLOGNE(WmsLogTag::WMS_HIERARCHY, "%{public}s jsSceneSession id:%{public}d has been destroyed",
                where, persistentId);
            return;
        }
        auto jsCallBack = jsSceneSession->GetJSCallback(MAIN_WINDOW_TOP_MOST_CHANGE_CB);
        if (!jsCallBack) {
            TLOGNE(WmsLogTag::WMS_HIERARCHY, "jsCallBack is nullptr");
            return;
        }
        napi_value jsMainWindowTopmostObj = CreateJsValue(env, isTopmost);
        napi_value argv[] = { jsMainWindowTopmostObj };
        napi_call_function(env, NapiGetUndefined(env), jsCallBack->GetNapiValue(), ArraySize(argv), argv, nullptr);
    }, "OnMainWindowTopmostChange: " + std::to_string(isTopmost));
}

void JsSceneSession::OnSubSessionZLevelChange(int32_t zLevel)
{
    auto task = [weakThis = wptr(this), persistentId = persistentId_, zLevel, env = env_] {
        auto jsSceneSession = weakThis.promote();
        if (!jsSceneSession || jsSceneSessionMap_.find(persistentId) == jsSceneSessionMap_.end()) {
            TLOGNE(WmsLogTag::WMS_HIERARCHY, "jsSceneSession id:%{public}d has been destroyed", persistentId);
            return;
        }
        auto jsCallBack = jsSceneSession->GetJSCallback(Z_LEVEL_CHANGE_CB);
        if (!jsCallBack) {
            TLOGNE(WmsLogTag::WMS_HIERARCHY, "jsCallBack is nullptr");
            return;
        }
        napi_value jsZLevelObj = CreateJsValue(env, zLevel);
        napi_value argv[] = {jsZLevelObj};
        napi_call_function(env, NapiGetUndefined(env), jsCallBack->GetNapiValue(), ArraySize(argv), argv, nullptr);
    };
    taskScheduler_->PostMainThreadTask(task, "OnSubSessionZLevelChange: " + std::to_string(zLevel));
}

void JsSceneSession::OnSubModalTypeChange(SubWindowModalType subWindowModalType)
{
    const char* const where = __func__;
    auto task = [weakThis = wptr(this), persistentId = persistentId_, subWindowModalType, env = env_, where] {
        auto jsSceneSession = weakThis.promote();
        if (!jsSceneSession || jsSceneSessionMap_.find(persistentId) == jsSceneSessionMap_.end()) {
            TLOGNE(WmsLogTag::WMS_LIFE, "%{public}s jsSceneSession id:%{public}d has been destroyed",
                where, persistentId);
            return;
        }
        auto jsCallBack = jsSceneSession->GetJSCallback(SUB_MODAL_TYPE_CHANGE_CB);
        if (!jsCallBack) {
            TLOGNE(WmsLogTag::WMS_HIERARCHY, "%{public}s jsCallBack is nullptr", where);
            return;
        }
        napi_value jsSessionModalTypeObj = CreateJsValue(env, subWindowModalType);
        napi_value argv[] = {jsSessionModalTypeObj};
        napi_call_function(env, NapiGetUndefined(env), jsCallBack->GetNapiValue(), ArraySize(argv), argv, nullptr);
    };
    taskScheduler_->PostMainThreadTask(task,
        "OnSubModalTypeChange: " + std::to_string(static_cast<uint32_t>(subWindowModalType)));
}

void JsSceneSession::OnMainModalTypeChange(bool isModal)
{
    const char* const where = __func__;
    auto task = [weakThis = wptr(this), persistentId = persistentId_, isModal, env = env_, where] {
        auto jsSceneSession = weakThis.promote();
        if (!jsSceneSession || jsSceneSessionMap_.find(persistentId) == jsSceneSessionMap_.end()) {
            TLOGNE(WmsLogTag::WMS_LIFE, "%{public}s jsSceneSession id:%{public}d has been destroyed",
                where, persistentId);
            return;
        }
        auto jsCallBack = jsSceneSession->GetJSCallback(MAIN_MODAL_TYPE_CHANGE_CB);
        if (!jsCallBack) {
            TLOGNE(WmsLogTag::WMS_HIERARCHY, "%{public}s jsCallBack is nullptr", where);
            return;
        }
        napi_value jsMainSessionModalType = CreateJsValue(env, isModal);
        napi_value argv[] = {jsMainSessionModalType};
        napi_call_function(env, NapiGetUndefined(env), jsCallBack->GetNapiValue(), ArraySize(argv), argv, nullptr);
    };
    taskScheduler_->PostMainThreadTask(task, "OnMainModalTypeChange: " + std::to_string(isModal));
}

void JsSceneSession::OnThrowSlipAnimationStateChange(bool isAnimating, bool isFullScreen)
{
    const char* const where = __func__;
    auto task = [weakThis = wptr(this), persistentId = persistentId_, isAnimating, isFullScreen, env = env_, where] {
        auto jsSceneSession = weakThis.promote();
        if (!jsSceneSession || jsSceneSessionMap_.find(persistentId) == jsSceneSessionMap_.end()) {
            TLOGNE(WmsLogTag::WMS_LAYOUT_PC, "%{public}s jsSceneSession id: %{public}d has been destroyed",
                where, persistentId);
            return;
        }
        auto jsCallBack = jsSceneSession->GetJSCallback(THROW_SLIP_ANIMATION_STATE_CHANGE_CB);
        if (!jsCallBack) {
            TLOGNE(WmsLogTag::WMS_LAYOUT_PC, "%{public}s jsCallBack is nullptr", where);
            return;
        }
        napi_value jsIsAnimating = CreateJsValue(env, isAnimating);
        napi_value jsIsFullScreen = CreateJsValue(env, isFullScreen);
        napi_value argv[] = { jsIsAnimating, jsIsFullScreen };
        napi_call_function(env, NapiGetUndefined(env), jsCallBack->GetNapiValue(), ArraySize(argv), argv, nullptr);
    };
    taskScheduler_->PostMainThreadTask(task, __func__);
}

void JsSceneSession::OnFullScreenWaterfallModeChange(bool isWaterfallMode)
{
    const char* const where = __func__;
    auto task = [weakThis = wptr(this), persistentId = persistentId_, isWaterfallMode, env = env_, where] {
        auto jsSceneSession = weakThis.promote();
        if (!jsSceneSession || jsSceneSessionMap_.find(persistentId) == jsSceneSessionMap_.end()) {
            TLOGNE(WmsLogTag::WMS_LAYOUT, "%{public}s jsSceneSession id: %{public}d has been destroyed",
                where, persistentId);
            return;
        }
        auto jsCallBack = jsSceneSession->GetJSCallback(FULLSCREEN_WATERFALL_MODE_CHANGE_CB);
        if (!jsCallBack) {
            TLOGNE(WmsLogTag::WMS_LAYOUT, "%{public}s jsCallBack is nullptr", where);
            return;
        }
        napi_value jsIsWaterfallMode = CreateJsValue(env, isWaterfallMode);
        napi_value argv[] = { jsIsWaterfallMode };
        napi_call_function(env, NapiGetUndefined(env), jsCallBack->GetNapiValue(), ArraySize(argv), argv, nullptr);
    };
    taskScheduler_->PostMainThreadTask(task, __func__);
}

void JsSceneSession::OnClick(bool requestFocus, bool isClick)
{
    TLOGD(WmsLogTag::WMS_FOCUS, "id: %{public}d, requestFocus: %{public}u, isClick: %{public}u",
        persistentId_, requestFocus, isClick);
    auto task = [weakThis = wptr(this), persistentId = persistentId_, requestFocus, isClick, env = env_] {
        auto jsSceneSession = weakThis.promote();
        if (!jsSceneSession || jsSceneSessionMap_.find(persistentId) == jsSceneSessionMap_.end()) {
            TLOGNE(WmsLogTag::WMS_LIFE, "OnClick jsSceneSession id:%{public}d has been destroyed",
                persistentId);
            return;
        }
        auto jsCallBack = jsSceneSession->GetJSCallback(CLICK_CB);
        if (!jsCallBack) {
            TLOGNE(WmsLogTag::WMS_LIFE, "jsCallBack is nullptr");
            return;
        }
        napi_value jsRequestFocusObj = CreateJsValue(env, requestFocus);
        napi_value jsIsClickObj = CreateJsValue(env, isClick);
        napi_value argv[] = {jsRequestFocusObj, jsIsClickObj};
        napi_call_function(env, NapiGetUndefined(env), jsCallBack->GetNapiValue(), ArraySize(argv), argv, nullptr);
    };
    taskScheduler_->PostMainThreadTask(task, "OnClick: requestFocus" + std::to_string(requestFocus));
}

void JsSceneSession::OnContextTransparent()
{
    TLOGD(WmsLogTag::DEFAULT, "[NAPI]");
    auto task = [weakThis = wptr(this), persistentId = persistentId_, env = env_] {
        auto jsSceneSession = weakThis.promote();
        if (!jsSceneSession || jsSceneSessionMap_.find(persistentId) == jsSceneSessionMap_.end()) {
            TLOGNE(WmsLogTag::WMS_LIFE, "OnContextTransparent jsSceneSession id:%{public}d has been destroyed",
                persistentId);
            return;
        }
        auto jsCallBack = jsSceneSession->GetJSCallback(CONTEXT_TRANSPARENT_CB);
        if (!jsCallBack) {
            TLOGNE(WmsLogTag::WMS_LIFE, "jsCallBack is nullptr");
            return;
        }
        napi_value argv[] = {};
        napi_call_function(env, NapiGetUndefined(env), jsCallBack->GetNapiValue(), 0, argv, nullptr);
    };
    taskScheduler_->PostMainThreadTask(task, "OnContextTransparent");
}

void JsSceneSession::ChangeSessionVisibilityWithStatusBar(const SessionInfo& info, bool visible)
{
    WLOGI("bundleName %{public}s, moduleName %{public}s, "
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
            TLOGNE(WmsLogTag::WMS_LIFE, "ChangeSessionVisibilityWithStatusBar jsSceneSession is null");
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
        WLOGFE("jsCallBack is nullptr");
        return;
    }
    if (sessionInfo == nullptr) {
        WLOGFE("sessionInfo is nullptr");
        return;
    }
    napi_value jsSessionInfo = CreateJsSessionInfo(env_, *sessionInfo);
    if (jsSessionInfo == nullptr) {
        WLOGFE("target session info is nullptr");
        return;
    }
    napi_value visibleNapiV = CreateJsValue(env_, visible);
    napi_value argv[] = {jsSessionInfo, visibleNapiV};
    napi_call_function(env_, NapiGetUndefined(env_),
        jsCallBack->GetNapiValue(), ArraySize(argv), argv, nullptr);
}

sptr<SceneSession> JsSceneSession::GenSceneSession(SessionInfo& info, bool needAddRequestInfo)
{
    sptr<SceneSession> sceneSession = nullptr;
    if (info.persistentId_ == 0) {
        auto result = SceneSessionManager::GetInstance().CheckIfReuseSession(info);
        if (result == BrokerStates::BROKER_NOT_START) {
            TLOGE(WmsLogTag::WMS_LIFE, "BrokerStates not started");
            return nullptr;
        }
        if (result == BrokerStates::BROKER_STARTED && info.collaboratorType_ == CollaboratorType::REDIRECT_TYPE) {
            TLOGW(WmsLogTag::WMS_LIFE, "redirect and not create session.");
            return nullptr;
        }
        ReuseSession(sceneSession, info);
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
    AddRequestTaskInfo(sceneSession, info.requestId, needAddRequestInfo);
    return sceneSession;
}

void JsSceneSession::ReuseSession(sptr<SceneSession>& sceneSession, SessionInfo& info)
{
    if (info.reuse || info.isAtomicService_ || !info.specifiedFlag_.empty()) {
        TLOGI(WmsLogTag::WMS_LIFE, "session need to be reusesd.");
        if (SceneSessionManager::GetInstance().CheckCollaboratorType(info.collaboratorType_)) {
            sceneSession = SceneSessionManager::GetInstance().FindSessionByAffinity(info.sessionAffinity);
        } else {
            SessionIdentityInfo identityInfo = { info.bundleName_, info.moduleName_, info.abilityName_,
                info.appIndex_, info.appInstanceKey_, info.windowType_, info.isAtomicService_,
                info.specifiedFlag_ };
            sceneSession = SceneSessionManager::GetInstance().GetSceneSessionByIdentityInfo(identityInfo);
        }
    }
    if (sceneSession == nullptr) {
        TLOGI(WmsLogTag::WMS_LIFE, "SceneSession not exist, request a new one.");
        sceneSession = SceneSessionManager::GetInstance().RequestSceneSession(info);
        if (sceneSession == nullptr) {
            TLOGE(WmsLogTag::WMS_LIFE, "RequestSceneSession return nullptr");
            return;
        }
    } else {
        sceneSession->SetSessionInfo(info);
    }
    info.persistentId_ = sceneSession->GetPersistentId();
    sceneSession->SetSessionInfoPersistentId(sceneSession->GetPersistentId());
}

void JsSceneSession::PendingSessionActivation(SessionInfo& info)
{
    TLOGI(WmsLogTag::WMS_LIFE, "id:%{public}d,bundleName:%{public}s,moduleName:%{public}s,abilityName:%{public}s,"
        "appIndex:%{public}d,reuse:%{public}d,requestId:%{public}d,specifiedFlag:%{public}s",
        info.persistentId_, info.bundleName_.c_str(), info.moduleName_.c_str(), info.abilityName_.c_str(),
        info.appIndex_, info.reuse, info.requestId, info.specifiedFlag_.c_str());
    auto sceneSession = GenSceneSession(info, true);
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
        info.isCalledRightlyByCallerId_ = (info.callerToken_ == callerSession->GetAbilityToken()) &&
            SessionPermission::VerifyPermissionByBundleName(info.bundleName_,
                                                            "ohos.permission.CALLED_TRANSITION_ON_LOCK_SCREEN",
                                                            SceneSessionManager::GetInstance().GetCurrentUserId());
        TLOGI(WmsLogTag::WMS_SCB,
            "isCalledRightlyByCallerId result is: %{public}d", info.isCalledRightlyByCallerId_);
    }
    std::shared_ptr<SessionInfo> sessionInfo = std::make_shared<SessionInfo>(info);
    auto task = [weakThis = wptr(this), sessionInfo] {
        auto jsSceneSession = weakThis.promote();
        if (jsSceneSession == nullptr) {
            TLOGNE(WmsLogTag::WMS_LIFE, "PendingSessionActivation JsSceneSession is null");
            SceneSessionManager::GetInstance().RemoveLifeCycleTaskByPersistentId(
                sessionInfo->persistentId_, LifeCycleTaskType::START);
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
    const char* const where = __func__;
    auto task = [weakThis = wptr(this), persistentId = persistentId_, weakSession = weakSession_,
        sessionInfo, env = env_, where] {
        auto session = weakSession.promote();
        if (session == nullptr) {
            TLOGNE(WmsLogTag::WMS_LIFE, "session is nullptr");
            SceneSessionManager::GetInstance().RemoveLifeCycleTaskByPersistentId(
                sessionInfo->persistentId_, LifeCycleTaskType::START);
            return;
        }
        auto jsSceneSession = weakThis.promote();
        if (!jsSceneSession || jsSceneSessionMap_.find(persistentId) == jsSceneSessionMap_.end()) {
            TLOGNE(WmsLogTag::WMS_LIFE, "jsSceneSession id:%{public}d has been destroyed", persistentId);
            SceneSessionManager::GetInstance().RemoveLifeCycleTaskByPersistentId(
                sessionInfo->persistentId_, LifeCycleTaskType::START);
            return;
        }
        auto jsCallBack = jsSceneSession->GetJSCallback(PENDING_SCENE_CB);
        if (!jsCallBack) {
            TLOGNE(WmsLogTag::WMS_LIFE, "jsCallBack is nullptr");
            SceneSessionManager::GetInstance().RemoveLifeCycleTaskByPersistentId(
                sessionInfo->persistentId_, LifeCycleTaskType::START);
            return;
        }

        napi_value jsSessionInfo = CreateJsSessionInfo(env, *sessionInfo);
        if (jsSessionInfo == nullptr) {
            TLOGNE(WmsLogTag::WMS_LIFE, "target session info is nullptr");
            SceneSessionManager::GetInstance().RemoveLifeCycleTaskByPersistentId(
                sessionInfo->persistentId_, LifeCycleTaskType::START);
            return;
        }
        napi_value argv[] = {jsSessionInfo};
        TLOGNI(WmsLogTag::WMS_LIFE, "%{public}s task success, id:%{public}d, requestId:%{public}d",
            where, sessionInfo->persistentId_, sessionInfo->requestId);
        napi_value callResult = nullptr;
        napi_call_function(env, NapiGetUndefined(env),
            jsCallBack->GetNapiValue(), ArraySize(argv), argv, &callResult);
        SceneSessionManager::GetInstance().RemoveLifeCycleTaskByPersistentId(
            sessionInfo->persistentId_, LifeCycleTaskType::START);
        ProcessPendingSessionActivationResult(env, callResult, sessionInfo);
    };
    taskScheduler_->PostMainThreadTask(task, "PendingSessionActivationInner");
}

napi_value JsSceneSession::CreateSessionInfosNapiValue(
    napi_env env, const std::vector<std::shared_ptr<SessionInfo>>& sessionInfos)
{
    napi_value arrayValue = nullptr;
    napi_create_array_with_length(env, sessionInfos.size(), &arrayValue);
 
    if (arrayValue == nullptr) {
        TLOGE(WmsLogTag::WMS_LIFE, "Failed to create napi array");
        return NapiGetUndefined(env);
    }
 
    int32_t index = 0;
    for (const auto& sessionInfo : sessionInfos) {
        napi_value objValue = nullptr;
        napi_create_object(env, &objValue);
        if (objValue == nullptr) {
            TLOGE(WmsLogTag::WMS_LIFE, "failed to create napi object");
            return NapiGetUndefined(env);
        }
        napi_set_element(env, arrayValue, index++, CreateJsSessionInfo(env, *sessionInfo));
    }
    return arrayValue;
}

napi_value JsSceneSession::CreatePendingInfosNapiValue(napi_env env,
    const std::vector<std::shared_ptr<SessionInfo>>& sessionInfos,
    const std::vector<std::shared_ptr<PendingSessionActivationConfig>>& configs)
{
    if (configs.empty()) {
        return CreateSessionInfosNapiValue(env, sessionInfos);
    }

    if (sessionInfos.size() != configs.size()) {
        TLOGE(WmsLogTag::WMS_LIFE,
            "The caller Param is illegal parameters.sessionInfo: %{public}zu configs: %{public}zu",
            sessionInfos.size(), configs.size());
        return NapiGetUndefined(env);
    }

    napi_value arrayValue = nullptr;
    napi_create_array_with_length(env, sessionInfos.size(), &arrayValue);
    if (arrayValue == nullptr) {
        TLOGE(WmsLogTag::WMS_LIFE, "Failed to create napi array");
        return NapiGetUndefined(env);
    }
 
    int32_t index = 0;
    for (size_t i = 0; i < sessionInfos.size(); i++) {
        napi_value objValue = nullptr;
        napi_create_object(env, &objValue);
        if (objValue == nullptr) {
            TLOGE(WmsLogTag::WMS_LIFE, "failed to create napi object");
            return NapiGetUndefined(env);
        }
        napi_set_element(env, arrayValue, index++, CreateJsSessionInfo(env, *(sessionInfos[i]), configs[i]));
    }
    return arrayValue;
}
 
void JsSceneSession::BatchPendingSessionsActivation(const std::vector<std::shared_ptr<SessionInfo>>& sessionInfos,
    const std::vector<std::shared_ptr<PendingSessionActivationConfig>>& configs)
{
    if (!configs.empty() && sessionInfos.size() != configs.size()) {
        TLOGE(WmsLogTag::WMS_LIFE,
            "The caller Param is illegal parameters.sessionInfo: %{public}zu configs: %{public}zu",
            sessionInfos.size(), configs.size());
        return;
    }

    std::vector<sptr<SceneSession>> sceneSessions;
    for (auto& info : sessionInfos) {
        if (info == nullptr) {
            TLOGE(WmsLogTag::WMS_LIFE, "sessioninfo is null");
            return;
        }
        TLOGI(WmsLogTag::WMS_LIFE, "bundleName %{public}s, moduleName %{public}s, abilityName %{public}s, "
            "appIndex %{public}d, reuse %{public}d, requestId %{public}d, specifiedFlag %{public}s",
            info->bundleName_.c_str(), info->moduleName_.c_str(),
            info->abilityName_.c_str(), info->appIndex_, info->reuse, info->requestId, info->specifiedFlag_.c_str());
        auto callerSession = SceneSessionManager::GetInstance().GetSceneSession(info->callerPersistentId_);
        if (callerSession != nullptr) {
            info->isCalledRightlyByCallerId_ = (info->callerToken_ == callerSession->GetAbilityToken()) &&
                SessionPermission::VerifyPermissionByBundleName(info->bundleName_,
                                                                "ohos.permission.CALLED_TRANSITION_ON_LOCK_SCREEN",
                                                                SceneSessionManager::GetInstance().GetCurrentUserId());
            TLOGI(WmsLogTag::WMS_SCB,
                "isCalledRightlyByCallerId result is: %{public}d", info->isCalledRightlyByCallerId_);
        }
    }
    BatchPendingSessionsActivationInner(sessionInfos, configs);
}

void JsSceneSession::BatchPendingSessionsActivationInner(const std::vector<std::shared_ptr<SessionInfo>>& sessionInfos,
    const std::vector<std::shared_ptr<PendingSessionActivationConfig>>& configs)
{
    const char* const where = __func__;
    auto task = [weakThis = wptr(this), persistentId = persistentId_, weakSession = weakSession_,
        sessionInfos, configs, env = env_, where] {
        auto session = weakSession.promote();
        if (session == nullptr) {
            TLOGNE(WmsLogTag::WMS_LIFE, "session is nullptr");
            return;
        }
        auto jsSceneSession = weakThis.promote();
        if (!jsSceneSession || jsSceneSessionMap_.find(persistentId) == jsSceneSessionMap_.end()) {
            TLOGNE(WmsLogTag::WMS_LIFE, "jsSceneSession id:%{public}d has been destroyed", persistentId);
            return;
        }
        auto jsCallBack = jsSceneSession->GetJSCallback(BATCH_PENDING_SCENE_ACTIVE_CB);
        if (!jsCallBack) {
            TLOGNE(WmsLogTag::WMS_LIFE, "jsCallBack is nullptr");
            return;
        }
        napi_value jsSessionInfos = CreatePendingInfosNapiValue(env, sessionInfos, configs);
        if (jsSessionInfos == nullptr) {
            TLOGNE(WmsLogTag::WMS_LIFE, "target session info is nullptr");
            return;
        }
        napi_value argv[] = { jsSessionInfos };
        TLOGNI(WmsLogTag::WMS_LIFE, "%{public}s task success, id:%{public}d", where, persistentId);
        napi_call_function(env, NapiGetUndefined(env),
            jsCallBack->GetNapiValue(), ArraySize(argv), argv, nullptr);
    };
    taskScheduler_->PostMainThreadTask(task, "BatchPendingSessionsActivationInner");
}

void JsSceneSession::OnBackPressed(bool needMoveToBackground)
{
    TLOGI(WmsLogTag::DEFAULT, "%{public}d", needMoveToBackground);
    auto task = [weakThis = wptr(this), persistentId = persistentId_, needMoveToBackground, env = env_] {
        auto jsSceneSession = weakThis.promote();
        if (!jsSceneSession || jsSceneSessionMap_.find(persistentId) == jsSceneSessionMap_.end()) {
            TLOGNE(WmsLogTag::WMS_LIFE, "OnBackPressed jsSceneSession id:%{public}d has been destroyed",
                persistentId);
            return;
        }
        auto jsCallBack = jsSceneSession->GetJSCallback(BACK_PRESSED_CB);
        if (!jsCallBack) {
            TLOGNE(WmsLogTag::WMS_LIFE, "jsCallBack is nullptr");
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
            TLOGNE(WmsLogTag::WMS_LIFE, "TerminateSession jsSceneSession id:%{public}d has been destroyed",
                persistentId);
            SceneSessionManager::GetInstance().RemoveLifeCycleTaskByPersistentId(
                persistentId, LifeCycleTaskType::STOP);
            return;
        }
        auto jsCallBack = jsSceneSession->GetJSCallback(TERMINATE_SESSION_CB);
        if (!jsCallBack) {
            TLOGNE(WmsLogTag::WMS_LIFE, "jsCallBack is nullptr");
            SceneSessionManager::GetInstance().RemoveLifeCycleTaskByPersistentId(
                persistentId, LifeCycleTaskType::STOP);
            return;
        }
        napi_value jsSessionInfo = CreateJsSessionInfo(env, *sessionInfo);
        if (jsSessionInfo == nullptr) {
            TLOGNE(WmsLogTag::WMS_LIFE, "target session info is nullptr");
            SceneSessionManager::GetInstance().RemoveLifeCycleTaskByPersistentId(
                persistentId, LifeCycleTaskType::STOP);
            return;
        }
        napi_value argv[] = {jsSessionInfo};
        napi_call_function(env, NapiGetUndefined(env), jsCallBack->GetNapiValue(), ArraySize(argv), argv, nullptr);
        SceneSessionManager::GetInstance().RemoveLifeCycleTaskByPersistentId(
            persistentId, LifeCycleTaskType::STOP);
    };
    taskScheduler_->PostMainThreadTask(task, "TerminateSession name:" + info.abilityName_);
}

void JsSceneSession::TerminateSessionNew(const SessionInfo& info, bool needStartCaller,
    bool isFromBroker, bool isForceClean)
{
    TLOGI(WmsLogTag::WMS_LIFE, "bundleName=%{public}s, abilityName=%{public}s",
        info.bundleName_.c_str(), info.abilityName_.c_str());
    bool needRemoveSession = !needStartCaller && !isFromBroker;
    auto task = [weakThis = wptr(this), persistentId = persistentId_, needStartCaller,
        needRemoveSession, isForceClean, env = env_] {
        auto jsSceneSession = weakThis.promote();
        if (!jsSceneSession || jsSceneSessionMap_.find(persistentId) == jsSceneSessionMap_.end()) {
            TLOGNE(WmsLogTag::WMS_LIFE, "TerminateSessionNew jsSceneSession id:%{public}d has been destroyed",
                persistentId);
            SceneSessionManager::GetInstance().RemoveLifeCycleTaskByPersistentId(
                persistentId, LifeCycleTaskType::STOP);
            return;
        }
        auto jsCallBack = jsSceneSession->GetJSCallback(TERMINATE_SESSION_CB_NEW);
        if (!jsCallBack) {
            TLOGNE(WmsLogTag::WMS_LIFE, "jsCallBack is nullptr");
            SceneSessionManager::GetInstance().RemoveLifeCycleTaskByPersistentId(
                persistentId, LifeCycleTaskType::STOP);
            return;
        }
        napi_value jsNeedStartCaller = CreateJsValue(env, needStartCaller);
        if (jsNeedStartCaller == nullptr) {
            TLOGNE(WmsLogTag::WMS_LIFE, "jsNeedStartCaller is nullptr");
            SceneSessionManager::GetInstance().RemoveLifeCycleTaskByPersistentId(
                persistentId, LifeCycleTaskType::STOP);
            return;
        }
        napi_value jsNeedRemoveSession = CreateJsValue(env, needRemoveSession);
        if (jsNeedRemoveSession == nullptr) {
            TLOGNE(WmsLogTag::WMS_LIFE, "jsNeedRemoveSession is nullptr");
            SceneSessionManager::GetInstance().RemoveLifeCycleTaskByPersistentId(
                persistentId, LifeCycleTaskType::STOP);
            return;
        }
        napi_value jsIsForceClean = CreateJsValue(env, isForceClean);
        if (jsIsForceClean == nullptr) {
            TLOGNE(WmsLogTag::WMS_LIFE, "jsIsForceClean is nullptr");
            SceneSessionManager::GetInstance().RemoveLifeCycleTaskByPersistentId(
                persistentId, LifeCycleTaskType::STOP);
            return;
        }
        napi_value argv[] = {jsNeedStartCaller, jsNeedRemoveSession, jsIsForceClean};
        napi_call_function(env, NapiGetUndefined(env), jsCallBack->GetNapiValue(), ArraySize(argv), argv, nullptr);
        SceneSessionManager::GetInstance().RemoveLifeCycleTaskByPersistentId(
            persistentId, LifeCycleTaskType::STOP);
    };
    taskScheduler_->PostMainThreadTask(task, "TerminateSessionNew, name:" + info.abilityName_);
}

void JsSceneSession::TerminateSessionTotal(const SessionInfo& info, TerminateType terminateType)
{
    TLOGI(WmsLogTag::WMS_LIFE, "bundleName=%{public}s, abilityName=%{public}s, terminateType=%{public}d",
        info.bundleName_.c_str(), info.abilityName_.c_str(), static_cast<int32_t>(terminateType));

    auto task = [weakThis = wptr(this), persistentId = persistentId_, terminateType, env = env_] {
        auto jsSceneSession = weakThis.promote();
        if (!jsSceneSession || jsSceneSessionMap_.find(persistentId) == jsSceneSessionMap_.end()) {
            TLOGNE(WmsLogTag::WMS_LIFE, "TerminateSessionTotal jsSceneSession id:%{public}d has been destroyed",
                persistentId);
            return;
        }
        auto jsCallBack = jsSceneSession->GetJSCallback(TERMINATE_SESSION_CB_TOTAL);
        if (!jsCallBack) {
            TLOGNE(WmsLogTag::WMS_LIFE, "jsCallBack is nullptr");
            return;
        }
        napi_value jsTerminateType = CreateJsValue(env, static_cast<int32_t>(terminateType));
        if (jsTerminateType == nullptr) {
            TLOGNE(WmsLogTag::WMS_LIFE, "jsTerminateType is nullptr");
            return;
        }
        napi_value argv[] = {jsTerminateType};
        napi_call_function(env, NapiGetUndefined(env), jsCallBack->GetNapiValue(), ArraySize(argv), argv, nullptr);
    };
    taskScheduler_->PostMainThreadTask(task, "TerminateSessionTotal:name:" + info.abilityName_);
}

void JsSceneSession::UpdateSessionLabel(const std::string& label)
{
    TLOGD(WmsLogTag::DEFAULT, "[NAPI]");
    auto task = [weakThis = wptr(this), persistentId = persistentId_, label, env = env_] {
        auto jsSceneSession = weakThis.promote();
        if (!jsSceneSession || jsSceneSessionMap_.find(persistentId) == jsSceneSessionMap_.end()) {
            TLOGNE(WmsLogTag::WMS_LIFE, "UpdateSessionLabel jsSceneSession id:%{public}d has been destroyed",
                persistentId);
            return;
        }
        auto jsCallBack = jsSceneSession->GetJSCallback(UPDATE_SESSION_LABEL_CB);
        if (!jsCallBack) {
            TLOGNE(WmsLogTag::WMS_LIFE, "jsCallBack is nullptr");
            return;
        }
        napi_value jsLabel = CreateJsValue(env, label);
        if (jsLabel == nullptr) {
            TLOGNE(WmsLogTag::WMS_LIFE, "this target jsLabel is nullptr");
            return;
        }
        napi_value argv[] = {jsLabel};
        napi_call_function(env, NapiGetUndefined(env), jsCallBack->GetNapiValue(), ArraySize(argv), argv, nullptr);
    };
    taskScheduler_->PostMainThreadTask(task, "UpdateSessionLabel");
}

void JsSceneSession::ProcessUpdateSessionLabelRegister()
{
    TLOGD(WmsLogTag::DEFAULT, "in");
    NofitySessionLabelUpdatedFunc func = [weakThis = wptr(this)](const std::string& label) {
        auto jsSceneSession = weakThis.promote();
        if (!jsSceneSession) {
            TLOGNE(WmsLogTag::WMS_LIFE, "ProcessUpdateSessionLabelRegister jsSceneSession is null");
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
    TLOGD(WmsLogTag::DEFAULT, "success");
}

void JsSceneSession::ProcessUpdateSessionIconRegister()
{
    TLOGD(WmsLogTag::DEFAULT, "in");
    NofitySessionIconUpdatedFunc func = [weakThis = wptr(this)](const std::string& iconPath) {
        auto jsSceneSession = weakThis.promote();
        if (!jsSceneSession) {
            TLOGNE(WmsLogTag::WMS_LIFE, "ProcessUpdateSessionIconRegister jsSceneSession is null");
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
    TLOGD(WmsLogTag::DEFAULT, "success");
}

void JsSceneSession::UpdateSessionIcon(const std::string& iconPath)
{
    TLOGD(WmsLogTag::DEFAULT, "[NAPI]");
    auto task = [weakThis = wptr(this), persistentId = persistentId_, iconPath, env = env_] {
        auto jsSceneSession = weakThis.promote();
        if (!jsSceneSession || jsSceneSessionMap_.find(persistentId) == jsSceneSessionMap_.end()) {
            TLOGNE(WmsLogTag::WMS_LIFE, "UpdateSessionIcon jsSceneSession id:%{public}d has been destroyed",
                persistentId);
            return;
        }
        auto jsCallBack = jsSceneSession->GetJSCallback(UPDATE_SESSION_ICON_CB);
        if (!jsCallBack) {
            TLOGNE(WmsLogTag::WMS_LIFE, "jsCallBack is nullptr");
            return;
        }
        napi_value jsIconPath = CreateJsValue(env, iconPath);
        if (jsIconPath == nullptr) {
            TLOGNE(WmsLogTag::WMS_LIFE, "this target jsIconPath is nullptr");
            return;
        }
        napi_value argv[] = {jsIconPath};
        napi_call_function(env, NapiGetUndefined(env), jsCallBack->GetNapiValue(), ArraySize(argv), argv, nullptr);
    };
    taskScheduler_->PostMainThreadTask(task, "UpdateSessionIcon");
}

void JsSceneSession::OnSessionException(const SessionInfo& info, const ExceptionInfo& exceptionInfo, bool startFail)
{
    TLOGI(WmsLogTag::WMS_LIFE, "bundleName=%{public}s, abilityName=%{public}s, startFail=%{public}d",
        info.bundleName_.c_str(), info.abilityName_.c_str(), startFail);

    std::shared_ptr<SessionInfo> sessionInfo = std::make_shared<SessionInfo>(info);
    auto task = [weakThis = wptr(this), persistentId = persistentId_,
        sessionInfo, exceptionInfo, startFail, env = env_] {
        auto jsSceneSession = weakThis.promote();
        if (!jsSceneSession || jsSceneSessionMap_.find(persistentId) == jsSceneSessionMap_.end()) {
            TLOGNE(WmsLogTag::WMS_LIFE, "OnSessionException jsSceneSession id:%{public}d has been destroyed",
                persistentId);
            SceneSessionManager::GetInstance().RemoveLifeCycleTaskByPersistentId(
                persistentId, LifeCycleTaskType::STOP);
            return;
        }
        auto jsCallBack = jsSceneSession->GetJSCallback(SESSION_EXCEPTION_CB);
        if (!jsCallBack) {
            TLOGNE(WmsLogTag::WMS_LIFE, "jsCallBack is nullptr");
            SceneSessionManager::GetInstance().RemoveLifeCycleTaskByPersistentId(
                persistentId, LifeCycleTaskType::STOP);
            return;
        }
        napi_value jsSessionInfo = CreateJsSessionInfo(env, *sessionInfo);
        napi_value jsExceptionInfo = CreateJsExceptionInfo(env, exceptionInfo);
        napi_value jsStartFail = CreateJsValue(env, startFail);
        if (jsSessionInfo == nullptr || jsExceptionInfo == nullptr) {
            TLOGNE(WmsLogTag::WMS_LIFE, "target session info or exception info is nullptr");
            SceneSessionManager::GetInstance().RemoveLifeCycleTaskByPersistentId(
                persistentId, LifeCycleTaskType::STOP);
            return;
        }
        napi_value argv[] = {jsSessionInfo, jsExceptionInfo, jsStartFail};
        napi_call_function(env, NapiGetUndefined(env), jsCallBack->GetNapiValue(), ArraySize(argv), argv, nullptr);
        SceneSessionManager::GetInstance().RemoveLifeCycleTaskByPersistentId(
            persistentId, LifeCycleTaskType::STOP);
    };
    taskScheduler_->PostMainThreadTask(task, "OnSessionException, name" + info.bundleName_);
}

void JsSceneSession::PendingSessionToForeground(const SessionInfo& info)
{
    TLOGI(WmsLogTag::WMS_LIFE, "bundleName=%{public}s, abilityName=%{public}s",
        info.bundleName_.c_str(), info.abilityName_.c_str());

    std::shared_ptr<SessionInfo> sessionInfo = std::make_shared<SessionInfo>(info);
    auto task = [weakThis = wptr(this), persistentId = persistentId_, sessionInfo, env = env_] {
        auto jsSceneSession = weakThis.promote();
        if (!jsSceneSession || jsSceneSessionMap_.find(persistentId) == jsSceneSessionMap_.end()) {
            TLOGNE(WmsLogTag::WMS_LIFE, "PendingSessionToForeground jsSceneSession id:%{public}d has been destroyed",
                persistentId);
            return;
        }
        auto jsCallBack = jsSceneSession->GetJSCallback(PENDING_SESSION_TO_FOREGROUND_CB);
        if (!jsCallBack) {
            TLOGNE(WmsLogTag::WMS_LIFE, "jsCallBack is nullptr");
            return;
        }
        if (sessionInfo == nullptr) {
            TLOGNE(WmsLogTag::WMS_LIFE, "sessionInfo is nullptr");
            return;
        }
        napi_value jsSessionInfo = CreateJsSessionInfo(env, *sessionInfo);
        if (jsSessionInfo == nullptr) {
            TLOGNE(WmsLogTag::WMS_LIFE, "target session info is nullptr");
            return;
        }
        napi_value argv[] = {jsSessionInfo};
        napi_call_function(env, NapiGetUndefined(env), jsCallBack->GetNapiValue(), ArraySize(argv), argv, nullptr);
    };
    taskScheduler_->PostMainThreadTask(task, "PendingSessionToForeground:" + info.bundleName_);
}

void JsSceneSession::PendingSessionToBackground(const SessionInfo& info, const BackgroundParams& params)
{
    TLOGI(WmsLogTag::WMS_LIFE, "bundleName=%{public}s, abilityName=%{public}s, shouldBackToCaller=%{public}d",
        info.bundleName_.c_str(), info.abilityName_.c_str(), params.shouldBackToCaller);
    std::shared_ptr<SessionInfo> sessionInfo = std::make_shared<SessionInfo>(info);
    auto task = [weakThis = wptr(this), persistentId = persistentId_, sessionInfo, env = env_, params] {
        auto jsSceneSession = weakThis.promote();
        if (!jsSceneSession || jsSceneSessionMap_.find(persistentId) == jsSceneSessionMap_.end()) {
            TLOGNE(WmsLogTag::WMS_LIFE, "jsSceneSession id:%{public}d has been destroyed", persistentId);
            return;
        }
        auto jsCallBack = jsSceneSession->GetJSCallback(PENDING_SESSION_TO_BACKGROUND_CB);
        if (!jsCallBack) {
            TLOGNE(WmsLogTag::WMS_LIFE, "jsCallBack is nullptr");
            return;
        }
        napi_value jsSessionInfo = CreateJsSessionInfo(env, *sessionInfo);
        napi_value jsShouldBackToCaller = CreateJsValue(env, params.shouldBackToCaller);
        napi_value jsWantParams = OHOS::AppExecFwk::WrapWantParams(env, params.wantParams);
        if (jsSessionInfo == nullptr) {
            TLOGNE(WmsLogTag::WMS_LIFE, "target session info is nullptr");
            return;
        }
        napi_value argv[] = {jsSessionInfo, jsShouldBackToCaller, jsWantParams};
        napi_call_function(env, NapiGetUndefined(env), jsCallBack->GetNapiValue(), ArraySize(argv), argv, nullptr);
    };
    taskScheduler_->PostMainThreadTask(task, "PendingSessionToBackground, name:" + info.bundleName_);
}

void JsSceneSession::PendingSessionToBackgroundForDelegator(const SessionInfo& info, bool shouldBackToCaller)
{
    TLOGI(WmsLogTag::WMS_LIFE,
        "bundleName=%{public}s, abilityName=%{public}s, shouldBackToCaller=%{public}d",
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
            TLOGNE(WmsLogTag::WMS_LIFE, "jsCallBack is nullptr");
            return;
        }
        napi_value jsSessionInfo = CreateJsSessionInfo(env, *sessionInfo);
        napi_value jsShouldBackToCaller = CreateJsValue(env, shouldBackToCaller);
        if (jsSessionInfo == nullptr) {
            TLOGNE(WmsLogTag::WMS_LIFE, "target session info is nullptr");
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
    const char* const where = __func__;
    auto task = [weakThis = wptr(this), persistentId = persistentId_, propertyMap, env = env_, where] {
        auto jsSceneSession = weakThis.promote();
        if (!jsSceneSession || jsSceneSessionMap_.find(persistentId) == jsSceneSessionMap_.end()) {
            TLOGNE(WmsLogTag::WMS_LIFE, "OnSystemBarPropertyChange jsSceneSession id:%{public}d has been destroyed",
                persistentId);
            return;
        }
        auto jsCallBack = jsSceneSession->GetJSCallback(SYSTEMBAR_PROPERTY_CHANGE_CB);
        if (jsCallBack == nullptr) {
            TLOGNE(WmsLogTag::WMS_LIFE, "jsCallBack is nullptr");
            return;
        }
        napi_value jsArrayObject = CreateJsSystemBarPropertyArrayObject(env, propertyMap);
        if (jsArrayObject == nullptr) {
            TLOGNE(WmsLogTag::WMS_IMMS, "%{public}s jsArrayObject is nullptr", where);
            return;
        }
        napi_value argv[] = {jsArrayObject};
        napi_call_function(env, NapiGetUndefined(env), jsCallBack->GetNapiValue(), ArraySize(argv), argv, nullptr);
    };
    taskScheduler_->PostMainThreadTask(task, "OnSystemBarPropertyChange");
}

void JsSceneSession::OnNeedAvoid(bool status)
{
    TLOGD(WmsLogTag::WMS_IMMS, "%{public}d", status);
    auto task = [weakThis = wptr(this), persistentId = persistentId_, needAvoid = status, env = env_] {
        auto jsSceneSession = weakThis.promote();
        if (!jsSceneSession || jsSceneSessionMap_.find(persistentId) == jsSceneSessionMap_.end()) {
            TLOGNE(WmsLogTag::WMS_LIFE, "OnNeedAvoid jsSceneSession id:%{public}d has been destroyed",
                persistentId);
            return;
        }
        auto jsCallBack = jsSceneSession->GetJSCallback(NEED_AVOID_CB);
        if (jsCallBack == nullptr) {
            TLOGNE(WmsLogTag::WMS_LIFE, "jsCallBack is nullptr");
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
    TLOGI(WmsLogTag::DEFAULT, "%{public}d", status);
    auto task = [weakThis = wptr(this), persistentId = persistentId_, isPlaying = status, env = env_] {
        auto jsSceneSession = weakThis.promote();
        if (!jsSceneSession || jsSceneSessionMap_.find(persistentId) == jsSceneSessionMap_.end()) {
            TLOGNE(WmsLogTag::WMS_LIFE, "OnIsCustomAnimationPlaying jsSceneSession id:%{public}d has been destroyed",
                persistentId);
            return;
        }
        auto jsCallBack = jsSceneSession->GetJSCallback(CUSTOM_ANIMATION_PLAYING_CB);
        if (jsCallBack == nullptr) {
            TLOGNE(WmsLogTag::WMS_LIFE, "jsCallBack is nullptr");
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
    TLOGI(WmsLogTag::DEFAULT, "%{public}d", showWhenLocked);
    auto task = [weakThis = wptr(this), persistentId = persistentId_, flag = showWhenLocked, env = env_] {
        auto jsSceneSession = weakThis.promote();
        if (!jsSceneSession || jsSceneSessionMap_.find(persistentId) == jsSceneSessionMap_.end()) {
            TLOGNE(WmsLogTag::WMS_LIFE, "OnShowWhenLocked jsSceneSession id:%{public}d has been destroyed",
                persistentId);
            return;
        }
        auto jsCallBack = jsSceneSession->GetJSCallback(SHOW_WHEN_LOCKED_CB);
        if (jsCallBack == nullptr) {
            TLOGNE(WmsLogTag::WMS_LIFE, "jsCallBack is nullptr");
            return;
        }
        napi_value jsSessionStateObj = CreateJsValue(env, flag);
        napi_value argv[] = {jsSessionStateObj};
        napi_call_function(env, NapiGetUndefined(env), jsCallBack->GetNapiValue(), ArraySize(argv), argv, nullptr);
    };
    taskScheduler_->PostMainThreadTask(task, "OnShowWhenLocked:" +std::to_string(showWhenLocked));
}

void JsSceneSession::OnReuqestedOrientationChange(uint32_t orientation, bool needAnimation)
{
    auto task =
    [weakThis = wptr(this), persistentId = persistentId_, rotation = orientation, needAnimation, env = env_] {
        auto jsSceneSession = weakThis.promote();
        if (!jsSceneSession || jsSceneSessionMap_.find(persistentId) == jsSceneSessionMap_.end()) {
            TLOGNE(WmsLogTag::WMS_ROTATION,
                   "OnReuqestedOrientationChange jsSceneSession id:%{public}d has been destroyed",
                   persistentId);
            return;
        }
        auto jsCallBack = jsSceneSession->GetJSCallback(REQUESTED_ORIENTATION_CHANGE_CB);
        if (jsCallBack == nullptr) {
            TLOGNE(WmsLogTag::WMS_LIFE, "jsCallBack is nullptr");
            return;
        }
        napi_value rotationValue = CreateJsValue(env, rotation);
        napi_value animationValue = CreateJsValue(env, needAnimation);
        napi_value argv[] = { rotationValue, animationValue };
        napi_call_function(env, NapiGetUndefined(env), jsCallBack->GetNapiValue(), ArraySize(argv), argv, nullptr);
        TLOGI(WmsLogTag::DEFAULT, "%{public}u", rotation);
    };
    std::string taskName;
    if (needAnimation) {
        taskName = "OnReuqestedOrientationChange:orientation";
    } else {
        taskName = "OnReuqestedOrientationChange:pageOrientation";
    }
    taskScheduler_->RemoveMainThreadTaskByName(taskName);
    taskScheduler_->PostMainThreadTask(task, taskName);
}

void JsSceneSession::OnGetTargetOrientationConfigInfo(uint32_t targetOrientation)
{
    auto task = [weakThis = wptr(this), persistentId = persistentId_, targetOrientation, env = env_] {
        auto jsSceneSession = weakThis.promote();
        if (!jsSceneSession || jsSceneSessionMap_.find(persistentId) == jsSceneSessionMap_.end()) {
            TLOGNE(WmsLogTag::WMS_ROTATION,
                "jsSceneSession id:%{public}d has been destroyed", persistentId);
            return;
        }
        auto jsCallBack = jsSceneSession->GetJSCallback(SESSION_GET_TARGET_ORIENTATION_CONFIG_INFO_CB);
        if (!jsCallBack) {
            TLOGNE(WmsLogTag::WMS_ROTATION, "jsCallBack is nullptr");
            return;
        }
        napi_value orientationValue = CreateJsValue(env, targetOrientation);
        napi_value argv[] = {orientationValue};
        napi_call_function(env, NapiGetUndefined(env), jsCallBack->GetNapiValue(), ArraySize(argv), argv, nullptr);
        TLOGNI(WmsLogTag::WMS_ROTATION, "Get target orientation(%{public}u) success", targetOrientation);
    };
    taskScheduler_->PostMainThreadTask(task, "OnGetTargetOrientationConfigInfo" + std::to_string(targetOrientation));
}

napi_value JsSceneSession::OnSetShowRecent(napi_env env, napi_callback_info info)
{
    auto session = weakSession_.promote();
    if (session == nullptr) {
        WLOGFE("session is null, id:%{public}d", persistentId_);
        return NapiGetUndefined(env);
    }
    size_t argc = 4;
    napi_value argv[4] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    bool showRecent = true;
    if (argc == ARGC_ONE && GetType(env, argv[0]) == napi_boolean) {
        if (!ConvertFromJsValue(env, argv[0], showRecent)) {
            WLOGFE("Failed to convert parameter to bool");
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
        WLOGFE("Argc is invalid: %{public}zu", argc);
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    uint32_t zOrder;
    if (!ConvertFromJsValue(env, argv[0], zOrder)) {
        WLOGFE("Failed to convert parameter to uint32_t");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    auto session = weakSession_.promote();
    if (session == nullptr) {
        WLOGFW("session is null, id:%{public}d", persistentId_);
        return NapiGetUndefined(env);
    }
    session->SetZOrder(zOrder);
    return NapiGetUndefined(env);
}

napi_value JsSceneSession::OnGetZOrder(napi_env env, napi_callback_info info)
{
    auto session = weakSession_.promote();
    if (session == nullptr) {
        TLOGE(WmsLogTag::WMS_PC, "session is nullptr, id:%{public}d", persistentId_);
        return NapiGetUndefined(env);
    }
    uint32_t sessionZOrder = session->GetZOrder();
    return CreateJsValue(env, sessionZOrder);
}

napi_value JsSceneSession::OnGetUid(napi_env env, napi_callback_info info)
{
    auto session = weakSession_.promote();
    if (session == nullptr) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "session is nullptr, id:%{public}d", persistentId_);
        return NapiGetUndefined(env);
    }
    int32_t sessionUid = session->GetCallingUid();
    return CreateJsValue(env, sessionUid);
}

napi_value JsSceneSession::OnSetFloatingScale(napi_env env, napi_callback_info info)
{
    size_t argc = 4;
    napi_value argv[4] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc < 1) { // 1: params num
        WLOGFE("Argc is invalid: %{public}zu", argc);
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    double_t floatingScale = 1.0;
    if (!ConvertFromJsValue(env, argv[0], floatingScale)) {
        WLOGFE("Failed to convert parameter to bool");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    auto session = weakSession_.promote();
    if (session == nullptr) {
        WLOGFE("session is nullptr, id:%{public}d", persistentId_);
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
        TLOGE(WmsLogTag::WMS_MULTI_WINDOW, "Argc is invalid: %{public}zu", argc);
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }

    bool isMidScene = false;
    if (!ConvertFromJsValue(env, argv[0], isMidScene)) {
        TLOGE(WmsLogTag::WMS_MULTI_WINDOW, "Failed to convert parameter to isMidScene");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }

    auto session = weakSession_.promote();
    if (session == nullptr) {
        TLOGE(WmsLogTag::WMS_MULTI_WINDOW, "session is nullptr, id:%{public}d", persistentId_);
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
        WLOGFE("Argc is invalid: %{public}zu", argc);
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    bool scbKeepKeyboardFlag = false;
    if (!ConvertFromJsValue(env, argv[0], scbKeepKeyboardFlag)) {
        WLOGFE("Failed to convert parameter to bool");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    auto session = weakSession_.promote();
    if (session == nullptr) {
        WLOGFE("session is nullptr, id:%{public}d", persistentId_);
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
        WLOGFE("Argc count is invalid: %{public}zu", argc);
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
                                      "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }

    double offsetX = 0.0f;
    if (!ConvertFromJsValue(env, argv[0], offsetX)) {
        WLOGFE("Failed to convert parameter to double");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
                                      "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }

    double offsetY = 0.0f;
    if (!ConvertFromJsValue(env, argv[1], offsetY)) {
        WLOGFE("Failed to convert parameter to double");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
                                      "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }

    auto session = weakSession_.promote();
    if (session == nullptr) {
        WLOGFE("session is nullptr, id:%{public}d", persistentId_);
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
        TLOGE(WmsLogTag::WMS_MULTI_WINDOW, "Argc count is invalid: %{public}zu", argc);
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
                                      "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    bool isExitSplitOnBackground = false;
    if (!ConvertFromJsValue(env, argv[0], isExitSplitOnBackground)) {
        TLOGE(WmsLogTag::WMS_MULTI_WINDOW, "Failed to convert parameter to bool");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
                                      "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    auto session = weakSession_.promote();
    if (session == nullptr) {
        TLOGE(WmsLogTag::WMS_MULTI_WINDOW, "session is nullptr, id:%{public}d", persistentId_);
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
        TLOGE(WmsLogTag::WMS_SCB, "Argc is invalid: %{public}zu", argc);
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    bool isWaterMarkAdded = false;
    if (!ConvertFromJsValue(env, argv[0], isWaterMarkAdded)) {
        TLOGE(WmsLogTag::WMS_SCB, "Failed to convert parameter to bool");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    auto session = weakSession_.promote();
    if (session == nullptr) {
        TLOGE(WmsLogTag::WMS_SCB, "session is nullptr, id:%{public}d", persistentId_);
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
    auto session = weakSession_.promote();
    if (session == nullptr) {
        TLOGE(WmsLogTag::WMS_PIP, "session is nullptr, id:%{public}d", persistentId_);
        return;
    }
    session->RegisterProcessPrepareClosePiPCallback([weakThis = wptr(this)] {
        auto jsSceneSession = weakThis.promote();
        if (!jsSceneSession) {
            TLOGNE(WmsLogTag::WMS_LIFE, "OnPrepareClosePiPSession jsSceneSession is null");
            return;
        }
        jsSceneSession->OnPrepareClosePiPSession();
    });
    TLOGD(WmsLogTag::WMS_PIP, "success");
}

void JsSceneSession::OnPrepareClosePiPSession()
{
    TLOGI(WmsLogTag::WMS_PIP, "[NAPI]");
    auto task = [weakThis = wptr(this), persistentId = persistentId_, env = env_] {
        auto jsSceneSession = weakThis.promote();
        if (!jsSceneSession || jsSceneSessionMap_.find(persistentId) == jsSceneSessionMap_.end()) {
            TLOGNE(WmsLogTag::WMS_LIFE, "OnPrepareClosePiPSession jsSceneSession id:%{public}d has been destroyed",
                persistentId);
            return;
        }
        auto jsCallBack = jsSceneSession->GetJSCallback(PREPARE_CLOSE_PIP_SESSION);
        if (!jsCallBack) {
            TLOGNE(WmsLogTag::WMS_PIP, "jsCallBack is nullptr");
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
        WLOGFE("argc is invalid : %{public}zu", argc);
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    bool scbSystemActive = false;
    if (!ConvertFromJsValue(env, argv[0], scbSystemActive)){
        WLOGFE("Failed to convert parameter to bool");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    auto session = weakSession_.promote();
    if (session == nullptr) {
        WLOGFE("session is null, id:%{public}d", persistentId_);
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
        WLOGFE("Argc is invalid: %{public}zu", argc);
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input Parameter is missing or invalid" ));
        return NapiGetUndefined(env);
    }

    bool touchable = false;
    if (!ConvertFromJsValue(env, argv[0], touchable)) {
        WLOGFE("Failed to convert parameter to touchable");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }

    auto session = weakSession_.promote();
    if (session == nullptr) {
        WLOGFE("session is null, id:%{public}d", persistentId_);
        return NapiGetUndefined(env);
    }

    session->SetSystemTouchable(touchable);
    return NapiGetUndefined(env);
}

napi_value JsSceneSession::OnSetWindowInputType(napi_env env, napi_callback_info info)
{
    size_t argc = 4;
    napi_value argv[4] = { nullptr };
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc != 1) {
        WLOGFE("Argc is invalid: %{public}zu", argc);
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input Parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }

    uint32_t windowInputType = 0;
    if (!ConvertFromJsValue(env, argv[0], windowInputType)) {
        WLOGFE("Failed to convert parameter to windowInputType");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }

    auto session = weakSession_.promote();
    if (session == nullptr) {
        WLOGFE("session is null, id:%{public}d", persistentId_);
        return NapiGetUndefined(env);
    }

    session->SetSessionInfoWindowInputType(windowInputType);
    return NapiGetUndefined(env);
}

napi_value JsSceneSession::OnSetExpandInputFlag(napi_env env, napi_callback_info info)
{
    size_t argc = 4;
    napi_value argv[4] = { nullptr };
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc != 1) {
        TLOGE(WmsLogTag::WMS_EVENT, "Argc is invalid: %{public}zu", argc);
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input Parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }

    uint32_t expandInputFlag = 0;
    if (!ConvertFromJsValue(env, argv[0], expandInputFlag)) {
        TLOGE(WmsLogTag::WMS_EVENT, "Failed to convert parameter to expandInputFlag");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }

    auto session = weakSession_.promote();
    if (session == nullptr) {
        TLOGE(WmsLogTag::WMS_EVENT, "session is null, id:%{public}d", persistentId_);
        return NapiGetUndefined(env);
    }

    session->SetSessionInfoExpandInputFlag(expandInputFlag);
    return NapiGetUndefined(env);
}

napi_value JsSceneSession::SetScale(napi_env env, napi_callback_info info)
{
    if (Session::IsScbCoreEnabled()) {
        return nullptr;
    }
    TLOGD(WmsLogTag::DEFAULT, "[NAPI]");
    JsSceneSession* me = CheckParamsAndGetThis<JsSceneSession>(env, info);
    return (me != nullptr) ? me->OnSetScale(env, info) : nullptr;
}

napi_value JsSceneSession::OnSetScale(napi_env env, napi_callback_info info)
{
    size_t argc = ARG_COUNT_4;
    napi_value argv[ARG_COUNT_4] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc < ARG_COUNT_4) { // ARG_COUNT_4: params num
        WLOGFE("Argc is invalid: %{public}zu", argc);
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    double_t scaleX = 1.0;
    if (!ConvertFromJsValue(env, argv[ARG_INDEX_0], scaleX)) {
        WLOGFE("Failed to convert parameter to scaleX");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    double_t scaleY = 1.0;
    if (!ConvertFromJsValue(env, argv[ARG_INDEX_1], scaleY)) {
        WLOGFE("Failed to convert parameter to scaleY");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    double_t pivotX = 0.0;
    if (!ConvertFromJsValue(env, argv[ARG_INDEX_2], pivotX)) {
        WLOGFE("Failed to convert parameter to pivotX");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    double_t pivotY = 0.0;
    if (!ConvertFromJsValue(env, argv[ARG_INDEX_3], pivotY)) {
        WLOGFE("Failed to convert parameter to pivotY");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    auto session = weakSession_.promote();
    if (session == nullptr) {
        WLOGFE("session is nullptr, id:%{public}d", persistentId_);
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
        TLOGE(WmsLogTag::WMS_LAYOUT, "Argc is invalid: %{public}zu", argc);
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    int32_t left = 0;
    if (!ConvertFromJsValue(env, argv[ARG_INDEX_0], left)) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "Failed to convert parameter to left");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    int32_t top = 0;
    if (!ConvertFromJsValue(env, argv[ARG_INDEX_1], top)) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "Failed to convert parameter to top");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    int32_t width = 0;
    if (!ConvertFromJsValue(env, argv[ARG_INDEX_2], width)) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "Failed to convert parameter to width");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    int32_t height = 0;
    if (!ConvertFromJsValue(env, argv[ARG_INDEX_3], height)) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "Failed to convert parameter to height");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    auto session = weakSession_.promote();
    if (session == nullptr) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "session is nullptr, id:%{public}d", persistentId_);
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
        TLOGE(WmsLogTag::WMS_LAYOUT, "Argc is invalid: %{public}zu", argc);
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input Parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }

    bool movable = true;
    if (!ConvertFromJsValue(env, argv[0], movable)) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "Failed to convert parameter to movable");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }

    auto session = weakSession_.promote();
    if (session == nullptr) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "session is nullptr, id:%{public}d", persistentId_);
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
        TLOGE(WmsLogTag::WMS_LAYOUT, "Argc count is invalid: %{public}zu", argc);
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
                                      "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    bool isVisible = true;
    if (!ConvertFromJsValue(env, argv[0], isVisible)) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "Failed to convert parameter to bool");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
                                      "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }

    auto session = weakSession_.promote();
    if (session == nullptr) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "session is nullptr, id:%{public}d", persistentId_);
        return NapiGetUndefined(env);
    }
    session->SetSplitButtonVisible(isVisible);
    return NapiGetUndefined(env);
}

napi_value JsSceneSession::RequestHideKeyboard(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::DEFAULT, "[NAPI]");
    JsSceneSession* me = CheckParamsAndGetThis<JsSceneSession>(env, info);
    return (me != nullptr) ? me->OnRequestHideKeyboard(env, info) : nullptr;
}

napi_value JsSceneSession::OnRequestHideKeyboard(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::DEFAULT, "[NAPI]");
    auto session = weakSession_.promote();
    if (session == nullptr) {
        WLOGFE("session is nullptr, id:%{public}d", persistentId_);
        return NapiGetUndefined(env);
    }
    session->RequestHideKeyboard();
    return NapiGetUndefined(env);
}

napi_value JsSceneSession::OnSendFbActionEvent(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value argv[1] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc < 1) {
        TLOGE(WmsLogTag::WMS_SYSTEM, "Argc count is invalid: %{public}zu", argc);
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
                                      "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    std::string action;
    if (!ConvertFromJsValue(env, argv[0], action)) {
        TLOGE(WmsLogTag::WMS_SYSTEM, "Failed to convert parameter to string");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
                                      "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }

    auto session = weakSession_.promote();
    if (session == nullptr) {
        TLOGE(WmsLogTag::WMS_SYSTEM, "session is nullptr, id:%{public}d", persistentId_);
        return NapiGetUndefined(env);
    }
    session->SendFbActionEvent(action);
    return NapiGetUndefined(env);
}

napi_value JsSceneSession::OnSetPipActionEvent(napi_env env, napi_callback_info info)
{
    size_t argc = 4;
    napi_value argv[4] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc < 1) {
        TLOGE(WmsLogTag::WMS_PIP, "Argc count is invalid: %{public}zu", argc);
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
                                      "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    std::string action;
    if (!ConvertFromJsValue(env, argv[0], action)) {
        TLOGE(WmsLogTag::WMS_PIP, "Failed to convert parameter to string");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
                                      "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    int32_t status = -1;
    if (argc > 1) {
        if (!ConvertFromJsValue(env, argv[1], status)) {
            TLOGE(WmsLogTag::WMS_PIP, "Failed to convert parameter to int");
            napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
                                          "Input parameter is missing or invalid"));
            return NapiGetUndefined(env);
        }
    }

    auto session = weakSession_.promote();
    if (session == nullptr) {
        TLOGE(WmsLogTag::WMS_PIP, "session is nullptr, id:%{public}d", persistentId_);
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
        TLOGE(WmsLogTag::WMS_PIP, "Argc count is invalid: %{public}zu", argc);
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    auto controlType = WsPiPControlType::VIDEO_PLAY_PAUSE;
    if (!ConvertFromJsValue(env, argv[0], controlType)) {
        TLOGE(WmsLogTag::WMS_PIP, "Failed to convert parameter to int");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    auto status = WsPiPControlStatus::PLAY;
    if (argc > 1) {
        if (!ConvertFromJsValue(env, argv[1], status)) {
            TLOGE(WmsLogTag::WMS_PIP, "Failed to convert parameter to int");
            napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
                "Input parameter is missing or invalid"));
            return NapiGetUndefined(env);
        }
    }

    auto session = weakSession_.promote();
    if (session == nullptr) {
        TLOGE(WmsLogTag::WMS_PIP, "session is nullptr, id:%{public}d", persistentId_);
        return NapiGetUndefined(env);
    }
    session->SetPiPControlEvent(controlType, status);
    return NapiGetUndefined(env);
}

napi_value JsSceneSession::OnNotifyPipOcclusionChange(napi_env env, napi_callback_info info)
{
    size_t argc = ARG_COUNT_4;
    napi_value argv[ARG_COUNT_4] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc < 1) {
        TLOGE(WmsLogTag::WMS_PIP, "Argc count is invalid: %{public}zu", argc);
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    bool occluded = false;
    if (!ConvertFromJsValue(env, argv[0], occluded)) {
        TLOGE(WmsLogTag::WMS_PIP, "Failed to convert parameter, keep default: false");
    }
    auto session = weakSession_.promote();
    if (session == nullptr) {
        TLOGE(WmsLogTag::WMS_PIP, "session is nullptr, id:%{public}d", persistentId_);
        return NapiGetUndefined(env);
    }
    TLOGI(WmsLogTag::WMS_PIP, "persistId:%{public}d, occluded:%{public}d", persistentId_, occluded);
    // Maybe expand with session visibility&state change
    SceneSessionManager::GetInstance().HandleKeepScreenOn(session, !occluded, WINDOW_SCREEN_LOCK_PREFIX,
                                                          session->keepScreenLock_);
    return NapiGetUndefined(env);
}

napi_value JsSceneSession::OnNotifyPipSizeChange(napi_env env, napi_callback_info info)
{
    size_t argc = ARG_COUNT_4;
    napi_value argv[ARG_COUNT_4] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc != ARGC_THREE) {
        TLOGE(WmsLogTag::WMS_PIP, "Argc count is invalid: %{public}zu", argc);
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    double width = 0.0;
    double height = 0.0;
    double scale = 0.0;
    if (!ConvertFromJsValue(env, argv[ARG_INDEX_0], width) || !ConvertFromJsValue(env, argv[ARG_INDEX_1], height) ||
        !ConvertFromJsValue(env, argv[ARG_INDEX_2], scale)) {
        TLOGE(WmsLogTag::WMS_PIP, "Failed to convert parameter, keep default: false");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }

    auto session = weakSession_.promote();
    if (session == nullptr) {
        TLOGE(WmsLogTag::WMS_PIP, "session is nullptr, id:%{public}d", persistentId_);
        return NapiGetUndefined(env);
    }

    TLOGI(WmsLogTag::WMS_PIP, "persistId:%{public}d, width:%{public}f height:%{public}f scale:%{public}f",
          persistentId_, width, height, scale);
    // Maybe expand with session visibility&state change
    session->NotifyPipWindowSizeChange(width, height, scale);
    return NapiGetUndefined(env);
}

napi_value JsSceneSession::OnNotifyPiPActiveStatusChange(napi_env env, napi_callback_info info)
{
    size_t argc = ARG_COUNT_1;
    napi_value argv[ARG_COUNT_1] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc != ARG_COUNT_1) {
        TLOGE(WmsLogTag::WMS_PIP, "Argc count is invalid: %{public}zu", argc);
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }

    bool status = false;
    if (!ConvertFromJsValue(env, argv[ARG_INDEX_0], status)) {
        TLOGE(WmsLogTag::WMS_PIP, "Failed to convert parameter, keep default: false");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }

    auto session = weakSession_.promote();
    if (session == nullptr) {
        TLOGE(WmsLogTag::WMS_PIP, "session is nullptr, id:%{public}d", persistentId_);
        return NapiGetUndefined(env);
    }

    TLOGI(WmsLogTag::WMS_PIP, "persistId:%{public}d", persistentId_);
    session->NotifyPiPActiveStatusChange(status);
    return NapiGetUndefined(env);
}

napi_value JsSceneSession::OnNotifyDisplayStatusBarTemporarily(napi_env env, napi_callback_info info)
{
    auto session = weakSession_.promote();
    if (session == nullptr) {
        TLOGE(WmsLogTag::WMS_IMMS, "session is nullptr, win %{public}d", persistentId_);
        return NapiGetUndefined(env);
    }
    size_t argc = ARGC_FOUR;
    napi_value argv[ARGC_FOUR] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    bool isTempDisplay = false;
    if (argc == ARGC_ONE && GetType(env, argv[0]) == napi_boolean) {
        if (!ConvertFromJsValue(env, argv[0], isTempDisplay)) {
            TLOGE(WmsLogTag::WMS_IMMS, "failed to convert parameter to bool");
            return NapiGetUndefined(env);
        }
    }
    session->SetIsDisplayStatusBarTemporarily(isTempDisplay);

    TLOGI(WmsLogTag::WMS_IMMS, "win [%{public}u %{public}s] isTempDisplay %{public}u",
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
        TLOGE(WmsLogTag::WMS_SCB, "argc is invalid : %{public}zu", argc);
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    bool isTemporarilyShowWhenLocked = false;
    if (!ConvertFromJsValue(env, argv[0], isTemporarilyShowWhenLocked)) {
        TLOGE(WmsLogTag::WMS_SCB, "Failed to convert parameter to bool");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    auto session = weakSession_.promote();
    if (session == nullptr) {
        TLOGE(WmsLogTag::WMS_SCB, "session_ is null");
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
        TLOGE(WmsLogTag::WMS_SCB, "argc is invalid : %{public}zu", argc);
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    bool skip = false;
    if (!ConvertFromJsValue(env, argv[0], skip)) {
        TLOGE(WmsLogTag::WMS_SCB, "Failed to convert parameter to bool");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    auto session = weakSession_.promote();
    if (session == nullptr) {
        TLOGE(WmsLogTag::WMS_SCB, "session_ is null");
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
        WLOGFE("Argc is invalid: %{public}zu", argc);
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    bool isSkip = false;
    if (!ConvertFromJsValue(env, argv[0], isSkip)) {
        WLOGFE("Failed to convert parameter to bool");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    auto session = weakSession_.promote();
    if (session == nullptr) {
        WLOGFE("session is nullptr, id:%{public}d", persistentId_);
        return NapiGetUndefined(env);
    }
    session->SetSkipSelfWhenShowOnVirtualScreen(isSkip);
    TLOGD(WmsLogTag::DEFAULT, "end");
    return NapiGetUndefined(env);
}

napi_value JsSceneSession::OnSetSkipEventOnCastPlus(napi_env env, napi_callback_info info)
{
    size_t argc = 4;
    napi_value argv[4] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc != 1) { // 1: params num
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "Argc is invalid: %{public}zu", argc);
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    bool isSkip = false;
    if (!ConvertFromJsValue(env, argv[0], isSkip)) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "Failed to convert parameter to bool");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    auto session = weakSession_.promote();
    if (session == nullptr) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "session is nullptr, id:%{public}d", persistentId_);
        return NapiGetUndefined(env);
    }
    session->SetSkipEventOnCastPlus(isSkip);
    TLOGI(WmsLogTag::WMS_ATTRIBUTE, "end");
    return NapiGetUndefined(env);
}

napi_value JsSceneSession::OnSetAppSupportPhoneInPc(napi_env env, napi_callback_info info)
{
    size_t argc = ARGC_FOUR;
    napi_value argv[ARGC_FOUR] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc < ARGC_ONE) {
        TLOGE(WmsLogTag::WMS_COMPAT, "Argc is invalid: %{public}zu", argc);
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    bool isSupportPhone = false;
    if (!ConvertFromJsValue(env, argv[0], isSupportPhone)) {
        TLOGE(WmsLogTag::WMS_COMPAT, "Failed to convert parameter to isSupportPhone");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    auto session = weakSession_.promote();
    if (session == nullptr) {
        TLOGE(WmsLogTag::WMS_COMPAT, "session is nullptr, id:%{public}d", persistentId_);
        return NapiGetUndefined(env);
    }
    session->SetAppSupportPhoneInPc(isSupportPhone);
    return NapiGetUndefined(env);
}

napi_value JsSceneSession::OnToggleCompatibleMode(napi_env env, napi_callback_info info)
{
    size_t argc = ARGC_FOUR;
    napi_value argv[ARGC_FOUR] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc < ARGC_ONE || argc > ARGC_TWO) {
        TLOGE(WmsLogTag::WMS_COMPAT, "Argc is invalid: %{public}zu", argc);
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    bool toCompatibleMode = false;
    if (!ConvertFromJsValue(env, argv[ARG_INDEX_0], toCompatibleMode)) {
        TLOGE(WmsLogTag::WMS_COMPAT, "Failed to convert parameter");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    auto session = weakSession_.promote();
    if (session == nullptr) {
        TLOGE(WmsLogTag::WMS_COMPAT, "session is nullptr, id:%{public}d", persistentId_);
        return NapiGetUndefined(env);
    }

    if (!toCompatibleMode) {
        session->SetCompatibleModeProperty(nullptr);
        return NapiGetUndefined(env);
    }

    sptr<CompatibleModeProperty> compatibleModeProperty = session->GetSessionProperty()->GetCompatibleModeProperty();
    if (compatibleModeProperty == nullptr) {
        compatibleModeProperty = sptr<CompatibleModeProperty>::MakeSptr();
    }
    if (!toCompatibleMode || !ConvertCompatibleModePropertyFromJs(env, argv[ARG_INDEX_1], *compatibleModeProperty)) {
        TLOGE(WmsLogTag::WMS_COMPAT, "convert compatible mode property failed, id:%{public}d", persistentId_);
        return NapiGetUndefined(env);
    }
    session->SetCompatibleModeProperty(compatibleModeProperty);
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
        TLOGE(WmsLogTag::WMS_SCB, "session is nullptr, id:%{public}d", persistentId_);
        return NapiGetUndefined(env);
    }
    session->SetUniqueDensityDpi(useUnique, densityDpi);
    return NapiGetUndefined(env);
}

napi_value JsSceneSession::OnSetBlank(napi_env env, napi_callback_info info)
{
    size_t argc = ARGC_FOUR;
    napi_value argv[ARGC_FOUR] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);

    if (argc < ARGC_ONE) {
        TLOGE(WmsLogTag::WMS_SCB, "Argc is invalid: %{public}zu", argc);
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }

    bool isAddBlank = false;
    if (!ConvertFromJsValue(env, argv[0], isAddBlank)) {
        TLOGE(WmsLogTag::WMS_SCB, "Failed to convert parameter to isAddBlank");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }

    auto session = weakSession_.promote();
    if (session == nullptr) {
        TLOGE(WmsLogTag::WMS_SCB, "session is nullptr, id:%{public}d", persistentId_);
        return NapiGetUndefined(env);
    }
    session->SetBlank(isAddBlank);
    return NapiGetUndefined(env);
}

napi_value JsSceneSession::OnRemoveBlank(napi_env env, napi_callback_info info)
{
    auto session = weakSession_.promote();
    if (session == nullptr) {
        TLOGE(WmsLogTag::WMS_SCB, "session is nullptr, id:%{public}d", persistentId_);
        return NapiGetUndefined(env);
    }
    session->NotifyRemoveBlank();
    return NapiGetUndefined(env);
}

napi_value JsSceneSession::OnAddSnapshot(napi_env env, napi_callback_info info)
{
    size_t argc = ARGC_FOUR;
    napi_value argv[ARGC_FOUR] = { nullptr };
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);

    bool useFfrt = false;
    if (argc >= ARGC_ONE && GetType(env, argv[0]) == napi_boolean) {
        if (!ConvertFromJsValue(env, argv[0], useFfrt)) {
            TLOGE(WmsLogTag::WMS_PATTERN, "Failed to convert parameter to useFfrt");
            napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
                "Input parameter is missing or invalid"));
            return NapiGetUndefined(env);
        }
    }

    bool needPersist = false;
    if (argc >= ARGC_TWO && GetType(env, argv[1]) == napi_boolean) {
        if (!ConvertFromJsValue(env, argv[1], needPersist)) {
            TLOGE(WmsLogTag::WMS_PATTERN, "Failed to convert parameter to needPersist");
            napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
                "Input parameter is missing or invalid"));
            return NapiGetUndefined(env);
        }
    }

    TLOGI(WmsLogTag::WMS_PATTERN, "argc: %{public}zu, useFfrt: %{public}d, needPersist: %{public}d",
        argc, useFfrt, needPersist);
    auto session = weakSession_.promote();
    if (session == nullptr) {
        TLOGE(WmsLogTag::WMS_PATTERN, "session is null, id:%{public}d", persistentId_);
        return NapiGetUndefined(env);
    }
    session->NotifyAddSnapshot(useFfrt, needPersist, true);
    return NapiGetUndefined(env);
}

napi_value JsSceneSession::OnRemoveSnapshot(napi_env env, napi_callback_info info)
{
    auto session = weakSession_.promote();
    if (session == nullptr) {
        TLOGE(WmsLogTag::WMS_PATTERN, "session is nullptr, id:%{public}d", persistentId_);
        return NapiGetUndefined(env);
    }
    session->NotifyRemoveSnapshot();
    return NapiGetUndefined(env);
}

napi_value JsSceneSession::OnSetBufferAvailableCallbackEnable(napi_env env, napi_callback_info info)
{
    size_t argc = ARGC_FOUR;
    napi_value argv[ARGC_FOUR] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);

    if (argc < ARGC_ONE) {
        TLOGE(WmsLogTag::WMS_SCB, "Argc is invalid: %{public}zu", argc);
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }

    bool enable = false;
    if (!ConvertFromJsValue(env, argv[0], enable)) {
        TLOGE(WmsLogTag::WMS_SCB, "Failed to convert parameter to enable");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }

    auto session = weakSession_.promote();
    if (session == nullptr) {
        TLOGE(WmsLogTag::WMS_SCB, "session is nullptr, id:%{public}d", persistentId_);
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
        TLOGE(WmsLogTag::WMS_SCB, "Argc is invalid: %{public}zu", argc);
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    uint32_t defaultRequestedOrientation = 0;
    if (!ConvertFromJsValue(env, argv[0], defaultRequestedOrientation)) {
        TLOGE(WmsLogTag::WMS_SCB, "Failed to convert parameter to defaultRequestedOrientation");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    auto session = weakSession_.promote();
    if (session == nullptr) {
        TLOGE(WmsLogTag::WMS_SCB, "session is nullptr, id:%{public}d", persistentId_);
        return NapiGetUndefined(env);
    }
    auto windowOrientation = static_cast<Orientation>(defaultRequestedOrientation);
    if (windowOrientation < Orientation::BEGIN || windowOrientation > Orientation::END) {
        TLOGE(WmsLogTag::WMS_SCB, "[NAPI]Orientation %{public}u invalid, id:%{public}d",
            defaultRequestedOrientation, persistentId_);
        return NapiGetUndefined(env);
    }
    session->SetDefaultRequestedOrientation(windowOrientation);
    return NapiGetUndefined(env);
}

napi_value JsSceneSession::OnSetPcAppInpadCompatibleMode(napi_env env, napi_callback_info info)
{
    size_t argc = ARGC_FOUR;
    napi_value argv[ARGC_FOUR] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc < ARGC_ONE) {
        TLOGE(WmsLogTag::WMS_SCB, "Argc is invalid: %{public}zu", argc);
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
                                      "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    bool enabled = false;
    if (!ConvertFromJsValue(env, argv[0], enabled)) {
        TLOGE(WmsLogTag::WMS_SCB, "Failed to convert parameter to enable");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
                                      "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    auto session = weakSession_.promote();
    if (session == nullptr) {
        TLOGE(WmsLogTag::WMS_SCB, "session is nullptr, id:%{public}d", persistentId_);
        return NapiGetUndefined(env);
    }
    session->SetPcAppInpadCompatibleMode(enabled);
    return NapiGetUndefined(env);
}

napi_value JsSceneSession::OnSetMobileAppInPadLayoutFullScreen(napi_env env, napi_callback_info info)
{
    size_t argc = ARGC_FOUR;
    napi_value argv[ARGC_FOUR] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc < ARGC_ONE) {
        TLOGE(WmsLogTag::WMS_SCB, "Argc is invalid: %{public}zu", argc);
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
                                      "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    bool isMobileAppInPadLayoutFullScreen = false;
    if (!ConvertFromJsValue(env, argv[ARG_INDEX_0], isMobileAppInPadLayoutFullScreen)) {
        TLOGE(WmsLogTag::WMS_SCB, "Failed to convert parameter to enable");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
                                      "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    auto session = weakSession_.promote();
    if (session == nullptr) {
        TLOGE(WmsLogTag::WMS_SCB, "session is nullptr, id:%{public}d", persistentId_);
        return NapiGetUndefined(env);
    }
    session->SetMobileAppInPadLayoutFullScreen(isMobileAppInPadLayoutFullScreen);
    return NapiGetUndefined(env);
}

napi_value JsSceneSession::OnSetPcAppInpadSpecificSystemBarInvisible(napi_env env, napi_callback_info info)
{
    size_t argc = ARGC_FOUR;
    napi_value argv[ARGC_FOUR] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc < ARGC_ONE) {
        TLOGE(WmsLogTag::WMS_SCB, "Argc is invalid: %{public}zu", argc);
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
                                      "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    bool isPcAppInpadSpecificSystemBarInvisible = false;
    if (!ConvertFromJsValue(env, argv[0], isPcAppInpadSpecificSystemBarInvisible)) {
        TLOGE(WmsLogTag::WMS_SCB, "Failed to convert parameter to enable");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
                                      "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    auto session = weakSession_.promote();
    if (session == nullptr) {
        TLOGE(WmsLogTag::WMS_SCB, "session is nullptr, id:%{public}d", persistentId_);
        return NapiGetUndefined(env);
    }
    session->SetPcAppInpadSpecificSystemBarInvisible(isPcAppInpadSpecificSystemBarInvisible);
    return NapiGetUndefined(env);
}

napi_value JsSceneSession::OnSetPcAppInpadOrientationLandscape(napi_env env, napi_callback_info info)
{
    size_t argc = ARGC_FOUR;
    napi_value argv[ARGC_FOUR] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc < ARGC_ONE) {
        TLOGE(WmsLogTag::WMS_SCB, "Argc is invalid: %{public}zu", argc);
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
                                      "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    bool isPcAppInpadOrientationLandscape = false;
    if (!ConvertFromJsValue(env, argv[0], isPcAppInpadOrientationLandscape)) {
        TLOGE(WmsLogTag::WMS_SCB, "Failed to convert parameter to enable");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
                                      "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    auto session = weakSession_.promote();
    if (session == nullptr) {
        TLOGE(WmsLogTag::WMS_SCB, "session is nullptr, id:%{public}d", persistentId_);
        return NapiGetUndefined(env);
    }
    session->SetPcAppInpadOrientationLandscape(isPcAppInpadOrientationLandscape);
    return NapiGetUndefined(env);
}

napi_value JsSceneSession::OnUpdateSceneAnimationConfig(napi_env env, napi_callback_info info)
{
    size_t argc = ARGC_FOUR;
    napi_value argv[ARGC_FOUR] = { nullptr };
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc != ARGC_ONE) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "Argc is invalid: %{public}zu", argc);
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    SceneAnimationConfig animationConfig;
    if (!convertAnimConfigFromJs(env, argv[0], animationConfig)) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "Failed to convert parameter to animationConfig.");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is invalid."));
        return NapiGetUndefined(env);
    }
    auto session = weakSession_.promote();
    if (session == nullptr) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "session is nullptr, id:%{public}d", persistentId_);
        return NapiGetUndefined(env);
    }
    session->SetSceneAnimationConfig(animationConfig);
    return NapiGetUndefined(env);
}

napi_value JsSceneSession::OnSetIsPcAppInPad(napi_env env, napi_callback_info info)
{
    size_t argc = ARGC_FOUR;
    napi_value argv[ARGC_FOUR] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc < ARGC_ONE) {
        TLOGE(WmsLogTag::WMS_SCB, "Argc is invalid: %{public}zu", argc);
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
                                      "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    bool enable = false;
    if (!ConvertFromJsValue(env, argv[0], enable)) {
        TLOGE(WmsLogTag::WMS_SCB, "Failed to convert parameter to enable");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
                                      "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    auto session = weakSession_.promote();
    if (session == nullptr) {
        TLOGE(WmsLogTag::WMS_SCB, "session is nullptr, id:%{public}d", persistentId_);
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
        TLOGE(WmsLogTag::WMS_SCB, "Argc is invalid: %{public}zu", argc);
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }

    bool enable = true;
    if (!ConvertFromJsValue(env, argv[0], enable)) {
        TLOGE(WmsLogTag::WMS_SCB, "Failed to convert parameter to enable");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }

    auto session = weakSession_.promote();
    if (session == nullptr) {
        TLOGE(WmsLogTag::WMS_SCB, "session is null, id:%{public}d", persistentId_);
        return NapiGetUndefined(env);
    }
    session->SetStartingWindowExitAnimationFlag(enable);
    return NapiGetUndefined(env);
}

napi_value JsSceneSession::OnPcAppInPadNormalClose(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::WMS_COMPAT, "[NAPI]");
    auto session = weakSession_.promote();
    if (session == nullptr) {
        TLOGE(WmsLogTag::WMS_COMPAT, "session is nullptr, id:%{public}d", persistentId_);
        return NapiGetUndefined(env);
    }
    if (!(session->GetSessionProperty()->GetIsPcAppInPad())) {
        return NapiGetUndefined(env);
    }
    session->PcAppInPadNormalClose();
    return NapiGetUndefined(env);
}

napi_value JsSceneSession::OnSetWindowEnableDragBySystem(napi_env env, napi_callback_info info)
{
    size_t argc = ARGC_FOUR;
    napi_value argv[ARGC_FOUR] = { nullptr };
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc != ARGC_ONE) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "Argc is invalid: %{public}zu", argc);
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }

    bool enableDrag = true;
    if (!ConvertFromJsValue(env, argv[0], enableDrag)) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "Failed to convert parameter to bool");
        return NapiGetUndefined(env);
    }

    auto session = weakSession_.promote();
    if (session == nullptr) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "session is nullptr, id:%{public}d", persistentId_);
        return NapiGetUndefined(env);
    }
    session->SetWindowEnableDragBySystem(enableDrag);
    return NapiGetUndefined(env);
}

napi_value JsSceneSession::OnActivateDragBySystem(napi_env env, napi_callback_info info)
{
    size_t argc = ARGC_FOUR;
    napi_value argv[ARGC_FOUR] = { nullptr };
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc != ARGC_ONE) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "Argc is invalid: %{public}zu", argc);
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }

    bool activateDrag = true;
    if (!ConvertFromJsValue(env, argv[ARG_INDEX_0], activateDrag)) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "Failed to convert parameter to bool");
        return NapiGetUndefined(env);
    }

    auto session = weakSession_.promote();
    if (session == nullptr) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "Session is nullptr, id:%{public}d", persistentId_);
        return NapiGetUndefined(env);
    }
    session->ActivateDragBySystem(activateDrag);
    return NapiGetUndefined(env);
}

napi_value JsSceneSession::OnSyncScenePanelGlobalPosition(napi_env env, napi_callback_info info)
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

    std::string reason;
    if (!ConvertFromJsValue(env, argv[0], reason)) {
        TLOGE(WmsLogTag::WMS_PIPELINE, "Failed to convert parameter to sync reason");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    auto it = std::find_if(g_syncGlobalPositionPermission.begin(), g_syncGlobalPositionPermission.end(),
        [reason](const std::string& permission) { return permission.find(reason) != std::string::npos; });
    if (it == g_syncGlobalPositionPermission.end()) {
        TLOGE(WmsLogTag::WMS_PIPELINE, "called reason:%{public}s is not permitted", reason.c_str());
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_NO_PERMISSION),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    TLOGI(WmsLogTag::WMS_PIPELINE, "called reason:%{public}s", reason.c_str());
    auto session = weakSession_.promote();
    if (session == nullptr) {
        TLOGE(WmsLogTag::WMS_SCB, "session is nullptr, id:%{public}d", persistentId_);
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
        TLOGE(WmsLogTag::WMS_SCB, "Argc is invalid: %{public}zu", argc);
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }

    std::string reason;
    if (!ConvertFromJsValue(env, argv[0], reason)) {
        TLOGE(WmsLogTag::WMS_PIPELINE, "Failed to convert parameter to un sync reason");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    auto it = std::find_if(g_syncGlobalPositionPermission.begin(), g_syncGlobalPositionPermission.end(),
        [reason](const std::string& permission) { return permission.find(reason) != std::string::npos; });
    if (it == g_syncGlobalPositionPermission.end()) {
        TLOGE(WmsLogTag::WMS_PIPELINE, "called reason:%{public}s is not permitted", reason.c_str());
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_NO_PERMISSION),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    TLOGI(WmsLogTag::WMS_PIPELINE, "called reason:%{public}s", reason.c_str());
    auto session = weakSession_.promote();
    if (session == nullptr) {
        TLOGE(WmsLogTag::WMS_SCB, "session is nullptr, id:%{public}d", persistentId_);
        return NapiGetUndefined(env);
    }
    session->SyncScenePanelGlobalPosition(false);
    return NapiGetUndefined(env);
}

napi_value JsSceneSession::OnSetIsPendingToBackgroundState(napi_env env, napi_callback_info info)
{
    auto session = weakSession_.promote();
    if (session == nullptr) {
        TLOGE(WmsLogTag::WMS_LIFE, "session is null, id: %{public}d", persistentId_);
        return NapiGetUndefined(env);
    }
    size_t argc = ARGC_FOUR;
    napi_value argv[ARGC_FOUR] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc != ARGC_ONE) {
        TLOGE(WmsLogTag::WMS_LIFE, "Argc is invalid: %{public}zu", argc);
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM), "InputInvalid"));
        return NapiGetUndefined(env);
    }
    bool isPendingToBackgroundState = false;
    if (GetType(env, argv[0]) == napi_boolean) {
        if (!ConvertFromJsValue(env, argv[0], isPendingToBackgroundState)) {
            TLOGE(WmsLogTag::WMS_LIFE, "Failed to convert parameter to isPendingToBackgroundState");
            napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
                "Input parameter is missing or invalid"));
            return NapiGetUndefined(env);
        }
        TLOGD(WmsLogTag::WMS_LIFE, "isPendingToBackgroundState: %{public}u", isPendingToBackgroundState);
    }
    session->SetIsPendingToBackgroundState(isPendingToBackgroundState);
    return NapiGetUndefined(env);
}

napi_value JsSceneSession::OnSetLabel(napi_env env, napi_callback_info info)
{
    auto session = weakSession_.promote();
    if (session == nullptr) {
        TLOGE(WmsLogTag::WMS_LIFE, "session is null, id: %{public}d", persistentId_);
        return NapiGetUndefined(env);
    }
    size_t argc = ARGC_FOUR;
    napi_value argv[ARGC_FOUR] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc != ARGC_ONE) {
        TLOGE(WmsLogTag::WMS_LIFE, "Argc is invalid: %{public}zu", argc);
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM), "Input Invalid"));
        return NapiGetUndefined(env);
    }
    std::string label;
    if (!ConvertFromJsValue(env, argv[0], label)) {
        TLOGE(WmsLogTag::WMS_LIFE, "Failed to convert parameter to string");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    TLOGI(WmsLogTag::WMS_LIFE, "label:%{public}s, id:%{public}d", label.c_str(), persistentId_);
    session->UpdateSessionLabel(label);
    return NapiGetUndefined(env);
}

napi_value JsSceneSession::OnSetIsActivatedAfterScreenLocked(napi_env env, napi_callback_info info)
{
    size_t argc = ARGC_FOUR;
    napi_value argv[ARGC_FOUR] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc != ARGC_ONE) {
        TLOGE(WmsLogTag::WMS_LIFE, "Argc is invalid: %{public}zu", argc);
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM), "InputInvalid"));
        return NapiGetUndefined(env);
    }
    bool isActivatedAfterScreenLocked = false;
    if (!ConvertFromJsValue(env, argv[0], isActivatedAfterScreenLocked)) {
        TLOGE(WmsLogTag::WMS_LIFE, "Failed to convert parameter to isActivatedAfterScreenLocked");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    TLOGD(WmsLogTag::WMS_LIFE, "isActivatedAfterScreenLocked: %{public}u", isActivatedAfterScreenLocked);
    auto session = weakSession_.promote();
    if (session == nullptr) {
        TLOGE(WmsLogTag::WMS_LIFE, "session is null, id: %{public}d", persistentId_);
        return NapiGetUndefined(env);
    }
    session->SetIsActivatedAfterScreenLocked(isActivatedAfterScreenLocked);
    return NapiGetUndefined(env);
}

napi_value JsSceneSession::OnSetNeedSyncSessionRect(napi_env env, napi_callback_info info)
{
    size_t argc = ARGC_FOUR;
    napi_value argv[ARGC_FOUR] = { nullptr };
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);

    if (argc != ARGC_ONE) {
        TLOGE(WmsLogTag::WMS_PIPELINE, "Argc is invalid: %{public}zu", argc);
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }

    bool needSync = true;
    if (!ConvertFromJsValue(env, argv[0], needSync)) {
        TLOGE(WmsLogTag::WMS_PIPELINE, "Failed to convert parameter to needSync");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    TLOGI(WmsLogTag::WMS_PIPELINE, "needSync:%{public}u, id:%{public}d", needSync, persistentId_);
    auto session = weakSession_.promote();
    if (session == nullptr) {
        TLOGE(WmsLogTag::WMS_PIPELINE, "session is nullptr, id:%{public}d", persistentId_);
        return NapiGetUndefined(env);
    }
    session->SetNeedSyncSessionRect(needSync);
    return NapiGetUndefined(env);
}

void JsSceneSession::ProcessSetWindowRectAutoSaveRegister()
{
    auto session = weakSession_.promote();
    if (session == nullptr) {
        TLOGE(WmsLogTag::WMS_MAIN, "session is nullptr, id:%{public}d", persistentId_);
        return;
    }
    const char* const where = __func__;
    session->SetWindowRectAutoSaveCallback([weakThis = wptr(this), where](bool enabled, bool isSaveBySpecifiedFlag) {
        auto jsSceneSession = weakThis.promote();
        if (!jsSceneSession) {
            TLOGNE(WmsLogTag::WMS_MAIN, "%{public}s: jsSceneSession is null", where);
            return;
        }
        jsSceneSession->OnSetWindowRectAutoSave(enabled, isSaveBySpecifiedFlag);
    });
    TLOGI(WmsLogTag::WMS_MAIN, "success");
}

void JsSceneSession::OnSetWindowRectAutoSave(bool enabled, bool isSaveBySpecifiedFlag)
{
    const char* const where = __func__;
    auto task = [weakThis = wptr(this), persistentId = persistentId_, enabled,
        isSaveBySpecifiedFlag, env = env_, where] {
        auto jsSceneSession = weakThis.promote();
        if (!jsSceneSession || jsSceneSessionMap_.find(persistentId) == jsSceneSessionMap_.end()) {
            TLOGNE(WmsLogTag::WMS_MAIN, "%{public}s: jsSceneSession id:%{public}d has been destroyed",
                where, persistentId);
            return;
        }
        auto jsCallBack = jsSceneSession->GetJSCallback(SET_WINDOW_RECT_AUTO_SAVE_CB);
        if (!jsCallBack) {
            TLOGNE(WmsLogTag::WMS_MAIN, "%{public}s: jsCallBack is nullptr", where);
            return;
        }
        napi_value jsEnabled = CreateJsValue(env, enabled);
        napi_value jsSaveBySpecifiedFlag = CreateJsValue(env, isSaveBySpecifiedFlag);
        napi_value argv[] = { jsEnabled, jsSaveBySpecifiedFlag };
        napi_call_function(env, NapiGetUndefined(env), jsCallBack->GetNapiValue(), ArraySize(argv), argv, nullptr);
    };
    taskScheduler_->PostMainThreadTask(task, __func__);
}

void JsSceneSession::ProcessSetSupportedWindowModesRegister()
{
    auto session = weakSession_.promote();
    if (session == nullptr) {
        TLOGE(WmsLogTag::WMS_LAYOUT_PC, "session is nullptr, id:%{public}d", persistentId_);
        return;
    }
    const char* const where = __func__;
    session->RegisterSupportWindowModesCallback([weakThis = wptr(this), where](
        std::vector<AppExecFwk::SupportWindowMode>&& supportedWindowModes) {
        auto jsSceneSession = weakThis.promote();
        if (!jsSceneSession) {
            TLOGNE(WmsLogTag::WMS_LAYOUT_PC, "%{public}s: jsSceneSession is null", where);
            return;
        }
        jsSceneSession->OnSetSupportedWindowModes(std::move(supportedWindowModes));
    });
    TLOGD(WmsLogTag::WMS_LAYOUT_PC, "success");
}

void JsSceneSession::OnSetSupportedWindowModes(std::vector<AppExecFwk::SupportWindowMode>&& supportedWindowModes)
{
    const char* const where = __func__;
    taskScheduler_->PostMainThreadTask([weakThis = wptr(this), persistentId = persistentId_,
        supportedWindowModes = std::move(supportedWindowModes), env = env_, where] {
        auto jsSceneSession = weakThis.promote();
        if (!jsSceneSession || jsSceneSessionMap_.find(persistentId) == jsSceneSessionMap_.end()) {
            TLOGNE(WmsLogTag::WMS_LAYOUT_PC, "%{public}s: jsSceneSession id:%{public}d has been destroyed",
                where, persistentId);
            return;
        }
        auto jsCallBack = jsSceneSession->GetJSCallback(SET_SUPPORT_WINDOW_MODES_CB);
        if (!jsCallBack) {
            TLOGNE(WmsLogTag::WMS_LAYOUT_PC, "%{public}s: jsCallBack is nullptr", where);
            return;
        }
        napi_value jsSupportWindowModes = CreateSupportWindowModes(env, supportedWindowModes);
        napi_value argv[] = { jsSupportWindowModes };
        napi_call_function(env, NapiGetUndefined(env), jsCallBack->GetNapiValue(), ArraySize(argv), argv, nullptr);
    }, __func__);
}

void JsSceneSession::RegisterUpdateAppUseControlCallback()
{
    auto session = weakSession_.promote();
    if (session == nullptr) {
        TLOGE(WmsLogTag::WMS_LIFE, "session is nullptr, id:%{public}d", persistentId_);
        return;
    }
    const char* const where = __func__;
    session->RegisterUpdateAppUseControlCallback(
        [weakThis = wptr(this), where](ControlAppType type, bool isNeedControl, bool isControlRecentOnly) {
        auto jsSceneSession = weakThis.promote();
        if (!jsSceneSession) {
            TLOGNE(WmsLogTag::WMS_LIFE, "%{public}s: jsSceneSession is null", where);
            return;
        }
        jsSceneSession->OnUpdateAppUseControl(type, isNeedControl, isControlRecentOnly);
    });
    TLOGI(WmsLogTag::WMS_LIFE, "success");
}

void JsSceneSession::OnUpdateAppUseControl(ControlAppType type, bool isNeedControl, bool isControlRecentOnly)
{
    const char* const where = __func__;
    auto task = [weakThis = wptr(this), persistentId = persistentId_, type, isNeedControl,
        isControlRecentOnly, env = env_, where] {
        auto jsSceneSession = weakThis.promote();
        if (!jsSceneSession || jsSceneSessionMap_.find(persistentId) == jsSceneSessionMap_.end()) {
            TLOGNE(WmsLogTag::WMS_LIFE, "%{public}s: jsSceneSession id:%{public}d has been destroyed",
                where, persistentId);
            return;
        }
        auto jsCallBack = jsSceneSession->GetJSCallback(UPDATE_APP_USE_CONTROL_CB);
        if (!jsCallBack) {
            TLOGNE(WmsLogTag::WMS_LIFE, "%{public}s: jsCallBack is nullptr", where);
            return;
        }
        napi_value jsTypeArgv = CreateJsValue(env, static_cast<uint8_t>(type));
        napi_value jsIsNeedControlArgv = CreateJsValue(env, isNeedControl);
        napi_value jsIsControlRecentOnlyArgv = CreateJsValue(env, isControlRecentOnly);
        napi_value argv[] = { jsTypeArgv, jsIsNeedControlArgv, jsIsControlRecentOnlyArgv };
        napi_call_function(env, NapiGetUndefined(env), jsCallBack->GetNapiValue(), ArraySize(argv), argv, nullptr);
    };
    taskScheduler_->PostMainThreadTask(task, __func__);
}

napi_value JsSceneSession::OnSetFrameGravity(napi_env env, napi_callback_info info)
{
    size_t argc = ARGC_FOUR;
    napi_value argv[ARGC_FOUR] = { nullptr };
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc != ARGC_ONE) {
        TLOGE(WmsLogTag::WMS_LAYOUT_PC, "Argc is invalid: %{public}zu", argc);
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    Gravity gravity = Gravity::TOP_LEFT;
    if (!ConvertFromJsValue(env, argv[0], gravity)) {
        TLOGE(WmsLogTag::WMS_LAYOUT_PC, "Failed to convert parameter to gravity");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    auto session = weakSession_.promote();
    if (session == nullptr) {
        TLOGE(WmsLogTag::WMS_LAYOUT_PC, "session is nullptr, id:%{public}d", persistentId_);
        return NapiGetUndefined(env);
    }
    session->SetFrameGravity(gravity);
    return NapiGetUndefined(env);
}

napi_value JsSceneSession::OnSetUseStartingWindowAboveLocked(napi_env env, napi_callback_info info)
{
    size_t argc = ARGC_FOUR;
    napi_value argv[ARGC_FOUR] = { nullptr };
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc != ARGC_ONE) {
        TLOGE(WmsLogTag::WMS_PATTERN, "Argc is invalid: %{public}zu", argc);
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }

    bool useStartingWindowAboveLocked = false;
    if (!ConvertFromJsValue(env, argv[0], useStartingWindowAboveLocked)) {
        TLOGE(WmsLogTag::WMS_PATTERN, "Failed to convert parameter to useStartingWindowAboveLocked");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    auto session = weakSession_.promote();
    if (session == nullptr) {
        TLOGE(WmsLogTag::WMS_PATTERN, "session is nullptr, id:%{public}d", persistentId_);
        return NapiGetUndefined(env);
    }
    session->SetUseStartingWindowAboveLocked(useStartingWindowAboveLocked);
    return NapiGetUndefined(env);
}

napi_value JsSceneSession::OnSaveSnapshotSync(napi_env env, napi_callback_info info)
{
    auto session = weakSession_.promote();
    if (session == nullptr) {
        TLOGE(WmsLogTag::WMS_PATTERN, "session is nullptr, id:%{public}d", persistentId_);
        return NapiGetUndefined(env);
    }
    session->SaveSnapshot(false);
    return NapiGetUndefined(env);
}

napi_value JsSceneSession::OnSaveSnapshotAsync(napi_env env, napi_callback_info info)
{
    auto session = weakSession_.promote();
    if (session == nullptr) {
        TLOGE(WmsLogTag::WMS_PATTERN, "session is nullptr, id:%{public}d", persistentId_);
        return NapiGetUndefined(env);
    }
    session->SaveSnapshot(true);
    return NapiGetUndefined(env);
}

napi_value JsSceneSession::OnSetBorderUnoccupied(napi_env env, napi_callback_info info)
{
    size_t argc = ARGC_FOUR;
    napi_value argv[ARGC_FOUR] = { nullptr };
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc != ARGC_ONE) {
        TLOGE(WmsLogTag::WMS_PATTERN, "Argc is invalid: %{public}zu", argc);
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    bool borderUnoccupied = false;
    if (!ConvertFromJsValue(env, argv[0], borderUnoccupied)) {
        TLOGE(WmsLogTag::WMS_PATTERN, "Failed to convert parameter to enabled");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    auto session = weakSession_.promote();
    if (session == nullptr) {
        TLOGE(WmsLogTag::WMS_PATTERN, "session is nullptr, id:%{public}d", persistentId_);
        return NapiGetUndefined(env);
    }
    session->SetBorderUnoccupied(borderUnoccupied);
    return NapiGetUndefined(env);
}

napi_value JsSceneSession::OnSetEnableAddSnapshot(napi_env env, napi_callback_info info)
{
    size_t argc = ARGC_FOUR;
    napi_value argv[ARGC_FOUR] = { nullptr };
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc != ARGC_ONE) {
        TLOGE(WmsLogTag::WMS_PATTERN, "Argc is invalid: %{public}zu", argc);
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    bool enableAddSnapshot = true;
    if (!ConvertFromJsValue(env, argv[0], enableAddSnapshot)) {
        TLOGE(WmsLogTag::WMS_PATTERN, "Failed to convert parameter to enableAddSnapshot");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    auto session = weakSession_.promote();
    if (session == nullptr) {
        TLOGE(WmsLogTag::WMS_PATTERN, "session is nullptr, id:%{public}d", persistentId_);
        return NapiGetUndefined(env);
    }
    session->SetEnableAddSnapshot(enableAddSnapshot);
    return NapiGetUndefined(env);
}

napi_value JsSceneSession::OnSetFreezeImmediately(napi_env env, napi_callback_info info)
{
    size_t argc = ARGC_FOUR;
    napi_value argv[ARGC_FOUR] = { nullptr };
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc != ARGC_THREE) {
        TLOGE(WmsLogTag::WMS_PATTERN, "Argc is invalid: %{public}zu", argc);
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    double scaleValue;
    if (!ConvertFromJsValue(env, argv[ARG_INDEX_0], scaleValue)) {
        TLOGE(WmsLogTag::WMS_PATTERN, "Failed to convert parameter to scaleValue");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    float scaleParam = GreatOrEqual(scaleValue, 0.0f) && LessOrEqual(scaleValue, 1.0f) ?
        static_cast<float>(scaleValue) : 0.0f;
    bool isFreeze = false;
    if (!ConvertFromJsValue(env, argv[ARG_INDEX_1], isFreeze)) {
        TLOGE(WmsLogTag::WMS_PATTERN, "Failed to convert parameter to isFreeze");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    double blurValue;
    if (!ConvertFromJsValue(env, argv[ARG_INDEX_2], blurValue)) {
        TLOGE(WmsLogTag::WMS_PATTERN, "Failed to convert parameter to blurValue");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    float blurParam = static_cast<float>(blurValue);
    auto session = weakSession_.promote();
    if (session == nullptr) {
        TLOGE(WmsLogTag::WMS_PATTERN, "session is nullptr, id:%{public}d", persistentId_);
        return NapiGetUndefined(env);
    }
    std::shared_ptr<Media::PixelMap> pixelPtr = session->SetFreezeImmediately(scaleParam, isFreeze, blurParam);
    if (isFreeze) {
        if (pixelPtr == nullptr) {
            TLOGE(WmsLogTag::WMS_PATTERN, "Failed to create pixelPtr");
            napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_STATE_ABNORMALLY),
                "System is abnormal"));
            return NapiGetUndefined(env);
        }
        napi_value pixelMapObj = Media::PixelMapNapi::CreatePixelMap(env, pixelPtr);
        if (pixelMapObj == nullptr) {
            TLOGE(WmsLogTag::WMS_PATTERN, "Failed to create pixel map object");
            napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_STATE_ABNORMALLY),
                "System is abnormal"));
            return NapiGetUndefined(env);
        }
        return pixelMapObj;
    }
    return NapiGetUndefined(env);
}

napi_value JsSceneSession::OnThrowSlipDirectly(napi_env env, napi_callback_info info)
{
    size_t argc = ARGC_FOUR;
    napi_value argv[ARGC_FOUR] = { nullptr };
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc != ARGC_THREE) {
        TLOGE(WmsLogTag::WMS_LAYOUT_PC, "Argc is invalid: %{public}zu", argc);
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }

    ThrowSlipMode throwSlipMode = ThrowSlipMode::INVALID;
    napi_value nativeValue = argv[0];
    if (nativeValue == nullptr || !ConvertThrowSlipModeFromJs(env, nativeValue, throwSlipMode)) {
        TLOGE(WmsLogTag::WMS_LAYOUT_PC, "Failed to convert parameter to throwSlipMode");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    double vx = 0.0;
    if (!ConvertFromJsValue(env, argv[ARGC_ONE], vx)) {
        TLOGE(WmsLogTag::WMS_LAYOUT_PC, "Failed to convert parameter to vx");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    double vy = 0.0;
    if (!ConvertFromJsValue(env, argv[ARGC_TWO], vy)) {
        TLOGE(WmsLogTag::WMS_LAYOUT_PC, "Failed to convert parameter to vy");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }

    auto session = weakSession_.promote();
    if (session == nullptr) {
        TLOGE(WmsLogTag::WMS_LAYOUT_PC, "session is nullptr, id:%{public}d", persistentId_);
        return NapiGetUndefined(env);
    }
    session->ThrowSlipDirectly(throwSlipMode, WSRectF {static_cast<float>(vx), static_cast<float>(vy), 0.0f, 0.0f});
    return NapiGetUndefined(env);
}

void JsSceneSession::ProcessSessionLockStateChangeRegister()
{
    auto session = weakSession_.promote();
    if (session == nullptr) {
        TLOGE(WmsLogTag::WMS_MAIN, "session is nullptr, id:%{public}d", persistentId_);
        return;
    }
    session->RegisterSessionLockStateChangeCallback([weakThis = wptr(this), where = __func__](bool isLockedState) {
        auto jsSceneSession = weakThis.promote();
        if (!jsSceneSession) {
            TLOGNE(WmsLogTag::WMS_MAIN, "%{public}s: jsSceneSession is null", where);
            return;
        }
        jsSceneSession->OnSessionLockStateChange(isLockedState);
    });
    TLOGI(WmsLogTag::WMS_MAIN, "success");
}

void JsSceneSession::OnSessionLockStateChange(bool isLockedState)
{
    auto task = [weakThis = wptr(this), persistentId = persistentId_, isLockedState, env = env_, where = __func__] {
        auto jsSceneSession = weakThis.promote();
        if (!jsSceneSession || jsSceneSessionMap_.find(persistentId) == jsSceneSessionMap_.end()) {
            TLOGNE(WmsLogTag::WMS_MAIN, "%{public}s: jsSceneSession id:%{public}d has been destroyed.",
                where, persistentId);
            return;
        }
        auto jsCallBack = jsSceneSession->GetJSCallback(SESSION_LOCK_STATE_CHANGE_CB);
        if (!jsCallBack) {
            TLOGNE(WmsLogTag::WMS_MAIN, "%{public}s: jsCallBack is nullptr", where);
            return;
        }
        napi_value jsSessionLockState = CreateJsValue(env, isLockedState);
        napi_value argv[] = { jsSessionLockState };
        napi_call_function(env, NapiGetUndefined(env), jsCallBack->GetNapiValue(), ArraySize(argv), argv, nullptr);
    };
    taskScheduler_->PostMainThreadTask(task, __func__);
}

napi_value JsSceneSession::OnSendContainerModalEvent(napi_env env, napi_callback_info info)
{
    size_t argc = ARGC_FOUR;
    napi_value argv[ARGC_FOUR] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc < ARGC_TWO) {
        TLOGE(WmsLogTag::WMS_EVENT, "Argc count is invalid: %{public}zu", argc);
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
                                      "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    std::string eventName;
    if (!ConvertFromJsValue(env, argv[0], eventName)) {
        TLOGE(WmsLogTag::WMS_EVENT, "Failed to convert parameter to string");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
                                      "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    std::string eventValue;
    if (!ConvertFromJsValue(env, argv[1], eventValue) || eventValue.empty()) {
        TLOGE(WmsLogTag::WMS_EVENT, "Failed to convert parameter to string");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
                                      "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }

    auto session = weakSession_.promote();
    if (session == nullptr) {
        TLOGE(WmsLogTag::WMS_EVENT, "session is nullptr, id:%{public}d", persistentId_);
        return NapiGetUndefined(env);
    }
    if (!session->IsPcWindow()) {
        TLOGE(WmsLogTag::WMS_EVENT, "device type not support");
        return NapiGetUndefined(env);
    }
    session->SendContainerModalEvent(eventName, eventValue);
    return NapiGetUndefined(env);
}

napi_value JsSceneSession::OnNotifyRotationProperty(napi_env env, napi_callback_info info)
{
    size_t argc = ARGC_FOUR;
    napi_value argv[ARGC_FOUR] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc < ARGC_THREE) {
        TLOGE(WmsLogTag::WMS_ROTATION, "Argc count is invalid: %{public}zu", argc);
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
                                      "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }

    uint32_t rotation = 0;
    if (!ConvertFromJsValue(env, argv[0], rotation)) {
        TLOGE(WmsLogTag::WMS_ROTATION, "[NAPI]Failed to convert rotation from js object");
        return NapiGetUndefined(env);
    }
    int32_t width = 0;
    if (!ConvertFromJsValue(env, argv[ARGC_ONE], width)) {
        TLOGE(WmsLogTag::WMS_ROTATION, "[NAPI]Failed to convert width from js object");
        return NapiGetUndefined(env);
    }
    int32_t height = 0;
    if (!ConvertFromJsValue(env, argv[ARGC_TWO], height)) {
        TLOGE(WmsLogTag::WMS_ROTATION, "[NAPI]Failed to convert height from js object");
        return NapiGetUndefined(env);
    }
    auto session = weakSession_.promote();
    if (session == nullptr) {
        TLOGNE(WmsLogTag::WMS_ROTATION, "session is nullptr, id:%{public}d", persistentId_);
        return NapiGetUndefined(env);
    }
    TLOGNI(WmsLogTag::WMS_ROTATION,
        "call NotifyRotationProperty rotation:%{public}d, width=%{public}d, height=%{pubilc}d",
        rotation, width, height);
    session->NotifyRotationProperty(rotation, width, height);
    return NapiGetUndefined(env);
}

napi_value JsSceneSession::OnNotifyPageRotationIsIgnored(napi_env env, napi_callback_info info)
{
    auto session = weakSession_.promote();
    if (session == nullptr) {
        TLOGE(WmsLogTag::WMS_ROTATION, "session is nullptr, id:%{public}d", persistentId_);
        return NapiGetUndefined(env);
    }
    session->NotifyPageRotationIsIgnored();
    return NapiGetUndefined(env);
}

void JsSceneSession::ProcessUpdateSessionLabelAndIconRegister()
{
    TLOGD(WmsLogTag::WMS_MAIN, "in");
    auto session = weakSession_.promote();
    if (session == nullptr) {
        TLOGE(WmsLogTag::WMS_MAIN, "session is nullptr, id:%{public}d", persistentId_);
        return;
    }
    const char* const where = __func__;
    session->SetUpdateSessionLabelAndIconListener([weakThis = wptr(this), where](const std::string& label,
        const std::shared_ptr<Media::PixelMap>& icon) {
        auto jsSceneSession = weakThis.promote();
        if (!jsSceneSession) {
            TLOGNE(WmsLogTag::WMS_MAIN, "%{public}s jsSceneSession is null", where);
            return;
        }
        jsSceneSession->UpdateSessionLabelAndIcon(label, icon);
    });
    TLOGD(WmsLogTag::WMS_MAIN, "success");
}

void JsSceneSession::UpdateSessionLabelAndIcon(const std::string& label, const std::shared_ptr<Media::PixelMap>& icon)
{
    TLOGI(WmsLogTag::WMS_MAIN, "in");
    const char* const where = __func__;
    auto task = [weakThis = wptr(this), persistentId = persistentId_, label, icon, env = env_, where] {
        auto jsSceneSession = weakThis.promote();
        if (!jsSceneSession || jsSceneSessionMap_.find(persistentId) == jsSceneSessionMap_.end()) {
            TLOGNE(WmsLogTag::WMS_MAIN, "%{public}s jsSceneSession id:%{public}d has been destroyed",
                where, persistentId);
            return;
        }
        auto jsCallBack = jsSceneSession->GetJSCallback(UPDATE_SESSION_LABEL_AND_ICON_CB);
        if (!jsCallBack) {
            TLOGNE(WmsLogTag::WMS_MAIN, "%{public}s jsCallBack is nullptr", where);
            return;
        }
        napi_value jsLabel = CreateJsValue(env, label);
        if (jsLabel == nullptr) {
            TLOGNE(WmsLogTag::WMS_MAIN, "%{public}s label is nullptr", where);
            return;
        }
        napi_value jsIcon = Media::PixelMapNapi::CreatePixelMap(env, icon);
        if (jsIcon == nullptr) {
            TLOGNE(WmsLogTag::WMS_MAIN, "%{public}s icon is nullptr", where);
            return;
        }
        napi_value argv[] = {jsLabel, jsIcon};
        napi_call_function(env, NapiGetUndefined(env), jsCallBack->GetNapiValue(), ArraySize(argv), argv, nullptr);
    };
    taskScheduler_->PostMainThreadTask(task, __func__);
}

void JsSceneSession::ProcessKeyboardStateChangeRegister()
{
    auto session = weakSession_.promote();
    if (session == nullptr) {
        TLOGE(WmsLogTag::WMS_KEYBOARD, "session is nullptr, id:%{public}d", persistentId_);
        return;
    }
    session->SetKeyboardStateChangeListener(
        [weakThis = wptr(this), where = __func__](
            SessionState state, const KeyboardEffectOption& effectOption, uint32_t callingSessionId,
            DisplayId targetDisplayId) {
        auto jsSceneSession = weakThis.promote();
        if (!jsSceneSession) {
            TLOGNE(WmsLogTag::WMS_KEYBOARD, "%{public}s: jsSceneSession is null", where);
            return;
        }
        jsSceneSession->OnKeyboardStateChange(state, effectOption, callingSessionId, targetDisplayId);
    });
    TLOGD(WmsLogTag::WMS_KEYBOARD, "success");
}

void JsSceneSession::OnKeyboardStateChange(SessionState state, const KeyboardEffectOption& effectOption,
    const uint32_t callingSessionId, DisplayId targetDisplayId)
{
    auto session = weakSession_.promote();
    if (session == nullptr) {
        TLOGW(WmsLogTag::WMS_KEYBOARD, "session is nullptr, id:%{public}d", persistentId_);
        return;
    }
    auto task = [weakThis = wptr(this), persistentId = persistentId_, state, env = env_, effectOption, callingSessionId,
        targetDisplayId, where = __func__] {
        auto jsSceneSession = weakThis.promote();
        if (!jsSceneSession || jsSceneSessionMap_.find(persistentId) == jsSceneSessionMap_.end()) {
            TLOGNE(WmsLogTag::WMS_KEYBOARD, "OnKeyboardStateChange jsSceneSession id:%{public}d has been destroyed",
                persistentId);
            return;
        }
        auto jsCallBack = jsSceneSession->GetJSCallback(KEYBOARD_STATE_CHANGE_CB);
        if (!jsCallBack) {
            TLOGNE(WmsLogTag::WMS_KEYBOARD, "jsCallBack is nullptr");
            return;
        }
        napi_value jsKeyboardStateObj = CreateJsValue(env, state);
        napi_value jsKeyboardEffectOptionObj = ConvertKeyboardEffectOptionToJsValue(env, effectOption);
        napi_value jsKeyboardCallingIdObj = CreateJsValue(env, callingSessionId);
        napi_value jsKeyboardTargetDisplayId = CreateJsNumber(env, static_cast<int64_t>(targetDisplayId));
        napi_value argv[] = {
            jsKeyboardStateObj,
            jsKeyboardEffectOptionObj,
            jsKeyboardCallingIdObj,
            jsKeyboardTargetDisplayId,
        };
        napi_handle_scope scope = nullptr;
        napi_open_handle_scope(env, &scope);
        auto value = jsCallBack->GetNapiValue();
        if (value == nullptr) {
            TLOGNE(WmsLogTag::WMS_KEYBOARD, "%{public}s: jsCallBack->GetNapiValue() is null", where);
            napi_close_handle_scope(env, scope);
            return;
        }
        auto ret = napi_call_function(env, NapiGetUndefined(env), jsCallBack->GetNapiValue(), ArraySize(argv),
            argv, nullptr);
        napi_close_handle_scope(env, scope);
        if (ret != napi_ok) {
            TLOGNE(WmsLogTag::WMS_KEYBOARD, "%{public}s: napi_call_function result is error", where);
            return;
        }
        TLOGNI(WmsLogTag::WMS_KEYBOARD, "%{public}s: id: %{public}d, state: %{public}d, callingSessionId: %{public}u",
            where, persistentId, state, callingSessionId);
    };
    taskScheduler_->PostMainThreadTask(task, "OnKeyboardStateChange, state:" +
        std::to_string(static_cast<uint32_t>(state)));
}

void JsSceneSession::ProcessCallingSessionIdChangeRegister()
{
    auto session = weakSession_.promote();
    if (session == nullptr) {
        TLOGE(WmsLogTag::WMS_KEYBOARD, "session is nullptr, id:%{public}d", persistentId_);
        return;
    }
    session->SetCallingSessionIdSessionListenser([weakThis = wptr(this), where = __func__](uint32_t callingSessionId) {
        auto jsSceneSession = weakThis.promote();
        if (!jsSceneSession) {
            TLOGNE(WmsLogTag::WMS_KEYBOARD, "%{public}s: jsSceneSession is null", where);
            return;
        }
        jsSceneSession->OnCallingSessionIdChange(callingSessionId);
    });
    TLOGD(WmsLogTag::WMS_KEYBOARD, "success");
}

void JsSceneSession::OnCallingSessionIdChange(uint32_t callingSessionId)
{
    auto session = weakSession_.promote();
    if (session == nullptr) {
        TLOGW(WmsLogTag::WMS_KEYBOARD, "session is nullptr, id:%{public}d", persistentId_);
        return;
    }
    auto task = [weakThis = wptr(this), persistentId = persistentId_, callingSessionId, env = env_, where = __func__] {
        auto jsSceneSession = weakThis.promote();
        if (!jsSceneSession || jsSceneSessionMap_.find(persistentId) == jsSceneSessionMap_.end()) {
            TLOGNE(WmsLogTag::WMS_KEYBOARD, "OnCallingSessionIdChange jsSceneSession id:%{public}d has been destroyed",
                persistentId);
            return;
        }
        auto jsCallBack = jsSceneSession->GetJSCallback(CALLING_SESSION_ID_CHANGE_CB);
        if (!jsCallBack) {
            TLOGNE(WmsLogTag::WMS_KEYBOARD, "jsCallBack is nullptr");
            return;
        }
        napi_value argv[] = { CreateJsValue(env, callingSessionId) };
        napi_call_function(env, NapiGetUndefined(env), jsCallBack->GetNapiValue(), ArraySize(argv), argv, nullptr);
        TLOGNI(WmsLogTag::WMS_KEYBOARD, "%{public}s: id: %{public}d, newCallingId: %{public}d",
            where, persistentId, callingSessionId);
    };
    taskScheduler_->PostMainThreadTask(std::move(task),
        "OnCallingSessionIdChange, callingId:" + std::to_string(callingSessionId));
}

void JsSceneSession::ProcessKeyboardEffectOptionChangeRegister()
{
    auto session = weakSession_.promote();
    if (session == nullptr) {
        TLOGE(WmsLogTag::WMS_KEYBOARD, "session is nullptr, id:%{public}d", persistentId_);
        return;
    }
    session->SetKeyboardEffectOptionChangeListener(
        [weakThis = wptr(this)](const KeyboardEffectOption& effectOption) {
        auto jsSceneSession = weakThis.promote();
        if (!jsSceneSession) {
            TLOGNE(WmsLogTag::WMS_KEYBOARD, "ProcessKeyboardEffectOptionChangeRegister jsSceneSession is null");
            return;
        }
        jsSceneSession->OnKeyboardEffectOptionChange(effectOption);
    });
    TLOGI(WmsLogTag::WMS_KEYBOARD, "Register success. id: %{public}d", persistentId_);
}

void JsSceneSession::OnKeyboardEffectOptionChange(const KeyboardEffectOption& effectOption)
{
    TLOGI(WmsLogTag::WMS_KEYBOARD, "Change KeyboardEffectOption to %{public}s", effectOption.ToString().c_str());
    auto task = [weakThis = wptr(this), persistentId = persistentId_, env = env_, effectOption] {
        auto jsSceneSession = weakThis.promote();
        if (!jsSceneSession || jsSceneSessionMap_.find(persistentId) == jsSceneSessionMap_.end()) {
            TLOGNE(WmsLogTag::WMS_KEYBOARD, "jsSceneSession id:%{public}d has been destroyed",
                persistentId);
            return;
        }
        auto jsCallBack = jsSceneSession->GetJSCallback(KEYBOARD_EFFECT_OPTION_CHANGE_CB);
        if (!jsCallBack) {
            TLOGNE(WmsLogTag::WMS_KEYBOARD, "jsCallBack is nullptr");
            return;
        }
        napi_value jsKeyboardEffectOption = ConvertKeyboardEffectOptionToJsValue(env, effectOption);
        napi_value argv[] = { jsKeyboardEffectOption };
        napi_call_function(env, NapiGetUndefined(env), jsCallBack->GetNapiValue(), ArraySize(argv), argv, nullptr);
    };
    taskScheduler_->PostMainThreadTask(task, __func__);
}

void JsSceneSession::ProcessSetWindowCornerRadiusRegister()
{
    auto session = weakSession_.promote();
    if (session == nullptr) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "session is nullptr, id:%{public}d", persistentId_);
        return;
    }
    const char* const where = __func__;
    session->SetWindowCornerRadiusCallback([weakThis = wptr(this), where](float cornerRadius) {
        auto jsSceneSession = weakThis.promote();
        if (!jsSceneSession) {
            TLOGNE(WmsLogTag::WMS_ATTRIBUTE, "%{public}s: jsSceneSession is null", where);
            return;
        }
        jsSceneSession->OnSetWindowCornerRadius(cornerRadius);
    });
    TLOGD(WmsLogTag::WMS_ATTRIBUTE, "success");
}

void JsSceneSession::OnSetWindowCornerRadius(float cornerRadius)
{
    const char* const where = __func__;
    taskScheduler_->PostMainThreadTask([weakThis = wptr(this), persistentId = persistentId_,
        cornerRadius, env = env_, where] {
        auto jsSceneSession = weakThis.promote();
        if (!jsSceneSession || jsSceneSessionMap_.find(persistentId) == jsSceneSessionMap_.end()) {
            TLOGNE(WmsLogTag::WMS_ATTRIBUTE, "%{public}s: jsSceneSession id:%{public}d has been destroyed",
                where, persistentId);
            return;
        }
        TLOGND(WmsLogTag::WMS_ATTRIBUTE, "%{public}s: cornerRadius is %{public}f", where, cornerRadius);
        auto jsCallBack = jsSceneSession->GetJSCallback(SET_WINDOW_CORNER_RADIUS_CB);
        if (!jsCallBack) {
            TLOGNE(WmsLogTag::WMS_ATTRIBUTE, "%{public}s: jsCallBack is nullptr", where);
            return;
        }
        napi_value jsCornerRadius = CreateJsValue(env, cornerRadius);
        napi_value argv[] = { jsCornerRadius };
        napi_call_function(env, NapiGetUndefined(env), jsCallBack->GetNapiValue(), ArraySize(argv), argv, nullptr);
    }, __func__);
}

void JsSceneSession::ProcessSetWindowShadowsRegister()
{
    auto session = weakSession_.promote();
    if (session == nullptr) {
        TLOGE(WmsLogTag::WMS_ANIMATION, "session is nullptr, id:%{public}d", persistentId_);
        return;
    }
    const char* const where = __func__;
    session->SetWindowShadowsCallback([weakThis = wptr(this), where](const ShadowsInfo& shadowsInfo) {
        auto jsSceneSession = weakThis.promote();
        if (!jsSceneSession) {
            TLOGNE(WmsLogTag::WMS_ANIMATION, "%{public}s: jsSceneSession is null", where);
            return;
        }
        jsSceneSession->OnSetWindowShadows(shadowsInfo);
    });
    TLOGD(WmsLogTag::WMS_ANIMATION, "success");
}

void JsSceneSession::OnSetWindowShadows(const ShadowsInfo& shadowsInfo)
{
    const char* const where = __func__;
    taskScheduler_->PostMainThreadTask([weakThis = wptr(this), persistentId = persistentId_,
        shadowsInfo, env = env_, where] {
        auto jsSceneSession = weakThis.promote();
        if (!jsSceneSession || jsSceneSessionMap_.find(persistentId) == jsSceneSessionMap_.end()) {
            TLOGNE(WmsLogTag::WMS_ANIMATION, "%{public}s: jsSceneSession id:%{public}d has been destroyed",
                where, persistentId);
            return;
        }
        TLOGND(WmsLogTag::WMS_ANIMATION, "%{public}s: shadow radius is %{public}f, color is %{public}s, "
            "offsetX is %{public}f, offsetY is %{public}f ", where, shadowsInfo.radius_, shadowsInfo.color_.c_str(),
            shadowsInfo.offsetX_, shadowsInfo.offsetY_);
        auto jsCallBack = jsSceneSession->GetJSCallback(SET_WINDOW_SHADOWS_CB);
        if (!jsCallBack) {
            TLOGNE(WmsLogTag::WMS_ANIMATION, "%{public}s: jsCallBack is nullptr", where);
            return;
        }
        napi_value jsShadowsInfoObj = CreateJsShadowsInfo(env, shadowsInfo);
        napi_value argv[] = { jsShadowsInfoObj };
        napi_call_function(env, NapiGetUndefined(env), jsCallBack->GetNapiValue(), ArraySize(argv), argv, nullptr);
    }, __func__);
}

void JsSceneSession::ProcessSetHighlightChangeRegister()
{
    NotifyHighlightChangeFunc func = [weakThis = wptr(this), where = __func__](bool isHighlight) {
        auto jsSceneSession = weakThis.promote();
        if (!jsSceneSession) {
            TLOGNE(WmsLogTag::WMS_FOCUS, "%{public}s: jsSceneSession is null", where);
            return;
        }
        jsSceneSession->NotifyHighlightChange(isHighlight);
    };
    auto session = weakSession_.promote();
    if (session == nullptr) {
        TLOGE(WmsLogTag::WMS_FOCUS, "session is nullptr");
        return;
    }
    session->SetHighlightChangeNotifyFunc(func);
}

void JsSceneSession::NotifyHighlightChange(bool isHighlight)
{
    TLOGI(WmsLogTag::WMS_FOCUS, "isHighlight: %{public}d, id: %{public}d", isHighlight, persistentId_);
    auto task = [weakThis = wptr(this), isHighlight, env = env_, persistentId = persistentId_, where = __func__] {
        auto jsSceneSession = weakThis.promote();
        if (!jsSceneSession || jsSceneSessionMap_.find(persistentId) == jsSceneSessionMap_.end()) {
            TLOGNE(WmsLogTag::WMS_FOCUS, "%{public}s: jsSceneSession id: %{public}d has been destroyed",
                where, persistentId);
            return;
        }
        auto jsCallBack = jsSceneSession->GetJSCallback(HIGHLIGHT_CHANGE_CB);
        if (!jsCallBack) {
            TLOGNE(WmsLogTag::WMS_FOCUS, "%{public}s: jsCallBack is nullptr", where);
            return;
        }
        napi_value jsIsHighlight = CreateJsValue(env, isHighlight);
        napi_value argv[] = { jsIsHighlight };
        napi_call_function(env, NapiGetUndefined(env), jsCallBack->GetNapiValue(), ArraySize(argv), argv, nullptr);
    };
    taskScheduler_->PostMainThreadTask(task, "NotifyHighlightChange");
}

void JsSceneSession::ProcessWindowAnchorInfoChangeRegister()
{
    auto session = weakSession_.promote();
    if (session == nullptr) {
        TLOGE(WmsLogTag::WMS_SUB, "session is nullptr");
        return;
    }
    if (!session->GetSystemConfig().supportFollowRelativePositionToParent_) {
        TLOGD(WmsLogTag::WMS_SUB, "system config not support");
        return;
    }
    NotifyWindowAnchorInfoChangeFunc func =
        [weakThis = wptr(this), where = __func__](const WindowAnchorInfo& windowAnchorInfo) {
            auto jsSceneSession = weakThis.promote();
            if (!jsSceneSession) {
                TLOGNE(WmsLogTag::WMS_SUB, "%{public}s: jsSceneSession is null", where);
                return;
            }
            jsSceneSession->NotifyWindowAnchorInfoChange(windowAnchorInfo);
        };
    session->SetWindowAnchorInfoChangeFunc(std::move(func));
}

void JsSceneSession::NotifyWindowAnchorInfoChange(const WindowAnchorInfo& windowAnchorInfo)
{
    TLOGI(WmsLogTag::WMS_SUB, "isAnchorEnabled: %{public}d, id: %{public}d",
    windowAnchorInfo.isAnchorEnabled_, persistentId_);
    auto task = [weakThis = wptr(this), windowAnchorInfo,
    env = env_, persistentId = persistentId_, where = __func__] {
        auto jsSceneSession = weakThis.promote();
        if (!jsSceneSession || jsSceneSessionMap_.find(persistentId) == jsSceneSessionMap_.end()) {
            TLOGNE(WmsLogTag::WMS_SUB, "%{public}s: jsSceneSession id: %{public}d has been destroyed",
                where, persistentId);
            return;
        }
        auto jsCallBack = jsSceneSession->GetJSCallback(WINDOW_ANCHOR_INFO_CHANGE_CB);
        if (!jsCallBack) {
            TLOGNE(WmsLogTag::WMS_SUB, "%{public}s: jsCallBack is nullptr", where);
            return;
        }
        napi_value WindowAnchorInfoObj = CreateJsWindowAnchorInfo(env, windowAnchorInfo);
        if (WindowAnchorInfoObj == nullptr) {
            TLOGNE(WmsLogTag::WMS_SUB, "window anchor info obj is nullptr");
        }
        napi_value argv[] = { WindowAnchorInfoObj };
        napi_call_function(env, NapiGetUndefined(env), jsCallBack->GetNapiValue(), ArraySize(argv), argv, nullptr);
    };
    taskScheduler_->PostMainThreadTask(task, "NotifyWindowAnchorInfoChange");
}

void JsSceneSession::ProcessFollowParentRectRegister()
{
    NotifyFollowParentRectFunc func = [weakThis = wptr(this), where = __func__](bool isFollow) {
        auto jsSceneSession = weakThis.promote();
        if (!jsSceneSession) {
            TLOGNE(WmsLogTag::WMS_SUB, "%{public}s: jsSceneSession is null", where);
            return;
        }
        jsSceneSession->NotifyFollowParentRect(isFollow);
    };
    auto session = weakSession_.promote();
    if (session == nullptr) {
        TLOGE(WmsLogTag::WMS_SUB, "session is nullptr");
        return;
    }
    session->SetFollowParentRectFunc(std::move(func));
}

void JsSceneSession::NotifyFollowParentRect(bool isFollow)
{
    TLOGI(WmsLogTag::WMS_SUB, "isFollow: %{public}d, id: %{public}d", isFollow, persistentId_);
    auto task = [weakThis = wptr(this), isFollow, env = env_, persistentId = persistentId_, where = __func__] {
        auto jsSceneSession = weakThis.promote();
        if (!jsSceneSession || jsSceneSessionMap_.find(persistentId) == jsSceneSessionMap_.end()) {
            TLOGNE(WmsLogTag::WMS_SUB, "%{public}s: jsSceneSession id: %{public}d has been destroyed",
                where, persistentId);
            return;
        }
        auto jsCallBack = jsSceneSession->GetJSCallback(FOLLOW_PARENT_RECT_CB);
        if (!jsCallBack) {
            TLOGNE(WmsLogTag::WMS_SUB, "%{public}s: jsCallBack is nullptr", where);
            return;
        }
        napi_value jsIsFollow = CreateJsValue(env, isFollow);
        napi_value argv[] = { jsIsFollow };
        napi_call_function(env, NapiGetUndefined(env), jsCallBack->GetNapiValue(), ArraySize(argv), argv, nullptr);
    };
    taskScheduler_->PostMainThreadTask(task, "NotifyFollowParentRect");
}

napi_value JsSceneSession::OnSetColorSpace(napi_env env, napi_callback_info info)
{
    size_t argc = ARGC_FOUR;
    napi_value argv[ARGC_FOUR] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc < ARGC_ONE) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "Argc is invalid: %{public}zu", argc);
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
                                      "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    ColorSpace colorSpace = ColorSpace::COLOR_SPACE_DEFAULT;
    if (!ConvertFromJsValue(env, argv[0], colorSpace)) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "Failed to convert parameter to ColorSpace");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
                                      "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    auto session = weakSession_.promote();
    if (session == nullptr) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "session is nullptr, id:%{public}d", persistentId_);
        return NapiGetUndefined(env);
    }
    session->SetColorSpace(colorSpace);
    return NapiGetUndefined(env);
}

napi_value JsSceneSession::OnSetSnapshotSkip(napi_env env, napi_callback_info info)
{
    size_t argc = ARGC_FOUR;
    napi_value argv[ARGC_FOUR] = { nullptr };
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc < ARGC_ONE) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "Argc is invalid: %{public}zu", argc);
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
                                      "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    bool isSkip = false;
    if (!ConvertFromJsValue(env, argv[0], isSkip)) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "Failed to convert parameter to isSkip");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
                                      "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    auto session = weakSession_.promote();
    if (session == nullptr) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "session is nullptr, id: %{public}d", persistentId_);
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_STATE_ABNORMALLY),
                                      "Session is nullptr"));
        return NapiGetUndefined(env);
    }
    if (session->SetSnapshotSkip(isSkip) != WMError::WM_OK) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "set snapshotSkip failed, id: %{public}d", persistentId_);
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_SYSTEM_ABNORMALLY),
                                      "Set failed"));
        return NapiGetUndefined(env);
    }
    return NapiGetUndefined(env);
}

void JsSceneSession::ProcessSnapshotSkipChangeRegister()
{
    auto session = weakSession_.promote();
    if (session == nullptr) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "session is nullptr, id:%{public}d", persistentId_);
        return;
    }
    session->RegisterSnapshotSkipChangeCallback([weakThis = wptr(this)](bool isSkip) {
        auto jsSceneSession = weakThis.promote();
        if (!jsSceneSession) {
            TLOGNE(WmsLogTag::WMS_ATTRIBUTE, "jsSceneSession is null");
            return;
        }
        jsSceneSession->OnSnapshotSkipChange(isSkip);
    });
}

void JsSceneSession::OnSnapshotSkipChange(bool isSkip)
{
    taskScheduler_->PostMainThreadTask([weakThis = wptr(this), persistentId = persistentId_, isSkip, env = env_] {
        auto jsSceneSession = weakThis.promote();
        if (!jsSceneSession || jsSceneSessionMap_.find(persistentId) == jsSceneSessionMap_.end()) {
            TLOGNE(WmsLogTag::WMS_ATTRIBUTE, "jsSceneSession id:%{public}d has been destroyed", persistentId);
            return;
        }
        auto jsCallBack = jsSceneSession->GetJSCallback(SNAPSHOT_SKIP_CHANGE_CB);
        if (!jsCallBack) {
            TLOGNE(WmsLogTag::WMS_ATTRIBUTE, "jsCallBack is nullptr");
            return;
        }
        TLOGNI(WmsLogTag::WMS_ATTRIBUTE, "isSkip change to %{public}d", isSkip);
        napi_value jsIsSkip = CreateJsValue(env, isSkip);
        napi_value argv[] = { jsIsSkip };
        napi_call_function(env, NapiGetUndefined(env), jsCallBack->GetNapiValue(), ArraySize(argv), argv, nullptr);
    }, __func__);
}

napi_value JsSceneSession::OnAddSidebarBlur(napi_env env, napi_callback_info info)
{
    auto session = weakSession_.promote();
    if (session == nullptr) {
        TLOGE(WmsLogTag::WMS_PC, "session is nullptr, id: %{public}d", persistentId_);
        return NapiGetUndefined(env);
    }
    session->AddSidebarBlur();
    return NapiGetUndefined(env);
}

napi_value JsSceneSession::OnSetSidebarBlur(napi_env env, napi_callback_info info)
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
    bool isDefaultSidebarBlur = false;
    if (!ConvertFromJsValue(env, argv[0], isDefaultSidebarBlur)) {
        TLOGE(WmsLogTag::WMS_PC, "Failed to convert parameter to isDefaultSidebarBlur");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
                                      "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    bool isNeedAnimation = false;
    if (!ConvertFromJsValue(env, argv[1], isNeedAnimation)) {
        TLOGE(WmsLogTag::WMS_PC, "Failed to convert parameter to isNeedAnimation");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
                                      "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    auto session = weakSession_.promote();
    if (session == nullptr) {
        TLOGE(WmsLogTag::WMS_PC, "session is nullptr, id:%{public}d", persistentId_);
        return NapiGetUndefined(env);
    }
    session->SetSidebarBlur(isDefaultSidebarBlur, isNeedAnimation);
    return NapiGetUndefined(env);
}

napi_value JsSceneSession::OnSetSidebarBlurMaximize(napi_env env, napi_callback_info info)
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
    bool isMaximize = false;
    if (!ConvertFromJsValue(env, argv[0], isMaximize)) {
        TLOGE(WmsLogTag::WMS_PC, "Failed to convert parameter to isMaximize");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
                                      "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    auto session = weakSession_.promote();
    if (session == nullptr) {
        TLOGE(WmsLogTag::WMS_PC, "session is nullptr, id:%{public}d", persistentId_);
        return NapiGetUndefined(env);
    }
    session->SetSidebarBlurMaximize(isMaximize);
    return NapiGetUndefined(env);
}

void JsSceneSession::ProcessSetParentSessionRegister()
{
    auto session = weakSession_.promote();
    if (session == nullptr) {
        TLOGE(WmsLogTag::WMS_SUB, "session is nullptr, id:%{public}d", persistentId_);
        return;
    }
    const char* const where = __func__;
    session->SetParentSessionCallback([weakThis = wptr(this), where](int32_t oldParentWindowId,
        int32_t newParentWindowId) {
        auto jsSceneSession = weakThis.promote();
        if (!jsSceneSession) {
            TLOGNE(WmsLogTag::WMS_SUB, "%{public}s: jsSceneSession is null", where);
            return;
        }
        jsSceneSession->OnSetParentSession(oldParentWindowId, newParentWindowId);
    });
}

void JsSceneSession::OnSetParentSession(int32_t oldParentWindowId, int32_t newParentWindowId)
{
    const char* const where = __func__;
    taskScheduler_->PostMainThreadTask([weakThis = wptr(this), persistentId = persistentId_,
        oldParentWindowId, newParentWindowId, env = env_, where] {
        auto jsSceneSession = weakThis.promote();
        if (!jsSceneSession || jsSceneSessionMap_.find(persistentId) == jsSceneSessionMap_.end()) {
            TLOGNE(WmsLogTag::WMS_SUB, "%{public}s jsSceneSession id:%{public}d has been destroyed",
                where, persistentId);
            return;
        }
        auto jsCallBack = jsSceneSession->GetJSCallback(SET_PARENT_SESSION_CB);
        if (!jsCallBack) {
            TLOGNE(WmsLogTag::WMS_SUB, "%{public}s jsCallBack is nullptr", where);
            return;
        }
        TLOGND(WmsLogTag::WMS_SUB, "%{public}s oldParentWindowId: %{public}d newParentWindowId: %{public}d",
            where, oldParentWindowId, newParentWindowId);
        napi_value jsOldParentWindowId = CreateJsValue(env, oldParentWindowId);
        napi_value jsNewParentWindowId = CreateJsValue(env, newParentWindowId);
        napi_value argv[] = { jsOldParentWindowId, jsNewParentWindowId };
        napi_call_function(env, NapiGetUndefined(env), jsCallBack->GetNapiValue(), ArraySize(argv), argv, nullptr);
    }, where);
}

void JsSceneSession::ProcessUpdateFlagRegister()
{
    auto session = weakSession_.promote();
    if (session == nullptr) {
        TLOGE(WmsLogTag::WMS_MAIN, "session is nullptr, id:%{public}d", persistentId_);
        return;
    }
    const char* const where = __func__;
    session->NotifyUpdateFlagCallback([weakThis = wptr(this), where](const std::string& flag) {
        auto jsSceneSession = weakThis.promote();
        if (!jsSceneSession) {
            TLOGNE(WmsLogTag::WMS_MAIN, "%{public}s: jsSceneSession is null", where);
            return;
        }
        jsSceneSession->OnUpdateFlag(flag);
    });
    TLOGD(WmsLogTag::WMS_MAIN, "success");
}

void JsSceneSession::OnUpdateFlag(const std::string& flag)
{
    const char* const where = __func__;
    taskScheduler_->PostMainThreadTask([weakThis = wptr(this), persistentId = persistentId_,
        flag, env = env_, where] {
        auto jsSceneSession = weakThis.promote();
        if (!jsSceneSession || jsSceneSessionMap_.find(persistentId) == jsSceneSessionMap_.end()) {
            TLOGNE(WmsLogTag::WMS_MAIN, "%{public}s: jsSceneSession id:%{public}d has been destroyed",
                where, persistentId);
            return;
        }
        TLOGND(WmsLogTag::WMS_MAIN, "%{public}s: flag is %{public}s", where, flag.c_str());
        auto jsCallBack = jsSceneSession->GetJSCallback(UPDATE_FLAG_CB);
        if (!jsCallBack) {
            TLOGNE(WmsLogTag::WMS_MAIN, "%{public}s: jsCallBack is nullptr", where);
            return;
        }
        napi_value jsFlag = CreateJsValue(env, flag);
        napi_value argv[] = { jsFlag };
        napi_call_function(env, NapiGetUndefined(env), jsCallBack->GetNapiValue(), ArraySize(argv), argv, nullptr);
    }, __func__);
}

void JsSceneSession::ProcessSessionUpdateFollowScreenChange()
{
    auto session = weakSession_.promote();
    if (session == nullptr) {
        TLOGE(WmsLogTag::DEFAULT, "session is nullptr, id:%{public}d", persistentId_);
        return;
    }
    session->RegisterFollowScreenChangeCallback([weakThis = wptr(this)](bool isFollowScreenChange) {
        auto jsSceneSession = weakThis.promote();
        if (!jsSceneSession) {
            TLOGNE(WmsLogTag::DEFAULT, "jsSceneSession is null");
            return;
        }
        jsSceneSession->OnUpdateFollowScreenChange(isFollowScreenChange);
    });
}

void JsSceneSession::OnUpdateFollowScreenChange(bool isFollowScreenChange)
{
    TLOGI(WmsLogTag::DEFAULT, "follow screen change: %{public}u", isFollowScreenChange);
    std::string info = "OnUpdateFollowScreenChange, isFollowScreenChange:" + std::to_string(isFollowScreenChange);
    taskScheduler_->PostMainThreadTask([weakThis = wptr(this), persistentId = persistentId_,
        isFollowScreenChange, env = env_] {
        auto jsSceneSession = weakThis.promote();
        if (!jsSceneSession || jsSceneSessionMap_.find(persistentId) == jsSceneSessionMap_.end()) {
            TLOGNE(WmsLogTag::DEFAULT, "jsSceneSession id:%{public}d has been destroyed", persistentId);
            return;
        }
        auto jsCallBack = jsSceneSession->GetJSCallback(UPDATE_FOLLOW_SCREEN_CHANGE_CB);
        if (!jsCallBack) {
            TLOGNE(WmsLogTag::DEFAULT, "jsCallBack is nullptr");
            return;
        }
        napi_value followScreenChangeObj = CreateJsValue(env, isFollowScreenChange);
        napi_value argv[] = { followScreenChangeObj };
        napi_call_function(env, NapiGetUndefined(env), jsCallBack->GetNapiValue(), ArraySize(argv), argv, nullptr);
        }, info);
}

napi_value JsSceneSession::OnSetCurrentRotation(napi_env env, napi_callback_info info)
{
    size_t argc = ARGC_FOUR;
    napi_value argv[ARGC_FOUR] = { nullptr };
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc < ARGC_ONE) {
        TLOGE(WmsLogTag::WMS_ROTATION, "Argc is invalid: %{public}zu", argc);
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
                                      "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    int32_t currentRotation = 0;
    if (!ConvertFromJsValue(env, argv[0], currentRotation)) {
        TLOGE(WmsLogTag::WMS_ROTATION, "Failed to convert parameter to currentRotation");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
                                      "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    auto session = weakSession_.promote();
    if (session == nullptr) {
        TLOGE(WmsLogTag::WMS_ROTATION, "session is nullptr, id: %{public}d", persistentId_);
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_STATE_ABNORMALLY),
                                      "Session is nullptr"));
        return NapiGetUndefined(env);
    }
    if (session->SetCurrentRotation(currentRotation) != WSError::WS_OK) {
        TLOGE(WmsLogTag::WMS_ROTATION, "set currentRotation failed, id: %{public}d", persistentId_);
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_STATE_ABNORMALLY),
                                      "Set failed"));
        return NapiGetUndefined(env);
    }
    return NapiGetUndefined(env);
}

void JsSceneSession::ProcessSetSubWindowSourceRegister()
{
    NotifySetSubWindowSourceFunc func = [weakThis = wptr(this), where = __func__](SubWindowSource source) {
        auto jsSceneSession = weakThis.promote();
        if (!jsSceneSession) {
            TLOGNE(WmsLogTag::WMS_SUB, "%{public}s: jsSceneSession is null", where);
            return;
        }
        jsSceneSession->NotifySetSubWindowSource(source);
    };
    auto session = weakSession_.promote();
    if (session == nullptr) {
        TLOGE(WmsLogTag::WMS_SUB, "session is nullptr");
        return;
    }
    session->SetSubWindowSourceFunc(std::move(func));
}

void JsSceneSession::NotifySetSubWindowSource(SubWindowSource source)
{
    auto task = [weakThis = wptr(this), source, env = env_, persistentId = persistentId_, where = __func__] {
        auto jsSceneSession = weakThis.promote();
        if (!jsSceneSession || jsSceneSessionMap_.find(persistentId) == jsSceneSessionMap_.end()) {
            TLOGNE(WmsLogTag::WMS_SUB, "%{public}s: jsSceneSession id: %{public}d has been destroyed",
                where, persistentId);
            return;
        }
        auto jsCallBack = jsSceneSession->GetJSCallback(SET_SUB_WINDOW_SOURCE_CB);
        if (!jsCallBack) {
            TLOGNE(WmsLogTag::WMS_SUB, "%{public}s: jsCallBack is nullptr", where);
            return;
        }
        napi_value jsSource = CreateJsValue(env, source);
        if (jsSource == nullptr) {
            TLOGNE(WmsLogTag::WMS_SUB, "%{public}s: jsSource is nullptr", where);
            return;
        }
        napi_value argv[] = { jsSource };
        napi_call_function(env, NapiGetUndefined(env), jsCallBack->GetNapiValue(), ArraySize(argv), argv, nullptr);
    };
    taskScheduler_->PostMainThreadTask(task, __func__);
}

napi_value JsSceneSession::RequestSpecificSessionClose(napi_env env, napi_callback_info info)
{
    TLOGI(WmsLogTag::WMS_LIFE, "[NAPI]");
    JsSceneSession* me = CheckParamsAndGetThis<JsSceneSession>(env, info);
    return (me != nullptr) ? me->OnRequestSpecificSessionClose(env, info) : nullptr;
}

napi_value JsSceneSession::OnRequestSpecificSessionClose(napi_env env, napi_callback_info info)
{
    TLOGI(WmsLogTag::WMS_LIFE, "[NAPI]");
    auto session = weakSession_.promote();
    if (session == nullptr) {
        TLOGE(WmsLogTag::WMS_LIFE, "session is nullptr, id:%{public}d", persistentId_);
        return NapiGetUndefined(env);
    }
    session->CloseSpecificScene();
    return NapiGetUndefined(env);
}

void JsSceneSession::ProcessAnimateToTargetPropertyRegister()
{
    auto session = weakSession_.promote();
    if (session == nullptr) {
        TLOGE(WmsLogTag::WMS_ANIMATION, "Session is null, id: %{public}d", persistentId_);
        return;
    }
    auto animateToCB = [weakThis = wptr(this), where = __func__](const WindowAnimationProperty& animationProperty,
        const WindowAnimationOption& animationOption) {
        auto jsSceneSession = weakThis.promote();
        if (!jsSceneSession) {
            TLOGNE(WmsLogTag::WMS_ANIMATION, "%{public}s: JsSceneSession is null", where);
            return;
        }
        jsSceneSession->OnAnimateToTargetProperty(animationProperty, animationOption);
    };
    session->RegisterAnimateToCallback(animateToCB);
}

void JsSceneSession::OnAnimateToTargetProperty(const WindowAnimationProperty& animationProperty,
    const WindowAnimationOption& animationOption)
{
    taskScheduler_->PostMainThreadTask([weakThis = wptr(this), where = __func__, env = env_,
        persistentId = persistentId_, animationProperty, animationOption]() {
        auto jsSceneSession = weakThis.promote();
        if (!jsSceneSession || jsSceneSessionMap_.find(persistentId) == jsSceneSessionMap_.end()) {
            TLOGNE(WmsLogTag::WMS_ANIMATION, "%{public}s: JsSceneSession id:%{public}d has been destroyed",
                where, persistentId);
            return;
        }
        TLOGNI(WmsLogTag::WMS_ANIMATION, "%{public}s: Animate to with scale: %{public}f, animationOption: %{public}s",
            where, animationProperty.targetScale, animationOption.ToString().c_str());
        auto jsCallback = jsSceneSession->GetJSCallback(ANIMATE_TO_CB);
        if (!jsCallback) {
            TLOGNE(WmsLogTag::WMS_ANIMATION, "%{public}s: JsCallback is null", where);
            return;
        }
        napi_value jsAnimationProperty = ConvertWindowAnimationPropertyToJsValue(env, animationProperty);
        napi_value jsAnimationOption = ConvertWindowAnimationOptionToJsValue(env, animationOption);
        napi_value argv[] = { jsAnimationProperty, jsAnimationOption };
        napi_call_function(env, NapiGetUndefined(env), jsCallback->GetNapiValue(), ArraySize(argv), argv, nullptr);
    }, __func__);
}

void JsSceneSession::ProcessSceneOutlineParamsChangeRegister()
{
    auto session = weakSession_.promote();
    if (session == nullptr) {
        TLOGE(WmsLogTag::WMS_ANIMATION, "Session is nullptr, id: %{public}d", persistentId_);
        return;
    }
    auto callback = [weakThis = wptr(this), where = __func__] (bool enabled, const OutlineStyleParams& params) {
        auto jsSceneSession = weakThis.promote();
        if (!jsSceneSession) {
            TLOGNE(WmsLogTag::WMS_ANIMATION, "%{public}s jsSceneSession is nullptr.", where);
            return;
        }
        jsSceneSession->OnOutlineParamsChange(enabled, params);
    };
    session->SetOutlineParamsChangeCallback(std::move(callback));
}

void JsSceneSession::OnOutlineParamsChange(bool isOutlineEnabled, const OutlineStyleParams& outlineStyleParams)
{
    auto task = [weakThis = wptr(this), where = __func__, env = env_, persistentId = persistentId_, isOutlineEnabled,
                 outlineStyleParams]() {
        auto jsSceneSession = weakThis.promote();
        if (!jsSceneSession || jsSceneSessionMap_.find(persistentId) == jsSceneSessionMap_.end()) {
            TLOGNE(WmsLogTag::WMS_ANIMATION, "%{public}s: JsSceneSession is invalid, id: %{public}d",
                   where, persistentId);
            return;
        }
        TLOGNI(WmsLogTag::WMS_ANIMATION, "%{public}s, id: %{public}d, enabled: %{public}d, params: %{public}s",
               where, persistentId, isOutlineEnabled, outlineStyleParams.ToString().c_str());
        auto jsCallBack = jsSceneSession->GetJSCallback(SCENE_OUTLINE_PARAMS_CHANGE_CB);
        if (!jsCallBack) {
            TLOGNE(WmsLogTag::WMS_ANIMATION, "%{public}s: JsCallback is nullptr.", where);
            return;
        }
        napi_value enabledObj = CreateJsValue(env, isOutlineEnabled);
        napi_value colorObj = CreateJsValue(env, outlineStyleParams.outlineColor_);
        napi_value widthObj = CreateJsValue(env, outlineStyleParams.outlineWidth_);
        napi_value shapeObj = CreateJsValue(env, static_cast<uint32_t>(outlineStyleParams.outlineShape_));
        napi_value argv[] = { enabledObj, colorObj, widthObj, shapeObj };
        napi_status ret = napi_call_function(env, NapiGetUndefined(env), jsCallBack->GetNapiValue(),
                                             ArraySize(argv), argv, nullptr);
        if (ret != napi_ok) {
            TLOGNE(WmsLogTag::WMS_ANIMATION, "%{public}s: napi call function failed, ret: %{public}d.", where, ret);
        }
    };
    taskScheduler_->PostMainThreadTask(task, __func__);
}

void JsSceneSession::AddRequestTaskInfo(sptr<SceneSession> sceneSession, int32_t requestId, bool needAddRequestInfo)
{
    if (needAddRequestInfo && sceneSession != nullptr) {
        SceneSessionManager::GetInstance().AddRequestTaskInfo(sceneSession, requestId);
    }
}

void JsSceneSession::ProcessRestartAppRegister()
{
    auto session = weakSession_.promote();
    if (session == nullptr) {
        TLOGE(WmsLogTag::WMS_LIFE, "session is nullptr, id:%{public}d", persistentId_);
        return;
    }
    session->SetRestartAppListener([weakThis = wptr(this)](const SessionInfo& info, int32_t callingPid) {
        auto jsSceneSession = weakThis.promote();
        if (!jsSceneSession) {
            TLOGE(WmsLogTag::WMS_LIFE, "jsSceneSession is null");
            return;
        }
        jsSceneSession->OnRestartApp(info, callingPid);
    });
}

void JsSceneSession::OnRestartApp(const SessionInfo& info, int32_t callingPid)
{
    TLOGD(WmsLogTag::WMS_LIFE, "[NAPI]");
    std::shared_ptr<SessionInfo> sessionInfo = std::make_shared<SessionInfo>(info);
    if (!info.isRestartApp_) {
        SessionIdentityInfo identityInfo = { info.bundleName_, info.moduleName_, info.abilityName_,
            info.appIndex_, info.appInstanceKey_, info.windowType_, info.isAtomicService_,
            info.specifiedFlag_ };
        sptr<SceneSession> sceneSession =
            SceneSessionManager::GetInstance().GetSceneSessionByIdentityInfo(identityInfo);
        if (!sceneSession) {
            sceneSession = SceneSessionManager::GetInstance().RequestSceneSession(info);
        } else {
            if (sceneSession->GetCallingPid() == callingPid) {
                sceneSession->NotifyRestart();
            } else {
                TLOGI(WmsLogTag::WMS_LIFE, "restart app callerSession not in same process:%{public}d", callingPid);
                sceneSession->SetRestartInSameProcess(false);
            }
        }
        if (!sceneSession) {
            TLOGE(WmsLogTag::WMS_LIFE, "sceneSession is nullptr");
            return;
        }
        sceneSession->SetRestartCallerPersistentId(info.restartCallerPersistentId_);
        sceneSession->SetRestartApp(true);
        sessionInfo = std::make_shared<SessionInfo>(sceneSession->GetSessionInfo());
    }
    auto task = [weakThis = wptr(this), sessionInfo, persistentId = persistentId_, where = __func__, env = env_] {
        auto jsSceneSession = weakThis.promote();
        if (!jsSceneSession || jsSceneSessionMap_.find(persistentId) == jsSceneSessionMap_.end()) {
            TLOGNE(WmsLogTag::WMS_LIFE, "%{public}s: jsSceneSession id:%{public}d has been destroyed",
                where, persistentId);
            return;
        }
        auto jsCallBack = jsSceneSession->GetJSCallback(RESTART_APP_CB);
        if (!jsCallBack) {
            TLOGNE(WmsLogTag::WMS_LIFE, "%{public}s: jsCallBack is nullptr", where);
            return;
        }
        napi_value jsSessionInfo = CreateJsSessionInfo(env, *sessionInfo);
        if (jsSessionInfo == nullptr) {
            TLOGNE(WmsLogTag::WMS_LIFE, "%{public}s: target sessionInfo is nullptr", where);
            return;
        }
        napi_value argv[] = {jsSessionInfo};
        napi_call_function(env, NapiGetUndefined(env), jsCallBack->GetNapiValue(), ArraySize(argv), argv, nullptr);
    };
    taskScheduler_->PostMainThreadTask(task, __func__);
}

void JsSceneSession::ProcessRotationLockChangeRegister()
{
    auto session = weakSession_.promote();
    if (session == nullptr) {
        TLOGE(WmsLogTag::WMS_ROTATION, "session is nullptr, id:%{public}d", persistentId_);
        return;
    }
    session->RegisterRotationLockChangeCallback([weakThis = wptr(this)](bool locked) {
        auto jsSceneSession = weakThis.promote();
        if (!jsSceneSession) {
            TLOGNE(WmsLogTag::WMS_ROTATION, "jsSceneSession is null");
            return;
        }
        jsSceneSession->OnRotationLockChange(locked);
    });
}
 
void JsSceneSession::OnRotationLockChange(bool locked)
{
    TLOGI(WmsLogTag::WMS_ROTATION, "rotation lock change to: %{public}d", locked);
    std::string info = "OnRotationLockChange, locked:" + std::to_string(locked);
    taskScheduler_->PostMainThreadTask([weakThis = wptr(this), persistentId = persistentId_, locked, env = env_] {
        auto jsSceneSession = weakThis.promote();
        if (!jsSceneSession || jsSceneSessionMap_.find(persistentId) == jsSceneSessionMap_.end()) {
            TLOGNE(WmsLogTag::WMS_ROTATION, "jsSceneSession id:%{public}d has been destroyed", persistentId);
            return;
        }
        auto jsCallBack = jsSceneSession->GetJSCallback(ROTATION_LOCK_CHANGE_CB);
        if (!jsCallBack) {
            TLOGNE(WmsLogTag::WMS_ROTATION, "jsCallBack is nullptr");
            return;
        }
        napi_value rotationLockChangeObj = CreateJsValue(env, locked);
        napi_value argv[] = { rotationLockChangeObj };
        napi_call_function(env, NapiGetUndefined(env), jsCallBack->GetNapiValue(), ArraySize(argv), argv, nullptr);
    }, info);
}

void JsSceneSession::ProcessCompatibleModeChangeRegister()
{
    auto session = weakSession_.promote();
    if (session == nullptr) {
        TLOGE(WmsLogTag::WMS_COMPAT, "session is nullptr, id:%{public}d", persistentId_);
        return;
    }
    session->RegisterCompatibleModeChangeCallback([weakThis = wptr(this)](CompatibleStyleMode mode) {
        auto jsSceneSession = weakThis.promote();
        if (!jsSceneSession) {
            TLOGNE(WmsLogTag::WMS_COMPAT, "jsSceneSession is null");
            return;
        }
        jsSceneSession->OnCompatibleModeChange(mode);
    });
}

void JsSceneSession::OnCompatibleModeChange(CompatibleStyleMode mode)
{
    TLOGI(WmsLogTag::WMS_COMPAT, "compatible mode change to: %{public}d", mode);
    taskScheduler_->PostMainThreadTask([weakThis = wptr(this), persistentId = persistentId_, mode, env = env_] {
        auto jsSceneSession = weakThis.promote();
        if (!jsSceneSession || jsSceneSessionMap_.find(persistentId) == jsSceneSessionMap_.end()) {
            TLOGNE(WmsLogTag::WMS_COMPAT, "jsSceneSession id:%{public}d has been destroyed", persistentId);
            return;
        }
        auto jsCallBack = jsSceneSession->GetJSCallback(COMPATIBLE_MODE_CHANGE_CB);
        if (!jsCallBack) {
            TLOGNE(WmsLogTag::WMS_COMPAT, "jsCallBack is nullptr");
            return;
        }
        napi_value argv[] = { CreateJsValue(env, mode) };
        napi_call_function(env, NapiGetUndefined(env), jsCallBack->GetNapiValue(), ArraySize(argv), argv, nullptr);
    }, __func__);
}
} // namespace OHOS::Rosen
