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

#include <vector>
#include <new>

#include <hitrace_meter.h>
#include <js_err_utils.h>
#include "js_runtime_utils.h"
#include "native_engine/native_reference.h"
#include "display_manager.h"
#include "window_manager_hilog.h"
#include "singleton_container.h"
#include "js_display_listener.h"
#include "js_display.h"
#include "js_display_manager.h"
#include "scene_board_judgement.h"
#include "screen.h"
#include "screen_manager.h"
#include "surface_utils.h"

namespace OHOS {
namespace Rosen {
namespace {
const std::map<DmErrorCode, std::string> brightnessInfoErrCodeMap = {
    {DmErrorCode::DM_ERROR_DEVICE_NOT_SUPPORT, "Capability not supported."},
    {DmErrorCode::DM_ERROR_SYSTEM_INNORMAL, "This display manager service works abnormally."},
    {DmErrorCode::DM_ERROR_ILLEGAL_PARAM, "Parameter error. Possible cause: 1. Invalid parameter range."}
};
}
using namespace AbilityRuntime;
constexpr size_t ARGC_ONE = 1;
constexpr size_t ARGC_TWO = 2;
constexpr size_t ARGC_THREE = 3;
constexpr size_t ARGS_MAX = 4;
constexpr int32_t INDEX_ONE = 1;
class JsDisplayManager {
public:
explicit JsDisplayManager(napi_env env) {
}

~JsDisplayManager() = default;

static void Finalizer(napi_env env, void* data, void* hint)
{
    TLOGD(WmsLogTag::DMS, "called");
    std::unique_ptr<JsDisplayManager>(static_cast<JsDisplayManager*>(data));
}

static napi_value GetDefaultDisplay(napi_env env, napi_callback_info info)
{
    JsDisplayManager* me = CheckParamsAndGetThis<JsDisplayManager>(env, info);
    return (me != nullptr) ? me->OnGetDefaultDisplay(env, info) : nullptr;
}

static napi_value GetDefaultDisplaySync(napi_env env, napi_callback_info info)
{
    JsDisplayManager* me = CheckParamsAndGetThis<JsDisplayManager>(env, info);
    return (me != nullptr) ? me->OnGetDefaultDisplaySync(env, info) : nullptr;
}

static napi_value GetPrimaryDisplaySync(napi_env env, napi_callback_info info)
{
    JsDisplayManager* me = CheckParamsAndGetThis<JsDisplayManager>(env, info);
    return (me != nullptr) ? me->OnGetPrimaryDisplaySync(env, info) : nullptr;
}

static napi_value GetDisplayByIdSync(napi_env env, napi_callback_info info)
{
    JsDisplayManager* me = CheckParamsAndGetThis<JsDisplayManager>(env, info);
    return (me != nullptr) ? me->OnGetDisplayByIdSync(env, info) : nullptr;
}

static napi_value GetBrightnessInfoChange(napi_env env, napi_callback_info info)
{
    JsDisplayManager* me = CheckParamsAndGetThis<JsDisplayManager>(env, info);
    return (me != nullptr) ? me->OnGetBrightnessInfoChange(env, info) : nullptr;
}

static napi_value GetAllDisplay(napi_env env, napi_callback_info info)
{
    JsDisplayManager* me = CheckParamsAndGetThis<JsDisplayManager>(env, info);
    return (me != nullptr) ? me->OnGetAllDisplay(env, info) : nullptr;
}

static napi_value GetAllDisplayPhysicalResolution(napi_env env, napi_callback_info info)
{
    JsDisplayManager* me = CheckParamsAndGetThis<JsDisplayManager>(env, info);
    return (me != nullptr) ? me->OnGetAllDisplayPhysicalResolution(env, info) : nullptr;
}

static napi_value GetAllDisplays(napi_env env, napi_callback_info info)
{
    JsDisplayManager* me = CheckParamsAndGetThis<JsDisplayManager>(env, info);
    return (me != nullptr) ? me->OnGetAllDisplays(env, info) : nullptr;
}

static napi_value RegisterDisplayManagerCallback(napi_env env, napi_callback_info info)
{
    JsDisplayManager* me = CheckParamsAndGetThis<JsDisplayManager>(env, info);
    return (me != nullptr) ? me->OnRegisterDisplayManagerCallback(env, info) : nullptr;
}

static napi_value UnregisterDisplayManagerCallback(napi_env env, napi_callback_info info)
{
    JsDisplayManager* me = CheckParamsAndGetThis<JsDisplayManager>(env, info);
    return (me != nullptr) ? me->OnUnregisterDisplayManagerCallback(env, info) : nullptr;
}

static napi_value HasPrivateWindow(napi_env env, napi_callback_info info)
{
    JsDisplayManager* me = CheckParamsAndGetThis<JsDisplayManager>(env, info);
    return (me != nullptr) ? me->OnHasPrivateWindow(env, info) : nullptr;
}

static napi_value IsFoldable(napi_env env, napi_callback_info info)
{
    auto* me = CheckParamsAndGetThis<JsDisplayManager>(env, info);
    return (me != nullptr) ? me->OnIsFoldable(env, info) : nullptr;
}

static napi_value IsCaptured(napi_env env, napi_callback_info info)
{
    auto* me = CheckParamsAndGetThis<JsDisplayManager>(env, info);
    return (me != nullptr) ? me->OnIsCaptured(env, info) : nullptr;
}

static napi_value GetFoldStatus(napi_env env, napi_callback_info info)
{
    auto* me = CheckParamsAndGetThis<JsDisplayManager>(env, info);
    return (me != nullptr) ? me->OnGetFoldStatus(env, info) : nullptr;
}

static napi_value GetFoldDisplayMode(napi_env env, napi_callback_info info)
{
    auto* me = CheckParamsAndGetThis<JsDisplayManager>(env, info);
    return (me != nullptr) ? me->OnGetFoldDisplayMode(env, info) : nullptr;
}

static napi_value SetFoldDisplayMode(napi_env env, napi_callback_info info)
{
    auto* me = CheckParamsAndGetThis<JsDisplayManager>(env, info);
    return (me != nullptr) ? me->OnSetFoldDisplayMode(env, info) : nullptr;
}

static napi_value SetFoldStatusLocked(napi_env env, napi_callback_info info)
{
    auto* me = CheckParamsAndGetThis<JsDisplayManager>(env, info);
    return (me != nullptr) ? me->OnSetFoldStatusLocked(env, info) : nullptr;
}

static napi_value GetCurrentFoldCreaseRegion(napi_env env, napi_callback_info info)
{
    auto* me = CheckParamsAndGetThis<JsDisplayManager>(env, info);
    return (me != nullptr) ? me->OnGetCurrentFoldCreaseRegion(env, info) : nullptr;
}

static napi_value CreateVirtualScreen(napi_env env, napi_callback_info info)
{
    auto* me = CheckParamsAndGetThis<JsDisplayManager>(env, info);
    return (me != nullptr) ? me->OnCreateVirtualScreen(env, info) : nullptr;
}

static napi_value MakeUnique(napi_env env, napi_callback_info info)
{
    auto* me = CheckParamsAndGetThis<JsDisplayManager>(env, info);
    return (me != nullptr) ? me->OnMakeUnique(env, info) : nullptr;
}

static napi_value DestroyVirtualScreen(napi_env env, napi_callback_info info)
{
    auto* me = CheckParamsAndGetThis<JsDisplayManager>(env, info);
    return (me != nullptr) ? me->OnDestroyVirtualScreen(env, info) : nullptr;
}

static napi_value SetVirtualScreenSurface(napi_env env, napi_callback_info info)
{
    auto* me = CheckParamsAndGetThis<JsDisplayManager>(env, info);
    return (me != nullptr) ? me->OnSetVirtualScreenSurface(env, info) : nullptr;
}

static napi_value AddVirtualScreenBlockList(napi_env env, napi_callback_info info)
{
    auto* me = CheckParamsAndGetThis<JsDisplayManager>(env, info);
    return (me != nullptr) ? me->OnAddVirtualScreenBlockList(env, info) : nullptr;
}

static napi_value RemoveVirtualScreenBlockList(napi_env env, napi_callback_info info)
{
    auto* me = CheckParamsAndGetThis<JsDisplayManager>(env, info);
    return (me != nullptr) ? me->OnRemoveVirtualScreenBlockList(env, info) : nullptr;
}

static napi_value ConvertRelativeCoordinateToGlobal(napi_env env, napi_callback_info info)
{
    auto* me = CheckParamsAndGetThis<JsDisplayManager>(env, info);
    return (me != nullptr) ? me->OnConvertRelativeCoordinateToGlobal(env, info) : nullptr;
}

static napi_value ConvertGlobalCoordinateToRelative(napi_env env, napi_callback_info info)
{
    auto* me = CheckParamsAndGetThis<JsDisplayManager>(env, info);
    return (me != nullptr) ? me->OnConvertGlobalCoordinateToRelative(env, info) : nullptr;
}

static napi_value RegisterDisplayAttributeChangeCallback(napi_env env, napi_callback_info info)
{
    JsDisplayManager* me = CheckParamsAndGetThis<JsDisplayManager>(env, info);
    return (me != nullptr) ? me->OnRegisterDisplayAttributeChangeCallback(env, info) : nullptr;
}

private:
std::map<std::string, std::map<std::unique_ptr<NativeReference>, sptr<JsDisplayListener>>> jsCbMap_;
std::map<std::string, std::map<std::unique_ptr<NativeReference>, sptr<JsDisplayListener>>> jsAttributeCbMap_;
std::mutex mtx_;

napi_value OnGetDefaultDisplay(napi_env env, napi_callback_info info)
{
    TLOGI(WmsLogTag::DMS, "called");
    DMError errCode = DMError::DM_OK;
    size_t argc = 4;
    napi_value argv[4] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc != 0 && argc != ARGC_ONE) {
        TLOGE(WmsLogTag::DMS, "params not match");
        errCode = DMError::DM_ERROR_INVALID_PARAM;
    }

    napi_value lastParam = nullptr;
    if (argc == ARGC_ONE && GetType(env, argv[0]) == napi_function) {
        lastParam = argv[0];
    }
    napi_value result = nullptr;
    std::unique_ptr<NapiAsyncTask> napiAsyncTask = CreateEmptyAsyncTask(env, lastParam, &result);
    auto asyncTask = [this, env, errCode, task = napiAsyncTask.get()] {
        if (errCode != DMError::DM_OK) {
            task->Reject(env, CreateJsError(env,
                static_cast<int32_t>(errCode), "JsDisplayManager::OnGetDefaultDisplay failed."));
            delete task;
            return;
        }
        HITRACE_METER_FMT(HITRACE_TAG_WINDOW_MANAGER, "Async:GetDefaultDisplay");
        sptr<Display> display = SingletonContainer::Get<DisplayManager>().GetDefaultDisplay();
        if (display != nullptr) {
            task->Resolve(env, CreateJsDisplayObject(env, display));
            TLOGD(WmsLogTag::DMS, "OnGetDefaultDisplay success");
        } else {
            task->Reject(env, CreateJsError(env,
                static_cast<int32_t>(DMError::DM_ERROR_NULLPTR), "JsDisplayManager::OnGetDefaultDisplay failed."));
        }
        delete task;
    };
    NapiSendDmsEvent(env, asyncTask, napiAsyncTask, "OnGetDefaultDisplay");
    return result;
}

napi_value OnGetPrimaryDisplaySync(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::DMS, "called");
    std::string functionName = "getPrimaryDisplaySync";
    HITRACE_METER_FMT(HITRACE_TAG_WINDOW_MANAGER, "Sync:OnGetPrimaryDisplaySync");
    sptr<Display> display = SingletonContainer::Get<DisplayManager>().GetPrimaryDisplaySync();
    if (display == nullptr) {
        TLOGE(WmsLogTag::DMS, "[NAPI]Display info is nullptr, js error will be happen");
        napi_throw(env, JsErrUtils::CreateJsError(env, DmErrorCode::DM_ERROR_INVALID_SCREEN,
            GetFormatMsg(functionName, "invalid display id")));
        return NapiGetUndefined(env);
    }
    return CreateJsDisplayObject(env, display);
}

napi_value OnGetDefaultDisplaySync(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::DMS, "called");
    std::string functionName = "getDefaultDisplaySync";
    HITRACE_METER_FMT(HITRACE_TAG_WINDOW_MANAGER, "Sync:GetDefaultDisplay");
    sptr<Display> display = SingletonContainer::Get<DisplayManager>().GetDefaultDisplaySync(true);
    if (display == nullptr) {
        TLOGE(WmsLogTag::DMS, "[NAPI]Display info is nullptr, js error will be happen");
        napi_throw(env, JsErrUtils::CreateJsError(env, DmErrorCode::DM_ERROR_INVALID_SCREEN,
            GetFormatMsg(functionName, "display is not created or destroyed")));
        return NapiGetUndefined(env);
    }
    return CreateJsDisplayObject(env, display);
}

