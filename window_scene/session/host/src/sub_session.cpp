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

#include "rs_adapter.h"
#include "screen_session_manager_client/include/screen_session_manager_client.h"
#include "session/screen/include/screen_session.h"
#include <transaction/rs_sync_transaction_controller.h>
#include <transaction/rs_transaction.h>
#include <ui/rs_surface_node.h>
#include "window_helper.h"
#include "pointer_event.h"
#include "window_manager_hilog.h"


namespace OHOS::Rosen {
namespace {
constexpr HiviewDFX::HiLogLabel LABEL = { LOG_CORE, HILOG_DOMAIN_WINDOW, "SubSession" };
constexpr int32_t SUFFIX_INDEX = -1;
} // namespace

SubSession::SubSession(const SessionInfo& info, const sptr<SpecificSessionCallback>& specificCallback)
    : SceneSession(info, specificCallback)
{
    pcFoldScreenController_ = sptr<PcFoldScreenController>::MakeSptr(wptr(this), GetPersistentId());
    TLOGD(WmsLogTag::WMS_LIFE, "Create");
}

SubSession::~SubSession()
{
    TLOGD(WmsLogTag::WMS_LIFE, "id: %{public}d", GetPersistentId());
}

void SubSession::OnFirstStrongRef(const void* objectId)
{
    // OnFirstStrongRef is overridden in the parent class IPCObjectStub,
    // so its parent implementation must be invoked here to avoid IPC communication issues.
    SceneSession::OnFirstStrongRef(objectId);

    moveDragController_ = sptr<MoveDragController>::MakeSptr(wptr(this));
    if (specificCallback_ != nullptr && specificCallback_->onWindowInputPidChangeCallback_ != nullptr) {
        moveDragController_->SetNotifyWindowPidChangeCallback(specificCallback_->onWindowInputPidChangeCallback_);
    }
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

bool SubSession::IsNeedCrossDisplayRendering() const
{
    return IsFollowParentMultiScreenPolicy() && (IsAnyParentSessionDragMoving() || IsAnyParentSessionDragZooming());
}

void SubSession::UpdateSessionRectInner(const WSRect& rect, SizeChangeReason reason,
    const MoveConfiguration& moveConfiguration, const RectAnimationConfig& rectAnimationConfig)
{
    if (moveConfiguration.displayId != DISPLAY_ID_INVALID) {
        SetShouldFollowParentWhenShow(false);
    }
    if (IsNeedCrossDisplayRendering()) {
        auto newRequestRect = GetSessionRequestRect();
        if (reason == SizeChangeReason::MOVE) {
            newRequestRect.posX_ = rect.posX_;
            newRequestRect.posY_ = rect.posY_;
        } else if (reason == SizeChangeReason::RESIZE && rect.width_ > 0 && rect.height_ > 0) {
            newRequestRect.width_ = rect.width_;
            newRequestRect.height_ = rect.height_;
        }
        SetSessionRequestRect(newRequestRect);
        SetRequestRectWhenFollowParent(newRequestRect);
        WSRect globaleRect =
            layoutController_->ConvertRelativeRectToGlobal(newRequestRect, moveConfiguration.displayId);
        UpdateSizeChangeReason(reason);
        TLOGI(WmsLogTag::WMS_LAYOUT, "need show cross window, id:%{public}d, globalRect:%{public}s",
            GetPersistentId(), globaleRect.ToString().c_str());
        HandleCrossMoveTo(globaleRect);
        return;
    }

    if (IsSessionForeground() && (reason == SizeChangeReason::MOVE || reason == SizeChangeReason::RESIZE)) {
        SetOriPosYBeforeRaisedByKeyboard(0);
        const WSRect& winRect = GetSessionRect();
        if (reason == SizeChangeReason::MOVE && (rect.posX_ != winRect.posX_ || rect.posY_ != winRect.posY_)) {
            isSubWindowResizingOrMoving_ = true;
        }
        if (reason == SizeChangeReason::RESIZE && (rect.width_ != winRect.width_ || rect.height_ != winRect.height_)) {
            isSubWindowResizingOrMoving_ = true;
        }
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
        session->isSubWindowResizingOrMoving_ = false;
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
    auto ret = SceneSession::ProcessPointDownSession(posX, posY);
    PresentFocusIfPointDown();
    return ret;
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

bool SubSession::IsVisibleNotBackground() const
{
    const auto& mainOrFloatSession = GetMainOrFloatSession();
    if (mainOrFloatSession) {
        return mainOrFloatSession->IsVisibleNotBackground() && Session::IsVisibleNotBackground();
    }
    return Session::IsVisibleNotBackground();
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

void SubSession::HandleCrossSurfaceNodeByWindowAnchor(SizeChangeReason reason, DisplayId displayId)
{
    auto surfaceNode = GetSurfaceNodeForMoveDrag();
    if (!surfaceNode) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "Id:%{public}d, SurfaceNode is null", GetPersistentId());
        return;
    }
    if (reason == SizeChangeReason::DRAG || reason == SizeChangeReason::DRAG_MOVE) {
        auto dragMoveMountedNode = GetWindowDragMoveMountedNode(displayId, this->GetZOrder());
        if (dragMoveMountedNode == nullptr) {
            TLOGE(WmsLogTag::WMS_LAYOUT, "dragMoveMountedNode is null");
            return;
        }
        {
            AutoRSTransaction trans(surfaceNode->GetRSUIContext());
            surfaceNode->SetPositionZ(GetZOrder());
            surfaceNode->SetIsCrossNode(true);
        }
        {
            AutoRSTransaction trans(dragMoveMountedNode->GetRSUIContext());
            dragMoveMountedNode->AddCrossScreenChild(surfaceNode, SUFFIX_INDEX, true);
        }
        cloneNodeCountDuringCross_.fetch_add(1);
        TLOGI(WmsLogTag::WMS_LAYOUT, "Add sub window to display:%{public}" PRIu64 " persistentId:%{public}d, "
            "cloneNodeCountDuringCross:%{public}d", displayId, GetPersistentId(), cloneNodeCountDuringCross_.load());
    } else if (reason == SizeChangeReason::DRAG_END) {
        surfaceNode->SetPositionZ(moveDragController_->GetOriginalPositionZ());
        auto dragMoveMountedNode = GetWindowDragMoveMountedNode(displayId, this->GetZOrder());
        if (dragMoveMountedNode == nullptr) {
            TLOGE(WmsLogTag::WMS_LAYOUT, "dragMoveMountedNode is null");
            return;
        }
        dragMoveMountedNode->RemoveCrossScreenChild(surfaceNode);
        surfaceNode->SetIsCrossNode(false);
        cloneNodeCountDuringCross_.fetch_sub(1);
        TLOGI(WmsLogTag::WMS_LAYOUT, "Remove sub window from display:%{public}" PRIu64 " persistentId:%{public}d, "
            "cloneNodeCountDuringCross:%{public}d", displayId, GetPersistentId(), cloneNodeCountDuringCross_.load());
    }
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

void SubSession::HandleCrossMoveToSurfaceNode(WSRect& globalRect)
{
    auto movedSurfaceNode = GetSurfaceNodeForMoveDrag();
    if (movedSurfaceNode == nullptr) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "SurfaceNode is null");
        return;
    }

    TLOGI(WmsLogTag::WMS_LAYOUT, "displayId:%{public}" PRIu64 ", persistentId:%{public}d, globalRect:%{public}s",
        GetScreenId(), GetPersistentId(), globalRect.ToString().c_str());
    for (const auto displayId : GetNewDisplayIdsDuringMoveTo(globalRect)) {
        if (displayId == GetScreenId()) {
            continue;
        }
        auto screenSession = ScreenSessionManagerClient::GetInstance().GetScreenSessionById(displayId);
        if (screenSession == nullptr) {
            TLOGE(WmsLogTag::WMS_LAYOUT, "ScreenSession is null");
            continue;
        }
        if (screenSession->GetDisplayNode() == nullptr) {
            TLOGE(WmsLogTag::WMS_LAYOUT, "DisplayNode is null");
            continue;
        }
        if (screenSession->GetScreenProperty().GetScreenType() == ScreenType::VIRTUAL) {
            TLOGD(WmsLogTag::WMS_LAYOUT, "virtual screen, no need to add cross parent child");
            continue;
        }
        auto dragMoveMountedNode = GetWindowDragMoveMountedNode(displayId, this->GetZOrder());
        if (dragMoveMountedNode == nullptr) {
            TLOGE(WmsLogTag::WMS_LAYOUT, "dragMoveMountedNode is null");
            continue;
        }
        movedSurfaceNode->SetPositionZ(GetZOrder());
        movedSurfaceNode->SetIsCrossNode(true);
        RSTransactionAdapter::FlushImplicitTransaction(movedSurfaceNode->GetRSUIContext());
        {
            AutoRSTransaction trans(dragMoveMountedNode->GetRSUIContext());
            dragMoveMountedNode->AddCrossScreenChild(movedSurfaceNode, SUFFIX_INDEX, true);
        }
        cloneNodeCount_++;
        TLOGI(WmsLogTag::WMS_LAYOUT, "Add sub window to display:%{public}" PRIu64 " persistentId:%{public}d",
            displayId, GetPersistentId());
    }
}

std::set<uint64_t> SubSession::GetNewDisplayIdsDuringMoveTo(WSRect& newRect)
{
    std::set<uint64_t> newAddedDisplayIdSet;
    WSRect windowRect = newRect;
    std::map<ScreenId, ScreenProperty> screenProperties =
        ScreenSessionManagerClient::GetInstance().GetAllScreensProperties();
    std::lock_guard<std::mutex> lock(displayIdSetDuringMoveToMutex_);
    for (const auto& [screenId, screenProperty] : screenProperties) {
        if (displayIdSetDuringMoveTo_.find(screenId) != displayIdSetDuringMoveTo_.end()) {
            continue;
        }
        WSRect screenRect = {
            screenProperty.GetStartX(),
            screenProperty.GetStartY(),
            screenProperty.GetBounds().rect_.GetWidth(),
            screenProperty.GetBounds().rect_.GetHeight(),
        };
        if (windowRect.IsOverlap(screenRect)) {
            TLOGI(WmsLogTag::WMS_LAYOUT, "Overlap with new screen:%{public}" PRIu64 " persistentId:%{public}d",
                screenId, GetPersistentId());
            displayIdSetDuringMoveTo_.insert(screenId);
            newAddedDisplayIdSet.insert(screenId);
        }
    }
    return newAddedDisplayIdSet;
}

void SubSession::AddSurfaceNodeToScreen(DisplayId draggingOrMovingParentDisplayId)
{
    auto currSurfacedNode = GetSurfaceNodeForMoveDrag();
    if (currSurfacedNode == nullptr) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "SurfaceNode is null");
        return;
    }
    DisplayId currDisplayId = GetDisplayId();
    WSRect targetRect = GetLayoutController()->ConvertRelativeRectToGlobal(GetSessionRect(), currDisplayId);
    TLOGI(WmsLogTag::WMS_LAYOUT, "Id:%{public}d, currDisplayId:%{public}" PRIu64 ", winRect:%{public}s",
        GetPersistentId(), currDisplayId, GetSessionRect().ToString().c_str());
    for (const auto displayId : GetNewDisplayIdsDuringMoveTo(targetRect)) {
        if (displayId == currDisplayId && currDisplayId == draggingOrMovingParentDisplayId) {
            continue;
        }
        auto screenSession = ScreenSessionManagerClient::GetInstance().GetScreenSessionById(displayId);
        if (screenSession == nullptr) {
            TLOGE(WmsLogTag::WMS_LAYOUT, "ScreenSession is null");
            continue;
        }
        if (screenSession->GetDisplayNode() == nullptr) {
            TLOGE(WmsLogTag::WMS_LAYOUT, "DisplayNode is null");
            continue;
        }
        if (screenSession->GetScreenProperty().GetScreenType() == ScreenType::VIRTUAL) {
            TLOGD(WmsLogTag::WMS_LAYOUT, "virtual screen, no need to add cross parent child");
            continue;
        }
        auto dragMoveMountedNode = GetWindowDragMoveMountedNode(displayId, this->GetZOrder());
        if (dragMoveMountedNode == nullptr) {
            TLOGE(WmsLogTag::WMS_LAYOUT, "dragMoveMountedNode is null");
            continue;
        }
        currSurfacedNode->SetPositionZ(GetZOrder());
        currSurfacedNode->SetIsCrossNode(true);
        RSTransactionAdapter::FlushImplicitTransaction(currSurfacedNode->GetRSUIContext());
        {
            AutoRSTransaction trans(dragMoveMountedNode->GetRSUIContext());
            dragMoveMountedNode->AddCrossScreenChild(currSurfacedNode, SUFFIX_INDEX, true);
        }
        cloneNodeCount_++;
        TLOGI(WmsLogTag::WMS_LAYOUT, "Add sub window to display:%{public}" PRIu64 " persistentId:%{public}d, "
            "cloneNodeCount:%{public}d", displayId, GetPersistentId(), cloneNodeCount_);
    }
}

