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

#include "js_screen_session.h"

#include <hitrace_meter.h>
#include <js_runtime_utils.h>
#include <ui_content.h>

#include "interfaces/include/ws_common.h"
#include "js_screen_utils.h"
#include "window_manager_hilog.h"
#include "singleton_container.h"
#include "screen_manager.h"

namespace OHOS::Rosen {
using namespace AbilityRuntime;
namespace {
const std::string ON_CONNECTION_CALLBACK = "connect";
const std::string ON_DISCONNECTION_CALLBACK = "disconnect";
const std::string ON_PROPERTY_CHANGE_CALLBACK = "propertyChange";
const std::string ON_POWER_STATUS_CHANGE_CALLBACK = "powerStatusChange";
const std::string ON_SENSOR_ROTATION_CHANGE_CALLBACK = "sensorRotationChange";
const std::string ON_SCREEN_ORIENTATION_CHANGE_CALLBACK = "screenOrientationChange";
const std::string ON_SCREEN_ROTATION_LOCKED_CHANGE = "screenRotationLockedChange";
const std::string ON_SCREEN_DENSITY_CHANGE = "screenDensityChange";
const std::string ON_SCREEN_EXTEND_CHANGE = "screenExtendChange";
const std::string ON_HOVER_STATUS_CHANGE_CALLBACK = "hoverStatusChange";
const std::string ON_SCREEN_CAPTURE_NOTIFY = "screenCaptureNotify";
const std::string ON_SUPER_FOLD_STATUS_CHANGE_CALLBACK = "superFoldStatusChange";
const std::string ON_CAMERA_FOLD_STATUS_CHANGE_CALLBACK = "cameraStatusChange";
const std::string ON_SECONDARY_REFLEXION_CHANGE_CALLBACK = "secondaryReflexionChange";
const std::string ON_CAMERA_BACKSELFIE_CHANGE_CALLBACK = "cameraBackSelfieChange";
const std::string ON_EXTEND_SCREEN_CONNECT_STATUS_CHANGE_CALLBACK = "extendScreenConnectStatusChange";
const std::string ON_BEFORE_PROPERTY_CHANGE_CALLBACK = "beforeScreenPropertyChange";
const std::string ON_SCREEN_CHANGE_CALLBACK = "screenModeChange";
constexpr size_t ARGC_ONE = 1;
} // namespace

napi_value JsScreenSession::Create(napi_env env, const sptr<ScreenSession>& screenSession)
{
    TLOGD(WmsLogTag::DMS, "Create.");
    napi_value objValue = nullptr;
    napi_create_object(env, &objValue);
    if (objValue == nullptr) {
        TLOGE(WmsLogTag::DMS, "[NAPI]Object is null!");
        return NapiGetUndefined(env);
    }

    auto jsScreenSession = std::make_unique<JsScreenSession>(env, screenSession);
    napi_wrap(env, objValue, jsScreenSession.release(), JsScreenSession::Finalizer, nullptr, nullptr);
    napi_set_named_property(env, objValue, "screenId",
        CreateJsValue(env, static_cast<int64_t>(screenSession->GetScreenId())));
    napi_set_named_property(env, objValue, "name",
        CreateJsValue(env, static_cast<std::string>(screenSession->GetName())));
    napi_set_named_property(env, objValue, "isExtend", CreateJsValue(env, screenSession->GetIsExtend()));
    napi_set_named_property(env, objValue, "innerName",
        CreateJsValue(env, static_cast<std::string>(screenSession->GetInnerName())));
    napi_set_named_property(env, objValue, "displayGroupId",
        CreateJsValue(env, static_cast<int64_t>(screenSession->GetDisplayGroupId())));
    napi_set_named_property(env, objValue, "mainDisplayIdOfGroup",
        CreateJsValue(env, static_cast<int64_t>(screenSession->GetMainDisplayIdOfGroup())));
    napi_set_named_property(env, objValue, "isRealScreen",
        CreateJsValue(env, screenSession->GetIsRealScreen()));
    napi_set_named_property(env, objValue, "isUniqueRotationLocked",
        CreateJsValue(env, screenSession->GetUniqueRotationLock()));
    napi_set_named_property(env, objValue, "uniqueRotation",
        CreateJsValue(env, screenSession->GetUniqueRotation()));
    std::map<int32_t, int32_t> uniqueRotationOrientationMap = screenSession->GetUniqueRotationOrientationMap();
    napi_value jsMap = ConvertMapToJsMap(env, uniqueRotationOrientationMap);
    napi_set_named_property(env, objValue, "uniqueRotationOrientationMap", jsMap);

    const char* moduleName = "JsScreenSession";
    BindNativeFunction(env, objValue, "on", moduleName, JsScreenSession::RegisterCallback);
    BindNativeFunction(env, objValue, "setScreenRotationLocked", moduleName,
        JsScreenSession::SetScreenRotationLocked);
    BindNativeFunction(env, objValue, "setTouchEnabled", moduleName,
        JsScreenSession::SetTouchEnabled);
    BindNativeFunction(env, objValue, "loadContent", moduleName, JsScreenSession::LoadContent);
    BindNativeFunction(env, objValue, "getScreenUIContext", moduleName,
        JsScreenSession::GetScreenUIContext);
    BindNativeFunction(env, objValue, "destroyContent", moduleName,
        JsScreenSession::DestroyContent);
    BindNativeFunction(env, objValue, "releaseResource", moduleName,
        JsScreenSession::ReleaseResource);
    return objValue;
}

void JsScreenSession::Finalizer(napi_env env, void* data, void* hint)
{
    TLOGD(WmsLogTag::DMS, "Finalizer.");
    std::unique_ptr<JsScreenSession>(static_cast<JsScreenSession*>(data));
}

JsScreenSession::JsScreenSession(napi_env env, const sptr<ScreenSession>& screenSession)
    : env_(env), screenSession_(screenSession)
{
    std::string name = screenSession_ ? screenSession_->GetName() : "UNKNOWN";
    screenScene_ = new(std::nothrow) ScreenScene(name);
    if (screenSession_) {
        screenScene_->SetDisplayId(screenSession_->GetScreenId());
        bool isRealScreen = screenSession_->GetIsRealScreen();
        float densityInCurResolution = screenSession_->GetDensityInCurResolution();
        SetScreenSceneDpiFunc func = [this, isRealScreen, densityInCurResolution](float density) {
            TLOGNI(WmsLogTag::DMS, "Screen Scene Dpi change, new density = %{public}f", density);
            if (!screenScene_ || !screenSession_) {
                TLOGNE(WmsLogTag::DMS, "[NAPI]screenScene or screenSession is nullptr");
                return;
            }
            auto screenBounds = screenSession_->GetScreenProperty().GetBounds();
            Rect rect = { screenBounds.rect_.left_, screenBounds.rect_.top_,
                screenBounds.rect_.width_, screenBounds.rect_.height_ };
            screenScene_->SetDisplayDensity(density);
            if (!isRealScreen) {
                screenScene_->UpdateViewportConfig(rect, WindowSizeChangeReason::UPDATE_DPI_SYNC);
            } else {
                screenScene_->SetDisplayDensity(densityInCurResolution);
                screenScene_->UpdateViewportConfig(rect, WindowSizeChangeReason::UPDATE_DPI_SYNC);
            }
            OnScreenDensityChange();
        };
        screenSession_->SetScreenSceneDpiChangeListener(func);
        DestroyScreenSceneFunc destroyFunc = [screenScene = screenScene_]() {
            if (screenScene) {
                screenScene->Destroy();
            }
        };
        screenSession_->SetScreenSceneDestroyListener(destroyFunc);
    }
}

napi_value JsScreenSession::ConvertMapToJsMap(napi_env env, const std::map<int32_t, int32_t>& map)
{
    constexpr size_t argCount = 2;
    napi_value global;
    napi_value mapConstructor;
    napi_status status = napi_get_global(env, &global);
    if (status != napi_ok) {
        napi_throw_error(env, nullptr, "Failed to get global object");
        return nullptr;
    }

    status = napi_get_named_property(env, global, "Map", &mapConstructor);
    if (status != napi_ok || mapConstructor == nullptr) {
        napi_throw_error(env, nullptr, "Failed to get Map constructor");
        return nullptr;
    }

    napi_value jsMap;
    status = napi_new_instance(env, mapConstructor, 0, nullptr, &jsMap);
    if (status != napi_ok || jsMap == nullptr) {
        napi_throw_error(env, nullptr, "Failed to create Map instance");
        return nullptr;
    }

    napi_value setMethod;
    status = napi_get_named_property(env, jsMap, "set", &setMethod);
    if (status != napi_ok || setMethod == nullptr) {
        napi_throw_error(env, nullptr, "Failed to get Map.set method");
        return nullptr;
    }

    for (const auto& pair : map) {
        napi_value key, value;
        status = napi_create_int32(env, pair.first, &key);
        if (status != napi_ok) {
            napi_throw_error(env, nullptr, "Failed to create key int32");
            return nullptr;
        }
        status = napi_create_int32(env, pair.second, &value);
        if (status != napi_ok) {
            napi_throw_error(env, nullptr, "Failed to create value int32");
            return nullptr;
        }

        napi_value args[argCount] = {key, value};
        status = napi_call_function(env, jsMap, setMethod, argCount, args, nullptr);
        if (status != napi_ok) {
            napi_throw_error(env, nullptr, "Failed to call Map.set");
            return nullptr;
        }
    }

    TLOGD(WmsLogTag::DMS, "Converted map with %{public}zu entries to JS Map", map.size());
    return jsMap;
}

JsScreenSession::~JsScreenSession()
{
    TLOGI(WmsLogTag::DMS, "~JsScreenSession");
}

std::shared_ptr<NativeReference> JsScreenSession::GetJSCallback(const std::string& callbackType)
{
    HITRACE_METER_FMT(HITRACE_TAG_WINDOW_MANAGER, "JsScreenSession::GetJSCallback[%s]", callbackType.c_str());
    std::shared_ptr<NativeReference> jsCallBack = nullptr;
    std::shared_lock<std::shared_mutex> lock(jsCbMapMutex_);
    auto iter = mCallback_.find(callbackType);
    if (iter == mCallback_.end()) {
        TLOGE(WmsLogTag::DEFAULT, "%{public}s callback not found!", callbackType.c_str());
    } else {
        jsCallBack = iter->second;
    }
    return jsCallBack;
}

bool JsScreenSession::IsCallbackRegistered(const std::string& callbackType)
{
    HITRACE_METER_FMT(HITRACE_TAG_WINDOW_MANAGER, "JsScreenSession::IsCallbackRegistered[%s]", callbackType.c_str());
    std::shared_lock<std::shared_mutex> lock(jsCbMapMutex_);
    if (mCallback_.count(callbackType) > 0) {
        return true;
    }
    return false;
}

napi_value JsScreenSession::LoadContent(napi_env env, napi_callback_info info)
{
    JsScreenSession* me = CheckParamsAndGetThis<JsScreenSession>(env, info);
    return (me != nullptr) ? me->OnLoadContent(env, info) : nullptr;
}

napi_value JsScreenSession::OnLoadContent(napi_env env, napi_callback_info info)
{
    TLOGW(WmsLogTag::DMS, "[NAPI]JsScreenSession::OnLoadContent");
    size_t argc = 4;
    napi_value argv[4] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc < 2) {  // 2: params num
        TLOGE(WmsLogTag::DMS, "[NAPI]Argc is invalid: %{public}zu", argc);
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }
    std::string contentUrl;
    napi_value context = argv[1];
    napi_value storage = argc < 3 ? nullptr : argv[2];
    if (!ConvertFromJsValue(env, argv[0], contentUrl)) {
        TLOGE(WmsLogTag::DMS, "[NAPI]Failed to convert parameter to content url");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM),
            "Input parameter is missing or invalid"));
        return NapiGetUndefined(env);
    }

    if (context == nullptr) {
        TLOGE(WmsLogTag::DMS, "[NAPI]Failed to get context object");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_STATE_ABNORMALLY)));
        return NapiGetUndefined(env);
    }
    void* pointerResult = nullptr;
    napi_unwrap(env, context, &pointerResult);
    auto contextNativePointer = static_cast<std::weak_ptr<Context>*>(pointerResult);
    if (contextNativePointer == nullptr) {
        TLOGE(WmsLogTag::DMS, "[NAPI]Failed to get context pointer from js object");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_STATE_ABNORMALLY)));
        return NapiGetUndefined(env);
    }
    auto contextWeakPtr = *contextNativePointer;

    std::shared_ptr<NativeReference> contentStorage = nullptr;
    if (storage != nullptr) {
        napi_ref ref = nullptr;
        napi_create_reference(env, storage, 1, &ref);
        contentStorage = std::shared_ptr<NativeReference>(reinterpret_cast<NativeReference*>(ref));
    }

    return ScheduleLoadContentTask(env, contentUrl, contextWeakPtr, contentStorage);
}

