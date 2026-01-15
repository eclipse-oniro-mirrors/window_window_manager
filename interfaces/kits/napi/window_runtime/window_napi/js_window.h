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

#ifndef OHOS_JS_WINDOW_H
#define OHOS_JS_WINDOW_H

#include "js_runtime_utils.h"

#ifndef WINDOW_PREVIEW
#include "js_window_register_manager.h"
#else
#include "mock/js_window_register_manager.h"
#endif

#include "js_window_utils.h"
#include "native_engine/native_engine.h"
#include "native_engine/native_value.h"
#include "window.h"

namespace OHOS {
namespace Rosen {
napi_value CreateJsWindowObject(napi_env env, sptr<Window>& window);
napi_value CreateJsWindowArrayObject(napi_env env, const std::vector<sptr<Window>>& windows);
std::shared_ptr<NativeReference> FindJsWindowObject(const std::string& windowName);
void BindFunctions(napi_env env, napi_value object, const char* moduleName);
napi_value NapiGetUndefined(napi_env env);
napi_valuetype GetType(napi_env env, napi_value value);
bool NapiIsCallable(napi_env env, napi_value value);
napi_value NapiThrowError(napi_env env, WmErrorCode errCode);
napi_value NapiThrowError(napi_env env, WmErrorCode errCode, const std::string& msg);
class JsWindow final {
public:
    explicit JsWindow(const sptr<Window>& window);
    ~JsWindow();
    sptr<Window> GetWindow() { return windowToken_; }
    static void Finalizer(napi_env env, void* data, void* hint);
    static napi_value Show(napi_env env, napi_callback_info info);
    static napi_value ShowWindow(napi_env env, napi_callback_info info);
    static napi_value ShowWithAnimation(napi_env env, napi_callback_info info);
    static napi_value Destroy(napi_env env, napi_callback_info info);
    static napi_value DestroyWindow(napi_env env, napi_callback_info info);
    static napi_value Hide(napi_env env, napi_callback_info info);
    static napi_value HideWithAnimation(napi_env env, napi_callback_info info);

    /*
     * Window Layout
     */
    static napi_value MoveTo(napi_env env, napi_callback_info info);
    static napi_value MoveWindowTo(napi_env env, napi_callback_info info);
    static napi_value MoveWindowToAsync(napi_env env, napi_callback_info info);
    static napi_value MoveWindowToGlobal(napi_env env, napi_callback_info info);
    static napi_value MoveWindowToGlobalDisplay(napi_env env, napi_callback_info info);
    static napi_value GetGlobalScaledRect(napi_env env, napi_callback_info info);
    static napi_value Resize(napi_env env, napi_callback_info info);
    static napi_value ResizeWindow(napi_env env, napi_callback_info info);
    static napi_value ResizeWindowAsync(napi_env env, napi_callback_info info);
    static napi_value ResizeWindowWithAnimation(napi_env env, napi_callback_info info);
    static napi_value ClientToGlobalDisplay(napi_env env, napi_callback_info info);
    static napi_value GlobalDisplayToClient(napi_env env, napi_callback_info info);

