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


#include <want.h>

#include "extension_window_impl.h"
#include "extension/extension_business_info.h"
#include "window_manager_hilog.h"
#include "wm_common.h"

namespace OHOS {
namespace Rosen {
namespace {
constexpr HiviewDFX::HiLogLabel LABEL = {LOG_CORE, HILOG_DOMAIN_WINDOW, "ExtensionWindowImpl"};
}

Rosen::ExtensionWindowImpl::ExtensionWindowImpl(sptr<Window> sessionImpl)
    : windowExtensionSessionImpl_(sessionImpl) {
}

ExtensionWindowImpl::~ExtensionWindowImpl() {}

WMError ExtensionWindowImpl::GetAvoidAreaByType(AvoidAreaType type, AvoidArea& avoidArea)
{
    WLOGI("GetAvoidAreaByType is called");
    return windowExtensionSessionImpl_->GetAvoidAreaByType(type, avoidArea);
}

sptr<Window> ExtensionWindowImpl::GetWindow()
{
    return windowExtensionSessionImpl_;
}

WMError ExtensionWindowImpl::HideNonSecureWindows(bool shouldHide)
{
    TLOGD(WmsLogTag::WMS_UIEXT, "HideNonSecureWindows is called");
    return windowExtensionSessionImpl_->HideNonSecureWindows(shouldHide);
}

WMError ExtensionWindowImpl::SetWaterMarkFlag(bool isEnable)
{
    TLOGD(WmsLogTag::WMS_UIEXT, "SetWaterMarkFlag is called");
    return windowExtensionSessionImpl_->SetWaterMarkFlag(isEnable);
}

WMError ExtensionWindowImpl::HidePrivacyContentForHost(bool needHide)
{
    TLOGD(WmsLogTag::WMS_UIEXT, "enter");
    return windowExtensionSessionImpl_->HidePrivacyContentForHost(needHide);
}

bool ExtensionWindowImpl::IsPcWindow() const
{
    TLOGI(WmsLogTag::WMS_UIEXT, "in");
    return windowExtensionSessionImpl_->IsPcWindow();
}

bool ExtensionWindowImpl::IsPcOrPadFreeMultiWindowMode() const
{
    TLOGI(WmsLogTag::WMS_UIEXT, "in");
    return windowExtensionSessionImpl_->IsPcOrPadFreeMultiWindowMode();
}

WMError ExtensionWindowImpl::OccupyEvents(int32_t eventFlags)
{
    TLOGI(WmsLogTag::WMS_UIEXT, "events: %{public}d", eventFlags);
    auto dataHandler = windowExtensionSessionImpl_->GetExtensionDataHandler();
    if (!dataHandler) {
        TLOGE(WmsLogTag::WMS_UIEXT, "null dataHandler");
        return WMError::WM_ERROR_NULLPTR;
    }

    AAFwk::Want want;
    want.SetParam("type", std::string("OccupyEvents"));
    want.SetParam("eventFlags", eventFlags);
    auto ret = dataHandler->SendDataSync(SubSystemId::ARKUI_UIEXT,
        static_cast<uint32_t>(Extension::Businesscode::OCCUPY_EVENTS), want);
    if (ret != DataHandlerErr::OK) {
        TLOGE(WmsLogTag::WMS_UIEXT, "send failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    return WMError::WM_OK;
}
} // namespace Rosen
} // namespace OHOS
