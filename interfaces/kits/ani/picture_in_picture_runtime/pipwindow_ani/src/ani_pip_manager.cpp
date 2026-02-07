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

#include "ani_pip_manager.h"

#include <sstream>

#include "xcomponent_controller.h"

#include "window_manager_hilog.h"
#include "ani_pip_utils.h"

namespace OHOS {
namespace Rosen {
using namespace AbilityRuntime;
using namespace Ace;
namespace {
    static std::map<ani_object, AniPipManager*> localObjs;
    const std::unordered_set<std::string> PIP_CONTENT_CALLBACK {"stateChange", "nodeUpdate"};
}

ani_status AniPipManager::Init(ani_env* env, ani_namespace nsp)
{
    TLOGI(WmsLogTag::WMS_PIP, "AniPipManager Init");
    if (env == nullptr) {
        TLOGE(WmsLogTag::WMS_PIP, "null env");
        return ANI_ERROR;
    }

    ani_function setObjFunc = nullptr;
    ani_status ret = env->Namespace_FindFunction(nsp, "setNativeObj", "l:", &setObjFunc);
    if (ret != ANI_OK) {
        TLOGE(WmsLogTag::WMS_PIP, "find setNativeObj func fail %{public}u", ret);
        return ret;
    }

    std::unique_ptr<AniPipManager> aniPiPManager = std::make_unique<AniPipManager>();

    ret = env->Function_Call_Void(setObjFunc, aniPiPManager.release());
    if (ret != ANI_OK) {
        TLOGE(WmsLogTag::WMS_PIP, "find setNativeObj func fail %{public}u", ret);
        return ret;
    }
    return ret;
}

AniPipManager::AniPipManager(){TLOGI(WmsLogTag::WMS_PIP, "AniPipManager");}
AniPipManager::~AniPipManager(){TLOGI(WmsLogTag::WMS_PIP, "~AniPipManager");}

void AniPipManager::InitXComponentController(ani_env* env, ani_long nativeObj, ani_object xComponentController)
{
    TLOGI(WmsLogTag::WMS_PIP, "InitXComponentController");
    AniPipManager* me = reinterpret_cast<AniPipManager*>(nativeObj);
    if (me != nullptr) {
        return me->OnInitXComponentController(env, xComponentController);
    } else {
        TLOGE(WmsLogTag::WMS_PIP, "AniPipManager is nullptr");
        return;
    }
}

void AniPipManager::OnInitXComponentController(ani_env* env, ani_object xComponentController)
{
    TLOGD(WmsLogTag::WMS_PIP, "OnInitXComponentController");
    std::shared_ptr<XComponentController> xComponentControllerResult =
        XComponentController::GetXComponentControllerFromAniValue(env, xComponentController);
    sptr<Window> pipWindow = Window::Find(PIP_WINDOW_NAME);
    if (pipWindow == nullptr) {
        TLOGE(WmsLogTag::WMS_PIP, "Failed to find pip window");
        return;
    }
    int32_t windowId = static_cast<int32_t>(pipWindow->GetWindowId());
    auto pipController = PictureInPictureManager::GetPipControllerInfo(windowId);
    if (pipController == nullptr) {
        TLOGE(WmsLogTag::WMS_PIP, "Failed to get pipController");
        return;
    }
    TLOGI(WmsLogTag::WMS_PIP, "set xComponentController to window: %{public}u", windowId);
    WMError errCode = pipController->SetXComponentController(xComponentControllerResult);
    if (errCode != WMError::WM_OK) {
        TLOGE(WmsLogTag::WMS_PIP, "Failed to set xComponentController");
    }
    return;
}

ani_ref AniPipManager::GetCustomUIController(ani_env* env, ani_long nativeObj)
{
    AniPipManager* me = reinterpret_cast<AniPipManager*>(nativeObj);
    if (me != nullptr) {
        return me->OnGetCustomUIController(env);
    } else {
        TLOGE(WmsLogTag::WMS_PIP, "AniPipManager is nullptr");
        return nullptr;
    }
}

ani_ref AniPipManager::OnGetCustomUIController(ani_env* env)
{
    TLOGD(WmsLogTag::WMS_PIP, "OnGetCustomUIController");
    sptr<Window> pipWindow = Window::Find(PIP_WINDOW_NAME);
    if (pipWindow == nullptr) {
        TLOGE(WmsLogTag::WMS_PIP, "Failed to find pip window");
        return AniGetUndefined(env);
    }
    int32_t windowId = static_cast<int32_t>(pipWindow->GetWindowId());
    TLOGI(WmsLogTag::WMS_PIP, "winId: %{public}u", windowId);
    auto pipController = PictureInPictureManager::GetPipControllerInfo(windowId);
    if (pipController == nullptr) {
        TLOGE(WmsLogTag::WMS_PIP, "Failed to get pipController");
        return AniGetUndefined(env);
    }
    ani_ref ref = pipController->GetANICustomNodeController();
    if (ref == nullptr) {
        TLOGI(WmsLogTag::WMS_PIP, "invalid custom UI controller");
        return AniGetUndefined(env);
    }
    return ref;
}

ani_ref AniPipManager::GetTypeNode(ani_env* env, ani_long nativeObj)
{
    AniPipManager* me = reinterpret_cast<AniPipManager*>(nativeObj);
    if (me != nullptr) {
        return me->OnGetTypeNode(env);
    } else {
        TLOGE(WmsLogTag::WMS_PIP, "AniPipManager is nullptr");
        return nullptr;
    }
}

ani_ref AniPipManager::OnGetTypeNode(ani_env* env)
{
    TLOGD(WmsLogTag::WMS_PIP, "OnGetTypeNode");
    sptr<Window> pipWindow = Window::Find(PIP_WINDOW_NAME);
    if (pipWindow == nullptr) {
        TLOGE(WmsLogTag::WMS_PIP, "Failed to find pip window");
        return AniGetUndefined(env);
    }
    int32_t windowId = static_cast<int32_t>(pipWindow->GetWindowId());
    TLOGI(WmsLogTag::WMS_PIP, "winId: %{public}u", windowId);
    auto pipController = PictureInPictureManager::GetPipControllerInfo(windowId);
    if (pipController == nullptr) {
        TLOGE(WmsLogTag::WMS_PIP, "Failed to get pipController");
        return AniGetUndefined(env);
    }
    ani_ref ref = pipController->GetANITypeNode();
    if (!ref) {
        TLOGE(WmsLogTag::WMS_PIP, "invalid typeNode");
        return AniGetUndefined(env);
    }
    return ref;
}
 
void AniPipManager::SetTypeNodeEnabled(ani_env* env, ani_long nativeObj)
{
    TLOGI(WmsLogTag::WMS_PIP, "SetTypeNodeEnabled start");
    AniPipManager* me = reinterpret_cast<AniPipManager*>(nativeObj);
    if (me != nullptr) {
        return me->OnSetTypeNodeEnabled(env);
    } else {
        TLOGE(WmsLogTag::WMS_PIP, "AniPipManager is nullptr");
        return;
    }
}

void AniPipManager::OnSetTypeNodeEnabled(ani_env* env)
{
    TLOGD(WmsLogTag::WMS_PIP, "OnSetTypeNodeEnabled");
    sptr<Window> pipWindow = Window::Find(PIP_WINDOW_NAME);
    if (pipWindow == nullptr) {
        TLOGE(WmsLogTag::WMS_PIP, "Failed to find pip window");
        return;
    }
    int32_t windowId = static_cast<int32_t>(pipWindow->GetWindowId());
    TLOGI(WmsLogTag::WMS_PIP, "winId: %{public}u", windowId);
    auto pipController = PictureInPictureManager::GetPipControllerInfo(windowId);
    if (pipController == nullptr) {
        TLOGE(WmsLogTag::WMS_PIP, "Failed to get PictureInPictureControllerBase");
        return;
    }
    pipController->OnPictureInPictureStart();
    return;
}

void AniPipManager::SetPipNodeType(ani_env* env, ani_long nativeObj, ani_object typeNode, ani_boolean markPip)
{
    TLOGI(WmsLogTag::WMS_PIP, "SetPipNodeType start");
    AniPipManager* me = reinterpret_cast<AniPipManager*>(nativeObj);
    if (me != nullptr) {
        return me->OnSetPipNodeType(env, typeNode, markPip);
    } else {
        TLOGE(WmsLogTag::WMS_PIP, "AniPipManager is nullptr");
        return;
    }
}

void AniPipManager::OnSetPipNodeType(ani_env* env, ani_object typeNode, ani_boolean markPip)
{
    TLOGD(WmsLogTag::WMS_PIP, "OnSetPipNodeType start");
    ani_boolean isUndefined;
    env->Reference_IsUndefined(typeNode, &isUndefined);
    if (!isUndefined) {
        XComponentControllerErrorCode ret =
            XComponentController::SetSurfaceCallbackModeFromAniValue(env, typeNode, markPip ?
            SurfaceCallbackMode::PIP: SurfaceCallbackMode::DEFAULT);
        TLOGI(WmsLogTag::WMS_PIP, "set surface mode, ret:%{public}u, isPip:%{public}d",
            static_cast<uint32_t>(ret), static_cast<uint32_t>(markPip));
    }
    return;
}

void AniPipManager::RegisterCallback(ani_env* env, ani_long nativeObj, ani_string type, ani_ref callback)
{
    TLOGI(WmsLogTag::WMS_PIP, "RegisterCallback start");
    AniPipManager* me = reinterpret_cast<AniPipManager*>(nativeObj);
    if (me != nullptr) {
        return me->OnRegisterCallback(env, type, callback);
    } else {
        TLOGE(WmsLogTag::WMS_PIP, "AniPipManager is nullptr");
        return;
    }
}

void AniPipManager::OnRegisterCallback(ani_env* env, ani_string type, ani_ref callback)
{
    std::string cbType;
    GetStdString(env, type, cbType);
    TLOGI(WmsLogTag::WMS_PIP, "type:%{public}s", cbType.c_str());
    if (PIP_CONTENT_CALLBACK.count(cbType) == 0) {
        TLOGE(WmsLogTag::WMS_PIP, "OnRegisterCallback cbType is not in PIP_CONTENT_CALLBACK");
        return;
    }
    if (!callback) {
        TLOGE(WmsLogTag::WMS_PIP, "Callback is null");
        return;
    }

    ani_ref fnRef = {};
    env->GlobalReference_Create(callback, &fnRef);

    sptr<Window> pipWindow = Window::Find(PIP_WINDOW_NAME);
    if (pipWindow == nullptr) {
        TLOGE(WmsLogTag::WMS_PIP, "Failed to find pip window");
        return;
    }
    int32_t windowId = static_cast<int32_t>(pipWindow->GetWindowId());
    auto pipController = PictureInPictureManager::GetPipControllerInfo(windowId);
    if (!pipController) {
        TLOGE(WmsLogTag::WMS_PIP, "Failed to get PictureInPictureControllerBase");
        return;
    }
    TLOGI(WmsLogTag::WMS_PIP, "OnRegisterCallback to window:%{public}d", windowId);
    WMError errCode = pipController->RegisterPipContentListenerWithType(cbType, fnRef);
    if (errCode != WMError::WM_OK) {
        TLOGE(WmsLogTag::WMS_PIP, "Failed to registerCallback");
        return;
    }
    TLOGI(WmsLogTag::WMS_PIP, "Register type %{public}s success!", cbType.c_str());
    return;
}

void AniPipManager::UnregisterCallback(ani_env* env, ani_long nativeObj, ani_string type)
{
    TLOGI(WmsLogTag::WMS_PIP, "UnregisterCallback start");
    AniPipManager* me = reinterpret_cast<AniPipManager*>(nativeObj);
    if (me != nullptr) {
        return me->OnUnregisterCallback(env, type);
    } else {
        TLOGE(WmsLogTag::WMS_PIP, "AniPipManager is nullptr");
        return;
    }
}

void AniPipManager::OnUnregisterCallback(ani_env* env, ani_string type)
{
    std::string cbType;
    GetStdString(env, type, cbType);
    TLOGI(WmsLogTag::WMS_PIP, "OnUnregisterCallback type:%{public}s", cbType.c_str());
    if (PIP_CONTENT_CALLBACK.count(cbType) == 0) {
        TLOGE(WmsLogTag::WMS_PIP, "cbType is not in PIP_CONTENT_CALLBACK");
        return;
    }

    sptr<Window> pipWindow = Window::Find(PIP_WINDOW_NAME);
    if (pipWindow == nullptr) {
        TLOGE(WmsLogTag::WMS_PIP, "Failed to find pip window");
        return;
    }
    int32_t windowId = static_cast<int32_t>(pipWindow->GetWindowId());
    auto pipController = PictureInPictureManager::GetPipControllerInfo(windowId);
    if (pipController == nullptr) {
        TLOGE(WmsLogTag::WMS_PIP, "Failed to get pictureInPictureController");
        return;
    }
    TLOGI(WmsLogTag::WMS_PIP, "UnRegisterPipContentListenerWithType to window:%{public}d", windowId);
    WMError errCode = pipController->UnRegisterPipContentListenerWithType(cbType);
    if (errCode != WMError::WM_OK) {
        TLOGE(WmsLogTag::WMS_PIP, "Failed to set UnRegisterPipContentListenerWithType");
        return;
    }
    TLOGI(WmsLogTag::WMS_PIP, "unregister type:%{public}s success!", cbType.c_str());
    return;
}
} // namespace Rosen
} // namespace OHOS


