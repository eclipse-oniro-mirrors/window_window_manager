/*
 * Copyright (c) 2021-2023 Huawei Device Co., Ltd.
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

#include "js_window.h"
#include <new>
#include <optional>

#ifndef WINDOW_PREVIEW
#include "js_transition_controller.h"
#else
#include "mock/js_transition_controller.h"
#endif

#include "js_err_utils.h"
#include "js_window_animation_utils.h"
#include "js_window_utils.h"
#include "window.h"
#include "window_helper.h"
#include "window_manager_hilog.h"
#include "window_option.h"
#include "wm_math.h"
#include "pixel_map.h"
#include "pixel_map_napi.h"
#include "napi_remote_object.h"
#include "napi_common_want.h"
#include "permission.h"
#include "request_info.h"
#include "ui_content.h"

namespace OHOS {
namespace Rosen {
using namespace AbilityRuntime;
namespace {
constexpr HiviewDFX::HiLogLabel LABEL = {LOG_CORE, HILOG_DOMAIN_WINDOW, "JsWindow"};
constexpr Rect g_emptyRect = {0, 0, 0, 0};
constexpr int32_t MIN_DECOR_HEIGHT = 37;
constexpr int32_t MAX_DECOR_HEIGHT = 112;
constexpr size_t INDEX_ZERO = 0;
constexpr size_t INDEX_ONE = 1;
constexpr size_t INDEX_TWO = 2;
constexpr size_t INDEX_THREE = 3;
constexpr size_t ONE_PARAMS_SIZE = 1;
constexpr size_t TWO_PARAMS_SIZE = 2;
constexpr size_t THREE_PARAMS_SIZE = 3;
constexpr size_t FOUR_PARAMS_SIZE = 4;
constexpr size_t ARG_COUNT_ZERO = 0;
constexpr size_t ARG_COUNT_ONE = 1;
constexpr size_t ARG_COUNT_TWO = 2;
constexpr double MIN_GRAY_SCALE = 0.0;
constexpr double MAX_GRAY_SCALE = 1.0;
constexpr uint32_t DEFAULT_WINDOW_MAX_WIDTH = 3840;
}

static thread_local std::map<std::string, std::shared_ptr<NativeReference>> g_jsWindowMap;
static std::mutex g_mutex;
static int g_ctorCnt = 0;
static int g_dtorCnt = 0;
static int g_finalizerCnt = 0;
JsWindow::JsWindow(const sptr<Window>& window)
    : windowToken_(window), registerManager_(std::make_unique<JsWindowRegisterManager>())
{
    NotifyNativeWinDestroyFunc func = [this](const std::string& windowName) {
        {
            std::lock_guard<std::mutex> lock(g_mutex);
            if (windowName.empty() || g_jsWindowMap.count(windowName) == 0) {
                TLOGE(WmsLogTag::WMS_LIFE, "Can not find window %{public}s ", windowName.c_str());
                return;
            }
            g_jsWindowMap.erase(windowName);
            TLOGI(WmsLogTag::WMS_LIFE, "Remove window %{public}s", windowName.c_str());
        }
        windowToken_ = nullptr;
        TLOGI(WmsLogTag::WMS_LIFE, "Destroy window %{public}s in js window", windowName.c_str());
    };
    windowToken_->RegisterWindowDestroyedListener(func);
    windowName_ = windowToken_->GetWindowName();
    TLOGI(WmsLogTag::WMS_LIFE, "window: %{public}s, ctorCnt: %{public}d", windowName_.c_str(), ++g_ctorCnt);
}

JsWindow::~JsWindow()
{
    TLOGI(WmsLogTag::WMS_LIFE, "window: %{public}s, dtorCnt: %{public}d", windowName_.c_str(), ++g_dtorCnt);
    if (windowToken_ != nullptr) {
        windowToken_->UnregisterWindowDestroyedListener();
    }
    windowToken_ = nullptr;
}

const std::string& JsWindow::GetWindowName() const
{
    return windowName_;
}

void JsWindow::Finalizer(napi_env env, void* data, void* hint)
{
    WLOGI("g_finalizerCnt:%{public}d", ++g_finalizerCnt);
    auto jsWin = std::unique_ptr<JsWindow>(static_cast<JsWindow*>(data));
    if (jsWin == nullptr) {
        WLOGFE("jsWin is nullptr");
        return;
    }
    std::string windowName = jsWin->GetWindowName();
    std::lock_guard<std::mutex> lock(g_mutex);
    g_jsWindowMap.erase(windowName);
    TLOGI(WmsLogTag::WMS_LIFE, "Remove window %{public}s", windowName.c_str());
}

napi_value JsWindow::Show(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::DEFAULT, "Show");
    JsWindow* me = CheckParamsAndGetThis<JsWindow>(env, info);
    return (me != nullptr) ? me->OnShow(env, info) : nullptr;
}

napi_value JsWindow::ShowWindow(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::DEFAULT, "Show");
    JsWindow* me = CheckParamsAndGetThis<JsWindow>(env, info);
    return (me != nullptr) ? me->OnShowWindow(env, info) : nullptr;
}

napi_value JsWindow::ShowWithAnimation(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::DEFAULT, "ShowWithAnimation");
    JsWindow* me = CheckParamsAndGetThis<JsWindow>(env, info);
    return (me != nullptr) ? me->OnShowWithAnimation(env, info) : nullptr;
}

napi_value JsWindow::Destroy(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::DEFAULT, "Destroy");
    JsWindow* me = CheckParamsAndGetThis<JsWindow>(env, info);
    return (me != nullptr) ? me->OnDestroy(env, info) : nullptr;
}

napi_value JsWindow::DestroyWindow(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::DEFAULT, "Destroy");
    JsWindow* me = CheckParamsAndGetThis<JsWindow>(env, info);
    return (me != nullptr) ? me->OnDestroyWindow(env, info) : nullptr;
}

napi_value JsWindow::Hide(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::DEFAULT, "Hide");
    JsWindow* me = CheckParamsAndGetThis<JsWindow>(env, info);
    return (me != nullptr) ? me->OnHide(env, info) : nullptr;
}

napi_value JsWindow::HideWithAnimation(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::DEFAULT, "HideWithAnimation");
    JsWindow* me = CheckParamsAndGetThis<JsWindow>(env, info);
    return (me != nullptr) ? me->OnHideWithAnimation(env, info) : nullptr;
}

napi_value JsWindow::Recover(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::WMS_LAYOUT_PC, "[NAPI]");
    JsWindow* me = CheckParamsAndGetThis<JsWindow>(env, info);
    return (me != nullptr) ? me->OnRecover(env, info) : nullptr;
}

napi_value JsWindow::Restore(napi_env env, napi_callback_info info)
{
    JsWindow* me = CheckParamsAndGetThis<JsWindow>(env, info);
    return (me != nullptr) ? me->OnRestore(env, info) : nullptr;
}

/** @note @window.layout */
napi_value JsWindow::MoveTo(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::DEFAULT, "MoveTo");
    JsWindow* me = CheckParamsAndGetThis<JsWindow>(env, info);
    return (me != nullptr) ? me->OnMoveTo(env, info) : nullptr;
}

/** @note @window.layout */
napi_value JsWindow::MoveWindowTo(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::DEFAULT, "MoveTo");
    JsWindow* me = CheckParamsAndGetThis<JsWindow>(env, info);
    return (me != nullptr) ? me->OnMoveWindowTo(env, info) : nullptr;
}

/** @note @window.layout */
napi_value JsWindow::MoveWindowToAsync(napi_env env, napi_callback_info info)
{
    TLOGI(WmsLogTag::WMS_LAYOUT, "MoveToAsync");
    JsWindow* me = CheckParamsAndGetThis<JsWindow>(env, info);
    return (me != nullptr) ? me->OnMoveWindowToAsync(env, info) : nullptr;
}

/** @note @window.layout */
napi_value JsWindow::MoveWindowToGlobal(napi_env env, napi_callback_info info)
{
    TLOGI(WmsLogTag::WMS_LAYOUT, "MoveWindowToGlobal");
    JsWindow* me = CheckParamsAndGetThis<JsWindow>(env, info);
    return (me != nullptr) ? me->OnMoveWindowToGlobal(env, info) : nullptr;
}

/** @note @window.layout */
napi_value JsWindow::MoveWindowToGlobalDisplay(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::WMS_LAYOUT, "[NAPI]");
    JsWindow* me = CheckParamsAndGetThis<JsWindow>(env, info);
    return (me != nullptr) ? me->OnMoveWindowToGlobalDisplay(env, info) : nullptr;
}

/** @note @window.layout */
napi_value JsWindow::GetGlobalScaledRect(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::WMS_LAYOUT, "[NAPI]");
    JsWindow* me = CheckParamsAndGetThis<JsWindow>(env, info);
    return (me != nullptr) ? me->OnGetGlobalScaledRect(env, info) : nullptr;
}

/** @note @window.layout */
napi_value JsWindow::Resize(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::DEFAULT, "Resize");
    JsWindow* me = CheckParamsAndGetThis<JsWindow>(env, info);
    return (me != nullptr) ? me->OnResize(env, info) : nullptr;
}

/** @note @window.layout */
napi_value JsWindow::ResizeWindow(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::DEFAULT, "Resize");
    JsWindow* me = CheckParamsAndGetThis<JsWindow>(env, info);
    return (me != nullptr) ? me->OnResizeWindow(env, info) : nullptr;
}

/** @note @window.layout */
napi_value JsWindow::ResizeWindowAsync(napi_env env, napi_callback_info info)
{
    TLOGI(WmsLogTag::WMS_LAYOUT, "ResizeAsync");
    JsWindow* me = CheckParamsAndGetThis<JsWindow>(env, info);
    return (me != nullptr) ? me->OnResizeWindowAsync(env, info) : nullptr;
}

napi_value JsWindow::ResizeWindowWithAnimation(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::WMS_LAYOUT, "in");
    JsWindow* me = CheckParamsAndGetThis<JsWindow>(env, info);
    return (me != nullptr) ? me->OnResizeWindowWithAnimation(env, info) : nullptr;
}

/** @note @window.layout */
napi_value JsWindow::ClientToGlobalDisplay(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::WMS_LAYOUT, "[NAPI]");
    JsWindow* me = CheckParamsAndGetThis<JsWindow>(env, info);
    return (me != nullptr) ? me->OnClientToGlobalDisplay(env, info) : nullptr;
}

/** @note @window.layout */
napi_value JsWindow::GlobalDisplayToClient(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::WMS_LAYOUT, "[NAPI]");
    JsWindow* me = CheckParamsAndGetThis<JsWindow>(env, info);
    return (me != nullptr) ? me->OnGlobalDisplayToClient(env, info) : nullptr;
}

napi_value JsWindow::SetWindowType(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::DEFAULT, "SetWindowType");
    JsWindow* me = CheckParamsAndGetThis<JsWindow>(env, info);
    return (me != nullptr) ? me->OnSetWindowType(env, info) : nullptr;
}

/** @note @window.layout */
napi_value JsWindow::SetWindowMode(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::DEFAULT, "SetWindowMode");
    JsWindow* me = CheckParamsAndGetThis<JsWindow>(env, info);
    return (me != nullptr) ? me->OnSetWindowMode(env, info) : nullptr;
}

napi_value JsWindow::GetProperties(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::DEFAULT, "GetProperties");
    JsWindow* me = CheckParamsAndGetThis<JsWindow>(env, info);
    return (me != nullptr) ? me->OnGetProperties(env, info) : nullptr;
}

napi_value JsWindow::GetWindowPropertiesSync(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::DEFAULT, "GetProperties");
    JsWindow* me = CheckParamsAndGetThis<JsWindow>(env, info);
    return (me != nullptr) ? me->OnGetWindowPropertiesSync(env, info) : nullptr;
}

napi_value JsWindow::RegisterWindowCallback(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::WMS_ATTRIBUTE, "Enter");
    JsWindow* me = CheckParamsAndGetThis<JsWindow>(env, info);
    return (me != nullptr) ? me->OnRegisterWindowCallback(env, info) : nullptr;
}

napi_value JsWindow::UnregisterWindowCallback(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::WMS_ATTRIBUTE, "Enter");
    JsWindow* me = CheckParamsAndGetThis<JsWindow>(env, info);
    return (me != nullptr) ? me->OnUnregisterWindowCallback(env, info) : nullptr;
}

napi_value JsWindow::BindDialogTarget(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::DEFAULT, "BindDialogTarget");
    JsWindow* me = CheckParamsAndGetThis<JsWindow>(env, info);
    return (me != nullptr) ? me->OnBindDialogTarget(env, info) : nullptr;
}

napi_value JsWindow::SetDialogBackGestureEnabled(napi_env env, napi_callback_info info)
{
    JsWindow* me = CheckParamsAndGetThis<JsWindow>(env, info);
    return (me != nullptr) ? me->OnSetDialogBackGestureEnabled(env, info) : nullptr;
}

napi_value JsWindow::LoadContent(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::DEFAULT, "[NAPI]");
    JsWindow* me = CheckParamsAndGetThis<JsWindow>(env, info);
    return (me != nullptr) ? me->OnLoadContent(env, info, false) : nullptr;
}

napi_value JsWindow::LoadContentByName(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::DEFAULT, "[NAPI]");
    JsWindow* me = CheckParamsAndGetThis<JsWindow>(env, info);
    return (me != nullptr) ? me->OnLoadContent(env, info, true) : nullptr;
}

napi_value JsWindow::GetUIContext(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::DEFAULT, "GetUIContext");
    JsWindow* me = CheckParamsAndGetThis<JsWindow>(env, info);
    return (me != nullptr) ? me->OnGetUIContext(env, info) : nullptr;
}

napi_value JsWindow::SetUIContent(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::DEFAULT, "[NAPI]");
    JsWindow* me = CheckParamsAndGetThis<JsWindow>(env, info);
    return (me != nullptr) ? me->OnSetUIContent(env, info) : nullptr;
}

napi_value JsWindow::SetFullScreen(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::WMS_IMMS, "[NAPI]");
    JsWindow* me = CheckParamsAndGetThis<JsWindow>(env, info);
    return (me != nullptr) ? me->OnSetFullScreen(env, info) : nullptr;
}

napi_value JsWindow::SetLayoutFullScreen(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::WMS_IMMS, "[NAPI]");
    JsWindow* me = CheckParamsAndGetThis<JsWindow>(env, info);
    return (me != nullptr) ? me->OnSetLayoutFullScreen(env, info) : nullptr;
}

napi_value JsWindow::SetTitleAndDockHoverShown(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::WMS_LAYOUT_PC, "[NAPI]");
    JsWindow* me = CheckParamsAndGetThis<JsWindow>(env, info);
    return (me != nullptr) ? me->OnSetTitleAndDockHoverShown(env, info) : nullptr;
}

napi_value JsWindow::SetWindowLayoutFullScreen(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::WMS_IMMS, "[NAPI]");
    JsWindow* me = CheckParamsAndGetThis<JsWindow>(env, info);
    return (me != nullptr) ? me->OnSetWindowLayoutFullScreen(env, info) : nullptr;
}

napi_value JsWindow::SetSystemBarEnable(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::WMS_IMMS, "[NAPI]");
    JsWindow* me = CheckParamsAndGetThis<JsWindow>(env, info);
    return (me != nullptr) ? me->OnSetSystemBarEnable(env, info) : nullptr;
}

napi_value JsWindow::SetWindowSystemBarEnable(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::WMS_IMMS, "[NAPI]");
    JsWindow* me = CheckParamsAndGetThis<JsWindow>(env, info);
    return (me != nullptr) ? me->OnSetWindowSystemBarEnable(env, info) : nullptr;
}

napi_value JsWindow::SetSpecificSystemBarEnabled(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::WMS_IMMS, "[NAPI]");
    JsWindow* me = CheckParamsAndGetThis<JsWindow>(env, info);
    return (me != nullptr) ? me->OnSetSpecificSystemBarEnabled(env, info) : nullptr;
}

napi_value JsWindow::EnableLandscapeMultiWindow(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::WMS_MULTI_WINDOW, "EnableLandscapeMultiWindow");
    JsWindow* me = CheckParamsAndGetThis<JsWindow>(env, info);
    return (me != nullptr) ? me->OnEnableLandscapeMultiWindow(env, info) : nullptr;
}

napi_value JsWindow::DisableLandscapeMultiWindow(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::WMS_MULTI_WINDOW, "DisableLandscapeMultiWindow");
    JsWindow* me = CheckParamsAndGetThis<JsWindow>(env, info);
    return (me != nullptr) ? me->OnDisableLandscapeMultiWindow(env, info) : nullptr;
}

napi_value JsWindow::SetSystemBarProperties(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::WMS_IMMS, "[NAPI]");
    JsWindow* me = CheckParamsAndGetThis<JsWindow>(env, info);
    return (me != nullptr) ? me->OnSetSystemBarProperties(env, info) : nullptr;
}

napi_value JsWindow::GetWindowSystemBarPropertiesSync(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::WMS_IMMS, "[NAPI]");
    JsWindow* me = CheckParamsAndGetThis<JsWindow>(env, info);
    return (me != nullptr) ? me->OnGetWindowSystemBarPropertiesSync(env, info) : nullptr;
}

napi_value JsWindow::SetWindowSystemBarProperties(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::WMS_IMMS, "[NAPI]");
    JsWindow* me = CheckParamsAndGetThis<JsWindow>(env, info);
    return (me != nullptr) ? me->OnSetWindowSystemBarProperties(env, info) : nullptr;
}

napi_value JsWindow::SetStatusBarColor(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::WMS_IMMS, "[NAPI]");
    JsWindow* me = CheckParamsAndGetThis<JsWindow>(env, info);
    return (me != nullptr) ? me->OnSetStatusBarColor(env, info) : nullptr;
}

napi_value JsWindow::GetStatusBarProperty(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::WMS_IMMS, "[NAPI]");
    JsWindow* me = CheckParamsAndGetThis<JsWindow>(env, info);
    return (me != nullptr) ? me->OnGetStatusBarPropertySync(env, info) : nullptr;
}

napi_value JsWindow::GetWindowStateSnapshot(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::WMS_IMMS, "[NAPI]");
    JsWindow* me = CheckParamsAndGetThis<JsWindow>(env, info);
    return (me != nullptr) ? me->OnGetWindowStateSnapshot(env, info) : nullptr;
}

napi_value JsWindow::GetAvoidArea(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::WMS_IMMS, "[NAPI]");
    JsWindow* me = CheckParamsAndGetThis<JsWindow>(env, info);
    return (me != nullptr) ? me->OnGetAvoidArea(env, info) : nullptr;
}

napi_value JsWindow::GetWindowAvoidAreaSync(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::WMS_IMMS, "[NAPI]");
    JsWindow* me = CheckParamsAndGetThis<JsWindow>(env, info);
    return (me != nullptr) ? me->OnGetWindowAvoidAreaSync(env, info) : nullptr;
}

napi_value JsWindow::GetWindowAvoidAreaIgnoringVisibilitySync(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::WMS_IMMS, "[NAPI]");
    JsWindow* me = CheckParamsAndGetThis<JsWindow>(env, info);
    return (me != nullptr) ? me->OnGetWindowAvoidAreaIgnoringVisibilitySync(env, info) : nullptr;
}

napi_value JsWindow::IsShowing(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::DEFAULT, "IsShowing");
    JsWindow* me = CheckParamsAndGetThis<JsWindow>(env, info);
    return (me != nullptr) ? me->OnIsShowing(env, info) : nullptr;
}

napi_value JsWindow::IsWindowShowingSync(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::WMS_LIFE, "IsShowing");
    JsWindow* me = CheckParamsAndGetThis<JsWindow>(env, info);
    return (me != nullptr) ? me->OnIsWindowShowingSync(env, info) : nullptr;
}

napi_value JsWindow::IsSupportWideGamut(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::DEFAULT, "IsSupportWideGamut");
    JsWindow* me = CheckParamsAndGetThis<JsWindow>(env, info);
    return (me != nullptr) ? me->OnIsSupportWideGamut(env, info) : nullptr;
}

napi_value JsWindow::IsWindowSupportWideGamut(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::DEFAULT, "IsSupportWideGamut");
    JsWindow* me = CheckParamsAndGetThis<JsWindow>(env, info);
    return (me != nullptr) ? me->OnIsWindowSupportWideGamut(env, info) : nullptr;
}

napi_value JsWindow::SetBackgroundColor(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::DEFAULT, "SetBackgroundColor");
    JsWindow* me = CheckParamsAndGetThis<JsWindow>(env, info);
    return (me != nullptr) ? me->OnSetBackgroundColor(env, info) : nullptr;
}

napi_value JsWindow::SetWindowBackgroundColorSync(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::DEFAULT, "SetBackgroundColor");
    JsWindow* me = CheckParamsAndGetThis<JsWindow>(env, info);
    return (me != nullptr) ? me->OnSetWindowBackgroundColorSync(env, info) : nullptr;
}

napi_value JsWindow::SetBrightness(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::DEFAULT, "SetBrightness");
    JsWindow* me = CheckParamsAndGetThis<JsWindow>(env, info);
    return (me != nullptr) ? me->OnSetBrightness(env, info) : nullptr;
}

napi_value JsWindow::SetWindowBrightness(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::DEFAULT, "SetBrightness");
    JsWindow* me = CheckParamsAndGetThis<JsWindow>(env, info);
    return (me != nullptr) ? me->OnSetWindowBrightness(env, info) : nullptr;
}

napi_value JsWindow::SetDimBehind(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::DEFAULT, "SetDimBehind");
    JsWindow* me = CheckParamsAndGetThis<JsWindow>(env, info);
    return (me != nullptr) ? me->OnSetDimBehind(env, info) : nullptr;
}

napi_value JsWindow::SetFocusable(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::WMS_FOCUS, "SetFocusable");
    JsWindow* me = CheckParamsAndGetThis<JsWindow>(env, info);
    return (me != nullptr) ? me->OnSetFocusable(env, info) : nullptr;
}

napi_value JsWindow::SetWindowFocusable(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::DEFAULT, "SetFocusable");
    JsWindow* me = CheckParamsAndGetThis<JsWindow>(env, info);
    return (me != nullptr) ? me->OnSetWindowFocusable(env, info) : nullptr;
}

/** @note @window.hierarchy */
napi_value JsWindow::SetTopmost(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::WMS_HIERARCHY, "SetTopmost");
    JsWindow* me = CheckParamsAndGetThis<JsWindow>(env, info);
    return (me != nullptr) ? me->OnSetTopmost(env, info) : nullptr;
}

/** @note @window.hierarchy */
napi_value JsWindow::SetWindowTopmost(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::WMS_HIERARCHY, "[NAPI]");
    JsWindow* me = CheckParamsAndGetThis<JsWindow>(env, info);
    return (me != nullptr) ? me->OnSetWindowTopmost(env, info) : nullptr;
}

/** @note @window.hierarchy */
napi_value JsWindow::SetSubWindowZLevel(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::WMS_HIERARCHY, "[NAPI]");
    JsWindow* me = CheckParamsAndGetThis<JsWindow>(env, info);
    return (me != nullptr) ? me->OnSetSubWindowZLevel(env, info) : nullptr;
}

/** @note @window.hierarchy */
napi_value JsWindow::GetSubWindowZLevel(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::WMS_HIERARCHY, "[NAPI]");
    JsWindow* me = CheckParamsAndGetThis<JsWindow>(env, info);
    return (me != nullptr) ? me->OnGetSubWindowZLevel(env, info) : nullptr;
}

napi_value JsWindow::SetWindowDelayRaiseOnDrag(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::WMS_FOCUS, "[NAPI]");
    JsWindow* me = CheckParamsAndGetThis<JsWindow>(env, info);
    return (me != nullptr) ? me->OnSetWindowDelayRaiseOnDrag(env, info) : nullptr;
}

napi_value JsWindow::SetKeepScreenOn(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::DEFAULT, "SetKeepScreenOn");
    JsWindow* me = CheckParamsAndGetThis<JsWindow>(env, info);
    return (me != nullptr) ? me->OnSetKeepScreenOn(env, info) : nullptr;
}

napi_value JsWindow::SetWindowKeepScreenOn(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::DEFAULT, "SetKeepScreenOn");
    JsWindow* me = CheckParamsAndGetThis<JsWindow>(env, info);
    return (me != nullptr) ? me->OnSetWindowKeepScreenOn(env, info) : nullptr;
}

napi_value JsWindow::SetWakeUpScreen(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::DEFAULT, "SetWakeUpScreen");
    JsWindow* me = CheckParamsAndGetThis<JsWindow>(env, info);
    return (me != nullptr) ? me->OnSetWakeUpScreen(env, info) : nullptr;
}

napi_value JsWindow::SetOutsideTouchable(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::DEFAULT, "SetOutsideTouchable");
    JsWindow* me = CheckParamsAndGetThis<JsWindow>(env, info);
    return (me != nullptr) ? me->OnSetOutsideTouchable(env, info) : nullptr;
}

napi_value JsWindow::SetPrivacyMode(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::DEFAULT, "SetPrivacyMode");
    JsWindow* me = CheckParamsAndGetThis<JsWindow>(env, info);
    return (me != nullptr) ? me->OnSetPrivacyMode(env, info) : nullptr;
}

napi_value JsWindow::SetWindowPrivacyMode(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::DEFAULT, "SetPrivacyMode");
    JsWindow* me = CheckParamsAndGetThis<JsWindow>(env, info);
    return (me != nullptr) ? me->OnSetWindowPrivacyMode(env, info) : nullptr;
}

napi_value JsWindow::SetTouchable(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::DEFAULT, "Enter");
    JsWindow* me = CheckParamsAndGetThis<JsWindow>(env, info);
    return (me != nullptr) ? me->OnSetTouchable(env, info) : nullptr;
}

napi_value JsWindow::SetTouchableAreas(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::WMS_EVENT, "[NAPI]");
    JsWindow* me = CheckParamsAndGetThis<JsWindow>(env, info);
    return (me != nullptr) ? me->OnSetTouchableAreas(env, info) : nullptr;
}

napi_value JsWindow::SetResizeByDragEnabled(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::DEFAULT, "[NAPI]");
    JsWindow* me = CheckParamsAndGetThis<JsWindow>(env, info);
    return (me != nullptr) ? me->OnSetResizeByDragEnabled(env, info) : nullptr;
}

/** @note @window.hierarchy */
napi_value JsWindow::SetRaiseByClickEnabled(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::DEFAULT, "SetRaiseByClickEnabled");
    JsWindow* me = CheckParamsAndGetThis<JsWindow>(env, info);
    return (me != nullptr) ? me->OnSetRaiseByClickEnabled(env, info) : nullptr;
}

napi_value JsWindow::SetMainWindowRaiseByClickEnabled(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::DEFAULT, "SetMainWindowRaiseByClickEnabled");
    JsWindow* me = CheckParamsAndGetThis<JsWindow>(env, info);
    return (me != nullptr) ? me->OnSetMainWindowRaiseByClickEnabled(env, info) : nullptr;
}

napi_value JsWindow::HideNonSystemFloatingWindows(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::WMS_ATTRIBUTE, "in");
    JsWindow* me = CheckParamsAndGetThis<JsWindow>(env, info);
    return (me != nullptr) ? me->OnHideNonSystemFloatingWindows(env, info) : nullptr;
}

napi_value JsWindow::SetWindowTouchable(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::DEFAULT, "Enter");
    JsWindow* me = CheckParamsAndGetThis<JsWindow>(env, info);
    return (me != nullptr) ? me->OnSetWindowTouchable(env, info) : nullptr;
}

napi_value JsWindow::SetTransparent(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::DEFAULT, "SetTransparent");
    JsWindow* me = CheckParamsAndGetThis<JsWindow>(env, info);
    return (me != nullptr) ? me->OnSetTransparent(env, info) : nullptr;
}

napi_value JsWindow::ChangeCallingWindowId(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::DEFAULT, "ChangeCallingWindowId");
    JsWindow* me = CheckParamsAndGetThis<JsWindow>(env, info);
    return (me != nullptr) ? me->OnChangeCallingWindowId(env, info) : nullptr;
}

napi_value JsWindow::SetPreferredOrientation(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::DEFAULT, "SetPreferredOrientation");
    JsWindow* me = CheckParamsAndGetThis<JsWindow>(env, info);
    if (me != nullptr) {
        return me->OnSetPreferredOrientation(env, info);
    }
    return NapiThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
        "[window][setPreferredOrientation]msg: Window is nullptr.");
}

napi_value JsWindow::GetPreferredOrientation(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::WMS_ROTATION, "GetPreferredOrientation");
    JsWindow* me = CheckParamsAndGetThis<JsWindow>(env, info);
    if (me != nullptr) {
        return me->OnGetPreferredOrientation(env, info);
    }
    return NapiThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
        "[window][getPreferredOrientation]msg: Window is nullptr.");
}

napi_value JsWindow::ConvertOrientationAndRotation(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::WMS_ROTATION, "ConvertOrientationAndRotation");
    JsWindow* me = CheckParamsAndGetThis<JsWindow>(env, info);
    return (me != nullptr) ? me->OnConvertOrientationAndRotation(env, info) : nullptr;
}

napi_value JsWindow::SetSnapshotSkip(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::DEFAULT, "SetSnapshotSkip");
    JsWindow* me = CheckParamsAndGetThis<JsWindow>(env, info);
    return (me != nullptr) ? me->OnSetSnapshotSkip(env, info) : nullptr;
}

napi_value JsWindow::SetSingleFrameComposerEnabled(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::DEFAULT, "SetSingleFrameComposerEnabled");
    JsWindow* me = CheckParamsAndGetThis<JsWindow>(env, info);
    return (me != nullptr) ? me->OnSetSingleFrameComposerEnabled(env, info) : nullptr;
}

/** @note @window.hierarchy */
napi_value JsWindow::RaiseToAppTop(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::DEFAULT, "RaiseToAppTop");
    JsWindow* me = CheckParamsAndGetThis<JsWindow>(env, info);
    return (me != nullptr) ? me->OnRaiseToAppTop(env, info) : nullptr;
}

napi_value JsWindow::DisableWindowDecor(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::WMS_DECOR, "[NAPI]");
    JsWindow* me = CheckParamsAndGetThis<JsWindow>(env, info);
    return (me != nullptr) ? me->OnDisableWindowDecor(env, info) : nullptr;
}

napi_value JsWindow::SetColorSpace(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::DEFAULT, "SetColorSpace");
    JsWindow* me = CheckParamsAndGetThis<JsWindow>(env, info);
    return (me != nullptr) ? me->OnSetColorSpace(env, info) : nullptr;
}

napi_value JsWindow::SetWindowColorSpace(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::DEFAULT, "SetColorSpace");
    JsWindow* me = CheckParamsAndGetThis<JsWindow>(env, info);
    return (me != nullptr) ? me->OnSetWindowColorSpace(env, info) : nullptr;
}

napi_value JsWindow::GetColorSpace(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::DEFAULT, "GetColorSpace");
    JsWindow* me = CheckParamsAndGetThis<JsWindow>(env, info);
    return (me != nullptr) ? me->OnGetColorSpace(env, info) : nullptr;
}

napi_value JsWindow::GetWindowColorSpaceSync(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::DEFAULT, "GetColorSpace");
    JsWindow* me = CheckParamsAndGetThis<JsWindow>(env, info);
    return (me != nullptr) ? me->OnGetWindowColorSpaceSync(env, info) : nullptr;
}

napi_value JsWindow::Dump(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::DEFAULT, "Dump");
    JsWindow* me = CheckParamsAndGetThis<JsWindow>(env, info);
    return (me != nullptr) ? me->OnDump(env, info) : nullptr;
}

napi_value JsWindow::SetForbidSplitMove(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::DEFAULT, "SetForbidSplitMove");
    JsWindow* me = CheckParamsAndGetThis<JsWindow>(env, info);
    return (me != nullptr) ? me->OnSetForbidSplitMove(env, info) : nullptr;
}

napi_value JsWindow::Opacity(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::DEFAULT, "Opacity");
    JsWindow* me = CheckParamsAndGetThis<JsWindow>(env, info);
    return (me != nullptr) ? me->OnOpacity(env, info) : nullptr;
}

napi_value JsWindow::Scale(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::DEFAULT, "Scale");
    JsWindow* me = CheckParamsAndGetThis<JsWindow>(env, info);
    return (me != nullptr) ? me->OnScale(env, info) : nullptr;
}

napi_value JsWindow::Rotate(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::DEFAULT, "Rotate");
    JsWindow* me = CheckParamsAndGetThis<JsWindow>(env, info);
    return (me != nullptr) ? me->OnRotate(env, info) : nullptr;
}

napi_value JsWindow::Translate(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::DEFAULT, "Translate");
    JsWindow* me = CheckParamsAndGetThis<JsWindow>(env, info);
    return (me != nullptr) ? me->OnTranslate(env, info) : nullptr;
}

napi_value JsWindow::GetTransitionController(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::DEFAULT, "GetTransitionController");
    JsWindow* me = CheckParamsAndGetThis<JsWindow>(env, info);
    return (me != nullptr) ? me->OnGetTransitionController(env, info) : nullptr;
}

napi_value JsWindow::SetCornerRadius(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::DEFAULT, "SetCornerRadius");
    JsWindow* me = CheckParamsAndGetThis<JsWindow>(env, info);
    return (me != nullptr) ? me->OnSetCornerRadius(env, info) : nullptr;
}

napi_value JsWindow::SetWindowCornerRadius(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::WMS_ATTRIBUTE, "[NAPI]");
    JsWindow* me = CheckParamsAndGetThis<JsWindow>(env, info);
    if (me != nullptr) {
        return me->OnSetWindowCornerRadius(env, info);
    }
    return NapiThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
        "[window][setWindowCornerRadius]msg: Window is nullptr.");
}

napi_value JsWindow::GetWindowCornerRadius(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::WMS_ATTRIBUTE, "[NAPI]");
    JsWindow* me = CheckParamsAndGetThis<JsWindow>(env, info);
    if (me != nullptr) {
        return me->OnGetWindowCornerRadius(env, info);
    }
    return NapiThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
        "[window][getWindowCornerRadius]msg: Window is nullptr.");
}

napi_value JsWindow::SetShadow(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::DEFAULT, "SetShadow");
    JsWindow* me = CheckParamsAndGetThis<JsWindow>(env, info);
    return (me != nullptr) ? me->OnSetShadow(env, info) : nullptr;
}

napi_value JsWindow::SetWindowShadowRadius(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::WMS_ATTRIBUTE, "[NAPI]");
    JsWindow* me = CheckParamsAndGetThis<JsWindow>(env, info);
    if (me != nullptr) {
        return me->OnSetWindowShadowRadius(env, info);
    }
    return NapiThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
        "[window][setWindowShadowRadius]msg: Window is nullptr.");
}

napi_value JsWindow::SetBlur(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::DEFAULT, "SetBlur");
    JsWindow* me = CheckParamsAndGetThis<JsWindow>(env, info);
    return (me != nullptr) ? me->OnSetBlur(env, info) : nullptr;
}

napi_value JsWindow::SetBackdropBlur(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::DEFAULT, "SetBackdropBlur");
    JsWindow* me = CheckParamsAndGetThis<JsWindow>(env, info);
    return (me != nullptr) ? me->OnSetBackdropBlur(env, info) : nullptr;
}

napi_value JsWindow::SetBackdropBlurStyle(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::DEFAULT, "SetBackdropBlurStyle");
    JsWindow* me = CheckParamsAndGetThis<JsWindow>(env, info);
    return (me != nullptr) ? me->OnSetBackdropBlurStyle(env, info) : nullptr;
}

napi_value JsWindow::SetWaterMarkFlag(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::DEFAULT, "SetWaterMarkFlag");
    JsWindow* me = CheckParamsAndGetThis<JsWindow>(env, info);
    return (me != nullptr) ? me->OnSetWaterMarkFlag(env, info) : nullptr;
}

napi_value JsWindow::SetHandwritingFlag(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::DEFAULT, "[NAPI]");
    JsWindow* me = CheckParamsAndGetThis<JsWindow>(env, info);
    return (me != nullptr) ? me->OnSetHandwritingFlag(env, info) : nullptr;
}

napi_value JsWindow::SetAspectRatio(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::WMS_LAYOUT, "[NAPI]");
    JsWindow* me = CheckParamsAndGetThis<JsWindow>(env, info);
    return (me != nullptr) ? me->OnSetAspectRatio(env, info) : nullptr;
}

napi_value JsWindow::SetContentAspectRatio(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::WMS_LAYOUT, "[NAPI]");
    JsWindow* me = CheckParamsAndGetThis<JsWindow>(env, info);
    return (me != nullptr) ? me->OnSetContentAspectRatio(env, info) : nullptr;
}

napi_value JsWindow::ResetAspectRatio(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::WMS_LAYOUT, "[NAPI]");
    JsWindow* me = CheckParamsAndGetThis<JsWindow>(env, info);
    return (me != nullptr) ? me->OnResetAspectRatio(env, info) : nullptr;
}

napi_value JsWindow::Minimize(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::WMS_LAYOUT, "[NAPI]");
    JsWindow* me = CheckParamsAndGetThis<JsWindow>(env, info);
    return (me != nullptr) ? me->OnMinimize(env, info) : nullptr;
}

napi_value JsWindow::Maximize(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::WMS_LAYOUT_PC, "[NAPI]");
    JsWindow* me = CheckParamsAndGetThis<JsWindow>(env, info);
    return (me != nullptr) ? me->OnMaximize(env, info) : nullptr;
}

/** @note @window.hierarchy */
napi_value JsWindow::RaiseAboveTarget(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::DEFAULT, "[NAPI]");
    JsWindow* me = CheckParamsAndGetThis<JsWindow>(env, info);
    return (me != nullptr) ? me->OnRaiseAboveTarget(env, info) : nullptr;
}

/** @note @window.hierarchy */
napi_value JsWindow::RaiseMainWindowAboveTarget(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::WMS_HIERARCHY, "[NAPI]");
    JsWindow* me = CheckParamsAndGetThis<JsWindow>(env, info);
    return (me != nullptr) ? me->OnRaiseMainWindowAboveTarget(env, info) : nullptr;
}

napi_value JsWindow::KeepKeyboardOnFocus(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::DEFAULT, "[NAPI]");
    JsWindow* me = CheckParamsAndGetThis<JsWindow>(env, info);
    return (me != nullptr) ? me->OnKeepKeyboardOnFocus(env, info) : nullptr;
}

napi_value JsWindow::EnableDrag(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::WMS_LAYOUT, "[NAPI]");
    JsWindow* me = CheckParamsAndGetThis<JsWindow>(env, info);
    return (me != nullptr) ? me->OnEnableDrag(env, info) : nullptr;
}

/** @note @window.layout */
napi_value JsWindow::GetWindowLimits(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::DEFAULT, "[NAPI]");
    JsWindow* me = CheckParamsAndGetThis<JsWindow>(env, info);
    return (me != nullptr) ? me->OnGetWindowLimits(env, info) : nullptr;
}

/** @note @window.layout */
napi_value JsWindow::GetWindowLimitsVP(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::WMS_LAYOUT, "[NAPI]");
    JsWindow* me = CheckParamsAndGetThis<JsWindow>(env, info);
    return (me != nullptr) ? me->OnGetWindowLimitsVP(env, info) : nullptr;
}

/** @note @window.layout */
napi_value JsWindow::SetWindowLimits(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::DEFAULT, "[NAPI]");
    JsWindow* me = CheckParamsAndGetThis<JsWindow>(env, info);
    return (me != nullptr) ? me->OnSetWindowLimits(env, info) : nullptr;
}

napi_value JsWindow::SetWindowDecorVisible(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::WMS_DECOR, "[NAPI]");
    JsWindow* me = CheckParamsAndGetThis<JsWindow>(env, info);
    if (me != nullptr) {
        return me->OnSetWindowDecorVisible(env, info);
    }
    return NapiThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
        "[window][setWindowDecorVisible]msg: Window is nullptr.");
}

napi_value JsWindow::GetWindowDecorVisible(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::WMS_DECOR, "[NAPI]");
    JsWindow* me = CheckParamsAndGetThis<JsWindow>(env, info);
    if (me != nullptr) {
        return me->OnGetWindowDecorVisible(env, info);
    }
    return NapiThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
        "[window][getWindowDecorVisible]msg: Window is nullptr.");
}

napi_value JsWindow::SetWindowTitleMoveEnabled(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::WMS_DECOR, "[NAPI]");
    JsWindow* me = CheckParamsAndGetThis<JsWindow>(env, info);
    if (me != nullptr) {
        return me->OnSetWindowTitleMoveEnabled(env, info);
    }
    return NapiThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
        "[window][setWindowTitleMoveEnabled]msg: Window is nullptr.");
}

napi_value JsWindow::SetSubWindowModal(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::WMS_SUB, "[NAPI]");
    JsWindow* me = CheckParamsAndGetThis<JsWindow>(env, info);
    return (me != nullptr) ? me->OnSetSubWindowModal(env, info) : nullptr;
}

napi_value JsWindow::SetFollowParentMultiScreenPolicy(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::WMS_MAIN, "[NAPI]");
    JsWindow* me = CheckParamsAndGetThis<JsWindow>(env, info);
    return (me != nullptr) ? me->OnSetFollowParentMultiScreenPolicy(env, info) : nullptr;
}

napi_value JsWindow::SetWindowTransitionAnimation(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::WMS_ANIMATION, "[NAPI]");
    JsWindow* me = CheckParamsAndGetThis<JsWindow>(env, info);
    if (me != nullptr) {
        return me->OnSetWindowTransitionAnimation(env, info);
    }
    return NapiThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
        "[window][setWindowTransitionAnimation]msg: Window is nullptr.");
}

napi_value JsWindow::GetWindowTransitionAnimation(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::WMS_ANIMATION, "[NAPI]");
    JsWindow* me = CheckParamsAndGetThis<JsWindow>(env, info);
    if (me != nullptr) {
        return me->OnGetWindowTransitionAnimation(env, info);
    }
    return NapiThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
        "[window][getWindowTransitionAnimation]msg: Window is nullptr.");
}

napi_value JsWindow::SetWindowDecorHeight(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::WMS_DECOR, "[NAPI]");
    JsWindow* me = CheckParamsAndGetThis<JsWindow>(env, info);
    if (me != nullptr) {
        return me->OnSetWindowDecorHeight(env, info);
    }
    return NapiThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
        "[window][setWindowDecorHeight]msg: Window is nullptr.");
}

napi_value JsWindow::GetWindowDecorHeight(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::WMS_DECOR, "[NAPI]");
    JsWindow* me = CheckParamsAndGetThis<JsWindow>(env, info);
    if (me != nullptr) {
        return me->OnGetWindowDecorHeight(env, info);
    }
    return NapiThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
        "[window][getWindowDecorHeight]msg: Window is nullptr.");
}

napi_value JsWindow::SetDecorButtonStyle(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::WMS_DECOR, "[NAPI]");
    JsWindow* me = CheckParamsAndGetThis<JsWindow>(env, info);
    if (me != nullptr) {
        return me->OnSetDecorButtonStyle(env, info);
    }
    return NapiThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
        "[window][setDecorButtonStyle]msg: Window is nullptr.");
}

napi_value JsWindow::GetDecorButtonStyle(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::WMS_DECOR, "[NAPI]");
    JsWindow* me = CheckParamsAndGetThis<JsWindow>(env, info);
    if (me != nullptr) {
        return me->OnGetDecorButtonStyle(env, info);
    }
    return NapiThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
        "[window][getDecorButtonStyle]msg: Window is nullptr.");
}

napi_value JsWindow::GetTitleButtonRect(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::WMS_DECOR, "[NAPI]");
    JsWindow* me = CheckParamsAndGetThis<JsWindow>(env, info);
    if (me != nullptr) {
        return me->OnGetTitleButtonRect(env, info);
    }
    return NapiThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
        "[window][getTitleButtonRect]msg: Window is nullptr.");
}

napi_value JsWindow::SetWindowContainerColor(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::WMS_DECOR, "[NAPI]");
    JsWindow* me = CheckParamsAndGetThis<JsWindow>(env, info);
    return (me != nullptr) ? me->OnSetWindowContainerColor(env, info) : nullptr;
}

napi_value JsWindow::SetWindowContainerModalColor(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::WMS_DECOR, "[NAPI]");
    JsWindow* me = CheckParamsAndGetThis<JsWindow>(env, info);
    return (me != nullptr) ? me->OnSetWindowContainerModalColor(env, info) : nullptr;
}

napi_value JsWindow::SetWindowMask(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::WMS_PC, "[NAPI]");
    JsWindow* me = CheckParamsAndGetThis<JsWindow>(env, info);
    return (me != nullptr) ? me->OnSetWindowMask(env, info) : nullptr;
}

napi_value JsWindow::SetTitleButtonVisible(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::WMS_DECOR, "[NAPI]");
    JsWindow* me = CheckParamsAndGetThis<JsWindow>(env, info);
    return (me != nullptr) ? me->OnSetTitleButtonVisible(env, info) : nullptr;
}

napi_value JsWindow::SetWindowTitleButtonVisible(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::WMS_DECOR, "[NAPI]");
    JsWindow* me = CheckParamsAndGetThis<JsWindow>(env, info);
    if (me != nullptr) {
        return me->OnSetWindowTitleButtonVisible(env, info);
    }
    return NapiThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
        "[window][setWindowTitleButtonVisible]msg: Window is nullptr.");
}

napi_value JsWindow::SetWindowTitle(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::WMS_DECOR, "[NAPI]");
    JsWindow* me = CheckParamsAndGetThis<JsWindow>(env, info);
    if (me != nullptr) {
        return me->OnSetWindowTitle(env, info);
    }
    return NapiThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
        "[window][setWindowTitle]msg: Window is nullptr.");
}

napi_value JsWindow::SetWindowGrayScale(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::DEFAULT, "[NAPI]");
    JsWindow* me = CheckParamsAndGetThis<JsWindow>(env, info);
    return (me != nullptr) ? me->OnSetWindowGrayScale(env, info) : nullptr;
}

napi_value JsWindow::SetImmersiveModeEnabledState(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::WMS_IMMS, "[NAPI]");
    JsWindow* me = CheckParamsAndGetThis<JsWindow>(env, info);
    return (me != nullptr) ? me->OnSetImmersiveModeEnabledState(env, info) : nullptr;
}

napi_value JsWindow::GetImmersiveModeEnabledState(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::WMS_IMMS, "[NAPI]");
    JsWindow* me = CheckParamsAndGetThis<JsWindow>(env, info);
    return (me != nullptr) ? me->OnGetImmersiveModeEnabledState(env, info) : nullptr;
}

napi_value JsWindow::IsImmersiveLayout(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::WMS_IMMS, "[NAPI]");
    JsWindow* me = CheckParamsAndGetThis<JsWindow>(env, info);
    return (me != nullptr) ? me->OnIsImmersiveLayout(env, info) : nullptr;
}

napi_value JsWindow::IsInFreeWindowMode(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::WMS_IMMS, "[NAPI]");
    JsWindow* me = CheckParamsAndGetThis<JsWindow>(env, info);
    return (me != nullptr) ? me->OnIsInFreeWindowMode(env, info) : nullptr;
}

napi_value JsWindow::GetWindowStatus(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::WMS_PC, "[NAPI]");
    JsWindow* me = CheckParamsAndGetThis<JsWindow>(env, info);
    return (me != nullptr) ? me->OnGetWindowStatus(env, info) : nullptr;
}

napi_value JsWindow::IsFocused(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::WMS_FOCUS, "[NAPI]");
    JsWindow* me = CheckParamsAndGetThis<JsWindow>(env, info);
    return (me != nullptr) ? me->OnIsFocused(env, info) : nullptr;
}

napi_value JsWindow::RequestFocus(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::WMS_FOCUS, "[NAPI]");
    JsWindow* me = CheckParamsAndGetThis<JsWindow>(env, info);
    return (me != nullptr) ? me->OnRequestFocus(env, info) : nullptr;
}

napi_value JsWindow::StartMoving(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::WMS_LAYOUT, "[NAPI]");
    JsWindow* me = CheckParamsAndGetThis<JsWindow>(env, info);
    return (me != nullptr) ? me->OnStartMoving(env, info) : nullptr;
}

napi_value JsWindow::StopMoving(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::WMS_LAYOUT_PC, "[NAPI]");
    JsWindow* me = CheckParamsAndGetThis<JsWindow>(env, info);
    return (me != nullptr) ? me->OnStopMoving(env, info) : nullptr;
}

napi_value JsWindow::CreateSubWindowWithOptions(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::WMS_SUB, "[NAPI]");
    JsWindow* me = CheckParamsAndGetThis<JsWindow>(env, info);
    return (me != nullptr) ? me->OnCreateSubWindowWithOptions(env, info) : nullptr;
}

napi_value JsWindow::SetParentWindow(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::WMS_SUB, "[NAPI]");
    JsWindow* me = CheckParamsAndGetThis<JsWindow>(env, info);
    return (me != nullptr) ? me->OnSetParentWindow(env, info) : nullptr;
}

napi_value JsWindow::GetParentWindow(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::WMS_SUB, "[NAPI]");
    JsWindow* me = CheckParamsAndGetThis<JsWindow>(env, info);
    return (me != nullptr) ? me->OnGetParentWindow(env, info) : nullptr;
}

napi_value JsWindow::SetGestureBackEnabled(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::WMS_IMMS, "[NAPI]");
    JsWindow* me = CheckParamsAndGetThis<JsWindow>(env, info);
    return (me != nullptr) ? me->OnSetGestureBackEnabled(env, info) : nullptr;
}

napi_value JsWindow::GetGestureBackEnabled(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::WMS_IMMS, "[NAPI]");
    JsWindow* me = CheckParamsAndGetThis<JsWindow>(env, info);
    return (me != nullptr) ? me->OnGetGestureBackEnabled(env, info) : nullptr;
}

napi_value JsWindow::GetWindowDensityInfo(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::WMS_ATTRIBUTE, "[NAPI]");
    JsWindow* me = CheckParamsAndGetThis<JsWindow>(env, info);
    return (me != nullptr) ? me->OnGetWindowDensityInfo(env, info) : nullptr;
}

napi_value JsWindow::SetDefaultDensityEnabled(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::WMS_ATTRIBUTE, "[NAPI]");
    JsWindow* me = CheckParamsAndGetThis<JsWindow>(env, info);
    return (me != nullptr) ? me->OnSetDefaultDensityEnabled(env, info) : nullptr;
}

napi_value JsWindow::IsMainWindowFullScreenAcrossDisplays(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::WMS_ATTRIBUTE, "[NAPI]");
    JsWindow* me = CheckParamsAndGetThis<JsWindow>(env, info);
    return (me != nullptr) ? me->OnIsMainWindowFullScreenAcrossDisplays(env, info) : nullptr;
}

napi_value JsWindow::SetSystemAvoidAreaEnabled(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::WMS_IMMS, "[NAPI]");
    JsWindow* me = CheckParamsAndGetThis<JsWindow>(env, info);
    return (me != nullptr) ? me->OnSetSystemAvoidAreaEnabled(env, info) : nullptr;
}

napi_value JsWindow::IsSystemAvoidAreaEnabled(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::WMS_IMMS, "[NAPI]");
    JsWindow* me = CheckParamsAndGetThis<JsWindow>(env, info);
    return (me != nullptr) ? me->OnIsSystemAvoidAreaEnabled(env, info) : nullptr;
}

napi_value JsWindow::SetExclusivelyHighlighted(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::WMS_FOCUS, "[NAPI]");
    JsWindow* me = CheckParamsAndGetThis<JsWindow>(env, info);
    return (me != nullptr) ? me->OnSetExclusivelyHighlighted(env, info) : nullptr;
}

napi_value JsWindow::IsWindowHighlighted(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::WMS_FOCUS, "[NAPI]");
    JsWindow* me = CheckParamsAndGetThis<JsWindow>(env, info);
    return (me != nullptr) ? me->OnIsWindowHighlighted(env, info) : nullptr;
}

napi_value JsWindow::SetDragKeyFramePolicy(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::WMS_LAYOUT_PC, "[NAPI]");
    JsWindow* me = CheckParamsAndGetThis<JsWindow>(env, info);
    return (me != nullptr) ? me->OnSetDragKeyFramePolicy(env, info) : nullptr;
}

napi_value JsWindow::SetRelativePositionToParentWindowEnabled(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::WMS_SUB, "[NAPI]");
    JsWindow* me = CheckParamsAndGetThis<JsWindow>(env, info);
    if (me != nullptr) {
        return me->OnSetRelativePositionToParentWindowEnabled(env, info);
    }
    return NapiThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
        "[window][setRelativePositionToParentWindowEnabled]msg: Window is nullptr.");
}

napi_value JsWindow::SetFollowParentWindowLayoutEnabled(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::WMS_SUB, "[NAPI]");
    JsWindow* me = CheckParamsAndGetThis<JsWindow>(env, info);
    return (me != nullptr) ? me->OnSetFollowParentWindowLayoutEnabled(env, info) : nullptr;
}

napi_value JsWindow::SetReceiveDragEventEnabled(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::WMS_EVENT, "[NAPI]");
    JsWindow* me = CheckParamsAndGetThis<JsWindow>(env, info);
    return (me != nullptr) ? me->OnSetReceiveDragEventEnabled(env, info) : nullptr;
}

napi_value JsWindow::IsReceiveDragEventEnabled(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::WMS_EVENT, "[NAPI]");
    JsWindow* me = CheckParamsAndGetThis<JsWindow>(env, info);
    return (me != nullptr) ? me->OnIsReceiveDragEventEnabled(env, info) : nullptr;
}

napi_value JsWindow::SetSeparationTouchEnabled(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::WMS_EVENT, "[NAPI]");
    JsWindow* me = CheckParamsAndGetThis<JsWindow>(env, info);
    return (me != nullptr) ? me->OnSetSeparationTouchEnabled(env, info) : nullptr;
}

napi_value JsWindow::IsSeparationTouchEnabled(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::WMS_EVENT, "[NAPI]");
    JsWindow* me = CheckParamsAndGetThis<JsWindow>(env, info);
    return (me != nullptr) ? me->OnIsSeparationTouchEnabled(env, info) : nullptr;
}

napi_value JsWindow::SetWindowShadowEnabled(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::WMS_ATTRIBUTE, "[NAPI]");
    JsWindow* me = CheckParamsAndGetThis<JsWindow>(env, info);
    return (me != nullptr) ? me->OnSetWindowShadowEnabled(env, info) : nullptr;
}

napi_value JsWindow::RestoreMainWindow(napi_env env, napi_callback_info info)
{
    JsWindow* me = CheckParamsAndGetThis<JsWindow>(env, info);
    return (me != nullptr) ? me->OnRestoreMainWindow(env, info) : nullptr;
}

WMError UpdateStatusBarProperty(const sptr<Window>& window, const uint32_t contentColor)
{
    if (window == nullptr) {
        TLOGNE(WmsLogTag::WMS_IMMS, "window is nullptr");
        return WMError::WM_ERROR_INVALID_WINDOW;
    }
    auto property = window->GetSystemBarPropertyByType(WindowType::WINDOW_TYPE_STATUS_BAR);
    property.contentColor_ = contentColor;
    property.settingFlag_ = static_cast<SystemBarSettingFlag>(static_cast<uint32_t>(property.settingFlag_) |
        static_cast<uint32_t>(SystemBarSettingFlag::COLOR_SETTING));
    SystemBarPropertyFlag flag = { false, false, true, false };
    return window->UpdateSystemBarPropertyForPage(WindowType::WINDOW_TYPE_STATUS_BAR, property, flag);
}

napi_value NapiGetUndefined(napi_env env)
{
    napi_value result = nullptr;
    napi_get_undefined(env, &result);
    return result;
}

napi_value NapiThrowError(napi_env env, WmErrorCode errCode)
{
    napi_throw(env, JsErrUtils::CreateJsError(env, errCode));
    return NapiGetUndefined(env);
}

napi_value NapiThrowError(napi_env env, WmErrorCode errCode, const std::string& msg)
{
    napi_throw(env, JsErrUtils::CreateJsError(env, errCode, msg));
    return NapiGetUndefined(env);
}

napi_valuetype GetType(napi_env env, napi_value value)
{
    napi_valuetype res = napi_undefined;
    napi_typeof(env, value, &res);
    return res;
}

template <typename T>
bool ConvertFromOptionalJsValue(napi_env env, napi_value jsValue, T& value, const T& defaultValue)
{
    if (GetType(env, jsValue) == napi_undefined) {
        value = defaultValue;
        return true;
    }
    return ConvertFromJsValue(env, jsValue, value);
}

napi_value JsWindow::OnShow(napi_env env, napi_callback_info info)
{
    WMError errCode = WMError::WM_OK;
    size_t argc = 4;
    napi_value argv[4] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc > 1) {
        TLOGE(WmsLogTag::WMS_LIFE, "Argc is invalid: %{public}zu", argc);
        errCode = WMError::WM_ERROR_INVALID_PARAM;
    }
    wptr<Window> weakToken(windowToken_);
    napi_value result = nullptr;
    napi_value lastParam = (argc == 0) ? nullptr : (GetType(env, argv[0]) == napi_function ? argv[0] : nullptr);
    std::shared_ptr<NapiAsyncTask> napiAsyncTask = CreateEmptyAsyncTask(env, lastParam, &result);
    auto asyncTask = [weakToken, errCode, env, task = napiAsyncTask] {
        auto weakWindow = weakToken.promote();
        if (weakWindow == nullptr) {
            TLOGNE(WmsLogTag::WMS_LIFE, "window is nullptr");
            task->Reject(env, JsErrUtils::CreateJsError(env, WMError::WM_ERROR_NULLPTR));
            return;
        }
        if (errCode != WMError::WM_OK) {
            task->Reject(env, JsErrUtils::CreateJsError(env, errCode));
            TLOGNE(WmsLogTag::WMS_LIFE, "window is nullptr or get invalid param");
            return;
        }
        if (WindowHelper::IsMainWindowAndNotShown(weakWindow->GetType(), weakWindow->GetWindowState())) {
            TLOGNW(WmsLogTag::WMS_LIFE,
                "window Type %{public}u and window state %{public}u is not supported, [%{public}u, %{public}s]",
                static_cast<uint32_t>(weakWindow->GetType()), static_cast<uint32_t>(weakWindow->GetWindowState()),
                weakWindow->GetWindowId(), weakWindow->GetWindowName().c_str());
            task->Resolve(env, NapiGetUndefined(env));
            return;
        }
        WMError ret = weakWindow->Show(0, false, true);
        if (ret == WMError::WM_OK) {
            task->Resolve(env, NapiGetUndefined(env));
        } else {
            task->Reject(env, JsErrUtils::CreateJsError(env, ret, "Window show failed"));
        }
        TLOGNI(WmsLogTag::WMS_LIFE, "Window [%{public}u] show end, ret=%{public}d", weakWindow->GetWindowId(), ret);
    };
    if (napi_send_event(env, asyncTask, napi_eprio_high, "OnShow") != napi_status::napi_ok) {
        TLOGE(WmsLogTag::WMS_LIFE, "napi send event failed, window state is abnormal");
    }
    return result;
}

napi_value JsWindow::OnShowWindow(napi_env env, napi_callback_info info)
{
    wptr<Window> weakToken(windowToken_);
    napi_value result = nullptr;
    size_t argc = 4;
    napi_value argv[4] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    napi_value lastParam = (argc == 0) ? nullptr :
        ((argv[0] != nullptr && GetType(env, argv[0]) == napi_function) ? argv[0] : nullptr);
    napi_value showWindowOptions = (argc == 0) ? nullptr :
        ((argv[0] != nullptr && GetType(env, argv[0]) == napi_object) ? argv[0] : nullptr);
    bool focusOnShow = true;
    bool isShowWithOptions = false;
    if (showWindowOptions != nullptr) {
        isShowWithOptions = true;
        WmErrorCode parseRet = ParseShowWindowOptions(env, showWindowOptions, focusOnShow);
        if (parseRet != WmErrorCode::WM_OK) {
            return NapiThrowError(env, parseRet);
        }
    }
    std::shared_ptr<NapiAsyncTask> napiAsyncTask = CreateEmptyAsyncTask(env, lastParam, &result);
    auto asyncTask = [weakToken, env, task = napiAsyncTask, focusOnShow, isShowWithOptions] {
        auto weakWindow = weakToken.promote();
        if (weakWindow == nullptr) {
            task->Reject(env, JsErrUtils::CreateJsError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
                "[window][showWindow]msg: Window is nullptr."));
            TLOGNE(WmsLogTag::WMS_LIFE, "window is nullptr or get invalid param");
            return;
        }
        if (WindowHelper::IsMainWindowAndNotShown(weakWindow->GetType(), weakWindow->GetWindowState())) {
            TLOGNW(WmsLogTag::WMS_LIFE,
                "window Type %{public}u and window state %{public}u is not supported, [%{public}u, %{public}s]",
                static_cast<uint32_t>(weakWindow->GetType()), static_cast<uint32_t>(weakWindow->GetWindowState()),
                weakWindow->GetWindowId(), weakWindow->GetWindowName().c_str());
            task->Resolve(env, NapiGetUndefined(env));
            return;
        }
        if (focusOnShow == false &&
            (WindowHelper::IsModalSubWindow(weakWindow->GetType(), weakWindow->GetWindowFlags()) ||
             WindowHelper::IsDialogWindow(weakWindow->GetType()))) {
            task->Reject(env, JsErrUtils::CreateJsError(env, WmErrorCode::WM_ERROR_INVALID_CALLING));
            TLOGNE(WmsLogTag::WMS_FOCUS, "only normal sub window supports setting focusOnShow");
            return;
        }
        weakWindow->SetShowWithOptions(isShowWithOptions);
        WMError ret = weakWindow->Show(0, false, focusOnShow, true);
        TLOGNI(WmsLogTag::WMS_LIFE, "Window [%{public}u, %{public}s] show with ret=%{public}d",
            weakWindow->GetWindowId(), weakWindow->GetWindowName().c_str(), ret);
        if (ret == WMError::WM_OK) {
            task->Resolve(env, NapiGetUndefined(env));
        } else {
            task->Reject(env, JsErrUtils::CreateJsError(env, WM_JS_TO_ERROR_CODE_MAP.at(ret),
                "[window][showWindow]msg: Window show failed."));
        }
        TLOGNI(WmsLogTag::WMS_LIFE, "Window [%{public}u, %{public}s] show end, ret=%{public}d",
            weakWindow->GetWindowId(), weakWindow->GetWindowName().c_str(), ret);
    };
    if (napi_send_event(env, asyncTask, napi_eprio_high, "OnShowWindow") != napi_status::napi_ok) {
        TLOGE(WmsLogTag::WMS_LIFE, "napi send event failed, window state is abnormal");
    }
    return result;
}

napi_value JsWindow::OnShowWithAnimation(napi_env env, napi_callback_info info)
{
    WmErrorCode errCode = Permission::IsSystemCallingOrStartByHdcd(true) ?
        WmErrorCode::WM_OK : WmErrorCode::WM_ERROR_NOT_SYSTEM_APP;
    if (errCode == WmErrorCode::WM_OK) {
        if (windowToken_ == nullptr) {
            errCode = WmErrorCode::WM_ERROR_STATE_ABNORMALLY;
        } else {
            auto winType = windowToken_->GetType();
            if (!WindowHelper::IsSystemWindow(winType)) {
                TLOGE(WmsLogTag::WMS_LIFE,
                    "Window Type %{public}u is not supported", static_cast<uint32_t>(winType));
                errCode = WmErrorCode::WM_ERROR_INVALID_CALLING;
            }
        }
    }
    wptr<Window> weakToken(windowToken_);
    const char* const where = __func__;
    napi_value result = nullptr;
    size_t argc = 4;
    napi_value argv[4] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    napi_value lastParam = (argc == 0) ? nullptr :
        ((argv[0] != nullptr && GetType(env, argv[0]) == napi_function) ? argv[0] : nullptr);
    std::shared_ptr<NapiAsyncTask> napiAsyncTask = CreateEmptyAsyncTask(env, lastParam, &result);
    auto asyncTask = [weakToken, errCode, where, env, task = napiAsyncTask] {
        if (errCode != WmErrorCode::WM_OK) {
            task->Reject(env, JsErrUtils::CreateJsError(env, errCode));
            return;
        }
        auto weakWindow = weakToken.promote();
        if (weakWindow == nullptr) {
            TLOGNE(WmsLogTag::WMS_LIFE, "window is nullptr");
            task->Reject(env, JsErrUtils::CreateJsError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY));
            return;
        }
        WmErrorCode ret = WM_JS_TO_ERROR_CODE_MAP.at(weakWindow->Show(0, true, true));
        if (ret == WmErrorCode::WM_OK) {
            task->Resolve(env, NapiGetUndefined(env));
        } else {
            task->Reject(env, JsErrUtils::CreateJsError(env, ret, "Window show failed"));
        }
        TLOGNI(WmsLogTag::WMS_LIFE, "%{public}s Window [%{public}u, %{public}s] end, ret=%{public}d",
            where, weakWindow->GetWindowId(), weakWindow->GetWindowName().c_str(), ret);
    };
    if (napi_send_event(env, asyncTask, napi_eprio_high, "OnShowWithAnimation") != napi_status::napi_ok) {
        TLOGE(WmsLogTag::WMS_LIFE, "napi send event failed, window state is abnormal");
    }
    return result;
}

napi_value JsWindow::OnDestroy(napi_env env, napi_callback_info info)
{
    WMError errCode = WMError::WM_OK;
    size_t argc = 4;
    napi_value argv[4] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc > 1) {
        TLOGE(WmsLogTag::WMS_LIFE, "Argc is invalid: %{public}zu", argc);
        errCode = WMError::WM_ERROR_INVALID_PARAM;
    }
    wptr<Window> weakToken(windowToken_);
    const char* const where = __func__;
    napi_value lastParam = (argc == 0) ? nullptr : (GetType(env, argv[0]) == napi_function ? argv[0] : nullptr);
    napi_value result = nullptr;
    std::shared_ptr<NapiAsyncTask> napiAsyncTask = CreateEmptyAsyncTask(env, lastParam, &result);
    auto asyncTask = [this, weakToken, errCode, where, env, task = napiAsyncTask] {
        auto weakWindow = weakToken.promote();
        if (weakWindow == nullptr) {
            TLOGNE(WmsLogTag::WMS_LIFE, "window is nullptr");
            task->Reject(env, JsErrUtils::CreateJsError(env, WMError::WM_ERROR_NULLPTR));
            return;
        }
        if (errCode != WMError::WM_OK) {
            task->Reject(env, JsErrUtils::CreateJsError(env, errCode));
            TLOGNE(WmsLogTag::WMS_LIFE, "window is nullptr or get invalid param");
            return;
        }
        if (WindowHelper::IsMainWindow(weakWindow->GetType())) {
            TLOGNW(WmsLogTag::WMS_LIFE, "window Type %{public}u is not supported, [%{public}u, %{public}s]",
                static_cast<uint32_t>(weakWindow->GetType()),
                weakWindow->GetWindowId(), weakWindow->GetWindowName().c_str());
            task->Resolve(env, NapiGetUndefined(env));
            return;
        }
        WMError ret = weakWindow->Destroy();
        TLOGNI(WmsLogTag::WMS_LIFE, "%{public}s Window [%{public}u, %{public}s] end, ret=%{public}d",
            where, weakWindow->GetWindowId(), weakWindow->GetWindowName().c_str(), ret);
        if (ret != WMError::WM_OK) {
            task->Reject(env, JsErrUtils::CreateJsError(env, ret, "Window destroy failed"));
            return;
        }
        windowToken_ = nullptr; // ensure window dtor when finalizer invalid
        task->Resolve(env, NapiGetUndefined(env));
    };
    if (napi_send_event(env, asyncTask, napi_eprio_high, "OnDestroy") != napi_status::napi_ok) {
        TLOGE(WmsLogTag::WMS_LIFE, "napi send event failed, window state is abnormal");
    }
    return result;
}

napi_value JsWindow::OnDestroyWindow(napi_env env, napi_callback_info info)
{
    wptr<Window> weakToken(windowToken_);
    const char* const where = __func__;
    size_t argc = 4;
    napi_value argv[4] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    napi_value lastParam = (argc == 0) ? nullptr :
        ((argv[0] != nullptr && GetType(env, argv[0]) == napi_function) ? argv[0] : nullptr);
    napi_value result = nullptr;
    std::shared_ptr<NapiAsyncTask> napiAsyncTask = CreateEmptyAsyncTask(env, lastParam, &result);
    auto asyncTask = [this, weakToken, where, env, task = napiAsyncTask] {
        auto weakWindow = weakToken.promote();
        if (weakWindow == nullptr) {
            TLOGNE(WmsLogTag::WMS_LIFE, "window is nullptr or get invalid param");
            task->Reject(env, JsErrUtils::CreateJsError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
                "[window][destroyWindow]msg: Window is nullptr."));
            return;
        }
        if (WindowHelper::IsMainWindow(weakWindow->GetType())) {
            TLOGNW(WmsLogTag::WMS_LIFE, "window Type %{public}u is not supported, [%{public}u, %{public}s]",
                static_cast<uint32_t>(weakWindow->GetType()),
                weakWindow->GetWindowId(), weakWindow->GetWindowName().c_str());
            task->Resolve(env, NapiGetUndefined(env));
            return;
        }
        WmErrorCode ret = WM_JS_TO_ERROR_CODE_MAP.at(weakWindow->Destroy());
        TLOGNI(WmsLogTag::WMS_LIFE, "%{public}s end, window [%{public}u, %{public}s] ret=%{public}d",
            where, weakWindow->GetWindowId(), weakWindow->GetWindowName().c_str(), ret);
        if (ret != WmErrorCode::WM_OK) {
            task->Reject(env, JsErrUtils::CreateJsError(env, ret,
                "[window][destroyWindow]msg: Window destroy failed."));
            return;
        }
        windowToken_ = nullptr; // ensure window dtor when finalizer invalid
        task->Resolve(env, NapiGetUndefined(env));
    };
    if (napi_send_event(env, asyncTask, napi_eprio_high, "OnDestroyWindow") != napi_status::napi_ok) {
        TLOGE(WmsLogTag::WMS_LIFE, "napi send event failed, window state is abnormal");
    }
    return result;
}

napi_value JsWindow::OnHide(napi_env env, napi_callback_info info)
{
    WmErrorCode errCode = Permission::IsSystemCallingOrStartByHdcd(true) ?
        WmErrorCode::WM_OK : WmErrorCode::WM_ERROR_NOT_SYSTEM_APP;
    return HideWindowFunction(env, info, errCode);
}

napi_value JsWindow::HideWindowFunction(napi_env env, napi_callback_info info, WmErrorCode errCode)
{
    wptr<Window> weakToken(windowToken_);
    const char* const where = __func__;
    size_t argc = 4;
    napi_value argv[4] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    napi_value lastParam = (argc == 0) ? nullptr :
        (argv[0] != nullptr && GetType(env, argv[0]) == napi_function ? argv[0] : nullptr);
    napi_value result = nullptr;
    std::shared_ptr<NapiAsyncTask> napiAsyncTask = CreateEmptyAsyncTask(env, lastParam, &result);
    auto asyncTask = [weakToken, errCode, where, env, task = napiAsyncTask] {
        if (errCode != WmErrorCode::WM_OK) {
            task->Reject(env, JsErrUtils::CreateJsError(env, errCode));
            return;
        }
        auto weakWindow = weakToken.promote();
        if (weakWindow == nullptr) {
            TLOGNE(WmsLogTag::WMS_LIFE, "window is nullptr or get invalid param");
            task->Reject(env, JsErrUtils::CreateJsError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
                "[window][hide]msg: Window is nullptr."));
            return;
        }
        if (WindowHelper::IsMainWindow(weakWindow->GetType())) {
            TLOGNW(WmsLogTag::WMS_LIFE, "window Type %{public}u is not supported, [%{public}u, %{public}s]",
                static_cast<uint32_t>(weakWindow->GetType()),
                weakWindow->GetWindowId(), weakWindow->GetWindowName().c_str());
            task->Resolve(env, NapiGetUndefined(env));
            return;
        }
        WmErrorCode ret = WM_JS_TO_ERROR_CODE_MAP.at(weakWindow->Hide(0, false, false, true));
        if (ret == WmErrorCode::WM_OK) {
            task->Resolve(env, NapiGetUndefined(env));
        } else {
            task->Reject(env, JsErrUtils::CreateJsError(env, ret,
                "[window][hide]msg: Window hide failed."));
        }
        TLOGNI(WmsLogTag::WMS_LIFE, "%{public}s end, window [%{public}u] ret=%{public}d", where,
            weakWindow->GetWindowId(), ret);
    };
    if (napi_send_event(env, asyncTask, napi_eprio_high, "HideWindowFunction") != napi_status::napi_ok) {
        TLOGE(WmsLogTag::WMS_LIFE, "napi send event failed, window state is abnormal");
    }
    return result;
}

napi_value JsWindow::OnHideWithAnimation(napi_env env, napi_callback_info info)
{
    WmErrorCode errCode = Permission::IsSystemCallingOrStartByHdcd(true) ?
        WmErrorCode::WM_OK : WmErrorCode::WM_ERROR_NOT_SYSTEM_APP;
    if (errCode == WmErrorCode::WM_OK) {
        if (windowToken_) {
            auto winType = windowToken_->GetType();
            if (!WindowHelper::IsSystemWindow(winType)) {
                TLOGE(WmsLogTag::WMS_LIFE,
                    "window Type %{public}u is not supported", static_cast<uint32_t>(winType));
                errCode = WmErrorCode::WM_ERROR_INVALID_CALLING;
            }
        } else {
            errCode = WmErrorCode::WM_ERROR_STATE_ABNORMALLY;
        }
    }
    wptr<Window> weakToken(windowToken_);
    const char* const where = __func__;
    size_t argc = 4;
    napi_value argv[4] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    napi_value lastParam = (argc == 0) ? nullptr :
        ((argv[0] != nullptr && GetType(env, argv[0]) == napi_function) ? argv[0] : nullptr);
    napi_value result = nullptr;
    std::shared_ptr<NapiAsyncTask> napiAsyncTask = CreateEmptyAsyncTask(env, lastParam, &result);
    auto asyncTask = [weakToken, errCode, where, env, task = napiAsyncTask] {
        if (errCode != WmErrorCode::WM_OK) {
            task->Reject(env, JsErrUtils::CreateJsError(env, errCode));
            return;
        }
        auto weakWindow = weakToken.promote();
        if (weakWindow == nullptr) {
            TLOGNE(WmsLogTag::WMS_LIFE, "window is nullptr");
            task->Reject(env, JsErrUtils::CreateJsError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY));
            return;
        }
        WmErrorCode ret = WM_JS_TO_ERROR_CODE_MAP.at(weakWindow->Hide(0, true, false));
        if (ret == WmErrorCode::WM_OK) {
            task->Resolve(env, NapiGetUndefined(env));
        } else {
            task->Reject(env, JsErrUtils::CreateJsError(env, ret, "Window show failed"));
        }
        TLOGNI(WmsLogTag::WMS_LIFE, "%{public}s end, window [%{public}u, %{public}s] ret=%{public}d",
            where, weakWindow->GetWindowId(), weakWindow->GetWindowName().c_str(), ret);
    };
    if (napi_send_event(env, asyncTask, napi_eprio_high, "OnHideWithAnimation") != napi_status::napi_ok) {
        TLOGE(WmsLogTag::WMS_LIFE, "napi send event failed, window state is abnormal");
    }
    return result;
}

napi_value JsWindow::OnRecover(napi_env env, napi_callback_info info)
{
    size_t argc = FOUR_PARAMS_SIZE;
    napi_value argv[FOUR_PARAMS_SIZE] = { nullptr };
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    napi_value lastParam = (argc == 0) ? nullptr :
        (GetType(env, argv[INDEX_ZERO]) == napi_function ? argv[INDEX_ZERO] : nullptr);
    napi_value result = nullptr;
    std::shared_ptr<NapiAsyncTask> napiAsyncTask = CreateEmptyAsyncTask(env, lastParam, &result);
    const char* const where = __func__;
    auto asyncTask = [windowToken = wptr<Window>(windowToken_), env, task = napiAsyncTask, where] {
        auto window = windowToken.promote();
        if (window == nullptr) {
            TLOGNE(WmsLogTag::WMS_LAYOUT_PC, "%{public}s window is nullptr", where);
            task->Reject(env, JsErrUtils::CreateJsError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
                 "[window][recover]msg: Window is nullptr"));
            return;
        }
        WmErrorCode ret = WM_JS_TO_ERROR_CODE_MAP.at(window->Recover(1));
        if (ret == WmErrorCode::WM_OK) {
            task->Resolve(env, NapiGetUndefined(env));
        } else {
            task->Reject(env, JsErrUtils::CreateJsError(env, ret, "[window][recover]msg: Failed"));
        }
        TLOGNI(WmsLogTag::WMS_LAYOUT_PC, "%{public}s end, window [%{public}u] ret=%{public}d",
            where, window->GetWindowId(), ret);
    };
    if (napi_send_event(env, asyncTask, napi_eprio_immediate, "OnRecover") != napi_status::napi_ok) {
        napiAsyncTask->Reject(env, CreateJsError(env,
            static_cast<int32_t>(WmErrorCode::WM_ERROR_STATE_ABNORMALLY), "[window][recover]msg: Send event failed"));
    }
    return result;
}

napi_value JsWindow::OnRestore(napi_env env, napi_callback_info info)
{
    size_t argc = FOUR_PARAMS_SIZE;
    napi_value argv[FOUR_PARAMS_SIZE] = { nullptr };
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    napi_value lastParam = (argc == 0) ? nullptr :
        (GetType(env, argv[INDEX_ZERO]) == napi_function ? argv[INDEX_ZERO] : nullptr);
    napi_value result = nullptr;
    std::shared_ptr<NapiAsyncTask> napiAsyncTask = CreateEmptyAsyncTask(env, lastParam, &result);
    const char* const where = __func__;
    auto asyncTask = [windowToken = wptr<Window>(windowToken_), env, task = napiAsyncTask, where] {
        auto window = windowToken.promote();
        if (window == nullptr) {
            TLOGNE(WmsLogTag::WMS_LAYOUT_PC, "%{public}s window is nullptr", where);
            task->Reject(env, JsErrUtils::CreateJsError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY));
            return;
        }
        if (!WindowHelper::IsMainWindow(window->GetType())) {
            TLOGNE(WmsLogTag::WMS_LAYOUT_PC, "%{public}s Restore fail, not main window", where);
            task->Reject(env,
                JsErrUtils::CreateJsError(env, WmErrorCode::WM_ERROR_INVALID_CALLING));
            return;
        }
        WmErrorCode ret = WM_JS_TO_ERROR_CODE_MAP.at(window->Restore());
        if (ret == WmErrorCode::WM_OK) {
            task->Resolve(env, NapiGetUndefined(env));
        } else {
            task->Reject(env, JsErrUtils::CreateJsError(env, ret, "Window restore failed"));
        }
    };
    if (napi_send_event(env, asyncTask, napi_eprio_immediate, "OnRestore") != napi_status::napi_ok) {
        napiAsyncTask->Reject(env, CreateJsError(env,
            static_cast<int32_t>(WmErrorCode::WM_ERROR_STATE_ABNORMALLY), "send event failed"));
    }
    return result;
}

/** @note @window.layout */
napi_value JsWindow::OnMoveTo(napi_env env, napi_callback_info info)
{
    WMError errCode = WMError::WM_OK;
    size_t argc = 4;
    napi_value argv[4] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc < 2 || argc > 3) { // 2:minimum param num, 3: maximum param num
        TLOGE(WmsLogTag::WMS_LAYOUT, "Argc is invalid: %{public}zu", argc);
        errCode = WMError::WM_ERROR_INVALID_PARAM;
    }
    int32_t x = 0;
    if (errCode == WMError::WM_OK && !ConvertFromJsValue(env, argv[0], x)) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "Failed to convert parameter to x");
        errCode = WMError::WM_ERROR_INVALID_PARAM;
    }

    int32_t y = 0;
    if (errCode == WMError::WM_OK && !ConvertFromJsValue(env, argv[1], y)) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "Failed to convert parameter to y");
        errCode = WMError::WM_ERROR_INVALID_PARAM;
    }
    // 2: params num; 2: index of callback
    napi_value lastParam = (argc <= 2) ? nullptr : (GetType(env, argv[2]) == napi_function ? argv[2] : nullptr);
    napi_value result = nullptr;
    std::shared_ptr<NapiAsyncTask> napiAsyncTask = CreateEmptyAsyncTask(env, lastParam, &result);
    auto asyncTask = [windowToken = wptr<Window>(windowToken_), errCode, x, y,
                      env, task = napiAsyncTask, where = __func__] {
        if (errCode != WMError::WM_OK) {
            task->Reject(env, JsErrUtils::CreateJsError(env, errCode));
            TLOGNE(WmsLogTag::WMS_LAYOUT, "%{public}s: invalid param", where);
            return;
        }
        auto window = windowToken.promote();
        if (window == nullptr) {
            TLOGNE(WmsLogTag::WMS_LAYOUT, "%{public}s: window is nullptr", where);
            task->Reject(env, JsErrUtils::CreateJsError(env, WMError::WM_ERROR_NULLPTR));
            return;
        }
        WMError ret = window->MoveTo(x, y);
        if (ret == WMError::WM_OK) {
            task->Resolve(env, NapiGetUndefined(env));
        } else {
            task->Reject(env, JsErrUtils::CreateJsError(env, ret, "Window move failed"));
        }
        TLOGND(WmsLogTag::WMS_LAYOUT, "%{public}s: end, window [%{public}u, %{public}s] ret=%{public}d",
               where, window->GetWindowId(), window->GetWindowName().c_str(), ret);
    };
    if (napi_send_event(env, asyncTask, napi_eprio_high, "OnMoveTo") != napi_status::napi_ok) {
        napiAsyncTask->Reject(env,
            JsErrUtils::CreateJsError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY, "failed to send event"));
    }
    return result;
}

/** @note @window.layout */
napi_value JsWindow::OnMoveWindowTo(napi_env env, napi_callback_info info)
{
    WmErrorCode errCode = WmErrorCode::WM_OK;
    size_t argc = 4;
    napi_value argv[4] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc < 2) { // 2:minimum param num
        TLOGE(WmsLogTag::WMS_LAYOUT, "Argc is invalid: %{public}zu", argc);
        errCode = WmErrorCode::WM_ERROR_INVALID_PARAM;
    }
    int32_t x = 0;
    if (errCode == WmErrorCode::WM_OK && !ConvertFromJsValue(env, argv[0], x)) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "Failed to convert parameter to x");
        errCode = WmErrorCode::WM_ERROR_INVALID_PARAM;
    }
    int32_t y = 0;
    if (errCode == WmErrorCode::WM_OK && !ConvertFromJsValue(env, argv[1], y)) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "Failed to convert parameter to y");
        errCode = WmErrorCode::WM_ERROR_INVALID_PARAM;
    }
    if (errCode == WmErrorCode::WM_ERROR_INVALID_PARAM) {
        return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM,
            "[window][moveWindowTo]msg: Failed");
    }
    // 2: params num; 2: index of callback
    napi_value lastParam = (argc <= 2) ? nullptr :
        ((argv[2] != nullptr && GetType(env, argv[2]) == napi_function) ? argv[2] : nullptr);
    napi_value result = nullptr;
    std::shared_ptr<NapiAsyncTask> napiAsyncTask = CreateEmptyAsyncTask(env, lastParam, &result);
    auto asyncTask = [windowToken = wptr<Window>(windowToken_), x, y,
                      env, task = napiAsyncTask, where = __func__] {
        auto window = windowToken.promote();
        if (window == nullptr) {
            TLOGNE(WmsLogTag::WMS_LAYOUT, "%{public}s: window is nullptr", where);
            task->Reject(env, JsErrUtils::CreateJsError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
                 "[window][moveWindowTo]msg: Window is nullptr"));
            return;
        }
        WmErrorCode ret = WM_JS_TO_ERROR_CODE_MAP.at(window->MoveTo(x, y));
        if (ret == WmErrorCode::WM_OK) {
            task->Resolve(env, NapiGetUndefined(env));
        } else {
            task->Reject(env, JsErrUtils::CreateJsError(env, ret,
                 "[window][moveWindowTo]msg: Move failed"));
        }
        TLOGND(WmsLogTag::WMS_LAYOUT, "%{public}s: window [%{public}u, %{public}s] ret=%{public}d",
               where, window->GetWindowId(), window->GetWindowName().c_str(), ret);
    };
    if (napi_send_event(env, asyncTask, napi_eprio_high, "OnMoveWindowTo") != napi_status::napi_ok) {
        napiAsyncTask->Reject(env,
            JsErrUtils::CreateJsError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
                "[window][moveWindowTo]msg: Failed to send event"));
    }
    return result;
}

static void SetMoveWindowToAsyncTask(NapiAsyncTask::ExecuteCallback& execute, NapiAsyncTask::CompleteCallback& complete,
    const wptr<Window>& weakToken, int32_t x, int32_t y, MoveConfiguration moveConfiguration)
{
    std::shared_ptr<WmErrorCode> errCodePtr = std::make_shared<WmErrorCode>(WmErrorCode::WM_OK);
    const char* const where = __func__;
    execute = [weakToken, errCodePtr, x, y, moveConfiguration, where] {
        if (errCodePtr == nullptr) {
            return;
        }
        if (*errCodePtr != WmErrorCode::WM_OK) {
            return;
        }
        auto weakWindow = weakToken.promote();
        if (weakWindow == nullptr) {
            TLOGE(WmsLogTag::WMS_LAYOUT, "window is nullptr");
            *errCodePtr = WmErrorCode::WM_ERROR_STATE_ABNORMALLY;
            return;
        }
        *errCodePtr = WM_JS_TO_ERROR_CODE_MAP.at(weakWindow->MoveToAsync(x, y, moveConfiguration));
        TLOGND(WmsLogTag::WMS_LAYOUT, "%{public}s end, window [%{public}u, %{public}s] err=%{public}d",
            where, weakWindow->GetWindowId(), weakWindow->GetWindowName().c_str(), *errCodePtr);
    };
    complete = [weakToken, errCodePtr](napi_env env, NapiAsyncTask& task, int32_t status) {
        if (errCodePtr == nullptr) {
            task.Reject(env, JsErrUtils::CreateJsError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY));
            return;
        }
        if (*errCodePtr == WmErrorCode::WM_OK) {
            task.Resolve(env, NapiGetUndefined(env));
        } else {
            task.Reject(env, JsErrUtils::CreateJsError(env, *errCodePtr, "JsWindow::OnMoveWindowToAsync failed"));
        }
    };
}

/** @note @window.layout */
napi_value JsWindow::OnMoveWindowToAsync(napi_env env, napi_callback_info info)
{
    size_t argc = FOUR_PARAMS_SIZE;
    napi_value argv[FOUR_PARAMS_SIZE] = { nullptr };
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc < 2) { // 2: minimum param num
        TLOGE(WmsLogTag::WMS_LAYOUT, "Argc is invalid: %{public}zu", argc);
        return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM,
            "[window][moveWindowToAsync]msg: Number of parameters is invalid");
    }
    int32_t x = 0;
    if (!ConvertFromJsValue(env, argv[INDEX_ZERO], x)) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "Failed to convert parameter to x");
        return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM,
            "[window][moveWindowToAsync]msg: Failed to convert parameter to x");
    }
    int32_t y = 0;
    if (!ConvertFromJsValue(env, argv[INDEX_ONE], y)) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "Failed to convert parameter to y");
        return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM,
            "[window][moveWindowToAsync]msg: Failed to convert parameter to y");
    }
    MoveConfiguration moveConfiguration;
    size_t lastParamIndex = INDEX_TWO;
    if (argc > 2 && argv[INDEX_TWO] != nullptr && GetType(env, argv[INDEX_TWO]) == napi_object) { // 2: x/y params num
        lastParamIndex = INDEX_THREE; // MoveConfiguration is optional param
        if (!GetMoveConfigurationFromJsValue(env, argv[INDEX_TWO], moveConfiguration)) {
            TLOGE(WmsLogTag::WMS_LAYOUT, "Failed to convert parameter to moveConfiguration");
            return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM,
                "[window][moveWindowToAsync]msg: Failed to convert parameter to moveConfiguration");
        }
    }
    NapiAsyncTask::ExecuteCallback execute;
    NapiAsyncTask::CompleteCallback complete;
    SetMoveWindowToAsyncTask(execute, complete, wptr<Window>(windowToken_), x, y, moveConfiguration);

    napi_value lastParam = (argc <= lastParamIndex) ? nullptr :
        ((argv[lastParamIndex] != nullptr && GetType(env, argv[lastParamIndex]) == napi_function) ?
         argv[lastParamIndex] : nullptr);
    napi_value result = nullptr;
    NapiAsyncTask::Schedule("JsWindow::OnMoveWindowToAsync",
        env, CreateAsyncTaskWithLastParam(env, lastParam, std::move(execute), std::move(complete), &result));
    return result;
}

static void SetMoveWindowToGlobalAsyncTask(NapiAsyncTask::ExecuteCallback& execute,
    NapiAsyncTask::CompleteCallback& complete, const wptr<Window>& weakToken,
    int32_t x, int32_t y, MoveConfiguration moveConfiguration)
{
    std::shared_ptr<WmErrorCode> errCodePtr = std::make_shared<WmErrorCode>(WmErrorCode::WM_OK);
    const char* const where = __func__;
    execute = [weakToken, errCodePtr, x, y, moveConfiguration, where] {
        if (errCodePtr == nullptr) {
            return;
        }
        if (*errCodePtr != WmErrorCode::WM_OK) {
            return;
        }
        auto window = weakToken.promote();
        if (window == nullptr) {
            TLOGNE(WmsLogTag::WMS_LAYOUT, "%{public}s window is nullptr", where);
            *errCodePtr = WmErrorCode::WM_ERROR_STATE_ABNORMALLY;
            return;
        }
        *errCodePtr = WM_JS_TO_ERROR_CODE_MAP.at(window->MoveWindowToGlobal(x, y, moveConfiguration));
        TLOGND(WmsLogTag::WMS_LAYOUT, "%{public}s end, window [%{public}u, %{public}s] err=%{public}d",
            where, window->GetWindowId(), window->GetWindowName().c_str(), *errCodePtr);
    };
    complete = [weakToken, errCodePtr](napi_env env, NapiAsyncTask& task, int32_t status) {
        if (errCodePtr == nullptr) {
            task.Reject(env, JsErrUtils::CreateJsError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY));
            return;
        }
        if (*errCodePtr == WmErrorCode::WM_OK) {
            task.Resolve(env, NapiGetUndefined(env));
        } else {
            task.Reject(env, JsErrUtils::CreateJsError(
                env, *errCodePtr, "JsWindow::OnMoveWindowToGlobal failed"));
        }
    };
}

/** @note @window.layout */
napi_value JsWindow::OnMoveWindowToGlobal(napi_env env, napi_callback_info info)
{
    size_t argc = FOUR_PARAMS_SIZE;
    napi_value argv[FOUR_PARAMS_SIZE] = { nullptr };
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc < 2) { // 2:minimum param num
        TLOGE(WmsLogTag::WMS_LAYOUT, "Argc is invalid: %{public}zu", argc);
        return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM,
            "[window][moveWindowToGlobal]msg: Number of parameters is invalid");
    }
    int32_t x = 0;
    if (!ConvertFromJsValue(env, argv[INDEX_ZERO], x)) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "Failed to convert parameter to x");
        return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM,
            "[window][moveWindowToGlobal]msg: Failed to convert parameter to x");
    }
    int32_t y = 0;
    if (!ConvertFromJsValue(env, argv[INDEX_ONE], y)) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "Failed to convert parameter to y");
        return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM,
            "[window][moveWindowToGlobal]msg: Failed to convert parameter to y");
    }
    MoveConfiguration moveConfiguration;
    size_t lastParamIndex = INDEX_TWO;
    if (argc > TWO_PARAMS_SIZE && argv[INDEX_TWO] != nullptr && GetType(env, argv[INDEX_TWO]) == napi_object) {
        lastParamIndex = INDEX_THREE; // MoveConfiguration is optional param
        if (!GetMoveConfigurationFromJsValue(env, argv[INDEX_TWO], moveConfiguration)) {
            TLOGE(WmsLogTag::WMS_LAYOUT, "Failed to convert parameter to moveConfiguration");
            return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM,
                "[window][moveWindowToGlobal]msg: Failed to convert parameter to moveConfiguration");
        }
    }
    NapiAsyncTask::ExecuteCallback execute;
    NapiAsyncTask::CompleteCallback complete;
    SetMoveWindowToGlobalAsyncTask(execute, complete, wptr<Window>(windowToken_), x, y, moveConfiguration);

    napi_value lastParam = (argc <= lastParamIndex) ? nullptr :
        ((argv[lastParamIndex] != nullptr && GetType(env, argv[lastParamIndex]) == napi_function) ?
         argv[lastParamIndex] : nullptr);
    napi_value result = nullptr;
    NapiAsyncTask::Schedule("JsWindow::OnMoveWindowToGlobal",
        env, CreateAsyncTaskWithLastParam(env, lastParam, std::move(execute), std::move(complete), &result));
    return result;
}

/** @note @window.layout */
napi_value JsWindow::OnMoveWindowToGlobalDisplay(napi_env env, napi_callback_info info)
{
    size_t argc = TWO_PARAMS_SIZE;
    napi_value argv[TWO_PARAMS_SIZE] = { nullptr };
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc != TWO_PARAMS_SIZE) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "Invalid argc: %{public}zu", argc);
        return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM,
            "[window][moveWindowToGlobalDisplay]msg: Invalid argc");
    }

    int32_t x = 0;
    if (!ConvertFromJsValue(env, argv[INDEX_ZERO], x)) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "Failed to convert parameter to x");
        return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM,
            "[window][moveWindowToGlobalDisplay]msg: Failed to convert parameter to x");
    }
    int32_t y = 0;
    if (!ConvertFromJsValue(env, argv[INDEX_ONE], y)) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "Failed to convert parameter to y");
        return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM,
            "[window][moveWindowToGlobalDisplay]msg: Failed to convert parameter to y");
    }

    napi_value result = nullptr;
    std::shared_ptr<NapiAsyncTask> napiAsyncTask = CreateEmptyAsyncTask(env, nullptr, &result);
    auto asyncTask = [windowToken = wptr<Window>(windowToken_), x, y, env, napiAsyncTask, where = __func__] {
        auto window = windowToken.promote();
        if (!window) {
            TLOGNE(WmsLogTag::WMS_LAYOUT, "%{public}s: window is nullptr", where);
            napiAsyncTask->Reject(env, JsErrUtils::CreateJsError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
                "[window][moveWindowToGlobalDisplay]msg: Window is nullptr"));
            return;
        }
        auto moveResult = window->MoveWindowToGlobalDisplay(x, y);
        auto it = WM_JS_TO_ERROR_CODE_MAP.find(moveResult);
        WmErrorCode ret = (it != WM_JS_TO_ERROR_CODE_MAP.end()) ? it->second : WmErrorCode::WM_ERROR_STATE_ABNORMALLY;
        if (ret == WmErrorCode::WM_OK) {
            napiAsyncTask->Resolve(env, NapiGetUndefined(env));
        } else {
            napiAsyncTask->Reject(env, JsErrUtils::CreateJsError(env, ret,
                "[window][moveWindowToGlobalDisplay]msg: Failed to move window"));
        }
        TLOGNI(WmsLogTag::WMS_LAYOUT,
            "%{public}s: window: [%{public}u, %{public}s], x: %{public}d, y: %{public}d, ret: %{public}d",
            where, window->GetWindowId(), window->GetWindowName().c_str(), x, y, static_cast<int32_t>(ret));
    };
    if (napi_status::napi_ok != napi_send_event(env, asyncTask, napi_eprio_high)) {
        napiAsyncTask->Reject(env,
            JsErrUtils::CreateJsError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
                "[window][moveWindowToGlobalDisplay]msg: Failed to send event"));
    }
    return result;
}

/** @note @window.layout */
napi_value JsWindow::OnGetGlobalScaledRect(napi_env env, napi_callback_info info)
{
    if (windowToken_ == nullptr) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "window is nullptr");
        return NapiThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
            "[window][getGlobalScaledRect]msg: Window is nullptr");
    }
    Rect globalScaledRect;
    WmErrorCode ret = WM_JS_TO_ERROR_CODE_MAP.at(windowToken_->GetGlobalScaledRect(globalScaledRect));
    if (ret != WmErrorCode::WM_OK) {
        return NapiThrowError(env, ret, "[window][getGlobalScaledRect]");
    }
    TLOGI(WmsLogTag::WMS_LAYOUT, "Window [%{public}u, %{public}s] end",
        windowToken_->GetWindowId(), windowToken_->GetWindowName().c_str());
    napi_value globalScaledRectObj = GetRectAndConvertToJsValue(env, globalScaledRect);
    if (globalScaledRectObj == nullptr) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "globalScaledRectObj is nullptr");
        return NapiThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
            "[window][getGlobalScaledRect]msg: Failed to convert result into JS value object.");
    }
    return globalScaledRectObj;
}

/** @note @window.layout */
napi_value JsWindow::OnResize(napi_env env, napi_callback_info info)
{
    WMError errCode = WMError::WM_OK;
    size_t argc = 4;
    napi_value argv[4] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc < 2 || argc > 3) { // 2: minimum param num, 3: maximum param num
        TLOGE(WmsLogTag::WMS_LAYOUT, "Argc is invalid: %{public}zu", argc);
        errCode = WMError::WM_ERROR_INVALID_PARAM;
    }
    int32_t width = 0;
    if (errCode == WMError::WM_OK && !ConvertFromJsValue(env, argv[0], width)) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "Failed to convert parameter to width");
        errCode = WMError::WM_ERROR_INVALID_PARAM;
    }
    int32_t height = 0;
    if (errCode == WMError::WM_OK && !ConvertFromJsValue(env, argv[1], height)) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "Failed to convert parameter to height");
        errCode = WMError::WM_ERROR_INVALID_PARAM;
    }
    if (width <= 0 || height <= 0) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "width or height should greater than 0!");
        errCode = WMError::WM_ERROR_INVALID_PARAM;
    }
    // 2: params num; 2: index of callback
    napi_value lastParam = (argc <= 2) ? nullptr : (GetType(env, argv[2]) == napi_function ? argv[2] : nullptr);
    napi_value result = nullptr;
    std::shared_ptr<NapiAsyncTask> napiAsyncTask = CreateEmptyAsyncTask(env, lastParam, &result);
    auto asyncTask = [windowToken = wptr<Window>(windowToken_), errCode, width, height,
                      env, task = napiAsyncTask, where = __func__] {
        if (errCode != WMError::WM_OK) {
            task->Reject(env, JsErrUtils::CreateJsError(env, errCode, "[window][resize]msg: Invalid param"));
            TLOGNE(WmsLogTag::WMS_LAYOUT, "%{public}s: invalid param", where);
            return;
        }
        auto window = windowToken.promote();
        if (window == nullptr) {
            TLOGNE(WmsLogTag::WMS_LAYOUT, "%{public}s: window is nullptr", where);
            task->Reject(env, JsErrUtils::CreateJsError(env, WMError::WM_ERROR_NULLPTR,
                "[window][resize]msg: Window is nullptr"));
            return;
        }
        WMError ret = window->Resize(static_cast<uint32_t>(width), static_cast<uint32_t>(height));
        if (ret == WMError::WM_OK) {
            task->Resolve(env, NapiGetUndefined(env));
        } else {
            task->Reject(env, JsErrUtils::CreateJsError(env, ret,
                "[window][resize]msg: Failed"));
        }
        TLOGND(WmsLogTag::WMS_LAYOUT, "%{public}s: end, window [%{public}u, %{public}s] ret=%{public}d",
               where, window->GetWindowId(), window->GetWindowName().c_str(), ret);
    };
    if (napi_send_event(env, asyncTask, napi_eprio_high, "OnResize") != napi_status::napi_ok) {
        napiAsyncTask->Reject(env,
            JsErrUtils::CreateJsError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
                "[window][resize]msg: Failed to send event"));
    }
    return result;
}

/** @note @window.layout */
napi_value JsWindow::OnResizeWindow(napi_env env, napi_callback_info info)
{
    WmErrorCode errCode = WmErrorCode::WM_OK;
    size_t argc = 4;
    napi_value argv[4] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc < 2) { // 2: minimum param num
        TLOGE(WmsLogTag::WMS_LAYOUT, "Argc is invalid: %{public}zu", argc);
        errCode = WmErrorCode::WM_ERROR_INVALID_PARAM;
    }
    int32_t width = 0;
    if (errCode == WmErrorCode::WM_OK && !ConvertFromJsValue(env, argv[0], width)) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "Failed to convert parameter to width");
        errCode = WmErrorCode::WM_ERROR_INVALID_PARAM;
    }
    int32_t height = 0;
    if (errCode == WmErrorCode::WM_OK && !ConvertFromJsValue(env, argv[1], height)) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "Failed to convert parameter to height");
        errCode = WmErrorCode::WM_ERROR_INVALID_PARAM;
    }
    if (width <= 0 || height <= 0) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "width or height should greater than 0!");
        errCode = WmErrorCode::WM_ERROR_INVALID_PARAM;
    }
    if (errCode == WmErrorCode::WM_ERROR_INVALID_PARAM) {
        return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM);
    }
    // 2: params num; 2: index of callback
    napi_value lastParam = (argc <= 2) ? nullptr :
        ((argv[2] != nullptr && GetType(env, argv[2]) == napi_function) ? argv[2] : nullptr);
    napi_value result = nullptr;
    std::shared_ptr<NapiAsyncTask> napiAsyncTask = CreateEmptyAsyncTask(env, lastParam, &result);
    auto asyncTask = [windowToken = wptr<Window>(windowToken_), width, height,
                      env, task = napiAsyncTask, where = __func__] {
        auto window = windowToken.promote();
        if (window == nullptr) {
            TLOGNE(WmsLogTag::WMS_LAYOUT, "%{public}s: window is nullptr", where);
            task->Reject(env, JsErrUtils::CreateJsError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY));
            return;
        }
        WmErrorCode ret = WM_JS_TO_ERROR_CODE_MAP.at(
            window->Resize(static_cast<uint32_t>(width), static_cast<uint32_t>(height)));
        if (ret == WmErrorCode::WM_OK) {
            task->Resolve(env, NapiGetUndefined(env));
        } else {
            task->Reject(env, JsErrUtils::CreateJsError(env, ret, "Window resize failed"));
        }
        TLOGND(WmsLogTag::WMS_LAYOUT, "%{public}s: window [%{public}u, %{public}s] ret=%{public}d",
               where, window->GetWindowId(), window->GetWindowName().c_str(), ret);
    };
    if (napi_send_event(env, asyncTask, napi_eprio_high, "OnResizeWindow") != napi_status::napi_ok) {
        napiAsyncTask->Reject(env,
            JsErrUtils::CreateJsError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY, "failed to send event"));
    }
    return result;
}

static void SetResizeWindowAsyncTask(NapiAsyncTask::ExecuteCallback& execute, NapiAsyncTask::CompleteCallback& complete,
    wptr<Window> weakToken, int32_t width, int32_t height)
{
    std::shared_ptr<WmErrorCode> errCodePtr = std::make_shared<WmErrorCode>(WmErrorCode::WM_OK);
    const char* const where = __func__;
    execute = [weakToken, errCodePtr, width, height, where] {
        if (errCodePtr == nullptr) {
            return;
        }
        if (*errCodePtr != WmErrorCode::WM_OK) {
            return;
        }
        auto weakWindow = weakToken.promote();
        if (weakWindow == nullptr) {
            TLOGE(WmsLogTag::WMS_LAYOUT, "window is nullptr");
            *errCodePtr = WmErrorCode::WM_ERROR_STATE_ABNORMALLY;
            return;
        }
        *errCodePtr = WM_JS_TO_ERROR_CODE_MAP.at(
            weakWindow->ResizeAsync(static_cast<uint32_t>(width), static_cast<uint32_t>(height)));
        TLOGND(WmsLogTag::WMS_LAYOUT, "%{public}s end, window [%{public}u, %{public}s] err=%{public}d",
            where, weakWindow->GetWindowId(), weakWindow->GetWindowName().c_str(), *errCodePtr);
    };
    complete = [weakToken, errCodePtr](napi_env env, NapiAsyncTask& task, int32_t status) {
        if (errCodePtr == nullptr) {
            task.Reject(env, JsErrUtils::CreateJsError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY));
            return;
        }
        if (*errCodePtr == WmErrorCode::WM_OK) {
            task.Resolve(env, NapiGetUndefined(env));
        } else {
            task.Reject(env, JsErrUtils::CreateJsError(env, *errCodePtr, "[window][resizeWindowAsync]"));
        }
    };
}

/** @note @window.layout */
napi_value JsWindow::OnResizeWindowAsync(napi_env env, napi_callback_info info)
{
    WmErrorCode errCode = WmErrorCode::WM_OK;
    size_t argc = 4; // 4: number of arg
    napi_value argv[4] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    std::string errorMsg;
    if (argc < 2) { // 2: minimum param num
        TLOGE(WmsLogTag::WMS_LAYOUT, "Argc is invalid: %{public}zu", argc);
        errCode = WmErrorCode::WM_ERROR_INVALID_PARAM;
        errorMsg = "The number of parameters is invalid";
    }
    int32_t width = 0;
    if (errCode == WmErrorCode::WM_OK && !ConvertFromJsValue(env, argv[0], width)) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "Failed to convert parameter to width");
        errCode = WmErrorCode::WM_ERROR_INVALID_PARAM;
        errorMsg = "Failed to convert parameter to width";
    }
    int32_t height = 0;
    if (errCode == WmErrorCode::WM_OK && !ConvertFromJsValue(env, argv[1], height)) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "Failed to convert parameter to height");
        errCode = WmErrorCode::WM_ERROR_INVALID_PARAM;
        errorMsg = "Failed to convert parameter to height";
    }
    if (width <= 0 || height <= 0) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "width or height should greater than 0!");
        errCode = WmErrorCode::WM_ERROR_INVALID_PARAM;
        errorMsg = "width or height should greater than 0!";
    }
    if (errCode == WmErrorCode::WM_ERROR_INVALID_PARAM) {
        return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM,
            "[window][resizeWindowAsync]msg: " + errorMsg);
    }

    wptr<Window> weakToken(windowToken_);
    NapiAsyncTask::ExecuteCallback execute;
    NapiAsyncTask::CompleteCallback complete;
    SetResizeWindowAsyncTask(execute, complete, weakToken, width, height);

    // 2: params num; 2: index of callback
    napi_value lastParam = (argc <= 2) ? nullptr :
        ((argv[2] != nullptr && GetType(env, argv[2]) == napi_function) ? argv[2] : nullptr);
    napi_value result = nullptr;
    NapiAsyncTask::Schedule("JsWindow::OnResizeWindowAsync",
        env, CreateAsyncTaskWithLastParam(env, lastParam, std::move(execute), std::move(complete), &result));
    return result;
}

static void SetResizeWindowWithAnimationAsyncTask(NapiAsyncTask::ExecuteCallback& execute,
    NapiAsyncTask::CompleteCallback& complete, const wptr<Window>& weakToken, const Rect& rect,
    const RectAnimationConfig& rectAnimationConfig)
{
    std::shared_ptr<WmErrorCode> errCodePtr = std::make_shared<WmErrorCode>(WmErrorCode::WM_OK);
    const char* const where = __func__;
    execute = [weakToken, errCodePtr, rect, rectAnimationConfig, where] {
        if (errCodePtr == nullptr) {
            return;
        }
        auto window = weakToken.promote();
        if (window == nullptr) {
            TLOGNE(WmsLogTag::WMS_LAYOUT, "%{public}s window is nullptr", where);
            *errCodePtr = WmErrorCode::WM_ERROR_STATE_ABNORMALLY;
            return;
        }
        *errCodePtr = WM_JS_TO_ERROR_CODE_MAP.at(window->ResizeAsync(rect.width_, rect.height_));
        TLOGND(WmsLogTag::WMS_LAYOUT, "%{public}s end, window [%{public}u, %{public}s] err=%{public}d",
            where, window->GetWindowId(), window->GetWindowName().c_str(), *errCodePtr);
    };
    complete = [errCodePtr, where](napi_env env, NapiAsyncTask& task, int32_t status) {
        if (errCodePtr == nullptr) {
            task.Reject(env, JsErrUtils::CreateJsError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY));
            return;
        }
        if (*errCodePtr == WmErrorCode::WM_OK) {
            TLOGNI(WmsLogTag::WMS_LAYOUT, "%{public}s window resize success", where);
            task.Resolve(env, NapiGetUndefined(env));
        } else {
            task.Reject(env, JsErrUtils::CreateJsError(env, *errCodePtr,
                "JsWindow::OnResizeWindowWithAnimation failed"));
        }
    };
}

/** @note @window.layout */
napi_value JsWindow::OnResizeWindowWithAnimation(napi_env env, napi_callback_info info)
{
    size_t argc = FOUR_PARAMS_SIZE;
    napi_value argv[FOUR_PARAMS_SIZE] = { nullptr };
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc < THREE_PARAMS_SIZE) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "Argc is invalid: %{public}zu", argc);
        return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM);
    }
    int32_t width = 0;
    if (!ConvertFromJsValue(env, argv[INDEX_ZERO], width)) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "Failed to convert parameter to width");
        return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM);
    }
    int32_t height = 0;
    if (!ConvertFromJsValue(env, argv[INDEX_ONE], height)) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "Failed to convert parameter to height");
        return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM);
    }
    if (width <= 0 || height <= 0) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "width or height should greater than 0!");
        return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM);
    }

    RectAnimationConfig rectAnimationConfig;
    if (argv[INDEX_TWO] == nullptr || !ParseRectAnimationConfig(env, argv[INDEX_TWO], rectAnimationConfig)) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "Failed to convert object to rectAnimationConfig");
        return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM);
    }

    NapiAsyncTask::ExecuteCallback execute;
    NapiAsyncTask::CompleteCallback complete;
    Rect rect = { 0, 0, width, height };
    SetResizeWindowWithAnimationAsyncTask(execute, complete, wptr<Window>(windowToken_), rect,
        rectAnimationConfig);
    napi_value lastParam = (argc <= THREE_PARAMS_SIZE) ? nullptr :
        ((argv[INDEX_THREE] != nullptr && GetType(env, argv[INDEX_THREE]) == napi_function) ?
         argv[INDEX_THREE] : nullptr);
    napi_value result = nullptr;
    NapiAsyncTask::Schedule("JsWindow::OnResizeWindowWithAnimation",
        env, CreateAsyncTaskWithLastParam(env, lastParam, std::move(execute), std::move(complete), &result));
    return result;
}

/** @note @window.layout */
template <typename PositionTransformFunc>
napi_value JsWindow::HandlePositionTransform(
    napi_env env, napi_callback_info info, PositionTransformFunc transformFunc, const char* caller)
{
    size_t argc = TWO_PARAMS_SIZE;
    napi_value argv[TWO_PARAMS_SIZE] = { nullptr };
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc != TWO_PARAMS_SIZE) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "%{public}s: Invalid argc: %{public}zu", caller, argc);
        return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM);
    }

    int32_t x = 0;
    if (!ConvertFromJsValue(env, argv[INDEX_ZERO], x)) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "%{public}s: Failed to convert parameter to x", caller);
        return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM);
    }
    int32_t y = 0;
    if (!ConvertFromJsValue(env, argv[INDEX_ONE], y)) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "%{public}s: Failed to convert parameter to y", caller);
        return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM);
    }

    if (!windowToken_) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "%{public}s: window is nullptr", caller);
        return NapiThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
    }

    Position inPosition { x, y };
    Position outPosition;
    auto transformRet = transformFunc(windowToken_, inPosition, outPosition);
    auto it = WM_JS_TO_ERROR_CODE_MAP.find(transformRet);
    WmErrorCode errCode = (it != WM_JS_TO_ERROR_CODE_MAP.end()) ? it->second : WmErrorCode::WM_ERROR_STATE_ABNORMALLY;
    if (errCode != WmErrorCode::WM_OK) {
        return NapiThrowError(env, errCode);
    }
    TLOGI(WmsLogTag::WMS_LAYOUT, "%{public}s: windowId: %{public}u, inPosition: %{public}s, outPosition: %{public}s",
        caller, windowToken_->GetWindowId(), inPosition.ToString().c_str(), outPosition.ToString().c_str());

    auto jsOutPosition = BuildJsPosition(env, outPosition);
    if (!jsOutPosition) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "%{public}s: Failed to build JS position object", caller);
        return NapiThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
    }
    return jsOutPosition;
}

/** @note @window.layout */
napi_value JsWindow::OnClientToGlobalDisplay(napi_env env, napi_callback_info info)
{
    return HandlePositionTransform(
        env, info,
        [](const sptr<Window>& window, const Position& inPosition, Position& outPosition) {
            return window->ClientToGlobalDisplay(inPosition, outPosition);
        },
        __func__);
}

/** @note @window.layout */
napi_value JsWindow::OnGlobalDisplayToClient(napi_env env, napi_callback_info info)
{
    return HandlePositionTransform(
        env, info,
        [](const sptr<Window>& window, const Position& inPosition, Position& outPosition) {
            return window->GlobalDisplayToClient(inPosition, outPosition);
        },
        __func__);
}

napi_value JsWindow::OnSetWindowType(napi_env env, napi_callback_info info)
{
    WMError errCode = WMError::WM_OK;
    size_t argc = 4;
    napi_value argv[4] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc < 1 || argc > 2) { // 2 is max num of argc
        WLOGFE("Argc is invalid: %{public}zu", argc);
        errCode = WMError::WM_ERROR_INVALID_PARAM;
    }
    WindowType winType = WindowType::SYSTEM_WINDOW_BASE;
    uint32_t resultValue = 0;
    if (errCode == WMError::WM_OK && !ConvertFromJsValue(env, argv[0], resultValue)) {
        WLOGFE("Failed to convert parameter to windowType");
        errCode = WMError::WM_ERROR_INVALID_PARAM;
    }
    if (resultValue >= static_cast<uint32_t>(WindowType::SYSTEM_WINDOW_BASE) &&
        resultValue <= static_cast<uint32_t>(WindowType::SYSTEM_WINDOW_END)) {
        winType = static_cast<WindowType>(resultValue); // adapt to the old version
    } else if (JS_TO_NATIVE_WINDOW_TYPE_MAP.count(static_cast<ApiWindowType>(resultValue)) != 0) {
        winType = JS_TO_NATIVE_WINDOW_TYPE_MAP.at(static_cast<ApiWindowType>(resultValue));
    } else {
        WLOGFE("Do not support this type: %{public}u", resultValue);
        errCode = WMError::WM_ERROR_INVALID_PARAM;
    }

    wptr<Window> weakToken(windowToken_);
    const char* const where = __func__;
    NapiAsyncTask::CompleteCallback complete =
        [weakToken, winType, errCode, where](napi_env env, NapiAsyncTask& task, int32_t status) {
            auto weakWindow = weakToken.promote();
            if (weakWindow == nullptr) {
                WLOGFE("window is nullptr");
                task.Reject(env, JsErrUtils::CreateJsError(env, WMError::WM_ERROR_NULLPTR));
                return;
            }
            if (errCode != WMError::WM_OK) {
                task.Reject(env, JsErrUtils::CreateJsError(env, errCode));
                WLOGFE("get invalid param");
                return;
            }
            WMError ret = weakWindow->SetWindowType(winType);
            if (ret == WMError::WM_OK) {
                task.Resolve(env, NapiGetUndefined(env));
            } else {
                task.Reject(env, JsErrUtils::CreateJsError(env, ret, "Window set type failed"));
            }
            WLOGI("%{public}s end, window [%{public}u, %{public}s] ret=%{public}d",
                where, weakWindow->GetWindowId(), weakWindow->GetWindowName().c_str(), ret);
        };
    napi_value lastParam = (argc <= 1) ? nullptr :
        (GetType(env, argv[1]) == napi_function ? argv[1] : nullptr);
    napi_value result = nullptr;
    NapiAsyncTask::Schedule("JsWindow::OnSetWindowType",
        env, CreateAsyncTaskWithLastParam(env, lastParam, nullptr, std::move(complete), &result));
    return result;
}

/** @note @window.layout */
napi_value JsWindow::OnSetWindowMode(napi_env env, napi_callback_info info)
{
    if (!Permission::IsSystemCalling() && !Permission::IsStartByHdcd()) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "permission denied!");
        return NapiThrowError(env, WmErrorCode::WM_ERROR_NOT_SYSTEM_APP,
            "[window][setwindowMode]msg: Permission denied!");
    }
    WmErrorCode errCode = WmErrorCode::WM_OK;
    size_t argc = 4;
    napi_value argv[4] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc < 1) {
        errCode = WmErrorCode::WM_ERROR_INVALID_PARAM;
    }
    WindowMode winMode = WindowMode::WINDOW_MODE_FULLSCREEN;
    if (errCode == WmErrorCode::WM_OK) {
        napi_value nativeMode = argv[0];
        if (nativeMode == nullptr) {
            errCode = WmErrorCode::WM_ERROR_INVALID_PARAM;
        } else {
            uint32_t resultValue = 0;
            napi_get_value_uint32(env, nativeMode, &resultValue);
            if (resultValue >= static_cast<uint32_t>(WindowMode::WINDOW_MODE_SPLIT_PRIMARY)) {
                winMode = static_cast<WindowMode>(resultValue);
            } else if (resultValue >= static_cast<uint32_t>(ApiWindowMode::UNDEFINED) &&
                resultValue <= static_cast<uint32_t>(ApiWindowMode::MODE_END)) {
                winMode = JS_TO_NATIVE_WINDOW_MODE_MAP.at(
                    static_cast<ApiWindowMode>(resultValue));
            } else {
                errCode = WmErrorCode::WM_ERROR_INVALID_PARAM;
            }
        }
    }
    if (errCode == WmErrorCode::WM_ERROR_INVALID_PARAM) {
        return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM, "[window][setwindowMode]msg: Failed");
    }
    napi_value lastParam = (argc == 1) ? nullptr :
        ((argv[1] != nullptr && GetType(env, argv[1]) == napi_function) ? argv[1] : nullptr);
    napi_value result = nullptr;
    std::shared_ptr<NapiAsyncTask> napiAsyncTask = CreateEmptyAsyncTask(env, lastParam, &result);
    auto asyncTask = [windowToken = wptr<Window>(windowToken_), winMode,
                      env, task = napiAsyncTask, where = __func__] {
        auto window = windowToken.promote();
        if (window == nullptr) {
            TLOGNE(WmsLogTag::WMS_LAYOUT, "%{public}s: window is nullptr", where);
            task->Reject(env, JsErrUtils::CreateJsError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
                "[window][setwindowMode]msg: Window is nullptr"));
            return;
        }
        WmErrorCode ret = WM_JS_TO_ERROR_CODE_MAP.at(window->SetWindowMode(winMode));
        if (ret == WmErrorCode::WM_OK) {
            task->Resolve(env, NapiGetUndefined(env));
        } else {
            task->Reject(env, JsErrUtils::CreateJsError(env, ret,
                "[window][setwindowMode]msg: Window set mode failed"));
        }
        TLOGNI(WmsLogTag::WMS_LAYOUT, "%{public}s: end, window [%{public}u, %{public}s] ret=%{public}d",
               where, window->GetWindowId(), window->GetWindowName().c_str(), ret);
    };
    if (napi_send_event(env, asyncTask, napi_eprio_high, "OnSetWindowMode") != napi_status::napi_ok) {
        napiAsyncTask->Reject(env,
            JsErrUtils::CreateJsError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
                "[window][setwindowMode]msg: Failed to send event"));
    }
    return result;
}

napi_value JsWindow::OnGetProperties(napi_env env, napi_callback_info info)
{
    WMError errCode = WMError::WM_OK;
    size_t argc = 4;
    napi_value argv[4] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc > 1) {
        WLOGFE("Argc is invalid: %{public}zu", argc);
        errCode = WMError::WM_ERROR_INVALID_PARAM;
    }
    napi_value lastParam = (argc == 0) ? nullptr : (GetType(env, argv[0]) == napi_function ? argv[0] : nullptr);
    napi_value result = nullptr;
    std::shared_ptr<NapiAsyncTask> napiAsyncTask = CreateEmptyAsyncTask(env, lastParam, &result);
    auto asyncTask = [weakToken = wptr<Window>(windowToken_), errCode, env, task = napiAsyncTask] {
        auto window = weakToken.promote();
        if (window == nullptr) {
            WLOGFE("window is nullptr");
            task->Reject(env, JsErrUtils::CreateJsError(env, WMError::WM_ERROR_NULLPTR));
            return;
        }
        if (errCode != WMError::WM_OK) {
            task->Reject(env, JsErrUtils::CreateJsError(env, errCode));
            WLOGFE("window is nullptr or get invalid param");
            return;
        }
        WindowPropertyInfo windowPropertyInfo;
        WMError ret = window->GetWindowPropertyInfo(windowPropertyInfo);
        if (ret != WMError::WM_OK) {
            task->Reject(env, JsErrUtils::CreateJsError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY));
            return;
        }
        auto objValue = CreateJsWindowPropertiesObject(env, windowPropertyInfo);
        if (objValue != nullptr) {
            task->Resolve(env, objValue);
        } else {
            task->Reject(env, JsErrUtils::CreateJsError(env, WMError::WM_ERROR_NULLPTR,
                "Window get properties failed"));
        }
        WLOGFD("Window [%{public}u, %{public}s] get properties end",
            window->GetWindowId(), window->GetWindowName().c_str());
    };
    if (napi_send_event(env, asyncTask, napi_eprio_high, "OnGetProperties") != napi_status::napi_ok) {
        TLOGE(WmsLogTag::WMS_IMMS, "napi_send_event failed");
        napiAsyncTask->Reject(env,
            JsErrUtils::CreateJsError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY, "failed to send event"));
    }
    return result;
}

napi_value JsWindow::OnGetWindowPropertiesSync(napi_env env, napi_callback_info info)
{
    if (windowToken_ == nullptr) {
        WLOGFW("window is nullptr or get invalid param");
        return NapiThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
            "[window][getWindowProperties]msg: The window is not created or destroyed");
    }
    WindowPropertyInfo windowPropertyInfo;
    WMError ret = windowToken_->GetWindowPropertyInfo(windowPropertyInfo);
    if (ret != WMError::WM_OK) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "get window properties failed");
        return NapiThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
            "[window][getWindowProperties]msg: The window is not created or destroyed");
    }
    auto objValue = CreateJsWindowPropertiesObject(env, windowPropertyInfo);
    TLOGNI(WmsLogTag::WMS_ATTRIBUTE, "Get Prop, wid: %{public}u", windowToken_->GetWindowId());
    if (objValue != nullptr) {
        return objValue;
    } else {
        return NapiThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
            "[window][getWindowProperties]msg: Internal task error");
    }
}

bool NapiIsCallable(napi_env env, napi_value value)
{
    bool result = false;
    napi_is_callable(env, value, &result);
    return result;
}

napi_value JsWindow::OnRegisterWindowCallback(napi_env env, napi_callback_info info)
{
    if (windowToken_ == nullptr) {
        WLOGFE("Window is nullptr");
        return NapiThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
            "[window][on]msg: Window is nullptr.");
    }
    sptr<Window> windowToken = windowToken_;
    constexpr size_t argcMin = 2;
    constexpr size_t argcMax = 3;
    size_t argc = 4;
    napi_value argv[4] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc < argcMin || argc > argcMax) {
        WLOGFE("Argc is invalid: %{public}zu", argc);
        return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM);
    }
    std::string cbType;
    if (!ConvertFromJsValue(env, argv[0], cbType)) {
        WLOGFE("Failed to convert parameter to callbackType");
        return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM);
    }
    size_t cbIndex = argc - 1;
    napi_value callback = argv[cbIndex];
    if (!NapiIsCallable(env, callback)) {
        WLOGI("Callback(info->argv[%{public}zu]) is not callable", cbIndex);
        return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM);
    }

    napi_value parameter = nullptr;
    if (argc > argcMin) {
        parameter = argv[cbIndex - 1];
    }

    WmErrorCode ret = registerManager_->RegisterListener(windowToken, cbType, CaseType::CASE_WINDOW,
        env, callback, parameter);
    if (ret != WmErrorCode::WM_OK) {
        return NapiThrowError(env, ret, "[window][on]msg: Register listener failed.");
    }
    TLOGI(WmsLogTag::DEFAULT, "Id=%{public}u, type=%{public}s", windowToken->GetWindowId(), cbType.c_str());
    // if comptible mode app adpt to immersive, avoid area change will be called when regist
    if (cbType == AVOID_AREA_CHANGE_CB && windowToken->IsAdaptToCompatibleImmersive()) {
        TLOGI(WmsLogTag::WMS_COMPAT, "notify avoid area change for compatible mode app when regist callback");
        windowToken->HookCompatibleModeAvoidAreaNotify();
    }
    return NapiGetUndefined(env);
}

napi_value JsWindow::OnUnregisterWindowCallback(napi_env env, napi_callback_info info)
{
    if (windowToken_ == nullptr) {
        WLOGFE("Window is nullptr");
        return NapiThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY, "[window][off]msg: Window is nullptr.");
    }
    size_t argc = 4;
    napi_value argv[4] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc < 1) { // 2: maximum params nums
        WLOGFE("Argc is invalid: %{public}zu", argc);
        return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM);
    }
    std::string cbType;
    if (!ConvertFromJsValue(env, argv[0], cbType)) {
        WLOGFE("Failed to convert parameter to callbackType");
        return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM);
    }

    napi_value value = nullptr;
    WmErrorCode ret = WmErrorCode::WM_OK;
    if (argc == 1) {
        ret = registerManager_->UnregisterListener(windowToken_, cbType, CaseType::CASE_WINDOW, env, value);
    } else {
        value = argv[1];
        if (value == nullptr || !NapiIsCallable(env, value)) {
            ret = registerManager_->UnregisterListener(windowToken_, cbType, CaseType::CASE_WINDOW, env, nullptr);
        } else {
            ret = registerManager_->UnregisterListener(windowToken_, cbType, CaseType::CASE_WINDOW, env, value);
        }
    }

    if (ret != WmErrorCode::WM_OK) {
        return NapiThrowError(env, ret, "[window][off]msg: Unregister listener failed.");
    }
    WLOGFI("Unregister end, window [%{public}u, %{public}s], type=%{public}s",
        windowToken_->GetWindowId(), windowToken_->GetWindowName().c_str(), cbType.c_str());
    return NapiGetUndefined(env);
}

static sptr<IRemoteObject> GetBindDialogToken(napi_env env, napi_value argv0)
{
    sptr<IRemoteObject> token = NAPI_ohos_rpc_getNativeRemoteObject(env, argv0);
    if (token != nullptr) {
        return token;
    }
    std::shared_ptr<AbilityRuntime::RequestInfo> requestInfo =
        AbilityRuntime::RequestInfo::UnwrapRequestInfo(env, argv0);
    return (requestInfo != nullptr) ? requestInfo->GetToken() : nullptr;
}

napi_value JsWindow::OnBindDialogTarget(napi_env env, napi_callback_info info)
{
    if (windowToken_ == nullptr) {
        TLOGE(WmsLogTag::WMS_DIALOG, "window is nullptr!");
        return NapiThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
            "[window][bindDialogTarget]msg: Window is nullptr.");
    }
    if (!Permission::IsSystemCalling()) {
        TLOGE(WmsLogTag::WMS_DIALOG, "permission denied, require system application!");
        return NapiThrowError(env, WmErrorCode::WM_ERROR_NOT_SYSTEM_APP);
    }

    size_t argc = 4;
    napi_value argv[4] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);

    if (argc < 2) { // at least 2 params
        return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM);
    }
    sptr<IRemoteObject> token = GetBindDialogToken(env, argv[0]);
    if (token == nullptr) {
        TLOGE(WmsLogTag::WMS_DIALOG, "token is null!");
        return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM);
    }
    napi_value value = argv[1];
    if (value == nullptr || !NapiIsCallable(env, value)) {
        registerManager_->RegisterListener(windowToken_,
            "dialogDeathRecipient", CaseType::CASE_WINDOW, env, nullptr);
    } else {
        registerManager_->RegisterListener(windowToken_, "dialogDeathRecipient", CaseType::CASE_WINDOW, env, value);
    }

    napi_value result = nullptr;
    const int lastParamIdx = 2;
    napi_value lastParam = (argc == lastParamIdx) ? nullptr :
        (GetType(env, argv[lastParamIdx]) == napi_function ? argv[lastParamIdx] : nullptr);
    std::shared_ptr<NapiAsyncTask> napiAsyncTask = CreateEmptyAsyncTask(env, lastParam, &result);
    wptr<Window> weakToken(windowToken_);
    auto asyncTask = [weakToken, env, task = napiAsyncTask, token]() {
        auto weakWindow = weakToken.promote();
        if (weakWindow == nullptr) {
            task->Reject(env, JsErrUtils::CreateJsError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
                "[window][bindDialogTarget]msg: Weak window is nullptr."));
            return;
        }
        WmErrorCode ret = WM_JS_TO_ERROR_CODE_MAP.at(weakWindow->BindDialogTarget(token));
        if (ret == WmErrorCode::WM_OK) {
            task->Resolve(env, NapiGetUndefined(env));
        } else {
            task->Reject(env, JsErrUtils::CreateJsError(env, ret,
                "[window][bindDialogTarget]msg: Bind dialog target failed."));
        }

        TLOGI(WmsLogTag::WMS_SYSTEM, "BindDialogTarget end, window [%{public}u, %{public}s]",
            weakToken->GetWindowId(), weakToken->GetWindowName().c_str());
    };
    if (napi_send_event(env, asyncTask, napi_eprio_immediate, "OnBindDialogTarget") != napi_status::napi_ok) {
        napiAsyncTask->Reject(env, CreateJsError(env, static_cast<int32_t>(WmErrorCode::WM_ERROR_STATE_ABNORMALLY),
            "[window][bindDialogTarget]msg: Send event failed."));
    }
    return result;
}

napi_value JsWindow::OnSetDialogBackGestureEnabled(napi_env env, napi_callback_info info)
{
    size_t argc = 4;
    napi_value argv[4] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc < 1) { // at least 1 params
        return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM,
            "[window][setDialogBackGestureEnabled]msg: Mandatory parameters are left unspecified");
    }

    napi_value nativeVal = argv[0];
    if (nativeVal == nullptr) {
        TLOGE(WmsLogTag::WMS_DIALOG, "Failed to convert parameter to enable");
    }
    bool isEnabled = false;
    napi_status retCode = napi_get_value_bool(env, nativeVal, &isEnabled);
    if (retCode != napi_ok) {
        return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM,
            "[window][setDialogBackGestureEnabled]msg: Incorrect parameter types");
    }

    wptr<Window> weakToken(windowToken_);
    std::shared_ptr<WmErrorCode> errCodePtr = std::make_shared<WmErrorCode>(WmErrorCode::WM_OK);
    NapiAsyncTask::ExecuteCallback execute = [weakToken, isEnabled, errCodePtr] {
        if (errCodePtr == nullptr) {
            return;
        }
        auto window = weakToken.promote();
        if (window == nullptr) {
            *errCodePtr = WmErrorCode::WM_ERROR_STATE_ABNORMALLY;
            return;
        }
        *errCodePtr = WM_JS_TO_ERROR_CODE_MAP.at(window->SetDialogBackGestureEnabled(isEnabled));
        TLOGI(WmsLogTag::WMS_DIALOG, "Window [%{public}u, %{public}s] set dialog window end",
            window->GetWindowId(), window->GetWindowName().c_str());
    };
    NapiAsyncTask::CompleteCallback complete =
        [weakToken, errCodePtr](napi_env env, NapiAsyncTask& task, int32_t status) {
            if (errCodePtr == nullptr) {
                task.Reject(env, JsErrUtils::CreateJsError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
                    "[window][setDialogBackGestureEnabled]"));
                return;
            }
            *errCodePtr == WmErrorCode::WM_OK ? task.Resolve(env, NapiGetUndefined(env)) :
                task.Reject(env, JsErrUtils::CreateJsError(env, *errCodePtr, "[window][setDialogBackGestureEnabled]"));
        };
    napi_value result = nullptr;
    NapiAsyncTask::Schedule("JsWindow::OnSetTopmost",
        env, CreateAsyncTaskWithLastParam(env, nullptr, std::move(execute), std::move(complete), &result));
    return result;
}

static void LoadContentTask(std::shared_ptr<NativeReference> contentStorage, std::string contextUrl,
    sptr<Window> weakWindow, napi_env env, NapiAsyncTask& task, bool isLoadedByName)
{
    napi_value nativeStorage =  (contentStorage == nullptr) ? nullptr : contentStorage->GetNapiValue();
    AppExecFwk::Ability* ability = nullptr;
    GetAPI7Ability(env, ability);
    WmErrorCode ret;
    if (isLoadedByName) {
        ret = WM_JS_TO_ERROR_CODE_MAP.at(weakWindow->SetUIContentByName(contextUrl, env, nativeStorage, ability));
    } else {
        ret = WM_JS_TO_ERROR_CODE_MAP.at(
            weakWindow->NapiSetUIContent(contextUrl, env, nativeStorage, BackupAndRestoreType::NONE, nullptr, ability));
    }
    if (ret == WmErrorCode::WM_OK) {
        task.Resolve(env, NapiGetUndefined(env));
    } else {
        task.Reject(env, JsErrUtils::CreateJsError(env, ret,
            "[window][loadContent]msg: Window load content failed."));
    }
    WLOGFI("end, window [%{public}u, %{public}s] ret=%{public}d",
        weakWindow->GetWindowId(), weakWindow->GetWindowName().c_str(), ret);
    return;
}

napi_value JsWindow::LoadContentScheduleOld(napi_env env, napi_callback_info info, bool isLoadedByName)
{
    WMError errCode = WMError::WM_OK;
    size_t argc = 4;
    napi_value argv[4] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc < 1 || argc > 2) { // 2 maximum param num
        TLOGE(WmsLogTag::WMS_LIFE, "Argc is invalid: %{public}zu", argc);
        errCode = WMError::WM_ERROR_INVALID_PARAM;
    }
    std::string contextUrl;
    if (errCode == WMError::WM_OK && !ConvertFromJsValue(env, argv[0], contextUrl)) {
        TLOGE(WmsLogTag::WMS_LIFE, "Failed to convert parameter to context url");
        errCode = WMError::WM_ERROR_INVALID_PARAM;
    }
    napi_value callBack = nullptr;
    if (argc == 2) { // 2 param num
        callBack = argv[1];
    }
    std::shared_ptr<NativeReference> contentStorage = nullptr;
    wptr<Window> weakToken(windowToken_);
    napi_value result = nullptr;
    std::shared_ptr<NapiAsyncTask> napiAsyncTask = CreateEmptyAsyncTask(env, callBack, &result);
    auto asyncTask = [weakToken, contentStorage, contextUrl, errCode, isLoadedByName, env, task = napiAsyncTask] {
        auto weakWindow = weakToken.promote();
        if (weakWindow == nullptr) {
            TLOGNE(WmsLogTag::WMS_LIFE, "window is nullptr");
            task->Reject(env, JsErrUtils::CreateJsError(env, WM_JS_TO_ERROR_CODE_MAP.at(WMError::WM_ERROR_NULLPTR),
                "[window][loadContent]msg: Window is nullptr."));
            return;
        }
        if (errCode != WMError::WM_OK) {
            task->Reject(env, JsErrUtils::CreateJsError(env, errCode));
            TLOGNE(WmsLogTag::WMS_LIFE, "Window is nullptr or get invalid param");
            return;
        }
        LoadContentTask(contentStorage, contextUrl, weakWindow, env, *task, isLoadedByName);
    };
    if (napi_send_event(env, asyncTask, napi_eprio_high, "LoadContentScheduleOld") != napi_status::napi_ok) {
        TLOGE(WmsLogTag::WMS_LIFE, "napi send event failed, window state is abnormal");
    }
    return result;
}

napi_value JsWindow::LoadContentScheduleNew(napi_env env, napi_callback_info info, bool isLoadedByName)
{
    WmErrorCode errCode = WmErrorCode::WM_OK;
    size_t argc = 4;
    napi_value argv[4] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc < 2) { // 2 param num
        TLOGE(WmsLogTag::WMS_LIFE, "Argc is invalid: %{public}zu", argc);
        errCode = WmErrorCode::WM_ERROR_INVALID_PARAM;
    }
    std::string contextUrl;
    if (errCode == WmErrorCode::WM_OK && !ConvertFromJsValue(env, argv[0], contextUrl)) {
        TLOGE(WmsLogTag::WMS_LIFE, "Failed to convert parameter to context url");
        errCode = WmErrorCode::WM_ERROR_INVALID_PARAM;
    }
    napi_value storage = nullptr;
    napi_value callBack = nullptr;
    if (argc == 2) { // 2: num of params
        storage = argv[1];
    } else if (argc >= 3) { // 3: num of params
        storage = argv[1];
        callBack = ((argv[2] != nullptr && GetType(env, argv[2]) == napi_function) ? // 2 param num
            argv[2] : nullptr); // 2 param num
    }
    if (errCode == WmErrorCode::WM_ERROR_INVALID_PARAM) {
        TLOGE(WmsLogTag::WMS_LIFE, "Window is nullptr or get invalid param");
        return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM);
    }
    std::shared_ptr<NativeReference> contentStorage = nullptr;
    if (storage != nullptr) {
        napi_ref result = nullptr;
        napi_create_reference(env, storage, 1, &result);
        contentStorage = std::shared_ptr<NativeReference>(reinterpret_cast<NativeReference*>(result));
    }
    wptr<Window> weakToken(windowToken_);
    napi_value result = nullptr;
    std::shared_ptr<NapiAsyncTask> napiAsyncTask = CreateEmptyAsyncTask(env, callBack, &result);
    auto asyncTask = [weakToken, contentStorage, contextUrl, isLoadedByName, env, task = napiAsyncTask] {
        auto weakWindow = weakToken.promote();
        if (weakWindow == nullptr) {
            TLOGNE(WmsLogTag::WMS_LIFE, "Window is nullptr or get invalid param");
            task->Reject(env, JsErrUtils::CreateJsError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
                "[window][loadContent]msg: Weak window is nullptr."));
            return;
        }
        LoadContentTask(contentStorage, contextUrl, weakWindow, env, *task, isLoadedByName);
    };
    if (napi_send_event(env, asyncTask, napi_eprio_high, "LoadContentScheduleNew") != napi_status::napi_ok) {
        TLOGE(WmsLogTag::WMS_LIFE, "napi send event failed, window state is abnormal");
    }
    return result;
}

napi_value JsWindow::OnLoadContent(napi_env env, napi_callback_info info, bool isLoadedByName)
{
    bool oldApi = false;
    size_t argc = 4;
    napi_value argv[4] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc == 1) {
        oldApi = true;
    } else if (argc == 2) { // 2 param num
        napi_value value = argv[1];
        if (value== nullptr || GetType(env, value) != napi_function) {
            oldApi = false;
        } else {
            oldApi = true;
        }
    }
    if (oldApi) {
        return LoadContentScheduleOld(env, info, isLoadedByName);
    } else {
        return LoadContentScheduleNew(env, info, isLoadedByName);
    }
}

napi_value JsWindow::OnGetUIContext(napi_env env, napi_callback_info info)
{
    size_t argc = 4;
    napi_value argv[4] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc >= 1) {
        WLOGFE("Argc is invalid: %{public}zu, expect zero params", argc);
        return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM);
    }

    if (windowToken_ == nullptr) {
        WLOGFE("window is nullptr");
        return NapiThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
            "[window][getUIContext]msg: Window is nullptr.");
    }

    auto uicontent = windowToken_->GetUIContent();
    if (uicontent == nullptr) {
        WLOGFW("uicontent is nullptr");
        return NapiThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
            "[window][getUIContext]msg: Uicontent is nullptr.");
    }

    napi_value uiContext = uicontent->GetUINapiContext();
    if (uiContext == nullptr) {
        WLOGFE("uiContext obtained from jsEngine is nullptr");
        return NapiThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
            "[window][getUIContext]msg: UiContext obtained from jsEngine is nullptr.");
    } else {
        return uiContext;
    }
}

napi_value JsWindow::OnSetUIContent(napi_env env, napi_callback_info info)
{
    WmErrorCode errCode = WmErrorCode::WM_OK;
    size_t argc = 4;
    napi_value argv[4] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc < 1) { // 2 maximum param num
        TLOGE(WmsLogTag::WMS_LIFE, "Argc is invalid: %{public}zu", argc);
        errCode = WmErrorCode::WM_ERROR_INVALID_PARAM;
    }
    std::string contextUrl;
    if (errCode == WmErrorCode::WM_OK && !ConvertFromJsValue(env, argv[0], contextUrl)) {
        TLOGE(WmsLogTag::WMS_LIFE, "Failed to convert parameter to context url");
        errCode = WmErrorCode::WM_ERROR_INVALID_PARAM;
    }
    napi_value callBack = nullptr;
    if (argc >= 2) { // 2 param num
        callBack = argv[1];
    }
    std::shared_ptr<NativeReference> contentStorage = nullptr;
    if (errCode == WmErrorCode::WM_ERROR_INVALID_PARAM) {
        return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM);
    }

    wptr<Window> weakToken(windowToken_);
    napi_value result = nullptr;
    std::shared_ptr<NapiAsyncTask> napiAsyncTask = CreateEmptyAsyncTask(env, callBack, &result);
    auto asyncTask = [weakToken, contentStorage, contextUrl, env, task = napiAsyncTask] {
        auto weakWindow = weakToken.promote();
        if (weakWindow == nullptr) {
            TLOGNE(WmsLogTag::WMS_LIFE, "Window is nullptr");
            task->Reject(env, JsErrUtils::CreateJsError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
                "[window][setUIContent]msg: Window is nullptr."));
            return;
        }
        LoadContentTask(contentStorage, contextUrl, weakWindow, env, *task, false);
    };
    if (napi_send_event(env, asyncTask, napi_eprio_high, "OnSetUIContent") != napi_status::napi_ok) {
        TLOGE(WmsLogTag::WMS_LIFE, "napi send event failed, window state is abnormal");
    }
    return result;
}

napi_value JsWindow::OnSetFullScreen(napi_env env, napi_callback_info info)
{
    WMError errCode = WMError::WM_OK;
    size_t argc = 4;
    napi_value argv[4] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc < 1 || argc > 2) { // 2: maximum params num
        TLOGE(WmsLogTag::WMS_IMMS, "Argc is invalid %{public}zu", argc);
        errCode = WMError::WM_ERROR_INVALID_PARAM;
    }
    bool isFullScreen = false;
    if (errCode == WMError::WM_OK) {
        napi_value nativeVal = argv[0];
        if (nativeVal == nullptr) {
            TLOGE(WmsLogTag::WMS_IMMS, "Failed to convert parameter to isFullScreen");
            errCode = WMError::WM_ERROR_INVALID_PARAM;
        } else {
            CHECK_NAPI_RETCODE(errCode, WMError::WM_ERROR_INVALID_PARAM,
                napi_get_value_bool(env, nativeVal, &isFullScreen));
        }
    }

    const char* const where = __func__;

    napi_value lastParam = (argc <= 1) ? nullptr : (GetType(env, argv[1]) == napi_function ? argv[1] : nullptr);
    napi_value result = nullptr;
    std::shared_ptr<NapiAsyncTask> napiAsyncTask = CreateEmptyAsyncTask(env, lastParam, &result);
    auto asyncTask = [weakToken = wptr<Window>(windowToken_), isFullScreen, errCode, where,
                      env, task = napiAsyncTask] {
        auto weakWindow = weakToken.promote();
        if (weakWindow == nullptr) {
            TLOGNE(WmsLogTag::WMS_IMMS, "window is nullptr");
            task->Reject(env, JsErrUtils::CreateJsError(env, WMError::WM_ERROR_NULLPTR));
            return;
        }
        if (errCode != WMError::WM_OK) {
            task->Reject(env, JsErrUtils::CreateJsError(env, errCode, "Invalidate params."));
            return;
        }
        WMError ret = weakWindow->SetFullScreen(isFullScreen);
        if (ret == WMError::WM_OK) {
            task->Resolve(env, NapiGetUndefined(env));
        } else {
            TLOGNE(WmsLogTag::WMS_IMMS, "%{public}s failed, ret %{public}d", where, ret);
            task->Reject(env, JsErrUtils::CreateJsError(env, ret, "Window SetFullScreen failed."));
        }
    };
    if (napi_send_event(env, asyncTask, napi_eprio_high, "OnSetFullScreen") != napi_status::napi_ok) {
        TLOGE(WmsLogTag::WMS_IMMS, "napi_send_event failed");
        napiAsyncTask->Reject(env,
            JsErrUtils::CreateJsError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY, "failed to send event"));
    }
    return result;
}

napi_value JsWindow::OnSetLayoutFullScreen(napi_env env, napi_callback_info info)
{
    WMError errCode = WMError::WM_OK;
    size_t argc = 4;
    napi_value argv[4] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc < 1 || argc > 2) { // 2: maximum params num
        TLOGE(WmsLogTag::WMS_IMMS, "Argc is invalid %{public}zu", argc);
        errCode = WMError::WM_ERROR_INVALID_PARAM;
    }
    bool isLayoutFullScreen = false;
    if (errCode == WMError::WM_OK) {
        napi_value nativeVal = argv[0];
        if (nativeVal == nullptr) {
            TLOGE(WmsLogTag::WMS_IMMS, "Failed to convert parameter to isLayoutFullScreen");
            errCode = WMError::WM_ERROR_INVALID_PARAM;
        } else {
            CHECK_NAPI_RETCODE(errCode, WMError::WM_ERROR_INVALID_PARAM,
                napi_get_value_bool(env, nativeVal, &isLayoutFullScreen));
        }
    }
    const char* const where = __func__;
    napi_value lastParam = (argc <= 1) ? nullptr : (GetType(env, argv[1]) == napi_function ? argv[1] : nullptr);
    napi_value result = nullptr;
    std::shared_ptr<NapiAsyncTask> napiAsyncTask = CreateEmptyAsyncTask(env, lastParam, &result);
    auto asyncTask = [weakToken = wptr<Window>(windowToken_), isLayoutFullScreen, errCode, where,
                      env, task = napiAsyncTask] {
        auto weakWindow = weakToken.promote();
        if (weakWindow == nullptr) {
            TLOGNE(WmsLogTag::WMS_IMMS, "window is nullptr");
            task->Reject(env, JsErrUtils::CreateJsError(env, WMError::WM_ERROR_NULLPTR));
            return;
        }
        if (errCode != WMError::WM_OK) {
            task->Reject(env, JsErrUtils::CreateJsError(env, errCode, "Invalidate params."));
            return;
        }
        WMError ret = weakWindow->SetLayoutFullScreen(isLayoutFullScreen);
        if (ret == WMError::WM_OK) {
            task->Resolve(env, NapiGetUndefined(env));
        } else {
            TLOGNE(WmsLogTag::WMS_IMMS, "%{public}s failed, ret %{public}d", where, ret);
            task->Reject(env, JsErrUtils::CreateJsError(env,
                ret, "Window OnSetLayoutFullScreen failed."));
        }
    };
    if (napi_send_event(env, asyncTask, napi_eprio_high, "OnSetLayoutFullScreen") != napi_status::napi_ok) {
        TLOGE(WmsLogTag::WMS_IMMS, "napi_send_event failed");
        napiAsyncTask->Reject(env,
            JsErrUtils::CreateJsError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY, "failed to send event"));
    }
    return result;
}

napi_value JsWindow::OnSetTitleAndDockHoverShown(napi_env env, napi_callback_info info)
{
    size_t argc = FOUR_PARAMS_SIZE;
    napi_value argv[FOUR_PARAMS_SIZE] = { nullptr };
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc > 2) { // 2: maximum params num
        TLOGE(WmsLogTag::WMS_LAYOUT_PC, "Argc is invalid: %{public}zu", argc);
        return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM,
            "[window][setTitleAndDockHoverShown]msg: Mandatory parameters are left unspecified");
    }
    bool isTitleHoverShown = true;
    if (argc > 0 && !ConvertFromJsValue(env, argv[INDEX_ZERO], isTitleHoverShown)) {
        TLOGE(WmsLogTag::WMS_LAYOUT_PC, "Failed to convert isTitleHoverShown parameter");
    }
    bool isDockHoverShown = true;
    if (argc > 1 && !ConvertFromJsValue(env, argv[INDEX_ONE], isDockHoverShown)) {
        TLOGE(WmsLogTag::WMS_LAYOUT_PC, "Failed to convert isDockHoverShown parameter");
    }
    napi_value result = nullptr;
    std::shared_ptr<NapiAsyncTask> napiAsyncTask = CreateEmptyAsyncTask(env, nullptr, &result);
    const char* const where = __func__;
    auto asyncTask = [weakToken = wptr<Window>(windowToken_), isTitleHoverShown,
        isDockHoverShown, env, task = napiAsyncTask, where] {
        auto window = weakToken.promote();
        if (window == nullptr) {
            TLOGNE(WmsLogTag::WMS_LAYOUT_PC, "%{public}s window is nullptr", where);
            task->Reject(env, JsErrUtils::CreateJsError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
                "[window][setTitleAndDockHoverShown]msg: The window is not created or destroyed"));
            return;
        }
        WMError errCode = window->SetTitleAndDockHoverShown(isTitleHoverShown, isDockHoverShown);
        WmErrorCode ret = WM_JS_TO_ERROR_CODE_MAP.at(errCode);
        if (ret != WmErrorCode::WM_OK) {
            TLOGNE(WmsLogTag::WMS_LAYOUT_PC, "%{public}s set title and dock hover show failed, "
                "ret:%{public}d", where, ret);
            task->Reject(env, JsErrUtils::CreateJsError(env, ret, "[window][setTitleAndDockHoverShown]"));
            return;
        }
        task->Resolve(env, NapiGetUndefined(env));
        TLOGNI(WmsLogTag::WMS_LAYOUT_PC, "%{public}s window [%{public}u, %{public}s] [%{public}d, %{public}d]",
            where, window->GetWindowId(), window->GetWindowName().c_str(),
            isTitleHoverShown, isDockHoverShown);
    };
    if (napi_send_event(env, asyncTask, napi_eprio_high, "OnSetTitleAndDockHoverShown") != napi_status::napi_ok) {
        napiAsyncTask->Reject(env, CreateJsError(env,
            static_cast<int32_t>(WmErrorCode::WM_ERROR_STATE_ABNORMALLY),
            "[window][setTitleAndDockHoverShown]msg: Internal task error"));
    }
    return result;
}

napi_value JsWindow::OnSetWindowLayoutFullScreen(napi_env env, napi_callback_info info)
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
        [weakToken = wptr<Window>(windowToken_), isLayoutFullScreen, where](napi_env env,
            NapiAsyncTask& task, int32_t status) {
            auto window = weakToken.promote();
            if (window == nullptr) {
                TLOGNE(WmsLogTag::WMS_IMMS, "%{public}s window is nullptr", where);
                task.Reject(env, JsErrUtils::CreateJsError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
                    "[window][setWindowLayoutFullScreen]msg: The window is not created or destroyed"));
                return;
            }
            // compatibleMode app adapt to immersive need apply avoidArea method
            if (window->IsPcOrPadFreeMultiWindowMode() && !window->IsAdaptToCompatibleImmersive()) {
                TLOGNE(WmsLogTag::WMS_IMMS, "%{public}s device not support", where);
                window->SetIgnoreSafeArea(isLayoutFullScreen);
                task.Resolve(env, NapiGetUndefined(env));
                return;
            }
            WmErrorCode ret = WM_JS_TO_ERROR_CODE_MAP.at(window->SetLayoutFullScreen(isLayoutFullScreen));
            if (ret == WmErrorCode::WM_OK) {
                task.Resolve(env, NapiGetUndefined(env));
            } else {
                TLOGNE(WmsLogTag::WMS_IMMS, "%{public}s failed, ret %{public}d", where, ret);
                task.Reject(env, JsErrUtils::CreateJsError(env, ret, "[window][setWindowLayoutFullScreen]"));
            }
        };

    napi_value lastParam = (argc <= 1) ? nullptr :
        ((argv[1] != nullptr && GetType(env, argv[1]) == napi_function) ? argv[1] : nullptr);
    napi_value result = nullptr;
    auto asyncTask = CreateAsyncTask(env, lastParam, nullptr,
        std::make_unique<NapiAsyncTask::CompleteCallback>(std::move(complete)), &result);
    NapiAsyncTask::Schedule("JsWindow::OnSetWindowLayoutFullScreen", env, std::move(asyncTask));
    return result;
}

napi_value JsWindow::OnSetSystemBarEnable(napi_env env, napi_callback_info info)
{
    size_t argc = FOUR_PARAMS_SIZE;
    napi_value argv[FOUR_PARAMS_SIZE] = { nullptr };
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    napi_value lastParam = nullptr;
    if (argc > ARG_COUNT_ZERO && argv[INDEX_ZERO] != nullptr && GetType(env, argv[INDEX_ZERO]) == napi_function) {
        lastParam = argv[INDEX_ZERO];
    } else if (argc > ARG_COUNT_ONE && argv[INDEX_ONE] != nullptr && GetType(env, argv[INDEX_ONE]) == napi_function) {
        lastParam = argv[INDEX_ONE];
    }
    napi_value result = nullptr;
    std::shared_ptr<NapiAsyncTask> napiAsyncTask = CreateEmptyAsyncTask(env, lastParam, &result);
    std::unordered_map<WindowType, SystemBarProperty> systemBarProperties;
    std::unordered_map<WindowType, SystemBarPropertyFlag> systemBarPropertyFlags;
    if (argc > ARG_COUNT_TWO || !GetSystemBarStatus(env, info, systemBarProperties, systemBarPropertyFlags)) {
        TLOGE(WmsLogTag::WMS_IMMS, "failed to convert parameter to systemBarProperties");
        napiAsyncTask->Reject(env, JsErrUtils::CreateJsError(env, WMError::WM_ERROR_INVALID_PARAM,
            "JsWindow::OnSetSystemBarEnable failed"));
        return result;
    }
    auto asyncTask = [weakToken = wptr<Window>(windowToken_), env, task = napiAsyncTask,
        systemBarProperties = std::move(systemBarProperties),
        systemBarPropertyFlags = std::move(systemBarPropertyFlags)] {
        auto window = weakToken.promote();
        if (window == nullptr) {
            TLOGNE(WmsLogTag::WMS_IMMS, "window is nullptr");
            task->Reject(env, JsErrUtils::CreateJsError(env, WMError::WM_ERROR_NULLPTR));
            return;
        }
        auto errCode = window->UpdateSystemBarProperties(systemBarProperties, systemBarPropertyFlags);
        if (errCode == WMError::WM_OK) {
            task->Resolve(env, NapiGetUndefined(env));
        } else {
            TLOGNE(WmsLogTag::WMS_IMMS, "set system bar enable failed, errcode: %{public}d", errCode);
            task->Reject(env, JsErrUtils::CreateJsError(env, errCode, "JsWindow::OnSetSystemBarEnable failed"));
        }
    };
    if (napi_send_event(env, asyncTask, napi_eprio_high, "OnSetSystemBarEnable") != napi_status::napi_ok) {
        TLOGE(WmsLogTag::WMS_IMMS, "napi_send_event failed");
    }
    return result;
}

napi_value JsWindow::OnSetWindowSystemBarEnable(napi_env env, napi_callback_info info)
{
    size_t argc = FOUR_PARAMS_SIZE;
    napi_value argv[FOUR_PARAMS_SIZE] = { nullptr };
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    std::unordered_map<WindowType, SystemBarProperty> systemBarProperties;
    std::unordered_map<WindowType, SystemBarPropertyFlag> systemBarPropertyFlags;
    if (argc < ARG_COUNT_ONE) {
        TLOGE(WmsLogTag::WMS_IMMS, "invalid argc");
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
    auto asyncTask = [weakToken = wptr<Window>(windowToken_), env, task = napiAsyncTask,
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

napi_value JsWindow::OnSetSpecificSystemBarEnabled(napi_env env, napi_callback_info info)
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
    auto systemBarType = name.compare("status") == 0 ? WindowType::WINDOW_TYPE_STATUS_BAR :
                    (name.compare("navigation") == 0 ? WindowType::WINDOW_TYPE_NAVIGATION_BAR :
                                                       WindowType::WINDOW_TYPE_NAVIGATION_INDICATOR);
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
    auto asyncTask = [weakToken = wptr<Window>(windowToken_), env, task = napiAsyncTask,
        systemBarType, systemBarEnable, systemBarEnableAnimation, argc, where] {
        auto window = weakToken.promote();
        if (window == nullptr) {
            TLOGNE(WmsLogTag::WMS_IMMS, "%{public}s window is nullptr", where);
            task->Reject(env, JsErrUtils::CreateJsError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
                "[window][setSpecificSystemBarEnabled]msg: The window is not created or destroyed"));
            return;
        }
        auto property = window->GetSystemBarPropertyByType(systemBarType);
        auto isSetAnimation = argc > ARG_COUNT_TWO;
        window->UpdateSpecificSystemBarEnabled(systemBarEnable,
            isSetAnimation ? systemBarEnableAnimation : property.enableAnimation_, property);
        SystemBarPropertyFlag propertyFlag = { true, false, false, isSetAnimation };
        auto errCode =
            WM_JS_TO_ERROR_CODE_MAP.at(window->UpdateSystemBarPropertyForPage(systemBarType, property, propertyFlag));
        if (errCode == WmErrorCode::WM_OK) {
            window->NotifySystemBarPropertyUpdate(systemBarType, property);
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

napi_value JsWindow::OnSetSystemBarProperties(napi_env env, napi_callback_info info)
{
    size_t argc = FOUR_PARAMS_SIZE;
    napi_value argv[FOUR_PARAMS_SIZE] = { nullptr };
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    napi_value lastParam = (argc <= ARG_COUNT_ONE) ? nullptr :
        (GetType(env, argv[INDEX_ONE]) == napi_function ? argv[INDEX_ONE] : nullptr);
    napi_value result = nullptr;
    std::shared_ptr<NapiAsyncTask> napiAsyncTask = CreateEmptyAsyncTask(env, lastParam, &result);
    std::unordered_map<WindowType, SystemBarProperty> systemBarProperties;
    std::unordered_map<WindowType, SystemBarPropertyFlag> systemBarPropertyFlags;
    if (argc < ARG_COUNT_ONE || argc > ARG_COUNT_TWO || argv[INDEX_ZERO] == nullptr ||
        !GetSystemBarPropertiesFromJs(env, argv[INDEX_ZERO], systemBarProperties, systemBarPropertyFlags)) {
        TLOGE(WmsLogTag::WMS_IMMS, "Failed to convert parameter to systemBarProperties");
        napiAsyncTask->Reject(env, JsErrUtils::CreateJsError(env, WMError::WM_ERROR_INVALID_PARAM));
        return result;
    }
    auto asyncTask = [weakToken = wptr<Window>(windowToken_), env, task = napiAsyncTask,
        systemBarProperties = std::move(systemBarProperties),
        systemBarPropertyFlags = std::move(systemBarPropertyFlags)] {
        auto window = weakToken.promote();
        if (window == nullptr) {
            TLOGNE(WmsLogTag::WMS_IMMS, "window is nullptr");
            task->Reject(env, JsErrUtils::CreateJsError(env, WMError::WM_ERROR_NULLPTR));
            return;
        }
        auto errCode = window->UpdateSystemBarProperties(systemBarProperties, systemBarPropertyFlags);
        if (errCode == WMError::WM_OK) {
            task->Resolve(env, NapiGetUndefined(env));
        } else {
            TLOGNE(WmsLogTag::WMS_IMMS, "set system bar properties failed, errcode: %{public}d", errCode);
            task->Reject(env, JsErrUtils::CreateJsError(env, errCode, "JsWindow::OnSetSystemBarProperties failed"));
        }
    };
    if (napi_send_event(env, asyncTask, napi_eprio_high, "OnSetSystemBarProperties") != napi_status::napi_ok) {
        TLOGE(WmsLogTag::WMS_IMMS, "napi_send_event failed");
    }
    return result;
}

napi_value JsWindow::OnSetWindowSystemBarProperties(napi_env env, napi_callback_info info)
{
    size_t argc = FOUR_PARAMS_SIZE;
    napi_value argv[FOUR_PARAMS_SIZE] = { nullptr };
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    napi_value lastParam = (argc <= ARG_COUNT_ONE) ? nullptr :
        (GetType(env, argv[INDEX_ONE]) == napi_function ? argv[INDEX_ONE] : nullptr);
    napi_value result = nullptr;
    std::shared_ptr<NapiAsyncTask> napiAsyncTask = CreateEmptyAsyncTask(env, lastParam, &result);
    std::unordered_map<WindowType, SystemBarProperty> systemBarProperties;
    std::unordered_map<WindowType, SystemBarPropertyFlag> systemBarPropertyFlags;
    if (argc < ARG_COUNT_ONE) {
        TLOGE(WmsLogTag::WMS_IMMS, "argc is invalid");
        return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM,
            "[window][setWindowSystemBarProperties]msg: Mandatory parameters are left unspecified");
    }
    if (argv[INDEX_ZERO] == nullptr || !GetSystemBarPropertiesFromJs(env,
        argv[INDEX_ZERO], systemBarProperties, systemBarPropertyFlags)) {
        TLOGE(WmsLogTag::WMS_IMMS, "failed to convert parameter");
        return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM,
            "[window][setWindowSystemBarProperties]msg: Incorrect parameter types");
    }
    auto asyncTask = [weakToken = wptr<Window>(windowToken_), env, task = napiAsyncTask,
        systemBarProperties = std::move(systemBarProperties),
        systemBarPropertyFlags = std::move(systemBarPropertyFlags)] {
        auto window = weakToken.promote();
        if (window == nullptr) {
            TLOGNE(WmsLogTag::WMS_IMMS, "window is nullptr");
            task->Reject(env, JsErrUtils::CreateJsError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
                "[window][setWindowSystemBarProperties]msg: The window is not created or destroyed"));
            return;
        }
        auto errCode = WM_JS_TO_ERROR_CODE_MAP.at(
            window->UpdateSystemBarProperties(systemBarProperties, systemBarPropertyFlags));
        if (errCode == WmErrorCode::WM_OK) {
            task->Resolve(env, NapiGetUndefined(env));
        } else {
            TLOGNE(WmsLogTag::WMS_IMMS, "set system bar properties failed, errcode: %{public}d", errCode);
            task->Reject(env, JsErrUtils::CreateJsError(env, errCode,
                "[window][setWindowSystemBarProperties]"));
        }
    };
    if (napi_send_event(env, asyncTask, napi_eprio_high, "OnSetWindowSystemBarProperties") != napi_status::napi_ok) {
        TLOGE(WmsLogTag::WMS_IMMS, "napi_send_event failed");
        napiAsyncTask->Reject(env, JsErrUtils::CreateJsError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
            "[window][setWindowSystemBarProperties]msg: Internal task error"));
    }
    return result;
}

napi_value JsWindow::OnGetWindowSystemBarPropertiesSync(napi_env env, napi_callback_info info)
{
    if (windowToken_ == nullptr) {
        TLOGE(WmsLogTag::WMS_IMMS, "window is null");
        return NapiThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
            "[window][getWindowSystemBarProperties]msg: The window is not created or destroyed");
    }
    if (!WindowHelper::IsMainWindow(windowToken_->GetType())) {
        TLOGE(WmsLogTag::WMS_IMMS, "only main window is allowed");
        return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_CALLING,
            "[window][getWindowSystemBarProperties]msg: Invalid window type. Only main windows are supported");
    }
    auto objValue = CreateJsSystemBarPropertiesObject(env, windowToken_);
    if (objValue == nullptr) {
        TLOGE(WmsLogTag::WMS_IMMS, "get properties failed");
        return NapiThrowError(env, WmErrorCode::WM_ERROR_SYSTEM_ABNORMALLY,
            "[window][getWindowSystemBarProperties]msg: Create js object failed");
    }
    return objValue;
}

napi_value JsWindow::OnSetStatusBarColor(napi_env env, napi_callback_info info)
{
    size_t argc = FOUR_PARAMS_SIZE;
    napi_value argv[FOUR_PARAMS_SIZE] = { nullptr };
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc < ARG_COUNT_ONE || argv[INDEX_ZERO] == nullptr) {
        TLOGE(WmsLogTag::WMS_IMMS, "no enough arguments");
        return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM,
            "[window][setStatusBarColor]msg: Invalid parameters");
    }
    uint32_t contentColor = 0;
    if (!ParseColorMetrics(env, argv[INDEX_ZERO], contentColor)) {
        TLOGE(WmsLogTag::WMS_IMMS, "parse color failed");
        return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM,
            "[window][setStatusBarColor]msg: parse color failed");
    }
    napi_value result = nullptr;
    napi_value lastParam = (argc <= ARG_COUNT_ONE) ? nullptr :
        (GetType(env, argv[INDEX_ONE]) == napi_function ? argv[INDEX_ONE] : nullptr);
    std::shared_ptr<NapiAsyncTask> napiAsyncTask = CreateEmptyAsyncTask(env, lastParam, &result);
    TLOGI(WmsLogTag::WMS_IMMS, "target color: %{public}u", contentColor);
    auto asyncTask = [weakToken = wptr<Window>(windowToken_), env, contentColor, task = napiAsyncTask] {
        auto window = weakToken.promote();
        if (window == nullptr) {
            TLOGNE(WmsLogTag::WMS_IMMS, "window is null");
            task->Reject(env, JsErrUtils::CreateJsError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
                "[window][setStatusBarColor]msg: The window is not created or destroyed"));
            return;
        }
        auto errCode = UpdateStatusBarProperty(window, contentColor);
        if (errCode == WMError::WM_OK) {
            task->Resolve(env, NapiGetUndefined(env));
        } else {
            TLOGNE(WmsLogTag::WMS_IMMS, "set status bar property error: %{public}d", errCode);
            task->Reject(env, JsErrUtils::CreateJsError(env, WM_JS_TO_ERROR_CODE_MAP.at(errCode),
                "[window][setStatusBarColor]"));
        }
    };
    if (napi_send_event(env, asyncTask, napi_eprio_high, "OnSetStatusBarColor") != napi_status::napi_ok) {
        TLOGE(WmsLogTag::WMS_IMMS, "napi_send_event failed");
        napiAsyncTask->Reject(env, JsErrUtils::CreateJsError(env, WmErrorCode::WM_ERROR_SYSTEM_ABNORMALLY,
            "[window][setStatusBarColor]msg: Internal task error"));
    }
    return result;
}

napi_value JsWindow::OnGetStatusBarPropertySync(napi_env env, napi_callback_info info)
{
    if (windowToken_ == nullptr) {
        TLOGE(WmsLogTag::WMS_IMMS, "window is null");
        return NapiThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
            "[window][getStatusBarProperty]msg: The window is not created or destroyed");
    }
    if (!WindowHelper::IsMainWindow(windowToken_->GetType())) {
        TLOGE(WmsLogTag::WMS_IMMS, "only main window is allowed");
        return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_CALLING,
            "[window][getStatusBarProperty]msg: Invalid window type. Only main windows are supported");
    }
    auto objValue = GetStatusBarPropertyObject(env, windowToken_);
    if (objValue == nullptr) {
        TLOGE(WmsLogTag::WMS_IMMS, "get property failed");
        return NapiThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
            "[window][getStatusBarProperty]msg: Internal task error");
    }
    return objValue;
}

napi_value JsWindow::OnGetWindowStateSnapshot(napi_env env, napi_callback_info info)
{
    napi_value result = nullptr;
    std::shared_ptr<NapiAsyncTask> napiAsyncTask = CreateEmptyAsyncTask(env, nullptr, &result);
    const char* const where = __func__;
    auto asyncTask = [weakToken = wptr<Window>(windowToken_), env, task = napiAsyncTask, where] {
        auto weakWindow = weakToken.promote();
        if (weakWindow == nullptr) {
            TLOGNE(WmsLogTag::WMS_ATTRIBUTE, "%{public}s: window is nullptr", where);
            task->Reject(env, JsErrUtils::CreateJsError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
                "[window][getWindowStateSnapshot]msg: The window is not created or destroyed"));
            return;
        }
        std::string winStateSnapshot;
        WMError errCode = weakWindow->GetWindowStateSnapshot(winStateSnapshot);
        if (errCode != WMError::WM_OK) {
            TLOGNE(WmsLogTag::WMS_ATTRIBUTE, "%{public}s: get window state snapshot failed, errCode=%{public}d",
                where, static_cast<int32_t>(errCode));
            auto retErrCode = WmErrorCode::WM_ERROR_SYSTEM_ABNORMALLY;
            if (WM_JS_TO_ERROR_CODE_MAP.count(errCode) > 0) {
                retErrCode = WM_JS_TO_ERROR_CODE_MAP.at(errCode);
            }
            task->Reject(env, JsErrUtils::CreateJsError(env, retErrCode,
                "[window][getWindowStateSnapshot]msg: get window state snapshot failed"));
            return;
        }
        auto objValue = CreateJsValue(env, winStateSnapshot);
        if (objValue == nullptr) {
            TLOGNE(WmsLogTag::WMS_ATTRIBUTE, "%{public}s: create js object failed: winStateSnapshot=%{public}s",
                where, winStateSnapshot.c_str());
            task->Reject(env, JsErrUtils::CreateJsError(env, WmErrorCode::WM_ERROR_SYSTEM_ABNORMALLY,
                "[window][getWindowStateSnapshot]msg: this window manager service works abnormally"));
            return;
        }
        TLOGNI(WmsLogTag::WMS_ATTRIBUTE, "%{public}s: winStateSnapshot=%{public}s", where, winStateSnapshot.c_str());
        task->Resolve(env, objValue);
    };
    if (napi_send_event(env, asyncTask, napi_eprio_high, "OnGetWindowStateSnapshot") != napi_status::napi_ok) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "napi_send_event failed");
        napiAsyncTask->Reject(env, JsErrUtils::CreateJsError(env, WmErrorCode::WM_ERROR_SYSTEM_ABNORMALLY,
            "[window][getWindowStateSnapshot]msg: this window manager service works abnormally"));
    }
    return result;
}

napi_value JsWindow::OnEnableLandscapeMultiWindow(napi_env env, napi_callback_info info)
{
    TLOGI(WmsLogTag::WMS_MULTI_WINDOW, "In");
    WmErrorCode err = (windowToken_ == nullptr) ? WmErrorCode::WM_ERROR_STATE_ABNORMALLY : WmErrorCode::WM_OK;
    size_t argc = 4;
    napi_value argv[4] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    napi_value result = nullptr;
    std::shared_ptr<NapiAsyncTask> napiAsyncTask = CreateEmptyAsyncTask(env, nullptr, &result);
    auto asyncTask = [weakToken = wptr<Window>(windowToken_), env, task = napiAsyncTask, err]() mutable {
        auto weakWindow = weakToken.promote();
        err = (weakWindow == nullptr) ? WmErrorCode::WM_ERROR_STATE_ABNORMALLY : err;
        if (err != WmErrorCode::WM_OK) {
            task->Reject(env, JsErrUtils::CreateJsError(env, err));
            return;
        }
        WMError ret = weakWindow->SetLandscapeMultiWindow(true);
        if (ret == WMError::WM_OK) {
            task->Resolve(env, NapiGetUndefined(env));
        } else {
            task->Reject(env, JsErrUtils::CreateJsError(env, WmErrorCode::WM_ERROR_SYSTEM_ABNORMALLY,
                "JsWindow::OnEnableLandscapeMultiWindow failed"));
        }
    };
    if (napi_send_event(env, asyncTask, napi_eprio_high, "OnEnableLandscapeMultiWindow") != napi_status::napi_ok) {
        napiAsyncTask->Reject(
            env, JsErrUtils::CreateJsError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY, "failed to send event"));
    }
    return result;
}

napi_value JsWindow::OnDisableLandscapeMultiWindow(napi_env env, napi_callback_info info)
{
    TLOGI(WmsLogTag::WMS_MULTI_WINDOW, "In");
    WmErrorCode err = (windowToken_ == nullptr) ? WmErrorCode::WM_ERROR_STATE_ABNORMALLY : WmErrorCode::WM_OK;
    size_t argc = 4;
    napi_value argv[4] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);

    napi_value result = nullptr;
    std::shared_ptr<NapiAsyncTask> napiAsyncTask = CreateEmptyAsyncTask(env, nullptr, &result);
    auto asyncTask = [weakToken = wptr<Window>(windowToken_), env, task = napiAsyncTask, err]() mutable {
        auto weakWindow = weakToken.promote();
        err = (weakWindow == nullptr) ? WmErrorCode::WM_ERROR_STATE_ABNORMALLY : err;
        if (err != WmErrorCode::WM_OK) {
            task->Reject(env, JsErrUtils::CreateJsError(env, err));
            return;
        }
        WMError ret = weakWindow->SetLandscapeMultiWindow(false);
        if (ret == WMError::WM_OK) {
            task->Resolve(env, NapiGetUndefined(env));
        } else {
            task->Reject(env, JsErrUtils::CreateJsError(env, WmErrorCode::WM_ERROR_SYSTEM_ABNORMALLY,
                "JsWindow::OnDisableLandscapeMultiWindow failed"));
        }
    };
    if (napi_send_event(env, asyncTask, napi_eprio_high, "OnDisableLandscapeMultiWindow") != napi_status::napi_ok) {
        napiAsyncTask->Reject(
            env, JsErrUtils::CreateJsError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY, "failed to send event"));
    }
    return result;
}

static void ParseAvoidAreaParam(napi_env env, napi_callback_info info, WMError& errCode, AvoidAreaType& avoidAreaType)
{
    size_t argc = 4;
    napi_value argv[4] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc < 1 || argc > 2) { // 2: maximum params num
        TLOGE(WmsLogTag::WMS_IMMS, "Argc is invalid %{public}zu", argc);
        errCode = WMError::WM_ERROR_INVALID_PARAM;
    }
    if (errCode == WMError::WM_OK) {
        napi_value nativeMode = argv[0];
        if (nativeMode == nullptr) {
            TLOGE(WmsLogTag::WMS_IMMS, "Failed to convert parameter to AvoidAreaType");
            errCode = WMError::WM_ERROR_INVALID_PARAM;
        } else {
            uint32_t resultValue = 0;
            CHECK_NAPI_RETCODE(errCode, WMError::WM_ERROR_INVALID_PARAM,
                napi_get_value_uint32(env, nativeMode, &resultValue));
            avoidAreaType = static_cast<AvoidAreaType>(resultValue);
            errCode = avoidAreaType > AvoidAreaType::TYPE_KEYBOARD ?
                WMError::WM_ERROR_INVALID_PARAM : WMError::WM_OK;
        }
    }
}

napi_value JsWindow::OnGetAvoidArea(napi_env env, napi_callback_info info)
{
    WMError errCode = WMError::WM_OK;
    AvoidAreaType avoidAreaType = AvoidAreaType::TYPE_SYSTEM;
    ParseAvoidAreaParam(env, info, errCode, avoidAreaType);
    const char* const where = __func__;
    size_t argc = 4;
    napi_value argv[4] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    napi_value lastParam = (argc <= 1) ? nullptr :
        (GetType(env, argv[1]) == napi_function ? argv[1] : nullptr);
    napi_value result = nullptr;
    std::shared_ptr<NapiAsyncTask> napiAsyncTask = CreateEmptyAsyncTask(env, lastParam, &result);
    auto asyncTask = [weakToken = wptr<Window>(windowToken_), errCode, avoidAreaType, where,
                      env, task = napiAsyncTask] {
        auto weakWindow = weakToken.promote();
        if (weakWindow == nullptr) {
            TLOGNE(WmsLogTag::WMS_IMMS, "%{public}s window is nullptr", where);
            task->Reject(env, JsErrUtils::CreateJsError(env, WMError::WM_ERROR_NULLPTR));
            return;
        }
        if (errCode != WMError::WM_OK) {
            task->Reject(env, JsErrUtils::CreateJsError(env, errCode));
            TLOGNE(WmsLogTag::WMS_IMMS, "%{public}s window is nullptr or get invalid param", where);
            return;
        }
        // getAvoidRect by avoidAreaType
        AvoidArea avoidArea;
        WMError ret = weakWindow->GetAvoidAreaByType(avoidAreaType, avoidArea);
        if (ret != WMError::WM_OK) {
            TLOGNE(WmsLogTag::WMS_IMMS, "%{public}s failed, ret %{public}d", where, ret);
            avoidArea.topRect_ = g_emptyRect;
            avoidArea.leftRect_ = g_emptyRect;
            avoidArea.rightRect_ = g_emptyRect;
            avoidArea.bottomRect_ = g_emptyRect;
        }
        napi_value avoidAreaObj = ConvertAvoidAreaToJsValue(env, avoidArea, avoidAreaType);
        if (avoidAreaObj != nullptr) {
            task->Resolve(env, avoidAreaObj);
        } else {
            TLOGNE(WmsLogTag::WMS_IMMS, "ConvertAvoidAreaToJsValue failed");
            task->Reject(env, JsErrUtils::CreateJsError(env, WMError::WM_ERROR_NULLPTR,
                "JsWindow::OnGetAvoidArea failed"));
        }
    };
    if (napi_send_event(env, asyncTask, napi_eprio_high, "OnGetAvoidArea") != napi_status::napi_ok) {
        TLOGE(WmsLogTag::WMS_IMMS, "napi_send_event failed");
        napiAsyncTask->Reject(env,
            JsErrUtils::CreateJsError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY, "failed to send event"));
    }
    return result;
}

napi_value JsWindow::OnGetWindowAvoidAreaSync(napi_env env, napi_callback_info info)
{
    WmErrorCode errCode = WmErrorCode::WM_OK;
    size_t argc = 4;
    napi_value argv[4] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc < 1) { // 1: params num
        TLOGE(WmsLogTag::WMS_IMMS, "invalid argc %{public}zu", argc);
        return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM,
            "[window][getWindowAvoidArea]msg: Mandatory parameters are left unspecified");
    }
    AvoidAreaType avoidAreaType = AvoidAreaType::TYPE_SYSTEM;
    napi_value nativeMode = argv[0];
    if (nativeMode == nullptr) {
        TLOGE(WmsLogTag::WMS_IMMS, "invalid param");
        return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM,
            "[window][getWindowAvoidArea]msg: Incorrect parameter types");
    }
    uint32_t resultValue = 0;
    CHECK_NAPI_RETCODE(errCode, WmErrorCode::WM_ERROR_INVALID_PARAM,
        napi_get_value_uint32(env, nativeMode, &resultValue));
    avoidAreaType = static_cast<AvoidAreaType>(resultValue);
    errCode = avoidAreaType >= AvoidAreaType::TYPE_END ?
        WmErrorCode::WM_ERROR_INVALID_PARAM : WmErrorCode::WM_OK;
    if (errCode == WmErrorCode::WM_ERROR_INVALID_PARAM) {
        TLOGE(WmsLogTag::WMS_IMMS, "invalid param");
        return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM,
            "[window][getWindowAvoidArea]msg: Parameter verification failed");
    }

    if (windowToken_ == nullptr) {
        TLOGE(WmsLogTag::WMS_IMMS, "window is nullptr");
        return NapiThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
            "[window][getWindowAvoidArea]msg: The window is not created or destroyed");
    }
    // getAvoidRect by avoidAreaType
    AvoidArea avoidArea;
    WmErrorCode ret = WM_JS_TO_ERROR_CODE_MAP.at(windowToken_->GetAvoidAreaByType(avoidAreaType, avoidArea));
    if (ret != WmErrorCode::WM_OK) {
        TLOGE(WmsLogTag::WMS_IMMS, "GetWindowAvoidAreaSync failed, ret %{public}d", ret);
        avoidArea.topRect_ = g_emptyRect;
        avoidArea.leftRect_ = g_emptyRect;
        avoidArea.rightRect_ = g_emptyRect;
        avoidArea.bottomRect_ = g_emptyRect;
    }
    napi_value avoidAreaObj = ConvertAvoidAreaToJsValue(env, avoidArea, avoidAreaType);
    if (avoidAreaObj != nullptr) {
        return avoidAreaObj;
    } else {
        TLOGE(WmsLogTag::WMS_IMMS, "ConvertAvoidAreaToJsValue failed");
        return NapiThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
            "[window][getWindowAvoidArea]msg: Convert avoid area failed");
    }
}

napi_value JsWindow::OnGetWindowAvoidAreaIgnoringVisibilitySync(napi_env env, napi_callback_info info)
{
    WmErrorCode errCode = WmErrorCode::WM_OK;
    size_t argc = 4;
    napi_value argv[4] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc < 1) { // 1: params num
        TLOGE(WmsLogTag::WMS_IMMS, "invalid argc %{public}zu", argc);
        return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM,
            "[window][getWindowAvoidAreaIgnoringVisibility]msg: Mandatory parameters are left unspecified");
    }
    AvoidAreaType avoidAreaType = AvoidAreaType::TYPE_SYSTEM;
    napi_value nativeType = argv[0];
    if (nativeType == nullptr) {
        return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM,
            "[window][getWindowAvoidAreaIgnoringVisibility]msg: Incorrect parameter types");
    }
    uint32_t resultValue = 0;
    CHECK_NAPI_RETCODE(errCode, WmErrorCode::WM_ERROR_INVALID_PARAM,
        napi_get_value_uint32(env, nativeType, &resultValue));
    avoidAreaType = static_cast<AvoidAreaType>(resultValue);
    errCode = avoidAreaType >= AvoidAreaType::TYPE_END ?
        WmErrorCode::WM_ERROR_INVALID_PARAM : WmErrorCode::WM_OK;
    if (errCode == WmErrorCode::WM_ERROR_INVALID_PARAM) {
        TLOGE(WmsLogTag::WMS_IMMS, "invalid param");
        return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM,
            "[window][getWindowAvoidAreaIgnoringVisibility]msg: Parameter verification failed");
    }
    if (windowToken_ == nullptr) {
        TLOGE(WmsLogTag::WMS_IMMS, "window is nullptr");
        return NapiThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
            "[window][getWindowAvoidAreaIgnoringVisibility]msg: The window is not created or destroyed");
    }

    AvoidArea avoidArea;
    WmErrorCode ret = WM_JS_TO_ERROR_CODE_MAP.at(
        windowToken_->GetAvoidAreaByTypeIgnoringVisibility(avoidAreaType, avoidArea));
    if (ret != WmErrorCode::WM_OK) {
        TLOGE(WmsLogTag::WMS_IMMS, "failed, ret %{public}d", ret);
        return NapiThrowError(env, ret, "[window][getWindowAvoidAreaIgnoringVisibility]");
    }
    napi_value avoidAreaObj = ConvertAvoidAreaToJsValue(env, avoidArea, avoidAreaType, true);
    if (avoidAreaObj != nullptr) {
        return avoidAreaObj;
    } else {
        TLOGE(WmsLogTag::WMS_IMMS, "ConvertAvoidAreaToJsValue failed");
        return NapiThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
            "[window][getWindowAvoidAreaIgnoringVisibility]msg: Convert avoid area failed");
    }
}

napi_value JsWindow::OnIsShowing(napi_env env, napi_callback_info info)
{
    WMError errCode = WMError::WM_OK;
    size_t argc = 4;
    napi_value argv[4] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc > 1) {
        TLOGE(WmsLogTag::WMS_LIFE, "Argc is invalid: %{public}zu", argc);
        errCode = WMError::WM_ERROR_INVALID_PARAM;
    }
    wptr<Window> weakToken(windowToken_);
    const char* const where = __func__;
    napi_value lastParam = (argc == 0) ? nullptr : (GetType(env, argv[0]) == napi_function ? argv[0] : nullptr);
    napi_value result = nullptr;
    std::shared_ptr<NapiAsyncTask> napiAsyncTask = CreateEmptyAsyncTask(env, lastParam, &result);
    auto asyncTask = [weakToken, errCode, where, env, task = napiAsyncTask] {
        auto weakWindow = weakToken.promote();
        if (weakWindow == nullptr) {
            TLOGNE(WmsLogTag::WMS_LIFE, "window is nullptr");
            task->Reject(env, JsErrUtils::CreateJsError(env, WMError::WM_ERROR_NULLPTR));
            return;
        }
        if (errCode != WMError::WM_OK) {
            task->Reject(env, JsErrUtils::CreateJsError(env, errCode));
            TLOGNE(WmsLogTag::WMS_LIFE, "window is nullptr or get invalid param");
            return;
        }
        bool state = weakWindow->GetWindowState() == WindowState::STATE_SHOWN;
        task->Resolve(env, CreateJsValue(env, state));
        TLOGNI(WmsLogTag::WMS_LIFE, "%{public}s end, window [%{public}u, %{public}s] state=%{public}u",
            where, weakWindow->GetWindowId(), weakWindow->GetWindowName().c_str(), state);
    };
    if (napi_send_event(env, asyncTask, napi_eprio_high, "OnIsShowing") != napi_status::napi_ok) {
        TLOGE(WmsLogTag::WMS_LIFE, "napi send event failed, window state is abnormal");
    }
    return result;
}

napi_value JsWindow::OnIsWindowShowingSync(napi_env env, napi_callback_info info)
{
    if (windowToken_ == nullptr) {
        TLOGE(WmsLogTag::WMS_LIFE, "window is nullptr");
        return NapiThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
            "[window][isWindowShowing]msg: Window is nullptr.");
    }
    bool state = (windowToken_->GetWindowState() == WindowState::STATE_SHOWN);
    TLOGD(WmsLogTag::WMS_LIFE, "Id=%{public}u, state=%{public}u", windowToken_->GetWindowId(), state);
    return CreateJsValue(env, state);
}

napi_value JsWindow::OnSetPreferredOrientation(napi_env env, napi_callback_info info)
{
    WmErrorCode errCode = WmErrorCode::WM_OK;
    Orientation requestedOrientation = Orientation::UNSPECIFIED;
    std::string errMsg = "[window][setPreferredOrientation]msg: ";
    size_t argc = 4;
    napi_value argv[4] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc < 1) { // 1: params num
        TLOGE(WmsLogTag::WMS_ROTATION, "Argc is invalid: %{public}zu", argc);
        errCode = WmErrorCode::WM_ERROR_INVALID_PARAM;
        errMsg = "Mandatory parameters are left unspecified.";
    } else {
        if (argv[0] == nullptr) {
            errCode = WmErrorCode::WM_ERROR_INVALID_PARAM;
            errMsg += "Failed to convert parameter to Orientation.";
            TLOGE(WmsLogTag::WMS_ROTATION, "Failed to convert parameter to Orientation");
        } else {
            uint32_t resultValue = 0;
            if (errCode == WmErrorCode::WM_OK && !ConvertFromJsValue(env, argv[0], resultValue)) {
                TLOGE(WmsLogTag::WMS_ROTATION, "Failed to convert parameter to orientation");
                errCode = WmErrorCode::WM_ERROR_INVALID_PARAM;
                errMsg += "Failed to convert parameter to orientation.";
            }
            auto apiOrientation = static_cast<ApiOrientation>(resultValue);
            if (apiOrientation < ApiOrientation::BEGIN ||
                apiOrientation > ApiOrientation::END) {
                TLOGE(WmsLogTag::WMS_ROTATION, "Orientation %{public}u invalid!",
                    static_cast<uint32_t>(apiOrientation));
                errCode = WmErrorCode::WM_ERROR_INVALID_PARAM;
                errMsg += "Invalid parameter value range.";
            } else {
                requestedOrientation = JS_TO_NATIVE_ORIENTATION_MAP.at(apiOrientation);
            }
        }
    }
    if (errCode == WmErrorCode::WM_ERROR_INVALID_PARAM) {
        return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM, errMsg);
    }

    napi_value lastParam = (argc <= 1) ? nullptr :
        ((argv[1] != nullptr && GetType(env, argv[1]) == napi_function) ? argv[1] : nullptr);
    napi_value result = nullptr;
    std::shared_ptr<NapiAsyncTask> napiAsyncTask = CreateEmptyAsyncTask(env, lastParam, &result);
    auto asyncTask = [windowToken = wptr<Window>(windowToken_), errCode, requestedOrientation, env,
            task = napiAsyncTask, where = __func__] {
        if (errCode != WmErrorCode::WM_OK) {
            task->Reject(env, JsErrUtils::CreateJsError(env, errCode));
            TLOGNE(WmsLogTag::WMS_ROTATION, "%{public}s: invalid param", where);
            return;
        }
        auto weakWindow = windowToken.promote();
        if (weakWindow == nullptr) {
            task->Reject(env, JsErrUtils::CreateJsError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
                "[window][setPreferredOrientation]msg: Window is nullptr."));
            return;
        }
        if (requestedOrientation == Orientation::INVALID) {
            task->Reject(env, JsErrUtils::CreateJsError(env, WmErrorCode::WM_ERROR_INVALID_PARAM,
                    "[window][setPreferredOrientation]msg: Invalid parameter value range."));
            return;
        }
        weakWindow->SetRequestedOrientation(requestedOrientation);
        task->Resolve(env, NapiGetUndefined(env));
        TLOGNI(WmsLogTag::WMS_ROTATION, "%{public}s end, window [%{public}u, %{public}s] orientation=%{public}u",
            where, weakWindow->GetWindowId(), weakWindow->GetWindowName().c_str(),
            static_cast<uint32_t>(requestedOrientation));
    };
    if (napi_send_event(env, asyncTask, napi_eprio_high, "OnSetPreferredOrientation") != napi_status::napi_ok) {
        napiAsyncTask->Reject(env, JsErrUtils::CreateJsError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
            "[window][setPreferredOrientation]msg: Send event failed."));
    }
    return result;
}

napi_value JsWindow::OnGetPreferredOrientation(napi_env env, napi_callback_info info)
{
    size_t argc = 4;
    napi_value argv[4] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc >= 1) {
        TLOGE(WmsLogTag::WMS_ROTATION, "Argc is invalid: %{public}zu, expect zero params", argc);
        return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM,
            "[window][getPreferredOrientation]msg: Unexpected parameters.");
    }
    if (windowToken_ == nullptr) {
        TLOGE(WmsLogTag::WMS_ROTATION, "window is nullptr");
        napi_throw(env, JsErrUtils::CreateJsError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
            "[window][getPreferredOrientation]msg: Window is nullptr."));
        return NapiGetUndefined(env);
    }
    Orientation requestedOrientation = windowToken_->GetRequestedOrientation();
    ApiOrientation apiOrientation = ApiOrientation::UNSPECIFIED;
    if (requestedOrientation >= Orientation::BEGIN &&
        requestedOrientation <= Orientation::END) {
        apiOrientation = NATIVE_TO_JS_ORIENTATION_MAP.at(requestedOrientation);
    } else {
        TLOGE(WmsLogTag::WMS_ROTATION, "Orientation %{public}u invalid!", static_cast<uint32_t>(requestedOrientation));
    }
    TLOGD(WmsLogTag::WMS_ROTATION, "end, window [%{public}u, %{public}s] orientation=%{public}u",
        windowToken_->GetWindowId(), windowToken_->GetWindowName().c_str(), static_cast<uint32_t>(apiOrientation));
    return CreateJsValue(env, static_cast<uint32_t>(apiOrientation));
}

napi_value JsWindow::OnConvertOrientationAndRotation(napi_env env, napi_callback_info info)
{
    size_t argc = FOUR_PARAMS_SIZE;
    napi_value argv[FOUR_PARAMS_SIZE] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc != THREE_PARAMS_SIZE) {
        TLOGE(WmsLogTag::WMS_ROTATION, "Argc is invalid: %{public}zu, expect three params", argc);
        return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM,
            "[window][convertOrientationAndRotation]msg: invalid argc");
    }
    if (windowToken_ == nullptr) {
        TLOGE(WmsLogTag::WMS_ROTATION, "window is nullptr");
        return NapiThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
            "[window][convertOrientationAndRotation]msg: invalid window");
    }
    int32_t from = 0;
    if (!ConvertFromJsValue(env, argv[INDEX_ZERO], from)) {
        TLOGE(WmsLogTag::WMS_ROTATION, "Failed to convert parameter to RotationInfoType of from");
        return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM,
            "[window][convertOrientationAndRotation]msg: Failed to convert parameter to RotationInfoType of from");
    }
    RotationInfoType fromRotationInfoType = static_cast<RotationInfoType>(from);
    if (fromRotationInfoType < RotationInfoType::WINDOW_ORIENTATION ||
        fromRotationInfoType > RotationInfoType::DISPLAY_ROTATION) {
        TLOGE(WmsLogTag::WMS_ROTATION, "Invalid from RotationInfoType : %{public}d", from);
        return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM,
            "[window][convertOrientationAndRotation]msg: Invalid from RotationInfoType");
    }
    int32_t to = 0;
    if (!ConvertFromJsValue(env, argv[INDEX_ONE], to)) {
        TLOGE(WmsLogTag::WMS_ROTATION, "Failed to convert parameter to RotationInfoType of to");
        return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM,
            "[window][convertOrientationAndRotation]msg: Failed to convert parameter to RotationInfoType of to");
    }
    RotationInfoType toRotationInfoType = static_cast<RotationInfoType>(to);
    if (toRotationInfoType < RotationInfoType::WINDOW_ORIENTATION ||
        toRotationInfoType > RotationInfoType::DISPLAY_ROTATION) {
        TLOGE(WmsLogTag::WMS_ROTATION, "Invalid to RotationInfoType : %{public}d", to);
        return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM,
            "[window][convertOrientationAndRotation]msg: Invalid to RotationInfoType");
    }
    int32_t value = 0;
    if (!ConvertFromJsValue(env, argv[INDEX_TWO], value)) {
        TLOGE(WmsLogTag::WMS_ROTATION, "Failed to convert parameter to RotationInfoValue");
        return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM,
            "[window][convertOrientationAndRotation]msg: Failed to convert parameter to RotationInfoValue");
    }
    int32_t convertedValue = 0;
    WmErrorCode ret = WM_JS_TO_ERROR_CODE_MAP.at(
        windowToken_->ConvertOrientationAndRotation(fromRotationInfoType, toRotationInfoType, value, convertedValue));
    if (ret != WmErrorCode::WM_OK) {
        return NapiThrowError(env, ret,
            "[window][convertOrientationAndRotation]msg: Failed to convert Orientation adn Rotation");
    }
    TLOGD(WmsLogTag::WMS_ROTATION, "end, convertRotationValue : %{public}d", convertedValue);
    return CreateJsValue(env, convertedValue);
}

napi_value JsWindow::OnIsSupportWideGamut(napi_env env, napi_callback_info info)
{
    WMError errCode = WMError::WM_OK;
    size_t argc = 4;
    napi_value argv[4] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc > 1) {
        WLOGFE("Argc is invalid: %{public}zu", argc);
        errCode = WMError::WM_ERROR_INVALID_PARAM;
    }
    const char* const where = __func__;
    napi_value lastParam = (argc == 0) ? nullptr :
        (GetType(env, argv[0]) == napi_function ? argv[0] : nullptr);
    napi_value result = nullptr;
    std::shared_ptr<NapiAsyncTask> napiAsyncTask = CreateEmptyAsyncTask(env, lastParam, &result);
    auto asyncTask = [weakToken = wptr<Window>(windowToken_), errCode, where, env, task = napiAsyncTask] {
        auto weakWindow = weakToken.promote();
        if (weakWindow == nullptr) {
            WLOGFE("window is nullptr");
            task->Reject(env, JsErrUtils::CreateJsError(env, WMError::WM_ERROR_NULLPTR));
            return;
        }
        if (errCode != WMError::WM_OK) {
            task->Reject(env, JsErrUtils::CreateJsError(env, errCode));
            WLOGFE("window is nullptr or get invalid param");
            return;
        }
        bool flag = weakWindow->IsSupportWideGamut();
        task->Resolve(env, CreateJsValue(env, flag));
        WLOGI("%{public}s end, window [%{public}u, %{public}s] ret=%{public}u",
            where, weakWindow->GetWindowId(), weakWindow->GetWindowName().c_str(), flag);
    };
    if (napi_send_event(env, asyncTask, napi_eprio_high, "OnIsSupportWideGamut") != napi_status::napi_ok) {
        TLOGE(WmsLogTag::WMS_IMMS, "napi_send_event failed");
        napiAsyncTask->Reject(env,
            JsErrUtils::CreateJsError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY, "failed to send event"));
    }
    return result;
}

napi_value JsWindow::OnIsWindowSupportWideGamut(napi_env env, napi_callback_info info)
{
    const char* const where = __func__;

    size_t argc = 4;
    napi_value argv[4] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    napi_value lastParam = (argc == 0) ? nullptr :
        ((argv[0] != nullptr && GetType(env, argv[0]) == napi_function) ? argv[0] : nullptr);
    napi_value result = nullptr;
    std::shared_ptr<NapiAsyncTask> napiAsyncTask = CreateEmptyAsyncTask(env, lastParam, &result);
    auto asyncTask = [weakToken = wptr<Window>(windowToken_), where, env, task = napiAsyncTask] {
        auto weakWindow = weakToken.promote();
        if (weakWindow == nullptr) {
            WLOGFE("window is nullptr or get invalid param");
            task->Reject(env,
                JsErrUtils::CreateJsError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
                    "[window][isWindowSupportWideGamut]msg: The window is not created or destroyed"));
            return;
        }
        bool flag = weakWindow->IsSupportWideGamut();
        task->Resolve(env, CreateJsValue(env, flag));
        WLOGI("%{public}s end, window [%{public}u, %{public}s] ret=%{public}u",
            where, weakWindow->GetWindowId(), weakWindow->GetWindowName().c_str(), flag);
    };
    if (napi_send_event(env, asyncTask, napi_eprio_high, "OnIsWindowSupportWideGamut") != napi_status::napi_ok) {
        TLOGE(WmsLogTag::WMS_IMMS, "napi_send_event failed");
        napiAsyncTask->Reject(env,
            JsErrUtils::CreateJsError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
                "[window][isWindowSupportWideGamut]msg: Internal task error"));
    }
    return result;
}

napi_value JsWindow::OnSetBackgroundColor(napi_env env, napi_callback_info info)
{
    WMError errCode = WMError::WM_OK;
    size_t argc = 4;
    napi_value argv[4] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc < 1 || argc > 2) { // 2: maximum params num
        WLOGFE("Argc is invalid: %{public}zu", argc);
        errCode = WMError::WM_ERROR_INVALID_PARAM;
    }
    std::string color;
    if (errCode == WMError::WM_OK && !ConvertFromJsValue(env, argv[0], color)) {
        WLOGFE("Failed to convert parameter to background color");
        errCode = WMError::WM_ERROR_INVALID_PARAM;
    }

    const char* const where = __func__;
    napi_value lastParam = (argc <= 1) ? nullptr :
        (GetType(env, argv[1]) == napi_function ? argv[1] : nullptr);
    napi_value result = nullptr;
    std::shared_ptr<NapiAsyncTask> napiAsyncTask = CreateEmptyAsyncTask(env, lastParam, &result);
    auto asyncTask = [weakToken = wptr<Window>(windowToken_), color, errCode, where, env, task = napiAsyncTask] {
        auto weakWindow = weakToken.promote();
        if (weakWindow == nullptr) {
            WLOGFE("window is nullptr");
            task->Reject(env, JsErrUtils::CreateJsError(env, WMError::WM_ERROR_NULLPTR,
                "[window][setBackgroundColor]msg: invalid window"));
            return;
        }
        if (errCode != WMError::WM_OK) {
            task->Reject(env, JsErrUtils::CreateJsError(env, errCode,
                "[window][setBackgroundColor]msg: invalid params"));
            return;
        }
        WMError ret = weakWindow->SetBackgroundColor(color);
        if (ret == WMError::WM_OK) {
            task->Resolve(env, NapiGetUndefined(env));
        } else {
            task->Reject(env, JsErrUtils::CreateJsError(env, ret,
                "[window][setBackgroundColor]msg: set background color failed"));
        }
        WLOGD("%{public}s window [%{public}u, %{public}s] end",
            where, weakWindow->GetWindowId(), weakWindow->GetWindowName().c_str());
    };
    if (napi_send_event(env, asyncTask, napi_eprio_high, "OnSetBackgroundColor") != napi_status::napi_ok) {
        TLOGE(WmsLogTag::WMS_IMMS, "napi_send_event failed");
        napiAsyncTask->Reject(env,
            JsErrUtils::CreateJsError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
                "[window][setBackgroundColor]msg: failed to send event"));
    }
    return result;
}

napi_value JsWindow::OnSetWindowBackgroundColorSync(napi_env env, napi_callback_info info)
{
    WmErrorCode errCode = WmErrorCode::WM_OK;
    size_t argc = 4;
    napi_value argv[4] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc < 1) { // 1: params num
        WLOGFE("Argc is invalid: %{public}zu", argc);
        return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM,
            "[window][setWindowBackgroundColor]msg: Mandatory parameters are left unspecified");
    }
    std::string color;
    if (errCode == WmErrorCode::WM_OK && !GetWindowBackgroundColorFromJs(env, argv[0], color)) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "parse js color failed");
        errCode = WmErrorCode::WM_ERROR_INVALID_PARAM;
    }
    if (errCode == WmErrorCode::WM_ERROR_INVALID_PARAM) {
        return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM,
            "[window][setWindowBackgroundColor]msg: Incorrect parameter types");
    }

    if (windowToken_ == nullptr) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "window is null");
        return NapiThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
            "[window][setWindowBackgroundColor]msg: The window is not created or destroyed");
    }
    auto retErr = windowToken_->SetBackgroundColor(color);
    TLOGI(WmsLogTag::WMS_ATTRIBUTE, "win=%{public}u, color=%{public}s, retErr=%{public}d",
        windowToken_->GetWindowId(), color.c_str(), static_cast<int32_t>(retErr));
    WmErrorCode ret = WM_JS_TO_ERROR_CODE_MAP.at(retErr);
    if (ret == WmErrorCode::WM_OK) {
        return NapiGetUndefined(env);
    } else {
        return NapiThrowError(env, ret, "[window][setWindowBackgroundColor]");
    }
}

napi_value JsWindow::OnSetBrightness(napi_env env, napi_callback_info info)
{
    WMError errCode = WMError::WM_OK;
    size_t argc = 4;
    napi_value argv[4] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc < 1 || argc > 2) { // 2: maximum params num
        WLOGFE("Argc is invalid: %{public}zu", argc);
        errCode = WMError::WM_ERROR_INVALID_PARAM;
    }
    double brightness = UNDEFINED_BRIGHTNESS;
    if (errCode == WMError::WM_OK) {
        napi_value nativeVal = argv[0];
        if (nativeVal == nullptr) {
            WLOGFE("Failed to convert parameter to brightness");
            errCode = WMError::WM_ERROR_INVALID_PARAM;
        } else {
            CHECK_NAPI_RETCODE(errCode, WMError::WM_ERROR_INVALID_PARAM,
                napi_get_value_double(env, nativeVal, &brightness));
        }
    }

    napi_value lastParam = (argc <= 1) ? nullptr :
        (GetType(env, argv[1]) == napi_function ? argv[1] : nullptr);
    napi_value result = nullptr;
    std::shared_ptr<NapiAsyncTask> napiAsyncTask = CreateEmptyAsyncTask(env, lastParam, &result);
    auto asyncTask = [weakToken = wptr<Window>(windowToken_), brightness, errCode, env, task = napiAsyncTask] {
        auto weakWindow = weakToken.promote();
        if (weakWindow == nullptr) {
            WLOGFE("window is nullptr");
            task->Reject(env, JsErrUtils::CreateJsError(env, WMError::WM_ERROR_NULLPTR));
            return;
        }
        if (errCode != WMError::WM_OK) {
            task->Reject(env, JsErrUtils::CreateJsError(env, errCode, "Invalidate params."));
            return;
        }
        WMError ret = weakWindow->SetBrightness(brightness);
        if (ret == WMError::WM_OK) {
            task->Resolve(env, NapiGetUndefined(env));
        } else {
            task->Reject(env, JsErrUtils::CreateJsError(env, ret, "Window set brightness failed"));
        }
        TLOGNI(WmsLogTag::WMS_ATTRIBUTE, "winId=%{public}u, brightness=%{public}f, result: %{public}d",
            weakWindow->GetWindowId(), brightness, ret);
    };
    if (napi_send_event(env, asyncTask, napi_eprio_high, "OnSetBrightness") != napi_status::napi_ok) {
        TLOGE(WmsLogTag::WMS_IMMS, "napi_send_event failed");
        napiAsyncTask->Reject(env,
            JsErrUtils::CreateJsError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY, "failed to send event"));
    }
    return result;
}

napi_value JsWindow::OnSetWindowBrightness(napi_env env, napi_callback_info info)
{
    WmErrorCode errCode = WmErrorCode::WM_OK;
    size_t argc = 4;
    napi_value argv[4] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc < 1) { // 1: params num
        TLOGE(WmsLogTag::WMS_IMMS, "Argc is invalid: %{public}zu", argc);
        return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM,
            "[window][setWindowBrightness]msg: Mandatory parameters are left unspecified");
    }
    double brightness = UNDEFINED_BRIGHTNESS;
    napi_value nativeVal = argv[0];
    if (nativeVal == nullptr) {
        TLOGE(WmsLogTag::WMS_IMMS, "Failed to convert parameter to brightness");
        errCode = WmErrorCode::WM_ERROR_INVALID_PARAM;
    } else {
        CHECK_NAPI_RETCODE(errCode, WmErrorCode::WM_ERROR_INVALID_PARAM,
            napi_get_value_double(env, nativeVal, &brightness));
    }
    if (errCode == WmErrorCode::WM_ERROR_INVALID_PARAM) {
        return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM,
            "[window][setWindowBrightness]msg: Incorrect parameter types");
    }
    napi_value lastParam = (argc <= 1) ? nullptr :
        ((argv[1] != nullptr && GetType(env, argv[1]) == napi_function) ? argv[1] : nullptr);
    napi_value result = nullptr;
    std::shared_ptr<NapiAsyncTask> napiAsyncTask = CreateEmptyAsyncTask(env, lastParam, &result);
    auto asyncTask = [weakToken = wptr<Window>(windowToken_), brightness, env, task = napiAsyncTask, where = __func__] {
        auto weakWindow = weakToken.promote();
        if (weakWindow == nullptr) {
            task->Reject(env, JsErrUtils::CreateJsError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
                "[window][setWindowBrightness]msg: The window is not created or destroyed"));
            return;
        }
        WmErrorCode ret = WM_JS_TO_ERROR_CODE_MAP.at(weakWindow->SetBrightness(brightness));
        if (ret == WmErrorCode::WM_OK) {
            task->Resolve(env, NapiGetUndefined(env));
        } else {
            task->Reject(env, JsErrUtils::CreateJsError(env, ret, "[window][setWindowBrightness]"));
        }
        TLOGNI(WmsLogTag::WMS_ATTRIBUTE, "%{public}s end. WinId=%{public}u, brightness=%{public}f, result: %{public}d",
            where, weakWindow->GetWindowId(), brightness, ret);
    };
    if (napi_send_event(env, asyncTask, napi_eprio_high, "OnSetWindowBrightness") != napi_status::napi_ok) {
        TLOGE(WmsLogTag::WMS_IMMS, "napi_send_event failed");
        napiAsyncTask->Reject(env,
            JsErrUtils::CreateJsError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
                "[window][setWindowBrightness]msg: Internal task error"));
    }
    return result;
}

napi_value JsWindow::OnSetDimBehind(napi_env env, napi_callback_info info)
{
    NapiAsyncTask::CompleteCallback complete =
        [](napi_env env, NapiAsyncTask& task, int32_t status) {
            task.Reject(env, JsErrUtils::CreateJsError(env, WMError::WM_ERROR_DEVICE_NOT_SUPPORT));
        };
    size_t argc = 4;
    napi_value argv[4] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    napi_value lastParam = (argc <= 1) ? nullptr : (GetType(env, argv[1]) == napi_function ? argv[1] : nullptr);
    napi_value result = nullptr;
    NapiAsyncTask::Schedule("JsWindow::OnSetDimBehind",
        env, CreateAsyncTaskWithLastParam(env, lastParam, nullptr, std::move(complete), &result));
    return result;
}

napi_value JsWindow::OnSetFocusable(napi_env env, napi_callback_info info)
{
    WMError errCode = WMError::WM_OK;
    size_t argc = 4;
    napi_value argv[4] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc < 1 || argc > 2) { // 2: maximum params num
        TLOGE(WmsLogTag::WMS_FOCUS, "Argc is invalid: %{public}zu", argc);
        errCode = WMError::WM_ERROR_INVALID_PARAM;
    }
    bool focusable = true;
    if (!ConvertFromJsValue(env, argv[0], focusable)) {
        TLOGE(WmsLogTag::WMS_FOCUS, "Failed to convert parameter to focusable");
        errCode = WMError::WM_ERROR_INVALID_PARAM;
    }

    napi_value lastParam = (argc <= 1) ? nullptr : (GetType(env, argv[1]) == napi_function ? argv[1] : nullptr);
    napi_value result = nullptr;
    std::shared_ptr<NapiAsyncTask> napiAsyncTask = CreateEmptyAsyncTask(env, lastParam, &result);
    auto asyncTask =
        [weakToken = wptr<Window>(windowToken_), focusable, errCode, env, task = napiAsyncTask, where = __func__] {
            auto window = weakToken.promote();
            if (window == nullptr) {
                TLOGNE(WmsLogTag::WMS_FOCUS, "%{public}s: window is nullptr", where);
                task->Reject(env, JsErrUtils::CreateJsError(env, WMError::WM_ERROR_NULLPTR));
                return;
            }
            if (errCode != WMError::WM_OK) {
                task->Reject(env, JsErrUtils::CreateJsError(env, errCode, "Invalidate params."));
                return;
            }
            WMError ret = window->SetFocusable(focusable);
            if (ret == WMError::WM_OK) {
                task->Resolve(env, NapiGetUndefined(env));
            } else {
                task->Reject(env, JsErrUtils::CreateJsError(env, ret, "Window set focusable failed"));
            }
            TLOGNI(WmsLogTag::WMS_FOCUS, "%{public}s: Window [%{public}u, %{public}s] set focusable end",
                where, window->GetWindowId(), window->GetWindowName().c_str());
        };
    if (napi_send_event(env, asyncTask, napi_eprio_high, "OnSetFocusable") != napi_status::napi_ok) {
        TLOGE(WmsLogTag::WMS_FOCUS, "window state is abnormal!");
    }
    return result;
}

napi_value JsWindow::OnSetWindowFocusable(napi_env env, napi_callback_info info)
{
    size_t argc = 4;
    napi_value argv[4] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc < 1) { // 1: maximum params num
        TLOGE(WmsLogTag::WMS_FOCUS, "Argc is invalid: %{public}zu", argc);
        return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM,
            "[window][setWindowFocusable]msg: Argc is invalid");
    }
    bool focusable = true;
    if (!ConvertFromJsValue(env, argv[0], focusable)) {
        TLOGE(WmsLogTag::WMS_FOCUS, "Failed to convert parameter to focusable");
        return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM,
            "[window][setWindowFocusable]msg: Failed to convert parameter to focusable");
    }

    napi_value lastParam = (argc <= 1) ? nullptr :
        ((argv[1] != nullptr && GetType(env, argv[1]) == napi_function) ? argv[1] : nullptr);
    napi_value result = nullptr;
    std::shared_ptr<NapiAsyncTask> napiAsyncTask = CreateEmptyAsyncTask(env, lastParam, &result);
    auto asyncTask =
        [weakToken = wptr<Window>(windowToken_), focusable, env, task = napiAsyncTask, where = __func__] {
            auto window = weakToken.promote();
            if (window == nullptr) {
                task->Reject(env,
                    JsErrUtils::CreateJsError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
                        "[window][setWindowFocusable]msg: Window is nullptr"));
                return;
            }
            WmErrorCode ret = WM_JS_TO_ERROR_CODE_MAP.at(window->SetFocusable(focusable));
            if (ret == WmErrorCode::WM_OK) {
                task->Resolve(env, NapiGetUndefined(env));
            } else {
                task->Reject(env, JsErrUtils::CreateJsError(env, ret,
                    "[window][setWindowFocusable]msg: Window set focusable failed"));
            }
            TLOGNI(WmsLogTag::WMS_FOCUS, "%{public}s: Window [%{public}u, %{public}s] set focusable end",
                where, window->GetWindowId(), window->GetWindowName().c_str());
        };
    if (napi_send_event(env, asyncTask, napi_eprio_high, "OnSetWindowFocusable") != napi_status::napi_ok) {
        napiAsyncTask->Reject(env,
            JsErrUtils::CreateJsError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
                "[window][setWindowFocusable]msg: Failed to send event"));
    }
    return result;
}

napi_value JsWindow::OnSetTopmost(napi_env env, napi_callback_info info)
{
    if (!Permission::IsSystemCalling()) {
        TLOGE(WmsLogTag::WMS_HIERARCHY, "SetTopmost permission denied!");
        return NapiThrowError(env, WmErrorCode::WM_ERROR_NOT_SYSTEM_APP);
    }
    if (windowToken_ == nullptr) {
        return NapiThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
    }
    if (!WindowHelper::IsMainWindow(windowToken_->GetType())) {
        TLOGE(WmsLogTag::WMS_HIERARCHY, "SetTopmost is not allowed since window is not main window");
        return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_CALLING);
    }

    size_t argc = 4;
    napi_value argv[4] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc != 1 || argv[0] == nullptr) {
        TLOGE(WmsLogTag::WMS_HIERARCHY, "Argc is invalid: %{public}zu. Failed to convert parameter to topmost", argc);
        return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM);
    }
    bool topmost = false;
    napi_get_value_bool(env, argv[0], &topmost);

    wptr<Window> weakToken(windowToken_);
    std::shared_ptr<WmErrorCode> errCodePtr = std::make_shared<WmErrorCode>(WmErrorCode::WM_OK);
    NapiAsyncTask::ExecuteCallback execute = [weakToken, topmost, errCodePtr] {
        if (errCodePtr == nullptr) {
            return;
        }
        auto window = weakToken.promote();
        if (window == nullptr) {
            *errCodePtr = WmErrorCode::WM_ERROR_STATE_ABNORMALLY;
            return;
        }
        *errCodePtr = WM_JS_TO_ERROR_CODE_MAP.at(window->SetTopmost(topmost));
        TLOGI(WmsLogTag::WMS_HIERARCHY, "Window [%{public}u, %{public}s] set topmost end",
            window->GetWindowId(), window->GetWindowName().c_str());
    };
    NapiAsyncTask::CompleteCallback complete =
        [weakToken, errCodePtr](napi_env env, NapiAsyncTask& task, int32_t status) {
            if (errCodePtr == nullptr) {
                task.Reject(env, JsErrUtils::CreateJsError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
                    "[window][setTopmost]msg: System abnormal"));
                return;
            }
            if (*errCodePtr == WmErrorCode::WM_OK) {
                task.Resolve(env, NapiGetUndefined(env));
            } else {
                task.Reject(env, JsErrUtils::CreateJsError(env, *errCodePtr,
                    "[window][setTopmost]msg: Window set topmost failed"));
            }
        };
    napi_value result = nullptr;
    NapiAsyncTask::Schedule("JsWindow::OnSetTopmost",
        env, CreateAsyncTaskWithLastParam(env, nullptr, std::move(execute), std::move(complete), &result));
    return result;
}

napi_value JsWindow::OnSetWindowTopmost(napi_env env, napi_callback_info info)
{
    if (windowToken_ == nullptr) {
        TLOGE(WmsLogTag::WMS_HIERARCHY, "windowToken is nullptr");
        return NapiThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
            "[window][setWindowTopmost]msg: WindowToken is nullptr");
    }
    if (windowToken_->IsPadAndNotFreeMultiWindowCompatibleMode()) {
        TLOGE(WmsLogTag::WMS_HIERARCHY, "This is PcAppInPad, not support");
        return NapiGetUndefined(env);
    }
    if (!windowToken_->IsPcOrPadFreeMultiWindowMode()) {
        TLOGE(WmsLogTag::WMS_HIERARCHY, "device not support");
        return NapiThrowError(env, WmErrorCode::WM_ERROR_DEVICE_NOT_SUPPORT,
            "[window][setWindowTopmost]msg: Device not support");
    }
    if (!WindowHelper::IsMainWindow(windowToken_->GetType())) {
        TLOGE(WmsLogTag::WMS_HIERARCHY, "not allowed since window is not main window");
        return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_CALLING,
            "[window][setWindowTopmost]msg: Not allowed since window is not main window");
    }
    size_t argc = FOUR_PARAMS_SIZE;
    napi_value argv[FOUR_PARAMS_SIZE] = { nullptr };
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    bool isMainWindowTopmost = false;
    if (argc != 1 || !ConvertFromJsValue(env, argv[INDEX_ZERO], isMainWindowTopmost)) {
        TLOGE(WmsLogTag::WMS_HIERARCHY,
            "Argc is invalid: %{public}zu. Failed to convert parameter to topmost", argc);
        return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM,
            "[window][setWindowTopmost]msg: Argc is invalid");
    }
    napi_value result = nullptr;
    std::shared_ptr<NapiAsyncTask> napiAsyncTask = CreateEmptyAsyncTask(env, nullptr, &result);
    const char* const where = __func__;
    auto asyncTask = [weakToken = wptr<Window>(windowToken_), isMainWindowTopmost, env,
        task = napiAsyncTask, where] {
        auto window = weakToken.promote();
        if (window == nullptr) {
            TLOGNE(WmsLogTag::WMS_HIERARCHY, "%{public}s window is nullptr", where);
            WmErrorCode wmErrorCode = WmErrorCode::WM_ERROR_STATE_ABNORMALLY;
            task->Reject(env, JsErrUtils::CreateJsError(env, wmErrorCode, "window is nullptr"));
            return;
        }
        auto ret = WM_JS_TO_ERROR_CODE_MAP.at(window->SetMainWindowTopmost(isMainWindowTopmost));
        if (ret != WmErrorCode::WM_OK) {
            task->Reject(env, JsErrUtils::CreateJsError(env, ret, "Window set main window topmost failed"));
            return;
        }
        task->Resolve(env, NapiGetUndefined(env));
        TLOGNI(WmsLogTag::WMS_HIERARCHY,
            "%{public}s id: %{public}u, name: %{public}s, isMainWindowTopmost: %{public}d",
            where, window->GetWindowId(), window->GetWindowName().c_str(), isMainWindowTopmost);
    };
    if (napi_send_event(env, asyncTask, napi_eprio_high, "OnSetWindowTopmost") != napi_status::napi_ok) {
        napiAsyncTask->Reject(env,
            CreateJsError(env, static_cast<int32_t>(WmErrorCode::WM_ERROR_STATE_ABNORMALLY), "send event failed"));
    }
    return result;
}

/** @note @window.hierarchy */
napi_value JsWindow::OnSetSubWindowZLevel(napi_env env, napi_callback_info info)
{
    if (windowToken_ == nullptr) {
        return NapiThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
            "[window][setSubWindowZLevel]msg: WindowToken is nullptr");
    }
    if (!WindowHelper::IsSubWindow(windowToken_->GetType())) {
        TLOGE(WmsLogTag::WMS_HIERARCHY, "not allowed since window is not sub window");
        return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_CALLING,
            "[window][setSubWindowZLevel]msg: Not allowed since window is not sub window");
    }
    WmErrorCode errCode = WmErrorCode::WM_OK;
    size_t argc = FOUR_PARAMS_SIZE;
    napi_value argv[FOUR_PARAMS_SIZE] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc != 1 || argv[0] == nullptr) { // 1: params num
        TLOGE(WmsLogTag::WMS_HIERARCHY, "argc is invalid: %{public}zu", argc);
        return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM,
            "[window][setSubWindowZLevel]msg: Argc is invalid");
    }
    int32_t zLevel = 0;
    if (errCode == WmErrorCode::WM_OK && !ConvertFromJsValue(env, argv[0], zLevel)) {
        TLOGE(WmsLogTag::WMS_HIERARCHY, "failed to convert paramter to zLevel");
        return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM,
            "[window][setSubWindowZLevel]msg: Failed to convert paramter to zLevel");
    }
    napi_value lastParam = nullptr;
    napi_value result = nullptr;
    std::shared_ptr<NapiAsyncTask> napiAsyncTask = CreateEmptyAsyncTask(env, lastParam, &result);
    auto asyncTask = [windowToken = wptr<Window>(windowToken_), zLevel, env,
        task = napiAsyncTask, where = __func__] {
        auto window = windowToken.promote();
        if (window == nullptr) {
            TLOGNE(WmsLogTag::WMS_HIERARCHY, "%{public}s window is nullptr", where);
            task->Reject(env, JsErrUtils::CreateJsError(env,
                WmErrorCode::WM_ERROR_STATE_ABNORMALLY, "[window][setSubWindowZLevel]msg: Window is nullptr"));
            return;
        }
        WmErrorCode ret = WM_JS_TO_ERROR_CODE_MAP.at(window->SetSubWindowZLevel(zLevel));
        ret == WmErrorCode::WM_OK ? task->Resolve(env, NapiGetUndefined(env)) :
            task->Reject(env, JsErrUtils::CreateJsError(env, ret,
                "[window][setSubWindowZLevel]msg: Set sub window zLevel failed"));
        TLOGNI(WmsLogTag::WMS_HIERARCHY, "window [%{public}u, %{public}s], zLevel = %{public}d, ret = %{public}d",
            window->GetWindowId(), window->GetWindowName().c_str(), zLevel, ret);
    };
    if (napi_send_event(env, asyncTask, napi_eprio_high, "OnSetSubWindowZLevel") != napi_status::napi_ok) {
        napiAsyncTask->Reject(env, JsErrUtils::CreateJsError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
            "[window][setSubWindowZLevel]msg: Failed to send event"));
    }
    return result;
}

/** @note @window.hierarchy */
napi_value JsWindow::OnGetSubWindowZLevel(napi_env env, napi_callback_info info)
{
    size_t argc = FOUR_PARAMS_SIZE;
    napi_value argv[FOUR_PARAMS_SIZE] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc > 1) {
        TLOGE(WmsLogTag::WMS_HIERARCHY, "argc is invalid: %{public}zu", argc);
        return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM);
    }
    if (windowToken_ == nullptr) {
        TLOGE(WmsLogTag::WMS_HIERARCHY, "window is nullptr");
        return NapiThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
            "[window][getSubWindowZLevel]msg: Window is nullptr");
    }
    int32_t zLevel = 0;
    WmErrorCode ret = WM_JS_TO_ERROR_CODE_MAP.at(windowToken_->GetSubWindowZLevel(zLevel));
    if (ret != WmErrorCode::WM_OK) {
        return NapiThrowError(env, ret, "[window][getSubWindowZLevel]msg: Get sub window zLevel failed");
    }
    napi_value objValue = nullptr;
    napi_set_named_property(env, objValue, "zLevel", CreateJsValue(env, zLevel));
    if (objValue != nullptr) {
        return objValue;
    } else {
        return NapiThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
    }
}

napi_value JsWindow::OnSetWindowDelayRaiseOnDrag(napi_env env, napi_callback_info info)
{
    if (windowToken_ == nullptr) {
        TLOGE(WmsLogTag::WMS_FOCUS, "windowToken is nullptr");
        return NapiThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
            "[window][setWindowDelayRaiseOnDrag]msg: WindowToken is nullptr");
    }
    size_t argc = FOUR_PARAMS_SIZE;
    napi_value argv[FOUR_PARAMS_SIZE] = { nullptr };
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc != 1 || argv[0] == nullptr) {
        TLOGE(WmsLogTag::WMS_FOCUS,
            "Argc is invalid: %{public}zu. Failed to convert parameter to delay raise", argc);
        return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM,
            "[window][setWindowDelayRaiseOnDrag]msg: Argc is invalid");
    }
    bool isDelayRaise = false;
    if (!ConvertFromJsValue(env, argv[INDEX_ZERO], isDelayRaise)) {
        TLOGE(WmsLogTag::WMS_FOCUS, "Failed to convert parameter from jsValue");
        return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM,
            "[window][setWindowDelayRaiseOnDrag]msg: Failed to convert parameter from jsValue");
    }
    auto result = windowToken_->SetWindowDelayRaiseEnabled(isDelayRaise);
    if (result != WMError::WM_OK) {
        TLOGE(WmsLogTag::WMS_FOCUS, "failed");
        return NapiThrowError(env, WM_JS_TO_ERROR_CODE_MAP.at(result),
            "[window][setWindowDelayRaiseOnDrag]");
    }
    return NapiGetUndefined(env);
}

napi_value JsWindow::OnSetKeepScreenOn(napi_env env, napi_callback_info info)
{
    WMError errCode = WMError::WM_OK;
    size_t argc = 4;
    napi_value argv[4] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc < 1 || argc > 2) { // 2: maximum params num
        WLOGFE("Argc is invalid: %{public}zu", argc);
        errCode = WMError::WM_ERROR_INVALID_PARAM;
    }
    bool keepScreenOn = true;
    if (errCode == WMError::WM_OK) {
        napi_value nativeVal = argv[0];
        if (nativeVal == nullptr) {
            WLOGFE("Failed to convert parameter to keepScreenOn");
            errCode = WMError::WM_ERROR_INVALID_PARAM;
        } else {
            CHECK_NAPI_RETCODE(errCode, WMError::WM_ERROR_INVALID_PARAM,
                napi_get_value_bool(env, nativeVal, &keepScreenOn));
        }
    }

    napi_value lastParam = (argc <= 1) ? nullptr :
        (GetType(env, argv[1]) == napi_function ? argv[1] : nullptr);
    napi_value result = nullptr;
    std::shared_ptr<NapiAsyncTask> napiAsyncTask = CreateEmptyAsyncTask(env, lastParam, &result);
    auto asyncTask = [weakToken = wptr<Window>(windowToken_), keepScreenOn, errCode,
                      env, task = napiAsyncTask] {
        auto weakWindow = weakToken.promote();
        if (weakWindow == nullptr) {
            WLOGFE("window is nullptr");
            task->Reject(env, JsErrUtils::CreateJsError(env, WMError::WM_ERROR_NULLPTR));
            return;
        }
        if (errCode != WMError::WM_OK) {
            task->Reject(env, JsErrUtils::CreateJsError(env, errCode, "Invalidate params."));
            return;
        }
        WMError ret = weakWindow->SetKeepScreenOn(keepScreenOn);
        if (ret == WMError::WM_OK) {
            task->Resolve(env, NapiGetUndefined(env));
        } else {
            task->Reject(env, JsErrUtils::CreateJsError(env, ret, "Window set keep screen on failed"));
        }
        WLOGI("Window [%{public}u, %{public}s] set keep screen on end",
            weakWindow->GetWindowId(), weakWindow->GetWindowName().c_str());
    };
    if (napi_send_event(env, asyncTask, napi_eprio_high, "OnSetKeepScreenOn") != napi_status::napi_ok) {
        TLOGE(WmsLogTag::WMS_IMMS, "napi_send_event failed");
        napiAsyncTask->Reject(env,
            JsErrUtils::CreateJsError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY, "failed to send event"));
    }
    return result;
}

napi_value JsWindow::OnSetWindowKeepScreenOn(napi_env env, napi_callback_info info)
{
    WmErrorCode errCode = WmErrorCode::WM_OK;
    size_t argc = 4;
    napi_value argv[4] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc < 1) { // 1: params num
        WLOGFE("Argc is invalid: %{public}zu", argc);
        return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM,
            "[window][setWindowKeepScreenOn]msg: Mandatory parameters are left unspecified");
    }
    bool keepScreenOn = true;
    napi_value nativeVal = argv[0];
    if (nativeVal == nullptr) {
        WLOGFE("Failed to convert parameter to keepScreenOn");
        errCode = WmErrorCode::WM_ERROR_INVALID_PARAM;
    } else {
        CHECK_NAPI_RETCODE(errCode, WmErrorCode::WM_ERROR_INVALID_PARAM,
            napi_get_value_bool(env, nativeVal, &keepScreenOn));
    }
    if (errCode == WmErrorCode::WM_ERROR_INVALID_PARAM) {
        return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM,
            "[window][setWindowKeepScreenOn]msg: Incorrect parameter types");
    }
    wptr<Window> weakToken(windowToken_);
    std::shared_ptr<WmErrorCode> errCodePtr = std::make_shared<WmErrorCode>(WmErrorCode::WM_OK);
    NapiAsyncTask::ExecuteCallback execute = [weakToken, keepScreenOn, errCodePtr, where = __func__] {
        if (errCodePtr == nullptr) {
            return;
        }
        auto weakWindow = weakToken.promote();
        if (weakWindow == nullptr) {
            *errCodePtr = WmErrorCode::WM_ERROR_STATE_ABNORMALLY;
            return;
        }
        *errCodePtr = WM_JS_TO_ERROR_CODE_MAP.at(weakWindow->SetKeepScreenOn(keepScreenOn));
        TLOGNI(WmsLogTag::WMS_ATTRIBUTE, "%{public}s end. Wid: %{public}u, isKeepScreenOn: %{public}d",
            where, weakWindow->GetWindowId(), keepScreenOn);
    };
    NapiAsyncTask::CompleteCallback complete =
        [weakToken, keepScreenOn, errCodePtr](napi_env env, NapiAsyncTask& task, int32_t status) {
            if (errCodePtr == nullptr) {
                task.Reject(env, JsErrUtils::CreateJsError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
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

    napi_value lastParam = (argc <= 1) ? nullptr :
        ((argv[1] != nullptr && GetType(env, argv[1]) == napi_function) ? argv[1] : nullptr);
    napi_value result = nullptr;
    NapiAsyncTask::Schedule("JsWindow::OnSetWindowKeepScreenOn",
        env, CreateAsyncTaskWithLastParam(env, lastParam, std::move(execute), std::move(complete), &result));
    return result;
}

napi_value JsWindow::OnSetWakeUpScreen(napi_env env, napi_callback_info info)
{
    if (!Permission::IsSystemCalling() && !Permission::IsStartByHdcd()) {
        WLOGFE("set wake up screen permission denied!");
        return NapiThrowError(env, WmErrorCode::WM_ERROR_NOT_SYSTEM_APP,
            "[window][setWakeUpScreen]msg: permission denied");
    }
    if (windowToken_ == nullptr) {
        return NapiThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
            "[window][setWakeUpScreen]msg: invalid window");
    }
    size_t argc = 4;
    napi_value argv[4] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc < 1) {
        WLOGFE("Argc is invalid: %{public}zu", argc);
        return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM);
    }
    bool wakeUp = false;
    napi_value nativeVal = argv[0];
    if (nativeVal == nullptr) {
        WLOGFE("Failed to convert parameter to keepScreenOn");
        return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM);
    } else {
        napi_get_value_bool(env, nativeVal, &wakeUp);
    }

    WmErrorCode ret = WM_JS_TO_ERROR_CODE_MAP.at(windowToken_->SetTurnScreenOn(wakeUp));
    if (ret != WmErrorCode::WM_OK) {
        return NapiThrowError(env, ret, "[window][setWakeUpScreen]msg: turn screen on failed");
    }

    WLOGI("Window [%{public}u, %{public}s] set wake up screen %{public}d end",
        windowToken_->GetWindowId(), windowToken_->GetWindowName().c_str(), wakeUp);
    return NapiGetUndefined(env);
}

napi_value JsWindow::OnSetOutsideTouchable(napi_env env, napi_callback_info info)
{
    size_t argc = 4;
    napi_value argv[4] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    napi_value lastParam = (argc <= 1) ? nullptr :
        (GetType(env, argv[1]) == napi_function ? argv[1] : nullptr);
    napi_value result = nullptr;
    std::shared_ptr<NapiAsyncTask> napiAsyncTask = CreateEmptyAsyncTask(env, lastParam, &result);
    auto asyncTask = [env, task = napiAsyncTask] {
        task->Reject(env, JsErrUtils::CreateJsError(env, WMError::WM_ERROR_DEVICE_NOT_SUPPORT));
    };
    if (napi_send_event(env, asyncTask, napi_eprio_high, "OnSetOutsideTouchable") != napi_status::napi_ok) {
        TLOGE(WmsLogTag::WMS_EVENT, "napi_send_event failed");
    }
    return result;
}

napi_value JsWindow::OnSetPrivacyMode(napi_env env, napi_callback_info info)
{
    WMError errCode = WMError::WM_OK;
    size_t argc = 4;
    napi_value argv[4] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc < 1 || argc > 2) { // 2: maximum params num
        WLOGFE("Argc is invalid: %{public}zu", argc);
        errCode = WMError::WM_ERROR_INVALID_PARAM;
    }
    bool isPrivacyMode = false;
    if (errCode == WMError::WM_OK) {
        napi_value nativeVal = argv[0];
        if (nativeVal == nullptr) {
            WLOGFE("Failed to convert parameter to isPrivacyMode");
            errCode = WMError::WM_ERROR_INVALID_PARAM;
        } else {
            CHECK_NAPI_RETCODE(errCode, WMError::WM_ERROR_INVALID_PARAM,
                napi_get_value_bool(env, nativeVal, &isPrivacyMode));
        }
    }

    napi_value lastParam = (argc <= 1) ? nullptr :
        (GetType(env, argv[1]) == napi_function ? argv[1] : nullptr);
    napi_value result = nullptr;
    std::shared_ptr<NapiAsyncTask> napiAsyncTask = CreateEmptyAsyncTask(env, lastParam, &result);
    auto asyncTask = [weakToken = wptr<Window>(windowToken_), isPrivacyMode, errCode, env, task = napiAsyncTask] {
        auto weakWindow = weakToken.promote();
        if (weakWindow == nullptr) {
            WLOGFE("window is nullptr");
            task->Reject(env, JsErrUtils::CreateJsError(env, WMError::WM_ERROR_NULLPTR,
                "[window][setPrivacyMode]msg: invalid window"));
            return;
        }
        if (errCode != WMError::WM_OK) {
            task->Reject(env, JsErrUtils::CreateJsError(env, errCode,
                "[window][setPrivacyMode]msg: invalid params"));
            return;
        }
        weakWindow->SetPrivacyMode(isPrivacyMode);
        task->Resolve(env, NapiGetUndefined(env));
        WLOGI("Window [%{public}u, %{public}s] set privacy mode end, mode=%{public}u",
            weakWindow->GetWindowId(), weakWindow->GetWindowName().c_str(), isPrivacyMode);
    };
    if (napi_send_event(env, asyncTask, napi_eprio_high, "OnSetPrivacyMode") != napi_status::napi_ok) {
        TLOGE(WmsLogTag::WMS_IMMS, "napi_send_event failed");
        napiAsyncTask->Reject(env,
            JsErrUtils::CreateJsError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
                "[window][setPrivacyMode]msg: failed to send event"));
    }
    return result;
}

napi_value JsWindow::OnSetWindowPrivacyMode(napi_env env, napi_callback_info info)
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
    auto asyncTask = [weakToken = wptr<Window>(windowToken_), isPrivacyMode, where, env, task = napiAsyncTask] {
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
                "[window][setWindowPrivacyMode]msg: Need ohos.permission.PRIVACY_WINDOW permission"));
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
        napiAsyncTask->Reject(env,
            JsErrUtils::CreateJsError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
                "[window][setWindowPrivacyMode]msg: Internal task error"));
    }
    return result;
}

napi_value JsWindow::OnSetTouchable(napi_env env, napi_callback_info info)
{
    WMError errCode = WMError::WM_OK;
    size_t argc = 4;
    napi_value argv[4] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc < 1 || argc > 2) { // 2: maximum params num
        WLOGFE("Argc is invalid: %{public}zu", argc);
        errCode = WMError::WM_ERROR_INVALID_PARAM;
    }
    bool touchable = true;
    if (errCode == WMError::WM_OK) {
        napi_value nativeVal = argv[0];
        if (nativeVal == nullptr) {
            WLOGFE("Failed to convert parameter to touchable");
            errCode = WMError::WM_ERROR_INVALID_PARAM;
        } else {
            CHECK_NAPI_RETCODE(errCode, WMError::WM_ERROR_INVALID_PARAM,
                napi_get_value_bool(env, nativeVal, &touchable));
        }
    }

    wptr<Window> weakToken(windowToken_);
    const char* const where = __func__;
    napi_value lastParam = (argc <= 1) ? nullptr :
        (GetType(env, argv[1]) == napi_function ? argv[1] : nullptr);
    napi_value result = nullptr;
    std::shared_ptr<NapiAsyncTask> napiAsyncTask = CreateEmptyAsyncTask(env, lastParam, &result);
    auto asyncTask = [weakToken, touchable, errCode, where, env, task = napiAsyncTask] {
        auto weakWindow = weakToken.promote();
        if (weakWindow == nullptr) {
            WLOGFE("window is nullptr");
            task->Reject(env, JsErrUtils::CreateJsError(env, WMError::WM_ERROR_NULLPTR));
            return;
        }
        if (errCode != WMError::WM_OK) {
            task->Reject(env, JsErrUtils::CreateJsError(env, errCode, "Invalidate params."));
            return;
        }
        WMError ret = weakWindow->SetTouchable(touchable);
        if (ret == WMError::WM_OK) {
            task->Resolve(env, NapiGetUndefined(env));
        } else {
            task->Reject(env, JsErrUtils::CreateJsError(env, ret, "Window set touchable failed"));
        }
        WLOGI("%{public}s end, window [%{public}u, %{public}s]",
            where, weakWindow->GetWindowId(), weakWindow->GetWindowName().c_str());
    };
    if (napi_send_event(env, asyncTask, napi_eprio_high, "OnSetTouchable") != napi_status::napi_ok) {
        TLOGE(WmsLogTag::WMS_EVENT, "napi_send_event failed");
    }
    return result;
}

napi_value JsWindow::OnSetTouchableAreas(napi_env env, napi_callback_info info)
{
    if (!Permission::IsSystemCalling()) {
        TLOGE(WmsLogTag::WMS_EVENT, "OnSetTouchableAreas permission denied!");
        return NapiThrowError(env, WmErrorCode::WM_ERROR_NOT_SYSTEM_APP);
    }
    if (windowToken_ == nullptr) {
        TLOGE(WmsLogTag::WMS_EVENT, "WindowToken_ is nullptr");
        return NapiThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
            "[window][setTouchableAreas]msg:window is null");
    }
    Rect windowRect = windowToken_->GetRect();
    std::vector<Rect> touchableAreas;
    WmErrorCode errCode = ParseTouchableAreas(env, info, windowRect, touchableAreas);
    if (errCode != WmErrorCode::WM_OK) {
        return NapiThrowError(env, errCode);
    }
    wptr<Window> weakToken(windowToken_);
    const char* const where = __func__;
    napi_value result = nullptr;
    std::shared_ptr<NapiAsyncTask> napiAsyncTask = CreateEmptyAsyncTask(env, nullptr, &result);
    auto asyncTask = [weakToken, touchableAreas, where, env, task = napiAsyncTask] {
        auto weakWindow = weakToken.promote();
        if (weakWindow == nullptr) {
            TLOGNE(WmsLogTag::WMS_EVENT, "%{public}s window is nullptr", where);
            task->Reject(env, JsErrUtils::CreateJsError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
                "[window][setTouchableAreas]msg:window is null"));
            return;
        }
        WMError ret = weakWindow->SetTouchHotAreas(touchableAreas);
        if (ret == WMError::WM_OK) {
            task->Resolve(env, NapiGetUndefined(env));
        } else {
            WmErrorCode wmErrorCode = WM_JS_TO_ERROR_CODE_MAP.at(ret);
            task->Reject(env, JsErrUtils::CreateJsError(env, wmErrorCode, "OnSetTouchableAreas failed"));
        }
        TLOGNI(WmsLogTag::WMS_EVENT, "%{public}s Window [%{public}u, %{public}s] end",
            where, weakWindow->GetWindowId(), weakWindow->GetWindowName().c_str());
    };
    if (napi_send_event(env, asyncTask, napi_eprio_high, "OnSetTouchableAreas") != napi_status::napi_ok) {
        napiAsyncTask->Reject(env, JsErrUtils::CreateJsError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
            "[window][setTouchableAreas]msg:failed to send event"));
    }
    return result;
}

napi_value JsWindow::OnSetResizeByDragEnabled(napi_env env, napi_callback_info info)
{
    size_t argc = 4;
    napi_value argv[4] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc < 1 || argc > 2) { // 2: maximum params num
        TLOGE(WmsLogTag::WMS_LAYOUT, "Argc is invalid: %{public}zu", argc);
        return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM,
            "[window][setResizeByDragEnabled]msg: Number of parameters is invalid");
    }
    bool dragEnabled = true;
    if (!ConvertFromJsValue(env, argv[0], dragEnabled)) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "Failed to convert parameter to dragEnabled");
        return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM,
            "[window][setResizeByDragEnabled]msg: Failed to convert parameter to dragEnabled");
    }
    napi_value lastParam = (argc <= 1) ? nullptr : (GetType(env, argv[1]) == napi_function ? argv[1] : nullptr);
    napi_value result = nullptr;
    std::shared_ptr<NapiAsyncTask> napiAsyncTask = CreateEmptyAsyncTask(env, lastParam, &result);
    auto asyncTask = [weakToken = wptr<Window>(windowToken_), dragEnabled,
                      env, task = napiAsyncTask, where = __func__] {
        auto window = weakToken.promote();
        if (window == nullptr) {
            TLOGNE(WmsLogTag::WMS_LAYOUT, "%{public}s: window is nullptr", where);
            task->Reject(env, JsErrUtils::CreateJsError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
                "[window][setResizeByDragEnabled]msg: Window is nullptr"));
            return;
        }
        WmErrorCode ret = WM_JS_TO_ERROR_CODE_MAP.at(window->SetResizeByDragEnabled(dragEnabled));
        if (ret == WmErrorCode::WM_OK) {
            task->Resolve(env, NapiGetUndefined(env));
        } else {
            task->Reject(env, JsErrUtils::CreateJsError(env, ret));
        }
        TLOGNE(WmsLogTag::WMS_LAYOUT, "%{public}s: Window [%{public}u, %{public}s] set dragEnabled end",
               where, window->GetWindowId(), window->GetWindowName().c_str());
    };
    if (napi_send_event(env, asyncTask, napi_eprio_high, "OnSetResizeByDragEnabled") != napi_status::napi_ok) {
        napiAsyncTask->Reject(env,
            JsErrUtils::CreateJsError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
                "[window][setResizeByDragEnabled]msg: Failed to send event"));
    }
    return result;
}

napi_value JsWindow::OnSetRaiseByClickEnabled(napi_env env, napi_callback_info info)
{
    WMError errCode = WMError::WM_OK;
    size_t argc = 4;
    napi_value argv[4] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc < 1 || argc > 2) { // 2: maximum params num
        TLOGE(WmsLogTag::WMS_HIERARCHY, "Argc is invalid: %{public}zu", argc);
        errCode = WMError::WM_ERROR_INVALID_PARAM;
    }
    bool raiseEnabled = true;
    if (!ConvertFromJsValue(env, argv[0], raiseEnabled)) {
        TLOGE(WmsLogTag::WMS_HIERARCHY, "Failed to convert parameter to raiseEnabled");
        errCode = WMError::WM_ERROR_INVALID_PARAM;
    }

    napi_value lastParam = (argc <= 1) ? nullptr : (GetType(env, argv[1]) == napi_function ? argv[1] : nullptr);
    napi_value result = nullptr;
    std::shared_ptr<NapiAsyncTask> napiAsyncTask = CreateEmptyAsyncTask(env, lastParam, &result);
    auto asyncTask =
        [weakToken = wptr<Window>(windowToken_), raiseEnabled, env, errCode, task = napiAsyncTask, where = __func__] {
            auto window = weakToken.promote();
            if (window == nullptr) {
                TLOGNE(WmsLogTag::WMS_HIERARCHY, "%{public}s: window is nullptr", where);
                task->Reject(env, JsErrUtils::CreateJsError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY));
                return;
            }
            if (errCode != WMError::WM_OK) {
                WmErrorCode wmErrorCode = WM_JS_TO_ERROR_CODE_MAP.at(errCode);
                task->Reject(env, JsErrUtils::CreateJsError(env, wmErrorCode, "Invalidate params."));
                return;
            }
            WmErrorCode ret = WM_JS_TO_ERROR_CODE_MAP.at(window->SetRaiseByClickEnabled(raiseEnabled));
            if (ret == WmErrorCode::WM_OK) {
                task->Resolve(env, NapiGetUndefined(env));
            } else {
                task->Reject(env, JsErrUtils::CreateJsError(env, ret, "set raiseEnabled failed"));
            }
            TLOGNI(WmsLogTag::WMS_HIERARCHY, "%{public}s: Window [%{public}u, %{public}s] set raiseEnabled end",
                where, window->GetWindowId(), window->GetWindowName().c_str());
        };
    if (napi_send_event(env, asyncTask, napi_eprio_high, "OnSetRaiseByClickEnabled") != napi_status::napi_ok) {
        napiAsyncTask->Reject(env,
            JsErrUtils::CreateJsError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY, "failed to send event"));
    }
    return result;
}

napi_value JsWindow::OnSetMainWindowRaiseByClickEnabled(napi_env env, napi_callback_info info)
{
    WMError errCode = WMError::WM_OK;
    size_t argc = ONE_PARAMS_SIZE;
    napi_value argv[ONE_PARAMS_SIZE] = { nullptr };
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc != ARG_COUNT_ONE) {
        TLOGE(WmsLogTag::WMS_HIERARCHY, "Argc is invalid: %{public}zu", argc);
        errCode = WMError::WM_ERROR_INVALID_PARAM;
    }
    bool raiseEnabled = true;
    if (!ConvertFromJsValue(env, argv[0], raiseEnabled)) {
        TLOGE(WmsLogTag::WMS_HIERARCHY, "Failed to convert parameter to raiseEnabled");
        errCode = WMError::WM_ERROR_INVALID_PARAM;
    }
    if (!Permission::IsSystemCallingOrStartByHdcd(true)) {
        TLOGE(WmsLogTag::WMS_HIERARCHY, "permission denied, require system application");
        napi_throw(env, JsErrUtils::CreateJsError(env, WmErrorCode::WM_ERROR_NOT_SYSTEM_APP,
            "[window][setMainWindowRaiseByClickEnabled]msg: Permission denied."));
        return NapiGetUndefined(env);
    }
    napi_value result = nullptr;
    std::shared_ptr<NapiAsyncTask> napiAsyncTask = CreateEmptyAsyncTask(env, nullptr, &result);
    auto asyncTask =
        [weakToken = wptr<Window>(windowToken_), raiseEnabled, env, errCode, task = napiAsyncTask, where = __func__] {
            auto window = weakToken.promote();
            if (window == nullptr) {
                TLOGNE(WmsLogTag::WMS_HIERARCHY, "%{public}s: window is nullptr", where);
                task->Reject(env, JsErrUtils::CreateJsError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY));
                return;
            }
            if (errCode != WMError::WM_OK) {
                WmErrorCode wmErrorCode = WM_JS_TO_ERROR_CODE_MAP.at(errCode);
                task->Reject(env, JsErrUtils::CreateJsError(env, wmErrorCode,
                    "[window][setMainWindowRaiseByClickEnabled]msg: Invalidate params."));
                return;
            }
            WmErrorCode ret = WM_JS_TO_ERROR_CODE_MAP.at(window->SetMainWindowRaiseByClickEnabled(raiseEnabled));
            if (ret == WmErrorCode::WM_OK) {
                task->Resolve(env, NapiGetUndefined(env));
            } else {
                task->Reject(env, JsErrUtils::CreateJsError(env, ret,
                    "[window][setMainWindowRaiseByClickEnabled]msg: set raiseEnabled failed"));
            }
            TLOGNI(WmsLogTag::WMS_HIERARCHY, "%{public}s: main window %{public}u set raiseEnabled : %{public}d",
                where, window->GetWindowId(), raiseEnabled);
        };
    if (napi_send_event(env, asyncTask, napi_eprio_high, "SetMainWindowRaiseByClickEnabled") != napi_status::napi_ok) {
        napiAsyncTask->Reject(env, JsErrUtils::CreateJsError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
            "[window][setMainWindowRaiseByClickEnabled]msg: failed to send event"));
    }
    return result;
}

napi_value JsWindow::OnHideNonSystemFloatingWindows(napi_env env, napi_callback_info info)
{
    size_t argc = FOUR_PARAMS_SIZE;
    napi_value argv[FOUR_PARAMS_SIZE] = { nullptr };
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc < ONE_PARAMS_SIZE || argc > TWO_PARAMS_SIZE) { // 2: maximum params num
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "Argc is invalid: %{public}zu", argc);
        return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM);
    }
    bool shouldHide = false;
    if (!ConvertFromJsValue(env, argv[INDEX_ZERO], shouldHide)) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "Failed to convert parameter to shouldHide");
        return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM);
    }
    napi_value lastParam = (argc <= ONE_PARAMS_SIZE) ? nullptr :
        (GetType(env, argv[INDEX_ONE]) == napi_function ? argv[INDEX_ONE] : nullptr);
    napi_value result = nullptr;
    std::shared_ptr<NapiAsyncTask> napiAsyncTask = CreateEmptyAsyncTask(env, lastParam, &result);
    auto asyncTask = [weakToken = wptr<Window>(windowToken_), shouldHide, env,
        task = napiAsyncTask, where = __func__] {
        auto window = weakToken.promote();
        if (window == nullptr) {
            TLOGNE(WmsLogTag::WMS_ATTRIBUTE, "%{public}s window is nullptr", where);
            task->Reject(env, JsErrUtils::CreateJsError(env,
                WmErrorCode::WM_ERROR_STATE_ABNORMALLY, "[window][hideNonSystemFloatingWindows]msg: invalid window"));
            return;
        }
        if (window->IsFloatingWindowAppType()) {
            TLOGNE(WmsLogTag::WMS_ATTRIBUTE, "%{public}s window is app floating window", where);
            task->Reject(env, JsErrUtils::CreateJsError(env, WmErrorCode::WM_ERROR_INVALID_CALLING,
                "[window][hideNonSystemFloatingWindows]msg: current window is an app window"));
            return;
        }
        WMError ret = window->HideNonSystemFloatingWindows(shouldHide);
        if (ret != WMError::WM_OK) {
            TLOGNE(WmsLogTag::WMS_ATTRIBUTE, "%{public}s failed", where);
            task->Reject(env, JsErrUtils::CreateJsError(env, WM_JS_TO_ERROR_CODE_MAP.at(ret),
                "[window][hideNonSystemFloatingWindows]msg: hide non-system floating windows failed"));
            return;
        }
        task->Resolve(env, NapiGetUndefined(env));
        TLOGNI(WmsLogTag::WMS_ATTRIBUTE,
            "%{public}s end. Wid: %{public}u, shouldHide: %{public}d",
            where, window->GetWindowId(), shouldHide);
    };
    if (napi_send_event(env, asyncTask, napi_eprio_high, "OnHideNonSystemFloatingWindows") != napi_status::napi_ok) {
        napiAsyncTask->Reject(env,
            JsErrUtils::CreateJsError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
                "[window][hideNonSystemFloatingWindows]msg: send event failed"));
    }
    return result;
}

napi_value JsWindow::OnSetSingleFrameComposerEnabled(napi_env env, napi_callback_info info)
{
    WmErrorCode errCode = WmErrorCode::WM_OK;
    if (!Permission::IsSystemCalling() && !Permission::IsStartByHdcd()) {
        WLOGFE("set single frame composer enabled permission denied!");
        errCode = WmErrorCode::WM_ERROR_NOT_SYSTEM_APP;
    }

    bool enabled = false;
    if (errCode == WmErrorCode::WM_OK) {
        size_t argc = 4;
        napi_value argv[4] = {nullptr};
        napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
        if (argc != 1) { // 1: the param num
            WLOGFE("Invalid parameter, argc is invalid: %{public}zu", argc);
            errCode = WmErrorCode::WM_ERROR_INVALID_PARAM;
        }
        if (errCode == WmErrorCode::WM_OK) {
            napi_value nativeVal = argv[0];
            if (nativeVal == nullptr) {
                WLOGFE("Invalid parameter, failed to convert parameter to enabled");
                errCode = WmErrorCode::WM_ERROR_INVALID_PARAM;
            } else {
                CHECK_NAPI_RETCODE(errCode, WmErrorCode::WM_ERROR_INVALID_PARAM,
                    napi_get_value_bool(env, nativeVal, &enabled));
            }
        }
    }

    const char* const where = __func__;
    napi_value lastParam = nullptr;
    napi_value result = nullptr;
    std::shared_ptr<NapiAsyncTask> napiAsyncTask = CreateEmptyAsyncTask(env, lastParam, &result);
    auto asyncTask = [weakToken = wptr<Window>(windowToken_), enabled, errCode, where, env, task = napiAsyncTask] {
        if (errCode != WmErrorCode::WM_OK) {
            task->Reject(env, JsErrUtils::CreateJsError(env, errCode,
                "[window][setSingleFrameComposerEnabled]msg: permission denied or invalid parameter"));
            return;
        }

        auto weakWindow = weakToken.promote();
        WmErrorCode wmErrorCode;
        if (weakWindow == nullptr) {
            WLOGFE("window is nullptr");
            wmErrorCode = WM_JS_TO_ERROR_CODE_MAP.at(WMError::WM_ERROR_NULLPTR);
            task->Reject(env, JsErrUtils::CreateJsError(env, wmErrorCode,
                "[window][setSingleFrameComposerEnabled]msg: invalid window"));
            return;
        }

        WMError ret = weakWindow->SetSingleFrameComposerEnabled(enabled);
        if (ret == WMError::WM_OK) {
            task->Resolve(env, NapiGetUndefined(env));
        } else {
            wmErrorCode = WM_JS_TO_ERROR_CODE_MAP.at(ret);
            WLOGFE("Set single frame composer enabled failed, ret is %{public}d", wmErrorCode);
            task->Reject(env, JsErrUtils::CreateJsError(env, wmErrorCode,
                        "[window][setSingleFrameComposerEnabled]msg: set single frame composer enabled failed"));
            return;
        }
        WLOGI("%{public}s end, window [%{public}u, %{public}s] enabled flag=%{public}d",
            where, weakWindow->GetWindowId(), weakWindow->GetWindowName().c_str(), enabled);
    };
    if (napi_send_event(env, asyncTask, napi_eprio_high, "OnSetSingleFrameComposerEnabled") != napi_status::napi_ok) {
        TLOGE(WmsLogTag::WMS_IMMS, "napi_send_event failed");
        napiAsyncTask->Reject(env,
            JsErrUtils::CreateJsError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
                "[window][setSingleFrameComposerEnabled]msg: failed to send event"));
    }
    return result;
}

void GetSubWindowId(napi_env env, napi_value nativeVal, WmErrorCode& errCode, int32_t& subWindowId)
{
    if (nativeVal == nullptr) {
        WLOGFE("Failed to get subWindowId");
        errCode = WmErrorCode::WM_ERROR_INVALID_PARAM;
    } else {
        int32_t resultValue = 0;
        CHECK_NAPI_RETCODE(errCode, WmErrorCode::WM_ERROR_INVALID_PARAM,
            napi_get_value_int32(env, nativeVal, &resultValue));
        if (resultValue <= 0) {
            WLOGFE("Failed to get subWindowId due to resultValue less than or equal to 0");
            errCode = WmErrorCode::WM_ERROR_INVALID_PARAM;
        } else {
            subWindowId = resultValue;
        }
    }
    return;
}

napi_value JsWindow::OnRaiseAboveTarget(napi_env env, napi_callback_info info)
{
    WmErrorCode errCode = WmErrorCode::WM_OK;
    size_t argc = 4;
    napi_value argv[4] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc < 1 || argc > 2) { // 2: maximum params num
        TLOGE(WmsLogTag::WMS_HIERARCHY, "Argc is invalid: %{public}zu", argc);
        return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM);
    }
    int32_t subWindowId = -1;
    GetSubWindowId(env, argv[0], errCode, subWindowId);
    if (errCode == WmErrorCode::WM_ERROR_INVALID_PARAM) {
        return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM);
    }

    napi_value lastParam = (argc <= 1) ? nullptr : (GetType(env, argv[1]) == napi_function ? argv[1] : nullptr);
    napi_value result = nullptr;
    std::shared_ptr<NapiAsyncTask> napiAsyncTask = CreateEmptyAsyncTask(env, lastParam, &result);
    auto asyncTask =
        [weakToken = wptr<Window>(windowToken_), subWindowId, errCode, env, task = napiAsyncTask, where = __func__] {
            auto window = weakToken.promote();
            if (window == nullptr) {
                task->Reject(env, JsErrUtils::CreateJsError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
                    "[window][raiseAboveTarget]msg: Window is nullptr"));
                return;
            }
            if (errCode != WmErrorCode::WM_OK) {
                task->Reject(env, JsErrUtils::CreateJsError(env, errCode,
                    "[window][raiseAboveTarget]msg: Invalidate params."));
                return;
            }
            WmErrorCode ret = WM_JS_TO_ERROR_CODE_MAP.at(window->RaiseAboveTarget(subWindowId));
            if (ret == WmErrorCode::WM_OK) {
                task->Resolve(env, NapiGetUndefined(env));
            } else {
                task->Reject(env, JsErrUtils::CreateJsError(env, ret,
                    "[window][raiseAboveTarget]msg: Window set raiseAboveTarget failed"));
            }
        };
    if (napi_send_event(env, asyncTask, napi_eprio_high, "OnRaiseAboveTarget") != napi_status::napi_ok) {
        napiAsyncTask->Reject(env,
            JsErrUtils::CreateJsError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
                "[window][raiseAboveTarget]msg: Failed to send event"));
    }
    return result;
}

WmErrorCode JsWindow::CheckRaiseMainWindowParams(napi_env env, size_t argc, napi_value argv[],
                                                 int32_t sourceId, int32_t& targetId)
{
    if (argc != ONE_PARAMS_SIZE || argv[0] == nullptr) {
        TLOGE(WmsLogTag::WMS_HIERARCHY, "argc is invalid: %{public}zu", argc);
        return WmErrorCode::WM_ERROR_ILLEGAL_PARAM;
    }
    if (!ConvertFromJsValue(env, argv[INDEX_ZERO], targetId)) {
        TLOGE(WmsLogTag::WMS_HIERARCHY, "failed to convert parameter to target window id");
        return WmErrorCode::WM_ERROR_ILLEGAL_PARAM;
    }
    if (targetId <= static_cast<int32_t>(INVALID_WINDOW_ID) || targetId == sourceId) {
        TLOGE(WmsLogTag::WMS_HIERARCHY, "target window id is invalid or equals to source window id");
        return WmErrorCode::WM_ERROR_ILLEGAL_PARAM;
    }
    return WmErrorCode::WM_OK;
}

napi_value JsWindow::OnRaiseMainWindowAboveTarget(napi_env env, napi_callback_info info)
{
    WmErrorCode errCode = WmErrorCode::WM_OK;
    if (!Permission::IsSystemCallingOrStartByHdcd(true)) {
        TLOGE(WmsLogTag::WMS_HIERARCHY, "permission denied, require system application");
        errCode = WmErrorCode::WM_ERROR_NOT_SYSTEM_APP;
    }
    size_t argc = FOUR_PARAMS_SIZE;
    napi_value argv[FOUR_PARAMS_SIZE] = { nullptr };
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    int32_t targetId = static_cast<int32_t>(INVALID_WINDOW_ID);
    errCode == WmErrorCode::WM_OK ?
        CheckRaiseMainWindowParams(env, argc, argv, windowToken_->GetWindowId(), targetId) : errCode;
    napi_value lastParam = nullptr;
    napi_value result = nullptr;
    std::shared_ptr<NapiAsyncTask> napiAsyncTask = CreateEmptyAsyncTask(env, lastParam, &result);
    auto asyncTask = [weakToken = wptr<Window>(windowToken_), targetId, env, task = napiAsyncTask, errCode] {
        if (errCode != WmErrorCode::WM_OK) {
            task->Reject(env, JsErrUtils::CreateJsError(env, errCode,
                "[window][raiseMainWindowAboveTarget]msg: Invalidate params."));
            return;
        }
        auto window = weakToken.promote();
        if (window == nullptr) {
            TLOGNE(WmsLogTag::WMS_HIERARCHY, "window is nullptr");
            task->Reject(env, JsErrUtils::CreateJsError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
                "[window][raiseMainWindowAboveTarget]msg: Window is nullptr"));
            return;
        }
        WmErrorCode ret = WM_JS_TO_ERROR_CODE_MAP.at(window->RaiseMainWindowAboveTarget(targetId));
        if (ret == WmErrorCode::WM_OK) {
            task->Resolve(env, NapiGetUndefined(env));
        } else {
            task->Reject(env, JsErrUtils::CreateJsError(env, ret,
                "[window][raiseMainWindowAboveTarget]msg: Raise main window above target failed"));
        }
        TLOGNI(WmsLogTag::WMS_HIERARCHY,
               "source window: %{public}u, target window: %{public}u, ret = %{public}d",
               window->GetWindowId(), targetId, ret);
    };
    if (napi_send_event(env, asyncTask, napi_eprio_high, "OnRaiseMainWindowAboveTarget") != napi_status::napi_ok) {
        napiAsyncTask->Reject(
            env, JsErrUtils::CreateJsError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
                "[window][raiseMainWindowAboveTarget]msg: Failed to send event"));
    }
    return result;
}

napi_value JsWindow::OnKeepKeyboardOnFocus(napi_env env, napi_callback_info info)
{
    size_t argc = 4;
    napi_value argv[4] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc < 1) {
        WLOGFE("Argc is invalid: %{public}zu", argc);
        return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM,
            "[window][OnKeepKeyboardOnFocus]msg:argc is invalid");
    }
    bool keepKeyboardFlag = false;
    napi_value nativeVal = argv[0];
    if (nativeVal == nullptr) {
        WLOGFE("Failed to get parameter keepKeyboardFlag");
        return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM,
            "[window][OnKeepKeyboardOnFocus]msg:nativeVal is null");
    } else {
        WmErrorCode errCode = WmErrorCode::WM_OK;
        CHECK_NAPI_RETCODE(errCode, WmErrorCode::WM_ERROR_INVALID_PARAM,
            napi_get_value_bool(env, nativeVal, &keepKeyboardFlag));
        if (errCode == WmErrorCode::WM_ERROR_INVALID_PARAM) {
            return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM,
                "[window][OnKeepKeyboardOnFocus]msg:invalid nativeVal");
        }
    }

    if (windowToken_ == nullptr) {
        WLOGFE("WindowToken_ is nullptr");
        return NapiThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
            "[window][OnKeepKeyboardOnFocus]msg:windowToken_ is null");
    }
    if (!WindowHelper::IsSystemWindow(windowToken_->GetType()) &&
        !WindowHelper::IsSubWindow(windowToken_->GetType())) {
        WLOGFE("not allowed since window is not system window or app subwindow");
        return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_CALLING,
            "[window][OnKeepKeyboardOnFocus]msg:windowType is invalid");
    }

    WmErrorCode ret = windowToken_->KeepKeyboardOnFocus(keepKeyboardFlag);
    if (ret != WmErrorCode::WM_OK) {
        WLOGFE("failed");
        return NapiThrowError(env, ret,
            "[window][OnKeepKeyboardOnFocus]msg:failed");
    }

    WLOGFI("end, window [%{public}u, %{public}s] keepKeyboardFlag=%{public}d",
        windowToken_->GetWindowId(), windowToken_->GetWindowName().c_str(), keepKeyboardFlag);

    return NapiGetUndefined(env);
}

napi_value JsWindow::OnSetWindowTouchable(napi_env env, napi_callback_info info)
{
    WmErrorCode errCode = WmErrorCode::WM_OK;
    size_t argc = 4;
    napi_value argv[4] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc < 1) { // 1: params num
        WLOGFE("Argc is invalid: %{public}zu", argc);
        return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM,
            "[window][setWindowTouchable]msg: Mandatory parameters are left unspecified");
    }
    bool touchable = true;
    napi_value nativeVal = argv[0];
    if (nativeVal == nullptr) {
        WLOGFE("Failed to convert parameter to touchable");
        errCode = WmErrorCode::WM_ERROR_INVALID_PARAM;
    } else {
        CHECK_NAPI_RETCODE(errCode, WmErrorCode::WM_ERROR_INVALID_PARAM,
            napi_get_value_bool(env, nativeVal, &touchable));
    }
    if (errCode == WmErrorCode::WM_ERROR_INVALID_PARAM) {
        return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM,
            "[window][setWindowTouchable]msg: Incorrect parameter types");
    }

    wptr<Window> weakToken(windowToken_);
    napi_value lastParam = (argc <= 1) ? nullptr :
        ((argv[1] != nullptr && GetType(env, argv[1]) == napi_function) ? argv[1] : nullptr);
    napi_value result = nullptr;
    std::shared_ptr<NapiAsyncTask> napiAsyncTask = CreateEmptyAsyncTask(env, lastParam, &result);
    auto asyncTask = [weakToken, touchable, env, task = napiAsyncTask] {
        auto weakWindow = weakToken.promote();
        if (weakWindow == nullptr) {
            task->Reject(env,
                JsErrUtils::CreateJsError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
                    "[window][setWindowTouchable]msg: The window is not created or destroyed"));
            return;
        }
        WmErrorCode ret = WM_JS_TO_ERROR_CODE_MAP.at(weakWindow->SetTouchable(touchable));
        ret == WmErrorCode::WM_OK ? task->Resolve(env, NapiGetUndefined(env)) :
            task->Reject(env, JsErrUtils::CreateJsError(env, ret, "[window][setWindowTouchable]"));
        WLOGI("Window [%{public}u, %{public}s] set touchable end",
            weakWindow->GetWindowId(), weakWindow->GetWindowName().c_str());
    };
    if (napi_send_event(env, asyncTask, napi_eprio_high, "OnSetWindowTouchable") != napi_status::napi_ok) {
        napiAsyncTask->Reject(env, JsErrUtils::CreateJsError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
            "[window][setWindowTouchable]msg: Internal task error"));
    }
    return result;
}

napi_value JsWindow::OnSetTransparent(napi_env env, napi_callback_info info)
{
    WMError errCode = WMError::WM_OK;
    size_t argc = 4;
    napi_value argv[4] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc < 1 || argc > 2) { // 2: maximum params num
        WLOGFE("Argc is invalid: %{public}zu", argc);
        errCode = WMError::WM_ERROR_INVALID_PARAM;
    }
    bool isTransparent = true;
    if (errCode == WMError::WM_OK) {
        napi_value nativeVal = argv[0];
        if (nativeVal == nullptr) {
            WLOGFE("Failed to convert parameter to isTransparent");
            errCode = WMError::WM_ERROR_INVALID_PARAM;
        } else {
            CHECK_NAPI_RETCODE(errCode, WMError::WM_ERROR_INVALID_PARAM,
                napi_get_value_bool(env, nativeVal, &isTransparent));
        }
    }

    napi_value lastParam = (argc <= 1) ? nullptr :
        (GetType(env, argv[1]) == napi_function ? argv[1] : nullptr);
    napi_value result = nullptr;
    std::shared_ptr<NapiAsyncTask> napiAsyncTask = CreateEmptyAsyncTask(env, lastParam, &result);
    auto asyncTask = [env, task = napiAsyncTask, weakToken = wptr<Window>(windowToken_), isTransparent, errCode] {
        auto weakWindow = weakToken.promote();
        if (weakWindow == nullptr) {
            WLOGFE("window is nullptr");
            task->Reject(env, JsErrUtils::CreateJsError(env, WMError::WM_ERROR_NULLPTR));
            return;
        }
        if (errCode != WMError::WM_OK) {
            task->Reject(env, JsErrUtils::CreateJsError(env, errCode, "Invalidate params."));
            return;
        }
        WMError ret = weakWindow->SetTransparent(isTransparent);
        if (ret == WMError::WM_OK) {
            task->Resolve(env, NapiGetUndefined(env));
        } else {
            task->Reject(env, JsErrUtils::CreateJsError(env, ret, "Window set transparent failed"));
        }
        WLOGI("Window [%{public}u, %{public}s] set transparent end",
            weakWindow->GetWindowId(), weakWindow->GetWindowName().c_str());
    };
    if (napi_send_event(env, asyncTask, napi_eprio_high, "OnSetTransparent") != napi_status::napi_ok) {
        napiAsyncTask->Reject(env,
            JsErrUtils::CreateJsError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY, "failed to send event"));
    }
    return result;
}

napi_value JsWindow::OnChangeCallingWindowId(napi_env env, napi_callback_info info)
{
    WMError errCode = WMError::WM_OK;
    size_t argc = 4;
    napi_value argv[4] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc < 1 || argc > 2) { // 2: maximum params num
        TLOGE(WmsLogTag::WMS_LIFE, "Argc is invalid: %{public}zu", argc);
        errCode = WMError::WM_ERROR_INVALID_PARAM;
    }
    uint32_t callingWindow = INVALID_WINDOW_ID;
    if (errCode == WMError::WM_OK) {
        napi_value nativeVal = argv[0];
        if (nativeVal == nullptr) {
            TLOGE(WmsLogTag::WMS_LIFE, "Failed to convert parameter to touchable");
            errCode = WMError::WM_ERROR_INVALID_PARAM;
        } else {
            napi_get_value_uint32(env, nativeVal, &callingWindow);
        }
    }

    wptr<Window> weakToken(windowToken_);
    napi_value lastParam = (argc <= 1) ? nullptr : (GetType(env, argv[1]) == napi_function ? argv[1] : nullptr);
    napi_value result = nullptr;
    std::shared_ptr<NapiAsyncTask> napiAsyncTask = CreateEmptyAsyncTask(env, lastParam, &result);
    auto asyncTask = [weakToken, callingWindow, errCode, env, task = napiAsyncTask] {
        auto weakWindow = weakToken.promote();
        if (weakWindow == nullptr) {
            TLOGNE(WmsLogTag::WMS_LIFE, "window is nullptr");
            task->Reject(env, JsErrUtils::CreateJsError(env, WMError::WM_ERROR_NULLPTR));
            return;
        }
        if (errCode != WMError::WM_OK) {
            task->Reject(env, JsErrUtils::CreateJsError(env, errCode, "Invalidate params."));
            return;
        }
        WMError ret = weakWindow->ChangeCallingWindowId(callingWindow);
        if (ret == WMError::WM_OK) {
            task->Resolve(env, NapiGetUndefined(env));
        } else {
            task->Reject(env, JsErrUtils::CreateJsError(env, ret, "Window set calling window failed"));
        }
        TLOGNI(WmsLogTag::WMS_LIFE, "Window [%{public}u, %{public}s] set calling window end",
            weakWindow->GetWindowId(), weakWindow->GetWindowName().c_str());
    };
    if (napi_send_event(env, asyncTask, napi_eprio_high, "OnChangeCallingWindowId") != napi_status::napi_ok) {
        TLOGE(WmsLogTag::WMS_LIFE, "napi send event failed, window state is abnormal");
    }
    return result;
}

napi_value JsWindow::OnDisableWindowDecor(napi_env env, napi_callback_info info)
{
    if (windowToken_ == nullptr) {
        return NapiThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
            "[window][disableWindowDecor]msg: windowToken is nullptr");
    }
    WmErrorCode ret = WM_JS_TO_ERROR_CODE_MAP.at(windowToken_->DisableAppWindowDecor());
    if (ret != WmErrorCode::WM_OK) {
        TLOGE(WmsLogTag::WMS_DECOR, "Window DisableWindowDecor failed");
        return NapiThrowError(env, ret);
    }
    TLOGI(WmsLogTag::WMS_DECOR, "Window [%{public}u, %{public}s] end",
        windowToken_->GetWindowId(), windowToken_->GetWindowName().c_str());
    return NapiGetUndefined(env);
}

napi_value JsWindow::OnSetColorSpace(napi_env env, napi_callback_info info)
{
    WMError errCode = WMError::WM_OK;
    ColorSpace colorSpace = ColorSpace::COLOR_SPACE_DEFAULT;
    size_t argc = 4;
    napi_value argv[4] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc < 1 || argc > 2) { // 2: maximum params num
        WLOGFE("Argc is invalid: %{public}zu", argc);
        errCode = WMError::WM_ERROR_INVALID_PARAM;
    } else {
        napi_value nativeType = argv[0];
        if (nativeType == nullptr) {
            errCode = WMError::WM_ERROR_INVALID_PARAM;
            WLOGFE("Failed to convert parameter to ColorSpace");
        } else {
            uint32_t resultValue = 0;
            CHECK_NAPI_RETCODE(errCode, WMError::WM_ERROR_INVALID_PARAM,
                napi_get_value_uint32(env, nativeType, &resultValue));
            colorSpace = static_cast<ColorSpace>(resultValue);
            if (colorSpace > ColorSpace::COLOR_SPACE_WIDE_GAMUT || colorSpace < ColorSpace::COLOR_SPACE_DEFAULT) {
                WLOGFE("ColorSpace %{public}u invalid!", static_cast<uint32_t>(colorSpace));
                errCode = WMError::WM_ERROR_INVALID_PARAM;
            }
        }
    }

    napi_value lastParam = (argc <= 1) ? nullptr :
        (GetType(env, argv[1]) == napi_function ? argv[1] : nullptr);
    napi_value result = nullptr;
    std::shared_ptr<NapiAsyncTask> napiAsyncTask = CreateEmptyAsyncTask(env, lastParam, &result);
    auto asyncTask = [weakToken = wptr<Window>(windowToken_), colorSpace, errCode, env, task = napiAsyncTask] {
        auto weakWindow = weakToken.promote();
        if (weakWindow == nullptr) {
            WLOGFE("window is nullptr");
            task->Reject(env, JsErrUtils::CreateJsError(env, WMError::WM_ERROR_NULLPTR));
            return;
        }
        if (errCode != WMError::WM_OK) {
            task->Reject(env, JsErrUtils::CreateJsError(env, errCode, "OnSetColorSpace failed"));
            WLOGFE("window is nullptr or get invalid param");
            return;
        }
        weakWindow->SetColorSpace(colorSpace);
        task->Resolve(env, NapiGetUndefined(env));
        WLOGI("Window [%{public}u, %{public}s] OnSetColorSpace end, colorSpace=%{public}u",
            weakWindow->GetWindowId(), weakWindow->GetWindowName().c_str(), static_cast<uint32_t>(colorSpace));
    };
    if (napi_send_event(env, asyncTask, napi_eprio_high, "OnSetColorSpace") != napi_status::napi_ok) {
        TLOGE(WmsLogTag::WMS_IMMS, "napi_send_event failed");
        napiAsyncTask->Reject(env,
            JsErrUtils::CreateJsError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY, "failed to send event"));
    }
    return result;
}

napi_value JsWindow::OnSetWindowColorSpace(napi_env env, napi_callback_info info)
{
    WmErrorCode errCode = WmErrorCode::WM_OK;
    ColorSpace colorSpace = ColorSpace::COLOR_SPACE_DEFAULT;
    size_t argc = 4;
    napi_value argv[4] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc < 1) { // 1: params num
        TLOGE(WmsLogTag::WMS_IMMS, "Argc is invalid: %{public}zu", argc);
        return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM,
            "[window][setWindowColorSpace]msg: Mandatory parameters are left unspecified");
    }
    napi_value nativeType = argv[0];
    if (nativeType == nullptr) {
        TLOGE(WmsLogTag::WMS_IMMS, "Failed to convert parameter to ColorSpace");
        return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM,
            "[window][setWindowColorSpace]msg: Incorrect parameter types");
    }
    uint32_t resultValue = 0;
    CHECK_NAPI_RETCODE(errCode, WmErrorCode::WM_ERROR_INVALID_PARAM,
        napi_get_value_uint32(env, nativeType, &resultValue));
    colorSpace = static_cast<ColorSpace>(resultValue);
    if (colorSpace > ColorSpace::COLOR_SPACE_WIDE_GAMUT || colorSpace < ColorSpace::COLOR_SPACE_DEFAULT) {
        TLOGE(WmsLogTag::WMS_IMMS, "ColorSpace %{public}u invalid!", static_cast<uint32_t>(colorSpace));
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
    auto asyncTask = [weakToken = wptr<Window>(windowToken_), colorSpace, where, env, task = napiAsyncTask] {
        auto weakWindow = weakToken.promote();
        if (weakWindow == nullptr) {
            TLOGE(WmsLogTag::WMS_IMMS, "window is nullptr");
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
        napiAsyncTask->Reject(env,
            JsErrUtils::CreateJsError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
                "[window][setWindowColorSpace]msg: Internal task error"));
    }
    return result;
}

napi_value JsWindow::OnGetColorSpace(napi_env env, napi_callback_info info)
{
    WMError errCode = WMError::WM_OK;
    size_t argc = 4;
    napi_value argv[4] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc > 1) {
        WLOGFE("Argc is invalid: %{public}zu", argc);
        errCode = WMError::WM_ERROR_INVALID_PARAM;
    }
    const char* const where = __func__;

    napi_value lastParam = (argc == 0) ? nullptr :
        (GetType(env, argv[0]) == napi_function ? argv[0] : nullptr);
    napi_value result = nullptr;
    std::shared_ptr<NapiAsyncTask> napiAsyncTask = CreateEmptyAsyncTask(env, lastParam, &result);
    auto asyncTask = [weakToken = wptr<Window>(windowToken_), errCode, where, env, task = napiAsyncTask] {
        auto weakWindow = weakToken.promote();
        if (weakWindow == nullptr) {
            WLOGFE("window is nullptr");
            task->Reject(env, JsErrUtils::CreateJsError(env, WMError::WM_ERROR_NULLPTR));
            return;
        }
        if (errCode != WMError::WM_OK) {
            task->Reject(env, JsErrUtils::CreateJsError(env, errCode));
            WLOGFE("window is nullptr or get invalid param");
            return;
        }
        ColorSpace colorSpace = weakWindow->GetColorSpace();
        task->Resolve(env, CreateJsValue(env, static_cast<uint32_t>(colorSpace)));
        WLOGI("%{public}s end, window [%{public}u, %{public}s] colorSpace=%{public}u",
            where, weakWindow->GetWindowId(), weakWindow->GetWindowName().c_str(),
            static_cast<uint32_t>(colorSpace));
    };
    if (napi_send_event(env, asyncTask, napi_eprio_high, "OnGetColorSpace") != napi_status::napi_ok) {
        TLOGE(WmsLogTag::WMS_IMMS, "napi_send_event failed");
        napiAsyncTask->Reject(env,
            JsErrUtils::CreateJsError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY, "failed to send event"));
    }
    return result;
}

napi_value JsWindow::OnGetWindowColorSpaceSync(napi_env env, napi_callback_info info)
{
    if (windowToken_ == nullptr) {
        WLOGFE("window is nullptr");
        return NapiThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
            "[window][getWindowColorSpace]msg: The window is not created or destroyed");
    }
    ColorSpace colorSpace = windowToken_->GetColorSpace();
    WLOGFI("end, window [%{public}u, %{public}s] colorSpace=%{public}u",
        windowToken_->GetWindowId(), windowToken_->GetWindowName().c_str(), static_cast<uint32_t>(colorSpace));

    return CreateJsValue(env, static_cast<uint32_t>(colorSpace));
}

napi_value JsWindow::OnDump(napi_env env, napi_callback_info info)
{
    WLOGFI("[NAPI]");
    size_t argc = 4;
    napi_value argv[4] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc < 1 || argc > 2) { // 2: maximum params num
        WLOGFE("Argc is invalid: %{public}zu", argc);
        return nullptr;
    }
    if (windowToken_ == nullptr) {
        WLOGFE("window is nullptr or get invalid param");
        return nullptr;
    }
    std::vector<std::string> params;
    if (!ConvertNativeValueToVector(env, argv[0], params)) {
        WLOGFE("ConvertNativeValueToVector fail");
        return nullptr;
    }
    std::vector<std::string> dumpInfo;
    windowToken_->DumpInfo(params, dumpInfo);
    napi_value dumpInfoValue = CreateNativeArray(env, dumpInfo);
    WLOGI("Window [%{public}u, %{public}s] dump end",
        windowToken_->GetWindowId(), windowToken_->GetWindowName().c_str());
    return dumpInfoValue;
}

napi_value JsWindow::Snapshot(napi_env env, napi_callback_info info)
{
    WLOGI("Snapshot");
    JsWindow* me = CheckParamsAndGetThis<JsWindow>(env, info);
    return (me != nullptr) ? me->OnSnapshot(env, info) : nullptr;
}

napi_value JsWindow::SnapshotSync(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::WMS_ATTRIBUTE, "[NAPI]");
    JsWindow* me = CheckParamsAndGetThis<JsWindow>(env, info);
    return (me != nullptr) ? me->OnSnapshotSync(env, info) : nullptr;
}

napi_value JsWindow::SnapshotIgnorePrivacy(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::WMS_ATTRIBUTE, "SnapshotIgnorePrivacy");
    JsWindow* me = CheckParamsAndGetThis<JsWindow>(env, info);
    return (me != nullptr) ? me->OnSnapshotIgnorePrivacy(env, info) : nullptr;
}

napi_value JsWindow::OnSetForbidSplitMove(napi_env env, napi_callback_info info)
{
    WmErrorCode errCode = WmErrorCode::WM_OK;
    size_t argc = 4;
    napi_value argv[4] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc < 1) { // 1: params num
        errCode = WmErrorCode::WM_ERROR_INVALID_PARAM;
    }
    bool isForbidSplitMove = false;
    if (errCode == WmErrorCode::WM_OK) {
        if (argv[0] == nullptr) {
            errCode = WmErrorCode::WM_ERROR_INVALID_PARAM;
        } else {
            CHECK_NAPI_RETCODE(errCode, WmErrorCode::WM_ERROR_INVALID_PARAM,
                napi_get_value_bool(env, argv[0], &isForbidSplitMove));
        }
    }
    if (errCode == WmErrorCode::WM_ERROR_INVALID_PARAM) {
        return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM,
            "[window][setForbidSplitMove]msg: Invalid param");
    }
    napi_value lastParam = (argc <= 1) ? nullptr :
        ((argv[1] != nullptr && GetType(env, argv[1]) == napi_function) ? argv[1] : nullptr);
    napi_value result = nullptr;
    std::shared_ptr<NapiAsyncTask> napiAsyncTask = CreateEmptyAsyncTask(env, lastParam, &result);
    auto asyncTask = [weakToken = wptr<Window>(windowToken_), task = napiAsyncTask, isForbidSplitMove, env] {
        auto window = weakToken.promote();
        if (window == nullptr) {
            task->Reject(env, JsErrUtils::CreateJsError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
                "[window][setForbidSplitMove]msg: Window is nullptr"));
            return;
        }
        WmErrorCode ret;
        if (isForbidSplitMove) {
            ret = WM_JS_TO_ERROR_CODE_MAP.at(
                window->AddWindowFlag(WindowFlag::WINDOW_FLAG_FORBID_SPLIT_MOVE));
        } else {
            ret = WM_JS_TO_ERROR_CODE_MAP.at(
                window->RemoveWindowFlag(WindowFlag::WINDOW_FLAG_FORBID_SPLIT_MOVE));
        }
        if (ret == WmErrorCode::WM_OK) {
            task->Resolve(env, NapiGetUndefined(env));
        } else {
            task->Reject(env, JsErrUtils::CreateJsError(env, ret, "[window][setForbidSplitMove]msg: Failed"));
        }
    };
    if (napi_send_event(env, asyncTask, napi_eprio_high, "OnSetForbidSplitMove") != napi_status::napi_ok) {
        napiAsyncTask->Reject(env,
            JsErrUtils::CreateJsError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
                "[window][setForbidSplitMove]msg: Failed to send event"));
    }
    return result;
}

napi_value JsWindow::OnSnapshot(napi_env env, napi_callback_info info)
{
    size_t argc = 4;
    napi_value argv[4] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    napi_value lastParam = (argc == 0) ? nullptr :
        ((argv[0] != nullptr && GetType(env, argv[0]) == napi_function) ? argv[0] : nullptr);
    napi_value result = nullptr;
    std::shared_ptr<NapiAsyncTask> napiAsyncTask = CreateEmptyAsyncTask(env, lastParam, &result);
    auto asyncTask = [weakToken = wptr<Window>(windowToken_), env, task = napiAsyncTask] {
        auto weakWindow = weakToken.promote();
        if (weakWindow == nullptr) {
            WLOGFE("window is nullptr");
            task->Reject(env, JsErrUtils::CreateJsError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
                "[window][snapshot]msg: The window is not created or destroyed"));
            return;
        }

        std::shared_ptr<Media::PixelMap> pixelMap = weakWindow->Snapshot();
        if (pixelMap == nullptr) {
            task->Reject(env, JsErrUtils::CreateJsError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
                "[window][snapshot]msg: Get pixelMap failed"));
            WLOGFE("window snapshot get pixelmap is null");
            return;
        }

        auto nativePixelMap = Media::PixelMapNapi::CreatePixelMap(env, pixelMap);
        if (nativePixelMap == nullptr) {
            WLOGFE("window snapshot get nativePixelMap is null");
        }
        task->Resolve(env, nativePixelMap);
        WLOGI("Window [%{public}u, %{public}s] OnSnapshot, WxH=%{public}dx%{public}d",
            weakWindow->GetWindowId(), weakWindow->GetWindowName().c_str(),
            pixelMap->GetWidth(), pixelMap->GetHeight());
    };
    if (napi_send_event(env, asyncTask, napi_eprio_high, "OnSnapshot") != napi_status::napi_ok) {
        TLOGE(WmsLogTag::WMS_IMMS, "napi_send_event failed");
        napiAsyncTask->Reject(env,
            JsErrUtils::CreateJsError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
                "[window][snapshot]msg: Internal task error"));
    }
    return result;
}

napi_value JsWindow::OnSnapshotSync(napi_env env, napi_callback_info info)
{
    if (windowToken_ == nullptr) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "windowToken is null");
        return NapiThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
            "[window][snapshotSync]msg: The window is not created or destroyed");
    }
    std::shared_ptr<Media::PixelMap> pixelMap = nullptr;
    WmErrorCode ret = WM_JS_TO_ERROR_CODE_MAP.at(windowToken_->Snapshot(pixelMap));
    TLOGI(WmsLogTag::WMS_ATTRIBUTE, "winId: %{public}u snapshot end, result: %{public}d",
        windowToken_->GetWindowId(), ret);
    if (ret != WmErrorCode::WM_OK) {
        return NapiThrowError(env, ret, "[window][snapshotSync]");
    }
    auto nativePixelMap = Media::PixelMapNapi::CreatePixelMap(env, pixelMap);
    if (nativePixelMap == nullptr) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "winId: %{public}u get nativePixelMap is null",
            windowToken_->GetWindowId());
        return NapiThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
            "[window][snapshotSync]msg: Create pixelMap failed");
    }
    return nativePixelMap;
}

napi_value JsWindow::OnSnapshotIgnorePrivacy(napi_env env, napi_callback_info info)
{
    size_t argc = 4;
    napi_value argv[4] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    napi_value result = nullptr;
    std::shared_ptr<NapiAsyncTask> napiAsyncTask = CreateEmptyAsyncTask(env, nullptr, &result);
    auto asyncTask = [weakToken = wptr<Window>(windowToken_), env, task = napiAsyncTask] {
        auto weakWindow = weakToken.promote();
            if (weakWindow == nullptr) {
                TLOGNE(WmsLogTag::WMS_ATTRIBUTE, "window is nullptr");
                task->Reject(env, JsErrUtils::CreateJsError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
                    "[window][snapshotIgnorePrivacy]msg: The window is not created or destroyed"));
                return;
            }

            std::shared_ptr<Media::PixelMap> pixelMap = nullptr;
            WmErrorCode ret = WM_JS_TO_ERROR_CODE_MAP.at(weakWindow->SnapshotIgnorePrivacy(pixelMap));
            if (ret == WmErrorCode::WM_ERROR_DEVICE_NOT_SUPPORT) {
                task->Reject(env, JsErrUtils::CreateJsError(env, ret,
                    "[window][snapshotIgnorePrivacy]"));
                TLOGNE(WmsLogTag::WMS_ATTRIBUTE, "device not support");
                return;
            } else if (ret != WmErrorCode::WM_OK) {
                task->Reject(env, JsErrUtils::CreateJsError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
                    "[window][snapshotIgnorePrivacy]msg: Create pixelMap failed"));
                TLOGNE(WmsLogTag::WMS_ATTRIBUTE, "get pixelmap failed, code:%{public}d", ret);
                return;
            }

            auto nativePixelMap = Media::PixelMapNapi::CreatePixelMap(env, pixelMap);
            if (nativePixelMap == nullptr) {
                task->Reject(env, JsErrUtils::CreateJsError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
                    "[window][snapshotIgnorePrivacy]msg: Create pixelMap failed"));
                TLOGNE(WmsLogTag::WMS_ATTRIBUTE, "get nativePixelMap is null");
                return;
            }
            task->Resolve(env, nativePixelMap);
            TLOGNI(WmsLogTag::WMS_ATTRIBUTE, "windowId:%{public}u, WxH=%{public}dx%{public}d",
                weakWindow->GetWindowId(), pixelMap->GetWidth(), pixelMap->GetHeight());
    };
    if (napi_send_event(env, asyncTask, napi_eprio_high, "OnSnapshotIgnorePrivacy") != napi_status::napi_ok) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "napi_send_event failed");
        napiAsyncTask->Reject(env,
            JsErrUtils::CreateJsError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
                "[window][snapshotIgnorePrivacy]msg: Internal task error"));
    }
    return result;
}

napi_value JsWindow::OnSetSnapshotSkip(napi_env env, napi_callback_info info)
{
    WmErrorCode errCode = WmErrorCode::WM_OK;
    size_t argc = 4;
    napi_value argv[4] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc < 1) { // 1: params num
        WLOGFE(" inbalid param");
        errCode = WmErrorCode::WM_ERROR_INVALID_PARAM;
    }
    bool isSkip = false;
    if (errCode == WmErrorCode::WM_OK) {
        napi_value nativeVal = argv[0];
        if (nativeVal == nullptr) {
            errCode = WmErrorCode::WM_ERROR_INVALID_PARAM;
        } else {
            CHECK_NAPI_RETCODE(errCode, WmErrorCode::WM_ERROR_INVALID_PARAM,
                napi_get_value_bool(env, nativeVal, &isSkip));
        }
    }
    if (errCode == WmErrorCode::WM_ERROR_INVALID_PARAM) {
        return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM,
            "[window][setSnapshotSkip]msg: invalid params");
    }

    if (windowToken_ == nullptr) {
        return NapiThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
            "[window][setSnapshotSkip]msg: invalid window");
    }

    WmErrorCode ret = WM_JS_TO_ERROR_CODE_MAP.at(windowToken_->SetSnapshotSkip(isSkip));
    if (ret != WmErrorCode::WM_OK) {
        WLOGFE("Window SetSnapshotSkip failed");
        return NapiThrowError(env, ret, "[window][setSnapshotSkip]msg: set snapshot skip failed");
    }
    WLOGI("[%{public}u, %{public}s] set snapshotSkip end",
        windowToken_->GetWindowId(), windowToken_->GetWindowName().c_str());

    return NapiGetUndefined(env);
}

napi_value JsWindow::OnRaiseToAppTop(napi_env env, napi_callback_info info)
{
    size_t argc = 4;
    napi_value argv[4] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    napi_value lastParam = (argc == 0) ? nullptr :
        ((argv[0] != nullptr && GetType(env, argv[0]) == napi_function) ? argv[0] : nullptr);
    napi_value result = nullptr;
    std::shared_ptr<NapiAsyncTask> napiAsyncTask = CreateEmptyAsyncTask(env, lastParam, &result);
    auto asyncTask =
        [weakToken = wptr<Window>(windowToken_), env, task = napiAsyncTask, where = __func__] {
            auto window = weakToken.promote();
            if (window == nullptr) {
                TLOGNE(WmsLogTag::WMS_HIERARCHY, "%{public}s: window is nullptr", where);
                task->Reject(env, JsErrUtils::CreateJsError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
                    "[window][raiseToAppTop]msg: Window is nullptr"));
                return;
            }

            WmErrorCode errCode = WM_JS_TO_ERROR_CODE_MAP.at(window->RaiseToAppTop());
            if (errCode != WmErrorCode::WM_OK) {
                TLOGNE(WmsLogTag::WMS_HIERARCHY, "raise window zorder failed");
                task->Reject(env, JsErrUtils::CreateJsError(env, errCode,
                    "[window][raiseToAppTop]msg: Raise window zorder failed"));
                return;
            }
            task->Resolve(env, NapiGetUndefined(env));
            TLOGNI(WmsLogTag::WMS_HIERARCHY, "%{public}s: Window [%{public}u, %{public}s] zorder raise success",
                where, window->GetWindowId(), window->GetWindowName().c_str());
        };
    if (napi_send_event(env, asyncTask, napi_eprio_high, "OnRaiseToAppTop") != napi_status::napi_ok) {
        napiAsyncTask->Reject(env,
            JsErrUtils::CreateJsError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
                "[window][raiseToAppTop]msg: Failed to send event"));
    }
    return result;
}

napi_value JsWindow::OnOpacity(napi_env env, napi_callback_info info)
{
    size_t argc = 4;
    napi_value argv[4] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc < 1) {
        WLOGFE("Argc is invalid: %{public}zu", argc);
        return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM);
    }
    if (windowToken_ == nullptr) {
        WLOGFE("WindowToken_ is nullptr");
        return NapiThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
    }
    if (!WindowHelper::IsSystemWindow(windowToken_->GetType())) {
        WLOGFE("Opacity is not allowed since window is not system window");
        return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_CALLING);
    }
    napi_value nativeVal = argv[0];
    if (nativeVal == nullptr) {
        WLOGFE("Failed to convert parameter to alpha");
        return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM);
    }
    double alpha = 0.0;
    napi_status statusCode = napi_get_value_double(env, nativeVal, &alpha);
    if (statusCode != napi_ok) {
        return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM);
    }
    if (MathHelper::LessNotEqual(alpha, 0.0) || MathHelper::GreatNotEqual(alpha, 1.0)) {
        WLOGFE("alpha should greater than 0 or smaller than 1.0");
        return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM);
    }

    WmErrorCode ret = WM_JS_TO_ERROR_CODE_MAP.at(windowToken_->SetAlpha(alpha));
    if (ret != WmErrorCode::WM_OK) {
        WLOGFE("Window Opacity failed");
        return NapiThrowError(env, ret);
    }
    WLOGFI("end, window [%{public}u, %{public}s] alpha=%{public}f",
        windowToken_->GetWindowId(), windowToken_->GetWindowName().c_str(), alpha);
    return NapiGetUndefined(env);
}

static bool IsPivotValid(double data)
{
    if (MathHelper::LessNotEqual(data, 0.0) || (MathHelper::GreatNotEqual(data, 1.0))) {
        return false;
    }
    return true;
}

static bool IsScaleValid(double data)
{
    if (!MathHelper::GreatNotEqual(data, 0.0)) {
        return false;
    }
    return true;
}

bool JsWindow::ParseScaleOption(napi_env env, napi_value jsObject, Transform& trans)
{
    double data = 0.0f;
    if (ParseJsValue(jsObject, env, "pivotX", data)) {
        if (!IsPivotValid(data)) {
            return false;
        }
        trans.pivotX_ = data;
    }
    if (ParseJsValue(jsObject, env, "pivotY", data)) {
        if (!IsPivotValid(data)) {
            return false;
        }
        trans.pivotY_ = data;
    }
    if (ParseJsValue(jsObject, env, "x", data)) {
        if (!IsScaleValid(data)) {
            return false;
        }
        trans.scaleX_ = data;
    }
    if (ParseJsValue(jsObject, env, "y", data)) {
        if (!IsScaleValid(data)) {
            return false;
        }
        trans.scaleY_ = data;
    }
    return true;
}

napi_value JsWindow::OnScale(napi_env env, napi_callback_info info)
{
    if (!Permission::IsSystemCalling()) {
        TLOGE(WmsLogTag::WMS_SYSTEM, "not system app, permission denied!");
        return NapiThrowError(env, WmErrorCode::WM_ERROR_NOT_SYSTEM_APP);
    }

    size_t argc = 4;
    napi_value argv[4] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc < 1) {
        WLOGFE("Argc is invalid: %{public}zu", argc);
        return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM);
    }
    if (windowToken_ == nullptr) {
        WLOGFE("WindowToken_ is nullptr");
        return NapiThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
    }
    if (!WindowHelper::IsSystemWindow(windowToken_->GetType())) {
        WLOGFE("Scale is not allowed since window is not system window");
        return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_CALLING);
    }
    napi_value nativeObj = argv[0];
    if (nativeObj == nullptr) {
        WLOGFE("Failed to convert object to ScaleOptions");
        return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM);
    }
    auto trans = windowToken_->GetTransform();
    if (!ParseScaleOption(env, nativeObj, trans)) {
        WLOGFE(" PivotX or PivotY should between 0.0 ~ 1.0, scale should greater than 0.0");
        return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM);
    }
    WmErrorCode ret = WM_JS_TO_ERROR_CODE_MAP.at(windowToken_->SetTransform(trans));
    if (ret != WmErrorCode::WM_OK) {
        WLOGFE("Window Scale failed");
        return NapiThrowError(env, ret);
    }
    WLOGI("Window [%{public}u, %{public}s] Scale end",
        windowToken_->GetWindowId(), windowToken_->GetWindowName().c_str());
    WLOGI("scaleX=%{public}f, scaleY=%{public}f, pivotX=%{public}f pivotY=%{public}f",
        trans.scaleX_, trans.scaleY_, trans.pivotX_, trans.pivotY_);
    return NapiGetUndefined(env);
}

bool JsWindow::ParseRotateOption(napi_env env, napi_value jsObject, Transform& trans)
{
    double data = 0.0f;
    if (ParseJsValue(jsObject, env, "pivotX", data)) {
        if (!IsPivotValid(data)) {
            return false;
        }
        trans.pivotX_ = data;
    }
    if (ParseJsValue(jsObject, env, "pivotY", data)) {
        if (!IsPivotValid(data)) {
            return false;
        }
        trans.pivotY_ = data;
    }
    if (ParseJsValue(jsObject, env, "x", data)) {
        trans.rotationX_ = data;
    }
    if (ParseJsValue(jsObject, env, "y", data)) {
        trans.rotationY_ = data;
    }
    if (ParseJsValue(jsObject, env, "z", data)) {
        trans.rotationZ_ = data;
    }
    return true;
}

napi_value JsWindow::OnRotate(napi_env env, napi_callback_info info)
{
    if (!Permission::IsSystemCalling()) {
        TLOGE(WmsLogTag::WMS_SYSTEM, "not system app, permission denied!");
        return NapiThrowError(env, WmErrorCode::WM_ERROR_NOT_SYSTEM_APP);
    }

    size_t argc = 4;
    napi_value argv[4] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc < 1) {
        WLOGFE("Argc is invalid: %{public}zu", argc);
        return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM);
    }
    if (windowToken_ == nullptr) {
        WLOGFE("WindowToken_ is nullptr");
        return NapiThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
    }
    if (!WindowHelper::IsSystemWindow(windowToken_->GetType())) {
        WLOGFE("Rotate is not allowed since window is not system window");
        return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_CALLING);
    }
    napi_value nativeObj = argv[0];
    if (nativeObj == nullptr) {
        WLOGFE("Failed to convert object to RotateOptions");
        return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM);
    }
    // cannot use sync task since next transform base on current transform
    auto trans = windowToken_->GetTransform();
    if (!ParseRotateOption(env, nativeObj, trans)) {
        WLOGFE(" PivotX or PivotY should between 0.0 ~ 1.0");
        return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM);
    }
    WmErrorCode ret = WM_JS_TO_ERROR_CODE_MAP.at(windowToken_->SetTransform(trans));
    if (ret != WmErrorCode::WM_OK) {
        WLOGFE("Window Rotate failed");
        return NapiThrowError(env, ret);
    }
    WLOGI("Window [%{public}u, %{public}s] Rotate end",
        windowToken_->GetWindowId(), windowToken_->GetWindowName().c_str());
    WLOGI("rotateX=%{public}f, rotateY=%{public}f, rotateZ=%{public}f pivotX=%{public}f pivotY=%{public}f",
        trans.rotationX_, trans.rotationY_, trans.rotationZ_, trans.pivotX_, trans.pivotY_);
    return NapiGetUndefined(env);
}

bool JsWindow::ParseTranslateOption(napi_env env, napi_value jsObject, Transform& trans)
{
    double data = 0.0f;
    if (ParseJsValue(jsObject, env, "x", data)) {
        trans.translateX_ = data;
    }
    if (ParseJsValue(jsObject, env, "y", data)) {
        trans.translateY_ = data;
    }
    if (ParseJsValue(jsObject, env, "z", data)) {
        trans.translateZ_ = data;
    }
    return true;
}

napi_value JsWindow::OnTranslate(napi_env env, napi_callback_info info)
{
    if (!Permission::IsSystemCalling()) {
        TLOGE(WmsLogTag::WMS_SYSTEM, "not system app, permission denied!");
        return NapiThrowError(env, WmErrorCode::WM_ERROR_NOT_SYSTEM_APP);
    }

    size_t argc = 4;
    napi_value argv[4] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc < 1) {
        WLOGFE("Argc is invalid: %{public}zu", argc);
        return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM);
    }
    if (windowToken_ == nullptr) {
        WLOGFE("WindowToken_ is nullptr");
        return NapiThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
    }
    if (!WindowHelper::IsSystemWindow(windowToken_->GetType())) {
        WLOGFE("Translate is not allowed since window is not system window");
        return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_CALLING);
    }
    napi_value nativeObj = argv[0];
    if (nativeObj == nullptr) {
        WLOGFE("Failed to convert object to TranslateOptions");
        return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM);
    }
    auto trans = windowToken_->GetTransform();
    if (!ParseTranslateOption(env, nativeObj, trans)) {
        return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM);
    }
    WmErrorCode ret = WM_JS_TO_ERROR_CODE_MAP.at(windowToken_->SetTransform(trans));
    if (ret != WmErrorCode::WM_OK) {
        WLOGFE("Window Translate failed");
        return NapiThrowError(env, ret);
    }
    WLOGI("Window [%{public}u, %{public}s] Translate end, "
        "translateX=%{public}f, translateY=%{public}f, translateZ=%{public}f",
        windowToken_->GetWindowId(), windowToken_->GetWindowName().c_str(),
        trans.translateX_, trans.translateY_, trans.translateZ_);
    return NapiGetUndefined(env);
}

WmErrorCode JsWindow::CreateTransitionController(napi_env env)
{
    if (windowToken_ == nullptr) {
        TLOGE(WmsLogTag::WMS_SYSTEM, "windowToken is nullptr");
        return WmErrorCode::WM_ERROR_STATE_ABNORMALLY;
    }
    if (!WindowHelper::IsSystemWindow(windowToken_->GetType())) {
        TLOGE(WmsLogTag::WMS_SYSTEM, "Unexpected window type:%{public}d", windowToken_->GetType());
        return WmErrorCode::WM_ERROR_INVALID_CALLING;
    }
    napi_value objValue = nullptr;
    napi_create_object(env, &objValue);
    if (objValue == nullptr) {
        TLOGE(WmsLogTag::WMS_SYSTEM, "Failed to convert to TransitionController Object");
        return WmErrorCode::WM_ERROR_STATE_ABNORMALLY;
    }
    auto name = GetWindowName();
    std::shared_ptr<NativeReference> jsWindowObj = FindJsWindowObject(name);
    if (jsWindowObj == nullptr || jsWindowObj->GetNapiValue() == nullptr) {
        TLOGE(WmsLogTag::WMS_SYSTEM, "FindJsWindowObject failed");
        return WmErrorCode::WM_ERROR_STATE_ABNORMALLY;
    }
    sptr<JsTransitionController> nativeController = new JsTransitionController(
        env, jsWindowObj, windowToken_);
    auto nativeControllerVal = new wptr<JsTransitionController>(nativeController);
    auto finalizer = [](napi_env, void* data, void*) {
        TLOGNI(WmsLogTag::WMS_SYSTEM, "Finalizer for wptr JsTransitionController called");
        delete static_cast<wptr<JsTransitionController>*>(data);
    };
    if (napi_wrap(env, objValue, nativeControllerVal, finalizer, nullptr, nullptr) != napi_ok) {
        finalizer(env, nativeControllerVal, nullptr);
        TLOGE(WmsLogTag::WMS_SYSTEM, "Failed to wrap TransitionController Object");
        return WmErrorCode::WM_ERROR_STATE_ABNORMALLY;
    };
    WmErrorCode ret = WM_JS_TO_ERROR_CODE_MAP.at(windowToken_->RegisterAnimationTransitionController(nativeController));
    napi_ref result = nullptr;
    napi_create_reference(env, objValue, 1, &result);
    jsTransControllerObj_.reset(reinterpret_cast<NativeReference*>(result));
    nativeController->SetJsController(jsTransControllerObj_);
    TLOGE(WmsLogTag::WMS_SYSTEM, "Window [%{public}u, %{public}s] create success",
        windowToken_->GetWindowId(), windowToken_->GetWindowName().c_str());
    return ret;
}

napi_value JsWindow::OnGetTransitionController(napi_env env, napi_callback_info info)
{
    if (!Permission::IsSystemCalling()) {
        TLOGE(WmsLogTag::WMS_SYSTEM, "not system app, permission denied!");
        return NapiThrowError(env, WmErrorCode::WM_ERROR_NOT_SYSTEM_APP);
    }

    if (windowToken_ == nullptr) {
        TLOGE(WmsLogTag::WMS_SYSTEM, "windowToken is nullptr");
        return NapiThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
    }
    if (!WindowHelper::IsSystemWindow(windowToken_->GetType())) {
        TLOGE(WmsLogTag::WMS_SYSTEM, "Unexpected window type:%{public}d", windowToken_->GetType());
        return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_CALLING);
    }
    if (jsTransControllerObj_ == nullptr || jsTransControllerObj_->GetNapiValue() == nullptr) {
        WmErrorCode ret = CreateTransitionController(env);
        if (ret != WmErrorCode::WM_OK) {
            TLOGE(WmsLogTag::WMS_SYSTEM, "CreateTransitionController failed, error code:%{public}d", ret);
            napi_throw(env, JsErrUtils::CreateJsError(env, ret));
        }
    }
    return jsTransControllerObj_ == nullptr ? nullptr : jsTransControllerObj_->GetNapiValue();
}

napi_value JsWindow::OnSetCornerRadius(napi_env env, napi_callback_info info)
{
    if (!Permission::IsSystemCalling() && !Permission::IsStartByHdcd()) {
        TLOGE(WmsLogTag::WMS_SYSTEM, "Permission denied!");
        return NapiThrowError(env, WmErrorCode::WM_ERROR_NOT_SYSTEM_APP);
    }

    size_t argc = 4;
    napi_value argv[4] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc < 1) {
        TLOGE(WmsLogTag::WMS_SYSTEM, "Argc is invalid: %{public}zu", argc);
        return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM);
    }
    if (windowToken_ == nullptr) {
        TLOGE(WmsLogTag::WMS_SYSTEM, "windowToken is nullptr");
        return NapiThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
    }
    if (!WindowHelper::IsSystemWindow(windowToken_->GetType())) {
        TLOGE(WmsLogTag::WMS_SYSTEM, "Unexpected window type:%{public}d", windowToken_->GetType());
        return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_CALLING);
    }
    napi_value nativeVal = argv[0];
    if (nativeVal == nullptr) {
        TLOGE(WmsLogTag::WMS_SYSTEM, "Invalid radius due to nativeVal is nullptr");
        return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM);
    }
    double radius = 0.0;
    napi_status statusCode = napi_get_value_double(env, nativeVal, &radius);
    if (statusCode != napi_ok) {
        TLOGE(WmsLogTag::WMS_SYSTEM, "napi get radius value failed");
        return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM);
    }
    if (MathHelper::LessNotEqual(radius, 0.0)) {
        TLOGE(WmsLogTag::WMS_SYSTEM, "Invalid radius:%{public}f", radius);
        return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM);
    }
    WmErrorCode ret = WM_JS_TO_ERROR_CODE_MAP.at(windowToken_->SetCornerRadius(radius));
    if (ret != WmErrorCode::WM_OK) {
        TLOGE(WmsLogTag::WMS_SYSTEM, "SetCornerRadius failed:%{public}d", ret);
        return NapiThrowError(env, ret);
    }
    TLOGI(WmsLogTag::WMS_SYSTEM, "Window [%{public}u, %{public}s] set success, radius=%{public}f",
        windowToken_->GetWindowId(), windowToken_->GetWindowName().c_str(), radius);
    return NapiGetUndefined(env);
}

napi_value JsWindow::OnSetWindowCornerRadius(napi_env env, napi_callback_info info)
{
    size_t argc = FOUR_PARAMS_SIZE;
    napi_value argv[FOUR_PARAMS_SIZE] = { nullptr };
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc != ARG_COUNT_ONE) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "Argc is invalid: %{public}zu", argc);
        return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM,
            "[window][setWindowCornerRadius]msg: Exactly one parameter is required.");
    }
    double radius = 0.0;
    if (!ConvertFromJsValue(env, argv[INDEX_ZERO], radius)) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "Failed to convert parameter to radius");
        return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM,
            "[window][setWindowCornerRadius]msg: Failed to convert parameter to radius.");
    }
    float cornerRadius = static_cast<float>(radius);
    if (MathHelper::LessNotEqual(cornerRadius, 0.0f)) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "The corner radius is less than zero.");
        return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM,
            "[window][setWindowCornerRadius]msg: The corner radius is less than zero.");
    }

    const char* const where = __func__;
    napi_value result = nullptr;
    std::shared_ptr<NapiAsyncTask> napiAsyncTask = CreateEmptyAsyncTask(env, nullptr, &result);
    auto asyncTask = [weakWindow = wptr(windowToken_), where, env, task = napiAsyncTask, cornerRadius] {
        auto window = weakWindow.promote();
        if (window == nullptr) {
            TLOGNE(WmsLogTag::WMS_ATTRIBUTE, "%{public}s window is nullptr", where);
            WmErrorCode wmErrorCode = WM_JS_TO_ERROR_CODE_MAP.at(WMError::WM_ERROR_NULLPTR);
            task->Reject(env, JsErrUtils::CreateJsError(env, wmErrorCode,
                "[window][setWindowCornerRadius]msg: Native window is nullptr."));
            return;
        }
        WmErrorCode ret = WM_JS_TO_ERROR_CODE_MAP.at(window->SetWindowCornerRadius(cornerRadius));
        if (ret != WmErrorCode::WM_OK) {
            TLOGNE(WmsLogTag::WMS_ATTRIBUTE, "Set failed!");
            task->Reject(env, JsErrUtils::CreateJsError(env, ret,
                "[window][setWindowCornerRadius]msg: Set window corner radius failed."));
        } else {
            TLOGNI(WmsLogTag::WMS_ATTRIBUTE, "Window [%{public}u, %{public}s] set success.",
                window->GetWindowId(), window->GetWindowName().c_str());
            task->Resolve(env, NapiGetUndefined(env));
        }
    };
    napi_status status = napi_send_event(env, std::move(asyncTask), napi_eprio_high, "OnSetWindowCornerRadius");
    if (status != napi_status::napi_ok) {
        napiAsyncTask->Reject(env,
            CreateJsError(env, static_cast<int32_t>(WmErrorCode::WM_ERROR_STATE_ABNORMALLY),
                "[window][setWindowCornerRadius]msg: Send event failed."));
    }
    return result;
}

napi_value JsWindow::OnGetWindowCornerRadius(napi_env env, napi_callback_info info)
{
    size_t argc = FOUR_PARAMS_SIZE;
    napi_value argv[FOUR_PARAMS_SIZE] = { nullptr };
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc > ARG_COUNT_ZERO) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "Argc is invalid: %{public}zu", argc);
        return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM,
            "[window][getWindowCornerRadius]msg: Unexpected parameters.");
    }
    if (windowToken_ == nullptr) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "WindowToken is nullptr.");
        return NapiThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
            "[window][getWindowCornerRadius]msg: Native window is nullptr.");
    }
    if (!WindowHelper::IsFloatOrSubWindow(windowToken_->GetType())) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "This is not sub window or float window.");
        return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_CALLING,
            "[window][getWindowCornerRadius]msg: This is not sub window or float window.");
    }

    float cornerRadius = 0.0f;
    WmErrorCode ret = WM_JS_TO_ERROR_CODE_MAP.at(windowToken_->GetWindowCornerRadius(cornerRadius));
    if (ret != WmErrorCode::WM_OK) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "Get failed, cornerRadius: %{public}f.", cornerRadius);
        return NapiThrowError(env, ret, "[window][getWindowCornerRadius]msg: Get window corner radius failed.");
    }
    napi_value jsCornerRadius = CreateJsValue(env, cornerRadius);
    if (jsCornerRadius == nullptr) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "jsCornerRadius is nullptr");
        return NapiThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
            "[window][getWindowCornerRadius]msg: Convert corner radius to js value failed.");
    }
    TLOGI(WmsLogTag::WMS_ATTRIBUTE, "Window [%{public}u, %{public}s] Get success, cornerRadius=%{public}f.",
        windowToken_->GetWindowId(), windowToken_->GetWindowName().c_str(), cornerRadius);
    return jsCornerRadius;
}

napi_value JsWindow::OnSetShadow(napi_env env, napi_callback_info info)
{
    WmErrorCode ret = WmErrorCode::WM_OK;
    double radius;
    std::shared_ptr<ShadowsInfo> shadowsInfo = std::make_shared<ShadowsInfo>();
    size_t argc = 4;
    napi_value argv[4] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);

    if (argc < 1) { // 1: min param num
        return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM);
    }
    if (windowToken_ == nullptr) {
        return NapiThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
    }
    if (!WindowHelper::IsSystemWindow(windowToken_->GetType()) &&
        !WindowHelper::IsSubWindow(windowToken_->GetType())) {
        return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_CALLING);
    }

    if (argv[0] == nullptr) {
        return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM);
    }
    napi_status statusCode = napi_get_value_double(env, argv[0], &radius);
    if (statusCode != napi_ok) {
        return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM);
    }
    if (MathHelper::LessNotEqual(radius, 0.0)) {
        return NapiThrowError(env,  WmErrorCode::WM_ERROR_INVALID_PARAM);
    }
    ret = WM_JS_TO_ERROR_CODE_MAP.at(windowToken_->SetShadowRadius(radius));
    shadowsInfo->radius_ = radius;
    shadowsInfo->hasRadiusValue_ = true;

    ParseShadowOptionalParameters(ret, shadowsInfo, env, argv, argc);
    if (ret != WmErrorCode::WM_OK) {
        napi_throw(env, JsErrUtils::CreateJsError(env, ret));
    }

    WmErrorCode syncShadowsRet = WM_JS_TO_ERROR_CODE_MAP.at(windowToken_->SyncShadowsToComponent(*shadowsInfo));
    if (syncShadowsRet != WmErrorCode::WM_OK) {
        TLOGE(WmsLogTag::WMS_ANIMATION, "Sync shadows to component failed! ret:  %{public}u",
            static_cast<int32_t>(syncShadowsRet));
    }
    return NapiGetUndefined(env);
}

void JsWindow::ParseShadowOptionalParameters(WmErrorCode& ret, std::shared_ptr<ShadowsInfo>& shadowsInfo,
    napi_env env, const napi_value* argv, size_t argc)
{
    std::string color;
    double offsetX;
    double offsetY;
    if ((ret == WmErrorCode::WM_OK) && (argc >= 2)) { // parse the 2nd param: color
        if (ConvertFromJsValue(env, argv[1], color)) {
            ret = WM_JS_TO_ERROR_CODE_MAP.at(windowToken_->SetShadowColor(color));
            shadowsInfo->color_ = color;
            shadowsInfo->hasColorValue_ = true;
        }
    }
    if ((ret == WmErrorCode::WM_OK) && argc >= 3) { // parse the 3rd param: offsetX
        if (argv[2] != nullptr) { // 2: the 3rd param
            napi_get_value_double(env, argv[2], &offsetX);
            ret = WM_JS_TO_ERROR_CODE_MAP.at(windowToken_->SetShadowOffsetX(offsetX));
            shadowsInfo->offsetX_ = offsetX;
            shadowsInfo->hasOffsetXValue_ = true;
        }
    }
    if ((ret == WmErrorCode::WM_OK) && argc >= 4) { // parse the 4th param: offsetY
        if (argv[3] != nullptr) {  // 3: the 4th param
            napi_get_value_double(env, argv[3], &offsetY);
            ret = WM_JS_TO_ERROR_CODE_MAP.at(windowToken_->SetShadowOffsetY(offsetY));
            shadowsInfo->offsetY_ = offsetY;
            shadowsInfo->hasOffsetYValue_ = true;
        }
    }
}

napi_value JsWindow::OnSetWindowShadowRadius(napi_env env, napi_callback_info info)
{
    size_t argc = FOUR_PARAMS_SIZE;
    napi_value argv[FOUR_PARAMS_SIZE] = { nullptr };
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc != ARG_COUNT_ONE) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "Argc is invalid: %{public}zu", argc);
        return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM,
            "[window][setWindowShadowRadius]msg: Exactly one parameter is required.");
    }
    if (windowToken_ == nullptr) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "WindowToken is nullptr.");
        return NapiThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
            "[window][setWindowShadowRadius]msg: Native window is nullptr.");
    }
    if (!WindowHelper::IsFloatOrSubWindow(windowToken_->GetType())) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "This is not sub window or float window.");
        return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_CALLING,
            "[window][setWindowShadowRadius]msg: This is not sub window or float window.");
    }

    double result = 0.0;
    if (!ConvertFromJsValue(env, argv[INDEX_ZERO], result)) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "Napi get radius value failed.");
        return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM,
            "[window][setWindowShadowRadius]msg: Napi get radius value failed.");
    }
    float radius = static_cast<float>(result);
    if (MathHelper::LessNotEqual(radius, 0.0f)) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "The shadow radius is less than zero.");
        return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM,
            "[window][setWindowShadowRadius]msg: The shadow radius is less than zero.");
    }
    std::shared_ptr<ShadowsInfo> shadowsInfo = std::make_shared<ShadowsInfo>();
    shadowsInfo->radius_ = result;
    shadowsInfo->hasRadiusValue_ = true;
    WmErrorCode ret = WM_JS_TO_ERROR_CODE_MAP.at(windowToken_->SetWindowShadowRadius(radius));
    if (ret != WmErrorCode::WM_OK) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "Set failed, radius: %{public}f.", radius);
        return NapiThrowError(env, ret, "[window][setWindowShadowRadius]msg: Set shadow radius failed.");
    }
    WmErrorCode syncShadowsRet = WM_JS_TO_ERROR_CODE_MAP.at(windowToken_->SyncShadowsToComponent(*shadowsInfo));
    if (syncShadowsRet != WmErrorCode::WM_OK) {
        TLOGE(WmsLogTag::WMS_ANIMATION, "Sync shadows to component fail! ret:  %{public}u",
            static_cast<int32_t>(syncShadowsRet));
        return NapiThrowError(env, syncShadowsRet,
            "[window][setWindowShadowRadius]msg: Sync shadows to component failed.");
    }
    TLOGI(WmsLogTag::WMS_ATTRIBUTE, "Window [%{public}u, %{public}s] set success, radius=%{public}f.",
        windowToken_->GetWindowId(), windowToken_->GetWindowName().c_str(), radius);
    return NapiGetUndefined(env);
}

napi_value JsWindow::OnSetBlur(napi_env env, napi_callback_info info)
{
    size_t argc = 4;
    napi_value argv[4] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc < 1) {
        TLOGE(WmsLogTag::WMS_SYSTEM, "Argc is invalid: %{public}zu", argc);
        return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM);
    }
    if (windowToken_ == nullptr) {
        TLOGE(WmsLogTag::WMS_SYSTEM, "windowToken is nullptr");
        return NapiThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
    }
    if (!WindowHelper::IsSystemWindow(windowToken_->GetType())) {
        TLOGE(WmsLogTag::WMS_SYSTEM, "Unexpected window type:%{public}d", windowToken_->GetType());
        return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_CALLING);
    }
    napi_value nativeVal = argv[0];
    if (nativeVal == nullptr) {
        TLOGE(WmsLogTag::WMS_SYSTEM, "nativeVal is nullptr");
        return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM);
    }
    double radius = 0.0;
    napi_status statusCode = napi_get_value_double(env, nativeVal, &radius);
    if (statusCode != napi_ok) {
        TLOGE(WmsLogTag::WMS_SYSTEM, "napi get radius value failed");
        return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM);
    }
    if (MathHelper::LessNotEqual(radius, 0.0)) {
        TLOGE(WmsLogTag::WMS_SYSTEM, "Invalid radius:%{public}f", radius);
        return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM);
    }
    WmErrorCode ret = WM_JS_TO_ERROR_CODE_MAP.at(windowToken_->SetBlur(radius));
    if (ret != WmErrorCode::WM_OK) {
        TLOGE(WmsLogTag::WMS_SYSTEM, "failed:%{public}d", ret);
        return NapiThrowError(env, ret);
    }
    TLOGI(WmsLogTag::WMS_SYSTEM, "end, window [%{public}u, %{public}s] radius=%{public}f",
        windowToken_->GetWindowId(), windowToken_->GetWindowName().c_str(), radius);
    return NapiGetUndefined(env);
}

napi_value JsWindow::OnSetBackdropBlur(napi_env env, napi_callback_info info)
{
    size_t argc = 4;
    napi_value argv[4] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc < 1) {
        TLOGE(WmsLogTag::WMS_SYSTEM, "Argc is invalid: %{public}zu", argc);
        return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM);
    }
    if (windowToken_ == nullptr) {
        TLOGE(WmsLogTag::WMS_SYSTEM, "windowToken is nullptr");
        return NapiThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
    }
    if (!WindowHelper::IsSystemWindow(windowToken_->GetType())) {
        TLOGE(WmsLogTag::WMS_SYSTEM, "Unexpected window type:%{public}d", windowToken_->GetType());
        return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_CALLING);
    }
    napi_value nativeVal = argv[0];
    if (nativeVal == nullptr) {
        TLOGE(WmsLogTag::WMS_SYSTEM, "Invalid radius due to nativeVal is nullptr");
        return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM);
    }
    double radius = 0.0;
    napi_status statusCode = napi_get_value_double(env, nativeVal, &radius);
    if (statusCode != napi_ok) {
        TLOGE(WmsLogTag::WMS_SYSTEM, "napi get radius value failed");
        return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM);
    }
    if (MathHelper::LessNotEqual(radius, 0.0)) {
        TLOGE(WmsLogTag::WMS_SYSTEM, "Invalid radius:%{public}f", radius);
        return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM);
    }
    WmErrorCode ret = WM_JS_TO_ERROR_CODE_MAP.at(windowToken_->SetBackdropBlur(radius));
    if (ret != WmErrorCode::WM_OK) {
        TLOGE(WmsLogTag::WMS_SYSTEM, "failed:%{public}d", ret);
        return NapiThrowError(env, ret);
    }
    TLOGI(WmsLogTag::WMS_SYSTEM, "Window [%{public}u, %{public}s] set success, radius=%{public}f",
        windowToken_->GetWindowId(), windowToken_->GetWindowName().c_str(), radius);
    return NapiGetUndefined(env);
}

napi_value JsWindow::OnSetBackdropBlurStyle(napi_env env, napi_callback_info info)
{
    size_t argc = 4;
    napi_value argv[4] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc < 1) {
        TLOGE(WmsLogTag::WMS_SYSTEM, "Argc is invalid: %{public}zu", argc);
        return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM);
    }
    if (windowToken_ == nullptr) {
        TLOGE(WmsLogTag::WMS_SYSTEM, "windowToken is nullptr");
        return NapiThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
    }
    if (!WindowHelper::IsSystemWindow(windowToken_->GetType())) {
        TLOGE(WmsLogTag::WMS_SYSTEM, "Unexpected window type:%{public}d", windowToken_->GetType());
        return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_CALLING);
    }

    napi_value nativeMode = argv[0];
    if (nativeMode == nullptr) {
        TLOGE(WmsLogTag::WMS_SYSTEM, "nativeMode is nullptr");
        return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM);
    }
    uint32_t resultValue = 0;
    napi_status statusCode = napi_get_value_uint32(env, nativeMode, &resultValue);
    if (statusCode != napi_ok) {
        return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM);
    }
    if (resultValue > static_cast<uint32_t>(WindowBlurStyle::WINDOW_BLUR_THICK)) {
        TLOGE(WmsLogTag::WMS_SYSTEM, "Invalid window blur style:%{public}u", resultValue);
        return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM);
    }
    WindowBlurStyle style = static_cast<WindowBlurStyle>(resultValue);
    WmErrorCode ret = WM_JS_TO_ERROR_CODE_MAP.at(windowToken_->SetBackdropBlurStyle(style));
    if (ret != WmErrorCode::WM_OK) {
        TLOGE(WmsLogTag::WMS_SYSTEM, "set failed: %{public}d", ret);
        return NapiThrowError(env, ret);
    }

    TLOGI(WmsLogTag::WMS_SYSTEM, "Window [%{public}u, %{public}s] set success, style=%{public}u",
        windowToken_->GetWindowId(), windowToken_->GetWindowName().c_str(), style);
    return NapiGetUndefined(env);
}

napi_value JsWindow::OnSetWaterMarkFlag(napi_env env, napi_callback_info info)
{
    size_t argc = 4;
    napi_value argv[4] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc < 1) {
        WLOGFE("Argc is invalid: %{public}zu", argc);
        return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM);
    }

    napi_value nativeBool = argv[0];
    if (nativeBool == nullptr) {
        WLOGFE("Invalid window flag");
        return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM);
    }

    bool isAddSafetyLayer = false;
    napi_status statusCode = napi_get_value_bool(env, nativeBool, &isAddSafetyLayer);
    if (statusCode != napi_ok) {
        return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM);
    }
    const char* const where = __func__;
    napi_value lastParam = (argc == 1) ? nullptr :
        (GetType(env, argv[1]) == napi_function ? argv[1] : nullptr);
    napi_value result = nullptr;
    std::shared_ptr<NapiAsyncTask> napiAsyncTask = CreateEmptyAsyncTask(env, lastParam, &result);
    auto asyncTask = [weakToken = wptr<Window>(windowToken_), isAddSafetyLayer, where, env, task = napiAsyncTask] {
        auto window = weakToken.promote();
        if (window == nullptr) {
            task->Reject(env,
                JsErrUtils::CreateJsError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
                "[window][setWaterMarkFlag]msg: invalid window"));
            return;
        }
        WMError ret = WMError::WM_OK;
        if (isAddSafetyLayer) {
            ret = window->AddWindowFlag(WindowFlag::WINDOW_FLAG_WATER_MARK);
        } else {
            ret = window->RemoveWindowFlag(WindowFlag::WINDOW_FLAG_WATER_MARK);
        }
        if (ret == WMError::WM_OK) {
            task->Resolve(env, NapiGetUndefined(env));
        } else {
            task->Reject(env, JsErrUtils::CreateJsError(env, WM_JS_TO_ERROR_CODE_MAP.at(ret),
                "[window][setWaterMarkFlag]msg: set water mark flag failed"));
        }
        WLOGI("%{public}s end, window [%{public}u, %{public}s] ret=%{public}d",
            where, window->GetWindowId(), window->GetWindowName().c_str(), ret);
    };
    if (napi_send_event(env, asyncTask, napi_eprio_high, "OnSetWaterMarkFlag") != napi_status::napi_ok) {
        TLOGE(WmsLogTag::WMS_IMMS, "napi_send_event failed");
        napiAsyncTask->Reject(env,
            JsErrUtils::CreateJsError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
                "[window][setWaterMarkFlag]msg: failed to send event"));
    }
    return result;
}

napi_value JsWindow::OnSetHandwritingFlag(napi_env env, napi_callback_info info)
{
    size_t argc = 4;
    napi_value argv[4] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc < 1) {
        WLOGFE("Argc is invalid: %{public}zu", argc);
        return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM);
    }

    napi_value nativeBool = argv[0];
    if (nativeBool == nullptr) {
        WLOGFE("SetHandwritingFlag Invalid window flag");
        return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM);
    }
    bool isAddFlag = false;
    napi_get_value_bool(env, nativeBool, &isAddFlag);
    wptr<Window> weakToken(windowToken_);
    std::shared_ptr<WmErrorCode> errCodePtr = std::make_shared<WmErrorCode>(WmErrorCode::WM_OK);
    NapiAsyncTask::ExecuteCallback execute = [weakToken, isAddFlag, errCodePtr] {
        if (errCodePtr == nullptr) {
            return;
        }
        auto weakWindow = weakToken.promote();
        if (weakWindow == nullptr) {
            *errCodePtr = WmErrorCode::WM_ERROR_STATE_ABNORMALLY;
            return;
        }
        WMError ret = isAddFlag ? weakWindow->AddWindowFlag(WindowFlag::WINDOW_FLAG_HANDWRITING) :
            weakWindow->RemoveWindowFlag(WindowFlag::WINDOW_FLAG_HANDWRITING);
        *errCodePtr = WM_JS_TO_ERROR_CODE_MAP.at(ret);
        WLOGI("Window [%{public}u, %{public}s] set handwriting flag on end",
            weakWindow->GetWindowId(), weakWindow->GetWindowName().c_str());
    };
    NapiAsyncTask::CompleteCallback complete =
        [weakToken, isAddFlag, errCodePtr](napi_env env, NapiAsyncTask& task, int32_t status) {
            if (errCodePtr == nullptr) {
                task.Reject(env, JsErrUtils::CreateJsError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
                    "[window][setHandwritingFlag]msg:System abnormal."));
                return;
            }
            if (*errCodePtr == WmErrorCode::WM_OK) {
                task.Resolve(env, NapiGetUndefined(env));
            } else {
                task.Reject(env, JsErrUtils::CreateJsError(env, *errCodePtr, "SetHandwritingFlag failed."));
            }
        };

    napi_value lastParam = (argc == 1) ? nullptr : (GetType(env, argv[1]) == napi_function ? argv[1] : nullptr);
    napi_value result = nullptr;
    NapiAsyncTask::Schedule("JsWindow::OnSetHandwritingFlag",
        env, CreateAsyncTaskWithLastParam(env, lastParam, std::move(execute), std::move(complete), &result));
    return result;
}

napi_value JsWindow::OnSetAspectRatio(napi_env env, napi_callback_info info)
{
    WMError errCode = WMError::WM_OK;
    size_t argc = 4;
    napi_value argv[4] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc < 1 || argc > 2) { // 2: maximum params num
        TLOGE(WmsLogTag::WMS_LAYOUT, "Argc is invalid: %{public}zu", argc);
        errCode = WMError::WM_ERROR_INVALID_PARAM;
    }

    if (windowToken_ == nullptr) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "WindowToken is nullptr");
        return NapiThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
            "[window][setAspectRatio]msg: Window is nullptr");
    }

    if (!WindowHelper::IsMainWindow(windowToken_->GetType())) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "SetAspectRatio is only allowed main window");
        return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_CALLING,
            "[window][setAspectRatio]msg: SetAspectRatio is only allowed for the main window");
    }

    double aspectRatio = 0.0;
    if (errCode == WMError::WM_OK) {
        napi_value nativeVal = argv[0];
        if (nativeVal == nullptr) {
            errCode = WMError::WM_ERROR_INVALID_PARAM;
        } else {
            CHECK_NAPI_RETCODE(errCode, WMError::WM_ERROR_INVALID_PARAM,
                napi_get_value_double(env, nativeVal, &aspectRatio));
        }
    }

    if (errCode == WMError::WM_ERROR_INVALID_PARAM || aspectRatio <= 0.0) {
        return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM,
            "[window][setAspectRatio]msg: Falied");
    }

    napi_value lastParam = (argc == 1) ? nullptr : (GetType(env, argv[1]) == napi_function ? argv[1] : nullptr);
    napi_value result = nullptr;
    std::shared_ptr<NapiAsyncTask> napiAsyncTask = CreateEmptyAsyncTask(env, lastParam, &result);
    auto asyncTask =
        [weakToken = wptr<Window>(windowToken_), task = napiAsyncTask, aspectRatio, where = __func__, env] {
            auto window = weakToken.promote();
            if (window == nullptr) {
                task->Reject(env, JsErrUtils::CreateJsError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
                    "[window][setAspectRatio]msg: Falied"));
                return;
            }
            WMError ret = window->SetAspectRatio(aspectRatio);
            if (ret == WMError::WM_OK) {
                task->Resolve(env, NapiGetUndefined(env));
            } else {
                task->Reject(env, JsErrUtils::CreateJsError(env, WM_JS_TO_ERROR_CODE_MAP.at(ret),
                    "[window][setAspectRatio]msg: Falied"));
            }
            TLOGNI(WmsLogTag::WMS_LAYOUT, "%{public}s: end, window [%{public}u, %{public}s] ret=%{public}d",
                where, window->GetWindowId(), window->GetWindowName().c_str(), ret);
        };
    if (napi_send_event(env, asyncTask, napi_eprio_high, "OnSetAspectRatio") != napi_status::napi_ok) {
        napiAsyncTask->Reject(env,
            JsErrUtils::CreateJsError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
                "[window][setAspectRatio]msg: Falied"));
    }
    return result;
}

napi_value JsWindow::OnResetAspectRatio(napi_env env, napi_callback_info info)
{
    size_t argc = 4;
    napi_value argv[4] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc > 1) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "Argc is invalid: %{public}zu", argc);
        return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM,
            "[window][resetAspectRatio]msg: Number of parameters is invalid");
    }

    if (windowToken_ == nullptr) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "WindowToken is nullptr");
        return NapiThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
            "[window][resetAspectRatio]msg: Window is nullptr");
    }

    if (!WindowHelper::IsMainWindow(windowToken_->GetType())) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "ResetAspectRatio is not allowed since window is main window");
        return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_CALLING,
            "[window][resetAspectRatio]msg: ResetAspectRatio is not allowed since window is main window");
    }

    napi_value lastParam = (argc == 0) ? nullptr :
        (GetType(env, argv[0]) == napi_function ? argv[0] : nullptr);
    napi_value result = nullptr;
    std::shared_ptr<NapiAsyncTask> napiAsyncTask = CreateEmptyAsyncTask(env, lastParam, &result);
    auto asyncTask = [weakToken = wptr<Window>(windowToken_), task = napiAsyncTask, where = __func__, env] {
        auto window = weakToken.promote();
        if (window == nullptr) {
            task->Reject(env, JsErrUtils::CreateJsError(env,
                WmErrorCode::WM_ERROR_STATE_ABNORMALLY, "[window][resetAspectRatio]msg: window is nullptr"));
            return;
        }
        WMError ret = window->ResetAspectRatio();
        if (ret == WMError::WM_OK) {
            task->Resolve(env, NapiGetUndefined(env));
        } else {
            task->Reject(env, JsErrUtils::CreateJsError(env, ret, "[window][resetAspectRatio]msg: Failed."));
        }
        TLOGND(WmsLogTag::WMS_LAYOUT, "%{public}s end, window [%{public}u, %{public}s] ret=%{public}d",
            where, window->GetWindowId(), window->GetWindowName().c_str(), ret);
    };
    if (napi_send_event(env, asyncTask, napi_eprio_high, "OnResetAspectRatio") != napi_status::napi_ok) {
        napiAsyncTask->Reject(env,
            JsErrUtils::CreateJsError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
                "[window][resetAspectRatio]msg: Failed to send event"));
    }
    return result;
}

napi_value JsWindow::OnSetContentAspectRatio(napi_env env, napi_callback_info info)
{
    const std::string errMsgPrefix = "[window][setContentAspectRatio]msg: ";
    auto logAndThrowError = [env, where = __func__, errMsgPrefix](WmErrorCode code, const std::string& msg) {
        TLOGNE(WmsLogTag::WMS_LAYOUT, "%{public}s: %{public}s", where, msg.c_str());
        return NapiThrowError(env, code, errMsgPrefix + msg);
    };

    size_t argc = THREE_PARAMS_SIZE;
    napi_value argv[THREE_PARAMS_SIZE] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc < ONE_PARAMS_SIZE || argc > THREE_PARAMS_SIZE) {
        return logAndThrowError(WmErrorCode::WM_ERROR_INVALID_PARAM, "Number of parameters is invalid");
    }

    double aspectRatio = 0.0;
    if (!ConvertFromJsValue(env, argv[INDEX_ZERO], aspectRatio)) {
        return logAndThrowError(WmErrorCode::WM_ERROR_INVALID_PARAM, "Failed to convert parameter to aspectRatio");
    }

    bool isPersistent = true;
    if (argc >= TWO_PARAMS_SIZE && !ConvertFromOptionalJsValue(env, argv[INDEX_ONE], isPersistent, true)) {
        return logAndThrowError(WmErrorCode::WM_ERROR_INVALID_PARAM, "Failed to convert parameter to isPersistent");
    }

    bool needUpdateRect = true;
    if (argc == THREE_PARAMS_SIZE && !ConvertFromOptionalJsValue(env, argv[INDEX_TWO], needUpdateRect, true)) {
        return logAndThrowError(WmErrorCode::WM_ERROR_INVALID_PARAM, "Failed to convert parameter to needUpdateRect");
    }

    napi_value result = nullptr;
    std::shared_ptr<NapiAsyncTask> napiAsyncTask = CreateEmptyAsyncTask(env, nullptr, &result);
    auto asyncTask = [windowToken = wptr<Window>(windowToken_), aspectRatio, isPersistent, needUpdateRect,
                      env, napiAsyncTask, where = __func__, errMsgPrefix] {
        auto window = windowToken.promote();
        if (!window) {
            TLOGNE(WmsLogTag::WMS_LAYOUT, "%{public}s: Window is nullptr", where);
            napiAsyncTask->Reject(env, JsErrUtils::CreateJsError(
                env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY, errMsgPrefix + "Window is nullptr"));
            return;
        }
        WMError ret = window->SetContentAspectRatio(aspectRatio, isPersistent, needUpdateRect);
        auto it = WM_JS_TO_ERROR_CODE_MAP.find(ret);
        WmErrorCode code = (it != WM_JS_TO_ERROR_CODE_MAP.end()) ? it->second : WmErrorCode::WM_ERROR_STATE_ABNORMALLY;
        if (code == WmErrorCode::WM_OK) {
            napiAsyncTask->Resolve(env, NapiGetUndefined(env));
        } else {
            napiAsyncTask->Reject(env, JsErrUtils::CreateJsError(env, code, errMsgPrefix + "Failed"));
        }
    };
    if (napi_send_event(env, asyncTask, napi_eprio_high, __func__) != napi_status::napi_ok) {
        napiAsyncTask->Reject(env, JsErrUtils::CreateJsError(
            env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY, errMsgPrefix + "Failed to send event"));
    }
    return result;
}

napi_value JsWindow::OnMinimize(napi_env env, napi_callback_info info)
{
    size_t argc = FOUR_PARAMS_SIZE;
    napi_value argv[FOUR_PARAMS_SIZE] = { nullptr };
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc > 1) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "Argc is invalid: %{public}zu", argc);
        return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM);
    }

    if (windowToken_ != nullptr && WindowHelper::IsFloatOrSubWindow(windowToken_->GetType())) {
        if (!windowToken_->IsSceneBoardEnabled()) {
            return NapiThrowError(env, WmErrorCode::WM_ERROR_DEVICE_NOT_SUPPORT);
        }
        TLOGI(WmsLogTag::WMS_LAYOUT, "subWindow or float window use hide");
        return HideWindowFunction(env, info, WmErrorCode::WM_OK);
    }

    napi_value lastParam = (argc == 0) ? nullptr :
        (GetType(env, argv[INDEX_ZERO]) == napi_function ? argv[INDEX_ZERO] : nullptr);
    napi_value result = nullptr;
    std::shared_ptr<NapiAsyncTask> napiAsyncTask = CreateEmptyAsyncTask(env, lastParam, &result);
    const char* const where = __func__;
    auto asyncTask = [weakToken = wptr<Window>(windowToken_), env, task = napiAsyncTask, where] {
        auto window = weakToken.promote();
        if (window == nullptr) {
            TLOGNE(WmsLogTag::WMS_LAYOUT, "%{public}s window is nullptr", where);
            task->Reject(env, JsErrUtils::CreateJsError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
                "[window][minimize]msg: Window is nullptr."));
            return;
        }
        WMError ret = window->Minimize();
        if (ret == WMError::WM_OK) {
            task->Resolve(env, NapiGetUndefined(env));
        } else {
            WmErrorCode wmErrorCode = WM_JS_TO_ERROR_CODE_MAP.at(ret);
            task->Reject(env, JsErrUtils::CreateJsError(env, wmErrorCode,
                "[window][minimize]msg: Minimize failed."));
        }
        TLOGNI(WmsLogTag::WMS_PC, "%{public}s Window [%{public}u, %{public}s] minimize end, ret=%{public}d",
            where, window->GetWindowId(), window->GetWindowName().c_str(), ret);
    };
    if (napi_send_event(env, asyncTask, napi_eprio_immediate, "OnMinimize") != napi_status::napi_ok) {
        napiAsyncTask->Reject(env, CreateJsError(env,
            static_cast<int32_t>(WmErrorCode::WM_ERROR_STATE_ABNORMALLY), "send event failed"));
    }
    return result;
}

std::optional<MaximizePresentation> ParsePresentation(napi_env env, napi_value napiPresentation)
{
    if (env == nullptr) {
        TLOGE(WmsLogTag::WMS_LAYOUT_PC, "env is nullptr");
        return std::nullopt;
    }
    if (napiPresentation == nullptr || GetType(env, napiPresentation) == napi_undefined) {
        return MaximizePresentation::ENTER_IMMERSIVE;
    }
    using T = std::underlying_type_t<MaximizePresentation>;
    T value = static_cast<T>(MaximizePresentation::ENTER_IMMERSIVE);
    if (!ConvertFromJsValue(env, napiPresentation, value) ||
        value < static_cast<T>(MaximizePresentation::FOLLOW_APP_IMMERSIVE_SETTING) ||
        value > static_cast<T>(MaximizePresentation::ENTER_IMMERSIVE_DISABLE_TITLE_AND_DOCK_HOVER)) {
        TLOGE(WmsLogTag::WMS_LAYOUT_PC, "Invalid presentation param");
        return std::nullopt;
    }
    return static_cast<MaximizePresentation>(value);
}

std::optional<WaterfallResidentState> ParseWaterfallResidentState(napi_env env, napi_value napiAcrossDisplay)
{
    if (env == nullptr) {
        TLOGE(WmsLogTag::WMS_LAYOUT_PC, "env is nullptr");
        return std::nullopt;
    }
    if (napiAcrossDisplay == nullptr || GetType(env, napiAcrossDisplay) == napi_undefined) {
        return WaterfallResidentState::UNCHANGED;
    }
    bool acrossDisplay = false;
    if (!ConvertFromJsValue(env, napiAcrossDisplay, acrossDisplay)) {
        TLOGE(WmsLogTag::WMS_LAYOUT_PC, "Invalid acrossDisplay param");
        return std::nullopt;
    }
    return acrossDisplay ? WaterfallResidentState::OPEN : WaterfallResidentState::CLOSE;
}

napi_value JsWindow::OnMaximize(napi_env env, napi_callback_info info)
{
    if (windowToken_ == nullptr) {
        TLOGE(WmsLogTag::WMS_LAYOUT_PC, "window is nullptr");
        return NapiThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY, "[window][maximize]msg: window is nullptr");
    }
    if (!(WindowHelper::IsMainWindow(windowToken_->GetType()) || windowToken_->IsSubWindowMaximizeSupported())) {
        TLOGE(WmsLogTag::WMS_LAYOUT_PC, "only support main or sub Window");
        return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_CALLING,
            "[window][maximize]msg: Only support main or sub Window");
    }
    size_t argc = TWO_PARAMS_SIZE;
    napi_value argv[TWO_PARAMS_SIZE] = { nullptr };
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    auto presentationOpt = ParsePresentation(env, argv[INDEX_ZERO]);
    if (!presentationOpt) {
        return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM,
                              "[window][maximize]msg: Failed to convert parameter to presentation");
    }
    auto waterfallResidentStateOpt = ParseWaterfallResidentState(env, argv[INDEX_ONE]);
    if (!waterfallResidentStateOpt) {
        return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM,
                              "[window][maximize]msg: Failed to convert parameter to acrossDisplay");
    }
    napi_value result = nullptr;
    std::shared_ptr<NapiAsyncTask> napiAsyncTask = CreateEmptyAsyncTask(env, nullptr, &result);
    auto asyncTask = [windowToken = wptr<Window>(windowToken_),
                      presentation = *presentationOpt, waterfallResidentState = *waterfallResidentStateOpt,
                      env, napiAsyncTask, where = __func__] {
        auto window = windowToken.promote();
        if (window == nullptr) {
            napiAsyncTask->Reject(env,
                JsErrUtils::CreateJsError(
                    env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY, "[window][maximize]msg: window is nullptr"));
            return;
        }
        WMError ret = window->Maximize(presentation, waterfallResidentState);
        if (ret == WMError::WM_OK) {
            napiAsyncTask->Resolve(env, NapiGetUndefined(env));
        } else {
            WmErrorCode wmErrorCode = WM_JS_TO_ERROR_CODE_MAP.at(ret);
            napiAsyncTask->Reject(env, JsErrUtils::CreateJsError(env, wmErrorCode, "[window][maximize]msg: Failed"));
        }
        TLOGNI(WmsLogTag::WMS_LAYOUT_PC,
            "%{public}s: windowId: %{public}u, presentation: %{public}d, waterfallResidentState: %{public}u",
            where, window->GetWindowId(), static_cast<int32_t>(presentation),
            static_cast<uint32_t>(waterfallResidentState));
    };
    if (napi_send_event(env, asyncTask, napi_eprio_immediate, "OnMaximize") != napi_status::napi_ok) {
        napiAsyncTask->Reject(env,
            JsErrUtils::CreateJsError(
                env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY, "[window][maximize]msg: Failed to send event"));
    }
    return result;
}

std::shared_ptr<NativeReference> FindJsWindowObject(const std::string& windowName)
{
    WLOGFD("Try to find window %{public}s in g_jsWindowMap", windowName.c_str());
    std::lock_guard<std::mutex> lock(g_mutex);
    if (g_jsWindowMap.find(windowName) == g_jsWindowMap.end()) {
        WLOGFD("Can not find window %{public}s in g_jsWindowMap", windowName.c_str());
        return nullptr;
    }
    return g_jsWindowMap[windowName];
}

napi_value CreateJsWindowObject(napi_env env, sptr<Window>& window)
__attribute__((no_sanitize("cfi")))
{
    std::string windowName = window->GetWindowName();
    // avoid repeatedly create js window when getWindow
    std::shared_ptr<NativeReference> jsWindowObj = FindJsWindowObject(windowName);
    if (jsWindowObj != nullptr && jsWindowObj->GetNapiValue() != nullptr) {
        WLOGD("FindJsWindowObject %{public}s", windowName.c_str());
        return jsWindowObj->GetNapiValue();
    }
    napi_value objValue = nullptr;
    napi_create_object(env, &objValue);

    WLOGI("CreateJsWindow %{public}s", windowName.c_str());
    std::unique_ptr<JsWindow> jsWindow = std::make_unique<JsWindow>(window);
    napi_wrap(env, objValue, jsWindow.release(), JsWindow::Finalizer, nullptr, nullptr);

    BindFunctions(env, objValue, "JsWindow");

    std::shared_ptr<NativeReference> jsWindowRef;
    napi_ref result = nullptr;
    napi_create_reference(env, objValue, 1, &result);
    jsWindowRef.reset(reinterpret_cast<NativeReference*>(result));
    std::lock_guard<std::mutex> lock(g_mutex);
    g_jsWindowMap[windowName] = jsWindowRef;
    TLOGI(WmsLogTag::WMS_LIFE, "Add window %{public}s", windowName.c_str());
    return objValue;
}

napi_value CreateJsWindowArrayObject(napi_env env, const std::vector<sptr<Window>>& windows)
{
    napi_value arrayValue = nullptr;
    napi_create_array_with_length(env, windows.size(), &arrayValue);
    if (arrayValue == nullptr) {
        TLOGE(WmsLogTag::DEFAULT, "Failed to create napi array");
        return nullptr;
    }
    uint32_t index = 0;
    for (size_t i = 0; i < windows.size(); i++) {
        auto window = windows[i];
        if (window == nullptr) {
            TLOGW(WmsLogTag::DEFAULT, "window is null");
        } else {
            napi_set_element(env, arrayValue, index++, CreateJsWindowObject(env, window));
        }
    }
    return arrayValue;
}

bool JsWindow::ParseWindowLimits(napi_env env, napi_value jsObject, WindowLimits& windowLimits)
{
    uint32_t data = 0;
    uint32_t defaultValue = 0;
    PixelUnit pixelUnit = PixelUnit::PX;
    if (GetType(env, jsObject) != napi_object) {
        return false;
    }
    auto parseField = [&](const char* fieldName, auto& field, auto& defValue) -> bool {
        if (!ParseJsValueOrGetDefault(jsObject, env, fieldName, data, defValue)) {
            TLOGE(WmsLogTag::WMS_LAYOUT, "Failed to convert object to %{public}s", fieldName);
            return false;
        }
        field = data;
        return true;
    };

    if (!parseField("maxWidth", windowLimits.maxWidth_, defaultValue)) {
        return false;
    }
    if (!parseField("minWidth", windowLimits.minWidth_, defaultValue)) {
        return false;
    }
    if (!parseField("maxHeight", windowLimits.maxHeight_, defaultValue)) {
        return false;
    }
    if (!parseField("minHeight", windowLimits.minHeight_, defaultValue)) {
        return false;
    }
    if (!ParseJsValueOrGetDefault(jsObject, env, "pixelUnit", pixelUnit, PixelUnit::PX)) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "Failed to convert object to pixelUnit");
        return false;
    }
    windowLimits.pixelUnit_ = pixelUnit;
    return true;
}

static NapiAsyncTask::ExecuteCallback GetEnableDragExecuteCallback(bool enableDrag,
    const wptr<Window>& weakToken, const std::shared_ptr<WmErrorCode>& errCodePtr)
{
    NapiAsyncTask::ExecuteCallback execute = [weakToken, enableDrag, errCodePtr] {
        if (errCodePtr == nullptr) {
            return;
        }
        auto window = weakToken.promote();
        if (window == nullptr) {
            *errCodePtr = WmErrorCode::WM_ERROR_STATE_ABNORMALLY;
            return;
        }
        if (WindowHelper::IsInputWindow(window->GetType())) {
            TLOGE(WmsLogTag::WMS_LAYOUT, "is not allowed since input window");
            *errCodePtr = WmErrorCode::WM_ERROR_INVALID_CALLING;
            return;
        }
        *errCodePtr = WM_JS_TO_ERROR_CODE_MAP.at(window->EnableDrag(enableDrag));
        TLOGNI(WmsLogTag::WMS_LAYOUT, "Window [%{public}u, %{public}s] set enable drag end",
            window->GetWindowId(), window->GetWindowName().c_str());
    };
    return execute;
}

static NapiAsyncTask::CompleteCallback GetEnableDragCompleteCallback(
    const std::shared_ptr<WmErrorCode>& errCodePtr)
{
    NapiAsyncTask::CompleteCallback complete = [errCodePtr](napi_env env, NapiAsyncTask& task, int32_t status) {
        if (errCodePtr == nullptr) {
            task.Reject(env,
                JsErrUtils::CreateJsError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY, "Set enable drag failed."));
            return;
        }
        TLOGNI(WmsLogTag::WMS_LAYOUT, "OnEnableDrag: ret: %{public}u", *errCodePtr);
        if (*errCodePtr == WmErrorCode::WM_OK) {
            task.Resolve(env, NapiGetUndefined(env));
        } else {
            task.Reject(env, JsErrUtils::CreateJsError(env, *errCodePtr, "Set enable drag failed."));
        }
    };
    return complete;
}

napi_value JsWindow::OnEnableDrag(napi_env env, napi_callback_info info)
{
    size_t argc = FOUR_PARAMS_SIZE;
    napi_value argv[FOUR_PARAMS_SIZE] = { nullptr };
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc < 1 || argv[INDEX_ZERO] == nullptr) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "Argc is invalid: %{public}zu", argc);
        return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM,
            "[window][enableDrag]msg: Argc is invalid");
    }

    bool enableDrag = false;
    if (!ConvertFromJsValue(env, argv[INDEX_ZERO], enableDrag)) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "Failed to convert parameter from jsValue");
        return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM,
            "[window][enableDrag]msg: Failed to convert parameter from jsValue");
    }
    std::shared_ptr<WmErrorCode> errCodePtr = std::make_shared<WmErrorCode>(WmErrorCode::WM_OK);
    NapiAsyncTask::ExecuteCallback execute =
        GetEnableDragExecuteCallback(enableDrag, wptr<Window>(windowToken_), errCodePtr);
    NapiAsyncTask::CompleteCallback complete = GetEnableDragCompleteCallback(errCodePtr);

    napi_value result = nullptr;
    NapiAsyncTask::Schedule("JsWindow::OnEnableDrag",
        env, CreateAsyncTaskWithLastParam(env, nullptr, std::move(execute), std::move(complete), &result));
    return result;
}

/** @note @window.layout */
napi_value JsWindow::OnSetWindowLimits(napi_env env, napi_callback_info info)
{
    size_t argc = FOUR_PARAMS_SIZE;
    napi_value argv[FOUR_PARAMS_SIZE] = { nullptr };
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc < 1 || argv[INDEX_ZERO] == nullptr) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "Argc is invalid: %{public}zu", argc);
        return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM,
            "[window][setWindowLimits]msg: Argc is invalid");
    }
    WindowLimits windowLimits;
    if (!ParseWindowLimits(env, argv[INDEX_ZERO], windowLimits)) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "Failed to convert object to windowLimits");
        return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM,
            "[window][setWindowLimits]msg:Failed to convert object to windowLimits");
    }
    if (windowLimits.maxWidth_ < 0 || windowLimits.maxHeight_ < 0 ||
        windowLimits.minWidth_ < 0 || windowLimits.minHeight_ < 0) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "Width or height should be greater than or equal to 0");
        return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM,
            "[window][setWindowLimits]msg: Width or height should be greater than or equal to 0");
    }
    size_t lastParamIndex = INDEX_ONE;
    bool isForcible = false;
    if (argc >= 2 && argv[INDEX_ONE] != nullptr && GetType(env, argv[INDEX_ONE]) == napi_boolean) { // 2:params num
        lastParamIndex = INDEX_TWO;
        if (windowToken_ == nullptr) {
            TLOGE(WmsLogTag::WMS_LAYOUT, "window is nullptr");
            return NapiThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
                "[window][setWindowLimits]msg: Window is nullptr");
        }
        if (!windowToken_->IsPhonePadOrPcWindow()) {
            TLOGE(WmsLogTag::WMS_LAYOUT, "device not support");
            return NapiThrowError(env, WmErrorCode::WM_ERROR_DEVICE_NOT_SUPPORT,
                "[window][setWindowLimits]msg: Device not support");
        }
        if (!ConvertFromJsValue(env, argv[INDEX_ONE], isForcible)) {
            TLOGE(WmsLogTag::WMS_LAYOUT, "Failed to convert parameter to isForcible");
            return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM,
                "[window][setWindowLimits]msg: Failed to convert parameter to isForcible");
        }
    }
    napi_value lastParam = (argc <= lastParamIndex) ? nullptr :
                            (GetType(env, argv[lastParamIndex]) == napi_function ? argv[lastParamIndex] : nullptr);
    napi_value result = nullptr;
    std::shared_ptr<NapiAsyncTask> napiAsyncTask = CreateEmptyAsyncTask(env, lastParam, &result);
    auto asyncTask = [windowToken = wptr<Window>(windowToken_), windowLimits, isForcible,
                      env, task = napiAsyncTask, where = __func__]() mutable {
        auto window = windowToken.promote();
        if (window == nullptr) {
            TLOGNE(WmsLogTag::WMS_LAYOUT, "%{public}s: window is nullptr", where);
            task->Reject(env, JsErrUtils::CreateJsError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
                "[window][setWindowLimits]msg: Window is nullptr"));
            return;
        }
        WmErrorCode ret = WM_JS_TO_ERROR_CODE_MAP.at(window->SetWindowLimits(windowLimits, isForcible));
        if (ret == WmErrorCode::WM_OK) {
            auto objValue = GetWindowLimitsAndConvertToJsValue(env, windowLimits);
            if (objValue == nullptr) {
                task->Reject(env, JsErrUtils::CreateJsError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
                    "[window][setWindowLimits]msg: Set window limits failed"));
            } else {
                task->Resolve(env, objValue);
            }
        } else {
            task->Reject(env, JsErrUtils::CreateJsError(env, ret,
                "[window][setWindowLimits]msg: Set window limits failed"));
        }
    };
    if (napi_send_event(env, asyncTask, napi_eprio_high, "OnSetWindowLimits") != napi_status::napi_ok) {
        napiAsyncTask->Reject(env,
            JsErrUtils::CreateJsError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
                "[window][setWindowLimits]msg: Failed to send event"));
    }
    return result;
}

/** @note @window.layout */
napi_value JsWindow::OnGetWindowLimits(napi_env env, napi_callback_info info)
{
    size_t argc = 4;
    napi_value argv[4] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc > 1) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "Argc is invalid: %{public}zu", argc);
        return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM,
            "[window][getWindowLimits]msg: Argc is invalid");
    }

    if (windowToken_ == nullptr) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "window is nullptr");
        return NapiThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
            "[window][getWindowLimits]msg: Window is nullptr");
    }
    WindowLimits windowLimits;
    WmErrorCode ret = WM_JS_TO_ERROR_CODE_MAP.at(windowToken_->GetWindowLimits(windowLimits));
    if (ret != WmErrorCode::WM_OK) {
        return NapiThrowError(env, ret, "[window][getWindowLimits]msg: Falied");
    }
    auto objValue = GetWindowLimitsAndConvertToJsValue(env, windowLimits);
    TLOGI(WmsLogTag::WMS_LAYOUT, "Window [%{public}u, %{public}s] get window limits end",
        windowToken_->GetWindowId(), windowToken_->GetWindowName().c_str());
    if (objValue != nullptr) {
        return objValue;
    } else {
        return NapiThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY, "[window][getWindowLimits]msg: Nullptr");
    }
}

/** @note @window.layout */
napi_value JsWindow::OnGetWindowLimitsVP(napi_env env, napi_callback_info info)
{
    size_t argc = FOUR_PARAMS_SIZE;
    napi_value argv[FOUR_PARAMS_SIZE] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc > 1) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "Argc is invalid: %{public}zu", argc);
        return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM,
            "[window][getWindowLimitsVP]msg: Argc is invalid");
    }

    if (windowToken_ == nullptr) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "window is nullptr");
        return NapiThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
            "[window][getWindowLimitsVP]msg: Window is nullptr");
    }
    WindowLimits windowLimits;
    WmErrorCode ret = WM_JS_TO_ERROR_CODE_MAP.at(windowToken_->GetWindowLimits(windowLimits, true));
    if (ret != WmErrorCode::WM_OK) {
        return NapiThrowError(env, ret, "[window][getWindowLimitsVP]msg: Failed");
    }
    auto objValue = GetWindowLimitsAndConvertToJsValue(env, windowLimits);
    TLOGI(WmsLogTag::WMS_LAYOUT, "Id: %{public}u, name: %{public}s, getWindowLimitsVP end",
        windowToken_->GetWindowId(), windowToken_->GetWindowName().c_str());
    if (objValue != nullptr) {
        return objValue;
    } else {
        return NapiThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY, "[window][getWindowLimitsVP]msg: Nullptr");
    }
}

napi_value JsWindow::OnSetWindowDecorVisible(napi_env env, napi_callback_info info)
{
    size_t argc = FOUR_PARAMS_SIZE;
    napi_value argv[FOUR_PARAMS_SIZE] = { nullptr };
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc < 1) {
        TLOGE(WmsLogTag::WMS_DECOR, "Argc is invalid: %{public}zu", argc);
        return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM,
            "[window][setWindowDecorVisible]msg: Mandatory parameters are left unspecified.");
    }
    if (windowToken_ == nullptr) {
        TLOGE(WmsLogTag::WMS_DECOR, "WindowToken is nullptr");
        return NapiThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
            "[window][setWindowDecorVisible]msg: WindowToken is nullptr.");
    }
    bool isVisible = true;
    if (!ConvertFromJsValue(env, argv[INDEX_ZERO], isVisible)) {
        TLOGE(WmsLogTag::WMS_DECOR, "Failed to convert parameter to isVisible");
        return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM,
            "[window][setWindowDecorVisible]msg: Failed to convert parameter to isVisible.");
    }
    WmErrorCode ret = WM_JS_TO_ERROR_CODE_MAP.at(windowToken_->SetDecorVisible(isVisible));
    if (ret != WmErrorCode::WM_OK) {
        TLOGE(WmsLogTag::WMS_DECOR, "Window decor set visible failed");
        return NapiThrowError(env, ret,
            "[window][setWindowDecorVisible]msg: Window decor set visible failed.");
    }
    TLOGI(WmsLogTag::WMS_DECOR, "Window [%{public}u, %{public}s] end",
        windowToken_->GetWindowId(), windowToken_->GetWindowName().c_str());
    return NapiGetUndefined(env);
}

napi_value JsWindow::OnGetWindowDecorVisible(napi_env env, napi_callback_info info)
{
    if (windowToken_ == nullptr) {
        TLOGE(WmsLogTag::WMS_DECOR, "window is nullptr");
        return NapiThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
            "[window][getWindowDecorVisible]msg: WindowToken is nullptr.");
    }
    bool isVisible = false;
    WmErrorCode ret = WM_JS_TO_ERROR_CODE_MAP.at(windowToken_->GetDecorVisible(isVisible));
    if (ret != WmErrorCode::WM_OK) {
        TLOGE(WmsLogTag::WMS_DECOR, "Get window decor visibility failed");
        return NapiThrowError(env, ret,
            "[window][getWindowDecorVisible]msg: Get window decor visibility failed.");
    }
    TLOGI(WmsLogTag::WMS_DECOR, "end, window [%{public}u, %{public}s] isVisible=%{public}d",
        windowToken_->GetWindowId(), windowToken_->GetWindowName().c_str(), isVisible);
    return CreateJsValue(env, isVisible);
}

napi_value JsWindow::OnSetWindowTitleMoveEnabled(napi_env env, napi_callback_info info)
{
    size_t argc = FOUR_PARAMS_SIZE;
    napi_value argv[FOUR_PARAMS_SIZE] = { nullptr };
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc != 1) {
        TLOGE(WmsLogTag::WMS_DECOR, "Argc is invalid: %{public}zu", argc);
        return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM,
            "[window][setWindowTitleMoveEnabled]msg: Exactly one parameter is required.");
    }
    bool enable = true;
    if (!ConvertFromJsValue(env, argv[INDEX_ZERO], enable)) {
        TLOGE(WmsLogTag::WMS_DECOR, "Failed to convert parameter to enable");
        return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM,
            "[window][setWindowTitleMoveEnabled]msg: Failed to convert parameter to enable.");
    }
    if (windowToken_ == nullptr) {
        TLOGE(WmsLogTag::WMS_DECOR, "windowToken is nullptr");
        return NapiThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
            "[window][setWindowTitleMoveEnabled]msg: WindowToken is nullptr.");
    }
    WmErrorCode ret = WM_JS_TO_ERROR_CODE_MAP.at(windowToken_->SetWindowTitleMoveEnabled(enable));
    if (ret != WmErrorCode::WM_OK) {
        TLOGE(WmsLogTag::WMS_DECOR, "Window set title move enable failed");
        return NapiThrowError(env, ret,
            "[window][setWindowTitleMoveEnabled]msg: Window set title move enable failed.");
    }
    TLOGI(WmsLogTag::WMS_DECOR, "Window [%{public}u, %{public}s] end",
        windowToken_->GetWindowId(), windowToken_->GetWindowName().c_str());
    return NapiGetUndefined(env);
}

napi_value JsWindow::OnSetSubWindowModal(napi_env env, napi_callback_info info)
{
    size_t argc = FOUR_PARAMS_SIZE;
    napi_value argv[FOUR_PARAMS_SIZE] = { nullptr };
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc < 1 || argc > 2) { // 1: the minimum param num  2: the maximum param num
        TLOGE(WmsLogTag::WMS_SUB, "Argc is invalid: %{public}zu", argc);
        return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM,
            "[window][setSubWindowModal]msg: Argc is invalid");
    }
    bool isModal = false;
    if (!ConvertFromJsValue(env, argv[INDEX_ZERO], isModal)) {
        TLOGE(WmsLogTag::WMS_SUB, "Failed to convert parameter to isModal");
        return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM,
            "[window][setSubWindowModal]msg: Failed to convert parameter to isModal");
    }
    ModalityType modalityType = ModalityType::WINDOW_MODALITY;
    ApiModalityType apiModalityType;
    if (argc == 2 && ConvertFromJsValue(env, argv[INDEX_ONE], apiModalityType)) { // 2: the param num
        if (!isModal) {
            TLOGE(WmsLogTag::WMS_SUB, "Normal subwindow not support modalityType");
            return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM,
                "[window][setSubWindowModal]msg: Normal subwindow not support modalityType");
        }
        using T = std::underlying_type_t<ApiModalityType>;
        T type = static_cast<T>(apiModalityType);
        if (type >= static_cast<T>(ApiModalityType::BEGIN) &&
            type <= static_cast<T>(ApiModalityType::END)) {
            modalityType = JS_TO_NATIVE_MODALITY_TYPE_MAP.at(apiModalityType);
        } else {
            TLOGE(WmsLogTag::WMS_SUB, "Failed to convert parameter to modalityType");
            return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM,
                "[window][setPrivacyMode]msg: Convert parameter to modality type failed");
        }
    }
    napi_value result = nullptr;
    std::shared_ptr<NapiAsyncTask> napiAsyncTask = CreateEmptyAsyncTask(env, nullptr, &result);
    const char* const where = __func__;
    auto asyncTask = [where, weakToken = wptr<Window>(windowToken_), isModal, modalityType, env, task = napiAsyncTask] {
        auto window = weakToken.promote();
        if (window == nullptr) {
            TLOGNE(WmsLogTag::WMS_SUB, "%{public}s window is nullptr", where);
            WmErrorCode wmErrorCode = WM_JS_TO_ERROR_CODE_MAP.at(WMError::WM_ERROR_NULLPTR);
            task->Reject(env, JsErrUtils::CreateJsError(env, wmErrorCode,
                "[window][setSubWindowModal]msg: invalid window"));
            return;
        }
        if (!WindowHelper::IsSubWindow(window->GetType())) {
            TLOGNE(WmsLogTag::WMS_SUB, "%{public}s invalid call, type:%{public}d",
                where, window->GetType());
            task->Reject(env, JsErrUtils::CreateJsError(env,
                WmErrorCode::WM_ERROR_INVALID_CALLING, "[window][setSubWindowModal]msg: Invalid window type"));
            return;
        }
        WMError ret = window->SetSubWindowModal(isModal, modalityType);
        if (ret == WMError::WM_OK) {
            task->Resolve(env, NapiGetUndefined(env));
        } else {
            WmErrorCode wmErrorCode = WM_JS_TO_ERROR_CODE_MAP.at(ret);
            TLOGNE(WmsLogTag::WMS_SUB, "%{public}s set failed, ret is %{public}d", where, wmErrorCode);
            task->Reject(env, JsErrUtils::CreateJsError(env, wmErrorCode,
                "[window][setSubWindowModal]msg: set subwindow modal failed"));
            return;
        }
        TLOGNI(WmsLogTag::WMS_SUB,
            "%{public}s id:%{public}u, name:%{public}s, isModal:%{public}d, modalityType:%{public}hhu",
            where, window->GetWindowId(), window->GetWindowName().c_str(), isModal, modalityType);
    };
    if (napi_send_event(env, asyncTask, napi_eprio_high, "OnSetSubWindowModal") != napi_status::napi_ok) {
        napiAsyncTask->Reject(env,
            CreateJsError(env, static_cast<int32_t>(WmErrorCode::WM_ERROR_STATE_ABNORMALLY),
                "[window][setSubWindowModal]msg: send event failed"));
    }
    return result;
}

static std::function<void()> GetFollowParentMultiScreenPolicyTask(const wptr<Window>& weakToken, bool enabled,
    napi_env env, std::shared_ptr<NapiAsyncTask>& napiAsyncTask)
{
    return [weakToken, enabled, env, task = napiAsyncTask] {
        auto window = weakToken.promote();
        if (window == nullptr) {
            TLOGNE(WmsLogTag::WMS_SUB, "OnSetFollowParentMultiScreenPolicy failed, window is null");
            task->Reject(env, JsErrUtils::CreateJsError(env,
                WmErrorCode::WM_ERROR_STATE_ABNORMALLY, "window is null"));
            return;
        }
        if (!WindowHelper::IsSubWindow(window->GetType())) {
            TLOGNE(WmsLogTag::WMS_SUB, "OnSetFollowParentMultiScreenPolicy invalid call, type:%{public}d",
                window->GetType());
            task->Reject(env, JsErrUtils::CreateJsError(env,
                WmErrorCode::WM_ERROR_INVALID_CALLING, "invalid window type"));
            return;
        }
        WMError ret = window->SetFollowParentMultiScreenPolicy(enabled);
        if (ret != WMError::WM_OK) {
            WmErrorCode wmErrorCode = WM_JS_TO_ERROR_CODE_MAP.at(ret);
            TLOGNE(WmsLogTag::WMS_MAIN, "OnSetFollowParentMultiScreenPolicy failed, ret is %{public}d", wmErrorCode);
            task->Reject(env, JsErrUtils::CreateJsError(env,
                wmErrorCode, "Set multi-screen simultaneous display failed"));
            return;
        }
        task->Resolve(env, NapiGetUndefined(env));
        TLOGNI(WmsLogTag::WMS_SUB, "OnSetFollowParentMultiScreenPolicy id:%{public}u, name:%{public}s, "
            "enabled:%{public}d", window->GetWindowId(), window->GetWindowName().c_str(), enabled);
    };
}

napi_value JsWindow::OnSetFollowParentMultiScreenPolicy(napi_env env, napi_callback_info info)
{
    size_t argc = ONE_PARAMS_SIZE;
    napi_value argv[ONE_PARAMS_SIZE] = { nullptr };
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc != ARG_COUNT_ONE) {
        TLOGE(WmsLogTag::WMS_SUB, "Argc is invalid: %{public}zu", argc);
        return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM,
            "[window][setFollowParentMultiScreenPolicy]msg: Argc is invalid");
    }
    bool enabled = false;
    if (!ConvertFromJsValue(env, argv[INDEX_ZERO], enabled)) {
        TLOGE(WmsLogTag::WMS_SUB, "Failed to convert parameter to enabled");
        return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM,
            "[window][setFollowParentMultiScreenPolicy]msg: Failed to convert parameter to enabled");
    }
    napi_value result = nullptr;
    std::shared_ptr<NapiAsyncTask> napiAsyncTask = CreateEmptyAsyncTask(env, nullptr, &result);
    auto asyncTask = GetFollowParentMultiScreenPolicyTask(wptr<Window>(windowToken_), enabled, env, napiAsyncTask);
    napi_status status = napi_send_event(env, asyncTask, napi_eprio_high, "OnSetFollowParentMultiScreenPolicy");
    if (status != napi_status::napi_ok) {
        napiAsyncTask->Reject(env,
            CreateJsError(env, static_cast<int32_t>(WmErrorCode::WM_ERROR_STATE_ABNORMALLY),
            "[window][setFollowParentMultiScreenPolicy]msg: send event failed"));
    }
    return result;
}

napi_value JsWindow::OnSetWindowTransitionAnimation(napi_env env, napi_callback_info info)
{
    if (windowToken_ == nullptr) {
        TLOGE(WmsLogTag::WMS_ANIMATION, "Window instance not exist");
        return NapiThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
            "[window][setWindowTransitionAnimation]msg: Window instance not exist.");
    }
    WmErrorCode ret = WM_JS_TO_ERROR_CODE_MAP.at(windowToken_->IsTransitionAnimationSupported());
    if (ret != WmErrorCode::WM_OK) {
        return NapiThrowError(env, ret);
    }
    size_t argc = FOUR_PARAMS_SIZE;
    napi_value argv[FOUR_PARAMS_SIZE] = { nullptr };
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc != ARG_COUNT_TWO) {
        TLOGE(WmsLogTag::WMS_ANIMATION, "Argc is invalid: %{public}zu", argc);
        return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM,
            "[window][setWindowTransitionAnimation]msg: Incorrect number of parameters. Expected 2.");
    }
    uint32_t type = 0;
    if (!ConvertFromJsValue(env, argv[INDEX_ZERO], type) || type >= static_cast<uint32_t>(WindowTransitionType::END)) {
        TLOGE(WmsLogTag::WMS_ANIMATION, "Failed to convert parameter to type");
        return NapiThrowError(env, WmErrorCode::WM_ERROR_ILLEGAL_PARAM,
            "[window][setWindowTransitionAnimation]msg: Failed to convert parameter to type,");
    }
    TransitionAnimation animation;
    WmErrorCode convertResult = WmErrorCode::WM_OK;
    if (!ConvertTransitionAnimationFromJsValue(env, argv[INDEX_ONE], animation, convertResult)) {
        TLOGE(WmsLogTag::WMS_ANIMATION, "Failed to convert parameter to animation");
        return NapiThrowError(env, convertResult,
            "[window][setWindowTransitionAnimation]msg: Failed to convert parameter to animation,");
    }
    const char* const where = __func__;
    napi_value result = nullptr;
    std::shared_ptr<NapiAsyncTask> napiAsyncTask = CreateEmptyAsyncTask(env, nullptr, &result);
    auto asyncTask = [weakToken = wptr<Window>(windowToken_), task = napiAsyncTask, env, type, animation, where] {
        auto window = weakToken.promote();
        if (window == nullptr) {
            task->Reject(env, CreateJsError(env, static_cast<int32_t>(WmErrorCode::WM_ERROR_STATE_ABNORMALLY),
                "[window][setWindowTransitionAnimation]msg: Native window is nullptr,"));
            return;
        }
        WmErrorCode ret = WM_JS_TO_ERROR_CODE_MAP.at(window->SetWindowTransitionAnimation(
            static_cast<WindowTransitionType>(type), animation));
        if (ret == WmErrorCode::WM_OK) {
            task->Resolve(env, NapiGetUndefined(env));
        } else {
            TLOGNE(WmsLogTag::WMS_ANIMATION, "%{public}s set failed, ret is %{public}d", where, ret);
            task->Reject(env, JsErrUtils::CreateJsError(env, ret,
                "[window][setWindowTransitionAnimation]msg: Set window transition animation failed."));
        }
    };
    if (napi_send_event(env, asyncTask, napi_eprio_high, "OnSetWindowTransitionAnimation") != napi_status::napi_ok) {
        napiAsyncTask->Reject(env, JsErrUtils::CreateJsError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
            "[window][setWindowTransitionAnimation]msg: Send event failed."));
    }
    return result;
}

napi_value JsWindow::OnGetWindowTransitionAnimation(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::WMS_ANIMATION, "[NAPI]");
    if (windowToken_ == nullptr) {
        TLOGE(WmsLogTag::WMS_ANIMATION, "Window instance not exist");
        return NapiThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
            "[window][getWindowTransitionAnimation]msg: Transition animation is not enable.");
    }
    WmErrorCode ret = WM_JS_TO_ERROR_CODE_MAP.at(windowToken_->IsTransitionAnimationSupported());
    if (ret != WmErrorCode::WM_OK) {
        return NapiThrowError(env, ret,
            "[window][getWindowTransitionAnimation]msg: Transition animation is not enable.");
    }
    size_t argc = ONE_PARAMS_SIZE;
    napi_value argv[ONE_PARAMS_SIZE] = { nullptr };
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc != ARG_COUNT_ONE) {
        TLOGE(WmsLogTag::WMS_ANIMATION, "Argc is invalid: %{public}zu", argc);
        return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM,
            "[window][getWindowTransitionAnimation]msg: Exactly one parameter is required.");
    }
    WindowTransitionType type = WindowTransitionType::DESTROY;
    if (!ConvertFromJsValue(env, argv[INDEX_ZERO], type) || type >= WindowTransitionType::END) {
        TLOGE(WmsLogTag::WMS_ANIMATION, "Failed to convert parameter to type");
        return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM,
            "[window][getWindowTransitionAnimation]msg: Failed to convert parameter to type.");
    }
    napi_value result = ConvertTransitionAnimationToJsValue(env, windowToken_->GetWindowTransitionAnimation(type));
    if (result != nullptr) {
        return result;
    }
    return NapiGetUndefined(env);
}

napi_value JsWindow::OnSetWindowDecorHeight(napi_env env, napi_callback_info info)
{
    size_t argc = FOUR_PARAMS_SIZE;
    napi_value argv[FOUR_PARAMS_SIZE] = { nullptr };
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc < 1) {
        TLOGE(WmsLogTag::WMS_DECOR, "Argc is invalid: %{public}zu", argc);
        return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM,
            "[window][setWindowDecorHeight]msg: Mandatory parameters are left unspecified.");
    }
    if (windowToken_ == nullptr) {
        TLOGE(WmsLogTag::WMS_DECOR, "WindowToken is nullptr");
        return NapiThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
            "[window][setWindowDecorHeight]msg: WindowToken is nullptr.");
    }
    int32_t height = 0;
    if (!ConvertFromJsValue(env, argv[INDEX_ZERO], height)) {
        TLOGE(WmsLogTag::WMS_DECOR, "Failed to convert parameter to height");
        return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM,
            "[window][setWindowDecorHeight]msg: Failed to convert parameter to height.");
    }

    if (height < MIN_DECOR_HEIGHT || height > MAX_DECOR_HEIGHT) {
        TLOGE(WmsLogTag::WMS_DECOR, "height should greater than 37 or smaller than 112");
        return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM,
            "[window][setWindowDecorHeight]msg: Height should greater than 37 or smaller than 112.");
    }

    WmErrorCode ret = WM_JS_TO_ERROR_CODE_MAP.at(windowToken_->SetDecorHeight(height));
    if (ret != WmErrorCode::WM_OK) {
        TLOGE(WmsLogTag::WMS_DECOR, "Set window decor height failed");
        return NapiThrowError(env, ret,
            "[window][setWindowDecorHeight]msg: Set window decor height failed.");
    }
    TLOGI(WmsLogTag::WMS_DECOR, "end, window [%{public}u, %{public}s] height=%{public}d",
        windowToken_->GetWindowId(), windowToken_->GetWindowName().c_str(), height);
    return NapiGetUndefined(env);
}

napi_value JsWindow::OnGetWindowDecorHeight(napi_env env, napi_callback_info info)
{
    if (windowToken_ == nullptr) {
        TLOGE(WmsLogTag::WMS_DECOR, "window is nullptr");
        return NapiThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
            "[window][getWindowDecorHeight]msg: Window is nullptr.");
    }
    int32_t height = 0;
    WMError ret = windowToken_->GetDecorHeight(height);
    if (ret != WMError::WM_OK) {
        if (ret == WMError::WM_ERROR_DEVICE_NOT_SUPPORT) {
            return NapiThrowError(env, WmErrorCode::WM_ERROR_DEVICE_NOT_SUPPORT,
                "[window][getWindowDecorHeight]msg: Device not support.");
        }
        return NapiThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
            "[window][getWindowDecorHeight]msg: Get window decor height failed.");
    }
    TLOGI(WmsLogTag::WMS_DECOR, "end, window [%{public}u, %{public}s] height=%{public}d",
        windowToken_->GetWindowId(), windowToken_->GetWindowName().c_str(), height);
    return CreateJsValue(env, height);
}

napi_value JsWindow::OnSetDecorButtonStyle(napi_env env, napi_callback_info info)
{
    if (windowToken_ == nullptr) {
        TLOGE(WmsLogTag::WMS_DECOR, "window is nullptr");
        return NapiThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
            "[window][setDecorButtonStyle]msg: Window is nullptr.");
    }
    size_t argc = FOUR_PARAMS_SIZE;
    napi_value argv[FOUR_PARAMS_SIZE] = { nullptr };
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc != 1) {
        TLOGE(WmsLogTag::WMS_DECOR, "Argc is invalid: %{public}zu", argc);
        return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM,
            "[window][setDecorButtonStyle]msg: Exactly one parameter is required.");
    }
    DecorButtonStyle decorButtonStyle;
    WMError res = windowToken_->GetDecorButtonStyle(decorButtonStyle);
    if (res != WMError::WM_OK) {
        return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_CALLING,
            "[window][setDecorButtonStyle]msg: Called by invalid window type.");
    }
    if (argv[INDEX_ZERO] == nullptr || !ConvertDecorButtonStyleFromJs(env, argv[INDEX_ZERO], decorButtonStyle)) {
        TLOGE(WmsLogTag::WMS_DECOR, "Argc is invalid");
        return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM,
            "[window][setDecorButtonStyle]msg: Argc is invalid.");
    }
    if (!WindowHelper::CheckButtonStyleValid(decorButtonStyle)) {
        TLOGE(WmsLogTag::WMS_DECOR, "out of range params");
        return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM,
            "[window][setDecorButtonStyle]msg: Out of range params.");
    }
    WMError errCode = windowToken_->SetDecorButtonStyle(decorButtonStyle);
    WmErrorCode ret = WM_JS_TO_ERROR_CODE_MAP.at(errCode);
    if (ret != WmErrorCode::WM_OK) {
        return NapiThrowError(env, ret, "[window][setDecorButtonStyle]msg: Set decorButtonStyle failed.");
    }
    return NapiGetUndefined(env);
}

napi_value JsWindow::OnGetDecorButtonStyle(napi_env env, napi_callback_info info)
{
    if (windowToken_ == nullptr) {
        TLOGE(WmsLogTag::WMS_DECOR, "window is nullptr");
        return NapiThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
            "[window][getDecorButtonStyle]msg: Window is nullptr.");
    }
    DecorButtonStyle decorButtonStyle;
    WMError errCode = windowToken_->GetDecorButtonStyle(decorButtonStyle);
    WmErrorCode ret = WM_JS_TO_ERROR_CODE_MAP.at(errCode);
    if (ret != WmErrorCode::WM_OK) {
        return NapiThrowError(env, ret);
    }
    auto jsDecorButtonStyle = CreateJsDecorButtonStyleObj(env, decorButtonStyle);
    if (jsDecorButtonStyle == nullptr) {
        TLOGE(WmsLogTag::WMS_DECOR, "decorButtonStyle format failed");
        return NapiThrowError(env, WmErrorCode::WM_ERROR_SYSTEM_ABNORMALLY,
            "[window][getDecorButtonStyle]msg: DecorButtonStyle format failed.");
    }
    return jsDecorButtonStyle;
}

napi_value JsWindow::OnGetTitleButtonRect(napi_env env, napi_callback_info info)
{
    if (windowToken_ == nullptr) {
        TLOGE(WmsLogTag::WMS_DECOR, "window is nullptr");
        return NapiThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
            "[window][getTitleButtonRect]msg: Window is nullptr.");
    }
    TitleButtonRect titleButtonRect;
    WmErrorCode ret = WM_JS_TO_ERROR_CODE_MAP.at(windowToken_->GetTitleButtonArea(titleButtonRect));
    if (ret != WmErrorCode::WM_OK) {
        return NapiThrowError(env, ret, "[window][getTitleButtonRect]msg: Get titleButtonRect failed.");
    }
    TLOGI(WmsLogTag::WMS_DECOR, "Window [%{public}u, %{public}s] end",
        windowToken_->GetWindowId(), windowToken_->GetWindowName().c_str());
    napi_value TitleButtonAreaObj = ConvertTitleButtonAreaToJsValue(env, titleButtonRect);
    if (TitleButtonAreaObj == nullptr) {
        return NapiThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
            "[window][getTitleButtonRect]msg: TitleButtonRect convert Failed.");
    }
    return TitleButtonAreaObj;
}

napi_value JsWindow::OnSetWindowContainerColor(napi_env env, napi_callback_info info)
{
    size_t argc = FOUR_PARAMS_SIZE;
    napi_value argv[FOUR_PARAMS_SIZE] = { nullptr };
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc != 2) {  // 2: params num
        TLOGE(WmsLogTag::WMS_DECOR, "Argc is invalid: %{public}zu", argc);
        return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM,
            "[window][setWindowContainerColor]msg: Mandatory parameters are left unspecified");
    }
    if (windowToken_ == nullptr) {
        TLOGE(WmsLogTag::WMS_DECOR, "WindowToken_ is nullptr");
        return NapiThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
            "[window][setWindowContainerColor]msg: The window is not created or destroyed");
    }
    std::string activeColor;
    if (!ConvertFromJsValue(env, argv[INDEX_ZERO], activeColor)) {
        TLOGE(WmsLogTag::WMS_DECOR, "Failed to convert parameter to window container activeColor");
        return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM,
            "[window][setWindowContainerColor]msg: Failed to convert parameter to activeColor");
    }
    std::string inactiveColor;
    if (!ConvertFromJsValue(env, argv[INDEX_ONE], inactiveColor)) {
        TLOGE(WmsLogTag::WMS_DECOR, "Failed to convert parameter to window container inactiveColor");
        return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM,
            "[window][setWindowContainerColor]msg: Failed to convert parameter to inactiveColor");
    }
    WMError errCode = windowToken_->SetWindowContainerColor(activeColor, inactiveColor);
    TLOGI(WmsLogTag::WMS_DECOR, "winId: %{public}u set activeColor: %{public}s, inactiveColor: %{public}s"
        ", result: %{public}d", windowToken_->GetWindowId(), activeColor.c_str(), inactiveColor.c_str(), errCode);
    WmErrorCode ret = WM_JS_TO_ERROR_CODE_MAP.at(errCode);
    if (ret != WmErrorCode::WM_OK) {
        TLOGE(WmsLogTag::WMS_DECOR, "set window container color failed!");
        return NapiThrowError(env, ret, "[window][setWindowContainerColor]");
    }
    return NapiGetUndefined(env);
}

napi_value JsWindow::OnSetWindowContainerModalColor(napi_env env, napi_callback_info info)
{
    size_t argc = FOUR_PARAMS_SIZE;
    napi_value argv[FOUR_PARAMS_SIZE] = { nullptr };
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc != ARG_COUNT_TWO) {
        TLOGE(WmsLogTag::WMS_DECOR, "Argc is invalid: %{public}zu", argc);
        return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM);
    }
    if (windowToken_ == nullptr) {
        TLOGE(WmsLogTag::WMS_DECOR, "WindowToken_ is nullptr");
        return NapiThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
            "[window][setWindowContainerModalColor]msg: invalid window");
    }
    std::string activeColor;
    if (!ConvertFromJsValue(env, argv[INDEX_ZERO], activeColor)) {
        TLOGE(WmsLogTag::WMS_DECOR, "Failed to convert parameter to window container activeColor");
        return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM);
    }
    std::string inactiveColor;
    if (!ConvertFromJsValue(env, argv[INDEX_ONE], inactiveColor)) {
        TLOGE(WmsLogTag::WMS_DECOR, "Failed to convert parameter to window container inactiveColor");
        return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM);
    }
    WMError errCode = windowToken_->SetWindowContainerModalColor(activeColor, inactiveColor);
    TLOGI(WmsLogTag::WMS_DECOR, "Window [%{public}u, %{public}s] set activeColor: %{public}s,"
        " inactiveColor: %{public}s, result: %{public}d", windowToken_->GetWindowId(),
        windowToken_->GetWindowName().c_str(), activeColor.c_str(), inactiveColor.c_str(), errCode);
    WmErrorCode ret = WM_JS_TO_ERROR_CODE_MAP.at(errCode);
    if (ret != WmErrorCode::WM_OK) {
        TLOGE(WmsLogTag::WMS_DECOR, "set window container color failed!");
        return NapiThrowError(env, ret, "[window][setWindowContainerModalColor]msg: set container modal color failed");
    }
    return NapiGetUndefined(env);
}

napi_value JsWindow::OnSetTitleButtonVisible(napi_env env, napi_callback_info info)
{
    if (!Permission::IsSystemCalling()) {
        TLOGE(WmsLogTag::WMS_DECOR, "set title button visible permission denied!");
        return NapiThrowError(env, WmErrorCode::WM_ERROR_NOT_SYSTEM_APP);
    }
    size_t argc = FOUR_PARAMS_SIZE;
    napi_value argv[FOUR_PARAMS_SIZE] = { nullptr };
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc < 3) { // 3: params num
        TLOGE(WmsLogTag::WMS_DECOR, "Argc is invalid: %{public}zu", argc);
        return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM);
    }
    bool isMaximizeVisible = true;
    if (!ConvertFromJsValue(env, argv[INDEX_ZERO], isMaximizeVisible)) {
        TLOGE(WmsLogTag::WMS_DECOR, "Failed to convert parameter to isMaximizeVisible");
        return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM);
    }
    bool isMinimizeVisible = true;
    if (!ConvertFromJsValue(env, argv[INDEX_ONE], isMinimizeVisible)) {
        TLOGE(WmsLogTag::WMS_DECOR, "Failed to convert parameter to isMinimizeVisible");
        return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM);
    }
    bool isSplitVisible = true;
    if (!ConvertFromJsValue(env, argv[INDEX_TWO], isSplitVisible)) {
        TLOGE(WmsLogTag::WMS_DECOR, "Failed to convert parameter to isSplitVisible");
        return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM);
    }
    bool isCloseVisible = true;
    if (argc >= FOUR_PARAMS_SIZE && !ConvertFromJsValue(env, argv[INDEX_THREE], isCloseVisible)) {
        TLOGE(WmsLogTag::WMS_DECOR, "Failed to convert parameter to isCloseVisible");
        return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM);
    }
    if (windowToken_ == nullptr) {
        TLOGE(WmsLogTag::WMS_DECOR, "WindowToken is nullptr");
        return NapiThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
            "[window][setTitleButtonVisible]msg: WindowToken is nullptr");
    }
    WMError errCode = windowToken_->SetTitleButtonVisible(isMaximizeVisible, isMinimizeVisible, isSplitVisible,
        isCloseVisible);
    WmErrorCode ret = WM_JS_TO_ERROR_CODE_MAP.at(errCode);
    if (ret != WmErrorCode::WM_OK) {
        TLOGE(WmsLogTag::WMS_DECOR, "set title button visible failed!");
        return NapiThrowError(env, ret);
    }
    TLOGI(WmsLogTag::WMS_DECOR,
        "Window [%{public}u, %{public}s] end [%{public}d, %{public}d, %{public}d, %{public}d]",
        windowToken_->GetWindowId(), windowToken_->GetWindowName().c_str(), isMaximizeVisible, isMinimizeVisible,
        isSplitVisible, isCloseVisible);
    return NapiGetUndefined(env);
}

napi_value JsWindow::OnSetWindowTitleButtonVisible(napi_env env, napi_callback_info info)
{
    size_t argc = FOUR_PARAMS_SIZE;
    napi_value argv[FOUR_PARAMS_SIZE] = { nullptr };
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc < 2) { // 2: min params num
        TLOGE(WmsLogTag::WMS_DECOR, "Argc is invalid: %{public}zu", argc);
        return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM,
            "[window][setWindowTitleButtonVisible]msg: Mandatory parameters are left unspecified.");
    }
    bool isMaximizeVisible = true;
    if (!ConvertFromJsValue(env, argv[INDEX_ZERO], isMaximizeVisible)) {
        TLOGE(WmsLogTag::WMS_DECOR, "Failed to convert parameter to isMaximizeVisible");
        return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM,
            "[window][setWindowTitleButtonVisible]msg: Failed to convert parameter to isMaximizeVisible.");
    }
    bool isMinimizeVisible = true;
    if (!ConvertFromJsValue(env, argv[INDEX_ONE], isMinimizeVisible)) {
        TLOGE(WmsLogTag::WMS_DECOR, "Failed to convert parameter to isMinimizeVisible");
        return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM,
            "[window][setWindowTitleButtonVisible]msg: Failed to convert parameter to isMinimizeVisible.");
    }
    bool isCloseVisible = true;
    if (argc > 2 && !ConvertFromJsValue(env, argv[INDEX_TWO], isCloseVisible)) { // 2: min params num
        TLOGE(WmsLogTag::WMS_DECOR, "Failed to convert parameter to isCloseVisible");
        return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM,
            "[window][setWindowTitleButtonVisible]msg: Failed to convert parameter to isCloseVisible.");
    }
    if (windowToken_ == nullptr) {
        TLOGE(WmsLogTag::WMS_DECOR, "WindowToken is nullptr");
        return NapiThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
            "[window][setWindowTitleButtonVisible]msg: WindowToken is nullptr.");
    }
    WMError errCode = windowToken_->SetTitleButtonVisible(isMaximizeVisible, isMinimizeVisible, isMaximizeVisible,
        isCloseVisible);
    WmErrorCode ret = WM_JS_TO_ERROR_CODE_MAP.at(errCode);
    if (ret != WmErrorCode::WM_OK) {
        TLOGE(WmsLogTag::WMS_DECOR, "set title button visible failed!");
        return NapiThrowError(env, ret, "[window][setWindowTitleButtonVisible]msg: Set title button visible failed.");
    }
    TLOGI(WmsLogTag::WMS_DECOR, "Window [%{public}u, %{public}s] [%{public}d, %{public}d, %{public}d]",
        windowToken_->GetWindowId(), windowToken_->GetWindowName().c_str(),
        isMaximizeVisible, isMinimizeVisible, isCloseVisible);
    return NapiGetUndefined(env);
}

napi_value JsWindow::OnSetWindowTitle(napi_env env, napi_callback_info info)
{
    size_t argc = FOUR_PARAMS_SIZE;
    napi_value argv[FOUR_PARAMS_SIZE] = { nullptr };
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc != 1) {
        TLOGW(WmsLogTag::WMS_DECOR, "Argc is invalid: %{public}zu", argc);
        return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM,
            "[window][setWindowTitle]msg: Exactly one parameter is required.");
    }
    std::string title;
    if (!ConvertFromJsValue(env, argv[INDEX_ZERO], title)) {
        TLOGE(WmsLogTag::WMS_DECOR, "Failed to convert parameter to title");
        return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM,
            "[window][setWindowTitle]msg: Failed to convert parameter to title.");
    }
    napi_value result = nullptr;
    std::shared_ptr<NapiAsyncTask> napiAsyncTask = CreateEmptyAsyncTask(env, nullptr, &result);
    const char* const where = __func__;
    auto asyncTask = [windowToken = wptr<Window>(windowToken_), title, env, task = napiAsyncTask, where] {
        auto window = windowToken.promote();
        if (window == nullptr) {
            TLOGNE(WmsLogTag::WMS_DECOR, "%{public}s window is nullptr", where);
            task->Reject(env, JsErrUtils::CreateJsError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
                "[window][setWindowTitle]msg: Window is nullptr."));
            return;
        }
        WmErrorCode ret = WM_JS_TO_ERROR_CODE_MAP.at(window->SetWindowTitle(title));
        if (ret == WmErrorCode::WM_OK) {
            TLOGNI(WmsLogTag::WMS_DECOR, "%{public}s Window [%{public}u] end", where, window->GetWindowId());
            task->Resolve(env, NapiGetUndefined(env));
        } else {
            task->Reject(env, JsErrUtils::CreateJsError(env, ret,
                "[window][setWindowTitle]msg: Window set title failed."));
        }
    };
    if (napi_send_event(env, asyncTask, napi_eprio_high, "OnSetWindowTitle") != napi_status::napi_ok) {
        napiAsyncTask->Reject(env, JsErrUtils::CreateJsError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
            "[window][setWindowTitle]msg: Send event failed."));
    }
    return result;
}

napi_value JsWindow::OnSetWindowMask(napi_env env, napi_callback_info info)
{
    size_t argc = FOUR_PARAMS_SIZE;
    napi_value argv[FOUR_PARAMS_SIZE] = { nullptr };
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc < 1) {
        TLOGE(WmsLogTag::WMS_PC, "Argc is invalid: %{public}zu", argc);
        return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM,
            "[window][setWindowMask]msg: Mandatory parameters are left unspecified");
    }
    if (!CheckWindowMaskParams(env, argv[INDEX_ZERO])) {
        return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM,
            "[window][setWindowMask]msg: Incorrect parameter types");
    }
    std::vector<std::vector<uint32_t>> windowMask;
    if (!GetWindowMaskFromJsValue(env, argv[INDEX_ZERO], windowMask)) {
        TLOGE(WmsLogTag::WMS_PC, "GetWindowMaskFromJsValue failed");
        return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM,
            "[window][setWindowMask]msg: Incorrect parameter types");
    }
    napi_value result = nullptr;
    std::shared_ptr<NapiAsyncTask> napiAsyncTask = CreateEmptyAsyncTask(env, nullptr, &result);
    const char* const where = __func__;
    auto asyncTask = [weakToken = wptr<Window>(windowToken_), windowMask = std::move(windowMask), env,
        task = napiAsyncTask, where] {
        auto window = weakToken.promote();
        if (window == nullptr) {
            TLOGNE(WmsLogTag::WMS_PC, "%{public}s window is nullptr", where);
            WmErrorCode wmErrorCode = WmErrorCode::WM_ERROR_STATE_ABNORMALLY;
            task->Reject(env, JsErrUtils::CreateJsError(env, wmErrorCode,
                "[window][setWindowMask]msg: The window is not created or destroyed"));
            return;
        }
        if (!WindowHelper::IsSubWindow(window->GetType()) &&
            !WindowHelper::IsAppFloatingWindow(window->GetType())) {
            WmErrorCode wmErrorCode = WmErrorCode::WM_ERROR_INVALID_CALLING;
            task->Reject(env, JsErrUtils::CreateJsError(env, wmErrorCode, "[window][setWindowMask]msg: "
                "Invalid window type. Only sub windows and float windows are supported"));
            return;
        }
        WmErrorCode ret = WM_JS_TO_ERROR_CODE_MAP.at(window->SetWindowMask(windowMask));
        if (ret != WmErrorCode::WM_OK) {
            task->Reject(env, JsErrUtils::CreateJsError(env, ret, "[window][setWindowMask]"));
            TLOGNE(WmsLogTag::WMS_PC, "%{public}s Window [%{public}u, %{public}s]",
                where, window->GetWindowId(), window->GetWindowName().c_str());
            return;
        }
        task->Resolve(env, NapiGetUndefined(env));
        TLOGNI(WmsLogTag::WMS_PC, "%{public}s Window [%{public}u, %{public}s] end",
            where, window->GetWindowId(), window->GetWindowName().c_str());
    };
    if (napi_send_event(env, asyncTask, napi_eprio_high, "OnSetWindowMask") != napi_status::napi_ok) {
        napiAsyncTask->Reject(env, JsErrUtils::CreateJsError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
            "[window][setWindowMask]msg: Internal task error"));
    }
    return result;
}

bool JsWindow::CheckWindowMaskParams(napi_env env, napi_value jsObject)
{
    if (env == nullptr || jsObject == nullptr) {
        TLOGE(WmsLogTag::WMS_PC, "Env is nullptr or jsObject is nullptr");
        return false;
    }
    if (windowToken_ == nullptr) {
        TLOGE(WmsLogTag::WMS_PC, "windowToken is nullptr");
        return false;
    }
    uint32_t size = 0;
    napi_get_array_length(env, jsObject, &size);
    WindowLimits windowLimits;
    WmErrorCode ret = WM_JS_TO_ERROR_CODE_MAP.at(windowToken_->GetWindowLimits(windowLimits));
    if (ret == WmErrorCode::WM_OK) {
        if (size == 0 || size > windowLimits.maxWidth_) {
            TLOGE(WmsLogTag::WMS_PC, "Invalid windowMask size:%{public}u, vpRatio:%{public}f, maxWidth:%{public}u",
                size, windowLimits.vpRatio_, windowLimits.maxWidth_);
            return false;
        }
    } else {
        TLOGW(WmsLogTag::WMS_PC, "Get windowLimits failed, error code is %{public}d", ret);
        if (size == 0 || size > DEFAULT_WINDOW_MAX_WIDTH) {
            TLOGE(WmsLogTag::WMS_PC, "Invalid windowMask size:%{public}u", size);
            return false;
        }
    }
    return true;
}

void SetWindowGrayScaleTask(const wptr<Window>& weakToken, double grayScale,
    NapiAsyncTask::ExecuteCallback& execute, NapiAsyncTask::CompleteCallback& complete)
{
    std::shared_ptr<WmErrorCode> err = std::make_shared<WmErrorCode>(WmErrorCode::WM_OK);
    execute = [weakToken, grayScale, err] {
        if (err == nullptr) {
            TLOGE(WmsLogTag::WMS_ATTRIBUTE, "wm error code is null");
            return;
        }
        auto window = weakToken.promote();
        if (window == nullptr) {
            TLOGE(WmsLogTag::WMS_ATTRIBUTE, "window is null");
            *err = WmErrorCode::WM_ERROR_STATE_ABNORMALLY;
            return;
        }
        *err = WM_JS_TO_ERROR_CODE_MAP.at(window->SetGrayScale(static_cast<float>(grayScale)));
        TLOGI(WmsLogTag::WMS_ATTRIBUTE, "end, window [%{public}u, %{public}s] grayScale=%{public}f",
            window->GetWindowId(), window->GetWindowName().c_str(), grayScale);
    };

    complete = [err](napi_env env, NapiAsyncTask& task, int32_t status) {
        if (err == nullptr) {
            task.Reject(env, JsErrUtils::CreateJsError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
                "[window][setWindowGrayScale]msg: invalid error code"));
            return;
        }
        if (*err == WmErrorCode::WM_OK) {
            task.Resolve(env, NapiGetUndefined(env));
        } else {
            task.Reject(env, JsErrUtils::CreateJsError(env, *err,
                "[window][setWindowGrayScale]msg: set window gray scale failed"));
        }
    };
}

napi_value JsWindow::OnSetWindowGrayScale(napi_env env, napi_callback_info info)
{
    size_t argc = 4;
    napi_value argv[4] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc != 1) {    // 1: the param num
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "Argc is invalid: %{public}zu", argc);
        return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM);
    }
    napi_value nativeVal = argv[0];
    if (nativeVal == nullptr) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "Failed to convert parameter to grayScale");
        return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM);
    }
    double grayScale = 0.0;
    napi_get_value_double(env, nativeVal, &grayScale);
    constexpr double eps = 1e-6;
    if (grayScale < (MIN_GRAY_SCALE - eps) || grayScale > (MAX_GRAY_SCALE + eps)) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE,
            "grayScale should be greater than or equal to 0.0, and should be smaller than or equal to 1.0");
        return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM);
    }

    wptr<Window> weakToken(windowToken_);
    NapiAsyncTask::ExecuteCallback execute;
    NapiAsyncTask::CompleteCallback complete;
    SetWindowGrayScaleTask(weakToken, grayScale, execute, complete);

    napi_value result = nullptr;
    NapiAsyncTask::Schedule("JsWindow::OnSetWindowGrayScale",
        env, CreateAsyncTaskWithLastParam(env, nullptr, std::move(execute), std::move(complete), &result));
    return result;
}

napi_value JsWindow::OnSetImmersiveModeEnabledState(napi_env env, napi_callback_info info)
{
    size_t argc = 4;
    napi_value argv[4] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc != 1) {
        TLOGW(WmsLogTag::WMS_IMMS, "Argc is invalid %{public}zu", argc);
        return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM,
            "[window][setImmersiveModeEnabledState]msg: Mandatory parameters are left unspecified");
    }
    if (windowToken_ == nullptr) {
        TLOGE(WmsLogTag::WMS_IMMS, "windowToken_ is nullptr");
        return NapiThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
            "[window][setImmersiveModeEnabledState]msg: The window is not created or destroyed");
    }
    if (!WindowHelper::IsMainWindow(windowToken_->GetType()) &&
        !WindowHelper::IsSubWindow(windowToken_->GetType())) {
        TLOGE(WmsLogTag::WMS_IMMS, "not allowed since invalid window type");
        return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_CALLING, "[window][setImmersiveModeEnabledState]"
            "msg: Invalid window type. Only main windows and sub windows are supported");
    }
    WmErrorCode ret = WmErrorCode::WM_OK;
    napi_value nativeVal = argv[0];
    bool enable = true;
    if (nativeVal == nullptr) {
        TLOGE(WmsLogTag::WMS_IMMS, "Failed to convert parameter to enable");
        ret = WmErrorCode::WM_ERROR_INVALID_PARAM;
    } else {
        napi_get_value_bool(env, nativeVal, &enable);
    }
    TLOGI(WmsLogTag::WMS_IMMS, "enable %{public}d", static_cast<int32_t>(enable));
    if (windowToken_->IsPcOrPadFreeMultiWindowMode()) {
        TLOGE(WmsLogTag::WMS_IMMS, "id:%{public}u device not support", windowToken_->GetWindowId());
        windowToken_->SetIgnoreSafeArea(enable);
        return NapiGetUndefined(env);
    }
    if (ret != WmErrorCode::WM_OK) {
        return NapiThrowError(env, ret,
            "[window][setImmersiveModeEnabledState]msg: Incorrect parameter types");
    }
    ret = WM_JS_TO_ERROR_CODE_MAP.at(windowToken_->SetImmersiveModeEnabledState(enable));
    if (ret == WmErrorCode::WM_ERROR_STATE_ABNORMALLY) {
        TLOGE(WmsLogTag::WMS_IMMS, "set failed, ret %{public}d", ret);
        return NapiThrowError(env, ret,
            "[window][setImmersiveModeEnabledState]msg: The window is not created or destroyed");
    } else if (ret == WmErrorCode::WM_ERROR_SYSTEM_ABNORMALLY) {
        TLOGE(WmsLogTag::WMS_IMMS, "set failed, ret %{public}d", ret);
        return NapiThrowError(env, ret, "[window][setImmersiveModeEnabledState]msg: Internal IPC error");
    }
    TLOGI(WmsLogTag::WMS_IMMS, "win %{public}u set end", windowToken_->GetWindowId());
    return NapiGetUndefined(env);
}

napi_value JsWindow::OnGetImmersiveModeEnabledState(napi_env env, napi_callback_info info)
{
    if (windowToken_ == nullptr) {
        TLOGE(WmsLogTag::WMS_IMMS, "windowToken_ is nullptr");
        return NapiThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
            "[window][getImmersiveModeEnabledState]msg: The window is not created or destroyed");
    }
    if (!WindowHelper::IsMainWindow(windowToken_->GetType()) &&
        !WindowHelper::IsSubWindow(windowToken_->GetType())) {
        TLOGE(WmsLogTag::WMS_IMMS, "not allowed since invalid window type");
        return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_CALLING, "[window][getImmersiveModeEnabledState]msg: "
            "Invalid window type. Only main windows and sub windows are supported");
    }

    bool isEnabled = windowToken_->GetImmersiveModeEnabledState();
    TLOGI(WmsLogTag::WMS_IMMS, "win %{public}u isEnabled %{public}u set end", windowToken_->GetWindowId(), isEnabled);
    return CreateJsValue(env, isEnabled);
}

napi_value JsWindow::OnIsImmersiveLayout(napi_env env, napi_callback_info info)
{
    if (windowToken_ == nullptr) {
        TLOGE(WmsLogTag::WMS_IMMS, "windowToken_ is nullptr");
        return NapiThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
            "[window][isImmersiveLayout]msg: The window is not created or destroyed");
    }
    bool isImmersiveLayout = false;
    auto ret = WM_JS_TO_ERROR_CODE_MAP.at(windowToken_->IsImmersiveLayout(isImmersiveLayout));
    if (ret != WmErrorCode::WM_OK) {
        TLOGE(WmsLogTag::WMS_IMMS, "failed, ret %{public}d", ret);
        return NapiThrowError(env, ret, "[window][isImmersiveLayout]");
    }
    TLOGI(WmsLogTag::WMS_IMMS, "win %{public}u isImmersiveLayout %{public}u end",
        windowToken_->GetWindowId(), isImmersiveLayout);
    return CreateJsValue(env, isImmersiveLayout);
}

napi_value JsWindow::OnIsInFreeWindowMode(napi_env env, napi_callback_info info)
{
    if (windowToken_ == nullptr) {
        TLOGE(WmsLogTag::WMS_IMMS, "windowToken_ is nullptr");
        return NapiThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
            "[window][OnIsInFreeWindowMode]msg: invalid window");
    }
    bool isInFreeWindowMode = windowToken_->IsPcOrPadFreeMultiWindowMode();
    TLOGI(WmsLogTag::WMS_IMMS, "win %{public}u isInFreeWindowMod %{public}u end",
        windowToken_->GetWindowId(), isInFreeWindowMode);
    return CreateJsValue(env, isInFreeWindowMode);
}

napi_value JsWindow::OnGetWindowStatus(napi_env env, napi_callback_info info)
{
    auto window = windowToken_;
    if (window == nullptr) {
        TLOGE(WmsLogTag::WMS_PC, "window is nullptr");
        return NapiThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
            "[window][getWindowStatus]msg: Window is nullptr");
    }
    WindowStatus windowStatus;
    WmErrorCode ret = WM_JS_TO_ERROR_CODE_MAP.at(window->GetWindowStatus(windowStatus));
    if (ret != WmErrorCode::WM_OK) {
        TLOGE(WmsLogTag::WMS_PC, "failed, ret=%{public}d", ret);
        return NapiThrowError(env, ret,
            "[window][getWindowStatus]msg: Falied");
    }
    auto objValue = CreateJsValue(env, windowStatus);
    if (objValue != nullptr) {
        TLOGI(WmsLogTag::WMS_PC, "id:[%{public}u] end", window->GetWindowId());
        return objValue;
    } else {
        TLOGE(WmsLogTag::WMS_PC, "create js value windowStatus failed");
        return NapiThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
            "[window][getWindowStatus]msg: Create js value windowStatus failed");
    }
}

napi_value JsWindow::OnIsFocused(napi_env env, napi_callback_info info)
{
    auto window = windowToken_;
    if (window == nullptr) {
        TLOGE(WmsLogTag::WMS_FOCUS, "window is nullptr");
        return NapiThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
            "[window][isFocused]msg: Window is nullptr");
    }

    bool isFocused = window->IsFocused();
    TLOGI(WmsLogTag::WMS_FOCUS, "end, window [%{public}u, %{public}s] isFocused=%{public}u",
        windowToken_->GetWindowId(), windowToken_->GetWindowName().c_str(), isFocused);
    return CreateJsValue(env, isFocused);
}

static void SetRequestFocusTask(NapiAsyncTask::ExecuteCallback& execute, NapiAsyncTask::CompleteCallback& complete,
    wptr<Window> weakToken, bool isFocused)
{
    std::shared_ptr<WmErrorCode> errCodePtr = std::make_shared<WmErrorCode>(WmErrorCode::WM_OK);
    const char* const where = __func__;
    execute = [weakToken, errCodePtr, isFocused, where] {
        if (errCodePtr == nullptr) {
            return;
        }
        if (*errCodePtr != WmErrorCode::WM_OK) {
            return;
        }
        auto window = weakToken.promote();
        if (window == nullptr) {
            *errCodePtr = WmErrorCode::WM_ERROR_STATE_ABNORMALLY;
            return;
        }
        *errCodePtr = WM_JS_TO_ERROR_CODE_MAP.at(window->RequestFocusByClient(isFocused));
        TLOGNI(WmsLogTag::WMS_FOCUS, "%{public}s end, window [%{public}u, %{public}s] err=%{public}d",
            where, window->GetWindowId(), window->GetWindowName().c_str(), *errCodePtr);
    };
    complete = [weakToken, errCodePtr](napi_env env, NapiAsyncTask& task, int32_t status) {
        if (errCodePtr == nullptr) {
            task.Reject(env, JsErrUtils::CreateJsError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
                "[window][requestFocus]msg: System abnormal"));
            return;
        }
        if (*errCodePtr == WmErrorCode::WM_OK) {
            task.Resolve(env, NapiGetUndefined(env));
        } else {
            task.Reject(env, JsErrUtils::CreateJsError(env, *errCodePtr,
                "[window][requestFocus]msg: Request focus failed"));
        }
    };
}

napi_value JsWindow::OnRequestFocus(napi_env env, napi_callback_info info)
{
    if (!Permission::IsSystemCalling()) {
        TLOGE(WmsLogTag::WMS_FOCUS, "permission denied!");
        return NapiThrowError(env, WmErrorCode::WM_ERROR_NOT_SYSTEM_APP);
    }
    if (windowToken_ == nullptr) {
        TLOGE(WmsLogTag::WMS_FOCUS, "window is nullptr");
        return NapiThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
    }

    size_t argc = 4; // number of arg
    napi_value argv[4] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc != 1 || argv[0] == nullptr) { // 1: maximum params num
        TLOGE(WmsLogTag::WMS_FOCUS, "Argc is invalid: %{public}zu", argc);
        return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM);
    }

    bool isFocused = false;
    napi_status retCode = napi_get_value_bool(env, argv[0], &isFocused);
    if (retCode != napi_ok) {
        return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM);
    }
    wptr<Window> weakToken(windowToken_);
    NapiAsyncTask::ExecuteCallback execute;
    NapiAsyncTask::CompleteCallback complete;
    SetRequestFocusTask(execute, complete, weakToken, isFocused);
    // only return promise<void>
    napi_value result = nullptr;
    NapiAsyncTask::Schedule("JsWindow::OnRequestFocus",
        env, CreateAsyncTaskWithLastParam(env, nullptr, std::move(execute), std::move(complete), &result));
    return result;
}

napi_value JsWindow::OnStartMoving(napi_env env, napi_callback_info info)
{
    if (windowToken_ == nullptr) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "windowToken is nullptr.");
        return NapiThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
            "[window][startMoving]msg: WindowToken is nullptr");
    }
    size_t argc = FOUR_PARAMS_SIZE;
    napi_value argv[FOUR_PARAMS_SIZE] = { nullptr };
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc > ARG_COUNT_ZERO) {
        return OnStartMoveWindowWithCoordinate(env, argc, argv);
    }
    if (WindowHelper::IsInputWindow(windowToken_->GetType())) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "is not allowed since input window");
        return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_CALLING,
            "[window][startMoving]msg: Not allowed since input window");
    }
    std::shared_ptr<WmErrorCode> err = std::make_shared<WmErrorCode>(WmErrorCode::WM_OK);
    const char* const funcName = __func__;
    NapiAsyncTask::ExecuteCallback execute = [this, weakToken = wptr<Window>(windowToken_), err, funcName] {
        if (err == nullptr) {
            TLOGNE(WmsLogTag::WMS_LAYOUT, "%{public}s: wm error code is null.", funcName);
            return;
        }
        auto window = weakToken.promote();
        if (window == nullptr) {
            TLOGNE(WmsLogTag::WMS_LAYOUT, "%{public}s: This window is nullptr.", funcName);
            *err = WmErrorCode::WM_ERROR_STATE_ABNORMALLY;
            return;
        }
        *err = window->StartMoveWindow();
    };

    NapiAsyncTask::CompleteCallback complete = [err](napi_env env, NapiAsyncTask& task, int32_t status) {
        if (err == nullptr) {
            task.Reject(env, CreateJsError(env, static_cast<int32_t>(WmErrorCode::WM_ERROR_STATE_ABNORMALLY),
                "[window][startMoving]msg: System abnormal."));
            return;
        }
        if (*err == WmErrorCode::WM_OK) {
            task.Resolve(env, NapiGetUndefined(env));
        } else {
            task.Reject(env, CreateJsError(env, static_cast<int32_t>(*err),
            "[window][startMoving]msg: Move system window failed."));
        }
    };
    napi_value result = nullptr;
    NapiAsyncTask::Schedule("JsWindow::OnStartMoving",
        env, CreateAsyncTaskWithLastParam(env, nullptr, std::move(execute), std::move(complete), &result));
    return result;
}

napi_value JsWindow::OnStartMoveWindowWithCoordinate(napi_env env, size_t argc, napi_value* argv)
{
    if (windowToken_ == nullptr) {
        TLOGE(WmsLogTag::WMS_LAYOUT_PC, "windowToken is nullptr.");
        return NapiThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
    }
    if (argc != ARG_COUNT_TWO) {
        TLOGE(WmsLogTag::WMS_LAYOUT_PC, "Argc is invalid: %{public}zu", argc);
        return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM);
    }
    int32_t offsetX;
    if (!ConvertFromJsValue(env, argv[INDEX_ZERO], offsetX)) {
        TLOGE(WmsLogTag::WMS_LAYOUT_PC, "failed to convert parameter to offsetX");
        return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM);
    }
    int32_t offsetY;
    if (!ConvertFromJsValue(env, argv[INDEX_ONE], offsetY)) {
        TLOGE(WmsLogTag::WMS_LAYOUT_PC, "failed to convert parameter to offsetY");
        return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM);
    }
    napi_value result = nullptr;
    std::shared_ptr<NapiAsyncTask> napiAsyncTask = CreateEmptyAsyncTask(env, nullptr, &result);
    auto asyncTask = [windowToken = wptr<Window>(windowToken_), offsetX, offsetY,
                      env, task = napiAsyncTask, where = __func__] {
        auto window = windowToken.promote();
        if (window == nullptr) {
            TLOGNE(WmsLogTag::WMS_LAYOUT_PC, "%{public}s window is nullptr.", where);
            task->Reject(env, JsErrUtils::CreateJsError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY));
            return;
        }
        WmErrorCode ret = window->StartMoveWindowWithCoordinate(offsetX, offsetY);
        if (ret == WmErrorCode::WM_OK) {
            task->Resolve(env, NapiGetUndefined(env));
        } else {
            task->Reject(env, JsErrUtils::CreateJsError(env, ret, "move window failed"));
        }
    };
    napi_status status = napi_send_event(env, std::move(asyncTask),
                                         napi_eprio_high, "OnStartMoveWindowWithCoordinate");
    if (status != napi_status::napi_ok) {
        napiAsyncTask->Reject(env, CreateJsError(env,
            static_cast<int32_t>(WmErrorCode::WM_ERROR_STATE_ABNORMALLY), "send event failed"));
    }
    return result;
}

napi_value JsWindow::OnStopMoving(napi_env env, napi_callback_info info)
{
    if (windowToken_ == nullptr) {
        TLOGE(WmsLogTag::WMS_LAYOUT_PC, "windowToken is nullptr.");
        return NapiThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
            "[window][stopMoving]msg: WindowToken is nullptr");
    }
    napi_value result = nullptr;
    std::shared_ptr<NapiAsyncTask> napiAsyncTask = CreateEmptyAsyncTask(env, nullptr, &result);
    const char* const where = __func__;
    auto asyncTask = [windowToken = wptr<Window>(windowToken_), env, task = napiAsyncTask, where] {
        auto window = windowToken.promote();
        if (window == nullptr) {
            TLOGNE(WmsLogTag::WMS_LAYOUT_PC, "%{public}s window is nullptr.", where);
            task->Reject(env, JsErrUtils::CreateJsError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
                "[window][stopMoving]msg: Window is nullptr"));
            return;
        }
        WmErrorCode ret = window->StopMoveWindow();
        if (ret == WmErrorCode::WM_OK) {
            task->Resolve(env, NapiGetUndefined(env));
        } else {
            task->Reject(env, JsErrUtils::CreateJsError(env, ret,
                "[window][stopMoving]msg: Stop moving window failed"));
        }
    };
    if (napi_send_event(env, std::move(asyncTask), napi_eprio_high, "OnStopMoving") != napi_status::napi_ok) {
        napiAsyncTask->Reject(env, CreateJsError(env,
            static_cast<int32_t>(WmErrorCode::WM_ERROR_STATE_ABNORMALLY),
            "[window][stopMoving]msg: Send event failed"));
    }
    return result;
}

napi_value JsWindow::OnSetGestureBackEnabled(napi_env env, napi_callback_info info)
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
    auto execute = [weakToken = wptr<Window>(windowToken_), errCodePtr, enabled, where] {
        auto window = weakToken.promote();
        if (window == nullptr) {
            TLOGNE(WmsLogTag::WMS_IMMS, "%{public}s window is nullptr", where);
            *errCodePtr = WmErrorCode::WM_ERROR_STATE_ABNORMALLY;
            return;
        }
        if (!WindowHelper::IsMainWindow(window->GetType())) {
            TLOGNE(WmsLogTag::WMS_IMMS, "%{public}s invalid window type", where);
            *errCodePtr = WmErrorCode::WM_ERROR_INVALID_CALLING;
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
    NapiAsyncTask::Schedule("JsWindow::OnSetGestureBackEnabled",
        env, CreateAsyncTaskWithLastParam(env, nullptr, std::move(execute), std::move(complete), &result));
    return result;
}

napi_value JsWindow::OnGetGestureBackEnabled(napi_env env, napi_callback_info info)
{
    if (windowToken_ == nullptr) {
        TLOGE(WmsLogTag::WMS_IMMS, "windowToken is nullptr");
        return NapiThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
            "[window][isGestureBackEnabled]msg: The window is not created or destroyed");
    }
    if (!WindowHelper::IsMainWindow(windowToken_->GetType())) {
        TLOGE(WmsLogTag::WMS_IMMS, "get failed since invalid window type");
        return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_CALLING,
            "[window][isGestureBackEnabled]msg: Invalid window type. Only main windows are supported.");
    }
    bool enable = true;
    WmErrorCode ret = WM_JS_TO_ERROR_CODE_MAP.at(windowToken_->GetGestureBackEnabled(enable));
    if (ret == WmErrorCode::WM_ERROR_DEVICE_NOT_SUPPORT) {
        TLOGE(WmsLogTag::WMS_IMMS, "device is not support");
        return NapiThrowError(env, WmErrorCode::WM_ERROR_DEVICE_NOT_SUPPORT,
            "[window][isGestureBackEnabled]");
    } else if (ret != WmErrorCode::WM_OK) {
        TLOGE(WmsLogTag::WMS_IMMS, "get failed, ret %{public}d", ret);
        return NapiThrowError(env, WmErrorCode::WM_ERROR_SYSTEM_ABNORMALLY,
            "[window][isGestureBackEnabled]");
    }
    TLOGI(WmsLogTag::WMS_IMMS, "win [%{public}u, %{public}s] enable %{public}u",
        windowToken_->GetWindowId(), windowToken_->GetWindowName().c_str(), enable);
    return CreateJsValue(env, enable);
}

napi_value JsWindow::OnCreateSubWindowWithOptions(napi_env env, napi_callback_info info)
{
    if (windowToken_ == nullptr) {
        TLOGE(WmsLogTag::WMS_SUB, "window is null");
        napi_throw(env, JsErrUtils::CreateJsError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
            "[window][createSubWindowWithOptions]msg: Window is nullptr."));
        return NapiGetUndefined(env);
    }
    if (!windowToken_->IsPcOrFreeMultiWindowCapabilityEnabled()) {
        TLOGE(WmsLogTag::WMS_SUB, "device not support");
        return NapiGetUndefined(env);
    }
    size_t argc = FOUR_PARAMS_SIZE;
    napi_value argv[FOUR_PARAMS_SIZE] = { nullptr };
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc < 2) { // 2: minimum params num
        TLOGE(WmsLogTag::WMS_SUB, "Argc is invalid: %{public}zu", argc);
        return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM);
    }
    std::string windowName;
    if (!ConvertFromJsValue(env, argv[INDEX_ZERO], windowName)) {
        TLOGE(WmsLogTag::WMS_SUB, "Failed to convert parameter to windowName");
        napi_throw(env, JsErrUtils::CreateJsError(env, WmErrorCode::WM_ERROR_INVALID_PARAM));
        return NapiGetUndefined(env);
    }
    sptr<WindowOption> windowOption = new WindowOption();
    if (!ParseSubWindowOptions(env, argv[INDEX_ONE], windowOption)) {
        TLOGE(WmsLogTag::WMS_SUB, "Failed to convert parameter to options");
        napi_throw(env, JsErrUtils::CreateJsError(env, WmErrorCode::WM_ERROR_INVALID_PARAM));
        return NapiGetUndefined(env);
    }
    if ((windowOption->GetWindowFlags() & static_cast<uint32_t>(WindowFlag::WINDOW_FLAG_IS_APPLICATION_MODAL)) &&
        !windowToken_->IsPcOrPadFreeMultiWindowMode()) {
        TLOGE(WmsLogTag::WMS_SUB, "device not support");
        napi_throw(env, JsErrUtils::CreateJsError(env, WmErrorCode::WM_ERROR_DEVICE_NOT_SUPPORT));
        return NapiGetUndefined(env);
    }
    if (windowOption->GetWindowTopmost() && !Permission::IsSystemCalling() && !Permission::IsStartByHdcd()) {
        TLOGE(WmsLogTag::WMS_SUB, "Modal subwindow has topmost, but no system permission");
        napi_throw(env, JsErrUtils::CreateJsError(env, WmErrorCode::WM_ERROR_NOT_SYSTEM_APP));
        return NapiGetUndefined(env);
    }
    napi_value callback = (argc > 2 && argv[2] != nullptr && GetType(env, argv[2]) == napi_function) ?
        argv[2] : nullptr;
    napi_value result = nullptr;
    std::shared_ptr<NapiAsyncTask> napiAsyncTask = CreateEmptyAsyncTask(env, callback, &result);
    const char* const where = __func__;
    auto asyncTask = [windowToken = wptr<Window>(windowToken_), windowName = std::move(windowName),
        windowOption, env, task = napiAsyncTask, where]() mutable {
        auto window = windowToken.promote();
        if (window == nullptr) {
            TLOGNE(WmsLogTag::WMS_SUB, "%{public}s window is nullptr", where);
            task->Reject(env, JsErrUtils::CreateJsError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
                "[window][createSubWindowWithOptions]msg: Window is nullptr on asyncTask."));
            return;
        }
        if (!WindowHelper::IsFloatOrSubWindow(window->GetType()) &&
            !WindowHelper::IsMainWindow(window->GetType())) {
            TLOGNE(WmsLogTag::WMS_SUB, "%{public}s invalid window type: %{public}d", where, window->GetType());
            task->Reject(env, JsErrUtils::CreateJsError(env, WmErrorCode::WM_ERROR_INVALID_CALLING,
                "invalid window type"));
            return;
        }
        windowOption->SetWindowType(WindowType::WINDOW_TYPE_APP_SUB_WINDOW);
        windowOption->SetWindowMode(WindowMode::WINDOW_MODE_FLOATING);
        windowOption->SetOnlySupportSceneBoard(true);
        windowOption->SetParentId(windowToken->GetWindowId());
        windowOption->SetWindowTag(WindowTag::SUB_WINDOW);
        auto subWindow = Window::Create(windowName, windowOption, window->GetContext());
        if (subWindow == nullptr) {
            TLOGNE(WmsLogTag::WMS_SUB, "%{public}s create sub window failed.", where);
            task->Reject(env, JsErrUtils::CreateJsError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
                "[window][createSubWindowWithOptions]msg: Create sub window failed."));
            return;
        }
        task->Resolve(env, CreateJsWindowObject(env, subWindow));
        TLOGNI(WmsLogTag::WMS_SUB, "%{public}s create sub window %{public}s end", where, windowName.c_str());
    };
    if (napi_send_event(env, asyncTask, napi_eprio_vip, "OnCreateSubWindowWithOptions") != napi_status::napi_ok) {
        napiAsyncTask->Reject(env, CreateJsError(env, static_cast<int32_t>(WmErrorCode::WM_ERROR_STATE_ABNORMALLY),
            "[window][createSubWindowWithOptions]msg: Send event failed."));
    }
    return result;
}

napi_value JsWindow::OnSetParentWindow(napi_env env, napi_callback_info info)
{
    size_t argc = FOUR_PARAMS_SIZE;
    napi_value argv[FOUR_PARAMS_SIZE] = { nullptr };
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc != ARG_COUNT_ONE) {
        TLOGE(WmsLogTag::WMS_SUB, "Argc is invalid: %{public}zu", argc);
        return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM);
    }
    int32_t newParentWindowId = INVALID_WINDOW_ID;
    if (!ConvertFromJsValue(env, argv[INDEX_ZERO], newParentWindowId)) {
        TLOGE(WmsLogTag::WMS_SUB, "Failed to convert parameter to newParentWindowId");
        return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM);
    }
    napi_value result = nullptr;
    std::shared_ptr<NapiAsyncTask> napiAsyncTask = CreateEmptyAsyncTask(env, nullptr, &result);
    auto asyncTask = [weakToken = wptr<Window>(windowToken_), newParentWindowId, env,
                      task = napiAsyncTask, where = __func__] {
        auto window = weakToken.promote();
        if (window == nullptr) {
            TLOGNE(WmsLogTag::WMS_SUB, "%{public}s: window is nullptr", where);
            task->Reject(env, JsErrUtils::CreateJsError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
                "[window][setParentWindow]msg: Window is nullptr."));
            return;
        }
        WMError ret = window->SetParentWindow(newParentWindowId);
        if (ret != WMError::WM_OK) {
            WmErrorCode wmErrorCode = WM_JS_TO_ERROR_CODE_MAP.at(ret);
            task->Reject(env, JsErrUtils::CreateJsError(env, wmErrorCode,
                "[window][setParentWindow]msg: Set parent window failed."));
            return;
        }
        task->Resolve(env, NapiGetUndefined(env));
        TLOGNI(WmsLogTag::WMS_SUB, "%{public}s window id: %{public}u set parent window id: %{public}u end",
            where, window->GetWindowId(), newParentWindowId);
    };
    if (napi_send_event(env, std::move(asyncTask), napi_eprio_high, "OnSetParentWindow") != napi_status::napi_ok) {
        napiAsyncTask->Reject(env, CreateJsError(env, static_cast<int32_t>(WmErrorCode::WM_ERROR_STATE_ABNORMALLY),
            "[window][setParentWindow]msg: Send event failed."));
    }
    return result;
}

napi_value JsWindow::OnGetParentWindow(napi_env env, napi_callback_info info)
{
    if (windowToken_ == nullptr) {
        TLOGE(WmsLogTag::WMS_SUB, "windowToken is nullptr");
        return NapiThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
    }
    sptr<Window> parentWindow = nullptr;
    WmErrorCode ret = WM_JS_TO_ERROR_CODE_MAP.at(windowToken_->GetParentWindow(parentWindow));
    if (ret != WmErrorCode::WM_OK) {
        TLOGE(WmsLogTag::WMS_SUB, "get failed, result=%{public}d", ret);
        return NapiThrowError(env, ret, "[window][getParentWindow]msg: Get parent window failed.");
    }
    if (parentWindow == nullptr) {
        TLOGE(WmsLogTag::WMS_SUB, "parentWindow is nullptr");
        return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARENT);
    }
    auto objValue = CreateJsWindowObject(env, parentWindow);
    if (objValue == nullptr) {
        TLOGE(WmsLogTag::WMS_SUB, "create js window failed");
        return NapiThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
            "[window][getParentWindow]msg: Create js window object failed.");
    }
    TLOGI(WmsLogTag::WMS_SUB, "window id: %{public}u set parent window id: %{public}u end",
        windowToken_->GetWindowId(), parentWindow->GetWindowId());
    return objValue;
}

napi_value JsWindow::OnGetWindowDensityInfo(napi_env env, napi_callback_info info)
{
    if (windowToken_ == nullptr) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "windowToken is null");
        return NapiThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
            "[window][getWindowDensityInfo]msg: The window is not created or destroyed");
    }
    WindowDensityInfo densityInfo;
    WmErrorCode ret = WM_JS_TO_ERROR_CODE_MAP.at(windowToken_->GetWindowDensityInfo(densityInfo));
    if (ret != WmErrorCode::WM_OK) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "get failed, result=%{public}d", ret);
        return NapiThrowError(env, ret, "[window][getWindowDensityInfo]");
    }
    auto objValue = ConvertWindowDensityInfoToJsValue(env, densityInfo);
    if (objValue != nullptr) {
        TLOGD(WmsLogTag::WMS_ATTRIBUTE, "win [%{public}u, %{public}s] get density: %{public}s",
            windowToken_->GetWindowId(), windowToken_->GetWindowName().c_str(), densityInfo.ToString().c_str());
        return objValue;
    } else {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "create js windowDensityInfo failed");
        return NapiThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
            "[window][getWindowDensityInfo]msg: Internal task error");
    }
}

napi_value JsWindow::OnSetDefaultDensityEnabled(napi_env env, napi_callback_info info)
{
    size_t argc = FOUR_PARAMS_SIZE;
    napi_value argv[FOUR_PARAMS_SIZE] = { nullptr };
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc != INDEX_ONE) {
        TLOGW(WmsLogTag::WMS_ATTRIBUTE, "Argc is invalid %{public}zu", argc);
        return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM,
            "[window][setDefaultDensityEnabled]msg: Mandatory parameters are left unspecified");
    }
    bool enabled = false;
    if (!ConvertFromJsValue(env, argv[INDEX_ZERO], enabled)) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "Failed to convert parameter to enable");
        return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM,
            "[window][setDefaultDensityEnabled]msg: Incorrect parameter types");
    }
    if (windowToken_ == nullptr) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "windowToken is null");
        return NapiThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
            "[window][setDefaultDensityEnabled]msg: The window is not created or destroyed");
    }
    WmErrorCode ret = WM_JS_TO_ERROR_CODE_MAP.at(windowToken_->SetWindowDefaultDensityEnabled(enabled));
    TLOGI(WmsLogTag::WMS_ATTRIBUTE, "winId: %{public}u set enabled=%{public}u result=%{public}d",
        windowToken_->GetWindowId(), enabled, ret);
    if (ret != WmErrorCode::WM_OK) {
        return NapiThrowError(env, ret, "[window][setDefaultDensityEnabled]");
    }
    return NapiGetUndefined(env);
}

napi_value JsWindow::OnIsMainWindowFullScreenAcrossDisplays(napi_env env, napi_callback_info info)
{
    if (windowToken_ == nullptr) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "windowToken is null");
        return NapiThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
            "[window][isMainWindowFullScreenAcrossDisplays]msg: invalid window");
    }
    std::shared_ptr<bool> isAcrossDisplaysPtr = std::make_shared<bool>(false);
    std::shared_ptr<WmErrorCode> errCodePtr = std::make_shared<WmErrorCode>(WmErrorCode::WM_OK);
    const char* const where = __func__;
    auto execute = [weakToken = wptr<Window>(windowToken_), isAcrossDisplaysPtr, errCodePtr, where] {
        auto window = weakToken.promote();
        if (window == nullptr) {
            TLOGNE(WmsLogTag::WMS_ATTRIBUTE, "%{public}s window is null", where);
            *errCodePtr = WmErrorCode::WM_ERROR_STATE_ABNORMALLY;
            return;
        }
        *errCodePtr =
            WM_JS_TO_ERROR_CODE_MAP.at(window->IsMainWindowFullScreenAcrossDisplays(*isAcrossDisplaysPtr));
        TLOGNI(WmsLogTag::WMS_ATTRIBUTE, "%{public}s winId: %{public}u, isAcrossDisplays: %{public}u, "
            "result: %{public}d", where, window->GetWindowId(), *isAcrossDisplaysPtr, *errCodePtr);
    };
    auto complete = [isAcrossDisplaysPtr, errCodePtr, where](napi_env env, NapiAsyncTask& task, int32_t status) {
        if (*errCodePtr == WmErrorCode::WM_OK) {
            auto objValue = CreateJsValue(env, *isAcrossDisplaysPtr);
            if (objValue != nullptr) {
                task.Resolve(env, objValue);
            } else {
                TLOGNE(WmsLogTag::WMS_ATTRIBUTE, "%{public}s convert to js value failed", where);
                task.Reject(env, JsErrUtils::CreateJsError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
                    "[window][isMainWindowFullScreenAcrossDisplays]msg: create js value failed"));
            }
        } else {
            task.Reject(env, JsErrUtils::CreateJsError(env, *errCodePtr,
                "[window][isMainWindowFullScreenAcrossDisplays]msg: query failed"));
        }
    };
    napi_value result = nullptr;
    NapiAsyncTask::Schedule("JsWindow::OnIsMainWindowFullScreenAcrossDisplays",
        env, CreateAsyncTaskWithLastParam(env, nullptr, std::move(execute), std::move(complete), &result));
    return result;
}

napi_value JsWindow::OnSetSystemAvoidAreaEnabled(napi_env env, napi_callback_info info)
{
    size_t argc = FOUR_PARAMS_SIZE;
    napi_value argv[FOUR_PARAMS_SIZE] = { nullptr };
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc < INDEX_ONE) {
        TLOGE(WmsLogTag::WMS_IMMS, "argc is invalid: %{public}zu", argc);
        return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM,
            "[window][setSystemAvoidAreaEnabled]msg: Mandatory parameters are left unspecified");
    }
    bool enable = false;
    if (!ConvertFromJsValue(env, argv[INDEX_ZERO], enable)) {
        TLOGE(WmsLogTag::WMS_IMMS, "Failed to convert parameter to enable");
        return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM,
            "[window][setSystemAvoidAreaEnabled]msg: Incorrect parameter types");
    }
    std::shared_ptr<WmErrorCode> errCodePtr = std::make_shared<WmErrorCode>(WmErrorCode::WM_OK);
    const char* const where = __func__;
    auto execute = [weakToken = wptr<Window>(windowToken_), errCodePtr, enable, where] {
        auto window = weakToken.promote();
        if (window == nullptr) {
            TLOGNE(WmsLogTag::WMS_IMMS, "%{public}s window is nullptr", where);
            *errCodePtr = WmErrorCode::WM_ERROR_STATE_ABNORMALLY;
            return;
        }
        if (!WindowHelper::IsSystemWindow(window->GetType())) {
            TLOGNE(WmsLogTag::WMS_IMMS, "%{public}s only system window is valid", where);
            *errCodePtr = WmErrorCode::WM_ERROR_INVALID_CALLING;
            return;
        }
        uint32_t option = 0;
        *errCodePtr = WM_JS_TO_ERROR_CODE_MAP.at(window->GetAvoidAreaOption(option));
        if (*errCodePtr != WmErrorCode::WM_OK) {
            return;
        }
        if (enable) {
            option |= static_cast<uint32_t>(AvoidAreaOption::ENABLE_SYSTEM_WINDOW);
        } else {
            option &= ~static_cast<uint32_t>(AvoidAreaOption::ENABLE_SYSTEM_WINDOW);
        }
        *errCodePtr = WM_JS_TO_ERROR_CODE_MAP.at(window->SetAvoidAreaOption(option));
    };
    auto complete = [errCodePtr, where](napi_env env, NapiAsyncTask& task, int32_t status) {
        if (*errCodePtr == WmErrorCode::WM_OK) {
            task.Resolve(env, NapiGetUndefined(env));
        } else {
            TLOGNE(WmsLogTag::WMS_IMMS, "%{public}s failed, ret %{public}d", where, *errCodePtr);
            task.Reject(env, JsErrUtils::CreateJsError(env, *errCodePtr, "[window][setSystemAvoidAreaEnabled]"));
        }
    };
    napi_value result = nullptr;
    NapiAsyncTask::Schedule("JsWindow::OnSetSystemAvoidAreaEnabled",
        env, CreateAsyncTaskWithLastParam(env, nullptr, std::move(execute), std::move(complete), &result));
    return result;
}

napi_value JsWindow::OnIsSystemAvoidAreaEnabled(napi_env env, napi_callback_info info)
{
    if (windowToken_ == nullptr) {
        TLOGE(WmsLogTag::WMS_IMMS, "windowToken is nullptr");
        return NapiThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
            "[window][isSystemAvoidAreaEnabled]msg: The window is not created or destroyed");
    }
    if (!WindowHelper::IsSystemWindow(windowToken_->GetType())) {
        TLOGE(WmsLogTag::WMS_IMMS, "only system window is valid");
        return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_CALLING,
            "[window][isSystemAvoidAreaEnabled]msg: Invalid window type.");
    }
    uint32_t avoidAreaOption = 0;
    WmErrorCode ret = WM_JS_TO_ERROR_CODE_MAP.at(windowToken_->GetAvoidAreaOption(avoidAreaOption));
    if (ret != WmErrorCode::WM_OK) {
        TLOGE(WmsLogTag::WMS_IMMS, "get failed, ret %{public}d", ret);
        return NapiThrowError(env, ret, "[window][isSystemAvoidAreaEnabled]");
    }
    bool enabled = avoidAreaOption & static_cast<uint32_t>(AvoidAreaOption::ENABLE_SYSTEM_WINDOW);
    if (auto objValue = CreateJsValue(env, enabled)) {
        TLOGI(WmsLogTag::WMS_IMMS, "win %{public}u enabled %{public}u", windowToken_->GetWindowId(), enabled);
        return objValue;
    } else {
        TLOGE(WmsLogTag::WMS_IMMS, "create js object failed");
        return NapiThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
            "[window][isSystemAvoidAreaEnabled]msg: Create js value failed");
    }
}

napi_value JsWindow::OnSetExclusivelyHighlighted(napi_env env, napi_callback_info info)
{
    size_t argc = FOUR_PARAMS_SIZE;
    napi_value argv[FOUR_PARAMS_SIZE] = { nullptr };
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc != ARG_COUNT_ONE) {
        TLOGE(WmsLogTag::WMS_FOCUS, "argc is invalid: %{public}zu", argc);
        return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM,
            "[window][setExclusivelyHighlighted]msg: Argc is invalid");
    }
    bool exclusivelyHighlighted = true;
    if (!ConvertFromJsValue(env, argv[INDEX_ZERO], exclusivelyHighlighted)) {
        TLOGE(WmsLogTag::WMS_FOCUS, "Failed to convert parameter to exclusivelyHighlighted");
        return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM,
            "[window][setExclusivelyHighlighted]msg: Failed to convert parameter to exclusivelyHighlighted");
    }
    napi_value result = nullptr;
    std::shared_ptr<NapiAsyncTask> napiAsyncTask = CreateEmptyAsyncTask(env, nullptr, &result);
    auto asyncTask = [weakToken = wptr<Window>(windowToken_), exclusivelyHighlighted, env,
                      task = napiAsyncTask, where = __func__] {
        auto window = weakToken.promote();
        if (window == nullptr) {
            TLOGNE(WmsLogTag::WMS_FOCUS, "%{public}s: window is nullptr", where);
            task->Reject(env, JsErrUtils::CreateJsError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
                "[window][setExclusivelyHighlighted]msg: Window is nullptr"));
            return;
        }
        WMError ret = window->SetExclusivelyHighlighted(exclusivelyHighlighted);
        if (ret == WMError::WM_OK) {
            task->Resolve(env, NapiGetUndefined(env));
        } else {
            WmErrorCode wmErrorCode = WM_JS_TO_ERROR_CODE_MAP.at(ret);
            task->Reject(env, JsErrUtils::CreateJsError(env, wmErrorCode,
                "[window][setExclusivelyHighlighted]msg: Set exclusively highlighted failed"));
        }
        TLOGNI(WmsLogTag::WMS_FOCUS, "%{public}s: end, window: [%{public}u, %{public}s]",
            where, window->GetWindowId(), window->GetWindowName().c_str());
    };
    if (napi_send_event(env, asyncTask, napi_eprio_high, "OnSetExclusivelyHighlighted") != napi_status::napi_ok) {
        napiAsyncTask->Reject(env, CreateJsError(env,
            static_cast<int32_t>(WmErrorCode::WM_ERROR_STATE_ABNORMALLY),
                "[window][setExclusivelyHighlighted]msg: Failed to send event"));
    }
    return result;
}

napi_value JsWindow::OnIsWindowHighlighted(napi_env env, napi_callback_info info)
{
    if (windowToken_ == nullptr) {
        TLOGE(WmsLogTag::WMS_FOCUS, "windowToken is nullptr");
        return NapiThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
            "[window][isWindowHighlighted]msg: WindowToken is nullptr");
    }
    bool isHighlighted = false;
    WmErrorCode ret = WM_JS_TO_ERROR_CODE_MAP.at(windowToken_->IsWindowHighlighted(isHighlighted));
    if (ret != WmErrorCode::WM_OK) {
        TLOGE(WmsLogTag::WMS_FOCUS, "get window highlight failed, ret: %{public}d", ret);
        return NapiThrowError(env, ret, "[window][isWindowHighlighted]");
    }
    TLOGI(WmsLogTag::WMS_FOCUS, "get window highlight end, isHighlighted: %{public}u", isHighlighted);
    return CreateJsValue(env, isHighlighted);
}

static void SetDragKeyFramePolicyTask(NapiAsyncTask::ExecuteCallback& execute,
    NapiAsyncTask::CompleteCallback& complete, const wptr<Window>& weakToken, const KeyFramePolicy& keyFramePolicy)
{
    std::shared_ptr<WmErrorCode> errCodePtr = std::make_shared<WmErrorCode>(WmErrorCode::WM_OK);
    const char* const where = __func__;
    execute = [weakToken, keyFramePolicy, errCodePtr, where] {
        if (errCodePtr == nullptr) {
            TLOGNE(WmsLogTag::WMS_LAYOUT_PC, "%{public}s errCodePtr is nullptr", where);
            return;
        }
        auto window = weakToken.promote();
        if (window == nullptr) {
            TLOGNE(WmsLogTag::WMS_LAYOUT_PC, "%{public}s window is nullptr", where);
            *errCodePtr = WmErrorCode::WM_ERROR_STATE_ABNORMALLY;
            return;
        }
        auto result = window->SetDragKeyFramePolicy(keyFramePolicy);
        auto iter = WM_JS_TO_ERROR_CODE_MAP.find(result);
        if (iter == WM_JS_TO_ERROR_CODE_MAP.end()) {
            TLOGNE(WmsLogTag::WMS_LAYOUT_PC, "%{public}s convert to WmErrorCode failed: %{public}d", where, result);
            *errCodePtr = WmErrorCode::WM_ERROR_STATE_ABNORMALLY;
        } else {
            *errCodePtr = iter->second;
        }
    };
    complete = [keyFramePolicy, errCodePtr, where](napi_env env, NapiAsyncTask& task, int32_t status) {
        if (errCodePtr == nullptr) {
            task.Reject(env, JsErrUtils::CreateJsError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY));
            return;
        }
        if (*errCodePtr == WmErrorCode::WM_OK) {
            auto objValue = ConvertKeyFramePolicyToJsValue(env, keyFramePolicy);
            if (objValue != nullptr) {
                task.Resolve(env, objValue);
            } else {
                TLOGNE(WmsLogTag::WMS_LAYOUT_PC, "%{public}s convert to js value failed", where);
                task.Reject(env, JsErrUtils::CreateJsError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY));
            }
        } else {
            TLOGNE(WmsLogTag::WMS_LAYOUT_PC, "%{public}s failed, ret %{public}d", where, *errCodePtr);
            task.Reject(env, JsErrUtils::CreateJsError(env, *errCodePtr, "set key frame policy failed"));
        }
    };
}

napi_value JsWindow::OnSetDragKeyFramePolicy(napi_env env, napi_callback_info info)
{
    size_t argc = FOUR_PARAMS_SIZE;
    napi_value argv[FOUR_PARAMS_SIZE] = { nullptr };
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc != INDEX_ONE || argv[INDEX_ZERO] == nullptr) {
        TLOGE(WmsLogTag::WMS_LAYOUT_PC, "argc is invalid: %{public}zu", argc);
        return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM);
    }
    KeyFramePolicy keyFramePolicy;
    if (!ParseKeyFramePolicy(env, argv[INDEX_ZERO], keyFramePolicy)) {
        TLOGE(WmsLogTag::WMS_LAYOUT_PC, "Failed to convert parameter to keyFramePolicy");
        return NapiThrowError(env, WmErrorCode::WM_ERROR_ILLEGAL_PARAM);
    }
    wptr<Window> weakToken(windowToken_);
    NapiAsyncTask::ExecuteCallback execute;
    NapiAsyncTask::CompleteCallback complete;
    SetDragKeyFramePolicyTask(execute, complete, weakToken, keyFramePolicy);
    napi_value result = nullptr;
    NapiAsyncTask::Schedule("JsWindow::OnSetDragKeyFramePolicy",
        env, CreateAsyncTaskWithLastParam(env, nullptr, std::move(execute), std::move(complete), &result));
    return result;
}

napi_value JsWindow::OnSetRelativePositionToParentWindowEnabled(napi_env env, napi_callback_info info)
{
    size_t argc = FOUR_PARAMS_SIZE;
    napi_value argv[FOUR_PARAMS_SIZE] = { nullptr };
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc < INDEX_ONE) {
        TLOGE(WmsLogTag::WMS_SUB, "argc is invalid: %{public}zu", argc);
        return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM,
            "[window][setRelativePositionToParentWindowEnabled]msg: Mandatory parameters are left unspecified.");
    }
    bool enabled = false;
    if (!ConvertFromJsValue(env, argv[INDEX_ZERO], enabled)) {
        TLOGE(WmsLogTag::WMS_SUB, "Failed to convert parameter to enable");
        return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM,
            "[window][setRelativePositionToParentWindowEnabled]msg: Failed to convert parameter to enable.");
    }
    WindowAnchor anchor = WindowAnchor::TOP_START;
    if (argc > INDEX_ONE && !ConvertFromJsValue(env, argv[INDEX_ONE], anchor)) {
        TLOGE(WmsLogTag::WMS_SUB, "Failed to convert parameter to anchor");
        return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM,
            "[window][setRelativePositionToParentWindowEnabled]msg: Failed to convert parameter to anchor.");
    }
    int32_t offsetX = 0;
    if (argc > INDEX_TWO && !ConvertFromJsValue(env, argv[INDEX_TWO], offsetX)) {
        TLOGE(WmsLogTag::WMS_SUB, "Failed to convert parameter to offsetX");
        return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM,
            "[window][setRelativePositionToParentWindowEnabled]msg: Failed to convert parameter to offsetX.");
    }
    int32_t offsetY = 0;
    if (argc > INDEX_THREE && !ConvertFromJsValue(env, argv[INDEX_THREE], offsetY)) {
        TLOGE(WmsLogTag::WMS_SUB, "Failed to convert parameter to offsetY");
        return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM,
            "[window][setRelativePositionToParentWindowEnabled]msg: Failed to convert parameter to offsetY.");
    }
    const char* const where = __func__;
    napi_value result = nullptr;
    std::shared_ptr napiAsyncTask = CreateEmptyAsyncTask(env, nullptr, &result);
    WindowAnchorInfo windowAnchorInfo = { enabled, anchor, offsetX, offsetY };
    auto asyncTask = [weakToken = wptr(windowToken_), task = napiAsyncTask, env, windowAnchorInfo, where] {
        auto window = weakToken.promote();
        if (window == nullptr) {
            TLOGNE(WmsLogTag::WMS_SUB, "%{public}s window is nullptr", where);
            task->Reject(env, JsErrUtils::CreateJsError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
                "[window][setRelativePositionToParentWindowEnabled]msg: Window is nullptr."));
            return;
        }
        if (!WindowHelper::IsSubWindow(window->GetType())) {
            TLOGNE(WmsLogTag::WMS_SUB, "%{public}s only sub window is valid", where);
            task->Reject(env, JsErrUtils::CreateJsError(env, WmErrorCode::WM_ERROR_INVALID_CALLING,
                "[window][setRelativePositionToParentWindowEnabled]msg: Only sub window is valid."));
            return;
        }
        WmErrorCode ret = WM_JS_TO_ERROR_CODE_MAP.at(window->SetWindowAnchorInfo(windowAnchorInfo));
        if (ret == WmErrorCode::WM_OK) {
            task->Resolve(env, NapiGetUndefined(env));
        } else {
            TLOGNE(WmsLogTag::WMS_SUB, "%{public}s failed, ret %{public}d", where, ret);
            task->Reject(env, JsErrUtils::CreateJsError(env, ret,
                "[window][setRelativePositionToParentWindowEnabled]msg: Set window anchor info failed."));
        }
    };
    napi_status status = napi_send_event(env, asyncTask, napi_eprio_high, "OnSetRelativePositionToParentWindowEnabled");
    if (status != napi_status::napi_ok) {
        napiAsyncTask->Reject(env, JsErrUtils::CreateJsError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
            "[window][setRelativePositionToParentWindowEnabled]msg: Send event failed."));
    }
    return result;
}

napi_value JsWindow::OnSetFollowParentWindowLayoutEnabled(napi_env env, napi_callback_info info)
{
    size_t argc = FOUR_PARAMS_SIZE;
    napi_value argv[FOUR_PARAMS_SIZE] = { nullptr };
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc != INDEX_ONE) {
        TLOGE(WmsLogTag::WMS_SUB, "argc is invalid: %{public}zu", argc);
        return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM,
            "[window][setFollowParentWindowLayoutEnabled]msg: Argc is invalid");
    }
    bool isFollow = false;
    if (!ConvertFromJsValue(env, argv[INDEX_ZERO], isFollow)) {
        TLOGE(WmsLogTag::WMS_SUB, "Failed to convert parameter to enable");
        return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM,
            "[window][setFollowParentWindowLayoutEnabled]msg: Falied to convert parameter to enable");
    }
    std::shared_ptr<WmErrorCode> errCodePtr = std::make_shared<WmErrorCode>(WmErrorCode::WM_OK);
    const char* const where = __func__;
    napi_value result = nullptr;
    std::shared_ptr<NapiAsyncTask> napiAsyncTask = CreateEmptyAsyncTask(env, nullptr, &result);
    auto asyncTask = [weakToken = wptr<Window>(windowToken_), task = napiAsyncTask, env, isFollow, where] {
        auto window = weakToken.promote();
        if (window == nullptr) {
            TLOGNE(WmsLogTag::WMS_SUB, "%{public}s window is nullptr", where);
            task->Reject(env, CreateJsError(env, static_cast<int32_t>(WmErrorCode::WM_ERROR_STATE_ABNORMALLY),
            "[window][setFollowParentWindowLayoutEnabled]msg: Window is nullptr"));
            return;
        }
        if (!WindowHelper::IsSubWindow(window->GetType()) && !WindowHelper::IsDialogWindow(window->GetType())) {
            TLOGNE(WmsLogTag::WMS_SUB, "%{public}s only sub window and dialog is valid", where);
            task->Reject(env, CreateJsError(env, static_cast<int32_t>(WmErrorCode::WM_ERROR_INVALID_CALLING),
            "[window][setFollowParentWindowLayoutEnabled]msg: Support sub window or dialog only"));
            return;
        }
        WmErrorCode ret = WM_JS_TO_ERROR_CODE_MAP.at(window->SetFollowParentWindowLayoutEnabled(isFollow));
        if (ret == WmErrorCode::WM_OK) {
            task->Resolve(env, NapiGetUndefined(env));
        } else {
            TLOGNE(WmsLogTag::WMS_SUB, "%{public}s failed, ret %{public}d", where, ret);
            task->Reject(env, JsErrUtils::CreateJsError(env, ret,
                "[window][setFollowParentWindowLayoutEnabled]msg: Set follow parent layout failed"));
        }
    };
    napi_status status = napi_send_event(env, asyncTask, napi_eprio_high, "SetFollowParentWindowLayoutEnabled");
    if (status != napi_status::napi_ok) {
        napiAsyncTask->Reject(env, CreateJsError(env,
            static_cast<int32_t>(WmErrorCode::WM_ERROR_STATE_ABNORMALLY),
            "[window][setFollowParentWindowLayoutEnabled]msg: Send event failed"));
    }
    return result;
}

napi_value JsWindow::OnSetReceiveDragEventEnabled(napi_env env, napi_callback_info info)
{
    size_t argc = FOUR_PARAMS_SIZE;
    napi_value argv[FOUR_PARAMS_SIZE] = { nullptr };
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc != INDEX_ONE) {
        TLOGE(WmsLogTag::WMS_EVENT, "argc is invalid: %{public}zu", argc);
        return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM);
    }
    bool enabled = true;
    if (!ConvertFromJsValue(env, argv[INDEX_ZERO], enabled)) {
        TLOGE(WmsLogTag::WMS_EVENT, "Failed to convert parameter to enable");
        return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM);
    }
    std::shared_ptr<WmErrorCode> errCodePtr = std::make_shared<WmErrorCode>(WmErrorCode::WM_OK);
    const char* const where = __func__;
    auto execute = [weakToken = wptr<Window>(windowToken_), errCodePtr, enabled, where] {
        auto window = weakToken.promote();
        if (window == nullptr) {
            TLOGNE(WmsLogTag::WMS_EVENT, "%{public}s window is null", where);
            *errCodePtr = WmErrorCode::WM_ERROR_STATE_ABNORMALLY;
            return;
        }
        *errCodePtr = WM_JS_TO_ERROR_CODE_MAP.at(window->SetReceiveDragEventEnabled(enabled));
        TLOGNI(WmsLogTag::WMS_EVENT, "%{public}s winId: %{public}u, set enable: %{public}u",
            where, window->GetWindowId(), enabled);
    };
    auto complete = [errCodePtr, where](napi_env env, NapiAsyncTask& task, int32_t status) {
        if (*errCodePtr == WmErrorCode::WM_OK) {
            task.Resolve(env, NapiGetUndefined(env));
        } else {
            TLOGNE(WmsLogTag::WMS_EVENT, "%{public}s set failed, result: %{public}d", where, *errCodePtr);
            task.Reject(env, JsErrUtils::CreateJsError(env, *errCodePtr,
                "[window][setReceiveDragEventEnabled]msg: set window receive drag failed"));
        }
    };
    napi_value result = nullptr;
    NapiAsyncTask::Schedule("JsWindow::OnSetReceiveDragEventEnabled",
        env, CreateAsyncTaskWithLastParam(env, nullptr, std::move(execute), std::move(complete), &result));
    return result;
}

napi_value JsWindow::OnIsReceiveDragEventEnabled(napi_env env, napi_callback_info info)
{
    if (windowToken_ == nullptr){
        TLOGE(WmsLogTag::WMS_EVENT, "IsReceiveDragEventEnabled");
        return NapiThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY, 
            "[window][isReceiveDragEventEnabled]msg: IsReceiveDragEventEnabled");
    }

    bool isReceiveDragEventEnabled = windowToken_->IsReceiveDragEventEnabled();
    TLOGD(WmsLogTag::WMS_EVENT, "Id=%{public}u, state=%{public}u", windowToken_->GetWindowId(),
        isReceiveDragEventEnabled);
    return CreateJsValue(env, isReceiveDragEventEnabled);
}

napi_value JsWindow::OnSetSeparationTouchEnabled(napi_env env, napi_callback_info info)
{
    size_t argc = FOUR_PARAMS_SIZE;
    napi_value argv[FOUR_PARAMS_SIZE] = { nullptr };
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc != INDEX_ONE) {
        TLOGE(WmsLogTag::WMS_EVENT, "argc is invalid: %{public}zu", argc);
        return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM);
    }
    bool enabled = true;
    if (!ConvertFromJsValue(env, argv[INDEX_ZERO], enabled)) {
        TLOGE(WmsLogTag::WMS_EVENT, "Failed to convert parameter to enable");
        return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM);
    }
    std::shared_ptr<WmErrorCode> errCodePtr = std::make_shared<WmErrorCode>(WmErrorCode::WM_OK);
    const char* const where = __func__;
    auto execute = [weakToken = wptr<Window>(windowToken_), errCodePtr, enabled, where] {
        auto window = weakToken.promote();
        if (window == nullptr) {
            TLOGNE(WmsLogTag::WMS_EVENT, "%{public}s window is null", where);
            *errCodePtr = WmErrorCode::WM_ERROR_STATE_ABNORMALLY;
            return;
        }
        *errCodePtr = WM_JS_TO_ERROR_CODE_MAP.at(window->SetSeparationTouchEnabled(enabled));
        TLOGNI(WmsLogTag::WMS_EVENT, "%{public}s winId: %{public}u, set enable: %{public}u",
            where, window->GetWindowId(), enabled);
    };
    auto complete = [errCodePtr, where](napi_env env, NapiAsyncTask& task, int32_t status) {
        if (*errCodePtr == WmErrorCode::WM_OK) {
            task.Resolve(env, NapiGetUndefined(env));
        } else {
            TLOGNE(WmsLogTag::WMS_EVENT, "%{public}s set failed, result: %{public}d", where, *errCodePtr);
            task.Reject(env, JsErrUtils::CreateJsError(env, *errCodePtr,
                "[window][setSeparationTouchEnabled]msg: set window receive drag failed"));
        }
    };
    napi_value result = nullptr;
    NapiAsyncTask::Schedule("JsWindow::OnSetSeparationTouchEnabled",
        env, CreateAsyncTaskWithLastParam(env, nullptr, std::move(execute), std::move(complete), &result));
    return result;
}

napi_value JsWindow::OnIsSeparationTouchEnabled(napi_env env, napi_callback_info info)
{
    if (windowToken_ == nullptr){
        TLOGE(WmsLogTag::WMS_EVENT, "IsSeparationTouchEnabled");
        return NapiThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY, 
            "[window][isSeparationTouchEnabled]msg: isSeparationTouchEnabled");
    }

    bool isSeparationTouchEnabled = windowToken_->IsSeparationTouchEnabled();
    TLOGD(WmsLogTag::WMS_EVENT, "Id=%{public}u, state=%{public}u", windowToken_->GetWindowId(),
        isSeparationTouchEnabled);
    return CreateJsValue(env, isSeparationTouchEnabled);
}

napi_value JsWindow::OnSetWindowShadowEnabled(napi_env env, napi_callback_info info)
{
    size_t argc = FOUR_PARAMS_SIZE;
    napi_value argv[FOUR_PARAMS_SIZE] = { nullptr };
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc != INDEX_ONE) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "argc is invalid: %{public}zu", argc);
        return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM,
            "[window][setWindowShadowEnabled]msg: Mandatory parameters are left unspecified");
    }
    bool enabled = true;
    if (!ConvertFromJsValue(env, argv[INDEX_ZERO], enabled)) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "Failed to convert parameter to enable");
        return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM,
            "[window][setWindowShadowEnabled]msg: Convert enable failed");
    }
    std::shared_ptr<WmErrorCode> errCodePtr = std::make_shared<WmErrorCode>(WmErrorCode::WM_OK);
    const char* const where = __func__;
    auto execute = [weakToken = wptr<Window>(windowToken_), errCodePtr, enabled, where] {
        auto window = weakToken.promote();
        if (window == nullptr) {
            TLOGNE(WmsLogTag::WMS_ATTRIBUTE, "%{public}s window is null", where);
            *errCodePtr = WmErrorCode::WM_ERROR_STATE_ABNORMALLY;
            return;
        }
        *errCodePtr = WM_JS_TO_ERROR_CODE_MAP.at(window->SetWindowShadowEnabled(enabled));
        TLOGNI(WmsLogTag::WMS_ATTRIBUTE, "%{public}s winId: %{public}u, set enable: %{public}u",
            where, window->GetWindowId(), enabled);
    };
    auto complete = [errCodePtr, where](napi_env env, NapiAsyncTask& task, int32_t status) {
        if (*errCodePtr == WmErrorCode::WM_OK) {
            task.Resolve(env, NapiGetUndefined(env));
        } else {
            TLOGNE(WmsLogTag::WMS_ATTRIBUTE, "%{public}s set failed, result: %{public}d", where, *errCodePtr);
            task.Reject(env, JsErrUtils::CreateJsError(env, *errCodePtr,
                "[window][setWindowShadowEnabled]"));
        }
    };
    napi_value result = nullptr;
    NapiAsyncTask::Schedule("JsWindow::OnSetWindowShadowEnabled",
        env, CreateAsyncTaskWithLastParam(env, nullptr, std::move(execute), std::move(complete), &result));
    return result;
}

napi_value JsWindow::OnRestoreMainWindow(napi_env env, napi_callback_info info)
{
    size_t argc = ARG_COUNT_ONE;
    napi_value argv[ONE_PARAMS_SIZE] = { nullptr };
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);

    AAFwk::WantParams wantParams;
    if (argc > ARG_COUNT_ZERO) {
        napi_value wantValue = argv[INDEX_ZERO];
        if (wantValue != nullptr && !AppExecFwk::UnwrapWantParams(env, wantValue, wantParams)) {
            TLOGE(WmsLogTag::WMS_LIFE, "Failed to convert parameters to wantParameters");
            return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM,
                "Failed to convert parameters to wantParameters.");
        }
    }

    std::shared_ptr<WmErrorCode> errCodePtr = std::make_shared<WmErrorCode>(WmErrorCode::WM_OK);
    wptr<Window> windowToken(windowToken_);
    std::shared_ptr<AAFwk::WantParams> parameters = std::make_shared<AAFwk::WantParams>(wantParams);
    NapiAsyncTask::ExecuteCallback execute = [windowToken, errCodePtr, parameters] {
        if (errCodePtr == nullptr || parameters == nullptr) {
            return;
        }
        auto window = windowToken.promote();
        if (window == nullptr) {
            *errCodePtr = WmErrorCode::WM_ERROR_STATE_ABNORMALLY;
            return;
        }
        *errCodePtr = WM_JS_TO_ERROR_CODE_MAP.at(window->RestoreMainWindow(parameters));
    };
    NapiAsyncTask::CompleteCallback complete =
        [errCodePtr, parameters](napi_env env, NapiAsyncTask& task, int32_t status) {
            if (errCodePtr == nullptr || parameters == nullptr) {
                task.Reject(env, JsErrUtils::CreateJsError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY));
                return;
            }
            if (*errCodePtr == WmErrorCode::WM_OK) {
                task.Resolve(env, NapiGetUndefined(env));
            } else {
                task.Reject(env, JsErrUtils::CreateJsError(env, *errCodePtr, "Window restore failed."));
            }
        };
    napi_value result = nullptr;
    NapiAsyncTask::Schedule("JsWindow::OnRestoreMainWindow",
        env, CreateAsyncTaskWithLastParam(env, nullptr, std::move(execute), std::move(complete), &result));
    return result;
}

napi_value JsWindow::SetRotationLocked(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::WMS_ROTATION, "[NAPI]");
    JsWindow* me = CheckParamsAndGetThis<JsWindow>(env, info);
    return (me != nullptr) ? me->OnSetRotationLocked(env, info) : nullptr;
}
 
napi_value JsWindow::OnSetRotationLocked(napi_env env, napi_callback_info info)
{
    const std::string errMsgPrefix = "[window][setRotationLocked]msg: ";
    if (windowToken_ == nullptr) {
        TLOGE(WmsLogTag::WMS_ROTATION, "windowToken is nullptr");
        return NapiThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY, errMsgPrefix + "window is nullptr");
    }
    if (!Permission::IsSystemCalling()) {
        TLOGE(WmsLogTag::WMS_ROTATION, "permission denied, require system application!");
        return NapiThrowError(env, WmErrorCode::WM_ERROR_NOT_SYSTEM_APP, errMsgPrefix + "not system application");
    }
    size_t argc = FOUR_PARAMS_SIZE;
    napi_value argv[FOUR_PARAMS_SIZE] = { nullptr };
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc != 1 || argv[0] == nullptr) {
        TLOGE(WmsLogTag::WMS_ROTATION, "Argc is invalid: %{public}zu.", argc);
        return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM, errMsgPrefix + "parameter is invalid");
    }
    bool locked = false;
    if (!ConvertFromJsValue(env, argv[INDEX_ZERO], locked)) {
        TLOGE(WmsLogTag::WMS_ROTATION, "Failed to convert parameter from jsValue");
        return NapiThrowError(env, WmErrorCode::WM_ERROR_INVALID_PARAM,
            errMsgPrefix + "fail to convert parameter to locked");
    }
    napi_value result = nullptr;
    std::shared_ptr<NapiAsyncTask> napiAsyncTask = CreateEmptyAsyncTask(env, nullptr, &result);
    auto asyncTask = [windowToken = wptr<Window>(windowToken_), locked, env, napiAsyncTask, errMsgPrefix] {
        auto window = windowToken.promote();
        if (!window) {
            TLOGNE(WmsLogTag::WMS_ROTATION, "window is nullptr");
            napiAsyncTask->Reject(env, JsErrUtils::CreateJsError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
                errMsgPrefix + "window is nullptr"));
            return;
        }
        auto ret = window->SetRotationLocked(locked);
        auto it = WM_JS_TO_ERROR_CODE_MAP.find(ret);
        WmErrorCode code = (it != WM_JS_TO_ERROR_CODE_MAP.end()) ? it->second : WmErrorCode::WM_ERROR_STATE_ABNORMALLY;
        if (code == WmErrorCode::WM_OK) {
            napiAsyncTask->Resolve(env, NapiGetUndefined(env));
        } else {
            napiAsyncTask->Reject(env, JsErrUtils::CreateJsError(env, code,
                errMsgPrefix + "set rotation locked failed"));
        }
    };
    if (napi_send_event(env, asyncTask, napi_eprio_high, __func__) != napi_status::napi_ok) {
        napiAsyncTask->Reject(env, JsErrUtils::CreateJsError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
            errMsgPrefix + "failed to send event"));
    }
    return result;
}
 
napi_value JsWindow::GetRotationLocked(napi_env env, napi_callback_info info)
{
    TLOGD(WmsLogTag::WMS_ROTATION, "[NAPI]");
    JsWindow* me = CheckParamsAndGetThis<JsWindow>(env, info);
    return (me != nullptr) ? me->OnGetRotationLocked(env, info) : nullptr;
}
 
napi_value JsWindow::OnGetRotationLocked(napi_env env, napi_callback_info info)
{
    const std::string errMsgPrefix = "[window][getRotationLocked]msg: ";
    if (windowToken_ == nullptr) {
        TLOGE(WmsLogTag::WMS_ROTATION, "window is nullptr");
        return NapiThrowError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY, errMsgPrefix + "window is nullptr");
    }
    if (!Permission::IsSystemCalling()) {
        TLOGE(WmsLogTag::WMS_ROTATION, "permission denied, require system application!");
        return NapiThrowError(env, WmErrorCode::WM_ERROR_NOT_SYSTEM_APP, errMsgPrefix + "not system application");
    }
    bool locked = false;
    WmErrorCode code = WM_JS_TO_ERROR_CODE_MAP.at(windowToken_->GetRotationLocked(locked));
    if (code != WmErrorCode::WM_OK) {
        TLOGE(WmsLogTag::WMS_ROTATION, "Get rotation locked failed");
        return NapiThrowError(env, code, errMsgPrefix + "get rotation locked failed");
    }
    TLOGI(WmsLogTag::WMS_ROTATION, "window [%{public}u, %{public}s] locked = %{public}d",
        windowToken_->GetWindowId(), windowToken_->GetWindowName().c_str(), locked);
    return CreateJsValue(env, locked);
}

void BindFunctions(napi_env env, napi_value object, const char* moduleName)
{
    BindNativeFunction(env, object, "startMoving", moduleName, JsWindow::StartMoving);
    BindNativeFunction(env, object, "stopMoving", moduleName, JsWindow::StopMoving);
    BindNativeFunction(env, object, "show", moduleName, JsWindow::Show);
    BindNativeFunction(env, object, "showWindow", moduleName, JsWindow::ShowWindow);
    BindNativeFunction(env, object, "showWithAnimation", moduleName, JsWindow::ShowWithAnimation);
    BindNativeFunction(env, object, "destroy", moduleName, JsWindow::Destroy);
    BindNativeFunction(env, object, "destroyWindow", moduleName, JsWindow::DestroyWindow);
    BindNativeFunction(env, object, "hide", moduleName, JsWindow::Hide);
    BindNativeFunction(env, object, "hideWithAnimation", moduleName, JsWindow::HideWithAnimation);
    BindNativeFunction(env, object, "recover", moduleName, JsWindow::Recover);
    BindNativeFunction(env, object, "restore", moduleName, JsWindow::Restore);
    BindNativeFunction(env, object, "restoreMainWindow", moduleName, JsWindow::RestoreMainWindow);
    BindNativeFunction(env, object, "moveTo", moduleName, JsWindow::MoveTo);
    BindNativeFunction(env, object, "moveWindowTo", moduleName, JsWindow::MoveWindowTo);
    BindNativeFunction(env, object, "moveWindowToAsync", moduleName, JsWindow::MoveWindowToAsync);
    BindNativeFunction(env, object, "moveWindowToGlobal", moduleName, JsWindow::MoveWindowToGlobal);
    BindNativeFunction(env, object, "moveWindowToGlobalDisplay", moduleName, JsWindow::MoveWindowToGlobalDisplay);
    BindNativeFunction(env, object, "getGlobalRect", moduleName, JsWindow::GetGlobalScaledRect);
    BindNativeFunction(env, object, "resetSize", moduleName, JsWindow::Resize);
    BindNativeFunction(env, object, "resize", moduleName, JsWindow::ResizeWindow);
    BindNativeFunction(env, object, "resizeAsync", moduleName, JsWindow::ResizeWindowAsync);
    BindNativeFunction(env, object, "resizeWindowWithAnimation",
        moduleName, JsWindow::ResizeWindowWithAnimation);
    BindNativeFunction(env, object, "clientToGlobalDisplay", moduleName, JsWindow::ClientToGlobalDisplay);
    BindNativeFunction(env, object, "globalDisplayToClient", moduleName, JsWindow::GlobalDisplayToClient);
    BindNativeFunction(env, object, "setWindowType", moduleName, JsWindow::SetWindowType);
    BindNativeFunction(env, object, "setWindowMode", moduleName, JsWindow::SetWindowMode);
    BindNativeFunction(env, object, "getProperties", moduleName, JsWindow::GetProperties);
    BindNativeFunction(env, object, "getWindowProperties", moduleName, JsWindow::GetWindowPropertiesSync);
    BindNativeFunction(env, object, "on", moduleName, JsWindow::RegisterWindowCallback);
    BindNativeFunction(env, object, "off", moduleName, JsWindow::UnregisterWindowCallback);
    BindNativeFunction(env, object, "bindDialogTarget", moduleName, JsWindow::BindDialogTarget);
    BindNativeFunction(env, object, "setDialogBackGestureEnabled", moduleName, JsWindow::SetDialogBackGestureEnabled);
    BindNativeFunction(env, object, "loadContent", moduleName, JsWindow::LoadContent);
    BindNativeFunction(env, object, "loadContentByName", moduleName, JsWindow::LoadContentByName);
    BindNativeFunction(env, object, "getUIContext", moduleName, JsWindow::GetUIContext);
    BindNativeFunction(env, object, "setUIContent", moduleName, JsWindow::SetUIContent);
    BindNativeFunction(env, object, "setFullScreen", moduleName, JsWindow::SetFullScreen);
    BindNativeFunction(env, object, "setLayoutFullScreen", moduleName, JsWindow::SetLayoutFullScreen);
    BindNativeFunction(env, object, "setTitleAndDockHoverShown",
        moduleName, JsWindow::SetTitleAndDockHoverShown);
    BindNativeFunction(env, object, "setWindowLayoutFullScreen", moduleName, JsWindow::SetWindowLayoutFullScreen);
    BindNativeFunction(env, object, "setSystemBarEnable", moduleName, JsWindow::SetSystemBarEnable);
    BindNativeFunction(env, object, "setWindowSystemBarEnable", moduleName, JsWindow::SetWindowSystemBarEnable);
    BindNativeFunction(env, object, "setSystemBarProperties", moduleName, JsWindow::SetSystemBarProperties);
    BindNativeFunction(env, object, "getWindowSystemBarProperties",
        moduleName, JsWindow::GetWindowSystemBarPropertiesSync);
    BindNativeFunction(env, object, "setWindowSystemBarProperties",
        moduleName, JsWindow::SetWindowSystemBarProperties);
    BindNativeFunction(env, object, "setStatusBarColor", moduleName, JsWindow::SetStatusBarColor);
    BindNativeFunction(env, object, "getStatusBarProperty", moduleName, JsWindow::GetStatusBarProperty);
    BindNativeFunction(env, object, "getWindowStateSnapshot", moduleName, JsWindow::GetWindowStateSnapshot);
    BindNativeFunction(env, object, "getAvoidArea", moduleName, JsWindow::GetAvoidArea);
    BindNativeFunction(env, object, "getWindowAvoidArea", moduleName, JsWindow::GetWindowAvoidAreaSync);
    BindNativeFunction(env, object, "getWindowAvoidAreaIgnoringVisibility",
        moduleName, JsWindow::GetWindowAvoidAreaIgnoringVisibilitySync);
    BindNativeFunction(env, object, "isShowing", moduleName, JsWindow::IsShowing);
    BindNativeFunction(env, object, "isWindowShowing", moduleName, JsWindow::IsWindowShowingSync);
    BindNativeFunction(env, object, "isSupportWideGamut", moduleName, JsWindow::IsSupportWideGamut);
    BindNativeFunction(env, object, "isWindowSupportWideGamut", moduleName, JsWindow::IsWindowSupportWideGamut);
    BindNativeFunction(env, object, "setColorSpace", moduleName, JsWindow::SetColorSpace);
    BindNativeFunction(env, object, "setWindowColorSpace", moduleName, JsWindow::SetWindowColorSpace);
    BindNativeFunction(env, object, "getColorSpace", moduleName, JsWindow::GetColorSpace);
    BindNativeFunction(env, object, "getWindowColorSpace", moduleName, JsWindow::GetWindowColorSpaceSync);
    BindNativeFunction(env, object, "setBackgroundColor", moduleName, JsWindow::SetBackgroundColor);
    BindNativeFunction(env, object, "setWindowBackgroundColor", moduleName, JsWindow::SetWindowBackgroundColorSync);
    BindNativeFunction(env, object, "setBrightness", moduleName, JsWindow::SetBrightness);
    BindNativeFunction(env, object, "setWindowBrightness", moduleName, JsWindow::SetWindowBrightness);
    BindNativeFunction(env, object, "setTopmost", moduleName, JsWindow::SetTopmost);
    BindNativeFunction(env, object, "setSubWindowZLevel", moduleName, JsWindow::SetSubWindowZLevel);
    BindNativeFunction(env, object, "getSubWindowZLevel", moduleName, JsWindow::GetSubWindowZLevel);
    BindNativeFunction(env, object, "setWindowTopmost", moduleName, JsWindow::SetWindowTopmost);
    BindNativeFunction(env, object, "setWindowDelayRaiseOnDrag", moduleName, JsWindow::SetWindowDelayRaiseOnDrag);
    BindNativeFunction(env, object, "setDimBehind", moduleName, JsWindow::SetDimBehind);
    BindNativeFunction(env, object, "setFocusable", moduleName, JsWindow::SetFocusable);
    BindNativeFunction(env, object, "setWindowFocusable", moduleName, JsWindow::SetWindowFocusable);
    BindNativeFunction(env, object, "setKeepScreenOn", moduleName, JsWindow::SetKeepScreenOn);
    BindNativeFunction(env, object, "setWindowKeepScreenOn", moduleName, JsWindow::SetWindowKeepScreenOn);
    BindNativeFunction(env, object, "setWakeUpScreen", moduleName, JsWindow::SetWakeUpScreen);
    BindNativeFunction(env, object, "setOutsideTouchable", moduleName, JsWindow::SetOutsideTouchable);
    BindNativeFunction(env, object, "setPrivacyMode", moduleName, JsWindow::SetPrivacyMode);
    BindNativeFunction(env, object, "setWindowPrivacyMode", moduleName, JsWindow::SetWindowPrivacyMode);
    BindNativeFunction(env, object, "setTouchable", moduleName, JsWindow::SetTouchable);
    BindNativeFunction(env, object, "setTouchableAreas", moduleName, JsWindow::SetTouchableAreas);
    BindNativeFunction(env, object, "setWindowTouchable", moduleName, JsWindow::SetWindowTouchable);
    BindNativeFunction(env, object, "setTransparent", moduleName, JsWindow::SetTransparent);
    BindNativeFunction(env, object, "changeCallingWindowId", moduleName, JsWindow::ChangeCallingWindowId);
    BindNativeFunction(env, object, "setSnapshotSkip", moduleName, JsWindow::SetSnapshotSkip);
    BindNativeFunction(env, object, "raiseToAppTop", moduleName, JsWindow::RaiseToAppTop);
    BindNativeFunction(env, object, "disableWindowDecor", moduleName, JsWindow::DisableWindowDecor);
    BindNativeFunction(env, object, "dump", moduleName, JsWindow::Dump);
    BindNativeFunction(env, object, "setForbidSplitMove", moduleName, JsWindow::SetForbidSplitMove);
    BindNativeFunction(env, object, "setPreferredOrientation", moduleName, JsWindow::SetPreferredOrientation);
    BindNativeFunction(env, object, "getPreferredOrientation", moduleName, JsWindow::GetPreferredOrientation);
    BindNativeFunction(env, object, "convertOrientationAndRotation",
        moduleName, JsWindow::ConvertOrientationAndRotation);
    BindNativeFunction(env, object, "opacity", moduleName, JsWindow::Opacity);
    BindNativeFunction(env, object, "scale", moduleName, JsWindow::Scale);
    BindNativeFunction(env, object, "rotate", moduleName, JsWindow::Rotate);
    BindNativeFunction(env, object, "translate", moduleName, JsWindow::Translate);
    BindNativeFunction(env, object, "getTransitionController", moduleName, JsWindow::GetTransitionController);
    BindNativeFunction(env, object, "snapshot", moduleName, JsWindow::Snapshot);
    BindNativeFunction(env, object, "snapshotSync", moduleName, JsWindow::SnapshotSync);
    BindNativeFunction(env, object, "snapshotIgnorePrivacy", moduleName, JsWindow::SnapshotIgnorePrivacy);
    BindNativeFunction(env, object, "setCornerRadius", moduleName, JsWindow::SetCornerRadius);
    BindNativeFunction(env, object, "setWindowCornerRadius", moduleName, JsWindow::SetWindowCornerRadius);
    BindNativeFunction(env, object, "getWindowCornerRadius", moduleName, JsWindow::GetWindowCornerRadius);
    BindNativeFunction(env, object, "setShadow", moduleName, JsWindow::SetShadow);
    BindNativeFunction(env, object, "setWindowShadowRadius", moduleName, JsWindow::SetWindowShadowRadius);
    BindNativeFunction(env, object, "setBlur", moduleName, JsWindow::SetBlur);
    BindNativeFunction(env, object, "setBackdropBlur", moduleName, JsWindow::SetBackdropBlur);
    BindNativeFunction(env, object, "setBackdropBlurStyle", moduleName, JsWindow::SetBackdropBlurStyle);
    BindNativeFunction(env, object, "setAspectRatio", moduleName, JsWindow::SetAspectRatio);
    BindNativeFunction(env, object, "setContentAspectRatio", moduleName, JsWindow::SetContentAspectRatio);
    BindNativeFunction(env, object, "resetAspectRatio", moduleName, JsWindow::ResetAspectRatio);
    BindNativeFunction(env, object, "setWaterMarkFlag", moduleName, JsWindow::SetWaterMarkFlag);
    BindNativeFunction(env, object, "setHandwritingFlag", moduleName, JsWindow::SetHandwritingFlag);
    BindNativeFunction(env, object, "minimize", moduleName, JsWindow::Minimize);
    BindNativeFunction(env, object, "maximize", moduleName, JsWindow::Maximize);
    BindNativeFunction(env, object, "setResizeByDragEnabled", moduleName, JsWindow::SetResizeByDragEnabled);
    BindNativeFunction(env, object, "setRaiseByClickEnabled", moduleName, JsWindow::SetRaiseByClickEnabled);
    BindNativeFunction(env, object, "setMainWindowRaiseByClickEnabled", moduleName,
        JsWindow::SetMainWindowRaiseByClickEnabled);
    BindNativeFunction(env, object, "raiseAboveTarget", moduleName, JsWindow::RaiseAboveTarget);
    BindNativeFunction(env, object, "raiseMainWindowAboveTarget", moduleName, JsWindow::RaiseMainWindowAboveTarget);
    BindNativeFunction(env, object, "hideNonSystemFloatingWindows", moduleName,
        JsWindow::HideNonSystemFloatingWindows);
    BindNativeFunction(env, object, "keepKeyboardOnFocus", moduleName, JsWindow::KeepKeyboardOnFocus);
    BindNativeFunction(env, object, "setWindowLimits", moduleName, JsWindow::SetWindowLimits);
    BindNativeFunction(env, object, "getWindowLimits", moduleName, JsWindow::GetWindowLimits);
    BindNativeFunction(env, object, "getWindowLimitsVP", moduleName, JsWindow::GetWindowLimitsVP);
    BindNativeFunction(env, object, "setSpecificSystemBarEnabled", moduleName, JsWindow::SetSpecificSystemBarEnabled);
    BindNativeFunction(env, object, "setSingleFrameComposerEnabled", moduleName,
        JsWindow::SetSingleFrameComposerEnabled);
    BindNativeFunction(env, object, "enableLandscapeMultiWindow", moduleName, JsWindow::EnableLandscapeMultiWindow);
    BindNativeFunction(env, object, "disableLandscapeMultiWindow", moduleName, JsWindow::DisableLandscapeMultiWindow);
    BindNativeFunction(env, object, "setWindowTitle", moduleName, JsWindow::SetWindowTitle);
    BindNativeFunction(env, object, "setWindowDecorVisible", moduleName, JsWindow::SetWindowDecorVisible);
    BindNativeFunction(env, object, "getWindowDecorVisible", moduleName, JsWindow::GetWindowDecorVisible);
    BindNativeFunction(env, object, "setWindowTitleMoveEnabled", moduleName, JsWindow::SetWindowTitleMoveEnabled);
    BindNativeFunction(env, object, "setSubWindowModal", moduleName, JsWindow::SetSubWindowModal);
    BindNativeFunction(env, object, "setFollowParentMultiScreenPolicy",
        moduleName, JsWindow::SetFollowParentMultiScreenPolicy);
    BindNativeFunction(env, object, "setWindowTransitionAnimation",
        moduleName, JsWindow::SetWindowTransitionAnimation);
    BindNativeFunction(env, object, "getWindowTransitionAnimation",
        moduleName, JsWindow::GetWindowTransitionAnimation);
    BindNativeFunction(env, object, "enableDrag", moduleName, JsWindow::EnableDrag);
    BindNativeFunction(env, object, "setWindowDecorHeight", moduleName, JsWindow::SetWindowDecorHeight);
    BindNativeFunction(env, object, "getWindowDecorHeight", moduleName, JsWindow::GetWindowDecorHeight);
    BindNativeFunction(env, object, "setDecorButtonStyle", moduleName, JsWindow::SetDecorButtonStyle);
    BindNativeFunction(env, object, "getDecorButtonStyle", moduleName, JsWindow::GetDecorButtonStyle);
    BindNativeFunction(env, object, "getTitleButtonRect", moduleName, JsWindow::GetTitleButtonRect);
    BindNativeFunction(env, object, "setTitleButtonVisible", moduleName, JsWindow::SetTitleButtonVisible);
    BindNativeFunction(env, object, "setWindowTitleButtonVisible", moduleName, JsWindow::SetWindowTitleButtonVisible);
    BindNativeFunction(env, object, "setWindowContainerColor", moduleName, JsWindow::SetWindowContainerColor);
    BindNativeFunction(env, object, "setWindowContainerModalColor", moduleName, JsWindow::SetWindowContainerModalColor);
    BindNativeFunction(env, object, "setWindowMask", moduleName, JsWindow::SetWindowMask);
    BindNativeFunction(env, object, "setWindowGrayScale", moduleName, JsWindow::SetWindowGrayScale);
    BindNativeFunction(env, object, "setImmersiveModeEnabledState", moduleName, JsWindow::SetImmersiveModeEnabledState);
    BindNativeFunction(env, object, "getImmersiveModeEnabledState", moduleName, JsWindow::GetImmersiveModeEnabledState);
    BindNativeFunction(env, object, "getWindowStatus", moduleName, JsWindow::GetWindowStatus);
    BindNativeFunction(env, object, "isFocused", moduleName, JsWindow::IsFocused);
    BindNativeFunction(env, object, "requestFocus", moduleName, JsWindow::RequestFocus);
    BindNativeFunction(env, object, "createSubWindowWithOptions", moduleName, JsWindow::CreateSubWindowWithOptions);
    BindNativeFunction(env, object, "setParentWindow", moduleName, JsWindow::SetParentWindow);
    BindNativeFunction(env, object, "getParentWindow", moduleName, JsWindow::GetParentWindow);
    BindNativeFunction(env, object, "setGestureBackEnabled", moduleName, JsWindow::SetGestureBackEnabled);
    BindNativeFunction(env, object, "isGestureBackEnabled", moduleName, JsWindow::GetGestureBackEnabled);
    BindNativeFunction(env, object, "getWindowDensityInfo", moduleName, JsWindow::GetWindowDensityInfo);
    BindNativeFunction(env, object, "setDefaultDensityEnabled", moduleName, JsWindow::SetDefaultDensityEnabled);
    BindNativeFunction(env, object, "isMainWindowFullScreenAcrossDisplays", moduleName,
        JsWindow::IsMainWindowFullScreenAcrossDisplays);
    BindNativeFunction(env, object, "setSystemAvoidAreaEnabled", moduleName, JsWindow::SetSystemAvoidAreaEnabled);
    BindNativeFunction(env, object, "isSystemAvoidAreaEnabled", moduleName, JsWindow::IsSystemAvoidAreaEnabled);
    BindNativeFunction(env, object, "setExclusivelyHighlighted", moduleName, JsWindow::SetExclusivelyHighlighted);
    BindNativeFunction(env, object, "isWindowHighlighted", moduleName, JsWindow::IsWindowHighlighted);
    BindNativeFunction(env, object, "setDragKeyFramePolicy", moduleName, JsWindow::SetDragKeyFramePolicy);
    BindNativeFunction(env, object, "setRelativePositionToParentWindowEnabled", moduleName,
        JsWindow::SetRelativePositionToParentWindowEnabled);
    BindNativeFunction(env, object, "setFollowParentWindowLayoutEnabled", moduleName,
        JsWindow::SetFollowParentWindowLayoutEnabled);
    BindNativeFunction(env, object, "setReceiveDragEventEnabled", moduleName, JsWindow::SetReceiveDragEventEnabled);
    BindNativeFunction(env, object, "isReceiveDragEventEnabled", moduleName, JsWindow::IsReceiveDragEventEnabled);
    BindNativeFunction(env, object, "setSeparationTouchEnabled", moduleName, JsWindow::SetSeparationTouchEnabled);
    BindNativeFunction(env, object, "isSeparationTouchEnabled", moduleName, JsWindow::IsSeparationTouchEnabled);
    BindNativeFunction(env, object, "setWindowShadowEnabled", moduleName, JsWindow::SetWindowShadowEnabled);
    BindNativeFunction(env, object, "isImmersiveLayout", moduleName, JsWindow::IsImmersiveLayout);
    BindNativeFunction(env, object, "isInFreeWindowMode", moduleName, JsWindow::IsInFreeWindowMode);
    BindNativeFunction(env, object, "setRotationLocked", moduleName, JsWindow::SetRotationLocked);
    BindNativeFunction(env, object, "getRotationLocked", moduleName, JsWindow::GetRotationLocked);
}
}  // namespace Rosen
}  // namespace OHOS
