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

#ifndef RS_SCREEN_MODE_INFO_DMS_MOCK
#define RS_SCREEN_MODE_INFO_DMS_MOCK

#include <cstdint>
#include <parcel.h>

namespace OHOS {
namespace Rosen {
class RSScreenModeInfo : public Parcelable {
public:
    RSScreenModeInfo() = default;
    RSScreenModeInfo(int32_t width, int32_t height, uint32_t refreshRate, int32_t id)
        : width_(width),
          height_(height),
          refreshRate_(refreshRate),
          modeId_(id)
    {
    }
    ~RSScreenModeInfo() noexcept = default;

    bool Marshalling(Parcel& parcel) const override
    {
        return true;
    }
    static RSScreenModeInfo* Unmarshalling(Parcel& parcel)
    {
        return nullptr;
    }

    int32_t GetScreenWidth() const
    {
        return width_;
    }
    int32_t GetScreenHeight() const
    {
        return height_;
    }
    uint32_t GetScreenRefreshRate() const
    {
        return refreshRate_;
    }
    int32_t GetScreenModeId() const
    {
        return modeId_;
    }
    void SetScreenWidth(int32_t width)
    {
        width_ = width;
    }
    void SetScreenHeight(int32_t height)
    {
        height_ = height;
    }
    void SetScreenRefreshRate(uint32_t refreshRate)
    {
        refreshRate_ = refreshRate;
    }
    void SetScreenModeId(int32_t id)
    {
        id = modeId_;
    }

private:
    int32_t width_ = -1;
    int32_t height_ = -1;
    uint32_t refreshRate_ = 0;
    int32_t modeId_ = -1;
};
} // namespace Rosen
} // namespace OHOS

#endif // RS_SCREEN_MODE_INFO