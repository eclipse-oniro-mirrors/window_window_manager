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

#include "js_embeddable_window_stage.h"
#include <string>
#include "js_runtime_utils.h"
#include "js_extension_window.h"
#include "js_extension_window_utils.h"
#include "window_manager_hilog.h"
#include "js_window_utils.h"
#include "js_window.h"
#include "permission.h"

namespace OHOS {
namespace Rosen {
using namespace AbilityRuntime;
namespace {
constexpr int CONTENT_STORAGE_ARG = 2;
const std::unordered_set<std::string> g_emptyListener = { "windowStageClose" };

bool IsEmptyListener(const std::string& type)
{
    return g_emptyListener.find(type) != g_emptyListener.end();
}
} //namespace

JsEmbeddableWindowStage::JsEmbeddableWindowStage(sptr<Rosen::Window> window, sptr<AAFwk::SessionInfo> sessionInfo)
    : windowExtensionSessionImpl_(window), sessionInfo_(sessionInfo),
    extwinRegisterManager_(std::make_unique<JsExtensionWindowRegisterManager>())
{
}

JsEmbeddableWindowStage::~JsEmbeddableWindowStage()
{
}

void JsEmbeddableWindowStage::Finalizer(napi_env env, void* data, void* hint)
{
    TLOGI(WmsLogTag::WMS_UIEXT, "[NAPI]");
    std::unique_ptr<JsEmbeddableWindowStage>(static_cast<JsEmbeddableWindowStage*>(data));
}

napi_value JsEmbeddableWindowStage::GetMainWindow(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::WMS_UIEXT, "[NAPI]");
    JsEmbeddableWindowStage* me = CheckParamsAndGetThis<JsEmbeddableWindowStage>(env, info);
    return (me != nullptr) ? me->OnGetMainWindow(env, info) : nullptr;
}

napi_value JsEmbeddableWindowStage::GetMainWindowSync(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::WMS_UIEXT, "[NAPI]");
    JsEmbeddableWindowStage* me = CheckParamsAndGetThis<JsEmbeddableWindowStage>(env, info);
    return (me != nullptr) ? me->OnGetMainWindowSync(env, info) : nullptr;
}

napi_value JsEmbeddableWindowStage::On(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::WMS_UIEXT, "[NAPI]");
    JsEmbeddableWindowStage* me = CheckParamsAndGetThis<JsEmbeddableWindowStage>(env, info);
    return (me != nullptr) ? me->OnEvent(env, info) : nullptr;
}

napi_value JsEmbeddableWindowStage::Off(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::WMS_UIEXT, "[NAPI]");
    JsEmbeddableWindowStage* me = CheckParamsAndGetThis<JsEmbeddableWindowStage>(env, info);
    return (me != nullptr) ? me->OffEvent(env, info) : nullptr;
}

napi_value JsEmbeddableWindowStage::LoadContent(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::WMS_UIEXT, "[NAPI]");
    JsEmbeddableWindowStage* me = CheckParamsAndGetThis<JsEmbeddableWindowStage>(env, info);
    return (me != nullptr) ? me->OnLoadContent(env, info, false) : nullptr;
}

napi_value JsEmbeddableWindowStage::LoadContentByName(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::WMS_UIEXT, "[NAPI]");
    JsEmbeddableWindowStage* me = CheckParamsAndGetThis<JsEmbeddableWindowStage>(env, info);
    return (me != nullptr) ? me->OnLoadContent(env, info, true) : nullptr;
}

napi_value JsEmbeddableWindowStage::CreateSubWindowWithOptions(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::WMS_UIEXT, "[NAPI]");
    JsEmbeddableWindowStage* me = CheckParamsAndGetThis<JsEmbeddableWindowStage>(env, info);
    return (me != nullptr) ? me->OnCreateSubWindowWithOptions(env, info) : nullptr;
}

napi_value JsEmbeddableWindowStage::SetUIContent(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::WMS_UIEXT, "[NAPI]");
    JsEmbeddableWindowStage* me = CheckParamsAndGetThis<JsEmbeddableWindowStage>(env, info);
    return (me != nullptr) ? me->OnSetUIContent(env, info) : nullptr;
}

