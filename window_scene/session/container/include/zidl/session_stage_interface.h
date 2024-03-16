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

#ifndef OHOS_WINDOW_SCENE_SESSION_STAGE_INTERFACE_H
#define OHOS_WINDOW_SCENE_SESSION_STAGE_INTERFACE_H

#include <iremote_broker.h>
#include <list>
#include <map>

#include "interfaces/include/ws_common.h"
#include "occupied_area_change_info.h"
#include "window_drawing_content_info.h"

namespace OHOS::MMI {
class PointerEvent;
class KeyEvent;
class AxisEvent;
} // namespace MMI
namespace OHOS::Accessibility {
class AccessibilityElementInfo;
}
namespace OHOS::Rosen {
class RSTransaction;

class ISessionStage : public IRemoteBroker {
public:
    DECLARE_INTERFACE_DESCRIPTOR(u"OHOS.ISessionStage");

    // IPC interface
    virtual WSError SetActive(bool active) = 0;
    virtual WSError UpdateRect(const WSRect& rect, SizeChangeReason reason,
        const std::shared_ptr<RSTransaction>& rsTransaction = nullptr) = 0;
    virtual void UpdateDensity() = 0;
    virtual WSError HandleBackEvent() = 0;
    virtual WSError MarkProcessed(int32_t eventId) = 0;
    virtual WSError UpdateFocus(bool isFocused) = 0;
    virtual WSError NotifyDestroy() = 0;
    virtual WSError NotifyCloseExistPipWindow() = 0;
    virtual WSError NotifyTransferComponentData(const AAFwk::WantParams& wantParams)
    {
        return WSError::WS_OK;
    }
    virtual WSErrorCode NotifyTransferComponentDataSync(const AAFwk::WantParams& wantParams,
        AAFwk::WantParams& reWantParams)
    {
        return WSErrorCode::WS_OK;
    }
    virtual void NotifyOccupiedAreaChangeInfo(sptr<OccupiedAreaChangeInfo> info) = 0;
    virtual WSError UpdateAvoidArea(const sptr<AvoidArea>& avoidArea, AvoidAreaType type) = 0;
    virtual void NotifyScreenshot() = 0;
    virtual void DumpSessionElementInfo(const std::vector<std::string>& params) = 0;
    virtual WSError NotifyTouchOutside() = 0;
    virtual WSError NotifyWindowVisibility(bool isVisible) = 0;
    virtual WSError UpdateWindowMode(WindowMode mode) = 0;
    virtual void NotifyForegroundInteractiveStatus(bool interactive) = 0;
    virtual WSError UpdateMaximizeMode(MaximizeMode mode) = 0;
    virtual void NotifySessionForeground(uint32_t reason, bool withAnimation) = 0;
    virtual void NotifySessionBackground(uint32_t reason, bool withAnimation, bool isFromInnerkits) = 0;
    virtual WSError UpdateTitleInTargetPos(bool isShow, int32_t height) = 0;
    virtual void NotifyTransformChange(const Transform& transform) = 0;
    virtual WSError NotifyDialogStateChange(bool isForeground)
    {
        return WSError::WS_OK;
    }


    // **Non** IPC interface
    virtual void NotifyBackpressedEvent(bool& isConsumed) {}
    virtual void NotifyPointerEvent(const std::shared_ptr<MMI::PointerEvent>& pointerEvent) {}
    virtual void NotifyKeyEvent(const std::shared_ptr<MMI::KeyEvent>& keyEvent, bool& isConsumed,
        bool notifyInputMethod = true) {}
    virtual void NotifyFocusActiveEvent(bool isFocusActive) {}
    virtual void NotifyFocusStateEvent(bool focusState) {}
    virtual int32_t GetPersistentId() const
    {
        return -1;
    }
    virtual WSError NotifySearchElementInfoByAccessibilityId(int64_t elementId, int32_t mode, int64_t baseParent,
        std::list<Accessibility::AccessibilityElementInfo>& infos)
    {
        return WSError::WS_OK;
    }
    virtual WSError NotifySearchElementInfosByText(int64_t elementId, const std::string& text, int64_t baseParent,
        std::list<Accessibility::AccessibilityElementInfo>& infos)
    {
        return WSError::WS_OK;
    }
    virtual WSError NotifyFindFocusedElementInfo(int64_t elementId, int32_t focusType, int64_t baseParent,
        Accessibility::AccessibilityElementInfo& info)
    {
        return WSError::WS_OK;
    }
    virtual WSError NotifyFocusMoveSearch(int64_t elementId, int32_t direction, int64_t baseParent,
        Accessibility::AccessibilityElementInfo& info)
    {
        return WSError::WS_OK;
    }
    virtual WSError NotifyExecuteAction(int64_t elementId, const std::map<std::string, std::string>& actionArguments,
        int32_t action, int64_t baseParent)
    {
        return WSError::WS_OK;
    }
    virtual WSError NotifyAccessibilityHoverEvent(float pointX, float pointY, int32_t sourceType, int32_t eventType,
        int64_t timeMs)
    {
        return WSError::WS_OK;
    }
};
} // namespace OHOS::Rosen
#endif // OHOS_WINDOW_SCENE_SESSION_STAGE_INTERFACE_H
