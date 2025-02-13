/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#include "js_scene_utils.h"

#include <iomanip>

#include <event_handler.h>
#include <js_runtime_utils.h>
#include <napi_common_want.h>

#include "property/rs_properties_def.h"
#include "root_scene.h"
#include "window_manager_hilog.h"
#include "window_visibility_info.h"
#include "process_options.h"

namespace OHOS::Rosen {
using namespace AbilityRuntime;
namespace {
constexpr HiviewDFX::HiLogLabel LABEL = { LOG_CORE, HILOG_DOMAIN_WINDOW, "JsSceneUtils" };
constexpr int32_t US_PER_NS = 1000;
constexpr int32_t INVALID_VAL = -9999;

// Refer to OHOS::Ace::TouchType
enum class AceTouchType : int32_t {
    DOWN = 0,
    UP,
    MOVE,
    CANCEL,
};

int32_t GetMMITouchType(int32_t aceType)
{
    switch (aceType) {
        case static_cast<int32_t>(AceTouchType::DOWN):
            return MMI::PointerEvent::POINTER_ACTION_DOWN;
        case static_cast<int32_t>(AceTouchType::UP):
            return MMI::PointerEvent::POINTER_ACTION_UP;
        case static_cast<int32_t>(AceTouchType::MOVE):
            return MMI::PointerEvent::POINTER_ACTION_MOVE;
        case static_cast<int32_t>(AceTouchType::CANCEL):
            return MMI::PointerEvent::POINTER_ACTION_CANCEL;
        default:
            return MMI::PointerEvent::POINTER_ACTION_UNKNOWN;
    }
}
} // namespace

napi_value NapiGetUndefined(napi_env env)
{
    napi_value result = nullptr;
    napi_get_undefined(env, &result);
    return result;
}

napi_valuetype GetType(napi_env env, napi_value value)
{
    napi_valuetype res = napi_undefined;
    napi_typeof(env, value, &res);
    return res;
}

WSError GetIntValueFromString(const std::string& str, uint32_t& value)
{
    char* end;
    value = strtoul(str.c_str(), &end, 10); // 10 number convert base
    if (*end == '\0' && value != 0) {
        return WSError::WS_OK;
    }
    TLOGE(WmsLogTag::DEFAULT, "param %{public}s convert int failed", str.c_str());
    return WSError::WS_ERROR_INVALID_PARAM;
}

bool IsJsBundleNameUndefind(napi_env env, napi_value jsBundleName, SessionInfo& sessionInfo)
{
    if (GetType(env, jsBundleName) != napi_undefined) {
        std::string bundleName;
        if (!ConvertFromJsValue(env, jsBundleName, bundleName)) {
            WLOGFE("[NAPI]Failed to convert parameter to bundleName");
            return false;
        }
        sessionInfo.bundleName_ = bundleName;
    }
    return true;
}

bool IsJsModuleNameUndefind(napi_env env, napi_value jsModuleName, SessionInfo& sessionInfo)
{
    if (GetType(env, jsModuleName) != napi_undefined) {
        std::string moduleName;
        if (!ConvertFromJsValue(env, jsModuleName, moduleName)) {
            WLOGFE("[NAPI]Failed to convert parameter to moduleName");
            return false;
        }
        sessionInfo.moduleName_ = moduleName;
    }
    return true;
}

bool IsJsAbilityUndefind(napi_env env, napi_value jsAbilityName, SessionInfo& sessionInfo)
{
    if (GetType(env, jsAbilityName) != napi_undefined) {
        std::string abilityName;
        if (!ConvertFromJsValue(env, jsAbilityName, abilityName)) {
            WLOGFE("[NAPI]Failed to convert parameter to abilityName");
            return false;
        }
        sessionInfo.abilityName_ = abilityName;
    }
    return true;
}

bool IsJsAppIndexUndefind(napi_env env, napi_value jsAppIndex, SessionInfo& sessionInfo)
{
    if (GetType(env, jsAppIndex) != napi_undefined) {
        int32_t appIndex;
        if (!ConvertFromJsValue(env, jsAppIndex, appIndex)) {
            WLOGFE("[NAPI]Failed to convert parameter to appIndex");
            return false;
        }
        sessionInfo.appIndex_ = appIndex;
    }
    return true;
}

bool IsJsIsSystemUndefind(napi_env env, napi_value jsIsSystem, SessionInfo& sessionInfo)
{
    if (GetType(env, jsIsSystem) != napi_undefined) {
        bool isSystem;
        if (!ConvertFromJsValue(env, jsIsSystem, isSystem)) {
            WLOGFE("[NAPI]Failed to convert parameter to isSystem");
            return false;
        }
        sessionInfo.isSystem_ = isSystem;
    }
    return true;
}

bool IsJsSceneTypeUndefined(napi_env env, napi_value jsSceneType, SessionInfo& sessionInfo)
{
    if (GetType(env, jsSceneType) != napi_undefined) {
        uint32_t sceneType;
        if (!ConvertFromJsValue(env, jsSceneType, sceneType)) {
            WLOGFE("[NAPI]Failed to convert parameter to sceneType");
            return false;
        }
        sessionInfo.sceneType_ = static_cast<SceneType>(sceneType);
    } else if (sessionInfo.isSystem_) {
        sessionInfo.sceneType_ = SceneType::SYSTEM_WINDOW_SCENE;
    }
    return true;
}

bool IsJsPersistentIdUndefind(napi_env env, napi_value jsPersistentId, SessionInfo& sessionInfo)
{
    if (GetType(env, jsPersistentId) != napi_undefined) {
        int32_t persistentId;
        if (!ConvertFromJsValue(env, jsPersistentId, persistentId)) {
            WLOGFE("[NAPI]Failed to convert parameter to persistentId");
            return false;
        }
        sessionInfo.persistentId_ = persistentId;
    }
    return true;
}

bool IsJsCallStateUndefind(napi_env env, napi_value jsCallState, SessionInfo& sessionInfo)
{
    if (GetType(env, jsCallState) != napi_undefined) {
        int32_t callState;
        if (!ConvertFromJsValue(env, jsCallState, callState)) {
            WLOGFE("[NAPI]Failed to convert parameter to callState");
            return false;
        }
        sessionInfo.callState_ = static_cast<uint32_t>(callState);
    }
    return true;
}

bool IsJsWindowInputTypeUndefind(napi_env env, napi_value jsWindowInputType, SessionInfo& sessionInfo)
{
    if (GetType(env, jsWindowInputType) != napi_undefined) {
        uint32_t windowInputType = 0;
        if (!ConvertFromJsValue(env, jsWindowInputType, windowInputType)) {
            WLOGFE("[NAPI]Failed to convert parameter to windowInputType");
            return false;
        }
        sessionInfo.windowInputType_ = static_cast<uint32_t>(windowInputType);
    }
    return true;
}

bool IsJsSessionTypeUndefind(napi_env env, napi_value jsSessionType, SessionInfo& sessionInfo)
{
    uint32_t windowType = 0;
    if (GetType(env, jsSessionType) != napi_undefined) {
        if (!ConvertFromJsValue(env, jsSessionType, windowType)) {
            WLOGFE("[NAPI]Failed to convert parameter to windowType");
            return false;
        }
        if (JS_SESSION_TO_WINDOW_TYPE_MAP.count(static_cast<JsSessionType>(windowType)) != 0) {
            sessionInfo.windowType_ = static_cast<uint32_t>(
                JS_SESSION_TO_WINDOW_TYPE_MAP.at(static_cast<JsSessionType>(windowType)));
        }
    }
    if (windowType == 0 && sessionInfo.isSystem_) {
        sessionInfo.windowType_ = static_cast<uint32_t>(WindowType::WINDOW_TYPE_SCB_DEFAULT);
    }
    return true;
}

bool IsJsScreenIdUndefind(napi_env env, napi_value jsScreenId, SessionInfo& sessionInfo)
{
    if (GetType(env, jsScreenId) != napi_undefined) {
        int32_t screenId = -1;
        if (!ConvertFromJsValue(env, jsScreenId, screenId)) {
            WLOGFE("[NAPI]Failed to convert parameter to screenId");
            return false;
        }
        sessionInfo.screenId_ = static_cast<uint64_t>(screenId);
    }
    return true;
}

bool IsJsIsPersistentRecoverUndefined(napi_env env, napi_value jsIsPersistentRecover, SessionInfo& sessionInfo)
{
    if (GetType(env, jsIsPersistentRecover) != napi_undefined) {
        bool isPersistentRecover = false;
        if (!ConvertFromJsValue(env, jsIsPersistentRecover, isPersistentRecover)) {
            WLOGFE("[NAPI]Failed to convert parameter to isPersistentRecover");
            return false;
        }
        sessionInfo.isPersistentRecover_ = isPersistentRecover;
    }
    return true;
}

bool IsJsIsRotatableUndefined(napi_env env, napi_value jsIsRotatable, SessionInfo& sessionInfo)
{
    if (GetType(env, jsIsRotatable) != napi_undefined) {
        bool isRotable = false;
        if (!ConvertFromJsValue(env, jsIsRotatable, isRotable)) {
            WLOGFE("[NAPI]Failed to convert parameter to isRotable");
            return false;
        }
        sessionInfo.isRotable_ = isRotable;
    }
    return true;
}

bool IsJsProcessOptionUndefined(napi_env env, napi_value jsProcessOption, SessionInfo& sessionInfo)
{
    if (GetType(env, jsProcessOption) != napi_undefined) {
        std::shared_ptr<AAFwk::ProcessOptions> processOptions = std::make_shared<AAFwk::ProcessOptions>();
        if (!ConvertProcessOptionFromJs(env, jsProcessOption, processOptions)) {
            WLOGFE("[NAPI]Failed to convert parameter to processOptions");
            return false;
        }
        sessionInfo.processOptions = processOptions;
    }
    return true;
}

bool IsJsIsSetPointerAreasUndefined(napi_env env, napi_value jsIsSetPointerAreas, SessionInfo& sessionInfo)
{
    if (GetType(env, jsIsSetPointerAreas) != napi_undefined) {
        bool isSetPointerAreas = false;
        if (!ConvertFromJsValue(env, jsIsSetPointerAreas, isSetPointerAreas)) {
            WLOGFE("[NAPI]Failed to convert parameter to isSetPointerAreas");
            return false;
        }
        sessionInfo.isSetPointerAreas_ = isSetPointerAreas;
    }
    return true;
}

bool IsJsFullScreenStartUndefined(napi_env env, napi_value jsFullscreenStart, SessionInfo& sessionInfo)
{
    if (GetType(env, jsFullscreenStart) != napi_undefined) {
        bool fullScreenStart = false;
        if (!ConvertFromJsValue(env, jsFullscreenStart, fullScreenStart)) {
            TLOGI(WmsLogTag::WMS_LAYOUT, "Failed to convert parameter to fullScreenStart");
            return false;
        }
        sessionInfo.fullScreenStart_ = fullScreenStart;
    }
    return true;
}

static bool IsJsIsUseControlSessionUndefined(napi_env env, napi_value jsIsUseControlSession, SessionInfo& sessionInfo)
{
    if (GetType(env, jsIsUseControlSession) != napi_undefined) {
        bool isUseControlSession = false;
        if (!ConvertFromJsValue(env, jsIsUseControlSession, isUseControlSession)) {
            TLOGI(WmsLogTag::WMS_LIFE, "Failed to convert parameter to isUseControlSession");
            return false;
        }
        sessionInfo.isUseControlSession = isUseControlSession;
    }
    return true;
}

bool ConvertSessionInfoName(napi_env env, napi_value jsObject, SessionInfo& sessionInfo)
{
    napi_value jsBundleName = nullptr;
    napi_get_named_property(env, jsObject, "bundleName", &jsBundleName);
    napi_value jsModuleName = nullptr;
    napi_get_named_property(env, jsObject, "moduleName", &jsModuleName);
    napi_value jsAbilityName = nullptr;
    napi_get_named_property(env, jsObject, "abilityName", &jsAbilityName);
    napi_value jsAppIndex = nullptr;
    napi_get_named_property(env, jsObject, "appIndex", &jsAppIndex);
    napi_value jsIsSystem = nullptr;
    napi_get_named_property(env, jsObject, "isSystem", &jsIsSystem);
    napi_value jsSceneType = nullptr;
    napi_get_named_property(env, jsObject, "sceneType", &jsSceneType);
    napi_value jsWindowInputType = nullptr;
    napi_get_named_property(env, jsObject, "windowInputType", &jsWindowInputType);
    napi_value jsFullScreenStart = nullptr;
    napi_get_named_property(env, jsObject, "fullScreenStart", &jsFullScreenStart);
    if (!IsJsBundleNameUndefind(env, jsBundleName, sessionInfo)) {
        return false;
    }
    if (!IsJsModuleNameUndefind(env, jsModuleName, sessionInfo)) {
        return false;
    }
    if (!IsJsAbilityUndefind(env, jsAbilityName, sessionInfo)) {
        return false;
    }
    if (!IsJsAppIndexUndefind(env, jsAppIndex, sessionInfo)) {
        return false;
    }
    if (!IsJsIsSystemUndefind(env, jsIsSystem, sessionInfo)) {
        return false;
    }
    if (!IsJsSceneTypeUndefined(env, jsSceneType, sessionInfo)) {
        return false;
    }
    if (!IsJsWindowInputTypeUndefind(env, jsWindowInputType, sessionInfo)) {
        return false;
    }
    if (!IsJsFullScreenStartUndefined(env, jsFullScreenStart, sessionInfo)) {
        return false;
    }
    return true;
}

bool ConvertProcessOptionFromJs(napi_env env, napi_value jsObject,
    std::shared_ptr<AAFwk::ProcessOptions> processOptions)
{
    napi_value jsProcessMode = nullptr;
    napi_get_named_property(env, jsObject, "processMode", &jsProcessMode);
    napi_value jsStartupVisibility = nullptr;
    napi_get_named_property(env, jsObject, "startupVisibility", &jsStartupVisibility);

    int32_t processMode;
    if (!ConvertFromJsValue(env, jsProcessMode, processMode)) {
        WLOGFE("[NAPI]Failed to convert parameter to processMode");
        return false;
    }

    int32_t startupVisibility;
    if (!ConvertFromJsValue(env, jsStartupVisibility, startupVisibility)) {
        WLOGFE("[NAPI]Failed to convert parameter to startupVisibility");
        return false;
    }
    processOptions->processMode = static_cast<AAFwk::ProcessMode>(processMode);
    processOptions->startupVisibility = static_cast<AAFwk::StartupVisibility>(startupVisibility);

    return true;
}

bool ConvertSessionInfoState(napi_env env, napi_value jsObject, SessionInfo& sessionInfo)
{
    napi_value jsPersistentId = nullptr;
    napi_get_named_property(env, jsObject, "persistentId", &jsPersistentId);
    napi_value jsCallState = nullptr;
    napi_get_named_property(env, jsObject, "callState", &jsCallState);
    napi_value jsSessionType = nullptr;
    napi_get_named_property(env, jsObject, "sessionType", &jsSessionType);
    napi_value jsIsPersistentRecover = nullptr;
    napi_get_named_property(env, jsObject, "isPersistentRecover", &jsIsPersistentRecover);
    napi_value jsScreenId = nullptr;
    napi_get_named_property(env, jsObject, "screenId", &jsScreenId);
    napi_value jsIsRotable = nullptr;
    napi_get_named_property(env, jsObject, "isRotatable", &jsIsRotable);
    napi_value jsIsSetPointerAreas = nullptr;
    napi_get_named_property(env, jsObject, "isSetPointerAreas", &jsIsSetPointerAreas);
    napi_value jsProcessOption = nullptr;
    napi_get_named_property(env, jsObject, "processOptions", &jsProcessOption);
    napi_value jsIsUseControlSession = nullptr;
    napi_get_named_property(env, jsObject, "isAppUseControl", &jsIsUseControlSession);

    if (!IsJsPersistentIdUndefind(env, jsPersistentId, sessionInfo)) {
        return false;
    }
    if (!IsJsCallStateUndefind(env, jsCallState, sessionInfo)) {
        return false;
    }
    if (!IsJsSessionTypeUndefind(env, jsSessionType, sessionInfo)) {
        return false;
    }
    if (!IsJsScreenIdUndefind(env, jsScreenId, sessionInfo)) {
        return false;
    }
    if (!IsJsIsPersistentRecoverUndefined(env, jsIsPersistentRecover, sessionInfo)) {
        return false;
    }
    if (!IsJsIsRotatableUndefined(env, jsIsRotable, sessionInfo)) {
        return false;
    }
    if (!IsJsIsSetPointerAreasUndefined(env, jsIsSetPointerAreas, sessionInfo)) {
        return false;
    }
    if (!IsJsProcessOptionUndefined(env, jsProcessOption, sessionInfo)) {
        return false;
    }
    if (!IsJsIsUseControlSessionUndefined(env, jsIsUseControlSession, sessionInfo)) {
        return false;
    }
    return true;
}

napi_value CreateJsProcessOption(napi_env env, std::shared_ptr<AAFwk::ProcessOptions> processOptions)
{
    napi_value objValue = nullptr;
    napi_create_object(env, &objValue);
    if (objValue == nullptr) {
        WLOGFE("Failed to get object");
        return nullptr;
    }

    int32_t processMode = static_cast<int32_t>(processOptions->processMode);
    int32_t startupVisibility = static_cast<int32_t>(processOptions->startupVisibility);

    napi_set_named_property(env, objValue, "processMode", CreateJsValue(env, processMode));
    napi_set_named_property(env, objValue, "startupVisibility", CreateJsValue(env, startupVisibility));

    return objValue;
}

bool ConvertSessionInfoFromJs(napi_env env, napi_value jsObject, SessionInfo& sessionInfo)
{
    if (!ConvertSessionInfoName(env, jsObject, sessionInfo)) {
        return false;
    }
    if (!ConvertSessionInfoState(env, jsObject, sessionInfo)) {
        return false;
    }
    return true;
}

bool IsJsObjNameUndefind(napi_env env, napi_value jsObjName, int32_t& objName)
{
    if (GetType(env, jsObjName) != napi_undefined) {
        if (!ConvertFromJsValue(env, jsObjName, objName)) {
            WLOGFE("[NAPI]Failed to convert parameter to objName");
            return false;
        }
    }
    return true;
}

bool ConvertRectInfoFromJs(napi_env env, napi_value jsObject, WSRect& rect)
{
    napi_value jsLeftName = nullptr;
    napi_get_named_property(env, jsObject, "left", &jsLeftName);
    napi_value jsTopName = nullptr;
    napi_get_named_property(env, jsObject, "top", &jsTopName);
    napi_value jsRightName = nullptr;
    napi_get_named_property(env, jsObject, "right", &jsRightName);
    napi_value jsBottomName = nullptr;
    napi_get_named_property(env, jsObject, "bottom", &jsBottomName);

    int32_t leftName = 0;
    if (!IsJsObjNameUndefind(env, jsLeftName, leftName)) {
        return false;
    }
    rect.posX_ = leftName;

    int32_t topName = 0;
    if (!IsJsObjNameUndefind(env, jsTopName, topName)) {
        return false;
    }
    rect.posY_ = topName;

    int32_t rightName = 0;
    if (!IsJsObjNameUndefind(env, jsRightName, rightName)) {
        return false;
    }
    rect.width_ = rightName - rect.posX_;

    int32_t bottomName = 0;
    if (!IsJsObjNameUndefind(env, jsBottomName, bottomName)) {
        return false;
    }
    rect.height_ = bottomName - rect.posY_;
    return true;
}

bool ConvertHookInfoFromJs(napi_env env, napi_value jsObject, HookInfo& hookInfo)
{
    napi_value jsWidth = nullptr;
    napi_get_named_property(env, jsObject, "width", &jsWidth);
    napi_value jsHeight = nullptr;
    napi_get_named_property(env, jsObject, "height", &jsHeight);
    napi_value jsDensity = nullptr;
    napi_get_named_property(env, jsObject, "density", &jsDensity);
    napi_value jsRotation = nullptr;
    napi_get_named_property(env, jsObject, "rotation", &jsRotation);
    napi_value jsEnableHookRotation = nullptr;
    napi_get_named_property(env, jsObject, "enableHookRotation", &jsEnableHookRotation);

    uint32_t width = 0;
    if (!ConvertFromJsValue(env, jsWidth, width)) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "[NAPI]Failed to convert parameter to width");
        return false;
    }
    hookInfo.width_ = width;

    uint32_t height = 0;
    if (!ConvertFromJsValue(env, jsHeight, height)) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "[NAPI]Failed to convert parameter to height");
        return false;
    }
    hookInfo.height_ = height;

    double_t density = 1.0;
    if (!ConvertFromJsValue(env, jsDensity, density)) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "[NAPI]Failed to convert parameter to density");
        return false;
    }
    hookInfo.density_ = static_cast<float_t>(density);

    uint32_t rotation = 0;
    if (!ConvertFromJsValue(env, jsRotation, rotation)) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "[NAPI]Failed to convert parameter to rotation");
        return false;
    }
    hookInfo.rotation_ = rotation;

    bool enableHookRotation = false;
    if (!ConvertFromJsValue(env, jsEnableHookRotation, enableHookRotation)) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "[NAPI]Failed to convert parameter to enableHookRotation");
        return false;
    }
    hookInfo.enableHookRotation_ = enableHookRotation;
    return true;
}

