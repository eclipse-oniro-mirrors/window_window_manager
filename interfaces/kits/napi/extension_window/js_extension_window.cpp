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

#include "js_extension_window.h"

#include "js_extension_window_utils.h"
#include "js_runtime_utils.h"
#include "js_window_utils.h"
#include "js_window.h"
#include "window_manager_hilog.h"
#include "wm_common.h"
#include "extension_window.h"
#include "ui_content.h"
#include "permission.h"

namespace OHOS {
namespace Rosen {
using namespace AbilityRuntime;
namespace {
constexpr Rect g_emptyRect = {0, 0, 0, 0};
constexpr size_t INDEX_ZERO = 0;
constexpr size_t INDEX_ONE = 1;
constexpr size_t INDEX_TWO = 2;
constexpr size_t ARG_COUNT_ONE = 1;
constexpr size_t ARG_COUNT_TWO = 2;
constexpr size_t ARG_COUNT_THREE = 3;
constexpr size_t FOUR_PARAMS_SIZE = 4;
} // namespace

JsExtensionWindow::JsExtensionWindow(
    const std::shared_ptr<Rosen::ExtensionWindow> extensionWindow,
    int32_t hostWindowId)
    : extensionWindow_(extensionWindow), hostWindowId_(hostWindowId),
    extensionRegisterManager_(std::make_unique<JsExtensionWindowRegisterManager>()) {
}

JsExtensionWindow::JsExtensionWindow(const std::shared_ptr<Rosen::ExtensionWindow> extensionWindow,
    sptr<AAFwk::SessionInfo> sessionInfo)
    : extensionWindow_(extensionWindow), hostWindowId_(-1), sessionInfo_(sessionInfo),
    extensionRegisterManager_(std::make_unique<JsExtensionWindowRegisterManager>()) {
}

JsExtensionWindow::~JsExtensionWindow() {}

napi_value JsExtensionWindow::CreateJsExtensionWindow(napi_env env, sptr<Rosen::Window> window, int32_t hostWindowId)
{
    TLOGD(WmsLogTag::WMS_UIEXT, "Called.");
    napi_value objValue = nullptr;
    napi_create_object(env, &objValue);

    if (env == nullptr || window == nullptr) {
        TLOGE(WmsLogTag::WMS_UIEXT, "JsExtensionWindow env or window is nullptr");
        return nullptr;
    }

    std::shared_ptr<ExtensionWindow> extensionWindow = std::make_shared<ExtensionWindowImpl>(window);
    std::unique_ptr<JsExtensionWindow> jsExtensionWindow =
        std::make_unique<JsExtensionWindow>(extensionWindow, hostWindowId);
    napi_wrap(env, objValue, jsExtensionWindow.release(), JsExtensionWindow::Finalizer, nullptr, nullptr);

    napi_property_descriptor desc[] = {
        DECLARE_NAPI_GETTER("properties", JsExtensionWindow::GetProperties)
    };
    NAPI_CALL(env, napi_define_properties(env, objValue, sizeof(desc) / sizeof(desc[0]), desc));

    const char *moduleName = "JsExtensionWindow";
    BindNativeFunction(env, objValue, "getWindowAvoidArea", moduleName, JsExtensionWindow::GetWindowAvoidArea);
    BindNativeFunction(env, objValue, "on", moduleName, JsExtensionWindow::RegisterExtensionWindowCallback);
    BindNativeFunction(env, objValue, "off", moduleName, JsExtensionWindow::UnRegisterExtensionWindowCallback);
    BindNativeFunction(env, objValue, "hideNonSecureWindows", moduleName, JsExtensionWindow::HideNonSecureWindows);
    BindNativeFunction(env, objValue, "createSubWindowWithOptions", moduleName,
        JsExtensionWindow::CreateSubWindowWithOptions);
    BindNativeFunction(env, objValue, "setWaterMarkFlag", moduleName, JsExtensionWindow::SetWaterMarkFlag);
    BindNativeFunction(env, objValue, "hidePrivacyContentForHost", moduleName,
                       JsExtensionWindow::HidePrivacyContentForHost);

    return objValue;
}

napi_value JsExtensionWindow::CreateJsExtensionWindowObject(napi_env env, sptr<Rosen::Window> window,
    sptr<AAFwk::SessionInfo> sessionInfo)
{
    TLOGI(WmsLogTag::WMS_UIEXT, "JsExtensionWindow CreateJsExtensionWindow");
    napi_value objValue = nullptr;
    napi_create_object(env, &objValue);

    if (env == nullptr || window == nullptr) {
        TLOGE(WmsLogTag::WMS_UIEXT, "JsExtensionWindow env or window is nullptr");
        return nullptr;
    }

    std::shared_ptr<ExtensionWindow> extensionWindow = std::make_shared<ExtensionWindowImpl>(window);
    std::unique_ptr<JsExtensionWindow> jsExtensionWindow = std::make_unique<JsExtensionWindow>(extensionWindow,
        sessionInfo);
    napi_wrap(env, objValue, jsExtensionWindow.release(), JsExtensionWindow::Finalizer, nullptr, nullptr);

    const char *moduleName = "JsExtensionWindow";
    BindNativeFunction(env, objValue, "on", moduleName, JsExtensionWindow::RegisterExtensionWindowCallback);
    BindNativeFunction(env, objValue, "off", moduleName, JsExtensionWindow::UnRegisterExtensionWindowCallback);
    BindNativeFunction(env, objValue, "moveWindowTo", moduleName, JsExtensionWindow::MoveWindowTo);
    BindNativeFunction(env, objValue, "resize", moduleName, JsExtensionWindow::ResizeWindow);
    BindNativeFunction(env, objValue, "getUIContext", moduleName, JsExtensionWindow::GetUIContext);
    BindNativeFunction(env, objValue, "setWindowBrightness", moduleName, JsExtensionWindow::SetWindowBrightness);
    BindNativeFunction(env, objValue, "setWindowKeepScreenOn", moduleName, JsExtensionWindow::SetWindowKeepScreenOn);
    BindNativeFunction(env, objValue, "showWindow", moduleName, JsExtensionWindow::ShowWindow);
    BindNativeFunction(env, objValue, "destroyWindow", moduleName, JsExtensionWindow::DestroyWindow);
    BindNativeFunction(env, objValue, "loadContent", moduleName, JsExtensionWindow::LoadContent);
    BindNativeFunction(env, objValue, "loadContentByName", moduleName, JsExtensionWindow::LoadContentByName);
    BindNativeFunction(env, objValue, "setUIContent", moduleName, JsExtensionWindow::SetUIContent);
    BindNativeFunction(env, objValue, "isWindowShowing", moduleName, JsExtensionWindow::IsWindowShowingSync);
    BindNativeFunction(env, objValue, "getWindowProperties", moduleName, JsExtensionWindow::GetWindowPropertiesSync);
    BindNativeFunction(env, objValue, "getWindowAvoidArea", moduleName, JsExtensionWindow::GetWindowAvoidArea);
    BindNativeFunction(env, objValue, "setWindowBackgroundColor", moduleName,
        JsExtensionWindow::SetWindowBackgroundColorSync);
    BindNativeFunction(env, objValue, "setSpecificSystemBarEnabled", moduleName,
        JsExtensionWindow::SetSpecificSystemBarEnabled);
    BindNativeFunction(env, objValue, "setPreferredOrientation", moduleName,
        JsExtensionWindow::SetPreferredOrientation);
    BindNativeFunction(env, objValue, "getPreferredOrientation", moduleName,
        JsExtensionWindow::GetPreferredOrientation);
    return objValue;
}

void JsExtensionWindow::Finalizer(napi_env env, void* data, void* hint)
{
    TLOGI(WmsLogTag::WMS_UIEXT, "Finalizer is called");
    std::unique_ptr<JsExtensionWindow>(static_cast<JsExtensionWindow*>(data));
}

napi_value JsExtensionWindow::GetWindowAvoidArea(napi_env env, napi_callback_info info)
{
    TLOGI(WmsLogTag::WMS_UIEXT, "GetWindowAvoidArea is called");
    JsExtensionWindow* me = CheckParamsAndGetThis<JsExtensionWindow>(env, info);
    return (me != nullptr) ? me->OnGetWindowAvoidArea(env, info) : nullptr;
}

napi_value JsExtensionWindow::RegisterExtensionWindowCallback(napi_env env, napi_callback_info info)
{
    TLOGI(WmsLogTag::WMS_UIEXT, "RegisterExtensionWindowCallback is called");
    JsExtensionWindow* me = CheckParamsAndGetThis<JsExtensionWindow>(env, info);
    return (me != nullptr) ? me->OnRegisterExtensionWindowCallback(env, info) : nullptr;
}

napi_value JsExtensionWindow::UnRegisterExtensionWindowCallback(napi_env env, napi_callback_info info)
{
    TLOGI(WmsLogTag::WMS_UIEXT, "UnRegisterExtensionWindowCallback is called");
    JsExtensionWindow* me = CheckParamsAndGetThis<JsExtensionWindow>(env, info);
    return (me != nullptr) ? me->OnUnRegisterExtensionWindowCallback(env, info) : nullptr;
}

napi_value JsExtensionWindow::HideNonSecureWindows(napi_env env, napi_callback_info info)
{
    TLOGI(WmsLogTag::WMS_UIEXT, "HideNonSecureWindows is called");
    JsExtensionWindow* me = CheckParamsAndGetThis<JsExtensionWindow>(env, info);
    return (me != nullptr) ? me->OnHideNonSecureWindows(env, info) : nullptr;
}

napi_value JsExtensionWindow::CreateSubWindowWithOptions(napi_env env, napi_callback_info info)
{
    TLOGI(WmsLogTag::WMS_UIEXT, "CreateSubWindowWithOptions is called");
    JsExtensionWindow* me = CheckParamsAndGetThis<JsExtensionWindow>(env, info);
    return (me != nullptr) ? me->OnCreateSubWindowWithOptions(env, info) : nullptr;
}

napi_value JsExtensionWindow::SetWaterMarkFlag(napi_env env, napi_callback_info info)
{
    TLOGI(WmsLogTag::WMS_UIEXT, "SetWaterMark is called");
    JsExtensionWindow* me = CheckParamsAndGetThis<JsExtensionWindow>(env, info);
    return (me != nullptr) ? me->OnSetWaterMarkFlag(env, info) : nullptr;
}

napi_value JsExtensionWindow::HidePrivacyContentForHost(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::WMS_UIEXT, "enter");
    JsExtensionWindow* me = CheckParamsAndGetThis<JsExtensionWindow>(env, info);
    return (me != nullptr) ? me->OnHidePrivacyContentForHost(env, info) : nullptr;
}

napi_value JsExtensionWindow::LoadContent(napi_env env, napi_callback_info info)
{
    TLOGI(WmsLogTag::WMS_UIEXT, "LoadContent is called");
    JsExtensionWindow* me = CheckParamsAndGetThis<JsExtensionWindow>(env, info);
    return (me != nullptr) ? me->OnLoadContent(env, info, false) : nullptr;
}

napi_value JsExtensionWindow::LoadContentByName(napi_env env, napi_callback_info info)
{
    TLOGI(WmsLogTag::WMS_UIEXT, "LoadContentByName is called");
    JsExtensionWindow* me = CheckParamsAndGetThis<JsExtensionWindow>(env, info);
    return (me != nullptr) ? me->OnLoadContent(env, info, true) : nullptr;
}

napi_value JsExtensionWindow::ShowWindow(napi_env env, napi_callback_info info)
{
    TLOGI(WmsLogTag::WMS_UIEXT, "ShowWindow is called");
    JsExtensionWindow* me = CheckParamsAndGetThis<JsExtensionWindow>(env, info);
    return (me != nullptr) ? me->OnShowWindow(env, info) : nullptr;
}

napi_value JsExtensionWindow::IsWindowShowingSync(napi_env env, napi_callback_info info)
{
    TLOGI(WmsLogTag::WMS_UIEXT, "IsShowing is called");
    JsExtensionWindow* me = CheckParamsAndGetThis<JsExtensionWindow>(env, info);
    return (me != nullptr) ? me->OnIsWindowShowingSync(env, info) : nullptr;
}

napi_value JsExtensionWindow::SetUIContent(napi_env env, napi_callback_info info)
{
    TLOGI(WmsLogTag::WMS_UIEXT, "LoadContent is called");
    JsExtensionWindow* me = CheckParamsAndGetThis<JsExtensionWindow>(env, info);
    return (me != nullptr) ? me->OnSetUIContent(env, info) : nullptr;
}

napi_value JsExtensionWindow::DestroyWindow(napi_env env, napi_callback_info info)
{
    TLOGI(WmsLogTag::WMS_UIEXT, "Destroy is called");
    JsExtensionWindow* me = CheckParamsAndGetThis<JsExtensionWindow>(env, info);
    return (me != nullptr) ? me->OnDestroyWindow(env, info) : nullptr;
}

napi_value JsExtensionWindow::SetWindowBackgroundColorSync(napi_env env, napi_callback_info info)
{
    TLOGI(WmsLogTag::WMS_UIEXT, "SetBackgroundColor is called");
    JsExtensionWindow* me = CheckParamsAndGetThis<JsExtensionWindow>(env, info);
    return (me != nullptr) ? me->OnSetWindowBackgroundColorSync(env, info) : nullptr;
}

napi_value JsExtensionWindow::GetWindowPropertiesSync(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::WMS_UIEXT, "GetProperties is called");
    JsExtensionWindow* me = CheckParamsAndGetThis<JsExtensionWindow>(env, info);
    return (me != nullptr) ? me->OnGetWindowPropertiesSync(env, info) : nullptr;
}

