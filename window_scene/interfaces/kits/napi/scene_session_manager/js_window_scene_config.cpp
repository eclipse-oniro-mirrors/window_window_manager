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

#include "js_window_scene_config.h"

#include "window_manager_hilog.h"
#include "js_scene_utils.h"

namespace OHOS::Rosen {
using namespace AbilityRuntime;
namespace {
constexpr HiviewDFX::HiLogLabel LABEL = { LOG_CORE, HILOG_DOMAIN_WINDOW, "JsWindowSceneConfig" };
} // namespace

napi_value JsWindowSceneConfig::CreateWindowSceneConfig(napi_env env, const AppWindowSceneConfig& config)
{
    WLOGFI("[NAPI]");
    napi_value objValue = nullptr;
    napi_create_object(env, &objValue);
    if (objValue == nullptr) {
        WLOGFE("Object is null!");
        return NapiGetUndefined(env);
    }

    napi_set_named_property(env, objValue, "floatCornerRadius", CreateJsValue(env, config.floatCornerRadius_));
    napi_set_named_property(env, objValue, "focusedShadow", CreateShadowValue(env, config, true));
    napi_set_named_property(env, objValue, "unfocusedShadow", CreateShadowValue(env, config, false));
    napi_set_named_property(env, objValue, "focusedShadowDark", CreateShadowDarkValue(env, config, true));
    napi_set_named_property(env, objValue, "unfocusedShadowDark", CreateShadowDarkValue(env, config, false));
    napi_set_named_property(env, objValue, "keyboardAnimationIn",
        CreateKeyboardAnimationValue(env, config.keyboardAnimationIn_));
    napi_set_named_property(env, objValue, "keyboardAnimationOut",
        CreateKeyboardAnimationValue(env, config.keyboardAnimationOut_));
    napi_set_named_property(env, objValue, "windowAnimation", CreateWindowAnimationValue(env, config));
    napi_set_named_property(env, objValue, "systemUIStatusBar", CreateSystemUIStatusBarValue(env,
        config.systemUIStatusBarConfig_));
    napi_set_named_property(env, objValue, "multiWindowUIType", CreateJsValue(env, config.multiWindowUIType_));
    napi_set_named_property(env, objValue, "backgroundScreenLock", CreateJsValue(env, config.backgroundScreenLock_));
    napi_set_named_property(env, objValue, "rotationMode", CreateJsValue(env, config.rotationMode_));
    napi_set_named_property(env, objValue, "desktopStatusBarConfig",
        CreateWindowStatusBar(env, config.windowImmersive_.desktopStatusBarConfig_));
    napi_set_named_property(env, objValue, "leftRightStatusBarConfig",
        CreateWindowStatusBar(env, config.windowImmersive_.leftRightStatusBarConfig_));
    napi_set_named_property(env, objValue, "upDownStatusBarConfig",
        CreateWindowStatusBar(env, config.windowImmersive_.upDownStatusBarConfig_));
    return objValue;
}

napi_value JsWindowSceneConfig::CreateShadowValue(napi_env env, const AppWindowSceneConfig& config,
    bool focused)
{
    napi_value objValue = nullptr;
    napi_create_object(env, &objValue);
    if (objValue == nullptr) {
        WLOGFE("Object is null!");
        return NapiGetUndefined(env);
    }

    napi_set_named_property(env, objValue, "offsetX", CreateJsValue(env,
        focused ? config.focusedShadow_.offsetX_ : config.unfocusedShadow_.offsetX_));
    napi_set_named_property(env, objValue, "offsetY", CreateJsValue(env,
        focused ? config.focusedShadow_.offsetY_ : config.unfocusedShadow_.offsetY_));
    napi_set_named_property(env, objValue, "radius", CreateJsValue(env,
        focused ? config.focusedShadow_.radius_ : config.unfocusedShadow_.radius_));
    napi_set_named_property(env, objValue, "color", CreateJsValue(env,
        focused ? config.focusedShadow_.color_ : config.unfocusedShadow_.color_));

    return objValue;
}

napi_value JsWindowSceneConfig::CreateShadowDarkValue(napi_env env, const AppWindowSceneConfig& config,
    bool focused)
{
    napi_value objValue = nullptr;
    napi_create_object(env, &objValue);
    if (objValue == nullptr) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "Object is null!");
        return NapiGetUndefined(env);
    }

    napi_set_named_property(env, objValue, "offsetX", CreateJsValue(env,
        focused ? config.focusedShadowDark_.offsetX_ : config.unfocusedShadowDark_.offsetX_));
    napi_set_named_property(env, objValue, "offsetY", CreateJsValue(env,
        focused ? config.focusedShadowDark_.offsetY_ : config.unfocusedShadowDark_.offsetY_));
    napi_set_named_property(env, objValue, "radius", CreateJsValue(env,
        focused ? config.focusedShadowDark_.radius_ : config.unfocusedShadowDark_.radius_));
    napi_set_named_property(env, objValue, "color", CreateJsValue(env,
        focused ? config.focusedShadowDark_.color_ : config.unfocusedShadowDark_.color_));

    return objValue;
}

