/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#include "screen_ani_manager.h"

#include <algorithm>
#include <hitrace_meter.h>

#include "ani.h"
#include "ani_err_utils.h"
#include "dm_common.h"
#include "pixel_map_taihe_ani.h"
#include "refbase.h"
#include "screen_ani.h"
#include "screen_ani_listener.h"
#include "screen_ani_utils.h"
#include "singleton_container.h"
#include "window_manager_hilog.h"

namespace OHOS {
namespace Rosen {

ScreenManagerAni::ScreenManagerAni() {}

void ScreenManagerAni::RegisterCallback(ani_env* env, ani_string type, ani_ref callback, ani_long nativeObj)
{
    TLOGI(WmsLogTag::DMS, "[ANI] start to register screen callback: %{public}ld", (long)nativeObj);
    ScreenManagerAni* screenManagerAni = reinterpret_cast<ScreenManagerAni*>(nativeObj);
    if (screenManagerAni != nullptr) {
        screenManagerAni->OnRegisterCallback(env, type, callback);
    } else {
        TLOGE(WmsLogTag::DMS, "[ANI] screenManagerAni null ptr");
    }
}

void ScreenManagerAni::UnRegisterCallback(ani_env* env, ani_string type, ani_long nativeObj, ani_ref callback)
{
    TLOGI(WmsLogTag::DMS, "[ANI] begin");
    ScreenManagerAni* screenManagerAni = reinterpret_cast<ScreenManagerAni*>(nativeObj);
    if (screenManagerAni != nullptr) {
        screenManagerAni->OnUnRegisterCallback(env, type, callback);
    } else {
        TLOGI(WmsLogTag::DMS, "[ANI] null ptr");
    }
}

void ScreenManagerAni::OnRegisterCallback(ani_env* env, ani_string type, ani_ref callback)
{
    TLOGI(WmsLogTag::DMS, "[ANI] begin");
    ani_ref cbRef{};
    std::string typeString;
    ScreenAniUtils::GetStdString(env, type, typeString);
    if (env->GlobalReference_Create(callback, &cbRef) != ANI_OK) {
        TLOGE(WmsLogTag::DMS, "[ANI] create global ref fail");
        env->GlobalReference_Delete(cbRef);
        return;
    }
    std::lock_guard<std::mutex> lock(mtx_);
    if (IsCallbackRegistered(env, typeString, cbRef)) {
        TLOGI(WmsLogTag::DMS, "[ANI] type %{public}s callback already registered!", typeString.c_str());
        env->GlobalReference_Delete(cbRef);
        return;
    }
    ani_boolean callbackUndefined = 0;
    env->Reference_IsUndefined(cbRef, &callbackUndefined);
    if (callbackUndefined) {
        TLOGE(WmsLogTag::DMS, "undefined");
        env->GlobalReference_Delete(cbRef);
        std::string errMsg = "[ANI] failed to register screen listener with type, cbk null or undefined";
        AniErrUtils::ThrowBusinessError(env, DmErrorCode::DM_ERROR_INVALID_PARAM, errMsg);
        return;
    }
    TLOGI(WmsLogTag::DMS, "create listener");
    sptr<ScreenAniListener> screenAniListener = new (std::nothrow) ScreenAniListener(env);
    if (screenAniListener == nullptr) {
        TLOGE(WmsLogTag::DMS, "[ANI] screenListener is nullptr");
        env->GlobalReference_Delete(cbRef);
        AniErrUtils::ThrowBusinessError(env, DMError::DM_ERROR_INVALID_PARAM, "screenListener is nullptr");
        return;
    }
    screenAniListener->AddCallback(typeString, cbRef);
    screenAniListener->SetMainEventHandler();
    DmErrorCode ret = ProcessRegisterCallback(env, typeString, screenAniListener);
    if (ret != DmErrorCode::DM_OK) {
        TLOGE(WmsLogTag::DMS, "[ANI] register screen listener with type, errcode: %{public}d", ret);
        env->GlobalReference_Delete(cbRef);
        std::string errMsg = "Failed to register screen listener with type";
        AniErrUtils::ThrowBusinessError(env, ret, errMsg);
        return;
    }
    // add listener to map
    jsCbMap_[typeString][cbRef] = screenAniListener;
}

DmErrorCode ScreenManagerAni::ProcessRegisterCallback(ani_env* env,
                                                      std::string& typeStr,
                                                      sptr<ScreenAniListener> screenAniListener)
{
    DmErrorCode ret = DmErrorCode::DM_ERROR_INVALID_PARAM;
    if (typeStr == ANI_EVENT_CHANGE || typeStr == ANI_EVENT_CONNECT || typeStr == ANI_EVENT_DISCONNECT) {
        TLOGI(WmsLogTag::DMS, "ProcessRegisterCallback %{public}s", typeStr.c_str());
        ret = DM_JS_TO_ERROR_CODE_MAP.at(
            SingletonContainer::Get<ScreenManager>().RegisterScreenListener(screenAniListener));
    }
    return ret;
}

void ScreenManagerAni::OnUnRegisterCallback(ani_env* env, ani_string type, ani_ref callback)
{
    TLOGI(WmsLogTag::DMS, "[ANI] begin");
    std::string typeString;
    ScreenAniUtils::GetStdString(env, type, typeString);
    std::lock_guard<std::mutex> lock(mtx_);
    ani_boolean callbackUndefined = 0;
    env->Reference_IsUndefined(callback, &callbackUndefined);
    ani_ref cbRef{};
    if (ANI_OK != env->GlobalReference_Create(callback, &cbRef)) {
        env->GlobalReference_Delete(cbRef);
        TLOGE(WmsLogTag::DMS, "[ANI]create global ref fail");
        return;
    }
    DmErrorCode ret;
    if (callbackUndefined) {
        TLOGI(WmsLogTag::DMS, "[ANI] OnUnRegisterCallback for all");
        ret = DM_JS_TO_ERROR_CODE_MAP.at(UnRegisterAllScreenListenerWithType(typeString));
    } else {
        TLOGI(WmsLogTag::DMS, "[ANI] OnUnRegisterCallback with type");
        ret = DM_JS_TO_ERROR_CODE_MAP.at(UnRegisterScreenListenerWithType(typeString, env, cbRef));
    }

    if (ret != DmErrorCode::DM_OK) {
        std::string errMsg = "[ANI] failed to unregister screen listener with type";
        TLOGE(WmsLogTag::DMS, "[ANI] failed to unregister screen listener with type");
        AniErrUtils::ThrowBusinessError(env, ret, errMsg);
    }
    env->GlobalReference_Delete(cbRef);
}

bool ScreenManagerAni::IsCallbackRegistered(ani_env* env, const std::string& type, ani_ref callback)
{
    if (jsCbMap_.empty() || jsCbMap_.find(type) == jsCbMap_.end()) {
        TLOGI(WmsLogTag::DMS, "method %{public}s not registered!", type.c_str());
        return false;
    }
    for (const auto& iter : jsCbMap_[type]) {
        ani_boolean isEquals = false;
        env->Reference_StrictEquals(callback, iter.first, &isEquals);
        if (isEquals) {
            TLOGE(WmsLogTag::DMS, "callback already registered!");
            return true;
        }
    }
    return false;
}

DMError ScreenManagerAni::UnRegisterScreenListenerWithType(std::string type, ani_env* env, ani_ref callback)
{
    TLOGI(WmsLogTag::DMS, "[ANI] UnRegisterScreenListenerWithType begin");
    DMError ret = DMError::DM_OK;
    if (jsCbMap_.empty() || jsCbMap_.find(type) == jsCbMap_.end()) {
        TLOGI(WmsLogTag::DMS, "[ANI] methodName %{public}s not registered!", type.c_str());
        return ret;
    }
    ani_ref cbRef{};
    if (env->GlobalReference_Create(callback, &cbRef) != ANI_OK) {
        TLOGE(WmsLogTag::DMS, "[ANI]create global ref fail");
        return DMError::DM_ERROR_INVALID_PARAM;
    }
    for (auto it = jsCbMap_[type].begin(); it != jsCbMap_[type].end(); it++) {
        ani_boolean isEquals = 0;
        env->Reference_StrictEquals(cbRef, it->first, &isEquals);
        if (isEquals) {
            it->second->RemoveCallback(env, type, callback);
            if (type == ANI_EVENT_CONNECT || type == ANI_EVENT_DISCONNECT || type == ANI_EVENT_CHANGE) {
                TLOGI(WmsLogTag::DMS, "[ANI] start to unregis screen event listener! event = %{public}s", type.c_str());
                sptr<ScreenManager::IScreenListener> thisListener(it->second);
                ret = SingletonContainer::Get<ScreenManager>().UnregisterScreenListener(thisListener);
            }
            jsCbMap_[type].erase(it);
            break;
        }
    }
    if (jsCbMap_[type].empty()) {
        jsCbMap_.erase(type);
    }
    env->GlobalReference_Delete(cbRef);
    return ret;
}

DMError ScreenManagerAni::UnRegisterAllScreenListenerWithType(std::string type)
{
    TLOGI(WmsLogTag::DMS, "[ANI] begin");
    if (jsCbMap_.empty() || jsCbMap_.find(type) == jsCbMap_.end()) {
        TLOGI(WmsLogTag::DMS,
              "[ANI] UnregisterAllScreenListenerWithType methodName %{public}s not registered!",
              type.c_str());
        return DMError::DM_OK;
    }
    DMError ret = DMError::DM_OK;
    for (auto it = jsCbMap_[type].begin(); it != jsCbMap_[type].end();) {
        it->second->RemoveAllCallback();
        if (type == ANI_EVENT_CONNECT || type == ANI_EVENT_DISCONNECT || type == ANI_EVENT_CHANGE) {
            sptr<ScreenManager::IScreenListener> thisListener(it->second);
            ret = SingletonContainer::Get<ScreenManager>().UnregisterScreenListener(thisListener);
        }
    }
    return ret;
}

ani_long ScreenManagerAni::MakeMirror(ani_env* env, ani_long mainScreen, ani_object mirrorScreen)
{
    ani_int length = 0;
    std::vector<ScreenId> screenIds;
    env->Object_GetPropertyByName_Int(mirrorScreen, "length", &length);
    TLOGI(WmsLogTag::DMS, "[ANI] length %{public}d", (ani_int)length);
    for (int32_t i = 0; i < length; i++) {
        ani_ref screenIdRef;
        auto ret = env->Object_CallMethodByName_Ref(mirrorScreen, "$_get", "i:Y", &screenIdRef, (ani_int)i);
        if (ANI_OK != ret) {
            TLOGE(WmsLogTag::DMS, "[ANI] get ani_array index %{public}u fail, ret: %{public}u", (ani_int)i, ret);
            AniErrUtils::ThrowBusinessError(env, DmErrorCode::DM_ERROR_INVALID_PARAM, "Failed to get screenId");
            return static_cast<ani_long>(INVALID_SCREEN_ID);
        }
        ani_long screenId;
        ret = env->Object_CallMethodByName_Long(static_cast<ani_object>(screenIdRef), "toLong", ":l", &screenId);
        if (ANI_OK != ret) {
            TLOGE(WmsLogTag::DMS, "[ANI] unbox screenId failed, ret: %{public}u", ret);
            AniErrUtils::ThrowBusinessError(env, DmErrorCode::DM_ERROR_INVALID_PARAM, "Failed to unbox screenId");
            return static_cast<ani_long>(INVALID_SCREEN_ID);
        }
        screenIds.emplace_back(static_cast<ScreenId>(screenId));
    }
    ScreenId screenGroupId = INVALID_SCREEN_ID;
    DmErrorCode ret = DM_JS_TO_ERROR_CODE_MAP.at(SingletonContainer::Get<ScreenManager>().MakeMirror(
        static_cast<ScreenId>(mainScreen), screenIds, screenGroupId));
    if (ret != DmErrorCode::DM_OK) {
        AniErrUtils::ThrowBusinessError(env, ret, "JsScreenManager::OnMakeMirror failed.");
        return static_cast<ani_long>(INVALID_SCREEN_ID);
    }
    return static_cast<ani_long>(screenGroupId);
}

ani_status ScreenManagerAni::InitScreenManagerAni(ani_namespace screenNameSpace, ani_env* env)
{
    TLOGI(WmsLogTag::DEFAULT, "[ANI] begin");
    ani_function setObjFunc = nullptr;
    ani_status ret = env->Namespace_FindFunction(screenNameSpace, "setScreenMgrRef", "l:", &setObjFunc);
    if (ret != ANI_OK) {
        TLOGE(WmsLogTag::DEFAULT, "[ANI] find setNativeObj func fail %{public}u", ret);
        return ret;
    }
    std::unique_ptr<ScreenManagerAni> aniScreenManager = std::make_unique<ScreenManagerAni>();
    ret = env->Function_Call_Void(setObjFunc, aniScreenManager.release());
    if (ret != ANI_OK) {
        TLOGE(WmsLogTag::DEFAULT, "[ANI] find setNativeObj func fail %{public}u", ret);
        return ret;
    }
    return ret;
}

void ScreenManagerAni::GetAllScreens(ani_env* env, ani_object screensAni)
{
    HITRACE_METER_FMT(HITRACE_TAG_WINDOW_MANAGER, "JsScreenManager::OnGetAllScreens");
    TLOGI(WmsLogTag::DMS, "[ANI] start");
    std::vector<sptr<Screen>> screens;
    auto res = DM_JS_TO_ERROR_CODE_MAP.at(SingletonContainer::Get<ScreenManager>().GetAllScreens(screens));
    if (res != DmErrorCode::DM_OK) {
        TLOGE(WmsLogTag::DMS, "[ANI] JsScreenManager::OnGetAllScreens failed.");
        AniErrUtils::ThrowBusinessError(env, res, "JsScreenManager::OnGetAllScreens failed.");
    } else if (!screens.empty()) {
        TLOGI(WmsLogTag::DMS,
              "[ANI] JsScreenManager::OnGetAllScreens succeed. size = %{public}u",
              static_cast<uint32_t>(screens.size()));
        if (ANI_OK != ScreenAniUtils::ConvertScreens(env, screens, screensAni)) {
            TLOGE(WmsLogTag::DMS, "[ANI] ConvertScreens fail");
        }
    } else {
        TLOGE(WmsLogTag::DMS, "[ANI] JsScreenManager::OnGetAllScreens null.");
        AniErrUtils::ThrowBusinessError(
            env, DmErrorCode::DM_ERROR_INVALID_SCREEN, "JsScreenManager::OnGetAllScreens failed.");
    }
}

void ScreenManagerAni::CreateVirtualScreen(ani_env* env, ani_object options, ani_object virtualScreen)
{
    TLOGI(WmsLogTag::DMS, "[ANI] start");
    if (env == nullptr) {
        TLOGE(WmsLogTag::DMS, "[ANI] env is nullptr");
        return;
    }
    ani_boolean undefRes = false;
    env->Reference_IsUndefined(options, &undefRes);
    if (undefRes != 0) {
        TLOGE(WmsLogTag::DMS, "[ANI] options is undefined or null");
        AniErrUtils::ThrowBusinessError(env, DmErrorCode::DM_ERROR_INVALID_PARAM, "options is undefined or null");
        return;
    }

    VirtualScreenOption option;
    auto ret = ScreenAniUtils::GetVirtualScreenOption(env, options, option);
    if (ret != DmErrorCode::DM_OK) {
        TLOGE(WmsLogTag::DMS, "[ANI] Get virtual screen options failed");
        AniErrUtils::ThrowBusinessError(env, ret, "Get virtual screen options failed");
        return;
    }

    auto screenId = SingletonContainer::Get<ScreenManager>().CreateVirtualScreen(option);
    auto screen = SingletonContainer::Get<ScreenManager>().GetScreenById(screenId);
    if (screen == nullptr) {
        ret = DmErrorCode::DM_ERROR_INVALID_SCREEN;
        if (screenId == ERROR_ID_NOT_SYSTEM_APP) {
            ret = DmErrorCode::DM_ERROR_NOT_SYSTEM_APP;
        } else if (screenId == ERROR_ID_NO_PERMISSION) {
            ret = DmErrorCode::DM_ERROR_NO_PERMISSION;
        }
        TLOGE(WmsLogTag::DMS, "[ANI] Get virtual screen failed");
        AniErrUtils::ThrowBusinessError(env, ret, "Get virtual screen failed");
        return;
    }
    if (ANI_OK != ScreenAniUtils::ConvertScreen(env, screen, virtualScreen)) {
        TLOGE(WmsLogTag::DMS, "[ANI] ConvertScreen fail");
    }
}

void ScreenManagerAni::SetVirtualScreenSurface(ani_env* env, ani_long screenId, ani_string surfaceIdAni)
{
    TLOGI(WmsLogTag::DMS, "[ANI] start");
    if (env == nullptr) {
        TLOGE(WmsLogTag::DMS, "[ANI] env is nullptr");
        return;
    }
    sptr<Surface> surface;
    auto ret = ScreenAniUtils::GetSurfaceFromAni(env, surfaceIdAni, surface);
    if (ret != ANI_OK) {
        TLOGE(WmsLogTag::DMS, "Failed to get surface.");
        AniErrUtils::ThrowBusinessError(env, DmErrorCode::DM_ERROR_INVALID_PARAM, "Failed to get surface");
        return;
    }
    if (surface == nullptr) {
        TLOGE(WmsLogTag::DMS, "Set virtual screen surface failed, surface is nullptr.");
        AniErrUtils::ThrowBusinessError(env, DmErrorCode::DM_ERROR_INVALID_PARAM, "surface is nullptr");
        return;
    }
    DmErrorCode res = DM_JS_TO_ERROR_CODE_MAP.at(
        SingletonContainer::Get<ScreenManager>().SetVirtualScreenSurface(static_cast<ScreenId>(screenId), surface));
    if (res != DmErrorCode::DM_OK) {
        TLOGE(WmsLogTag::DMS, "ScreenManager::SetVirtualScreenSurface failed.");
        AniErrUtils::ThrowBusinessError(env, res, "ScreenManager::SetVirtualScreenSurface failed.");
        return;
    }
}

void ScreenManagerAni::DestroyVirtualScreen(ani_env* env, ani_long screenId)
{
    TLOGI(WmsLogTag::DMS, "[ANI] start");
    auto ret = DM_JS_TO_ERROR_CODE_MAP.at(
        SingletonContainer::Get<ScreenManager>().DestroyVirtualScreen(static_cast<ScreenId>(screenId)));
    if (ret != DmErrorCode::DM_OK) {
        TLOGE(WmsLogTag::DMS, "ScreenManager::DestroyVirtualScreen failed.");
        AniErrUtils::ThrowBusinessError(env, ret, "ScreenManager::DestroyVirtualScreen failed.");
        return;
    }
}

ani_boolean ScreenManagerAni::IsScreenRotationLocked(ani_env* env)
{
    TLOGI(WmsLogTag::DMS, "[ANI] start");
    bool isLocked = false;
    auto ret = DM_JS_TO_ERROR_CODE_MAP.at(SingletonContainer::Get<ScreenManager>().IsScreenRotationLocked(isLocked));
    if (ret != DmErrorCode::DM_OK) {
        TLOGE(WmsLogTag::DMS, "ScreenManager::IsScreenRotationLocked failed.");
        AniErrUtils::ThrowBusinessError(env, ret, "ScreenManager::IsScreenRotationLocked failed.");
        return isLocked;
    }
    TLOGNI(WmsLogTag::DMS, "IsScreenRotationLocked success");
    return isLocked;
}

void ScreenManagerAni::SetScreenRotationLocked(ani_env* env, ani_boolean isLocked)
{
    TLOGI(WmsLogTag::DMS, "[ANI] start");
    auto ret = DM_JS_TO_ERROR_CODE_MAP.at(SingletonContainer::Get<ScreenManager>().SetScreenRotationLocked(isLocked));
    if (ret != DmErrorCode::DM_OK) {
        TLOGE(WmsLogTag::DMS, "ScreenManager::SetScreenRotationLocked failed.");
        AniErrUtils::ThrowBusinessError(env, ret, "ScreenManager::SetScreenRotationLocked failed.");
        return;
    }
    TLOGNI(WmsLogTag::DMS, "SetScreenRotationLocked success");
}

void ScreenManagerAni::SetMultiScreenRelativePosition(ani_env* env,
                                                      ani_object mainScreenOptionsAni,
                                                      ani_object secondaryScreenOptionsAni)
{
    TLOGI(WmsLogTag::DMS, "[ANI] start");
    if (env == nullptr) {
        TLOGE(WmsLogTag::DMS, "[ANI] env is nullptr");
        return;
    }
    MultiScreenPositionOptions mainScreenOptions;
    auto ret = ScreenAniUtils::GetMultiScreenPositionOptionsFromAni(env, mainScreenOptionsAni, mainScreenOptions);
    if (ret != ANI_OK) {
        TLOGE(WmsLogTag::DMS, "Get mainScreenOptions failed.");
        AniErrUtils::ThrowBusinessError(env, DmErrorCode::DM_ERROR_INVALID_PARAM, "Get mainScreenOptions failed.");
        return;
    }
    MultiScreenPositionOptions secondScreenOption;
    ret = ScreenAniUtils::GetMultiScreenPositionOptionsFromAni(env, secondaryScreenOptionsAni, secondScreenOption);
    if (ret != ANI_OK) {
        TLOGE(WmsLogTag::DMS, "Get secondScreenOption failed.");
        AniErrUtils::ThrowBusinessError(env, DmErrorCode::DM_ERROR_INVALID_PARAM, "Get secondScreenOption failed.");
        return;
    }
    DmErrorCode res = DM_JS_TO_ERROR_CODE_MAP.at(
        SingletonContainer::Get<ScreenManager>().SetMultiScreenRelativePosition(mainScreenOptions, secondScreenOption));
    res = (res == DmErrorCode::DM_ERROR_INVALID_SCREEN) ? DmErrorCode::DM_ERROR_INVALID_PARAM : res;
    if (res != DmErrorCode::DM_OK) {
        TLOGE(WmsLogTag::DMS, "ScreenManager::SetMultiScreenRelativePosition failed.");
        AniErrUtils::ThrowBusinessError(env, res, "ScreenManager::SetMultiScreenRelativePosition failed.");
        return;
    }
    TLOGNI(WmsLogTag::DMS, "SetScreenRotationLocked success");
}

void ScreenManagerAni::SetMultiScreenMode(ani_env* env,
                                          ani_long primaryScreenId,
                                          ani_long secondaryScreenId,
                                          ani_enum_item secondaryScreenMode)
{
    TLOGI(WmsLogTag::DMS, "[ANI] start");
    if (env == nullptr) {
        TLOGE(WmsLogTag::DMS, "[ANI] env is nullptr");
        return;
    }
    ani_int screenModeInt = 0;
    ani_status ret = env->EnumItem_GetValue_Int(secondaryScreenMode, &screenModeInt);
    if (ret != ANI_OK) {
        TLOGE(WmsLogTag::DMS, "[ANI] Get screenMode failed, ret: %{public}u", ret);
        AniErrUtils::ThrowBusinessError(env, DmErrorCode::DM_ERROR_INVALID_PARAM, "Failed to Get screenMode");
        return;
    }
    MultiScreenMode screenMode = static_cast<MultiScreenMode>(screenModeInt);
    DmErrorCode res = DM_JS_TO_ERROR_CODE_MAP.at(SingletonContainer::Get<ScreenManager>().SetMultiScreenMode(
        static_cast<ScreenId>(primaryScreenId), static_cast<ScreenId>(secondaryScreenId), screenMode));
    if (res != DmErrorCode::DM_OK) {
        TLOGE(WmsLogTag::DMS, "ScreenManager::SetMultiScreenMode failed.");
        AniErrUtils::ThrowBusinessError(env, res, "ScreenManager::SetMultiScreenMode failed.");
    }
}

void ScreenManagerAni::SetScreenPrivacyMaskImage(ani_env* env, ani_long screenId, ani_object imageAni)
{
    TLOGI(WmsLogTag::DMS, "[ANI] start");
    if (env == nullptr) {
        TLOGE(WmsLogTag::DMS, "[ANI] env is nullptr");
        return;
    }
    std::shared_ptr<Media::PixelMap> privacyMaskImg;
    ani_boolean isImageAniUndefined = false;
    env->Reference_IsUndefined(imageAni, &isImageAniUndefined);
    if (!isImageAniUndefined) {
        privacyMaskImg = OHOS::Media::PixelMapTaiheAni::GetNativePixelMap(env, imageAni);
        if (privacyMaskImg == nullptr) {
            TLOGE(WmsLogTag::DMS, "[ANI] Failed to convert parameter to pixelmap.");
            AniErrUtils::ThrowBusinessError(
                env, DmErrorCode::DM_ERROR_INVALID_PARAM, "Failed to convert parameter to pixelmap.");
            return;
        }
    }
    auto res = DM_JS_TO_ERROR_CODE_MAP.at(SingletonContainer::Get<ScreenManager>().SetScreenPrivacyMaskImage(
        static_cast<ScreenId>(screenId), privacyMaskImg));
    if (res != DmErrorCode::DM_OK) {
        TLOGE(WmsLogTag::DMS, "ScreenManager::SetScreenPrivacyMaskImage failed.");
        AniErrUtils::ThrowBusinessError(env, res, "ScreenManager::SetScreenPrivacyMaskImage failed.");
    }
}

ani_object ScreenManagerAni::MakeUnique(ani_env* env, ani_object uniqueScreenIds)
{
    TLOGI(WmsLogTag::DMS, "[ANI] start");
    ani_boolean isUniqueScreenIdsUndefined = false;
    auto ret = env->Reference_IsUndefined(uniqueScreenIds, &isUniqueScreenIdsUndefined);
    if (ret != ANI_OK) {
        TLOGE(WmsLogTag::DMS, "[ANI] check uniqueScreenIds is undefined failed");
        return ScreenAniUtils::CreateAniUndefined(env);
    }
    if (isUniqueScreenIdsUndefined) {
        TLOGE(WmsLogTag::DMS, "[ANI] UniqueScreenIds is nullptr.");
        AniErrUtils::ThrowBusinessError(env, DmErrorCode::DM_ERROR_INVALID_PARAM, "UniqueScreenIds is nullptr.");
        return ScreenAniUtils::CreateAniUndefined(env);
    }
    std::vector<ScreenId> screenIds;
    ret = ScreenAniUtils::GetScreenIdArrayFromAni(env, uniqueScreenIds, screenIds);
    if (ret != ANI_OK) {
        TLOGE(WmsLogTag::DMS, "[ANI] get screenId array failed, ret: %{public}u", ret);
        AniErrUtils::ThrowBusinessError(env, DmErrorCode::DM_ERROR_INVALID_PARAM, "get screenId array failed");
        return ScreenAniUtils::CreateAniUndefined(env);
    }
    std::vector<DisplayId> displayIds;
    DmErrorCode res =
        DM_JS_TO_ERROR_CODE_MAP.at(SingletonContainer::Get<ScreenManager>().MakeUniqueScreen(screenIds, displayIds));
    if (res != DmErrorCode::DM_OK) {
        TLOGE(WmsLogTag::DMS, "ScreenManager::MakeUniqueScreen failed.");
        AniErrUtils::ThrowBusinessError(env, res, "ScreenManager::MakeUniqueScreen failed.");
    }
    TLOGI(WmsLogTag::DMS, "[ANI] displayIds length %{public}d", (ani_int)displayIds.size());
    return ScreenAniUtils::CreateDisplayIdVectorAniObject(env, displayIds);
}

ani_long ScreenManagerAni::MakeMirrorWithRegion(ani_env* env,
                                                ani_long mainScreen,
                                                ani_object mirrorScreen,
                                                ani_object mainScreenRegionAni)
{
    TLOGI(WmsLogTag::DMS, "[ANI] start");
    if (env == nullptr) {
        TLOGE(WmsLogTag::DMS, "[ANI] env is nullptr");
        return INVALID_SCREEN_ID;
    }
    if (mirrorScreen == nullptr || mainScreenRegionAni == nullptr) {
        TLOGE(WmsLogTag::DMS, "[ANI] mirrorScreen or mainScreenRegionAni is nullptr");
        AniErrUtils::ThrowBusinessError(
            env, DmErrorCode::DM_ERROR_INVALID_PARAM, "mirrorScreen or mainScreenRegion is nullptr");
        return static_cast<ani_long>(INVALID_SCREEN_ID);
    }
    std::vector<ScreenId> mirrorScreenIds;
    ani_status ret = ScreenAniUtils::GetScreenIdArrayFromAni(env, mirrorScreen, mirrorScreenIds);
    if (ret != ANI_OK) {
        TLOGE(WmsLogTag::DMS, "[ANI] get screenId array failed, ret: %{public}u", ret);
        AniErrUtils::ThrowBusinessError(env, DmErrorCode::DM_ERROR_INVALID_PARAM, "get screenId array failed");
        return static_cast<ani_long>(INVALID_SCREEN_ID);
    }
    DMRect mainScreenRegion;
    if (ScreenAniUtils::GetRectFromAni(env, mainScreenRegionAni, mainScreenRegion) != ANI_OK) {
        AniErrUtils::ThrowBusinessError(
            env, DmErrorCode::DM_ERROR_INVALID_PARAM, "Failed to convert to mainScreenRegion");
        return static_cast<ani_long>(INVALID_SCREEN_ID);
    }
    ScreenId screenGroupId = INVALID_SCREEN_ID;
    DmErrorCode res = DM_JS_TO_ERROR_CODE_MAP.at(SingletonContainer::Get<ScreenManager>().MakeMirror(
        static_cast<ScreenId>(mainScreen), mirrorScreenIds, mainScreenRegion, screenGroupId));
    if (res != DmErrorCode::DM_OK) {
        AniErrUtils::ThrowBusinessError(env, res, "ScreenManager::MakeMirror failed.");
        return static_cast<ani_long>(INVALID_SCREEN_ID);
    }
    return static_cast<ani_long>(screenGroupId);
}

void ScreenManagerAni::StopMirror(ani_env* env, ani_object mirrorScreen)
{
    TLOGI(WmsLogTag::DMS, "[ANI] start");
    if (env == nullptr) {
        TLOGE(WmsLogTag::DMS, "[ANI] env is nullptr");
        return;
    }
    if (mirrorScreen == nullptr) {
        TLOGE(WmsLogTag::DMS, "[ANI] mirrorScreen is nullptr");
        AniErrUtils::ThrowBusinessError(env, DmErrorCode::DM_ERROR_INVALID_PARAM, "mirrorScreen is nullptr");
        return;
    }
    std::vector<ScreenId> screenIds;
    ani_status ret = ScreenAniUtils::GetScreenIdArrayFromAni(env, mirrorScreen, screenIds);
    if (ret != ANI_OK) {
        TLOGE(WmsLogTag::DMS, "[ANI] get screenId array failed, ret: %{public}u", ret);
        AniErrUtils::ThrowBusinessError(env, DmErrorCode::DM_ERROR_INVALID_PARAM, "get screenId array failed");
        return;
    }
    DmErrorCode res = DM_JS_TO_ERROR_CODE_MAP.at(SingletonContainer::Get<ScreenManager>().StopMirror(screenIds));
    if (res != DmErrorCode::DM_OK) {
        AniErrUtils::ThrowBusinessError(env, res, "ScreenManager::StopMirror failed.");
    }
}

ani_long ScreenManagerAni::MakeExpand(ani_env* env, ani_object expandOptionsAni)
{
    TLOGI(WmsLogTag::DMS, "[ANI] start");
    if (env == nullptr) {
        TLOGE(WmsLogTag::DMS, "[ANI] env is nullptr");
        return static_cast<ani_long>(INVALID_SCREEN_ID);
    }
    if (expandOptionsAni == nullptr) {
        TLOGE(WmsLogTag::DMS, "[ANI] expandOptionsAni is nullptr");
        AniErrUtils::ThrowBusinessError(env, DmErrorCode::DM_ERROR_INVALID_PARAM, "expandOptionsAni is nullptr");
        return static_cast<ani_long>(INVALID_SCREEN_ID);
    }
    std::vector<ExpandOption> options;
    ani_int length = 0;
    env->Object_GetPropertyByName_Int(expandOptionsAni, "length", &length);
    TLOGI(WmsLogTag::DMS, "[ANI] length %{public}d", (ani_int)length);
    for (int32_t i = 0; i < length; i++) {
        ani_ref optionRef;
        auto ret =
            env->Object_CallMethodByName_Ref(expandOptionsAni, "$_get", "i:Y", &optionRef, (ani_int)i);
        if (ret != ANI_OK) {
            TLOGE(WmsLogTag::DMS, "[ANI] get ani_array index %{public}u fail, ret: %{public}u", (ani_int)i, ret);
            AniErrUtils::ThrowBusinessError(env, DmErrorCode::DM_ERROR_INVALID_PARAM, "Get optionRef failed");
            return static_cast<ani_long>(INVALID_SCREEN_ID);
        }
        ExpandOption expandOption;
        ret = ScreenAniUtils::GetExpandOptionFromAni(env, static_cast<ani_object>(optionRef), expandOption);
        if (ret != ANI_OK) {
            TLOGE(WmsLogTag::DMS, "[ANI] Get expandOption failed, ret: %{public}u", ret);
            AniErrUtils::ThrowBusinessError(env, DmErrorCode::DM_ERROR_INVALID_PARAM, "Get expandOption failed");
            return static_cast<ani_long>(INVALID_SCREEN_ID);
        }
        options.emplace_back(expandOption);
    }
    ScreenId screenGroupId = INVALID_SCREEN_ID;
    DmErrorCode res =
        DM_JS_TO_ERROR_CODE_MAP.at(SingletonContainer::Get<ScreenManager>().MakeExpand(options, screenGroupId));
    if (res != DmErrorCode::DM_OK) {
        AniErrUtils::ThrowBusinessError(env, res, "ScreenManager::MakeExpand failed.");
        return static_cast<ani_long>(INVALID_SCREEN_ID);
    }
    return static_cast<ani_long>(screenGroupId);
}

void ScreenManagerAni::StopExpand(ani_env* env, ani_object expandScreensAni)
{
    TLOGI(WmsLogTag::DMS, "[ANI] start");
    if (env == nullptr) {
        TLOGE(WmsLogTag::DMS, "[ANI] env is nullptr");
        return;
    }
    if (expandScreensAni == nullptr) {
        TLOGE(WmsLogTag::DMS, "[ANI] expandScreensAni is nullptr");
        AniErrUtils::ThrowBusinessError(env, DmErrorCode::DM_ERROR_INVALID_PARAM, "expandScreensAni is nullptr");
        return;
    }
    std::vector<ScreenId> screenIds;
    ani_status ret = ScreenAniUtils::GetScreenIdArrayFromAni(env, expandScreensAni, screenIds);
    if (ret != ANI_OK) {
        TLOGE(WmsLogTag::DMS, "[ANI] get screenId array failed, ret: %{public}u", ret);
        AniErrUtils::ThrowBusinessError(env, DmErrorCode::DM_ERROR_INVALID_PARAM, "get screenId array failed");
        return;
    }
    DmErrorCode res = DM_JS_TO_ERROR_CODE_MAP.at(SingletonContainer::Get<ScreenManager>().StopExpand(screenIds));
    if (res != DmErrorCode::DM_OK) {
        AniErrUtils::ThrowBusinessError(env, res, "ScreenManager::StopExpand failed.");
    }
}

ani_status ScreenManagerAni::NspBindNativeFunctions(ani_env* env, ani_namespace nsp)
{
    std::array funcs = {
        ani_native_function{ "syncOn", nullptr, reinterpret_cast<void*>(ScreenManagerAni::RegisterCallback) },
        ani_native_function{ "syncOff", nullptr, reinterpret_cast<void*>(ScreenManagerAni::UnRegisterCallback) },
        ani_native_function{ "makeMirrorInternal", nullptr, reinterpret_cast<void*>(ScreenManagerAni::MakeMirror) },
        ani_native_function{
            "getAllScreensInternal", nullptr, reinterpret_cast<void*>(ScreenManagerAni::GetAllScreens) },
        ani_native_function{
            "createVirtualScreenInternal", nullptr, reinterpret_cast<void*>(ScreenManagerAni::CreateVirtualScreen) },
        ani_native_function{ "setVirtualScreenSurfaceInternal",
                             nullptr,
                             reinterpret_cast<void*>(ScreenManagerAni::SetVirtualScreenSurface) },
        ani_native_function{
            "destroyVirtualScreenInternal", nullptr, reinterpret_cast<void*>(ScreenManagerAni::DestroyVirtualScreen) },
        ani_native_function{ "isScreenRotationLockedInternal",
                             nullptr,
                             reinterpret_cast<void*>(ScreenManagerAni::IsScreenRotationLocked) },
        ani_native_function{ "setScreenRotationLockedInternal",
                             nullptr,
                             reinterpret_cast<void*>(ScreenManagerAni::SetScreenRotationLocked) },
        ani_native_function{ "setMultiScreenRelativePositionInternal",
                             nullptr,
                             reinterpret_cast<void*>(ScreenManagerAni::SetMultiScreenRelativePosition) },
        ani_native_function{
            "setMultiScreenModeInternal", nullptr, reinterpret_cast<void*>(ScreenManagerAni::SetMultiScreenMode) },
        ani_native_function{ "setScreenPrivacyMaskImageInternal",
                             nullptr,
                             reinterpret_cast<void*>(ScreenManagerAni::SetScreenPrivacyMaskImage) },
        ani_native_function{ "makeUniqueInternal", nullptr, reinterpret_cast<void*>(ScreenManagerAni::MakeUnique) },
        ani_native_function{
            "makeMirrorWithRegionInternal", nullptr, reinterpret_cast<void*>(ScreenManagerAni::MakeMirrorWithRegion) },
        ani_native_function{ "stopMirrorInternal", nullptr, reinterpret_cast<void*>(ScreenManagerAni::StopMirror) },
        ani_native_function{ "makeExpandInternal", nullptr, reinterpret_cast<void*>(ScreenManagerAni::MakeExpand) },
        ani_native_function{ "stopExpandInternal", nullptr, reinterpret_cast<void*>(ScreenManagerAni::StopExpand) },
    };
    ani_status ret = env->Namespace_BindNativeFunctions(nsp, funcs.data(), funcs.size());
    if (ret != ANI_OK) {
        TLOGE(WmsLogTag::DMS, "[ANI] bind namespace fail %{public}u", ret);
        return ANI_NOT_FOUND;
    }
    return ANI_OK;
}

ani_status ScreenManagerAni::ClassBindNativeFunctions(ani_env* env, ani_class screenCls)
{
    std::array methods = {
        ani_native_function{ "setDensityDpiInternal", "d:", reinterpret_cast<void*>(ScreenAni::SetDensityDpi) },
        ani_native_function{
            "setScreenActiveModeInternal", nullptr, reinterpret_cast<void*>(ScreenAni::SetScreenActiveMode) },
        ani_native_function{ "setOrientationInternal", nullptr, reinterpret_cast<void*>(ScreenAni::SetOrientation) },
    };
    ani_status ret = env->Class_BindNativeMethods(screenCls, methods.data(), methods.size());
    if (ret != ANI_OK) {
        TLOGE(WmsLogTag::DMS, "[ANI] bind namespace methods fail %{public}u", ret);
        return ANI_NOT_FOUND;
    }
    std::array staticMethods = {
        ani_native_function{ "nativeTransferStatic",
                             "C{std.interop.ESValue}C{std.core.Object}:z",
                             reinterpret_cast<void*>(ScreenAni::TransferStatic) },
        ani_native_function{
            "nativeTransferDynamic", "l:C{std.interop.ESValue}", reinterpret_cast<void*>(ScreenAni::TransferDynamic) },
    };
    ret = env->Class_BindStaticNativeMethods(screenCls, staticMethods.data(), staticMethods.size());
    if (ret != ANI_OK) {
        TLOGE(WmsLogTag::DMS, "[ANI] bind namespace static methods fail %{public}u", ret);
        return ANI_NOT_FOUND;
    }
    return ANI_OK;
}

extern "C" {
ANI_EXPORT ani_status ANI_Constructor(ani_vm* vm, uint32_t* result)
{
    using namespace OHOS::Rosen;
    TLOGI(WmsLogTag::DMS, "[ANI] start to ANI_Constructor");
    ani_status ret;
    ani_env* env;
    if ((ret = vm->GetEnv(ANI_VERSION_1, &env)) != ANI_OK) {
        TLOGE(WmsLogTag::DMS, "[ANI] null env");
        return ANI_NOT_FOUND;
    }
    ani_namespace nsp;
    if ((ret = env->FindNamespace("@ohos.screen.screen", &nsp)) != ANI_OK) {
        TLOGE(WmsLogTag::DMS, "[ANI] null env %{public}u", ret);
        return ANI_NOT_FOUND;
    }
    ScreenManagerAni::InitScreenManagerAni(nsp, env);
    ret = ScreenManagerAni::NspBindNativeFunctions(env, nsp);
    if (ret != ANI_OK) {
        TLOGE(WmsLogTag::DMS, "[ANI] bind namespace fail %{public}u", ret);
        return ret;
    }

    ani_class screenCls = nullptr;
    if ((ret = env->FindClass("@ohos.screen.screen.ScreenImpl", &screenCls)) != ANI_OK) {
        TLOGE(WmsLogTag::DMS, "[ANI] null env %{public}u", ret);
        return ANI_NOT_FOUND;
    }
    ret = ScreenManagerAni::ClassBindNativeFunctions(env, screenCls);
    if (ret != ANI_OK) {
        TLOGE(WmsLogTag::DMS, "[ANI] bind screen class fail %{public}u", ret);
        return ret;
    }
    if (result == nullptr) {
        TLOGE(WmsLogTag::DMS, "[ANI] Result is nullptr");
        return ANI_INVALID_ARGS;
    }
    *result = ANI_VERSION_1;
    return ANI_OK;
}
}

} // namespace Rosen
} // namespace OHOS