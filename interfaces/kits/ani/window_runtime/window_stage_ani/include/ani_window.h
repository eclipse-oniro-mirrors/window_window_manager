/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#ifndef OHOS_ANI_WINDOW_H
#define OHOS_ANI_WINDOW_H

#include <functional>

#include "ani.h"
#include "ani_window_register_manager.h"
#include "window.h"
#include "wm_animation_common.h"

namespace OHOS {
namespace Rosen {
#ifdef _WIN32
#define WINDOW_EXPORT __attribute__((dllexport))
#else
#define WINDOW_EXPORT __attribute__((visibility("default")))
#endif


class AniWindow {
public:
    explicit AniWindow(const sptr<Window>& window);
    explicit AniWindow(const std::shared_ptr<OHOS::Rosen::Window>& window);
    ~AniWindow();
    sptr<Window> GetWindow() { return windowToken_; }
    ani_ref GetAniRef() { return aniRef_; }
    void SetAniRef(const ani_ref& aniRef) { aniRef_ = aniRef; }

    /* transfer window class  */
    static ani_object NativeTransferStatic(ani_env* aniEnv, ani_class cls, ani_object input);
    static ani_object NativeTransferDynamic(ani_env* aniEnv, ani_class cls, ani_long nativeObj);

    /* window obj stored in ANI */
    static AniWindow* GetWindowObjectFromEnv(ani_env* env, ani_object obj);
    static void ThrowIfWindowInvalid(ani_env* env, ani_object obj, ani_long nativeObj);

