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

#include "js_pip_controller.h"
#include "js_pip_utils.h"
#include "picture_in_picture_manager.h"
#include "window_manager_hilog.h"

namespace OHOS {
namespace Rosen {
using namespace AbilityRuntime;
namespace {
    constexpr int32_t NUMBER_ZERO = 0;
    constexpr int32_t NUMBER_ONE = 1;
    constexpr int32_t NUMBER_TWO = 2;
    constexpr int32_t NUMBER_FOUR = 4;
    const std::string STATE_CHANGE_CB = "stateChange";
    const std::string CONTROL_PANEL_ACTION_EVENT_CB = "controlPanelActionEvent";
    const std::string CONTROL_EVENT_CB = "controlEvent";
    const std::string SIZE_CHANGE_CB = "pipWindowSizeChange";
    const std::string ACTIVE_STATUS_CHANGE_CB = "activeStatusChange";
}

void BindFunctions(napi_env env, napi_value object, const char* moduleName)
{
    BindNativeFunction(env, object, "startPiP", moduleName, JsPipController::StartPictureInPicture);
    BindNativeFunction(env, object, "stopPiP", moduleName, JsPipController::StopPictureInPicture);
    BindNativeFunction(env, object, "updateContentNode", moduleName, JsPipController::UpdateContentNode);
    BindNativeFunction(env, object, "updateContentSize", moduleName, JsPipController::UpdateContentSize);
    BindNativeFunction(env, object, "updatePiPControlStatus", moduleName, JsPipController::UpdatePiPControlStatus);
    BindNativeFunction(env, object, "setAutoStartEnabled", moduleName, JsPipController::SetAutoStartEnabled);
    BindNativeFunction(env, object, "setPiPControlEnabled", moduleName, JsPipController::SetPiPControlEnabled);
    BindNativeFunction(env, object, "getPiPWindowInfo", moduleName, JsPipController::GetPiPWindowInfo);
    BindNativeFunction(env, object, "getPiPSettingSwitch", moduleName, JsPipController::GetPiPSettingSwitch);
    BindNativeFunction(env, object, "isPiPActive", moduleName, JsPipController::IsPiPActive);
    BindNativeFunction(env, object, "on", moduleName, JsPipController::RegisterCallback);
    BindNativeFunction(env, object, "off", moduleName, JsPipController::UnregisterCallback);
    // test api
    BindNativeFunction(env, object, "isPiPSupported", moduleName, JsPipController::PictureInPicturePossible);
}

napi_value CreateJsPipControllerObject(napi_env env, sptr<PictureInPictureController>& pipController)
{
    napi_value objValue = nullptr;
    napi_status status = napi_create_object(env, &objValue);
    if (status != napi_ok || objValue == nullptr) {
        TLOGE(WmsLogTag::WMS_PIP, "failed to create js obj, error:%{public}d", status);
        return NapiGetUndefined(env);
    }

    TLOGI(WmsLogTag::WMS_PIP, "CreateJsPipController");
    std::unique_ptr<JsPipController> jsPipController = std::make_unique<JsPipController>(pipController);
    napi_wrap(env, objValue, jsPipController.release(), JsPipController::Finalizer, nullptr, nullptr);

    BindFunctions(env, objValue, "JsPipController");
    return objValue;
}

JsPipController::JsPipController(const sptr<PictureInPictureController>& pipController)
    : pipController_(pipController)
{
    listenerCodeMap_ = {
        { STATE_CHANGE_CB, ListenerType::STATE_CHANGE_CB },
        { CONTROL_PANEL_ACTION_EVENT_CB, ListenerType::CONTROL_PANEL_ACTION_EVENT_CB },
        { CONTROL_EVENT_CB, ListenerType::CONTROL_EVENT_CB },
        { SIZE_CHANGE_CB, ListenerType::SIZE_CHANGE_CB },
        { ACTIVE_STATUS_CHANGE_CB, ListenerType::ACTIVE_STATUS_CHANGE_CB },
    };
}

JsPipController::~JsPipController()
{
}

void JsPipController::Finalizer(napi_env env, void* data, void* hint)
{
    TLOGD(WmsLogTag::WMS_PIP, "Finalizer is called");
    std::unique_ptr<JsPipController>(static_cast<JsPipController*>(data));
}

napi_value JsPipController::StartPictureInPicture(napi_env env, napi_callback_info info)
{
    JsPipController* me = CheckParamsAndGetThis<JsPipController>(env, info);
    return (me != nullptr) ? me->OnStartPictureInPicture(env, info) : nullptr;
}

napi_value JsPipController::OnStartPictureInPicture(napi_env env, napi_callback_info info)
{
    TLOGI(WmsLogTag::WMS_PIP, "OnStartPictureInPicture is called");
    if (PictureInPictureManager::ShouldAbortPipStart()) {
        TLOGI(WmsLogTag::WMS_PIP, "OnStartPictureInPicture abort");
        return NapiGetUndefined(env);
    }

    napi_value result = nullptr;
    std::shared_ptr<NapiAsyncTask> napiAsyncTask = CreateEmptyAsyncTask(env, nullptr, &result);
    auto asyncTask = [this, env, task = napiAsyncTask,
        weak = wptr<PictureInPictureController>(pipController_)]() {
        auto pipController = weak.promote();
        if (pipController == nullptr) {
            task->Reject(env, CreateJsError(env, static_cast<int32_t>(WmErrorCode::WM_ERROR_PIP_STATE_ABNORMALLY),
                "JsPipController::OnStartPictureInPicture failed."));
            return;
        }
        pipController->SetStateChangeReason(PiPStateChangeReason::REQUEST_START);
        WMError errCode = pipController->StartPictureInPicture(StartPipType::USER_START);
        if (errCode != WMError::WM_OK) {
            task->Reject(env, CreateJsError(env, static_cast<int32_t>(WM_JS_TO_ERROR_CODE_MAP.at(errCode)),
                "JsPipController::OnStartPictureInPicture failed."));
            return;
        }
        task->Resolve(env, NapiGetUndefined(env));
    };
    if (napi_send_event(env, asyncTask, napi_eprio_immediate, "OnStartPictureInPicture") != napi_status::napi_ok) {
        napiAsyncTask->Reject(env, CreateJsError(env,
            static_cast<int32_t>(WMError::WM_ERROR_PIP_INTERNAL_ERROR), "Send event failed"));
    }
    return result;
}

napi_value JsPipController::StopPictureInPicture(napi_env env, napi_callback_info info)
{
    JsPipController* me = CheckParamsAndGetThis<JsPipController>(env, info);
    return (me != nullptr) ? me->OnStopPictureInPicture(env, info) : nullptr;
}

napi_value JsPipController::OnStopPictureInPicture(napi_env env, napi_callback_info info)
{
    TLOGI(WmsLogTag::WMS_PIP, "OnStopPictureInPicture is called");
    napi_value result = nullptr;
    std::shared_ptr<NapiAsyncTask> napiAsyncTask = CreateEmptyAsyncTask(env, nullptr, &result);
    auto asyncTask = [this, env, task = napiAsyncTask,
        weak = wptr<PictureInPictureController>(pipController_)]() {
        auto pipController = weak.promote();
        if (pipController == nullptr) {
            task->Reject(env, CreateJsError(env, static_cast<int32_t>(WmErrorCode::WM_ERROR_STATE_ABNORMALLY),
                "JsPipController::OnStopPictureInPicture failed."));
            return;
        }
        pipController->SetStateChangeReason(PiPStateChangeReason::REQUEST_DELETE);
        WMError errCode = pipController->StopPictureInPictureFromClient();
        if (errCode != WMError::WM_OK) {
            task->Reject(env, CreateJsError(env, static_cast<int32_t>(WM_JS_TO_ERROR_CODE_MAP.at(errCode)),
                "JsPipController::OnStopPictureInPicture failed."));
            return;
        }
        task->Resolve(env, NapiGetUndefined(env));
    };
    if (napi_send_event(env, asyncTask, napi_eprio_immediate, "OnStopPictureInPicture") != napi_status::napi_ok) {
        napiAsyncTask->Reject(env, CreateJsError(env,
            static_cast<int32_t>(WMError::WM_ERROR_PIP_INTERNAL_ERROR), "Send event failed"));
    }
    return result;
}

napi_value JsPipController::SetAutoStartEnabled(napi_env env, napi_callback_info info)
{
    JsPipController* me = CheckParamsAndGetThis<JsPipController>(env, info);
    return (me != nullptr) ? me->OnSetAutoStartEnabled(env, info) : nullptr;
}

napi_value JsPipController::OnSetAutoStartEnabled(napi_env env, napi_callback_info info)
{
    TLOGI(WmsLogTag::WMS_PIP, "OnSetAutoStartEnabled is called");
    size_t argc = NUMBER_FOUR;
    napi_value argv[NUMBER_FOUR] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc != 1) {
        TLOGE(WmsLogTag::WMS_PIP, "Argc count is invalid: %{public}zu", argc);
        return NapiThrowInvalidParam(env, "[PiPWindow][setAutoStartEnabled]msg: Invalid args count, 1 arg is needed");
    }
    bool enable = false;
    if (!ConvertFromJsValue(env, argv[0], enable)) {
        TLOGE(WmsLogTag::WMS_PIP, "Failed to convert parameter to bool");
        return NapiGetUndefined(env);
    }
    if (pipController_ == nullptr) {
        TLOGE(WmsLogTag::WMS_PIP, "error, controller is nullptr");
        return NapiGetUndefined(env);
    }
    pipController_->SetAutoStartEnabled(enable);
    return NapiGetUndefined(env);
}

napi_value JsPipController::UpdateContentNode(napi_env env, napi_callback_info info)
{
    JsPipController *me = CheckParamsAndGetThis<JsPipController>(env, info);
    return (me != nullptr) ? me->OnUpdateContentNode(env, info) : nullptr;
}

napi_value JsPipController::OnUpdateContentNode(napi_env env, napi_callback_info info)
{
    TLOGI(WmsLogTag::WMS_PIP, "OnUpdateContentNode is called");
    size_t argc = NUMBER_FOUR;
    napi_value argv[NUMBER_FOUR] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc != NUMBER_ONE) {
        TLOGE(WmsLogTag::WMS_PIP, "Argc count is invalid:%{public}zu", argc);
        return NapiThrowInvalidParam(env, "[PiPWindow][updateContentNode]msg: Invalid args count, 1 arg is needed");
    }
    napi_value typeNode = argv[0];
    if (GetType(env, typeNode) == napi_null || GetType(env, typeNode) == napi_undefined) {
        TLOGE(WmsLogTag::WMS_PIP, "Invalid typeNode");
        return NapiThrowInvalidParam(env, "[PiPWindow][updateContentNode]msg: Invalid typeNode");
    }
    napi_ref typeNodeRef = nullptr;
    napi_create_reference(env, typeNode, NUMBER_ONE, &typeNodeRef);
    napi_value result = nullptr;
    std::shared_ptr<NapiAsyncTask> napiAsyncTask = CreateEmptyAsyncTask(env, nullptr, &result);
    auto asyncTask = [env, task = napiAsyncTask, typeNodeRef,
            weak = wptr<PictureInPictureController>(pipController_)]() {
        if (!PictureInPictureManager::IsSupportPiP()) {
            task->Reject(env, CreateJsError(env, static_cast<int32_t>(WmErrorCode::WM_ERROR_DEVICE_NOT_SUPPORT),
                "Capability not supported. Failed to call the API due to limited device capabilities."));
            napi_delete_reference(env, typeNodeRef);
            return;
        }
        auto pipController = weak.promote();
        if (pipController == nullptr) {
            task->Reject(env, CreateJsError(env, static_cast<int32_t>(WmErrorCode::WM_ERROR_PIP_INTERNAL_ERROR),
                "PiP internal error."));
            napi_delete_reference(env, typeNodeRef);
            return;
        }
        napi_ref oldTypeNodeRef = pipController->GetTypeNode();
        pipController->UpdateContentNodeRef(typeNodeRef);
        if (oldTypeNodeRef != nullptr) {
            napi_delete_reference(env, oldTypeNodeRef);
            oldTypeNodeRef = nullptr;
        }
        task->Resolve(env, NapiGetUndefined(env));
    };
    if (napi_send_event(env, asyncTask, napi_eprio_immediate, "OnUpdateContentNode") != napi_status::napi_ok) {
        napi_delete_reference(env, typeNodeRef);
        napiAsyncTask->Reject(env, CreateJsError(env,
            static_cast<int32_t>(WMError::WM_ERROR_PIP_INTERNAL_ERROR), "Send event failed"));
    }
    return result;
}

