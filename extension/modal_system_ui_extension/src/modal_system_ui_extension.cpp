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

std::string ModalSystemUiExtension::ToString(const AAFwk::WantParams& wantParams)
{
    std::string result;
    if (wantParams.Size() != 0) {
        result += "{";
        for (auto it : wantParams.GetParams()) {
            int typeId = AAFwk::WantParams::GetDataType(it.second);
            result += "\"" + it.first + "\":";
            if (typeId == VALUE_TYPE_STRING && AAFwk::WantParams::GetStringByType(it.second, typeId)[0] != '{') {
                result += "\"" + AAFwk::WantParams::GetStringByType(it.second, typeId) + "\"";
            } else {
                result += AAFwk::WantParams::GetStringByType(it.second, typeId);
            }
            if (it != *wantParams.GetParams().rbegin()) {
                result += ",";
            }
        }
        result += "}";
    } else {
        result += "{}";
    }
    return result;
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
        !data.WriteString16(Str8ToStr16(ModalSystemUiExtension::ToString(want_.GetParams())))) {
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