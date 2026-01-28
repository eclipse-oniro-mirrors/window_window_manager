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

    void OnFirstStrongRef(const void* objectId) override;

    WSError Show(sptr<WindowSessionProperty> property) override;
    WSError Hide() override;
    WSError HideSync() override;
    WSError Hide(bool needSyncHide);
    WSError ProcessPointDownSession(int32_t posX, int32_t posY) override;
    int32_t GetMissionId() const override;
    WSError TransferKeyEvent(const std::shared_ptr<MMI::KeyEvent>& keyEvent) override;
    void RectCheck(uint32_t curWidth, uint32_t curHeight) override;
    bool IsVisibleForeground() const override;
    bool IsVisibleNotBackground() const override;
    bool IsNeedCrossDisplayRendering() const override;
    void HandleCrossMoveToSurfaceNode(WSRect& globalRect) override;
    std::set<uint64_t> GetNewDisplayIdsDuringMoveTo(WSRect& newRect);
    void HandleCrossSurfaceNodeByWindowAnchor(SizeChangeReason reason, DisplayId displayId) override;

    void SetParentSessionCallback(NotifySetParentSessionFunc&& func) override;
    WMError NotifySetParentSession(int32_t oldParentWindowId, int32_t newParentWindowId) override;

protected:
    void UpdatePointerArea(const WSRect& rect) override;
    bool CheckPointerEventDispatch(const std::shared_ptr<MMI::PointerEvent>& pointerEvent) const override;

    /*
     * Window Layout
     */
    void NotifySessionRectChange(const WSRect& rect,
        SizeChangeReason reason = SizeChangeReason::UNDEFINED, DisplayId displayId = DISPLAY_ID_INVALID,
        const RectAnimationConfig& rectAnimationConfig = {}) override;
    void UpdateSessionRectInner(const WSRect& rect, SizeChangeReason reason,
        const MoveConfiguration& moveConfiguration, const RectAnimationConfig& rectAnimationConfig = {}) override;

    /*
     * Window Hierarchy
     */
    bool IsTopmost() const override;
    bool IsModal() const override;
    bool IsApplicationModal() const override;
    WSError SetSubWindowZLevel(int32_t zLevel) override;
    int32_t GetSubWindowZLevel() const override;

    /*
     * PC Window
     */
    WSError NotifyFollowParentMultiScreenPolicy(bool enabled) override;
    bool IsFollowParentMultiScreenPolicy() const override;
    bool isFollowParentMultiScreenPolicy_ = false;

    /*
     * Sub Window
     */
    NotifySetParentSessionFunc setParentSessionFunc_;

private:
    /*
     * Window Layout
     */
    std::atomic<int32_t> cloneNodeCountDuringCross_ = 0;
    void AddSurfaceNodeToScreen(DisplayId draggingOrMovingParentDisplayId) override;
    void RemoveSurfaceNodeFromScreen() override;
};
} // namespace OHOS::Rosen
#endif // OHOS_ROSEN_WINDOW_SCENE_SUB_SESSION_H
