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

#include "session/host/include/sub_session.h"
#include "screen_session_manager/include/screen_session_manager_client.h"

#include "common/include/session_permission.h"
#include "key_event.h"
#include "window_helper.h"
#include "parameters.h"
#include "pointer_event.h"
#include "window_manager_hilog.h"


namespace OHOS::Rosen {
namespace {
constexpr HiviewDFX::HiLogLabel LABEL = { LOG_CORE, HILOG_DOMAIN_WINDOW, "SubSession" };
} // namespace

SubSession::SubSession(const SessionInfo& info, const sptr<SpecificSessionCallback>& specificCallback)
    : SceneSession(info, specificCallback)
{
    moveDragController_ = new (std::nothrow) MoveDragController(GetPersistentId());
    if (moveDragController_ != nullptr && specificCallback != nullptr &&
        specificCallback->onWindowInputPidChangeCallback_ != nullptr) {
        moveDragController_->SetNotifyWindowPidChangeCallback(specificCallback->onWindowInputPidChangeCallback_);
    }
    SetMoveDragCallback();
    TLOGD(WmsLogTag::WMS_LIFE, "Create SubSession");
}

SubSession::~SubSession()
{
    TLOGD(WmsLogTag::WMS_LIFE, "~SubSession, id: %{public}d", GetPersistentId());
}

WSError SubSession::Show(sptr<WindowSessionProperty> property)
{
    if (!CheckPermissionWithPropertyAnimation(property)) {
        return WSError::WS_ERROR_NOT_SYSTEM_APP;
    }
    auto task = [weakThis = wptr(this), property]() {
        auto session = weakThis.promote();
        if (!session) {
            TLOGE(WmsLogTag::WMS_SUB, "session is null");
            return WSError::WS_ERROR_DESTROYED_OBJECT;
        }
        TLOGI(WmsLogTag::WMS_LIFE, "Show session, id: %{public}d", session->GetPersistentId());

        // use property from client
        auto sessionProperty = session->GetSessionProperty();
        if (property && property->GetAnimationFlag() == static_cast<uint32_t>(WindowAnimation::CUSTOM) &&
            sessionProperty) {
            sessionProperty->SetAnimationFlag(static_cast<uint32_t>(WindowAnimation::CUSTOM));
            session->NotifyIsCustomAnimationPlaying(true);
        }
        auto ret = session->SceneSession::Foreground(property);
        return ret;
    };
    PostTask(task, "Show");
    return WSError::WS_OK;
}

WSError SubSession::Hide()
{
    return Hide(false);  // async mode
}

WSError SubSession::HideSync()
{
    return Hide(true);  // sync mode
}

WSError SubSession::Hide(bool needSyncHide)
{
    if (!CheckPermissionWithPropertyAnimation(GetSessionProperty())) {
        return WSError::WS_ERROR_NOT_SYSTEM_APP;
    }
    auto task = [weakThis = wptr(this)]() {
        auto session = weakThis.promote();
        if (!session) {
            TLOGE(WmsLogTag::WMS_SUB, "session is null");
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
        ret = session->SceneSession::Background();
        return ret;
    };

    if (needSyncHide) {
        return PostSyncTask(task, "HideSync");
    }

    PostTask(task, "HideAsync");
    return WSError::WS_OK;
}

WSError SubSession::ProcessPointDownSession(int32_t posX, int32_t posY)
{
    const auto& id = GetPersistentId();
    WLOGFI("id: %{public}d, type: %{public}d", id, GetWindowType());
    auto parentSession = GetParentSession();
    if (parentSession && parentSession->CheckDialogOnForeground()) {
        WLOGFI("Has dialog foreground, id: %{public}d, type: %{public}d", id, GetWindowType());
        return WSError::WS_OK;
    }
    auto sessionProperty = GetSessionProperty();
    if (sessionProperty && sessionProperty->GetRaiseEnabled()) {
        RaiseToAppTopForPointDown();
    }
    PresentFocusIfPointDown();
    return SceneSession::ProcessPointDownSession(posX, posY);
}

int32_t SubSession::GetMissionId() const
{
    auto parentSession = GetParentSession();
    return parentSession != nullptr ? parentSession->GetPersistentId() : SceneSession::GetMissionId();
}

WSError SubSession::TransferKeyEvent(const std::shared_ptr<MMI::KeyEvent>& keyEvent)
{
    if (!IsSessionValid()) {
        return WSError::WS_ERROR_INVALID_SESSION;
    }
    if (keyEvent == nullptr) {
        WLOGFE("KeyEvent is nullptr");
        return WSError::WS_ERROR_NULLPTR;
    }
    auto parentSession = GetParentSession();
    if (parentSession && parentSession->CheckDialogOnForeground()) {
        TLOGD(WmsLogTag::WMS_DIALOG, "Its main window has dialog on foreground, not transfer pointer event");
        return WSError::WS_ERROR_INVALID_PERMISSION;
    }

    WSError ret = Session::TransferKeyEvent(keyEvent);
    return ret;
}

void SubSession::UpdatePointerArea(const WSRect& rect)
{
    auto property = GetSessionProperty();
    if (!(property->IsDecorEnable() && GetWindowMode() == WindowMode::WINDOW_MODE_FLOATING)) {
        return;
    }
    Session::UpdatePointerArea(rect);
}

bool SubSession::CheckPointerEventDispatch(const std::shared_ptr<MMI::PointerEvent>& pointerEvent) const
{
    auto sessionState = GetSessionState();
    int32_t action = pointerEvent->GetPointerAction();
    auto isPC = systemConfig_.uiType_ == UI_TYPE_PC;
    if (isPC && sessionState != SessionState::STATE_FOREGROUND &&
        sessionState != SessionState::STATE_ACTIVE &&
        action != MMI::PointerEvent::POINTER_ACTION_LEAVE_WINDOW) {
        WLOGFW("Current Session Info: [persistentId: %{public}d, "
            "state: %{public}d, action:%{public}d]", GetPersistentId(), GetSessionState(), action);
        return false;
    }
    return true;
}

void SubSession::RectCheck(uint32_t curWidth, uint32_t curHeight)
{
    uint32_t minWidth = GetSystemConfig().miniWidthOfSubWindow_;
    uint32_t minHeight = GetSystemConfig().miniHeightOfSubWindow_;
    uint32_t maxFloatingWindowSize = GetSystemConfig().maxFloatingWindowSize_;
    RectSizeCheckProcess(curWidth, curHeight, minWidth, minHeight, maxFloatingWindowSize);
}

bool SubSession::IsTopmost() const
{
    bool isTopmost = false;
    auto sessionProperty = GetSessionProperty();
    if (sessionProperty) {
        isTopmost = sessionProperty->IsTopmost();
    }
    TLOGI(WmsLogTag::WMS_SUB, "isTopmost: %{public}d", isTopmost);
    return isTopmost;
}

bool SubSession::IsModal() const
{
    return WindowHelper::IsModalSubWindow(GetSessionProperty()->GetWindowType(),
                                          GetSessionProperty()->GetWindowFlags());
}

bool SubSession::IsApplicationModal() const
{
    return WindowHelper::IsApplicationModalSubWindow(GetSessionProperty()->GetWindowType(),
                                                     GetSessionProperty()->GetWindowFlags());
}

bool SubSession::IsVisibleForeground() const
{
    const auto& mainOrFloatSession = GetMainOrFloatSession();
    if (mainOrFloatSession) {
        return mainOrFloatSession->IsVisibleForeground() && Session::IsVisibleForeground();
    }
    return Session::IsVisibleForeground();
}
} // namespace OHOS::Rosen
