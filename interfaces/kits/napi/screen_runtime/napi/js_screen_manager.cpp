/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#include "js_screen_manager.h"

#include <cinttypes>
#include <hitrace_meter.h>
#include <vector>
#include <new>
#include "js_runtime_utils.h"
#include "js_screen.h"
#include "js_screen_listener.h"

#include "native_engine/native_reference.h"
#include "screen_manager.h"
#include "singleton_container.h"
#include "surface_utils.h"
#include "window_manager_hilog.h"
#include "pixel_map_napi.h"

namespace OHOS {
namespace Rosen {
using namespace AbilityRuntime;
constexpr size_t ARGC_ONE = 1;
constexpr size_t ARGC_TWO = 2;
constexpr size_t ARGC_THREE = 3;
constexpr size_t ARGC_FOUR = 4;
constexpr int32_t INDEX_ZERO = 0;
constexpr int32_t INDEX_ONE = 1;
constexpr int32_t INDEX_TWO = 2;
constexpr uint32_t MAX_SCREENS_NUM = 1000;

class JsScreenManager {
public:
explicit JsScreenManager(napi_env env) {
}

~JsScreenManager() = default;

static void Finalizer(napi_env env, void* data, void* hint)
{
    TLOGI(WmsLogTag::DMS, "called");
    std::unique_ptr<JsScreenManager>(static_cast<JsScreenManager*>(data));
}

static napi_value GetAllScreens(napi_env env, napi_callback_info info)
{
    JsScreenManager* me = CheckParamsAndGetThis<JsScreenManager>(env, info);
    return (me != nullptr) ? me->OnGetAllScreens(env, info) : nullptr;
}

static napi_value RegisterScreenManagerCallback(napi_env env, napi_callback_info info)
{
    JsScreenManager* me = CheckParamsAndGetThis<JsScreenManager>(env, info);
    return (me != nullptr) ? me->OnRegisterScreenManagerCallback(env, info) : nullptr;
}

static napi_value UnregisterScreenMangerCallback(napi_env env, napi_callback_info info)
{
    JsScreenManager* me = CheckParamsAndGetThis<JsScreenManager>(env, info);
    return (me != nullptr) ? me->OnUnregisterScreenManagerCallback(env, info) : nullptr;
}

static napi_value MakeMirror(napi_env env, napi_callback_info info)
{
    JsScreenManager* me = CheckParamsAndGetThis<JsScreenManager>(env, info);
    return (me != nullptr) ? me->OnMakeMirror(env, info) : nullptr;
}

static napi_value MakeMirrorWithRegion(napi_env env, napi_callback_info info)
{
    JsScreenManager* me = CheckParamsAndGetThis<JsScreenManager>(env, info);
    return (me != nullptr) ? me->OnMakeMirrorWithRegion(env, info) : nullptr;
}

static napi_value SetMultiScreenMode(napi_env env, napi_callback_info info)
{
    JsScreenManager* me = CheckParamsAndGetThis<JsScreenManager>(env, info);
    return (me != nullptr) ? me->OnSetMultiScreenMode(env, info) : nullptr;
}

static napi_value SetMultiScreenRelativePosition(napi_env env, napi_callback_info info)
{
    JsScreenManager* me = CheckParamsAndGetThis<JsScreenManager>(env, info);
    return (me != nullptr) ? me->OnSetMultiScreenRelativePosition(env, info) : nullptr;
}

static napi_value MakeExpand(napi_env env, napi_callback_info info)
{
    JsScreenManager* me = CheckParamsAndGetThis<JsScreenManager>(env, info);
    return (me != nullptr) ? me->OnMakeExpand(env, info) : nullptr;
}

static napi_value StopMirror(napi_env env, napi_callback_info info)
{
    JsScreenManager* me = CheckParamsAndGetThis<JsScreenManager>(env, info);
    return (me != nullptr) ? me->OnStopMirror(env, info) : nullptr;
}

static napi_value StopExpand(napi_env env, napi_callback_info info)
{
    JsScreenManager* me = CheckParamsAndGetThis<JsScreenManager>(env, info);
    return (me != nullptr) ? me->OnStopExpand(env, info) : nullptr;
}

static napi_value CreateVirtualScreen(napi_env env, napi_callback_info info)
{
    JsScreenManager* me = CheckParamsAndGetThis<JsScreenManager>(env, info);
    return (me != nullptr) ? me->OnCreateVirtualScreen(env, info) : nullptr;
}

static napi_value DestroyVirtualScreen(napi_env env, napi_callback_info info)
{
    JsScreenManager* me = CheckParamsAndGetThis<JsScreenManager>(env, info);
    return (me != nullptr) ? me->OnDestroyVirtualScreen(env, info) : nullptr;
}

static napi_value SetVirtualScreenSurface(napi_env env, napi_callback_info info)
{
    JsScreenManager* me = CheckParamsAndGetThis<JsScreenManager>(env, info);
    return (me != nullptr) ? me->OnSetVirtualScreenSurface(env, info) : nullptr;
}

static napi_value SetScreenPrivacyMaskImage(napi_env env, napi_callback_info info)
{
    JsScreenManager* me = CheckParamsAndGetThis<JsScreenManager>(env, info);
    return (me != nullptr) ? me->OnSetScreenPrivacyMaskImage(env, info) : nullptr;
}

static napi_value IsScreenRotationLocked(napi_env env, napi_callback_info info)
{
    JsScreenManager* me = CheckParamsAndGetThis<JsScreenManager>(env, info);
    return (me != nullptr) ? me->OnIsScreenRotationLocked(env, info) : nullptr;
}

static napi_value SetScreenRotationLocked(napi_env env, napi_callback_info info)
{
    JsScreenManager* me = CheckParamsAndGetThis<JsScreenManager>(env, info);
    return (me != nullptr) ? me->OnSetScreenRotationLocked(env, info) : nullptr;
}

static napi_value MakeUnique(napi_env env, napi_callback_info info)
{
    JsScreenManager* me = CheckParamsAndGetThis<JsScreenManager>(env, info);
    return (me != nullptr) ? me->OnMakeUnique(env, info) : nullptr;
}

private:
std::map<std::string, std::map<std::unique_ptr<NativeReference>, sptr<JsScreenListener>>> jsCbMap_;
std::mutex mtx_;

napi_value OnGetAllScreens(napi_env env, napi_callback_info info)
{
    TLOGI(WmsLogTag::DMS, "OnGetAllScreens is called");
    size_t argc = 4;
    napi_value argv[4] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    napi_value lastParam = nullptr;
    if (argc >= ARGC_ONE && argv[ARGC_ONE - 1] != nullptr &&
        GetType(env, argv[ARGC_ONE - 1]) == napi_function) {
        lastParam = argv[ARGC_ONE - 1];
    }
    napi_value result = nullptr;
    std::unique_ptr<NapiAsyncTask> napiAsyncTask = CreateEmptyAsyncTask(env, lastParam, &result);
    auto asyncTask = [this, env, task = napiAsyncTask.get()]() {
        HITRACE_METER_FMT(HITRACE_TAG_WINDOW_MANAGER, "JsScreenManager::OnGetAllScreens");
        std::vector<sptr<Screen>> screens;
        auto res = DM_JS_TO_ERROR_CODE_MAP.at(SingletonContainer::Get<ScreenManager>().GetAllScreens(screens));
        if (res != DmErrorCode::DM_OK) {
            task->Reject(env, CreateJsError(env, static_cast<int32_t>(res),
                "JsScreenManager::OnGetAllScreens failed."));
        } else if (!screens.empty()) {
            task->Resolve(env, CreateJsScreenVectorObject(env, screens));
            TLOGNI(WmsLogTag::DMS, "JsScreenManager::OnGetAllScreens success");
        } else {
            task->Reject(env, CreateJsError(env,
                static_cast<int32_t>(DmErrorCode::DM_ERROR_INVALID_SCREEN),
                "JsScreenManager::OnGetAllScreens failed."));
        }
        delete task;
    };
    NapiSendDmsEvent(env, asyncTask, napiAsyncTask, "OnGetAllScreens");
    return result;
}

napi_value CreateJsDisplayIdVectorObject(napi_env env, std::vector<DisplayId>& displayIds)
{
    napi_value arrayValue = nullptr;
    napi_create_array_with_length(env, displayIds.size(), &arrayValue);
    if (arrayValue == nullptr) {
        TLOGE(WmsLogTag::DMS, "Failed to get screens");
        return NapiGetUndefined(env);
    }
    size_t i = 0;
    for (auto& displayId : displayIds) {
        napi_set_element(env, arrayValue, i++, CreateJsValue(env, static_cast<uint32_t>(displayId)));
    }
    return arrayValue;
}

napi_value CreateJsScreenVectorObject(napi_env env, std::vector<sptr<Screen>>& screens)
{
    napi_value arrayValue = nullptr;
    napi_create_array_with_length(env, screens.size(), &arrayValue);
    if (arrayValue == nullptr) {
        TLOGE(WmsLogTag::DMS, "Failed to get screens");
        return NapiGetUndefined(env);
    }
    size_t i = 0;
    for (auto& screen : screens) {
        if (screen == nullptr) {
            continue;
        }
        napi_set_element(env, arrayValue, i++, CreateJsScreenObject(env, screen));
    }
    return arrayValue;
}

bool IfCallbackRegistered(napi_env env, const std::string& type, napi_value jsListenerObject)
{
    if (jsCbMap_.empty() || jsCbMap_.find(type) == jsCbMap_.end()) {
        TLOGI(WmsLogTag::DMS, "methodName %{public}s not registered!", type.c_str());
        return false;
    }

    for (auto& iter : jsCbMap_[type]) {
        bool isEquals = false;
        napi_strict_equals(env, jsListenerObject, iter.first->GetNapiValue(), &isEquals);
        if (isEquals) {
            TLOGE(WmsLogTag::DMS, "callback already registered!");
            return true;
        }
    }
    return false;
}

DmErrorCode RegisterScreenListenerWithType(napi_env env, const std::string& type, napi_value value)
{
    if (IfCallbackRegistered(env, type, value)) {
        TLOGE(WmsLogTag::DMS, "callback already registered!");
        return DmErrorCode::DM_ERROR_INVALID_CALLING;
    }
    std::unique_ptr<NativeReference> callbackRef;
    napi_ref result = nullptr;
    napi_create_reference(env, value, 1, &result);
    callbackRef.reset(reinterpret_cast<NativeReference*>(result));
    sptr<JsScreenListener> screenListener = new(std::nothrow) JsScreenListener(env);
    if (screenListener == nullptr) {
        TLOGE(WmsLogTag::DMS, "screenListener is nullptr");
        return DmErrorCode::DM_ERROR_INVALID_SCREEN;
    }
    if (type == EVENT_CONNECT || type == EVENT_DISCONNECT || type == EVENT_CHANGE) {
        DmErrorCode ret = DM_JS_TO_ERROR_CODE_MAP.at(
            SingletonContainer::Get<ScreenManager>().RegisterScreenListener(screenListener));
        if (ret != DmErrorCode::DM_OK) {
            return ret;
        }
        TLOGI(WmsLogTag::DMS, "success");
    } else {
        TLOGE(WmsLogTag::DMS, "failed method: %{public}s not support!",
            type.c_str());
        return DmErrorCode::DM_ERROR_INVALID_CALLING;
    }
    screenListener->AddCallback(type, value);
    jsCbMap_[type][std::move(callbackRef)] = screenListener;
    return DmErrorCode::DM_OK;
}

DmErrorCode UnregisterAllScreenListenerWithType(const std::string& type)
{
    if (jsCbMap_.empty() || jsCbMap_.find(type) == jsCbMap_.end()) {
        TLOGE(WmsLogTag::DMS, "methodName %{public}s not registered!",
            type.c_str());
        return DmErrorCode::DM_OK;
    }
    for (auto it = jsCbMap_[type].begin(); it != jsCbMap_[type].end();) {
        it->second->RemoveAllCallback();
        if (type == EVENT_CONNECT || type == EVENT_DISCONNECT || type == EVENT_CHANGE) {
            sptr<ScreenManager::IScreenListener> thisListener(it->second);
            SingletonContainer::Get<ScreenManager>().UnregisterScreenListener(thisListener);
            TLOGI(WmsLogTag::DMS, "success");
        }
        jsCbMap_[type].erase(it++);
    }
    jsCbMap_.erase(type);
    return DmErrorCode::DM_OK;
}

DmErrorCode UnRegisterScreenListenerWithType(napi_env env, const std::string& type, napi_value value)
{
    if (jsCbMap_.empty() || jsCbMap_.find(type) == jsCbMap_.end()) {
        TLOGI(WmsLogTag::DMS, "methodName %{public}s not registered!",
            type.c_str());
        return DmErrorCode::DM_OK;
    }
    if (value == nullptr) {
        TLOGE(WmsLogTag::DMS, "value is nullptr");
        return DmErrorCode::DM_ERROR_INVALID_SCREEN;
    }
    DmErrorCode ret = DmErrorCode::DM_OK;
    for (auto it = jsCbMap_[type].begin(); it != jsCbMap_[type].end();) {
        bool isEquals = false;
        napi_strict_equals(env, value, it->first->GetNapiValue(), &isEquals);
        if (isEquals) {
            it->second->RemoveCallback(env, type, value);
            if (type == EVENT_CONNECT || type == EVENT_DISCONNECT || type == EVENT_CHANGE) {
                sptr<ScreenManager::IScreenListener> thisListener(it->second);
                ret = DM_JS_TO_ERROR_CODE_MAP.at(
                    SingletonContainer::Get<ScreenManager>().UnregisterScreenListener(thisListener));
            }
            jsCbMap_[type].erase(it++);
            break;
        } else {
            it++;
        }
    }
    if (jsCbMap_[type].empty()) {
        jsCbMap_.erase(type);
    }
    if (ret == DmErrorCode::DM_OK) {
        TLOGI(WmsLogTag::DMS, "success");
    }
    return ret;
}

bool NapiIsCallable(napi_env env, napi_value value)
{
    bool result = false;
    napi_is_callable(env, value, &result);
    return result;
}

napi_value NapiThrowError(napi_env env, DmErrorCode errCode, std::string msg = "")
{
    napi_throw(env, CreateJsError(env, static_cast<int32_t>(errCode), msg));
    return NapiGetUndefined(env);
}

napi_value OnRegisterScreenManagerCallback(napi_env env, napi_callback_info info)
{
    TLOGI(WmsLogTag::DMS, "called");
    size_t argc = 4;
    napi_value argv[4] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc < ARGC_TWO) {
        TLOGE(WmsLogTag::DMS, "Invalid args count, need 2 args!");
        return NapiThrowError(env, DmErrorCode::DM_ERROR_INVALID_PARAM, "Invalid args count, 2 args is needed.");
    }
    std::string cbType;
    if (!ConvertFromJsValue(env, argv[0], cbType)) {
        TLOGE(WmsLogTag::DMS, "Failed to convert parameter to eventType");
        std::string errMsg = "Failed to convert parameter to eventType";
        return NapiThrowError(env, DmErrorCode::DM_ERROR_INVALID_PARAM, errMsg);
    }
    napi_value value = argv[INDEX_ONE];
    if (value == nullptr) {
        TLOGI(WmsLogTag::DMS, "argv[1] is nullptr");
        std::string errMsg = "OnRegisterScreenManagerCallback error, argv[1] is nullptr";
        return NapiThrowError(env, DmErrorCode::DM_ERROR_INVALID_PARAM, errMsg);
    }
    if (!NapiIsCallable(env, value)) {
        TLOGI(WmsLogTag::DMS, "argv[1] is not callable");
        std::string errMsg = "OnRegisterScreenManagerCallback error, argv[1] is not callable";
        return NapiThrowError(env, DmErrorCode::DM_ERROR_INVALID_PARAM, errMsg);
    }
    std::lock_guard<std::mutex> lock(mtx_);
    DmErrorCode ret = RegisterScreenListenerWithType(env, cbType, value);
    if (ret != DmErrorCode::DM_OK) {
        TLOGE(WmsLogTag::DMS, "failed");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(ret)));
    }
    return NapiGetUndefined(env);
}

