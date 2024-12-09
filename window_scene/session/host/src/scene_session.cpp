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

#include <ability_manager_client.h>
#include <algorithm>
#include <climits>
#include <hitrace_meter.h>
#include <type_traits>
#ifdef IMF_ENABLE
#include <input_method_controller.h>
#endif // IMF_ENABLE
#include <ipc_skeleton.h>
#include <pointer_event.h>
#include <transaction/rs_sync_transaction_controller.h>
#include <transaction/rs_transaction.h>
#include <ui/rs_surface_node.h>

#include "proxy/include/window_info.h"

#include "common/include/session_permission.h"
#ifdef DEVICE_STATUS_ENABLE
#include "interaction_manager.h"
#endif // DEVICE_STATUS_ENABLE
#include "interfaces/include/ws_common.h"
#include "pixel_map.h"
#include "session/screen/include/screen_session.h"
#include "screen_session_manager/include/screen_session_manager_client.h"
#include "session/host/include/scene_persistent_storage.h"
#include "session/host/include/session_utils.h"
#include "display_manager.h"
#include "session_helper.h"
#include "window_helper.h"
#include "window_manager_hilog.h"
#include "wm_math.h"
#include <running_lock.h>
#include "screen_manager.h"
#include "screen.h"
#include "singleton_container.h"
#include "screen_session_manager/include/screen_session_manager_client.h"
#include "fold_screen_state_internel.h"
#include "session/host/include/ability_info_manager.h"

#ifdef POWER_MANAGER_ENABLE
#include <power_mgr_client.h>
#endif

namespace OHOS::Rosen {
namespace {
constexpr HiviewDFX::HiLogLabel LABEL = { LOG_CORE, HILOG_DOMAIN_WINDOW, "SceneSession" };
const std::string DLP_INDEX = "ohos.dlp.params.index";
constexpr const char* APP_CLONE_INDEX = "ohos.extra.param.key.appCloneIndex";

bool CheckIfRectElementIsTooLarge(const WSRect& rect)
{
    int32_t largeNumber = static_cast<int32_t>(SHRT_MAX);
    if (rect.posX_ >= largeNumber || rect.posY_ >= largeNumber ||
        rect.width_ >= largeNumber || rect.height_ >= largeNumber) {
        return true;
    }
    return false;
}
} // namespace

MaximizeMode SceneSession::maximizeMode_ = MaximizeMode::MODE_RECOVER;
wptr<SceneSession> SceneSession::enterSession_ = nullptr;
std::mutex SceneSession::enterSessionMutex_;
std::shared_mutex SceneSession::windowDragHotAreaMutex_;
std::map<uint32_t, WSRect> SceneSession::windowDragHotAreaMap_;
static bool g_enableForceUIFirst = system::GetParameter("window.forceUIFirst.enabled", "1") == "1";

SceneSession::SceneSession(const SessionInfo& info, const sptr<SpecificSessionCallback>& specificCallback)
    : Session(info)
{
    GeneratePersistentId(false, info.persistentId_);
    specificCallback_ = specificCallback;
    SetCollaboratorType(info.collaboratorType_);
    TLOGI(WmsLogTag::WMS_LIFE, "Create session, id: %{public}d", GetPersistentId());
}

SceneSession::~SceneSession()
{
    TLOGI(WmsLogTag::WMS_LIFE, "~SceneSession, id: %{public}d", GetPersistentId());
}

WSError SceneSession::ConnectInner(const sptr<ISessionStage>& sessionStage,
    const sptr<IWindowEventChannel>& eventChannel,
    const std::shared_ptr<RSSurfaceNode>& surfaceNode, SystemSessionConfig& systemConfig,
    sptr<WindowSessionProperty> property, sptr<IRemoteObject> token, int32_t pid, int32_t uid,
    const std::string& identityToken)
{
    auto task = [weakThis = wptr(this), sessionStage, eventChannel, surfaceNode, &systemConfig, property, token, pid,
        uid, identityToken]() {
        auto session = weakThis.promote();
        if (!session) {
            TLOGE(WmsLogTag::WMS_LIFE, "session is null");
            return WSError::WS_ERROR_DESTROYED_OBJECT;
        }

        if (SessionHelper::IsMainWindow(session->GetWindowType())) {
            if (!session->CheckIdentityTokenIfMatched(identityToken)) {
                TLOGNW(WmsLogTag::WMS_LIFE, "check failed");
                return WSError::WS_OK;
            }
        }
        if (property) {
            property->SetCollaboratorType(session->GetCollaboratorType());
        }
        auto ret = session->Session::ConnectInner(
            sessionStage, eventChannel, surfaceNode, systemConfig, property, token, pid, uid);
        if (ret != WSError::WS_OK) {
            return ret;
        }
        session->NotifyPropertyWhenConnect();
        session->isStatusBarVisible_ = true;
        return ret;
    };
    return PostSyncTask(task, "ConnectInner");
}

WSError SceneSession::Connect(const sptr<ISessionStage>& sessionStage, const sptr<IWindowEventChannel>& eventChannel,
    const std::shared_ptr<RSSurfaceNode>& surfaceNode, SystemSessionConfig& systemConfig,
    sptr<WindowSessionProperty> property, sptr<IRemoteObject> token,
    const std::string& identityToken)
{
    // Get pid and uid before posting task.
    int32_t pid = IPCSkeleton::GetCallingRealPid();
    int32_t uid = IPCSkeleton::GetCallingUid();
    return ConnectInner(sessionStage, eventChannel, surfaceNode, systemConfig,
        property, token, pid, uid, identityToken);
}

WSError SceneSession::Reconnect(const sptr<ISessionStage>& sessionStage, const sptr<IWindowEventChannel>& eventChannel,
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
        return session->ReconnectInner(property);
    });
}

WSError SceneSession::ReconnectInner(sptr<WindowSessionProperty> property)
{
    if (property == nullptr) {
        TLOGE(WmsLogTag::WMS_RECOVER, "property is nullptr");
        return WSError::WS_ERROR_NULLPTR;
    }
    WindowState windowState = property->GetWindowState();
    TLOGI(WmsLogTag::WMS_RECOVER, "persistentId: %{public}d, windowState: %{public}d ", GetPersistentId(), windowState);
    WSError ret = WSError::WS_OK;
    switch (windowState) {
        case WindowState::STATE_INITIAL: {
            TLOGE(WmsLogTag::WMS_RECOVER, "persistentId: %{public}d, invalid window state: STATE_INITIAL",
                GetPersistentId());
            ret = WSError::WS_ERROR_INVALID_PARAM;
            break;
        }
        case WindowState::STATE_CREATED:
            break;
        case WindowState::STATE_SHOWN: {
            UpdateSessionState(SessionState::STATE_FOREGROUND);
            UpdateActiveStatus(true);
            break;
        }
        case WindowState::STATE_HIDDEN: {
            UpdateSessionState(SessionState::STATE_BACKGROUND);
            break;
        }
        default:
            TLOGE(WmsLogTag::WMS_RECOVER, "persistentId: %{public}d, invalid window state: %{public}u",
                GetPersistentId(), windowState);
            ret = WSError::WS_ERROR_INVALID_PARAM;
            break;
    }
    if (ret != WSError::WS_OK) {
        Session::Disconnect(false);
    }
    return ret;
}

WSError SceneSession::Foreground(
    sptr<WindowSessionProperty> property, bool isFromClient, const std::string& identityToken)
{
    if (!CheckPermissionWithPropertyAnimation(property)) {
        return WSError::WS_ERROR_NOT_SYSTEM_APP;
    }
    if (isFromClient && SessionHelper::IsMainWindow(GetWindowType())) {
        if (!CheckPidIfMatched() || !CheckIdentityTokenIfMatched(identityToken)) {
            TLOGW(WmsLogTag::WMS_LIFE, "check failed");
            return WSError::WS_OK;
        }
    }
    return ForegroundTask(property);
}

WSError SceneSession::ForegroundTask(const sptr<WindowSessionProperty>& property)
{
    auto task = [weakThis = wptr(this), property]() {
        auto session = weakThis.promote();
        if (!session) {
            TLOGE(WmsLogTag::WMS_LIFE, "session is null");
            return WSError::WS_ERROR_DESTROYED_OBJECT;
        }
        auto sessionProperty = session->GetSessionProperty();
        if (property && sessionProperty) {
            sessionProperty->SetWindowMode(property->GetWindowMode());
            sessionProperty->SetDecorEnable(property->IsDecorEnable());
            session->SetFocusableOnShow(property->GetFocusableOnShow());
        }
        int32_t persistentId = session->GetPersistentId();
        auto ret = session->Session::Foreground(property);
        if (ret != WSError::WS_OK) {
            TLOGE(WmsLogTag::WMS_LIFE, "session foreground failed, ret=%{public}d persistentId=%{public}d",
                ret, persistentId);
            return ret;
        }
        auto leashWinSurfaceNode = session->GetLeashWinSurfaceNode();
        if (leashWinSurfaceNode && sessionProperty) {
            bool lastPrivacyMode = sessionProperty->GetPrivacyMode() || sessionProperty->GetSystemPrivacyMode();
            leashWinSurfaceNode->SetSecurityLayer(lastPrivacyMode);
        }
        if (session->specificCallback_ != nullptr) {
            if (Session::IsScbCoreEnabled()) {
                session->dirtyFlags_ |= static_cast<uint32_t>(SessionUIDirtyFlag::AVOID_AREA);
            } else {
                session->specificCallback_->onUpdateAvoidArea_(persistentId);
            }
            session->specificCallback_->onWindowInfoUpdate_(
                persistentId, WindowUpdateType::WINDOW_UPDATE_ADDED);
            session->specificCallback_->onHandleSecureSessionShouldHide_(session);
            session->UpdateGestureBackEnabled();
        } else {
            TLOGI(WmsLogTag::WMS_LIFE, "foreground specific callback does not take effect, callback function null");
        }
        return WSError::WS_OK;
    };
    PostTask(task, "Foreground");
    return WSError::WS_OK;
}

WSError SceneSession::Background(bool isFromClient, const std::string& identityToken)
{
    if (!CheckPermissionWithPropertyAnimation(GetSessionProperty())) {
        return WSError::WS_ERROR_NOT_SYSTEM_APP;
    }

    if (isFromClient && SessionHelper::IsMainWindow(GetWindowType())) {
        if (!CheckPidIfMatched() || !CheckIdentityTokenIfMatched(identityToken)) {
            TLOGW(WmsLogTag::WMS_LIFE, "check failed");
            return WSError::WS_OK;
        }
    }

    return BackgroundTask(true);
}

WSError SceneSession::NotifyFrameLayoutFinishFromApp(bool notifyListener, const WSRect& rect)
{
    TLOGI(WmsLogTag::WMS_LAYOUT, "%{public}d, %{public}s", notifyListener, rect.ToString().c_str());
    auto task = [weakThis = wptr(this), notifyListener, rect]() {
        auto session = weakThis.promote();
        if (!session) {
            TLOGE(WmsLogTag::WMS_MULTI_WINDOW, "session is null");
            return WSError::WS_ERROR_DESTROYED_OBJECT;
        }
        session->layoutRect_ = rect;
        session->NotifyLayoutFinished();
        if (notifyListener && session->frameLayoutFinishFunc_) {
            TLOGD(WmsLogTag::WMS_MULTI_WINDOW, "id: %{public}d", session->GetPersistentId());
            session->frameLayoutFinishFunc_();
        }
        return WSError::WS_OK;
    };
    PostTask(task, "NotifyFrameLayoutFinishFromApp");
    return WSError::WS_OK;
}

WSError SceneSession::BackgroundTask(const bool isSaveSnapshot)
{
    auto task = [weakThis = wptr(this), isSaveSnapshot]() {
        auto session = weakThis.promote();
        if (!session) {
            TLOGE(WmsLogTag::WMS_LIFE, "session is null");
            return WSError::WS_ERROR_DESTROYED_OBJECT;
        }
        auto state = session->GetSessionState();
        if (state == SessionState::STATE_BACKGROUND) {
            return WSError::WS_OK;
        }
        auto ret = session->Session::Background();
        if (ret != WSError::WS_OK) {
            return ret;
        }
        if (WindowHelper::IsMainWindow(session->GetWindowType()) && isSaveSnapshot) {
            session->SaveSnapshot(true);
        }
        if (session->specificCallback_ != nullptr) {
            if (Session::IsScbCoreEnabled()) {
                session->dirtyFlags_ |= static_cast<uint32_t>(SessionUIDirtyFlag::AVOID_AREA);
            } else {
                session->specificCallback_->onUpdateAvoidArea_(session->GetPersistentId());
            }
            session->specificCallback_->onWindowInfoUpdate_(
                session->GetPersistentId(), WindowUpdateType::WINDOW_UPDATE_REMOVED);
            session->specificCallback_->onHandleSecureSessionShouldHide_(session);
            session->UpdateGestureBackEnabled();
        }
        return WSError::WS_OK;
    };
    PostTask(task, "Background");
    return WSError::WS_OK;
}

void SceneSession::ClearSpecificSessionCbMap()
{
    auto task = [weakThis = wptr(this)]() {
        auto session = weakThis.promote();
        if (!session) {
            TLOGE(WmsLogTag::WMS_SYSTEM, "session is null");
            return;
        }
        if (session->clearCallbackMapFunc_) {
            session->clearCallbackMapFunc_(true, session->GetPersistentId());
            TLOGD(WmsLogTag::WMS_SYSTEM, "ClearCallbackMap, id: %{public}d", session->GetPersistentId());
        } else {
            TLOGE(WmsLogTag::WMS_SYSTEM, "get callback failed, id: %{public}d", session->GetPersistentId());
        }
    };
    PostTask(task, "ClearSpecificSessionCbMap");
}

void SceneSession::RegisterShowWhenLockedCallback(NotifyShowWhenLockedFunc&& callback)
{
    const char* const where = __func__;
    auto task = [weakThis = wptr(this), callback = std::move(callback), where] {
        auto session = weakThis.promote();
        if (!session) {
            TLOGNE(WmsLogTag::WMS_LIFE, "%{public}s session is nullptr", where);
            return;
        }
        session->onShowWhenLockedFunc_ = std::move(callback);
        session->onShowWhenLockedFunc_(session->GetShowWhenLockedFlagValue());
    };
    PostTask(task, where);
}

void SceneSession::RegisterForceHideChangeCallback(NotifyForceHideChangeFunc&& callback)
{
    const char* const where = __func__;
    auto task = [weakThis = wptr(this), callback = std::move(callback), where] {
        auto session = weakThis.promote();
        if (!session) {
            TLOGNE(WmsLogTag::WMS_LIFE, "%{public}s session is nullptr", where);
            return;
        }
        session->onForceHideChangeFunc_ = std::move(callback);
    };
    PostTask(task, where);
}

void SceneSession::RegisterClearCallbackMapCallback(ClearCallbackMapFunc&& callback)
{
    const char* const where = __func__;
    auto task = [weakThis = wptr(this), callback = std::move(callback), where] {
        auto session = weakThis.promote();
        if (!session) {
            TLOGNE(WmsLogTag::WMS_LIFE, "%{public}s session is nullptr", where);
            return;
        }
        session->clearCallbackMapFunc_ = std::move(callback);
    };
    PostTask(task, where);
}

WSError SceneSession::Disconnect(bool isFromClient, const std::string& identityToken)
{
    if (isFromClient && SessionHelper::IsMainWindow(GetWindowType())) {
        if (!CheckPidIfMatched() || !CheckIdentityTokenIfMatched(identityToken)) {
            TLOGW(WmsLogTag::WMS_LIFE, "check failed");
            return WSError::WS_OK;
        }
    }

    return DisconnectTask(isFromClient, true);
}

WSError SceneSession::DisconnectTask(bool isFromClient, bool isSaveSnapshot)
{
    PostTask([weakThis = wptr(this), isFromClient, isSaveSnapshot]() {
        auto session = weakThis.promote();
        if (!session) {
            TLOGE(WmsLogTag::WMS_LIFE, "session is null");
            return WSError::WS_ERROR_DESTROYED_OBJECT;
        }
        if (isFromClient) {
            TLOGI(WmsLogTag::WMS_LIFE, "Client need notify destroy session, id: %{public}d",
                session->GetPersistentId());
            session->SetSessionState(SessionState::STATE_DISCONNECT);
            return WSError::WS_OK;
        }
        auto state = session->GetSessionState();
        auto isMainWindow = SessionHelper::IsMainWindow(session->GetWindowType());
        if ((session->needSnapshot_ || (state == SessionState::STATE_ACTIVE && isMainWindow)) && isSaveSnapshot) {
            session->SaveSnapshot(false);
        }
        session->Session::Disconnect(isFromClient);
        session->isTerminating_ = false;
        if (session->specificCallback_ != nullptr) {
            session->specificCallback_->onHandleSecureSessionShouldHide_(session);
            session->isEnableGestureBack_ = true;
            session->UpdateGestureBackEnabled();
            session->isEnableGestureBackHadSet_ = false;
        }
        return WSError::WS_OK;
    },
        "Disconnect");
    return WSError::WS_OK;
}