napi_value OnGetDisplayByIdSync(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::DMS, "called");
    HITRACE_METER_FMT(HITRACE_TAG_WINDOW_MANAGER, "Sync:OnGetDisplayByIdSync");
    std::string functionName = "getDisplayByIdSync";
    size_t argc = 4;
    napi_value argv[4] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc < ARGC_ONE) {
        std::string errMsg = "Invalid args count, need one arg";
        napi_throw(env, JsErrUtils::CreateJsError(env, DmErrorCode::DM_ERROR_INVALID_PARAM,
            GetFormatMsg(functionName, errMsg)));
        return NapiGetUndefined(env);
    }
    int64_t displayId = static_cast<int64_t>(DISPLAY_ID_INVALID);
    if (!ConvertFromJsValue(env, argv[0], displayId)) {
        TLOGE(WmsLogTag::DMS, "[NAPI]Failed to convert parameter to displayId");
        std::string errMsg = "Failed to convert parameter to displayId";
        napi_throw(env, JsErrUtils::CreateJsError(env, DmErrorCode::DM_ERROR_INVALID_PARAM,
            GetFormatMsg(functionName, errMsg)));
        return NapiGetUndefined(env);
    }
    if (displayId < 0) {
        std::string errMsg = "displayid is invalid, less than 0";
        napi_throw(env, JsErrUtils::CreateJsError(env, DmErrorCode::DM_ERROR_INVALID_PARAM,
            GetFormatMsg(functionName, errMsg)));
        return NapiGetUndefined(env);
    }
    sptr<Display> display = SingletonContainer::Get<DisplayManager>().GetDisplayById(static_cast<DisplayId>(displayId));
    if (display == nullptr) {
        TLOGE(WmsLogTag::DMS, "[NAPI]Display info is nullptr, js error will be happen");
        std::ostringstream oss;
        oss << "[display][getDisplayByIdSync]message: display is null, ";
        oss << "display id " << displayId << " ";
        oss << "corresponding display does not exist.";
        napi_throw(env, JsErrUtils::CreateJsError(env, DmErrorCode::DM_ERROR_SYSTEM_INNORMAL, oss.str()));
        return NapiGetUndefined(env);
    }
    HITRACE_METER_FMT(HITRACE_TAG_WINDOW_MANAGER, "Sync:OnGetDisplayByIdSync end");
    return CreateJsDisplayObject(env, display);
}

std::string GetFormatMsg(std::string functionName, std::string errMsg = "", std::string cbType = "")
{
    if (!cbType.empty()) {
        functionName = functionName + "(" + cbType + ")";
    }
    if (errMsg.empty()) {
        return "[display][" + functionName + "]";
    }
    return "[display][" + functionName + "]msg: " + errMsg;
}

napi_value OnGetBrightnessInfoChange(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::DMS, "called");
    HITRACE_METER_FMT(HITRACE_TAG_WINDOW_MANAGER, "OnGetBrightnessInfoChange");
    std::string functionName = "getBrightnessInfo";
    size_t argc = ARGS_MAX;
    napi_value argv[ARGS_MAX] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc < ARGC_ONE) {
        std::string errMsg = "Invalid args count, need one arg";
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(DmErrorCode::DM_ERROR_ILLEGAL_PARAM), errMsg));
        return NapiGetUndefined(env);
    }
    int64_t displayId = static_cast<int64_t>(DISPLAY_ID_INVALID);
    if (!ConvertFromJsValue(env, argv[0], displayId)) {
        TLOGE(WmsLogTag::DMS, "[NAPI]Failed to convert parameter to displayId");
        std::string errMsg = "Failed to convert parameter to displayId";
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(DmErrorCode::DM_ERROR_ILLEGAL_PARAM), errMsg));
        return NapiGetUndefined(env);
    }
    ScreenBrightnessInfo brightnessInfo;
    auto errCode = SingletonContainer::Get<DisplayManager>().GetBrightnessInfo(displayId, brightnessInfo);
    if (errCode != DMError::DM_OK) {
        return NapiThrowError(env, static_cast<DmErrorCode>(errCode), "Failed to get brightness info", functionName);
    }

    return CreateJsBrightnessInfo(env, brightnessInfo);
}
 
napi_value OnGetAllDisplay(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::DMS, "called");
    DMError errCode = DMError::DM_OK;
    size_t argc = 4;
    napi_value argv[4] = {nullptr};
    std::string taskName = "OnGetAllDisplay";
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc != 0 && argc != ARGC_ONE) {
        TLOGE(WmsLogTag::DMS, "params not match");
        errCode = DMError::DM_ERROR_INVALID_PARAM;
        return NapiGetUndefined(env);
    }
    napi_value lastParam = nullptr;
    if (argc == ARGC_ONE && GetType(env, argv[0]) == napi_function) {
        lastParam = argv[0];
    }
    napi_value result = nullptr;
    std::unique_ptr<NapiAsyncTask> napiAsyncTask = CreateEmptyAsyncTask(env, lastParam, &result);
    auto asyncTask = [this, env, task = napiAsyncTask.get()]() {
        std::vector<sptr<Display>> displays = SingletonContainer::Get<DisplayManager>().GetAllDisplays();
        if (!displays.empty()) {
            task->Resolve(env, CreateJsDisplayArrayObject(env, displays));
            TLOGNI(WmsLogTag::DMS, "GetAllDisplays success");
        } else {
            task->Reject(env, CreateJsError(env,
                static_cast<int32_t>(DMError::DM_ERROR_NULLPTR), "JsDisplayManager::OnGetAllDisplay failed."));
        }
        delete task;
    };
    NapiSendDmsEvent(env, asyncTask, napiAsyncTask, taskName);
    return result;
}

void NapiSendDmsEvent(napi_env env, std::function<void()> asyncTask,
    std::unique_ptr<AbilityRuntime::NapiAsyncTask>& napiAsyncTask, std::string taskName)
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
        TLOGI(WmsLogTag::DMS, "%{public}s:success", taskName.c_str());
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

napi_value CreateJsDisplayPhysicalArrayObject(napi_env env,
    const std::vector<DisplayPhysicalResolution>& physicalArray)
{
    TLOGD(WmsLogTag::DMS, "called");
    napi_value arrayValue = nullptr;
    napi_create_array_with_length(env, physicalArray.size(), &arrayValue);
    if (arrayValue == nullptr) {
        TLOGE(WmsLogTag::DMS, "Failed to create display array");
        return NapiGetUndefined(env);
    }
    int32_t i = 0;
    for (const auto& displayItem : physicalArray) {
        napi_set_element(env, arrayValue, i++, CreateJsDisplayPhysicalInfoObject(env, displayItem));
    }
    return arrayValue;
}

napi_value OnGetAllDisplayPhysicalResolution(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::DMS, "called");
    DMError errCode = DMError::DM_OK;
    std::string functionName = "getAllDisplayPhysicalResolution";
    size_t argc = 4;
    napi_value argv[4] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc != 0 && argc != ARGC_ONE) {
        TLOGE(WmsLogTag::DMS, "params not match");
        errCode = DMError::DM_ERROR_INVALID_PARAM;
    }

    napi_value lastParam = nullptr;
    if (argc == ARGC_ONE && GetType(env, argv[0]) == napi_function) {
        lastParam = argv[0];
    }
    napi_value result = nullptr;
    std::unique_ptr<NapiAsyncTask> napiAsyncTask = CreateEmptyAsyncTask(env, lastParam, &result);
    auto asyncTask = [this, env, errCode, task = napiAsyncTask.get(), functionName] {
        if (errCode != DMError::DM_OK) {
            task->Reject(env, JsErrUtils::CreateJsError(env, errCode,
                GetFormatMsg(functionName, "paramter is invalid")));
            delete task;
            return;
        }
        std::vector<DisplayPhysicalResolution> displayPhysicalArray =
            SingletonContainer::Get<DisplayManager>().GetAllDisplayPhysicalResolution();
        if (!displayPhysicalArray.empty()) {
            task->Resolve(env, CreateJsDisplayPhysicalArrayObject(env, displayPhysicalArray));
            TLOGNI(WmsLogTag::DMS, "OnGetAllDisplayPhysicalResolution success");
        } else {
            task->Reject(env, JsErrUtils::CreateJsError(env, DmErrorCode::DM_ERROR_SYSTEM_INNORMAL,
                GetFormatMsg(functionName)));
        }
        delete task;
    };
    NapiSendDmsEvent(env, asyncTask, napiAsyncTask, "OnGetAllDisplayPhysicalResolution");
    return result;
}

napi_value OnGetAllDisplays(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::DMS, "called");
    std::string functionName = "getAllDisplays";
    size_t argc = 4;
    napi_value argv[4] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    napi_value lastParam = nullptr;
    if (argc >= ARGC_ONE && argv[ARGC_ONE - 1] != nullptr &&
        GetType(env, argv[ARGC_ONE - 1]) == napi_function) {
        lastParam = argv[0];
    }
    napi_value result = nullptr;
    std::unique_ptr<NapiAsyncTask> napiAsyncTask = CreateEmptyAsyncTask(env, lastParam, &result);
    auto asyncTask = [this, env, task = napiAsyncTask.get(), functionName] {
        std::vector<sptr<Display>> displays = SingletonContainer::Get<DisplayManager>().GetAllDisplays();
        if (!displays.empty()) {
            task->Resolve(env, CreateJsDisplayArrayObject(env, displays));
            TLOGND(WmsLogTag::DMS, "GetAllDisplays success");
        } else {
            auto errorPending = false;
            napi_is_exception_pending(env, &errorPending);
            if (errorPending) {
                napi_value exception = nullptr;
                napi_get_and_clear_last_exception(env, &exception);
            }
            task->Reject(env, JsErrUtils::CreateJsError(env, DmErrorCode::DM_ERROR_INVALID_SCREEN,
                GetFormatMsg(functionName)));
        }
        delete task;
    };
    NapiSendDmsEvent(env, asyncTask, napiAsyncTask, "OnGetDefaultDisplay");
    return result;
}

void TransReturnErrorToNew(DMError& ret)
{
    if (ret == DMError::DM_ERROR_NULLPTR || ret == DMError::DM_ERROR_INVALID_PARAM) {
        ret = DMError::DM_ERROR_ILLEGAL_PARAM;
    }
}

