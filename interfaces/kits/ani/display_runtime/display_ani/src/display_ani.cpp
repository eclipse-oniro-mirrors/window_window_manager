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

#include "display_ani.h"

#include <hitrace_meter.h>

#include "ani.h"
#include "ani_err_utils.h"
#include "display.h"
#include "display_ani_manager.h"
#include "display_ani_utils.h"
#include "display_info.h"
#include "display_manager.h"
#include "dm_common.h"
#include "interop_js/arkts_esvalue.h"
#include "interop_js/arkts_interop_js_api.h"
#include "interop_js/hybridgref_ani.h"
#include "interop_js/hybridgref_napi.h"
#include "js_display.h"
#include "refbase.h"
#include "singleton_container.h"
#include "window_manager_hilog.h"

namespace OHOS {
namespace Rosen {

// construct, set registerManager.
DisplayAni::DisplayAni(const sptr<Display>& display) : display_(display)
{
}

void DisplayAni::GetCutoutInfo(ani_env* env, ani_object obj, ani_object cutoutInfoObj)
{
    auto display = SingletonContainer::Get<DisplayManager>().GetDefaultDisplay();
    TLOGI(WmsLogTag::DMS, "[ANI] begin");
    if (display == nullptr) {
        AniErrUtils::ThrowBusinessError(env, DmErrorCode::DM_ERROR_INVALID_SCREEN, "");
        return;
    }
    sptr<CutoutInfo> cutoutInfo = display->GetCutoutInfo();
    if (cutoutInfo == nullptr) {
        AniErrUtils::ThrowBusinessError(env, DmErrorCode::DM_ERROR_INVALID_SCREEN, "");
    }
    std::vector<DMRect> rects = cutoutInfo->GetBoundingRects();
    // bounding rects
    ani_ref boundingRects = nullptr;
    ani_status status = env->Object_GetFieldByName_Ref(cutoutInfoObj,
        "<property>boundingRects", &boundingRects);
    if (ANI_OK != status) {
        TLOGE(WmsLogTag::DMS, "[ANI] get field bounding rects fail, ani_status = %{public}d", status);
    }
    ani_int length;
    if (ANI_OK != env->Object_GetPropertyByName_Int(static_cast<ani_object>(boundingRects), "length", &length)) {
        TLOGE(WmsLogTag::DMS, "[ANI] get ani_array len fail");
    }
    for (int i = 0; i < std::min(int(length), static_cast<int>(rects.size())); i++) {
        ani_ref currentCutoutInfo;
        if (ANI_OK != env->Object_CallMethodByName_Ref(static_cast<ani_object>(boundingRects), "$_get",
            "i:Y", &currentCutoutInfo, (ani_int)i)) {
            TLOGE(WmsLogTag::DMS, "[ANI] get ani_array index %{public}u fail", (ani_int)i);
        }
        TLOGI(WmsLogTag::DMS, "current i: %{public}d", i);
        DisplayAniUtils::ConvertRect(rects[i], static_cast<ani_object>(currentCutoutInfo), env);
    }
    // waterfall area
    ani_ref waterfallObj = nullptr;
    env->Object_GetFieldByName_Ref(cutoutInfoObj, "<property>waterfallDisplayAreaRects",
        &waterfallObj);
    auto waterfallDisplayAreaRects = cutoutInfo->GetWaterfallDisplayAreaRects();
    DisplayAniUtils::ConvertWaterArea(waterfallDisplayAreaRects, static_cast<ani_object>(waterfallObj), env);
}

void DisplayAni::GetRoundedCorner(ani_env* env, ani_object obj, ani_object roundedCornerObj)
{
    TLOGI(WmsLogTag::DMS, "[ANI] start");
    if (env == nullptr) {
        TLOGE(WmsLogTag::DMS, "[ANI] env is nullptr");
        return;
    }
    auto display = SingletonContainer::Get<DisplayManager>().GetDefaultDisplay();
    if (display == nullptr) {
        AniErrUtils::ThrowBusinessError(env, DmErrorCode::DM_ERROR_INVALID_SCREEN, "Invalid display or screen.");
        return;
    }
    std::vector<RoundedCorner> roundedCorner;
    auto errCode = display->GetRoundedCorner(roundedCorner);
    if (errCode != DMError::DM_OK) {
        TLOGE(WmsLogTag::DMS, "[ANI] Display get rounded corner failed.");
        AniErrUtils::ThrowBusinessError(env, errCode, "Display get rounded corner failed.");
        return;
    }
    if (roundedCorner.empty()) {
        AniErrUtils::ThrowBusinessError(env, DmErrorCode::DM_ERROR_SYSTEM_INNORMAL,
            "This display manager service works abnormally.");
    } else {
        DisplayAniUtils::ConvertRoundedCorner(roundedCorner, roundedCornerObj, env);
    }
}

ani_string DisplayAni::GetDisplayCapability(ani_env* env)
{
    TLOGI(WmsLogTag::DMS, "[ANI] Start");
    if (env == nullptr) {
        TLOGE(WmsLogTag::DMS, "[ANI] env is nullptr");
        return nullptr;
    }
    auto display = SingletonContainer::Get<DisplayManager>().GetDefaultDisplay();
    std::string capabilitInfo;
    DmErrorCode ret = DM_JS_TO_ERROR_CODE_MAP.at(display->GetDisplayCapability(capabilitInfo));
    if (ret != DmErrorCode::DM_OK) {
        TLOGE(WmsLogTag::DMS, "[ANI] Display get capability failed.");
        AniErrUtils::ThrowBusinessError(env, ret, "GetDisplayCapability failed.");
        return nullptr;
    }
    TLOGI(WmsLogTag::DMS, "[ANI] GetDisplayCapability = %{public}s", capabilitInfo.c_str());
    ani_string capabilitInfoAni;
    ani_status status = DisplayAniUtils::GetAniString(env, capabilitInfo, &capabilitInfoAni);
    if (status != ANI_OK) {
        TLOGE(WmsLogTag::DMS, "[ANI] GetAniString failed, ani_status = %{public}d", status);
        return nullptr;
    }
    return capabilitInfoAni;
}

void DisplayAni::GetAvailableArea(ani_env* env, ani_object obj, ani_object availableAreaObj)
{
    TLOGI(WmsLogTag::DMS, "[ANI] begin");
    ani_long id;
    env->Object_GetFieldByName_Long(obj, "<property>id", &id);
    auto display = SingletonContainer::Get<DisplayManager>().GetDisplayById(id);
    if (display == nullptr) {
        TLOGE(WmsLogTag::DMS, "[ANI] can not find display.");
        AniErrUtils::ThrowBusinessError(env, DmErrorCode::DM_ERROR_INVALID_SCREEN,
            "JsDisplay::GetAvailableArea failed, can not find display.");
        return;
    }
    DMRect area;
    DmErrorCode ret = DM_JS_TO_ERROR_CODE_MAP.at(display->GetAvailableArea(area));
    if (ret != DmErrorCode::DM_OK) {
        TLOGE(WmsLogTag::DMS, "[ANI] Display get available area failed.");
        AniErrUtils::ThrowBusinessError(env, ret, "JsDisplay::GetAvailableArea failed.");
        return;
    }
    DisplayAniUtils::ConvertRect(area, availableAreaObj, env);
}

void DisplayAni::GetLiveCreaseRegion(ani_env* env, ani_object obj, ani_object foldCreaseRegionObj)
{
    TLOGI(WmsLogTag::DMS, "[ANI] begin");
    if (env == nullptr) {
        TLOGE(WmsLogTag::DMS, "[ANI] env is nullptr");
        return;
    }
    ani_long id;
    env->Object_GetFieldByName_Long(obj, "<property>id", &id);
    auto display = SingletonContainer::Get<DisplayManager>().GetDisplayById(id);
    if (display == nullptr) {
        TLOGE(WmsLogTag::DMS, "[ANI] can not find display.");
        AniErrUtils::ThrowBusinessError(env, DmErrorCode::DM_ERROR_INVALID_SCREEN,
            "JsDisplay::GetAvailableArea failed, can not find display.");
        return;
    }
    FoldCreaseRegion region;
    DMError nativeErrorCode = display->GetLiveCreaseRegion(region);
    auto errorCodeMapping = DM_JS_TO_ERROR_CODE_MAP.find(nativeErrorCode);
    if (errorCodeMapping == DM_JS_TO_ERROR_CODE_MAP.end()) {
        TLOGE(WmsLogTag::DMS, "Unrecognized DMError: %{public}d", static_cast<int32_t>(nativeErrorCode));
        AniErrUtils::ThrowBusinessError(env,
            DmErrorCode::DM_ERROR_INVALID_PARAM, "JsDisplay::GetAvailableArea failed.");
        return;
    }
    DisplayAniUtils::SetFoldCreaseRegion(env, region, foldCreaseRegionObj);
}

ani_boolean DisplayAni::HasImmersiveWindow(ani_env* env, ani_object obj)
{
    TLOGI(WmsLogTag::DMS, "[ANI] begin");
    ani_long id;
    env->Object_GetFieldByName_Long(obj, "<property>id", &id);
    auto display = SingletonContainer::Get<DisplayManager>().GetDisplayById(id);
    if (display == nullptr) {
        TLOGE(WmsLogTag::DMS, "[ANI]can not find display.");
        AniErrUtils::ThrowBusinessError(env, DmErrorCode::DM_ERROR_INVALID_SCREEN,
            "can not find display.");
        return false;
    }
    bool immersive = false;
    DmErrorCode ret = DM_JS_TO_ERROR_CODE_MAP.at(display->HasImmersiveWindow(immersive));
    if (ret != DmErrorCode::DM_OK) {
        TLOGE(WmsLogTag::DMS, "[ANI] get display has immersive window failed.");
        AniErrUtils::ThrowBusinessError(env, ret, "JsDisplay::HasImmersiveWindow failed.");
        return false;
    }
    return immersive;
}

bool DisplayAni::IsCallbackRegistered(ani_env* env, const std::string& type, ani_ref callback)
{
    std::lock_guard<std::mutex> lock(mtx_);
    if (jsCbMap_.empty() || jsCbMap_.find(type) == jsCbMap_.end()) {
        TLOGI(WmsLogTag::DMS, "method %{public}s not registered.", type.c_str());
        return false;
    }
    for (const auto& iter : jsCbMap_[type]) {
        ani_boolean isEquals = false;
        env->Reference_StrictEquals(callback, iter.first, &isEquals);
        if (isEquals) {
            TLOGE(WmsLogTag::DMS, "callback is already registered!");
            return true;
        }
    }
    return false;
}

void DisplayAni::RegisterCallback(ani_env* env, ani_object obj, ani_string type, ani_ref callback)
{
    TLOGI(WmsLogTag::DMS, "[ANI] begin");
    ani_long displayRef;
    env->Object_GetFieldByName_Long(obj, "displayRef", &displayRef);
    DisplayAni* displayAni = reinterpret_cast<DisplayAni*>(displayRef);
    if (displayAni != nullptr) {
        displayAni->OnRegisterCallback(env, obj, type, callback);
    } else {
        TLOGE(WmsLogTag::DMS, "[ANI] displayAni not found");
    }
}

void DisplayAni::OnRegisterCallback(ani_env* env, ani_object obj, ani_string type, ani_ref callback)
{
    std::string typeString;
    DisplayAniUtils::GetStdString(env, type, typeString);
    ani_boolean callbackUndefined = 0;
    env->Reference_IsUndefined(callback, &callbackUndefined);
    if (callbackUndefined) {
        std::string errMsg = "[ANI] failed to register display listener with type, cbk null or undefined";
        TLOGE(WmsLogTag::DMS, "callback undef");
        AniErrUtils::ThrowBusinessError(env, DmErrorCode::DM_ERROR_INVALID_PARAM, errMsg);
        return;
    }
    ani_ref cbRef{};
    if (ANI_OK != env->GlobalReference_Create(callback, &cbRef)) {
        env->GlobalReference_Delete(cbRef);
        TLOGE(WmsLogTag::DMS, "[ANI]create global ref fail");
        return;
    }
    if (IsCallbackRegistered(env, typeString, cbRef)) {
        env->GlobalReference_Delete(cbRef);
        TLOGI(WmsLogTag::DMS, "Callback has already been registered!");
        return;
    }

    sptr<DisplayAniListener> displayAniListener = new(std::nothrow) DisplayAniListener(env);
    if (displayAniListener == nullptr) {
        TLOGE(WmsLogTag::DMS, "[ANI]displayListener is nullptr");
        env->GlobalReference_Delete(cbRef);
        AniErrUtils::ThrowBusinessError(env, DMError::DM_ERROR_INVALID_PARAM, "displayListener is nullptr");
        return;
    }
    displayAniListener->AddCallback(typeString, cbRef);
    displayAniListener->SetMainEventHandler();
    DMError ret = DMError::DM_OK;
    std::lock_guard<std::mutex> lock(mtx_);
    if (typeString == ANI_EVENT_AVAILABLE_AREA_CHANGED) {
        auto displayId = display_->GetId();
        TLOGI(WmsLogTag::DMS, "[ANI] availableAreaChange begin");
        ret = SingletonContainer::Get<DisplayManager>().RegisterAvailableAreaListener(displayAniListener, displayId);
    } else {
        ret = DMError::DM_ERROR_INVALID_PARAM;
    }
    DmErrorCode retCode = DM_JS_TO_ERROR_CODE_MAP.at(ret);
    if (retCode != DmErrorCode::DM_OK) {
        TLOGI(WmsLogTag::DMS, "Failed to register display listener with type");
        env->GlobalReference_Delete(cbRef);
        std::string errMsg = "Failed to register display listener with type";
        AniErrUtils::ThrowBusinessError(env, DMError::DM_ERROR_INVALID_PARAM, errMsg);
        return;
    }
    jsCbMap_[typeString][cbRef] = displayAniListener;
}

void DisplayAni::UnRegisterCallback(ani_env* env, ani_object obj, ani_string type, ani_ref callback)
{
    TLOGI(WmsLogTag::DMS, "[ANI] begin");
    ani_long displayRef;
    env->Object_GetFieldByName_Long(obj, "displayRef", &displayRef);
    DisplayAni* displayAni = reinterpret_cast<DisplayAni*>(displayRef);
    if (displayAni != nullptr) {
        displayAni->OnUnRegisterCallback(env, obj, type, callback);
    } else {
        TLOGE(WmsLogTag::DMS, "[ANI] DisplayAni null");
    }
}

void DisplayAni::OnUnRegisterCallback(ani_env* env, ani_object obj, ani_string type, ani_ref callback)
{
    TLOGI(WmsLogTag::DMS, "[ANI] begin");
    std::string typeString;
    DisplayAniUtils::GetStdString(env, type, typeString);
    ani_boolean callbackUndefined = 0;
    env->Reference_IsUndefined(callback, &callbackUndefined);
    ani_ref cbRef{};
    if (ANI_OK != env->GlobalReference_Create(callback, &cbRef)) {
        TLOGE(WmsLogTag::DMS, "[ANI]create global ref fail");
        env->GlobalReference_Delete(cbRef);
        return;
    }
    DmErrorCode ret;
    if (callbackUndefined) {
        TLOGI(WmsLogTag::DMS, "[ANI] for all");
        ret = DM_JS_TO_ERROR_CODE_MAP.at(UnregisterAllDisplayListenerWithType(typeString));
    } else {
        TLOGI(WmsLogTag::DMS, "[ANI] with type");
        ret = DM_JS_TO_ERROR_CODE_MAP.at(UnregisterDisplayListenerWithType(typeString, env, cbRef));
    }
    if (ret != DmErrorCode::DM_OK) {
        std::string errMsg = "[ANI] failed to unregister display listener with type";
        TLOGE(WmsLogTag::DMS, "[ANI] failed to unregister display listener with type");
        AniErrUtils::ThrowBusinessError(env, DMError::DM_ERROR_INVALID_PARAM, errMsg);
    }
    env->GlobalReference_Delete(cbRef);
}

DMError DisplayAni::UnregisterAllDisplayListenerWithType(std::string type)
{
    TLOGI(WmsLogTag::DMS, "[ANI] begin");
    std::lock_guard<std::mutex> lock(mtx_);
    if (jsCbMap_.empty() || jsCbMap_.find(type) == jsCbMap_.end()) {
        TLOGI(WmsLogTag::DMS, "[ANI] methodName %{public}s not registered!",
            type.c_str());
        return DMError::DM_OK;
    }
    DMError ret = DMError::DM_OK;
    for (auto it = jsCbMap_[type].begin(); it != jsCbMap_[type].end();) {
        if (it->second == nullptr) {
            TLOGE(WmsLogTag::DMS, "listener is null");
            continue;
        }
        it->second->RemoveAllCallback();
        if (type == ANI_EVENT_AVAILABLE_AREA_CHANGED) {
            auto displayId = display_->GetId();
            sptr<DisplayManager::IAvailableAreaListener> thisListener(it->second);
            ret = SingletonContainer::Get<DisplayManager>().UnregisterAvailableAreaListener(thisListener, displayId);
        } else {
            ret = DMError::DM_ERROR_INVALID_PARAM;
        }
        jsCbMap_[type].erase(it++);
    }
    jsCbMap_.erase(type);
    return ret;
}

DMError DisplayAni::UnregisterDisplayListenerWithType(std::string type, ani_env* env, ani_ref callback)
{
    TLOGI(WmsLogTag::DMS, "[ANI] begin");
    std::lock_guard<std::mutex> lock(mtx_);
    if (jsCbMap_.empty() || jsCbMap_.find(type) == jsCbMap_.end()) {
        TLOGI(WmsLogTag::DMS, "[ANI] methodName %{public}s not registered!", type.c_str());
        return DMError::DM_OK;
    }
    DMError ret = DMError::DM_OK;
    for (auto it = jsCbMap_[type].begin(); it != jsCbMap_[type].end();) {
        ani_boolean isEquals = 0;
        env->Reference_StrictEquals(callback, it->first, &isEquals);
        if (isEquals) {
            if (it->second == nullptr) {
                TLOGE(WmsLogTag::DMS, "listener is null");
                it++;
                continue;
            }
            it->second->RemoveCallback(env, type, callback);
            if (type == ANI_EVENT_AVAILABLE_AREA_CHANGED) {
                TLOGI(WmsLogTag::DMS, "[ANI] start to unregis display event listener! event = %{public}s",
                    type.c_str());
                auto displayId = display_->GetId();
                sptr<DisplayManager::IAvailableAreaListener> thisListener(it->second);
                ret = SingletonContainer::Get<DisplayManager>().UnregisterAvailableAreaListener(thisListener,
                    displayId);
            } else {
                ret = DMError::DM_ERROR_INVALID_PARAM;
            }
            jsCbMap_[type].erase(it++);
            TLOGI(WmsLogTag::DMS, "[ANI] unregister display listener with type: %{public}s ret: %{public}u",
                type.c_str(), ret);
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

void DisplayAni::CreateDisplayAni(sptr<Display> display, ani_object displayObj, ani_env* env)
{
    TLOGI(WmsLogTag::DMS, "[ANI] CreateDisplayAni begin");
    std::shared_ptr<DisplayAni> displayAni = DisplayAniUtils::FindAniDisplayObject(display, display->GetId());
    if (displayAni == nullptr) {
        TLOGE(WmsLogTag::DMS, "[ANI] CreateDisplayAni failed");
        return;
    }
    if (ANI_OK != env->Object_SetFieldByName_Long(static_cast<ani_object>(displayObj),
        "displayRef", reinterpret_cast<ani_long>(displayAni.get()))) {
        TLOGE(WmsLogTag::DMS, "[ANI] set displayAni ref fail");
    }
}

ani_boolean DisplayAni::TransferStatic(ani_env* env, ani_object obj, ani_object input, ani_object displayAniObj)
{
    TLOGI(WmsLogTag::DMS, "begin");
    void* unwrapResult = nullptr;
    auto ret = arkts_esvalue_unwrap(env, input, &unwrapResult);
    if (!ret) {
        TLOGE(WmsLogTag::DMS, "[ANI] fail to unwrap input, %{public}d", ret);
        return false;
    }
    if (unwrapResult == nullptr) {
        TLOGE(WmsLogTag::DMS, "[ANI] unwrapResult is nullptr");
        return false;
    }
    JsDisplay* jsDisplay = static_cast<JsDisplay*>(unwrapResult);
    if (jsDisplay == nullptr) {
        TLOGE(WmsLogTag::DMS, "[ANI] jsDisplay is nullptr");
        return false;
    }

    sptr<Display> display = jsDisplay->GetDisplay();
    if (DisplayAniUtils::CvtDisplay(display, env, displayAniObj) != ANI_OK) {
        TLOGE(WmsLogTag::DMS, "[ANI] convert display failed");
        return false;
    }
    DisplayAni::CreateDisplayAni(display, displayAniObj, env);
    return true;
}

ani_object DisplayAni::TransferDynamic(ani_env* env, ani_object obj, ani_long nativeObj)
{
    TLOGI(WmsLogTag::DMS, "begin");
    DisplayAni* aniDisplay = reinterpret_cast<DisplayAni*>(nativeObj);
    if (aniDisplay == nullptr) {
        TLOGE(WmsLogTag::DMS, "[ANI] aniDisplay is nullptr");
        return nullptr;
    }
    napi_env napiEnv = {};
    if (!arkts_napi_scope_open(env, &napiEnv)) {
        TLOGE(WmsLogTag::DMS, "arkts_napi_scope_open failed");
        return nullptr;
    }

    sptr<OHOS::Rosen::Display> display = aniDisplay->GetDisplay();
    napi_value jsDisplay = CreateJsDisplayObject(napiEnv, display);
    hybridgref ref = nullptr;
    if (!hybridgref_create_from_napi(napiEnv, jsDisplay, &ref)) {
        TLOGE(WmsLogTag::DMS, "hybridgref_create_from_napi failed");
        return nullptr;
    }
    ani_object result = nullptr;
    if (!hybridgref_get_esvalue(env, ref, &result)) {
        hybridgref_delete_from_napi(napiEnv, ref);
        TLOGE(WmsLogTag::DMS, "hybridgref_get_esvalue failed");
        return nullptr;
    }
    hybridgref_delete_from_napi(napiEnv, ref);
    if (!arkts_napi_scope_close_n(napiEnv, 0, nullptr, nullptr)) {
        TLOGE(WmsLogTag::DMS, "arkts_napi_scope_close_n failed");
        return nullptr;
    }
    return result;
}

ani_status DisplayAni::NspBindNativeFunctions(ani_env* env, ani_namespace nsp)
{
    std::array funcs = {
        ani_native_function {"isFoldable", ":z", reinterpret_cast<void *>(DisplayManagerAni::IsFoldableAni)},
        ani_native_function {"setFoldDisplayModeReasonNative", nullptr,
            reinterpret_cast<void *>(DisplayManagerAni::SetFoldDisplayModeReasonAni)},
        ani_native_function {"setFoldDisplayModeNative", nullptr,
            reinterpret_cast<void *>(DisplayManagerAni::SetFoldDisplayModeAni)},
        ani_native_function {"getFoldDisplayModeNative", ":i",
            reinterpret_cast<void *>(DisplayManagerAni::GetFoldDisplayModeAni)},
        ani_native_function {"setFoldStatusLockedNative", nullptr,
            reinterpret_cast<void *>(DisplayManagerAni::SetFoldStatusLockedAni)},
        ani_native_function {"getFoldStatusNative", ":i", reinterpret_cast<void *>(DisplayManagerAni::GetFoldStatus)},
        ani_native_function {"getCurrentFoldCreaseRegionNative", "C{std.core.Object}l:",
            reinterpret_cast<void *>(DisplayManagerAni::GetCurrentFoldCreaseRegion)},
        ani_native_function {"getPrimaryDisplaySyncNative", "C{std.core.Object}:",
            reinterpret_cast<void *>(DisplayManagerAni::GetPrimaryDisplaySyncAni)},
        ani_native_function {"getDisplayByIdSyncNative", "C{std.core.Object}l:",
            reinterpret_cast<void *>(DisplayManagerAni::GetDisplayByIdSyncAni)},
        ani_native_function {"getDefaultDisplaySyncNative", "C{std.core.Object}:",
            reinterpret_cast<void *>(DisplayManagerAni::GetDefaultDisplaySyncAni)},
        ani_native_function {"getBrightnessInfoNative", nullptr,
            reinterpret_cast<void *>(DisplayManagerAni::GetBrightnessInfoAni)},
        ani_native_function {"getAllDisplaysSyncNative", "C{std.core.Array}:",
            reinterpret_cast<void *>(DisplayManagerAni::GetAllDisplaysAni)},
        ani_native_function {"syncOn", nullptr,
            reinterpret_cast<void *>(DisplayManagerAni::RegisterCallback)},
        ani_native_function {"syncOff", nullptr,
            reinterpret_cast<void *>(DisplayManagerAni::UnRegisterCallback)},
        ani_native_function {"hasPrivateWindow", nullptr,
            reinterpret_cast<void *>(DisplayManagerAni::HasPrivateWindow)},
        ani_native_function {"getAllDisplayPhysicalResolutionNative", nullptr,
            reinterpret_cast<void *>(DisplayManagerAni::GetAllDisplayPhysicalResolution)},
        ani_native_function {"convertGlobalToRelativeCoordinateNative", nullptr,
            reinterpret_cast<void *>(DisplayManagerAni::ConvertGlobalToRelativeCoordinate)},
        ani_native_function {"convertRelativeToGlobalCoordinateNative", nullptr,
            reinterpret_cast<void *>(DisplayManagerAni::ConvertRelativeToGlobalCoordinate)},
        ani_native_function {"createVirtualScreenNative", nullptr,
            reinterpret_cast<void *>(DisplayManagerAni::CreateVirtualScreen)},
        ani_native_function {"destroyVirtualScreenNative", nullptr,
            reinterpret_cast<void *>(DisplayManagerAni::DestroyVirtualScreen)},
        ani_native_function {"setVirtualScreenSurfaceNative", nullptr,
            reinterpret_cast<void *>(DisplayManagerAni::SetVirtualScreenSurface)},
        ani_native_function {"makeUniqueNative", nullptr,
            reinterpret_cast<void *>(DisplayManagerAni::MakeUnique)},
        ani_native_function {"addVirtualScreenBlocklistNative", nullptr,
            reinterpret_cast<void *>(DisplayManagerAni::AddVirtualScreenBlocklist)},
        ani_native_function {"removeVirtualScreenBlocklistNative", nullptr,
            reinterpret_cast<void *>(DisplayManagerAni::RemoveVirtualScreenBlocklist)},
        ani_native_function {"isCaptured", nullptr, reinterpret_cast<void *>(DisplayManagerAni::IsCaptured)},
        ani_native_function {"finalizerDisplayNative", nullptr,
            reinterpret_cast<void *>(DisplayManagerAni::FinalizerDisplay)},
        ani_native_function {"onChangeWithAttributeNative", nullptr,
            reinterpret_cast<void *>(DisplayManagerAni::RegisterDisplayAttributeListener)},
    };
    auto ret = env->Namespace_BindNativeFunctions(nsp, funcs.data(), funcs.size());
    if (ret != ANI_OK) {
        TLOGE(WmsLogTag::DMS, "[ANI] bind namespace fail %{public}u", ret);
        return ANI_NOT_FOUND;
    }
    return ANI_OK;
}

ani_status DisplayAni::ClassBindNativeFunctions(ani_env* env, ani_class displayCls)
{
    std::array methods = {
        ani_native_function {"getCutoutInfoInternal", "C{@ohos.display.display.CutoutInfo}:",
            reinterpret_cast<void *>(DisplayAni::GetCutoutInfo)},
        ani_native_function {"getDisplayCapabilityInternal", nullptr,
            reinterpret_cast<void *>(DisplayAni::GetDisplayCapability)},
        ani_native_function {"getAvailableAreaInternal", "C{@ohos.display.display.Rect}:",
            reinterpret_cast<void *>(DisplayAni::GetAvailableArea)},
        ani_native_function {"getLiveCreaseRegionInternal", nullptr,
            reinterpret_cast<void *>(DisplayAni::GetLiveCreaseRegion)},
        ani_native_function {"hasImmersiveWindowInternal", ":z",
            reinterpret_cast<void *>(DisplayAni::HasImmersiveWindow)},
        ani_native_function {"syncOn", nullptr,
            reinterpret_cast<void *>(DisplayAni::RegisterCallback)},
        ani_native_function {"syncOff", nullptr,
            reinterpret_cast<void *>(DisplayAni::UnRegisterCallback)},
        ani_native_function {"getRoundedCornerInternal", nullptr,
            reinterpret_cast<void *>(DisplayAni::GetRoundedCorner)},
    };
    auto ret = env->Class_BindNativeMethods(displayCls, methods.data(), methods.size());
    if (ret != ANI_OK) {
        TLOGE(WmsLogTag::DMS, "[ANI] bind class methods fail %{public}u", ret);
        return ANI_NOT_FOUND;
    }
    std::array staticMethods = {
        ani_native_function {"nativeTransferStatic", "C{std.interop.ESValue}C{std.core.Object}:z",
            reinterpret_cast<void *>(DisplayAni::TransferStatic)},
        ani_native_function {"nativeTransferDynamic", "l:C{std.interop.ESValue}",
            reinterpret_cast<void *>(DisplayAni::TransferDynamic)},
    };
    ret = env->Class_BindStaticNativeMethods(displayCls, staticMethods.data(), staticMethods.size());
    if (ret != ANI_OK) {
        TLOGE(WmsLogTag::DMS, "[ANI] bind class static methods fail %{public}u", ret);
        return ANI_NOT_FOUND;
    }
    return ANI_OK;
}

extern "C" {
ANI_EXPORT ani_status ANI_Constructor(ani_vm* vm, uint32_t* result)
{
    using namespace OHOS::Rosen;
    ani_status ret;
    ani_env* env;
    if ((ret = vm->GetEnv(ANI_VERSION_1, &env)) != ANI_OK) {
        TLOGE(WmsLogTag::DMS, "[ANI] null env");
        return ANI_NOT_FOUND;
    }
    ani_namespace nsp;
    if ((ret = env->FindNamespace("@ohos.display.display", &nsp)) != ANI_OK) {
        TLOGE(WmsLogTag::DMS, "[ANI] null env %{public}u", ret);
        return ANI_NOT_FOUND;
    }
    DisplayManagerAni::InitDisplayManagerAni(nsp, env);
    ret = DisplayAni::NspBindNativeFunctions(env, nsp);
    if (ret != ANI_OK) {
        TLOGE(WmsLogTag::DMS, "[ANI] bind namespace fail %{public}u", ret);
        return ret;
    }

    ani_class displayCls = nullptr;
    if ((ret = env->FindClass("@ohos.display.display.DisplayImpl", &displayCls)) != ANI_OK) {
        TLOGE(WmsLogTag::DMS, "[ANI] null env %{public}u", ret);
        return ANI_NOT_FOUND;
    }
    ret = DisplayAni::ClassBindNativeFunctions(env, displayCls);
    if (ANI_OK != ret) {
        TLOGE(WmsLogTag::DMS, "[ANI] bind class fail %{public}u", ret);
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

}  // namespace Rosen
}  // namespace OHOS
