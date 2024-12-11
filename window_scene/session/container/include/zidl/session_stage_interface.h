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
        const SceneAnimationConfig& config = { nullptr, ROTATE_ANIMATION_DURATION }) = 0;
    virtual void UpdateDensity() = 0;
    virtual WSError UpdateOrientation() = 0;
    virtual WSError HandleBackEvent() = 0;
    virtual WSError MarkProcessed(int32_t eventId) = 0;
    virtual WSError UpdateFocus(bool isFocused) = 0;
    virtual WSError NotifyDestroy() = 0;

    /**
     * @brief Notify client to close the existing pip window.
     *
     * Called when starting pip but there is already a pip window foreground. The previous one will be destroyed if
     * the new starting request has a higher priority.
     *
     * @return Returns WSError::WS_OK if called success, otherwise failed.
     */
    virtual WSError NotifyCloseExistPipWindow() = 0;
    virtual WSError NotifyTransferComponentData(const AAFwk::WantParams& wantParams) = 0;
    virtual WSErrorCode NotifyTransferComponentDataSync(const AAFwk::WantParams& wantParams,
        AAFwk::WantParams& reWantParams) = 0;
    virtual void NotifyOccupiedAreaChangeInfo(sptr<OccupiedAreaChangeInfo> info,
                                              const std::shared_ptr<RSTransaction>& rsTransaction = nullptr) = 0;
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
    virtual WSError NotifyDialogStateChange(bool isForeground) = 0;

    /**
     * @brief Set pip event to client.
     *
     * Set the pip event to client. Such as close, restore, destroy events.
     *
     * @param action Indicates the action name.
     * @param status Indicates the status num.
     * @return Returns WSError::WS_OK if called success, otherwise failed.
     */
    virtual WSError SetPipActionEvent(const std::string& action, int32_t status) = 0;

    /**
     * @brief Set the media control event to client.
     *
     * Set the media control event to client. The event is from pip control panel operation.
     *
     * @param controlType Indicates the {@link WsPiPControlType} component in pip control panel.
     * @param status Indicates the {@link WsPiPControlStatus} required state of specified component.
     * @return Returns WSError::WS_OK if called success, otherwise failed.
     */
    virtual WSError SetPiPControlEvent(WsPiPControlType controlType, WsPiPControlStatus status) = 0;
    virtual WSError UpdateDisplayId(uint64_t displayId) = 0;
    virtual void NotifyDisplayMove(DisplayId from, DisplayId to) = 0;
    virtual WSError SwitchFreeMultiWindow(bool enable) = 0;
    virtual WSError NotifyCompatibleModeEnableInPad(bool enable)
    {
        return WSError::WS_OK;
    }
    virtual void SetUniqueVirtualPixelRatio(bool useUniqueDensity, float virtualPixelRatio) = 0;
    virtual void NotifySessionFullScreen(bool fullScreen) {}
    virtual WSError GetUIContentRemoteObj(sptr<IRemoteObject>& uiContentRemoteObj) = 0;

    // **Non** IPC interface
    virtual void NotifyBackpressedEvent(bool& isConsumed) {}
    virtual void NotifyPointerEvent(const std::shared_ptr<MMI::PointerEvent>& pointerEvent) {}
    virtual void NotifyKeyEvent(const std::shared_ptr<MMI::KeyEvent>& keyEvent, bool& isConsumed,
        bool notifyInputMethod = true) {}
    virtual void NotifyFocusActiveEvent(bool isFocusActive) {}
    virtual void NotifyFocusStateEvent(bool focusState) {}
    virtual bool NotifyOnKeyPreImeEvent(const std::shared_ptr<MMI::KeyEvent>& keyEvent) {return false;}
    virtual int32_t GetPersistentId() const
    {
        return -1;
    }
    virtual WSError NotifyDensityFollowHost(bool isFollowHost, float densityValue)
    {
        return WSError::WS_OK;
    }
    virtual WSError NotifyAccessibilityHoverEvent(float pointX, float pointY, int32_t sourceType, int32_t eventType,
        int64_t timeMs)
    {
        return WSError::WS_OK;
    }
    virtual WSError NotifyAccessibilityChildTreeRegister(
        uint32_t windowId, int32_t treeId, int64_t accessibilityId)
    {
        return WSError::WS_OK;
    }
    virtual WSError NotifyAccessibilityChildTreeUnregister()
    {
        return WSError::WS_OK;
    }
    virtual WSError NotifyAccessibilityDumpChildInfo(
        const std::vector<std::string>& params, std::vector<std::string>& info)
    {
        return WSError::WS_OK;
    }

    virtual void NotifyKeyboardPanelInfoChange(const KeyboardPanelInfo& keyboardPanelInfo) {}
    virtual WSError NotifyDumpInfo(const std::vector<std::string>& params, std::vector<std::string>& info)
    {
        return WSError::WS_OK;
    }

    virtual WSError SetEnableDragBySystem(bool dragEnable) = 0;
};
} // namespace OHOS::Rosen
#endif // OHOS_WINDOW_SCENE_SESSION_STAGE_INTERFACE_H
