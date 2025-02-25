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
#include "js_scene_utils.h"
#include "task_scheduler.h"

namespace OHOS::Rosen {
enum class ListenerFuncType : uint32_t {
    PENDING_SCENE_CB,
    CHANGE_SESSION_VISIBILITY_WITH_STATUS_BAR,
    SESSION_STATE_CHANGE_CB,
    BUFFER_AVAILABLE_CHANGE_CB,
    SESSION_EVENT_CB,
    SESSION_RECT_CHANGE_CB,
    SESSION_PIP_CONTROL_STATUS_CHANGE_CB,
    SESSION_AUTO_START_PIP_CB,
    CREATE_SUB_SESSION_CB,
    BIND_DIALOG_TARGET_CB,
    RAISE_TO_TOP_CB,
    RAISE_TO_TOP_POINT_DOWN_CB,
    BACK_PRESSED_CB,
    SESSION_FOCUSABLE_CHANGE_CB,
    SESSION_TOUCHABLE_CHANGE_CB,
    SESSION_TOP_MOST_CHANGE_CB,
    SUB_MODAL_TYPE_CHANGE_CB,
    MAIN_MODAL_TYPE_CHANGE_CB,
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
    FORCE_HIDE_CHANGE_CB,
    WINDOW_DRAG_HOT_AREA_CB,
    TOUCH_OUTSIDE_CB,
    SESSIONINFO_LOCKEDSTATE_CHANGE_CB,
    PREPARE_CLOSE_PIP_SESSION,
    LANDSCAPE_MULTI_WINDOW_CB,
    CONTEXT_TRANSPARENT_CB,
    KEYBOARD_GRAVITY_CHANGE_CB,
    ADJUST_KEYBOARD_LAYOUT_CB,
    LAYOUT_FULL_SCREEN_CB,
    DEFAULT_DENSITY_ENABLED_CB,
    NEXT_FRAME_LAYOUT_FINISH_CB,
    MAIN_WINDOW_TOP_MOST_CHANGE_CB,
    TITLE_DOCK_HOVER_SHOW_CB,
    SET_WINDOW_RECT_AUTO_SAVE_CB,
    UPDATE_APP_USE_CONTROL_CB,
    RESTORE_MAIN_WINDOW_CB,
    UPDATE_SESSION_LABEL_AND_ICON_CB,
    KEYBOARD_STATE_CHANGE_CB,
    KEYBOARD_VIEW_MODE_CHANGE_CB,
    HIGHLIGHT_CHANGE_CB,
    SET_SUPPORT_WINDOW_MODES_CB,
    SESSION_LOCK_STATE_CHANGE_CB,
};

class SceneSession;
class JsSceneSession : public RefBase {
public:
    JsSceneSession(napi_env env, const sptr<SceneSession>& session);
    ~JsSceneSession();

    static napi_value Create(napi_env env, const sptr<SceneSession>& session);
    static void Finalizer(napi_env env, void* data, void* hint);

