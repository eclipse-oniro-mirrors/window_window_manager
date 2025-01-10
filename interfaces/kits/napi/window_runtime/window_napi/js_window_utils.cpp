/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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
#include "js_window_utils.h"
#include <iomanip>
#include <regex>
#include <sstream>
#include "accesstoken_kit.h"
#include "bundle_constants.h"
#include "ipc_skeleton.h"
#include "window_manager_hilog.h"
#include "js_window.h"

namespace OHOS {
namespace Rosen {
using namespace AbilityRuntime;
namespace {
constexpr HiviewDFX::HiLogLabel LABEL = {LOG_CORE, HILOG_DOMAIN_WINDOW, "JsUtils"};
constexpr size_t INDEX_ZERO = 0;
constexpr size_t INDEX_ONE = 1;
constexpr size_t INDEX_TWO = 2;
constexpr size_t FOUR_PARAMS_SIZE = 4;
constexpr size_t ARG_COUNT_ZERO = 0;
constexpr size_t ARG_COUNT_TWO = 2;
constexpr size_t ARG_COUNT_THREE = 3;
constexpr int32_t MAX_TOUCHABLE_AREAS = 10;
}

napi_value WindowTypeInit(napi_env env)
{
    WLOGFD("WindowTypeInit");

    CHECK_NAPI_ENV_RETURN_IF_NULL(env);

    napi_value objValue = nullptr;
    CHECK_NAPI_CREATE_OBJECT_RETURN_IF_NULL(env, objValue);

    napi_set_named_property(env, objValue, "TYPE_APP", CreateJsValue(env,
        static_cast<int32_t>(ApiWindowType::TYPE_APP)));
    napi_set_named_property(env, objValue, "TYPE_SYSTEM_ALERT", CreateJsValue(env,
        static_cast<int32_t>(ApiWindowType::TYPE_SYSTEM_ALERT)));
    napi_set_named_property(env, objValue, "TYPE_INPUT_METHOD", CreateJsValue(env,
        static_cast<int32_t>(ApiWindowType::TYPE_INPUT_METHOD)));
    napi_set_named_property(env, objValue, "TYPE_STATUS_BAR", CreateJsValue(env,
        static_cast<int32_t>(ApiWindowType::TYPE_STATUS_BAR)));
    napi_set_named_property(env, objValue, "TYPE_PANEL", CreateJsValue(env,
        static_cast<int32_t>(ApiWindowType::TYPE_PANEL)));
    napi_set_named_property(env, objValue, "TYPE_KEYGUARD", CreateJsValue(env,
        static_cast<int32_t>(ApiWindowType::TYPE_KEYGUARD)));
    napi_set_named_property(env, objValue, "TYPE_VOLUME_OVERLAY", CreateJsValue(env,
        static_cast<int32_t>(ApiWindowType::TYPE_VOLUME_OVERLAY)));
    napi_set_named_property(env, objValue, "TYPE_NAVIGATION_BAR", CreateJsValue(env,
        static_cast<int32_t>(ApiWindowType::TYPE_NAVIGATION_BAR)));
    napi_set_named_property(env, objValue, "TYPE_FLOAT", CreateJsValue(env,
        static_cast<int32_t>(ApiWindowType::TYPE_FLOAT)));
    napi_set_named_property(env, objValue, "TYPE_FLOAT_CAMERA", CreateJsValue(env,
        static_cast<int32_t>(ApiWindowType::TYPE_FLOAT_CAMERA)));
    napi_set_named_property(env, objValue, "TYPE_WALLPAPER", CreateJsValue(env,
        static_cast<int32_t>(ApiWindowType::TYPE_WALLPAPER)));
    napi_set_named_property(env, objValue, "TYPE_DESKTOP", CreateJsValue(env,
        static_cast<int32_t>(ApiWindowType::TYPE_DESKTOP)));
    napi_set_named_property(env, objValue, "TYPE_LAUNCHER_RECENT", CreateJsValue(env,
        static_cast<int32_t>(ApiWindowType::TYPE_LAUNCHER_RECENT)));
    napi_set_named_property(env, objValue, "TYPE_LAUNCHER_DOCK", CreateJsValue(env,
        static_cast<int32_t>(ApiWindowType::TYPE_LAUNCHER_DOCK)));
    napi_set_named_property(env, objValue, "TYPE_VOICE_INTERACTION", CreateJsValue(env,
        static_cast<int32_t>(ApiWindowType::TYPE_VOICE_INTERACTION)));
    napi_set_named_property(env, objValue, "TYPE_DIALOG", CreateJsValue(env,
        static_cast<int32_t>(ApiWindowType::TYPE_DIALOG)));
    napi_set_named_property(env, objValue, "TYPE_POINTER", CreateJsValue(env,
        static_cast<int32_t>(ApiWindowType::TYPE_POINTER)));
    napi_set_named_property(env, objValue, "TYPE_SCREENSHOT", CreateJsValue(env,
        static_cast<int32_t>(ApiWindowType::TYPE_SCREENSHOT)));
    napi_set_named_property(env, objValue, "TYPE_SYSTEM_TOAST", CreateJsValue(env,
        static_cast<int32_t>(ApiWindowType::TYPE_SYSTEM_TOAST)));
    napi_set_named_property(env, objValue, "TYPE_DIVIDER", CreateJsValue(env,
        static_cast<int32_t>(ApiWindowType::TYPE_DIVIDER)));
    napi_set_named_property(env, objValue, "TYPE_GLOBAL_SEARCH", CreateJsValue(env,
        static_cast<int32_t>(ApiWindowType::TYPE_GLOBAL_SEARCH)));
    napi_set_named_property(env, objValue, "TYPE_HANDWRITE", CreateJsValue(env,
        static_cast<int32_t>(ApiWindowType::TYPE_HANDWRITE)));

    return objValue;
}

napi_value AvoidAreaTypeInit(napi_env env)
{
    WLOGFD("AvoidAreaTypeInit");

    CHECK_NAPI_ENV_RETURN_IF_NULL(env);

    napi_value objValue = nullptr;
    CHECK_NAPI_CREATE_OBJECT_RETURN_IF_NULL(env, objValue);

    napi_set_named_property(env, objValue, "TYPE_SYSTEM", CreateJsValue(env,
        static_cast<int32_t>(AvoidAreaType::TYPE_SYSTEM)));
    napi_set_named_property(env, objValue, "TYPE_CUTOUT", CreateJsValue(env,
        static_cast<int32_t>(AvoidAreaType::TYPE_CUTOUT)));
    napi_set_named_property(env, objValue, "TYPE_SYSTEM_GESTURE", CreateJsValue(env,
        static_cast<int32_t>(AvoidAreaType::TYPE_SYSTEM_GESTURE)));
    napi_set_named_property(env, objValue, "TYPE_KEYBOARD",
        CreateJsValue(env, static_cast<int32_t>(AvoidAreaType::TYPE_KEYBOARD)));
    napi_set_named_property(env, objValue, "TYPE_NAVIGATION_INDICATOR",
        CreateJsValue(env, static_cast<int32_t>(AvoidAreaType::TYPE_NAVIGATION_INDICATOR)));
    return objValue;
}

napi_value WindowModeInit(napi_env env)
{
    WLOGFD("WindowModeInit");

    CHECK_NAPI_ENV_RETURN_IF_NULL(env);

    napi_value objValue = nullptr;
    CHECK_NAPI_CREATE_OBJECT_RETURN_IF_NULL(env, objValue);

    napi_set_named_property(env, objValue, "UNDEFINED", CreateJsValue(env,
        static_cast<int32_t>(ApiWindowMode::UNDEFINED)));
    napi_set_named_property(env, objValue, "FULLSCREEN", CreateJsValue(env,
        static_cast<int32_t>(ApiWindowMode::FULLSCREEN)));
    napi_set_named_property(env, objValue, "PRIMARY", CreateJsValue(env,
        static_cast<int32_t>(ApiWindowMode::PRIMARY)));
    napi_set_named_property(env, objValue, "SECONDARY", CreateJsValue(env,
        static_cast<int32_t>(ApiWindowMode::SECONDARY)));
    napi_set_named_property(env, objValue, "FLOATING", CreateJsValue(env,
        static_cast<int32_t>(ApiWindowMode::FLOATING)));
    return objValue;
}

napi_value ColorSpaceInit(napi_env env)
{
    WLOGFD("ColorSpaceInit");

    CHECK_NAPI_ENV_RETURN_IF_NULL(env);

    napi_value objValue = nullptr;
    CHECK_NAPI_CREATE_OBJECT_RETURN_IF_NULL(env, objValue);

    napi_set_named_property(env, objValue, "DEFAULT", CreateJsValue(env,
        static_cast<int32_t>(ColorSpace::COLOR_SPACE_DEFAULT)));
    napi_set_named_property(env, objValue, "WIDE_GAMUT", CreateJsValue(env,
        static_cast<int32_t>(ColorSpace::COLOR_SPACE_WIDE_GAMUT)));
    return objValue;
}

napi_value OrientationInit(napi_env env)
{
    WLOGFD("OrientationInit");

    CHECK_NAPI_ENV_RETURN_IF_NULL(env);

    napi_value objValue = nullptr;
    CHECK_NAPI_CREATE_OBJECT_RETURN_IF_NULL(env, objValue);

    napi_set_named_property(env, objValue, "UNSPECIFIED", CreateJsValue(env,
        static_cast<int32_t>(ApiOrientation::UNSPECIFIED)));
    napi_set_named_property(env, objValue, "PORTRAIT", CreateJsValue(env,
        static_cast<int32_t>(ApiOrientation::PORTRAIT)));
    napi_set_named_property(env, objValue, "LANDSCAPE", CreateJsValue(env,
        static_cast<int32_t>(ApiOrientation::LANDSCAPE)));
    napi_set_named_property(env, objValue, "PORTRAIT_INVERTED", CreateJsValue(env,
        static_cast<int32_t>(ApiOrientation::PORTRAIT_INVERTED)));
    napi_set_named_property(env, objValue, "LANDSCAPE_INVERTED", CreateJsValue(env,
        static_cast<int32_t>(ApiOrientation::LANDSCAPE_INVERTED)));
    napi_set_named_property(env, objValue, "AUTO_ROTATION", CreateJsValue(env,
        static_cast<int32_t>(ApiOrientation::AUTO_ROTATION)));
    napi_set_named_property(env, objValue, "AUTO_ROTATION_PORTRAIT", CreateJsValue(env,
        static_cast<int32_t>(ApiOrientation::AUTO_ROTATION_PORTRAIT)));
    napi_set_named_property(env, objValue, "AUTO_ROTATION_LANDSCAPE", CreateJsValue(env,
        static_cast<int32_t>(ApiOrientation::AUTO_ROTATION_LANDSCAPE)));
    napi_set_named_property(env, objValue, "AUTO_ROTATION_RESTRICTED", CreateJsValue(env,
        static_cast<int32_t>(ApiOrientation::AUTO_ROTATION_RESTRICTED)));
    napi_set_named_property(env, objValue, "AUTO_ROTATION_PORTRAIT_RESTRICTED", CreateJsValue(env,
        static_cast<int32_t>(ApiOrientation::AUTO_ROTATION_PORTRAIT_RESTRICTED)));
    napi_set_named_property(env, objValue, "AUTO_ROTATION_LANDSCAPE_RESTRICTED", CreateJsValue(env,
        static_cast<int32_t>(ApiOrientation::AUTO_ROTATION_LANDSCAPE_RESTRICTED)));
    napi_set_named_property(env, objValue, "LOCKED", CreateJsValue(env,
        static_cast<int32_t>(ApiOrientation::LOCKED)));
    napi_set_named_property(env, objValue, "AUTO_ROTATION_UNSPECIFIED", CreateJsValue(env,
        static_cast<int32_t>(ApiOrientation::AUTO_ROTATION_UNSPECIFIED)));
    napi_set_named_property(env, objValue, "USER_ROTATION_PORTRAIT", CreateJsValue(env,
        static_cast<int32_t>(ApiOrientation::USER_ROTATION_PORTRAIT)));
    napi_set_named_property(env, objValue, "USER_ROTATION_LANDSCAPE", CreateJsValue(env,
        static_cast<int32_t>(ApiOrientation::USER_ROTATION_LANDSCAPE)));
    napi_set_named_property(env, objValue, "USER_ROTATION_PORTRAIT_INVERTED", CreateJsValue(env,
        static_cast<int32_t>(ApiOrientation::USER_ROTATION_PORTRAIT_INVERTED)));
    napi_set_named_property(env, objValue, "USER_ROTATION_LANDSCAPE_INVERTED", CreateJsValue(env,
        static_cast<int32_t>(ApiOrientation::USER_ROTATION_LANDSCAPE_INVERTED)));
    napi_set_named_property(env, objValue, "FOLLOW_DESKTOP", CreateJsValue(env,
        static_cast<int32_t>(ApiOrientation::FOLLOW_DESKTOP)));
    return objValue;
}

napi_value WindowStageEventTypeInit(napi_env env)
{
    WLOGFD("WindowStageEventTypeInit");

    CHECK_NAPI_ENV_RETURN_IF_NULL(env);

    napi_value objValue = nullptr;
    CHECK_NAPI_CREATE_OBJECT_RETURN_IF_NULL(env, objValue);

    napi_set_named_property(env, objValue, "SHOWN", CreateJsValue(env,
        static_cast<int32_t>(LifeCycleEventType::FOREGROUND)));
    napi_set_named_property(env, objValue, "ACTIVE", CreateJsValue(env,
        static_cast<int32_t>(LifeCycleEventType::ACTIVE)));
    napi_set_named_property(env, objValue, "INACTIVE", CreateJsValue(env,
        static_cast<int32_t>(LifeCycleEventType::INACTIVE)));
    napi_set_named_property(env, objValue, "HIDDEN", CreateJsValue(env,
        static_cast<int32_t>(LifeCycleEventType::BACKGROUND)));
    napi_set_named_property(env, objValue, "RESUMED", CreateJsValue(env,
        static_cast<int32_t>(LifeCycleEventType::RESUMED)));
    napi_set_named_property(env, objValue, "PAUSED", CreateJsValue(env,
        static_cast<int32_t>(LifeCycleEventType::PAUSED)));
    return objValue;
}

napi_value WindowEventTypeInit(napi_env env)
{
    WLOGFD("WindowEventTypeInit");

    CHECK_NAPI_ENV_RETURN_IF_NULL(env);

    napi_value objValue = nullptr;
    CHECK_NAPI_CREATE_OBJECT_RETURN_IF_NULL(env, objValue);

    napi_set_named_property(env, objValue, "WINDOW_SHOWN", CreateJsValue(env,
        static_cast<int32_t>(LifeCycleEventType::FOREGROUND)));
    napi_set_named_property(env, objValue, "WINDOW_ACTIVE", CreateJsValue(env,
        static_cast<int32_t>(LifeCycleEventType::ACTIVE)));
    napi_set_named_property(env, objValue, "WINDOW_INACTIVE", CreateJsValue(env,
        static_cast<int32_t>(LifeCycleEventType::INACTIVE)));
    napi_set_named_property(env, objValue, "WINDOW_HIDDEN", CreateJsValue(env,
        static_cast<int32_t>(LifeCycleEventType::BACKGROUND)));
    napi_set_named_property(env, objValue, "WINDOW_DESTROYED", CreateJsValue(env,
        static_cast<int32_t>(LifeCycleEventType::DESTROYED)));
    return objValue;
}

napi_value WindowLayoutModeInit(napi_env env)
{
    WLOGFD("WindowLayoutModeInit");
    CHECK_NAPI_ENV_RETURN_IF_NULL(env);

    napi_value objValue = nullptr;
    CHECK_NAPI_CREATE_OBJECT_RETURN_IF_NULL(env, objValue);

    napi_set_named_property(env, objValue, "WINDOW_LAYOUT_MODE_CASCADE", CreateJsValue(env,
        static_cast<int32_t>(WindowLayoutMode::CASCADE)));
    napi_set_named_property(env, objValue, "WINDOW_LAYOUT_MODE_TILE", CreateJsValue(env,
        static_cast<int32_t>(WindowLayoutMode::TILE)));
    return objValue;
}

napi_value BlurStyleInit(napi_env env)
{
    WLOGD("BlurStyleInit");
    CHECK_NAPI_ENV_RETURN_IF_NULL(env);

    napi_value objValue = nullptr;
    CHECK_NAPI_CREATE_OBJECT_RETURN_IF_NULL(env, objValue);

    napi_set_named_property(env, objValue, "OFF", CreateJsValue(env,
        static_cast<int32_t>(WindowBlurStyle::WINDOW_BLUR_OFF)));
    napi_set_named_property(env, objValue, "THIN", CreateJsValue(env,
        static_cast<int32_t>(WindowBlurStyle::WINDOW_BLUR_THIN)));
    napi_set_named_property(env, objValue, "REGULAR", CreateJsValue(env,
        static_cast<int32_t>(WindowBlurStyle::WINDOW_BLUR_REGULAR)));
    napi_set_named_property(env, objValue, "THICK", CreateJsValue(env,
        static_cast<int32_t>(WindowBlurStyle::WINDOW_BLUR_THICK)));
    return objValue;
}

napi_value MaximizePresentationInit(napi_env env)
{
    WLOGD("MaximizePresentationInit");
    CHECK_NAPI_ENV_RETURN_IF_NULL(env);

    napi_value objValue = nullptr;
    CHECK_NAPI_CREATE_OBJECT_RETURN_IF_NULL(env, objValue);

    napi_set_named_property(env, objValue, "FOLLOW_APP_IMMERSIVE_SETTING", CreateJsValue(env,
        static_cast<int32_t>(MaximizePresentation::FOLLOW_APP_IMMERSIVE_SETTING)));
    napi_set_named_property(env, objValue, "EXIT_IMMERSIVE", CreateJsValue(env,
        static_cast<int32_t>(MaximizePresentation::EXIT_IMMERSIVE)));
    napi_set_named_property(env, objValue, "ENTER_IMMERSIVE", CreateJsValue(env,
        static_cast<int32_t>(MaximizePresentation::ENTER_IMMERSIVE)));
    napi_set_named_property(env, objValue, "ENTER_IMMERSIVE_DISABLE_TITLE_AND_DOCK_HOVER", CreateJsValue(env,
        static_cast<int32_t>(MaximizePresentation::ENTER_IMMERSIVE_DISABLE_TITLE_AND_DOCK_HOVER)));
    return objValue;
}

napi_value WindowErrorInit(napi_env env)
{
    WLOGFD("WindowErrorInit");
    CHECK_NAPI_ENV_RETURN_IF_NULL(env);

    napi_value objValue = nullptr;
    CHECK_NAPI_CREATE_OBJECT_RETURN_IF_NULL(env, objValue);

    napi_set_named_property(env, objValue, "WM_DO_NOTHING", CreateJsValue(env,
        static_cast<int32_t>(WMError::WM_DO_NOTHING)));
    napi_set_named_property(env, objValue, "WM_ERROR_NO_MEM", CreateJsValue(env,
        static_cast<int32_t>(WMError::WM_ERROR_NO_MEM)));
    napi_set_named_property(env, objValue, "WM_ERROR_DESTROYED_OBJECT", CreateJsValue(env,
        static_cast<int32_t>(WMError::WM_ERROR_DESTROYED_OBJECT)));
    napi_set_named_property(env, objValue, "WM_ERROR_INVALID_WINDOW", CreateJsValue(env,
        static_cast<int32_t>(WMError::WM_ERROR_INVALID_WINDOW)));
    napi_set_named_property(env, objValue, "WM_ERROR_INVALID_WINDOW_MODE_OR_SIZE", CreateJsValue(env,
        static_cast<int32_t>(WMError::WM_ERROR_INVALID_WINDOW_MODE_OR_SIZE)));
    napi_set_named_property(env, objValue, "WM_ERROR_INVALID_OPERATION", CreateJsValue(env,
        static_cast<int32_t>(WMError::WM_ERROR_INVALID_OPERATION)));
    napi_set_named_property(env, objValue, "WM_ERROR_INVALID_PERMISSION", CreateJsValue(env,
        static_cast<int32_t>(WMError::WM_ERROR_INVALID_PERMISSION)));
    napi_set_named_property(env, objValue, "WM_ERROR_NO_REMOTE_ANIMATION", CreateJsValue(env,
        static_cast<int32_t>(WMError::WM_ERROR_NO_REMOTE_ANIMATION)));
    napi_set_named_property(env, objValue, "WM_ERROR_DEVICE_NOT_SUPPORT", CreateJsValue(env,
        static_cast<int32_t>(WMError::WM_ERROR_DEVICE_NOT_SUPPORT)));
    napi_set_named_property(env, objValue, "WM_ERROR_NULLPTR", CreateJsValue(env,
        static_cast<int32_t>(WMError::WM_ERROR_NULLPTR)));
    napi_set_named_property(env, objValue, "WM_ERROR_INVALID_TYPE", CreateJsValue(env,
        static_cast<int32_t>(WMError::WM_ERROR_INVALID_TYPE)));
    napi_set_named_property(env, objValue, "WM_ERROR_INVALID_PARAM", CreateJsValue(env,
        static_cast<int32_t>(WMError::WM_ERROR_INVALID_PARAM)));
    napi_set_named_property(env, objValue, "WM_ERROR_SAMGR", CreateJsValue(env,
        static_cast<int32_t>(WMError::WM_ERROR_SAMGR)));
    napi_set_named_property(env, objValue, "WM_ERROR_IPC_FAILED", CreateJsValue(env,
        static_cast<int32_t>(WMError::WM_ERROR_IPC_FAILED)));
    napi_set_named_property(env, objValue, "WM_ERROR_START_ABILITY_FAILED", CreateJsValue(env,
        static_cast<int32_t>(WMError::WM_ERROR_START_ABILITY_FAILED)));
    return objValue;
}

napi_value WindowErrorCodeInit(napi_env env)
{
    WLOGFD("WindowErrorCodeInit");
    CHECK_NAPI_ENV_RETURN_IF_NULL(env);

    napi_value objValue = nullptr;
    CHECK_NAPI_CREATE_OBJECT_RETURN_IF_NULL(env, objValue);
    napi_set_named_property(env, objValue, "WM_ERROR_NO_PERMISSION", CreateJsValue(env,
        static_cast<int32_t>(WmErrorCode::WM_ERROR_NO_PERMISSION)));
    napi_set_named_property(env, objValue, "WM_ERROR_NOT_SYSTEM_APP", CreateJsValue(env,
        static_cast<int32_t>(WmErrorCode::WM_ERROR_NOT_SYSTEM_APP)));
    napi_set_named_property(env, objValue, "WM_ERROR_INVALID_PARAM", CreateJsValue(env,
        static_cast<int32_t>(WmErrorCode::WM_ERROR_INVALID_PARAM)));
    napi_set_named_property(env, objValue, "WM_ERROR_DEVICE_NOT_SUPPORT", CreateJsValue(env,
        static_cast<int32_t>(WmErrorCode::WM_ERROR_DEVICE_NOT_SUPPORT)));
    napi_set_named_property(env, objValue, "WM_ERROR_REPEAT_OPERATION", CreateJsValue(env,
        static_cast<int32_t>(WmErrorCode::WM_ERROR_REPEAT_OPERATION)));
    napi_set_named_property(env, objValue, "WM_ERROR_STATE_ABNORMALLY", CreateJsValue(env,
        static_cast<int32_t>(WmErrorCode::WM_ERROR_STATE_ABNORMALLY)));
    napi_set_named_property(env, objValue, "WM_ERROR_SYSTEM_ABNORMALLY", CreateJsValue(env,
        static_cast<int32_t>(WmErrorCode::WM_ERROR_SYSTEM_ABNORMALLY)));
    napi_set_named_property(env, objValue, "WM_ERROR_INVALID_CALLING", CreateJsValue(env,
        static_cast<int32_t>(WmErrorCode::WM_ERROR_INVALID_CALLING)));
    napi_set_named_property(env, objValue, "WM_ERROR_STAGE_ABNORMALLY", CreateJsValue(env,
        static_cast<int32_t>(WmErrorCode::WM_ERROR_STAGE_ABNORMALLY)));
    napi_set_named_property(env, objValue, "WM_ERROR_CONTEXT_ABNORMALLY", CreateJsValue(env,
        static_cast<int32_t>(WmErrorCode::WM_ERROR_CONTEXT_ABNORMALLY)));
    napi_set_named_property(env, objValue, "WM_ERROR_START_ABILITY_FAILED", CreateJsValue(env,
        static_cast<int32_t>(WmErrorCode::WM_ERROR_START_ABILITY_FAILED)));
    return objValue;
}

napi_value WindowStatusTypeInit(napi_env env)
{
    WLOGFD("WindowStatusTypeInit");
    CHECK_NAPI_ENV_RETURN_IF_NULL(env);

    napi_value objValue = nullptr;
    CHECK_NAPI_CREATE_OBJECT_RETURN_IF_NULL(env, objValue);
    napi_set_named_property(env, objValue, "UNDEFINED", CreateJsValue(env,
        static_cast<int32_t>(WindowStatus::WINDOW_STATUS_UNDEFINED)));
    napi_set_named_property(env, objValue, "FULL_SCREEN", CreateJsValue(env,
        static_cast<int32_t>(WindowStatus::WINDOW_STATUS_FULLSCREEN)));
    napi_set_named_property(env, objValue, "MAXIMIZE", CreateJsValue(env,
        static_cast<int32_t>(WindowStatus::WINDOW_STATUS_MAXIMIZE)));
    napi_set_named_property(env, objValue, "MINIMIZE", CreateJsValue(env,
        static_cast<int32_t>(WindowStatus::WINDOW_STATUS_MINIMIZE)));
    napi_set_named_property(env, objValue, "FLOATING", CreateJsValue(env,
        static_cast<int32_t>(WindowStatus::WINDOW_STATUS_FLOATING)));
    napi_set_named_property(env, objValue, "SPLIT_SCREEN", CreateJsValue(env,
        static_cast<int32_t>(WindowStatus::WINDOW_STATUS_SPLITSCREEN)));
    return objValue;
}

napi_value RectChangeReasonInit(napi_env env)
{
    TLOGD(WmsLogTag::WMS_LAYOUT, "RectChangeReasonInit called");

    CHECK_NAPI_ENV_RETURN_IF_NULL(env);

    napi_value objValue = nullptr;
    CHECK_NAPI_CREATE_OBJECT_RETURN_IF_NULL(env, objValue);

    napi_set_named_property(env, objValue, "UNDEFINED",
        CreateJsValue(env, static_cast<uint32_t>(RectChangeReason::UNDEFINED)));
    napi_set_named_property(env, objValue, "MAXIMIZE",
        CreateJsValue(env, static_cast<uint32_t>(RectChangeReason::MAXIMIZE)));
    napi_set_named_property(env, objValue, "RECOVER",
        CreateJsValue(env, static_cast<uint32_t>(RectChangeReason::RECOVER)));
    napi_set_named_property(env, objValue, "MOVE",
        CreateJsValue(env, static_cast<uint32_t>(RectChangeReason::MOVE)));
    napi_set_named_property(env, objValue, "DRAG",
        CreateJsValue(env, static_cast<uint32_t>(RectChangeReason::DRAG)));
    napi_set_named_property(env, objValue, "DRAG_START",
        CreateJsValue(env, static_cast<uint32_t>(RectChangeReason::DRAG_START)));
    napi_set_named_property(env, objValue, "DRAG_END",
        CreateJsValue(env, static_cast<uint32_t>(RectChangeReason::DRAG_END)));
    return objValue;
}

napi_value GetRectAndConvertToJsValue(napi_env env, const Rect& rect)
{
    napi_value objValue = nullptr;
    CHECK_NAPI_CREATE_OBJECT_RETURN_IF_NULL(env, objValue);

    napi_set_named_property(env, objValue, "left", CreateJsValue(env, rect.posX_));
    napi_set_named_property(env, objValue, "top", CreateJsValue(env, rect.posY_));
    napi_set_named_property(env, objValue, "width", CreateJsValue(env, rect.width_));
    napi_set_named_property(env, objValue, "height", CreateJsValue(env, rect.height_));
    return objValue;
}

napi_value CreateJsWindowPropertiesObject(napi_env env, sptr<Window>& window, const Rect& drawableRect)
{
    WLOGD("CreateJsWindowPropertiesObject");
    napi_value objValue = nullptr;
    CHECK_NAPI_CREATE_OBJECT_RETURN_IF_NULL(env, objValue);

    Rect windowRect = window->GetRect();
    napi_value windowRectObj = GetRectAndConvertToJsValue(env, windowRect);
    if (windowRectObj == nullptr) {
        WLOGFE("GetWindowRect failed!");
    }
    napi_set_named_property(env, objValue, "windowRect", windowRectObj);

    napi_value drawableRectObj = GetRectAndConvertToJsValue(env, drawableRect);
    if (drawableRectObj == nullptr) {
        WLOGFE("GetDrawableRect failed!");
    }
    napi_set_named_property(env, objValue, "drawableRect", drawableRectObj);
    
    WindowType type = window->GetType();
    if (NATIVE_JS_TO_WINDOW_TYPE_MAP.count(type) != 0) {
        napi_set_named_property(env, objValue, "type", CreateJsValue(env, NATIVE_JS_TO_WINDOW_TYPE_MAP.at(type)));
    } else {
        napi_set_named_property(env, objValue, "type", CreateJsValue(env, type));
    }
    napi_set_named_property(env, objValue, "isLayoutFullScreen", CreateJsValue(env, window->IsLayoutFullScreen()));
    napi_set_named_property(env, objValue, "isFullScreen", CreateJsValue(env, window->IsFullScreen()));
    napi_set_named_property(env, objValue, "touchable", CreateJsValue(env, window->GetTouchable()));
    napi_set_named_property(env, objValue, "focusable", CreateJsValue(env, window->GetFocusable()));
    napi_set_named_property(env, objValue, "name", CreateJsValue(env, window->GetWindowName()));
    napi_set_named_property(env, objValue, "isPrivacyMode", CreateJsValue(env, window->IsPrivacyMode()));
    napi_set_named_property(env, objValue, "isKeepScreenOn", CreateJsValue(env, window->IsKeepScreenOn()));
    napi_set_named_property(env, objValue, "brightness", CreateJsValue(env, window->GetBrightness()));
    napi_set_named_property(env, objValue, "isTransparent", CreateJsValue(env, window->IsTransparent()));
    napi_set_named_property(env, objValue, "isRoundCorner", CreateJsValue(env, false)); // empty method
    napi_set_named_property(env, objValue, "dimBehindValue", CreateJsValue(env, 0));
    napi_set_named_property(env, objValue, "id", CreateJsValue(env, window->GetWindowId()));
    napi_set_named_property(env, objValue, "displayId", CreateJsValue(env,
        static_cast<int64_t>(window->GetDisplayId())));
    return objValue;
}

static std::string GetHexColor(uint32_t color)
{
    std::stringstream ioss;
    std::string temp;
    ioss << std::setiosflags(std::ios::uppercase) << std::hex << color;
    ioss >> temp;
    int count = RGBA_LENGTH - static_cast<int>(temp.length());
    std::string tmpColor(count, '0');
    tmpColor += temp;
    std::string finalColor("#");
    finalColor += tmpColor;
    return finalColor;
}

napi_value CreateJsSystemBarPropertiesObject(napi_env env, sptr<Window>& window)
{
    napi_value objValue = nullptr;
    CHECK_NAPI_CREATE_OBJECT_RETURN_IF_NULL(env, objValue);

    SystemBarProperty status = window->GetSystemBarPropertyByType(WindowType::WINDOW_TYPE_STATUS_BAR);
    SystemBarProperty navi = window->GetSystemBarPropertyByType(WindowType::WINDOW_TYPE_NAVIGATION_BAR);
    napi_set_named_property(env, objValue, "statusBarColor",
        CreateJsValue(env, GetHexColor(status.backgroundColor_)));
    napi_set_named_property(env, objValue, "statusBarContentColor",
        CreateJsValue(env, GetHexColor(status.contentColor_)));
    napi_set_named_property(env, objValue, "isStatusBarLightIcon",
        CreateJsValue(env, status.contentColor_ == SYSTEM_COLOR_WHITE));
    napi_set_named_property(env, objValue, "navigationBarColor",
        CreateJsValue(env, GetHexColor(navi.backgroundColor_)));
    napi_set_named_property(env, objValue, "navigationBarContentColor",
        CreateJsValue(env, GetHexColor(navi.contentColor_)));
    napi_set_named_property(env, objValue, "isNavigationBarLightIcon",
        CreateJsValue(env, navi.contentColor_ == SYSTEM_COLOR_WHITE));
    napi_set_named_property(env, objValue, "enableStatusBarAnimation",
                            CreateJsValue(env, status.enableAnimation_));
    napi_set_named_property(env, objValue, "enableNavigationBarAnimation",
                            CreateJsValue(env, navi.enableAnimation_));
    return objValue;
}

static napi_value CreateJsSystemBarRegionTintObject(napi_env env, const SystemBarRegionTint& tint)
{
    napi_value objValue = nullptr;
    CHECK_NAPI_CREATE_OBJECT_RETURN_IF_NULL(env, objValue);

    if (NATIVE_JS_TO_WINDOW_TYPE_MAP.count(tint.type_) != 0) {
        napi_set_named_property(env, objValue, "type", CreateJsValue(env, NATIVE_JS_TO_WINDOW_TYPE_MAP.at(tint.type_)));
    } else {
        napi_set_named_property(env, objValue, "type", CreateJsValue(env, tint.type_));
    }
    napi_set_named_property(env, objValue, "isEnable", CreateJsValue(env, tint.prop_.enable_));
    std::string bkgColor = GetHexColor(tint.prop_.backgroundColor_);
    napi_set_named_property(env, objValue, "backgroundColor", CreateJsValue(env, bkgColor));
    std::string contentColor = GetHexColor(tint.prop_.contentColor_);
    napi_set_named_property(env, objValue, "contentColor", CreateJsValue(env, contentColor));
    Rect rect = tint.region_;
    napi_set_named_property(env, objValue, "region", GetRectAndConvertToJsValue(env, rect));

    WLOGFD("Type %{public}u [%{public}u %{public}s %{public}s]",
        tint.type_, tint.prop_.enable_, bkgColor.c_str(), contentColor.c_str());
    WLOGFD("Region [%{public}d %{public}d %{public}u %{public}u]",
        rect.posX_, rect.posY_, rect.width_, rect.height_);
    return objValue;
}

napi_value CreateJsWindowInfoArrayObject(napi_env env, const std::vector<sptr<WindowVisibilityInfo>>& infos)
{
    napi_value arrayValue = nullptr;
    napi_create_array_with_length(env, infos.size(), &arrayValue);
    if (arrayValue == nullptr) {
        WLOGFE("[NAPI]Failed to convert windowVisibilityInfo to jsArrayObject");
        return nullptr;
    }
    uint32_t index = 0;
    for (size_t i = 0; i < infos.size(); i++) {
        auto info = infos[i];
        auto windowType = info->GetWindowType();
        auto windowVisibilityState = info->GetWindowVisibilityState();
        if (windowType >= WindowType::APP_MAIN_WINDOW_BASE && windowType < WindowType::APP_MAIN_WINDOW_END &&
            windowVisibilityState != WindowVisibilityState::WINDOW_VISIBILITY_STATE_TOTALLY_OCCUSION) {
            napi_set_element(env, arrayValue, index++, CreateJsWindowInfoObject(env, info));
        }
    }
    return arrayValue;
}

napi_value CreateJsWindowInfoObject(napi_env env, const sptr<WindowVisibilityInfo>& info)
{
    napi_value objValue = nullptr;
    CHECK_NAPI_CREATE_OBJECT_RETURN_IF_NULL(env, objValue);
    napi_set_named_property(env, objValue, "rect", GetRectAndConvertToJsValue(env, info->GetRect()));
    napi_set_named_property(env, objValue, "bundleName", CreateJsValue(env, info->GetBundleName()));
    napi_set_named_property(env, objValue, "abilityName", CreateJsValue(env, info->GetAbilityName()));
    napi_set_named_property(env, objValue, "windowId", CreateJsValue(env, info->GetWindowId()));
    napi_set_named_property(env, objValue, "windowStatusType",
        CreateJsValue(env, static_cast<int32_t>(info->GetWindowStatus())));
    napi_set_named_property(env, objValue, "isFocused", CreateJsValue(env, info->IsFocused()));
    return objValue;
}

napi_value CreateJsSystemBarRegionTintArrayObject(napi_env env, const SystemBarRegionTints& tints)
{
    if (tints.empty()) {
        WLOGFE("Empty tints");
        return nullptr;
    }
    napi_value objValue = nullptr;
    napi_create_array_with_length(env, tints.size(), &objValue);
    napi_value array = objValue;
    if (array == nullptr) {
        WLOGFE("Failed to convert SystemBarProperties to jsArrayObject");
        return nullptr;
    }
    uint32_t index = 0;
    for (size_t i = 0; i < tints.size(); i++) {
        napi_set_element(env, array, index++, CreateJsSystemBarRegionTintObject(env, tints[i]));
    }
    return objValue;
}

bool GetSystemBarStatus(napi_env env, napi_callback_info info,
    std::map<WindowType, SystemBarProperty>& systemBarProperties,
    std::map<WindowType, SystemBarPropertyFlag>& systemBarpropertyFlags)
{
    size_t argc = FOUR_PARAMS_SIZE;
    napi_value argv[FOUR_PARAMS_SIZE] = { nullptr };
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    uint32_t size = INDEX_ZERO;
    napi_value nativeArray = nullptr;
    if (argc > ARG_COUNT_ZERO && GetType(env, argv[INDEX_ZERO]) != napi_function) {
        nativeArray = argv[INDEX_ZERO];
        if (nativeArray == nullptr) {
            TLOGE(WmsLogTag::WMS_IMMS, "Failed to convert parameter to SystemBarArray");
            return false;
        }
        napi_get_array_length(env, nativeArray, &size);
    }
    systemBarProperties[WindowType::WINDOW_TYPE_STATUS_BAR].enable_ = false;
    systemBarProperties[WindowType::WINDOW_TYPE_NAVIGATION_INDICATOR].enable_ = false;
    systemBarProperties[WindowType::WINDOW_TYPE_NAVIGATION_BAR].enable_ = false;
    for (uint32_t i = 0; i < size; i++) {
        std::string name;
        napi_value getElementValue = nullptr;
        napi_get_element(env, nativeArray, i, &getElementValue);
        if (!ConvertFromJsValue(env, getElementValue, name)) {
            TLOGE(WmsLogTag::WMS_IMMS, "Failed to convert parameter to SystemBarName");
            return false;
        }
        if (name.compare("status") == 0) {
            systemBarProperties[WindowType::WINDOW_TYPE_STATUS_BAR].enable_ = true;
            systemBarProperties[WindowType::WINDOW_TYPE_NAVIGATION_INDICATOR].enable_ = true;
        } else if (name.compare("navigation") == 0) {
            systemBarProperties[WindowType::WINDOW_TYPE_NAVIGATION_BAR].enable_ = true;
        }
    }
    systemBarpropertyFlags[WindowType::WINDOW_TYPE_STATUS_BAR].enableFlag = true;
    systemBarpropertyFlags[WindowType::WINDOW_TYPE_NAVIGATION_BAR].enableFlag = true;
    systemBarpropertyFlags[WindowType::WINDOW_TYPE_NAVIGATION_INDICATOR].enableFlag = true;
    return true;
}

bool ParseAndCheckRect(napi_env env, napi_value jsObject,
    const Rect& windowRect, Rect& touchableRect)
{
    int32_t data = 0;
    if (ParseJsValue(jsObject, env, "left", data)) {
        touchableRect.posX_ = data;
    } else {
        TLOGE(WmsLogTag::WMS_EVENT, "Failed to convert object:legt");
        return false;
    }
    if (ParseJsValue(jsObject, env, "top", data)) {
        touchableRect.posY_ = data;
    } else {
        TLOGE(WmsLogTag::WMS_EVENT, "Failed to convert object:top");
        return false;
    }
    uint32_t udata = 0;
    if (ParseJsValue(jsObject, env, "width", udata)) {
        touchableRect.width_ = udata;
    } else {
        TLOGE(WmsLogTag::WMS_EVENT, "Failed to convert object:width");
        return false;
    }
    if (ParseJsValue(jsObject, env, "height", udata)) {
        touchableRect.height_ = udata;
    } else {
        TLOGE(WmsLogTag::WMS_EVENT, "Failed to convert object:height");
        return false;
    }
    if ((touchableRect.posX_ < 0) || (touchableRect.posY_ < 0) ||
        (touchableRect.posX_ > static_cast<int32_t>(windowRect.width_)) ||
        (touchableRect.posY_ > static_cast<int32_t>(windowRect.height_)) ||
        (touchableRect.width_ > (windowRect.width_ - static_cast<uint32_t>(touchableRect.posX_))) ||
        (touchableRect.height_ > (windowRect.height_ - static_cast<uint32_t>(touchableRect.posY_)))) {
        TLOGE(WmsLogTag::WMS_EVENT, "Outside the window area");
        return false;
    }
    return true;
}

WmErrorCode ParseTouchableAreas(napi_env env, napi_callback_info info,
    const Rect& windowRect, std::vector<Rect>& touchableAreas)
{
    WmErrorCode errCode = WmErrorCode::WM_ERROR_INVALID_PARAM;
    size_t argc = 4;
    napi_value argv[4] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc != 1) {
        TLOGE(WmsLogTag::WMS_EVENT, "Argc is invalid:%{public}zu", argc);
        return errCode;
    }
    if (GetType(env, argv[0]) != napi_object) {
        TLOGE(WmsLogTag::WMS_EVENT, "GetType error");
        return errCode;
    }
    napi_value nativeArray = argv[0];
    if (nativeArray == nullptr) {
        TLOGE(WmsLogTag::WMS_EVENT, "Failed to convert parameter");
        return errCode;
    }
    uint32_t size = 0;
    napi_get_array_length(env, nativeArray, &size);
    if (size > MAX_TOUCHABLE_AREAS) {
        TLOGE(WmsLogTag::WMS_EVENT, "Exceeded maximum limit");
        return errCode;
    }
    errCode = WmErrorCode::WM_OK;
    for (uint32_t i = 0; i < size; i++) {
        napi_value getElementValue = nullptr;
        napi_get_element(env, nativeArray, i, &getElementValue);
        if (getElementValue == nullptr) {
            TLOGE(WmsLogTag::WMS_EVENT, "Failed to get element");
            errCode = WmErrorCode::WM_ERROR_INVALID_PARAM;
            break;
        }
        Rect touchableArea;
        if (ParseAndCheckRect(env, getElementValue, windowRect, touchableArea)) {
            touchableAreas.emplace_back(touchableArea);
        } else {
            errCode = WmErrorCode::WM_ERROR_INVALID_PARAM;
            break;
        }
    }
    return errCode;
}

bool GetSpecificBarStatus(napi_env env, napi_callback_info info, bool& systemBarEnable, bool& systemBarEnableAnimation)
{
    size_t argc = FOUR_PARAMS_SIZE;
    napi_value argv[FOUR_PARAMS_SIZE] = { nullptr };
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc < ARG_COUNT_TWO || !ConvertFromJsValue(env, argv[INDEX_ONE], systemBarEnable)) {
        TLOGE(WmsLogTag::WMS_IMMS, "Argc is invalid: %{public}zu or Failed to convert enable parameter to bool", argc);
        return false;
    }
    if (argc >= ARG_COUNT_THREE && !ConvertFromJsValue(env, argv[INDEX_TWO], systemBarEnableAnimation)) {
        TLOGE(WmsLogTag::WMS_IMMS, "Failed to convert enableAnimation parameter to bool");
        return false;
    }
    return true;
}

