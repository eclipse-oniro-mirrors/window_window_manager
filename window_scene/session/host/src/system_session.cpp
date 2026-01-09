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

#include "session/host/include/system_session.h"

#include "common/include/session_permission.h"
#include "key_event.h"
#include "window_helper.h"
#include "window_manager_hilog.h"
#include "pointer_event.h"

namespace OHOS::Rosen {
namespace {
constexpr HiviewDFX::HiLogLabel LABEL = { LOG_CORE, HILOG_DOMAIN_WINDOW, "SystemSession" };
} // namespace

constexpr uint32_t MIN_SYSTEM_WINDOW_WIDTH = 5;
constexpr uint32_t MIN_SYSTEM_WINDOW_HEIGHT = 5;
constexpr uint8_t MAX_FB_CLICK_COUNT = 8;
const std::string FB_CLICK_EVENT = "click";

SystemSession::SystemSession(const SessionInfo& info, const sptr<SpecificSessionCallback>& specificCallback,
    int32_t userId): SceneSession(info, specificCallback, userId)
{
    TLOGD(WmsLogTag::WMS_LIFE, "Create");
    pcFoldScreenController_ = sptr<PcFoldScreenController>::MakeSptr(wptr(this), GetPersistentId());
}

SystemSession::~SystemSession()
{
    TLOGD(WmsLogTag::WMS_LIFE, "id: %{public}d", GetPersistentId());
}

void SystemSession::OnFirstStrongRef(const void* objectId)
{
    // OnFirstStrongRef is overridden in the parent class IPCObjectStub,
    // so its parent implementation must be invoked here to avoid IPC communication issues.
    SceneSession::OnFirstStrongRef(objectId);

    moveDragController_ = sptr<MoveDragController>::MakeSptr(wptr(this));
    if (specificCallback_ != nullptr && specificCallback_->onWindowInputPidChangeCallback_ != nullptr) {
        moveDragController_->SetNotifyWindowPidChangeCallback(specificCallback_->onWindowInputPidChangeCallback_);
    }
}

void SystemSession::UpdateCameraWindowStatus(bool isShowing)
{
    if (specificCallback_ == nullptr) {
        return;
    }
    if (GetWindowType() == WindowType::WINDOW_TYPE_FLOAT_CAMERA) {
        if (!specificCallback_->onCameraFloatSessionChange_) {
            return;
        }
        TLOGI(WmsLogTag::WMS_SYSTEM, "CameraFloat status: %{public}d, id: %{public}d", isShowing, GetPersistentId());
        specificCallback_->onCameraFloatSessionChange_(GetSessionProperty()->GetAccessTokenId(), isShowing);
    } else if (GetWindowType() == WindowType::WINDOW_TYPE_PIP && GetWindowMode() == WindowMode::WINDOW_MODE_PIP) {
        if (!specificCallback_->onCameraSessionChange_) {
            return;
        }
        auto pipType = GetPiPTemplateInfo().pipTemplateType;
        if (pipType == static_cast<uint32_t>(PiPTemplateType::VIDEO_CALL) ||
            pipType == static_cast<uint32_t>(PiPTemplateType::VIDEO_MEETING)) {
            TLOGI(WmsLogTag::WMS_SYSTEM, "PiPWindow status: %{public}d, id: %{public}d", isShowing, GetPersistentId());
            specificCallback_->onCameraSessionChange_(GetSessionProperty()->GetAccessTokenId(), isShowing);
        }
    } else {
        TLOGI(WmsLogTag::WMS_SYSTEM, "Skip window type, isShowing: %{public}d", isShowing);
    }
}

WSError SystemSession::Show(sptr<WindowSessionProperty> property)
{
    if (!CheckPermissionWithPropertyAnimation(property)) {
        return WSError::WS_ERROR_NOT_SYSTEM_APP;
    }
    auto type = GetWindowType();
    if (((type == WindowType::WINDOW_TYPE_TOAST) || (type == WindowType::WINDOW_TYPE_FLOAT)) &&
        !SessionPermission::IsSystemCalling()) {
        auto parentSession = GetParentSession();
        if (parentSession == nullptr) {
            WLOGFW("parent session is null");
            return WSError::WS_ERROR_INVALID_PARENT;
        }
        if ((type == WindowType::WINDOW_TYPE_TOAST) && !parentSession->IsSessionForeground()) {
            WLOGFW("parent session is not in foreground");
            return WSError::WS_ERROR_INVALID_OPERATION;
        }
    }
    PostTask([weakThis = wptr(this), property]() {
        auto session = weakThis.promote();
        if (!session) {
            WLOGFE("session is null");
            return WSError::WS_ERROR_DESTROYED_OBJECT;
        }
        TLOGI(WmsLogTag::WMS_LIFE, "Show session, id: %{public}d", session->GetPersistentId());
        // use property from client
        if (property && property->GetAnimationFlag() == static_cast<uint32_t>(WindowAnimation::CUSTOM)) {
            session->GetSessionProperty()->SetAnimationFlag(static_cast<uint32_t>(WindowAnimation::CUSTOM));
            session->NotifyIsCustomAnimationPlaying(true);
        }
        session->UpdateCameraWindowStatus(true);
        session->UpdatePiPWindowStateChanged(true);
        auto ret = session->SceneSession::Foreground(property);
        return ret;
    }, "Show");
    return WSError::WS_OK;
}

WSError SystemSession::Hide()
{
    if (!CheckPermissionWithPropertyAnimation(GetSessionProperty())) {
        return WSError::WS_ERROR_NOT_SYSTEM_APP;
    }
    auto type = GetWindowType();
    if (NeedSystemPermission(type)) {
        // Do not need to verify the permission to hide the input method status bar.
        if (!SessionPermission::IsSystemCalling() && type != WindowType::WINDOW_TYPE_INPUT_METHOD_STATUS_BAR) {
            TLOGE(WmsLogTag::WMS_LIFE, "Hide permission denied id: %{public}d type:%{public}u",
                GetPersistentId(), type);
            return WSError::WS_ERROR_INVALID_PERMISSION;
        }
    }
    PostTask([weakThis = wptr(this)]() {
        auto session = weakThis.promote();
        if (!session) {
            TLOGE(WmsLogTag::WMS_LIFE, "session is null");
            return WSError::WS_ERROR_DESTROYED_OBJECT;
        }
        TLOGI(WmsLogTag::WMS_LIFE, "Hide session, id: %{public}d", session->GetPersistentId());
        auto ret = session->SetActive(false);
        if (ret != WSError::WS_OK) {
            return ret;
        }
        // background will remove surfaceNode, custom not execute
        // not animation playing when already background; inactive may be animation playing
        auto sessionProperty = session->GetSessionProperty();
        if (sessionProperty &&
            sessionProperty->GetAnimationFlag() == static_cast<uint32_t>(WindowAnimation::CUSTOM)) {
            session->NotifyIsCustomAnimationPlaying(true);
            return WSError::WS_OK;
        }
        session->UpdateCameraWindowStatus(false);
        session->UpdatePiPWindowStateChanged(false);
        ret = session->SceneSession::Background();
        return ret;
    }, "Hide");
    return WSError::WS_OK;
}

WSError SystemSession::Disconnect(bool isFromClient, const std::string& identityToken)
{
    PostTask([weakThis = wptr(this), isFromClient]() {
        auto session = weakThis.promote();
        if (!session) {
            TLOGE(WmsLogTag::WMS_LIFE, "session is null");
            return WSError::WS_ERROR_DESTROYED_OBJECT;
        }
        TLOGI(WmsLogTag::WMS_LIFE, "Disconnect session, id: %{public}d", session->GetPersistentId());
        session->SceneSession::Disconnect(isFromClient);
        session->UpdateCameraWindowStatus(false);
        session->UpdatePiPWindowStateChanged(false);
        return WSError::WS_OK;
    }, "Disconnect");
    return WSError::WS_OK;
}

WSError SystemSession::ProcessPointDownSession(int32_t posX, int32_t posY)
{
    const auto& id = GetPersistentId();
    const auto& type = GetWindowType();
    auto parentSession = GetParentSession();
    if (parentSession && parentSession->CheckDialogOnForeground()) {
        WLOGFI("Parent has dialog foreground, id: %{public}d, type: %{public}d", id, type);
        parentSession->HandlePointDownDialog();
        if (!IsTopDialog()) {
            return WSError::WS_OK;
        }
    }
    if (type == WindowType::WINDOW_TYPE_DIALOG) {
        Session::ProcessClickModalWindowOutside(posX, posY);
        auto sessionProperty = GetSessionProperty();
        if (sessionProperty && sessionProperty->GetRaiseEnabled()) {
            RaiseToAppTopForPointDown();
        }
    }
    TLOGD(WmsLogTag::WMS_EVENT, "id:%{public}d, type:%{public}d", id, type);
    auto ret = SceneSession::ProcessPointDownSession(posX, posY);
    PresentFocusIfPointDown();
    return ret;
}

int32_t SystemSession::GetMissionId() const
{
    auto parentSession = GetParentSession();
    return parentSession != nullptr ? parentSession->GetPersistentId() : SceneSession::GetMissionId();
}

WSError SystemSession::TransferKeyEvent(const std::shared_ptr<MMI::KeyEvent>& keyEvent)
{
    if (!IsSessionValid()) {
        return WSError::WS_ERROR_INVALID_SESSION;
    }
    if (keyEvent == nullptr) {
        WLOGFE("KeyEvent is nullptr");
        return WSError::WS_ERROR_NULLPTR;
    }
    if (GetWindowType() == WindowType::WINDOW_TYPE_DIALOG) {
        if (keyEvent->GetKeyCode() == MMI::KeyEvent::KEYCODE_BACK) {
            return WSError::WS_ERROR_INVALID_PERMISSION;
        }
        auto parentSession = GetParentSession();
        if (parentSession && parentSession->CheckDialogOnForeground() &&
            !IsTopDialog()) {
            return WSError::WS_ERROR_INVALID_PERMISSION;
        }
        if (!CheckKeyEventDispatch(keyEvent)) {
            WLOGFW("Do not dispatch the key event.");
            return WSError::WS_DO_NOTHING;
        }
    }

    WSError ret = Session::TransferKeyEvent(keyEvent);
    return ret;
}

WSError SystemSession::ProcessBackEvent()
{
    if (GetWindowType() == WindowType::WINDOW_TYPE_DIALOG && !dialogSessionBackGestureEnabled_) {
        TLOGI(WmsLogTag::WMS_DIALOG, "this is dialog, id: %{public}d", GetPersistentId());
        return WSError::WS_OK;
    }
    return Session::ProcessBackEvent();
}

WSError SystemSession::NotifyClientToUpdateRect(const std::string& updateReason,
    std::shared_ptr<RSTransaction> rsTransaction)
{
    PostTask([weakThis = wptr(this), rsTransaction, updateReason]() {
        auto session = weakThis.promote();
        if (!session) {
            WLOGFE("session is null");
            return WSError::WS_ERROR_DESTROYED_OBJECT;
        }
        WSError ret = session->NotifyClientToUpdateRectTask(updateReason, rsTransaction);
        if (ret != WSError::WS_OK) {
            return ret;
        }
        if (session->specificCallback_ != nullptr && session->specificCallback_->onUpdateAvoidArea_ != nullptr) {
            if (Session::IsScbCoreEnabled()) {
                session->dirtyFlags_ |= static_cast<uint32_t>(SessionUIDirtyFlag::AVOID_AREA);
            } else {
                session->specificCallback_->onUpdateAvoidArea_(session->GetPersistentId());
            }
        }
        return ret;
    }, "NotifyClientToUpdateRect");
    return WSError::WS_OK;
}

bool SystemSession::CheckKeyEventDispatch(const std::shared_ptr<MMI::KeyEvent>& keyEvent) const
{
    auto currentRect = GetSessionRect();
    if (!GetRSVisible() || currentRect.width_ == 0 || currentRect.height_ == 0) {
        WLOGE("Error size: [width: %{public}d, height: %{public}d], isRSVisible_: %{public}d,"
            " persistentId: %{public}d",
            currentRect.width_, currentRect.height_, GetRSVisible(), GetPersistentId());
        return false;
    }

    auto parentSession = GetParentSession();
    if (parentSession == nullptr) {
        WLOGFW("Dialog parent is null");
        return false;
    }
    auto parentSessionState = parentSession->GetSessionState();
    if ((parentSessionState != SessionState::STATE_FOREGROUND &&
        parentSessionState != SessionState::STATE_ACTIVE) ||
        (state_ != SessionState::STATE_FOREGROUND &&
        state_ != SessionState::STATE_ACTIVE)) {
        TLOGE(WmsLogTag::WMS_DIALOG, "Dialog's parent info : [persistentId: %{public}d, state:%{public}d];"
            "Dialog info:[persistentId: %{public}d, state:%{public}d]",
            parentSession->GetPersistentId(), parentSessionState, GetPersistentId(), GetSessionState());
        return false;
    }
    return true;
}

bool SystemSession::NeedSystemPermission(WindowType type)
{
    return !(type == WindowType::WINDOW_TYPE_SCENE_BOARD || type == WindowType::WINDOW_TYPE_SYSTEM_FLOAT ||
        type == WindowType::WINDOW_TYPE_SYSTEM_SUB_WINDOW || type == WindowType::WINDOW_TYPE_TOAST ||
        type == WindowType::WINDOW_TYPE_DRAGGING_EFFECT || type == WindowType::WINDOW_TYPE_APP_LAUNCHING ||
        type == WindowType::WINDOW_TYPE_PIP || type == WindowType::WINDOW_TYPE_FLOAT ||
        type == WindowType::WINDOW_TYPE_FB);
}

bool SystemSession::CheckPointerEventDispatch(const std::shared_ptr<MMI::PointerEvent>& pointerEvent) const
{
    auto sessionState = GetSessionState();
    int32_t action = pointerEvent->GetPointerAction();
    auto isPC = systemConfig_.IsPcWindow();
    bool isDialog = WindowHelper::IsDialogWindow(GetWindowType());
    if (isPC && isDialog && sessionState != SessionState::STATE_FOREGROUND &&
        sessionState != SessionState::STATE_ACTIVE &&
        action != MMI::PointerEvent::POINTER_ACTION_LEAVE_WINDOW) {
        WLOGFW("false, Current Session Info: [persistentId: %{public}d, "
            "state: %{public}d, action:%{public}d]", GetPersistentId(), GetSessionState(), action);
        return false;
    }
    return true;
}

void SystemSession::UpdatePointerArea(const WSRect& rect)
{
    auto property = GetSessionProperty();
    if (!(property->IsDecorEnable() && GetWindowMode() == WindowMode::WINDOW_MODE_FLOATING)) {
        return;
    }
    Session::UpdatePointerArea(rect);
}

void SystemSession::RectCheck(uint32_t curWidth, uint32_t curHeight)
{
    uint32_t minWidth = MIN_SYSTEM_WINDOW_WIDTH;
    uint32_t minHeight = MIN_SYSTEM_WINDOW_HEIGHT;
    uint32_t maxFloatingWindowSize = GetSystemConfig().maxFloatingWindowSize_;
    RectSizeCheckProcess(curWidth, curHeight, minWidth, minHeight, maxFloatingWindowSize);
}

bool SystemSession::IsVisibleForeground() const
{
    if (GetWindowType() == WindowType::WINDOW_TYPE_DIALOG &&
        parentSession_ && WindowHelper::IsMainWindow(parentSession_->GetWindowType())) {
        return parentSession_->IsVisibleForeground() && Session::IsVisibleForeground();
    }
    return Session::IsVisibleForeground();
}

bool SystemSession::IsVisibleNotBackground() const
{
    if (GetWindowType() == WindowType::WINDOW_TYPE_DIALOG &&
        parentSession_ && WindowHelper::IsMainWindow(parentSession_->GetWindowType())) {
        return parentSession_->IsVisibleNotBackground() && Session::IsVisibleNotBackground();
    }
    return Session::IsVisibleNotBackground();
}

WSError SystemSession::SetDialogSessionBackGestureEnabled(bool isEnabled)
{
    return PostSyncTask([weakThis = wptr(this), isEnabled]() {
        auto session = weakThis.promote();
        if (!session) {
            WLOGFE("session is null");
            return WSError::WS_ERROR_DESTROYED_OBJECT;
        }
        WindowType windowType = session->GetWindowType();
        if (windowType != WindowType::WINDOW_TYPE_DIALOG) {
            TLOGE(WmsLogTag::WMS_DIALOG, "windowType not support. WinId:%{public}u, WindowType:%{public}u",
                session->GetWindowId(), static_cast<uint32_t>(windowType));
            return WSError::WS_ERROR_INVALID_CALLING;
        }
        session->dialogSessionBackGestureEnabled_ = isEnabled;
        return WSError::WS_OK;
    });
}

void SystemSession::UpdatePiPWindowStateChanged(bool isForeground)
{
    if (specificCallback_ == nullptr || !specificCallback_->onPiPStateChange_) {
        return;
    }
    if (GetWindowType() == WindowType::WINDOW_TYPE_PIP) {
        TLOGI(WmsLogTag::WMS_PIP, "pip state changed, bundleName:%{public}s, state:%{public}d",
            GetSessionInfo().bundleName_.c_str(), isForeground);
        specificCallback_->onPiPStateChange_(GetSessionInfo().bundleName_, isForeground);
    } else {
        TLOGD(WmsLogTag::WMS_PIP, "skip type");
    }
}

int32_t SystemSession::GetSubWindowZLevel() const
{
    int32_t zLevel = 0;
    auto sessionProperty = GetSessionProperty();
    zLevel = sessionProperty->GetSubWindowZLevel();
    return zLevel;
}

WMError SystemSession::UpdateFloatingBall(const FloatingBallTemplateInfo& fbTemplateInfo)
{
    if (!WindowHelper::IsFbWindow(GetWindowType())) {
        return WMError::WM_DO_NOTHING;
    }

    if (GetFbTemplateInfo().template_ == static_cast<uint32_t>(FloatingBallTemplate::STATIC)) {
        TLOGE(WmsLogTag::WMS_SYSTEM, "Fb static template can't update");
        return WMError::WM_ERROR_FB_UPDATE_STATIC_TEMPLATE_DENIED;
    }

    if (GetFbTemplateInfo().template_ != 0 && GetFbTemplateInfo().template_ != fbTemplateInfo.template_) {
        TLOGE(WmsLogTag::WMS_SYSTEM, "Fb template type can't update %{public}d, %{public}d",
            GetFbTemplateInfo().template_, fbTemplateInfo.template_);
        return WMError::WM_ERROR_FB_UPDATE_TEMPLATE_TYPE_DENIED;
    }

    int32_t callingPid = IPCSkeleton::GetCallingPid();
    auto task = [weakThis = wptr(this), fbTemplateInfo, callingPid, where = __func__] {
        auto session = weakThis.promote();
        if (!session) {
            TLOGNE(WmsLogTag::WMS_SYSTEM, "%{public}s session is null", where);
            return WMError::WM_ERROR_INVALID_OPERATION;
        }
        if (callingPid != session->GetCallingPid()) {
            TLOGNW(WmsLogTag::WMS_SYSTEM, "%{public}s permission denied, not call by the same process", where);
            return WMError::WM_ERROR_INVALID_CALLING;
        }
        TLOGNI(WmsLogTag::WMS_SYSTEM, "update template %{public}d", fbTemplateInfo.template_);
        session->NotifyUpdateFloatingBall(fbTemplateInfo);
        return WMError::WM_OK;
    };
    PostTask(std::move(task), __func__);
    return WMError::WM_OK;
}

WSError SystemSession::StopFloatingBall()
{
    TLOGI(WmsLogTag::WMS_SYSTEM, "session StopFloatingBall");
    if (!WindowHelper::IsFbWindow(GetWindowType())) {
        return WSError::WS_DO_NOTHING;
    }
    int32_t callingPid = IPCSkeleton::GetCallingPid();
    auto task = [weakThis = wptr(this), callingPid, where = __func__] {
        auto session = weakThis.promote();
        if (!session) {
            TLOGNE(WmsLogTag::WMS_SYSTEM, "%{public}s session is null", where);
            return WSError::WS_ERROR_INVALID_OPERATION;
        }
        if (callingPid != session->GetCallingPid()) {
            TLOGNW(WmsLogTag::WMS_SYSTEM, "%{public}s permission denied, not call by the same process", where);
            return WSError::WS_ERROR_INVALID_CALLING;
        }
        session->NotifyStopFloatingBall();
        return WSError::WS_OK;
    };
    PostTask(std::move(task), __func__);
    return WSError::WS_OK;
}

WMError SystemSession::GetFloatingBallWindowId(uint32_t& windowId)
{
    TLOGI(WmsLogTag::WMS_SYSTEM, "session GetFloatingBallWindowId");
    if (!WindowHelper::IsFbWindow(GetWindowType())) {
        return WMError::WM_DO_NOTHING;
    }
    int32_t callingPid = IPCSkeleton::GetCallingPid();
    return PostSyncTask([weakThis = wptr(this), callingPid, &windowId, where = __func__] {
        auto session = weakThis.promote();
        if (!session) {
            TLOGNE(WmsLogTag::WMS_SYSTEM, "%{public}s session is null", where);
            return WMError::WM_ERROR_INVALID_OPERATION;
        }
        if (callingPid != session->GetCallingPid()) {
            TLOGNW(WmsLogTag::WMS_SYSTEM, "%{public}s permission denied, not call by the same process", where);
            return WMError::WM_ERROR_INVALID_CALLING;
        }
        windowId = session->GetFbWindowId();
        TLOGND(WmsLogTag::WMS_SYSTEM, "%{public}s mode: %{public}u", where, windowId);
        return WMError::WM_OK;
    }, __func__);
}

WMError SystemSession::RestoreFbMainWindow(const std::shared_ptr<AAFwk::Want>& want)
{
    if (!WindowHelper::IsFbWindow(GetWindowType())) {
        return WMError::WM_DO_NOTHING;
    }
    if (!SessionPermission::VerifyCallingPermission(PermissionConstants::PERMISSION_FLOATING_BALL)) {
        TLOGNE(WmsLogTag::WMS_SYSTEM, "Check floating ball permission failed");
        return WMError::WM_ERROR_INVALID_PERMISSION;
    }
    int32_t callingPid = IPCSkeleton::GetCallingPid();
    return PostSyncTask([weakThis = wptr(this), &want, callingPid, where = __func__]() {
        auto session = weakThis.promote();
        if (!session) {
            TLOGNE(WmsLogTag::WMS_SYSTEM, "%{public}s session is null", where);
            return WMError::WM_ERROR_INVALID_OPERATION;
        }
        if (callingPid != session->GetCallingPid()) {
            TLOGNE(WmsLogTag::WMS_SYSTEM, "%{public}s permission denied, not call by the same process", where);
            return WMError::WM_ERROR_INVALID_CALLING;
        }
        if (session->GetSessionInfo().bundleName_ != want->GetBundle()) {
            TLOGNE(WmsLogTag::WMS_SYSTEM, "Session bundle %{public}s is not want bundle %{public}s",
                session->GetSessionInfo().bundleName_.c_str(), want->GetBundle().c_str());
            return WMError::WM_ERROR_FB_RESTORE_MAIN_WINDOW_FAILED;
        }
        {
            std::lock_guard<std::mutex> lock(session->fbClickMutex_);
            if (session->fbClickCnt_ == 0) {
                TLOGNE(WmsLogTag::WMS_SYSTEM, "%{public}s not click, deny", where);
                return WMError::WM_ERROR_FB_RESTORE_MAIN_WINDOW_FAILED;
            }
            session->fbClickCnt_--;
        }
        TLOGNI(WmsLogTag::WMS_SYSTEM,
            "%{public}s restore window, bundle %{public}s, ability %{public}s, session bundle %{public}s",
            where, want->GetBundle().c_str(), want->GetElement().GetAbilityName().c_str(),
            session->GetSessionInfo().bundleName_.c_str());
        session->NotifyRestoreFloatingBallMainWindow(want);
        return WMError::WM_OK;
    });
}

void SystemSession::NotifyUpdateFloatingBall(const FloatingBallTemplateInfo& fbTemplateInfo)
{
    PostTask([weakThis = wptr(this), fbTemplateInfo, where = __func__] {
        auto session = weakThis.promote();
        if (!session) {
            TLOGNE(WmsLogTag::WMS_LAYOUT, "%{public}s session is null", where);
            return;
        }
        session->SetFbTemplateInfo(fbTemplateInfo);
        if (session->updateFloatingBallFunc_) {
            session->updateFloatingBallFunc_(fbTemplateInfo);
        }
    }, __func__);
}

void SystemSession::NotifyStopFloatingBall()
{
    TLOGI(WmsLogTag::DEFAULT, "Notify StopFloatingBall");
    PostTask([weakThis = wptr(this), where = __func__] {
        auto session = weakThis.promote();
        if (!session) {
            TLOGNE(WmsLogTag::WMS_SYSTEM, "%{public}s session is null", where);
            return;
        }
        if (session->stopFloatingBallFunc_) {
            TLOGI(WmsLogTag::WMS_SYSTEM, "StopFloatingBall Func");
            session->stopFloatingBallFunc_();
        } else {
            TLOGW(WmsLogTag::WMS_SYSTEM, "%{public}s StopFloatingBall Func is null", where);
            session->needStopFb_ = true;
        }
    }, __func__);
}

void SystemSession::NotifyRestoreFloatingBallMainWindow(const std::shared_ptr<AAFwk::Want>& want)
{
    TLOGI(WmsLogTag::WMS_SYSTEM, "Notify RestoreFloatingBallMainwindow");
    PostTask([weakThis = wptr(this), want, where = __func__] {
        auto session = weakThis.promote();
        if (!session) {
            TLOGNE(WmsLogTag::WMS_SYSTEM, "%{public}s session is null", where);
            return;
        }
        if (session->restoreFloatingBallMainWindowFunc_) {
            TLOGI(WmsLogTag::WMS_SYSTEM, "restoreFlotingBallMainwindowFunc");
            session->restoreFloatingBallMainWindowFunc_(want);
        }  else {
            TLOGW(WmsLogTag::WMS_SYSTEM, "%{public}s restoreFlotingBallMainwindowFunc Func is null", where);
            session->fbWant_= want;
        }
    }, __func__);
}

WSError SystemSession::SendFbActionEvent(const std::string& action)
{
    TLOGI(WmsLogTag::WMS_SYSTEM, "action: %{public}s", action.c_str());
    if (!sessionStage_) {
        return WSError::WS_ERROR_NULLPTR;
    }
    if (action == FB_CLICK_EVENT) {
        std::lock_guard<std::mutex> lock(fbClickMutex_);
        if (fbClickCnt_ < MAX_FB_CLICK_COUNT) {
            ++fbClickCnt_;
        }
    }
    return sessionStage_->SendFbActionEvent(action);
}

FloatingBallTemplateInfo SystemSession::GetFbTemplateInfo() const
{
    std::lock_guard<std::mutex> lock(fbTemplateMutex_);
    return fbTemplateInfo_;
}

void SystemSession::SetFbTemplateInfo(const FloatingBallTemplateInfo& fbTemplateInfo)
{
    std::lock_guard<std::mutex> lock(fbTemplateMutex_);
    fbTemplateInfo_ = fbTemplateInfo;
}

uint32_t SystemSession::GetFbWindowId() const
{
    uint32_t windowId = 0;
    if (getFbPanelWindowIdFunc_) {
        getFbPanelWindowIdFunc_(windowId);
    }
    TLOGI(WmsLogTag::WMS_SYSTEM, "Get fb window Id %{public}d", windowId);
    return windowId;
}

void SystemSession::SetFloatingBallUpdateCallback(NotifyUpdateFloatingBallFunc&& func)
{
    PostTask([weakThis = wptr(this), func = std::move(func), where = __func__] {
        auto session = weakThis.promote();
        if (!session || !func) {
            TLOGNE(WmsLogTag::WMS_SYSTEM, "%{public}s update floating ball id is null", where);
            return;
        }
        session->updateFloatingBallFunc_ = std::move(func);
        session->updateFloatingBallFunc_(session->GetFbTemplateInfo());
    }, __func__);
}

void SystemSession::SetFloatingBallStopCallback(NotifyStopFloatingBallFunc&& func)
{
    PostTask([weakThis = wptr(this), func = std::move(func), where = __func__] {
        TLOGI(WmsLogTag::WMS_SYSTEM, "Register Callback StopFloatingBall");
        auto session = weakThis.promote();
        if (!session || !func) {
            TLOGNE(WmsLogTag::WMS_SYSTEM, "%{public}s stop floating ball id is null", where);
            return;
        }
        session->stopFloatingBallFunc_ = std::move(func);
        if (session->needStopFb_) {
            TLOGW(WmsLogTag::WMS_SYSTEM, "StopFloatingBall when register callBack");
            session->stopFloatingBallFunc_();
            session->needStopFb_ = false;
        }
    }, __func__);
}

void SystemSession::SetFloatingBallRestoreMainWindowCallback(NotifyRestoreFloatingBallMainWindowFunc&& func)
{
    PostTask([weakThis = wptr(this), func = std::move(func), where = __func__] {
        TLOGI(WmsLogTag::WMS_SYSTEM, "Register Callback RestoreFloatingBallMainWindow");
        auto session = weakThis.promote();
        if (!session || !func) {
            TLOGNE(WmsLogTag::WMS_SYSTEM, "%{public}s stop floating ball id is null", where);
            return;
        }
        session->restoreFloatingBallMainWindowFunc_ = std::move(func);
        if (session->fbWant_ != nullptr) {
            TLOGW(WmsLogTag::WMS_SYSTEM, "RestoreFbMainWindow when register callBack");
            session->restoreFloatingBallMainWindowFunc_(session->fbWant_);
            session->fbWant_ = nullptr;
        }
    }, __func__);
}

void SystemSession::SetRestoreFloatMainWindowCallback(NotifyRestoreFloatMainWindowFunc&& func)
{
    PostTask(
        [weakThis = wptr(this), func = std::move(func), where = __func__] {
            TLOGI(WmsLogTag::WMS_SYSTEM, "Register Callback RestoreFloatMainWindow");
            auto session = weakThis.promote();
            if (!session || !func) {
                TLOGNE(WmsLogTag::WMS_SYSTEM, "%{public}s session or func dont exists", where);
                return;
            }
            session->restoreFloatMainWindowFunc_ = std::move(func);
        }, __func__);
}

WMError SystemSession::RestoreFloatMainWindow(const std::shared_ptr<AAFwk::WantParams>& wantParameters)
{
    int32_t callingPid = IPCSkeleton::GetCallingPid();
    if (GetWindowType() != WindowType::WINDOW_TYPE_FLOAT) {
        TLOGNE(WmsLogTag::WMS_SYSTEM, "Check window type failed");
        return WMError::WM_ERROR_INVALID_CALLING;
    }
    return PostSyncTask([weakThis = wptr(this), callingPid, wantParameters, where = __func__, state = state_.load()]() {
        auto session = weakThis.promote();
        if (!session) {
            TLOGE(WmsLogTag::WMS_SYSTEM, "%{public}s session is null", where);
            return WMError::WM_ERROR_INVALID_OPERATION;
        }
        if (callingPid != session->GetCallingPid()) {
            TLOGE(WmsLogTag::WMS_SYSTEM, "%{public}s permission denied, not call by the same process", where);
            return WMError::WM_ERROR_INVALID_CALLING;
        }
        if (state != SessionState::STATE_FOREGROUND && state != SessionState::STATE_ACTIVE) {
            TLOGE(WmsLogTag::WMS_SYSTEM, "%{public}s window state is not at foreground or active", where);
            return WMError::WM_ERROR_INVALID_CALLING;
        }
        if (!session->getSCBEnterRecentFunc_) {
            TLOGE(WmsLogTag::WMS_SYSTEM, "cannot get recent func");
            return WMError::WM_ERROR_SYSTEM_ABNORMALLY;
        }
        if (session->getSCBEnterRecentFunc_()) {
            TLOGE(WmsLogTag::WMS_SYSTEM, "current window is at recent state");
            return WMError::WM_ERROR_START_ABILITY_FAILED;
        }
        {
            std::lock_guard<std::mutex> lock(session->floatWindowDownEventMutex_);
            if (session->floatWindowDownEventCnt_ == 0) {
                TLOGE(WmsLogTag::WMS_SYSTEM, "%{public}s no enough click, deny", where);
                return WMError::WM_ERROR_INVALID_CALLING;
            }
            session->floatWindowDownEventCnt_ = 0;
        }
        TLOGI(WmsLogTag::WMS_SYSTEM, "%{public}s restore float main window", where);
        session->NotifyRestoreFloatMainWindow(wantParameters);
        return WMError::WM_OK;
    });
}

void SystemSession::NotifyRestoreFloatMainWindow(const std::shared_ptr<AAFwk::WantParams>& wantParameters)
{
    TLOGI(WmsLogTag::WMS_SYSTEM, "Notify RestoreFloatingBallMainwindow");
    PostTask(
        [weakThis = wptr(this), wantParameters, where = __func__] {
            auto session = weakThis.promote();
            if (!session) {
                TLOGNE(WmsLogTag::WMS_SYSTEM, "%{public}s session is null", where);
                return;
            }
            if (session->restoreFloatMainWindowFunc_) {
                TLOGI(WmsLogTag::WMS_SYSTEM, "restoreFloatMainWindowFunc");
                session->restoreFloatMainWindowFunc_(wantParameters);
            } else {
                TLOGW(WmsLogTag::WMS_SYSTEM, "%{public}s restoreFloatMainWindowFunc Func is null", where);
            }
        }, __func__);
}

void SystemSession::RegisterGetSCBEnterRecentFunc(GetSCBEnterRecentFunc&& callback)
{
    getSCBEnterRecentFunc_ = std::move(callback);
}

void SystemSession::RegisterGetFbPanelWindowIdFunc(GetFbPanelWindowIdFunc&& callback)
{
    getFbPanelWindowIdFunc_ = std::move(callback);
}
} // namespace OHOS::Rosen