bool ConvertPointerItemFromJs(napi_env env, napi_value touchObject, MMI::PointerEvent& pointerEvent)
{
    auto vpr = RootScene::staticRootScene_->GetDisplayDensity();
    MMI::PointerEvent::PointerItem pointerItem;
    napi_value jsId = nullptr;
    napi_get_named_property(env, touchObject, "id", &jsId);
    napi_value jsTouchType = nullptr;
    napi_get_named_property(env, touchObject, "type", &jsTouchType);
    napi_value jsWindowX = nullptr;
    napi_get_named_property(env, touchObject, "windowX", &jsWindowX);
    napi_value jsWindowY = nullptr;
    napi_get_named_property(env, touchObject, "windowY", &jsWindowY);
    napi_value jsDisplayX = nullptr;
    napi_get_named_property(env, touchObject, "displayX", &jsDisplayX);
    napi_value jsDisplayY = nullptr;
    napi_get_named_property(env, touchObject, "displayY", &jsDisplayY);
    int32_t id;
    if (!ConvertFromJsValue(env, jsId, id)) {
        WLOGFE("[NAPI]Failed to convert parameter to id");
        return false;
    }
    pointerItem.SetPointerId(id);
    pointerEvent.SetPointerId(id);
    int32_t touchType;
    if (!ConvertFromJsValue(env, jsTouchType, touchType)) {
        WLOGFE("[NAPI]Failed to convert parameter to touchType");
        return false;
    }
    pointerEvent.SetPointerAction(GetMMITouchType(touchType));
    double windowX;
    if (!ConvertFromJsValue(env, jsWindowX, windowX)) {
        WLOGFE("[NAPI]Failed to convert parameter to windowX");
        return false;
    }
    pointerItem.SetWindowX(std::round(windowX * vpr));
    double windowY;
    if (!ConvertFromJsValue(env, jsWindowY, windowY)) {
        WLOGFE("[NAPI]Failed to convert parameter to windowY");
        return false;
    }
    pointerItem.SetWindowY(std::round(windowY * vpr));
    double displayX;
    if (!ConvertFromJsValue(env, jsDisplayX, displayX)) {
        WLOGFE("[NAPI]Failed to convert parameter to displayX");
        return false;
    }
    pointerItem.SetDisplayX(std::round(displayX * vpr));
    double displayY;
    if (!ConvertFromJsValue(env, jsDisplayY, displayY)) {
        WLOGFE("[NAPI]Failed to convert parameter to displayY");
        return false;
    }
    pointerItem.SetDisplayY(std::round(displayY * vpr));
    pointerEvent.AddPointerItem(pointerItem);
    return true;
}