napi_value JsPipController::UpdateContentSize(napi_env env, napi_callback_info info)
{
    JsPipController* me = CheckParamsAndGetThis<JsPipController>(env, info);
    return (me != nullptr) ? me->OnUpdateContentSize(env, info) : nullptr;
}

napi_value JsPipController::OnUpdateContentSize(napi_env env, napi_callback_info info)
{
    TLOGI(WmsLogTag::WMS_PIP, "OnUpdateContentSize is called");
    size_t argc = NUMBER_FOUR;
    napi_value argv[NUMBER_FOUR] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc != NUMBER_TWO) {
        TLOGE(WmsLogTag::WMS_PIP, "Invalid args count, need 2 args but received: %{public}zu", argc);
        return NapiThrowInvalidParam(env, "[PiPWindow][updateContentSize]msg: Invalid args count, 2 args is needed");
    }
    int32_t width = 0;
    std::string errMsg = "";
    if (!ConvertFromJsValue(env, argv[0], width) || width <= 0) {
        errMsg = "[PiPWindow][updateContentSize]msg: Failed to convert parameter to int or width <= 0";
        TLOGE(WmsLogTag::WMS_PIP, "%{public}s", errMsg.c_str());
        return NapiThrowInvalidParam(env, errMsg);
    }
    int32_t height = 0;
    if (!ConvertFromJsValue(env, argv[1], height) || height <= 0) {
        errMsg = "[PiPWindow][updateContentSize]msg: Failed to convert parameter to int or height <= 0";
        TLOGE(WmsLogTag::WMS_PIP, "%{public}s", errMsg.c_str());
        return NapiThrowInvalidParam(env, errMsg);
    }
    if (pipController_ == nullptr) {
        errMsg = "[PiPWindow][updateContentSize]msg: Controller is nullptr";
        TLOGE(WmsLogTag::WMS_PIP, "%{public}s", errMsg.c_str());
        return NapiThrowInvalidParam(env, errMsg);
    }
    pipController_->UpdateContentSize(width, height);
    return NapiGetUndefined(env);
}