napi_value JsWindowSceneConfig::CreateWindowAnimationValue(napi_env env, const AppWindowSceneConfig& config)
{
    napi_value objValue = nullptr;
    napi_create_object(env, &objValue);
    if (objValue == nullptr) {
        WLOGFE("Object is null!");
        return NapiGetUndefined(env);
    }
    napi_set_named_property(env, objValue, "duration", CreateJsValue(env, config.windowAnimation_.duration_));
    napi_set_named_property(env, objValue, "curveType", CreateJsValue(env, config.windowAnimation_.curveType_));
    napi_set_named_property(env, objValue, "ctrlX1", CreateJsValue(env, config.windowAnimation_.ctrlX1_));
    napi_set_named_property(env, objValue, "ctrlY1", CreateJsValue(env, config.windowAnimation_.ctrlY1_));
    napi_set_named_property(env, objValue, "ctrlX2", CreateJsValue(env, config.windowAnimation_.ctrlX2_));
    napi_set_named_property(env, objValue, "ctrlY2", CreateJsValue(env, config.windowAnimation_.ctrlY2_));
    napi_set_named_property(env, objValue, "scaleX", CreateJsValue(env, config.windowAnimation_.scaleX_));
    napi_set_named_property(env, objValue, "scaleY", CreateJsValue(env, config.windowAnimation_.scaleY_));
    napi_set_named_property(env, objValue, "rotationX", CreateJsValue(env, config.windowAnimation_.rotationX_));
    napi_set_named_property(env, objValue, "rotationY", CreateJsValue(env, config.windowAnimation_.rotationY_));
    napi_set_named_property(env, objValue, "rotationZ", CreateJsValue(env, config.windowAnimation_.rotationZ_));
    napi_set_named_property(env, objValue, "angle", CreateJsValue(env, config.windowAnimation_.angle_));
    napi_set_named_property(env, objValue, "translateX", CreateJsValue(env, config.windowAnimation_.translateX_));
    napi_set_named_property(env, objValue, "translateY", CreateJsValue(env, config.windowAnimation_.translateY_));
    napi_set_named_property(env, objValue, "opacity", CreateJsValue(env, config.windowAnimation_.opacity_));
    return objValue;
}

napi_value JsWindowSceneConfig::CreateKeyboardAnimationValue(napi_env env,
    const KeyboardSceneAnimationConfig& config)
{
    napi_value objValue = nullptr;
    napi_create_object(env, &objValue);
    if (objValue == nullptr) {
        WLOGFE("Object is null!");
        return NapiGetUndefined(env);
    }

    napi_set_named_property(env, objValue, "curveType", CreateJsValue(env, config.curveType_));
    napi_set_named_property(env, objValue, "ctrlX1", CreateJsValue(env, config.ctrlX1_));
    napi_set_named_property(env, objValue, "ctrlY1", CreateJsValue(env, config.ctrlY1_));
    napi_set_named_property(env, objValue, "ctrlX2", CreateJsValue(env, config.ctrlX2_));
    napi_set_named_property(env, objValue, "ctrlY2", CreateJsValue(env, config.ctrlY2_));
    napi_set_named_property(env, objValue, "duration", CreateJsValue(env, config.duration_));

    return objValue;
}

napi_value JsWindowSceneConfig::CreateSystemUIStatusBarValue(napi_env env,
    const SystemUIStatusBarConfig& config)
{
    napi_value objValue = nullptr;
    napi_create_object(env, &objValue);
    if (objValue == nullptr) {
        WLOGFE("Object is null!");
        return NapiGetUndefined(env);
    }
    napi_set_named_property(env, objValue, "showInLandscapeMode", CreateJsValue(env, config.showInLandscapeMode_));
    napi_set_named_property(env, objValue, "immersiveStatusBarBgColor",
        CreateJsValue(env, config.immersiveStatusBarBgColor_));
    napi_set_named_property(env, objValue, "immersiveStatusBarContentColor",
        CreateJsValue(env, config.immersiveStatusBarContentColor_));
    return objValue;
}

