/*
 * Copyright (c) 2026-2026 Huawei Device Co., Ltd.
 * Licensed under * Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with * License.
 * You may obtain a copy of * License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under * License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See * License for * specific language governing permissions and
 * limitations under * License.
 */

#include "ani_fv_window_controller.h"

#include "ani_fv_window.h"
#include "float_view_option.h"

#include "graphic_common.h"
#include "window_manager.h"
#include "ani_common_want.h"
#include "window_manager_hilog.h"
#include "permission.h"

namespace OHOS {
namespace Rosen {
namespace {
const std::string FLOAT_VIEW_PERMISSION = "ohos.permission.FLOAT_VIEW";
}
using namespace AbilityRuntime;
AniFvController::AniFvController(const sptr<FloatViewController>& fvController)
    : fvController_(fvController)
{
    TLOGI(WmsLogTag::WMS_SYSTEM, "[FV]AniFvController generate");
}

AniFvController::~AniFvController()
{
    TLOGI(WmsLogTag::WMS_SYSTEM, "[FV]~AniFvController destroy");
}

void AniFvController::Finalizer(ani_env* env, ani_long nativeObj)
{
    TLOGI(WmsLogTag::WMS_SYSTEM, "[FV]Finalizer start");
    AniFvController* aniFvController = reinterpret_cast<AniFvController*>(nativeObj);
    if (aniFvController == nullptr) {
        TLOGI(WmsLogTag::WMS_SYSTEM, "[FV]aniFvController is nullptr,skip");
        return;
    }
    {
        std::lock_guard<std::mutex> lock(mtxListener_);
        if (!typeCallbackListenerMap_.empty()) {
            for (auto& [listenerType, innerMap] : typeCallbackListenerMap_) {
                for (auto& [ref, listener] : innerMap) {
                    env->GlobalReference_Delete(ref);
                }
            }
            typeCallbackListenerMap_.clear();
        }
    }
    ani_status ret = AniFvWindow::DelAniFvControllerObj(aniFvController);
    if (ret == ANI_OK) {
        TLOGI(WmsLogTag::WMS_SYSTEM, "[FV]Finalizer finish");
    }
}

ani_ref CreateAniFvControllerObject(ani_env* env, const sptr<FloatViewController>& fvController)
{
    TLOGI(WmsLogTag::WMS_SYSTEM, "[FV]start");
    ani_class cls = nullptr;
    ani_status ret;
    if ((ret = env->FindClass("@ohos.window.floatView.floatView.FvControllerInternal", &cls)) != ANI_OK) {
        TLOGD(WmsLogTag::WMS_SYSTEM, "[FV]failed to find class FvControllerInternal %{public}u", ret);
        return AniFvUtils::AniGetUndefined(env);
    }
    std::unique_ptr<AniFvController> aniFvController = std::make_unique<AniFvController>(fvController);
    ani_method initFunc = nullptr;
    if ((ret = env->Class_FindMethod(cls, "<ctor>", ":", &initFunc)) != ANI_OK) {
        TLOGE(WmsLogTag::WMS_SYSTEM, "[FV]get ctor constructor failed %{public}u", ret);
        return AniFvUtils::AniGetUndefined(env);
    }
    ani_object obj = nullptr;
    if ((ret = env->Object_New(cls, initFunc, &obj)) != ANI_OK) {
        TLOGE(WmsLogTag::WMS_SYSTEM, "[FV]failed to create AniFvController obj");
        return AniFvUtils::AniGetUndefined(env);
    }
    ani_method setObjFunc = nullptr;
    if ((ret = env->Class_FindMethod(cls, "setNativeObj", "l:", &setObjFunc)) != ANI_OK) {
        TLOGE(WmsLogTag::WMS_SYSTEM, "[FV]find setNativeObj failed %{public}u", ret);
        return AniFvUtils::AniGetUndefined(env);
    }
    ret = env->Object_CallMethod_Void(obj, setObjFunc, reinterpret_cast<ani_long>(aniFvController.get()));
    if (ret != ANI_OK) {
        TLOGE(WmsLogTag::WMS_SYSTEM, "[FV]set setNativeObj failed %{public}u", ret);
        return AniFvUtils::AniGetUndefined(env);
    }
    AniFvWindow::AddAniFvControllerObj(aniFvController.release(), obj);
    return obj;
}

void AniFvController::StartFloatViewAni(ani_env* env, ani_object obj, ani_long nativeObj)
{
    TLOGI(WmsLogTag::WMS_SYSTEM, "[FV]StartFloatViewAni start");
    if (!Permission::CheckCallingPermission(FLOAT_VIEW_PERMISSION)) {
        AniFvUtils::AniThrowError(env, WmErrorCode::WM_ERROR_NO_PERMISSION,
            "Permission denied. ohos.permission.FLOAT_VIEW is required.");
        return;
    }
    AniFvController* aniFvController = reinterpret_cast<AniFvController*>(nativeObj);
    if (aniFvController == nullptr) {
        TLOGE(WmsLogTag::WMS_SYSTEM, "[FV]aniFvController is nullptr");
        AniFvUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
            "Float view controller is not available.");
        return;
    }
    aniFvController->OnStartFloatViewAni(env);
}

void AniFvController::OnStartFloatViewAni(ani_env* env)
{
    if (fvController_ == nullptr) {
        TLOGE(WmsLogTag::WMS_SYSTEM, "[FV]fvController_ is nullptr");
        AniFvUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
            "Float view controller is not available.");
        return;
    }
    WMError errCode = fvController_->StartFloatView();
    if (errCode != WMError::WM_OK) {
        TLOGE(WmsLogTag::WMS_SYSTEM, "[FV]StartFloatView failed");
        AniFvUtils::AniThrowError(env, errCode, "Failed to start float view.");
        return;
    }
}