napi_value JsScreenSession::ScheduleLoadContentTask(napi_env env, const std::string& contentUrl,
    std::weak_ptr<Context> contextWeakPtr, std::shared_ptr<NativeReference> contentStorage)
{
    if (screenScene_ == nullptr) {
        TLOGE(WmsLogTag::DMS, "[NAPI]screenScene is nullptr");
        return NapiGetUndefined(env);
    }
    napi_value nativeStorage = contentStorage ? contentStorage->GetNapiValue() : nullptr;
    screenScene_->LoadContent(contentUrl, env, nativeStorage, contextWeakPtr.lock().get());
    return NapiGetUndefined(env);
}

napi_value JsScreenSession::SetScreenRotationLocked(napi_env env, napi_callback_info info)
{
    JsScreenSession* me = CheckParamsAndGetThis<JsScreenSession>(env, info);
    return (me != nullptr) ? me->OnSetScreenRotationLocked(env, info) : nullptr;
}

napi_value JsScreenSession::OnSetScreenRotationLocked(napi_env env, napi_callback_info info)
{
    TLOGI(WmsLogTag::DMS, "Called");
    size_t argc = 4;
    napi_value argv[4] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc < 1) {
        TLOGE(WmsLogTag::DMS, "[NAPI]Argc is invalid: %{public}zu", argc);
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(DmErrorCode::DM_ERROR_INVALID_PARAM)));
        return NapiGetUndefined(env);
    }
    bool isLocked = true;
    napi_value nativeVal = argv[0];
    if (nativeVal == nullptr) {
        TLOGE(WmsLogTag::DMS, "ConvertNativeValueTo isLocked failed!");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(DmErrorCode::DM_ERROR_INVALID_PARAM)));
        return NapiGetUndefined(env);
    }
    napi_get_value_bool(env, nativeVal, &isLocked);
    if (screenSession_ == nullptr) {
        TLOGE(WmsLogTag::DMS, "Failed to register screen change listener, session is null!");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(DmErrorCode::DM_ERROR_INVALID_PARAM)));
        return NapiGetUndefined(env);
    }
    screenSession_->SetScreenRotationLockedFromJs(isLocked);
    napi_value lastParam = nullptr;
    napi_value result = nullptr;
    std::unique_ptr<NapiAsyncTask> napiAsyncTask = CreateEmptyAsyncTask(env, lastParam, &result);
    auto asyncTask = [isLocked, env, task = napiAsyncTask.get()]() {
        HITRACE_METER_FMT(HITRACE_TAG_WINDOW_MANAGER, "JsScreenSession::OnSetScreenRotationLocked");
        auto res = DM_JS_TO_ERROR_CODE_MAP.at(
            SingletonContainer::Get<ScreenManager>().SetScreenRotationLockedFromJs(isLocked));
        if (res == DmErrorCode::DM_OK) {
            task->Resolve(env, NapiGetUndefined(env));
            TLOGNI(WmsLogTag::DMS, "OnSetScreenRotationLocked success");
        } else {
            task->Reject(env, CreateJsError(env, static_cast<int32_t>(res),
                                                "JsScreenSession::OnSetScreenRotationLocked failed."));
            TLOGNE(WmsLogTag::DMS, "OnSetScreenRotationLocked failed");
        }
        delete task;
    };
    if (napi_send_event(env, asyncTask, napi_eprio_immediate, "OnSetScreenRotationLocked") != napi_status::napi_ok) {
        napiAsyncTask->Reject(env, CreateJsError(env,
                static_cast<int32_t>(DmErrorCode::DM_ERROR_INVALID_SCREEN), "Send event failed!"));
    } else {
        napiAsyncTask.release();
    }
    return result;
}

