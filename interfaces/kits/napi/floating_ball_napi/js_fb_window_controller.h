/*
 * Copyright (c) 2025-2025 Huawei Device Co., Ltd.
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

#ifndef OHOS_JS_FB_CONTROLLER_H
#define OHOS_JS_FB_CONTROLLER_H

#include <map>
#include <refbase.h>
#include "floating_ball_controller.h"
#include "wm_common.h"
#include "js_fb_window_listener.h"
#include "window_histogram_management.h"


namespace OHOS {
namespace Rosen {
napi_value CreateJsFbControllerObject(napi_env env, const sptr<FloatingBallController>& floatingController);
class JsFbController {
public:
    explicit JsFbController(const sptr<FloatingBallController>& floatingController);
    virtual ~JsFbController();
    static void Finalizer(napi_env env, void* data, void* hint);
    static napi_value StartFloatingBall(napi_env env, napi_callback_info info);
    static napi_value StopFloatingBall(napi_env env, napi_callback_info info);
    static napi_value RestoreMainWindow(napi_env env, napi_callback_info info);
    static napi_value UpdateFloatingBall(napi_env env, napi_callback_info info);
    static napi_value RegisterCallback(napi_env env, napi_callback_info info);
    static napi_value UnregisterCallback(napi_env env, napi_callback_info info);
    static napi_value GetFloatingBallWindowInfo(napi_env env, napi_callback_info info);
    static napi_value SetInApplicationVisible(napi_env env, napi_callback_info info);
    napi_value GetFloatingBallOptionFromJs(napi_env env, napi_value optionObject, FbOption& option);
    sptr<FloatingBallController> GetController() const;
private:
    napi_value OnStartFloatingBall(napi_env env, napi_callback_info info);
    napi_value StartFloatingBallTask(napi_env env, const FbOption& option);
    napi_value OnUpdateFloatingBall(napi_env env, napi_callback_info info);
    napi_value UpdateFloatingBallTask(napi_env env, const FbOption &option);
    napi_value OnSetInApplicationVisible(napi_env env, napi_callback_info info);
    napi_value OnStopFloatingBall(napi_env env, napi_callback_info info);
    napi_value OnRestoreMainWindow(napi_env env, napi_callback_info info);

    napi_value OnRegisterCallback(napi_env env, napi_callback_info info);
    napi_value OnUnregisterCallback(napi_env env, napi_callback_info info);

    napi_value OnGetFloatingBallWindowInfo(napi_env env, napi_callback_info info);

    bool IsCallbackRegistered(napi_env env, const std::string& type, napi_value jsListenerObject);
    napi_value RegisterListenerWithType(napi_env env, const std::string& type, napi_value value);
    napi_value UnRegisterListenerWithType(napi_env env, const std::string& type, napi_value value);
    WmErrorCode UnRegisterListener(const std::string& type, const sptr<JsFbWindowListener>& fbWindowListener);

    WMError ProcessStateChangeRegister(const sptr<JsFbWindowListener>& listener);
    WMError ProcessClickEventRegister(const sptr<JsFbWindowListener>& listener);
    WMError ProcessStateChangeUnRegister(const sptr<JsFbWindowListener>& listener);
    WMError ProcessClickEventUnRegister(const sptr<JsFbWindowListener>& listener);

    template <typename T>
 	void HandleProperty(napi_env env, napi_value optionObject, const char* propertyName,
 	    void (FbOption::*setter)(const T&), FbOption& option);
    napi_value GetIcon(napi_env env, napi_value value, FbOption& option);

    sptr<FloatingBallController> fbController_ = nullptr;
    std::unordered_map<std::string, std::set<sptr<JsFbWindowListener>>> jsCbMap_;
    std::mutex callbBackMutex_;
};
} // namespace Rosen
} // namespace OHOS

#endif // OHOS_JS_FB_CONTROLLER_H
