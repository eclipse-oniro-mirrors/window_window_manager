/*
 * Copyright (c) 2022-2025 Huawei Device Co., Ltd.
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

#include "displaymanageripc_fuzzer.h"

#include <iremote_broker.h>
#include <iservice_registry.h>
#include <securec.h>
#include <system_ability_definition.h>

#include "display_manager_interface_code.h"
#include "idisplay_manager.h"
#include "window_manager_hilog.h"

namespace OHOS ::Rosen {
namespace {
constexpr HiviewDFX::HiLogLabel LABEL = {LOG_CORE, HILOG_DOMAIN_DISPLAY, "DisplayManagerIPC_Fuzzer"};
}
template<class T>
size_t GetObject(T &object, const uint8_t *data, size_t size)
{
    size_t objectSize = sizeof(object);
    if (objectSize > size) {
        return 0;
    }
    return memcpy_s(&object, objectSize, data, objectSize) == EOK ? objectSize : 0;
}

std::pair<sptr<IDisplayManager>, sptr<IRemoteObject>> GetProxy()
{
    sptr<ISystemAbilityManager> systemAbilityManager =
        SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (!systemAbilityManager) {
        WLOGFE("Failed to get system ability mgr.");
        return { nullptr, nullptr };
    }
    sptr<IRemoteObject> remoteObject = systemAbilityManager->GetSystemAbility(DISPLAY_MANAGER_SERVICE_SA_ID);
    if (!remoteObject) {
        WLOGFE("Failed to get display manager service.");
        return { nullptr, nullptr };
    }
    sptr<IDisplayManager> displayManagerServiceProxy = iface_cast<IDisplayManager>(remoteObject);
    if ((!displayManagerServiceProxy) || (!displayManagerServiceProxy->AsObject())) {
        WLOGFE("Failed to get system display manager services");
        return { nullptr, nullptr };
    }
    return { displayManagerServiceProxy, remoteObject };
}

bool IPCFuzzTest(const uint8_t* data, size_t size)
{
    uint32_t code;
    int flags, waitTime;
    if (data == nullptr || size < sizeof(code) + sizeof(flags) + sizeof(waitTime)) {
        return false;
    }
    auto proxy = GetProxy();
    if (proxy.first == nullptr || proxy.second == nullptr) {
        return false;
    }
    size_t startPos = 0;
    startPos += GetObject<uint32_t>(code, data + startPos, size - startPos);
    startPos += GetObject<int>(flags, data + startPos, size - startPos);
    startPos += GetObject<int>(waitTime, data + startPos, size - startPos);
    MessageParcel sendData;
    MessageParcel reply;
    MessageOption option(flags, waitTime);
    uint32_t dataSize = (size - startPos) > 1024 * 1024 ? 1024 * 1024 : (size - startPos);
    sendData.WriteBuffer(data + startPos, dataSize);
    proxy.second->SendRequest(code, sendData, reply, option);
    return true;
}

void IPCSpecificInterfaceFuzzTest1(const sptr<IRemoteObject>& proxy, MessageParcel& sendData, MessageParcel& reply,
    MessageOption& option)
{
    for (uint32_t i = 0; i <= static_cast<uint32_t>(IDisplayManagerIpcCode::COMMAND_GET_SCREEN_BRIGHTNESS); i++) {
        proxy->SendRequest(i, sendData, reply, option);
    }
}

void IPCSpecificInterfaceFuzzTest2(const sptr<IRemoteObject>& proxy, MessageParcel& sendData, MessageParcel& reply,
    MessageOption& option, const sptr<IDisplayManager>& manager)
{
    int flags = option.GetFlags();
    option.SetFlags(MessageOption::TF_SYNC);
    proxy->SendRequest(static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_CREATE_VIRTUAL_SCREEN),
        sendData, reply, option);
    int32_t dmError;
    manager->DestroyVirtualScreen(reply.ReadUint64(), dmError);
    option.SetFlags(flags);
    proxy->SendRequest(static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_DESTROY_VIRTUAL_SCREEN),
        sendData, reply, option);
    proxy->SendRequest(
        static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_SET_VIRTUAL_SCREEN_SURFACE),
        sendData, reply, option);
    proxy->SendRequest(static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_GET_SCREEN_INFO_BY_ID),
        sendData, reply, option);
    proxy->SendRequest(
        static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_GET_SCREEN_GROUP_INFO_BY_ID),
        sendData, reply, option);
    proxy->SendRequest(static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_SET_SCREEN_ACTIVE_MODE),
        sendData, reply, option);
    proxy->SendRequest(static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_GET_ALL_SCREEN_INFOS),
        sendData, reply, option);
    proxy->SendRequest(static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_SET_ORIENTATION),
        sendData, reply, option);
    proxy->SendRequest(static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_SET_VIRTUAL_PIXEL_RATIO),
        sendData, reply, option);
    proxy->SendRequest(static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_SCREEN_MAKE_MIRROR),
        sendData, reply, option);
    proxy->SendRequest(static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_SCREEN_MAKE_EXPAND),
        sendData, reply, option);
    proxy->SendRequest(
        static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_REMOVE_VIRTUAL_SCREEN_FROM_SCREEN_GROUP),
        sendData, reply, option);
    proxy->SendRequest(
        static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_SCREEN_GET_SUPPORTED_COLOR_GAMUTS),
        sendData, reply, option);
    proxy->SendRequest(static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_SCREEN_GET_COLOR_GAMUT),
        sendData, reply, option);
    proxy->SendRequest(static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_SCREEN_SET_COLOR_GAMUT),
        sendData, reply, option);
    proxy->SendRequest(static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_SCREEN_GET_GAMUT_MAP),
        sendData, reply, option);
    proxy->SendRequest(static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_SCREEN_SET_GAMUT_MAP),
        sendData, reply, option);
    proxy->SendRequest(
        static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_SCREEN_SET_COLOR_TRANSFORM),
        sendData, reply, option);
}

bool IPCInterfaceFuzzTest(const uint8_t* data, size_t size)
{
    int flags, waitTime;
    if (data == nullptr || size < sizeof(flags) + sizeof(waitTime)) {
        return false;
    }
    auto proxy = GetProxy();
    if (proxy.first == nullptr || proxy.second == nullptr) {
        return false;
    }
    size_t startPos = 0;
    startPos += GetObject<int>(flags, data + startPos, size - startPos);
    startPos += GetObject<int>(waitTime, data + startPos, size - startPos);
    MessageParcel sendData;
    MessageParcel reply;
    MessageOption option(flags, waitTime);
    sendData.WriteInterfaceToken(IDisplayManager::GetDescriptor());
    uint32_t dataSize = (size - startPos) > 1024 * 1024 ? 1024 * 1024 : (size - startPos);
    sendData.WriteBuffer(data + startPos, dataSize);
    IPCSpecificInterfaceFuzzTest1(proxy.second, sendData, reply, option);
    IPCSpecificInterfaceFuzzTest2(proxy.second, sendData, reply, option, proxy.first);
    return true;
}
} // namespace.OHOS::Rosen

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    /* Run your code on data */
    OHOS::Rosen::IPCFuzzTest(data, size);
    OHOS::Rosen::IPCInterfaceFuzzTest(data, size);
    return 0;
}