napi_value JsScreenSession::SetTouchEnabled(napi_env env, napi_callback_info info)
{
    JsScreenSession* me = CheckParamsAndGetThis<JsScreenSession>(env, info);
    return (me != nullptr) ? me->OnSetTouchEnabled(env, info) : nullptr;
}

napi_value JsScreenSession::OnSetTouchEnabled(napi_env env, napi_callback_info info)
{
    TLOGI(WmsLogTag::WMS_EVENT, "napi called");
    size_t argc = 4;
    napi_value argv[4] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc != ARGC_ONE) {
        TLOGE(WmsLogTag::WMS_EVENT, "[NAPI]Argc is invalid: %{public}zu", argc);
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(DmErrorCode::DM_ERROR_INVALID_PARAM)));
        return NapiGetUndefined(env);
    }
    bool isTouchEnabled = true;
    napi_value nativeVal = argv[0];
    if (nativeVal == nullptr) {
        TLOGE(WmsLogTag::WMS_EVENT, "ConvertNativeValueTo isTouchEnabled failed!");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(DmErrorCode::DM_ERROR_INVALID_PARAM)));
        return NapiGetUndefined(env);
    }
    napi_get_value_bool(env, nativeVal, &isTouchEnabled);
    if (screenSession_ == nullptr) {
        TLOGE(WmsLogTag::WMS_EVENT, "Failed to register screen change listener, session is null!");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(DmErrorCode::DM_ERROR_INVALID_PARAM)));
        return NapiGetUndefined(env);
    }
    screenSession_->SetTouchEnabledFromJs(isTouchEnabled);
    return NapiGetUndefined(env);
}

void JsScreenSession::RegisterScreenChangeListener()
{
    if (screenSession_ == nullptr) {
        TLOGE(WmsLogTag::DMS, "Failed to register screen change listener, session is null!");
        return;
    }

    screenSession_->RegisterScreenChangeListener(this);
    TLOGI(WmsLogTag::DMS, "register screen change listener success.");
}

void JsScreenSession::UnRegisterScreenChangeListener()
{
    if (screenSession_ == nullptr) {
        TLOGE(WmsLogTag::DMS, "Failed to unregister screen change listener, session is null!");
        return;
    }

    screenSession_->UnregisterScreenChangeListener(this);
    TLOGI(WmsLogTag::DMS, "unregister screen change listener success.");
}

napi_value JsScreenSession::RegisterCallback(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::DMS, "Register callback.");
    JsScreenSession* me = CheckParamsAndGetThis<JsScreenSession>(env, info);
    return (me != nullptr) ? me->OnRegisterCallback(env, info) : nullptr;
}

napi_value JsScreenSession::OnRegisterCallback(napi_env env, napi_callback_info info)
{
    TLOGI(WmsLogTag::DMS, "On register callback.");
    size_t argc = 4;
    napi_value argv[4] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc < 2) { // 2: params num
        TLOGE(WmsLogTag::DMS, "Argc is invalid: %{public}zu", argc);
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM)));
        return NapiGetUndefined(env);
    }

    std::string callbackType;
    if (!ConvertFromJsValue(env, argv[0], callbackType)) {
        TLOGE(WmsLogTag::DMS, "Failed to convert parameter to callback type.");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM)));
        return NapiGetUndefined(env);
    }

    napi_value callback = argv[1];
    if (!NapiIsCallable(env, callback)) {
        TLOGE(WmsLogTag::DMS, "Failed to register callback, callback is not callable!");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM)));
        return NapiGetUndefined(env);
    }

    if (IsCallbackRegistered(callbackType)) {
        TLOGE(WmsLogTag::DMS, "Failed to register callback, callback is already existed!");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_REPEAT_OPERATION)));
        return NapiGetUndefined(env);
    }

    napi_ref result = nullptr;
    napi_create_reference(env, callback, 1, &result);
    std::shared_ptr<NativeReference> callbackRef(reinterpret_cast<NativeReference*>(result));
    {
        std::unique_lock<std::shared_mutex> lock(jsCbMapMutex_);
        mCallback_[callbackType] = callbackRef;
    }
    RegisterScreenChangeListener();

    return NapiGetUndefined(env);
}

napi_value JsScreenSession::DestroyContent(napi_env env, napi_callback_info info)
{
    JsScreenSession* me = CheckParamsAndGetThis<JsScreenSession>(env, info);
    return (me != nullptr) ? me->OnDestroyContent(env, info) : nullptr;
}

napi_value JsScreenSession::OnDestroyContent(napi_env env, napi_callback_info info)
{
    TLOGI(WmsLogTag::DMS, "[NAPI]OnDestroyContent");
    size_t argc = 1;
    napi_value argv[1] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc > 0) { // 0: params num
        TLOGE(WmsLogTag::DMS, "Argc is invalid: %{public}zu", argc);
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM)));
        return NapiGetUndefined(env);
    }

    if (screenScene_ == nullptr) {
        TLOGE(WmsLogTag::DMS, "screenScene_ is nullptr");
        std::string errMsg = "[destroyContent]message: content is null, possible causes: content has been destroyed.";
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WmErrorCode::WM_ERROR_STATE_ABNORMALLY), errMsg));
        return NapiGetUndefined(env);
    }

    screenScene_->Destroy();
    TLOGI(WmsLogTag::DMS, "destroy screen scene finish");
    if (screenSession_ != nullptr) {
        TLOGW(WmsLogTag::DMS, "destroy screen scene, screenId:%{public}" PRIu64 ", sessionId:%{public}" PRIu64,
            screenSession_->GetScreenId(), screenSession_->GetSessionId());
    }
    return NapiGetUndefined(env);
}