DMError RegisterDisplayListenerWithType(napi_env env, const std::string& type, napi_value value)
{
    if (IfCallbackRegistered(env, type, value)) {
        TLOGE(WmsLogTag::DMS, "callback already registered!");
        return DMError::DM_OK;
    }
    std::unique_ptr<NativeReference> callbackRef;
    napi_ref result = nullptr;
    napi_create_reference(env, value, 1, &result);
    callbackRef.reset(reinterpret_cast<NativeReference*>(result));
    sptr<JsDisplayListener> displayListener = new(std::nothrow) JsDisplayListener(env);
    DMError ret = DMError::DM_OK;
    if (displayListener == nullptr) {
        TLOGE(WmsLogTag::DMS, "displayListener is nullptr");
        return DMError::DM_ERROR_INVALID_PARAM;
    }
    if (type == EVENT_ADD || type == EVENT_REMOVE || type == EVENT_CHANGE) {
        ret = SingletonContainer::Get<DisplayManager>().RegisterDisplayListener(displayListener);
    } else if (type == EVENT_PRIVATE_MODE_CHANGE) {
        ret = SingletonContainer::Get<DisplayManager>().RegisterPrivateWindowListener(displayListener);
    } else if (type == EVENT_FOLD_STATUS_CHANGED) {
        ret = SingletonContainer::Get<DisplayManager>().RegisterFoldStatusListener(displayListener);
    } else if (type == EVENT_DISPLAY_MODE_CHANGED) {
        ret = SingletonContainer::Get<DisplayManager>().RegisterDisplayModeListener(displayListener);
    } else if (type == EVENT_AVAILABLE_AREA_CHANGED) {
        ret = SingletonContainer::Get<DisplayManager>().RegisterAvailableAreaListener(displayListener);
    } else if (type == EVENT_BRIGHTNESS_INFO_CHANGED) {
        ret = SingletonContainer::Get<DisplayManager>().RegisterBrightnessInfoListener(displayListener);
        TransReturnErrorToNew(ret);
    } else if (type == EVENT_FOLD_ANGLE_CHANGED) {
        ret = SingletonContainer::Get<DisplayManager>().RegisterFoldAngleListener(displayListener);
    } else if (type == EVENT_CAPTURE_STATUS_CHANGED) {
        ret = SingletonContainer::Get<DisplayManager>().RegisterCaptureStatusListener(displayListener);
    } else {
        TLOGE(WmsLogTag::DMS, "RegisterDisplayListenerWithType failed, %{public}s not support", type.c_str());
        return DMError::DM_ERROR_INVALID_PARAM;
    }
    if (ret != DMError::DM_OK) {
        TLOGE(WmsLogTag::DMS, "RegisterDisplayListenerWithType failed, ret: %{public}u", ret);
        return ret;
    }
    displayListener->AddCallback(type, value);
    jsCbMap_[type][std::move(callbackRef)] = displayListener;
    return DMError::DM_OK;
}

DMError RegisterDisplayAttributeListener(napi_env env, std::vector<std::string>& attributes, napi_value value)
{
    TLOGI(WmsLogTag::DMS, "called");
    FilterRegisteredCallback(env, attributes, value);
    if (attributes.empty()) {
        TLOGI(WmsLogTag::DMS, "All attributes callback have been already registered!");
        return DMError::DM_OK;
    }
    sptr<JsDisplayListener> displayListener = new(std::nothrow) JsDisplayListener(env);
    DMError ret = DMError::DM_OK;
    if (displayListener == nullptr) {
        TLOGE(WmsLogTag::DMS, "displayListener is nullptr");
        return DMError::DM_ERROR_INVALID_PARAM;
    }
    std::vector<std::string> attributesToServer;
    FilterRegisteredAttribute(env, attributes, attributesToServer);
    ret = SingletonContainer::Get<DisplayManager>().RegisterDisplayAttributeListener(attributesToServer,
        displayListener);
    if (ret != DMError::DM_OK) {
        TLOGE(WmsLogTag::DMS, "Register display attribute listener failed, ret: %{public}u", ret);
        return ret;
    }
    for (auto attribute: attributes) {
        std::unique_ptr<NativeReference> callbackRef;
        napi_ref result = nullptr;
        napi_create_reference(env, value, 1, &result);
        callbackRef.reset(reinterpret_cast<NativeReference*>(result));
        displayListener->AddCallback(attribute, value);
        jsAttributeCbMap_[attribute][std::move(callbackRef)] = displayListener;
    }
    return DMError::DM_OK;
}
 
void FilterRegisteredAttribute(napi_env env, std::vector<std::string>& attributes,
    std::vector<std::string>& attributesToServer)
{
    for (auto attribute : attributes) {
        auto it = jsAttributeCbMap_.find(attribute);
        if (it == jsAttributeCbMap_.end()) {
            attributesToServer.push_back(attribute);
        }
    }
}
 
void FilterRegisteredCallback(napi_env env, std::vector<std::string>& attributes, napi_value value)
{
    for (auto it = attributes.begin(); it != attributes.end();) {
        if (IsAttributeCallbackRegistered(env, *it, value)) {
            TLOGW(WmsLogTag::DMS, "Attribute: %{public}s current callback already registered!", it->c_str());
            it = attributes.erase(it);
        } else {
            ++it;
        }
    }
}
 
bool IsAttributeCallbackRegistered(napi_env env, const std::string& type, napi_value jsListenerObject)
{
    if (jsAttributeCbMap_.empty() || jsAttributeCbMap_.find(type) == jsAttributeCbMap_.end()) {
        TLOGI(WmsLogTag::DMS, "%{public}s not registered!", type.c_str());
        return false;
    }
 
    for (auto& iter : jsAttributeCbMap_[type]) {
        bool isEquals = false;
        napi_strict_equals(env, jsListenerObject, iter.first->GetNapiValue(), &isEquals);
        if (isEquals) {
            TLOGE(WmsLogTag::DMS, "callback already registered!");
            return true;
        }
    }
    return false;
}

bool IfCallbackRegistered(napi_env env, const std::string& type, napi_value jsListenerObject)
{
    if (jsCbMap_.empty() || jsCbMap_.find(type) == jsCbMap_.end()) {
        TLOGI(WmsLogTag::DMS, "%{public}s not registered!", type.c_str());
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

void UnRegisterAllAttributeListener()
{
    for (auto itAttribute = jsAttributeCbMap_.begin(); itAttribute != jsAttributeCbMap_.end();) {
        for (auto it = itAttribute->second.begin(); it != itAttribute->second.end();) {
            sptr<DisplayManager::IDisplayAttributeListener> thisListener(it->second);
            auto ret = SingletonContainer::Get<DisplayManager>().UnRegisterDisplayAttributeListener(thisListener);
            itAttribute->second.erase(it++);
            TLOGI(WmsLogTag::DMS, "attribute %{public}s  ret: %{public}u", itAttribute->first.c_str(), ret);
        }
        jsAttributeCbMap_.erase(itAttribute++);
    }
}

DMError UnregisterAllDisplayListenerWithType(const std::string& type)
{
    if (type == EVENT_CHANGE) {
        UnRegisterAllAttributeListener();
    }
    if (jsCbMap_.empty() || jsCbMap_.find(type) == jsCbMap_.end()) {
        TLOGI(WmsLogTag::DMS, "%{public}s not registered!", type.c_str());
        return DMError::DM_OK;
    }
    DMError ret = DMError::DM_OK;
    for (auto it = jsCbMap_[type].begin(); it != jsCbMap_[type].end();) {
        it->second->RemoveAllCallback();
        if (type == EVENT_ADD || type == EVENT_REMOVE || type == EVENT_CHANGE) {
            sptr<DisplayManager::IDisplayListener> thisListener(it->second);
            ret = SingletonContainer::Get<DisplayManager>().UnregisterDisplayListener(thisListener);
        } else if (type == EVENT_PRIVATE_MODE_CHANGE) {
            sptr<DisplayManager::IPrivateWindowListener> thisListener(it->second);
            ret = SingletonContainer::Get<DisplayManager>().UnregisterPrivateWindowListener(thisListener);
        } else if (type == EVENT_AVAILABLE_AREA_CHANGED) {
            sptr<DisplayManager::IAvailableAreaListener> thisListener(it->second);
            ret = SingletonContainer::Get<DisplayManager>().UnregisterAvailableAreaListener(thisListener);
        } else if (type == EVENT_BRIGHTNESS_INFO_CHANGED) {
            sptr<DisplayManager::IBrightnessInfoListener> thisListener(it->second);
            ret = SingletonContainer::Get<DisplayManager>().UnregisterBrightnessInfoListener(thisListener);
            TransReturnErrorToNew(ret);
        } else if (type == EVENT_FOLD_STATUS_CHANGED) {
            sptr<DisplayManager::IFoldStatusListener> thisListener(it->second);
            ret = SingletonContainer::Get<DisplayManager>().UnregisterFoldStatusListener(thisListener);
        } else if (type == EVENT_DISPLAY_MODE_CHANGED) {
            sptr<DisplayManager::IDisplayModeListener> thisListener(it->second);
            ret = SingletonContainer::Get<DisplayManager>().UnregisterDisplayModeListener(thisListener);
        } else if (type == EVENT_FOLD_ANGLE_CHANGED) {
            sptr<DisplayManager::IFoldAngleListener> thisListener(it->second);
            ret = SingletonContainer::Get<DisplayManager>().UnregisterFoldAngleListener(thisListener);
        } else if (type == EVENT_CAPTURE_STATUS_CHANGED) {
            sptr<DisplayManager::ICaptureStatusListener> thisListener(it->second);
            ret = SingletonContainer::Get<DisplayManager>().UnregisterCaptureStatusListener(thisListener);
        } else {
            ret = DMError::DM_ERROR_INVALID_PARAM;
        }
        jsCbMap_[type].erase(it++);
        TLOGI(WmsLogTag::DMS, "type %{public}s  ret: %{public}u", type.c_str(), ret);
    }
    jsCbMap_.erase(type);
    return ret;
}

DMError UnregBrightnessInfoListener(sptr<DisplayManager::IBrightnessInfoListener> thisListener)
{
    DMError ret = DMError::DM_OK;
    ret = SingletonContainer::Get<DisplayManager>().UnregisterBrightnessInfoListener(thisListener);
    TransReturnErrorToNew(ret);
    return ret;
}

void UnRegisterAttributeListener(napi_env env, napi_value callback)
{
    std::vector<std::string> attributesNotListened;
    for (auto itAttribute = jsAttributeCbMap_.begin(); itAttribute != jsAttributeCbMap_.end();) {
        for (auto it = itAttribute->second.begin(); it != itAttribute->second.end();) {
            bool isEquals = false;
            napi_strict_equals(env, callback, it->first->GetNapiValue(), &isEquals);
            if (isEquals) {
                it->second->RemoveCallback(env, itAttribute->first, callback);
                sptr<DisplayManager::IDisplayAttributeListener> thisListener(it->second);
                SingletonContainer::Get<DisplayManager>().UnRegisterDisplayAttributeListener(thisListener);
                it = itAttribute->second.erase(it);
                continue;
            }
            it++;
        }
        if (itAttribute->second.empty()) {
            attributesNotListened.push_back(itAttribute->first);
            itAttribute = jsAttributeCbMap_.erase(itAttribute);
            continue;
        }
        itAttribute++;
    }
    if (!attributesNotListened.empty()) {
        SingletonContainer::Get<DisplayManager>().UnRegisterDisplayAttribute(attributesNotListened);
    }
}

DMError UnRegisterDisplayListenerWithType(napi_env env, const std::string& type, napi_value value)
{
    if (type == EVENT_CHANGE) {
        UnRegisterAttributeListener(env, value);
    }
    if (jsCbMap_.empty() || jsCbMap_.find(type) == jsCbMap_.end()) {
        TLOGI(WmsLogTag::DMS, "%{public}s not registered!", type.c_str());
        return DMError::DM_OK;
    }
    DMError ret = DMError::DM_OK;
    for (auto it = jsCbMap_[type].begin(); it != jsCbMap_[type].end();) {
        bool isEquals = false;
        napi_strict_equals(env, value, it->first->GetNapiValue(), &isEquals);
        if (isEquals) {
            it->second->RemoveCallback(env, type, value);
            if (type == EVENT_ADD || type == EVENT_REMOVE || type == EVENT_CHANGE) {
                sptr<DisplayManager::IDisplayListener> thisListener(it->second);
                ret = SingletonContainer::Get<DisplayManager>().UnregisterDisplayListener(thisListener);
            } else if (type == EVENT_PRIVATE_MODE_CHANGE) {
                sptr<DisplayManager::IPrivateWindowListener> thisListener(it->second);
                ret = SingletonContainer::Get<DisplayManager>().UnregisterPrivateWindowListener(thisListener);
            } else if (type == EVENT_AVAILABLE_AREA_CHANGED) {
                sptr<DisplayManager::IAvailableAreaListener> thisListener(it->second);
                ret = SingletonContainer::Get<DisplayManager>().UnregisterAvailableAreaListener(thisListener);
            } else if (type == EVENT_BRIGHTNESS_INFO_CHANGED) {
                sptr<DisplayManager::IBrightnessInfoListener> thisListener(it->second);
                ret = UnregBrightnessInfoListener(thisListener);
            } else if (type == EVENT_FOLD_STATUS_CHANGED) {
                sptr<DisplayManager::IFoldStatusListener> thisListener(it->second);
                ret = SingletonContainer::Get<DisplayManager>().UnregisterFoldStatusListener(thisListener);
            } else if (type == EVENT_DISPLAY_MODE_CHANGED) {
                sptr<DisplayManager::IDisplayModeListener> thisListener(it->second);
                ret = SingletonContainer::Get<DisplayManager>().UnregisterDisplayModeListener(thisListener);
            } else if (type == EVENT_FOLD_ANGLE_CHANGED) {
                sptr<DisplayManager::IFoldAngleListener> thisListener(it->second);
                ret = SingletonContainer::Get<DisplayManager>().UnregisterFoldAngleListener(thisListener);
            } else if (type == EVENT_CAPTURE_STATUS_CHANGED) {
                sptr<DisplayManager::ICaptureStatusListener> thisListener(it->second);
                ret = SingletonContainer::Get<DisplayManager>().UnregisterCaptureStatusListener(thisListener);
            } else {
                ret = DMError::DM_ERROR_INVALID_PARAM;
            }
            jsCbMap_[type].erase(it++);
            TLOGI(WmsLogTag::DMS, "type %{public}s  ret: %{public}u", type.c_str(), ret);
            break;
        } else {
            it++;
        }
    }
    if (jsCbMap_[type].empty()) {
        jsCbMap_.erase(type);
    }
    return ret;
}

bool NapiIsCallable(napi_env env, napi_value value)
{
    bool result = false;
    napi_is_callable(env, value, &result);
    return result;
}

std::string GetBrightnessInfoErrorMsg(DmErrorCode errCode)
{
    auto it = brightnessInfoErrCodeMap.find(errCode);
    if (it != brightnessInfoErrCodeMap.end()) {
        return it->second;
    } else {
        return "Unknown error code.";
    }
}

napi_value OnRegisterDisplayManagerCallback(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::DMS, "OnRegisterDisplayManagerCallback is called");
    std::string functionName = "on";
    size_t argc = 4;
    napi_value argv[4] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc < ARGC_TWO) {
        TLOGE(WmsLogTag::DMS, "JsDisplayManager Params not match: %{public}zu", argc);
        napi_throw(env, JsErrUtils::CreateJsError(env, DmErrorCode::DM_ERROR_INVALID_PARAM,
            GetFormatMsg(functionName, "Invalid args count, need 2 args")));
        return NapiGetUndefined(env);
    }
    std::string cbType;
    if (!ConvertFromJsValue(env, argv[0], cbType)) {
        napi_throw(env, JsErrUtils::CreateJsError(env, DmErrorCode::DM_ERROR_INVALID_PARAM,
            GetFormatMsg(functionName, "Failed to convert parameter to callbackType", cbType)));
        TLOGE(WmsLogTag::DMS, "Failed to convert parameter to callbackType");
        return NapiGetUndefined(env);
    }
    napi_value value = argv[INDEX_ONE];
    if (value == nullptr) {
        TLOGI(WmsLogTag::DMS, "OnRegisterDisplayManagerCallback info->argv[1] is nullptr");
        napi_throw(env, JsErrUtils::CreateJsError(env, DmErrorCode::DM_ERROR_INVALID_PARAM,
            GetFormatMsg(functionName, "OnRegisterDisplayManagerCallback is nullptr", cbType)));
        return NapiGetUndefined(env);
    }
    if (!NapiIsCallable(env, value)) {
        TLOGI(WmsLogTag::DMS, "OnRegisterDisplayManagerCallback info->argv[1] is not callable");
        napi_throw(env, JsErrUtils::CreateJsError(env, DmErrorCode::DM_ERROR_INVALID_PARAM,
            GetFormatMsg(functionName, "OnRegisterDisplayManagerCallback is not callable", cbType)));
        return NapiGetUndefined(env);
    }
    std::lock_guard<std::mutex> lock(mtx_);
    DmErrorCode ret = DM_JS_TO_ERROR_CODE_MAP.at(RegisterDisplayListenerWithType(env, cbType, value));
    if (ret != DmErrorCode::DM_OK) {
        DmErrorCode errCode = DmErrorCode::DM_ERROR_INVALID_PARAM;
        std::string errMsg = "Failed to register display listener with type";
        if (ret == DmErrorCode::DM_ERROR_NOT_SYSTEM_APP) {
            errCode = ret;
        } else if (cbType == EVENT_BRIGHTNESS_INFO_CHANGED) {
            errCode = ret;
            errMsg = GetBrightnessInfoErrorMsg(errCode);
        }
        TLOGE(WmsLogTag::DMS, "Failed to register display listener with type");
        napi_throw(env, JsErrUtils::CreateJsError(env, errCode, GetFormatMsg(functionName, "", cbType)));
        return NapiGetUndefined(env);
    }
    return NapiGetUndefined(env);
}

