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

#include "session/host/include/zidl/session_proxy.h"

#include "ability_start_setting.h"
#include <ipc_types.h>
#include <message_option.h>
#include <transaction/rs_transaction.h>
#include <ui/rs_surface_node.h>
#include <feature/window_keyframe/rs_window_keyframe_node.h>

#include "parcel/accessibility_event_info_parcel.h"
#include "process_options.h"
#include "start_window_option.h"
#include "want.h"
#include "key_event.h"
#include "pointer_event.h"
#include "process_options.h"
#include "rate_limited_logger.h"
#include "session/host/include/zidl/session_ipc_interface_code.h"
#include "window_manager_hilog.h"
namespace OHOS::Rosen {
namespace {
constexpr HiviewDFX::HiLogLabel LABEL = { LOG_CORE, HILOG_DOMAIN_WINDOW, "SessionProxy" };
constexpr int32_t MAX_IPC_CAPACITY_FOR_WANT = 216 * 1024;

bool WriteAbilitySessionInfoBasic(MessageParcel& data, sptr<AAFwk::SessionInfo> abilitySessionInfo)
{
    if (abilitySessionInfo == nullptr) {
        WLOGFE("abilitySessionInfo is null");
        return false;
    }
    if (data.GetMaxCapacity() < MAX_IPC_CAPACITY_FOR_WANT) {
        data.SetMaxCapacity(MAX_IPC_CAPACITY_FOR_WANT);
    }
    if (!data.WriteParcelable(&(abilitySessionInfo->want)) ||
        !data.WriteInt32(abilitySessionInfo->requestCode) ||
        !data.WriteInt32(abilitySessionInfo->persistentId) ||
        !data.WriteInt32(static_cast<uint32_t>(abilitySessionInfo->state)) ||
        !data.WriteInt64(abilitySessionInfo->uiAbilityId) ||
        !data.WriteInt32(abilitySessionInfo->callingTokenId) ||
        !data.WriteInt32(abilitySessionInfo->requestId) ||
        !data.WriteInt32(abilitySessionInfo->specifiedReason) ||
        !data.WriteBool(abilitySessionInfo->reuse) ||
        !data.WriteParcelable(abilitySessionInfo->processOptions.get())) {
        return false;
    }
    return true;
}
} // namespace

WSError SessionProxy::Foreground(
    sptr<WindowSessionProperty> property, bool isFromClient, const std::string& identityToken)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::WMS_LIFE, "WriteInterfaceToken failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }

    if (property) {
        if (!data.WriteBool(true) || !data.WriteParcelable(property.GetRefPtr())) {
            TLOGE(WmsLogTag::WMS_LIFE, "Write property failed");
            return WSError::WS_ERROR_IPC_FAILED;
        }
    } else {
        if (!data.WriteBool(false)) {
            TLOGE(WmsLogTag::WMS_LIFE, "Write property failed");
            return WSError::WS_ERROR_IPC_FAILED;
        }
    }
    if (!data.WriteBool(isFromClient)) {
        TLOGE(WmsLogTag::WMS_LIFE, "Write isFromClient failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    if (!data.WriteString(identityToken)) {
        TLOGE(WmsLogTag::WMS_LIFE, "Write identityToken failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::WMS_LIFE, "remote is null");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    int sendCode = remote->SendRequest(static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_FOREGROUND),
        data, reply, option);
    if (sendCode != ERR_NONE) {
        TLOGE(WmsLogTag::WMS_LIFE, "SendRequest failed, code: %{public}d", sendCode);
        return WSError::WS_ERROR_IPC_FAILED;
    }
    int32_t ret = reply.ReadInt32();
    return static_cast<WSError>(ret);
}

WSError SessionProxy::Background(bool isFromClient, const std::string& identityToken)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::WMS_LIFE, "WriteInterfaceToken failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    if (!data.WriteBool(isFromClient)) {
        TLOGE(WmsLogTag::WMS_LIFE, "Write isFromClient failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    if (!data.WriteString(identityToken)) {
        TLOGE(WmsLogTag::WMS_LIFE, "Write identityToken failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::WMS_LIFE, "remote is null");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    int sendCode = remote->SendRequest(static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_BACKGROUND),
        data, reply, option);
    if (sendCode != ERR_NONE) {
        TLOGE(WmsLogTag::WMS_LIFE, "SendRequest failed, code: %{public}d", sendCode);
        return WSError::WS_ERROR_IPC_FAILED;
    }
    int32_t ret = reply.ReadInt32();
    return static_cast<WSError>(ret);
}

WSError SessionProxy::Show(sptr<WindowSessionProperty> property)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        WLOGFE("WriteInterfaceToken failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }

    if (property) {
        if (!data.WriteBool(true) || !data.WriteParcelable(property.GetRefPtr())) {
            WLOGFE("Write property failed");
            return WSError::WS_ERROR_IPC_FAILED;
        }
    } else {
        if (!data.WriteBool(false)) {
            WLOGFE("Write property failed");
            return WSError::WS_ERROR_IPC_FAILED;
        }
    }

    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        WLOGFE("remote is null");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    if (remote->SendRequest(static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_SHOW),
        data, reply, option) != ERR_NONE) {
        WLOGFE("SendRequest failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    int32_t ret = reply.ReadInt32();
    return static_cast<WSError>(ret);
}

WSError SessionProxy::Hide()
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        WLOGFE("WriteInterfaceToken failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }

    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        WLOGFE("remote is null");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    if (remote->SendRequest(static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_HIDE),
        data, reply, option) != ERR_NONE) {
        WLOGFE("SendRequest failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    int32_t ret = reply.ReadInt32();
    return static_cast<WSError>(ret);
}

WSError SessionProxy::Disconnect(bool isFromClient, const std::string& identityToken)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::WMS_LIFE, "WriteInterfaceToken failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }

    if (!data.WriteBool(isFromClient)) {
        TLOGE(WmsLogTag::WMS_LIFE, "Write isFromClient failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    if (!data.WriteString(identityToken)) {
        TLOGE(WmsLogTag::WMS_LIFE, "Write identityToken failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::WMS_LIFE, "remote is null");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    int sendCode = remote->SendRequest(static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_DISCONNECT),
        data, reply, option);
    if (sendCode != ERR_NONE) {
        TLOGE(WmsLogTag::WMS_LIFE, "SendRequest failed, code: %{public}d", sendCode);
        return WSError::WS_ERROR_IPC_FAILED;
    }
    int32_t ret = reply.ReadInt32();
    return static_cast<WSError>(ret);
}

WSError SessionProxy::Connect(const sptr<ISessionStage>& sessionStage, const sptr<IWindowEventChannel>& eventChannel,
    const std::shared_ptr<RSSurfaceNode>& surfaceNode, SystemSessionConfig& systemConfig,
    sptr<WindowSessionProperty> property, sptr<IRemoteObject> token,
    const std::string& identityToken)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::WMS_LIFE, "WriteInterfaceToken failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    if (!data.WriteRemoteObject(sessionStage->AsObject())) {
        TLOGE(WmsLogTag::WMS_LIFE, "Write ISessionStage failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    if (!data.WriteRemoteObject(eventChannel->AsObject())) {
        TLOGE(WmsLogTag::WMS_LIFE, "Write IWindowEventChannel failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    if (!surfaceNode || !surfaceNode->Marshalling(data)) {
        TLOGE(WmsLogTag::WMS_LIFE, "Write surfaceNode failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    if (property) {
        if (!data.WriteBool(true) || !data.WriteParcelable(property.GetRefPtr())) {
            TLOGE(WmsLogTag::WMS_LIFE, "Write property failed");
            return WSError::WS_ERROR_IPC_FAILED;
        }
    } else {
        if (!data.WriteBool(false)) {
            TLOGE(WmsLogTag::WMS_LIFE, "Write property failed");
            return WSError::WS_ERROR_IPC_FAILED;
        }
    }
    if (token != nullptr) {
        if (!data.WriteRemoteObject(token)) {
            TLOGE(WmsLogTag::WMS_LIFE, "Write abilityToken failed");
            return WSError::WS_ERROR_IPC_FAILED;
        }
    }
    if (!data.WriteString(identityToken)) {
        TLOGE(WmsLogTag::WMS_LIFE, "Write identityToken failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::WMS_LIFE, "remote is null");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    int sendCode = remote->SendRequest(static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_CONNECT),
        data, reply, option);
    if (sendCode != ERR_NONE) {
        TLOGE(WmsLogTag::WMS_LIFE, "SendRequest failed, code: %{public}d", sendCode);
        return WSError::WS_ERROR_IPC_FAILED;
    }
    sptr<SystemSessionConfig> config = reply.ReadParcelable<SystemSessionConfig>();
    if (config) {
        systemConfig = *config;
    }
    if (property) {
        property->SetPersistentId(reply.ReadInt32());
        property->SetDisplayId(reply.ReadUint64());
        bool needUpdate = reply.ReadBool();
        property->SetIsNeedUpdateWindowMode(needUpdate);
        if (needUpdate) {
            property->SetWindowMode(static_cast<WindowMode>(reply.ReadUint32()));
        }
        Rect preRect = property->GetWindowRect();
        Rect rect = { reply.ReadInt32(), reply.ReadInt32(), reply.ReadUint32(), reply.ReadUint32() };
        TLOGI(WmsLogTag::WMS_LAYOUT, "updateRect when connect."
            "preRect:[%{public}d,%{public}d,%{public}u,%{public}u]"
            "rect:[%{public}d,%{public}d,%{public}u,%{public}u]",
            preRect.posX_, preRect.posY_, preRect.width_, preRect.height_,
            rect.posX_, rect.posY_, rect.width_, rect.height_);
        if (preRect.IsUninitializedRect() && !rect.IsUninitializedRect()) {
            property->SetWindowRect(rect);
        }
        property->SetCollaboratorType(reply.ReadInt32());
        property->SetFullScreenStart(reply.ReadBool());
        uint32_t size = reply.ReadUint32();
        if (size > 0 && size <= WINDOW_SUPPORT_MODE_MAX_SIZE) {
            std::vector<AppExecFwk::SupportWindowMode> supportedWindowModes;
            supportedWindowModes.reserve(size);
            for (uint32_t i = 0; i < size; i++) {
                supportedWindowModes.push_back(
                    static_cast<AppExecFwk::SupportWindowMode>(reply.ReadInt32()));
            }
            property->SetSupportedWindowModes(supportedWindowModes);
        }
        WindowSizeLimits windowSizeLimits = { reply.ReadUint32(), reply.ReadUint32(),
                                              reply.ReadUint32(), reply.ReadUint32() };
        property->SetWindowSizeLimits(windowSizeLimits);
        property->SetIsAppSupportPhoneInPc(reply.ReadBool());
        property->SetIsPcAppInPad(reply.ReadBool());
        property->SetRequestedOrientation(static_cast<Orientation>(reply.ReadUint32()));
        property->SetUserRequestedOrientation(static_cast<Orientation>(reply.ReadUint32()));
        property->SetAppInstanceKey(reply.ReadString());
        property->SetAppIndex(reply.ReadInt32());
        property->SetDragEnabled(reply.ReadBool());
        property->SetIsAtomicService(reply.ReadBool());
        property->SetIsAbilityHook(reply.ReadBool());
        property->SetPcAppInpadCompatibleMode(reply.ReadBool());
        property->SetPcAppInpadSpecificSystemBarInvisible(reply.ReadBool());
        property->SetPcAppInpadOrientationLandscape(reply.ReadBool());
        property->SetMobileAppInPadLayoutFullScreen(reply.ReadBool());
        property->SetCompatibleModeProperty(reply.ReadParcelable<CompatibleModeProperty>());
        property->SetUseControlState(reply.ReadBool());
        property->SetAncoRealBundleName(reply.ReadString());
        sptr<MissionInfo> missionInfo = reply.ReadParcelable<MissionInfo>();
        if (missionInfo == nullptr) {
            TLOGE(WmsLogTag::WMS_LIFE, "read missionInfo is nullptr.");
            return WSError::WS_ERROR_IPC_FAILED;
        }
        property->SetMissionInfo(*missionInfo);
        property->SetIsShowDecorInFreeMultiWindow(reply.ReadBool());
        property->SetPrelaunch(reply.ReadBool());
        property->SetFrameNum(reply.ReadInt32());
    }
    int32_t ret = reply.ReadInt32();
    return static_cast<WSError>(ret);
}

WSError SessionProxy::DrawingCompleted()
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::WMS_LIFE, "WriteInterfaceToken failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }

    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::WMS_LIFE, "remote is null");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    if (remote->SendRequest(static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_DRAWING_COMPLETED),
        data, reply, option) != ERR_NONE) {
        TLOGE(WmsLogTag::WMS_LIFE, "SendRequest failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    return static_cast<WSError>(reply.ReadInt32());
}

WSError SessionProxy::RemoveStartingWindow()
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::WMS_STARTUP_PAGE, "WriteInterfaceToken failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::WMS_STARTUP_PAGE, "remote is null");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    if (remote->SendRequest(static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_APP_REMOVE_STARTING_WINDOW),
        data, reply, option) != ERR_NONE) {
        TLOGE(WmsLogTag::WMS_STARTUP_PAGE, "SendRequest failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    return static_cast<WSError>(reply.ReadInt32());
}

WSError SessionProxy::ChangeSessionVisibilityWithStatusBar(sptr<AAFwk::SessionInfo> abilitySessionInfo, bool visible)
{
    if (abilitySessionInfo == nullptr) {
        WLOGFE("abilitySessionInfo is null");
        return WSError::WS_ERROR_INVALID_SESSION;
    }

    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        WLOGFE("Write interfaceToken failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    if (!WriteAbilitySessionInfoBasic(data, abilitySessionInfo)) {
        WLOGFE("Write abilitySessionInfoBasic failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    if (abilitySessionInfo->callerToken) {
        if (!data.WriteBool(true) || !data.WriteRemoteObject(abilitySessionInfo->callerToken)) {
            WLOGFE("Write callerToken info failed");
            return WSError::WS_ERROR_IPC_FAILED;
        }
    } else {
        if (!data.WriteBool(false)) {
            WLOGFE("Write has not callerToken info failed");
            return WSError::WS_ERROR_IPC_FAILED;
        }
    }
    if (abilitySessionInfo->startSetting) {
        if (!data.WriteBool(true) || !data.WriteParcelable(abilitySessionInfo->startSetting.get())) {
            WLOGFE("Write startSetting failed");
            return WSError::WS_ERROR_IPC_FAILED;
        }
    } else {
        if (!data.WriteBool(false)) {
            WLOGFE("Write has not startSetting failed");
            return WSError::WS_ERROR_IPC_FAILED;
        }
    }
    data.WriteBool(visible);
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        WLOGFE("remote is null");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    if (remote->SendRequest(static_cast<uint32_t>(
        SessionInterfaceCode::TRANS_ID_CHANGE_SESSION_VISIBILITY_WITH_STATUS_BAR),
        data, reply, option) != ERR_NONE) {
        WLOGFE("SendRequest failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    int32_t ret = reply.ReadInt32();
    return static_cast<WSError>(ret);
}

WSError SessionProxy::PendingSessionActivation(sptr<AAFwk::SessionInfo> abilitySessionInfo)
{
    if (abilitySessionInfo == nullptr) {
        WLOGFE("abilitySessionInfo is null");
        return WSError::WS_ERROR_INVALID_SESSION;
    }

    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        WLOGFE("Write interfaceToken failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    if (!WriteAbilitySessionInfoBasic(data, abilitySessionInfo)) {
        WLOGFE("Write abilitySessionInfoBasic failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    if (!data.WriteBool(abilitySessionInfo->canStartAbilityFromBackground)) {
        TLOGE(WmsLogTag::WMS_LIFE, "Write canStartAbilityFromBackground failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    if (!data.WriteBool(abilitySessionInfo->isAtomicService) ||
        !data.WriteBool(abilitySessionInfo->isBackTransition) ||
        !data.WriteBool(abilitySessionInfo->needClearInNotShowRecent)) {
        TLOGE(WmsLogTag::WMS_LIFE, "Write isAtomicService or isBackTransition or needClearInNotShowRecent failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    if (abilitySessionInfo->callerToken) {
        if (!data.WriteBool(true) || !data.WriteRemoteObject(abilitySessionInfo->callerToken)) {
            WLOGFE("Write callerToken info failed");
            return WSError::WS_ERROR_IPC_FAILED;
        }
    } else {
        if (!data.WriteBool(false)) {
            WLOGFE("Write has not callerToken info failed");
            return WSError::WS_ERROR_IPC_FAILED;
        }
    }
    if (abilitySessionInfo->startSetting) {
        if (!data.WriteBool(true) || !data.WriteParcelable(abilitySessionInfo->startSetting.get())) {
            WLOGFE("Write startSetting failed");
            return WSError::WS_ERROR_IPC_FAILED;
        }
    } else {
        if (!data.WriteBool(false)) {
            WLOGFE("Write has not startSetting failed");
            return WSError::WS_ERROR_IPC_FAILED;
        }
    }
    if (!data.WriteString(abilitySessionInfo->instanceKey)) {
        TLOGE(WmsLogTag::WMS_LIFE, "Write instanceKey failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    if (abilitySessionInfo->startWindowOption) {
        if (!data.WriteBool(true) || !data.WriteParcelable(abilitySessionInfo->startWindowOption.get())) {
            TLOGE(WmsLogTag::WMS_STARTUP_PAGE, "Write startWindowOption failed");
            return WSError::WS_ERROR_IPC_FAILED;
        }
    } else {
        if (!data.WriteBool(false)) {
            TLOGE(WmsLogTag::WMS_STARTUP_PAGE, "Write has not startWindowOption failed");
            return WSError::WS_ERROR_IPC_FAILED;
        }
    }
    auto size = abilitySessionInfo->supportWindowModes.size();
    if (size > 0 && size <= WINDOW_SUPPORT_MODE_MAX_SIZE) {
        if (!data.WriteUint32(static_cast<uint32_t>(size))) {
            return WSError::WS_ERROR_IPC_FAILED;
        }
        for (decltype(size) i = 0; i < size; i++) {
            if (!data.WriteInt32(static_cast<int32_t>(abilitySessionInfo->supportWindowModes[i]))) {
                return WSError::WS_ERROR_IPC_FAILED;
            }
        }
    } else {
        if (!data.WriteUint32(0)) {
            return WSError::WS_ERROR_IPC_FAILED;
        }
    }
    if (!data.WriteString(abilitySessionInfo->specifiedFlag)) {
        TLOGE(WmsLogTag::WMS_LIFE, "Write specifiedFlag failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    if (!data.WriteBool(abilitySessionInfo->reuseDelegatorWindow)) {
        TLOGE(WmsLogTag::WMS_LIFE, "Write reuseDelegatorWindow failed.");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    if (!data.WriteBool(abilitySessionInfo->hideStartWindow)) {
        TLOGE(WmsLogTag::WMS_LIFE, "Write hideStartWindow failed.");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    if (!data.WriteInt32(abilitySessionInfo->scenarios)) {
        TLOGE(WmsLogTag::WMS_LIFE, "Write scenarios failed.");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    if (!data.WriteParcelable(abilitySessionInfo->windowCreateParams.get())) {
        TLOGE(WmsLogTag::WMS_LIFE, "Write windowCreateParams failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    if (!data.WriteBool(abilitySessionInfo->isPrelaunch)) {
        TLOGE(WmsLogTag::WMS_LIFE, "Write isPrelaunch failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    if (!data.WriteInt32(abilitySessionInfo->frameNum)) {
        TLOGE(WmsLogTag::WMS_LIFE, "Write frameNum failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        WLOGFE("remote is null");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    if (remote->SendRequest(static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_ACTIVE_PENDING_SESSION),
        data, reply, option) != ERR_NONE) {
        WLOGFE("SendRequest failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    int32_t ret = reply.ReadInt32();
    return static_cast<WSError>(ret);
}


WSError SessionProxy::BatchPendingSessionsActivation(
    const std::vector<sptr<AAFwk::SessionInfo>>& abilitySessionInfos,
    const std::vector<PendingSessionActivationConfig>& configs)
{
    if (abilitySessionInfos.empty()) {
        TLOGE(WmsLogTag::WMS_LIFE, "abilitySessionInfos is empty");
        return WSError::WS_ERROR_INVALID_PARAM;
    }

    if (!configs.empty() && abilitySessionInfos.size() != configs.size()) {
        TLOGE(WmsLogTag::WMS_LIFE, "abilitySessionInfos param is error");
        return WSError::WS_ERROR_INVALID_PARAM;
    }

    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::WMS_LIFE, "Write interfaceToken failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    if (!data.WriteInt32(static_cast<int32_t>(abilitySessionInfos.size()))) {
        TLOGE(WmsLogTag::WMS_LIFE, "Write ability session info list size failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    for (auto abilitySessionInfo : abilitySessionInfos) {
        WSError writeRet = WriteOneSessionInfo(data, abilitySessionInfo);
        if (writeRet != WSError::WS_OK) {
            return writeRet;
        }
    }
    
    if (!data.WriteInt32(static_cast<int32_t>(configs.size()))) {
        TLOGE(WmsLogTag::WMS_LIFE, "Write ability config list size failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }

    for (const auto& config : configs) {
        WSError writeRet = WriteOnePendingSessionActivationConfig(data, config);
        if (writeRet != WSError::WS_OK) {
            return writeRet;
        }
    }
    
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::WMS_LIFE, "remote is null");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    TLOGI(WmsLogTag::WMS_LIFE, "batch pending session activations size: %{public}zu", abilitySessionInfos.size());
    if (remote->SendRequest(static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_BATCH_ACTIVE_PENDING_SESSION),
        data, reply, option) != ERR_NONE) {
        TLOGE(WmsLogTag::WMS_LIFE, "SendRequest failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    return static_cast<WSError>(reply.ReadInt32());
}

WSError SessionProxy::WriteOnePendingSessionActivationConfig(MessageParcel& data,
    const PendingSessionActivationConfig& configs)
{
    if (!data.WriteBool(configs.forceStart)) {
        TLOGE(WmsLogTag::WMS_LIFE, "Write forceStart failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    if (!data.WriteBool(configs.forceNewWant)) {
        TLOGE(WmsLogTag::WMS_LIFE, "Write forceNewWant failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    
    return WSError::WS_OK;
}
 
WSError SessionProxy::WriteOneSessionInfo(MessageParcel& data, const sptr<AAFwk::SessionInfo>& abilitySessionInfo)
{
    if (!WriteAbilitySessionInfoBasic(data, abilitySessionInfo)) {
        TLOGE(WmsLogTag::WMS_LIFE, "Write abilitySessionInfoBasic failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    if (!data.WriteBool(abilitySessionInfo->canStartAbilityFromBackground)) {
        TLOGE(WmsLogTag::WMS_LIFE, "Write canStartAbilityFromBackground failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    if (!data.WriteBool(abilitySessionInfo->isAtomicService) ||
        !data.WriteBool(abilitySessionInfo->isBackTransition) ||
        !data.WriteBool(abilitySessionInfo->needClearInNotShowRecent)) {
        TLOGE(WmsLogTag::WMS_LIFE, "Write isAtomicService or isBackTransition or needClearInNotShowRecent failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    if (abilitySessionInfo->callerToken) {
        if (!data.WriteBool(true) || !data.WriteRemoteObject(abilitySessionInfo->callerToken)) {
            TLOGE(WmsLogTag::WMS_LIFE, "Write callerToken info failed");
            return WSError::WS_ERROR_IPC_FAILED;
        }
    } else {
        if (!data.WriteBool(false)) {
            TLOGE(WmsLogTag::WMS_LIFE, "Write has not callerToken info failed");
            return WSError::WS_ERROR_IPC_FAILED;
        }
    }
    if (abilitySessionInfo->startSetting) {
        if (!data.WriteBool(true) || !data.WriteParcelable(abilitySessionInfo->startSetting.get())) {
            TLOGE(WmsLogTag::WMS_LIFE, "Write startSetting failed");
            return WSError::WS_ERROR_IPC_FAILED;
        }
    } else {
        if (!data.WriteBool(false)) {
            TLOGE(WmsLogTag::WMS_LIFE, "Write has not startSetting failed");
            return WSError::WS_ERROR_IPC_FAILED;
        }
    }
    return WriteOneSessionInfoPart(data, abilitySessionInfo);
}

WSError SessionProxy::WriteOneSessionInfoPart(MessageParcel& data, const sptr<AAFwk::SessionInfo>& abilitySessionInfo)
{
    if (abilitySessionInfo->startWindowOption) {
        if (!data.WriteBool(true) || !data.WriteParcelable(abilitySessionInfo->startWindowOption.get())) {
            TLOGE(WmsLogTag::WMS_STARTUP_PAGE, "Write startWindowOption failed");
            return WSError::WS_ERROR_IPC_FAILED;
        }
    } else {
        if (!data.WriteBool(false)) {
            TLOGE(WmsLogTag::WMS_STARTUP_PAGE, "Write has not startWindowOption failed");
            return WSError::WS_ERROR_IPC_FAILED;
        }
    }
    if (!data.WriteString(abilitySessionInfo->instanceKey)) {
        TLOGE(WmsLogTag::WMS_LIFE, "Write instanceKey failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    auto size = abilitySessionInfo->supportWindowModes.size();
    if (size > 0 && size <= WINDOW_SUPPORT_MODE_MAX_SIZE) {
        if (!data.WriteUint32(static_cast<uint32_t>(size))) {
            return WSError::WS_ERROR_IPC_FAILED;
        }
        for (decltype(size) i = 0; i < size; i++) {
            if (!data.WriteInt32(static_cast<int32_t>(abilitySessionInfo->supportWindowModes[i]))) {
                return WSError::WS_ERROR_IPC_FAILED;
            }
        }
    } else {
        if (!data.WriteUint32(0)) {
            return WSError::WS_ERROR_IPC_FAILED;
        }
    }
    if (!data.WriteString(abilitySessionInfo->specifiedFlag)) {
        TLOGE(WmsLogTag::WMS_LIFE, "Write specifiedFlag failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    if (!data.WriteBool(abilitySessionInfo->reuseDelegatorWindow)) {
        TLOGE(WmsLogTag::WMS_LIFE, "Write reuseDelegatorWindow failed.");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    return WSError::WS_OK;
}

WSError SessionProxy::TerminateSession(const sptr<AAFwk::SessionInfo> abilitySessionInfo)
{
    if (abilitySessionInfo == nullptr) {
        WLOGFE("abilitySessionInfo is null");
        return WSError::WS_ERROR_INVALID_SESSION;
    }
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        WLOGFE("WriteInterfaceToken failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    if (!data.WriteParcelable(&(abilitySessionInfo->want))) {
        WLOGFE("Write want info failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    if (abilitySessionInfo->callerToken) {
        if (!data.WriteBool(true) || !data.WriteRemoteObject(abilitySessionInfo->callerToken)) {
            WLOGFE("Write ability info failed");
            return WSError::WS_ERROR_IPC_FAILED;
        }
    } else {
        if (!data.WriteBool(false)) {
            WLOGFE("Write ability info failed");
            return WSError::WS_ERROR_IPC_FAILED;
        }
    }
    if (!data.WriteInt32(abilitySessionInfo->resultCode)) {
        WLOGFE("Write resultCode info failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        WLOGFE("remote is null");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    if (remote->SendRequest(static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_TERMINATE),
        data, reply, option) != ERR_NONE) {
        WLOGFE("SendRequest failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    int32_t ret = reply.ReadInt32();
    return static_cast<WSError>(ret);
}

WSError SessionProxy::NotifySessionException(const sptr<AAFwk::SessionInfo> abilitySessionInfo,
    const ExceptionInfo& exceptionInfo)
{
    if (abilitySessionInfo == nullptr) {
        TLOGE(WmsLogTag::WMS_LIFE, "abilitySessionInfo is null");
        return WSError::WS_ERROR_INVALID_SESSION;
    }
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::WMS_LIFE, "WriteInterfaceToken failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    if (!data.WriteParcelable(&(abilitySessionInfo->want))) {
        TLOGE(WmsLogTag::WMS_LIFE, "Write want info failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    if (abilitySessionInfo->callerToken) {
        if (!data.WriteBool(true) || !data.WriteRemoteObject(abilitySessionInfo->callerToken)) {
            TLOGE(WmsLogTag::WMS_LIFE, "Write ability info failed");
            return WSError::WS_ERROR_IPC_FAILED;
        }
    } else {
        if (!data.WriteBool(false)) {
            TLOGE(WmsLogTag::WMS_LIFE, "Write ability info failed");
            return WSError::WS_ERROR_IPC_FAILED;
        }
    }
    if (!data.WriteInt32(abilitySessionInfo->persistentId)) {
        TLOGE(WmsLogTag::WMS_LIFE, "Write persistentId info failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    if (!data.WriteInt32(abilitySessionInfo->errorCode) ||
        !data.WriteString(abilitySessionInfo->errorReason)) {
        TLOGE(WmsLogTag::WMS_LIFE, "Write error info failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    if (!data.WriteString(abilitySessionInfo->identityToken)) {
        TLOGE(WmsLogTag::WMS_LIFE, "Write identity token info failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    if (!data.WriteBool(exceptionInfo.needRemoveSession)) {
        TLOGE(WmsLogTag::WMS_LIFE, "Write needRemoveSession info failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    if (!data.WriteBool(exceptionInfo.needClearCallerLink)) {
        TLOGE(WmsLogTag::WMS_LIFE, "Write needClearCallerLink info failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::WMS_LIFE, "remote is null");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    if (remote->SendRequest(static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_EXCEPTION),
        data, reply, option) != ERR_NONE) {
        TLOGE(WmsLogTag::WMS_LIFE, "SendRequest failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    int32_t ret = reply.ReadInt32();
    return static_cast<WSError>(ret);
}

bool WriteEventParam(MessageParcel& data, SessionEvent event, const SessionEventParam& param)
{
    if (event == SessionEvent::EVENT_MAXIMIZE) {
        if (!data.WriteUint32(param.waterfallResidentState)) {
            TLOGE(WmsLogTag::WMS_EVENT, "Failed to write waterfallResidentState");
            return false;
        }
    } else if (event == SessionEvent::EVENT_SWITCH_COMPATIBLE_MODE) {
        if (!data.WriteUint32(param.compatibleStyleMode)) {
            TLOGE(WmsLogTag::WMS_EVENT, "Failed to write compatibleStyleMode");
            return false;
        }
    }
    return true;
}

WSError SessionProxy::OnSessionEvent(SessionEvent event, const SessionEventParam& param)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        WLOGFE("WriteInterfaceToken failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    if (!data.WriteUint32(static_cast<uint32_t>(event))) {
        WLOGFE("Write event id failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    if (!WriteEventParam(data, event, param)) {
        return WSError::WS_ERROR_IPC_FAILED;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        WLOGFE("remote is null");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    if (remote->SendRequest(static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_SESSION_EVENT),
        data, reply, option) != ERR_NONE) {
        WLOGFE("SendRequest failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    int32_t ret = reply.ReadInt32();
    return static_cast<WSError>(ret);
}

WSError SessionProxy::SyncSessionEvent(SessionEvent event)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "WriteInterfaceToken failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    if (!data.WriteInt32(static_cast<int32_t>(event))) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "Write event id failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    if (Remote()->SendRequest(static_cast<int32_t>(SessionInterfaceCode::TRANS_ID_SYNC_SESSION_EVENT),
        data, reply, option) != ERR_NONE) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "SendRequest failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    int32_t ret = reply.ReadInt32();
    return static_cast<WSError>(ret);
}

WSError SessionProxy::OnLayoutFullScreenChange(bool isLayoutFullScreen)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "WriteInterfaceToken failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    if (!data.WriteBool(isLayoutFullScreen)) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "Write isLayoutFullScreen failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "remote is null");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    if (remote->SendRequest(static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_LAYOUT_FULL_SCREEN_CHANGE),
        data, reply, option) != ERR_NONE) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "SendRequest failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    int32_t ret = reply.ReadInt32();
    return static_cast<WSError>(ret);
}

WSError SessionProxy::OnDefaultDensityEnabled(bool isDefaultDensityEnabled)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "WriteInterfaceToken failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    if (!data.WriteBool(isDefaultDensityEnabled)) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "Write isDefaultDensityEnabled failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "remote is null");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    if (remote->SendRequest(static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_DEFAULT_DENSITY_ENABLED),
        data, reply, option) != ERR_NONE) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "SendRequest failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    int32_t ret = reply.ReadInt32();
    return static_cast<WSError>(ret);
}

WMError SessionProxy::OnUpdateColorMode(const std::string& colorMode, bool hasDarkRes)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "WriteInterfaceToken failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    if (!data.WriteString(colorMode)) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "Write colorMode failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    if (!data.WriteBool(hasDarkRes)) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "Write hasDarkRes failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "remote is null");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    int sendCode = remote->SendRequest(
        static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_UPDATE_COLOR_MODE),
        data, reply, option);
    if (sendCode != ERR_NONE) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "SendRequest failed, code: %{public}d", sendCode);
        return WMError::WM_ERROR_IPC_FAILED;
    }
    return WMError::WM_OK;
}

WSError SessionProxy::OnTitleAndDockHoverShowChange(bool isTitleHoverShown, bool isDockHoverShown)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::WMS_LAYOUT_PC, "WriteInterfaceToken failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    if (!data.WriteBool(isTitleHoverShown) || !data.WriteBool(isDockHoverShown)) {
        TLOGE(WmsLogTag::WMS_LAYOUT_PC, "Write isTitleHoverShown or isDockHoverShown failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::WMS_LAYOUT_PC, "remote is null");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    if (remote->SendRequest(static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_TITLE_AND_DOCK_HOVER_SHOW_CHANGE),
        data, reply, option) != ERR_NONE) {
        TLOGE(WmsLogTag::WMS_LAYOUT_PC, "SendRequest failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    uint32_t ret = reply.ReadUint32();
    return static_cast<WSError>(ret);
}

WSError SessionProxy::OnRestoreMainWindow()
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::WMS_LAYOUT_PC, "WriteInterfaceToken failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::WMS_LAYOUT_PC, "remote is null");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    if (remote->SendRequest(static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_RESTORE_MAIN_WINDOW),
        data, reply, option) != ERR_NONE) {
        TLOGE(WmsLogTag::WMS_LAYOUT_PC, "SendRequest failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    uint32_t ret = 0;
    if (!reply.ReadUint32(ret)) {
        TLOGE(WmsLogTag::WMS_LAYOUT_PC, "Read reply failed.");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    return static_cast<WSError>(ret);
}

WMError SessionProxy::RestoreFloatMainWindow(const std::shared_ptr<AAFwk::WantParams>& wantParams)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::WMS_LIFE, "WriteInterfaceToken failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    if (!data.WriteParcelable(wantParams.get())) {
        TLOGE(WmsLogTag::WMS_LIFE, "write wantParams failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::WMS_LIFE, "remote is null");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    if (remote->SendRequest(
        static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_RESTORE_FLOAT_MAIN_WINDOW),
        data, reply, option) != ERR_NONE) {
        TLOGE(WmsLogTag::WMS_LIFE, "SendRequest failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    uint32_t ret = 0;
    if (!reply.ReadUint32(ret)) {
        TLOGE(WmsLogTag::WMS_LIFE, "Read reply failed.");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    return static_cast<WMError>(ret);
}

/** @note @window.layout */
WSError SessionProxy::UpdateSessionRect(const WSRect& rect, SizeChangeReason reason,
    bool isGlobal, bool isFromMoveToGlobal, const MoveConfiguration& moveConfiguration,
    const RectAnimationConfig& rectAnimationConfig)
{
    TLOGI_LMT(TEN_SECONDS, RECORD_100_TIMES, WmsLogTag::WMS_LAYOUT,
        "Rect:%{public}s global:%{public}d isFromMoveToGlobal:%{public}d cfg:%{public}s",
        rect.ToString().c_str(), isGlobal, isFromMoveToGlobal, moveConfiguration.ToString().c_str());
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "WriteInterfaceToken failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    if (!((data.WriteInt32(static_cast<int32_t>(rect.posX_))) &&
        (data.WriteInt32(static_cast<int32_t>(rect.posY_))) &&
        (data.WriteUint32(static_cast<uint32_t>(rect.width_))) &&
        (data.WriteUint32(static_cast<uint32_t>(rect.height_))))) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "Write rect failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }

    if (!data.WriteUint32(static_cast<uint32_t>(reason))) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "Write SessionSizeChangeReason failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }

    if (!data.WriteBool(isGlobal)) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "Write bool failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }

    if (!data.WriteBool(isFromMoveToGlobal)) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "Write bool failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }

    if (!data.WriteUint64(static_cast<uint64_t>(moveConfiguration.displayId))) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "Write session displayId failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }

    if (reason == SizeChangeReason::MOVE_WITH_ANIMATION || reason == SizeChangeReason::RESIZE_WITH_ANIMATION) {
        if (!data.WriteUint32(rectAnimationConfig.duration) || !data.WriteFloat(rectAnimationConfig.x1) ||
            !data.WriteFloat(rectAnimationConfig.y1) || !data.WriteFloat(rectAnimationConfig.x2) ||
            !data.WriteFloat(rectAnimationConfig.y2)) {
            TLOGE(WmsLogTag::WMS_LAYOUT, "Write rectAnimationConfig failed");
            return WSError::WS_ERROR_IPC_FAILED;
        }
    }

    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "remote is null");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    if (remote->SendRequest(static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_UPDATE_SESSION_RECT),
        data, reply, option) != ERR_NONE) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "SendRequest failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    int32_t ret = reply.ReadInt32();
    return static_cast<WSError>(ret);
}

/** @note @window.layout */
WSError SessionProxy::UpdateGlobalDisplayRectFromClient(const WSRect& rect, SizeChangeReason reason)
{
    MessageParcel data;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "Failed to write interface token");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    if (!data.WriteInt32(rect.posX_) || !data.WriteInt32(rect.posY_) ||
        !data.WriteInt32(rect.width_) || !data.WriteInt32(rect.height_)) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "Failed to write rect");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    if (!data.WriteUint32(static_cast<uint32_t>(reason))) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "Failed to write reason");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "remote is nullptr");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    int sendRet = remote->SendRequest(
        static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_UPDATE_GLOBAL_DISPLAY_RECT), data, reply, option);
    if (sendRet != ERR_NONE) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "Failed to send request, error = %{public}d", sendRet);
        return WSError::WS_ERROR_IPC_FAILED;
    }
    return WSError::WS_OK;
}

/** @note @window.layout */
WMError SessionProxy::GetGlobalScaledRect(Rect& globalScaledRect)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "WriteInterfaceToken failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "remote is null");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    if (remote->SendRequest(static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_GET_GLOBAL_SCALED_RECT),
        data, reply, option) != ERR_NONE) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "SendRequest failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    int32_t posX = 0;
    int32_t posY = 0;
    uint32_t width = 0;
    uint32_t height = 0;
    int32_t ret = 0;
    if (!reply.ReadInt32(posX) || !reply.ReadInt32(posY) ||
        !reply.ReadUint32(width) || !reply.ReadUint32(height) || !reply.ReadInt32(ret)) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "read failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    globalScaledRect = { posX, posY, width, height };
    return static_cast<WMError>(ret);
}

/** @note @window.layout */
WSError SessionProxy::UpdateClientRect(const WSRect& rect)
{
    TLOGD(WmsLogTag::WMS_LAYOUT, "rect:[%{public}d, %{public}d, %{public}d, %{public}d]",
        rect.posX_, rect.posY_, rect.width_, rect.height_);
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "WriteInterfaceToken failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    if (!data.WriteInt32(rect.posX_) ||
        !data.WriteInt32(rect.posY_) ||
        !data.WriteInt32(rect.width_) ||
        !data.WriteInt32(rect.height_)) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "Write rect failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }

    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "remote is null");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    if (remote->SendRequest(static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_UPDATE_CLIENT_RECT),
        data, reply, option) != ERR_NONE) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "SendRequest failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    int32_t ret = reply.ReadInt32();
    return static_cast<WSError>(ret);
}

/** @note @window.hierarchy */
WSError SessionProxy::RaiseToAppTop()
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        WLOGFE("WriteInterfaceToken failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        WLOGFE("remote is null");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    if (remote->SendRequest(static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_RAISE_TO_APP_TOP),
        data, reply, option) != ERR_NONE) {
        WLOGFE("SendRequest failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    int32_t ret = reply.ReadInt32();
    return static_cast<WSError>(ret);
}

WSError SessionProxy::NotifyFrameLayoutFinishFromApp(bool notifyListener, const WSRect& rect)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::WMS_MULTI_WINDOW, "WriteInterfaceToken failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }

    if (!data.WriteBool(notifyListener)) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "Write notifyListener failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }

    if (!data.WriteInt32(rect.posX_) || !data.WriteInt32(rect.posY_) ||
        !data.WriteInt32(rect.width_) || !data.WriteInt32(rect.height_)) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "Write rect failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }

    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::WMS_MULTI_WINDOW, "remote is null");
        return WSError::WS_ERROR_IPC_FAILED;
    }

    if (remote->SendRequest(static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_FRAME_LAYOUT_FINISH),
        data, reply, option) != ERR_NONE) {
        TLOGE(WmsLogTag::WMS_MULTI_WINDOW, "SendRequest failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    return WSError::WS_OK;
}

WMError SessionProxy::NotifySnapshotUpdate()
{
    TLOGI(WmsLogTag::WMS_PATTERN, "in");
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::WMS_PATTERN, "WriteInterfaceToken failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::WMS_PATTERN, "remote is null");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    int sendCode = remote->SendRequest(
        static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_NOTIFY_SNAPSHOT_UPDATE),
        data, reply, option);
    if (sendCode != ERR_NONE) {
        TLOGE(WmsLogTag::WMS_PATTERN, "SendRequest failed, code: %{public}d", sendCode);
        return WMError::WM_ERROR_IPC_FAILED;
    }
    return WMError::WM_OK;
}

WMError SessionProxy::NotifyRemovePrelaunchStartingWindow()
{
    TLOGI(WmsLogTag::WMS_PATTERN, "in");
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::WMS_PATTERN, "WriteInterfaceToken failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::WMS_PATTERN, "remote is null");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    int sendCode = remote->SendRequest(
        static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_NOTIFY_REMOVE_PRELAUNCH_STARTING_WINDOW),
        data, reply, option);
    if (sendCode != ERR_NONE) {
        TLOGE(WmsLogTag::WMS_PATTERN, "SendRequest failed, code: %{public}d", sendCode);
        return WMError::WM_ERROR_IPC_FAILED;
    }
    return WMError::WM_OK;
}

/** @note @window.hierarchy */
WSError SessionProxy::RaiseAboveTarget(int32_t subWindowId)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        WLOGFE("WriteInterfaceToken failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    if (!data.WriteInt32(subWindowId)) {
        WLOGFE("Write subWindowId failed");
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        WLOGFE("remote is null");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    if (remote->SendRequest(static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_RAISE_ABOVE_TARGET),
        data, reply, option) != ERR_NONE) {
        WLOGFE("SendRequest failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    int32_t ret = reply.ReadInt32();
    return static_cast<WSError>(ret);
}

/** @note @window.hierarchy */
WSError SessionProxy::RaiseMainWindowAboveTarget(int32_t targetId)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        WLOGFE("WriteInterfaceToken failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    if (!data.WriteInt32(targetId)) {
        WLOGFE("Write targetId failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        WLOGFE("remote is null");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    if (remote->SendRequest(static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_RAISE_MAIN_WINDOW_ABOVE_TARGET),
        data, reply, option) != ERR_NONE) {
        WLOGFE("SendRequest failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    int32_t ret = reply.ReadInt32();
    return static_cast<WSError>(ret);
}

WSError SessionProxy::RaiseAppMainWindowToTop()
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        WLOGFE("WriteInterfaceToken failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        WLOGFE("remote is null");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    if (remote->SendRequest(static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_RAISE_APP_MAIN_WINDOW),
        data, reply, option) != ERR_NONE) {
        WLOGFE("SendRequest failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    int32_t ret = reply.ReadInt32();
    return static_cast<WSError>(ret);
}

WSError SessionProxy::OnNeedAvoid(bool status)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        WLOGFE("WriteInterfaceToken failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    if (!data.WriteBool(status)) {
        WLOGFE("Write status failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        WLOGFE("remote is null");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    if (remote->SendRequest(static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_NEED_AVOID),
        data, reply, option) != ERR_NONE) {
        WLOGFE("SendRequest failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    int32_t ret = reply.ReadInt32();
    return static_cast<WSError>(ret);
}

AvoidArea SessionProxy::GetAvoidAreaByType(AvoidAreaType type, const WSRect& rect, int32_t apiVersion)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    AvoidArea avoidArea;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::WMS_IMMS, "WriteInterfaceToken failed");
        return avoidArea;
    }
    if (!data.WriteUint32(static_cast<uint32_t>(type))) {
        TLOGE(WmsLogTag::WMS_IMMS, "write type error");
        return avoidArea;
    }
    if (!data.WriteInt32(rect.posX_) || !data.WriteInt32(rect.posY_) ||
        !data.WriteInt32(rect.width_) || !data.WriteInt32(rect.height_)) {
        TLOGE(WmsLogTag::WMS_IMMS, "write rect error");
        return avoidArea;
    }
    if (!data.WriteInt32(apiVersion)) {
        TLOGE(WmsLogTag::WMS_IMMS, "write api version error");
        return avoidArea;
    }

    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::WMS_IMMS, "remote is null");
        return avoidArea;
    }
    int sendCode = remote->SendRequest(static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_GET_AVOID_AREA),
        data, reply, option);
    if (sendCode != ERR_NONE) {
        TLOGE(WmsLogTag::WMS_IMMS, "SendRequest failed, code: %{public}d", sendCode);
        return avoidArea;
    }
    sptr<AvoidArea> area = reply.ReadParcelable<AvoidArea>();
    if (area == nullptr) {
        return avoidArea;
    }
    return *area;
}

AvoidArea SessionProxy::GetAvoidAreaByTypeIgnoringVisibility(AvoidAreaType type, const WSRect& rect)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    AvoidArea avoidArea;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::WMS_IMMS, "WriteInterfaceToken failed");
        return avoidArea;
    }
    if (!data.WriteUint32(static_cast<uint32_t>(type))) {
        TLOGE(WmsLogTag::WMS_IMMS, "write type error");
        return avoidArea;
    }
    if (!data.WriteInt32(rect.posX_) || !data.WriteInt32(rect.posY_) ||
        !data.WriteInt32(rect.width_) || !data.WriteInt32(rect.height_)) {
        TLOGE(WmsLogTag::WMS_IMMS, "write rect error");
        return avoidArea;
    }

    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::WMS_IMMS, "remote is null");
        return avoidArea;
    }
    int sendCode = remote->SendRequest(static_cast<uint32_t>(
        SessionInterfaceCode::TRANS_ID_GET_AVOID_AREA_IGNORING_VISIBILITY),
        data, reply, option);
    if (sendCode != ERR_NONE) {
        TLOGE(WmsLogTag::WMS_IMMS, "SendRequest failed, code: %{public}d", sendCode);
        return avoidArea;
    }
    sptr<AvoidArea> area = reply.ReadParcelable<AvoidArea>();
    if (area == nullptr) {
        return avoidArea;
    }
    return *area;
}

