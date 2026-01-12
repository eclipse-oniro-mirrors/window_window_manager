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
#include "pixel_map_napi.h"

namespace OHOS {
namespace Rosen {
// methods of filling the image in recent
enum class ImageFit {
        FILL,
        CONTAIN,
        COVER,
        FIT_WIDTH,
        FIT_HEIGHT,
        NONE,
        SCALE_DOWN,
        TOP_LEFT,
        TOP,
        TOP_RIGHT,
        LEFT,
        CENTER,
        RIGHT,
        BOTTOM_LEFT,
        BOTTOM,
        BOTTOM_RIGHT,
        MATRIX,
};
using namespace AbilityRuntime;
namespace {
const int CONTENT_STORAGE_ARG = 2;
const uint32_t MIN_RESOURCE_ID = 0x1000000;
const uint32_t MAX_RESOURCE_ID = 0xffffffff;
constexpr size_t ARG_COUNT_ONE = 1;
constexpr size_t ARG_COUNT_TWO = 2;
constexpr size_t INDEX_ZERO = 0;
constexpr size_t INDEX_ONE = 1;
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
    WLOGFI("[NAPI]");
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
    WLOGFD("[NAPI]");
    JsWindowStage* me = CheckParamsAndGetThis<JsWindowStage>(env, info);
    return (me != nullptr) ? me->OnGetMainWindow(env, info) : nullptr;
}

napi_value JsWindowStage::GetMainWindowSync(napi_env env, napi_callback_info info)
{
    WLOGFD("[NAPI]");
    JsWindowStage* me = CheckParamsAndGetThis<JsWindowStage>(env, info);
    return (me != nullptr) ? me->OnGetMainWindowSync(env, info) : nullptr;
}

napi_value JsWindowStage::On(napi_env env, napi_callback_info info)
{
    WLOGFD("[NAPI]");
    JsWindowStage* me = CheckParamsAndGetThis<JsWindowStage>(env, info);
    return (me != nullptr) ? me->OnEvent(env, info) : nullptr;
}

napi_value JsWindowStage::Off(napi_env env, napi_callback_info info)
{
    WLOGFD("[NAPI]");
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
    WLOGFD("[NAPI]");
    JsWindowStage* me = CheckParamsAndGetThis<JsWindowStage>(env, info);
    return (me != nullptr) ? me->OnGetWindowMode(env, info) : nullptr;
}

napi_value JsWindowStage::CreateSubWindow(napi_env env, napi_callback_info info)
{
    WLOGFD("[NAPI]");
    JsWindowStage* me = CheckParamsAndGetThis<JsWindowStage>(env, info);
    return (me != nullptr) ? me->OnCreateSubWindow(env, info) : nullptr;
}

napi_value JsWindowStage::CreateSubWindowWithOptions(napi_env env, napi_callback_info info)
{
    WLOGFD("[NAPI]");
    JsWindowStage* me = CheckParamsAndGetThis<JsWindowStage>(env, info);
    return (me != nullptr) ? me->OnCreateSubWindowWithOptions(env, info) : nullptr;
}

napi_value JsWindowStage::GetSubWindow(napi_env env, napi_callback_info info)
{
    WLOGFD("[NAPI]");
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
    WLOGFD("[NAPI]");
    JsWindowStage* me = CheckParamsAndGetThis<JsWindowStage>(env, info);
    return (me != nullptr) ? me->OnSetShowOnLockScreen(env, info) : nullptr;
}

napi_value JsWindowStage::DisableWindowDecor(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::WMS_DECOR, "[NAPI]");
    JsWindowStage* me = CheckParamsAndGetThis<JsWindowStage>(env, info);
    return (me != nullptr) ? me->OnDisableWindowDecor(env, info) : nullptr;
}

napi_value JsWindowStage::SetDefaultDensityEnabled(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::WMS_ATTRIBUTE, "[NAPI]");
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
    TLOGD(WmsLogTag::WMS_STARTUP_PAGE, "[NAPI]");
    JsWindowStage* me = CheckParamsAndGetThis<JsWindowStage>(env, info);
    return (me != nullptr) ? me->OnRemoveStartingWindow(env, info) : nullptr;
}

napi_value JsWindowStage::SetWindowRectAutoSave(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::WMS_LAYOUT_PC, "[NAPI]");
    JsWindowStage* me = CheckParamsAndGetThis<JsWindowStage>(env, info);
    if (me != nullptr) {
        return me->OnSetWindowRectAutoSave(env, info);
    }
    return NapiThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
        "[window][setWindowRectAutoSave]msg: Window is nullptr.");
}

napi_value JsWindowStage::IsWindowRectAutoSave(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::WMS_LAYOUT_PC, "[NAPI]");
    JsWindowStage* me = CheckParamsAndGetThis<JsWindowStage>(env, info);
    if (me != nullptr) {
        return me->OnIsWindowRectAutoSave(env, info);
    }
    return NapiThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
        "[window][isWindowRectAutoSave]msg: Window is nullptr.");
}

napi_value JsWindowStage::SetSupportedWindowModes(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::WMS_LAYOUT_PC, "[NAPI]");
    JsWindowStage* me = CheckParamsAndGetThis<JsWindowStage>(env, info);
    return (me != nullptr) ? me->OnSetSupportedWindowModes(env, info) : nullptr;
}

napi_value JsWindowStage::SetImageForRecent(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::WMS_PATTERN, "[NAPI]");
    JsWindowStage* me = CheckParamsAndGetThis<JsWindowStage>(env, info);
    return (me != nullptr) ? me->OnSetImageForRecent(env, info) : nullptr;
}

napi_value JsWindowStage::RemoveImageForRecent(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::WMS_PATTERN, "[NAPI]");
    JsWindowStage* me = CheckParamsAndGetThis<JsWindowStage>(env, info);
    return (me != nullptr) ? me->OnRemoveImageForRecent(env, info) : nullptr;
}

napi_value JsWindowStage::OnSetUIContent(napi_env env, napi_callback_info info)
{
    size_t argc = 4;
    napi_value argv[4] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc < 2) { // 2: minimum param num
        WLOGFE("Argc is invalid: %{public}zu", argc);
        return NapiGetUndefined(env);
    }

    // Parse info->argv[0] as abilitycontext
    auto objContext = argv[0];
    if (objContext == nullptr) {
        WLOGFE("Context is nullptr");
        return NapiGetUndefined(env);
    }

    // Parse info->argv[1] as url
    std::string contextUrl;
    if (!ConvertFromJsValue(env, argv[1], contextUrl)) {
        WLOGFE("Failed to convert parameter to url");
        return NapiGetUndefined(env);
    }

    auto weakScene = windowScene_.lock();
    if (weakScene == nullptr || weakScene->GetMainWindow() == nullptr) {
        WLOGFE("WindowScene is null or window is null");
        return NapiGetUndefined(env);
    }
    weakScene->GetMainWindow()->NapiSetUIContent(contextUrl, env, argv[CONTENT_STORAGE_ARG]);
    return NapiGetUndefined(env);
}