void AniFvController::StopFloatViewAni(ani_env* env, ani_object obj, ani_long nativeObj)
{
    TLOGI(WmsLogTag::WMS_SYSTEM, "[FV]StopFloatViewAni start");
    AniFvController* aniFvController = reinterpret_cast<AniFvController*>(nativeObj);
    if (aniFvController == nullptr) {
        TLOGE(WmsLogTag::WMS_SYSTEM, "[FV]aniFvController is nullptr");
        AniFvUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
            "Float view controller is not available.");
        return;
    }
    aniFvController->OnStopFloatViewAni(env);
}

void AniFvController::OnStopFloatViewAni(ani_env* env)
{
    if (fvController_ == nullptr) {
        TLOGE(WmsLogTag::WMS_SYSTEM, "[FV]fvController_ is nullptr");
        AniFvUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
            "Float view controller is not available.");
        return;
    }
    WMError errCode = fvController_->StopFloatViewFromClient();
    if (errCode != WMError::WM_OK) {
        TLOGE(WmsLogTag::WMS_SYSTEM, "[FV]StopFloatView failed");
        AniFvUtils::AniThrowError(env, errCode, "Failed to stop float view.");
        return;
    }
}

void AniFvController::SetUIContextAni(ani_env* env, ani_object obj, ani_long nativeObj,
    ani_string contextUrl, ani_object contentStorage)
{
    TLOGI(WmsLogTag::WMS_SYSTEM, "[FV]SetUIContextAni start");
    AniFvController* aniFvController = reinterpret_cast<AniFvController*>(nativeObj);
    if (aniFvController == nullptr) {
        TLOGE(WmsLogTag::WMS_SYSTEM, "[FV]aniFvController is nullptr");
        AniFvUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
            "Float view controller is not available.");
        return;
    }
    aniFvController->OnSetUIContextAni(env, contextUrl, contentStorage);
}