napi_value JsPipController::UpdatePiPControlStatus(napi_env env, napi_callback_info info)
{
    JsPipController* me = CheckParamsAndGetThis<JsPipController>(env, info);
    return (me != nullptr) ? me->OnUpdatePiPControlStatus(env, info) : nullptr;
}

napi_value JsPipController::OnUpdatePiPControlStatus(napi_env env, napi_callback_info info)
{
    TLOGI(WmsLogTag::WMS_PIP, "called");
    size_t argc = NUMBER_FOUR;
    napi_value argv[NUMBER_FOUR] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc != NUMBER_TWO) {
        TLOGE(WmsLogTag::WMS_PIP, "Invalid args count, need 2 args but received: %{public}zu", argc);
        return NapiThrowInvalidParam(env,
            "[PiPWindow][updatePiPControlStatus]msg: Invalid args count, 2 args is needed");
    }
    auto controlType = PiPControlType::VIDEO_PLAY_PAUSE;
    std::string errMsg;
    if (!ConvertFromJsValue(env, argv[0], controlType)) {
        errMsg = "[PiPWindow][updatePiPControlStatus]msg: Failed to convert parameter to int or controlType < 0";
        TLOGE(WmsLogTag::WMS_PIP, "%{public}s", errMsg.c_str());
        return NapiThrowInvalidParam(env, errMsg);
    }
    auto status = PiPControlStatus::PLAY;
    if (!ConvertFromJsValue(env, argv[1], status)) {
        errMsg = "[PiPWindow][updatePiPControlStatus]msg: Failed to convert parameter to int";
        TLOGE(WmsLogTag::WMS_PIP, "%{public}s", errMsg.c_str());
        return NapiThrowInvalidParam(env, errMsg);
    }
    if (pipController_ == nullptr) {
        errMsg = "[PiPWindow][updatePiPControlStatus]msg: Controller is nullptr";
        TLOGE(WmsLogTag::WMS_PIP, "%{public}s", errMsg.c_str());
        return NapiThrowInvalidParam(env, errMsg);
    }
    pipController_->UpdatePiPControlStatus(controlType, status);
    return NapiGetUndefined(env);
}

