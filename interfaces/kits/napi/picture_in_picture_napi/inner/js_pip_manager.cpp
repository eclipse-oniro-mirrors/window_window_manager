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

#include "js_pip_manager.h"
#include <sstream>
#include "window_manager_hilog.h"
#include "picture_in_picture_manager.h"
#include "xcomponent_controller.h"

namespace OHOS {
namespace Rosen {
using namespace AbilityRuntime;
using namespace Ace;
namespace {
    constexpr int32_t NUMBER_ONE = 1;
    constexpr int32_t NUMBER_TWO = 2;
    constexpr int32_t NUMBER_FOUR = 4;
    const std::string STATE_CHANGE_CB = "stateChange";
    const std::string UPDATE_TYPE_CB = "nodeUpdate";
}

napi_valuetype GetType(napi_env env, napi_value value)
{
    napi_valuetype res = napi_undefined;
    napi_typeof(env, value, &res);
    return res;
}

napi_value NapiGetUndefined(napi_env env)
{
    napi_value result = nullptr;
    napi_get_undefined(env, &result);
    return result;
}

napi_value NapiThrowInvalidParam(napi_env env, std::string msg = "")
{
    napi_throw(env, AbilityRuntime::CreateJsError(env, static_cast<int32_t>(WmErrorCode::WM_ERROR_INVALID_PARAM), msg));
    return NapiGetUndefined(env);
}

napi_value GetSurfaceIdFromJs(napi_env env, napi_value surfaceIdNapiValue,
    sptr<PictureInPictureControllerBase> pipController)
{
    if (surfaceIdNapiValue == nullptr || GetType(env, surfaceIdNapiValue) != napi_string) {
        TLOGE(WmsLogTag::WMS_PIP, "Failed to convert parameter to surface. Invalidate params");
        return NapiGetUndefined(env);
    }
    char buffer[PATH_MAX];
    size_t length = 0;
    uint64_t surfaceId = 0;
    if (napi_get_value_string_utf8(env, surfaceIdNapiValue, buffer, PATH_MAX, &length) != napi_ok) {
        TLOGE(WmsLogTag::WMS_PIP, "Failed to convert parameter to surface");
        return NapiGetUndefined(env);
    }
    std::istringstream inputStream(buffer);
    inputStream >> surfaceId;
    pipController->SetSurfaceId(surfaceId);
    TLOGI(WmsLogTag::WMS_PIP, "surfaceId: %{public}" PRIu64"", surfaceId);
    for (auto& listener : pipController->GetPictureInPictureStartObserver()) {
        if (listener == nullptr) {
            TLOGE(WmsLogTag::WMS_PIP, "one start listener is nullptr");
            continue;
        }
        listener->OnPipStart(pipController->GetControllerId(), pipController->GetWebRequestId(), surfaceId);
    }
    return NapiGetUndefined(env);
}

JsPipManager::JsPipManager()
{
    listenerCodeMap_ = {
            { STATE_CHANGE_CB, ListenerType::STATE_CHANGE_CB },
            { UPDATE_TYPE_CB, ListenerType::UPDATE_TYPE_CB },
    };
}

JsPipManager::~JsPipManager()
{
}

void JsPipManager::Finalizer(napi_env env, void* data, void* hint)
{
    TLOGD(WmsLogTag::WMS_PIP, "[NAPI]");
    std::unique_ptr<JsPipManager>(static_cast<JsPipManager*>(data));
}

napi_value JsPipManager::InitXComponentController(napi_env env, napi_callback_info info)
{
    JsPipManager* me = CheckParamsAndGetThis<JsPipManager>(env, info);
    return (me != nullptr) ? me->OnInitXComponentController(env, info) : nullptr;
}

napi_value JsPipManager::OnInitXComponentController(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::WMS_PIP, "[NAPI]");
    size_t argc = 4;
    napi_value argv[4] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc < NUMBER_ONE) {
        TLOGE(WmsLogTag::WMS_PIP, "Argc count is invalid: %{public}zu", argc);
        return NapiThrowInvalidParam(env);
    }
    napi_value xComponentController = argv[0];
    std::shared_ptr<XComponentController> xComponentControllerResult =
        XComponentController::GetXComponentControllerFromNapiValue(env, xComponentController);
    sptr<Window> pipWindow = Window::Find(PIP_WINDOW_NAME);
    if (pipWindow == nullptr) {
        TLOGE(WmsLogTag::WMS_PIP, "Failed to find pip window");
        return NapiGetUndefined(env);
    }
    int32_t windowId = static_cast<int32_t>(pipWindow->GetWindowId());
    sptr<PictureInPictureControllerBase> pipController = PictureInPictureManager::GetPipControllerInfo(windowId);
    if (pipController == nullptr) {
        TLOGE(WmsLogTag::WMS_PIP, "Failed to get pictureInPictureController");
        return NapiGetUndefined(env);
    }
    TLOGI(WmsLogTag::WMS_PIP, "set xComponentController to window: %{public}u", windowId);
    WMError errCode = pipController->SetXComponentController(xComponentControllerResult);
    if (errCode != WMError::WM_OK) {
        TLOGE(WmsLogTag::WMS_PIP, "Failed to set xComponentController");
    }
    return NapiGetUndefined(env);
}

