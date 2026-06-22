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

/**
 * @file oh_window.h
 *
 * @brief Declares APIs for window
 *
 * @syscap SystemCapability.Window.SessionManager
 * @library libnative_window_manager.so
 * @kit ArkUI
 * @since 15
 */
#ifndef OH_WINDOW_H
#define OH_WINDOW_H

#include <stdint.h>
#include "oh_window_comm.h"
#include "pointer_event_ndk.h"
#include "wm_common.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Set whether to show status bar.
 *
 * @param windowId WindowId when window is created.
 * @param enabled If true, the status bar is displayed. If false, the status bar is hidden.
 * @param enableAnimation If true, the status bar is displayed and hidden with animation.
 *                        If false, the status bar is displayed and hidden with no animation.
 * @return Return the result code.
 *         {@link OK} the function call is successful.
 *         {@link WINDOW_MANAGER_ERRORCODE_INVALID_PARAM} parameter error.
 *         {@link WINDOW_MANAGER_ERRORCODE_DEVICE_NOT_SUPPORTED} capability not supported.
 *         {@link WINDOW_MANAGER_ERRORCODE_STATE_ABNORMAL} this window state is abnormal.
 *         {@link WINDOW_MANAGER_ERRORCODE_SYSTEM_ABNORMAL} the window manager service works abnormal.
 * @since 15
 */
int32_t OH_WindowManager_SetWindowStatusBarEnabled(int32_t windowId, bool enabled, bool enableAnimation);

/**
 * @brief Set status bar content color.
 *
 * @param windowId WindowId when window is created.
 * @param color The color value to set, the format is ARGB.
 * @return Return the result code.
 *         {@link OK} the function call is successful.
 *         {@link WINDOW_MANAGER_ERRORCODE_INVALID_PARAM} parameter error.
 *         {@link WINDOW_MANAGER_ERRORCODE_DEVICE_NOT_SUPPORTED} capability not supported.
 *         {@link WINDOW_MANAGER_ERRORCODE_STATE_ABNORMAL} this window state is abnormal.
 *         {@link WINDOW_MANAGER_ERRORCODE_SYSTEM_ABNORMAL} the window manager service works abnormal.
 * @since 15
 */
int32_t OH_WindowManager_SetWindowStatusBarColor(int32_t windowId, int32_t color);

/**
 * @brief Set whether to show navigation bar.
 *
 * @param windowId WindowId when window is created.
 * @param enabled If true, the navigation bar is displayed. If false, the navigation bar is hidden.
 * @param enableAnimation If true, the navigation bar is displayed and hidden with animation.
 *                        If false, the navigation bar is displayed and hidden with no animation.
 * @return Return the result code.
 *         {@link OK} the function call is successful.
 *         {@link WINDOW_MANAGER_ERRORCODE_INVALID_PARAM} parameter error.
 *         {@link WINDOW_MANAGER_ERRORCODE_DEVICE_NOT_SUPPORTED} capability not supported.
 *         {@link WINDOW_MANAGER_ERRORCODE_STATE_ABNORMAL} this window state is abnormal.
 *         {@link WINDOW_MANAGER_ERRORCODE_SYSTEM_ABNORMAL} the window manager service works abnormal.
 * @since 15
 */
int32_t OH_WindowManager_SetWindowNavigationBarEnabled(int32_t windowId, bool enabled, bool enableAnimation);

/**
 * @brief Get the avoid area
 *
 * @param windowId WindowId when window is created.
 * @param type Type of the avoid area.
 * @param avoidArea Indicates the pointer to a WindowManager_AvoidArea object.
 * @return Returns the status code of the execution.
 * @return Return the result code.
 *         {@link OK} the function call is successful, return avoid area ptr in avoidArea.
 *         {@link WINDOW_MANAGER_ERRORCODE_INVALID_PARAM} parameter error.
 *         {@link WINDOW_MANAGER_ERRORCODE_STATE_ABNORMAL} this window state is abnormal.
 *         {@link WINDOW_MANAGER_ERRORCODE_SYSTEM_ABNORMAL} the window manager service works abnormal.
 * @since 15
 */
