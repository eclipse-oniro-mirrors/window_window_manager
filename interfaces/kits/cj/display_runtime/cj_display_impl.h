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

#ifndef OHOS_CJ_DISPLAY_IMPL
#define OHOS_CJ_DISPLAY_IMPL

#include "display_manager.h"
#include "display_utils.h"
#include "ffi_remote_data.h"

namespace OHOS {
namespace Rosen {
class DisplayImpl final : public OHOS::FFI::FFIData {
    DECL_TYPE(DisplayImpl, OHOS::FFI::FFIData)
public:
    explicit DisplayImpl(const sptr<Display>& display);
    ~DisplayImpl() override;
    static sptr<DisplayImpl> CreateDisplayImpl(sptr<Display>& display);
    static sptr<DisplayImpl> FindDisplayObject(uint64_t displayId);
    uint32_t GetInfoId();
    char* GetName();
    bool GetAlive();
    uint32_t GetState();
    uint32_t GetRefreshRate();
    uint32_t GetRotation();
    uint32_t GetOrientation();
    int32_t GetWidth();
    int32_t GetHeight();
    float GetDensityDPI();
    float GetVirtualPixelRatio();
    float GetXDPI();
    float GetYDPI();
    RetStruct GetColorSpaces();
    RetStruct GetHdrFormats();
    uint32_t GetAvailableWidth();
    uint32_t GetAvailableHeight();
    RetStruct GetCutoutInfo();
    RetStruct GetAvailableArea();
    int32_t OnUnRegisterAllDisplayManagerCallback(const std::string& type);
    DMError UnRegisterAllDisplayListenerWithType(const std::string& type);
    int32_t OnRegisterDisplayManagerCallback(const std::string& type, int64_t funcId);
    DMError RegisterDisplayListenerWithType(const std::string& type, int64_t funcId);
    int32_t OnUnRegisterDisplayManagerCallback(const std::string& type, int64_t funcId);
    DMError UnRegisterDisplayListenerWithType(const std::string& type, int64_t funcId);

private:
    sptr<Display> display_ = nullptr;
};

enum class DisplayStateMode : uint32_t {
    STATE_UNKNOWN = 0,
    STATE_OFF,
    STATE_ON,
    STATE_DOZE,
    STATE_DOZE_SUSPEND,
    STATE_VR,
    STATE_ON_SUSPEND
};
} // namespace Rosen
} // namespace OHOS

#endif // DISPLAY_DISPLAY_IMPL