WSError SessionProxy::GetAllAvoidAreas(std::map<AvoidAreaType, AvoidArea>& avoidAreas)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        WLOGFE("WriteInterfaceToken failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        WLOGFE("remote is null");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    if (remote->SendRequest(static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_GET_ALL_AVOID_AREAS),
        data, reply, option) != ERR_NONE) {
        WLOGFE("SendRequest failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    uint32_t size = reply.ReadUint32();
    constexpr uint32_t AVOID_AREA_TYPE_MAX_SIZE = 100;
    if (size > AVOID_AREA_TYPE_MAX_SIZE) {
        TLOGE(WmsLogTag::WMS_IMMS, "size is invalid");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    for (uint32_t i = 0; i < size; i++) {
        uint32_t type = reply.ReadUint32();
        if (type < static_cast<uint32_t>(AvoidAreaType::TYPE_SYSTEM) ||
            type > static_cast<uint32_t>(AvoidAreaType::TYPE_NAVIGATION_INDICATOR)) {
            WLOGFE("Read type failed");
            return WSError::WS_ERROR_IPC_FAILED;
        }
        sptr<AvoidArea> area = reply.ReadParcelable<AvoidArea>();
        if (area == nullptr) {
            return WSError::WS_ERROR_IPC_FAILED;
        }
        avoidAreas[static_cast<AvoidAreaType>(type)] = *area;
    }
    uint32_t ret = reply.ReadUint32();
    return static_cast<WSError>(ret);
}


WSError SessionProxy::GetTargetOrientationConfigInfo(Orientation targetOrientation,
    const std::map<Rosen::WindowType, Rosen::SystemBarProperty>& targetProperties,
    const std::map<Rosen::WindowType, Rosen::SystemBarProperty>& currentProperties)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::WMS_ROTATION, "WriteInterfaceToken failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    if (!data.WriteUint32(static_cast<uint32_t>(targetOrientation))) {
        TLOGE(WmsLogTag::WMS_ROTATION, "write orientation error");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    if (!data.WriteUint32(static_cast<uint32_t>(targetProperties.size()))) {
        TLOGE(WmsLogTag::WMS_ROTATION, "write targetProperties size error");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    for (const auto& [type, systemBarProperty] : targetProperties) {
        if (!data.WriteUint32(static_cast<uint32_t>(type))) {
            return WSError::WS_ERROR_IPC_FAILED;
        }
        if (!data.WriteBool(systemBarProperty.enable_)) {
            return WSError::WS_ERROR_IPC_FAILED;
        }
        if (!data.WriteUint32(systemBarProperty.backgroundColor_)) {
            return WSError::WS_ERROR_IPC_FAILED;
        }
        if (!data.WriteUint32(systemBarProperty.contentColor_)) {
            return WSError::WS_ERROR_IPC_FAILED;
        }
        if (!data.WriteBool(systemBarProperty.enableAnimation_)) {
            return WSError::WS_ERROR_IPC_FAILED;
        }
        if (!data.WriteUint32(static_cast<uint32_t>(systemBarProperty.settingFlag_))) {
            return WSError::WS_ERROR_IPC_FAILED;
        }
    }

    if (!data.WriteUint32(static_cast<uint32_t>(currentProperties.size()))) {
        TLOGE(WmsLogTag::WMS_ROTATION, "write currentProperties size error");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    for (const auto& [type, systemBarProperty] : currentProperties) {
        if (!data.WriteUint32(static_cast<uint32_t>(type))) {
            return WSError::WS_ERROR_IPC_FAILED;
        }
        if (!data.WriteBool(systemBarProperty.enable_)) {
            return WSError::WS_ERROR_IPC_FAILED;
        }
        if (!data.WriteUint32(systemBarProperty.backgroundColor_)) {
            return WSError::WS_ERROR_IPC_FAILED;
        }
        if (!data.WriteUint32(systemBarProperty.contentColor_)) {
            return WSError::WS_ERROR_IPC_FAILED;
        }
        if (!data.WriteBool(systemBarProperty.enableAnimation_)) {
            return WSError::WS_ERROR_IPC_FAILED;
        }
        if (!data.WriteUint32(static_cast<uint32_t>(systemBarProperty.settingFlag_))) {
            return WSError::WS_ERROR_IPC_FAILED;
        }
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::WMS_ROTATION, "remote is null");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    if (remote->SendRequest(static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_GET_TARGET_ORIENTATION_CONFIG_INFO),
        data, reply, option) != ERR_NONE) {
        TLOGE(WmsLogTag::WMS_ROTATION, "SendRequest failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    uint32_t ret = reply.ReadUint32();
    return static_cast<WSError>(ret);
}

WSError SessionProxy::ConvertOrientationAndRotation(const RotationInfoType from, const RotationInfoType to,
    const int32_t value, int32_t& convertedValue)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::WMS_ROTATION, "WriteInterfaceToken failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    if (!data.WriteUint32(static_cast<uint32_t>(from))) {
        TLOGE(WmsLogTag::WMS_ROTATION, "write from type error");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    if (!data.WriteUint32(static_cast<uint32_t>(to))) {
        TLOGE(WmsLogTag::WMS_ROTATION, "write to type error");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    if (!data.WriteInt32(static_cast<int32_t>(value))) {
        TLOGE(WmsLogTag::WMS_ROTATION, "write the value to be converted error");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::WMS_ROTATION, "remote is null");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    if (remote->SendRequest(static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_CONVERT_ORIENTATION_AND_ROTATION),
        data, reply, option) != ERR_NONE) {
        TLOGE(WmsLogTag::WMS_ROTATION, "SendRequest failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    if (!reply.ReadInt32(convertedValue)) {
        TLOGE(WmsLogTag::WMS_ROTATION, "read failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    int32_t ret = 0;
    if (!reply.ReadInt32(ret)) {
        TLOGE(WmsLogTag::WMS_ROTATION, "read ret failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    return static_cast<WSError>(ret);
}

WSError SessionProxy::RequestSessionBack(bool needMoveToBackground)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        WLOGFE("WriteInterfaceToken failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    if (!data.WriteBool(needMoveToBackground)) {
        WLOGFE("Write needMoveToBackground failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        WLOGFE("remote is null");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    if (remote->SendRequest(static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_BACKPRESSED),
        data, reply, option) != ERR_NONE) {
        WLOGFE("SendRequest failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    int32_t ret = reply.ReadInt32();
    return static_cast<WSError>(ret);
}

WSError SessionProxy::MarkProcessed(int32_t eventId)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        WLOGFE("WriteInterfaceToken failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    if (!data.WriteInt32(eventId)) {
        WLOGFE("WriteInterfaceToken failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        WLOGFE("remote is null");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    if (remote->SendRequest(static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_MARK_PROCESSED),
        data, reply, option) != ERR_NONE) {
        WLOGFE("SendRequest failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    int32_t ret = reply.ReadInt32();
    return static_cast<WSError>(ret);
}

WSError OHOS::Rosen::SessionProxy::SetGlobalMaximizeMode(MaximizeMode mode)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        WLOGFE("WriteInterfaceToken failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    if (!data.WriteUint32(static_cast<uint32_t>(mode))) {
        WLOGFE("Write uint32_t failed");
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        WLOGFE("remote is null");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    if (remote->SendRequest(static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_SET_MAXIMIZE_MODE),
        data, reply, option) != ERR_NONE) {
        WLOGFE("SendRequest failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    int32_t ret = reply.ReadInt32();
    return static_cast<WSError>(ret);
}

WSError SessionProxy::GetGlobalMaximizeMode(MaximizeMode& mode)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        WLOGFE("WriteInterfaceToken failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        WLOGFE("remote is null");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    if (remote->SendRequest(static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_GET_MAXIMIZE_MODE),
        data, reply, option) != ERR_NONE) {
        WLOGFE("SendRequest failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    mode = static_cast<MaximizeMode>(reply.ReadUint32());
    int32_t ret = reply.ReadInt32();
    return static_cast<WSError>(ret);
}

/** @note @window.layout */
WSError SessionProxy::SetAspectRatio(float ratio)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        WLOGFE("WriteInterfaceToken failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    if (!data.WriteFloat(ratio)) {
        WLOGFE("Write ratio failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        WLOGFE("remote is null");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    if (remote->SendRequest(static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_SET_ASPECT_RATIO),
                            data, reply, option) != ERR_NONE) {
        WLOGFE("SendRequest failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    int32_t ret = reply.ReadInt32();
    return static_cast<WSError>(ret);
}

/** @note @window.layout */
WSError SessionProxy::SetContentAspectRatio(float ratio, bool isPersistent, bool needUpdateRect)
{
    MessageParcel data;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "Failed to write interface token");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    if (!data.WriteFloat(ratio)) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "Failed to write ratio");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    if (!data.WriteBool(isPersistent)) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "Failed to write isPersistent");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    if (!data.WriteBool(needUpdateRect)) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "Failed to write needUpdateRect");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "remote is null");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    MessageParcel reply;
    MessageOption option;
    int sendRet = remote->SendRequest(
        static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_SET_CONTENT_ASPECT_RATIO), data, reply, option);
    if (sendRet != ERR_NONE) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "Failed to send request, error = %{public}d", sendRet);
        return WSError::WS_ERROR_IPC_FAILED;
    }
    int32_t ret = reply.ReadInt32();
    return static_cast<WSError>(ret);
}

WSError SessionProxy::UpdateWindowSceneAfterCustomAnimation(bool isAdd)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        WLOGFE("WriteInterfaceToken failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    if (!data.WriteBool(isAdd)) {
        WLOGFE("Write isAdd failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        WLOGFE("remote is null");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    if (remote->SendRequest(static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_UPDATE_CUSTOM_ANIMATION),
                            data, reply, option) != ERR_NONE) {
        WLOGFE("SendRequest failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    int32_t ret = reply.ReadInt32();
    return static_cast<WSError>(ret);
}

WSError SessionProxy::SetLandscapeMultiWindow(bool isLandscapeMultiWindow)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::WMS_MULTI_WINDOW, "WriteInterfaceToken failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    if (!data.WriteBool(isLandscapeMultiWindow)) {
        TLOGE(WmsLogTag::WMS_MULTI_WINDOW, "Write isLandscapeMultiWindow failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        WLOGFE("remote is null");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    if (remote->SendRequest(static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_SET_LANDSCAPE_MULTI_WINDOW),
                            data, reply, option) != ERR_NONE) {
        WLOGFE("SendRequest failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    int32_t ret = reply.ReadInt32();
    return static_cast<WSError>(ret);
}

WSError SessionProxy::GetIsMidScene(bool& isMidScene)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::WMS_MULTI_WINDOW, "WriteInterfaceToken failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::WMS_MULTI_WINDOW, "remote is null");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    if (remote->SendRequest(static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_GET_IS_MID_SCENE),
        data, reply, option) != ERR_NONE) {
        TLOGE(WmsLogTag::WMS_MULTI_WINDOW, "SendRequest failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    if (!reply.ReadBool(isMidScene)) {
        TLOGE(WmsLogTag::WMS_MULTI_WINDOW, "Read isMidScene failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    int32_t ret = reply.ReadInt32();
    return static_cast<WSError>(ret);
}

WSError SessionProxy::TransferAbilityResult(uint32_t resultCode, const AAFwk::Want& want)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::WMS_UIEXT, "WriteInterfaceToken failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    if (!data.WriteUint32(resultCode)) {
        TLOGE(WmsLogTag::WMS_UIEXT, "resultCode write failed.");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    if (!data.WriteParcelable(&want)) {
        TLOGE(WmsLogTag::WMS_UIEXT, "want write failed.");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::WMS_UIEXT, "remote is null");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    int sendCode = remote->SendRequest(static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_TRANSFER_ABILITY_RESULT),
        data, reply, option);
    if (sendCode != ERR_NONE) {
        TLOGE(WmsLogTag::WMS_UIEXT, "SendRequest failed, code: %{public}d", sendCode);
        return WSError::WS_ERROR_IPC_FAILED;
    }
    int32_t ret = reply.ReadInt32();
    return static_cast<WSError>(ret);
}

int32_t SessionProxy::TransferExtensionData(const AAFwk::WantParams& wantParams)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::WMS_UIEXT, "WriteInterfaceToken failed");
        return IPC_PROXY_ERR;
    }
    if (!data.WriteParcelable(&wantParams)) {
        TLOGE(WmsLogTag::WMS_UIEXT, "wantParams write failed.");
        return IPC_PROXY_ERR;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::WMS_UIEXT, "remote is null");
        return IPC_PROXY_ERR;
    }
    int sendCode = remote->SendRequest(static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_TRANSFER_EXTENSION_DATA),
        data, reply, option);
    if (sendCode != ERR_NONE) {
        TLOGE(WmsLogTag::WMS_UIEXT, "SendRequest failed, code: %{public}d", sendCode);
    }
    int32_t ret = reply.ReadInt32();
    if (ret != ERR_NONE) {
        TLOGE(WmsLogTag::WMS_UIEXT, "Ret value read by ReadInt32 is abnormal, ret: %{public}d", ret);
    }
    return sendCode;
}