napi_value JsScreenSession::ReleaseResource(napi_env env, napi_callback_info info)
{
    JsScreenSession* me = CheckParamsAndGetThis<JsScreenSession>(env, info);
    return (me != nullptr) ? me->OnReleaseResource(env, info) : nullptr;
}

napi_value JsScreenSession::OnReleaseResource(napi_env env, napi_callback_info info)
{
    TLOGI(WmsLogTag::DMS, "[NAPI]");
    mCallback_.clear();
    return NapiGetUndefined(env);
}

napi_value JsScreenSession::GetScreenUIContext(napi_env env, napi_callback_info info)
{
    JsScreenSession* me = CheckParamsAndGetThis<JsScreenSession>(env, info);
    return (me != nullptr) ? me->OnGetScreenUIContext(env, info) : nullptr;
}

napi_value JsScreenSession::OnGetScreenUIContext(napi_env env, napi_callback_info info)
{
    TLOGI(WmsLogTag::DMS, "[NAPI]OnGetScreenUIContext");
    size_t argc = 1;
    napi_value argv[1] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc > 0) { // 0: params num
        TLOGE(WmsLogTag::DMS, "Argc is invalid: %{public}zu", argc);
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WSErrorCode::WS_ERROR_INVALID_PARAM)));
        return NapiGetUndefined(env);
    }
    std::string errMsg = "[getScreenUIContext]message: content is null, possible causes: content does not exist.";
    if (screenScene_ == nullptr) {
        TLOGE(WmsLogTag::DMS, "screenScene_ is nullptr");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WmErrorCode::WM_ERROR_STATE_ABNORMALLY), errMsg));
        return NapiGetUndefined(env);
    }
    const auto& uiContent = screenScene_->GetUIContent();
    if (uiContent == nullptr) {
        TLOGE(WmsLogTag::DMS, "uiContent is nullptr");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WmErrorCode::WM_ERROR_STATE_ABNORMALLY), errMsg));
        return NapiGetUndefined(env);
    }
    napi_value uiContext = uiContent->GetUINapiContext();
    if (uiContext == nullptr) {
        TLOGE(WmsLogTag::DMS, "uiContext obtained from jsEngine is nullptr");
        errMsg = "[getScreenUIContext]message: content is null, " \
            "possible causes: content obtained from jsEngine is nullptr.";
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WmErrorCode::WM_ERROR_STATE_ABNORMALLY), errMsg));
        return NapiGetUndefined(env);
    }
    TLOGI(WmsLogTag::DMS, "success");
    return uiContext;
}

void JsScreenSession::CallJsCallback(const std::string& callbackType)
{
    TLOGI(WmsLogTag::DMS, "Call js callback: %{public}s.", callbackType.c_str());
    if (!IsCallbackRegistered(callbackType)) {
        TLOGE(WmsLogTag::DMS, "Callback is unregistered!");
        return;
    }
    if (callbackType == ON_DISCONNECTION_CALLBACK) {
        TLOGE(WmsLogTag::DMS, "Call js callback %{public}s start", callbackType.c_str());
        UnRegisterScreenChangeListener();
    }
    wptr<ScreenSession> screenSessionWeak(screenSession_);
    auto jsCallbackRef = GetJSCallback(callbackType);
    // ensure CallJsCallback ends before asyncTask, prevent jsCallbackRef deref not in js thread.
    std::lock_guard<std::mutex> lock(callbackMutex_);
    auto asyncTask = [jsCallbackRef, callbackType, screenSessionWeak, env = env_, &callbackMutex = callbackMutex_]() {
        HITRACE_METER_FMT(HITRACE_TAG_WINDOW_MANAGER, "JsScreenSession::CallJsCallback");
        if (jsCallbackRef == nullptr) {
            TLOGNE(WmsLogTag::DMS, "Call js callback %{public}s failed, jsCallbackRef is null!", callbackType.c_str());
            return;
        }
        auto method = jsCallbackRef->GetNapiValue();
        if (method == nullptr) {
            TLOGNE(WmsLogTag::DMS, "Call js callback %{public}s failed, method is null!", callbackType.c_str());
            return;
        }
        std::lock_guard<std::mutex> lock(callbackMutex);
        if (callbackType == ON_CONNECTION_CALLBACK || callbackType == ON_DISCONNECTION_CALLBACK) {
            auto screenSession = screenSessionWeak.promote();
            if (screenSession == nullptr) {
                TLOGNE(WmsLogTag::DMS, "Call js callback %{public}s failed, screenSession is null!",
                    callbackType.c_str());
                return;
            }
            napi_value argv[] = { JsScreenUtils::CreateJsScreenProperty(
                env, screenSession->GetScreenProperty()) };
            napi_call_function(env, NapiGetUndefined(env), method, ArraySize(argv), argv, nullptr);
        } else {
            napi_value argv[] = {};
            napi_call_function(env, NapiGetUndefined(env), method, 0, argv, nullptr);
        }
        TLOGNI(WmsLogTag::DMS, "The js callback has been executed: %{public}s.", callbackType.c_str());
    };
    if (env_ != nullptr) {
        napi_status ret = napi_send_event(env_, asyncTask, napi_eprio_vip, "CallJsCallback");
        if (ret != napi_status::napi_ok) {
            TLOGE(WmsLogTag::DMS, "Failed to SendEvent.");
        }
    } else {
        TLOGE(WmsLogTag::DMS, "env is nullptr");
    }
}

void JsScreenSession::OnConnect(ScreenId screenId)
{
    CallJsCallback(ON_CONNECTION_CALLBACK);
}

void JsScreenSession::OnDisconnect(ScreenId screenId)
{
    CallJsCallback(ON_DISCONNECTION_CALLBACK);
}

void JsScreenSession::OnSensorRotationChange(float sensorRotation, ScreenId screenId, bool isSwitchUser)
{
    const std::string callbackType = ON_SENSOR_ROTATION_CHANGE_CALLBACK;
    if (!IsCallbackRegistered(callbackType)) {
        TLOGE(WmsLogTag::DMS, "Callback %{public}s is unregistered!", callbackType.c_str());
        return;
    }

    auto jsCallbackRef = GetJSCallback(callbackType);
    wptr<ScreenSession> screenSessionWeak(screenSession_);
    auto asyncTask = [jsCallbackRef, callbackType, screenSessionWeak, sensorRotation, isSwitchUser, env = env_]() {
        if (jsCallbackRef == nullptr) {
            TLOGNE(WmsLogTag::DMS, "Call js callback %{public}s failed, jsCallbackRef is null!", callbackType.c_str());
            return;
        }
        auto method = jsCallbackRef->GetNapiValue();
        if (method == nullptr) {
            TLOGNE(WmsLogTag::DMS, "Call js callback %{public}s failed, method is null!", callbackType.c_str());
            return;
        }
        auto screenSession = screenSessionWeak.promote();
        if (screenSession == nullptr) {
            TLOGNE(WmsLogTag::DMS, "Call js callback %{public}s failed, screenSession is null!", callbackType.c_str());
            return;
        }
        napi_value argv[] = { CreateJsValue(env, sensorRotation), CreateJsValue(env, isSwitchUser) };
        napi_call_function(env, NapiGetUndefined(env), method, ArraySize(argv), argv, nullptr);
    };

    if (env_ != nullptr) {
        napi_status ret = napi_send_event(env_, asyncTask, napi_eprio_immediate, "OnSensorRotationChange");
        if (ret != napi_status::napi_ok) {
            TLOGE(WmsLogTag::DMS, "Failed to SendEvent.");
        }
    } else {
        TLOGE(WmsLogTag::DMS, "env is nullptr");
    }
}

