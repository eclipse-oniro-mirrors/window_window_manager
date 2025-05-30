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

#ifndef OH_NATIVE_DISPLAY_MANAGER_H
#define OH_NATIVE_DISPLAY_MANAGER_H

/**
 * @addtogroup OH_DisplayManager
 * @{
 *
 * @brief Defines the data structures for the C APIs of the display module.
 *
 * @syscap SystemCapability.WindowManager.WindowManager.Core
 * @since 12
 * @version 1.0
 */

/**
 * @file oh_display_manager.h
 *
 * @brief Defines the data structures for the C APIs of the display module.
 *
 * @kit ArkUI
 * @library libnative_display_manager.so.
 * @syscap SystemCapability.WindowManager.WindowManager.Core
 * @since 12
 * @version 1.0
 */

#include "oh_display_info.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Obtain the default display Id.
 *
 * @param { *displayId } Indicates the pointer to an <b>uint64_t</b> object.
 * @return { @link DISPLAY_MANAGER_OK } If the operation is successful
 *         { @link DISPLAY_MANAGER_ERROR_INVALID_PARAM } If Parameter error.
 *         { @link DISPLAY_MANAGER_ERROR_SYSTEM_ABNORMAL } If display manager service works abnormally.
 * @since 12
 */
NativeDisplayManager_ErrorCode OH_NativeDisplayManager_GetDefaultDisplayId(uint64_t *displayId);

/**
 * @brief Obtain the default display width.
 *
 * @param { *displayWidth } Indicates the pointer to an <b>int32_t</b> object.
 * @return { @link DISPLAY_MANAGER_OK } If the operation is successful
 *         { @link DISPLAY_MANAGER_ERROR_INVALID_PARAM } If Parameter error.
 *         { @link DISPLAY_MANAGER_ERROR_SYSTEM_ABNORMAL } If display manager service works abnormally.
 * @since 12
 */
NativeDisplayManager_ErrorCode OH_NativeDisplayManager_GetDefaultDisplayWidth(int32_t *displayWidth);

/**
 * @brief Obtain the default display height.
 *
 * @param { *displayHeight } Indicates the pointer to an <b>int32_t</b> object.
 * @return { @link DISPLAY_MANAGER_OK } If the operation is successful
 *         { @link DISPLAY_MANAGER_ERROR_INVALID_PARAM } If Parameter error.
 *         { @link DISPLAY_MANAGER_ERROR_SYSTEM_ABNORMAL } If display manager service works abnormally.
 * @since 12
 */
NativeDisplayManager_ErrorCode OH_NativeDisplayManager_GetDefaultDisplayHeight(int32_t *displayHeight);

/**
 * @brief Obtain the default display rotation.
 *
 * @param { *displayRotation } Indicates the pointer to an <b>NativeDisplayManager_Rotation</b> object.
 * @return { @link DISPLAY_MANAGER_OK } If the operation is successful
 *         { @link DISPLAY_MANAGER_ERROR_INVALID_PARAM } If Parameter error.
 *         { @link DISPLAY_MANAGER_ERROR_SYSTEM_ABNORMAL } If display manager service works abnormally.
 * @since 12
 */
NativeDisplayManager_ErrorCode OH_NativeDisplayManager_GetDefaultDisplayRotation(
    NativeDisplayManager_Rotation *displayRotation);

/**
 * @brief Obtain the default display orientation.
 *
 * @param { *displayOrientation } Indicates the pointer to an <b>NativeDisplayManager_Orientation</b> object.
 * @return { @link DISPLAY_MANAGER_OK } If the operation is successful
 *         { @link DISPLAY_MANAGER_ERROR_INVALID_PARAM } If Parameter error.
 *         { @link DISPLAY_MANAGER_ERROR_SYSTEM_ABNORMAL } If display manager service works abnormally.
 * @since 12
 */
NativeDisplayManager_ErrorCode OH_NativeDisplayManager_GetDefaultDisplayOrientation(
    NativeDisplayManager_Orientation *displayOrientation);

/**
 * @brief Obtain the default display virtualPixels.
 *
 * @param { *virtualPixels } Indicates the pointer to an <b>float</b> object.
 * @return { @link DISPLAY_MANAGER_OK } If the operation is successful
 *         { @link DISPLAY_MANAGER_ERROR_INVALID_PARAM } If Parameter error.
 *         { @link DISPLAY_MANAGER_ERROR_SYSTEM_ABNORMAL } If display manager service works abnormally.
 * @since 12
 */
