/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include "session/host/include/keyboard_session.h"
#include "session/host/include/session.h"
#include "common/include/session_permission.h"
#include "display_manager.h"
#include "screen_session_manager/include/screen_session_manager_client.h"
#include "session_helper.h"
#include <parameters.h>
#include "window_helper.h"
#include "window_manager_hilog.h"

namespace OHOS::Rosen {

KeyboardSession::KeyboardSession(const SessionInfo& info, const sptr<SpecificSessionCallback>& specificCallback,
    const sptr<KeyboardSessionCallback>& keyboardCallback)
    : SystemSession(info, specificCallback)
{
    keyboardCallback_ = keyboardCallback;
    TLOGI(WmsLogTag::WMS_KEYBOARD, "Create KeyboardSession");
}

KeyboardSession::~KeyboardSession()
{
    TLOGI(WmsLogTag::WMS_KEYBOARD, "~KeyboardSession");
}

WSError KeyboardSession::Show(sptr<WindowSessionProperty> property)
{
    auto task = [weakThis = wptr(this), property]() {
        auto session = weakThis.promote();
        if (!session) {
            TLOGE(WmsLogTag::WMS_KEYBOARD, "Session is null, show keyboard failed");
            return WSError::WS_ERROR_DESTROYED_OBJECT;
        }

        TLOGI(WmsLogTag::WMS_KEYBOARD, "Show keyboard session, id: %{public}d", session->GetPersistentId());
        auto ret = session->SceneSession::Foreground(property);
        session->OnKeyboardSessionShown();
        return ret;
    };
    PostTask(task, "Show");
    return WSError::WS_OK;
}

WSError KeyboardSession::Hide()
{
    auto task = [weakThis = wptr(this)]() {
        auto session = weakThis.promote();
        if (!session) {
            TLOGE(WmsLogTag::WMS_KEYBOARD, "Session is null, hide keyboard failed!");
            return WSError::WS_ERROR_DESTROYED_OBJECT;
        }

        TLOGI(WmsLogTag::WMS_KEYBOARD, "Hide keyboard session, set callingSessionId to 0, id: %{public}d",
            session->GetPersistentId());
        auto ret = session->SetActive(false);
        if (ret != WSError::WS_OK) {
            TLOGE(WmsLogTag::WMS_KEYBOARD, "Set session active state failed, ret: %{public}d", ret);
            return ret;
        }
        ret = session->SceneSession::Background();
        session->RestoreCallingSession();
        if (session->GetSessionProperty()) {
            session->GetSessionProperty()->SetCallingSessionId(INVALID_WINDOW_ID);
        }
        return ret;
    };
    PostTask(task, "Hide");
    return WSError::WS_OK;
}

WSError KeyboardSession::Disconnect(bool isFromClient)
{
    auto task = [weakThis = wptr(this), isFromClient]() {
        auto session = weakThis.promote();
        if (!session) {
            TLOGE(WmsLogTag::WMS_KEYBOARD, "Session is null, disconnect keyboard session failed!");
            return WSError::WS_ERROR_DESTROYED_OBJECT;
        }
        TLOGI(WmsLogTag::WMS_KEYBOARD, "Disconnect keyboard session, id: %{public}d, isFromClient: %{public}d",
            session->GetPersistentId(), isFromClient);
        session->SceneSession::Disconnect(isFromClient);
        session->RestoreCallingSession();
        if (session->GetSessionProperty()) {
            session->GetSessionProperty()->SetCallingSessionId(INVALID_WINDOW_ID);
        }
        return WSError::WS_OK;
    };
    PostTask(task, "Disconnect");
    return WSError::WS_OK;
}

WSError KeyboardSession::NotifyClientToUpdateRect(std::shared_ptr<RSTransaction> rsTransaction)
{
    auto task = [weakThis = wptr(this), rsTransaction]() {
        auto session = weakThis.promote();
        WSError ret = session->NotifyClientToUpdateRectTask(weakThis, rsTransaction);
        if (ret != WSError::WS_OK) {
            return ret;
        }
        session->RaiseCallingSession(true);
        if (session->specificCallback_ != nullptr && session->specificCallback_->onUpdateAvoidArea_ != nullptr) {
            session->specificCallback_->onUpdateAvoidArea_(session->GetPersistentId());
        }
        if (session->reason_ != SizeChangeReason::DRAG) {
            session->reason_ = SizeChangeReason::UNDEFINED;
            session->isDirty_ = false;
        }
        return ret;
    };
    PostTask(task, "NotifyClientToUpdateRect");
    return WSError::WS_OK;
}

WSError KeyboardSession::SetKeyboardSessionGravity(SessionGravity gravity, uint32_t percent)
{
    auto task = [weakThis = wptr(this), gravity, percent]() -> WSError {
        auto session = weakThis.promote();
        if (!session) {
            TLOGE(WmsLogTag::WMS_KEYBOARD, "keyboard session is null");
            return WSError::WS_ERROR_DESTROYED_OBJECT;
        }
        TLOGI(WmsLogTag::WMS_KEYBOARD, "keyboardId: %{public}d, gravity: %{public}d, percent: %{public}d",
            session->GetPersistentId(), gravity, percent);

        if (session->sessionChangeCallback_ && session->sessionChangeCallback_->onKeyboardGravityChange_) {
            session->sessionChangeCallback_->onKeyboardGravityChange_(gravity);
        }
        if (session->GetSessionProperty()) {
            session->GetSessionProperty()->SetKeyboardSessionGravity(gravity, percent);
        }
        session->RelayoutKeyBoard();
        if (gravity == SessionGravity::SESSION_GRAVITY_FLOAT) {
            session->SetWindowAnimationFlag(false);
            if (session->IsSessionForeground()) {
                session->RestoreCallingSession();
            }
        } else {
            session->SetWindowAnimationFlag(true);
            if (session->IsSessionForeground()) {
                session->RaiseCallingSession();
            }
        }
        return WSError::WS_OK;
    };
    PostTask(task, "SetKeyboardSessionGravity");
    return WSError::WS_OK;
}

void KeyboardSession::SetCallingSessionId(uint32_t callingSessionId)
{
    TLOGI(WmsLogTag::WMS_KEYBOARD, "Calling session id: %{public}d", callingSessionId);
    UpdateCallingSessionIdAndPosition(callingSessionId);
    if (keyboardCallback_ == nullptr || keyboardCallback_->onCallingSessionIdChange_ == nullptr ||
        GetSessionProperty() == nullptr) {
        TLOGE(WmsLogTag::WMS_KEYBOARD, "KeyboardCallback_ or sessionProperty is null, callingSessionId: %{public}d",
            callingSessionId);
        return;
    }
    keyboardCallback_->onCallingSessionIdChange_(GetSessionProperty()->GetCallingSessionId());
}

sptr<SceneSession> KeyboardSession::GetSceneSession(uint32_t persistentId)
{
    if (keyboardCallback_ == nullptr || keyboardCallback_->onGetSceneSession_ == nullptr) {
        TLOGE(WmsLogTag::WMS_KEYBOARD, "Get scene session failed, persistentId: %{public}d", persistentId);
        return nullptr;
    }
    return keyboardCallback_->onGetSceneSession_(persistentId);
}

int32_t KeyboardSession::GetFocusedSessionId()
{
    if (keyboardCallback_ == nullptr || keyboardCallback_->onGetFocusedSessionId_ == nullptr) {
        TLOGE(WmsLogTag::WMS_KEYBOARD, "keyboardCallback_ is nullptr, get focusedSessionId failed!");
        return INVALID_WINDOW_ID;
    }
    return keyboardCallback_->onGetFocusedSessionId_();
}

bool KeyboardSession::IsStatusBarVisible(const sptr<SceneSession>& statusBarSession)
{
    if (statusBarSession == nullptr) {
        return false;
    }
    if (statusBarSession->IsVisible()) {
        TLOGD(WmsLogTag::WMS_KEYBOARD, "Status Bar is at foreground, id: %{public}d",
            statusBarSession->GetPersistentId());
        return true;
    }
    TLOGD(WmsLogTag::WMS_KEYBOARD, "Status Bar is at background, id: %{public}d", statusBarSession->GetPersistentId());
    return false;
}

int32_t KeyboardSession::GetStatusBarHeight()
{
    int32_t statusBarHeight = 0;
    int32_t height = 0;
    if (specificCallback_ == nullptr || specificCallback_->onGetSceneSessionVectorByType_ == nullptr ||
        GetSessionProperty() == nullptr) {
        return statusBarHeight;
    }

    std::vector<sptr<SceneSession>> statusBarVector = specificCallback_->onGetSceneSessionVectorByType_(
        WindowType::WINDOW_TYPE_STATUS_BAR, GetSessionProperty()->GetDisplayId());
    for (const auto& statusBar : statusBarVector) {
        if (statusBar == nullptr || !IsStatusBarVisible(statusBar)) {
            continue;
        }
        height = statusBar->GetSessionRect().height_;
        statusBarHeight = (statusBarHeight > height) ? statusBarHeight : height;
    }
    TLOGI(WmsLogTag::WMS_KEYBOARD, "Status Bar height: %{public}d", statusBarHeight);
    return statusBarHeight;
}

void KeyboardSession::NotifyOccupiedAreaChangeInfo(const sptr<SceneSession>& callingSession, const WSRect& rect,
    const WSRect& occupiedArea)
{
    // if keyboard will occupy calling, notify calling window the occupied area and safe height
    const WSRect& safeRect = SessionHelper::GetOverlap(occupiedArea, rect, 0, 0);
    const WSRect& lastSafeRect = callingSession->GetLastSafeRect();
    if (lastSafeRect == safeRect) {
        TLOGI(WmsLogTag::WMS_KEYBOARD, "SafeRect is same to lastSafeRect: %{public}s", safeRect.ToString().c_str());
        return;
    }
    callingSession->SetLastSafeRect(safeRect);
    double textFieldPositionY = 0.0;
    double textFieldHeight = 0.0;
    if (GetSessionProperty() != nullptr) {
        textFieldPositionY = GetSessionProperty()->GetTextFieldPositionY();
        textFieldHeight = GetSessionProperty()->GetTextFieldHeight();
    }
    sptr<OccupiedAreaChangeInfo> info = new OccupiedAreaChangeInfo(OccupiedAreaType::TYPE_INPUT,
        SessionHelper::TransferToRect(safeRect), safeRect.height_, textFieldPositionY, textFieldHeight);
    TLOGI(WmsLogTag::WMS_KEYBOARD, "lastSafeRect: %{public}s, safeRect: %{public}s, keyboardRect: %{public}s, "
        "textFieldPositionY_: %{public}f, textFieldHeight_: %{public}f", lastSafeRect.ToString().c_str(),
        safeRect.ToString().c_str(), occupiedArea.ToString().c_str(), textFieldPositionY, textFieldHeight);
    callingSession->NotifyOccupiedAreaChangeInfo(info);
}

sptr<SceneSession> KeyboardSession::GetCallingSession()
{
    sptr<SceneSession> callingSession = nullptr;
    if (GetSessionProperty() != nullptr) {
        callingSession = GetSceneSession(GetSessionProperty()->GetCallingSessionId());
    }

    return callingSession;
}

void KeyboardSession::RaiseCallingSession(bool isKeyboardUpdated)
{
    sptr<SceneSession> callingSession = GetCallingSession();
    if (callingSession == nullptr) {
        TLOGI(WmsLogTag::WMS_KEYBOARD, "Calling session is nullptr");
        return;
    }
    SessionGravity gravity = SessionGravity::SESSION_GRAVITY_DEFAULT;
    uint32_t percent = 0;
    GetSessionProperty()->GetSessionGravity(gravity, percent);
    if (gravity == SessionGravity::SESSION_GRAVITY_FLOAT) {
        TLOGI(WmsLogTag::WMS_KEYBOARD, "No need to raise calling session, gravity: %{public}d", gravity);
        return;
    }

    WSRect callingSessionRect = callingSession->GetSessionRect();
    bool isCallingSessionFloating = (callingSession->GetWindowMode() == WindowMode::WINDOW_MODE_FLOATING);
    bool isMainOrParentFloating = WindowHelper::IsMainWindow(callingSession->GetWindowType()) ||
        (WindowHelper::IsSubWindow(callingSession->GetWindowType()) && callingSession->GetParentSession() != nullptr &&
         callingSession->GetParentSession()->GetWindowMode() == WindowMode::WINDOW_MODE_FLOATING);
    if (isCallingSessionFloating && isMainOrParentFloating &&
        (system::GetParameter("const.product.devicetype", "unknown") == "phone" ||
         system::GetParameter("const.product.devicetype", "unknown") == "tablet")) {
        return;
    }
    
    if (isKeyboardUpdated && isCallingSessionFloating) {
        callingSessionRect = callingSessionRestoringRect_;
    }
    const WSRect& keyboardSessionRect = (GetSessionRect().height_ != 0) ? GetSessionRect() : GetSessionRequestRect();
    if (SessionHelper::IsEmptyRect(SessionHelper::GetOverlap(keyboardSessionRect, callingSessionRect, 0, 0))) {
        TLOGI(WmsLogTag::WMS_KEYBOARD, "No overlap area, keyboardRect: %{public}s, callingSessionRect: %{public}s",
            keyboardSessionRect.ToString().c_str(), callingSessionRect.ToString().c_str());
        return;
    }

    if (!isKeyboardUpdated) {
        callingSessionRestoringRect_ = callingSessionRect;
    }

    WSRect newRect = callingSessionRect;
    int32_t statusHeight = GetStatusBarHeight();
    if (isCallingSessionFloating && callingSessionRect.posY_ > statusHeight) {
        // calculate new rect of calling window
        newRect.posY_ = std::max(keyboardSessionRect.posY_ - static_cast<int32_t>(newRect.height_), statusHeight);
        NotifyOccupiedAreaChangeInfo(callingSession, newRect, keyboardSessionRect);
        callingSession->UpdateSessionRect(newRect, SizeChangeReason::UNDEFINED);
        callingSessionRaisedRect_ = newRect;
    } else {
        NotifyOccupiedAreaChangeInfo(callingSession, newRect, keyboardSessionRect);
    }
    TLOGI(WmsLogTag::WMS_KEYBOARD, "keyboardRect: %{public}s, OriCallingRect: %{public}s, NewCallingRect: %{public}s"
        ", callingRestoreRect_: %{public}s, isCallingSessionFloating: %{public}d, isKeyboardUpdated: %{public}d",
        keyboardSessionRect.ToString().c_str(), callingSessionRect.ToString().c_str(), newRect.ToString().c_str(),
        callingSessionRestoringRect_.ToString().c_str(), isCallingSessionFloating, isKeyboardUpdated);
}

void KeyboardSession::RestoreCallingSession()
{
    sptr<SceneSession> callingSession = GetCallingSession();
    if (callingSession == nullptr) {
        TLOGI(WmsLogTag::WMS_KEYBOARD, "Calling session is nullptr");
        return;
    }

    TLOGI(WmsLogTag::WMS_KEYBOARD, "Calling session, RestoringRect_: %{public}s, RaisedRect_: %{public}s"
        ", curRect_: %{public}s, sessionMode: %{public}d", callingSessionRestoringRect_.ToString().c_str(),
        callingSessionRaisedRect_.ToString().c_str(), callingSession->GetSessionRect().ToString().c_str(),
        callingSession->GetWindowMode());
    WSRect overlapRect = { 0, 0, 0, 0 };
    NotifyOccupiedAreaChangeInfo(callingSession, callingSessionRestoringRect_, overlapRect);
    if (!SessionHelper::IsEmptyRect(callingSessionRestoringRect_) &&
        callingSession->GetWindowMode() == WindowMode::WINDOW_MODE_FLOATING &&
        callingSession->GetSessionRect() == callingSessionRaisedRect_) {
        callingSession->UpdateSessionRect(callingSessionRestoringRect_, SizeChangeReason::UNDEFINED);
    }
    callingSessionRestoringRect_ = { 0, 0, 0, 0 };
    callingSessionRaisedRect_ = { 0, 0, 0, 0 };
}

// Use focused session id when calling session id is invalid.
void KeyboardSession::UseFocusIdIfCallingSessionIdInvalid()
{
    if (GetCallingSession() != nullptr) {
        return;
    }
    int32_t focusedSessionId = GetFocusedSessionId();
    if (GetSceneSession(focusedSessionId) == nullptr) {
        TLOGE(WmsLogTag::WMS_KEYBOARD, "Focused session is null, id: %{public}d", focusedSessionId);
    } else {
        TLOGI(WmsLogTag::WMS_KEYBOARD, "Using focusedSession id: %{public}d", focusedSessionId);
            GetSessionProperty()->SetCallingSessionId(focusedSessionId);
    }
}

void KeyboardSession::OnKeyboardSessionShown()
{
    if (GetSessionProperty() == nullptr) {
        TLOGE(WmsLogTag::WMS_KEYBOARD, "Keyboard session property is nullptr, raise calling session failed.");
        return;
    }
    UseFocusIdIfCallingSessionIdInvalid();
    TLOGI(WmsLogTag::WMS_KEYBOARD, "Raise keyboard session, persistentId: %{public}d, callingSessionId: %{public}d",
        GetPersistentId(), GetSessionProperty()->GetCallingSessionId());
    RaiseCallingSession();
}

void KeyboardSession::UpdateCallingSessionIdAndPosition(uint32_t callingSessionId)
{
    if (GetSessionProperty() == nullptr) {
        TLOGE(WmsLogTag::WMS_KEYBOARD, "Session property is nullptr.");
        return;
    }
    uint32_t curSessionId = GetSessionProperty()->GetCallingSessionId();
    // When calling window id changes, restore the old calling session, raise the new calling session.
    if (curSessionId != INVALID_WINDOW_ID && callingSessionId != curSessionId) {
        TLOGI(WmsLogTag::WMS_KEYBOARD, "CallingSession curId: %{public}d, newId: %{public}d",
            curSessionId, callingSessionId);
        RestoreCallingSession();

        GetSessionProperty()->SetCallingSessionId(callingSessionId);
        UseFocusIdIfCallingSessionIdInvalid();
        RaiseCallingSession();
    } else {
        GetSessionProperty()->SetCallingSessionId(callingSessionId);
    }
}

void KeyboardSession::RelayoutKeyBoard()
{
    if (GetSessionProperty() == nullptr) {
        TLOGE(WmsLogTag::WMS_KEYBOARD, "Session property is nullptr, relayout keyboard failed");
        return;
    }
    SessionGravity gravity = SessionGravity::SESSION_GRAVITY_DEFAULT;
    uint32_t percent = 0;
    GetSessionProperty()->GetSessionGravity(gravity, percent);
    TLOGI(WmsLogTag::WMS_KEYBOARD, "Gravity: %{public}d, percent: %{public}d", gravity, percent);
    if (gravity == SessionGravity::SESSION_GRAVITY_FLOAT) {
        return;
    }
    auto displayId = GetSessionProperty()->GetDisplayId();
    auto screenSession = ScreenSessionManagerClient::GetInstance().GetScreenSession(displayId);
    uint32_t screenWidth = 0;
    uint32_t screenHeight = 0;
    if (screenSession != nullptr) {
        screenWidth = screenSession->GetScreenProperty().GetBounds().rect_.width_;
        screenHeight = screenSession->GetScreenProperty().GetBounds().rect_.height_;
    }

    auto requestRect = GetSessionProperty()->GetRequestRect();
    if (gravity == SessionGravity::SESSION_GRAVITY_BOTTOM) {
        requestRect.width_ = static_cast<uint32_t>(screenWidth);
        requestRect.posX_ = 0;
        if (percent != 0) {
            // 100: for calc percent.
            requestRect.height_ = static_cast<uint32_t>(screenHeight * percent / 100u);
        }
    }
    requestRect.posY_ = screenHeight - static_cast<int32_t>(requestRect.height_);
    GetSessionProperty()->SetRequestRect(requestRect);
    TLOGI(WmsLogTag::WMS_KEYBOARD, "Id: %{public}d, rect: %{public}s", GetPersistentId(),
        SessionHelper::TransferToWSRect(requestRect).ToString().c_str());
    UpdateSessionRect(SessionHelper::TransferToWSRect(requestRect), SizeChangeReason::UNDEFINED);
}
} // namespace OHOS::Rosen
