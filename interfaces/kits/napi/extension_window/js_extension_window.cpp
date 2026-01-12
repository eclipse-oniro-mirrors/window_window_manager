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

#include "js_extension_window.h"

#include "js_err_utils.h"
#include "js_extension_window_utils.h"
#include "js_runtime_utils.h"
#include "js_window_utils.h"
#include "js_window.h"
#include "window_manager_hilog.h"
#include "wm_common.h"
#include "extension_window.h"
#include "ui_content.h"
#include "permission.h"

namespace OHOS {
namespace Rosen {
using namespace AbilityRuntime;
namespace {
constexpr Rect g_emptyRect = {0, 0, 0, 0};
constexpr size_t INDEX_ZERO = 0;
constexpr size_t INDEX_ONE = 1;
constexpr size_t INDEX_TWO = 2;
constexpr size_t ARG_COUNT_ONE = 1;
constexpr size_t ARG_COUNT_TWO = 2;
constexpr size_t ARG_COUNT_THREE = 3;
constexpr size_t FOUR_PARAMS_SIZE = 4;
constexpr HiviewDFX::HiLogLabel LABEL = {LOG_CORE, HILOG_DOMAIN_WINDOW, "JsExtensionWindow"};
const std::unordered_set<std::string> g_emptyListener = {
    "windowVisibilityChange",
    "noInteractionDetected",
    "dialogTargetTouch",
    "windowEvent",
    "windowStatusChange",
    "windowTitleButtonRectChange",
    "windowRectChange",
    "rotationChange",
    "touchOutside",
};
const std::unordered_set<std::string> g_unsupportListener = {
    "windowWillClose",
    "windowHighlightChange",
};
const std::unordered_set<std::string> g_invalidListener = {
    "subWindowClose",
};

bool IsEmptyListener(const std::string& type)
{
    return g_emptyListener.find(type) != g_emptyListener.end();
}

bool IsUnsupportListener(const std::string& type)
{
    return g_unsupportListener.find(type) != g_unsupportListener.end();
}

bool IsInvalidListener(const std::string& type)
{
    return g_invalidListener.find(type) != g_invalidListener.end();
}
} // namespace

JsExtensionWindow::JsExtensionWindow(
    const std::shared_ptr<Rosen::ExtensionWindow> extensionWindow,
    int32_t hostWindowId)
    : extensionWindow_(extensionWindow), hostWindowId_(hostWindowId),
    extensionRegisterManager_(std::make_unique<JsExtensionWindowRegisterManager>()) {
}

JsExtensionWindow::JsExtensionWindow(const std::shared_ptr<Rosen::ExtensionWindow> extensionWindow,
    sptr<AAFwk::SessionInfo> sessionInfo)
    : extensionWindow_(extensionWindow), hostWindowId_(-1), sessionInfo_(sessionInfo),
    extensionRegisterManager_(std::make_unique<JsExtensionWindowRegisterManager>()) {
}

JsExtensionWindow::~JsExtensionWindow() {}

napi_value JsExtensionWindow::CreateJsExtensionWindow(napi_env env, sptr<Rosen::Window> window, int32_t hostWindowId)
{
    TLOGD(WmsLogTag::WMS_UIEXT, "Called.");
    napi_value objValue = nullptr;
    napi_create_object(env, &objValue);

    if (env == nullptr || window == nullptr || objValue == nullptr) {
        TLOGE(WmsLogTag::WMS_UIEXT, "JsExtensionWindow env or window is nullptr");
        return nullptr;
    }

    std::shared_ptr<ExtensionWindow> extensionWindow = std::make_shared<ExtensionWindowImpl>(window);
    std::unique_ptr<JsExtensionWindow> jsExtensionWindow =
        std::make_unique<JsExtensionWindow>(extensionWindow, hostWindowId);
    napi_wrap(env, objValue, jsExtensionWindow.release(), JsExtensionWindow::Finalizer, nullptr, nullptr);

    napi_property_descriptor desc[] = {
        DECLARE_NAPI_GETTER("properties", JsExtensionWindow::GetProperties)
    };
    NAPI_CALL(env, napi_define_properties(env, objValue, sizeof(desc) / sizeof(desc[0]), desc));

    const char *moduleName = "JsExtensionWindow";
    BindNativeFunction(env, objValue, "getWindowAvoidArea", moduleName, JsExtensionWindow::GetWindowAvoidArea);
    BindNativeFunction(env, objValue, "on", moduleName, JsExtensionWindow::RegisterExtensionWindowCallback);
    BindNativeFunction(env, objValue, "off", moduleName, JsExtensionWindow::UnRegisterExtensionWindowCallback);
    BindNativeFunction(env, objValue, "hideNonSecureWindows", moduleName, JsExtensionWindow::HideNonSecureWindows);
    BindNativeFunction(env, objValue, "createSubWindowWithOptions", moduleName,
        JsExtensionWindow::CreateSubWindowWithOptions);
    BindNativeFunction(env, objValue, "setWaterMarkFlag", moduleName, JsExtensionWindow::SetWaterMarkFlag);
    BindNativeFunction(env, objValue, "hidePrivacyContentForHost", moduleName,
                       JsExtensionWindow::HidePrivacyContentForHost);
    BindNativeFunction(env, objValue, "occupyEvents", moduleName, JsExtensionWindow::OccupyEvents);

    return objValue;
}

napi_value JsExtensionWindow::CreateJsExtensionWindowObject(napi_env env, sptr<Rosen::Window> window,
    sptr<AAFwk::SessionInfo> sessionInfo)
{
    TLOGI(WmsLogTag::WMS_UIEXT, "[NAPI]");
    napi_value objValue = nullptr;
    napi_create_object(env, &objValue);

    if (env == nullptr || window == nullptr || objValue == nullptr) {
        TLOGE(WmsLogTag::WMS_UIEXT, "JsExtensionWindow env or window is nullptr");
        return nullptr;
    }

    std::shared_ptr<ExtensionWindow> extensionWindow = std::make_shared<ExtensionWindowImpl>(window);
    std::unique_ptr<JsExtensionWindow> jsExtensionWindow = std::make_unique<JsExtensionWindow>(extensionWindow,
        sessionInfo);
    napi_wrap(env, objValue, jsExtensionWindow.release(), JsExtensionWindow::Finalizer, nullptr, nullptr);

    const char *moduleName = "JsExtensionWindow";
    BindNativeFunction(env, objValue, "on", moduleName, JsExtensionWindow::AtomicRegisterExtensionWindowCallback);
    BindNativeFunction(env, objValue, "off", moduleName, JsExtensionWindow::AtomicUnRegisterExtensionWindowCallback);
    BindNativeFunction(env, objValue, "getUIContext", moduleName, JsExtensionWindow::GetUIContext);
    BindNativeFunction(env, objValue, "setWindowBrightness", moduleName, JsExtensionWindow::SetWindowBrightness);
    BindNativeFunction(env, objValue, "setWindowKeepScreenOn", moduleName, JsExtensionWindow::SetWindowKeepScreenOn);
    BindNativeFunction(env, objValue, "loadContent", moduleName, JsExtensionWindow::LoadContent);
    BindNativeFunction(env, objValue, "loadContentByName", moduleName, JsExtensionWindow::LoadContentByName);
    BindNativeFunction(env, objValue, "setUIContent", moduleName, JsExtensionWindow::SetUIContent);
    BindNativeFunction(env, objValue, "isWindowShowing", moduleName, JsExtensionWindow::IsWindowShowingSync);
    BindNativeFunction(env, objValue, "getWindowProperties", moduleName, JsExtensionWindow::GetWindowPropertiesSync);
    BindNativeFunction(env, objValue, "getWindowAvoidArea", moduleName, JsExtensionWindow::GetWindowAvoidArea);
    BindNativeFunction(env, objValue, "setSpecificSystemBarEnabled", moduleName,
        JsExtensionWindow::SetSpecificSystemBarEnabled);
    BindNativeFunction(env, objValue, "getPreferredOrientation", moduleName,
        JsExtensionWindow::GetPreferredOrientation);
    BindNativeFunction(env, objValue, "createSubWindowWithOptions", moduleName,
        JsExtensionWindow::AtomicServiceCreateSubWindowWithOptions);
    BindNativeFunction(env, objValue, "setWindowLayoutFullScreen", moduleName,
        JsExtensionWindow::SetWindowLayoutFullScreen);
    BindNativeFunction(env, objValue, "getWindowColorSpace", moduleName, JsExtensionWindow::GetWindowColorSpace);
    BindNativeFunction(env, objValue, "setWindowColorSpace", moduleName, JsExtensionWindow::SetWindowColorSpace);
    BindNativeFunction(env, objValue, "setWindowPrivacyMode", moduleName, JsExtensionWindow::SetWindowPrivacyMode);
    BindNativeFunction(env, objValue, "setWindowSystemBarEnable", moduleName,
        JsExtensionWindow::SetWindowSystemBarEnable);
    BindNativeFunction(env, objValue, "isGestureBackEnabled", moduleName, JsExtensionWindow::GetGestureBackEnabled);
    BindNativeFunction(env, objValue, "setGestureBackEnabled", moduleName, JsExtensionWindow::SetGestureBackEnabled);
    BindNativeFunction(env, objValue, "getImmersiveModeEnabledState", moduleName,
        JsExtensionWindow::GetImmersiveModeEnabledState);
    BindNativeFunction(env, objValue, "setImmersiveModeEnabledState", moduleName,
        JsExtensionWindow::SetImmersiveModeEnabledState);
    BindNativeFunction(env, objValue, "isFocused", moduleName, JsExtensionWindow::IsFocused);
    BindNativeFunction(env, objValue, "isWindowSupportWideGamut", moduleName,
        JsExtensionWindow::IsWindowSupportWideGamut);
    BindNativeFunction(env, objValue, "getGlobalRect", moduleName, JsExtensionWindow::GetGlobalScaledRect);
    BindNativeFunction(env, objValue, "getStatusBarProperty", moduleName, JsExtensionWindow::GetStatusBarProperty);

    //return default value
    BindNativeFunction(env, objValue, "getTitleButtonRect", moduleName, JsExtensionWindow::GetTitleButtonRect);
    BindNativeFunction(env, objValue, "getWindowStatus", moduleName, JsExtensionWindow::GetWindowStatus);
    BindNativeFunction(env, objValue, "getWindowDensityInfo", moduleName, JsExtensionWindow::GetWindowDensityInfo);
    BindNativeFunction(env, objValue, "getWindowSystemBarProperties", moduleName,
        JsExtensionWindow::GetWindowSystemBarProperties);
    BindNativeFunction(env, objValue, "setStatusBarColor", moduleName, JsExtensionWindow::SetStatusBarColor);

    RegisterUnsupportFuncs(env, objValue, moduleName);

    return objValue;
}

void JsExtensionWindow::RegisterUnsupportFuncs(napi_env env, napi_value objValue, const char *moduleName)
{
    BindNativeFunction(env, objValue, "moveWindowTo", moduleName, JsExtensionWindow::EmptyAsyncCall);
    BindNativeFunction(env, objValue, "moveWindowToAsync", moduleName, JsExtensionWindow::EmptyAsyncCall);
    BindNativeFunction(env, objValue, "moveWindowToGlobal", moduleName, JsExtensionWindow::EmptyAsyncCall);
    BindNativeFunction(env, objValue, "resize", moduleName, JsExtensionWindow::EmptyAsyncCall);
    BindNativeFunction(env, objValue, "resizeAsync", moduleName, JsExtensionWindow::EmptyAsyncCall);
    BindNativeFunction(env, objValue, "showWindow", moduleName, JsExtensionWindow::EmptyAsyncCall);
    BindNativeFunction(env, objValue, "destroyWindow", moduleName, JsExtensionWindow::EmptyAsyncCall);
    BindNativeFunction(env, objValue, "setPreferredOrientation", moduleName, JsExtensionWindow::EmptyAsyncCall);
    BindNativeFunction(env, objValue, "setWindowFocusable", moduleName, JsExtensionWindow::EmptyAsyncCall);
    BindNativeFunction(env, objValue, "setExclusivelyHighlighted", moduleName, JsExtensionWindow::EmptyAsyncCall);
    BindNativeFunction(env, objValue, "setWindowTouchable", moduleName, JsExtensionWindow::EmptyAsyncCall);
    BindNativeFunction(env, objValue, "snapshot", moduleName, JsExtensionWindow::EmptyAsyncCall);
    BindNativeFunction(env, objValue, "snapshotIgnorePrivacy", moduleName, JsExtensionWindow::EmptyAsyncCall);
    BindNativeFunction(env, objValue, "setAspectRatio", moduleName, JsExtensionWindow::EmptyAsyncCall);
    BindNativeFunction(env, objValue, "resetAspectRatio", moduleName, JsExtensionWindow::EmptyAsyncCall);
    BindNativeFunction(env, objValue, "minimize", moduleName, JsExtensionWindow::EmptyAsyncCall);
    BindNativeFunction(env, objValue, "maximize", moduleName, JsExtensionWindow::EmptyAsyncCall);
    BindNativeFunction(env, objValue, "setResizeByDragEnabled", moduleName, JsExtensionWindow::EmptyAsyncCall);
    BindNativeFunction(env, objValue, "enableLandscapeMultiWindow", moduleName, JsExtensionWindow::EmptyAsyncCall);
    BindNativeFunction(env, objValue, "disableLandscapeMultiWindow", moduleName, JsExtensionWindow::EmptyAsyncCall);
    BindNativeFunction(env, objValue, "setWindowMask", moduleName, JsExtensionWindow::EmptyAsyncCall);
    BindNativeFunction(env, objValue, "setWindowGrayScale", moduleName, JsExtensionWindow::EmptyAsyncCall);
    BindNativeFunction(env, objValue, "setParentWindow", moduleName, JsExtensionWindow::EmptyAsyncCall);
    BindNativeFunction(env, objValue, "setFollowParentMultiScreenPolicy", moduleName,
        JsExtensionWindow::EmptyAsyncCall);
    BindNativeFunction(env, objValue, "setWindowSystemBarProperties", moduleName, JsExtensionWindow::EmptyAsyncCall);

    BindNativeFunction(env, objValue, "setWindowBackgroundColor", moduleName, JsExtensionWindow::EmptySyncCall);
    BindNativeFunction(env, objValue, "setWindowDecorVisible", moduleName, JsExtensionWindow::EmptySyncCall);
    BindNativeFunction(env, objValue, "getParentWindow", moduleName, JsExtensionWindow::EmptySyncCall);

    BindNativeFunction(env, objValue, "setFollowParentWindowLayoutEnabled", moduleName,
        JsExtensionWindow::UnsupportAsyncCall);
    BindNativeFunction(env, objValue, "setSystemAvoidAreaEnabled", moduleName, JsExtensionWindow::UnsupportAsyncCall);
    BindNativeFunction(env, objValue, "setWindowTopmost", moduleName, JsExtensionWindow::UnsupportAsyncCall);
    BindNativeFunction(env, objValue, "setWindowLimits", moduleName, JsExtensionWindow::UnsupportAsyncCall);
    BindNativeFunction(env, objValue, "recover", moduleName, JsExtensionWindow::UnsupportAsyncCall);
    BindNativeFunction(env, objValue, "restore", moduleName, JsExtensionWindow::UnsupportAsyncCall);
    BindNativeFunction(env, objValue, "setWindowTitle", moduleName, JsExtensionWindow::UnsupportAsyncCall);
    BindNativeFunction(env, objValue, "setSubWindowModal", moduleName, JsExtensionWindow::UnsupportAsyncCall);
    BindNativeFunction(env, objValue, "startMoving", moduleName, JsExtensionWindow::UnsupportAsyncCall);
    BindNativeFunction(env, objValue, "stopMoving", moduleName, JsExtensionWindow::UnsupportAsyncCall);
    BindNativeFunction(env, objValue, "setTitleAndDockHoverShown", moduleName, JsExtensionWindow::UnsupportAsyncCall);

    BindNativeFunction(env, objValue, "isSystemAvoidAreaEnabled", moduleName, JsExtensionWindow::UnsupportSyncCall);
    BindNativeFunction(env, objValue, "isWindowHighlighted", moduleName, JsExtensionWindow::UnsupportSyncCall);
    BindNativeFunction(env, objValue, "getWindowLimits", moduleName, JsExtensionWindow::UnsupportSyncCall);
    BindNativeFunction(env, objValue, "getWindowLimitsVP", moduleName, JsExtensionWindow::UnsupportSyncCall);
    BindNativeFunction(env, objValue, "getWindowDecorVisible", moduleName, JsExtensionWindow::UnsupportSyncCall);
    BindNativeFunction(env, objValue, "setWindowTitleMoveEnabled", moduleName, JsExtensionWindow::UnsupportSyncCall);
    BindNativeFunction(env, objValue, "setWindowDecorHeight", moduleName, JsExtensionWindow::UnsupportSyncCall);
    BindNativeFunction(env, objValue, "getWindowDecorHeight", moduleName, JsExtensionWindow::UnsupportSyncCall);
    BindNativeFunction(env, objValue, "setDecorButtonStyle", moduleName, JsExtensionWindow::UnsupportSyncCall);
    BindNativeFunction(env, objValue, "getDecorButtonStyle", moduleName, JsExtensionWindow::UnsupportSyncCall);
    BindNativeFunction(env, objValue, "setWindowTitleButtonVisible", moduleName, JsExtensionWindow::UnsupportSyncCall);
    BindNativeFunction(env, objValue, "setWindowDelayRaiseOnDrag", moduleName, JsExtensionWindow::UnsupportSyncCall);

    BindNativeFunction(env, objValue, "setDialogBackGestureEnabled", moduleName, JsExtensionWindow::InvalidAsyncCall);
    BindNativeFunction(env, objValue, "setSubWindowZLevel", moduleName, JsExtensionWindow::InvalidAsyncCall);
    BindNativeFunction(env, objValue, "setWindowCornerRadius", moduleName, JsExtensionWindow::EmptySyncCall);
    BindNativeFunction(env, objValue, "getWindowTransitionAnimation", moduleName, JsExtensionWindow::EmptySyncCall);

    BindNativeFunction(env, objValue, "keepKeyboardOnFocus", moduleName, JsExtensionWindow::InvalidSyncCall);
    BindNativeFunction(env, objValue, "getSubWindowZLevel", moduleName, JsExtensionWindow::InvalidSyncCall);
    BindNativeFunction(env, objValue, "getWindowCornerRadius", moduleName, JsExtensionWindow::EmptySyncCall);
    BindNativeFunction(env, objValue, "setWindowShadowRadius", moduleName, JsExtensionWindow::EmptySyncCall);
    BindNativeFunction(env, objValue, "setWindowTransitionAnimation", moduleName, JsExtensionWindow::EmptySyncCall);
}

void JsExtensionWindow::Finalizer(napi_env env, void* data, void* hint)
{
    TLOGI(WmsLogTag::WMS_UIEXT, "[NAPI]");
    std::unique_ptr<JsExtensionWindow>(static_cast<JsExtensionWindow*>(data));
}

napi_value JsExtensionWindow::GetWindowAvoidArea(napi_env env, napi_callback_info info)
{
    TLOGI(WmsLogTag::WMS_UIEXT, "[NAPI]");
    JsExtensionWindow* me = CheckParamsAndGetThis<JsExtensionWindow>(env, info);
    return (me != nullptr) ? me->OnGetWindowAvoidArea(env, info) : nullptr;
}

napi_value JsExtensionWindow::RegisterExtensionWindowCallback(napi_env env, napi_callback_info info)
{
    TLOGI(WmsLogTag::WMS_UIEXT, "[NAPI]");
    JsExtensionWindow* me = CheckParamsAndGetThis<JsExtensionWindow>(env, info);
    return (me != nullptr) ? me->OnRegisterExtensionWindowCallback(env, info) : nullptr;
}

napi_value JsExtensionWindow::UnRegisterExtensionWindowCallback(napi_env env, napi_callback_info info)
{
    TLOGI(WmsLogTag::WMS_UIEXT, "[NAPI]");
    JsExtensionWindow* me = CheckParamsAndGetThis<JsExtensionWindow>(env, info);
    return (me != nullptr) ? me->OnUnRegisterExtensionWindowCallback(env, info) : nullptr;
}

napi_value JsExtensionWindow::AtomicRegisterExtensionWindowCallback(napi_env env, napi_callback_info info)
{
    TLOGI(WmsLogTag::WMS_UIEXT, "[NAPI]");
    JsExtensionWindow* me = CheckParamsAndGetThis<JsExtensionWindow>(env, info);
    return (me != nullptr) ? me->OnRegisterExtensionWindowCallback(env, info, true) : nullptr;
}

napi_value JsExtensionWindow::AtomicUnRegisterExtensionWindowCallback(napi_env env, napi_callback_info info)
{
    TLOGI(WmsLogTag::WMS_UIEXT, "[NAPI]");
    JsExtensionWindow* me = CheckParamsAndGetThis<JsExtensionWindow>(env, info);
    return (me != nullptr) ? me->OnUnRegisterExtensionWindowCallback(env, info, true) : nullptr;
}


napi_value JsExtensionWindow::HideNonSecureWindows(napi_env env, napi_callback_info info)
{
    TLOGI(WmsLogTag::WMS_UIEXT, "[NAPI]");
    JsExtensionWindow* me = CheckParamsAndGetThis<JsExtensionWindow>(env, info);
    return (me != nullptr) ? me->OnHideNonSecureWindows(env, info) : nullptr;
}

napi_value JsExtensionWindow::CreateSubWindowWithOptions(napi_env env, napi_callback_info info)
{
    TLOGI(WmsLogTag::WMS_UIEXT, "[NAPI]");
    JsExtensionWindow* me = CheckParamsAndGetThis<JsExtensionWindow>(env, info);
    return (me != nullptr) ? me->OnCreateSubWindowWithOptions(env, info) : nullptr;
}

napi_value JsExtensionWindow::SetWaterMarkFlag(napi_env env, napi_callback_info info)
{
    TLOGI(WmsLogTag::WMS_UIEXT, "[NAPI]");
    JsExtensionWindow* me = CheckParamsAndGetThis<JsExtensionWindow>(env, info);
    return (me != nullptr) ? me->OnSetWaterMarkFlag(env, info) : nullptr;
}

napi_value JsExtensionWindow::HidePrivacyContentForHost(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::WMS_UIEXT, "[NAPI]");
    JsExtensionWindow* me = CheckParamsAndGetThis<JsExtensionWindow>(env, info);
    return (me != nullptr) ? me->OnHidePrivacyContentForHost(env, info) : nullptr;
}

napi_value JsExtensionWindow::LoadContent(napi_env env, napi_callback_info info)
{
    TLOGI(WmsLogTag::WMS_UIEXT, "[NAPI]");
    JsExtensionWindow* me = CheckParamsAndGetThis<JsExtensionWindow>(env, info);
    return (me != nullptr) ? me->OnLoadContent(env, info, false) : nullptr;
}

napi_value JsExtensionWindow::LoadContentByName(napi_env env, napi_callback_info info)
{
    TLOGI(WmsLogTag::WMS_UIEXT, "[NAPI]");
    JsExtensionWindow* me = CheckParamsAndGetThis<JsExtensionWindow>(env, info);
    return (me != nullptr) ? me->OnLoadContent(env, info, true) : nullptr;
}

napi_value JsExtensionWindow::IsWindowShowingSync(napi_env env, napi_callback_info info)
{
    TLOGI(WmsLogTag::WMS_UIEXT, "[NAPI]");
    JsExtensionWindow* me = CheckParamsAndGetThis<JsExtensionWindow>(env, info);
    return (me != nullptr) ? me->OnIsWindowShowingSync(env, info) : nullptr;
}

napi_value JsExtensionWindow::SetUIContent(napi_env env, napi_callback_info info)
{
    TLOGI(WmsLogTag::WMS_UIEXT, "[NAPI]");
    JsExtensionWindow* me = CheckParamsAndGetThis<JsExtensionWindow>(env, info);
    return (me != nullptr) ? me->OnSetUIContent(env, info) : nullptr;
}

napi_value JsExtensionWindow::GetWindowPropertiesSync(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::WMS_UIEXT, "[NAPI]");
    JsExtensionWindow* me = CheckParamsAndGetThis<JsExtensionWindow>(env, info);
    return (me != nullptr) ? me->OnGetWindowPropertiesSync(env, info) : nullptr;
}