napi_value JsWindowStage::OnGetMainWindow(napi_env env, napi_callback_info info)
{
    size_t argc = 4;
    napi_value argv[4] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    napi_value callback = (argv[0] != nullptr && GetType(env, argv[0]) == napi_function) ?
        argv[0] : nullptr;
    napi_value result = nullptr;
    std::shared_ptr<NapiAsyncTask> napiAsyncTask = CreateEmptyAsyncTask(env, callback, &result);
    auto asyncTask = [weak = windowScene_, env, task = napiAsyncTask] {
        auto weakScene = weak.lock();
        if (weakScene == nullptr) {
            task->Reject(env, JsErrUtils::CreateJsError(env, WmErrorCode::WM_ERROR_STAGE_ABNORMALLY,
                "[window][getMainWindow]msg: Window scene is nullptr."));
            TLOGNE(WmsLogTag::WMS_LIFE, "WindowScene_ is nullptr!");
            return;
        }
        auto window = weakScene->GetMainWindow();
        if (window != nullptr) {
            task->Resolve(env, OHOS::Rosen::CreateJsWindowObject(env, window));
            TLOGNI(WmsLogTag::WMS_LIFE, "Get main window [%{public}u, %{public}s]",
                window->GetWindowId(), window->GetWindowName().c_str());
        } else {
            task->Reject(env, JsErrUtils::CreateJsError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
                "[window][getMainWindow]msg: Get main window failed."));
            TLOGNE(WmsLogTag::WMS_LIFE, "Get main window failed.");
        }
    };
    if (napi_send_event(env, asyncTask, napi_eprio_high, "OnGetMainWindow") != napi_status::napi_ok) {
        TLOGE(WmsLogTag::WMS_LIFE, "napi send event failed, window state is abnormal");
    }
    return result;
}

napi_value JsWindowStage::OnGetMainWindowSync(napi_env env, napi_callback_info info)
{
    auto weakScene = windowScene_.lock();
    if (weakScene == nullptr) {
        WLOGFE("WindowScene is null");
        napi_throw(env, JsErrUtils::CreateJsError(env, WmErrorCode::WM_ERROR_STAGE_ABNORMALLY,
            "[window][getMainWindowSync]msg: Window scene is nullptr."));
        return NapiGetUndefined(env);
    }
    auto window = weakScene->GetMainWindow();
    if (window == nullptr) {
        WLOGFE("Window is null");
        napi_throw(env, JsErrUtils::CreateJsError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
            "[window][getMainWindowSync]msg: Get main window failed."));
        return NapiGetUndefined(env);
    }

    return OHOS::Rosen::CreateJsWindowObject(env, window);
}

napi_value JsWindowStage::OnEvent(napi_env env, napi_callback_info info)
{
    auto weakScene = windowScene_.lock();
    if (weakScene == nullptr) {
        WLOGFE("Window scene is null");
        napi_throw(env, JsErrUtils::CreateJsError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
            "[window][on]msg: Window scene is nullptr."));
        return NapiGetUndefined(env);
    }
    size_t argc = 4;
    napi_value argv[4] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc < 2) { // 2: minimum param nums
        WLOGFE("Argc is invalid: %{public}zu", argc);
        napi_throw(env, JsErrUtils::CreateJsError(env, WmErrorCode::WM_ERROR_INVALID_PARAM));
        return NapiGetUndefined(env);
    }

    // Parse argv[0] as string
    std::string eventString;
    if (!ConvertFromJsValue(env, argv[0], eventString)) {
        WLOGFE("Failed to convert parameter to string");
        napi_throw(env, JsErrUtils::CreateJsError(env, WmErrorCode::WM_ERROR_INVALID_PARAM));
        return NapiGetUndefined(env);
    }
    napi_value value = argv[1];
    if (!NapiIsCallable(env, value)) {
        WLOGFE("Callback(argv[1]) is not callable");
        napi_throw(env, JsErrUtils::CreateJsError(env, WmErrorCode::WM_ERROR_INVALID_PARAM));
        return NapiGetUndefined(env);
    }

    auto window = weakScene->GetMainWindow();
    if (window == nullptr) {
        WLOGFE("Get window failed");
        napi_throw(env, JsErrUtils::CreateJsError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
            "[window][on]msg: Get window failed."));
        return NapiGetUndefined(env);
    }
    auto ret = g_listenerManager->RegisterListener(window, eventString, CaseType::CASE_STAGE, env, value);
    if (ret != WmErrorCode::WM_OK) {
        TLOGE(WmsLogTag::DEFAULT, "register event %{public}s failed, ret=%{public}d", eventString.c_str(), ret);
        napi_throw(env, JsErrUtils::CreateJsError(env, ret, "[window][on]msg: Register event listener failed."));
        return NapiGetUndefined(env);
    }
    WLOGI("Window [%{public}u, %{public}s] register event %{public}s",
        window->GetWindowId(), window->GetWindowName().c_str(), eventString.c_str());

    return NapiGetUndefined(env);
}

napi_value JsWindowStage::OffEvent(napi_env env, napi_callback_info info)
{
    auto weakScene = windowScene_.lock();
    if (weakScene == nullptr) {
        WLOGFE("Window scene is null");
        napi_throw(env, JsErrUtils::CreateJsError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
            "[window][off]msg: Window scene is nullptr."));
        return NapiGetUndefined(env);
    }
    size_t argc = 4;
    napi_value argv[4] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    // Parse argv[0] as string
    std::string eventString;
    if (!ConvertFromJsValue(env, argv[0], eventString)) {
        WLOGFE("Failed to convert parameter to string");
        napi_throw(env, JsErrUtils::CreateJsError(env, WmErrorCode::WM_ERROR_INVALID_PARAM));
        return NapiGetUndefined(env);
    }

    auto window = weakScene->GetMainWindow();
    if (window == nullptr) {
        WLOGFE("Get window failed");
        napi_throw(env, JsErrUtils::CreateJsError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
            "[window][off]msg: Get main window failed."));
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
        TLOGE(WmsLogTag::DEFAULT, "unregister event %{public}s failed, ret=%{public}d", eventString.c_str(), ret);
        napi_throw(env, JsErrUtils::CreateJsError(env, ret, "[window][off]msg: Unregister event listener failed."));
        return NapiGetUndefined(env);
    }
    WLOGI("Window [%{public}u, %{public}s] unregister event %{public}s",
        window->GetWindowId(), window->GetWindowName().c_str(), eventString.c_str());

    return NapiGetUndefined(env);
}