napi_value OnUnregisterDisplayManagerCallback(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::DMS, "called");
    std::string functionName = "off";
    size_t argc = 4;
    napi_value argv[4] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc < ARGC_ONE) {
        TLOGE(WmsLogTag::DMS, "JsDisplayManager Params not match %{public}zu", argc);
        std::string errMsg = "Invalid args count, need one arg at least!";
        napi_throw(env, JsErrUtils::CreateJsError(env, DmErrorCode::DM_ERROR_INVALID_PARAM,
            GetFormatMsg(functionName, errMsg)));
        return NapiGetUndefined(env);
    }
    std::string cbType;
    if (!ConvertFromJsValue(env, argv[0], cbType)) {
        TLOGE(WmsLogTag::DMS, "Failed to convert parameter to callbackType");
        std::string errMsg = "Failed to convert parameter to string";
        napi_throw(env, JsErrUtils::CreateJsError(env, DmErrorCode::DM_ERROR_INVALID_PARAM,
            GetFormatMsg(functionName, errMsg, cbType)));
        return NapiGetUndefined(env);
    }
    std::lock_guard<std::mutex> lock(mtx_);
    DmErrorCode ret;
    if (argc == ARGC_ONE) {
        ret = DM_JS_TO_ERROR_CODE_MAP.at(UnregisterAllDisplayListenerWithType(cbType));
    } else {
        napi_value value = argv[INDEX_ONE];
        if ((value == nullptr) || (!NapiIsCallable(env, value))) {
            ret = DM_JS_TO_ERROR_CODE_MAP.at(UnregisterAllDisplayListenerWithType(cbType));
        } else {
            ret = DM_JS_TO_ERROR_CODE_MAP.at(UnRegisterDisplayListenerWithType(env, cbType, value));
        }
    }
    if (ret != DmErrorCode::DM_OK) {
        DmErrorCode errCode = DmErrorCode::DM_ERROR_INVALID_PARAM;
        if (ret == DmErrorCode::DM_ERROR_NOT_SYSTEM_APP) {
            errCode = ret;
        }
        TLOGW(WmsLogTag::DMS, "failed to unregister display listener with type");
        std::string errMsg = "failed to unregister display listener with type";
        napi_throw(env, JsErrUtils::CreateJsError(env, errCode, GetFormatMsg(functionName, "", cbType)));
        return NapiGetUndefined(env);
    }
    return NapiGetUndefined(env);
}

napi_value OnHasPrivateWindow(napi_env env, napi_callback_info info)
{
    bool hasPrivateWindow = false;
    size_t argc = 4;
    napi_value argv[4] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc < ARGC_ONE) {
        std::string errMsg = "Invalid args count, need one arg";
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(DmErrorCode::DM_ERROR_INVALID_PARAM), errMsg));
        return NapiGetUndefined(env);
    }
    int64_t displayId = static_cast<int64_t>(DISPLAY_ID_INVALID);
    if (!ConvertFromJsValue(env, argv[0], displayId)) {
        TLOGE(WmsLogTag::DMS, "[NAPI]Failed to convert parameter to displayId");
        std::string errMsg = "Failed to convert parameter to displayId";
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(DmErrorCode::DM_ERROR_INVALID_PARAM), errMsg));
        return NapiGetUndefined(env);
    }
    if (displayId < 0) {
        std::string errMsg = "displayid is invalid, less than 0";
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(DmErrorCode::DM_ERROR_INVALID_PARAM), errMsg));
        return NapiGetUndefined(env);
    }
    DmErrorCode errCode = DM_JS_TO_ERROR_CODE_MAP.at(
        SingletonContainer::Get<DisplayManager>().HasPrivateWindow(displayId, hasPrivateWindow));
    TLOGD(WmsLogTag::DMS, "[NAPI]Display id = %{public}" PRIu64", hasPrivateWindow = %{public}u err = %{public}d",
        static_cast<uint64_t>(displayId), hasPrivateWindow, errCode);
    if (errCode != DmErrorCode::DM_OK) {
        napi_throw(env, JsErrUtils::CreateJsError(env, errCode));
        return NapiGetUndefined(env);
    }
    napi_value result;
    napi_get_boolean(env, hasPrivateWindow, &result);
    return result;
}

napi_value CreateJsDisplayArrayObject(napi_env env, std::vector<sptr<Display>>& displays)
{
    TLOGD(WmsLogTag::DMS, "CreateJsDisplayArrayObject is called");
    napi_value arrayValue = nullptr;
    napi_create_array_with_length(env, displays.size(), &arrayValue);
    if (arrayValue == nullptr) {
        TLOGE(WmsLogTag::DMS, "Failed to create display array");
        return NapiGetUndefined(env);
    }
    int32_t i = 0;
    for (auto& display : displays) {
        if (display == nullptr) {
            continue;
        }
        napi_set_element(env, arrayValue, i++, CreateJsDisplayObject(env, display));
    }
    return arrayValue;
}

napi_value OnIsFoldable(napi_env env, napi_callback_info info)
{
    std::string functionName = "isFoldable";
    size_t argc = 4;
    napi_value argv[4] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc >= ARGC_ONE) {
        napi_throw(env, JsErrUtils::CreateJsError(env, DmErrorCode::DM_ERROR_INVALID_PARAM,
            GetFormatMsg(functionName, "Input parameter invalid")));
        return NapiGetUndefined(env);
    }
    bool foldable = SingletonContainer::Get<DisplayManager>().IsFoldable();
    TLOGD(WmsLogTag::DMS, "[NAPI]foldable = %{public}u", foldable);
    napi_value result;
    napi_get_boolean(env, foldable, &result);
    return result;
}

napi_value OnIsCaptured(napi_env env, napi_callback_info info)
{
    std::string functionName = "isCaptured";
    size_t argc = 4;  // default arg length
    napi_value argv[4] = { nullptr };  // default arg length
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc >= ARGC_ONE) {
        napi_throw(env, JsErrUtils::CreateJsError(env, DmErrorCode::DM_ERROR_INVALID_PARAM,
            GetFormatMsg(functionName, "Input parameter invalid")));
        return NapiGetUndefined(env);
    }
    bool isCapture = SingletonContainer::Get<DisplayManager>().IsCaptured();
    TLOGD(WmsLogTag::DMS, "[NAPI]IsCaptured = %{public}u", isCapture);
    napi_value result;
    napi_get_boolean(env, isCapture, &result);
    return result;
}

napi_value OnGetFoldStatus(napi_env env, napi_callback_info info)
{
    std::string functionName = "getFoldStatus";
    size_t argc = 4;
    napi_value argv[4] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc >= ARGC_ONE) {
        napi_throw(env, JsErrUtils::CreateJsError(env, DmErrorCode::DM_ERROR_INVALID_PARAM,
            GetFormatMsg(functionName, "Input parameter invalid")));
        return NapiGetUndefined(env);
    }
    FoldStatus status = SingletonContainer::Get<DisplayManager>().GetFoldStatus();
    TLOGD(WmsLogTag::DMS, "[NAPI]status = %{public}u", status);
    return CreateJsValue(env, status);
}

napi_value OnGetFoldDisplayMode(napi_env env, napi_callback_info info)
{
    std::string functionName = "getFoldDisplayMode";
    size_t argc = 4;
    napi_value argv[4] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc >= ARGC_ONE) {
        napi_throw(env, JsErrUtils::CreateJsError(env, DmErrorCode::DM_ERROR_INVALID_PARAM,
            GetFormatMsg(functionName, "Input parameter invalid")));
        return NapiGetUndefined(env);
    }
    FoldDisplayMode mode = SingletonContainer::Get<DisplayManager>().GetFoldDisplayModeForExternal();
    TLOGD(WmsLogTag::DMS, "[NAPI]mode = %{public}u", mode);
    return CreateJsValue(env, mode);
}