    static napi_value SetWindowType(napi_env env, napi_callback_info info);
    static napi_value SetWindowMode(napi_env env, napi_callback_info info);
    static napi_value GetProperties(napi_env env, napi_callback_info info);
    static napi_value GetWindowPropertiesSync(napi_env env, napi_callback_info info);
    static napi_value RegisterWindowCallback(napi_env env, napi_callback_info info);
    static napi_value UnregisterWindowCallback(napi_env env, napi_callback_info info);
    static napi_value BindDialogTarget(napi_env env, napi_callback_info info);
    static napi_value SetDialogBackGestureEnabled(napi_env env, napi_callback_info info);
    static napi_value LoadContent(napi_env env, napi_callback_info info);
    static napi_value LoadContentByName(napi_env env, napi_callback_info info);
    static napi_value GetUIContext(napi_env env, napi_callback_info info);
    static napi_value SetUIContent(napi_env env, napi_callback_info info);
    static napi_value IsShowing(napi_env env, napi_callback_info info);
    static napi_value IsWindowShowingSync(napi_env env, napi_callback_info info);
    static napi_value SetBackgroundColor(napi_env env, napi_callback_info info);
    static napi_value SetWindowBackgroundColorSync(napi_env env, napi_callback_info info);
    static napi_value SetBrightness(napi_env env, napi_callback_info info);
    static napi_value SetWindowBrightness(napi_env env, napi_callback_info info);
    static napi_value SetDimBehind(napi_env env, napi_callback_info info);
    static napi_value SetFocusable(napi_env env, napi_callback_info info);
    static napi_value SetWindowFocusable(napi_env env, napi_callback_info info);
    static napi_value SetTopmost(napi_env env, napi_callback_info info);
    static napi_value SetWindowTopmost(napi_env env, napi_callback_info info);
    static napi_value SetSubWindowZLevel(napi_env env, napi_callback_info info);
    static napi_value GetSubWindowZLevel(napi_env env, napi_callback_info info);
    static napi_value SetWindowDelayRaiseOnDrag(napi_env env, napi_callback_info info);
    static napi_value SetKeepScreenOn(napi_env env, napi_callback_info info);
    static napi_value SetWindowKeepScreenOn(napi_env env, napi_callback_info info);
    static napi_value SetWakeUpScreen(napi_env env, napi_callback_info info);
    static napi_value SetOutsideTouchable(napi_env env, napi_callback_info info);
    static napi_value SetPrivacyMode(napi_env env, napi_callback_info info);
    static napi_value SetWindowPrivacyMode(napi_env env, napi_callback_info info);
    static napi_value SetTouchable(napi_env env, napi_callback_info info);
    static napi_value SetTouchableAreas(napi_env env, napi_callback_info info);
    static napi_value SetResizeByDragEnabled(napi_env env, napi_callback_info info);
    static napi_value SetRaiseByClickEnabled(napi_env env, napi_callback_info info);
    static napi_value SetMainWindowRaiseByClickEnabled(napi_env env, napi_callback_info info);
    static napi_value HideNonSystemFloatingWindows(napi_env env, napi_callback_info info);
    static napi_value SetWindowTouchable(napi_env env, napi_callback_info info);
    static napi_value SetTransparent(napi_env env, napi_callback_info info);
    static napi_value ChangeCallingWindowId(napi_env env, napi_callback_info info);
    static napi_value SetPreferredOrientation(napi_env env, napi_callback_info info);
    static napi_value GetPreferredOrientation(napi_env env, napi_callback_info info);
    static napi_value ConvertOrientationAndRotation(napi_env env, napi_callback_info info);
    static napi_value SetSnapshotSkip(napi_env env, napi_callback_info info);
    static napi_value RaiseToAppTop(napi_env env, napi_callback_info info);
    static napi_value SetAspectRatio(napi_env env, napi_callback_info info);
    static napi_value SetContentAspectRatio(napi_env env, napi_callback_info info);
    static napi_value ResetAspectRatio(napi_env env, napi_callback_info info);
    static napi_value Minimize(napi_env env, napi_callback_info info);
    static napi_value RaiseAboveTarget(napi_env env, napi_callback_info info);
    static napi_value RaiseMainWindowAboveTarget(napi_env env, napi_callback_info info);
    static napi_value KeepKeyboardOnFocus(napi_env env, napi_callback_info info);
    static napi_value GetWindowLimits(napi_env env, napi_callback_info info);
    static napi_value GetWindowLimitsVP(napi_env env, napi_callback_info info);
    static napi_value SetWindowLimits(napi_env env, napi_callback_info info);
    static napi_value SetSingleFrameComposerEnabled(napi_env env, napi_callback_info info);
    static napi_value EnableLandscapeMultiWindow(napi_env env, napi_callback_info info);
    static napi_value DisableLandscapeMultiWindow(napi_env env, napi_callback_info info);
    static napi_value IsFocused(napi_env env, napi_callback_info info);
    static napi_value RequestFocus(napi_env env, napi_callback_info info);
    static napi_value StartMoving(napi_env env, napi_callback_info info);
    static napi_value StopMoving(napi_env env, napi_callback_info info);
    static napi_value GetWindowDensityInfo(napi_env env, napi_callback_info info);
    static napi_value SetDefaultDensityEnabled(napi_env env, napi_callback_info info);
    static napi_value IsMainWindowFullScreenAcrossDisplays(napi_env env, napi_callback_info info);
    static napi_value EnableDrag(napi_env env, napi_callback_info info);
    static napi_value SetExclusivelyHighlighted(napi_env env, napi_callback_info info);
    static napi_value IsWindowHighlighted(napi_env env, napi_callback_info info);
    static napi_value RestoreMainWindow(napi_env env, napi_callback_info info);