napi_value OnUnregisterScreenManagerCallback(napi_env env, napi_callback_info info)
{
    TLOGI(WmsLogTag::DMS, "called");
    size_t argc = 4;
    napi_value argv[4] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc < ARGC_ONE) {
        TLOGE(WmsLogTag::DMS, "Invalid args count, need one arg at least!");
        return NapiThrowError(env, DmErrorCode::DM_ERROR_INVALID_PARAM, "Invalid args count, need one arg at least!");
    }
    std::string cbType;
    if (!ConvertFromJsValue(env, argv[0], cbType)) {
        TLOGE(WmsLogTag::DMS, "Failed to convert parameter to eventType");
        std::string errMsg = "Failed to convert parameter to eventType";
        return NapiThrowError(env, DmErrorCode::DM_ERROR_INVALID_PARAM, errMsg);
    }
    std::lock_guard<std::mutex> lock(mtx_);
    if (argc == ARGC_ONE) {
        DmErrorCode ret = UnregisterAllScreenListenerWithType(cbType);
        if (ret != DmErrorCode::DM_OK) {
            TLOGE(WmsLogTag::DMS, "unregister all callback failed");
            napi_throw(env, CreateJsError(env, static_cast<int32_t>(ret)));
        }
    } else {
        napi_value value = argv[INDEX_ONE];
        if ((value == nullptr) || (!NapiIsCallable(env, value))) {
            DmErrorCode ret = UnregisterAllScreenListenerWithType(cbType);
            if (ret != DmErrorCode::DM_OK) {
                TLOGE(WmsLogTag::DMS, "failed");
                napi_throw(env, CreateJsError(env, static_cast<int32_t>(ret)));
            }
        } else {
            DmErrorCode ret = UnRegisterScreenListenerWithType(env, cbType, value);
            if (ret != DmErrorCode::DM_OK) {
                TLOGE(WmsLogTag::DMS, "failed");
                napi_throw(env, CreateJsError(env, static_cast<int32_t>(ret)));
            }
        }
    }
    return NapiGetUndefined(env);
}

