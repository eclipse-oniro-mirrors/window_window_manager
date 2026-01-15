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

#ifndef OHOS_WINDOW_SCENE_JS_SCENE_SESSION_H
#define OHOS_WINDOW_SCENE_JS_SCENE_SESSION_H

#include <map>

#include <js_runtime_utils.h>
#include <native_engine/native_engine.h>
#include <native_engine/native_value.h>
#include <refbase.h>

#include "interfaces/include/ws_common.h"
#include "session/host/include/scene_session.h"

namespace OHOS::Rosen {
enum class ListenerFuncType : uint32_t {
    PENDING_SCENE_CB,
    CHANGE_SESSION_VISIBILITY_WITH_STATUS_BAR,
    SESSION_STATE_CHANGE_CB,
    UPDATE_TRANSITION_ANIMATION_CB,
    BUFFER_AVAILABLE_CHANGE_CB,
    SESSION_EVENT_CB,
    SESSION_RECT_CHANGE_CB,
    SESSION_WINDOW_LIMITS_CHANGE_CB,
    SESSION_PIP_CONTROL_STATUS_CHANGE_CB,
    SESSION_AUTO_START_PIP_CB,
    CREATE_SUB_SESSION_CB,
    CLEAR_SUB_SESSION_CB,
    BIND_DIALOG_TARGET_CB,
    RAISE_TO_TOP_CB,
    RAISE_TO_TOP_POINT_DOWN_CB,
    CLICK_MODAL_WINDOW_OUTSIDE_CB,
    BACK_PRESSED_CB,
    SESSION_FOCUSABLE_CHANGE_CB,
    SESSION_TOUCHABLE_CHANGE_CB,
    SESSION_TOP_MOST_CHANGE_CB,
    SUB_MODAL_TYPE_CHANGE_CB,
    MAIN_MODAL_TYPE_CHANGE_CB,
    THROW_SLIP_ANIMATION_STATE_CHANGE_CB,
    FULLSCREEN_WATERFALL_MODE_CHANGE_CB,
    CLICK_CB,
    TERMINATE_SESSION_CB,
    TERMINATE_SESSION_CB_NEW,
    TERMINATE_SESSION_CB_TOTAL,
    SESSION_EXCEPTION_CB,
    UPDATE_SESSION_LABEL_CB,
    UPDATE_SESSION_ICON_CB,
    SYSTEMBAR_PROPERTY_CHANGE_CB,
    NEED_AVOID_CB,
    PENDING_SESSION_TO_FOREGROUND_CB,
    PENDING_SESSION_TO_BACKGROUND_FOR_DELEGATOR_CB,
    CUSTOM_ANIMATION_PLAYING_CB,
    NEED_DEFAULT_ANIMATION_FLAG_CHANGE_CB,
    SHOW_WHEN_LOCKED_CB,
    REQUESTED_ORIENTATION_CHANGE_CB,
    RAISE_ABOVE_TARGET_CB,
    RAISE_MAIN_WINDOW_ABOVE_TARGET_CB,
    FORCE_HIDE_CHANGE_CB,
    WINDOW_DRAG_HOT_AREA_CB,
    TOUCH_OUTSIDE_CB,
    SESSIONINFO_LOCKEDSTATE_CHANGE_CB,
    PREPARE_CLOSE_PIP_SESSION,
    LANDSCAPE_MULTI_WINDOW_CB,
    CONTEXT_TRANSPARENT_CB,
    ADJUST_KEYBOARD_LAYOUT_CB,
    LAYOUT_FULL_SCREEN_CB,
    DEFAULT_DENSITY_ENABLED_CB,
    WINDOW_SHADOW_ENABLE_CHANGE_CB,
    NEXT_FRAME_LAYOUT_FINISH_CB,
    PRIVACY_MODE_CHANGE_CB,
    RESTORE_MAIN_WINDOW_CB,
    RESTORE_FLOAT_MAIN_WINDOW_CB,
    MAIN_WINDOW_TOP_MOST_CHANGE_CB,
    TITLE_DOCK_HOVER_SHOW_CB,
    SET_WINDOW_RECT_AUTO_SAVE_CB,
    UPDATE_APP_USE_CONTROL_CB,
    SESSION_DISPLAY_ID_CHANGE_CB,
    SET_SUPPORT_WINDOW_MODES_CB,
    WINDOW_MOVING_CB,
    SESSION_LOCK_STATE_CHANGE_CB,
    UPDATE_SESSION_LABEL_AND_ICON_CB,
    KEYBOARD_STATE_CHANGE_CB,
    KEYBOARD_EFFECT_OPTION_CHANGE_CB,
    SET_WINDOW_CORNER_RADIUS_CB,
    HIGHLIGHT_CHANGE_CB,
    FOLLOW_PARENT_RECT_CB,
    SET_PARENT_SESSION_CB,
    UPDATE_FLAG_CB,
    Z_LEVEL_CHANGE_CB,
    SESSION_GET_TARGET_ORIENTATION_CONFIG_INFO_CB,
    UPDATE_PIP_TEMPLATE_INFO_CB,
    SET_PIP_PARENT_WINDOWID_CB,
    UPDATE_FOLLOW_SCREEN_CHANGE_CB,
    USE_IMPLICIT_ANIMATION_CB,
    WINDOW_ANCHOR_INFO_CHANGE_CB,
    SET_WINDOW_SHADOWS_CB,
    SET_SUB_WINDOW_SOURCE_CB,
    ANIMATE_TO_CB,
    PENDING_SESSION_TO_BACKGROUND_CB,
    BATCH_PENDING_SCENE_ACTIVE_CB,
    FLOATING_BALL_UPDATE_CB,
    FLOATING_BALL_STOP_CB,
    FLOATING_BALL_RESTORE_MAIN_WINDOW_CB,
    SCENE_OUTLINE_PARAMS_CHANGE_CB,
    RESTART_APP_CB,
    CALLING_SESSION_ID_CHANGE_CB,
    ROTATION_LOCK_CHANGE_CB,
    SNAPSHOT_SKIP_CHANGE_CB,
    COMPATIBLE_MODE_CHANGE_CB,
};

class SceneSession;
class JsSceneSession : public RefBase {
public:
    JsSceneSession(napi_env env, const sptr<SceneSession>& session);
    ~JsSceneSession();