    // colorspace, gamut
    static napi_value IsSupportWideGamut(napi_env env, napi_callback_info info);
    static napi_value IsWindowSupportWideGamut(napi_env env, napi_callback_info info);
    static napi_value SetColorSpace(napi_env env, napi_callback_info info);
    static napi_value SetWindowColorSpace(napi_env env, napi_callback_info info);
    static napi_value GetColorSpace(napi_env env, napi_callback_info info);
    static napi_value GetWindowColorSpaceSync(napi_env env, napi_callback_info info);
    static napi_value Dump(napi_env env, napi_callback_info info);
    static napi_value SetForbidSplitMove(napi_env env, napi_callback_info info);
    static napi_value Snapshot(napi_env env, napi_callback_info info);
    static napi_value SnapshotSync(napi_env env, napi_callback_info info);
    static napi_value SnapshotIgnorePrivacy(napi_env env, napi_callback_info info);

    // animation config
    static napi_value Opacity(napi_env env, napi_callback_info info);
    static napi_value Scale(napi_env env, napi_callback_info info);
    static napi_value Rotate(napi_env env, napi_callback_info info);
    static napi_value Translate(napi_env env, napi_callback_info info);
    static napi_value GetTransitionController(napi_env env, napi_callback_info info);

    /*
     * Window Property
     */
    static napi_value SetCornerRadius(napi_env env, napi_callback_info info);
    static napi_value SetWindowCornerRadius(napi_env env, napi_callback_info info);
    static napi_value GetWindowCornerRadius(napi_env env, napi_callback_info info);
    static napi_value SetShadow(napi_env env, napi_callback_info info);
    static napi_value SetWindowShadowRadius(napi_env env, napi_callback_info info);
    static napi_value SetBlur(napi_env env, napi_callback_info info);
    static napi_value SetBackdropBlur(napi_env env, napi_callback_info info);
    static napi_value SetBackdropBlurStyle(napi_env env, napi_callback_info info);
    static napi_value SetWaterMarkFlag(napi_env env, napi_callback_info info);
    static napi_value SetHandwritingFlag(napi_env env, napi_callback_info info);
    static napi_value SetWindowGrayScale(napi_env env, napi_callback_info info);
    static napi_value SetRotationLocked(napi_env env, napi_callback_info info);
    static napi_value GetRotationLocked(napi_env env, napi_callback_info info);

    /*
     * Sub Window
     */
    static napi_value CreateSubWindowWithOptions(napi_env env, napi_callback_info info);
    static napi_value SetSubWindowModal(napi_env env, napi_callback_info info);
    static napi_value SetParentWindow(napi_env env, napi_callback_info info);
    static napi_value GetParentWindow(napi_env env, napi_callback_info info);

    /*
     * Gesture Back
     */
    static napi_value SetGestureBackEnabled(napi_env env, napi_callback_info info);
    static napi_value GetGestureBackEnabled(napi_env env, napi_callback_info info);

    /*
     * PC Window
     */
    static napi_value GetWindowStatus(napi_env env, napi_callback_info info);
    static napi_value SetWindowMask(napi_env env, napi_callback_info info);
    static napi_value SetFollowParentMultiScreenPolicy(napi_env env, napi_callback_info info);
    static napi_value IsInFreeWindowMode(napi_env env, napi_callback_info info);

    /*
     * Window Transition Animation For PC
     */
    static napi_value SetWindowTransitionAnimation(napi_env env, napi_callback_info info);
    static napi_value GetWindowTransitionAnimation(napi_env env, napi_callback_info info);