napi_value OnMakeMirror(napi_env env, napi_callback_info info)
{
    TLOGI(WmsLogTag::DMS, "called");
    size_t argc = 4;
    napi_value argv[4] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc < ARGC_TWO) {
        return NapiThrowError(env, DmErrorCode::DM_ERROR_INVALID_PARAM, "Invalid args count, need 2 args at least!");
    }

    int64_t mainScreenId;
    if (!ConvertFromJsValue(env, argv[0], mainScreenId)) {
        return NapiThrowError(env, DmErrorCode::DM_ERROR_INVALID_PARAM, "Failed to convert parameter to int");
    }
    napi_value array = argv[INDEX_ONE];
    if (array == nullptr) {
        return NapiThrowError(env, DmErrorCode::DM_ERROR_INVALID_PARAM, "Failed to get mirrorScreen, is nullptr");
    }
    uint32_t size = 0;
    napi_get_array_length(env, array, &size);
    std::vector<ScreenId> screenIds;
    for (uint32_t i = 0; i < size; i++) {
        uint32_t screenId;
        napi_value value = nullptr;
        napi_get_element(env, array, i, &value);
        if (!ConvertFromJsValue(env, value, screenId)) {
            return NapiThrowError(env, DmErrorCode::DM_ERROR_INVALID_PARAM, "Failed to convert parameter to ScreenId");
        }
        screenIds.emplace_back(static_cast<ScreenId>(screenId));
    }
    napi_value lastParam = nullptr;
    if (argc >= ARGC_THREE && argv[ARGC_THREE - 1] != nullptr &&
        GetType(env, argv[ARGC_THREE - 1]) == napi_function) {
        lastParam = argv[ARGC_THREE - 1];
    }
    napi_value result = nullptr;
    std::unique_ptr<NapiAsyncTask> napiAsyncTask = CreateEmptyAsyncTask(env, lastParam, &result);
    auto asyncTask = [mainScreenId, screenIds, env, task = napiAsyncTask.get()]() {
        HITRACE_METER_FMT(HITRACE_TAG_WINDOW_MANAGER, "JsScreenManager::OnMakeMirror");
        ScreenId screenGroupId = INVALID_SCREEN_ID;
        DmErrorCode ret = DM_JS_TO_ERROR_CODE_MAP.at(
            SingletonContainer::Get<ScreenManager>().MakeMirror(mainScreenId, screenIds, screenGroupId));
        if (ret == DmErrorCode::DM_OK) {
            task->Resolve(env, CreateJsValue(env, static_cast<uint32_t>(screenGroupId)));
        } else {
            task->Reject(env,
                CreateJsError(env, static_cast<int32_t>(ret), "JsScreenManager::OnMakeMirror failed."));
        }
    };
    NapiSendDmsEvent(env, asyncTask, napiAsyncTask, "OnMakeMirror");
    return result;
}

napi_value OnMakeMirrorWithRegion(napi_env env, napi_callback_info info)
{
    TLOGI(WmsLogTag::DMS, "called");
    size_t argc = 4;
    napi_value argv[4] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc < ARGC_THREE) {
        return NapiThrowError(env, DmErrorCode::DM_ERROR_INVALID_PARAM, "Invalid args count, need 3 args at least!");
    }
    int64_t mainScreenId;
    if (!ConvertFromJsValue(env, argv[INDEX_ZERO], mainScreenId)) {
        return NapiThrowError(env, DmErrorCode::DM_ERROR_INVALID_PARAM, "Failed to convert parameter to mainScreenId");
    }
    napi_value array = argv[INDEX_ONE];
    if (array == nullptr) {
        return NapiThrowError(env, DmErrorCode::DM_ERROR_INVALID_PARAM, "Failed to get mirrorScreen, is nullptr");
    }
    uint32_t size = 0;
    napi_get_array_length(env, array, &size);
    std::vector<ScreenId> mirrorScreenIds;
    for (uint32_t i = 0; i < size; i++) {
        uint32_t screenId;
        napi_value value = nullptr;
        napi_get_element(env, array, i, &value);
        if (!ConvertFromJsValue(env, value, screenId)) {
            return NapiThrowError(env, DmErrorCode::DM_ERROR_INVALID_PARAM, "Failed to convert parameter to ScreenId");
        }
        mirrorScreenIds.emplace_back(static_cast<ScreenId>(screenId));
    }
    DMRect mainScreenRegion;
    if (GetRectFromJs(env, argv[INDEX_TWO], mainScreenRegion) == -1) {
        return NapiThrowError(env, DmErrorCode::DM_ERROR_INVALID_PARAM, "Failed to convert to mainScreenRegion");
    }
    napi_value lastParam = nullptr;
    napi_value result = nullptr;
    std::unique_ptr<NapiAsyncTask> napiAsyncTask = CreateEmptyAsyncTask(env, lastParam, &result);
    auto asyncTask = [mainScreenId, mirrorScreenIds, mainScreenRegion, env, task = napiAsyncTask.get()]() {
        ScreenId screenGroupId = INVALID_SCREEN_ID;
        DmErrorCode ret = DM_JS_TO_ERROR_CODE_MAP.at(SingletonContainer::Get<ScreenManager>().MakeMirror(mainScreenId,
            mirrorScreenIds, mainScreenRegion, screenGroupId));
        if (ret == DmErrorCode::DM_OK) {
            task->Resolve(env, CreateJsValue(env, static_cast<uint32_t>(screenGroupId)));
        } else {
            task->Reject(env,
                CreateJsError(env, static_cast<int32_t>(ret), "JsScreenManager::OnMakeMirrorWithRegion failed."));
        }
    };
    NapiSendDmsEvent(env, asyncTask, napiAsyncTask, "OnMakeMirrorWithRegion");
    return result;
}

napi_value OnSetMultiScreenMode(napi_env env, napi_callback_info info)
{
    size_t argc = ARGC_FOUR;
    napi_value argv[ARGC_FOUR] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc < ARGC_THREE) {
        return NapiThrowError(env, DmErrorCode::DM_ERROR_INVALID_PARAM, "Invalid args count, need three args");
    }
    int64_t mainScreenId;
    if (!ConvertFromJsValue(env, argv[INDEX_ZERO], mainScreenId)) {
        return NapiThrowError(env, DmErrorCode::DM_ERROR_INVALID_PARAM, "Failed to convert parameter to int");
    }
    int64_t secondaryScreenId;
    if (!ConvertFromJsValue(env, argv[INDEX_ONE], secondaryScreenId)) {
        return NapiThrowError(env, DmErrorCode::DM_ERROR_INVALID_PARAM, "Failed to convert parameter to int");
    }
    if (mainScreenId < 0 || secondaryScreenId < 0) {
        return NapiThrowError(env, DmErrorCode::DM_ERROR_INVALID_PARAM, "ScreenId cannot be a negative number");
    }
    MultiScreenMode screenMode;
    if (!ConvertFromJsValue(env, argv[INDEX_TWO], screenMode)) {
        return NapiThrowError(env, DmErrorCode::DM_ERROR_INVALID_PARAM, "Failed to convert parameter");
    }
    napi_value lastParam = nullptr;
    napi_value result = nullptr;
    std::unique_ptr<NapiAsyncTask> napiAsyncTask = CreateEmptyAsyncTask(env, lastParam, &result);
    auto asyncTask = [mainScreenId, secondaryScreenId, env, screenMode,
        task = napiAsyncTask.get()]() {
        DmErrorCode ret = DM_JS_TO_ERROR_CODE_MAP.at(
            SingletonContainer::Get<ScreenManager>().SetMultiScreenMode(mainScreenId, secondaryScreenId,
                screenMode));
        if (ret == DmErrorCode::DM_OK) {
            task->Resolve(env, NapiGetUndefined(env));
        } else {
            task->Reject(env,
                CreateJsError(env, static_cast<int32_t>(ret), "JsScreenManager::OnSetMultiScreenMode failed."));
        }
        delete task;
    };
    NapiSendDmsEvent(env, asyncTask, napiAsyncTask, "OnSetMultiScreenMode");
    return result;
}

napi_value OnSetMultiScreenRelativePosition(napi_env env, napi_callback_info info)
{
    size_t argc = ARGC_FOUR;
    napi_value argv[ARGC_FOUR] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc < ARGC_TWO) {
        return NapiThrowError(env, DmErrorCode::DM_ERROR_INVALID_PARAM, "Invalid args count, need two args");
    }
    MultiScreenPositionOptions mainScreenOptions;
    if (GetMultiScreenPositionOptionsFromJs(env, argv[INDEX_ZERO], mainScreenOptions) == -1) {
        return NapiThrowError(env, DmErrorCode::DM_ERROR_INVALID_PARAM, "Failed to convert parameter to int");
    }
    MultiScreenPositionOptions secondScreenOption;
    if (GetMultiScreenPositionOptionsFromJs(env, argv[INDEX_ONE], secondScreenOption) == -1) {
        return NapiThrowError(env, DmErrorCode::DM_ERROR_INVALID_PARAM, "Failed to convert parameter to int");
    }
    napi_value lastParam = nullptr;
    napi_value result = nullptr;
    std::unique_ptr<NapiAsyncTask> napiAsyncTask = CreateEmptyAsyncTask(env, lastParam, &result);
    auto asyncTask = [mainScreenOptions, secondScreenOption, env,
        task = napiAsyncTask.get()]() {
        DmErrorCode ret = DM_JS_TO_ERROR_CODE_MAP.at(
            SingletonContainer::Get<ScreenManager>().SetMultiScreenRelativePosition(mainScreenOptions,
                secondScreenOption));
        if (ret == DmErrorCode::DM_OK) {
            task->Resolve(env, NapiGetUndefined(env));
        } else {
            task->Reject(env,
                CreateJsError(env, static_cast<int32_t>(ret), "OnSetMultiScreenRelativePosition failed."));
        }
        delete task;
    };
    NapiSendDmsEvent(env, asyncTask, napiAsyncTask, "OnSetMultiScreenRelativePosition");
    return result;
}

