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

#include "ani_pip_window.h"

#include "ani.h"

#include <hitrace_meter.h>

#include "window_manager_hilog.h"
#include "ani_pip_utils.h"
#include "ani_pip_manager.h"

namespace OHOS {
namespace Rosen {
namespace {
    constexpr uint32_t MAX_CONTROL_GROUP_NUM = 3;
    const std::set<PiPControlGroup> VIDEO_PLAY_CONTROLS {
        PiPControlGroup::VIDEO_PREVIOUS_NEXT,
        PiPControlGroup::FAST_FORWARD_BACKWARD,
    };
    const std::set<PiPControlGroup> VIDEO_CALL_CONTROLS {
        PiPControlGroup::VIDEO_CALL_MICROPHONE_SWITCH,
        PiPControlGroup::VIDEO_CALL_HANG_UP_BUTTON,
        PiPControlGroup::VIDEO_CALL_CAMERA_SWITCH,
        PiPControlGroup::VIDEO_CALL_MUTE_SWITCH,
    };
    const std::set<PiPControlGroup> VIDEO_MEETING_CONTROLS {
        PiPControlGroup::VIDEO_MEETING_HANG_UP_BUTTON,
        PiPControlGroup::VIDEO_MEETING_CAMERA_SWITCH,
        PiPControlGroup::VIDEO_MEETING_MUTE_SWITCH,
        PiPControlGroup::VIDEO_MEETING_MICROPHONE_SWITCH,
    };
    const std::set<PiPControlGroup> VIDEO_LIVE_CONTROLS {
        PiPControlGroup::VIDEO_PLAY_PAUSE,
        PiPControlGroup::VIDEO_LIVE_MUTE_SWITCH,
    };
    const std::map<PiPTemplateType, std::set<PiPControlGroup>> TEMPLATE_CONTROL_MAP {
        {PiPTemplateType::VIDEO_PLAY, VIDEO_PLAY_CONTROLS},
        {PiPTemplateType::VIDEO_CALL, VIDEO_CALL_CONTROLS},
        {PiPTemplateType::VIDEO_MEETING, VIDEO_MEETING_CONTROLS},
        {PiPTemplateType::VIDEO_LIVE, VIDEO_LIVE_CONTROLS},
    };
}

ani_status AniPiPWindow::Init(ani_env* env, ani_namespace nsp)
{
    TLOGI(WmsLogTag::WMS_PIP, "start");
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
    std::unique_ptr<AniPiPWindow> aniPiPWindow = std::make_unique<AniPiPWindow>();
    ret = env->Function_Call_Void(setObjFunc, aniPiPWindow.release());
    if (ret != ANI_OK) {
        TLOGE(WmsLogTag::WMS_PIP, "call setNativeObj func failed: %{public}u", ret);
        return ret;
    }
    return ret;
}

ani_ref AniPiPWindow::CreateAniPiPControllerObject(ani_env* env, sptr<PictureInPictureControllerAni>& pipController)
{
    TLOGI(WmsLogTag::WMS_PIP, "start");

    ani_class cls = nullptr;
    ani_status ret;
    if ((ret = env->FindClass("@ohos.PiPWindow.PiPWindow.PiPControllerInternal", &cls)) != ANI_OK) {
        TLOGD(WmsLogTag::WMS_PIP, "failed to find class PiPControllerInternal %{public}u", ret);
        return AniGetUndefined(env);
    }
    std::unique_ptr<AniPipController> aniPiPController = std::make_unique<AniPipController>(pipController);

    ani_method initFunc = nullptr;
    if ((ret = env->Class_FindMethod(cls, "<ctor>", ":", &initFunc)) != ANI_OK) {
        TLOGE(WmsLogTag::WMS_PIP, "get ctor fail %{public}u", ret);
        return AniGetUndefined(env);
    }
    ani_object obj = nullptr;
    if ((ret = env->Object_New(cls, initFunc, &obj)) != ANI_OK) {
        TLOGE(WmsLogTag::WMS_PIP, "failed to create ani obj %{public}u", ret);
        return AniGetUndefined(env);
    }
    ani_method setObjFunc = nullptr;
    if ((ret = env->Class_FindMethod(cls, "setNativeObj", "l:", &setObjFunc)) != ANI_OK) {
        TLOGE(WmsLogTag::WMS_PIP, "get setNativeObj fail %{public}u", ret);
        return AniGetUndefined(env);
    }

    ret = env->Object_CallMethod_Void(obj, setObjFunc, reinterpret_cast<ani_long>(aniPiPController.get()));
    if (ret != ANI_OK) {
        TLOGE(WmsLogTag::WMS_PIP, "set setNativeObj failed %{public}u", ret);
        return AniGetUndefined(env);
    }

    AddAniPiPControllerObj(aniPiPController.release(), obj);
    TLOGI(WmsLogTag::WMS_PIP, "finish");
    return obj;
}

void AniPiPWindow::AddAniPiPControllerObj(AniPipController* aniPipController, ani_object obj)
{
    std::lock_guard<std::mutex> lock(mtxLocalObjs_);
    localObjs.insert(std::pair(aniPipController, obj));
}

ani_status AniPiPWindow::DelAniPiPControllerObj(AniPipController* aniPipController)
{
    std::lock_guard<std::mutex> lock(mtxLocalObjs_);
    auto it = localObjs.find(aniPipController);
    if (it != localObjs.end()) {
        TLOGI(WmsLogTag::WMS_SYSTEM, "del aniPipController success");
        delete aniPipController;
        localObjs.erase(it);
        return ANI_OK;
    }
    TLOGE(WmsLogTag::WMS_SYSTEM, "del aniPipController failed");
    return ANI_ERROR;
}

void AniPiPWindow::Finalizer(ani_env* env, ani_long nativeObj)
{
    TLOGI(WmsLogTag::WMS_PIP, "Finalizer start");
    AniPipController* aniPipController = reinterpret_cast<AniPipController*>(nativeObj);
    if (!aniPipController) {
        AniThrowError(env, WMError::WM_ERROR_NULLPTR, "aniPipController is nullptr, skip");
        return;
    }
    AniPipController::DelListener(env);
    ani_status ret = DelAniPiPControllerObj(aniPipController);
    if (ret == ANI_OK) {
        TLOGI(WmsLogTag::WMS_SYSTEM, "Finalizer finish");
    }
}

static bool checkControlsRules(uint32_t pipTemplateType, std::vector<std::uint32_t> controlGroups)
{
    TLOGI(WmsLogTag::WMS_PIP, "checkControlsRules");
    auto iter = TEMPLATE_CONTROL_MAP.find(static_cast<PiPTemplateType>(pipTemplateType));
    auto controls = iter->second;
    if (controlGroups.size() > MAX_CONTROL_GROUP_NUM) {
        return false;
    }
    for (auto control : controlGroups) {
        if (controls.find(static_cast<PiPControlGroup>(control)) == controls.end()) {
            TLOGE(WmsLogTag::WMS_PIP, "pipOption param error, controlGroup not matches, controlGroup: %{public}u",
                control);
            return false;
        }
    }
    if (pipTemplateType == static_cast<uint32_t>(PiPTemplateType::VIDEO_PLAY)) {
        auto iterFirst = std::find(controlGroups.begin(), controlGroups.end(),
            static_cast<uint32_t>(PiPControlGroup::VIDEO_PREVIOUS_NEXT));
        auto iterSecond = std::find(controlGroups.begin(), controlGroups.end(),
            static_cast<uint32_t>(PiPControlGroup::FAST_FORWARD_BACKWARD));
        if (iterFirst != controlGroups.end() && iterSecond != controlGroups.end()) {
            TLOGE(WmsLogTag::WMS_PIP,
                "ipOption param error, %{public}u conflicts with %{public}u in controlGroups",
                static_cast<uint32_t>(PiPControlGroup::VIDEO_PREVIOUS_NEXT),
                static_cast<uint32_t>(PiPControlGroup::FAST_FORWARD_BACKWARD));
            return false;
        }
    }
    return true;
}

static bool checkOptionParams(PipOptionAni& option)
{
    TLOGI(WmsLogTag::WMS_PIP, "checkOptionParams");
    if (option.GetContext() == nullptr) {
        TLOGE(WmsLogTag::WMS_PIP, "PipOptionAni param error, context is nullptr.");
        return false;
    }
    if (option.GetXComponentController() == nullptr) {
        TLOGE(WmsLogTag::WMS_PIP, "PipOptionAni param error, XComponentController is nullptr.");
        return false;
    }
    uint32_t pipTemplateType = option.GetPipTemplate();
    if (TEMPLATE_CONTROL_MAP.find(static_cast<PiPTemplateType>(pipTemplateType)) ==
        TEMPLATE_CONTROL_MAP.end()) {
        TLOGE(WmsLogTag::WMS_PIP, "PipOptionAni param error, pipTemplateType not exists.");
        return false;
    }
    uint32_t defaultWindowSizeType = option.GetDefaultWindowSizeType();
    if (!(defaultWindowSizeType == DefaultWindowSizeType::NOT_SET ||
            defaultWindowSizeType == DefaultWindowSizeType::SMALL ||
            defaultWindowSizeType == DefaultWindowSizeType::LARGE)) {
        TLOGE(WmsLogTag::WMS_PIP, "PipOptionAni param error, invalid defaultWindowSizeType:%{public}u",
            defaultWindowSizeType);
        return false;
    }
    return checkControlsRules(pipTemplateType, option.GetControlGroup());
}

static bool GetControlGroupFromJs(ani_env* env, ani_ref controlGroup, std::vector<std::uint32_t>& controls)
{
    TLOGI(WmsLogTag::WMS_PIP, "start");
    if (controlGroup == nullptr) {
        return false;
    }

    ani_size length = 0;
    auto array = static_cast<ani_array>(controlGroup);
    env->Array_GetLength(array, &length);

    for (size_t i = 0; i < length; i++) {
        ani_ref getElementValue = nullptr;
        env->Array_Get(array, i, &getElementValue);
        ani_int ret;
        env->EnumItem_GetValue_Int(static_cast<ani_enum_item>(getElementValue), &ret);
        uint32_t controlType = static_cast<uint32_t>(ret);

        auto iter = std::find(controls.begin(), controls.end(), controlType);
        if (iter != controls.end()) {
            TLOGI(WmsLogTag::WMS_PIP, "The controlType already exists. controlType: %{public}u", controlType);
        } else {
            controls.push_back(controlType);
        }
    }
    return true;
}

bool OptionSetContext(ani_env* env, ani_object optionObject, PipOptionAni& option)
{
    ani_ref contextPtrValue;
    env->Object_GetPropertyByName_Ref(optionObject, "context", &contextPtrValue);
    ani_boolean isContextUndefined = false;
    env->Reference_IsUndefined(contextPtrValue, &isContextUndefined);
    void* contextPtr = nullptr;
    if (!isContextUndefined) {
        TLOGI(WmsLogTag::WMS_PIP, "[GetPiPOption]begin");
        ani_object aniContextPtr = reinterpret_cast<ani_object>(contextPtrValue);
        contextPtr = GetAbilityContext(env, aniContextPtr);
        auto context = static_cast<std::weak_ptr<AbilityRuntime::Context>*>(contextPtr);
        if (context == nullptr) {
            TLOGE(WmsLogTag::WMS_PIP, "context is nullptr");
            return false;
        }
        option.SetContext(contextPtr);
        return true;
    }
    return false;
}

bool OptionSetXComponentController(ani_env* env, ani_object optionObject, PipOptionAni& option)
{
    ani_status result;
    ani_ref xComponentControllerRef = nullptr;
    result = env->Object_GetPropertyByName_Ref(optionObject, "componentController", &xComponentControllerRef);
    ani_boolean isComponentControllerUndefined = false;
    env->Reference_IsUndefined(xComponentControllerRef, &isComponentControllerUndefined);
    if (!isComponentControllerUndefined && result == ANI_OK) {
        TLOGI(WmsLogTag::WMS_PIP, "[GetPiPOption]set xComponentControllerResult begin");
        ani_object xComponentControllerValue = static_cast<ani_object>(xComponentControllerRef);
        std::shared_ptr<XComponentController> xComponentControllerResult =
            XComponentController::GetXComponentControllerFromAniValue(env, xComponentControllerValue);
        option.SetXComponentController(xComponentControllerResult);
        return true;
    }
    return false;
}

void OptionSetNavigationIdValue(ani_env* env, ani_object optionObject, PipOptionAni& option)
{
    ani_ref navigationIdValue;
    env->Object_GetPropertyByName_Ref(optionObject, "navigationId", &navigationIdValue);
    ani_boolean isNavigationIdValueUndefined = false;
    env->Reference_IsUndefined(navigationIdValue, &isNavigationIdValueUndefined);
    if (!isNavigationIdValueUndefined) {
        TLOGI(WmsLogTag::WMS_PIP, "[GetPiPOption]begin");
        ani_string aniNavigationIdValue = reinterpret_cast<ani_string>(navigationIdValue);
        std::string navigationId;
        GetStdString(env, aniNavigationIdValue, navigationId);
        TLOGI(WmsLogTag::WMS_PIP, "[GetPiPOption]navigationId: %{public}s", navigationId.c_str());
        option.SetNavigationId(navigationId);
    }
}

void OptionSetContentSize(ani_env* env, ani_object optionObject, PipOptionAni& option)
{
    ani_ref widthRef;
    ani_status rets;
    if ((rets = env->Object_GetPropertyByName_Ref(optionObject, "contentWidth", &widthRef)) != ANI_OK) {
        TLOGE(WmsLogTag::WMS_PIP, "contentWidth get fail %{public}u", rets);
        return;
    };

    ani_boolean isWidthRefUndefined = false;
    env->Reference_IsUndefined(widthRef, &isWidthRefUndefined);
    if (!widthRef || isWidthRefUndefined) {return;}

    ani_int widthValue;
    rets = env->Object_CallMethodByName_Int(static_cast<ani_object>(widthRef), "toInt", ":i", &widthValue);
    if (rets != ANI_OK) {
        TLOGE(WmsLogTag::WMS_PIP, "contentWidth get fail %{public}u", rets);
        return;
    }
    uint32_t width = static_cast<uint32_t>(widthValue);
    TLOGI(WmsLogTag::WMS_PIP, "[GetPiPOption]get width: %{public}d", width);

    ani_ref heightRef;
    if ((rets = env->Object_GetPropertyByName_Ref(optionObject, "contentHeight", &heightRef)) != ANI_OK) {
        TLOGE(WmsLogTag::WMS_PIP, "contentHeight get fail %{public}u", rets);
        return;
    };

    ani_boolean isHeightRefUndefined = false;
    env->Reference_IsUndefined(heightRef, &isHeightRefUndefined);
    if (!heightRef || isHeightRefUndefined) {return;}

    ani_int heightValue;
    rets = env->Object_CallMethodByName_Int(static_cast<ani_object>(heightRef), "toInt", ":i", &heightValue);
    if (rets != ANI_OK) {
        TLOGE(WmsLogTag::WMS_PIP, "contentHeight get fail %{public}u", rets);
        return;
    }
    uint32_t height = static_cast<uint32_t>(heightValue);
    TLOGI(WmsLogTag::WMS_PIP, "[GetPiPOption]get height: %{public}d", height);

    option.SetContentSize(width, height);
}

void OptionSetPipTemplate(ani_env* env, ani_object optionObject, PipOptionAni& option)
{
    ani_ref templateTypeValue;
    env->Object_GetPropertyByName_Ref(optionObject, "templateType", &templateTypeValue);
    
    ani_boolean isTemplateTypeValueUndefined = false;
    env->Reference_IsUndefined(templateTypeValue, &isTemplateTypeValueUndefined);
    if (!templateTypeValue || isTemplateTypeValueUndefined) {return;}

    ani_int ret;
    env->EnumItem_GetValue_Int(static_cast<ani_enum_item>(templateTypeValue), &ret);
    uint32_t templateType = static_cast<uint32_t>(ret);
    option.SetPipTemplate(templateType);
}

void OptionSetControlGroup(ani_env* env, ani_object optionObject, PipOptionAni& option)
{
    ani_ref controlGroup;
    std::vector<std::uint32_t> controls;
    if (ANI_OK != env->Object_GetPropertyByName_Ref(optionObject, "controlGroups", &controlGroup) &&
        option.GetPipTemplate() == static_cast<uint32_t>(PiPTemplateType::VIDEO_LIVE)) {
        TLOGI(WmsLogTag::WMS_PIP, "controls is undefined");
        controls.push_back(static_cast<uint32_t>(PiPControlGroup::VIDEO_PLAY_PAUSE));
    }
    GetControlGroupFromJs(env, controlGroup, controls);
    option.SetControlGroup(controls);
}

void OptionSetCustomUIController(ani_env* env, ani_object optionObject, PipOptionAni& option)
{
    ani_ref nodeController = nullptr;
    ani_status result = env->Object_GetPropertyByName_Ref(optionObject, "customUIController", &nodeController);
    ani_boolean isNodeControllerUndefined = false;
    env->Reference_IsUndefined(nodeController, &isNodeControllerUndefined);
    if (!isNodeControllerUndefined && result == ANI_OK) {
        TLOGI(WmsLogTag::WMS_PIP, "[GetPiPOption]set customUIController begin");
        ani_ref nodeControllerRef = nullptr;
        env->GlobalReference_Create(nodeController, &nodeControllerRef);
        option.SetNodeControllerRef(nodeControllerRef);
    }
}

void SetDefaultWindowSizeType(ani_env* env, ani_object optionObject, PipOptionAni& option)
{
    ani_status rets;
    ani_ref defaultWindowSizeTypeRef;
    rets = env->Object_GetPropertyByName_Ref(optionObject, "defaultWindowSizeType", &defaultWindowSizeTypeRef);
    ani_boolean isDefaultWindowSizeTypeRefUndefined = false;
    env->Reference_IsUndefined(defaultWindowSizeTypeRef, &isDefaultWindowSizeTypeRefUndefined);
    if (rets != ANI_OK || isDefaultWindowSizeTypeRefUndefined) {
        TLOGE(WmsLogTag::WMS_PIP, "defaultWindowSizeType get fail %{public}u", rets);
        return;
    };
    ani_int defaultWindowSizeTypeValue;
    rets = env->Object_CallMethodByName_Int(static_cast<ani_object>(defaultWindowSizeTypeRef),
        "toInt", ":i", &defaultWindowSizeTypeValue);
    if (rets != ANI_OK) {
        TLOGE(WmsLogTag::WMS_PIP, "contentWidth get fail %{public}u", rets);
        return;
    }
    uint32_t defaultWindowSizeType = static_cast<uint32_t>(defaultWindowSizeTypeValue);
    option.SetDefaultWindowSizeType(defaultWindowSizeType);
}

void SetSetStorage(ani_env* env, ani_object optionObject, PipOptionAni& option)
{
    ani_ref storageValue;
    ani_status result = env->Object_GetPropertyByName_Ref(optionObject, "localStorage", &storageValue);
    if (result == ANI_OK) {TLOGI(WmsLogTag::WMS_PIP, "[GetPiPOption]Object_GetPropertyByName_Ref localStorage ok");}
    TLOGI(WmsLogTag::WMS_PIP, "[GetPiPOption]get localStorage result: %{public}u", result);
    ani_boolean isStorageValueUndefined = false;
    env->Reference_IsUndefined(storageValue, &isStorageValueUndefined);
    if (!isStorageValueUndefined && result == ANI_OK) {
        TLOGI(WmsLogTag::WMS_PIP, "[GetPiPOption]set storageRef begin");
        ani_ref storageRef;
        env->GlobalReference_Create(storageValue, &storageRef);
        option.SetStorageRef(storageRef);
        TLOGI(WmsLogTag::WMS_PIP, "[GetPiPOption]localStorage is set");
        ani_boolean isStorageRefUndefined = false;
        env->Reference_IsUndefined(storageRef, &isStorageRefUndefined);
        if (isStorageRefUndefined) { TLOGE(WmsLogTag::WMS_PIP, "[GetPiPOption]storageRef is undefined"); }
    }
}

bool GetPiPOption(ani_env* env, ani_object optionObject, PipOptionAni& option)
{
    if (!OptionSetContext(env, optionObject, option) || !OptionSetXComponentController(env, optionObject, option)) {
        return false;
    }
    OptionSetNavigationIdValue(env, optionObject, option);
    OptionSetContentSize(env, optionObject, option);
    OptionSetPipTemplate(env, optionObject, option);
    OptionSetControlGroup(env, optionObject, option);
    OptionSetCustomUIController(env, optionObject, option);
    SetDefaultWindowSizeType(env, optionObject, option);
    SetSetStorage(env, optionObject, option);
    return checkOptionParams(option);
}

ani_ref AniPiPWindow::CreatePiPController(ani_env* env, PipOptionAni pipOption)
{
    TLOGI(WmsLogTag::WMS_PIP, "CreatePiPController");
    if (!PictureInPictureManager::IsSupportPiP()) {
        return AniThrowError(env, WMError::WM_ERROR_DEVICE_NOT_SUPPORT, "device not support pip");
    }
    sptr<PipOptionAni> pipOptionPtr = new PipOptionAni(pipOption);
    auto context = static_cast<std::weak_ptr<AbilityRuntime::Context>*>(pipOptionPtr->GetContext());
    if (context == nullptr) {
        return AniThrowError(env, WMError::WM_ERROR_PIP_INTERNAL_ERROR, "Invalid context");
    }
    sptr<Window> mainWindow = Window::GetMainWindowWithContext(context->lock());
    if (mainWindow == nullptr) {
        return AniThrowError(env, WMError::WM_ERROR_PIP_INTERNAL_ERROR, "Invalid mainWindow");
    }
    sptr<PictureInPictureControllerAni> pipController =
            new PictureInPictureControllerAni(pipOptionPtr, mainWindow, mainWindow->GetWindowId(), env);

    PiPTemplateInfo pipTemplateInfo;
    pipOption.GetPiPTemplateInfo(pipTemplateInfo);
    mainWindow->UpdatePiPTemplateInfo(pipTemplateInfo);

    return CreateAniPiPControllerObject(env, pipController);
}

AniPiPWindow::AniPiPWindow() {TLOGI(WmsLogTag::WMS_PIP, "AniPiPWindow");}
AniPiPWindow::~AniPiPWindow() {TLOGI(WmsLogTag::WMS_PIP, "~AniPiPWindow");}

ani_ref AniPiPWindow::Create(ani_env* env, ani_long nativeObj, ani_object configuration, ani_object nativeXComponent)
{
    TLOGI(WmsLogTag::WMS_PIP, "start");
    AniPiPWindow* pipWindowAni = reinterpret_cast<AniPiPWindow*>(nativeObj);
    return pipWindowAni != nullptr ? pipWindowAni->OnCreate(env, configuration, nativeXComponent) : nullptr;
}

ani_ref AniPiPWindow::OnCreate(ani_env* env, ani_object configuration, ani_object nativeXComponent)
{
    TLOGI(WmsLogTag::WMS_PIP, "start");
    ani_boolean isConfigurationUndefined = false;
    env->Reference_IsUndefined(configuration, &isConfigurationUndefined);
    if (!configuration || isConfigurationUndefined) {
        TLOGE(WmsLogTag::WMS_PIP, "config is null or undefined");
        return AniThrowError(env, WMError::WM_ERROR_INVALID_PARAM,
            "Failed to convert object to pipConfiguration or pipConfiguration is null");
    }
    PipOptionAni pipOption;
    if (!GetPiPOption(env, configuration, pipOption)) {
        std::string errMsg = "nvalid parameters in config, please check if context/xComponentController is null,"
            "or controlGroup mismatch the corresponding pipTemplateType, or defaultWindowSizeType is invalid";
        TLOGE(WmsLogTag::WMS_PIP, "%{public}s", errMsg.c_str());
        return AniThrowError(env, WMError::WM_ERROR_INVALID_PARAM, errMsg);
    }
    ani_ref nativeTypeNodeRef = nullptr;
    ani_boolean isnativeXComponentUndefined = false;
    env->Reference_IsUndefined(nativeXComponent, &isnativeXComponentUndefined);
    if (nativeXComponent && !isnativeXComponentUndefined) {
        TLOGI(WmsLogTag::WMS_PIP, "nativeXComponent enabled");
        pipOption.SetTypeNodeEnabled(true);
        env->GlobalReference_Create(nativeXComponent, &nativeTypeNodeRef);
        pipOption.SetTypeNodeRef(nativeTypeNodeRef);
    } else {
        TLOGW(WmsLogTag::WMS_PIP, "nativeXComponent is nullptr or undefined");
        pipOption.SetTypeNodeEnabled(false);
        pipOption.SetTypeNodeRef(nullptr);
    }

    return AniPiPWindow::CreatePiPController(env, pipOption);
}

ani_boolean AniPiPWindow::IsPiPEnabledAni(ani_env* env)
{
    TLOGI(WmsLogTag::WMS_PIP, "start");
    bool isPiPEnabled = PictureInPictureManager::GetPipEnabled();
    TLOGI(WmsLogTag::WMS_PIP, "isPiPEnabled = %{public}u", isPiPEnabled);
    return static_cast<ani_boolean>(isPiPEnabled);
}

}  // namespace Rosen
}  // namespace OHOS