    /*
     * Window Decor
     */
    static napi_value DisableWindowDecor(napi_env env, napi_callback_info info);
    static napi_value SetWindowDecorVisible(napi_env env, napi_callback_info info);
    static napi_value GetWindowDecorVisible(napi_env env, napi_callback_info info);
    static napi_value SetWindowTitleMoveEnabled(napi_env env, napi_callback_info info);
    static napi_value SetWindowDecorHeight(napi_env env, napi_callback_info info);
    static napi_value GetWindowDecorHeight(napi_env env, napi_callback_info info);
    static napi_value GetTitleButtonRect(napi_env env, napi_callback_info info);
    static napi_value SetTitleButtonVisible(napi_env env, napi_callback_info info);
    static napi_value SetWindowTitleButtonVisible(napi_env env, napi_callback_info info);
    static napi_value SetWindowContainerColor(napi_env env, napi_callback_info info);
    static napi_value SetWindowContainerModalColor(napi_env env, napi_callback_info info);
    static napi_value SetDecorButtonStyle(napi_env env, napi_callback_info info);
    static napi_value GetDecorButtonStyle(napi_env env, napi_callback_info info);
    static napi_value SetWindowTitle(napi_env env, napi_callback_info info);

    /*
     * PC Window Layout
     */
    static napi_value Recover(napi_env env, napi_callback_info info);
    static napi_value Maximize(napi_env env, napi_callback_info info);
    static napi_value SetTitleAndDockHoverShown(napi_env env, napi_callback_info info);
    static napi_value Restore(napi_env env, napi_callback_info info);
    static napi_value SetDragKeyFramePolicy(napi_env env, napi_callback_info info);

    /*
     * Window Immersive
     */
    static napi_value SetFullScreen(napi_env env, napi_callback_info info);
    static napi_value SetLayoutFullScreen(napi_env env, napi_callback_info info);
    static napi_value SetWindowLayoutFullScreen(napi_env env, napi_callback_info info);
    static napi_value SetSystemBarEnable(napi_env env, napi_callback_info info);
    static napi_value SetWindowSystemBarEnable(napi_env env, napi_callback_info info);
    static napi_value SetSystemBarProperties(napi_env env, napi_callback_info info);
    static napi_value GetWindowSystemBarPropertiesSync(napi_env env, napi_callback_info info);
    static napi_value SetWindowSystemBarProperties(napi_env env, napi_callback_info info);
    static napi_value SetStatusBarColor(napi_env env, napi_callback_info info);
    static napi_value GetStatusBarProperty(napi_env env, napi_callback_info info);
    static napi_value GetWindowStateSnapshot(napi_env env, napi_callback_info info);
    static napi_value GetAvoidArea(napi_env env, napi_callback_info info);
    static napi_value GetWindowAvoidAreaSync(napi_env env, napi_callback_info info);
    static napi_value GetWindowAvoidAreaIgnoringVisibilitySync(napi_env env, napi_callback_info info);
    static napi_value SetSpecificSystemBarEnabled(napi_env env, napi_callback_info info);
    static napi_value SetSystemAvoidAreaEnabled(napi_env env, napi_callback_info info);
    static napi_value IsSystemAvoidAreaEnabled(napi_env env, napi_callback_info info);
    static napi_value SetImmersiveModeEnabledState(napi_env env, napi_callback_info info);
    static napi_value GetImmersiveModeEnabledState(napi_env env, napi_callback_info info);
    static napi_value IsImmersiveLayout(napi_env env, napi_callback_info info);
    static napi_value SetRelativePositionToParentWindowEnabled(napi_env env, napi_callback_info info);
    static napi_value SetFollowParentWindowLayoutEnabled(napi_env env, napi_callback_info info);
    static napi_value SetWindowShadowEnabled(napi_env env, napi_callback_info info);

