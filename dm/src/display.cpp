/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

#include "display.h"

#include <cstdint>
#include <new>
#include <refbase.h>

#include "class_var_definition.h"
#include "display_info.h"
#include "display_manager_adapter.h"
#include "dm_common.h"
#include "singleton_container.h"
#include "window_manager_hilog.h"
#include "pixel_map_napi.h"
#include "ani.h"
#include "concurrency_helpers.h"
#include <event_handler.h>
#include <event_runner.h>

namespace OHOS::Rosen {
std::shared_ptr<OHOS::AppExecFwk::EventHandler> g_eventHandler;
std::once_flag g_onceFlagForInitEventHandler;
std::shared_ptr<OHOS::AppExecFwk::EventHandler> GetMainEventHandler()
{
    std::call_once(g_onceFlagForInitEventHandler, [] {
        g_eventHandler =
            std::make_shared<OHOS::AppExecFwk::EventHandler>(OHOS::AppExecFwk::EventRunner::GetMainEventRunner());
    });
    return g_eventHandler;
}
class Display::Impl : public RefBase {
public:
    Impl(const std::string& name, sptr<DisplayInfo> info)
    {
        name_= name;
        SetDisplayInfo(info);
    }
    ~Impl() = default;
    DEFINE_VAR_FUNC_GET_SET(std::string, Name, name);
    sptr<DisplayInfo> GetDisplayInfo()
    {
        std::lock_guard<std::mutex> lock(displayInfoMutex_);
        return displayInfo_;
    }

    void SetDisplayInfo(sptr<DisplayInfo> value)
    {
        std::lock_guard<std::mutex> lock(displayInfoMutex_);
        displayInfo_ = value;
        displayUpdateTime_ = std::chrono::steady_clock::now();
    }

    void SetDisplayInfoEnv(void* env, EnvType type)
    {
        env_ = env;
        envType_ = type;
    }

    void* GetDisplayInfoEnv()
    {
        return env_;
    }

    EnvType GetEnvType()
    {
        return envType_;
    }

    void SetValidFlag(bool validFlag)
    {
        validFlag_ = validFlag;
    }

    bool GetValidFlag() const
    {
        return validFlag_;
    }

