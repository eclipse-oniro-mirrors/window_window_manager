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
#include "js_window_manager.h"
#include <ability.h>
#include <cinttypes>
#include <hitrace_meter.h>
#include <new>
#include <transaction/rs_interfaces.h>
#include "ability_context.h"
#include "display_manager.h"
#include "dm_common.h"
#include "wm_common.h"
#include "js_window.h"
#include "js_window_utils.h"
#include "window_helper.h"
#include "window_manager_hilog.h"
#include "window_option.h"
#include "pixel_map_napi.h"
#include "permission.h"
#include "singleton_container.h"

namespace OHOS {
namespace Rosen {
using namespace AbilityRuntime;
namespace {
constexpr HiviewDFX::HiLogLabel LABEL = {LOG_CORE, HILOG_DOMAIN_WINDOW, "JsWindowManager"};
const std::string PIP_WINDOW = "pip_window";
constexpr size_t INDEX_ZERO = 0;
constexpr size_t INDEX_ONE = 1;
constexpr size_t ARGC_ONE = 1;
constexpr size_t ARGC_TWO = 2;
constexpr size_t ARGC_THREE = 3;
constexpr size_t ARGC_FOUR = 4;
constexpr int32_t INVALID_COORDINATE = -1;
}

JsWindowManager::JsWindowManager() : registerManager_(std::make_unique<JsWindowRegisterManager>())
{
}

JsWindowManager::~JsWindowManager()
{
}

void JsWindowManager::Finalizer(napi_env env, void* data, void* hint)
{
    WLOGI("Finalizer");
    std::unique_ptr<JsWindowManager>(static_cast<JsWindowManager*>(data));
}

napi_value JsWindowManager::Create(napi_env env, napi_callback_info info)
{
    JsWindowManager* me = CheckParamsAndGetThis<JsWindowManager>(env, info);
    return (me != nullptr) ? me->OnCreate(env, info) : nullptr;
}

napi_value JsWindowManager::CreateWindow(napi_env env, napi_callback_info info)
{
    JsWindowManager* me = CheckParamsAndGetThis<JsWindowManager>(env, info);
    return (me != nullptr) ? me->OnCreateWindow(env, info) : nullptr;
}

napi_value JsWindowManager::FindWindow(napi_env env, napi_callback_info info)
{
    JsWindowManager* me = CheckParamsAndGetThis<JsWindowManager>(env, info);
    return (me != nullptr) ? me->OnFindWindow(env, info) : nullptr;
}

napi_value JsWindowManager::FindWindowSync(napi_env env, napi_callback_info info)
{
    JsWindowManager* me = CheckParamsAndGetThis<JsWindowManager>(env, info);
    return (me != nullptr) ? me->OnFindWindowSync(env, info) : nullptr;
}

napi_value JsWindowManager::MinimizeAll(napi_env env, napi_callback_info info)
{
    JsWindowManager* me = CheckParamsAndGetThis<JsWindowManager>(env, info);
    return (me != nullptr) ? me->OnMinimizeAll(env, info) : nullptr;
}

napi_value JsWindowManager::ToggleShownStateForAllAppWindows(napi_env env, napi_callback_info info)
{
    JsWindowManager* me = CheckParamsAndGetThis<JsWindowManager>(env, info);
    return (me != nullptr) ? me->OnToggleShownStateForAllAppWindows(env, info) : nullptr;
}

napi_value JsWindowManager::RegisterWindowManagerCallback(napi_env env, napi_callback_info info)
{
    JsWindowManager* me = CheckParamsAndGetThis<JsWindowManager>(env, info);
    return (me != nullptr) ? me->OnRegisterWindowManagerCallback(env, info) : nullptr;
}

napi_value JsWindowManager::UnregisterWindowMangerCallback(napi_env env, napi_callback_info info)
{
    JsWindowManager* me = CheckParamsAndGetThis<JsWindowManager>(env, info);
    return (me != nullptr) ? me->OnUnregisterWindowManagerCallback(env, info) : nullptr;
}

napi_value JsWindowManager::GetTopWindow(napi_env env, napi_callback_info info)
{
    JsWindowManager* me = CheckParamsAndGetThis<JsWindowManager>(env, info);
    return (me != nullptr) ? me->OnGetTopWindow(env, info) : nullptr;
}

/** @note @window.hierarchy */
napi_value JsWindowManager::GetLastWindow(napi_env env, napi_callback_info info)
{
    JsWindowManager* me = CheckParamsAndGetThis<JsWindowManager>(env, info);
    return (me != nullptr) ? me->OnGetLastWindow(env, info) : nullptr;
}

napi_value JsWindowManager::GetSnapshot(napi_env env, napi_callback_info info)
{
    JsWindowManager* me = CheckParamsAndGetThis<JsWindowManager>(env, info);
    return (me != nullptr) ? me->OnGetSnapshot(env, info) : nullptr;
}

/** @note @window.layout */
napi_value JsWindowManager::SetWindowLayoutMode(napi_env env, napi_callback_info info)
{
    JsWindowManager* me = CheckParamsAndGetThis<JsWindowManager>(env, info);
    return (me != nullptr) ? me->OnSetWindowLayoutMode(env, info) : nullptr;
}

napi_value JsWindowManager::SetGestureNavigationEnabled(napi_env env, napi_callback_info info)
{
    JsWindowManager* me = CheckParamsAndGetThis<JsWindowManager>(env, info);
    return (me != nullptr) ? me->OnSetGestureNavigationEnabled(env, info) : nullptr;
}

napi_value JsWindowManager::SetWaterMarkImage(napi_env env, napi_callback_info info)
{
    JsWindowManager* me = CheckParamsAndGetThis<JsWindowManager>(env, info);
    return (me != nullptr) ? me->OnSetWaterMarkImage(env, info) : nullptr;
}

napi_value JsWindowManager::ShiftAppWindowFocus(napi_env env, napi_callback_info info)
{
    JsWindowManager* me = CheckParamsAndGetThis<JsWindowManager>(env, info);
    return (me != nullptr) ? me->OnShiftAppWindowFocus(env, info) : nullptr;
}

napi_value JsWindowManager::GetAllWindowLayoutInfo(napi_env env, napi_callback_info info)
{
    JsWindowManager* me = CheckParamsAndGetThis<JsWindowManager>(env, info);
    return (me != nullptr) ? me->OnGetAllWindowLayoutInfo(env, info) : nullptr;
}

napi_value JsWindowManager::GetGlobalWindowMode(napi_env env, napi_callback_info info)
{
    TLOGI(WmsLogTag::WMS_ATTRIBUTE, "[NAPI]");
    JsWindowManager* me = CheckParamsAndGetThis<JsWindowManager>(env, info);
    return (me != nullptr) ? me->OnGetGlobalWindowMode(env, info) : nullptr;
}

napi_value JsWindowManager::GetVisibleWindowInfo(napi_env env, napi_callback_info info)
{
    JsWindowManager* me = CheckParamsAndGetThis<JsWindowManager>(env, info);
    return (me != nullptr) ? me->OnGetVisibleWindowInfo(env, info) : nullptr;
}

napi_value JsWindowManager::GetWindowsByCoordinate(napi_env env, napi_callback_info info)
{
    JsWindowManager* me = CheckParamsAndGetThis<JsWindowManager>(env, info);
    return (me != nullptr) ? me->OnGetWindowsByCoordinate(env, info) : nullptr;
}

napi_value JsWindowManager::ShiftAppWindowPointerEvent(napi_env env, napi_callback_info info)
{
    JsWindowManager* me = CheckParamsAndGetThis<JsWindowManager>(env, info);
    return (me != nullptr) ? me->OnShiftAppWindowPointerEvent(env, info) : nullptr;
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

static uint32_t GetParentId(napi_env env)
{
    AppExecFwk::Ability* ability = nullptr;
    uint32_t parentId = 0;
    bool isOldApi = GetAPI7Ability(env, ability);
    if (isOldApi) {
        if (ability == nullptr) {
            WLOGE("FA mode GetAPI7Ability failed");
            return parentId;
        }
        auto window = ability->GetWindow();
        if (window == nullptr) {
            WLOGE("Get mainWindow failed");
            return parentId;
        }
        parentId = window->GetWindowId();
    }
    return parentId;
}

static bool GetWindowTypeAndParentId(napi_env env, uint32_t& parentId, WindowType& winType,
    napi_value nativeString, napi_value nativeType)
{
    napi_value type = nativeType;
    if (type == nullptr) {
        WLOGFE("Failed to convert parameter to windowType");
        return false;
    }
    uint32_t resultValue = 0;
    napi_get_value_uint32(env, type, &resultValue);
    if (resultValue >= static_cast<uint32_t>(ApiWindowType::TYPE_BASE) &&
        resultValue < static_cast<uint32_t>(ApiWindowType::TYPE_END)) {
        winType = JS_TO_NATIVE_WINDOW_TYPE_MAP.at(static_cast<ApiWindowType>(resultValue));
    } else {
        WLOGFE("Type %{public}u is not supported", resultValue);
        return false;
    }

    AppExecFwk::Ability* ability = nullptr;
    bool isOldApi = GetAPI7Ability(env, ability);
    if (isOldApi) {
        if (ability == nullptr || !WindowHelper::IsSubWindow(winType)) {
            WLOGE("FA mode GetAPI7Ability failed or type %{public}u is not subWinodw", winType);
            return false;
        }
        auto window = ability->GetWindow();
        if (window == nullptr) {
            WLOGE("Get mainWindow failed");
            return false;
        }
        parentId = window->GetWindowId();
    } else {
        if (!WindowHelper::IsSystemWindow(winType)) {
            WLOGFE("Only SystemWindow support create in stage mode, type is %{public}u", winType);
            return false;
        }
    }
    return true;
}

static void CreateNewSystemWindowTask(void* contextPtr, sptr<WindowOption> windowOption,
    napi_env env, NapiAsyncTask& task)
{
    WLOGFI("[NAPI]");
    if (windowOption == nullptr) {
        task.Reject(env, JsErrUtils::CreateJsError(env, WmErrorCode::WM_ERROR_SYSTEM_ABNORMALLY,
            "New window option failed"));
        WLOGFE("New window option failed");
        return;
    }
    auto context = static_cast<std::weak_ptr<AbilityRuntime::Context>*>(contextPtr);
    if (contextPtr == nullptr || context == nullptr) {
        task.Reject(env, JsErrUtils::CreateJsError(env, WmErrorCode::WM_ERROR_CONTEXT_ABNORMALLY,
            "Context is nullptr"));
        WLOGFE("Context is nullptr");
        return;
    }
    if (windowOption->GetWindowType() == WindowType::WINDOW_TYPE_FLOAT ||
        windowOption->GetWindowType() == WindowType::WINDOW_TYPE_FLOAT_CAMERA) {
        auto abilityContext = Context::ConvertTo<AbilityRuntime::AbilityContext>(context->lock());
        if (abilityContext != nullptr) {
            if (!CheckCallingPermission("ohos.permission.SYSTEM_FLOAT_WINDOW")) {
                task.Reject(env, JsErrUtils::CreateJsError(env, WmErrorCode::WM_ERROR_NO_PERMISSION,
                    "TYPE_FLOAT CheckCallingPermission failed"));
                return;
            }
        }
    }
    WMError wmError = WMError::WM_OK;
    sptr<Window> window = Window::Create(windowOption->GetWindowName(), windowOption, context->lock(), wmError);
    WmErrorCode wmErrorCode = WM_JS_TO_ERROR_CODE_MAP.at(wmError);
    if (window != nullptr && wmErrorCode == WmErrorCode::WM_OK) {
        task.Resolve(env, CreateJsWindowObject(env, window));
    } else {
        WLOGFE("Create window failed");
        task.Reject(env, JsErrUtils::CreateJsError(env, wmErrorCode, "Create window failed"));
    }
}

static void CreateSystemWindowTask(void* contextPtr, std::string windowName, WindowType winType,
    napi_env env, NapiAsyncTask& task)
{
    WLOGFD("[NAPI]");
    auto context = static_cast<std::weak_ptr<AbilityRuntime::Context>*>(contextPtr);
    if (contextPtr == nullptr || context == nullptr) {
        task.Reject(env, JsErrUtils::CreateJsError(env, WMError::WM_ERROR_NULLPTR, "Context is nullptr"));
        WLOGFE("Context is nullptr");
        return;
    }
    if (winType == WindowType::WINDOW_TYPE_FLOAT || winType == WindowType::WINDOW_TYPE_FLOAT_CAMERA) {
        auto abilityContext = Context::ConvertTo<AbilityRuntime::AbilityContext>(context->lock());
        if (abilityContext != nullptr) {
            if (!CheckCallingPermission("ohos.permission.SYSTEM_FLOAT_WINDOW")) {
                task.Reject(env, JsErrUtils::CreateJsError(env, WMError::WM_ERROR_INVALID_PERMISSION,
                    "TYPE_FLOAT CheckCallingPermission failed"));
                return;
            }
        }
    }
    sptr<WindowOption> windowOption = new(std::nothrow) WindowOption();
    if (windowOption == nullptr) {
        task.Reject(env, JsErrUtils::CreateJsError(env, WMError::WM_ERROR_NULLPTR, "New window option failed"));
        WLOGFE("New window option failed");
        return;
    }
    windowOption->SetWindowType(winType);
    WMError wmError = WMError::WM_OK;
    sptr<Window> window = Window::Create(windowName, windowOption, context->lock(), wmError);
    WmErrorCode wmErrorCode = WM_JS_TO_ERROR_CODE_MAP.at(wmError);
    if (window != nullptr && wmErrorCode == WmErrorCode::WM_OK) {
        task.Resolve(env, CreateJsWindowObject(env, window));
    } else {
        WLOGFE("Create window failed");
        task.Reject(env, JsErrUtils::CreateJsError(env, wmErrorCode, "Create window failed"));
    }
}

static void CreateNewSubWindowTask(sptr<WindowOption> windowOption, napi_env env, NapiAsyncTask& task)
{
    if (windowOption == nullptr) {
        task.Reject(env, JsErrUtils::CreateJsError(env, WmErrorCode::WM_ERROR_SYSTEM_ABNORMALLY,
            "New window option failed"));
        WLOGFE("New window option failed");
        return;
    }
    windowOption->SetWindowMode(Rosen::WindowMode::WINDOW_MODE_FLOATING);
    if (windowOption->GetParentId() == INVALID_WINDOW_ID) {
        uint32_t parentId = GetParentId(env);
        if (!parentId) {
            task.Reject(env, JsErrUtils::CreateJsError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
                "parent window missed"));
            WLOGFE("can not find parent window");
            return;
        }
        windowOption->SetParentId(parentId);
    }
    sptr<Window> window = Window::Create(windowOption->GetWindowName(), windowOption);
    if (window != nullptr) {
        task.Resolve(env, CreateJsWindowObject(env, window));
    } else {
        WLOGFE("Create window failed");
        task.Reject(env, JsErrUtils::CreateJsError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
            "Create window failed"));
    }
}

static void CreateSubWindowTask(uint32_t parentWinId, std::string windowName, WindowType winType,
    napi_env env, NapiAsyncTask& task, bool newErrorCode = false)
{
    WLOGFI("parent id=%{public}u", parentWinId);
    sptr<WindowOption> windowOption = new(std::nothrow) WindowOption();
    if (windowOption == nullptr) {
        if (newErrorCode) {
            task.Reject(env, JsErrUtils::CreateJsError(env, WmErrorCode::WM_ERROR_SYSTEM_ABNORMALLY,
                "New window option failed"));
        } else {
            task.Reject(env, JsErrUtils::CreateJsError(env, WMError::WM_ERROR_NULLPTR, "New window option failed"));
        }
        WLOGFE("New window option failed");
        return;
    }
    windowOption->SetWindowType(winType);
    windowOption->SetWindowMode(Rosen::WindowMode::WINDOW_MODE_FLOATING);
    windowOption->SetParentId(parentWinId);
    sptr<Window> window = Window::Create(windowName, windowOption);
    if (window != nullptr) {
        task.Resolve(env, CreateJsWindowObject(env, window));
    } else {
        WLOGFE("Create window failed");
        if (newErrorCode) {
            task.Reject(env, JsErrUtils::CreateJsError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
                "Create window failed"));
        } else {
            task.Reject(env, JsErrUtils::CreateJsError(env, WMError::WM_ERROR_NULLPTR, "Create window failed"));
        }
    }
}