    static napi_value Create(napi_env env, const sptr<SceneSession>& session);

    sptr<SceneSession> GetNativeSession() const;

private:

    /*
     * Window Lifecycle
     */
    void ProcessPendingSceneSessionActivationRegister();
    void ProcessBatchPendingSceneSessionsActivationRegister();
    void ProcessSessionStateChangeRegister();
    void ProcessUpdateTransitionAnimationRegister();
    void ProcessSessionEventRegister();
    void ProcessTerminateSessionRegister();
    void ProcessTerminateSessionRegisterNew();
    void ProcessTerminateSessionRegisterTotal();
    void ProcessSessionExceptionRegister();
    void ProcessPendingSessionToForegroundRegister();
    void ProcessPendingSessionToBackgroundRegister();
    void ProcessPendingSessionToBackgroundForDelegatorRegister();
    void ProcessSessionLockStateChangeRegister();
    void ProcessSessionUpdateFollowScreenChange();
    void OnSessionLockStateChange(bool isLockedState);
    sptr<SceneSession> GenSceneSession(SessionInfo& info, bool needAddRequestInfo = false);
    void ReuseSession(sptr<SceneSession>& sceneSession, SessionInfo& info);
    void AddRequestTaskInfo(sptr<SceneSession> sceneSession, int32_t requestId, bool needAddRequestInfo);
    void PendingSessionActivation(SessionInfo& info);
    void PendingSessionActivationInner(std::shared_ptr<SessionInfo> sessionInfo);
    void BatchPendingSessionsActivation(const std::vector<std::shared_ptr<SessionInfo>>& sessionInfos,
        const std::vector<std::shared_ptr<PendingSessionActivationConfig>>& configs);
    void BatchPendingSessionsActivationInner(const std::vector<std::shared_ptr<SessionInfo>>& sessionInfos,
        const std::vector<std::shared_ptr<PendingSessionActivationConfig>>& configs);
    void OnSessionStateChange(const SessionState& state);
    void OnUpdateTransitionAnimation(const WindowTransitionType& type, const TransitionAnimation& animation);
    void OnSessionEvent(uint32_t eventId, const SessionEventParam& param);
    void TerminateSession(const SessionInfo& info);
    void TerminateSessionNew(const SessionInfo& info, bool needStartCaller,
        bool isFromBroker, bool isForceClean = false);
    void TerminateSessionTotal(const SessionInfo& info, TerminateType terminateType);
    void OnSessionException(const SessionInfo& info, const ExceptionInfo& exceptionInfo, bool startFail);
    void PendingSessionToForeground(const SessionInfo& info);
    void PendingSessionToBackground(const SessionInfo& info, const BackgroundParams& params);
    void PendingSessionToBackgroundForDelegator(const SessionInfo& info, bool shouldBackToCaller);
    static napi_value SetTemporarilyShowWhenLocked(napi_env env, napi_callback_info info);