napi_value JsExtensionWindow::SetSpecificSystemBarEnabled(napi_env env, napi_callback_info info)
{
    TLOGI(WmsLogTag::WMS_UIEXT, "[NAPI]");
    JsExtensionWindow* me = CheckParamsAndGetThis<JsExtensionWindow>(env, info);
    return (me != nullptr) ? me->OnSetSpecificSystemBarEnabled(env, info) : nullptr;
}

napi_value JsExtensionWindow::GetPreferredOrientation(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::WMS_UIEXT, "[NAPI]");
    ApiOrientation apiOrientation = ApiOrientation::UNSPECIFIED;
    return CreateJsValue(env, static_cast<uint32_t>(apiOrientation));
}

napi_value JsExtensionWindow::GetUIContext(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::WMS_UIEXT, "[NAPI]");
    JsExtensionWindow* me = CheckParamsAndGetThis<JsExtensionWindow>(env, info);
    return (me != nullptr) ? me->OnGetUIContext(env, info) : nullptr;
}

napi_value JsExtensionWindow::SetWindowBrightness(napi_env env, napi_callback_info info)
{
    TLOGI(WmsLogTag::WMS_UIEXT, "[NAPI]");
    JsExtensionWindow* me = CheckParamsAndGetThis<JsExtensionWindow>(env, info);
    return (me != nullptr) ? me->OnSetWindowBrightness(env, info) : nullptr;
}

napi_value JsExtensionWindow::SetWindowKeepScreenOn(napi_env env, napi_callback_info info)
{
    TLOGI(WmsLogTag::WMS_UIEXT, "[NAPI]");
    JsExtensionWindow* me = CheckParamsAndGetThis<JsExtensionWindow>(env, info);
    return (me != nullptr) ? me->OnSetWindowKeepScreenOn(env, info) : nullptr;
}

napi_value JsExtensionWindow::OccupyEvents(napi_env env, napi_callback_info info)
{
    JsExtensionWindow* me = CheckParamsAndGetThis<JsExtensionWindow>(env, info);
    return (me != nullptr) ? me->OnOccupyEvents(env, info) : nullptr;
}

napi_value JsExtensionWindow::AtomicServiceCreateSubWindowWithOptions(napi_env env, napi_callback_info info)
{
    JsExtensionWindow* me = CheckParamsAndGetThis<JsExtensionWindow>(env, info);
    return (me != nullptr) ? me->OnUnsupportAsyncCall(env, info) : nullptr;
}

napi_value JsExtensionWindow::SetWindowLayoutFullScreen(napi_env env, napi_callback_info info)
{
    JsExtensionWindow* me = CheckParamsAndGetThis<JsExtensionWindow>(env, info);
    return (me != nullptr) ? me->OnSetWindowLayoutFullScreen(env, info) : nullptr;
}

napi_value JsExtensionWindow::GetWindowColorSpace(napi_env env, napi_callback_info info)
{
    JsExtensionWindow* me = CheckParamsAndGetThis<JsExtensionWindow>(env, info);
    return (me != nullptr) ? me->OnGetWindowColorSpace(env, info) : nullptr;
}

napi_value JsExtensionWindow::SetWindowColorSpace(napi_env env, napi_callback_info info)
{
    JsExtensionWindow* me = CheckParamsAndGetThis<JsExtensionWindow>(env, info);
    return (me != nullptr) ? me->OnSetWindowColorSpace(env, info) : nullptr;
}