    std::chrono::steady_clock::time_point GetDisplayUpdateTime()
    {
        return displayUpdateTime_;
    }

private:
    sptr<DisplayInfo> displayInfo_;
    bool validFlag_ = false;
    void* env_ = nullptr;
    EnvType envType_ = EnvType::NONE;
    std::chrono::steady_clock::time_point displayUpdateTime_{};
    std::mutex displayInfoMutex_;
};

Display::Display(const std::string& name, sptr<DisplayInfo> info)
    : pImpl_(new Impl(name, info))
{
}

Display::~Display()
{
}

DisplayId Display::GetId() const
{
    if (pImpl_ == nullptr || pImpl_->GetDisplayInfo() == nullptr) {
        TLOGE(WmsLogTag::DMS, "pImpl_ or pImpl_->GetDisplayInfo is nullptr");
        return DisplayId(0);
    }
    return pImpl_->GetDisplayInfo()->GetDisplayId();
}

std::string Display::GetName() const
{
    if (pImpl_ == nullptr || pImpl_->GetDisplayInfo() == nullptr) {
        TLOGE(WmsLogTag::DMS, "pImpl_ or pImpl_->GetDisplayInfo is nullptr");
        return std::string();
    }
    return pImpl_->GetDisplayInfo()->GetName();
}

int32_t Display::GetWidth() const
{
    UpdateDisplayInfo();
    if (pImpl_ == nullptr || pImpl_->GetDisplayInfo() == nullptr) {
        TLOGE(WmsLogTag::DMS, "pImpl_ or pImpl_->GetDisplayInfo is nullptr");
        return 0;
    }
    return pImpl_->GetDisplayInfo()->GetWidth();
}

int32_t Display::GetHeight() const
{
    UpdateDisplayInfo();
    if (pImpl_ == nullptr || pImpl_->GetDisplayInfo() == nullptr) {
        TLOGE(WmsLogTag::DMS, "pImpl_ or pImpl_->GetDisplayInfo is nullptr");
        return 0;
    }
    return pImpl_->GetDisplayInfo()->GetHeight();
}

int32_t Display::GetPhysicalWidth() const
{
    UpdateDisplayInfo();
    if (pImpl_ == nullptr || pImpl_->GetDisplayInfo() == nullptr) {
        TLOGE(WmsLogTag::DMS, "pImpl_ or pImpl_->GetDisplayInfo is nullptr");
        return 0;
    }
    return pImpl_->GetDisplayInfo()->GetPhysicalWidth();
}

int32_t Display::GetPhysicalHeight() const
{
    UpdateDisplayInfo();
    if (pImpl_ == nullptr || pImpl_->GetDisplayInfo() == nullptr) {
        TLOGE(WmsLogTag::DMS, "pImpl_ or pImpl_->GetDisplayInfo is nullptr");
        return 0;
    }
    return pImpl_->GetDisplayInfo()->GetPhysicalHeight();
}

uint32_t Display::GetRefreshRate() const
{
    UpdateDisplayInfo();
    if (pImpl_ == nullptr || pImpl_->GetDisplayInfo() == nullptr) {
        TLOGE(WmsLogTag::DMS, "pImpl_ or pImpl_->GetDisplayInfo is nullptr");
        return 0;
    }
    return pImpl_->GetDisplayInfo()->GetRefreshRate();
}

ScreenId Display::GetScreenId() const
{
    UpdateDisplayInfo();
    if (pImpl_ == nullptr || pImpl_->GetDisplayInfo() == nullptr) {
        TLOGE(WmsLogTag::DMS, "pImpl_ or pImpl_->GetDisplayInfo is nullptr");
        return SCREEN_ID_INVALID;
    }
    return pImpl_->GetDisplayInfo()->GetScreenId();
}

Rotation Display::GetRotation() const
{
    UpdateDisplayInfo();
    if (pImpl_ == nullptr || pImpl_->GetDisplayInfo() == nullptr) {
        TLOGE(WmsLogTag::DMS, "pImpl_ or pImpl_->GetDisplayInfo is nullptr");
        return Rotation::ROTATION_0;
    }
    return pImpl_->GetDisplayInfo()->GetRotation();
}

Rotation Display::GetOriginRotation() const
{
    UpdateDisplayInfo();
    if (pImpl_ == nullptr) {
        TLOGE(WmsLogTag::DMS, "pImpl_ is nullptr");
        return Rotation::ROTATION_0;
    }
    auto displayInfo = pImpl_->GetDisplayInfo();
    if (displayInfo == nullptr) {
        TLOGE(WmsLogTag::DMS, "displayInfo is nullptr");
        return Rotation::ROTATION_0;
    }
    return displayInfo->GetOriginRotation();
}

Orientation Display::GetOrientation() const
{
    UpdateDisplayInfo();
    if (pImpl_ == nullptr || pImpl_->GetDisplayInfo() == nullptr) {
        TLOGE(WmsLogTag::DMS, "pImpl_ or pImpl_->GetDisplayInfo is nullptr");
        return Orientation::UNSPECIFIED;
    }
    return pImpl_->GetDisplayInfo()->GetOrientation();
}

void Display::UpdateDisplayInfo(sptr<DisplayInfo> displayInfo) const
{
    if (displayInfo == nullptr || pImpl_ == nullptr) {
        TLOGE(WmsLogTag::DMS, "displayInfo or pImpl_ is nullptr");
        return;
    }

    pImpl_->SetDisplayInfo(displayInfo);
}

// per display info on thread which is presented for env
// env is used to update display info
void Display::SetDisplayInfoEnv(void* env, EnvType type)
{
    if (pImpl_ == nullptr) {
        TLOGE(WmsLogTag::DMS, "pImpl_ is nullptr");
        return;
    }

    pImpl_->SetDisplayInfoEnv(env, type);
}

uint32_t Display::GetDisplayInfoLifeTime()
{
    if (pImpl_ == nullptr) {
        TLOGE(WmsLogTag::DMS, "pImpl_ is nullptr");
        return 0;
    }

    return std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() -
        pImpl_->GetDisplayUpdateTime()).count();
}

void Display::UpdateDisplayInfo() const
{
    if (pImpl_ == nullptr) {
        TLOGE(WmsLogTag::DMS, "pImpl_ is nullptr");
        return;
    }

    if (pImpl_->GetValidFlag()) {
        TLOGD(WmsLogTag::DMS, "do nothing PID %{public}d TID %{public}d ENV", getpid(), gettid());
        return;
    }

    auto displayInfo = SingletonContainer::Get<DisplayManagerAdapter>().GetDisplayInfo(GetId());
    if (displayInfo == nullptr) {
            return;
    }
    UpdateDisplayInfo(displayInfo);
    EnvType type = pImpl_->GetEnvType();
    void* env = pImpl_->GetDisplayInfoEnv();
    if (type == EnvType::NAPI) {
        DisplayNapiEnv nenv = static_cast<DisplayNapiEnv>(env);
        pImpl_->SetValidFlag(true);
        TLOGD(WmsLogTag::DMS, "set validFlag true PID %{public}d TID %{public}d", getpid(), gettid());

        // post task to current thread, the task will be executed after current task
        // the task is used to presented current task is finish
        auto asyncTask = [this]() {
            pImpl_->SetValidFlag(false);
            TLOGD(WmsLogTag::DMS, "set validFlag false PID %{public}d TID %{public}d", getpid(), gettid());
        };
        napi_status ret = napi_send_event(nenv, asyncTask, napi_eprio_vip, "UpdateDisplayValid");
        if (ret != napi_status::napi_ok) {
            pImpl_->SetValidFlag(false);
            TLOGD(WmsLogTag::DMS, "Failed to SendEvent");
        }
    } else if (type == EnvType::ANI) {
        pImpl_->SetValidFlag(true);
        TLOGD(WmsLogTag::DMS, "set validFlag true PID %{public}d TID %{public}d", getpid(), gettid());
        auto asyncTask = [this]() {
            pImpl_->SetValidFlag(false);
            TLOGD(WmsLogTag::DMS, "set validFlag false PID %{public}d TID %{public}d", getpid(), gettid());
        };
        std::shared_ptr<OHOS::AppExecFwk::EventHandler> eventHandler = GetMainEventHandler();
        eventHandler->PostTask(asyncTask, 0);
    } else {
        TLOGD(WmsLogTag::DMS, "update without env");
    }
}