ani_status OHOS::Rosen::ANI_Manager_Constructor(ani_vm *vm, uint32_t *result)
{
    TLOGI(WmsLogTag::WMS_PIP, "start");
    ani_status ret;
    ani_env* env;
    if ((ret = vm->GetEnv(ANI_VERSION_1, &env)) != ANI_OK) {
        TLOGE(WmsLogTag::WMS_PIP, "GetEnv failed");
        return ANI_NOT_FOUND;
    }
    ani_namespace nsp;
    if ((ret = env->FindNamespace("@ohos.PiPWindow.PiPManager", &nsp)) != ANI_OK) {
        TLOGE(WmsLogTag::WMS_PIP, "FindNamespace failed,ani_status ret is %{public}u", ret);
        return ANI_NOT_FOUND;
    }

    std::array funcs = {
        ani_native_function{"InitXComponentController", nullptr,
            reinterpret_cast<void*>(AniPipManager::InitXComponentController)},
        ani_native_function{"GetCustomUIController", nullptr,
            reinterpret_cast<void*>(AniPipManager::GetCustomUIController)},
        ani_native_function{"GetTypeNode", nullptr, reinterpret_cast<void*>(AniPipManager::GetTypeNode)},
        ani_native_function{"RegisterCallback", nullptr, reinterpret_cast<void*>(AniPipManager::RegisterCallback)},
        ani_native_function{"UnregisterCallback", "lC{std.core.String}:",
            reinterpret_cast<void*>(AniPipManager::UnregisterCallback)},
        ani_native_function{"SetTypeNodeEnabled", "l:", reinterpret_cast<void*>(AniPipManager::SetTypeNodeEnabled)},
        ani_native_function{"SetPipNodeType", nullptr, reinterpret_cast<void*>(AniPipManager::SetPipNodeType)},
    };

    for (auto func : funcs) {
        if ((ret = env->Namespace_BindNativeFunctions(nsp, &func, 1u)) != ANI_OK) {
            TLOGE(WmsLogTag::WMS_PIP, "bind namespace fail %{public}u, %{public}s, %{public}s",
                ret, func.name, func.signature);
            return ANI_NOT_FOUND;
        }
    }
    AniPipManager::Init(env, nsp);
    *result = ANI_VERSION_1;
    return ANI_OK;
}
