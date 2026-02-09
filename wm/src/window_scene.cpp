/*
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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

#include <ability_context.h>
#include <configuration.h>

#include "perform_reporter.h"
#include "singleton_container.h"
#include "static_call.h"
#include "window_manager.h"
#include "window_manager_hilog.h"
#include "wm_common_inner.h"

namespace OHOS {
namespace Rosen {
namespace {
const std::string MAIN_WINDOW_ID = "main window";
std::atomic<uint32_t> g_count { 0 };

std::string GenerateMainWindowName(const std::shared_ptr<AbilityRuntime::Context>& context)
{
    if (context == nullptr) {
        return MAIN_WINDOW_ID + std::to_string(g_count++);
    } else {
        std::string windowName = context->GetBundleName() + std::to_string(g_count++);
        std::size_t pos = windowName.find_last_of('.');
        return (pos == std::string::npos) ? windowName : windowName.substr(pos + 1); // skip '.'
    }
}
}

WindowScene::~WindowScene()
{
    TLOGI(WmsLogTag::WMS_MAIN, "winId %{public}u destructor!", mainWindowId_);
}

void WindowScene::OnLastStrongRef(const void *)
{
    if (auto mainWindow = GetMainWindow()) {
        mainWindow->Destroy();
    }
}

WMError WindowScene::Init(DisplayId displayId, const std::shared_ptr<AbilityRuntime::Context>& context,
    sptr<IWindowLifeCycle>& listener, sptr<WindowOption> option)
{
    TLOGI(WmsLogTag::WMS_MAIN, "WindowScene init with normal option!");
    if (option == nullptr) {
        option = sptr<WindowOption>::MakeSptr();
    }
    option->SetDisplayId(displayId);
    option->SetWindowTag(WindowTag::MAIN_WINDOW);
    if (context != nullptr) {
        option->SetBundleName(context->GetBundleName());
    }
    auto mainWindow = SingletonContainer::Get<StaticCall>().CreateWindow(
        GenerateMainWindowName(context), option, context);
    if (mainWindow == nullptr) {
        TLOGE(WmsLogTag::WMS_MAIN, "mainWindow is null after create Window!");
        return WMError::WM_ERROR_NULLPTR;
    }
    {
        std::lock_guard<std::mutex> lock(mainWindowMutex_);
        mainWindow_ = mainWindow;
    }
    mainWindowId_ = mainWindow->GetWindowId();
    mainWindow->RegisterLifeCycleListener(listener);

    // report when application startup request window
    SingletonContainer::Get<WindowInfoReporter>()
        .ReportStartWindow(option->GetBundleName(), mainWindow->GetWindowName());
    return WMError::WM_OK;
}

WMError WindowScene::Init(DisplayId displayId, const std::shared_ptr<AbilityRuntime::Context>& context,
    sptr<IWindowLifeCycle>& listener, sptr<WindowOption> option, const sptr<IRemoteObject>& iSession,
    const std::string& identityToken, bool isModuleAbilityHookEnd)
{
    TLOGI(WmsLogTag::WMS_MAIN, "WindowScene with window session!");
    if (option == nullptr || iSession == nullptr) {
        TLOGE(WmsLogTag::WMS_MAIN, "failed with option or iSession null!");
        return WMError::WM_ERROR_NULLPTR;
    }
    option->SetDisplayId(displayId);
    option->SetWindowName(GenerateMainWindowName(context));
    if (context != nullptr) {
        option->SetBundleName(context->GetBundleName());
        std::string moduleName = context->GetHapModuleInfo() ? context->GetHapModuleInfo()->moduleName : "";
        TLOGI(WmsLogTag::WMS_MAIN, "set isModuleAbilityHookEnd!");
        isModuleAbilityHookEnd =
            SingletonContainer::Get<WindowManager>().IsModuleHookOff(isModuleAbilityHookEnd, moduleName);
    }
    auto mainWindow = SingletonContainer::Get<StaticCall>()
        .CreateWindow(option, context, iSession, identityToken, isModuleAbilityHookEnd);
    if (mainWindow == nullptr) {
        TLOGE(WmsLogTag::WMS_MAIN, "mainWindow is null after create Window!");
        return WMError::WM_ERROR_NULLPTR;
    }
    {
        std::lock_guard<std::mutex> lock(mainWindowMutex_);
        mainWindow_ = mainWindow;
    }
    mainWindowId_ = mainWindow->GetWindowId();
    mainWindow->RegisterLifeCycleListener(listener);

    // report when application startup request window
    SingletonContainer::Get<WindowInfoReporter>()
        .ReportStartWindow(option->GetBundleName(), mainWindow->GetWindowName());
    return WMError::WM_OK;
}

sptr<Window> WindowScene::CreateWindow(const std::string& windowName, sptr<WindowOption>& option) const
{
    auto mainWindow = GetMainWindow();
    if (windowName.empty() || mainWindow == nullptr || option == nullptr) {
        TLOGE(WmsLogTag::WMS_MAIN, "new windowName: %{public}s", windowName.c_str());
        return nullptr;
    }
    option->SetParentId(mainWindow->GetWindowId());
    option->SetWindowTag(WindowTag::SUB_WINDOW);
    TLOGD(WmsLogTag::WMS_SUB, "windowName: %{public}s, parentId: %{public}u",
        windowName.c_str(), mainWindow->GetWindowId());
    return SingletonContainer::Get<StaticCall>().CreateWindow(windowName, option, mainWindow->GetContext());
}

sptr<Window> WindowScene::GetMainWindow() const
{
    std::lock_guard<std::mutex> lock(mainWindowMutex_);
    return mainWindow_;
}

std::vector<sptr<Window>> WindowScene::GetSubWindow()
{
    auto mainWindow = GetMainWindow();
    if (mainWindow == nullptr) {
        TLOGE(WmsLogTag::WMS_MAIN, "failed, because main window is null");
        return {};
    }
    uint32_t parentId = mainWindow->GetWindowId();
    return SingletonContainer::Get<StaticCall>().GetSubWindow(parentId);
}

WMError WindowScene::GoForeground(uint32_t reason)
{
    TLOGI(WmsLogTag::WMS_MAIN, "reason: %{public}u", reason);
    auto mainWindow = GetMainWindow();
    if (mainWindow == nullptr) {
        TLOGE(WmsLogTag::WMS_MAIN, "failed, because main window is null");
        return WMError::WM_ERROR_NULLPTR;
    }
    return mainWindow->Show(reason);
}

WMError WindowScene::GoResume()
{
    TLOGI(WmsLogTag::WMS_MAIN, "in");
    auto mainWindow = GetMainWindow();
    if (mainWindow == nullptr) {
        TLOGE(WmsLogTag::WMS_MAIN, "failed, because main window is null");
        return WMError::WM_ERROR_NULLPTR;
    }
    mainWindow->Resume();
    return WMError::WM_OK;
}

WMError WindowScene::GoPause()
{
    TLOGI(WmsLogTag::WMS_LIFE, "in");
    auto mainWindow = GetMainWindow();
    if (mainWindow == nullptr) {
        TLOGE(WmsLogTag::WMS_LIFE, "failed, because main window is null");
        return WMError::WM_ERROR_NULLPTR;
    }
    mainWindow->Pause();
    return WMError::WM_OK;
}

WMError WindowScene::GoBackground(uint32_t reason)
{
    TLOGI(WmsLogTag::WMS_MAIN, "reason: %{public}u", reason);
    auto mainWindow = GetMainWindow();
    if (mainWindow == nullptr) {
        TLOGE(WmsLogTag::WMS_MAIN, "failed, because main window is null");
        return WMError::WM_ERROR_NULLPTR;
    }
    return mainWindow->Hide(reason);
}

WMError WindowScene::GoDestroy(uint32_t reason)
{
    TLOGI(WmsLogTag::WMS_MAIN, "reason: %{public}u", reason);
    auto mainWindow = GetMainWindow();
    if (mainWindow == nullptr) {
        TLOGE(WmsLogTag::WMS_MAIN, "main window is null");
        return WMError::WM_ERROR_NULLPTR;
    }
    WMError ret = mainWindow->Destroy(reason);
    if (ret != WMError::WM_OK) {
        TLOGE(WmsLogTag::WMS_MAIN, "failed, name: %{public}s", mainWindow->GetWindowName().c_str());
        return ret;
    }

    std::lock_guard<std::mutex> lock(mainWindowMutex_);
    mainWindow_ = nullptr;
    return WMError::WM_OK;
}

WMError WindowScene::GoDestroyHookWindow()
{
    return GoDestroy(static_cast<uint32_t>(WindowStateChangeReason::ABILITY_HOOK));
}

WMError WindowScene::OnNewWant(const AAFwk::Want& want)
{
    auto mainWindow = GetMainWindow();
    if (mainWindow == nullptr) {
        TLOGE(WmsLogTag::WMS_MAIN, "failed, because main window is null");
        return WMError::WM_ERROR_NULLPTR;
    }
    mainWindow->OnNewWant(want);
    return WMError::WM_OK;
}

WMError WindowScene::SetSystemBarProperty(WindowType type, const SystemBarProperty& property) const
{
    auto mainWindow = GetMainWindow();
    if (mainWindow == nullptr) {
        TLOGE(WmsLogTag::WMS_MAIN, "failed, because main window is null");
        return WMError::WM_ERROR_NULLPTR;
    }
    return mainWindow->SetSystemBarProperty(type, property);
}

WMError WindowScene::RequestFocus() const
{
    auto mainWindow = GetMainWindow();
    if (mainWindow == nullptr) {
        TLOGE(WmsLogTag::WMS_MAIN, "failed, because main window is null");
        return WMError::WM_ERROR_NULLPTR;
    }
    return mainWindow->RequestFocus();
}

void WindowScene::UpdateConfiguration(const std::shared_ptr<AppExecFwk::Configuration>& configuration)
{
    if (auto mainWindow = GetMainWindow()) {
        TLOGI(WmsLogTag::WMS_MAIN, "winId: %{public}u", mainWindow->GetWindowId());
        mainWindow->UpdateConfiguration(configuration);
    } else {
        TLOGE(WmsLogTag::WMS_MAIN, "failed, because main window is null");
    }
}

void WindowScene::UpdateConfigurationForSpecified(const std::shared_ptr<AppExecFwk::Configuration>& configuration,
    const std::shared_ptr<Global::Resource::ResourceManager>& resourceManager)
{
    if (auto mainWindow = GetMainWindow()) {
        TLOGI(WmsLogTag::WMS_ATTRIBUTE, "winId: %{public}u", mainWindow->GetWindowId());
        mainWindow->UpdateConfigurationForSpecified(configuration, resourceManager);
    } else {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "main window is null");
    }
}

std::string WindowScene::GetContentInfo(BackupAndRestoreType type) const
{
    auto mainWindow = GetMainWindow();
    if (mainWindow == nullptr) {
        TLOGE(WmsLogTag::WMS_MAIN, "failed, because main window is null");
        return "";
    }
    return mainWindow->GetContentInfo(type);
}

WMError WindowScene::NotifyMemoryLevel(int32_t level)
{
    auto mainWindow = GetMainWindow();
    if (mainWindow == nullptr) {
        TLOGE(WmsLogTag::WMS_MAIN, "failed, because main window is null");
        return WMError::WM_ERROR_NULLPTR;
    }
    TLOGI(WmsLogTag::WMS_MAIN, "level: %{public}d", level);
    return mainWindow->NotifyMemoryLevel(level);
}

WMError WindowScene::SetNavDestinationInfo(const std::string& navDestinationInfo)
{
    auto mainWindow = GetMainWindow();
    if (mainWindow == nullptr) {
        TLOGE(WmsLogTag::WMS_LIFE, "failed, because main window is null");
        return WMError::WM_ERROR_NULLPTR;
    }
    mainWindow->SetNavDestinationInfo(navDestinationInfo);
    return WMError::WM_OK;
}

WMError WindowScene::SetHookedWindowElementInfo(const AppExecFwk::ElementName& elementName)
{
    auto mainWindow  = GetMainWindow();
    if (mainWindow == nullptr) {
        TLOGE(WmsLogTag::WMS_MAIN, "failed, because main window is null");
        return WMError::WM_ERROR_NULLPTR;
    }
    TLOGI(WmsLogTag::WMS_MAIN, "set hooked window element info");
    return mainWindow->SetHookTargetElementInfo(elementName);
}
} // namespace Rosen
} // namespace OHOS