static uint32_t GetColorFromJs(napi_env env, napi_value jsObject,
    const char* name, uint32_t defaultColor, bool& flag)
{
    napi_value jsColor = nullptr;
    napi_get_named_property(env, jsObject, name, &jsColor);
    if (GetType(env, jsColor) != napi_undefined) {
        std::string colorStr;
        if (!ConvertFromJsValue(env, jsColor, colorStr)) {
            WLOGFE("Failed to convert parameter to color");
            return defaultColor;
        }
        std::regex pattern("^#([A-Fa-f0-9]{6}|[A-Fa-f0-9]{8})$");
        if (!std::regex_match(colorStr, pattern)) {
            WLOGFD("Invalid color input");
            return defaultColor;
        }
        std::string color = colorStr.substr(1);
        if (color.length() == RGB_LENGTH) {
            color = "FF" + color; // ARGB
        }
        flag = true;
        std::stringstream ss;
        uint32_t hexColor;
        ss << std::hex << color;
        ss >> hexColor;
        WLOGI("Origin %{public}s, process %{public}s, final %{public}x",
            colorStr.c_str(), color.c_str(), hexColor);
        return hexColor;
    }
    return defaultColor;
}

bool SetWindowStatusBarContentColor(napi_env env, napi_value jsObject,
    std::map<WindowType, SystemBarProperty>& properties, std::map<WindowType, SystemBarPropertyFlag>& propertyFlags)
{
    auto statusProperty = properties[WindowType::WINDOW_TYPE_STATUS_BAR];
    napi_value jsStatusContentColor = nullptr;
    napi_get_named_property(env, jsObject, "statusBarContentColor", &jsStatusContentColor);
    napi_value jsStatusIcon = nullptr;
    napi_get_named_property(env, jsObject, "isStatusBarLightIcon", &jsStatusIcon);
    if (GetType(env, jsStatusContentColor) != napi_undefined) {
        properties[WindowType::WINDOW_TYPE_STATUS_BAR].contentColor_ =  GetColorFromJs(env,
            jsObject, "statusBarContentColor", statusProperty.contentColor_,
            propertyFlags[WindowType::WINDOW_TYPE_STATUS_BAR].contentColorFlag);
    } else if (GetType(env, jsStatusIcon) != napi_undefined) {
        bool isStatusBarLightIcon;
        if (!ConvertFromJsValue(env, jsStatusIcon, isStatusBarLightIcon)) {
            TLOGE(WmsLogTag::DEFAULT, "Convert status icon value failed");
            return false;
        }
        if (isStatusBarLightIcon) {
            properties[WindowType::WINDOW_TYPE_STATUS_BAR].contentColor_ = SYSTEM_COLOR_WHITE;
        } else {
            properties[WindowType::WINDOW_TYPE_STATUS_BAR].contentColor_ = SYSTEM_COLOR_BLACK;
        }
        propertyFlags[WindowType::WINDOW_TYPE_STATUS_BAR].contentColorFlag = true;
    }
    return true;
}