int32_t OH_WindowManager_GetWindowAvoidArea(
    int32_t windowId, WindowManager_AvoidAreaType type, WindowManager_AvoidArea* avoidArea);

/**
 * @brief Checks whether the window is displayed.
 *
 * @param windowId WindowId when window is created.
 * @param isShow Whether the window is displayed. The value true means that the window is displayed,
          and false means the opposite.
 * @return Returns the result code.
 *         {@link OK} the function call is successful.
 *         {@link WINDOW_MANAGER_ERRORCODE_INVALID_PARAM} parameter error.
 *         {@link WINDOW_MANAGER_ERRORCODE_STATE_ABNORMAL} this window state is abnormal.
 * @since 15
 */
int32_t OH_WindowManager_IsWindowShown(int32_t windowId, bool* isShow);

/**
 * @brief Show window.
 *
 * @param windowId WindowId when window is created.
 * @return Returns the result code.
 *         {@link OK} the function call is successful.
 *         {@link WINDOW_MANAGER_ERRORCODE_INVALID_PARAM} parameter error.
 *         {@link WINDOW_MANAGER_ERRORCODE_STATE_ABNORMAL} this window state is abnormal.
 *         {@link WINDOW_MANAGER_ERRORCODE_SYSTEM_ABNORMAL} the window manager service works abnormally.
 * @since 15
 */
int32_t OH_WindowManager_ShowWindow(int32_t windowId);

/**
 * @brief Sets the background color of window.
 *
 * @param windowId WindowId when window is created.
 * @param color the specified color.
 * @return Return the result code.
 *         {@link OK} the function call is successful.
 *         {@link WINDOW_MANAGER_ERRORCODE_INVALID_PARAM} parameter error.
 *         {@link WINDOW_MANAGER_ERRORCODE_STATE_ABNORMAL} this window state is abnormal.
 * @since 16
 */
int32_t OH_WindowManager_SetWindowBackgroundColor(int32_t windowId, const char* color);

/**
 * @brief Sets the brightness of window.
 *
 * @param windowId WindowId when window is created.
 * @param brightness the specified brightness value.
 * @return Return the result code.
 *         {@link OK} the function call is successful.
 *         {@link WINDOW_MANAGER_ERRORCODE_INVALID_PARAM} parameter error.
 *         {@link WINDOW_MANAGER_ERRORCODE_STATE_ABNORMAL} this window state is abnormal.
 *         {@link WINDOW_MANAGER_ERRORCODE_SYSTEM_ABNORMAL} the window manager service works abnormal.
 * @since 16
 */
int32_t OH_WindowManager_SetWindowBrightness(int32_t windowId, float brightness);

/**
 * @brief Sets whether keep screen on or not.
 *
 * @param windowId WindowId when window is created.
 * @param isKeepScreenOn keep screen on if true, or not if false.
 * @return Return the result code.
 *         {@link OK} the function call is successful.
 *         {@link WINDOW_MANAGER_ERRORCODE_INVALID_PARAM} parameter error.
 *         {@link WINDOW_MANAGER_ERRORCODE_STATE_ABNORMAL} this window state is abnormal.
 *         {@link WINDOW_MANAGER_ERRORCODE_SYSTEM_ABNORMAL} the window manager service works abnormal.
 * @since 16
 */
int32_t OH_WindowManager_SetWindowKeepScreenOn(int32_t windowId, bool isKeepScreenOn);

/**
 * @brief Sets whether is private mode or not.
 *
 * @permission {@code ohos.permission.PRIVACY_WINDOW}
 * @param windowId WindowId when window is created.
 * @param isPrivacy In private mode if true, or not if false.
 * @return Returns the result code.
 *         {@link OK} the function call is successful.
 *         {@link WINDOW_MANAGER_ERRORCODE_INVALID_PARAM} parameter error.
 *         {@link WINDOW_MANAGER_ERRORCODE_STATE_ABNORMAL} this window state is abnormal.
 *         {@link WINDOW_MANAGER_ERRORCODE_SYSTEM_ABNORMAL} the window manager service works abnormally.
 *         {@link WINDOW_MANAGER_ERRORCODE_NO_PERMISSION} permission verification failed.
 * @since 16
 */
