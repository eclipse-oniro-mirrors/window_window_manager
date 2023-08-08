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
}

JsWindowManager::JsWindowManager() : registerManager_(std::make_unique<JsWindowRegisterManager>())
{
}

JsWindowManager::~JsWindowManager()
{
}

void JsWindowManager::Finalizer(NativeEngine* engine, void* data, void* hint)
{
    WLOGI("Finalizer");
    std::unique_ptr<JsWindowManager>(static_cast<JsWindowManager*>(data));
}

NativeValue* JsWindowManager::Create(NativeEngine* engine, NativeCallbackInfo* info)
{
    JsWindowManager* me = CheckParamsAndGetThis<JsWindowManager>(engine, info);
    return (me != nullptr) ? me->OnCreate(*engine, *info) : nullptr;
}

NativeValue* JsWindowManager::CreateWindow(NativeEngine* engine, NativeCallbackInfo* info)
{
    JsWindowManager* me = CheckParamsAndGetThis<JsWindowManager>(engine, info);
    return (me != nullptr) ? me->OnCreateWindow(*engine, *info) : nullptr;
}

NativeValue* JsWindowManager::FindWindow(NativeEngine* engine, NativeCallbackInfo* info)
{
    JsWindowManager* me = CheckParamsAndGetThis<JsWindowManager>(engine, info);
    return (me != nullptr) ? me->OnFindWindow(*engine, *info) : nullptr;
}

NativeValue* JsWindowManager::FindWindowSync(NativeEngine* engine, NativeCallbackInfo* info)
{
    JsWindowManager* me = CheckParamsAndGetThis<JsWindowManager>(engine, info);
    return (me != nullptr) ? me->OnFindWindowSync(*engine, *info) : nullptr;
}

NativeValue* JsWindowManager::MinimizeAll(NativeEngine* engine, NativeCallbackInfo* info)
{
    JsWindowManager* me = CheckParamsAndGetThis<JsWindowManager>(engine, info);
    return (me != nullptr) ? me->OnMinimizeAll(*engine, *info) : nullptr;
}

NativeValue* JsWindowManager::ToggleShownStateForAllAppWindows(NativeEngine* engine, NativeCallbackInfo* info)
{
    JsWindowManager* me = CheckParamsAndGetThis<JsWindowManager>(engine, info);
    return (me != nullptr) ? me->OnToggleShownStateForAllAppWindows(*engine, *info) : nullptr;
}

NativeValue* JsWindowManager::RegisterWindowManagerCallback(NativeEngine* engine, NativeCallbackInfo* info)
{
    JsWindowManager* me = CheckParamsAndGetThis<JsWindowManager>(engine, info);
    return (me != nullptr) ? me->OnRegisterWindowManagerCallback(*engine, *info) : nullptr;
}

NativeValue* JsWindowManager::UnregisterWindowMangerCallback(NativeEngine* engine, NativeCallbackInfo* info)
{
    JsWindowManager* me = CheckParamsAndGetThis<JsWindowManager>(engine, info);
    return (me != nullptr) ? me->OnUnregisterWindowManagerCallback(*engine, *info) : nullptr;
}

NativeValue* JsWindowManager::GetTopWindow(NativeEngine* engine, NativeCallbackInfo* info)
{
    JsWindowManager* me = CheckParamsAndGetThis<JsWindowManager>(engine, info);
    return (me != nullptr) ? me->OnGetTopWindow(*engine, *info) : nullptr;
}

NativeValue* JsWindowManager::GetLastWindow(NativeEngine* engine, NativeCallbackInfo* info)
{
    JsWindowManager* me = CheckParamsAndGetThis<JsWindowManager>(engine, info);
    return (me != nullptr) ? me->OnGetLastWindow(*engine, *info) : nullptr;
}

NativeValue* JsWindowManager::SetWindowLayoutMode(NativeEngine* engine, NativeCallbackInfo* info)
{
    JsWindowManager* me = CheckParamsAndGetThis<JsWindowManager>(engine, info);
    return (me != nullptr) ? me->OnSetWindowLayoutMode(*engine, *info) : nullptr;
}

NativeValue* JsWindowManager::SetGestureNavigationEnabled(NativeEngine* engine, NativeCallbackInfo* info)
{
    JsWindowManager* me = CheckParamsAndGetThis<JsWindowManager>(engine, info);
    return (me != nullptr) ? me->OnSetGestureNavigationEnabled(*engine, *info) : nullptr;
}

NativeValue* JsWindowManager::SetWaterMarkImage(NativeEngine* engine, NativeCallbackInfo* info)
{
    JsWindowManager* me = CheckParamsAndGetThis<JsWindowManager>(engine, info);
    return (me != nullptr) ? me->OnSetWaterMarkImage(*engine, *info) : nullptr;
}

static void GetNativeContext(NativeEngine& engine, NativeValue* nativeContext, void*& contextPtr, WMError& errCode)
{
    AppExecFwk::Ability* ability = nullptr;
    bool isOldApi = GetAPI7Ability(engine, ability);
    WLOGFD("FA mode:%{public}u", isOldApi);
    if (isOldApi) {
        return;
    }
    if (nativeContext != nullptr) {
        auto objContext = AbilityRuntime::ConvertNativeValueTo<NativeObject>(nativeContext);
        if (objContext == nullptr) {
            WLOGFE("ConvertNativeValueTo Context Object failed");
            errCode = WMError::WM_ERROR_INVALID_PARAM;
            return;
        }
        contextPtr = objContext->GetNativePointer();
    }
}