    static void SetWindowColorSpace(ani_env* env, ani_object obj, ani_long nativeObj, ani_int colorSpace);
    static void SetPreferredOrientation(ani_env* env, ani_object obj, ani_long nativeObj, ani_int orientation);
    static ani_int GetPreferredOrientation(ani_env* env, ani_object obj, ani_long nativeObj);
    static ani_int ConvertOrientationAndRotation(ani_env* env, ani_object obj, ani_long nativeObj,
        ani_int from, ani_int to, ani_int value);
    static void SetWindowPrivacyMode(ani_env* env, ani_object obj, ani_long nativeObj, ani_boolean isPrivacyMode);
    static void Recover(ani_env* env, ani_object obj, ani_long nativeObj);
    static void SetUIContent(ani_env* env, ani_object obj, ani_long nativeObj, ani_string path);
    static void SetWindowKeepScreenOn(ani_env* env, ani_object obj, ani_long nativeObj, ani_boolean isKeepScreenOn);
    static void SetWaterMarkFlag(ani_env* env, ani_object obj, ani_long nativeObj, ani_boolean enable);
    static void RaiseMainWindowAboveTarget(ani_env* env, ani_object obj, ani_long nativeObj, ani_int windowId);
    static void SetMainWindowRaiseByClickEnabled(ani_env* env, ani_object obj, ani_long nativeObj, ani_boolean enable);
    static void SetWindowFocusable(ani_env* env, ani_object obj, ani_long nativeObj, ani_boolean isFocusable);
    static ani_int GetSubWindowZLevel(ani_env* env, ani_object obj, ani_long nativeObj);
    static void SetSubWindowZLevel(ani_env* env, ani_object obj, ani_long nativeObj, ani_int zLevel);
    static void RaiseAboveTarget(ani_env* env, ani_object obj, ani_long nativeObj, ani_int windowId);
    static void RaiseToAppTop(ani_env* env, ani_object obj, ani_long nativeObj);
    static void SetTopmost(ani_env* env, ani_object obj, ani_long nativeObj, ani_boolean isTopmost);
    static void SetReceiveDragEventEnabled(ani_env* env, ani_object obj, ani_long nativeObj, ani_boolean enabled);
    static ani_boolean IsReceiveDragEventEnabled(ani_env* env, ani_object obj, ani_long nativeObj);
    static void SetSeparationTouchEnabled(ani_env* env, ani_object obj, ani_long nativeObj, ani_boolean enabled);
    static ani_boolean IsSeparationTouchEnabled(ani_env* env, ani_object obj, ani_long nativeObj);
    static void RequestFocus(ani_env* env, ani_object obj, ani_long nativeObj, ani_boolean isFocused);
    static void SetSubWindowModal(ani_env* env, ani_object obj, ani_long nativeObj, ani_boolean isModal);
    static void SetSubWindowModalType(ani_env* env, ani_object obj, ani_long nativeObj, ani_boolean isModal,
        ani_int modalityType);
    static ani_boolean IsFocused(ani_env* env, ani_object obj, ani_long nativeObj);
    static void LoadContent(ani_env* env, ani_object obj, ani_long nativeObj,
        ani_string path, ani_object storage);
    static void LoadContentByName(ani_env* env, ani_object obj, ani_long nativeObj,
        ani_string path, ani_object storage);
    static void SetWindowSystemBarEnable(ani_env* env, ani_object obj, ani_long nativeObj, ani_object nameAry);
    static void SetWindowTouchable(ani_env* env, ani_object obj, ani_long nativeObj, ani_boolean isTouchable);
    static void SetDialogBackGestureEnabled(ani_env* env, ani_object obj, ani_long nativeObj, ani_boolean enabled);
    static void SetWindowMask(ani_env* env, ani_object obj, ani_long nativeObj, ani_array windowMask);
    static void SetTouchableAreas(ani_env* env, ani_object obj, ani_long nativeObj, ani_array rects);
    static ani_object GetUIContext(ani_env* env, ani_object obj, ani_long nativeObj);
    static ani_object GetWindowAvoidArea(ani_env* env, ani_object obj, ani_long nativeObj, ani_int type);
    static ani_object GetWindowAvoidAreaIgnoringVisibility(ani_env* env, ani_object obj,
        ani_long nativeObj, ani_int type);
    static void RegisterWindowCallback(ani_env* env, ani_object obj, ani_long nativeObj, ani_string type,
        ani_ref callback);
    static void RegisterNoInteractionDetectedCallback(ani_env* env, ani_object obj, ani_long nativeObj, ani_string type,
        ani_long timeout, ani_ref callback);
    static void UnregisterWindowCallback(ani_env* env, ani_object obj, ani_long nativeObj, ani_string type,
        ani_ref callback);
    static void SetWindowTitle(ani_env* env, ani_object obj, ani_long nativeObj, ani_string titleName);
    static void SetTitleButtonVisible(ani_env* env, ani_object obj, ani_long nativeObj, ani_boolean isMaximizeVisible,
        ani_boolean isMinimizeVisible, ani_boolean isSplitVisible, ani_boolean isCloseVisible);
    static void SetWindowTitleMoveEnabled(ani_env* env, ani_object obj, ani_long nativeObj, ani_boolean enabled);
    static ani_object GetTitleButtonRect(ani_env* env, ani_object obj, ani_long nativeObj);
    static ani_object GetDecorButtonStyle(ani_env* env, ani_object obj, ani_long nativeObj);
    static void SetTitleAndDockHoverShown(ani_env* env, ani_object obj, ani_long nativeObj,
        ani_object isTitleHoverShown, ani_object isDockHoverShown);
    static void SetHandwritingFlag(ani_env* env, ani_object obj, ani_long nativeObj, ani_boolean enable);
    static ani_boolean GetWindowDecorVisible(ani_env* env, ani_object obj, ani_long nativeObj);
    static void ShowWindow(ani_env* env, ani_object obj, ani_long nativeObj);
    static void ShowWindowWithOptions(ani_env* env, ani_object obj, ani_long nativeObj,
        ani_object aniShowWindowOptions);
    static void BindDialogTarget(ani_env* env, ani_object obj, ani_long nativeObj,
        ani_object argv, ani_ref deathCallback);
    static void DestroyWindow(ani_env* env, ani_object obj, ani_long nativeObj);
    static ani_boolean IsWindowShowing(ani_env* env, ani_object obj, ani_long nativeObj);
    static ani_boolean IsWindowHighlighted(ani_env* env, ani_object obj, ani_long nativeObj);
    static void HideWithAnimation(ani_env* env, ani_object obj, ani_long nativeObj);
    static void ShowWithAnimation(ani_env* env, ani_object obj, ani_long nativeObj);
    static void Opacity(ani_env* env, ani_object obj, ani_long nativeObj, ani_double opacity);
    static void Scale(ani_env* env, ani_object obj, ani_long nativeObj, ani_object scaleOptions);
    static void Translate(ani_env* env, ani_object obj, ani_long nativeObj, ani_object translateOptions);
    static void Rotate(ani_env* env, ani_object obj, ani_long nativeObj, ani_object rotateOptions);
    static void SetShadow(ani_env* env, ani_object obj, ani_long nativeObj, ani_double radius,
        ani_string color, ani_object offsetX, ani_object offsetY);
    static void SetCornerRadius(ani_env* env, ani_object obj, ani_long nativeObj, ani_double cornerRadius);
    static void SetBlur(ani_env* env, ani_object obj, ani_long nativeObj, ani_double radius);
    static void SetBackdropBlurStyle(ani_env* env, ani_object obj, ani_long nativeObj, ani_int blurStyle);
    static void SetBackdropBlur(ani_env* env, ani_object obj, ani_long nativeObj, ani_double radius);
    static ani_double GetWindowCornerRadius(ani_env* env, ani_object obj, ani_long nativeObj);
    static void SetWindowCornerRadius(ani_env* env, ani_object obj, ani_long nativeObj, ani_double cornerRadius);
    static void SetWindowShadowRadius(ani_env* env, ani_object obj, ani_long nativeObj, ani_double radius);
    static void Finalizer(ani_env* env, ani_long nativeObj);
    static void SetContentAspectRatio(ani_env* env, ani_object obj, ani_long nativeObj,
                                      ani_double ratio, ani_boolean isPersistent, ani_boolean needUpdateRect);
    static ani_object CreateAniWindow(ani_env* env, OHOS::sptr<OHOS::Rosen::Window>& window);
    static ani_int GetWindowStatus(ani_env* env, ani_object obj, ani_long nativeObj);
    static void Minimize(ani_env* env, ani_object obj, ani_long nativeObj);
    static void Maximize(ani_env* env, ani_object obj, ani_long nativeObj,
                         ani_object aniPresentation, ani_object aniAcrossDisplay);
    static void StartMoving(ani_env* env, ani_object obj, ani_long nativeObj);
    static void StartMoveWindowWithCoordinate(ani_env* env, ani_object obj, ani_long nativeObj,
                                              ani_int offsetX, ani_int offsetY);
    static void StopMoving(ani_env* env, ani_object obj, ani_long nativeObj);
    static void SetRotationLocked(ani_env* env, ani_object obj, ani_long nativeObj, ani_boolean locked);
    static ani_boolean GetRotationLocked(ani_env* env, ani_object obj, ani_long nativeObj);
    static ani_boolean IsInFreeWindowMode(ani_env* env, ani_object obj, ani_long nativeObj);
    static ani_string GetWindowStateSnapshot(ani_env* env, ani_object obj, ani_long nativeObj);
    static void SetRelativePositionToParentWindowEnabled(ani_env* env, ani_object obj, ani_long nativeObj,
        ani_boolean enabled, ani_int anchor, ani_int offsetX, ani_int offsetY);
    static void SetWindowDelayRaiseOnDrag(ani_env* env, ani_object obj, ani_long nativeObj, ani_boolean isEnabled);
    static void SetDefaultDensityEnabled(ani_env* env, ani_object obj, ani_long nativeObj, ani_boolean enabled);
    static void SetWindowContainerColor(ani_env* env, ani_object obj, ani_long nativeObj,
        ani_string activeColor, ani_string inactiveColor);
    static void SetWindowContainerModalColor(ani_env* env, ani_object obj, ani_long nativeObj,
        ani_string activeColor, ani_string inactiveColor);
    static bool IsMainWindowFullScreenAcrossDisplays(ani_env* env, ani_object obj, ani_long nativeObj);
    static void SetWindowShadowEnabled(ani_env* env, ani_object obj, ani_long nativeObj, ani_boolean enable);
    static bool IsImmersiveLayout(ani_env* env, ani_object obj, ani_long nativeObj);
    static ani_object GetTransitionController(ani_env* env, ani_object obj, ani_long nativeObj);
    static void KeepKeyboardOnFocus(ani_env* env, ani_object obj, ani_long nativeObj, ani_boolean keepKeyboardFlag);