    static napi_value ActivateDragBySystem(napi_env env, napi_callback_info info);
    static napi_value RegisterCallback(napi_env env, napi_callback_info info);
    static napi_value UpdateNativeVisibility(napi_env env, napi_callback_info info);
    static napi_value SetShowRecent(napi_env env, napi_callback_info info);
    static napi_value SetZOrder(napi_env env, napi_callback_info info);
    static napi_value SetTouchable(napi_env env, napi_callback_info info);
    static napi_value SetWindowInputType(napi_env env, napi_callback_info info);
    static napi_value SetExpandInputFlag(napi_env env, napi_callback_info info);
    static napi_value SetSystemActive(napi_env env, napi_callback_info info);
    static napi_value SetPrivacyMode(napi_env env, napi_callback_info info);
    static napi_value SetFloatingScale(napi_env env, napi_callback_info info);
    static napi_value SetIsMidScene(napi_env env, napi_callback_info info);
    static napi_value SetSystemSceneOcclusionAlpha(napi_env env, napi_callback_info info);
    static napi_value ResetOcclusionAlpha(napi_env env, napi_callback_info info);
    static napi_value SetSystemSceneForceUIFirst(napi_env env, napi_callback_info info);
    static napi_value SetUIFirstSwitch(napi_env env, napi_callback_info info);
    static napi_value MarkSystemSceneUIFirst(napi_env env, napi_callback_info info);
    static napi_value SetFocusable(napi_env env, napi_callback_info info);
    static napi_value SetFocusableOnShow(napi_env env, napi_callback_info info);
    static napi_value SetSystemFocusable(napi_env env, napi_callback_info info);
    static napi_value SetSystemSceneBlockingFocus(napi_env env, napi_callback_info info);
    static napi_value UpdateSizeChangeReason(napi_env env, napi_callback_info info);
    static napi_value OpenKeyboardSyncTransaction(napi_env env, napi_callback_info info);
    static napi_value CloseKeyboardSyncTransaction(napi_env env, napi_callback_info info);
    static napi_value NotifyKeyboardAnimationCompleted(napi_env env, napi_callback_info info);
    static napi_value CallingWindowStateChange(napi_env env, napi_callback_info info);
    static napi_value SetScale(napi_env env, napi_callback_info info);
    static napi_value SetWindowLastSafeRect(napi_env env, napi_callback_info info);
    static napi_value SetMovable(napi_env env, napi_callback_info info);
    static napi_value SetSplitButtonVisible(napi_env env, napi_callback_info info);
    static napi_value RequestHideKeyboard(napi_env env, napi_callback_info info);
    static napi_value SetSCBKeepKeyboard(napi_env env, napi_callback_info info);
    static napi_value SetOffset(napi_env env, napi_callback_info info);
    static napi_value SetExitSplitOnBackground(napi_env env, napi_callback_info info);
    static napi_value SetWaterMarkFlag(napi_env env, napi_callback_info info);
    static napi_value SetPipActionEvent(napi_env env, napi_callback_info info);
    static napi_value NotifyPipOcclusionChange(napi_env env, napi_callback_info info);
    static napi_value NotifyPipSizeChange(napi_env env, napi_callback_info info);
    static napi_value NotifyPipActiveWindowStatusChange(napi_env env, napi_callback_info info);
    static napi_value SetPiPControlEvent(napi_env env, napi_callback_info info);
    static napi_value NotifyDisplayStatusBarTemporarily(napi_env env, napi_callback_info info);
    static napi_value SetSkipDraw(napi_env env, napi_callback_info info);
    static napi_value SyncScenePanelGlobalPosition(napi_env env, napi_callback_info info);
    static napi_value UnSyncScenePanelGlobalPosition(napi_env env, napi_callback_info info);
    static napi_value SetNeedSyncSessionRect(napi_env env, napi_callback_info info);
    static napi_value MaskSupportEnterWaterfallMode(napi_env env, napi_callback_info info);
    static napi_value UpdateFullScreenWaterfallMode(napi_env env, napi_callback_info info);
    static void BindNativeMethod(napi_env env, napi_value objValue, const char* moduleName);
    static void BindNativeMethodForKeyboard(napi_env env, napi_value objValue, const char* moduleName);
    static void BindNativeMethodForCompatiblePcMode(napi_env env, napi_value objValue, const char* moduleName);
    static void BindNativeMethodForPcAppInPadNormal(napi_env env, napi_value objValue, const char* moduleName);
    static void BindNativeMethodForSCBSystemSession(napi_env env, napi_value objValue, const char* moduleName);
    static void BindNativeMethodForFocus(napi_env env, napi_value objValue, const char* moduleName);
    static void BindNativeMethodForWaterfall(napi_env env, napi_value objValue, const char* moduleName);
    static napi_value SetSkipSelfWhenShowOnVirtualScreen(napi_env env, napi_callback_info info);
    static napi_value SetSkipEventOnCastPlus(napi_env env, napi_callback_info info);
    static napi_value ToggleCompatibleMode(napi_env, napi_callback_info info);
    static napi_value SetAppSupportPhoneInPc(napi_env env, napi_callback_info info);
    static napi_value SetUniqueDensityDpiFromSCB(napi_env env, napi_callback_info info);
    static napi_value SetBlank(napi_env env, napi_callback_info info);
    static napi_value RemoveBlank(napi_env env, napi_callback_info info);
    static napi_value AddSnapshot(napi_env env, napi_callback_info info);
    static napi_value RemoveSnapshot(napi_env env, napi_callback_info info);
    static napi_value SetBufferAvailableCallbackEnable(napi_env env, napi_callback_info info);
    static napi_value SyncDefaultRequestedOrientation(napi_env env, napi_callback_info info);
    static napi_value SetIsPcAppInPad(napi_env env, napi_callback_info info);
    static napi_value SetStartingWindowExitAnimationFlag(napi_env env, napi_callback_info info);
    static napi_value PcAppInPadNormalClose(napi_env env, napi_callback_info info);
    static napi_value SetWindowEnableDragBySystem(napi_env env, napi_callback_info info);
    static napi_value SetIsPendingToBackgroundState(napi_env env, napi_callback_info info);
    static napi_value SetIsActivatedAfterScreenLocked(napi_env env, napi_callback_info info);
    static napi_value SetLabel(napi_env env, napi_callback_info info);
    static napi_value SetFrameGravity(napi_env env, napi_callback_info info);
    static napi_value SetUseStartingWindowAboveLocked(napi_env env, napi_callback_info info);
    static napi_value SaveSnapshotSync(napi_env env, napi_callback_info info);
    static napi_value SaveSnapshotAsync(napi_env env, napi_callback_info info);
    static napi_value SetBorderUnoccupied(napi_env env, napi_callback_info info);
    static napi_value SetEnableAddSnapshot(napi_env env, napi_callback_info info);
    static napi_value SetFreezeImmediately(napi_env env, napi_callback_info info);
    static napi_value SendContainerModalEvent(napi_env env, napi_callback_info info);
    static napi_value SetExclusivelyHighlighted(napi_env env, napi_callback_info info);
    static napi_value SetColorSpace(napi_env env, napi_callback_info info);
    static napi_value SetSnapshotSkip(napi_env env, napi_callback_info info);
    static napi_value ThrowSlipDirectly(napi_env env, napi_callback_info info);
    static napi_value AddSidebarBlur(napi_env env, napi_callback_info info);
    static napi_value SetSidebarBlur(napi_env env, napi_callback_info info);
    static napi_value NotifyRotationProperty(napi_env env, napi_callback_info info);
    static napi_value NotifyPageRotationIsIgnored(napi_env env, napi_callback_info info);
    static napi_value SetCurrentRotation(napi_env env, napi_callback_info info);
    static napi_value SetSidebarBlurMaximize(napi_env env, napi_callback_info info);
    static napi_value RequestSpecificSessionClose(napi_env env, napi_callback_info info);
    static napi_value SendFbActionEvent(napi_env env, napi_callback_info info);
    static napi_value GetUid(napi_env env, napi_callback_info info);
    static napi_value CreateSessionInfosNapiValue(
        napi_env env, const std::vector<std::shared_ptr<SessionInfo>>& sessionInfos);
    static napi_value CreatePendingInfosNapiValue(napi_env env,
        const std::vector<std::shared_ptr<SessionInfo>>& sessionInfos,
        const std::vector<std::shared_ptr<PendingSessionActivationConfig>>& configs);
    static napi_value SetPcAppInpadCompatibleMode(napi_env env, napi_callback_info info);
    static napi_value SetPcAppInpadSpecificSystemBarInvisible(napi_env env, napi_callback_info info);
    static napi_value SetPcAppInpadOrientationLandscape(napi_env env, napi_callback_info info);
    static napi_value UpdateSceneAnimationConfig(napi_env env, napi_callback_info info);
    static napi_value SetMobileAppInPadLayoutFullScreen(napi_env env, napi_callback_info info);
    /*
     * PC Window
     */
    static napi_value GetZOrder(napi_env env, napi_callback_info info);