napi_value JsExtensionWindow::MoveWindowTo(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::WMS_UIEXT, "MoveTo");
    JsExtensionWindow* me = CheckParamsAndGetThis<JsExtensionWindow>(env, info);
    return (me != nullptr) ? me->OnMoveWindowTo(env, info) : nullptr;
}

napi_value JsExtensionWindow::ResizeWindow(napi_env env, napi_callback_info info)
{
    TLOGI(WmsLogTag::WMS_UIEXT, "Resize");
    JsExtensionWindow* me = CheckParamsAndGetThis<JsExtensionWindow>(env, info);
    return (me != nullptr) ? me->OnResizeWindow(env, info) : nullptr;
}

napi_value JsExtensionWindow::SetSpecificSystemBarEnabled(napi_env env, napi_callback_info info)
{
    TLOGI(WmsLogTag::WMS_UIEXT, "SetSystemBarEnable");
    JsExtensionWindow* me = CheckParamsAndGetThis<JsExtensionWindow>(env, info);
    return (me != nullptr) ? me->OnSetSpecificSystemBarEnabled(env, info) : nullptr;
}

napi_value JsExtensionWindow::SetPreferredOrientation(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::WMS_UIEXT, "SetPreferredOrientation");
    JsExtensionWindow* me = CheckParamsAndGetThis<JsExtensionWindow>(env, info);
    return (me != nullptr) ? me->OnSetPreferredOrientation(env, info) : nullptr;
}

