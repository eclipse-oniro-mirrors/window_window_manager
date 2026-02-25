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

#include "js_err_utils.h"
#include "js_window_manager.h"
#include "js_window.h"
#include "js_window_utils.h"
#include "window_helper.h"
#include "window_manager_hilog.h"
#include "window_option.h"
namespace OHOS {
namespace Rosen {
using namespace AbilityRuntime;
namespace {
constexpr HiviewDFX::HiLogLabel LABEL = {LOG_CORE, HILOG_DOMAIN_WINDOW, "JsWindowManager"};
const std::string PIP_WINDOW = "pip_window";
}

JsWindowManager::JsWindowManager() : registerManager_(std::make_unique<JsWindowRegisterManager>())
{
}

JsWindowManager::~JsWindowManager()
{
}

void JsWindowManager::Finalizer(napi_env env, void* data, void* hint)
{
    TLOGD(WmsLogTag::DEFAULT, "Enter");
    std::unique_ptr<JsWindowManager>(static_cast<JsWindowManager*>(data));
}

napi_value JsWindowManager::Create(napi_env env, napi_callback_info info)
{
    WLOGI("mock: Create");
    return nullptr;
}

napi_value JsWindowManager::CreateWindowNapi(napi_env env, napi_callback_info info)
{
    WLOGI("mock: CreateWindowNapi");
    return nullptr;
}

napi_value JsWindowManager::FindWindow(napi_env env, napi_callback_info info)
{
    WLOGI("mock: FindWindow");
    return nullptr;
}

napi_value JsWindowManager::FindWindowSync(napi_env env, napi_callback_info info)
{
    WLOGI("mock: FindWindowSync");
    return nullptr;
}

napi_value JsWindowManager::RegisterWindowManagerCallback(napi_env env, napi_callback_info info)
{
    WLOGI("mock: RegisterWindowManagerCallback");
    return nullptr;
}

napi_value JsWindowManager::UnregisterWindowMangerCallback(napi_env env, napi_callback_info info)
{
    WLOGI("mock: UnregisterWindowMangerCallback");
    return nullptr;
}

napi_value JsWindowManager::GetTopWindow(napi_env env, napi_callback_info info)
{
    WLOGI("mock: GetTopWindow");
    return nullptr;
}

/** @note @window.hierarchy */
napi_value JsWindowManager::GetLastWindow(napi_env env, napi_callback_info info)
{
    WLOGI("GetLastWindow");
    JsWindowManager* me = CheckParamsAndGetThis<JsWindowManager>(env, info);
    return (me != nullptr) ? me->OnGetLastWindow(env, info) : nullptr;
}

napi_value JsWindowManager::MinimizeAll(napi_env env, napi_callback_info info)
{
    WLOGI("mock: MinimizeAll");
    return nullptr;
}

napi_value JsWindowManager::MinimizeAllWithExclusion(napi_env env, napi_callback_info info)
{
    TLOGI(WmsLogTag::WMS_ATTRIBUTE, "mock: MinimizeAllWithExclusion");
    return nullptr;
}

napi_value JsWindowManager::ToggleShownStateForAllAppWindows(napi_env env, napi_callback_info info)
{
    WLOGI("mock: ToggleShownStateForAllAppWindows");
    return nullptr;
}

napi_value JsWindowManager::GetAllWindowLayoutInfo(napi_env env, napi_callback_info info)
{
    TLOGI(WmsLogTag::WMS_ATTRIBUTE, "mock");
    return nullptr;
}

napi_value JsWindowManager::GetGlobalWindowMode(napi_env env, napi_callback_info info)
{
    TLOGI(WmsLogTag::WMS_ATTRIBUTE, "mock");
    return nullptr;
}

napi_value JsWindowManager::SetWindowLayoutMode(napi_env env, napi_callback_info info)
{
    WLOGI("mock: SetWindowLayoutMode");
    return nullptr;
}

napi_value JsWindowManager::SetGestureNavigationEnabled(napi_env env, napi_callback_info info)
{
    WLOGI("mock: SetGestureNavigationEnabled");
    return nullptr;
}

napi_value JsWindowManager::SetWaterMarkImage(napi_env env, napi_callback_info info)
{
    WLOGI("mock: SetWaterMarkImage");
    return nullptr;
}

napi_value JsWindowManager::ShiftAppWindowFocus(napi_env env, napi_callback_info info)
{
    WLOGI("mock: ShiftAppWindowFocus");
    return nullptr;
}

napi_value JsWindowManager::NotifyScreenshotEvent(napi_env env, napi_callback_info info)
{
    TLOGI(WmsLogTag::WMS_ATTRIBUTE, "mock");
    return nullptr;
}

static void GetNativeContext(napi_env env, napi_value nativeContext, void*& contextPtr, WMError& errCode)
{
    AppExecFwk::Ability* ability = nullptr;
    bool isOldApi = GetAPI7Ability(env, ability);
    WLOGFD("FA mode:%{public}u", isOldApi);
    if (isOldApi) {
        return;
    }
    if (nativeContext != nullptr) {
        napi_unwrap(env, nativeContext, &contextPtr);
    }
}

static napi_value GetTopWindowTask(void* contextPtr, napi_env env, napi_value callback, bool newApi)
{
    struct TopWindowInfoList {
        sptr<Window> window = nullptr;
    };
    std::shared_ptr<TopWindowInfoList> lists = std::make_shared<TopWindowInfoList>();
    NapiAsyncTask::ExecuteCallback execute = [lists, newApi, contextPtr]() {
        if (lists == nullptr) {
            WLOGFE("lists == nullptr");
            return;
        }
        lists->window = Window::GetTopWindowWithId(0);
    };
    NapiAsyncTask::CompleteCallback complete = [lists, newApi](napi_env env, NapiAsyncTask& task, int32_t status) {
        if (lists == nullptr) {
            if (newApi) {
                task.Reject(env, JsErrUtils::CreateJsError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
                    "napi abnormal"));
            } else {
                task.Reject(env, JsErrUtils::CreateJsError(env, WMError::WM_ERROR_NULLPTR, "napi abnormal"));
            }
            return;
        }
        if (lists->window == nullptr) {
            if (newApi) {
                task.Reject(env, JsErrUtils::CreateJsError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
                    "Get top window failed"));
            } else {
                task.Reject(env, JsErrUtils::CreateJsError(env, WMError::WM_ERROR_NULLPTR, "Get top window failed"));
            }
            return;
        }
        task.Resolve(env, CreateJsWindowObject(env, lists->window));
        WLOGD("Get top window success");
    };
    napi_value result = nullptr;
    NapiAsyncTask::Schedule("JsWindowManager::OnGetTopWindow",
        env, CreateAsyncTaskWithLastParam(env, callback, std::move(execute), std::move(complete), &result));
    return result;
}