WSError SceneSession::UpdateActiveStatus(bool isActive)
{
    auto task = [weakThis = wptr(this), isActive]() {
        auto session = weakThis.promote();
        if (!session) {
            WLOGFE("[WMSCom] session is null");
            return WSError::WS_ERROR_DESTROYED_OBJECT;
        }
        if (!session->IsSessionValid()) {
            TLOGW(WmsLogTag::WMS_MAIN, "Session is invalid, id: %{public}d state: %{public}u",
                session->GetPersistentId(), session->GetSessionState());
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
    };
    PostTask(task, "UpdateActiveStatus:" + std::to_string(isActive));
    return WSError::WS_OK;
}

WSError SceneSession::OnSessionEvent(SessionEvent event)
{
    auto task = [weakThis = wptr(this), event]() {
        auto session = weakThis.promote();
        if (!session) {
            WLOGFE("[WMSCom] session is null");
            return WSError::WS_ERROR_DESTROYED_OBJECT;
        }
        WLOGFI("[WMSCom] SceneSession OnSessionEvent event: %{public}d", static_cast<int32_t>(event));
        if (event == SessionEvent::EVENT_START_MOVE) {
            if (!(session->moveDragController_ && !session->moveDragController_->GetStartDragFlag() &&
                session->IsFocused() && session->IsMovableWindowType() &&
                session->moveDragController_->HasPointDown())) {
                TLOGW(WmsLogTag::WMS_LAYOUT, "Window is not movable, id: %{public}d", session->GetPersistentId());
                return WSError::WS_OK;
            }
            HITRACE_METER_FMT(HITRACE_TAG_WINDOW_MANAGER, "SceneSession::StartMove");
            session->moveDragController_->InitMoveDragProperty();
            if (session->IsFullScreenMovable()) {
                WSRect rect = session->moveDragController_->GetFullScreenToFloatingRect(session->winRect_,
                    session->GetSessionRequestRect());
                session->Session::UpdateRect(rect, SizeChangeReason::RECOVER, "OnSessionEvent", nullptr);
                session->moveDragController_->SetStartMoveFlag(true);
                session->moveDragController_->CalcFirstMoveTargetRect(rect, true);
            } else {
                session->moveDragController_->SetStartMoveFlag(true);
                session->moveDragController_->CalcFirstMoveTargetRect(session->winRect_, false);
            }
            session->SetSessionEventParam({session->moveDragController_->GetOriginalPointerPosX(),
                session->moveDragController_->GetOriginalPointerPosY()});
        }
        if (session->moveDragController_ && event == SessionEvent::EVENT_DRAG) {
            WSRect rect = session->moveDragController_->GetTargetRect();
            session->SetSessionEventParam({rect.posX_, rect.posY_, rect.width_, rect.height_});
        }
        if (session->sessionChangeCallback_ && session->sessionChangeCallback_->OnSessionEvent_) {
            session->sessionChangeCallback_->OnSessionEvent_(static_cast<uint32_t>(event),
                session->sessionEventParam_);
        }
        return WSError::WS_OK;
    };
    PostTask(task, "OnSessionEvent:" + std::to_string(static_cast<int>(event)));
    return WSError::WS_OK;
}

uint32_t SceneSession::GetWindowDragHotAreaType(uint32_t type, int32_t pointerX, int32_t pointerY)
{
    std::shared_lock<std::shared_mutex> lock(windowDragHotAreaMutex_);
    for (auto it = windowDragHotAreaMap_.begin(); it != windowDragHotAreaMap_.end(); ++it) {
        uint32_t key = it->first;
        WSRect rect = it->second;
        if (rect.IsInRegion(pointerX, pointerY)) {
            type |= key;
        }
    }
    return type;
}

void SceneSession::AddOrUpdateWindowDragHotArea(uint32_t type, const WSRect& area)
{
    std::unique_lock<std::shared_mutex> lock(windowDragHotAreaMutex_);
    auto const result = windowDragHotAreaMap_.insert({type, area});
    if (!result.second) {
        result.first->second = area;
    }
}

SubWindowModalType SceneSession::GetSubWindowModalType() const
{
    SubWindowModalType modalType = SubWindowModalType::TYPE_UNDEFINED;
    auto property = GetSessionProperty();
    if (property == nullptr) {
        TLOGE(WmsLogTag::DEFAULT, "property is nullptr");
        return modalType;
    }
    auto windowType = property->GetWindowType();
    if (WindowHelper::IsToastSubWindow(windowType, property->GetWindowFlags())) {
        return SubWindowModalType::TYPE_TOAST;
    }
    if (WindowHelper::IsDialogWindow(windowType)) {
        modalType = SubWindowModalType::TYPE_DIALOG;
    } else if (WindowHelper::IsModalSubWindow(windowType, property->GetWindowFlags())) {
        modalType = SubWindowModalType::TYPE_WINDOW_MODALITY;
    } else if (WindowHelper::IsSubWindow(windowType)) {
        modalType = SubWindowModalType::TYPE_NORMAL;
    }
    return modalType;
}

void SceneSession::SetSessionEventParam(SessionEventParam param)
{
    sessionEventParam_ = param;
}

void SceneSession::RegisterSessionChangeCallback(const sptr<SceneSession::SessionChangeCallback>&
    sessionChangeCallback)
{
    std::lock_guard<std::mutex> guard(sessionChangeCbMutex_);
    sessionChangeCallback_ = sessionChangeCallback;
}

void SceneSession::RegisterDefaultAnimationFlagChangeCallback(NotifyWindowAnimationFlagChangeFunc&& callback)
{
    auto task = [weakThis = wptr(this), callback = std::move(callback)] {
        auto session = weakThis.promote();
        if (!session) {
            TLOGNE(WmsLogTag::WMS_LIFE, "session is null");
            return;
        }
        session->onWindowAnimationFlagChange_ = std::move(callback);
        session->onWindowAnimationFlagChange_(session->IsNeedDefaultAnimation());
    };
    PostTask(task, __func__);
}

void SceneSession::RegisterDefaultDensityEnabledCallback(NotifyDefaultDensityEnabledFunc&& callback)
{
    auto task = [weakThis = wptr(this), callback = std::move(callback)] {
        auto session = weakThis.promote();
        if (!session) {
            TLOGNE(WmsLogTag::WMS_LIFE, "session is null");
            return;
        }
        session->onDefaultDensityEnabledFunc_ = std::move(callback);
    };
    PostTask(task, __func__);
}

void SceneSession::RegisterNeedAvoidCallback(NotifyNeedAvoidFunc&& callback)
{
    auto task = [weakThis = wptr(this), callback = std::move(callback)] {
        auto session = weakThis.promote();
        if (!session) {
            TLOGNE(WmsLogTag::WMS_IMMS, "session is null");
            return;
        }
        session->onNeedAvoid_ = std::move(callback);
    };
    PostTask(task, __func__);
}

WSError SceneSession::SetGlobalMaximizeMode(MaximizeMode mode)
{
    auto task = [weakThis = wptr(this), mode]() {
        auto session = weakThis.promote();
        if (!session) {
            WLOGFE("[WMSCom] session is null");
            return WSError::WS_ERROR_DESTROYED_OBJECT;
        }
        WLOGFD("[WMSCom] mode: %{public}u", static_cast<uint32_t>(mode));
        session->maximizeMode_ = mode;
        ScenePersistentStorage::Insert("maximize_state", static_cast<int32_t>(session->maximizeMode_),
            ScenePersistentStorageType::MAXIMIZE_STATE);
        return WSError::WS_OK;
    };
    return PostSyncTask(task, "SetGlobalMaximizeMode");
}

WSError SceneSession::GetGlobalMaximizeMode(MaximizeMode& mode)
{
    auto task = [weakThis = wptr(this), &mode]() {
        auto session = weakThis.promote();
        if (!session) {
            WLOGFE("[WMSCom] session is null");
            return WSError::WS_ERROR_DESTROYED_OBJECT;
        }
        mode = maximizeMode_;
        WLOGFD("[WMSCom] mode: %{public}u", static_cast<uint32_t>(mode));
        return WSError::WS_OK;
    };
    return PostSyncTask(task, "GetGlobalMaximizeMode");
}

static WSError CheckAspectRatioValid(const sptr<SceneSession>& session, float ratio, float vpr)
{
    if (MathHelper::NearZero(ratio)) {
        return WSError::WS_OK;
    }
    if (!session) {
        return WSError::WS_ERROR_INVALID_PARAM;
    }
    auto sessionProperty = session->GetSessionProperty();
    if (!sessionProperty) {
        return WSError::WS_ERROR_INVALID_PARAM;
    }
    auto limits = sessionProperty->GetWindowLimits();
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
    return WSError::WS_OK;
}

WSError SceneSession::SetAspectRatio(float ratio)
{
    auto task = [weakThis = wptr(this), ratio]() {
        auto session = weakThis.promote();
        if (!session) {
            TLOGE(WmsLogTag::WMS_LAYOUT, "session is null");
            return WSError::WS_ERROR_DESTROYED_OBJECT;
        }
        if (!session->GetSessionProperty()) {
            TLOGE(WmsLogTag::WMS_LAYOUT, "SetAspectRatio failed because property is null");
            return WSError::WS_ERROR_NULLPTR;
        }
        float vpr = 1.5f; // 1.5f: default virtual pixel ratio
        auto display = DisplayManager::GetInstance().GetDefaultDisplay();
        if (display) {
            vpr = display->GetVirtualPixelRatio();
            WLOGD("vpr = %{public}f", vpr);
        }
        WSError ret = CheckAspectRatioValid(session, ratio, vpr);
        if (ret != WSError::WS_OK) {
            return ret;
        }
        session->aspectRatio_ = ratio;
        if (session->moveDragController_) {
            session->moveDragController_->SetAspectRatio(ratio);
        }
        session->SaveAspectRatio(session->aspectRatio_);
        WSRect fixedRect = session->winRect_;
        TLOGI(WmsLogTag::WMS_LAYOUT, "Before fixing, the id:%{public}d, the current rect: %{public}s, "
            "ratio: %{public}f", session->GetPersistentId(), fixedRect.ToString().c_str(), ratio);
        if (session->FixRectByAspectRatio(fixedRect)) {
            TLOGI(WmsLogTag::WMS_LAYOUT, "After fixing, the id:%{public}d, the fixed rect: %{public}s",
                session->GetPersistentId(), fixedRect.ToString().c_str());
            session->NotifySessionRectChange(fixedRect, SizeChangeReason::RESIZE);
        }
        return WSError::WS_OK;
    };
    return PostSyncTask(task, "SetAspectRatio");
}

WSError SceneSession::UpdateRect(const WSRect& rect, SizeChangeReason reason,
    const std::string& updateReason, const std::shared_ptr<RSTransaction>& rsTransaction)
{
    const char* const funcName = __func__;
    auto task = [weakThis = wptr(this), rect, reason, rsTransaction, updateReason, funcName]() {
        auto session = weakThis.promote();
        if (!session) {
            TLOGNE(WmsLogTag::WMS_LAYOUT, "%{public}s: session is null", funcName);
            return WSError::WS_ERROR_DESTROYED_OBJECT;
        }
        if (session->winRect_ == rect && session->reason_ != SizeChangeReason::DRAG_END &&
            (session->GetWindowType() != WindowType::WINDOW_TYPE_KEYBOARD_PANEL &&
             session->GetWindowType() != WindowType::WINDOW_TYPE_INPUT_METHOD_FLOAT)) {
            if (!session->sessionStage_) {
                TLOGND(WmsLogTag::WMS_LAYOUT, "%{public}s: skip same rect update id:%{public}d rect:%{public}s",
                    funcName, session->GetPersistentId(), rect.ToString().c_str());
                return WSError::WS_OK;
            } else if (session->GetClientRect() == rect) {
                TLOGND(WmsLogTag::WMS_LAYOUT, "%{public}s: skip same rect update id:%{public}d rect:%{public}s "
                    "clientRect:%{public}s", funcName, session->GetPersistentId(), rect.ToString().c_str(),
                    session->GetClientRect().ToString().c_str());
                return WSError::WS_OK;
            }
        }
        if (rect.IsInvalid()) {
            TLOGNE(WmsLogTag::WMS_LAYOUT, "%{public}s: id:%{public}d rect:%{public}s is invalid",
                funcName, session->GetPersistentId(), rect.ToString().c_str());
            return WSError::WS_ERROR_INVALID_PARAM;
        }
        HITRACE_METER_FMT(HITRACE_TAG_WINDOW_MANAGER,
            "SceneSession::UpdateRect%d [%d, %d, %u, %u]",
            session->GetPersistentId(), rect.posX_, rect.posY_, rect.width_, rect.height_);
        // position change no need to notify client, since frame layout finish will notify
        if (NearEqual(rect.width_, session->winRect_.width_) && NearEqual(rect.height_, session->winRect_.height_) &&
            (session->reason_ != SizeChangeReason::MOVE || !session->rectChangeListenerRegistered_)) {
            TLOGNI(WmsLogTag::WMS_LAYOUT, "%{public}s: position change no need notify client id:%{public}d, "
                "rect:%{public}s, preRect: %{public}s", funcName,
                session->GetPersistentId(), rect.ToString().c_str(), session->winRect_.ToString().c_str());
            session->winRect_ = rect;
        } else {
            session->winRect_ = rect;
            session->NotifyClientToUpdateRect(updateReason, rsTransaction);
        }
        session->dirtyFlags_ |= static_cast<uint32_t>(SessionUIDirtyFlag::RECT);
        TLOGNI(WmsLogTag::WMS_LAYOUT, "%{public}s: id:%{public}d, reason:%{public}d %{public}s, "
            "rect:%{public}s, clientRect:%{public}s",
            funcName, session->GetPersistentId(), session->reason_, updateReason.c_str(),
            rect.ToString().c_str(), session->GetClientRect().ToString().c_str());

        return WSError::WS_OK;
    };
    PostTask(task, "UpdateRect" + GetRectInfo(rect));
    return WSError::WS_OK;
}

bool SceneSession::IsKeyboardNeedLeftOffset(bool isPhone, const sptr<WindowSessionProperty>& sessionProperty)
{
    static bool isFoldable = ScreenSessionManagerClient::GetInstance().IsFoldable();
    bool isFolded = ScreenSessionManagerClient::GetInstance().GetFoldStatus() == OHOS::Rosen::FoldStatus::FOLDED;
    bool isDualDevice = FoldScreenStateInternel::IsDualDisplayFoldDevice();
    uint32_t screenWidth = 0;
    uint32_t screenHeight = 0;
    if (!GetScreenWidthAndHeightFromServer(sessionProperty, screenWidth, screenHeight)) {
        return false;
    }
    bool isLandscape = screenWidth > screenHeight ? true : false;
    bool result = isPhone && (!isFoldable || isFolded || isDualDevice) && isLandscape;
    TLOGI(WmsLogTag::WMS_LAYOUT, "isPhone:%{public}d, isFoldable:%{public}d, isFolded:%{public}d, "
        "isDualDevice:%{public}d, isLandscape:%{public}d, screenWidth:%{public}u, screenHeight:%{public}u, "
        "isKeyboardNeedLeftOffset:%{public}d", isPhone, isFoldable, isFolded, isDualDevice, isLandscape,
        screenWidth, screenHeight, result);
    return result;
}

void SceneSession::FixKeyboardPositionByKeyboardPanel(sptr<SceneSession> panelSession,
    sptr<SceneSession> keyboardSession)
{
    if (panelSession == nullptr || keyboardSession == nullptr) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "keyboard or panel session is null");
        return;
    }

    SessionGravity gravity = keyboardSession->GetKeyboardGravity();
    if (gravity == SessionGravity::SESSION_GRAVITY_FLOAT) {
        keyboardSession->winRect_.posX_ = panelSession->winRect_.posX_;
    } else {
        auto sessionProperty = keyboardSession->GetSessionProperty();
        if (sessionProperty == nullptr) {
            TLOGE(WmsLogTag::WMS_LAYOUT, "keyboard property is null");
            return;
        }
        static bool isPhone = systemConfig_.uiType_ == UI_TYPE_PHONE;
        if (!IsKeyboardNeedLeftOffset(isPhone, sessionProperty) || panelSession->winRect_.posX_ != 0) {
            keyboardSession->winRect_.posX_ = panelSession->winRect_.posX_;
        }
    }
    keyboardSession->winRect_.posY_ = panelSession->winRect_.posY_;
    TLOGI(WmsLogTag::WMS_LAYOUT, "panelId:%{public}d, keyboardId:%{public}d, panelRect:%{public}s, "
        "keyboardRect:%{public}s, gravity:%{public}d", panelSession->GetPersistentId(),
        keyboardSession->GetPersistentId(), panelSession->winRect_.ToString().c_str(),
        keyboardSession->winRect_.ToString().c_str(), gravity);
}

WSError SceneSession::NotifyClientToUpdateRectTask(const std::string& updateReason,
    std::shared_ptr<RSTransaction> rsTransaction)
{
    TLOGD(WmsLogTag::WMS_LAYOUT, "id:%{public}d, reason:%{public}d, rect:%{public}s",
        GetPersistentId(), reason_, winRect_.ToString().c_str());
    bool isMoveOrDrag = moveDragController_ &&
        (moveDragController_->GetStartDragFlag() || moveDragController_->GetStartMoveFlag());
    if (isMoveOrDrag && reason_ == SizeChangeReason::UNDEFINED) {
        TLOGD(WmsLogTag::WMS_LAYOUT, "skip redundant rect update!");
        return WSError::WS_ERROR_REPEAT_OPERATION;
    }
    WSError ret = WSError::WS_OK;
    HITRACE_METER_FMT(HITRACE_TAG_WINDOW_MANAGER,
        "SceneSession::NotifyClientToUpdateRect%d [%d, %d, %u, %u] reason:%u",
        GetPersistentId(), winRect_.posX_, winRect_.posY_, winRect_.width_, winRect_.height_, reason_);

    if (!Session::IsScbCoreEnabled() && isKeyboardPanelEnabled_) {
        sptr<SceneSession> self(this);
        if (GetWindowType() == WindowType::WINDOW_TYPE_KEYBOARD_PANEL) {
            const auto& keyboardSession = GetKeyboardSession();
            FixKeyboardPositionByKeyboardPanel(self, keyboardSession);
            if (keyboardSession != nullptr) {
                ret = keyboardSession->Session::UpdateRect(keyboardSession->winRect_, reason_, updateReason, nullptr);
            }
            return ret;
        }
        if (GetWindowType() == WindowType::WINDOW_TYPE_INPUT_METHOD_FLOAT) {
            FixKeyboardPositionByKeyboardPanel(GetKeyboardPanelSession(), self);
        }
    }

    // once reason is undefined, not use rsTransaction
    // when rotation, sync cnt++ in marshalling. Although reason is undefined caused by resize
    if (reason_ == SizeChangeReason::UNDEFINED || reason_ == SizeChangeReason::MOVE ||
        reason_ == SizeChangeReason::RESIZE) {
        ret = Session::UpdateRect(winRect_, reason_, updateReason, nullptr);
    } else {
        ret = Session::UpdateRect(winRect_, reason_, updateReason, rsTransaction);
#ifdef DEVICE_STATUS_ENABLE
        // When the drag is in progress, the drag window needs to be notified to rotate.
        if (rsTransaction != nullptr) {
            RotateDragWindow(rsTransaction);
        }
#endif // DEVICE_STATUS_ENABLE
    }

    return ret;
}

WSError SceneSession::NotifyClientToUpdateRect(const std::string& updateReason,
    std::shared_ptr<RSTransaction> rsTransaction)
{
    auto task = [weakThis = wptr(this), rsTransaction, updateReason]() {
        auto session = weakThis.promote();
        if (!session) {
            TLOGE(WmsLogTag::WMS_LAYOUT, "session is null");
            return WSError::WS_ERROR_DESTROYED_OBJECT;
        }
        WSError ret = session->NotifyClientToUpdateRectTask(updateReason, rsTransaction);
        if (ret != WSError::WS_OK) {
            return ret;
        }
        if (session->specificCallback_ != nullptr) {
            if (Session::IsScbCoreEnabled()) {
                session->dirtyFlags_ |= static_cast<uint32_t>(SessionUIDirtyFlag::AVOID_AREA);
            } else {
                session->specificCallback_->onUpdateAvoidArea_(session->GetPersistentId());
            }
        }
        return ret;
    };
    PostTask(task, "NotifyClientToUpdateRect");
    return WSError::WS_OK;
}

bool SceneSession::GetScreenWidthAndHeightFromServer(const sptr<WindowSessionProperty>& sessionProperty,
    uint32_t& screenWidth, uint32_t& screenHeight)
{
    if (isScreenAngleMismatch_) {
        screenWidth = targetScreenWidth_;
        screenHeight = targetScreenHeight_;
        TLOGI(WmsLogTag::WMS_KEYBOARD, "screenWidth: %{public}d, screenHeight: %{public}d", screenWidth, screenHeight);
        return true;
    }

    const auto& screenSession = sessionProperty == nullptr ? nullptr :
        ScreenSessionManagerClient::GetInstance().GetScreenSession(sessionProperty->GetDisplayId());
    if (screenSession != nullptr) {
        screenWidth = screenSession->GetScreenProperty().GetBounds().rect_.width_;
        screenHeight = screenSession->GetScreenProperty().GetBounds().rect_.height_;
    } else {
        TLOGI(WmsLogTag::WMS_KEYBOARD, "sessionProperty or screenSession is nullptr, use defaultDisplayInfo");
        auto defaultDisplayInfo = DisplayManager::GetInstance().GetDefaultDisplay();
        if (defaultDisplayInfo != nullptr) {
            screenWidth = static_cast<uint32_t>(defaultDisplayInfo->GetWidth());
            screenHeight = static_cast<uint32_t>(defaultDisplayInfo->GetHeight());
        } else {
            TLOGE(WmsLogTag::WMS_KEYBOARD, "defaultDisplayInfo is null, get screenWidthAndHeight failed");
            return false;
        }
    }
    TLOGI(WmsLogTag::WMS_KEYBOARD, "screenWidth: %{public}d, screenHeight: %{public}d", screenWidth, screenHeight);
    return true;
}

bool SceneSession::GetScreenWidthAndHeightFromClient(const sptr<WindowSessionProperty>& sessionProperty,
    uint32_t& screenWidth, uint32_t& screenHeight)
{
    if (isScreenAngleMismatch_) {
        screenWidth = targetScreenWidth_;
        screenHeight = targetScreenHeight_;
        TLOGI(WmsLogTag::WMS_KEYBOARD, "screenWidth: %{public}d, screenHeight: %{public}d", screenWidth, screenHeight);
        return true;
    }

    auto defaultDisplayInfo = DisplayManager::GetInstance().GetDefaultDisplay();
    if (defaultDisplayInfo != nullptr) {
        screenWidth = static_cast<uint32_t>(defaultDisplayInfo->GetWidth());
        screenHeight = static_cast<uint32_t>(defaultDisplayInfo->GetHeight());
    } else {
        TLOGE(WmsLogTag::WMS_KEYBOARD, "defaultDisplayInfo is null, get screenWidthAndHeight failed");
        return false;
    }
    TLOGI(WmsLogTag::WMS_KEYBOARD, "screenWidth: %{public}d, screenHeight: %{public}d", screenWidth, screenHeight);
    return true;
}

void SceneSession::NotifyTargetScreenWidthAndHeight(bool isScreenAngleMismatch, uint32_t screenWidth,
    uint32_t screenHeight)
{
    auto task = [weakThis = wptr(this), isScreenAngleMismatch, screenWidth, screenHeight]() {
        auto session = weakThis.promote();
        if (!session) {
            TLOGE(WmsLogTag::WMS_KEYBOARD, "keyboard session is null");
            return;
        }
        session->isScreenAngleMismatch_ = isScreenAngleMismatch;
        session->targetScreenWidth_ = screenWidth;
        session->targetScreenHeight_ = screenHeight;
        TLOGI(WmsLogTag::WMS_KEYBOARD, "target isMismatch: %{public}d, width_: %{public}d, height_: %{public}d",
            isScreenAngleMismatch, screenWidth, screenHeight);
        return;
    };
    PostTask(task, "NotifyTargetScreenWidthAndHeight");
}

bool SceneSession::UpdateInputMethodSessionRect(const WSRect& rect, WSRect& newWinRect, WSRect& newRequestRect)
{
    uint32_t screenWidth = 0;
    uint32_t screenHeight = 0;
    auto sessionProperty = GetSessionProperty();
    if (!sessionProperty) {
        TLOGE(WmsLogTag::WMS_KEYBOARD, "sessionProperty is null");
        return false;
    }
    SessionGravity gravity = GetKeyboardGravity();
    if (GetWindowType() == WindowType::WINDOW_TYPE_INPUT_METHOD_FLOAT &&
        (gravity == SessionGravity::SESSION_GRAVITY_BOTTOM || gravity == SessionGravity::SESSION_GRAVITY_DEFAULT)) {
        if (!GetScreenWidthAndHeightFromServer(sessionProperty, screenWidth, screenHeight)) {
            return false;
        }
        newWinRect.width_ = (gravity == SessionGravity::SESSION_GRAVITY_BOTTOM) ?
            static_cast<int32_t>(screenWidth) : rect.width_;
        newRequestRect.width_ = newWinRect.width_;
        newWinRect.height_ = rect.height_;
        newRequestRect.height_ = newWinRect.height_;
        newWinRect.posX_ = (gravity == SessionGravity::SESSION_GRAVITY_BOTTOM) ? 0 : rect.posX_;
        newRequestRect.posX_ = newWinRect.posX_;
        newWinRect.posY_ = static_cast<int32_t>(screenHeight) - newWinRect.height_;
        newRequestRect.posY_ = newWinRect.posY_;
        TLOGI(WmsLogTag::WMS_KEYBOARD, "rect: %{public}s, newRequestRect: %{public}s, newWinRect: %{public}s",
            rect.ToString().c_str(), newRequestRect.ToString().c_str(), newWinRect.ToString().c_str());
        return true;
    }
    TLOGD(WmsLogTag::WMS_KEYBOARD, "There is no need to update input rect");
    return false;
}

void SceneSession::SetSessionRectChangeCallback(const NotifySessionRectChangeFunc& func)
{
    auto task = [weakThis = wptr(this), func]() {
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
    };
    PostTask(task, "SetSessionRectChangeCallback");
}

void SceneSession::SetRestoreMainWindowCallback(NotifyRestoreMainWindowFunc&& func)
{
    const char* const funcName = __func__;
    auto task = [weakThis = wptr(this), func = std::move(func), funcName] {
        auto session = weakThis.promote();
        if (!session || !func) {
            TLOGNE(WmsLogTag::WMS_LIFE, "session or RestoreMainWindowFunc is null");
            return;
        }
        session->onRestoreMainWindowFunc_ = std::move(func);
        TLOGNI(WmsLogTag::WMS_LIFE, "%{public}s id: %{public}d",
            funcName, session->GetPersistentId());
    };
    PostTask(task, funcName);
}

void SceneSession::SetKeyboardGravityChangeCallback(const NotifyKeyboardGravityChangeFunc& func)
{
    auto task = [weakThis = wptr(this), func]() {
        auto session = weakThis.promote();
        if (!session || !func) {
            WLOGFE("session or gravityChangeFunc is null");
            return WSError::WS_ERROR_DESTROYED_OBJECT;
        }
        session->keyboardGravityChangeFunc_ = func;
        session->keyboardGravityChangeFunc_(session->GetKeyboardGravity());
        TLOGI(WmsLogTag::WMS_KEYBOARD, "Notify gravity change when register, id: %{public}d gravity: %{public}d",
            session->GetPersistentId(), session->GetKeyboardGravity());
        return WSError::WS_OK;
    };
    PostTask(task, "SetKeyboardGravityChangeCallback");
}

void SceneSession::SetAdjustKeyboardLayoutCallback(const NotifyKeyboardLayoutAdjustFunc& func)
{
    auto task = [weakThis = wptr(this), func]() {
        auto session = weakThis.promote();
        if (!session || !func) {
            TLOGE(WmsLogTag::WMS_KEYBOARD, "session or keyboardLayoutFunc is null");
            return WSError::WS_ERROR_DESTROYED_OBJECT;
        }
        session->adjustKeyboardLayoutFunc_ = func;
        auto property = session->GetSessionProperty();
        if (property == nullptr) {
            TLOGE(WmsLogTag::WMS_KEYBOARD, "property is null");
            return WSError::WS_ERROR_DESTROYED_OBJECT;
        }
        KeyboardLayoutParams params = property->GetKeyboardLayoutParams();
        session->adjustKeyboardLayoutFunc_(params);
        TLOGI(WmsLogTag::WMS_KEYBOARD, "Notify adjust keyboard layout when register, keyboardId: %{public}d, "
            "gravity: %{public}u, LandscapeKeyboardRect: %{public}s, PortraitKeyboardRect: %{public}s, "
            "LandscapePanelRect: %{public}s, PortraitPanelRect: %{public}s", session->GetPersistentId(),
            static_cast<uint32_t>(params.gravity_), params.LandscapeKeyboardRect_.ToString().c_str(),
            params.PortraitKeyboardRect_.ToString().c_str(), params.LandscapePanelRect_.ToString().c_str(),
            params.PortraitPanelRect_.ToString().c_str());
        return WSError::WS_OK;
    };
    PostTask(task, "SetAdjustKeyboardLayoutCallback");
}

void SceneSession::SetSessionPiPControlStatusChangeCallback(const NotifySessionPiPControlStatusChangeFunc& func)
{
    auto task = [weakThis = wptr(this), func]() {
        auto session = weakThis.promote();
        if (!session) {
            TLOGE(WmsLogTag::WMS_PIP, "session is null");
            return WSError::WS_ERROR_DESTROYED_OBJECT;
        }
        session->sessionPiPControlStatusChangeFunc_ = func;
        return WSError::WS_OK;
    };
    PostTask(task, __func__);
}

void SceneSession::SetAutoStartPiPStatusChangeCallback(const NotifyAutoStartPiPStatusChangeFunc& func)
{
    auto task = [weakThis = wptr(this), func] {
        auto session = weakThis.promote();
        if (!session) {
            TLOGNE(WmsLogTag::WMS_PIP, "session is null");
            return;
        }
        session->autoStartPiPStatusChangeFunc_ = func;
    };
    PostTask(task, __func__);
}

void SceneSession::UpdateSessionRectInner(const WSRect& rect, const SizeChangeReason& reason)
{
    auto newWinRect = winRect_;
    auto newRequestRect = GetSessionRequestRect();
    SizeChangeReason newReason = reason;
    if (reason == SizeChangeReason::MOVE) {
        newWinRect.posX_ = rect.posX_;
        newWinRect.posY_ = rect.posY_;
        newRequestRect.posX_ = rect.posX_;
        newRequestRect.posY_ = rect.posY_;
        if (!Session::IsScbCoreEnabled() && !WindowHelper::IsMainWindow(GetWindowType())) {
            SetSessionRect(newWinRect);
        }
        SetSessionRequestRect(newRequestRect);
        NotifySessionRectChange(newRequestRect, reason);
    } else if (reason == SizeChangeReason::RESIZE) {
        bool needUpdateInputMethod = UpdateInputMethodSessionRect(rect, newWinRect, newRequestRect);
        if (needUpdateInputMethod) {
            newReason = SizeChangeReason::UNDEFINED;
            TLOGD(WmsLogTag::WMS_KEYBOARD, "Input rect has totally changed, need to modify reason, id: %{public}d",
                GetPersistentId());
        } else if (rect.width_ > 0 && rect.height_ > 0) {
            newWinRect.width_ = rect.width_;
            newWinRect.height_ = rect.height_;
            newRequestRect.width_ = rect.width_;
            newRequestRect.height_ = rect.height_;
        }
        if (!Session::IsScbCoreEnabled() && GetWindowType() != WindowType::WINDOW_TYPE_INPUT_METHOD_FLOAT) {
            SetSessionRect(newWinRect);
        }
        SetSessionRequestRect(newRequestRect);
        NotifySessionRectChange(newRequestRect, newReason);
    } else {
        if (!Session::IsScbCoreEnabled()) {
            SetSessionRect(rect);
        }
        NotifySessionRectChange(rect, reason);
    }
    TLOGI(WmsLogTag::WMS_LAYOUT, "Id:%{public}d reason:%{public}d newReason:%{public}d rect:%{public}s "
        "newRequestRect:%{public}s newWinRect:%{public}s", GetPersistentId(), reason,
        newReason, rect.ToString().c_str(), newRequestRect.ToString().c_str(), newWinRect.ToString().c_str());
}