float Display::GetVirtualPixelRatio() const
{
    UpdateDisplayInfo();
    if (pImpl_ == nullptr || pImpl_->GetDisplayInfo() == nullptr) {
        TLOGE(WmsLogTag::DMS, "pImpl_ or pImpl_->GetDisplayInfo is nullptr");
        return 0;
    }
    return pImpl_->GetDisplayInfo()->GetVirtualPixelRatio();
}

int Display::GetDpi() const
{
    return static_cast<int>(GetVirtualPixelRatio() * DOT_PER_INCH);
}

sptr<DisplayInfo> Display::GetDisplayInfo() const
{
    UpdateDisplayInfo();
    if (pImpl_ == nullptr || pImpl_->GetDisplayInfo() == nullptr) {
        TLOGE(WmsLogTag::DMS, "pImpl_ or pImpl_->GetDisplayInfo is nullptr");
        return nullptr;
    }
    return pImpl_->GetDisplayInfo();
}

sptr<DisplayInfo> Display::GetDisplayInfoWithCache() const
{
    if (pImpl_ == nullptr || pImpl_->GetDisplayInfo() == nullptr) {
        TLOGE(WmsLogTag::DMS, "pImpl_ or pImpl_->GetDisplayInfo is nullptr");
        return nullptr;
    }
    return pImpl_->GetDisplayInfo();
}

sptr<CutoutInfo> Display::GetCutoutInfo(sptr<DisplayInfo> displayInfo) const
{
    if (displayInfo == nullptr) {
        displayInfo = GetDisplayInfo();
        if (displayInfo == nullptr) {
            return nullptr;
        }
    }
    return SingletonContainer::Get<DisplayManagerAdapter>().GetCutoutInfo(displayInfo->GetDisplayId(),
        displayInfo->GetWidth(), displayInfo->GetHeight(), displayInfo->GetOriginRotation());
}

DMError Display::GetRoundedCorner(std::vector<RoundedCorner>& roundedCorner) const
{
    auto displayInfo = GetDisplayInfo();
    if (displayInfo == nullptr) {
        return DMError::DM_ERROR_NULLPTR;
    }
    return SingletonContainer::Get<DisplayManagerAdapter>().GetRoundedCorner(roundedCorner,
        displayInfo->GetDisplayId(), displayInfo->GetWidth(), displayInfo->GetHeight());
}

DMError Display::HasImmersiveWindow(bool& immersive)
{
    return SingletonContainer::Get<DisplayManagerAdapter>().HasImmersiveWindow(GetScreenId(), immersive);
}

DMError Display::GetAvailableArea(DMRect& area) const
{
    return SingletonContainer::Get<DisplayManagerAdapter>().GetAvailableArea(GetId(), area);
}

DMError Display::GetSupportedHDRFormats(std::vector<uint32_t>& hdrFormats) const
{
    return SingletonContainer::Get<ScreenManagerAdapter>().GetSupportedHDRFormats(GetScreenId(), hdrFormats);
}

DMError Display::GetSupportedColorSpaces(std::vector<uint32_t>& colorSpaces) const
{
    return SingletonContainer::Get<ScreenManagerAdapter>().GetSupportedColorSpaces(GetScreenId(), colorSpaces);
}

DMError Display::GetDisplayCapability(std::string& capabilitInfo) const
{
    return SingletonContainer::Get<DisplayManagerAdapter>().GetDisplayCapability(capabilitInfo);
}

DMError Display::GetLiveCreaseRegion(FoldCreaseRegion& region) const
{
    ScreenId screenId = GetScreenId();
    ScreenId rsScreenId;
    bool ret = SingletonContainer::Get<DisplayManagerAdapter>().ConvertScreenIdToRsScreenId(screenId, rsScreenId);
    if (!ret) {
        TLOGE(WmsLogTag::DMS, "convertScreenIdToRsScreenId falied");
        return DMError::DM_ERROR_DEVICE_NOT_SUPPORT;
    }
    // when rsScreenId is not 0, there is no crease region in the current screen
    if (rsScreenId == MAIN_SCREEN_ID_DEFAULT) {
        return SingletonContainer::Get<DisplayManagerAdapter>().GetLiveCreaseRegion(region);
    }
    region = FoldCreaseRegion(screenId, {});
    return DMError::DM_OK;
}
} // namespace OHOS::Rosen
