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

#ifndef OHOS_ROSEN_WINDOW_SCENE_SUB_SESSION_H
#define OHOS_ROSEN_WINDOW_SCENE_SUB_SESSION_H

#include "session/host/include/scene_session.h"

namespace OHOS::Rosen {
class SubSession : public SceneSession {
public:
    SubSession(const SessionInfo& info, const sptr<SpecificSessionCallback>& specificCallback);
    ~SubSession();

    WSError Show(sptr<WindowSessionProperty> property) override;
    WSError Hide() override;
    WSError ProcessPointDownSession(int32_t posX, int32_t posY) override;
    int32_t GetMissionId() const override;
    WSError TransferKeyEvent(const std::shared_ptr<MMI::KeyEvent>& keyEvent) override;
    void RectCheck(uint32_t curWidth, uint32_t curHeight) override;
    bool IsVisibleForeground() const override;

protected:
    void UpdatePointerArea(const WSRect& rect) override;
    bool CheckPointerEventDispatch(const std::shared_ptr<MMI::PointerEvent>& pointerEvent) const override;

    /*
     * Window Hierarchy
     */
    bool IsTopmost() const override;
    bool IsModal() const override;
    bool IsApplicationModal() const override;
};
} // namespace OHOS::Rosen
#endif // OHOS_ROSEN_WINDOW_SCENE_SUB_SESSION_H
