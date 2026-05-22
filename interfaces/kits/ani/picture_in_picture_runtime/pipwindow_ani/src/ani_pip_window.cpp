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
#include "interop_js/arkts_esvalue.h"
#include "interop_js/arkts_interop_js_api.h"
#include "interop_js/hybridgref_ani.h"
#include "interop_js/hybridgref_napi.h"
#include "permission.h"

#include "window_manager_hilog.h"
#include "picture_in_picture_controller.h"
#include "ani_pip_utils.h"
#include "ani_pip_manager.h"
#include "js_pip_controller.h"

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
    return ANI_OK;
}

ani_object AniPiPWindow::CreateAniPiPControllerObject(ani_env* env, sptr<PictureInPictureControllerAni>& pipController)
{
    TLOGI(WmsLogTag::WMS_PIP, "start");

    ani_class cls = nullptr;
    ani_status ret;
    if ((ret = env->FindClass("@ohos.PiPWindow.PiPWindow.PiPControllerInternal", &cls)) != ANI_OK) {
        TLOGD(WmsLogTag::WMS_PIP, "failed to find class PiPControllerInternal %{public}u", ret);
        return AniPipUtils::AniGetUndefined(env);
    }
    std::unique_ptr<AniPipController> aniPiPController = std::make_unique<AniPipController>(pipController);

    ani_method initFunc = nullptr;
    if ((ret = env->Class_FindMethod(cls, "<ctor>", ":", &initFunc)) != ANI_OK) {
        TLOGE(WmsLogTag::WMS_PIP, "get ctor fail %{public}u", ret);
        return AniPipUtils::AniGetUndefined(env);
    }
    ani_object obj = nullptr;
    if ((ret = env->Object_New(cls, initFunc, &obj)) != ANI_OK) {
        TLOGE(WmsLogTag::WMS_PIP, "failed to create ani obj %{public}u", ret);
        return AniPipUtils::AniGetUndefined(env);
    }
    ani_method setObjFunc = nullptr;
    if ((ret = env->Class_FindMethod(cls, "setNativeObj", "l:", &setObjFunc)) != ANI_OK) {
        TLOGE(WmsLogTag::WMS_PIP, "get setNativeObj fail %{public}u", ret);
        return AniPipUtils::AniGetUndefined(env);
    }

    ret = env->Object_CallMethod_Void(obj, setObjFunc, reinterpret_cast<ani_long>(aniPiPController.get()));
    if (ret != ANI_OK) {
        TLOGE(WmsLogTag::WMS_PIP, "set setNativeObj failed %{public}u", ret);
        return AniPipUtils::AniGetUndefined(env);
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
    if (it == localObjs.end()) {
        TLOGE(WmsLogTag::WMS_SYSTEM, "del aniPipController failed, no such aniPipController");
        return ANI_ERROR;
    }
    delete aniPipController;
    localObjs.erase(it);
    TLOGI(WmsLogTag::WMS_SYSTEM, "del aniPipController success");
    return ANI_OK;
}

void AniPiPWindow::Finalizer(ani_env* env, ani_long nativeObj)
{
    TLOGI(WmsLogTag::WMS_PIP, "start");
    AniPipController* aniPipController = reinterpret_cast<AniPipController*>(nativeObj);
    if (!aniPipController) {
        AniPipUtils::AniThrowError(env, WMError::WM_ERROR_NULLPTR, "aniPipController is nullptr, skip");
        return;
    }
    AniPipController::DelListener(env);
    ani_status ret = DelAniPiPControllerObj(aniPipController);
    if (ret == ANI_OK) {
        TLOGI(WmsLogTag::WMS_SYSTEM, "finish");
    }
}

bool AniPiPWindow::checkControlsRules(uint32_t pipTemplateType, std::vector<std::uint32_t> controlGroups)
{
    TLOGI(WmsLogTag::WMS_PIP, "start");
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

bool AniPiPWindow::checkOptionParams(PipOptionAni& option)
{
    TLOGI(WmsLogTag::WMS_PIP, "start");
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

bool AniPiPWindow::GetControlGroupFromJs(ani_env* env, ani_ref controlGroup, std::vector<std::uint32_t>& controls)
{
    TLOGI(WmsLogTag::WMS_PIP, "start");

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

bool AniPiPWindow::OptionSetContext(ani_env* env, ani_object optionObject, PipOptionAni& option)
{
    ani_status result;
    ani_ref contextPtrValue;
    result = env->Object_GetPropertyByName_Ref(optionObject, "context", &contextPtrValue);
    ani_boolean isContextUndefined = false;
    env->Reference_IsUndefined(contextPtrValue, &isContextUndefined);
    void* contextPtr = nullptr;
    if (isContextUndefined || result != ANI_OK) {
        TLOGE(WmsLogTag::WMS_PIP, "context is invalid");
        return false;
    }
    TLOGI(WmsLogTag::WMS_PIP, "[GetPiPOption]set context begin");
    ani_object aniContextPtr = reinterpret_cast<ani_object>(contextPtrValue);
    contextPtr = AniPipUtils::GetAbilityContext(env, aniContextPtr);
    auto context = static_cast<std::weak_ptr<AbilityRuntime::Context>*>(contextPtr);
    if (context == nullptr) {
        TLOGE(WmsLogTag::WMS_PIP, "context is nullptr");
        return false;
    }
    option.SetContext(contextPtr);
    return true;
}

bool AniPiPWindow::OptionSetXComponentController(ani_env* env, ani_object optionObject, PipOptionAni& option)
{
    ani_status result;
    ani_ref xComponentControllerRef = nullptr;
    result = env->Object_GetPropertyByName_Ref(optionObject, "componentController", &xComponentControllerRef);
    ani_boolean isComponentControllerUndefined = false;
    env->Reference_IsUndefined(xComponentControllerRef, &isComponentControllerUndefined);
    if (isComponentControllerUndefined || result != ANI_OK) {
        TLOGE(WmsLogTag::WMS_PIP, "componentController is invalid");
        return false;
    }
    TLOGI(WmsLogTag::WMS_PIP, "[GetPiPOption]set xComponentControllerResult begin");
    ani_object xComponentControllerValue = static_cast<ani_object>(xComponentControllerRef);
    std::shared_ptr<XComponentController> xComponentControllerResult =
        XComponentController::GetXComponentControllerFromAniValue(env, xComponentControllerValue);
    option.SetXComponentController(xComponentControllerResult);
    return true;
}

void AniPiPWindow::OptionSetNavigationIdValue(ani_env* env, ani_object optionObject, PipOptionAni& option)
{
    ani_status result;
    ani_ref navigationIdValue;
    result = env->Object_GetPropertyByName_Ref(optionObject, "navigationId", &navigationIdValue);
    ani_boolean isNavigationIdValueUndefined = false;
    env->Reference_IsUndefined(navigationIdValue, &isNavigationIdValueUndefined);
    if (isNavigationIdValueUndefined || result != ANI_OK) {
        return;
    }
    TLOGI(WmsLogTag::WMS_PIP, "[GetPiPOption]set navigationId begin");
    ani_string aniNavigationIdValue = reinterpret_cast<ani_string>(navigationIdValue);
    std::string navigationId;
    AniPipUtils::GetStdString(env, aniNavigationIdValue, navigationId);
    TLOGI(WmsLogTag::WMS_PIP, "[GetPiPOption]navigationId: %{public}s", navigationId.c_str());
    option.SetNavigationId(navigationId);
}

void AniPiPWindow::GetWidthFromAniObject(ani_env* env, ani_object optionObject, uint32_t& width)
{
    ani_ref widthRef;
    ani_status rets;
    if ((rets = env->Object_GetPropertyByName_Ref(optionObject, "contentWidth", &widthRef)) != ANI_OK) {
        return;
    }
    ani_boolean isWidthRefUndefined = false;
    env->Reference_IsUndefined(widthRef, &isWidthRefUndefined);
    if (isWidthRefUndefined) {
        return;
    }
    ani_int widthValue;
    rets = env->Object_CallMethodByName_Int(static_cast<ani_object>(widthRef), "toInt", ":i", &widthValue);
    if (rets != ANI_OK) {
        TLOGI(WmsLogTag::WMS_PIP, "get widthValue failed");
        return;
    }
    width = static_cast<uint32_t>(widthValue);
    TLOGI(WmsLogTag::WMS_PIP, "[GetPiPOption]get width: %{public}d", width);
}

void AniPiPWindow::GetHeightFromAniObject(ani_env* env, ani_object optionObject, uint32_t& height)
{
    ani_ref heightRef;
    ani_status rets;
    if ((rets = env->Object_GetPropertyByName_Ref(optionObject, "contentHeight", &heightRef)) != ANI_OK) {
        return;
    }
    ani_boolean isHeightRefUndefined = false;
    env->Reference_IsUndefined(heightRef, &isHeightRefUndefined);
    if (isHeightRefUndefined) {
        return;
    }
    ani_int heightValue;
    rets = env->Object_CallMethodByName_Int(static_cast<ani_object>(heightRef), "toInt", ":i", &heightValue);
    if (rets != ANI_OK) {
        TLOGI(WmsLogTag::WMS_PIP, "get heightValue failed");
        return;
    }
    height = static_cast<uint32_t>(heightValue);
    TLOGI(WmsLogTag::WMS_PIP, "[GetPiPOption]get height: %{public}d", height);
}

void AniPiPWindow::OptionSetContentSize(ani_env* env, ani_object optionObject, PipOptionAni& option)
{
    uint32_t width = 0;
    uint32_t height = 0;
    GetWidthFromAniObject(env, optionObject, width);
    GetHeightFromAniObject(env, optionObject, height);
    option.SetContentSize(width, height);
}

void AniPiPWindow::OptionSetPipTemplate(ani_env* env, ani_object optionObject, PipOptionAni& option)
{
    ani_ref templateTypeValue;
    env->Object_GetPropertyByName_Ref(optionObject, "templateType", &templateTypeValue);
    ani_boolean isTemplateTypeValueUndefined = false;
    env->Reference_IsUndefined(templateTypeValue, &isTemplateTypeValueUndefined);
    if (!templateTypeValue || isTemplateTypeValueUndefined) {
        return;
    }
    TLOGI(WmsLogTag::WMS_PIP, "[GetPiPOption]set templateType begin");
    ani_int pipTemplateValue;
    if (env->EnumItem_GetValue_Int(static_cast<ani_enum_item>(templateTypeValue), &pipTemplateValue) != ANI_OK) {
        TLOGI(WmsLogTag::WMS_PIP, "pipTemplateValue failed");
        return;
    }
    uint32_t templateType = static_cast<uint32_t>(pipTemplateValue);
    option.SetPipTemplate(templateType);
}

void AniPiPWindow::OptionSetControlGroup(ani_env* env, ani_object optionObject, PipOptionAni& option)
{
    ani_ref controlGroup = nullptr;
    ani_status ret = env->Object_GetPropertyByName_Ref(optionObject, "controlGroups", &controlGroup);
    if (ret != ANI_OK) {
        return;
    }
    ani_boolean iscontrolGroupUndefined = false;
    env->Reference_IsUndefined(controlGroup, &iscontrolGroupUndefined);
    std::vector<std::uint32_t> controls {};
    if (iscontrolGroupUndefined && option.GetPipTemplate() == static_cast<uint32_t>(PiPTemplateType::VIDEO_LIVE)) {
        TLOGI(WmsLogTag::WMS_PIP, "controls is undefined and templateType is VIDEO_LIVE, use VIDEO_PLAY_PAUSE");
        controls.push_back(static_cast<uint32_t>(PiPControlGroup::VIDEO_PLAY_PAUSE));
        option.SetControlGroup(controls);
        return;
    }
    if (!controlGroup || iscontrolGroupUndefined) {
        return;
    }
    TLOGI(WmsLogTag::WMS_PIP, "[GetPiPOption]set controlGroup begin");
    GetControlGroupFromJs(env, controlGroup, controls);
    option.SetControlGroup(controls);
}

void AniPiPWindow::OptionSetCustomUIController(ani_env* env, ani_object optionObject, PipOptionAni& option)
{
    ani_ref nodeController = nullptr;
    ani_status result = env->Object_GetPropertyByName_Ref(optionObject, "customUIController", &nodeController);
    ani_boolean isNodeControllerUndefined = false;
    env->Reference_IsUndefined(nodeController, &isNodeControllerUndefined);
    if (isNodeControllerUndefined || result != ANI_OK) {
        return;
    }
    TLOGI(WmsLogTag::WMS_PIP, "[GetPiPOption]set customUIController begin");
    ani_ref nodeControllerRef = nullptr;
    env->GlobalReference_Create(nodeController, &nodeControllerRef);
    option.SetNodeControllerRef(nodeControllerRef);
}

void AniPiPWindow::OptionSetDefaultWindowSizeType(ani_env* env, ani_object optionObject, PipOptionAni& option)
{
    ani_status rets;
    ani_ref defaultWindowSizeTypeRef;
    rets = env->Object_GetPropertyByName_Ref(optionObject, "defaultWindowSizeType", &defaultWindowSizeTypeRef);
    ani_boolean isDefaultWindowSizeTypeRefUndefined = false;
    env->Reference_IsUndefined(defaultWindowSizeTypeRef, &isDefaultWindowSizeTypeRefUndefined);
    if (rets != ANI_OK || isDefaultWindowSizeTypeRefUndefined) {
        return;
    }
    ani_int defaultWindowSizeTypeValue;
    rets = env->Object_CallMethodByName_Int(static_cast<ani_object>(defaultWindowSizeTypeRef),
        "toInt", ":i", &defaultWindowSizeTypeValue);
    if (rets != ANI_OK) {
        TLOGI(WmsLogTag::WMS_PIP, "defaultWindowSizeType get fail %{public}u", rets);
        return;
    }
    TLOGI(WmsLogTag::WMS_PIP, "[GetPiPOption]set defaultWindowSizeType begin");
    uint32_t defaultWindowSizeType = static_cast<uint32_t>(defaultWindowSizeTypeValue);
    option.SetDefaultWindowSizeType(defaultWindowSizeType);
}

void AniPiPWindow::OptionSetStorage(ani_env* env, ani_object optionObject, PipOptionAni& option)
{
    ani_ref storageValue;
    ani_status result = env->Object_GetPropertyByName_Ref(optionObject, "localStorage", &storageValue);
    ani_boolean isStorageValueUndefined = false;
    env->Reference_IsUndefined(storageValue, &isStorageValueUndefined);
    if (isStorageValueUndefined || result != ANI_OK) {
        return;
    }
    TLOGI(WmsLogTag::WMS_PIP, "[GetPiPOption]set storageRef begin");
    ani_ref storageRef;
    env->GlobalReference_Create(storageValue, &storageRef);
    option.SetStorageRef(storageRef);
}

void AniPiPWindow::OptionSetHandleId(ani_env* env, ani_object optionObject, PipOptionAni& option)
{
    ani_status rets;
    ani_ref handleIdRef;
    rets = env->Object_GetPropertyByName_Ref(optionObject, "handleId", &handleIdRef);
    ani_boolean isHandleIdRefUndefined = false;
    env->Reference_IsUndefined(handleIdRef, &isHandleIdRefUndefined);
    if (rets != ANI_OK || isHandleIdRefUndefined) {
        return;
    }
    ani_int handleIdValue;
    rets = env->Object_CallMethodByName_Int(static_cast<ani_object>(handleIdRef), "toInt", ":i", &handleIdValue);
    if (rets != ANI_OK) {
        TLOGE(WmsLogTag::WMS_PIP, "handleIdValue get fail %{public}u", rets);
        return;
    }
    TLOGI(WmsLogTag::WMS_PIP, "[GetPiPOption]set handleId begin");
    int32_t handleId = static_cast<int32_t>(handleIdValue);
    option.SetHandleId(handleId);
}

void AniPiPWindow::OptionSetCornerAdsorptionEnabled(ani_env* env, ani_object optionObject, PipOptionAni& option)
{
    ani_status rets;
    ani_ref CornerAdsorptionEnabledRef;
    rets = env->Object_GetPropertyByName_Ref(optionObject, "cornerAdsorptionEnabled", &CornerAdsorptionEnabledRef);
    ani_boolean isCornerAdsorptionEnabledRefUndefined = false;
    env->Reference_IsUndefined(CornerAdsorptionEnabledRef, &isCornerAdsorptionEnabledRefUndefined);
    if (rets != ANI_OK || isCornerAdsorptionEnabledRefUndefined) {
        TLOGI(WmsLogTag::WMS_PIP, "CornerAdsorptionEnabledRef get fail %{public}u", rets);
        return;
    }
    ani_boolean CornerAdsorptionEnabled;
    rets = env->Object_CallMethodByName_Boolean(static_cast<ani_object>(CornerAdsorptionEnabledRef),
                                                "toBoolean", ":z", &CornerAdsorptionEnabled);
    if (rets != ANI_OK) {
        TLOGE(WmsLogTag::WMS_PIP, "CornerAdsorptionEnabled get fail %{public}u", rets);
        return;
    }
    TLOGI(WmsLogTag::WMS_PIP, "[GetPiPOption]set CornerAdsorptionEnabled begin");
    option.SetCornerAdsorptionEnabled(static_cast<bool>(CornerAdsorptionEnabled));
}

bool AniPiPWindow::GetPiPOption(ani_env* env, ani_object optionObject, PipOptionAni& option)
{
    if (!OptionSetContext(env, optionObject, option) || !OptionSetXComponentController(env, optionObject, option)) {
        return false;
    }
    OptionSetNavigationIdValue(env, optionObject, option);
    OptionSetContentSize(env, optionObject, option);
    OptionSetPipTemplate(env, optionObject, option);
    OptionSetControlGroup(env, optionObject, option);
    OptionSetCustomUIController(env, optionObject, option);
    OptionSetDefaultWindowSizeType(env, optionObject, option);
    OptionSetStorage(env, optionObject, option);
    OptionSetHandleId(env, optionObject, option);
    OptionSetCornerAdsorptionEnabled(env, optionObject, option);
    return checkOptionParams(option);
}

ani_ref AniPiPWindow::CreatePiPController(ani_env* env, PipOptionAni pipOption)
{
    TLOGI(WmsLogTag::WMS_PIP, "start");
    if (!PictureInPictureManager::IsSupportPiP()) {
        return AniPipUtils::AniThrowError(env, WMError::WM_ERROR_DEVICE_NOT_SUPPORT, "device not support pip");
    }
    sptr<PipOptionAni> pipOptionPtr = new PipOptionAni(pipOption);
    auto context = static_cast<std::weak_ptr<AbilityRuntime::Context>*>(pipOptionPtr->GetContext());
    if (context == nullptr) {
        return AniPipUtils::AniThrowError(env, WMError::WM_ERROR_PIP_INTERNAL_ERROR, "Invalid context");
    }
    sptr<Window> mainWindow = Window::GetMainWindowWithContext(context->lock());
    if (mainWindow == nullptr) {
        return AniPipUtils::AniThrowError(env, WMError::WM_ERROR_PIP_INTERNAL_ERROR, "Invalid mainWindow");
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
        return AniPipUtils::AniThrowError(env, WMError::WM_ERROR_INVALID_PARAM,
            "Failed to convert object to pipConfiguration or pipConfiguration is null");
    }
    PipOptionAni pipOption;
    if (!GetPiPOption(env, configuration, pipOption)) {
        std::string errMsg = "nvalid parameters in config, please check if context/xComponentController is null,"
            "or controlGroup mismatch the corresponding pipTemplateType, or defaultWindowSizeType is invalid";
        TLOGE(WmsLogTag::WMS_PIP, "%{public}s", errMsg.c_str());
        return AniPipUtils::AniThrowError(env, WMError::WM_ERROR_INVALID_PARAM, errMsg);
    }
    ani_ref nativeTypeNodeRef = nullptr;
    ani_boolean isnativeXComponentUndefined = false;
    env->Reference_IsUndefined(nativeXComponent, &isnativeXComponentUndefined);
    if (!nativeXComponent || isnativeXComponentUndefined) {
        TLOGW(WmsLogTag::WMS_PIP, "nativeXComponent is nullptr or undefined");
        pipOption.SetTypeNodeEnabled(false);
        pipOption.SetTypeNodeRef(nullptr);
        return AniPiPWindow::CreatePiPController(env, pipOption);
    }
    TLOGI(WmsLogTag::WMS_PIP, "nativeXComponent enabled");
    pipOption.SetTypeNodeEnabled(true);
    env->GlobalReference_Create(nativeXComponent, &nativeTypeNodeRef);
    pipOption.SetTypeNodeRef(nativeTypeNodeRef);
    return AniPiPWindow::CreatePiPController(env, pipOption);
}

ani_boolean AniPiPWindow::IsPiPEnabledAni(ani_env* env)
{
    TLOGI(WmsLogTag::WMS_PIP, "start");
    bool isPiPEnabled = PictureInPictureManager::GetPipEnabled();
    TLOGI(WmsLogTag::WMS_PIP, "isPiPEnabled = %{public}u", isPiPEnabled);
    return static_cast<ani_boolean>(isPiPEnabled);
}

ani_object AniPiPWindow::NativeTransferStatic(ani_env* aniEnv, ani_class cls, ani_object input)
{
    TLOGI(WmsLogTag::WMS_PIP, "start");
    void *unwrapResult = nullptr;
    if (!arkts_esvalue_unwrap(aniEnv, input, &unwrapResult)) {
        TLOGE(WmsLogTag::WMS_PIP, "fail to unwrap input");
        AniPipUtils::AniThrowError(aniEnv, WMError::WM_ERROR_INVALID_PARAM);
        return AniPipUtils::AniGetUndefined(aniEnv);
    }
    if (unwrapResult == nullptr) {
        TLOGE(WmsLogTag::WMS_PIP, "unwrapResult is nullptr");
        AniPipUtils::AniThrowError(aniEnv, WMError::WM_ERROR_INVALID_PARAM);
        return AniPipUtils::AniGetUndefined(aniEnv);
    }
    JsPipController* jsPipController = static_cast<JsPipController*>(unwrapResult);
    if (jsPipController == nullptr) {
        TLOGE(WmsLogTag::WMS_PIP, "jsPipController is nullptr");
        AniPipUtils::AniThrowError(aniEnv, WMError::WM_ERROR_INVALID_PARAM);
        return AniPipUtils::AniGetUndefined(aniEnv);
    }

    // get PictureInPictureController
    sptr<PictureInPictureController> pipController = jsPipController->GetController();

    // get PipOption and transfer it to PipOptionAni
    sptr<PipOption> pipOption = pipController->GetPipOption();
    sptr<PipOptionAni> pipOptionAni = new PipOptionAni();
    if (!AniPipUtils::TransferToPipOptionAni(aniEnv, pipOption, pipOptionAni)) {
        TLOGE(WmsLogTag::WMS_PIP, "fail to transfer PipOption to PipOptionAni");
        AniPipUtils::AniThrowError(aniEnv, WMError::WM_ERROR_INVALID_PARAM);
        return AniPipUtils::AniGetUndefined(aniEnv);
    }

    // get PictureInPictureControllerAni
    sptr<Window> mainWindow = pipController->GetMainWindow();
    uint32_t mainWindowId = pipController->GetMainWindowId();
    sptr<PictureInPictureControllerAni> pipControllerAni =
            new PictureInPictureControllerAni(pipOptionAni, mainWindow, mainWindowId, aniEnv);

    // pipControllerAni get attributes from pipController
    AniPipUtils::TransferToPipControllerAni(pipController, pipControllerAni);

    // Create AniPiPControllerObject
    return CreateAniPiPControllerObject(aniEnv, pipControllerAni);
}

ani_object AniPiPWindow::NativeTransferDynamic(ani_env* aniEnv, ani_class cls, ani_long nativeObj)
{
    TLOGI(WmsLogTag::WMS_PIP, "start");
    AniPipController* aniPipController = reinterpret_cast<AniPipController*>(nativeObj);
    if (aniPipController == nullptr) {
        TLOGE(WmsLogTag::WMS_PIP, "AniPipController is nullptr");
        AniPipUtils::AniThrowError(aniEnv, WMError::WM_ERROR_INVALID_PARAM);
        return AniPipUtils::AniGetUndefined(aniEnv);
    }
    napi_env napiEnv {};
    if (!arkts_napi_scope_open(aniEnv, &napiEnv)) {
        TLOGE(WmsLogTag::WMS_PIP, "napi scope open fail");
        AniPipUtils::AniThrowError(aniEnv, WMError::WM_ERROR_INVALID_PARAM);
        return AniPipUtils::AniGetUndefined(aniEnv);
    }

    // PictureInPictureControllerAni
    sptr<PictureInPictureControllerAni> pipControllerAni = aniPipController->GetController();

    // get PipOptionAni and transfer it to PipOption
    sptr<PipOptionAni> pipOptionAni = pipControllerAni->GetPipOptionAni();
    sptr<PipOption> pipOption = new PipOption();
    if (!AniPipUtils::TransferToPipOptionNapi(aniEnv, pipOptionAni, pipOption)) {
        TLOGE(WmsLogTag::WMS_PIP, "fail to transfer PipOptionAni to PipOption");
        AniPipUtils::AniThrowError(aniEnv, WMError::WM_ERROR_INVALID_PARAM);
        return AniPipUtils::AniGetUndefined(aniEnv);
    }
 
    // get PictureInPictureController
    sptr<Window> mainWindow = pipControllerAni->GetMainWindow();
    uint32_t mainWindowId = pipControllerAni->GetMainWindowId();
    sptr<PictureInPictureController> pipController =
            new PictureInPictureController(pipOption, mainWindow, mainWindowId, napiEnv);

    // pipController get attributes from pipControllerAni
    AniPipUtils::TransferToPipControllerNapi(pipControllerAni, pipController);
    
    // Create jsPipController and return ESValue
    napi_value jsPipController = CreateJsPipControllerObject(napiEnv, pipController);
    
    // convert napi_value to AniObject
    return AniPipUtils::ConvertNapiValueToAniObject(aniEnv, napiEnv, jsPipController);
}

ani_status AniPiPWindow::BindNamespaceFunctions(ani_env* env, ani_namespace& nsp)
{
    // bind namespace.function
    ani_status ret = env->FindNamespace("@ohos.PiPWindow.PiPWindow", &nsp);
    if (ret != ANI_OK) {
        TLOGE(WmsLogTag::WMS_PIP, "FindNamespace failed,ani_status ret is %{public}u", ret);
        return ANI_NOT_FOUND;
    }

    std::array funcs = {
        ani_native_function{"isPiPEnabled", ":z", reinterpret_cast<void*>(AniPiPWindow::IsPiPEnabledAni)},
        ani_native_function{"createSync", nullptr, reinterpret_cast<void*>(AniPiPWindow::Create)},
        ani_native_function{"pipControllerFinalizerCallback", "l:", reinterpret_cast<void*>(AniPiPWindow::Finalizer)},
    };

    for (const auto& func : funcs) {
        ret = env->Namespace_BindNativeFunctions(nsp, &func, 1u);
        if (ret != ANI_OK) {
            TLOGE(WmsLogTag::WMS_PIP, "bind namespace fail %{public}u, %{public}s, %{public}s",
                ret, func.name, func.signature);
            return ANI_NOT_FOUND;
        }
    }
    return ANI_OK;
}

ani_status AniPiPWindow::BindClassMethods(ani_env* env)
{
    // bind namespace.cls.method
    ani_class cls = nullptr;
    ani_status ret = env->FindClass("@ohos.PiPWindow.PiPWindow.Transfer", &cls);
    if (ret != ANI_OK) {
        TLOGD(WmsLogTag::WMS_PIP, "null env %{public}u", ret);
        return ret;
    }

    std::array s_methods = {
        ani_native_function {"nativeTransferStatic", "C{std.interop.ESValue}:C{std.core.Object}",
            reinterpret_cast<void*>(AniPiPWindow::NativeTransferStatic)},
        ani_native_function {"nativeTransferDynamic", "l:C{std.interop.ESValue}",
            reinterpret_cast<void*>(AniPiPWindow::NativeTransferDynamic)},
    };

    for (const auto& method : s_methods) {
        ret = env->Class_BindStaticNativeMethods(cls, &method, 1u);
        if (ret != ANI_OK) {
            TLOGE(WmsLogTag::WMS_PIP, "[ANI] bind window static method fail %{public}u, %{public}s, %{public}s",
                ret, method.name, method.signature);
            return ANI_NOT_FOUND;
        }
    }
    return ANI_OK;
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

    // bind namespace.function
    ani_namespace nsp;
    ret = AniPiPWindow::BindNamespaceFunctions(env, nsp);
    if (ret != ANI_OK) {
        return ANI_NOT_FOUND;
    }

    // bind namespace.cls.method
    ret = AniPiPWindow::BindClassMethods(env);
    if (ret != ANI_OK) {
        return ret;
    }

    ANI_Controller_Constructor(vm, result);
    ANI_Manager_Constructor(vm, result);

    AniPiPWindow::Init(env, nsp);
    *result = ANI_VERSION_1;
    TLOGI(WmsLogTag::WMS_PIP, "finish");
    return ANI_OK;
}  // ANI_Constructor
}  // extern