static void LoadContentTask(std::shared_ptr<NativeReference> contentStorage, std::string contextUrl,
    sptr<Window> weakWindow, napi_env env, WsNapiAsyncTask& task, bool isLoadedByName)
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
            "[window][loadContent]msg: Window load content failed."));
    }
    WLOGI("Window [%{public}u, %{public}s] load content end, ret=%{public}d",
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
        TLOGE(WmsLogTag::WMS_LIFE, "Failed to convert parameter to context url");
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
        TLOGE(WmsLogTag::WMS_LIFE, "Window scene is null or get invalid param");
        napi_throw(env, JsErrUtils::CreateJsError(env, WmErrorCode::WM_ERROR_INVALID_PARAM));
        return NapiGetUndefined(env);
    }

    std::shared_ptr<NativeReference> contentStorage = nullptr;
    if (storage != nullptr) {
        napi_ref result = nullptr;
        napi_create_reference(env, storage, 1, &result);
        contentStorage = std::shared_ptr<NativeReference>(reinterpret_cast<NativeReference*>(result));
    }

    napi_value result = nullptr;
    std::shared_ptr<WsNapiAsyncTask> napiAsyncTask = CreateEmptyWsNapiAsyncTask(env, callBack, &result);
    auto asyncTask = [weak = windowScene_, contentStorage, contextUrl, isLoadedByName, env, task = napiAsyncTask] {
        auto weakScene = weak.lock();
        sptr<Window> win = weakScene ? weakScene->GetMainWindow() : nullptr;
        if (win == nullptr) {
            task->Reject(env, JsErrUtils::CreateJsError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
                "[window][loadContent]msg: Window is nullptr."));
            TLOGNE(WmsLogTag::WMS_LIFE, "Get window failed");
            return;
        }
        LoadContentTask(contentStorage, contextUrl, win, env, *task, isLoadedByName);
    };
    if (napi_send_event(env, asyncTask, napi_eprio_high, "OnLoadContent") != napi_status::napi_ok) {
        TLOGE(WmsLogTag::WMS_LIFE, "napi send event failed, window state is abnormal");
    }
    return result;
}

napi_value JsWindowStage::OnGetWindowMode(napi_env env, napi_callback_info info)
{
    size_t argc = 4;
    napi_value argv[4] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    napi_value callback = (GetType(env, argv[0]) == napi_function) ? argv[0] : nullptr;
    napi_value result = nullptr;
    std::shared_ptr<NapiAsyncTask> napiAsyncTask = CreateEmptyAsyncTask(env, callback, &result);
    auto asyncTask = [weak = windowScene_, env, task = napiAsyncTask] {
        auto weakScene = weak.lock();
        if (weakScene == nullptr) {
            task->Reject(env, JsErrUtils::CreateJsError(env, WMError::WM_ERROR_NULLPTR));
            TLOGNE(WmsLogTag::WMS_LIFE, "windowScene_ is nullptr");
            return;
        }
        auto window = weakScene->GetMainWindow();
        if (window == nullptr) {
            task->Reject(env, JsErrUtils::CreateJsError(env, WMError::WM_ERROR_NULLPTR, "Get window failed"));
            TLOGNE(WmsLogTag::WMS_LIFE, "Get window failed");
            return;
        }
        Rosen::WindowMode mode = window->GetWindowMode();
        if (NATIVE_TO_JS_WINDOW_MODE_MAP.count(mode) != 0) {
            task->Resolve(env, CreateJsValue(env, NATIVE_TO_JS_WINDOW_MODE_MAP.at(mode)));
            TLOGNI(WmsLogTag::WMS_LIFE, "Window [%{public}u, %{public}s] get mode %{public}u, api mode %{public}u",
                window->GetWindowId(), window->GetWindowName().c_str(),
                mode, NATIVE_TO_JS_WINDOW_MODE_MAP.at(mode));
        } else {
            task->Resolve(env, CreateJsValue(env, mode));
            TLOGNE(WmsLogTag::WMS_LIFE, "Get mode %{public}u, but not in apimode", mode);
        }
    };
    if (napi_send_event(env, asyncTask, napi_eprio_high, "OnGetWindowMode") != napi_status::napi_ok) {
        TLOGE(WmsLogTag::WMS_LIFE, "napi send event failed, window state is abnormal");
    }
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
        TLOGE(WmsLogTag::WMS_LIFE, "Failed to convert parameter to windowName");
        errCode = WmErrorCode::WM_ERROR_INVALID_PARAM;
    }
    if (errCode == WmErrorCode::WM_ERROR_INVALID_PARAM) {
        TLOGE(WmsLogTag::WMS_LIFE, "Get invalid param");
        napi_throw(env, JsErrUtils::CreateJsError(env, WmErrorCode::WM_ERROR_INVALID_PARAM));
        return NapiGetUndefined(env);
    }
    napi_value callback = (argv[1] != nullptr && GetType(env, argv[1]) == napi_function) ? argv[1] : nullptr;
    napi_value result = nullptr;
    std::shared_ptr<NapiAsyncTask> napiAsyncTask = CreateEmptyAsyncTask(env, callback, &result);
    auto asyncTask = [weak = windowScene_, windowName, env, task = napiAsyncTask] {
        auto weakScene = weak.lock();
        if (weakScene == nullptr) {
            TLOGNE(WmsLogTag::WMS_LIFE, "Window scene is null");
            task->Reject(env, JsErrUtils::CreateJsError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
                "[window][createSubWindow]msg: Window scene is nullptr."));
            return;
        }
        sptr<Rosen::WindowOption> windowOption = new Rosen::WindowOption();
        windowOption->SetWindowType(Rosen::WindowType::WINDOW_TYPE_APP_SUB_WINDOW);
        windowOption->SetWindowMode(Rosen::WindowMode::WINDOW_MODE_FLOATING);
        auto window = weakScene->CreateWindow(windowName, windowOption);
        if (window == nullptr) {
            TLOGNE(WmsLogTag::WMS_LIFE, "Create window failed");
            task->Reject(env, JsErrUtils::CreateJsError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
                "[window][createSubWindow]msg: Create window failed."));
            return;
        }
        task->Resolve(env, CreateJsWindowObject(env, window));
        TLOGNI(WmsLogTag::WMS_LIFE, "Create sub window %{public}s end", windowName.c_str());
    };
    if (napi_send_event(env, asyncTask, napi_eprio_high, "OnCreateSubWindow") != napi_status::napi_ok) {
        TLOGE(WmsLogTag::WMS_LIFE, "napi send event failed, window state is abnormal");
    }
    return result;
}