int32_t OH_WindowManager_SetWindowPrivacyMode(int32_t windowId, bool isPrivacy);

/**
 * @brief Get the properties of current window.
 *
 * @param windowId WindowId when window is created.
 * @param windowProperties Properties of current window.
 * @return Return the result code.
 *         {@link OK} the function call is successful, return window properties ptr in windowProperties.
 *         {@link WINDOW_MANAGER_ERRORCODE_INVALID_PARAM} parameter error.
 *         {@link WINDOW_MANAGER_ERRORCODE_STATE_ABNORMAL} this window state is abnormal.
 *         {@link WINDOW_MANAGER_ERRORCODE_SYSTEM_ABNORMAL} the window manager service works abnormal.
 * @since 16
 */
int32_t OH_WindowManager_GetWindowProperties(
    int32_t windowId, WindowManager_WindowProperties* windowProperties);

/**
 * @brief Obtains snapshot of window.
 *
 * @param windowId windowId when window is created.
 * @param pixelMap snapshot of window.
 * @return Return the result code.
 *         {@link OK} the function call is successful, return pixel map ptr in pixelMap.
 *         {@link WINDOW_MANAGER_ERRORCODE_INVALID_PARAM} parameter error.
 *         {@link WINDOW_MANAGER_ERRORCODE_STATE_ABNORMAL} this window state is abnormal.
 *         {@link WINDOW_MANAGER_ERRORCODE_SYSTEM_ABNORMAL} the window manager service works abnormal.
 * @since 16
 */
int32_t OH_WindowManager_Snapshot(int32_t windowId, OH_PixelmapNative* pixelMap);

/**
 * @brief set window touchable status.
 *
 * @param windowId windowId when window is created.
 * @param touchable window touchable status.
 * @return Return the result code.
 *         {@link OK} the function call is successful.
 *         {@link WINDOW_MANAGER_ERRORCODE_INVALID_PARAM} parameter error.
 *         {@link WINDOW_MANAGER_ERRORCODE_STATE_ABNORMAL} this window state is abnormal.
 *         {@link WINDOW_MANAGER_ERRORCODE_SYSTEM_ABNORMAL} the window manager service works abnormal.
 * @since 16
 */
int32_t OH_WindowManager_SetWindowTouchable(int32_t windowId, bool touchable);

/**
 * @brief Set focusable property of window.
 *
 * @param windowId WindowId when window is created.
 * @param isFocusable Window can be focused or not.
 * @return Returns the result code.
 *         {@link OK} the function call is successful.
 *         {@link WINDOW_MANAGER_ERRORCODE_INVALID_PARAM} parameter error.
 *         {@link WINDOW_MANAGER_ERRORCODE_STATE_ABNORMAL} this window state is abnormal.
 *         {@link WINDOW_MANAGER_ERRORCODE_SYSTEM_ABNORMAL} the window manager service works abnormally.
 * @since 16
 */
int32_t OH_WindowManager_SetWindowFocusable(int32_t windowId, bool isFocusable);

/**
 * @brief Get Layout info of all windows on the selected display.
 *
 * @param display IdIndicate the id of display.
 * @param windowLayoutInfo Pointer to the layout information of the visible windows on the specified screen.
 * @param windowLayoutInfoSize Pointer to the size of the array of layout information of the visible windows on the
 *                             specified screen.
 * @return Returns the result code.
 *         {@link OK} the function call is successful, return window layout info list.
 *         {@link WINDOW_MANAGER_ERRORCODE_INVALID_PARAM} parameter error.
 *         {@link WINDOW_MANAGER_ERRORCODE_DEVICE_NOT_SUPPORTED} capability not supported.
 *         {@link WINDOW_MANAGER_ERRORCODE_SYSTEM_ABNORMAL} the window manager service works abnormally.
 * @since 17
 */
int32_t OH_WindowManager_GetAllWindowLayoutInfoList(
    int64_t displayId, WindowManager_Rect** windowLayoutInfoList, size_t* windowLayoutInfoSize);

/**
 * @brief Release the memory of window layout info list.
 *
 * @param windowLayoutInfo Pointer to the layout information of the visible windows on the specified screen.
 * @since 17
 */
