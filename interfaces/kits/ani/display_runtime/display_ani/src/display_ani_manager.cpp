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
#include <hitrace_meter.h>
#include <algorithm>

#include "ani.h"
#include "display_ani_listener.h"
#include "display_ani.h"
#include "display_ani_manager.h"
#include "display_info.h"
#include "display.h"
#include "singleton_container.h"
#include "display_manager.h"
#include "window_manager_hilog.h"
#include "dm_common.h"
#include "display_ani_utils.h"
#include "refbase.h"
#include "ani_err_utils.h"

namespace OHOS {
namespace Rosen {

DisplayManagerAni::DisplayManagerAni()
{
}

ani_status DisplayManagerAni::initDisplayManagerAni(ani_namespace displayNameSpace, ani_env* env)
{
    TLOGI(WmsLogTag::DEFAULT, "[ANI]");
    ani_function setObjFunc = nullptr;
    ani_status ret = env->Namespace_FindFunction(displayNameSpace, "setDisplayMgrRef", "J:V", &setObjFunc);
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

ani_int DisplayManagerAni::getFoldDisplayModeAni(ani_env* env)
{
    auto mode = SingletonContainer::Get<DisplayManager>().GetFoldDisplayMode();
    TLOGI(WmsLogTag::DMS, "[ANI]" PRIu64", getFoldDisplayMode = %{public}u", mode);
    return static_cast<ani_int>(mode);
}

ani_boolean DisplayManagerAni::isFoldableAni(ani_env* env)
{
    bool foldable = SingletonContainer::Get<DisplayManager>().IsFoldable();
    TLOGI(WmsLogTag::DMS, "[ANI]" PRIu64", isFoldable = %{public}u", foldable);
    return static_cast<ani_boolean>(foldable);
}

ani_int DisplayManagerAni::getFoldStatus(ani_env* env)
{
    auto status = SingletonContainer::Get<DisplayManager>().GetFoldStatus();
    TLOGI(WmsLogTag::DMS, "[ANI]" PRIu64", getFoldStatus = %{public}u", status);
    return static_cast<ani_int>(status);
}

ani_object DisplayManagerAni::getCurrentFoldCreaseRegion(ani_env* env, ani_object obj)
{
    auto region = SingletonContainer::Get<DisplayManager>().GetCurrentFoldCreaseRegion();
    ani_class rectCls;
    if (ANI_OK != env->FindClass("L@ohos/display/display/RectImpl;", &rectCls)) {
        TLOGE(WmsLogTag::DMS, "[ANI] null class RectImpl");
        return obj;
    }
    ani_int displayId = static_cast<ani_int>(region->GetDisplayId());
    std::vector<DMRect> rects = region->GetCreaseRects();
    ani_array_ref aniRects = nullptr;
    if (ANI_OK != env->Object_SetFieldByName_Int(obj, "<property>displayId", displayId)) {
        TLOGE(WmsLogTag::DMS, "[ANI] set displayId field fail");
    }
    if (ANI_OK != env->Object_SetFieldByName_Ref(obj, "<property>creaseRects", aniRects)) {
        TLOGE(WmsLogTag::DMS, "[ANI] set creaseRects field fail");
    }
    return obj;
}

void DisplayManagerAni::getAllDisplaysAni(ani_env* env, ani_object arrayObj)
{
    TLOGI(WmsLogTag::DMS, "[ANI] getAllDisplaysAni start");
    std::vector<sptr<Display>> displays = SingletonContainer::Get<DisplayManager>().GetAllDisplays();
    if (displays.empty()) {
        AniErrUtils::ThrowBusinessError(env, DmErrorCode::DM_ERROR_INVALID_SCREEN, "");
    }
    ani_double length;
    if (ANI_OK != env->Object_GetPropertyByName_Double(arrayObj, "length", &length)) {
        TLOGE(WmsLogTag::DMS, "[ANI] get ani_array len fail");
    }

    for (int i = 0; i < std::min(int(length), static_cast<int>(displays.size())); i++) {
        ani_ref currentDisplay;
        if (ANI_OK != env->Object_CallMethodByName_Ref(arrayObj, "$_get",
            "I:Lstd/core/Object;", &currentDisplay, (ani_int)i)) {
            TLOGE(WmsLogTag::DMS, "[ANI] get ani_array index %{public}u fail", (ani_int)i);
        }
        TLOGI(WmsLogTag::DMS, "current i: %{public}d", i);
        DisplayAniUtils::cvtDisplay(displays[i], env, static_cast<ani_object>(currentDisplay));
    }
    TLOGI(WmsLogTag::DMS, "[ANI] getAllDisplaysAni end");
}

ani_status DisplayManagerAni::getDisplayByIdSyncAni(ani_env* env, ani_object obj, ani_int displayId)
{
    TLOGE(WmsLogTag::DMS, "[ANI] getDisplayByIdSyncAni begin");
    if (displayId < 0) {
        TLOGE(WmsLogTag::DMS, "[ANI] Invalid displayId, less than 0");
        return ANI_ERROR;
    }
    sptr<Display> display = SingletonContainer::Get<DisplayManager>().GetDisplayById(static_cast<DisplayId>(displayId));
    if (display == nullptr) {
        AniErrUtils::ThrowBusinessError(env, DmErrorCode::DM_ERROR_SYSTEM_INNORMAL, "");
    }
    if (display == nullptr) {
        TLOGE(WmsLogTag::DMS, "[ANI] Display null");
        return ANI_ERROR;
    }
    ani_status ret = DisplayAniUtils::cvtDisplay(display, env, obj);
    return ret;
}

ani_status DisplayManagerAni::getDefaultDisplaySyncAni(ani_env* env, ani_object obj)
{
    sptr<Display> display = SingletonContainer::Get<DisplayManager>().GetDefaultDisplaySync(true);
    if (display == nullptr) {
        TLOGE(WmsLogTag::DMS, "[ANI] Display null");
        return ANI_ERROR;
    }
    TLOGI(WmsLogTag::DMS, "[ANI] getDefaultDisplaySyncAni");
    ani_status ret = DisplayAniUtils::cvtDisplay(display, env, obj);
    return ret;
}

void DisplayManagerAni::registerCallback(ani_env* env, ani_string type,
    ani_ref callback, ani_long nativeObj)
{
    DisplayManagerAni* displayManagerAni = reinterpret_cast<DisplayManagerAni*>(nativeObj);
    if (displayManagerAni != nullptr) {
        displayManagerAni->onRegisterCallback(env, type, callback);
    } else {
        TLOGI(WmsLogTag::DMS, "[ANI] null ptr");
    }
}

void DisplayManagerAni::onRegisterCallback(ani_env* env, ani_string type, ani_ref callback)
{
    TLOGI(WmsLogTag::DMS, "[ANI] onRegisterCallback");
    std::lock_guard<std::mutex> lock(mtx_);
    std::string typeString;
    DisplayAniUtils::GetStdString(env, type, typeString);
    ani_boolean callbackNull = 0;
    env->Reference_IsNull(callback, &callbackNull);
    DmErrorCode ret;
    if (callbackNull) {
        std::string errMsg = "[ANI] failed to register display listener with type";
        TLOGE(WmsLogTag::DMS, "callbackNull");
        AniErrUtils::ThrowBusinessError(env, DmErrorCode::DM_ERROR_INVALID_PARAM, errMsg);
        return;
    }
    ani_ref cbRef{};
    if (env->GlobalReference_Create(callback, &cbRef) != ANI_OK) {
        TLOGE(WmsLogTag::DEFAULT, "[ANI]create global ref fail");
    };
    TLOGI(WmsLogTag::DMS, "create listener");
    sptr<DisplayAniListener> displayAniListener = new(std::nothrow) DisplayAniListener(env);
    if (displayAniListener == nullptr) {
        TLOGE(WmsLogTag::DMS, "[ANI]displayListener is nullptr");
        AniErrUtils::ThrowBusinessError(env, DMError::DM_ERROR_INVALID_PARAM, "displayListener is nullptr");
        return;
    }
    displayAniListener->AddCallback(typeString, cbRef);

    displayAniListener->SetMainEventHandler();
    ret = processRegisterCallback(env, typeString, displayAniListener);
    if (ret != DmErrorCode::DM_OK) {
        TLOGE(WmsLogTag::DMS, "[ANI] register display listener with type, errcode: %{public}d", ret);
        std::string errMsg = "Failed to register display listener with type";
        AniErrUtils::ThrowBusinessError(env, DmErrorCode::DM_ERROR_INVALID_PARAM, errMsg);
        return;
    }
    // add listener to map
    jsCbMap_[typeString][callback] = displayAniListener;
    if (typeString == EVENT_ADD) {
        displayAniListener->OnCreate(1);
    }
}

DmErrorCode DisplayManagerAni::processRegisterCallback(ani_env* env, std::string& typeStr,
    sptr<DisplayAniListener> displayAniListener)
{
    DmErrorCode ret = DmErrorCode::DM_ERROR_INVALID_PARAM;
    if (typeStr == EVENT_ADD || typeStr == EVENT_REMOVE || typeStr == EVENT_CHANGE) {
        TLOGI(WmsLogTag::DMS, "processRegisterCallback %{public}s", typeStr.c_str());
        ret = DM_JS_TO_ERROR_CODE_MAP.at(
            SingletonContainer::Get<DisplayManager>().RegisterDisplayListener(displayAniListener));
    } else if (typeStr == EVENT_FOLD_STATUS_CHANGED) {
        ret = DM_JS_TO_ERROR_CODE_MAP.at(
            SingletonContainer::Get<DisplayManager>().RegisterFoldStatusListener(displayAniListener));
    } else if (typeStr == EVENT_DISPLAY_MODE_CHANGED) {
        ret = DM_JS_TO_ERROR_CODE_MAP.at(
            SingletonContainer::Get<DisplayManager>().RegisterDisplayModeListener(displayAniListener));
    }
    return ret;
}

void DisplayManagerAni::unRegisterCallback(ani_env* env, ani_string type,
    ani_ref callback, ani_long nativeObj)
{
    TLOGI(WmsLogTag::DMS, "[ANI] unRegisterCallback begin");
    DisplayManagerAni* displayManagerAni = reinterpret_cast<DisplayManagerAni*>(nativeObj);
    if (displayManagerAni != nullptr) {
        displayManagerAni->onUnRegisterCallback(env, type, callback);
    } else {
        TLOGI(WmsLogTag::DMS, "[ANI] null ptr");
    }
}

void DisplayManagerAni::onUnRegisterCallback(ani_env* env, ani_string type, ani_ref callback)
{
    TLOGI(WmsLogTag::DMS, "[ANI] onUnRegisterCallback begin");
    std::string typeString;
    DisplayAniUtils::GetStdString(env, type, typeString);
    std::lock_guard<std::mutex> lock(mtx_);
    ani_boolean callbackNull = 0;
    env->Reference_IsNull(callback, &callbackNull);
    DmErrorCode ret;
    if (callbackNull) {
        TLOGI(WmsLogTag::DMS, "[ANI] onUnRegisterCallback for all");
        ret = DM_JS_TO_ERROR_CODE_MAP.at(UnregisterAllDisplayListenerWithType(typeString));
    } else {
        TLOGI(WmsLogTag::DMS, "[ANI] onUnRegisterCallback with type");
        ret = DM_JS_TO_ERROR_CODE_MAP.at(UnRegisterDisplayListenerWithType(typeString, env, callback));
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
}

DMError DisplayManagerAni::UnRegisterDisplayListenerWithType(std::string type, ani_env* env, ani_ref callback)
{
    TLOGI(WmsLogTag::DMS, "[ANI] UnRegisterDisplayListenerWithType begin");
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
            if (type == EVENT_ADD || type == EVENT_REMOVE || type == EVENT_CHANGE) {
                TLOGI(WmsLogTag::DMS, "[ANI] start to unregis display event listener! event = %{public}s",
                    type.c_str());
                sptr<DisplayManager::IDisplayListener> thisListener(it->second);
                ret = SingletonContainer::Get<DisplayManager>().UnregisterDisplayListener(thisListener);
            } else if (type == EVENT_FOLD_STATUS_CHANGED) {
                sptr<DisplayManager::IFoldStatusListener> thisListener(it->second);
                ret = SingletonContainer::Get<DisplayManager>().UnregisterFoldStatusListener(thisListener);
            } else if (type == EVENT_DISPLAY_MODE_CHANGED) {
                sptr<DisplayManager::IDisplayModeListener> thisListener(it->second);
                ret = SingletonContainer::Get<DisplayManager>().UnregisterDisplayModeListener(thisListener);
            }
        }
    }
    return ret;
}

DMError DisplayManagerAni::UnregisterAllDisplayListenerWithType(std::string type)
{
    TLOGI(WmsLogTag::DMS, "[ANI] UnregisterAllDisplayListenerWithType begin");
    if (jsCbMap_.empty() || jsCbMap_.find(type) == jsCbMap_.end()) {
        TLOGI(WmsLogTag::DMS, "[ANI] UnregisterAllDisplayListenerWithType methodName %{public}s not registered!",
            type.c_str());
        return DMError::DM_OK;
    }
    DMError ret = DMError::DM_OK;
    for (auto it = jsCbMap_[type].begin(); it != jsCbMap_[type].end();) {
        it->second->RemoveAllCallback();
        if (type == EVENT_ADD || type == EVENT_REMOVE || type == EVENT_CHANGE) {
            sptr<DisplayManager::IDisplayListener> thisListener(it->second);
            ret = SingletonContainer::Get<DisplayManager>().UnregisterDisplayListener(thisListener);
        } else if (type == EVENT_FOLD_STATUS_CHANGED) {
            sptr<DisplayManager::IFoldStatusListener> thisListener(it->second);
            ret = SingletonContainer::Get<DisplayManager>().UnregisterFoldStatusListener(thisListener);
        } else if (type == EVENT_DISPLAY_MODE_CHANGED) {
            sptr<DisplayManager::IDisplayModeListener> thisListener(it->second);
            ret = SingletonContainer::Get<DisplayManager>().UnregisterDisplayModeListener(thisListener);
        }
    }
    return ret;
}
}
}