bool ConvertTouchesObjectFromJs(napi_env env, napi_value jsTouches, int32_t pointerId, MMI::PointerEvent& pointerEvent)
{
    // iterator touches
    if (jsTouches == nullptr) {
        WLOGFE("[NAPI]Failed to convert to touchesObject list");
        return false;
    }
    bool isArray = false;
    napi_is_array(env, jsTouches, &isArray);
    if (!isArray) {
        return false;
    }
    uint32_t length = 0;
    napi_get_array_length(env, jsTouches, &length);
    for (uint32_t i = 0; i < length; i++) {
        napi_value touchesObject = nullptr;
        napi_get_element(env, jsTouches, i, &touchesObject);
        if (touchesObject == nullptr) {
            WLOGFE("[NAPI]Failed get to touchesObject");
            return false;
        }
        napi_value jsNoChangedId = nullptr;
        napi_get_named_property(env, touchesObject, "id", &jsNoChangedId);
        int32_t noChangedId;
        if (!ConvertFromJsValue(env, jsNoChangedId, noChangedId)) {
            WLOGFE("[NAPI]Failed to convert parameter to jsNoChangeId");
            return false;
        }
        if (pointerId == noChangedId) {
            continue;
        }
        if (!ConvertPointerItemFromJs(env, touchesObject, pointerEvent)) {
            return false;
        }
    }
    return true;
}

bool ConvertPointerEventFromJs(napi_env env, napi_value jsObject, MMI::PointerEvent& pointerEvent)
{
    napi_value jsSourceType = nullptr;
    napi_get_named_property(env, jsObject, "source", &jsSourceType);
    napi_value jsTimestamp = nullptr;
    napi_get_named_property(env, jsObject, "timestamp", &jsTimestamp);
    napi_value jsChangedTouches = nullptr;
    napi_get_named_property(env, jsObject, "changedTouches", &jsChangedTouches);
    napi_value jsTouches = nullptr;
    napi_get_named_property(env, jsObject, "touches", &jsTouches);
    int32_t sourceType;
    if (!ConvertFromJsValue(env, jsSourceType, sourceType)) {
        WLOGFE("[NAPI]Failed to convert parameter to sourceType");
        return false;
    }
    pointerEvent.SetSourceType(MMI::PointerEvent::SOURCE_TYPE_TOUCHSCREEN);
    double timestamp;
    if (!ConvertFromJsValue(env, jsTimestamp, timestamp)) {
        WLOGFE("[NAPI]Failed to convert parameter to timestamp");
        return false;
    }
    pointerEvent.SetActionTime(std::round(timestamp / US_PER_NS));
    if (jsChangedTouches == nullptr) {
        WLOGFE("[NAPI]Failed to convert parameter to touchesArray");
        return false;
    }
    // use changedTouches[0] only
    napi_value touchObject = nullptr;
    napi_get_element(env, jsChangedTouches, 0, &touchObject);
    if (touchObject == nullptr) {
        WLOGFE("[NAPI]Failed get to touchObject");
        return false;
    }
    napi_value jsId = nullptr;
    napi_get_named_property(env, touchObject, "id", &jsId);
    int32_t pointerId;
    if (!ConvertFromJsValue(env, jsId, pointerId)) {
        WLOGFE("[NAPI]Failed to convert parameter to id");
        return false;
    }
    if (!ConvertPointerItemFromJs(env, touchObject, pointerEvent)) {
        return false;
    }
    if (!ConvertTouchesObjectFromJs(env, jsTouches, pointerId, pointerEvent)) {
        return false;
    }
    pointerEvent.SetPointerId(pointerId);
    if (!ConvertDeviceIdFromJs(env, jsObject, pointerEvent)) {
        return false;
    }
    return true;
}

bool ConvertDeviceIdFromJs(napi_env env, napi_value jsObject, MMI::PointerEvent& pointerEvent)
{
    napi_value jsDeviceId = nullptr;
    napi_get_named_property(env, jsObject, "deviceId", &jsDeviceId);
    int32_t deviceId = 0;
    if (!ConvertFromJsValue(env, jsDeviceId, deviceId)) {
        WLOGFE("[NAPI]Failed to convert parameter to deviceId");
        return false;
    }
    pointerEvent.SetDeviceId(deviceId);
    return true;
}

bool ConvertInt32ArrayFromJs(napi_env env, napi_value jsObject, std::vector<int32_t>& intList)
{
    bool isArray = false;
    napi_is_array(env, jsObject, &isArray);
    if (jsObject == nullptr || !isArray) {
        WLOGFE("[NAPI]Failed to convert to integer list");
        return false;
    }

    uint32_t length = 0;
    napi_get_array_length(env, jsObject, &length);
    for (uint32_t i = 0; i < length; i++) {
        int32_t persistentId;
        napi_value elementVal = nullptr;
        napi_get_element(env, jsObject, i, &elementVal);
        if (!ConvertFromJsValue(env, elementVal, persistentId)) {
            WLOGFE("[NAPI]Failed to convert to index %{public}u to integer", i);
            return false;
        }
        intList.push_back(persistentId);
    }

    return true;
}