static napi_value CreateJsSubWindowArrayObject(napi_env env,
    std::vector<sptr<Window>> subWinVec)
{
    napi_value arrayValue = nullptr;
    napi_create_array_with_length(env, subWinVec.size(), &arrayValue);
    if (arrayValue == nullptr) {
        WLOGFE("Failed to convert subWinVec to jsArrayObject");
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
    size_t argc = 4;
    napi_value argv[4] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    napi_value callback = (argv[0] != nullptr && GetType(env, argv[0]) == napi_function) ? argv[0] : nullptr;
    napi_value result = nullptr;
    std::shared_ptr<NapiAsyncTask> napiAsyncTask = CreateEmptyAsyncTask(env, callback, &result);
    auto asyncTask = [weak = windowScene_, env, task = napiAsyncTask] {
        auto weakScene = weak.lock();
        if (weakScene == nullptr) {
            TLOGNE(WmsLogTag::WMS_LIFE, "Window scene is nullptr");
            task->Reject(env, JsErrUtils::CreateJsError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
                "[window][getSubWindow]msg: Window scene is nullptr."));
            return;
        }
        std::vector<sptr<Window>> subWindowVec = weakScene->GetSubWindow();
        task->Resolve(env, CreateJsSubWindowArrayObject(env, subWindowVec));
        TLOGNI(WmsLogTag::WMS_LIFE, "Get sub windows, size = %{public}zu", subWindowVec.size());
    };
    if (napi_send_event(env, asyncTask, napi_eprio_high, "OnGetSubWindow") != napi_status::napi_ok) {
        TLOGE(WmsLogTag::WMS_LIFE, "napi send event failed, window state is abnormal");
    }
    return result;
}

napi_value JsWindowStage::OnSetWindowModal(napi_env env, napi_callback_info info)
{
    auto windowScene = windowScene_.lock();
    if (windowScene == nullptr) {
        TLOGE(WmsLogTag::WMS_MAIN, "WindowScene is null");
        napi_throw(env, JsErrUtils::CreateJsError(env, WmErrorCode::WM_ERROR_STAGE_ABNORMALLY,
            "[window][setWindowModal]msg: Invalid window scene"));
        return NapiGetUndefined(env);
    }
    auto window = windowScene->GetMainWindow();
    if (window == nullptr) {
        TLOGE(WmsLogTag::WMS_MAIN, "window is nullptr");
        napi_throw(env, JsErrUtils::CreateJsError(env, WmErrorCode::WM_ERROR_STAGE_ABNORMALLY,
            "[window][setWindowModal]msg: Invalid main window"));
        return NapiGetUndefined(env);
    }
    if (window->IsPadAndNotFreeMultiWindowCompatibleMode()) {
        TLOGE(WmsLogTag::WMS_MAIN, "This is PcAppInPad, not support");
        return NapiGetUndefined(env);
    }
    if (!window->IsPcOrPadFreeMultiWindowMode()) {
        TLOGE(WmsLogTag::WMS_MAIN, "device not support");
        napi_throw(env, JsErrUtils::CreateJsError(env, WmErrorCode::WM_ERROR_DEVICE_NOT_SUPPORT,
            "[window][setWindowModal]msg: Device not support"));
        return NapiGetUndefined(env);
    }
    size_t argc = FOUR_PARAMS_SIZE;
    napi_value argv[FOUR_PARAMS_SIZE] = { nullptr };
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc != 1) {
        TLOGE(WmsLogTag::WMS_MAIN, "Argc is invalid: %{public}zu", argc);
        napi_throw(env, JsErrUtils::CreateJsError(env, WmErrorCode::WM_ERROR_INVALID_PARAM,
            "[window][setWindowModal]msg: Argc is invalid"));
        return NapiGetUndefined(env);
    }
    bool isModal = false;
    if (!ConvertFromJsValue(env, argv[INDEX_ZERO], isModal)) {
        TLOGE(WmsLogTag::WMS_MAIN, "Failed to convert parameter to bool");
        napi_throw(env, JsErrUtils::CreateJsError(env, WmErrorCode::WM_ERROR_INVALID_PARAM,
            "[window][setWindowModal]msg: Failed to convert parameter to bool"));
        return NapiGetUndefined(env);
    }
    napi_value result = nullptr;
    std::shared_ptr<NapiAsyncTask> napiAsyncTask = CreateEmptyAsyncTask(env, nullptr, &result);
    const char* const where = __func__;
    auto asyncTask = [where, weakWindow = wptr(window), isModal, env, task = napiAsyncTask] {
        auto window = weakWindow.promote();
        if (window == nullptr) {
            TLOGNE(WmsLogTag::WMS_MAIN, "%{public}s failed, window is null", where);
            task->Reject(env, JsErrUtils::CreateJsError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
                "[window][setWindowModal]msg: window is not valid"));
            return;
        }
        WMError ret = window->SetWindowModal(isModal);
        if (ret != WMError::WM_OK) {
            WmErrorCode wmErrorCode = WM_JS_TO_ERROR_CODE_MAP.at(ret);
            TLOGNE(WmsLogTag::WMS_MAIN, "%{public}s failed, ret is %{public}d", where, wmErrorCode);
            task->Reject(env, JsErrUtils::CreateJsError(env, wmErrorCode,
                "[window][setWindowModal]msg: set main window modal failed"));
            return;
        }
        task->Resolve(env, NapiGetUndefined(env));
        TLOGNI(WmsLogTag::WMS_MAIN, "%{public}s id:%{public}u, name:%{public}s, isModal:%{public}d",
            where, window->GetWindowId(), window->GetWindowName().c_str(), isModal);
    };
    if (napi_send_event(env, asyncTask, napi_eprio_high, "OnSetWindowModal") != napi_status::napi_ok) {
        napiAsyncTask->Reject(env,
            JsErrUtils::CreateJsError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
                "[window][setWindowModal]msg: send event failed"));
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
        WLOGFE("Argc is invalid: %{public}zu", argc);
        napi_throw(env, JsErrUtils::CreateJsError(env, WmErrorCode::WM_ERROR_INVALID_PARAM));
        return CreateJsValue(env, static_cast<int32_t>(WmErrorCode::WM_ERROR_INVALID_PARAM));
    }
    auto weakScene = windowScene_.lock();
    if (weakScene == nullptr || weakScene->GetMainWindow() == nullptr) {
        WLOGFE("WindowScene is null or window is null");
        napi_throw(env, JsErrUtils::CreateJsError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY));
        return CreateJsValue(env, static_cast<int32_t>(WmErrorCode::WM_ERROR_STATE_ABNORMALLY));
    }

    bool showOnLockScreen = false;
    napi_value nativeVal = argv[0];
    if (nativeVal == nullptr) {
        WLOGFE("Failed to convert parameter to boolean");
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
    WLOGFI("Window [%{public}u, %{public}s] %{public}u, ret=%{public}u",
        window->GetWindowId(), window->GetWindowName().c_str(), showOnLockScreen, ret);

    return CreateJsValue(env, static_cast<int32_t>(ret));
}