    /*
     * Window Event
     */
    static napi_value SetReceiveDragEventEnabled(napi_env env, napi_callback_info info);
    static napi_value IsReceiveDragEventEnabled(napi_env env, napi_callback_info info);
    static napi_value SetSeparationTouchEnabled(napi_env env, napi_callback_info info);
    static napi_value IsSeparationTouchEnabled(napi_env env, napi_callback_info info);

private:
    const std::string& GetWindowName() const;
    static bool ParseScaleOption(napi_env env, napi_value jsObject, Transform& trans);
    static bool ParseRotateOption(napi_env env, napi_value jsObject, Transform& trans);
    static bool ParseTranslateOption(napi_env env, napi_value jsObject, Transform& trans);
    static bool ParseWindowLimits(napi_env env, napi_value jsObject, WindowLimits& windowLimits);
    void ParseShadowOptionalParameters(WmErrorCode& ret, std::shared_ptr<ShadowsInfo>& shadowsInfo,
        napi_env env, const napi_value* argv, size_t argc);
    bool CheckWindowMaskParams(napi_env env, napi_value jsObject);
    napi_value LoadContentScheduleOld(napi_env env, napi_callback_info info, bool isLoadedByName);
    napi_value LoadContentScheduleNew(napi_env env, napi_callback_info info, bool isLoadedByName);
    napi_value HideWindowFunction(napi_env env, napi_callback_info info, WmErrorCode errCode);
    napi_value OnShow(napi_env env, napi_callback_info info);
    napi_value OnShowWindow(napi_env env, napi_callback_info info);
    napi_value OnShowWithAnimation(napi_env env, napi_callback_info info);
    napi_value OnDestroy(napi_env env, napi_callback_info info);
    napi_value OnDestroyWindow(napi_env env, napi_callback_info info);
    napi_value OnHide(napi_env env, napi_callback_info info);
    napi_value OnHideWithAnimation(napi_env env, napi_callback_info info);

    /*
     * Window Layout
     */
    napi_value OnMoveTo(napi_env env, napi_callback_info info);
    napi_value OnMoveWindowTo(napi_env env, napi_callback_info info);
    napi_value OnMoveWindowToAsync(napi_env env, napi_callback_info info);
    napi_value OnMoveWindowToGlobal(napi_env env, napi_callback_info info);
    napi_value OnMoveWindowToGlobalDisplay(napi_env env, napi_callback_info info);
    napi_value OnGetGlobalScaledRect(napi_env env, napi_callback_info info);
    napi_value OnResize(napi_env env, napi_callback_info info);
    napi_value OnResizeWindow(napi_env env, napi_callback_info info);
    napi_value OnResizeWindowAsync(napi_env env, napi_callback_info info);
    napi_value OnResizeWindowWithAnimation(napi_env env, napi_callback_info info);
    template <typename PositionTransformFunc>
    napi_value HandlePositionTransform(
        napi_env env, napi_callback_info info, PositionTransformFunc transformFunc, const char* caller);
    napi_value OnClientToGlobalDisplay(napi_env env, napi_callback_info info);
    napi_value OnGlobalDisplayToClient(napi_env env, napi_callback_info info);

    napi_value OnSetWindowType(napi_env env, napi_callback_info info);
    napi_value OnSetWindowMode(napi_env env, napi_callback_info info);
    napi_value OnGetProperties(napi_env env, napi_callback_info info);
    napi_value OnGetWindowPropertiesSync(napi_env env, napi_callback_info info);
    napi_value OnRegisterWindowCallback(napi_env env, napi_callback_info info);
    napi_value OnUnregisterWindowCallback(napi_env env, napi_callback_info info);
    napi_value OnBindDialogTarget(napi_env env, napi_callback_info info);
    napi_value OnSetDialogBackGestureEnabled(napi_env env, napi_callback_info info);
    napi_value OnLoadContent(napi_env env, napi_callback_info info, bool isLoadedByName);
    napi_value OnGetUIContext(napi_env env, napi_callback_info info);
    napi_value OnSetUIContent(napi_env env, napi_callback_info info);
    napi_value OnIsShowing(napi_env env, napi_callback_info info);
    napi_value OnIsWindowShowingSync(napi_env env, napi_callback_info info);
    napi_value OnSetPreferredOrientation(napi_env env, napi_callback_info info);
    napi_value OnGetPreferredOrientation(napi_env env, napi_callback_info info);
    napi_value OnConvertOrientationAndRotation(napi_env env, napi_callback_info info);
    napi_value OnRaiseToAppTop(napi_env env, napi_callback_info info);
    napi_value OnSetAspectRatio(napi_env env, napi_callback_info info);
    napi_value OnSetContentAspectRatio(napi_env env, napi_callback_info info);
    napi_value OnResetAspectRatio(napi_env env, napi_callback_info info);
    napi_value OnMinimize(napi_env env, napi_callback_info info);
    WmErrorCode CheckRaiseMainWindowParams(napi_env env, size_t argc, napi_value argv[],
                                           int32_t sourceId, int32_t& targetId);
    napi_value OnRaiseAboveTarget(napi_env env, napi_callback_info info);
    napi_value OnRaiseMainWindowAboveTarget(napi_env env, napi_callback_info info);
    napi_value OnKeepKeyboardOnFocus(napi_env env, napi_callback_info info);
    napi_value OnSetWindowLimits(napi_env env, napi_callback_info info);
    napi_value OnGetWindowLimits(napi_env env, napi_callback_info info);
    napi_value OnGetWindowLimitsVP(napi_env env, napi_callback_info info);

