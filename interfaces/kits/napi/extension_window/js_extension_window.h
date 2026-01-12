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

#ifndef OHOS_JS_EXTENSION_WINDOW_H
#define OHOS_JS_EXTENSION_WINDOW_H

#include "extension_window.h"
#include "extension_window_impl.h"
#include "js_extension_window_register_manager.h"
#include "native_engine/native_engine.h"
#include "native_engine/native_reference.h"
#include "native_engine/native_value.h"
#include "session_info.h"

namespace OHOS {
namespace Rosen {
napi_valuetype GetType(napi_env env, napi_value value);
class JsExtensionWindow {
public:
    explicit JsExtensionWindow(const std::shared_ptr<Rosen::ExtensionWindow> extensionWindow, int32_t hostWindowId);
    JsExtensionWindow(const std::shared_ptr<Rosen::ExtensionWindow> extensionWindow,
        sptr<AAFwk::SessionInfo> sessionInfo);
    ~JsExtensionWindow();
    static napi_value CreateJsExtensionWindow(napi_env env, sptr<Rosen::Window> window, int32_t hostWindowId);
    static napi_value CreateJsExtensionWindowObject(napi_env env, sptr<Rosen::Window> window,
    sptr<AAFwk::SessionInfo> sessionInfo);
    static void Finalizer(napi_env env, void* data, void* hint);
    static void RegisterUnsupportFuncs(napi_env env, napi_value objValue, const char *moduleName);
    static napi_value GetWindowAvoidArea(napi_env env, napi_callback_info info);
    static napi_value RegisterExtensionWindowCallback(napi_env env, napi_callback_info info);
    static napi_value UnRegisterExtensionWindowCallback(napi_env env, napi_callback_info info);
    static napi_value AtomicRegisterExtensionWindowCallback(napi_env env, napi_callback_info info);
    static napi_value AtomicUnRegisterExtensionWindowCallback(napi_env env, napi_callback_info info);
    static napi_value HideNonSecureWindows(napi_env env, napi_callback_info info);
    static napi_value LoadContent(napi_env env, napi_callback_info info);
    static napi_value LoadContentByName(napi_env env, napi_callback_info info);
    static napi_value IsWindowShowingSync(napi_env env, napi_callback_info info);
    static napi_value SetUIContent(napi_env env, napi_callback_info info);
    static napi_value SetWindowBackgroundColorSync(napi_env env, napi_callback_info info);
    static napi_value GetWindowPropertiesSync(napi_env env, napi_callback_info info);
    static napi_value MoveWindowTo(napi_env env, napi_callback_info info);
    static napi_value ResizeWindow(napi_env env, napi_callback_info info);
    static napi_value SetSpecificSystemBarEnabled(napi_env env, napi_callback_info info);
    static napi_value SetPreferredOrientation(napi_env env, napi_callback_info info);
    static napi_value GetPreferredOrientation(napi_env env, napi_callback_info info);
    static napi_value GetUIContext(napi_env env, napi_callback_info info);
    static napi_value SetWindowBrightness(napi_env env, napi_callback_info info);
    static napi_value SetWindowKeepScreenOn(napi_env env, napi_callback_info info);
    static napi_value CreateSubWindowWithOptions(napi_env env, napi_callback_info info);
    static napi_value SetWaterMarkFlag(napi_env env, napi_callback_info info);
    static napi_value HidePrivacyContentForHost(napi_env env, napi_callback_info info);
    static napi_value OccupyEvents(napi_env env, napi_callback_info info);
    static napi_value AtomicServiceCreateSubWindowWithOptions(napi_env env, napi_callback_info info);
    static napi_value SetWindowLayoutFullScreen(napi_env env, napi_callback_info info);
    static napi_value GetWindowColorSpace(napi_env env, napi_callback_info info);
    static napi_value SetWindowColorSpace(napi_env env, napi_callback_info info);
    static napi_value SetWindowPrivacyMode(napi_env env, napi_callback_info info);
    static napi_value SetWindowSystemBarEnable(napi_env env, napi_callback_info info);
    static napi_value GetGestureBackEnabled(napi_env env, napi_callback_info info);
    static napi_value SetGestureBackEnabled(napi_env env, napi_callback_info info);
    static napi_value GetImmersiveModeEnabledState(napi_env env, napi_callback_info info);
    static napi_value SetImmersiveModeEnabledState(napi_env env, napi_callback_info info);
    static napi_value IsFocused(napi_env env, napi_callback_info info);
    static napi_value IsWindowSupportWideGamut(napi_env env, napi_callback_info info);
    static napi_value GetGlobalScaledRect(napi_env env, napi_callback_info info);
    static napi_value GetTitleButtonRect(napi_env env, napi_callback_info info);
    static napi_value GetWindowStatus(napi_env env, napi_callback_info info);
    static napi_value EmptyAsyncCall(napi_env env, napi_callback_info info);
    static napi_value EmptySyncCall(napi_env env, napi_callback_info info);
    static napi_value UnsupportAsyncCall(napi_env env, napi_callback_info info);
    static napi_value UnsupportSyncCall(napi_env env, napi_callback_info info);
    static napi_value InvalidAsyncCall(napi_env env, napi_callback_info info);
    static napi_value InvalidSyncCall(napi_env env, napi_callback_info info);
    static napi_value GetWindowDensityInfo(napi_env env, napi_callback_info info);
    static napi_value GetWindowSystemBarProperties(napi_env env, napi_callback_info info);
    static napi_value GetStatusBarProperty(napi_env env, napi_callback_info info);
    static napi_value SetStatusBarColor(napi_env env, napi_callback_info info);

private:
    napi_value OnGetWindowAvoidArea(napi_env env, napi_callback_info info);
    napi_value OnRegisterRectChangeCallback(napi_env env, size_t argc, napi_value* argv,
        const sptr<Window>& windowImpl);
    napi_value OnRegisterExtensionWindowCallback(napi_env env, napi_callback_info info, bool atomicService = false);
    napi_value OnUnRegisterExtensionWindowCallback(napi_env env, napi_callback_info info, bool atomicService = false);
    napi_value OnHideNonSecureWindows(napi_env env, napi_callback_info info);
    napi_value OnLoadContent(napi_env env, napi_callback_info info, bool isLoadedByName);
    napi_value OnIsWindowShowingSync(napi_env env, napi_callback_info info);
    napi_value OnSetUIContent(napi_env env, napi_callback_info info);
    napi_value OnSetWindowBackgroundColorSync(napi_env env, napi_callback_info info);
    napi_value OnGetWindowPropertiesSync(napi_env env, napi_callback_info info);
    napi_value OnMoveWindowTo(napi_env env, napi_callback_info info);
    napi_value OnResizeWindow(napi_env env, napi_callback_info info);
    napi_value OnSetSpecificSystemBarEnabled(napi_env env, napi_callback_info info);
    napi_value OnSetPreferredOrientation(napi_env env, napi_callback_info info);
    napi_value OnGetUIContext(napi_env env, napi_callback_info info);
    napi_value OnSetWindowBrightness(napi_env env, napi_callback_info info);
    napi_value OnSetWindowKeepScreenOn(napi_env env, napi_callback_info info);
    napi_value OnSetWaterMarkFlag(napi_env env, napi_callback_info info);
    napi_value OnHidePrivacyContentForHost(napi_env env, napi_callback_info info);