    static ani_object CreateSubWindowWithOptions(ani_env* env, ani_object obj, ani_long nativeObj,
        ani_string name, ani_object options);
    static void Hide(ani_env* env, ani_object obj, ani_long nativeObj);
    static void RestoreMainWindow(ani_env* env, ani_object obj, ani_long nativeObj, ani_object wantParameters);

    ani_ref GetParentWindow(ani_env* env);
    void SetParentWindow(ani_env* env, ani_int windowId);
    void SetWindowTopmost(ani_env* env, ani_boolean isWindowTopmost);
    void SetDecorButtonStyle(ani_env* env, ani_object decorStyle);
    void SetWindowTitleButtonVisible(ani_env* env, ani_object visibleParam);
    void Restore(ani_env* env);
    void HideWindowFunction(ani_env* env, WmErrorCode errCode);
    /*
     * Window Layout
     */
    void Resize(ani_env* env, ani_int width, ani_int height);
    void MoveWindowTo(ani_env* env, ani_int x, ani_int y);
    ani_object GetGlobalRect(ani_env* env);

    ani_int GetWindowDecorHeight(ani_env* env);
    ani_object SetWindowBackgroundColor(ani_env* env, const std::string& color);
    ani_object SetImmersiveModeEnabledState(ani_env* env, bool enable);
    ani_object SetWindowDecorVisible(ani_env* env, bool isVisible);
    ani_object SetWindowDecorHeight(ani_env* env, ani_int height);
    ani_object GetWindowPropertiesSync(ani_env* env);
    ani_boolean IsWindowSupportWideGamut(ani_env* env);
    ani_object SetWindowLayoutFullScreen(ani_env* env, ani_boolean isLayoutFullScreen);
    ani_object SetRaiseByClickEnabled(ani_env* env, ani_boolean enable);
    ani_object SetExclusivelyHighlighted(ani_env* env, ani_boolean exclusivelyHighlighted);
    void SetSystemBarProperties(ani_env* env, ani_object aniSystemBarProperties);
    ani_object SetSpecificSystemBarEnabled(ani_env* env, ani_string name, ani_boolean enable,
        ani_object enableAnimation);
    ani_object SetDragKeyFramePolicy(ani_env* env, ani_object aniKeyFramePolicy);
    ani_object Snapshot(ani_env* env);
    ani_object SnapshotSync(ani_env* env);
    void HideNonSystemFloatingWindows(ani_env* env, ani_boolean shouldHide);
    void SetWindowBrightness(ani_env* env, ani_double brightness);
    ani_int GetWindowColorSpace(ani_env* env);
    void SetWakeUpScreen(ani_env* env, ani_boolean wakeUp);
    void SetSnapshotSkip(ani_env* env, ani_boolean isSkip);
    ani_object SnapshotIgnorePrivacy(ani_env* env);
    ani_object GetStatusBarProperty(ani_env* env);
    void SetStatusBarColor(ani_env* env, ani_long color);
    ani_boolean IsSystemAvoidAreaEnabled(ani_env* env);
    void SetSystemAvoidAreaEnabled(ani_env* env, ani_boolean enabled);
    ani_object GetWindowDensityInfo(ani_env* env);
    ani_boolean GetImmersiveModeEnabledState(ani_env* env);
    void SetWindowGrayScale(ani_env* env, ani_double grayScale);
    ani_object GetWindowSystemBarProperties(ani_env* env);
    ani_boolean IsGestureBackEnabled(ani_env* env);
    void SetGestureBackEnabled(ani_env* env, ani_boolean enabled);
    void SetSingleFrameComposerEnabled(ani_env* env, ani_boolean enabled);
    void ResizeAsync(ani_env* env, ani_int width, ani_int height);
    ani_object SetWindowLimits(ani_env* env, ani_object inWindowLimits, ani_object forcible);
    ani_object GetWindowLimits(ani_env* env);
    ani_object GetWindowLimitsVP(ani_env* env);
    void SetAspectRatio(ani_env* env, ani_double ratio);
    void ResetAspectRatio(ani_env* env);
    void SetResizeByDragEnabled(ani_env* env, ani_boolean enable);
    void EnableDrag(ani_env* env, ani_boolean enable);
    void MoveWindowToGlobal(ani_env* env, ani_int x, ani_int y, ani_object inMoveConfiguration);
    void MoveWindowToAsync(ani_env* env, ani_int x, ani_int y, ani_object inMoveConfiguration);
    void SetWindowMode(ani_env* env, ani_enum_item mode);
    void SetForbidSplitMove(ani_env* env, ani_boolean isForbidSplitMove);
    void SetFollowParentWindowLayoutEnabled(ani_env* env, ani_boolean enable);
    void SetFollowParentMultiScreenPolicy(ani_env* env, ani_boolean enable);
    void MoveWindowToGlobalDisplay(ani_env* env, ani_int x, ani_int y);
    ani_object HandlePositionTransform(ani_env* env, ani_int x, ani_int y,
        std::function<WMError(sptr<Window>&, const Position&, Position&)> transformFunc);

