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
using KeyboardPanelRectUpdateCallback = std::function<void()>;
class SCBSystemSession : public SceneSession {
public:
    SCBSystemSession(const SessionInfo& info, const sptr<SpecificSessionCallback>& specificCallback);
    ~SCBSystemSession();
    void RegisterBufferAvailableCallback(const SystemSessionBufferAvailableCallback& func) override;
    WSError ProcessPointDownSession(int32_t posX, int32_t posY) override;

    WSError NotifyClientToUpdateRect(std::shared_ptr<RSTransaction> rsTransaction) override;
    void PresentFocusIfPointDown() override;
    WSError TransferKeyEvent(const std::shared_ptr<MMI::KeyEvent>& keyEvent) override;
    void PresentFoucusIfNeed(int32_t pointerAcrion) override;
    WSError UpdateFocus(bool isFocused) override;
    WSError UpdateWindowMode(WindowMode mode) override;
    WSError SetSystemSceneBlockingFocus(bool blocking) override;
    void BindKeyboardSession(sptr<SceneSession> session) override;
    sptr<SceneSession> GetKeyboardSession() const override;
    void SetKeyboardPanelRectUpdateCallback(const KeyboardPanelRectUpdateCallback& func);
    void SetSkipSelfWhenShowOnVirtualScreen(bool isSkip) override;
    std::shared_ptr<RSSurfaceNode> GetSurfaceNode();

    bool IsVisibleForeground() const override;

protected:
    void UpdatePointerArea(const WSRect& rect) override;
    void NotifyClientToUpdateAvoidArea() override;

private:
    KeyboardPanelRectUpdateCallback keyboardPanelRectUpdateCallback_;
};
} // namespace OHOS::Rosen
#endif // OHOS_ROSEN_WINDOW_SCENE_SCB_SYSTEM_SESSION_H