napi_value OnSetFoldDisplayMode(napi_env env, napi_callback_info info)
{
    size_t argc = 4;
    napi_value argv[4] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc < ARGC_ONE) {
        std::string errMsg = "Invalid args count, need one arg";
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(DmErrorCode::DM_ERROR_INVALID_PARAM), errMsg));
        return NapiGetUndefined(env);
    }
    FoldDisplayMode mode = FoldDisplayMode::UNKNOWN;
    if (!ConvertFromJsValue(env, argv[0], mode)) {
        TLOGE(WmsLogTag::DMS, "[NAPI]Failed to convert parameter to FoldDisplayMode");
        std::string errMsg = "Failed to convert parameter to FoldDisplayMode";
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(DmErrorCode::DM_ERROR_INVALID_PARAM), errMsg));
        return NapiGetUndefined(env);
    }
    std::string reason = "";
    if (argc == ARGC_TWO) {
        if (!ConvertFromJsValue(env, argv[INDEX_ONE], reason)) {
            TLOGE(WmsLogTag::DMS, "[NAPI]Failed to convert parameter to reason");
            std::string errMsg = "Failed to convert parameter to reason";
            napi_throw(env, CreateJsError(env, static_cast<int32_t>(DmErrorCode::DM_ERROR_INVALID_PARAM), errMsg));
            return NapiGetUndefined(env);
        }
    }
    DMError dmError = SingletonContainer::Get<DisplayManager>().SetFoldDisplayModeFromJs(mode, reason);
    std::string errMsg = "";
    if (DM_ERROR_JS_TO_ERROR_MESSAGE_MAP.find(dmError) != DM_ERROR_JS_TO_ERROR_MESSAGE_MAP.end()) {
        errMsg = DM_ERROR_JS_TO_ERROR_MESSAGE_MAP.at(dmError);
    }
    DmErrorCode errCode = DmErrorCode::DM_ERROR_SYSTEM_INNORMAL;
    if (DM_JS_TO_ERROR_CODE_MAP.find(dmError) != DM_JS_TO_ERROR_CODE_MAP.end()) {
        errCode = DM_JS_TO_ERROR_CODE_MAP.at(dmError);
    }
    TLOGI(WmsLogTag::DMS, "[NAPI]%{public}d, error message: %{public}s", static_cast<int32_t>(errCode), errMsg.c_str());
    if (errCode != DmErrorCode::DM_OK) {
        napi_throw(env, JsErrUtils::CreateJsError(env, errCode, errMsg));
        return NapiGetUndefined(env);
    }
    return NapiGetUndefined(env);
}

napi_value OnSetFoldStatusLocked(napi_env env, napi_callback_info info)
{
    size_t argc = 4;
    napi_value argv[4] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc < ARGC_ONE) {
        std::string errMsg = "Invalid args count, need one arg";
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(DmErrorCode::DM_ERROR_INVALID_PARAM), errMsg));
        return NapiGetUndefined(env);
    }
    bool locked = false;
    if (!ConvertFromJsValue(env, argv[0], locked)) {
        TLOGE(WmsLogTag::DMS, "[NAPI]Failed to convert parameter to SetFoldStatusLocked");
        std::string errMsg = "Failed to convert parameter to SetFoldStatusLocked";
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(DmErrorCode::DM_ERROR_INVALID_PARAM), errMsg));
        return NapiGetUndefined(env);
    }
    DmErrorCode errCode = DM_JS_TO_ERROR_CODE_MAP.at(
        SingletonContainer::Get<DisplayManager>().SetFoldStatusLockedFromJs(locked));
    if (errCode != DmErrorCode::DM_OK) {
        napi_throw(env, JsErrUtils::CreateJsError(env, errCode));
        return NapiGetUndefined(env);
    }
    TLOGI(WmsLogTag::DMS, "[NAPI]locked: %{public}d", locked);
    return NapiGetUndefined(env);
}

napi_value OnGetCurrentFoldCreaseRegion(napi_env env, napi_callback_info info)
{
    std::string functionName = "getCurrentFoldCreaseRegion";
    size_t argc = 4;
    napi_value argv[4] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc >= ARGC_ONE) {
        napi_throw(env, JsErrUtils::CreateJsError(env, DmErrorCode::DM_ERROR_INVALID_PARAM,
            GetFormatMsg(functionName, "Input parameter invalid")));
        return NapiGetUndefined(env);
    }
    sptr<FoldCreaseRegion> region = SingletonContainer::Get<DisplayManager>().GetCurrentFoldCreaseRegion();
    return CreateJsFoldCreaseRegionObject(env, region);
}

napi_value CreateJsFoldCreaseRegionObject(napi_env env, sptr<FoldCreaseRegion> region)
{
    TLOGI(WmsLogTag::DMS, "called");
    napi_value objValue = nullptr;
    napi_create_object(env, &objValue);
    if (objValue == nullptr) {
        TLOGE(WmsLogTag::DMS, "Failed to convert prop to jsObject");
        return NapiGetUndefined(env);
    }
    if (region == nullptr) {
        TLOGW(WmsLogTag::DMS, "region is nullptr");
        return NapiGetUndefined(env);
    }
    DisplayId displayId = region->GetDisplayId();
    std::vector<DMRect> creaseRects = region->GetCreaseRects();
    napi_set_named_property(env, objValue, "displayId", CreateJsValue(env, static_cast<uint32_t>(displayId)));
    napi_set_named_property(env, objValue, "creaseRects", CreateJsCreaseRectsArrayObject(env, creaseRects));
    return objValue;
}

napi_value CreateJsCreaseRectsArrayObject(napi_env env, std::vector<DMRect> creaseRects)
{
    napi_value arrayValue = nullptr;
    napi_create_array_with_length(env, creaseRects.size(), &arrayValue);
    size_t i = 0;
    for (const auto& rect : creaseRects) {
        napi_set_element(env, arrayValue, i++, CreateJsRectObject(env, rect));
    }
    return arrayValue;
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
        return NapiThrowError(env, DmErrorCode::DM_ERROR_INVALID_PARAM, errMsg, "createVirtualScreen");
    }
    napi_value lastParam = nullptr;
    if (argc >= ARGC_TWO && argv[ARGC_TWO - 1] != nullptr &&
        GetType(env, argv[ARGC_TWO - 1]) == napi_function) {
        lastParam = argv[ARGC_TWO - 1];
    }
    napi_value result = nullptr;
    std::unique_ptr<NapiAsyncTask> napiAsyncTask = CreateEmptyAsyncTask(env, lastParam, &result);
    auto asyncTask = [option, env, task = napiAsyncTask.get()]() {
        HITRACE_METER_FMT(HITRACE_TAG_WINDOW_MANAGER, "JsDisplayManager::OnCreateVirtualScreen");
        auto screenId = SingletonContainer::Get<ScreenManager>().CreateVirtualScreen(option);
        auto screen = SingletonContainer::Get<ScreenManager>().GetScreenById(screenId);
        if (screen == nullptr) {
            DmErrorCode ret = DmErrorCode::DM_ERROR_INVALID_SCREEN;
            TLOGNI(WmsLogTag::DMS, "screenId is %{public}" PRIu64"", screenId);
            if (screenId == ERROR_ID_NOT_SYSTEM_APP) {
                ret = DmErrorCode::DM_ERROR_NO_PERMISSION;
            } else if (screenId == ERROR_ID_NO_PERMISSION) {
                ret =  DmErrorCode::DM_ERROR_NO_PERMISSION;
            }
            task->Reject(env, JsErrUtils::CreateJsError(env, ret, "[display][createVirtualScreen]"));
            TLOGNE(WmsLogTag::DMS, "JsDisplayManager::CreateVirtualScreen failed.");
        } else {
            task->Resolve(env, CreateJsValue(env, static_cast<int64_t>(screenId)));
            TLOGNI(WmsLogTag::DMS, "JsDisplayManager::OnCreateVirtualScreen success");
        }
        delete task;
    };
    NapiSendDmsEvent(env, asyncTask, napiAsyncTask, "OnCreateVirtualScreen");
    return result;
}

napi_value OnMakeUnique(napi_env env, napi_callback_info info)
{
    TLOGI(WmsLogTag::DMS, "called");
    std::string functionName = "makeUnique";
    size_t argc = 4;
    int64_t screenId = -1LL;
    DmErrorCode errCode = DmErrorCode::DM_OK;
    std::string errMsg = "";
    napi_value argv[4] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc < ARGC_ONE) {
        TLOGE(WmsLogTag::DMS, "[NAPI]Argc is invalid: %{public}zu", argc);
        errMsg = "Invalid args count, need one arg at least!";
        errCode = DmErrorCode::DM_ERROR_INVALID_PARAM;
    } else {
        if (!ConvertFromJsValue(env, argv[0], screenId) || static_cast<int32_t>(screenId) < 0) {
            TLOGE(WmsLogTag::DMS, "Failed to convert parameter to screen id.");
            errMsg = "Failed to convert parameter to screen id.";
            errCode = DmErrorCode::DM_ERROR_INVALID_PARAM;
        }
    }
    if (errCode == DmErrorCode::DM_ERROR_INVALID_PARAM || screenId == -1LL) {
        TLOGE(WmsLogTag::DMS, "JsDisplayManager::OnMakeUnique failed, Invalidate params.");
        return NapiThrowError(env, DmErrorCode::DM_ERROR_INVALID_PARAM, errMsg, functionName);
    }
    std::vector<ScreenId> screenIds;
    screenIds.emplace_back(static_cast<ScreenId>(screenId));
    napi_value lastParam = nullptr;
    napi_value result = nullptr;
    std::unique_ptr<NapiAsyncTask> napiAsyncTask = CreateEmptyAsyncTask(env, lastParam, &result);
    auto asyncTask = [this, screenIds, env, task = napiAsyncTask.get(), functionName]() {
        HITRACE_METER_FMT(HITRACE_TAG_WINDOW_MANAGER, "JsDisplayManager::OnMakeUnique");
        std::vector<DisplayId> displayIds;
        DmErrorCode ret = DM_JS_TO_ERROR_CODE_MAP.at(
            SingletonContainer::Get<ScreenManager>().MakeUniqueScreen(screenIds, displayIds));
        ret = (ret == DmErrorCode::DM_ERROR_NOT_SYSTEM_APP) ? DmErrorCode::DM_ERROR_NO_PERMISSION : ret;
        if (ret == DmErrorCode::DM_OK) {
            task->Resolve(env, NapiGetUndefined(env));
            TLOGNI(WmsLogTag::DMS, "makeUnique success");
        } else {
            task->Reject(env,
                JsErrUtils::CreateJsError(env, ret, GetFormatMsg(functionName)));
        }
        delete task;
    };
    NapiSendDmsEvent(env, asyncTask, napiAsyncTask, "OnMakeUnique");
    return result;
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
        TLOGE(WmsLogTag::DMS, "Invalidate params.");
        return NapiThrowError(env, DmErrorCode::DM_ERROR_INVALID_PARAM, errMsg, "destroyVirtualScreen");
    }
    napi_value lastParam = nullptr;
    if (argc >= ARGC_TWO && argv[ARGC_TWO - 1] != nullptr &&
        GetType(env, argv[ARGC_TWO - 1]) == napi_function) {
        lastParam = argv[ARGC_TWO - 1];
    }
    napi_value result = nullptr;
    std::unique_ptr<NapiAsyncTask> napiAsyncTask = CreateEmptyAsyncTask(env, lastParam, &result);
    auto asyncTask = [screenId, env, task = napiAsyncTask.get()]() {
        HITRACE_METER_FMT(HITRACE_TAG_WINDOW_MANAGER, "JsDisplayManager::OnDestroyVirtualScreen");
        auto res = DM_JS_TO_ERROR_CODE_MAP.at(
            SingletonContainer::Get<ScreenManager>().DestroyVirtualScreen(screenId));
        res = (res == DmErrorCode::DM_ERROR_NOT_SYSTEM_APP) ? DmErrorCode::DM_ERROR_NO_PERMISSION : res;
        if (res != DmErrorCode::DM_OK) {
            task->Reject(env, JsErrUtils::CreateJsError(env, res, "[display][destroyVirtualScreen]"));
            TLOGNE(WmsLogTag::DMS, "JsDisplayManager::DestroyVirtualScreen failed.");
            delete task;
            return;
        }
        task->Resolve(env, NapiGetUndefined(env));
        TLOGNI(WmsLogTag::DMS, "JsDisplayManager::OnDestroyVirtualScreen success");
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
        return NapiThrowError(env, DmErrorCode::DM_ERROR_INVALID_PARAM, errMsg, "setVirtualScreenSurface");
    }
    napi_value lastParam = nullptr;
    if (argc >= ARGC_THREE && argv[ARGC_THREE - 1] != nullptr &&
        GetType(env, argv[ARGC_THREE - 1]) == napi_function) {
        lastParam = argv[ARGC_THREE - 1];
    }
    napi_value result = nullptr;
    std::unique_ptr<NapiAsyncTask> napiAsyncTask = CreateEmptyAsyncTask(env, lastParam, &result);
    auto asyncTask = [screenId, surface, env, task = napiAsyncTask.get()]() {
        HITRACE_METER_FMT(HITRACE_TAG_WINDOW_MANAGER, "JsDisplayManager::OnSetVirtualScreenSurface");
        auto res = DM_JS_TO_ERROR_CODE_MAP.at(
            SingletonContainer::Get<ScreenManager>().SetVirtualScreenSurface(screenId, surface));
        res = (res == DmErrorCode::DM_ERROR_NOT_SYSTEM_APP) ? DmErrorCode::DM_ERROR_NO_PERMISSION : res;
        if (res != DmErrorCode::DM_OK) {
            task->Reject(env, JsErrUtils::CreateJsError(env, res, "[display][setVirtualScreenSurface]"));
            TLOGNE(WmsLogTag::DMS, "JsDisplayManager::SetVirtualScreenSurface failed.");
        } else {
            task->Resolve(env, NapiGetUndefined(env));
        }
        delete task;
    };
    NapiSendDmsEvent(env, asyncTask, napiAsyncTask, "OnSetVirtualScreenSurface");
    return result;
}