    napi_value OnActivateDragBySystem(napi_env env, napi_callback_info info);
    napi_value OnRegisterCallback(napi_env env, napi_callback_info info);
    napi_value OnUpdateNativeVisibility(napi_env env, napi_callback_info info);
    napi_value OnSetShowRecent(napi_env env, napi_callback_info info);
    napi_value OnSetZOrder(napi_env env, napi_callback_info info);
    napi_value OnSetTouchable(napi_env env, napi_callback_info info);
    napi_value OnSetWindowInputType(napi_env env, napi_callback_info info);
    napi_value OnSetExpandInputFlag(napi_env env, napi_callback_info info);
    napi_value OnSetSystemActive(napi_env env, napi_callback_info info);
    napi_value OnSetPrivacyMode(napi_env env, napi_callback_info info);
    napi_value OnSetFloatingScale(napi_env env, napi_callback_info info);
    napi_value OnSetIsMidScene(napi_env env, napi_callback_info info);
    napi_value OnSetSystemSceneOcclusionAlpha(napi_env env, napi_callback_info info);
    napi_value OnResetOcclusionAlpha(napi_env env, napi_callback_info info);
    napi_value OnSetSystemSceneForceUIFirst(napi_env env, napi_callback_info info);
    napi_value OnSetUIFirstSwitch(napi_env env, napi_callback_info info);
    napi_value OnMarkSystemSceneUIFirst(napi_env env, napi_callback_info info);
    napi_value OnSetFocusable(napi_env env, napi_callback_info info);
    napi_value OnSetFocusableOnShow(napi_env env, napi_callback_info info);
    napi_value OnSetSystemFocusable(napi_env env, napi_callback_info info);
    napi_value OnSetSystemSceneBlockingFocus(napi_env env, napi_callback_info info);
    napi_value OnUpdateSizeChangeReason(napi_env env, napi_callback_info info);
    napi_value OnOpenKeyboardSyncTransaction(napi_env env, napi_callback_info info);
    napi_value OnCloseKeyboardSyncTransaction(napi_env env, napi_callback_info info);
    napi_value OnCallingWindowStateChange(napi_env env, napi_callback_info info);
    napi_value OnNotifyKeyboardAnimationCompleted(napi_env env, napi_callback_info info);
    napi_value OnSetScale(napi_env env, napi_callback_info info);
    napi_value OnSetWindowLastSafeRect(napi_env env, napi_callback_info info);
    napi_value OnSetMovable(napi_env env, napi_callback_info info);
    napi_value OnSetSplitButtonVisible(napi_env env, napi_callback_info info);
    napi_value OnRequestHideKeyboard(napi_env env, napi_callback_info info);
    napi_value OnSetSCBKeepKeyboard(napi_env env, napi_callback_info info);
    napi_value OnSetOffset(napi_env env, napi_callback_info info);
    napi_value OnSetExitSplitOnBackground(napi_env env, napi_callback_info info);
    napi_value OnSetWaterMarkFlag(napi_env env, napi_callback_info info);
    napi_value OnSetPipActionEvent(napi_env env, napi_callback_info info);
    napi_value OnSetPiPControlEvent(napi_env env, napi_callback_info info);
    napi_value OnNotifyPipOcclusionChange(napi_env env, napi_callback_info info);
    napi_value OnNotifyPipSizeChange(napi_env env, napi_callback_info info);
    napi_value OnNotifyPiPActiveStatusChange(napi_env env, napi_callback_info info);
    napi_value OnNotifyDisplayStatusBarTemporarily(napi_env env, napi_callback_info info);
    napi_value OnSetTemporarilyShowWhenLocked(napi_env env, napi_callback_info info);
    napi_value OnSetSkipDraw(napi_env env, napi_callback_info info);
    napi_value OnSetSkipSelfWhenShowOnVirtualScreen(napi_env env, napi_callback_info info);
    napi_value OnSetSkipEventOnCastPlus(napi_env env, napi_callback_info info);
    napi_value OnToggleCompatibleMode(napi_env env, napi_callback_info info);
    napi_value OnSetAppSupportPhoneInPc(napi_env env, napi_callback_info info);
    napi_value OnSetUniqueDensityDpiFromSCB(napi_env env, napi_callback_info info);
    napi_value OnSetBlank(napi_env env, napi_callback_info info);
    napi_value OnRemoveBlank(napi_env env, napi_callback_info info);
    napi_value OnAddSnapshot(napi_env env, napi_callback_info info);
    napi_value OnRemoveSnapshot(napi_env env, napi_callback_info info);
    napi_value OnSetBufferAvailableCallbackEnable(napi_env env, napi_callback_info info);
    napi_value OnSyncDefaultRequestedOrientation(napi_env env, napi_callback_info info);
    napi_value OnSetIsPcAppInPad(napi_env env, napi_callback_info info);
    napi_value OnSetStartingWindowExitAnimationFlag(napi_env env, napi_callback_info info);
    napi_value OnPcAppInPadNormalClose(napi_env env, napi_callback_info info);
    napi_value OnSyncScenePanelGlobalPosition(napi_env env, napi_callback_info info);
    napi_value OnUnSyncScenePanelGlobalPosition(napi_env env, napi_callback_info info);
    napi_value OnSetNeedSyncSessionRect(napi_env env, napi_callback_info info);
    napi_value OnSetWindowEnableDragBySystem(napi_env env, napi_callback_info info);
    napi_value OnSetIsPendingToBackgroundState(napi_env env, napi_callback_info info);
    napi_value OnSetIsActivatedAfterScreenLocked(napi_env env, napi_callback_info info);
    napi_value OnSetLabel(napi_env env, napi_callback_info info);
    napi_value OnSetFrameGravity(napi_env env, napi_callback_info info);
    napi_value OnSetUseStartingWindowAboveLocked(napi_env env, napi_callback_info info);
    napi_value OnSaveSnapshotSync(napi_env env, napi_callback_info info);
    napi_value OnSaveSnapshotAsync(napi_env env, napi_callback_info info);
    napi_value OnSetBorderUnoccupied(napi_env env, napi_callback_info info);
    napi_value OnSetEnableAddSnapshot(napi_env env, napi_callback_info info);
    napi_value OnSetFreezeImmediately(napi_env env, napi_callback_info info);
    napi_value OnMaskSupportEnterWaterfallMode(napi_env env, napi_callback_info info);
    napi_value OnUpdateFullScreenWaterfallMode(napi_env env, napi_callback_info info);
    napi_value OnSendContainerModalEvent(napi_env env, napi_callback_info info);
    napi_value OnSetExclusivelyHighlighted(napi_env env, napi_callback_info info);
    napi_value OnSetColorSpace(napi_env env, napi_callback_info info);
    napi_value OnSetSnapshotSkip(napi_env env, napi_callback_info info);
    napi_value OnThrowSlipDirectly(napi_env env, napi_callback_info info);
    napi_value OnAddSidebarBlur(napi_env env, napi_callback_info info);
    napi_value OnSetSidebarBlur(napi_env env, napi_callback_info info);
    napi_value OnNotifyRotationProperty(napi_env env, napi_callback_info info);
    napi_value OnNotifyPageRotationIsIgnored(napi_env env, napi_callback_info info);
    napi_value OnNotifyRotationChange(napi_env env, napi_callback_info info);
    napi_value OnSetCurrentRotation(napi_env env, napi_callback_info info);
    napi_value OnSetSidebarBlurMaximize(napi_env env, napi_callback_info info);
    static napi_value GetJsPanelSessionObj(napi_env env, const sptr<SceneSession>& session);
    napi_value OnRequestSpecificSessionClose(napi_env env, napi_callback_info info);
    napi_value OnSendFbActionEvent(napi_env env, napi_callback_info info);
    napi_value OnSetPcAppInpadCompatibleMode(napi_env env, napi_callback_info info);
    napi_value OnSetPcAppInpadSpecificSystemBarInvisible(napi_env env, napi_callback_info info);
    napi_value OnSetPcAppInpadOrientationLandscape(napi_env env, napi_callback_info info);
    napi_value OnUpdateSceneAnimationConfig(napi_env env, napi_callback_info info);
    napi_value OnGetUid(napi_env env, napi_callback_info info);
    napi_value OnSetMobileAppInPadLayoutFullScreen(napi_env env, napi_callback_info info);