napi_value JsExtensionWindow::SetWindowPrivacyMode(napi_env env, napi_callback_info info)
{
    JsExtensionWindow* me = CheckParamsAndGetThis<JsExtensionWindow>(env, info);
    return (me != nullptr) ? me->OnSetWindowPrivacyMode(env, info) : nullptr;
}

napi_value JsExtensionWindow::SetWindowSystemBarEnable(napi_env env, napi_callback_info info)
{
    JsExtensionWindow* me = CheckParamsAndGetThis<JsExtensionWindow>(env, info);
    return (me != nullptr) ? me->OnSetWindowSystemBarEnable(env, info) : nullptr;
}

napi_value JsExtensionWindow::GetGestureBackEnabled(napi_env env, napi_callback_info info)
{
    JsExtensionWindow* me = CheckParamsAndGetThis<JsExtensionWindow>(env, info);
    return (me != nullptr) ? me->OnGetGestureBackEnabled(env, info) : nullptr;
}

napi_value JsExtensionWindow::SetGestureBackEnabled(napi_env env, napi_callback_info info)
{
    JsExtensionWindow* me = CheckParamsAndGetThis<JsExtensionWindow>(env, info);
    return (me != nullptr) ? me->OnSetGestureBackEnabled(env, info) : nullptr;
}

napi_value JsExtensionWindow::GetImmersiveModeEnabledState(napi_env env, napi_callback_info info)
{
    JsExtensionWindow* me = CheckParamsAndGetThis<JsExtensionWindow>(env, info);
    return (me != nullptr) ? me->OnGetImmersiveModeEnabledState(env, info) : nullptr;
}

napi_value JsExtensionWindow::SetImmersiveModeEnabledState(napi_env env, napi_callback_info info)
{
    JsExtensionWindow* me = CheckParamsAndGetThis<JsExtensionWindow>(env, info);
    return (me != nullptr) ? me->OnSetImmersiveModeEnabledState(env, info) : nullptr;
}

napi_value JsExtensionWindow::IsFocused(napi_env env, napi_callback_info info)
{
    JsExtensionWindow* me = CheckParamsAndGetThis<JsExtensionWindow>(env, info);
    return (me != nullptr) ? me->OnIsFocused(env, info) : nullptr;
}

napi_value JsExtensionWindow::IsWindowSupportWideGamut(napi_env env, napi_callback_info info)
{
    JsExtensionWindow* me = CheckParamsAndGetThis<JsExtensionWindow>(env, info);
    return (me != nullptr) ? me->OnIsWindowSupportWideGamut(env, info) : nullptr;
}

napi_value JsExtensionWindow::GetGlobalScaledRect(napi_env env, napi_callback_info info)
{
    JsExtensionWindow* me = CheckParamsAndGetThis<JsExtensionWindow>(env, info);
    return (me != nullptr) ? me->OnGetGlobalScaledRect(env, info) : nullptr;
}

napi_value JsExtensionWindow::GetStatusBarProperty(napi_env env, napi_callback_info info)
{
    JsExtensionWindow* me = CheckParamsAndGetThis<JsExtensionWindow>(env, info);
    return (me != nullptr) ? me->OnGetStatusBarPropertySync(env, info) : nullptr;
}

napi_value JsExtensionWindow::GetTitleButtonRect(napi_env env, napi_callback_info info)
{
    TitleButtonRect titleButtonRect { 0, 0, 0, 0 };
    napi_value TitleButtonAreaObj = ConvertTitleButtonAreaToJsValue(env, titleButtonRect);
    if (TitleButtonAreaObj == nullptr) {
        return NapiThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
    }
    return TitleButtonAreaObj;
}

napi_value JsExtensionWindow::GetWindowStatus(napi_env env, napi_callback_info info)
{
    WindowStatus windowStatus = WindowStatus::WINDOW_STATUS_UNDEFINED;
    return CreateJsValue(env, static_cast<uint32_t>(windowStatus));
}

napi_value JsExtensionWindow::SetStatusBarColor(napi_env env, napi_callback_info info)
{
    JsExtensionWindow* me = CheckParamsAndGetThis<JsExtensionWindow>(env, info);
    return (me != nullptr) ? me->OnSetStatusBarColor(env, info) : nullptr;
}

napi_value JsExtensionWindow::EmptyAsyncCall(napi_env env, napi_callback_info info)
{
    JsExtensionWindow* me = CheckParamsAndGetThis<JsExtensionWindow>(env, info);
    return (me != nullptr) ? me->OnEmptyAsyncCall(env, info) : nullptr;
}

napi_value JsExtensionWindow::EmptySyncCall(napi_env env, napi_callback_info info)
{
    return NapiGetUndefined(env);
}

napi_value JsExtensionWindow::UnsupportAsyncCall(napi_env env, napi_callback_info info)
{
    JsExtensionWindow* me = CheckParamsAndGetThis<JsExtensionWindow>(env, info);
    return (me != nullptr) ? me->OnUnsupportAsyncCall(env, info) : nullptr;
}

napi_value JsExtensionWindow::UnsupportSyncCall(napi_env env, napi_callback_info info)
{
    return NapiThrowError(env, WmErrorCode::WM_ERROR_DEVICE_NOT_SUPPORT);
}

napi_value JsExtensionWindow::InvalidAsyncCall(napi_env env, napi_callback_info info)
{
    JsExtensionWindow* me = CheckParamsAndGetThis<JsExtensionWindow>(env, info);
    return (me != nullptr) ? me->OnInvalidAsyncCall(env, info) : nullptr;
}

napi_value JsExtensionWindow::InvalidSyncCall(napi_env env, napi_callback_info info)
{
    return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_CALLING);
}

napi_value JsExtensionWindow::GetWindowDensityInfo(napi_env env, napi_callback_info info)
{
    WindowDensityInfo densityInfo {};
    auto objValue = ConvertWindowDensityInfoToJsValue(env, densityInfo);
    if (objValue == nullptr) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "create js windowDensityInfo failed");
        return NapiThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
    }
    return objValue;
}

napi_value JsExtensionWindow::GetWindowSystemBarProperties(napi_env env, napi_callback_info info)
{
    auto window = sptr<Window>::MakeSptr();
    auto objValue = CreateJsSystemBarPropertiesObject(env, window);
    if (objValue == nullptr) {
        TLOGE(WmsLogTag::WMS_IMMS, "get properties failed");
        return NapiThrowError(env, WmErrorCode::WM_ERROR_SYSTEM_ABNORMALLY);
    }
    return objValue;
}

napi_valuetype GetType(napi_env env, napi_value value)
{
    napi_valuetype res = napi_undefined;
    napi_typeof(env, value, &res);
    return res;
}

static void LoadContentTask(std::shared_ptr<NativeReference> contentStorage, std::string contextUrl,
    const std::shared_ptr<Rosen::ExtensionWindow> win, napi_env env, NapiAsyncTask& task,
    sptr<IRemoteObject> parentToken, bool isLoadedByName)
{
    napi_value nativeStorage = (contentStorage == nullptr) ? nullptr : contentStorage->GetNapiValue();
    sptr<Window> windowImpl = win->GetWindow();
    WMError ret;
    if (isLoadedByName) {
        ret = windowImpl->NapiSetUIContentByName(contextUrl, env, nativeStorage,
            BackupAndRestoreType::NONE, parentToken);
    } else {
        ret = windowImpl->NapiSetUIContent(contextUrl, env, nativeStorage, BackupAndRestoreType::NONE, parentToken);
    }
    if (ret == WMError::WM_OK) {
        task.Resolve(env, NapiGetUndefined(env));
    } else {
        task.Reject(env, JsErrUtils::CreateJsError(env, ret, "Parameter verification failed"));
    }
    TLOGI(WmsLogTag::WMS_UIEXT, "Window [%{public}u, %{public}s] end, ret=%{public}d",
        windowImpl->GetWindowId(), windowImpl->GetWindowName().c_str(), ret);
}

napi_value JsExtensionWindow::OnSetWindowKeepScreenOn(napi_env env, napi_callback_info info)
{
    WmErrorCode errCode = WmErrorCode::WM_OK;
    size_t argc = ARG_COUNT_TWO;
    napi_value argv[INDEX_TWO] = { nullptr };
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc < ARG_COUNT_ONE) {
        TLOGE(WmsLogTag::WMS_UIEXT, "Argc is invalid: %{public}zu", argc);
        return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM,
            "[window][setWindowKeepScreenOn]msg: Mandatory parameters are left unspecified");
    }
    bool keepScreenOn = true;
    napi_value nativeVal = argv[INDEX_ZERO];
    if (nativeVal == nullptr) {
        TLOGE(WmsLogTag::WMS_UIEXT, "Failed to convert parameter to keepScreenOn");
        errCode = WmErrorCode::WM_ERROR_INVALID_PARAM;
    } else {
        CHECK_NAPI_RETCODE(errCode, WmErrorCode::WM_ERROR_INVALID_PARAM,
            napi_get_value_bool(env, nativeVal, &keepScreenOn));
    }
    if (errCode == WmErrorCode::WM_ERROR_INVALID_PARAM) {
        return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM,
            "[window][setWindowKeepScreenOn]msg: Incorrect parameter types");
    }
    wptr<Window> weakToken(extensionWindow_->GetWindow());
    std::shared_ptr<WmErrorCode> errCodePtr = std::make_shared<WmErrorCode>(WmErrorCode::WM_OK);
    NapiAsyncTask::ExecuteCallback execute = [weakToken, keepScreenOn, errCodePtr] {
        if (errCodePtr == nullptr) {
            return;
        }
        auto weakWindow = weakToken.promote();
        if (weakWindow == nullptr) {
            *errCodePtr = WmErrorCode::WM_ERROR_STATE_ABNORMALLY;
            return;
        }
        *errCodePtr = WM_JS_TO_ERROR_CODE_MAP.at(weakWindow->ExtensionSetKeepScreenOn(keepScreenOn));
        TLOGNI(WmsLogTag::WMS_UIEXT, "Window [%{public}u, %{public}s] set keep screen on end",
            weakWindow->GetWindowId(), weakWindow->GetWindowName().c_str());
    };
    NapiAsyncTask::CompleteCallback complete =
        [weakToken, keepScreenOn, errCodePtr](napi_env env, NapiAsyncTask& task, int32_t status) {
            if (errCodePtr == nullptr) {
                task.Reject(env, JsErrUtils::CreateJsError(env,  WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
                    "[window][setWindowKeepScreenOn]"));
                return;
            }
            if (*errCodePtr == WmErrorCode::WM_OK) {
                task.Resolve(env, NapiGetUndefined(env));
            } else {
                task.Reject(env, JsErrUtils::CreateJsError(env, *errCodePtr,
                    "[window][setWindowKeepScreenOn]"));
            }
        };

    napi_value lastParam = nullptr;
    if (argc > ARG_COUNT_ONE && argv[INDEX_ONE] != nullptr && GetType(env, argv[INDEX_ONE]) == napi_function) {
        lastParam = argv[INDEX_ONE];
    }
    napi_value result = nullptr;
    NapiAsyncTask::Schedule("JsExtensionWindow::OnSetWindowKeepScreenOn",
        env, CreateAsyncTaskWithLastParam(env, lastParam, std::move(execute), std::move(complete), &result));
    return result;
}

napi_value JsExtensionWindow::OnSetWindowBrightness(napi_env env, napi_callback_info info)
{
    WmErrorCode errCode = WmErrorCode::WM_OK;
    size_t argc = ARG_COUNT_TWO;
    napi_value argv[INDEX_TWO] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc < ARG_COUNT_ONE) {
        TLOGE(WmsLogTag::WMS_UIEXT, "Argc is invalid: %{public}zu", argc);
        return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM,
            "[window][setWindowBrightness]msg: Mandatory parameters are left unspecified");
    }
    double brightness = UNDEFINED_BRIGHTNESS;
    napi_value nativeVal = argv[0];
    if (nativeVal == nullptr) {
        TLOGE(WmsLogTag::WMS_UIEXT, "Failed to convert parameter to brightness");
        errCode = WmErrorCode::WM_ERROR_INVALID_PARAM;
    } else {
        CHECK_NAPI_RETCODE(errCode, WmErrorCode::WM_ERROR_INVALID_PARAM,
            napi_get_value_double(env, nativeVal, &brightness));
    }
    if (errCode == WmErrorCode::WM_ERROR_INVALID_PARAM) {
        return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM,
            "[window][setWindowBrightness]msg: Incorrect parameter types");
    }

    napi_value lastParam = nullptr;
    if (argc > ARG_COUNT_ONE && argv[INDEX_ONE] != nullptr && GetType(env, argv[INDEX_ONE]) == napi_function) {
        lastParam = argv[INDEX_ONE];
    }
    napi_value result = nullptr;
    std::shared_ptr<NapiAsyncTask> napiAsyncTask = CreateEmptyAsyncTask(env, lastParam, &result);
    auto asyncTask = [weakToken = wptr<Window>(extensionWindow_->GetWindow()), brightness, env, task = napiAsyncTask] {
        auto weakWindow = weakToken.promote();
        if (weakWindow == nullptr) {
            task->Reject(env, JsErrUtils::CreateJsError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
                "[window][setWindowBrightness]msg: The window is not created or destroyed"));
            return;
        }
        WmErrorCode ret = WM_JS_TO_ERROR_CODE_MAP.at(weakWindow->ExtensionSetBrightness(brightness));
        if (ret == WmErrorCode::WM_OK) {
            task->Resolve(env, NapiGetUndefined(env));
        } else {
            task->Reject(env, JsErrUtils::CreateJsError(env, ret, "[window][setWindowBrightness]"));
        }
        TLOGNI(WmsLogTag::WMS_ATTRIBUTE, "Window [%{public}u, %{public}s] set brightness end, result: %{public}d",
            weakWindow->GetWindowId(), weakWindow->GetWindowName().c_str(), ret);
    };
    if (napi_send_event(env, asyncTask, napi_eprio_high, "OnSetWindowBrightness") != napi_status::napi_ok) {
        TLOGE(WmsLogTag::WMS_IMMS, "napi_send_event failed");
        napiAsyncTask->Reject(env, JsErrUtils::CreateJsError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
            "[window][setWindowBrightness]msg: Internal task error"));
    }
    return result;
}