napi_value JsEmbeddableWindowStage::EmptyAsyncCall(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::WMS_UIEXT, "[NAPI]");
    JsEmbeddableWindowStage* me = CheckParamsAndGetThis<JsEmbeddableWindowStage>(env, info);
    return (me != nullptr) ? me->OnEmptyAsyncCall(env, info) : nullptr;
}

napi_value JsEmbeddableWindowStage::EmptySyncCall(napi_env env, napi_callback_info info)
{
    return NapiGetUndefined(env);
}

napi_value JsEmbeddableWindowStage::UnsupportAsyncCall(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::WMS_UIEXT, "[NAPI]");
    JsEmbeddableWindowStage* me = CheckParamsAndGetThis<JsEmbeddableWindowStage>(env, info);
    return (me != nullptr) ? me->OnUnsupportAsyncCall(env, info) : nullptr;
}

napi_value JsEmbeddableWindowStage::UnsupportSyncCall(napi_env env, napi_callback_info info)
{
    return NapiThrowError(env, WmErrorCode::WM_ERROR_DEVICE_NOT_SUPPORT);
}

napi_value JsEmbeddableWindowStage::OnGetMainWindow(napi_env env, napi_callback_info info)
{
    const char* const where = __func__;
    size_t argc = 4;
    napi_value argv[4] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    napi_value callback = (argc > 0 && argv[0] != nullptr && GetType(env, argv[0]) == napi_function) ?
        argv[0] : nullptr;
    napi_value result = nullptr;
    std::shared_ptr<NapiAsyncTask> napiAsyncTask = CreateEmptyAsyncTask(env, callback, &result);
    auto asyncTask = [windowImpl = windowExtensionSessionImpl_, sessionInfo = sessionInfo_,
        where, env, task = napiAsyncTask] {
            if (windowImpl != nullptr) {
                task->Resolve(env, Rosen::JsExtensionWindow::CreateJsExtensionWindowObject(env,
                windowImpl, sessionInfo));
                TLOGNI(WmsLogTag::WMS_UIEXT, "%{public}s [%{public}u, %{public}s]",
                    where, windowImpl->GetWindowId(), windowImpl->GetWindowName().c_str());
            } else {
                task->Reject(env, CreateJsError(env,
                    static_cast<int32_t>(WmErrorCode::WM_ERROR_STATE_ABNORMALLY),
                    "Get main window failed."));
            }
        };
    if (napi_send_event(env, asyncTask, napi_eprio_high, "OnGetMainWindow") != napi_status::napi_ok) {
        napiAsyncTask->Reject(env, CreateJsError(env,
            static_cast<int32_t>(WmErrorCode::WM_ERROR_STATE_ABNORMALLY), "send event failed"));
    }
    return result;
}

napi_value JsEmbeddableWindowStage::OnGetMainWindowSync(napi_env env, napi_callback_info info)
{
    if (windowExtensionSessionImpl_ == nullptr) {
        TLOGE(WmsLogTag::WMS_UIEXT, "window is null");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WmErrorCode::WM_ERROR_STATE_ABNORMALLY)));
        return NapiGetUndefined(env);
    }

    return Rosen::JsExtensionWindow::CreateJsExtensionWindowObject(env,
        windowExtensionSessionImpl_, sessionInfo_);
}

napi_value JsEmbeddableWindowStage::OnEvent(napi_env env, napi_callback_info info)
{
    sptr<Window> windowImpl = windowExtensionSessionImpl_;
    if (windowImpl == nullptr) {
        TLOGE(WmsLogTag::WMS_UIEXT, "windowImpl is null");
        return NapiGetUndefined(env);
    }
    size_t argc = 4;
    napi_value argv[4] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc < 2) { // 2: minimum param nums
        TLOGE(WmsLogTag::WMS_UIEXT, "argc is invalid: %{public}zu", argc);
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WmErrorCode::WM_ERROR_INVALID_PARAM)));
        return NapiGetUndefined(env);
    }

    // Parse argv[0] as string
    std::string eventString;
    if (!ConvertFromJsValue(env, argv[0], eventString)) {
        TLOGE(WmsLogTag::WMS_UIEXT, "Failed to convert parameter to string");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WmErrorCode::WM_ERROR_INVALID_PARAM)));
        return NapiGetUndefined(env);
    }
    if (IsEmptyListener(eventString)) {
        return NapiGetUndefined(env);
    }
    napi_value value = argv[1];
    if (!NapiIsCallable(env, value)) {
        TLOGE(WmsLogTag::WMS_UIEXT, "Callback(argv[1]) is not callable");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WmErrorCode::WM_ERROR_INVALID_PARAM)));
        return NapiGetUndefined(env);
    }

    WmErrorCode ret = extwinRegisterManager_->AtomicServiceRegisterListener(windowImpl, eventString,
        CaseType::CASE_STAGE, env, value);
    if (ret != WmErrorCode::WM_OK) {
        TLOGE(WmsLogTag::WMS_UIEXT, "RegisterListener fail");
        return NapiThrowError(env, ret);
    }
    TLOGI(WmsLogTag::WMS_UIEXT, "Window [%{public}u, %{public}s] register event %{public}s",
        windowImpl->GetWindowId(), windowImpl->GetWindowName().c_str(), eventString.c_str());

    return NapiGetUndefined(env);
}

