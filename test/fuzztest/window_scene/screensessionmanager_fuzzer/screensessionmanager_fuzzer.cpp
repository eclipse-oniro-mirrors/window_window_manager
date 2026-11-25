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

#include "screensessionmanager_fuzzer.h"

#include <iremote_broker.h>
#include <iservice_registry.h>
#include <system_ability_definition.h>

#include "session_manager.h"
#include "zidl/screen_session_manager_proxy.h"
#include "zidl/mock_session_manager_service_interface.h"
#include "window_manager_hilog.h"
#include "data_source.h"

namespace OHOS::Rosen {
namespace {
constexpr HiviewDFX::HiLogLabel LABEL = {LOG_CORE, HILOG_DOMAIN_WINDOW, "ScreenSessionManagerFuzzTest"};
}

using DMMessage = DisplayManagerMessage;

std::pair<sptr<IScreenSessionManager>, sptr<IRemoteObject>> GetProxy()
{
    sptr<ISystemAbilityManager> systemAbilityManager =
        SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (!systemAbilityManager) {
        WLOGFE("Failed to get system ability mgr");
        return {nullptr, nullptr};
    }

    sptr<IRemoteObject> remoteObject = systemAbilityManager->GetSystemAbility(DISPLAY_MANAGER_SERVICE_SA_ID);
    if (!remoteObject) {
        WLOGFE("Remote object is nullptr");
        return {nullptr, nullptr};
    }

    auto screenSessionManagerProxy = iface_cast<IScreenSessionManager>(remoteObject);
    if (!screenSessionManagerProxy) {
        WLOGFE("Get screen session manager proxy failed, nullptr");
        return {nullptr, nullptr};
    }

    WLOGFD("GetProxy success");

    return {screenSessionManagerProxy, remoteObject};
}

void IPCFuzzTest(const uint8_t* data, size_t size)
{
    auto [proxy, remoteObject] = GetProxy();
    if (!proxy || !remoteObject) {
        return;
    }

    DataSource source(data, size);
    uint32_t code = source.GetObject<uint32_t>();
    int flags = source.GetObject<int>();
    int waitTime = source.GetObject<int>();

    MessageParcel sendData;
    MessageParcel reply;
    MessageOption option(flags, waitTime);
    auto rawSize = source.size_ - source.pos_;
    auto buf = source.GetRaw(rawSize);
    if (buf) {
        sendData.WriteBuffer(buf, rawSize);
    }
    remoteObject->SendRequest(code, sendData, reply, option);
}

void IPCSpecificInterfaceFuzzTest1(sptr<IRemoteObject> proxy, MessageParcel& sendData, MessageParcel& reply,
    MessageOption& option)
{
    proxy->SendRequest(static_cast<uint32_t>(DMMessage::TRANS_ID_GET_DEFAULT_DISPLAY_INFO),
        sendData, reply, option);
    proxy->SendRequest(static_cast<uint32_t>(DMMessage::TRANS_ID_REGISTER_DISPLAY_MANAGER_AGENT),
        sendData, reply, option);
    proxy->SendRequest(static_cast<uint32_t>(DMMessage::TRANS_ID_UNREGISTER_DISPLAY_MANAGER_AGENT),
        sendData, reply, option);
    proxy->SendRequest(static_cast<uint32_t>(DMMessage::TRANS_ID_WAKE_UP_BEGIN),
        sendData, reply, option);
    proxy->SendRequest(static_cast<uint32_t>(DMMessage::TRANS_ID_WAKE_UP_END),
        sendData, reply, option);
    proxy->SendRequest(static_cast<uint32_t>(DMMessage::TRANS_ID_SUSPEND_BEGIN),
        sendData, reply, option);
    proxy->SendRequest(static_cast<uint32_t>(DMMessage::TRANS_ID_SUSPEND_END),
        sendData, reply, option);
    proxy->SendRequest(static_cast<uint32_t>(DMMessage::TRANS_ID_SET_DISPLAY_STATE),
        sendData, reply, option);
    proxy->SendRequest(static_cast<uint32_t>(DMMessage::TRANS_ID_SET_SCREEN_POWER_FOR_ALL),
        sendData, reply, option);
    proxy->SendRequest(static_cast<uint32_t>(DMMessage::TRANS_ID_GET_DISPLAY_STATE),
        sendData, reply, option);
    proxy->SendRequest(static_cast<uint32_t>(DMMessage::TRANS_ID_NOTIFY_DISPLAY_EVENT),
        sendData, reply, option);
    proxy->SendRequest(static_cast<uint32_t>(DMMessage::TRANS_ID_GET_SCREEN_POWER),
        sendData, reply, option);
    proxy->SendRequest(static_cast<uint32_t>(DMMessage::TRANS_ID_GET_DISPLAY_BY_ID),
        sendData, reply, option);
    proxy->SendRequest(static_cast<uint32_t>(DMMessage::TRANS_ID_GET_DISPLAY_BY_SCREEN),
        sendData, reply, option);
    proxy->SendRequest(static_cast<uint32_t>(DMMessage::TRANS_ID_GET_ALL_DISPLAYIDS),
        sendData, reply, option);
    proxy->SendRequest(static_cast<uint32_t>(DMMessage::TRANS_ID_GET_SCREEN_INFO_BY_ID),
        sendData, reply, option);
}

void IPCSpecificInterfaceFuzzTest2(sptr<IRemoteObject> proxy, MessageParcel& sendData, MessageParcel& reply,
    MessageOption& option)
{
    proxy->SendRequest(static_cast<uint32_t>(DMMessage::TRANS_ID_GET_ALL_SCREEN_INFOS),
        sendData, reply, option);
    proxy->SendRequest(static_cast<uint32_t>(DMMessage::TRANS_ID_SCREEN_GET_SUPPORTED_COLOR_GAMUTS),
        sendData, reply, option);
    proxy->SendRequest(static_cast<uint32_t>(DMMessage::TRANS_ID_CREATE_VIRTUAL_SCREEN),
        sendData, reply, option);
    proxy->SendRequest(static_cast<uint32_t>(DMMessage::TRANS_ID_SET_VIRTUAL_SCREEN_SURFACE),
        sendData, reply, option);
    proxy->SendRequest(static_cast<uint32_t>(DMMessage::TRANS_ID_DESTROY_VIRTUAL_SCREEN),
        sendData, reply, option);
    proxy->SendRequest(static_cast<uint32_t>(DMMessage::TRANS_ID_SCREEN_MAKE_MIRROR),
        sendData, reply, option);
    proxy->SendRequest(static_cast<uint32_t>(DMMessage::TRANS_ID_SCREEN_STOP_MIRROR),
        sendData, reply, option);
    proxy->SendRequest(static_cast<uint32_t>(DMMessage::TRANS_ID_SCREEN_MAKE_EXPAND),
        sendData, reply, option);
    proxy->SendRequest(static_cast<uint32_t>(DMMessage::TRANS_ID_SCREEN_STOP_EXPAND),
        sendData, reply, option);
    proxy->SendRequest(static_cast<uint32_t>(DMMessage::TRANS_ID_GET_SCREEN_GROUP_INFO_BY_ID),
        sendData, reply, option);
    proxy->SendRequest(static_cast<uint32_t>(DMMessage::TRANS_ID_REMOVE_VIRTUAL_SCREEN_FROM_SCREEN_GROUP),
        sendData, reply, option);
    proxy->SendRequest(static_cast<uint32_t>(DMMessage::TRANS_ID_GET_DISPLAY_SNAPSHOT),
        sendData, reply, option);
    proxy->SendRequest(static_cast<uint32_t>(DMMessage::TRANS_ID_SET_SCREEN_ACTIVE_MODE),
        sendData, reply, option);
    proxy->SendRequest(static_cast<uint32_t>(DMMessage::TRANS_ID_SET_VIRTUAL_PIXEL_RATIO),
        sendData, reply, option);
    proxy->SendRequest(static_cast<uint32_t>(DMMessage::TRANS_ID_SCREEN_GET_COLOR_GAMUT),
        sendData, reply, option);
    proxy->SendRequest(static_cast<uint32_t>(DMMessage::TRANS_ID_SCREEN_SET_COLOR_GAMUT),
        sendData, reply, option);
}

void IPCSpecificInterfaceFuzzTest3(sptr<IRemoteObject> proxy, MessageParcel& sendData, MessageParcel& reply,
    MessageOption& option)
{
    proxy->SendRequest(static_cast<uint32_t>(DMMessage::TRANS_ID_SCREEN_GET_GAMUT_MAP),
        sendData, reply, option);
    proxy->SendRequest(static_cast<uint32_t>(DMMessage::TRANS_ID_SCREEN_SET_GAMUT_MAP),
        sendData, reply, option);
    proxy->SendRequest(static_cast<uint32_t>(DMMessage::TRANS_ID_SCREEN_SET_COLOR_TRANSFORM),
        sendData, reply, option);
    proxy->SendRequest(static_cast<uint32_t>(DMMessage::TRANS_ID_SET_ORIENTATION),
        sendData, reply, option);
    proxy->SendRequest(static_cast<uint32_t>(DMMessage::TRANS_ID_SET_SCREEN_ROTATION_LOCKED),
        sendData, reply, option);
    proxy->SendRequest(static_cast<uint32_t>(DMMessage::TRANS_ID_IS_SCREEN_ROTATION_LOCKED),
        sendData, reply, option);
    proxy->SendRequest(static_cast<uint32_t>(DMMessage::TRANS_ID_GET_CUTOUT_INFO),
        sendData, reply, option);
    proxy->SendRequest(static_cast<uint32_t>(DMMessage::TRANS_ID_HAS_PRIVATE_WINDOW),
        sendData, reply, option);
    proxy->SendRequest(static_cast<uint32_t>(DMMessage::TRANS_ID_SCENE_BOARD_DUMP_ALL_SCREEN),
        sendData, reply, option);
    proxy->SendRequest(static_cast<uint32_t>(DMMessage::TRANS_ID_SCENE_BOARD_DUMP_SPECIAL_SCREEN),
        sendData, reply, option);
    proxy->SendRequest(static_cast<uint32_t>(DMMessage::TRANS_ID_SCENE_BOARD_SET_FOLD_DISPLAY_MODE),
        sendData, reply, option);
    proxy->SendRequest(static_cast<uint32_t>(DMMessage::TRANS_ID_SCENE_BOARD_GET_FOLD_DISPLAY_MODE),
        sendData, reply, option);
    proxy->SendRequest(static_cast<uint32_t>(DMMessage::TRANS_ID_SCENE_BOARD_IS_FOLDABLE),
        sendData, reply, option);
    proxy->SendRequest(static_cast<uint32_t>(DMMessage::TRANS_ID_SCENE_BOARD_GET_FOLD_STATUS),
        sendData, reply, option);
    proxy->SendRequest(static_cast<uint32_t>(DMMessage::TRANS_ID_SCENE_BOARD_GET_CURRENT_FOLD_CREASE_REGION),
        sendData, reply, option);
}

void IPCInterfaceFuzzTest(const uint8_t* data, size_t size)
{
    auto [proxy, remoteObject] = GetProxy();
    if (!proxy || !remoteObject) {
        return;
    }

    DataSource source(data, size);
    int flags = source.GetObject<int>();
    int waitTime = source.GetObject<int>();

    MessageParcel sendData;
    MessageParcel reply;
    MessageOption option(flags, waitTime);
    sendData.WriteInterfaceToken(proxy->GetDescriptor());
    auto rawSize = source.size_ - source.pos_;
    auto buf = source.GetRaw(rawSize);
    if (buf) {
        sendData.WriteBuffer(buf, rawSize);
    }
    IPCSpecificInterfaceFuzzTest1(remoteObject, sendData, reply, option);
    IPCSpecificInterfaceFuzzTest2(remoteObject, sendData, reply, option);
    IPCSpecificInterfaceFuzzTest3(remoteObject, sendData, reply, option);
}

void ProxyInterfaceFuzzTestPart1(const uint8_t* data, size_t size)
{
    auto [proxy, remoteObject] = GetProxy();
    if (!proxy || !remoteObject) {
        return;
    }

    DataSource source(data, size);

    proxy->GetDefaultDisplayInfo();

    PowerStateChangeReason reason = source.GetObject<PowerStateChangeReason>();
    proxy->WakeUpBegin(reason);
    proxy->WakeUpEnd();

    reason = source.GetObject<PowerStateChangeReason>();
    proxy->SuspendBegin(reason);
    proxy->SuspendEnd();

    DisplayState displayState = source.GetObject<DisplayState>();
    proxy->SetDisplayState(displayState);

    ScreenPowerState powerState = source.GetObject<ScreenPowerState>();
    reason = source.GetObject<PowerStateChangeReason>();
    proxy->SetScreenPowerForAll(powerState, reason);

    DisplayId displayId = source.GetObject<DisplayId>();
    proxy->GetDisplayState(displayId);

    DisplayEvent displayEvent = source.GetObject<DisplayEvent>();
    proxy->NotifyDisplayEvent(displayEvent);

    ScreenId dmsScreenId = source.GetObject<ScreenId>();
    proxy->GetScreenPower(dmsScreenId);
}

void ProxyInterfaceFuzzTestPart2(const uint8_t* data, size_t size)
{
    auto [proxy, remoteObject] = GetProxy();
    if (!proxy || !remoteObject) {
        return;
    }

    DataSource source(data, size);

    DisplayId displayId = source.GetObject<DisplayId>();
    proxy->GetDisplayInfoById(displayId);

    ScreenId dmsScreenId = source.GetObject<ScreenId>();
    proxy->GetDisplayInfoByScreen(dmsScreenId);
    proxy->GetAllDisplayIds();
    proxy->GetScreenInfoById(dmsScreenId);

    std::vector<sptr<ScreenInfo>> screenInfos;
    proxy->GetAllScreenInfos(screenInfos);

    dmsScreenId = source.GetObject<ScreenId>();
    std::vector<ScreenColorGamut> colorGamuts;
    proxy->GetScreenSupportedColorGamuts(dmsScreenId, colorGamuts);

    VirtualScreenOption screenOption;
    screenOption.name_ = source.GetString();
    screenOption.width_ = source.GetObject<uint32_t>();
    screenOption.height_ = source.GetObject<uint32_t>();
    screenOption.density_ = source.GetObject<float>();
    screenOption.flags_ = source.GetObject<int32_t>();
    screenOption.isForShot_ = source.GetObject<bool>();
    sptr<IRemoteObject> displayManagerAgent = nullptr;
    proxy->CreateVirtualScreen(screenOption, displayManagerAgent);
}

void ProxyInterfaceFuzzTestPart3(const uint8_t* data, size_t size)
{
    auto [proxy, remoteObject] = GetProxy();
    if (!proxy || !remoteObject) {
        return;
    }

    DataSource source(data, size);

    ScreenId screenId = source.GetObject<ScreenId>();
    sptr<IBufferProducer> bufferProducer = nullptr;
    proxy->SetVirtualScreenSurface(screenId, bufferProducer);
    proxy->DestroyVirtualScreen(screenId);

    ScreenId mainScreenId = source.GetObject<ScreenId>();
    std::vector<ScreenId> screenIds{
        source.GetObject<ScreenId>(), source.GetObject<ScreenId>(), source.GetObject<ScreenId>()};
    ScreenId screenGroupId;
    proxy->MakeMirror(mainScreenId, screenIds, screenGroupId);
    proxy->StopMirror(screenIds);

    std::vector<Point> startPoint{
        Point(source.GetObject<int32_t>(), source.GetObject<int32_t>()),
        Point(source.GetObject<int32_t>(), source.GetObject<int32_t>()),
        Point(source.GetObject<int32_t>(), source.GetObject<int32_t>())
    };
    proxy->MakeExpand(screenIds, startPoint, screenGroupId);
    proxy->StopExpand(screenIds);
    proxy->RemoveVirtualScreenFromGroup(screenIds);

    screenId = source.GetObject<ScreenId>();
    proxy->GetScreenGroupInfoById(screenId);

    DisplayId displayId = source.GetObject<DisplayId>();
    proxy->GetDisplaySnapshot(displayId, nullptr);

    screenId = source.GetObject<ScreenId>();
    uint32_t modeId = source.GetObject<uint32_t>();
    proxy->SetScreenActiveMode(screenId, modeId);
}

void ProxyInterfaceFuzzTestPart4(const uint8_t* data, size_t size)
{
    auto [proxy, remoteObject] = GetProxy();
    if (!proxy || !remoteObject) {
        return;
    }

    DataSource source(data, size);

    ScreenId screenId = source.GetObject<ScreenId>();
    float virtualPixelRatio = source.GetObject<float>();
    proxy->SetVirtualPixelRatio(screenId, virtualPixelRatio);

    screenId = source.GetObject<ScreenId>();
    ScreenColorGamut colorGamut;
    int32_t colorGamutIdx = source.GetObject<int32_t>();
    proxy->GetScreenColorGamut(screenId, colorGamut);
    proxy->SetScreenColorGamut(screenId, colorGamutIdx);

    screenId = source.GetObject<ScreenId>();
    ScreenGamutMap gamutMap = source.GetObject<ScreenGamutMap>();
    proxy->SetScreenGamutMap(screenId, gamutMap);
    proxy->GetScreenGamutMap(screenId, gamutMap);
    proxy->SetScreenColorTransform(screenId);

    screenId = source.GetObject<ScreenId>();
    Orientation orientation = source.GetObject<Orientation>();
    proxy->SetOrientation(screenId, orientation, false);

    bool isLocked = source.GetObject<bool>();
    proxy->SetScreenRotationLocked(isLocked);
    proxy->IsScreenRotationLocked(isLocked);

    DisplayId displayId = source.GetObject<DisplayId>();
    proxy->GetCutoutInfo(displayId);
}

void ProxyInterfaceFuzzTestPart5(const uint8_t* data, size_t size)
{
    auto [proxy, remoteObject] = GetProxy();
    if (!proxy || !remoteObject) {
        return;
    }

    DataSource source(data, size);

    DisplayId displayId = source.GetObject<DisplayId>();
    bool hasPrivateWindow;
    proxy->HasPrivateWindow(displayId, hasPrivateWindow);

    ScreenId screenId = source.GetObject<ScreenId>();
    std::string dumpInfo;
    proxy->DumpAllScreensInfo(dumpInfo);
    proxy->DumpSpecialScreenInfo(screenId, dumpInfo);

    FoldDisplayMode displayMode = source.GetObject<FoldDisplayMode>();
    proxy->SetFoldDisplayMode(displayMode);
    proxy->GetFoldDisplayMode();
    proxy->IsFoldable();
    proxy->GetFoldStatus();
    proxy->GetCurrentFoldCreaseRegion();
}

void ProxyInterfaceFuzzTest(const uint8_t* data, size_t size)
{
    ProxyInterfaceFuzzTestPart1(data, size);
    ProxyInterfaceFuzzTestPart2(data, size);
    ProxyInterfaceFuzzTestPart3(data, size);
    ProxyInterfaceFuzzTestPart4(data, size);
    ProxyInterfaceFuzzTestPart5(data, size);
}
}

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    /* Run your code on data */
    OHOS::Rosen::IPCFuzzTest(data, size);
    OHOS::Rosen::IPCInterfaceFuzzTest(data, size);
    OHOS::Rosen::ProxyInterfaceFuzzTest(data, size);
    return 0;
}
