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

#ifndef OHOS_ROSEN_WINDOW_SCENE_SCB_SYSTEM_SESSION_H
#define OHOS_ROSEN_WINDOW_SCENE_SCB_SYSTEM_SESSION_H

#include "session/host/include/scene_session.h"

namespace OHOS::Rosen {
class SCBSystemSession : public SceneSession {
public:
    SCBSystemSession(const SessionInfo& info, const sptr<SpecificSessionCallback>& specificCallback,
        int32_t userId = 0);
    ~SCBSystemSession();
    WSError ProcessPointDownSession(int32_t posX, int32_t posY) override;

    WSError NotifyClientToUpdateRect(const std::string& updateReason,
        std::shared_ptr<RSTransaction> rsTransaction) override;
    void PresentFocusIfPointDown() override;
    WSError TransferKeyEvent(const std::shared_ptr<MMI::KeyEvent>& keyEvent) override;
    WSError UpdateFocus(bool isFocused) override;
    WSError UpdateWindowMode(WindowMode mode) override;
    WSError SetSystemSceneBlockingFocus(bool blocking) override;
    void BindKeyboardSession(sptr<SceneSession> session) override;
    sptr<SceneSession> GetKeyboardSession() const override;
    void SetSkipSelfWhenShowOnVirtualScreen(bool isSkip) override;
    void SetSkipEventOnCastPlus(bool isSkip) override;
    void SyncScenePanelGlobalPosition(bool needSync) override;
    bool IsNeedSyncScenePanelGlobalPosition() override { return isNeedSyncGlobalPos_; }
    bool IsVisibleForeground() const override;
    bool IsVisibleNotBackground() const override;

    /*
     * App Use Control
     */
    bool GetIsUseControlSession() const override;
    void SetIsUseControlSession(bool isUseControlSession) override;

protected:
    void UpdatePointerArea(const WSRect& rect) override;
    void NotifyClientToUpdateAvoidArea() override;

private:
    bool isNeedSyncGlobalPos_ = true; // can only accessed in main thread

    /*
     * App Use Control
     */
    bool isUseControlSession_ = false; // Indicates whether the session is used for controlling a main session.
};
} // namespace OHOS::Rosen
#endif // OHOS_ROSEN_WINDOW_SCENE_SCB_SYSTEM_SESSION_H
