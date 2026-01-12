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

#include "screen_scene.h"

#include <event_handler.h>
#include <transaction/rs_interfaces.h>
#include <ui_content.h>
#include <viewport_config.h>

#include "app_mgr_client.h"
#include "singleton.h"
#include "singleton_container.h"

#include "dm_common.h"
#include "display_manager.h"
#include "rs_adapter.h"
#include "screen_session_manager_client.h"
#include "intention_event_manager.h"
#include "input_transfer_station.h"
#include "window_manager_hilog.h"
#include "root_scene.h"
#include "load_intention_event.h"

namespace OHOS {
namespace Rosen {
namespace {
constexpr float MIN_DPI = 1e-6;
std::atomic<bool> g_ssIsDestroyed = false;
const std::string INPUT_AND_VSYNC_THREAD = "InputAndVsyncThread";
} // namespace

ScreenScene::ScreenScene(std::string name) : name_(name)
{
    orientation_ = static_cast<int32_t>(DisplayOrientation::PORTRAIT);
    NodeId nodeId = 0;
    vsyncStation_ = std::make_shared<VsyncStation>(nodeId);
    handler_ = std::make_shared<AppExecFwk::EventHandler>(AppExecFwk::EventRunner::GetMainEventRunner());
    g_ssIsDestroyed = false;
    displayId_ = DISPLAY_ID_INVALID;
}

ScreenScene::~ScreenScene()
{
    g_ssIsDestroyed = true;
    Destroy();
}

WMError ScreenScene::Destroy(uint32_t reason)
{
    std::function<void()> task; //延长task的生命周期
    {
        std::lock_guard<std::mutex> lock(mutex_);
        if (!uiContent_) {
            TLOGD(WmsLogTag::DMS, "Destroy uiContent_ is nullptr!");
            return WMError::WM_OK;
        }
        std::shared_ptr<Ace::UIContent> uiContent = std::move(uiContent_);
        uiContent_ = nullptr;
        RootScene::staticRootScene_->RemoveRootScene(displayId_);
        vsyncStation_->Destroy();
        task = [uiContent]() {
            if (uiContent != nullptr) {
                uiContent->Destroy();
                TLOGD(WmsLogTag::DMS, "ScreenScene: uiContent destroy success!");
            }
        };
    }
    if (handler_) {
        handler_->PostSyncTask(task, "ScreenScene:Destroy");
    } else {
        task();
    }
    return WMError::WM_OK;
}

void ScreenScene::LoadContent(const std::string& contentUrl, napi_env env, napi_value storage,
    AbilityRuntime::Context* context)
{
    if (g_ssIsDestroyed) {
        TLOGI(WmsLogTag::DMS, "ScreenScene has been destructed!");
        return;
    }
    if (context == nullptr) {
        TLOGE(WmsLogTag::DMS, "context is nullptr!");
        return;
    }
    std::lock_guard<std::mutex> lock(mutex_);
    uiContent_ = Ace::UIContent::Create(context, reinterpret_cast<NativeEngine*>(env));
    if (uiContent_ == nullptr) {
        TLOGE(WmsLogTag::DMS, "uiContent_ is nullptr!");
        return;
    }
    TLOGW(WmsLogTag::DMS, "load concent");
    uiContent_->Initialize(this, contentUrl, storage);
    uiContent_->Foreground();
    uiContent_->SetFrameLayoutFinishCallback(std::move(frameLayoutFinishCb_));
    wptr<Window> weakWindow(this);
    RootScene::staticRootScene_->AddRootScene(DEFAULT_DISPLAY_ID, weakWindow);
    RegisterInputEventListener();
}

void ScreenScene::RegisterInputEventListener()
{
    auto mainEventRunner = AppExecFwk::EventRunner::GetMainEventRunner();
    if (mainEventRunner) {
        TLOGI(WmsLogTag::DMS, "MainEventRunner is available");
        handler_ = std::make_shared<AppExecFwk::EventHandler>(mainEventRunner);
    } else {
        TLOGI(WmsLogTag::DMS, "MainEventRunner is not available");
        handler_ = AppExecFwk::EventHandler::Current();
        if (!handler_) {
            handler_ =
                std::make_shared<AppExecFwk::EventHandler>(AppExecFwk::EventRunner::Create(INPUT_AND_VSYNC_THREAD));
        }
    }
    if (RootScene::staticRootScene_ != nullptr && LoadIntentionEvent() &&
        !EnableInputEventListener(uiContent_.get(), handler_, this)) {
        TLOGI(WmsLogTag::DMS, "EnableInputEventListener fail");
    }
    InputTransferStation::GetInstance().MarkRegisterToMMI();
}

void ScreenScene::UpdateViewportConfig(const Rect& rect, WindowSizeChangeReason reason)
{
    if (g_ssIsDestroyed) {
        TLOGI(WmsLogTag::DMS, "ScreenScene has been destructed!");
        return;
    }
    std::lock_guard<std::mutex> lock(mutex_);
    if (uiContent_ == nullptr) {
        TLOGE(WmsLogTag::DMS, "uiContent_ is nullptr!");
        return;
    }
    Ace::ViewportConfig config;
    config.SetSize(rect.width_, rect.height_);
    config.SetPosition(rect.posX_, rect.posY_);
    config.SetDensity(density_);
    config.SetOrientation(orientation_);
    config.SetDisplayId(GetDisplayId());
    uiContent_->UpdateViewportConfig(config, reason);
}

void ScreenScene::UpdateConfiguration(const std::shared_ptr<AppExecFwk::Configuration>& configuration)
{
    if (g_ssIsDestroyed) {
        TLOGI(WmsLogTag::DMS, "ScreenScene has been destructed!");
        return;
    }
    std::lock_guard<std::mutex> lock(mutex_);
    if (uiContent_) {
        TLOGD(WmsLogTag::DMS, "notify root scene ace");
        uiContent_->UpdateConfiguration(configuration);
        if (configuration == nullptr) {
            return;
        }
        std::string colorMode = configuration->GetItem(AAFwk::GlobalConfigurationKey::SYSTEM_COLORMODE);
        bool isDark = (colorMode == AppExecFwk::ConfigurationInner::COLOR_MODE_DARK);
        bool ret = RSInterfaces::GetInstance().SetGlobalDarkColorMode(isDark);
        if (!ret) {
            TLOGI(WmsLogTag::DMS, "SetGlobalDarkColorMode fail with colorMode : %{public}s", colorMode.c_str());
        }
    }
}

void ScreenScene::UpdateConfigurationForAll(const std::shared_ptr<AppExecFwk::Configuration>& configuration)
{
    TLOGI(WmsLogTag::DMS, "update configuration.");
    UpdateConfiguration(configuration);
    if (configurationUpdateCallback_) {
        configurationUpdateCallback_(configuration);
    }
}

void ScreenScene::SetOnConfigurationUpdatedCallback(
    const std::function<void(const std::shared_ptr<AppExecFwk::Configuration>&)>& callback)
{
    configurationUpdateCallback_ = callback;
}

void ScreenScene::RequestVsync(const std::shared_ptr<VsyncCallback>& vsyncCallback)
{
    vsyncStation_->RequestVsync(vsyncCallback);
}

int64_t ScreenScene::GetVSyncPeriod()
{
    return vsyncStation_->GetVSyncPeriod();
}

void ScreenScene::FlushFrameRate(uint32_t rate, int32_t animatorExpectedFrameRate, uint32_t rateType)
{
    vsyncStation_->FlushFrameRate(GetRSUIContext(), rate, animatorExpectedFrameRate, rateType);
}

void ScreenScene::OnBundleUpdated(const std::string& bundleName)
{
    if (g_ssIsDestroyed) {
        TLOGI(WmsLogTag::DMS, "ScreenScene has been destructed!");
        return;
    }
    TLOGD(WmsLogTag::DMS, "bundle %{public}s updated", bundleName.c_str());
    std::lock_guard<std::mutex> lock(mutex_);
    if (uiContent_) {
        uiContent_->UpdateResource();
    }
}

void ScreenScene::SetFrameLayoutFinishCallback(std::function<void()>&& callback)
{
    if (g_ssIsDestroyed) {
        TLOGI(WmsLogTag::DMS, "ScreenScene has been destructed!");
        return;
    }
    frameLayoutFinishCb_ = callback;
    std::lock_guard<std::mutex> lock(mutex_);
    if (uiContent_) {
        uiContent_->SetFrameLayoutFinishCallback(std::move(frameLayoutFinishCb_));
        frameLayoutFinishCb_ = nullptr;
    }
    TLOGI(WmsLogTag::WMS_LAYOUT, "SetFrameLayoutFinishCallback end");
}

void ScreenScene::SetDisplayDensity(float density)
{
    if (density < MIN_DPI) {
        TLOGE(WmsLogTag::DMS, "invalid density");
        return;
    }
    density_ = density;
}

uint64_t ScreenScene::GetDisplayId() const
{
    return displayId_;
}

void ScreenScene::SetDisplayId(DisplayId displayId)
{
    displayId_ = displayId;
}

void ScreenScene::SetDisplayOrientation(int32_t orientation)
{
    if (orientation < static_cast<int32_t>(DisplayOrientation::PORTRAIT) ||
        orientation > static_cast<int32_t>(DisplayOrientation::UNKNOWN)) {
        TLOGE(WmsLogTag::DMS, "invalid orientation");
        return;
    }
    orientation_ = orientation;
}

Ace::UIContent* ScreenScene::GetUIContent() const
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (uiContent_) {
        return uiContent_.get();
    } else {
        TLOGE(WmsLogTag::DMS, "uiContent_ is nullptr!");
        return nullptr;
    }
}

std::shared_ptr<RSUIDirector> ScreenScene::GetRSUIDirector() const
{
    RETURN_IF_RS_CLIENT_MULTI_INSTANCE_DISABLED(nullptr);
    sptr<Display> display;
    if (displayId_ == DISPLAY_ID_INVALID) {
        display = DisplayManager::GetInstance().GetDefaultDisplay();
        TLOGE(WmsLogTag::WMS_SCB, "displayId is invalid, use default display");
    } else {
        display = DisplayManager::GetInstance().GetDisplayById(displayId_);
    }
    if (!display) {
        TLOGE(WmsLogTag::WMS_SCB, "display is null, displayId: %{public}" PRIu64, displayId_);
        return nullptr;
    }
    auto screenId = display->GetScreenId();
    auto rsUIDirector = ScreenSessionManagerClient::GetInstance().GetRSUIDirector(screenId);
    TLOGD(WmsLogTag::WMS_SCB, "%{public}s, screenId: %{public}" PRIu64 ", windowId: %{public}d",
          RSAdapterUtil::RSUIDirectorToStr(rsUIDirector).c_str(), screenId, GetWindowId());
    return rsUIDirector;
}

std::shared_ptr<RSUIContext> ScreenScene::GetRSUIContext() const
{
    RETURN_IF_RS_CLIENT_MULTI_INSTANCE_DISABLED(nullptr);
    auto rsUIDirector = GetRSUIDirector();
    auto rsUIContext = rsUIDirector ? rsUIDirector->GetRSUIContext() : nullptr;
    TLOGD(WmsLogTag::WMS_SCB, "%{public}s, windowId: %{public}d",
          RSAdapterUtil::RSUIContextToStr(rsUIContext).c_str(), GetWindowId());
    return rsUIContext;
}
} // namespace Rosen
} // namespace OHOS
