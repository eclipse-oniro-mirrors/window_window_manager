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

#include "modal_system_ui_extension.h"

#include <atomic>
#include <memory>

#include <message_parcel.h>
#include <ability_manager_client.h>
#include <iremote_object.h>

#include "window_manager_hilog.h"

using namespace OHOS::AAFwk;

namespace OHOS {
namespace Rosen {
namespace {
constexpr int32_t INVALID_USERID = -1;
constexpr int32_t MESSAGE_PARCEL_KEY_SIZE = 3;
constexpr int32_t VALUE_TYPE_STRING = 9;
std::atomic_bool g_isDialogShow = false;
sptr<IRemoteObject> g_remoteObject = nullptr;
} // namespace

ModalSystemUiExtension::ModalSystemUiExtension() {}

ModalSystemUiExtension::~ModalSystemUiExtension()
{
    dialogConnectionCallback_ = nullptr;
}

bool ModalSystemUiExtension::CreateModalUIExtension(const AAFwk::Want& want)
{
    dialogConnectionCallback_ = sptr<OHOS::AAFwk::IAbilityConnection>(new DialogAbilityConnection(want));
    if (g_isDialogShow) {
        AppExecFwk::ElementName element;
        dialogConnectionCallback_->OnAbilityConnectDone(element, g_remoteObject, INVALID_USERID);
        TLOGI(WmsLogTag::WMS_UIEXT, "dialog has been shown");
        return true;
    }

    auto abilityManagerClient = AbilityManagerClient::GetInstance();
    if (abilityManagerClient == nullptr) {
        TLOGE(WmsLogTag::WMS_UIEXT, "AbilityManagerClient is nullptr");
        return false;
    }

    AAFwk::Want systemUIWant;
    systemUIWant.SetElementName("com.ohos.sceneboard", "com.ohos.sceneboard.systemdialog");
    auto result = abilityManagerClient->ConnectAbility(systemUIWant, dialogConnectionCallback_, INVALID_USERID);
    if (result != ERR_OK) {
        TLOGE(WmsLogTag::WMS_UIEXT, "ConnectAbility failed, result = %{public}d", result);
        return false;
    }
    TLOGI(WmsLogTag::WMS_UIEXT, "ConnectAbility success");
    return true;
}

std::string ModalSystemUiExtension::ToString(const AAFwk::WantParams& wantParams_)
{
    std::string result;
    if (wantParams_.Size() != 0) {
        result += "{";
        for (auto it: wantParams_.GetParams()) {
            int typeId = AAFwk::WantParams::GetDataType(it.second);
            result += "\"" + it.first + "\":";
            if (typeId == VALUE_TYPE_STRING && AAFwk::WantParams::GetStringByType(it.second, typeId)[0] != '{') {
                result += "\"" + AAFwk::WantParams::GetStringByType(it.second, typeId) + "\"";
            } else {
                result += AAFwk::WantParams::GetStringByType(it.second, typeId);
            }
            if (it != *wantParams_.GetParams().rbegin()) {
                result += ",";
            }
        }
        result += "}";
    } else {
        result += "{}";
    }
    return result;
}

void ModalSystemUiExtension::DialogAbilityConnection::OnAbilityConnectDone(
    const AppExecFwk::ElementName& element, const sptr<IRemoteObject>& remoteObject, int resultCode)
{
    TLOGI(WmsLogTag::WMS_UIEXT, "called");
    std::lock_guard lock(mutex_);
    if (remoteObject == nullptr) {
        TLOGE(WmsLogTag::WMS_UIEXT, "remoteObject is nullptr");
        return;
    }
    if (g_remoteObject == nullptr) {
        g_remoteObject = remoteObject;
    }
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    if (!data.WriteInt32(MESSAGE_PARCEL_KEY_SIZE)) {
        TLOGE(WmsLogTag::WMS_UIEXT, "write message parcel key size failed");
        return;
    }
    if (!data.WriteString16(u"bundleName") || !data.WriteString16(Str8ToStr16(want_.GetElement().GetBundleName()))) {
        TLOGE(WmsLogTag::WMS_UIEXT, "write bundleName failed");
        return;
    }
    if (!data.WriteString16(u"abilityName") || !data.WriteString16(Str8ToStr16(want_.GetElement().GetAbilityName()))) {
        TLOGE(WmsLogTag::WMS_UIEXT, "write abilityName failed");
        return;
    }
    if (!data.WriteString16(u"parameters") ||
        !data.WriteString16(Str8ToStr16(ModalSystemUiExtension::ToString(want_.GetParams())))) {
        TLOGE(WmsLogTag::WMS_UIEXT, "write parameters failed");
        return;
    }
    int32_t ret = remoteObject->SendRequest(AAFwk::IAbilityConnection::ON_ABILITY_CONNECT_DONE, data, reply, option);
    if (ret != ERR_OK) {
        TLOGE(WmsLogTag::WMS_UIEXT, "show dialog failed");
        return;
    }
    g_isDialogShow = true;
}

void ModalSystemUiExtension::DialogAbilityConnection::OnAbilityDisconnectDone(const AppExecFwk::ElementName& element,
    int resultCode)
{
    TLOGI(WmsLogTag::WMS_UIEXT, "called");
    std::lock_guard lock(mutex_);
    g_isDialogShow = false;
    g_remoteObject = nullptr;
}
} // namespace Rosen
} // namespace OHOS