napi_value JsPipManager::InitWebXComponentController(napi_env env, napi_callback_info info)
{
    JsPipManager* me = CheckParamsAndGetThis<JsPipManager>(env, info);
    return (me != nullptr) ? me->OnInitWebXComponentController(env, info) : nullptr;
}

napi_value JsPipManager::OnInitWebXComponentController(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::WMS_PIP, "[NAPI]");
    size_t argc = 4;
    napi_value argv[4] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc != NUMBER_TWO) {
        TLOGE(WmsLogTag::WMS_PIP, "Argc count is invalid: %{public}zu", argc);
        return NapiThrowInvalidParam(env);
    }
    napi_value xComponentController = argv[0];
    std::shared_ptr<XComponentController> xComponentControllerResult =
        XComponentController::GetXComponentControllerFromNapiValue(env, xComponentController);
    sptr<Window> pipWindow = Window::Find(PIP_WINDOW_NAME);
    if (pipWindow == nullptr) {
        TLOGE(WmsLogTag::WMS_PIP, "Failed to find pip window");
        return NapiGetUndefined(env);
    }
    int32_t windowId = static_cast<int32_t>(pipWindow->GetWindowId());
    sptr<PictureInPictureControllerBase> pipController = PictureInPictureManager::GetPipControllerInfo(windowId);
    if (pipController == nullptr) {
        TLOGE(WmsLogTag::WMS_PIP, "Failed to get webPictureInPictureController");
        return NapiGetUndefined(env);
    }
    TLOGI(WmsLogTag::WMS_PIP, "set xComponentController to window: %{public}u", windowId);
    WMError errCode = pipController->SetXComponentController(xComponentControllerResult);
    if (errCode != WMError::WM_OK) {
        TLOGE(WmsLogTag::WMS_PIP, "Failed to set xComponentController");
        return NapiGetUndefined(env);
    }
    napi_value surfaceIdNapiValue = argv[1];
    return GetSurfaceIdFromJs(env, surfaceIdNapiValue, pipController);
}

napi_value JsPipManager::GetCustomUIController(napi_env env, napi_callback_info info)
{
    JsPipManager* me = CheckParamsAndGetThis<JsPipManager>(env, info);
    return (me != nullptr) ? me->OnGetCustomUIController(env, info) : nullptr;
}

napi_value JsPipManager::OnGetCustomUIController(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::WMS_PIP, "[NAPI]");
    sptr<Window> pipWindow = Window::Find(PIP_WINDOW_NAME);
    if (pipWindow == nullptr) {
        TLOGE(WmsLogTag::WMS_PIP, "Failed to find pip window");
        return NapiGetUndefined(env);
    }
    int32_t windowId = static_cast<int32_t>(pipWindow->GetWindowId());
    TLOGI(WmsLogTag::WMS_PIP, "winId: %{public}u", windowId);
    sptr<PictureInPictureControllerBase> pipController =
        PictureInPictureManager::GetPipControllerInfo(windowId);
    if (pipController == nullptr) {
        TLOGE(WmsLogTag::WMS_PIP, "Failed to get pictureInPictureController");
        return NapiGetUndefined(env);
    }
    napi_ref ref = pipController->GetCustomNodeController();
    if (ref == nullptr) {
        TLOGI(WmsLogTag::WMS_PIP, "invalid custom UI controller");
        return NapiGetUndefined(env);
    }
    napi_value uiController = nullptr;
    napi_get_reference_value(env, ref, &uiController);
    return uiController;
}