napi_value JsEmbeddableWindowStage::OffEvent(napi_env env, napi_callback_info info)
{
    sptr<Window> windowImpl = windowExtensionSessionImpl_;
    if (windowImpl == nullptr) {
        TLOGE(WmsLogTag::WMS_UIEXT, "windowImpl is null");
        return NapiGetUndefined(env);
    }
    size_t argc = 4;
    napi_value argv[4] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    // Parse argv[0] as string
    std::string eventString;
    if (!ConvertFromJsValue(env, argv[0], eventString)) {
        TLOGE(WmsLogTag::WMS_UIEXT, "Failed to convert parameter to string");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WmErrorCode::WM_ERROR_INVALID_PARAM)));
        return NapiGetUndefined(env);
    }
    if (eventString.compare("windowStageEvent") != 0) {
        TLOGE(WmsLogTag::WMS_UIEXT, "Event %{public}s is invalid", eventString.c_str());
        if (IsEmptyListener(eventString)) {
            return NapiGetUndefined(env);
        }
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WmErrorCode::WM_ERROR_INVALID_PARAM)));
        return NapiGetUndefined(env);
    }

    napi_value value = nullptr;
    WmErrorCode ret = WmErrorCode::WM_OK;
    if (argc == 1) {
        ret = extwinRegisterManager_->AtomicServiceUnregisterListener(windowImpl, eventString,
            CaseType::CASE_STAGE, env, nullptr);
    } else {
        value = argv[1];
        if (value != nullptr && GetType(env, value) == napi_function) {
            ret = extwinRegisterManager_->AtomicServiceUnregisterListener(windowImpl, eventString,
                CaseType::CASE_STAGE, env, value);
        } else {
            ret = extwinRegisterManager_->AtomicServiceUnregisterListener(windowImpl, eventString,
                CaseType::CASE_STAGE, env, nullptr);
        }
    }
    
    if (ret != WmErrorCode::WM_OK) {
        TLOGE(WmsLogTag::WMS_UIEXT, "UnregisterListener fail");
        return NapiThrowError(env, ret);
    }
    TLOGI(WmsLogTag::WMS_UIEXT, "Window [%{public}u, %{public}s] unregister event %{public}s",
        windowImpl->GetWindowId(), windowImpl->GetWindowName().c_str(), eventString.c_str());

    return NapiGetUndefined(env);
}

static void LoadContentTask(std::shared_ptr<NativeReference> contentStorage, std::string contextUrl,
    sptr<Window> windowImpl, napi_env env, NapiAsyncTask& task, sptr<IRemoteObject> parentToken,
    bool isLoadedByName)
{
    napi_value nativeStorage = (contentStorage == nullptr) ? nullptr : contentStorage->GetNapiValue();
    WMError ret;
    if (isLoadedByName) {
        ret = windowImpl->NapiSetUIContentByName(contextUrl, env, nativeStorage,
            BackupAndRestoreType::NONE, parentToken);
    } else {
        ret = windowImpl->NapiSetUIContent(contextUrl, env, nativeStorage, BackupAndRestoreType::NONE, parentToken);
    }
    if (ret == WMError::WM_OK) {
        task.Resolve(env, NapiGetUndefined(env));
    } else {
        task.Reject(env, CreateJsError(env, static_cast<int32_t>(ret), "Window load content failed"));
    }
    TLOGI(WmsLogTag::WMS_UIEXT, "Window [%{public}u, %{public}s] end, ret=%{public}d",
        windowImpl->GetWindowId(), windowImpl->GetWindowName().c_str(), ret);
}