void OH_WindowManager_ReleaseAllWindowLayoutInfoList(WindowManager_Rect* windowLayoutInfoList);

/**
 * @brief app can inject a touchEvent to target window without Focus and zOrder changed, just send to ArkUI.
 *
 * @param windowId windowId when window is created.
 * @param touchEvent multimodal touchEvent.
 * @param windowX The position of the event relative to the abscissa of the window.
 * @param windowY The position of the event relative to the ordinate of the window.
 * @return Returns the result code.
 *         {@link OK} the function call is successful.
 *         {@link WINDOW_MANAGER_ERRORCODE_INVALID_PARAM} parameter error.
 *         {@link WINDOW_MANAGER_ERRORCODE_STATE_ABNORMAL} this window state is abnormal.
 *         {@link WINDOW_MANAGER_ERRORCODE_SYSTEM_ABNORMAL} the window manager service works abnormally.
 * @since 20
 */
int32_t OH_WindowManager_InjectTouchEvent(
    int32_t windowId, Input_TouchEvent* touchEvent, int32_t windowX, int32_t windowY);

/**
 * @brief Get all main window info on device.
 *
 * @permission {@code ohos.permission.CUSTOM_SCREEN_CAPTURE}
 * @param infoList Indicates the pointer to a main window info list.
 * @param mainWindowInfoSize The size of main window info list.
 * @return Returns the status code of the execution.
 *         {@link WS_OK} the function call is successful.
 *         {@link WINDOW_MANAGER_ERRORCODE_NO_PERMISSION} permission verification failed.
 *         {@link WINDOW_MANAGER_ERRORCODE_DEVICE_NOT_SUPPORTED} capability not supported.
 *         {@link WINDOW_MANAGER_ERRORCODE_SYSTEM_ABNORMAL} the window manager service works abnormally.
 * @since 21
 */
int32_t OH_WindowManager_GetAllMainWindowInfo(
    WindowManager_MainWindowInfo** infoList, size_t* mainWindowInfoSize);
 
/**
 * @brief Release all main window info list.
 *
 * @param infoList Pointer to the main window info list.
 * @since 21
 */
void OH_WindowManager_ReleaseAllMainWindowInfo(WindowManager_MainWindowInfo* infoList);
 
/**
 * @brief Callback interface for getting main windows' snapshot.
 *
 * @param snapshotPixelMapList List of windows' snapshot
 * @param snapshotListSize Size of snapshotPixelMapList
 * @since 21
 */
typedef void (*OH_WindowManager_WindowSnapshotCallback)(const OH_PixelmapNative** snapshotPixelMapList,
    size_t snapshotListSize);
 
/**
 * @brief Get snapshot of  the specified windows.
 *
 * @permission {@code ohos.permission.CUSTOM_SCREEN_CAPTURE}
 * @param windowIdList Main window id list for getting snapshot.
 * @param windowIdListSize Size of main window id list.
 * @param config Configuration for getting snapshot.
 * @param callback Snapshot callback object.
 * @return Returns the status code of the execution.
 *         {@link WS_OK} the function call is successful.
 *         {@link WINDOW_MANAGER_ERRORCODE_NO_PERMISSION} permission verification failed.
 *         {@link WINDOW_MANAGER_ERRORCODE_DEVICE_NOT_SUPPORTED} capability not supported.
 *         {@link WINDOW_MANAGER_ERRORCODE_SYSTEM_ABNORMAL} the window manager service works abnormally.
 * @since 21
 */
int32_t OH_WindowManager_GetMainWindowSnapshot(int32_t* windowIdList, size_t windowIdListSize,
    WindowManager_WindowSnapshotConfig config, OH_WindowManager_WindowSnapshotCallback callback);
 
/**
 * @brief Release main window snapshot list.
 *
 * @param snapshotPixelMapList Indicates the pointer of a windows' snapshot list.
 * @since 21
 */
void OH_WindowManager_ReleaseMainWindowSnapshot(const OH_PixelmapNative* snapshotPixelMapList);