napi_value OnMakeExpand(napi_env env, napi_callback_info info)
{
    TLOGI(WmsLogTag::DMS, "called");
    size_t argc = 4;
    napi_value argv[4] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc < ARGC_ONE) {
        TLOGE(WmsLogTag::DMS, "Invalid args count, need one arg at least!");
        return NapiThrowError(env, DmErrorCode::DM_ERROR_INVALID_PARAM, "Invalid args count, need one arg at least!");
    }

    napi_value array = argv[0];
    if (array == nullptr) {
        TLOGE(WmsLogTag::DMS, "Failed to get options, options is nullptr");
        return NapiThrowError(env, DmErrorCode::DM_ERROR_INVALID_PARAM, "Failed to get options, options is nullptr");
    }
    uint32_t size = 0;
    napi_get_array_length(env, array, &size);
    std::vector<ExpandOption> options;
    for (uint32_t i = 0; i < size; ++i) {
        napi_value object = nullptr;
        napi_get_element(env, array, i, &object);
        ExpandOption expandOption;
        int32_t res = GetExpandOptionFromJs(env, object, expandOption);
        if (res == -1) {
            TLOGE(WmsLogTag::DMS, "expandoption param %{public}d error!", i);
            return NapiThrowError(env, DmErrorCode::DM_ERROR_INVALID_PARAM, "expandoption param error!");
        }
        options.emplace_back(expandOption);
    }
    napi_value lastParam = nullptr;
    if (argc >= ARGC_TWO && argv[ARGC_TWO - 1] != nullptr && GetType(env, argv[ARGC_TWO - 1]) == napi_function) {
        lastParam = argv[ARGC_TWO - 1];
    }
    napi_value result = nullptr;
    std::unique_ptr<NapiAsyncTask> napiAsyncTask = CreateEmptyAsyncTask(env, lastParam, &result);
    auto asyncTask = [options, env, task = napiAsyncTask.get()]() {
        HITRACE_METER_FMT(HITRACE_TAG_WINDOW_MANAGER, "JsScreenManager::OnMakeExpand");
        ScreenId screenGroupId = INVALID_SCREEN_ID;
        DmErrorCode ret = DM_JS_TO_ERROR_CODE_MAP.at(
            SingletonContainer::Get<ScreenManager>().MakeExpand(options, screenGroupId));
        if (ret == DmErrorCode::DM_OK) {
            task->Resolve(env, CreateJsValue(env, static_cast<uint32_t>(screenGroupId)));
            TLOGNI(WmsLogTag::DMS, "MakeExpand success");
        } else {
            task->Reject(env, CreateJsError(env, static_cast<int32_t>(ret), "JsScreenManager::OnMakeExpand failed."));
            TLOGNE(WmsLogTag::DMS, "MakeExpand failed");
        }
        delete task;
    };
    NapiSendDmsEvent(env, asyncTask, napiAsyncTask, "OnMakeExpand");
    return result;
}

napi_value OnStopMirror(napi_env env, napi_callback_info info)
{
    TLOGI(WmsLogTag::DMS, "called");
    size_t argc = 4;
    napi_value argv[4] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc < ARGC_ONE) {
        TLOGE(WmsLogTag::DMS, "Invalid args count, need one arg at least!");
        return NapiThrowError(env, DmErrorCode::DM_ERROR_INVALID_PARAM, "Invalid args count, need one arg at least!");
    }

    napi_value array = argv[0];
    if (array == nullptr) {
        TLOGE(WmsLogTag::DMS, "Failed to get mirrorScreen, mirrorScreen is nullptr");
        return NapiThrowError(env, DmErrorCode::DM_ERROR_INVALID_PARAM, "Failed to get mirrorScreen, is nullptr");
    }
    uint32_t size = 0;
    napi_get_array_length(env, array, &size);
    if (size > MAX_SCREENS_NUM) {
        return NapiThrowError(env, DmErrorCode::DM_ERROR_INVALID_PARAM, "size of mirrorScreen is greater than 1000");
    }
    std::vector<ScreenId> screenIds;
    for (uint32_t i = 0; i < size; i++) {
        uint32_t screenId;
        napi_value value = nullptr;
        napi_get_element(env, array, i, &value);
        if (!ConvertFromJsValue(env, value, screenId)) {
            return NapiThrowError(env, DmErrorCode::DM_ERROR_INVALID_PARAM, "Failed to convert parameter to ScreenId");
        }
        screenIds.emplace_back(static_cast<ScreenId>(screenId));
    }
    napi_value lastParam = nullptr;
    if (argc >= ARGC_TWO && argv[ARGC_TWO - 1] != nullptr && GetType(env, argv[ARGC_TWO - 1]) == napi_function) {
        lastParam = argv[ARGC_TWO - 1];
    }
    napi_value result = nullptr;
    std::unique_ptr<NapiAsyncTask> napiAsyncTask = CreateEmptyAsyncTask(env, lastParam, &result);
    auto asyncTask = [screenIds, env, task = napiAsyncTask.get()]() {
        HITRACE_METER_FMT(HITRACE_TAG_WINDOW_MANAGER, "JsScreenManager::OnStopMirror");
        DmErrorCode ret = DM_JS_TO_ERROR_CODE_MAP.at(
            SingletonContainer::Get<ScreenManager>().StopMirror(screenIds));
        if (ret == DmErrorCode::DM_OK) {
            task->Resolve(env, NapiGetUndefined(env));
        } else {
            task->Reject(env,
                CreateJsError(env, static_cast<int32_t>(ret), "JsScreenManager::OnStopMirror failed."));
        }
        delete task;
    };
    NapiSendDmsEvent(env, asyncTask, napiAsyncTask, "OnStopMirror");
    return result;
}

napi_value OnStopExpand(napi_env env, napi_callback_info info)
{
    TLOGI(WmsLogTag::DMS, "called");
    size_t argc = 4;
    napi_value argv[4] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc < ARGC_ONE) {
        TLOGE(WmsLogTag::DMS, "Invalid args count, need one arg at least!");
        return NapiThrowError(env, DmErrorCode::DM_ERROR_INVALID_PARAM, "Invalid args count, need one arg at least!");
    }

    napi_value array = argv[0];
    if (array == nullptr) {
        return NapiThrowError(env, DmErrorCode::DM_ERROR_INVALID_PARAM, "Failed to get expandScreen, is nullptr");
    }
    uint32_t size = 0;
    napi_get_array_length(env, array, &size);
    if (size > MAX_SCREENS_NUM) {
        return NapiThrowError(env, DmErrorCode::DM_ERROR_INVALID_PARAM, "size of expandScreen is greater than 1000");
    }
    std::vector<ScreenId> screenIds;
    for (uint32_t i = 0; i < size; i++) {
        uint32_t screenId;
        napi_value value = nullptr;
        napi_get_element(env, array, i, &value);
        if (!ConvertFromJsValue(env, value, screenId)) {
            return NapiThrowError(env, DmErrorCode::DM_ERROR_INVALID_PARAM, "Failed to convert parameter to ScreenId");
        }
        screenIds.emplace_back(static_cast<ScreenId>(screenId));
    }
    napi_value lastParam = nullptr;
    if (argc >= ARGC_TWO && argv[ARGC_TWO - 1] != nullptr && GetType(env, argv[ARGC_TWO - 1]) == napi_function) {
        lastParam = argv[ARGC_TWO - 1];
    }
    napi_value result = nullptr;
    std::unique_ptr<NapiAsyncTask> napiAsyncTask = CreateEmptyAsyncTask(env, lastParam, &result);
    auto asyncTask = [screenIds, env, task = napiAsyncTask.get()]() {
        HITRACE_METER_FMT(HITRACE_TAG_WINDOW_MANAGER, "JsScreenManager::OnStopExpand");
        DmErrorCode ret = DM_JS_TO_ERROR_CODE_MAP.at(
            SingletonContainer::Get<ScreenManager>().StopExpand(screenIds));
        if (ret == DmErrorCode::DM_OK) {
            task->Resolve(env, NapiGetUndefined(env));
            TLOGNI(WmsLogTag::DMS, "MakeExpand success");
        } else {
            task->Reject(env,
                CreateJsError(env, static_cast<int32_t>(ret), "JsScreenManager::OnStopExpand failed."));
        }
        delete task;
    };
    NapiSendDmsEvent(env, asyncTask, napiAsyncTask, "OnStopExpand");
    return result;
}

napi_value OnMakeUnique(napi_env env, napi_callback_info info)
{
    TLOGI(WmsLogTag::DMS, "OnMakeUnique is called");
    size_t argc = 4;
    napi_value argv[4] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc < ARGC_ONE) {
        TLOGE(WmsLogTag::DMS, "Invalid args count, need one arg at least!");
        return NapiThrowError(env, DmErrorCode::DM_ERROR_INVALID_PARAM, "Invalid args count, need one arg at least!");
    }
    napi_value array = argv[0];
    if (array == nullptr) {
        TLOGE(WmsLogTag::DMS, "Failed to get options, options is nullptr");
        return NapiThrowError(env, DmErrorCode::DM_ERROR_INVALID_PARAM, "Failed to get options, options is nullptr");
    }
    uint32_t size = 0;
    napi_get_array_length(env, array, &size);
    std::vector<ScreenId> screenIds;
    for (uint32_t i = 0; i < size; i++) {
        uint32_t screenId;
        napi_value value = nullptr;
        napi_get_element(env, array, i, &value);
        if (!ConvertFromJsValue(env, value, screenId)) {
            return NapiThrowError(env, DmErrorCode::DM_ERROR_INVALID_PARAM, "Failed to convert parameter to ScreenId");
        }
        if (static_cast<int32_t>(screenId) < 0) {
            return NapiThrowError(env, DmErrorCode::DM_ERROR_INVALID_PARAM, "ScreenId cannot be a negative number");
        }
        screenIds.emplace_back(static_cast<ScreenId>(screenId));
    }
    napi_value lastParam = nullptr;
    napi_value result = nullptr;
    std::unique_ptr<NapiAsyncTask> napiAsyncTask = CreateEmptyAsyncTask(env, lastParam, &result);
    auto asyncTask = [this, screenIds, env, task = napiAsyncTask.get()]() {
        HITRACE_METER_FMT(HITRACE_TAG_WINDOW_MANAGER, "JsScreenManager::OnMakeUnique");
        std::vector<DisplayId> displayIds;
        DmErrorCode ret = DM_JS_TO_ERROR_CODE_MAP.at(
            SingletonContainer::Get<ScreenManager>().MakeUniqueScreen(screenIds, displayIds));
        if (ret == DmErrorCode::DM_OK) {
            task->Resolve(env, CreateJsDisplayIdVectorObject(env, displayIds));
            TLOGNI(WmsLogTag::DMS, "makeUnique success");
        } else {
            task->Reject(env,
                CreateJsError(env, static_cast<int32_t>(ret), "JsScreenManager::OnMakeUnique failed."));
        }
        delete task;
    };
    NapiSendDmsEvent(env, asyncTask, napiAsyncTask, "OnMakeUnique");
    return result;
}