void AniFvController::OnSetUIContextAni(ani_env* env, ani_string contextUrl, ani_object contentStorage)
{
    if (fvController_ == nullptr) {
        TLOGE(WmsLogTag::WMS_SYSTEM, "[FV]fvController_ is nullptr");
        AniFvUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
            "Float view controller is not available.");
        return;
    }
    std::string url;
    if (AniFvUtils::GetStdString(env, contextUrl, url) != ANI_OK) {
        TLOGE(WmsLogTag::WMS_SYSTEM, "[FV]Failed to get contextUrl");
        AniFvUtils::AniThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM, "param get failed.");
        return;
    }
    WMError errCode = fvController_->SetUIContext(url, contentStorage);
    if (errCode != WMError::WM_OK) {
        TLOGE(WmsLogTag::WMS_SYSTEM, "[FV]SetUIContext failed");
        AniFvUtils::AniThrowError(env, errCode, "Failed to set UI content.");
        return;
    }
}

void AniFvController::SetFloatViewVisibilityInAppAni(ani_env* env, ani_object obj, ani_long nativeObj,
    ani_boolean visibleInApp)
{
    TLOGI(WmsLogTag::WMS_SYSTEM, "[FV]SetFloatViewVisibilityInAppAni start");
    AniFvController* aniFvController = reinterpret_cast<AniFvController*>(nativeObj);
    if (aniFvController == nullptr) {
        TLOGE(WmsLogTag::WMS_SYSTEM, "[FV]aniFvController is nullptr");
        AniFvUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
            "Float view controller is not available.");
        return;
    }
    aniFvController->OnSetFloatViewVisibilityInAppAni(env, visibleInApp);
}

void AniFvController::OnSetFloatViewVisibilityInAppAni(ani_env* env, ani_boolean visibleInApp)
{
    if (fvController_ == nullptr) {
        TLOGE(WmsLogTag::WMS_SYSTEM, "[FV]fvController_ is nullptr");
        AniFvUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
            "Float view controller is not available.");
        return;
    }
    bool visible = static_cast<bool>(visibleInApp);
    WMError errCode = fvController_->SetVisibilityInApp(visible);
    if (errCode != WMError::WM_OK) {
        TLOGE(WmsLogTag::WMS_SYSTEM, "[FV]SetFloatViewVisibilityInApp failed");
        AniFvUtils::AniThrowError(env, errCode, "Failed to set float view visibility in app.");
        return;
    }
}

void AniFvController::SetWindowSizeAni(ani_env* env, ani_object obj, ani_long nativeObj, ani_object sizeObj)
{
    TLOGI(WmsLogTag::WMS_SYSTEM, "[FV]SetWindowSizeAni start");
    AniFvController* aniFvController = reinterpret_cast<AniFvController*>(nativeObj);
    if (aniFvController == nullptr) {
        TLOGE(WmsLogTag::WMS_SYSTEM, "[FV]aniFvController is nullptr");
        AniFvUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
            "Float view controller is not available.");
        return;
    }
    aniFvController->OnSetWindowSizeAni(env, sizeObj);
}

void AniFvController::OnSetWindowSizeAni(ani_env* env, ani_object sizeObj)
{
    if (fvController_ == nullptr) {
        TLOGE(WmsLogTag::WMS_SYSTEM, "[FV]fvController_ is nullptr");
        AniFvUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
            "Float view controller is not available.");
        return;
    }
    Rect rect;
    if (!AniFvUtils::ParseWindowSize(env, sizeObj, rect)) {
        TLOGE(WmsLogTag::WMS_SYSTEM, "[FV]parse window size failed");
        AniFvUtils::AniThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM, "Convert window size failed.");
        return;
    }
    if (rect.width_ <= 0 || rect.height_ <= 0) {
        TLOGE(WmsLogTag::WMS_SYSTEM, "[FV]window size param illegal");
        AniFvUtils::AniThrowError(env, WmErrorCode::WM_ERROR_ILLEGAL_PARAM, "Window size param illegal.");
        return;
    }
    WMError errCode = fvController_->SetWindowSize(rect);
    if (errCode != WMError::WM_OK) {
        TLOGE(WmsLogTag::WMS_SYSTEM, "[FV]SetWindowSize failed");
        AniFvUtils::AniThrowError(env, errCode, "Failed to set window size.");
        return;
    }
}