bool ConvertStringMapFromJs(napi_env env, napi_value value, std::unordered_map<std::string, std::string>& stringMap)
{
    if (value == nullptr) {
        WLOGFE("value is nullptr");
        return false;
    }

    if (!CheckTypeForNapiValue(env, value, napi_object)) {
        WLOGFE("The type of value is not napi_object.");
        return false;
    }

    std::vector<std::string> propNames;
    napi_value array = nullptr;
    napi_get_property_names(env, value, &array);
    if (!ParseArrayStringValue(env, array, propNames)) {
        WLOGFE("Failed to property names");
        return false;
    }

    for (const auto &propName : propNames) {
        napi_value prop = nullptr;
        napi_get_named_property(env, value, propName.c_str(), &prop);
        if (prop == nullptr) {
            WLOGFW("prop is null: %{public}s", propName.c_str());
            continue;
        }
        if (!CheckTypeForNapiValue(env, prop, napi_string)) {
            WLOGFW("prop is not string: %{public}s", propName.c_str());
            continue;
        }
        std::string valName;
        if (!ConvertFromJsValue(env, prop, valName)) {
            WLOGFW("Failed to ConvertFromJsValue: %{public}s", propName.c_str());
            continue;
        }
        stringMap.emplace(propName, valName);
    }
    return true;
}

bool ConvertRotateAnimationConfigFromJs(napi_env env, napi_value value, RotateAnimationConfig& config)
{
    napi_value jsDuration = nullptr;
    napi_get_named_property(env, value, "duration", &jsDuration);
    if (GetType(env, jsDuration) != napi_undefined) {
        int32_t duration = ROTATE_ANIMATION_DURATION;
        if (!ConvertFromJsValue(env, jsDuration, duration)) {
            TLOGE(WmsLogTag::DEFAULT, "Failed to convert parameter to duration");
            return false;
        }
        config.duration_ = duration;
    }
    return true;
}

bool ConvertJsonFromJs(napi_env env, napi_value value, nlohmann::json& payload)
{
    if (value == nullptr || !CheckTypeForNapiValue(env, value, napi_object)) {
        WLOGFE("The type of value is not napi_object or is nullptr.");
        return false;
    }

    napi_value array = nullptr;
    napi_get_property_names(env, value, &array);
    std::vector<std::string> propNames;
    if (!ParseArrayStringValue(env, array, propNames)) {
        WLOGFE("Failed to property names");
        return false;
    }

    for (const auto& propName : propNames) {
        napi_value prop = nullptr;
        napi_get_named_property(env, value, propName.c_str(), &prop);
        if (prop == nullptr) {
            WLOGFW("prop is null: %{public}s", propName.c_str());
            continue;
        }
        if (!CheckTypeForNapiValue(env, prop, napi_string)) {
            WLOGFW("prop is not string: %{public}s", propName.c_str());
            continue;
        }
        std::string valName;
        if (!ConvertFromJsValue(env, prop, valName)) {
            WLOGFW("Failed to ConvertFromJsValue: %{public}s", propName.c_str());
            continue;
        }
        payload[propName] = std::move(valName);
    }
    return true;
}

bool ConvertDragResizeTypeFromJs(napi_env env, napi_value value, DragResizeType& dragResizeType)
{
    uint32_t dragResizeTypeValue;
    if (!ConvertFromJsValue(env, value, dragResizeTypeValue)) {
        return false;
    }
    if (dragResizeTypeValue > static_cast<uint32_t>(DragResizeType::RESIZE_WHEN_DRAG_END)) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "Failed to convert parameter to dragResizeType");
        return false;
    }
    dragResizeType = static_cast<DragResizeType>(dragResizeTypeValue);
    return true;
}

bool ParseArrayStringValue(napi_env env, napi_value array, std::vector<std::string> &vector)
{
    if (array == nullptr) {
        WLOGFE("array is nullptr!");
        return false;
    }
    bool isArray = false;
    if (napi_is_array(env, array, &isArray) != napi_ok || isArray == false) {
        WLOGFE("not array!");
        return false;
    }

    uint32_t arrayLen = 0;
    napi_get_array_length(env, array, &arrayLen);
    if (arrayLen == 0) {
        return true;
    }
    vector.reserve(arrayLen);
    for (uint32_t i = 0; i < arrayLen; i++) {
        std::string strItem;
        napi_value jsValue = nullptr;
        napi_get_element(env, array, i, &jsValue);
        if (!ConvertFromJsValue(env, jsValue, strItem)) {
            WLOGFW("Failed to ConvertFromJsValue, index: %{public}u", i);
            continue;
        }
        vector.emplace_back(std::move(strItem));
    }
    return true;
}

JsSessionType GetApiType(WindowType type)
{
    auto iter = WINDOW_TO_JS_SESSION_TYPE_MAP.find(type);
    if (iter == WINDOW_TO_JS_SESSION_TYPE_MAP.end()) {
        WLOGFE("[NAPI]window type: %{public}u cannot map to api type!", type);
        return JsSessionType::TYPE_UNDEFINED;
    } else {
        return iter->second;
    }
}

static napi_value CreateSupportWindowModes(napi_env env,
    const std::vector<AppExecFwk::SupportWindowMode>& supportWindowModes)
{
    napi_value arrayValue = nullptr;
    napi_create_array_with_length(env, supportWindowModes.size(), &arrayValue);
    if (arrayValue == nullptr) {
        TLOGE(WmsLogTag::WMS_LIFE, "Failed to create napi array");
        return NapiGetUndefined(env);
    }
    int32_t index = 0;
    for (const auto supportWindowMode : supportWindowModes) {
        napi_set_element(env, arrayValue, index++, CreateJsValue(env, static_cast<int32_t>(supportWindowMode)));
    }
    return arrayValue;
}

napi_value CreateJsSessionInfo(napi_env env, const SessionInfo& sessionInfo)
{
    napi_value objValue = nullptr;
    napi_create_object(env, &objValue);
    if (objValue == nullptr) {
        WLOGFE("[NAPI]Failed to get jsObject");
        return nullptr;
    }
    napi_set_named_property(env, objValue, "bundleName", CreateJsValue(env, sessionInfo.bundleName_));
    napi_set_named_property(env, objValue, "moduleName", CreateJsValue(env, sessionInfo.moduleName_));
    napi_set_named_property(env, objValue, "abilityName", CreateJsValue(env, sessionInfo.abilityName_));
    napi_set_named_property(env, objValue, "appIndex", CreateJsValue(env, sessionInfo.appIndex_));
    napi_set_named_property(env, objValue, "isSystem", CreateJsValue(env, sessionInfo.isSystem_));
    napi_set_named_property(env, objValue, "isCastSession", CreateJsValue(env, sessionInfo.isCastSession_));
    napi_set_named_property(env, objValue, "persistentId",
        CreateJsValue(env, static_cast<int32_t>(sessionInfo.persistentId_)));
    napi_set_named_property(env, objValue, "callerPersistentId",
        CreateJsValue(env, static_cast<int32_t>(sessionInfo.callerPersistentId_)));
    napi_set_named_property(env, objValue, "callerBundleName", CreateJsValue(env, sessionInfo.callerBundleName_));
    napi_set_named_property(env, objValue, "callerAbilityName", CreateJsValue(env, sessionInfo.callerAbilityName_));
    napi_set_named_property(env, objValue, "callState",
        CreateJsValue(env, static_cast<int32_t>(sessionInfo.callState_)));
    napi_set_named_property(env, objValue, "windowMode",
        CreateJsValue(env, static_cast<int32_t>(sessionInfo.windowMode)));
    napi_set_named_property(env, objValue, "screenId",
        CreateJsValue(env, static_cast<int32_t>(sessionInfo.screenId_)));
    napi_set_named_property(env, objValue, "sessionType",
        CreateJsValue(env, static_cast<uint32_t>(GetApiType(static_cast<WindowType>(sessionInfo.windowType_)))));
    napi_set_named_property(env, objValue, "sessionState",
        CreateJsValue(env, static_cast<int32_t>(sessionInfo.sessionState_)));
    napi_set_named_property(env, objValue, "requestOrientation",
        CreateJsValue(env, sessionInfo.requestOrientation_));
    napi_set_named_property(env, objValue, "isCalledRightlyByCallerId",
        CreateJsValue(env, sessionInfo.isCalledRightlyByCallerId_));
    napi_set_named_property(env, objValue, "isAtomicService",
        CreateJsValue(env, sessionInfo.isAtomicService_));
    napi_set_named_property(env, objValue, "isBackTransition",
        CreateJsValue(env, sessionInfo.isBackTransition_));
    napi_set_named_property(env, objValue, "needClearInNotShowRecent",
        CreateJsValue(env, sessionInfo.needClearInNotShowRecent_));
    if (sessionInfo.processOptions != nullptr) {
        napi_set_named_property(env, objValue, "processOptions",
            CreateJsProcessOption(env, sessionInfo.processOptions));
    }
    napi_set_named_property(env, objValue, "errorReason",
        CreateJsValue(env, sessionInfo.errorReason));
    napi_set_named_property(env, objValue, "isFromIcon", CreateJsValue(env, sessionInfo.isFromIcon_));
    SetJsSessionInfoByWant(env, sessionInfo, objValue);
    if (sessionInfo.want != nullptr) {
        napi_set_named_property(env, objValue, "want", AppExecFwk::WrapWant(env, *sessionInfo.want));
    }
    napi_set_named_property(env, objValue, "supportWindowModes",
        CreateSupportWindowModes(env, sessionInfo.supportWindowModes));
    return objValue;
}