    void ClearCbMap(bool needRemove, int32_t persistentId);
    sptr<SceneSession> GetNativeSession() const;

private:
    static napi_value ActivateDragBySystem(napi_env env, napi_callback_info info);
    static napi_value RegisterCallback(napi_env env, napi_callback_info info);
    static napi_value UpdateNativeVisibility(napi_env env, napi_callback_info info);
    static napi_value SetShowRecent(napi_env env, napi_callback_info info);
    static napi_value SetZOrder(napi_env env, napi_callback_info info);
    static napi_value SetTouchable(napi_env env, napi_callback_info info);
    static napi_value SetSystemActive(napi_env env, napi_callback_info info);
    static napi_value SetPrivacyMode(napi_env env, napi_callback_info info);
    static napi_value SetFloatingScale(napi_env env, napi_callback_info info);
    static napi_value SetIsMidScene(napi_env env, napi_callback_info info);
    static napi_value SetSystemSceneOcclusionAlpha(napi_env env, napi_callback_info info);
    static napi_value SetSystemSceneForceUIFirst(napi_env env, napi_callback_info info);
    static napi_value SetUIFirstSwitch(napi_env env, napi_callback_info info);
    static napi_value MarkSystemSceneUIFirst(napi_env env, napi_callback_info info);
    static napi_value SetFocusable(napi_env env, napi_callback_info info);
    static napi_value SetFocusableOnShow(napi_env env, napi_callback_info info);
    static napi_value SetSystemSceneBlockingFocus(napi_env env, napi_callback_info info);
    static napi_value UpdateSizeChangeReason(napi_env env, napi_callback_info info);
    static napi_value OpenKeyboardSyncTransaction(napi_env env, napi_callback_info info);
    static napi_value CloseKeyboardSyncTransaction(napi_env env, napi_callback_info info);
    static napi_value NotifyTargetScreenWidthAndHeight(napi_env env, napi_callback_info info);
    static napi_value SetScale(napi_env env, napi_callback_info info);
    static napi_value SetWindowLastSafeRect(napi_env env, napi_callback_info info);
    static napi_value RequestHideKeyboard(napi_env env, napi_callback_info info);
    static napi_value SetSCBKeepKeyboard(napi_env env, napi_callback_info info);
    static napi_value SetOffset(napi_env env, napi_callback_info info);
    static napi_value SetExitSplitOnBackground(napi_env env, napi_callback_info info);
    static napi_value SetWaterMarkFlag(napi_env env, napi_callback_info info);
    static napi_value SetPipActionEvent(napi_env env, napi_callback_info info);
    static napi_value NotifyPipOcclusionChange(napi_env env, napi_callback_info info);
    static napi_value NotifyPipSizeChange(napi_env env, napi_callback_info info);
    static napi_value SetPiPControlEvent(napi_env env, napi_callback_info info);
    static napi_value NotifyDisplayStatusBarTemporarily(napi_env env, napi_callback_info info);
    static napi_value SetTemporarilyShowWhenLocked(napi_env env, napi_callback_info info);
    static napi_value SetSkipDraw(napi_env env, napi_callback_info info);
    static napi_value SyncScenePanelGlobalPosition(napi_env env, napi_callback_info info);
    static napi_value UnSyncScenePanelGlobalPosition(napi_env env, napi_callback_info info);
    static napi_value SetNeedSyncSessionRect(napi_env env, napi_callback_info info);
    static void BindNativeMethod(napi_env env, napi_value objValue, const char* moduleName);
    static void BindNativeMethodForKeyboard(napi_env env, napi_value objValue, const char* moduleName);
    static void BindNativeMethodForCompatiblePcMode(napi_env env, napi_value objValue, const char* moduleName);
    static void BindNativeMethodForSCBSystemSession(napi_env env, napi_value objValue, const char* moduleName);
    static void BindNativeMethodForFocus(napi_env env, napi_value objValue, const char* moduleName);
    static napi_value SetSkipSelfWhenShowOnVirtualScreen(napi_env env, napi_callback_info info);
    static napi_value SetCompatibleModeInPc(napi_env env, napi_callback_info info);
    static napi_value SetCompatibleModeEnableInPad(napi_env env, napi_callback_info info);
    static napi_value SetAppSupportPhoneInPc(napi_env env, napi_callback_info info);
    static napi_value SetCompatibleWindowSizeInPc(napi_env env, napi_callback_info info);
    static napi_value SetUniqueDensityDpiFromSCB(napi_env env, napi_callback_info info);
    static napi_value SetBlankFlag(napi_env env, napi_callback_info info);
    static napi_value RemoveBlank(napi_env env, napi_callback_info info);
    static napi_value SetBufferAvailableCallbackEnable(napi_env env, napi_callback_info info);
    static napi_value IsDeviceWakeupByApplication(napi_env env, napi_callback_info info);
    static napi_value SyncDefaultRequestedOrientation(napi_env env, napi_callback_info info);
    static napi_value SetIsPcAppInPad(napi_env env, napi_callback_info info);
    static napi_value SetStartingWindowExitAnimationFlag(napi_env env, napi_callback_info info);
    static napi_value SetIsActivatedAfterScreenLocked(napi_env env, napi_callback_info info);
    static napi_value SetFrameGravity(napi_env env, napi_callback_info info);
    static napi_value SetWindowEnableDragBySystem(napi_env env, napi_callback_info info);
    static napi_value SetUseStartingWindowAboveLocked(napi_env env, napi_callback_info info);
    static napi_value SetFreezeImmediately(napi_env env, napi_callback_info info);
    static napi_value SetColorSpace(napi_env env, napi_callback_info info);
    static napi_value SetSnapshotSkip(napi_env env, napi_callback_info info);
    static napi_value SetExclusivelyHighlighted(napi_env env, napi_callback_info info);

