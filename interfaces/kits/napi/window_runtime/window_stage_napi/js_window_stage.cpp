/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

#include "js_err_utils.h"
#include "js_window_stage.h"
#include "js_window.h"
#include "window_manager_hilog.h"
#include "permission.h"

namespace OHOS {
namespace Rosen {
using namespace AbilityRuntime;
namespace {
const int CONTENT_STORAGE_ARG = 2;
constexpr size_t INDEX_ZERO = 0;
constexpr size_t FOUR_PARAMS_SIZE = 4;
constexpr HiviewDFX::HiLogLabel LABEL = {LOG_CORE, HILOG_DOMAIN_WINDOW, "JsWindowStage"};
} // namespace

std::unique_ptr<JsWindowRegisterManager> g_listenerManager = std::make_unique<JsWindowRegisterManager>();
JsWindowStage::JsWindowStage(const std::shared_ptr<Rosen::WindowScene>& windowScene)
    : windowScene_(windowScene)
{
}

JsWindowStage::~JsWindowStage()
{
}

void JsWindowStage::Finalizer(napi_env env, void* data, void* hint)
{
    WLOGI("[NAPI]Finalizer");
    std::unique_ptr<JsWindowStage>(static_cast<JsWindowStage*>(data));
}

napi_value JsWindowStage::SetUIContent(napi_env env, napi_callback_info info)
{
    WLOGFI("[NAPI]");
    JsWindowStage* me = CheckParamsAndGetThis<JsWindowStage>(env, info);
    return (me != nullptr) ? me->OnSetUIContent(env, info) : nullptr;
}

napi_value JsWindowStage::GetMainWindow(napi_env env, napi_callback_info info)
{
    WLOGFD("[NAPI]GetMainWindow");
    JsWindowStage* me = CheckParamsAndGetThis<JsWindowStage>(env, info);
    return (me != nullptr) ? me->OnGetMainWindow(env, info) : nullptr;
}

napi_value JsWindowStage::GetMainWindowSync(napi_env env, napi_callback_info info)
{
    WLOGFD("[NAPI]GetMainWindowSync");
    JsWindowStage* me = CheckParamsAndGetThis<JsWindowStage>(env, info);
    return (me != nullptr) ? me->OnGetMainWindowSync(env, info) : nullptr;
}

napi_value JsWindowStage::On(napi_env env, napi_callback_info info)
{
    WLOGFD("[NAPI]On");
    JsWindowStage* me = CheckParamsAndGetThis<JsWindowStage>(env, info);
    return (me != nullptr) ? me->OnEvent(env, info) : nullptr;
}

napi_value JsWindowStage::Off(napi_env env, napi_callback_info info)
{
    WLOGFD("[NAPI]Off");
    JsWindowStage* me = CheckParamsAndGetThis<JsWindowStage>(env, info);
    return (me != nullptr) ? me->OffEvent(env, info) : nullptr;
}

napi_value JsWindowStage::LoadContent(napi_env env, napi_callback_info info)
{
    WLOGFI("[NAPI]");
    JsWindowStage* me = CheckParamsAndGetThis<JsWindowStage>(env, info);
    return (me != nullptr) ? me->OnLoadContent(env, info, false) : nullptr;
}

napi_value JsWindowStage::LoadContentByName(napi_env env, napi_callback_info info)
{
    WLOGFI("[NAPI]");
    JsWindowStage* me = CheckParamsAndGetThis<JsWindowStage>(env, info);
    return (me != nullptr) ? me->OnLoadContent(env, info, true) : nullptr;
}

napi_value JsWindowStage::GetWindowMode(napi_env env, napi_callback_info info)
{
    WLOGFD("[NAPI]GetWindowMode");
    JsWindowStage* me = CheckParamsAndGetThis<JsWindowStage>(env, info);
    return (me != nullptr) ? me->OnGetWindowMode(env, info) : nullptr;
}

napi_value JsWindowStage::CreateSubWindow(napi_env env, napi_callback_info info)
{
    WLOGFD("[NAPI]CreateSubWindow");
    JsWindowStage* me = CheckParamsAndGetThis<JsWindowStage>(env, info);
    return (me != nullptr) ? me->OnCreateSubWindow(env, info) : nullptr;
}

napi_value JsWindowStage::CreateSubWindowWithOptions(napi_env env, napi_callback_info info)
{
    WLOGFD("[NAPI]CreateSubWindowWithOptions");
    JsWindowStage* me = CheckParamsAndGetThis<JsWindowStage>(env, info);
    return (me != nullptr) ? me->OnCreateSubWindowWithOptions(env, info) : nullptr;
}

napi_value JsWindowStage::GetSubWindow(napi_env env, napi_callback_info info)
{
    WLOGFD("[NAPI]GetSubWindow");
    JsWindowStage* me = CheckParamsAndGetThis<JsWindowStage>(env, info);
    return (me != nullptr) ? me->OnGetSubWindow(env, info) : nullptr;
}

napi_value JsWindowStage::SetWindowModal(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::WMS_MAIN, "[NAPI]");
    JsWindowStage* me = CheckParamsAndGetThis<JsWindowStage>(env, info);
    return (me != nullptr) ? me->OnSetWindowModal(env, info) : nullptr;
}

/** @note @window.hierarchy */
napi_value JsWindowStage::SetShowOnLockScreen(napi_env env, napi_callback_info info)
{
    WLOGFD("[NAPI]SetShowOnLockScreen");
    JsWindowStage* me = CheckParamsAndGetThis<JsWindowStage>(env, info);
    return (me != nullptr) ? me->OnSetShowOnLockScreen(env, info) : nullptr;
}

napi_value JsWindowStage::DisableWindowDecor(napi_env env, napi_callback_info info)
{
    WLOGFD("[NAPI]DisableWindowDecor");
    JsWindowStage* me = CheckParamsAndGetThis<JsWindowStage>(env, info);
    return (me != nullptr) ? me->OnDisableWindowDecor(env, info) : nullptr;
}

napi_value JsWindowStage::SetDefaultDensityEnabled(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::WMS_LAYOUT, "SetDefaultDensityEnabled");
    JsWindowStage* me = CheckParamsAndGetThis<JsWindowStage>(env, info);
    return (me != nullptr) ? me->OnSetDefaultDensityEnabled(env, info) : nullptr;
}