bool SetWindowNavigationBarContentColor(napi_env env, napi_value jsObject,
    std::map<WindowType, SystemBarProperty>& properties, std::map<WindowType, SystemBarPropertyFlag>& propertyFlags)
{
    auto navProperty = properties[WindowType::WINDOW_TYPE_NAVIGATION_BAR];
    napi_value jsNavigationContentColor = nullptr;
    napi_get_named_property(env, jsObject, "navigationBarContentColor", &jsNavigationContentColor);
    napi_value jsNavigationIcon = nullptr;
    napi_get_named_property(env, jsObject, "isNavigationBarLightIcon", &jsNavigationIcon);
    if (GetType(env, jsNavigationContentColor) != napi_undefined) {
        properties[WindowType::WINDOW_TYPE_NAVIGATION_BAR].contentColor_ = GetColorFromJs(env,
            jsObject, "navigationBarContentColor", navProperty.contentColor_,
            propertyFlags[WindowType::WINDOW_TYPE_NAVIGATION_BAR].contentColorFlag);
    } else if (GetType(env, jsNavigationIcon) != napi_undefined) {
        bool isNavigationBarLightIcon;
        if (!ConvertFromJsValue(env, jsNavigationIcon, isNavigationBarLightIcon)) {
            TLOGE(WmsLogTag::DEFAULT, "Convert navigation icon value failed");
            return false;
        }
        if (isNavigationBarLightIcon) {
            properties[WindowType::WINDOW_TYPE_NAVIGATION_BAR].contentColor_ = SYSTEM_COLOR_WHITE;
        } else {
            properties[WindowType::WINDOW_TYPE_NAVIGATION_BAR].contentColor_ = SYSTEM_COLOR_BLACK;
        }
        propertyFlags[WindowType::WINDOW_TYPE_NAVIGATION_BAR].contentColorFlag = true;
    }
    return true;
}