void SessionProxy::NotifySyncOn()
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::WMS_UIEXT, "WriteInterfaceToken failed");
        return;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::WMS_UIEXT, "remote is null");
        return;
    }
    int sendCode = remote->SendRequest(static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_NOTIFY_SYNC_ON),
        data, reply, option);
    if (sendCode != ERR_NONE) {
        TLOGE(WmsLogTag::WMS_UIEXT, "SendRequest failed, code: %{public}d", sendCode);
    }
}

void SessionProxy::NotifyAsyncOn()
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::WMS_UIEXT, "WriteInterfaceToken failed");
        return;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::WMS_UIEXT, "remote is null");
        return;
    }
    int sendCode = remote->SendRequest(static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_NOTIFY_ASYNC_ON),
        data, reply, option);
    if (sendCode != ERR_NONE) {
        TLOGE(WmsLogTag::WMS_UIEXT, "SendRequest failed, code: %{public}d", sendCode);
    }
}

void SessionProxy::NotifyExtensionDied()
{
    TLOGI(WmsLogTag::WMS_UIEXT, "NotifyExtensionDied called.");
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::WMS_UIEXT, "WriteInterfaceToken failed");
        return;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::WMS_UIEXT, "remote is null");
        return;
    }
    int sendCode = remote->SendRequest(static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_NOTIFY_EXTENSION_DIED),
        data, reply, option);
    if (sendCode != ERR_NONE) {
        TLOGE(WmsLogTag::WMS_UIEXT, "SendRequest failed, code: %{public}d", sendCode);
    }
}

