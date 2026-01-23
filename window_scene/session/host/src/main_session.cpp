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

#include "session/host/include/main_session.h"

#include <ipc_skeleton.h>

#include "common/include/fold_screen_state_internel.h"
#include "common/include/session_permission.h"
#include "window_helper.h"
#include "session_helper.h"
#include "session/host/include/scene_persistent_storage.h"
#include "wm_math.h"

namespace OHOS::Rosen {
namespace {
constexpr HiviewDFX::HiLogLabel LABEL = { LOG_CORE, HILOG_DOMAIN_WINDOW, "MainSession" };
constexpr int32_t MAX_LABEL_SIZE = 1024;
const uint64_t PRELAUNCH_DONE_TIME = system::GetIntParameter<int>("window.prelaunchDoneTime", 6000);
} // namespace

MainSession::MainSession(const SessionInfo& info, const sptr<SpecificSessionCallback>& specificCallback, int32_t userId)
    : SceneSession(info, specificCallback, userId)
{
    scenePersistence_ = sptr<ScenePersistence>::MakeSptr(info.bundleName_, GetPersistentId(), capacity_);
    pcFoldScreenController_ = sptr<PcFoldScreenController>::MakeSptr(wptr(this), GetPersistentId());

    WLOGFD("Create MainSession");
}

MainSession::~MainSession()
{
    WLOGD("~MainSession, id: %{public}d", GetPersistentId());
}

void MainSession::OnFirstStrongRef(const void* objectId)
{
    // OnFirstStrongRef is overridden in the parent class IPCObjectStub,
    // so its parent implementation must be invoked here to avoid IPC communication issues.
    SceneSession::OnFirstStrongRef(objectId);

    moveDragController_ = sptr<MoveDragController>::MakeSptr(wptr(this));
    if (specificCallback_ != nullptr && specificCallback_->onWindowInputPidChangeCallback_ != nullptr) {
        moveDragController_->SetNotifyWindowPidChangeCallback(specificCallback_->onWindowInputPidChangeCallback_);
    }
    std::string key = GetRatioPreferenceKey();
    if (!key.empty()) {
        if (ScenePersistentStorage::HasKey(key, ScenePersistentStorageType::ASPECT_RATIO)) {
            float aspectRatio = 0.f;
            ScenePersistentStorage::Get(key, aspectRatio, ScenePersistentStorageType::ASPECT_RATIO);
            TLOGI(WmsLogTag::WMS_LAYOUT,
                  "init aspectRatio, bundleName:%{public}s, key:%{public}s, value:%{public}f",
                  sessionInfo_.bundleName_.c_str(), key.c_str(), aspectRatio);
            Session::SetAspectRatio(aspectRatio);
        }
    }
}

WSError MainSession::Reconnect(const sptr<ISessionStage>& sessionStage, const sptr<IWindowEventChannel>& eventChannel,
    const std::shared_ptr<RSSurfaceNode>& surfaceNode, sptr<WindowSessionProperty> property, sptr<IRemoteObject> token,
    int32_t pid, int32_t uid)
{
    return PostSyncTask([weakThis = wptr(this), sessionStage, eventChannel, surfaceNode, property, token, pid, uid]() {
        auto session = weakThis.promote();
        if (!session) {
            WLOGFE("session is null");
            return WSError::WS_ERROR_DESTROYED_OBJECT;
        }
        WSError ret = LOCK_GUARD_EXPR(SCENE_GUARD,
            session->Session::Reconnect(sessionStage, eventChannel, surfaceNode, property, token, pid, uid));
        if (ret != WSError::WS_OK) {
            return ret;
        }
        WindowState windowState = property->GetWindowState();
        if (windowState == WindowState::STATE_SHOWN) {
            session->isActive_ = true;
            session->UpdateSessionState(SessionState::STATE_ACTIVE);
        } else {
            session->isActive_ = false;
            session->UpdateSessionState(SessionState::STATE_BACKGROUND);
        }
        if (session->pcFoldScreenController_) {
            session->pcFoldScreenController_->OnConnect();
        }
        session->RestoreAspectRatio(property->GetAspectRatio());
        return ret;
    });
}

WSError MainSession::ProcessPointDownSession(int32_t posX, int32_t posY)
{
    const auto& id = GetPersistentId();
    TLOGD(WmsLogTag::WMS_INPUT_KEY_FLOW, "id:%{public}d, type:%{public}d", id, GetWindowType());
    auto isModal = IsModal();
    if (!isModal && CheckDialogOnForeground()) {
        HandlePointDownDialog();
        return WSError::WS_OK;
    }
    if (isModal) {
        Session::ProcessClickModalWindowOutside(posX, posY);
    }
    auto ret = SceneSession::ProcessPointDownSession(posX, posY);
    PresentFocusIfPointDown();
    return ret;
}

void MainSession::NotifyForegroundInteractiveStatus(bool interactive)
{
    SetForegroundInteractiveStatus(interactive);
    if (!IsSessionValid() || !sessionStage_) {
        TLOGW(WmsLogTag::WMS_MAIN, "Session or sessionStage is invalid, id: %{public}d state: %{public}u",
            GetPersistentId(), GetSessionState());
        return;
    }
    const auto& state = GetSessionState();
    if (isVisible_ || state == SessionState::STATE_ACTIVE || state == SessionState::STATE_FOREGROUND) {
        WLOGFI("NotifyForegroundInteractiveStatus %{public}d", interactive);
        sessionStage_->NotifyForegroundInteractiveStatus(interactive);
    }
}

WSError MainSession::TransferKeyEvent(const std::shared_ptr<MMI::KeyEvent>& keyEvent)
{
    if (!IsSessionValid()) {
        return WSError::WS_ERROR_INVALID_SESSION;
    }
    if (keyEvent == nullptr) {
        WLOGFE("KeyEvent is nullptr");
        return WSError::WS_ERROR_NULLPTR;
    }
    if (CheckDialogOnForeground()) {
        TLOGD(WmsLogTag::WMS_DIALOG, "Has dialog on foreground, not transfer pointer event");
        return WSError::WS_ERROR_INVALID_PERMISSION;
    }

    WSError ret = Session::TransferKeyEvent(keyEvent);
    return ret;
}

void MainSession::UpdatePointerArea(const WSRect& rect)
{
    if (GetWindowMode() != WindowMode::WINDOW_MODE_FLOATING) {
        return;
    }
    Session::UpdatePointerArea(rect);
}

bool MainSession::CheckPointerEventDispatch(const std::shared_ptr<MMI::PointerEvent>& pointerEvent) const
{
    auto sessionState = GetSessionState();
    int32_t action = pointerEvent->GetPointerAction();
    if (sessionState != SessionState::STATE_FOREGROUND &&
        sessionState != SessionState::STATE_ACTIVE &&
        action != MMI::PointerEvent::POINTER_ACTION_LEAVE_WINDOW) {
        WLOGFW("Current Session Info: [persistentId: %{public}d, "
            "state: %{public}d, action:%{public}d]", GetPersistentId(), GetSessionState(), action);
        return false;
    }
    return true;
}

WSError MainSession::SetTopmost(bool topmost)
{
    TLOGI(WmsLogTag::WMS_HIERARCHY, "SetTopmost id: %{public}d, topmost: %{public}d", GetPersistentId(), topmost);
    PostTask([weakThis = wptr(this), topmost]() {
        auto session = weakThis.promote();
        if (!session) {
            TLOGE(WmsLogTag::WMS_HIERARCHY, "session is null");
            return;
        }
        auto property = session->GetSessionProperty();
        if (property) {
            TLOGI(WmsLogTag::WMS_HIERARCHY, "Notify session topmost change, id: %{public}d, topmost: %{public}u",
                session->GetPersistentId(), topmost);
            property->SetTopmost(topmost);
            if (session->onSessionTopmostChange_) {
                session->onSessionTopmostChange_(topmost);
            }
        }
    }, "SetTopmost");
    return WSError::WS_OK;
}

bool MainSession::IsTopmost() const
{
    return GetSessionProperty()->IsTopmost();
}

WSError MainSession::SetMainWindowTopmost(bool isTopmost)
{
    GetSessionProperty()->SetMainWindowTopmost(isTopmost);
    TLOGD(WmsLogTag::WMS_HIERARCHY, "id: %{public}d, isTopmost: %{public}u",
        GetPersistentId(), isTopmost);
    if (mainWindowTopmostChangeFunc_) {
        mainWindowTopmostChangeFunc_(isTopmost);
    }
    return WSError::WS_OK;
}

bool MainSession::IsMainWindowTopmost() const
{
    return GetSessionProperty()->IsMainWindowTopmost();
}

void MainSession::RectCheck(uint32_t curWidth, uint32_t curHeight)
{
    uint32_t minWidth = GetSystemConfig().miniWidthOfMainWindow_;
    uint32_t minHeight = GetSystemConfig().miniHeightOfMainWindow_;
    uint32_t maxFloatingWindowSize = GetSystemConfig().maxFloatingWindowSize_;
    RectSizeCheckProcess(curWidth, curHeight, minWidth, minHeight, maxFloatingWindowSize);
}

void MainSession::SetExitSplitOnBackground(bool isExitSplitOnBackground)
{
    TLOGD(WmsLogTag::WMS_MULTI_WINDOW, "id:%{public}d, isExitSplitOnBackground:%{public}d", persistentId_,
        isExitSplitOnBackground);
    isExitSplitOnBackground_ = isExitSplitOnBackground;
}

bool MainSession::IsExitSplitOnBackground() const
{
    return isExitSplitOnBackground_;
}

void MainSession::RecoverSnapshotPersistence(const SessionInfo& info)
{
    if (scenePersistence_ == nullptr) {
        return;
    }
    if (info.persistentId_ != 0 && info.persistentId_ != GetPersistentId()) {
        // persistentId changed due to id conflicts. Need to rename the old snapshot if exists
        scenePersistence_->RenameSnapshotFromOldPersistentId(info.persistentId_);
        RenameSnapshotFromOldPersistentId(info.persistentId_);
    }
    if (info.isPersistentRecover_) {
        if (HasSnapshot()) {
            TLOGI(WmsLogTag::WMS_PATTERN, "%{public}d", GetPersistentId());
        }
        RecoverImageForRecent();
    } else {
        ClearSnapshotPersistence();
    }
}

void MainSession::ClearSnapshotPersistence()
{
    if (scenePersistence_ == nullptr) {
        return;
    }
    auto task = [weakThis = wptr(this), where = __func__]() {
        auto session = weakThis.promote();
        if (session == nullptr) {
            TLOGE(WmsLogTag::WMS_PATTERN, "%{public}s, session is nullptr", where);
            return;
        }
        TLOGI(WmsLogTag::WMS_PATTERN, "%{public}s, %{public}d", where, session->GetPersistentId());
        session->DeletePersistentImageFit();
        for (uint32_t screenStatus = SCREEN_UNKNOWN; screenStatus < SCREEN_COUNT; screenStatus++) {
            session->DeleteHasSnapshot(screenStatus);
        }
        session->DeleteHasSnapshotFreeMultiWindow();
    };
    auto snapshotFfrtHelper = scenePersistence_->GetSnapshotFfrtHelper();
    std::string taskName = "ClearSnapshotPersistence" + std::to_string(GetPersistentId());
    snapshotFfrtHelper->CancelTask(taskName);
    snapshotFfrtHelper->SubmitTask(std::move(task), taskName);
}

void MainSession::NotifyClientToUpdateInteractive(bool interactive)
{
    if (!sessionStage_) {
        return;
    }
    const auto state = GetSessionState();
    TLOGI(WmsLogTag::WMS_LIFE, "state: %{public}d", state);
    if (state == SessionState::STATE_ACTIVE || state == SessionState::STATE_FOREGROUND) {
        TLOGI(WmsLogTag::WMS_LIFE, "interactive: %{public}d", interactive);
        sessionStage_->NotifyForegroundInteractiveStatus(interactive);
        isClientInteractive_ = interactive;
    }
}

/*
 * Notify when updating highlight instead after hightlight functionality enabled
 */
WSError MainSession::UpdateFocus(bool isFocused)
{
    if (FoldScreenStateInternel::IsSuperFoldDisplayDevice() && !IsFocused() && isFocused && pcFoldScreenController_) {
        pcFoldScreenController_->UpdateSupportEnterWaterfallMode();
    }
    return Session::UpdateFocus(isFocused);
}

WSError MainSession::OnTitleAndDockHoverShowChange(bool isTitleHoverShown, bool isDockHoverShown)
{
    const char* const funcName = __func__;
    PostTask([weakThis = wptr(this), isTitleHoverShown, isDockHoverShown, funcName] {
        auto session = weakThis.promote();
        if (!session) {
            TLOGNE(WmsLogTag::WMS_LAYOUT_PC, "%{public}s session is null", funcName);
            return;
        }
        TLOGND(WmsLogTag::WMS_LAYOUT_PC, "%{public}s isTitleHoverShown: %{public}d, isDockHoverShown: %{public}d",
            funcName, isTitleHoverShown, isDockHoverShown);
        if (session->onTitleAndDockHoverShowChangeFunc_) {
            session->onTitleAndDockHoverShowChangeFunc_(isTitleHoverShown, isDockHoverShown);
        }
    }, funcName);
    return WSError::WS_OK;
}

WSError MainSession::OnRestoreMainWindow()
{
    bool isAppSupportPhoneInPc = GetSessionProperty()->GetIsAppSupportPhoneInPc();
    int32_t callingPid = IPCSkeleton::GetCallingPid();
    uint32_t callingToken = IPCSkeleton::GetCallingTokenID();
    std::string appInstanceKey = GetSessionProperty()->GetAppInstanceKey();
    bool isAppBoundSystemTray = (SceneSession::isAppBoundSystemTrayCallback_ &&
                                SceneSession::isAppBoundSystemTrayCallback_(callingPid, callingToken, appInstanceKey));
    TLOGI(WmsLogTag::WMS_MAIN,
        "isAppSupportPhoneInPc: %{public}d callingPid: %{public}d callingTokenId: %{public}u appInstanceKey: "
        "%{public}s isAppBoundSystemTray: %{public}d",
        isAppSupportPhoneInPc,
        callingPid,
        callingToken,
        appInstanceKey.c_str(),
        isAppBoundSystemTray);
    // check if the application is bound to the system tray
    if (isAppSupportPhoneInPc && !isAppBoundSystemTray) {
        TLOGE(WmsLogTag::WMS_LAYOUT_PC, "The application is not bound to the system tray.");
        return WSError::WS_ERROR_INVALID_CALLING;
    }
    PostTask([weakThis = wptr(this), isAppSupportPhoneInPc, callingPid, callingToken] {
        auto session = weakThis.promote();
        if (!session) {
            TLOGNE(WmsLogTag::WMS_LAYOUT_PC, "session is null");
            return;
        }
        if (session->onRestoreMainWindowFunc_) {
            session->onRestoreMainWindowFunc_(isAppSupportPhoneInPc, callingPid, callingToken);
        }
    }, __func__);
    return WSError::WS_OK;
}

WSError MainSession::OnSetWindowRectAutoSave(bool enabled, bool isSaveBySpecifiedFlag)
{
    const char* const where = __func__;
    PostTask([weakThis = wptr(this), enabled, isSaveBySpecifiedFlag, where] {
        auto session = weakThis.promote();
        if (!session) {
            TLOGNE(WmsLogTag::WMS_MAIN, "session is null");
            return;
        }
        if (session->onSetWindowRectAutoSaveFunc_) {
            session->onSetWindowRectAutoSaveFunc_(enabled, isSaveBySpecifiedFlag);
            TLOGNI(WmsLogTag::WMS_MAIN, "%{public}s id %{public}d isSaveBySpecifiedFlag: %{public}d "
                "enable:%{public}d", where, session->GetPersistentId(), isSaveBySpecifiedFlag, enabled);
        }
    }, __func__);
    return WSError::WS_OK;
}

WSError MainSession::NotifySupportWindowModesChange(
    const std::vector<AppExecFwk::SupportWindowMode>& supportedWindowModes)
{
    const char* const where = __func__;
    PostTask([weakThis = wptr(this), supportedWindowModes = supportedWindowModes, where]() mutable {
        auto session = weakThis.promote();
        if (!session) {
            TLOGNE(WmsLogTag::WMS_LAYOUT_PC, "%{public}s session is null", where);
            return;
        }
        if (session->onSetSupportedWindowModesFunc_) {
            session->onSetSupportedWindowModesFunc_(std::move(supportedWindowModes));
        }
    }, __func__);
    return WSError::WS_OK;
}

WSError MainSession::NotifyMainModalTypeChange(bool isModal)
{
    const char* const where = __func__;
    PostTask([weakThis = wptr(this), isModal, where] {
        auto session = weakThis.promote();
        if (!session) {
            TLOGNE(WmsLogTag::WMS_LIFE, "%{public}s session is null", where);
            return;
        }
        TLOGNI(WmsLogTag::WMS_HIERARCHY, "%{public}s main window isModal:%{public}d", where, isModal);
        if (session->onMainModalTypeChange_) {
            session->onMainModalTypeChange_(isModal);
        }
    }, __func__);
    return WSError::WS_OK;
}

bool MainSession::IsModal() const
{
    return WindowHelper::IsModalMainWindow(GetSessionProperty()->GetWindowType(),
                                           GetSessionProperty()->GetWindowFlags());
}

bool MainSession::IsApplicationModal() const
{
    return IsModal();
}

void MainSession::RegisterSessionLockStateChangeCallback(NotifySessionLockStateChangeCallback&& callback)
{
    PostTask([weakThis = wptr(this), callback = std::move(callback)] {
        auto session = weakThis.promote();
        if (!session) {
            TLOGNE(WmsLogTag::WMS_MAIN, "session is null");
            return;
        }
        session->onSessionLockStateChangeCallback_ = std::move(callback);
        if (session->onSessionLockStateChangeCallback_ && session->GetSessionLockState()) {
            session->onSessionLockStateChangeCallback_(session->GetSessionLockState());
        }
    }, __func__);
}

void MainSession::NotifySubAndDialogFollowRectChange(const WSRect& rect, bool isGlobal, bool needFlush)
{
    std::unordered_map<int32_t, NotifySurfaceBoundsChangeFunc> funcMap;
    {
        std::lock_guard lock(registerNotifySurfaceBoundsChangeMutex_);
        funcMap = notifySurfaceBoundsChangeFuncMap_;
    }
    WSRect newRect;
    bool isCompatMode = IsInCompatScaleMode();
    for (const auto& [sessionId, func] : funcMap) {
        auto subSession = GetSceneSessionById(sessionId);
        if (subSession && subSession->GetIsFollowParentLayout() && func) {
            if (!isCompatMode) {
                func(rect, isGlobal, needFlush);
                continue;
            }
            if (newRect.IsEmpty()) {
                HookStartMoveRect(newRect, rect);
            }
            if (GetCallingPid() != subSession->GetCallingPid()) {
                func(newRect, isGlobal, needFlush);
            }
        }
    }
}

void MainSession::SetSubWindowBoundsDuringCross(const WSRect& parentRect, bool isGlobal, bool needFlush)
{
    for (const auto& subSession : GetSubSession()) {
        if (subSession && subSession->GetWindowAnchorInfo().isAnchorEnabled_ && subSession->IsSessionForeground()) {
            WSRect subRect = subSession->GetSessionRect();
            subSession->CalcSubWindowRectByAnchor(parentRect, subRect);
            subSession->UpdateSizeChangeReason(SizeChangeReason::UNDEFINED);
            subSession->SetSurfaceBounds(subRect, isGlobal, needFlush);
        }
    }
}

void MainSession::NotifySubSessionRectChangeByAnchor(const WSRect& parentRect,
    SizeChangeReason reason, DisplayId displayId)
{
    for (const auto& subSession : GetSubSession()) {
        if (subSession && subSession->GetWindowAnchorInfo().isAnchorEnabled_) {
            WSRect subRect = subSession->GetSessionRect();
            subSession->CalcSubWindowRectByAnchor(parentRect, subRect);
            subSession->NotifySessionRectChange(subRect, reason, displayId);
        }
    }
}

void MainSession::HandleSubSessionSurfaceNodeByWindowAnchor(SizeChangeReason reason, DisplayId displayId)
{
    for (const auto& subSession : GetSubSession()) {
        if (subSession && subSession->GetWindowAnchorInfo().isAnchorEnabled_ && subSession->IsSessionForeground()) {
            subSession->HandleCrossSurfaceNodeByWindowAnchor(reason, displayId);
        }
    }
}

void MainSession::NotifySessionLockStateChange(bool isLockedState)
{
    PostTask([weakThis = wptr(this), isLockedState] {
        auto session = weakThis.promote();
        if (!session) {
            TLOGNE(WmsLogTag::WMS_MAIN, "session is null");
            return;
        }
        if (session->GetSessionLockState() == isLockedState) {
            TLOGNW(WmsLogTag::WMS_MAIN, "isLockedState is already %{public}d", isLockedState);
            return;
        }
        session->SetSessionLockState(isLockedState);
        if (session->onSessionLockStateChangeCallback_) {
            TLOGNI(WmsLogTag::WMS_MAIN, "onSessionLockStageChange to:%{public}d", isLockedState);
            session->onSessionLockStateChangeCallback_(isLockedState);
        }
    }, __func__);
}

void MainSession::SetSessionLockState(bool isLockedState)
{
    isLockedState_ = isLockedState;
}

bool MainSession::GetSessionLockState() const
{
    return isLockedState_;
}

WSError MainSession::SetSessionLabelAndIcon(const std::string& label,
    const std::shared_ptr<Media::PixelMap>& icon)
{
    TLOGI(WmsLogTag::WMS_MAIN, "id: %{public}d", persistentId_);
    int32_t callingPid = IPCSkeleton::GetCallingPid();
    const bool pidCheck = (callingPid != -1) && (callingPid == GetCallingPid());
    if (!pidCheck ||
        !SessionPermission::VerifyCallingPermission(PermissionConstants::PERMISSION_SET_ABILITY_INSTANCE_INFO)) {
        TLOGE(WmsLogTag::WMS_MAIN,
            "The caller has not permission granted or not the same processs, "
            "callingPid_: %{public}d, callingPid: %{public}d, bundleName: %{public}s",
            GetCallingPid(), callingPid, GetSessionInfo().bundleName_.c_str());
        return WSError::WS_ERROR_INVALID_PERMISSION;
    }
    if (!systemConfig_.IsPcWindow()) {
        TLOGE(WmsLogTag::WMS_MAIN, "device not support");
        return WSError::WS_ERROR_DEVICE_NOT_SUPPORT;
    }
    if (label.empty() || label.length() > MAX_LABEL_SIZE) {
        TLOGE(WmsLogTag::WMS_MAIN, "invalid label");
        return WSError::WS_ERROR_SET_SESSION_LABEL_FAILED;
    }
    return SetSessionLabelAndIconInner(label, icon);
}

WSError MainSession::SetSessionLabelAndIconInner(const std::string& label,
    const std::shared_ptr<Media::PixelMap>& icon)
{
    const char* const where = __func__;
    PostTask([weakThis = wptr(this), where, label, icon] {
        auto session = weakThis.promote();
        if (session == nullptr) {
            TLOGNE(WmsLogTag::WMS_MAIN, "%{public}s session is nullptr", where);
            return WSError::WS_ERROR_NULLPTR;
        }
        session->label_ = label;
        if (session->updateSessionLabelAndIconFunc_) {
            session->updateSessionLabelAndIconFunc_(label, icon);
        }
        return WSError::WS_OK;
    }, __func__);
    return WSError::WS_OK;
}

void MainSession::SetUpdateSessionLabelAndIconListener(NofitySessionLabelAndIconUpdatedFunc&& func)
{
    const char* const where = __func__;
    PostTask([weakThis = wptr(this), func = std::move(func), where] {
        auto session = weakThis.promote();
        if (!session) {
            TLOGNE(WmsLogTag::WMS_MAIN, "%{public}s session is null", where);
            return;
        }
        session->updateSessionLabelAndIconFunc_ = std::move(func);
    }, __func__);
}

WSError MainSession::UpdateFlag(const std::string& flag)
{
    const char* const where = __func__;
    PostTask([weakThis = wptr(this), flag, where] {
        auto session = weakThis.promote();
        if (!session) {
            TLOGNE(WmsLogTag::WMS_MAIN, "session is null");
            return;
        }
        session->sessionInfo_.specifiedFlag_ = flag;
        if (session->onUpdateFlagFunc_) {
            session->onUpdateFlagFunc_(flag);
            TLOGND(WmsLogTag::WMS_MAIN, "%{public}s id %{public}d flag: %{public}s",
                where, session->GetPersistentId(), flag.c_str());
        }
    }, __func__);
    return WSError::WS_OK;
}

WMError MainSession::GetRouterStackInfo(std::string& routerStackInfo) const
{
    if (!sessionStage_) {
        TLOGE(WmsLogTag::WMS_LIFE, "sessionStage is nullptr");
        return WMError::WM_ERROR_NULLPTR;
    }
    return sessionStage_->GetRouterStackInfo(routerStackInfo);
}

void MainSession::SetRecentSessionState(RecentSessionInfo& info, const SessionState& state)
{
    switch (state) {
        case SessionState::STATE_DISCONNECT: {
            info.sessionState = RecentSessionState::DISCONNECT;
            break;
        }
        case SessionState::STATE_CONNECT: {
            info.sessionState = RecentSessionState::CONNECT;
            break;
        }
        case SessionState::STATE_FOREGROUND: {
            info.sessionState = RecentSessionState::FOREGROUND;
            break;
        }
        case SessionState::STATE_BACKGROUND: {
            info.sessionState = RecentSessionState::BACKGROUND;
            break;
        }
        case SessionState::STATE_ACTIVE: {
            info.sessionState = RecentSessionState::ACTIVE;
            break;
        }
        case SessionState::STATE_INACTIVE: {
            info.sessionState = RecentSessionState::INACTIVE;
            break;
        }
        default: {
            info.sessionState = RecentSessionState::END;
            break;
        }
    }
}

void MainSession::RegisterForceSplitFullScreenChangeCallback(ForceSplitFullScreenChangeCallback&& callback)
{
    forceSplitFullScreenChangeCallback_ = std::move(callback);
}

WSError MainSession::NotifyIsFullScreenInForceSplitMode(bool isFullScreen)
{
    isFullScreenInForceSplit_.store(isFullScreen);
    if (forceSplitFullScreenChangeCallback_) {
        forceSplitFullScreenChangeCallback_(GetCallingUid(), isFullScreen);
    }
    return WSError::WS_OK;
}

bool MainSession::IsFullScreenInForceSplit()
{
    return isFullScreenInForceSplit_.load();
}

void MainSession::RegisterCompatibleModeChangeCallback(CompatibleModeChangeCallback&& callback)
{
    compatibleModeChangeCallback_ = callback;
}

WSError MainSession::NotifyCompatibleModeChange(CompatibleStyleMode mode)
{
    if (compatibleModeChangeCallback_) {
        compatibleModeChangeCallback_(mode);
    }
    return WSError::WS_OK;
}

bool MainSession::RestoreAspectRatio(float ratio)
{
    TLOGD(WmsLogTag::WMS_LAYOUT, "windowId: %{public}d, ratio: %{public}f", GetPersistentId(), ratio);

    // If ratio is nearly zero, no aspect ratio needs to be restored.
    // This avoids overwriting the persistent configuration.
    if (MathHelper::NearZero(ratio)) {
        return false;
    }
    Session::SetAspectRatio(ratio);
    return true;
}

WMError MainSession::GetAppForceLandscapeConfigEnable(bool& enableForceSplit)
{
    if (forceSplitEnableFunc_ == nullptr) {
        TLOGE(WmsLogTag::WMS_COMPAT, "forceSplitEnableFunc_ is null");
        return WMError::WM_ERROR_NULLPTR;
    }
    enableForceSplit = forceSplitEnableFunc_(sessionInfo_.bundleName_);
    return WMError::WM_OK;
}

WSError MainSession::NotifyAppForceLandscapeConfigEnableUpdated()
{
    if (!sessionStage_) {
        TLOGE(WmsLogTag::WMS_COMPAT, "sessionStage_ is null");
        return WSError::WS_ERROR_NULLPTR;
    }
    return sessionStage_->NotifyAppForceLandscapeConfigEnableUpdated();
}

void MainSession::RegisterForceSplitEnableListener(NotifyForceSplitEnableFunc&& func)
{
    forceSplitEnableFunc_ = std::move(func);
}

void MainSession::RemovePrelaunchStartingWindow()
{
    auto lifecycleListeners = GetListeners<ILifecycleListener>();
    for (auto& listener : lifecycleListeners) {
        if (auto listenerPtr = listener.lock()) {
            listenerPtr->OnRemovePrelaunchStartingWindow();
        }
    }
}

void MainSession::SetPrelaunch()
{
    prelaunchStart_ = static_cast<int64_t>(std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count());
    TLOGI(WmsLogTag::WMS_LIFE, "SetPrelaunch timestamp: %{public}" PRIu64, prelaunchStart_);
    sessionInfo_.isPrelaunch_ = true;
}

bool MainSession::IsPrelaunch() const
{
    int64_t nowTimeStamp = static_cast<int64_t>(std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count());

    if (sessionInfo_.isPrelaunch_) {
        TLOGI(WmsLogTag::WMS_LIFE, "IsPrelaunch now - start: %{public}" PRIu64, nowTimeStamp - prelaunchStart_);
    }

    int64_t timeDiff = nowTimeStamp - prelaunchStart_;
    return sessionInfo_.isPrelaunch_ && timeDiff > 0 && timeDiff > PRELAUNCH_DONE_TIME;
}
} // namespace OHOS::Rosen