// total entrance
extern "C" {
ANI_EXPORT ani_status OHOS::Rosen::ANI_Constructor(ani_vm *vm, uint32_t *result)
{
    using namespace OHOS::Rosen;
    ani_status ret;
    ani_env* env;
    TLOGI(WmsLogTag::WMS_PIP, "start");
    if ((ret = vm->GetEnv(ANI_VERSION_1, &env)) != ANI_OK) {
        TLOGE(WmsLogTag::WMS_PIP, "GetEnv failed");
        return ANI_NOT_FOUND;
    }
    ani_namespace nsp;
    if ((ret = env->FindNamespace("@ohos.PiPWindow.PiPWindow", &nsp)) != ANI_OK) {
        TLOGE(WmsLogTag::WMS_PIP, "FindNamespace failed,ani_status ret is %{public}u", ret);
        return ANI_NOT_FOUND;
    }

    std::array funcs = {
        ani_native_function{"isPiPEnabled", ":z", reinterpret_cast<void*>(AniPiPWindow::IsPiPEnabledAni)},
        ani_native_function{"createSync", nullptr, reinterpret_cast<void*>(AniPiPWindow::Create)},
        ani_native_function{"pipControllerFinalizerCallback", "l:", reinterpret_cast<void*>(AniPiPWindow::Finalizer)},
    };

    for (auto func : funcs) {
        if ((ret = env->Namespace_BindNativeFunctions(nsp, &func, 1u)) != ANI_OK) {
            TLOGE(WmsLogTag::WMS_PIP, "bind namespace fail %{public}u, %{public}s, %{public}s",
                ret, func.name, func.signature);
            return ANI_NOT_FOUND;
        }
    }

    ANI_Controller_Constructor(vm, result);
    ANI_Manager_Constructor(vm, result);

    AniPiPWindow::Init(env, nsp);
    *result = ANI_VERSION_1;
    return ANI_OK;
}  // ANI_Constructor
}  // extern