napi_value JsExtensionWindow::OnGetUIContext(napi_env env, napi_callback_info info)
{
    size_t argc = 4;
    napi_value argv[4] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc >= 1) {
        TLOGE(WmsLogTag::WMS_UIEXT, "Argc is invalid: %{public}zu, expect zero params", argc);
        return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM,
            "[window][getUIContext]msg: Mandatory parameters are left unspecified");
    }
    sptr<Window> windowImpl = extensionWindow_->GetWindow();
    if (windowImpl == nullptr) {
        TLOGE(WmsLogTag::WMS_UIEXT, "window is nullptr");
        return NapiThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
            "[window][getUIContext]msg: The window is not created or destroyed.");
    }

    auto uicontent = windowImpl->GetUIContent();
    if (uicontent == nullptr) {
        TLOGW(WmsLogTag::WMS_UIEXT, "uicontent is nullptr");
        return NapiThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
            "[window][getUIContext]msg: Uicontent is nullptr");
    }

    napi_value uiContext = uicontent->GetUINapiContext();
    if (uiContext == nullptr) {
        TLOGE(WmsLogTag::WMS_UIEXT, "uiContext obtained from jsEngine is nullptr");
        return NapiThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
            "[window][getUIContext]msg: UiContext obtained from jsEngine is nullptr");
    } else {
        return uiContext;
    }
}

napi_value JsExtensionWindow::OnSetSpecificSystemBarEnabled(napi_env env, napi_callback_info info)
{
    size_t argc = FOUR_PARAMS_SIZE;
    napi_value argv[FOUR_PARAMS_SIZE] = { nullptr };
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    std::string name;
    if (!ConvertFromJsValue(env, argv[INDEX_ZERO], name) ||
        (name.compare("status") != 0 && name.compare("navigation") != 0 && name.compare("navigationIndicator") != 0)) {
        TLOGE(WmsLogTag::WMS_IMMS, "invalid systemBar name");
        return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM,
            "[window][setSpecificSystemBarEnabled]msg: Incorrect parameter types");
    }
    
    bool systemBarEnable = false;
    bool systemBarEnableAnimation = false;
    if (!GetSpecificBarStatus(env, info, systemBarEnable, systemBarEnableAnimation)) {
        TLOGE(WmsLogTag::WMS_IMMS, "invalid param or argc:%{public}zu", argc);
        return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM,
            "[window][setSpecificSystemBarEnabled]msg: Mandatory parameters are left unspecified");
    }
    napi_value result = nullptr;
    const char* const where = __func__;
    std::shared_ptr<NapiAsyncTask> napiAsyncTask = CreateEmptyAsyncTask(env, nullptr, &result);
    auto asyncTask = [weakToken = wptr<Window>(extensionWindow_->GetWindow()), env, task = napiAsyncTask,
        name, systemBarEnable, systemBarEnableAnimation, where] {
        auto window = weakToken.promote();
        if (window == nullptr) {
            TLOGNE(WmsLogTag::WMS_IMMS, "%{public}s window is nullptr", where);
            task->Reject(env, JsErrUtils::CreateJsError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
                "[window][setSpecificSystemBarEnabled]msg: The window is not created or destroyed"));
            return;
        }
        auto ret = window->UpdateHostSpecificSystemBarEnabled(name, systemBarEnable, systemBarEnableAnimation);
        auto errCode = WM_JS_TO_ERROR_CODE_MAP.at(ret);
        if (errCode == WmErrorCode::WM_OK) {
            task->Resolve(env, NapiGetUndefined(env));
        } else {
            TLOGNE(WmsLogTag::WMS_IMMS, "%{public}s failed, ret %{public}d", where, errCode);
            task->Reject(env, JsErrUtils::CreateJsError(env, errCode,
                "[window][setSpecificSystemBarEnabled]"));
        }
    };
    if (napi_send_event(env, asyncTask, napi_eprio_high, "OnSetSpecificSystemBarEnabled") != napi_status::napi_ok) {
        TLOGE(WmsLogTag::WMS_IMMS, "napi_send_event failed");
        napiAsyncTask->Reject(env, JsErrUtils::CreateJsError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
            "[window][setSpecificSystemBarEnabled]msg: Internal task error"));
    }
    return result;
}

napi_value JsExtensionWindow::OnGetWindowPropertiesSync(napi_env env, napi_callback_info info)
{
    sptr<Window> windowImpl = extensionWindow_->GetWindow();
    if (windowImpl == nullptr) {
        TLOGW(WmsLogTag::WMS_UIEXT, "window is nullptr");
        return NapiThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
            "[window][getWindowProperties]msg: The window is not created or destroyed");
    }
    auto objValue = CreateJsExtensionWindowProperties(env, windowImpl);
    TLOGI(WmsLogTag::WMS_UIEXT, "Window [%{public}u, %{public}s] get properties end",
        windowImpl->GetWindowId(), windowImpl->GetWindowName().c_str());
    if (objValue != nullptr) {
        return objValue;
    } else {
        return NapiThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
            "[window][getWindowProperties]msg: Internal task error");
    }
}

napi_value JsExtensionWindow::OnIsWindowShowingSync(napi_env env, napi_callback_info info)
{
    sptr<Window> windowImpl = extensionWindow_->GetWindow();
    if (windowImpl == nullptr) {
        TLOGE(WmsLogTag::WMS_UIEXT, "window is nullptr");
        return NapiThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
            "[window][isWindowShowing]msg: The window is not created or destroyed");
    }
    bool state = (windowImpl->GetWindowState() == WindowState::STATE_SHOWN);
    TLOGI(WmsLogTag::WMS_UIEXT, "Window [%{public}u, %{public}s] get show state end, state=%{public}u",
        windowImpl->GetWindowId(), windowImpl->GetWindowName().c_str(), state);
    return CreateJsValue(env, state);
}

napi_value JsExtensionWindow::OnSetUIContent(napi_env env, napi_callback_info info)
{
    WmErrorCode errCode = WmErrorCode::WM_OK;
    size_t argc = 4;
    napi_value argv[4] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc < 1) { // 2 maximum param num
        TLOGE(WmsLogTag::WMS_UIEXT, "Argc is invalid: %{public}zu", argc);
        errCode = WmErrorCode::WM_ERROR_INVALID_PARAM;
    }
    std::string contextUrl;
    if (errCode == WmErrorCode::WM_OK && !ConvertFromJsValue(env, argv[0], contextUrl)) {
        TLOGE(WmsLogTag::WMS_UIEXT, "Failed to convert parameter to context url");
        errCode = WmErrorCode::WM_ERROR_INVALID_PARAM;
        return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM,
            "[window][setUIContent]msg: Parameter verification failed");
    }
    napi_value lastParam = nullptr;
    if (argc >= 2) { // 2 param num
        lastParam = argv[1];
    }
    std::shared_ptr<NativeReference> contentStorage = nullptr;

    sptr<IRemoteObject> parentToken = sessionInfo_->parentToken;
    napi_value result = nullptr;
    std::shared_ptr<NapiAsyncTask> napiAsyncTask = CreateEmptyAsyncTask(env, lastParam, &result);
    auto asyncTask = [extWindow = extensionWindow_, contentStorage, contextUrl, parentToken,
        env, task = napiAsyncTask]() {
        if (extWindow == nullptr) {
            TLOGNE(WmsLogTag::WMS_UIEXT, "Window is nullptr");
            task->Reject(env, JsErrUtils::CreateJsError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
                "[window][setUIContent]msg: The window is not created or destroyed"));
            return;
        }
        LoadContentTask(contentStorage, contextUrl, extWindow, env, *task, parentToken, false);
    };
    if (napi_send_event(env, asyncTask, napi_eprio_high, "OnSetUIContent") != napi_status::napi_ok) {
        napiAsyncTask->Reject(env, JsErrUtils::CreateJsError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
            "[window][setUIContent]msg: Internal task error"));
    }
    return result;
}

napi_value JsExtensionWindow::OnLoadContent(napi_env env, napi_callback_info info, bool isLoadedByName)
{
    TLOGI(WmsLogTag::WMS_UIEXT, "OnLoadContent is called");
    WmErrorCode errCode = WmErrorCode::WM_OK;
    size_t argc = 4;
    napi_value argv[4] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    std::string contextUrl;
    if (!ConvertFromJsValue(env, argv[0], contextUrl)) {
        TLOGI(WmsLogTag::WMS_UIEXT, "Failed to convert parameter to context url");
        errCode = WmErrorCode::WM_ERROR_INVALID_PARAM;
    }
    napi_value storage = nullptr;
    napi_value lastParam = nullptr;
    napi_value value1 = argv[1];
    napi_value value2 = argv[2];
    if (GetType(env, value1) == napi_function) {
        lastParam = value1;
    } else if (GetType(env, value1) == napi_object) {
        storage = value1;
    }
    if (GetType(env, value2) == napi_function) {
        lastParam = value2;
    }
    if (errCode == WmErrorCode::WM_ERROR_INVALID_PARAM) {
        TLOGI(WmsLogTag::WMS_UIEXT, "Invalid param");
        napi_throw(env, JsErrUtils::CreateJsError(env, WmErrorCode::WM_ERROR_INVALID_PARAM,
            "[window][loadContent]msg: Incorrect parameters types"));
        return NapiGetUndefined(env);
    }

    std::shared_ptr<NativeReference> contentStorage = nullptr;
    if (storage != nullptr) {
        napi_ref result = nullptr;
        napi_create_reference(env, storage, 1, &result);
        contentStorage = std::shared_ptr<NativeReference>(reinterpret_cast<NativeReference*>(result));
    }

    sptr<IRemoteObject> parentToken = sessionInfo_->parentToken;
    napi_value result = nullptr;
    std::shared_ptr<NapiAsyncTask> napiAsyncTask = CreateEmptyAsyncTask(env, lastParam, &result);
    auto asyncTask = [extWindow = extensionWindow_, contentStorage, contextUrl, parentToken, isLoadedByName,
        env, task = napiAsyncTask]() {
        if (extWindow == nullptr) {
            TLOGNE(WmsLogTag::WMS_UIEXT, "Window is nullptr");
            task->Reject(env, JsErrUtils::CreateJsError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
                "[window][loadContent]msg: The window is not created or destroyed"));
            return;
        }
        LoadContentTask(contentStorage, contextUrl, extWindow, env, *task, parentToken, isLoadedByName);
    };
    if (napi_send_event(env, asyncTask, napi_eprio_high, "OnLoadContent") != napi_status::napi_ok) {
        napiAsyncTask->Reject(env, JsErrUtils::CreateJsError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
            "[window][loadContent]msg: Internal task error"));
    }
    return result;
}

napi_value JsExtensionWindow::OnGetWindowAvoidArea(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::WMS_UIEXT, "OnGetWindowAvoidArea is called");

    WmErrorCode errCode = WmErrorCode::WM_OK;
    size_t argc = 4;
    napi_value argv[4] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc < 1) { // 1: params num
        return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM,
            "[window][getWindowAvoidArea]msg: Mandatory parameters are left unspecified");
    }
    AvoidAreaType avoidAreaType = AvoidAreaType::TYPE_SYSTEM;
    napi_value nativeMode = argv[0];
    if (nativeMode == nullptr) {
        return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM,
            "[window][getWindowAvoidArea]msg: Incorrect parameter types");
    }
    uint32_t resultValue = 0;
    napi_get_value_uint32(env, nativeMode, &resultValue);
    avoidAreaType = static_cast<AvoidAreaType>(resultValue);
    errCode = avoidAreaType >= AvoidAreaType::TYPE_END ?
        WmErrorCode::WM_ERROR_INVALID_PARAM : WmErrorCode::WM_OK;
    if (errCode == WmErrorCode::WM_ERROR_INVALID_PARAM) {
        return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM,
            "[window][getWindowAvoidArea]msg: Parameter verification failed");
    }

    if (extensionWindow_ == nullptr) {
        TLOGE(WmsLogTag::WMS_UIEXT, "ExtensionWindow_ is nullptr");
        return NapiThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
            "[window][getWindowAvoidArea]msg: The window is not created or destroyed");
    }
    // getAvoidRect by avoidAreaType
    AvoidArea avoidArea;
    WmErrorCode ret = WM_JS_TO_ERROR_CODE_MAP.at(extensionWindow_->GetAvoidAreaByType(avoidAreaType, avoidArea));
    if (ret != WmErrorCode::WM_OK) {
        TLOGE(WmsLogTag::WMS_UIEXT, "OnGetAvoidAreaByType failed");
        avoidArea.topRect_ = g_emptyRect;
        avoidArea.leftRect_ = g_emptyRect;
        avoidArea.rightRect_ = g_emptyRect;
        avoidArea.bottomRect_ = g_emptyRect;
    }
    napi_value avoidAreaObj = ConvertAvoidAreaToJsValue(env, avoidArea, avoidAreaType);
    if (avoidAreaObj != nullptr) {
        TLOGD(WmsLogTag::WMS_UIEXT, "avoidAreaObj is finish");
        return avoidAreaObj;
    } else {
        TLOGE(WmsLogTag::WMS_UIEXT, "avoidAreaObj is nullptr");
        return NapiThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
            "[window][getWindowAvoidArea]msg: Convert avoid area failed");
    }
}