static uint32_t GetParentId(NativeEngine& engine)
{
    AppExecFwk::Ability* ability = nullptr;
    uint32_t parentId = 0;
    bool isOldApi = GetAPI7Ability(engine, ability);
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

static bool GetWindowTypeAndParentId(NativeEngine& engine, uint32_t& parentId, WindowType& winType,
    NativeValue* nativeString, NativeValue* nativeType)
{
    NativeNumber* type = ConvertNativeValueTo<NativeNumber>(nativeType);
    if (type == nullptr) {
        WLOGFE("Failed to convert parameter to windowType");
        return false;
    }

    if (static_cast<uint32_t>(*type) >= static_cast<uint32_t>(ApiWindowType::TYPE_BASE) &&
        static_cast<uint32_t>(*type) < static_cast<uint32_t>(ApiWindowType::TYPE_END)) {
        winType = JS_TO_NATIVE_WINDOW_TYPE_MAP.at(static_cast<ApiWindowType>(static_cast<uint32_t>(*type)));
    } else {
        WLOGFE("Type %{public}u is not supported", static_cast<uint32_t>(*type));
        return false;
    }

    AppExecFwk::Ability* ability = nullptr;
    bool isOldApi = GetAPI7Ability(engine, ability);
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
    NativeEngine& engine, AsyncTask& task)
{
    WLOGI("CreateSystemWindowTask");
    if (windowOption == nullptr) {
        int32_t err = static_cast<int32_t>(WmErrorCode::WM_ERROR_SYSTEM_ABNORMALLY);
        task.Reject(engine, CreateJsError(engine, err, "New window option failed"));
        WLOGFE("New window option failed");
        return;
    }
    auto context = static_cast<std::weak_ptr<AbilityRuntime::Context>*>(contextPtr);
    if (contextPtr == nullptr || context == nullptr) {
        int32_t err = static_cast<int32_t>(WmErrorCode::WM_ERROR_CONTEXT_ABNORMALLY);
        task.Reject(engine, CreateJsError(engine, err, "Context is nullptr"));
        WLOGFE("Context is nullptr");
        return;
    }
    if (windowOption->GetWindowType() == WindowType::WINDOW_TYPE_FLOAT ||
        windowOption->GetWindowType() == WindowType::WINDOW_TYPE_FLOAT_CAMERA) {
        auto abilityContext = Context::ConvertTo<AbilityRuntime::AbilityContext>(context->lock());
        if (abilityContext != nullptr) {
            if (!CheckCallingPermission("ohos.permission.SYSTEM_FLOAT_WINDOW")) {
                int32_t err = static_cast<int32_t>(WmErrorCode::WM_ERROR_NO_PERMISSION);
                task.Reject(engine, CreateJsError(engine, err, "TYPE_FLOAT CheckCallingPermission failed"));
                return;
            }
        }
    }
    WMError wmError = WMError::WM_OK;
    sptr<Window> window = Window::Create(windowOption->GetWindowName(), windowOption, context->lock(), wmError);
    WmErrorCode wmErrorCode = WM_JS_TO_ERROR_CODE_MAP.at(wmError);
    if (window != nullptr && wmErrorCode == WmErrorCode::WM_OK) {
        task.Resolve(engine, CreateJsWindowObject(engine, window));
    } else {
        WLOGFE("Create window failed");
        int32_t err = static_cast<int32_t>(wmErrorCode);
        task.Reject(engine, CreateJsError(engine, err, "Create window failed"));
    }
}

static void CreateSystemWindowTask(void* contextPtr, std::string windowName, WindowType winType,
    NativeEngine& engine, AsyncTask& task)
{
    WLOGFD("CreateSystemWindowTask");
    auto context = static_cast<std::weak_ptr<AbilityRuntime::Context>*>(contextPtr);
    if (contextPtr == nullptr || context == nullptr) {
        int32_t err = static_cast<int32_t>(WMError::WM_ERROR_NULLPTR);
        task.Reject(engine, CreateJsError(engine, err, "Context is nullptr"));
        WLOGFE("Context is nullptr");
        return;
    }
    if (winType == WindowType::WINDOW_TYPE_FLOAT || winType == WindowType::WINDOW_TYPE_FLOAT_CAMERA) {
        auto abilityContext = Context::ConvertTo<AbilityRuntime::AbilityContext>(context->lock());
        if (abilityContext != nullptr) {
            if (!CheckCallingPermission("ohos.permission.SYSTEM_FLOAT_WINDOW")) {
                int32_t err = static_cast<int32_t>(WMError::WM_ERROR_INVALID_PERMISSION);
                task.Reject(engine, CreateJsError(engine, err, "TYPE_FLOAT CheckCallingPermission failed"));
                return;
            }
        }
    }
    sptr<WindowOption> windowOption = new(std::nothrow) WindowOption();
    if (windowOption == nullptr) {
        int32_t err = static_cast<int32_t>(WMError::WM_ERROR_NULLPTR);
        task.Reject(engine, CreateJsError(engine, err, "New window option failed"));
        WLOGFE("New window option failed");
        return;
    }
    windowOption->SetWindowType(winType);
    WMError wmError = WMError::WM_OK;
    sptr<Window> window = Window::Create(windowName, windowOption, context->lock(), wmError);
    WmErrorCode wmErrorCode = WM_JS_TO_ERROR_CODE_MAP.at(wmError);
    if (window != nullptr && wmErrorCode == WmErrorCode::WM_OK) {
        task.Resolve(engine, CreateJsWindowObject(engine, window));
    } else {
        WLOGFE("Create window failed");
        int32_t err = static_cast<int32_t>(wmErrorCode);
        task.Reject(engine, CreateJsError(engine, err, "Create window failed"));
    }
}

static void CreateNewSubWindowTask(sptr<WindowOption> windowOption, NativeEngine& engine, AsyncTask& task)
{
    if (windowOption == nullptr) {
        int32_t err = static_cast<int32_t>(WmErrorCode::WM_ERROR_SYSTEM_ABNORMALLY);
        task.Reject(engine, CreateJsError(engine, err, "New window option failed"));
        WLOGFE("New window option failed");
        return;
    }
    windowOption->SetWindowMode(Rosen::WindowMode::WINDOW_MODE_FLOATING);
    if (windowOption->GetParentId() == INVALID_WINDOW_ID) {
        uint32_t parentId = GetParentId(engine);
        if (!parentId) {
            int32_t err = static_cast<int32_t>(WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
            task.Reject(engine, CreateJsError(engine, err, "parent window missed"));
            WLOGFE("can not find parent window");
            return;
        }
        windowOption->SetParentId(parentId);
    }
    sptr<Window> window = Window::Create(windowOption->GetWindowName(), windowOption);
    if (window != nullptr) {
        task.Resolve(engine, CreateJsWindowObject(engine, window));
    } else {
        WLOGFE("Create window failed");
        int32_t err = static_cast<int32_t>(WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
        task.Reject(engine, CreateJsError(engine, err, "Create window failed"));
    }
}

static void CreateSubWindowTask(uint32_t parentWinId, std::string windowName, WindowType winType,
    NativeEngine& engine, AsyncTask& task, bool newErrorCode = false)
{
    WLOGI("CreateSubWindowTask, parent id = %{public}u", parentWinId);
    sptr<WindowOption> windowOption = new(std::nothrow) WindowOption();
    if (windowOption == nullptr) {
        int32_t err = newErrorCode ? static_cast<int32_t>(WmErrorCode::WM_ERROR_SYSTEM_ABNORMALLY)
            : static_cast<int32_t>(WMError::WM_ERROR_NULLPTR);
        task.Reject(engine, CreateJsError(engine, err, "New window option failed"));
        WLOGFE("New window option failed");
        return;
    }
    windowOption->SetWindowType(winType);
    windowOption->SetWindowMode(Rosen::WindowMode::WINDOW_MODE_FLOATING);
    windowOption->SetParentId(parentWinId);
    sptr<Window> window = Window::Create(windowName, windowOption);
    if (window != nullptr) {
        task.Resolve(engine, CreateJsWindowObject(engine, window));
    } else {
        WLOGFE("Create window failed");
        int32_t err = newErrorCode ? static_cast<int32_t>(WmErrorCode::WM_ERROR_STATE_ABNORMALLY)
            : static_cast<int32_t>(WMError::WM_ERROR_NULLPTR);
        task.Reject(engine, CreateJsError(engine, err, "Create window failed"));
    }
}

static bool isConfigOptionWindowTypeValid(NativeEngine& engine, WindowOption& option)
{
    WindowType type = option.GetWindowType();
    AppExecFwk::Ability* ability = nullptr;
    bool isOldApi = GetAPI7Ability(engine, ability);
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

NativeValue* JsWindowManager::OnCreate(NativeEngine& engine, NativeCallbackInfo& info)
{
    WLOGFD("OnCreate");
    NativeValue* nativeString = nullptr;
    NativeValue* nativeContext = nullptr;
    NativeValue* nativeType = nullptr;
    NativeValue* callback = nullptr;
    if (info.argc >= 2 && info.argv[0]->TypeOf() == NATIVE_STRING) { // 2: minimum params num
        nativeString = info.argv[0];
        nativeType = info.argv[1];
        // 2: minimum params num
        callback = (info.argc == 2) ? nullptr :
            (info.argv[2]->TypeOf() == NATIVE_FUNCTION ? info.argv[2] : nullptr); // 2: index of callback
    } else if (info.argc >= 3) { // 3: minimum params num
        nativeContext = info.argv[0]->TypeOf() == NATIVE_OBJECT ? info.argv[0] : nullptr;
        nativeString = info.argv[1];
        nativeType = info.argv[2]; // 2: index of type
        // 3: minimum params num;
        callback = (info.argc == 3) ? nullptr :
            (info.argv[3]->TypeOf() == NATIVE_FUNCTION ? info.argv[3] : nullptr); // 3: index of callback
    }
    std::string windowName;
    WMError errCode = WMError::WM_OK;
    if (!ConvertFromJsValue(engine, nativeString, windowName)) {
        WLOGFE("Failed to convert parameter to windowName");
        errCode = WMError::WM_ERROR_INVALID_PARAM;
    }
    uint32_t parentId = INVALID_WINDOW_ID;
    WindowType winType = WindowType::SYSTEM_WINDOW_BASE;
    if (errCode == WMError::WM_OK &&
        !GetWindowTypeAndParentId(engine, parentId, winType, nativeString, nativeType)) {
        errCode = WMError::WM_ERROR_INVALID_PARAM;
    }
    void* contextPtr = nullptr;
    GetNativeContext(engine, nativeContext, contextPtr, errCode);

    WLOGFD("Window name = %{public}s, type = %{public}u, err = %{public}d", windowName.c_str(), winType, errCode);
    AsyncTask::CompleteCallback complete =
        [=](NativeEngine& engine, AsyncTask& task, int32_t status) {
            if (errCode != WMError::WM_OK) {
                task.Reject(engine, CreateJsError(engine, static_cast<int32_t>(errCode), "Invalidate params"));
                return;
            }
            if (parentId == INVALID_WINDOW_ID) {
                return CreateSystemWindowTask(contextPtr, windowName, winType, engine, task);
            } else {
                return CreateSubWindowTask(parentId, windowName, winType, engine, task);
            }
        };
    NativeValue* result = nullptr;
    AsyncTask::Schedule("JsWindowManager::OnCreate", engine,
        CreateAsyncTaskWithLastParam(engine, callback, nullptr, std::move(complete), &result));
    return result;
}

bool JsWindowManager::ParseConfigOption(NativeEngine& engine, NativeObject* jsObject,
    WindowOption& option, void*& contextPtr)
{
    std::string windowName;
    if (ParseJsValue(jsObject, engine, "name", windowName)) {
        option.SetWindowName(windowName);
    } else {
        WLOGFE("Failed to convert parameter to windowName");
        return false;
    }

    uint32_t winType;
    if (ParseJsValue(jsObject, engine, "windowType", winType)) {
        if (winType >= static_cast<uint32_t>(ApiWindowType::TYPE_BASE) &&
            winType < static_cast<uint32_t>(ApiWindowType::TYPE_END)) {
            option.SetWindowType(JS_TO_NATIVE_WINDOW_TYPE_MAP.at(static_cast<ApiWindowType>(winType)));
        } else {
            option.SetWindowType(static_cast<WindowType>(winType));
        }
    } else {
        WLOGFE("Failed to convert parameter to winType");
        return false;
    }

    if (!isConfigOptionWindowTypeValid(engine, option)) {
        return false;
    }

    NativeValue* value = jsObject->GetProperty("ctx");
    if (value->TypeOf() == NATIVE_UNDEFINED) {
        return true;
    }
    WMError errCode = WMError::WM_OK;
    GetNativeContext(engine, value, contextPtr, errCode);
    if (errCode != WMError::WM_OK) {
        return false;
    }

    int64_t displayId = static_cast<int64_t>(DISPLAY_ID_INVALID);
    if (ParseJsValue(jsObject, engine, "displayId", displayId)) {
        if (displayId < 0 ||
            SingletonContainer::Get<DisplayManager>().GetDisplayById(static_cast<uint64_t>(displayId)) == nullptr) {
            return false;
        }
        option.SetDisplayId(displayId);
    } else {
        return true;
    }

    int64_t parentId = -1;
    if (ParseJsValue(jsObject, engine, "parentId", parentId)) {
        option.SetParentId(parentId);
    }

    return true;
}

NativeValue* JsWindowManager::OnCreateWindow(NativeEngine& engine, NativeCallbackInfo& info)
{
    WLOGFD("Called");
    if (info.argc < 1) {
        engine.Throw(CreateJsError(engine, static_cast<int32_t>(WmErrorCode::WM_ERROR_INVALID_PARAM)));
        return engine.CreateUndefined();
    }
    NativeObject* nativeObj = ConvertNativeValueTo<NativeObject>(info.argv[0]);
    if (nativeObj == nullptr) {
        WLOGFE("Failed to convert object to CreateWindow");
        engine.Throw(CreateJsError(engine, static_cast<int32_t>(WmErrorCode::WM_ERROR_INVALID_PARAM)));
        return engine.CreateUndefined();
    }
    WindowOption option;
    void* contextPtr = nullptr;
    if (!ParseConfigOption(engine, nativeObj, option, contextPtr)) {
        WLOGFE("Failed to parse config");
        engine.Throw(CreateJsError(engine, static_cast<int32_t>(WmErrorCode::WM_ERROR_INVALID_PARAM)));
        return engine.CreateUndefined();
    }
    NativeValue* callback = nullptr;
    if (info.argc > 1) {
        callback = info.argv[1]->TypeOf() == NATIVE_FUNCTION ? info.argv[1] : nullptr; // 1: index of callback
    }
    AsyncTask::CompleteCallback complete =
        [=](NativeEngine& engine, AsyncTask& task, int32_t status) {
            sptr<WindowOption> windowOption = new WindowOption(option);
            if (WindowHelper::IsSystemWindow(option.GetWindowType())) {
                return CreateNewSystemWindowTask(contextPtr, windowOption, engine, task);
            }
            if (WindowHelper::IsSubWindow(option.GetWindowType())) {
                return CreateNewSubWindowTask(windowOption, engine, task);
            }
        };

    NativeValue* result = nullptr;
    AsyncTask::Schedule("JsWindowManager::OnCreateWindow", engine,
        CreateAsyncTaskWithLastParam(engine, callback, nullptr, std::move(complete), &result));
    return result;
}

NativeValue* JsWindowManager::OnFindWindow(NativeEngine& engine, NativeCallbackInfo& info)
{
    WLOGFD("OnFindWindow");
    std::string windowName;
    WMError errCode = WMError::WM_OK;
    if (info.argc < 1 || info.argc > 2) { // 2: maximum params num
        WLOGFE("Argc is invalid: %{public}zu", info.argc);
        errCode = WMError::WM_ERROR_INVALID_PARAM;
    } else {
        if (!ConvertFromJsValue(engine, info.argv[0], windowName)) {
            WLOGFE("Failed to convert parameter to windowName");
            errCode = WMError::WM_ERROR_INVALID_PARAM;
        }
    }
    WLOGI("Window name = %{public}s, err = %{public}d", windowName.c_str(), errCode);
    AsyncTask::CompleteCallback complete =
        [=](NativeEngine& engine, AsyncTask& task, int32_t status) {
            if (errCode != WMError::WM_OK) {
                task.Reject(engine, CreateJsError(engine, static_cast<int32_t>(errCode), "Invalidate params"));
                return;
            }
            HITRACE_METER_FMT(HITRACE_TAG_WINDOW_MANAGER, "WM:Find %s", windowName.c_str());
            std::shared_ptr<NativeReference> jsWindowObj = FindJsWindowObject(windowName);
            if (jsWindowObj != nullptr && jsWindowObj->Get() != nullptr) {
                WLOGI("Find window: %{public}s, use exist js window", windowName.c_str());
                task.Resolve(engine, jsWindowObj->Get());
            } else {
                sptr<Window> window = Window::Find(windowName);
                if (window == nullptr) {
                    WLOGFE("Cannot find window: %{public}s", windowName.c_str());
                    task.Reject(engine, CreateJsError(engine,
                        static_cast<int32_t>(WMError::WM_ERROR_NULLPTR), "Cannot find window"));
                } else {
                    task.Resolve(engine, CreateJsWindowObject(engine, window));
                    WLOGI("Find window: %{public}s, create js window", windowName.c_str());
                }
            }
        };

    NativeValue* lastParam = (info.argc <= 1) ? nullptr :
        (info.argv[1]->TypeOf() == NATIVE_FUNCTION ? info.argv[1] : nullptr);
    NativeValue* result = nullptr;
    AsyncTask::Schedule("JsWindowManager::OnFindWindow",
        engine, CreateAsyncTaskWithLastParam(engine, lastParam, nullptr, std::move(complete), &result));
    return result;
}

NativeValue* JsWindowManager::OnFindWindowSync(NativeEngine& engine, NativeCallbackInfo& info)
{
    WLOGFD("OnFindWindowSync");
    std::string windowName;
    WmErrorCode errCode = WmErrorCode::WM_OK;
    if (info.argc < 1) { // 1: params num
        WLOGFE("Argc is invalid: %{public}zu", info.argc);
        errCode = WmErrorCode::WM_ERROR_INVALID_PARAM;
    } else {
        if (!ConvertFromJsValue(engine, info.argv[0], windowName)) {
            WLOGFE("Failed to convert parameter to windowName");
            errCode = WmErrorCode::WM_ERROR_INVALID_PARAM;
        }
    }
    if (errCode == WmErrorCode::WM_ERROR_INVALID_PARAM) {
        engine.Throw(CreateJsError(engine, static_cast<int32_t>(WmErrorCode::WM_ERROR_INVALID_PARAM)));
        return engine.CreateUndefined();
    }

    WLOGI("Window name = %{public}s, err = %{public}d", windowName.c_str(), errCode);
    std::shared_ptr<NativeReference> jsWindowObj = FindJsWindowObject(windowName);
    if (jsWindowObj != nullptr && jsWindowObj->Get() != nullptr) {
        WLOGI("Find window: %{public}s, use exist js window", windowName.c_str());
        return jsWindowObj->Get();
    } else {
        sptr<Window> window = Window::Find(windowName);
        if (window == nullptr) {
            engine.Throw(CreateJsError(engine, static_cast<int32_t>(WmErrorCode::WM_ERROR_STATE_ABNORMALLY)));
            return engine.CreateUndefined();
        } else {
            return CreateJsWindowObject(engine, window);
        }
    }
}

NativeValue* JsWindowManager::OnMinimizeAll(NativeEngine& engine, NativeCallbackInfo& info)
{
    WLOGI("OnMinimizeAll");
    WmErrorCode errCode = WmErrorCode::WM_OK;
    if (info.argc < 1) {
        WLOGFE("Argc is invalid: %{public}zu", info.argc);
        errCode = WmErrorCode::WM_ERROR_INVALID_PARAM;
    }
    int64_t displayId = static_cast<int64_t>(DISPLAY_ID_INVALID);
    if (errCode == WmErrorCode::WM_OK && !ConvertFromJsValue(engine, info.argv[0], displayId)) {
        WLOGFE("Failed to convert parameter to displayId");
        errCode = WmErrorCode::WM_ERROR_INVALID_PARAM;
    }
    if (displayId < 0 ||
        SingletonContainer::Get<DisplayManager>().GetDisplayById(static_cast<uint64_t>(displayId)) == nullptr) {
        errCode = WmErrorCode::WM_ERROR_INVALID_PARAM;
    }
    if (errCode == WmErrorCode::WM_ERROR_INVALID_PARAM) {
        WLOGFE("JsWindowManager::OnMinimizeAll failed, Invalidate params.");
        engine.Throw(CreateJsError(engine, static_cast<int32_t>(WmErrorCode::WM_ERROR_INVALID_PARAM)));
        return engine.CreateUndefined();
    }

    WLOGI("Display id = %{public}" PRIu64", err = %{public}d", static_cast<uint64_t>(displayId), errCode);
    AsyncTask::CompleteCallback complete =
        [=](NativeEngine& engine, AsyncTask& task, int32_t status) {
            HITRACE_METER_FMT(HITRACE_TAG_WINDOW_MANAGER, "WM:MinimizeAll: " PRIu64"",
                static_cast<uint64_t>(displayId));
            WmErrorCode ret = WM_JS_TO_ERROR_CODE_MAP.at(
                SingletonContainer::Get<WindowManager>().MinimizeAllAppWindows(static_cast<uint64_t>(displayId)));
            if (ret == WmErrorCode::WM_OK) {
                task.Resolve(engine, engine.CreateUndefined());
                WLOGFI("OnMinimizeAll success");
            } else {
                task.Reject(engine, CreateJsError(engine, static_cast<int32_t>(ret), "OnMinimizeAll failed"));
            }
        };
    NativeValue* lastParam = (info.argc <= 1) ? nullptr :
        ((info.argv[1] != nullptr && info.argv[1]->TypeOf() == NATIVE_FUNCTION) ?
        info.argv[1] : nullptr);
    NativeValue* result = nullptr;
    AsyncTask::Schedule("JsWindowManager::OnMinimizeAll",
        engine, CreateAsyncTaskWithLastParam(engine, lastParam, nullptr, std::move(complete), &result));
    return result;
}

NativeValue* JsWindowManager::OnToggleShownStateForAllAppWindows(NativeEngine& engine, NativeCallbackInfo& info)
{
    WLOGI("OnToggleShownStateForAllAppWindows");
    AsyncTask::CompleteCallback complete =
        [=](NativeEngine& engine, AsyncTask& task, int32_t status) {
            WmErrorCode ret = WM_JS_TO_ERROR_CODE_MAP.at(
                SingletonContainer::Get<WindowManager>().ToggleShownStateForAllAppWindows());
            if (ret == WmErrorCode::WM_OK) {
                task.Resolve(engine, engine.CreateUndefined());
                WLOGI("OnToggleShownStateForAllAppWindows success");
            } else {
                task.Reject(engine, CreateJsError(engine, static_cast<int32_t>(ret),
                    "OnToggleShownStateForAllAppWindows failed"));
            }
        };
    NativeValue* lastParam = (info.argc <= 0) ? nullptr :
        (info.argv[0]->TypeOf() == NATIVE_FUNCTION ? info.argv[0] : nullptr);
    NativeValue* result = nullptr;
    AsyncTask::Schedule("JsWindowManager::OnToggleShownStateForAllAppWindows",
        engine, CreateAsyncTaskWithLastParam(engine, lastParam, nullptr, std::move(complete), &result));
    return result;
}

NativeValue* JsWindowManager::OnRegisterWindowManagerCallback(NativeEngine& engine, NativeCallbackInfo& info)
{
    WLOGFD("OnRegisterWindowManagerCallback");
    if (info.argc < 2) { // 2: params num
        WLOGFE("Argc is invalid: %{public}zu", info.argc);
        engine.Throw(CreateJsError(engine, static_cast<int32_t>(WmErrorCode::WM_ERROR_INVALID_PARAM)));
        return engine.CreateUndefined();
    }
    std::string cbType;
    if (!ConvertFromJsValue(engine, info.argv[0], cbType)) {
        WLOGFE("Failed to convert parameter to callbackType");
        engine.Throw(CreateJsError(engine, static_cast<int32_t>(WmErrorCode::WM_ERROR_INVALID_PARAM)));
        return engine.CreateUndefined();
    }
    NativeValue* value = info.argv[1];
    if (!value->IsCallable()) {
        WLOGI("Callback(argv[1]) is not callable");
        engine.Throw(CreateJsError(engine, static_cast<int32_t>(WmErrorCode::WM_ERROR_INVALID_PARAM)));
        return engine.CreateUndefined();
    }

    WmErrorCode ret = registerManager_->RegisterListener(nullptr, cbType, CaseType::CASE_WINDOW_MANAGER, engine, value);
    if (ret != WmErrorCode::WM_OK) {
        engine.Throw(CreateJsError(engine, static_cast<int32_t>(ret)));
        return engine.CreateUndefined();
    }
    WLOGI("Register end, type = %{public}s", cbType.c_str());
    return engine.CreateUndefined();
}

NativeValue* JsWindowManager::OnUnregisterWindowManagerCallback(NativeEngine& engine, NativeCallbackInfo& info)
{
    WLOGFD("OnUnregisterWindowManagerCallback");
    if (info.argc < 1) {
        WLOGFE("Argc is invalid: %{public}zu", info.argc);
        engine.Throw(CreateJsError(engine, static_cast<int32_t>(WmErrorCode::WM_ERROR_INVALID_PARAM)));
        return engine.CreateUndefined();
    }
    std::string cbType;
    if (!ConvertFromJsValue(engine, info.argv[0], cbType)) {
        WLOGFE("Failed to convert parameter to callbackType");
        engine.Throw(CreateJsError(engine, static_cast<int32_t>(WmErrorCode::WM_ERROR_INVALID_PARAM)));
        return engine.CreateUndefined();
    }

    NativeValue* value = nullptr;
    WmErrorCode ret = WmErrorCode::WM_OK;
    if (info.argc == 1) {
        ret = registerManager_->UnregisterListener(nullptr, cbType, CaseType::CASE_WINDOW_MANAGER, value);
    } else {
        value = info.argv[1];
        if ((value == nullptr) || (!value->IsCallable())) {
            ret = registerManager_->UnregisterListener(nullptr, cbType, CaseType::CASE_WINDOW_MANAGER, nullptr);
        } else {
            ret = registerManager_->UnregisterListener(nullptr, cbType, CaseType::CASE_WINDOW_MANAGER, value);
        }
    }
    if (ret != WmErrorCode::WM_OK) {
        engine.Throw(CreateJsError(engine, static_cast<int32_t>(ret)));
        return engine.CreateUndefined();
    }
    WLOGI("Unregister end, type = %{public}s", cbType.c_str());
    return engine.CreateUndefined();
}

static void GetTopWindowTask(void* contextPtr, NativeEngine& engine, AsyncTask& task, bool newApi)
{
    std::string windowName;
    sptr<Window> window = nullptr;
    AppExecFwk::Ability* ability = nullptr;
    bool isOldApi = GetAPI7Ability(engine, ability);
    int32_t error;
    if (isOldApi) {
        if (ability->GetWindow() == nullptr) {
            error = newApi ? static_cast<int32_t>(WmErrorCode::WM_ERROR_STATE_ABNORMALLY) :
                static_cast<int32_t>(WMError::WM_ERROR_NULLPTR);
            task.Reject(engine, CreateJsError(engine, error, "FA mode can not get ability window"));
            WLOGE("FA mode can not get ability window");
            return;
        }
        window = Window::GetTopWindowWithId(ability->GetWindow()->GetWindowId());
    } else {
        auto context = static_cast<std::weak_ptr<AbilityRuntime::Context>*>(contextPtr);
        if (contextPtr == nullptr || context == nullptr) {
            error = newApi ? static_cast<int32_t>(WmErrorCode::WM_ERROR_STATE_ABNORMALLY) :
                static_cast<int32_t>(WMError::WM_ERROR_NULLPTR);
            task.Reject(engine, CreateJsError(engine, error, "Stage mode without context"));
            WLOGFE("Stage mode without context");
            return;
        }
        window = Window::GetTopWindowWithContext(context->lock());
    }
    if (window == nullptr) {
        error = newApi ? static_cast<int32_t>(WmErrorCode::WM_ERROR_STATE_ABNORMALLY) :
            static_cast<int32_t>(WMError::WM_ERROR_NULLPTR);
        task.Reject(engine, CreateJsError(engine, error, "Get top window failed"));
        WLOGFE("Get top window failed");
        return;
    }
    windowName = window->GetWindowName();
    std::shared_ptr<NativeReference> jsWindowObj = FindJsWindowObject(windowName);
    if (jsWindowObj != nullptr && jsWindowObj->Get() != nullptr) {
        task.Resolve(engine, jsWindowObj->Get());
    } else {
        task.Resolve(engine, CreateJsWindowObject(engine, window));
    }
    WLOGI("Get top window %{public}s success", windowName.c_str());
    return;
}

NativeValue* JsWindowManager::OnGetTopWindow(NativeEngine& engine, NativeCallbackInfo& info)
{
    WLOGFD("OnGetTopWindow");
    WMError errCode = WMError::WM_OK;
    NativeValue* nativeContext = nullptr;
    NativeValue* nativeCallback = nullptr;
    void* contextPtr = nullptr;
    if (info.argc > 2) { // 2: maximum params num
        WLOGFE("Argc is invalid: %{public}zu", info.argc);
        errCode = WMError::WM_ERROR_INVALID_PARAM;
    } else {
        if (info.argc > 0 && info.argv[0]->TypeOf() == NATIVE_OBJECT) { // (context, callback?)
            nativeContext = info.argv[0];
            nativeCallback = (info.argc == 1) ? nullptr :
                (info.argv[1]->TypeOf() == NATIVE_FUNCTION ? info.argv[1] : nullptr);
        } else { // (callback?)
            nativeCallback = (info.argc == 0) ? nullptr :
                (info.argv[0]->TypeOf() == NATIVE_FUNCTION ? info.argv[0] : nullptr);
        }
        GetNativeContext(engine, nativeContext, contextPtr, errCode);
    }

    WLOGI("err %{public}u", errCode);
    AsyncTask::CompleteCallback complete =
        [=](NativeEngine& engine, AsyncTask& task, int32_t status) {
            if (errCode != WMError::WM_OK) {
                task.Reject(engine, CreateJsError(engine, static_cast<int32_t>(errCode), "Invalidate params"));
                return;
            }
            return GetTopWindowTask(contextPtr, engine, task, false);
        };
    NativeValue* result = nullptr;
    AsyncTask::Schedule("JsWindowManager::OnGetTopWindow",
        engine, CreateAsyncTaskWithLastParam(engine, nativeCallback, nullptr, std::move(complete), &result));
    return result;
}

NativeValue* JsWindowManager::OnGetLastWindow(NativeEngine& engine, NativeCallbackInfo& info)
{
    WLOGFD("OnGetLastWindow");
    WMError errCode = WMError::WM_OK;
    NativeValue* nativeContext = nullptr;
    NativeValue* nativeCallback = nullptr;
    void* contextPtr = nullptr;
    if (info.argc < 1) {
        WLOGFE("Argc is invalid: %{public}zu", info.argc);
        engine.Throw(CreateJsError(engine, static_cast<int32_t>(WmErrorCode::WM_ERROR_INVALID_PARAM)));
        return engine.CreateUndefined();
    } else {
        nativeContext = info.argv[0];
        nativeCallback = (info.argc == 1) ? nullptr : info.argv[1];
        GetNativeContext(engine, nativeContext, contextPtr, errCode);
    }
    if (errCode != WMError::WM_OK) {
        engine.Throw(CreateJsError(engine, static_cast<int32_t>(WmErrorCode::WM_ERROR_INVALID_PARAM)));
        return engine.CreateUndefined();
    }

    AsyncTask::CompleteCallback complete =
        [=](NativeEngine& engine, AsyncTask& task, int32_t status) {
            return GetTopWindowTask(contextPtr, engine, task, true);
        };
    NativeValue* result = nullptr;
    AsyncTask::Schedule("JsWindowManager::OnGetTopWindow",
        engine, CreateAsyncTaskWithLastParam(engine, nativeCallback, nullptr, std::move(complete), &result));
    return result;
}

NativeValue* JsWindowManager::OnSetWindowLayoutMode(NativeEngine& engine, NativeCallbackInfo& info)
{
    WLOGFD("OnSetWindowLayoutMode");
    WmErrorCode errCode = WmErrorCode::WM_OK;
    if (info.argc < 1) { // 1: minimum params num
        WLOGFE("Argc is invalid: %{public}zu", info.argc);
        errCode = WmErrorCode::WM_ERROR_INVALID_PARAM;
    }
    WindowLayoutMode winLayoutMode = WindowLayoutMode::CASCADE;
    if (errCode == WmErrorCode::WM_OK) {
        NativeNumber* nativeMode = ConvertNativeValueTo<NativeNumber>(info.argv[0]);
        if (nativeMode == nullptr) {
            WLOGFE("Failed to convert parameter to windowLayoutMode");
            errCode = WmErrorCode::WM_ERROR_INVALID_PARAM;
        } else {
            winLayoutMode = static_cast<WindowLayoutMode>(static_cast<uint32_t>(*nativeMode));
        }
    }
    if (winLayoutMode != WindowLayoutMode::CASCADE && winLayoutMode != WindowLayoutMode::TILE) {
        errCode = WmErrorCode::WM_ERROR_INVALID_PARAM;
    }
    if (errCode == WmErrorCode::WM_ERROR_INVALID_PARAM) {
        WLOGFE("JsWindowManager::OnSetWindowLayoutMode failed, Invalidate params.");
        engine.Throw(CreateJsError(engine, static_cast<int32_t>(WmErrorCode::WM_ERROR_INVALID_PARAM)));
        return engine.CreateUndefined();
    }

    WLOGI("LayoutMode = %{public}u, err = %{public}d", winLayoutMode, errCode);
    AsyncTask::CompleteCallback complete =
        [=](NativeEngine& engine, AsyncTask& task, int32_t status) {
            WmErrorCode ret = WM_JS_TO_ERROR_CODE_MAP.at(
                SingletonContainer::Get<WindowManager>().SetWindowLayoutMode(winLayoutMode));
            if (ret == WmErrorCode::WM_OK) {
                task.Resolve(engine, engine.CreateUndefined());
                WLOGD("SetWindowLayoutMode success");
            } else {
                task.Reject(engine, CreateJsError(engine, static_cast<int32_t>(ret), "SetWindowLayoutMode failed"));
            }
        };
    // 1: maximum params num; 1: index of callback
    NativeValue* lastParam = (info.argc <= 1) ? nullptr :
        ((info.argv[1] != nullptr && info.argv[1]->TypeOf() == NATIVE_FUNCTION) ?
        info.argv[1] : nullptr);
    NativeValue* result = nullptr;
    AsyncTask::Schedule("JsWindowManager::OnSetWindowLayoutMode",
        engine, CreateAsyncTaskWithLastParam(engine, lastParam, nullptr, std::move(complete), &result));
    return result;
}

NativeValue* JsWindowManager::OnSetGestureNavigationEnabled(NativeEngine& engine, NativeCallbackInfo& info)
{
    WLOGFD("OnSetGestureNavigationEnabled");
    if (info.argc < 1) { // 1: minimum params num
        WLOGFE("Argc is invalid: %{public}zu", info.argc);
        engine.Throw(CreateJsError(engine, static_cast<int32_t>(WmErrorCode::WM_ERROR_INVALID_PARAM)));
        return engine.CreateUndefined();
    }

    NativeBoolean* nativeBool = ConvertNativeValueTo<NativeBoolean>(info.argv[0]);
    if (nativeBool == nullptr) {
        WLOGFE("Failed to convert parameter to bool");
        engine.Throw(CreateJsError(engine, static_cast<int32_t>(WmErrorCode::WM_ERROR_INVALID_PARAM)));
        return engine.CreateUndefined();
    }
    bool gestureNavigationEnable = static_cast<bool>(*nativeBool);

    WLOGI("Set gesture navigation enable as %{public}d", gestureNavigationEnable);
    AsyncTask::CompleteCallback complete =
        [gestureNavigationEnable](NativeEngine& engine, AsyncTask& task, int32_t status) {
            WmErrorCode ret = WM_JS_TO_ERROR_CODE_MAP.at(
                SingletonContainer::Get<WindowManager>().SetGestureNavigaionEnabled(gestureNavigationEnable));
            if (ret == WmErrorCode::WM_OK) {
                task.Resolve(engine, engine.CreateUndefined());
                WLOGD("SetGestureNavigationEnabled success");
            } else {
                task.Reject(engine, CreateJsError(engine,
                    static_cast<int32_t>(ret), "SetGestureNavigationEnabled failed"));
            }
        };
    // 1: maximum params num; 1: index of callback
    NativeValue* lastParam = (info.argc <= 1) ? nullptr :
        ((info.argv[1] != nullptr && info.argv[1]->TypeOf() == NATIVE_FUNCTION) ?
        info.argv[1] : nullptr);
    NativeValue* result = nullptr;
    AsyncTask::Schedule("JsWindowManager::OnSetGestureNavigationEnabled",
        engine, CreateAsyncTaskWithLastParam(engine, lastParam, nullptr, std::move(complete), &result));
    return result;
}

NativeValue* JsWindowManager::OnSetWaterMarkImage(NativeEngine& engine, NativeCallbackInfo& info)
{
    WLOGFD("OnSetWaterMarkImage");
    NativeValue* nativeObject = nullptr;
    NativeValue* nativeBoolean = nullptr;
    if (info.argc < 2) { // 2: params num
        WLOGFE("Argc is invalid: %{public}zu", info.argc);
        engine.Throw(CreateJsError(engine, static_cast<int32_t>(WmErrorCode::WM_ERROR_INVALID_PARAM)));
        return engine.CreateUndefined();
    } else {
        if (info.argc > 0 && info.argv[0]->TypeOf() == NATIVE_OBJECT) {
            nativeObject = info.argv[0];
            nativeBoolean = (info.argc == 1) ? nullptr :
                (info.argv[1]->TypeOf() == NATIVE_BOOLEAN ? info.argv[1] : nullptr);
        }
    }
    
    std::shared_ptr<Media::PixelMap> pixelMap;
    pixelMap = OHOS::Media::PixelMapNapi::GetPixelMap(reinterpret_cast<napi_env>(&engine),
        reinterpret_cast<napi_value>(nativeObject));
    if (pixelMap == nullptr) {
        WLOGFE("Failed to convert parameter to PixelMap");
        engine.Throw(CreateJsError(engine, static_cast<int32_t>(WmErrorCode::WM_ERROR_INVALID_PARAM)));
        return engine.CreateUndefined();
    }

    NativeBoolean* nativeBool = ConvertNativeValueTo<NativeBoolean>(nativeBoolean);
    if (nativeBool == nullptr) {
        WLOGFE("Failed to convert parameter to bool");
        engine.Throw(CreateJsError(engine, static_cast<int32_t>(WmErrorCode::WM_ERROR_INVALID_PARAM)));
        return engine.CreateUndefined();
    }
    bool isShow = static_cast<bool>(*nativeBool);

    if (!Permission::IsSystemCalling()) {
        WLOGFE("set watermark image permission denied!");
        engine.Throw(CreateJsError(engine, static_cast<int32_t>(WmErrorCode::WM_ERROR_NOT_SYSTEM_APP)));
        return engine.CreateUndefined();
    }

    AsyncTask::CompleteCallback complete =
        [=](NativeEngine& engine, AsyncTask& task, int32_t status) {
            RSInterfaces::GetInstance().ShowWatermark(pixelMap, isShow);
            task.Resolve(engine, engine.CreateUndefined());
            WLOGD("OnSetWaterMarkImage success");
        };
    // 2: maximum params num; 2: index of callback
    NativeValue* lastParam = (info.argc <= 2) ? nullptr :
        (info.argv[2]->TypeOf() == NATIVE_FUNCTION ? info.argv[2] : nullptr);
    NativeValue* result = nullptr;
    AsyncTask::Schedule("JsWindowManager::OnSetWaterMarkImage",
        engine, CreateAsyncTaskWithLastParam(engine, lastParam, nullptr, std::move(complete), &result));
    return result;
}

NativeValue* JsWindowManagerInit(NativeEngine* engine, NativeValue* exportObj)
{
    WLOGFD("JsWindowManagerInit");

    if (engine == nullptr || exportObj == nullptr) {
        WLOGFE("JsWindowManagerInit engine or exportObj is nullptr");
        return nullptr;
    }

    NativeObject* object = ConvertNativeValueTo<NativeObject>(exportObj);
    if (object == nullptr) {
        WLOGFE("JsWindowManagerInit object is nullptr");
        return nullptr;
    }

    std::unique_ptr<JsWindowManager> jsWinManager = std::make_unique<JsWindowManager>();
    object->SetNativePointer(jsWinManager.release(), JsWindowManager::Finalizer, nullptr);
    object->SetProperty("WindowType", WindowTypeInit(engine));
    object->SetProperty("AvoidAreaType", AvoidAreaTypeInit(engine));
    object->SetProperty("WindowMode", WindowModeInit(engine));
    object->SetProperty("ColorSpace", ColorSpaceInit(engine));
    object->SetProperty("WindowStageEventType", WindowStageEventTypeInit(engine));
    object->SetProperty("WindowEventType", WindowEventTypeInit(engine));
    object->SetProperty("WindowLayoutMode", WindowLayoutModeInit(engine));
    object->SetProperty("Orientation", OrientationInit(engine));
    object->SetProperty("BlurStyle", BlurStyleInit(engine));
    object->SetProperty("WmErrorCode", WindowErrorCodeInit(engine));
    object->SetProperty("WMError", WindowErrorInit(engine));
    const char *moduleName = "JsWindowManager";
    BindNativeFunction(*engine, *object, "create", moduleName, JsWindowManager::Create);
    BindNativeFunction(*engine, *object, "createWindow", moduleName, JsWindowManager::CreateWindow);
    BindNativeFunction(*engine, *object, "find", moduleName, JsWindowManager::FindWindow);
    BindNativeFunction(*engine, *object, "findWindow", moduleName, JsWindowManager::FindWindowSync);
    BindNativeFunction(*engine, *object, "on", moduleName, JsWindowManager::RegisterWindowManagerCallback);
    BindNativeFunction(*engine, *object, "off", moduleName, JsWindowManager::UnregisterWindowMangerCallback);
    BindNativeFunction(*engine, *object, "getTopWindow", moduleName, JsWindowManager::GetTopWindow);
    BindNativeFunction(*engine, *object, "getLastWindow", moduleName, JsWindowManager::GetLastWindow);
    BindNativeFunction(*engine, *object, "minimizeAll", moduleName, JsWindowManager::MinimizeAll);
    BindNativeFunction(*engine, *object, "toggleShownStateForAllAppWindows", moduleName,
        JsWindowManager::ToggleShownStateForAllAppWindows);
    BindNativeFunction(*engine, *object, "setWindowLayoutMode", moduleName, JsWindowManager::SetWindowLayoutMode);
    BindNativeFunction(*engine, *object, "setGestureNavigationEnabled", moduleName,
        JsWindowManager::SetGestureNavigationEnabled);
    BindNativeFunction(*engine, *object, "setWaterMarkImage", moduleName, JsWindowManager::SetWaterMarkImage);
    return engine->CreateUndefined();
}
}  // namespace Rosen
}  // namespace OHOS
