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

#include "session/host/include/scb_system_session.h"

#include <hisysevent.h>
#include "pointer_event.h"
#include "rs_adapter.h"
#include <ui/rs_surface_node.h>
#include "window_manager_hilog.h"

namespace OHOS::Rosen {
namespace {
constexpr HiviewDFX::HiLogLabel LABEL = { LOG_CORE, HILOG_DOMAIN_WINDOW, "SCBSystemSession" };
const std::unordered_map<std::string, SurfaceWindowType> surfaceWindowTypeMap = {
    { "SCBDesktop", SurfaceWindowType::SCB_DESKTOP },
    { "SCBWallpaper", SurfaceWindowType::SCB_WALLPAPER },
    { "SCBScreenLock", SurfaceWindowType::SCB_SCREEN_LOCK },
    { "SCBNegativeScreen", SurfaceWindowType::SCB_NEGATIVE_SCREEN },
    { "SCBDropdownPanel", SurfaceWindowType::SCB_DROPDOWN_PANEL },
    { "SCBVolumePanel", SurfaceWindowType::SCB_VOLUME_PANEL },
    { "SCBBannerNotification", SurfaceWindowType::SCB_BANNER_NOTIFICATION },
    { "SCBGestureBack", SurfaceWindowType::SCB_GESTURE_BACK }
};
} // namespace

SCBSystemSession::SCBSystemSession(const SessionInfo& info, const sptr<SpecificSessionCallback>& specificCallback,
    int32_t userId): SceneSession(info, specificCallback, userId)
{
    auto name = sessionInfo_.bundleName_;
    auto pos = name.find_last_of('.');
    name = (pos == std::string::npos) ? name : name.substr(pos + 1); // skip '.'
    if (sessionInfo_.isSystem_) {
        RSSurfaceNodeConfig config;
        config.SurfaceNodeName = name;
        config.surfaceWindowType = SurfaceWindowType::SYSTEM_SCB_WINDOW;
        for (const auto& iter : surfaceWindowTypeMap) {
            if (name.find(iter.first) != std::string::npos) {
                config.surfaceWindowType = iter.second;
                break;
            }
        }
        surfaceNode_ = Rosen::RSSurfaceNode::Create(config, Rosen::RSSurfaceNodeType::APP_WINDOW_NODE);
        shadowSurfaceNode_ = RSAdapterUtil::IsClientMultiInstanceEnabled() && surfaceNode_ ?
            surfaceNode_->CreateShadowSurfaceNode() : nullptr;
        RSAdapterUtil::SetRSUIContext(surfaceNode_, GetRSUIContext(), true);
        TLOGD(WmsLogTag::WMS_SCB, "Create RSSurfaceNode: %{public}s, name: %{public}s",
              RSAdapterUtil::RSNodeToStr(surfaceNode_).c_str(), name.c_str());
        SetIsUseControlSession(info.isUseControlSession);
    }
    WLOGFD("Create SCBSystemSession");
}

SCBSystemSession::~SCBSystemSession()
{
    WLOGD("~SCBSystemSession, id: %{public}d", GetPersistentId());
}

WSError SCBSystemSession::ProcessPointDownSession(int32_t posX, int32_t posY)
{
    const auto id = GetPersistentId();
    const auto type = GetWindowType();
    TLOGD(WmsLogTag::WMS_INPUT_KEY_FLOW, "id: %{public}d, type: %{public}d", id, type);
    auto ret = SceneSession::ProcessPointDownSession(posX, posY);
    PresentFocusIfPointDown();
    return ret;
}

WSError SCBSystemSession::NotifyClientToUpdateRect(const std::string& updateReason,
    std::shared_ptr<RSTransaction> rsTransaction)
{
    PostTask([weakThis = wptr(this), rsTransaction, updateReason]() {
        auto session = weakThis.promote();
        if (!session) {
            WLOGFE("session is null");
            return WSError::WS_ERROR_DESTROYED_OBJECT;
        }
        WSError ret = session->NotifyClientToUpdateRectTask(updateReason, rsTransaction);
        if (session->specificCallback_ != nullptr && session->specificCallback_->onUpdateAvoidArea_ != nullptr &&
            session->specificCallback_->onClearDisplayStatusBarTemporarilyFlags_ != nullptr) {
            if (Session::IsScbCoreEnabled()) {
                session->dirtyFlags_ |= static_cast<uint32_t>(SessionUIDirtyFlag::AVOID_AREA);
            } else {
                session->specificCallback_->onUpdateAvoidArea_(session->GetPersistentId());
            }
            session->specificCallback_->onClearDisplayStatusBarTemporarilyFlags_();
        }
        return ret;
    }, "NotifyClientToUpdateRect");
    return WSError::WS_OK;
}

void SCBSystemSession::BindKeyboardSession(sptr<SceneSession> session)
{
    if (session == nullptr) {
        TLOGE(WmsLogTag::WMS_KEYBOARD, "session is nullptr");
        return;
    }
    keyboardSession_ = session;
    TLOGI(WmsLogTag::WMS_KEYBOARD, "Success, id: %{public}d", keyboardSession_->GetPersistentId());
}

sptr<SceneSession> SCBSystemSession::GetKeyboardSession() const
{
    return keyboardSession_;
}

void SCBSystemSession::PresentFocusIfPointDown()
{
    TLOGI(WmsLogTag::WMS_FOCUS, "Id:%{public}d,type:%{public}d", GetPersistentId(), GetWindowType());
    if (!isFocused_ && GetFocusable()) {
        FocusChangeReason reason = FocusChangeReason::CLICK;
        NotifyRequestFocusStatusNotifyManager(true, false, reason);
    }
    NotifyClick();
}

WSError SCBSystemSession::TransferKeyEvent(const std::shared_ptr<MMI::KeyEvent>& keyEvent)
{
    if (keyEvent == nullptr) {
        WLOGFE("KeyEvent is nullptr");
        return WSError::WS_ERROR_NULLPTR;
    }

    WSError ret = Session::TransferKeyEvent(keyEvent);
    return ret;
}

WSError SCBSystemSession::UpdateFocus(bool isFocused)
{
    if (isFocused_ == isFocused) {
        TLOGD(WmsLogTag::WMS_FOCUS, "Session focus do not change");
        return WSError::WS_DO_NOTHING;
    }
    isFocused_ = isFocused;
    // notify scb arkui focus
    if (isFocused) {
        HiSysEventWrite(
            OHOS::HiviewDFX::HiSysEvent::Domain::WINDOW_MANAGER,
            "FOCUS_WINDOW",
            OHOS::HiviewDFX::HiSysEvent::EventType::BEHAVIOR,
            "PID", getpid(),
            "UID", getuid(),
            "BUNDLE_NAME", sessionInfo_.bundleName_,
            "WINDOW_TYPE", static_cast<uint32_t>(GetWindowType()));
        NotifyUIRequestFocus();
    } else {
        NotifyUILostFocus();
    }
    return WSError::WS_OK;
}

WSError SCBSystemSession::UpdateWindowMode(WindowMode mode)
{
    WLOGFD("session is system, id: %{public}d, mode: %{public}d, name: %{public}s, state: %{public}u",
        GetPersistentId(), static_cast<int32_t>(mode), sessionInfo_.bundleName_.c_str(), GetSessionState());
    return WSError::WS_ERROR_INVALID_SESSION;
}

WSError SCBSystemSession::SetSystemSceneBlockingFocus(bool blocking)
{
    TLOGD(WmsLogTag::WMS_FOCUS, "Session set blocking focus, id: %{public}d, mode: %{public}d",
        GetPersistentId(), blocking);
    blockingFocus_ = blocking;
    return WSError::WS_OK;
}

void SCBSystemSession::UpdatePointerArea(const WSRect& rect)
{
    // scb system session do nothing
    return;
}

void SCBSystemSession::SetSkipSelfWhenShowOnVirtualScreen(bool isSkip)
{
    TLOGD(WmsLogTag::WMS_SCB, "Set skip Self, isSkip: %{public}d", isSkip);
    PostTask([weakThis = wptr(this), isSkip, where = __func__]() {
        auto session = weakThis.promote();
        if (!session) {
            TLOGNE(WmsLogTag::WMS_SCB, "%{public}s session is null", where);
            return;
        }
        std::shared_ptr<RSSurfaceNode> surfaceNode = session->GetSurfaceNode();
        if (!surfaceNode) {
            TLOGNE(WmsLogTag::WMS_SCB, "%{public}s surfaceNode_ is null", where);
            return;
        }
        if (session->specificCallback_ != nullptr &&
            session->specificCallback_->onSetSkipSelfWhenShowOnVirtualScreen_ != nullptr) {
            session->specificCallback_->onSetSkipSelfWhenShowOnVirtualScreen_(surfaceNode->GetId(), isSkip);
        }
        return;
    }, __func__);
}

void SCBSystemSession::SetSkipEventOnCastPlus(bool isSkip)
{
    TLOGD(WmsLogTag::WMS_SCB, "Set skip event on cast plus, wid: %{public}d, isSkip: %{public}d",
        GetPersistentId(), isSkip);
    PostTask([weakThis = wptr(this), isSkip, where = __func__]() {
        auto session = weakThis.promote();
        if (!session) {
            TLOGNE(WmsLogTag::WMS_SCB, "%{public}s session is null", where);
            return;
        }
        if (session->specificCallback_ != nullptr &&
            session->specificCallback_->onSetSkipEventOnCastPlus_ != nullptr) {
            session->specificCallback_->onSetSkipEventOnCastPlus_(session->GetPersistentId(), isSkip);
        }
        return;
    }, __func__);
}

bool SCBSystemSession::IsVisibleForeground() const
{
    return isVisible_;
}

bool SCBSystemSession::IsVisibleNotBackground() const
{
    return isVisible_;
}

void SCBSystemSession::NotifyClientToUpdateAvoidArea()
{
    SceneSession::NotifyClientToUpdateAvoidArea();
}

void SCBSystemSession::SyncScenePanelGlobalPosition(bool needSync)
{
    TLOGI(WmsLogTag::WMS_PIPELINE, "change isNeedSyncGlobalPos from %{public}d to %{public}d",
        isNeedSyncGlobalPos_, needSync);
    isNeedSyncGlobalPos_ = needSync;
}

bool SCBSystemSession::GetIsUseControlSession() const
{
    return isUseControlSession_;
}

void SCBSystemSession::SetIsUseControlSession(bool isUseControlSession)
{
    isUseControlSession_ = isUseControlSession;
}
} // namespace OHOS::Rosen