    napi_value OnIsFocused(napi_env env, napi_callback_info info);
    napi_value OnRequestFocus(napi_env env, napi_callback_info info);
    napi_value OnGetWindowDensityInfo(napi_env env, napi_callback_info info);
    napi_value OnSetDefaultDensityEnabled(napi_env env, napi_callback_info info);
    napi_value OnIsMainWindowFullScreenAcrossDisplays(napi_env env, napi_callback_info info);
    napi_value OnSetExclusivelyHighlighted(napi_env env, napi_callback_info info);
    napi_value OnIsWindowHighlighted(napi_env env, napi_callback_info info);
    napi_value OnRestoreMainWindow(napi_env env, napi_callback_info info);

    // colorspace, gamut
    napi_value OnIsSupportWideGamut(napi_env env, napi_callback_info info);
    napi_value OnIsWindowSupportWideGamut(napi_env env, napi_callback_info info);
    napi_value OnSetColorSpace(napi_env env, napi_callback_info info);
    napi_value OnSetWindowColorSpace(napi_env env, napi_callback_info info);
    napi_value OnGetColorSpace(napi_env env, napi_callback_info info);
    napi_value OnGetWindowColorSpaceSync(napi_env env, napi_callback_info info);
    napi_value OnSetBackgroundColor(napi_env env, napi_callback_info info);
    napi_value OnSetWindowBackgroundColorSync(napi_env env, napi_callback_info info);
    napi_value OnSetBrightness(napi_env env, napi_callback_info info);
    napi_value OnSetWindowBrightness(napi_env env, napi_callback_info info);
    napi_value OnSetDimBehind(napi_env env, napi_callback_info info);
    napi_value OnSetFocusable(napi_env env, napi_callback_info info);
    napi_value OnSetWindowFocusable(napi_env env, napi_callback_info info);
    napi_value OnSetTopmost(napi_env env, napi_callback_info info);
    napi_value OnSetWindowTopmost(napi_env env, napi_callback_info info);
    napi_value OnSetSubWindowZLevel(napi_env env, napi_callback_info info);
    napi_value OnGetSubWindowZLevel(napi_env env, napi_callback_info info);
    napi_value OnSetWindowDelayRaiseOnDrag(napi_env env, napi_callback_info info);
    napi_value OnSetKeepScreenOn(napi_env env, napi_callback_info info);
    napi_value OnSetWindowKeepScreenOn(napi_env env, napi_callback_info info);
    napi_value OnSetWakeUpScreen(napi_env env, napi_callback_info info);
    napi_value OnSetOutsideTouchable(napi_env env, napi_callback_info info);
    napi_value OnSetPrivacyMode(napi_env env, napi_callback_info info);
    napi_value OnSetWindowPrivacyMode(napi_env env, napi_callback_info info);
    napi_value OnSetTouchable(napi_env env, napi_callback_info info);
    napi_value OnSetTouchableAreas(napi_env env, napi_callback_info info);
    napi_value OnSetResizeByDragEnabled(napi_env env, napi_callback_info info);
    napi_value OnSetRaiseByClickEnabled(napi_env env, napi_callback_info info);
    napi_value OnSetMainWindowRaiseByClickEnabled(napi_env env, napi_callback_info info);
    napi_value OnHideNonSystemFloatingWindows(napi_env env, napi_callback_info info);
    napi_value OnSetWindowTouchable(napi_env env, napi_callback_info info);
    napi_value OnSetTransparent(napi_env env, napi_callback_info info);
    napi_value OnChangeCallingWindowId(napi_env env, napi_callback_info info);
    napi_value OnDump(napi_env env, napi_callback_info info);
    napi_value OnSetForbidSplitMove(napi_env env, napi_callback_info info);
    napi_value OnSnapshot(napi_env env, napi_callback_info info);
    napi_value OnSnapshotSync(napi_env env, napi_callback_info info);
    napi_value OnSnapshotIgnorePrivacy(napi_env env, napi_callback_info info);
    napi_value OnSetSnapshotSkip(napi_env env, napi_callback_info info);
    napi_value OnSetSingleFrameComposerEnabled(napi_env env, napi_callback_info info);
    napi_value OnEnableLandscapeMultiWindow(napi_env env, napi_callback_info info);
    napi_value OnDisableLandscapeMultiWindow(napi_env env, napi_callback_info info);