    /*
     * PC Window
     */
    napi_value OnGetZOrder(napi_env env, napi_callback_info info);

    bool IsCallbackRegistered(napi_env env, const std::string& type, napi_value jsListenerObject);
    void ProcessChangeSessionVisibilityWithStatusBarRegister();
    void ProcessBufferAvailableChangeRegister();
    void ProcessCreateSubSessionRegister();
    void ProcessClearSubSessionRegister();
    void ProcessBindDialogTargetRegister();
    void ProcessSessionRectChangeRegister();
    void ProcessSessionWindowLimitsChangeRegister();
    void ProcessFloatingBallUpdateRegister();
    void ProcessFloatingBallStopRegister();
    void ProcessFloatingBallRestoreMainWindowRegister();
    void ProcessSessionDisplayIdChangeRegister();
    void ProcessSessionPiPControlStatusChangeRegister();
    void ProcessAutoStartPiPStatusChangeRegister();
    void ProcessRaiseToTopRegister();
    void ProcessRaiseToTopForPointDownRegister();
    void ProcessClickModalWindowOutsideRegister();
    void ProcessBackPressedRegister();
    void ProcessSessionFocusableChangeRegister();
    void ProcessSessionTouchableChangeRegister();
    void ProcessSessionTopmostChangeRegister();
    void ProcessMainWindowTopmostChangeRegister();
    void ProcessSubWindowZLevelChangeRegister();
    void ProcessSubModalTypeChangeRegister();
    void ProcessMainModalTypeChangeRegister();
    void RegisterThrowSlipAnimationStateChangeCallback();
    void RegisterFullScreenWaterfallModeChangeCallback();
    void ProcessClickRegister();
    void ProcessUpdateSessionLabelRegister();
    void ProcessUpdateSessionIconRegister();
    void ProcessSystemBarPropertyChangeRegister();
    void ProcessNeedAvoidRegister();
    void ProcessSessionDefaultAnimationFlagChangeRegister();
    void ProcessIsCustomAnimationPlaying();
    void ProcessShowWhenLockedRegister();
    void ProcessRequestedOrientationChange();
    void ProcessRaiseAboveTargetRegister();
    void ProcessRaiseMainWindowAboveTargetRegister();
    void ProcessForceHideChangeRegister();
    void ProcessWindowDragHotAreaRegister();
    void ProcessTouchOutsideRegister();
    void ProcessSessionInfoLockedStateChangeRegister();
    void ProcessPrepareClosePiPSessionRegister();
    void ProcessLandscapeMultiWindowRegister();
    void ProcessContextTransparentRegister();
    void ProcessAdjustKeyboardLayoutRegister();
    void ProcessLayoutFullScreenChangeRegister();
    void ProcessDefaultDensityEnabledRegister();
    void ProcessWindowShadowEnableChangeRegister();
    void ProcessTitleAndDockHoverShowChangeRegister();
    void ProcessRestoreMainWindowRegister();
    void ProcessRestoreFloatMainWindowRegister();
    void ProcessFrameLayoutFinishRegister();
    void ProcessRegisterCallback(ListenerFuncType listenerFuncType);
    void ProcessSetWindowRectAutoSaveRegister();
    void RegisterUpdateAppUseControlCallback();
    void ProcessWindowMovingRegister();
    void ProcessUpdateSessionLabelAndIconRegister();
    void ProcessKeyboardStateChangeRegister();
    void ProcessCallingSessionIdChangeRegister();
    void ProcessKeyboardEffectOptionChangeRegister();
    void ProcessSetHighlightChangeRegister();
    void ProcessWindowAnchorInfoChangeRegister();
    void ProcessFollowParentRectRegister();
    void ProcessGetTargetOrientationConfigInfoRegister();
    void ProcessUpdatePiPTemplateInfoRegister();
    void ProcessSetPiPParentWindowIdRegister();
    void ProcessUseImplicitAnimationChangeRegister();
    void ProcessSetSubWindowSourceRegister();
    void ProcessAnimateToTargetPropertyRegister();
    void ProcessSceneOutlineParamsChangeRegister();
    void ProcessRestartAppRegister();

