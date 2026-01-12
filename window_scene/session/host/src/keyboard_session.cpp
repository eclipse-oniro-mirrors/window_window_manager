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

#include "perform_reporter.h"
#include "session/host/include/keyboard_session.h"

#include <hitrace_meter.h>
#include "rs_adapter.h"
#include "screen_session_manager_client/include/screen_session_manager_client.h"
#include "session_helper.h"
#include <ui/rs_surface_node.h>
#include "window_helper.h"
#include "window_manager_hilog.h"

#define RETURN_IF_PARAM_IS_NULL(param, ...)                                   \
    do {                                                                      \
        if (!param) {                                                         \
            TLOGE(WmsLogTag::WMS_KEYBOARD, "The %{public}s is null", #param); \
            return __VA_ARGS__;                                               \
        }                                                                     \
    } while (false)                                                           \

namespace OHOS::Rosen {
namespace {
    constexpr float MOVE_DRAG_POSITION_Z = 100.5f;
    constexpr int32_t INSERT_TO_THE_END = -1;
}
KeyboardSession::KeyboardSession(const SessionInfo& info, const sptr<SpecificSessionCallback>& specificCallback,
    const sptr<KeyboardSessionCallback>& keyboardCallback, int32_t userId)
    : SystemSession(info, specificCallback, userId)
{
    keyboardCallback_ = keyboardCallback;
    scenePersistence_ = sptr<ScenePersistence>::MakeSptr(info.bundleName_, GetPersistentId());
    if (info.persistentId_ != 0 && info.persistentId_ != GetPersistentId()) {
        // persistentId changed due to id conflicts. Need to rename the old snapshot if exists
        scenePersistence_->RenameSnapshotFromOldPersistentId(info.persistentId_);
        TLOGI(WmsLogTag::WMS_KEYBOARD, "RenameSnapshotFromOldPersistentId %{public}d", info.persistentId_);
    }
    TLOGI(WmsLogTag::WMS_KEYBOARD, "Create KeyboardSession");
}

KeyboardSession::~KeyboardSession()
{
    TLOGI(WmsLogTag::WMS_KEYBOARD, "Destroy KeyboardSession");
}

void KeyboardSession::BindKeyboardPanelSession(sptr<SceneSession> panelSession)
{
    if (panelSession == nullptr) {
        TLOGE(WmsLogTag::WMS_KEYBOARD, "PanelSession is null");
        return;
    }
    keyboardPanelSession_ = panelSession;
    TLOGI(WmsLogTag::WMS_KEYBOARD, "Success, panelId: %{public}d", panelSession->GetPersistentId());
}

sptr<SceneSession> KeyboardSession::GetKeyboardPanelSession() const
{
    return keyboardPanelSession_;
}

SessionGravity KeyboardSession::GetKeyboardGravity() const
{
    SessionGravity gravity = static_cast<SessionGravity>(GetSessionProperty()->GetKeyboardLayoutParams().gravity_);
    TLOGD(WmsLogTag::WMS_KEYBOARD, "Gravity: %{public}d", gravity);
    return gravity;
}

WSError KeyboardSession::Show(sptr<WindowSessionProperty> property)
{
    if (property == nullptr) {
        TLOGE(WmsLogTag::WMS_KEYBOARD, "Session property is null");
        return WSError::WS_ERROR_NULLPTR;
    }
    if (!CheckPermissionWithPropertyAnimation(property)) {
        return WSError::WS_ERROR_NOT_SYSTEM_APP;
    }
    PostTask([weakThis = wptr(this), property]() {
        auto session = weakThis.promote();
        if (!session) {
            TLOGE(WmsLogTag::WMS_KEYBOARD, "Session is null, show keyboard failed");
            return WSError::WS_ERROR_DESTROYED_OBJECT;
        }
        if (session->systemConfig_.IsPcWindow()) {
            if (auto surfaceNode = session->GetSurfaceNode()) {
                surfaceNode->SetUIFirstSwitch(RSUIFirstSwitch::FORCE_DISABLE);
            }
        }
        if (session->GetKeyboardGravity() == SessionGravity::SESSION_GRAVITY_BOTTOM) {
            session->NotifySystemKeyboardAvoidChange(SystemKeyboardAvoidChangeReason::KEYBOARD_SHOW);
        }
        const auto targetDisplayId = property->GetDisplayId();
        auto sessionProperty = session->GetSessionProperty();
        sessionProperty->SetKeyboardEffectOption(property->GetKeyboardEffectOption());
        sessionProperty->SetDisplayId(targetDisplayId);
        session->UseFocusIdIfCallingSessionIdInvalid(property->GetCallingSessionId());
        TLOGNI(WmsLogTag::WMS_KEYBOARD,
            "Show keyboard session, id: %{public}d, calling id: %{public}d, targetDisplayId: %{public}" PRIu64 ", "
            "effectOption: %{public}s",
            session->GetPersistentId(),
            session->GetCallingSessionId(),
            targetDisplayId,
            property->GetKeyboardEffectOption().ToString().c_str());
        return session->SceneSession::Foreground(property);
    }, "Show");
    return WSError::WS_OK;
}

WSError KeyboardSession::Hide()
{
    if (!CheckPermissionWithPropertyAnimation(GetSessionProperty())) {
        return WSError::WS_ERROR_NOT_SYSTEM_APP;
    }
    PostTask([weakThis = wptr(this)]() {
        auto session = weakThis.promote();
        if (!session) {
            TLOGE(WmsLogTag::WMS_KEYBOARD, "Session is null, hide keyboard failed");
            return WSError::WS_ERROR_DESTROYED_OBJECT;
        }

        TLOGNI(WmsLogTag::WMS_KEYBOARD, "Hide keyboard session, set callingSessionId to 0, id: %{public}d",
            session->GetPersistentId());
        auto ret = session->SetActive(false);
        if (ret != WSError::WS_OK) {
            TLOGE(WmsLogTag::WMS_KEYBOARD, "Set session active state failed, ret: %{public}d", ret);
            return ret;
        }
        ret = session->SceneSession::Background();
        WSRect rect = {0, 0, 0, 0};
        session->NotifyKeyboardPanelInfoChange(rect, false);
        if (session->systemConfig_.IsPcWindow() || session->GetSessionScreenName() == "HiCar" ||
            session->GetSessionScreenName() == "SuperLauncher" ||
            session->GetSessionScreenName() == "PadWithCar") {
            TLOGD(WmsLogTag::WMS_KEYBOARD, "PC or virtual screen, restore calling session");
            !session->IsSystemKeyboard() ? session->RestoreCallingSession(session->GetCallingSessionId(), nullptr) :
                session->NotifySystemKeyboardAvoidChange(SystemKeyboardAvoidChangeReason::KEYBOARD_HIDE);
            auto sessionProperty = session->GetSessionProperty();
            if (sessionProperty) {
                sessionProperty->SetCallingSessionId(INVALID_WINDOW_ID);
            }
        }
        return ret;
    }, "Hide");
    return WSError::WS_OK;
}

WSError KeyboardSession::Disconnect(bool isFromClient, const std::string& identityToken)
{
    PostTask([weakThis = wptr(this), isFromClient]() {
        auto session = weakThis.promote();
        if (!session) {
            TLOGE(WmsLogTag::WMS_KEYBOARD, "Session is null, disconnect keyboard session failed");
            return WSError::WS_ERROR_DESTROYED_OBJECT;
        }
        TLOGI(WmsLogTag::WMS_KEYBOARD, "Disconnect keyboard session, id: %{public}d, isFromClient: %{public}d",
            session->GetPersistentId(), isFromClient);
        if (session->keyboardPanelSession_) {
            std::vector<Rect> keyboardPanelHotAreas;
            session->keyboardPanelSession_->GetSessionProperty()->SetTouchHotAreas(keyboardPanelHotAreas);
        }
        session->SceneSession::Disconnect(isFromClient);
        WSRect rect = {0, 0, 0, 0};
        session->NotifyKeyboardPanelInfoChange(rect, false);
        !session->IsSystemKeyboard() ? session->RestoreCallingSession(session->GetCallingSessionId(), nullptr) :
            session->NotifySystemKeyboardAvoidChange(SystemKeyboardAvoidChangeReason::KEYBOARD_DISCONNECT);
        return WSError::WS_OK;
    }, "Disconnect");
    return WSError::WS_OK;
}

WSError KeyboardSession::NotifyClientToUpdateRect(const std::string& updateReason,
    std::shared_ptr<RSTransaction> rsTransaction)
{
    PostTask([weakThis = wptr(this), rsTransaction, updateReason]() {
        auto session = weakThis.promote();
        if (!session) {
            TLOGE(WmsLogTag::WMS_KEYBOARD, "Session is null");
            return WSError::WS_ERROR_DESTROYED_OBJECT;
        }

        WSError ret = session->NotifyClientToUpdateRectTask(updateReason, rsTransaction);
        return ret;
    }, "NotifyClientToUpdateRect");
    return WSError::WS_OK;
}

void KeyboardSession::SetCallingSessionId(uint32_t callingSessionId)
{
    PostTask([weakThis = wptr(this), callingSessionId]() mutable {
        auto session = weakThis.promote();
        if (!session) {
            TLOGE(WmsLogTag::WMS_KEYBOARD, "Session is null");
            return;
        }
        if (session->GetSceneSession(callingSessionId) == nullptr) {
            uint32_t focusedSessionId = static_cast<uint32_t>(session->GetFocusedSessionId());
            if (session->GetSceneSession(focusedSessionId) == nullptr) {
                TLOGE(WmsLogTag::WMS_KEYBOARD, "Focused session is null, set id: %{public}d failed", focusedSessionId);
                return;
            } else {
                TLOGI(WmsLogTag::WMS_KEYBOARD, "Using focusedSession id: %{public}d", focusedSessionId);
                callingSessionId = focusedSessionId;
            }
        }
        uint32_t curCallingId = session->GetCallingSessionId();
        TLOGI(WmsLogTag::WMS_KEYBOARD, "CurId: %{public}d, newId: %{public}d", curCallingId, callingSessionId);
        // When the keyboard is shown, if the callingId changes, restore the cur calling session.
        if (curCallingId != INVALID_WINDOW_ID && callingSessionId != curCallingId && session->IsSessionForeground()) {
            session->RestoreCallingSession(curCallingId, nullptr);
            sptr<SceneSession> callingSession = session->GetSceneSession(curCallingId);
            WSRect panelRect = session->GetPanelRect();
            if (callingSession != nullptr && session->GetKeyboardGravity() == SessionGravity::SESSION_GRAVITY_BOTTOM) {
                session->RecalculatePanelRectForAvoidArea(panelRect);
                WSRect endRect = {panelRect.posX_, panelRect.posY_ + panelRect.height_, panelRect.width_,
                    panelRect.height_};
                // panelRect as beginRect
                callingSession->NotifyKeyboardAnimationCompleted(false, panelRect, endRect);
            }
        }
        session->GetSessionProperty()->SetCallingSessionId(callingSessionId);

        if (session->callingSessionIdChangeFunc_ == nullptr) {
            TLOGE(WmsLogTag::WMS_KEYBOARD, "KeyboardCallback_, callingSessionId: %{public}d", callingSessionId);
            return;
        }
        session->callingSessionIdChangeFunc_(callingSessionId);
    }, "SetCallingSessionId");
    return;
}

uint32_t KeyboardSession::GetCallingSessionId()
{
    auto sessionProperty = GetSessionProperty();
    if (sessionProperty == nullptr) {
        TLOGE(WmsLogTag::WMS_KEYBOARD, "Session property is null");
        return INVALID_SESSION_ID;
    }
    return sessionProperty->GetCallingSessionId();
}

bool KeyboardSession::isNeedProcessKeyboardOccupiedAreaInfo(
    const KeyboardLayoutParams& lastParams, const KeyboardLayoutParams& params)
{
    bool isKeyboardRectsChanged = (
        lastParams.LandscapeKeyboardRect_ != params.LandscapeKeyboardRect_ ||
        lastParams.PortraitKeyboardRect_ != params.PortraitKeyboardRect_ ||
        lastParams.LandscapePanelRect_ != params.LandscapePanelRect_ ||
        lastParams.PortraitPanelRect_ != params.PortraitPanelRect_);
    
    return IsSessionForeground() && lastParams != params && !isKeyboardRectsChanged &&
        lastParams.isValidAvoidHeight() && params.isValidAvoidHeight();
}

WSError KeyboardSession::AdjustKeyboardLayout(const KeyboardLayoutParams& params)
{
    PostTask([weakThis = wptr(this), params]() -> WSError {
        auto session = weakThis.promote();
        if (!session) {
            TLOGE(WmsLogTag::WMS_KEYBOARD, "Keyboard session is null");
            return WSError::WS_ERROR_DESTROYED_OBJECT;
        }
        // set keyboard layout params
        auto sessionProperty = session->GetSessionProperty();
        const KeyboardLayoutParams lastParams = sessionProperty->GetKeyboardLayoutParams();
        sessionProperty->SetKeyboardLayoutParams(params);
        // handle keyboard gravity change
        if (params.gravity_ == WindowGravity::WINDOW_GRAVITY_FLOAT) {
            session->NotifySystemKeyboardAvoidChange(SystemKeyboardAvoidChangeReason::KEYBOARD_GRAVITY_FLOAT);
            session->SetWindowAnimationFlag(false);
        } else {
            if (session->IsSessionForeground()) {
                session->NotifySystemKeyboardAvoidChange(SystemKeyboardAvoidChangeReason::KEYBOARD_GRAVITY_BOTTOM);
            }
            session->SetWindowAnimationFlag(true);
        }
        // avoidHeight is set and keyboardLayourParams is not changed, notify avoidArea in case ui params don't flush
        if (session->isNeedProcessKeyboardOccupiedAreaInfo(lastParams, params)) {
            TLOGI(WmsLogTag::WMS_KEYBOARD, "Keyboard avoidHeight is set, id: %{public}d",
                session->GetCallingSessionId());
            session->ProcessKeyboardOccupiedAreaInfo(session->GetCallingSessionId(), true, false);
        }
        // notify keyboard layout param
        if (session->adjustKeyboardLayoutFunc_) {
            session->adjustKeyboardLayoutFunc_(params);
        }
        TLOGNI(WmsLogTag::WMS_KEYBOARD, "Adjust keyboard layout, keyboardId: %{public}d, gravity: %{public}u, "
            "landscapeAvoidHeight: %{public}d, portraitAvoidHeight: %{public}d, "
            "Landscape: %{public}s, Portrait: %{public}s, LandscapePanel: %{public}s, "
            "PortraitPanel: %{public}s, dispId: %{public}" PRIu64, session->GetPersistentId(),
            static_cast<uint32_t>(params.gravity_), params.landscapeAvoidHeight_, params.portraitAvoidHeight_,
            params.LandscapeKeyboardRect_.ToString().c_str(), params.PortraitKeyboardRect_.ToString().c_str(),
            params.LandscapePanelRect_.ToString().c_str(), params.PortraitPanelRect_.ToString().c_str(),
            params.displayId_);
        return WSError::WS_OK;
    }, "AdjustKeyboardLayout");
    return WSError::WS_OK;
}

sptr<SceneSession> KeyboardSession::GetSceneSession(uint32_t persistentId)
{
    if (keyboardCallback_ == nullptr || keyboardCallback_->onGetSceneSession == nullptr) {
        TLOGE(WmsLogTag::WMS_KEYBOARD, "Get scene session failed, persistentId: %{public}d", persistentId);
        return nullptr;
    }
    return keyboardCallback_->onGetSceneSession(persistentId);
}

int32_t KeyboardSession::GetFocusedSessionId()
{
    if (keyboardCallback_ == nullptr || keyboardCallback_->onGetFocusedSessionId == nullptr) {
        TLOGE(WmsLogTag::WMS_KEYBOARD, "keyboardCallback is null, get focusedSessionId failed");
        return INVALID_WINDOW_ID;
    }
    return keyboardCallback_->onGetFocusedSessionId();
}

static WSRect CalculateSafeRectForMidScene(const WSRect& windowRect, const WSRect& keyboardRect, float scaleX,
    float scaleY)
{
    if (MathHelper::NearZero(scaleX) || MathHelper::NearZero(scaleY)) {
        return { 0, 0, 0, 0 };
    }
    const WSRect scaledWindowRect = {
        windowRect.posX_,
        windowRect.posY_,
        static_cast<int32_t>(windowRect.width_ * scaleX),
        static_cast<int32_t>(windowRect.height_ * scaleY)
    };

    const WSRect overlap = SessionHelper::GetOverlap(scaledWindowRect, keyboardRect, 0, 0);
    if (SessionHelper::IsEmptyRect(overlap)) {
        return { 0, 0, 0, 0 };
    }

    const WSRect result = {
        static_cast<int32_t>((overlap.posX_ - scaledWindowRect.posX_) / scaleX),
        static_cast<int32_t>((overlap.posY_ - scaledWindowRect.posY_) / scaleY),
        static_cast<int32_t>(overlap.width_ / scaleX),
        static_cast<int32_t>(overlap.height_ / scaleY)
    };
    return result;
}

void KeyboardSession::NotifyOccupiedAreaChanged(const sptr<SceneSession>& callingSession,
    sptr<OccupiedAreaChangeInfo>& occupiedAreaInfo,
    bool needRecalculateAvoidAreas, std::shared_ptr<RSTransaction> rsTransaction)
{
    if (occupiedAreaInfo == nullptr) {
        TLOGE(WmsLogTag::WMS_KEYBOARD, "occupiedAreaInfo is null");
        return;
    }
    if (callingSession->IsSystemSession()) {
        NotifyRootSceneOccupiedAreaChange(occupiedAreaInfo);
        TLOGI(WmsLogTag::WMS_KEYBOARD, "Calling id: %{public}d, safeRect: %{public}s"
            ", textFieldPositionY_: %{public}f, textFieldHeight_: %{public}f",
            callingSession->GetPersistentId(), occupiedAreaInfo->rect_.ToString().c_str(),
            occupiedAreaInfo->textFieldPositionY_, occupiedAreaInfo->textFieldHeight_);
    } else {
        std::map<AvoidAreaType, AvoidArea> avoidAreas = {};
        if (needRecalculateAvoidAreas) {
            callingSession->GetAllAvoidAreas(avoidAreas);
        }
        const WSRect& callingSessionRect = callingSession->GetSessionRect();
        callingSession->NotifyOccupiedAreaChangeInfo(occupiedAreaInfo, rsTransaction,
            SessionHelper::TransferToRect(callingSessionRect), avoidAreas);
    }
}

bool KeyboardSession::CalculateOccupiedArea(const sptr<SceneSession>& callingSession, const WSRect& callingSessionRect,
    const WSRect& panelRect, sptr<OccupiedAreaChangeInfo>& occupiedAreaInfo)
{
    if (callingSession == nullptr) {
        TLOGE(WmsLogTag::WMS_KEYBOARD, "callingSession is null");
        return false;
    }
    // if keyboard will occupy calling, notify calling window the occupied area and safe height
    const WSRect& safeRect = !callingSession->GetIsMidScene() ?
        SessionHelper::GetOverlap(panelRect, CalculateScaledRect(callingSessionRect, callingSession->GetScaleX(),
            callingSession->GetScaleY()), 0, 0) :
        CalculateSafeRectForMidScene(callingSessionRect, panelRect,
            callingSession->GetScaleX(), callingSession->GetScaleY());
    const WSRect& lastSafeRect = callingSession->GetLastSafeRect();
    if (lastSafeRect == safeRect) {
        TLOGI(WmsLogTag::WMS_KEYBOARD, "Same safeRect: %{public}s", safeRect.ToString().c_str());
        return false;
    }
    callingSession->SetLastSafeRect(safeRect);
    double textFieldPositionY = 0.0;
    double textFieldHeight = 0.0;
    auto sessionProperty = GetSessionProperty();
    if (sessionProperty != nullptr) {
        textFieldPositionY = sessionProperty->GetTextFieldPositionY();
        textFieldHeight = sessionProperty->GetTextFieldHeight();
    }
    occupiedAreaInfo = sptr<OccupiedAreaChangeInfo>::MakeSptr(OccupiedAreaType::TYPE_INPUT,
        SessionHelper::TransferToRect(safeRect), safeRect.height_, textFieldPositionY, textFieldHeight);
    return true;
}

void KeyboardSession::ForceProcessKeyboardOccupiedAreaInfo()
{
    if (IsVisibleForeground() && GetKeyboardGravity() == SessionGravity::SESSION_GRAVITY_BOTTOM) {
        TLOGI(WmsLogTag::WMS_KEYBOARD, "set CalculateOccupiedAreaWaitUntilDragEnd");
        isCalculateOccupiedAreaWaitUntilDragEnd_ = true;
    }
}

void KeyboardSession::ProcessKeyboardOccupiedAreaInfo(uint32_t callingId, bool needRecalculateAvoidAreas,
    bool needCheckRSTransaction)
{
    sptr<SceneSession> callingSession = GetSceneSession(callingId);
    if (callingSession == nullptr) {
        TLOGI(WmsLogTag::WMS_KEYBOARD, "Calling session is null");
        if (needCheckRSTransaction) {
            CloseRSTransaction();
        }
        return;
    }
    sptr<OccupiedAreaChangeInfo> occupiedAreaInfo = nullptr;
    std::shared_ptr<RSTransaction> rsTransaction = needCheckRSTransaction ? GetRSTransaction() : nullptr;
    bool occupiedAreaChanged = RaiseCallingSession(callingSession, occupiedAreaInfo);
    if (occupiedAreaChanged) {
        NotifyOccupiedAreaChanged(callingSession, occupiedAreaInfo, needRecalculateAvoidAreas, rsTransaction);
    }
    TLOGD(WmsLogTag::WMS_KEYBOARD, "id: %{public}d, needRecalculateAvoidAreas: %{public}d"
        ", occupiedAreaChanged: %{public}d", callingId, needRecalculateAvoidAreas, occupiedAreaChanged);
    if (needCheckRSTransaction) {
        CloseRSTransaction();
    }
}

void KeyboardSession::NotifyKeyboardPanelInfoChange(WSRect rect, bool isKeyboardPanelShow)
{
    if (!sessionStage_) {
        TLOGE(WmsLogTag::WMS_KEYBOARD, "SessionStage is null, notify keyboard panel rect change failed");
        return;
    }
    KeyboardPanelInfo keyboardPanelInfo;
    keyboardPanelInfo.rect_ = SessionHelper::TransferToRect(rect);
    keyboardPanelInfo.gravity_ = static_cast<WindowGravity>(GetKeyboardGravity());
    keyboardPanelInfo.isShowing_ = isKeyboardPanelShow;

    sessionStage_->NotifyKeyboardPanelInfoChange(keyboardPanelInfo);
}

bool KeyboardSession::CheckIfNeedRaiseCallingSession(sptr<SceneSession> callingSession, bool isCallingSessionFloating)
{
    if (callingSession == nullptr) {
        TLOGI(WmsLogTag::WMS_KEYBOARD, "Calling session is null");
        return false;
    }

    SessionGravity gravity = GetKeyboardGravity();
    if (gravity == SessionGravity::SESSION_GRAVITY_FLOAT) {
        TLOGI(WmsLogTag::WMS_KEYBOARD, "No need to raise calling session, gravity: %{public}d", gravity);
        return false;
    }

    /**
     * When an app calls move or resize, if the layout pipeline hasn't yet refreshed the window size, the calling
     * window dimensions obtained for keyboard avoidance will be incorrect. To prevent the app's intended dimensions
     * from being overridden, avoidance is deliberately skipped.
     */
    if (callingSession->isSubWindowResizingOrMoving_ && WindowHelper::IsSubWindow(callingSession->GetWindowType())) {
        TLOGI(WmsLogTag::WMS_KEYBOARD, "subWindow is resizing or moving");
        return false;
    }

    bool isMainOrParentFloating = WindowHelper::IsMainWindow(callingSession->GetWindowType()) ||
        (SessionHelper::IsNonSecureToUIExtension(callingSession->GetWindowType()) &&
         callingSession->GetParentSession() != nullptr &&
         callingSession->GetParentSession()->GetWindowMode() == WindowMode::WINDOW_MODE_FLOATING);
    bool isFreeMultiWindowMode = callingSession->IsFreeMultiWindowMode();
    bool isMidScene = callingSession->GetIsMidScene();
    bool isPhoneNotFreeMultiWindow = systemConfig_.IsPhoneWindow() && !isFreeMultiWindowMode;
    bool isPadNotFreeMultiWindow = systemConfig_.IsPadWindow() && !isFreeMultiWindowMode;
    if (isCallingSessionFloating && isMainOrParentFloating && !isMidScene &&
        (isPhoneNotFreeMultiWindow || isPadNotFreeMultiWindow)) {
        TLOGI(WmsLogTag::WMS_KEYBOARD, "No need to raise calling session in float window, "
            "isPhoneNotFreeMultiWindow: %{public}d, isPadNotFreeMultiWindow: %{public}d",
            isPhoneNotFreeMultiWindow, isPadNotFreeMultiWindow);
        return false;
    }

    return true;
}

bool KeyboardSession::RaiseCallingSession(const sptr<SceneSession>& callingSession,
    sptr<OccupiedAreaChangeInfo>& occupiedAreaInfo)
{
    bool occupiedAreaChanged = false;
    WSRect panelAvoidRect = GetPanelRect();
    if (!keyboardAvoidAreaActive_) {
        TLOGI(WmsLogTag::WMS_KEYBOARD, "Id: %{public}d, isSystemKeyboard: %{public}d, state: %{public}d, "
            "gravity: %{public}d", callingSession->GetPersistentId(), IsSystemKeyboard(), GetSessionState(),
            GetKeyboardGravity());
        return false;
    }
    if (!IsSessionForeground()) {
        TLOGI(WmsLogTag::WMS_KEYBOARD, "Keyboard is not foreground, sessionState: %{public}d", GetSessionState());
        return false;
    }
    NotifyKeyboardPanelInfoChange(panelAvoidRect, true);

    bool isCallingSessionFloating = (callingSession->GetWindowMode() == WindowMode::WINDOW_MODE_FLOATING) &&
        !callingSession->GetIsMidScene();
    if (!CheckIfNeedRaiseCallingSession(callingSession, isCallingSessionFloating)) {
        return false;
    }

    WSRect callingSessionRect = callingSession->GetSessionRect();
    float callingSessionScaleY = callingSession->GetScaleY();
    int32_t oriPosYBeforeRaisedByKeyboard = callingSession->GetOriPosYBeforeRaisedByKeyboard();
    if (oriPosYBeforeRaisedByKeyboard != 0 && isCallingSessionFloating) {
        callingSessionRect.posY_ = oriPosYBeforeRaisedByKeyboard;
    }
    // update panel rect for avoid area caculate
    RecalculatePanelRectForAvoidArea(panelAvoidRect);
    if (SessionHelper::IsEmptyRect(SessionHelper::GetOverlap(panelAvoidRect, CalculateScaledRect(
        callingSessionRect, callingSession->GetScaleX(), callingSessionScaleY), 0, 0)) &&
        oriPosYBeforeRaisedByKeyboard == 0) {
        TLOGI(WmsLogTag::WMS_KEYBOARD, "No overlap area, keyboardRect: %{public}s, callingRect: %{public}s",
            panelAvoidRect.ToString().c_str(), callingSessionRect.ToString().c_str());
        return CalculateOccupiedArea(callingSession, callingSessionRect, panelAvoidRect, occupiedAreaInfo);
    }

    WSRect newRect = callingSessionRect;
    int32_t statusHeight = callingSession->GetStatusBarHeight();
    int32_t scaledPosY = MathHelper::GreatNotEqual(callingSessionScaleY, 1) ?
        newRect.posY_ - static_cast<int32_t>(std::round((callingSessionScaleY - 1) * newRect.height_ / 2)) :
        newRect.posY_;
    if (IsNeedRaiseSubWindow(callingSession, newRect) && isCallingSessionFloating && scaledPosY > statusHeight) {
        if (oriPosYBeforeRaisedByKeyboard == 0) {
            oriPosYBeforeRaisedByKeyboard = callingSessionRect.posY_;
            callingSession->SetOriPosYBeforeRaisedByKeyboard(callingSessionRect.posY_);
        }
        // calculate new rect of calling session
        newRect.posY_ = std::max(panelAvoidRect.posY_ - newRect.height_, statusHeight);
        newRect.posY_ = std::min(oriPosYBeforeRaisedByKeyboard, newRect.posY_);
        if (MathHelper::GreatNotEqual(callingSessionScaleY, 1)) {
            scaledPosY = newRect.posY_ - static_cast<int32_t>(std::round(
                (callingSessionScaleY - 1) * newRect.height_ / 2));
            if (scaledPosY < statusHeight) {
                newRect.posY_ = newRect.posY_ + (statusHeight - scaledPosY);
            }
        }
        occupiedAreaChanged = CalculateOccupiedArea(callingSession, newRect, panelAvoidRect, occupiedAreaInfo);
        if (!IsSystemKeyboard()) {
            callingSession->UpdateSessionRect(newRect, SizeChangeReason::UNDEFINED);
        }
    } else {
        occupiedAreaChanged = CalculateOccupiedArea(callingSession, newRect, panelAvoidRect, occupiedAreaInfo);
    }

    TLOGI(WmsLogTag::WMS_KEYBOARD, "KeyboardRect: %{public}s, callSession OriRect: %{public}s, newRect: %{public}s"
        ", isFloating: %{public}d, oriPosY: %{public}d",
        panelAvoidRect.ToString().c_str(), callingSessionRect.ToString().c_str(), newRect.ToString().c_str(),
        isCallingSessionFloating, oriPosYBeforeRaisedByKeyboard);
    return occupiedAreaChanged;
}

WSRect KeyboardSession::CalculateScaledRect(WSRect sessionRect, float scaleX, float scaleY)
{
    if (!(MathHelper::GreatNotEqual(scaleY, 1) || MathHelper::GreatNotEqual(scaleX, 1))) {
        return sessionRect;
    }
    int32_t centerX = static_cast<int32_t>(sessionRect.posX_ + std::round(static_cast<float>(sessionRect.width_) / 2));
    int32_t centerY = static_cast<int32_t>(sessionRect.posY_ + std::round(static_cast<float>(sessionRect.height_) / 2));
    sessionRect.width_ = static_cast<int32_t>(std::round(sessionRect.width_ * scaleX));
    sessionRect.height_ = static_cast<int32_t>(std::round(sessionRect.height_ * scaleY));
    sessionRect.posX_ = centerX - static_cast<int32_t>(std::round(static_cast<float>(sessionRect.width_) / 2));
    sessionRect.posY_ = centerY - static_cast<int32_t>(std::round(static_cast<float>(sessionRect.height_) / 2));

    TLOGI(WmsLogTag::WMS_KEYBOARD, "scaledRect: %{public}s, scaleX: %{public}f, scaleY: %{public}f, centerX: %{public}d"
        ", centerY: %{public}d", sessionRect.ToString().c_str(), scaleX, scaleY, centerX, centerY);
    return sessionRect;
}

void KeyboardSession::RestoreCallingSession(uint32_t callingId, const std::shared_ptr<RSTransaction>& rsTransaction)
{
    if (!keyboardAvoidAreaActive_) {
        TLOGI(WmsLogTag::WMS_KEYBOARD, "Id: %{public}d, isSystemKeyboard: %{public}d, state: %{public}d, "
            "gravity: %{public}d", GetPersistentId(), IsSystemKeyboard(), GetSessionState(), GetKeyboardGravity());
        return;
    }
    sptr<SceneSession> callingSession = GetSceneSession(callingId);
    if (callingSession == nullptr) {
        TLOGI(WmsLogTag::WMS_KEYBOARD, "Calling session is null");
        return;
    }
    const WSRect& emptyRect = { 0, 0, 0, 0 };
    int32_t oriPosYBeforeRaisedByKeyboard = callingSession->GetOriPosYBeforeRaisedByKeyboard();
    sptr<OccupiedAreaChangeInfo> occupiedAreaInfo = nullptr;
    bool occupiedAreaChanged = CalculateOccupiedArea(callingSession, emptyRect, emptyRect, occupiedAreaInfo);
    if (occupiedAreaChanged) {
        NotifyOccupiedAreaChanged(callingSession, occupiedAreaInfo, true, rsTransaction);
    }
    if (oriPosYBeforeRaisedByKeyboard != 0 &&
        callingSession->GetWindowMode() == WindowMode::WINDOW_MODE_FLOATING) {
        WSRect callingSessionRestoringRect = callingSession->GetSessionRect();
        if (oriPosYBeforeRaisedByKeyboard != 0) {
            callingSessionRestoringRect.posY_ = oriPosYBeforeRaisedByKeyboard;
        }
        TLOGI(WmsLogTag::WMS_KEYBOARD, "OriPosYBeforeRaisedByKeyboard: %{public}d, sessionMode: %{public}d",
            oriPosYBeforeRaisedByKeyboard, callingSession->GetWindowMode());
        if (!IsSystemKeyboard()) {
            callingSession->UpdateSessionRect(callingSessionRestoringRect, SizeChangeReason::UNDEFINED);
        }
    }
    callingSession->SetOriPosYBeforeRaisedByKeyboard(0); // 0: default value
}

void KeyboardSession::HandleKeyboardMoveDragEnd(const WSRect& rect, SizeChangeReason reason, DisplayId displayId)
{
    PostTask([weakThis = wptr(this), rect, reason, displayId, where = __func__] {
        auto session = weakThis.promote();
        if (session == nullptr) {
            TLOGNE(WmsLogTag::WMS_KEYBOARD, "%{public}s session is null", where);
            return;
        }
        uint32_t screenWidth = 0;
        uint32_t screenHeight = 0;
        auto sessionProperty = session->GetSessionProperty();
        bool ret = session->GetScreenWidthAndHeightFromClient(sessionProperty, screenWidth, screenHeight);
        if (!ret) {
            TLOGNE(WmsLogTag::WMS_KEYBOARD, "%{public}s get screen size failed", where);
            return;
        }
        bool isLand = screenWidth > screenHeight;
        KeyboardLayoutParams params = sessionProperty->GetKeyboardLayoutParams();
        if (displayId != DISPLAY_ID_INVALID) {
            params.displayId_ = displayId;
        }
        if (isLand) {
            params.LandscapeKeyboardRect_.posX_ = rect.posX_;
            params.LandscapeKeyboardRect_.posY_ = rect.posY_;
            params.LandscapePanelRect_.posX_ = rect.posX_;
            params.LandscapePanelRect_.posY_ = rect.posY_;
        } else {
            params.PortraitKeyboardRect_.posX_ = rect.posX_;
            params.PortraitKeyboardRect_.posY_ = rect.posY_;
            params.PortraitPanelRect_.posX_ = rect.posX_;
            params.PortraitPanelRect_.posY_ = rect.posY_;
        }
        sessionProperty->SetKeyboardLayoutParams(params);
        if (session->adjustKeyboardLayoutFunc_) {
            session->adjustKeyboardLayoutFunc_(params);
        }
        TLOGI(WmsLogTag::WMS_KEYBOARD,
            "isLand:%{public}d, landRect:%{public}s, portraitRect:%{public}s, displayId:%{public}" PRIu64,
            isLand, params.LandscapeKeyboardRect_.ToString().c_str(),
            params.PortraitKeyboardRect_.ToString().c_str(), displayId);
    }, __func__ + GetRectInfo(rect));
}

// Use focused session id when calling session id is invalid.
void KeyboardSession::UseFocusIdIfCallingSessionIdInvalid(uint32_t callingSessionId)
{
    if (const auto callingSession = GetSceneSession(callingSessionId)) {
        GetSessionProperty()->SetCallingSessionId(callingSessionId);
        return;
    }

    const auto focusedSessionId = static_cast<uint32_t>(GetFocusedSessionId());
    if (GetSceneSession(focusedSessionId) == nullptr) {
        TLOGE(WmsLogTag::WMS_KEYBOARD, "Focused session is null, id: %{public}d", focusedSessionId);
    } else {
        TLOGI(WmsLogTag::WMS_KEYBOARD, "Using focusedSession id: %{public}d", focusedSessionId);
        GetSessionProperty()->SetCallingSessionId(focusedSessionId);
    }
}

void KeyboardSession::EnableCallingSessionAvoidArea()
{
    ProcessKeyboardOccupiedAreaInfo(GetCallingSessionId(), true, false);
}

void KeyboardSession::NotifySystemKeyboardAvoidChange(SystemKeyboardAvoidChangeReason reason)
{
    if (!systemConfig_.IsPcWindow()) {
        TLOGI(WmsLogTag::WMS_KEYBOARD, "This device is not pc");
        return;
    }
    if (!IsSystemKeyboard()) {
        TLOGI(WmsLogTag::WMS_KEYBOARD, "This is not system keyboard, id: %{public}d", GetPersistentId());
        return;
    }
    if (keyboardCallback_ == nullptr || keyboardCallback_->onSystemKeyboardAvoidChange == nullptr) {
        TLOGI(WmsLogTag::WMS_KEYBOARD, "SystemKeyboardAvoidChange callback is null, id: %{public}d",
            GetPersistentId());
        return;
    }
    keyboardCallback_->onSystemKeyboardAvoidChange(GetScreenId(), reason);
}

void KeyboardSession::OpenKeyboardSyncTransaction()
{
    auto task = [weakThis = wptr(this)]() {
        auto session = weakThis.promote();
        if (!session) {
            TLOGNE(WmsLogTag::WMS_KEYBOARD, "Keyboard session is null");
            return WSError::WS_ERROR_DESTROYED_OBJECT;
        }
        if (session->isKeyboardSyncTransactionOpen_) {
            TLOGNI(WmsLogTag::WMS_KEYBOARD, "Keyboard sync transaction is already open");
            return WSError::WS_OK;
        }
        TLOGNI(WmsLogTag::WMS_KEYBOARD, "Open keyboard sync");
        session->isKeyboardSyncTransactionOpen_ = true;
        auto transactionController = RSSyncTransactionController::GetInstance();
        if (transactionController) {
            transactionController->OpenSyncTransaction(session->GetEventHandler());
        }
        session->PostKeyboardAnimationSyncTimeoutTask();
        return WSError::WS_OK;
    };
    PostSyncTask(task);
}

void KeyboardSession::CloseKeyboardSyncTransaction(const WSRect& keyboardPanelRect,
    bool isKeyboardShow, const WindowAnimationInfo& animationInfo)
{
    PostTask([weakThis = wptr(this), keyboardPanelRect, isKeyboardShow, animationInfo]() {
        auto session = weakThis.promote();
        if (!session) {
            TLOGE(WmsLogTag::WMS_KEYBOARD, "Keyboard session is null");
            return WSError::WS_ERROR_DESTROYED_OBJECT;
        }
        auto callingId = animationInfo.callingId;
        TLOGNI(WmsLogTag::WMS_KEYBOARD, "Close keyboard sync, callingId: %{public}d, isShow: %{public}d,"
            " isGravityChanged: %{public}d", callingId, isKeyboardShow, animationInfo.isGravityChanged);
        std::shared_ptr<RSTransaction> rsTransaction = nullptr;
        if (session->isKeyboardSyncTransactionOpen_) {
            rsTransaction = session->GetRSTransaction();
        }
        // The callingId may change in WindowManager.
        // Use scb's callingId to properly handle callingWindow raise/restore.
        sptr<SceneSession> callingSession = session->GetSceneSession(callingId);
        if (callingSession != nullptr) {
            callingSession->NotifyKeyboardAnimationWillBegin(isKeyboardShow, animationInfo.beginRect,
                animationInfo.endRect, animationInfo.animated, rsTransaction);
        }
        bool isLayoutFinished = true;
        if (isKeyboardShow) {
            // When the keyboard shows/hides rapidly in succession,
            // the attributes aren't refreshed but occupied area info recalculation needs to be triggered.
            if (!animationInfo.isGravityChanged) {
                session->stateChanged_ = true;
            }
            // If the vsync period terminates, immediately notify all registered listeners.
            if (session->keyboardCallback_ != nullptr &&
                session->keyboardCallback_->isLastFrameLayoutFinished != nullptr) {
                isLayoutFinished = session->keyboardCallback_->isLastFrameLayoutFinished();
            }
            if (isLayoutFinished) {
                TLOGI(WmsLogTag::WMS_KEYBOARD, "vsync period completed, id: %{public}d", callingId);
                session->ProcessKeyboardOccupiedAreaInfo(callingId, true, true);
                session->stateChanged_ = false;
            }
            if (session->IsSessionForeground() && session->GetCallingSessionId() == INVALID_WINDOW_ID &&
                !animationInfo.isGravityChanged) {
                session->GetSessionProperty()->SetCallingSessionId(callingId);
            }
        } else {
            session->RestoreCallingSession(callingId, rsTransaction);
            if (!animationInfo.isGravityChanged) {
                session->GetSessionProperty()->SetCallingSessionId(INVALID_WINDOW_ID);
            }
        }
        if (isLayoutFinished) {
            session->CloseRSTransaction();
        }
        return WSError::WS_OK;
    }, "CloseKeyboardSyncTransaction");
}

void KeyboardSession::CloseRSTransaction()
{
    if (!isKeyboardSyncTransactionOpen_) {
        TLOGI(WmsLogTag::WMS_KEYBOARD, "Keyboard sync transaction is closed");
        return;
    }
    isKeyboardSyncTransactionOpen_ = false;
    auto handler = GetEventHandler();
    if (handler) {
        TLOGI(WmsLogTag::WMS_KEYBOARD, "cancelled");
        handler->RemoveTask(KEYBOARD_ANIM_SYNC_EVENT_NAME);
    }
    auto transactionController = RSSyncTransactionController::GetInstance();
    if (transactionController) {
        transactionController->CloseSyncTransaction(GetEventHandler());
    }
}

std::shared_ptr<RSTransaction> KeyboardSession::GetRSTransaction()
{
    auto transactionController = RSSyncTransactionController::GetInstance();
    std::shared_ptr<RSTransaction> rsTransaction = nullptr;
    if (transactionController) {
        rsTransaction = transactionController->GetRSTransaction();
    }
    return rsTransaction;
}

std::string KeyboardSession::GetSessionScreenName()
{
    auto sessionProperty = GetSessionProperty();
    if (sessionProperty != nullptr) {
        auto displayId = sessionProperty->GetDisplayId();
        auto screenSession = ScreenSessionManagerClient::GetInstance().GetScreenSession(displayId);
        if (screenSession != nullptr) {
            return screenSession->GetName();
        }
    }
    return "";
}

bool KeyboardSession::IsVisibleForeground() const
{
    return isVisible_;
}

bool KeyboardSession::IsVisibleNotBackground() const
{
    return isVisible_;
}

void KeyboardSession::RecalculatePanelRectForAvoidArea(WSRect& panelRect)
{
    auto sessionProperty = GetSessionProperty();
    KeyboardLayoutParams params = sessionProperty->GetKeyboardLayoutParams();
    if (params.landscapeAvoidHeight_ < 0 || params.portraitAvoidHeight_ < 0) {
        return;
    }
    // need to get screen property if the landscape width is same to the portrait
    if (params.LandscapePanelRect_.width_ != params.PortraitPanelRect_.width_) {
        if (static_cast<uint32_t>(panelRect.width_) == params.LandscapePanelRect_.width_) {
            panelRect.posY_ += panelRect.height_ - params.landscapeAvoidHeight_;
            panelRect.height_ = params.landscapeAvoidHeight_;
            TLOGI(WmsLogTag::WMS_KEYBOARD, "LandscapeAvoidHeight %{public}d", panelRect.height_);
            return;
        }
        if (static_cast<uint32_t>(panelRect.width_) == params.PortraitPanelRect_.width_) {
            panelRect.posY_ += panelRect.height_ - params.portraitAvoidHeight_;
            panelRect.height_ = params.portraitAvoidHeight_;
            TLOGI(WmsLogTag::WMS_KEYBOARD, "PortraitAvoidHeight %{public}d", panelRect.height_);
            return;
        }
    }
    uint32_t screenWidth = 0;
    uint32_t screenHeight = 0;
    bool result = GetScreenWidthAndHeightFromClient(sessionProperty, screenWidth, screenHeight);
    if (!result) {
        TLOGE(WmsLogTag::WMS_KEYBOARD, "Get screen width and height failed");
        return;
    }
    bool isLandscape = screenHeight < screenWidth;
    int32_t height_ = isLandscape ? params.landscapeAvoidHeight_ : params.portraitAvoidHeight_;
    panelRect.posY_ += panelRect.height_ - height_;
    panelRect.height_ = height_;
    TLOGI(WmsLogTag::WMS_KEYBOARD, "IsLandscape %{public}d, avoidHeight %{public}d", isLandscape, panelRect.height_);
}

WSError KeyboardSession::ChangeKeyboardEffectOption(const KeyboardEffectOption& effectOption)
{
    PostTask([weakThis = wptr(this), effectOption]() {
        auto session = weakThis.promote();
        if (!session) {
            TLOGNE(WmsLogTag::WMS_KEYBOARD, "Session is null, change keyboard effect option failed");
            return WSError::WS_ERROR_DESTROYED_OBJECT;
        }
        session->GetSessionProperty()->SetKeyboardEffectOption(effectOption);
        if (session->changeKeyboardEffectOptionFunc_) {
            session->changeKeyboardEffectOptionFunc_(effectOption);
            return WSError::WS_OK;
        }
        TLOGNE(WmsLogTag::WMS_KEYBOARD, "Can not found changeKeyboardEffectOptionFunc_");
        return WSError::WS_OK;
    }, __func__);
    return WSError::WS_OK;
}

void KeyboardSession::NotifyRootSceneOccupiedAreaChange(const sptr<OccupiedAreaChangeInfo>& info)
{
    ScreenId defaultScreenId = ScreenSessionManagerClient::GetInstance().GetDefaultScreenId();
    auto displayId = GetSessionProperty()->GetDisplayId();
    if (displayId != defaultScreenId) {
        TLOGI(WmsLogTag::WMS_KEYBOARD, "DisplayId: %{public}" PRIu64", defaultScreenId: %{public}" PRIu64"",
            displayId, defaultScreenId);
        return;
    }
    if (keyboardCallback_ == nullptr || keyboardCallback_->onNotifyOccupiedAreaChange == nullptr) {
        TLOGE(WmsLogTag::WMS_KEYBOARD, "Callback is null");
        return;
    }
    keyboardCallback_->onNotifyOccupiedAreaChange(info);
}

bool KeyboardSession::IsNeedRaiseSubWindow(const sptr<SceneSession>& callingSession, const WSRect& callingSessionRect)
{
    if (!SessionHelper::IsSubWindow(callingSession->GetWindowType())) {
        TLOGD(WmsLogTag::WMS_KEYBOARD, "Not sub window");
        return true;
    }

    auto mainSession = callingSession->GetMainSession();
    if (mainSession != nullptr && WindowHelper::IsSplitWindowMode(mainSession->GetWindowMode()) &&
        callingSessionRect == mainSession->GetSessionRect()) {
        TLOGI(WmsLogTag::WMS_KEYBOARD, "No need to raise, parentId: %{public}d, rect: %{public}s",
            mainSession->GetPersistentId(), callingSessionRect.ToString().c_str());
        return false;
    }

    return true;
}

void KeyboardSession::HandleCrossScreenChild(bool isMoveOrDrag)
{
    RETURN_IF_PARAM_IS_NULL(moveDragController_);
    auto displayIds = isMoveOrDrag ?
        moveDragController_->GetNewAddedDisplayIdsDuringMoveDrag() :
        moveDragController_->GetDisplayIdsDuringMoveDrag();
    for (const auto displayId : displayIds) {
        if (displayId == moveDragController_->GetMoveDragStartDisplayId()) {
            continue;
        }
        auto screenSession = ScreenSessionManagerClient::GetInstance().GetScreenSessionById(displayId);
        if (screenSession == nullptr) {
            TLOGE(WmsLogTag::WMS_KEYBOARD, "ScreenSession is null");
            continue;
        }
        if (screenSession->GetScreenProperty().GetScreenType() == ScreenType::VIRTUAL) {
            TLOGI(WmsLogTag::WMS_KEYBOARD, "virtual screen, no need to add cross parent child");
            continue;
        }
        RETURN_IF_PARAM_IS_NULL(keyboardPanelSession_);
        auto keyboardPanelSurfaceNode = keyboardPanelSession_->GetSurfaceNode();
        RETURN_IF_PARAM_IS_NULL(keyboardPanelSurfaceNode);
        auto dragMoveMountedNode = GetWindowDragMoveMountedNode(displayId, this->GetZOrder());
        if (dragMoveMountedNode == nullptr) {
            TLOGE(WmsLogTag::WMS_LAYOUT, "dragMoveMountedNode is null");
            continue;
        }
        if (isMoveOrDrag) {
            {
                AutoRSTransaction trans(keyboardPanelSurfaceNode->GetRSUIContext());
                keyboardPanelSurfaceNode->SetPositionZ(MOVE_DRAG_POSITION_Z);
                keyboardPanelSurfaceNode->SetIsCrossNode(true);
            }
            {
                AutoRSTransaction trans(dragMoveMountedNode->GetRSUIContext());
                dragMoveMountedNode->AddCrossScreenChild(keyboardPanelSurfaceNode, INSERT_TO_THE_END, true);
            }
            TLOGI(WmsLogTag::WMS_KEYBOARD, "Add window: %{public}d to display: %{public}" PRIu64,
                keyboardPanelSession_->GetPersistentId(), displayId);
        } else {
            keyboardPanelSurfaceNode->SetPositionZ(moveDragController_->GetOriginalPositionZ());
            dragMoveMountedNode->RemoveCrossScreenChild(keyboardPanelSurfaceNode);
            keyboardPanelSurfaceNode->SetIsCrossNode(false);
            TLOGI(WmsLogTag::WMS_KEYBOARD, "Remove window: %{public}d from display: %{public}" PRIu64,
                keyboardPanelSession_->GetPersistentId(), displayId);
        }
    }
}

void KeyboardSession::HandleMoveDragSurfaceNode(SizeChangeReason reason)
{
    if (reason == SizeChangeReason::DRAG || reason == SizeChangeReason::DRAG_MOVE) {
        HandleCrossScreenChild(true);
    } else if (reason == SizeChangeReason::DRAG_END) {
        HandleCrossScreenChild(false);
    }
}

void KeyboardSession::SetSurfaceBounds(const WSRect& rect, bool isGlobal, bool needFlush)
{
    HITRACE_METER_FMT(HITRACE_TAG_WINDOW_MANAGER,
        "KeyboardSession::SetSurfaceBounds id:%d [%d, %d, %d, %d] reason:%u",
        GetPersistentId(), rect.posX_, rect.posY_, rect.width_, rect.height_, GetSizeChangeReason());
    TLOGD(WmsLogTag::WMS_KEYBOARD, "id: %{public}d, rect: %{public}s isGlobal: %{public}d needFlush: %{public}d",
        GetPersistentId(), rect.ToString().c_str(), isGlobal, needFlush);
    {
        AutoRSTransaction trans(GetRSUIContext(), needFlush);
        if (keyboardPanelSession_ == nullptr) {
            TLOGE(WmsLogTag::WMS_KEYBOARD, "keyboard panel session is null");
            return;
        }
        auto surfaceNode = keyboardPanelSession_->GetSurfaceNode();
        if (surfaceNode == nullptr) {
            TLOGE(WmsLogTag::WMS_KEYBOARD, "keyboard panel surfacenode is null");
            return;
        }
        surfaceNode->SetGlobalPositionEnabled(isGlobal);
        surfaceNode->SetBounds(rect.posX_, rect.posY_, rect.width_, rect.height_);
        surfaceNode->SetFrame(rect.posX_, rect.posY_, rect.width_, rect.height_);
    }
}

void KeyboardSession::SetKeyboardEffectOptionChangeListener(const NotifyKeyboarEffectOptionChangeFunc& func)
{
    PostTask([weakThis = wptr(this), func, where = __func__] {
        auto session = weakThis.promote();
        if (!session || !func) {
            TLOGNE(WmsLogTag::WMS_KEYBOARD, "%{public}s session or NotifyKeyboarEffectOptionChangeFunc is null",
                where);
            return WSError::WS_ERROR_DESTROYED_OBJECT;
        }
        session->changeKeyboardEffectOptionFunc_ = func;
        TLOGND(WmsLogTag::WMS_KEYBOARD, "%{public}s Register changeKeyboardEffectOptionFunc_ success", where);
        return WSError::WS_OK;
    }, __func__);
}

WSRect KeyboardSession::GetPanelRect() const
{
    WSRect panelRect = { 0, 0, 0, 0 };
    if (keyboardPanelSession_ != nullptr) {
        panelRect = keyboardPanelSession_->GetSessionRect();
    } else {
        TLOGE(WmsLogTag::WMS_KEYBOARD, "Panel session is null, get panel rect failed");
    }
    return panelRect;
}

void KeyboardSession::SetSkipSelfWhenShowOnVirtualScreen(bool isSkip)
{
    TLOGI(WmsLogTag::WMS_KEYBOARD, "Set Skip keyboard, isSkip: %{public}d", isSkip);
    PostTask([weakThis = wptr(this), isSkip, where = __func__]() {
        auto session = weakThis.promote();
        if (!session) {
            TLOGNE(WmsLogTag::WMS_KEYBOARD, "%{public}s: Session is null", where);
            return;
        }
        std::shared_ptr<RSSurfaceNode> surfaceNode = session->GetSurfaceNode();
        if (!surfaceNode) {
            TLOGNE(WmsLogTag::WMS_KEYBOARD, "%{public}s: SurfaceNode is null", where);
            return;
        }
        if (session->specificCallback_ != nullptr
            && session->specificCallback_->onSetSkipSelfWhenShowOnVirtualScreen_ != nullptr) {
            session->specificCallback_->onSetSkipSelfWhenShowOnVirtualScreen_(surfaceNode->GetId(), isSkip);
        }
    }, __func__);
}

void KeyboardSession::PostKeyboardAnimationSyncTimeoutTask()
{
    // anim_sync_exception
    int32_t const THRESHOLD = 1000;
    auto task = [weakThis = wptr(this)]() {
        auto session = weakThis.promote();
        if (!session) {
            TLOGNE(WmsLogTag::WMS_KEYBOARD, "keyboard session is null");
            return;
        }
        if (!session->GetIsKeyboardSyncTransactionOpen()) {
            TLOGND(WmsLogTag::WMS_KEYBOARD, "closed anim_sync in time");
            return;
        }
        std::string msg("close anim_sync timeout");
        WindowInfoReporter::GetInstance().ReportKeyboardLifeCycleException(
            session->GetPersistentId(),
            KeyboardLifeCycleException::ANIM_SYNC_EXCEPTION,
            msg);
    };
    auto handler = GetEventHandler();
    if (!handler) {
        TLOGE(WmsLogTag::WMS_KEYBOARD, "handler is null");
        return;
    }
    handler->PostTask(task, KEYBOARD_ANIM_SYNC_EVENT_NAME, THRESHOLD);
}

void KeyboardSession::SetSkipEventOnCastPlus(bool isSkip)
{
    PostTask([weakThis = wptr(this), isSkip, where = __func__]() {
        auto session = weakThis.promote();
        if (!session) {
            TLOGNE(WmsLogTag::WMS_SCB, "%{public}s session is null", where);
            return;
        }
        TLOGNI(WmsLogTag::WMS_SCB, "%{public}s wid: %{public}d, isSkip: %{public}d", where,
            session->GetPersistentId(), isSkip);
        if (session->specificCallback_ != nullptr &&
            session->specificCallback_->onSetSkipEventOnCastPlus_ != nullptr) {
            session->specificCallback_->onSetSkipEventOnCastPlus_(session->GetPersistentId(), isSkip);
        }
    }, __func__);
}

WSError KeyboardSession::UpdateSizeChangeReason(SizeChangeReason reason)
{
    PostTask([weakThis = wptr(this), reason, where = __func__]() {
        auto keyboardSession = weakThis.promote();
        if (keyboardSession == nullptr) {
            TLOGE(WmsLogTag::WMS_KEYBOARD, "%{public}s session is null", where);
            return WSError::WS_ERROR_DESTROYED_OBJECT;
        }
        if (reason == SizeChangeReason::DRAG_START || reason == SizeChangeReason::DRAG_MOVE ||
            reason == SizeChangeReason::DRAG_END || reason == SizeChangeReason::UNDEFINED) {
            auto panelSession = keyboardSession->GetKeyboardPanelSession();
            if (panelSession != nullptr) {
                panelSession->UpdateSizeChangeReason(reason);
            }
        }
        TLOGD(WmsLogTag::WMS_KEYBOARD, "%{public}s Id: %{public}d, reason: %{public}d",
            where, keyboardSession->GetPersistentId(), static_cast<int32_t>(reason));
        keyboardSession->SceneSession::UpdateSizeChangeReason(reason);
        return WSError::WS_OK;
    }, __func__);
    return WSError::WS_OK;
}

void KeyboardSession::CalculateOccupiedAreaAfterUIRefresh()
{
    bool needRecalculateOccupiedArea = false;
    const uint32_t keyboardDirtyFlags = GetDirtyFlags();
    if ((keyboardDirtyFlags & static_cast<uint32_t>(SessionUIDirtyFlag::VISIBLE)) !=
        static_cast<uint32_t>(SessionUIDirtyFlag::NONE) ||
        (keyboardDirtyFlags & static_cast<uint32_t>(SessionUIDirtyFlag::RECT)) !=
        static_cast<uint32_t>(SessionUIDirtyFlag::NONE) || stateChanged_) {
        HandleLayoutAvoidAreaUpdate(AvoidAreaType::TYPE_KEYBOARD);
        TLOGD(WmsLogTag::WMS_KEYBOARD, "Keyboard panel rect has changed");
        needRecalculateOccupiedArea = true;
    }
    // Recalculate the occupied area info when calling session rect changes && keyboard is visible.
    uint32_t callingId = GetCallingSessionId();
    sptr<SceneSession> callingSession = GetSceneSession(callingId);
    if (callingSession && (callingSession->GetDirtyFlags() & static_cast<uint32_t>(SessionUIDirtyFlag::RECT)) !=
        static_cast<uint32_t>(SessionUIDirtyFlag::NONE) && IsVisibleForeground()) {
        SizeChangeReason reason = callingSession->GetSizeChangeReason();
        // Skip recalculation during drag operations and reset oriPosYBeforeRaisedByKeyboard_.
        if ((reason >= SizeChangeReason::DRAG && reason <= SizeChangeReason::DRAG_END) ||
            reason == SizeChangeReason::DRAG_MOVE) {
            if (callingSession->GetOriPosYBeforeRaisedByKeyboard() != 0) {
                TLOGI(WmsLogTag::WMS_KEYBOARD, "Skip recalculation and reset oriPosYBeforeRaisedByKeyboard_,"
                    " id: %{public}d", callingId);
                callingSession->SetOriPosYBeforeRaisedByKeyboard(0);
            }
        } else {
            TLOGD(WmsLogTag::WMS_KEYBOARD, "Calling session rect has changed");
            needRecalculateOccupiedArea = true;
        }
        if (reason == SizeChangeReason::DRAG_END && isCalculateOccupiedAreaWaitUntilDragEnd_) {
            needRecalculateOccupiedArea = true;
            isCalculateOccupiedAreaWaitUntilDragEnd_ = false;
            TLOGI(WmsLogTag::WMS_KEYBOARD, "CalculateOccupiedAreaWaitUntilDragEnd");
        }
    }
    if (needRecalculateOccupiedArea) {
        ProcessKeyboardOccupiedAreaInfo(callingId, false, stateChanged_);
        stateChanged_ = false;
    }
}

WMError KeyboardSession::HandleActionUpdateKeyboardTouchHotArea(const sptr<WindowSessionProperty>& property,
    WSPropertyChangeAction action)
{
    if (GetWindowType() != WindowType::WINDOW_TYPE_INPUT_METHOD_FLOAT) {
        return WMError::WM_ERROR_INVALID_TYPE;
    }
    auto sessionProperty = GetSessionProperty();
    KeyboardTouchHotAreas keyboardTouchHotAreas = property->GetKeyboardTouchHotAreas();
    auto displayId = (keyboardTouchHotAreas.displayId_ == DISPLAY_ID_INVALID) ?
        sessionProperty->GetDisplayId() : keyboardTouchHotAreas.displayId_;
    bool isLandscape = false;
    WMError ret = IsLandscape(displayId, isLandscape);
    if (ret != WMError::WM_OK) {
        return ret;
    }
    std::vector<Rect>& keyboardHotAreas = isLandscape ?
        keyboardTouchHotAreas.landscapeKeyboardHotAreas_ : keyboardTouchHotAreas.portraitKeyboardHotAreas_;
    std::vector<Rect>& panelHotAreas = isLandscape ?
        keyboardTouchHotAreas.landscapePanelHotAreas_ : keyboardTouchHotAreas.portraitPanelHotAreas_;
    if (keyboardTouchHotAreas != sessionProperty->GetKeyboardTouchHotAreas()) {
        PrintRectsInfo(keyboardHotAreas, "keyboardHotAreas");
        PrintRectsInfo(panelHotAreas, "panelHotAreas");
    }
    sessionProperty->SetTouchHotAreas(keyboardHotAreas);
    keyboardPanelSession_->GetSessionProperty()->SetTouchHotAreas(panelHotAreas);
    sessionProperty->SetKeyboardTouchHotAreas(keyboardTouchHotAreas);
    return WMError::WM_OK;
}

WMError KeyboardSession::IsLandscape(uint64_t displayId, bool& isLandscape)
{
    ScreenProperty screenProperty;
    std::map<ScreenId, ScreenProperty> screensProperties =
        ScreenSessionManagerClient::GetInstance().GetAllScreensProperties();
    if (screensProperties.find(displayId) != screensProperties.end()) {
        screenProperty = screensProperties[displayId];
    } else {
        ScreenId screenId = ScreenSessionManagerClient::GetInstance().GetDefaultScreenId();
        TLOGI(WmsLogTag::WMS_KEYBOARD, "Use default screenId: %{public}" PRIu64
            ",invalid displayId: %{public}" PRIu64"", screenId, displayId);
        if (screensProperties.find(screenId) != screensProperties.end()) {
            screenProperty = screensProperties[screenId];
            displayId = screenId;
        } else {
            TLOGE(WmsLogTag::WMS_KEYBOARD, "invalid screen id: %{public}" PRIu64, screenId);
            return WMError::WM_ERROR_INVALID_DISPLAY;
        }
    }
    auto displayRect = screenProperty.GetBounds().rect_;
    int32_t displayWidth = displayRect.GetWidth();
    int32_t displayHeight = displayRect.GetHeight();
    isLandscape = displayWidth > displayHeight;
    if (displayWidth == displayHeight) {
        DisplayOrientation orientation = screenProperty.GetDisplayOrientation();
        if (orientation == DisplayOrientation::UNKNOWN) {
            TLOGW(WmsLogTag::WMS_KEYBOARD, "Display orientation is UNKNOWN");
        }
        isLandscape = (orientation == DisplayOrientation::LANDSCAPE ||
            orientation == DisplayOrientation::LANDSCAPE_INVERTED);
    }
    auto display = DisplayManager::GetInstance().GetDisplayById(displayId);
    std::string dispName = (display != nullptr) ? display->GetName() : "UNKNOWN";
    TLOGI(WmsLogTag::WMS_KEYBOARD, "s-displayInfo: %{public}" PRIu64 ", %{public}d|%{public}d|%{public}d|%{public}s",
        displayId, displayWidth, displayHeight, isLandscape, dispName.c_str());
    return WMError::WM_OK;
}

void KeyboardSession::PrintRectsInfo(const std::vector<Rect>& rects, const std::string& infoTag)
{
    std::ostringstream oss;
    for (const auto& rect : rects) {
        oss << "[" << rect.posX_ << "," << rect.posY_ << "," << rect.width_ << "," << rect.height_ << "]";
    }
    TLOGI(WmsLogTag::WMS_KEYBOARD, "%{public}s: %{public}s", infoTag.c_str(), oss.str().c_str());
}
} // namespace OHOS::Rosen