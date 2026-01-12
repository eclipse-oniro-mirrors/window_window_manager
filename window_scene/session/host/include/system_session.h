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

#ifndef OHOS_ROSEN_WINDOW_SCENE_SYSTEM_SESSION_H
#define OHOS_ROSEN_WINDOW_SCENE_SYSTEM_SESSION_H

#include "session/host/include/scene_session.h"

namespace OHOS::Rosen {
class SystemSession : public SceneSession {
public:
    SystemSession(const SessionInfo& info, const sptr<SpecificSessionCallback>& specificCallback);
    ~SystemSession();

    void OnFirstStrongRef(const void* objectId) override;

    WSError Show(sptr<WindowSessionProperty> property) override;
    WSError Hide() override;
    WSError Disconnect(bool isFromClient = false, const std::string& identityToken = "") override;
    int32_t GetMissionId() const override;
    WSError ProcessPointDownSession(int32_t posX, int32_t posY) override;

    WSError TransferKeyEvent(const std::shared_ptr<MMI::KeyEvent>& keyEvent) override;
    WSError ProcessBackEvent() override;

    WSError NotifyClientToUpdateRect(const std::string& updateReason,
        std::shared_ptr<RSTransaction> rsTransaction) override;
    void RectCheck(uint32_t curWidth, uint32_t curHeight) override;
    bool IsVisibleForeground() const override;
    bool IsVisibleNotBackground() const override;
    WSError SetDialogSessionBackGestureEnabled(bool isEnabled) override;
    int32_t GetSubWindowZLevel() const override;

    /*
     * Floating Ball Window
     */
    WMError UpdateFloatingBall(const FloatingBallTemplateInfo& fbTemplateInfo) override;
    WSError StopFloatingBall() override;
    WMError GetFloatingBallWindowId(uint32_t& windowId) override;
    WMError RestoreFbMainWindow(const std::shared_ptr<AAFwk::Want>& want) override;
    WSError SendFbActionEvent(const std::string& action) override;
    FloatingBallTemplateInfo GetFbTemplateInfo() const override;
    void SetFbTemplateInfo(const FloatingBallTemplateInfo& fbTemplateInfo) override;
    uint32_t GetFbWindowId() const override;
    void SetFloatingBallUpdateCallback(NotifyUpdateFloatingBallFunc&& func) override;
    void SetFloatingBallStopCallback(NotifyStopFloatingBallFunc&& func) override;
    void SetFloatingBallRestoreMainWindowCallback(NotifyRestoreFloatingBallMainWindowFunc&& func) override;
    void RegisterGetFbPanelWindowIdFunc(GetFbPanelWindowIdFunc&& func) override;

    /*
     * Float Window
     */
    void SetRestoreFloatMainWindowCallback(NotifyRestoreFloatMainWindowFunc&& func) override;
    void RegisterGetSCBEnterRecentFunc(GetSCBEnterRecentFunc&& callback) override;
    WMError RestoreFloatMainWindow(const std::shared_ptr<AAFwk::WantParams>& wantParameters) override;
protected:
    bool CheckKeyEventDispatch(const std::shared_ptr<MMI::KeyEvent>& keyEvent) const;
    void UpdatePointerArea(const WSRect& rect) override;
    bool CheckPointerEventDispatch(const std::shared_ptr<MMI::PointerEvent>& pointerEvent) const override;

    /*
     * Floating Ball Window
     */
    void NotifyUpdateFloatingBall(const FloatingBallTemplateInfo& fbTemplateInfo) override;
    void NotifyStopFloatingBall() override;
    void NotifyRestoreFloatingBallMainWindow(const std::shared_ptr<AAFwk::Want>& want) override;

    /*
     * Float Window
     */
    void NotifyRestoreFloatMainWindow(const std::shared_ptr<AAFwk::WantParams>& wantParameters);
private:
    void UpdateCameraWindowStatus(bool isShowing);
    bool NeedSystemPermission(WindowType type);
    void UpdatePiPWindowStateChanged(bool isForeground);

    bool dialogSessionBackGestureEnabled_ = false;

    /*
     * Floating Ball Window
     */
    std::shared_mutex fbCallBackMutex_;
    bool needStopFb_ = false;
    std::shared_ptr<AAFwk::Want> fbWant_ = nullptr;

    GetFbPanelWindowIdFunc getFbPanelWindowIdFunc_;
    GetSCBEnterRecentFunc getSCBEnterRecentFunc_;
};
} // namespace OHOS::Rosen
#endif // OHOS_ROSEN_WINDOW_SCENE_SYSTEM_SESSION_H