    /*
     * Window animation
     */
    static void SetWindowTransitionAnimation(ani_env* env, ani_object obj, ani_long nativeObj,
        ani_enum_item transitionType, ani_object animation);
    static ani_object GetWindowTransitionAnimation(ani_env* env, ani_object obj, ani_long nativeObj,
        ani_enum_item transitionType);
private:
    ani_object OnCreateSubWindowWithOptions(ani_env* env, ani_string name, ani_object options);
    void OnHide(ani_env* env);
    void OnSetWindowColorSpace(ani_env* env, ani_int colorSpace);
    void OnSetPreferredOrientation(ani_env* env, ani_int orientation);
    ani_int OnGetPreferredOrientation(ani_env* env);
    ani_int OnConvertOrientationAndRotation(ani_env* env, ani_int from, ani_int to, ani_int value);
    void OnSetWindowPrivacyMode(ani_env* env, ani_boolean isPrivacyMode);
    void OnRecover(ani_env* env);
    void OnSetUIContent(ani_env* env, ani_string path);
    void OnSetWindowKeepScreenOn(ani_env* env, ani_boolean isKeepScreenOn);
    void OnSetWaterMarkFlag(ani_env* env, ani_boolean enable);
    void OnRaiseMainWindowAboveTarget(ani_env* env, ani_int windowId);
    void OnSetMainWindowRaiseByClickEnabled(ani_env* env, ani_boolean enable);
    void OnSetWindowFocusable(ani_env* env, ani_boolean isFocusable);
    ani_int OnGetSubWindowZLevel(ani_env* env);
    ani_boolean OnIsFocused(ani_env* env);
    void OnSetSubWindowZLevel(ani_env* env, ani_int zLevel);
    void OnRaiseAboveTarget(ani_env* env, ani_int windowId);
    void OnRaiseToAppTop(ani_env* env);
    void OnSetTopmost(ani_env* env, ani_boolean isTopmost);
    void OnSetReceiveDragEventEnabled(ani_env* env, ani_boolean enabled);
    bool OnIsReceiveDragEventEnabled(ani_env* env);
    void OnSetSeparationTouchEnabled(ani_env* env, ani_boolean enabled);
    bool OnIsSeparationTouchEnabled(ani_env* env);
    void OnRequestFocus(ani_env* env, ani_boolean isFocused);
    void OnSetSubWindowModal(ani_env* env, ani_boolean isModal);
    void OnSetSubWindowModalType(ani_env* env, ani_boolean isModal, ani_int modalityType);
    void OnLoadContent(ani_env* env, ani_string path, ani_object storage, bool isLoadByName);
    void OnSetWindowSystemBarEnable(ani_env* env, ani_object nameAry);
    void OnSetWindowTouchable(ani_env* env, ani_boolean isTouchable);
    void OnSetDialogBackGestureEnabled(ani_env* env, ani_boolean enabled);
    void OnSetWindowMask(ani_env* env, ani_array windowMaskArray);
    void OnSetTouchableAreas(ani_env* env, ani_array rects);
    ani_object OnGetUIContext(ani_env* env);
    ani_object OnGetWindowAvoidArea(ani_env* env, ani_int type);
    ani_object OnGetWindowAvoidAreaIgnoringVisibility(ani_env* env, ani_int type);
    void OnRegisterWindowCallback(ani_env* env, ani_string type, ani_ref callback, ani_long timeout);
    void OnUnregisterWindowCallback(ani_env* env, ani_string type, ani_ref callback);
    void OnShowWindow(ani_env* env);
    void OnShowWindowWithOptions(ani_env* env, ani_object aniShowWindowOptions);
    void OnSetWindowTitle(ani_env* env, ani_string titleName);
    void OnSetTitleButtonVisible(ani_env* env, ani_boolean isMaximizeVisible,
        ani_boolean isMinimizeVisible, ani_boolean isSplitVisible, ani_boolean isCloseVisible);
    void OnSetWindowTitleMoveEnabled(ani_env* env, ani_boolean enabled);
    ani_object OnGetTitleButtonRect(ani_env* env);
    ani_object OnGetDecorButtonStyle(ani_env* env);
    void OnSetTitleAndDockHoverShown(ani_env* env, ani_object isTitleHoverShown, ani_object isDockHoverShown);
    void OnSetHandwritingFlag(ani_env* env, ani_boolean enable);
    ani_boolean OnGetWindowDecorVisible(ani_env* env);
    void OnBindDialogTarget(ani_env* env, ani_object argv, ani_ref deathCallback);
    void OnDestroyWindow(ani_env* env);
    ani_boolean OnIsWindowShowing(ani_env* env);
    ani_boolean OnIsWindowHighlighted(ani_env* env);
    void OnHideWithAnimation(ani_env* env);
    void OnShowWithAnimation(ani_env* env);
    void OnOpacity(ani_env* env, ani_double opacity);
    void OnScale(ani_env* env, ani_object scaleOptions);
    void OnTranslate(ani_env* env, ani_object translateOptions);
    void OnRotate(ani_env* env, ani_object rotateOptions);
    void OnSetShadow(ani_env* env, ani_double radius, ani_string color, ani_object offsetX, ani_object offsetY);
    void OnSetCornerRadius(ani_env* env, ani_double cornerRadius);
    static bool ParseScaleOption(ani_env* env, ani_object scaleOptions, Transform& trans);
    static bool ParseTranslateOption(ani_env* env, ani_object translateOptions, Transform& trans);
    static bool ParseRotateOption(ani_env* env, ani_object rotateOptions, Transform& trans);
    bool CheckWindowMaskParams(ani_env* env, ani_array windowMask);
    void OnSetBlur(ani_env* env, ani_double radius);
    void OnSetBackdropBlurStyle(ani_env* env, ani_int blurStyle);
    void OnSetBackdropBlur(ani_env* env, ani_double radius);
    ani_double OnGetWindowCornerRadius(ani_env* env);
    void OnSetWindowCornerRadius(ani_env* env, ani_double cornerRadius);
    void OnSetWindowShadowRadius(ani_env* env, ani_double radius);
    bool GetSystemBarStatus(std::map<WindowType, SystemBarProperty>& systemBarProperties,
        std::map<WindowType, SystemBarPropertyFlag>& systemBarpropertyFlags,
        const std::vector<std::string>& names, sptr<Window>& window);
    void UpdateSystemBarProperties(std::map<WindowType, SystemBarProperty>& systemBarProperties,
        const std::map<WindowType, SystemBarPropertyFlag>& systemBarPropertyFlags, sptr<Window> windowToken);
    WMError SetSystemBarPropertiesByFlags(std::map<WindowType, SystemBarPropertyFlag>& systemBarPropertyFlags,
        std::map<WindowType, SystemBarProperty>& systemBarProperties, sptr<Window> windowToken);
    void OnSetRotationLocked(ani_env* env, ani_boolean locked);
    bool OnGetRotationLocked(ani_env* env);
    bool OnIsInFreeWindowMode(ani_env* env);
    ani_string OnGetWindowStateSnapshot(ani_env* env);
    void OnSetWindowDelayRaiseOnDrag(ani_env* env, ani_boolean isEnabled);
    void OnSetRelativePositionToParentWindowEnabled(ani_env* env, ani_boolean enabled,
        ani_int anchor, ani_int offsetX, ani_int offsetY);
    void OnSetDefaultDensityEnabled(ani_env* env, ani_boolean enabled);
    void OnSetWindowContainerColor(ani_env* env, ani_string activeColor, ani_string inactiveColor);
    void OnSetWindowContainerModalColor(ani_env* env, ani_string activeColor, ani_string inactiveColor);
    bool OnIsMainWindowFullScreenAcrossDisplays(ani_env* env);
    void OnSetWindowShadowEnabled(ani_env* env, ani_boolean enable);
    bool OnIsImmersiveLayout(ani_env* env);
    void OnKeepKeyboardOnFocus(ani_env* env, ani_boolean keepKeyboardFlag);
    void OnRestoreMainWindow(ani_env* env, ani_object wantParameters);