ani_object AniFvController::GetWindowPropertiesAni(ani_env* env, ani_object obj, ani_long nativeObj)
{
    TLOGI(WmsLogTag::WMS_SYSTEM, "[FV]GetWindowPropertiesAni start");
    AniFvController* aniFvController = reinterpret_cast<AniFvController*>(nativeObj);
    if (aniFvController == nullptr) {
        TLOGE(WmsLogTag::WMS_SYSTEM, "[FV]aniFvController is nullptr");
        AniFvUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
            "Float view controller is not available.");
        return static_cast<ani_object>(AniFvUtils::AniGetUndefined(env));
    }
    return aniFvController->OnGetWindowPropertiesAni(env);
}

ani_object AniFvController::OnGetWindowPropertiesAni(ani_env* env)
{
    if (fvController_ == nullptr) {
        TLOGE(WmsLogTag::WMS_SYSTEM, "[FV]fvController_ is nullptr");
        AniFvUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
            "Float view controller is not available.");
        return static_cast<ani_object>(AniFvUtils::AniGetUndefined(env));
    }
    auto templateType = static_cast<uint32_t>(fvController_->GetTemplateType());
    auto state = fvController_->GetCurState();
    if (!fvController_->IsStateWithWindow(state)) {
        AniFvUtils::AniThrowError(env, WmErrorCode::WM_ERROR_FV_INVALID_STATE,
            "current state do not support to get window properties.");
        return static_cast<ani_object>(AniFvUtils::AniGetUndefined(env));
    }
    auto fvWindow = fvController_->GetWindow();
    if (fvWindow == nullptr) {
        TLOGE(WmsLogTag::WMS_SYSTEM, "[FV]window is null");
        AniFvUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY, "Float view window is not available.");
        return static_cast<ani_object>(AniFvUtils::AniGetUndefined(env));
    }
    auto windowInfo = fvController_->GetWindowInfo();
    auto jsObject = AniFvUtils::CreateAniFloatViewPropertiesObject(env, templateType, fvWindow, windowInfo, state);
    if (jsObject == nullptr) {
        AniFvUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
            "Failed to create window properties object.");
        return static_cast<ani_object>(AniFvUtils::AniGetUndefined(env));
    }
    return jsObject;
}

void AniFvController::RestoreMainWindowAni(ani_env* env, ani_object obj, ani_long nativeObj, ani_object wantParameters)
{
    TLOGI(WmsLogTag::WMS_SYSTEM, "[FV]RestoreMainWindowAni start");
    AniFvController* aniFvController = reinterpret_cast<AniFvController*>(nativeObj);
    if (aniFvController == nullptr) {
        TLOGE(WmsLogTag::WMS_SYSTEM, "[FV]aniFvController is nullptr");
        AniFvUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
            "Float view controller is not available.");
        return;
    }
    aniFvController->OnRestoreMainWindowAni(env, wantParameters);
}

void AniFvController::OnRestoreMainWindowAni(ani_env* env, ani_object wantParameters)
{
    if (fvController_ == nullptr) {
        TLOGE(WmsLogTag::WMS_SYSTEM, "[FV]fvController_ is nullptr");
        AniFvUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
            "Float view controller is not available.");
        return;
    }
    AAFwk::WantParams wantParams;
    if (wantParameters != nullptr) {
        ani_boolean isUndefined = false;
        ani_boolean isNull = false;
        env->Reference_IsUndefined(wantParameters, &isUndefined);
        env->Reference_IsNull(wantParameters, &isNull);
        if (!isUndefined && !isNull && !AppExecFwk::UnwrapWantParams(env, wantParameters, wantParams)) {
            TLOGE(WmsLogTag::WMS_SYSTEM, "Failed to convert parameters to wantParameters");
            AniFvUtils::AniThrowError(env, WmErrorCode::WM_ERROR_SYSTEM_ABNORMALLY, "convert parameters failed");
            return;
        }
    }

    std::shared_ptr<AAFwk::WantParams> parameters = std::make_shared<AAFwk::WantParams>(wantParams);
    WmErrorCode errCode = WM_JS_TO_ERROR_CODE_MAP.at(fvController_->RestoreMainWindow(parameters));
    if (errCode != WmErrorCode::WM_OK) {
        TLOGE(WmsLogTag::WMS_SYSTEM, "[FV]RestoreMainWindow failed");
        AniFvUtils::AniThrowError(env, errCode);
    }
    return;
}