napi_value JsWindowStage::SetCustomDensity(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::WMS_ATTRIBUTE, "[NAPI]");
    JsWindowStage* me = CheckParamsAndGetThis<JsWindowStage>(env, info);
    return (me != nullptr) ? me->OnSetCustomDensity(env, info) : nullptr;
}

napi_value JsWindowStage::RemoveStartingWindow(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::WMS_MAIN, "[NAPI]");
    JsWindowStage* me = CheckParamsAndGetThis<JsWindowStage>(env, info);
    return (me != nullptr) ? me->OnRemoveStartingWindow(env, info) : nullptr;
}

napi_value JsWindowStage::SetWindowRectAutoSave(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::WMS_MAIN, "[NAPI]");
    JsWindowStage* me = CheckParamsAndGetThis<JsWindowStage>(env, info);
    return (me != nullptr) ? me->OnSetWindowRectAutoSave(env, info) : nullptr;
}

napi_value JsWindowStage::IsWindowRectAutoSave(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::WMS_MAIN, "[NAPI]");
    JsWindowStage* me = CheckParamsAndGetThis<JsWindowStage>(env, info);
    return (me != nullptr) ? me->OnIsWindowRectAutoSave(env, info) : nullptr;
}

napi_value JsWindowStage::OnSetUIContent(napi_env env, napi_callback_info info)
{
    size_t argc = 4;
    napi_value argv[4] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc < 2) { // 2: minimum param num
        WLOGFE("[NAPI]Argc is invalid: %{public}zu", argc);
        return NapiGetUndefined(env);
    }

    // Parse info->argv[0] as abilitycontext
    auto objContext = argv[0];
    if (objContext == nullptr) {
        WLOGFE("[NAPI]Context is nullptr");
        return NapiGetUndefined(env);
    }

    // Parse info->argv[1] as url
    std::string contextUrl;
    if (!ConvertFromJsValue(env, argv[1], contextUrl)) {
        WLOGFE("[NAPI]Failed to convert parameter to url");
        return NapiGetUndefined(env);
    }

    auto weakScene = windowScene_.lock();
    if (weakScene == nullptr || weakScene->GetMainWindow() == nullptr) {
        WLOGFE("[NAPI]WindowScene is null or window is null");
        return NapiGetUndefined(env);
    }
    weakScene->GetMainWindow()->NapiSetUIContent(contextUrl, env, argv[CONTENT_STORAGE_ARG]);
    return NapiGetUndefined(env);
}

napi_value JsWindowStage::OnGetMainWindow(napi_env env, napi_callback_info info)
{
    NapiAsyncTask::CompleteCallback complete =
        [weak = windowScene_](napi_env env, NapiAsyncTask& task, int32_t status) {
            auto weakScene = weak.lock();
            if (weakScene == nullptr) {
                task.Reject(env, JsErrUtils::CreateJsError(env, WmErrorCode::WM_ERROR_STAGE_ABNORMALLY));
                WLOGFE("[NAPI]WindowScene_ is nullptr!");
                return;
            }
            auto window = weakScene->GetMainWindow();
            if (window != nullptr) {
                task.Resolve(env, OHOS::Rosen::CreateJsWindowObject(env, window));
                WLOGI("[NAPI]Get main window [%{public}u, %{public}s]",
                    window->GetWindowId(), window->GetWindowName().c_str());
            } else {
                task.Reject(env, JsErrUtils::CreateJsError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
                    "Get main window failed."));
            }
        };
    size_t argc = 4;
    napi_value argv[4] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    napi_value callback = (argv[0] != nullptr && GetType(env, argv[0]) == napi_function) ?
        argv[0] : nullptr;
    napi_value result = nullptr;
    NapiAsyncTask::Schedule("JsWindowStage::OnGetMainWindow",
        env, CreateAsyncTaskWithLastParam(env, callback, nullptr, std::move(complete), &result));
    return result;
}

napi_value JsWindowStage::OnGetMainWindowSync(napi_env env, napi_callback_info info)
{
    auto weakScene = windowScene_.lock();
    if (weakScene == nullptr) {
        WLOGFE("[NAPI]WindowScene is null");
        napi_throw(env, JsErrUtils::CreateJsError(env, WmErrorCode::WM_ERROR_STAGE_ABNORMALLY));
        return NapiGetUndefined(env);
    }
    auto window = weakScene->GetMainWindow();
    if (window == nullptr) {
        WLOGFE("[NAPI]window is null");
        napi_throw(env, JsErrUtils::CreateJsError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY));
        return NapiGetUndefined(env);
    }

    return OHOS::Rosen::CreateJsWindowObject(env, window);
}

napi_value JsWindowStage::OnEvent(napi_env env, napi_callback_info info)
{
    auto weakScene = windowScene_.lock();
    if (weakScene == nullptr) {
        WLOGFE("[NAPI]Window scene is null");
        napi_throw(env, JsErrUtils::CreateJsError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY));
        return NapiGetUndefined(env);
    }
    size_t argc = 4;
    napi_value argv[4] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc < 2) { // 2: minimum param nums
        WLOGFE("[NAPI]argc is invalid: %{public}zu", argc);
        napi_throw(env, JsErrUtils::CreateJsError(env, WmErrorCode::WM_ERROR_INVALID_PARAM));
        return NapiGetUndefined(env);
    }

    // Parse argv[0] as string
    std::string eventString;
    if (!ConvertFromJsValue(env, argv[0], eventString)) {
        WLOGFE("[NAPI]Failed to convert parameter to string");
        napi_throw(env, JsErrUtils::CreateJsError(env, WmErrorCode::WM_ERROR_INVALID_PARAM));
        return NapiGetUndefined(env);
    }
    napi_value value = argv[1];
    if (!NapiIsCallable(env, value)) {
        WLOGFE("[NAPI]Callback(argv[1]) is not callable");
        napi_throw(env, JsErrUtils::CreateJsError(env, WmErrorCode::WM_ERROR_INVALID_PARAM));
        return NapiGetUndefined(env);
    }

    auto window = weakScene->GetMainWindow();
    if (window == nullptr) {
        WLOGFE("[NAPI]Get window failed");
        napi_throw(env, JsErrUtils::CreateJsError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY));
        return NapiGetUndefined(env);
    }
    auto ret = g_listenerManager->RegisterListener(window, eventString, CaseType::CASE_STAGE, env, value);
    if (ret != WmErrorCode::WM_OK) {
        TLOGE(WmsLogTag::DEFAULT, "register event %{public}s failed, ret = %{public}d", eventString.c_str(), ret);
        napi_throw(env, JsErrUtils::CreateJsError(env, ret));
        return NapiGetUndefined(env);
    }
    WLOGI("[NAPI]Window [%{public}u, %{public}s] register event %{public}s",
        window->GetWindowId(), window->GetWindowName().c_str(), eventString.c_str());

    return NapiGetUndefined(env);
}

