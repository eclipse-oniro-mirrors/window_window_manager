/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#include "root_scene.h"

#include <bundlemgr/launcher_service.h>
#include <event_handler.h>
#include <input_manager.h>
#include <int_wrapper.h>
#include <iremote_stub.h>
#include <transaction/rs_interfaces.h>
#include <ui_content.h>
#include <ui/rs_node.h>
#include <viewport_config.h>

#include "ability_context.h"
#include "app_mgr_client.h"
#include "extension/extension_business_info.h"
#include "fold_screen_state_internel.h"
#include "input_transfer_station.h"
#include "singleton.h"
#include "singleton_container.h"

#include "intention_event_manager.h"
#include "window_manager_hilog.h"

namespace OHOS {
namespace Rosen {
namespace {
constexpr HiviewDFX::HiLogLabel LABEL = { LOG_CORE, HILOG_DOMAIN_WINDOW, "RootScene" };
const std::string INPUT_AND_VSYNC_THREAD = "InputAndVsyncThread";
constexpr int32_t API_VERSION_18 = 18;

class BundleStatusCallback : public IRemoteStub<AppExecFwk::IBundleStatusCallback> {
public:
    explicit BundleStatusCallback(RootScene* rootScene) : rootScene_(rootScene) {}
    virtual ~BundleStatusCallback() = default;

    void OnBundleStateChanged(const uint8_t installType,
        const int32_t resultCode, const std::string& resultMsg, const std::string& bundleName) override {}

    void OnBundleAdded(const std::string& bundleName, const int userId) override
    {
        rootScene_->OnBundleUpdated(bundleName);
    }

    void OnBundleUpdated(const std::string& bundleName, const int userId) override
    {
        rootScene_->OnBundleUpdated(bundleName);
    }

    void OnBundleRemoved(const std::string& bundleName, const int userId) override {}

private:
    RootScene* rootScene_;
};
} // namespace

sptr<RootScene> RootScene::staticRootScene_;
std::function<void(const std::shared_ptr<AppExecFwk::Configuration>&)> RootScene::configurationUpdatedCallback_;

RootScene::RootScene()
{
    launcherService_ = new AppExecFwk::LauncherService();
    if (!launcherService_->RegisterCallback(new BundleStatusCallback(this))) {
        WLOGFE("Failed to register bundle status callback.");
    }

    NodeId nodeId = 0;
    vsyncStation_ = std::make_shared<VsyncStation>(nodeId);
    handler_ = std::make_shared<AppExecFwk::EventHandler>(AppExecFwk::EventRunner::GetMainEventRunner());
}

RootScene::~RootScene()
{
    TLOGI(WmsLogTag::WMS_MAIN, "destroyed");
    uiContent_ = nullptr;
}

void RootScene::LoadContent(const std::string& contentUrl, napi_env env, napi_value storage,
    AbilityRuntime::Context* context)
{
    if (context == nullptr) {
        WLOGFE("context is nullptr!");
        return;
    }
    uiContent_ = Ace::UIContent::Create(context, reinterpret_cast<NativeEngine*>(env));
    if (uiContent_ == nullptr) {
        WLOGFE("uiContent_ is nullptr!");
        return;
    }

    context_ = context->weak_from_this();
    uiContent_->Initialize(this, contentUrl, storage);
    uiContent_->Foreground();
    uiContent_->SetFrameLayoutFinishCallback(std::move(frameLayoutFinishCb_));
    uiContent_->AddFocusActiveChangeCallback(notifyWatchFocusActiveChangeCallback_);
    RegisterInputEventListener();
}

void RootScene::SetDisplayOrientation(int32_t orientation)
{
    orientation_ = orientation;
}

void RootScene::UpdateViewportConfig(const Rect& rect, WindowSizeChangeReason reason)
{
    if (updateRootSceneRectCallback_ != nullptr) {
        updateRootSceneRectCallback_(rect);
    }

    if (uiContent_ == nullptr) {
        WLOGFE("uiContent_ is nullptr!");
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

void RootScene::UpdateConfiguration(const std::shared_ptr<AppExecFwk::Configuration>& configuration)
{
    if (uiContent_) {
        WLOGFD("in");
        uiContent_->UpdateConfiguration(configuration);
        if (configuration == nullptr) {
            return;
        }
        std::string colorMode = configuration->GetItem(AAFwk::GlobalConfigurationKey::SYSTEM_COLORMODE);
        bool isDark = (colorMode == AppExecFwk::ConfigurationInner::COLOR_MODE_DARK);
        bool ret = RSInterfaces::GetInstance().SetGlobalDarkColorMode(isDark);
        if (ret == false) {
            WLOGFE("SetGlobalDarkColorMode fail with colorMode : %{public}s", colorMode.c_str());
        }
    }
}

void RootScene::UpdateConfigurationForSpecified(const std::shared_ptr<AppExecFwk::Configuration>& configuration,
    const std::shared_ptr<Global::Resource::ResourceManager>& resourceManager)
{
    if (uiContent_ == nullptr) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "uiContent is null, winId: %{public}u", GetWindowId());
        return;
    }
    uiContent_->UpdateConfiguration(configuration, resourceManager);
    if (configuration == nullptr) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "config is null, winId: %{public}u", GetWindowId());
        return;
    }
    std::string colorMode = configuration->GetItem(AAFwk::GlobalConfigurationKey::SYSTEM_COLORMODE);
    bool isDark = (colorMode == AppExecFwk::ConfigurationInner::COLOR_MODE_DARK);
    if (!RSInterfaces::GetInstance().SetGlobalDarkColorMode(isDark)) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "set dark color mode failed, winId: %{public}u, colorMode: %{public}s",
            GetWindowId(), colorMode.c_str());
    }
}