napi_value JsExtensionWindow::GetPreferredOrientation(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::WMS_UIEXT, "GetPreferredOrientation");
    JsExtensionWindow* me = CheckParamsAndGetThis<JsExtensionWindow>(env, info);
    return (me != nullptr) ? me->OnGetPreferredOrientation(env, info) : nullptr;
}

napi_value JsExtensionWindow::GetUIContext(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::WMS_UIEXT, "GetUIContext");
    JsExtensionWindow* me = CheckParamsAndGetThis<JsExtensionWindow>(env, info);
    return (me != nullptr) ? me->OnGetUIContext(env, info) : nullptr;
}

napi_value JsExtensionWindow::SetWindowBrightness(napi_env env, napi_callback_info info)
{
    TLOGI(WmsLogTag::WMS_UIEXT, "SetBrightness");
    JsExtensionWindow* me = CheckParamsAndGetThis<JsExtensionWindow>(env, info);
    return (me != nullptr) ? me->OnSetWindowBrightness(env, info) : nullptr;
}

napi_value JsExtensionWindow::SetWindowKeepScreenOn(napi_env env, napi_callback_info info)
{
    TLOGI(WmsLogTag::WMS_UIEXT, "SetKeepScreenOn");
    JsExtensionWindow* me = CheckParamsAndGetThis<JsExtensionWindow>(env, info);
    return (me != nullptr) ? me->OnSetWindowKeepScreenOn(env, info) : nullptr;
}

napi_valuetype GetType(napi_env env, napi_value value)
{
    napi_valuetype res = napi_undefined;
    napi_typeof(env, value, &res);
    return res;
}

static void LoadContentTask(std::shared_ptr<NativeReference> contentStorage, std::string contextUrl,
    const std::shared_ptr<Rosen::ExtensionWindow> win, napi_env env, NapiAsyncTask& task,
    sptr<IRemoteObject> parentToken, bool isLoadedByName)
{
    napi_value nativeStorage = (contentStorage == nullptr) ? nullptr : contentStorage->GetNapiValue();
    sptr<Window> windowImpl = win->GetWindow();
    WMError ret;
    if (isLoadedByName) {
        ret = windowImpl->SetUIContentByName(contextUrl, env, nativeStorage);
    } else {
        ret = windowImpl->NapiSetUIContent(contextUrl, env, nativeStorage, BackupAndRestoreType::NONE, parentToken);
    }
    if (ret == WMError::WM_OK) {
        task.Resolve(env, NapiGetUndefined(env));
    } else {
        task.Reject(env, CreateJsError(env, static_cast<int32_t>(ret), "Window load content failed"));
    }
    TLOGI(WmsLogTag::WMS_UIEXT, "Window [%{public}u, %{public}s] load content end, ret = %{public}d",
        windowImpl->GetWindowId(), windowImpl->GetWindowName().c_str(), ret);
}

napi_value JsExtensionWindow::OnSetWindowKeepScreenOn(napi_env env, napi_callback_info info)
{
    size_t argc = 4;
    napi_value argv[4] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    napi_value lastParam = (argc <= 1) ? nullptr :
        ((argv[1] != nullptr && GetType(env, argv[1]) == napi_function) ? argv[1] : nullptr);
    napi_value result = nullptr;
    std::shared_ptr<NapiAsyncTask> napiAsyncTask = CreateEmptyAsyncTask(env, lastParam, &result);
    auto asyncTask = [env, task = napiAsyncTask]() {
        task->Reject(env,
            CreateJsError(env, static_cast<int32_t>(WmErrorCode::WM_ERROR_DEVICE_NOT_SUPPORT)));
    };
    if (napi_status::napi_ok != napi_send_event(env, asyncTask, napi_eprio_high)) {
        napiAsyncTask->Reject(env, CreateJsError(env,
            static_cast<int32_t>(WmErrorCode::WM_ERROR_STATE_ABNORMALLY), "send event failed"));
    }
    return result;
}

napi_value JsExtensionWindow::OnSetWindowBrightness(napi_env env, napi_callback_info info)
{
    size_t argc = 4;
    napi_value argv[4] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    napi_value lastParam = (argc <= 1) ? nullptr :
        ((argv[1] != nullptr && GetType(env, argv[1]) == napi_function) ? argv[1] : nullptr);
    napi_value result = nullptr;
    std::shared_ptr<NapiAsyncTask> napiAsyncTask = CreateEmptyAsyncTask(env, lastParam, &result);
    auto asyncTask = [env, task = napiAsyncTask]() {
        task->Reject(env,
            CreateJsError(env, static_cast<int32_t>(WmErrorCode::WM_ERROR_DEVICE_NOT_SUPPORT)));
    };
    if (napi_status::napi_ok != napi_send_event(env, asyncTask, napi_eprio_high)) {
        napiAsyncTask->Reject(env, CreateJsError(env,
            static_cast<int32_t>(WmErrorCode::WM_ERROR_STATE_ABNORMALLY), "send event failed"));
    }
    return result;
}

napi_value JsExtensionWindow::OnGetUIContext(napi_env env, napi_callback_info info)
{
    return NapiThrowError(env, WmErrorCode::WM_ERROR_DEVICE_NOT_SUPPORT);
}