napi_value JsWindowStage::OffEvent(napi_env env, napi_callback_info info)
{
    auto weakScene = windowScene_.lock();
    if (weakScene == nullptr) {
        WLOGFE("[NAPI]Window scene is null");
        napi_throw(env, JsErrUtils::CreateJsError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY));
        return NapiGetUndefined(env);
    }
    size_t argc = 4;
    napi_value argv[4] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    // Parse argv[0] as string
    std::string eventString;
    if (!ConvertFromJsValue(env, argv[0], eventString)) {
        WLOGFE("[NAPI]Failed to convert parameter to string");
        napi_throw(env, JsErrUtils::CreateJsError(env, WmErrorCode::WM_ERROR_INVALID_PARAM));
        return NapiGetUndefined(env);
    }

    auto window = weakScene->GetMainWindow();
    if (window == nullptr) {
        WLOGFE("[NAPI]Get window failed");
        napi_throw(env, JsErrUtils::CreateJsError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY));
        return NapiGetUndefined(env);
    }
    napi_value value = nullptr;
    WmErrorCode ret = WmErrorCode::WM_OK;
    if (argc == 1) {
        ret = g_listenerManager->UnregisterListener(window, eventString, CaseType::CASE_STAGE, env, nullptr);
    } else {
        value = argv[1];
        if (value != nullptr && GetType(env, value) == napi_function) {
            ret = g_listenerManager->UnregisterListener(window, eventString, CaseType::CASE_STAGE, env, value);
        } else {
            ret = g_listenerManager->UnregisterListener(window, eventString, CaseType::CASE_STAGE, env, nullptr);
        }
    }
    if (ret != WmErrorCode::WM_OK) {
        TLOGE(WmsLogTag::DEFAULT, "unregister event %{public}s failed, ret = %{public}d", eventString.c_str(), ret);
        napi_throw(env, JsErrUtils::CreateJsError(env, ret));
        return NapiGetUndefined(env);
    }
    WLOGI("[NAPI]Window [%{public}u, %{public}s] unregister event %{public}s",
        window->GetWindowId(), window->GetWindowName().c_str(), eventString.c_str());

    return NapiGetUndefined(env);
}

static void LoadContentTask(std::shared_ptr<NativeReference> contentStorage, std::string contextUrl,
    sptr<Window> weakWindow, napi_env env, NapiAsyncTask& task, bool isLoadedByName)
{
    napi_value nativeStorage = (contentStorage == nullptr) ? nullptr : contentStorage->GetNapiValue();
    WMError ret;
    if (isLoadedByName) {
        ret = weakWindow->SetUIContentByName(contextUrl, env, nativeStorage);
    } else {
        ret = weakWindow->NapiSetUIContent(contextUrl, env, nativeStorage);
    }
    if (ret == WMError::WM_OK) {
        task.Resolve(env, NapiGetUndefined(env));
    } else {
        task.Reject(env, JsErrUtils::CreateJsError(env, WM_JS_TO_ERROR_CODE_MAP.at(ret),
            "Window load content failed"));
    }
    WLOGI("[NAPI]Window [%{public}u, %{public}s] load content end, ret = %{public}d",
        weakWindow->GetWindowId(), weakWindow->GetWindowName().c_str(), ret);
    return;
}

napi_value JsWindowStage::OnLoadContent(napi_env env, napi_callback_info info, bool isLoadedByName)
{
    WmErrorCode errCode = WmErrorCode::WM_OK;
    std::string contextUrl;
    size_t argc = 4;
    napi_value argv[4] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (!ConvertFromJsValue(env, argv[0], contextUrl)) {
        WLOGFE("[NAPI]Failed to convert parameter to context url");
        errCode = WmErrorCode::WM_ERROR_INVALID_PARAM;
    }
    napi_value storage = nullptr;
    napi_value callBack = nullptr;
    napi_value value1 = argv[1];
    napi_value value2 = argv[2]; // 2: param index
    if (GetType(env, value1) == napi_function) {
        callBack = value1;
    } else if (GetType(env, value1) == napi_object) {
        storage = value1;
    }
    if (GetType(env, value2) == napi_function) {
        callBack = value2;
    }
    if (errCode == WmErrorCode::WM_ERROR_INVALID_PARAM) {
        WLOGFE("[NAPI]Window scene is null or get invalid param");
        napi_throw(env, JsErrUtils::CreateJsError(env, WmErrorCode::WM_ERROR_INVALID_PARAM));
        return NapiGetUndefined(env);
    }

    std::shared_ptr<NativeReference> contentStorage = nullptr;
    if (storage != nullptr) {
        napi_ref result = nullptr;
        napi_create_reference(env, storage, 1, &result);
        contentStorage = std::shared_ptr<NativeReference>(reinterpret_cast<NativeReference*>(result));
    }

    NapiAsyncTask::CompleteCallback complete =
        [weak = windowScene_, contentStorage, contextUrl, isLoadedByName](
            napi_env env, NapiAsyncTask& task, int32_t status) {
            auto weakScene = weak.lock();
            sptr<Window> win = weakScene ? weakScene->GetMainWindow() : nullptr;
            if (win == nullptr) {
                task.Reject(env, JsErrUtils::CreateJsError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY));
                WLOGFE("[NAPI]Get window failed");
                return;
            }
            LoadContentTask(contentStorage, contextUrl, win, env, task, isLoadedByName);
        };
    napi_value result = nullptr;
    NapiAsyncTask::Schedule("JsWindowStage::OnLoadContent",
        env, CreateAsyncTaskWithLastParam(env, callBack, nullptr, std::move(complete), &result));
    return result;
}