napi_value JsPipController::SetPiPControlEnabled(napi_env env, napi_callback_info info)
{
    JsPipController* me = CheckParamsAndGetThis<JsPipController>(env, info);
    return (me != nullptr) ? me->OnSetPiPControlEnabled(env, info) : nullptr;
}

napi_value JsPipController::OnSetPiPControlEnabled(napi_env env, napi_callback_info info)
{
    TLOGI(WmsLogTag::WMS_PIP, "called");
    size_t argc = NUMBER_FOUR;
    napi_value argv[NUMBER_FOUR] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc != NUMBER_TWO) {
        TLOGE(WmsLogTag::WMS_PIP, "Invalid args count, need 2 args but received: %{public}zu", argc);
        return NapiThrowInvalidParam(env,
            "[PiPWindow][setPiPControlEnabled]msg: Invalid args count, 2 args is needed");
    }
    auto controlType = PiPControlType::VIDEO_PLAY_PAUSE;
    std::string errMsg = "";
    if (!ConvertFromJsValue(env, argv[0], controlType)) {
        errMsg = "[PiPWindow][setPiPControlEnabled]msg: Failed to convert parameter to int";
        TLOGE(WmsLogTag::WMS_PIP, "%{public}s", errMsg.c_str());
        return NapiThrowInvalidParam(env, errMsg);
    }
    bool enabled = true;
    if (!ConvertFromJsValue(env, argv[1], enabled)) {
        errMsg = "[PiPWindow][setPiPControlEnabled]msg: Failed to convert parameter to bool";
        TLOGE(WmsLogTag::WMS_PIP, "%{public}s", errMsg.c_str());
        return NapiThrowInvalidParam(env, errMsg);
    }
    if (pipController_ == nullptr) {
        errMsg = "[PiPWindow][setPiPControlEnabled]msg: Controller is nullptr";
        TLOGE(WmsLogTag::WMS_PIP, "%{public}s", errMsg.c_str());
        return NapiThrowInvalidParam(env, errMsg);
    }
    pipController_->UpdatePiPControlStatus(controlType, enabled ?
        PiPControlStatus::ENABLED : PiPControlStatus::DISABLED);
    return NapiGetUndefined(env);
}

