/*
 * Copyright (c) 2023-2023 Huawei Device Co., Ltd.
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

#include "js_pip_window_manager.h"

#include "js_pip_controller.h"
#include "js_pip_utils.h"
#include "js_runtime_utils.h"
#include "window_manager_hilog.h"
#include "window.h"
#include "xcomponent_controller.h"
#include <algorithm>

namespace OHOS {
namespace Rosen {
using namespace AbilityRuntime;
using namespace Ace;
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

std::mutex JsPipWindowManager::mutex_;

static int32_t checkControlsRules(uint32_t pipTemplateType, std::vector<std::uint32_t> controlGroups)
{
    auto iter = TEMPLATE_CONTROL_MAP.find(static_cast<PiPTemplateType>(pipTemplateType));
    auto controls = iter->second;
    if (controlGroups.size() > MAX_CONTROL_GROUP_NUM) {
        return -1;
    }
    for (auto control : controlGroups) {
        if (controls.find(static_cast<PiPControlGroup>(control)) == controls.end()) {
            TLOGE(WmsLogTag::WMS_PIP, "pipOption param error, controlGroup not matches, controlGroup: %{public}u",
                control);
            return -1;
        }
    }
    if (pipTemplateType == static_cast<uint32_t>(PiPTemplateType::VIDEO_PLAY)) {
        auto iterFirst = std::find(controlGroups.begin(), controlGroups.end(),
            static_cast<uint32_t>(PiPControlGroup::VIDEO_PREVIOUS_NEXT));
        auto iterSecond = std::find(controlGroups.begin(), controlGroups.end(),
            static_cast<uint32_t>(PiPControlGroup::FAST_FORWARD_BACKWARD));
        if (iterFirst != controlGroups.end() && iterSecond != controlGroups.end()) {
            TLOGE(WmsLogTag::WMS_PIP, "pipOption param error, %{public}u conflicts with %{public}u in controlGroups",
                static_cast<uint32_t>(PiPControlGroup::VIDEO_PREVIOUS_NEXT),
                static_cast<uint32_t>(PiPControlGroup::FAST_FORWARD_BACKWARD));
            return -1;
        }
    }
    return 0;
}

static int32_t checkOptionParams(PipOption& option)
{
    if (option.GetContext() == nullptr) {
        TLOGE(WmsLogTag::WMS_PIP, "pipOption param error, context is nullptr.");
        return -1;
    }
    if (option.GetXComponentController() == nullptr) {
        TLOGE(WmsLogTag::WMS_PIP, "pipOption param error, XComponentController is nullptr.");
        return -1;
    }
    uint32_t pipTemplateType = option.GetPipTemplate();
    if (TEMPLATE_CONTROL_MAP.find(static_cast<PiPTemplateType>(pipTemplateType)) ==
        TEMPLATE_CONTROL_MAP.end()) {
        TLOGE(WmsLogTag::WMS_PIP, "pipOption param error, pipTemplateType not exists.");
        return -1;
    }
    return checkControlsRules(pipTemplateType, option.GetControlGroup());
}

static bool GetControlGroupFromJs(napi_env env, napi_value controlGroup, std::vector<std::uint32_t>& controls,
    uint32_t templateType)
{
    if (controlGroup == nullptr) {
        return false;
    }
    napi_valuetype type;
    napi_typeof(env, controlGroup, &type);
    if (type == napi_undefined && templateType == static_cast<uint32_t>(PiPTemplateType::VIDEO_LIVE)) {
        TLOGI(WmsLogTag::WMS_PIP, "controls is undefined");
        controls.push_back(static_cast<uint32_t>(PiPControlGroup::VIDEO_PLAY_PAUSE));
    }
    uint32_t size = 0;
    napi_get_array_length(env, controlGroup, &size);
    for (uint32_t i = 0; i < size; i++) {
        uint32_t controlType;
        napi_value getElementValue = nullptr;
        napi_get_element(env, controlGroup, i, &getElementValue);
        if (!ConvertFromJsValue(env, getElementValue, controlType)) {
            TLOGE(WmsLogTag::WMS_PIP, "Failed to convert parameter to controlType");
            continue;
        }
        auto iter = std::find(controls.begin(), controls.end(), controlType);
        if (iter != controls.end()) {
            TLOGI(WmsLogTag::WMS_PIP, "The controlType already exists. controlType: %{public}u", controlType);
        } else {
            controls.push_back(controlType);
        }
    }
    return true;
}

static int32_t GetPictureInPictureOptionFromJs(napi_env env, napi_value optionObject, PipOption& option)
{
    napi_value contextPtrValue = nullptr;
    napi_value navigationIdValue = nullptr;
    napi_value templateTypeValue = nullptr;
    napi_value widthValue = nullptr;
    napi_value heightValue = nullptr;
    napi_value xComponentControllerValue = nullptr;
    napi_value controlGroup = nullptr;
    napi_value nodeController = nullptr;
    napi_ref nodeControllerRef = nullptr;
    void* contextPtr = nullptr;
    std::string navigationId = "";
    uint32_t templateType = static_cast<uint32_t>(PiPTemplateType::VIDEO_PLAY);
    uint32_t width = 0;
    uint32_t height = 0;
    std::vector<std::uint32_t> controls;

    napi_get_named_property(env, optionObject, "context", &contextPtrValue);
    napi_get_named_property(env, optionObject, "navigationId", &navigationIdValue);
    napi_get_named_property(env, optionObject, "templateType", &templateTypeValue);
    napi_get_named_property(env, optionObject, "contentWidth", &widthValue);
    napi_get_named_property(env, optionObject, "contentHeight", &heightValue);
    napi_get_named_property(env, optionObject, "componentController", &xComponentControllerValue);
    napi_get_named_property(env, optionObject, "controlGroups", &controlGroup);
    napi_get_named_property(env, optionObject, "customUIController", &nodeController);
    napi_create_reference(env, nodeController, 1, &nodeControllerRef);
    napi_unwrap(env, contextPtrValue, &contextPtr);
    ConvertFromJsValue(env, navigationIdValue, navigationId);
    ConvertFromJsValue(env, templateTypeValue, templateType);
    ConvertFromJsValue(env, widthValue, width);
    ConvertFromJsValue(env, heightValue, height);
    GetControlGroupFromJs(env, controlGroup, controls, templateType);
    std::shared_ptr<XComponentController> xComponentControllerResult =
        XComponentController::GetXComponentControllerFromNapiValue(env, xComponentControllerValue);
    option.SetContext(contextPtr);
    option.SetNavigationId(navigationId);
    option.SetPipTemplate(templateType);
    option.SetContentSize(width, height);
    option.SetControlGroup(controls);
    option.SetXComponentController(xComponentControllerResult);
    option.SetNodeControllerRef(nodeControllerRef);
    return checkOptionParams(option);
}

JsPipWindowManager::JsPipWindowManager()
{
}

JsPipWindowManager::~JsPipWindowManager()
{
}

void JsPipWindowManager::Finalizer(napi_env env, void* data, void* hint)
{
    TLOGD(WmsLogTag::WMS_PIP, "Finalizer");
    std::unique_ptr<JsPipWindowManager>(static_cast<JsPipWindowManager*>(data));
}

napi_value JsPipWindowManager::IsPipEnabled(napi_env env, napi_callback_info info)
{
    JsPipWindowManager* me = CheckParamsAndGetThis<JsPipWindowManager>(env, info);
    return (me != nullptr) ? me->OnIsPipEnabled(env, info) : nullptr;
}

napi_value JsPipWindowManager::OnIsPipEnabled(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::WMS_PIP, "OnIsPipEnabled called");
    return CreateJsValue(env, true);
}

napi_value JsPipWindowManager::CreatePipController(napi_env env, napi_callback_info info)
{
    JsPipWindowManager* me = CheckParamsAndGetThis<JsPipWindowManager>(env, info);
    return (me != nullptr) ? me->OnCreatePipController(env, info) : nullptr;
}

std::unique_ptr<NapiAsyncTask> JsPipWindowManager::CreateEmptyAsyncTask(
    napi_env env, napi_value lastParam, napi_value* result)
{
    napi_valuetype type = napi_undefined;
    napi_typeof(env, lastParam, &type);
    if (lastParam == nullptr || type != napi_function) {
        napi_deferred nativeDeferred = nullptr;
        napi_create_promise(env, &nativeDeferred, result);
        return std::make_unique<NapiAsyncTask>(nativeDeferred, std::unique_ptr<NapiAsyncTask::ExecuteCallback>(),
            std::unique_ptr<NapiAsyncTask::CompleteCallback>());
    } else {
        napi_get_undefined(env, result);
        napi_ref callbackRef = nullptr;
        napi_create_reference(env, lastParam, 1, &callbackRef);
        return std::make_unique<NapiAsyncTask>(callbackRef, std::unique_ptr<NapiAsyncTask::ExecuteCallback>(),
            std::unique_ptr<NapiAsyncTask::CompleteCallback>());
    }
}

napi_value JsPipWindowManager::NapiSendTask(napi_env env, PipOption& pipOption)
{
    napi_value result = nullptr;
    std::unique_ptr<NapiAsyncTask> napiAsyncTask = CreateEmptyAsyncTask(env, nullptr, &result);
    auto asyncTask = [this, env, task = napiAsyncTask.get(), pipOption]() {
        if (!PictureInPictureManager::IsSupportPiP()) {
            task->Reject(env, CreateJsError(env, static_cast<int32_t>(
                WMError::WM_ERROR_DEVICE_NOT_SUPPORT), "device not support pip."));
            return;
        }
        sptr<PipOption> pipOptionPtr = new PipOption(pipOption);
        auto context = static_cast<std::weak_ptr<AbilityRuntime::Context>*>(pipOptionPtr->GetContext());
        if (context == nullptr) {
            task->Reject(env, CreateJsError(env, static_cast<int32_t>(
                WMError::WM_ERROR_PIP_INTERNAL_ERROR), "Invalid context"));
            return;
        }
        sptr<Window> mainWindow = Window::GetMainWindowWithContext(context->lock());
        if (mainWindow == nullptr) {
            task->Reject(env, CreateJsError(env, static_cast<int32_t>(
                WMError::WM_ERROR_PIP_INTERNAL_ERROR), "Invalid mainWindow"));
            return;
        }
        sptr<PictureInPictureController> pipController =
            new PictureInPictureController(pipOptionPtr, mainWindow, mainWindow->GetWindowId(), env);
        task->Resolve(env, CreateJsPipControllerObject(env, pipController));
        delete task;
    };
    if (napi_status::napi_ok != napi_send_event(env, asyncTask, napi_eprio_immediate)) {
        napiAsyncTask->Reject(env, CreateJsError(env,
            static_cast<int32_t>(WMError::WM_ERROR_PIP_INTERNAL_ERROR), "Send event failed"));
    } else {
        napiAsyncTask.release();
    }
    return result;
}

napi_value JsPipWindowManager::OnCreatePipController(napi_env env, napi_callback_info info)
{
    TLOGI(WmsLogTag::WMS_PIP, "[NAPI]");
    std::lock_guard<std::mutex> lock(mutex_);
    size_t argc = 4;
    napi_value argv[4] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc < 1) {
        return NapiThrowInvalidParam(env, "Missing args when creating pipController");
    }
    napi_value config = argv[0];
    if (config == nullptr) {
        TLOGE(WmsLogTag::WMS_PIP, "config is null");
        return NapiThrowInvalidParam(env, "Failed to convert object to pipConfiguration or pipConfiguration is null");
    }
    PipOption pipOption;
    if (GetPictureInPictureOptionFromJs(env, config, pipOption) == -1) {
        std::string errMsg = "Invalid parameters in config, please check if context/xComponentController is null,"
            " or controlGroup mismatch the corresponding pipTemplateType";
        TLOGE(WmsLogTag::WMS_PIP, "%{public}s", errMsg.c_str());
        return NapiThrowInvalidParam(env, errMsg);
    }
    if (argc > 1) {
        napi_value typeNode = argv[1];
        napi_ref typeNodeRef = nullptr;
        if (typeNode != nullptr && GetType(env, typeNode) != napi_undefined) {
            TLOGI(WmsLogTag::WMS_PIP, "typeNode enabled");
            pipOption.SetTypeNodeEnabled(true);
            napi_create_reference(env, typeNode, 1, &typeNodeRef);
            pipOption.SetTypeNodeRef(typeNodeRef);
        } else {
            pipOption.SetTypeNodeEnabled(false);
            pipOption.SetTypeNodeRef(nullptr);
        }
    }
    return NapiSendTask(env, pipOption);
}

napi_value JsPipWindowManagerInit(napi_env env, napi_value exportObj)
{
    TLOGD(WmsLogTag::WMS_PIP, "JsPipWindowManagerInit");
    if (env == nullptr || exportObj == nullptr) {
        TLOGE(WmsLogTag::WMS_PIP, "JsPipWindowManagerInit env or exportObj is nullptr");
        return nullptr;
    }
    std::unique_ptr<JsPipWindowManager> jsPipManager = std::make_unique<JsPipWindowManager>();
    napi_wrap(env, exportObj, jsPipManager.release(), JsPipWindowManager::Finalizer, nullptr, nullptr);
    const char* moduleName = "JsPipWindowManager";
    BindNativeFunction(env, exportObj, "create", moduleName, JsPipWindowManager::CreatePipController);
    BindNativeFunction(env, exportObj, "isPiPEnabled", moduleName, JsPipWindowManager::IsPipEnabled);
    InitEnums(env, exportObj);
    return NapiGetUndefined(env);
}
}
}