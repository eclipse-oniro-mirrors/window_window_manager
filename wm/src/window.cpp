/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#include "window.h"

#include "root_scene.h"
#include "scene_board_judgement.h"
#include "session/host/include/zidl/session_interface.h"
#include "window_adapter.h"
#include "window_helper.h"
#include "window_impl.h"
#include "window_session_impl.h"
#include "window_scene_session_impl.h"
#include "window_extension_session_impl.h"
#include "window_manager_hilog.h"
#include "wm_common.h"
#include "floating_ball_template_info.h"

namespace OHOS {
namespace Rosen {
namespace {
constexpr HiviewDFX::HiLogLabel LABEL = {LOG_CORE, HILOG_DOMAIN_WINDOW, "Window"};
}

static sptr<Window> CreateWindowWithSession(sptr<WindowOption>& option,
    const std::shared_ptr<OHOS::AbilityRuntime::Context>& context, WMError& errCode,
    sptr<ISession> iSession = nullptr, const std::string& identityToken = "", bool isModuleAbilityHookEnd = false,
    const std::shared_ptr<RSUIContext>& rsUiContext = nullptr)
{
    WLOGFD("in");
    sptr<WindowSessionImpl> windowSessionImpl = nullptr;
    auto sessionType = option->GetWindowSessionType();
    if (sessionType == WindowSessionType::SCENE_SESSION) {
        windowSessionImpl = sptr<WindowSceneSessionImpl>::MakeSptr(option, rsUiContext);
    } else if (sessionType == WindowSessionType::EXTENSION_SESSION) {
        option->SetWindowType(WindowType::WINDOW_TYPE_UI_EXTENSION);
        windowSessionImpl = sptr<WindowExtensionSessionImpl>::MakeSptr(option);
    }
    if (windowSessionImpl == nullptr) {
        WLOGFE("malloc windowSessionImpl failed");
        return nullptr;
    }

    windowSessionImpl->SetWindowType(option->GetWindowType());
    WMError error = windowSessionImpl->Create(context, iSession, identityToken, isModuleAbilityHookEnd,
        option->IsBlockSubwindow());
    if (error != WMError::WM_OK) {
        errCode = error;
        WLOGFE("error: %{public}u", static_cast<uint32_t>(errCode));
        return nullptr;
    }
    return windowSessionImpl;
}

sptr<Window> Window::Create(const std::string& windowName, sptr<WindowOption>& option,
    const std::shared_ptr<OHOS::AbilityRuntime::Context>& context, WMError& errCode,
    const std::shared_ptr<RSUIContext>& rsUiContext)
{
    if (windowName.empty()) {
        WLOGFE("window name is empty");
        return nullptr;
    }
    if (option == nullptr) {
        option = sptr<WindowOption>::MakeSptr();
    }
    uint32_t version = 10;
    if (context != nullptr && context->GetApplicationInfo() != nullptr) {
        version = context->GetApplicationInfo()->apiCompatibleVersion;
    }
    // 10 ArkUI new framework support after API10
    if (version < 10) {
        option->AddWindowFlag(WindowFlag::WINDOW_FLAG_NEED_AVOID);
    }
    WindowType type = option->GetWindowType();
    if (!(WindowHelper::IsAppWindow(type) || WindowHelper::IsSystemWindow(type))) {
        WLOGFE("window type is invalid %{public}d", type);
        return nullptr;
    }
    option->SetWindowName(windowName);
    if (SceneBoardJudgement::IsSceneBoardEnabled()) {
        return CreateWindowWithSession(option, context, errCode, nullptr, "", false, rsUiContext);
    }
    if (option->GetOnlySupportSceneBoard()) {
        errCode = WMError::WM_ERROR_DEVICE_NOT_SUPPORT;
        return nullptr;
    }
    sptr<WindowImpl> windowImpl = sptr<WindowImpl>::MakeSptr(option);
    if (windowImpl == nullptr) {
        WLOGFE("malloc windowImpl failed");
        return nullptr;
    }
    WMError error = windowImpl->Create(option->GetParentId(), context);
    if (error != WMError::WM_OK) {
        errCode = error;
        return nullptr;
    }
    return windowImpl;
}

sptr<Window> Window::Create(sptr<WindowOption>& option, const std::shared_ptr<OHOS::AbilityRuntime::Context>& context,
    const sptr<IRemoteObject>& iSession, WMError& errCode, const std::string& identityToken,
    bool isModuleAbilityHookEnd)
{
    // create from ability mgr service
    if (!iSession || !option) {
        WLOGFE("host window session is nullptr: %{public}u or option is null: %{public}u",
            iSession == nullptr, option == nullptr);
        return nullptr;
    }
    if (option->GetWindowName().empty()) {
        WLOGFE("window name in option is empty");
        return nullptr;
    }
    uint32_t version = 10;
    if ((context != nullptr) && (context->GetApplicationInfo() != nullptr)) {
        version = context->GetApplicationInfo()->apiCompatibleVersion;
    }
    // 10 ArkUI new framework support after API10
    if (version < 10) {
        option->AddWindowFlag(WindowFlag::WINDOW_FLAG_NEED_AVOID);
    }
    WindowType type = option->GetWindowType();
    if (!(WindowHelper::IsAppWindow(type) || WindowHelper::IsUIExtensionWindow(type) ||
          WindowHelper::IsAppComponentWindow(type))) {
        WLOGFE("window type is invalid %{public}d", type);
        return nullptr;
    }
    return CreateWindowWithSession(option, context, errCode,
        iface_cast<Rosen::ISession>(iSession), identityToken, isModuleAbilityHookEnd);
}

WMError Window::GetAndVerifyWindowTypeForArkUI(uint32_t parentId, const std::string& windowName,
    WindowType parentWindowType, WindowType& windowType)
{
    if (SceneBoardJudgement::IsSceneBoardEnabled()) {
        return WindowSceneSessionImpl::GetAndVerifyWindowTypeForArkUI(parentId, windowName,
            parentWindowType, windowType);
    } else {
        return WindowImpl::GetWindowTypeForArkUI(parentWindowType, windowType);
    }
}

sptr<Window> Window::CreatePiP(sptr<WindowOption>& option, const PiPTemplateInfo& pipTemplateInfo,
    const std::shared_ptr<OHOS::AbilityRuntime::Context>& context, WMError& errCode)
{
    if (!SceneBoardJudgement::IsSceneBoardEnabled()) {
        return nullptr;
    }
    if (!option) {
        TLOGE(WmsLogTag::WMS_PIP, "option is null.");
        return nullptr;
    }
    if (option->GetWindowName().empty()) {
        TLOGE(WmsLogTag::WMS_PIP, "the window name of option is empty.");
        return nullptr;
    }
    if (!WindowHelper::IsPipWindow(option->GetWindowType())) {
        TLOGE(WmsLogTag::WMS_PIP, "window type is not pip window.");
        return nullptr;
    }
    sptr<WindowSessionImpl> windowSessionImpl = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    if (windowSessionImpl == nullptr) {
        TLOGE(WmsLogTag::WMS_PIP, "malloc windowSessionImpl failed.");
        return nullptr;
    }
    windowSessionImpl->GetProperty()->SetPiPTemplateInfo(pipTemplateInfo);
    WMError error = windowSessionImpl->Create(context, nullptr);
    if (error != WMError::WM_OK) {
        errCode = error;
        TLOGW(WmsLogTag::WMS_PIP, "Create pip window, error: %{public}u", static_cast<uint32_t>(errCode));
        return nullptr;
    }
    return windowSessionImpl;
}

sptr<Window> Window::CreateFb(sptr<WindowOption>& option, const FloatingBallTemplateBaseInfo& fbTemplateBaseInfo,
    const std::shared_ptr<Media::PixelMap>& icon, const std::shared_ptr<OHOS::AbilityRuntime::Context>& context,
    WMError& errCode)
{
    if (!SceneBoardJudgement::IsSceneBoardEnabled()) {
        return nullptr;
    }
    if (!option) {
        TLOGE(WmsLogTag::WMS_SYSTEM, "option is null.");
        return nullptr;
    }
    if (option->GetWindowName().empty()) {
        TLOGE(WmsLogTag::WMS_SYSTEM, "the window name of option is empty.");
        return nullptr;
    }
    if (!WindowHelper::IsFbWindow(option->GetWindowType())) {
        TLOGE(WmsLogTag::WMS_SYSTEM, "window type is not fb window.");
        return nullptr;
    }
    sptr<WindowSessionImpl> windowSessionImpl = sptr<WindowSceneSessionImpl>::MakeSptr(option);
    if (windowSessionImpl == nullptr) {
        TLOGE(WmsLogTag::WMS_SYSTEM, "malloc windowSessionImpl failed.");
        return nullptr;
    }
    FloatingBallTemplateInfo fbTemplateInfo = FloatingBallTemplateInfo(fbTemplateBaseInfo, icon);
    windowSessionImpl->GetProperty()->SetFbTemplateInfo(fbTemplateInfo);
    WMError error = windowSessionImpl->Create(context, nullptr);
    if (error != WMError::WM_OK) {
        errCode = error;
        TLOGW(WmsLogTag::WMS_SYSTEM, "Create fb window, error: %{public}u", static_cast<uint32_t>(errCode));
        return nullptr;
    }
    return windowSessionImpl;
}

sptr<Window> Window::Find(const std::string& windowName)
{
    if (SceneBoardJudgement::IsSceneBoardEnabled()) {
        return WindowSessionImpl::Find(windowName);
    } else {
        return WindowImpl::Find(windowName);
    }
}

uint32_t Window::GetParentMainWindowId(uint32_t windowId)
{
    if (SceneBoardJudgement::IsSceneBoardEnabled()) {
        return static_cast<uint32_t>(WindowSceneSessionImpl::GetParentMainWindowId(windowId));
    } else {
        return INVALID_SESSION_ID;
    }
}

sptr<Window> Window::GetTopWindowWithContext(const std::shared_ptr<AbilityRuntime::Context>& context)
{
    if (SceneBoardJudgement::IsSceneBoardEnabled()) {
        return WindowSceneSessionImpl::GetTopWindowWithContext(context);
    } else {
        return WindowImpl::GetTopWindowWithContext(context);
    }
}

sptr<Window> Window::GetTopWindowWithId(uint32_t mainWinId)
{
    if (SceneBoardJudgement::IsSceneBoardEnabled()) {
        return WindowSceneSessionImpl::GetTopWindowWithId(mainWinId);
    } else {
        return WindowImpl::GetTopWindowWithId(mainWinId);
    }
}

sptr<Window> Window::GetWindowWithId(uint32_t windId)
{
    if (SceneBoardJudgement::IsSceneBoardEnabled()) {
        return WindowSceneSessionImpl::GetWindowWithId(windId);
    } else {
        return WindowImpl::GetWindowWithId(windId);
    }
}

sptr<Window> Window::GetMainWindowWithContext(const std::shared_ptr<AbilityRuntime::Context>& context)
{
    if (SceneBoardJudgement::IsSceneBoardEnabled()) {
        return WindowSceneSessionImpl::GetMainWindowWithContext(context);
    } else {
        return nullptr;
    }
}

std::vector<sptr<Window>> Window::GetSubWindow(uint32_t parentId)
{
    if (SceneBoardJudgement::IsSceneBoardEnabled()) {
        return WindowSessionImpl::GetSubWindow(parentId);
    } else {
        return WindowImpl::GetSubWindow(parentId);
    }
}

void Window::UpdateConfigurationForAll(const std::shared_ptr<AppExecFwk::Configuration>& configuration,
    const std::vector<std::shared_ptr<AbilityRuntime::Context>>& ignoreWindowContexts)
{
    if (SceneBoardJudgement::IsSceneBoardEnabled()) {
        WindowSceneSessionImpl::UpdateConfigurationForAll(configuration, ignoreWindowContexts);
        RootScene::UpdateConfigurationForAll(configuration, ignoreWindowContexts);
        WindowExtensionSessionImpl::UpdateConfigurationForAll(configuration, ignoreWindowContexts);
    } else {
        WindowImpl::UpdateConfigurationForAll(configuration, ignoreWindowContexts);
    }
}

void Window::UpdateConfigurationSyncForAll(const std::shared_ptr<AppExecFwk::Configuration>& configuration)
{
    if (SceneBoardJudgement::IsSceneBoardEnabled()) {
        WindowSceneSessionImpl::UpdateConfigurationSyncForAll(configuration);
        RootScene::UpdateConfigurationSyncForAll(configuration);
        WindowExtensionSessionImpl::UpdateConfigurationSyncForAll(configuration);
    } else {
        WindowImpl::UpdateConfigurationSyncForAll(configuration);
    }
}
} // namespace Rosen
} // namespace OHOS