napi_value JsPipManager::GetTypeNode(napi_env env, napi_callback_info info)
{
    JsPipManager* me = CheckParamsAndGetThis<JsPipManager>(env, info);
    return (me != nullptr) ? me->OnGetTypeNode(env, info) : nullptr;
}

napi_value JsPipManager::OnGetTypeNode(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::WMS_PIP, "[NAPI]");
    sptr<Window> pipWindow = Window::Find(PIP_WINDOW_NAME);
    if (pipWindow == nullptr) {
        TLOGE(WmsLogTag::WMS_PIP, "Failed to find pip window");
        return NapiGetUndefined(env);
    }
    int32_t windowId = static_cast<int32_t>(pipWindow->GetWindowId());
    TLOGI(WmsLogTag::WMS_PIP, "winId: %{public}u", windowId);
    sptr<PictureInPictureControllerBase> pipController = PictureInPictureManager::GetPipControllerInfo(windowId);
    if (pipController == nullptr) {
        TLOGE(WmsLogTag::WMS_PIP, "Failed to get pictureInPictureController");
        return NapiGetUndefined(env);
    }
    napi_ref ref = pipController->GetTypeNode();
    if (ref == nullptr) {
        TLOGI(WmsLogTag::WMS_PIP, "invalid typeNode");
        return NapiGetUndefined(env);
    }
    napi_value typeNode = nullptr;
    napi_get_reference_value(env, ref, &typeNode);
    return typeNode;
}

napi_value JsPipManager::SetTypeNodeEnabled(napi_env env, napi_callback_info info)
{
    JsPipManager* me = CheckParamsAndGetThis<JsPipManager>(env, info);
    return (me != nullptr) ? me->OnSetTypeNodeEnabled(env, info) : nullptr;
}

napi_value JsPipManager::OnSetTypeNodeEnabled(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::WMS_PIP, "[NAPI]");
    sptr<Window> pipWindow = Window::Find(PIP_WINDOW_NAME);
    if (pipWindow == nullptr) {
        TLOGE(WmsLogTag::WMS_PIP, "Failed to find pip window");
        return NapiGetUndefined(env);
    }
    int32_t windowId = static_cast<int32_t>(pipWindow->GetWindowId());
    TLOGI(WmsLogTag::WMS_PIP, "winId: %{public}u", windowId);
    sptr<PictureInPictureControllerBase> pipController = PictureInPictureManager::GetPipControllerInfo(windowId);
    if (pipController == nullptr) {
        TLOGE(WmsLogTag::WMS_PIP, "Failed to get pictureInPictureController");
        return NapiGetUndefined(env);
    }
    pipController->OnPictureInPictureStart();
    return NapiGetUndefined(env);
}

napi_value JsPipManager::SetPipNodeType(napi_env env, napi_callback_info info)
{
    JsPipManager* me = CheckParamsAndGetThis<JsPipManager>(env, info);
    return (me != nullptr) ? me->OnSetPipNodeType(env, info) : nullptr;
}

napi_value JsPipManager::OnSetPipNodeType(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::WMS_PIP, "[NAPI]");
    size_t argc = NUMBER_FOUR;
    napi_value argv[NUMBER_FOUR] = { nullptr };
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc != NUMBER_TWO) {
        TLOGE(WmsLogTag::WMS_PIP, "Argc count is invalid:%{public}zu", argc);
        return NapiGetUndefined(env);
    }
    napi_value typeNode = argv[0];
    bool markPip = false;
    if (!ConvertFromJsValue(env, argv[NUMBER_ONE], markPip)) {
        TLOGW(WmsLogTag::WMS_PIP, "Failed to convert param to bool");
        return NapiGetUndefined(env);
    }
    if (typeNode != nullptr && GetType(env, typeNode) != napi_undefined) {
        XComponentControllerErrorCode ret =
            XComponentController::SetSurfaceCallbackMode(env, typeNode, markPip ?
            SurfaceCallbackMode::PIP: SurfaceCallbackMode::DEFAULT);
        TLOGI(WmsLogTag::WMS_PIP, "set surface mode, ret:%{public}u, isPip:%{public}d",
            static_cast<uint32_t>(ret), static_cast<uint32_t>(markPip));
    }
    return NapiGetUndefined(env);
}