napi_value OnAddVirtualScreenBlockList(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::DMS, "in");
    std::string functionName = "addVirtualScreenBlockList";
    size_t argc = ARGC_THREE;
    napi_value argv[ARGC_THREE] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc < ARGC_ONE) {
        TLOGE(WmsLogTag::DMS, "[NAPI]Argc is invalid: %{public}zu", argc);
        return NapiThrowError(
            env, DmErrorCode::DM_ERROR_INVALID_PARAM, "Invalid parameter count", functionName);
    }
    napi_value nativeArray = argv[0];
    uint32_t size = 0;
    if (GetType(env, nativeArray) != napi_object ||
        napi_get_array_length(env, nativeArray, &size) == napi_invalid_arg) {
            return NapiThrowError(env, DmErrorCode::DM_ERROR_INVALID_PARAM,
                "Failed to convert parameter to black list array", functionName);
    }
    std::vector<int32_t> persistentIds;
    for (uint32_t i = 0; i < size; i++) {
        int32_t persistentId = 0;
        napi_value element = nullptr;
        napi_get_element(env, nativeArray, i, &element);
        if (!ConvertFromJsValue(env, element, persistentId)) {
            return NapiThrowError(env, DmErrorCode::DM_ERROR_INVALID_PARAM,
                "Failed to convert parameter to persistent id", functionName);
        }
        persistentIds.push_back(persistentId);
    }
    napi_value result = nullptr;
    std::unique_ptr<NapiAsyncTask> napiAsyncTask = CreateEmptyAsyncTask(env, nullptr, &result);
    auto asyncTask = [persistentIds, env, task = napiAsyncTask.get()] {
        if (persistentIds.size() == 0) {
            TLOGND(WmsLogTag::DMS, "RemoveVirtualScreenBlockList: persistentIds size is 0, no need update");
            task->Resolve(env, NapiGetUndefined(env));
            return;
        }
        auto res = DM_JS_TO_ERROR_CODE_MAP.at(
            SingletonContainer::Get<ScreenManager>().AddVirtualScreenBlockList(persistentIds));
        if (res != DmErrorCode::DM_OK) {
            TLOGNE(WmsLogTag::DMS, "failed");
            task->Reject(env, CreateJsError(env, static_cast<int32_t>(res), "add black list failed"));
        } else {
            task->Resolve(env, NapiGetUndefined(env));
        }
        delete task;
    };
    NapiSendDmsEvent(env, asyncTask, napiAsyncTask, "AddVirtualScreenBlockList");
    return result;
}

napi_value OnRemoveVirtualScreenBlockList(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::DMS, "in");
    std::string functionName = "removeVirtualScreenBlockList";
    size_t argc = ARGC_THREE;
    napi_value argv[ARGC_THREE] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc < ARGC_ONE) {
        TLOGE(WmsLogTag::DMS, "[NAPI]Argc is invalid: %{public}zu", argc);
        return NapiThrowError(env, DmErrorCode::DM_ERROR_INVALID_PARAM, "Invalid parameter count", functionName);
    }
    napi_value nativeArray = argv[0];
    uint32_t size = 0;
    if (GetType(env, nativeArray) != napi_object ||
        napi_get_array_length(env, nativeArray, &size) == napi_invalid_arg) {
            return NapiThrowError(env, DmErrorCode::DM_ERROR_INVALID_PARAM,
                "Failed to convert parameter to black list array", functionName);
    }
    std::vector<int32_t> persistentIds;
    for (uint32_t i = 0; i < size; i++) {
        int32_t persistentId = 0;
        napi_value element = nullptr;
        napi_get_element(env, nativeArray, i, &element);
        if (!ConvertFromJsValue(env, element, persistentId)) {
            return NapiThrowError(env, DmErrorCode::DM_ERROR_INVALID_PARAM,
                "Failed to convert parameter to persistent id", functionName);
        }
        persistentIds.push_back(persistentId);
    }
    napi_value result = nullptr;
    std::unique_ptr<NapiAsyncTask> napiAsyncTask = CreateEmptyAsyncTask(env, nullptr, &result);
    auto asyncTask = [persistentIds, env, task = napiAsyncTask.get()] {
        if (persistentIds.size() == 0) {
            TLOGND(WmsLogTag::DMS, "RemoveVirtualScreenBlockList: persistentIds size is 0, no need update");
            task->Resolve(env, NapiGetUndefined(env));
            return;
        }
        auto res = DM_JS_TO_ERROR_CODE_MAP.at(
            SingletonContainer::Get<ScreenManager>().RemoveVirtualScreenBlockList(persistentIds));
        if (res != DmErrorCode::DM_OK) {
            TLOGNE(WmsLogTag::DMS, "failed");
            task->Reject(env, CreateJsError(env, static_cast<int32_t>(res), "remove black list failed"));
        } else {
            task->Resolve(env, NapiGetUndefined(env));
        }
        delete task;
    };
    NapiSendDmsEvent(env, asyncTask, napiAsyncTask, "RemoveVirtualScreenBlockList");
    return result;
}

napi_value OnConvertGlobalCoordinateToRelative(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::DMS, "in");
    Position globalPosition;
    int64_t displayIdTemp = 0;
    std::string functionName = "convertGlobalToRelativeCoordinate";
    size_t argc = ARGC_TWO;
    napi_value argv[ARGC_TWO] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc < ARGC_ONE) {
        return NapiThrowError(env, DmErrorCode::DM_ERROR_ILLEGAL_PARAM,
            "Invalid args count, need one arg at least!", functionName);
    }
    if (argv[0] == nullptr) {
        return NapiThrowError(env, DmErrorCode::DM_ERROR_ILLEGAL_PARAM,
            "Failed to get globalPosition, globalPosition is nullptr", functionName);
    }
    if (argc == ARGC_TWO) {
        if (!ConvertFromJsValue(env, argv[1], displayIdTemp)) {
            return NapiThrowError(env, DmErrorCode::DM_ERROR_ILLEGAL_PARAM,
                "Failed to convert displayIdObject to displayId.", functionName);
        }
        if (displayIdTemp < 0) {
            return NapiThrowError(env, DmErrorCode::DM_ERROR_ILLEGAL_PARAM, "displayId less than 0.", functionName);
        }
    }
    DmErrorCode errCode = DmErrorCode::DM_OK;
    errCode = GetPositionFromJs(env, argv[0], globalPosition);
    if (errCode != DmErrorCode::DM_OK) {
        return NapiThrowError(env, errCode, "Get position from js failed.", functionName);
    }
    RelativePosition relativePosition;
    if (argc == ARGC_TWO) {
        errCode = DM_JS_TO_ERROR_CODE_MAP.at(
            SingletonContainer::Get<DisplayManager>().ConvertGlobalCoordinateToRelativeWithDisplayId(globalPosition,
                static_cast<DisplayId>(displayIdTemp), relativePosition));
    } else {
        errCode = DM_JS_TO_ERROR_CODE_MAP.at(
            SingletonContainer::Get<DisplayManager>().ConvertGlobalCoordinateToRelative(globalPosition,
                relativePosition));
    }
    if (errCode != DmErrorCode::DM_OK) {
        return NapiThrowError(env, errCode, "", functionName);
    }
    return CreateJsRelativePositionObject(env, relativePosition);
}

napi_value CreateJsRelativePositionObject(napi_env env, RelativePosition& relativePosition)
{
    TLOGD(WmsLogTag::DMS, "called");
    napi_value objValue = nullptr;
    napi_create_object(env, &objValue);
    if (objValue == nullptr) {
        TLOGE(WmsLogTag::DMS, "Failed to get object");
        return NapiGetUndefined(env);
    }
    napi_set_named_property(env, objValue, "displayId",
        CreateJsValue(env, static_cast<int64_t>(relativePosition.displayId)));
    napi_set_named_property(env, objValue, "position",
        CreateJsGlobalPositionObject(env, relativePosition.position));
    return objValue;
}

napi_value OnConvertRelativeCoordinateToGlobal(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::DMS, "in");
    DmErrorCode errCode = DmErrorCode::DM_OK;
    RelativePosition relativePosition;
    std::string functionName = "convertRelativeToGlobalCoordinate";
    std::string errMsg;
    size_t argc = ARGC_ONE;
    napi_value argv[ARGC_ONE] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc < ARGC_ONE) {
        return NapiThrowError(env, DmErrorCode::DM_ERROR_ILLEGAL_PARAM,
            "Invalid args count, need one arg at least!", functionName);
    }

    errCode = GetRelativePositionFromJs(env, argv[0], relativePosition);
    if (errCode != DmErrorCode::DM_OK) {
        return NapiThrowError(env, errCode, "Failed to get relativePosition!", functionName);
    }
    Position globalPosition;
    errCode = DM_JS_TO_ERROR_CODE_MAP.at(
        SingletonContainer::Get<DisplayManager>().ConvertRelativeCoordinateToGlobal(relativePosition, globalPosition));
    if (errCode != DmErrorCode::DM_OK) {
        return NapiThrowError(env, errCode, "", functionName);
    }
    return CreateJsGlobalPositionObject(env, globalPosition);
}

napi_value CreateJsGlobalPositionObject(napi_env env, Position& globalPosition)
{
    TLOGD(WmsLogTag::DMS, "called");
    napi_value objValue = nullptr;
    napi_create_object(env, &objValue);
    if (objValue == nullptr) {
        TLOGE(WmsLogTag::DMS, "Failed to get object");
        return NapiGetUndefined(env);
    }
    napi_set_named_property(env, objValue, "x", CreateJsValue(env, globalPosition.x));
    napi_set_named_property(env, objValue, "y", CreateJsValue(env, globalPosition.y));
    return objValue;
}