static bool isConfigOptionWindowTypeValid(napi_env env, WindowOption& option)
{
    WindowType type = option.GetWindowType();
    AppExecFwk::Ability* ability = nullptr;
    bool isOldApi = GetAPI7Ability(env, ability);
    if (isOldApi) {
        if (ability == nullptr || !WindowHelper::IsSubWindow(type)) {
            WLOGE("FA mode GetAPI7Ability failed or convert parameter to invalid winType %{public}u", type);
            return false;
        }
    } else {
        if (!WindowHelper::IsSystemWindow(type)) {
            WLOGFE("Stage mode convert parameter to invalid winType %{public}u", type);
            return false;
        }
    }

    return true;
}

napi_value JsWindowManager::OnCreate(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::WMS_LIFE, "[NAPI]");
    napi_value nativeString = nullptr, nativeContext = nullptr, nativeType = nullptr, callback = nullptr;
    size_t argc = 4;
    napi_value argv[4] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc >= 2 && GetType(env, argv[0]) == napi_string) { // 2: minimum params num
        nativeString = argv[0];
        nativeType = argv[1];
        callback = (argc == 2) ? nullptr : (GetType(env, argv[2]) == napi_function ? argv[2] : nullptr); // 2: index
    } else if (argc >= 3) { // 3: minimum params num
        nativeContext = GetType(env, argv[0]) == napi_object ? argv[0] : nullptr;
        nativeString = argv[1];
        nativeType = argv[2]; // 2: index of type
        callback = (argc == 3) ? nullptr : (GetType(env, argv[3]) == napi_function ? argv[3] : nullptr); // 3: index
    }
    std::string windowName;
    WMError errCode = WMError::WM_OK;
    if (!ConvertFromJsValue(env, nativeString, windowName)) {
        TLOGE(WmsLogTag::WMS_LIFE, "Failed to convert parameter to windowName");
        errCode = WMError::WM_ERROR_INVALID_PARAM;
    }
    uint32_t parentId = INVALID_WINDOW_ID;
    WindowType winType = WindowType::SYSTEM_WINDOW_BASE;
    if (errCode == WMError::WM_OK &&
        !GetWindowTypeAndParentId(env, parentId, winType, nativeString, nativeType)) {
        errCode = WMError::WM_ERROR_INVALID_PARAM;
    }
    void* contextPtr = nullptr;
    GetNativeContext(env, nativeContext, contextPtr, errCode);

    TLOGD(WmsLogTag::WMS_LIFE, "Window name=%{public}s, type=%{public}u, err=%{public}d", windowName.c_str(), winType,
        errCode);
    napi_value result = nullptr;
    std::shared_ptr<NapiAsyncTask> napiAsyncTask = CreateEmptyAsyncTask(env, callback, &result);
    auto asyncTask = [errCode, parentId, contextPtr, windowName, winType, env, task = napiAsyncTask] {
        if (errCode != WMError::WM_OK) {
            task->Reject(env, JsErrUtils::CreateJsError(env, errCode, "Invalidate params"));
            return;
        }
        if (parentId == INVALID_WINDOW_ID) {
            return CreateSystemWindowTask(contextPtr, windowName, winType, env, *task);
        } else {
            return CreateSubWindowTask(parentId, windowName, winType, env, *task);
        }
    };
    if (napi_send_event(env, asyncTask, napi_eprio_high) != napi_status::napi_ok) {
        TLOGE(WmsLogTag::WMS_LIFE, "napi send event failed, window state is abnormal");
    }
    return result;
}