napi_value CreateJsSessionRecoverInfo(
    napi_env env, const SessionInfo& sessionInfo, const sptr<WindowSessionProperty> property)
{
    napi_value objValue = nullptr;
    napi_create_object(env, &objValue);
    if (objValue == nullptr) {
        WLOGFE("[NAPI]Failed to get jsObject");
        return nullptr;
    }
    napi_set_named_property(env, objValue, "bundleName", CreateJsValue(env, sessionInfo.bundleName_));
    napi_set_named_property(env, objValue, "moduleName", CreateJsValue(env, sessionInfo.moduleName_));
    napi_set_named_property(env, objValue, "abilityName", CreateJsValue(env, sessionInfo.abilityName_));
    napi_set_named_property(env, objValue, "appIndex", CreateJsValue(env, sessionInfo.appIndex_));
    napi_set_named_property(env, objValue, "screenId",
        CreateJsValue(env, static_cast<int32_t>(sessionInfo.screenId_)));
    napi_set_named_property(env, objValue, "windowMode",
        CreateJsValue(env, static_cast<int32_t>(sessionInfo.windowMode)));
    napi_set_named_property(env, objValue, "sessionState",
        CreateJsValue(env, static_cast<int32_t>(sessionInfo.sessionState_)));
    napi_set_named_property(env, objValue, "sessionType",
        CreateJsValue(env, static_cast<uint32_t>(GetApiType(static_cast<WindowType>(sessionInfo.windowType_)))));
    napi_set_named_property(env, objValue, "requestOrientation",
        CreateJsValue(env, sessionInfo.requestOrientation_));
    Rect rect = property->GetWindowRect();
    WSRect wsRect = { rect.posX_, rect.posY_, rect.width_, rect.height_ };
    napi_set_named_property(env, objValue, "recoverRect", CreateJsSessionRect(env, wsRect));
    napi_set_named_property(env, objValue, "mainWindowTopmost", CreateJsValue(env, property->IsMainWindowTopmost()));
    return objValue;
}

void SetJsSessionInfoByWant(napi_env env, const SessionInfo& sessionInfo, napi_value objValue)
{
    if (sessionInfo.want != nullptr) {
        napi_set_named_property(env, objValue, "windowTop",
            GetWindowRectIntValue(env,
            sessionInfo.want->GetIntParam(AAFwk::Want::PARAM_RESV_WINDOW_TOP, INVALID_VAL)));
        napi_set_named_property(env, objValue, "windowLeft",
            GetWindowRectIntValue(env,
            sessionInfo.want->GetIntParam(AAFwk::Want::PARAM_RESV_WINDOW_LEFT, INVALID_VAL)));
        napi_set_named_property(env, objValue, "windowWidth",
            GetWindowRectIntValue(env,
            sessionInfo.want->GetIntParam(AAFwk::Want::PARAM_RESV_WINDOW_WIDTH, INVALID_VAL)));
        napi_set_named_property(env, objValue, "windowHeight",
            GetWindowRectIntValue(env,
            sessionInfo.want->GetIntParam(AAFwk::Want::PARAM_RESV_WINDOW_HEIGHT, INVALID_VAL)));
        napi_set_named_property(env, objValue, "withAnimation",
            CreateJsValue(env, sessionInfo.want->GetBoolParam(AAFwk::Want::PARAM_RESV_WITH_ANIMATION, true)));
        napi_set_named_property(env, objValue, "focusedOnShow",
            CreateJsValue(env, sessionInfo.want->GetBoolParam(AAFwk::Want::PARAM_RESV_WINDOW_FOCUSED, true)));
        napi_set_named_property(env, objValue, "isStartupInstallFree",
            CreateJsValue(env, (sessionInfo.want->GetFlags() & AAFwk::Want::FLAG_INSTALL_ON_DEMAND) ==
                AAFwk::Want::FLAG_INSTALL_ON_DEMAND));
        auto params = sessionInfo.want->GetParams();
        napi_set_named_property(env, objValue, "fileManagerMode",
            CreateJsValue(env, params.GetStringParam("fileManagerMode")));
        napi_set_named_property(env, objValue, "floatingDisplayMode",
            CreateJsValue(env, params.GetIntParam("floatingDisplayMode", INVALID_VAL)));
        auto executeParams = params.GetWantParams("ohos.insightIntent.executeParam.param");
        napi_set_named_property(env, objValue, "extraFormIdentity",
            CreateJsValue(env, executeParams.GetStringParam("ohos.extra.param.key.form_identity")));
        if (params.HasParam("expectWindowMode")) {
            napi_set_named_property(env, objValue, "expectWindowMode",
                CreateJsValue(env, params.GetIntParam("expectWindowMode", INVALID_VAL)));
        }
        if (params.HasParam("isStartFromAppDock")) {
            napi_set_named_property(env, objValue, "isStartFromAppDock",
                CreateJsValue(env, params.GetIntParam("isStartFromAppDock", INVALID_VAL)));
        }
        if (params.HasParam("dockAppDirection")) {
            napi_set_named_property(env, objValue, "dockAppDirection",
                CreateJsValue(env, params.GetIntParam("dockAppDirection", INVALID_VAL)));
        }
        if (params.HasParam("isAppFromRecentAppsOrDockApps")) {
            napi_set_named_property(env, objValue, "isAppFromRecentAppsOrDockApps",
                CreateJsValue(env, params.GetIntParam("isAppFromRecentAppsOrDockApps", INVALID_VAL)));
        }
    }
}

napi_value GetWindowRectIntValue(napi_env env, int val)
{
    if (val != INVALID_VAL) {
        return  CreateJsValue(env, val);
    } else {
        return NapiGetUndefined(env);
    }
}

napi_value CreateJsSessionState(napi_env env)
{
    napi_value objValue = nullptr;
    napi_create_object(env, &objValue);
    if (objValue == nullptr) {
        WLOGFE("Failed to get object");
        return nullptr;
    }

    napi_set_named_property(env, objValue, "STATE_DISCONNECT", CreateJsValue(env,
        static_cast<int32_t>(SessionState::STATE_DISCONNECT)));
    napi_set_named_property(env, objValue, "STATE_CONNECT", CreateJsValue(env,
        static_cast<int32_t>(SessionState::STATE_CONNECT)));
    napi_set_named_property(env, objValue, "STATE_FOREGROUND", CreateJsValue(env,
        static_cast<int32_t>(SessionState::STATE_FOREGROUND)));
    napi_set_named_property(env, objValue, "STATE_ACTIVE", CreateJsValue(env,
        static_cast<int32_t>(SessionState::STATE_ACTIVE)));
    napi_set_named_property(env, objValue, "STATE_INACTIVE", CreateJsValue(env,
        static_cast<int32_t>(SessionState::STATE_INACTIVE)));
    napi_set_named_property(env, objValue, "STATE_BACKGROUND", CreateJsValue(env,
        static_cast<int32_t>(SessionState::STATE_BACKGROUND)));
    napi_set_named_property(env, objValue, "STATE_END", CreateJsValue(env,
        static_cast<int32_t>(SessionState::STATE_END)));

    return objValue;
}

napi_value CreateJsSessionSizeChangeReason(napi_env env)
{
    napi_value objValue = nullptr;
    napi_create_object(env, &objValue);
    if (objValue == nullptr) {
        WLOGFE("Failed to get object");
        return nullptr;
    }

    napi_set_named_property(env, objValue, "UNDEFINED", CreateJsValue(env,
        static_cast<int32_t>(SizeChangeReason::UNDEFINED)));
    napi_set_named_property(env, objValue, "MAXIMIZE", CreateJsValue(env,
        static_cast<int32_t>(SizeChangeReason::MAXIMIZE)));
    napi_set_named_property(env, objValue, "RECOVER", CreateJsValue(env,
        static_cast<int32_t>(SizeChangeReason::RECOVER)));
    napi_set_named_property(env, objValue, "ROTATION", CreateJsValue(env,
        static_cast<int32_t>(SizeChangeReason::ROTATION)));
    napi_set_named_property(env, objValue, "DRAG", CreateJsValue(env,
        static_cast<int32_t>(SizeChangeReason::DRAG)));
    napi_set_named_property(env, objValue, "DRAG_START", CreateJsValue(env,
        static_cast<int32_t>(SizeChangeReason::DRAG_START)));
    napi_set_named_property(env, objValue, "DRAG_MOVE", CreateJsValue(env,
        static_cast<int32_t>(SizeChangeReason::DRAG_MOVE)));
    napi_set_named_property(env, objValue, "DRAG_END", CreateJsValue(env,
        static_cast<int32_t>(SizeChangeReason::DRAG_END)));
    napi_set_named_property(env, objValue, "RESIZE", CreateJsValue(env,
        static_cast<int32_t>(SizeChangeReason::RESIZE)));
    napi_set_named_property(env, objValue, "MOVE", CreateJsValue(env,
        static_cast<int32_t>(SizeChangeReason::MOVE)));
    napi_set_named_property(env, objValue, "HIDE", CreateJsValue(env,
        static_cast<int32_t>(SizeChangeReason::HIDE)));
    napi_set_named_property(env, objValue, "TRANSFORM", CreateJsValue(env,
        static_cast<int32_t>(SizeChangeReason::TRANSFORM)));
    napi_set_named_property(env, objValue, "CUSTOM_ANIMATION_SHOW", CreateJsValue(env,
        static_cast<int32_t>(SizeChangeReason::CUSTOM_ANIMATION_SHOW)));
    napi_set_named_property(env, objValue, "FULL_TO_SPLIT", CreateJsValue(env,
        static_cast<int32_t>(SizeChangeReason::FULL_TO_SPLIT)));
    napi_set_named_property(env, objValue, "SPLIT_TO_FULL", CreateJsValue(env,
        static_cast<int32_t>(SizeChangeReason::SPLIT_TO_FULL)));
    napi_set_named_property(env, objValue, "FULL_TO_FLOATING", CreateJsValue(env,
        static_cast<int32_t>(SizeChangeReason::FULL_TO_FLOATING)));
    napi_set_named_property(env, objValue, "FLOATING_TO_FULL", CreateJsValue(env,
        static_cast<int32_t>(SizeChangeReason::FLOATING_TO_FULL)));
    CreatePiPSizeChangeReason(env, objValue);
    napi_set_named_property(env, objValue, "END", CreateJsValue(env,
        static_cast<int32_t>(SizeChangeReason::END)));

    return objValue;
}