napi_value JsPipManager::RegisterCallback(napi_env env, napi_callback_info info)
{
    JsPipManager* me = CheckParamsAndGetThis<JsPipManager>(env, info);
    return (me != nullptr) ? me->OnRegisterCallback(env, info) : nullptr;
}

bool NapiIsCallable(napi_env env, napi_value value)
{
    bool result = false;
    napi_is_callable(env, value, &result);
    return result;
}

napi_value JsPipManager::OnRegisterCallback(napi_env env, napi_callback_info info)
{
    TLOGI(WmsLogTag::WMS_PIP, "[NAPI]");
    size_t argc = NUMBER_FOUR;
    napi_value argv[NUMBER_FOUR] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc < NUMBER_TWO) {
        TLOGE(WmsLogTag::WMS_PIP, "Params count not match: %{public}zu", argc);
        return NapiThrowInvalidParam(env);
    }
    std::string cbType;
    if (!ConvertFromJsValue(env, argv[0], cbType)) {
        TLOGE(WmsLogTag::WMS_PIP, "Failed to convert param to cbType");
        return NapiThrowInvalidParam(env);
    }
    napi_value value = argv[1];
    if (value == nullptr || !NapiIsCallable(env, value)) {
        TLOGE(WmsLogTag::WMS_PIP, "Callback is null or not callable");
        return NapiThrowInvalidParam(env);
    }
    WmErrorCode errCode = RegisterListenerWithType(env, cbType, value);
    if (errCode != WmErrorCode::WM_OK) {
        TLOGE(WmsLogTag::WMS_PIP, "Failed to registerCallback");
        return NapiGetUndefined(env);
    }
    TLOGI(WmsLogTag::WMS_PIP, "Register type %{public}s success!", cbType.c_str());
    return NapiGetUndefined(env);
}

WmErrorCode JsPipManager::RegisterListenerWithType(napi_env env, const std::string& type, napi_value value)
{
    if (IfCallbackRegistered(env, type, value)) {
        TLOGE(WmsLogTag::WMS_PIP, "Callback already registered!");
        return WmErrorCode::WM_ERROR_INVALID_CALLING;
    }
    std::shared_ptr<NativeReference> callbackRef;
    napi_ref result = nullptr;
    napi_create_reference(env, value, 1, &result);
    callbackRef.reset(reinterpret_cast<NativeReference*>(result));
    auto pipWindowListener = sptr<JsPiPWindowListener>::MakeSptr(env, callbackRef);
    if (pipWindowListener == nullptr) {
        TLOGE(WmsLogTag::WMS_PIP, "New JsPiPWindowListener failed");
        return WmErrorCode::WM_ERROR_STATE_ABNORMALLY;
    }
    WmErrorCode ret = RegisterListener(type, pipWindowListener);
    if (ret != WmErrorCode::WM_OK) {
        TLOGE(WmsLogTag::WMS_PIP, "register type %{public}s failed", type.c_str());
        return ret;
    }
    TLOGI(WmsLogTag::WMS_PIP, "Register type %{public}s success! callback map size: %{public}zu",
          type.c_str(), jsCbMap_[type].size());
    return WmErrorCode::WM_OK;
}