NativeDisplayManager_ErrorCode OH_NativeDisplayManager_GetDefaultDisplayVirtualPixelRatio(float *virtualPixels);

/**
 * @brief Obtain the default display refreshRate.
 *
 * @param { *refreshRate } Indicates the pointer to an <b>uint32_t</b> object.
 * @return { @link DISPLAY_MANAGER_OK } If the operation is successful
 *         { @link DISPLAY_MANAGER_ERROR_INVALID_PARAM } If Parameter error.
 *         { @link DISPLAY_MANAGER_ERROR_SYSTEM_ABNORMAL } If display manager service works abnormally.
 * @since 12
 */
NativeDisplayManager_ErrorCode OH_NativeDisplayManager_GetDefaultDisplayRefreshRate(uint32_t *refreshRate);

/**
 * @brief Obtain the default display densityDpi.
 *
 * @param { *densityDpi } Indicates the pointer to an <b>int32_t</b> object.
 * @return { @link DISPLAY_MANAGER_OK } If the operation is successful
 *         { @link DISPLAY_MANAGER_ERROR_INVALID_PARAM } If Parameter error.
 *         { @link DISPLAY_MANAGER_ERROR_SYSTEM_ABNORMAL } If display manager service works abnormally.
 * @since 12
 */
NativeDisplayManager_ErrorCode OH_NativeDisplayManager_GetDefaultDisplayDensityDpi(int32_t *densityDpi);

/**
 * @brief Obtain the default display densityPixels.
 *
 * @param { *densityPixels } Indicates the pointer to an <b>float</b> object.
 * @return { @link DISPLAY_MANAGER_OK } If the operation is successful
 *         { @link DISPLAY_MANAGER_ERROR_INVALID_PARAM } If Parameter error.
 *         { @link DISPLAY_MANAGER_ERROR_SYSTEM_ABNORMAL } If display manager service works abnormally.
 * @since 12
 */
NativeDisplayManager_ErrorCode OH_NativeDisplayManager_GetDefaultDisplayDensityPixels(float *densityPixels);

/**
 * @brief Obtain the default display scaledDensity.
 *
 * @param { *scaledDensity } Indicates the pointer to an <b>float</b> object.
 * @return { @link DISPLAY_MANAGER_OK } If the operation is successful
 *         { @link DISPLAY_MANAGER_ERROR_INVALID_PARAM } If Parameter error.
 *         { @link DISPLAY_MANAGER_ERROR_SYSTEM_ABNORMAL } If display manager service works abnormally.
 * @since 12
 */
NativeDisplayManager_ErrorCode OH_NativeDisplayManager_GetDefaultDisplayScaledDensity(float *scaledDensity);

/**
 * @brief Obtain the default display xDpi.
 *
 * @param { *xDpi } Indicates the pointer to an <b>float</b> object.
 * @return { @link DISPLAY_MANAGER_OK } If the operation is successful
 *         { @link DISPLAY_MANAGER_ERROR_INVALID_PARAM } If Parameter error.
 *         { @link DISPLAY_MANAGER_ERROR_SYSTEM_ABNORMAL } If display manager service works abnormally.
 * @since 12
 */
NativeDisplayManager_ErrorCode OH_NativeDisplayManager_GetDefaultDisplayDensityXdpi(float *xDpi);

/**
 * @brief Obtain the default display yDpi.
 *
 * @param { *yDpi } Indicates the pointer to an <b>float</b> object.
 * @return { @link DISPLAY_MANAGER_OK } If the operation is successful
 *         { @link DISPLAY_MANAGER_ERROR_INVALID_PARAM } If Parameter error.
 *         { @link DISPLAY_MANAGER_ERROR_SYSTEM_ABNORMAL } If display manager service works abnormally.
 * @since 12
 */
NativeDisplayManager_ErrorCode OH_NativeDisplayManager_GetDefaultDisplayDensityYdpi(float *yDpi);

