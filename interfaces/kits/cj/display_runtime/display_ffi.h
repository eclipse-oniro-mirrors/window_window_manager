/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef OHOS_DISPLAY_FFI_H
#define OHOS_DISPLAY_FFI_H

#include "cj_common_ffi.h"
#include "display_utils.h"

extern "C" {
FFI_EXPORT RetStruct FfiOHOSGetDefaultDisplaySync();
FFI_EXPORT RetStruct FfiOHOSGetAllDisplays();
FFI_EXPORT RetStruct FfiOHOSHasPrivateWindow(uint32_t displayId);
FFI_EXPORT bool FfiOHOSIsFoldable();
FFI_EXPORT void FfiOHOSSetFoldDisplayMode(uint32_t mode);
FFI_EXPORT uint32_t FfiOHOSGetFoldStatus();
FFI_EXPORT uint32_t FfiOHOSGetFoldDisplayMode();
FFI_EXPORT RetStruct FfiOHOSGetCurrentFoldCreaseRegion();
FFI_EXPORT bool FfiOHOSIsCaptured();
FFI_EXPORT RetStruct FfiOHOSGetAllDisplayPhysicalResolution();
FFI_EXPORT int32_t FfiOHOSUnRegisterAllDisplayManagerCallback(const char* type);
FFI_EXPORT int32_t FfiOHOSRegisterDisplayManagerCallback(const char* type, int64_t callbackId);
FFI_EXPORT int32_t FfiOHOSUnRegisterDisplayManagerCallback(const char* type, int64_t callbackId);

FFI_EXPORT uint32_t FfiOHOSDisplayGetId(int64_t id);
FFI_EXPORT char* FfiOHOSGetDisplayName(int64_t id);
FFI_EXPORT bool FfiOHOSDisplayGetAlive(int64_t id);
FFI_EXPORT uint32_t FfiOHOSDisplayGetState(int64_t id);
FFI_EXPORT uint32_t FfiOHOSDisplayGetRefreshRate(int64_t id);
FFI_EXPORT uint32_t FfiOHOSDisplayGetRotation(int64_t id);
FFI_EXPORT uint32_t FfiOHOSDisplayGetOrientation(int64_t id);
FFI_EXPORT int32_t FfiOHOSDisplayGetWidth(int64_t id);
FFI_EXPORT int32_t FfiOHOSDisplayGetHeight(int64_t id);
FFI_EXPORT float FfiOHOSDisplayGetDensityDPI(int64_t id);
FFI_EXPORT float FfiOHOSDisplayGetVirtualPixelRatio(int64_t id);
FFI_EXPORT float FfiOHOSDisplayGetXDPI(int64_t id);
FFI_EXPORT float FfiOHOSDisplayGetYDPI(int64_t id);
FFI_EXPORT RetStruct FfiOHOSDisplayGetColorSpaces(int64_t id);
FFI_EXPORT RetStruct FfiOHOSDisplayGetHdrFormats(int64_t id);
FFI_EXPORT uint32_t FfiOHOSDisplayGetAvailableWidth(int64_t id);
FFI_EXPORT uint32_t FfiOHOSDisplayGetAvailableHeight(int64_t id);
FFI_EXPORT RetStruct FfiOHOSDisplayGetCutoutInfo(int64_t id);
FFI_EXPORT RetStruct FfiOHOSDisplayGetAvailableArea(int64_t id);
FFI_EXPORT int32_t FfiOHOSDisplayUnRegisterAllDisplayManagerCallback(const char* type, int64_t id);
FFI_EXPORT int32_t FfiOHOSDisplayRegisterDisplayManagerCallback(const char* type, int64_t id, int64_t callbackId);
FFI_EXPORT int32_t FfiOHOSDisplayUnRegisterDisplayManagerCallback(const char* type, int64_t id, int64_t callbackId);
}

#endif // DISPLAY_FFI_H