void JsScreenSession::OnScreenOrientationChange(float screenOrientation, ScreenId screenId)
{
    const std::string callbackType = ON_SCREEN_ORIENTATION_CHANGE_CALLBACK;
    TLOGI(WmsLogTag::DMS, "Call js callback: %{public}s.", callbackType.c_str());
    if (!IsCallbackRegistered(callbackType)) {
        TLOGE(WmsLogTag::DMS, "Callback %{public}s is unregistered!", callbackType.c_str());
        return;
    }

    auto jsCallbackRef = GetJSCallback(callbackType);
    wptr<ScreenSession> screenSessionWeak(screenSession_);
    auto asyncTask = [jsCallbackRef, callbackType, screenSessionWeak, screenOrientation, env = env_]() {
        HITRACE_METER_FMT(HITRACE_TAG_WINDOW_MANAGER, "jsScreenSession::OnScreenOrientationChange");
        if (jsCallbackRef == nullptr) {
            TLOGNE(WmsLogTag::DMS, "Call js callback %{public}s failed, jsCallbackRef is null!", callbackType.c_str());
            return;
        }
        auto method = jsCallbackRef->GetNapiValue();
        if (method == nullptr) {
            TLOGNE(WmsLogTag::DMS, "Call js callback %{public}s failed, method is null!", callbackType.c_str());
            return;
        }
        auto screenSession = screenSessionWeak.promote();
        if (screenSession == nullptr) {
            TLOGNE(WmsLogTag::DMS, "Call js callback %{public}s failed, screenSession is null!", callbackType.c_str());
            return;
        }
        napi_value argv[] = { CreateJsValue(env, screenOrientation) };
        napi_call_function(env, NapiGetUndefined(env), method, ArraySize(argv), argv, nullptr);
    };
    if (env_ != nullptr) {
        napi_status ret = napi_send_event(env_, asyncTask, napi_eprio_immediate, "OnScreenOrientationChange");
        if (ret != napi_status::napi_ok) {
            TLOGE(WmsLogTag::DMS, "Failed to SendEvent.");
        }
    } else {
        TLOGE(WmsLogTag::DMS, "env is nullptr");
    }
}

void JsScreenSession::OnPropertyChange(const ScreenProperty& newProperty, ScreenPropertyChangeReason reason,
    ScreenId screenId)
{
    const std::string callbackType = ON_PROPERTY_CHANGE_CALLBACK;
    TLOGD(WmsLogTag::DMS, "Call js callback: %{public}s.", callbackType.c_str());
    if (!IsCallbackRegistered(callbackType)) {
        TLOGE(WmsLogTag::DMS, "Callback %{public}s is unregistered!", callbackType.c_str());
        return;
    }
    if (reason == ScreenPropertyChangeReason::RELATIVE_POSITION_CHANGE) {
        TLOGW(WmsLogTag::DMS, "relative position change, no need call callback");
        return;
    }
    auto jsCallbackRef = GetJSCallback(callbackType);
    wptr<ScreenSession> screenSessionWeak(screenSession_);
    auto asyncTask = [jsCallbackRef, callbackType, screenSessionWeak, newProperty, reason, env = env_]() {
        HITRACE_METER_FMT(HITRACE_TAG_WINDOW_MANAGER, "jsScreenSession::OnPropertyChange");
        if (jsCallbackRef == nullptr) {
            TLOGNE(WmsLogTag::DMS, "Call js callback %{public}s failed, jsCallbackRef is null!", callbackType.c_str());
            return;
        }
        auto method = jsCallbackRef->GetNapiValue();
        if (method == nullptr) {
            TLOGNE(WmsLogTag::DMS, "Call js callback %{public}s failed, method is null!", callbackType.c_str());
            return;
        }
        auto screenSession = screenSessionWeak.promote();
        if (screenSession == nullptr) {
            TLOGNE(WmsLogTag::DMS, "Call js callback %{public}s failed, screenSession is null!", callbackType.c_str());
            return;
        }
        napi_value propertyChangeReason = CreateJsValue(env, static_cast<int32_t>(reason));
        auto property = screenSession->GetScreenProperty();
        napi_value argv[] = { JsScreenUtils::CreateJsScreenProperty(env, property), propertyChangeReason };
        napi_call_function(env, NapiGetUndefined(env), method, ArraySize(argv), argv, nullptr);
    };
    if (env_ != nullptr) {
        napi_status ret = napi_send_event(env_, asyncTask, napi_eprio_immediate, "OnPropertyChange");
        if (ret != napi_status::napi_ok) {
            TLOGE(WmsLogTag::DMS, "Failed to SendEvent.");
        }
    } else {
        TLOGE(WmsLogTag::DMS, "env is nullptr");
    }
}

void JsScreenSession::OnScreenDensityChange()
{
    const std::string callbackType = ON_SCREEN_DENSITY_CHANGE;
    TLOGD(WmsLogTag::DMS, "Call js callback: %{public}s.", callbackType.c_str());
    if (!IsCallbackRegistered(callbackType)) {
        TLOGE(WmsLogTag::DMS, "Callback %{public}s is unregistered!", callbackType.c_str());
        return;
    }

    auto jsCallbackRef = GetJSCallback(callbackType);
    wptr<ScreenSession> screenSessionWeak(screenSession_);
    auto asyncTask = [jsCallbackRef, callbackType, screenSessionWeak, env = env_]() {
        HITRACE_METER_FMT(HITRACE_TAG_WINDOW_MANAGER, "jsScreenSession::OnScreenDensityChange");
        if (jsCallbackRef == nullptr) {
            TLOGNE(WmsLogTag::DMS, "Call js callback %{public}s failed, jsCallbackRef is null!", callbackType.c_str());
            return;
        }
        auto method = jsCallbackRef->GetNapiValue();
        if (method == nullptr) {
            TLOGNE(WmsLogTag::DMS, "Call js callback %{public}s failed, method is null!", callbackType.c_str());
            return;
        }
        auto screenSession = screenSessionWeak.promote();
        if (screenSession == nullptr) {
            TLOGNE(WmsLogTag::DMS, "Call js callback %{public}s failed, screenSession is null!", callbackType.c_str());
            return;
        }
        napi_value argv[] = {};
        napi_call_function(env, NapiGetUndefined(env), method, 0, argv, nullptr);
    };
    if (env_ != nullptr) {
        napi_status ret = napi_send_event(env_, asyncTask, napi_eprio_immediate, "OnScreenDensityChange");
        if (ret != napi_status::napi_ok) {
            TLOGE(WmsLogTag::DMS, "Failed to SendEvent.");
        }
    } else {
        TLOGE(WmsLogTag::DMS, "env is nullptr");
    }
}