void RootScene::UpdateConfigurationForAll(const std::shared_ptr<AppExecFwk::Configuration>& configuration,
    const std::vector<std::shared_ptr<AbilityRuntime::Context>>& ignoreWindowContexts)
{
    TLOGI(WmsLogTag::WMS_ATTRIBUTE, "in");
    if (staticRootScene_) {
        auto context = staticRootScene_->GetContext();
        if (context == nullptr) {
            TLOGE(WmsLogTag::WMS_ATTRIBUTE, "context is null, winId: %{public}u", staticRootScene_->GetWindowId());
            return;
        }
        if (std::count(ignoreWindowContexts.begin(), ignoreWindowContexts.end(), context) == 0) {
            staticRootScene_->UpdateConfiguration(configuration);
            if (configurationUpdatedCallback_) {
                configurationUpdatedCallback_(configuration);
            }
        }
    }
}

void RootScene::UpdateConfigurationSync(const std::shared_ptr<AppExecFwk::Configuration>& configuration)
{
    if (uiContent_ == nullptr) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "uiContent is null, winId: %{public}d", GetWindowId());
        return;
    }
    TLOGI(WmsLogTag::WMS_ATTRIBUTE, "winId: %{public}d", GetWindowId());
    uiContent_->UpdateConfigurationSyncForAll(configuration);
}

void RootScene::UpdateConfigurationSyncForAll(const std::shared_ptr<AppExecFwk::Configuration>& configuration)
{
    TLOGD(WmsLogTag::WMS_ATTRIBUTE, "in");
    if (staticRootScene_ != nullptr) {
        staticRootScene_->UpdateConfigurationSync(configuration);
    }
}

void RootScene::RegisterInputEventListener()
{
    auto mainEventRunner = AppExecFwk::EventRunner::GetMainEventRunner();
    if (mainEventRunner) {
        WLOGFD("MainEventRunner is available");
        eventHandler_ = std::make_shared<AppExecFwk::EventHandler>(mainEventRunner);
    } else {
        WLOGFD("MainEventRunner is not available");
        eventHandler_ = AppExecFwk::EventHandler::Current();
        if (!eventHandler_) {
            eventHandler_ =
                std::make_shared<AppExecFwk::EventHandler>(AppExecFwk::EventRunner::Create(INPUT_AND_VSYNC_THREAD));
        }
    }
    if (!(DelayedSingleton<IntentionEventManager>::GetInstance()->EnableInputEventListener(
        uiContent_.get(), eventHandler_))) {
        WLOGFE("EnableInputEventListener fail");
    }
    InputTransferStation::GetInstance().MarkRegisterToMMI();
}

void RootScene::RequestVsync(const std::shared_ptr<VsyncCallback>& vsyncCallback)
{
    vsyncStation_->RequestVsync(vsyncCallback);
}

int64_t RootScene::GetVSyncPeriod()
{
    return vsyncStation_->GetVSyncPeriod();
}

void RootScene::FlushFrameRate(uint32_t rate, int32_t animatorExpectedFrameRate, uint32_t rateType)
{
    vsyncStation_->FlushFrameRate(rate, animatorExpectedFrameRate, rateType);
}