napi_value JsExtensionWindow::OnSetPreferredOrientation(napi_env env, napi_callback_info info)
{
    size_t argc = 4;
    napi_value argv[4] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    napi_value lastParam = (argc <= 1) ? nullptr :
        ((argv[1] != nullptr && GetType(env, argv[1]) == napi_function) ? argv[1] : nullptr);
    napi_value result = nullptr;
    std::shared_ptr<NapiAsyncTask> napiAsyncTask = CreateEmptyAsyncTask(env, lastParam, &result);
    auto asyncTask = [env, task = napiAsyncTask]() {
        task->Reject(env,
            CreateJsError(env, static_cast<int32_t>(WmErrorCode::WM_ERROR_DEVICE_NOT_SUPPORT)));
    };
    if (napi_status::napi_ok != napi_send_event(env, asyncTask, napi_eprio_high)) {
        napiAsyncTask->Reject(env, CreateJsError(env,
            static_cast<int32_t>(WmErrorCode::WM_ERROR_STATE_ABNORMALLY), "send event failed"));
    }
    return result;
}

napi_value JsExtensionWindow::OnGetPreferredOrientation(napi_env env, napi_callback_info info)
{
    return NapiThrowError(env, WmErrorCode::WM_ERROR_DEVICE_NOT_SUPPORT);
}

napi_value JsExtensionWindow::OnSetSpecificSystemBarEnabled(napi_env env, napi_callback_info info)
{
    napi_value result = nullptr;
    std::shared_ptr<NapiAsyncTask> napiAsyncTask = CreateEmptyAsyncTask(env, nullptr, &result);
    auto asyncTask = [env, task = napiAsyncTask]() {
        task->Reject(env,
            CreateJsError(env, static_cast<int32_t>(WmErrorCode::WM_ERROR_DEVICE_NOT_SUPPORT)));
    };
    if (napi_status::napi_ok != napi_send_event(env, asyncTask, napi_eprio_high)) {
        napiAsyncTask->Reject(env, CreateJsError(env,
            static_cast<int32_t>(WmErrorCode::WM_ERROR_STATE_ABNORMALLY), "send event failed"));
    }
    return result;
}

napi_value JsExtensionWindow::OnResizeWindow(napi_env env, napi_callback_info info)
{
    size_t argc = 4;
    napi_value argv[4] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    napi_value lastParam = (argc <= 2) ? nullptr :
        ((argv[2] != nullptr && GetType(env, argv[2]) == napi_function) ? argv[2] : nullptr);
    napi_value result = nullptr;
    std::shared_ptr<NapiAsyncTask> napiAsyncTask = CreateEmptyAsyncTask(env, lastParam, &result);
    auto asyncTask = [env, task = napiAsyncTask]() {
        task->Reject(env,
            CreateJsError(env, static_cast<int32_t>(WmErrorCode::WM_ERROR_DEVICE_NOT_SUPPORT)));
    };
    if (napi_status::napi_ok != napi_send_event(env, asyncTask, napi_eprio_high)) {
        napiAsyncTask->Reject(env, CreateJsError(env,
            static_cast<int32_t>(WmErrorCode::WM_ERROR_STATE_ABNORMALLY), "send event failed"));
    }
    return result;
}

napi_value JsExtensionWindow::OnMoveWindowTo(napi_env env, napi_callback_info info)
{
    size_t argc = 4;
    napi_value argv[4] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    napi_value lastParam = (argc <= 2) ? nullptr :
        ((argv[2] != nullptr && GetType(env, argv[2]) == napi_function) ? argv[2] : nullptr);
    napi_value result = nullptr;
    std::shared_ptr<NapiAsyncTask> napiAsyncTask = CreateEmptyAsyncTask(env, lastParam, &result);
    auto asyncTask = [env, task = napiAsyncTask]() {
        task->Reject(env,
            CreateJsError(env, static_cast<int32_t>(WmErrorCode::WM_ERROR_DEVICE_NOT_SUPPORT)));
    };
    if (napi_status::napi_ok != napi_send_event(env, asyncTask, napi_eprio_high)) {
        napiAsyncTask->Reject(env, CreateJsError(env,
            static_cast<int32_t>(WmErrorCode::WM_ERROR_STATE_ABNORMALLY), "send event failed"));
    }
    return result;
}

napi_value JsExtensionWindow::OnGetWindowPropertiesSync(napi_env env, napi_callback_info info)
{
    sptr<Window> windowImpl = extensionWindow_->GetWindow();
    if (windowImpl == nullptr) {
        TLOGW(WmsLogTag::WMS_UIEXT, "window is nullptr");
        return NapiThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
    }
    auto objValue = CreateJsExtensionWindowProperties(env, windowImpl);
    TLOGI(WmsLogTag::WMS_UIEXT, "Window [%{public}u, %{public}s] get properties end",
        windowImpl->GetWindowId(), windowImpl->GetWindowName().c_str());
    if (objValue != nullptr) {
        return objValue;
    } else {
        return NapiThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
    }
}

napi_value JsExtensionWindow::OnSetWindowBackgroundColorSync(napi_env env, napi_callback_info info)
{
    return NapiThrowError(env, WmErrorCode::WM_ERROR_DEVICE_NOT_SUPPORT);
}

napi_value JsExtensionWindow::OnDestroyWindow(napi_env env, napi_callback_info info)
{
    size_t argc = 4;
    napi_value argv[4] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    napi_value lastParam = (argc == 0) ? nullptr :
        ((argv[0] != nullptr && GetType(env, argv[0]) == napi_function) ? argv[0] : nullptr);
    napi_value result = nullptr;
    std::shared_ptr<NapiAsyncTask> napiAsyncTask = CreateEmptyAsyncTask(env, lastParam, &result);
    auto asyncTask = [extWindow = extensionWindow_, env, task = napiAsyncTask]() {
        if (extWindow == nullptr) {
            TLOGNE(WmsLogTag::WMS_UIEXT, "extensionWindow is null");
            task->Reject(env,
                CreateJsError(env, static_cast<int32_t>(WmErrorCode::WM_ERROR_STATE_ABNORMALLY)));
            return;
        }
        sptr<Window> windowImpl = extWindow->GetWindow();
        if (windowImpl == nullptr) {
            TLOGNE(WmsLogTag::WMS_UIEXT, "window is nullptr");
            task->Reject(env,
                CreateJsError(env, static_cast<int32_t>(WmErrorCode::WM_ERROR_STATE_ABNORMALLY)));
            return;
        }
        WmErrorCode ret = WM_JS_TO_ERROR_CODE_MAP.at(windowImpl->Destroy());
        TLOGNI(WmsLogTag::WMS_UIEXT, "Window [%{public}u, %{public}s] destroy end, ret = %{public}d",
            windowImpl->GetWindowId(), windowImpl->GetWindowName().c_str(), ret);
        if (ret != WmErrorCode::WM_OK) {
            task->Reject(env,
                CreateJsError(env, static_cast<int32_t>(ret),
                "Window destroy failed"));
            return;
        }
        task->Resolve(env, NapiGetUndefined(env));
    };
    if (napi_status::napi_ok != napi_send_event(env, asyncTask, napi_eprio_high)) {
        napiAsyncTask->Reject(env, CreateJsError(env,
            static_cast<int32_t>(WmErrorCode::WM_ERROR_STATE_ABNORMALLY), "send event failed"));
    }
    return result;
}