napi_value JsExtensionWindow::OnRegisterRectChangeCallback(napi_env env, size_t argc, napi_value* argv,
    const sptr<Window>& windowImpl)
{
    if (argc < ARG_COUNT_THREE) {
        TLOGE(WmsLogTag::WMS_UIEXT, "OnRectChange: argc is invalid: %{public}zu", argc);
        return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM,
            "[window][on]msg: Mandatory parameters are left unspecified");
    }
    if (!windowImpl->IsPcWindow()) {
        TLOGE(WmsLogTag::WMS_UIEXT, "Device is not PC");
        return NapiThrowError(env, WmErrorCode::WM_ERROR_DEVICE_NOT_SUPPORT,
            "[window][on]msg: The device not support");
    }
    uint32_t reasons = 0;
    if (!ConvertFromJsValue(env, argv[INDEX_ONE], reasons)) {
        TLOGE(WmsLogTag::WMS_UIEXT, "Failed to convert parameter to rectChangeReasons");
        return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM,
            "[window][on]msg: Incorrect parameters types");
    }
    if (reasons != static_cast<uint32_t>(ComponentRectChangeReason::HOST_WINDOW_RECT_CHANGE)) {
        TLOGE(WmsLogTag::WMS_UIEXT, "Unsupported rect change reasons");
        return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM,
            "[window][on]msg: Parameter verification faile");
    }
    napi_value cbValue = argv[INDEX_TWO];
    if (!NapiIsCallable(env, cbValue)) {
        TLOGE(WmsLogTag::WMS_UIEXT, "Callback(info->argv[2]) is not callable");
        return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM,
            "[window][on]msg: Incorrect parameters types");
    }
    WmErrorCode ret = extensionRegisterManager_->RegisterListener(windowImpl, RECT_CHANGE, CaseType::CASE_WINDOW,
        env, cbValue);
    if (ret != WmErrorCode::WM_OK) {
        TLOGW(WmsLogTag::WMS_UIEXT, "Failed, window [%{public}u, %{public}s], type=%{public}s, reasons=%{public}u",
            windowImpl->GetWindowId(), windowImpl->GetWindowName().c_str(), RECT_CHANGE.c_str(), reasons);
        return NapiThrowError(env, ret);
    }
    TLOGI(WmsLogTag::WMS_UIEXT, "Success, window [%{public}u, %{public}s], type=%{public}s, reasons=%{public}u",
        windowImpl->GetWindowId(), windowImpl->GetWindowName().c_str(), RECT_CHANGE.c_str(), reasons);
    return NapiGetUndefined(env);
}

napi_value JsExtensionWindow::OnRegisterExtensionWindowCallback(napi_env env, napi_callback_info info,
    bool atomicService)
{
    sptr<Window> windowImpl = extensionWindow_->GetWindow();
    if (windowImpl == nullptr) {
        TLOGE(WmsLogTag::WMS_UIEXT, "WindowImpl is nullptr");
        return NapiThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
            "[window][on]msg: The window is not created or destroyed");
    }
    size_t argc = FOUR_PARAMS_SIZE;
    napi_value argv[FOUR_PARAMS_SIZE] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc < ARG_COUNT_TWO) {
        TLOGE(WmsLogTag::WMS_UIEXT, "Argc is invalid: %{public}zu", argc);
        return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM,
            "[window][on]msg: Mandatory parameters are left unspecified");
    }
    std::string cbType;
    if (!ConvertFromJsValue(env, argv[INDEX_ZERO], cbType)) {
        TLOGE(WmsLogTag::WMS_UIEXT, "Failed to convert parameter to callbackType");
        return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM,
            "[window][on]msg: Incorrect parameters types");
    }
    if (cbType == RECT_CHANGE) {
        return OnRegisterRectChangeCallback(env, argc, argv, windowImpl);
    }
    if (atomicService) {
        if (IsInvalidListener(cbType)) {
            return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_CALLING,
                "[window][on]");
        }
        if (IsEmptyListener(cbType)) {
            return NapiGetUndefined(env);
        }
        if (IsUnsupportListener(cbType)) {
            return NapiThrowError(env, WmErrorCode::WM_ERROR_DEVICE_NOT_SUPPORT,
                "[window][on]msg: The device not support");
        }
    }
    napi_value value = argv[INDEX_ONE];
    if (!NapiIsCallable(env, value)) {
        TLOGE(WmsLogTag::WMS_UIEXT, "Callback(info->argv[1]) is not callable");
        return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM,
            "[window][on]msg: Incorrect parameters types");
    }
    WmErrorCode ret = WmErrorCode::WM_OK;
    if (atomicService) {
        ret = extensionRegisterManager_->AtomicServiceRegisterListener(windowImpl, cbType, CaseType::CASE_WINDOW,
            env, value);
    } else {
        ret = extensionRegisterManager_->RegisterListener(windowImpl, cbType, CaseType::CASE_WINDOW,
            env, value);
    }
    if (ret != WmErrorCode::WM_OK) {
        TLOGE(WmsLogTag::WMS_UIEXT, "Callback(info->argv[1]) is not callable");
        return NapiThrowError(env, ret, "[window][on]");
    }
    TLOGI(WmsLogTag::WMS_UIEXT, "Register end, window [%{public}u, %{public}s], type=%{public}s",
          windowImpl->GetWindowId(), windowImpl->GetWindowName().c_str(), cbType.c_str());
    return NapiGetUndefined(env);
}

napi_value JsExtensionWindow::OnUnRegisterExtensionWindowCallback(napi_env env, napi_callback_info info,
    bool atomicService)
{
    sptr<Window> windowImpl = extensionWindow_->GetWindow();
    if (windowImpl == nullptr) {
        TLOGE(WmsLogTag::WMS_UIEXT, "windowImpl is nullptr");
        return NapiThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
            "[window][off]msg: The window is not created or destroyed");
    }
    size_t argc = FOUR_PARAMS_SIZE;
    napi_value argv[FOUR_PARAMS_SIZE] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc < ARG_COUNT_ONE) {
        TLOGE(WmsLogTag::WMS_UIEXT, "Argc is invalid: %{public}zu", argc);
        return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM,
            "[window][off]msg: Mandatory parameters are left unspecified");
    }
    std::string cbType;
    if (!ConvertFromJsValue(env, argv[INDEX_ZERO], cbType)) {
        TLOGE(WmsLogTag::WMS_UIEXT, "Failed to convert parameter to callbackType");
        return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM,
            "[window][off]msg: Incorrect parameters types");
    }
    if (cbType == RECT_CHANGE) {
        if (!windowImpl->IsPcWindow()) {
            TLOGE(WmsLogTag::WMS_UIEXT, "Device is not PC");
            return NapiThrowError(env, WmErrorCode::WM_ERROR_DEVICE_NOT_SUPPORT,
                "[window][off]msg: The device not support");
        }
    }
    if (atomicService) {
        if (IsInvalidListener(cbType)) {
            return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_CALLING,
                "[window][off]");
        }
        if (IsEmptyListener(cbType)) {
            return NapiGetUndefined(env);
        }
        if (IsUnsupportListener(cbType)) {
            return NapiThrowError(env, WmErrorCode::WM_ERROR_DEVICE_NOT_SUPPORT,
                "[window][off]msg: The device not support");
        }
    }

    napi_value value = nullptr;
    WmErrorCode ret = WmErrorCode::WM_OK;
    if (argc > ARG_COUNT_ONE && argv[INDEX_ONE] != nullptr && NapiIsCallable(env, argv[INDEX_ONE])) {
        value = argv[INDEX_ONE];
    }
    if (atomicService) {
        ret = extensionRegisterManager_->AtomicServiceUnregisterListener(windowImpl, cbType,
            CaseType::CASE_WINDOW, env, value);
    } else {
        ret = extensionRegisterManager_->UnregisterListener(windowImpl, cbType, CaseType::CASE_WINDOW, env, value);
    }

    if (ret != WmErrorCode::WM_OK) {
        return NapiThrowError(env, ret, "[window][off]");
    }
    TLOGI(WmsLogTag::WMS_UIEXT, "UnRegister end, window [%{public}u, %{public}s], type=%{public}s",
          windowImpl->GetWindowId(), windowImpl->GetWindowName().c_str(), cbType.c_str());
    return NapiGetUndefined(env);
}

napi_value JsExtensionWindow::OnHideNonSecureWindows(napi_env env, napi_callback_info info)
{
    napi_value result = nullptr;
    std::shared_ptr<NapiAsyncTask> napiAsyncTask = CreateEmptyAsyncTask(env, nullptr, &result);
    if (extensionWindow_ == nullptr) {
        TLOGE(WmsLogTag::WMS_UIEXT, "extensionWindow_ is nullptr");
        napiAsyncTask->Reject(env, CreateJsError(env, static_cast<int32_t>(WmErrorCode::WM_ERROR_STATE_ABNORMALLY),
                "[window][hideNonSecureWindows]msg: extensionWindow_ is nullptr"));
        return result;
    }
    sptr<Window> windowImpl = extensionWindow_->GetWindow();
    if (windowImpl == nullptr) {
        TLOGE(WmsLogTag::WMS_UIEXT, "windowImpl is nullptr");
        napiAsyncTask->Reject(env, CreateJsError(env, static_cast<int32_t>(WmErrorCode::WM_ERROR_STATE_ABNORMALLY),
                "[window][hideNonSecureWindows]msg: windowImpl is nullptr"));
        return result;
    }
    size_t argc = 4;
    napi_value argv[4] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc < 1) {
        TLOGE(WmsLogTag::WMS_UIEXT, "Argc is invalid: %{public}zu", argc);
        return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM,
            "[window][hideNonSecureWindows]msg: Argc is invalid");
    }
    bool shouldHide = false;
    if (!ConvertFromJsValue(env, argv[0], shouldHide)) {
        TLOGE(WmsLogTag::WMS_UIEXT, "Failed to convert parameter to bool");
        return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM,
            "[window][hideNonSecureWindows]msg: Failed to convert parameter to bool");
    }

    auto ret = WM_JS_TO_ERROR_CODE_MAP.at(extensionWindow_->HideNonSecureWindows(shouldHide));
    const char* const where = __func__;
    auto asyncTask = [where, env, ret, task = napiAsyncTask]() mutable {
        if (ret == WmErrorCode::WM_OK) {
            task->Resolve(env, NapiGetUndefined(env));
        } else {
            TLOGNE(WmsLogTag::WMS_UIEXT, "%{public}s failed, code: %{public}d", where, ret);
            task->Reject(env, CreateJsError(env, static_cast<int32_t>(ret),
                "[window][hideNonSecureWindows]msg: hideNonSecureWindows failed"));
        }
    };
    if (napi_send_event(env, asyncTask, napi_eprio_high, "OnHideNonSecureWindows") != napi_status::napi_ok) {
        napiAsyncTask->Reject(env,
            CreateJsError(env, static_cast<int32_t>(WmErrorCode::WM_ERROR_STATE_ABNORMALLY),
                "[window][hideNonSecureWindows]msg: Failed to send event"));
    }
    TLOGI(WmsLogTag::WMS_UIEXT, "end, window [%{public}u, %{public}s], shouldHide:%{public}u",
          windowImpl->GetWindowId(), windowImpl->GetWindowName().c_str(), shouldHide);
    return result;
}

napi_value JsExtensionWindow::OnSetWaterMarkFlag(napi_env env, napi_callback_info info)
{
    napi_value result = nullptr;
    std::shared_ptr<NapiAsyncTask> napiAsyncTask = CreateEmptyAsyncTask(env, nullptr, &result);
    if (extensionWindow_ == nullptr) {
        TLOGE(WmsLogTag::WMS_UIEXT, "extensionWindow_ is nullptr");
        napiAsyncTask->Reject(env, CreateJsError(env, static_cast<int32_t>(WmErrorCode::WM_ERROR_STATE_ABNORMALLY),
                "[window][setWaterMarkFlag]msg: extensionWindow_ is nullptr"));
        return result;
    }
    sptr<Window> windowImpl = extensionWindow_->GetWindow();
    if (windowImpl == nullptr) {
        TLOGE(WmsLogTag::WMS_UIEXT, "windowImpl is nullptr");
        napiAsyncTask->Reject(env, CreateJsError(env, static_cast<int32_t>(WmErrorCode::WM_ERROR_STATE_ABNORMALLY),
                "[window][setWaterMarkFlag]msg: windowImpl is nullptr"));
        return result;
    }
    size_t argc = 4;
    napi_value argv[4] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc < 1) {
        TLOGE(WmsLogTag::WMS_UIEXT, "Argc is invalid: %{public}zu", argc);
        return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM,
            "[window][setWaterMarkFlag]msg: Argc is invalid");
    }
    bool isEnable = false;
    if (!ConvertFromJsValue(env, argv[0], isEnable)) {
        TLOGE(WmsLogTag::WMS_UIEXT, "Failed to convert parameter to bool");
        return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM,
            "[window][setWaterMarkFlag]msg: Failed to convert parameter to bool");
    }

    auto ret = WM_JS_TO_ERROR_CODE_MAP.at(extensionWindow_->SetWaterMarkFlag(isEnable));
    const char* const where = __func__;
    auto asyncTask = [where, env, ret, task = napiAsyncTask]() mutable {
        if (ret == WmErrorCode::WM_OK) {
            task->Resolve(env, NapiGetUndefined(env));
        } else {
            TLOGNE(WmsLogTag::WMS_UIEXT, "%{public}s failed, code: %{public}d", where, ret);
            task->Reject(env, CreateJsError(env, static_cast<int32_t>(ret),
                "[window][setWaterMarkFlag]msg: setWaterMarkFlag failed"));
        }
    };
    if (napi_send_event(env, asyncTask, napi_eprio_high, "OnSetWaterMarkFlag") != napi_status::napi_ok) {
        napiAsyncTask->Reject(env,
            CreateJsError(env, static_cast<int32_t>(WmErrorCode::WM_ERROR_STATE_ABNORMALLY),
                "[window][setWaterMarkFlag]msg: Failed to send event"));
    }
    TLOGI(WmsLogTag::WMS_UIEXT, "end, window [%{public}u, %{public}s], isEnable:%{public}u.",
          windowImpl->GetWindowId(), windowImpl->GetWindowName().c_str(), isEnable);
    return result;
}