void SubSession::RemoveSurfaceNodeFromScreen()
{
    auto currSurfacedNode = GetSurfaceNodeForMoveDrag();
    if (currSurfacedNode == nullptr) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "SurfaceNode is null");
        return;
    }
    std::lock_guard<std::mutex> lock(displayIdSetDuringMoveToMutex_);
    for (const auto displayId : displayIdSetDuringMoveTo_) {
        auto screenSession = ScreenSessionManagerClient::GetInstance().GetScreenSessionById(displayId);
        if (screenSession == nullptr) {
            TLOGE(WmsLogTag::WMS_LAYOUT, "ScreenSession is null");
            continue;
        }
        if (screenSession->GetDisplayNode() == nullptr) {
            TLOGE(WmsLogTag::WMS_LAYOUT, "DisplayNode is null");
            continue;
        }
        if (screenSession->GetScreenProperty().GetScreenType() == ScreenType::VIRTUAL) {
            TLOGD(WmsLogTag::WMS_LAYOUT, "virtual screen, no need to remove cross parent child");
            continue;
        }
        currSurfacedNode->SetPositionZ(moveDragController_->GetOriginalPositionZ());
        screenSession->GetDisplayNode()->RemoveCrossScreenChild(currSurfacedNode);
        cloneNodeCount_--;
        currSurfacedNode->SetIsCrossNode(false);
        TLOGI(WmsLogTag::WMS_LAYOUT, "Remove sub window from display:%{public}" PRIu64 " persistentId:%{public}d, "
            "cloneNodeCount:%{public}d", displayId, GetPersistentId(), cloneNodeCount_);
    }
}
} // namespace OHOS::Rosen
