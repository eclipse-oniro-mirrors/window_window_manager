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
#ifndef OHOS_JS_FLOAT_VIEW_MANAGER_H
#define OHOS_JS_FLOAT_VIEW_MANAGER_H

#include "js_runtime_utils.h"
#include "float_view_option.h"
#include "floating_ball_option.h"
#include "js_fb_window_controller.h"
#include "js_float_view_controller.h"
#include "window_histogram_management.h"

namespace OHOS {
namespace Rosen {
napi_value JsFloatViewWMInit(napi_env env, napi_value exportObj);
class JsFloatViewManager {
public:
    JsFloatViewManager();
    virtual ~JsFloatViewManager();
    static void Finalizer(napi_env env, void* data, void* hint);
    static napi_value CreateFloatViewController(napi_env env, napi_callback_info info);
    static napi_value IsFloatViewEnabled(napi_env env, napi_callback_info info);
    static napi_value GetFloatViewLimits(napi_env env, napi_callback_info info);
    static napi_value Bind(napi_env env, napi_callback_info info);
    static napi_value UnBind(napi_env env, napi_callback_info info);
private:
    napi_value OnCreateFloatViewController(napi_env env, napi_callback_info info);
    std::string CheckAndGetParam(napi_env env, napi_callback_info info, FvOption& option);
    napi_value CreateFloatViewControllerTask(napi_env env, const FvOption &option);
    napi_value OnIsFloatViewEnabled(napi_env env, napi_callback_info info);
    napi_value OnGetFloatViewLimits(napi_env env, napi_callback_info info);
    napi_value OnBind(napi_env env, napi_callback_info info);
    napi_value OnUnBind(napi_env env, napi_callback_info info);

    std::pair<void*, void*> GetBindControllers(napi_env env, napi_value argv[]);
    napi_value BindTask(napi_env env, const sptr<FloatViewController> &fwController,
        const sptr<FloatingBallController> &fbController, const FbOption &option);
    napi_value UnBindTask(napi_env env, const sptr<FloatViewController> &fwController,
        const sptr<FloatingBallController> &fbController);
};
} // namespace Rosen
} // namespace OHOS
#endif