    napi_value OnActivateDragBySystem(napi_env env, napi_callback_info info);
    napi_value OnRegisterCallback(napi_env env, napi_callback_info info);
    napi_value OnUpdateNativeVisibility(napi_env env, napi_callback_info info);
    napi_value OnSetShowRecent(napi_env env, napi_callback_info info);
    napi_value OnSetZOrder(napi_env env, napi_callback_info info);
    napi_value OnSetTouchable(napi_env env, napi_callback_info info);
    napi_value OnSetSystemActive(napi_env env, napi_callback_info info);
    napi_value OnSetPrivacyMode(napi_env env, napi_callback_info info);
    napi_value OnSetFloatingScale(napi_env env, napi_callback_info info);
    napi_value OnSetIsMidScene(napi_env env, napi_callback_info info);
    napi_value OnSetSystemSceneOcclusionAlpha(napi_env env, napi_callback_info info);
    napi_value OnSetSystemSceneForceUIFirst(napi_env env, napi_callback_info info);
    napi_value OnSetUIFirstSwitch(napi_env env, napi_callback_info info);
    napi_value OnMarkSystemSceneUIFirst(napi_env env, napi_callback_info info);
    napi_value OnSetFocusable(napi_env env, napi_callback_info info);
    napi_value OnSetFocusableOnShow(napi_env env, napi_callback_info info);
    napi_value OnSetSystemSceneBlockingFocus(napi_env env, napi_callback_info info);
    napi_value OnUpdateSizeChangeReason(napi_env env, napi_callback_info info);
    napi_value OnOpenKeyboardSyncTransaction(napi_env env, napi_callback_info info);
    napi_value OnCloseKeyboardSyncTransaction(napi_env env, napi_callback_info info);
    napi_value OnNotifyTargetScreenWidthAndHeight(napi_env env, napi_callback_info info);
    napi_value OnSetScale(napi_env env, napi_callback_info info);
    napi_value OnSetWindowLastSafeRect(napi_env env, napi_callback_info info);
    napi_value OnRequestHideKeyboard(napi_env env, napi_callback_info info);
    napi_value OnSetSCBKeepKeyboard(napi_env env, napi_callback_info info);
    napi_value OnSetOffset(napi_env env, napi_callback_info info);
    napi_value OnSetExitSplitOnBackground(napi_env env, napi_callback_info info);
    napi_value OnSetWaterMarkFlag(napi_env env, napi_callback_info info);
    napi_value OnSetPipActionEvent(napi_env env, napi_callback_info info);
    napi_value OnSetPiPControlEvent(napi_env env, napi_callback_info info);
    napi_value OnNotifyPipOcclusionChange(napi_env env, napi_callback_info info);
    napi_value OnNotifyPipSizeChange(napi_env env, napi_callback_info info);
    napi_value OnNotifyDisplayStatusBarTemporarily(napi_env env, napi_callback_info info);
    napi_value OnSetTemporarilyShowWhenLocked(napi_env env, napi_callback_info info);
    napi_value OnSetSkipDraw(napi_env env, napi_callback_info info);
    napi_value OnSetSkipSelfWhenShowOnVirtualScreen(napi_env env, napi_callback_info info);
    napi_value OnSetCompatibleModeInPc(napi_env env, napi_callback_info info);
    napi_value OnSetAppSupportPhoneInPc(napi_env env, napi_callback_info info);
    napi_value OnSetCompatibleWindowSizeInPc(napi_env env, napi_callback_info info);
    napi_value OnSetCompatibleModeEnableInPad(napi_env env, napi_callback_info info);
    napi_value OnSetUniqueDensityDpiFromSCB(napi_env env, napi_callback_info info);
    napi_value OnSetBlankFlag(napi_env env, napi_callback_info info);
    napi_value OnRemoveBlank(napi_env env, napi_callback_info info);
    napi_value OnSetBufferAvailableCallbackEnable(napi_env env, napi_callback_info info);
    napi_value OnIsDeviceWakeupByApplication(napi_env env, napi_callback_info info);
    napi_value OnSyncDefaultRequestedOrientation(napi_env env, napi_callback_info info);
    napi_value OnSetIsPcAppInPad(napi_env env, napi_callback_info info);
    napi_value OnSetStartingWindowExitAnimationFlag(napi_env env, napi_callback_info info);
    napi_value OnSyncScenePanelGlobalPosition(napi_env env, napi_callback_info info);
    napi_value OnUnSyncScenePanelGlobalPosition(napi_env env, napi_callback_info info);
    napi_value OnSetNeedSyncSessionRect(napi_env env, napi_callback_info info);
    napi_value OnSetIsActivatedAfterScreenLocked(napi_env env, napi_callback_info info);
    napi_value OnSetFrameGravity(napi_env env, napi_callback_info info);
    napi_value OnSetWindowEnableDragBySystem(napi_env env, napi_callback_info info);
    napi_value OnSetUseStartingWindowAboveLocked(napi_env env, napi_callback_info info);
    napi_value OnSetFreezeImmediately(napi_env env, napi_callback_info info);
    napi_value OnSetColorSpace(napi_env env, napi_callback_info info);
    napi_value OnSetSnapshotSkip(napi_env env, napi_callback_info info);
    napi_value OnSetExclusivelyHighlighted(napi_env env, napi_callback_info info);

