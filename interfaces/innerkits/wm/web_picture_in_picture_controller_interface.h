/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#ifndef OHOS_WEB_PICTURE_IN_PICTURE_CONTROLLER_INTERFACE_H
#define OHOS_WEB_PICTURE_IN_PICTURE_CONTROLLER_INTERFACE_H

#include <wm_common.h>
#include <window.h>
#include "native_pip_window_listener.h"
#include "web_picture_in_picture_controller.h"

namespace OHOS {
namespace Rosen {

class WebPictureInPictureControllerInterface : virtual public RefBase {
public:
    WebPictureInPictureControllerInterface() = default;
    ~WebPictureInPictureControllerInterface() = default;
    WMError Create(const PiPConfig& pipConfig);
    WMError StartPip(uint32_t controllerId);
    WMError StopPip();
    WMError UpdateContentSize(int32_t width, int32_t height);
    WMError UpdatePiPControlStatus(PiPControlType controlType, PiPControlStatus status);
    WMError setPiPControlEnabled(PiPControlType controlType, bool enabled);
    WMError SetPipParentWindowId(uint32_t windowId);
    WMError SetPipInitialSurfaceRect(int32_t positionX, int32_t positionY, uint32_t width, uint32_t height);
    WMError UnsetPipInitialSurfaceRect();
    WMError RegisterStartPipListener(NativePipStartPipCallback);
    WMError RegisterLifeCycleListener(NativePipLifeCycleCallback);
    WMError RegisterControlEventListener(NativePipControlEventCallback);
    WMError RegisterResizeListener(NativePipResizeCallback);
    WMError UnregisterStartPipListener(NativePipStartPipCallback);
    WMError UnregisterLifeCycleListener(NativePipLifeCycleCallback);
    WMError UnregisterControlEventListener(NativePipControlEventCallback);
    WMError UnregisterResizeListener(NativePipResizeCallback);
    WMError UnregisterAllPiPStart();
    WMError UnregisterAllPiPLifecycle();
    WMError UnregisterAllPiPControlObserver();
    WMError UnregisterAllPiPWindowSize();
    bool IsStarted() const { return isStarted_; }

private:
    enum class ListenerType : uint32_t {
        STATE_CHANGE_CB,
        CONTROL_EVENT_CB,
        SIZE_CHANGE_CB,
        PIP_START_CB,
    };
    WMError CheckRegisterParam(ListenerType type, const sptr<NativePiPWindowListener>& listener);
    WMError RegisterListenerWithType(ListenerType type, const sptr<NativePiPWindowListener>& listener);
    WMError UnregisterListenerWithType(ListenerType type, const sptr<NativePiPWindowListener>& listener);
    WMError ProcessStateChangeRegister(const sptr<NativePiPWindowListener>& listener);
    WMError ProcessControlEventRegister(const sptr<NativePiPWindowListener>& listener);
    WMError ProcessSizeChangeRegister(const sptr<NativePiPWindowListener>& listener);
    WMError ProcessPipStartRegister(const sptr<NativePiPWindowListener>& listener);
    WMError ProcessStateChangeUnregister(const sptr<NativePiPWindowListener>& listener);
    WMError ProcessControlEventUnregister(const sptr<NativePiPWindowListener>& listener);
    WMError ProcessSizeChangeUnregister(const sptr<NativePiPWindowListener>& listener);
    WMError ProcessPipStartUnregister(const sptr<NativePiPWindowListener>& listener);
    bool IsRegistered(ListenerType type, const sptr<NativePiPWindowListener>& listener);
    WMError UnregisterAll(ListenerType type);
    bool isPipEnabled_ = false;
    bool isStarted_ = false;
    sptr<WebPictureInPictureController> sptrWebPipController_;
    std::set<sptr<NativePiPWindowListener>> lifeCycleCallbackSet_;
    std::set<sptr<NativePiPWindowListener>> controlEventCallbackSet_;
    std::set<sptr<NativePiPWindowListener>> resizeCallbackSet_;
    std::set<sptr<NativePiPWindowListener>> startPipCallbackSet_;
};
} // namespace Rosen
} // namespace OHOS
#endif // OHOS_WEB_PICTURE_IN_PICTURE_CONTROLLER_INTERFACE_H