napi_value JsWindowStage::OnDisableWindowDecor(napi_env env, napi_callback_info info)
{
    auto weakScene = windowScene_.lock();
    if (weakScene == nullptr) {
        TLOGE(WmsLogTag::WMS_DECOR, "WindowScene is null");
        return CreateJsValue(env, static_cast<int32_t>(WmErrorCode::WM_ERROR_STAGE_ABNORMALLY));
    }

    auto window = weakScene->GetMainWindow();
    if (window == nullptr) {
        TLOGE(WmsLogTag::WMS_DECOR, "Window is null");
        return CreateJsValue(env, static_cast<int32_t>(WmErrorCode::WM_ERROR_STATE_ABNORMALLY));
    }
    WmErrorCode ret = WM_JS_TO_ERROR_CODE_MAP.at(window->DisableAppWindowDecor());
    if (ret != WmErrorCode::WM_OK) {
        napi_throw(env, JsErrUtils::CreateJsError(env, ret));
    TLOGI(WmsLogTag::WMS_DECOR, "Window [%{public}u, %{public}s] end",
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
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "Argc is invalid: %{public}zu", argc);
        return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM,
            "[window][setDefaultDensityEnabled]msg: Mandatory parameters are left unspecified");
    }

    auto weakScene = windowScene_.lock();
    if (weakScene == nullptr) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "WindowScene is null");
        return NapiThrowError(env, WmErrorCode::WM_ERROR_STAGE_ABNORMALLY,
            "[window][setDefaultDensityEnabled]msg: The window stage is not created or destroyed");
    }

    auto window = weakScene->GetMainWindow();
    if (window == nullptr) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "Window is null");
        return NapiThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
            "[window][setDefaultDensityEnabled]msg: The main window is not created or destroyed");
    }

    bool enabled = false;
    if (!ConvertFromJsValue(env, argv[0], enabled)) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "Failed to convert parameter to boolean");
        return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM,
            "[window][setDefaultDensityEnabled]msg: Incorrect parameter types");
    }

    WmErrorCode ret = WM_JS_TO_ERROR_CODE_MAP.at(window->SetDefaultDensityEnabled(enabled));
    TLOGI(WmsLogTag::WMS_ATTRIBUTE, "Window [%{public}u,%{public}s] enabled=%{public}u ret=%{public}u",
        window->GetWindowId(), window->GetWindowName().c_str(), enabled, ret);
    if (ret != WmErrorCode::WM_OK) {
        return NapiThrowError(env, ret, "[window][setDefaultDensityEnabled]");
    }
    return NapiGetUndefined(env);
}

napi_value JsWindowStage::OnSetCustomDensity(napi_env env, napi_callback_info info)
{
    size_t argc = FOUR_PARAMS_SIZE;
    napi_value argv[FOUR_PARAMS_SIZE] = { nullptr };
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc < ARG_COUNT_ONE || argc > ARG_COUNT_TWO) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "Argc is invalid: %{public}zu", argc);
        return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM,
            "[window][setCustomDensity]msg: Mandatory parameters are left unspecified");
    }

    double density = UNDEFINED_DENSITY;
    if (!ConvertFromJsValue(env, argv[INDEX_ZERO], density)) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "Failed to convert parameter to double");
        return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM,
            "[window][setCustomDensity]msg: Incorrect parameter types");
    }

    bool applyToSubWindow = false;
    if (GetType(env, argv[INDEX_ONE]) != napi_undefined && GetType(env, argv[INDEX_ONE]) != napi_null) {
        if (!ConvertFromJsValue(env, argv[INDEX_ONE], applyToSubWindow)) {
            TLOGE(WmsLogTag::WMS_ATTRIBUTE, "Failed to convert parameter to boolean");
            return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM,
                "[window][setCustomDensity]msg: Incorrect parameter types");
        }
    }

    auto windowScene = windowScene_.lock();
    if (windowScene == nullptr) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "windowScene is null");
        return NapiThrowError(env, WmErrorCode::WM_ERROR_STAGE_ABNORMALLY,
            "[window][setCustomDensity]msg: The window stage is not created or destroyed");
    }
    auto window = windowScene->GetMainWindow();
    if (window == nullptr) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "Window is null");
        return NapiThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
            "[window][setCustomDensity]msg: The main window is not created or destroyed");
    }

    WmErrorCode ret = WM_JS_TO_ERROR_CODE_MAP.at(window->SetCustomDensity(density, applyToSubWindow));
    TLOGI(WmsLogTag::WMS_ATTRIBUTE, "Window [%{public}u,%{public}s] set density=%{public}f, "
        "applyToSubWindow=%{public}d, result=%{public}u",
        window->GetWindowId(), window->GetWindowName().c_str(), density, applyToSubWindow, ret);
    if (ret != WmErrorCode::WM_OK) {
        return NapiThrowError(env, ret, "[window][setCustomDensity]");
    }
    return NapiGetUndefined(env);
}

napi_value JsWindowStage::OnCreateSubWindowWithOptions(napi_env env, napi_callback_info info)
{
    auto windowScene = windowScene_.lock();
    if (windowScene == nullptr) {
        TLOGE(WmsLogTag::WMS_SUB, "WindowScene is null");
        napi_throw(env, JsErrUtils::CreateJsError(env, WmErrorCode::WM_ERROR_STAGE_ABNORMALLY,
            "[window][createSubWindowWithOptions]msg: Window scene is nullptr."));
        return NapiGetUndefined(env);
    }
    size_t argc = 4;
    napi_value argv[4] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    std::string windowName;
    if (!ConvertFromJsValue(env, argv[0], windowName)) {
        WLOGFE("Failed to convert parameter to windowName");
        napi_throw(env, JsErrUtils::CreateJsError(env, WmErrorCode::WM_ERROR_INVALID_PARAM));
        return NapiGetUndefined(env);
    }
    sptr<WindowOption> option = new WindowOption();
    if (!ParseSubWindowOptions(env, argv[1], option)) {
        WLOGFE("Get invalid options param");
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
    napi_value callback = (argv[2] != nullptr && GetType(env, argv[2]) == napi_function) ? argv[2] : nullptr;
    napi_value result = nullptr;
    std::shared_ptr<NapiAsyncTask> napiAsyncTask = CreateEmptyAsyncTask(env, callback, &result);
    auto asyncTask = [where, windowScene, windowName = std::move(windowName), option, env,
        task = napiAsyncTask]() mutable {
        option->SetWindowType(WindowType::WINDOW_TYPE_APP_SUB_WINDOW);
        option->SetWindowMode(WindowMode::WINDOW_MODE_FLOATING);
        option->SetOnlySupportSceneBoard(true);
        auto window = windowScene->CreateWindow(windowName, option);
        if (window == nullptr) {
            TLOGNE(WmsLogTag::WMS_SUB, "%{public}s Create window failed", where);
            task->Reject(env, JsErrUtils::CreateJsError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
                "[window][createSubWindowWithOptions]msg: Create window failed."));
            return;
        }
        task->Resolve(env, CreateJsWindowObject(env, window));
        TLOGNI(WmsLogTag::WMS_SUB, "%{public}s Create sub window %{public}s end",
            where, windowName.c_str());
    };
    if (napi_send_event(env, asyncTask, napi_eprio_high, "OnCreateSubWindowWithOptions") != napi_status::napi_ok) {
        TLOGE(WmsLogTag::WMS_SUB, "napi send event failed, window state is abnormal");
    }
    return result;
}