napi_value JsExtensionWindow::OnHidePrivacyContentForHost(napi_env env, napi_callback_info info)
{
    napi_value result = nullptr;
    std::shared_ptr<NapiAsyncTask> napiAsyncTask = CreateEmptyAsyncTask(env, nullptr, &result);
    if (extensionWindow_ == nullptr) {
        TLOGE(WmsLogTag::WMS_UIEXT, "extensionWindow_ is nullptr");
        napiAsyncTask->Reject(env, CreateJsError(env, static_cast<int32_t>(WmErrorCode::WM_ERROR_STATE_ABNORMALLY),
                "[window][hidePrivacyContentForHost]msg: extensionWindow_ is nullptr"));
        return result;
    }
    sptr<Window> windowImpl = extensionWindow_->GetWindow();
    if (windowImpl == nullptr) {
        TLOGE(WmsLogTag::WMS_UIEXT, "windowImpl is nullptr");
        napiAsyncTask->Reject(env, CreateJsError(env, static_cast<int32_t>(WmErrorCode::WM_ERROR_STATE_ABNORMALLY),
                "[window][hidePrivacyContentForHost]msg: windowImpl is nullptr"));
        return result;
    }
    size_t argc = 4;
    napi_value argv[4] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc < 1) {
        TLOGE(WmsLogTag::WMS_UIEXT, "Argc is invalid: %{public}zu", argc);
        return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM,
            "[window][hidePrivacyContentForHost]msg: Argc is invalid");
    }
    bool needHide = false;
    if (!ConvertFromJsValue(env, argv[0], needHide)) {
        TLOGE(WmsLogTag::WMS_UIEXT, "Failed to convert parameter to bool");
        return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM,
            "[window][hidePrivacyContentForHost]msg: Failed to convert parameter to bool");
    }

    auto ret = WM_JS_TO_ERROR_CODE_MAP.at(extensionWindow_->HidePrivacyContentForHost(needHide));
    const char* const where = __func__;
    auto asyncTask = [where, env, ret, task = napiAsyncTask]() mutable {
        if (ret == WmErrorCode::WM_OK) {
            task->Resolve(env, NapiGetUndefined(env));
        } else {
            TLOGNE(WmsLogTag::WMS_UIEXT, "%{public}s failed, code: %{public}d", where, ret);
            task->Reject(env, CreateJsError(env, static_cast<int32_t>(ret),
                "[window][hidePrivacyContentForHost]msg: hidePrivacyContentForHost failed"));
        }
    };
    if (napi_send_event(env, asyncTask, napi_eprio_high, "OnHidePrivacyContentForHost") != napi_status::napi_ok) {
        napiAsyncTask->Reject(env,
            CreateJsError(env, static_cast<int32_t>(WmErrorCode::WM_ERROR_STATE_ABNORMALLY),
                "[window][hidePrivacyContentForHost]msg: Failed to send event"));
    }

    TLOGI(WmsLogTag::WMS_UIEXT, "finished, window [%{public}u, %{public}s], needHide:%{public}u.",
          windowImpl->GetWindowId(), windowImpl->GetWindowName().c_str(), needHide);
    return result;
}

napi_value JsExtensionWindow::GetProperties(napi_env env, napi_callback_info info)
{
    TLOGI(WmsLogTag::WMS_UIEXT, "in");
    napi_value jsThis;
    NAPI_CALL(env, napi_get_cb_info(env, info, nullptr, nullptr, &jsThis, nullptr));

    JsExtensionWindow* jsExtensionWindow = nullptr;
    NAPI_CALL(env, napi_unwrap(env, jsThis, reinterpret_cast<void**>(&jsExtensionWindow)));
    if (!jsExtensionWindow || !jsExtensionWindow->extensionWindow_) {
        TLOGE(WmsLogTag::WMS_UIEXT, "window is nullptr");
        return nullptr;
    }
    sptr<Rosen::Window> window = jsExtensionWindow->extensionWindow_->GetWindow();
    return CreateJsExtensionWindowPropertiesObject(env, window);
}

napi_value JsExtensionWindow::OnCreateSubWindowWithOptions(napi_env env, napi_callback_info info)
{
    if (extensionWindow_ == nullptr) {
        TLOGE(WmsLogTag::WMS_UIEXT, "extensionWindow is null");
        napi_throw(env, JsErrUtils::CreateJsError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
            "[window][createSubWindowWithOptions]msg: The window is not created or destroyed"));
        return NapiGetUndefined(env);
    }
    size_t argc = 4;
    napi_value argv[4] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    std::string windowName;
    if (!ConvertFromJsValue(env, argv[0], windowName)) {
        TLOGE(WmsLogTag::WMS_UIEXT, "Failed to convert parameter to windowName");
        napi_throw(env, JsErrUtils::CreateJsError(env, WmErrorCode::WM_ERROR_INVALID_PARAM,
            "[window][createSubWindowWithOptions]msg: Failed to convert parameter to windowName"));
        return NapiGetUndefined(env);
    }
    sptr<WindowOption> option = new WindowOption();
    if (!ParseSubWindowOptions(env, argv[1], option)) {
        TLOGE(WmsLogTag::WMS_UIEXT, "Get invalid options param");
        napi_throw(env, JsErrUtils::CreateJsError(env, WmErrorCode::WM_ERROR_INVALID_PARAM,
            "[window][createSubWindowWithOptions]msg: Failed to convert parameter to options"));
        return NapiGetUndefined(env);
    }
    if ((option->GetWindowFlags() & static_cast<uint32_t>(WindowFlag::WINDOW_FLAG_IS_APPLICATION_MODAL)) &&
        !extensionWindow_->IsPcOrPadFreeMultiWindowMode()) {
        TLOGE(WmsLogTag::WMS_SUB, "device not support");
        napi_throw(env, JsErrUtils::CreateJsError(env, WmErrorCode::WM_ERROR_DEVICE_NOT_SUPPORT,
            "[window][createSubWindowWithOptions]msg: Device not support"));
        return NapiGetUndefined(env);
    }
    if (option->GetWindowTopmost() && !Permission::IsSystemCalling() && !Permission::IsStartByHdcd()) {
        TLOGE(WmsLogTag::WMS_SUB, "Modal subwindow has topmost, but no system permission");
        napi_throw(env, JsErrUtils::CreateJsError(env, WmErrorCode::WM_ERROR_NOT_SYSTEM_APP,
            "[window][createSubWindowWithOptions]msg: Insufficient system permissions"));
        return NapiGetUndefined(env);
    }
    bool followCreatorLifecycle = false;
    if (argc >= ARG_COUNT_THREE && GetType(env, argv[INDEX_TWO]) != napi_undefined &&
        !ConvertFromJsValue(env, argv[INDEX_TWO], followCreatorLifecycle)) {
        TLOGE(WmsLogTag::WMS_SUB, "Failed to convert followCreatorLifecycle parameter to bool");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(WmErrorCode::WM_ERROR_INVALID_PARAM)));
        return NapiGetUndefined(env);
    }
    option->SetFollowCreatorLifecycle(followCreatorLifecycle);
    option->SetParentId(hostWindowId_);
    const char* const where = __func__;
    napi_value result = nullptr;
    std::shared_ptr<NapiAsyncTask> napiAsyncTask = CreateEmptyAsyncTask(env, nullptr, &result);
    auto asyncTask = [where, extensionWindow = extensionWindow_, windowName = std::move(windowName),
        windowOption = option, env, task = napiAsyncTask]() mutable {
        auto extWindow = extensionWindow->GetWindow();
        if (extWindow == nullptr) {
            task->Reject(env, JsErrUtils::CreateJsError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
                "[window][createSubWindowWithOptions]msg: Window is nullptr on asyncTask"));
            return;
        }
        windowOption->SetWindowType(Rosen::WindowType::WINDOW_TYPE_APP_SUB_WINDOW);
        windowOption->SetWindowMode(Rosen::WindowMode::WINDOW_MODE_FLOATING);
        windowOption->SetOnlySupportSceneBoard(true);
        windowOption->SetIsUIExtFirstSubWindow(true);
        auto window = Window::Create(windowName, windowOption, extWindow->GetContext());
        if (window == nullptr) {
            task->Reject(env, JsErrUtils::CreateJsError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
                "[window][createSubWindowWithOptions]msg: Create sub window failed."));
            return;
        }
        if (!window->IsTopmost()) {
            extWindow->NotifyModalUIExtensionMayBeCovered(false);
        }
        task->Resolve(env, CreateJsWindowObject(env, window));
        TLOGNI(WmsLogTag::WMS_UIEXT, "%{public}s %{public}s end", where, windowName.c_str());
    };
    if (napi_send_event(env, asyncTask, napi_eprio_high, "OnCreateSubWindowWithOptions") != napi_status::napi_ok) {
        napiAsyncTask->Reject(env, JsErrUtils::CreateJsError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
            "[window][createSubWindowWithOptions]msg: Internal task error"));
    }
    return result;
}

napi_value JsExtensionWindow::OnOccupyEvents(napi_env env, napi_callback_info info)
{
    size_t argc = FOUR_PARAMS_SIZE;
    napi_value argv[FOUR_PARAMS_SIZE] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc < 1) {
        TLOGE(WmsLogTag::WMS_UIEXT, "Argc is invalid: %{public}zu", argc);
        return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM,
            "[window][occupyEvents]msg: Mandatory parameters are left unspecified");
    }
    int32_t eventFlags = 0;
    if (!ConvertFromJsValue(env, argv[0], eventFlags)) {
        TLOGE(WmsLogTag::WMS_UIEXT, "Failed to convert parameter to int32_t");
        return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM,
            "[window][occupyEvents]msg: Incorrect parameters types");
    }
    napi_value lastParam = (argc <= 1) ? nullptr : (GetType(env, argv[1]) == napi_function ? argv[1] : nullptr);
    napi_value result = nullptr;
    std::shared_ptr<NapiAsyncTask> napiAsyncTask = CreateEmptyAsyncTask(env, lastParam, &result);
    auto asyncTask = [weakToken = std::weak_ptr<ExtensionWindow>(extensionWindow_), eventFlags, env,
        task = napiAsyncTask] {
        auto weakWindow = weakToken.lock();
        if (weakWindow == nullptr) {
            TLOGNE(WmsLogTag::WMS_UIEXT, "window is nullptr");
            task->Reject(env, JsErrUtils::CreateJsError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
                "[window][occupyEvents]msg: The window is not created or destroyed"));
            return;
        }
        auto ret = WM_JS_TO_ERROR_CODE_MAP.at(weakWindow->OccupyEvents(eventFlags));
        if (ret == WmErrorCode::WM_OK) {
            task->Resolve(env, NapiGetUndefined(env));
        } else {
            TLOGNE(WmsLogTag::WMS_UIEXT, "OnOccupyEvents failed, code: %{public}d", ret);
            task->Reject(env, JsErrUtils::CreateJsError(env, ret, "[window][occupyEvents]"));
        }
    };
    if (napi_send_event(env, asyncTask, napi_eprio_high, "OnOccupyEvents") != napi_status::napi_ok) {
        TLOGE(WmsLogTag::WMS_UIEXT, "napi_send_event failed");
        napiAsyncTask->Reject(env, JsErrUtils::CreateJsError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
            "[window][occupyEvents]msg: Internal task error"));
    }
    return result;
}
 
napi_value JsExtensionWindow::OnSetWindowLayoutFullScreen(napi_env env, napi_callback_info info)
{
    WmErrorCode errCode = WmErrorCode::WM_OK;
    size_t argc = 4;
    napi_value argv[4] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc < 1) { // 1: params num
        TLOGE(WmsLogTag::WMS_IMMS, "Argc is invalid %{public}zu", argc);
        return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM,
            "[window][setWindowLayoutFullScreen]msg: Mandatory parameters are left unspecified");
    }
    bool isLayoutFullScreen = false;
    napi_value nativeVal = argv[0];
    if (nativeVal == nullptr) {
        TLOGE(WmsLogTag::WMS_IMMS, "Failed to convert parameter to isLayoutFullScreen");
        errCode = WmErrorCode::WM_ERROR_INVALID_PARAM;
    } else {
        CHECK_NAPI_RETCODE(errCode, WmErrorCode::WM_ERROR_INVALID_PARAM,
            napi_get_value_bool(env, nativeVal, &isLayoutFullScreen));
    }
    if (errCode != WmErrorCode::WM_OK) {
        return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM,
            "[window][setWindowLayoutFullScreen]msg: Incorrect parameter types");
    }
    const char* const where = __func__;
    NapiAsyncTask::CompleteCallback complete =
        [weakToken = wptr<Window>(extensionWindow_->GetWindow()), isLayoutFullScreen, where](napi_env env,
            NapiAsyncTask& task, int32_t status) {
            auto window = weakToken.promote();
            if (window == nullptr) {
                TLOGNE(WmsLogTag::WMS_IMMS, "%{public}s window is nullptr", where);
                task.Reject(env, JsErrUtils::CreateJsError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
                    "[window][setWindowLayoutFullScreen]msg: The window is not created or destroyed"));
                return;
            }
            // compatibleModeInPc need apply avoidArea method
            if (window->IsPcOrPadFreeMultiWindowMode() && !window->GetCompatibleModeInPc()) {
                TLOGNE(WmsLogTag::WMS_IMMS, "%{public}s device not support", where);
                task.Resolve(env, NapiGetUndefined(env));
                return;
            }
            WmErrorCode ret = WM_JS_TO_ERROR_CODE_MAP.at(window->SetLayoutFullScreen(isLayoutFullScreen));
            if (ret == WmErrorCode::WM_OK) {
                task.Resolve(env, NapiGetUndefined(env));
            } else {
                TLOGNE(WmsLogTag::WMS_IMMS, "%{public}s failed, ret %{public}d", where, ret);
                task.Reject(env, JsErrUtils::CreateJsError(env, ret,
                    "[window][setWindowLayoutFullScreen]"));
            }
        };

    napi_value lastParam = (argc <= 1) ? nullptr :
        ((argv[1] != nullptr && GetType(env, argv[1]) == napi_function) ? argv[1] : nullptr);
    napi_value result = nullptr;
    auto asyncTask = CreateAsyncTask(env, lastParam, nullptr,
        std::make_unique<NapiAsyncTask::CompleteCallback>(std::move(complete)), &result);
    NapiAsyncTask::Schedule("JsExtensionWindow::OnSetWindowLayoutFullScreen", env, std::move(asyncTask));
    return result;
}