napi_value JsWindowManager::OnGetLastWindow(napi_env env, napi_callback_info info)
{
    WMError errCode = WMError::WM_OK;
    napi_value nativeContext = nullptr;
    napi_value nativeCallback = nullptr;
    void* contextPtr = nullptr;
    size_t argc = 4;
    napi_value argv[4] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc < 1) {
        WLOGFE("Argc is invalid: %{public}zu", argc);
        napi_throw(env, JsErrUtils::CreateJsError(env, WmErrorCode::WM_ERROR_INVALID_PARAM));
        return NapiGetUndefined(env);
    } else {
        nativeContext = argv[0];
        nativeCallback = (argc == 1) ? nullptr : argv[1];
        GetNativeContext(env, nativeContext, contextPtr, errCode);
    }
    if (errCode != WMError::WM_OK) {
        napi_throw(env, JsErrUtils::CreateJsError(env, WmErrorCode::WM_ERROR_INVALID_PARAM));
        return NapiGetUndefined(env);
    }

    return GetTopWindowTask(contextPtr, env, nativeCallback, true);
}

napi_value JsWindowManagerInit(napi_env env, napi_value exportObj)
{
    if (env == nullptr || exportObj == nullptr) {
        WLOGFE("JsWindowManagerInit env or exportObj is nullptr");
        return nullptr;
    }

    std::unique_ptr<JsWindowManager> jsWinManager = std::make_unique<JsWindowManager>();
    napi_wrap(env, exportObj, jsWinManager.release(), JsWindowManager::Finalizer, nullptr, nullptr);
    napi_set_named_property(env, exportObj, "WindowType", WindowTypeInit(env));
    napi_set_named_property(env, exportObj, "AvoidAreaType", AvoidAreaTypeInit(env));
    napi_set_named_property(env, exportObj, "WindowMode", WindowModeInit(env));
    napi_set_named_property(env, exportObj, "ColorSpace", ColorSpaceInit(env));
    napi_set_named_property(env, exportObj, "WindowStageEventType", WindowStageEventTypeInit(env));
    napi_set_named_property(env, exportObj, "WindowAnchor", WindowAnchorInit(env));
    napi_set_named_property(env, exportObj, "PixelUnit", PixelUnitInit(env));
    napi_set_named_property(env, exportObj, "WindowEventType", WindowEventTypeInit(env));
    napi_set_named_property(env, exportObj, "WindowLayoutMode", WindowLayoutModeInit(env));
    napi_set_named_property(env, exportObj, "Orientation", OrientationInit(env));
    napi_set_named_property(env, exportObj, "BlurStyle", BlurStyleInit(env));
    napi_set_named_property(env, exportObj, "WmErrorCode", WindowErrorCodeInit(env));
    napi_set_named_property(env, exportObj, "WMError", WindowErrorInit(env));
    napi_set_named_property(env, exportObj, "WindowStatusType", WindowStatusTypeInit(env));
    napi_set_named_property(env, exportObj, "MaximizePresentation", MaximizePresentationInit(env));
    const char* moduleName = "JsWindowManager";
    BindNativeFunction(env, exportObj, "create", moduleName, JsWindowManager::Create);
    BindNativeFunction(env, exportObj, "createWindow", moduleName, JsWindowManager::CreateWindowNapi);
    BindNativeFunction(env, exportObj, "find", moduleName, JsWindowManager::FindWindow);
    BindNativeFunction(env, exportObj, "findWindow", moduleName, JsWindowManager::FindWindowSync);
    BindNativeFunction(env, exportObj, "on", moduleName, JsWindowManager::RegisterWindowManagerCallback);
    BindNativeFunction(env, exportObj, "off", moduleName, JsWindowManager::UnregisterWindowMangerCallback);
    BindNativeFunction(env, exportObj, "getTopWindow", moduleName, JsWindowManager::GetTopWindow);
    BindNativeFunction(env, exportObj, "getLastWindow", moduleName, JsWindowManager::GetLastWindow);
    BindNativeFunction(env, exportObj, "minimizeAll", moduleName, JsWindowManager::MinimizeAll);
    BindNativeFunction(env, exportObj, "minimizeAllWithExclusion", moduleName,
        JsWindowManager::MinimizeAllWithExclusion);
    BindNativeFunction(env, exportObj, "toggleShownStateForAllAppWindows", moduleName,
        JsWindowManager::ToggleShownStateForAllAppWindows);
    BindNativeFunction(env, exportObj, "getAllWindowLayoutInfo", moduleName, JsWindowManager::GetAllWindowLayoutInfo);
    BindNativeFunction(env, exportObj, "getGlobalWindowMode", moduleName, JsWindowManager::GetGlobalWindowMode);
    BindNativeFunction(env, exportObj, "setWindowLayoutMode", moduleName, JsWindowManager::SetWindowLayoutMode);
    BindNativeFunction(env, exportObj, "setGestureNavigationEnabled", moduleName,
        JsWindowManager::SetGestureNavigationEnabled);
    BindNativeFunction(env, exportObj, "setWaterMarkImage", moduleName, JsWindowManager::SetWaterMarkImage);
    BindNativeFunction(env, exportObj, "shiftAppWindowFocus", moduleName, JsWindowManager::ShiftAppWindowFocus);
    BindNativeFunction(env, exportObj, "notifyScreenshotEvent", moduleName, JsWindowManager::NotifyScreenshotEvent);
    return NapiGetUndefined(env);
}
}  // namespace Rosen
}  // namespace OHOS