    // animation Config
    napi_value OnOpacity(napi_env env, napi_callback_info info);
    napi_value OnScale(napi_env env, napi_callback_info info);
    napi_value OnRotate(napi_env env, napi_callback_info info);
    napi_value OnTranslate(napi_env env, napi_callback_info info);
    napi_value OnGetTransitionController(napi_env env, napi_callback_info info);
    WmErrorCode CreateTransitionController(napi_env env);

    /*
     * Window Property
     */
    napi_value OnSetCornerRadius(napi_env env, napi_callback_info info);
    napi_value OnSetWindowCornerRadius(napi_env env, napi_callback_info info);
    napi_value OnGetWindowCornerRadius(napi_env env, napi_callback_info info);
    napi_value OnSetShadow(napi_env env, napi_callback_info info);
    napi_value OnSetWindowShadowRadius(napi_env env, napi_callback_info info);
    napi_value OnSetBlur(napi_env env, napi_callback_info info);
    napi_value OnSetBackdropBlur(napi_env env, napi_callback_info info);
    napi_value OnSetBackdropBlurStyle(napi_env env, napi_callback_info info);
    napi_value OnSetWaterMarkFlag(napi_env env, napi_callback_info info);
    napi_value OnSetHandwritingFlag(napi_env env, napi_callback_info info);
    napi_value OnSetWindowGrayScale(napi_env env, napi_callback_info info);
    napi_value OnEnableDrag(napi_env env, napi_callback_info info);
    napi_value OnStartMoving(napi_env env, napi_callback_info info);
    napi_value OnStopMoving(napi_env env, napi_callback_info info);
    napi_value OnSetRotationLocked(napi_env env, napi_callback_info info);
    napi_value OnGetRotationLocked(napi_env env, napi_callback_info info);

    /*
     * Sub Window
     */
    napi_value OnCreateSubWindowWithOptions(napi_env env, napi_callback_info info);
    napi_value OnSetSubWindowModal(napi_env env, napi_callback_info info);
    napi_value OnSetParentWindow(napi_env env, napi_callback_info info);
    napi_value OnGetParentWindow(napi_env env, napi_callback_info info);

    /*
     * Gesture Back
     */
    napi_value OnSetGestureBackEnabled(napi_env env, napi_callback_info info);
    napi_value OnGetGestureBackEnabled(napi_env env, napi_callback_info info);

    /*
     * PC Window
     */
    napi_value OnSetWindowMask(napi_env env, napi_callback_info info);
    napi_value OnGetWindowStatus(napi_env env, napi_callback_info info);
    napi_value OnSetFollowParentMultiScreenPolicy(napi_env env, napi_callback_info info);
    napi_value OnIsInFreeWindowMode(napi_env env, napi_callback_info info);

    /*
     * Window Transition Animation For PC
     */
    napi_value OnSetWindowTransitionAnimation(napi_env env, napi_callback_info info);
    napi_value OnGetWindowTransitionAnimation(napi_env env, napi_callback_info info);