    /*
     * Window Property
    */
    void ProcessSetWindowCornerRadiusRegister();
    void ProcessSetWindowShadowsRegister();
    void ProcessRotationLockChangeRegister();
    void ProcessSnapshotSkipChangeRegister();

    /*
     * PC Window Layout
     */
    void ProcessSetSupportedWindowModesRegister();
    void ProcessUpdateFlagRegister();

    /*
     * Sub Window
     */
    void ProcessSetParentSessionRegister();
    void OnSetParentSession(int32_t oldParentWindowId, int32_t newParentWindowId);

    void ChangeSessionVisibilityWithStatusBar(const SessionInfo& info, bool visible);
    void ChangeSessionVisibilityWithStatusBarInner(std::shared_ptr<SessionInfo> sessionInfo, bool visible);
    void OnBufferAvailableChange(const bool isBufferAvailable, bool startWindowInvisible = false);
    void OnCreateSubSession(const sptr<SceneSession>& sceneSession);
    void OnClearSubSession(int32_t subPersistentId);
    void OnBindDialogTarget(const sptr<SceneSession>& sceneSession);
    void OnSessionRectChange(const WSRect& rect,
        SizeChangeReason reason = SizeChangeReason::UNDEFINED, DisplayId displayId = DISPLAY_ID_INVALID,
        const RectAnimationConfig& rectAnimationConfig = {});
    void OnSessionWindowLimitsChange(const WindowLimits& windowlimits);
    void OnFloatingBallUpdate(const FloatingBallTemplateInfo& fbTemplateInfo);
    void OnFloatingBallStop();
    void OnFloatingBallRestoreMainWindow(const std::shared_ptr<AAFwk::Want>& want);
    void OnSessionDisplayIdChange(uint64_t displayId);
    void OnSessionPiPControlStatusChange(WsPiPControlType controlType, WsPiPControlStatus status);
    void OnAutoStartPiPStatusChange(bool isAutoStart, uint32_t priority, uint32_t width, uint32_t height);
    void OnRaiseToTop();
    void OnRaiseToTopForPointDown();
    void OnClickModalWindowOutside();
    void OnRaiseAboveTarget(int32_t subWindowId);
    void OnRaiseMainWindowAboveTarget(int32_t targetId);
    void OnBackPressed(bool needMoveToBackground);
    void OnSessionFocusableChange(bool isFocusable);
    void OnSessionTouchableChange(bool touchable);
    void OnSessionTopmostChange(bool topmost);
    void OnMainWindowTopmostChange(bool isTopmost);
    void OnSubSessionZLevelChange(int32_t zLevel);
    void OnSubModalTypeChange(SubWindowModalType subWindowModalType);
    void OnMainModalTypeChange(bool isModal);
    void OnThrowSlipAnimationStateChange(bool isAnimating, bool isFullScreen);
    void OnFullScreenWaterfallModeChange(bool isWaterfallMode);
    void OnClick(bool requestFocus, bool isClick);
    void UpdateSessionLabel(const std::string& label);
    void UpdateSessionIcon(const std::string& iconPath);
    void OnSystemBarPropertyChange(const std::unordered_map<WindowType, SystemBarProperty>& propertyMap);
    void OnNeedAvoid(bool status);
    void OnDefaultAnimationFlagChange(bool isNeedDefaultAnimationFlag);
    void OnIsCustomAnimationPlaying(bool status);
    void OnShowWhenLocked(bool showWhenLocked);
    void OnReuqestedOrientationChange(uint32_t orientation, bool needAnimation = true);
    void OnForceHideChange(bool hide);
    void OnWindowDragHotArea(DisplayId displayId, uint32_t type, SizeChangeReason reason);
    void OnTouchOutside();
    void OnSessionInfoLockedStateChange(bool lockedState);
    void OnPrepareClosePiPSession();
    void OnContextTransparent();
    void SetLandscapeMultiWindow(bool isLandscapeMultiWindow);
    void OnAdjustKeyboardLayout(const KeyboardLayoutParams& params);
    void OnLayoutFullScreenChange(bool isLayoutFullScreen);
    void OnDefaultDensityEnabled(bool isDefaultDensityEnabled);
    void OnWindowShadowEnableChange(bool isEnabled);
    void OnTitleAndDockHoverShowChange(bool isTitleHoverShown = true, bool isDockHoverShown = true);
    void RestoreMainWindow(bool isAppSupportPhoneInPc, int32_t callingPid, uint32_t callingToken);
    void RestoreFloatMainWindow(const std::shared_ptr<AAFwk::WantParams>& wantParameters);
    void NotifyFrameLayoutFinish();
    void ProcessPrivacyModeChangeRegister();
    void NotifyPrivacyModeChange(bool isPrivacyMode);
    void OnSetWindowRectAutoSave(bool enabled, bool isSaveBySpecifiedFlag);
    void OnUpdateAppUseControl(ControlAppType type, bool isNeedControl, bool isControlRecentOnly);
    void OnWindowMoving(DisplayId displayId, int32_t pointerX, int32_t pointerY);
    void UpdateSessionLabelAndIcon(const std::string& label, const std::shared_ptr<Media::PixelMap>& icon);
    void OnKeyboardStateChange(SessionState state, const KeyboardEffectOption& effectOption,
        const uint32_t callingSessionId, const DisplayId targetDisplayId);
    void OnKeyboardEffectOptionChange(const KeyboardEffectOption& effectOption);
    void NotifyHighlightChange(bool isHighlight);
    void NotifyWindowAnchorInfoChange(const WindowAnchorInfo& windowAnchorInfo);
    void NotifyFollowParentRect(bool isFollow);
    void OnGetTargetOrientationConfigInfo(uint32_t targetOrientation);
    void OnRotationChange(int32_t persistentId, bool isRegister);
    void OnUpdatePiPTemplateInfo(PiPTemplateInfo& pipTemplateInfo);
    void OnSetPiPParentWindowId(uint32_t windowId);
    void OnUpdateFollowScreenChange(bool isFollowScreenChange);
    void OnUseImplicitAnimationChange(bool useImplicit);
    void NotifySetSubWindowSource(SubWindowSource source);
    void OnAnimateToTargetProperty(const WindowAnimationProperty& animationProperty,
        const WindowAnimationOption& animationOption);
    void OnOutlineParamsChange(bool isOutlineEnabled, const OutlineStyleParams& outlineStyleParams);
    void OnRestartApp(const SessionInfo& info, int32_t callingPid);
    void OnCallingSessionIdChange(uint32_t callingSessionId);