void JsScreenSession::OnPowerStatusChange(DisplayPowerEvent event, EventStatus eventStatus,
    PowerStateChangeReason reason)
{
    const std::string callbackType = ON_POWER_STATUS_CHANGE_CALLBACK;
    TLOGD(WmsLogTag::DMS, "[UL_POWER]%{public}s.", callbackType.c_str());
    if (!IsCallbackRegistered(callbackType)) {
        TLOGW(WmsLogTag::DMS, "[UL_POWER]%{public}s is unregistered!", callbackType.c_str());
        return;
    }
    auto jsCallbackRef = GetJSCallback(callbackType);
    wptr<ScreenSession> screenSessionWeak(screenSession_);
    auto asyncTask = [jsCallbackRef, callbackType, screenSessionWeak, event, eventStatus, reason, env = env_]() {
        HITRACE_METER_FMT(HITRACE_TAG_WINDOW_MANAGER, "jsScreenSession::OnPowerStatusChange");
        if (jsCallbackRef == nullptr) {
            TLOGNE(WmsLogTag::DMS, "[UL_POWER]%{public}s failed, jsCallbackRef is null!", callbackType.c_str());
            return;
        }
        auto method = jsCallbackRef->GetNapiValue();
        if (method == nullptr) {
            TLOGNE(WmsLogTag::DMS, "[UL_POWER]%{public}s failed, method is null!", callbackType.c_str());
            return;
        }
        auto screenSession = screenSessionWeak.promote();
        if (screenSession == nullptr) {
            TLOGNE(WmsLogTag::DMS, "[UL_POWER]%{public}s failed, screenSession is null!", callbackType.c_str());
            return;
        }
        napi_value displayPowerEvent = CreateJsValue(env, static_cast<int32_t>(event));
        napi_value powerEventStatus = CreateJsValue(env, static_cast<int32_t>(eventStatus));
        napi_value powerStateChangeReason = CreateJsValue(env, static_cast<int32_t>(reason));
        napi_value argv[] = { displayPowerEvent, powerEventStatus, powerStateChangeReason };
        napi_call_function(env, NapiGetUndefined(env), method, ArraySize(argv), argv, nullptr);
    };
    if (env_ != nullptr) {
        napi_status ret = napi_send_event(env_, asyncTask, napi_eprio_vip, "OnPowerStatusChange");
        if (ret != napi_status::napi_ok) {
            TLOGE(WmsLogTag::DMS, "Failed to SendEvent.");
        } else {
            TLOGI(WmsLogTag::DMS, "Sucess to SendEvent.");
        }
    } else {
        TLOGE(WmsLogTag::DMS, "env is nullptr");
    }
}

void JsScreenSession::OnScreenRotationLockedChange(bool isLocked, ScreenId screenId)
{
    const std::string callbackType = ON_SCREEN_ROTATION_LOCKED_CHANGE;
    TLOGD(WmsLogTag::DMS, "Call js callback: %{public}s isLocked:%{public}u.", callbackType.c_str(), isLocked);
    if (!IsCallbackRegistered(callbackType)) {
        TLOGE(WmsLogTag::DMS, "Callback %{public}s is unregistered!", callbackType.c_str());
        return;
    }

    auto jsCallbackRef = GetJSCallback(callbackType);
    auto asyncTask = [jsCallbackRef, callbackType, isLocked, env = env_]() {
        HITRACE_METER_FMT(HITRACE_TAG_WINDOW_MANAGER, "jsScreenSession::OnScreenRotationLockedChange");
        if (jsCallbackRef == nullptr) {
            TLOGNE(WmsLogTag::DMS, "Call js callback %{public}s failed, jsCallbackRef is null!", callbackType.c_str());
            return;
        }
        auto method = jsCallbackRef->GetNapiValue();
        if (method == nullptr) {
            TLOGNE(WmsLogTag::DMS, "Call js callback %{public}s failed, method is null!", callbackType.c_str());
            return;
        }
        napi_value argv[] = { CreateJsValue(env, isLocked) };
        napi_call_function(env, NapiGetUndefined(env), method, ArraySize(argv), argv, nullptr);
    };
    if (env_ != nullptr) {
        napi_status ret = napi_send_event(env_, asyncTask, napi_eprio_immediate, "OnScreenRotationLockedChange");
        if (ret != napi_status::napi_ok) {
            TLOGE(WmsLogTag::DMS, "Failed to SendEvent.");
        }
    } else {
        TLOGE(WmsLogTag::DMS, "env is nullptr");
    }
}

void JsScreenSession::OnScreenExtendChange(ScreenId mainScreenId, ScreenId extendScreenId)
{
    const std::string callbackType = ON_SCREEN_EXTEND_CHANGE;
    if (!IsCallbackRegistered(callbackType)) {
        TLOGW(WmsLogTag::DMS, "Callback is unregistered!");
        return;
    }
    auto jsCallbackRef = GetJSCallback(callbackType);
    wptr<ScreenSession> screenSessionWeak(screenSession_);
    auto asyncTask = [jsCallbackRef, callbackType, mainScreenId, extendScreenId, env = env_]() {
        HITRACE_METER_FMT(HITRACE_TAG_WINDOW_MANAGER, "jsScreenSession::OnScreenDensityChange");
        if (jsCallbackRef == nullptr) {
            TLOGNE(WmsLogTag::DMS, "Call js callback failed, jsCallbackRef is null!");
            return;
        }
        auto method = jsCallbackRef->GetNapiValue();
        if (method == nullptr) {
            TLOGNE(WmsLogTag::DMS, "Call js callback failed, method is null!");
            return;
        }
        napi_value mainId = CreateJsValue(env, static_cast<int64_t>(mainScreenId));
        napi_value extendId = CreateJsValue(env, static_cast<int64_t>(extendScreenId));
        napi_value argv[] = { mainId, extendId };
        napi_call_function(env, NapiGetUndefined(env), method, ArraySize(argv), argv, nullptr);
    };
    if (env_ != nullptr) {
        napi_status ret = napi_send_event(env_, asyncTask, napi_eprio_immediate, "OnScreenExtendChange");
        if (ret != napi_status::napi_ok) {
            TLOGE(WmsLogTag::DMS, "OnScreenExtendChange: Failed to SendEvent.");
        }
    } else {
        TLOGE(WmsLogTag::DMS, "OnScreenExtendChange: env is nullptr");
    }
}

void JsScreenSession::OnHoverStatusChange(int32_t hoverStatus, bool needRotate, ScreenId screenId)
{
    const std::string callbackType = ON_HOVER_STATUS_CHANGE_CALLBACK;
    TLOGI(WmsLogTag::DMS, "Call js callback: %{public}s.", callbackType.c_str());
    if (!IsCallbackRegistered(callbackType)) {
        TLOGE(WmsLogTag::DMS, "Callback %{public}s is unregistered!", callbackType.c_str());
        return;
    }

    auto jsCallbackRef = GetJSCallback(callbackType);
    wptr<ScreenSession> screenSessionWeak(screenSession_);
    auto napiTask = [jsCallbackRef, callbackType, screenSessionWeak, hoverStatus, needRotate, env = env_]() {
        HITRACE_METER_FMT(HITRACE_TAG_WINDOW_MANAGER, "jsScreenSession::OnHoverStatusChange");
        if (jsCallbackRef == nullptr) {
            TLOGNE(WmsLogTag::DMS, "Call js callback %{public}s failed, jsCallbackRef is null!", callbackType.c_str());
            return;
        }
        auto method = jsCallbackRef->GetNapiValue();
        if (method == nullptr) {
            TLOGNE(WmsLogTag::DMS, "Call js callback %{public}s failed, method is null!", callbackType.c_str());
            return;
        }
        auto screenSession = screenSessionWeak.promote();
        if (screenSession == nullptr) {
            TLOGNE(WmsLogTag::DMS, "Call js callback %{public}s failed, screenSession is null!", callbackType.c_str());
            return;
        }
        napi_value argv[] = { CreateJsValue(env, hoverStatus), CreateJsValue(env, needRotate) };
        napi_call_function(env, NapiGetUndefined(env), method, ArraySize(argv), argv, nullptr);
    };
    if (env_ != nullptr) {
        napi_status ret = napi_send_event(env_, napiTask, napi_eprio_immediate, "OnHoverStatusChange");
        if (ret != napi_status::napi_ok) {
            TLOGE(WmsLogTag::DMS, "Failed to SendEvent.");
        }
    } else {
        TLOGE(WmsLogTag::DMS, "env is nullptr");
    }
}

