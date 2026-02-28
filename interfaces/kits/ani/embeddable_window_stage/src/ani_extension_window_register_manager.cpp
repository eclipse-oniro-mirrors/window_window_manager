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

#include "ani_extension_window_register_manager.h"

#include "ani.h"
#include "ani_extension_window_listener.h"
#include "singleton_container.h"
#include "window_manager.h"
#include "window_manager_hilog.h"

namespace OHOS {
namespace Rosen {

AniExtensionWindowRegisterManager::AniExtensionWindowRegisterManager()
{
    // white register list for window
    listenerCodeMap_[CaseType::CASE_WINDOW] = {
        {WINDOW_SIZE_CHANGE_CB, ListenerType::WINDOW_SIZE_CHANGE_CB},
        {AVOID_AREA_CHANGE_CB, ListenerType::AVOID_AREA_CHANGE_CB},
        {WINDOW_EVENT_CB, ListenerType::WINDOW_EVENT_CB},
        {WINDOW_RECT_CHANGE_CB, ListenerType::WINDOW_RECT_CHANGE_CB},
    };
    // white register list for window stage
    listenerCodeMap_[CaseType::CASE_STAGE] = {
        {WINDOW_STAGE_EVENT_CB, ListenerType::WINDOW_STAGE_EVENT_CB}
    };
}

AniExtensionWindowRegisterManager::~AniExtensionWindowRegisterManager()
{
}

WmErrorCode AniExtensionWindowRegisterManager::ProcessWindowChangeRegister(sptr<AniExtensionWindowListener>& listener,
    sptr<Window>& window, bool isRegister)
{
    if (window == nullptr) {
        TLOGE(WmsLogTag::WMS_UIEXT, "[ANI]window is nullptr");
        return WmErrorCode::WM_ERROR_STATE_ABNORMALLY;
    }
    sptr<IWindowChangeListener> thisListener(listener);
    WmErrorCode ret = WmErrorCode::WM_OK;
    if (isRegister) {
        ret = WM_JS_TO_ERROR_CODE_MAP.at(window->RegisterWindowChangeListener(thisListener));
    } else {
        ret = WM_JS_TO_ERROR_CODE_MAP.at(window->UnregisterWindowChangeListener(thisListener));
    }
    return ret;
}

WmErrorCode AniExtensionWindowRegisterManager::ProcessAvoidAreaChangeRegister(
    sptr<AniExtensionWindowListener>& listener, sptr<Window>& window, bool isRegister)
{
    if (window == nullptr) {
        TLOGE(WmsLogTag::WMS_UIEXT, "[ANI]window is nullptr");
        return WmErrorCode::WM_ERROR_STATE_ABNORMALLY;
    }
    sptr<IAvoidAreaChangedListener> thisListener(listener);
    WmErrorCode ret = WmErrorCode::WM_OK;
    if (isRegister) {
        ret = WM_JS_TO_ERROR_CODE_MAP.at(window->RegisterAvoidAreaChangeListener(thisListener));
    } else {
        ret = WM_JS_TO_ERROR_CODE_MAP.at(window->UnregisterAvoidAreaChangeListener(thisListener));
    }
    return ret;
}

WmErrorCode AniExtensionWindowRegisterManager::ProcessLifeCycleEventRegister(sptr<AniExtensionWindowListener>& listener,
    sptr<Window>& window, bool isRegister)
{
    if (window == nullptr) {
        TLOGE(WmsLogTag::WMS_UIEXT, "[ANI]window is nullptr");
        return WmErrorCode::WM_ERROR_STATE_ABNORMALLY;
    }
    sptr<IWindowLifeCycle> thisListener(listener);
    WmErrorCode ret = WmErrorCode::WM_OK;
    if (isRegister) {
        ret = WM_JS_TO_ERROR_CODE_MAP.at(window->RegisterLifeCycleListener(thisListener));
    } else {
        ret = WM_JS_TO_ERROR_CODE_MAP.at(window->UnregisterLifeCycleListener(thisListener));
    }
    return ret;
}

WmErrorCode AniExtensionWindowRegisterManager::ProcessWindowRectChangeRegister(
    sptr<AniExtensionWindowListener>& listener, sptr<Window>& window, bool isRegister)
{
    if (window == nullptr) {
        TLOGE(WmsLogTag::WMS_UIEXT, "[ANI]window is nullptr");
        return WmErrorCode::WM_ERROR_STATE_ABNORMALLY;
    }
    WmErrorCode ret = WmErrorCode::WM_OK;
    if (isRegister) {
        ret = WM_JS_TO_ERROR_CODE_MAP.at(window->RegisterWindowRectChangeListener(listener));
    } else {
        ret = WM_JS_TO_ERROR_CODE_MAP.at(window->UnregisterWindowRectChangeListener(listener));
    }
    return ret;
}

bool AniExtensionWindowRegisterManager::IsCallbackRegistered(ani_env* env, const std::string& type, ani_object fn)
{
    if (aniCbMap_.empty() || aniCbMap_.find(type) == aniCbMap_.end()) {
        TLOGI(WmsLogTag::WMS_UIEXT, "[ANI]Method %{public}s has not been registered", type.c_str());
        return false;
    }

    for (auto iter = aniCbMap_[type].begin(); iter != aniCbMap_[type].end(); ++iter) {
        ani_ref callback = static_cast<ani_ref>(fn);
        ani_boolean isEqual = ANI_FALSE;
        env->Reference_StrictEquals(callback, iter->first, &isEqual);
        if (isEqual) {
            TLOGE(WmsLogTag::WMS_UIEXT, "[ANI]Method %{public}s has already been registered", type.c_str());
            return true;
        }
    }
    return false;
}

WmErrorCode AniExtensionWindowRegisterManager::RegisterListener(sptr<Window>& window, const std::string& type,
    CaseType caseType, ani_env* env, ani_object fn)
{
    std::lock_guard<std::mutex> lock(mtx_);
    if (IsCallbackRegistered(env, type, fn)) {
        return WmErrorCode::WM_ERROR_STATE_ABNORMALLY;
    }
    if (listenerCodeMap_[caseType].count(type) == 0) {
        TLOGE(WmsLogTag::WMS_UIEXT, "[ANI]Type %{public}s listener is not supported", type.c_str());
        return WmErrorCode::WM_ERROR_STATE_ABNORMALLY;
    }
    ani_status ret {};
    ani_ref fnRef {};
    if ((ret = env->GlobalReference_Create(fn, &fnRef)) != ANI_OK) {
        TLOGE(WmsLogTag::WMS_UIEXT, "[ANI]Create fn global reference failed, ret: %{public}u", ret);
        return WmErrorCode::WM_ERROR_STATE_ABNORMALLY;
    };
    ani_vm* vm = nullptr;
    ani_status aniRet = env->GetVM(&vm);
    if (aniRet != ANI_OK || vm == nullptr) {
        TLOGE(WmsLogTag::WMS_UIEXT, "[ANI]Get VM failed, ret: %{public}u", aniRet);
        return WmErrorCode::WM_ERROR_STATE_ABNORMALLY;
    }
    auto extensionWindowListener = sptr<AniExtensionWindowListener>::MakeSptr(env, vm, fnRef);
    if (extensionWindowListener == nullptr) {
        TLOGE(WmsLogTag::WMS_UIEXT, "[ANI]New AniExtensionWindowListener failed");
        return WmErrorCode::WM_ERROR_STATE_ABNORMALLY;
    }
    extensionWindowListener->SetMainEventHandler();
    WmErrorCode retCode = ProcessRegister(caseType, extensionWindowListener, window, type, true);
    if (retCode != WmErrorCode::WM_OK) {
        TLOGE(WmsLogTag::WMS_UIEXT, "[ANI]Register type %{public}s listener failed, ret: %{public}d",
            type.c_str(), retCode);
        env->GlobalReference_Delete(fnRef);
        return retCode;
    }
    aniCbMap_[type][fnRef] = extensionWindowListener;
    TLOGI(WmsLogTag::WMS_UIEXT, "[ANI]Register type %{public}s listener success! callback map size: %{public}zu",
        type.c_str(), aniCbMap_[type].size());
    return WmErrorCode::WM_OK;
}

WmErrorCode AniExtensionWindowRegisterManager::UnregisterListener(sptr<Window>& window, const std::string& type,
    CaseType caseType, ani_env* env, ani_object fn)
{
    std::lock_guard<std::mutex> lock(mtx_);
    if (aniCbMap_.empty() || aniCbMap_.find(type) == aniCbMap_.end()) {
        TLOGE(WmsLogTag::WMS_UIEXT, "[ANI]Type %{public}s listener was not registered", type.c_str());
        return WmErrorCode::WM_ERROR_STATE_ABNORMALLY;
    }
    if (listenerCodeMap_[caseType].count(type) == 0) {
        TLOGE(WmsLogTag::WMS_UIEXT, "[ANI]Type %{public}s listener is not supported", type.c_str());
        return WmErrorCode::WM_ERROR_STATE_ABNORMALLY;
    }
    ani_boolean isUndefined = ANI_FALSE;
    env->Reference_IsUndefined(static_cast<ani_ref>(fn), &isUndefined);
    if (isUndefined == ANI_TRUE) {
        TLOGI(WmsLogTag::WMS_UIEXT, "[ANI]Unregister all callback, type: %{public}s", type.c_str());
        for (auto it = aniCbMap_[type].begin(); it != aniCbMap_[type].end();) {
            WmErrorCode ret = ProcessRegister(caseType, it->second, window, type, false);
            if (ret != WmErrorCode::WM_OK) {
                TLOGE(WmsLogTag::WMS_UIEXT, "[ANI]Unregister type %{public}s listener failed, ret: %{public}d",
                    type.c_str(), ret);
                return ret;
            }
            env->GlobalReference_Delete(it->second->GetAniCallback());
            it->second->SetAniCallback(nullptr);
            aniCbMap_[type].erase(it++);
        }
    } else {
        bool findFlag = false;
        for (auto it = aniCbMap_[type].begin(); it != aniCbMap_[type].end(); ++it) {
            ani_ref callback = static_cast<ani_ref>(fn);
            ani_boolean isEqual = ANI_FALSE;
            env->Reference_StrictEquals(callback, it->first, &isEqual);
            if (!isEqual) {
                continue;
            }
            findFlag = true;
            WmErrorCode ret = ProcessRegister(caseType, it->second, window, type, false);
            if (ret != WmErrorCode::WM_OK) {
                TLOGE(WmsLogTag::WMS_UIEXT, "[ANI]Unregister type %{public}s listener failed, ret: %{public}d",
                    type.c_str(), ret);
                return ret;
            }
            env->GlobalReference_Delete(it->second->GetAniCallback());
            it->second->SetAniCallback(nullptr);
            aniCbMap_[type].erase(it);
            break;
        }
        if (!findFlag) {
            TLOGE(WmsLogTag::WMS_UIEXT,
                "[ANI]Unregister type %{public}s listener failed because not found callback!", type.c_str());
            return WmErrorCode::WM_ERROR_STATE_ABNORMALLY;
        }
    }
    TLOGI(WmsLogTag::WMS_UIEXT, "[ANI]Unregister type %{public}s listener success! callback map size: %{public}zu",
        type.c_str(), aniCbMap_[type].size());
    // erase type when there is no callback in one type
    if (aniCbMap_[type].empty()) {
        aniCbMap_.erase(type);
    }
    return WmErrorCode::WM_OK;
}

WmErrorCode AniExtensionWindowRegisterManager::ProcessRegister(CaseType caseType,
    sptr<AniExtensionWindowListener>& listener, sptr<Window>& window, const std::string& type,
    bool isRegister)
{
    WmErrorCode ret = WmErrorCode::WM_OK;
    if (caseType == CaseType::CASE_WINDOW) {
        switch (listenerCodeMap_[caseType][type]) {
            case ListenerType::WINDOW_SIZE_CHANGE_CB:
                ret = ProcessWindowChangeRegister(listener, window, isRegister);
                break;
            case ListenerType::AVOID_AREA_CHANGE_CB:
                ret = ProcessAvoidAreaChangeRegister(listener, window, isRegister);
                break;
            case ListenerType::WINDOW_EVENT_CB:
                ret = ProcessLifeCycleEventRegister(listener, window, isRegister);
                break;
            case ListenerType::WINDOW_RECT_CHANGE_CB:
                ret = ProcessWindowRectChangeRegister(listener, window, isRegister);
                break;
            default:
                break;
        }
    } else if (caseType == CaseType::CASE_STAGE) {
        if (type == WINDOW_STAGE_EVENT_CB) {
            ret = ProcessLifeCycleEventRegister(listener, window, isRegister);
        }
    }
    return ret;
}

} // namespace Rosen
} // namespace OHOS