WmErrorCode JsPipManager::RegisterListener(const std::string& type,
                                           const sptr <JsPiPWindowListener>& pipWindowListener)
{
    sptr<Window> pipWindow = Window::Find(PIP_WINDOW_NAME);
    if (pipWindow == nullptr) {
        TLOGE(WmsLogTag::WMS_PIP, "Failed to find pip window");
        return WmErrorCode::WM_ERROR_STATE_ABNORMALLY;
    }
    int32_t windowId = static_cast<int32_t>(pipWindow->GetWindowId());
    sptr<PictureInPictureControllerBase> pipController = PictureInPictureManager::GetPipControllerInfo(windowId);
    if (pipController == nullptr) {
        TLOGE(WmsLogTag::WMS_PIP, "Failed to get pictureInPictureController");
        return WmErrorCode::WM_ERROR_STATE_ABNORMALLY;
    }
    jsCbMap_[type].insert(pipWindowListener);
    switch (listenerCodeMap_[type]) {
        case ListenerType::STATE_CHANGE_CB: {
            sptr <IPiPLifeCycle> lifeCycleListener(pipWindowListener);
            pipController->RegisterPiPLifecycle(lifeCycleListener);
            break;
        }
        case ListenerType::UPDATE_TYPE_CB: {
            sptr <IPiPTypeNodeObserver> typeNodeChangeObserver(pipWindowListener);
            pipController->RegisterPiPTypeNodeChange(typeNodeChangeObserver);
            break;
        }
        default:
            TLOGE(WmsLogTag::WMS_PIP, "Failed to match ListenerType");
            return WmErrorCode::WM_ERROR_INVALID_PARAM;
    }
    return WmErrorCode::WM_OK;
}

bool JsPipManager::IfCallbackRegistered(napi_env env, const std::string& type, napi_value jsListenerObject)
{
    if (jsCbMap_.empty() || jsCbMap_.find(type) == jsCbMap_.end()) {
        TLOGI(WmsLogTag::WMS_PIP, "methodName %{public}s not registered!", type.c_str());
        return false;
    }
    for (auto& listener : jsCbMap_[type]) {
        bool isEquals = false;
        napi_strict_equals(env, jsListenerObject, listener->GetCallbackRef()->GetNapiValue(), &isEquals);
        if (isEquals) {
            TLOGE(WmsLogTag::WMS_PIP, "Callback already registered!");
            return true;
        }
    }
    return false;
}

napi_value JsPipManager::UnregisterCallback(napi_env env, napi_callback_info info)
{
    JsPipManager* me = CheckParamsAndGetThis<JsPipManager>(env, info);
    return (me != nullptr) ? me->OnUnregisterCallback(env, info) : nullptr;
}

napi_value JsPipManager::OnUnregisterCallback(napi_env env, napi_callback_info info)
{
    TLOGI(WmsLogTag::WMS_PIP, "[NAPI]");
    size_t argc = NUMBER_FOUR;
    napi_value argv[NUMBER_FOUR] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc != NUMBER_TWO) {
        TLOGE(WmsLogTag::WMS_PIP, "Params count not match:%{public}zu", argc);
        return NapiThrowInvalidParam(env);
    }
    std::string cbType;
    if (!ConvertFromJsValue(env, argv[0], cbType)) {
        TLOGE(WmsLogTag::WMS_PIP, "Failed to convert param to cbType");
        return NapiThrowInvalidParam(env);
    }
    napi_value value = argv[NUMBER_ONE];
    if (value == nullptr || !NapiIsCallable(env, value)) {
        TLOGE(WmsLogTag::WMS_PIP, "Callback is null or not callable");
        return NapiThrowInvalidParam(env);
    }
    WmErrorCode errCode = UnRegisterListenerWithType(env, cbType, value);
    if (errCode != WmErrorCode::WM_OK) {
        TLOGE(WmsLogTag::WMS_PIP, "Failed to set UnRegisterPipContentListenerWithType");
        return NapiGetUndefined(env);
    }
    TLOGI(WmsLogTag::WMS_PIP, "unregister type:%{public}s success!", cbType.c_str());
    return NapiGetUndefined(env);
}