static int32_t GetExpandOptionFromJs(napi_env env, napi_value optionObject, ExpandOption& option)
{
    napi_value screedIdValue = nullptr;
    napi_value startXValue = nullptr;
    napi_value startYValue = nullptr;
    uint32_t screenId;
    uint32_t startX;
    uint32_t startY;
    napi_get_named_property(env, optionObject, "screenId", &screedIdValue);
    napi_get_named_property(env, optionObject, "startX", &startXValue);
    napi_get_named_property(env, optionObject, "startY", &startYValue);
    if (!ConvertFromJsValue(env, screedIdValue, screenId)) {
        TLOGE(WmsLogTag::DMS, "Failed to convert screedIdValue to callbackType");
        return -1;
    }
    if (!ConvertFromJsValue(env, startXValue, startX)) {
        TLOGE(WmsLogTag::DMS, "Failed to convert startXValue to callbackType");
        return -1;
    }
    if (!ConvertFromJsValue(env, startYValue, startY)) {
        TLOGE(WmsLogTag::DMS, "Failed to convert startYValue to callbackType");
        return -1;
    }
    option = {screenId, startX, startY};
    return 0;
}

static int32_t GetMultiScreenPositionOptionsFromJs(napi_env env, napi_value optionObject,
    MultiScreenPositionOptions& option)
{
    napi_value screedIdValue = nullptr;
    napi_value startXValue = nullptr;
    napi_value startYValue = nullptr;
    int64_t screenId;
    uint32_t startX;
    uint32_t startY;
    napi_get_named_property(env, optionObject, "id", &screedIdValue);
    napi_get_named_property(env, optionObject, "startX", &startXValue);
    napi_get_named_property(env, optionObject, "startY", &startYValue);
    if (!ConvertFromJsValue(env, screedIdValue, screenId)) {
        TLOGE(WmsLogTag::DMS, "Failed to convert screedIdValue to callbackType");
        return -1;
    }
    if (screenId < 0) {
        TLOGE(WmsLogTag::DMS, "Failed to convert,screenIdValue must be non-negative number");
        return -1;
    }
    if (!ConvertFromJsValue(env, startXValue, startX)) {
        TLOGE(WmsLogTag::DMS, "Failed to convert startXValue to callbackType");
        return -1;
    }
    if (!ConvertFromJsValue(env, startYValue, startY)) {
        TLOGE(WmsLogTag::DMS, "Failed to convert startYValue to callbackType");
        return -1;
    }
    option = {screenId, startX, startY};
    return 0;
}

static int32_t GetRectFromJs(napi_env env, napi_value optionObject, DMRect& rect)
{
    napi_value leftValue = nullptr;
    napi_value topValue = nullptr;
    napi_value widthValue = nullptr;
    napi_value heightValue = nullptr;
    int32_t left;
    int32_t top;
    uint32_t width;
    uint32_t height;
    napi_get_named_property(env, optionObject, "left", &leftValue);
    napi_get_named_property(env, optionObject, "top", &topValue);
    napi_get_named_property(env, optionObject, "width", &widthValue);
    napi_get_named_property(env, optionObject, "height", &heightValue);
    if (!ConvertFromJsValue(env, leftValue, left)) {
        TLOGE(WmsLogTag::DMS, "Failed to convert leftValue to callbackType");
        return -1;
    }
    if (!ConvertFromJsValue(env, topValue, top)) {
        TLOGE(WmsLogTag::DMS, "Failed to convert topValue to callbackType");
        return -1;
    }
    if (!ConvertFromJsValue(env, widthValue, width)) {
        TLOGE(WmsLogTag::DMS, "Failed to convert widthValue to callbackType");
        return -1;
    }
    if (!ConvertFromJsValue(env, heightValue, height)) {
        TLOGE(WmsLogTag::DMS, "Failed to convert heightValue to callbackType");
        return -1;
    }
    rect = {left, top, width, height};
    return 0;
}

napi_value OnCreateVirtualScreen(napi_env env, napi_callback_info info)
{
    TLOGI(WmsLogTag::DMS, "called");
    DmErrorCode errCode = DmErrorCode::DM_OK;
    VirtualScreenOption option;
    size_t argc = 4;
    std::string errMsg = "";
    napi_value argv[4] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc < ARGC_ONE) {
        errMsg = "Invalid args count, need one arg at least!";
        errCode = DmErrorCode::DM_ERROR_INVALID_PARAM;
    } else {
        napi_value object = argv[0];
        if (object == nullptr) {
            errMsg = "Failed to get options, options is nullptr";
            errCode = DmErrorCode::DM_ERROR_INVALID_PARAM;
        } else {
            errCode = GetVirtualScreenOptionFromJs(env, object, option);
        }
    }
    if (errCode == DmErrorCode::DM_ERROR_INVALID_PARAM) {
        return NapiThrowError(env, DmErrorCode::DM_ERROR_INVALID_PARAM, errMsg);
    }
    napi_value lastParam = nullptr;
    if (argc >= ARGC_TWO && argv[ARGC_TWO - 1] != nullptr &&
        GetType(env, argv[ARGC_TWO - 1]) == napi_function) {
        lastParam = argv[ARGC_TWO - 1];
    }
    napi_value result = nullptr;
    std::unique_ptr<NapiAsyncTask> napiAsyncTask = CreateEmptyAsyncTask(env, lastParam, &result);
    auto asyncTask = [option, env, task = napiAsyncTask.get()]() {
        HITRACE_METER_FMT(HITRACE_TAG_WINDOW_MANAGER, "JsScreenManager::OnCreateVirtualScreen");
        auto screenId = SingletonContainer::Get<ScreenManager>().CreateVirtualScreen(option);
        auto screen = SingletonContainer::Get<ScreenManager>().GetScreenById(screenId);
        if (screen == nullptr) {
            DmErrorCode ret = DmErrorCode::DM_ERROR_INVALID_SCREEN;
            TLOGNI(WmsLogTag::DMS, "screenId is %{public}" PRIu64"", screenId);
            if (screenId == ERROR_ID_NOT_SYSTEM_APP) {
                ret = DmErrorCode::DM_ERROR_NOT_SYSTEM_APP;
            } else if (screenId == ERROR_ID_NO_PERMISSION) {
                ret = DmErrorCode::DM_ERROR_NO_PERMISSION;
            }
            task->Reject(env, CreateJsError(env, static_cast<int32_t>(ret), "CreateVirtualScreen failed."));
            TLOGNE(WmsLogTag::DMS, "ScreenManager::CreateVirtualScreen failed.");
        } else {
            task->Resolve(env, CreateJsScreenObject(env, screen));
            TLOGNI(WmsLogTag::DMS, "JsScreenManager::OnCreateVirtualScreen success");
        }
        delete task;
    };
    NapiSendDmsEvent(env, asyncTask, napiAsyncTask, "OnCreateVirtualScreen");
    return result;
}

DmErrorCode GetVirtualScreenOptionFromJs(napi_env env, napi_value optionObject, VirtualScreenOption& option)
{
    napi_value name = nullptr;
    napi_get_named_property(env, optionObject, "name", &name);
    if (!ConvertFromJsValue(env, name, option.name_)) {
        TLOGE(WmsLogTag::DMS, "Failed to convert parameter to name.");
        return DmErrorCode::DM_ERROR_INVALID_PARAM;
    }
    napi_value width = nullptr;
    napi_get_named_property(env, optionObject, "width", &width);
    if (!ConvertFromJsValue(env, width, option.width_)) {
        TLOGE(WmsLogTag::DMS, "Failed to convert parameter to width.");
        return DmErrorCode::DM_ERROR_INVALID_PARAM;
    }
    napi_value height = nullptr;
    napi_get_named_property(env, optionObject, "height", &height);
    if (!ConvertFromJsValue(env, height, option.height_)) {
        TLOGE(WmsLogTag::DMS, "Failed to convert parameter to height.");
        return DmErrorCode::DM_ERROR_INVALID_PARAM;
    }
    napi_value density = nullptr;
    napi_get_named_property(env, optionObject, "density", &density);
    double densityValue;
    if (!ConvertFromJsValue(env, density, densityValue)) {
        TLOGE(WmsLogTag::DMS, "Failed to convert parameter to density.");
        return DmErrorCode::DM_ERROR_INVALID_PARAM;
    }
    option.density_ = static_cast<float>(densityValue);

    napi_value surfaceIdNapiValue = nullptr;
    napi_get_named_property(env, optionObject, "surfaceId", &surfaceIdNapiValue);
    if (!GetSurfaceFromJs(env, surfaceIdNapiValue, option.surface_)) {
        return DmErrorCode::DM_ERROR_INVALID_PARAM;
    }

    napi_value supportsFocus = nullptr;
    napi_status status = napi_get_named_property(env, optionObject, "supportsFocus", &supportsFocus);
    if (status != napi_ok) {
        TLOGE(WmsLogTag::DMS, "Failed to get parameter to supportsFocus.");
        return DmErrorCode::DM_ERROR_INVALID_PARAM;
    }
    if (!ConvertFromJsValue(env, supportsFocus, option.supportsFocus_)) {
        TLOGE(WmsLogTag::DMS, "No supportsFocus parameter to convert");
    }
    return DmErrorCode::DM_OK;
}