    /*
     * Window Layout
     */
    void OnSetContentAspectRatio(
        ani_env* env, ani_double ratio, ani_boolean isPersistent, ani_boolean needUpdateRect);
    ani_int OnGetWindowStatus(ani_env* env);
    void OnMinimize(ani_env* env);
    void OnMaximize(ani_env* env, ani_object aniPresentation, ani_object aniAcrossDisplay);
    void OnStartMoving(ani_env* env);
    void OnStartMoveWindowWithCoordinate(ani_env* env, ani_int offsetX, ani_int offsetY);
    void OnStopMoving(ani_env* env);

    /*
     * Window animation
     */
    WmErrorCode CreateTransitionController(ani_env* env, ani_object obj);
    ani_object OnGetTransitionController(ani_env* env, ani_object obj);
    void OnSetWindowTransitionAnimation(ani_env* env, ani_enum_item transitionType, ani_object animation);
    ani_object OnGetWindowTransitionAnimation(ani_env* env, ani_enum_item transitionType);

    sptr<Window> windowToken_ = nullptr;
    std::unique_ptr<AniWindowRegisterManager> registerManager_ = nullptr;
    ani_ref aniRef_ = nullptr;
    ani_object aniTransControllerObj_ = nullptr;
};

/* window obj stored in ANI */
AniWindow* GetWindowObjectFromAni(void* aniObj);
ani_ref CreateAniWindowObject(ani_env* env, sptr<Window>& window);
ani_ref FindAniWindowObject(const std::string& windowName);
void DropWindowObjectByAni(ani_object obj);
ani_status ANI_Window_Constructor(ani_vm *vm, uint32_t *result);
}  // namespace Rosen
}  // namespace OHOS
#endif  // OHOS_ANI_WINDOW_H
