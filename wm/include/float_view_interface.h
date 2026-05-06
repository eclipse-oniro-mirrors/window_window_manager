
/*
 * Copyright (c) 2026-2026 Huawei Device Co., Ltd.
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
 
#ifndef OHOS_FLOAT_VIEW_INTERFACE_H
#define OHOS_FLOAT_VIEW_INTERFACE_H

#include "wm_common.h"
#include "window.h"

namespace OHOS {
namespace Rosen {
/**
 * @class IFvStateChangeObserver
 *
 * @brief fv click observer.
 */
class IFvStateChangeObserver : virtual public RefBase {
public:
    virtual void OnStateChange(const FloatViewState& state, const std::string& stopReason) = 0;
};

/**
 * @class IFvRectChangeObserver
 *
 * @brief fv rectangle change observer.
 */
class IFvRectChangeObserver : virtual public RefBase {
public:
    virtual void OnRectangleChange(const Rect& window, double scale, const std::string& reason) = 0;
};

/**
 * @class IFvLimitsChangeObserver
 *
 * @brief fv specification info change observer.
 */
class IFvLimitsChangeObserver : virtual public RefBase {
public:
    virtual void OnLimitsChange(const FloatViewLimits& limits) = 0;
};

} // namespace Rosen
} // namespace OHOS
#endif //OHOS_FLOAT_VIEW_INTERFACE_H