WSError SceneSession::UpdateSessionRect(
    const WSRect &rect, const SizeChangeReason reason, bool isGlobal, bool isFromMoveToGlobal)
{
    if ((reason == SizeChangeReason::MOVE || reason == SizeChangeReason::RESIZE) &&
        GetWindowType() == WindowType::WINDOW_TYPE_PIP) {
        return WSError::WS_DO_NOTHING;
    }
    WSRect newRect = rect;
    if (isGlobal && WindowHelper::IsSubWindow(Session::GetWindowType()) &&
        (systemConfig_.uiType_ == UI_TYPE_PHONE ||
         (systemConfig_.uiType_ == UI_TYPE_PAD && !IsFreeMultiWindowMode()))) {
        auto parentSession = GetParentSession();
        if (parentSession) {
            auto parentRect = parentSession->GetSessionRect();
            if (!CheckIfRectElementIsTooLarge(parentRect)) {
                newRect.posX_ -= parentRect.posX_;
                newRect.posY_ -= parentRect.posY_;
            }
        }
    }
    if (isFromMoveToGlobal && WindowHelper::IsSubWindow(Session::GetWindowType()) &&
        (systemConfig_.uiType_ == UI_TYPE_PHONE ||
         (systemConfig_.uiType_ == UI_TYPE_PAD && !IsFreeMultiWindowMode()))) {
        auto parentSession = GetParentSession();
        if (parentSession && parentSession->GetFloatingScale() != 0) {
            Rect parentGlobalRect;
            WMError errorCode = parentSession->GetGlobalScaledRect(parentGlobalRect);
            newRect.posX_ = (newRect.posX_ - parentGlobalRect.posX_) / parentSession->GetFloatingScale();
            newRect.posY_ = (newRect.posY_ - parentGlobalRect.posY_) / parentSession->GetFloatingScale();
        }
    }
    Session::RectCheckProcess();
    auto task = [weakThis = wptr(this), newRect, reason]() {
        auto session = weakThis.promote();
        if (!session) {
            TLOGE(WmsLogTag::WMS_LAYOUT, "session is null");
            return WSError::WS_ERROR_DESTROYED_OBJECT;
        }
        session->UpdateSessionRectInner(newRect, reason);
        return WSError::WS_OK;
    };
    PostTask(task, "UpdateSessionRect" + GetRectInfo(rect));
    return WSError::WS_OK;
}

/** @note @window.layout */
WSError SceneSession::UpdateClientRect(const WSRect& rect)
{
    const char* const funcName = __func__;
    auto task = [weakThis = wptr(this), rect, funcName] {
        auto session = weakThis.promote();
        if (!session) {
            TLOGNE(WmsLogTag::WMS_LAYOUT, "%{public}s: session is null", funcName);
            return;
        }
        if (rect.IsInvalid()) {
            TLOGNE(WmsLogTag::WMS_LAYOUT, "%{public}s: id:%{public}d rect:%{public}s is invalid",
                funcName, session->GetPersistentId(), rect.ToString().c_str());
            return;
        }
        if (rect == session->GetClientRect()) {
            TLOGND(WmsLogTag::WMS_LAYOUT, "%{public}s: id:%{public}d skip same rect",
                funcName, session->GetPersistentId());
            return;
        }
        session->SetClientRect(rect);
    };
    PostTask(task, "UpdateClientRect" + GetRectInfo(rect));
    return WSError::WS_OK;
}

/** @note @window.hierarchy */
WSError SceneSession::RaiseToAppTop()
{
    if (!SessionPermission::IsSystemCalling()) {
        WLOGFE("raise to app top permission denied!");
        return WSError::WS_ERROR_NOT_SYSTEM_APP;
    }
    auto task = [weakThis = wptr(this)]() {
        auto session = weakThis.promote();
        if (!session) {
            WLOGFE("session is null");
            return WSError::WS_ERROR_DESTROYED_OBJECT;
        }
        if (session->sessionChangeCallback_ && session->sessionChangeCallback_->onRaiseToTop_) {
            TLOGI(WmsLogTag::WMS_SUB, "id: %{public}d", session->GetPersistentId());
            session->sessionChangeCallback_->onRaiseToTop_();
            session->SetMainSessionUIStateDirty(true);
        }
        return WSError::WS_OK;
    };
    return PostSyncTask(task, "RaiseToAppTop");
}

/** @note @window.hierarchy */
WSError SceneSession::RaiseAboveTarget(int32_t subWindowId)
{
    if (!SessionPermission::IsSystemCalling() && !SessionPermission::IsStartByHdcd()) {
        WLOGFE("RaiseAboveTarget permission denied!");
        return WSError::WS_ERROR_NOT_SYSTEM_APP;
    }
    auto subSession = std::find_if(subSession_.begin(), subSession_.end(), [subWindowId](sptr<SceneSession> session) {
        bool res = (session != nullptr && session->GetWindowId() == subWindowId) ? true : false;
        return res;
    });
    int32_t callingPid = IPCSkeleton::GetCallingPid();
    if (subSession != subSession_.end() && callingPid != (*subSession)->GetCallingPid()) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "permission denied! id: %{public}d", subWindowId);
        return WSError::WS_ERROR_INVALID_CALLING;
    }
    auto task = [weakThis = wptr(this), subWindowId]() {
        auto session = weakThis.promote();
        if (!session) {
            WLOGFE("session is null");
            return WSError::WS_ERROR_DESTROYED_OBJECT;
        }
        if (session->sessionChangeCallback_ && session->sessionChangeCallback_->onRaiseAboveTarget_) {
            session->sessionChangeCallback_->onRaiseAboveTarget_(subWindowId);
        }
        return WSError::WS_OK;
    };
    return PostSyncTask(task, "RaiseAboveTarget");
}

WSError SceneSession::BindDialogSessionTarget(const sptr<SceneSession>& sceneSession)
{
    if (sceneSession == nullptr) {
        TLOGE(WmsLogTag::WMS_DIALOG, "dialog session is null");
        return WSError::WS_ERROR_NULLPTR;
    }
    if (sessionChangeCallback_ != nullptr && sessionChangeCallback_->onBindDialogTarget_) {
        TLOGI(WmsLogTag::WMS_DIALOG, "id: %{public}d", sceneSession->GetPersistentId());
        sessionChangeCallback_->onBindDialogTarget_(sceneSession);
    }
    return WSError::WS_OK;
}

WSError SceneSession::SetSystemBarProperty(WindowType type, SystemBarProperty systemBarProperty)
{
    TLOGD(WmsLogTag::WMS_IMMS, "persistentId():%{public}u type:%{public}u"
        "enable:%{public}u bgColor:%{public}x Color:%{public}x enableAnimation:%{public}u settingFlag:%{public}u",
        GetPersistentId(), static_cast<uint32_t>(type),
        systemBarProperty.enable_, systemBarProperty.backgroundColor_, systemBarProperty.contentColor_,
        systemBarProperty.enableAnimation_, systemBarProperty.settingFlag_);
    auto property = GetSessionProperty();
    if (property == nullptr) {
        TLOGE(WmsLogTag::WMS_DIALOG, "property is null");
        return WSError::WS_ERROR_NULLPTR;
    }
    property->SetSystemBarProperty(type, systemBarProperty);
    if (sessionChangeCallback_ != nullptr && sessionChangeCallback_->OnSystemBarPropertyChange_) {
        sessionChangeCallback_->OnSystemBarPropertyChange_(property->GetSystemBarProperty());
    }
    return WSError::WS_OK;
}

void SceneSession::SetIsStatusBarVisible(bool isVisible)
{
    auto task = [weakThis = wptr(this), isVisible] {
        sptr<SceneSession> sceneSession = weakThis.promote();
        if (sceneSession == nullptr) {
            TLOGNE(WmsLogTag::WMS_IMMS, "session is null");
            return;
        }
        sceneSession->SetIsStatusBarVisibleInner(isVisible);
    };
    PostTask(task, __func__);
}

WSError SceneSession::SetIsStatusBarVisibleInner(bool isVisible)
{
    bool isNeedNotify = isStatusBarVisible_ != isVisible;
    TLOGI(WmsLogTag::WMS_IMMS, "Window [%{public}d, %{public}s] status bar visible %{public}u, "
        "need notify %{public}u", GetPersistentId(), GetWindowName().c_str(), isVisible, isNeedNotify);
    isStatusBarVisible_ = isVisible;
    if (!isNeedNotify) {
        return WSError::WS_OK;
    }
    if (isLastFrameLayoutFinishedFunc_ == nullptr) {
        TLOGE(WmsLogTag::WMS_IMMS, "isLastFrameLayoutFinishedFunc is null, id: %{public}d", GetPersistentId());
        return WSError::WS_ERROR_NULLPTR;
    }
    bool isLayoutFinished = false;
    WSError ret = isLastFrameLayoutFinishedFunc_(isLayoutFinished);
    if (ret != WSError::WS_OK) {
        TLOGE(WmsLogTag::WMS_IMMS, "isLastFrameLayoutFinishedFunc failed: %{public}d", ret);
        return ret;
    }
    if (isLayoutFinished) {
        if (specificCallback_ && specificCallback_->onUpdateAvoidAreaByType_) {
            specificCallback_->onUpdateAvoidAreaByType_(GetPersistentId(), AvoidAreaType::TYPE_SYSTEM);
        }
    } else {
        dirtyFlags_ |= static_cast<uint32_t>(SessionUIDirtyFlag::AVOID_AREA);
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
    OnShowWhenLocked(GetShowWhenLockedFlagValue());
}

/** @note @window.hierarchy */
WSError SceneSession::RaiseAppMainWindowToTop()
{
    auto task = [weakThis = wptr(this)]() {
        auto session = weakThis.promote();
        if (!session) {
            WLOGFE("session is null");
            return WSError::WS_ERROR_DESTROYED_OBJECT;
        }
        if (session->IsFocusedOnShow()) {
            FocusChangeReason reason = FocusChangeReason::MOVE_UP;
            session->NotifyRequestFocusStatusNotifyManager(true, true, reason);
            session->NotifyClick();
        } else {
            session->SetFocusedOnShow(true);
        }
        return WSError::WS_OK;
    };
    PostTask(task, "RaiseAppMainWindowToTop");
    return WSError::WS_OK;
}

WSError SceneSession::OnNeedAvoid(bool status)
{
    auto task = [weakThis = wptr(this), status]() {
        auto session = weakThis.promote();
        if (!session) {
            TLOGE(WmsLogTag::WMS_IMMS, "session is null");
            return WSError::WS_ERROR_DESTROYED_OBJECT;
        }
        TLOGI(WmsLogTag::WMS_IMMS, "SceneSession OnNeedAvoid status:%{public}d, id:%{public}d",
            static_cast<int32_t>(status), session->GetPersistentId());
        if (session->onNeedAvoid_) {
            session->onNeedAvoid_(status);
        }
        return WSError::WS_OK;
    };
    PostTask(task, "OnNeedAvoid");
    return WSError::WS_OK;
}

WSError SceneSession::OnShowWhenLocked(bool showWhenLocked)
{
    WLOGFD("SceneSession ShowWhenLocked status:%{public}d", static_cast<int32_t>(showWhenLocked));
    if (onShowWhenLockedFunc_) {
        onShowWhenLockedFunc_(showWhenLocked);
    }
    return WSError::WS_OK;
}

bool SceneSession::IsShowWhenLocked() const
{
    return (GetSessionProperty()->GetWindowFlags() &
        static_cast<uint32_t>(WindowFlag::WINDOW_FLAG_SHOW_WHEN_LOCKED)) || IsTemporarilyShowWhenLocked();
}

bool SceneSession::GetShowWhenLockedFlagValue() const
{
    return GetSessionProperty()->GetWindowFlags() & static_cast<uint32_t>(WindowFlag::WINDOW_FLAG_SHOW_WHEN_LOCKED);
}

void SceneSession::CalculateAvoidAreaRect(WSRect& rect, WSRect& avoidRect, AvoidArea& avoidArea) const
{
    if (SessionHelper::IsEmptyRect(rect) || SessionHelper::IsEmptyRect(avoidRect)) {
        return;
    }
    Rect avoidAreaRect = SessionHelper::TransferToRect(
        SessionHelper::GetOverlap(rect, avoidRect, rect.posX_, rect.posY_));
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
    auto sessionProperty = GetSessionProperty();
    if (sessionProperty == nullptr ||
        (sessionProperty->GetWindowFlags() & static_cast<uint32_t>(WindowFlag::WINDOW_FLAG_NEED_AVOID))) {
        return;
    }
    uint64_t displayId = sessionProperty->GetDisplayId();
    auto screenSession = ScreenSessionManagerClient::GetInstance().GetScreenSession(displayId);
    if ((Session::GetWindowMode() == WindowMode::WINDOW_MODE_FLOATING ||
         Session::GetWindowMode() == WindowMode::WINDOW_MODE_SPLIT_PRIMARY ||
         Session::GetWindowMode() == WindowMode::WINDOW_MODE_SPLIT_SECONDARY) &&
        WindowHelper::IsMainWindow(Session::GetWindowType()) &&
        (systemConfig_.uiType_ == UI_TYPE_PHONE ||
         (systemConfig_.uiType_ == UI_TYPE_PAD && !IsFreeMultiWindowMode())) &&
        (!screenSession || screenSession->GetName() != "HiCar")) {
        float miniScale = 0.316f; // Pressed mini floating Scale with 0.001 precision
        if (Session::GetFloatingScale() <= miniScale) {
            return;
        }
        float vpr = 3.5f; // 3.5f: default pixel ratio
        auto display = DisplayManager::GetInstance().GetDefaultDisplay();
        if (display == nullptr) {
            WLOGFE("display is null");
            return;
        }
        vpr = display->GetVirtualPixelRatio();
        int32_t floatingBarHeight = 32; // 32: floating windowBar Height
        avoidArea.topRect_.height_ = vpr * floatingBarHeight;
        avoidArea.topRect_.width_ = static_cast<uint32_t>(display->GetWidth());
        return;
    }
    if (!isStatusBarVisible_) {
        TLOGI(WmsLogTag::WMS_IMMS, "status bar not visible");
        return;
    }
    std::vector<sptr<SceneSession>> statusBarVector;
    if (specificCallback_ != nullptr && specificCallback_->onGetSceneSessionVectorByType_) {
        statusBarVector = specificCallback_->onGetSceneSessionVectorByType_(
            WindowType::WINDOW_TYPE_STATUS_BAR, sessionProperty->GetDisplayId());
    }
    for (auto& statusBar : statusBarVector) {
        WSRect statusBarRect = statusBar->GetSessionRect();
        TLOGI(WmsLogTag::WMS_IMMS, "window %{public}s status bar %{public}s",
              rect.ToString().c_str(), statusBarRect.ToString().c_str());
        CalculateAvoidAreaRect(rect, statusBarRect, avoidArea);
    }
    return;
}

void SceneSession::GetKeyboardAvoidArea(WSRect& rect, AvoidArea& avoidArea)
{
    if (Session::CheckEmptyKeyboardAvoidAreaIfNeeded()) {
        TLOGI(WmsLogTag::WMS_IMMS, "Keyboard avoid area needs to be empty when in floating mode");
        return;
    }
    auto sessionProperty = GetSessionProperty();
    if (!sessionProperty) {
        TLOGE(WmsLogTag::WMS_IMMS, "Failed to get session property");
        return;
    }
    std::vector<sptr<SceneSession>> inputMethodVector;
    if (specificCallback_ != nullptr && specificCallback_->onGetSceneSessionVectorByType_) {
        inputMethodVector = specificCallback_->onGetSceneSessionVectorByType_(
            WindowType::WINDOW_TYPE_INPUT_METHOD_FLOAT, sessionProperty->GetDisplayId());
    }
    for (auto& inputMethod : inputMethodVector) {
        if (inputMethod->GetSessionState() != SessionState::STATE_FOREGROUND &&
            inputMethod->GetSessionState() != SessionState::STATE_ACTIVE) {
            continue;
        }
        SessionGravity gravity = inputMethod->GetKeyboardGravity();
        if (gravity == SessionGravity::SESSION_GRAVITY_FLOAT) {
            continue;
        }
        if (isKeyboardPanelEnabled_) {
            WSRect keyboardRect = {0, 0, 0, 0};
            if (inputMethod && inputMethod->GetKeyboardPanelSession()) {
                keyboardRect = inputMethod->GetKeyboardPanelSession()->GetSessionRect();
            }
            TLOGI(WmsLogTag::WMS_IMMS, "window %{public}s keyboard %{public}s",
                  rect.ToString().c_str(), keyboardRect.ToString().c_str());
            CalculateAvoidAreaRect(rect, keyboardRect, avoidArea);
        } else {
            WSRect inputMethodRect = inputMethod->GetSessionRect();
            TLOGI(WmsLogTag::WMS_IMMS, "window %{public}s input method %{public}s",
                  rect.ToString().c_str(), inputMethodRect.ToString().c_str());
            CalculateAvoidAreaRect(rect, inputMethodRect, avoidArea);
        }
    }
    return;
}

void SceneSession::GetCutoutAvoidArea(WSRect& rect, AvoidArea& avoidArea)
{
    auto display = DisplayManager::GetInstance().GetDisplayById(GetSessionProperty()->GetDisplayId());
    if (display == nullptr) {
        TLOGE(WmsLogTag::WMS_IMMS, "Failed to get display");
        return;
    }
    sptr<CutoutInfo> cutoutInfo = display->GetCutoutInfo();
    if (cutoutInfo == nullptr) {
        TLOGI(WmsLogTag::WMS_IMMS, "There is no CutoutInfo");
        return;
    }
    std::vector<DMRect> cutoutAreas = cutoutInfo->GetBoundingRects();
    if (cutoutAreas.empty()) {
        TLOGI(WmsLogTag::WMS_IMMS, "There is no cutoutAreas");
        return;
    }
    for (auto& cutoutArea : cutoutAreas) {
        WSRect cutoutAreaRect = {
            cutoutArea.posX_,
            cutoutArea.posY_,
            cutoutArea.width_,
            cutoutArea.height_
        };
        TLOGI(WmsLogTag::WMS_IMMS, "window %{public}s cutout %{public}s",
              rect.ToString().c_str(), cutoutAreaRect.ToString().c_str());
        CalculateAvoidAreaRect(rect, cutoutAreaRect, avoidArea);
    }

    return;
}

void SceneSession::GetAINavigationBarArea(WSRect rect, AvoidArea& avoidArea) const
{
    if (isDisplayStatusBarTemporarily_.load()) {
        TLOGI(WmsLogTag::WMS_IMMS, "temporary show navigation bar, no need to avoid");
        return;
    }
    if (Session::GetWindowMode() == WindowMode::WINDOW_MODE_PIP) {
        TLOGI(WmsLogTag::WMS_IMMS, "window mode pip return");
        return;
    }
    auto sessionProperty = GetSessionProperty();
    if (!sessionProperty) {
        TLOGE(WmsLogTag::WMS_IMMS, "Failed to get session property");
        return;
    }
    WSRect barArea;
    if (specificCallback_ != nullptr && specificCallback_->onGetAINavigationBarArea_) {
        barArea = specificCallback_->onGetAINavigationBarArea_(sessionProperty->GetDisplayId());
    }
    TLOGI(WmsLogTag::WMS_IMMS, "window %{public}s AI bar %{public}s",
          rect.ToString().c_str(), barArea.ToString().c_str());
    CalculateAvoidAreaRect(rect, barArea, avoidArea);
}

bool SceneSession::CheckGetAvoidAreaAvailable(AvoidAreaType type)
{
    if (type == AvoidAreaType::TYPE_KEYBOARD) {
        return true;
    }
    WindowMode mode = GetWindowMode();
    WindowType winType = GetWindowType();
    std::string uiType = systemConfig_.uiType_;
    if (WindowHelper::IsMainWindow(winType)) {
        if (mode == WindowMode::WINDOW_MODE_FLOATING && type != AvoidAreaType::TYPE_SYSTEM) {
            return false;
        }

        if (mode != WindowMode::WINDOW_MODE_FLOATING ||
            uiType == UI_TYPE_PHONE || uiType == UI_TYPE_PAD) {
            return true;
        }
    }
    if (WindowHelper::IsSubWindow(winType)) {
        auto parentSession = GetParentSession();
        if (parentSession != nullptr && parentSession->GetSessionRect() == GetSessionRect()) {
            return parentSession->CheckGetAvoidAreaAvailable(type);
        }
    }
    TLOGI(WmsLogTag::WMS_IMMS, "Window [%{public}u, %{public}s] type %{public}u "
        "avoidAreaType %{public}u windowMode %{public}u, return default avoid area.",
        GetPersistentId(), GetWindowName().c_str(), static_cast<uint32_t>(winType),
        static_cast<uint32_t>(type), static_cast<uint32_t>(mode));
    return false;
}

void SceneSession::AddModalUIExtension(const ExtensionWindowEventInfo& extensionInfo)
{
    TLOGD(WmsLogTag::WMS_UIEXT, "parentId=%{public}d, persistentId=%{public}d, pid=%{public}d", GetPersistentId(),
        extensionInfo.persistentId, extensionInfo.pid);
    {
        std::unique_lock<std::shared_mutex> lock(modalUIExtensionInfoListMutex_);
        modalUIExtensionInfoList_.push_back(extensionInfo);
    }
    NotifySessionInfoChange();
}

void SceneSession::UpdateModalUIExtension(const ExtensionWindowEventInfo& extensionInfo)
{
    TLOGD(WmsLogTag::WMS_UIEXT, "persistentId=%{public}d,pid=%{public}d,"
        "Rect:[%{public}d %{public}d %{public}d %{public}d]",
        extensionInfo.persistentId, extensionInfo.pid, extensionInfo.windowRect.posX_,
        extensionInfo.windowRect.posY_, extensionInfo.windowRect.width_, extensionInfo.windowRect.height_);
    {
        std::unique_lock<std::shared_mutex> lock(modalUIExtensionInfoListMutex_);
        auto iter = std::find_if(modalUIExtensionInfoList_.begin(), modalUIExtensionInfoList_.end(),
            [extensionInfo](const ExtensionWindowEventInfo& eventInfo) {
            return extensionInfo.persistentId == eventInfo.persistentId && extensionInfo.pid == eventInfo.pid;
        });
        if (iter == modalUIExtensionInfoList_.end()) {
            return;
        }
        iter->windowRect = extensionInfo.windowRect;
    }
    NotifySessionInfoChange();
}

void SceneSession::RemoveModalUIExtension(int32_t persistentId)
{
    TLOGI(WmsLogTag::WMS_UIEXT, "parentId=%{public}d, persistentId=%{public}d", GetPersistentId(), persistentId);
    {
        std::unique_lock<std::shared_mutex> lock(modalUIExtensionInfoListMutex_);
        auto iter = std::find_if(modalUIExtensionInfoList_.begin(), modalUIExtensionInfoList_.end(),
            [persistentId](const ExtensionWindowEventInfo& extensionInfo) {
            return extensionInfo.persistentId == persistentId;
        });
        if (iter == modalUIExtensionInfoList_.end()) {
            return;
        }
        modalUIExtensionInfoList_.erase(iter);
    }
    NotifySessionInfoChange();
}

bool SceneSession::HasModalUIExtension()
{
    std::shared_lock<std::shared_mutex> lock(modalUIExtensionInfoListMutex_);
    return !modalUIExtensionInfoList_.empty();
}

ExtensionWindowEventInfo SceneSession::GetLastModalUIExtensionEventInfo()
{
    std::shared_lock<std::shared_mutex> lock(modalUIExtensionInfoListMutex_);
    return modalUIExtensionInfoList_.back();
}

Vector2f SceneSession::GetSessionGlobalPosition(bool useUIExtension)
{
    WSRect windowRect = GetSessionGlobalRect();
    if (useUIExtension && HasModalUIExtension()) {
        auto rect = GetLastModalUIExtensionEventInfo().windowRect;
        windowRect.posX_ = rect.posX_;
        windowRect.posY_ = rect.posY_;
    }
    Vector2f position(windowRect.posX_, windowRect.posY_);
    return position;
}

void SceneSession::AddUIExtSurfaceNodeId(uint64_t surfaceNodeId, int32_t persistentId)
{
    std::unique_lock<std::shared_mutex> lock(uiExtNodeIdToPersistentIdMapMutex_);
    TLOGI(WmsLogTag::WMS_UIEXT, "Add uiExtension pair surfaceNodeId=%{public}" PRIu64 ", persistentId=%{public}d",
        surfaceNodeId, persistentId);
    uiExtNodeIdToPersistentIdMap_.insert(std::make_pair(surfaceNodeId, persistentId));
}

void SceneSession::RemoveUIExtSurfaceNodeId(int32_t persistentId)
{
    std::unique_lock<std::shared_mutex> lock(uiExtNodeIdToPersistentIdMapMutex_);
    TLOGI(WmsLogTag::WMS_UIEXT, "Remove uiExtension by persistentId=%{public}d", persistentId);
    auto pairIter = std::find_if(uiExtNodeIdToPersistentIdMap_.begin(), uiExtNodeIdToPersistentIdMap_.end(),
        [persistentId](const auto& entry) { return entry.second == persistentId; });
    if (pairIter != uiExtNodeIdToPersistentIdMap_.end()) {
        TLOGI(WmsLogTag::WMS_UIEXT,
            "Successfully removed uiExtension pair surfaceNodeId=%{public}" PRIu64 ", persistentId=%{public}d",
            pairIter->first, persistentId);
        uiExtNodeIdToPersistentIdMap_.erase(pairIter);
        return;
    }
    TLOGE(WmsLogTag::WMS_UIEXT, "Failed to remove uiExtension by persistentId=%{public}d", persistentId);
}

int32_t SceneSession::GetUIExtPersistentIdBySurfaceNodeId(uint64_t surfaceNodeId) const
{
    std::shared_lock<std::shared_mutex> lock(uiExtNodeIdToPersistentIdMapMutex_);
    auto ret = uiExtNodeIdToPersistentIdMap_.find(surfaceNodeId);
    if (ret == uiExtNodeIdToPersistentIdMap_.end()) {
        TLOGE(WmsLogTag::WMS_UIEXT, "Failed to find uiExtension by surfaceNodeId=%{public}" PRIu64 "", surfaceNodeId);
        return 0;
    }
    return ret->second;
}

AvoidArea SceneSession::GetAvoidAreaByTypeInner(AvoidAreaType type)
{
    if (!CheckGetAvoidAreaAvailable(type)) {
        return {};
    }

    AvoidArea avoidArea;
    WSRect rect = GetSessionRect();
    switch (type) {
        case AvoidAreaType::TYPE_SYSTEM: {
            GetSystemAvoidArea(rect, avoidArea);
            return avoidArea;
        }
        case AvoidAreaType::TYPE_CUTOUT: {
            GetCutoutAvoidArea(rect, avoidArea);
            return avoidArea;
        }
        case AvoidAreaType::TYPE_SYSTEM_GESTURE: {
            return avoidArea;
        }
        case AvoidAreaType::TYPE_KEYBOARD: {
            GetKeyboardAvoidArea(rect, avoidArea);
            return avoidArea;
        }
        case AvoidAreaType::TYPE_NAVIGATION_INDICATOR: {
            GetAINavigationBarArea(rect, avoidArea);
            return avoidArea;
        }
        default: {
            TLOGE(WmsLogTag::WMS_IMMS, "cannot find type %{public}u, id %{public}d",
                type, GetPersistentId());
            return avoidArea;
        }
    }
}

AvoidArea SceneSession::GetAvoidAreaByType(AvoidAreaType type)
{
    auto task = [weakThis = wptr(this), type]() -> AvoidArea {
        auto session = weakThis.promote();
        if (!session) {
            TLOGE(WmsLogTag::WMS_IMMS, "session is null");
            return {};
        }
        return session->GetAvoidAreaByTypeInner(type);
    };
    return PostSyncTask(task, "GetAvoidAreaByType");
}

WSError SceneSession::GetAllAvoidAreas(std::map<AvoidAreaType, AvoidArea>& avoidAreas)
{
    auto task = [weakThis = wptr(this), &avoidAreas] {
        auto session = weakThis.promote();
        if (!session) {
            TLOGE(WmsLogTag::WMS_IMMS, "session is null");
            return WSError::WS_ERROR_NULLPTR;
        }

        using T = std::underlying_type_t<AvoidAreaType>;
        for (T avoidType = static_cast<T>(AvoidAreaType::TYPE_SYSTEM);
            avoidType <= static_cast<T>(AvoidAreaType::TYPE_NAVIGATION_INDICATOR); avoidType++) {
            auto type = static_cast<AvoidAreaType>(avoidType);
            avoidAreas[type] = session->GetAvoidAreaByTypeInner(type);
        }
        return WSError::WS_OK;
    };
    return PostSyncTask(task, "GetAllAvoidAreas");
}

WSError SceneSession::UpdateAvoidArea(const sptr<AvoidArea>& avoidArea, AvoidAreaType type)
{
    if (!sessionStage_) {
        return WSError::WS_ERROR_NULLPTR;
    }
    return sessionStage_->UpdateAvoidArea(avoidArea, type);
}

WSError SceneSession::SetPipActionEvent(const std::string& action, int32_t status)
{
    TLOGI(WmsLogTag::WMS_PIP, "action: %{public}s, status: %{public}d", action.c_str(), status);
    if (!sessionStage_) {
        return WSError::WS_ERROR_NULLPTR;
    }
    return sessionStage_->SetPipActionEvent(action, status);
}

WSError SceneSession::SetPiPControlEvent(WsPiPControlType controlType, WsPiPControlStatus status)
{
    TLOGI(WmsLogTag::WMS_PIP, "controlType: %{public}u, status: %{public}u", controlType, status);
    if (GetWindowType() != WindowType::WINDOW_TYPE_PIP || GetWindowMode() != WindowMode::WINDOW_MODE_PIP) {
        return WSError::WS_ERROR_INVALID_TYPE;
    }
    if (!sessionStage_) {
        return WSError::WS_ERROR_NULLPTR;
    }
    return sessionStage_->SetPiPControlEvent(controlType, status);
}

void SceneSession::RegisterProcessPrepareClosePiPCallback(NotifyPrepareClosePiPSessionFunc&& callback)
{
    auto task = [weakThis = wptr(this), callback = std::move(callback)] {
        auto session = weakThis.promote();
        if (!session) {
            TLOGNE(WmsLogTag::WMS_PIP, "session is null");
            return;
        }
        session->onPrepareClosePiPSession_ = std::move(callback);
    };
    PostTask(task, __func__);
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
            WLOGFE("Failed to set the cursor style");
        }
    }
    preWindowArea = { Session::GetWindowId(), area };
}

