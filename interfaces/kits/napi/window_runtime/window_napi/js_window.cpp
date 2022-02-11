/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#include "js_window.h"
#include "window.h"
#include "window_manager_hilog.h"
#include "window_option.h"
namespace OHOS {
namespace Rosen {
using namespace AbilityRuntime;
namespace {
    constexpr HiviewDFX::HiLogLabel LABEL = {LOG_CORE, HILOG_DOMAIN_WINDOW, "JsWindow"};
    constexpr Rect EMPTY_RECT = {0, 0, 0, 0};
}

static std::map<std::string, std::shared_ptr<NativeReference>> g_jsWindowMap;
std::recursive_mutex g_mutex;
JsWindow::JsWindow(const sptr<Window>& window) : windowToken_(window)
{
}

JsWindow::~JsWindow()
{
    WLOGFI("JsWindow::~JsWindow is called");
}

std::string JsWindow::GetWindowName()
{
    if (windowToken_ == nullptr) {
        return "";
    }
    return windowToken_->GetWindowName();
}

void JsWindow::Finalizer(NativeEngine* engine, void* data, void* hint)
{
    WLOGFI("JsWindow::Finalizer is called");
    auto jsWin = std::unique_ptr<JsWindow>(static_cast<JsWindow*>(data));
    std::string windowName = jsWin->GetWindowName();
    WLOGFI("JsWindow::Finalizer windowName : %{public}s", windowName.c_str());
    std::lock_guard<std::recursive_mutex> lock(g_mutex);
    if (g_jsWindowMap.find(windowName) != g_jsWindowMap.end()) {
        WLOGFI("JsWindow::windowName %{public}s is destroyed", windowName.c_str());
        g_jsWindowMap.erase(windowName);
    }
}

NativeValue* JsWindow::Show(NativeEngine* engine, NativeCallbackInfo* info)
{
    WLOGFI("JsWindow::Show is called");
    JsWindow* me = CheckParamsAndGetThis<JsWindow>(engine, info);
    return (me != nullptr) ? me->OnShow(*engine, *info) : nullptr;
}

NativeValue* JsWindow::Destroy(NativeEngine* engine, NativeCallbackInfo* info)
{
    WLOGFI("JsWindow::Destroy is called");
    JsWindow* me = CheckParamsAndGetThis<JsWindow>(engine, info);
    return (me != nullptr) ? me->OnDestroy(*engine, *info) : nullptr;
}

NativeValue* JsWindow::Hide(NativeEngine* engine, NativeCallbackInfo* info)
{
    WLOGFI("JsWindow::Hide is called");
    JsWindow* me = CheckParamsAndGetThis<JsWindow>(engine, info);
    return (me != nullptr) ? me->OnHide(*engine, *info) : nullptr;
}

NativeValue* JsWindow::MoveTo(NativeEngine* engine, NativeCallbackInfo* info)
{
    WLOGFI("JsWindow::MoveTo is called");
    JsWindow* me = CheckParamsAndGetThis<JsWindow>(engine, info);
    return (me != nullptr) ? me->OnMoveTo(*engine, *info) : nullptr;
}

NativeValue* JsWindow::Resize(NativeEngine* engine, NativeCallbackInfo* info)
{
    WLOGFI("JsWindow::Resize is called");
    JsWindow* me = CheckParamsAndGetThis<JsWindow>(engine, info);
    return (me != nullptr) ? me->OnResize(*engine, *info) : nullptr;
}

NativeValue* JsWindow::SetWindowType(NativeEngine* engine, NativeCallbackInfo* info)
{
    WLOGFI("JsWindow::SetWindowType is called");
    JsWindow* me = CheckParamsAndGetThis<JsWindow>(engine, info);
    return (me != nullptr) ? me->OnSetWindowType(*engine, *info) : nullptr;
}

NativeValue* JsWindow::SetWindowMode(NativeEngine* engine, NativeCallbackInfo* info)
{
    WLOGFI("JsWindow::SetWindowMode is called");
    JsWindow* me = CheckParamsAndGetThis<JsWindow>(engine, info);
    return (me != nullptr) ? me->OnSetWindowMode(*engine, *info) : nullptr;
}

NativeValue* JsWindow::GetProperties(NativeEngine* engine, NativeCallbackInfo* info)
{
    WLOGFI("JsWindow::GetProperties is called");
    JsWindow* me = CheckParamsAndGetThis<JsWindow>(engine, info);
    return (me != nullptr) ? me->OnGetProperties(*engine, *info) : nullptr;
}

NativeValue* JsWindow::RegisterWindowCallback(NativeEngine* engine, NativeCallbackInfo* info)
{
    WLOGFI("JsWindow::RegisterWindowCallback is called");
    JsWindow* me = CheckParamsAndGetThis<JsWindow>(engine, info);
    return (me != nullptr) ? me->OnRegisterWindowCallback(*engine, *info) : nullptr;
}

NativeValue* JsWindow::UnregisterWindowCallback(NativeEngine* engine, NativeCallbackInfo* info)
{
    WLOGFI("JsWindow::UnregisterWindowCallback is called");
    JsWindow* me = CheckParamsAndGetThis<JsWindow>(engine, info);
    return (me != nullptr) ? me->OnUnregisterWindowCallback(*engine, *info) : nullptr;
}

NativeValue* JsWindow::LoadContent(NativeEngine* engine, NativeCallbackInfo* info)
{
    WLOGFI("JsWindow::LoadContent is called");
    JsWindow* me = CheckParamsAndGetThis<JsWindow>(engine, info);
    return (me != nullptr) ? me->OnLoadContent(*engine, *info) : nullptr;
}

NativeValue* JsWindow::SetFullScreen(NativeEngine* engine, NativeCallbackInfo* info)
{
    WLOGFI("JsWindow::SetFullScreen is called");
    JsWindow* me = CheckParamsAndGetThis<JsWindow>(engine, info);
    return (me != nullptr) ? me->OnSetFullScreen(*engine, *info) : nullptr;
}

NativeValue* JsWindow::SetLayoutFullScreen(NativeEngine* engine, NativeCallbackInfo* info)
{
    WLOGFI("JsWindow::SetLayoutFullScreen is called");
    JsWindow* me = CheckParamsAndGetThis<JsWindow>(engine, info);
    return (me != nullptr) ? me->OnSetLayoutFullScreen(*engine, *info) : nullptr;
}

NativeValue* JsWindow::SetSystemBarEnable(NativeEngine* engine, NativeCallbackInfo* info)
{
    WLOGFI("JsWindow::SetSystemBarEnable is called");
    JsWindow* me = CheckParamsAndGetThis<JsWindow>(engine, info);
    return (me != nullptr) ? me->OnSetSystemBarEnable(*engine, *info) : nullptr;
}

NativeValue* JsWindow::SetSystemBarProperties(NativeEngine* engine, NativeCallbackInfo* info)
{
    WLOGFI("JsWindow::SetBarProperties is called");
    JsWindow* me = CheckParamsAndGetThis<JsWindow>(engine, info);
    return (me != nullptr) ? me->OnSetSystemBarProperties(*engine, *info) : nullptr;
}

NativeValue* JsWindow::GetAvoidArea(NativeEngine* engine, NativeCallbackInfo* info)
{
    WLOGFI("JsWindow::GetAvoidArea is called");
    JsWindow* me = CheckParamsAndGetThis<JsWindow>(engine, info);
    return (me != nullptr) ? me->OnGetAvoidArea(*engine, *info) : nullptr;
}

NativeValue* JsWindow::IsShowing(NativeEngine* engine, NativeCallbackInfo* info)
{
    WLOGFI("JsWindow::IsShowing is called");
    JsWindow* me = CheckParamsAndGetThis<JsWindow>(engine, info);
    return (me != nullptr) ? me->OnIsShowing(*engine, *info) : nullptr;
}

NativeValue* JsWindow::IsSupportWideGamut(NativeEngine* engine, NativeCallbackInfo* info)
{
    WLOGFI("JsWindow::IsSupportWideGamut is called");
    JsWindow* me = CheckParamsAndGetThis<JsWindow>(engine, info);
    return (me != nullptr) ? me->OnIsSupportWideGamut(*engine, *info) : nullptr;
}

NativeValue* JsWindow::SetColorSpace(NativeEngine* engine, NativeCallbackInfo* info)
{
    WLOGFI("JsWindow::SetColorSpace is called");
    JsWindow* me = CheckParamsAndGetThis<JsWindow>(engine, info);
    return (me != nullptr) ? me->OnSetColorSpace(*engine, *info) : nullptr;
}

NativeValue* JsWindow::GetColorSpace(NativeEngine* engine, NativeCallbackInfo* info)
{
    WLOGFI("JsWindow::GetColorSpace is called");
    JsWindow* me = CheckParamsAndGetThis<JsWindow>(engine, info);
    return (me != nullptr) ? me->OnGetColorSpace(*engine, *info) : nullptr;
}

NativeValue* JsWindow::OnShow(NativeEngine& engine, NativeCallbackInfo& info)
{
    WLOGFI("JsWindow::OnShow is called");
    WMError errCode = WMError::WM_OK;
    if (windowToken_ == nullptr) {
        WLOGFE("JsWindow windowToken_ is nullptr");
        errCode = WMError::WM_ERROR_INVALID_PARAM;
    }
    AsyncTask::CompleteCallback complete =
        [=](NativeEngine& engine, AsyncTask& task, int32_t status) {
            if (errCode != WMError::WM_OK) {
                task.Reject(engine, CreateJsError(engine, static_cast<int32_t>(errCode), "Invalidate params."));
                return;
            }
            WMError ret = windowToken_->Show();
            if (ret == WMError::WM_OK) {
                task.Resolve(engine, engine.CreateUndefined());
                WLOGFI("JsWindow::OnShow success");
            } else {
                task.Reject(engine, CreateJsError(engine, static_cast<int32_t>(ret), "ShowWindow failed."));
            }
        };

    NativeValue* lastParam = (info.argc == 0) ? nullptr : info.argv[0];
    NativeValue* result = nullptr;
    AsyncTask::Schedule(
        engine, CreateAsyncTaskWithLastParam(engine, lastParam, nullptr, std::move(complete), &result));
    return result;
}

NativeValue* JsWindow::OnDestroy(NativeEngine& engine, NativeCallbackInfo& info)
{
    WLOGFI("JsWindow::OnDestroy is called");
    WMError errCode = WMError::WM_OK;
    if (windowToken_ == nullptr) {
        WLOGFE("JsWindow windowToken_ is nullptr");
        errCode = WMError::WM_ERROR_INVALID_PARAM;
    }
    AsyncTask::CompleteCallback complete =
        [=](NativeEngine& engine, AsyncTask& task, int32_t status) {
            if (errCode != WMError::WM_OK) {
                task.Reject(engine, CreateJsError(engine, static_cast<int32_t>(errCode), "Invalidate params."));
                return;
            }
            WMError ret = windowToken_->Destroy();
            if (ret != WMError::WM_OK) {
                task.Reject(engine, CreateJsError(engine, static_cast<int32_t>(ret), "JsWindow::OnDestroy failed."));
                return;
            }
            std::string windowName = windowToken_->GetWindowName();
            std::lock_guard<std::recursive_mutex> lock(g_mutex);
            if (g_jsWindowMap.find(windowName) != g_jsWindowMap.end()) {
                g_jsWindowMap.erase(windowName);
                WLOGFI("JsWindow::OnDestroy windowName %{public}s is destroyed", windowName.c_str());
            }
            // FIX ME: windowToken = nullptr in aync task and don't affect other async task
            task.Resolve(engine, engine.CreateUndefined());
            WLOGFI("JsWindow::OnDestroy success");
        };

    NativeValue* lastParam = (info.argc == 0) ? nullptr : info.argv[0];
    NativeValue* result = nullptr;
    AsyncTask::Schedule(
        engine, CreateAsyncTaskWithLastParam(engine, lastParam, nullptr, std::move(complete), &result));
    return result;
}

NativeValue* JsWindow::OnHide(NativeEngine& engine, NativeCallbackInfo& info)
{
    WLOGFI("JsWindow::OnHide is called");
    WMError errCode = WMError::WM_OK;
    if (windowToken_ == nullptr) {
        WLOGFE("JsWindow windowToken_ is nullptr");
        errCode = WMError::WM_ERROR_INVALID_PARAM;
    }
    AsyncTask::CompleteCallback complete =
        [=](NativeEngine& engine, AsyncTask& task, int32_t status) {
            if (errCode != WMError::WM_OK) {
                task.Reject(engine, CreateJsError(engine, static_cast<int32_t>(errCode), "Invalidate params."));
                return;
            }
            WMError ret = windowToken_->Hide();
            if (ret == WMError::WM_OK) {
                task.Resolve(engine, engine.CreateUndefined());
                WLOGFI("JsWindow::OnHide success");
            } else {
                task.Reject(engine, CreateJsError(engine, static_cast<int32_t>(ret), "JsWindow::OnHide failed."));
            }
        };

    NativeValue* lastParam = (info.argc == 0) ? nullptr : info.argv[0];
    NativeValue* result = nullptr;
    AsyncTask::Schedule(
        engine, CreateAsyncTaskWithLastParam(engine, lastParam, nullptr, std::move(complete), &result));
    return result;
}

NativeValue* JsWindow::OnMoveTo(NativeEngine& engine, NativeCallbackInfo& info)
{
    WLOGFI("JsWindow::OnMoveTo is called");
    WMError errCode = WMError::WM_OK;
    if (windowToken_ == nullptr) {
        WLOGFE("JsWindow windowToken_ is nullptr");
        errCode = WMError::WM_ERROR_INVALID_PARAM;
    }
    int32_t x;
    if (!ConvertFromJsValue(engine, info.argv[0], x)) {
        WLOGFE("Failed to convert parameter to x");
        errCode = WMError::WM_ERROR_INVALID_PARAM;
    }

    int32_t y;
    if (!ConvertFromJsValue(engine, info.argv[ARGC_ONE], y)) {
        WLOGFE("Failed to convert parameter to y");
        errCode = WMError::WM_ERROR_INVALID_PARAM;
    }
    AsyncTask::CompleteCallback complete =
        [=](NativeEngine& engine, AsyncTask& task, int32_t status) {
            if (errCode != WMError::WM_OK) {
                task.Reject(engine, CreateJsError(engine, static_cast<int32_t>(errCode), "Invalidate params."));
                return;
            }
            WMError ret = windowToken_->MoveTo(x, y);
            if (ret == WMError::WM_OK) {
                task.Resolve(engine, engine.CreateUndefined());
                WLOGFI("JsWindow::OnMoveTo success");
            } else {
                task.Reject(engine, CreateJsError(engine, static_cast<int32_t>(ret), "JsWindow::OnMoveTo failed."));
            }
        };

    NativeValue* lastParam = (info.argc == ARGC_TWO) ? nullptr : info.argv[INDEX_TWO];
    NativeValue* result = nullptr;
    AsyncTask::Schedule(
        engine, CreateAsyncTaskWithLastParam(engine, lastParam, nullptr, std::move(complete), &result));
    return result;
}

NativeValue* JsWindow::OnResize(NativeEngine& engine, NativeCallbackInfo& info)
{
    WLOGFI("JsWindow::OnResize is called");
    WMError errCode = WMError::WM_OK;
    if (windowToken_ == nullptr) {
        WLOGFE("JsWindow windowToken_ is nullptr");
        errCode = WMError::WM_ERROR_INVALID_PARAM;
    }
    uint32_t width;
    if (!ConvertFromJsValue(engine, info.argv[0], width)) {
        WLOGFE("Failed to convert parameter to width");
        errCode = WMError::WM_ERROR_INVALID_PARAM;
    }

    uint32_t height;
    if (!ConvertFromJsValue(engine, info.argv[ARGC_ONE], height)) {
        WLOGFE("Failed to convert parameter to height");
        errCode = WMError::WM_ERROR_INVALID_PARAM;
    }
    AsyncTask::CompleteCallback complete =
        [=](NativeEngine& engine, AsyncTask& task, int32_t status) {
            if (errCode != WMError::WM_OK) {
                task.Reject(engine, CreateJsError(engine, static_cast<int32_t>(errCode), "Invalidate params."));
                return;
            }
            WMError ret = windowToken_->Resize(width, height);
            if (ret == WMError::WM_OK) {
                task.Resolve(engine, engine.CreateUndefined());
            } else {
                task.Reject(engine, CreateJsError(engine, static_cast<int32_t>(ret), "JsWindow::OnResize failed."));
            }
        };
    NativeValue* lastParam = (info.argc == ARGC_TWO) ? nullptr : info.argv[INDEX_TWO];
    NativeValue* result = nullptr;
    AsyncTask::Schedule(
        engine, CreateAsyncTaskWithLastParam(engine, lastParam, nullptr, std::move(complete), &result));
    return result;
}

NativeValue* JsWindow::OnSetWindowType(NativeEngine& engine, NativeCallbackInfo& info)
{
    WLOGFI("JsWindow::OnSetWindowType is called");
    WMError errCode = WMError::WM_OK;
    if (windowToken_ == nullptr) {
        WLOGFE("JsWindow windowToken_ is nullptr");
        errCode = WMError::WM_ERROR_INVALID_PARAM;
    }
    NativeNumber* nativeType = ConvertNativeValueTo<NativeNumber>(info.argv[0]);
    if (nativeType == nullptr) {
        WLOGFE("Failed to convert parameter to windowType");
        errCode = WMError::WM_ERROR_INVALID_PARAM;
    }
    // adapt to the old version
    WindowType winType = WindowType::SYSTEM_WINDOW_BASE;
    if (static_cast<uint32_t>(*nativeType) >= static_cast<uint32_t>(WindowType::SYSTEM_WINDOW_BASE)) {
        winType = static_cast<WindowType>(static_cast<uint32_t>(*nativeType));
    } else {
        if (JS_TO_NATIVE_WINDOW_TYPE_MAP.count(static_cast<ApiWindowType>(static_cast<uint32_t>(*nativeType))) != 0) {
            winType = JS_TO_NATIVE_WINDOW_TYPE_MAP.at(static_cast<ApiWindowType>(static_cast<uint32_t>(*nativeType)));
        } else {
            WLOGFE("Do not surppot this type");
            errCode = WMError::WM_ERROR_INVALID_PARAM;
        }
    }
    AsyncTask::CompleteCallback complete =
        [=](NativeEngine& engine, AsyncTask& task, int32_t status) {
            if (errCode != WMError::WM_OK) {
                task.Reject(engine, CreateJsError(engine, static_cast<int32_t>(errCode), "Invalidate params."));
                return;
            }
            WMError ret = windowToken_->SetWindowType(winType);
            if (ret == WMError::WM_OK) {
                task.Resolve(engine, engine.CreateUndefined());
                WLOGFI("JsWindow::OnSetWindowType success");
            } else {
                task.Reject(engine, CreateJsError(engine,
                    static_cast<int32_t>(ret), "JsWindow::OnSetWindowType failed."));
            }
        };

    NativeValue* lastParam = (info.argc == ARGC_ONE) ? nullptr : info.argv[INDEX_ONE];
    NativeValue* result = nullptr;
    AsyncTask::Schedule(
        engine, CreateAsyncTaskWithLastParam(engine, lastParam, nullptr, std::move(complete), &result));
    return result;
}

NativeValue* JsWindow::OnSetWindowMode(NativeEngine& engine, NativeCallbackInfo& info)
{
    WLOGFI("JsWindow::OnSetWindowMode is called");
    WMError errCode = WMError::WM_OK;
    if (windowToken_ == nullptr) {
        WLOGFE("JsWindow windowToken_ is nullptr");
        errCode = WMError::WM_ERROR_INVALID_PARAM;
    }
    NativeNumber* nativeMode = ConvertNativeValueTo<NativeNumber>(info.argv[0]);
    if (nativeMode == nullptr) {
        WLOGFE("Failed to convert parameter to windowMode");
        errCode = WMError::WM_ERROR_INVALID_PARAM;
    }
    WindowMode winMode = static_cast<WindowMode>(static_cast<uint32_t>(*nativeMode));
    AsyncTask::CompleteCallback complete =
        [=](NativeEngine& engine, AsyncTask& task, int32_t status) {
            if (errCode != WMError::WM_OK) {
                task.Reject(engine, CreateJsError(engine, static_cast<int32_t>(errCode), "Invalidate params."));
                return;
            }
            WMError ret = windowToken_->SetWindowMode(winMode);
            if (ret == WMError::WM_OK) {
                task.Resolve(engine, engine.CreateUndefined());
                WLOGFI("JsWindow::OnSetWindowMode success");
            } else {
                task.Reject(engine,
                    CreateJsError(engine, static_cast<int32_t>(ret), "JsWindow::OnSetWindowMode failed."));
            }
        };

    NativeValue* lastParam = (info.argc == ARGC_ONE) ? nullptr : info.argv[INDEX_ONE];
    NativeValue* result = nullptr;
    AsyncTask::Schedule(
        engine, CreateAsyncTaskWithLastParam(engine, lastParam, nullptr, std::move(complete), &result));
    return result;
}

NativeValue* JsWindow::OnGetProperties(NativeEngine& engine, NativeCallbackInfo& info)
{
    WLOGFI("JsWindow::OnGetProperties is called");
    WMError errCode = WMError::WM_OK;
    if (windowToken_ == nullptr) {
        WLOGFE("JsWindow windowToken_ is nullptr");
        errCode = WMError::WM_ERROR_INVALID_PARAM;
    }
    AsyncTask::CompleteCallback complete =
        [=](NativeEngine& engine, AsyncTask& task, int32_t status) {
            if (errCode != WMError::WM_OK) {
                task.Reject(engine, CreateJsError(engine, static_cast<int32_t>(errCode), "Invalidate params."));
                return;
            }
            auto objValue = CreateJsWindowPropertiesObject(engine, windowToken_);
            WLOGFI("JsWindow::OnGetProperties objValue %{public}p", objValue);
            if (objValue != nullptr) {
                task.Resolve(engine, objValue);
                WLOGFI("JsWindow::OnGetProperties success");
            } else {
                task.Reject(engine, CreateJsError(engine,
                    static_cast<int32_t>(WMError::WM_ERROR_NULLPTR), "JsWindow::OnGetProperties failed."));
            }
        };

    NativeValue* lastParam = (info.argc == 0) ? nullptr : info.argv[0];
    NativeValue* result = nullptr;
    AsyncTask::Schedule(
        engine, CreateAsyncTaskWithLastParam(engine, lastParam, nullptr, std::move(complete), &result));
    return result;
}

bool JsWindow::IfCallbackRegistered(std::string type, NativeValue* jsListenerObject)
{
    if (jsCallbackMap_.empty() || jsCallbackMap_.find(type) == jsCallbackMap_.end()) {
        WLOGFI("JsWindow::IfCallbackRegistered methodName %{public}s not registertd!", type.c_str());
        return false;
    }

    for (auto iter = jsCallbackMap_[type].begin(); iter != jsCallbackMap_[type].end(); iter++) {
        if (jsListenerObject->StrictEquals((*iter)->Get())) {
            WLOGFE("JsWindow::IfCallbackRegistered callback already registered!");
            return true;
        }
    }
    return false;
}

void JsWindow::RegisterWindowListenerWithType(NativeEngine& engine, std::string type, NativeValue* value)
{
    if (IfCallbackRegistered(type, value)) {
        WLOGFE("JsWindow::RegisterWindowListenerWithType callback already registered!");
        return;
    }
    std::unique_ptr<NativeReference> callbackRef;
    callbackRef.reset(engine.CreateReference(value, 1));
    if (jsListenerMap_.find(type) == jsListenerMap_.end()) {
        sptr<JsWindowListener> windowListener = new JsWindowListener(&engine);
        if (type.compare(WINDOW_SIZE_CHANGE_CB) == 0) {
            sptr<IWindowChangeListener> thisListener(windowListener);
            windowToken_->RegisterWindowChangeListener(thisListener);
            WLOGFI("JsWindow::RegisterWindowListenerWithType windowSizeChange success");
        } else if (type.compare(SYSTEM_AVOID_AREA_CHANGE_CB) == 0) {
            sptr<IAvoidAreaChangedListener> thisListener(windowListener);
            windowToken_->RegisterAvoidAreaChangeListener(thisListener);
            WLOGFI("JsWindow::RegisterWindowListenerWithType systemAvoidAreaChange success");
        } else {
            WLOGFE("JsWindow::RegisterWindowListenerWithType failed method: %{public}s not support!",
                type.c_str());
            return;
        }
        windowListener->AddCallback(value);
        jsListenerMap_[type] = windowListener;
    } else {
        jsListenerMap_[type]->AddCallback(value);
    }
    jsCallbackMap_[type].push_back(std::move(callbackRef));
    return;
}

void JsWindow::UnregisterAllWindowListenerWithType(std::string type)
{
    if (jsListenerMap_.empty() || jsListenerMap_.find(type) == jsListenerMap_.end()) {
        WLOGFI("JsWindow::UnregisterAllWindowListenerWithType methodName %{public}s not registerted!",
            type.c_str());
        return;
    }
    jsListenerMap_[type]->RemoveAllCallback();
    if (type.compare(WINDOW_SIZE_CHANGE_CB) == 0) {
        sptr<IWindowChangeListener> thisListener(nullptr);
        windowToken_->RegisterWindowChangeListener(thisListener);
        WLOGFI("JsWindow::UnregisterAllWindowListenerWithType windowSizeChange success");
    }
    if (type.compare(SYSTEM_AVOID_AREA_CHANGE_CB) == 0) {
        windowToken_->UnregisterAvoidAreaChangeListener();
        WLOGFI("JsWindow::UnregisterAllWindowListenerWithType systemAvoidAreaChange success");
    }
    jsListenerMap_.erase(type);
    jsCallbackMap_.erase(type);
    return;
}

void JsWindow::UnregisterWindowListenerWithType(std::string type, NativeValue* value)
{
    if (jsListenerMap_.empty() || jsListenerMap_.find(type) == jsListenerMap_.end()) {
        WLOGFI("JsWindow::UnregisterWindowListenerWithType methodName %{public}s not registerted!",
            type.c_str());
        return;
    }
    for (auto it = jsCallbackMap_[type].begin(); it != jsCallbackMap_[type].end();) {
        if (value->StrictEquals((*it)->Get())) {
            jsListenerMap_[type]->RemoveCallback(value);
            jsCallbackMap_[type].erase(it++);
            break;
        } else {
            it++;
        }
    }
    // one type with multi jscallback, erase type when there is no callback in one type
    if (jsCallbackMap_[type].empty()) {
        if (type.compare(WINDOW_SIZE_CHANGE_CB) == 0) {
            sptr<IWindowChangeListener> thisListener(nullptr);
            windowToken_->RegisterWindowChangeListener(thisListener);
            WLOGFI("JsWindow::UnregisterWindowListenerWithType windowSizeChange success");
        }
        if (type.compare(SYSTEM_AVOID_AREA_CHANGE_CB) == 0) {
            windowToken_->UnregisterAvoidAreaChangeListener();
            WLOGFI("JsWindow::UnregisterWindowListenerWithType systemAvoidAreaChange success");
        }
        jsCallbackMap_.erase(type);
        jsListenerMap_.erase(type);
    }
    return;
}

NativeValue* JsWindow::OnRegisterWindowCallback(NativeEngine& engine, NativeCallbackInfo& info)

{
    WLOGFI("JsWindow::OnRegisterWindowCallback is called");
    if (windowToken_ == nullptr) {
        WLOGFE("JsWindow windowToken_ is nullptr");
        return engine.CreateUndefined();
    }
    if (info.argc != ARGC_TWO) {
        WLOGFE("Params not match");
        return engine.CreateUndefined();
    }
    std::string cbType;
    if (!ConvertFromJsValue(engine, info.argv[0], cbType)) {
        WLOGFE("Failed to convert parameter to callbackType");
        return engine.CreateUndefined();
    }
    NativeValue* value = info.argv[1];
    if (!value->IsCallable()) {
        WLOGFI("JsWindow::OnRegisterWindowCallback info->argv[1] is not callable");
        return engine.CreateUndefined();
    }
    std::lock_guard<std::mutex> lock(mtx_);
    RegisterWindowListenerWithType(engine, cbType, value);
    return engine.CreateUndefined();
}

NativeValue* JsWindow::OnUnregisterWindowCallback(NativeEngine& engine, NativeCallbackInfo& info)
{
    WLOGFI("JsWindow::OnUnregisterWindowCallback is called");
    if (windowToken_ == nullptr) {
        WLOGFE("JsWindow windowToken_ is nullptr");
        return engine.CreateUndefined();
    }
    if (info.argc == 0) {
        WLOGFE("Params not match");
        return engine.CreateUndefined();
    }
    std::string cbType;
    if (!ConvertFromJsValue(engine, info.argv[0], cbType)) {
        WLOGFE("Failed to convert parameter to callbackType");
        return engine.CreateUndefined();
    }
    std::lock_guard<std::mutex> lock(mtx_);
    if (info.argc == 1) {
        UnregisterAllWindowListenerWithType(cbType);
    } else {
        NativeValue* value = info.argv[ARGC_ONE];
        if (!value->IsCallable()) {
            WLOGFI("JsWindow::OnUnregisterWindowManagerCallback info->argv[1] is not callable");
            return engine.CreateUndefined();
        }
        UnregisterWindowListenerWithType(cbType, value);
    }

    return engine.CreateUndefined();
}

NativeValue* JsWindow::OnLoadContent(NativeEngine& engine, NativeCallbackInfo& info)
{
    WLOGFI("JsWindow::OnLoadContent is called");
    WMError errCode = WMError::WM_OK;
    if (windowToken_ == nullptr || info.argc <= 0) {
        WLOGFE("JsWindow windowToken_ is nullptr");
        errCode = WMError::WM_ERROR_INVALID_PARAM;
    }
    std::string contextUrl;
    if (!ConvertFromJsValue(engine, info.argv[0], contextUrl)) {
        WLOGFE("Failed to convert parameter to context url");
        errCode = WMError::WM_ERROR_INVALID_PARAM;
    }
    NativeValue* storage = nullptr;
    NativeValue* callBack = nullptr;
    if (info.argc == ARGC_TWO) {
        NativeValue* value = info.argv[INDEX_ONE];
        if (value->TypeOf() == NATIVE_FUNCTION) {
            callBack = info.argv[INDEX_ONE];
        } else {
            storage = info.argv[INDEX_ONE];
        }
    } else if (info.argc == ARGC_THREE) {
        storage = info.argv[INDEX_ONE];
        callBack = info.argv[INDEX_TWO];
    }
    contentStorage_ = static_cast<void*>(storage);
    AsyncTask::CompleteCallback complete =
        [=](NativeEngine& engine, AsyncTask& task, int32_t status) {
            if (errCode != WMError::WM_OK) {
                task.Reject(engine, CreateJsError(engine, static_cast<int32_t>(errCode), "Invalidate params."));
                return;
            }
            WMError ret = windowToken_->SetUIContent(contextUrl, &engine,
                static_cast<NativeValue*>(contentStorage_), false);
            if (ret == WMError::WM_OK) {
                task.Resolve(engine, engine.CreateUndefined());
                WLOGFI("JsWindow::OnLoadContent success");
            } else {
                task.Reject(engine,
                    CreateJsError(engine, static_cast<int32_t>(ret), "JsWindow::OnLoadContent failed."));
            }
        };
    NativeValue* result = nullptr;
    AsyncTask::Schedule(
        engine, CreateAsyncTaskWithLastParam(engine, callBack, nullptr, std::move(complete), &result));
    return result;
}

NativeValue* JsWindow::OnSetFullScreen(NativeEngine& engine, NativeCallbackInfo& info)
{
    WLOGFI("JsWindow::OnSetFullScreen is called");
    WMError errCode = WMError::WM_OK;
    if (windowToken_ == nullptr || info.argc < ARGC_ONE) {
        WLOGFE("JsWindow windowToken_ is nullptr or param is too small!");
        errCode = WMError::WM_ERROR_INVALID_PARAM;
    }
    NativeBoolean* nativeVal = ConvertNativeValueTo<NativeBoolean>(info.argv[0]);
    if (nativeVal == nullptr) {
        WLOGFE("Failed to convert parameter to isFullScreen");
        errCode = WMError::WM_ERROR_INVALID_PARAM;
    }
    bool isFullScreen = static_cast<bool>(*nativeVal);
    AsyncTask::CompleteCallback complete =
        [=](NativeEngine& engine, AsyncTask& task, int32_t status) {
            if (errCode != WMError::WM_OK) {
                task.Reject(engine, CreateJsError(engine, static_cast<int32_t>(errCode), "Invalidate params."));
                return;
            }
            WMError ret = windowToken_->SetFullScreen(isFullScreen);
            if (ret == WMError::WM_OK) {
                task.Resolve(engine, engine.CreateUndefined());
                WLOGFI("JsWindow::OnSetFullScreen success");
            } else {
                task.Reject(engine, CreateJsError(engine,
                    static_cast<int32_t>(ret), "JsWindow::OnSetFullScreen failed."));
            }
        };

    NativeValue* lastParam = (info.argc == ARGC_ONE) ? nullptr : info.argv[INDEX_ONE];
    NativeValue* result = nullptr;
    AsyncTask::Schedule(
        engine, CreateAsyncTaskWithLastParam(engine, lastParam, nullptr, std::move(complete), &result));
    return result;
}

NativeValue* JsWindow::OnSetLayoutFullScreen(NativeEngine& engine, NativeCallbackInfo& info)
{
    WLOGFI("JsWindow::OnSetLayoutFullScreen is called");
    WMError errCode = WMError::WM_OK;
    if (windowToken_ == nullptr || info.argc < ARGC_ONE) {
        WLOGFE("JsWindow windowToken_ is nullptr or param is too small!");
        errCode = WMError::WM_ERROR_INVALID_PARAM;
    }
    NativeBoolean* nativeVal = ConvertNativeValueTo<NativeBoolean>(info.argv[0]);
    if (nativeVal == nullptr) {
        WLOGFE("Failed to convert parameter to isLayoutFullScreen");
        errCode = WMError::WM_ERROR_INVALID_PARAM;
    }
    bool isLayoutFullScreen = static_cast<bool>(*nativeVal);
    AsyncTask::CompleteCallback complete =
        [=](NativeEngine& engine, AsyncTask& task, int32_t status) {
            if (errCode != WMError::WM_OK) {
                task.Reject(engine, CreateJsError(engine, static_cast<int32_t>(errCode), "Invalidate params."));
                return;
            }
            WMError ret = windowToken_->SetLayoutFullScreen(isLayoutFullScreen);
            if (ret == WMError::WM_OK) {
                task.Resolve(engine, engine.CreateUndefined());
                WLOGFI("JsWindow::OnSetLayoutFullScreen success");
            } else {
                task.Reject(engine, CreateJsError(engine,
                    static_cast<int32_t>(ret), "JsWindow::OnSetLayoutFullScreen failed."));
            }
        };
    NativeValue* lastParam = (info.argc == ARGC_ONE) ? nullptr : info.argv[INDEX_ONE];
    NativeValue* result = nullptr;
    AsyncTask::Schedule(
        engine, CreateAsyncTaskWithLastParam(engine, lastParam, nullptr, std::move(complete), &result));
    return result;
}

NativeValue* JsWindow::OnSetSystemBarEnable(NativeEngine& engine, NativeCallbackInfo& info)
{
    WLOGFI("JsWindow::OnSetSystemBarEnable is called");
    WMError errCode = WMError::WM_OK;
    if (windowToken_ == nullptr || info.argc < ARGC_ONE) {
        WLOGFE("JsWindow windowToken_ is nullptr or param is too small!");
        errCode = WMError::WM_ERROR_INVALID_PARAM;
    }
    std::map<WindowType, SystemBarProperty> systemBarProperties;
    if (!GetSystemBarStatus(systemBarProperties, engine, info, windowToken_)) {
        errCode = WMError::WM_ERROR_INVALID_PARAM;
    }
    AsyncTask::CompleteCallback complete =
        [=](NativeEngine& engine, AsyncTask& task, int32_t status) {
            if (errCode != WMError::WM_OK) {
                task.Reject(engine, CreateJsError(engine, static_cast<int32_t>(errCode), "Invalidate params."));
                return;
            }
            WMError ret = windowToken_->SetSystemBarProperty(WindowType::WINDOW_TYPE_STATUS_BAR,
                systemBarProperties.at(WindowType::WINDOW_TYPE_STATUS_BAR));
            ret = windowToken_->SetSystemBarProperty(WindowType::WINDOW_TYPE_NAVIGATION_BAR,
                systemBarProperties.at(WindowType::WINDOW_TYPE_NAVIGATION_BAR));
            if (ret == WMError::WM_OK) {
                task.Resolve(engine, engine.CreateUndefined());
                WLOGFI("JsWindow::OnSetSystemBarEnable success");
            } else {
                task.Reject(engine, CreateJsError(engine,
                    static_cast<int32_t>(ret), "JsWindow::OnSetSystemBarEnable failed."));
            }
        };

    NativeValue* lastParam = (info.argc == ARGC_TWO) ?  info.argv[INDEX_ONE] : nullptr;
    NativeValue* result = nullptr;
    AsyncTask::Schedule(
        engine, CreateAsyncTaskWithLastParam(engine, lastParam, nullptr, std::move(complete), &result));
    return result;
}

NativeValue* JsWindow::OnSetSystemBarProperties(NativeEngine& engine, NativeCallbackInfo& info)
{
    WLOGFI("JsWindow::OnSetSystemBarProperties is called");
    WMError errCode = WMError::WM_OK;
    if (windowToken_ == nullptr || info.argc < ARGC_ONE) {
        WLOGFE("JsWindow windowToken_ is nullptr or param is too small!");
        errCode = WMError::WM_ERROR_INVALID_PARAM;
    }
    NativeObject* nativeObj = ConvertNativeValueTo<NativeObject>(info.argv[0]);
    if (nativeObj == nullptr) {
        WLOGFE("Failed to convert object to SystemBarProperties");
        errCode = WMError::WM_ERROR_INVALID_PARAM;
    }
    std::map<WindowType, SystemBarProperty> systemBarProperties;
    if (!SetSystemBarPropertiesFromJs(engine, nativeObj, systemBarProperties, windowToken_)) {
        WLOGFE("Failed to GetSystemBarProperties From Js Object");
        errCode = WMError::WM_ERROR_INVALID_PARAM;
    }
    AsyncTask::CompleteCallback complete =
        [=](NativeEngine& engine, AsyncTask& task, int32_t status) {
            if (errCode != WMError::WM_OK) {
                task.Reject(engine, CreateJsError(engine, static_cast<int32_t>(errCode), "Invalidate params."));
                return;
            }
            WMError ret = windowToken_->SetSystemBarProperty(WindowType::WINDOW_TYPE_STATUS_BAR,
                systemBarProperties.at(WindowType::WINDOW_TYPE_STATUS_BAR));
            ret = windowToken_->SetSystemBarProperty(WindowType::WINDOW_TYPE_NAVIGATION_BAR,
                systemBarProperties.at(WindowType::WINDOW_TYPE_NAVIGATION_BAR));
            if (ret == WMError::WM_OK) {
                task.Resolve(engine, engine.CreateUndefined());
                WLOGFI("JsWindow::OnSetSystemBarProperties success");
            } else {
                task.Reject(engine, CreateJsError(engine,
                    static_cast<int32_t>(WMError::WM_ERROR_NULLPTR), "JsWindow::OnSetSystemBarProperties failed."));
            }
        };

    NativeValue* lastParam = (info.argc == ARGC_ONE) ? nullptr : info.argv[INDEX_ONE];
    NativeValue* result = nullptr;
    AsyncTask::Schedule(
        engine, CreateAsyncTaskWithLastParam(engine, lastParam, nullptr, std::move(complete), &result));
    return result;
}

NativeValue* JsWindow::OnGetAvoidArea(NativeEngine& engine, NativeCallbackInfo& info)
{
    WLOGFI("JsWindow::OnGetAvoidArea is called");
    WMError errCode = WMError::WM_OK;
    if (windowToken_ == nullptr || info.argc < ARGC_ONE) {
        WLOGFE("JsWindow windowToken_ is nullptr or param is too small!");
        errCode = WMError::WM_ERROR_INVALID_PARAM;
    }
    // Parse info->argv[0] as AvoidAreaType
    NativeNumber* nativeMode = ConvertNativeValueTo<NativeNumber>(info.argv[0]);
    if (nativeMode == nullptr) {
        WLOGFE("Failed to convert parameter to AvoidAreaType");
        errCode = WMError::WM_ERROR_INVALID_PARAM;
    }
    AvoidAreaType avoidAreaType = static_cast<AvoidAreaType>(static_cast<uint32_t>(*nativeMode));
    WLOGFI("JsWindow::OnGetAvoidArea get avoidAreaType success %{public}u", avoidAreaType);

    AsyncTask::CompleteCallback complete =
        [=](NativeEngine& engine, AsyncTask& task, int32_t status) {
            if (errCode != WMError::WM_OK) {
                task.Reject(engine, CreateJsError(engine, static_cast<int32_t>(errCode), "Invalidate params."));
                return;
            }
            // getAvoidRect by avoidAreaType
            AvoidArea avoidArea;
            WMError ret = windowToken_->GetAvoidAreaByType(avoidAreaType, avoidArea);
            if (ret == WMError::WM_OK) {
                WLOGFI("JsWindow::OnGetAvoidArea GetAvoidAreaByType Success");
            } else {
                WLOGFE("JsWindow::OnGetAvoidArea GetAvoidAreaByType Failed");
                avoidArea = { EMPTY_RECT, EMPTY_RECT, EMPTY_RECT, EMPTY_RECT }; // left, top, right, bottom
            }

            // native avoidArea -> js avoidArea
            NativeValue* avoidAreaObj = ChangeAvoidAreaToJsValue(engine, avoidArea);
            if (avoidAreaObj != nullptr) {
                WLOGFI("JsWindow::OnGetAvoidArea ChangeAvoidAreaToJsValue Success");
                task.Resolve(engine, avoidAreaObj);
            } else {
                task.Reject(engine, CreateJsError(engine,
                    static_cast<int32_t>(WMError::WM_ERROR_NULLPTR), "JsWindow::OnGetAvoidArea failed."));
            }
        };

    WLOGFI("JsWindow::OnGetAvoidArea AsyncTask end");
    NativeValue* lastParam = (info.argc == ARGC_ONE) ? nullptr : info.argv[INDEX_ONE];
    NativeValue* result = nullptr;
    AsyncTask::Schedule(
        engine, CreateAsyncTaskWithLastParam(engine, lastParam, nullptr, std::move(complete), &result));
    return result;
}

NativeValue* JsWindow::OnIsShowing(NativeEngine& engine, NativeCallbackInfo& info)
{
    WLOGFI("JsWindow::OnIsShowing is called");
    AsyncTask::CompleteCallback complete =
        [=](NativeEngine& engine, AsyncTask& task, int32_t status) {
            if (windowToken_ == nullptr) {
                task.Reject(engine, CreateJsError(engine, static_cast<int32_t>(WMError::WM_ERROR_NULLPTR),
                    "JsWindow::OnIsShowing failed."));
                WLOGFE("JsWindow windowToken_ is nullptr");
                return;
            }
            bool state = windowToken_->GetShowState();
            task.Resolve(engine, CreateJsValue(engine, state));
        };

    NativeValue* lastParam = (info.argc == 0) ? nullptr : info.argv[0];
    NativeValue* result = nullptr;
    AsyncTask::Schedule(
        engine, CreateAsyncTaskWithLastParam(engine, lastParam, nullptr, std::move(complete), &result));
    return result;
}

NativeValue* JsWindow::OnIsSupportWideGamut(NativeEngine& engine, NativeCallbackInfo& info)
{
    WLOGFI("JsWindow::OnIsSupportWideGamut is called");
    AsyncTask::CompleteCallback complete =
        [=](NativeEngine& engine, AsyncTask& task, int32_t status) {
            if (windowToken_ == nullptr) {
                task.Reject(engine, CreateJsError(engine, static_cast<int32_t>(WMError::WM_ERROR_NULLPTR),
                    "JsWindow::OnIsSupportWideGamut failed."));
                WLOGFE("JsWindow windowToken_ is nullptr");
                return;
            }
            bool flag = windowToken_->IsSupportWideGamut();
            task.Resolve(engine, CreateJsValue(engine, flag));
        };

    NativeValue* lastParam = (info.argc == 0) ? nullptr :
        (info.argv[0]->TypeOf() == NATIVE_FUNCTION ? info.argv[0] : nullptr);
    NativeValue* result = nullptr;
    AsyncTask::Schedule(
        engine, CreateAsyncTaskWithLastParam(engine, lastParam, nullptr, std::move(complete), &result));
    return result;
}

NativeValue* JsWindow::OnSetColorSpace(NativeEngine& engine, NativeCallbackInfo& info)
{
    WLOGFI("JsWindow::OnSetColorSpace is called");
    WMError errCode = WMError::WM_OK;
    ColorSpace colorSpace = ColorSpace::COLOR_SPACE_DEFAULT;
    if (windowToken_ == nullptr) {
        WLOGFE("JsWindow windowToken_ is nullptr");
        errCode = WMError::WM_ERROR_INVALID_PARAM;
    }

    if (info.argc < 1) {
        errCode = WMError::WM_ERROR_INVALID_PARAM;
        WLOGFE("JsWindow::OnSetColorSpace argc < 1");
    } else {
        NativeNumber* nativeType = ConvertNativeValueTo<NativeNumber>(info.argv[0]);
        if (nativeType == nullptr) {
            errCode = WMError::WM_ERROR_INVALID_PARAM;
            WLOGFE("JsWindow::OnSetColorSpace Failed to convert parameter to ColorSpace");
        } else {
            colorSpace = static_cast<ColorSpace>(static_cast<uint32_t>(*nativeType));
            WLOGFI("JsWindow::OnSetColorSpace %{public}u", static_cast<uint32_t>(colorSpace));
        }
    }

    AsyncTask::CompleteCallback complete =
        [=](NativeEngine& engine, AsyncTask& task, int32_t status) {
            if (windowToken_ == nullptr || errCode != WMError::WM_OK) {
                task.Reject(engine, CreateJsError(engine, static_cast<int32_t>(WMError::WM_ERROR_NULLPTR),
                    "JsWindow::OnSetColorSpace failed."));
                WLOGFE("JsWindow windowToken_ is nullptr or args error");
                return;
            }
            windowToken_->SetColorSpace(colorSpace);
            task.Resolve(engine, CreateJsValue(engine, engine.CreateUndefined()));
        };

    NativeValue* lastParam = (info.argc <= ARGC_ONE) ? nullptr :
        (info.argv[INDEX_ONE]->TypeOf() == NATIVE_FUNCTION ? info.argv[INDEX_ONE] : nullptr);
    NativeValue* result = nullptr;
    AsyncTask::Schedule(
        engine, CreateAsyncTaskWithLastParam(engine, lastParam, nullptr, std::move(complete), &result));
    return result;
}

NativeValue* JsWindow::OnGetColorSpace(NativeEngine& engine, NativeCallbackInfo& info)
{
    WLOGFI("JsWindow::OnGetColorSpace is called");
    AsyncTask::CompleteCallback complete =
        [=](NativeEngine& engine, AsyncTask& task, int32_t status) {
            if (windowToken_ == nullptr) {
                task.Reject(engine, CreateJsError(engine, static_cast<int32_t>(WMError::WM_ERROR_NULLPTR),
                    "JsWindow::OnGetColorSpace failed."));
                WLOGFE("JsWindow windowToken_ is nullptr");
                return;
            }
            ColorSpace colorSpace = windowToken_->GetColorSpace();
            task.Resolve(engine, CreateJsValue(engine, static_cast<uint32_t>(colorSpace)));
        };

    NativeValue* lastParam = (info.argc == 0) ? nullptr :
        (info.argv[0]->TypeOf() == NATIVE_FUNCTION ? info.argv[0] : nullptr);
    NativeValue* result = nullptr;
    AsyncTask::Schedule(
        engine, CreateAsyncTaskWithLastParam(engine, lastParam, nullptr, std::move(complete), &result));
    return result;
}

std::shared_ptr<NativeReference> FindJsWindowObject(std::string windowName)
{
    WLOGFI("JsWindow::FindJsWindowObject is called");
    std::lock_guard<std::recursive_mutex> lock(g_mutex);
    if (g_jsWindowMap.find(windowName) == g_jsWindowMap.end()) {
        return nullptr;
    }
    return g_jsWindowMap[windowName];
}

NativeValue* CreateJsWindowObject(NativeEngine& engine, sptr<Window>& window)
{
    WLOGFI("JsWindow::CreateJsWindow is called");
    std::string windowName = window->GetWindowName();
    // avoid repeatly creat js window when getWindow
    std::shared_ptr<NativeReference> jsWindowObj = FindJsWindowObject(windowName);
    if (jsWindowObj != nullptr && jsWindowObj->Get() != nullptr) {
        WLOGFI("JsWindow::CreateJsWindow FindJsWindowObject %{public}s", windowName.c_str());
        return jsWindowObj->Get();
    }
    NativeValue* objValue = engine.CreateObject();
    NativeObject* object = ConvertNativeValueTo<NativeObject>(objValue);

    std::unique_ptr<JsWindow> jsWindow = std::make_unique<JsWindow>(window);
    object->SetNativePointer(jsWindow.release(), JsWindow::Finalizer, nullptr);

    BindNativeFunction(engine, *object, "show", JsWindow::Show);
    BindNativeFunction(engine, *object, "destroy", JsWindow::Destroy);
    BindNativeFunction(engine, *object, "hide", JsWindow::Hide);
    BindNativeFunction(engine, *object, "moveTo", JsWindow::MoveTo);
    BindNativeFunction(engine, *object, "resetSize", JsWindow::Resize);
    BindNativeFunction(engine, *object, "setWindowType", JsWindow::SetWindowType);
    BindNativeFunction(engine, *object, "setWindowMode", JsWindow::SetWindowMode);
    BindNativeFunction(engine, *object, "getProperties", JsWindow::GetProperties);
    BindNativeFunction(engine, *object, "on", JsWindow::RegisterWindowCallback);
    BindNativeFunction(engine, *object, "off", JsWindow::UnregisterWindowCallback);
    BindNativeFunction(engine, *object, "loadContent", JsWindow::LoadContent);
    BindNativeFunction(engine, *object, "setFullScreen", JsWindow::SetFullScreen);
    BindNativeFunction(engine, *object, "setLayoutFullScreen", JsWindow::SetLayoutFullScreen);
    BindNativeFunction(engine, *object, "setSystemBarEnable", JsWindow::SetSystemBarEnable);
    BindNativeFunction(engine, *object, "setSystemBarProperties", JsWindow::SetSystemBarProperties);
    BindNativeFunction(engine, *object, "getAvoidArea", JsWindow::GetAvoidArea);
    BindNativeFunction(engine, *object, "isShowing", JsWindow::IsShowing);
    BindNativeFunction(engine, *object, "isSupportWideGamut", JsWindow::IsSupportWideGamut);
    BindNativeFunction(engine, *object, "setColorSpace", JsWindow::SetColorSpace);
    BindNativeFunction(engine, *object, "getColorSpace", JsWindow::GetColorSpace);
    std::shared_ptr<NativeReference> jsWindowRef;
    jsWindowRef.reset(engine.CreateReference(objValue, 1));
    std::lock_guard<std::recursive_mutex> lock(g_mutex);
    g_jsWindowMap[windowName] = jsWindowRef;
    return objValue;
}
}  // namespace Rosen
}  // namespace OHOS