void CreatePiPSizeChangeReason(napi_env env, napi_value objValue)
{
    napi_set_named_property(env, objValue, "PIP_START", CreateJsValue(env,
        static_cast<int32_t>(SizeChangeReason::PIP_START)));
    napi_set_named_property(env, objValue, "PIP_SHOW", CreateJsValue(env,
        static_cast<int32_t>(SizeChangeReason::PIP_SHOW)));
    napi_set_named_property(env, objValue, "PIP_AUTO_START", CreateJsValue(env,
        static_cast<int32_t>(SizeChangeReason::PIP_AUTO_START)));
    napi_set_named_property(env, objValue, "PIP_RATIO_CHANGE", CreateJsValue(env,
        static_cast<int32_t>(SizeChangeReason::PIP_RATIO_CHANGE)));
    napi_set_named_property(env, objValue, "PIP_RESTORE", CreateJsValue(env,
        static_cast<int32_t>(SizeChangeReason::PIP_RESTORE)));
}

napi_value CreateJsSessionStartupVisibility(napi_env env)
{
    if (env == nullptr) {
        WLOGFE("Env is nullptr");
        return nullptr;
    }
    napi_value objValue = nullptr;
    napi_create_object(env, &objValue);
    if (objValue == nullptr) {
        WLOGFE("Failed to create object!");
        return NapiGetUndefined(env);
    }

    napi_set_named_property(env, objValue, "UNSPECIFIED", CreateJsValue(env,
        static_cast<int32_t>(AAFwk::StartupVisibility::UNSPECIFIED)));
    napi_set_named_property(env, objValue, "STARTUP_HIDE", CreateJsValue(env,
        static_cast<int32_t>(AAFwk::StartupVisibility::STARTUP_HIDE)));
    napi_set_named_property(env, objValue, "STARTUP_SHOW", CreateJsValue(env,
        static_cast<int32_t>(AAFwk::StartupVisibility::STARTUP_SHOW)));
    napi_set_named_property(env, objValue, "END", CreateJsValue(env,
        static_cast<int32_t>(AAFwk::StartupVisibility::END)));
    return objValue;
}

napi_value CreateJsWindowVisibility(napi_env env)
{
    if (env == nullptr) {
        WLOGFE("Env is nullptr");
        return nullptr;
    }
    napi_value objValue = nullptr;
    napi_create_object(env, &objValue);
    if (objValue == nullptr) {
        WLOGFE("Failed to create object!");
        return NapiGetUndefined(env);
    }

    napi_set_named_property(env, objValue, "NO_OCCLUSION", CreateJsValue(env,
        static_cast<int32_t>(WindowVisibilityState::WINDOW_VISIBILITY_STATE_NO_OCCLUSION)));
    napi_set_named_property(env, objValue, "PARTIAL_OCCLUSION", CreateJsValue(env,
        static_cast<int32_t>(WindowVisibilityState::WINDOW_VISIBILITY_STATE_PARTICALLY_OCCLUSION)));
    napi_set_named_property(env, objValue, "COMPLETE_OCCLUSION", CreateJsValue(env,
        static_cast<int32_t>(WindowVisibilityState::WINDOW_VISIBILITY_STATE_TOTALLY_OCCUSION)));
    napi_set_named_property(env, objValue, "END", CreateJsValue(env,
        static_cast<int32_t>(WindowVisibilityState::WINDOW_LAYER_STATE_MAX)));
    return objValue;
}

napi_value CreateJsSessionProcessMode(napi_env env)
{
    if (env == nullptr) {
        WLOGFE("Env is nullptr");
        return nullptr;
    }
    napi_value objValue = nullptr;
    napi_create_object(env, &objValue);
    if (objValue == nullptr) {
        WLOGFE("Failed to create object!");
        return NapiGetUndefined(env);
    }

    napi_set_named_property(env, objValue, "UNSPECIFIED", CreateJsValue(env,
        static_cast<int32_t>(AAFwk::ProcessMode::UNSPECIFIED)));
    napi_set_named_property(env, objValue, "NEW_PROCESS_ATTACH_TO_PARENT", CreateJsValue(env,
        static_cast<int32_t>(AAFwk::ProcessMode::NEW_PROCESS_ATTACH_TO_PARENT)));
    napi_set_named_property(env, objValue, "NEW_PROCESS_ATTACH_TO_STATUS_BAR_ITEM", CreateJsValue(env,
        static_cast<int32_t>(AAFwk::ProcessMode::NEW_PROCESS_ATTACH_TO_STATUS_BAR_ITEM)));
    napi_set_named_property(env, objValue, "END", CreateJsValue(env,
        static_cast<int32_t>(AAFwk::ProcessMode::END)));
    return objValue;
}

napi_value CreateJsSessionPiPControlType(napi_env env)
{
    napi_value objValue = nullptr;
    napi_create_object(env, &objValue);
    if (objValue == nullptr) {
        WLOGFE("Failed to create object!");
        return NapiGetUndefined(env);
    }

    napi_set_named_property(env, objValue, "VIDEO_PLAY_PAUSE", CreateJsValue(env,
        static_cast<uint32_t>(PiPControlType::VIDEO_PLAY_PAUSE)));
    napi_set_named_property(env, objValue, "VIDEO_PREVIOUS", CreateJsValue(env,
        static_cast<uint32_t>(PiPControlType::VIDEO_PREVIOUS)));
    napi_set_named_property(env, objValue, "VIDEO_NEXT", CreateJsValue(env,
        static_cast<uint32_t>(PiPControlType::VIDEO_NEXT)));
    napi_set_named_property(env, objValue, "FAST_FORWARD", CreateJsValue(env,
        static_cast<uint32_t>(PiPControlType::FAST_FORWARD)));
    napi_set_named_property(env, objValue, "FAST_BACKWARD", CreateJsValue(env,
        static_cast<uint32_t>(PiPControlType::FAST_BACKWARD)));
    napi_set_named_property(env, objValue, "HANG_UP_BUTTON", CreateJsValue(env,
        static_cast<uint32_t>(PiPControlType::HANG_UP_BUTTON)));
    napi_set_named_property(env, objValue, "MICROPHONE_SWITCH", CreateJsValue(env,
        static_cast<uint32_t>(PiPControlType::MICROPHONE_SWITCH)));
    napi_set_named_property(env, objValue, "CAMERA_SWITCH", CreateJsValue(env,
        static_cast<uint32_t>(PiPControlType::CAMERA_SWITCH)));
    napi_set_named_property(env, objValue, "MUTE_SWITCH", CreateJsValue(env,
        static_cast<uint32_t>(PiPControlType::MUTE_SWITCH)));
    return objValue;
}

napi_value CreateJsSessionPiPControlStatus(napi_env env)
{
    napi_value objValue = nullptr;
    napi_create_object(env, &objValue);
    if (objValue == nullptr) {
        WLOGFE("Failed to create object!");
        return NapiGetUndefined(env);
    }

    napi_set_named_property(env, objValue, "PLAY", CreateJsValue(env,
        static_cast<int32_t>(PiPControlStatus::PLAY)));
    napi_set_named_property(env, objValue, "PAUSE", CreateJsValue(env,
        static_cast<int32_t>(PiPControlStatus::PAUSE)));
    napi_set_named_property(env, objValue, "OPEN", CreateJsValue(env,
        static_cast<int32_t>(PiPControlStatus::OPEN)));
    napi_set_named_property(env, objValue, "CLOSE", CreateJsValue(env,
        static_cast<int32_t>(PiPControlStatus::CLOSE)));
    napi_set_named_property(env, objValue, "ENABLED", CreateJsValue(env,
        static_cast<int32_t>(PiPControlStatus::ENABLED)));
    napi_set_named_property(env, objValue, "DISABLED", CreateJsValue(env,
        static_cast<int32_t>(PiPControlStatus::DISABLED)));
    return objValue;
}

