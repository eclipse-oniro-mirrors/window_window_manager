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

#include "display_ani_manager.h"

#include <algorithm>
#include <hitrace_meter.h>

#include "ani.h"
#include "ani_err_utils.h"
#include "display.h"
#include "display_ani.h"
#include "display_ani_listener.h"
#include "display_ani_utils.h"
#include "display_info.h"
#include "display_manager.h"
#include "dm_common.h"
#include "refbase.h"
#include "singleton_container.h"
#include "window_manager_hilog.h"

namespace OHOS {
namespace Rosen {

DisplayManagerAni::DisplayManagerAni()
{
}

ani_status DisplayManagerAni::InitDisplayManagerAni(ani_namespace displayNameSpace, ani_env* env)
{
    TLOGI(WmsLogTag::DEFAULT, "[ANI]");
    ani_function setObjFunc = nullptr;
    ani_status ret = env->Namespace_FindFunction(displayNameSpace, "setDisplayMgrRef", "l:", &setObjFunc);
    if (ret != ANI_OK) {
        TLOGE(WmsLogTag::DEFAULT, "[ANI] find setNativeObj func fail %{public}u", ret);
        return ret;
    }
    std::unique_ptr<DisplayManagerAni> aniDisplayManager = std::make_unique<DisplayManagerAni>();
    ret = env->Function_Call_Void(setObjFunc, aniDisplayManager.release());
    if (ret != ANI_OK) {
        TLOGE(WmsLogTag::DEFAULT, "[ANI] find setNativeObj func fail %{public}u", ret);
        return ret;
    }
    return ret;
}

ani_int DisplayManagerAni::GetFoldDisplayModeAni(ani_env* env)
{
    auto mode = SingletonContainer::Get<DisplayManager>().GetFoldDisplayMode();
    TLOGI(WmsLogTag::DMS, "[ANI]" PRIu64", getFoldDisplayMode = %{public}u", mode);
    return static_cast<ani_int>(mode);
}

ani_boolean DisplayManagerAni::IsFoldableAni(ani_env* env)
{
    bool foldable = SingletonContainer::Get<DisplayManager>().IsFoldable();
    TLOGI(WmsLogTag::DMS, "[ANI]" PRIu64", isFoldable = %{public}u", foldable);
    return static_cast<ani_boolean>(foldable);
}

ani_boolean DisplayManagerAni::IsCaptured(ani_env* env)
{
    bool isCapture = SingletonContainer::Get<DisplayManager>().IsCaptured();
    TLOGI(WmsLogTag::DMS, "[ANI]" PRIu64", isCapture = %{public}u", isCapture);
    return static_cast<ani_boolean>(isCapture);
}

ani_int DisplayManagerAni::GetFoldStatus(ani_env* env)
{
    auto status = SingletonContainer::Get<DisplayManager>().GetFoldStatus();
    TLOGI(WmsLogTag::DMS, "[ANI]" PRIu64", GetFoldStatus = %{public}u", status);
    return static_cast<ani_int>(status);
}

void DisplayManagerAni::GetCurrentFoldCreaseRegion(ani_env* env, ani_object obj, ani_long nativeObj)
{
    DisplayManagerAni* displayManagerAni = reinterpret_cast<DisplayManagerAni*>(nativeObj);
    if (displayManagerAni != nullptr) {
        displayManagerAni->OnGetCurrentFoldCreaseRegion(env, obj);
    } else {
        TLOGI(WmsLogTag::DMS, "[ANI] null ptr");
    }
}

void DisplayManagerAni::OnGetCurrentFoldCreaseRegion(ani_env* env, ani_object obj)
{
    TLOGI(WmsLogTag::DMS, "[ANI] begin");
    sptr<FoldCreaseRegion> region = SingletonContainer::Get<DisplayManager>().GetCurrentFoldCreaseRegion();
    if (region == nullptr) {
        return;
    }
    TLOGI(WmsLogTag::DMS, "[ANI] SingletonContainer GetCurrentFoldCreaseRegion");
    uint64_t displayId = region->GetDisplayId();
    std::vector<DMRect> rects = region->GetCreaseRects();
    if (rects.size() == 0) {
        return;
    }
    TLOGI(WmsLogTag::DMS, "[ANI] DisplayManager GetCurrentFoldCreaseRegion success %{public}d", (int)displayId);
    if (ANI_OK != env->Object_SetFieldByName_Long(obj, "<property>displayId", (ani_long)displayId)) {
        TLOGE(WmsLogTag::DMS, "[ANI] set displayId field fail");
    }
    ani_ref creaseRectsObj{};
    if (ANI_OK != env->Object_GetFieldByName_Ref(obj, "<property>creaseRects", &creaseRectsObj)) {
        TLOGE(WmsLogTag::DMS, "[ANI] get ani_array len fail");
    }
    ani_int length;
    if (ANI_OK != env->Object_GetPropertyByName_Int(static_cast<ani_object>(creaseRectsObj), "length", &length)) {
        TLOGE(WmsLogTag::DMS, "[ANI] get ani_array len fail");
    }
    TLOGI(WmsLogTag::DMS, "[ANI] set CurrentFoldCreaseRegion property begin");
    for (int i = 0; i < std::min(int(length), static_cast<int>(rects.size())); i++) {
        ani_ref currentCrease;
        if (ANI_OK != env->Object_CallMethodByName_Ref(static_cast<ani_object>(creaseRectsObj),
            "$_get", "i:C{std.core.Object}", &currentCrease, (ani_int)i)) {
            TLOGE(WmsLogTag::DMS, "[ANI] get ani_array index %{public}u fail", (ani_int)i);
        }
        TLOGI(WmsLogTag::DMS, "current i: %{public}d", i);
        DisplayAniUtils::ConvertRect(rects[i], static_cast<ani_object>(currentCrease), env);
    }
}

void DisplayManagerAni::GetAllDisplaysAni(ani_env* env, ani_object arrayObj)
{
    TLOGI(WmsLogTag::DMS, "[ANI] start");
    std::vector<sptr<Display>> displays = SingletonContainer::Get<DisplayManager>().GetAllDisplays();
    if (displays.empty()) {
        AniErrUtils::ThrowBusinessError(env, DmErrorCode::DM_ERROR_INVALID_SCREEN, "");
    }
    ani_int length;
    if (ANI_OK != env->Object_GetPropertyByName_Int(arrayObj, "length", &length)) {
        TLOGE(WmsLogTag::DMS, "[ANI] get ani_array len fail");
    }

    for (int i = 0; i < std::min(int(length), static_cast<int>(displays.size())); i++) {
        ani_ref currentDisplay;
        if (ANI_OK != env->Object_CallMethodByName_Ref(arrayObj, "$_get", "i:C{std.core.Object}",
            &currentDisplay, (ani_int)i)) {
            TLOGE(WmsLogTag::DMS, "[ANI] get ani_array index %{public}u fail", (ani_int)i);
        }
        TLOGI(WmsLogTag::DMS, "current i: %{public}d", i);
        DisplayAniUtils::CvtDisplay(displays[i], env, static_cast<ani_object>(currentDisplay));
        DisplayAni::CreateDisplayAni(displays[i], static_cast<ani_object>(currentDisplay), env);
    }
    TLOGI(WmsLogTag::DMS, "[ANI] GetAllDisplaysAni end");
}

void DisplayManagerAni::GetDisplayByIdSyncAni(ani_env* env, ani_object obj, ani_long displayId)
{
    TLOGI(WmsLogTag::DMS, "[ANI] begin");
    if (displayId < 0) {
        TLOGE(WmsLogTag::DMS, "[ANI] Invalid displayId, less than 0");
        AniErrUtils::ThrowBusinessError(env, DmErrorCode::DM_ERROR_INVALID_PARAM, "Invalid displayId");
        return;
    }
    sptr<Display> display = SingletonContainer::Get<DisplayManager>().GetDisplayById(static_cast<DisplayId>(displayId));
    if (display == nullptr) {
        TLOGE(WmsLogTag::DMS, "[ANI] Display null");
        AniErrUtils::ThrowBusinessError(env, DmErrorCode::DM_ERROR_SYSTEM_INNORMAL, "");
        return;
    }
    DisplayAniUtils::CvtDisplay(display, env, obj);
    DisplayAni::CreateDisplayAni(display, static_cast<ani_object>(obj), env);
}

void DisplayManagerAni::GetDefaultDisplaySyncAni(ani_env* env, ani_object obj)
{
    sptr<Display> display = SingletonContainer::Get<DisplayManager>().GetDefaultDisplaySync(true);
    if (display == nullptr) {
        TLOGE(WmsLogTag::DMS, "[ANI] Display null");
        return;
    }
    TLOGI(WmsLogTag::DMS, "[ANI] GetDefaultDisplaySyncAni");
    DisplayAniUtils::CvtDisplay(display, env, obj);
    DisplayAni::CreateDisplayAni(display, static_cast<ani_object>(obj), env);
    return;
}

void DisplayManagerAni::RegisterCallback(ani_env* env, ani_string type,
    ani_ref callback, ani_long nativeObj)
{
    DisplayManagerAni* displayManagerAni = reinterpret_cast<DisplayManagerAni*>(nativeObj);
    if (displayManagerAni != nullptr) {
        displayManagerAni->OnRegisterCallback(env, type, callback);
    } else {
        TLOGI(WmsLogTag::DMS, "[ANI] null ptr");
    }
}

void DisplayManagerAni::OnRegisterCallback(ani_env* env, ani_string type, ani_ref callback)
{
    std::string typeString;
    DisplayAniUtils::GetStdString(env, type, typeString);
    ani_ref cbRef{};
    if (env->GlobalReference_Create(callback, &cbRef) != ANI_OK) {
        TLOGE(WmsLogTag::DMS, "[ANI] create global ref fail");
        return;
    }
    std::lock_guard<std::mutex> lock(mtx_);
    if (IsCallbackRegistered(env, typeString, cbRef)) {
        TLOGI(WmsLogTag::DMS, "[ANI] type %{public}s callback already registered!", typeString.c_str());
        env->GlobalReference_Delete(cbRef);
        return;
    }
    TLOGI(WmsLogTag::DMS, "[ANI] onRegisterCallback");
    ani_boolean callbackUndefined = 0;
    env->Reference_IsUndefined(cbRef, &callbackUndefined);
    DmErrorCode ret;
    if (callbackUndefined) {
        std::string errMsg = "[ANI] failed to register display listener with type, cbk null or undefined";
        TLOGE(WmsLogTag::DMS, "callbackNull or undefined");
        AniErrUtils::ThrowBusinessError(env, DmErrorCode::DM_ERROR_INVALID_PARAM, errMsg);
        env->GlobalReference_Delete(cbRef);
        return;
    }
    sptr<DisplayAniListener> displayAniListener = sptr<DisplayAniListener>::MakeSptr(env);
    if (displayAniListener == nullptr) {
        TLOGE(WmsLogTag::DMS, "[ANI]displayListener is nullptr");
        env->GlobalReference_Delete(cbRef);
        AniErrUtils::ThrowBusinessError(env, DMError::DM_ERROR_INVALID_PARAM, "displayListener is nullptr");
        return;
    }
    displayAniListener->AddCallback(typeString, cbRef);
    displayAniListener->SetMainEventHandler();

    ret = ProcessRegisterCallback(env, typeString, displayAniListener);
    if (ret != DmErrorCode::DM_OK) {
        TLOGE(WmsLogTag::DMS, "[ANI] register display listener with type, errcode: %{public}d", ret);
        env->GlobalReference_Delete(cbRef);
        std::string errMsg = "Failed to register display listener with type";
        AniErrUtils::ThrowBusinessError(env, ret, errMsg);
        return;
    }
    // add listener to map
    jsCbMap_[typeString][cbRef] = displayAniListener;
}

bool DisplayManagerAni::IsCallbackRegistered(ani_env* env, const std::string& type, ani_ref callback)
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

DmErrorCode DisplayManagerAni::ProcessRegisterCallback(ani_env* env, std::string& typeStr,
    sptr<DisplayAniListener> displayAniListener)
{
    DmErrorCode ret = DmErrorCode::DM_ERROR_INVALID_PARAM;
    if (typeStr == ANI_EVENT_ADD || typeStr == ANI_EVENT_REMOVE || typeStr == ANI_EVENT_CHANGE) {
        TLOGI(WmsLogTag::DMS, "ProcessRegisterCallback %{public}s", typeStr.c_str());
        ret = DM_JS_TO_ERROR_CODE_MAP.at(
            SingletonContainer::Get<DisplayManager>().RegisterDisplayListener(displayAniListener));
    } else if (typeStr == ANI_EVENT_FOLD_STATUS_CHANGED) {
        ret = DM_JS_TO_ERROR_CODE_MAP.at(
            SingletonContainer::Get<DisplayManager>().RegisterFoldStatusListener(displayAniListener));
    } else if (typeStr == ANI_EVENT_DISPLAY_MODE_CHANGED) {
        ret = DM_JS_TO_ERROR_CODE_MAP.at(
            SingletonContainer::Get<DisplayManager>().RegisterDisplayModeListener(displayAniListener));
    } else if (typeStr == ANI_EVENT_AVAILABLE_AREA_CHANGED) {
        ret = DM_JS_TO_ERROR_CODE_MAP.at(
            SingletonContainer::Get<DisplayManager>().RegisterAvailableAreaListener(displayAniListener));
    } else if (typeStr == ANI_EVENT_FOLD_ANGLE_CHANGED) {
        ret = DM_JS_TO_ERROR_CODE_MAP.at(
            SingletonContainer::Get<DisplayManager>().RegisterFoldAngleListener(displayAniListener));
    } else if (typeStr == ANI_EVENT_CAPTURE_STATUS_CHANGED) {
        ret = DM_JS_TO_ERROR_CODE_MAP.at(
            SingletonContainer::Get<DisplayManager>().RegisterCaptureStatusListener(displayAniListener));
    } else if (typeStr == ANI_EVENT_PRIVATE_MODE_CHANGE) {
        ret = DM_JS_TO_ERROR_CODE_MAP.at(
            SingletonContainer::Get<DisplayManager>().RegisterPrivateWindowListener(displayAniListener));
    }
    return ret;
}

void DisplayManagerAni::UnRegisterCallback(ani_env* env, ani_string type,
    ani_long nativeObj, ani_ref callback)
{
    TLOGI(WmsLogTag::DMS, "[ANI] begin");
    DisplayManagerAni* displayManagerAni = reinterpret_cast<DisplayManagerAni*>(nativeObj);
    if (displayManagerAni != nullptr) {
        displayManagerAni->OnUnRegisterCallback(env, type, callback);
    } else {
        TLOGI(WmsLogTag::DMS, "[ANI] null ptr");
    }
}

void DisplayManagerAni::OnUnRegisterCallback(ani_env* env, ani_string type, ani_ref callback)
{
    TLOGI(WmsLogTag::DMS, "[ANI] begin");
    ani_ref cbRef{};
    if (env->GlobalReference_Create(callback, &cbRef) != ANI_OK) {
        TLOGE(WmsLogTag::DMS, "[ANI] create global ref fail");
        return;
    }
    std::string typeString;
    DisplayAniUtils::GetStdString(env, type, typeString);
    std::lock_guard<std::mutex> lock(mtx_);
    ani_boolean callbackNull = 0;
    env->Reference_IsUndefined(cbRef, &callbackNull);
    DmErrorCode ret;
    if (callbackNull) {
        TLOGI(WmsLogTag::DMS, "[ANI] for all");
        ret = DM_JS_TO_ERROR_CODE_MAP.at(UnregisterAllDisplayListenerWithType(typeString));
    } else {
        TLOGI(WmsLogTag::DMS, "[ANI] with type");
        ret = DM_JS_TO_ERROR_CODE_MAP.at(UnRegisterDisplayListenerWithType(typeString, env, cbRef));
    }

    if (ret != DmErrorCode::DM_OK) {
        DmErrorCode errCode = DmErrorCode::DM_ERROR_INVALID_PARAM;
        if (ret == DmErrorCode::DM_ERROR_NOT_SYSTEM_APP) {
            errCode = ret;
        }
        std::string errMsg = "[ANI] failed to unregister display listener with type";
        TLOGE(WmsLogTag::DMS, "[ANI] failed to unregister display listener with type");
        AniErrUtils::ThrowBusinessError(env, DMError::DM_ERROR_INVALID_PARAM, errMsg);
    }
    env->GlobalReference_Delete(cbRef);
}

DMError DisplayManagerAni::UnRegisterDisplayListenerWithType(std::string type, ani_env* env, ani_ref callback)
{
    TLOGI(WmsLogTag::DMS, "[ANI] begin");
    if (jsCbMap_.empty() || jsCbMap_.find(type) == jsCbMap_.end()) {
        TLOGI(WmsLogTag::DMS, "[ANI] methodName %{public}s not registered!", type.c_str());
        return DMError::DM_OK;
    }
    DMError ret = DMError::DM_OK;
    for (auto it = jsCbMap_[type].begin(); it != jsCbMap_[type].end(); it++) {
        ani_boolean isEquals = 0;
        env->Reference_StrictEquals(callback, it->first, &isEquals);
        if (isEquals) {
            it->second->RemoveCallback(env, type, callback);
            if (type == ANI_EVENT_ADD || type == ANI_EVENT_REMOVE || type == ANI_EVENT_CHANGE) {
                sptr<DisplayManager::IDisplayListener> thisListener(it->second);
                ret = SingletonContainer::Get<DisplayManager>().UnregisterDisplayListener(thisListener);
            } else if (type == ANI_EVENT_FOLD_STATUS_CHANGED) {
                sptr<DisplayManager::IFoldStatusListener> thisListener(it->second);
                ret = SingletonContainer::Get<DisplayManager>().UnregisterFoldStatusListener(thisListener);
                TLOGI(WmsLogTag::DMS, "[ANI] UnRegisterDisplayListener foldStatusChange success");
            } else if (type == ANI_EVENT_DISPLAY_MODE_CHANGED) {
                sptr<DisplayManager::IDisplayModeListener> thisListener(it->second);
                ret = SingletonContainer::Get<DisplayManager>().UnregisterDisplayModeListener(thisListener);
                TLOGI(WmsLogTag::DMS, "[ANI] UnRegisterDisplayListener foldDisplayModeChange success");
            } else if (type == ANI_EVENT_AVAILABLE_AREA_CHANGED) {
                sptr<DisplayManager::IAvailableAreaListener> thisListener(it->second);
                ret = SingletonContainer::Get<DisplayManager>().UnregisterAvailableAreaListener(thisListener);
                TLOGI(WmsLogTag::DMS, "[ANI] UnRegisterDisplayListener availableAreaListener success");
            } else if (type == ANI_EVENT_FOLD_ANGLE_CHANGED) {
                sptr<DisplayManager::IFoldAngleListener> thisListener(it->second);
                ret = SingletonContainer::Get<DisplayManager>().UnregisterFoldAngleListener(thisListener);
                TLOGI(WmsLogTag::DMS, "[ANI] UnRegisterDisplayListener foldAngleListener success");
            } else if (type == ANI_EVENT_CAPTURE_STATUS_CHANGED) {
                sptr<DisplayManager::ICaptureStatusListener> thisListener(it->second);
                SingletonContainer::Get<DisplayManager>().UnregisterCaptureStatusListener(thisListener);
                TLOGI(WmsLogTag::DMS, "[ANI] UnRegisterDisplayListener captureStatusListener success");
            } else if (type == ANI_EVENT_PRIVATE_MODE_CHANGE) {
                sptr<DisplayManager::IPrivateWindowListener> thisListener(it->second);
                ret = SingletonContainer::Get<DisplayManager>().UnregisterPrivateWindowListener(thisListener);
                TLOGI(WmsLogTag::DMS, "[ANI] UnRegisterDisplayListener privateWindowListener success");
            }
            jsCbMap_[type].erase(it);
            break;
        }
    }
    if (jsCbMap_[type].empty()) {
        jsCbMap_.erase(type);
    }
    return ret;
}

DMError DisplayManagerAni::UnregisterAllDisplayListenerWithType(std::string type)
{
    TLOGI(WmsLogTag::DMS, "[ANI] begin");
    if (jsCbMap_.empty() || jsCbMap_.find(type) == jsCbMap_.end()) {
        TLOGI(WmsLogTag::DMS, "[ANI] methodName %{public}s not registered!",
            type.c_str());
        return DMError::DM_OK;
    }
    DMError ret = DMError::DM_OK;
    for (auto it = jsCbMap_[type].begin(); it != jsCbMap_[type].end();) {
        it->second->RemoveAllCallback();
        if (type == ANI_EVENT_ADD || type == ANI_EVENT_REMOVE || type == ANI_EVENT_CHANGE) {
            sptr<DisplayManager::IDisplayListener> thisListener(it->second);
            ret = SingletonContainer::Get<DisplayManager>().UnregisterDisplayListener(thisListener);
        } else if (type == ANI_EVENT_FOLD_STATUS_CHANGED) {
            sptr<DisplayManager::IFoldStatusListener> thisListener(it->second);
            ret = SingletonContainer::Get<DisplayManager>().UnregisterFoldStatusListener(thisListener);
        } else if (type == ANI_EVENT_DISPLAY_MODE_CHANGED) {
            sptr<DisplayManager::IDisplayModeListener> thisListener(it->second);
            ret = SingletonContainer::Get<DisplayManager>().UnregisterDisplayModeListener(thisListener);
        } else if (type == ANI_EVENT_AVAILABLE_AREA_CHANGED) {
            sptr<DisplayManager::IAvailableAreaListener> thisListener(it->second);
            ret = SingletonContainer::Get<DisplayManager>().UnregisterAvailableAreaListener(thisListener);
        } else if (type == ANI_EVENT_FOLD_ANGLE_CHANGED) {
            sptr<DisplayManager::IFoldAngleListener> thisListener(it->second);
            ret = SingletonContainer::Get<DisplayManager>().UnregisterFoldAngleListener(thisListener);
        } else if (type == ANI_EVENT_CAPTURE_STATUS_CHANGED) {
            sptr<DisplayManager::ICaptureStatusListener> thisListener(it->second);
            ret = SingletonContainer::Get<DisplayManager>().UnregisterCaptureStatusListener(thisListener);
        } else if (type == ANI_EVENT_PRIVATE_MODE_CHANGE) {
            sptr<DisplayManager::IPrivateWindowListener> thisListener(it->second);
            ret = SingletonContainer::Get<DisplayManager>().UnregisterPrivateWindowListener(thisListener);
        }
        jsCbMap_[type].erase(it++);
    }
    jsCbMap_.erase(type);
    return ret;
}

ani_boolean DisplayManagerAni::HasPrivateWindow(ani_env* env, ani_long displayId)
{
    TLOGI(WmsLogTag::DMS, "[ANI] begin");
    bool hasPrivateWindow = false;
    if (displayId < 0) {
        TLOGE(WmsLogTag::DMS, "Invalid displayId: %{public}" PRId64, displayId);
        AniErrUtils::ThrowBusinessError(env, DmErrorCode::DM_ERROR_INVALID_PARAM, "Invalid displayId");
        return hasPrivateWindow;
    }
    DmErrorCode errCode = DM_JS_TO_ERROR_CODE_MAP.at(
        SingletonContainer::Get<DisplayManager>().HasPrivateWindow(static_cast<int64_t>(displayId), hasPrivateWindow));
    if (errCode != DmErrorCode::DM_OK) {
        TLOGE(WmsLogTag::DMS, "Failed to convert parameter to displayId");
        AniErrUtils::ThrowBusinessError(env, errCode, "Failed to convert parameter to displayId");
        return hasPrivateWindow;
    }
    return hasPrivateWindow;
}

void DisplayManagerAni::GetAllDisplayPhysicalResolution(ani_env* env, ani_object arrayObj, ani_long nativeObj)
{
    TLOGI(WmsLogTag::DMS, "[ANI] begin");
    DisplayManagerAni* displayManagerAni = reinterpret_cast<DisplayManagerAni*>(nativeObj);
    displayManagerAni->OnGetAllDisplayPhysicalResolution(env, arrayObj);
}

void DisplayManagerAni::OnGetAllDisplayPhysicalResolution(ani_env* env, ani_object arrayObj)
{
    TLOGI(WmsLogTag::DMS, "[ANI] begin");
    std::vector<DisplayPhysicalResolution> displayPhysicalArray =
            SingletonContainer::Get<DisplayManager>().GetAllDisplayPhysicalResolution();
    if (displayPhysicalArray.empty()) {
        AniErrUtils::ThrowBusinessError(env, DmErrorCode::DM_ERROR_SYSTEM_INNORMAL,
            "JsDisplayManager::OnGetAllDisplayPhysicalResolution failed.");
    } else {
        DisplayAniUtils::ConvertDisplayPhysicalResolution(displayPhysicalArray, arrayObj, env);
    }
}

void DisplayManagerAni::FinalizerDisplay(ani_env* env, ani_object displayObj, ani_long nativeObj)
{
    TLOGI(WmsLogTag::DMS, "[ANI] DMS FinalizerDisplayNative begin");
    DisplayManagerAni* displayManagerAni = reinterpret_cast<DisplayManagerAni*>(nativeObj);
    displayManagerAni->OnFinalizerDisplay(env, displayObj);
}
 
void DisplayManagerAni::OnFinalizerDisplay(ani_env* env, ani_object displayObj)
{
    TLOGI(WmsLogTag::DMS, "[ANI] DMS FinalizerDisplayNative begin");
    ani_long displayId;
    if (ANI_OK != env->Object_GetFieldByName_Long(displayObj, "<property>id", &displayId)) {
        TLOGE(WmsLogTag::DMS, "[ANI] DMS FinalizerDisplayNative get displayId failed");
        return;
    }
    DisplayAniUtils::DisposeAniDisplayObject(static_cast<DisplayId>(displayId));
}
}
}