/**
 * @brief Create the cutout info of the device.
 *
 * @param { **cutoutInfo } Indicates the pointer to an <b>NativeDisplayManager_CutoutInfo</b> object.
 * @return { @link DISPLAY_MANAGER_OK } If the operation is successful
 *         { @link DISPLAY_MANAGER_ERROR_INVALID_PARAM } If Parameter error.
 *         { @link DISPLAY_MANAGER_ERROR_SYSTEM_ABNORMAL } If display manager service works abnormally.
 * @since 12
 */
NativeDisplayManager_ErrorCode OH_NativeDisplayManager_CreateDefaultDisplayCutoutInfo(
    NativeDisplayManager_CutoutInfo **cutoutInfo);

/**
 * @brief Destroy an <b>NativeDisplayManager_CutoutInfo</b> object and reclaims the memory occupied by the object.
 *
 * @param { **cutoutInfo } Indicates the pointer to an <b>NativeDisplayManager_CutoutInfo</b> object.
 * @return { @link DISPLAY_MANAGER_OK } If the operation is successful
 *         { @link DISPLAY_MANAGER_ERROR_INVALID_PARAM } If Parameter error.
 * @since 12
 */
NativeDisplayManager_ErrorCode OH_NativeDisplayManager_DestroyDefaultDisplayCutoutInfo(
    NativeDisplayManager_CutoutInfo *cutoutInfo);

/**
 * @brief Check whether the device is foldable.
 *
 * @return { bool } true means the device is foldable.
 * @since 12
 */
bool OH_NativeDisplayManager_IsFoldable();

/**
 * @brief Get the display mode of the foldable device.
 *
 * @param { *displayMode } Indicates the pointer to an <b>NativeDisplayManager_FoldDisplayMode</b> object.
 * @return { @link DISPLAY_MANAGER_OK } If the operation is successful
 *         { @link DISPLAY_MANAGER_ERROR_INVALID_PARAM } If Parameter error.
 *         { @link DISPLAY_MANAGER_ERROR_DEVICE_NOT_SUPPORTED } device not support.
 * @since 12
 */
NativeDisplayManager_ErrorCode OH_NativeDisplayManager_GetFoldDisplayMode(
    NativeDisplayManager_FoldDisplayMode *displayMode);

/**
 * @brief the callback function type when display change.
 *
 * @param { *displayId } change display id.
 * @since 12
 */
typedef void (*OH_NativeDisplayManager_DisplayChangeCallback)(uint64_t displayId);

/**
 * @brief Register the callback for display change listener.
 *
 * @param { displayChangeCallback } display change callback.
 * @param { *listenerIndex } Indicates the pointer to an <b>uint32_t</b> object. used in unregister call.
 * @return { @link DISPLAY_MANAGER_OK } If the operation is successful
 *         { @link DISPLAY_MANAGER_ERROR_INVALID_PARAM } If Parameter error.
 *         { @link DISPLAY_MANAGER_ERROR_SYSTEM_ABNORMAL } If display manager service works abnormally.
 * @since 12
 */
NativeDisplayManager_ErrorCode OH_NativeDisplayManager_RegisterDisplayChangeListener(
    OH_NativeDisplayManager_DisplayChangeCallback displayChangeCallback, uint32_t *listenerIndex);

/**
 * @brief Unregister the callback for display changes listener.
 *
 * @param { listenerIndex } display changed listener index.
 * @return { @link DISPLAY_MANAGER_OK } If the operation is successful
 *         { @link DISPLAY_MANAGER_ERROR_INVALID_PARAM } If Parameter error.
 *         { @link DISPLAY_MANAGER_ERROR_SYSTEM_ABNORMAL } If display manager service works abnormally.
 * @since 12
 */
NativeDisplayManager_ErrorCode OH_NativeDisplayManager_UnregisterDisplayChangeListener(uint32_t listenerIndex);

/**
 * @brief the callback function type when display fold change.
 *
 * @param { displayMode } current fold display mode.
 * @since 12
 */
typedef void (*OH_NativeDisplayManager_FoldDisplayModeChangeCallback)(
    NativeDisplayManager_FoldDisplayMode displayMode);

/**
 * @brief Register the callback for display mode change listener.
 *
 * @param { displayModeChangeCallback } display mode change callback.
 * @param { *listenerIndex } Indicates the pointer to an <b>uint32_t</b> object. used in unregister call.
 * @return { @link DISPLAY_MANAGER_OK } If the operation is successful
 *         { @link DISPLAY_MANAGER_ERROR_INVALID_PARAM } If Parameter error.
 *         { @link DISPLAY_MANAGER_ERROR_DEVICE_NOT_SUPPORTED } device not support.
 *         { @link DISPLAY_MANAGER_ERROR_SYSTEM_ABNORMAL } If display manager service works abnormally.
 * @since 12
 */