    /*
     * Window Property
    */
    void OnSetWindowCornerRadius(float cornerRadius);
    void OnSetWindowShadows(const ShadowsInfo& shadowsInfo);
    void OnRotationLockChange(bool locked);
    void OnSnapshotSkipChange(bool isSkip);

    /*
     * PC Window Layout
     */
    void OnSetSupportedWindowModes(std::vector<AppExecFwk::SupportWindowMode>&& supportedWindowModes);
    void OnUpdateFlag(const std::string& flag);

    /*
     * Compatible Mode
     */
    void ProcessCompatibleModeChangeRegister();
    void OnCompatibleModeChange(CompatibleStyleMode mode);

    bool HandleCloseKeyboardSyncTransactionKeyboardBaseInfo(napi_env env,
        napi_value argv[], int index, KeyboardBaseInfo& keyboardBaseInfo);
    bool HandleCloseKeyboardSyncTransactionKeyboardAnimationRectConfig(napi_env env,
        napi_value argv[], int index, KeyboardAnimationRectConfig& keyboardAnimationRectConfig);
    bool HandleCallingWindowInfoData(napi_env env,
        napi_value argv[], int index, CallingWindowInfoData& callingWindowInfoData);

    static void Finalizer(napi_env env, void* data, void* hint);

    std::shared_ptr<NativeReference> GetJSCallback(const std::string& functionName);
    void ClearCbMap();

    napi_env env_;
    wptr<SceneSession> weakSession_ = nullptr;
    int32_t persistentId_ = -1;
    std::shared_mutex jsCbMapMutex_;
    std::map<std::string, std::shared_ptr<NativeReference>> jsCbMap_;
    std::shared_ptr<MainThreadScheduler> taskScheduler_;
    static std::map<int32_t, napi_ref> jsSceneSessionMap_;
};
} // namespace OHOS::Rosen

#endif // OHOS_WINDOW_SCENE_JS_SCENE_SESSION_H