bool JsWindowManager::ParseRequiredConfigOption(napi_env env, napi_value jsObject,
    WindowOption& option)
{
    std::string windowName;
    if (ParseJsValue(jsObject, env, "name", windowName)) {
        option.SetWindowName(windowName);
    } else {
        WLOGFE("Failed to convert parameter to windowName");
        return false;
    }

    uint32_t winType;
    if (ParseJsValue(jsObject, env, "windowType", winType)) {
        if (winType >= static_cast<uint32_t>(ApiWindowType::TYPE_BASE) &&
            winType < static_cast<uint32_t>(ApiWindowType::TYPE_END)) {
            option.SetWindowType(JS_TO_NATIVE_WINDOW_TYPE_MAP.at(static_cast<ApiWindowType>(winType)));
        } else {
            TLOGE(WmsLogTag::DEFAULT, "Invalid winType");
            return false;
        }
    } else {
        WLOGFE("Failed to convert parameter to winType");
        return false;
    }
    return true;
}

bool JsWindowManager::ParseConfigOption(napi_env env, napi_value jsObject,
    WindowOption& option, void*& contextPtr)
{
    if (!ParseRequiredConfigOption(env, jsObject, option)) {
        return false;
    }
    if (!isConfigOptionWindowTypeValid(env, option)) {
        return false;
    }
    if (!ParseZIndex(env, jsObject, option)) {
        return false;
    }
    napi_value value = nullptr;
    napi_get_named_property(env, jsObject, "ctx", &value);
    if (GetType(env, value) == napi_undefined) {
        return true;
    }
    WMError errCode = WMError::WM_OK;
    GetNativeContext(env, value, contextPtr, errCode);
    if (errCode != WMError::WM_OK) {
        return false;
    }

    bool dialogDecorEnable = false;
    if (ParseJsValue(jsObject, env, "decorEnabled", dialogDecorEnable)) {
        option.SetDialogDecorEnable(dialogDecorEnable);
    }

    std::string dialogTitle;
    if (ParseJsValue(jsObject, env, "title", dialogTitle)) {
        option.SetDialogTitle(dialogTitle);
    }

    int64_t displayId = static_cast<int64_t>(DISPLAY_ID_INVALID);
    if (ParseJsValue(jsObject, env, "displayId", displayId)) {
        if (displayId < 0 ||
            SingletonContainer::Get<DisplayManager>().GetDisplayById(static_cast<uint64_t>(displayId)) == nullptr) {
            return false;
        }
        option.SetDisplayId(displayId);
    } else {
        return true;
    }

    int64_t parentId = -1;
    if (ParseJsValue(jsObject, env, "parentId", parentId)) {
        option.SetParentId(parentId);
    }

    return true;
}

napi_value JsWindowManager::OnCreateWindow(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::WMS_LIFE, "[NAPI]");
    size_t argc = 4;
    napi_value argv[4] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc < 1) {
        napi_throw(env, JsErrUtils::CreateJsError(env, WmErrorCode::WM_ERROR_INVALID_PARAM));
        return NapiGetUndefined(env);
    }
    napi_value nativeObj = argv[0];
    if (nativeObj == nullptr) {
        TLOGE(WmsLogTag::WMS_LIFE, "Failed to convert object to CreateWindow");
        napi_throw(env, JsErrUtils::CreateJsError(env, WmErrorCode::WM_ERROR_INVALID_PARAM));
        return NapiGetUndefined(env);
    }
    WindowOption option;
    void* contextPtr = nullptr;
    if (!ParseConfigOption(env, nativeObj, option, contextPtr)) {
        TLOGE(WmsLogTag::WMS_LIFE, "Failed to parse config");
        napi_throw(env, JsErrUtils::CreateJsError(env, WmErrorCode::WM_ERROR_INVALID_PARAM));
        return NapiGetUndefined(env);
    }
    napi_value callback = nullptr;
    if (argc > 1) {
        callback = GetType(env, argv[1]) == napi_function ? argv[1] : nullptr; // 1: index of callback
    }
    napi_value result = nullptr;
    std::shared_ptr<NapiAsyncTask> napiAsyncTask = CreateEmptyAsyncTask(env, callback, &result);
    auto asyncTask = [option, contextPtr, env, task = napiAsyncTask] {
        sptr<WindowOption> windowOption = new WindowOption(option);
        if (WindowHelper::IsSystemWindow(option.GetWindowType())) {
            return CreateNewSystemWindowTask(contextPtr, windowOption, env, *task);
        }
        if (WindowHelper::IsSubWindow(option.GetWindowType())) {
            return CreateNewSubWindowTask(windowOption, env, *task);
        }
    };
    if (napi_send_event(env, asyncTask, napi_eprio_high) != napi_status::napi_ok) {
        TLOGE(WmsLogTag::WMS_LIFE, "napi send event failed, window state is abnormal");
    }
    return result;
}

napi_value JsWindowManager::OnGetSnapshot(napi_env env, napi_callback_info info)
{
    constexpr int maxArgumentsNum = 4;
    size_t argc = maxArgumentsNum;
    napi_value argv[maxArgumentsNum] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc != 1) {
        TLOGE(WmsLogTag::WMS_SYSTEM, "Argc is invalid:%{public}zu", argc);
        napi_throw(env, JsErrUtils::CreateJsError(env, WmErrorCode::WM_ERROR_INVALID_PARAM));
        return NapiGetUndefined(env);
    }
    int32_t windowId = 0;
    if (!ConvertFromJsValue(env, argv[0], windowId)) {
        TLOGE(WmsLogTag::WMS_SYSTEM, "Failed to convert parameter to integer");
        napi_throw(env, JsErrUtils::CreateJsError(env, WmErrorCode::WM_ERROR_INVALID_PARAM));
        return NapiGetUndefined(env);
    }
    std::shared_ptr<WindowSnapshotDataPack> dataPack = std::make_shared<WindowSnapshotDataPack>();
    NapiAsyncTask::ExecuteCallback execute = [dataPack, windowId]() {
        dataPack->result = SingletonContainer::Get<WindowManager>()
            .GetSnapshotByWindowId(windowId, dataPack->pixelMap);
    };
    NapiAsyncTask::CompleteCallback complete =
        [=](napi_env env, NapiAsyncTask& task, int32_t status) {
            if (dataPack->result != WMError::WM_OK) {
                task.Reject(env, JsErrUtils::CreateJsError(env, WM_JS_TO_ERROR_CODE_MAP.at(dataPack->result)));
                TLOGNW(WmsLogTag::WMS_SYSTEM, "Get snapshot not ok!");
                return;
            }
            if (dataPack->pixelMap == nullptr) {
                task.Reject(env, JsErrUtils::CreateJsError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY));
                TLOGNE(WmsLogTag::WMS_SYSTEM, "Get snapshot is nullptr!");
                return;
            }
            auto nativePixelMap = Media::PixelMapNapi::CreatePixelMap(env, dataPack->pixelMap);
            if (nativePixelMap == nullptr) {
                task.Reject(env, JsErrUtils::CreateJsError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY));
                TLOGNE(WmsLogTag::WMS_SYSTEM, "Create native pixelmap is nullptr!");
                return;
            }
            task.Resolve(env, nativePixelMap);
        };
    napi_value lastParam = (argc <= 1) ? nullptr : argv[0];
    napi_value result = nullptr;
    NapiAsyncTask::Schedule("JsWindowManager::OnGetSnapshot",
        env, CreateAsyncTaskWithLastParam(env, lastParam, std::move(execute), std::move(complete), &result));
    return result;
}