    napi_value OnCreateSubWindowWithOptions(napi_env env, napi_callback_info info);
    napi_value OnOccupyEvents(napi_env env, napi_callback_info info);
    napi_value OnSetWindowLayoutFullScreen(napi_env env, napi_callback_info info);
    napi_value OnGetWindowColorSpace(napi_env env, napi_callback_info info);
    napi_value OnSetWindowColorSpace(napi_env env, napi_callback_info info);
    napi_value OnSetWindowPrivacyMode(napi_env env, napi_callback_info info);
    napi_value OnSetWindowSystemBarEnable(napi_env env, napi_callback_info info);
    napi_value OnGetGestureBackEnabled(napi_env env, napi_callback_info info);
    napi_value OnSetGestureBackEnabled(napi_env env, napi_callback_info info);
    napi_value OnGetImmersiveModeEnabledState(napi_env env, napi_callback_info info);
    napi_value OnSetImmersiveModeEnabledState(napi_env env, napi_callback_info info);
    napi_value OnIsFocused(napi_env env, napi_callback_info info);
    napi_value OnIsWindowSupportWideGamut(napi_env env, napi_callback_info info);
    napi_value OnGetGlobalScaledRect(napi_env env, napi_callback_info info);
    napi_value OnUnsupportAsyncCall(napi_env env, napi_callback_info info);
    napi_value OnEmptyAsyncCall(napi_env env, napi_callback_info info);
    napi_value OnInvalidAsyncCall(napi_env env, napi_callback_info info);
    napi_value OnGetStatusBarPropertySync(napi_env env, napi_callback_info info);
    napi_value OnSetStatusBarColor(napi_env env, napi_callback_info info);

    static napi_value GetProperties(napi_env env, napi_callback_info info);

    std::shared_ptr<Rosen::ExtensionWindow> extensionWindow_;
    int32_t hostWindowId_ = 0;
    sptr<AAFwk::SessionInfo> sessionInfo_ = nullptr;
    std::unique_ptr<JsExtensionWindowRegisterManager> extensionRegisterManager_ = nullptr;
    static inline const std::string RECT_CHANGE = "rectChange";
};
}  // namespace Rosen
}  // namespace OHOS
#endif  // OHOS_JS_EXTENSION_WINDOW_H