napi_value JsWindowStage::OnRemoveStartingWindow(napi_env env, napi_callback_info info)
{
    auto windowScene = windowScene_.lock();
    if (windowScene == nullptr) {
        TLOGE(WmsLogTag::WMS_STARTUP_PAGE, "windowScene is null");
        napi_throw(env, JsErrUtils::CreateJsError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
            "[window][removeStartingWindow]msg: The window stage is not created or destroyed"));
        return NapiGetUndefined(env);
    }

    size_t argc = FOUR_PARAMS_SIZE;
    napi_value argv[FOUR_PARAMS_SIZE] = { nullptr };
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    napi_value lastParam = (argc == 0) ? nullptr :
        (argv[INDEX_ZERO] != nullptr && GetType(env, argv[INDEX_ZERO]) == napi_function ? argv[INDEX_ZERO] : nullptr);
    napi_value result = nullptr;
    std::shared_ptr<NapiAsyncTask> napiAsyncTask = CreateEmptyAsyncTask(env, lastParam, &result);
    auto window = windowScene->GetMainWindow();
    const char* const where = __func__;
    auto asyncTask = [weakWindow = wptr(window), where, env, task = napiAsyncTask] {
        auto window = weakWindow.promote();
        if (window == nullptr) {
            TLOGNE(WmsLogTag::WMS_STARTUP_PAGE, "%{public}s window is nullptr", where);
            task->Reject(env, JsErrUtils::CreateJsError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
                "[window][removeStartingWindow]msg: The main window is not created or destroyed"));
            return;
        }
        WmErrorCode ret = WM_JS_TO_ERROR_CODE_MAP.at(window->NotifyRemoveStartingWindow());
        if (ret == WmErrorCode::WM_OK) {
            task->Resolve(env, NapiGetUndefined(env));
        } else {
            task->Reject(env, JsErrUtils::CreateJsError(env, ret, "[window][removeStartingWindow]"));
        }
    };
    if (napi_send_event(env, asyncTask, napi_eprio_immediate, "OnRemoveStartingWindow") != napi_status::napi_ok) {
        napiAsyncTask->Reject(env, CreateJsError(env, static_cast<int32_t>(WmErrorCode::WM_ERROR_STATE_ABNORMALLY),
            "[window][removeStartingWindow]msg: Internal task error"));
    }
    return result;
}

napi_value JsWindowStage::OnSetWindowRectAutoSave(napi_env env, napi_callback_info info)
{
    auto windowScene = windowScene_.lock();
    if (windowScene == nullptr) {
        TLOGE(WmsLogTag::WMS_LAYOUT_PC, "WindowScene is null");
        return NapiThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
            "[window][setWindowRectAutoSave]msg: WindowScene is null.");
    }
    size_t argc = FOUR_PARAMS_SIZE;
    napi_value argv[FOUR_PARAMS_SIZE] = { nullptr };
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc != ARG_COUNT_TWO) {
        TLOGE(WmsLogTag::WMS_LAYOUT_PC, "Argc is invalid: %{public}zu", argc);
        return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM,
            "[window][setWindowRectAutoSave]msg: Incorrect number of parameters. Expected 2.");
    }
    bool enabled = false;
    if (!ConvertFromJsValue(env, argv[INDEX_ZERO], enabled)) {
        TLOGE(WmsLogTag::WMS_LAYOUT_PC, "Failed to convert parameter to enabled");
        return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM,
            "[window][setWindowRectAutoSave]msg: Failed to convert parameter to enabled.");
    }
    bool isSaveBySpecifiedFlag = false;
    ConvertFromJsValue(env, argv[INDEX_ONE], isSaveBySpecifiedFlag);
    auto window = windowScene->GetMainWindow();
    const char* const where = __func__;
    napi_value result = nullptr;
    std::shared_ptr<NapiAsyncTask> napiAsyncTask = CreateEmptyAsyncTask(env, nullptr, &result);
    auto asyncTask = [weakWindow = wptr(window), where, env, task = napiAsyncTask, enabled, isSaveBySpecifiedFlag] {
        auto window = weakWindow.promote();
        if (window == nullptr) {
            TLOGNE(WmsLogTag::WMS_LAYOUT_PC, "%{public}s window is nullptr", where);
            WmErrorCode wmErroeCode = WM_JS_TO_ERROR_CODE_MAP.at(WMError::WM_ERROR_NULLPTR);
            task->Reject(env, JsErrUtils::CreateJsError(env, wmErroeCode,
                "[window][setWindowRectAutoSave]msg: Window is nullptr."));
            return;
        }
        WmErrorCode ret = WM_JS_TO_ERROR_CODE_MAP.at(window->SetWindowRectAutoSave(enabled, isSaveBySpecifiedFlag));
        if (ret != WmErrorCode::WM_OK) {
            TLOGNE(WmsLogTag::WMS_LAYOUT_PC, "%{public}s enable recover position failed!", where);
            task->Reject(env, JsErrUtils::CreateJsError(env, ret,
                "[window][setWindowRectAutoSave]msg: Window recover position failed."));
        } else {
            task->Resolve(env, NapiGetUndefined(env));
            TLOGNI(WmsLogTag::WMS_MAIN, "%{public}s id %{public}d isSaveBySpecifiedFlag: %{public}d "
                "enable:%{public}d", where, window->GetWindowId(), isSaveBySpecifiedFlag, enabled);
        }
    };
    if (napi_send_event(env, asyncTask, napi_eprio_high, "OnSetWindowRectAutoSave") != napi_status::napi_ok) {
        napiAsyncTask->Reject(env,
            CreateJsError(env, static_cast<int32_t>(WmErrorCode::WM_ERROR_STATE_ABNORMALLY),
            "[window][setWindowRectAutoSave]msg: Send event failed."));
    }
    return result;
}