bool GetSystemBarPropertiesFromJs(napi_env env, napi_value jsObject,
    std::map<WindowType, SystemBarProperty>& properties,
    std::map<WindowType, SystemBarPropertyFlag>& propertyFlags)
{
    properties[WindowType::WINDOW_TYPE_STATUS_BAR].backgroundColor_ =
        GetColorFromJs(env, jsObject, "statusBarColor",
            properties[WindowType::WINDOW_TYPE_STATUS_BAR].backgroundColor_,
            propertyFlags[WindowType::WINDOW_TYPE_STATUS_BAR].backgroundColorFlag);
    properties[WindowType::WINDOW_TYPE_NAVIGATION_BAR].backgroundColor_ =
        GetColorFromJs(env, jsObject, "navigationBarColor",
            properties[WindowType::WINDOW_TYPE_NAVIGATION_BAR].backgroundColor_,
            propertyFlags[WindowType::WINDOW_TYPE_NAVIGATION_BAR].backgroundColorFlag);

    if (!SetWindowStatusBarContentColor(env, jsObject, properties, propertyFlags) ||
        !SetWindowNavigationBarContentColor(env, jsObject, properties, propertyFlags)) {
        return false;
    }
    bool enableStatusBarAnimation = false;
    if (ParseJsValue(jsObject, env, "enableStatusBarAnimation", enableStatusBarAnimation)) {
        properties[WindowType::WINDOW_TYPE_STATUS_BAR].enableAnimation_ = enableStatusBarAnimation;
        propertyFlags[WindowType::WINDOW_TYPE_STATUS_BAR].enableAnimationFlag = true;
    }
    bool enableNavigationBarAnimation = false;
    if (ParseJsValue(jsObject, env, "enableNavigationBarAnimation", enableNavigationBarAnimation)) {
        properties[WindowType::WINDOW_TYPE_NAVIGATION_BAR].enableAnimation_ = enableNavigationBarAnimation;
        propertyFlags[WindowType::WINDOW_TYPE_NAVIGATION_BAR].enableAnimationFlag = true;
    }
    return true;
}