napi_value JsWindowStage::OnGetWindowMode(napi_env env, napi_callback_info info)
{
    NapiAsyncTask::CompleteCallback complete =
        [weak = windowScene_](napi_env env, NapiAsyncTask& task, int32_t status) {
            auto weakScene = weak.lock();
            if (weakScene == nullptr) {
                task.Reject(env, JsErrUtils::CreateJsError(env, WMError::WM_ERROR_NULLPTR));
                WLOGFE("[NAPI]windowScene_ is nullptr");
                return;
            }
            auto window = weakScene->GetMainWindow();
            if (window == nullptr) {
                task.Reject(env, JsErrUtils::CreateJsError(env, WMError::WM_ERROR_NULLPTR, "Get window failed"));
                WLOGFE("[NAPI]Get window failed");
                return;
            }
            Rosen::WindowMode mode = window->GetMode();
            if (NATIVE_TO_JS_WINDOW_MODE_MAP.count(mode) != 0) {
                task.Resolve(env, CreateJsValue(env, NATIVE_TO_JS_WINDOW_MODE_MAP.at(mode)));
                WLOGI("[NAPI]Window [%{public}u, %{public}s] get mode %{public}u, api mode %{public}u",
                    window->GetWindowId(), window->GetWindowName().c_str(),
                    mode, NATIVE_TO_JS_WINDOW_MODE_MAP.at(mode));
            } else {
                task.Resolve(env, CreateJsValue(env, mode));
                WLOGFE("[NAPI]Get mode %{public}u, but not in apimode", mode);
            }
        };

    size_t argc = 4;
    napi_value argv[4] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    napi_value callback = (GetType(env, argv[0]) == napi_function) ? argv[0] : nullptr;
    napi_value result = nullptr;
    NapiAsyncTask::Schedule("JsWindowStage::OnGetWindowMode",
        env, CreateAsyncTaskWithLastParam(env, callback, nullptr, std::move(complete), &result));
    return result;
}

napi_value JsWindowStage::OnCreateSubWindow(napi_env env, napi_callback_info info)
{
    WmErrorCode errCode = WmErrorCode::WM_OK;
    std::string windowName;
    size_t argc = 4;
    napi_value argv[4] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (!ConvertFromJsValue(env, argv[0], windowName)) {
        WLOGFE("[NAPI]Failed to convert parameter to windowName");
        errCode = WmErrorCode::WM_ERROR_INVALID_PARAM;
    }
    if (errCode == WmErrorCode::WM_ERROR_INVALID_PARAM) {
        WLOGFE("[NAPI]get invalid param");
        napi_throw(env, JsErrUtils::CreateJsError(env, WmErrorCode::WM_ERROR_INVALID_PARAM));
        return NapiGetUndefined(env);
    }
    NapiAsyncTask::CompleteCallback complete =
        [weak = windowScene_, windowName](napi_env env, NapiAsyncTask& task, int32_t status) {
            auto weakScene = weak.lock();
            if (weakScene == nullptr) {
                WLOGFE("[NAPI]Window scene is null");
                task.Reject(env, JsErrUtils::CreateJsError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY));
                return;
            }
            sptr<Rosen::WindowOption> windowOption = new Rosen::WindowOption();
            windowOption->SetWindowType(Rosen::WindowType::WINDOW_TYPE_APP_SUB_WINDOW);
            windowOption->SetWindowMode(Rosen::WindowMode::WINDOW_MODE_FLOATING);
            auto window = weakScene->CreateWindow(windowName, windowOption);
            if (window == nullptr) {
                WLOGFE("[NAPI]Get window failed");
                task.Reject(env, JsErrUtils::CreateJsError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
                    "Get window failed"));
                return;
            }
            task.Resolve(env, CreateJsWindowObject(env, window));
            WLOGI("[NAPI]Create sub window %{public}s end", windowName.c_str());
        };
    napi_value callback = (argv[1] != nullptr && GetType(env, argv[1]) == napi_function) ? argv[1] : nullptr;
    napi_value result = nullptr;
    NapiAsyncTask::Schedule("JsWindowStage::OnCreateSubWindow",
        env, CreateAsyncTaskWithLastParam(env, callback, nullptr, std::move(complete), &result));
    return result;
}

static napi_value CreateJsSubWindowArrayObject(napi_env env,
    std::vector<sptr<Window>> subWinVec)
{
    napi_value arrayValue = nullptr;
    napi_create_array_with_length(env, subWinVec.size(), &arrayValue);
    if (arrayValue == nullptr) {
        WLOGFE("[NAPI]Failed to convert subWinVec to jsArrayObject");
        return nullptr;
    }
    uint32_t index = 0;
    for (size_t i = 0; i < subWinVec.size(); i++) {
        napi_set_element(env, arrayValue, index++, CreateJsWindowObject(env, subWinVec[i]));
    }
    return arrayValue;
}

napi_value JsWindowStage::OnGetSubWindow(napi_env env, napi_callback_info info)
{
    NapiAsyncTask::CompleteCallback complete =
        [weak = windowScene_](napi_env env, NapiAsyncTask& task, int32_t status) {
            auto weakScene = weak.lock();
            if (weakScene == nullptr) {
                WLOGFE("[NAPI]Window scene is nullptr");
                task.Reject(env, JsErrUtils::CreateJsError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY));
                return;
            }
            std::vector<sptr<Window>> subWindowVec = weakScene->GetSubWindow();
            task.Resolve(env, CreateJsSubWindowArrayObject(env, subWindowVec));
            WLOGI("[NAPI]Get sub windows, size = %{public}zu", subWindowVec.size());
        };
    size_t argc = 4;
    napi_value argv[4] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    napi_value callback = (argv[0] != nullptr && GetType(env, argv[0]) == napi_function) ? argv[0] : nullptr;
    napi_value result = nullptr;
    NapiAsyncTask::Schedule("JsWindowStage::OnGetSubWindow",
        env, CreateAsyncTaskWithLastParam(env, callback, nullptr, std::move(complete), &result));
    return result;
}