NativeDisplayManager_ErrorCode OH_NativeDisplayManager_RegisterFoldDisplayModeChangeListener(
    OH_NativeDisplayManager_FoldDisplayModeChangeCallback displayModeChangeCallback, uint32_t *listenerIndex);

/**
 * @brief Unregister the callback for display mode change listener.
 *
 * @param { listenerIndex } display mode change listener index.
 * @return { @link DISPLAY_MANAGER_OK } If the operation is successful
 *         { @link DISPLAY_MANAGER_ERROR_INVALID_PARAM } If Parameter error.
 *         { @link DISPLAY_MANAGER_ERROR_DEVICE_NOT_SUPPORTED } device not support.
 *         { @link DISPLAY_MANAGER_ERROR_SYSTEM_ABNORMAL } If display manager service works abnormally.
 * @since 12
 */
NativeDisplayManager_ErrorCode OH_NativeDisplayManager_UnregisterFoldDisplayModeChangeListener(uint32_t listenerIndex);

/**
 * @brief Create all displays.
 *
 * @param allDisplays Output parameter for all displays information.
 * @return { @link DISPLAY_MANAGER_OK } If the operation is successful.
 *         { @link DISPLAY_MANAGER_ERROR_INVALID_PARAM } If Parameter error.
 *         { @link DISPLAY_MANAGER_ERROR_SYSTEM_ABNORMAL } If display manager service works abnormally.
 * @since 14
 */
NativeDisplayManager_ErrorCode OH_NativeDisplayManager_CreateAllDisplays(
    NativeDisplayManager_DisplaysInfo **allDisplays);

/**
 * @brief Destroy all displays.
 *
 * @param allDisplays all displays to be free.
 * @since 14
 */
void OH_NativeDisplayManager_DestroyAllDisplays(NativeDisplayManager_DisplaysInfo *allDisplays);

/**
 * @brief Create display information by display id.
 *
 * @param displayId The display id.
 * @param displayInfo The pointer to the display information.
 * @return { @link DISPLAY_MANAGER_OK } If the operation is successful.
 *         { @link DISPLAY_MANAGER_ERROR_INVALID_PARAM } If Parameter error.
 *         { @link DISPLAY_MANAGER_ERROR_SYSTEM_ABNORMAL } If display manager service works abnormally.
 * @since 14
 */
NativeDisplayManager_ErrorCode OH_NativeDisplayManager_CreateDisplayById(uint32_t displayId,
    NativeDisplayManager_DisplayInfo **displayInfo);

/**
 * @brief Destroy the display information.
 *
 * @param displayInfo the target display to be free.
 * @since 14
 */
void OH_NativeDisplayManager_DestroyDisplay(NativeDisplayManager_DisplayInfo *displayInfo);

/**
 * @brief Create a primary display.
 *
 * @param displayInfo The information of the created display.
 * @return { @link DISPLAY_MANAGER_OK } If the operation is successful.
 *         { @link DISPLAY_MANAGER_ERROR_INVALID_PARAM } If Parameter error.
 *         { @link DISPLAY_MANAGER_ERROR_SYSTEM_ABNORMAL } If display manager service works abnormally.
 * @since 14
 */
NativeDisplayManager_ErrorCode OH_NativeDisplayManager_CreatePrimaryDisplay(
    NativeDisplayManager_DisplayInfo **displayInfo);

/**
 * @brief callback function type when available area change.
 *
 * @param { displayId } changed display id.
 * @since 20
 */
typedef void (*OH_NativeDisplayManager_AvailableAreaChangeCallback)(uint64_t displayId);

/**
 * @brief Register the callback for display mode change listener.
 *
 * @param { availableAreaChangeCallback } Available area change callback.
 * @param { *listenerIndex } Indicates the pointer to an <b>uint32_t</b> object. used in register call.
 * @return { @link DISPLAY_MANAGER_OK } If the operation is successful.
 *         { @link DISPLAY_MANAGER_ERROR_ILLEGAL_PARAM } If Parameter illegal.
 *         { @link DISPLAY_MANAGER_ERROR_DEVICE_NOT_SUPPORT } device not support.
 *         { @link DISPLAY_MANAGER_ERROR_SYSTEM_ABNORMAL } If display manager service works abnormally.
 * @since 20
 */