void ConvertJSSystemBarStyleToSystemBarProperties(napi_env env, napi_value jsObject,
    std::map<WindowType, SystemBarProperty>& properties, std::map<WindowType, SystemBarPropertyFlag>& propertyFlags)
{
    properties[WindowType::WINDOW_TYPE_STATUS_BAR].contentColor_ = GetColorFromJs(env,
        jsObject, "statusBarContentColor",
        properties[WindowType::WINDOW_TYPE_STATUS_BAR].contentColor_,
        propertyFlags[WindowType::WINDOW_TYPE_STATUS_BAR].contentColorFlag);
}

napi_value ConvertAvoidAreaToJsValue(napi_env env, const AvoidArea& avoidArea, AvoidAreaType type)
{
    napi_value objValue = nullptr;
    CHECK_NAPI_CREATE_OBJECT_RETURN_IF_NULL(env, objValue);

    napi_set_named_property(env, objValue, "visible",
        CreateJsValue(env, type == AvoidAreaType::TYPE_CUTOUT ? false : true));
    napi_set_named_property(env, objValue, "leftRect", GetRectAndConvertToJsValue(env, avoidArea.leftRect_));
    napi_set_named_property(env, objValue, "topRect", GetRectAndConvertToJsValue(env, avoidArea.topRect_));
    napi_set_named_property(env, objValue, "rightRect", GetRectAndConvertToJsValue(env, avoidArea.rightRect_));
    napi_set_named_property(env, objValue, "bottomRect", GetRectAndConvertToJsValue(env, avoidArea.bottomRect_));
    return objValue;
}