napi_value JsWindowStage::OnSetSupportedWindowModes(napi_env env, napi_callback_info info)
{
    auto windowScene = windowScene_.lock();
    if (windowScene == nullptr) {
        TLOGE(WmsLogTag::WMS_LAYOUT_PC, "WindowScene is null");
        napi_throw(env, JsErrUtils::CreateJsError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
            "[window][setSupportedWindowModes]msg: The window is not created or destoryed."));
        return NapiGetUndefined(env);
    }

    size_t argc = FOUR_PARAMS_SIZE;
    napi_value argv[FOUR_PARAMS_SIZE] = { nullptr };
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (!(argc == ARG_COUNT_ONE || argc == ARG_COUNT_TWO)) {
        TLOGE(WmsLogTag::WMS_LAYOUT_PC, "Argc is invalid: %{public}zu", argc);
        napi_throw(env, JsErrUtils::CreateJsError(env, WmErrorCode::WM_ERROR_INVALID_PARAM,
            "[window][setSupportedWindowModes]msg: The number of parameters is invalid"));
        return NapiGetUndefined(env);
    }

    if (GetType(env, argv[INDEX_ZERO]) != napi_object) {
        TLOGE(WmsLogTag::WMS_LAYOUT_PC, "GetType error");
        napi_throw(env, JsErrUtils::CreateJsError(env, WmErrorCode::WM_ERROR_INVALID_PARAM,
            "[window][setSupportedWindowModes]msg: Failed to covert object"));
        return NapiGetUndefined(env);
    }

    std::vector<AppExecFwk::SupportWindowMode> supportedWindowModes;
    if (!ConvertNativeValueToVector(env, argv[INDEX_ZERO], supportedWindowModes)) {
        TLOGE(WmsLogTag::WMS_LAYOUT_PC, "ConvertNativeValueToVector failed");
        napi_throw(env, JsErrUtils::CreateJsError(env, WmErrorCode::WM_ERROR_INVALID_PARAM,
            "[window][setSupportedWindowModes]msg: Failed to convert supportedWindowModes parameter"));
        return NapiGetUndefined(env);
    }

    bool grayOutMaximizeButton = false;
    if (argc == ARG_COUNT_TWO && !ConvertFromJsValue(env, argv[INDEX_ONE], grayOutMaximizeButton)) {
        TLOGE(WmsLogTag::WMS_LAYOUT_PC, "Failed to convert grayOutMaximizeButton parameter");
        napi_throw(env, JsErrUtils::CreateJsError(env, WmErrorCode::WM_ERROR_ILLEGAL_PARAM,
            "[window][setSupportedWindowModes]msg: Failed to convert grayOutMaximizeButton parameter"));
        return NapiGetUndefined(env);
    }

    auto window = windowScene->GetMainWindow();
    const char* const where = __func__;
    napi_value result = nullptr;
    std::shared_ptr<NapiAsyncTask> napiAsyncTask = CreateEmptyAsyncTask(env, nullptr, &result);
    auto asyncTask = [weakWindow = wptr(window), supportedWindowModes = std::move(supportedWindowModes),
        grayOutMaximizeButton, where, env, task = napiAsyncTask] {
        auto window = weakWindow.promote();
        if (window == nullptr) {
            TLOGNE(WmsLogTag::WMS_LAYOUT_PC, "%{public}s window is nullptr", where);
            WmErrorCode wmErroeCode = WM_JS_TO_ERROR_CODE_MAP.at(WMError::WM_ERROR_NULLPTR);
            task->Reject(env, JsErrUtils::CreateJsError(env, wmErroeCode, "The window is not created or destoryed."));
            return;
        }
        WmErrorCode ret = WM_JS_TO_ERROR_CODE_MAP.at(window->SetSupportedWindowModes(supportedWindowModes,
            grayOutMaximizeButton));
        if (ret != WmErrorCode::WM_OK) {
            TLOGNE(WmsLogTag::WMS_LAYOUT_PC, "window [%{public}u, %{public}s] "
                "set window support modes failed!", window->GetWindowId(),
                window->GetWindowName().c_str());
            task->Reject(env, JsErrUtils::CreateJsError(env, ret, "[window][setSupportedWindowModes]"));
        } else {
            TLOGNI(WmsLogTag::WMS_LAYOUT_PC, "window [%{public}u, %{public}s] "
                "set window support modes succeed.", window->GetWindowId(),
                window->GetWindowName().c_str());
            task->Resolve(env, NapiGetUndefined(env));
        }
    };
    napi_status status = napi_send_event(env, std::move(asyncTask), napi_eprio_high, "OnSetSupportedWindowModes");
    if (status != napi_status::napi_ok) {
        napiAsyncTask->Reject(env, CreateJsError(env, static_cast<int32_t>(WmErrorCode::WM_ERROR_STATE_ABNORMALLY),
            "[window][setSupportedWindowModes]msg: Internal task error"));
    }
    return result;
}

napi_value JsWindowStage::OnSetImageForRecent(napi_env env, napi_callback_info info)
{
    size_t argc = FOUR_PARAMS_SIZE;
    napi_value argv[FOUR_PARAMS_SIZE] = { nullptr };
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc != ARG_COUNT_TWO) {
        TLOGE(WmsLogTag::WMS_PATTERN, "Argc is invalid: %{public}zu", argc);
        return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM);
    }
    uint32_t imgResourceId = 0;
    std::shared_ptr<Media::PixelMap> pixelMap = nullptr;
    if (GetType(env, argv[INDEX_ZERO]) == napi_number) {
        if (!ConvertFromJsValue(env, argv[INDEX_ZERO], imgResourceId)) {
            TLOGE(WmsLogTag::WMS_PATTERN, "Get imgResourceId error");
            return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM);
        }
        if (imgResourceId < MIN_RESOURCE_ID || imgResourceId > MAX_RESOURCE_ID) {
            TLOGE(WmsLogTag::WMS_PATTERN, "imgRsourceId invalid: %{public}d", imgResourceId);
            return NapiThrowError(env, WmErrorCode::WM_ERROR_ILLEGAL_PARAM);
        }
    } else if (GetType(env, argv[INDEX_ZERO]) == napi_object) {
        pixelMap = OHOS::Media::PixelMapNapi::GetPixelMap(env, argv[INDEX_ZERO]);
        if (pixelMap == nullptr) {
            TLOGE(WmsLogTag::WMS_PATTERN, "Get pixelMap error");
            return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM);
        }
    } else {
        TLOGE(WmsLogTag::WMS_PATTERN, "Get imgResourceId or pixelMap error");
        return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM);
    }
    ImageFit imageFit = ImageFit::FILL;
    if (!ConvertFromJsValue(env, argv[INDEX_ONE], imageFit)) {
        TLOGE(WmsLogTag::WMS_PATTERN, "Get imageFit error");
        return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM);
    }
    if (imageFit < ImageFit::FILL || imageFit > ImageFit::MATRIX) {
        TLOGE(WmsLogTag::WMS_PATTERN, "imageFit invalid: %{public}d", imageFit);
        return NapiThrowError(env, WmErrorCode::WM_ERROR_ILLEGAL_PARAM);
    }

    napi_value result = nullptr;
    std::shared_ptr<NapiAsyncTask> napiAsyncTask = CreateEmptyAsyncTask(env, nullptr, &result);
    const char* const where = __func__;
    auto asyncTask = [weakWindow = windowScene_, where, env, imgResourceId, pixelMap, imageFit, task = napiAsyncTask] {
        auto windowScene = weakWindow.lock();
        if (windowScene == nullptr) {
            TLOGNE(WmsLogTag::WMS_PATTERN, "%{public}s windowScene is null", where);
            task->Reject(env, JsErrUtils::CreateJsError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY));
            return;
        }
        auto window = windowScene->GetMainWindow();
        if (window == nullptr) {
            TLOGNE(WmsLogTag::WMS_PATTERN, "%{public}s window is nullptr", where);
            WmErrorCode wmErroeCode = WM_JS_TO_ERROR_CODE_MAP.at(WMError::WM_ERROR_NULLPTR);
            task->Reject(env, JsErrUtils::CreateJsError(env, wmErroeCode, "window is nullptr."));
            return;
        }
        WmErrorCode ret = WmErrorCode::WM_OK;
        if (pixelMap) {
            ret = WM_JS_TO_ERROR_CODE_MAP.at(window->SetImageForRecentPixelMap(pixelMap, imageFit));
        } else {
            ret = WM_JS_TO_ERROR_CODE_MAP.at(window->SetImageForRecent(imgResourceId, imageFit));
        }
        if (ret == WmErrorCode::WM_OK) {
            task->Resolve(env, NapiGetUndefined(env));
        } else {
            task->Reject(env, JsErrUtils::CreateJsError(env, ret, "set image for recent failed."));
        }
    };
    if (napi_send_event(env, asyncTask, napi_eprio_high, "OnSetImageForRecent") != napi_status::napi_ok) {
        napiAsyncTask->Reject(env,
            CreateJsError(env, static_cast<int32_t>(WmErrorCode::WM_ERROR_STATE_ABNORMALLY), "send event failed"));
    }
    return result;
}