bool AniFvController::IsCallbackRegistered(ani_env* env, CallbackType callbackType, ani_ref callback)
{
    if (typeCallbackListenerMap_.empty() ||
        typeCallbackListenerMap_.find(callbackType) == typeCallbackListenerMap_.end()) {
        TLOGI(WmsLogTag::WMS_SYSTEM,
            "[FV]callback type %{public}d not registered!", static_cast<uint32_t>(callbackType));
        return false;
    }
    for (auto& [ref, listener] : typeCallbackListenerMap_[callbackType]) {
        ani_boolean isEquals = ANI_FALSE;
        ani_status ret = env->Reference_StrictEquals(callback, ref, &isEquals);
        if (ret == ANI_OK && isEquals == ANI_TRUE) {
            return true;
        }
    }
    return false;
}

void AniFvController::RegisterCallbackWithType(ani_env* env, CallbackType callbackType, ani_ref callback)
{
    std::lock_guard<std::mutex> lock(mtxListener_);
    TLOGI(WmsLogTag::WMS_SYSTEM, "RegisterCallbackWithType is called, type: %{public}d", callbackType);
    if (callback == nullptr) {
        TLOGE(WmsLogTag::WMS_SYSTEM, "RegisterCallbackWithType callback is null");
        AniFvUtils::AniThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM, "Not enough parameters provided.");
        return;
    }
    if (IsCallbackRegistered(env, callbackType, callback)) {
        TLOGE(WmsLogTag::WMS_SYSTEM, "[FV] callback already registered");
        AniFvUtils::AniThrowError(env, WmErrorCode::WM_ERROR_FV_REPEAT_OPERATION, "Callback already registered.");
        return;
    }
    ani_ref cbRef{};
    if (env->GlobalReference_Create(callback, &cbRef) != ANI_OK) {
        TLOGE(WmsLogTag::WMS_SYSTEM, "[FV]create ref failed");
        AniFvUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY, "Failed to create callback reference.");
        return;
    }
    ani_vm* vm = nullptr;
    if (env->GetVM(&vm) != ANI_OK || vm == nullptr) {
        TLOGE(WmsLogTag::WMS_SYSTEM, "[FV]get vm failed");
        AniFvUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY, "Float view internal error.");
        env->GlobalReference_Delete(cbRef);
        return;
    }
    auto listener = sptr<AniFvWindowListener>::MakeSptr(env, vm, cbRef);
    if (listener == nullptr) {
        TLOGE(WmsLogTag::WMS_SYSTEM, "[FV]create callback func failed");
        AniFvUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY, "Failed to create callback listener.");
        env->GlobalReference_Delete(cbRef);
        return;
    }
    listener->SetMainEventHandler();
    auto ret = DoRegisterListenerWithType(callbackType, listener);
    if (ret != WMError::WM_OK) {
        TLOGE(WmsLogTag::WMS_SYSTEM, "[FV]register type %{public}d failed", static_cast<uint32_t>(callbackType));
        AniFvUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY, "Failed to register callback.");
        env->GlobalReference_Delete(cbRef);
        return;
    }
    typeCallbackListenerMap_[callbackType][cbRef] = listener;
    TLOGI(WmsLogTag::WMS_SYSTEM, "[FV]register callback:%{public}d success, size: %{public}zu",
        static_cast<uint32_t>(callbackType), typeCallbackListenerMap_[callbackType].size());
    return;
}