napi_value JsExtensionWindow::OnGetWindowColorSpace(napi_env env, napi_callback_info info)
{
    sptr<Window> window = extensionWindow_->GetWindow();
    if (window == nullptr) {
        TLOGE(WmsLogTag::WMS_UIEXT, "window is nullptr");
        return NapiThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
            "[window][getWindowColorSpace]msg: The window is not created or destroyed");
    }
    ColorSpace colorSpace = window->GetColorSpace();
    TLOGI(WmsLogTag::WMS_UIEXT, "end, window [%{public}u, %{public}s] colorSpace=%{public}u",
        window->GetWindowId(), window->GetWindowName().c_str(), static_cast<uint32_t>(colorSpace));

    return CreateJsValue(env, static_cast<uint32_t>(colorSpace));
}

napi_value JsExtensionWindow::OnSetWindowColorSpace(napi_env env, napi_callback_info info)
{
    WmErrorCode errCode = WmErrorCode::WM_OK;
    ColorSpace colorSpace = ColorSpace::COLOR_SPACE_DEFAULT;
    size_t argc = 4;
    napi_value argv[4] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc < 1) { // 1: params num
        TLOGE(WmsLogTag::WMS_UIEXT, "Argc is invalid: %{public}zu", argc);
        return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM,
            "[window][setWindowColorSpace]msg: Mandatory parameters are left unspecified");
    }
    napi_value nativeType = argv[0];
    if (nativeType == nullptr) {
        TLOGE(WmsLogTag::WMS_UIEXT, "Failed to convert parameter to ColorSpace");
        return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM,
            "[window][setWindowColorSpace]msg: Incorrect parameter types");
    }
    uint32_t resultValue = 0;
    CHECK_NAPI_RETCODE(errCode, WmErrorCode::WM_ERROR_INVALID_PARAM,
        napi_get_value_uint32(env, nativeType, &resultValue));
    colorSpace = static_cast<ColorSpace>(resultValue);
    if (colorSpace > ColorSpace::COLOR_SPACE_WIDE_GAMUT || colorSpace < ColorSpace::COLOR_SPACE_DEFAULT) {
        TLOGE(WmsLogTag::WMS_UIEXT, "ColorSpace %{public}u invalid!", static_cast<uint32_t>(colorSpace));
        errCode = WmErrorCode::WM_ERROR_INVALID_PARAM;
    }
    if (errCode == WmErrorCode::WM_ERROR_INVALID_PARAM) {
        return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM,
            "[window][setWindowColorSpace]msg: Parameter verification failed");
    }

    napi_value lastParam = (argc <= 1) ? nullptr :
        ((argv[1] != nullptr && GetType(env, argv[1]) == napi_function) ? argv[1] : nullptr);
    napi_value result = nullptr;
    std::shared_ptr<NapiAsyncTask> napiAsyncTask = CreateEmptyAsyncTask(env, lastParam, &result);
    const char* const where = __func__;
    auto asyncTask = [weakToken = wptr<Window>(extensionWindow_->GetWindow()), colorSpace, where, env,
        task = napiAsyncTask] {
        auto weakWindow = weakToken.promote();
        if (weakWindow == nullptr) {
            WLOGFE("window is nullptr");
            task->Reject(env, JsErrUtils::CreateJsError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
                "[window][setWindowColorSpace]msg: The window is not created or destroyed"));
            return;
        }
        weakWindow->SetColorSpace(colorSpace);
        task->Resolve(env, NapiGetUndefined(env));
        WLOGI("%{public}s end, window [%{public}u, %{public}s] colorSpace=%{public}u",
            where, weakWindow->GetWindowId(), weakWindow->GetWindowName().c_str(),
            static_cast<uint32_t>(colorSpace));
    };
    if (napi_send_event(env, asyncTask, napi_eprio_high, "OnSetWindowColorSpace") != napi_status::napi_ok) {
        TLOGE(WmsLogTag::WMS_IMMS, "napi_send_event failed");
        napiAsyncTask->Reject(env, JsErrUtils::CreateJsError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
            "[window][setWindowColorSpace]msg: Internal task error"));
    }
    return result;
}

napi_value JsExtensionWindow::OnSetWindowPrivacyMode(napi_env env, napi_callback_info info)
{
    WmErrorCode errCode = WmErrorCode::WM_OK;
    size_t argc = 4;
    napi_value argv[4] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc < 1) { // 1: params num
        WLOGFE("Argc is invalid: %{public}zu", argc);
        return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM,
            "[window][setWindowPrivacyMode]msg: Mandatory parameters are left unspecified");
    }
    bool isPrivacyMode = false;
    napi_value nativeVal = argv[0];
    if (nativeVal == nullptr) {
        WLOGFE("Failed to convert parameter to isPrivacyMode");
        errCode = WmErrorCode::WM_ERROR_INVALID_PARAM;
    } else {
        CHECK_NAPI_RETCODE(errCode, WmErrorCode::WM_ERROR_INVALID_PARAM,
            napi_get_value_bool(env, nativeVal, &isPrivacyMode));
    }
    if (errCode == WmErrorCode::WM_ERROR_INVALID_PARAM) {
        return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM,
            "[window][setWindowPrivacyMode]msg: Incorrect parameter types");
    }
    const char* const where = __func__;
    napi_value lastParam = (argc <= 1) ? nullptr :
        ((argv[1] != nullptr && GetType(env, argv[1]) == napi_function) ? argv[1] : nullptr);
    napi_value result = nullptr;
    std::shared_ptr<NapiAsyncTask> napiAsyncTask = CreateEmptyAsyncTask(env, lastParam, &result);
    auto asyncTask = [weakToken = wptr<Window>(extensionWindow_->GetWindow()), isPrivacyMode, where, env,
        task = napiAsyncTask] {
        auto weakWindow = weakToken.promote();
        if (weakWindow == nullptr) {
            task->Reject(env,
                JsErrUtils::CreateJsError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
                    "[window][setWindowPrivacyMode]msg: The window is not created or destroyed"));
            return;
        }
        WmErrorCode ret = WM_JS_TO_ERROR_CODE_MAP.at(weakWindow->SetPrivacyMode(isPrivacyMode));
        if (ret == WmErrorCode::WM_ERROR_NO_PERMISSION) {
            task->Reject(env, JsErrUtils::CreateJsError(env, ret,
                "[window][setWindowPrivacyMode]msg: Need PRIVACY_WINDOW permission"));
            WLOGI("%{public}s failed, window [%{public}u, %{public}s] mode=%{public}u",
                where, weakWindow->GetWindowId(), weakWindow->GetWindowName().c_str(), isPrivacyMode);
            return;
        }
        task->Resolve(env, NapiGetUndefined(env));
        WLOGI("%{public}s succeed, window [%{public}u, %{public}s] mode=%{public}u",
            where, weakWindow->GetWindowId(), weakWindow->GetWindowName().c_str(), isPrivacyMode);
    };
    if (napi_send_event(env, asyncTask, napi_eprio_high, "OnSetWindowPrivacyMode") != napi_status::napi_ok) {
        TLOGE(WmsLogTag::WMS_IMMS, "napi_send_event failed");
        napiAsyncTask->Reject(env, JsErrUtils::CreateJsError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
            "[window][setWindowPrivacyMode]msg: Internal task error"));
    }
    return result;
}

napi_value JsExtensionWindow::OnSetWindowSystemBarEnable(napi_env env, napi_callback_info info)
{
    size_t argc = FOUR_PARAMS_SIZE;
    napi_value argv[FOUR_PARAMS_SIZE] = { nullptr };
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    std::unordered_map<WindowType, SystemBarProperty> systemBarProperties;
    std::unordered_map<WindowType, SystemBarPropertyFlag> systemBarPropertyFlags;
    if (argc < ARG_COUNT_ONE) {
        return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM,
            "[window][setWindowSystemBarEnable]msg: Mandatory parameters are left unspecified");
    }
    if (!GetSystemBarStatus(env, info, systemBarProperties, systemBarPropertyFlags)) {
        TLOGE(WmsLogTag::WMS_IMMS, "failed to convert parameter to systemBarProperties");
        return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM,
            "[window][setWindowSystemBarEnable]msg: Incorrect parameter types");
    }
    napi_value lastParam = nullptr;
    if (argc >= ARG_COUNT_ONE && argv[INDEX_ZERO] != nullptr && GetType(env, argv[INDEX_ZERO]) == napi_function) {
        lastParam = argv[INDEX_ZERO];
    } else if (argc >= ARG_COUNT_TWO && argv[INDEX_ONE] != nullptr && GetType(env, argv[INDEX_ONE]) == napi_function) {
        lastParam = argv[INDEX_ONE];
    }
    napi_value result = nullptr;
    std::shared_ptr<NapiAsyncTask> napiAsyncTask = CreateEmptyAsyncTask(env, lastParam, &result);
    auto asyncTask = [weakToken = wptr<Window>(extensionWindow_->GetWindow()), env, task = napiAsyncTask,
        systemBarProperties = std::move(systemBarProperties),
        systemBarPropertyFlags = std::move(systemBarPropertyFlags)] {
        auto window = weakToken.promote();
        if (window == nullptr) {
            TLOGNE(WmsLogTag::WMS_IMMS, "window is nullptr");
            task->Reject(env, JsErrUtils::CreateJsError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
                "[window][setWindowSystemBarEnable]msg: The window is not created or destroyed"));
            return;
        }
        auto errCode = WM_JS_TO_ERROR_CODE_MAP.at(
            window->UpdateSystemBarProperties(systemBarProperties, systemBarPropertyFlags));
        if (errCode == WmErrorCode::WM_OK) {
            task->Resolve(env, NapiGetUndefined(env));
        } else {
            TLOGNE(WmsLogTag::WMS_IMMS, "set system bar enable failed, errcode: %{public}d", errCode);
            task->Reject(env, JsErrUtils::CreateJsError(env, errCode,
                "[window][setWindowSystemBarEnable]"));
        }
    };
    if (napi_send_event(env, asyncTask, napi_eprio_high, "OnSetWindowSystemBarEnable") != napi_status::napi_ok) {
        TLOGE(WmsLogTag::WMS_IMMS, "napi_send_event failed");
        napiAsyncTask->Reject(env, JsErrUtils::CreateJsError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
            "[window][setWindowSystemBarEnable]msg: Internal task error"));
    }
    return result;
}

napi_value JsExtensionWindow::OnGetGestureBackEnabled(napi_env env, napi_callback_info info)
{
    sptr<Window> windowImpl = extensionWindow_->GetWindow();
    if (windowImpl == nullptr) {
        TLOGE(WmsLogTag::WMS_IMMS, "extensionWindow is nullptr");
        return NapiThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
            "[window][isGestureBackEnabled]msg: The window is not created or destroyed");
    }
    bool enable = true;
    WmErrorCode ret = WM_JS_TO_ERROR_CODE_MAP.at(windowImpl->GetGestureBackEnabled(enable));
    if (ret != WmErrorCode::WM_OK) {
        TLOGE(WmsLogTag::WMS_IMMS, "get failed, ret %{public}d", ret);
        return NapiThrowError(env, WmErrorCode::WM_ERROR_SYSTEM_ABNORMALLY,
            "[window][isGestureBackEnabled]");
    }
    TLOGI(WmsLogTag::WMS_IMMS, "win [%{public}u, %{public}s] enable %{public}u",
        windowImpl->GetWindowId(), windowImpl->GetWindowName().c_str(), enable);
    return CreateJsValue(env, enable);
}

napi_value JsExtensionWindow::OnSetGestureBackEnabled(napi_env env, napi_callback_info info)
{
    size_t argc = FOUR_PARAMS_SIZE;
    napi_value argv[FOUR_PARAMS_SIZE] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc < INDEX_ONE) {
        TLOGE(WmsLogTag::WMS_IMMS, "argc is invalid: %{public}zu", argc);
        return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM,
            "[window][setGestureBackEnabled]msg: Mandatory parameters are left unspecified");
    }
    bool enabled = true;
    if (argv[INDEX_ZERO] == nullptr || napi_get_value_bool(env, argv[INDEX_ZERO], &enabled) != napi_ok) {
        TLOGE(WmsLogTag::WMS_IMMS, "failed to convert parameter to enabled");
        return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM,
            "[window][setGestureBackEnabled]msg: Incorrect parameter types");
    }
    std::shared_ptr<WmErrorCode> errCodePtr = std::make_shared<WmErrorCode>(WmErrorCode::WM_OK);
    const char* const where = __func__;
    auto execute = [weakToken = wptr<Window>(extensionWindow_->GetWindow()), errCodePtr, enabled, where] {
        auto window = weakToken.promote();
        if (window == nullptr) {
            TLOGNE(WmsLogTag::WMS_IMMS, "%{public}s window is nullptr", where);
            *errCodePtr = WmErrorCode::WM_ERROR_STATE_ABNORMALLY;
            return;
        }
        *errCodePtr = WM_JS_TO_ERROR_CODE_MAP.at(window->SetGestureBackEnabled(enabled));
    };
    auto complete = [errCodePtr, where](napi_env env, NapiAsyncTask& task, int32_t status) {
        if (*errCodePtr == WmErrorCode::WM_OK) {
            task.Resolve(env, NapiGetUndefined(env));
        } else {
            TLOGNE(WmsLogTag::WMS_IMMS, "%{public}s set failed, ret %{public}d", where, *errCodePtr);
            task.Reject(env, JsErrUtils::CreateJsError(env, *errCodePtr,
                "[window][setGestureBackEnabled]"));
        }
    };
    napi_value result = nullptr;
    NapiAsyncTask::Schedule("JsExtensionWindow::OnSetGestureBackEnabled",
        env, CreateAsyncTaskWithLastParam(env, nullptr, std::move(execute), std::move(complete), &result));
    return result;
}