void SessionProxy::NotifyExtensionTimeout(int32_t errorCode)
{
    TLOGI(WmsLogTag::WMS_UIEXT, "NotifyExtensionTimeout(errorCode:%{public}d) called.", errorCode);
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::WMS_UIEXT, "WriteInterfaceToken failed");
        return;
    }
    if (!data.WriteInt32(static_cast<int32_t>(errorCode))) {
        TLOGE(WmsLogTag::WMS_UIEXT, "errorCode write failed.");
        return;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::WMS_UIEXT, "remote is null");
        return;
    }
    int sendCode = remote->SendRequest(static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_NOTIFY_EXTENSION_TIMEOUT),
        data, reply, option);
    if (sendCode != ERR_NONE) {
        TLOGE(WmsLogTag::WMS_UIEXT, "SendRequest failed, code: %{public}d", sendCode);
    }
}

void SessionProxy::TriggerBindModalUIExtension()
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::WMS_UIEXT, "WriteInterfaceToken failed");
        return;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::WMS_UIEXT, "remote is null");
        return;
    }
    int sendCode = remote->SendRequest(
        static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_TRIGGER_BIND_MODAL_UI_EXTENSION), data, reply, option);
    if (sendCode != ERR_NONE) {
        TLOGE(WmsLogTag::WMS_UIEXT, "SendRequest failed, code: %{public}d", sendCode);
    }
}

WSError SessionProxy::UpdateWindowAnimationFlag(bool needDefaultAnimationFlag)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        WLOGFE("WriteInterfaceToken failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    if (!data.WriteBool(needDefaultAnimationFlag)) {
        WLOGFE("wantParams write failed.");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        WLOGFE("remote is null");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    if (remote->SendRequest(static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_UPDATE_WINDOW_ANIMATION_FLAG),
        data, reply, option) != ERR_NONE) {
        WLOGFE("SendRequest failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    int32_t ret = reply.ReadInt32();
    return static_cast<WSError>(ret);
}

WSError SessionProxy::TransferAccessibilityEvent(const Accessibility::AccessibilityEventInfo& info,
    int64_t uiExtensionIdLevel)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::WMS_UIEXT, "WriteInterfaceToken failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    Accessibility::AccessibilityEventInfoParcel infoParcel(info);
    if (!data.WriteParcelable(&infoParcel)) {
        TLOGE(WmsLogTag::WMS_UIEXT, "infoParcel write failed.");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    if (!data.WriteInt64(uiExtensionIdLevel)) {
        TLOGE(WmsLogTag::WMS_UIEXT, "idVec write failed.");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::WMS_UIEXT, "remote is null");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    int sendCode = remote->SendRequest(
        static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_NOTIFY_REPORT_ACCESSIBILITY_EVENT), data, reply, option);
    if (sendCode != ERR_NONE) {
        TLOGE(WmsLogTag::WMS_UIEXT, "SendRequest failed, code: %{public}d", sendCode);
        return WSError::WS_ERROR_IPC_FAILED;
    }
    return WSError::WS_OK;
}

void SessionProxy::NotifyPiPWindowPrepareClose()
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        WLOGFE("writeInterfaceToken failed");
        return;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        WLOGFE("remote is null");
        return;
    }
    if (remote->SendRequest(static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_NOTIFY_PIP_WINDOW_PREPARE_CLOSE),
        data, reply, option) != ERR_NONE) {
        WLOGFE("SendRequest failed");
        return;
    }
}

WSError SessionProxy::UpdatePiPRect(const Rect& rect, SizeChangeReason reason)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        WLOGFE("writeInterfaceToken failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    if (!data.WriteInt32(rect.posX_)) {
        WLOGFE("write posX_ failed.");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    if (!data.WriteInt32(rect.posY_)) {
        WLOGFE("write posY_ failed.");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    if (!data.WriteUint32(rect.width_)) {
        WLOGFE("write width_ failed.");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    if (!data.WriteUint32(rect.height_)) {
        WLOGFE("write height_ failed.");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    if (!data.WriteUint32(static_cast<uint32_t>(reason))) {
        WLOGFE("reason write failed.");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        WLOGFE("remote is null");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    if (remote->SendRequest(static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_UPDATE_PIP_RECT),
        data, reply, option) != ERR_NONE) {
        WLOGFE("SendRequest failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    int32_t ret = reply.ReadInt32();
    return static_cast<WSError>(ret);
}

WSError SessionProxy::UpdatePiPControlStatus(WsPiPControlType controlType, WsPiPControlStatus status)
{
    TLOGI(WmsLogTag::WMS_PIP, "controlType:%{public}u, status:%{public}d", controlType, status);
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::WMS_PIP, "writeInterfaceToken failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    if (!data.WriteUint32(static_cast<uint32_t>(controlType))) {
        TLOGE(WmsLogTag::WMS_PIP, "Write controlType failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    if (!data.WriteInt32(static_cast<int32_t>(status))) {
        TLOGE(WmsLogTag::WMS_PIP, "write status failed.");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::WMS_PIP, "remote is null");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    if (remote->SendRequest(static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_UPDATE_PIP_CONTROL_STATUS),
        data, reply, option) != ERR_NONE) {
        TLOGE(WmsLogTag::WMS_PIP, "SendRequest failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    int32_t ret = reply.ReadInt32();
    return static_cast<WSError>(ret);
}

WSError SessionProxy::SetAutoStartPiP(bool isAutoStart, uint32_t priority, uint32_t width, uint32_t height)
{
    TLOGD(WmsLogTag::WMS_PIP, "isAutoStart:%{public}u priority:%{public}u width:%{public}u height:%{public}u",
        isAutoStart, priority, width, height);
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::WMS_PIP, "writeInterfaceToken failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    if (!data.WriteBool(isAutoStart)) {
        TLOGE(WmsLogTag::WMS_PIP, "write isAutoStart failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    if (!data.WriteUint32(priority)) {
        TLOGE(WmsLogTag::WMS_PIP, "write priority failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    if (!data.WriteUint32(width)) {
        TLOGE(WmsLogTag::WMS_PIP, "write width failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    if (!data.WriteUint32(height)) {
        TLOGE(WmsLogTag::WMS_PIP, "write height failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::WMS_PIP, "remote is null");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    if (remote->SendRequest(static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_SET_AUTOSTART_PIP),
        data, reply, option) != ERR_NONE) {
        TLOGE(WmsLogTag::WMS_PIP, "SendRequest failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    int32_t ret = reply.ReadInt32();
    return static_cast<WSError>(ret);
}

WMError SessionProxy::UpdateFloatingBall(const FloatingBallTemplateInfo& fbTemplateInfo)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::WMS_SYSTEM, "writeInterfaceToken failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    if (!data.WriteParcelable(&fbTemplateInfo)) {
        TLOGE(WmsLogTag::WMS_SYSTEM, "write fbTemplateInfo failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::WMS_SYSTEM, "remote is null");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    auto errCode = remote->SendRequest(static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_UPDATE_FLOATING_BALL),
                                       data, reply, option);
    if (errCode != ERR_NONE) {
        TLOGE(WmsLogTag::WMS_SYSTEM, "SendRequest failed code: %{public}d", errCode);
        return WMError::WM_ERROR_IPC_FAILED;
    }
    int32_t ret = 0;
    if (!reply.ReadInt32(ret)) {
        TLOGE(WmsLogTag::WMS_SYSTEM, "Read reply failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    return static_cast<WMError>(ret);
}

WMError SessionProxy::RestoreFbMainWindow(const std::shared_ptr<AAFwk::Want>& want)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (want == nullptr) {
        TLOGE(WmsLogTag::WMS_SYSTEM, "want is nullptr");
        return WMError::WM_ERROR_IPC_FAILED;
    }

    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::WMS_SYSTEM, "writeInterfaceToken failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    if (!data.WriteParcelable(want.get())) {
        TLOGE(WmsLogTag::WMS_SYSTEM, "write icon failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::WMS_SYSTEM, "remote is null");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    auto errCode = remote->SendRequest(
        static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_START_FLOATING_BALL_MAIN_WINDOW), data, reply, option);
    if (errCode != ERR_NONE) {
        TLOGE(WmsLogTag::WMS_SYSTEM, "SendRequest failed code: %{public}d", errCode);
        return WMError::WM_ERROR_IPC_FAILED;
    }
    int32_t ret = 0;
    if (!reply.ReadInt32(ret)) {
        TLOGE(WmsLogTag::WMS_SYSTEM, "Read reply failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    return static_cast<WMError>(ret);
}

WMError SessionProxy::GetFloatingBallWindowId(uint32_t& windowId)
{
    TLOGI(WmsLogTag::WMS_SYSTEM, "GetFloatingBallWindowId start");
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::WMS_SYSTEM, "writeInterfaceToken failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::WMS_SYSTEM, "remote is null");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    auto errCode = remote->SendRequest(
        static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_GET_FLOATING_BALL_WINDOW_ID), data, reply, option);
    if (errCode != ERR_NONE) {
        TLOGE(WmsLogTag::WMS_SYSTEM, "SendRequest failed code: %{public}d", errCode);
        return WMError::WM_ERROR_IPC_FAILED;
    }

    int32_t ret = 0;
    if (!reply.ReadInt32(ret)) {
        TLOGE(WmsLogTag::WMS_SYSTEM, "Read reply failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    if (!reply.ReadUint32(windowId)) {
        TLOGE(WmsLogTag::WMS_SYSTEM, "Read windowId failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }

    TLOGI(WmsLogTag::WMS_SYSTEM, "GetFloatingBallWindowId send success, %{public}d, %{public}d", ret, windowId);
    return static_cast<WMError>(ret);
}

WSError SessionProxy::ProcessPointDownSession(int32_t posX, int32_t posY)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        WLOGFE("writeInterfaceToken failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    if (!data.WriteInt32(posX)) {
        WLOGFE("width poX failed.");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    if (!data.WriteInt32(posY)) {
        WLOGFE("width posY failed.");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        WLOGFE("remote is null");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    if (remote->SendRequest(static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_PROCESS_POINT_DOWN_SESSION),
        data, reply, option) != ERR_NONE) {
        WLOGFE("SendRequest failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    return static_cast<WSError>(reply.ReadInt32());
}

WSError SessionProxy::SendPointEventForMoveDrag(const std::shared_ptr<MMI::PointerEvent>& pointerEvent,
    bool isExecuteDelayRaise)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        WLOGFE("writeInterfaceToken failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    if (!pointerEvent->WriteToParcel(data)) {
        WLOGFE("width pointerEvent failed.");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    if (!data.WriteBool(isExecuteDelayRaise)) {
        TLOGE(WmsLogTag::WMS_FOCUS, "write isExecuteDelayRaise failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        WLOGFE("remote is null");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    if (remote->SendRequest(static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_SEND_POINTEREVENT_FOR_MOVE_DRAG),
        data, reply, option) != ERR_NONE) {
        WLOGFE("SendRequest failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    return static_cast<WSError>(reply.ReadInt32());
}

bool SessionProxy::IsStartMoving()
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "writeInterfaceToken failed");
        return false;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "remote is null");
        return false;
    }
    if (remote->SendRequest(static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_IS_START_MOVING),
        data, reply, option) != ERR_NONE) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "SendRequest failed");
        return false;
    }
    bool isMoving = false;
    if (!reply.ReadBool(isMoving)) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "Read isMoving failed");
        return false;
    }
    return isMoving;
}

WMError SessionProxy::SetSystemWindowEnableDrag(bool enableDrag)
{
    TLOGD(WmsLogTag::WMS_LAYOUT, "enableDrag: %{public}d", enableDrag);
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "WriteInterfaceToken failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    if (!data.WriteBool(enableDrag)) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "write enableDrag failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::DEFAULT, "remote is null");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    if (remote->SendRequest(
        static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_SET_SYSTEM_DRAG_ENABLE),
        data, reply, option) != ERR_NONE) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "SendRequest failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    int32_t ret = reply.ReadInt32();
    return static_cast<WMError>(ret);
}

WSError SessionProxy::UpdateRectChangeListenerRegistered(bool isRegister)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "WriteInterfaceToken failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    if (!data.WriteBool(isRegister)) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "write isRegister failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "remote is null");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    if (remote->SendRequest(
        static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_UPDATE_RECTCHANGE_LISTENER_REGISTERED),
        data, reply, option) != ERR_NONE) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "SendRequest failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    int32_t ret = reply.ReadInt32();
    return static_cast<WSError>(ret);
}

void SessionProxy::SetCallingSessionId(const uint32_t callingSessionId)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::WMS_KEYBOARD, "writeInterfaceToken failed");
        return;
    }
    if (!data.WriteUint32(callingSessionId)) {
        TLOGE(WmsLogTag::WMS_KEYBOARD, "Write callingSessionId failed.");
        return;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::DEFAULT, "remote is null");
        return;
    }
    int sendCode = remote->SendRequest(
        static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_SET_CALLING_SESSION_ID),
        data, reply, option);
    if (sendCode != ERR_NONE) {
        TLOGE(WmsLogTag::WMS_KEYBOARD, "SendRequest failed, code: %{public}d", sendCode);
        return;
    }
}