napi_value CreateJsSessionGravity(napi_env env)
{
    napi_value objValue = nullptr;
    napi_create_object(env, &objValue);
    if (objValue == nullptr) {
        WLOGFE("Failed to create object!");
        return NapiGetUndefined(env);
    }
    using T = std::underlying_type_t<Gravity>;
    napi_set_named_property(env, objValue, "CENTER", CreateJsValue(env,
        static_cast<T>(Gravity::CENTER)));
    napi_set_named_property(env, objValue, "TOP", CreateJsValue(env,
        static_cast<T>(Gravity::TOP)));
    napi_set_named_property(env, objValue, "BOTTOM", CreateJsValue(env,
        static_cast<T>(Gravity::BOTTOM)));
    napi_set_named_property(env, objValue, "LEFT", CreateJsValue(env,
        static_cast<T>(Gravity::LEFT)));
    napi_set_named_property(env, objValue, "RIGHT", CreateJsValue(env,
        static_cast<T>(Gravity::RIGHT)));
    napi_set_named_property(env, objValue, "TOP_LEFT", CreateJsValue(env,
        static_cast<T>(Gravity::TOP_LEFT)));
    napi_set_named_property(env, objValue, "TOP_RIGHT", CreateJsValue(env,
        static_cast<T>(Gravity::TOP_RIGHT)));
    napi_set_named_property(env, objValue, "BOTTOM_LEFT", CreateJsValue(env,
        static_cast<T>(Gravity::BOTTOM_LEFT)));
    napi_set_named_property(env, objValue, "BOTTOM_RIGHT", CreateJsValue(env,
        static_cast<T>(Gravity::BOTTOM_RIGHT)));
    napi_set_named_property(env, objValue, "RESIZE", CreateJsValue(env,
        static_cast<T>(Gravity::RESIZE)));
    napi_set_named_property(env, objValue, "RESIZE_ASPECT", CreateJsValue(env,
        static_cast<T>(Gravity::RESIZE_ASPECT)));
    napi_set_named_property(env, objValue, "RESIZE_ASPECT_TOP_LEFT", CreateJsValue(env,
        static_cast<T>(Gravity::RESIZE_ASPECT_TOP_LEFT)));
    napi_set_named_property(env, objValue, "RESIZE_ASPECT_BOTTOM_RIGHT", CreateJsValue(env,
        static_cast<T>(Gravity::RESIZE_ASPECT_BOTTOM_RIGHT)));
    napi_set_named_property(env, objValue, "RESIZE_ASPECT_FILL", CreateJsValue(env,
        static_cast<T>(Gravity::RESIZE_ASPECT_FILL)));
    napi_set_named_property(env, objValue, "RESIZE_ASPECT_FILL_TOP_LEFT", CreateJsValue(env,
        static_cast<T>(Gravity::RESIZE_ASPECT_FILL_TOP_LEFT)));
    napi_set_named_property(env, objValue, "RESIZE_ASPECT_FILL_BOTTOM_RIGHT", CreateJsValue(env,
        static_cast<T>(Gravity::RESIZE_ASPECT_FILL_BOTTOM_RIGHT)));
    return objValue;
}

napi_value CreateJsSessionDragResizeType(napi_env env)
{
    napi_value objValue = nullptr;
    napi_create_object(env, &objValue);
    if (objValue == nullptr) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "Failed to create object!");
        return NapiGetUndefined(env);
    }

    napi_set_named_property(env, objValue, "RESIZE_TYPE_UNDEFINED", CreateJsValue(env,
        static_cast<uint32_t>(DragResizeType::RESIZE_TYPE_UNDEFINED)));
    napi_set_named_property(env, objValue, "RESIZE_EACH_FRAME", CreateJsValue(env,
        static_cast<uint32_t>(DragResizeType::RESIZE_EACH_FRAME)));
    napi_set_named_property(env, objValue, "RESIZE_WHEN_DRAG_END", CreateJsValue(env,
        static_cast<uint32_t>(DragResizeType::RESIZE_WHEN_DRAG_END)));
    return objValue;
}

template<typename T>
napi_value CreateJsSessionRect(napi_env env, const T& rect)
{
    WLOGFD("CreateJsSessionRect.");
    napi_value objValue = nullptr;
    napi_create_object(env, &objValue);
    if (objValue == nullptr) {
        WLOGFE("Failed to create object!");
        return NapiGetUndefined(env);
    }

    napi_set_named_property(env, objValue, "posX_", CreateJsValue(env, rect.posX_));
    napi_set_named_property(env, objValue, "posY_", CreateJsValue(env, rect.posY_));
    napi_set_named_property(env, objValue, "width_", CreateJsValue(env, rect.width_));
    napi_set_named_property(env, objValue, "height_", CreateJsValue(env, rect.height_));
    return objValue;
}

napi_value CreateJsSessionEventParam(napi_env env, const SessionEventParam& param)
{
    WLOGFD("CreateJsSessionEventParam.");
    napi_value objValue = nullptr;
    napi_create_object(env, &objValue);
    if (objValue == nullptr) {
        WLOGFE("Failed to create object!");
        return NapiGetUndefined(env);
    }

    napi_set_named_property(env, objValue, "pointerX", CreateJsValue(env, param.pointerX_));
    napi_set_named_property(env, objValue, "pointerY", CreateJsValue(env, param.pointerY_));
    napi_set_named_property(env, objValue, "sessionWidth", CreateJsValue(env, param.sessionWidth_));
    napi_set_named_property(env, objValue, "sessionHeight", CreateJsValue(env, param.sessionHeight_));
    napi_set_named_property(env, objValue, "dragResizeType", CreateJsValue(env, param.dragResizeType));
    return objValue;
}

napi_value SubWindowModalTypeInit(napi_env env)
{
    if (env == nullptr) {
        TLOGE(WmsLogTag::WMS_SUB, "Env is nullptr");
        return nullptr;
    }
    napi_value objValue = nullptr;
    napi_create_object(env, &objValue);
    if (objValue == nullptr) {
        TLOGE(WmsLogTag::WMS_SUB, "Failed to get object");
        return nullptr;
    }

    napi_set_named_property(env, objValue, "TYPE_UNDEFINED", CreateJsValue(env,
        static_cast<int32_t>(SubWindowModalType::TYPE_UNDEFINED)));
    napi_set_named_property(env, objValue, "TYPE_NORMAL", CreateJsValue(env,
        static_cast<int32_t>(SubWindowModalType::TYPE_NORMAL)));
    napi_set_named_property(env, objValue, "TYPE_DIALOG", CreateJsValue(env,
        static_cast<int32_t>(SubWindowModalType::TYPE_DIALOG)));
    napi_set_named_property(env, objValue, "TYPE_WINDOW_MODALITY", CreateJsValue(env,
        static_cast<int32_t>(SubWindowModalType::TYPE_WINDOW_MODALITY)));
    napi_set_named_property(env, objValue, "TYPE_TOAST", CreateJsValue(env,
        static_cast<int32_t>(SubWindowModalType::TYPE_TOAST)));
    napi_set_named_property(env, objValue, "TYPE_APPLICATION_MODALITY", CreateJsValue(env,
        static_cast<int32_t>(SubWindowModalType::TYPE_APPLICATION_MODALITY)));
    return objValue;
}

static std::string GetHexColor(uint32_t color)
{
    const int32_t rgbaLength = 8;

    std::stringstream ioss;
    std::string temp;
    ioss << std::setiosflags(std::ios::uppercase) << std::hex << color;
    ioss >> temp;
    int count = rgbaLength - static_cast<int>(temp.length());
    std::string finalColor("#");
    std::string tmpColor(count, '0');
    tmpColor += temp;
    finalColor += tmpColor;

    return finalColor;
}

static napi_value CreateJsSystemBarPropertyObject(
    napi_env env, const WindowType type, const SystemBarProperty& property)
{
    napi_value objValue = nullptr;
    napi_create_object(env, &objValue);
    if (objValue == nullptr) {
        WLOGFE("Failed to get jsObject");
        return nullptr;
    }
    if (WINDOW_TO_JS_SESSION_TYPE_MAP.count(type) != 0) {
        napi_set_named_property(env, objValue, "type", CreateJsValue(env, WINDOW_TO_JS_SESSION_TYPE_MAP.at(type)));
    } else {
        napi_set_named_property(env, objValue, "type", CreateJsValue(env, type));
    }
    napi_set_named_property(env, objValue, "enable", CreateJsValue(env, property.enable_));
    std::string bkgColor = GetHexColor(property.backgroundColor_);
    napi_set_named_property(env, objValue, "backgroundcolor", CreateJsValue(env, bkgColor));
    std::string contentColor = GetHexColor(property.contentColor_);
    napi_set_named_property(env, objValue, "contentcolor", CreateJsValue(env, contentColor));
    napi_set_named_property(env, objValue, "enableAnimation", CreateJsValue(env, property.enableAnimation_));
    napi_set_named_property(
        env, objValue, "settingFlag", CreateJsValue(env, static_cast<uint32_t>(property.settingFlag_)));

    return objValue;
}

napi_value CreateJsKeyboardLayoutParams(napi_env env, const KeyboardLayoutParams& params)
{
    napi_value objValue = nullptr;
    napi_create_object(env, &objValue);
    if (objValue == nullptr) {
        WLOGFE("Failed to get jsObject");
        return nullptr;
    }

    napi_set_named_property(env, objValue, "landscapeKeyboardRect",
        CreateJsSessionRect(env, params.LandscapeKeyboardRect_));
    napi_set_named_property(env, objValue, "portraitKeyboardRect",
        CreateJsSessionRect(env, params.PortraitKeyboardRect_));
    napi_set_named_property(env, objValue, "landscapePanelRect",
        CreateJsSessionRect(env, params.LandscapePanelRect_));
    napi_set_named_property(env, objValue, "portraitPanelRect",
        CreateJsSessionRect(env, params.PortraitPanelRect_));
    return objValue;
}

napi_value CreateJsSystemBarPropertyArrayObject(
    napi_env env, const std::unordered_map<WindowType, SystemBarProperty>& propertyMap)
{
    if (propertyMap.empty()) {
        WLOGFE("Empty propertyMap");
        return nullptr;
    }
    napi_value arrayValue = nullptr;
    napi_create_array_with_length(env, propertyMap.size(), &arrayValue);
    if (arrayValue == nullptr) {
        WLOGFE("Failed to get jsArrayObject");
        return nullptr;
    }
    uint32_t index = 0;
    for (auto iter: propertyMap) {
        napi_set_element(env, arrayValue, index++, CreateJsSystemBarPropertyObject(env, iter.first, iter.second));
    }
    return arrayValue;
}

static void SetTypeProperty(napi_value object, napi_env env, const std::string& name, JsSessionType type)
{
    napi_set_named_property(env, object, name.c_str(), CreateJsValue(env, static_cast<int32_t>(type)));
}

