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

#ifndef OHOS_JS_FLOAT_VIEW_LISTENER_H
#define OHOS_JS_FLOAT_VIEW_LISTENER_H

#include "native_engine/native_value.h"
#include "float_view_interface.h"
#include "refbase.h"
#include "wm_common.h"

namespace OHOS {
namespace Rosen {

class JsFloatViewListener : public IFvStateChangeObserver,
                            public IFvRectChangeObserver,
                            public IFvLimitsChangeObserver {
public:
    JsFloatViewListener(napi_env env, const std::shared_ptr<NativeReference>& callback)
        : env_(env), jsCallBack_(callback) {}
    ~JsFloatViewListener() override;
    std::shared_ptr<NativeReference> GetCallbackRef() const;

    void OnStateChange(const FloatViewState& state, const std::string& stopReason) override;
    void OnRectangleChange(const Rect& window, double scale, const std::string& reason) override;
    void OnLimitsChange(const FloatViewLimits& limits) override;

private:
    napi_env env_ = nullptr;
    std::shared_ptr<NativeReference> jsCallBack_ = nullptr;
};
}  // namespace Rosen
}  // namespace OHOS
#endif // OHOS_JS_FLOAT_VIEW_LISTENER_H
