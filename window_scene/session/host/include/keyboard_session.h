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

#ifndef OHOS_ROSEN_WINDOW_SCENE_KEYBOARD_SESSION_H
#define OHOS_ROSEN_WINDOW_SCENE_KEYBOARD_SESSION_H

#include "session/host/include/system_session.h"
#include "transaction/rs_sync_transaction_controller.h"

namespace OHOS::Rosen {
enum class SystemKeyboardAvoidChangeReason : uint32_t {
    KEYBOARD_BEGIN,
    KEYBOARD_CREATED,
    KEYBOARD_SHOW,
    KEYBOARD_HIDE,
    KEYBOARD_DISCONNECT,
    KEYBOARD_GRAVITY_BOTTOM,
    KEYBOARD_GRAVITY_FLOAT,
    KEYBOARD_END,
};

using OnGetSceneSessionCallback = std::function<sptr<SceneSession>(uint32_t callingSessionId)>;
using OnGetFocusedSessionIdCallback = std::function<int32_t()>;
using OnCallingSessionIdChangeCallback = std::function<void(uint32_t callingSessionId)>;
using OnSystemKeyboardAvoidChangeCallback = std::function<void(DisplayId displayId,
    SystemKeyboardAvoidChangeReason reason)>;
using NotifyOccupiedAreaChangeCallback = std::function<void(const sptr<OccupiedAreaChangeInfo>& info)>;
using IsLastFrameLayoutFinishedCallback = std::function<bool()>;

const std::string KEYBOARD_ANIM_SYNC_EVENT_NAME { "KeyboardAnimationSyncException" };

class KeyboardSession : public SystemSession {
public:
    // callback for notify SceneSessionManager
    struct KeyboardSessionCallback : public RefBase {
        OnGetSceneSessionCallback onGetSceneSession;
        OnGetFocusedSessionIdCallback onGetFocusedSessionId;
        OnCallingSessionIdChangeCallback onCallingSessionIdChange;
        OnSystemKeyboardAvoidChangeCallback onSystemKeyboardAvoidChange;
        NotifyOccupiedAreaChangeCallback onNotifyOccupiedAreaChange;
        IsLastFrameLayoutFinishedCallback isLastFrameLayoutFinished;
    };
    KeyboardSession(const SessionInfo& info, const sptr<SpecificSessionCallback>& specificCallback,
        const sptr<KeyboardSessionCallback>& keyboardCallback, int32_t userId = 0);
    ~KeyboardSession();

    WSError Show(sptr<WindowSessionProperty> property) override;
    WSError Hide() override;
    WSError Disconnect(bool isFromClient = false, const std::string& identityToken = "") override;
    WSError NotifyClientToUpdateRect(const std::string& updateReason,
        std::shared_ptr<RSTransaction> rsTransaction) override;

    void BindKeyboardPanelSession(sptr<SceneSession> panelSession) override;
    sptr<SceneSession> GetKeyboardPanelSession() const override;
    SessionGravity GetKeyboardGravity() const override;
    void OpenKeyboardSyncTransaction() override;
    void CloseKeyboardSyncTransaction(const WSRect& keyboardPanelRect, bool isKeyboardShow,
        const WindowAnimationInfo& animationInfo) override;
    bool IsVisibleForeground() const override;
    bool IsVisibleNotBackground() const override;
    uint32_t GetCallingSessionId() override;
    void RecalculatePanelRectForAvoidArea(WSRect& panelRect) override;
    WSError ChangeKeyboardEffectOption(const KeyboardEffectOption& effectOption) override;
    void SetKeyboardEffectOptionChangeListener(const NotifyKeyboarEffectOptionChangeFunc& func) override;
    void SetSkipSelfWhenShowOnVirtualScreen(bool isSkip) override;
    WSError UpdateSizeChangeReason(SizeChangeReason reason) override;
    bool GetIsKeyboardSyncTransactionOpen() const { return isKeyboardSyncTransactionOpen_; }
    void SetSkipEventOnCastPlus(bool isSkip) override;
    void ForceProcessKeyboardOccupiedAreaInfo() override;

protected:
    void EnableCallingSessionAvoidArea() override;
    void HandleKeyboardMoveDragEnd(const WSRect& rect, SizeChangeReason reason = SizeChangeReason::UNDEFINED,
        DisplayId displayId = DISPLAY_ID_INVALID) override;
    void RestoreCallingSession(uint32_t callingId, const std::shared_ptr<RSTransaction>& rsTransaction) override;

private:
    sptr<SceneSession> GetSceneSession(uint32_t persistentId);
    int32_t GetFocusedSessionId();
    WSRect GetPanelRect() const;