bool RootScene::IsLastFrameLayoutFinished()
{
    int32_t requestTimes = vsyncStation_->GetRequestVsyncTimes();
    TLOGI(WmsLogTag::WMS_LAYOUT, "vsync request times: %{public}d", requestTimes);
    return requestTimes <= 0;
}

void RootScene::OnFlushUIParams()
{
    vsyncStation_->DecreaseRequestVsyncTimes();
}

void RootScene::OnBundleUpdated(const std::string& bundleName)
{
    WLOGFD("bundle %{public}s updated", bundleName.c_str());
    if (uiContent_) {
        uiContent_->UpdateResource();
    }
}

void RootScene::SetOnConfigurationUpdatedCallback(
    const std::function<void(const std::shared_ptr<AppExecFwk::Configuration>&)>& callback)
{
    configurationUpdatedCallback_ = callback;
}

void RootScene::SetFrameLayoutFinishCallback(std::function<void()>&& callback)
{
    frameLayoutFinishCb_ = callback;
    if (uiContent_) {
        uiContent_->SetFrameLayoutFinishCallback(std::move(frameLayoutFinishCb_));
    }
    TLOGI(WmsLogTag::WMS_LAYOUT, "end");
}

void RootScene::SetUiDvsyncSwitch(bool dvsyncSwitch)
{
    vsyncStation_->SetUiDvsyncSwitch(dvsyncSwitch);
}

WMError RootScene::GetAvoidAreaByType(AvoidAreaType type, AvoidArea& avoidArea, const Rect& rect, int32_t apiVersion)
{
    if (getSessionAvoidAreaByTypeCallback_ == nullptr) {
        TLOGE(WmsLogTag::WMS_IMMS, "getSessionAvoidAreaByTypeCallback is nullptr");
        return WMError::WM_ERROR_NULLPTR;
    }
    if (apiVersion != API_VERSION_INVALID && apiVersion < API_VERSION_18) {
        TLOGI(WmsLogTag::WMS_IMMS, "root scene UIExtension type %{public}u api %{public}d not supported",
            type, apiVersion);
        return WMError::WM_DO_NOTHING;
    }
    avoidArea = getSessionAvoidAreaByTypeCallback_(type);
    TLOGI(WmsLogTag::WMS_IMMS, "root scene type %{public}u area %{public}s", type, avoidArea.ToString().c_str());
    return WMError::WM_OK;
}

void RootScene::RegisterGetSessionAvoidAreaByTypeCallback(GetSessionAvoidAreaByTypeCallback&& callback)
{
    getSessionAvoidAreaByTypeCallback_ = std::move(callback);
}

void RootScene::RegisterUpdateRootSceneRectCallback(UpdateRootSceneRectCallback&& callback)
{
    updateRootSceneRectCallback_ = std::move(callback);
}

void RootScene::RegisterUpdateRootSceneAvoidAreaCallback(UpdateRootSceneAvoidAreaCallback&& callback)
{
    updateRootSceneAvoidAreaCallback_ = std::move(callback);
}

WMError RootScene::RegisterAvoidAreaChangeListener(const sptr<IAvoidAreaChangedListener>& listener)
{
    if (listener == nullptr) {
        TLOGE(WmsLogTag::WMS_IMMS, "listener is null");
        return WMError::WM_ERROR_NULLPTR;
    }
    bool firstInserted = false;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        if (avoidAreaChangeListeners_.find(listener) == avoidAreaChangeListeners_.end()) {
            TLOGI(WmsLogTag::WMS_IMMS, "register success");
            avoidAreaChangeListeners_.insert(listener);
            firstInserted = true;
        }
    }
    if (firstInserted) {
        updateRootSceneAvoidAreaCallback_();
    }
    return WMError::WM_OK;
}

WMError RootScene::UnregisterAvoidAreaChangeListener(const sptr<IAvoidAreaChangedListener>& listener)
{
    if (listener == nullptr) {
        TLOGE(WmsLogTag::WMS_IMMS, "listener is null");
        return WMError::WM_ERROR_NULLPTR;
    }
    TLOGI(WmsLogTag::WMS_IMMS, "unregister success");
    std::lock_guard<std::mutex> lock(mutex_);
    avoidAreaChangeListeners_.erase(listener);
    return WMError::WM_OK;
}