napi_value JsWindowManager::OnFindWindow(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::WMS_LIFE, "[NAPI]");
    std::string windowName;
    WMError errCode = WMError::WM_OK;
    size_t argc = 4;
    napi_value argv[4] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc < 1 || argc > 2) { // 2: maximum params num
        TLOGE(WmsLogTag::WMS_LIFE, "Argc is invalid: %{public}zu", argc);
        errCode = WMError::WM_ERROR_INVALID_PARAM;
    } else {
        if (!ConvertFromJsValue(env, argv[0], windowName)) {
            TLOGE(WmsLogTag::WMS_LIFE, "Failed to convert parameter to windowName");
            errCode = WMError::WM_ERROR_INVALID_PARAM;
        }
    }
    if (windowName.compare(PIP_WINDOW) == 0) {
        errCode = WMError::WM_ERROR_INVALID_PARAM;
    }
    TLOGI(WmsLogTag::WMS_LIFE, "Window name=%{public}s, err=%{public}d", windowName.c_str(), errCode);
    napi_value lastParam = (argc <= 1) ? nullptr : (GetType(env, argv[1]) == napi_function ? argv[1] : nullptr);
    napi_value result = nullptr;
    std::shared_ptr<NapiAsyncTask> napiAsyncTask = CreateEmptyAsyncTask(env, lastParam, &result);
    auto asyncTask = [errCode, windowName, env, task = napiAsyncTask] {
        if (errCode != WMError::WM_OK) {
            task->Reject(env, JsErrUtils::CreateJsError(env, errCode, "Invalidate params"));
            return;
        }
        HITRACE_METER_FMT(HITRACE_TAG_WINDOW_MANAGER, "WM:Find %s", windowName.c_str());
        std::shared_ptr<NativeReference> jsWindowObj = FindJsWindowObject(windowName);
        if (jsWindowObj != nullptr && jsWindowObj->GetNapiValue() != nullptr) {
            TLOGNI(WmsLogTag::WMS_LIFE, "Find window: %{public}s, use exist js window", windowName.c_str());
            task->Resolve(env, jsWindowObj->GetNapiValue());
        } else {
            sptr<Window> window = Window::Find(windowName);
            if (window == nullptr) {
                TLOGNE(WmsLogTag::WMS_LIFE, "Cannot find window: %{public}s", windowName.c_str());
                task->Reject(env, JsErrUtils::CreateJsError(env, WMError::WM_ERROR_NULLPTR, "Cannot find window"));
            } else {
                task->Resolve(env, CreateJsWindowObject(env, window));
                TLOGNI(WmsLogTag::WMS_LIFE, "Find window: %{public}s, create js window", windowName.c_str());
            }
        }
    };
    if (napi_send_event(env, asyncTask, napi_eprio_high) != napi_status::napi_ok) {
        TLOGE(WmsLogTag::WMS_LIFE, "napi send event failed, window state is abnormal");
    }
    return result;
}

napi_value JsWindowManager::OnFindWindowSync(napi_env env, napi_callback_info info)
{
    WLOGFD("[NAPI]");
    std::string windowName;
    WmErrorCode errCode = WmErrorCode::WM_OK;
    size_t argc = 4;
    napi_value argv[4] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc < 1) { // 1: params num
        WLOGFE("Argc is invalid: %{public}zu", argc);
        errCode = WmErrorCode::WM_ERROR_INVALID_PARAM;
    } else {
        if (!ConvertFromJsValue(env, argv[0], windowName)) {
            WLOGFE("Failed to convert parameter to windowName");
            errCode = WmErrorCode::WM_ERROR_INVALID_PARAM;
        }
    }
    if (windowName.compare(PIP_WINDOW) == 0) {
        errCode = WmErrorCode::WM_ERROR_INVALID_PARAM;
    }
    if (errCode == WmErrorCode::WM_ERROR_INVALID_PARAM) {
        napi_throw(env, JsErrUtils::CreateJsError(env, WmErrorCode::WM_ERROR_INVALID_PARAM));
        return NapiGetUndefined(env);
    }

    WLOGD("Window name=%{public}s, err=%{public}d", windowName.c_str(), errCode);
    std::shared_ptr<NativeReference> jsWindowObj = FindJsWindowObject(windowName);
    if (jsWindowObj != nullptr && jsWindowObj->GetNapiValue() != nullptr) {
        WLOGD("Find window: %{public}s, use exist js window", windowName.c_str());
        return jsWindowObj->GetNapiValue();
    } else {
        sptr<Window> window = Window::Find(windowName);
        if (window == nullptr) {
            napi_throw(env, JsErrUtils::CreateJsError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY));
            return NapiGetUndefined(env);
        } else {
            return CreateJsWindowObject(env, window);
        }
    }
}

napi_value JsWindowManager::OnMinimizeAll(napi_env env, napi_callback_info info)
{
    TLOGI(WmsLogTag::WMS_LIFE, "[NAPI]");
    WmErrorCode errCode = WmErrorCode::WM_OK;
    size_t argc = 4;
    napi_value argv[4] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc < 1) {
        TLOGE(WmsLogTag::WMS_LIFE, "Argc is invalid: %{public}zu", argc);
        errCode = WmErrorCode::WM_ERROR_INVALID_PARAM;
    }
    int64_t displayId = static_cast<int64_t>(DISPLAY_ID_INVALID);
    if (errCode == WmErrorCode::WM_OK && !ConvertFromJsValue(env, argv[0], displayId)) {
        TLOGE(WmsLogTag::WMS_LIFE, "Failed to convert parameter to displayId");
        errCode = WmErrorCode::WM_ERROR_INVALID_PARAM;
    }
    if (displayId < 0 ||
        SingletonContainer::Get<DisplayManager>().GetDisplayById(static_cast<uint64_t>(displayId)) == nullptr) {
        errCode = WmErrorCode::WM_ERROR_INVALID_PARAM;
    }
    if (errCode == WmErrorCode::WM_ERROR_INVALID_PARAM) {
        TLOGE(WmsLogTag::WMS_LIFE, "JsWindowManager::OnMinimizeAll failed, Invalidate params.");
        napi_throw(env, JsErrUtils::CreateJsError(env, WmErrorCode::WM_ERROR_INVALID_PARAM));
        return NapiGetUndefined(env);
    }

    TLOGI(WmsLogTag::WMS_LIFE, "Display id=%{public}" PRIu64 ", err=%{public}d", static_cast<uint64_t>(displayId),
        errCode);
    napi_value lastParam = (argc <= 1) ? nullptr :
        ((argv[1] != nullptr && GetType(env, argv[1]) == napi_function) ? argv[1] : nullptr);
    napi_value result = nullptr;
    std::shared_ptr<NapiAsyncTask> napiAsyncTask = CreateEmptyAsyncTask(env, lastParam, &result);
    auto asyncTask = [displayId, env, task = napiAsyncTask] {
        HITRACE_METER_FMT(HITRACE_TAG_WINDOW_MANAGER, "WM:MinimizeAll: " PRIu64"",
            static_cast<uint64_t>(displayId));
        WmErrorCode ret = WM_JS_TO_ERROR_CODE_MAP.at(
            SingletonContainer::Get<WindowManager>().MinimizeAllAppWindows(static_cast<uint64_t>(displayId)));
        if (ret == WmErrorCode::WM_OK) {
            task->Resolve(env, NapiGetUndefined(env));
            TLOGNI(WmsLogTag::WMS_LIFE, "OnMinimizeAll success");
        } else {
            task->Reject(env, JsErrUtils::CreateJsError(env, ret, "OnMinimizeAll failed"));
        }
    };
    if (napi_send_event(env, asyncTask, napi_eprio_high) != napi_status::napi_ok) {
        TLOGE(WmsLogTag::WMS_LIFE, "napi send event failed, window state is abnormal");
    }
    return result;
}