napi_value JsWindowSceneConfig::CreateWindowStatusBar(napi_env env,
    const StatusBarConfig& config)
{
    if (config.backgroundColor_.empty() || config.contentColor_.empty()) {
        WLOGFE("WindowStatusBar is null!");
        return NapiGetUndefined(env);
    }
    napi_value objValue = nullptr;
    napi_create_object(env, &objValue);
    if (objValue == nullptr) {
        WLOGFE("Object is null!");
        return NapiGetUndefined(env);
    }
    napi_set_named_property(env, objValue, "showHide", CreateJsValue(env, config.showHide_));
    napi_set_named_property(env, objValue, "backgroundColor", CreateJsValue(env, config.backgroundColor_));
    napi_set_named_property(env, objValue, "contentColor", CreateJsValue(env, config.contentColor_));
    return objValue;
}

JsWindowSceneConfig::JsWindowSceneConfig()
{
    WLOGFD("Construct JsWindowSceneConfig");
}

JsWindowSceneConfig::~JsWindowSceneConfig()
{
    WLOGFD("Destroy  JsWindowSceneConfig");
}

napi_value JsWindowSceneConfig::CreateFreeMultiWindowConfig(napi_env env, const SystemSessionConfig& config)
{
    TLOGD(WmsLogTag::WMS_LAYOUT_PC, "in");
    napi_value objValue = nullptr;
    napi_create_object(env, &objValue);
    if (objValue == nullptr) {
        TLOGW(WmsLogTag::WMS_LAYOUT_PC, "Object is null!");
        return NapiGetUndefined(env);
    }

    napi_set_named_property(env, objValue, "freeMultiWindowSupport", CreateJsValue(env,
        config.freeMultiWindowSupport_));
    napi_set_named_property(env, objValue, "maxMainFloatingWindowNumber", CreateJsValue(env,
        config.freeMultiWindowConfig_.maxMainFloatingWindowNumber_));
    napi_set_named_property(env, objValue, "defaultWindowMode", CreateJsValue(env,
        config.freeMultiWindowConfig_.defaultWindowMode_));
    return objValue;
}

napi_value JsWindowSceneConfig::CreateWindowLimits(napi_env env, const WindowLimits& windowLimits)
{
    TLOGD(WmsLogTag::WMS_LAYOUT_PC, "in");
    napi_value objValue = nullptr;
    napi_create_object(env, &objValue);
    if (objValue == nullptr) {
        TLOGW(WmsLogTag::WMS_LAYOUT_PC, "Object is null!");
        return NapiGetUndefined(env);
    }
    napi_set_named_property(env, objValue, "maxWidth", CreateJsValue(env,
        windowLimits.maxWidth_));
    napi_set_named_property(env, objValue, "maxHeight", CreateJsValue(env,
        windowLimits.maxHeight_));
    napi_set_named_property(env, objValue, "minWidth", CreateJsValue(env,
        windowLimits.minWidth_));
    napi_set_named_property(env, objValue, "minHeight", CreateJsValue(env,
        windowLimits.minHeight_));
    napi_set_named_property(env, objValue, "pixelUnit", CreateJsValue(env,
        static_cast<uint32_t>(windowLimits.pixelUnit_)));
    napi_set_named_property(env, objValue, "isInitialized", CreateJsValue(env, !windowLimits.IsUninitialized()));
    return objValue;
}

napi_value JsWindowSceneConfig::CreateSingleHandCompatibleConfig(napi_env env,
                                                                 const SingleHandCompatibleModeConfig& config)
{
    TLOGD(WmsLogTag::WMS_LAYOUT, "in");
    napi_value objValue = nullptr;
    napi_create_object(env, &objValue);
    if (objValue == nullptr) {
        TLOGW(WmsLogTag::WMS_LAYOUT, "Object is null!");
        return NapiGetUndefined(env);
    }

    napi_set_named_property(env, objValue, "enabled", CreateJsValue(env, config.enabled));
    napi_set_named_property(env, objValue, "singleHandScale", CreateJsValue(env, config.singleHandScale));
    napi_set_named_property(env, objValue, "heightChangeRatio", CreateJsValue(env, config.heightChangeRatio));
    napi_set_named_property(env, objValue, "widthChangeRatio", CreateJsValue(env, config.widthChangeRatio));
    return objValue;
}

napi_value JsWindowSceneConfig::CreateSystemConfig(napi_env env, const SystemSessionConfig& config)
{
    TLOGI(WmsLogTag::DEFAULT, "in");
    napi_value objValue = nullptr;
    napi_create_object(env, &objValue);
    if (objValue == nullptr) {
        TLOGE(WmsLogTag::DEFAULT, "Object is null!");
        return NapiGetUndefined(env);
    }

    napi_set_named_property(env, objValue, "maxMidSceneNum", CreateJsValue(env,
        config.maxMidSceneNum_));
    napi_set_named_property(env, objValue, "maxFloatingWindowSize", CreateJsValue(env,
        config.maxFloatingWindowSize_));
    return objValue;
}
} // namespace OHOS::Rosen