napi_value JsExtensionWindow::OnIsWindowShowingSync(napi_env env, napi_callback_info info)
{
    sptr<Window> windowImpl = extensionWindow_->GetWindow();
    if (windowImpl == nullptr) {
        TLOGE(WmsLogTag::WMS_UIEXT, "window is nullptr");
        return NapiThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
    }
    bool state = (windowImpl->GetWindowState() == WindowState::STATE_SHOWN);
    TLOGI(WmsLogTag::WMS_UIEXT, "Window [%{public}u, %{public}s] get show state end, state = %{public}u",
        windowImpl->GetWindowId(), windowImpl->GetWindowName().c_str(), state);
    return CreateJsValue(env, state);
}

napi_value JsExtensionWindow::OnShowWindow(napi_env env, napi_callback_info info)
{
    size_t argc = 4;
    napi_value argv[4] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    napi_value lastParam = (argc == 0) ? nullptr :
        ((argv[0] != nullptr && GetType(env, argv[0]) == napi_function) ? argv[0] : nullptr);
    napi_value result = nullptr;
    std::shared_ptr<NapiAsyncTask> napiAsyncTask = CreateEmptyAsyncTask(env, lastParam, &result);
    auto asyncTask = [extWindow = extensionWindow_, env, task = napiAsyncTask]() {
        if (extWindow == nullptr) {
            TLOGNE(WmsLogTag::WMS_UIEXT, "extensionWindow is null");
            task->Reject(env, CreateJsError(env,
                static_cast<int32_t>(WmErrorCode::WM_ERROR_STATE_ABNORMALLY)));
            return;
        }
        sptr<Window> windowImpl = extWindow->GetWindow();
        if (windowImpl == nullptr) {
            TLOGNE(WmsLogTag::WMS_UIEXT, "window is nullptr");
            task->Reject(env, CreateJsError(env,
                static_cast<int32_t>(WmErrorCode::WM_ERROR_STATE_ABNORMALLY)));
            return;
        }
        WMError ret = windowImpl->Show(0, false);
        if (ret == WMError::WM_OK) {
            task->Resolve(env, NapiGetUndefined(env));
        } else {
            task->Reject(env, CreateJsError(env,
                static_cast<int32_t>(WM_JS_TO_ERROR_CODE_MAP.at(ret)), "Window show failed"));
        }
        TLOGNI(WmsLogTag::WMS_UIEXT, "Window [%{public}u, %{public}s] show end, ret = %{public}d",
            windowImpl->GetWindowId(), windowImpl->GetWindowName().c_str(), ret);
    };
    if (napi_status::napi_ok != napi_send_event(env, asyncTask, napi_eprio_high)) {
        napiAsyncTask->Reject(env, CreateJsError(env,
            static_cast<int32_t>(WmErrorCode::WM_ERROR_STATE_ABNORMALLY), "send event failed"));
    }
    return result;
}

napi_value JsExtensionWindow::OnSetUIContent(napi_env env, napi_callback_info info)
{
    WmErrorCode errCode = WmErrorCode::WM_OK;
    size_t argc = 4;
    napi_value argv[4] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc < 1) { // 2 maximum param num
        TLOGE(WmsLogTag::WMS_UIEXT, "Argc is invalid: %{public}zu", argc);
        errCode = WmErrorCode::WM_ERROR_INVALID_PARAM;
    }
    std::string contextUrl;
    if (errCode == WmErrorCode::WM_OK && !ConvertFromJsValue(env, argv[0], contextUrl)) {
        TLOGE(WmsLogTag::WMS_UIEXT, "Failed to convert parameter to context url");
        errCode = WmErrorCode::WM_ERROR_INVALID_PARAM;
    }
    napi_value lastParam = nullptr;
    if (argc >= 2) { // 2 param num
        lastParam = argv[1];
    }
    std::shared_ptr<NativeReference> contentStorage = nullptr;
    if (errCode == WmErrorCode::WM_ERROR_INVALID_PARAM) {
        return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM);
    }

    sptr<IRemoteObject> parentToken = sessionInfo_->parentToken;
    napi_value result = nullptr;
    std::shared_ptr<NapiAsyncTask> napiAsyncTask = CreateEmptyAsyncTask(env, lastParam, &result);
    auto asyncTask = [extWindow = extensionWindow_, contentStorage, contextUrl, parentToken,
        env, task = napiAsyncTask]() {
        if (extWindow == nullptr) {
            TLOGNE(WmsLogTag::WMS_UIEXT, "Window is nullptr");
            task->Reject(env,
                CreateJsError(env, static_cast<int32_t>(WmErrorCode::WM_ERROR_STATE_ABNORMALLY)));
            return;
        }
        LoadContentTask(contentStorage, contextUrl, extWindow, env, *task, parentToken, false);
    };
    if (napi_status::napi_ok != napi_send_event(env, asyncTask, napi_eprio_high)) {
        napiAsyncTask->Reject(env, CreateJsError(env,
            static_cast<int32_t>(WmErrorCode::WM_ERROR_STATE_ABNORMALLY), "send event failed"));
    }
    return result;
}