void SessionProxy::SetCustomDecorHeight(int32_t height)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::WMS_DECOR, "writeInterfaceToken failed");
        return;
    }
    if (!data.WriteInt32(height)) {
        TLOGE(WmsLogTag::WMS_DECOR, "Write height failed.");
        return;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::DEFAULT, "remote is null");
        return;
    }
    if (remote->SendRequest(static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_SET_CUSTOM_DECOR_HEIGHT),
        data, reply, option) != ERR_NONE) {
        TLOGE(WmsLogTag::WMS_DECOR, "SendRequest failed");
        return;
    }
}

WSError SessionProxy::SetDecorVisible(bool isVisible)
{
    MessageParcel data;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::WMS_DECOR, "Failed to write interface token");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    if (!data.WriteBool(isVisible)) {
        TLOGE(WmsLogTag::WMS_DECOR, "Failed to write isVisible");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::DEFAULT, "remote is null");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    int sendRet = remote->SendRequest(
        static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_SET_DECOR_VISIBLE), data, reply, option);
    if (sendRet != ERR_NONE) {
        TLOGE(WmsLogTag::WMS_DECOR, "Failed to send request, error = %{public}d", sendRet);
        return WSError::WS_ERROR_IPC_FAILED;
    }
    return WSError::WS_OK;
}

WSError SessionProxy::AdjustKeyboardLayout(const KeyboardLayoutParams& params)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::WMS_KEYBOARD, "WriteInterfaceToken failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    if (!data.WriteParcelable(&params)) {
        TLOGE(WmsLogTag::WMS_KEYBOARD, "keyboard layout params write failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::DEFAULT, "remote is null");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    int sendCode = remote->SendRequest(
        static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_ADJUST_KEYBOARD_LAYOUT),
        data, reply, option);
    if (sendCode != ERR_NONE) {
        TLOGE(WmsLogTag::WMS_KEYBOARD, "SendRequest failed, code: %{public}d", sendCode);
        return WSError::WS_ERROR_IPC_FAILED;
    }
    return static_cast<WSError>(reply.ReadInt32());
}

WSError SessionProxy::ChangeKeyboardEffectOption(const KeyboardEffectOption& effectOption)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::WMS_KEYBOARD, "WriteInterfaceToken failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    if (!data.WriteParcelable(&effectOption)) {
        TLOGE(WmsLogTag::WMS_KEYBOARD, "keyboard layout params write failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::DEFAULT, "remote is null");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    int sendCode = remote->SendRequest(
        static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_CHANGE_KEYBOARD_VIEW_MODE),
        data, reply, option);
    if (sendCode != ERR_NONE) {
        TLOGE(WmsLogTag::WMS_KEYBOARD, "SendRequest failed, code: %{public}d", sendCode);
        return WSError::WS_ERROR_IPC_FAILED;
    }
    return WSError::WS_OK;
}

WMError SessionProxy::UpdateSessionPropertyByAction(const sptr<WindowSessionProperty>& property,
    WSPropertyChangeAction action)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    if (action == WSPropertyChangeAction::ACTION_UPDATE_KEEP_SCREEN_ON ||
        action == WSPropertyChangeAction::ACTION_UPDATE_VIEW_KEEP_SCREEN_ON) {
        option.SetFlags(MessageOption::TF_ASYNC);
    }
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::DEFAULT, "WriteInterfaceToken failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    if (!data.WriteUint64(static_cast<uint64_t>(action))) {
        TLOGE(WmsLogTag::DEFAULT, "Write PropertyChangeAction failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    if (property) {
        if (!data.WriteBool(true) || !property->Write(data, action)) {
            TLOGE(WmsLogTag::DEFAULT, "Write property failed");
            return WMError::WM_ERROR_IPC_FAILED;
        }
    } else {
        if (!data.WriteBool(false)) {
            TLOGE(WmsLogTag::DEFAULT, "Write property failed");
            return WMError::WM_ERROR_IPC_FAILED;
        }
    }

    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::DEFAULT, "remote is null");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    if (remote->SendRequest(static_cast<uint32_t>(
        SessionInterfaceCode::TRANS_ID_UPDATE_SESSION_PROPERTY),
        data, reply, option) != ERR_NONE) {
        TLOGE(WmsLogTag::DEFAULT, "SendRequest failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    int32_t ret = reply.ReadInt32();
    return static_cast<WMError>(ret);
}

WMError SessionProxy::GetAppForceLandscapeConfig(AppForceLandscapeConfig& config)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::WMS_COMPAT, "WriteInterfaceToken failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::WMS_COMPAT, "remote is null");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    int sendCode = remote->SendRequest(
        static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_GET_FORCE_LANDSCAPE_CONFIG),
        data, reply, option);
    if (sendCode != ERR_NONE) {
        TLOGE(WmsLogTag::WMS_COMPAT, "SendRequest failed, code: %{public}d", sendCode);
        return WMError::WM_ERROR_IPC_FAILED;
    }
    sptr<AppForceLandscapeConfig> replyConfig = reply.ReadParcelable<AppForceLandscapeConfig>();
    if (replyConfig) {
        config = *replyConfig;
    }
    int32_t ret = reply.ReadInt32();
    return static_cast<WMError>(ret);
}

WMError SessionProxy::GetAppForceLandscapeConfigEnable(bool& enableForceSplit)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::WMS_COMPAT, "WriteInterfaceToken failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::WMS_COMPAT, "remote is null");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    int sendCode = remote->SendRequest(
        static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_GET_FORCE_LANDSCAPE_CONFIG_ENABLE),
        data, reply, option);
    if (sendCode != ERR_NONE) {
        TLOGE(WmsLogTag::WMS_COMPAT, "SendRequest failed, code: %{public}d", sendCode);
        return WMError::WM_ERROR_IPC_FAILED;
    }
    if (!reply.ReadBool(enableForceSplit)) {
        TLOGE(WmsLogTag::WMS_COMPAT, "Read enableForceSplit failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    int32_t ret = 0;
    if (!reply.ReadInt32(ret)) {
        TLOGE(WmsLogTag::WMS_COMPAT, "read ret failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    return static_cast<WMError>(ret);
}

WMError SessionProxy::GetAppHookWindowInfoFromServer(HookWindowInfo& hookWindowInfo)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "WriteInterfaceToken failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    sptr<IRemoteObject> remote = Remote();
    if (!remote) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "remote is null");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    uint32_t requestCode = static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_GET_HOOK_WINDOW_INFO);
    if (remote->SendRequest(requestCode, data, reply, option) != ERR_NONE) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "SendRequest failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    sptr<HookWindowInfo> replyInfo = reply.ReadParcelable<HookWindowInfo>();
    if (replyInfo) {
        hookWindowInfo = *replyInfo;
    }
    int32_t ret = 0;
    if (!reply.ReadInt32(ret)) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "read ret failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    return static_cast<WMError>(ret);
}

void SessionProxy::NotifyWindowStatusDidChangeAfterShowWindow()
{
    TLOGD(WmsLogTag::WMS_LAYOUT, "in");
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "WriteInterfaceToken failed");
        return;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "remote is null");
        return;
    }
    uint32_t requestCode = static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_NOTIFY_WINDOW_STATUS_AFTER_SHOW_WINDOW);
    int ret = remote->SendRequest(requestCode, data, reply, option);
    if (ret != ERR_NONE) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "SendRequest failed, ret: %{public}d", ret);
    }
}

WSError SessionProxy::SetDialogSessionBackGestureEnabled(bool isEnabled)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::WMS_DIALOG, "WriteInterfaceToken failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    if (!data.WriteBool(isEnabled)) {
        TLOGE(WmsLogTag::WMS_DIALOG, "Write isEnabled failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::WMS_DIALOG, "remote is null");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    if (remote->SendRequest(
        static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_SET_DIALOG_SESSION_BACKGESTURE_ENABLE),
        data, reply, option) != ERR_NONE) {
        TLOGE(WmsLogTag::WMS_DIALOG, "SendRequest failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    int32_t ret = reply.ReadInt32();
    return static_cast<WSError>(ret);
}

int32_t SessionProxy::GetStatusBarHeight()
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    int32_t height = 0;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::WMS_IMMS, "WriteInterfaceToken failed");
        return height;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::WMS_IMMS, "remote is null");
        return height;
    }
    int sendCode = remote->SendRequest(static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_GET_STATUSBAR_HEIGHT),
        data, reply, option);
    if (sendCode != ERR_NONE) {
        TLOGE(WmsLogTag::WMS_IMMS, "SendRequest failed, code: %{public}d", sendCode);
        return height;
    }
    height = reply.ReadInt32();
    return height;
}

void SessionProxy::NotifyExtensionEventAsync(uint32_t notifyEvent)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::WMS_UIEXT, "WriteInterfaceToken failed");
        return;
    }
    if (!data.WriteUint32(notifyEvent)) {
        TLOGE(WmsLogTag::WMS_UIEXT, "Write notifyEvent failed");
        return;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::WMS_UIEXT, "remote is null");
        return;
    }
    int sendCode = remote->SendRequest(
        static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_NOTIFY_EXTENSION_EVENT_ASYNC), data, reply, option);
    if (sendCode != ERR_NONE) {
        TLOGE(WmsLogTag::WMS_UIEXT, "SendRequest failed, code: %{public}d", sendCode);
    }
}

WSError SessionProxy::SendExtensionData(MessageParcel& data, MessageParcel& reply, MessageOption& option)
{
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::WMS_UIEXT, "remote is null");
        return WSError::WS_ERROR_IPC_FAILED;
    }

    auto ret = remote->SendRequest(static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_SEND_EXTENSION_DATA), data,
                                   reply, option);
    if (ret != ERR_NONE) {
        TLOGE(WmsLogTag::WMS_UIEXT, "SendRequest failed, ret code: %{public}u", ret);
        return WSError::WS_ERROR_IPC_FAILED;
    }

    return WSError::WS_OK;
}


void SessionProxy::NotifyExtensionDetachToDisplay()
{
    TLOGD(WmsLogTag::WMS_UIEXT, "UIExtOnLock: UIExtcalled");
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);

    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::WMS_UIEXT, "UIExtOnLock: WriteInterfaceToken failed");
        return;
    }

    sptr<IRemoteObject> remote = Remote();
    if (!remote) {
        TLOGE(WmsLogTag::WMS_UIEXT, "UIExtOnLock: remote is null");
        return;
    }

    auto ret = remote->SendRequest(
        static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_NOTIFY_EXTENSION_DETACH_TO_DISPLAY), data, reply, option);
    if (ret != ERR_NONE) {
        TLOGE(WmsLogTag::WMS_UIEXT, "UIExtOnLock: SendRequest failed, ret code: %{public}u", ret);
    }
}

