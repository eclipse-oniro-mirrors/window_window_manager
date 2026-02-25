/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

#ifndef OHOS_WINDOW_MANAGER_AGENT_INTERFACE_H
#define OHOS_WINDOW_MANAGER_AGENT_INTERFACE_H

#include <iremote_broker.h>
#include "wm_common.h"
#include "window_manager.h"

namespace OHOS {
namespace Rosen {
class IWindowManagerAgent : public IRemoteBroker {
public:
    DECLARE_INTERFACE_DESCRIPTOR(u"OHOS.IWindowManagerAgent");

    enum class WindowManagerAgentMsg : uint32_t {
        TRANS_ID_UPDATE_FOCUS = 1,
        TRANS_ID_UPDATE_SYSTEM_BAR_PROPS,
        TRANS_ID_UPDATE_WINDOW_STATUS,
        TRANS_ID_UPDATE_WINDOW_VISIBILITY,
        TRANS_ID_UPDATE_WINDOW_DRAWING_STATE,
        TRANS_ID_UPDATE_CAMERA_FLOAT,
        TRANS_ID_UPDATE_WATER_MARK_FLAG,
        TRANS_ID_UPDATE_VISIBLE_WINDOW_NUM = 8,
        TRANS_ID_UPDATE_GESTURE_NAVIGATION_ENABLED,
        TRANS_ID_UPDATE_CAMERA_WINDOW_STATUS,
        TRANS_ID_UPDATE_WINDOW_MODE_TYPE,
        TRANS_ID_UPDATE_WINDOW_STYLE_TYPE,
        TRANS_ID_NOTIFY_WINDOW_PID_VISIBILITY,
        TRANS_ID_UPDATE_PIP_WINDOW_STATE_CHANGED,
        TRANS_ID_NOTIFY_CALLING_DISPLAY_CHANGE,
        TRANS_ID_NOTIFY_WINDOW_PROPERTY_CHANGE,
        TRANS_ID_NOTIFY_WINDOW_SYSTEM_BAR_PROPERTY_CHANGE,
        TRANS_ID_NOTIFY_WINDOW_SUPPORT_ROTATION_CHANGE,
        TRANS_ID_NOTIFY_DISPLAY_GROUP_INFO_CHANGE,
    };

    virtual void UpdateFocusChangeInfo(const sptr<FocusChangeInfo>& focusChangeInfo, bool focused) = 0;
    virtual void UpdateDisplayGroupInfo(DisplayGroupId displayGroupId, DisplayId displayId, bool isAdd) = 0;
    virtual void UpdateWindowModeTypeInfo(WindowModeType type) = 0;
    virtual void UpdateSystemBarRegionTints(DisplayId displayId, const SystemBarRegionTints& tints) = 0;
    virtual void NotifyAccessibilityWindowInfo(const std::vector<sptr<AccessibilityWindowInfo>>& infos,
        WindowUpdateType type) = 0;
    virtual void UpdateWindowVisibilityInfo(const std::vector<sptr<WindowVisibilityInfo>>& visibilityInfos) = 0;
    virtual void UpdateWindowDrawingContentInfo(
        const std::vector<sptr<WindowDrawingContentInfo>>& windowDrawingContentInfos) = 0;
    virtual void UpdateCameraFloatWindowStatus(uint32_t accessTokenId, bool isShowing) = 0;
    virtual void NotifyWaterMarkFlagChangedResult(bool isShowing) = 0;
    virtual void UpdateVisibleWindowNum(const std::vector<VisibleWindowNumInfo>& visibleWindowNumInfo) = 0;
    virtual void NotifyGestureNavigationEnabledResult(bool enable) = 0;
    virtual void UpdateCameraWindowStatus(uint32_t accessTokenId, bool isShowing) = 0;
    virtual void NotifyWindowStyleChange(WindowStyleType type) = 0;
    virtual void NotifyCallingWindowDisplayChanged(const CallingWindowInfo& callingWindowInfo) = 0;
    virtual void NotifyWindowPidVisibilityChanged(const sptr<WindowPidVisibilityInfo>& info) = 0;
    virtual void NotifyWindowSystemBarPropertyChange(WindowType type, const SystemBarProperty& systemBarProperty) = 0;
    virtual void UpdatePiPWindowStateChanged(const std::string& bundleName, bool isForeground) = 0;
    virtual void NotifyWindowPropertyChange(uint32_t propertyDirtyFlags, const WindowInfoList& windowInfoList) = 0;
    virtual void NotifySupportRotationChange(const SupportRotationInfo& supportRotationInfo) = 0;
};
} // namespace Rosen
} // namespace OHOS
#endif // OHOS_WINDOW_MANAGER_AGENT_INTERFACE_H