DmErrorCode GetRelativePositionFromJs(napi_env env, napi_value relativePositionObject,
    RelativePosition& relativePosition)
{
    TLOGD(WmsLogTag::DMS, "called");
    if (relativePositionObject == nullptr) {
        TLOGE(WmsLogTag::DMS, "Failed to get relativePosition, relativePosition is nullptr.");
        return DmErrorCode::DM_ERROR_ILLEGAL_PARAM;
    }
    napi_value displayId = nullptr;
    napi_get_named_property(env, relativePositionObject, "displayId", &displayId);
    int64_t displayIdTemp = 0;
    if (!ConvertFromJsValue(env, displayId, displayIdTemp)) {
        TLOGE(WmsLogTag::DMS, "Failed to convert parameter to displayId.");
        return DmErrorCode::DM_ERROR_ILLEGAL_PARAM;
    }
    if (displayIdTemp < 0) {
        TLOGE(WmsLogTag::DMS, "DisplayId is invalid, less than 0.");
        return DmErrorCode::DM_ERROR_ILLEGAL_PARAM;
    }
    relativePosition.displayId = static_cast<uint64_t>(displayIdTemp);
    napi_value position = nullptr;
    napi_get_named_property(env, relativePositionObject, "position", &position);
    return GetPositionFromJs(env, position, relativePosition.position);
}

DmErrorCode GetPositionFromJs(napi_env env, napi_value positionObject, Position& position)
{
    TLOGD(WmsLogTag::DMS, "called");
    napi_value positionX = nullptr;
    double positionXTemp = 0;
    napi_get_named_property(env, positionObject, "x", &positionX);
    if (!ConvertFromJsValue(env, positionX, positionXTemp)) {
        TLOGE(WmsLogTag::DMS, "Failed to convert parameter to positionX.");
        return DmErrorCode::DM_ERROR_ILLEGAL_PARAM;
    }
    if (positionXTemp < INT32_MIN || positionXTemp > INT32_MAX) {
        TLOGE(WmsLogTag::DMS, "Coordinate x exceeded the range!");
        return DmErrorCode::DM_ERROR_ILLEGAL_PARAM;
    }

    napi_value positionY = nullptr;
    double positionYTemp = 0;
    napi_get_named_property(env, positionObject, "y", &positionY);
    if (!ConvertFromJsValue(env, positionY, positionYTemp)) {
        TLOGE(WmsLogTag::DMS, "Failed to convert parameter to positionY.");
        return DmErrorCode::DM_ERROR_ILLEGAL_PARAM;
    }
    if (positionYTemp < INT32_MIN || positionYTemp > INT32_MAX) {
        TLOGE(WmsLogTag::DMS, "Coordinate y exceeded the range!");
        return DmErrorCode::DM_ERROR_ILLEGAL_PARAM;
    }

    position.x = static_cast<int32_t>(positionXTemp);
    position.y = static_cast<int32_t>(positionYTemp);
    return DmErrorCode::DM_OK;
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
    option.supportsInput_ = false;
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

napi_value OnRegisterDisplayAttributeChangeCallback(napi_env env, napi_callback_info info)
{
    TLOGI(WmsLogTag::DMS, "called");
    size_t argc = ARGC_TWO;
    napi_value argv[ARGC_TWO] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc < ARGC_TWO) {
        TLOGE(WmsLogTag::DMS, "Invalid arg count: %{public}zu, need two args", argc);
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(DmErrorCode::DM_ERROR_INVALID_PARAM),
            "Invalid args count, need two args"));
        return NapiGetUndefined(env);
    }
    std::vector<std::string> attributes;
    if (!GetAttributesFromJs(env, argv[0], attributes)) {
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(DmErrorCode::DM_ERROR_INVALID_PARAM),
            "Failed to convert parameter to attributes"));
        TLOGE(WmsLogTag::DMS, "Failed to convert parameter to attributes");
        return NapiGetUndefined(env);
    }
    FilterValidAttributes(attributes);
 
    napi_value value = argv[INDEX_ONE];
    if (value == nullptr) {
        TLOGE(WmsLogTag::DMS, "info->argv[1] is nullptr");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(DmErrorCode::DM_ERROR_INVALID_PARAM),
            "OnRegisterDisplayAttributeChangeCallback: callback is nullptr"));
        return NapiGetUndefined(env);
    }
    if (!NapiIsCallable(env, value)) {
        TLOGE(WmsLogTag::DMS, "info->argv[1] is not callable");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(DmErrorCode::DM_ERROR_INVALID_PARAM),
            "OnRegisterDisplayAttributeChangeCallback: callback is not callable"));
        return NapiGetUndefined(env);
    }
    DmErrorCode ret = DM_JS_TO_ERROR_CODE_MAP.at(RegisterDisplayAttributeListener(env, attributes, value));
    if (ret != DmErrorCode::DM_OK) {
        TLOGE(WmsLogTag::DMS, "Failed to register display attribute listener");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(ret), "Failed to register display attribute listener"));
        return NapiGetUndefined(env);
    }
    return NapiGetUndefined(env);
}

void FilterValidAttributes(std::vector<std::string>& attributes)
{
    static const std::set<std::string> validAttributes = {
        "id", "name", "alive", "state", "refreshRate",
        "rotation", "width", "height",
        "densityDPI", "densityPixels", "scaledDensity",
        "orientation", "xDPI", "yDPI", "colorSpaces",
        "hdrFormats", "availableWidth", "availableHeight",
        "x", "y", "screenShape", "sourceMode", "supportedRefreshRates"
    };
    
    attributes.erase(std::remove_if(attributes.begin(), attributes.end(),
        [](const std::string& attr) {
            bool invalid = validAttributes.find(attr) == validAttributes.end();
            if (invalid) {
                TLOGW(WmsLogTag::DMS, "Invalid attribute name: %{public}s", attr.c_str());
            }
            return invalid;
        }),
        attributes.end()
    );
}
 
bool GetAttributesFromJs(napi_env env, napi_value attributeArray, std::vector<std::string>& attributes)
{
    TLOGI(WmsLogTag::DMS, "called");
    if (attributeArray == nullptr || GetType(env, attributeArray) != napi_object) {
        TLOGE(WmsLogTag::DMS, "Failed to convert parameter to attribute, invalid params.");
        return false;
    }
    uint32_t size = 0;
    if (napi_get_array_length(env, attributeArray, &size) == napi_invalid_arg) {
        TLOGE(WmsLogTag::DMS, "Failed to get attribute array length.");
        return false;
    }
    for (uint32_t i = 0; i < size; i++) {
        std::string attribute;
        napi_value element = nullptr;
        napi_get_element(env, attributeArray, i, &element);
        if (!ConvertFromJsValue(env, element, attribute)) {
            TLOGE(WmsLogTag::DMS, "Failed to convert parameter to attribute");
            return false;
        }
        attributes.push_back(attribute);
    }
    return true;
}

napi_value NapiThrowError(napi_env env, DmErrorCode errCode, std::string msg = "", std::string functionName = "")
{
    napi_throw(env, JsErrUtils::CreateJsError(env, errCode, GetFormatMsg(functionName, msg)));
    return NapiGetUndefined(env);
}
};

napi_value InitDisplayState(napi_env env)
{
    TLOGD(WmsLogTag::DMS, "InitDisplayState called");

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
    napi_set_named_property(env, objValue, "STATE_UNKNOWN",
        CreateJsValue(env, static_cast<int32_t>(DisplayStateMode::STATE_UNKNOWN)));
    napi_set_named_property(env, objValue, "STATE_OFF",
        CreateJsValue(env, static_cast<int32_t>(DisplayStateMode::STATE_OFF)));
    napi_set_named_property(env, objValue, "STATE_ON",
        CreateJsValue(env, static_cast<int32_t>(DisplayStateMode::STATE_ON)));
    napi_set_named_property(env, objValue, "STATE_DOZE",
        CreateJsValue(env, static_cast<int32_t>(DisplayStateMode::STATE_DOZE)));
    napi_set_named_property(env, objValue, "STATE_DOZE_SUSPEND",
        CreateJsValue(env, static_cast<int32_t>(DisplayStateMode::STATE_DOZE_SUSPEND)));
    napi_set_named_property(env, objValue, "STATE_VR",
        CreateJsValue(env, static_cast<int32_t>(DisplayStateMode::STATE_VR)));
    napi_set_named_property(env, objValue, "STATE_ON_SUSPEND",
        CreateJsValue(env, static_cast<int32_t>(DisplayStateMode::STATE_ON_SUSPEND)));
    return objValue;
}

napi_value InitOrientation(napi_env env)
{
    TLOGD(WmsLogTag::DMS, "InitOrientation called");

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

    napi_set_named_property(env, objValue, "PORTRAIT",
        CreateJsValue(env, static_cast<uint32_t>(DisplayOrientation::PORTRAIT)));
    napi_set_named_property(env, objValue, "LANDSCAPE",
        CreateJsValue(env, static_cast<uint32_t>(DisplayOrientation::LANDSCAPE)));
    napi_set_named_property(env, objValue, "PORTRAIT_INVERTED",
        CreateJsValue(env, static_cast<uint32_t>(DisplayOrientation::PORTRAIT_INVERTED)));
    napi_set_named_property(env, objValue, "LANDSCAPE_INVERTED",
        CreateJsValue(env, static_cast<uint32_t>(DisplayOrientation::LANDSCAPE_INVERTED)));
    return objValue;
}