napi_value JsWindowManager::OnToggleShownStateForAllAppWindows(napi_env env, napi_callback_info info)
{
    WLOGFI("[NAPI]");
    size_t argc = 4;
    napi_value argv[4] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    napi_value lastParam = (argc == 0) ? nullptr :
        (GetType(env, argv[0]) == napi_function ? argv[0] : nullptr);
    napi_value result = nullptr;
    std::shared_ptr<NapiAsyncTask> napiAsyncTask = CreateEmptyAsyncTask(env, lastParam, &result);
    auto asyncTask = [env, task = napiAsyncTask, where = __func__] {
        WmErrorCode ret = WM_JS_TO_ERROR_CODE_MAP.at(
            SingletonContainer::Get<WindowManager>().ToggleShownStateForAllAppWindows());
        if (ret == WmErrorCode::WM_OK) {
            task->Resolve(env, NapiGetUndefined(env));
            WLOGI("%{public}s success", where);
        } else {
            task->Reject(env, JsErrUtils::CreateJsError(env, ret, "OnToggleShownStateForAllAppWindows failed"));
        }
    };
    if (napi_status::napi_ok != napi_send_event(env, asyncTask, napi_eprio_high)) {
        napiAsyncTask->Reject(env,
            JsErrUtils::CreateJsError(env, WmErrorCode::WM_ERROR_SYSTEM_ABNORMALLY, "failed to send event"));
    }
    return result;
}

napi_value JsWindowManager::OnRegisterWindowManagerCallback(napi_env env, napi_callback_info info)
{
    WLOGFD("[NAPI]");
    size_t argc = 4;
    napi_value argv[4] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc < 2) { // 2: params num
        WLOGFE("Argc is invalid: %{public}zu", argc);
        napi_throw(env, JsErrUtils::CreateJsError(env, WmErrorCode::WM_ERROR_INVALID_PARAM));
        return NapiGetUndefined(env);
    }
    std::string cbType;
    if (!ConvertFromJsValue(env, argv[0], cbType)) {
        WLOGFE("Failed to convert parameter to callbackType");
        napi_throw(env, JsErrUtils::CreateJsError(env, WmErrorCode::WM_ERROR_INVALID_PARAM));
        return NapiGetUndefined(env);
    }
    napi_value value = argv[1];
    if (!NapiIsCallable(env, value)) {
        WLOGI("Callback(argv[1]) is not callable");
        napi_throw(env, JsErrUtils::CreateJsError(env, WmErrorCode::WM_ERROR_INVALID_PARAM));
        return NapiGetUndefined(env);
    }

    WmErrorCode ret = registerManager_->RegisterListener(nullptr, cbType, CaseType::CASE_WINDOW_MANAGER, env, value);
    if (ret != WmErrorCode::WM_OK) {
        napi_throw(env, JsErrUtils::CreateJsError(env, ret));
        return NapiGetUndefined(env);
    }
    WLOGI("Register end, type=%{public}s", cbType.c_str());
    return NapiGetUndefined(env);
}

napi_value JsWindowManager::OnUnregisterWindowManagerCallback(napi_env env, napi_callback_info info)
{
    WLOGFD("[NAPI]");
    size_t argc = 4;
    napi_value argv[4] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc < 1) {
        WLOGFE("Argc is invalid: %{public}zu", argc);
        napi_throw(env, JsErrUtils::CreateJsError(env, WmErrorCode::WM_ERROR_INVALID_PARAM));
        return NapiGetUndefined(env);
    }
    std::string cbType;
    if (!ConvertFromJsValue(env, argv[0], cbType)) {
        WLOGFE("Failed to convert parameter to callbackType");
        napi_throw(env, JsErrUtils::CreateJsError(env, WmErrorCode::WM_ERROR_INVALID_PARAM));
        return NapiGetUndefined(env);
    }

    napi_value value = nullptr;
    WmErrorCode ret = WmErrorCode::WM_OK;
    if (argc == 1) {
        ret = registerManager_->UnregisterListener(nullptr, cbType, CaseType::CASE_WINDOW_MANAGER, env, value);
    } else {
        value = argv[1];
        if ((value == nullptr) || (!NapiIsCallable(env, value))) {
            ret = registerManager_->UnregisterListener(nullptr, cbType, CaseType::CASE_WINDOW_MANAGER, env, nullptr);
        } else {
            ret = registerManager_->UnregisterListener(nullptr, cbType, CaseType::CASE_WINDOW_MANAGER, env, value);
        }
    }
    if (ret != WmErrorCode::WM_OK) {
        napi_throw(env, JsErrUtils::CreateJsError(env, ret));
        return NapiGetUndefined(env);
    }
    WLOGI("Unregister end, type=%{public}s", cbType.c_str());
    return NapiGetUndefined(env);
}

static napi_value GetTopWindowTask(void* contextPtr, napi_env env, napi_value callback, bool newApi)
{
    struct TopWindowInfoList {
        sptr<Window> window = nullptr;
        AppExecFwk::Ability* ability = nullptr;
        int32_t errorCode = 0;
        std::string errMsg = "";
    };
    std::shared_ptr<TopWindowInfoList> lists = std::make_shared<TopWindowInfoList>();
    bool isOldApi = GetAPI7Ability(env, lists->ability);
    NapiAsyncTask::ExecuteCallback execute = [lists, isOldApi, newApi, contextPtr]() {
        if (lists == nullptr) {
            return;
        }
        if (isOldApi) {
            if (lists->ability->GetWindow() == nullptr) {
                lists->errorCode = newApi ? static_cast<int32_t>(WmErrorCode::WM_ERROR_STATE_ABNORMALLY) :
                    static_cast<int32_t>(WMError::WM_ERROR_NULLPTR);
                lists->errMsg = "FA mode can not get ability window";
                return;
            }
            lists->window = Window::GetTopWindowWithId(lists->ability->GetWindow()->GetWindowId());
        } else {
            auto context = static_cast<std::weak_ptr<AbilityRuntime::Context>*>(contextPtr);
            if (contextPtr == nullptr || context == nullptr) {
                lists->errorCode = newApi ? static_cast<int32_t>(WmErrorCode::WM_ERROR_STATE_ABNORMALLY) :
                    static_cast<int32_t>(WMError::WM_ERROR_NULLPTR);
                lists->errMsg = "Stage mode without context";
                return;
            }
            lists->window = Window::GetTopWindowWithContext(context->lock());
        }
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
        if (lists->errorCode != 0) {
            if (newApi) {
                task.Reject(env, JsErrUtils::CreateJsError(env, static_cast<WmErrorCode>(lists->errorCode),
                    lists->errMsg));
            } else {
                task.Reject(env, JsErrUtils::CreateJsError(env, static_cast<WMError>(lists->errorCode),
                    lists->errMsg));
            }
            WLOGFE("%{public}s", lists->errMsg.c_str());
            return;
        }
        if (lists->window == nullptr || lists->window->GetWindowState() == WindowState::STATE_DESTROYED) {
            if (newApi) {
                task.Reject(env, JsErrUtils::CreateJsError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
                    "Get top window failed"));
            } else {
                task.Reject(env, JsErrUtils::CreateJsError(env, WMError::WM_ERROR_NULLPTR,
                    "Get top window failed"));
            }
            return;
        }
        task.Resolve(env, CreateJsWindowObject(env, lists->window));
        WLOGD("Get top window success");
    };
    napi_value result = nullptr;
    auto asyncTask = CreateAsyncTask(env, callback,
        std::make_unique<NapiAsyncTask::ExecuteCallback>(std::move(execute)),
        std::make_unique<NapiAsyncTask::CompleteCallback>(std::move(complete)), &result);
    NapiAsyncTask::Schedule("JsWindowManager::OnGetTopWindow", env, std::move(asyncTask));
    return result;
}

napi_value JsWindowManager::OnGetTopWindow(napi_env env, napi_callback_info info)
{
    WLOGFD("[NAPI]");
    WMError errCode = WMError::WM_OK;
    napi_value nativeContext = nullptr;
    napi_value nativeCallback = nullptr;
    void* contextPtr = nullptr;
    size_t argc = 4;
    napi_value argv[4] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc > 2) { // 2: maximum params num
        WLOGFE("Argc is invalid: %{public}zu", argc);
        errCode = WMError::WM_ERROR_INVALID_PARAM;
    } else {
        if (argc > 0 && GetType(env, argv[0]) == napi_object) { // (context, callback?)
            nativeContext = argv[0];
            nativeCallback = (argc == 1) ? nullptr :
                (GetType(env, argv[1]) == napi_function ? argv[1] : nullptr);
        } else { // (callback?)
            nativeCallback = (argc == 0) ? nullptr :
                (GetType(env, argv[0]) == napi_function ? argv[0] : nullptr);
        }
        GetNativeContext(env, nativeContext, contextPtr, errCode);
    }
    return GetTopWindowTask(contextPtr, env, nativeCallback, false);
}