napi_value JsExtensionWindow::OnSetImmersiveModeEnabledState(napi_env env, napi_callback_info info)
{
    size_t argc = 4;
    napi_value argv[4] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc != 1) {
        TLOGW(WmsLogTag::WMS_IMMS, "argc is invalid %{public}zu", argc);
        return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM,
            "[window][setImmersiveModeEnabledState]msg: Mandatory parameters are left unspecified");
    }
    sptr<Window> windowImpl = extensionWindow_->GetWindow();
    if (windowImpl == nullptr) {
        TLOGE(WmsLogTag::WMS_IMMS, "extensionWindow is nullptr");
        return NapiThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
            "[window][setImmersiveModeEnabledState]msg: The window is not created or destroyed");
    }
    if (windowImpl->IsPcOrPadFreeMultiWindowMode()) {
        TLOGE(WmsLogTag::WMS_IMMS, "device not support");
        return NapiGetUndefined(env);
    }
    napi_value nativeVal = argv[0];
    if (nativeVal == nullptr) {
        TLOGE(WmsLogTag::WMS_IMMS, "Failed to convert parameter to enable");
        return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM,
            "[window][setImmersiveModeEnabledState]msg: Incorrect parameter types");
    }
    bool enable = true;
    napi_get_value_bool(env, nativeVal, &enable);
    TLOGI(WmsLogTag::WMS_IMMS, "enable %{public}d", static_cast<int32_t>(enable));
    WmErrorCode ret = WM_JS_TO_ERROR_CODE_MAP.at(windowImpl->SetImmersiveModeEnabledState(enable));
    if (ret != WmErrorCode::WM_OK) {
        TLOGE(WmsLogTag::WMS_IMMS, "set failed, ret %{public}d", ret);
        return NapiThrowError(env, WmErrorCode::WM_ERROR_SYSTEM_ABNORMALLY,
            "[window][setImmersiveModeEnabledState]msg: Internal IPC error or the window is destroyed");
    }
    TLOGI(WmsLogTag::WMS_IMMS, "win %{public}u set end", windowImpl->GetWindowId());
    return NapiGetUndefined(env);
}

napi_value JsExtensionWindow::OnGetImmersiveModeEnabledState(napi_env env, napi_callback_info info)
{
    sptr<Window> windowImpl = extensionWindow_->GetWindow();
    if (windowImpl == nullptr) {
        TLOGE(WmsLogTag::WMS_IMMS, "extensionWindow is nullptr");
        return NapiThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
            "[window][getImmersiveModeEnabledState]msg: The window is not created or destroyed");
    }
    bool enable = windowImpl->GetImmersiveModeEnabledState();
    TLOGI(WmsLogTag::WMS_IMMS, "win %{public}u isEnabled %{public}u set end", windowImpl->GetWindowId(), enable);
    return CreateJsValue(env, enable);
}

napi_value JsExtensionWindow::OnIsFocused(napi_env env, napi_callback_info info)
{
    sptr<Window> windowImpl = extensionWindow_->GetWindow();
    if (windowImpl == nullptr) {
        TLOGE(WmsLogTag::WMS_FOCUS, "window is nullptr");
        return NapiThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
            "[window][isFocused]msg: The window is not created or destroyed");
    }

    bool isFocused = windowImpl->IsComponentFocused();
    TLOGI(WmsLogTag::WMS_FOCUS, "end, window [%{public}u, %{public}s] isFocused=%{public}u",
        windowImpl->GetWindowId(), windowImpl->GetWindowName().c_str(), isFocused);
    return CreateJsValue(env, isFocused);
}

napi_value JsExtensionWindow::OnIsWindowSupportWideGamut(napi_env env, napi_callback_info info)
{
    const char* const where = __func__;
    size_t argc = 4;
    napi_value argv[4] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    napi_value lastParam = (argc == 0) ? nullptr :
        ((argv[0] != nullptr && GetType(env, argv[0]) == napi_function) ? argv[0] : nullptr);
    napi_value result = nullptr;
    std::shared_ptr<NapiAsyncTask> napiAsyncTask = CreateEmptyAsyncTask(env, lastParam, &result);
    auto asyncTask = [weakToken = wptr<Window>(extensionWindow_->GetWindow()), where, env, task = napiAsyncTask] {
        auto weakWindow = weakToken.promote();
        if (weakWindow == nullptr) {
            TLOGE(WmsLogTag::WMS_IMMS, "window is nullptr or get invalid param");
            task->Reject(env, JsErrUtils::CreateJsError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
                "[window][isWindowSupportWideGamut]msg: The window is not created or destroyed"));
            return;
        }
        bool flag = weakWindow->IsSupportWideGamut();
        task->Resolve(env, CreateJsValue(env, flag));
        TLOGE(WmsLogTag::WMS_IMMS, "%{public}s end, window [%{public}u, %{public}s] ret=%{public}u",
            where, weakWindow->GetWindowId(), weakWindow->GetWindowName().c_str(), flag);
    };
    if (napi_send_event(env, asyncTask, napi_eprio_high, "OnIsWindowSupportWideGamut") != napi_status::napi_ok) {
        TLOGE(WmsLogTag::WMS_IMMS, "napi_send_event failed");
        napiAsyncTask->Reject(env, JsErrUtils::CreateJsError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
            "[window][isWindowSupportWideGamut]msg: Internal task error"));
    }
    return result;
}

napi_value JsExtensionWindow::OnGetGlobalScaledRect(napi_env env, napi_callback_info info)
{
    sptr<Window> windowImpl = extensionWindow_->GetWindow();
    if (windowImpl == nullptr) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "extensionWindow is nullptr");
        return NapiThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
            "[window][getGlobalRect]msg: The window is not created or destroyed");
    }
    Rect globalScaledRect;
    WmErrorCode ret = WM_JS_TO_ERROR_CODE_MAP.at(windowImpl->GetGlobalScaledRect(globalScaledRect));
    if (ret != WmErrorCode::WM_OK) {
        return NapiThrowError(env, ret, "[window][getGlobalRect]");
    }
    TLOGI(WmsLogTag::WMS_LAYOUT, "Window [%{public}u, %{public}s] end",
        windowImpl->GetWindowId(), windowImpl->GetWindowName().c_str());
    napi_value globalScaledRectObj = GetRectAndConvertToJsValue(env, globalScaledRect);
    if (globalScaledRectObj == nullptr) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "globalScaledRectObj is nullptr");
        return NapiThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
            "[window][getGlobalRect]msg: Failed to convert result into JS value object");
    }
    return globalScaledRectObj;
}

napi_value JsExtensionWindow::OnGetStatusBarPropertySync(napi_env env, napi_callback_info info)
{
    sptr<Window> windowImpl = extensionWindow_->GetWindow();
    if (windowImpl == nullptr) {
        TLOGE(WmsLogTag::WMS_IMMS, "window is null");
        return NapiThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
            "[window][getStatusBarProperty]msg: The window is not created or destroyed");
    }
    auto objValue = GetHostStatusBarPropertyObject(env, windowImpl);
    if (objValue == nullptr) {
        TLOGE(WmsLogTag::WMS_IMMS, "get property failed");
        return NapiThrowError(env, WmErrorCode::WM_ERROR_SYSTEM_ABNORMALLY,
            "[window][getStatusBarProperty]msg: Internal task error");
    }
    return objValue;
}

napi_value JsExtensionWindow::OnSetStatusBarColor(napi_env env, napi_callback_info info)
{
    size_t argc = FOUR_PARAMS_SIZE;
    napi_value argv[FOUR_PARAMS_SIZE] = { nullptr };
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc < ARG_COUNT_ONE || argv[INDEX_ZERO] == nullptr) {
        TLOGE(WmsLogTag::WMS_IMMS, "no enough arguments");
        return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM);
    }
    uint32_t contentColor = 0;
    if (!ParseColorMetrics(env, argv[INDEX_ZERO], contentColor)) {
        TLOGE(WmsLogTag::WMS_IMMS, "parse color failed");
        return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM);
    }
    napi_value result = nullptr;
    napi_value lastParam = (argc <= ARG_COUNT_ONE) ? nullptr :
        (GetType(env, argv[INDEX_ONE]) == napi_function ? argv[INDEX_ONE] : nullptr);
    std::shared_ptr<NapiAsyncTask> napiAsyncTask = CreateEmptyAsyncTask(env, lastParam, &result);
    TLOGI(WmsLogTag::WMS_IMMS, "target color: %{public}u", contentColor);
    auto asyncTask = [weakToken = wptr<Window>(extensionWindow_->GetWindow()), env, contentColor,
        task = napiAsyncTask] {
        auto window = weakToken.promote();
        if (window == nullptr) {
            TLOGNE(WmsLogTag::WMS_IMMS, "window is null");
            task->Reject(env, CreateJsError(env, static_cast<int32_t>(WmErrorCode::WM_ERROR_STATE_ABNORMALLY),
                "[window][setStatusBarColor]msg: invalid window"));
            return;
        }
        auto errCode = window->SetStatusBarColorForExtension(contentColor);
        if (errCode == WMError::WM_OK) {
            task->Resolve(env, NapiGetUndefined(env));
        } else {
            TLOGNE(WmsLogTag::WMS_IMMS, "SetStatusBarColor error: %{public}d", errCode);
            task->Reject(env, CreateJsError(env, static_cast<int32_t>(WM_JS_TO_ERROR_CODE_MAP.at(errCode)),
                "[window][setStatusBarColor]msg: update status bar color failed"));
        }
    };
    if (napi_send_event(env, asyncTask, napi_eprio_high, "OnSetStatusBarColor") != napi_status::napi_ok) {
        TLOGE(WmsLogTag::WMS_IMMS, "napi_send_event failed");
        napiAsyncTask->Reject(env, CreateJsError(env, static_cast<int32_t>(WmErrorCode::WM_ERROR_SYSTEM_ABNORMALLY),
            "[window][setStatusBarColor]msg: send event failed"));
    }
    return result;
}

napi_value JsExtensionWindow::OnUnsupportAsyncCall(napi_env env, napi_callback_info info)
{
    size_t argc = 4;
    napi_value argv[4] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    napi_value lastParam = (argc <= 0) ? nullptr :
        ((argv[argc - 1] != nullptr && GetType(env, argv[argc - 1]) == napi_function) ? argv[argc - 1] : nullptr);
    napi_value result = nullptr;
    std::shared_ptr<NapiAsyncTask> napiAsyncTask = CreateEmptyAsyncTask(env, lastParam, &result);
    auto asyncTask = [env, task = napiAsyncTask] {
        task->Reject(env, CreateJsError(env, static_cast<int32_t>(WmErrorCode::WM_ERROR_DEVICE_NOT_SUPPORT)));
    };
    if (napi_send_event(env, asyncTask, napi_eprio_high, "OnUnsupportAsyncCall") != napi_status::napi_ok) {
        napiAsyncTask->Reject(env,
            CreateJsError(env, static_cast<int32_t>(WmErrorCode::WM_ERROR_STATE_ABNORMALLY), "failed to send event"));
    }
    return result;
}

napi_value JsExtensionWindow::OnEmptyAsyncCall(napi_env env, napi_callback_info info)
{
    size_t argc = 4;
    napi_value argv[4] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    napi_value lastParam = (argc <= 0) ? nullptr :
        ((argv[argc - 1] != nullptr && GetType(env, argv[argc - 1]) == napi_function) ? argv[argc - 1] : nullptr);
    napi_value result = nullptr;
    std::shared_ptr<NapiAsyncTask> napiAsyncTask = CreateEmptyAsyncTask(env, lastParam, &result);
    auto asyncTask = [env, task = napiAsyncTask] {
        task->Resolve(env, NapiGetUndefined(env));
    };
    if (napi_send_event(env, asyncTask, napi_eprio_high, "OnEmptyAsyncCall") != napi_status::napi_ok) {
        napiAsyncTask->Reject(env,
            CreateJsError(env, static_cast<int32_t>(WmErrorCode::WM_ERROR_STATE_ABNORMALLY), "failed to send event"));
    }
    return result;
}

napi_value JsExtensionWindow::OnInvalidAsyncCall(napi_env env, napi_callback_info info)
{
    size_t argc = 4;
    napi_value argv[4] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    napi_value lastParam = (argc <= 0) ? nullptr :
        ((argv[argc - 1] != nullptr && GetType(env, argv[argc - 1]) == napi_function) ? argv[argc - 1] : nullptr);
    napi_value result = nullptr;
    std::shared_ptr<NapiAsyncTask> napiAsyncTask = CreateEmptyAsyncTask(env, lastParam, &result);
    auto asyncTask = [env, task = napiAsyncTask] {
        task->Reject(env, CreateJsError(env, static_cast<int32_t>(WmErrorCode::WM_ERROR_INVALID_CALLING)));
    };
    if (napi_send_event(env, asyncTask, napi_eprio_high, "OnInvalidAsyncCall") != napi_status::napi_ok) {
        napiAsyncTask->Reject(env,
            CreateJsError(env, static_cast<int32_t>(WmErrorCode::WM_ERROR_STATE_ABNORMALLY), "failed to send event"));
    }
    return result;
}
}  // namespace Rosen
}  // namespace OHOS