WMError SessionProxy::SetGestureBackEnabled(bool isEnabled)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::WMS_IMMS, "WriteInterfaceToken failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    if (!data.WriteBool(isEnabled)) {
        TLOGE(WmsLogTag::WMS_IMMS, "Write isEnabled failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::WMS_IMMS, "remote is null");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    if (remote->SendRequest(
        static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_SET_GESTURE_BACK_ENABLE),
        data, reply, option) != ERR_NONE) {
        TLOGE(WmsLogTag::WMS_IMMS, "SendRequest failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    int32_t ret = reply.ReadInt32();
    return static_cast<WMError>(ret);
}

WSError SessionProxy::NotifySubModalTypeChange(SubWindowModalType subWindowModalType)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::WMS_HIERARCHY, "WriteInterfaceToken failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    if (!data.WriteUint32(static_cast<uint32_t>(subWindowModalType))) {
        TLOGE(WmsLogTag::WMS_HIERARCHY, "Write subWindowModalType failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::WMS_HIERARCHY, "remote is null");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    if (remote->SendRequest(static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_SUB_MODAL_TYPE_CHANGE),
        data, reply, option) != ERR_NONE) {
        TLOGE(WmsLogTag::WMS_HIERARCHY, "SendRequest failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    return WSError::WS_OK;
}

WSError SessionProxy::NotifyMainModalTypeChange(bool isModal)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::WMS_HIERARCHY, "WriteInterfaceToken failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    if (!data.WriteBool(isModal)) {
        TLOGE(WmsLogTag::WMS_HIERARCHY, "Write isModal failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::WMS_HIERARCHY, "remote is null");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    if (remote->SendRequest(static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_MAIN_MODAL_TYPE_CHANGE),
        data, reply, option) != ERR_NONE) {
        TLOGE(WmsLogTag::WMS_HIERARCHY, "SendRequest failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    return WSError::WS_OK;
}

WSError SessionProxy::OnSetWindowRectAutoSave(bool enabled, bool isSaveBySpecifiedFlag)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::WMS_MAIN, "WriteInterfaceToken failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    if (!data.WriteBool(enabled)) {
        TLOGE(WmsLogTag::WMS_MAIN, "Write enable failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    if (!data.WriteBool(isSaveBySpecifiedFlag)) {
        TLOGE(WmsLogTag::WMS_MAIN, "Write isSaveBySpecifiedFlag failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    TLOGD(WmsLogTag::WMS_MAIN, "enable: %{public}d, isSaveBySpecifiedFlag: %{public}d",
        enabled, isSaveBySpecifiedFlag);
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::WMS_MAIN, "remote is null");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    if (remote->SendRequest(static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_SET_WINDOW_RECT_AUTO_SAVE),
        data, reply, option) != ERR_NONE) {
        TLOGE(WmsLogTag::WMS_MAIN, "SendRequest failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    return WSError::WS_OK;
}

WSError SessionProxy::NotifySupportWindowModesChange(
    const std::vector<AppExecFwk::SupportWindowMode>& supportedWindowModes)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::WMS_LAYOUT_PC, "WriteInterfaceToken failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    auto size = supportedWindowModes.size();
    if (size > 0 && size <= WINDOW_SUPPORT_MODE_MAX_SIZE) {
        if (!data.WriteUint32(static_cast<uint32_t>(size))) {
            return WSError::WS_ERROR_IPC_FAILED;
        }
        for (decltype(size) i = 0; i < size; i++) {
            if (!data.WriteInt32(static_cast<int32_t>(supportedWindowModes[i]))) {
                return WSError::WS_ERROR_IPC_FAILED;
            }
        }
    } else {
        if (!data.WriteUint32(0)) {
            return WSError::WS_ERROR_IPC_FAILED;
        }
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::WMS_LAYOUT_PC, "remote is null");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    if (remote->SendRequest(static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_SET_SUPPORT_WINDOW_MODES),
        data, reply, option) != ERR_NONE) {
        TLOGE(WmsLogTag::WMS_LAYOUT_PC, "SendRequest failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    return WSError::WS_OK;
}

WSError SessionProxy::SetSessionLabelAndIcon(const std::string& label, const std::shared_ptr<Media::PixelMap>& icon)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::WMS_MAIN, "writeInterfaceToken failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    if (!data.WriteString(label)) {
        TLOGE(WmsLogTag::WMS_MAIN, "write label failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    if (!data.WriteParcelable(icon.get())) {
        TLOGE(WmsLogTag::WMS_MAIN, "write icon failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }

    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::WMS_MAIN, "remote is null");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    if (remote->SendRequest(static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_SET_SESSION_LABEL_AND_ICON),
        data, reply, option) != ERR_NONE) {
        TLOGE(WmsLogTag::WMS_MAIN, "sendRequest failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    int32_t ret = 0;
    if (!reply.ReadInt32(ret)) {
        TLOGE(WmsLogTag::WMS_MAIN, "read ret failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    return static_cast<WSError>(ret);
}

WSError SessionProxy::SetWindowCornerRadius(float cornerRadius)
{
    TLOGD(WmsLogTag::WMS_ATTRIBUTE, "cornerRadius: %{public}f", cornerRadius);
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "WriteInterfaceToken failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    if (!data.WriteFloat(cornerRadius)) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "Write enable failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "remote is null");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    if (remote->SendRequest(static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_SET_WINDOW_CORNER_RADIUS),
        data, reply, option) != ERR_NONE) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "SendRequest failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    return WSError::WS_OK;
}

WSError SessionProxy::SetWindowShadows(const ShadowsInfo& shadowsInfo)
{
    TLOGD(WmsLogTag::WMS_ANIMATION, "shadow radius: %{public}f, color: %{public}s, offsetX: %{public}f, "
        "offsetY: %{public}f", shadowsInfo.radius_, shadowsInfo.color_.c_str(), shadowsInfo.offsetX_,
        shadowsInfo.offsetY_);
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::WMS_ANIMATION, "WriteInterfaceToken failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    if (!data.WriteParcelable(&shadowsInfo)) {
        TLOGE(WmsLogTag::WMS_ANIMATION, "Write shadowsInfo failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::WMS_ANIMATION, "remote is null");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    if (remote->SendRequest(static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_SET_WINDOW_SHADOWS),
        data, reply, option) != ERR_NONE) {
        TLOGE(WmsLogTag::WMS_ANIMATION, "SendRequest failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    return WSError::WS_OK;
}

WSError SessionProxy::SetFollowParentWindowLayoutEnabled(bool isFollow)
{
    TLOGD(WmsLogTag::WMS_SUB, "isFollow: %{public}d", isFollow);
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::WMS_SUB, "WriteInterfaceToken failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    if (!data.WriteBool(isFollow)) {
        TLOGE(WmsLogTag::WMS_SUB, "Write enable failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::WMS_SUB, "remote is null");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    if (remote->SendRequest(static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_SET_FOLLOW_PARENT_LAYOUT_ENABLED),
        data, reply, option) != ERR_NONE) {
        TLOGE(WmsLogTag::WMS_SUB, "SendRequest failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    int32_t ret = 0;
    if (!reply.ReadInt32(ret)) {
        TLOGE(WmsLogTag::WMS_SUB, "read ret failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    return static_cast<WSError>(ret);
}

WSError SessionProxy::SetWindowAnchorInfo(const WindowAnchorInfo& windowAnchorInfo)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::WMS_SUB, "WriteInterfaceToken failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    if (!data.WriteParcelable(&windowAnchorInfo)) {
        TLOGE(WmsLogTag::WMS_SUB, "Write windowAnchorInfo failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    sptr remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::WMS_SUB, "remote is null");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    if (remote->SendRequest(static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_SET_WINDOW_ANCHOR_INFO),
        data, reply, option) != ERR_NONE) {
        TLOGE(WmsLogTag::WMS_SUB, "SendRequest failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    int32_t ret = 0;
    if (!reply.ReadInt32(ret)) {
        TLOGE(WmsLogTag::WMS_SUB, "read ret failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    return static_cast<WSError>(ret);
}


WSError SessionProxy::KeyFrameAnimateEnd()
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "WriteInterfaceToken failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "remote is null");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    if (remote->SendRequest(static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_KEY_FRAME_ANIMATE_END),
        data, reply, option) != ERR_NONE) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "SendRequest failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    int32_t ret = 0;
    if (!reply.ReadInt32(ret)) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "read ret failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    return static_cast<WSError>(ret);
}

WSError SessionProxy::SetWindowTransitionAnimation(WindowTransitionType transitionType,
    const TransitionAnimation& animation)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::WMS_ANIMATION, "WriteInterfaceToken failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    if (!data.WriteUint32(static_cast<uint32_t>(transitionType))) {
        TLOGE(WmsLogTag::WMS_ANIMATION, "Write type failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    if (!data.WriteParcelable(&animation)) {
        TLOGE(WmsLogTag::WMS_ANIMATION, "Write animation failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::WMS_ANIMATION, "Remote is null");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    if (remote->SendRequest(static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_SET_WINDOW_TRANSITION_ANIMATION),
        data, reply, option) != ERR_NONE) {
        TLOGE(WmsLogTag::WMS_ANIMATION, "SendRequest failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    int32_t ret = 0;
    if (!reply.ReadInt32(ret)) {
        TLOGE(WmsLogTag::WMS_ANIMATION, "Read ret failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    return static_cast<WSError>(ret);
}

WSError SessionProxy::UpdateKeyFrameCloneNode(std::shared_ptr<RSWindowKeyFrameNode>& rsKeyFrameNode,
    std::shared_ptr<RSTransaction>& rsTransaction)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "WriteInterfaceToken failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "remote is null");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    if (!rsKeyFrameNode || !rsKeyFrameNode->WriteToParcel(data)) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "write rsKeyFrameNode failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    if (!rsTransaction || !data.WriteParcelable(rsTransaction.get())) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "write rsTransaction failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    if (remote->SendRequest(static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_UPDATE_KEY_FRAME_CLONE_NODE),
        data, reply, option) != ERR_NONE) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "SendRequest failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    int32_t ret = 0;
    if (!reply.ReadInt32(ret)) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "read ret failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    return static_cast<WSError>(ret);
}

WSError SessionProxy::SetDragKeyFramePolicy(const KeyFramePolicy& keyFramePolicy)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "WriteInterfaceToken failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    if (!data.WriteParcelable(&keyFramePolicy)) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "Write keyFramePolicy failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "remote is null");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    if (remote->SendRequest(static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_SET_DRAG_KEY_FRAME_POLICY),
        data, reply, option) != ERR_NONE) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "SendRequest failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    int32_t ret = 0;
    if (!reply.ReadInt32(ret)) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "read ret failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    return static_cast<WSError>(ret);
}

WSError SessionProxy::StartMovingWithCoordinate(int32_t offsetX, int32_t offsetY,
    int32_t pointerPosX, int32_t pointerPosY, DisplayId displayId)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::WMS_LAYOUT_PC, "WriteInterfaceToken failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    if (!data.WriteInt32(offsetX)) {
        TLOGE(WmsLogTag::WMS_LAYOUT_PC, "Write offsetX failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    if (!data.WriteInt32(offsetY)) {
        TLOGE(WmsLogTag::WMS_LAYOUT_PC, "Write offsetY failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    if (!data.WriteInt32(pointerPosX)) {
        TLOGE(WmsLogTag::WMS_LAYOUT_PC, "Write pointerPosX failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    if (!data.WriteInt32(pointerPosY)) {
        TLOGE(WmsLogTag::WMS_LAYOUT_PC, "Write pointerPosY failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    if (!data.WriteUint64(displayId)) {
        TLOGE(WmsLogTag::WMS_LAYOUT_PC, "Write displayId failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::WMS_LAYOUT_PC, "remote is null");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    if (remote->SendRequest(static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_START_MOVING_WITH_COORDINATE),
        data, reply, option) != ERR_NONE) {
        TLOGE(WmsLogTag::WMS_LAYOUT_PC, "SendRequest failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    int32_t ret = reply.ReadInt32();
    return static_cast<WSError>(ret);
}
WSError SessionProxy::GetCrossAxisState(CrossAxisState& state)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::WMS_LAYOUT_PC, "writeInterfaceToken failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::WMS_LAYOUT_PC, "remote is null");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    if (remote->SendRequest(static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_GET_CROSS_AXIS_STATE),
        data, reply, option) != ERR_NONE) {
        TLOGE(WmsLogTag::WMS_LAYOUT_PC, "sendRequest failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    uint32_t ret = 0;
    if (!reply.ReadUint32(ret)) {
        TLOGE(WmsLogTag::WMS_LAYOUT_PC, "read ret failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    state = static_cast<CrossAxisState>(ret);
    return WSError::WS_OK;
}

WSError SessionProxy::GetWaterfallMode(bool& isWaterfallMode)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "writeInterfaceToken failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "remote is null");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    if (remote->SendRequest(static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_GET_WATERFALL_MODE),
        data, reply, option) != ERR_NONE) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "sendRequest failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    isWaterfallMode = reply.ReadBool();
    return WSError::WS_OK;
}

WMError SessionProxy::IsMainWindowFullScreenAcrossDisplays(bool& isAcrossDisplays)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "writeInterfaceToken failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "remote is null");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    uint32_t code = static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_MAIN_WINDOW_FULL_SCREEN_ACROSS_DISPLAYS);
    if (remote->SendRequest(code, data, reply, option) != ERR_NONE) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "sendRequest failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    int32_t ret = 0;
    if (!reply.ReadInt32(ret)) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "Read ret failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    if (!reply.ReadBool(isAcrossDisplays)) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "Read isAcrossDisplays failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    return static_cast<WMError>(ret);
}

WSError SessionProxy::OnContainerModalEvent(const std::string& eventName, const std::string& eventValue)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::WMS_EVENT, "WriteInterfaceToken failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    if (!data.WriteString(eventName)) {
        TLOGE(WmsLogTag::WMS_EVENT, "Write eventName failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    if (!data.WriteString(eventValue)) {
        TLOGE(WmsLogTag::WMS_EVENT, "Write eventValue failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::WMS_EVENT, "remote is null");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    if (remote->SendRequest(static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_CONTAINER_MODAL_EVENT),
        data, reply, option) != ERR_NONE) {
        TLOGE(WmsLogTag::WMS_EVENT, "SendRequest failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    return WSError::WS_OK;
}

WSError SessionProxy::NotifyFollowParentMultiScreenPolicy(bool enabled)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::WMS_SUB, "WriteInterfaceToken failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    if (!data.WriteBool(enabled)) {
        TLOGE(WmsLogTag::WMS_SUB, "Write enabled failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::WMS_SUB, "remote is null");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    if (remote->SendRequest(static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_SET_FOLLOW_PARENT_MULTI_SCREEN_POLICY),
        data, reply, option) != ERR_NONE) {
        TLOGE(WmsLogTag::WMS_SUB, "SendRequest failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    return WSError::WS_OK;
}

void SessionProxy::NotifyWindowAttachStateListenerRegistered(bool registered)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::WMS_MAIN, "WriteInterfaceToken failed");
        return;
    }
    if (!data.WriteBool(registered)) {
        TLOGE(WmsLogTag::WMS_MAIN, "Write enable failed");
        return;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::WMS_MAIN, "remote is null");
        return;
    }
    int sendCode = remote->SendRequest(
        static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_NOTIFY_WINDOW_ATTACH_STATE_LISTENER_REGISTERED),
        data, reply, option);
    if (sendCode != ERR_NONE) {
        TLOGE(WmsLogTag::WMS_MAIN, "SendRequest failed, code: %{public}d", sendCode);
    }
}

void SessionProxy::NotifyKeyboardWillShowRegistered(bool registered)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::WMS_KEYBOARD, "writeInterfaceToken failed");
        return;
    }
    if (!data.WriteBool(registered)) {
        TLOGE(WmsLogTag::WMS_KEYBOARD, "Write registered failed.");
        return;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::WMS_KEYBOARD, "Remote is null");
        return;
    }
    int sendCode = remote->SendRequest(
        static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_SET_KEYBOARD_WILL_SHOW_REGISTERED),
        data, reply, option);
    if (sendCode != ERR_NONE) {
        TLOGE(WmsLogTag::WMS_KEYBOARD, "SendRequest failed, code: %{public}d", sendCode);
    }
}

void SessionProxy::NotifyKeyboardWillHideRegistered(bool registered)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::WMS_KEYBOARD, "writeInterfaceToken failed");
        return;
    }
    if (!data.WriteBool(registered)) {
        TLOGE(WmsLogTag::WMS_KEYBOARD, "Write registered failed.");
        return;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::WMS_KEYBOARD, "Remote is null");
        return;
    }
    int sendCode = remote->SendRequest(
        static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_SET_KEYBOARD_WILL_HIDE_REGISTERED),
        data, reply, option);
    if (sendCode != ERR_NONE) {
        TLOGE(WmsLogTag::WMS_KEYBOARD, "SendRequest failed, code: %{public}d", sendCode);
    }
}

void SessionProxy::NotifyKeyboardDidShowRegistered(bool registered)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::WMS_KEYBOARD, "writeInterfaceToken failed");
        return;
    }
    if (!data.WriteBool(registered)) {
        TLOGE(WmsLogTag::WMS_KEYBOARD, "Write registered failed.");
        return;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::WMS_KEYBOARD, "Remote is null");
        return;
    }
    int sendCode = remote->SendRequest(
        static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_SET_KEYBOARD_DID_SHOW_REGISTERED),
        data, reply, option);
    if (sendCode != ERR_NONE) {
        TLOGE(WmsLogTag::WMS_KEYBOARD, "SendRequest failed, code: %{public}d", sendCode);
    }
}

void SessionProxy::NotifyKeyboardDidHideRegistered(bool registered)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::WMS_KEYBOARD, "writeInterfaceToken failed");
        return;
    }
    if (!data.WriteBool(registered)) {
        TLOGE(WmsLogTag::WMS_KEYBOARD, "Write registered failed.");
        return;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::WMS_KEYBOARD, "Remote is null");
        return;
    }
    int sendCode = remote->SendRequest(
        static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_SET_KEYBOARD_DID_HIDE_REGISTERED),
        data, reply, option);
    if (sendCode != ERR_NONE) {
        TLOGE(WmsLogTag::WMS_KEYBOARD, "SendRequest failed, code: %{public}d", sendCode);
    }
}