napi_value JsExtensionWindow::OnLoadContent(napi_env env, napi_callback_info info, bool isLoadedByName)
{
    TLOGI(WmsLogTag::WMS_UIEXT, "OnLoadContent is called");
    WmErrorCode errCode = WmErrorCode::WM_OK;
    size_t argc = 4;
    napi_value argv[4] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    std::string contextUrl;
    if (!ConvertFromJsValue(env, argv[0], contextUrl)) {
        TLOGI(WmsLogTag::WMS_UIEXT, "Failed to convert parameter to context url");
        errCode = WmErrorCode::WM_ERROR_INVALID_PARAM;
    }
    napi_value storage = nullptr;
    napi_value lastParam = nullptr;
    napi_value value1 = argv[1];
    napi_value value2 = argv[2];
    if (GetType(env, value1) == napi_function) {
        lastParam = value1;
    } else if (GetType(env, value1) == napi_object) {
        storage = value1;
    }
    if (GetType(env, value2) == napi_function) {
        lastParam = value2;
    }
    if (errCode == WmErrorCode::WM_ERROR_INVALID_PARAM) {
        TLOGI(WmsLogTag::WMS_UIEXT, "Invalid param");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WmErrorCode::WM_ERROR_INVALID_PARAM)));
        return NapiGetUndefined(env);
    }

    std::shared_ptr<NativeReference> contentStorage = nullptr;
    if (storage != nullptr) {
        napi_ref result = nullptr;
        napi_create_reference(env, storage, 1, &result);
        contentStorage = std::shared_ptr<NativeReference>(reinterpret_cast<NativeReference*>(result));
    }

    sptr<IRemoteObject> parentToken = sessionInfo_->parentToken;
    napi_value result = nullptr;
    std::shared_ptr<NapiAsyncTask> napiAsyncTask = CreateEmptyAsyncTask(env, lastParam, &result);
    auto asyncTask = [extWindow = extensionWindow_, contentStorage, contextUrl, parentToken, isLoadedByName,
        env, task = napiAsyncTask]() {
        if (extWindow == nullptr) {
            TLOGNE(WmsLogTag::WMS_UIEXT, "Window is nullptr");
            task->Reject(env, CreateJsError(env, static_cast<int32_t>(WmErrorCode::WM_ERROR_STATE_ABNORMALLY)));
            return;
        }
        LoadContentTask(contentStorage, contextUrl, extWindow, env, *task, parentToken, isLoadedByName);
    };
    if (napi_status::napi_ok != napi_send_event(env, asyncTask, napi_eprio_high)) {
        napiAsyncTask->Reject(env, CreateJsError(env,
            static_cast<int32_t>(WmErrorCode::WM_ERROR_STATE_ABNORMALLY), "send event failed"));
    }
    return result;
}

napi_value JsExtensionWindow::OnGetWindowAvoidArea(napi_env env, napi_callback_info info)
{
    TLOGI(WmsLogTag::WMS_UIEXT, "OnGetWindowAvoidArea is called");

    WmErrorCode errCode = WmErrorCode::WM_OK;
    size_t argc = 4;
    napi_value argv[4] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc < 1) { // 1: params num
        return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM);
    }
    AvoidAreaType avoidAreaType = AvoidAreaType::TYPE_SYSTEM;
    napi_value nativeMode = argv[0];
    if (nativeMode == nullptr) {
        errCode = WmErrorCode::WM_ERROR_INVALID_PARAM;
    } else {
        uint32_t resultValue = 0;
        napi_get_value_uint32(env, nativeMode, &resultValue);
        avoidAreaType = static_cast<AvoidAreaType>(resultValue);
        errCode = ((avoidAreaType > AvoidAreaType::TYPE_NAVIGATION_INDICATOR) ||
                   (avoidAreaType < AvoidAreaType::TYPE_SYSTEM)) ?
                  WmErrorCode::WM_ERROR_INVALID_PARAM : WmErrorCode::WM_OK;
    }
    if (errCode == WmErrorCode::WM_ERROR_INVALID_PARAM) {
        return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM);
    }

    if (extensionWindow_ == nullptr) {
        TLOGE(WmsLogTag::WMS_UIEXT, "extensionWindow_ is nullptr");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WmErrorCode::WM_ERROR_STAGE_ABNORMALLY)));
        return CreateJsValue(env, static_cast<int32_t>(WmErrorCode::WM_ERROR_STAGE_ABNORMALLY));
    }
    // getAvoidRect by avoidAreaType
    AvoidArea avoidArea;
    WmErrorCode ret = WM_JS_TO_ERROR_CODE_MAP.at(extensionWindow_->GetAvoidAreaByType(avoidAreaType, avoidArea));
    if (ret != WmErrorCode::WM_OK) {
        TLOGE(WmsLogTag::WMS_UIEXT, "OnGetAvoidAreaByType failed");
        avoidArea.topRect_ = g_emptyRect;
        avoidArea.leftRect_ = g_emptyRect;
        avoidArea.rightRect_ = g_emptyRect;
        avoidArea.bottomRect_ = g_emptyRect;
    }
    napi_value avoidAreaObj = ConvertAvoidAreaToJsValue(env, avoidArea, avoidAreaType);
    if (avoidAreaObj != nullptr) {
        TLOGI(WmsLogTag::WMS_UIEXT, "avoidAreaObj is finish");
        return avoidAreaObj;
    } else {
        TLOGE(WmsLogTag::WMS_UIEXT, "avoidAreaObj is nullptr");
        return NapiThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
    }
}

napi_value JsExtensionWindow::OnRegisterRectChangeCallback(napi_env env, size_t argc, napi_value* argv,
    const sptr<Window>& windowImpl)
{
    if (argc < ARG_COUNT_THREE) {
        TLOGE(WmsLogTag::WMS_UIEXT, "OnRectChange: argc is invalid: %{public}zu", argc);
        return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM);
    }
    if (!windowImpl->IsPcWindow()) {
        TLOGE(WmsLogTag::WMS_UIEXT, "Device is not PC");
        return NapiThrowError(env, WmErrorCode::WM_ERROR_DEVICE_NOT_SUPPORT);
    }
    uint32_t reasons = 0;
    if (!ConvertFromJsValue(env, argv[INDEX_ONE], reasons)) {
        TLOGE(WmsLogTag::WMS_UIEXT, "Failed to convert parameter to rectChangeReasons");
        return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM);
    }
    if (reasons != static_cast<uint32_t>(ComponentRectChangeReason::HOST_WINDOW_RECT_CHANGE)) {
        TLOGE(WmsLogTag::WMS_UIEXT, "Unsupported rect change reasons");
        return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM);
    }
    napi_value cbValue = argv[INDEX_TWO];
    if (!NapiIsCallable(env, cbValue)) {
        TLOGE(WmsLogTag::WMS_UIEXT, "Callback(info->argv[2]) is not callable");
        return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM);
    }
    WmErrorCode ret = extensionRegisterManager_->RegisterListener(windowImpl, RECT_CHANGE, CaseType::CASE_WINDOW,
        env, cbValue);
    if (ret != WmErrorCode::WM_OK) {
        TLOGW(WmsLogTag::WMS_UIEXT, "Failed, window [%{public}u, %{public}s], type=%{public}s, reasons=%{public}u",
            windowImpl->GetWindowId(), windowImpl->GetWindowName().c_str(), RECT_CHANGE.c_str(), reasons);
        return NapiThrowError(env, ret);
    }
    TLOGI(WmsLogTag::WMS_UIEXT, "Success, window [%{public}u, %{public}s], type=%{public}s, reasons = %{public}u",
        windowImpl->GetWindowId(), windowImpl->GetWindowName().c_str(), RECT_CHANGE.c_str(), reasons);
    return NapiGetUndefined(env);
}

