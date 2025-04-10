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
#include "window_helper.h"
#include "pointer_event.h"
#include "window_manager_hilog.h"


namespace OHOS::Rosen {
namespace {
constexpr HiviewDFX::HiLogLabel LABEL = { LOG_CORE, HILOG_DOMAIN_WINDOW, "SubSession" };
} // namespace

SubSession::SubSession(const SessionInfo& info, const sptr<SpecificSessionCallback>& specificCallback)
    : SceneSession(info, specificCallback)
{
    pcFoldScreenController_ = sptr<PcFoldScreenController>::MakeSptr(wptr(this), GetPersistentId());
    moveDragController_ = sptr<MoveDragController>::MakeSptr(GetPersistentId(), GetWindowType());
    if (specificCallback != nullptr &&
        specificCallback->onWindowInputPidChangeCallback_ != nullptr) {
        moveDragController_->SetNotifyWindowPidChangeCallback(specificCallback->onWindowInputPidChangeCallback_);
    }
    SetMoveDragCallback();
    TLOGD(WmsLogTag::WMS_LIFE, "Create");
}

SubSession::~SubSession()
{
    TLOGD(WmsLogTag::WMS_LIFE, "id: %{public}d", GetPersistentId());
}

WSError SubSession::Show(sptr<WindowSessionProperty> property)
{
    if (!CheckPermissionWithPropertyAnimation(property)) {
        return WSError::WS_ERROR_NOT_SYSTEM_APP;
    }
    if (property->GetWindowFlags() & static_cast<uint32_t>(WindowFlag::WINDOW_FLAG_IS_TEXT_MENU) &&
        GetForceHideState() != ForceHideState::NOT_HIDDEN) {
        TLOGI(WmsLogTag::WMS_SUB, "UEC force hide, id: %{public}d forceHideState: %{public}d",
            GetPersistentId(), GetForceHideState());
        return WSError::WS_ERROR_INVALID_OPERATION;
    }
    PostTask([weakThis = wptr(this), property]() {
        auto session = weakThis.promote();
        if (!session) {
            TLOGE(WmsLogTag::WMS_SUB, "session is null");
            return WSError::WS_ERROR_DESTROYED_OBJECT;
        }
        TLOGI(WmsLogTag::WMS_LIFE, "Show session, id: %{public}d", session->GetPersistentId());

        auto parentSession = session->GetParentSession();
        if (parentSession && session->GetShouldFollowParentWhenShow()) {
            session->CheckAndMoveDisplayIdRecursively(parentSession->GetSessionProperty()->GetDisplayId());
        } else {
            TLOGNE(WmsLogTag::WMS_SUB, "session has no parent, id: %{public}d", session->GetPersistentId());
        }
        // use property from client
        auto sessionProperty = session->GetSessionProperty();
        if (property && property->GetAnimationFlag() == static_cast<uint32_t>(WindowAnimation::CUSTOM) &&
            sessionProperty) {
            sessionProperty->SetAnimationFlag(static_cast<uint32_t>(WindowAnimation::CUSTOM));
            session->NotifyIsCustomAnimationPlaying(true);
        }
        auto ret = session->SceneSession::Foreground(property);
        return ret;
    }, "Show");
    return WSError::WS_OK;
}

void SubSession::NotifySessionRectChange(const WSRect& rect, SizeChangeReason reason, DisplayId displayId,
    const RectAnimationConfig& rectAnimationConfig)
{
    if (reason == SizeChangeReason::DRAG_END) {
        SetShouldFollowParentWhenShow(false);
    }
    SceneSession::NotifySessionRectChange(rect, reason, displayId, rectAnimationConfig);
}

void SubSession::UpdateSessionRectInner(const WSRect& rect, SizeChangeReason reason,
    const MoveConfiguration& moveConfiguration, const RectAnimationConfig& rectAnimationConfig)
{
    if (moveConfiguration.displayId != DISPLAY_ID_INVALID) {
        SetShouldFollowParentWhenShow(false);
    }
    SceneSession::UpdateSessionRectInner(rect, reason, moveConfiguration, rectAnimationConfig);
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
            TLOGNE(WmsLogTag::WMS_SUB, "session is null");
            return WSError::WS_ERROR_DESTROYED_OBJECT;
        }
        TLOGNI(WmsLogTag::WMS_LIFE, "Hide session, id: %{public}d", session->GetPersistentId());
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
    TLOGD(WmsLogTag::WMS_INPUT_KEY_FLOW, "id:%{public}d, type:%{public}d", id, GetWindowType());
    auto isModal = IsModal();
    auto parentSession = GetParentSession();
    if (!isModal && parentSession && parentSession->CheckDialogOnForeground()) {
        WLOGFI("Has dialog foreground, id: %{public}d, type: %{public}d", id, GetWindowType());
        return WSError::WS_OK;
    }
    if (isModal) {
        Session::ProcessClickModalWindowOutside(posX, posY);
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
    auto isPC = systemConfig_.IsPcWindow();
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

void SubSession::SetParentSessionCallback(NotifySetParentSessionFunc&& func)
{
    const char* const where = __func__;
    PostTask([weakThis = wptr(this), where, func = std::move(func)] {
        auto session = weakThis.promote();
        if (!session || !func) {
            TLOGNE(WmsLogTag::WMS_SUB, "%{public}s session or func is nullptr", where);
            return;
        }
        session->setParentSessionFunc_ = std::move(func);
        TLOGND(WmsLogTag::WMS_SUB, "%{public}s id: %{public}d", where,
            session->GetPersistentId());
    }, __func__);
}

WMError SubSession::NotifySetParentSession(int32_t oldParentWindowId, int32_t newParentWindowId)
{
    return PostSyncTask([weakThis = wptr(this), oldParentWindowId, newParentWindowId, where = __func__] {
        auto session = weakThis.promote();
        if (!session) {
            TLOGNE(WmsLogTag::WMS_SUB, "%{public}s session is nullptr", where);
            return WMError::WM_ERROR_INVALID_WINDOW;
        }
        if (session->setParentSessionFunc_) {
            session->setParentSessionFunc_(oldParentWindowId, newParentWindowId);
            TLOGND(WmsLogTag::WMS_SUB, "%{public}s id: %{public}d oldParentWindowId: %{public}d "
                "newParentWindowId: %{public}d", where, session->GetPersistentId(), oldParentWindowId,
                newParentWindowId);
        }
        return WMError::WM_OK;
    }, __func__);
}

WSError SubSession::NotifyFollowParentMultiScreenPolicy(bool enabled)
{
    PostTask([weakThis = wptr(this), enabled, funcName = __func__] {
        auto session = weakThis.promote();
        if (!session) {
            TLOGNE(WmsLogTag::WMS_SUB, "%{public}s: session is null", funcName);
            return;
        }
        TLOGNI(WmsLogTag::WMS_SUB, "%{public}s: enabled:%{public}d", funcName, enabled);
        session->isFollowParentMultiScreenPolicy_ = enabled;
    }, __func__);
    return WSError::WS_OK;
}

bool SubSession::IsFollowParentMultiScreenPolicy() const
{
    return isFollowParentMultiScreenPolicy_;
}

WSError SubSession::SetSubWindowZLevel(int32_t zLevel)
{
    PostTask([weakThis = wptr(this), zLevel]() {
        auto session = weakThis.promote();
        if (!session) {
            TLOGE(WmsLogTag::WMS_HIERARCHY, "session is null");
            return;
        }
        auto property = session->GetSessionProperty();
        TLOGI(WmsLogTag::WMS_HIERARCHY, "Notify session zLevel change, id: %{public}d, zLevel: %{public}d",
            session->GetPersistentId(), zLevel);
        property->SetSubWindowZLevel(zLevel);
        if (session->onSubSessionZLevelChange_) {
            session->onSubSessionZLevelChange_(zLevel);
        }
    }, "SetSubWindowZLevel");
    return WSError::WS_OK;
}

int32_t SubSession::GetSubWindowZLevel() const
{
    int32_t zLevel = 0;
    auto sessionProperty = GetSessionProperty();
    zLevel = sessionProperty->GetSubWindowZLevel();
    TLOGI(WmsLogTag::WMS_HIERARCHY, "zLevel: %{public}d", zLevel);
    return zLevel;
}
} // namespace OHOS::Rosen