napi_value InitDisplayErrorCode(napi_env env)
{
    TLOGD(WmsLogTag::DMS, "InitDisplayErrorCode called");

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
    TLOGD(WmsLogTag::DMS, "InitDisplayError called");

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

napi_value InitFoldStatus(napi_env env)
{
    TLOGD(WmsLogTag::DMS, "InitFoldStatus called");

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
    napi_set_named_property(env, objValue, "FOLD_STATUS_UNKNOWN",
        CreateJsValue(env, static_cast<uint32_t>(FoldStatus::UNKNOWN)));
    napi_set_named_property(env, objValue, "FOLD_STATUS_EXPANDED",
        CreateJsValue(env, static_cast<uint32_t>(FoldStatus::EXPAND)));
    napi_set_named_property(env, objValue, "FOLD_STATUS_FOLDED",
        CreateJsValue(env, static_cast<uint32_t>(FoldStatus::FOLDED)));
    napi_set_named_property(env, objValue, "FOLD_STATUS_HALF_FOLDED",
        CreateJsValue(env, static_cast<uint32_t>(FoldStatus::HALF_FOLD)));
    napi_set_named_property(env, objValue, "FOLD_STATUS_EXPANDED_WITH_SECOND_EXPANDED",
        CreateJsValue(env, static_cast<uint32_t>(FoldStatus::FOLD_STATE_EXPAND_WITH_SECOND_EXPAND)));
    napi_set_named_property(env, objValue, "FOLD_STATUS_EXPANDED_WITH_SECOND_HALF_FOLDED",
        CreateJsValue(env, static_cast<uint32_t>(FoldStatus::FOLD_STATE_EXPAND_WITH_SECOND_HALF_FOLDED)));
    napi_set_named_property(env, objValue, "FOLD_STATUS_FOLDED_WITH_SECOND_EXPANDED",
        CreateJsValue(env, static_cast<uint32_t>(FoldStatus::FOLD_STATE_FOLDED_WITH_SECOND_EXPAND)));
    napi_set_named_property(env, objValue, "FOLD_STATUS_FOLDED_WITH_SECOND_HALF_FOLDED",
        CreateJsValue(env, static_cast<uint32_t>(FoldStatus::FOLD_STATE_FOLDED_WITH_SECOND_HALF_FOLDED)));
    napi_set_named_property(env, objValue, "FOLD_STATUS_HALF_FOLDED_WITH_SECOND_EXPANDED",
        CreateJsValue(env, static_cast<uint32_t>(FoldStatus::FOLD_STATE_HALF_FOLDED_WITH_SECOND_EXPAND)));
    napi_set_named_property(env, objValue, "FOLD_STATUS_HALF_FOLDED_WITH_SECOND_HALF_FOLDED",
        CreateJsValue(env, static_cast<uint32_t>(FoldStatus::FOLD_STATE_HALF_FOLDED_WITH_SECOND_HALF_FOLDED)));
    return objValue;
}

napi_value InitFoldDisplayMode(napi_env env)
{
    TLOGD(WmsLogTag::DMS, "IniFoldDisplayMode called");

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

    napi_set_named_property(env, objValue, "FOLD_DISPLAY_MODE_UNKNOWN",
        CreateJsValue(env, static_cast<uint32_t>(FoldDisplayMode::UNKNOWN)));
    napi_set_named_property(env, objValue, "FOLD_DISPLAY_MODE_FULL",
        CreateJsValue(env, static_cast<uint32_t>(FoldDisplayMode::FULL)));
    napi_set_named_property(env, objValue, "FOLD_DISPLAY_MODE_MAIN",
        CreateJsValue(env, static_cast<uint32_t>(FoldDisplayMode::MAIN)));
    napi_set_named_property(env, objValue, "FOLD_DISPLAY_MODE_SUB",
        CreateJsValue(env, static_cast<uint32_t>(FoldDisplayMode::SUB)));
    napi_set_named_property(env, objValue, "FOLD_DISPLAY_MODE_COORDINATION",
        CreateJsValue(env, static_cast<uint32_t>(FoldDisplayMode::COORDINATION)));
    return objValue;
}

napi_value InitColorSpace(napi_env env)
{
    TLOGD(WmsLogTag::DMS, "InitColorSpace called");

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

    napi_set_named_property(env, objValue, "UNKNOWN",
        CreateJsValue(env, static_cast<uint32_t>(DmsColorSpace::UNKNOWN)));
    napi_set_named_property(env, objValue, "ADOBE_RGB",
        CreateJsValue(env, static_cast<uint32_t>(DmsColorSpace::ADOBE_RGB)));
    napi_set_named_property(env, objValue, "BT2020_HLG",
        CreateJsValue(env, static_cast<uint32_t>(DmsColorSpace::BT2020_HLG)));
    napi_set_named_property(env, objValue, "BT2020_PQ",
        CreateJsValue(env, static_cast<uint32_t>(DmsColorSpace::BT2020_PQ)));
    napi_set_named_property(env, objValue, "BT601_EBU",
        CreateJsValue(env, static_cast<uint32_t>(DmsColorSpace::BT601_EBU)));
    napi_set_named_property(env, objValue, "BT601_SMPTE_C",
        CreateJsValue(env, static_cast<uint32_t>(DmsColorSpace::BT601_SMPTE_C)));
    napi_set_named_property(env, objValue, "BT709",
        CreateJsValue(env, static_cast<uint32_t>(DmsColorSpace::BT709)));
    napi_set_named_property(env, objValue, "P3_HLG",
        CreateJsValue(env, static_cast<uint32_t>(DmsColorSpace::P3_HLG)));
    napi_set_named_property(env, objValue, "P3_PQ",
        CreateJsValue(env, static_cast<uint32_t>(DmsColorSpace::P3_PQ)));
    napi_set_named_property(env, objValue, "DISPLAY_P3",
        CreateJsValue(env, static_cast<uint32_t>(DmsColorSpace::DISPLAY_P3)));
    napi_set_named_property(env, objValue, "SRGB",
        CreateJsValue(env, static_cast<uint32_t>(DmsColorSpace::SRGB)));
    napi_set_named_property(env, objValue, "LINEAR_SRGB",
        CreateJsValue(env, static_cast<uint32_t>(DmsColorSpace::LINEAR_SRGB)));
    napi_set_named_property(env, objValue, "LINEAR_P3",
        CreateJsValue(env, static_cast<uint32_t>(DmsColorSpace::LINEAR_P3)));
    napi_set_named_property(env, objValue, "LINEAR_BT2020",
        CreateJsValue(env, static_cast<uint32_t>(DmsColorSpace::LINEAR_BT2020)));
    return objValue;
}

napi_value InitHDRFormat(napi_env env)
{
    TLOGD(WmsLogTag::DMS, "InitHDRFormat called");

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

    napi_set_named_property(env, objValue, "NONE",
        CreateJsValue(env, static_cast<uint32_t>(HDRFormat::NONE)));
    napi_set_named_property(env, objValue, "VIDEO_HLG",
        CreateJsValue(env, static_cast<uint32_t>(HDRFormat::VIDEO_HLG)));
    napi_set_named_property(env, objValue, "VIDEO_HDR10",
        CreateJsValue(env, static_cast<uint32_t>(HDRFormat::VIDEO_HDR10)));
    napi_set_named_property(env, objValue, "VIDEO_HDR_VIVID",
        CreateJsValue(env, static_cast<uint32_t>(HDRFormat::VIDEO_HDR_VIVID)));
    napi_set_named_property(env, objValue, "IMAGE_HDR_VIVID_DUAL",
        CreateJsValue(env, static_cast<uint32_t>(HDRFormat::IMAGE_HDR_VIVID_DUAL)));
    napi_set_named_property(env, objValue, "IMAGE_HDR_VIVID_SINGLE",
        CreateJsValue(env, static_cast<uint32_t>(HDRFormat::IMAGE_HDR_VIVID_SINGLE)));
    napi_set_named_property(env, objValue, "IMAGE_HDR_ISO_DUAL",
        CreateJsValue(env, static_cast<uint32_t>(HDRFormat::IMAGE_HDR_ISO_DUAL)));
    napi_set_named_property(env, objValue, "IMAGE_HDR_ISO_SINGLE",
        CreateJsValue(env, static_cast<uint32_t>(HDRFormat::IMAGE_HDR_ISO_SINGLE)));
    return objValue;
}

napi_value InitScreenShape(napi_env env)
{
    TLOGD(WmsLogTag::DMS, "InitScreenShape called");

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

    napi_set_named_property(env, objValue, "RECTANGLE",
        CreateJsValue(env, static_cast<uint32_t>(ScreenShape::RECTANGLE)));
    napi_set_named_property(env, objValue, "ROUND",
        CreateJsValue(env, static_cast<uint32_t>(ScreenShape::ROUND)));
    return objValue;
}

napi_value InitDisplaySourceMode(napi_env env)
{
    TLOGD(WmsLogTag::DMS, "InitDisplaySourceMode called");

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

    napi_set_named_property(env, objValue, "NONE",
        CreateJsValue(env, static_cast<uint32_t>(DisplaySourceMode::NONE)));
    napi_set_named_property(env, objValue, "MAIN",
        CreateJsValue(env, static_cast<uint32_t>(DisplaySourceMode::MAIN)));
    napi_set_named_property(env, objValue, "MIRROR",
        CreateJsValue(env, static_cast<uint32_t>(DisplaySourceMode::MIRROR)));
    napi_set_named_property(env, objValue, "EXTEND",
        CreateJsValue(env, static_cast<uint32_t>(DisplaySourceMode::EXTEND)));
    napi_set_named_property(env, objValue, "ALONE",
        CreateJsValue(env, static_cast<uint32_t>(DisplaySourceMode::ALONE)));
    return objValue;
}

static void BindCoordinateConvertNativeFunction(napi_env env, napi_value exportObj, const char* moduleName)
{
    BindNativeFunction(env, exportObj, "convertRelativeToGlobalCoordinate", moduleName,
        JsDisplayManager::ConvertRelativeCoordinateToGlobal);
    BindNativeFunction(env, exportObj, "convertGlobalToRelativeCoordinate", moduleName,
        JsDisplayManager::ConvertGlobalCoordinateToRelative);
}

napi_value JsDisplayManagerInit(napi_env env, napi_value exportObj)
{
    TLOGD(WmsLogTag::DMS, "JsDisplayManagerInit is called");

    if (env == nullptr || exportObj == nullptr) {
        TLOGE(WmsLogTag::DMS, "JsDisplayManagerInit env or exportObj is nullptr");
        return nullptr;
    }

    std::unique_ptr<JsDisplayManager> jsDisplayManager = std::make_unique<JsDisplayManager>(env);
    napi_wrap(env, exportObj, jsDisplayManager.release(), JsDisplayManager::Finalizer, nullptr, nullptr);

    napi_set_named_property(env, exportObj, "DisplayState", InitDisplayState(env));
    napi_set_named_property(env, exportObj, "Orientation", InitOrientation(env));
    napi_set_named_property(env, exportObj, "DmErrorCode", InitDisplayErrorCode(env));
    napi_set_named_property(env, exportObj, "DMError", InitDisplayError(env));
    napi_set_named_property(env, exportObj, "FoldStatus", InitFoldStatus(env));
    napi_set_named_property(env, exportObj, "FoldDisplayMode", InitFoldDisplayMode(env));
    napi_set_named_property(env, exportObj, "ColorSpace", InitColorSpace(env));
    napi_set_named_property(env, exportObj, "HDRFormat", InitHDRFormat(env));
    napi_set_named_property(env, exportObj, "ScreenShape", InitScreenShape(env));
    napi_set_named_property(env, exportObj, "DisplaySourceMode", InitDisplaySourceMode(env));

    const char *moduleName = "JsDisplayManager";
    BindNativeFunction(env, exportObj, "getDefaultDisplay", moduleName, JsDisplayManager::GetDefaultDisplay);
    BindNativeFunction(env, exportObj, "getDefaultDisplaySync", moduleName, JsDisplayManager::GetDefaultDisplaySync);
    BindNativeFunction(env, exportObj, "getPrimaryDisplaySync", moduleName, JsDisplayManager::GetPrimaryDisplaySync);
    BindNativeFunction(env, exportObj, "getDisplayByIdSync", moduleName, JsDisplayManager::GetDisplayByIdSync);
    BindNativeFunction(env, exportObj, "getBrightnessInfo", moduleName, JsDisplayManager::GetBrightnessInfoChange);
    BindNativeFunction(env, exportObj, "getAllDisplay", moduleName, JsDisplayManager::GetAllDisplay);
    BindNativeFunction(env, exportObj, "getAllDisplays", moduleName, JsDisplayManager::GetAllDisplays);
    BindNativeFunction(env, exportObj, "hasPrivateWindow", moduleName, JsDisplayManager::HasPrivateWindow);
    BindNativeFunction(env, exportObj, "isFoldable", moduleName, JsDisplayManager::IsFoldable);
    BindNativeFunction(env, exportObj, "isCaptured", moduleName, JsDisplayManager::IsCaptured);
    BindNativeFunction(env, exportObj, "getFoldStatus", moduleName, JsDisplayManager::GetFoldStatus);
    BindNativeFunction(env, exportObj, "getFoldDisplayMode", moduleName, JsDisplayManager::GetFoldDisplayMode);
    BindNativeFunction(env, exportObj, "setFoldDisplayMode", moduleName, JsDisplayManager::SetFoldDisplayMode);
    BindNativeFunction(env, exportObj, "setFoldStatusLocked", moduleName, JsDisplayManager::SetFoldStatusLocked);
    BindNativeFunction(env, exportObj, "getCurrentFoldCreaseRegion", moduleName,
        JsDisplayManager::GetCurrentFoldCreaseRegion);
    BindNativeFunction(env, exportObj, "on", moduleName, JsDisplayManager::RegisterDisplayManagerCallback);
    BindNativeFunction(env, exportObj, "off", moduleName, JsDisplayManager::UnregisterDisplayManagerCallback);
    BindNativeFunction(env, exportObj, "getAllDisplayPhysicalResolution", moduleName,
        JsDisplayManager::GetAllDisplayPhysicalResolution);
    BindNativeFunction(env, exportObj, "createVirtualScreen", moduleName, JsDisplayManager::CreateVirtualScreen);
    BindNativeFunction(env, exportObj, "makeUnique", moduleName, JsDisplayManager::MakeUnique);
    BindNativeFunction(env, exportObj, "destroyVirtualScreen", moduleName, JsDisplayManager::DestroyVirtualScreen);
    BindNativeFunction(env, exportObj, "setVirtualScreenSurface", moduleName,
        JsDisplayManager::SetVirtualScreenSurface);
    BindNativeFunction(env, exportObj, "addVirtualScreenBlocklist", moduleName,
        JsDisplayManager::AddVirtualScreenBlockList);
    BindNativeFunction(env, exportObj, "removeVirtualScreenBlocklist", moduleName,
        JsDisplayManager::RemoveVirtualScreenBlockList);
    BindNativeFunction(env, exportObj, "onChangeWithAttribute", moduleName,
        JsDisplayManager::RegisterDisplayAttributeChangeCallback);
    BindCoordinateConvertNativeFunction(env, exportObj, moduleName);
    return NapiGetUndefined(env);
}
}  // namespace Rosen
}  // namespace OHOS