bool GetSurfaceFromJs(napi_env env, napi_value surfaceIdNapiValue, sptr<Surface>& surface)
{
    if (surfaceIdNapiValue == nullptr || GetType(env, surfaceIdNapiValue) != napi_string) {
        TLOGE(WmsLogTag::DMS, "Failed to convert parameter to surface. Invalidate params.");
        return false;
    }

    char buffer[PATH_MAX];
    size_t length = 0;
    uint64_t surfaceId = 0;
    if (napi_get_value_string_utf8(env, surfaceIdNapiValue, buffer, PATH_MAX, &length) != napi_ok) {
        TLOGE(WmsLogTag::DMS, "Failed to convert parameter to surface.");
        return false;
    }
    std::istringstream inputStream(buffer);
    inputStream >> surfaceId;
    surface = SurfaceUtils::GetInstance()->GetSurface(surfaceId);
    if (surface == nullptr) {
        TLOGI(WmsLogTag::DMS, "GetSurfaceFromJs failed, surfaceId:%{public}" PRIu64"", surfaceId);
    }
    return true;
}

napi_value OnDestroyVirtualScreen(napi_env env, napi_callback_info info)
{
    TLOGI(WmsLogTag::DMS, "called");
    DmErrorCode errCode = DmErrorCode::DM_OK;
    int64_t screenId = -1LL;
    std::string errMsg = "";
    size_t argc = 4;
    napi_value argv[4] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc < ARGC_ONE) {
        TLOGE(WmsLogTag::DMS, "[NAPI]Argc is invalid: %{public}zu", argc);
        errMsg = "Invalid args count, need one arg at least!";
        errCode = DmErrorCode::DM_ERROR_INVALID_PARAM;
    } else {
        if (!ConvertFromJsValue(env, argv[0], screenId)) {
            TLOGE(WmsLogTag::DMS, "Failed to convert parameter to screen id.");
            errMsg = "Failed to convert parameter to screen id.";
            errCode = DmErrorCode::DM_ERROR_INVALID_PARAM;
        }
    }
    if (errCode == DmErrorCode::DM_ERROR_INVALID_PARAM || screenId == -1LL) {
        TLOGE(WmsLogTag::DMS, "failed, Invalidate params.");
        return NapiThrowError(env, DmErrorCode::DM_ERROR_INVALID_PARAM, errMsg);
    }
    napi_value lastParam = nullptr;
    if (argc >= ARGC_TWO && argv[ARGC_TWO - 1] != nullptr &&
        GetType(env, argv[ARGC_TWO - 1]) == napi_function) {
        lastParam = argv[ARGC_TWO - 1];
    }
    napi_value result = nullptr;
    std::unique_ptr<NapiAsyncTask> napiAsyncTask = CreateEmptyAsyncTask(env, lastParam, &result);
    auto asyncTask = [screenId, env, task = napiAsyncTask.get()]() {
        HITRACE_METER_FMT(HITRACE_TAG_WINDOW_MANAGER, "JsScreenManager::OnDestroyVirtualScreen");
        auto res = DM_JS_TO_ERROR_CODE_MAP.at(
            SingletonContainer::Get<ScreenManager>().DestroyVirtualScreen(screenId));
        if (res != DmErrorCode::DM_OK) {
            task->Reject(env, CreateJsError(env, static_cast<int32_t>(res),
                "ScreenManager::DestroyVirtualScreen failed."));
            TLOGNE(WmsLogTag::DMS, "ScreenManager::DestroyVirtualScreen failed.");
            delete task;
            return;
        }
        task->Resolve(env, NapiGetUndefined(env));
        TLOGNI(WmsLogTag::DMS, "JsScreenManager::OnDestroyVirtualScreen success");
        delete task;
    };
    NapiSendDmsEvent(env, asyncTask, napiAsyncTask, "OnDestroyVirtualScreen");
    return result;
}

napi_value OnSetVirtualScreenSurface(napi_env env, napi_callback_info info)
{
    TLOGI(WmsLogTag::DMS, "called");
    DmErrorCode errCode = DmErrorCode::DM_OK;
    int64_t screenId = -1LL;
    sptr<Surface> surface;
    size_t argc = 4;
    std::string errMsg = "";
    napi_value argv[4] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc < ARGC_TWO) {
        TLOGE(WmsLogTag::DMS, "[NAPI]Argc is invalid: %{public}zu", argc);
        errMsg = "Invalid args count, need 2 args at least!";
        errCode = DmErrorCode::DM_ERROR_INVALID_PARAM;
    } else {
        if (!ConvertFromJsValue(env, argv[0], screenId)) {
            errMsg = "Failed to convert parameter to screen id.";
            errCode = DmErrorCode::DM_ERROR_INVALID_PARAM;
        }
        if (!GetSurfaceFromJs(env, argv[1], surface)) {
            errMsg = "Failed to convert parameter.";
            errCode = DmErrorCode::DM_ERROR_INVALID_PARAM;
        }
    }
    if (errCode == DmErrorCode::DM_ERROR_INVALID_PARAM || surface == nullptr) {
        TLOGE(WmsLogTag::DMS, "failed, Invalidate params.");
        return NapiThrowError(env, DmErrorCode::DM_ERROR_INVALID_PARAM, errMsg);
    }
    napi_value lastParam = nullptr;
    if (argc >= ARGC_THREE && argv[ARGC_THREE - 1] != nullptr &&
        GetType(env, argv[ARGC_THREE - 1]) == napi_function) {
        lastParam = argv[ARGC_THREE - 1];
    }
    napi_value result = nullptr;
    std::unique_ptr<NapiAsyncTask> napiAsyncTask = CreateEmptyAsyncTask(env, lastParam, &result);
    auto asyncTask = [screenId, surface, env, task = napiAsyncTask.get()]() {
        HITRACE_METER_FMT(HITRACE_TAG_WINDOW_MANAGER, "JsScreenManager::OnSetVirtualScreenSurface");
        auto res = DM_JS_TO_ERROR_CODE_MAP.at(
            SingletonContainer::Get<ScreenManager>().SetVirtualScreenSurface(screenId, surface));
        if (res != DmErrorCode::DM_OK) {
            task->Reject(env, CreateJsError(env, static_cast<int32_t>(res),
                "ScreenManager::SetVirtualScreenSurface failed."));
            TLOGNE(WmsLogTag::DMS, "ScreenManager::SetVirtualScreenSurface failed.");
        } else {
            task->Resolve(env, NapiGetUndefined(env));
        }
        delete task;
    };
    NapiSendDmsEvent(env, asyncTask, napiAsyncTask, "OnSetVirtualScreenSurface");
    return result;
}

napi_value OnSetScreenPrivacyMaskImage(napi_env env, napi_callback_info info)
{
    TLOGI(WmsLogTag::DMS, "called");
    DmErrorCode errCode = DmErrorCode::DM_OK;
    int64_t screenId = -1LL;
    size_t argc = 4;
    std::string errMsg = "";
    napi_value argv[4] = {nullptr};
    std::shared_ptr<Media::PixelMap> privacyMaskImg;
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc < ARGC_ONE) {
        TLOGE(WmsLogTag::DMS, "[NAPI]Argc is invalid: %{public}zu", argc);
        errMsg = "Invalid args count, need 1 args at least!";
        errCode = DmErrorCode::DM_ERROR_INVALID_PARAM;
    } else {
        if (!ConvertFromJsValue(env, argv[0], screenId)) {
            errMsg = "Failed to convert parameter to screen id.";
            errCode = DmErrorCode::DM_ERROR_INVALID_PARAM;
        }
        if (argc > ARGC_ONE && GetType(env, argv[1]) == napi_object) {
            privacyMaskImg = OHOS::Media::PixelMapNapi::GetPixelMap(env, argv[1]);
            if (privacyMaskImg == nullptr) {
                errMsg = "Failed to convert parameter to pixelmap.";
                errCode = DmErrorCode::DM_ERROR_INVALID_PARAM;
            }
        }
    }
    if (errCode != DmErrorCode::DM_OK) {
        TLOGE(WmsLogTag::DMS, "failed, Invalidate params.");
        return NapiThrowError(env, DmErrorCode::DM_ERROR_INVALID_PARAM, errMsg);
    }
    napi_value lastParam = nullptr;
    lastParam = (argc >= ARGC_THREE && GetType(env, argv[ARGC_THREE - 1]) ==
        napi_function) ? argv[ARGC_THREE - 1] : argv[ARGC_TWO - 1];
    napi_value result = nullptr;
    std::unique_ptr<NapiAsyncTask> napiAsyncTask = CreateEmptyAsyncTask(env, lastParam, &result);
    auto asyncTask = [screenId, privacyMaskImg, env, task = napiAsyncTask.get()]() {
        HITRACE_METER_FMT(HITRACE_TAG_WINDOW_MANAGER, "JsScreenManager::OnSetScreenPrivacyMaskImage");
        auto res = DM_JS_TO_ERROR_CODE_MAP.at(
            SingletonContainer::Get<ScreenManager>().SetScreenPrivacyMaskImage(screenId, privacyMaskImg));
        if (res != DmErrorCode::DM_OK) {
            task->Reject(env, CreateJsError(env, static_cast<int32_t>(res), "OnSetScreenPrivacyMaskImage failed."));
            TLOGNE(WmsLogTag::DMS, "OnSetScreenPrivacyMaskImage failed.");
        } else {
            task->Resolve(env, NapiGetUndefined(env));
        }
        delete task;
    };
    NapiSendDmsEvent(env, asyncTask, napiAsyncTask, "OnSetScreenPrivacyMaskImage");
    return result;
}