void RootScene::NotifyAvoidAreaChangeForRoot(const sptr<AvoidArea>& avoidArea, AvoidAreaType type)
{
    TLOGI(WmsLogTag::WMS_IMMS, "type %{public}d area %{public}s.", type, avoidArea->ToString().c_str());
    std::lock_guard<std::mutex> lock(mutex_);
    for (auto& listener : avoidAreaChangeListeners_) {
        if (listener != nullptr) {
            listener->OnAvoidAreaChanged(*avoidArea, type);
        }
    }
}

WMError RootScene::RegisterOccupiedAreaChangeListener(const sptr<IOccupiedAreaChangeListener>& listener)
{
    if (listener == nullptr) {
        TLOGE(WmsLogTag::WMS_KEYBOARD, "listener is null");
        return WMError::WM_ERROR_NULLPTR;
    }
    std::lock_guard<std::mutex> lock(occupiedAreaMutex_);
    if (occupiedAreaChangeListeners_.find(listener) == occupiedAreaChangeListeners_.end()) {
        TLOGI(WmsLogTag::WMS_KEYBOARD, "register success");
        occupiedAreaChangeListeners_.insert(listener);
    }
    return WMError::WM_OK;
}

WMError RootScene::UnregisterOccupiedAreaChangeListener(const sptr<IOccupiedAreaChangeListener>& listener)
{
    if (listener == nullptr) {
        TLOGE(WmsLogTag::WMS_KEYBOARD, "listener is null");
        return WMError::WM_ERROR_NULLPTR;
    }
    TLOGI(WmsLogTag::WMS_KEYBOARD, "unregister success");
    std::lock_guard<std::mutex> lock(occupiedAreaMutex_);
    occupiedAreaChangeListeners_.erase(listener);
    return WMError::WM_OK;
}

void RootScene::NotifyOccupiedAreaChangeForRoot(const sptr<OccupiedAreaChangeInfo>& info)
{
    if (info == nullptr) {
        TLOGI(WmsLogTag::WMS_KEYBOARD, "occupied area info is null");
        return;
    }
    if (handler_ == nullptr) {
        TLOGI(WmsLogTag::WMS_KEYBOARD, "handler_ is null, notify occupied area for root failed.");
        return;
    }
    TLOGI(WmsLogTag::WMS_KEYBOARD, "occupiedRect: %{public}s, textField PositionY_: %{public}f, Height_: %{public}f",
        info->rect_.ToString().c_str(), info->textFieldPositionY_, info->textFieldHeight_);
    auto task = [weak = wptr(this), info]() {
        auto window = weak.promote();
        if (!window) {
            TLOGE(WmsLogTag::WMS_KEYBOARD, "window is null");
            return;
        }
        std::lock_guard<std::mutex> lock(window->occupiedAreaMutex_);
        for (const auto& listener : window->occupiedAreaChangeListeners_) {
            if (listener != nullptr) {
                listener->OnSizeChange(info);
            }
        }
    };
    handler_->PostTask(task, __func__);
}

void RootScene::RegisterNotifyWatchFocusActiveChangeCallback(NotifyWatchFocusActiveChangeCallback&& callback)
{
    notifyWatchFocusActiveChangeCallback_ = std::move(callback);
}

std::shared_ptr<Rosen::RSNode> RootScene::GetRSNodeByStringID(const std::string& stringId)
{
    TLOGI(WmsLogTag::WMS_LAYOUT, "id: %{public}s", stringId.c_str());
    if (uiContent_ == nullptr) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "uiContent is null, winId: %{public}d", GetWindowId());
        return nullptr;
    }
    TLOGI(WmsLogTag::WMS_LAYOUT, "end");
    return uiContent_->GetRSNodeByStringID(stringId);
}

void RootScene::SetTopWindowBoundaryByID(const std::string& stringId)
{
    TLOGI(WmsLogTag::WMS_LAYOUT, "id: %{public}s", stringId.c_str());
    if (uiContent_ == nullptr) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "uiContent is null, winId: %{public}d", GetWindowId());
        return;
    }
    TLOGI(WmsLogTag::WMS_LAYOUT, "end");
    uiContent_->SetTopWindowBoundaryByID(stringId);
}

void RootScene::GetExtensionConfig(AAFwk::WantParams& want) const
{
    int32_t rootHostWindowType = static_cast<int32_t>(WindowType::WINDOW_TYPE_SCENE_BOARD);
    want.SetParam(Extension::ROOT_HOST_WINDOW_TYPE_FIELD, AAFwk::Integer::Box(rootHostWindowType));
}
} // namespace Rosen
} // namespace OHOS