napi_value JsPipController::GetPiPWindowInfo(napi_env env, napi_callback_info info)
{
    JsPipController* me = CheckParamsAndGetThis<JsPipController>(env, info);
    return (me != nullptr) ? me->OnGetPiPWindowInfo(env, info) : nullptr;
}

napi_value JsPipController::OnGetPiPWindowInfo(napi_env env, napi_callback_info info)
{
    TLOGI(WmsLogTag::WMS_PIP, "called");
    napi_value result = nullptr;
    std::shared_ptr<NapiAsyncTask> napiAsyncTask = CreateEmptyAsyncTask(env, nullptr, &result);
    auto asyncTask = [this, env, task = napiAsyncTask,
        weak = wptr<PictureInPictureController>(pipController_)]() {
        if (!PictureInPictureManager::IsSupportPiP()) {
            task->Reject(env, CreateJsError(env, static_cast<int32_t>(WmErrorCode::WM_ERROR_DEVICE_NOT_SUPPORT),
                "Capability not supported. Failed to call the API due to limited device capabilities."));
            return;
        }
        auto pipController = weak.promote();
        if (pipController == nullptr) {
            task->Reject(env, CreateJsError(env, static_cast<int32_t>(WmErrorCode::WM_ERROR_PIP_INTERNAL_ERROR),
                "PiP internal error."));
            return;
        }
        const sptr<Window>& pipWindow = pipController->GetPipWindow();
        if (pipWindow == nullptr) {
            TLOGE(WmsLogTag::WMS_PIP, "Failed to get pip window");
            task->Reject(env, CreateJsError(env, static_cast<int32_t>(WmErrorCode::WM_ERROR_PIP_INTERNAL_ERROR),
                "PiP internal error."));
            return;
        }
        task->Resolve(env, CreateJsPiPWindowInfoObject(env, pipWindow));
    };
    if (napi_send_event(env, asyncTask, napi_eprio_immediate, "OnGetPiPWindowInfo") != napi_status::napi_ok) {
        napiAsyncTask->Reject(env, CreateJsError(env,
            static_cast<int32_t>(WMError::WM_ERROR_PIP_INTERNAL_ERROR), "Send event failed"));
    }
    return result;
}

napi_value JsPipController::GetPiPSettingSwitch(napi_env env, napi_callback_info info)
{
    JsPipController* me = CheckParamsAndGetThis<JsPipController>(env, info);
    return (me != nullptr) ? me->OnGetPiPSettingSwitch(env, info) : nullptr;
}

napi_value JsPipController::OnGetPiPSettingSwitch(napi_env env, napi_callback_info info)
{
    TLOGI(WmsLogTag::WMS_PIP, "called");
    napi_value result = nullptr;
    std::shared_ptr<NapiAsyncTask> napiAsyncTask = CreateEmptyAsyncTask(env, nullptr, &result);
    auto asyncTask = [this, env, task = napiAsyncTask,
        weak = wptr<PictureInPictureController>(pipController_)]() {
        auto pipController = weak.promote();
        if (pipController == nullptr) {
            task->Reject(env, CreateJsError(env, static_cast<int32_t>(WmErrorCode::WM_ERROR_PIP_INTERNAL_ERROR),
                "PiP internal error."));
            return;
        }
        if (!pipController->GetPipSettingSwitchStatusEnabled()) {
            task->Reject(env, CreateJsError(env, static_cast<int32_t>(WmErrorCode::WM_ERROR_DEVICE_NOT_SUPPORT),
                "Capability not supported. Failed to call the API due to limited device capabilities."));
            return;
        }
        task->Resolve(env, CreateJsValue(env, pipController->GetPiPSettingSwitchStatus()));
    };
    if (napi_send_event(env, asyncTask, napi_eprio_immediate, "OnGetPiPSettingSwitch") != napi_status::napi_ok) {
        napiAsyncTask->Reject(env, CreateJsError(env,
            static_cast<int32_t>(WMError::WM_ERROR_PIP_INTERNAL_ERROR), "Send event failed"));
    }
    return result;
}