napi_value JsWindowStage::OnSetWindowModal(napi_env env, napi_callback_info info)
{
    auto windowScene = windowScene_.lock();
    if (windowScene == nullptr) {
        TLOGE(WmsLogTag::WMS_MAIN, "WindowScene is null");
        napi_throw(env, JsErrUtils::CreateJsError(env, WmErrorCode::WM_ERROR_STAGE_ABNORMALLY));
        return NapiGetUndefined(env);
    }
    auto window = windowScene->GetMainWindow();
    if (window == nullptr) {
        TLOGE(WmsLogTag::WMS_MAIN, "window is nullptr");
        napi_throw(env, JsErrUtils::CreateJsError(env, WmErrorCode::WM_ERROR_STAGE_ABNORMALLY));
        return NapiGetUndefined(env);
    }
    if (!window->IsPcOrPadFreeMultiWindowMode()) {
        TLOGE(WmsLogTag::WMS_MAIN, "device not support");
        napi_throw(env, JsErrUtils::CreateJsError(env, WmErrorCode::WM_ERROR_DEVICE_NOT_SUPPORT));
        return NapiGetUndefined(env);
    }
    size_t argc = FOUR_PARAMS_SIZE;
    napi_value argv[FOUR_PARAMS_SIZE] = { nullptr };
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc != 1) {
        TLOGE(WmsLogTag::WMS_MAIN, "Argc is invalid: %{public}zu", argc);
        napi_throw(env, JsErrUtils::CreateJsError(env, WmErrorCode::WM_ERROR_INVALID_PARAM));
        return NapiGetUndefined(env);
    }
    bool isModal = false;
    if (!ConvertFromJsValue(env, argv[INDEX_ZERO], isModal)) {
        TLOGE(WmsLogTag::WMS_MAIN, "Failed to convert parameter to bool");
        napi_throw(env, JsErrUtils::CreateJsError(env, WmErrorCode::WM_ERROR_INVALID_PARAM));
        return NapiGetUndefined(env);
    }
    napi_value result = nullptr;
    std::shared_ptr<NapiAsyncTask> napiAsyncTask = CreateEmptyAsyncTask(env, nullptr, &result);
    const char* const where = __func__;
    auto asyncTask = [where, weakWindow = wptr(window), isModal, env, task = napiAsyncTask] {
        auto window = weakWindow.promote();
        if (window == nullptr) {
            TLOGNE(WmsLogTag::WMS_MAIN, "%{public}s failed, window is null", where);
            task->Reject(env, JsErrUtils::CreateJsError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY));
            return;
        }
        WMError ret = window->SetWindowModal(isModal);
        if (ret != WMError::WM_OK) {
            WmErrorCode wmErrorCode = WM_JS_TO_ERROR_CODE_MAP.at(ret);
            TLOGNE(WmsLogTag::WMS_MAIN, "%{public}s failed, ret is %{public}d", where, wmErrorCode);
            task->Reject(env, JsErrUtils::CreateJsError(env, wmErrorCode, "Set main window modal failed"));
            return;
        }
        task->Resolve(env, NapiGetUndefined(env));
        TLOGNI(WmsLogTag::WMS_MAIN, "%{public}s id:%{public}u, name:%{public}s, isModal:%{public}d",
            where, window->GetWindowId(), window->GetWindowName().c_str(), isModal);
    };
    if (napi_status::napi_ok != napi_send_event(env, asyncTask, napi_eprio_high)) {
        napiAsyncTask->Reject(env,
            CreateJsError(env, static_cast<int32_t>(WmErrorCode::WM_ERROR_STATE_ABNORMALLY), "send event failed"));
    }
    return result;
}

napi_value JsWindowStage::OnSetShowOnLockScreen(napi_env env, napi_callback_info info)
{
    if (!Permission::IsSystemCalling() && !Permission::IsStartByHdcd()) {
        WLOGFE("set show on lock screen permission denied!");
        napi_throw(env, JsErrUtils::CreateJsError(env, WmErrorCode::WM_ERROR_NOT_SYSTEM_APP));
        return CreateJsValue(env, static_cast<int32_t>(WmErrorCode::WM_ERROR_NOT_SYSTEM_APP));
    }

    size_t argc = 4;
    napi_value argv[4] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc < 1) {
        WLOGFE("[NAPI]Argc is invalid: %{public}zu", argc);
        napi_throw(env, JsErrUtils::CreateJsError(env, WmErrorCode::WM_ERROR_INVALID_PARAM));
        return CreateJsValue(env, static_cast<int32_t>(WmErrorCode::WM_ERROR_INVALID_PARAM));
    }
    auto weakScene = windowScene_.lock();
    if (weakScene == nullptr || weakScene->GetMainWindow() == nullptr) {
        WLOGFE("[NAPI]WindowScene is null or window is null");
        napi_throw(env, JsErrUtils::CreateJsError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY));
        return CreateJsValue(env, static_cast<int32_t>(WmErrorCode::WM_ERROR_STATE_ABNORMALLY));
    }

    bool showOnLockScreen = false;
    napi_value nativeVal = argv[0];
    if (nativeVal == nullptr) {
        WLOGFE("[NAPI]Failed to convert parameter to boolean");
        napi_throw(env, JsErrUtils::CreateJsError(env, WmErrorCode::WM_ERROR_INVALID_PARAM));
        return CreateJsValue(env, static_cast<int32_t>(WmErrorCode::WM_ERROR_INVALID_PARAM));
    } else {
        napi_get_value_bool(env, nativeVal, &showOnLockScreen);
    }

    auto window = weakScene->GetMainWindow();
    WmErrorCode ret;
    if (showOnLockScreen) {
        ret = WM_JS_TO_ERROR_CODE_MAP.at(
            window->AddWindowFlag(WindowFlag::WINDOW_FLAG_SHOW_WHEN_LOCKED));
    } else {
        ret = WM_JS_TO_ERROR_CODE_MAP.at(
            window->RemoveWindowFlag(WindowFlag::WINDOW_FLAG_SHOW_WHEN_LOCKED));
    }
    WLOGI("[NAPI]Window [%{public}u, %{public}s] SetShowOnLockScreen %{public}u, ret = %{public}u",
        window->GetWindowId(), window->GetWindowName().c_str(), showOnLockScreen, ret);

    return CreateJsValue(env, static_cast<int32_t>(ret));
}