    void SetCallingSessionId(uint32_t callingSessionId) override;
    void UseFocusIdIfCallingSessionIdInvalid(uint32_t callingSessionId);
    void NotifyKeyboardPanelInfoChange(WSRect rect, bool isKeyboardPanelShow);
    bool CheckIfNeedRaiseCallingSession(sptr<SceneSession> callingSession, bool isCallingSessionFloating);
    WSError AdjustKeyboardLayout(const KeyboardLayoutParams& params) override;
    std::shared_ptr<RSTransaction> GetRSTransaction();
    std::string GetSessionScreenName();
    void NotifySystemKeyboardAvoidChange(SystemKeyboardAvoidChangeReason reason);
    void NotifyRootSceneOccupiedAreaChange(const sptr<OccupiedAreaChangeInfo>& info);
    void HandleCrossScreenChild(bool isMoveOrDrag);
    void HandleMoveDragSurfaceNode(SizeChangeReason reason) override;
    void SetSurfaceBounds(const WSRect& rect, bool isGlobal, bool needFlush = true) override;
    bool IsNeedRaiseSubWindow(const sptr<SceneSession>& callingSession, const WSRect& callingSessionRect);
    void PostKeyboardAnimationSyncTimeoutTask();
    void ProcessKeyboardOccupiedAreaInfo(uint32_t callingId, bool needRecalculateAvoidAreas,
        bool needCheckRSTransaction);
    void NotifyOccupiedAreaChanged(const sptr<SceneSession>& callingSession,
        sptr<OccupiedAreaChangeInfo>& occupiedAreaInfo, bool needRecalculateAvoidAreas,
        std::shared_ptr<RSTransaction> rsTransaction);
    bool RaiseCallingSession(const sptr<SceneSession>& callingSession, sptr<OccupiedAreaChangeInfo>& occupiedAreaInfo);
    bool CalculateOccupiedArea(const sptr<SceneSession>& callingSession, const WSRect& callingSessionRect,
        const WSRect& panelRect, sptr<OccupiedAreaChangeInfo>& occupiedAreaInfo);
    void CloseRSTransaction();
    bool stateChanged_ = false;
    bool isNeedProcessKeyboardOccupiedAreaInfo(
        const KeyboardLayoutParams& lastParams, const KeyboardLayoutParams& params);
    void CalculateOccupiedAreaAfterUIRefresh() override;
    WSRect CalculateScaledRect(WSRect sessionRect, float scaleX, float scaleY);
    WMError HandleActionUpdateKeyboardTouchHotArea(const sptr<WindowSessionProperty>& property,
        WSPropertyChangeAction action) override;
    
    sptr<KeyboardSessionCallback> keyboardCallback_ = nullptr;
    bool isKeyboardSyncTransactionOpen_ = false;
    NotifyKeyboarEffectOptionChangeFunc changeKeyboardEffectOptionFunc_;
    bool isCalculateOccupiedAreaWaitUntilDragEnd_ = false;
    WMError IsLandscape(uint64_t screenId, bool& isLandscape);
    void PrintRectsInfo(const std::vector<Rect>& rects, const std::string& infoTag);
};
} // namespace OHOS::Rosen
#endif // OHOS_ROSEN_WINDOW_SCENE_KEYBOARD_SESSION_H