napi_value JsEmbeddableWindowStage::OnLoadContent(napi_env env, napi_callback_info info, bool isLoadedByName)
{
    WmErrorCode errCode = WmErrorCode::WM_OK;
    std::string contextUrl;
    size_t argc = 4;
    napi_value argv[4] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (!ConvertFromJsValue(env, argv[0], contextUrl)) {
        TLOGE(WmsLogTag::WMS_UIEXT, "Failed to convert parameter to context url");
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
        TLOGE(WmsLogTag::WMS_UIEXT, "Window scene is null or get invalid param");
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
    std::shared_ptr<NapiAsyncTask> napiAsyncTask = CreateEmptyAsyncTask(env, callBack, &result);
    auto asyncTask = [window = windowExtensionSessionImpl_, contentStorage, contextUrl, parentToken, isLoadedByName,
        env, task = napiAsyncTask] {
        if (window == nullptr) {
            task->Reject(env, CreateJsError(env, static_cast<int32_t>(WmErrorCode::WM_ERROR_STATE_ABNORMALLY)));
            TLOGNE(WmsLogTag::WMS_UIEXT, "Get windowExtensionSessionImpl failed");
            return;
        }
        LoadContentTask(contentStorage, contextUrl, window, env, *task, parentToken, isLoadedByName);
    };
    if (napi_send_event(env, asyncTask, napi_eprio_high, "OnLoadContent") != napi_status::napi_ok) {
        napiAsyncTask->Reject(env, CreateJsError(env,
            static_cast<int32_t>(WmErrorCode::WM_ERROR_STATE_ABNORMALLY), "send event failed"));
    }
    return result;
}

napi_value JsEmbeddableWindowStage::OnCreateSubWindowWithOptions(napi_env env, napi_callback_info info)
{
    if (windowExtensionSessionImpl_ == nullptr) {
        TLOGE(WmsLogTag::WMS_UIEXT, "extensionWindow is null");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WmErrorCode::WM_ERROR_STATE_ABNORMALLY)));
        return NapiGetUndefined(env);
    }
    size_t argc = 4;
    napi_value argv[4] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    std::string windowName;
    if (!ConvertFromJsValue(env, argv[0], windowName)) {
        TLOGE(WmsLogTag::WMS_UIEXT, "Failed to convert parameter to windowName");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WmErrorCode::WM_ERROR_INVALID_PARAM)));
        return NapiGetUndefined(env);
    }
    sptr<WindowOption> option = new WindowOption();
    if (!ParseSubWindowOptions(env, argv[1], option)) {
        TLOGE(WmsLogTag::WMS_UIEXT, "Get invalid options param");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WmErrorCode::WM_ERROR_INVALID_PARAM)));
        return NapiGetUndefined(env);
    }
    if ((option->GetWindowFlags() & static_cast<uint32_t>(WindowFlag::WINDOW_FLAG_IS_APPLICATION_MODAL)) &&
        !windowExtensionSessionImpl_->IsPcOrPadFreeMultiWindowMode()) {
        TLOGE(WmsLogTag::WMS_SUB, "device not support");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WmErrorCode::WM_ERROR_DEVICE_NOT_SUPPORT)));
        return NapiGetUndefined(env);
    }

    if (option->GetWindowTopmost() && !Permission::IsSystemCalling() && !Permission::IsStartByHdcd()) {
        TLOGE(WmsLogTag::WMS_SUB, "Modal subwindow has topmost, but no system permission");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WmErrorCode::WM_ERROR_NOT_SYSTEM_APP)));
        return NapiGetUndefined(env);
    }
    option->SetParentId(sessionInfo_->hostWindowId);
    const char* const where = __func__;
    napi_value lastParam = (argv[2] != nullptr && GetType(env, argv[2]) == napi_function) ? argv[2] : nullptr;
    napi_value result = nullptr;
    std::shared_ptr<NapiAsyncTask> napiAsyncTask = CreateEmptyAsyncTask(env, lastParam, &result);
    auto asyncTask = [where, extensionWindow = windowExtensionSessionImpl_, windowName = std::move(windowName),
        windowOption = option, env, task = napiAsyncTask]() mutable {
        if (extensionWindow == nullptr) {
            task->Reject(env, CreateJsError(env,
                static_cast<int32_t>(WmErrorCode::WM_ERROR_STATE_ABNORMALLY), "extension's window is null"));
            return;
        }
        windowOption->SetWindowType(WindowType::WINDOW_TYPE_APP_SUB_WINDOW);
        windowOption->SetWindowMode(WindowMode::WINDOW_MODE_FLOATING);
        windowOption->SetOnlySupportSceneBoard(true);
        windowOption->SetIsUIExtFirstSubWindow(true);
        auto window = Window::Create(windowName, windowOption, extensionWindow->GetContext());
        if (window == nullptr) {
            TLOGNE(WmsLogTag::WMS_SUB, "%{public}s Create window failed", where);
            task->Reject(env, CreateJsError(env, static_cast<int32_t>(WmErrorCode::WM_ERROR_STATE_ABNORMALLY),
                "create sub window failed"));
            return;
        }
        if (!window->IsTopmost()) {
            extensionWindow->NotifyModalUIExtensionMayBeCovered(false);
        }
        task->Resolve(env, CreateJsWindowObject(env, window));
        TLOGNI(WmsLogTag::WMS_SUB, "%{public}s Create sub window %{public}s end",
            where, windowName.c_str());
    };
    if (napi_send_event(env, asyncTask, napi_eprio_high, "OnCreateSubWindowWithOptions") != napi_status::napi_ok) {
        napiAsyncTask->Reject(env, CreateJsError(env,
            static_cast<int32_t>(WmErrorCode::WM_ERROR_STATE_ABNORMALLY), "send event failed"));
    }
    return result;
}