void AniFvController::UnregisterCallbackWithType(ani_env* env, CallbackType callbackType, ani_ref callback)
{
    std::lock_guard<std::mutex> lock(mtxListener_);
    TLOGI(WmsLogTag::WMS_SYSTEM, "[FV]UnregisterCallbackWithType is called, type: %{public}d",
        static_cast<uint32_t>(callbackType));
    if (callback == nullptr) {
        TLOGI(WmsLogTag::WMS_SYSTEM, "[FV]no callback specific, unregister all");
        for (auto& [ref, listener] : typeCallbackListenerMap_[callbackType]) {
            WMError ret = DoUnRegisterListenerWithType(callbackType, listener);
            if (ret != WMError::WM_OK) {
                TLOGE(WmsLogTag::WMS_SYSTEM,
                    "[FV]Unregister type %{public}d failed", static_cast<uint32_t>(callbackType));
                AniFvUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
                    "Failed to unregister callback.");
                return;
            }
            env->GlobalReference_Delete(ref);
        }
        typeCallbackListenerMap_.erase(callbackType);
        TLOGI(WmsLogTag::WMS_SYSTEM, "[FV]Unregister type %{public}d success", static_cast<uint32_t>(callbackType));
        return;
    }
    for (auto& [ref, listener] : typeCallbackListenerMap_[callbackType]) {
        ani_boolean isEquals = ANI_FALSE;
        ani_status ret = env->Reference_StrictEquals(callback, ref, &isEquals);
        if (ret != ANI_OK || isEquals == ANI_FALSE) {
            continue;
        }
        WMError retErr = DoUnRegisterListenerWithType(callbackType, listener);
        if (retErr != WMError::WM_OK) {
            TLOGE(WmsLogTag::WMS_SYSTEM, "[FV]Unregister type %{public}d failed", static_cast<uint32_t>(callbackType));
            AniFvUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
                "Failed to unregister callback.");
            return;
        }
        env->GlobalReference_Delete(ref);
        typeCallbackListenerMap_[callbackType].erase(ref);
        break;
    }
    TLOGI(WmsLogTag::WMS_SYSTEM, "[FV]Unregister type %{public}d success", static_cast<uint32_t>(callbackType));
}

WMError AniFvController::DoRegisterListenerWithType(
    CallbackType callbackType, sptr<AniFvWindowListener>& listener)
{
    if (fvController_ == nullptr) {
        TLOGE(WmsLogTag::WMS_SYSTEM, "[FV]fvController_ is nullptr");
        return WMError::WM_ERROR_NULLPTR;
    }
    WMError errCode = WMError::WM_OK;
    switch (callbackType) {
        case CallbackType::STATE_CHANGE_CB:
            errCode = fvController_->RegisterStateChangeListener(listener);
            break;
        case CallbackType::RECT_CHANGE_CB:
            errCode = fvController_->RegisterRectChangeListener(listener);
            break;
        case CallbackType::LIMITS_CHANGE_CB:
            errCode = fvController_->RegisterLimitsChangeListener(listener);
            break;
        default:
            break;
    }
    return errCode;
}

WMError AniFvController::DoUnRegisterListenerWithType(
    CallbackType callbackType, sptr<AniFvWindowListener>& listener)
{
    if (fvController_ == nullptr) {
        TLOGE(WmsLogTag::WMS_SYSTEM, "[FV]fvController_ is nullptr");
        return WMError::WM_ERROR_NULLPTR;
    }
    WMError errCode = WMError::WM_OK;
    switch (callbackType) {
        case CallbackType::STATE_CHANGE_CB:
            errCode = fvController_->UnregisterStateChangeListener(listener);
            break;
        case CallbackType::RECT_CHANGE_CB:
            errCode = fvController_->UnregisterRectChangeListener(listener);
            break;
        case CallbackType::LIMITS_CHANGE_CB:
            errCode = fvController_->UnregisterLimitsChangeListener(listener);
            break;
        default:
            break;
    }
    return errCode;
}