napi_value JsWindowStage::OnDisableWindowDecor(napi_env env, napi_callback_info info)
{
    auto weakScene = windowScene_.lock();
    if (weakScene == nullptr) {
        WLOGFE("[NAPI]WindowScene is null");
        return CreateJsValue(env, static_cast<int32_t>(WmErrorCode::WM_ERROR_STAGE_ABNORMALLY));
    }

    auto window = weakScene->GetMainWindow();
    if (window == nullptr) {
        WLOGFE("[NAPI]Window is null");
        return CreateJsValue(env, static_cast<int32_t>(WmErrorCode::WM_ERROR_STATE_ABNORMALLY));
    }
    WmErrorCode ret = WM_JS_TO_ERROR_CODE_MAP.at(window->DisableAppWindowDecor());
    if (ret != WmErrorCode::WM_OK) {
        napi_throw(env, JsErrUtils::CreateJsError(env, ret));
    WLOGI("[NAPI]Window [%{public}u, %{public}s] disable app window decor end",
        window->GetWindowId(), window->GetWindowName().c_str());
        return NapiGetUndefined(env);
    }
    return NapiGetUndefined(env);
}

napi_value JsWindowStage::OnSetDefaultDensityEnabled(napi_env env, napi_callback_info info)
{
    size_t argc = 4;
    napi_value argv[4] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc != 1) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "Argc is invalid: %{public}zu", argc);
        napi_throw(env, JsErrUtils::CreateJsError(env, WmErrorCode::WM_ERROR_INVALID_PARAM));
        return CreateJsValue(env, static_cast<int32_t>(WmErrorCode::WM_ERROR_INVALID_PARAM));
    }

    auto weakScene = windowScene_.lock();
    if (weakScene == nullptr) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "WindowScene is null");
        return CreateJsValue(env, static_cast<int32_t>(WmErrorCode::WM_ERROR_STAGE_ABNORMALLY));
    }

    auto window = weakScene->GetMainWindow();
    if (window == nullptr) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "Window is null");
        return CreateJsValue(env, static_cast<int32_t>(WmErrorCode::WM_ERROR_STATE_ABNORMALLY));
    }

    bool enabled = false;
    if (!ConvertFromJsValue(env, argv[0], enabled)) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "Failed to convert parameter to boolean");
        napi_throw(env, JsErrUtils::CreateJsError(env, WmErrorCode::WM_ERROR_INVALID_PARAM));
        return CreateJsValue(env, static_cast<int32_t>(WmErrorCode::WM_ERROR_INVALID_PARAM));
    }

    WmErrorCode ret = WM_JS_TO_ERROR_CODE_MAP.at(window->SetDefaultDensityEnabled(enabled));
    TLOGI(WmsLogTag::WMS_LAYOUT, "Window [%{public}u,%{public}s] SetDefaultDensityEnabled=%{public}u ret=%{public}u",
        window->GetWindowId(), window->GetWindowName().c_str(), enabled, ret);

    return CreateJsValue(env, static_cast<int32_t>(ret));
}

napi_value JsWindowStage::OnSetCustomDensity(napi_env env, napi_callback_info info)
{
    size_t argc = FOUR_PARAMS_SIZE;
    napi_value argv[FOUR_PARAMS_SIZE] = { nullptr };
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc != 1) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "Argc is invalid: %{public}zu", argc);
        return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM);
    }

    double density = UNDEFINED_DENSITY;
    if (!ConvertFromJsValue(env, argv[0], density)) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "Failed to convert parameter to double");
        return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM);
    }

    auto windowScene = windowScene_.lock();
    if (windowScene == nullptr) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "windowScene is null");
        return NapiThrowError(env, WmErrorCode::WM_ERROR_STAGE_ABNORMALLY);
    }
    auto window = windowScene->GetMainWindow();
    if (window == nullptr) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "Window is null");
        return NapiThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
    }

    WmErrorCode ret = WM_JS_TO_ERROR_CODE_MAP.at(window->SetCustomDensity(density));
    TLOGI(WmsLogTag::WMS_ATTRIBUTE, "Window [%{public}u,%{public}s] set density=%{public}f, result=%{public}u",
        window->GetWindowId(), window->GetWindowName().c_str(), density, ret);
    if (ret != WmErrorCode::WM_OK) {
        return NapiThrowError(env, ret);
    }
    return NapiGetUndefined(env);
}