    bool IsCallbackRegistered(napi_env env, const std::string& type, napi_value jsListenerObject);
    void ProcessChangeSessionVisibilityWithStatusBarRegister();
    void InitListenerFuncs();
    void ProcessPendingSceneSessionActivationRegister();
    void ProcessSessionStateChangeRegister();
    void ProcessBufferAvailableChangeRegister();
    void ProcessSessionEventRegister();
    void ProcessCreateSubSessionRegister();
    void ProcessBindDialogTargetRegister();
    void ProcessSessionRectChangeRegister();
    void ProcessSessionPiPControlStatusChangeRegister();
    void ProcessAutoStartPiPStatusChangeRegister();
    void ProcessRaiseToTopRegister();
    void ProcessRaiseToTopForPointDownRegister();
    void ProcessBackPressedRegister();
    void ProcessSessionFocusableChangeRegister();
    void ProcessSessionTouchableChangeRegister();
    void ProcessSessionTopmostChangeRegister();
    void ProcessMainWindowTopmostChangeRegister();
    void ProcessSubModalTypeChangeRegister();
    void ProcessMainModalTypeChangeRegister();
    void ProcessClickRegister();
    void ProcessTerminateSessionRegister();
    void ProcessTerminateSessionRegisterNew();
    void ProcessTerminateSessionRegisterTotal();
    void ProcessSessionExceptionRegister();
    void ProcessUpdateSessionLabelRegister();
    void ProcessUpdateSessionIconRegister();
    void ProcessSystemBarPropertyChangeRegister();
    void ProcessNeedAvoidRegister();
    void ProcessPendingSessionToForegroundRegister();
    void ProcessPendingSessionToBackgroundForDelegatorRegister();
    void ProcessSessionDefaultAnimationFlagChangeRegister();
    void ProcessIsCustomAnimationPlaying();
    void ProcessShowWhenLockedRegister();
    void ProcessRequestedOrientationChange();
    void ProcessRaiseAboveTargetRegister();
    void ProcessForceHideChangeRegister();
    void ProcessWindowDragHotAreaRegister();
    void ProcessTouchOutsideRegister();
    void ProcessSessionInfoLockedStateChangeRegister();
    void ProcessPrepareClosePiPSessionRegister();
    void ProcessLandscapeMultiWindowRegister();
    void ProcessContextTransparentRegister();
    void ProcessKeyboardGravityChangeRegister();
    void ProcessAdjustKeyboardLayoutRegister();
    void ProcessLayoutFullScreenChangeRegister();
    void ProcessDefaultDensityEnabledRegister();
    void ProcessTitleAndDockHoverShowChangeRegister();
    void ProcessRestoreMainWindowRegister();
    void ProcessFrameLayoutFinishRegister();
    void ProcessRegisterCallback(ListenerFuncType listenerFuncType);
    void ProcessSetWindowRectAutoSaveRegister();
    void RegisterUpdateAppUseControlCallback();
    void ProcessUpdateSessionLabelAndIconRegister();
    void ProcessKeyboardStateChangeRegister();
    void ProcessKeyboardViewModeChangeRegister();
    void ProcessSetHighlightChangeRegister();
    void ProcessSessionLockStateChangeRegister();
    void OnSessionLockStateChange(bool isLockedState);

    /*
     * PC Window Layout
     */
    void ProcessSetSupportedWindowModesRegister();

