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

#ifndef OH_NATIVE_DISPLAY_CAPTURE_H
#define OH_NATIVE_DISPLAY_CAPTURE_H

/**
 * @addtogroup OH_DisplayManager
 * @{
 *
 * @brief Defines the data structures for the C APIs of the display capture.
 *
 * @syscap SystemCapability.WindowManager.WindowManager.Core
 * @since 14
 * @version 1.0
 */

/**
 * @file oh_display_capture.h
 *
 * @brief Defines the data structures for the C APIs of the display capture.
 *
 * @kit ArkUI
 * @library libnative_display_manager.so.
 * @syscap SystemCapability.WindowManager.WindowManager.Core
 * @since 14
 * @version 1.0
 */

#include "image/pixelmap_native.h"
#include "oh_display_info.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Obtain screen capture.
 *
 * @param { displayId } this display to be captured.
 * @param { pixelMap } the pixelMap of the display by id.
 * @return { @link DISPLAY_MANAGER_OK } If the operation is successful.
 *         { @link DISPLAY_MANAGER_ERROR_NO_PERMISSION } If no permission.
 *         { @link DISPLAY_MANAGER_ERROR_INVALID_PARAM } If Parameter error.
 *         { @link DISPLAY_MANAGER_ERROR_DEVICE_NOT_SUPPORTED } If device not support.
 *         { @link DISPLAY_MANAGER_ERROR_SYSTEM_ABNORMAL } If display manager service works abnormally.
 * @syscap SystemCapability.Window.SessionManager.Core
 * @since 14
 */
NativeDisplayManager_ErrorCode OH_NativeDisplayManager_CreateScreenCapture(uint32_t displayId,
    OH_PixelmapNative **pixelMap);

#ifdef __cplusplus
}
#endif
/** @} */
#endif // OH_NATIVE_DISPLAY_CAPTURE_H