napi_value JsPipController::IsPiPActive(napi_env env, napi_callback_info info)
{
    JsPipController* me = CheckParamsAndGetThis<JsPipController>(env, info);
    return (me != nullptr) ? me->OnIsPiPActive(env, info) : nullptr;
}

napi_value JsPipController::OnIsPiPActive(napi_env env, napi_callback_info info)
{
    TLOGI(WmsLogTag::WMS_PIP, "called");
    wptr<PictureInPictureController> weakController(pipController_);
    std::shared_ptr<WmErrorCode> errCodePtr = std::make_shared<WmErrorCode>(WmErrorCode::WM_OK);
    std::shared_ptr<bool> statusPtr = std::make_shared<bool>(false);
    NapiAsyncTask::ExecuteCallback execute = [weakController, errCodePtr, statusPtr] {
        if (errCodePtr == nullptr || statusPtr == nullptr) {
            return;
        }
        auto pipController = weakController.promote();
        if (pipController == nullptr) {
            *errCodePtr = WmErrorCode::WM_ERROR_PIP_INTERNAL_ERROR;
            return;
        }
        bool status = false;
        *errCodePtr = ConvertErrorToCode(pipController->IsPiPActive(status));
        if (*errCodePtr == WmErrorCode::WM_OK) {
            *statusPtr = status;
        } else {
            *statusPtr = false;
        }
    };
    NapiAsyncTask::CompleteCallback complete = [errCodePtr, statusPtr]
        (napi_env env, NapiAsyncTask& task, int32_t status) {
            if (errCodePtr == nullptr || statusPtr == nullptr) {
                task.Reject(env, CreateJsError(env, static_cast<int32_t>(WmErrorCode::WM_ERROR_PIP_INTERNAL_ERROR),
                    "PiP internal error."));
                return;
            }
            if (*errCodePtr == WmErrorCode::WM_OK) {
                task.Resolve(env, CreateJsValue(env, *statusPtr));
            } else {
                task.Reject(env, CreateJsError(env, static_cast<int32_t>(*errCodePtr),
                    "JsPipController::OnIsPiPActive failed."));
            }
        };
    napi_value result = nullptr;
    NapiAsyncTask::Schedule("JsPipController::OnIsPiPActive", env,
        CreateAsyncTaskWithLastParam(env, nullptr, std::move(execute), std::move(complete), &result));
    return result;
}

napi_value JsPipController::RegisterCallback(napi_env env, napi_callback_info info)
{
    JsPipController* me = CheckParamsAndGetThis<JsPipController>(env, info);
    return (me != nullptr) ? me->OnRegisterCallback(env, info) : nullptr;
}

napi_value JsPipController::OnRegisterCallback(napi_env env, napi_callback_info info)
{
    TLOGI(WmsLogTag::WMS_PIP, "OnRegisterCallback is called");
    size_t argc = NUMBER_FOUR;
    napi_value argv[NUMBER_FOUR] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc < NUMBER_TWO) {
        TLOGE(WmsLogTag::WMS_PIP, "JsPipController Params not match: %{public}zu", argc);
        return NapiThrowInvalidParam(env, "[PiPWindow][on]msg: Invalid args count, count >= 2 is needed");
    }
    std::string cbType = "";
    if (!ConvertFromJsValue(env, argv[0], cbType)) {
        TLOGE(WmsLogTag::WMS_PIP, "Failed to convert parameter to callbackType");
        return NapiThrowInvalidParam(env, "[PiPWindow][on]msg: Failed to convert parameter to callbackType");
    }
    napi_value value = argv[1];
    if (value == nullptr || !NapiIsCallable(env, value)) {
        TLOGE(WmsLogTag::WMS_PIP, "Callback is nullptr or not callable");
        return NapiThrowInvalidParam(env,
            "[PiPWindow][on]msg: Callback is nullptr or not callable. Type is " + cbType);
    }
    WmErrorCode ret = RegisterListenerWithType(env, cbType, value);
    if (ret != WmErrorCode::WM_OK) {
        TLOGE(WmsLogTag::WMS_PIP, "OnRegisterCallback failed");
        return NapiThrowInvalidParam(env, "[PiPWindow][on]msg: RegisterCallback failed. Type is " + cbType);
    }
    return NapiGetUndefined(env);
}