napi_value GetWindowLimitsAndConvertToJsValue(napi_env env, const WindowLimits& windowLimits)
{
    napi_value objValue = nullptr;
    CHECK_NAPI_CREATE_OBJECT_RETURN_IF_NULL(env, objValue);

    napi_set_named_property(env, objValue, "maxWidth", CreateJsValue(env, windowLimits.maxWidth_));
    napi_set_named_property(env, objValue, "maxHeight", CreateJsValue(env, windowLimits.maxHeight_));
    napi_set_named_property(env, objValue, "minWidth", CreateJsValue(env, windowLimits.minWidth_));
    napi_set_named_property(env, objValue, "minHeight", CreateJsValue(env, windowLimits.minHeight_));
    return objValue;
}

napi_value ConvertTitleButtonAreaToJsValue(napi_env env, const TitleButtonRect& titleButtonRect)
{
    napi_value objValue = nullptr;
    CHECK_NAPI_CREATE_OBJECT_RETURN_IF_NULL(env, objValue);

    napi_set_named_property(env, objValue, "right", CreateJsValue(env, titleButtonRect.posX_));
    napi_set_named_property(env, objValue, "top", CreateJsValue(env, titleButtonRect.posY_));
    napi_set_named_property(env, objValue, "width", CreateJsValue(env, titleButtonRect.width_));
    napi_set_named_property(env, objValue, "height", CreateJsValue(env, titleButtonRect.height_));
    return objValue;
}