napi_value JsWindowStage::OnCreateSubWindowWithOptions(napi_env env, napi_callback_info info)
{
    auto windowScene = windowScene_.lock();
    if (windowScene == nullptr) {
        TLOGE(WmsLogTag::WMS_SUB, "WindowScene is null");
        napi_throw(env, JsErrUtils::CreateJsError(env, WmErrorCode::WM_ERROR_STAGE_ABNORMALLY));
        return NapiGetUndefined(env);
    }
    size_t argc = 4;
    napi_value argv[4] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    std::string windowName;
    if (!ConvertFromJsValue(env, argv[0], windowName)) {
        WLOGFE("[NAPI]Failed to convert parameter to windowName");
        napi_throw(env, JsErrUtils::CreateJsError(env, WmErrorCode::WM_ERROR_INVALID_PARAM));
        return NapiGetUndefined(env);
    }
    sptr<WindowOption> option = new WindowOption();
    if (!ParseSubWindowOptions(env, argv[1], option)) {
        WLOGFE("[NAPI]get invalid options param");
        napi_throw(env, JsErrUtils::CreateJsError(env, WmErrorCode::WM_ERROR_INVALID_PARAM));
        return NapiGetUndefined(env);
    }
    if ((option->GetWindowFlags() & static_cast<uint32_t>(WindowFlag::WINDOW_FLAG_IS_APPLICATION_MODAL)) &&
        !windowScene->GetMainWindow()->IsPcOrPadFreeMultiWindowMode()) {
        TLOGE(WmsLogTag::WMS_SUB, "device not support");
        napi_throw(env, JsErrUtils::CreateJsError(env, WmErrorCode::WM_ERROR_DEVICE_NOT_SUPPORT));
        return NapiGetUndefined(env);
    }

    if (option->GetWindowTopmost() && !Permission::IsSystemCalling() && !Permission::IsStartByHdcd()) {
        TLOGE(WmsLogTag::WMS_SUB, "Modal subwindow has topmost, but no system permission");
        napi_throw(env, JsErrUtils::CreateJsError(env, WmErrorCode::WM_ERROR_NOT_SYSTEM_APP));
        return NapiGetUndefined(env);
    }

    const char* const where = __func__;
    NapiAsyncTask::CompleteCallback complete =
        [where, windowScene, windowName = std::move(windowName), option]
            (napi_env env, NapiAsyncTask& task, int32_t status) mutable {
        option->SetWindowType(WindowType::WINDOW_TYPE_APP_SUB_WINDOW);
        option->SetWindowMode(WindowMode::WINDOW_MODE_FLOATING);
        option->SetOnlySupportSceneBoard(true);
        auto window = windowScene->CreateWindow(windowName, option);
        if (window == nullptr) {
            TLOGNE(WmsLogTag::WMS_SUB, "%{public}s [NAPI]Get window failed", where);
            task.Reject(env, JsErrUtils::CreateJsError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
                "Get window failed"));
            return;
        }
        task.Resolve(env, CreateJsWindowObject(env, window));
        TLOGNI(WmsLogTag::WMS_SUB, "%{public}s [NAPI]Create sub window %{public}s end",
            where, windowName.c_str());
    };
    napi_value callback = (argv[2] != nullptr && GetType(env, argv[2]) == napi_function) ? argv[2] : nullptr;
    napi_value result = nullptr;
    NapiAsyncTask::Schedule("JsWindowStage::OnCreateSubWindowWithOptions",
        env, CreateAsyncTaskWithLastParam(env, callback, nullptr, std::move(complete), &result));
    return result;
}

napi_value JsWindowStage::OnRemoveStartingWindow(napi_env env, napi_callback_info info)
{
    auto windowScene = windowScene_.lock();
    if (windowScene == nullptr) {
        TLOGE(WmsLogTag::WMS_MAIN, "windowScene is null");
        napi_throw(env, JsErrUtils::CreateJsError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY));
        return NapiGetUndefined(env);
    }

    const char* const where = __func__;
    NapiAsyncTask::CompleteCallback complete =
        [where, windowScene](napi_env env, NapiAsyncTask& task, int32_t status) {
        auto window = windowScene->GetMainWindow();
        if (window == nullptr) {
            TLOGNE(WmsLogTag::WMS_MAIN, "%{public}s [NAPI]Get main window failed", where);
            task.Reject(env,
                JsErrUtils::CreateJsError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY, "Get main window failed"));
            return;
        }
        WmErrorCode ret = WM_JS_TO_ERROR_CODE_MAP.at(window->NotifyRemoveStartingWindow());
        if (ret == WmErrorCode::WM_OK) {
            task.Resolve(env, NapiGetUndefined(env));
        } else {
            task.Reject(env, JsErrUtils::CreateJsError(env, ret, "Notify remove starting window failed"));
        }
    };

    size_t argc = FOUR_PARAMS_SIZE;
    napi_value argv[FOUR_PARAMS_SIZE] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    napi_value lastParam = (argc == 0) ? nullptr :
        (argv[INDEX_ZERO] != nullptr && GetType(env, argv[INDEX_ZERO]) == napi_function ? argv[INDEX_ZERO] : nullptr);
    napi_value result = nullptr;
    NapiAsyncTask::Schedule("JsWindow::OnRemoveStartingWindow",
        env, CreateAsyncTaskWithLastParam(env, lastParam, nullptr, std::move(complete), &result));
    return result;
}

napi_value JsWindowStage::OnSetWindowRectAutoSave(napi_env env, napi_callback_info info)
{
    auto windowScene = windowScene_.lock();
    if (windowScene == nullptr) {
        TLOGE(WmsLogTag::WMS_MAIN, "WindowScene is null");
        napi_throw(env, JsErrUtils::CreateJsError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY));
        return NapiGetUndefined(env);
    }
 
    size_t argc = FOUR_PARAMS_SIZE;
    napi_value argv[FOUR_PARAMS_SIZE] = { nullptr };
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc != 1) { // 1: maximum params num
        TLOGE(WmsLogTag::WMS_MAIN, "Argc is invalid: %{public}zu", argc);
        napi_throw(env, JsErrUtils::CreateJsError(env, WmErrorCode::WM_ERROR_INVALID_PARAM));
        return NapiGetUndefined(env);
    }
    bool enabled = false;
    if (!ConvertFromJsValue(env, argv[INDEX_ZERO], enabled)) {
        TLOGE(WmsLogTag::WMS_MAIN, "[NAPI]Failed to convert parameter to enabled");
        napi_throw(env, JsErrUtils::CreateJsError(env, WmErrorCode::WM_ERROR_INVALID_PARAM));
        return NapiGetUndefined(env);
    }

    auto window = windowScene->GetMainWindow();
    const char* const where = __func__;
    napi_value result = nullptr;
    std::shared_ptr<NapiAsyncTask> napiAsyncTask = CreateEmptyAsyncTask(env, nullptr, &result);
    auto asyncTask = [weakWindow = wptr(window), where, env, task = napiAsyncTask, enabled] {
        auto window = weakWindow.promote();
        if (window == nullptr) {
            TLOGNE(WmsLogTag::WMS_MAIN, "%{public}s window is nullptr", where);
            WmErrorCode wmErroeCode = WM_JS_TO_ERROR_CODE_MAP.at(WMError::WM_ERROR_NULLPTR);
            task->Reject(env, JsErrUtils::CreateJsError(env, wmErroeCode, "window is nullptr."));
            return;
        }
        WmErrorCode ret = WM_JS_TO_ERROR_CODE_MAP.at(window->SetWindowRectAutoSave(enabled));
        if (ret != WmErrorCode::WM_OK) {
            TLOGNE(WmsLogTag::WMS_MAIN, "%{public}s enable recover position failed!", where);
            task->Reject(env, JsErrUtils::CreateJsError(env,
                ret, "window recover position failed."));
        } else {
            task->Resolve(env, NapiGetUndefined(env));
        }
    };
    if (napi_status::napi_ok != napi_send_event(env, asyncTask, napi_eprio_high)) {
        napiAsyncTask->Reject(env,
            CreateJsError(env, static_cast<int32_t>(WmErrorCode::WM_ERROR_STATE_ABNORMALLY), "send event failed"));
    }
    return result;
}