napi_value JsWindowStage::OnRemoveImageForRecent(napi_env env, napi_callback_info info)
{
    napi_value result = nullptr;
    std::shared_ptr<NapiAsyncTask> napiAsyncTask = CreateEmptyAsyncTask(env, nullptr, &result);
    const char* const where = __func__;
    auto asyncTask = [weakWindow = windowScene_, where, env, task = napiAsyncTask] {
        auto windowScene = weakWindow.lock();
        if (windowScene == nullptr) {
            TLOGNE(WmsLogTag::WMS_PATTERN, "%{public}s windowScene is null", where);
            task->Reject(env, JsErrUtils::CreateJsError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY));
            return;
        }
        auto window = windowScene->GetMainWindow();
        if (window == nullptr) {
            TLOGNE(WmsLogTag::WMS_PATTERN, "%{public}s window is nullptr", where);
            WmErrorCode wmErroeCode = WM_JS_TO_ERROR_CODE_MAP.at(WMError::WM_ERROR_NULLPTR);
            task->Reject(env, JsErrUtils::CreateJsError(env, wmErroeCode, "window is nullptr."));
            return;
        }
        WmErrorCode ret = WM_JS_TO_ERROR_CODE_MAP.at(window->RemoveImageForRecent());
        if (ret == WmErrorCode::WM_OK) {
            task->Resolve(env, NapiGetUndefined(env));
        } else {
            task->Reject(env, JsErrUtils::CreateJsError(env, ret, "remove image for recent failed."));
        }
    };
    if (napi_send_event(env, asyncTask, napi_eprio_high, "OnRemoveImageForRecent") != napi_status::napi_ok) {
        napiAsyncTask->Reject(env,
            CreateJsError(env, static_cast<int32_t>(WmErrorCode::WM_ERROR_STATE_ABNORMALLY), "send event failed"));
    }
    return result;
}

napi_value JsWindowStage::OnIsWindowRectAutoSave(napi_env env, napi_callback_info info)
{
    auto windowScene = windowScene_.lock();
    if (windowScene == nullptr) {
        TLOGE(WmsLogTag::WMS_LAYOUT_PC, "WindowScene is null");
        napi_throw(env, JsErrUtils::CreateJsError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
            "[window][isWindowRectAutoSave]msg: WindowScene is null."));
        return NapiGetUndefined(env);
    }

    auto window = windowScene->GetMainWindow();
    const char* const where = __func__;
    napi_value result = nullptr;
    std::shared_ptr<NapiAsyncTask> napiAsyncTask = CreateEmptyAsyncTask(env, nullptr, &result);
    auto asyncTask = [weakWindow = wptr(window), where, env, task = napiAsyncTask] {
        auto window = weakWindow.promote();
        if (window == nullptr) {
            TLOGNE(WmsLogTag::WMS_LAYOUT_PC, "%{public}s Window is nullptr", where);
            task->Reject(env,
                JsErrUtils::CreateJsError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
                "[window][isWindowRectAutoSave]msg: Window is nullptr."));
            return;
        }
        bool enabled = false;
        WmErrorCode ret = WM_JS_TO_ERROR_CODE_MAP.at(window->IsWindowRectAutoSave(enabled));
        if (ret != WmErrorCode::WM_OK) {
            TLOGNE(WmsLogTag::WMS_LAYOUT_PC, "%{public}s get the auto-save state of the window rect failed!", where);
            task->Reject(env, JsErrUtils::CreateJsError(env,
                ret, "[window][isWindowRectAutoSave]msg: Window recover position failed."));
        } else {
            napi_value jsEnabled = CreateJsValue(env, enabled);
            task->Resolve(env, jsEnabled);
        }
    };
    if (napi_send_event(env, asyncTask, napi_eprio_high, "OnIsWindowRectAutoSave") != napi_status::napi_ok) {
        napiAsyncTask->Reject(env,
            CreateJsError(env, static_cast<int32_t>(WmErrorCode::WM_ERROR_STATE_ABNORMALLY),
            "[window][isWindowRectAutoSave]msg: Send event failed."));
    }
    return result;
}

napi_value CreateJsWindowStage(napi_env env, std::shared_ptr<Rosen::WindowScene> windowScene)
{
    WLOGFD("[NAPI]");
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
    BindNativeFunction(env,
        objValue, "setSupportedWindowModes", moduleName, JsWindowStage::SetSupportedWindowModes);
    BindNativeFunction(env,
        objValue, "setImageForRecent", moduleName, JsWindowStage::SetImageForRecent);
    BindNativeFunction(env,
        objValue, "removeImageForRecent", moduleName, JsWindowStage::RemoveImageForRecent);
    return objValue;
}
}  // namespace Rosen
}  // namespace OHOS