void JsScreenSession::OnScreenCaptureNotify(ScreenId mainScreenId, int32_t uid, const std::string& clientName)
{
    const std::string callbackType = ON_SCREEN_CAPTURE_NOTIFY;
    if (!IsCallbackRegistered(callbackType)) {
        TLOGW(WmsLogTag::DMS, "Callback is unregistered!");
        return;
    }
    auto jsCallbackRef = GetJSCallback(callbackType);
    auto asyncTask = [jsCallbackRef, callbackType, mainScreenId, uid, clientName, env = env_]() {
        HITRACE_METER_FMT(HITRACE_TAG_WINDOW_MANAGER, "jsScreenSession::OnScreenCaptureNotify");
        if (jsCallbackRef == nullptr) {
            TLOGNE(WmsLogTag::DMS, "Call js callback failed, jsCallbackRef is null!");
            return;
        }
        auto method = jsCallbackRef->GetNapiValue();
        if (method == nullptr) {
            TLOGNE(WmsLogTag::DMS, "Call js callback failed, method is null!");
            return;
        }
        napi_value mainId = CreateJsValue(env, static_cast<int64_t>(mainScreenId));
        napi_value clientUid = CreateJsValue(env, uid);
        napi_value client = CreateJsValue(env, clientName);
        napi_value argv[] = { mainId, clientUid, client };
        napi_call_function(env, NapiGetUndefined(env), method, ArraySize(argv), argv, nullptr);
    };
    if (env_ != nullptr) {
        napi_status ret = napi_send_event(env_, asyncTask, napi_eprio_immediate, "OnScreenCaptureNotify");
        if (ret != napi_status::napi_ok) {
            TLOGE(WmsLogTag::DMS, "OnScreenCaptureNotify: Failed to SendEvent.");
        }
    } else {
        TLOGE(WmsLogTag::DMS, "OnScreenCaptureNotify: env is nullptr");
    }
}

void JsScreenSession::OnCameraBackSelfieChange(bool isCameraBackSelfie, ScreenId screenId)
{
    const std::string callbackType = ON_CAMERA_BACKSELFIE_CHANGE_CALLBACK;
    TLOGI(WmsLogTag::DMS, "Call js callback: %{public}s.", callbackType.c_str());
    if (!IsCallbackRegistered(callbackType)) {
        TLOGE(WmsLogTag::DMS, "Callback %{public}s is unregistered!", callbackType.c_str());
        return;
    }

    auto jsCallbackRef = GetJSCallback(callbackType);
    wptr<ScreenSession> screenSessionWeak(screenSession_);
    auto napiTask = [jsCallbackRef, callbackType, screenSessionWeak, isCameraBackSelfie, env = env_]() {
        HITRACE_METER_FMT(HITRACE_TAG_WINDOW_MANAGER, "jsScreenSession::OnCameraBackSelfieChange");
        if (jsCallbackRef == nullptr) {
            TLOGNE(WmsLogTag::DMS, "Call js callback %{public}s failed, jsCallbackRef is null!", callbackType.c_str());
            return;
        }
        auto method = jsCallbackRef->GetNapiValue();
        if (method == nullptr) {
            TLOGNE(WmsLogTag::DMS, "Call js callback %{public}s failed, method is null!", callbackType.c_str());
            return;
        }
        auto screenSession = screenSessionWeak.promote();
        if (screenSession == nullptr) {
            TLOGNE(WmsLogTag::DMS, "Call js callback %{public}s failed, screenSession is null!", callbackType.c_str());
            return;
        }
        napi_value argv[] = { CreateJsValue(env, isCameraBackSelfie) };
        napi_call_function(env, NapiGetUndefined(env), method, ArraySize(argv), argv, nullptr);
    };
    if (env_ != nullptr) {
        napi_status ret = napi_send_event(env_, napiTask, napi_eprio_immediate, "OnCameraBackSelfieChange");
        if (ret != napi_status::napi_ok) {
            TLOGE(WmsLogTag::DMS, "OnCameraBackSelfieChange: Failed to SendEvent.");
        }
    } else {
        TLOGE(WmsLogTag::DMS, "OnCameraBackSelfieChange: env is nullptr");
    }
}

void JsScreenSession::OnSuperFoldStatusChange(ScreenId screenId, SuperFoldStatus superFoldStatus)
{
    const std::string callbackType = ON_SUPER_FOLD_STATUS_CHANGE_CALLBACK;
    TLOGD(WmsLogTag::DMS, "Call js callback: %{public}s.", callbackType.c_str());
    if (!IsCallbackRegistered(callbackType)) {
        TLOGE(WmsLogTag::DMS, "Callback %{public}s is unregistered!", callbackType.c_str());
        return;
    }
    auto jsCallbackRef = GetJSCallback(callbackType);
    auto asyncTask = [jsCallbackRef, callbackType, screenId, superFoldStatus, env = env_]() {
        HITRACE_METER_FMT(HITRACE_TAG_WINDOW_MANAGER, "jsScreenSession::OnSuperFoldStatusChange");
        if (jsCallbackRef == nullptr) {
            TLOGNE(WmsLogTag::DMS, "Call js callback failed, jsCallbackRef is null!");
            return;
        }
        auto method = jsCallbackRef->GetNapiValue();
        if (method == nullptr) {
            TLOGNE(WmsLogTag::DMS, "Call js callback failed, method is null!");
            return;
        }
        napi_value id = CreateJsValue(env, static_cast<int64_t>(screenId));
        napi_value status = CreateJsValue(env, static_cast<int32_t>(superFoldStatus));
        napi_value argv[] = { id, status };
        napi_call_function(env, NapiGetUndefined(env), method, ArraySize(argv), argv, nullptr);
    };
    if (env_ != nullptr) {
        napi_status ret = napi_send_event(env_, asyncTask, napi_eprio_immediate, "OnSuperFoldStatusChange");
        if (ret != napi_status::napi_ok) {
            TLOGE(WmsLogTag::DMS, "Failed to SendEvent.");
        }
    } else {
        TLOGE(WmsLogTag::DMS, "env is nullptr");
    }
}