WSError SceneSession::HandleEnterWinwdowArea(int32_t displayX, int32_t displayY)
{
    if (displayX < 0 || displayY < 0) {
        TLOGE(WmsLogTag::WMS_EVENT, "Illegal parameter, displayX:%{private}d, displayY:%{private}d",
            displayX, displayY);
        return WSError::WS_ERROR_INVALID_PARAM;
    }

    auto windowType = Session::GetWindowType();
    auto iter = Session::windowAreas_.cend();
    if (!IsSystemSession() &&
        Session::GetWindowMode() == WindowMode::WINDOW_MODE_FLOATING &&
        (windowType == WindowType::WINDOW_TYPE_APP_MAIN_WINDOW || WindowHelper::IsSubWindow(windowType))) {
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
            (windowType == WindowType::WINDOW_TYPE_APP_MAIN_WINDOW || WindowHelper::IsSubWindow(windowType))) {
            WSRectF rectF = Session::UpdateHotRect(rect);
            isInRegion = rectF.IsInRegion(displayX, displayY);
        } else {
            isInRegion = rect.IsInRegion(displayX, displayY);
        }
        if (!isInRegion) {
            WLOGFE("The wrong event(%{public}d, %{public}d) could not be matched to the region:"
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

WSError SceneSession::ProcessPointDownSession(int32_t posX, int32_t posY)
{
    const auto& id = GetPersistentId();
    WLOGFI("id: %{public}d, type: %{public}d", id, GetWindowType());

    // notify touch outside
    if (specificCallback_ != nullptr && specificCallback_->onSessionTouchOutside_ &&
        sessionInfo_.bundleName_.find("SCBGestureBack") == std::string::npos) {
        specificCallback_->onSessionTouchOutside_(id);
    }

    // notify outside down event
    if (specificCallback_ != nullptr && specificCallback_->onOutsideDownEvent_) {
        specificCallback_->onOutsideDownEvent_(posX, posY);
    }
    return WSError::WS_OK;
}

WSError SceneSession::SendPointEventForMoveDrag(const std::shared_ptr<MMI::PointerEvent>& pointerEvent)
{
    NotifyOutsideDownEvent(pointerEvent);
    TransferPointerEvent(pointerEvent, false);
    return WSError::WS_OK;
}

void SceneSession::NotifyOutsideDownEvent(const std::shared_ptr<MMI::PointerEvent>& pointerEvent)
{
    // notify touchOutside and touchDown event
    int32_t action = pointerEvent->GetPointerAction();
    if (action != MMI::PointerEvent::POINTER_ACTION_DOWN &&
        action != MMI::PointerEvent::POINTER_ACTION_BUTTON_DOWN) {
        return;
    }

    MMI::PointerEvent::PointerItem pointerItem;
    if (!pointerEvent->GetPointerItem(pointerEvent->GetPointerId(), pointerItem)) {
        return;
    }

    // notify outside down event
    if (specificCallback_ != nullptr && specificCallback_->onOutsideDownEvent_) {
        specificCallback_->onOutsideDownEvent_(pointerItem.GetDisplayX(), pointerItem.GetDisplayY());
    }
}

WSError SceneSession::TransferPointerEvent(const std::shared_ptr<MMI::PointerEvent>& pointerEvent,
    bool needNotifyClient)
{
    WLOGFD("[WMSCom] TransferPointEvent, id: %{public}d, type: %{public}d, needNotifyClient: %{public}d",
        GetPersistentId(), GetWindowType(), needNotifyClient);
    if (pointerEvent == nullptr) {
        WLOGFE("pointerEvent is null");
        return WSError::WS_ERROR_NULLPTR;
    }

    int32_t action = pointerEvent->GetPointerAction();
    {
        bool isSystemWindow = GetSessionInfo().isSystem_;
        std::lock_guard<std::mutex> guard(enterSessionMutex_);
        if (action == MMI::PointerEvent::POINTER_ACTION_ENTER_WINDOW) {
            WLOGFD("Set enter session, persistentId:%{public}d", GetPersistentId());
            enterSession_ = wptr<SceneSession>(this);
        }
        if ((enterSession_ != nullptr) &&
            (isSystemWindow && (action != MMI::PointerEvent::POINTER_ACTION_ENTER_WINDOW))) {
            WLOGFD("Remove enter session, persistentId:%{public}d", GetPersistentId());
            enterSession_ = nullptr;
        }
    }

    if (!CheckPointerEventDispatch(pointerEvent)) {
        WLOGFI("Do not dispatch this pointer event");
        return WSError::WS_DO_NOTHING;
    }

    bool isPointDown = (action == MMI::PointerEvent::POINTER_ACTION_DOWN ||
        action == MMI::PointerEvent::POINTER_ACTION_BUTTON_DOWN);
    if (specificCallback_ != nullptr && specificCallback_->onSessionTouchOutside_ != nullptr && isPointDown) {
        specificCallback_->onSessionTouchOutside_(GetPersistentId());
    }

    auto property = GetSessionProperty();
    if (property == nullptr) {
        return Session::TransferPointerEvent(pointerEvent, needNotifyClient);
    }
    auto windowType = property->GetWindowType();
    bool isMovableWindowType = IsMovableWindowType();
    bool isMainWindow = WindowHelper::IsMainWindow(windowType);
    bool isSubWindow = WindowHelper::IsSubWindow(windowType);
    bool isDialog = WindowHelper::IsDialogWindow(windowType);
    bool isMaxModeAvoidSysBar = property->GetMaximizeMode() == MaximizeMode::MODE_AVOID_SYSTEM_BAR;
    if (isMovableWindowType && (isMainWindow || isSubWindow || isDialog) &&
        !isMaxModeAvoidSysBar) {
        if (CheckDialogOnForeground() && isPointDown) {
            HandlePointDownDialog();
            pointerEvent->MarkProcessed();
            TLOGI(WmsLogTag::WMS_DIALOG, "There is dialog window foreground");
            return WSError::WS_OK;
        }
        if (!moveDragController_) {
            WLOGE("moveDragController_ is null");
            return Session::TransferPointerEvent(pointerEvent, needNotifyClient);
        }
        if (property->GetWindowMode() == WindowMode::WINDOW_MODE_FLOATING && property->GetDragEnabled()) {
            auto isPC = systemConfig_.uiType_ == UI_TYPE_PC;
            if ((isPC || IsFreeMultiWindowMode() || (property->GetIsPcAppInPad() && !isMainWindow)) &&
                moveDragController_->ConsumeDragEvent(pointerEvent, winRect_, property, systemConfig_)) {
                moveDragController_->UpdateGravityWhenDrag(pointerEvent, surfaceNode_);
                PresentFoucusIfNeed(pointerEvent->GetPointerAction());
                pointerEvent->MarkProcessed();
                return WSError::WS_OK;
            }
        }
        if (IsDecorEnable() && moveDragController_->ConsumeMoveEvent(pointerEvent, winRect_)) {
            PresentFoucusIfNeed(pointerEvent->GetPointerAction());
            pointerEvent->MarkProcessed();
            return WSError::WS_OK;
        }
    }

    bool raiseEnabled = property->GetWindowType() == WindowType::WINDOW_TYPE_DIALOG && property->GetRaiseEnabled() &&
        (action == MMI::PointerEvent::POINTER_ACTION_DOWN || action == MMI::PointerEvent::POINTER_ACTION_BUTTON_DOWN);
    if (raiseEnabled) {
        RaiseToAppTopForPointDown();
    }
    return Session::TransferPointerEvent(pointerEvent, needNotifyClient);
}

bool SceneSession::IsMovableWindowType()
{
    auto property = GetSessionProperty();
    if (property == nullptr) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "property is null");
        return false;
    }

    return property->GetWindowMode() == WindowMode::WINDOW_MODE_FLOATING ||
        property->GetWindowMode() == WindowMode::WINDOW_MODE_SPLIT_PRIMARY ||
        property->GetWindowMode() == WindowMode::WINDOW_MODE_SPLIT_SECONDARY ||
        IsFullScreenMovable();
}

bool SceneSession::IsFullScreenMovable()
{
    auto property = GetSessionProperty();
    if (property == nullptr) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "property is null");
        return false;
    }
    return property->GetWindowMode() == WindowMode::WINDOW_MODE_FULLSCREEN &&
        WindowHelper::IsWindowModeSupported(property->GetModeSupportInfo(), WindowMode::WINDOW_MODE_FLOATING);
}