bool CheckCallingPermission(std::string permission)
{
    WLOGD("Permission: %{public}s", permission.c_str());
    if (!permission.empty() &&
        Security::AccessToken::AccessTokenKit::VerifyAccessToken(IPCSkeleton::GetCallingTokenID(), permission)
        != AppExecFwk::Constants::PERMISSION_GRANTED) {
        WLOGFE("Permission %{public}s is not granted", permission.c_str());
        return false;
    }
    return true;
}

bool ParseSystemWindowTypeForApiWindowType(int32_t apiWindowType, WindowType& windowType)
{
    if (JS_TO_NATIVE_WINDOW_TYPE_MAP.count(static_cast<ApiWindowType>(apiWindowType)) != 0) {
        windowType = JS_TO_NATIVE_WINDOW_TYPE_MAP.at(static_cast<ApiWindowType>(apiWindowType));
        if (WindowHelper::IsSystemWindow(windowType)) {
            return true;
        }
    }
    return false;
}


bool GetAPI7Ability(napi_env env, AppExecFwk::Ability* &ability)
{
    napi_value global;
    if (napi_get_global(env, &global) != napi_ok) {
        WLOGI("Get global failed");
        return false;
    }
    napi_value jsAbility;
    napi_status status = napi_get_named_property(env, global, "ability", &jsAbility);
    if (status != napi_ok || jsAbility == nullptr) {
        WLOGI("Get ability property failed");
        return false;
    }

    if (napi_get_value_external(env, jsAbility, reinterpret_cast<void **>(&ability)) != napi_ok) {
        WLOGI("Get ability external failed");
        return false;
    }
    if (ability == nullptr) {
        return false;
    } else {
        WLOGI("Get ability");
    }
    return true;
}
bool GetWindowMaskFromJsValue(napi_env env, napi_value jsObject, std::vector<std::vector<uint32_t>>& windowMask)
{
    if (jsObject == nullptr) {
        WLOGFE("Failed to convert parameter to window mask");
        return false;
    }
    uint32_t size = 0;
    napi_get_array_length(env, jsObject, &size);
    for (uint32_t i = 0; i < size; i++) {
        std::vector<uint32_t> elementArray;
        napi_value getElementValue = nullptr;
        napi_get_element(env, jsObject, i, &getElementValue);
        if (!ConvertNativeValueToVector(env, getElementValue, elementArray)) {
            WLOGFE("Failed to convert parameter to window mask");
            return false;
        }
        windowMask.emplace_back(elementArray);
    }
    return true;
}