void AniFvController::OnStateChangeAni(ani_env* env, ani_object obj, ani_long nativeObj, ani_ref callback)
{
    TLOGI(WmsLogTag::WMS_SYSTEM, "[FV]OnStateChangeAni start");
    AniFvController* aniFvController = reinterpret_cast<AniFvController*>(nativeObj);
    if (aniFvController == nullptr) {
        TLOGE(WmsLogTag::WMS_SYSTEM, "[FV]aniFvController is nullptr");
        AniFvUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
            "Float view controller is not available.");
        return;
    }
    aniFvController->RegisterCallbackWithType(env, CallbackType::STATE_CHANGE_CB, callback);
}

void AniFvController::OnRectChangeAni(ani_env* env, ani_object obj, ani_long nativeObj, ani_ref callback)
{
    TLOGI(WmsLogTag::WMS_SYSTEM, "[FV]OnRectChangeAni start");
    AniFvController* aniFvController = reinterpret_cast<AniFvController*>(nativeObj);
    if (aniFvController == nullptr) {
        TLOGE(WmsLogTag::WMS_SYSTEM, "[FV]aniFvController is nullptr");
        AniFvUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
            "Float view controller is not available.");
        return;
    }
    aniFvController->RegisterCallbackWithType(env, CallbackType::RECT_CHANGE_CB, callback);
}

void AniFvController::OnLimitsChangeAni(ani_env* env, ani_object obj, ani_long nativeObj, ani_ref callback)
{
    TLOGI(WmsLogTag::WMS_SYSTEM, "[FV]OnLimitsChangeAni start");
    AniFvController* aniFvController = reinterpret_cast<AniFvController*>(nativeObj);
    if (aniFvController == nullptr) {
        TLOGE(WmsLogTag::WMS_SYSTEM, "[FV]aniFvController is nullptr");
        AniFvUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
            "Float view controller is not available.");
        return;
    }
    aniFvController->RegisterCallbackWithType(env, CallbackType::LIMITS_CHANGE_CB, callback);
}

void AniFvController::OffFvOnStateChangeAni(ani_env* env, ani_object obj, ani_long nativeObj, ani_ref callback)
{
    TLOGI(WmsLogTag::WMS_SYSTEM, "[FV]OffFvOnStateChangeAni start");
    AniFvController* aniFvController = reinterpret_cast<AniFvController*>(nativeObj);
    if (aniFvController == nullptr) {
        TLOGE(WmsLogTag::WMS_SYSTEM, "[FV]aniFvController is nullptr");
        AniFvUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
            "Float view controller is not available.");
        return;
    }
    aniFvController->UnregisterCallbackWithType(env, CallbackType::STATE_CHANGE_CB, callback);
}

void AniFvController::OffRectChangeAni(ani_env* env, ani_object obj, ani_long nativeObj, ani_ref callback)
{
    TLOGI(WmsLogTag::WMS_SYSTEM, "[FV]OffRectChangeAni start");
    AniFvController* aniFvController = reinterpret_cast<AniFvController*>(nativeObj);
    if (aniFvController == nullptr) {
        TLOGE(WmsLogTag::WMS_SYSTEM, "[FV]aniFvController is nullptr");
        AniFvUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
            "Float view controller is not available.");
        return;
    }
    aniFvController->UnregisterCallbackWithType(env, CallbackType::RECT_CHANGE_CB, callback);
}

void AniFvController::OffOnLimitsChangeAni(ani_env* env, ani_object obj, ani_long nativeObj, ani_ref callback)
{
    TLOGI(WmsLogTag::WMS_SYSTEM, "[FV]OffOnLimitsChangeAni start");
    AniFvController* aniFvController = reinterpret_cast<AniFvController*>(nativeObj);
    if (aniFvController == nullptr) {
        TLOGE(WmsLogTag::WMS_SYSTEM, "[FV]aniFvController is nullptr");
        AniFvUtils::AniThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
            "Float view controller is not available.");
        return;
    }
    aniFvController->UnregisterCallbackWithType(env, CallbackType::LIMITS_CHANGE_CB, callback);
}