    /*
     * Window Decor
     */
    napi_value OnDisableWindowDecor(napi_env env, napi_callback_info info);
    napi_value OnSetWindowDecorVisible(napi_env env, napi_callback_info info);
    napi_value OnGetWindowDecorVisible(napi_env env, napi_callback_info info);
    napi_value OnSetWindowTitleMoveEnabled(napi_env env, napi_callback_info info);
    napi_value OnSetWindowDecorHeight(napi_env env, napi_callback_info info);
    napi_value OnGetWindowDecorHeight(napi_env env, napi_callback_info info);
    napi_value OnGetTitleButtonRect(napi_env env, napi_callback_info info);
    napi_value OnSetTitleButtonVisible(napi_env env, napi_callback_info info);
    napi_value OnSetWindowTitleButtonVisible(napi_env env, napi_callback_info info);
    napi_value OnSetWindowContainerColor(napi_env env, napi_callback_info info);
    napi_value OnSetWindowContainerModalColor(napi_env env, napi_callback_info info);
    napi_value OnSetDecorButtonStyle(napi_env env, napi_callback_info info);
    napi_value OnGetDecorButtonStyle(napi_env env, napi_callback_info info);
    napi_value OnSetWindowTitle(napi_env env, napi_callback_info info);

    /*
     * PC Window Layout
     */
    napi_value OnRecover(napi_env env, napi_callback_info info);
    napi_value OnMaximize(napi_env env, napi_callback_info info);
    napi_value OnSetTitleAndDockHoverShown(napi_env env, napi_callback_info info);
    napi_value OnRestore(napi_env env, napi_callback_info info);
    napi_value OnStartMoveWindowWithCoordinate(napi_env env, size_t argc, napi_value* argv);
    napi_value OnSetDragKeyFramePolicy(napi_env env, napi_callback_info info);

    std::string windowName_;
    sptr<Window> windowToken_ = nullptr;
    std::unique_ptr<JsWindowRegisterManager> registerManager_ = nullptr;
    std::shared_ptr<NativeReference> jsTransControllerObj_ = nullptr;

    /*
     * Window Immersive
     */
    napi_value OnSetFullScreen(napi_env env, napi_callback_info info);
    napi_value OnSetLayoutFullScreen(napi_env env, napi_callback_info info);
    napi_value OnSetWindowLayoutFullScreen(napi_env env, napi_callback_info info);
    napi_value OnSetSystemBarEnable(napi_env env, napi_callback_info info);
    napi_value OnSetWindowSystemBarEnable(napi_env env, napi_callback_info info);
    napi_value OnSetSystemBarProperties(napi_env env, napi_callback_info info);
    napi_value OnGetWindowSystemBarPropertiesSync(napi_env env, napi_callback_info info);
    napi_value OnSetWindowSystemBarProperties(napi_env env, napi_callback_info info);
    napi_value OnSetStatusBarColor(napi_env env, napi_callback_info info);
    napi_value OnGetStatusBarPropertySync(napi_env env, napi_callback_info info);
    napi_value OnGetWindowStateSnapshot(napi_env env, napi_callback_info info);
    napi_value OnGetAvoidArea(napi_env env, napi_callback_info info);
    napi_value OnGetWindowAvoidAreaSync(napi_env env, napi_callback_info info);
    napi_value OnGetWindowAvoidAreaIgnoringVisibilitySync(napi_env env, napi_callback_info info);
    napi_value OnSetSpecificSystemBarEnabled(napi_env env, napi_callback_info info);
    napi_value OnSetImmersiveModeEnabledState(napi_env env, napi_callback_info info);
    napi_value OnGetImmersiveModeEnabledState(napi_env env, napi_callback_info info);
    napi_value OnIsImmersiveLayout(napi_env env, napi_callback_info info);
    napi_value OnSetSystemAvoidAreaEnabled(napi_env env, napi_callback_info info);
    napi_value OnIsSystemAvoidAreaEnabled(napi_env env, napi_callback_info info);
    napi_value OnSetFollowParentWindowLayoutEnabled(napi_env env, napi_callback_info info);
    napi_value OnSetRelativePositionToParentWindowEnabled(napi_env env, napi_callback_info info);
    napi_value OnSetWindowShadowEnabled(napi_env env, napi_callback_info info);

    /*
     * Window Event
     */
    napi_value OnSetReceiveDragEventEnabled(napi_env env, napi_callback_info info);
    napi_value OnIsReceiveDragEventEnabled(napi_env env, napi_callback_info info);
    napi_value OnSetSeparationTouchEnabled(napi_env env, napi_callback_info info);
    napi_value OnIsSeparationTouchEnabled(napi_env env, napi_callback_info info);
};
}  // namespace Rosen
}  // namespace OHOS
#endif