napi_value JsWindowStage::OnIsWindowRectAutoSave(napi_env env, napi_callback_info info)
{
    auto windowScene = windowScene_.lock();
    if (windowScene == nullptr) {
        TLOGE(WmsLogTag::WMS_MAIN, "WindowScene is null");
        napi_throw(env, JsErrUtils::CreateJsError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY));
        return NapiGetUndefined(env);
    }

    auto window = windowScene->GetMainWindow();
    const char* const where = __func__;
    napi_value result = nullptr;
    std::shared_ptr<NapiAsyncTask> napiAsyncTask = CreateEmptyAsyncTask(env, nullptr, &result);
    auto asyncTask = [weakWindow = wptr(window), where, env, task = napiAsyncTask] {
        auto window = weakWindow.promote();
        if (window == nullptr) {
            TLOGNE(WmsLogTag::WMS_MAIN, "%{public}s Window is nullptr", where);
            task->Reject(env,
                JsErrUtils::CreateJsError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY, "Window is nullptr."));
            return;
        }
        bool enabled = false;
        WmErrorCode ret = WM_JS_TO_ERROR_CODE_MAP.at(window->IsWindowRectAutoSave(enabled));
        if (ret != WmErrorCode::WM_OK) {
            TLOGNE(WmsLogTag::WMS_MAIN, "%{public}s get the auto-save state of the window rect failed!", where);
            task->Reject(env, JsErrUtils::CreateJsError(env,
                ret, "Window recover position failed."));
        } else {
            napi_value jsEnabled = CreateJsValue(env, enabled);
            task->Resolve(env, jsEnabled);
        }
    };
    if (napi_status::napi_ok != napi_send_event(env, asyncTask, napi_eprio_high)) {
        napiAsyncTask->Reject(env,
            CreateJsError(env, static_cast<int32_t>(WmErrorCode::WM_ERROR_STATE_ABNORMALLY), "send event failed"));
    }
    return result;
}

napi_value CreateJsWindowStage(napi_env env, std::shared_ptr<Rosen::WindowScene> windowScene)
{
    WLOGFD("[NAPI]CreateJsWindowStage");
    napi_value objValue = nullptr;
    napi_create_object(env, &objValue);

    std::unique_ptr<JsWindowStage> jsWindowStage = std::make_unique<JsWindowStage>(windowScene);
    napi_wrap(env, objValue, jsWindowStage.release(), JsWindowStage::Finalizer, nullptr, nullptr);

    const char* moduleName = "JsWindowStage";
    BindNativeFunction(env,
        objValue, "setUIContent", moduleName, JsWindowStage::SetUIContent);
    BindNativeFunction(env,
        objValue, "loadContent", moduleName, JsWindowStage::LoadContent);
    BindNativeFunction(env,
        objValue, "loadContentByName", moduleName, JsWindowStage::LoadContentByName);
    BindNativeFunction(env,
        objValue, "getMainWindow", moduleName, JsWindowStage::GetMainWindow);
    BindNativeFunction(env,
        objValue, "getMainWindowSync", moduleName, JsWindowStage::GetMainWindowSync);
    BindNativeFunction(env,
        objValue, "getWindowMode", moduleName, JsWindowStage::GetWindowMode);
    BindNativeFunction(env,
        objValue, "createSubWindow", moduleName, JsWindowStage::CreateSubWindow);
    BindNativeFunction(env,
        objValue, "createSubWindowWithOptions", moduleName, JsWindowStage::CreateSubWindowWithOptions);
    BindNativeFunction(env,
        objValue, "getSubWindow", moduleName, JsWindowStage::GetSubWindow);
    BindNativeFunction(env, objValue, "setWindowModal", moduleName, JsWindowStage::SetWindowModal);
    BindNativeFunction(env, objValue, "on", moduleName, JsWindowStage::On);
    BindNativeFunction(env, objValue, "off", moduleName, JsWindowStage::Off);
    BindNativeFunction(env,
        objValue, "setShowOnLockScreen", moduleName, JsWindowStage::SetShowOnLockScreen);
    BindNativeFunction(env,
        objValue, "disableWindowDecor", moduleName, JsWindowStage::DisableWindowDecor);
    BindNativeFunction(env,
        objValue, "setDefaultDensityEnabled", moduleName, JsWindowStage::SetDefaultDensityEnabled);
    BindNativeFunction(env,
        objValue, "setCustomDensity", moduleName, JsWindowStage::SetCustomDensity);
    BindNativeFunction(env,
        objValue, "removeStartingWindow", moduleName, JsWindowStage::RemoveStartingWindow);
    BindNativeFunction(env,
        objValue, "setWindowRectAutoSave", moduleName, JsWindowStage::SetWindowRectAutoSave);
    BindNativeFunction(env,
        objValue, "isWindowRectAutoSave", moduleName, JsWindowStage::IsWindowRectAutoSave);
    return objValue;
}
}  // namespace Rosen
}  // namespace OHOS