sptr<FloatViewController> AniFvController::GetController() const
{
    return fvController_;
}

auto GetNativeMethod()
{
    std::array funcs = {
        ani_native_function{"startFloatViewNative", nullptr,
            reinterpret_cast<void*>(AniFvController::StartFloatViewAni)},
        ani_native_function{"stopFloatViewNative", nullptr,
            reinterpret_cast<void*>(AniFvController::StopFloatViewAni)},
        ani_native_function{"setUIContextNative", nullptr,
            reinterpret_cast<void*>(AniFvController::SetUIContextAni)},
        ani_native_function{"setFloatViewVisibilityInAppNative", nullptr,
            reinterpret_cast<void*>(AniFvController::SetFloatViewVisibilityInAppAni)},
        ani_native_function{"setWindowSizeNative", nullptr,
            reinterpret_cast<void*>(AniFvController::SetWindowSizeAni)},
        ani_native_function{"getWindowPropertiesNative", nullptr,
            reinterpret_cast<void*>(AniFvController::GetWindowPropertiesAni)},
        ani_native_function{"restoreMainWindowNative", nullptr,
            reinterpret_cast<void*>(AniFvController::RestoreMainWindowAni)},
        ani_native_function{"onStateChangeNative", nullptr,
            reinterpret_cast<void*>(AniFvController::OnStateChangeAni)},
        ani_native_function{"onRectChangeNative", nullptr,
            reinterpret_cast<void*>(AniFvController::OnRectChangeAni)},
        ani_native_function{"onLimitsChangeNative", nullptr,
            reinterpret_cast<void*>(AniFvController::OnLimitsChangeAni)},
        ani_native_function{"offStateChangeNative", nullptr,
            reinterpret_cast<void*>(AniFvController::OffFvOnStateChangeAni)},
        ani_native_function{"offRectChangeNative", nullptr,
            reinterpret_cast<void*>(AniFvController::OffRectChangeAni)},
        ani_native_function{"offLimitsChangeNative", nullptr,
            reinterpret_cast<void*>(AniFvController::OffOnLimitsChangeAni)},
    };
    return funcs;
}

ani_status ANI_Controller_Constructor(ani_vm *vm, uint32_t *result)
{
    ani_status ret;
    ani_env* env;
    TLOGI(WmsLogTag::WMS_SYSTEM, "[FV]ANI_Controller_Constructor start");
    if ((ret = vm->GetEnv(ANI_VERSION_1, &env)) != ANI_OK) {
        TLOGE(WmsLogTag::WMS_SYSTEM, "[FV]null env");
        return ANI_NOT_FOUND;
    }
    ani_class cls = nullptr;
    if ((ret = env->FindClass("@ohos.window.floatView.floatView.FvControllerInternal", &cls)) != ANI_OK) {
        TLOGE(WmsLogTag::WMS_SYSTEM, "[FV]ANI_Controller_Constructor failed to find namespace, %{public}u", ret);
        return ANI_NOT_FOUND;
    }
    std::array funcs = GetNativeMethod();
    for (const auto& func : funcs) {
        if ((ret = env->Class_BindNativeMethods(cls, &func, 1u)) != ANI_OK) {
            TLOGE(WmsLogTag::WMS_SYSTEM, "[FV]bind class method fail %{public}u, %{public}s, %{public}s",
                ret, func.name, func.signature);
            return ANI_NOT_FOUND;
        }
    }
    *result = ANI_VERSION_1;
    TLOGI(WmsLogTag::WMS_SYSTEM, "[FV]ANI_Controller_Constructor Init finish");
    return ANI_OK;
}

}  // namespace Rosen
}  // namespace OHOS