WmErrorCode JsPipManager::UnRegisterListenerWithType(napi_env env, const std::string& type, napi_value value)
{
    if (jsCbMap_.empty() || jsCbMap_.find(type) == jsCbMap_.end()) {
        TLOGI(WmsLogTag::WMS_PIP, "methodName %{public}s not registered!", type.c_str());
        return WmErrorCode::WM_ERROR_INVALID_CALLING;
    }
    bool foundCallbackValue = false;
    for (auto& listener : jsCbMap_[type]) {
        bool isEquals = false;
        napi_strict_equals(env, value, listener->GetCallbackRef()->GetNapiValue(), &isEquals);
        if (!isEquals) {
            continue;
        }
        foundCallbackValue = true;
        WmErrorCode ret = UnRegisterListener(type, listener);
        if (ret != WmErrorCode::WM_OK) {
            TLOGE(WmsLogTag::WMS_PIP, "Unregister type %{public}s failed", type.c_str());
            return ret;
        }
        jsCbMap_[type].erase(listener);
        break;
    }
    if (!foundCallbackValue) {
        TLOGE(WmsLogTag::WMS_PIP, "Unregister type %{public}s failed because not found callback!", type.c_str());
        return WmErrorCode::WM_OK;
    }
    if (jsCbMap_[type].empty()) {
        jsCbMap_.erase(type);
    }
    TLOGI(WmsLogTag::WMS_PIP, "Unregister type %{public}s success!", type.c_str());
    return WmErrorCode::WM_OK;
}

WmErrorCode JsPipManager::UnRegisterListener(const std::string& type,
                                             const sptr<JsPiPWindowListener>& pipWindowListener)
{
    sptr<Window> pipWindow = Window::Find(PIP_WINDOW_NAME);
    if (pipWindow == nullptr) {
        TLOGE(WmsLogTag::WMS_PIP, "Failed to find pip window");
        return WmErrorCode::WM_ERROR_STATE_ABNORMALLY;
    }
    int32_t windowId = static_cast<int32_t>(pipWindow->GetWindowId());
    sptr<PictureInPictureControllerBase> pipController = PictureInPictureManager::GetPipControllerInfo(windowId);
    if (pipController == nullptr) {
        TLOGE(WmsLogTag::WMS_PIP, "Failed to get pictureInPictureController");
        return WmErrorCode::WM_ERROR_STATE_ABNORMALLY;
    }
    switch (listenerCodeMap_[type]) {
        case ListenerType::STATE_CHANGE_CB: {
            sptr <IPiPLifeCycle> lifeCycleListener(pipWindowListener);
            pipController->UnregisterPiPLifecycle(lifeCycleListener);
            break;
        }
        case ListenerType::UPDATE_TYPE_CB: {
            sptr<IPiPTypeNodeObserver> typeNodeChangeObserver(pipWindowListener);
            pipController->UnRegisterPiPTypeNodeChange(typeNodeChangeObserver);
            break;
        }
        default:
            TLOGE(WmsLogTag::WMS_PIP, "Failed to match ListenerType");
            return WmErrorCode::WM_ERROR_INVALID_PARAM;
    }
    return WmErrorCode::WM_OK;
}

napi_value JsPipManagerInit(napi_env env, napi_value exportObj)
{
    TLOGD(WmsLogTag::WMS_PIP, "[NAPI]");
    if (env == nullptr || exportObj == nullptr) {
        TLOGE(WmsLogTag::WMS_PIP, "JsPipManagerInit failed, env or exportObj is null");
        return nullptr;
    }
    std::unique_ptr<JsPipManager> jsPipManager = std::make_unique<JsPipManager>();
    napi_wrap(env, exportObj, jsPipManager.release(), JsPipManager::Finalizer, nullptr, nullptr);
    const char* moduleName = "JsPipManager";
    BindNativeFunction(env, exportObj, "initXComponentController", moduleName, JsPipManager::InitXComponentController);
    BindNativeFunction(env, exportObj, "initWebXComponentController", moduleName,
        JsPipManager::InitWebXComponentController);
    BindNativeFunction(env, exportObj, "getCustomUIController", moduleName, JsPipManager::GetCustomUIController);
    BindNativeFunction(env, exportObj, "getTypeNode", moduleName, JsPipManager::GetTypeNode);
    BindNativeFunction(env, exportObj, "on", moduleName, JsPipManager::RegisterCallback);
    BindNativeFunction(env, exportObj, "off", moduleName, JsPipManager::UnregisterCallback);
    BindNativeFunction(env, exportObj, "setTypeNodeEnabled", moduleName, JsPipManager::SetTypeNodeEnabled);
    BindNativeFunction(env, exportObj, "setPipNodeType", moduleName, JsPipManager::SetPipNodeType);
    return NapiGetUndefined(env);
}
} // namespace Rosen
} // namespace OHOS

