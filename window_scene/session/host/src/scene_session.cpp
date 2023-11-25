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

#include "session/host/include/scene_session.h"
#include <parameters.h>

#include <algorithm>
#include <hitrace_meter.h>
#include <ipc_skeleton.h>
#include <pointer_event.h>
#include "transaction/rs_sync_transaction_controller.h"
#include <transaction/rs_transaction.h>
#include <ui/rs_surface_node.h>

#include "../../proxy/include/window_info.h"

#include "common/include/session_permission.h"
#include "interfaces/include/ws_common.h"
#include "pixel_map.h"
#include "pip_util.h"
#include "session/host/include/scene_persistent_storage.h"
#include "session/host/include/session_utils.h"
#include "display_manager.h"
#include "session_helper.h"
#include "window_helper.h"
#include "window_manager_hilog.h"
#include "wm_math.h"
#include <running_lock.h>

namespace OHOS::Rosen {
namespace {
constexpr HiviewDFX::HiLogLabel LABEL = { LOG_CORE, HILOG_DOMAIN_WINDOW, "SceneSession" };
const std::string DLP_INDEX = "ohos.dlp.params.index";
} // namespace

MaximizeMode SceneSession::maximizeMode_ = MaximizeMode::MODE_RECOVER;
wptr<SceneSession> SceneSession::enterSession_ = nullptr;
std::mutex SceneSession::enterSessionMutex_;
std::map<int32_t, WSRect> SceneSession::windowDragHotAreaMap_;

SceneSession::SceneSession(const SessionInfo& info, const sptr<SpecificSessionCallback>& specificCallback)
    : Session(info)
{
    GeneratePersistentId(false, info.persistentId_);
    specificCallback_ = specificCallback;
    SetCollaboratorType(info.collaboratorType_);
    WLOGFI("[WMSCom] Create session, id: %{public}d", GetPersistentId());
}

SceneSession::~SceneSession()
{
    WLOGI("[WMSCom] ~SceneSession, id: %{public}d", GetPersistentId());
}

WSError SceneSession::Connect(const sptr<ISessionStage>& sessionStage, const sptr<IWindowEventChannel>& eventChannel,
    const std::shared_ptr<RSSurfaceNode>& surfaceNode, SystemSessionConfig& systemConfig,
    sptr<WindowSessionProperty> property, sptr<IRemoteObject> token, int32_t pid, int32_t uid)
{
    // Get pid and uid before posting task.
    pid = pid == -1 ? IPCSkeleton::GetCallingPid() : pid;
    uid = uid == -1 ? IPCSkeleton::GetCallingUid() : uid;
    return PostSyncTask(
        [weakThis = wptr(this), sessionStage, eventChannel, surfaceNode, &systemConfig, property, token, pid, uid]() {
        auto session = weakThis.promote();
        if (!session) {
            WLOGFE("session is null");
            return WSError::WS_ERROR_DESTROYED_OBJECT;
        }
        WLOGFI("[WMSCom] Connect session, id: %{public}d", session->GetPersistentId());
        auto ret = session->Session::Connect(
            sessionStage, eventChannel, surfaceNode, systemConfig, property, token, pid, uid);
        if (ret != WSError::WS_OK) {
            return ret;
        }
        session->NotifyPropertyWhenConnect();
        return ret;
    });
}

WSError SceneSession::Foreground(sptr<WindowSessionProperty> property)
{
    // return when screen is locked and show without ShowWhenLocked flag
    if (GetWindowType() == WindowType::WINDOW_TYPE_APP_MAIN_WINDOW &&
        GetStateFromManager(ManagerState::MANAGER_STATE_SCREEN_LOCKED) &&
        !IsShowWhenLocked()) {
        WLOGFW("[WMSCom] Foreground failed: Screen is locked, session %{public}d show without ShowWhenLocked flag",
            GetPersistentId());
        return WSError::WS_ERROR_INVALID_SHOW_WHEN_LOCKED;
    }

    PostTask([weakThis = wptr(this), property]() {
        auto session = weakThis.promote();
        if (!session) {
            WLOGFE("[WMSCom] session is null");
            return WSError::WS_ERROR_DESTROYED_OBJECT;
        }
        WLOGFI("[WMSCom] Foreground session, id: %{public}d", session->GetPersistentId());
        weakThis->SetTextFieldAvoidInfo(property->GetTextFieldPositionY(), property->GetTextFieldHeight());
        auto ret = session->Session::Foreground(property);
        if (ret != WSError::WS_OK) {
            return ret;
        }
        session->NotifyForeground();
        if (session->specificCallback_ != nullptr) {
            session->specificCallback_->onUpdateAvoidArea_(session->GetPersistentId());
            session->specificCallback_->onWindowInfoUpdate_(
                session->GetPersistentId(), WindowUpdateType::WINDOW_UPDATE_ADDED);
        }
        return WSError::WS_OK;
    });
    return WSError::WS_OK;
}

WSError SceneSession::Background()
{
    PostTask([weakThis = wptr(this)]() {
        auto session = weakThis.promote();
        if (!session) {
            WLOGFE("session is null");
            return WSError::WS_ERROR_DESTROYED_OBJECT;
        }
        WLOGFI("[WMSCom] Background session, id: %{public}d", session->GetPersistentId());
        auto ret = session->Session::Background();
        if (ret != WSError::WS_OK) {
            return ret;
        }
        if (WindowHelper::IsMainWindow(session->GetWindowType())) {
            session->snapshot_ = session->Snapshot();
            if (session->scenePersistence_ && session->snapshot_) {
                session->scenePersistence_->SaveSnapshot(session->snapshot_);
            }
        }
        session->NotifyBackground();
        session->snapshot_.reset();
        if (session->specificCallback_ != nullptr) {
            session->specificCallback_->onUpdateAvoidArea_(session->GetPersistentId());
            session->specificCallback_->onWindowInfoUpdate_(
                session->GetPersistentId(), WindowUpdateType::WINDOW_UPDATE_REMOVED);
        }
        return WSError::WS_OK;
    });
    return WSError::WS_OK;
}

void SceneSession::ClearSpecificSessionCbMap()
{
    PostTask([weakThis = wptr(this)]() {
        auto session = weakThis.promote();
        if (!session) {
            WLOGFE("[WMSSystem] session is null");
            return;
        }
        if (session->sessionChangeCallback_ && session->sessionChangeCallback_->clearCallbackFunc_) {
            session->sessionChangeCallback_->clearCallbackFunc_(true, session->GetPersistentId());
            WLOGFD("[WMSSystem] ClearCallbackMap, id: %{public}d", session->GetPersistentId());
        } else {
            WLOGFE("[WMSSystem] get callback failed, id: %{public}d", session->GetPersistentId());
        }
    });
}

WSError SceneSession::Disconnect()
{
    PostTask([weakThis = wptr(this)]() {
        auto session = weakThis.promote();
        if (!session) {
            WLOGFE("session is null");
            return WSError::WS_ERROR_DESTROYED_OBJECT;
        }
        WLOGFI("[WMSCom] Disconnect session, id: %{public}d", session->GetPersistentId());
        auto state = session->GetSessionState();
        if (state == SessionState::STATE_ACTIVE && WindowHelper::IsMainWindow(session->GetWindowType())) {
            session->snapshot_ = session->Snapshot();
            if (session->scenePersistence_ && session->snapshot_) {
                session->scenePersistence_->SaveSnapshot(session->snapshot_);
            }
            session->isActive_ = false;
        }
        session->Session::Disconnect();
        session->snapshot_.reset();
        session->isTerminating = false;
        return WSError::WS_OK;
    });
    return WSError::WS_OK;
}

WSError SceneSession::UpdateActiveStatus(bool isActive)
{
    PostTask([weakThis = wptr(this), isActive]() {
        auto session = weakThis.promote();
        if (!session) {
            WLOGFE("session is null");
            return WSError::WS_ERROR_DESTROYED_OBJECT;
        }
        if (!session->IsSessionValid()) {
            return WSError::WS_ERROR_INVALID_SESSION;
        }
        if (isActive == session->isActive_) {
            WLOGFD("[WMSCom] Session active do not change: %{public}d", isActive);
            return WSError::WS_DO_NOTHING;
        }

        WSError ret = WSError::WS_DO_NOTHING;
        if (isActive && session->GetSessionState() == SessionState::STATE_FOREGROUND) {
            session->UpdateSessionState(SessionState::STATE_ACTIVE);
            session->isActive_ = isActive;
            ret = WSError::WS_OK;
        }
        if (!isActive && session->GetSessionState() == SessionState::STATE_ACTIVE) {
            session->UpdateSessionState(SessionState::STATE_INACTIVE);
            session->isActive_ = isActive;
            ret = WSError::WS_OK;
        }
        WLOGFI("[WMSCom] UpdateActiveStatus, isActive: %{public}d, state: %{public}u",
            session->isActive_, session->GetSessionState());
        return ret;
    });
    return WSError::WS_OK;
}

WSError SceneSession::OnSessionEvent(SessionEvent event)
{
    PostTask([weakThis = wptr(this), event]() {
        auto session = weakThis.promote();
        if (!session) {
            WLOGFE("session is null");
            return WSError::WS_ERROR_DESTROYED_OBJECT;
        }
        WLOGFI("[WMSCom] SceneSession OnSessionEvent event: %{public}d", static_cast<int32_t>(event));
        if (event == SessionEvent::EVENT_START_MOVE && session->moveDragController_ &&
            !session->moveDragController_->GetStartDragFlag()) {
            HITRACE_METER_FMT(HITRACE_TAG_WINDOW_MANAGER, "SceneSession::StartMove");
            session->moveDragController_->InitMoveDragProperty();
            session->moveDragController_->SetStartMoveFlag(true);
            session->moveDragController_->ClacFirstMoveTargetRect(session->winRect_);
        }
        if (session->sessionChangeCallback_ && session->sessionChangeCallback_->OnSessionEvent_) {
            session->sessionChangeCallback_->OnSessionEvent_(static_cast<uint32_t>(event));
        }
        return WSError::WS_OK;
    });
    return WSError::WS_OK;
}

void SceneSession::RegisterSessionChangeCallback(const sptr<SceneSession::SessionChangeCallback>&
    sessionChangeCallback)
{
    std::lock_guard<std::mutex> guard(sessionChangeCbMutex_);
    sessionChangeCallback_ = sessionChangeCallback;
}

WSError SceneSession::SetGlobalMaximizeMode(MaximizeMode mode)
{
    return PostSyncTask([weakThis = wptr(this), mode]() {
        auto session = weakThis.promote();
        if (!session) {
            WLOGFE("session is null");
            return WSError::WS_ERROR_DESTROYED_OBJECT;
        }
        WLOGFD("[WMSCom] mode: %{public}u", static_cast<uint32_t>(mode));
        session->maximizeMode_ = mode;
        ScenePersistentStorage::Insert("maximize_state", static_cast<int32_t>(session->maximizeMode_),
            ScenePersistentStorageType::MAXIMIZE_STATE);
        return WSError::WS_OK;
    });
}

WSError SceneSession::GetGlobalMaximizeMode(MaximizeMode &mode)
{
    return PostSyncTask([weakThis = wptr(this), &mode]() {
        auto session = weakThis.promote();
        if (!session) {
            WLOGFE("session is null");
            return WSError::WS_ERROR_DESTROYED_OBJECT;
        }
        mode = maximizeMode_;
        WLOGFD("[WMSCom] mode: %{public}u", static_cast<uint32_t>(mode));
        return WSError::WS_OK;
    });
}

WSError SceneSession::SetAspectRatio(float ratio)
{
    return PostSyncTask([weakThis = wptr(this), ratio]() {
        auto session = weakThis.promote();
        if (!session) {
            WLOGFE("session is null");
            return WSError::WS_ERROR_DESTROYED_OBJECT;
        }
        if (!session->GetSessionProperty()) {
            WLOGE("SetAspectRatio failed because property is null");
            return WSError::WS_ERROR_NULLPTR;
        }
        WLOGFI("[WMSCom] ratio: %{public}f", ratio);
        float vpr = 1.5f; // 1.5f: default virtual pixel ratio
        auto display = DisplayManager::GetInstance().GetDefaultDisplay();
        if (display) {
            vpr = display->GetVirtualPixelRatio();
            WLOGD("vpr = %{public}f", vpr);
        }
        if (!MathHelper::NearZero(ratio)) {
            auto limits = session->GetSessionProperty()->GetWindowLimits();
            if (session->IsDecorEnable()) {
                if (limits.minWidth_ && limits.maxHeight_ &&
                    MathHelper::LessNotEqual(ratio, SessionUtils::ToLayoutWidth(limits.minWidth_, vpr) /
                    SessionUtils::ToLayoutHeight(limits.maxHeight_, vpr))) {
                    WLOGE("Failed, because aspectRatio is smaller than minWidth/maxHeight");
                    return WSError::WS_ERROR_INVALID_PARAM;
                } else if (limits.minHeight_ && limits.maxWidth_ &&
                    MathHelper::GreatNotEqual(ratio, SessionUtils::ToLayoutWidth(limits.maxWidth_, vpr) /
                    SessionUtils::ToLayoutHeight(limits.minHeight_, vpr))) {
                    WLOGE("Failed, because aspectRatio is bigger than maxWidth/minHeight");
                    return WSError::WS_ERROR_INVALID_PARAM;
                }
            } else {
                if (limits.minWidth_ && limits.maxHeight_ && MathHelper::LessNotEqual(ratio,
                    static_cast<float>(limits.minWidth_) / limits.maxHeight_)) {
                    WLOGE("Failed, because aspectRatio is smaller than minWidth/maxHeight");
                    return WSError::WS_ERROR_INVALID_PARAM;
                } else if (limits.minHeight_ && limits.maxWidth_ && MathHelper::GreatNotEqual(ratio,
                    static_cast<float>(limits.maxWidth_) / limits.minHeight_)) {
                    WLOGE("Failed, because aspectRatio is bigger than maxWidth/minHeight");
                    return WSError::WS_ERROR_INVALID_PARAM;
                }
            }
        }
        session->aspectRatio_ = ratio;
        if (session->moveDragController_) {
            session->moveDragController_->SetAspectRatio(ratio);
        }
        session->SaveAspectRatio(session->aspectRatio_);
        if (session->FixRectByAspectRatio(session->winRect_)) {
            session->NotifySessionRectChange(session->winRect_);
            session->UpdateRect(session->winRect_, SizeChangeReason::RESIZE);
        }
        return WSError::WS_OK;
    });
}

WSError SceneSession::UpdateRect(const WSRect& rect, SizeChangeReason reason,
    const std::shared_ptr<RSTransaction>& rsTransaction)
{
    std::lock_guard<std::recursive_mutex> lock(sizeChangeMutex_);
    if (winRect_ == rect) {
        WLOGFW("[WMSWinLayout] skip same rect update id:%{public}d!", GetPersistentId());
        return WSError::WS_OK;
    }
    winRect_ = rect;
    isDirty_ = true;
    HITRACE_METER_FMT(HITRACE_TAG_WINDOW_MANAGER,
        "SceneSession::UpdateRect%d [%d, %d, %u, %u]",
        GetPersistentId(), rect.posX_, rect.posY_, rect.width_, rect.height_);
    WLOGFD("[WMSWinLayout] Id: %{public}d, reason: %{public}d, rect: [%{public}d, %{public}d, %{public}u, %{public}u]",
        GetPersistentId(), reason_, rect.posX_, rect.posY_, rect.width_, rect.height_);
    return WSError::WS_OK;
}

WSError SceneSession::NotifyClientToUpdateRect()
{
    PostTask([weakThis = wptr(this)]() {
        auto session = weakThis.promote();
        if (!session) {
            WLOGFE("session is null");
            return WSError::WS_ERROR_DESTROYED_OBJECT;
        }
        std::lock_guard<std::recursive_mutex> lock(session->sizeChangeMutex_);
        WLOGFD("[WMSWinLayout] id:%{public}d, reason:%{public}d, rect:[%{public}d, %{public}d, %{public}u, %{public}u]",
            session->GetPersistentId(), session->reason_, session->winRect_.posX_,
            session->winRect_.posY_, session->winRect_.width_, session->winRect_.height_);
        bool isMoveOrDrag = session->moveDragController_ &&
            (session->moveDragController_->GetStartDragFlag() || session->moveDragController_->GetStartMoveFlag());
        if (isMoveOrDrag && session->reason_ == SizeChangeReason::UNDEFINED) {
            WLOGFD("[WMSWinLayout] skip redundant rect update!");
            return WSError::WS_ERROR_REPEAT_OPERATION;
        }
        auto transactionController = Rosen::RSSyncTransactionController::GetInstance();
        std::shared_ptr<RSTransaction> rsTransaction = nullptr;
        if (transactionController) {
            rsTransaction = transactionController->GetRSTransaction();
        }
        WSError ret = session->Session::UpdateRect(session->winRect_, session->reason_, rsTransaction);
        if (WindowHelper::IsPipWindow(session->GetWindowType()) && session->reason_ == SizeChangeReason::DRAG_END) {
            session->ClearPiPRectPivotInfo();
            ScenePersistentStorage::Insert("pip_window_pos_x", session->winRect_.posX_,
                ScenePersistentStorageType::PIP_INFO);
            ScenePersistentStorage::Insert("pip_window_pos_y", session->winRect_.posY_,
                ScenePersistentStorageType::PIP_INFO);
        }
        if ((ret == WSError::WS_OK || session->sessionInfo_.isSystem_) && session->specificCallback_ != nullptr) {
            session->specificCallback_->onUpdateAvoidArea_(session->GetPersistentId());
        }
        HITRACE_METER_FMT(HITRACE_TAG_WINDOW_MANAGER,
            "SceneSession::NotifyClientToUpdateRect%d [%d, %d, %u, %u] reason:%u",
            session->GetPersistentId(), session->winRect_.posX_,
            session->winRect_.posY_, session->winRect_.width_, session->winRect_.height_, session->reason_);
        // clear after use
        if (ret == WSError::WS_OK || session->sessionInfo_.isSystem_) {
            session->reason_ = SizeChangeReason::UNDEFINED;
            session->isDirty_ = false;
        }
        return ret;
    });
    return WSError::WS_OK;
}

bool SceneSession::UpdateInputMethodSessionRect(const WSRect&rect, WSRect& newWinRect, WSRect& newRequestRect)
{
    SessionGravity gravity;
    uint32_t percent = 0;
    GetSessionProperty()->GetSessionGravity(gravity, percent);
    if (GetWindowType() == WindowType::WINDOW_TYPE_INPUT_METHOD_FLOAT &&
        (gravity == SessionGravity::SESSION_GRAVITY_BOTTOM || gravity == SessionGravity::SESSION_GRAVITY_DEFAULT)) {
        auto defaultDisplayInfo = DisplayManager::GetInstance().GetDefaultDisplay();
        if (defaultDisplayInfo == nullptr) {
            WLOGFE("defaultDisplayInfo is nullptr");
            return false;
        }

        newWinRect.width_ = (gravity == SessionGravity::SESSION_GRAVITY_BOTTOM) ?
            static_cast<uint32_t>(defaultDisplayInfo->GetWidth()) : rect.width_;
        newRequestRect.width_ = newWinRect.width_;
        newWinRect.height_ = (gravity == SessionGravity::SESSION_GRAVITY_BOTTOM && percent != 0) ?
            static_cast<uint32_t>(defaultDisplayInfo->GetHeight()) * percent / 100u : rect.height_;
        newRequestRect.height_ = newWinRect.height_;
        newWinRect.posX_ = (gravity == SessionGravity::SESSION_GRAVITY_BOTTOM) ? 0 : newRequestRect.posX_;
        newRequestRect.posX_ = newWinRect.posX_;
        newWinRect.posY_ = defaultDisplayInfo->GetHeight() - static_cast<int32_t>(newWinRect.height_);
        newRequestRect.posY_ = newWinRect.posY_;
        WLOGFI("[WMSInput] rect: %{public}s, newRequestRect: %{public}s, "
            "newWinRect: %{public}s",
            rect.ToString().c_str(), newRequestRect.ToString().c_str(), newWinRect.ToString().c_str());
        return true;
    }
    WLOGFD("There is no need to update input rect");
    return false;
}

void SceneSession::SetSessionRectChangeCallback(const NotifySessionRectChangeFunc& func)
{
    PostTask([weakThis = wptr(this), func]() {
        auto session = weakThis.promote();
        if (!session) {
            WLOGFE("session is null");
            return WSError::WS_ERROR_DESTROYED_OBJECT;
        }
        session->sessionRectChangeFunc_ = func;
        if (session->sessionRectChangeFunc_ && session->GetWindowType() != WindowType::WINDOW_TYPE_APP_MAIN_WINDOW) {
            auto reason = SizeChangeReason::UNDEFINED;
            auto rect = session->GetSessionRequestRect();
            if (rect.width_ == 0 && rect.height_ == 0) {
                reason = SizeChangeReason::MOVE;
            }
            session->sessionRectChangeFunc_(session->GetSessionRequestRect(), reason);
        }
        return WSError::WS_OK;
    });
}

WSError SceneSession::UpdateSessionRect(const WSRect& rect, const SizeChangeReason& reason)
{
    PostTask([weakThis = wptr(this), rect, reason]() {
        auto session = weakThis.promote();
        if (!session) {
            WLOGFE("session is null");
            return WSError::WS_ERROR_DESTROYED_OBJECT;
        }
        auto newWinRect = session->winRect_;
        auto newRequestRect = session->GetSessionRequestRect();
        SizeChangeReason newReason = reason;
        if (reason == SizeChangeReason::MOVE) {
            newWinRect.posX_ = rect.posX_;
            newWinRect.posY_ = rect.posY_;
            newRequestRect.posX_ = rect.posX_;
            newRequestRect.posY_ = rect.posY_;
            session->SetSessionRect(newWinRect);
            session->SetSessionRequestRect(newRequestRect);
            session->NotifySessionRectChange(newRequestRect, reason);
        } else if (reason == SizeChangeReason::RESIZE) {
            bool needUpdateInputMethod = session->UpdateInputMethodSessionRect(rect, newWinRect, newRequestRect);
            if (needUpdateInputMethod) {
                newReason = SizeChangeReason::UNDEFINED;
                WLOGFD("Input rect has totally changed, need to modify reason, id: %{public}d",
                    session->GetPersistentId());
            } else if (rect.width_ > 0 && rect.height_ > 0) {
                newWinRect.width_ = rect.width_;
                newWinRect.height_ = rect.height_;
                newRequestRect.width_ = rect.width_;
                newRequestRect.height_ = rect.height_;
            }
            session->SetSessionRect(newWinRect);
            session->SetSessionRequestRect(newRequestRect);
            session->NotifySessionRectChange(newRequestRect, newReason);
        } else {
            session->SetSessionRect(rect);
            session->NotifySessionRectChange(rect, reason);
        }

        WLOGFI("[WMSCom] Id: %{public}d, reason: %{public}d, newReason: %{public}d, rect: %{public}s, "
            "newRequestRect: %{public}s, newWinRect: %{public}s", session->GetPersistentId(), reason,
            newReason, rect.ToString().c_str(), newRequestRect.ToString().c_str(), newWinRect.ToString().c_str());
        return WSError::WS_OK;
    });
    return WSError::WS_OK;
}

WSError SceneSession::RaiseToAppTop()
{
    if (!SessionPermission::IsSystemCalling()) {
        WLOGFE("raise to app top permission denied!");
        return WSError::WS_ERROR_NOT_SYSTEM_APP;
    }
    return PostSyncTask([weakThis = wptr(this)]() {
        auto session = weakThis.promote();
        if (!session) {
            WLOGFE("session is null");
            return WSError::WS_ERROR_DESTROYED_OBJECT;
        }
        if (session->sessionChangeCallback_ && session->sessionChangeCallback_->onRaiseToTop_) {
            WLOGFI("[WMSSub] id: %{public}d", session->GetPersistentId());
            session->sessionChangeCallback_->onRaiseToTop_();
        }
        return WSError::WS_OK;
    });
}

WSError SceneSession::RaiseAboveTarget(int32_t subWindowId)
{
    if (!SessionPermission::IsSystemCalling() && !SessionPermission::IsStartByHdcd()) {
        WLOGFE("RaiseAboveTarget permission denied!");
        return WSError::WS_ERROR_NOT_SYSTEM_APP;
    }
    return PostSyncTask([weakThis = wptr(this), subWindowId]() {
        auto session = weakThis.promote();
    if (!session) {
            WLOGFE("session is null");
            return WSError::WS_ERROR_DESTROYED_OBJECT;
        }
        if (session->sessionChangeCallback_ && session->sessionChangeCallback_->onRaiseAboveTarget_) {
            session->sessionChangeCallback_->onRaiseAboveTarget_(subWindowId);
        }
        return WSError::WS_OK;
    });
}

WSError SceneSession::BindDialogSessionTarget(const sptr<SceneSession>& sceneSession)
{
    if (sceneSession == nullptr) {
        WLOGFE("[WMSDialog] dialog session is null");
        return WSError::WS_ERROR_NULLPTR;
    }
    if (sessionChangeCallback_ != nullptr && sessionChangeCallback_->onBindDialogTarget_) {
        WLOGFI("[WMSDialog] id: %{public}d", sceneSession->GetPersistentId());
        sessionChangeCallback_->onBindDialogTarget_(sceneSession);
    }
    return WSError::WS_OK;
}

WSError SceneSession::SetSystemBarProperty(WindowType type, SystemBarProperty systemBarProperty)
{
    auto property = GetSessionProperty();
    if (property == nullptr) {
        return WSError::WS_ERROR_NULLPTR;
    }
    property->SetSystemBarProperty(type, systemBarProperty);
    WLOGFD("SceneSession SetSystemBarProperty status:%{public}d", static_cast<int32_t>(type));
    if (sessionChangeCallback_ != nullptr && sessionChangeCallback_->OnSystemBarPropertyChange_) {
        sessionChangeCallback_->OnSystemBarPropertyChange_(property->GetSystemBarProperty());
    }
    return WSError::WS_OK;
}

void SceneSession::NotifyPropertyWhenConnect()
{
    WLOGFI("Notify property when connect.");
    auto property = GetSessionProperty();
    if (property == nullptr) {
        WLOGFD("id: %{public}d property is nullptr", persistentId_);
        return;
    }
    NotifySessionFocusableChange(property->GetFocusable());
    NotifySessionTouchableChange(property->GetTouchable());
    OnShowWhenLocked(IsShowWhenLocked());
}

WSError SceneSession::OnNeedAvoid(bool status)
{
    PostTask([weakThis = wptr(this), status]() {
        auto session = weakThis.promote();
        if (!session) {
            WLOGFE("session is null");
            return WSError::WS_ERROR_DESTROYED_OBJECT;
        }
        WLOGFD("SceneSession OnNeedAvoid status:%{public}d", static_cast<int32_t>(status));
        if (session->sessionChangeCallback_ && session->sessionChangeCallback_->OnNeedAvoid_) {
            session->sessionChangeCallback_->OnNeedAvoid_(status);
        }
        return WSError::WS_OK;
    });
    return WSError::WS_OK;
}

WSError SceneSession::OnShowWhenLocked(bool showWhenLocked)
{
    WLOGFD("SceneSession ShowWhenLocked status:%{public}d", static_cast<int32_t>(showWhenLocked));
    if (sessionChangeCallback_ != nullptr && sessionChangeCallback_->OnShowWhenLocked_) {
        sessionChangeCallback_->OnShowWhenLocked_(showWhenLocked);
    }
    return WSError::WS_OK;
}

bool SceneSession::IsShowWhenLocked() const
{
    return GetSessionProperty()->GetWindowFlags() & static_cast<uint32_t>(WindowFlag::WINDOW_FLAG_SHOW_WHEN_LOCKED);
}

void SceneSession::CalculateAvoidAreaRect(WSRect& rect, WSRect& avoidRect, AvoidArea& avoidArea) const
{
    if (SessionHelper::IsEmptyRect(rect) || SessionHelper::IsEmptyRect(avoidRect)) {
        return;
    }
    Rect avoidAreaRect = SessionHelper::TransferToRect(SessionHelper::GetOverlap(rect, avoidRect, 0, 0));
    if (WindowHelper::IsEmptyRect(avoidAreaRect)) {
        return;
    }

    uint32_t avoidAreaCenterX = static_cast<uint32_t>(avoidAreaRect.posX_) + (avoidAreaRect.width_ >> 1);
    uint32_t avoidAreaCenterY = static_cast<uint32_t>(avoidAreaRect.posY_) + (avoidAreaRect.height_ >> 1);
    float res1 = float(avoidAreaCenterY) - float(rect.height_) / float(rect.width_) *
        float(avoidAreaCenterX);
    float res2 = float(avoidAreaCenterY) + float(rect.height_) / float(rect.width_) *
        float(avoidAreaCenterX) - float(rect.height_);
    if (res1 < 0) {
        if (res2 < 0) {
            avoidArea.topRect_ = avoidAreaRect;
        } else {
            avoidArea.rightRect_ = avoidAreaRect;
        }
    } else {
        if (res2 < 0) {
            avoidArea.leftRect_ = avoidAreaRect;
        } else {
            avoidArea.bottomRect_ = avoidAreaRect;
        }
    }
}

void SceneSession::GetSystemAvoidArea(WSRect& rect, AvoidArea& avoidArea)
{
    float vpr = 3.5f; // 3.5f: default pixel ratio
    int32_t floatingBarHeight = 32; // 32: floating windowBar Height
    if (GetSessionProperty()->GetWindowFlags() & static_cast<uint32_t>(WindowFlag::WINDOW_FLAG_NEED_AVOID)) {
        return;
    }
    if ((Session::GetWindowMode() == WindowMode::WINDOW_MODE_FLOATING ||
         Session::GetWindowMode() == WindowMode::WINDOW_MODE_SPLIT_PRIMARY ||
         Session::GetWindowMode() == WindowMode::WINDOW_MODE_SPLIT_SECONDARY) &&
        system::GetParameter("const.product.devicetype", "unknown") == "phone") {
        auto display = DisplayManager::GetInstance().GetDefaultDisplay();
        if (display) {
            vpr = display->GetVirtualPixelRatio();
        }
        avoidArea.topRect_.height_ = vpr * floatingBarHeight;
        return;
    }
    std::vector<sptr<SceneSession>> statusBarVector =
        specificCallback_->onGetSceneSessionVectorByType_(WindowType::WINDOW_TYPE_STATUS_BAR);
    for (auto& statusBar : statusBarVector) {
        if (!(statusBar->isVisible_)) {
            continue;
        }
        WSRect statusBarRect = statusBar->GetSessionRect();
        CalculateAvoidAreaRect(rect, statusBarRect, avoidArea);
    }

    return;
}

void SceneSession::GetKeyboardAvoidArea(WSRect& rect, AvoidArea& avoidArea)
{
    if ((Session::GetWindowMode() == WindowMode::WINDOW_MODE_FLOATING ||
         Session::GetWindowMode() == WindowMode::WINDOW_MODE_SPLIT_PRIMARY ||
         Session::GetWindowMode() == WindowMode::WINDOW_MODE_SPLIT_SECONDARY) &&
        system::GetParameter("const.product.devicetype", "unknown") == "phone") {
        return;
    }
    std::vector<sptr<SceneSession>> inputMethodVector =
        specificCallback_->onGetSceneSessionVectorByType_(WindowType::WINDOW_TYPE_INPUT_METHOD_FLOAT);
    for (auto& inputMethod : inputMethodVector) {
        if (inputMethod->GetSessionState() != SessionState::STATE_FOREGROUND &&
            inputMethod->GetSessionState() != SessionState::STATE_ACTIVE) {
            continue;
        }
        SessionGravity gravity;
        uint32_t percent = 0;
        inputMethod->GetSessionProperty()->GetSessionGravity(gravity, percent);
        if (gravity == SessionGravity::SESSION_GRAVITY_FLOAT) {
            continue;
        }
        WSRect inputMethodRect = inputMethod->GetSessionRect();
        CalculateAvoidAreaRect(rect, inputMethodRect, avoidArea);
    }

    return;
}

void SceneSession::GetCutoutAvoidArea(WSRect& rect, AvoidArea& avoidArea)
{
    auto display = DisplayManager::GetInstance().GetDisplayById(GetSessionProperty()->GetDisplayId());
    sptr<CutoutInfo> cutoutInfo = display->GetCutoutInfo();
    if (cutoutInfo == nullptr) {
        WLOGFI("GetCutoutAvoidArea There is no CutoutInfo");
        return;
    }
    std::vector<DMRect> cutoutAreas = cutoutInfo->GetBoundingRects();
    if (cutoutAreas.empty()) {
        WLOGFI("GetCutoutAvoidArea There is no cutoutAreas");
        return;
    }
    for (auto& cutoutArea : cutoutAreas) {
        WSRect cutoutAreaRect = {
            cutoutArea.posX_,
            cutoutArea.posY_,
            cutoutArea.width_,
            cutoutArea.height_
        };
        CalculateAvoidAreaRect(rect, cutoutAreaRect, avoidArea);
    }

    return;
}

void SceneSession::GetAINavigationBarArea(WSRect rect, AvoidArea& avoidArea) const
{
    WSRect barArea = specificCallback_->onGetAINavigationBarArea_();
    CalculateAvoidAreaRect(rect, barArea, avoidArea);
}

AvoidArea SceneSession::GetAvoidAreaByType(AvoidAreaType type)
{
    return PostSyncTask([weakThis = wptr(this), type]() -> AvoidArea {
        auto session = weakThis.promote();
        if (!session) {
            WLOGFE("session is null");
            return {};
        }
        AvoidArea avoidArea;
        WSRect rect = session->GetSessionRect();
        WLOGFD("GetAvoidAreaByType avoidAreaType:%{public}u", type);
        switch (type) {
            case AvoidAreaType::TYPE_SYSTEM: {
                session->GetSystemAvoidArea(rect, avoidArea);
                return avoidArea;
            }
            case AvoidAreaType::TYPE_KEYBOARD: {
                session->GetKeyboardAvoidArea(rect, avoidArea);
                return avoidArea;
            }
            case AvoidAreaType::TYPE_CUTOUT: {
                session->GetCutoutAvoidArea(rect, avoidArea);
                return avoidArea;
            }
            case AvoidAreaType::TYPE_NAVIGATION_INDICATOR: {
                session->GetAINavigationBarArea(rect, avoidArea);
                return avoidArea;
            }
            default: {
                WLOGFD("cannot find avoidAreaType: %{public}u", type);
                return avoidArea;
            }
        }
    });
}

WSError SceneSession::UpdateAvoidArea(const sptr<AvoidArea>& avoidArea, AvoidAreaType type)
{
    if (!sessionStage_) {
        return WSError::WS_ERROR_NULLPTR;
    }
    return sessionStage_->UpdateAvoidArea(avoidArea, type);
}

void SceneSession::HandleStyleEvent(MMI::WindowArea area)
{
    static std::pair<int32_t, MMI::WindowArea> preWindowArea =
        std::make_pair(INVALID_WINDOW_ID, MMI::WindowArea::EXIT);
    if (preWindowArea.first == Session::GetWindowId() && preWindowArea.second == area) {
        return;
    }
    if (area != MMI::WindowArea::EXIT) {
        if (Session::SetPointerStyle(area) != WSError::WS_OK) {
            WLOGFE("Failed to set the cursor style, WSError:%{public}d", Session::SetPointerStyle(area));
        }
    }
    preWindowArea = { Session::GetWindowId(), area };
}

WSError SceneSession::HandleEnterWinwdowArea(int32_t displayX, int32_t displayY)
{
    if (displayX < 0 || displayY < 0) {
        WLOGE("Illegal parameter, displayX:%{public}d, displayY:%{public}d", displayX, displayY);
        return WSError::WS_ERROR_INVALID_PARAM;
    }

    auto iter = Session::windowAreas_.cend();
    if (!Session::IsSystemSession() &&
        Session::GetWindowMode() == WindowMode::WINDOW_MODE_FLOATING &&
        Session::GetWindowType() == WindowType::WINDOW_TYPE_APP_MAIN_WINDOW) {
        iter = Session::windowAreas_.cbegin();
        for (;iter != Session::windowAreas_.cend(); ++iter) {
            WSRectF rect = iter->second;
            if (rect.IsInRegion(displayX, displayY)) {
                break;
            }
        }
    }

    MMI::WindowArea area = MMI::WindowArea::EXIT;
    if (iter == Session::windowAreas_.cend()) {
        bool isInRegion = false;
        WSRect rect = Session::winRect_;
        if (Session::GetWindowMode() == WindowMode::WINDOW_MODE_FLOATING &&
            Session::GetWindowType() == WindowType::WINDOW_TYPE_APP_MAIN_WINDOW) {
            WSRectF rectF = Session::UpdateHotRect(rect);
            isInRegion = rectF.IsInRegion(displayX, displayY);
        } else {
            isInRegion = rect.IsInRegion(displayX, displayY);
        }
        if (!isInRegion) {
            WLOGFE("The wrong event(%{public}d, %{public}d) could not be matched to the region:" \
                "[%{public}d, %{public}d, %{public}d, %{public}d]",
                displayX, displayY, rect.posX_, rect.posY_, rect.width_, rect.height_);
            return WSError::WS_ERROR_INVALID_TYPE;
        }
        area = MMI::WindowArea::FOCUS_ON_INNER;
    } else {
        area = iter->first;
    }
    HandleStyleEvent(area);
    return WSError::WS_OK;
}

WSError SceneSession::HandlePointerStyle(const std::shared_ptr<MMI::PointerEvent>& pointerEvent)
{
    if (pointerEvent == nullptr) {
        WLOGFE("pointerEvent is nullptr");
        return WSError::WS_ERROR_NULLPTR;
    }
    if (pointerEvent->GetSourceType() != MMI::PointerEvent::SOURCE_TYPE_MOUSE) {
        return WSError::WS_DO_NOTHING;
    }
    if (!(pointerEvent->GetPointerAction() == MMI::PointerEvent::POINTER_ACTION_MOVE &&
         pointerEvent->GetButtonId() == MMI::PointerEvent::BUTTON_NONE)) {
        return WSError::WS_DO_NOTHING;
    }

    MMI::PointerEvent::PointerItem pointerItem;
    if (!pointerEvent->GetPointerItem(pointerEvent->GetPointerId(), pointerItem)) {
        WLOGFE("Get pointeritem failed");
        pointerEvent->MarkProcessed();
        return WSError::WS_ERROR_INVALID_PARAM;
    }
    int32_t mousePointX = pointerItem.GetDisplayX();
    int32_t mousePointY = pointerItem.GetDisplayY();

    auto displayInfo = DisplayManager::GetInstance().GetDisplayById(pointerEvent->GetTargetDisplayId());
    if (displayInfo != nullptr) {
        float vpr = displayInfo->GetVirtualPixelRatio();
        if (vpr <= 0) {
            vpr = 1.5f;
        }
        Session::SetVpr(vpr);
    }
    return HandleEnterWinwdowArea(mousePointX, mousePointY);
}

WSError SceneSession::TransferPointerEvent(const std::shared_ptr<MMI::PointerEvent>& pointerEvent)
{
    WLOGFD("[WMSCom] TransferPointEvent, id: %{public}d, type: %{public}d", GetPersistentId(), GetWindowType());
    if (pointerEvent == nullptr) {
        WLOGFE("pointerEvent is null");
        return WSError::WS_ERROR_NULLPTR;
    }

    int32_t action = pointerEvent->GetPointerAction();
    {
        bool isSystemWindow = GetSessionInfo().isSystem_;
        if (action == MMI::PointerEvent::POINTER_ACTION_ENTER_WINDOW) {
            std::lock_guard<std::mutex> guard(enterSessionMutex_);
            WLOGFD("Set enter session, persistentId:%{public}d", GetPersistentId());
            enterSession_ = wptr<SceneSession>(this);
        }
        if ((enterSession_ != nullptr) &&
            (isSystemWindow && (action != MMI::PointerEvent::POINTER_ACTION_ENTER_WINDOW))) {
            std::lock_guard<std::mutex> guard(enterSessionMutex_);
            WLOGFD("Remove enter session, persistentId:%{public}d", GetPersistentId());
            enterSession_ = nullptr;
        }
    }

    if (!CheckPointerEventDispatch(pointerEvent)) {
        WLOGFI("Do not dispatch this pointer event");
        return WSError::WS_DO_NOTHING;
    }

    if (specificCallback_ != nullptr && specificCallback_->onSessionTouchOutside_ != nullptr &&
        (action == MMI::PointerEvent::POINTER_ACTION_DOWN || action == MMI::PointerEvent::POINTER_ACTION_BUTTON_DOWN)) {
        specificCallback_->onSessionTouchOutside_(GetPersistentId());
    }

    static bool isNew = true;
    if (isNew) {
        auto ret = HandlePointerStyle(pointerEvent);
        if ((ret != WSError::WS_OK) && (ret != WSError::WS_DO_NOTHING)) {
            WLOGFE("Failed to update the mouse cursor style, ret:%{public}d", ret);
        }
    }
    auto property = GetSessionProperty();
    if (property == nullptr) {
        return Session::TransferPointerEvent(pointerEvent);
    }
    if (property->GetWindowMode() == WindowMode::WINDOW_MODE_FLOATING &&
        WindowHelper::IsMainWindow(property->GetWindowType()) &&
        property->GetMaximizeMode() != MaximizeMode::MODE_AVOID_SYSTEM_BAR) {
        if (CheckDialogOnForeground()) {
            WLOGFI("[WMSDialog] There is dialog window foreground");
            return WSError::WS_OK;
        }
        if (!moveDragController_) {
            WLOGE("moveDragController_ is null");
            return Session::TransferPointerEvent(pointerEvent);
        }
        if (property->GetDragEnabled()) {
            if (!isNew) {
                moveDragController_->HandleMouseStyle(pointerEvent, winRect_);
            }
            auto isPhone = system::GetParameter("const.product.devicetype", "unknown") == "phone";
            if (!isPhone && moveDragController_->ConsumeDragEvent(pointerEvent, winRect_, property, systemConfig_)) {
                moveDragController_->UpdateGravityWhenDrag(pointerEvent, surfaceNode_);
                PresentFoucusIfNeed(pointerEvent->GetPointerAction());
                return WSError::WS_OK;
            }
        }
        if (IsDecorEnable() && moveDragController_->ConsumeMoveEvent(pointerEvent, winRect_)) {
            PresentFoucusIfNeed(pointerEvent->GetPointerAction());
            return WSError::WS_OK;
        }
    }

    if (property->GetWindowMode() == WindowMode::WINDOW_MODE_PIP &&
        WindowHelper::IsPipWindow(property->GetWindowType())) {
        if (!moveDragController_) {
            WLOGFE("moveDragController is null");
            return Session::TransferPointerEvent(pointerEvent);
        }
        if (moveDragController_->ConsumeMoveEvent(pointerEvent, winRect_)) {
            return WSError::WS_OK;
        }
    }

    bool raiseEnabled = (WindowHelper::IsSubWindow(property->GetWindowType()) ||
        property->GetWindowType() == WindowType::WINDOW_TYPE_DIALOG) && property->GetRaiseEnabled() &&
        (action == MMI::PointerEvent::POINTER_ACTION_DOWN || action == MMI::PointerEvent::POINTER_ACTION_BUTTON_DOWN);
    if (raiseEnabled) {
        RaiseToAppTopForPointDown();
    }
    return Session::TransferPointerEvent(pointerEvent);
}

WSError SceneSession::RequestSessionBack(bool needMoveToBackground)
{
    PostTask([weakThis = wptr(this), needMoveToBackground]() {
        auto session = weakThis.promote();
        if (!session) {
            WLOGFE("session is null");
            return WSError::WS_ERROR_DESTROYED_OBJECT;
        }
        if (!session->backPressedFunc_) {
            WLOGFW("Session didn't register back event consumer!");
            return WSError::WS_DO_NOTHING;
        }
        session->backPressedFunc_(needMoveToBackground);
        return WSError::WS_OK;
    });
    return WSError::WS_OK;
}

const wptr<SceneSession> SceneSession::GetEnterWindow()
{
    std::lock_guard<std::mutex> guard(enterSessionMutex_);
    return enterSession_;
}

void SceneSession::ClearEnterWindow()
{
    std::lock_guard<std::mutex> guard(enterSessionMutex_);
    enterSession_ = nullptr;
}

void SceneSession::NotifySessionRectChange(const WSRect& rect, const SizeChangeReason& reason)
{
    std::lock_guard<std::mutex> guard(sessionChangeCbMutex_);
    if (sessionRectChangeFunc_) {
        HITRACE_METER_FMT(HITRACE_TAG_WINDOW_MANAGER, "SceneSession::NotifySessionRectChange");
        sessionRectChangeFunc_(rect, reason);
    }
}

bool SceneSession::IsDecorEnable() const
{
    auto property = GetSessionProperty();
    if (property == nullptr) {
        WLOGE("property is nullptr");
        return false;
    }
    if (property->GetWindowMode() == WindowMode::WINDOW_MODE_FLOATING
        && system::GetParameter("const.product.devicetype", "unknown") == "phone") {
        /* FloatingWindow skip for Phone */
        return false;
    }
    return WindowHelper::IsMainWindow(property->GetWindowType()) && systemConfig_.isSystemDecorEnable_ &&
        WindowHelper::IsWindowModeSupported(systemConfig_.decorModeSupportInfo_, property->GetWindowMode());
}

std::string SceneSession::GetRatioPreferenceKey()
{
    std::string key = sessionInfo_.bundleName_ + sessionInfo_.moduleName_ + sessionInfo_.abilityName_;
    if (key.length() > ScenePersistentStorage::MAX_KEY_LEN) {
        return key.substr(key.length() - ScenePersistentStorage::MAX_KEY_LEN);
    }
    return key;
}

bool SceneSession::SaveAspectRatio(float ratio)
{
    std::string key = GetRatioPreferenceKey();
    if (!key.empty()) {
        ScenePersistentStorage::Insert(key, ratio, ScenePersistentStorageType::ASPECT_RATIO);
        WLOGD("SceneSession save aspectRatio , key %{public}s, value: %{public}f", key.c_str(), aspectRatio_);
        return true;
    }
    return false;
}

void SceneSession::FixRectByLimits(WindowLimits limits, WSRect& rect, float ratio, bool isDecor, float vpr)
{
    if (isDecor) {
        rect.width_ = SessionUtils::ToLayoutWidth(rect.width_, vpr);
        rect.height_ = SessionUtils::ToLayoutHeight(rect.height_, vpr);
        limits.minWidth_ = SessionUtils::ToLayoutWidth(limits.minWidth_, vpr);
        limits.maxWidth_ = SessionUtils::ToLayoutWidth(limits.maxWidth_, vpr);
        limits.minHeight_ = SessionUtils::ToLayoutHeight(limits.minHeight_, vpr);
        limits.maxHeight_ = SessionUtils::ToLayoutHeight(limits.maxHeight_, vpr);
    }
    if (static_cast<uint32_t>(rect.height_) > limits.maxHeight_) {
        rect.height_ = static_cast<int32_t>(limits.maxHeight_);
        rect.width_ = floor(rect.height_ * ratio);
    } else if (static_cast<uint32_t>(rect.width_) > limits.maxWidth_) {
        rect.width_ = static_cast<int32_t>(limits.maxWidth_);
        rect.height_ = floor(rect.width_ / ratio);
    } else if (static_cast<uint32_t>(rect.width_) < limits.minWidth_) {
        rect.width_ = static_cast<int32_t>(limits.minWidth_);
        rect.height_ = ceil(rect.width_ / ratio);
    } else if (static_cast<uint32_t>(rect.height_) < limits.minHeight_) {
        rect.height_ = static_cast<int32_t>(limits.minHeight_);
        rect.width_ = ceil(rect.height_ * ratio);
    }
    if (isDecor) {
        rect.height_ = SessionUtils::ToWinHeight(rect.height_, vpr) ;
        rect.width_ = SessionUtils::ToWinWidth(rect.width_, vpr);
    }
}
bool SceneSession::FixRectByAspectRatio(WSRect& rect)
{
    const int tolerancePx = 2; // 2: tolerance delta pixel value, unit: px
    WSRect originalRect = rect;
    auto property = GetSessionProperty();
    if (!property || property->GetWindowMode() != WindowMode::WINDOW_MODE_FLOATING ||
        !WindowHelper::IsMainWindow(GetWindowType())) {
        return false;
    }

    if (MathHelper::NearZero(aspectRatio_)) {
        return false;
    }
    float vpr = 1.5f; // 1.5f: default virtual pixel ratio
    auto display = DisplayManager::GetInstance().GetDefaultDisplay();
    if (display) {
        vpr = display->GetVirtualPixelRatio();
    }
    int32_t minW;
    int32_t maxW;
    int32_t minH;
    int32_t maxH;
    SessionUtils::CalcFloatWindowRectLimits(property->GetWindowLimits(), systemConfig_.maxFloatingWindowSize_, vpr,
        minW, maxW, minH, maxH);
    rect.width_ = std::max(minW, static_cast<int32_t>(rect.width_));
    rect.width_ = std::min(maxW, static_cast<int32_t>(rect.width_));
    rect.height_ = std::max(minH, static_cast<int32_t>(rect.height_));
    rect.height_ = std::min(maxH, static_cast<int32_t>(rect.height_));
    if (IsDecorEnable()) {
        if (SessionUtils::ToLayoutWidth(rect.width_, vpr) >
                SessionUtils::ToLayoutHeight(rect.height_, vpr) * aspectRatio_) {
            rect.width_ = SessionUtils::ToWinWidth(SessionUtils::ToLayoutHeight(rect.height_, vpr)* aspectRatio_, vpr);
        } else {
            rect.height_ = SessionUtils::ToWinHeight(SessionUtils::ToLayoutWidth(rect.width_, vpr) / aspectRatio_, vpr);
        }
    } else {
        if (rect.width_ > rect.height_ * aspectRatio_) {
            rect.width_ = rect.height_ * aspectRatio_;
        } else {
            rect.height_ = rect.width_ / aspectRatio_;
        }
    }
    FixRectByLimits(property->GetWindowLimits(), rect, aspectRatio_, IsDecorEnable(), vpr);
    if (std::abs(static_cast<int32_t>(originalRect.width_) - static_cast<int32_t>(rect.width_)) <= tolerancePx &&
        std::abs(static_cast<int32_t>(originalRect.height_) - static_cast<int32_t>(rect.height_)) <= tolerancePx) {
        rect = originalRect;
        return false;
    }
    return true;
}

void SceneSession::SetMoveDragCallback()
{
    if (moveDragController_) {
        MoveDragCallback callBack = [this](const SizeChangeReason& reason) {
            this->OnMoveDragCallback(reason);
        };
        moveDragController_->RegisterMoveDragCallback(callBack);
    }
}

void SceneSession::OnMoveDragCallback(const SizeChangeReason& reason)
{
    WSRect rect = moveDragController_->GetTargetRect();
    WLOGFD("OnMoveDragCallback rect: [%{public}d, %{public}d, %{public}u, %{public}u], reason : %{public}d",
        rect.posX_, rect.posY_, rect.width_, rect.height_, reason);
    if (reason == SizeChangeReason::DRAG || reason == SizeChangeReason::DRAG_END) {
        UpdateWinRectForSystemBar(rect);
    }
    HITRACE_METER_FMT(HITRACE_TAG_WINDOW_MANAGER,
        "SceneSession::OnMoveDragCallback [%d, %d, %u, %u]", rect.posX_, rect.posY_, rect.width_, rect.height_);
    SetSurfaceBounds(rect);
    if (WindowHelper::IsPipWindow(GetWindowType()) && reason == SizeChangeReason::MOVE) {
        NotifySessionRectChange(rect, reason);
    }
    if (reason != SizeChangeReason::MOVE) {
        UpdateRect(rect, reason);
    }
    if (reason == SizeChangeReason::DRAG_END) {
        NotifySessionRectChange(rect, reason);
        OnSessionEvent(SessionEvent::EVENT_END_MOVE);
    }
}

void SceneSession::UpdateWinRectForSystemBar(WSRect& rect)
{
    if (!specificCallback_) {
        WLOGFE("specificCallback_ is null!");
        return;
    }
    float tmpPosY = 0.0;
    std::vector<sptr<SceneSession>> statusBarVector =
        specificCallback_->onGetSceneSessionVectorByType_(WindowType::WINDOW_TYPE_STATUS_BAR);
    for (auto& statusBar : statusBarVector) {
        if (!(statusBar->isVisible_)) {
            continue;
        }
        WSRect statusBarRect = statusBar->GetSessionRect();
        if ((rect.posY_ < statusBarRect.posY_ + static_cast<int32_t>(statusBarRect.height_)) &&
            (rect.height_ != winRect_.height_ || rect.width_ != winRect_.width_)) {
            tmpPosY = rect.posY_ + rect.height_;
            rect.posY_ = statusBarRect.posY_ + statusBarRect.height_;
            rect.height_ = tmpPosY - rect.posY_;
        }
    }
    WLOGFD("after UpdateWinRectForSystemBar rect: [%{public}d, %{public}d, %{public}u, %{public}u]",
        rect.posX_, rect.posY_, rect.width_, rect.height_);
}

void SceneSession::SetSurfaceBounds(const WSRect& rect)
{
    auto rsTransaction = RSTransactionProxy::GetInstance();
    if (rsTransaction) {
        rsTransaction->Begin();
    }
    if (surfaceNode_ && leashWinSurfaceNode_) {
        leashWinSurfaceNode_->SetBounds(rect.posX_, rect.posY_, rect.width_, rect.height_);
        leashWinSurfaceNode_->SetFrame(rect.posX_, rect.posY_, rect.width_, rect.height_);
        surfaceNode_->SetBounds(0, 0, rect.width_, rect.height_);
        surfaceNode_->SetFrame(0, 0, rect.width_, rect.height_);
    } else if (WindowHelper::IsPipWindow(GetWindowType()) && surfaceNode_) {
        WLOGFD("PipWindow setSurfaceBounds");
        surfaceNode_->SetBounds(rect.posX_, rect.posY_, rect.width_, rect.height_);
        surfaceNode_->SetFrame(rect.posX_, rect.posY_, rect.width_, rect.height_);
    } else {
        WLOGE("SetSurfaceBounds surfaceNode is null!");
    }
    if (rsTransaction) {
        rsTransaction->Commit();
    }
}

void SceneSession::SetZOrder(uint32_t zOrder)
{
    if (zOrder_ != zOrder) {
        Session::SetZOrder(zOrder);
        if (specificCallback_ != nullptr) {
            specificCallback_->onWindowInfoUpdate_(GetPersistentId(), WindowUpdateType::WINDOW_UPDATE_PROPERTY);
        }
    }
}

void SceneSession::SetFloatingScale(float floatingScale)
{
    if (floatingScale_ != floatingScale) {
        Session::SetFloatingScale(floatingScale);
        if (specificCallback_ != nullptr) {
            specificCallback_->onWindowInfoUpdate_(GetPersistentId(), WindowUpdateType::WINDOW_UPDATE_PROPERTY);
        }
    }
}

void SceneSession::SetParentPersistentId(int32_t parentId)
{
    auto property = GetSessionProperty();
    if (property) {
        property->SetParentPersistentId(parentId);
    }
}

int32_t SceneSession::GetParentPersistentId() const
{
    auto property = GetSessionProperty();
    if (property) {
        return property->GetParentPersistentId();
    }
    return INVALID_SESSION_ID;
}

const std::string& SceneSession::GetWindowName() const
{
    return GetSessionProperty()->GetWindowName();
}

WSError SceneSession::SetTurnScreenOn(bool turnScreenOn)
{
    GetSessionProperty()->SetTurnScreenOn(turnScreenOn);
    return WSError::WS_OK;
}

bool SceneSession::IsTurnScreenOn() const
{
    return GetSessionProperty()->IsTurnScreenOn();
}

WSError SceneSession::SetKeepScreenOn(bool keepScreenOn)
{
    GetSessionProperty()->SetKeepScreenOn(keepScreenOn);
    return WSError::WS_OK;
}

bool SceneSession::IsKeepScreenOn() const
{
    return GetSessionProperty()->IsKeepScreenOn();
}

std::string SceneSession::GetSessionSnapshotFilePath() const
{
    WLOGFI("GetSessionSnapshotFilePath id %{public}d", GetPersistentId());
    if (Session::GetSessionState() < SessionState::STATE_BACKGROUND) {
        WLOGFI("GetSessionSnapshotFilePath UpdateSnapshot");
        auto snapshot = Snapshot();
        if (scenePersistence_ != nullptr) {
            scenePersistence_->SaveSnapshot(snapshot);
        }
    }
    if (scenePersistence_ != nullptr) {
        return scenePersistence_->GetSnapshotFilePath();
    }
    return "";
}

void SceneSession::SaveUpdatedIcon(const std::shared_ptr<Media::PixelMap> &icon)
{
    WLOGFI("run SaveUpdatedIcon");
    if (scenePersistence_ != nullptr) {
        scenePersistence_->SaveUpdatedIcon(icon);
    }
}

std::string SceneSession::GetUpdatedIconPath() const
{
    WLOGFI("run GetUpdatedIconPath");
    if (scenePersistence_ != nullptr) {
        return scenePersistence_->GetUpdatedIconPath();
    }
    return "";
}

void SceneSession::UpdateNativeVisibility(bool visible)
{
    isVisible_ = visible;

    if (specificCallback_ == nullptr) {
        WLOGFW("specific callback is null.");
        return;
    }

    if (visible) {
        specificCallback_->onWindowInfoUpdate_(GetPersistentId(), WindowUpdateType::WINDOW_UPDATE_ADDED);
    } else {
        specificCallback_->onWindowInfoUpdate_(GetPersistentId(), WindowUpdateType::WINDOW_UPDATE_REMOVED);
    }
    specificCallback_->onUpdateAvoidArea_(GetPersistentId());
    // update private state
    if (!GetSessionProperty()) {
        WLOGFE("UpdateNativeVisibility property is null");
        return;
    }
}

bool SceneSession::IsVisible() const
{
    return isVisible_;
}

void SceneSession::UpdateRotationAvoidArea()
{
    if (specificCallback_) {
        specificCallback_->onUpdateAvoidArea_(GetPersistentId());
    }
}

void SceneSession::SetPrivacyMode(bool isPrivacy)
{
    auto property = GetSessionProperty();
    if (!property) {
        WLOGFE("SetPrivacyMode property is null");
        return;
    }
    if (!surfaceNode_) {
        WLOGFE("surfaceNode_ is null");
        return;
    }
    bool lastPrivacyMode = property->GetPrivacyMode() || property->GetSystemPrivacyMode();
    if (lastPrivacyMode == isPrivacy) {
        WLOGFW("privacy mode is not change, do nothing, isPrivacy:%{public}d", isPrivacy);
        return;
    }
    property->SetPrivacyMode(isPrivacy);
    property->SetSystemPrivacyMode(isPrivacy);
    surfaceNode_->SetSecurityLayer(isPrivacy);
    if (leashWinSurfaceNode_ != nullptr) {
        leashWinSurfaceNode_->SetSecurityLayer(isPrivacy);
    }
    RSTransaction::FlushImplicitTransaction();
}

void SceneSession::SetSystemSceneOcclusionAlpha(double alpha)
{
    if (alpha < 0 || alpha > 1.0) {
        WLOGFE("OnSetSystemSceneOcclusionAlpha property is null");
        return;
    }
    if (!surfaceNode_) {
        WLOGFE("surfaceNode_ is null");
        return;
    }
    uint8_t alpha8bit = static_cast<uint8_t>(alpha * 255);
    WLOGFI("surfaceNode SetAbilityBGAlpha=%{public}u.", alpha8bit);
    surfaceNode_->SetAbilityBGAlpha(alpha8bit);
    if (leashWinSurfaceNode_ != nullptr) {
        leashWinSurfaceNode_->SetAbilityBGAlpha(alpha8bit);
    }
    RSTransaction::FlushImplicitTransaction();
}

WSError SceneSession::UpdateWindowAnimationFlag(bool needDefaultAnimationFlag)
{
    return PostSyncTask([weakThis = wptr(this), needDefaultAnimationFlag]() {
        auto session = weakThis.promote();
        if (!session) {
            WLOGFE("session is null");
            return WSError::WS_ERROR_DESTROYED_OBJECT;
        }
        session->needDefaultAnimationFlag_ = needDefaultAnimationFlag;
        if (session->sessionChangeCallback_ && session->sessionChangeCallback_->onWindowAnimationFlagChange_) {
            session->sessionChangeCallback_->onWindowAnimationFlagChange_(needDefaultAnimationFlag);
        }
        return WSError::WS_OK;
    });
}

void SceneSession::SetWindowAnimationFlag(bool needDefaultAnimationFlag)
{
    needDefaultAnimationFlag_ = needDefaultAnimationFlag;
    if (sessionChangeCallback_ && sessionChangeCallback_->onWindowAnimationFlagChange_) {
        sessionChangeCallback_->onWindowAnimationFlagChange_(needDefaultAnimationFlag);
    }
    return;
}

bool SceneSession::IsNeedDefaultAnimation() const
{
    return needDefaultAnimationFlag_;
}

bool SceneSession::IsAppSession() const
{
    if (GetWindowType() == WindowType::WINDOW_TYPE_APP_MAIN_WINDOW) {
        return true;
    }
    if (GetParentSession() && GetParentSession()->GetWindowType() == WindowType::WINDOW_TYPE_APP_MAIN_WINDOW) {
        return true;
    }
    return false;
}

void SceneSession::NotifyIsCustomAnimationPlaying(bool isPlaying)
{
    WLOGFI("id %{public}d %{public}u", GetPersistentId(), isPlaying);
    if (sessionChangeCallback_ != nullptr && sessionChangeCallback_->onIsCustomAnimationPlaying_) {
        sessionChangeCallback_->onIsCustomAnimationPlaying_(isPlaying);
    }
}

WSError SceneSession::UpdateWindowSceneAfterCustomAnimation(bool isAdd)
{
    PostTask([weakThis = wptr(this), isAdd]() {
        auto session = weakThis.promote();
        if (!session) {
            WLOGFE("session is null");
            return WSError::WS_ERROR_DESTROYED_OBJECT;
        }
        WLOGFI("UpdateWindowSceneAfterCustomAnimation, id %{public}d, isAdd: %{public}d",
            session->GetPersistentId(), isAdd);
        if (isAdd) {
            WLOGFE("SetOpacityFunc not register %{public}d", session->GetPersistentId());
            return WSError::WS_ERROR_INVALID_OPERATION;
        } else {
            WLOGFI("background after custom animation id %{public}d", session->GetPersistentId());
            // since background will remove surfaceNode
            session->Background();
            session->NotifyIsCustomAnimationPlaying(false);
        }
        return WSError::WS_OK;
    });
    return WSError::WS_OK;
}

bool SceneSession::IsFloatingWindowAppType() const
{
    auto property = GetSessionProperty();
    if (property == nullptr) {
        return false;
    }
    return property->IsFloatingWindowAppType();
}

std::vector<Rect> SceneSession::GetTouchHotAreas() const
{
    std::vector<Rect> touchHotAreas;
    auto property = GetSessionProperty();
    if (property) {
        property->GetTouchHotAreas(touchHotAreas);
    }
    return touchHotAreas;
}

void SceneSession::DumpSessionElementInfo(const std::vector<std::string>& params)
{
    if (!sessionStage_) {
        return;
    }
    return sessionStage_->DumpSessionElementInfo(params);
}

void SceneSession::NotifyTouchOutside()
{
    WLOGFD("id: %{public}d NotifyTouchOutside", GetPersistentId());
    if (sessionStage_) {
        WLOGFD("Notify sessionStage TouchOutside");
        sessionStage_->NotifyTouchOutside();
    }
    if (sessionChangeCallback_ && sessionChangeCallback_->OnTouchOutside_) {
        WLOGFD("Notify sessionChangeCallback TouchOutside");
        sessionChangeCallback_->OnTouchOutside_();
    }
}

bool SceneSession::CheckOutTouchOutsideRegister()
{
    if (sessionChangeCallback_ && sessionChangeCallback_->OnTouchOutside_) {
        return true;
    }
    return false;
}

void SceneSession::SetRequestedOrientation(Orientation orientation)
{
    WLOGFI("id: %{public}d orientation: %{public}u", GetPersistentId(), static_cast<uint32_t>(orientation));
    GetSessionProperty()->SetRequestedOrientation(orientation);
    if (sessionChangeCallback_ && sessionChangeCallback_->OnRequestedOrientationChange_) {
        sessionChangeCallback_->OnRequestedOrientationChange_(static_cast<uint32_t>(orientation));
    }
}

void SceneSession::NotifyForceHideChange(bool hide)
{
    WLOGFI("id: %{public}d forceHide: %{public}u", persistentId_, hide);
    auto property = GetSessionProperty();
    if (property == nullptr) {
        WLOGFD("id: %{public}d property is nullptr", persistentId_);
        return;
    }
    property->SetForceHide(hide);
    if (sessionChangeCallback_ && sessionChangeCallback_->OnForceHideChange_) {
        sessionChangeCallback_->OnForceHideChange_(hide);
    }
}

Orientation SceneSession::GetRequestedOrientation() const
{
    return GetSessionProperty()->GetRequestedOrientation();
}

int32_t SceneSession::GetCollaboratorType() const
{
    return collaboratorType_;
}

void SceneSession::SetCollaboratorType(int32_t collaboratorType)
{
    collaboratorType_ = collaboratorType;
    sessionInfo_.collaboratorType_ = collaboratorType;
}

void SceneSession::DumpSessionInfo(std::vector<std::string> &info) const
{
    std::string dumpInfo = "      Session ID #" + std::to_string(persistentId_);
    info.push_back(dumpInfo);
    dumpInfo = "        session name [" + SessionUtils::ConvertSessionName(sessionInfo_.bundleName_,
        sessionInfo_.abilityName_, sessionInfo_.moduleName_, sessionInfo_.appIndex_) + "]";
    info.push_back(dumpInfo);
    dumpInfo = "        runningState [" + std::string(isActive_ ? "FOREGROUND" : "BACKGROUND") + "]";
    info.push_back(dumpInfo);
    dumpInfo = "        lockedState [" + std::to_string(sessionInfo_.lockedState) + "]";
    info.push_back(dumpInfo);
    auto abilityInfo = sessionInfo_.abilityInfo;
    dumpInfo = "        continuable [" + (abilityInfo ? std::to_string(abilityInfo->continuable) : " ") + "]";
    info.push_back(dumpInfo);
    dumpInfo = "        timeStamp [" + sessionInfo_.time + "]";
    info.push_back(dumpInfo);
    dumpInfo = "        label [" + (abilityInfo ? abilityInfo->label : " ") + "]";
    info.push_back(dumpInfo);
    dumpInfo = "        iconPath [" + (abilityInfo ? abilityInfo->iconPath : " ") + "]";
    info.push_back(dumpInfo);
    dumpInfo = "        want [" + (sessionInfo_.want ? sessionInfo_.want->ToUri() : " ") + "]";
    info.push_back(dumpInfo);
}

std::shared_ptr<AppExecFwk::AbilityInfo> SceneSession::GetAbilityInfo() const
{
    const SessionInfo& sessionInfo = GetSessionInfo();
    return sessionInfo.abilityInfo;
}

void SceneSession::SetAbilitySessionInfo(std::shared_ptr<AppExecFwk::AbilityInfo> abilityInfo)
{
    SetSessionInfoAbilityInfo(abilityInfo);
}

void SceneSession::SetSelfToken(sptr<IRemoteObject> selfToken)
{
    selfToken_ = selfToken;
}

sptr<IRemoteObject> SceneSession::GetSelfToken() const
{
    return selfToken_;
}

WSError SceneSession::PendingSessionActivation(const sptr<AAFwk::SessionInfo> abilitySessionInfo)
{
    PostTask([weakThis = wptr(this), abilitySessionInfo]() {
        auto session = weakThis.promote();
        if (!session) {
            WLOGFE("session is null");
            return WSError::WS_ERROR_DESTROYED_OBJECT;
        }
        if (abilitySessionInfo == nullptr) {
            WLOGFE("abilitySessionInfo is null");
            return WSError::WS_ERROR_NULLPTR;
        }
        session->sessionInfo_.startMethod = StartMethod::START_CALL;

        SessionInfo info;
        info.abilityName_ = abilitySessionInfo->want.GetElement().GetAbilityName();
        info.bundleName_ = abilitySessionInfo->want.GetElement().GetBundleName();
        info.moduleName_ = abilitySessionInfo->want.GetModuleName();
        info.appIndex_ = abilitySessionInfo->want.GetIntParam(DLP_INDEX, 0);
        info.persistentId_ = abilitySessionInfo->persistentId;
        info.callerPersistentId_ = session->GetPersistentId();
        info.callState_ = static_cast<uint32_t>(abilitySessionInfo->state);
        info.uiAbilityId_ = abilitySessionInfo->uiAbilityId;
        info.want = std::make_shared<AAFwk::Want>(abilitySessionInfo->want);
        info.requestCode = abilitySessionInfo->requestCode;
        info.callerToken_ = abilitySessionInfo->callerToken;
        info.startSetting = abilitySessionInfo->startSetting;
        info.callingTokenId_ = abilitySessionInfo->callingTokenId;
        info.reuse = abilitySessionInfo->reuse;
        info.collaboratorType_ = session->collaboratorType_;
        if (info.want != nullptr) {
            info.windowMode = info.want->GetIntParam(AAFwk::Want::PARAM_RESV_WINDOW_MODE, 0);
            info.sessionAffinity = info.want->GetStringParam(Rosen::PARAM_KEY::PARAM_MISSION_AFFINITY_KEY);
            info.screenId_ = info.want->GetIntParam(AAFwk::Want::PARAM_RESV_DISPLAY_ID, 0);
        }
        WLOGFI("PendingSessionActivation:bundleName %{public}s, moduleName:%{public}s, abilityName:%{public}s, \
            appIndex:%{public}d, affinity:%{public}s", info.bundleName_.c_str(), info.moduleName_.c_str(),
            info.abilityName_.c_str(), info.appIndex_, info.sessionAffinity.c_str());
        WLOGFI("PendingSessionActivation callState:%{public}d, want persistentId: %{public}d, "
            "callingTokenId:%{public}d, uiAbilityId: %{public}" PRIu64 ", windowMode: %{public}d",
            info.callState_, info.persistentId_, info.callingTokenId_, info.uiAbilityId_, info.windowMode);
        if (session->pendingSessionActivationFunc_) {
            session->pendingSessionActivationFunc_(info);
        }
        return WSError::WS_OK;
    });
    return WSError::WS_OK;
}

WSError SceneSession::TerminateSession(const sptr<AAFwk::SessionInfo> abilitySessionInfo)
{
    PostTask([weakThis = wptr(this), abilitySessionInfo]() {
        auto session = weakThis.promote();
        if (!session) {
            WLOGFE("session is null");
            return WSError::WS_ERROR_DESTROYED_OBJECT;
        }
        if (abilitySessionInfo == nullptr) {
            WLOGFE("abilitySessionInfo is null");
            return WSError::WS_ERROR_NULLPTR;
        }
        if (session->isTerminating) {
            WLOGFE("TerminateSession isTerminating, return!");
            return WSError::WS_ERROR_INVALID_OPERATION;
        }
        session->isTerminating = true;
        SessionInfo info;
        info.abilityName_ = abilitySessionInfo->want.GetElement().GetAbilityName();
        info.bundleName_ = abilitySessionInfo->want.GetElement().GetBundleName();
        info.callerToken_ = abilitySessionInfo->callerToken;
        info.persistentId_ = static_cast<int32_t>(abilitySessionInfo->persistentId);
        {
            std::lock_guard<std::recursive_mutex> lock(session->sessionInfoMutex_);
            session->sessionInfo_.closeAbilityWant = std::make_shared<AAFwk::Want>(abilitySessionInfo->want);
            session->sessionInfo_.resultCode = abilitySessionInfo->resultCode;
        }
        if (session->terminateSessionFunc_) {
            session->terminateSessionFunc_(info);
        }
        return WSError::WS_OK;
    });
    return WSError::WS_OK;
}

WSError SceneSession::NotifySessionException(const sptr<AAFwk::SessionInfo> abilitySessionInfo)
{
    PostTask([weakThis = wptr(this), abilitySessionInfo]() {
        auto session = weakThis.promote();
        if (!session) {
            WLOGFE("session is null");
            return WSError::WS_ERROR_DESTROYED_OBJECT;
        }
        if (abilitySessionInfo == nullptr) {
            WLOGFE("abilitySessionInfo is null");
            return WSError::WS_ERROR_NULLPTR;
        }
        if (session->isTerminating) {
            WLOGFE("NotifySessionException isTerminating, return!");
            return WSError::WS_ERROR_INVALID_OPERATION;
        }
        session->isTerminating = true;
        SessionInfo info;
        info.abilityName_ = abilitySessionInfo->want.GetElement().GetAbilityName();
        info.bundleName_ = abilitySessionInfo->want.GetElement().GetBundleName();
        info.callerToken_ = abilitySessionInfo->callerToken;
        info.errorCode = abilitySessionInfo->errorCode;
        info.errorReason = abilitySessionInfo->errorReason;
        info.persistentId_ = static_cast<int32_t>(abilitySessionInfo->persistentId);
        {
            std::lock_guard<std::recursive_mutex> lock(session->sessionInfoMutex_);
            session->sessionInfo_.closeAbilityWant = std::make_shared<AAFwk::Want>(abilitySessionInfo->want);
            session->sessionInfo_.errorCode = abilitySessionInfo->errorCode;
            session->sessionInfo_.errorReason = abilitySessionInfo->errorReason;
        }
        if (!session->sessionExceptionFuncs_.empty()) {
            for (auto funcPtr : session->sessionExceptionFuncs_) {
                auto sessionExceptionFunc = *funcPtr;
                sessionExceptionFunc(info);
            }
        }
        return WSError::WS_OK;
    });
    return WSError::WS_OK;
}

WSRect SceneSession::GetLastSafeRect() const
{
    return lastSafeRect;
}

void SceneSession::SetLastSafeRect(WSRect rect)
{
    lastSafeRect.posX_ = rect.posX_;
    lastSafeRect.posY_ = rect.posY_;
    lastSafeRect.width_ = rect.width_;
    lastSafeRect.height_ = rect.height_;
    return;
}

bool SceneSession::AddSubSession(const sptr<SceneSession>& subSession)
{
    if (subSession == nullptr) {
        WLOGFE("[WMSSub] subSession is nullptr");
        return false;
    }
    const auto& persistentId = subSession->GetPersistentId();
    auto iter = std::find_if(subSession_.begin(), subSession_.end(),
        [persistentId](sptr<SceneSession> session) {
            bool res = (session != nullptr && session->GetPersistentId() == persistentId) ? true : false;
            return res;
        });
    if (iter != subSession_.end()) {
        WLOGFE("[WMSSub] Sub ession is already exists, id: %{public}d, parentId: %{public}d",
            subSession->GetPersistentId(), GetPersistentId());
        return false;
    }
    WLOGFD("[WMSSub] Success, id: %{public}d, parentId: %{public}d", subSession->GetPersistentId(), GetPersistentId());
    subSession_.push_back(subSession);
    return true;
}

bool SceneSession::RemoveSubSession(int32_t persistentId)
{
    auto iter = std::find_if(subSession_.begin(), subSession_.end(),
        [persistentId](sptr<SceneSession> session) {
            bool res = (session != nullptr && session->GetPersistentId() == persistentId) ? true : false;
            return res;
        });
    if (iter == subSession_.end()) {
        WLOGFE("[WMSSub] Could not find subsession, id: %{public}d, parentId: %{public}d",
            persistentId, GetPersistentId());
        return false;
    }
    WLOGFD("[WMSSub] Success, id: %{public}d, parentId: %{public}d", persistentId, GetPersistentId());
    subSession_.erase(iter);
    return true;
}

void SceneSession::NotifyPiPWindowPrepareClose()
{
    WLOGFD("NotifyPiPWindowPrepareClose");
    PostTask([weakThis = wptr(this)]() {
        auto session = weakThis.promote();
        if (!session) {
            WLOGFE("session is null");
            return;
        }
        if (session->sessionChangeCallback_ && session->sessionChangeCallback_->onPrepareClosePiPSession_) {
            session->sessionChangeCallback_->onPrepareClosePiPSession_();
        }
        WLOGFD("NotifyPiPWindowPrepareClose, id: %{public}d", session->GetPersistentId());
        return;
    });
}

WSError SceneSession::RecoveryPullPiPMainWindow(int32_t persistentId, const Rect& rect)
{
    WLOGFD("NotifyRecoveryPullPiPMainWindow");
    PostTask([weakThis = wptr(this), persistentId, rect]() {
        auto session = weakThis.promote();
        if (!session) {
            WLOGFE("session is null");
            return WSError::WS_ERROR_DESTROYED_OBJECT;
        }
        if (session->specificCallback_ != nullptr) {
            session->specificCallback_->onRecoveryPullPiPMainWindow_(persistentId, rect);
        }
        return WSError::WS_OK;
    });
    return WSError::WS_OK;
}

std::vector<sptr<SceneSession>> SceneSession::GetSubSession() const
{
    return subSession_;
}

WSRect SceneSession::GetSessionTargetRect() const
{
    WSRect rect;
    if (moveDragController_) {
        rect = moveDragController_->GetTargetRect();
    } else {
        WLOGFI("moveDragController_ is null");
    }
    return rect;
}

void SceneSession::SetWindowDragHotAreaListener(const NotifyWindowDragHotAreaFunc& func)
{
    if (moveDragController_) {
        moveDragController_->SetWindowDragHotAreaFunc(func);
    }
}

void SceneSession::NotifySessionForeground(uint32_t reason, bool withAnimation)
{
    if (!sessionStage_) {
        return;
    }
    return sessionStage_->NotifySessionForeground(reason, withAnimation);
}

void SceneSession::NotifySessionBackground(uint32_t reason, bool withAnimation, bool isFromInnerkits)
{
    if (!sessionStage_) {
        return;
    }
    return sessionStage_->NotifySessionBackground(reason, withAnimation, isFromInnerkits);
}

WSError SceneSession::SetTextFieldAvoidInfo(double textFieldPositionY, double textFieldHeight)
{
    textFieldPositionY_ = textFieldPositionY;
    textFieldHeight_ = textFieldHeight;
    return WSError::WS_OK;
}

bool SceneSession::InitPiPRectInfo()
{
    auto requestRect = GetSessionRequestRect();
    if (requestRect.width_ == 0 || requestRect.height_ == 0) {
        return false;
    }
    pipRectInfo_.originWidth_ = requestRect.width_;
    pipRectInfo_.originHeight_ = requestRect.height_;
    int32_t level = 0;
    ScenePersistentStorage::Get("pip_window_level", level, ScenePersistentStorageType::PIP_INFO);
    pipRectInfo_.level_ = static_cast<PiPScaleLevel>(level);
    return true;
}

void SceneSession::ClearPiPRectPivotInfo()
{
    pipRectInfo_.xPivot_ = PiPScalePivot::UNDEFINED;
    pipRectInfo_.yPivot_ = PiPScalePivot::UNDEFINED;
}

void SceneSession::SavePiPRectInfo()
{
    auto pipRect = GetSessionRequestRect();
    ScenePersistentStorage::Insert("pip_window_pos_x", pipRect.posX_, ScenePersistentStorageType::PIP_INFO);
    ScenePersistentStorage::Insert("pip_window_pos_y", pipRect.posY_, ScenePersistentStorageType::PIP_INFO);
    ScenePersistentStorage::Insert("pip_window_level", static_cast<int32_t>(pipRectInfo_.level_),
        ScenePersistentStorageType::PIP_INFO);
}

void SceneSession::GetNewPiPRect(const uint32_t displayWidth, const uint32_t displayHeight, Rect& rect)
{
    PiPUtil::GetRectByScale(displayWidth, displayHeight, pipRectInfo_.level_, rect);
    WLOGD("SceneSession::GetNewPiPRect rect = (%{public}d, %{public}d, %{public}d, %{public}d)",
        rect.posX_, rect.posY_, rect.width_, rect.height_);
    auto requestRect = GetSessionRequestRect();
    if (pipRectInfo_.xPivot_ == PiPScalePivot::UNDEFINED || pipRectInfo_.yPivot_ == PiPScalePivot::UNDEFINED) {
        // If no anchor, create anchor
        WLOGD("SceneSession::GetNewPiPRect can't find anchor, create it");
        PiPUtil::UpdateRectPivot(rect.posX_, rect.width_, displayWidth, pipRectInfo_.xPivot_);
        PiPUtil::UpdateRectPivot(rect.posY_, rect.height_, displayHeight, pipRectInfo_.yPivot_);
    } else {
        // If it has anchor, location by anchor
        WLOGD("SceneSession::GetNewPiPRect find anchor, resize");
        PiPUtil::GetRectByPivot(rect.posX_, requestRect.width_, rect.width_, displayWidth, pipRectInfo_.xPivot_);
        PiPUtil::GetRectByPivot(rect.posY_, requestRect.height_, rect.height_, displayHeight, pipRectInfo_.yPivot_);
    }
    PiPUtil::GetValidRect(displayWidth, displayHeight, rect);
    WLOGD("SceneSession::GetNewPiPRect valid rect = (%{public}d, %{public}d, %{public}d, %{public}d)",
        rect.posX_, rect.posY_, rect.width_, rect.height_);
}

void SceneSession::ProcessUpdatePiPRect(SizeChangeReason reason)
{
    if (GetWindowType() != WindowType::WINDOW_TYPE_PIP) {
        WLOGE("SceneSessionManager::ProcessUpdatePiPRect not pip window");
        return;
    }
    auto display = DisplayManager::GetInstance().GetDefaultDisplay();
    if (!display) {
        WLOGFE("can't find display info");
        return;
    }
    uint32_t displayWidth = static_cast<uint32_t>(display->GetWidth());
    uint32_t displayHeight = static_cast<uint32_t>(display->GetHeight());

    // default pos of phone is the right top
    Rect rect = { 0, 0, pipRectInfo_.originWidth_, pipRectInfo_.originHeight_ };
    ScenePersistentStorage::Get("pip_window_pos_x", rect.posX_, ScenePersistentStorageType::PIP_INFO);
    ScenePersistentStorage::Get("pip_window_pos_y", rect.posY_, ScenePersistentStorageType::PIP_INFO);
    if (rect.posX_ == 0) {
        rect.posX_ = displayWidth - PiPUtil::SAFE_PADDING_HORIZONTAL;
    }
    if (rect.posY_ == 0) {
        rect.posY_ = PiPUtil::SAFE_PADDING_VERTICAL_TOP;
    }
    WLOGD("SceneSession::ProcessUpdatePiPRectpip window rect: (%{public}d, %{public}d, %{public}u, %{public}u)",
        rect.posX_, rect.posY_, rect.width_, rect.height_);

    GetNewPiPRect(displayWidth, displayHeight, rect);
    WLOGD("SceneSession::ProcessUpdatePiPRectpip window new rect: (%{public}d, %{public}d, %{public}u, %{public}u)",
        rect.posX_, rect.posY_, rect.width_, rect.height_);
    ScenePersistentStorage::Insert("pip_window_pos_x", rect.posX_, ScenePersistentStorageType::PIP_INFO);
    ScenePersistentStorage::Insert("pip_window_pos_y", rect.posY_, ScenePersistentStorageType::PIP_INFO);

    WSRect newRect = SessionHelper::TransferToWSRect(rect);
    SetSessionRect(newRect);
    Session::UpdateRect(newRect, reason);
    NotifySessionRectChange(newRect, reason);
}

WSError SceneSession::UpdatePiPRect(uint32_t width, uint32_t height, PiPRectUpdateReason reason)
{
    PostTask([weakThis = wptr(this), width, height, reason]() {
        auto session = weakThis.promote();
        if (!session) {
            WLOGE("SceneSession::UpdatePiPRect session is null");
            return WSError::WS_ERROR_DESTROYED_OBJECT;
        }
        if (session->isTerminating) {
            WLOGE("SceneSession::UpdatePiPRect session is terminating");
            return WSError::WS_ERROR_INVALID_OPERATION;
        }
        switch (reason) {
            case PiPRectUpdateReason::REASON_PIP_START_WINDOW:
                session->InitPiPRectInfo();
                session->ProcessUpdatePiPRect(SizeChangeReason::CUSTOM_ANIMATION_SHOW);
                break;
            case PiPRectUpdateReason::REASON_PIP_SCALE_CHANGE:
                session->pipRectInfo_.level_ = static_cast<PiPScaleLevel>((static_cast<int32_t>(
                    session->pipRectInfo_.level_) + 1) % static_cast<int32_t>(PiPScaleLevel::COUNT));
                session->ProcessUpdatePiPRect(SizeChangeReason::TRANSFORM);
                break;
            case PiPRectUpdateReason::REASON_PIP_VIDEO_RATIO_CHANGE:
                session->ClearPiPRectPivotInfo();
                session->pipRectInfo_.originWidth_ = width;
                session->pipRectInfo_.originHeight_ = height;
                session->ProcessUpdatePiPRect(SizeChangeReason::UNDEFINED);
                break;
            case PiPRectUpdateReason::REASON_PIP_MOVE:
                session->ClearPiPRectPivotInfo();
                break;
            case PiPRectUpdateReason::REASON_PIP_DESTROY_WINDOW:
                session->ClearPiPRectPivotInfo();
                session->SavePiPRectInfo();
                break;
            default:
                return WSError::WS_DO_NOTHING;
        }
        return WSError::WS_OK;
    });
    return WSError::WS_OK;
}

WSError SceneSession::UpdateSizeChangeReason(SizeChangeReason reason)
{
    PostTask([weakThis = wptr(this), reason]() {
        auto session = weakThis.promote();
        if (!session) {
            WLOGFE("session is null");
            return WSError::WS_ERROR_DESTROYED_OBJECT;
        }
        if (session->sessionInfo_.isSystem_) {
            // system scene no need to update reason
            return WSError::WS_DO_NOTHING;
        }
        std::lock_guard<std::recursive_mutex> lock(session->sizeChangeMutex_);
        session->reason_ = reason;
        if (reason != SizeChangeReason::UNDEFINED) {
            HITRACE_METER_FMT(HITRACE_TAG_WINDOW_MANAGER,
                "SceneSession::UpdateSizeChangeReason%d reason:%d",
                session->GetPersistentId(), static_cast<uint32_t>(reason));
            WLOGFD("[WMSWinLayout]UpdateSizeChangeReason Id: %{public}d, reason: %{public}d",
                session->GetPersistentId(), reason);
        }
        return WSError::WS_OK;
    });
    return WSError::WS_OK;
}

bool SceneSession::IsDirtyWindow()
{
    std::lock_guard<std::recursive_mutex> lock(sizeChangeMutex_);
    return isDirty_;
}
} // namespace OHOS::Rosen