void JsScreenSession::OnSecondaryReflexionChange(ScreenId screenId, bool isSecondaryReflexion)
{
    const std::string callbackType = ON_SECONDARY_REFLEXION_CHANGE_CALLBACK;
    TLOGD(WmsLogTag::DMS, "Call js callback: %{public}s.", callbackType.c_str());
    if (!IsCallbackRegistered(callbackType)) {
        TLOGE(WmsLogTag::DMS, "Callback %{public}s is unregistered!", callbackType.c_str());
        return;
    }
    auto jsCallbackRef = GetJSCallback(callbackType);
    auto asyncTask = [jsCallbackRef, callbackType, screenId, isSecondaryReflexion, env = env_]() {
        HITRACE_METER_FMT(HITRACE_TAG_WINDOW_MANAGER, "jsScreenSession::OnSecondaryReflexionChange");
        if (jsCallbackRef == nullptr) {
            TLOGNE(WmsLogTag::DMS, "Call js callback failed, jsCallbackRef is null!");
            return;
        }
        auto method = jsCallbackRef->GetNapiValue();
        if (method == nullptr) {
            TLOGNE(WmsLogTag::DMS, "Call js callback failed, method is null!");
            return;
        }
        napi_value id = CreateJsValue(env, static_cast<int64_t>(screenId));
        napi_value status = CreateJsValue(env, static_cast<bool>(isSecondaryReflexion));
        napi_value argv[] = { id, status };
        napi_call_function(env, NapiGetUndefined(env), method, ArraySize(argv), argv, nullptr);
    };
    if (env_ != nullptr) {
        napi_status ret = napi_send_event(env_, asyncTask, napi_eprio_immediate, "OnSecondaryReflexionChange");
        if (ret != napi_status::napi_ok) {
            TLOGE(WmsLogTag::DMS, "Failed to SendEvent.");
        }
    } else {
        TLOGE(WmsLogTag::DMS, "env is nullptr");
    }
}

void JsScreenSession::OnExtendScreenConnectStatusChange(ScreenId screenId,
    ExtendScreenConnectStatus extendScreenConnectStatus)
{
    const std::string callbackType = ON_EXTEND_SCREEN_CONNECT_STATUS_CHANGE_CALLBACK;
    TLOGD(WmsLogTag::DMS, "Call js callback: %{public}s.", callbackType.c_str());
    if (!IsCallbackRegistered(callbackType)) {
        TLOGE(WmsLogTag::DMS, "Callback %{public}s is unregistered!", callbackType.c_str());
        return;
    }
    auto jsCallbackRef = GetJSCallback(callbackType);
    auto asyncTask = [jsCallbackRef, callbackType, screenId, extendScreenConnectStatus, env = env_]() {
        HITRACE_METER_FMT(HITRACE_TAG_WINDOW_MANAGER, "jsScreenSession::OnExtendScreenConnectStatusChange");
        if (jsCallbackRef == nullptr) {
            TLOGNE(WmsLogTag::DMS, "Call js callback failed, jsCallbackRef is null!");
            return;
        }
        auto method = jsCallbackRef->GetNapiValue();
        if (method == nullptr) {
            TLOGNE(WmsLogTag::DMS, "Call js callback failed, method is null!");
            return;
        }
        napi_value id = CreateJsValue(env, static_cast<int64_t>(screenId));
        napi_value status = CreateJsValue(env, static_cast<int32_t>(extendScreenConnectStatus));
        napi_value argv[] = { id, status };
        napi_call_function(env, NapiGetUndefined(env), method, ArraySize(argv), argv, nullptr);
    };
    if (env_ != nullptr) {
        napi_status ret = napi_send_event(env_, asyncTask, napi_eprio_immediate, "OnExtendScreenConnectStatusChange");
        if (ret != napi_status::napi_ok) {
            TLOGE(WmsLogTag::DMS, "OnExtendScreenConnectStatusChange: Failed to SendEvent.");
        }
    } else {
        TLOGE(WmsLogTag::DMS, "OnExtendScreenConnectStatusChange: env is nullptr");
    }
}

void JsScreenSession::OnBeforeScreenPropertyChange(FoldStatus foldStatus)
{
    const std::string callbackType = ON_BEFORE_PROPERTY_CHANGE_CALLBACK;
    TLOGD(WmsLogTag::DMS, "Call js callback: %{public}s.", callbackType.c_str());
    if (!IsCallbackRegistered(callbackType)) {
        TLOGE(WmsLogTag::DMS, "Callback %{public}s is unregistered!", callbackType.c_str());
        return;
    }
    auto jsCallbackRef = GetJSCallback(callbackType);
    auto asyncTask = [jsCallbackRef, callbackType, foldStatus, env = env_]() {
        HITRACE_METER_FMT(HITRACE_TAG_WINDOW_MANAGER, "jsScreenSession::OnBeforeScreenPropertyChange");
        if (jsCallbackRef == nullptr) {
            TLOGNE(WmsLogTag::DMS, "Call js callback failed, jsCallbackRef is null!");
            return;
        }
        auto method = jsCallbackRef->GetNapiValue();
        if (method == nullptr) {
            TLOGNE(WmsLogTag::DMS, "Call js callback failed, method is null!");
            return;
        }
        napi_value status = CreateJsValue(env, static_cast<std::uint32_t>(foldStatus));
        napi_value argv[] = { status };
        napi_call_function(env, NapiGetUndefined(env), method, ArraySize(argv), argv, nullptr);
    };
    if (env_ != nullptr) {
        napi_status ret = napi_send_event(env_, asyncTask, napi_eprio_immediate, "OnBeforeScreenPropertyChange");
        if (ret != napi_status::napi_ok) {
            TLOGE(WmsLogTag::DMS, "Failed to SendEvent.");
        }
    } else {
        TLOGE(WmsLogTag::DMS, "env is nullptr");
    }
}

void JsScreenSession::OnScreenModeChange(ScreenModeChangeEvent screenModeChangeEvent)
{
    const std::string callbackType = ON_SCREEN_CHANGE_CALLBACK;
    TLOGD(WmsLogTag::DMS, "Call js callback: %{public}s.", callbackType.c_str());
    if (!IsCallbackRegistered(callbackType)) {
        TLOGE(WmsLogTag::DMS, "Callback %{public}s is unregistered!", callbackType.c_str());
        return;
    }
    auto jsCallbackRef = GetJSCallback(callbackType);
    auto asyncTask = [jsCallbackRef, callbackType, screenModeChangeEvent, env = env_]() {
        HITRACE_METER_FMT(HITRACE_TAG_WINDOW_MANAGER, "jsScreenSession::OnScreenModeChange");
        if (jsCallbackRef == nullptr) {
            TLOGNE(WmsLogTag::DMS, "Call js callback failed, jsCallbackRef is null!");
            return;
        }
        auto method = jsCallbackRef->GetNapiValue();
        if (method == nullptr) {
            TLOGNE(WmsLogTag::DMS, "Call js callback failed, method is null!");
            return;
        }
        napi_value event = CreateJsValue(env, static_cast<std::uint32_t>(screenModeChangeEvent));
        napi_value argv[] = { event };
        napi_call_function(env, NapiGetUndefined(env), method, ArraySize(argv), argv, nullptr);
    };
    if (env_ != nullptr) {
        napi_status ret = napi_send_event(env_, asyncTask, napi_eprio_immediate, "OnScreenModeChange");
        if (ret != napi_status::napi_ok) {
            TLOGE(WmsLogTag::DMS, "Failed to SendEvent.");
        }
    } else {
        TLOGE(WmsLogTag::DMS, "env is nullptr");
    }
}
} // namespace OHOS::Rosen