NativeDisplayManager_ErrorCode OH_NativeDisplayManager_RegisterAvailableAreaChangeListener(
    OH_NativeDisplayManager_AvailableAreaChangeCallback availableAreaChangeCallback, uint32_t* listenerIndex);

/**
 * @brief Unregister the callback for display mode change listener.
 *
 * @param { *listenerIndex } Indicates the pointer to an <b>uint32_t</b> object. used in unregister call.
 * @return { @link DISPLAY_MANAGER_OK } If the operation is successful.
 *         { @link DISPLAY_MANAGER_ERROR_ILLEGAL_PARAM } If Parameter illegal.
 *         { @link DISPLAY_MANAGER_ERROR_DEVICE_NOT_SUPPORT } device not support.
 *         { @link DISPLAY_MANAGER_ERROR_SYSTEM_ABNORMAL } If display manager service works abnormally.
 * @since 20
 */
NativeDisplayManager_ErrorCode OH_NativeDisplayManager_UnregisterAvailableAreaChangeListener(uint32_t listenerIndex);

/**
 * @brief create available area.
 *
 * @param { displayid } displayid.
 * @param { **availableArea } Indicates the pointer to an <b>NativeDisplayManager_Rect</b> object.
 * @return { @link DISPLAY_MANAGER_OK } If the operation is successful.
 *         { @link DISPLAY_MANAGER_ERROR_ILLEGAL_PARAM } If Parameter illegal.
  *         { @link DISPLAY_MANAGER_ERROR_DEVICE_NOT_SUPPORT } device not support.
 *         { @link DISPLAY_MANAGER_ERROR_SYSTEM_ABNORMAL } If display manager service works abnormally.
 * @since 20
 */
NativeDisplayManager_ErrorCode OH_NativeDisplayManager_CreateAvailableArea(
    uint64_t displayId, NativeDisplayManager_Rect** availableArea);

/**
 * @brief destroy available area.
 *
 * @param { displayid } displayid.
 * @param { *availableArea } Indicates the pointer to an <b>NativeDisplayManager_Rect</b> object.
 * @return { @link DISPLAY_MANAGER_OK } If the operation is successful.
 *         { @link DISPLAY_MANAGER_ERROR_ILLEGAL_PARAM } If Parameter illegal.
 *         { @link DISPLAY_MANAGER_ERROR_DEVICE_NOT_SUPPORT } device not support.
 *         { @link DISPLAY_MANAGER_ERROR_SYSTEM_ABNORMAL } If display manager service works abnormally.
 * @since 20
 */
NativeDisplayManager_ErrorCode OH_NativeDisplayManager_DestroyAvailableArea(NativeDisplayManager_Rect* availableArea);

/**
 * @brief callback function type when display add.
 *
 * @param { displayId } change display id.
 * @since 20
 */
typedef void (*OH_NativeDisplayManager_DisplayAddCallback)(uint64_t displayId);

/**
 * @brief Register the callback for display add listener.
 *
 * @param { displayAddCallback } display add callback.
 * @param { *listenerIndex } Indicates the pointer to an <b>uint32_t</b> object.
 * @return { @link DISPLAY_MANAGER_OK } If the operation is successful.
 *         { @link DISPLAY_MANAGER_ERROR_ILLEGAL_PARAM } If Parameter illegal.
 *         { @link DISPLAY_MANAGER_ERROR_DEVICE_NOT_SUPPORT } device not support.
 *         { @link DISPLAY_MANAGER_ERROR_SYSTEM_ABNORMAL } If display manager service works abnormally.
 * @since 20
 */
NativeDisplayManager_ErrorCode OH_NativeDisplayManager_RegisterDisplayAddListener(
    OH_NativeDisplayManager_DisplayAddCallback displayAddCallback, uint32_t* listenerIndex);