WmErrorCode JsPipController::RegisterListenerWithType(napi_env env, const std::string& type, napi_value value)
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
    jsCbMap_[type].insert(pipWindowListener);

    switch (listenerCodeMap_[type]) {
        case ListenerType::STATE_CHANGE_CB:
            ProcessStateChangeRegister(pipWindowListener);
            break;
        case ListenerType::CONTROL_PANEL_ACTION_EVENT_CB:
            ProcessActionEventRegister(pipWindowListener);
            break;
        case ListenerType::CONTROL_EVENT_CB:
            ProcessControlEventRegister(pipWindowListener);
            break;
        case ListenerType::SIZE_CHANGE_CB:
            ProcessSizeChangeRegister(pipWindowListener);
            break;
        case ListenerType::ACTIVE_STATUS_CHANGE_CB:
            ProcessActiveStatusChangeRegister(pipWindowListener);
            break;
        default:
            break;
    }
    TLOGI(WmsLogTag::WMS_PIP, "Register type %{public}s success! callback map size: %{public}zu",
        type.c_str(), jsCbMap_[type].size());
    return WmErrorCode::WM_OK;
}

bool JsPipController::IfCallbackRegistered(napi_env env, const std::string& type, napi_value jsListenerObject)
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

void JsPipController::ProcessStateChangeRegister(const sptr<JsPiPWindowListener>& listener)
{
    if (pipController_ == nullptr) {
        TLOGE(WmsLogTag::WMS_PIP, "controller is nullptr");
        return;
    }
    sptr<IPiPLifeCycle> thisListener(listener);
    pipController_->RegisterPiPLifecycle(thisListener);
}

void JsPipController::ProcessActionEventRegister(const sptr<JsPiPWindowListener>& listener)
{
    if (pipController_ == nullptr) {
        TLOGE(WmsLogTag::WMS_PIP, "controller is nullptr");
        return;
    }
    sptr<IPiPActionObserver> thisListener(listener);
    pipController_->RegisterPiPActionObserver(listener);
}

void JsPipController::ProcessControlEventRegister(const sptr<JsPiPWindowListener>& listener)
{
    if (pipController_ == nullptr) {
        TLOGE(WmsLogTag::WMS_PIP, "controller is nullptr");
        return;
    }
    sptr<IPiPControlObserver> thisListener(listener);
    pipController_->RegisterPiPControlObserver(thisListener);
}

void JsPipController::ProcessSizeChangeRegister(const sptr<JsPiPWindowListener>& listener)
{
    if (pipController_ == nullptr) {
        TLOGE(WmsLogTag::WMS_PIP, "controller is nullptr");
        return;
    }
    sptr<IPiPWindowSize> thisListener(listener);
    pipController_->RegisterPiPWindowSize(thisListener);
}

void JsPipController::ProcessActiveStatusChangeRegister(const sptr<JsPiPWindowListener>& listener)
{
    if (pipController_ == nullptr) {
        TLOGE(WmsLogTag::WMS_PIP, "controller is nullptr");
        return;
    }
    sptr<IPiPActiveStatusObserver> thisListener(listener);
    pipController_->RegisterPiPActiveStatusChange(thisListener);
}

void JsPipController::ProcessStateChangeUnRegister(const sptr<JsPiPWindowListener>& listener)
{
    if (pipController_ == nullptr) {
        TLOGE(WmsLogTag::WMS_PIP, "controller is nullptr");
        return;
    }
    sptr<IPiPLifeCycle> thisListener(listener);
    pipController_->UnregisterPiPLifecycle(thisListener);
}

void JsPipController::ProcessActionEventUnRegister(const sptr<JsPiPWindowListener>& listener)
{
    if (pipController_ == nullptr) {
        TLOGE(WmsLogTag::WMS_PIP, "controller is nullptr");
        return;
    }
    sptr<IPiPActionObserver> thisListener(listener);
    pipController_->UnregisterPiPActionObserver(thisListener);
}

void JsPipController::ProcessControlEventUnRegister(const sptr<JsPiPWindowListener>& listener)
{
    if (pipController_ == nullptr) {
        TLOGE(WmsLogTag::WMS_PIP, "controller is nullptr");
        return;
    }
    sptr<IPiPControlObserver> thisListener(listener);
    pipController_->UnregisterPiPControlObserver(thisListener);
}

void JsPipController::ProcessSizeChangeUnRegister(const sptr<JsPiPWindowListener>& listener)
{
    if (pipController_ == nullptr) {
        TLOGE(WmsLogTag::WMS_PIP, "controller is nullptr");
        return;
    }
    sptr<IPiPWindowSize> thisListener(listener);
    pipController_->UnregisterPiPWindowSize(thisListener);
}