napi_value KeyboardGravityInit(napi_env env)
{
    WLOGFI("KeyboardGravityInit");

    if (env == nullptr) {
        WLOGFE("Env is nullptr");
        return nullptr;
    }

    napi_value objValue = nullptr;
    napi_create_object(env, &objValue);
    if (objValue == nullptr) {
        WLOGFE("Failed to get object");
        return nullptr;
    }
    napi_set_named_property(env, objValue, "GRAVITY_FLOAT", CreateJsValue(env,
        static_cast<int32_t>(SessionGravity::SESSION_GRAVITY_FLOAT)));
    napi_set_named_property(env, objValue, "GRAVITY_BOTTOM", CreateJsValue(env,
        static_cast<int32_t>(SessionGravity::SESSION_GRAVITY_BOTTOM)));
    napi_set_named_property(env, objValue, "GRAVITY_DEFAULT", CreateJsValue(env,
        static_cast<int32_t>(SessionGravity::SESSION_GRAVITY_DEFAULT)));
    return objValue;
}

napi_value SessionTypeInit(napi_env env)
{
    WLOGFD("in");

    if (env == nullptr) {
        WLOGFE("Env is nullptr");
        return nullptr;
    }

    napi_value objValue = nullptr;
    napi_create_object(env, &objValue);
    if (objValue == nullptr) {
        WLOGFE("Failed to get object");
        return nullptr;
    }

    SetTypeProperty(objValue, env, "TYPE_UNDEFINED", JsSessionType::TYPE_UNDEFINED);
    SetTypeProperty(objValue, env, "TYPE_APP", JsSessionType::TYPE_APP);
    SetTypeProperty(objValue, env, "TYPE_SUB_APP", JsSessionType::TYPE_SUB_APP);
    SetTypeProperty(objValue, env, "TYPE_SYSTEM_ALERT", JsSessionType::TYPE_SYSTEM_ALERT);
    SetTypeProperty(objValue, env, "TYPE_INPUT_METHOD", JsSessionType::TYPE_INPUT_METHOD);
    SetTypeProperty(objValue, env, "TYPE_STATUS_BAR", JsSessionType::TYPE_STATUS_BAR);
    SetTypeProperty(objValue, env, "TYPE_PANEL", JsSessionType::TYPE_PANEL);
    SetTypeProperty(objValue, env, "TYPE_KEYGUARD", JsSessionType::TYPE_KEYGUARD);
    SetTypeProperty(objValue, env, "TYPE_VOLUME_OVERLAY", JsSessionType::TYPE_VOLUME_OVERLAY);
    SetTypeProperty(objValue, env, "TYPE_NAVIGATION_BAR", JsSessionType::TYPE_NAVIGATION_BAR);
    SetTypeProperty(objValue, env, "TYPE_FLOAT", JsSessionType::TYPE_FLOAT);
    SetTypeProperty(objValue, env, "TYPE_WALLPAPER", JsSessionType::TYPE_WALLPAPER);
    SetTypeProperty(objValue, env, "TYPE_DESKTOP", JsSessionType::TYPE_DESKTOP);
    SetTypeProperty(objValue, env, "TYPE_LAUNCHER_DOCK", JsSessionType::TYPE_LAUNCHER_DOCK);
    SetTypeProperty(objValue, env, "TYPE_FLOAT_CAMERA", JsSessionType::TYPE_FLOAT_CAMERA);
    SetTypeProperty(objValue, env, "TYPE_DIALOG", JsSessionType::TYPE_DIALOG);
    SetTypeProperty(objValue, env, "TYPE_SCREENSHOT", JsSessionType::TYPE_SCREENSHOT);
    SetTypeProperty(objValue, env, "TYPE_TOAST", JsSessionType::TYPE_TOAST);
    SetTypeProperty(objValue, env, "TYPE_POINTER", JsSessionType::TYPE_POINTER);
    SetTypeProperty(objValue, env, "TYPE_LAUNCHER_RECENT", JsSessionType::TYPE_LAUNCHER_RECENT);
    SetTypeProperty(objValue, env, "TYPE_SCENE_BOARD", JsSessionType::TYPE_SCENE_BOARD);
    SetTypeProperty(objValue, env, "TYPE_DRAGGING_EFFECT", JsSessionType::TYPE_DRAGGING_EFFECT);
    SetTypeProperty(objValue, env, "TYPE_INPUT_METHOD_STATUS_BAR", JsSessionType::TYPE_INPUT_METHOD_STATUS_BAR);
    SetTypeProperty(objValue, env, "TYPE_GLOBAL_SEARCH", JsSessionType::TYPE_GLOBAL_SEARCH);
    SetTypeProperty(objValue, env, "TYPE_NEGATIVE_SCREEN", JsSessionType::TYPE_NEGATIVE_SCREEN);
    SetTypeProperty(objValue, env, "TYPE_VOICE_INTERACTION", JsSessionType::TYPE_VOICE_INTERACTION);
    SetTypeProperty(objValue, env, "TYPE_SYSTEM_TOAST", JsSessionType::TYPE_SYSTEM_TOAST);
    SetTypeProperty(objValue, env, "TYPE_SYSTEM_FLOAT", JsSessionType::TYPE_SYSTEM_FLOAT);
    SetTypeProperty(objValue, env, "TYPE_THEME_EDITOR", JsSessionType::TYPE_THEME_EDITOR);
    SetTypeProperty(objValue, env, "TYPE_PIP", JsSessionType::TYPE_PIP);
    SetTypeProperty(objValue, env, "TYPE_NAVIGATION_INDICATOR", JsSessionType::TYPE_NAVIGATION_INDICATOR);
    SetTypeProperty(objValue, env, "TYPE_HANDWRITE", JsSessionType::TYPE_HANDWRITE);
    SetTypeProperty(objValue, env, "TYPE_KEYBOARD_PANEL", JsSessionType::TYPE_KEYBOARD_PANEL);
    SetTypeProperty(objValue, env, "TYPE_DIVIDER", JsSessionType::TYPE_DIVIDER);
    SetTypeProperty(objValue, env, "TYPE_TRANSPARENT_VIEW", JsSessionType::TYPE_TRANSPARENT_VIEW);
    return objValue;
}

napi_value SceneTypeInit(napi_env env)
{
    WLOGFD("in");

    if (env == nullptr) {
        WLOGFE("Env is nullptr");
        return nullptr;
    }

    napi_value objValue = nullptr;
    napi_create_object(env, &objValue);
    if (objValue == nullptr) {
        WLOGFE("Failed to get object");
        return nullptr;
    }
    napi_set_named_property(env, objValue, "DEFAULT",
        CreateJsValue(env, static_cast<int32_t>(SceneType::DEFAULT)));
    napi_set_named_property(env, objValue, "WINDOW_SCENE",
        CreateJsValue(env, static_cast<int32_t>(SceneType::WINDOW_SCENE)));
    napi_set_named_property(env, objValue, "SYSTEM_WINDOW_SCENE",
        CreateJsValue(env, static_cast<int32_t>(SceneType::SYSTEM_WINDOW_SCENE)));
    napi_set_named_property(env, objValue, "TRANSFORM_SCENE",
        CreateJsValue(env, static_cast<int32_t>(SceneType::TRANSFORM_SCENE)));
    napi_set_named_property(env, objValue, "PANEL_SCENE",
        CreateJsValue(env, static_cast<int32_t>(SceneType::PANEL_SCENE)));
    napi_set_named_property(env, objValue, "INPUT_SCENE",
        CreateJsValue(env, static_cast<int32_t>(SceneType::INPUT_SCENE)));
    return objValue;
}

struct AsyncInfo {
    napi_env env;
    napi_async_work work;
    std::function<void()> func;
};

static void NapiAsyncWork(napi_env env, std::function<void()> task)
{
    napi_value resource = nullptr;
    AsyncInfo* info = new AsyncInfo();
    info->env = env;
    info->func = task;
    napi_create_string_utf8(env, "AsyncWork", NAPI_AUTO_LENGTH, &resource);
    napi_create_async_work(env, nullptr, resource, [](napi_env env, void* data) {
    },
    [](napi_env env, napi_status status, void* data) {
        AsyncInfo* info = (AsyncInfo*)data;
        info->func();
        napi_delete_async_work(env, info->work);
        delete info;
    }, (void*)info, &info->work);
    napi_queue_async_work(env, info->work);
}

MainThreadScheduler::MainThreadScheduler(napi_env env)
    : env_(env)
{
    GetMainEventHandler();
    envChecker_ = std::make_shared<int>(0);
}

inline void MainThreadScheduler::GetMainEventHandler()
{
    if (handler_ != nullptr) {
        return;
    }
    auto runner = OHOS::AppExecFwk::EventRunner::GetMainEventRunner();
    if (runner == nullptr) {
        return;
    }
    handler_ = std::make_shared<OHOS::AppExecFwk::EventHandler>(runner);
}

void MainThreadScheduler::PostMainThreadTask(Task&& localTask, std::string traceInfo, int64_t delayTime)
{
    GetMainEventHandler();
    auto task = [env = env_, localTask, traceInfo, envChecker = std::weak_ptr<int>(envChecker_)]() {
        HITRACE_METER_FMT(HITRACE_TAG_WINDOW_MANAGER, "SCBCb:%s", traceInfo.c_str());
        if (envChecker.expired()) {
            TLOGNE(WmsLogTag::WMS_MAIN, "post task expired because of invalid scheduler");
            return;
        }
        napi_handle_scope scope = nullptr;
        napi_open_handle_scope(env, &scope);
        localTask();
        napi_close_handle_scope(env, scope);
    };
    if (handler_ && handler_->GetEventRunner()->IsCurrentRunnerThread()) {
        return task();
    } else if (handler_ && !handler_->GetEventRunner()->IsCurrentRunnerThread()) {
        handler_->PostTask(std::move(task), "wms:" + traceInfo, delayTime,
            OHOS::AppExecFwk::EventQueue::Priority::IMMEDIATE);
    } else {
        NapiAsyncWork(env_, task);
    }
}
} // namespace OHOS::Rosen