napi_value OnIsScreenRotationLocked(napi_env env, napi_callback_info info)
{
    size_t argc = 4;
    napi_value argv[4] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    TLOGI(WmsLogTag::DMS, "called");
    napi_value lastParam = nullptr;
    if (argc >= ARGC_ONE && argv[ARGC_ONE - 1] != nullptr &&
        GetType(env, argv[ARGC_ONE - 1]) == napi_function) {
        lastParam = argv[ARGC_ONE - 1];
    }
    napi_value result = nullptr;
    std::unique_ptr<NapiAsyncTask> napiAsyncTask = CreateEmptyAsyncTask(env, lastParam, &result);
    auto asyncTask = [env, task = napiAsyncTask.get()]() {
        HITRACE_METER_FMT(HITRACE_TAG_WINDOW_MANAGER, "JsScreenManager::OnIsScreenRotationLocked");
        bool isLocked = false;
        auto res = DM_JS_TO_ERROR_CODE_MAP.at(
            SingletonContainer::Get<ScreenManager>().IsScreenRotationLocked(isLocked));
        if (res == DmErrorCode::DM_OK) {
            task->Resolve(env, CreateJsValue(env, isLocked));
            TLOGNI(WmsLogTag::DMS, "OnIsScreenRotationLocked success");
        } else {
            task->Reject(env, CreateJsError(env, static_cast<int32_t>(res),
                                            "JsScreenManager::OnIsScreenRotationLocked failed."));
            TLOGNE(WmsLogTag::DMS, "OnIsScreenRotationLocked failed");
        }
        delete task;
    };
    NapiSendDmsEvent(env, asyncTask, napiAsyncTask, "OnIsScreenRotationLocked");
    return result;
}

napi_value OnSetScreenRotationLocked(napi_env env, napi_callback_info info)
{
    TLOGI(WmsLogTag::DMS, "called");
    DmErrorCode errCode = DmErrorCode::DM_OK;
    size_t argc = 4;
    std::string errMsg = "";
    napi_value argv[4] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc < ARGC_ONE) {
        TLOGE(WmsLogTag::DMS, "[NAPI]Argc is invalid: %{public}zu", argc);
        errMsg = "Invalid args count, need one arg at least!";
        errCode = DmErrorCode::DM_ERROR_INVALID_PARAM;
    }
    bool isLocked = false;
    if (errCode == DmErrorCode::DM_OK) {
        napi_value nativeVal = argv[0];
        if (nativeVal == nullptr) {
            TLOGE(WmsLogTag::DMS, "[NAPI]Failed to convert parameter to isLocked");
            errMsg = "Failed to convert parameter to isLocked.";
            errCode = DmErrorCode::DM_ERROR_INVALID_PARAM;
        } else {
            napi_get_value_bool(env, nativeVal, &isLocked);
        }
    }
    if (errCode == DmErrorCode::DM_ERROR_INVALID_PARAM) {
        TLOGE(WmsLogTag::DMS, "failed, Invalidate params.");
        return NapiThrowError(env, DmErrorCode::DM_ERROR_INVALID_PARAM, errMsg);
    }
    napi_value lastParam = (argc <= ARGC_ONE) ? nullptr :
        ((argv[ARGC_TWO - 1] != nullptr && GetType(env, argv[ARGC_TWO - 1]) == napi_function) ?
        argv[1] : nullptr);
    napi_value result = nullptr;
    std::unique_ptr<NapiAsyncTask> napiAsyncTask = CreateEmptyAsyncTask(env, lastParam, &result);
    auto asyncTask = [isLocked, env, task = napiAsyncTask.get()]() {
        HITRACE_METER_FMT(HITRACE_TAG_WINDOW_MANAGER, "JsScreenManager::OnSetScreenRotationLocked");
        auto res = DM_JS_TO_ERROR_CODE_MAP.at(
            SingletonContainer::Get<ScreenManager>().SetScreenRotationLocked(isLocked));
        if (res == DmErrorCode::DM_OK) {
            task->Resolve(env, NapiGetUndefined(env));
            TLOGNI(WmsLogTag::DMS, "OnSetScreenRotationLocked success");
        } else {
            task->Reject(env, CreateJsError(env, static_cast<int32_t>(res),
                                                "JsScreenManager::OnSetScreenRotationLocked failed."));
            TLOGNE(WmsLogTag::DMS, "OnSetScreenRotationLocked failed");
        }
        delete task;
    };
    NapiSendDmsEvent(env, asyncTask, napiAsyncTask, "OnSetScreenRotationLocked");
    return result;
}

void NapiSendDmsEvent(napi_env env, std::function<void()> asyncTask,
    std::unique_ptr<AbilityRuntime::NapiAsyncTask>& napiAsyncTask, const std::string& taskName)
{
    if (!env) {
        TLOGE(WmsLogTag::DMS, "env is null");
        return;
    }
    if (napi_send_event(env, asyncTask, napi_eprio_immediate, taskName.c_str()) != napi_status::napi_ok) {
        napiAsyncTask->Reject(env, CreateJsError(env,
                static_cast<int32_t>(DmErrorCode::DM_ERROR_INVALID_SCREEN), "Send event failed!"));
    } else {
        napiAsyncTask.release();
        TLOGE(WmsLogTag::DMS, "send event success");
    }
}

std::unique_ptr<NapiAsyncTask> CreateEmptyAsyncTask(napi_env env, napi_value lastParam, napi_value* result)
{
    napi_valuetype type = napi_undefined;
    napi_typeof(env, lastParam, &type);
    if (lastParam == nullptr || type != napi_function) {
        napi_deferred nativeDeferred = nullptr;
        napi_create_promise(env, &nativeDeferred, result);
        return std::make_unique<NapiAsyncTask>(nativeDeferred, std::unique_ptr<NapiAsyncTask::ExecuteCallback>(),
            std::unique_ptr<NapiAsyncTask::CompleteCallback>());
    } else {
        napi_get_undefined(env, result);
        napi_ref callbackRef = nullptr;
        napi_create_reference(env, lastParam, 1, &callbackRef);
        return std::make_unique<NapiAsyncTask>(callbackRef, std::unique_ptr<NapiAsyncTask::ExecuteCallback>(),
            std::unique_ptr<NapiAsyncTask::CompleteCallback>());
    }
}
};

napi_value InitScreenOrientation(napi_env env)
{
    TLOGD(WmsLogTag::DMS, "called");

    if (env == nullptr) {
        TLOGE(WmsLogTag::DMS, "env is nullptr");
        return nullptr;
    }

    napi_value objValue = nullptr;
    napi_create_object(env, &objValue);
    if (objValue == nullptr) {
        TLOGE(WmsLogTag::DMS, "Failed to get object");
        return nullptr;
    }

    napi_set_named_property(env, objValue, "UNSPECIFIED",
        CreateJsValue(env, static_cast<int32_t>(Orientation::UNSPECIFIED)));
    napi_set_named_property(env, objValue, "VERTICAL",
        CreateJsValue(env, static_cast<int32_t>(Orientation::VERTICAL)));
    napi_set_named_property(env, objValue, "HORIZONTAL",
        CreateJsValue(env, static_cast<int32_t>(Orientation::HORIZONTAL)));
    napi_set_named_property(env, objValue, "REVERSE_VERTICAL",
        CreateJsValue(env, static_cast<int32_t>(Orientation::REVERSE_VERTICAL)));
    napi_set_named_property(env, objValue, "REVERSE_HORIZONTAL",
        CreateJsValue(env, static_cast<int32_t>(Orientation::REVERSE_HORIZONTAL)));
    return objValue;
}

napi_value InitScreenSourceMode(napi_env env)
{
    TLOGD(WmsLogTag::DMS, "called");

    if (env == nullptr) {
        TLOGE(WmsLogTag::DMS, "env is nullptr");
        return nullptr;
    }

    napi_value objValue = nullptr;
    napi_create_object(env, &objValue);
    if (objValue == nullptr) {
        TLOGE(WmsLogTag::DMS, "Failed to get object");
        return nullptr;
    }

    napi_set_named_property(env, objValue, "SCREEN_MAIN",
        CreateJsValue(env, static_cast<uint32_t>(ScreenSourceMode::SCREEN_MAIN)));
    napi_set_named_property(env, objValue, "SCREEN_MIRROR",
        CreateJsValue(env, static_cast<uint32_t>(ScreenSourceMode::SCREEN_MIRROR)));
    napi_set_named_property(env, objValue, "SCREEN_EXTEND",
        CreateJsValue(env, static_cast<uint32_t>(ScreenSourceMode::SCREEN_EXTEND)));
    napi_set_named_property(env, objValue, "SCREEN_ALONE",
        CreateJsValue(env, static_cast<uint32_t>(ScreenSourceMode::SCREEN_ALONE)));
    return objValue;
}

