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

extern "C" {
FFI_EXPORT int FfiOHOSCreateWindow = 0;
FFI_EXPORT int FfiOHOSSetWindowLayoutMode = 0;
FFI_EXPORT int FfiOHOSMinimizeAll = 0;
FFI_EXPORT int FfiOHOSGetLastWindow = 0;
FFI_EXPORT int FfiOHOSWindowHide = 0;
FFI_EXPORT int FfiOHOSWindowHideWithAnimation = 0;
FFI_EXPORT int FfiOHOSWindowShowWindow = 0;
FFI_EXPORT int FfiOHOSWindowShowWithAnimation = 0;
FFI_EXPORT int FfiOHOSWindowDestroyWindow = 0;
FFI_EXPORT int FfiOHOSWindowMoveWindowTo = 0;
FFI_EXPORT int FfiOHOSWindowResize = 0;
FFI_EXPORT int FfiOHOSWindowSetWindowMode = 0;
FFI_EXPORT int FfiOHOSWindowGetWindowProperties = 0;
FFI_EXPORT int FfiOHOSWindowSetWindowLayoutFullScreen = 0;
FFI_EXPORT int FfiOHOSSetWindowBackgroundColor = 0;
FFI_EXPORT int FfiOHOSWindowSetWindowBrightness = 0;
FFI_EXPORT int FfiOHOSWindowSetWindowFocusable = 0;
FFI_EXPORT int FfiOHOSWindowSetWindowKeepScreenOn = 0;
FFI_EXPORT int FfiOHOSWindowSetWindowPrivacyMode = 0;
FFI_EXPORT int FfiOHOSWindowSetWindowTouchable = 0;
FFI_EXPORT int FfiOHOSWindowSetForbidSplitMove = 0;
FFI_EXPORT int FfiOHOSWindowIsWindowSupportWideGamut = 0;
FFI_EXPORT int FfiOHOSWindowIsWindowShowing = 0;
FFI_EXPORT int FFiOHOSWindowFindWindow = 0;
FFI_EXPORT int FFiOHOSWindowToggleShownStateForAllAppWindows = 0;
FFI_EXPORT int FFiOHOSWindowSetGestureNavigationEnabled = 0;
FFI_EXPORT int FFiOHOSWindowSetWaterMarkImage = 0;
FFI_EXPORT int FFiOHOSWindowSetBackdropBlurStyle = 0;
FFI_EXPORT int FFiOHOSWindowSetPreferredOrientation = 0;
FFI_EXPORT int FFiOHOSWindowGetWindowAvoidArea = 0;
FFI_EXPORT int FFiOHOSWindowSetShadowRadius = 0;
FFI_EXPORT int FFiOHOSWindowSetShadowColor = 0;
FFI_EXPORT int FFiOHOSWindowSetShadowOffsetX = 0;
FFI_EXPORT int FFiOHOSWindowSetShadowOffsetY = 0;
FFI_EXPORT int FFiOHOSWindowSetBackdropBlur = 0;
FFI_EXPORT int FFiOHOSWindowSetBlur = 0;
FFI_EXPORT int FFiOHOSWindowSetWaterMarkFlag = 0;
FFI_EXPORT int FFiOHOSWindowSetAspectRatio = 0;
FFI_EXPORT int FFiOHOSWindowResetAspectRatio = 0;
FFI_EXPORT int FFiOHOSWindowMinimize = 0;
FFI_EXPORT int FFiOHOSWindowSetWindowColorSpace = 0;
FFI_EXPORT int FFiOHOSWindowSetCornerRadius = 0;
FFI_EXPORT int FFiOHOSWindowSetResizeByDragEnabled = 0;
FFI_EXPORT int FFiOHOSWindowRaiseToAppTop = 0;
FFI_EXPORT int FFiOHOSWindowSetSnapshotSkip = 0;
FFI_EXPORT int FFiOHOSWindowSetWakeUpScreen = 0;
FFI_EXPORT int FFiOHOSWindowSetRaiseByClickEnabled = 0;
FFI_EXPORT int FFiOHOSWindowGetWindowColorSpace = 0;
FFI_EXPORT int FFiOHOSWindowRaiseAboveTarget = 0;
FFI_EXPORT int FFiOHOSWindowTranslate = 0;
FFI_EXPORT int FFiOHOSWindowRotate = 0;
FFI_EXPORT int FFiOHOSWindowScale = 0;
FFI_EXPORT int FFiOHOSWindowOpacity = 0;
FFI_EXPORT int FFiOHOSWindowSnapshot = 0;
FFI_EXPORT int FFiOHOSWindowSetWindowSystemBarEnable = 0;
FFI_EXPORT int FFiOHOSWindowSetWindowSystemBarProperties = 0;
FFI_EXPORT int FFiOHOSMinimize = 0;
FFI_EXPORT int FFiOHOSSetWindowColorSpace = 0;
FFI_EXPORT int FfiOHOSOnKeyboardHeightChange = 0;
FFI_EXPORT int FfiOHOSOffKeyboardHeightChange = 0;
FFI_EXPORT int FfiOHOSBindWindowStage = 0;
FFI_EXPORT int FfiOHOSGetMainWindow = 0;
FFI_EXPORT int FfiOHOSCreateSubWindow = 0;
FFI_EXPORT int FfiOHOSGetSubWindow = 0;
FFI_EXPORT int FfiOHOSLoadContent = 0;
FFI_EXPORT int FfiOHOSLoadContentByName = 0;
FFI_EXPORT int FfiOHOSDisableWindowDecor = 0;
FFI_EXPORT int FfiOHOSSetShowOnLockScreen = 0;
FFI_EXPORT int FfiOHOSWindowMinimizeAll = 0;
FFI_EXPORT int FfiOHOSWindowSetWindowLayoutMode = 0;
FFI_EXPORT int FfiOHOSSetSubWindowModal = 0;
FFI_EXPORT int FfiOHOSIsFocused = 0;
FFI_EXPORT int FfiOHOSSetWindowLimits = 0;
FFI_EXPORT int FfiOHOSGetWindowLimits = 0;
FFI_EXPORT bool FfiOHOSGetImmersiveModeEnabledState = 0;
FFI_EXPORT int FfiOHOSSetImmersiveModeEnabledState = 0;
FFI_EXPORT int FfiOHOSKeepKeyboardOnFocus = 0;
FFI_EXPORT int FfiOHOSGetWindowDecorHeight = 0;
FFI_EXPORT int FfiOHOSSetWindowDecorHeight = 0;
FFI_EXPORT int FfiOHOSRecover = 0;
FFI_EXPORT int FfiOHOSSetWindowDecorVisible = 0;
FFI_EXPORT int FfiOHOSGetTitleButtonRect = 0;
FFI_EXPORT int FfiOHOSSetDialogBackGestureEnabled = 0;
FFI_EXPORT int FfiOHOSDisableLandscapeMultiWindow = 0;
FFI_EXPORT int FfiOHOSEnableLandscapeMultiWindow = 0;
FFI_EXPORT int FfiOHOSSetWindowGrayScale = 0;
FFI_EXPORT int FfiOHOSSpecificSystemBarEnabled = 0;
FFI_EXPORT int FfiOHOSGetWindowSystemBarProperties = 0;
FFI_EXPORT int FfiOHOSGetPreferredOrientation = 0;
FFI_EXPORT int FfiOHOSGetWindowStatus = 0;
FFI_EXPORT int FfiOHOSMaximize = 0;
FFI_EXPORT int FfiOHOSCreateSubWindowWithOptions = 0;
FFI_EXPORT int FfiOHOSOnNoInteractionDetected = 0;
FFI_EXPORT int FfiOHOSOnCallback = 0;
FFI_EXPORT int FfiOHOSOffCallback = 0;
FFI_EXPORT int FfiOHOSSetDefaultDensityEnabled = 0;
FFI_EXPORT int FfiOHOSStageOn = 0;
FFI_EXPORT int FfiOHOSStageOff = 0;
FFI_EXPORT int FfiOHOSCreateSubWindowWithOptionsStage = 0;
}