/**
 * @brief Lock the mouse cursor restricting it to a specified window area, and also control whether the cursor follows
 *        movement. Only supported by the focus window; the lock is automatically released when the window loses focus.
 *
 * @permission ohos.permission.LOCK_WINDOW_CURSOR
 * @param windowId WindowId when window is created.
 * @param isCursorFollowMovement Set mouse cursor lock mode.
 *        If true:the cursor follow the mouse movement.
 *        If false:the cursor does not follow the mouse.
 * @return Returns the status code of the execution.
 *         {@link WS_OK} the function call is successful.
 *         {@link WINDOW_MANAGER_ERRORCODE_NO_PERMISSION} permission verification failed.
 *         {@link WINDOW_MANAGER_ERRORCODE_DEVICE_NOT_SUPPORTED} capability not supported.
 *         {@link WINDOW_MANAGER_ERRORCODE_STATE_ABNORMAL} this window state is abnormal.
 *         {@link WINDOW_MANAGER_ERRORCODE_SYSTEM_ABNORMAL} the window manager service works abnormally.
 * @since 22
 */
int32_t OH_WindowManager_LockCursor(int32_t windowId, bool isCursorFollowMovement);

/**
 * @brief Clear the window mouse cursor status. Revert to mouse cursor free movement mode.
 *
 * @permission ohos.permission.LOCK_WINDOW_CURSOR
 * @param windowId WindowId when window is created.
 * @return Returns the status code of the execution.
 *         {@link WS_OK} the function call is successful.
 *         {@link WINDOW_MANAGER_ERRORCODE_NO_PERMISSION} permission verification failed.
 *         {@link WINDOW_MANAGER_ERRORCODE_DEVICE_NOT_SUPPORTED} capability not supported.
 *         {@link WINDOW_MANAGER_ERRORCODE_STATE_ABNORMAL} this window state is abnormal.
 *         {@link WINDOW_MANAGER_ERRORCODE_SYSTEM_ABNORMAL} the window manager service works abnormally.
 * @since 22
 */
int32_t OH_WindowManager_UnlockCursor(int32_t windowId);

/**
 * @brief Gets the system default display size scaling factor of the screen where the window is located.
 *
 * @param info Display size scaling factor information for the current window.
 * @param density System default display size scaling factor
 * @return Returns the status code of the execution.
 *         {@link WS_OK} the function call is successful.
 *         {@link WINDOW_MANAGER_ERRORCODE_INCORRECT_PARAM} Parameter error. Possible cause:
 *             1. Invalid parameter range.
 * @since 24
 */
int32_t OH_WindowManager_DensityInfo_GetDefaultDensity(
    const OH_WindowManager_DensityInfo* info, float* density);

/**
 * @brief Gets the system display size scaling factor of the screen where the window is located.
 *
 * @param info Display size scaling factor information for the current window.
 * @param density System display size scaling factor
 * @return Returns the status code of the execution.
 *         {@link WS_OK} the function call is successful.
 *         {@link WINDOW_MANAGER_ERRORCODE_INCORRECT_PARAM} Parameter error. Possible cause:
 *             1. Invalid parameter range.
 * @since 24
 */
int32_t OH_WindowManager_DensityInfo_GetSystemDensity(
    const OH_WindowManager_DensityInfo* info, float* density);

/**
 * @brief Gets the custom display size scaling factor of the window.
 *
 * @param info Display size scaling factor information for the current window.
 * @param density Custom display size scaling factor of the window. A return value of -1 indicates that no custom
 *     display size scaling factor has been set, or it has been reset.
 * @return Returns the status code of the execution.
 *         {@link WS_OK} the function call is successful.
 *         {@link WINDOW_MANAGER_ERRORCODE_INCORRECT_PARAM} Parameter error. Possible cause:
 *             1. Invalid parameter range.
 * @since 24
 */
int32_t OH_WindowManager_DensityInfo_GetCustomDensity(
    const OH_WindowManager_DensityInfo* info, float* density);