/**
 * @brief Unregister the callback for display add listener.
 *
 * @param { *listenerIndex } Indicates the pointer to an <b>uint32_t</b> object. used in unregister call.
 * @return { @link DISPLAY_MANAGER_OK } If the operation is successful.
 *         { @link DISPLAY_MANAGER_ERROR_ILLEGAL_PARAM } If Parameter illegal.
 *         { @link DISPLAY_MANAGER_ERROR_DEVICE_NOT_SUPPORT } device not support.
 *         { @link DISPLAY_MANAGER_ERROR_SYSTEM_ABNORMAL } If display manager service works abnormally.
 * @since 20
 */
NativeDisplayManager_ErrorCode OH_NativeDisplayManager_UnregisterDisplayAddListener(uint32_t listenerIndex);

/**
 * @brief callback function type when display remove.
 *
 * @param { displayId } changed display id.
 * @since 20
 */
typedef void (*OH_NativeDisplayManager_DisplayRemoveCallback)(uint64_t displayId);

/**
 * @brief Register the callback for display remove listener.
 *
 * @param { displayremoveCallback } display remove callback.
 * @param { *listenerIndex } Indicates the pointer to an <b>uint32_t</b> object.
 * @return { @link DISPLAY_MANAGER_OK } If the operation is successful.
 *         { @link DISPLAY_MANAGER_ERROR_ILLEGAL_PARAM } If Parameter illegal.
 *         { @link DISPLAY_MANAGER_ERROR_DEVICE_NOT_SUPPORT } device not support.
 *         { @link DISPLAY_MANAGER_ERROR_SYSTEM_ABNORMAL } If display manager service works abnormally.
 * @since 20
 */
NativeDisplayManager_ErrorCode OH_NativeDisplayManager_RegisterDisplayRemoveListener(
    OH_NativeDisplayManager_DisplayRemoveCallback displayRemoveCallback, uint32_t* listenerIndex);

/**
 * @brief Unregister the callback for display remove listener.
 *
 * @param { *listenerIndex } Indicates the pointer to an <b>uint32_t</b> object. used in unregister call.
 * @return { @link DISPLAY_MANAGER_OK } If the operation is successful.
 *         { @link DISPLAY_MANAGER_ERROR_ILLEGAL_PARAM } If Parameter illegal.
 *         { @link DISPLAY_MANAGER_ERROR_DEVICE_NOT_SUPPORT } device not support.
 *         { @link DISPLAY_MANAGER_ERROR_SYSTEM_ABNORMAL } If display manager service works abnormally.
 * @since 20
 */
NativeDisplayManager_ErrorCode OH_NativeDisplayManager_UnregisterDisplayRemoveListener(uint32_t listenerIndex);

/**
 * @brief get the source mode of the device.
 *
 * @param { displayid } displayid.
 * @param { *sourceMode } Indicates the pointer to an <b>NativeDisplayManager_SourceMode</b> object.
 * @return { @link DISPLAY_MANAGER_OK } If the operation is successful.
 *         { @link DISPLAY_MANAGER_ERROR_ILLEGAL_PARAM } If Parameter illegal.
 *         { @link DISPLAY_MANAGER_ERROR_DEVICE_NOT_SUPPORT } device not support.
 *         { @link DISPLAY_MANAGER_ERROR_SYSTEM_ABNORMAL } If display manager service works abnormally.
 * @since 20
 */
NativeDisplayManager_ErrorCode OH_NativeDisplayManager_GetDisplaySourceMode(
    uint64_t displayId, NativeDisplayManager_SourceMode* sourceMode);

/**
 * @brief get the position  of the device.
 *
 * @param { displayid } displayid.
 * @param { x } The x-coordinate of the screen's top-left corner relative to the primary screen's origin (in px).
 * @param { y } The y-coordinate of the screen's top-left corner relative to the primary screen's origin (in py).
 * @return { @link DISPLAY_MANAGER_OK } If the operation is successful.
 *         { @link DISPLAY_MANAGER_ERROR_ILLEGAL_PARAM } If Parameter illegal.
 *         { @link DISPLAY_MANAGER_ERROR_DEVICE_NOT_SUPPORT } device not support.
 *         { @link DISPLAY_MANAGER_ERROR_SYSTEM_ABNORMAL } If display manager service works abnormally.
 * @since 20
 */
NativeDisplayManager_ErrorCode OH_NativeDisplayManager_GetDisplayPosition(uint64_t displayId, int32_t* x, int32_t* y);

#ifdef __cplusplus
}
#endif
/** @} */
#endif // OH_NATIVE_DISPLAY_MANAGER_H