std::unique_ptr<NapiAsyncTask> CreateAsyncTask(napi_env env, napi_value lastParam,
    std::unique_ptr<NapiAsyncTask::ExecuteCallback>&& execute,
    std::unique_ptr<NapiAsyncTask::CompleteCallback>&& complete, napi_value* result)
{
    napi_valuetype type = napi_undefined;
    napi_typeof(env, lastParam, &type);
    if (lastParam == nullptr || type != napi_function) {
        napi_deferred nativeDeferred = nullptr;
        NAPI_CALL(env, napi_create_promise(env, &nativeDeferred, result));
        return std::make_unique<NapiAsyncTask>(nativeDeferred, std::move(execute), std::move(complete));
    } else {
        napi_get_undefined(env, result);
        napi_ref callbackRef = nullptr;
        napi_create_reference(env, lastParam, 1, &callbackRef);
        return std::make_unique<NapiAsyncTask>(callbackRef, std::move(execute), std::move(complete));
    }
}

std::unique_ptr<NapiAsyncTask> CreateEmptyAsyncTask(napi_env env,
    napi_value lastParam, napi_value* result)
{
    napi_valuetype type = napi_undefined;
    napi_typeof(env, lastParam, &type);
    if (lastParam == nullptr || type != napi_function) {
        napi_deferred nativeDeferred = nullptr;
        napi_create_promise(env, &nativeDeferred, result);
        return std::make_unique<NapiAsyncTask>(nativeDeferred,
            std::unique_ptr<NapiAsyncTask::ExecuteCallback>(),
            std::unique_ptr<NapiAsyncTask::CompleteCallback>());
    } else {
        napi_get_undefined(env, result);
        napi_ref callbackRef = nullptr;
        napi_create_reference(env, lastParam, 1, &callbackRef);
        return std::make_unique<NapiAsyncTask>(callbackRef,
            std::unique_ptr<NapiAsyncTask::ExecuteCallback>(),
            std::unique_ptr<NapiAsyncTask::CompleteCallback>());
    }
}

napi_value ModalityTypeInit(napi_env env)
{
    CHECK_NAPI_ENV_RETURN_IF_NULL(env);
    napi_value objValue = nullptr;
    CHECK_NAPI_CREATE_OBJECT_RETURN_IF_NULL(env, objValue);

    napi_set_named_property(env, objValue, "WINDOW_MODALITY",
        CreateJsValue(env, ApiModalityType::WINDOW_MODALITY));
    napi_set_named_property(env, objValue, "APPLICATION_MODALITY",
        CreateJsValue(env, ApiModalityType::APPLICATION_MODALITY));
    return objValue;
}

static bool ParseModalityParam(napi_env env, napi_value jsObject, const sptr<WindowOption>& windowOption)
{
    bool isModal = false;
    if (ParseJsValue(jsObject, env, "isModal", isModal)) {
        if (isModal) {
            windowOption->AddWindowFlag(WindowFlag::WINDOW_FLAG_IS_MODAL);
        }
    }
    bool isTopmost = false;
    if (ParseJsValue(jsObject, env, "isTopmost", isTopmost)) {
        if (!isModal && isTopmost) {
            TLOGE(WmsLogTag::WMS_SUB, "Normal subwindow not support topmost");
            return false;
        }
        windowOption->SetWindowTopmost(isTopmost);
    }
    ApiModalityType apiModalityType;
    if (ParseJsValue(jsObject, env, "modalityType", apiModalityType)) {
        if (!isModal) {
            TLOGE(WmsLogTag::WMS_SUB, "Normal subwindow not support modalityType");
            return false;
        }
        using T = std::underlying_type_t<ApiModalityType>;
        T modalityType = static_cast<T>(apiModalityType);
        if (modalityType >= static_cast<T>(ApiModalityType::BEGIN) &&
            modalityType <= static_cast<T>(ApiModalityType::END)) {
            auto type = JS_TO_NATIVE_MODALITY_TYPE_MAP.at(apiModalityType);
            if (type == ModalityType::APPLICATION_MODALITY) {
                windowOption->AddWindowFlag(WindowFlag::WINDOW_FLAG_IS_APPLICATION_MODAL);
            }
        } else {
            TLOGE(WmsLogTag::WMS_SUB, "Failed to convert parameter to modalityType");
            return false;
        }
    }
    return true;
}

bool ParseSubWindowOptions(napi_env env, napi_value jsObject, const sptr<WindowOption>& windowOption)
{
    if (jsObject == nullptr || windowOption == nullptr) {
        TLOGE(WmsLogTag::WMS_SUB, "jsObject or windowOption is null");
        return false;
    }
    std::string title;
    if (!ParseJsValue(jsObject, env, "title", title)) {
        TLOGE(WmsLogTag::WMS_SUB, "Failed to convert parameter to title");
        return false;
    }
    bool decorEnabled = false;
    if (!ParseJsValue(jsObject, env, "decorEnabled", decorEnabled)) {
        TLOGE(WmsLogTag::WMS_SUB, "Failed to convert parameter to decorEnabled");
        return false;
    }

    windowOption->SetSubWindowTitle(title);
    windowOption->SetSubWindowDecorEnable(decorEnabled);
    return ParseModalityParam(env, jsObject, windowOption);
}

napi_value ExtensionWindowAttributeInit(napi_env env)
{
    if (env == nullptr) {
        TLOGE(WmsLogTag::WMS_UIEXT, "env is nullptr");
        return nullptr;
    }
    napi_value objValue = nullptr;
    napi_create_object(env, &objValue);
    if (objValue == nullptr) {
        TLOGE(WmsLogTag::WMS_UIEXT, "Failed to create object");
        return nullptr;
    }
    napi_set_named_property(env, objValue, "SYSTEM_WINDOW",
        CreateJsValue(env, static_cast<int32_t>(ExtensionWindowAttribute::SYSTEM_WINDOW)));
    napi_set_named_property(env, objValue, "SUB_WINDOW",
        CreateJsValue(env, static_cast<int32_t>(ExtensionWindowAttribute::SUB_WINDOW)));
    return objValue;
}
} // namespace Rosen
} // namespace OHOS