/**
 * @brief Get the system display size scaling factor, the system default display size scaling factor,
 *     and the custom display size scaling factor information of the screen where the current window is located.
 *
 * @param windowId WindowId when window is created.
 * @param info Display size scaling factor information for the current window.
 *     A return value of NULL means this interface is not supported on the current device.
 * @return Returns the status code of the execution.
 *         {@link WS_OK} the function call is successful.
 *         {@link WINDOW_MANAGER_ERRORCODE_STATE_ABNORMAL} this window state is abnormal. Possible cause:
 *             1. The window is not created or destroyed;
 *             2. This window state is abnormal.
 *         {@link WINDOW_MANAGER_ERRORCODE_INCORRECT_PARAM} Parameter error. Possible cause:
 *             1. Invalid parameter range.
 * @since 24
 */
int32_t OH_WindowManager_GetDensityInfoCopy(int32_t windowId,
    const OH_WindowManager_DensityInfo** info);

/**
 * @brief Listen for changes in the display size scaling factor information of the window. The callback function is
 *     triggered when any of the system display size scaling factor, system default display size scaling factor, or
 *     custom display size scaling factor of the screen where the window resides changes.
 *     To unlisten for changes in the display size scaling factor information of the window, call
 *     OH_WindowManager_UnregisterDensityInfoChangeCallback.
 *
 * @param windowId WindowId when window is created.
 * @param callback Callback used to return the result of density information.
 * @return Returns the status code of the execution.
 *         {@link WS_OK} the function call is successful.
 *         {@link WINDOW_MANAGER_ERRORCODE_STATE_ABNORMAL} this window state is abnormal. Possible cause:
 *             1. The window is not created or destroyed;
 *             2. This window state is abnormal.
 *         {@link WINDOW_MANAGER_ERRORCODE_INCORRECT_PARAM} Parameter error. Possible cause:
 *             1. Invalid parameter range.
 * @since 24
 */
int32_t OH_WindowManager_RegisterDensityInfoChangeCallback(
    int32_t windowId, OH_WindowManager_DensityInfoCallback callback);

/**
 * @brief Unlisten for changes in the display size scaling factor information of the window. The callback function is
 *     triggered when any of the system display size scaling factor, system default display size scaling factor, or
 *     custom display size scaling factor of the screen where the window resides changes.
 *     To listen for changes in the display size scaling factor information of the window, call
 *     OH_WindowManager_RegisterDensityInfoChangeCallback.
 *
 * @param windowId WindowId when window is created.
 * @param callback Callback used to return the result of density information.
 * @return Returns the status code of the execution.
 *         {@link WS_OK} the function call is successful.
 *         {@link WINDOW_MANAGER_ERRORCODE_STATE_ABNORMAL} this window state is abnormal. Possible cause:
 *             1. The window is not created or destroyed;
 *             2. This window state is abnormal.
 *         {@link WINDOW_MANAGER_ERRORCODE_INCORRECT_PARAM} Parameter error. Possible cause:
 *             1. Invalid parameter range.
 * @since 24
 */
int32_t OH_WindowManager_UnregisterDensityInfoChangeCallback(
    int32_t windowId, OH_WindowManager_DensityInfoCallback callback);

/**
 * @brief Releases the memory occupied by DensityInfo.
 *
 * @param info Display size scaling factor information for the current window.
 * @return Returns the status code of the execution.
 *         {@link WS_OK} the function call is successful.
 *         {@link WINDOW_MANAGER_ERRORCODE_INCORRECT_PARAM} Parameter error. Possible cause:
 *             1. Invalid parameter range.
 * @since 24
 */
int32_t OH_WindowManager_DensityInfo_Release(const OH_WindowManager_DensityInfo* info);

/**
 * @brief Check whether the current frame is the first frame.
 *
 * @param metrics Frame metrics data object.
 * @param isFirstDrawFrame This parameter is the return value of the function,
 *     indicating whether the current frame is the first frame.
 * @return Returns the status code of the execution.
 *         {@link WS_OK} the function call is successful.
 *         {@link WINDOW_MANAGER_ERRORCODE_INCORRECT_PARAM} Parameter error. Possible cause:
 *             1. Invalid parameter range.
 * @since 26.0.0
 */
int32_t OH_WindowManager_FrameMetrics_IsFirstDrawFrame(
    const OH_WindowManager_FrameMetrics* metrics, bool* isFirstDrawFrame);

