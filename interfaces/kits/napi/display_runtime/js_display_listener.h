/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#ifndef OHOS_JS_DISPLAY_LISTENER_H
#define OHOS_JS_DISPLAY_LISTENER_H

#include <mutex>
#include "dm_common.h"
#include "native_engine/native_engine.h"
#include "native_engine/native_value.h"
#include "refbase.h"
#include "display_manager.h"

namespace OHOS {
namespace Rosen {
class JsDisplayListener : public DisplayManager::IDisplayListener,
                          public DisplayManager::IPrivateWindowListener,
                          public DisplayManager::IFoldStatusListener,
                          public DisplayManager::IFoldAngleListener,
                          public DisplayManager::ICaptureStatusListener,
                          public DisplayManager::IDisplayModeListener,
                          public DisplayManager::IAvailableAreaListener,
                          public DisplayManager::IBrightnessInfoListener,
                          public DisplayManager::IDisplayAttributeListener {
public:
    explicit JsDisplayListener(napi_env env);
    ~JsDisplayListener() override;
    void AddCallback(const std::string& type, napi_value jsListenerObject);
    void RemoveAllCallback();
    void RemoveCallback(napi_env env, const std::string& type, napi_value jsListenerObject);
    void OnCreate(DisplayId id) override;
    void OnDestroy(DisplayId id) override;
    void OnChange(DisplayId id) override;
    void OnPrivateWindow(bool hasPrivate) override;
    void OnFoldStatusChanged(FoldStatus foldStatus) override;
    void OnFoldAngleChanged(std::vector<float> foldAngles) override;
    void OnCaptureStatusChanged(bool isCapture) override;
    void OnDisplayModeChanged(FoldDisplayMode displayMode) override;
    void OnAvailableAreaChanged(DMRect area) override;
    void OnBrightnessInfoChanged(DisplayId id, const ScreenBrightnessInfo& info) override;
    void OnAttributeChange(DisplayId displayId, const std::vector<std::string>& attribuutes) override;

private:
    void CallJsMethod(const std::string& methodName, napi_value const * argv = nullptr, size_t argc = 0);
    void ProcessAttributeCallbacks(napi_env env, const std::vector<std::string>& attributes, DisplayId displayId,
        sptr<JsDisplayListener> thisListener);
    void RemoveDuplicateMethods(std::vector<napi_value>& methods);
    napi_env env_ = nullptr;
    std::mutex mtx_;
    wptr<JsDisplayListener> weakRef_  = nullptr;
    std::map<std::string, std::vector<std::unique_ptr<NativeReference>>> jsCallBack_;
    napi_value CreateDisplayIdArray(napi_env env, const std::vector<DisplayId>& data);
    static void CleanEnv(void* obj);
};
const std::string EVENT_ADD = "add";
const std::string EVENT_REMOVE = "remove";
const std::string EVENT_CHANGE = "change";
const std::string EVENT_PRIVATE_MODE_CHANGE = "privateModeChange";
const std::string EVENT_FOLD_STATUS_CHANGED = "foldStatusChange";
const std::string EVENT_FOLD_ANGLE_CHANGED = "foldAngleChange";
const std::string EVENT_CAPTURE_STATUS_CHANGED = "captureStatusChange";
const std::string EVENT_DISPLAY_MODE_CHANGED = "foldDisplayModeChange";
const std::string EVENT_AVAILABLE_AREA_CHANGED = "availableAreaChange";
const std::string EVENT_BRIGHTNESS_INFO_CHANGED = "brightnessInfoChange";
}  // namespace Rosen
}  // namespace OHOS
#endif /* OHOS_JS_DISPLAY_LISTENER_H */