/*
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
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

#include "modal_system_ui_extension.h"

#include <memory>

#include <ability_manager_client.h>
#include <ffrt.h>
#include <iremote_object.h>
#include <message_parcel.h>

#include "common/include/task_scheduler.h"
#include "perform_reporter.h"
#include "singleton_container.h"
#include "window_manager_hilog.h"

using namespace OHOS::AAFwk;

namespace OHOS {
namespace Rosen {
namespace {
constexpr int32_t INVALID_USERID = -1;
constexpr int32_t MESSAGE_PARCEL_KEY_SIZE = 5;
constexpr int32_t VALUE_TYPE_STRING = 9;
constexpr uint64_t DISCONNECT_ABILITY_DELAY_TIME_MICROSECONDS = 5000000;
} // namespace

ModalSystemUiExtension::ModalSystemUiExtension() {}

ModalSystemUiExtension::~ModalSystemUiExtension()
{
    dialogConnectionCallback_ = nullptr;
}

bool ModalSystemUiExtension::CreateModalUIExtension(const AAFwk::Want& want)
{
    return CreateModalUIExtension(want, INVALID_USERID);
}

bool ModalSystemUiExtension::CreateModalUIExtension(const AAFwk::Want& want, const int32_t userId)
{
    auto abilityManagerClient = AbilityManagerClient::GetInstance();
    if (abilityManagerClient == nullptr) {
        TLOGE(WmsLogTag::WMS_UIEXT, "AbilityManagerClient is nullptr");
        return false;
    }

    AAFwk::Want systemUIWant;
    systemUIWant.SetElementName("com.ohos.sceneboard", "com.ohos.sceneboard.systemdialog");
    dialogConnectionCallback_ = sptr<DialogAbilityConnection>::MakeSptr(want);
    auto result = abilityManagerClient->ConnectAbility(systemUIWant, dialogConnectionCallback_, userId);
    if (result != ERR_OK) {
        TLOGE(WmsLogTag::WMS_UIEXT, "ConnectAbility failed, result=%{public}d", result);
        return false;
    }
    TLOGI(WmsLogTag::WMS_UIEXT, "ConnectAbility success");
    return true;
}

ModalSystemUiExtension::DialogAbilityConnection::DialogAbilityConnection(const AAFwk::Want& want) : want_(want)
{
    taskScheduler_ = std::make_shared<TaskScheduler>("OS_ModalSystemUiExtension");
}

std::string ModalSystemUiExtension::DialogAbilityConnection::ToString(const AAFwk::Want& want)
{
    nlohmann::json wantParamsJson;
    AAFwk::to_json(wantParamsJson, want.GetParams());
    bool needReport = false;
    for (auto it = wantParamsJson.begin(); it != wantParamsJson.end(); ++it) {
        if (!(it->is_string())) {
            continue;
        }
        std::string str = it->get<std::string>();
        if (!str.empty() && str[0] == '{') {
            auto parsed = nlohmann::json::parse(str, nullptr, false);
            if (parsed.is_discarded()) {
                TLOGI(WmsLogTag::WMS_UIEXT, "json parse failed");
            } else {
                it.value() = parsed;
                needReport = true;
            }
        }
    }
    if (needReport) {
        ReportJsonStringParamsUsage(want.GetElement().GetBundleName(), want.GetElement().GetAbilityName());
    }
    return wantParamsJson.dump();
}

void ModalSystemUiExtension::DialogAbilityConnection::ReportJsonStringParamsUsage(const std::string& bundleName,
                                                                                  const std::string& abilityName)
{
    TLOGI(WmsLogTag::WMS_UIEXT, "Report json string params usage, bundleName:%{public}s, abilityName:%{public}s",
        bundleName.c_str(), abilityName.c_str());
    taskScheduler_->PostAsyncTask([bundleName, abilityName]() {
        std::ostringstream oss;
        oss << "The uiextensionAbility is started by the modal system and uses the json string to transfer parameters";
        oss << ", bundleName: " << bundleName;
        oss << ", abilityName: " << abilityName;
        SingletonContainer::Get<WindowInfoReporter>().ReportWindowException(
            static_cast<int32_t>(WindowDFXHelperType::WINDOW_MODAL_SYSTEM_USE_JSONSTRING), getpid(), oss.str());
    }, __func__);
}

bool ModalSystemUiExtension::DialogAbilityConnection::SendWant(const sptr<IRemoteObject>& remoteObject)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    if (!data.WriteInt32(MESSAGE_PARCEL_KEY_SIZE)) {
        TLOGE(WmsLogTag::WMS_UIEXT, "write message parcel key size failed");
        return false;
    }
    if (!data.WriteString16(u"bundleName") || !data.WriteString16(Str8ToStr16(want_.GetElement().GetBundleName()))) {
        TLOGE(WmsLogTag::WMS_UIEXT, "write bundleName failed");
        return false;
    }
    if (!data.WriteString16(u"abilityName") || !data.WriteString16(Str8ToStr16(want_.GetElement().GetAbilityName()))) {
        TLOGE(WmsLogTag::WMS_UIEXT, "write abilityName failed");
        return false;
    }
    if (!data.WriteString16(u"parameters") ||
        !data.WriteString16(Str8ToStr16(ToString(want_)))) {
        TLOGE(WmsLogTag::WMS_UIEXT, "write parameters failed");
        return false;
    }
    if (!data.WriteString16(u"uri") || !data.WriteString16(Str8ToStr16(want_.GetUri().ToString()))) {
        TLOGE(WmsLogTag::WMS_UIEXT, "write uri failed");
        return false;
    }
    if (!data.WriteString16(u"flags") || !data.WriteUint32(want_.GetFlags())) {
        TLOGE(WmsLogTag::WMS_UIEXT, "write flags failed");
        return false;
    }
    int32_t ret = remoteObject->SendRequest(AAFwk::IAbilityConnection::ON_ABILITY_CONNECT_DONE, data, reply, option);
    if (ret != ERR_OK) {
        TLOGE(WmsLogTag::WMS_UIEXT, "show dialog failed");
        return false;
    }
    return true;
}

void ModalSystemUiExtension::DialogAbilityConnection::OnAbilityConnectDone(
    const AppExecFwk::ElementName& element, const sptr<IRemoteObject>& remoteObject, int resultCode)
{
    TLOGI(WmsLogTag::WMS_UIEXT, "called");
    if (remoteObject == nullptr) {
        TLOGE(WmsLogTag::WMS_UIEXT, "remoteObject is nullptr");
        return;
    }
    if (!SendWant(remoteObject)) {
        return;
    }
    auto task = [weakThis = wptr(this)] {
        auto connection = weakThis.promote();
        if (!connection) {
            TLOGI(WmsLogTag::WMS_UIEXT, "session is null or already disconnected");
            return;
        }
        auto abilityManagerClient = AbilityManagerClient::GetInstance();
        if (abilityManagerClient == nullptr) {
            TLOGE(WmsLogTag::WMS_UIEXT, "AbilityManagerClient is nullptr");
            return;
        }
        auto result = abilityManagerClient->DisconnectAbility(connection);
        if (result != ERR_OK) {
            TLOGE(WmsLogTag::WMS_UIEXT, "DisconnectAbility failed, result=%{public}d", result);
        } else {
            TLOGI(WmsLogTag::WMS_UIEXT, "DisconnectAbility success");
        }
    };
    ffrt::task_handle handle = ffrt::submit_h(std::move(task),
        ffrt::task_attr().delay(DISCONNECT_ABILITY_DELAY_TIME_MICROSECONDS));
    if (handle == nullptr) {
        TLOGE(WmsLogTag::WMS_UIEXT, "Failed to post task");
    }
}

void ModalSystemUiExtension::DialogAbilityConnection::OnAbilityDisconnectDone(const AppExecFwk::ElementName& element,
    int resultCode)
{
    TLOGI(WmsLogTag::WMS_UIEXT, "called");
}
} // namespace Rosen
} // namespace OHOS