/**
 * @brief Get the time taken to process external input events in one frame.
 *
 * @param metrics Frame metrics data object.
 * @param duration This parameter is the return value of the function,
 *     indicating the time taken to process external input events in one frame.
 * @return Returns the status code of the execution.
 *         {@link WS_OK} the function call is successful.
 *         {@link WINDOW_MANAGER_ERRORCODE_INCORRECT_PARAM} Parameter error. Possible cause:
 *             1. Invalid parameter range.
 * @since 26.0.0
 */
int32_t OH_WindowManager_FrameMetrics_GetInputHandlingDuration(
    const OH_WindowManager_FrameMetrics* metrics, uint64_t* duration);

/**
 * @brief Get the time taken for layout measurement in one frame.
 *
 * @param metrics Frame metrics data object.
 * @param duration This parameter is the return value of the function,
 *     indicating the time taken for layout measurement in one frame.
 * @return Returns the status code of the execution.
 *         {@link WS_OK} the function call is successful.
 *         {@link WINDOW_MANAGER_ERRORCODE_INCORRECT_PARAM} Parameter error. Possible cause:
 *             1. Invalid parameter range.
 * @since 26.0.0
 */
int32_t OH_WindowManager_FrameMetrics_GetLayoutMeasureDuration(
    const OH_WindowManager_FrameMetrics* metrics, uint64_t* duration);

/**
 * @brief Get the start timestamp of the current frame.
 *
 * @param metrics Frame metrics data object.
 * @param timestamp This parameter is the return value of the function,
 *     indicating the start timestamp of the current frame.
 * @return Returns the status code of the execution.
 *         {@link WS_OK} the function call is successful.
 *         {@link WINDOW_MANAGER_ERRORCODE_INCORRECT_PARAM} Parameter error. Possible cause:
 *             1. Invalid parameter range.
 * @since 26.0.0
 */
int32_t OH_WindowManager_FrameMetrics_GetVsyncTimestamp(
    const OH_WindowManager_FrameMetrics* metrics, uint64_t* timestamp);

/**
 * @brief Subscribe to the listening event for changes in the window frame rate metrics.
 *     This interface must be used after loadContent() or setUICContent() has taken effect.
 *     After the application registers the frame metrics change listener, the registered callback will only be
 *     triggered when the client UI content is redrawn (e.g., page switching, interaction with responsive
 *     components, setting background color and opacity, etc.).
 *
 * @param windowId WindowId when window is created.
 * @param callback Callback used to return the result of frame metrics.
 * @return Returns the status code of the execution.
 *         {@link WS_OK} the function call is successful.
 *         {@link WINDOW_MANAGER_ERRORCODE_STATE_ABNORMAL} this window state is abnormal. Possible cause:
 *             1. The window is not created or destroyed;
 *             2. This window state is abnormal.
 *         {@link WINDOW_MANAGER_ERRORCODE_INCORRECT_PARAM} Parameter error. Possible cause:
 *             1. Invalid parameter range.
 * @since 26.0.0
 */
int32_t OH_WindowManager_RegisterFrameMetricsMeasuredCallback(int32_t windowId,
    OH_WindowManager_FrameMetricsMeasuredCallback callback);

/**
 * @brief Unsubscribe from the listening event for changes in the window frame rate metrics.
 *     This interface must be used after loadContent() or setUIContent() has taken effect.
 *
 * @param windowId WindowId when window is created.
 * @param callback Callback used to return the result of frame metrics.
 * @return Returns the status code of the execution.
 *         {@link WS_OK} the function call is successful.
 *         {@link WINDOW_MANAGER_ERRORCODE_STATE_ABNORMAL} this window state is abnormal. Possible cause:
 *             1. The window is not created or destroyed;
 *             2. This window state is abnormal.
 *         {@link WINDOW_MANAGER_ERRORCODE_INCORRECT_PARAM} Parameter error. Possible cause:
 *             1. Invalid parameter range.
 * @since 26.0.0
 */
int32_t OH_WindowManager_UnregisterFrameMetricsMeasuredCallback(int32_t windowId,
    OH_WindowManager_FrameMetricsMeasuredCallback callback);

#ifdef __cplusplus
}
#endif
#endif  // OH_WINDOW_H