napi_value JsEmbeddableWindowStage::OnSetUIContent(napi_env env, napi_callback_info info)
{
    size_t argc = 4;
    napi_value argv[4] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc < 2) { // 2: minimum param num
        TLOGE(WmsLogTag::WMS_UIEXT, "Argc is invalid: %{public}zu", argc);
        return NapiGetUndefined(env);
    }

    // Parse info->argv[0] as abilitycontext
    auto objContext = argv[0];
    if (objContext == nullptr) {
        TLOGE(WmsLogTag::WMS_UIEXT, "Context is nullptr");
        return NapiGetUndefined(env);
    }

    // Parse info->argv[1] as url
    std::string contextUrl;
    if (!ConvertFromJsValue(env, argv[1], contextUrl)) {
        TLOGE(WmsLogTag::WMS_UIEXT, "Failed to convert parameter to url");
        return NapiGetUndefined(env);
    }

    if (windowExtensionSessionImpl_ == nullptr) {
        TLOGE(WmsLogTag::WMS_UIEXT, "extensionWindow is null");
        return NapiGetUndefined(env);
    }
    windowExtensionSessionImpl_->NapiSetUIContent(contextUrl, env, argv[CONTENT_STORAGE_ARG]);
    return NapiGetUndefined(env);
}

napi_value JsEmbeddableWindowStage::OnEmptyAsyncCall(napi_env env, napi_callback_info info)
{
    size_t argc = 4;
    napi_value argv[4] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    napi_value lastParam = (argc <= 0) ? nullptr :
        ((argv[argc - 1] != nullptr && GetType(env, argv[argc - 1]) == napi_function) ? argv[argc - 1] : nullptr);
    napi_value result = nullptr;
    std::shared_ptr<NapiAsyncTask> napiAsyncTask = CreateEmptyAsyncTask(env, lastParam, &result);
    auto asyncTask = [env, task = napiAsyncTask] {
        task->Resolve(env, NapiGetUndefined(env));
    };
    if (napi_send_event(env, asyncTask, napi_eprio_high, "OnEmptyAsyncCall") != napi_status::napi_ok) {
        napiAsyncTask->Reject(env,
            CreateJsError(env, static_cast<int32_t>(WmErrorCode::WM_ERROR_STATE_ABNORMALLY), "failed to send event"));
    }
    return result;
}

