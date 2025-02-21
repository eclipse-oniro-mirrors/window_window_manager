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

#include "common/include/session_permission.h"
#include "window_helper.h"
#include "session_helper.h"
#include "session/host/include/scene_persistent_storage.h"

namespace OHOS::Rosen {
namespace {
constexpr HiviewDFX::HiLogLabel LABEL = { LOG_CORE, HILOG_DOMAIN_WINDOW, "MainSession" };
constexpr int32_t MAX_LABEL_SIZE = 1024;
} // namespace

MainSession::MainSession(const SessionInfo& info, const sptr<SpecificSessionCallback>& specificCallback)
    : SceneSession(info, specificCallback)
{
    scenePersistence_ = new ScenePersistence(info.bundleName_, GetPersistentId());
    if (info.persistentId_ != 0 && info.persistentId_ != GetPersistentId()) {
        // persistentId changed due to id conflicts. Need to rename the old snapshot if exists
        scenePersistence_->RenameSnapshotFromOldPersistentId(info.persistentId_);
    }
    moveDragController_ = new (std::nothrow) MoveDragController(GetPersistentId());
    if (moveDragController_  != nullptr && specificCallback != nullptr &&
        specificCallback->onWindowInputPidChangeCallback_ != nullptr) {
        moveDragController_->SetNotifyWindowPidChangeCallback(specificCallback->onWindowInputPidChangeCallback_);
    }
    SetMoveDragCallback();
    std::string key = GetRatioPreferenceKey();
    if (!key.empty()) {
        if (ScenePersistentStorage::HasKey(key, ScenePersistentStorageType::ASPECT_RATIO)) {
            ScenePersistentStorage::Get(key, aspectRatio_, ScenePersistentStorageType::ASPECT_RATIO);
            WLOGD("SceneSession init aspectRatio , key %{public}s, value: %{public}f", key.c_str(), aspectRatio_);
            if (moveDragController_) {
                moveDragController_->SetAspectRatio(aspectRatio_);
            }
        }
    }

    WLOGFD("Create MainSession");
}

MainSession::~MainSession()
{
    WLOGD("~MainSession, id: %{public}d", GetPersistentId());
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
        WSError ret = session->Session::Reconnect(sessionStage, eventChannel, surfaceNode, property, token, pid, uid);
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
            if (session->scenePersistence_) {
                session->scenePersistence_->SetHasSnapshot(true);
            }
        }
        return ret;
    });
}

WSError MainSession::ProcessPointDownSession(int32_t posX, int32_t posY)
{
    const auto& id = GetPersistentId();
    WLOGFI("id: %{public}d, type: %{public}d", id, GetWindowType());
    auto isModal = IsModal();
    if (!isModal && CheckDialogOnForeground()) {
        HandlePointDownDialog();
        return WSError::WS_OK;
    }
    PresentFocusIfPointDown();
    return SceneSession::ProcessPointDownSession(posX, posY);
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
    TLOGI(WmsLogTag::WMS_LAYOUT, "SetTopmost id: %{public}d, topmost: %{public}d", GetPersistentId(), topmost);
    auto task = [weakThis = wptr(this), topmost]() {
        auto session = weakThis.promote();
        if (!session) {
            TLOGE(WmsLogTag::WMS_LAYOUT, "session is null");
            return;
        }
        auto property = session->GetSessionProperty();
        if (property) {
            TLOGI(WmsLogTag::WMS_LAYOUT, "Notify session topmost change, id: %{public}d, topmost: %{public}u",
                session->GetPersistentId(), topmost);
            property->SetTopmost(topmost);
            if (session->sessionChangeCallback_ && session->sessionChangeCallback_->onSessionTopmostChange_) {
                session->sessionChangeCallback_->onSessionTopmostChange_(topmost);
            }
        }
    };
    PostTask(task, "SetTopmost");
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

void MainSession::NotifyClientToUpdateInteractive(bool interactive)
{
    if (!sessionStage_) {
        return;
    }
    const auto state = GetSessionState();
    if (state == SessionState::STATE_ACTIVE || state == SessionState::STATE_FOREGROUND) {
        WLOGFI("%{public}d", interactive);
        sessionStage_->NotifyForegroundInteractiveStatus(interactive);
        isClientInteractive_ = interactive;
    }
}

WSError MainSession::OnTitleAndDockHoverShowChange(bool isTitleHoverShown, bool isDockHoverShown)
{
    const char* const funcName = __func__;
    auto task = [weakThis = wptr(this), isTitleHoverShown, isDockHoverShown, funcName] {
        auto session = weakThis.promote();
        if (!session) {
            TLOGNE(WmsLogTag::WMS_IMMS, "%{public}s session is null", funcName);
            return;
        }
        TLOGNI(WmsLogTag::WMS_IMMS, "%{public}s isTitleHoverShown: %{public}d, isDockHoverShown: %{public}d", funcName,
            isTitleHoverShown, isDockHoverShown);
        if (session->onTitleAndDockHoverShowChangeFunc_) {
            session->onTitleAndDockHoverShowChangeFunc_(isTitleHoverShown, isDockHoverShown);
        }
    };
    PostTask(task, funcName);
    return WSError::WS_OK;
}

WSError MainSession::OnRestoreMainWindow()
{
    auto task = [weakThis = wptr(this)] {
        auto session = weakThis.promote();
        if (!session) {
            TLOGNE(WmsLogTag::WMS_LIFE, "session is null");
            return;
        }
        if (session->onRestoreMainWindowFunc_) {
            session->onRestoreMainWindowFunc_();
        }
    };
    PostTask(task, __func__);
    return WSError::WS_OK;
}

WSError MainSession::OnSetWindowRectAutoSave(bool enabled)
{
    auto task = [weakThis = wptr(this), enabled] {
        auto session = weakThis.promote();
        if (!session) {
            TLOGNE(WmsLogTag::WMS_MAIN, "session is null");
            return;
        }
        if (session->onSetWindowRectAutoSaveFunc_) {
            session->onSetWindowRectAutoSaveFunc_(enabled);
        }
    };
    PostTask(task, __func__);
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
    if (systemConfig_.uiType_ != UI_TYPE_PC) {
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
} // namespace OHOS::Rosen
