/*
 * Copyright (c) 2026-2026 Huawei Device Co., Ltd.
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

#include "ani_pip_controller.h"

#include "graphic_common.h"

#include "window_manager_hilog.h"
#include "ani_pip_utils.h"

namespace OHOS {
namespace Rosen {
namespace {
    constexpr float MAX_PIP_SCALE = 1.0f;
    constexpr const char* ETS_INFO_CLS = "@ohos.PiPWindow.PiPWindow.PiPWindowInfoInner";
    constexpr const char* ETS_SIZE_CLS = "@ohos.PiPWindow.PiPWindow.PiPWindowSizeInner";
}

AniPipController::AniPipController(const sptr<PictureInPictureControllerAni>& pipController)
    : pipController_(pipController)
{
    TLOGI(WmsLogTag::WMS_PIP, "AniPipController");
}

AniPipController::~AniPipController()
{
    TLOGI(WmsLogTag::WMS_PIP, "~AniPipController");
}

void AniPipController::DelListener(ani_env* env)
{
    std::lock_guard<std::mutex> lock(mtxListener_);
    if (!typeCallbackListenerMap_.empty()) {
        for (auto& [listenerType, innerMap] : typeCallbackListenerMap_) {
            for (auto& [ref, listener] : innerMap) {
                env->GlobalReference_Delete(ref);
            }
        }
        typeCallbackListenerMap_.clear();
        TLOGI(WmsLogTag::WMS_PIP, "Del Listener finish");
    }
}

void AniPipController::StartPiPAni(ani_env* env, ani_object obj, ani_long nativeObj)
{
    TLOGI(WmsLogTag::WMS_PIP, "startPiPAni start");
    AniPipController* aniPipController = reinterpret_cast<AniPipController*>(nativeObj);
    if (aniPipController != nullptr) {
        aniPipController->OnstartPiPAni(env);
    } else {
        TLOGE(WmsLogTag::WMS_PIP, "aniPipController is nullptr");
        AniThrowError(env, WMError::WM_ERROR_NULLPTR, "start PiP failed, A null pointer occurs");
    }
}

void AniPipController::OnstartPiPAni(ani_env* env)
{
    TLOGI(WmsLogTag::WMS_PIP, "OnstartPiPAni start");
    if (pipController_ == nullptr) {
        TLOGE(WmsLogTag::WMS_PIP, "pipController is nullptr");
        AniThrowError(env, WMError::WM_ERROR_NULLPTR, "start PiP failed, A null pointer occurs");
        return;
    }
    TLOGI(WmsLogTag::WMS_PIP, "startPiPAni next");
    pipController_->SetStateChangeReason(PiPStateChangeReason::REQUEST_START);
    WMError error = pipController_->StartPictureInPicture(StartPipType::USER_START);
    if (error != WMError::WM_OK) {
        TLOGE(WmsLogTag::WMS_PIP, "pipController is nullptr %{public}u",
            static_cast<int32_t>(WM_JS_TO_ERROR_CODE_MAP.at(error)));
        AniThrowError(env, error, "");
    } else {
        TLOGI(WmsLogTag::WMS_PIP, "finish");
    }
}

void AniPipController::StopPiPAni(ani_env* env, ani_object obj, ani_long nativeObj)
{
    TLOGI(WmsLogTag::WMS_PIP, "stopPiPAni start");
    AniPipController* aniPipController = reinterpret_cast<AniPipController*>(nativeObj);
    if (aniPipController != nullptr) {
        aniPipController->OnstopPiPAni(env);
    } else {
        TLOGE(WmsLogTag::WMS_PIP, "aniPipController is nullptr");
        AniThrowError(env, WMError::WM_ERROR_NULLPTR, "stop PiP failed, A null pointer occurs");
    }
}

void AniPipController::OnstopPiPAni(ani_env* env)
{
    TLOGI(WmsLogTag::WMS_PIP, "start");
    if (pipController_ == nullptr) {
        TLOGE(WmsLogTag::WMS_PIP, "pipController is nullptr");
        AniThrowError(env, WMError::WM_ERROR_NULLPTR, "stop PiP failed, A null pointer occurs");
        return;
    }
    TLOGI(WmsLogTag::WMS_PIP, "next");
    pipController_->SetStateChangeReason(PiPStateChangeReason::REQUEST_DELETE);
    WMError error = pipController_->StopPictureInPictureFromClient();
    if (error != WMError::WM_OK) {
        TLOGE(WmsLogTag::WMS_PIP, "pipController is nullptr, errorcode is %{public}u",
            static_cast<int32_t>(WM_JS_TO_ERROR_CODE_MAP.at(error)));
        AniThrowError(env, error, "");
    }
}

ani_boolean AniPipController::IsPiPSupportedAni(ani_env* env, ani_object obj, ani_long nativeObj)
{
    TLOGI(WmsLogTag::WMS_PIP, "start");
    bool isPiPSupported = PictureInPictureManager::IsSupportPiP();
    TLOGI(WmsLogTag::WMS_PIP, "isPiPSupported = %{public}u", isPiPSupported);
    return static_cast<ani_boolean>(isPiPSupported);
}

void AniPipController::SetAutoStartEnabledAni(ani_env* env, ani_object obj, ani_long nativeObj, ani_boolean enable)
{
    TLOGI(WmsLogTag::WMS_PIP, "start");
    AniPipController* aniPipController = reinterpret_cast<AniPipController*>(nativeObj);
    if (!aniPipController || !aniPipController->pipController_) {
        AniThrowError(env, WMError::WM_ERROR_NULLPTR, "aniPipController is null or pipController_ is null");
        return;
    }
    aniPipController->pipController_->SetAutoStartEnabled(static_cast<bool>(enable));
}

void AniPipController::UpdateContentSizeAni(ani_env* env, ani_object obj, ani_long nativeObj, ani_int w, ani_int h)
{
    TLOGI(WmsLogTag::WMS_PIP, "start");
    AniPipController* aniPipController = reinterpret_cast<AniPipController*>(nativeObj);
    if (!aniPipController || !aniPipController->pipController_) {
        AniThrowError(env, WMError::WM_ERROR_NULLPTR, "aniPipController is null or pipController_ is null");
        return;
    }
    aniPipController->pipController_->UpdateContentSize(static_cast<int32_t>(w), static_cast<int32_t>(h));
}

void AniPipController::UpdatePiPControlStatusAni(ani_env* env,
                                                 ani_object obj,
                                                 ani_long nativeObj,
                                                 ani_int controlType,
                                                 ani_int status)
{
    TLOGI(WmsLogTag::WMS_PIP, "start");
    AniPipController* aniPipController = reinterpret_cast<AniPipController*>(nativeObj);
    if (!aniPipController || !aniPipController->pipController_) {
        AniThrowError(env, WMError::WM_ERROR_NULLPTR, "aniPipController is null or pipController_ is null");
        return;
    }
    aniPipController->pipController_->UpdatePiPControlStatus(static_cast<PiPControlType>(controlType),
        static_cast<PiPControlStatus>(status));
}

void AniPipController::UpdateContentNodeAni(ani_env* env, ani_object obj, ani_long nativeObj, ani_object typeNode)
{
    TLOGI(WmsLogTag::WMS_PIP, "start");
    AniPipController* aniPipController = reinterpret_cast<AniPipController*>(nativeObj);
    if (!aniPipController || !aniPipController->pipController_) {
        AniThrowError(env, WMError::WM_ERROR_NULLPTR, "aniPipController is null or pipController_ is null");
        return;
    }
    if (!typeNode) {
        AniThrowError(env, WMError::WM_ERROR_INVALID_PARAM, "updateContentNode failed, invalid typeNode");
        return;
    }
    
    // check Undefined
    ani_boolean isTypeNodeUndefined = false;
    env->Reference_IsUndefined(typeNode, &isTypeNodeUndefined);
    if (isTypeNodeUndefined) {
        AniThrowError(env, WMError::WM_ERROR_PIP_INTERNAL_ERROR, "[GetPiPOption]typeNode is undefined");
        return;
    }
    
    // create global ref for node
    ani_ref nodeRef {};
    if (env->GlobalReference_Create(typeNode, &nodeRef) != ANI_OK) {
        AniThrowError(env, WMError::WM_ERROR_PIP_INTERNAL_ERROR, "updateContentNode failed, create ref failed");
        return;
    }

    // create new nodeRef and delete old nodeRef ref if exists
    ani_ref oldRef = aniPipController->pipController_->GetANITypeNode();
    aniPipController->pipController_->UpdateContentNodeRef(nodeRef);

    ani_status ret = ANI_OK;
    if (oldRef != nullptr) {
        ret = env->GlobalReference_Delete(oldRef);
    }
    if (ret != ANI_OK) {
        AniThrowError(env, WMError::WM_ERROR_PIP_INTERNAL_ERROR, "Delete old typeNode failed");
    }
}

void AniPipController::SetPiPControlEnabledAni(ani_env* env,
                                               ani_object obj,
                                               ani_long nativeObj,
                                               ani_int controlType,
                                               ani_boolean enabled)
{
    TLOGI(WmsLogTag::WMS_PIP, "start");
    AniPipController* aniPipController = reinterpret_cast<AniPipController*>(nativeObj);
    if (!aniPipController || !aniPipController->pipController_) {
        AniThrowError(env, WMError::WM_ERROR_NULLPTR, "aniPipController is null or pipController_ is null");
        return;
    }
    aniPipController->pipController_->UpdatePiPControlStatus(static_cast<PiPControlType>(controlType),
        enabled ? PiPControlStatus::ENABLED : PiPControlStatus::DISABLED);
}

ani_ref AniPipController::GetPiPWindowInfoAni(ani_env* env, ani_object obj, ani_long nativeObj)
{
    TLOGI(WmsLogTag::WMS_PIP, "start");
    AniPipController* aniPipController = reinterpret_cast<AniPipController*>(nativeObj);
    if (!aniPipController || !aniPipController->pipController_) {
        AniThrowError(env, WMError::WM_ERROR_NULLPTR, "aniPipController is null or pipController_ is null");
        return AniGetUndefined(env);
    }
    if (!PictureInPictureManager::IsSupportPiP()) {
        AniThrowError(env, WMError::WM_ERROR_DEVICE_NOT_SUPPORT, "Capability not supported.");
        return AniGetUndefined(env);
    }
    const sptr<Window>& pipWindow = aniPipController->pipController_->GetPipWindow();
    if (!pipWindow) {
        AniThrowError(env, WMError::WM_ERROR_PIP_INTERNAL_ERROR, "PiP internal error.");
        return AniGetUndefined(env);
    }

    const auto& r = pipWindow->GetRect();
    const auto& t = pipWindow->GetLayoutTransform();
    float maxScale = std::min(std::max(t.scaleX_, t.scaleY_), MAX_PIP_SCALE);
    TLOGI(WmsLogTag::WMS_PIP, "width:%{public}u, height:%{public}u, scale:%{public}f", r.width_, r.height_, maxScale);

    return OnGetPiPWindowInfoAni(env, pipWindow, r, maxScale);
}

ani_ref AniPipController::OnGetPiPWindowInfoAni(ani_env* env, const sptr<Window>& pipWindow, Rect r, float maxScale)
{
    ani_class sizeCls = nullptr;
    if (env->FindClass(ETS_SIZE_CLS, &sizeCls) != ANI_OK) {
        AniThrowError(env, WMError::WM_ERROR_PIP_INTERNAL_ERROR, "find ETS_SIZE_CLS failed");
        return AniGetUndefined(env);
    }
    ani_method sizeCtor = nullptr;
    if (env->Class_FindMethod(sizeCls, "<ctor>", "iid:", &sizeCtor) != ANI_OK) {
        AniThrowError(env, WMError::WM_ERROR_PIP_INTERNAL_ERROR, "find sizeCtor failed");
        return AniGetUndefined(env);
    }
    ani_object sizeObj = {};
    ani_status ret = env->Object_New(sizeCls, sizeCtor, &sizeObj,
                    static_cast<ani_int>(r.width_),
                    static_cast<ani_int>(r.height_),
                    static_cast<ani_double>(maxScale));
    if (ret != ANI_OK) {
        AniThrowError(env, WMError::WM_ERROR_PIP_INTERNAL_ERROR, "create sizeObj failed");
        return AniGetUndefined(env);
    }

    ani_class infoCls{};
    if (env->FindClass(ETS_INFO_CLS, &infoCls) != ANI_OK) {
        AniThrowError(env, WMError::WM_ERROR_PIP_INTERNAL_ERROR, "find ETS_INFO_CLS failed");
        return AniGetUndefined(env);
    }
    ani_method infoCtor{};
    if (env->Class_FindMethod(infoCls, "<ctor>", "iC{@ohos.PiPWindow.PiPWindow.PiPWindowSize}:", &infoCtor) != ANI_OK) {
        AniThrowError(env, WMError::WM_ERROR_PIP_INTERNAL_ERROR, "find infoCtor failed");
        return AniGetUndefined(env);
    }
    ani_object infoObj{};
    ret = env->Object_New(infoCls, infoCtor, &infoObj, static_cast<ani_int>(pipWindow->GetWindowId()), sizeObj);
    if (ret != ANI_OK) {
        AniThrowError(env, WMError::WM_ERROR_PIP_INTERNAL_ERROR, "create infoObj failed");
        return AniGetUndefined(env);
    }
    TLOGI(WmsLogTag::WMS_PIP, "finish");
    return infoObj;
}

bool AniPipController::GetPiPSettingSwitchAni(ani_env* env, ani_object obj, ani_long nativeObj)
{
    TLOGI(WmsLogTag::WMS_PIP, "start");
    AniPipController* aniPipController = reinterpret_cast<AniPipController*>(nativeObj);
    if (!aniPipController || !aniPipController->pipController_) {
        AniThrowError(env, WMError::WM_ERROR_NULLPTR, "aniPipController is null or pipController_ is null");
        return false;
    }
    if (!aniPipController->pipController_->GetPipSettingSwitchStatusEnabled()) {
        AniThrowError(env, WMError::WM_ERROR_DEVICE_NOT_SUPPORT,
            "Capability not supported. Failed to call the API due to limited device capabilities.");
        return false;
    }
    bool status = aniPipController->pipController_->GetPiPSettingSwitchStatus();
    TLOGI(WmsLogTag::WMS_PIP, "GetPiPSettingSwitchAni: %{public}d", status);
    return status;
}

bool AniPipController::IsPiPActiveAni(ani_env* env, ani_object obj, ani_long nativeObj)
{
    TLOGI(WmsLogTag::WMS_PIP, "start");
    AniPipController* aniPipController = reinterpret_cast<AniPipController*>(nativeObj);
    if (!aniPipController || !aniPipController->pipController_) {
        AniThrowError(env, WMError::WM_ERROR_NULLPTR, "aniPipController is null or pipController_ is null");
        return false;
    }
    bool status = false;
    WMError ret = aniPipController->pipController_->IsPiPActive(status);
    if (ret != WMError::WM_OK) {
        AniThrowError(env, ret, "");
        return false;
    }
    TLOGI(WmsLogTag::WMS_PIP, "IsPiPActive: %{public}d", status);
    return status;
}

void AniPipController::ProcessRegisterListener(AniListenerType type, sptr<AniPiPListener> listener, WMError& ret)
{
    switch (type) {
        case AniListenerType::STATE_CHANGE_CB:
            ret = pipController_->RegisterPiPLifecycle(listener);
            break;
        case AniListenerType::CONTROL_PANEL_ACTION_EVENT_CB:
            ret = pipController_->RegisterPiPActionObserver(listener);
            break;
        case AniListenerType::CONTROL_EVENT_CB:
            ret = pipController_->RegisterPiPControlObserver(listener);
            break;
        case AniListenerType::SIZE_CHANGE_CB:
            ret = pipController_->RegisterPiPWindowSize(listener);
            break;
        case AniListenerType::ACTIVE_STATUS_CHANGE_CB:
            ret = pipController_->RegisterPiPActiveStatusChange(listener);
            break;
        default:
            ret = WMError::WM_ERROR_INVALID_PARAM;
            break;
    }
}

WMError AniPipController::RegisterListener(ani_env* env, AniListenerType type, ani_ref cb)
{
    TLOGI(WmsLogTag::WMS_PIP, "start");
    if (IsCallbackRegistered(env, type, cb)) {
        TLOGE(WmsLogTag::WMS_PIP, "error: Callback already registered!");
        return WMError::WM_ERROR_INVALID_CALLING;
    }
    ani_ref cbRef{};
    if (env->GlobalReference_Create(cb, &cbRef) != ANI_OK) {
        return WMError::WM_ERROR_PIP_INTERNAL_ERROR;
    }
    ani_vm* vm = nullptr;
    if (env->GetVM(&vm) != ANI_OK || !vm) {
        TLOGE(WmsLogTag::WMS_PIP, "Get VM failed");
        env->GlobalReference_Delete(cbRef);
        return WMError::WM_ERROR_PIP_INTERNAL_ERROR;
    }
    auto listener = sptr<AniPiPListener>::MakeSptr(env, vm, cbRef);
    if (!listener) {
        TLOGE(WmsLogTag::WMS_PIP, "New AniPiPListener failed");
        env->GlobalReference_Delete(cbRef);
        return WMError::WM_ERROR_PIP_INTERNAL_ERROR;
    }
    listener->SetMainEventHandler();

    WMError ret = WMError::WM_OK;
    ProcessRegisterListener(type, listener, ret);
    if (ret != WMError::WM_OK) {
        env->GlobalReference_Delete(cbRef);
        return ret;
    }
    // save in typeCallbackListenerMap_
    {
        std::lock_guard<std::mutex> lock(mtxListener_);
        typeCallbackListenerMap_[type].insert({ cbRef, listener });
        TLOGI(WmsLogTag::DEFAULT, "Register listener success! this type's callback map size: %{public}zu",
            typeCallbackListenerMap_[type].size());
    }
    return WMError::WM_OK;
}

bool AniPipController::IsCallbackRegistered(ani_env* env, AniListenerType type, ani_ref cb)
{
    TLOGI(WmsLogTag::WMS_PIP, "start");
    std::lock_guard<std::mutex> lock(mtxListener_);
    // check typeCallbackListenerMap_
    auto it = typeCallbackListenerMap_.find(type);
    if (it == typeCallbackListenerMap_.end()) { return false; }
    // check typeCallbackListenerMap_[type]
    for (auto &kv : it->second) {
        ani_boolean eq = 0;
        env->Reference_StrictEquals(cb, kv.first, &eq);
        if (eq) {
            return true;
        }
    }
    // final result
    return false;
}

void AniPipController::OnStateChangeAni(ani_env* env, ani_object obj, ani_long nativeObj, ani_ref cb)
{
    TLOGI(WmsLogTag::WMS_PIP, "start");
    AniPipController* aniPipController = reinterpret_cast<AniPipController*>(nativeObj);
    if (!aniPipController || !aniPipController->pipController_) {
        AniThrowError(env, WMError::WM_ERROR_NULLPTR, "aniPipController is null or pipController_ is null");
        return;
    }
    auto ret = aniPipController->RegisterListener(env, AniListenerType::STATE_CHANGE_CB, cb);
    if (ret != WMError::WM_OK) {
        AniThrowError(env, ret, "onStateChange failed");
    }
}

void AniPipController::OnControlPanelActionEventAni(ani_env* env, ani_object obj, ani_long nativeObj, ani_ref cb)
{
    TLOGI(WmsLogTag::WMS_PIP, "start");
    AniPipController* aniPipController = reinterpret_cast<AniPipController*>(nativeObj);
    if (!aniPipController || !aniPipController->pipController_) {
        AniThrowError(env, WMError::WM_ERROR_NULLPTR, "aniPipController is null or pipController_ is null");
        return;
    }
    auto ret = aniPipController->RegisterListener(env, AniListenerType::CONTROL_PANEL_ACTION_EVENT_CB, cb);
    if (ret != WMError::WM_OK) {
        AniThrowError(env, ret, "onControlPanelActionEvent failed");
    }
}

void AniPipController::OnControlEventAni(ani_env* env, ani_object obj, ani_long nativeObj, ani_ref cb)
{
    TLOGI(WmsLogTag::WMS_PIP, "start");
    AniPipController* aniPipController = reinterpret_cast<AniPipController*>(nativeObj);
    if (!aniPipController || !aniPipController->pipController_) {
        AniThrowError(env, WMError::WM_ERROR_NULLPTR, "aniPipController is null or pipController_ is null");
        return;
    }
    auto ret = aniPipController->RegisterListener(env, AniListenerType::CONTROL_EVENT_CB, cb);
    if (ret != WMError::WM_OK) {
        AniThrowError(env, ret, "onControlEvent failed");
    }
}

void AniPipController::OnPipWindowSizeChangeAni(ani_env* env, ani_object obj, ani_long nativeObj, ani_ref cb)
{
    TLOGI(WmsLogTag::WMS_PIP, "start");
    AniPipController* aniPipController = reinterpret_cast<AniPipController*>(nativeObj);
    if (!aniPipController || !aniPipController->pipController_) {
        AniThrowError(env, WMError::WM_ERROR_NULLPTR, "aniPipController is null or pipController_ is null");
        return;
    }
    auto ret = aniPipController->RegisterListener(env, AniListenerType::SIZE_CHANGE_CB, cb);
    if (ret != WMError::WM_OK) {
        AniThrowError(env, ret, "onPipWindowSizeChange failed");
    }
}

void AniPipController::OnActiveStatusChangeAni(ani_env* env, ani_object obj, ani_long nativeObj, ani_ref cb)
{
    TLOGI(WmsLogTag::WMS_PIP, "start");
    AniPipController* aniPipController = reinterpret_cast<AniPipController*>(nativeObj);
    if (!aniPipController || !aniPipController->pipController_) {
        AniThrowError(env, WMError::WM_ERROR_NULLPTR, "aniPipController is null or pipController_ is null");
        return;
    }
    auto ret = aniPipController->RegisterListener(env, AniListenerType::ACTIVE_STATUS_CHANGE_CB, cb);
    if (ret != WMError::WM_OK) {
        AniThrowError(env, ret, "OnActiveStatusChange failed");
    }
}

WMError AniPipController::UnregisterListener(ani_env* env, AniListenerType type, ani_ref cb)
{
    TLOGI(WmsLogTag::WMS_PIP, "start");
    std::lock_guard<std::mutex> lock(mtxListener_);

    auto it = typeCallbackListenerMap_.find(type);
    if (it == typeCallbackListenerMap_.end() || it->second.empty()) {
        TLOGI(WmsLogTag::WMS_PIP, "there's no such listener of this type callback, no need to Unregister");
        return WMError::WM_OK;
    }

    ani_boolean isUndef = false;
    ani_boolean isNull = false;
    env->Reference_IsUndefined(cb, &isUndef);
    env->Reference_IsNull(cb, &isNull);
    if (cb == nullptr || isUndef || isNull) {
        TLOGI(WmsLogTag::WMS_PIP, "Unregister all callback of this type");
        for (auto &kv : it->second) {
            WMError res = ClearListener(type, kv.second);
            if (res != WMError::WM_OK) {
                return res;
            }
            env->GlobalReference_Delete(kv.second->GetAniCallback());
            kv.second->SetAniCallback(nullptr);
        }
        it->second.clear();
        return WMError::WM_OK;
    }

    TLOGI(WmsLogTag::WMS_PIP, "Unregister only one callback of this type");
    for (auto iter = it->second.begin(); iter != it->second.end(); ++iter) {
        ani_boolean eq = 0;
        env->Reference_StrictEquals(cb, iter->first, &eq);
        if (eq) {
            WMError res = ClearListener(type, iter->second);
            if (res != WMError::WM_OK) {
                return res;
            }
            env->GlobalReference_Delete(iter->second->GetAniCallback());
            iter->second->SetAniCallback(nullptr);
            it->second.erase(iter);
            return WMError::WM_OK;
        }
    }
    return WMError::WM_OK;
}

WMError AniPipController::ClearListener(const AniListenerType& type, sptr<AniPiPListener> listener)
{
    TLOGI(WmsLogTag::WMS_PIP, "start");
    switch (type) {
        case AniListenerType::STATE_CHANGE_CB:
            return pipController_->UnregisterPiPLifecycle(listener);
        case AniListenerType::CONTROL_PANEL_ACTION_EVENT_CB:
            return pipController_->UnregisterPiPActionObserver(listener);
        case AniListenerType::CONTROL_EVENT_CB:
            return pipController_->UnregisterPiPControlObserver(listener);
        case AniListenerType::SIZE_CHANGE_CB:
            return pipController_->UnregisterPiPWindowSize(listener);
        case AniListenerType::ACTIVE_STATUS_CHANGE_CB:
            return pipController_->UnregisterPiPActiveStatusChange(listener);
        default:
            TLOGI(WmsLogTag::WMS_PIP, "no such AniListenerType");
            return WMError::WM_ERROR_INVALID_PARAM;
    }
}

void AniPipController::OffStateChangeAni(ani_env* env, ani_object obj, ani_long nativeObj)
{
    TLOGI(WmsLogTag::WMS_PIP, "start");
    AniPipController* aniPipController = reinterpret_cast<AniPipController*>(nativeObj);
    if (!aniPipController || !aniPipController->pipController_) {
        AniThrowError(env, WMError::WM_ERROR_NULLPTR, "aniPipController is null or pipController_ is null");
        return;
    }
    auto ret = aniPipController->UnregisterListener(env, AniListenerType::STATE_CHANGE_CB, nullptr);
    if (ret != WMError::WM_OK) {
        AniThrowError(env, ret, "offStateChange failed");
    }
}

void AniPipController::OffControlPanelActionEventAni(ani_env* env, ani_object obj, ani_long nativeObj)
{
    TLOGI(WmsLogTag::WMS_PIP, "start");
    AniPipController* aniPipController = reinterpret_cast<AniPipController*>(nativeObj);
    if (!aniPipController || !aniPipController->pipController_) {
        AniThrowError(env, WMError::WM_ERROR_NULLPTR, "aniPipController is null or pipController_ is null");
        return;
    }
    auto ret = aniPipController->UnregisterListener(env, AniListenerType::CONTROL_PANEL_ACTION_EVENT_CB, nullptr);
    if (ret != WMError::WM_OK) {
        AniThrowError(env, ret, "offControlPanelActionEvent failed");
    }
}

void AniPipController::OffControlEventAni(ani_env* env, ani_object obj, ani_long nativeObj, ani_ref cb)
{
    TLOGI(WmsLogTag::WMS_PIP, "start");
    AniPipController* aniPipController = reinterpret_cast<AniPipController*>(nativeObj);
    if (!aniPipController || !aniPipController->pipController_) {
        AniThrowError(env, WMError::WM_ERROR_NULLPTR, "aniPipController is null or pipController_ is null");
        return;
    }
    auto ret = aniPipController->UnregisterListener(env, AniListenerType::CONTROL_EVENT_CB, cb);
    if (ret != WMError::WM_OK) {
        AniThrowError(env, ret, "offControlEvent failed");
    }
}

void AniPipController::OffPipWindowSizeChangeAni(ani_env* env, ani_object obj, ani_long nativeObj, ani_ref cb)
{
    TLOGI(WmsLogTag::WMS_PIP, "start");
    AniPipController* aniPipController = reinterpret_cast<AniPipController*>(nativeObj);
    if (!aniPipController || !aniPipController->pipController_) {
        AniThrowError(env, WMError::WM_ERROR_NULLPTR, "aniPipController is null or pipController_ is null");
        return;
    }
    auto ret = aniPipController->UnregisterListener(env, AniListenerType::SIZE_CHANGE_CB, cb);
    if (ret != WMError::WM_OK) {
        AniThrowError(env, ret, "offPipWindowSizeChange failed");
    }
}

void AniPipController::OffActiveStatusChangeAni(ani_env* env, ani_object obj, ani_long nativeObj, ani_ref cb)
{
    TLOGI(WmsLogTag::WMS_PIP, "start");
    AniPipController* aniPipController = reinterpret_cast<AniPipController*>(nativeObj);
    if (!aniPipController || !aniPipController->pipController_) {
        AniThrowError(env, WMError::WM_ERROR_NULLPTR, "aniPipController is null or pipController_ is null");
        return;
    }
    auto ret = aniPipController->UnregisterListener(env, AniListenerType::ACTIVE_STATUS_CHANGE_CB, cb);
    if (ret != WMError::WM_OK) {
        AniThrowError(env, ret, "OffActiveStatusChange failed");
    }
}

std::array bindMethods = {
        ani_native_function {"startPiPNative", "l:", reinterpret_cast<void*>(AniPipController::StartPiPAni)},
        ani_native_function {"stopPiPNative", "l:", reinterpret_cast<void*>(AniPipController::StopPiPAni)},
        ani_native_function {"isPiPSupportedNative", "l:z",
            reinterpret_cast<void*>(AniPipController::IsPiPSupportedAni)},
        ani_native_function {"setAutoStartEnabledNative", nullptr,
            reinterpret_cast<void*>(OHOS::Rosen::AniPipController::SetAutoStartEnabledAni)},
        ani_native_function {"updateContentSizeNative", nullptr,
            reinterpret_cast<void*>(OHOS::Rosen::AniPipController::UpdateContentSizeAni)},
        ani_native_function {"updatePiPControlStatusNative", nullptr,
            reinterpret_cast<void*>(OHOS::Rosen::AniPipController::UpdatePiPControlStatusAni)},
        ani_native_function {"updateContentNodeNative", nullptr,
            reinterpret_cast<void*>(OHOS::Rosen::AniPipController::UpdateContentNodeAni)},
        ani_native_function {"setPiPControlEnabledNative", nullptr,
            reinterpret_cast<void*>(OHOS::Rosen::AniPipController::SetPiPControlEnabledAni)},
        ani_native_function {"getPiPWindowInfoNative", nullptr,
            reinterpret_cast<void*>(OHOS::Rosen::AniPipController::GetPiPWindowInfoAni)},
        ani_native_function {"getPiPSettingSwitchNative", nullptr,
            reinterpret_cast<void*>(OHOS::Rosen::AniPipController::GetPiPSettingSwitchAni)},
        ani_native_function {"isPiPActiveNative", "l:z",
            reinterpret_cast<void*>(OHOS::Rosen::AniPipController::IsPiPActiveAni)},
        ani_native_function {"onStateChangeNative", nullptr,
            reinterpret_cast<void*>(OHOS::Rosen::AniPipController::OnStateChangeAni)},
        ani_native_function {"onControlPanelActionEventNative", nullptr,
            reinterpret_cast<void*>(OHOS::Rosen::AniPipController::OnControlPanelActionEventAni)},
        ani_native_function {"onControlEventNative", nullptr,
            reinterpret_cast<void*>(OHOS::Rosen::AniPipController::OnControlEventAni)},
        ani_native_function {"onPipWindowSizeChangeNative", nullptr,
            reinterpret_cast<void*>(OHOS::Rosen::AniPipController::OnPipWindowSizeChangeAni)},
        ani_native_function {"onActiveStatusChangeNative", nullptr,
            reinterpret_cast<void*>(OHOS::Rosen::AniPipController::OnActiveStatusChangeAni)},
        ani_native_function {"offStateChangeNative", nullptr,
            reinterpret_cast<void*>(OHOS::Rosen::AniPipController::OffStateChangeAni)},
        ani_native_function {"offControlPanelActionEventNative", nullptr,
            reinterpret_cast<void*>(OHOS::Rosen::AniPipController::OffControlPanelActionEventAni)},
        ani_native_function {"offControlEventNative", nullptr,
            reinterpret_cast<void*>(OHOS::Rosen::AniPipController::OffControlEventAni)},
        ani_native_function {"offPipWindowSizeChangeNative", nullptr,
            reinterpret_cast<void*>(OHOS::Rosen::AniPipController::OffPipWindowSizeChangeAni)},
        ani_native_function {"offActiveStatusChangeNative", nullptr,
            reinterpret_cast<void*>(OHOS::Rosen::AniPipController::OffActiveStatusChangeAni)},
};

} // namespace Rosen
} // namespace OHOS


ani_status OHOS::Rosen::ANI_Controller_Constructor(ani_vm *vm, uint32_t *result)
{
    TLOGI(WmsLogTag::WMS_PIP, "start");
    ani_status ret;
    ani_env* env;
    if ((ret = vm->GetEnv(ANI_VERSION_1, &env)) != ANI_OK) {
        TLOGE(WmsLogTag::WMS_PIP, "null env");
        return ANI_NOT_FOUND;
    }

    ani_class cls = nullptr;
    if ((ret = env->FindClass("@ohos.PiPWindow.PiPWindow.PiPControllerInternal", &cls)) != ANI_OK) {
        TLOGD(WmsLogTag::WMS_PIP, "null env %{public}u", ret);
        return ret;
    }

    for (auto method: bindMethods) {
        if ((ret = env->Class_BindNativeMethods(cls, &method, 1u)) != ANI_OK) {
            TLOGE(WmsLogTag::WMS_PIP, "bind window method fail %{public}u, %{public}s, %{public}s",
                ret, method.name, method.signature);
            return ret;
        }
    }

    *result = ANI_VERSION_1;
    return ANI_OK;
}