napi_value JsWindowManager::OnGetLastWindow(napi_env env, napi_callback_info info)
{
    WLOGFD("[NAPI]");
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

/** @note @window.layout */
napi_value JsWindowManager::OnSetWindowLayoutMode(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::WMS_LAYOUT, "[NAPI]");
    if (!Permission::IsSystemCalling() && !Permission::IsStartByHdcd()) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "permission denied!");
        return NapiThrowError(env, WmErrorCode::WM_ERROR_NOT_SYSTEM_APP);
    }
    size_t argc = 4;
    napi_value argv[4] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc < 1) { // 1: minimum params num
        TLOGE(WmsLogTag::WMS_LAYOUT, "Argc is invalid: %{public}zu", argc);
        return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM);
    }
    uint32_t winLayoutModeValue = 0;
    if (!ConvertFromJsValue(env, argv[0], winLayoutModeValue)) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "Failed to convert parameter to winLayoutModeValue");
        return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM);
    }
    WindowLayoutMode winLayoutMode = static_cast<WindowLayoutMode>(winLayoutModeValue);
    if (winLayoutMode != WindowLayoutMode::CASCADE && winLayoutMode != WindowLayoutMode::TILE) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "Invalid winLayoutMode: %{public}u", winLayoutMode);
        return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM);
    }
    TLOGI(WmsLogTag::WMS_LAYOUT, "winLayoutMode: %{public}u", winLayoutMode);
    // 1: maximum params num; 1: index of callback
    napi_value lastParam = (argc <= 1) ? nullptr :
        ((argv[1] != nullptr && GetType(env, argv[1]) == napi_function) ? argv[1] : nullptr);
    napi_value result = nullptr;
    std::shared_ptr<NapiAsyncTask> napiAsyncTask = CreateEmptyAsyncTask(env, lastParam, &result);
    auto asyncTask = [env, task = napiAsyncTask, winLayoutMode, where = __func__] {
        WmErrorCode ret = WM_JS_TO_ERROR_CODE_MAP.at(
            SingletonContainer::Get<WindowManager>().SetWindowLayoutMode(winLayoutMode));
        if (ret == WmErrorCode::WM_OK) {
            task->Resolve(env, NapiGetUndefined(env));
            TLOGND(WmsLogTag::WMS_LAYOUT, "%{public}s: success", where);
        } else {
            task->Reject(env, JsErrUtils::CreateJsError(env, ret, "SetWindowLayoutMode failed"));
        }
    };
    if (napi_status::napi_ok != napi_send_event(env, asyncTask, napi_eprio_high)) {
        napiAsyncTask->Reject(env,
            JsErrUtils::CreateJsError(env, WmErrorCode::WM_ERROR_SYSTEM_ABNORMALLY, "failed to send event"));
    }
    return result;
}

napi_value JsWindowManager::OnSetGestureNavigationEnabled(napi_env env, napi_callback_info info)
{
    WLOGFD("[NAPI]");
    size_t argc = 4;
    napi_value argv[4] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc < 1) { // 1: minimum params num
        WLOGFE("Argc is invalid: %{public}zu", argc);
        napi_throw(env, JsErrUtils::CreateJsError(env, WmErrorCode::WM_ERROR_INVALID_PARAM));
        return NapiGetUndefined(env);
    }

    napi_value nativeBool = argv[0];
    if (nativeBool == nullptr) {
        WLOGFE("Failed to convert parameter to bool");
        napi_throw(env, JsErrUtils::CreateJsError(env, WmErrorCode::WM_ERROR_INVALID_PARAM));
        return NapiGetUndefined(env);
    }
    bool gestureNavigationEnable = false;
    napi_get_value_bool(env, nativeBool, &gestureNavigationEnable);

    WLOGI("Set gesture navigation enable as %{public}d", gestureNavigationEnable);
    // 1: maximum params num; 1: index of callback
    napi_value lastParam = (argc <= 1) ? nullptr :
        ((argv[1] != nullptr && GetType(env, argv[1]) == napi_function) ? argv[1] : nullptr);
    napi_value result = nullptr;
    std::shared_ptr<NapiAsyncTask> napiAsyncTask = CreateEmptyAsyncTask(env, lastParam, &result);
    auto asyncTask = [gestureNavigationEnable, env, task = napiAsyncTask] {
        WmErrorCode ret = WM_JS_TO_ERROR_CODE_MAP.at(
            SingletonContainer::Get<WindowManager>().SetGestureNavigationEnabled(gestureNavigationEnable));
        if (ret == WmErrorCode::WM_OK) {
            task->Resolve(env, NapiGetUndefined(env));
            WLOGD("SetGestureNavigationEnabled success");
        } else {
            task->Reject(env, JsErrUtils::CreateJsError(env, ret, "SetGestureNavigationEnabled failed"));
        }
    };
    if (napi_status::napi_ok != napi_send_event(env, asyncTask, napi_eprio_high)) {
        TLOGE(WmsLogTag::WMS_IMMS, "napi_send_event failed");
        napiAsyncTask->Reject(env,
            JsErrUtils::CreateJsError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY, "failed to send event"));
    }
    return result;
}

napi_value JsWindowManager::OnSetWaterMarkImage(napi_env env, napi_callback_info info)
{
    WLOGFD("[NAPI]");
    napi_value nativeObject = nullptr;
    napi_value nativeBoolean = nullptr;
    size_t argc = 4;
    napi_value argv[4] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc < 2) { // 2: params num
        WLOGFE("Argc is invalid: %{public}zu", argc);
        napi_throw(env, JsErrUtils::CreateJsError(env, WmErrorCode::WM_ERROR_INVALID_PARAM));
        return NapiGetUndefined(env);
    } else {
        if (argc > 0 && GetType(env, argv[0]) == napi_object) {
            nativeObject = argv[0];
            nativeBoolean = (GetType(env, argv[1]) == napi_boolean ? argv[1] : nullptr);
        }
    }

    std::shared_ptr<Media::PixelMap> pixelMap;
    pixelMap = OHOS::Media::PixelMapNapi::GetPixelMap(env, nativeObject);
    if (pixelMap == nullptr) {
        WLOGFE("Failed to convert parameter to PixelMap");
        napi_throw(env, JsErrUtils::CreateJsError(env, WmErrorCode::WM_ERROR_INVALID_PARAM));
        return NapiGetUndefined(env);
    }

    if (nativeBoolean == nullptr) {
        WLOGFE("Failed to convert parameter to bool");
        napi_throw(env, JsErrUtils::CreateJsError(env, WmErrorCode::WM_ERROR_INVALID_PARAM));
        return NapiGetUndefined(env);
    }
    bool isShow = false;
    napi_get_value_bool(env, nativeBoolean, &isShow);
    if (!Permission::IsSystemCalling()) {
        WLOGFE("set watermark image permission denied!");
        napi_throw(env, JsErrUtils::CreateJsError(env, WmErrorCode::WM_ERROR_NOT_SYSTEM_APP));
        return NapiGetUndefined(env);
    }

    // 2: maximum params num; 2: index of callback
    napi_value lastParam = (argc <= 2) ? nullptr :
        (GetType(env, argv[2]) == napi_function ? argv[2] : nullptr);
    napi_value result = nullptr;
    std::shared_ptr<NapiAsyncTask> napiAsyncTask = CreateEmptyAsyncTask(env, lastParam, &result);
    auto asyncTask = [env, pixelMap, isShow, task = napiAsyncTask] {
        RSInterfaces::GetInstance().ShowWatermark(pixelMap, isShow);
        task->Resolve(env, NapiGetUndefined(env));
        WLOGD("OnSetWaterMarkImage success");
    };
    if (napi_status::napi_ok != napi_send_event(env, asyncTask, napi_eprio_high)) {
        TLOGE(WmsLogTag::WMS_IMMS, "napi_send_event failed");
        napiAsyncTask->Reject(env,
            JsErrUtils::CreateJsError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY, "failed to send event"));
    }
    return result;
}

