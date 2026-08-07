/*
 * Copyright (c) 2022-2024 Huawei Device Co., Ltd.
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

#include "window_impl.h"
#include "window_manager_hilog.h"

namespace OHOS {
namespace Rosen {
namespace {
constexpr HiviewDFX::HiLogLabel LABEL = {LOG_CORE, HILOG_DOMAIN_WINDOW, "Window"};
}
sptr<Window> Window::Create(const std::string& windowName, sptr<WindowOption>& option,
    const std::shared_ptr<OHOS::AbilityRuntime::Context>& context, WMError& errCode,
    const std::shared_ptr<RSUIContext>& rsUIContext)
{
    std::string errMsg;
    return Create(windowName, option, errMsg, context, errCode, rsUiContext);
}

sptr<Window> Window::Create(const std::string& windowName, sptr<WindowOption>& option, std::string& errMsg,
    const std::shared_ptr<OHOS::AbilityRuntime::Context>& context, WMError& errCode,
    const std::shared_ptr<RSUIContext>& rsUIContext)
{
    if (option == nullptr) {
        option = new(std::nothrow) WindowOption();
        if (option == nullptr) {
            WLOGFE("alloc WindowOption failed");
            return nullptr;
        }
    }
    sptr<WindowImpl> windowImpl = new(std::nothrow) WindowImpl(option);
    if (windowImpl == nullptr) {
        WLOGFE("alloc WindowImpl failed");
        return nullptr;
    }
    WMError error = windowImpl->Create(option->GetParentId(), context);
    if (error != WMError::WM_OK) {
        WLOGFE("error unequal to WMError::WM_OK");
        errCode = error;
        return nullptr;
    }
    return windowImpl;
}

sptr<Window> Window::Find(const std::string& windowName)
{
    return nullptr;
}

std::vector<sptr<Window>> Window::GetSubWindow(uint32_t parentId)
{
    return std::vector<sptr<Window>>();
}

sptr<Window> Window::GetTopWindowWithId(uint32_t mainWinId)
{
    return WindowImpl::GetTopWindowWithId(mainWinId);
}

sptr<Window> Window::GetTopWindowWithContext(const std::shared_ptr<AbilityRuntime::Context>& context)
{
    return nullptr;
}

void Window::UpdateConfigurationForAll(const std::shared_ptr<AppExecFwk::Configuration>& configuration,
    const std::vector<std::shared_ptr<AbilityRuntime::Context>>& ignoreWindowContexts)
{
    return WindowImpl::UpdateConfigurationForAll(configuration, ignoreWindowContexts);
}

void Window::UpdateConfigurationSyncForAll(const std::shared_ptr<AppExecFwk::Configuration>& configuration)
{
    return WindowImpl::UpdateConfigurationSyncForAll(configuration);
}

bool OccupiedAreaChangeInfo::Marshalling(Parcel& parcel) const
{
    return true;
}

OccupiedAreaChangeInfo* OccupiedAreaChangeInfo::Unmarshalling(Parcel& parcel)
{
    return nullptr;
}
} // namespace Rosen
} // namespace OHOS