    void ChangeSessionVisibilityWithStatusBar(SessionInfo& info, bool visible);
    void ChangeSessionVisibilityWithStatusBarInner(std::shared_ptr<SessionInfo> sessionInfo, bool visible);
    sptr<SceneSession> GenSceneSession(SessionInfo& info);
    void PendingSessionActivation(SessionInfo& info);
    void PendingSessionActivationInner(std::shared_ptr<SessionInfo> sessionInfo);
    void OnSessionStateChange(const SessionState& state);
    void OnBufferAvailableChange(const bool isBufferAvailable);
    void OnSessionEvent(uint32_t eventId, const SessionEventParam& param);
    void OnCreateSubSession(const sptr<SceneSession>& sceneSession);
    void OnBindDialogTarget(const sptr<SceneSession>& sceneSession);
    void OnSessionRectChange(const WSRect& rect, const SizeChangeReason& reason = SizeChangeReason::UNDEFINED);
    void OnSessionPiPControlStatusChange(WsPiPControlType controlType, WsPiPControlStatus status);
    void OnAutoStartPiPStatusChange(bool isAutoStart, uint32_t priority);
    void OnRaiseToTop();
    void OnRaiseToTopForPointDown();
    void OnRaiseAboveTarget(int32_t subWindowId);
    void OnBackPressed(bool needMoveToBackground);
    void OnSessionFocusableChange(bool isFocusable);
    void OnSessionTouchableChange(bool touchable);
    void OnSessionTopmostChange(bool topmost);
    void OnMainWindowTopmostChange(bool isTopmost);
    void OnSubModalTypeChange(SubWindowModalType subWindowModalType);
    void OnMainModalTypeChange(bool isModal);
    void OnClick();
    void TerminateSession(const SessionInfo& info);
    void TerminateSessionNew(const SessionInfo& info, bool needStartCaller, bool isFromBroker);
    void TerminateSessionTotal(const SessionInfo& info, TerminateType terminateType);
    void UpdateSessionLabel(const std::string& label);
    void UpdateSessionIcon(const std::string& iconPath);
    void OnSessionException(const SessionInfo& info, bool needRemoveSession, bool startFail);
    void OnSystemBarPropertyChange(const std::unordered_map<WindowType, SystemBarProperty>& propertyMap);
    void OnNeedAvoid(bool status);
    void PendingSessionToForeground(const SessionInfo& info);
    void PendingSessionToBackgroundForDelegator(const SessionInfo& info, bool shouldBackToCaller);
    void OnDefaultAnimationFlagChange(bool isNeedDefaultAnimationFlag);
    void OnIsCustomAnimationPlaying(bool status);
    void OnShowWhenLocked(bool showWhenLocked);
    void OnReuqestedOrientationChange(uint32_t orientation);
    void OnForceHideChange(bool hide);
    void OnWindowDragHotArea(uint32_t type, const SizeChangeReason& reason);
    void OnTouchOutside();
    void OnSessionInfoLockedStateChange(bool lockedState);
    void OnPrepareClosePiPSession();
    void OnContextTransparent();
    void SetLandscapeMultiWindow(bool isLandscapeMultiWindow);
    void OnKeyboardGravityChange(SessionGravity gravity);
    void OnAdjustKeyboardLayout(const KeyboardLayoutParams& params);
    void OnLayoutFullScreenChange(bool isLayoutFullScreen);
    void OnDefaultDensityEnabled(bool isDefaultDensityEnabled);
    void OnTitleAndDockHoverShowChange(bool isTitleHoverShown = true, bool isDockHoverShown = true);
    void RestoreMainWindow();
    void NotifyFrameLayoutFinish();
    void OnSetWindowRectAutoSave(bool enabled);
    void OnUpdateAppUseControl(ControlAppType type, bool isNeedControl);

    /*
     * PC Window Layout
     */
    void OnSetSupportedWindowModes(std::vector<AppExecFwk::SupportWindowMode>&& supportedWindowModes);

    void UpdateSessionLabelAndIcon(const std::string& label, const std::shared_ptr<Media::PixelMap>& icon);
    void OnKeyboardStateChange(SessionState state, KeyboardViewMode mode);
    void OnKeyboardViewModeChange(KeyboardViewMode mode);
    void NotifyHighlightChange(bool isHighlight);

    std::shared_ptr<NativeReference> GetJSCallback(const std::string& functionName);

    napi_env env_;
    wptr<SceneSession> weakSession_ = nullptr;
    int32_t persistentId_ = -1;
    wptr<SceneSession::SessionChangeCallback> sessionchangeCallback_ = nullptr;
    std::shared_mutex jsCbMapMutex_;
    std::map<std::string, std::shared_ptr<NativeReference>> jsCbMap_;
    std::shared_ptr<MainThreadScheduler> taskScheduler_;
    static std::map<int32_t, napi_ref> jsSceneSessionMap_;
};
} // namespace OHOS::Rosen

#endif // OHOS_WINDOW_SCENE_JS_SCENE_SESSION_H