napi_value JsExtensionWindow::OnRegisterExtensionWindowCallback(napi_env env, napi_callback_info info)
{
    sptr<Window> windowImpl = extensionWindow_->GetWindow();
    if (windowImpl == nullptr) {
        TLOGE(WmsLogTag::WMS_UIEXT, "WindowImpl is nullptr");
        return NapiThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
    }
    size_t argc = FOUR_PARAMS_SIZE;
    napi_value argv[FOUR_PARAMS_SIZE] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc < ARG_COUNT_TWO) {
        TLOGE(WmsLogTag::WMS_UIEXT, "Argc is invalid: %{public}zu", argc);
        return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM);
    }
    std::string cbType;
    if (!ConvertFromJsValue(env, argv[INDEX_ZERO], cbType)) {
        TLOGE(WmsLogTag::WMS_UIEXT, "Failed to convert parameter to callbackType");
        return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM);
    }
    if (cbType == RECT_CHANGE) {
        return OnRegisterRectChangeCallback(env, argc, argv, windowImpl);
    }
    napi_value value = argv[INDEX_ONE];
    if (!NapiIsCallable(env, value)) {
        TLOGE(WmsLogTag::WMS_UIEXT, "Callback(info->argv[1]) is not callable");
        return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM);
    }
    WmErrorCode ret = extensionRegisterManager_->RegisterListener(windowImpl, cbType, CaseType::CASE_WINDOW,
        env, value);
    if (ret != WmErrorCode::WM_OK) {
        TLOGE(WmsLogTag::WMS_UIEXT, "Callback(info->argv[1]) is not callable");
        return NapiThrowError(env, ret);
    }
    TLOGI(WmsLogTag::WMS_UIEXT, "Register end, window [%{public}u, %{public}s], type = %{public}s",
          windowImpl->GetWindowId(), windowImpl->GetWindowName().c_str(), cbType.c_str());
    return NapiGetUndefined(env);
}

napi_value JsExtensionWindow::OnUnRegisterExtensionWindowCallback(napi_env env, napi_callback_info info)
{
    sptr<Window> windowImpl = extensionWindow_->GetWindow();
    if (windowImpl == nullptr) {
        TLOGE(WmsLogTag::WMS_UIEXT, "windowImpl is nullptr");
        return NapiThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
    }
    size_t argc = FOUR_PARAMS_SIZE;
    napi_value argv[FOUR_PARAMS_SIZE] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc < ARG_COUNT_ONE) {
        TLOGE(WmsLogTag::WMS_UIEXT, "Argc is invalid: %{public}zu", argc);
        return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM);
    }
    std::string cbType;
    if (!ConvertFromJsValue(env, argv[INDEX_ZERO], cbType)) {
        TLOGE(WmsLogTag::WMS_UIEXT, "Failed to convert parameter to callbackType");
        return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM);
    }
    if (cbType == RECT_CHANGE) {
        if (!windowImpl->IsPcWindow()) {
            TLOGE(WmsLogTag::WMS_UIEXT, "Device is not PC");
            return NapiThrowError(env, WmErrorCode::WM_ERROR_DEVICE_NOT_SUPPORT);
        }
    }

    napi_value value = nullptr;
    WmErrorCode ret = WmErrorCode::WM_OK;
    if (argc == ARG_COUNT_ONE) {
        ret = extensionRegisterManager_->UnregisterListener(windowImpl, cbType, CaseType::CASE_WINDOW, env, value);
    } else {
        value = argv[INDEX_ONE];
        if (value == nullptr || !NapiIsCallable(env, value)) {
            ret = extensionRegisterManager_->UnregisterListener(windowImpl, cbType, CaseType::CASE_WINDOW,
                env, nullptr);
        } else {
            ret = extensionRegisterManager_->UnregisterListener(windowImpl, cbType, CaseType::CASE_WINDOW, env, value);
        }
    }

    if (ret != WmErrorCode::WM_OK) {
        return NapiThrowError(env, ret);
    }
    TLOGI(WmsLogTag::WMS_UIEXT, "UnRegister end, window [%{public}u, %{public}s], type = %{public}s",
          windowImpl->GetWindowId(), windowImpl->GetWindowName().c_str(), cbType.c_str());
    return NapiGetUndefined(env);
}

napi_value JsExtensionWindow::OnHideNonSecureWindows(napi_env env, napi_callback_info info)
{
    if (extensionWindow_ == nullptr) {
        TLOGE(WmsLogTag::WMS_UIEXT, "extensionWindow_ is nullptr");
        return NapiThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
    }
    sptr<Window> windowImpl = extensionWindow_->GetWindow();
    if (windowImpl == nullptr) {
        TLOGE(WmsLogTag::WMS_UIEXT, "windowImpl is nullptr");
        return NapiThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
    }
    size_t argc = 4;
    napi_value argv[4] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc < 1) {
        TLOGE(WmsLogTag::WMS_UIEXT, "Argc is invalid: %{public}zu", argc);
        return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM);
    }
    bool shouldHide = false;
    if (!ConvertFromJsValue(env, argv[0], shouldHide)) {
        TLOGE(WmsLogTag::WMS_UIEXT, "Failed to convert parameter to bool");
        return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM);
    }

    WmErrorCode ret = WmErrorCode::WM_OK;
    ret = WM_JS_TO_ERROR_CODE_MAP.at(extensionWindow_->HideNonSecureWindows(shouldHide));
    if (ret != WmErrorCode::WM_OK) {
        return NapiThrowError(env, ret);
    }
    TLOGI(WmsLogTag::WMS_UIEXT, "OnHideNonSecureWindows end, window [%{public}u, %{public}s], shouldHide:%{public}u",
          windowImpl->GetWindowId(), windowImpl->GetWindowName().c_str(), shouldHide);
    return NapiGetUndefined(env);
}

napi_value JsExtensionWindow::OnSetWaterMarkFlag(napi_env env, napi_callback_info info)
{
    if (extensionWindow_ == nullptr) {
        TLOGE(WmsLogTag::WMS_UIEXT, "extensionWindow_ is nullptr");
        return NapiThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
    }
    sptr<Window> windowImpl = extensionWindow_->GetWindow();
    if (windowImpl == nullptr) {
        TLOGE(WmsLogTag::WMS_UIEXT, "windowImpl is nullptr");
        return NapiThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
    }
    size_t argc = 4;
    napi_value argv[4] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc < 1) {
        TLOGE(WmsLogTag::WMS_UIEXT, "Argc is invalid: %{public}zu", argc);
        return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM);
    }
    bool isEnable = false;
    if (!ConvertFromJsValue(env, argv[0], isEnable)) {
        TLOGE(WmsLogTag::WMS_UIEXT, "Failed to convert parameter to bool");
        return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM);
    }

    WmErrorCode ret = WmErrorCode::WM_OK;
    ret = WM_JS_TO_ERROR_CODE_MAP.at(extensionWindow_->SetWaterMarkFlag(isEnable));
    if (ret != WmErrorCode::WM_OK) {
        return NapiThrowError(env, ret);
    }
    TLOGI(WmsLogTag::WMS_UIEXT, "OnSetWaterMark end, window [%{public}u, %{public}s], isEnable:%{public}u.",
          windowImpl->GetWindowId(), windowImpl->GetWindowName().c_str(), isEnable);
    return NapiGetUndefined(env);
}