napi_value JsEmbeddableWindowStage::OnUnsupportAsyncCall(napi_env env, napi_callback_info info)
{
    size_t argc = 4;
    napi_value argv[4] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    napi_value lastParam = (argc <= 0) ? nullptr :
        ((argv[argc - 1] != nullptr && GetType(env, argv[argc - 1]) == napi_function) ? argv[argc - 1] : nullptr);
    napi_value result = nullptr;
    std::shared_ptr<NapiAsyncTask> napiAsyncTask = CreateEmptyAsyncTask(env, lastParam, &result);
    auto asyncTask = [env, task = napiAsyncTask] {
        task->Reject(env, CreateJsError(env, static_cast<int32_t>(WmErrorCode::WM_ERROR_DEVICE_NOT_SUPPORT)));
    };
    if (napi_send_event(env, asyncTask, napi_eprio_high, "OnUnsupportAsyncCall") != napi_status::napi_ok) {
        napiAsyncTask->Reject(env,
            CreateJsError(env, static_cast<int32_t>(WmErrorCode::WM_ERROR_STATE_ABNORMALLY), "failed to send event"));
    }
    return result;
}

napi_value JsEmbeddableWindowStage::CreateJsEmbeddableWindowStage(napi_env env, sptr<Rosen::Window> window,
    sptr<AAFwk::SessionInfo> sessionInfo)
{
    TLOGD(WmsLogTag::WMS_UIEXT, "[NAPI]");
    napi_value objValue = nullptr;
    napi_create_object(env, &objValue);

    std::unique_ptr<JsEmbeddableWindowStage> jsEmbeddableWindowStage = std::make_unique<JsEmbeddableWindowStage>(window,
        sessionInfo);
    napi_wrap(env, objValue, jsEmbeddableWindowStage.release(), JsEmbeddableWindowStage::Finalizer, nullptr, nullptr);

    const char *moduleName = "JsEmbeddableWindowStage";
    BindNativeFunction(env,
        objValue, "loadContent", moduleName, JsEmbeddableWindowStage::LoadContent);
    BindNativeFunction(env,
        objValue, "loadContentByName", moduleName, JsEmbeddableWindowStage::LoadContentByName);
    BindNativeFunction(env,
        objValue, "getMainWindow", moduleName, JsEmbeddableWindowStage::GetMainWindow);
    BindNativeFunction(env,
        objValue, "getMainWindowSync", moduleName, JsEmbeddableWindowStage::GetMainWindowSync);
    BindNativeFunction(env, objValue, "on", moduleName, JsEmbeddableWindowStage::On);
    BindNativeFunction(env, objValue, "off", moduleName, JsEmbeddableWindowStage::Off);
    BindNativeFunction(env, objValue, "createSubWindow", moduleName, JsEmbeddableWindowStage::EmptyAsyncCall);
    BindNativeFunction(env, objValue, "getSubWindow", moduleName, JsEmbeddableWindowStage::EmptyAsyncCall);
    BindNativeFunction(env, objValue, "createSubWindowWithOptions", moduleName,
        JsEmbeddableWindowStage::CreateSubWindowWithOptions);
    BindNativeFunction(env, objValue, "setUIContent", moduleName, JsEmbeddableWindowStage::SetUIContent);

    BindNativeFunction(env, objValue, "setDefaultDensityEnabled", moduleName, JsEmbeddableWindowStage::EmptySyncCall);
    BindNativeFunction(env, objValue, "setCustomDensity", moduleName, JsEmbeddableWindowStage::EmptySyncCall);
    BindNativeFunction(env, objValue, "removeStartingWindow", moduleName, JsEmbeddableWindowStage::EmptyAsyncCall);

    BindNativeFunction(env, objValue, "setWindowModal", moduleName, JsEmbeddableWindowStage::UnsupportAsyncCall);
    BindNativeFunction(env, objValue, "setWindowRectAutoSave", moduleName, JsEmbeddableWindowStage::UnsupportAsyncCall);
    BindNativeFunction(env, objValue, "isWindowRectAutoSave", moduleName, JsEmbeddableWindowStage::UnsupportAsyncCall);
    BindNativeFunction(env, objValue, "setSupportedWindowModes", moduleName,
        JsEmbeddableWindowStage::UnsupportAsyncCall);

    return objValue;
}
}  // namespace Rosen
}  // namespace OHOS
