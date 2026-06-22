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

#ifndef RS_SCREEN_CAPABILITY
#define RS_SCREEN_CAPABILITY

#include <cstdint>
#include <parcel.h>
#include <refbase.h>
#include <string>
#include <vector>

#include "screen_manager/screen_types.h"

namespace OHOS {
namespace Rosen {
class RSScreenProps : public Parcelable {
public:
    RSScreenProps() = default;
    RSScreenProps(std::string propName, uint32_t propId, uint64_t value)
        : propName_(propName),
          propId_(propId),
          value_(value)
    {
    }
    ~RSScreenProps() = default;
    [[nodiscard]] static RSScreenProps* Unmarshalling(Parcel& parcel)
    {
        return nullptr;
    }
    bool Marshalling(Parcel& parcel) const override
    {
        return true;
    }

    void SetPropertyName(const std::string& propName)
    {
        propName_ = propName;
    }

    void SetPropId(uint32_t propId)
    {
        propId_ = propId;
    }

    void SetValue(uint64_t value)
    {
        value_ = value;
    }

    const std::string& GetPropertyName() const
    {
        return propName_;
    }

    uint32_t GetPropId() const
    {
        return propId_;
    }

    uint64_t GetValue() const
    {
        return value_;
    }

private:
    std::string propName_;
    uint32_t propId_;
    uint64_t value_;
};

class RSScreenCapability : public Parcelable {
public:
    RSScreenCapability() = default;
    ~RSScreenCapability() = default;
    [[nodiscard]] static RSScreenCapability* Unmarshalling(Parcel& parcel)
    {
        return nullptr;
    }
    bool Marshalling(Parcel& parcel) const override
    {
        return true;
    }

    void SetName(const std::string& name)
    {
        name_ = name;
    }

    void SetType(ScreenInterfaceType type)
    {
        type_ = type;
    }

    void SetPhyWidth(uint32_t phyWidth)
    {
        phyWidth_ = phyWidth;
    }

    void SetPhyHeight(uint32_t phyHeight)
    {
        phyHeight_ = phyHeight;
    }

    void SetSupportLayers(uint32_t supportLayers)
    {
        supportLayers_ = supportLayers;
    }

    void SetVirtualDispCount(uint32_t virtualDispCount)
    {
        virtualDispCount_ = virtualDispCount;
    }

    void SetSupportWriteBack(bool supportWriteBack)
    {
        supportWriteBack_ = supportWriteBack;
    }

    void SetProps(std::vector<RSScreenProps> props)
    {
        props_ = std::move(props);
    }

    const std::string& GetName() const
    {
        return name_;
    }

    ScreenInterfaceType GetType() const
    {
        return type_;
    }

    uint32_t GetPhyWidth() const
    {
        return phyWidth_;
    }

    uint32_t GetPhyHeight() const
    {
        return phyHeight_;
    }

    uint32_t GetSupportLayers() const
    {
        return supportLayers_;
    }

    uint32_t GetVirtualDispCount() const
    {
        return virtualDispCount_;
    }

    bool GetSupportWriteBack() const
    {
        return supportWriteBack_;
    }

    const std::vector<RSScreenProps>& GetProps() const
    {
        return props_;
    }

private:
    std::string name_;
    ScreenInterfaceType type_ = DISP_INVALID;
    uint32_t phyWidth_ = 0;
    uint32_t phyHeight_ = 0;
    uint32_t supportLayers_ = 0;
    uint32_t virtualDispCount_ = 0;
    bool supportWriteBack_ = false;
    std::vector<RSScreenProps> props_;
};

}  // namespace Rosen
}  // namespace OHOS

#endif  // RS_SCREEN_CAPABILITY