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
    virtual void OnPreparePictureInPictureStart() {}
    virtual void OnPictureInPictureStart() {}
    virtual void OnPreparePictureInPictureStop() {}
    virtual void OnPictureInPictureStop() {}
    virtual void OnPictureInPictureOperationError(int32_t errorCode) {}
    virtual void OnRestoreUserInterface() {}
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
};

/**
 * @class IPiPControlObserver
 *
 * @brief Pip control observer.
 */
class IPiPWindowSize : virtual public RefBase {
public:
    virtual void OnPipSizeChange(const PiPWindowSize& size) {}
};
} // namespace Rosen
} // namespace OHOS
#endif //OHOS_PICTURE_IN_PICTURE_INTERFACE_H
