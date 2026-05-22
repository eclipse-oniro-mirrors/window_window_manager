/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#include "ani_pip_utils.h"

#include <string>

#include "ani.h"

#include "window.h"
#include "wm_common.h"
#include "window_manager_hilog.h"
#include "interop_js/hybridgref_ani.h"
#include "interop_js/arkts_esvalue.h"
#include "interop_js/hybridgref_napi.h"
#include "interop_js/arkts_interop_js_api.h"

namespace OHOS {
namespace Rosen {
ani_vm* AniPipUtils::vm_ = nullptr;

constexpr const char* WM_ERROR_CODE_MSG_OK = "ok";
constexpr const char* WM_ERROR_CODE_MSG_NO_PERMISSION = "Permission verification failed,"
    "The application does not have the permission required to call the API";
constexpr const char* WM_ERROR_CODE_MSG_NOT_SYSTEM_APP = "Permission verification failed,"
    "A non-system application calls a system API";
constexpr const char* WM_ERROR_CODE_MSG_DEVICE_NOT_SUPPORT = "Capability not supported,"
    "Failed to call the API due to limited device capabilities";
constexpr const char* WM_ERROR_CODE_MSG_REPEAT_OPERATION = "Repeated operation";
constexpr const char* WM_ERROR_CODE_MSG_STATE_ABNORMALLY = "This window state is abnormal";
constexpr const char* WM_ERROR_CODE_MSG_SYSTEM_ABNORMALLY = "This window manager service works abnormally";
constexpr const char* WM_ERROR_CODE_MSG_INVALID_CALLING = "Unauthorized operation";
constexpr const char* WM_ERROR_CODE_MSG_STAGE_ABNORMALLY = "This window stage is abnormal";
constexpr const char* WM_ERROR_CODE_MSG_CONTEXT_ABNORMALLY = "This window context is abnormal";
constexpr const char* WM_ERROR_CODE_MSG_START_ABILITY_FAILED = "Failed to start the ability";
constexpr const char* WM_ERROR_CODE_MSG_INVALID_DISPLAY = "The display device is abnormal";
constexpr const char* WM_ERROR_CODE_MSG_INVALID_PARENT = "The parent window is invalid";
constexpr const char* WM_ERROR_CODE_MSG_PIP_DESTROY_FAILED = "Failed to destroy the PiP window";
constexpr const char* WM_ERROR_CODE_MSG_PIP_STATE_ABNORMALLY = "The PiP window state is abnormal";
constexpr const char* WM_ERROR_CODE_MSG_PIP_CREATE_FAILED = "Failed to create the PiP window";
constexpr const char* WM_ERROR_CODE_MSG_PIP_INTERNAL_ERROR = "PiP internal error";
constexpr const char* WM_ERROR_CODE_MSG_PIP_REPEAT_OPERATION = "Repeated PiP operation";
constexpr const char* WM_ERROR_CODE_MSG_INVALID_OP_IN_CUR_STATUS = "Invalid operation in current status";

static std::map<WmErrorCode, const char*> WM_ERROR_CODE_TO_ERROR_MSG_MAP {
    {WmErrorCode::WM_OK,                              WM_ERROR_CODE_MSG_OK                       },
    {WmErrorCode::WM_ERROR_NO_PERMISSION,             WM_ERROR_CODE_MSG_NO_PERMISSION            },
    {WmErrorCode::WM_ERROR_NOT_SYSTEM_APP,            WM_ERROR_CODE_MSG_NOT_SYSTEM_APP           },
    {WmErrorCode::WM_ERROR_DEVICE_NOT_SUPPORT,        WM_ERROR_CODE_MSG_DEVICE_NOT_SUPPORT       },
    {WmErrorCode::WM_ERROR_REPEAT_OPERATION,          WM_ERROR_CODE_MSG_REPEAT_OPERATION         },
    {WmErrorCode::WM_ERROR_STATE_ABNORMALLY,          WM_ERROR_CODE_MSG_STATE_ABNORMALLY         },
    {WmErrorCode::WM_ERROR_SYSTEM_ABNORMALLY,         WM_ERROR_CODE_MSG_SYSTEM_ABNORMALLY        },
    {WmErrorCode::WM_ERROR_INVALID_CALLING,           WM_ERROR_CODE_MSG_INVALID_CALLING          },
    {WmErrorCode::WM_ERROR_STAGE_ABNORMALLY,          WM_ERROR_CODE_MSG_STAGE_ABNORMALLY         },
    {WmErrorCode::WM_ERROR_CONTEXT_ABNORMALLY,        WM_ERROR_CODE_MSG_CONTEXT_ABNORMALLY       },
    {WmErrorCode::WM_ERROR_START_ABILITY_FAILED,      WM_ERROR_CODE_MSG_START_ABILITY_FAILED     },
    {WmErrorCode::WM_ERROR_INVALID_DISPLAY,           WM_ERROR_CODE_MSG_INVALID_DISPLAY          },
    {WmErrorCode::WM_ERROR_INVALID_PARENT,            WM_ERROR_CODE_MSG_INVALID_PARENT           },
    {WmErrorCode::WM_ERROR_PIP_DESTROY_FAILED,        WM_ERROR_CODE_MSG_PIP_DESTROY_FAILED       },
    {WmErrorCode::WM_ERROR_PIP_STATE_ABNORMALLY,      WM_ERROR_CODE_MSG_PIP_STATE_ABNORMALLY     },
    {WmErrorCode::WM_ERROR_PIP_CREATE_FAILED,         WM_ERROR_CODE_MSG_PIP_CREATE_FAILED        },
    {WmErrorCode::WM_ERROR_PIP_INTERNAL_ERROR,        WM_ERROR_CODE_MSG_PIP_INTERNAL_ERROR       },
    {WmErrorCode::WM_ERROR_PIP_REPEAT_OPERATION,      WM_ERROR_CODE_MSG_PIP_REPEAT_OPERATION     },
    {WmErrorCode::WM_ERROR_INVALID_OP_IN_CUR_STATUS,  WM_ERROR_CODE_MSG_INVALID_OP_IN_CUR_STATUS },
};

ani_object AniPipUtils::AniGetUndefined(ani_env* env)
{
    ani_ref res;
    env->GetUndefined(&res);
    return static_cast<ani_object>(res);
}

ani_status AniPipUtils::CreateBusinessError(ani_env* env, int32_t error, std::string message, ani_object* err)
{
    TLOGI(WmsLogTag::WMS_PIP, "start");
    ani_class aniClass;
    ani_status status = env->FindClass("@ohos.base.BusinessError", &aniClass);
    if (status != ANI_OK) {
        TLOGE(WmsLogTag::WMS_PIP, "class not found, status:%{public}d", static_cast<int32_t>(status));
        return status;
    }
    ani_method aniCtor;
    status = env->Class_FindMethod(aniClass, "<ctor>", "C{std.core.String}C{escompat.ErrorOptions}:", &aniCtor);
    if (status != ANI_OK) {
        TLOGE(WmsLogTag::WMS_PIP, "ctor not found, status:%{public}d", static_cast<int32_t>(status));
        return status;
    }
    ani_string aniMsg;
    env->String_NewUTF8(message.c_str(), static_cast<ani_size>(message.size()), &aniMsg);
    status = env->Object_New(aniClass, aniCtor, err, aniMsg, AniGetUndefined(env));
    if (status != ANI_OK) {
        TLOGE(WmsLogTag::WMS_PIP, "fail to new err, status:%{public}d", static_cast<int32_t>(status));
        return status;
    }
    status = env->Object_SetFieldByName_Int(*err, "code_", static_cast<ani_int>(error));
    if (status != ANI_OK) {
        TLOGE(WmsLogTag::WMS_PIP, "fail to set code, status:%{public}d", static_cast<int32_t>(status));
        return status;
    }
    TLOGI(WmsLogTag::WMS_PIP, "finish");
    return ANI_OK;
}

ani_ref AniPipUtils::AniThrowError(ani_env* env, WMError error, std::string msg)
{
    TLOGI(WmsLogTag::WMS_PIP, "start");
     // WMError → WmErrorCode
    WmErrorCode errorCode = WM_JS_TO_ERROR_CODE_MAP.at(error);
    std::string errorMessage = msg.empty() ? GetErrorMsg(errorCode) : msg;
    ani_object aniError;
    ani_status status = CreateBusinessError(env, static_cast<int32_t>(errorCode), errorMessage, &aniError);
    if (status != ANI_OK) {
        TLOGE(WmsLogTag::WMS_PIP, "CreateBusinessError failed, status:%{public}d", status);
        return AniGetUndefined(env);
    }
    env->ThrowError(static_cast<ani_error>(aniError));
    TLOGI(WmsLogTag::WMS_PIP, "finish");
    return AniGetUndefined(env);
}

std::string AniPipUtils::GetErrorMsg(WmErrorCode errorCode)
{
    auto it = WM_ERROR_CODE_TO_ERROR_MSG_MAP.find(errorCode);
    if (it == WM_ERROR_CODE_TO_ERROR_MSG_MAP.end()) {
        return "[FB]unknown WMError";
    }
    return it->second;
}

ani_status AniPipUtils::GetStdString(ani_env *env, ani_string ani_str, std::string &result)
{
    TLOGI(WmsLogTag::WMS_PIP, "start");
    ani_size strSize;
    ani_status ret = env->String_GetUTF8Size(ani_str, &strSize);
    if (ret != ANI_OK) {
        return ret;
    }
    std::vector<char> buffer(strSize + 1);
    char* utf8Buffer = buffer.data();
    ani_size bytes_written = 0;
    ret = env->String_GetUTF8(ani_str, utf8Buffer, strSize + 1, &bytes_written);
    if (ret != ANI_OK) {
        return ret;
    }
    utf8Buffer[bytes_written] = '\0';
    result = std::string(utf8Buffer);
    return ret;
}

void* AniPipUtils::GetAbilityContext(ani_env *env, ani_object aniObj)
{
    ani_long nativeContextLong;
    ani_class cls = nullptr;
    ani_field contextField = nullptr;
    ani_status status = ANI_ERROR;
    if ((status = env->FindClass("application.UIAbilityContext.UIAbilityContext", &cls)) != ANI_OK) {
        TLOGE(WmsLogTag::WMS_PIP,  "find class fail, status : %{public}d", status);
        return nullptr;
    }
    if ((status = env->Class_FindField(cls, "nativeContext", &contextField)) != ANI_OK) {
        TLOGE(WmsLogTag::WMS_PIP,  "find field fail, status : %{public}d", status);
        return nullptr;
    }
    if ((status = env->Object_GetField_Long(aniObj, contextField, &nativeContextLong)) != ANI_OK) {
        TLOGE(WmsLogTag::WMS_PIP,  "get field fail, status : %{public}d", status);
        return nullptr;
    }
    return (void*)nativeContextLong;
}

ani_status AniPipUtils::CallAniFunctionVoid(ani_env *env, const char* ns, const char* fn, const char* signature, ...)
{
    ani_status ret = ANI_OK;
    ani_namespace aniNamespace{};
    if ((ret = env->FindNamespace(ns, &aniNamespace)) != ANI_OK) {
        TLOGE(WmsLogTag::WMS_PIP, "cannot find namespace");
        return ret;
    }
    ani_function func{};
    if ((ret = env->Namespace_FindFunction(aniNamespace, fn, signature, &func)) != ANI_OK) {
        TLOGE(WmsLogTag::WMS_PIP, "cannot find function, errorcode is: %{public}u", ret);
        return ret;
    }
    va_list args;
    va_start(args, signature);
    ret = env->Function_Call_Void_V(func, args);
    va_end(args);
    if (ret != ANI_OK) {
        TLOGE(WmsLogTag::WMS_PIP, "cannot call function");
        return ret;
    }
    return ret;
}

ani_status AniPipUtils::GetAniString(ani_env* env, const std::string& str, ani_string* result)
{
    return env->String_NewUTF8(str.c_str(), static_cast<ani_size>(str.size()), result);
}

bool AniPipUtils::convertNativeRefToAniRef(ani_env* env,
                                           const std::shared_ptr<NativeReference>& nativeRef,
                                           ani_ref& aniRef)
{
    aniRef = nullptr;
    if (nativeRef == nullptr) {
        return true;
    }
    napi_env napiEnv = nullptr;
    bool napiScopeOpened = arkts_napi_scope_open(env, &napiEnv);
    if (!napiScopeOpened) {
        TLOGW(WmsLogTag::WMS_PIP, "arkts_napi_scope_open failed");
        return false;
    }
    if (napiEnv == nullptr) {
        TLOGE(WmsLogTag::WMS_PIP, "cannot open napi scope");
        return false;
    }
    napi_value value = nativeRef->GetNapiValue();
    if (value == nullptr) {
        TLOGW(WmsLogTag::WMS_PIP, "NativeReference has null napi value");
        return true;
    }
    hybridgref ref {};
    if (!hybridgref_create_from_napi(napiEnv, value, &ref)) {
        TLOGE(WmsLogTag::WMS_PIP, "hybridgref_create_from_napi failed");
        return false;
    }
    ani_object esValue = nullptr;
    if (!hybridgref_get_esvalue(env, ref, &esValue)) {
        TLOGE(WmsLogTag::WMS_PIP, "hybridgref_get_esvalue failed");
        hybridgref_delete_from_napi(napiEnv, ref);
        return false;
    }
    ani_status st = env->GlobalReference_Create(esValue, &aniRef);
    if (st != ANI_OK) {
        TLOGE(WmsLogTag::WMS_PIP, "GlobalReference_Create failed: %{public}d", static_cast<int32_t>(st));
        hybridgref_delete_from_napi(napiEnv, ref);
        return false;
    }
    if (!hybridgref_delete_from_napi(napiEnv, ref)) {
        TLOGW(WmsLogTag::WMS_PIP, "hybridgref_delete_from_napi failed");
        env->GlobalReference_Delete(aniRef);
        return false;
    }
    if (napiScopeOpened) {
        if (!arkts_napi_scope_close_n(napiEnv, 0, nullptr, nullptr)) {
            TLOGW(WmsLogTag::WMS_PIP, "arkts_napi_scope_close_n failed");
        }
    }
    return true;
}

bool AniPipUtils::TransferToPipOptionAni(ani_env* env,
                                         const sptr<PipOption>& pipOption,
                                         sptr<PipOptionAni>& pipOptionAni)
{
    TLOGI(WmsLogTag::WMS_PIP, "start");
    if (env == nullptr || pipOption == nullptr || pipOptionAni == nullptr) {
        TLOGE(WmsLogTag::WMS_PIP, "env、pipOption or pipOptionAni is nullptr");
        return false;
    }
    // copy simple fields
    pipOptionAni->SetContext(pipOption->GetContext());
    pipOptionAni->SetPipTemplate(pipOption->GetPipTemplate());
    pipOptionAni->SetNavigationId(pipOption->GetNavigationId());
    pipOptionAni->SetDefaultWindowSizeType(pipOption->GetDefaultWindowSizeType());
    pipOptionAni->SetControlGroup(pipOption->GetControlGroup());
    pipOptionAni->SetXComponentController(pipOption->GetXComponentController());
    pipOptionAni->SetTypeNodeEnabled(pipOption->IsTypeNodeEnabled());
    pipOptionAni->SetHandleId(pipOption->GetHandleId());
    pipOptionAni->SetCornerAdsorptionEnabled(pipOption->GetCornerAdsorptionEnabled());
    // copy content size
    uint32_t w = 0;
    uint32_t h = 0;
    pipOption->GetContentSize(w, h);
    pipOptionAni->SetContentSize(w, h);
    // copy pipControlStatusInfoList_
    for (const auto& statusInfo : pipOption->GetControlStatus()) {
        pipOptionAni->SetPiPControlStatus(statusInfo.controlType, statusInfo.status);
    }
    // copy pipControlEnableInfoList_
    for (const auto& enableInfo : pipOption->GetControlEnable()) {
        pipOptionAni->SetPiPControlEnabled(enableInfo.controlType, enableInfo.enabled);
    }

    TLOGI(WmsLogTag::WMS_PIP, "start convert nativeRef to ani_ref");

    ani_ref typeAni = nullptr;
    if (convertNativeRefToAniRef(env, pipOption->GetTypeNodeRef(), typeAni)) {
        pipOptionAni->SetTypeNodeRef(typeAni);
        TLOGI(WmsLogTag::WMS_PIP, "pip transfer typeNode to ani_ref successfully");
    }

    ani_ref nodeAni = nullptr;
    if (convertNativeRefToAniRef(env, pipOption->GetNodeControllerRef(), nodeAni)) {
        pipOptionAni->SetNodeControllerRef(nodeAni);
        TLOGI(WmsLogTag::WMS_PIP, "pip transfer customNodeController to ani_ref successfully");
    }

    ani_ref storageAni = nullptr;
    if (convertNativeRefToAniRef(env, pipOption->GetStorageRef(), storageAni)) {
        pipOptionAni->SetStorageRef(storageAni);
        TLOGI(WmsLogTag::WMS_PIP, "pip transfer storage to ani_ref successfully");
    }

    TLOGI(WmsLogTag::WMS_PIP, "finish");
    return true;
}

bool AniPipUtils::TransferToPipOptionNapi(ani_env* env,
                                          const sptr<PipOptionAni>& pipOptionAni,
                                          sptr<PipOption>& pipOption)
{
    TLOGI(WmsLogTag::WMS_PIP, "start");
    if (env == nullptr || pipOptionAni == nullptr || pipOption == nullptr) {
        TLOGE(WmsLogTag::WMS_PIP, "env、pipOptionAni or pipOption is nullptr");
        return false;
    }
    // copy simple fields
    pipOption->SetContext(pipOptionAni->GetContext());
    pipOption->SetPipTemplate(pipOptionAni->GetPipTemplate());
    pipOption->SetNavigationId(pipOptionAni->GetNavigationId());
    pipOption->SetDefaultWindowSizeType(pipOptionAni->GetDefaultWindowSizeType());
    pipOption->SetControlGroup(pipOptionAni->GetControlGroup());
    pipOption->SetXComponentController(pipOptionAni->GetXComponentController());
    pipOption->SetTypeNodeEnabled(pipOptionAni->IsTypeNodeEnabled());
    pipOption->SetHandleId(pipOptionAni->GetHandleId());
    pipOption->SetCornerAdsorptionEnabled(pipOptionAni->GetCornerAdsorptionEnabled());
    // copy content size
    uint32_t w = 0;
    uint32_t h = 0;
    pipOptionAni->GetContentSize(w, h);
    pipOption->SetContentSize(w, h);
    // copy pipControlStatusInfoList_
    for (const auto& statusInfo : pipOptionAni->GetControlStatus()) {
        pipOption->SetPiPControlStatus(statusInfo.controlType, statusInfo.status);
    }
    // copy pipControlEnableInfoList_
    for (const auto& enableInfo : pipOptionAni->GetControlEnable()) {
        pipOption->SetPiPControlEnabled(enableInfo.controlType, enableInfo.enabled);
    }

    TLOGI(WmsLogTag::WMS_PIP, "finish");
    return true;
}

void AniPipUtils::TransferToPipControllerAni(sptr<PictureInPictureController>& pipController,
                                             sptr<PictureInPictureControllerAni>& pipControllerAni)
{
    TLOGI(WmsLogTag::WMS_PIP, "start");
    if (pipController == nullptr || pipControllerAni == nullptr) {
        TLOGE(WmsLogTag::WMS_PIP, "pipController or pipControllerAni is nullptr");
        return;
    }
    // get fields from PictureInPictureControllerBase (exclude curState_)
    pipControllerAni->SetControllerId(pipController->GetControllerId());
    pipControllerAni->SetPipWindow(pipController->GetPipWindow());
    pipControllerAni->SetMainWindowLifeCycleListener(pipController->GetMainWindowLifeCycleListener());
    pipControllerAni->SetWindowRect(pipController->GetWindowRect());
    pipControllerAni->SetIsAutoStartEnabled(pipController->GetIsAutoStartEnabled());
    pipControllerAni->SetCurActiveStatus(pipController->GetCurActiveStatus());
    pipControllerAni->SetPipXComponentController(pipController->GetPipXComponentController());
    pipControllerAni->SetIsStoppedFromClient(pipController->GetIsStoppedFromClient());
    pipControllerAni->SetHandleId(pipController->GetHandleId());
    pipControllerAni->SetSurfaceId(pipController->GetSurfaceId());
    pipControllerAni->SetStateChangeReason(pipController->GetStateChangeReason());
    // get fields from PictureInPictureController
    pipControllerAni->SetMainWindowXComponentController(pipController->GetMainWindowXComponentController());
    pipControllerAni->SetFirstHandleId(pipController->GetFirstHandleId());
    TLOGI(WmsLogTag::WMS_PIP, "finish");
}

void AniPipUtils::TransferToPipControllerNapi(sptr<PictureInPictureControllerAni>& pipControllerAni,
                                              sptr<PictureInPictureController>& pipController)
{
    TLOGI(WmsLogTag::WMS_PIP, "start");
    if (pipControllerAni == nullptr || pipController == nullptr) {
        TLOGE(WmsLogTag::WMS_PIP, "pipControllerAni or pipController is nullptr");
        return;
    }
    // set fields from PictureInPictureControllerBase (exclude curState_)
    pipController->SetControllerId(pipControllerAni->GetControllerId());
    pipController->SetPipWindow(pipControllerAni->GetPipWindow());
    pipController->SetMainWindowLifeCycleListener(pipControllerAni->GetMainWindowLifeCycleListener());
    pipController->SetWindowRect(pipControllerAni->GetWindowRect());
    pipController->SetIsAutoStartEnabled(pipControllerAni->GetIsAutoStartEnabled());
    pipController->SetCurActiveStatus(pipControllerAni->GetCurActiveStatus());
    pipController->SetPipXComponentController(pipControllerAni->GetPipXComponentController());
    pipController->SetIsStoppedFromClient(pipControllerAni->GetIsStoppedFromClient());
    pipController->SetHandleId(pipControllerAni->GetHandleId());
    pipController->SetSurfaceId(pipControllerAni->GetSurfaceId());
    pipController->SetStateChangeReason(pipControllerAni->GetStateChangeReason());
    // set fields from PictureInPictureController
    pipController->SetMainWindowXComponentController(pipControllerAni->GetMainWindowXComponentController());
    pipController->SetFirstHandleId(pipControllerAni->GetFirstHandleId());
    TLOGI(WmsLogTag::WMS_PIP, "finish");
}

ani_object AniPipUtils::ConvertNapiValueToAniObject(ani_env* aniEnv, napi_env napiEnv, napi_value jsValue)
{
    TLOGI(WmsLogTag::WMS_PIP, "start");
    hybridgref ref {};
    if (!hybridgref_create_from_napi(napiEnv, jsValue, &ref)) {
        TLOGE(WmsLogTag::WMS_PIP, "create hybridgref fail");
        AniThrowError(aniEnv, WMError::WM_ERROR_INVALID_PARAM);
        return AniGetUndefined(aniEnv);
    }
    ani_object result {};
    if (!hybridgref_get_esvalue(aniEnv, ref, &result)) {
        TLOGE(WmsLogTag::WMS_PIP, "get esvalue fail");
        AniThrowError(aniEnv, WMError::WM_ERROR_INVALID_PARAM);
        return AniGetUndefined(aniEnv);
    }
    if (!hybridgref_delete_from_napi(napiEnv, ref)) {
        TLOGE(WmsLogTag::WMS_PIP, "delete hybridgref fail");
    }
    if (!arkts_napi_scope_close_n(napiEnv, 0, nullptr, nullptr)) {
        TLOGE(WmsLogTag::WMS_PIP, "napi close scope fail");
        AniThrowError(aniEnv, WMError::WM_ERROR_INVALID_PARAM);
        return AniGetUndefined(aniEnv);
    }
    TLOGI(WmsLogTag::WMS_PIP, "end");
    return result;
}

void AniPipUtils::InitVM(ani_env* env)
{
    if (env->GetVM(&vm_) != ANI_OK || !vm_) {
    TLOGE(WmsLogTag::WMS_PIP, "Get VM failed");
    AniPipUtils::AniThrowError(env, WMError::WM_ERROR_PIP_INTERNAL_ERROR);
    return;
    }
}
} // Rosen
} // OHOS