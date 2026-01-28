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

#ifndef OHOS_JS_PIP_CONTROLLER_H
#define OHOS_JS_PIP_CONTROLLER_H

#include <map>
#include <refbase.h>
#include "js_runtime_utils.h"
#include "picture_in_picture_controller.h"
#include "wm_common.h"
#include "js_pip_window_listener.h"

namespace OHOS {
namespace Rosen {
napi_value CreateJsPipControllerObject(napi_env env, sptr<PictureInPictureController>& pipController);
class JsPipController {
public:
    explicit JsPipController(const sptr<PictureInPictureController>& pipController);
    ~JsPipController();
    static void Finalizer(napi_env env, void* data, void* hint);
    static napi_value StartPictureInPicture(napi_env env, napi_callback_info info);
    static napi_value StopPictureInPicture(napi_env env, napi_callback_info info);
    static napi_value SetAutoStartEnabled(napi_env env, napi_callback_info info);
    static napi_value UpdateContentNode(napi_env env, napi_callback_info info);
    static napi_value UpdateContentSize(napi_env env, napi_callback_info info);
    static napi_value UpdatePiPControlStatus(napi_env env, napi_callback_info info);
    static napi_value SetPiPControlEnabled(napi_env env, napi_callback_info info);
    static napi_value GetPiPWindowInfo(napi_env env, napi_callback_info info);
    static napi_value GetPiPSettingSwitch(napi_env env, napi_callback_info info);
    static napi_value IsPiPActive(napi_env env, napi_callback_info info);
    static napi_value RegisterCallback(napi_env env, napi_callback_info info);
    static napi_value UnregisterCallback(napi_env env, napi_callback_info info);
    static napi_value PictureInPicturePossible(napi_env env, napi_callback_info info);
private:
    enum class ListenerType : uint32_t {
        STATE_CHANGE_CB,
        CONTROL_PANEL_ACTION_EVENT_CB,
        CONTROL_EVENT_CB,
        SIZE_CHANGE_CB,
        ACTIVE_STATUS_CHANGE_CB,
    };

    napi_value OnStartPictureInPicture(napi_env env, napi_callback_info info);
    napi_value OnStopPictureInPicture(napi_env env, napi_callback_info info);
    napi_value OnSetAutoStartEnabled(napi_env env, napi_callback_info info);
    napi_value OnUpdateContentNode(napi_env env, napi_callback_info info);
    napi_value OnUpdateContentSize(napi_env env, napi_callback_info info);
    napi_value OnUpdatePiPControlStatus(napi_env env, napi_callback_info info);
    napi_value OnSetPiPControlEnabled(napi_env env, napi_callback_info info);
    napi_value OnGetPiPWindowInfo(napi_env env, napi_callback_info info);
    napi_value OnGetPiPSettingSwitch(napi_env env, napi_callback_info info);
    napi_value OnIsPiPActive(napi_env env, napi_callback_info info);
    napi_value OnRegisterCallback(napi_env env, napi_callback_info info);
    napi_value OnUnregisterCallback(napi_env env, napi_callback_info info);
    napi_value OnPictureInPicturePossible(napi_env env, napi_callback_info info);

    bool IfCallbackRegistered(napi_env env, const std::string& type, napi_value jsListenerObject);
    WmErrorCode RegisterListenerWithType(napi_env env, const std::string& type, napi_value value);
    WmErrorCode UnRegisterListenerWithType(napi_env env, const std::string& type, napi_value value);
    WmErrorCode UnRegisterListener(const std::string& type, const sptr<JsPiPWindowListener>& pipWindowListener);

    void ProcessStateChangeRegister(const sptr<JsPiPWindowListener>& listener);
    void ProcessActionEventRegister(const sptr<JsPiPWindowListener>& listener);
    void ProcessControlEventRegister(const sptr<JsPiPWindowListener>& listener);
    void ProcessSizeChangeRegister(const sptr<JsPiPWindowListener>& listener);
    void ProcessActiveStatusChangeRegister(const sptr<JsPiPWindowListener>& listener);
    void ProcessStateChangeUnRegister(const sptr<JsPiPWindowListener>& listener);
    void ProcessActionEventUnRegister(const sptr<JsPiPWindowListener>& listener);
    void ProcessControlEventUnRegister(const sptr<JsPiPWindowListener>& listener);
    void ProcessSizeChangeUnRegister(const sptr<JsPiPWindowListener>& listener);
    void ProcessActiveStatusChangeUnRegister(const sptr<JsPiPWindowListener>& listener);

    sptr<PictureInPictureController> pipController_ = nullptr;
    std::map<std::string, ListenerType> listenerCodeMap_;
    std::unordered_map<std::string, std::set<sptr<JsPiPWindowListener>>> jsCbMap_;
};
} // namespace Rosen
} // namespace OHOS

#endif // OHOS_JS_PIP_CONTROLLER_H