napi_value JsWindowManager::OnShiftAppWindowFocus(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::WMS_FOCUS, "OnShiftAppWindowFocus");
    WMError errCode = WMError::WM_OK;
    size_t argc = 4;
    napi_value argv[4] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc != 2) { // 2: params num
        TLOGE(WmsLogTag::WMS_FOCUS, "Argc is invalid: %{public}zu", argc);
        errCode = WMError::WM_ERROR_INVALID_PARAM;
    }
    int32_t sourcePersistentId = static_cast<int32_t>(INVALID_WINDOW_ID);
    int32_t targetPersistentId = static_cast<int32_t>(INVALID_WINDOW_ID);
    if (errCode == WMError::WM_OK && !ConvertFromJsValue(env, argv[0], sourcePersistentId)) {
        TLOGE(WmsLogTag::WMS_FOCUS, "Failed to convert parameter to source window Id");
        errCode = WMError::WM_ERROR_INVALID_PARAM;
    }
    if (errCode == WMError::WM_OK && !ConvertFromJsValue(env, argv[1], targetPersistentId)) {
        TLOGE(WmsLogTag::WMS_FOCUS, "Failed to convert parameter to target window Id");
        errCode = WMError::WM_ERROR_INVALID_PARAM;
    }
    if (errCode == WMError::WM_ERROR_INVALID_PARAM) {
        napi_throw(env, JsErrUtils::CreateJsError(env, WmErrorCode::WM_ERROR_INVALID_PARAM));
        return NapiGetUndefined(env);
    }
    // only return promiss<void>
    napi_value lastParam = nullptr;
    napi_value result = nullptr;
    std::shared_ptr<NapiAsyncTask> napiAsyncTask = CreateEmptyAsyncTask(env, lastParam, &result);
    auto asyncTask = [env, task = napiAsyncTask, sourcePersistentId, targetPersistentId] {
            WmErrorCode ret = WM_JS_TO_ERROR_CODE_MAP.at(
                SingletonContainer::Get<WindowManager>().ShiftAppWindowFocus(sourcePersistentId, targetPersistentId));
            if (ret == WmErrorCode::WM_OK) {
                task->Resolve(env, NapiGetUndefined(env));
                TLOGND(WmsLogTag::WMS_FOCUS, "OnShiftAppWindowFocus success");
            } else {
                task->Reject(env, JsErrUtils::CreateJsError(env, ret, "ShiftAppWindowFocus failed"));
            }
        };
    if (napi_status::napi_ok != napi_send_event(env, asyncTask, napi_eprio_high)) {
        TLOGE(WmsLogTag::WMS_FOCUS, "window state is abnormal!");
    }
    return result;
}

napi_value JsWindowManager::OnGetAllWindowLayoutInfo(napi_env env, napi_callback_info info)
{
    size_t argc = ARGC_FOUR;
    napi_value argv[ARGC_FOUR] = { nullptr };
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc != ARGC_ONE) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "Argc is invalid: %{public}zu", argc);
        return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM);
    }
    int64_t displayId = static_cast<int64_t>(DISPLAY_ID_INVALID);
    if (!ConvertFromJsValue(env, argv[INDEX_ZERO], displayId)) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "Failed to convert parameter to displayId");
        return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM);
    }
    if (displayId < 0 ||
        SingletonContainer::Get<DisplayManager>().GetDisplayById(static_cast<uint64_t>(displayId)) == nullptr) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "invalid displayId");
        return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM);
    }
    napi_value result = nullptr;
    std::shared_ptr<NapiAsyncTask> napiAsyncTask = CreateEmptyAsyncTask(env, nullptr, &result);
    auto asyncTask = [env, task = napiAsyncTask, displayId, where = __func__] {
        std::vector<sptr<WindowLayoutInfo>> infos;
        WmErrorCode ret = WM_JS_TO_ERROR_CODE_MAP.at(
            SingletonContainer::Get<WindowManager>().GetAllWindowLayoutInfo(static_cast<uint64_t>(displayId), infos));
        if (ret == WmErrorCode::WM_OK) {
            task->Resolve(env, CreateJsWindowLayoutInfoArrayObject(env, infos));
            TLOGNI(WmsLogTag::WMS_ATTRIBUTE, "%{public}s success", where);
        } else {
            task->Reject(env, JsErrUtils::CreateJsError(env, ret, "failed"));
            TLOGNE(WmsLogTag::WMS_ATTRIBUTE, "%{public}s failed", where);
        }
    };
    if (napi_status::napi_ok != napi_send_event(env, asyncTask, napi_eprio_high)) {
        napiAsyncTask->Reject(env,
            CreateJsError(env, static_cast<int32_t>(WmErrorCode::WM_ERROR_STATE_ABNORMALLY), "send event failed"));
    }
    return result;
}

napi_value JsWindowManager::OnGetGlobalWindowMode(napi_env env, napi_callback_info info)
{
    size_t argc = ARGC_FOUR;
    napi_value argv[ARGC_FOUR] = { nullptr };
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc > ARGC_ONE) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "Argc is invalid: %{public}zu", argc);
        return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM);
    }
    DisplayId displayId = DISPLAY_ID_INVALID;
    if (argc == ARGC_ONE) {
        int64_t inputDisplayId = 0;
        if (!ConvertFromJsValue(env, argv[INDEX_ZERO], inputDisplayId)) {
            TLOGE(WmsLogTag::WMS_ATTRIBUTE, "Failed to convert parameter to displayId");
            return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM);
        }
        if (inputDisplayId >= 0) {
            displayId = static_cast<DisplayId>(inputDisplayId);
        }
    }
    if (displayId != DISPLAY_ID_INVALID &&
        SingletonContainer::Get<DisplayManager>().GetDisplayById(displayId) == nullptr) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "invalid displayId: %{public}" PRIu64, displayId);
        return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM);
    }
    napi_value result = nullptr;
    std::shared_ptr<NapiAsyncTask> napiAsyncTask = CreateEmptyAsyncTask(env, nullptr, &result);
    auto asyncTask = [env, task = napiAsyncTask, displayId, where = __func__] {
        GlobalWindowMode globalWinMode = GlobalWindowMode::UNKNOWN;
        WMError errCode = SingletonContainer::Get<WindowManager>().GetGlobalWindowMode(displayId, globalWinMode);
        if (errCode == WMError::WM_OK) {
            TLOGNI(WmsLogTag::WMS_ATTRIBUTE, "%{public}s ok, globalWinMode: %{public}u, displayId: %{public}" PRIu64,
                where, static_cast<uint32_t>(globalWinMode), displayId);
            task->Resolve(env, CreateJsValue(env, globalWinMode));
        } else {
            TLOGNE(WmsLogTag::WMS_ATTRIBUTE, "%{public}s failed, errCode: %{public}d, displayId: %{public}" PRIu64,
                where, static_cast<int32_t>(errCode), displayId);
            task->Reject(env, JsErrUtils::CreateJsError(env, WM_JS_TO_ERROR_CODE_MAP.at(errCode), "failed"));
        }
    };
    if (napi_send_event(env, asyncTask, napi_eprio_high) != napi_status::napi_ok) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "send event failed, displayId: %{public}" PRIu64, displayId);
        napiAsyncTask->Reject(env,
            CreateJsError(env, static_cast<int32_t>(WmErrorCode::WM_ERROR_SYSTEM_ABNORMALLY), "send event failed"));
    }
    return result;
}

napi_value JsWindowManager::OnGetVisibleWindowInfo(napi_env env, napi_callback_info info)
{
    size_t argc = 4;
    napi_value argv[4] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    napi_value lastParam = argc <= 0 || GetType(env, argv[0]) != napi_function ? nullptr : argv[0];
    napi_value result = nullptr;
    std::shared_ptr<NapiAsyncTask> napiAsyncTask = CreateEmptyAsyncTask(env, lastParam, &result);
    auto asyncTask = [env, task = napiAsyncTask]() {
        std::vector<sptr<WindowVisibilityInfo>> infos;
        WmErrorCode ret =
            WM_JS_TO_ERROR_CODE_MAP.at(SingletonContainer::Get<WindowManager>().GetVisibilityWindowInfo(infos));
        if (ret == WmErrorCode::WM_OK) {
            task->Resolve(env, CreateJsWindowInfoArrayObject(env, infos));
            TLOGND(WmsLogTag::WMS_ATTRIBUTE, "OnGetVisibleWindowInfo success");
        } else {
            TLOGNE(WmsLogTag::WMS_ATTRIBUTE, "OnGetVisibleWindowInfo failed");
            task->Reject(env, JsErrUtils::CreateJsError(env, ret, "OnGetVisibleWindowInfo failed"));
        }
    };
    if (napi_status::napi_ok != napi_send_event(env, asyncTask, napi_eprio_high)) {
        napiAsyncTask->Reject(env,
            JsErrUtils::CreateJsError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY, "failed to send event"));
    }
    return result;
}