napi_value InitMultiScreenMode(napi_env env)
{
    TLOGD(WmsLogTag::DMS, "called");

    if (env == nullptr) {
        TLOGE(WmsLogTag::DMS, "env is nullptr");
        return nullptr;
    }

    napi_value objValue = nullptr;
    napi_create_object(env, &objValue);
    if (objValue == nullptr) {
        TLOGE(WmsLogTag::DMS, "Failed to get object");
        return nullptr;
    }

    napi_set_named_property(env, objValue, "SCREEN_MIRROR",
        CreateJsValue(env, static_cast<uint32_t>(MultiScreenMode::SCREEN_MIRROR)));
    napi_set_named_property(env, objValue, "SCREEN_EXTEND",
        CreateJsValue(env, static_cast<uint32_t>(MultiScreenMode::SCREEN_EXTEND)));
    return objValue;
}

napi_value InitDisplayErrorCode(napi_env env)
{
    TLOGD(WmsLogTag::DMS, "called");

    if (env == nullptr) {
        TLOGE(WmsLogTag::DMS, "env is nullptr");
        return nullptr;
    }

    napi_value objValue = nullptr;
    napi_create_object(env, &objValue);
    if (objValue == nullptr) {
        TLOGE(WmsLogTag::DMS, "Failed to get object");
        return nullptr;
    }

    napi_set_named_property(env, objValue, "DM_ERROR_NO_PERMISSION",
        CreateJsValue(env, static_cast<int32_t>(DmErrorCode::DM_ERROR_NO_PERMISSION)));
    napi_set_named_property(env, objValue, "DM_ERROR_NOT_SYSTEM_APP",
        CreateJsValue(env, static_cast<int32_t>(DmErrorCode::DM_ERROR_NOT_SYSTEM_APP)));
    napi_set_named_property(env, objValue, "DM_ERROR_INVALID_PARAM",
        CreateJsValue(env, static_cast<int32_t>(DmErrorCode::DM_ERROR_INVALID_PARAM)));
    napi_set_named_property(env, objValue, "DM_ERROR_DEVICE_NOT_SUPPORT",
        CreateJsValue(env, static_cast<int32_t>(DmErrorCode::DM_ERROR_DEVICE_NOT_SUPPORT)));
    napi_set_named_property(env, objValue, "DM_ERROR_INVALID_SCREEN",
        CreateJsValue(env, static_cast<int32_t>(DmErrorCode::DM_ERROR_INVALID_SCREEN)));
    napi_set_named_property(env, objValue, "DM_ERROR_INVALID_CALLING",
        CreateJsValue(env, static_cast<int32_t>(DmErrorCode::DM_ERROR_INVALID_CALLING)));
    napi_set_named_property(env, objValue, "DM_ERROR_SYSTEM_INNORMAL",
        CreateJsValue(env, static_cast<int32_t>(DmErrorCode::DM_ERROR_SYSTEM_INNORMAL)));
    return objValue;
}

napi_value InitDisplayError(napi_env env)
{
    TLOGD(WmsLogTag::DMS, "called");

    if (env == nullptr) {
        TLOGE(WmsLogTag::DMS, "env is nullptr");
        return nullptr;
    }

    napi_value objValue = nullptr;
    napi_create_object(env, &objValue);
    if (objValue == nullptr) {
        TLOGE(WmsLogTag::DMS, "Failed to get object");
        return nullptr;
    }

    napi_set_named_property(env, objValue, "DM_ERROR_INIT_DMS_PROXY_LOCKED",
        CreateJsValue(env, static_cast<int32_t>(DMError::DM_ERROR_INIT_DMS_PROXY_LOCKED)));
    napi_set_named_property(env, objValue, "DM_ERROR_IPC_FAILED",
        CreateJsValue(env, static_cast<int32_t>(DMError::DM_ERROR_IPC_FAILED)));
    napi_set_named_property(env, objValue, "DM_ERROR_REMOTE_CREATE_FAILED",
        CreateJsValue(env, static_cast<int32_t>(DMError::DM_ERROR_REMOTE_CREATE_FAILED)));
    napi_set_named_property(env, objValue, "DM_ERROR_NULLPTR",
        CreateJsValue(env, static_cast<int32_t>(DMError::DM_ERROR_NULLPTR)));
    napi_set_named_property(env, objValue, "DM_ERROR_INVALID_PARAM",
        CreateJsValue(env, static_cast<int32_t>(DMError::DM_ERROR_INVALID_PARAM)));
    napi_set_named_property(env, objValue, "DM_ERROR_WRITE_INTERFACE_TOKEN_FAILED",
        CreateJsValue(env, static_cast<int32_t>(DMError::DM_ERROR_WRITE_INTERFACE_TOKEN_FAILED)));
    napi_set_named_property(env, objValue, "DM_ERROR_DEATH_RECIPIENT",
        CreateJsValue(env, static_cast<int32_t>(DMError::DM_ERROR_DEATH_RECIPIENT)));
    napi_set_named_property(env, objValue, "DM_ERROR_INVALID_MODE_ID",
        CreateJsValue(env, static_cast<int32_t>(DMError::DM_ERROR_INVALID_MODE_ID)));
    napi_set_named_property(env, objValue, "DM_ERROR_WRITE_DATA_FAILED",
        CreateJsValue(env, static_cast<int32_t>(DMError::DM_ERROR_WRITE_DATA_FAILED)));
    napi_set_named_property(env, objValue, "DM_ERROR_RENDER_SERVICE_FAILED",
        CreateJsValue(env, static_cast<int32_t>(DMError::DM_ERROR_RENDER_SERVICE_FAILED)));
    napi_set_named_property(env, objValue, "DM_ERROR_UNREGISTER_AGENT_FAILED",
        CreateJsValue(env, static_cast<int32_t>(DMError::DM_ERROR_UNREGISTER_AGENT_FAILED)));
    napi_set_named_property(env, objValue, "DM_ERROR_INVALID_CALLING",
        CreateJsValue(env, static_cast<int32_t>(DMError::DM_ERROR_INVALID_CALLING)));
    napi_set_named_property(env, objValue, "DM_ERROR_UNKNOWN",
        CreateJsValue(env, static_cast<int32_t>(DMError::DM_ERROR_UNKNOWN)));
    return objValue;
}

napi_value JsScreenManagerInit(napi_env env, napi_value exportObj)
{
    TLOGD(WmsLogTag::DMS, "called");

    if (env == nullptr || exportObj == nullptr) {
        TLOGE(WmsLogTag::DMS, "env or exportObj is nullptr");
        return nullptr;
    }

    std::unique_ptr<JsScreenManager> jsScreenManager = std::make_unique<JsScreenManager>(env);
    napi_wrap(env, exportObj, jsScreenManager.release(), JsScreenManager::Finalizer, nullptr, nullptr);
    napi_set_named_property(env, exportObj, "Orientation", InitScreenOrientation(env));
    napi_set_named_property(env, exportObj, "ScreenSourceMode", InitScreenSourceMode(env));
    napi_set_named_property(env, exportObj, "DmErrorCode", InitDisplayErrorCode(env));
    napi_set_named_property(env, exportObj, "DMError", InitDisplayError(env));
    napi_set_named_property(env, exportObj, "MultiScreenMode", InitMultiScreenMode(env));

    const char *moduleName = "JsScreenManager";
    BindNativeFunction(env, exportObj, "getAllScreens", moduleName, JsScreenManager::GetAllScreens);
    BindNativeFunction(env, exportObj, "on", moduleName, JsScreenManager::RegisterScreenManagerCallback);
    BindNativeFunction(env, exportObj, "off", moduleName, JsScreenManager::UnregisterScreenMangerCallback);
    BindNativeFunction(env, exportObj, "makeMirror", moduleName, JsScreenManager::MakeMirror);
    BindNativeFunction(env, exportObj, "makeMirrorWithRegion", moduleName, JsScreenManager::MakeMirrorWithRegion);
    BindNativeFunction(env, exportObj, "setMultiScreenMode", moduleName, JsScreenManager::SetMultiScreenMode);
    BindNativeFunction(env, exportObj, "setMultiScreenRelativePosition", moduleName,
        JsScreenManager::SetMultiScreenRelativePosition);
    BindNativeFunction(env, exportObj, "makeExpand", moduleName, JsScreenManager::MakeExpand);
    BindNativeFunction(env, exportObj, "stopMirror", moduleName, JsScreenManager::StopMirror);
    BindNativeFunction(env, exportObj, "stopExpand", moduleName, JsScreenManager::StopExpand);
    BindNativeFunction(env, exportObj, "createVirtualScreen", moduleName, JsScreenManager::CreateVirtualScreen);
    BindNativeFunction(env, exportObj, "destroyVirtualScreen", moduleName, JsScreenManager::DestroyVirtualScreen);
    BindNativeFunction(env, exportObj, "setVirtualScreenSurface", moduleName,
        JsScreenManager::SetVirtualScreenSurface);
    BindNativeFunction(env, exportObj, "setScreenPrivacyMaskImage", moduleName,
        JsScreenManager::SetScreenPrivacyMaskImage);
    BindNativeFunction(env, exportObj, "setScreenRotationLocked", moduleName,
        JsScreenManager::SetScreenRotationLocked);
    BindNativeFunction(env, exportObj, "isScreenRotationLocked", moduleName,
        JsScreenManager::IsScreenRotationLocked);
    BindNativeFunction(env, exportObj, "makeUnique", moduleName, JsScreenManager::MakeUnique);
    return NapiGetUndefined(env);
}
}  // namespace Rosen
}  // namespace OHOS
