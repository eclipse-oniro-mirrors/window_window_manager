/*
 * Copyright (c) 2025-2025 Huawei Device Co., Ltd.
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

#include "graphic_common.h"
#include "window_manager.h"

#include "floating_ball_option.h"
#include "ani_fb_window_controller.h"
#include "ani_fb_window.h"

namespace OHOS {
namespace Rosen {
using namespace AbilityRuntime;
AniFbController::AniFbController(const sptr<FloatingBallController>& fbController)
    : fbController_(fbController)
{
    TLOGI(WmsLogTag::WMS_SYSTEM, "[FB]AniFbController generate");
}

AniFbController::~AniFbController()
{
    TLOGI(WmsLogTag::WMS_SYSTEM, "[FB]~AniFbController destroy");
}

void AniFbController::Finalizer(ani_env* env, ani_long nativeObj)
{
    TLOGI(WmsLogTag::WMS_SYSTEM, "[FB]Finalizer start");
    AniFbController* aniFbController = reinterpret_cast<AniFbController*>(nativeObj);
    if (aniFbController == nullptr) {
        TLOGI(WmsLogTag::WMS_SYSTEM, "[FB]aniFbController is nullptr,skip");
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
    ani_status ret = AniFbWindow::DelAniFbControllerObj(aniFbController);
    if (ret == ANI_OK) {
        TLOGI(WmsLogTag::WMS_SYSTEM, "[FB]Finalizer finish");
    }
}

// Wrap the controller object into an object available for the ets layer
ani_ref CreateAniFbControllerObject(ani_env* env, const sptr<FloatingBallController>& fbController)
{
    TLOGI(WmsLogTag::WMS_SYSTEM, "[FB]start");
    ani_class cls = nullptr;
    ani_status ret;
    if ((ret = env->FindClass("@ohos.window.floatingBall.floatingBall.FbControllerInternal", &cls)) != ANI_OK) {
        TLOGD(WmsLogTag::WMS_SYSTEM, "[FB]failed to find class FbControllerInternal %{public}u", ret);
        return AniGetUndefined(env);
    }
    std::unique_ptr<AniFbController> aniFbController = std::make_unique<AniFbController>(fbController);
    ani_method initFunc = nullptr;
    if ((ret = env->Class_FindMethod(cls, "<ctor>", ":", &initFunc)) != ANI_OK) {
        TLOGE(WmsLogTag::WMS_SYSTEM, "[FB]get ctor constructor failed %{public}u", ret);
        return AniGetUndefined(env);
    }
    ani_object obj = nullptr;
    if ((ret = env->Object_New(cls, initFunc, &obj)) != ANI_OK) {
        TLOGE(WmsLogTag::WMS_SYSTEM, "[FB]failed to create AniFbController obj");
        return AniGetUndefined(env);
    }
    ani_method setObjFunc = nullptr;
    if ((ret = env->Class_FindMethod(cls, "setNativeObj", "l:", &setObjFunc)) != ANI_OK) {
        TLOGE(WmsLogTag::WMS_SYSTEM, "[FB]find setNativeObj failed %{public}u", ret);
        return AniGetUndefined(env);
    }
    ret = env->Object_CallMethod_Void(obj, setObjFunc, reinterpret_cast<ani_long>(aniFbController.get()));
    if (ret != ANI_OK) {
        TLOGE(WmsLogTag::WMS_SYSTEM, "[FB]set setNativeObj failed %{public}u", ret);
        return AniGetUndefined(env);
    }
    AniFbWindow::AddAniFbControllerObj(aniFbController.release(), obj);
    return obj;
}

bool CheckParamsInterface(ani_env* env, ani_object paramsInterface)
{
    if (paramsInterface == nullptr) {
        TLOGE(WmsLogTag::DEFAULT, "[FB]paramsInterface is nullptr");
        return false;
    }
    ani_boolean isUndefined = false;
    ani_boolean isNull = false;
    env->Reference_IsUndefined(paramsInterface, &isUndefined);
    env->Reference_IsNull(paramsInterface, &isNull);
    if (isUndefined || isNull) {
        TLOGE(WmsLogTag::DEFAULT, "[FB]paramsInterface is undefined or null");
        return false;
    }
    return true;
}

bool OptionSetTitle(ani_env* env, ani_object paramsInterface, FbOption& option)
{
    ani_ref titleValue;
    env->Object_GetPropertyByName_Ref(paramsInterface, "title", &titleValue);
    ani_boolean istitleValueUndefined = false;
    env->Reference_IsUndefined(titleValue, &istitleValueUndefined);
    if (!istitleValueUndefined) {
        TLOGI(WmsLogTag::WMS_SYSTEM, "[FB]option.SetTitle begin");
        ani_string anititleValue = reinterpret_cast<ani_string>(titleValue);
        std::string title;
        GetStdString(env, anititleValue, title);
        option.SetTitle(title);
        return true;
    } else {
        return false;
    }
}

bool OptionSetTemplate(ani_env* env, ani_object paramsInterface, FbOption& option)
{
    ani_ref templateTypeValue;
    env->Object_GetPropertyByName_Ref(paramsInterface, "template", &templateTypeValue);
    ani_int ret;
    ani_status res = env->EnumItem_GetValue_Int(static_cast<ani_enum_item>(templateTypeValue), &ret);
    if (res == ANI_OK) {
        TLOGI(WmsLogTag::WMS_SYSTEM, "[FB]option.SetTemplate begin");
        uint32_t template1 = static_cast<uint32_t>(ret);
        option.SetTemplate(template1);
        return true;
    } else {
        TLOGI(WmsLogTag::WMS_SYSTEM, "[FB]get template failed");
        return false;
    }
}

void OptionSetContent(ani_env* env, ani_object paramsInterface, FbOption& option)
{
    ani_ref contentValue;
    env->Object_GetPropertyByName_Ref(paramsInterface, "content", &contentValue);
    ani_boolean iscontentValueUndefined = false;
    env->Reference_IsUndefined(contentValue, &iscontentValueUndefined);
    if (!iscontentValueUndefined) {
        TLOGI(WmsLogTag::WMS_SYSTEM, "[FB]option.SetContent begin");
        ani_string anicontentValue = reinterpret_cast<ani_string>(contentValue);
        std::string content;
        GetStdString(env, anicontentValue, content);
        option.SetContent(content);
    }
}

void OptionSetBackgroundColor(ani_env* env, ani_object paramsInterface, FbOption& option)
{
    ani_ref backgroundColorValue;
    env->Object_GetPropertyByName_Ref(paramsInterface, "backgroundColor", &backgroundColorValue);
    ani_boolean isbackgroundColorValueUndefined = false;
    env->Reference_IsUndefined(backgroundColorValue, &isbackgroundColorValueUndefined);
    if (!isbackgroundColorValueUndefined) {
        TLOGI(WmsLogTag::WMS_SYSTEM, "[FB]option.SetBackgroundColor begin");
        ani_string anibackgroundColorValue = reinterpret_cast<ani_string>(backgroundColorValue);
        std::string backgroundColor;
        GetStdString(env, anibackgroundColorValue, backgroundColor);
        option.SetBackgroundColor(backgroundColor);
    }
}

void OptionSetIcon(ani_env* env, ani_object paramsInterface, FbOption& option)
{
    ani_ref iconValue;
    env->Object_GetPropertyByName_Ref(paramsInterface, "icon", &iconValue);
    ani_boolean isiconValueUndefined = false;
    env->Reference_IsUndefined(iconValue, &isiconValueUndefined);
    if (!isiconValueUndefined) {
        std::shared_ptr<Media::PixelMap> pixelMap = nullptr;
        ani_object iconValueObject = static_cast<ani_object>(iconValue);
        pixelMap = Media::PixelMapTaiheAni::GetNativePixelMap(env, iconValueObject);
        if (pixelMap == nullptr) {
            TLOGE(WmsLogTag::WMS_SYSTEM, "[FB]failed to extract icon: invalid PixelMap");
            return;
        }
        option.SetIcon(pixelMap);
    }
}

bool GetFbOption(ani_env* env, ani_object paramsInterface, FbOption& option)
{
    TLOGI(WmsLogTag::WMS_SYSTEM, "[FB]start");
    if (!CheckParamsInterface(env, paramsInterface)) {return false;}
    if (!OptionSetTitle(env, paramsInterface, option)) {return false;}
    if (!OptionSetTemplate(env, paramsInterface, option)) {return false;}
    OptionSetContent(env, paramsInterface, option);
    OptionSetBackgroundColor(env, paramsInterface, option);
    OptionSetIcon(env, paramsInterface, option);
    return true;
}

bool AniFbController::CheckParams(ani_env* env, const FbOption& option)
{
    TLOGI(WmsLogTag::WMS_SYSTEM, "[FB]start");
    if (option.GetTemplate() < static_cast<uint32_t>(FloatingBallTemplate::STATIC) ||
        option.GetTemplate() >= static_cast<uint32_t>(FloatingBallTemplate::END)) {
        TLOGE(WmsLogTag::WMS_SYSTEM, "[FB]template is invalid");
        AniThrowError<WmErrorCode>(env, WmErrorCode::WM_ERROR_FB_PARAM_INVALID, "template is invalid");
        return false;
    }
    if (option.GetTitle().length() < TITLE_MIN_LEN || option.GetTitle().length() > TITLE_MAX_LEN) {
        TLOGE(WmsLogTag::WMS_SYSTEM, "[FB]title length Exceed the limit %{public}zu", option.GetTitle().length());
        AniThrowError<WmErrorCode>(env, WmErrorCode::WM_ERROR_FB_PARAM_INVALID, "title is invalid");
        return false;
    }
    if (option.GetContent().length() > CONTENT_MAX_LEN) {
        TLOGE(WmsLogTag::WMS_SYSTEM, "[FB]content length Exceed the limit %{public}zu", option.GetContent().length());
        AniThrowError<WmErrorCode>(env, WmErrorCode::WM_ERROR_FB_PARAM_INVALID, "content length Exceed the limit");
        return false;
    }
    if (option.GetIcon() != nullptr && option.GetIcon()->GetByteCount() > PIXEL_MAP_MAX_SIZE) {
        TLOGE(WmsLogTag::WMS_SYSTEM, "[FB]icon size Exceed the limit %{public}d", option.GetIcon()->GetByteCount());
        AniThrowError<WmErrorCode>(env, WmErrorCode::WM_ERROR_FB_PARAM_INVALID, "icon size Exceed the limit");
        return false;
    }
    if (!option.GetBackgroundColor().empty() && !ColorParser::IsValidColorNoAlpha(option.GetBackgroundColor())) {
        TLOGE(WmsLogTag::WMS_SYSTEM, "[FB]backgroundColor is invalid");
        AniThrowError<WmErrorCode>(env, WmErrorCode::WM_ERROR_FB_PARAM_INVALID, "backgroundColor is invalid");
        return false;
    }
    if (option.GetTemplate() == static_cast<uint32_t>(FloatingBallTemplate::STATIC) &&
        option.GetIcon() == nullptr) {
        TLOGE(WmsLogTag::WMS_SYSTEM, "[FB]template %{public}u need icon", option.GetTemplate());
        AniThrowError<WmErrorCode>(env, WmErrorCode::WM_ERROR_FB_PARAM_INVALID, "current template need icon");
        return false;
    }
    return true;
}

void AniFbController::StartFloatingBallAni(ani_env* env,
                                           ani_object obj,
                                           ani_long nativeObj,
                                           ani_object paramsInterface)
{
    TLOGI(WmsLogTag::WMS_SYSTEM, "[FB]start");
    if (!Permission::CheckCallingPermission("ohos.permission.USE_FLOAT_BALL")) {
        AniThrowError<WmErrorCode>(env, WmErrorCode::WM_ERROR_NO_PERMISSION,
            "[FB]no permission ohos.permission.USE_FLOAT_BALL to start floatingBall");
        return;
    }
    // paramsInterface check
    if (paramsInterface == nullptr) {
        AniThrowError<WmErrorCode>(env, WmErrorCode::WM_ERROR_FB_INTERNAL_ERROR, "[FB]paramsInterface is nullptr");
        return;
    }
    ani_boolean isUndefined = false;
    ani_boolean isNull = false;
    env->Reference_IsUndefined(paramsInterface, &isUndefined);
    env->Reference_IsNull(paramsInterface, &isNull);
    if (isUndefined || isNull) {
        AniThrowError<WmErrorCode>(env, WmErrorCode::WM_ERROR_FB_INTERNAL_ERROR,
            "[FB]paramsInterface is undefined or null");
        return;
    }
    // check nullptr
    AniFbController* aniFbController = reinterpret_cast<AniFbController*>(nativeObj);
    if (aniFbController == nullptr) {
        AniThrowError<WmErrorCode>(env, WmErrorCode::WM_ERROR_FB_INTERNAL_ERROR,
            "[FB]AniFbController* aniFbController for nativeObj is nullptr");
        return;
    }
    // working
    aniFbController->OnstartFloatingBallAni(env, paramsInterface);
}

void AniFbController::OnstartFloatingBallAni(ani_env* env, ani_object paramsInterface)
{
    TLOGI(WmsLogTag::WMS_SYSTEM, "[FB]start");
    // check nullptr
    if (fbController_ == nullptr) {
        AniThrowError<WmErrorCode>(env, WmErrorCode::WM_ERROR_FB_INTERNAL_ERROR, "[FB]fbController_ is nullptr");
        return;
    }
    // fbOption
    FbOption fbOption;
    if (GetFbOption(env, paramsInterface, fbOption) == false) {
        AniThrowError<WmErrorCode>(env, WmErrorCode::WM_ERROR_FB_INTERNAL_ERROR,
            "[FB]Invalid parameters, please check if title is null, or parameter is invalid");
        return;
    }
    // check parameters
    if (CheckParams(env, fbOption) == false) {return;}
    // working
    sptr<FbOption> optionPtr = sptr<FbOption>::MakeSptr(fbOption);
    WMError errCode = fbController_->StartFloatingBall(optionPtr);
    // check result
    if (errCode != WMError::WM_OK) {
        TLOGE(WmsLogTag::WMS_SYSTEM, "[FB]fbController_->StartFloatingBall failed");
        AniThrowError<WmErrorCode>(env, WmErrorCode::WM_ERROR_FB_INTERNAL_ERROR, "[FB]internal error");
        return;
    }
    TLOGI(WmsLogTag::WMS_SYSTEM, "[FB]OnstartFloatingBallAni finish");
}

void AniFbController::UpdateFloatingBallAni(ani_env* env,
                                            ani_object obj,
                                            ani_long nativeObj,
                                            ani_object paramsInterface)
{
    TLOGI(WmsLogTag::WMS_SYSTEM, "[FB]start");
    // check nullptr
    AniFbController* aniFbController = reinterpret_cast<AniFbController*>(nativeObj);
    if (aniFbController == nullptr) {
        AniThrowError<WmErrorCode>(env, WmErrorCode::WM_ERROR_FB_INTERNAL_ERROR,
            "[FB]AniFbController* aniFbController for nativeObj is nullptr");
        return;
    }
    // working
    aniFbController->OnupdateFloatingBallAni(env, paramsInterface);
}

void AniFbController::OnupdateFloatingBallAni(ani_env* env, ani_object paramsInterface)
{
    TLOGI(WmsLogTag::WMS_SYSTEM, "[FB]start");
    // check nullptr
    if (fbController_ == nullptr) {
        AniThrowError<WmErrorCode>(env, WmErrorCode::WM_ERROR_FB_INTERNAL_ERROR, "[FB]fbController_ is nullptr");
        return;
    }
    // get fbOption
    FbOption fbOption;
    if (GetFbOption(env, paramsInterface, fbOption) == false) {
        AniThrowError<WmErrorCode>(env, WmErrorCode::WM_ERROR_FB_INTERNAL_ERROR,
            "[FB]Invalid parameters, please check if title is null, or parameter is invalid");
        return;
    }
    // check parameters
    if (CheckParams(env, fbOption) == false) {return;}
    // check nullptr
    sptr<FbOption> optionPtr = sptr<FbOption>::MakeSptr(fbOption);  // convert to sptr<FbOption>
    WMError errCode = fbController_->UpdateFloatingBall(optionPtr);
    // check result
    if (errCode != WMError::WM_OK) {
        TLOGE(WmsLogTag::WMS_SYSTEM, "[FB]fbController_->UpdateFloatingBall failed");
        AniThrowError<WmErrorCode>(env, WmErrorCode::WM_ERROR_FB_INTERNAL_ERROR, "[FB]internal error");
        return;
    }
    TLOGI(WmsLogTag::WMS_SYSTEM, "[FB]OnupdateFloatingBallAni finish");
}

void AniFbController::StopFloatingBallAni(ani_env* env, ani_object obj, ani_long nativeObj)
{
    TLOGI(WmsLogTag::WMS_SYSTEM, "[FB]start");
    // check nullptr
    AniFbController* aniFbController = reinterpret_cast<AniFbController*>(nativeObj);
    if (aniFbController == nullptr) {
        AniThrowError<WmErrorCode>(env, WmErrorCode::WM_ERROR_FB_INTERNAL_ERROR,
            "AniFbController* aniFbController for nativeObj is nullptr");
        return;
    }
    // working and unregister
    if (aniFbController->OnstopFloatingBallAni(env)) {
        ani_ref undef = AniGetUndefined(env);
        aniFbController->OnUnRegisterCallback(env, FbListenerType::CLICK_CB, undef, aniFbController);
    } else {
        AniThrowError<WmErrorCode>(env, WmErrorCode::WM_ERROR_FB_INTERNAL_ERROR, "[FB]OnstopFloatingBallAni failed");
    }
}

bool AniFbController::OnstopFloatingBallAni(ani_env* env)
{
    TLOGI(WmsLogTag::WMS_SYSTEM, "[FB]start");
    // check nullptr
    if (fbController_ == nullptr) {
        AniThrowError<WmErrorCode>(env, WmErrorCode::WM_ERROR_FB_INTERNAL_ERROR, "[FB]fbController_ is nullptr");
        return false;
    }
    // working
    WMError errCode = fbController_->StopFloatingBallFromClient();
    // check result
    if (errCode != WMError::WM_OK) {
        AniThrowError<WmErrorCode>(env, WmErrorCode::WM_ERROR_FB_INTERNAL_ERROR, "[FB]internal error");
        return false;
    }
    return true;
}

ani_object AniFbController::GetFloatingBallWindowInfoAni(ani_env* env, ani_object obj, ani_long nativeObj)
{
    TLOGI(WmsLogTag::WMS_SYSTEM, "[FB]start");
    // check nullptr
    AniFbController* aniFbController = reinterpret_cast<AniFbController*>(nativeObj);
    if (aniFbController == nullptr) {
        return AniThrowError<WmErrorCode>(env, WmErrorCode::WM_ERROR_FB_INTERNAL_ERROR,
            "[FB]AniFbController* aniFbController for nativeObj is nullptr");
    }
    // working
    return aniFbController->OnGetFloatingBallWindowInfoAni(env);
}

ani_object AniFbController::OnGetFloatingBallWindowInfoAni(ani_env* env)
{
    TLOGI(WmsLogTag::WMS_SYSTEM, "[FB]start");
    // check nullptr
    if (fbController_ == nullptr) {
        TLOGE(WmsLogTag::WMS_SYSTEM, "[FB]AniFbController is nullptr");
        AniThrowError<WMError>(env, WMError::WM_ERROR_FB_INTERNAL_ERROR, "[FB]floating ball internal error");
        return static_cast<ani_object>(AniGetUndefined(env));
    }
    // get windowId
    uint32_t windowId;
    WMError errCode = fbController_->GetFloatingBallWindowInfo(windowId);
    if (errCode != WMError::WM_OK) {
        AniThrowError<WMError>(env, WMError::WM_ERROR_FB_INTERNAL_ERROR, "[FB]GetFloatingBallWindowInfo failed");
        return static_cast<ani_object>(AniGetUndefined(env));
    }
    // wrap windowId to ani_interface(ani_object)
    return CreateFloatingBallWindowInfoObject(env, windowId);
}

// Wrap the windowId into an object available for the ets layer
ani_object AniFbController::CreateFloatingBallWindowInfoObject(ani_env* env, uint32_t& windowId)
{
    TLOGI(WmsLogTag::WMS_SYSTEM, "[FB]wrap windowId to ani_interface(ani_object) start");
    // find class of interface
    ani_class cls = nullptr;
    ani_status ret;
    if ((ret = env->FindClass("@ohos.window.floatingBall.floatingBall.FloatingBallWindowInfoInner", &cls)) != ANI_OK) {
        TLOGE(WmsLogTag::WMS_SYSTEM, "[FB]failed to find class FloatingBallWindowInfoInner %{public}u", ret);
        return static_cast<ani_object>(AniGetUndefined(env));
    }
    // find initFunc of interface
    ani_method initFunc = nullptr;
    if ((ret = env->Class_FindMethod(cls, "<ctor>", "i:", &initFunc)) != ANI_OK) {
        TLOGE(WmsLogTag::WMS_SYSTEM, "[FB]get ctor constructor failed %{public}u", ret);
        return static_cast<ani_object>(AniGetUndefined(env));
    }
    // init windowId of interface
    ani_object floatingBallWindowInfoObject = {};
    int windowIdAsInt = static_cast<int>(windowId);
    if ((ret = env->Object_New(cls, initFunc, &floatingBallWindowInfoObject, windowIdAsInt)) != ANI_OK) {
        TLOGE(WmsLogTag::WMS_SYSTEM, "[FB]failed to create floatingBallWindowInfoObject");
        return static_cast<ani_object>(AniGetUndefined(env));
    }
    return floatingBallWindowInfoObject;
}

void AniFbController::RestoreMainWindowAni(ani_env* env, ani_object obj, ani_long nativeObj, ani_object want)
{
    // check want
    if (want == nullptr) {
        TLOGE(WmsLogTag::WMS_SYSTEM, "[FB]want is nullptr");
        return;
    }
    ani_boolean isUndefined = false;
    ani_boolean isNull = false;
    env->Reference_IsUndefined(want, &isUndefined);
    env->Reference_IsNull(want, &isNull);
    if (isUndefined || isNull) {
        AniThrowError<WmErrorCode>(env, WmErrorCode::WM_ERROR_FB_INTERNAL_ERROR, "[FB]want is undefined or null");
        return;
    }
    // check nullptr
    AniFbController* aniFbController = reinterpret_cast<AniFbController*>(nativeObj);
    if (aniFbController == nullptr) {
        AniThrowError<WmErrorCode>(env, WmErrorCode::WM_ERROR_FB_INTERNAL_ERROR,
            "[FB]AniFbController* aniFbController for nativeObj is nullptr");
        return;
    }
    // working
    aniFbController->OnrestoreMainWindowAni(env, want);
}

void AniFbController::OnrestoreMainWindowAni(ani_env* env, ani_object want)
{
    TLOGI(WmsLogTag::WMS_SYSTEM, "[FB]start");
    // get AAFwk::Want
    AAFwk::Want wantValue;
    if (!AppExecFwk::UnwrapWant(env, want, wantValue)) {
        AniThrowError<WmErrorCode>(env, WmErrorCode::WM_ERROR_FB_INTERNAL_ERROR, "[FB]unWrap want failed");
        return;
    }
    // working
    std::shared_ptr<AAFwk::Want> abilityWant = std::make_shared<AAFwk::Want>(wantValue);
    WMError errCode = fbController_->RestoreMainWindow(abilityWant);
    // check result
    if (errCode != WMError::WM_OK) {
        AniThrowError<WmErrorCode>(env, WmErrorCode::WM_ERROR_FB_INTERNAL_ERROR, "[FB]internal error");
        return;
    }
}

bool AniFbController::IsCallbackRegistered(ani_env* env, FbListenerType fbListenerType, ani_ref callback)
{
    std::lock_guard<std::mutex> lock(mtxListener_);
    // check typeCallbackListenerMap_
    if (typeCallbackListenerMap_.empty() ||
        typeCallbackListenerMap_.find(fbListenerType) == typeCallbackListenerMap_.end()) {
        return false;
    }
    // check typeCallbackListenerMap_[fbListenerType]
    for (auto iter = typeCallbackListenerMap_[fbListenerType].begin();
            iter != typeCallbackListenerMap_[fbListenerType].end(); ++iter) {
        ani_boolean isEquals = 0;
        env->Reference_StrictEquals(callback, iter->first, &isEquals);
        if (isEquals) {
            return true;
        }
    }
    // final result
    return false;
}

void AniFbController::RegisterFbOnClickCallback(ani_env* env, ani_object obj, ani_long nativeObj, ani_ref callback)
{
    TLOGI(WmsLogTag::DEFAULT, "[FB]start");
    // check nullptr
    AniFbController* aniFbController = reinterpret_cast<AniFbController*>(nativeObj);
    if (aniFbController == nullptr) {
        AniThrowError<WmErrorCode>(env, WmErrorCode::WM_ERROR_FB_INTERNAL_ERROR,
            "[FB]AniFbController* aniFbController for nativeObj is nullptr");
        return;
    }
    // working
    aniFbController->OnRegisterFbOnClickCallback(env, callback, aniFbController);
}

void AniFbController::OnRegisterFbOnClickCallback(ani_env* env, ani_ref callback, AniFbController*& aniFbController)
{
    TLOGI(WmsLogTag::DEFAULT, "[FB]start");
    // working
    WmErrorCode ret = aniFbController->RegisterListenerOnClick(env, callback);
    // check result
    if (ret != WmErrorCode::WM_OK) {
        AniThrowError<WmErrorCode>(env, ret, "[FB]fbController_->RegisterListener failed");
        return;
    }
}

WmErrorCode AniFbController::RegisterListenerOnClick(ani_env* env, ani_ref callback)
{
    TLOGI(WmsLogTag::DEFAULT, "[FB]start");
    // IsCallbackRegistered
    if (IsCallbackRegistered(env, FbListenerType::CLICK_CB, callback)) {
        return WmErrorCode::WM_OK;
    }
    // parameters prepare
    ani_ref cbRef{};
    if (env->GlobalReference_Create(callback, &cbRef) != ANI_OK) {
        TLOGE(WmsLogTag::DEFAULT, "[FB]create global ref fail");
        return WmErrorCode::WM_ERROR_STATE_ABNORMALLY;
    };
    ani_vm* vm = nullptr;
    ani_status aniRet = env->GetVM(&vm);
    if (aniRet != ANI_OK || vm == nullptr) {
        TLOGE(WmsLogTag::DEFAULT, "[FB]Get VM failed, ret: %{public}u", aniRet);
        env->GlobalReference_Delete(cbRef);
        return WmErrorCode::WM_ERROR_STATE_ABNORMALLY;
    }
    // listener to instance
    auto fbControllerListener = sptr<AniFbWindowListener>::MakeSptr(env, vm, cbRef);
    if (fbControllerListener == nullptr) {
        TLOGE(WmsLogTag::DEFAULT, "[FB]New AniFbWindowListener failed");
        env->GlobalReference_Delete(cbRef);
        return WmErrorCode::WM_ERROR_STATE_ABNORMALLY;
    }
    // SetMainEventHandler
    fbControllerListener->SetMainEventHandler();
    // working
    WmErrorCode ret = ProcessOnClickListener(fbControllerListener);
    if (ret != WmErrorCode::WM_OK) {
        TLOGE(WmsLogTag::DEFAULT, "[FB]ProcessOnClickListener failed");
        env->GlobalReference_Delete(cbRef);
        return ret;
    }
    // save in typeCallbackListenerMap_
    typeCallbackListenerMap_[FbListenerType::CLICK_CB][cbRef] = fbControllerListener;
    TLOGI(WmsLogTag::DEFAULT, "[FB]Register RegisterListenerOnClick success! callback map size: %{public}zu",
        typeCallbackListenerMap_[FbListenerType::CLICK_CB].size());
    // final result
    return WmErrorCode::WM_OK;
}

WmErrorCode AniFbController::ProcessOnClickListener(sptr<AniFbWindowListener>& listener)
{
    TLOGI(WmsLogTag::DEFAULT, "[FB]start");
    // working
    WMError ret = WMError::WM_OK;
    ret = fbController_->RegisterFbClickObserver(listener);
    // convert result
    WmErrorCode res = static_cast<WmErrorCode>(ret);
    return res;
}

void AniFbController::RegisterFbOnStateChangeCallback(ani_env* env,
                                                      ani_object obj,
                                                      ani_long nativeObj,
                                                      ani_ref callback)
{
    TLOGI(WmsLogTag::DEFAULT, "[FB]start");
    // check nullptr
    AniFbController* aniFbController = reinterpret_cast<AniFbController*>(nativeObj);
    if (aniFbController == nullptr) {
        AniThrowError<WmErrorCode>(env, WmErrorCode::WM_ERROR_FB_INTERNAL_ERROR,
            "[FB]AniFbController* aniFbController for nativeObj is nullptr");
        return;
    }
    // working
    aniFbController->OnRegisterFbOnStateChangeCallback(env, callback, aniFbController);
}

void AniFbController::OnRegisterFbOnStateChangeCallback(ani_env* env,
                                                        ani_ref callback,
                                                        AniFbController*& aniFbController)
{
    TLOGI(WmsLogTag::DEFAULT, "[FB]start");
    // working
    WmErrorCode ret = aniFbController->RegisterListenerOnStateChange(env, callback);
    // check result
    if (ret != WmErrorCode::WM_OK) {
        AniThrowError<WmErrorCode>(env, ret, "[FB]fbController_->RegisterListener failed");
        return;
    }
}

WmErrorCode AniFbController::RegisterListenerOnStateChange(ani_env* env, ani_ref callback)
{
    TLOGI(WmsLogTag::DEFAULT, "[FB]start");
    // IsCallbackRegistered
    if (IsCallbackRegistered(env, FbListenerType::STATE_CHANGE_CB, callback)) {
        return WmErrorCode::WM_OK;
    }
    // parameters prepare
    ani_ref cbRef{};
    if (env->GlobalReference_Create(callback, &cbRef) != ANI_OK) {
        TLOGE(WmsLogTag::DEFAULT, "[FB]create global ref fail");
        return WmErrorCode::WM_ERROR_STATE_ABNORMALLY;
    };
    ani_vm* vm = nullptr;
    ani_status aniRet = env->GetVM(&vm);
    if (aniRet != ANI_OK || vm == nullptr) {
        TLOGE(WmsLogTag::DEFAULT, "[FB]Get VM failed, ret: %{public}u", aniRet);
        env->GlobalReference_Delete(cbRef);
        return WmErrorCode::WM_ERROR_STATE_ABNORMALLY;
    }
    // listener to instance
    auto fbControllerListener = sptr<AniFbWindowListener>::MakeSptr(env, vm, cbRef);
    if (fbControllerListener == nullptr) {
        TLOGE(WmsLogTag::DEFAULT, "[FB]New AniFbWindowListener failed");
        env->GlobalReference_Delete(cbRef);
        return WmErrorCode::WM_ERROR_STATE_ABNORMALLY;
    }
    // SetMainEventHandler
    fbControllerListener->SetMainEventHandler();
    // working
    WmErrorCode ret = ProcessOnStateChangeListener(fbControllerListener);
    if (ret != WmErrorCode::WM_OK) {
        TLOGE(WmsLogTag::DEFAULT, "[FB]ProcessOnStateChangeListener failed");
        env->GlobalReference_Delete(cbRef);
        return ret;
    }
    // save in typeCallbackListenerMap_
    typeCallbackListenerMap_[FbListenerType::STATE_CHANGE_CB][cbRef] = fbControllerListener;
    // log
    TLOGI(WmsLogTag::DEFAULT,
        "[FB]Register RegisterListenerOnStateChange success! callback map size: %{public}zu",
        typeCallbackListenerMap_[FbListenerType::STATE_CHANGE_CB].size());
    return WmErrorCode::WM_OK;
}

WmErrorCode AniFbController::ProcessOnStateChangeListener(sptr<AniFbWindowListener>& listener)
{
    TLOGI(WmsLogTag::DEFAULT, "[FB]start");
    // working
    WMError ret = WMError::WM_OK;
    ret = fbController_->RegisterFbLifecycle(listener);
    // convert result
    WmErrorCode res = static_cast<WmErrorCode>(ret);
    return res;
};

void AniFbController::UnRegisterFbOnClickCallback(ani_env* env,
                                                  ani_object obj,
                                                  ani_long nativeObj,
                                                  ani_ref callback)
{
    TLOGI(WmsLogTag::DEFAULT, "[FB]start");
    // check nullptr
    AniFbController* aniFbController = reinterpret_cast<AniFbController*>(nativeObj);
    if (aniFbController == nullptr) {
        AniThrowError<WmErrorCode>(env, WmErrorCode::WM_ERROR_FB_INTERNAL_ERROR,
            "[FB]AniFbController* aniFbController for nativeObj is nullptr");
        return;
    }
    // working
    aniFbController->OnUnRegisterCallback(env, FbListenerType::CLICK_CB, callback, aniFbController);
}

void AniFbController::UnRegisterFbOnStateChangeCallback(ani_env* env,
                                                        ani_object obj,
                                                        ani_long nativeObj,
                                                        ani_ref callback)
{
    TLOGI(WmsLogTag::DEFAULT, "[FB]start");
    // check nullptr
    AniFbController* aniFbController = reinterpret_cast<AniFbController*>(nativeObj);
    if (aniFbController == nullptr) {
        AniThrowError<WmErrorCode>(env, WmErrorCode::WM_ERROR_FB_INTERNAL_ERROR,
            "[FB]AniFbController* aniFbController for nativeObj is nullptr");
        return;
    }
    // working
    aniFbController->OnUnRegisterCallback(env, FbListenerType::STATE_CHANGE_CB, callback, aniFbController);
}

void AniFbController::OnUnRegisterCallback(ani_env* env,
                                           FbListenerType fbListenerType,
                                           ani_ref callback,
                                           AniFbController*& aniFbController)
{
    TLOGI(WmsLogTag::DEFAULT, "[FB]start");
    std::lock_guard<std::mutex> lock(mtxListener_);
    // registerted check
    if (typeCallbackListenerMap_.empty() ||
        typeCallbackListenerMap_.find(fbListenerType) == typeCallbackListenerMap_.end()) {
        return;
    }
    ani_boolean isUndef = ANI_FALSE;
    env->Reference_IsUndefined(callback, &isUndef);
    if (isUndef == ANI_TRUE) {
        for (auto it = typeCallbackListenerMap_[fbListenerType].begin();
            it != typeCallbackListenerMap_[fbListenerType].end();) {
            WMError ret = aniFbController->UnRegisterListener(fbListenerType, it->second);
            if (ret != WMError::WM_OK) {
                TLOGE(WmsLogTag::DEFAULT, "[FB]Unregister one FbListenerType code %{public}u failed", fbListenerType);
                ++it;
                continue;
            }
            env->GlobalReference_Delete(it->second->GetAniCallback());
            it->second->SetAniCallback(nullptr);
            it = typeCallbackListenerMap_[fbListenerType].erase(it);
        }
    } else {
        TLOGI(WmsLogTag::WMS_SYSTEM, "[FB]FbListenerType %{public}u,start unregister one callback", fbListenerType);
        for (auto it = typeCallbackListenerMap_[fbListenerType].begin();
            it != typeCallbackListenerMap_[fbListenerType].end();
            ++it) {
            ani_boolean isEquals = 0;
            env->Reference_StrictEquals(callback, it->first, &isEquals);
            if (!isEquals) {continue;}
            WMError ret = aniFbController->UnRegisterListener(fbListenerType, it->second);
            if (ret != WMError::WM_OK) {
                TLOGE(WmsLogTag::DEFAULT, "[FB]Unregister FbListenerType code %{public}u failed", fbListenerType);
                break;
            }
            env->GlobalReference_Delete(it->second->GetAniCallback());
            it->second->SetAniCallback(nullptr);
            it = typeCallbackListenerMap_[fbListenerType].erase(it);
            break;
        }
    }
    auto it = typeCallbackListenerMap_.find(fbListenerType);
    if (it != typeCallbackListenerMap_.end() && it->second.empty()) {
        typeCallbackListenerMap_.erase(it);
    }
    LogTypeCallbackListenerMapSize(fbListenerType);
}

void AniFbController::LogTypeCallbackListenerMapSize(FbListenerType fbListenerType)
{
    auto it = typeCallbackListenerMap_.find(fbListenerType);
    if (it != typeCallbackListenerMap_.end()) {
        TLOGI(WmsLogTag::DEFAULT, "[FB]latest 【%{public}u】 callback map size: %{public}zu",
            fbListenerType, it->second.size());
    } else {
        TLOGI(WmsLogTag::DEFAULT, "[FB]latest 【%{public}u】 callback map not found, size: 0", fbListenerType);
    }
}

WMError AniFbController::UnRegisterListener(FbListenerType fbListenerType, sptr<AniFbWindowListener>& listener)
{
    switch (fbListenerType) {
        case FbListenerType::CLICK_CB:
            return fbController_->UnRegisterFbClickObserver(listener);
        case FbListenerType::STATE_CHANGE_CB:
            return fbController_->UnRegisterFbLifecycle(listener);
        default:
            return WMError::WM_ERROR_FB_PARAM_INVALID;
    }
}
} // namespace Rosen
} // namespace OHOS

ani_status OHOS::Rosen::ANI_Controller_Constructor(ani_vm *vm, uint32_t *result)
{
    // ets ANI Construct
    ani_status ret;
    ani_env* env;
    TLOGI(WmsLogTag::WMS_SYSTEM, "[FB]ANI_Controller_Constructor start");
    if ((ret = vm->GetEnv(ANI_VERSION_1, &env)) != ANI_OK) {
        TLOGE(WmsLogTag::WMS_SYSTEM, "[FB]null env");
        return ANI_NOT_FOUND;
    }
    // find namespace from @ohos.window.floatingBall.ets
    ani_class cls = nullptr;
    if ((ret = env->FindClass("@ohos.window.floatingBall.floatingBall.FbControllerInternal", &cls)) != ANI_OK) {
        TLOGD(WmsLogTag::WMS_SYSTEM, "[FB]ANI_Controller_Constructor failed to find class, %{public}u", ret);
        return ANI_NOT_FOUND;
    }
    // Bind ets and cpp
    std::array methods = {
        ani_native_function {"startFloatingBallNative", nullptr,
            reinterpret_cast<void*>(AniFbController::StartFloatingBallAni)},
        ani_native_function {"updateFloatingBallNative", nullptr,
            reinterpret_cast<void*>(AniFbController::UpdateFloatingBallAni)},
        ani_native_function {"stopFloatingBallNative", "l:",
            reinterpret_cast<void*>(AniFbController::StopFloatingBallAni)},
        ani_native_function {"getFloatingBallWindowInfoNative", nullptr,
            reinterpret_cast<void*>(AniFbController::GetFloatingBallWindowInfoAni)},
        ani_native_function {"restoreMainWindowNative", nullptr,
            reinterpret_cast<void*>(AniFbController::RestoreMainWindowAni)},
        ani_native_function {"onClickNative", nullptr,
            reinterpret_cast<void*>(AniFbController::RegisterFbOnClickCallback)},
        ani_native_function {"onStateChangeNative", nullptr,
            reinterpret_cast<void*>(AniFbController::RegisterFbOnStateChangeCallback)},
        ani_native_function {"offClickNative", nullptr,
            reinterpret_cast<void*>(AniFbController::UnRegisterFbOnClickCallback)},
        ani_native_function {"offStateChangeNative", nullptr,
            reinterpret_cast<void*>(AniFbController::UnRegisterFbOnStateChangeCallback)},
    };
    // Bind each method in the array
    for (const auto& method: methods) {
        if ((ret = env->Class_BindNativeMethods(cls, &method, 1u)) != ANI_OK) {
            TLOGE(WmsLogTag::WMS_SYSTEM, "[FB]bind window method fail %{public}u, %{public}s, %{public}s",
                ret, method.name, method.signature);
            return ANI_NOT_FOUND;
        }
    }
    // result
    *result = ANI_VERSION_1;
    TLOGI(WmsLogTag::WMS_SYSTEM, "[FB]ANI_Controller_Constructor finish");
    return ANI_OK;
}