void JsPipController::ProcessActiveStatusChangeUnRegister(const sptr<JsPiPWindowListener>& listener)
{
    if (pipController_ == nullptr) {
        TLOGE(WmsLogTag::WMS_PIP, "controller is nullptr");
        return;
    }
    sptr<IPiPActiveStatusObserver> thisListener(listener);
    pipController_->UnregisterPiPActiveStatusChange(thisListener);
}

napi_value JsPipController::UnregisterCallback(napi_env env, napi_callback_info info)
{
    JsPipController* me = CheckParamsAndGetThis<JsPipController>(env, info);
    return (me != nullptr) ? me->OnUnregisterCallback(env, info) : nullptr;
}

napi_value JsPipController::OnUnregisterCallback(napi_env env, napi_callback_info info)
{
    size_t argc = NUMBER_FOUR;
    napi_value argv[NUMBER_FOUR] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc == NUMBER_ZERO || argc > NUMBER_TWO) {
        TLOGE(WmsLogTag::WMS_PIP, "JsPipController Params not match: %{public}zu", argc);
        return NapiThrowInvalidParam(env, "[PiPWindow][off]msg: Invalid args count, 0 < count <= 2 is needed");
    }
    std::string cbType = "";
    if (!ConvertFromJsValue(env, argv[0], cbType)) {
        TLOGE(WmsLogTag::WMS_PIP, "Failed to convert parameter to string");
        return NapiThrowInvalidParam(env, "[PiPWindow][off]msg: Failed to convert parameter to string");
    }
    if (argc == NUMBER_ONE) {
        UnRegisterListenerWithType(env, cbType, nullptr);
        return NapiGetUndefined(env);
    }
    napi_value value = argv[NUMBER_ONE];
    if (value != nullptr && NapiIsCallable(env, value)) {
        UnRegisterListenerWithType(env, cbType, value);
    }
    return NapiGetUndefined(env);
}

WmErrorCode JsPipController::UnRegisterListenerWithType(napi_env env, const std::string& type, napi_value value)
{
    if (jsCbMap_.empty() || jsCbMap_.find(type) == jsCbMap_.end()) {
        TLOGI(WmsLogTag::WMS_PIP, "methodName %{public}s not registered!", type.c_str());
        return WmErrorCode::WM_ERROR_INVALID_CALLING;
    }

    if (value == nullptr) {
        for (auto& listener : jsCbMap_[type]) {
            WmErrorCode ret = UnRegisterListener(type, listener);
            if (ret != WmErrorCode::WM_OK) {
                TLOGE(WmsLogTag::WMS_PIP, "Unregister type %{public}s failed, no value", type.c_str());
                return ret;
            }
        }
        jsCbMap_.erase(type);
    } else {
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
    }
    TLOGI(WmsLogTag::WMS_PIP, "Unregister type %{public}s success!", type.c_str());
    return WmErrorCode::WM_OK;
}

WmErrorCode JsPipController::UnRegisterListener(const std::string& type,
    const sptr<JsPiPWindowListener>& pipWindowListener)
{
    switch (listenerCodeMap_[type]) {
        case ListenerType::STATE_CHANGE_CB:
            ProcessStateChangeUnRegister(pipWindowListener);
            break;
        case ListenerType::CONTROL_PANEL_ACTION_EVENT_CB:
            ProcessActionEventUnRegister(pipWindowListener);
            break;
        case ListenerType::CONTROL_EVENT_CB:
            ProcessControlEventUnRegister(pipWindowListener);
            break;
        case ListenerType::SIZE_CHANGE_CB:
            ProcessSizeChangeUnRegister(pipWindowListener);
            break;
        case ListenerType::ACTIVE_STATUS_CHANGE_CB:
            ProcessActiveStatusChangeUnRegister(pipWindowListener);
            break;
        default:
            break;
    }
    return WmErrorCode::WM_OK;
}

napi_value JsPipController::PictureInPicturePossible(napi_env env, napi_callback_info info)
{
    JsPipController* me = CheckParamsAndGetThis<JsPipController>(env, info);
    return (me != nullptr) ? me->OnPictureInPicturePossible(env, info) : nullptr;
}

napi_value JsPipController::OnPictureInPicturePossible(napi_env env, napi_callback_info info)
{
    TLOGI(WmsLogTag::WMS_PIP, "called");
    bool isPiPSupported = false;
    if (pipController_ == nullptr) {
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WmErrorCode::WM_ERROR_PIP_INTERNAL_ERROR),
            "PiP internal error."));
        TLOGE(WmsLogTag::WMS_PIP, "error, controller is nullptr");
        return CreateJsValue(env, isPiPSupported);
    }
    pipController_->GetPipPossible(isPiPSupported);
    return CreateJsValue(env, isPiPSupported);
}
} // namespace Rosen
} // namespace OHOS
