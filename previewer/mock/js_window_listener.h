/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#ifndef OHOS_JS_WINDOW_LISTENER_H
#define OHOS_JS_WINDOW_LISTENER_H

#include <map>
#include <mutex>

#include "class_var_definition.h"
#include "js_window_utils.h"
#include "native_engine/native_engine.h"
#include "native_engine/native_value.h"
#include "refbase.h"
#include "window.h"
#include "window_manager.h"
#include "wm_common.h"

namespace OHOS {
namespace Rosen {
const std::string SYSTEM_AVOID_AREA_CHANGE_CB = "systemAvoidAreaChange";
const std::string AVOID_AREA_CHANGE_CB = "avoidAreaChange";
const std::string WINDOW_FOCUS_STATE_CHANGE_CB = "windowFocusStateChange";

class JsWindowListener : public IAvoidAreaChangedListener {
public:
    JsWindowListener(napi_env env, std::shared_ptr<NativeReference> callback)
        : env_(env), jsCallBack_(callback), weakRef_(wptr<JsWindowListener>(this)) {}
    ~JsWindowListener();
    void OnAvoidAreaChanged(const AvoidArea avoidArea, AvoidAreaType type) override;
    void CallJsMethod(const char* methodName, napi_value const* argv = nullptr, size_t argc = 0);
    void SetMainEventHandler();

private:
    napi_env env_ = nullptr;
    std::shared_ptr<NativeReference> jsCallBack_;
    wptr<JsWindowListener> weakRef_ = nullptr;
    std::shared_ptr<AppExecFwk::EventHandler> eventHandler_ = nullptr;
    DEFINE_VAR_DEFAULT_FUNC_SET(bool, IsDeprecatedInterface, isDeprecatedInterface, false)
};
}  // namespace Rosen
}  // namespace OHOS
#endif /* OHOS_JS_WINDOW_LISTENER_H */