WSError SceneSession::RequestSessionBack(bool needMoveToBackground)
{
    auto task = [weakThis = wptr(this), needMoveToBackground]() {
        auto session = weakThis.promote();
        if (!session) {
            WLOGFE("session is null");
            return WSError::WS_ERROR_DESTROYED_OBJECT;
        }
        if (!session->backPressedFunc_) {
            WLOGFW("Session didn't register back event consumer!");
            return WSError::WS_DO_NOTHING;
        }
        if (g_enableForceUIFirst) {
            auto rsTransaction = RSTransactionProxy::GetInstance();
            if (rsTransaction) {
                rsTransaction->Begin();
            }
            auto leashWinSurfaceNode = session->GetLeashWinSurfaceNode();
            if (leashWinSurfaceNode) {
                leashWinSurfaceNode->SetForceUIFirst(true);
                WLOGFI("leashWinSurfaceNode_ SetForceUIFirst id:%{public}u!", session->GetPersistentId());
            } else {
                WLOGFI("failed, leashWinSurfaceNode_ null id:%{public}u", session->GetPersistentId());
            }
            if (rsTransaction) {
                rsTransaction->Commit();
            }
        }
        session->backPressedFunc_(needMoveToBackground);
        return WSError::WS_OK;
    };
    PostTask(task, "RequestSessionBack:" + std::to_string(needMoveToBackground));
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

#ifdef DEVICE_STATUS_ENABLE
void SceneSession::RotateDragWindow(std::shared_ptr<RSTransaction> rsTransaction)
{
    Msdp::DeviceStatus::DragState state = Msdp::DeviceStatus::DragState::STOP;
    Msdp::DeviceStatus::InteractionManager::GetInstance()->GetDragState(state);
    if (state == Msdp::DeviceStatus::DragState::START) {
        Msdp::DeviceStatus::InteractionManager::GetInstance()->RotateDragWindowSync(rsTransaction);
    }
}
#endif // DEVICE_STATUS_ENABLE

void SceneSession::NotifySessionRectChange(const WSRect& rect, const SizeChangeReason& reason)
{
    auto task = [weakThis = wptr(this), rect, reason]() {
        auto session = weakThis.promote();
        if (!session) {
            WLOGFE("session is null");
            return;
        }
        if (session->sessionRectChangeFunc_) {
            HITRACE_METER_FMT(HITRACE_TAG_WINDOW_MANAGER, "SceneSession::NotifySessionRectChange");
            session->sessionRectChangeFunc_(rect, reason);
        }
    };
    PostTask(task, "NotifySessionRectChange" + GetRectInfo(rect));
}

bool SceneSession::IsDecorEnable() const
{
    auto property = GetSessionProperty();
    if (property == nullptr) {
        WLOGE("property is nullptr");
        return false;
    }
    auto windowType = property->GetWindowType();
    bool isMainWindow = WindowHelper::IsMainWindow(windowType);
    bool isSubWindow = WindowHelper::IsSubWindow(windowType);
    bool isDialogWindow = WindowHelper::IsDialogWindow(windowType);
    bool isValidWindow = isMainWindow ||
        ((isSubWindow || isDialogWindow) && property->IsDecorEnable());
    bool isWindowModeSupported = WindowHelper::IsWindowModeSupported(
        systemConfig_.decorModeSupportInfo_, property->GetWindowMode());
    bool enable = isValidWindow && systemConfig_.isSystemDecorEnable_ && isWindowModeSupported;
    return enable;
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

void SceneSession::HandleCompatibleModeMoveDrag(WSRect& rect, const SizeChangeReason& reason,
    bool isSupportDragInPcCompatibleMode)
{
    auto sessionProperty = GetSessionProperty();
    if (!sessionProperty) {
        TLOGE(WmsLogTag::WMS_SCB, "sessionProperty is null");
        return;
    }
    WindowLimits windowLimits = sessionProperty->GetWindowLimits();
    const int32_t compatibleInPcPortraitWidth = sessionProperty->GetCompatibleInPcPortraitWidth();
    const int32_t compatibleInPcPortraitHeight = sessionProperty->GetCompatibleInPcPortraitHeight();
    const int32_t compatibleInPcLandscapeWidth = sessionProperty->GetCompatibleInPcLandscapeWidth();
    const int32_t compatibleInPcLandscapeHeight = sessionProperty->GetCompatibleInPcLandscapeHeight();
    const int32_t compatibleInPcDragLimit = compatibleInPcLandscapeWidth - compatibleInPcPortraitWidth;
    WSRect windowRect = GetSessionRect();
    auto windowWidth = windowRect.width_;
    auto windowHeight = windowRect.height_;

    if (reason != SizeChangeReason::MOVE) {
        if (isSupportDragInPcCompatibleMode && windowWidth > windowHeight &&
            (rect.width_ < compatibleInPcLandscapeWidth - compatibleInPcDragLimit ||
             rect.width_ == static_cast<int32_t>(windowLimits.minWidth_))) {
            rect.width_ = compatibleInPcPortraitWidth;
            rect.height_ = compatibleInPcPortraitHeight;
            SetSurfaceBounds(rect);
            UpdateSizeChangeReason(reason);
            UpdateRect(rect, reason, "compatibleInPcPortrait");
        } else if (isSupportDragInPcCompatibleMode && windowWidth < windowHeight &&
            rect.width_ > compatibleInPcPortraitWidth + compatibleInPcDragLimit) {
            rect.width_ = compatibleInPcLandscapeWidth;
            rect.height_ = compatibleInPcLandscapeHeight;
            SetSurfaceBounds(rect);
            UpdateSizeChangeReason(reason);
            UpdateRect(rect, reason, "compatibleInPcLandscape");
        } else {
            if (windowWidth < windowHeight) {
                rect.width_ = compatibleInPcPortraitWidth;
                rect.height_ = compatibleInPcPortraitHeight;
            } else {
                rect.width_ = compatibleInPcLandscapeWidth;
                rect.height_ = compatibleInPcLandscapeHeight;
            }
            rect.posX_ = windowRect.posX_;
            rect.posY_ = windowRect.posY_;
            SetSurfaceBounds(rect);
            UpdateSizeChangeReason(reason);
        }
    } else {
        SetSurfaceBounds(rect);
        UpdateSizeChangeReason(reason);
    }
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
    if (!moveDragController_) {
        WLOGE("moveDragController_ is null");
        return;
    }

    auto property = GetSessionProperty();
    if (property == nullptr) {
        TLOGE(WmsLogTag::WMS_SCB, "property is null");
        return;
    }
    bool isCompatibleModeInPc = property->GetCompatibleModeInPc();
    bool isSupportDragInPcCompatibleMode = property->GetIsSupportDragInPcCompatibleMode();
    bool isMainWindow = WindowHelper::IsMainWindow(property->GetWindowType());
    WSRect rect = moveDragController_->GetTargetRect();
    WLOGFD("OnMoveDragCallback rect: [%{public}d, %{public}d, %{public}u, %{public}u], reason : %{public}d "
        "isCompatibleMode: %{public}d, isSupportDragInPcCompatibleMode: %{public}d",
        rect.posX_, rect.posY_, rect.width_, rect.height_, reason, isCompatibleModeInPc,
        isSupportDragInPcCompatibleMode);
    if (reason == SizeChangeReason::DRAG || reason == SizeChangeReason::DRAG_END) {
        UpdateWinRectForSystemBar(rect);
    }
    HITRACE_METER_FMT(HITRACE_TAG_WINDOW_MANAGER,
        "SceneSession::OnMoveDragCallback [%d, %d, %u, %u]", rect.posX_, rect.posY_, rect.width_, rect.height_);
    if (isCompatibleModeInPc && !IsFreeMultiWindowMode()) {
        HandleCompatibleModeMoveDrag(rect, reason, isSupportDragInPcCompatibleMode);
    } else {
        if (reason == SizeChangeReason::DRAG && IsFreeMultiWindowMode() && isMainWindow) {
            OnSessionEvent(SessionEvent::EVENT_DRAG);
            return;
        }
        SetSurfaceBounds(rect);
        UpdateSizeChangeReason(reason);
        if (reason != SizeChangeReason::MOVE) {
            UpdateRect(rect, reason, "OnMoveDragCallback");
        }
    }

    if (reason == SizeChangeReason::DRAG_END) {
        if (GetOriPosYBeforeRaisedByKeyboard() != 0) {
            TLOGI(WmsLogTag::WMS_KEYBOARD, "Calling session is moved and reset oriPosYBeforeRaisedByKeyboard");
            SetOriPosYBeforeRaisedByKeyboard(0);
        }
        NotifySessionRectChange(rect, reason);
        OnSessionEvent(SessionEvent::EVENT_END_MOVE);
    }
    if (reason == SizeChangeReason::DRAG_START) {
        OnSessionEvent(SessionEvent::EVENT_DRAG_START);
    }
}

void SceneSession::UpdateWinRectForSystemBar(WSRect& rect)
{
    if (!specificCallback_) {
        WLOGFE("specificCallback_ is null!");
        return;
    }
    auto sessionProperty = GetSessionProperty();
    if (!sessionProperty) {
        WLOGFE("get session property is null!");
        return;
    }
    float tmpPosY = 0.0;
    std::vector<sptr<SceneSession>> statusBarVector;
    if (specificCallback_->onGetSceneSessionVectorByType_) {
        statusBarVector = specificCallback_->onGetSceneSessionVectorByType_(
            WindowType::WINDOW_TYPE_STATUS_BAR, sessionProperty->GetDisplayId());
    }
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
    if (rsTransaction != nullptr) {
        rsTransaction->Begin();
    }
    auto leashWinSurfaceNode = GetLeashWinSurfaceNode();
    if (surfaceNode_ && leashWinSurfaceNode) {
        leashWinSurfaceNode->SetBounds(rect.posX_, rect.posY_, rect.width_, rect.height_);
        leashWinSurfaceNode->SetFrame(rect.posX_, rect.posY_, rect.width_, rect.height_);
        surfaceNode_->SetBounds(0, 0, rect.width_, rect.height_);
        surfaceNode_->SetFrame(0, 0, rect.width_, rect.height_);
    } else if (WindowHelper::IsPipWindow(GetWindowType()) && surfaceNode_) {
        TLOGD(WmsLogTag::WMS_PIP, "PipWindow setSurfaceBounds");
        surfaceNode_->SetBounds(rect.posX_, rect.posY_, rect.width_, rect.height_);
        surfaceNode_->SetFrame(rect.posX_, rect.posY_, rect.width_, rect.height_);
    } else if (WindowHelper::IsSubWindow(GetWindowType()) && surfaceNode_) {
        WLOGFD("subwindow setSurfaceBounds");
        surfaceNode_->SetBounds(rect.posX_, rect.posY_, rect.width_, rect.height_);
        surfaceNode_->SetFrame(rect.posX_, rect.posY_, rect.width_, rect.height_);
    } else if (WindowHelper::IsDialogWindow(GetWindowType()) && surfaceNode_) {
        TLOGD(WmsLogTag::WMS_DIALOG, "dialogWindow setSurfaceBounds");
        surfaceNode_->SetBounds(rect.posX_, rect.posY_, rect.width_, rect.height_);
        surfaceNode_->SetFrame(rect.posX_, rect.posY_, rect.width_, rect.height_);
    } else {
        WLOGE("SetSurfaceBounds surfaceNode is null!");
    }
    if (rsTransaction != nullptr) {
        rsTransaction->Commit();
    }
}

void SceneSession::SetZOrder(uint32_t zOrder)
{
    auto task = [weakThis = wptr(this), zOrder]() {
        auto session = weakThis.promote();
        if (session == nullptr) {
            WLOGFE("session is null");
            return;
        }
        if (session->zOrder_ != zOrder) {
            session->Session::SetZOrder(zOrder);
            if (session->specificCallback_ != nullptr) {
                session->specificCallback_->onWindowInfoUpdate_(session->GetPersistentId(),
                    WindowUpdateType::WINDOW_UPDATE_PROPERTY);
            }
        }
    };
    PostTask(task, "SetZOrder");
}

void SceneSession::SetFloatingScale(float floatingScale)
{
    if (floatingScale_ != floatingScale) {
        Session::SetFloatingScale(floatingScale);
        if (specificCallback_ != nullptr) {
            specificCallback_->onWindowInfoUpdate_(GetPersistentId(), WindowUpdateType::WINDOW_UPDATE_PROPERTY);
            if (Session::IsScbCoreEnabled()) {
                dirtyFlags_ |= static_cast<uint32_t>(SessionUIDirtyFlag::AVOID_AREA);
            } else {
                specificCallback_->onUpdateAvoidArea_(GetPersistentId());
            }
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

int32_t SceneSession::GetMainSessionId()
{
    const auto& mainSession = GetMainSession();
    if (mainSession) {
        return mainSession->GetPersistentId();
    }
    return INVALID_SESSION_ID;
}

std::string SceneSession::GetWindowNameAllType() const
{
    if (GetSessionInfo().isSystem_) {
        return GetSessionInfo().abilityName_;
    } else {
        return GetWindowName();
    }
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

WMError SceneSession::SetWindowEnableDragBySystem(bool enableDrag)
{
    TLOGI(WmsLogTag::WMS_LAYOUT, "enableDrag: %{public}d", enableDrag);
    auto task = [weakThis = wptr(this), enableDrag] {
        auto session = weakThis.promote();
        if (!session) {
            TLOGNE(WmsLogTag::WMS_LAYOUT, "session is null");
            return;
        }
        TLOGNI(WmsLogTag::WMS_LAYOUT, "id: %{public}d, enableDrag: %{public}d",
            session->GetPersistentId(), enableDrag);
        auto sessionProperty = session->GetSessionProperty();
        if (!sessionProperty) {
            TLOGNE(WmsLogTag::WMS_LAYOUT, "sessionProperty is null");
            return;
        }
        sessionProperty->SetDragEnabled(enableDrag);
        if (session->sessionStage_) {
            session->sessionStage_->SetEnableDragBySystem(enableDrag);
        }
    };
    PostTask(task, __func__);
    return WMError::WM_OK;
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

void SceneSession::SaveUpdatedIcon(const std::shared_ptr<Media::PixelMap>& icon)
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
    auto task = [weakThis = wptr(this), visible]() {
        auto session = weakThis.promote();
        if (!session) {
            TLOGE(WmsLogTag::WMS_LIFE, "session is null");
            return;
        }
        int32_t persistentId = session->GetPersistentId();
        WLOGFI("[WMSSCB] name: %{public}s, id: %{public}u, visible: %{public}u",
            session->sessionInfo_.bundleName_.c_str(), persistentId, visible);
        session->isVisible_ = visible;
        if (session->specificCallback_ == nullptr) {
            WLOGFW("specific callback is null.");
            return;
        }

        if (visible) {
            session->specificCallback_->onWindowInfoUpdate_(persistentId, WindowUpdateType::WINDOW_UPDATE_ADDED);
        } else {
            session->specificCallback_->onWindowInfoUpdate_(persistentId, WindowUpdateType::WINDOW_UPDATE_REMOVED);
        }
        session->NotifyAccessibilityVisibilityChange();
        session->specificCallback_->onUpdateAvoidArea_(persistentId);
        // update private state
        if (!session->GetSessionProperty()) {
            WLOGFE("UpdateNativeVisibility property is null");
            return;
        }
        if (session->updatePrivateStateAndNotifyFunc_ != nullptr) {
            session->updatePrivateStateAndNotifyFunc_(persistentId);
        }
    };
    PostTask(task, "UpdateNativeVisibility");
}

bool SceneSession::IsVisible() const
{
    return isVisible_;
}

void SceneSession::UpdateRotationAvoidArea()
{
    if (specificCallback_) {
        if (Session::IsScbCoreEnabled()) {
            dirtyFlags_ |= static_cast<uint32_t>(SessionUIDirtyFlag::AVOID_AREA);
        } else {
            specificCallback_->onUpdateAvoidArea_(GetPersistentId());
        }
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
    auto rsTransaction = RSTransactionProxy::GetInstance();
    if (rsTransaction != nullptr) {
        rsTransaction->Begin();
    }
    surfaceNode_->SetSecurityLayer(isPrivacy);
    auto leashWinSurfaceNode = GetLeashWinSurfaceNode();
    if (leashWinSurfaceNode != nullptr) {
        leashWinSurfaceNode->SetSecurityLayer(isPrivacy);
    }
    if (rsTransaction != nullptr) {
        rsTransaction->Commit();
    }
}

void SceneSession::SetSnapshotSkip(bool isSkip)
{
    auto property = GetSessionProperty();
    if (!property) {
        TLOGE(WmsLogTag::DEFAULT, "property is null");
        return;
    }
    if (!surfaceNode_) {
        TLOGE(WmsLogTag::DEFAULT, "surfaceNode_ is null");
        return;
    }
    bool lastSnapshotSkip = property->GetSnapshotSkip();
    if (lastSnapshotSkip == isSkip) {
        TLOGW(WmsLogTag::DEFAULT, "Snapshot skip does not change, do nothing, isSkip: %{public}d, "
            "id: %{public}d", isSkip, GetPersistentId());
        return;
    }
    property->SetSnapshotSkip(isSkip);
    auto rsTransaction = RSTransactionProxy::GetInstance();
    if (rsTransaction != nullptr) {
        rsTransaction->Begin();
    }
    surfaceNode_->SetSkipLayer(isSkip);
    auto leashWinSurfaceNode = GetLeashWinSurfaceNode();
    if (leashWinSurfaceNode != nullptr) {
        leashWinSurfaceNode->SetSkipLayer(isSkip);
    }
    if (rsTransaction != nullptr) {
        rsTransaction->Commit();
    }
}

void SceneSession::SetPiPTemplateInfo(const PiPTemplateInfo& pipTemplateInfo)
{
    pipTemplateInfo_ = pipTemplateInfo;
}

void SceneSession::SetSystemSceneOcclusionAlpha(double alpha)
{
    HITRACE_METER_FMT(HITRACE_TAG_WINDOW_MANAGER, "SceneSession::SetAbilityBGAlpha");
    if (alpha < 0 || alpha > 1.0) {
        WLOGFE("OnSetSystemSceneOcclusionAlpha property is null");
        return;
    }
    if (!surfaceNode_) {
        WLOGFE("surfaceNode_ is null");
        return;
    }
    uint8_t alpha8bit = static_cast<uint8_t>(alpha * 255);
    WLOGFI("SetAbilityBGAlpha alpha8bit=%{public}u.", alpha8bit);
    auto rsTransaction = RSTransactionProxy::GetInstance();
    if (rsTransaction != nullptr) {
        rsTransaction->Begin();
    }
    surfaceNode_->SetAbilityBGAlpha(alpha8bit);
    auto leashWinSurfaceNode = GetLeashWinSurfaceNode();
    if (leashWinSurfaceNode != nullptr) {
        leashWinSurfaceNode->SetAbilityBGAlpha(alpha8bit);
    }
    if (rsTransaction != nullptr) {
        rsTransaction->Commit();
    }
}

void SceneSession::SetSystemSceneForceUIFirst(bool forceUIFirst)
{
    HITRACE_METER_FMT(HITRACE_TAG_WINDOW_MANAGER, "SceneSession::SetForceUIFirst");
    auto leashWinSurfaceNode = GetLeashWinSurfaceNode();
    if (leashWinSurfaceNode == nullptr && surfaceNode_ == nullptr) {
        TLOGE(WmsLogTag::DEFAULT, "leashWindow and surfaceNode are nullptr");
        return;
    }
    auto rsTransaction = RSTransactionProxy::GetInstance();
    if (rsTransaction != nullptr) {
        rsTransaction->Begin();
    }
    if (leashWinSurfaceNode != nullptr) {
        TLOGI(WmsLogTag::DEFAULT, "%{public}s %{public}" PRIu64 " forceUIFirst=%{public}d.",
            leashWinSurfaceNode->GetName().c_str(), leashWinSurfaceNode->GetId(), forceUIFirst);
        leashWinSurfaceNode->SetForceUIFirst(forceUIFirst);
    } else if (surfaceNode_ != nullptr) {
        TLOGI(WmsLogTag::DEFAULT, "%{public}s %{public}" PRIu64 " forceUIFirst=%{public}d.",
            surfaceNode_->GetName().c_str(), surfaceNode_->GetId(), forceUIFirst);
        surfaceNode_->SetForceUIFirst(forceUIFirst);
    }
    if (rsTransaction != nullptr) {
        rsTransaction->Commit();
    }
}

WSError SceneSession::UpdateWindowAnimationFlag(bool needDefaultAnimationFlag)
{
    auto task = [weakThis = wptr(this), needDefaultAnimationFlag] {
        auto session = weakThis.promote();
        if (!session) {
            TLOGNE(WmsLogTag::WMS_LIFE, "session is null");
            return WSError::WS_ERROR_DESTROYED_OBJECT;
        }
        session->needDefaultAnimationFlag_ = needDefaultAnimationFlag;
        if (session->onWindowAnimationFlagChange_) {
            session->onWindowAnimationFlagChange_(needDefaultAnimationFlag);
        }
        return WSError::WS_OK;
    };
    return PostSyncTask(task, __func__);
}

void SceneSession::SetWindowAnimationFlag(bool needDefaultAnimationFlag)
{
    needDefaultAnimationFlag_ = needDefaultAnimationFlag;
    if (onWindowAnimationFlagChange_) {
        onWindowAnimationFlagChange_(needDefaultAnimationFlag);
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

/** @note @window.focus */
bool SceneSession::IsAppOrLowerSystemSession() const
{
    WindowType windowType = GetWindowType();
    if (windowType == WindowType::WINDOW_TYPE_NEGATIVE_SCREEN ||
        windowType == WindowType::WINDOW_TYPE_GLOBAL_SEARCH ||
        windowType == WindowType::WINDOW_TYPE_DESKTOP) {
        return true;
    }
    return IsAppSession();
}

/** @note @window.focus */
bool SceneSession::IsSystemSessionAboveApp() const
{
    WindowType windowType = GetWindowType();
    if (windowType == WindowType::WINDOW_TYPE_DIALOG || windowType == WindowType::WINDOW_TYPE_SYSTEM_ALARM_WINDOW) {
        return true;
    }
    if (windowType == WindowType::WINDOW_TYPE_PANEL &&
        sessionInfo_.bundleName_.find("SCBDropdownPanel") != std::string::npos) {
        return true;
    }
    return false;
}

void SceneSession::NotifyIsCustomAnimationPlaying(bool isPlaying)
{
    WLOGFI("id %{public}d %{public}u", GetPersistentId(), isPlaying);
    if (onIsCustomAnimationPlaying_) {
        onIsCustomAnimationPlaying_(isPlaying);
    }
}

WSError SceneSession::UpdateWindowSceneAfterCustomAnimation(bool isAdd)
{
    if (!SessionPermission::IsSystemCalling()) {
        TLOGE(WmsLogTag::WMS_SYSTEM, "failed to update with id:%{public}u!", GetPersistentId());
        return WSError::WS_ERROR_NOT_SYSTEM_APP;
    }
    auto task = [weakThis = wptr(this), isAdd]() {
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
    };
    PostTask(task, "UpdateWindowSceneAfterCustomAnimation:" + std::to_string(isAdd));
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

PiPTemplateInfo SceneSession::GetPiPTemplateInfo() const
{
    return pipTemplateInfo_;
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
    WLOGFI("id: %{public}d, type: %{public}d", GetPersistentId(), GetWindowType());
    if (sessionStage_) {
        WLOGFD("Notify sessionStage TouchOutside");
        sessionStage_->NotifyTouchOutside();
    }
    if (sessionChangeCallback_ && sessionChangeCallback_->OnTouchOutside_) {
        WLOGFD("Notify sessionChangeCallback TouchOutside");
        sessionChangeCallback_->OnTouchOutside_();
    }
}

void SceneSession::NotifyWindowVisibility()
{
    if (sessionStage_) {
        sessionStage_->NotifyWindowVisibility(GetRSVisible());
    } else {
        WLOGFE("Notify window(id:%{public}d) visibility failed, for this session stage is nullptr", GetPersistentId());
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
    if (onRequestedOrientationChange_) {
        onRequestedOrientationChange_(static_cast<uint32_t>(orientation));
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
    if (onForceHideChangeFunc_) {
        onForceHideChangeFunc_(hide);
    }
    SetForceTouchable(!hide);
    if (hide) {
        if (isFocused_) {
            FocusChangeReason reason = FocusChangeReason::DEFAULT;
            NotifyRequestFocusStatusNotifyManager(false, true, reason);
            SetForceHideState(ForceHideState::HIDDEN_WHEN_FOCUSED);
        } else if (forceHideState_ == ForceHideState::NOT_HIDDEN) {
            SetForceHideState(ForceHideState::HIDDEN_WHEN_UNFOCUSED);
        }
    } else {
        if (forceHideState_ == ForceHideState::HIDDEN_WHEN_FOCUSED) {
            SetForceHideState(ForceHideState::NOT_HIDDEN);
            FocusChangeReason reason = FocusChangeReason::DEFAULT;
            NotifyRequestFocusStatusNotifyManager(true, true, reason);
        } else {
            SetForceHideState(ForceHideState::NOT_HIDDEN);
        }
    }
}

Orientation SceneSession::GetRequestedOrientation() const
{
    return GetSessionProperty()->GetRequestedOrientation();
}

bool SceneSession::IsAnco() const
{
    return collaboratorType_ == static_cast<int32_t>(CollaboratorType::RESERVE_TYPE);
}

void SceneSession::SetBlankFlag(bool isAddBlank)
{
    isAddBlank_ = isAddBlank;
}

bool SceneSession::GetBlankFlag() const
{
    return isAddBlank_;
}

void SceneSession::SetBufferAvailableCallbackEnable(bool enable)
{
    bufferAvailableCallbackEnable_ = enable;
}

bool SceneSession::GetBufferAvailableCallbackEnable() const
{
    return bufferAvailableCallbackEnable_;
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

std::string SceneSession::GetClientIdentityToken() const
{
    return clientIdentityToken_;
}

void SceneSession::SetClientIdentityToken(const std::string& clientIdentityToken)
{
    clientIdentityToken_ = clientIdentityToken;
}

void SceneSession::DumpSessionInfo(std::vector<std::string>& info) const
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

void SceneSession::SetSessionState(SessionState state)
{
    Session::SetSessionState(state);
    NotifyAccessibilityVisibilityChange();
}

void SceneSession::UpdateSessionState(SessionState state)
{
    Session::UpdateSessionState(state);
    NotifyAccessibilityVisibilityChange();
}

bool SceneSession::IsVisibleForAccessibility() const
{
    if (Session::IsScbCoreEnabled()) {
        return GetSystemTouchable() && GetForegroundInteractiveStatus() && IsVisibleForeground();
    }
    return GetSystemTouchable() && GetForegroundInteractiveStatus() &&
        (IsVisible() || state_ == SessionState::STATE_ACTIVE || state_ == SessionState::STATE_FOREGROUND);
}

void SceneSession::SetForegroundInteractiveStatus(bool interactive)
{
    Session::SetForegroundInteractiveStatus(interactive);
    NotifyAccessibilityVisibilityChange();
    if (interactive) {
        return;
    }
    for (auto toastSession : toastSession_) {
        if (toastSession == nullptr) {
            TLOGD(WmsLogTag::WMS_TOAST, "toastSession session is nullptr");
            continue;
        }
        auto state = toastSession->GetSessionState();
        if (state != SessionState::STATE_FOREGROUND && state != SessionState::STATE_ACTIVE) {
            continue;
        }
        toastSession->SetActive(false);
        toastSession->BackgroundTask();
    }
}

void SceneSession::NotifyAccessibilityVisibilityChange()
{
    bool isVisibleForAccessibilityNew = IsVisibleForAccessibility();
    if (isVisibleForAccessibilityNew == isVisibleForAccessibility_.load()) {
        return;
    }
    WLOGFD("[WMSAccess] NotifyAccessibilityVisibilityChange id: %{public}d, access: %{public}d ",
        GetPersistentId(), isVisibleForAccessibilityNew);
    isVisibleForAccessibility_.store(isVisibleForAccessibilityNew);
    if (specificCallback_ && specificCallback_->onWindowInfoUpdate_) {
        if (isVisibleForAccessibilityNew) {
            specificCallback_->onWindowInfoUpdate_(GetPersistentId(), WindowUpdateType::WINDOW_UPDATE_ADDED);
        } else {
            specificCallback_->onWindowInfoUpdate_(GetPersistentId(), WindowUpdateType::WINDOW_UPDATE_REMOVED);
        }
    } else {
        WLOGFD("specificCallback_->onWindowInfoUpdate_ not exist, persistent id: %{public}d", GetPersistentId());
    }
}

void SceneSession::SetSystemTouchable(bool touchable)
{
    Session::SetSystemTouchable(touchable);
    NotifyAccessibilityVisibilityChange();
}

WSError SceneSession::ChangeSessionVisibilityWithStatusBar(
    const sptr<AAFwk::SessionInfo> abilitySessionInfo, bool visible)
{
    if (!SessionPermission::VerifyCallingPermission(PermissionConstants::PERMISSION_MANAGE_MISSION)) {
        TLOGE(WmsLogTag::WMS_LIFE, "The caller has not permission granted");
        return WSError::WS_ERROR_INVALID_PERMISSION;
    }
    auto task = [weakThis = wptr(this), abilitySessionInfo, visible]() {
        auto session = weakThis.promote();
        if (!session) {
            WLOGFE("session is null");
            return WSError::WS_ERROR_DESTROYED_OBJECT;
        }
        if (abilitySessionInfo == nullptr) {
            WLOGFE("abilitySessionInfo is null");
            return WSError::WS_ERROR_NULLPTR;
        }

        SessionInfo info;
        info.abilityName_ = abilitySessionInfo->want.GetElement().GetAbilityName();
        info.bundleName_ = abilitySessionInfo->want.GetElement().GetBundleName();
        info.moduleName_ = abilitySessionInfo->want.GetModuleName();
        int32_t appCloneIndex = abilitySessionInfo->want.GetIntParam(APP_CLONE_INDEX, 0);
        info.appIndex_ = appCloneIndex == 0 ? abilitySessionInfo->want.GetIntParam(DLP_INDEX, 0) : appCloneIndex;
        info.persistentId_ = abilitySessionInfo->persistentId;
        info.callerPersistentId_ = session->GetPersistentId();
        info.callerBundleName_ = abilitySessionInfo->want.GetStringParam(AAFwk::Want::PARAM_RESV_CALLER_BUNDLE_NAME);
        info.callerAbilityName_ = abilitySessionInfo->want.GetStringParam(AAFwk::Want::PARAM_RESV_CALLER_ABILITY_NAME);
        info.callState_ = static_cast<uint32_t>(abilitySessionInfo->state);
        info.uiAbilityId_ = abilitySessionInfo->uiAbilityId;
        info.want = std::make_shared<AAFwk::Want>(abilitySessionInfo->want);
        info.requestCode = abilitySessionInfo->requestCode;
        info.callerToken_ = abilitySessionInfo->callerToken;
        info.startSetting = abilitySessionInfo->startSetting;
        info.callingTokenId_ = abilitySessionInfo->callingTokenId;
        info.reuse = abilitySessionInfo->reuse;
        info.processOptions = abilitySessionInfo->processOptions;

        if (session->changeSessionVisibilityWithStatusBarFunc_) {
            session->changeSessionVisibilityWithStatusBarFunc_(info, visible);
        }

        return WSError::WS_OK;
    };
    PostTask(task, "ChangeSessionVisibilityWithStatusBar");
    return WSError::WS_OK;
}

static SessionInfo MakeSessionInfoDuringPendingActivation(const sptr<AAFwk::SessionInfo>& abilitySessionInfo,
    int32_t persistentId)
{
    SessionInfo info;
    info.abilityName_ = abilitySessionInfo->want.GetElement().GetAbilityName();
    info.bundleName_ = abilitySessionInfo->want.GetElement().GetBundleName();
    info.moduleName_ = abilitySessionInfo->want.GetModuleName();
    int32_t appCloneIndex = abilitySessionInfo->want.GetIntParam(APP_CLONE_INDEX, 0);
    info.appIndex_ = appCloneIndex == 0 ? abilitySessionInfo->want.GetIntParam(DLP_INDEX, 0) : appCloneIndex;
    info.persistentId_ = abilitySessionInfo->persistentId;
    info.callerPersistentId_ = persistentId;
    info.callerBundleName_ = abilitySessionInfo->want.GetStringParam(AAFwk::Want::PARAM_RESV_CALLER_BUNDLE_NAME);
    info.callerAbilityName_ = abilitySessionInfo->want.GetStringParam(AAFwk::Want::PARAM_RESV_CALLER_ABILITY_NAME);
    info.callState_ = static_cast<uint32_t>(abilitySessionInfo->state);
    info.uiAbilityId_ = abilitySessionInfo->uiAbilityId;
    info.want = std::make_shared<AAFwk::Want>(abilitySessionInfo->want);
    info.requestCode = abilitySessionInfo->requestCode;
    info.callerToken_ = abilitySessionInfo->callerToken;
    info.startSetting = abilitySessionInfo->startSetting;
    info.callingTokenId_ = abilitySessionInfo->callingTokenId;
    info.reuse = abilitySessionInfo->reuse;
    info.processOptions = abilitySessionInfo->processOptions;
    info.isAtomicService_ = abilitySessionInfo->isAtomicService;
    info.isBackTransition_ = abilitySessionInfo->isBackTransition;
    info.needClearInNotShowRecent_ = abilitySessionInfo->needClearInNotShowRecent;
    info.isFromIcon_ = abilitySessionInfo->isFromIcon;

    if (info.want != nullptr) {
        info.windowMode = info.want->GetIntParam(AAFwk::Want::PARAM_RESV_WINDOW_MODE, 0);
        info.sessionAffinity = info.want->GetStringParam(Rosen::PARAM_KEY::PARAM_MISSION_AFFINITY_KEY);
        info.screenId_ = static_cast<uint64_t>(info.want->GetIntParam(AAFwk::Want::PARAM_RESV_DISPLAY_ID, -1));
        TLOGI(WmsLogTag::WMS_LIFE, "want: screenId %{public}" PRIu64, info.screenId_);
    }
    if (info.windowMode == static_cast<int32_t>(WindowMode::WINDOW_MODE_FULLSCREEN)) {
        info.fullScreenStart_ = true;
    }
    TLOGI(WmsLogTag::WMS_LIFE, "bundleName:%{public}s, moduleName:%{public}s, "
        "abilityName:%{public}s, appIndex:%{public}d, affinity:%{public}s. "
        "callState:%{public}d, want persistentId:%{public}d, "
        "uiAbilityId:%{public}" PRIu64 ", windowMode:%{public}d, callerId:%{public}d "
        "needClearInNotShowRecent:%{public}u, isFromIcon:%{public}d",
        info.bundleName_.c_str(), info.moduleName_.c_str(), info.abilityName_.c_str(), info.appIndex_,
        info.sessionAffinity.c_str(), info.callState_, info.persistentId_, info.uiAbilityId_,
        info.windowMode, info.callerPersistentId_, info.needClearInNotShowRecent_, info.isFromIcon_);
    return info;
}

WSError SceneSession::PendingSessionActivation(const sptr<AAFwk::SessionInfo> abilitySessionInfo)
{
    if (!SessionPermission::VerifyCallingPermission(PermissionConstants::PERMISSION_MANAGE_MISSION)) {
        TLOGE(WmsLogTag::WMS_LIFE, "The caller has not permission granted");
        return WSError::WS_ERROR_INVALID_PERMISSION;
    }
    bool isFoundationCall = SessionPermission::IsFoundationCall();
    auto task = [weakThis = wptr(this), abilitySessionInfo, isFoundationCall]() {
        auto session = weakThis.promote();
        if (!session) {
            TLOGE(WmsLogTag::WMS_LIFE, "session is null");
            return WSError::WS_ERROR_DESTROYED_OBJECT;
        }
        if (abilitySessionInfo == nullptr) {
            TLOGE(WmsLogTag::WMS_LIFE, "abilitySessionInfo is null");
            return WSError::WS_ERROR_NULLPTR;
        }
        bool isFromAncoAndToAnco = session->IsAnco() && AbilityInfoManager::GetInstance().IsAnco(
            abilitySessionInfo->want.GetElement().GetBundleName(),
            abilitySessionInfo->want.GetElement().GetAbilityName(), abilitySessionInfo->want.GetModuleName());
        if (!session->IsPcOrPadEnableActivation() && WindowHelper::IsMainWindow(session->GetWindowType())) {
            SessionState sessionState = session->GetSessionState();
            TLOGI(WmsLogTag::WMS_LIFE, "sceneSession state:%{public}d, isFoundationCall:%{public}u, "
                "canStartAbilityFromBackground:%{public}u, foregroundInteractiveStatus:%{public}u",
                sessionState, isFoundationCall, abilitySessionInfo->canStartAbilityFromBackground,
                session->GetForegroundInteractiveStatus());
            bool isSessionForeground = sessionState == SessionState::STATE_FOREGROUND ||
                sessionState == SessionState::STATE_ACTIVE;
            if (isSessionForeground && !session->GetForegroundInteractiveStatus()) {
                TLOGW(WmsLogTag::WMS_LIFE, "start ability invalid, sceneSession in a non interactive state");
                return WSError::WS_ERROR_INVALID_OPERATION;
            }
            if (!isSessionForeground && !isFromAncoAndToAnco &&
                !(isFoundationCall && abilitySessionInfo->canStartAbilityFromBackground)) {
                TLOGW(WmsLogTag::WMS_LIFE, "no permission to start ability from Background");
                return WSError::WS_ERROR_INVALID_OPERATION;
            }
        }
        session->sessionInfo_.startMethod = StartMethod::START_CALL;
        SessionInfo info = MakeSessionInfoDuringPendingActivation(abilitySessionInfo, session->GetPersistentId());
        session->HandleCastScreenConnection(info, session);
        if (session->pendingSessionActivationFunc_) {
            session->pendingSessionActivationFunc_(info);
        }
        return WSError::WS_OK;
    };
    PostTask(task, "PendingSessionActivation");
    return WSError::WS_OK;
}

void SceneSession::HandleCastScreenConnection(SessionInfo& info, sptr<SceneSession> session)
{
    ScreenId defScreenId = ScreenSessionManagerClient::GetInstance().GetDefaultScreenId();
    if (defScreenId == info.screenId_) {
        return;
    }
    auto flag = Rosen::ScreenManager::GetInstance().GetVirtualScreenFlag(info.screenId_);
    if (flag != VirtualScreenFlag::CAST) {
        return;
    }
    TLOGI(WmsLogTag::WMS_LIFE, "Get exist session state :%{public}d persistentId:%{public}d",
        session->GetSessionState(), info.callerPersistentId_);
    if (session->GetSessionState() != SessionState::STATE_FOREGROUND &&
        session->GetSessionState() != SessionState::STATE_ACTIVE) {
        TLOGI(WmsLogTag::WMS_LIFE, "Get exist session state is not foreground");
        return;
    }
    info.isCastSession_ = true;
    std::vector<uint64_t> mirrorIds { info.screenId_ };
    Rosen::DMError ret = Rosen::ScreenManager::GetInstance().MakeUniqueScreen(mirrorIds);
    if (ret != Rosen::DMError::DM_OK) {
        TLOGE(WmsLogTag::WMS_LIFE, "MakeUniqueScreen failed,ret: %{public}d", ret);
        return;
    }
}

static bool IsNeedSystemPermissionByAction(WSPropertyChangeAction action,
    const sptr<WindowSessionProperty>& property, const sptr<WindowSessionProperty>& sessionProperty)
{
    switch (action) {
        case WSPropertyChangeAction::ACTION_UPDATE_TURN_SCREEN_ON:
        case WSPropertyChangeAction::ACTION_UPDATE_SNAPSHOT_SKIP:
        case WSPropertyChangeAction::ACTION_UPDATE_HIDE_NON_SYSTEM_FLOATING_WINDOWS:
        case WSPropertyChangeAction::ACTION_UPDATE_TOPMOST:
        case WSPropertyChangeAction::ACTION_UPDATE_DECOR_ENABLE:
        case WSPropertyChangeAction::ACTION_UPDATE_DRAGENABLED:
        case WSPropertyChangeAction::ACTION_UPDATE_RAISEENABLED:
        case WSPropertyChangeAction::ACTION_UPDATE_MODE_SUPPORT_INFO:
            return true;
        case WSPropertyChangeAction::ACTION_UPDATE_ANIMATION_FLAG:
            return property->GetAnimationFlag() == static_cast<uint32_t>(WindowAnimation::CUSTOM);
        case WSPropertyChangeAction::ACTION_UPDATE_FLAGS: {
            uint32_t oldFlags = sessionProperty->GetWindowFlags();
            uint32_t flags = property->GetWindowFlags();
            if ((oldFlags ^ flags) == static_cast<uint32_t>(WindowFlag::WINDOW_FLAG_WATER_MARK)) {
                return true;
            }
            break;
        }
        default:
            break;
    }
    return false;
}

WMError SceneSession::UpdateSessionPropertyByAction(const sptr<WindowSessionProperty>& property,
    WSPropertyChangeAction action)
{
    if (property == nullptr) {
        TLOGE(WmsLogTag::DEFAULT, "property is nullptr");
        return WMError::WM_ERROR_NULLPTR;
    }
    auto sessionProperty = GetSessionProperty();
    if (sessionProperty == nullptr) {
        TLOGE(WmsLogTag::DEFAULT, "get session property failed");
        return WMError::WM_ERROR_NULLPTR;
    }
    if (action == WSPropertyChangeAction::ACTION_UPDATE_PRIVACY_MODE) {
        if (!SessionPermission::VerifyCallingPermission("ohos.permission.PRIVACY_WINDOW")) {
            return WMError::WM_ERROR_INVALID_PERMISSION;
        }
    }

    bool isSystemCalling = SessionPermission::IsSystemCalling() || SessionPermission::IsStartByHdcd();
    if (!isSystemCalling && IsNeedSystemPermissionByAction(action, property, sessionProperty)) {
        TLOGE(WmsLogTag::DEFAULT, "permission denied! action: %{public}u", action);
        return WMError::WM_ERROR_NOT_SYSTEM_APP;
    }
    property->SetSystemCalling(isSystemCalling);
    wptr<SceneSession> weak = this;
    auto task = [weak, property, action]() -> WMError {
        auto sceneSession = weak.promote();
        if (sceneSession == nullptr) {
            TLOGE(WmsLogTag::DEFAULT, "the session is nullptr");
            return WMError::WM_DO_NOTHING;
        }
        TLOGD(WmsLogTag::DEFAULT, "Id: %{public}d, action: %{public}u", sceneSession->GetPersistentId(), action);
        HITRACE_METER_FMT(HITRACE_TAG_WINDOW_MANAGER, "SceneSession:UpdateProperty");
        return sceneSession->HandleUpdatePropertyByAction(property, sceneSession, action);
    };
    if (AppExecFwk::EventRunner::IsAppMainThread()) {
        PostTask(task, "UpdateProperty");
        return WMError::WM_OK;
    }
    return PostSyncTask(task, "UpdateProperty");
}

WMError SceneSession::SetGestureBackEnabled(bool isEnabled)
{
    auto task = [weakThis = wptr(this), isEnabled] {
        auto sceneSession = weakThis.promote();
        if (!sceneSession) {
            TLOGNE(WmsLogTag::WMS_IMMS, "session is invalid");
            return;
        }
        if (sceneSession->isEnableGestureBack_ == isEnabled) {
            TLOGNI(WmsLogTag::WMS_IMMS, "isEnabled equals last.");
            return;
        }
        TLOGNI(WmsLogTag::WMS_IMMS, "id: %{public}d, isEnabled: %{public}d",
            sceneSession->GetPersistentId(), isEnabled);
        sceneSession->isEnableGestureBack_ = isEnabled;
        sceneSession->isEnableGestureBackHadSet_ = true;
        sceneSession->UpdateGestureBackEnabled();
    };
    PostTask(task, __func__);
    return WMError::WM_OK;
}

bool SceneSession::GetGestureBackEnabled()
{
    return isEnableGestureBack_;
}

bool SceneSession::GetEnableGestureBackHadSet()
{
    return isEnableGestureBackHadSet_;
}

void SceneSession::SetSessionChangeByActionNotifyManagerListener(const SessionChangeByActionNotifyManagerFunc& func)
{
    TLOGD(WmsLogTag::DEFAULT, "setListener success");
    sessionChangeByActionNotifyManagerFunc_ = func;
}

WMError SceneSession::HandleUpdatePropertyByAction(const sptr<WindowSessionProperty>& property,
    const sptr<SceneSession>& sceneSession, WSPropertyChangeAction action)
{
    if (sceneSession == nullptr) {
        TLOGE(WmsLogTag::DEFAULT, "sceneSession is nullptr");
        return WMError::WM_ERROR_NULLPTR;
    }
    if (property == nullptr) {
        TLOGE(WmsLogTag::DEFAULT, "property is nullptr");
        return WMError::WM_ERROR_NULLPTR;
    }

    return ProcessUpdatePropertyByAction(property, sceneSession, action);
}

WMError SceneSession::ProcessUpdatePropertyByAction(const sptr<WindowSessionProperty>& property,
    const sptr<SceneSession>& sceneSession, WSPropertyChangeAction action)
{
    switch (static_cast<uint32_t>(action)) {
        case static_cast<uint32_t>(WSPropertyChangeAction::ACTION_UPDATE_TURN_SCREEN_ON):
            return HandleActionUpdateTurnScreenOn(property, sceneSession, action);
        case static_cast<uint32_t>(WSPropertyChangeAction::ACTION_UPDATE_KEEP_SCREEN_ON):
            return HandleActionUpdateKeepScreenOn(property, sceneSession, action);
        case static_cast<uint32_t>(WSPropertyChangeAction::ACTION_UPDATE_FOCUSABLE):
            return HandleActionUpdateFocusable(property, sceneSession, action);
        case static_cast<uint32_t>(WSPropertyChangeAction::ACTION_UPDATE_TOUCHABLE):
            return HandleActionUpdateTouchable(property, sceneSession, action);
        case static_cast<uint32_t>(WSPropertyChangeAction::ACTION_UPDATE_SET_BRIGHTNESS):
            return HandleActionUpdateSetBrightness(property, sceneSession, action);
        case static_cast<uint32_t>(WSPropertyChangeAction::ACTION_UPDATE_ORIENTATION):
            return HandleActionUpdateOrientation(property, sceneSession, action);
        case static_cast<uint32_t>(WSPropertyChangeAction::ACTION_UPDATE_PRIVACY_MODE):
            return HandleActionUpdatePrivacyMode(property, sceneSession, action);
        case static_cast<uint32_t>(WSPropertyChangeAction::ACTION_UPDATE_SYSTEM_PRIVACY_MODE):
            return HandleActionUpdatePrivacyMode(property, sceneSession, action);
        case static_cast<uint32_t>(WSPropertyChangeAction::ACTION_UPDATE_SNAPSHOT_SKIP):
            return HandleActionUpdateSnapshotSkip(property, sceneSession, action);
        case static_cast<uint32_t>(WSPropertyChangeAction::ACTION_UPDATE_MAXIMIZE_STATE):
            return HandleActionUpdateMaximizeState(property, sceneSession, action);
        case static_cast<uint32_t>(WSPropertyChangeAction::ACTION_UPDATE_OTHER_PROPS):
            return HandleActionUpdateOtherProps(property, sceneSession, action);
        case static_cast<uint32_t>(WSPropertyChangeAction::ACTION_UPDATE_STATUS_PROPS):
            return HandleActionUpdateStatusProps(property, sceneSession, action);
        case static_cast<uint32_t>(WSPropertyChangeAction::ACTION_UPDATE_NAVIGATION_PROPS):
            return HandleActionUpdateNavigationProps(property, sceneSession, action);
        case static_cast<uint32_t>(WSPropertyChangeAction::ACTION_UPDATE_NAVIGATION_INDICATOR_PROPS):
            return HandleActionUpdateNavigationIndicatorProps(property, sceneSession, action);
        case static_cast<uint32_t>(WSPropertyChangeAction::ACTION_UPDATE_FLAGS):
            return HandleActionUpdateFlags(property, sceneSession, action);
        case static_cast<uint32_t>(WSPropertyChangeAction::ACTION_UPDATE_MODE):
            return HandleActionUpdateMode(property, sceneSession, action);
        case static_cast<uint32_t>(WSPropertyChangeAction::ACTION_UPDATE_ANIMATION_FLAG):
            return HandleActionUpdateAnimationFlag(property, sceneSession, action);
        case static_cast<uint32_t>(WSPropertyChangeAction::ACTION_UPDATE_TOUCH_HOT_AREA):
            return HandleActionUpdateTouchHotArea(property, sceneSession, action);
        case static_cast<uint32_t>(WSPropertyChangeAction::ACTION_UPDATE_DECOR_ENABLE):
            return HandleActionUpdateDecorEnable(property, sceneSession, action);
        case static_cast<uint32_t>(WSPropertyChangeAction::ACTION_UPDATE_WINDOW_LIMITS):
            return HandleActionUpdateWindowLimits(property, sceneSession, action);
        case static_cast<uint32_t>(WSPropertyChangeAction::ACTION_UPDATE_DRAGENABLED):
            return HandleActionUpdateDragenabled(property, sceneSession, action);
        case static_cast<uint32_t>(WSPropertyChangeAction::ACTION_UPDATE_RAISEENABLED):
            return HandleActionUpdateRaiseenabled(property, sceneSession, action);
        case static_cast<uint32_t>(WSPropertyChangeAction::ACTION_UPDATE_HIDE_NON_SYSTEM_FLOATING_WINDOWS):
            return HandleActionUpdateHideNonSystemFloatingWindows(property, sceneSession, action);
        case static_cast<uint32_t>(WSPropertyChangeAction::ACTION_UPDATE_TEXTFIELD_AVOID_INFO):
            return HandleActionUpdateTextfieldAvoidInfo(property, sceneSession, action);
        case static_cast<uint32_t>(WSPropertyChangeAction::ACTION_UPDATE_WINDOW_MASK):
            return HandleActionUpdateWindowMask(property, sceneSession, action);
        case static_cast<uint32_t>(WSPropertyChangeAction::ACTION_UPDATE_TOPMOST):
            return HandleActionUpdateTopmost(property, sceneSession, action);
        case static_cast<uint32_t>(WSPropertyChangeAction::ACTION_UPDATE_MODE_SUPPORT_INFO):
            return HandleActionUpdateModeSupportInfo(property, sceneSession, action);
        default:
            TLOGE(WmsLogTag::DEFAULT, "Failed to find func handler!");
            return WMError::WM_DO_NOTHING;
    }
}

WMError SceneSession::HandleActionUpdateTurnScreenOn(const sptr<WindowSessionProperty>& property,
    const sptr<SceneSession>& sceneSession, WSPropertyChangeAction action)
{
    sceneSession->SetTurnScreenOn(property->IsTurnScreenOn());
#ifdef POWER_MANAGER_ENABLE
    auto task = [this, sceneSession]() {
        if (sceneSession == nullptr) {
            TLOGE(WmsLogTag::DEFAULT, "session is invalid");
            return;
        }
        TLOGD(WmsLogTag::DEFAULT, "Win: %{public}s, is turn on: %{public}d",
            sceneSession->GetWindowName().c_str(), sceneSession->IsTurnScreenOn());
        std::string identity = IPCSkeleton::ResetCallingIdentity();
        if (sceneSession->IsTurnScreenOn()) {
            TLOGI(WmsLogTag::DEFAULT, "turn screen on");
            PowerMgr::PowerMgrClient::GetInstance().WakeupDevice();
        }
        // set ipc identity to raw
        IPCSkeleton::SetCallingIdentity(identity);
    };
    PostTask(task, "HandleTurnScreenOn");
#else
    TLOGD(WmsLogTag::DEFAULT, "Can not found the sub system of PowerMgr");
#endif
    return WMError::WM_OK;
}

WMError SceneSession::HandleActionUpdateKeepScreenOn(const sptr<WindowSessionProperty>& property,
    const sptr<SceneSession>& sceneSession, WSPropertyChangeAction action)
{
    sceneSession->SetKeepScreenOn(property->IsKeepScreenOn());
    sceneSession->NotifySessionChangeByActionNotifyManager(sceneSession, property, action);
    return WMError::WM_OK;
}

WMError SceneSession::HandleActionUpdateFocusable(const sptr<WindowSessionProperty>& property,
    const sptr<SceneSession>& sceneSession, WSPropertyChangeAction action)
{
    sceneSession->SetFocusable(property->GetFocusable());
    sceneSession->NotifySessionChangeByActionNotifyManager(sceneSession, property, action);
    return WMError::WM_OK;
}

WMError SceneSession::HandleActionUpdateTouchable(const sptr<WindowSessionProperty>& property,
    const sptr<SceneSession>& sceneSession, WSPropertyChangeAction action)
{
    sceneSession->SetTouchable(property->GetTouchable());
    sceneSession->NotifySessionChangeByActionNotifyManager(sceneSession, property, action);
    return WMError::WM_OK;
}

WMError SceneSession::HandleActionUpdateSetBrightness(const sptr<WindowSessionProperty>& property,
    const sptr<SceneSession>& sceneSession, WSPropertyChangeAction action)
{
    if (sceneSession->GetWindowType() != WindowType::WINDOW_TYPE_APP_MAIN_WINDOW) {
        TLOGW(WmsLogTag::DEFAULT, "only app main window can set brightness");
        return WMError::WM_OK;
    }
    if (!sceneSession->IsSessionValid()) {
        TLOGW(WmsLogTag::WMS_MAIN, "Session is invalid, id: %{public}d state: %{public}u",
            sceneSession->GetPersistentId(), sceneSession->GetSessionState());
        return WMError::WM_ERROR_INVALID_SESSION;
    }
    float brightness = property->GetBrightness();
    if (std::abs(brightness - sceneSession->GetBrightness()) < std::numeric_limits<float>::epsilon()) {
        TLOGD(WmsLogTag::DEFAULT, "Session brightness do not change: [%{public}f]", brightness);
        return WMError::WM_OK;
    }
    sceneSession->SetBrightness(brightness);
    sceneSession->NotifySessionChangeByActionNotifyManager(sceneSession, property, action);
    return WMError::WM_OK;
}

WMError SceneSession::HandleActionUpdateOrientation(const sptr<WindowSessionProperty>& property,
    const sptr<SceneSession>& sceneSession, WSPropertyChangeAction action)
{
    sceneSession->SetRequestedOrientation(property->GetRequestedOrientation());
    return WMError::WM_OK;
}

WMError SceneSession::HandleActionUpdatePrivacyMode(const sptr<WindowSessionProperty>& property,
    const sptr<SceneSession>& sceneSession, WSPropertyChangeAction action)
{
    bool isPrivacyMode = property->GetPrivacyMode() || property->GetSystemPrivacyMode();
    sceneSession->SetPrivacyMode(isPrivacyMode);
    sceneSession->NotifySessionChangeByActionNotifyManager(sceneSession, property, action);
    return WMError::WM_OK;
}

WMError SceneSession::HandleActionUpdateSnapshotSkip(const sptr<WindowSessionProperty>& property,
    const sptr<SceneSession>& sceneSession, WSPropertyChangeAction action)
{
    sceneSession->SetSnapshotSkip(property->GetSnapshotSkip());
    return WMError::WM_OK;
}

WMError SceneSession::HandleActionUpdateMaximizeState(const sptr<WindowSessionProperty>& property,
    const sptr<SceneSession>& sceneSession, WSPropertyChangeAction action)
{
    auto sessionProperty = sceneSession->GetSessionProperty();
    if (sessionProperty != nullptr) {
        sessionProperty->SetMaximizeMode(property->GetMaximizeMode());
        sessionProperty->SetIsLayoutFullScreen(property->IsLayoutFullScreen());
    }
    return WMError::WM_OK;
}

WMError SceneSession::HandleActionUpdateOtherProps(const sptr<WindowSessionProperty>& property,
    const sptr<SceneSession>& sceneSession, WSPropertyChangeAction action)
{
    auto systemBarProperties = property->GetSystemBarProperty();
    for (auto iter : systemBarProperties) {
        sceneSession->SetSystemBarProperty(iter.first, iter.second);
    }
    sceneSession->NotifySessionChangeByActionNotifyManager(sceneSession, property, action);
    return WMError::WM_OK;
}

WMError SceneSession::HandleActionUpdateStatusProps(const sptr<WindowSessionProperty>& property,
    const sptr<SceneSession>& sceneSession, WSPropertyChangeAction action)
{
    HandleSpecificSystemBarProperty(WindowType::WINDOW_TYPE_STATUS_BAR, property, sceneSession);
    sceneSession->NotifySessionChangeByActionNotifyManager(sceneSession, property, action);
    return WMError::WM_OK;
}

WMError SceneSession::HandleActionUpdateNavigationProps(const sptr<WindowSessionProperty>& property,
    const sptr<SceneSession>& sceneSession, WSPropertyChangeAction action)
{
    HandleSpecificSystemBarProperty(WindowType::WINDOW_TYPE_NAVIGATION_BAR, property, sceneSession);
    sceneSession->NotifySessionChangeByActionNotifyManager(sceneSession, property, action);
    return WMError::WM_OK;
}

WMError SceneSession::HandleActionUpdateNavigationIndicatorProps(const sptr<WindowSessionProperty>& property,
    const sptr<SceneSession>& sceneSession, WSPropertyChangeAction action)
{
    HandleSpecificSystemBarProperty(WindowType::WINDOW_TYPE_NAVIGATION_INDICATOR, property, sceneSession);
    sceneSession->NotifySessionChangeByActionNotifyManager(sceneSession, property, action);
    return WMError::WM_OK;
}

WMError SceneSession::HandleActionUpdateFlags(const sptr<WindowSessionProperty>& property,
    const sptr<SceneSession>& sceneSession, WSPropertyChangeAction action)
{
    SetWindowFlags(sceneSession, property);
    sceneSession->NotifySessionChangeByActionNotifyManager(sceneSession, property, action);
    return WMError::WM_OK;
}

WMError SceneSession::HandleActionUpdateMode(const sptr<WindowSessionProperty>& property,
    const sptr<SceneSession>& sceneSession, WSPropertyChangeAction action)
{
    auto sessionProperty = sceneSession->GetSessionProperty();
    if (sessionProperty != nullptr) {
        sessionProperty->SetWindowMode(property->GetWindowMode());
    }
    sceneSession->NotifySessionChangeByActionNotifyManager(sceneSession, property, action);
    return WMError::WM_OK;
}

WMError SceneSession::HandleActionUpdateAnimationFlag(const sptr<WindowSessionProperty>& property,
    const sptr<SceneSession>& sceneSession, WSPropertyChangeAction action)
{
    auto sessionProperty = sceneSession->GetSessionProperty();
    if (sessionProperty != nullptr) {
        sessionProperty->SetAnimationFlag(property->GetAnimationFlag());
    }
    return WMError::WM_OK;
}

WMError SceneSession::HandleActionUpdateTouchHotArea(const sptr<WindowSessionProperty>& property,
    const sptr<SceneSession>& sceneSession, WSPropertyChangeAction action)
{
    auto sessionProperty = sceneSession->GetSessionProperty();
    if (sessionProperty != nullptr) {
        std::vector<Rect> touchHotAreas;
        property->GetTouchHotAreas(touchHotAreas);
        sessionProperty->SetTouchHotAreas(touchHotAreas);
    }
    return WMError::WM_OK;
}

WMError SceneSession::HandleActionUpdateDecorEnable(const sptr<WindowSessionProperty>& property,
    const sptr<SceneSession>& sceneSession, WSPropertyChangeAction action)
{
    if (property != nullptr && !property->GetSystemCalling()) {
        TLOGE(WmsLogTag::DEFAULT, "update decor enable permission denied!");
        return WMError::WM_ERROR_NOT_SYSTEM_APP;
    }
    auto sessionProperty = sceneSession->GetSessionProperty();
    if (sessionProperty != nullptr) {
        sessionProperty->SetDecorEnable(property->IsDecorEnable());
    }
    return WMError::WM_OK;
}

WMError SceneSession::HandleActionUpdateWindowLimits(const sptr<WindowSessionProperty>& property,
    const sptr<SceneSession>& sceneSession, WSPropertyChangeAction action)
{
    auto sessionProperty = sceneSession->GetSessionProperty();
    if (sessionProperty != nullptr) {
        sessionProperty->SetWindowLimits(property->GetWindowLimits());
        WindowLimits windowLimits = sessionProperty->GetWindowLimits();
        TLOGI(WmsLogTag::WMS_LAYOUT, "UpdateWindowLimits minWidth:%{public}u, minHeight:%{public}u, "
            "maxWidth:%{public}u, maxHeight:%{public}u, vpRatio:%{public}f", windowLimits.minWidth_,
            windowLimits.minHeight_, windowLimits.maxWidth_, windowLimits.maxHeight_, windowLimits.vpRatio_);
    }
    return WMError::WM_OK;
}

WMError SceneSession::HandleActionUpdateDragenabled(const sptr<WindowSessionProperty>& property,
    const sptr<SceneSession>& sceneSession, WSPropertyChangeAction action)
{
    if (!property->GetSystemCalling()) {
        TLOGE(WmsLogTag::DEFAULT, "Update property dragEnabled permission denied!");
        return WMError::WM_ERROR_NOT_SYSTEM_APP;
    }

    auto sessionProperty = sceneSession->GetSessionProperty();
    if (sessionProperty != nullptr) {
        sessionProperty->SetDragEnabled(property->GetDragEnabled());
    }
    return WMError::WM_OK;
}

WMError SceneSession::HandleActionUpdateRaiseenabled(const sptr<WindowSessionProperty>& property,
    const sptr<SceneSession>& sceneSession, WSPropertyChangeAction action)
{
    if (!property->GetSystemCalling()) {
        TLOGE(WmsLogTag::DEFAULT, "Update property raiseEnabled permission denied!");
        return WMError::WM_ERROR_NOT_SYSTEM_APP;
    }

    auto sessionProperty = sceneSession->GetSessionProperty();
    if (sessionProperty != nullptr) {
        sessionProperty->SetRaiseEnabled(property->GetRaiseEnabled());
    }
    return WMError::WM_OK;
}

WMError SceneSession::HandleActionUpdateHideNonSystemFloatingWindows(const sptr<WindowSessionProperty>& property,
    const sptr<SceneSession>& sceneSession, WSPropertyChangeAction action)
{
    if (!SessionPermission::IsSystemCalling() && !SessionPermission::IsStartByHdcd()) {
        TLOGE(WmsLogTag::DEFAULT, "Update property hideNonSystemFloatingWindows permission denied!");
        return WMError::WM_OK;
    }
    auto currentProperty = sceneSession->GetSessionProperty();
    if (currentProperty != nullptr) {
        sceneSession->NotifySessionChangeByActionNotifyManager(sceneSession, property, action);
        currentProperty->SetHideNonSystemFloatingWindows(property->GetHideNonSystemFloatingWindows());
    }
    return WMError::WM_OK;
}

WMError SceneSession::HandleActionUpdateTextfieldAvoidInfo(const sptr<WindowSessionProperty>& property,
    const sptr<SceneSession>& sceneSession, WSPropertyChangeAction action)
{
    auto sessionProperty = sceneSession->GetSessionProperty();
    if (sessionProperty != nullptr) {
        sessionProperty->SetTextFieldPositionY(property->GetTextFieldPositionY());
        sessionProperty->SetTextFieldHeight(property->GetTextFieldHeight());
    }
    return WMError::WM_OK;
}

WMError SceneSession::HandleActionUpdateWindowMask(const sptr<WindowSessionProperty>& property,
    const sptr<SceneSession>& sceneSession, WSPropertyChangeAction action)
{
    auto sessionProperty = sceneSession->GetSessionProperty();
    if (sessionProperty != nullptr) {
        sessionProperty->SetWindowMask(property->GetWindowMask());
        sessionProperty->SetIsShaped(property->GetIsShaped());
        sceneSession->NotifySessionChangeByActionNotifyManager(sceneSession, property, action);
    }
    return WMError::WM_OK;
}

WMError SceneSession::HandleActionUpdateTopmost(const sptr<WindowSessionProperty>& property,
    const sptr<SceneSession>& sceneSession, WSPropertyChangeAction action)
{
    if (!SessionPermission::IsSystemCalling()) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "UpdateTopmostProperty permission denied!");
        return WMError::WM_ERROR_NOT_SYSTEM_APP;
    }

    sceneSession->SetTopmost(property->IsTopmost());
    return WMError::WM_OK;
}

void SceneSession::HandleSpecificSystemBarProperty(WindowType type,
    const sptr<WindowSessionProperty>& property, const sptr<SceneSession>& sceneSession)
{
    auto systemBarProperties = property->GetSystemBarProperty();
    if (auto iter = systemBarProperties.find(type); iter != systemBarProperties.end()) {
        if (GetIsDisplayStatusBarTemporarily() && specificCallback_ && specificCallback_->onUpdateAvoidArea_) {
            SetIsDisplayStatusBarTemporarily(false);
            if (Session::IsScbCoreEnabled()) {
                dirtyFlags_ |= static_cast<uint32_t>(SessionUIDirtyFlag::AVOID_AREA);
            } else {
                specificCallback_->onUpdateAvoidArea_(GetPersistentId());
            }
        }
        SetSystemBarProperty(iter->first, iter->second);
        TLOGD(WmsLogTag::WMS_IMMS, "%{public}d, enable: %{public}d",
            static_cast<int32_t>(iter->first), iter->second.enable_);
    }
}

void SceneSession::SetWindowFlags(const sptr<SceneSession>& sceneSession,
    const sptr<WindowSessionProperty>& property)
{
    if (sceneSession == nullptr) {
        TLOGD(WmsLogTag::DEFAULT, "session is nullptr");
        return;
    }
    auto sessionProperty = sceneSession->GetSessionProperty();
    if (sessionProperty == nullptr) {
        TLOGE(WmsLogTag::DEFAULT, "get session property failed");
        return;
    }
    uint32_t flags = property->GetWindowFlags();
    uint32_t oldFlags = sessionProperty->GetWindowFlags();
    if (((oldFlags ^ flags) == static_cast<uint32_t>(WindowFlag::WINDOW_FLAG_SHOW_WHEN_LOCKED) ||
         (oldFlags ^ flags) == static_cast<uint32_t>(WindowFlag::WINDOW_FLAG_WATER_MARK)) &&
        !property->GetSystemCalling()) {
        TLOGE(WmsLogTag::DEFAULT, "Set window flags permission denied");
        return;
    }
    sessionProperty->SetWindowFlags(flags);
    if ((oldFlags ^ flags) == static_cast<uint32_t>(WindowFlag::WINDOW_FLAG_SHOW_WHEN_LOCKED)) {
        sceneSession->OnShowWhenLocked(flags & static_cast<uint32_t>(WindowFlag::WINDOW_FLAG_SHOW_WHEN_LOCKED));
    }
    TLOGI(WmsLogTag::DEFAULT, "flags: %{public}u", flags);
}

void SceneSession::NotifySessionChangeByActionNotifyManager(const sptr<SceneSession>& sceneSession,
    const sptr<WindowSessionProperty>& property, WSPropertyChangeAction action)
{
    TLOGD(WmsLogTag::DEFAULT, "id: %{public}d, action: %{public}d",
        GetPersistentId(), action);
    if (sessionChangeByActionNotifyManagerFunc_ == nullptr) {
        TLOGW(WmsLogTag::DEFAULT, "func is null");
        return;
    }
    sessionChangeByActionNotifyManagerFunc_(sceneSession, property, action);
}

WSError SceneSession::TerminateSession(const sptr<AAFwk::SessionInfo> abilitySessionInfo)
{
    auto task = [weakThis = wptr(this), abilitySessionInfo]() {
        auto session = weakThis.promote();
        if (!session) {
            TLOGE(WmsLogTag::WMS_LIFE, "session is null");
            return WSError::WS_ERROR_DESTROYED_OBJECT;
        }
        if (abilitySessionInfo == nullptr) {
            TLOGE(WmsLogTag::WMS_LIFE, "abilitySessionInfo is null");
            return WSError::WS_ERROR_NULLPTR;
        }
        if (session->isTerminating_) {
            TLOGE(WmsLogTag::WMS_LIFE, "TerminateSession: is terminating, return!");
            return WSError::WS_ERROR_INVALID_OPERATION;
        }
        session->isTerminating_ = true;
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
    };
    PostLifeCycleTask(task, "TerminateSession", LifeCycleTaskType::STOP);
    return WSError::WS_OK;
}

WSError SceneSession::NotifySessionExceptionInner(const sptr<AAFwk::SessionInfo> abilitySessionInfo,
    bool needRemoveSession, bool isFromClient, bool startFail)
{
    auto task = [weakThis = wptr(this), abilitySessionInfo, needRemoveSession, isFromClient, startFail]() {
        auto session = weakThis.promote();
        if (!session) {
            TLOGE(WmsLogTag::WMS_LIFE, "session is null");
            return WSError::WS_ERROR_DESTROYED_OBJECT;
        }
        if (abilitySessionInfo == nullptr) {
            TLOGE(WmsLogTag::WMS_LIFE, "abilitySessionInfo is null");
            return WSError::WS_ERROR_NULLPTR;
        }
        if (SessionHelper::IsMainWindow(session->GetWindowType()) && isFromClient &&
            !session->clientIdentityToken_.empty() &&
            session->clientIdentityToken_ != abilitySessionInfo->identityToken) {
            TLOGE(WmsLogTag::WMS_LIFE, "client exception not matched: %{public}s, %{public}s",
                session->clientIdentityToken_.c_str(), abilitySessionInfo->identityToken.c_str());
            return WSError::WS_ERROR_INVALID_PARAM;
        }
        if (session->isTerminating_) {
            TLOGE(WmsLogTag::WMS_LIFE, "NotifySessionExceptionInner: is terminating, return!");
            return WSError::WS_ERROR_INVALID_OPERATION;
        }
        session->isTerminating_ = true;
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
        if (session->sessionExceptionFunc_) {
            session->sessionExceptionFunc_(info, needRemoveSession, false);
        }
        if (session->jsSceneSessionExceptionFunc_) {
            session->jsSceneSessionExceptionFunc_(info, needRemoveSession, startFail);
        }
        return WSError::WS_OK;
    };
    PostLifeCycleTask(task, "NotifySessionExceptionInner", LifeCycleTaskType::STOP);
    return WSError::WS_OK;
}

WSError SceneSession::NotifySessionException(const sptr<AAFwk::SessionInfo> abilitySessionInfo, bool needRemoveSession)
{
    if (!SessionPermission::VerifyCallingPermission(PermissionConstants::PERMISSION_MANAGE_MISSION)) {
        TLOGE(WmsLogTag::WMS_LIFE, "permission failed.");
        return WSError::WS_ERROR_INVALID_PERMISSION;
    }
    return NotifySessionExceptionInner(abilitySessionInfo, needRemoveSession, true);
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

int32_t SceneSession::GetOriPosYBeforeRaisedByKeyboard() const
{
    return oriPosYBeforeRaisedByKeyboard_;
}

void SceneSession::SetOriPosYBeforeRaisedByKeyboard(int32_t posY)
{
    oriPosYBeforeRaisedByKeyboard_ = posY;
}

bool SceneSession::AddSubSession(const sptr<SceneSession>& subSession)
{
    if (subSession == nullptr) {
        TLOGE(WmsLogTag::WMS_SUB, "subSession is nullptr");
        return false;
    }
    const auto& persistentId = subSession->GetPersistentId();
    auto iter = std::find_if(subSession_.begin(), subSession_.end(),
        [persistentId](sptr<SceneSession> session) {
            bool res = (session != nullptr && session->GetPersistentId() == persistentId) ? true : false;
            return res;
        });
    if (iter != subSession_.end()) {
        TLOGE(WmsLogTag::WMS_SUB, "Sub ession is already exists, id: %{public}d, parentId: %{public}d",
            subSession->GetPersistentId(), GetPersistentId());
        return false;
    }
    TLOGD(WmsLogTag::WMS_SUB, "Success, id: %{public}d, parentId: %{public}d",
        subSession->GetPersistentId(), GetPersistentId());
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
        TLOGE(WmsLogTag::WMS_SUB, "Could not find subsession, id: %{public}d, parentId: %{public}d",
            persistentId, GetPersistentId());
        return false;
    }
    TLOGD(WmsLogTag::WMS_SUB, "Success, id: %{public}d, parentId: %{public}d", persistentId, GetPersistentId());
    subSession_.erase(iter);
    return true;
}

bool SceneSession::AddToastSession(const sptr<SceneSession>& toastSession)
{
    if (toastSession == nullptr) {
        TLOGE(WmsLogTag::WMS_TOAST, "toastSession is nullptr");
        return false;
    }
    const auto& persistentId = toastSession->GetPersistentId();
    auto iter = std::find_if(toastSession_.begin(), toastSession_.end(),
        [persistentId](sptr<SceneSession> session) {
            bool res = (session != nullptr && session->GetPersistentId() == persistentId) ? true : false;
            return res;
        });
    if (iter != toastSession_.end()) {
        TLOGE(WmsLogTag::WMS_TOAST, "Toast ession is already exists, id: %{public}d, parentId: %{public}d",
            toastSession->GetPersistentId(), GetPersistentId());
        return false;
    }
    TLOGD(WmsLogTag::WMS_TOAST, "Success, id: %{public}d, parentId: %{public}d",
        toastSession->GetPersistentId(), GetPersistentId());
    toastSession_.push_back(toastSession);
    return true;
}

bool SceneSession::RemoveToastSession(int32_t persistentId)
{
    auto iter = std::find_if(toastSession_.begin(), toastSession_.end(),
        [persistentId](sptr<SceneSession> session) {
            bool res = (session != nullptr && session->GetPersistentId() == persistentId) ? true : false;
            return res;
        });
    if (iter == toastSession_.end()) {
        TLOGE(WmsLogTag::WMS_TOAST, "Could not find toastSession, id: %{public}d, parentId: %{public}d",
            persistentId, GetPersistentId());
        return false;
    }
    TLOGD(WmsLogTag::WMS_TOAST, "Success, id: %{public}d, parentId: %{public}d", persistentId, GetPersistentId());
    toastSession_.erase(iter);
    return true;
}

void SceneSession::NotifyPiPWindowPrepareClose()
{
    TLOGD(WmsLogTag::WMS_PIP, "NotifyPiPWindowPrepareClose");
    int32_t callingPid = IPCSkeleton::GetCallingPid();
    auto task = [weakThis = wptr(this), callingPid]() {
        auto session = weakThis.promote();
        if (!session) {
            TLOGE(WmsLogTag::WMS_PIP, "session is null");
            return;
        }
        if (callingPid != session->GetCallingPid()) {
            TLOGW(WmsLogTag::WMS_PIP, "permission denied, not call by the same process");
            return;
        }
        if (session->onPrepareClosePiPSession_) {
            session->onPrepareClosePiPSession_();
        }
        TLOGD(WmsLogTag::WMS_PIP, "NotifyPiPWindowPrepareClose, id: %{public}d", session->GetPersistentId());
        return;
    };
    PostTask(task, "NotifyPiPWindowPrepareClose");
}

WSError SceneSession::SetLandscapeMultiWindow(bool isLandscapeMultiWindow)
{
    TLOGD(WmsLogTag::WMS_MULTI_WINDOW, "NotifySetLandscapeMultiWindow");
    int32_t callingPid = IPCSkeleton::GetCallingPid();
    auto task = [weakThis = wptr(this), isLandscapeMultiWindow, callingPid]() {
        auto session = weakThis.promote();
        if (!session) {
            TLOGE(WmsLogTag::WMS_MULTI_WINDOW, "session is null");
            return WSError::WS_ERROR_DESTROYED_OBJECT;
        }
        if (callingPid != session->GetCallingPid()) {
            TLOGE(WmsLogTag::WMS_MULTI_WINDOW, "premission denied, not call by the same process");
            return WSError::WS_ERROR_INVALID_PERMISSION;
        }
        if (session->sessionChangeCallback_ &&
            session->sessionChangeCallback_->onSetLandscapeMultiWindowFunc_) {
            session->sessionChangeCallback_->onSetLandscapeMultiWindowFunc_(
                isLandscapeMultiWindow);
        }
        TLOGD(WmsLogTag::WMS_MULTI_WINDOW, "NotifySetLandscapeMultiWindow, id: %{public}d,"
            "isLandscapeMultiWindow: %{public}u", session->GetPersistentId(), isLandscapeMultiWindow);
        return WSError::WS_OK;
    };
    PostTask(task, "NotifySetLandscapeMultiWindow");
    return WSError::WS_OK;
}

std::vector<sptr<SceneSession>> SceneSession::GetSubSession() const
{
    return subSession_;
}

std::vector<sptr<SceneSession>> SceneSession::GetToastSession() const
{
    return toastSession_;
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

void SceneSession::NotifySessionFullScreen(bool fullScreen)
{
    if (!sessionStage_) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "sessionStage is null");
        return;
    }
    sessionStage_->NotifySessionFullScreen(fullScreen);
}

WSError SceneSession::UpdatePiPRect(const Rect& rect, SizeChangeReason reason)
{
    if (!WindowHelper::IsPipWindow(GetWindowType())) {
        return WSError::WS_DO_NOTHING;
    }
    int32_t callingPid = IPCSkeleton::GetCallingPid();
    auto task = [weakThis = wptr(this), rect, reason, callingPid]() {
        auto session = weakThis.promote();
        if (!session || session->isTerminating_) {
            TLOGE(WmsLogTag::WMS_PIP, "SceneSession::UpdatePiPRect session is null or is terminating");
            return WSError::WS_ERROR_INVALID_OPERATION;
        }
        if (callingPid != session->GetCallingPid()) {
            TLOGW(WmsLogTag::WMS_PIP, "permission denied, not call by the same process");
            return WSError::WS_ERROR_INVALID_PERMISSION;
        }
        WSRect wsRect = SessionHelper::TransferToWSRect(rect);
        if (reason == SizeChangeReason::PIP_START) {
            session->SetSessionRequestRect(wsRect);
        }
        TLOGI(WmsLogTag::WMS_PIP, "rect:%{public}s, reason: %{public}u", wsRect.ToString().c_str(),
            static_cast<uint32_t>(reason));
        session->NotifySessionRectChange(wsRect, reason);
        return WSError::WS_OK;
    };
    if (mainHandler_ != nullptr) {
        mainHandler_->PostTask(std::move(task), "wms:UpdatePiPRect", 0, AppExecFwk::EventQueue::Priority::IMMEDIATE);
    } else {
        PostTask(task, "UpdatePiPRect");
    }
    return WSError::WS_OK;
}

WSError SceneSession::UpdatePiPControlStatus(WsPiPControlType controlType, WsPiPControlStatus status)
{
    TLOGI(WmsLogTag::WMS_PIP, "controlType:%{public}u, status:%{public}d", controlType, status);
    if (!WindowHelper::IsPipWindow(GetWindowType())) {
        return WSError::WS_DO_NOTHING;
    }
    int32_t callingPid = IPCSkeleton::GetCallingPid();
    auto task = [weakThis = wptr(this), controlType, status, callingPid]() {
        auto session = weakThis.promote();
        if (!session || session->isTerminating_) {
            TLOGE(WmsLogTag::WMS_PIP, "session is null or is terminating");
            return WSError::WS_ERROR_INVALID_OPERATION;
        }
        if (callingPid != session->GetCallingPid()) {
            TLOGW(WmsLogTag::WMS_PIP, "permission denied, not call by the same process");
            return WSError::WS_ERROR_INVALID_PERMISSION;
        }
        if (session->sessionPiPControlStatusChangeFunc_) {
            HITRACE_METER_FMT(HITRACE_TAG_WINDOW_MANAGER, "SceneSession::UpdatePiPControlStatus");
            session->sessionPiPControlStatusChangeFunc_(controlType, status);
        }
        return WSError::WS_OK;
    };
    PostTask(task, "UpdatePiPControlStatus");
    return WSError::WS_OK;
}

WSError SceneSession::SetAutoStartPiP(bool isAutoStart, uint32_t priority)
{
    TLOGI(WmsLogTag::WMS_PIP, "isAutoStart:%{public}u priority:%{public}u", isAutoStart, priority);
    auto task = [weakThis = wptr(this), isAutoStart, priority] {
        auto session = weakThis.promote();
        if (!session || session->isTerminating_) {
            TLOGNE(WmsLogTag::WMS_PIP, "session is null or is terminating");
            return;
        }
        if (session->autoStartPiPStatusChangeFunc_) {
            HITRACE_METER_FMT(HITRACE_TAG_WINDOW_MANAGER, "SceneSession::SetAutoStartPiP");
            session->autoStartPiPStatusChangeFunc_(isAutoStart, priority);
        }
    };
    PostTask(task, __func__);
    return WSError::WS_OK;
}

void SceneSession::SendPointerEventToUI(std::shared_ptr<MMI::PointerEvent> pointerEvent)
{
    NotifySystemSessionPointerEventFunc systemSessionPointerEventFunc = nullptr;
    {
        std::lock_guard<std::mutex> lock(pointerEventMutex_);
        systemSessionPointerEventFunc = systemSessionPointerEventFunc_;
    }
    if (systemSessionPointerEventFunc != nullptr) {
        systemSessionPointerEventFunc(pointerEvent);
    } else {
        TLOGE(WmsLogTag::WMS_EVENT, "PointerEventFunc_ nullptr, id:%{public}d", pointerEvent->GetId());
        pointerEvent->MarkProcessed();
    }
}

bool SceneSession::SendKeyEventToUI(std::shared_ptr<MMI::KeyEvent> keyEvent, bool isPreImeEvent)
{
    NotifySystemSessionKeyEventFunc systemSessionKeyEventFunc = nullptr;
    {
        std::shared_lock<std::shared_mutex> lock(keyEventMutex_);
        systemSessionKeyEventFunc = systemSessionKeyEventFunc_;
    }
    if (systemSessionKeyEventFunc != nullptr) {
        return systemSessionKeyEventFunc(keyEvent, isPreImeEvent);
    } else {
        TLOGE(WmsLogTag::WMS_EVENT, "id:%{public}d systemSessionKeyEventFunc_ is null", keyEvent->GetId());
        keyEvent->MarkProcessed();
    }
    return false;
}

WSError SceneSession::UpdateSizeChangeReason(SizeChangeReason reason)
{
    auto task = [weakThis = wptr(this), reason]() {
        auto session = weakThis.promote();
        if (!session) {
            WLOGFE("session is null");
            return WSError::WS_ERROR_DESTROYED_OBJECT;
        }
        session->reason_ = reason;
        if (reason != SizeChangeReason::UNDEFINED) {
            HITRACE_METER_FMT(HITRACE_TAG_WINDOW_MANAGER,
                "SceneSession::UpdateSizeChangeReason%d reason:%d",
                session->GetPersistentId(), static_cast<uint32_t>(reason));
            TLOGD(WmsLogTag::WMS_LAYOUT, "UpdateSizeChangeReason Id: %{public}d, reason: %{public}d",
                session->GetPersistentId(), reason);
        }
        return WSError::WS_OK;
    };
    PostTask(task, "UpdateSizeChangeReason");
    return WSError::WS_OK;
}

void SceneSession::ResetSizeChangeReasonIfDirty()
{
    if (IsDirtyWindow() && GetSizeChangeReason() != SizeChangeReason::DRAG) {
        UpdateSizeChangeReason(SizeChangeReason::UNDEFINED);
    }
}

bool SceneSession::IsDirtyWindow()
{
    return dirtyFlags_ & static_cast<uint32_t>(SessionUIDirtyFlag::RECT);
}

void SceneSession::NotifyUILostFocus()
{
    if (moveDragController_) {
        moveDragController_->OnLostFocus();
    }
    Session::NotifyUILostFocus();
}

void SceneSession::SetScale(float scaleX, float scaleY, float pivotX, float pivotY)
{
    if (scaleX_ != scaleX || scaleY_ != scaleY || pivotX_ != pivotX || pivotY_ != pivotY) {
        Session::SetScale(scaleX, scaleY, pivotX, pivotY);
        if (specificCallback_ != nullptr) {
            specificCallback_->onWindowInfoUpdate_(GetPersistentId(), WindowUpdateType::WINDOW_UPDATE_PROPERTY);
        }
        if (sessionStage_ != nullptr) {
            Transform transform;
            transform.scaleX_ = scaleX;
            transform.scaleY_ = scaleY;
            transform.pivotX_ = pivotX;
            transform.pivotY_ = pivotY;
            sessionStage_->NotifyTransformChange(transform);
        } else {
            WLOGFE("sessionStage_ is nullptr");
        }
    }
}

void SceneSession::RequestHideKeyboard(bool isAppColdStart)
{
#ifdef IMF_ENABLE
    auto task = [weakThis = wptr(this), isAppColdStart]() {
        auto session = weakThis.promote();
        if (!session) {
            TLOGE(WmsLogTag::WMS_KEYBOARD, "Session is null, notify inputMethod framework hide keyboard failed!");
            return;
        }
        TLOGI(WmsLogTag::WMS_KEYBOARD, "Notify inputMethod framework hide keyboard start, id: %{public}d,"
            "isAppColdStart: %{public}d", session->GetPersistentId(), isAppColdStart);
        if (MiscServices::InputMethodController::GetInstance()) {
            MiscServices::InputMethodController::GetInstance()->RequestHideInput();
            TLOGI(WmsLogTag::WMS_KEYBOARD, "Notify inputMethod framework hide keyboard end, id: %{public}d",
                session->GetPersistentId());
        }
    };
    PostExportTask(task, "RequestHideKeyboard");
#endif
}

bool SceneSession::IsStartMoving() const
{
    return isStartMoving_.load();
}

void SceneSession::SetIsStartMoving(const bool startMoving)
{
    isStartMoving_.store(startMoving);
}

void SceneSession::SetShouldHideNonSecureWindows(bool shouldHide)
{
    shouldHideNonSecureWindows_.store(shouldHide);
}

void SceneSession::CalculateCombinedExtWindowFlags()
{
    // Only correct when each flag is true when active, and once a uiextension is active, the host is active
    std::unique_lock<std::shared_mutex> lock(combinedExtWindowFlagsMutex_);
    combinedExtWindowFlags_.bitData = 0;
    for (const auto& iter: extWindowFlagsMap_) {
        combinedExtWindowFlags_.bitData |= iter.second.bitData;
    }
}

void SceneSession::UpdateExtWindowFlags(int32_t extPersistentId, const ExtensionWindowFlags& extWindowFlags,
    const ExtensionWindowFlags& extWindowActions)
{
    auto iter = extWindowFlagsMap_.find(extPersistentId);
    // Each flag is false when inactive, 0 means all flags are inactive
    auto oldFlags = iter != extWindowFlagsMap_.end() ? iter->second : ExtensionWindowFlags();
    ExtensionWindowFlags newFlags((extWindowFlags.bitData & extWindowActions.bitData) |
        (oldFlags.bitData & ~extWindowActions.bitData));
    if (newFlags.bitData == 0) {
        extWindowFlagsMap_.erase(extPersistentId);
    } else {
        extWindowFlagsMap_[extPersistentId] = newFlags;
    }
    CalculateCombinedExtWindowFlags();
}

ExtensionWindowFlags SceneSession::GetCombinedExtWindowFlags()
{
    std::shared_lock<std::shared_mutex> lock(combinedExtWindowFlagsMutex_);
    auto combinedExtWindowFlags = combinedExtWindowFlags_;
    combinedExtWindowFlags.hideNonSecureWindowsFlag = IsSessionForeground() &&
        (combinedExtWindowFlags.hideNonSecureWindowsFlag || shouldHideNonSecureWindows_.load());
    return combinedExtWindowFlags;
}

void SceneSession::NotifyDisplayMove(DisplayId from, DisplayId to)
{
    if (sessionStage_) {
        sessionStage_->NotifyDisplayMove(from, to);
    } else {
        WLOGFE("Notify display move failed, sessionStage is null");
    }
}

void SceneSession::RemoveExtWindowFlags(int32_t extPersistentId)
{
    extWindowFlagsMap_.erase(extPersistentId);
    CalculateCombinedExtWindowFlags();
}

void SceneSession::ClearExtWindowFlags()
{
    std::unique_lock<std::shared_mutex> lock(combinedExtWindowFlagsMutex_);
    extWindowFlagsMap_.clear();
    combinedExtWindowFlags_.bitData = 0;
}

WSError SceneSession::UpdateRectChangeListenerRegistered(bool isRegister)
{
    auto task = [weakThis = wptr(this), isRegister]() {
        auto session = weakThis.promote();
        if (!session) {
            TLOGE(WmsLogTag::WMS_LAYOUT, "session is null");
            return WSError::WS_ERROR_DESTROYED_OBJECT;
        }
        session->rectChangeListenerRegistered_ = isRegister;
        return WSError::WS_OK;
    };
    PostTask(task, "UpdateRectChangeListenerRegistered");
    return WSError::WS_OK;
}

WSError SceneSession::OnLayoutFullScreenChange(bool isLayoutFullScreen)
{
    auto task = [weakThis = wptr(this), isLayoutFullScreen]() {
        auto session = weakThis.promote();
        if (!session) {
            TLOGE(WmsLogTag::WMS_LAYOUT, "session is null");
            return WSError::WS_ERROR_DESTROYED_OBJECT;
        }
        TLOGI(WmsLogTag::WMS_LAYOUT, "OnLayoutFullScreenChange, isLayoutFullScreen: %{public}d",
            isLayoutFullScreen);
        if (session->sessionChangeCallback_ && session->sessionChangeCallback_->onLayoutFullScreenChangeFunc_) {
            session->sessionChangeCallback_->onLayoutFullScreenChangeFunc_(isLayoutFullScreen);
        }
        return WSError::WS_OK;
    };
    PostTask(task, "OnLayoutFullScreenChange");
    return WSError::WS_OK;
}

WSError SceneSession::OnDefaultDensityEnabled(bool isDefaultDensityEnabled)
{
    auto task = [weakThis = wptr(this), isDefaultDensityEnabled] {
        auto session = weakThis.promote();
        if (!session) {
            TLOGNE(WmsLogTag::WMS_LAYOUT, "OnDefaultDensityEnabled session is null");
            return;
        }
        TLOGNI(WmsLogTag::WMS_LAYOUT, "OnDefaultDensityEnabled, isDefaultDensityEnabled: %{public}d",
            isDefaultDensityEnabled);
        if (session->onDefaultDensityEnabledFunc_) {
            session->onDefaultDensityEnabledFunc_(isDefaultDensityEnabled);
        }
    };
    PostTask(task, "OnDefaultDensityEnabled");
    return WSError::WS_OK;
}

void SceneSession::SetForceHideState(ForceHideState forceHideState)
{
    forceHideState_ = forceHideState;
}

ForceHideState SceneSession::GetForceHideState() const
{
    return forceHideState_;
}

void SceneSession::SetIsDisplayStatusBarTemporarily(bool isTemporary)
{
    isDisplayStatusBarTemporarily_.store(isTemporary);
}

bool SceneSession::GetIsDisplayStatusBarTemporarily() const
{
    return isDisplayStatusBarTemporarily_.load();
}

bool SceneSession::IsDeviceWakeupByApplication() const
{
    return isDeviceWakeupByApplication_.load();
}

void SceneSession::SetIsLastFrameLayoutFinishedFunc(IsLastFrameLayoutFinishedFunc&& func)
{
    isLastFrameLayoutFinishedFunc_ = std::move(func);
}

void SceneSession::SetStartingWindowExitAnimationFlag(bool enable)
{
    TLOGI(WmsLogTag::DEFAULT, "SetStartingWindowExitAnimationFlag %{public}d", enable);
    needStartingWindowExitAnimation_.store(enable);
}

bool SceneSession::NeedStartingWindowExitAnimation() const
{
    return needStartingWindowExitAnimation_.load();
}

bool SceneSession::IsSystemSpecificSession() const
{
    return isSystemSpecificSession_;
}

void SceneSession::SetIsSystemSpecificSession(bool isSystemSpecificSession)
{
    isSystemSpecificSession_ = isSystemSpecificSession;
}

void SceneSession::SetTemporarilyShowWhenLocked(bool isTemporarilyShowWhenLocked)
{
    if (isTemporarilyShowWhenLocked_.load() == isTemporarilyShowWhenLocked) {
        return;
    }
    isTemporarilyShowWhenLocked_.store(isTemporarilyShowWhenLocked);
    TLOGI(WmsLogTag::WMS_SCB, "SetTemporarilyShowWhenLocked successfully, target:%{public}u",
        isTemporarilyShowWhenLocked);
}

bool SceneSession::IsTemporarilyShowWhenLocked() const
{
    return isTemporarilyShowWhenLocked_.load();
}

void SceneSession::SetSkipDraw(bool skip)
{
    if (!surfaceNode_) {
        WLOGFE("surfaceNode_ is null");
        return;
    }
    auto rsTransaction = RSTransactionProxy::GetInstance();
    if (rsTransaction != nullptr) {
        rsTransaction->Begin();
    }
    surfaceNode_->SetSkipDraw(skip);
    auto leashWinSurfaceNode = GetLeashWinSurfaceNode();
    if (leashWinSurfaceNode != nullptr) {
        leashWinSurfaceNode->SetSkipDraw(skip);
    }
    if (rsTransaction != nullptr) {
        rsTransaction->Commit();
    }
}

void SceneSession::SetSkipSelfWhenShowOnVirtualScreen(bool isSkip)
{
    TLOGW(WmsLogTag::WMS_SCB, "in sceneSession, do nothing");
    return;
}

WMError SceneSession::SetUniqueDensityDpi(bool useUnique, float dpi)
{
    TLOGI(WmsLogTag::DEFAULT, "SceneSession set unique dpi: id = %{public}d, dpi = %{public}f",
        GetPersistentId(), dpi);
    if (useUnique && (dpi > DOT_PER_INCH_MAXIMUM_VALUE || dpi < DOT_PER_INCH_MINIMUM_VALUE)) {
        TLOGE(WmsLogTag::DEFAULT, "Invalid input dpi value, valid input range for DPI is %{public}u ~ %{public}u",
            DOT_PER_INCH_MINIMUM_VALUE, DOT_PER_INCH_MAXIMUM_VALUE);
        return WMError::WM_ERROR_INVALID_PARAM;
    }
    float density = static_cast<float>(dpi) / 160; // 160 is the coefficient between density and dpi;
    if (!IsSessionValid()) {
        return WMError::WM_ERROR_INVALID_SESSION;
    }
    if (!sessionStage_) {
        TLOGE(WmsLogTag::DEFAULT, "sessionStage_ is nullptr");
        return WMError::WM_ERROR_NULLPTR;
    }
    sessionStage_->SetUniqueVirtualPixelRatio(useUnique, density);
    return WMError::WM_OK;
}

WMError SceneSession::HandleActionUpdateModeSupportInfo(const sptr<WindowSessionProperty>& property,
    const sptr<SceneSession>& sceneSession, WSPropertyChangeAction action)
{
    if (!property->GetSystemCalling()) {
        TLOGE(WmsLogTag::DEFAULT, "Update property modeSupportInfo permission denied!");
        return WMError::WM_ERROR_NOT_SYSTEM_APP;
    }

    auto sessionProperty = sceneSession->GetSessionProperty();
    if (sessionProperty != nullptr) {
        sessionProperty->SetModeSupportInfo(property->GetModeSupportInfo());
    }
    return WMError::WM_OK;
}

void SceneSession::RegisterForceSplitListener(const NotifyForceSplitFunc& func)
{
    forceSplitFunc_ = func;
}

void SceneSession::RegisterRequestedOrientationChangeCallback(NotifyReqOrientationChangeFunc&& callback)
{
    auto task = [weakThis = wptr(this), callback = std::move(callback)] {
        auto session = weakThis.promote();
        if (!session) {
            TLOGNE(WmsLogTag::WMS_LIFE, "session is null");
            return;
        }
        session->onRequestedOrientationChange_ = std::move(callback);
    };
    PostTask(task, __func__);
}

void SceneSession::RegisterIsCustomAnimationPlayingCallback(NotifyIsCustomAnimationPlayingCallback&& callback)
{
    auto task = [weakThis = wptr(this), callback = std::move(callback)] {
        auto session = weakThis.promote();
        if (!session) {
            TLOGNE(WmsLogTag::WMS_LIFE, "session is null");
            return;
        }
        session->onIsCustomAnimationPlaying_ = std::move(callback);
    };
    PostTask(task, __func__);
}

WMError SceneSession::GetAppForceLandscapeConfig(AppForceLandscapeConfig& config)
{
    if (forceSplitFunc_ == nullptr) {
        return WMError::WM_ERROR_NULLPTR;
    }
    config = forceSplitFunc_(sessionInfo_.bundleName_);
    return WMError::WM_OK;
}

bool SceneSession::CheckPermissionWithPropertyAnimation(const sptr<WindowSessionProperty>& property) const
{
    if (property && property->GetAnimationFlag() == static_cast<uint32_t>(WindowAnimation::CUSTOM)) {
        if (!SessionPermission::IsSystemCalling() && !SessionPermission::IsStartByHdcd()) {
            TLOGE(WmsLogTag::WMS_LIFE, "Not system app, no permission");
            return false;
        }
    }
    return true;
}

void SceneSession::SetNotifyVisibleChangeFunc(const NotifyVisibleChangeFunc& func)
{
    notifyVisibleChangeFunc_ = func;
}

void SceneSession::SetUpdatePrivateStateAndNotifyFunc(const UpdatePrivateStateAndNotifyFunc& func)
{
    updatePrivateStateAndNotifyFunc_ = func;
}

bool SceneSession::IsPcOrPadEnableActivation() const
{
    auto isPC = system::GetParameter("const.product.devicetype", "unknown") == "2in1";
    auto property = GetSessionProperty();
    bool isPcAppInPad = false;
    if (property != nullptr) {
        isPcAppInPad = property->GetIsPcAppInPad();
    }
    return isPC || IsFreeMultiWindowMode() || isPcAppInPad;
}

void SceneSession::UnregisterSessionChangeListeners()
{
    auto task = [weakThis = wptr(this)] {
        auto session = weakThis.promote();
        if (session == nullptr) {
            WLOGFE("UnregisterSessionChangeListeners session is null");
            return;
        }
        if (session->sessionChangeCallback_) {
            session->sessionChangeCallback_->onBindDialogTarget_ = nullptr;
            session->sessionChangeCallback_->onSessionTopmostChange_ = nullptr;
            session->sessionChangeCallback_->onRaiseToTop_ = nullptr;
            session->sessionChangeCallback_->OnSessionEvent_ = nullptr;
            session->sessionChangeCallback_->OnSystemBarPropertyChange_ = nullptr;
            session->sessionChangeCallback_->onRaiseAboveTarget_ = nullptr;
            session->sessionChangeCallback_->OnTouchOutside_ = nullptr;
            session->sessionChangeCallback_->onSetLandscapeMultiWindowFunc_ = nullptr;
            session->sessionChangeCallback_->onLayoutFullScreenChangeFunc_ = nullptr;
        }
        session->Session::UnregisterSessionChangeListeners();
    };
    PostTask(task, "UnregisterSessionChangeListeners");
}

uint32_t SceneSession::UpdateUIParam(const SessionUIParam& uiParam)
{
    bool lastVisible = IsVisible();
    dirtyFlags_ |= UpdateInteractiveInner(uiParam.interactive_) ?
        static_cast<uint32_t>(SessionUIDirtyFlag::INTERACTIVE) : 0;
    if (!uiParam.interactive_) {
        // keep ui state in recent
        return dirtyFlags_;
    }
    dirtyFlags_ |= UpdateVisibilityInner(true) ? static_cast<uint32_t>(SessionUIDirtyFlag::VISIBLE) : 0;

    dirtyFlags_ |= UpdateRectInner(uiParam, reason_) ?
        static_cast<uint32_t>(SessionUIDirtyFlag::RECT) : 0;
    dirtyFlags_ |= UpdateScaleInner(uiParam.scaleX_, uiParam.scaleY_, uiParam.pivotX_, uiParam.pivotY_) ?
        static_cast<uint32_t>(SessionUIDirtyFlag::SCALE) : 0;
    dirtyFlags_ |= UpdateZOrderInner(uiParam.zOrder_) ? static_cast<uint32_t>(SessionUIDirtyFlag::Z_ORDER) : 0;
    if (!lastVisible && IsVisible() && !isFocused_ && !postProcessFocusState_.enabled_ &&
        GetForegroundInteractiveStatus()) {
        postProcessFocusState_.enabled_ = true;
        postProcessFocusState_.isFocused_ = true;
        postProcessFocusState_.reason_ = isStarting_ ?
            FocusChangeReason::SCB_START_APP : FocusChangeReason::FOREGROUND;
    }
    return dirtyFlags_;
}

uint32_t SceneSession::UpdateUIParam()
{
    bool lastVisible = IsVisible();
    dirtyFlags_ |= UpdateVisibilityInner(false) ? static_cast<uint32_t>(SessionUIDirtyFlag::VISIBLE) : 0;
    if (lastVisible && !IsVisible() && isFocused_) {
        postProcessFocusState_.enabled_ = true;
        postProcessFocusState_.isFocused_ = false;
        postProcessFocusState_.reason_ = FocusChangeReason::BACKGROUND;
    }
    return dirtyFlags_;
}

bool SceneSession::UpdateVisibilityInner(bool visibility)
{
    if (isVisible_ == visibility) {
        return false;
    }
    TLOGI(WmsLogTag::WMS_PIPELINE, "id: %{public}d, visibility: %{public}u -> %{public}u",
        GetPersistentId(), isVisible_, visibility);
    isVisible_ = visibility;
    if (updatePrivateStateAndNotifyFunc_ != nullptr) {
        updatePrivateStateAndNotifyFunc_(GetPersistentId());
    }
    if (notifyVisibleChangeFunc_ != nullptr) {
        notifyVisibleChangeFunc_(GetPersistentId());
    }
    return true;
}

bool SceneSession::UpdateInteractiveInner(bool interactive)
{
    if (GetForegroundInteractiveStatus() == interactive) {
        return false;
    }
    SetForegroundInteractiveStatus(interactive);
    NotifyClientToUpdateInteractive(interactive);
    return true;
}

bool SceneSession::PipelineNeedNotifyClientToUpdateRect() const
{
    return IsVisibleForeground() && GetForegroundInteractiveStatus();
}

bool SceneSession::UpdateRectInner(const SessionUIParam& uiParam, SizeChangeReason reason)
{
    if (!((NotifyServerToUpdateRect(uiParam, reason) || IsDirtyWindow()) && PipelineNeedNotifyClientToUpdateRect())) {
        return false;
    }
    std::shared_ptr<RSTransaction> rsTransaction = nullptr;
    auto transactionController = RSSyncTransactionController::GetInstance();
    if (transactionController) {
        rsTransaction = transactionController->GetRSTransaction();
    }
    NotifyClientToUpdateRect("WMSPipeline", rsTransaction);
    return true;
}

bool SceneSession::NotifyServerToUpdateRect(const SessionUIParam& uiParam, SizeChangeReason reason)
{
    if (!GetForegroundInteractiveStatus()) {
        TLOGD(WmsLogTag::WMS_PIPELINE, "skip recent, id:%{public}d", GetPersistentId());
        return false;
    }
    if (uiParam.rect_.IsInvalid()) {
        TLOGE(WmsLogTag::WMS_PIPELINE, "id:%{public}d rect:%{public}s is invalid",
            GetPersistentId(), uiParam.rect_.ToString().c_str());
        return false;
    }
    auto globalRect = GetSessionGlobalRect();
    SetSessionGlobalRect(uiParam.rect_);
    if (!uiParam.needSync_ || !isNeedSyncSessionRect_) {
        TLOGI(WmsLogTag::WMS_LAYOUT, "id:%{public}d, scenePanelNeedSync:%{public}u needSyncSessionRect:%{public}u "
            "rectAfter:%{public}s preRect:%{public}s preGlobalRect:%{public}s", GetPersistentId(), uiParam.needSync_,
            isNeedSyncSessionRect_, uiParam.rect_.ToString().c_str(), winRect_.ToString().c_str(),
            globalRect.ToString().c_str());
        return false;
    }
    WSRect rect = { uiParam.rect_.posX_ - uiParam.transX_, uiParam.rect_.posY_ - uiParam.transY_,
        uiParam.rect_.width_, uiParam.rect_.height_ };
    if (winRect_ == rect) {
        TLOGD(WmsLogTag::WMS_PIPELINE, "skip same rect update id:%{public}d rect:%{public}s preGlobalRect:%{public}s!",
            GetPersistentId(), rect.ToString().c_str(), globalRect.ToString().c_str());
        return false;
    }
    if (rect.IsInvalid()) {
        TLOGE(WmsLogTag::WMS_PIPELINE, "id:%{public}d rect:%{public}s is invalid, preGlobalRect:%{public}s",
            GetPersistentId(), rect.ToString().c_str(), globalRect.ToString().c_str());
        return false;
    }
    TLOGI(WmsLogTag::WMS_LAYOUT, "id:%{public}d, updateRect rectAfter:%{public}s preRect:%{public}s "
        "preGlobalRect:%{public}s", GetPersistentId(), rect.ToString().c_str(),
        winRect_.ToString().c_str(), globalRect.ToString().c_str());
    winRect_ = rect;
    RectCheckProcess();
    return true;
}

void SceneSession::PostProcessNotifyAvoidArea()
{
    if (PipelineNeedNotifyClientToUpdateAvoidArea(dirtyFlags_)) {
        NotifyClientToUpdateAvoidArea();
    }
}

bool SceneSession::PipelineNeedNotifyClientToUpdateAvoidArea(uint32_t dirty) const
{
    return ((dirty & static_cast<uint32_t>(SessionUIDirtyFlag::VISIBLE)) && IsImmersiveType()) ||
        ((dirty & static_cast<uint32_t>(SessionUIDirtyFlag::AVOID_AREA)) && isVisible_);
}

void SceneSession::NotifyClientToUpdateAvoidArea()
{
    if (specificCallback_ == nullptr) {
        return;
    }
    if (specificCallback_->onUpdateAvoidArea_) {
        specificCallback_->onUpdateAvoidArea_(GetPersistentId());
    }
    if (specificCallback_->onUpdateOccupiedAreaIfNeed_) {
        specificCallback_->onUpdateOccupiedAreaIfNeed_(GetPersistentId());
    }
}

bool SceneSession::IsTransformNeedChange(float scaleX, float scaleY, float pivotX, float pivotY)
{
    bool nearEqual = NearEqual(scaleX_, scaleX) && NearEqual(scaleY_, scaleY) &&
        NearEqual(pivotX_, pivotX) && NearEqual(pivotY_, pivotY) &&
        NearEqual(clientScaleX_, scaleX) && NearEqual(clientScaleY_, scaleY) &&
        NearEqual(clientPivotX_, pivotX) && NearEqual(clientPivotY_, pivotY);
    return !nearEqual;
}

bool SceneSession::UpdateScaleInner(float scaleX, float scaleY, float pivotX, float pivotY)
{
    if (!IsTransformNeedChange(scaleX, scaleY, pivotX, pivotY)) {
        return false;
    }
    Session::SetScale(scaleX, scaleY, pivotX, pivotY);
    if (!IsSessionForeground()) {
        TLOGD(WmsLogTag::WMS_LAYOUT, "id:%{public}d, session is not foreground!", GetPersistentId());
        return false;
    }
    if (sessionStage_ != nullptr) {
        Transform transform;
        transform.scaleX_ = scaleX;
        transform.scaleY_ = scaleY;
        transform.pivotX_ = pivotX;
        transform.pivotY_ = pivotY;
        sessionStage_->NotifyTransformChange(transform);
        Session::SetClientScale(scaleX, scaleY, pivotX, pivotY);
    } else {
        WLOGFE("sessionStage is nullptr");
    }
    return true;
}

bool SceneSession::UpdateZOrderInner(uint32_t zOrder)
{
    if (zOrder_ == zOrder) {
        return false;
    }
    TLOGI(WmsLogTag::WMS_PIPELINE, "id: %{public}d, zOrder: %{public}u -> %{public}u, lastZOrder: %{public}u",
          GetPersistentId(), zOrder_, zOrder, lastZOrder_);
    lastZOrder_ = zOrder_;
    zOrder_ = zOrder;
    return true;
}

void SceneSession::SetPostProcessFocusState(PostProcessFocusState state)
{
    postProcessFocusState_ = state;
}

PostProcessFocusState SceneSession::GetPostProcessFocusState() const
{
    return postProcessFocusState_;
}

void SceneSession::ResetPostProcessFocusState()
{
    postProcessFocusState_.Reset();
}

void SceneSession::SetPostProcessProperty(bool state)
{
    postProcessProperty_ = state;
}

bool SceneSession::GetPostProcessProperty() const
{
    return postProcessProperty_;
}

bool SceneSession::IsImmersiveType() const
{
    WindowType type = GetWindowType();
    return type == WindowType::WINDOW_TYPE_STATUS_BAR ||
        type == WindowType::WINDOW_TYPE_NAVIGATION_BAR ||
        type == WindowType::WINDOW_TYPE_INPUT_METHOD_FLOAT;
}

void SceneSession::SetDefaultDisplayIdIfNeed()
{
    if (sessionInfo_.screenId_ == SCREEN_ID_INVALID) {
        auto defaultDisplayId = ScreenSessionManagerClient::GetInstance().GetDefaultScreenId();
        sessionInfo_.screenId_ = defaultDisplayId;
        TLOGI(WmsLogTag::WMS_LIFE, "winId: %{public}d, update screen id %{public}" PRIu64,
            GetPersistentId(), defaultDisplayId);
        auto sessionProperty = GetSessionProperty();
        if (sessionProperty) {
            sessionProperty->SetDisplayId(defaultDisplayId);
        }
    }
}

void SceneSession::UpdateGestureBackEnabled()
{
    if (specificCallback_ != nullptr &&
        specificCallback_->onUpdateGestureBackEnabled_ != nullptr) {
        specificCallback_->onUpdateGestureBackEnabled_(GetPersistentId());
    }
}

bool SceneSession::CheckIdentityTokenIfMatched(const std::string& identityToken)
{
    if (!identityToken.empty() && !clientIdentityToken_.empty() && identityToken != clientIdentityToken_) {
        TLOGW(WmsLogTag::WMS_LIFE,
            "failed, clientIdentityToken: %{public}s, identityToken: %{public}s, bundleName: %{public}s",
            clientIdentityToken_.c_str(), identityToken.c_str(), GetSessionInfo().bundleName_.c_str());
        return false;
    }
    return true;
}

bool SceneSession::CheckPidIfMatched()
{
    int32_t callingPid = IPCSkeleton::GetCallingPid();
    if (callingPid != -1 && callingPid != GetCallingPid()) {
        TLOGW(WmsLogTag::WMS_LIFE,
            "failed, callingPid_: %{public}d, callingPid: %{public}d, bundleName: %{public}s",
            GetCallingPid(), callingPid, GetSessionInfo().bundleName_.c_str());
        return false;
    }
    return true;
}

void SceneSession::SetNeedSyncSessionRect(bool needSync)
{
    auto task = [weakThis = wptr(this), needSync]() -> void {
        auto session = weakThis.promote();
        if (session == nullptr) {
            TLOGNE(WmsLogTag::WMS_PIPELINE, "SetNeedSyncSessionRect session is null");
            return;
        }
        TLOGNI(WmsLogTag::WMS_PIPELINE,
            "SetNeedSyncSessionRect: change isNeedSync from %{public}d to %{public}d, id:%{public}d",
            session->isNeedSyncSessionRect_, needSync, session->GetPersistentId());
        session->isNeedSyncSessionRect_ = needSync;
    };
    PostTask(task, __func__);
}

void SceneSession::MarkAvoidAreaAsDirty()
{
    dirtyFlags_ |= static_cast<uint32_t>(SessionUIDirtyFlag::AVOID_AREA);
}
} // namespace OHOS::Rosen
