/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#ifndef OHOS_WINDOW_SCENE_JS_SCREEN_SESSION_H
#define OHOS_WINDOW_SCENE_JS_SCREEN_SESSION_H

#include <map>

#include <native_engine/native_engine.h>
#include <native_engine/native_value.h>
#include "session/screen/include/screen_session.h"
#include "screen_scene.h"

namespace OHOS::Rosen {
class JsScreenSession : public IScreenChangeListener {
public:
    JsScreenSession(napi_env env, const sptr<ScreenSession>& screenSession);
    virtual ~JsScreenSession();

    static napi_value Create(napi_env env, const sptr<ScreenSession>& screenSession);
    static void Finalizer(napi_env env, void* data, void* hint);
    static napi_value ConvertMapToJsMap(napi_env env, const std::map<int32_t, int32_t>& map);

private:
    static napi_value LoadContent(napi_env env, napi_callback_info info);
    napi_value OnLoadContent(napi_env env, napi_callback_info info);
    static napi_value DestroyContent(napi_env env, napi_callback_info info);
    napi_value OnDestroyContent(napi_env env, napi_callback_info info);
    static napi_value ReleaseResource(napi_env env, napi_callback_info info);
    napi_value OnReleaseResource(napi_env env, napi_callback_info info);
    napi_value ScheduleLoadContentTask(napi_env env, const std::string& contentUrl,
        std::weak_ptr<AbilityRuntime::Context> contextWeakPtr, std::shared_ptr<NativeReference> contentStorage);
    static napi_value RegisterCallback(napi_env env, napi_callback_info info);
    napi_value OnRegisterCallback(napi_env env, napi_callback_info info);
    static napi_value SetScreenRotationLocked(napi_env env, napi_callback_info info);
    napi_value OnSetScreenRotationLocked(napi_env env, napi_callback_info info);
    static napi_value SetTouchEnabled(napi_env env, napi_callback_info info);
    napi_value OnSetTouchEnabled(napi_env env, napi_callback_info info);
    static napi_value GetScreenUIContext(napi_env env, napi_callback_info info);
    napi_value OnGetScreenUIContext(napi_env env, napi_callback_info info);
    void CallJsCallback(const std::string& callbackType);
    std::shared_ptr<NativeReference> GetJSCallback(const std::string& callbackType);
    bool IsCallbackRegistered(const std::string& callbackType);
    void RegisterScreenChangeListener();
    void UnRegisterScreenChangeListener();

    void OnConnect(ScreenId screenId) override;
    void OnDisconnect(ScreenId screenId) override;
    void OnPropertyChange(const ScreenProperty& newProperty, ScreenPropertyChangeReason reason,
        ScreenId screenId) override;
    void OnPowerStatusChange(DisplayPowerEvent event, EventStatus eventStatus,
        PowerStateChangeReason reason) override;
    void OnSensorRotationChange(float sensorRotation, ScreenId screenId, bool isSwitchUser) override;
    void OnScreenOrientationChange(float screenOrientation, ScreenId screenId) override;
    void OnScreenRotationLockedChange(bool isLocked, ScreenId screenId) override;
    void OnScreenDensityChange();
    void OnScreenExtendChange(ScreenId mainScreenId, ScreenId extendScreenId) override;
    void OnHoverStatusChange(int32_t hoverStatus, bool needRotate, ScreenId screenId) override;
    void OnScreenCaptureNotify(ScreenId mainScreenId, int32_t uid, const std::string& clientName) override;
    void OnCameraBackSelfieChange(bool isCameraBackSelfie, ScreenId screenId) override;
    void OnSuperFoldStatusChange(ScreenId screenId, SuperFoldStatus superFoldStatus) override;
    void OnSecondaryReflexionChange(ScreenId screenId, bool isSecondaryReflexion) override;
    void OnExtendScreenConnectStatusChange(ScreenId screenId,
        ExtendScreenConnectStatus extendScreenConnectStatus) override;
    void OnBeforeScreenPropertyChange(FoldStatus foldStatus) override;
    void OnScreenModeChange(ScreenModeChangeEvent screenModeChangeEvent) override;

    napi_env env_;
    sptr<ScreenSession> screenSession_;
    sptr<ScreenScene> screenScene_ = nullptr;
    std::map<std::string, std::shared_ptr<NativeReference>> mCallback_;
    std::shared_mutex jsCbMapMutex_;
    std::mutex callbackMutex_;
};
} // namespace OHOS::Rosen

#endif // OHOS_WINDOW_SCENE_JS_SCREEN_SESSION_H
