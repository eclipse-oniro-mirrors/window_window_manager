/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#ifndef WINDOW_FFI_H
#define WINDOW_FFI_H

#include <cstdint>
#include "cj_ffi/cj_common_ffi.h"
#include "singleton_container.h"
#include "window_impl.h"
#include "window_manager.h"
#include "window_manager_impl.h"
#include "window_stage_impl.h"

extern "C" {
    // window maneger
    FFI_EXPORT RetDataI64 FfiOHOSCreateWindow(char* name, uint32_t windowType, OHOS::AbilityRuntime::Context* context,
        int64_t displayId, int64_t parentId);
    FFI_EXPORT int32_t FfiOHOSWindowSetWindowLayoutMode(uint32_t mode);
    FFI_EXPORT int32_t FfiOHOSWindowMinimizeAll(int64_t displayId);
    FFI_EXPORT RetDataI64 FfiOHOSGetLastWindow(OHOS::AbilityRuntime::Context* ctx);
    FFI_EXPORT int32_t FfiOHOSShiftAppWindowFocus(int32_t sourceWindowId, int32_t targetWindowId);

    // window
    FFI_EXPORT int32_t FfiOHOSWindowHide(int64_t id);
    FFI_EXPORT int32_t FfiOHOSWindowHideWithAnimation(int64_t id);
    FFI_EXPORT int32_t FfiOHOSWindowShowWindow(int64_t id);
    FFI_EXPORT int32_t FfiOHOSWindowShowWithAnimation(int64_t id);
    FFI_EXPORT int32_t FfiOHOSWindowDestroyWindow(int64_t id);
    FFI_EXPORT int32_t FfiOHOSWindowMoveWindowTo(int64_t id, int32_t x, int32_t y);
    FFI_EXPORT int32_t FfiOHOSWindowResize(int64_t id, uint32_t width, uint32_t height);
    FFI_EXPORT int32_t FfiOHOSWindowSetWindowMode(int64_t id, uint32_t mode);
    FFI_EXPORT CWindowProperties FfiOHOSWindowGetWindowProperties(int64_t id, int32_t* errCode);
    FFI_EXPORT int32_t FfiOHOSWindowSetWindowLayoutFullScreen(int64_t id, bool isLayoutFullScreen);
    FFI_EXPORT int32_t FfiOHOSSetWindowBackgroundColor(int64_t id, const char* color);
    FFI_EXPORT int32_t FfiOHOSWindowSetWindowBrightness(int64_t id, float brightness);
    FFI_EXPORT int32_t FfiOHOSWindowSetWindowFocusable(int64_t id, bool focusable);
    FFI_EXPORT int32_t FfiOHOSWindowSetWindowKeepScreenOn(int64_t id, bool keepScreenOn);
    FFI_EXPORT int32_t FfiOHOSWindowSetWindowPrivacyMode(int64_t id, bool isPrivacyMode);
    FFI_EXPORT int32_t FfiOHOSWindowSetWindowTouchable(int64_t id, bool touchable);
    FFI_EXPORT int32_t FfiOHOSWindowSetForbidSplitMove(int64_t id, bool isForbidSplitMove);
    FFI_EXPORT bool FfiOHOSWindowIsWindowSupportWideGamut(int64_t id, int32_t* errCode);
    FFI_EXPORT bool FfiOHOSWindowIsWindowShowing(int64_t id, int32_t* errCode);
    FFI_EXPORT RetDataI64 FFiOHOSWindowFindWindow(char* name);
    FFI_EXPORT int32_t FFiOHOSWindowToggleShownStateForAllAppWindows();
    FFI_EXPORT int32_t FFiOHOSWindowSetGestureNavigationEnabled(bool enable);
    FFI_EXPORT int32_t FFiOHOSWindowSetWaterMarkImage(int64_t pixelMapId, bool enable);
    FFI_EXPORT int32_t FFiOHOSWindowSetBackdropBlurStyle(int64_t id, uint32_t blurStyle);
    FFI_EXPORT int32_t FFiOHOSWindowSetPreferredOrientation(int64_t id, uint32_t orientation);
    FFI_EXPORT int32_t FFiOHOSWindowGetWindowAvoidArea(int64_t id, uint32_t areaType, OHOS::Rosen::CAvoidArea* retPtr);
    FFI_EXPORT int32_t FFiOHOSWindowSetShadowRadius(int64_t id, double radius);
    FFI_EXPORT int32_t FFiOHOSWindowSetShadowColor(int64_t id, char* color);
    FFI_EXPORT int32_t FFiOHOSWindowSetShadowOffsetX(int64_t id, double offsetX);
    FFI_EXPORT int32_t FFiOHOSWindowSetShadowOffsetY(int64_t id, double offsetY);
    FFI_EXPORT int32_t FFiOHOSWindowSetBackdropBlur(int64_t id, double radius);
    FFI_EXPORT int32_t FFiOHOSWindowSetBlur(int64_t id, double radius);
    FFI_EXPORT int32_t FFiOHOSWindowSetWaterMarkFlag(int64_t id, bool enable);
    FFI_EXPORT int32_t FFiOHOSWindowSetAspectRatio(int64_t id, double ratio);
    FFI_EXPORT int32_t FFiOHOSWindowResetAspectRatio(int64_t id);
    FFI_EXPORT int32_t FFiOHOSWindowMinimize(int64_t id);
    FFI_EXPORT int32_t FFiOHOSWindowSetWindowColorSpace(int64_t id, uint32_t colorSpace);
    FFI_EXPORT int32_t FFiOHOSWindowSetCornerRadius(int64_t id, float radius);
    FFI_EXPORT int32_t FFiOHOSWindowSetResizeByDragEnabled(int64_t id, bool enable);
    FFI_EXPORT int32_t FFiOHOSWindowRaiseToAppTop(int64_t id);
    FFI_EXPORT int32_t FFiOHOSWindowSetSnapshotSkip(int64_t id, bool isSkip);
    FFI_EXPORT int32_t FFiOHOSWindowSetWakeUpScreen(int64_t id, bool wakeUp);
    FFI_EXPORT int32_t FFiOHOSWindowSetRaiseByClickEnabled(int64_t id, bool enable);
    FFI_EXPORT int32_t FFiOHOSWindowGetWindowColorSpace(int64_t id, int32_t* errCode);
    FFI_EXPORT int32_t FFiOHOSWindowRaiseAboveTarget(int64_t id, int32_t windowId);
    FFI_EXPORT int32_t FFiOHOSWindowTranslate(int64_t id, double x, double y, double z);
    FFI_EXPORT int32_t FFiOHOSWindowRotate(int64_t id, WindowRotate window);
    FFI_EXPORT int32_t FFiOHOSWindowScale(int64_t id, double x, double y, double pivotX, double pivotY);
    FFI_EXPORT int32_t FFiOHOSWindowOpacity(int64_t id, double opacity);
    FFI_EXPORT int64_t FFiOHOSWindowSnapshot(int64_t id, int32_t* errCode);
    FFI_EXPORT int32_t FFiOHOSWindowSetWindowSystemBarEnable(int64_t id, CArrString arr);
    FFI_EXPORT int32_t FFiOHOSWindowSetWindowSystemBarProperties(int64_t id, CJBarProperties properties);
    FFI_EXPORT int32_t FFiOHOSMinimize(int64_t id);
    FFI_EXPORT int32_t FFiOHOSSetWindowColorSpace(int64_t id, uint32_t colorSpace);
    FFI_EXPORT int32_t FfiOHOSOnKeyboardHeightChange(int64_t id, int64_t callbackId);
    FFI_EXPORT int32_t FfiOHOSOffKeyboardHeightChange(int64_t id);
    FFI_EXPORT int32_t FfiOHOSOnWindowEvent(int64_t id, int64_t callbackId);
    FFI_EXPORT int32_t FfiOHOSOnNoInteractionDetected(int64_t id, int64_t timeout, int64_t callbackId);
    FFI_EXPORT int32_t FfiOHOSOnCallback(int64_t id, int64_t callbackId, const char* callbackType);
    FFI_EXPORT int32_t FfiOHOSOffCallback(int64_t id, int64_t callbackId, const char* callbackType);
    FFI_EXPORT int32_t FfiOHOSSetSubWindowModal(int64_t id, bool isModal);
    FFI_EXPORT int32_t FfiOHOSIsFocused(int64_t id, int32_t* errCode);
    FFI_EXPORT int32_t FfiOHOSSetWindowLimits(int64_t id, OHOS::Rosen::CWindowLimits windloLimits,
                                              OHOS::Rosen::CWindowLimits* retPtr);
    FFI_EXPORT int32_t FfiOHOSGetWindowLimits(int64_t id, OHOS::Rosen::CWindowLimits* retPtr);
    FFI_EXPORT bool FfiOHOSGetImmersiveModeEnabledState(int64_t id, int32_t* errCode);
    FFI_EXPORT int32_t FfiOHOSSetImmersiveModeEnabledState(int64_t id, bool enabled);
    FFI_EXPORT int32_t FfiOHOSKeepKeyboardOnFocus(int64_t id, bool keepKeyboardFlag);
    FFI_EXPORT int32_t FfiOHOSGetWindowDecorHeight(int64_t id, int32_t* height);
    FFI_EXPORT int32_t FfiOHOSSetWindowDecorHeight(int64_t id, int32_t height);
    FFI_EXPORT int32_t FfiOHOSRecover(int64_t id);
    FFI_EXPORT int32_t FfiOHOSSetWindowDecorVisible(int64_t id, bool isVisible);
    FFI_EXPORT int32_t FfiOHOSGetTitleButtonRect(int64_t id, OHOS::Rosen::CTitleButtonRect* retPtr);
    FFI_EXPORT int32_t FfiOHOSSetDialogBackGestureEnabled(int64_t id, bool enabled);
    FFI_EXPORT int32_t FfiOHOSDisableLandscapeMultiWindow(int64_t id);
    FFI_EXPORT int32_t FfiOHOSEnableLandscapeMultiWindow(int64_t id);
    FFI_EXPORT int32_t FfiOHOSSetWindowGrayScale(int64_t id, float grayScale);
    FFI_EXPORT int32_t FfiOHOSSpecificSystemBarEnabled(int64_t id, int32_t name,
                                                       bool enable,
                                                       bool enableAnimation);
    FFI_EXPORT int32_t FfiOHOSGetWindowSystemBarProperties(int64_t id, CJBarProperties* systemBarProperty);
    FFI_EXPORT uint32_t FfiOHOSGetPreferredOrientation(int64_t id, int32_t* errCode);
    FFI_EXPORT int32_t FfiOHOSGetWindowStatus(int64_t id, int32_t* errCode);
    FFI_EXPORT int32_t FfiOHOSMaximize(int64_t id, int32_t presentation);
    FFI_EXPORT RetDataI64 FfiOHOSCreateSubWindowWithOptions(int64_t id, char* name, CSubWindowOptions option);

    // WindowStage
    FFI_EXPORT RetDataI64 FfiOHOSBindWindowStage(int64_t windowStageImplPtr);
    FFI_EXPORT RetDataI64 FfiOHOSGetMainWindow(int64_t id);
    FFI_EXPORT RetDataI64 FfiOHOSCreateSubWindow(int64_t id, char* name);
    FFI_EXPORT RetStruct FfiOHOSGetSubWindow(int64_t id);
    FFI_EXPORT int32_t FfiOHOSLoadContent(int64_t id, char* path);
    FFI_EXPORT int32_t FfiOHOSLoadContentByName(int64_t id, char* name);
    FFI_EXPORT int32_t FfiOHOSDisableWindowDecor(int64_t id);
    FFI_EXPORT int32_t FfiOHOSSetShowOnLockScreen(int64_t id, bool showOnLockScreen);
    FFI_EXPORT int32_t FfiOHOSSetDefaultDensityEnabled(int64_t id, bool enabled);
    FFI_EXPORT int32_t FfiOHOSStageOn(int64_t id, int64_t callbackId);
    FFI_EXPORT int32_t FfiOHOSStageOff(int64_t id, int64_t callbackId);
    FFI_EXPORT RetDataI64 FfiOHOSCreateSubWindowWithOptionsStage(int64_t id, const char* name,
        const char* title, bool decorEnabled, bool isModal);
}

#endif