napi_value JsExtensionWindow::OnHidePrivacyContentForHost(napi_env env, napi_callback_info info)
{
    if (extensionWindow_ == nullptr) {
        TLOGE(WmsLogTag::WMS_UIEXT, "extension window is nullptr");
        return NapiThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
    }

    sptr<Window> windowImpl = extensionWindow_->GetWindow();
    if (windowImpl == nullptr) {
        TLOGE(WmsLogTag::WMS_UIEXT, "windowImpl is nullptr");
        return NapiThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
    }

    size_t argc = 4;
    napi_value argv[4] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc < 1) {
        TLOGE(WmsLogTag::WMS_UIEXT, "Argc is invalid: %{public}zu", argc);
        return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM);
    }

    bool needHide = false;
    if (!ConvertFromJsValue(env, argv[0], needHide)) {
        TLOGE(WmsLogTag::WMS_UIEXT, "Failed to convert parameter to bool");
        return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM);
    }

    auto ret = WM_JS_TO_ERROR_CODE_MAP.at(extensionWindow_->HidePrivacyContentForHost(needHide));
    if (ret != WmErrorCode::WM_OK) {
        return NapiThrowError(env, ret);
    }

    TLOGI(WmsLogTag::WMS_UIEXT, "finished, window [%{public}u, %{public}s], needHide:%{public}u.",
          windowImpl->GetWindowId(), windowImpl->GetWindowName().c_str(), needHide);

    return NapiGetUndefined(env);
}

napi_value JsExtensionWindow::GetProperties(napi_env env, napi_callback_info info)
{
    TLOGI(WmsLogTag::WMS_UIEXT, "GetProperties is called");
    napi_value jsThis;
    NAPI_CALL(env, napi_get_cb_info(env, info, nullptr, nullptr, &jsThis, nullptr));

    JsExtensionWindow* jsExtensionWindow = nullptr;
    NAPI_CALL(env, napi_unwrap(env, jsThis, reinterpret_cast<void**>(&jsExtensionWindow)));
    if (!jsExtensionWindow || !jsExtensionWindow->extensionWindow_) {
        TLOGE(WmsLogTag::WMS_UIEXT, "window is nullptr");
        return nullptr;
    }
    sptr<Rosen::Window> window = jsExtensionWindow->extensionWindow_->GetWindow();
    return CreateJsExtensionWindowPropertiesObject(env, window);
}

napi_value JsExtensionWindow::OnCreateSubWindowWithOptions(napi_env env, napi_callback_info info)
{
    if (extensionWindow_ == nullptr) {
        TLOGE(WmsLogTag::WMS_UIEXT, "[NAPI]extensionWindow is null");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WmErrorCode::WM_ERROR_STATE_ABNORMALLY)));
        return NapiGetUndefined(env);
    }
    size_t argc = 4;
    napi_value argv[4] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    std::string windowName;
    if (!ConvertFromJsValue(env, argv[0], windowName)) {
        TLOGE(WmsLogTag::WMS_UIEXT, "[NAPI]Failed to convert parameter to windowName");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WmErrorCode::WM_ERROR_INVALID_PARAM)));
        return NapiGetUndefined(env);
    }
    sptr<WindowOption> option = new WindowOption();
    if (!ParseSubWindowOptions(env, argv[1], option)) {
        TLOGE(WmsLogTag::WMS_UIEXT, "[NAPI]Get invalid options param");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WmErrorCode::WM_ERROR_INVALID_PARAM)));
        return NapiGetUndefined(env);
    }
    if ((option->GetWindowFlags() & static_cast<uint32_t>(WindowFlag::WINDOW_FLAG_IS_APPLICATION_MODAL)) &&
        !extensionWindow_->IsPcOrPadFreeMultiWindowMode()) {
        TLOGE(WmsLogTag::WMS_SUB, "device not support");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WmErrorCode::WM_ERROR_DEVICE_NOT_SUPPORT)));
        return NapiGetUndefined(env);
    }
    if (option->GetWindowTopmost() && !Permission::IsSystemCalling() && !Permission::IsStartByHdcd()) {
        TLOGE(WmsLogTag::WMS_SUB, "Modal subwindow has topmost, but no system permission");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WmErrorCode::WM_ERROR_NOT_SYSTEM_APP)));
        return NapiGetUndefined(env);
    }
    option->SetParentId(hostWindowId_);
    const char* const where = __func__;
    napi_value lastParam = (argv[2] != nullptr && GetType(env, argv[2]) == napi_function) ? argv[2] : nullptr;
    napi_value result = nullptr;
    std::shared_ptr<NapiAsyncTask> napiAsyncTask = CreateEmptyAsyncTask(env, lastParam, &result);
    auto asyncTask = [where, extensionWindow = extensionWindow_, windowName = std::move(windowName),
        windowOption = option, env, task = napiAsyncTask]() mutable {
        auto extWindow = extensionWindow->GetWindow();
        if (extWindow == nullptr) {
            task->Reject(env, CreateJsError(env,
                static_cast<int32_t>(WmErrorCode::WM_ERROR_STATE_ABNORMALLY), "extension's window is null"));
            return;
        }
        windowOption->SetWindowType(Rosen::WindowType::WINDOW_TYPE_APP_SUB_WINDOW);
        windowOption->SetWindowMode(Rosen::WindowMode::WINDOW_MODE_FLOATING);
        windowOption->SetOnlySupportSceneBoard(true);
        windowOption->SetExtensionTag(true);
        auto window = Window::Create(windowName, windowOption, extWindow->GetContext());
        if (window == nullptr) {
            task->Reject(env, CreateJsError(env,
                static_cast<int32_t>(WmErrorCode::WM_ERROR_STATE_ABNORMALLY), "create sub window failed"));
            return;
        }
        if (!window->IsTopmost()) {
            extWindow->NotifyModalUIExtensionMayBeCovered(false);
        }
        task->Resolve(env, CreateJsWindowObject(env, window));
        TLOGNI(WmsLogTag::WMS_UIEXT, "%{public}s [NAPI]Create sub window %{public}s end",
            where, windowName.c_str());
    };
    if (napi_status::napi_ok != napi_send_event(env, asyncTask, napi_eprio_high)) {
        napiAsyncTask->Reject(env, CreateJsError(env,
            static_cast<int32_t>(WmErrorCode::WM_ERROR_STATE_ABNORMALLY), "send event failed"));
    }
    return result;
}

}  // namespace Rosen
}  // namespace OHOS