napi_value JsWindowManager::OnGetWindowsByCoordinate(napi_env env, napi_callback_info info)
{
    size_t argc = ARGC_FOUR;
    napi_value argv[ARGC_FOUR] = { nullptr };
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc < ARGC_ONE || argc > ARGC_FOUR) { // min param num 1, max param num 4
        return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM);
    }
    int64_t displayId = static_cast<int64_t>(DISPLAY_ID_INVALID);
    if (!ConvertFromJsValue(env, argv[INDEX_ZERO], displayId)) {
        TLOGE(WmsLogTag::WMS_PC, "Failed to convert parameter to displayId");
        return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM);
    }
    if (displayId < 0 ||
        SingletonContainer::Get<DisplayManager>().GetDisplayById(static_cast<uint64_t>(displayId)) == nullptr) {
        TLOGE(WmsLogTag::WMS_PC, "invalid displayId");
        return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM);
    }
    int32_t windowNumber = 0;
    if (argc > ARGC_ONE && !ConvertFromJsValue(env, argv[ARGC_ONE], windowNumber)) {
        windowNumber = 0;
    }
    int32_t x = INVALID_COORDINATE;
    if (argc > ARGC_TWO && !ConvertFromJsValue(env, argv[ARGC_TWO], x)) {
        x = INVALID_COORDINATE;
    }
    int32_t y = INVALID_COORDINATE;
    if (argc > ARGC_THREE && !ConvertFromJsValue(env, argv[ARGC_THREE], y)) {
        y = INVALID_COORDINATE;
    }
    napi_value result = nullptr;
    std::shared_ptr<NapiAsyncTask> napiAsyncTask = CreateEmptyAsyncTask(env, nullptr, &result);
    auto asyncTask = [displayId, windowNumber, x, y, env, task = napiAsyncTask] {
        std::vector<int32_t> windowIds;
        WmErrorCode ret = WM_JS_TO_ERROR_CODE_MAP.at(SingletonContainer::Get<WindowManager>().
            GetWindowIdsByCoordinate(static_cast<uint64_t>(displayId), windowNumber, x, y, windowIds));
        if (ret == WmErrorCode::WM_OK) {
            std::vector<sptr<Window>> windows(windowIds.size());
            for (size_t i = 0; i < windowIds.size(); i++) {
                windows[i] = Window::GetWindowWithId(windowIds[i]);
            }
            task->Resolve(env, CreateJsWindowArrayObject(env, windows));
        } else {
            task->Reject(env, JsErrUtils::CreateJsError(env, ret, "getWindowsByCoordinate failed"));
        }
    };
    if (napi_status::napi_ok != napi_send_event(env, asyncTask, napi_eprio_high)) {
        napiAsyncTask->Reject(env,
            CreateJsError(env, static_cast<int32_t>(WmErrorCode::WM_ERROR_STATE_ABNORMALLY), "send event failed"));
    }
    return result;
}

napi_value JsWindowManager::OnShiftAppWindowPointerEvent(napi_env env, napi_callback_info info)
{
    size_t argc = ARGC_FOUR;
    napi_value argv[ARGC_FOUR] = { nullptr };
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc != ARGC_TWO) {
        return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM);
    }
    int32_t sourceWindowId;
    if (!ConvertFromJsValue(env, argv[INDEX_ZERO], sourceWindowId)) {
        TLOGE(WmsLogTag::WMS_PC, "Failed to convert parameter to sourceWindowId");
        return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM);
    }
    int32_t targetWindowId;
    if (!ConvertFromJsValue(env, argv[INDEX_ONE], targetWindowId)) {
        TLOGE(WmsLogTag::WMS_PC, "Failed to convert parameter to targetWindowId");
        return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM);
    }
    if (sourceWindowId == static_cast<int32_t>(INVALID_WINDOW_ID) ||
        targetWindowId == static_cast<int32_t>(INVALID_WINDOW_ID)) {
        TLOGE(WmsLogTag::WMS_PC, "invalid sourceWindowId or targetWindowId");
        return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM);
    }
    napi_value result = nullptr;
    std::shared_ptr<NapiAsyncTask> napiAsyncTask = CreateEmptyAsyncTask(env, nullptr, &result);
    auto asyncTask = [sourceWindowId, targetWindowId, env, task = napiAsyncTask] {
        WmErrorCode ret = WM_JS_TO_ERROR_CODE_MAP.at(SingletonContainer::Get<WindowManager>().
            ShiftAppWindowPointerEvent(sourceWindowId, targetWindowId));
        if (ret == WmErrorCode::WM_OK) {
            task->Resolve(env, NapiGetUndefined(env));
        } else {
            task->Reject(env, JsErrUtils::CreateJsError(env, ret, "shiftAppWindowPointerEvent failed"));
        }
    };
    if (napi_status::napi_ok != napi_send_event(env, std::move(asyncTask), napi_eprio_high)) {
        napiAsyncTask->Reject(env,
            CreateJsError(env, static_cast<int32_t>(WmErrorCode::WM_ERROR_STATE_ABNORMALLY), "send event failed"));
    }
    return result;
}

napi_value JsWindowManagerInit(napi_env env, napi_value exportObj)
{
    WLOGFD("[NAPI]");

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
    napi_set_named_property(env, exportObj, "WindowEventType", WindowEventTypeInit(env));
    napi_set_named_property(env, exportObj, "WindowLayoutMode", WindowLayoutModeInit(env));
    napi_set_named_property(env, exportObj, "Orientation", OrientationInit(env));
    napi_set_named_property(env, exportObj, "BlurStyle", BlurStyleInit(env));
    napi_set_named_property(env, exportObj, "WmErrorCode", WindowErrorCodeInit(env));
    napi_set_named_property(env, exportObj, "WMError", WindowErrorInit(env));
    napi_set_named_property(env, exportObj, "WindowStatusType", WindowStatusTypeInit(env));
    napi_set_named_property(env, exportObj, "RectChangeReason", RectChangeReasonInit(env));
    napi_set_named_property(env, exportObj, "MaximizePresentation", MaximizePresentationInit(env));
    napi_set_named_property(env, exportObj, "ExtensionWindowAttribute", ExtensionWindowAttributeInit(env));
    napi_set_named_property(env, exportObj, "ModalityType", ModalityTypeInit(env));
    napi_set_named_property(env, exportObj, "RotationChangeType", RotationChangeTypeInit(env));
    napi_set_named_property(env, exportObj, "RectType", RectTypeInit(env));
    const char *moduleName = "JsWindowManager";
    BindNativeFunction(env, exportObj, "create", moduleName, JsWindowManager::Create);
    BindNativeFunction(env, exportObj, "createWindow", moduleName, JsWindowManager::CreateWindow);
    BindNativeFunction(env, exportObj, "find", moduleName, JsWindowManager::FindWindow);
    BindNativeFunction(env, exportObj, "findWindow", moduleName, JsWindowManager::FindWindowSync);
    BindNativeFunction(env, exportObj, "on", moduleName, JsWindowManager::RegisterWindowManagerCallback);
    BindNativeFunction(env, exportObj, "off", moduleName, JsWindowManager::UnregisterWindowMangerCallback);
    BindNativeFunction(env, exportObj, "getTopWindow", moduleName, JsWindowManager::GetTopWindow);
    BindNativeFunction(env, exportObj, "getLastWindow", moduleName, JsWindowManager::GetLastWindow);
    BindNativeFunction(env, exportObj, "getSnapshot", moduleName, JsWindowManager::GetSnapshot);
    BindNativeFunction(env, exportObj, "minimizeAll", moduleName, JsWindowManager::MinimizeAll);
    BindNativeFunction(env, exportObj, "toggleShownStateForAllAppWindows", moduleName,
        JsWindowManager::ToggleShownStateForAllAppWindows);
    BindNativeFunction(env, exportObj, "setWindowLayoutMode", moduleName, JsWindowManager::SetWindowLayoutMode);
    BindNativeFunction(env, exportObj, "setGestureNavigationEnabled", moduleName,
        JsWindowManager::SetGestureNavigationEnabled);
    BindNativeFunction(env, exportObj, "setWaterMarkImage", moduleName, JsWindowManager::SetWaterMarkImage);
    BindNativeFunction(env, exportObj, "shiftAppWindowFocus", moduleName, JsWindowManager::ShiftAppWindowFocus);
    BindNativeFunction(env, exportObj, "getAllWindowLayoutInfo", moduleName, JsWindowManager::GetAllWindowLayoutInfo);
    BindNativeFunction(env, exportObj, "getGlobalWindowMode", moduleName, JsWindowManager::GetGlobalWindowMode);
    BindNativeFunction(env, exportObj, "getVisibleWindowInfo", moduleName, JsWindowManager::GetVisibleWindowInfo);
    BindNativeFunction(env, exportObj, "getWindowsByCoordinate", moduleName, JsWindowManager::GetWindowsByCoordinate);
    BindNativeFunction(env, exportObj, "shiftAppWindowPointerEvent", moduleName,
        JsWindowManager::ShiftAppWindowPointerEvent);
    return NapiGetUndefined(env);
}
}  // namespace Rosen
}  // namespace OHOS
