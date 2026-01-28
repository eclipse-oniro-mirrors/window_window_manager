/*
 * Copyright (c) 2023-2023 Huawei Device Co., Ltd.
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

#ifndef OHOS_PICTURE_IN_PICTURE_INTERFACE_H
#define OHOS_PICTURE_IN_PICTURE_INTERFACE_H

#include "napi/native_api.h"
#include "wm_common.h"

namespace OHOS {
namespace Rosen {
/**
 * @class IPiPLifeCycle
 *
 * @brief Pip lifecycle listener.
 */
class IPiPLifeCycle : virtual public RefBase {
public:
    virtual void OnPreparePictureInPictureStart(PiPStateChangeReason reason) {}
    virtual void OnPictureInPictureStart(PiPStateChangeReason reason) {}
    virtual void OnPreparePictureInPictureStop(PiPStateChangeReason reason) {}
    virtual void OnPictureInPictureStop(PiPStateChangeReason reason) {}
    virtual void OnPictureInPictureOperationError(int32_t errorCode) {}
    virtual void OnRestoreUserInterface(PiPStateChangeReason reason) {}

    // native callback
    virtual void OnPreparePictureInPictureStart(uint32_t controllerId) {}
    virtual void OnPictureInPictureStart(uint32_t controllerId) {}
    virtual void OnPreparePictureInPictureStop(uint32_t controllerId) {}
    virtual void OnPictureInPictureStop(uint32_t controllerId) {}
    virtual void OnPictureInPictureOperationError(uint32_t controllerId, int32_t errorCode) {}
    virtual void OnRestoreUserInterface(uint32_t controllerId) {}
};

/**
 * @class IPiPActionObserver
 *
 * @brief Pip action observer.
 */
class IPiPActionObserver : virtual public RefBase {
public:
    virtual void OnActionEvent(const std::string& name, int32_t status) {}
};

/**
 * @class IPiPControlObserver
 *
 * @brief Pip control observer.
 */
class IPiPControlObserver : virtual public RefBase {
public:
    virtual void OnControlEvent(PiPControlType controlType, PiPControlStatus status) {}

    // native callback
    virtual void OnControlEvent(uint32_t controllerId, PiPControlType controlType, PiPControlStatus status) {}
};

/**
 * @class IPiPControlObserver
 *
 * @brief Pip control observer.
 */
class IPiPWindowSize : virtual public RefBase {
public:
    virtual void OnPipSizeChange(const PiPWindowSize& size) {}

    // native callback
    virtual void OnPipSizeChange(uint32_t controllerId, const PiPWindowSize& size) {}
};

/**
 * @class IPiPTypeNodeObserver
 *
 * @brief Pip typeNode observer.
 */
class IPiPTypeNodeObserver : virtual public RefBase {
public:
    virtual void OnPipTypeNodeChange(const napi_ref nodeRef) {}
};

class IPiPStartObserver : virtual public RefBase {
public:
    // native callback
    virtual void OnPipStart(uint32_t controllerId, uint8_t requestId, uint64_t surfaceId) {}
};

class IPiPActiveStatusObserver : virtual public RefBase {
public:
    virtual void OnActiveStatusChange(bool status) {}
};
} // namespace Rosen
} // namespace OHOS
#endif //OHOS_PICTURE_IN_PICTURE_INTERFACE_H