WSError SessionProxy::UpdateFlag(const std::string& flag)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::WMS_MAIN, "WriteInterfaceToken failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    if (!data.WriteString(flag)) {
        TLOGE(WmsLogTag::WMS_MAIN, "Write flag failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    TLOGD(WmsLogTag::WMS_MAIN, "specifiedFlag: %{public}s", flag.c_str());
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::WMS_MAIN, "remote is null");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    if (remote->SendRequest(
        static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_UPDATE_FLAG),
        data, reply, option) != ERR_NONE) {
        TLOGE(WmsLogTag::WMS_MAIN, "SendRequest failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    return WSError::WS_OK;
}

WSError SessionProxy::UpdatePiPTemplateInfo(PiPTemplateInfo& pipTemplateInfo)
{
    TLOGD(WmsLogTag::WMS_PIP, "UpdatePiPTemplateInfo, pipTemplateType: %{public}u, priority: %{public}d, "
        "defaultWindowSizeType: %{public}d, cornerAdsorptionEnabled: %{public}d", pipTemplateInfo.pipTemplateType,
        pipTemplateInfo.priority, pipTemplateInfo.defaultWindowSizeType,  pipTemplateInfo.cornerAdsorptionEnabled);
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::WMS_PIP, "writeInterfaceToken failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    if (!data.WriteParcelable(&pipTemplateInfo)) {
        TLOGE(WmsLogTag::WMS_PIP, "write pipTemplateInfo failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::WMS_PIP, "remote is null");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    if (remote->SendRequest(static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_UPDATE_PIP_TEMPLATE_INFO),
        data, reply, option) != ERR_NONE) {
        TLOGE(WmsLogTag::WMS_PIP, "SendRequest failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    int32_t ret = 0;
    if (!reply.ReadInt32(ret)) {
        TLOGE(WmsLogTag::WMS_PIP, "read ret failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    return static_cast<WSError>(ret);
}

WSError SessionProxy::SetPipParentWindowId(uint32_t windowId)
{
    TLOGD(WmsLogTag::WMS_PIP, "ParentWindowId: %{public}u", windowId);
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::WMS_PIP, "writeInterfaceToken failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    if (!data.WriteUint32(windowId)) {
        TLOGE(WmsLogTag::WMS_PIP, "write pipParentWindowId failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::WMS_PIP, "remote is null");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    if (remote->SendRequest(static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_SET_PIP_PARENT_WINDOWID),
        data, reply, option) != ERR_NONE) {
        TLOGE(WmsLogTag::WMS_PIP, "SendRequest failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    int32_t ret = 0;
    if (!reply.ReadInt32(ret)) {
        TLOGE(WmsLogTag::WMS_PIP, "read ret failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    return static_cast<WSError>(ret);
}

WMError SessionProxy::IsPiPActive(bool& status)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::WMS_PIP, "writeInterfaceToken failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::WMS_PIP, "remote is null");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    if (remote->SendRequest(static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_IS_PIP_ACTIVE),
        data, reply, option) != ERR_NONE) {
        TLOGE(WmsLogTag::WMS_PIP, "SendRequest failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    int32_t ret = 0;
    if (!reply.ReadInt32(ret)) {
        TLOGE(WmsLogTag::WMS_PIP, "read return code failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    if (!reply.ReadBool(status)) {
        TLOGE(WmsLogTag::WMS_PIP, "read status failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    return static_cast<WMError>(ret);
}

WSError SessionProxy::UpdateRotationChangeRegistered(int32_t persistentId, bool isRegister)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::WMS_ROTATION, "WriteInterfaceToken failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    if (!data.WriteInt32(persistentId)) {
        TLOGE(WmsLogTag::WMS_ROTATION, "Write persistentId failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    if (!data.WriteBool(isRegister)) {
        TLOGE(WmsLogTag::WMS_ROTATION, "Write isRegister failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::WMS_ROTATION, "remote is null");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    if (remote->SendRequest(
        static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_UPDATE_ROTATION_CHANGE),
        data, reply, option) != ERR_NONE) {
        TLOGE(WmsLogTag::WMS_ROTATION, "SendRequest failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    return WSError::WS_OK;
}

WMError SessionProxy::UpdateScreenshotAppEventRegistered(int32_t persistentId, bool isRegister)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "WriteInterfaceToken failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    if (!data.WriteInt32(persistentId)) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "Write persistentId failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    if (!data.WriteBool(isRegister)) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "Write isRegister failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "remote is null");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    if (remote->SendRequest(
        static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_UPDATE_SCREEN_SHOT_APP_EVENT_REGISTERED),
        data, reply, option) != ERR_NONE) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "SendRequest failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    int32_t ret = 0;
    if (!reply.ReadInt32(ret)) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "Read ret failed.");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    return static_cast<WMError>(ret);
}

WMError SessionProxy::UpdateAcrossDisplaysChangeRegistered(bool isRegister)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "WriteInterfaceToken failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    if (!data.WriteBool(isRegister)) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "Write isRegister failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "remote is null");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    uint32_t code = static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_UPDATE_ACROSS_DISPLAYS_REGISTERED);
    if (remote->SendRequest(code, data, reply, option) != ERR_NONE) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "SendRequest failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    int32_t ret = 0;
    if (!reply.ReadInt32(ret)) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "Read ret failed.");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    return static_cast<WMError>(ret);
}

WSError SessionProxy::RequestFocus(bool isFocused)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::WMS_FOCUS, "WriteInterfaceToken failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    if (!data.WriteBool(isFocused)) {
        TLOGE(WmsLogTag::WMS_FOCUS, "Write isFocused failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::WMS_FOCUS, "remote is null");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    if (remote->SendRequest(
        static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_REQUEST_FOCUS),
        data, reply, option) != ERR_NONE) {
        TLOGE(WmsLogTag::WMS_FOCUS, "SendRequest failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    int32_t ret = reply.ReadInt32();
    return static_cast<WSError>(ret);
}

WSError SessionProxy::GetIsHighlighted(bool& isHighlighted)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::WMS_FOCUS, "WriteInterfaceToken failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::WMS_FOCUS, "remote is null");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    if (remote->SendRequest(
        static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_GET_IS_HIGHLIGHTED),
        data, reply, option) != ERR_NONE) {
        TLOGE(WmsLogTag::WMS_FOCUS, "SendRequest failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    if (!reply.ReadBool(isHighlighted)) {
        TLOGE(WmsLogTag::WMS_FOCUS, "Read isHighlighted failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    return WSError::WS_OK;
}

WMError SessionProxy::NotifyDisableDelegatorChange()
{
    TLOGD(WmsLogTag::WMS_LIFE, "in");
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::WMS_LIFE, "WriteInterfaceToken failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::WMS_LIFE, "remote is null");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    if (remote->SendRequest(
        static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_NOTIFY_DISABLE_DELEGATOR_CHANGE),
        data, reply, option) != ERR_NONE) {
        TLOGE(WmsLogTag::WMS_LIFE, "SendRequest failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    int32_t ret = 0;
    if (!reply.ReadInt32(ret)) {
        TLOGE(WmsLogTag::WMS_LIFE, "Read ret failed.");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    return static_cast<WMError>(ret);
}

WSError OHOS::Rosen::SessionProxy::UseImplicitAnimation(bool useImplicit)
{
    TLOGD(WmsLogTag::WMS_PC, "in");
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::WMS_PC, "WriteInterfaceToken failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    if (!data.WriteBool(useImplicit)) {
        TLOGE(WmsLogTag::WMS_PC, "write useImplicit failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::WMS_PC, "remote is null");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    if (remote->SendRequest(
        static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_USE_IMPLICT_ANIMATION),
        data, reply, option) != ERR_NONE) {
        TLOGE(WmsLogTag::WMS_PC, "SendRequest failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    int32_t ret = 0;
    if (!reply.ReadInt32(ret)) {
        TLOGE(WmsLogTag::WMS_LIFE, "Read ret failed.");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    return static_cast<WSError>(ret);
}

WSError SessionProxy::SetSubWindowSource(SubWindowSource source)
{
    TLOGD(WmsLogTag::WMS_SUB, "source: %{public}d", source);
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::WMS_SUB, "WriteInterfaceToken failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    if (!data.WriteUint32(static_cast<uint32_t>(source))) {
        TLOGE(WmsLogTag::WMS_SUB, "Write source failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    sptr remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::WMS_SUB, "remote is null");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    if (remote->SendRequest(static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_SET_SUBWINDOW_SOURCE),
        data, reply, option) != ERR_NONE) {
        TLOGE(WmsLogTag::WMS_SUB, "SendRequest failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    int32_t ret = 0;
    if (!reply.ReadInt32(ret)) {
        TLOGE(WmsLogTag::WMS_SUB, "read ret failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    return static_cast<WSError>(ret);
}

WSError SessionProxy::SetFrameRectForPartialZoomIn(const Rect& frameRect)
{
    TLOGD(WmsLogTag::WMS_ANIMATION, "in");
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::WMS_ANIMATION, "WriteInterfaceToken failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    if (!(data.WriteInt32(frameRect.posX_) && data.WriteInt32(frameRect.posY_) &&
          data.WriteUint32(frameRect.width_) && data.WriteUint32(frameRect.height_))) {
        TLOGE(WmsLogTag::WMS_ANIMATION, "Write frame rect failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }

    auto remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::WMS_ANIMATION, "remote is null");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    if (remote->SendRequest(static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_SET_FRAMERECT_FOR_PARTIAL_ZOOMIN),
        data, reply, option) != ERR_NONE) {
        TLOGE(WmsLogTag::WMS_ANIMATION, "SendRequest failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    int32_t ret = 0;
    if (!reply.ReadInt32(ret)) {
        TLOGE(WmsLogTag::WMS_ANIMATION, "read ret failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    return static_cast<WSError>(ret);
}

void SessionProxy::NotifyFloatingBallPrepareClose()
{
    TLOGI(WmsLogTag::WMS_PIP, "NotifyFloatingBallPrepareClose");
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::WMS_SYSTEM, "writeInterfaceToken failed");
        return;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::WMS_SYSTEM, "remote is null");
        return;
    }
    auto errCode = remote->SendRequest(
        static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_NOTIFY_FLOATING_BALL_PREPARE_CLOSE), data, reply, option);
    if (errCode != ERR_NONE) {
        TLOGE(WmsLogTag::WMS_SYSTEM, "SendRequest failed code: %{public}d", errCode);
        return;
    }
}

WSError SessionProxy::NotifyIsFullScreenInForceSplitMode(bool isFullScreen)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::WMS_COMPAT, "WriteInterfaceToken failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    if (!data.WriteBool(isFullScreen)) {
        TLOGE(WmsLogTag::WMS_COMPAT, "write isFullScreen failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    auto remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::WMS_COMPAT, "remote is null");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    if (remote->SendRequest(static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_NOTIFY_IS_FULL_SCREEN_IN_FORCE_SPLIT),
        data, reply, option) != ERR_NONE) {
        TLOGE(WmsLogTag::WMS_COMPAT, "SendRequest failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    int32_t ret = 0;
    if (!reply.ReadInt32(ret)) {
        TLOGE(WmsLogTag::WMS_COMPAT, "read ret failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    return static_cast<WSError>(ret);
}

WSError SessionProxy::NotifyCompatibleModeChange(CompatibleStyleMode mode)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::WMS_COMPAT, "WriteInterfaceToken failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    if (!data.WriteInt32(static_cast<int32_t>(mode))) {
        TLOGE(WmsLogTag::WMS_COMPAT, "write mode failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    auto remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::WMS_COMPAT, "remote is null");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    if (remote->SendRequest(static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_NOTIFY_COMPATIBLE_MODE_CHANGE),
        data, reply, option) != ERR_NONE) {
        TLOGE(WmsLogTag::WMS_COMPAT, "SendRequest failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    return WSError::WS_OK;
}

WSError SessionProxy::NotifyAppForceLandscapeConfigEnableUpdated()
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::WMS_COMPAT, "WriteInterfaceToken failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }

    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::WMS_COMPAT, "remote is null");
        return WSError::WS_ERROR_IPC_FAILED;
    }

    if (remote->SendRequest(
        static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_GET_FORCE_LANDSCAPE_CONFIG_ENABLE),
        data, reply, option) != ERR_NONE) {
        TLOGE(WmsLogTag::WMS_COMPAT, "SendRequest failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    return WSError::WS_OK;
}

WSError SessionProxy::RestartApp(const std::shared_ptr<AAFwk::Want>& want)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (want == nullptr) {
        TLOGE(WmsLogTag::WMS_LIFE, "want is null");
        return WSError::WS_ERROR_INVALID_PARAM;
    }
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::WMS_LIFE, "WriteInterfaceToken failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    if (!data.WriteParcelable(want.get())) {
        TLOGE(WmsLogTag::WMS_LIFE, "write want failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::WMS_LIFE, "remote is null");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    auto errCode = remote->SendRequest(
        static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_RESTART_APP), data, reply, option);
    if (errCode != ERR_NONE) {
        TLOGE(WmsLogTag::WMS_LIFE, "SendRequest failed code: %{public}d", errCode);
        return WSError::WS_ERROR_IPC_FAILED;
    }
    int32_t ret = 0;
    if (!reply.ReadInt32(ret)) {
        TLOGE(WmsLogTag::WMS_LIFE, "read reply failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    return static_cast<WSError>(ret);
}

WMError SessionProxy::SendCommonEvent(int32_t command, const std::vector<int32_t>& parameters)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::WMS_EVENT, "Write interface token failed.");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    if (!data.WriteInt32(command)) {
        TLOGE(WmsLogTag::WMS_EVENT, "Write command failed.");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    for (auto dataInfo : parameters) {
        if (!data.WriteInt32(dataInfo)) {
            TLOGE(WmsLogTag::WMS_EVENT, "Write dataInfo failed.");
            return WMError::WM_ERROR_IPC_FAILED;
        }
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::WMS_EVENT, "Remote is null.");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    auto sendRet = remote->SendRequest(static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_SEND_COMMAND_EVENT),
        data, reply, option);
    if (sendRet != ERR_NONE) {
        TLOGE(WmsLogTag::WMS_EVENT, "Send request failed.code: %{public}d", sendRet);
        return WMError::WM_ERROR_IPC_FAILED;
    }
    int32_t ret = 0;
    if (!reply.ReadInt32(ret)) {
        TLOGE(WmsLogTag::WMS_EVENT, "Read reply failed.");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    return static_cast<WMError>(ret);
}
} // namespace OHOS::Rosen
