/*
 * Copyright (c) 2022-2023 Huawei Device Co., Ltd.
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

#include "window_scene.h"

#include <configuration.h>
#include "window_impl.h"
#include "window_manager_hilog.h"
#include "window_model.h"
#include "window_display.h"

namespace OHOS {
namespace Rosen {
const std::string WindowScene::MAIN_WINDOW_ID = "main window";
namespace {
constexpr HiviewDFX::HiLogLabel LABEL = {LOG_CORE, HILOG_DOMAIN_WINDOW, "WindowScene"};
}
WindowScene::~WindowScene()
{
}

WMError WindowScene::Init(DisplayId displayId, const std::shared_ptr<AbilityRuntime::Context>& context,
    sptr<IWindowLifeCycle>& listener, sptr<WindowOption> option)
{
    WLOGFD("WindowScene with window session!");
    displayId_ = displayId;
    if (option == nullptr) {
        option = new(std::nothrow) WindowOption();
        if (option == nullptr) {
            WLOGFE("alloc WindowOption failed");
            return WMError::WM_ERROR_NULLPTR;
        }
    }
    option->SetDisplayId(displayId);
    option->SetWindowTag(WindowTag::MAIN_WINDOW);

    mainWindow_ = Window::Create(GenerateMainWindowName(context), option, context);
    if (mainWindow_ == nullptr) {
        WLOGFE("mainWindow_ is NULL");
        return WMError::WM_ERROR_NULLPTR;
    }

    Previewer::PreviewerWindowModel& windowModel =  Previewer::PreviewerWindow::GetInstance().GetWindowParams();
    Ace::ViewportConfig config;
    config.SetSize(windowModel.originWidth, windowModel.originHeight);
    config.SetPosition(0, 0);
    config.SetOrientation(static_cast<int32_t>(Previewer::PreviewerWindow::TransOrientation(windowModel.orientation)));
    config.SetDensity(windowModel.density);
    mainWindow_->SetViewportConfig(config);

    Previewer::PreviewerWindow::GetInstance().SetWindowObject(mainWindow_.GetRefPtr());
    mainWindow_->RegisterLifeCycleListener(listener);

    Previewer::PreviewerDisplay::GetInstance().RegisterStatusChangedCallback(
        [this](FoldStatus status) {
            WLOGFI("FoldStatus changed to %{public}d", status);
            if (mainWindow_ == nullptr) {
                WLOGFE("mainWindow_ is NULL");
                return WMError::WM_ERROR_NULLPTR;
            }
            Previewer::PreviewerDisplay::GetInstance().SetFoldStatus(status);
            return WMError::WM_OK;
        }
    );

    return WMError::WM_OK;
}

std::string WindowScene::GenerateMainWindowName(const std::shared_ptr<AbilityRuntime::Context>& context) const
{
    if (context == nullptr) {
        return MAIN_WINDOW_ID + std::to_string(count++);
    } else {
        auto options = context->GetOptions();
        std::string windowName = options.bundleName + std::to_string(count++);
        std::size_t pos = windowName.find_last_of('.');
        return (pos == std::string::npos) ? windowName : windowName.substr(pos + 1); // skip '.'
    }
}

sptr<Window> WindowScene::CreateWindow(const std::string& windowName, sptr<WindowOption>& option) const
{
    return nullptr;
}

sptr<Window> WindowScene::CreateWindow(const std::string& windowName, sptr<WindowOption>& option,
    std::string& errMsg) const
{
    return nullptr;
}

const sptr<Window>& WindowScene::GetMainWindow() const
{
    return mainWindow_;
}

std::vector<sptr<Window>> WindowScene::GetSubWindow()
{
    return std::vector<sptr<Window>>();
}

WMError WindowScene::GoDestroy()
{
    if (mainWindow_ == nullptr) {
        return WMError::WM_ERROR_NULLPTR;
    }

    WMError ret = mainWindow_->Destroy();
    if (ret != WMError::WM_OK) {
        WLOGFE("WindowScene go destroy failed name: %{public}s", mainWindow_->GetWindowName().c_str());
        return ret;
    }
    mainWindow_ = nullptr;
    return WMError::WM_OK;
}

void WindowScene::UpdateConfiguration(const std::shared_ptr<AppExecFwk::Configuration>& configuration)
{
    if (mainWindow_ == nullptr) {
        WLOGFE("Update configuration failed, because main window is null");
        return;
    }
    WLOGFI("notify mainWindow winId:%{public}u", mainWindow_->GetWindowId());
    mainWindow_->UpdateConfiguration(configuration);
}

void WindowScene::UpdateConfigurationForSpecified(const std::shared_ptr<AppExecFwk::Configuration>& configuration,
    const std::shared_ptr<Global::Resource::ResourceManager>& resourceManager)
{
    if (mainWindow_ == nullptr) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "main window is null");
        return;
    }
    mainWindow_->UpdateConfigurationForSpecified(configuration, resourceManager);
}
} // namespace Rosen
} // namespace OHOS
