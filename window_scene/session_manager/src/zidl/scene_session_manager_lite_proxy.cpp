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

#include "session_manager/include/zidl/scene_session_manager_lite_proxy.h"

#include "marshalling_helper.h"
#include "window_manager_hilog.h"

namespace OHOS::Rosen {
namespace {
constexpr int32_t CYCLE_LIMIT = 1000;
constexpr HiviewDFX::HiLogLabel LABEL = {LOG_CORE, HILOG_DOMAIN_WINDOW, "SceneSessionManagerLiteProxy"};
constexpr int32_t MAX_TOPN_INFO_SIZE = 200;
}

WSError SceneSessionManagerLiteProxy::SetSessionLabel(const sptr<IRemoteObject>& token, const std::string& label)
{
    WLOGFD("run SceneSessionManagerLiteProxy::SetSessionLabel");
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        WLOGFE("WriteInterfaceToken failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    if (!data.WriteRemoteObject(token)) {
        WLOGFE("Write token failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    if (!data.WriteString(label)) {
        WLOGFE("Write label failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }

    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        WLOGFE("remote is null");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    if (remote->SendRequest(static_cast<uint32_t>(SceneSessionManagerLiteMessage::TRANS_ID_SET_SESSION_LABEL),
        data, reply, option) != ERR_NONE) {
        WLOGFE("SendRequest failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    return static_cast<WSError>(reply.ReadInt32());
}

WSError SceneSessionManagerLiteProxy::SetSessionIcon(const sptr<IRemoteObject>& token,
    const std::shared_ptr<Media::PixelMap>& icon)
{
    WLOGFD("run SceneSessionManagerLiteProxy::SetSessionIcon");
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        WLOGFE("WriteInterfaceToken failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    if (!data.WriteRemoteObject(token)) {
        WLOGFE("Write token failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    if (!data.WriteParcelable(icon.get())) {
        WLOGFE("Write icon failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }

    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        WLOGFE("remote is null");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    if (remote->SendRequest(static_cast<uint32_t>(SceneSessionManagerLiteMessage::TRANS_ID_SET_SESSION_ICON),
        data, reply, option) != ERR_NONE) {
        WLOGFE("SendRequest failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    return static_cast<WSError>(reply.ReadInt32());
}

WSError SceneSessionManagerLiteProxy::IsValidSessionIds(
    const std::vector<int32_t>& sessionIds, std::vector<bool>& results)
{
    WLOGFD("run SceneSessionManagerLiteProxy::IsValidSessionIds");
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        WLOGFE("WriteInterfaceToken failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    if (!data.WriteInt32Vector(sessionIds)) {
        WLOGFE("Write sessionIds failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }

    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        WLOGFE("remote is null");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    if (remote->SendRequest(static_cast<uint32_t>(SceneSessionManagerLiteMessage::TRANS_ID_IS_VALID_SESSION_IDS),
        data, reply, option) != ERR_NONE) {
        WLOGFE("SendRequest failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }

    reply.ReadBoolVector(&results);
    return static_cast<WSError>(reply.ReadInt32());
}

WSError SceneSessionManagerLiteProxy::PendingSessionToForeground(const sptr<IRemoteObject>& token)
{
    WLOGFD("run SceneSessionManagerLiteProxy::PendingSessionToForeground");
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        WLOGFE("Write interfaceToken failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }

    if (!data.WriteRemoteObject(token)) {
        WLOGFE("Write token failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }

    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        WLOGFE("remote is null");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    if (remote->SendRequest(static_cast<uint32_t>(
        SceneSessionManagerLiteMessage::TRANS_ID_PENDING_SESSION_TO_FOREGROUND),
        data, reply, option) != ERR_NONE) {
        WLOGFE("SendRequest failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    return static_cast<WSError>(reply.ReadInt32());
}

WSError SceneSessionManagerLiteProxy::PendingSessionToBackgroundForDelegator(const sptr<IRemoteObject>& token,
    bool shouldBackToCaller)
{
    TLOGD(WmsLogTag::WMS_LIFE, "run");
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::WMS_LIFE, "Write interfaceToken failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }

    if (!data.WriteRemoteObject(token)) {
        TLOGE(WmsLogTag::WMS_LIFE, "Write token failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }

    if (!data.WriteBool(shouldBackToCaller)) {
        TLOGE(WmsLogTag::WMS_LIFE, "Write shouldBackToCaller failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }

    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::WMS_LIFE, "remote is null");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    if (remote->SendRequest(static_cast<uint32_t>(
        SceneSessionManagerLiteMessage::TRANS_ID_PENDING_SESSION_TO_BACKGROUND_FOR_DELEGATOR),
        data, reply, option) != ERR_NONE) {
        TLOGE(WmsLogTag::WMS_LIFE, "SendRequest failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    return static_cast<WSError>(reply.ReadInt32());
}

WSError SceneSessionManagerLiteProxy::RegisterSessionListener(const sptr<ISessionListener>& listener, bool isRecover)
{
    WLOGFD("run SceneSessionManagerLiteProxy::RegisterSessionListener");
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    if (listener == nullptr) {
        WLOGFE("register mission listener, listener is nullptr");
        return WSError::WS_ERROR_INVALID_PARAM;
    }
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        WLOGFE("WriteInterfaceToken failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    if (!data.WriteRemoteObject(listener->AsObject())) {
        WLOGFE("write mission listener failed when register mission listener.");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        WLOGFE("remote is null");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    if (remote->SendRequest(static_cast<uint32_t>(
        SceneSessionManagerLiteMessage::TRANS_ID_REGISTER_SESSION_LISTENER),
        data, reply, option) != ERR_NONE) {
        WLOGFE("SendRequest failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    return static_cast<WSError>(reply.ReadInt32());
}

WSError SceneSessionManagerLiteProxy::UnRegisterSessionListener(const sptr<ISessionListener>& listener)
{
    WLOGFD("run SceneSessionManagerLiteProxy::UnRegisterSessionListener");
    if (listener == nullptr) {
        WLOGFE("unregister mission listener, listener is nullptr");
        return WSError::WS_ERROR_INVALID_PARAM;
    }
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        WLOGFE("WriteInterfaceToken failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    if (!data.WriteRemoteObject(listener->AsObject())) {
        WLOGFE("write mission listener failed when unregister mission listener.");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        WLOGFE("remote is null");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    if (remote->SendRequest(
        static_cast<uint32_t>(SceneSessionManagerLiteMessage::TRANS_ID_UNREGISTER_SESSION_LISTENER),
        data, reply, option) != ERR_NONE) {
        WLOGFE("SendRequest failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    return static_cast<WSError>(reply.ReadInt32());
}

WSError SceneSessionManagerLiteProxy::GetSessionInfos(const std::string& deviceId, int32_t numMax,
    std::vector<SessionInfoBean>& sessionInfos)
{
    WLOGFD("run SceneSessionManagerLiteProxy::GetSessionInfos");
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        WLOGFE("WriteInterfaceToken failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    if (!data.WriteString16(Str8ToStr16(deviceId))) {
        WLOGFE("GetSessionInfos write deviceId failed.");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    if (!data.WriteInt32(numMax)) {
        WLOGFE("GetSessionInfos numMax write failed.");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        WLOGFE("remote is null");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    if (remote->SendRequest(static_cast<uint32_t>(SceneSessionManagerLiteMessage::TRANS_ID_GET_MISSION_INFOS),
        data, reply, option) != ERR_NONE) {
        WLOGFE("SendRequest failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    WSError error = GetParcelableInfos(reply, sessionInfos);
    if (error != WSError::WS_OK) {
        WLOGFE("GetSessionInfos error");
        return error;
    }
    return static_cast<WSError>(reply.ReadInt32());
}

WSError SceneSessionManagerLiteProxy::GetMainWindowStatesByPid(int32_t pid, std::vector<MainWindowState>& windowStates)
{
    TLOGD(WmsLogTag::WMS_LIFE, "run");
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::WMS_LIFE, "remote is nullptr");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::WMS_LIFE, "WriteInterfaceToken failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    if (!data.WriteInt32(pid)) {
        TLOGE(WmsLogTag::WMS_LIFE, "write pid failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    if (remote->SendRequest(
        static_cast<uint32_t>(SceneSessionManagerLiteMessage::TRANS_ID_GET_MAIN_WINDOW_STATES_BY_PID),
        data, reply, option) != ERR_NONE) {
        TLOGE(WmsLogTag::WMS_LIFE, "SendRequest failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    WSError error = GetParcelableInfos(reply, windowStates);
    if (error != WSError::WS_OK) {
        TLOGE(WmsLogTag::WMS_LIFE, "GetWindowStates error");
        return error;
    }
    return static_cast<WSError>(reply.ReadInt32());
}

WSError SceneSessionManagerLiteProxy::GetSessionInfo(const std::string& deviceId, int32_t persistentId,
    SessionInfoBean& sessionInfo)
{
    WLOGFD("run SceneSessionManagerLiteProxy::GetSessionInfo");
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        WLOGFE("WriteInterfaceToken failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    if (!data.WriteString16(Str8ToStr16(deviceId))) {
        WLOGFE("GetSessionInfo write deviceId failed.");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    if (!data.WriteInt32(persistentId)) {
        WLOGFE("GetSessionInfo write persistentId failed.");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        WLOGFE("remote is null");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    if (remote->SendRequest(static_cast<uint32_t>(SceneSessionManagerLiteMessage::TRANS_ID_GET_MISSION_INFO_BY_ID),
        data, reply, option) != ERR_NONE) {
        WLOGFE("SendRequest failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    std::unique_ptr<SessionInfoBean> info(reply.ReadParcelable<SessionInfoBean>());
    if (info == nullptr) {
        WLOGFE("read missioninfo failed.");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    sessionInfo = *info;
    return static_cast<WSError>(reply.ReadInt32());
}

WSError SceneSessionManagerLiteProxy::GetSessionInfoByContinueSessionId(
    const std::string& continueSessionId, SessionInfoBean& sessionInfo)
{
    TLOGI(WmsLogTag::WMS_LIFE, "continueSessionId: %{public}s", continueSessionId.c_str());
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::WMS_LIFE, "WriteInterfaceToken failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    if (!data.WriteString(continueSessionId)) {
        TLOGE(WmsLogTag::WMS_LIFE, "GetSessionInfoByContinueSessionId write continueSessionId failed.");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::WMS_LIFE, "remote is null");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    if (remote->SendRequest(static_cast<uint32_t>(
        SceneSessionManagerLiteMessage::TRANS_ID_GET_SESSION_INFO_BY_CONTINUE_SESSION_ID),
        data, reply, option) != ERR_NONE) {
        TLOGE(WmsLogTag::WMS_LIFE, "SendRequest failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    sptr<SessionInfoBean> info(reply.ReadParcelable<SessionInfoBean>());
    if (info == nullptr) {
        TLOGE(WmsLogTag::WMS_LIFE, "read sessioninfo failed.");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    sessionInfo = *info;
    return static_cast<WSError>(reply.ReadInt32());
}

template<typename T>
WSError SceneSessionManagerLiteProxy::GetParcelableInfos(MessageParcel& reply, std::vector<T>& parcelableInfos)
{
    int32_t infoSize = reply.ReadInt32();
    if (infoSize > CYCLE_LIMIT || infoSize < 0) {
        WLOGFE("infoSize is too large or negative");
        return WSError::WS_ERROR_IPC_FAILED;
    }

    for (int32_t i = 0; i < infoSize; i++) {
        std::unique_ptr<T> info(reply.ReadParcelable<T>());
        if (!info) {
            WLOGFE("Read Parcelable infos failed.");
            return WSError::WS_ERROR_IPC_FAILED;
        }
        parcelableInfos.emplace_back(*info);
    }
    return WSError::WS_OK;
}

WSError SceneSessionManagerLiteProxy::TerminateSessionNew(const sptr<AAFwk::SessionInfo> abilitySessionInfo,
    bool needStartCaller, bool isFromBroker)
{
    if (abilitySessionInfo == nullptr) {
        WLOGFE("abilitySessionInfo is null");
        return WSError::WS_ERROR_INVALID_SESSION;
    }
    MessageParcel data, reply;
    MessageOption option(MessageOption::TF_ASYNC);
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        WLOGFE("WriteInterfaceToken failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    if (!data.WriteParcelable(abilitySessionInfo)) {
        WLOGFE("write abilitySessionInfo failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    if (!data.WriteBool(needStartCaller)) {
        WLOGFE("Write needStartCaller failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    if (!data.WriteBool(isFromBroker)) {
        WLOGFE("Write isFromBroker failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        WLOGFE("remote is null");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    if (remote->SendRequest(static_cast<uint32_t>(SceneSessionManagerLiteMessage::TRANS_ID_TERMINATE_SESSION_NEW),
        data, reply, option) != ERR_NONE) {
        WLOGFE("SendRequest failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    return static_cast<WSError>(reply.ReadInt32());
}

WSError SceneSessionManagerLiteProxy::GetFocusSessionToken(sptr<IRemoteObject>& token, DisplayId displayId)
{
    WLOGFD("run SceneSessionManagerLiteProxy::GetFocusSessionToken");
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        WLOGFE("Write interfaceToken failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    if (!data.WriteUint64(displayId)) {
        TLOGE(WmsLogTag::WMS_FOCUS, "write displayId failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        WLOGFE("remote is null");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    if (remote->SendRequest(static_cast<uint32_t>(SceneSessionManagerLiteMessage::TRANS_ID_GET_FOCUS_SESSION_TOKEN),
        data, reply, option) != ERR_NONE) {
        WLOGFE("SendRequest failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    token = reply.ReadRemoteObject();
    if (token == nullptr) {
        WLOGFE("get token nullptr.");
    }
    return static_cast<WSError>(reply.ReadInt32());
}

WSError SceneSessionManagerLiteProxy::GetFocusSessionElement(AppExecFwk::ElementName& element, DisplayId displayId)
{
    WLOGFD("run SceneSessionManagerLiteProxy::GetFocusSessionElement");
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        WLOGFE("Write interfaceToken failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    if (!data.WriteUint64(displayId)) {
        TLOGE(WmsLogTag::WMS_FOCUS, "write displayId failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        WLOGFE("remote is null");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    if (remote->SendRequest(static_cast<uint32_t>(SceneSessionManagerLiteMessage::TRANS_ID_GET_FOCUS_SESSION_ELEMENT),
        data, reply, option) != ERR_NONE) {
        WLOGFE("SendRequest failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    sptr<AppExecFwk::ElementName> ret = reply.ReadParcelable<AppExecFwk::ElementName>();
    if (ret) {
        element = *ret;
    } else {
        WLOGFD("get element null.");
    }
    return static_cast<WSError>(reply.ReadInt32());
}

WSError SceneSessionManagerLiteProxy::GetSessionSnapshot(const std::string& deviceId, int32_t persistentId,
    SessionSnapshot& snapshot, bool isLowResolution)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        WLOGFE("WriteInterfaceToken failed");
        return WSError::WS_ERROR_INVALID_PARAM;
    }
    if (!data.WriteString16(Str8ToStr16(deviceId))) {
        WLOGFE("Write deviceId failed.");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    if (!data.WriteInt32(persistentId)) {
        WLOGFE("Write persistentId failed");
        return WSError::WS_ERROR_INVALID_PARAM;
    }

    if (!data.WriteBool(isLowResolution)) {
        WLOGFE("Write isLowResolution failed");
        return WSError::WS_ERROR_INVALID_PARAM;
    }

    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        WLOGFE("remote is null");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    if (remote->SendRequest(static_cast<uint32_t>(SceneSessionManagerLiteMessage::TRANS_ID_GET_SESSION_SNAPSHOT),
        data, reply, option) != ERR_NONE) {
        WLOGFE("SendRequest failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    std::unique_ptr<SessionSnapshot> info(reply.ReadParcelable<SessionSnapshot>());
    if (info) {
        snapshot = *info;
    } else {
        WLOGFW("Read SessionSnapshot is null.");
    }
    return static_cast<WSError>(reply.ReadInt32());
}

WSError SceneSessionManagerLiteProxy::SetSessionContinueState(const sptr<IRemoteObject>& token,
    const ContinueState& continueState)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        WLOGFE("WriteInterfaceToken failed");
        return WSError::WS_ERROR_INVALID_PARAM;
    }
    if (!data.WriteRemoteObject(token)) {
        WLOGFE("Write token failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    if (!data.WriteInt32(static_cast<int32_t>(continueState))) {
        WLOGFE("Write continueState failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        WLOGFE("remote is null");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    if (remote->SendRequest(static_cast<uint32_t>(
        SceneSessionManagerLiteMessage::TRANS_ID_SET_SESSION_CONTINUE_STATE),
        data, reply, option) != ERR_NONE) {
        WLOGFE("SendRequest failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    return static_cast<WSError>(reply.ReadInt32());
}

WSError SceneSessionManagerLiteProxy::LockSession(int32_t sessionId)
{
    WLOGFD("run SceneSessionManagerLiteProxy::LockSession");
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!data.WriteInterfaceToken(GetDescriptor())) {
        WLOGFE("Write interface token failed.");
        return WSError::WS_ERROR_INVALID_PARAM;
    }
    if (!data.WriteInt32(sessionId)) {
        WLOGFE("Write persistentId failed");
        return WSError::WS_ERROR_INVALID_PARAM;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        WLOGFE("remote is null");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    if (remote->SendRequest(static_cast<uint32_t>(SceneSessionManagerLiteMessage::TRANS_ID_LOCK_SESSION),
        data, reply, option) != ERR_NONE) {
        WLOGFE("SendRequest failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    return static_cast<WSError>(reply.ReadInt32());
}

WSError SceneSessionManagerLiteProxy::UnlockSession(int32_t sessionId)
{
    WLOGFD("run SceneSessionManagerLiteProxy::UnlockSession");
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!data.WriteInterfaceToken(GetDescriptor())) {
        WLOGFE("Write interface token failed.");
        return WSError::WS_ERROR_INVALID_PARAM;
    }
    if (!data.WriteInt32(sessionId)) {
        WLOGFE("Write persistentId failed");
        return WSError::WS_ERROR_INVALID_PARAM;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        WLOGFE("remote is null");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    if (remote->SendRequest(static_cast<uint32_t>(SceneSessionManagerLiteMessage::TRANS_ID_UNLOCK_SESSION),
        data, reply, option) != ERR_NONE) {
        WLOGFE("SendRequest failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    return static_cast<WSError>(reply.ReadInt32());
}

WSError SceneSessionManagerLiteProxy::MoveSessionsToForeground(const std::vector<std::int32_t>& sessionIds,
    int32_t topSessionId)
{
    WLOGFD("run SceneSessionManagerLiteProxy::MoveSessionsToForeground");
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        WLOGFE("WriteInterfaceToken failed");
        return WSError::WS_ERROR_INVALID_PARAM;
    }
    if (!data.WriteInt32Vector(sessionIds)) {
        WLOGFE("Write sessionIds failed");
        return WSError::WS_ERROR_INVALID_PARAM;
    }
    if (!data.WriteInt32(topSessionId)) {
        WLOGFE("Write topSessionId failed");
        return WSError::WS_ERROR_INVALID_PARAM;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        WLOGFE("remote is null");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    if (remote->SendRequest(static_cast<uint32_t>(
        SceneSessionManagerLiteMessage::TRANS_ID_MOVE_MISSIONS_TO_FOREGROUND),
        data, reply, option) != ERR_NONE) {
        WLOGFE("SendRequest failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    return static_cast<WSError>(reply.ReadInt32());
}

WSError SceneSessionManagerLiteProxy::MoveSessionsToBackground(const std::vector<std::int32_t>& sessionIds,
    std::vector<int32_t>& result)
{
    WLOGFD("run SceneSessionManagerLiteProxy::MoveSessionsToBackground");
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        WLOGFE("WriteInterfaceToken failed");
        return WSError::WS_ERROR_INVALID_PARAM;
    }
    if (!data.WriteInt32Vector(sessionIds)) {
        WLOGFE("Write sessionIds failed");
        return WSError::WS_ERROR_INVALID_PARAM;
    }
    if (!data.WriteInt32Vector(result)) {
        WLOGFE("Write result failed");
        return WSError::WS_ERROR_INVALID_PARAM;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        WLOGFE("remote is null");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    if (remote->SendRequest(static_cast<uint32_t>(
        SceneSessionManagerLiteMessage::TRANS_ID_MOVE_MISSIONS_TO_BACKGROUND),
        data, reply, option) != ERR_NONE) {
        WLOGFE("SendRequest failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    reply.ReadInt32Vector(&result);
    return static_cast<WSError>(reply.ReadInt32());
}

WSError SceneSessionManagerLiteProxy::ClearSession(int32_t persistentId)
{
    WLOGFD("run SceneSessionManagerLiteProxy::ClearSession");
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        WLOGFE("ClearSession WriteInterfaceToken failed");
        return WSError::WS_ERROR_INVALID_PARAM;
    }

    if (!data.WriteInt32(persistentId)) {
        WLOGFE("Write persistentId failed");
        return WSError::WS_ERROR_INVALID_PARAM;
    }

    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        WLOGFE("remote is null");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    if (remote->SendRequest(static_cast<uint32_t>(SceneSessionManagerLiteMessage::TRANS_ID_CLEAR_SESSION),
        data, reply, option) != ERR_NONE) {
        WLOGFE("SendRequest failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    return static_cast<WSError>(reply.ReadInt32());
}

WSError SceneSessionManagerLiteProxy::ClearAllSessions()
{
    WLOGFD("run SceneSessionManagerLiteProxy::ClearSession");
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        WLOGFE("ClearAllSessions WriteInterfaceToken failed");
        return WSError::WS_ERROR_INVALID_PARAM;
    }

    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        WLOGFE("remote is null");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    if (remote->SendRequest(static_cast<uint32_t>(SceneSessionManagerLiteMessage::TRANS_ID_CLEAR_ALL_SESSIONS),
        data, reply, option) != ERR_NONE) {
        WLOGFE("SendRequest failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    return static_cast<WSError>(reply.ReadInt32());
}

void SceneSessionManagerLiteProxy::GetFocusWindowInfo(FocusChangeInfo& focusInfo, DisplayId displayId)
{
    WLOGFD("get focus Winow info lite proxy");
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        WLOGFE("WriteInterfaceToken failed");
        return;
    }
    if (!data.WriteUint64(displayId)) {
        TLOGE(WmsLogTag::WMS_FOCUS, "write displayId failed");
        return;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        WLOGFE("remote is null");
        return;
    }
    if (remote->SendRequest(static_cast<uint32_t>(SceneSessionManagerLiteMessage::TRANS_ID_GET_FOCUS_SESSION_INFO),
        data, reply, option) != ERR_NONE) {
        WLOGFE("SendRequest failed");
        return;
    }
    sptr<FocusChangeInfo> info = reply.ReadParcelable<FocusChangeInfo>();
    if (info) {
        focusInfo = *info;
    } else {
        WLOGFE("info is null.");
    }
}

WMError SceneSessionManagerLiteProxy::RegisterWindowManagerAgent(WindowManagerAgentType type,
    const sptr<IWindowManagerAgent>& windowManagerAgent)
{
    MessageOption option;
    MessageParcel reply;
    MessageParcel data;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        WLOGFE("Write InterfaceToken failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }

    if (!data.WriteUint32(static_cast<uint32_t>(type))) {
        WLOGFE("Write type failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }

    if (!data.WriteRemoteObject(windowManagerAgent->AsObject())) {
        WLOGFE("Write IWindowManagerAgent failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }

    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        WLOGFE("remote is nullptr");
        return WMError::WM_ERROR_NULLPTR;
    }
    if (remote->SendRequest(static_cast<uint32_t>(
        SceneSessionManagerLiteMessage::TRANS_ID_REGISTER_WINDOW_MANAGER_AGENT), data, reply, option) != ERR_NONE) {
        WLOGFE("SendRequest failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }

    return static_cast<WMError>(reply.ReadInt32());
}

WMError SceneSessionManagerLiteProxy::UnregisterWindowManagerAgent(WindowManagerAgentType type,
    const sptr<IWindowManagerAgent>& windowManagerAgent)
{
    MessageParcel reply;
    MessageOption option;
    MessageParcel data;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        WLOGFE("Write InterfaceToken failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }

    if (!data.WriteUint32(static_cast<uint32_t>(type))) {
        WLOGFE("Write type failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }

    if (!data.WriteRemoteObject(windowManagerAgent->AsObject())) {
        WLOGFE("Write IWindowManagerAgent failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }

    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        WLOGFE("remote is nullptr");
        return WMError::WM_ERROR_NULLPTR;
    }
    if (remote->SendRequest(static_cast<uint32_t>(
        SceneSessionManagerLiteMessage::TRANS_ID_UNREGISTER_WINDOW_MANAGER_AGENT), data, reply, option) != ERR_NONE) {
        WLOGFE("SendRequest failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }

    return static_cast<WMError>(reply.ReadInt32());
}

WMError SceneSessionManagerLiteProxy::CheckWindowId(int32_t windowId, int32_t& pid)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        WLOGFE("Failed to write interfaceToken");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    if (!data.WriteInt32(windowId)) {
        WLOGFE("Failed to write windowId");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        WLOGFE("remote is nullptr");
        return WMError::WM_ERROR_NULLPTR;
    }
    int32_t ret = remote->SendRequest(static_cast<uint32_t>(SceneSessionManagerLiteMessage::TRANS_ID_CHECK_WINDOW_ID),
        data, reply, option);
    if (ret != ERR_NONE) {
        WLOGFE("Send request failed, ret:%{public}d", ret);
        return WMError::WM_ERROR_IPC_FAILED;
    }
    if (!reply.ReadInt32(pid)) {
        WLOGFE("Failed to read pid");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    return WMError::WM_OK;
}

WMError SceneSessionManagerLiteProxy::CheckUIExtensionCreation(int32_t windowId, uint32_t tokenId,
    const AppExecFwk::ElementName& element, AppExecFwk::ExtensionAbilityType extensionAbilityType, int32_t& pid)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::WMS_UIEXT, "UIExtOnLock: Failed to write interfaceToken");
        return WMError::WM_ERROR_IPC_FAILED;
    }

    if (!data.WriteInt32(windowId)) {
        TLOGE(WmsLogTag::WMS_UIEXT, "UIExtOnLock: Failed to write windowId");
        return WMError::WM_ERROR_IPC_FAILED;
    }

    if (!data.WriteUint32(tokenId)) {
        TLOGE(WmsLogTag::WMS_UIEXT, "UIExtOnLock: Failed to write tokenId");
        return WMError::WM_ERROR_IPC_FAILED;
    }

    if (!data.WriteInt32(static_cast<int32_t>(extensionAbilityType))) {
        TLOGE(WmsLogTag::WMS_UIEXT, "UIExtOnLock: Failed to write extensionAbilityType");
        return WMError::WM_ERROR_IPC_FAILED;
    }

    data.WriteParcelable(&element);

    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::WMS_UIEXT, "remote is null");
        return WMError::WM_ERROR_NULLPTR;
    }

    int32_t ret =
        remote->SendRequest(static_cast<uint32_t>(SceneSessionManagerLiteMessage::TRANS_ID_UI_EXTENSION_CREATION_CHECK),
            data, reply, option);
    if (ret != ERR_NONE) {
        TLOGE(WmsLogTag::WMS_UIEXT, "UIExtOnLock: Send request failed, ret:%{public}d", ret);
        return WMError::WM_ERROR_IPC_FAILED;
    }

    int32_t errCode = 0;
    if (!reply.ReadInt32(errCode)) {
        TLOGE(WmsLogTag::WMS_UIEXT, "UIExtOnLock: Failed to read errcode");
        return WMError::WM_ERROR_IPC_FAILED;
    }

    if (!reply.ReadInt32(pid)) {
        TLOGE(WmsLogTag::WMS_UIEXT, "UIExtOnLock: Failed to read pid");
        return WMError::WM_ERROR_IPC_FAILED;
    }

    TLOGD(WmsLogTag::WMS_UIEXT, "UIExtOnLock: errcode %{public}u", errCode);
    return static_cast<WMError>(errCode);
}

WMError SceneSessionManagerLiteProxy::GetVisibilityWindowInfo(std::vector<sptr<WindowVisibilityInfo>>& infos)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        WLOGFE("GetVisibilityWindowInfo Write interfaceToken failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }

    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        WLOGFE("remote is nullptr");
        return WMError::WM_ERROR_NULLPTR;
    }
    if (remote->SendRequest(static_cast<uint32_t>(
        SceneSessionManagerLiteMessage::TRANS_ID_GET_VISIBILITY_WINDOW_INFO_ID), data, reply, option) != ERR_NONE) {
        return WMError::WM_ERROR_IPC_FAILED;
    }
    if (!MarshallingHelper::UnmarshallingVectorParcelableObj<WindowVisibilityInfo>(reply, infos)) {
        WLOGFE("read visibility window infos failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    return static_cast<WMError>(reply.ReadInt32());
}

WMError SceneSessionManagerLiteProxy::GetWindowModeType(WindowModeType& windowModeType)
{
    WLOGFI("get Window mode type proxy");
    MessageParcel data;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        WLOGFE("WriteInterfaceToken failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    MessageParcel reply;
    MessageOption option;
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        WLOGFE("remote is null");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    if (remote->SendRequest(static_cast<uint32_t>(
        SceneSessionManagerLiteMessage::TRANS_ID_GET_WINDOW_MODE_TYPE), data, reply, option) != ERR_NONE) {
        WLOGFE("SendRequest failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }

    windowModeType = static_cast<WindowModeType>(reply.ReadUint32());
    return static_cast<WMError>(reply.ReadInt32());
}

WMError SceneSessionManagerLiteProxy::GetMainWindowInfos(int32_t topNum, std::vector<MainWindowInfo>& topNInfo)
{
    TLOGI(WmsLogTag::WMS_MAIN, "get main info in %{public}d", topNum);
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::WMS_MAIN, "WriteInterfaceToken failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }

    if ((topNum <= 0) || (topNum >= MAX_TOPN_INFO_SIZE)) {
        return WMError::WM_ERROR_INVALID_PARAM;
    }

    if (!data.WriteInt32(topNum)) {
        TLOGE(WmsLogTag::WMS_MAIN, "topNum write fail");
        return WMError::WM_ERROR_IPC_FAILED;
    }

    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::WMS_MAIN, "remote is null");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    if (remote->SendRequest(static_cast<int32_t>(SceneSessionManagerLiteMessage::TRANS_ID_GET_TOPN_MAIN_WINDOW_INFO),
                            data, reply, option) != ERR_NONE) {
        TLOGE(WmsLogTag::WMS_MAIN, "send request fail");
        return WMError::WM_ERROR_IPC_FAILED;
    }

    WMError error = static_cast<WMError>(GetParcelableInfos(reply, topNInfo));
    if (error != WMError::WM_OK) {
        TLOGE(WmsLogTag::WMS_MAIN, "get info error");
        return error;
    }

    return static_cast<WMError>(reply.ReadInt32());
}

WMError SceneSessionManagerLiteProxy::GetCallingWindowInfo(CallingWindowInfo& callingWindowInfo)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::WMS_KEYBOARD, "WriteInterfaceToken failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    if (!data.WriteParcelable(&callingWindowInfo)) {
        TLOGE(WmsLogTag::WMS_KEYBOARD, "Write callingWindowInfo failed, id: %{public}d, userId: %{public}d",
            callingWindowInfo.windowId_, callingWindowInfo.userId_);
        return WMError::WM_ERROR_IPC_FAILED;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::WMS_KEYBOARD, "remote is null");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    if (remote->SendRequest(static_cast<uint32_t>(SceneSessionManagerLiteMessage::TRANS_ID_GET_CALLING_WINDOW_INFO),
        data, reply, option) != ERR_NONE) {
        TLOGE(WmsLogTag::WMS_KEYBOARD, "SendRequest failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    auto ret = static_cast<WMError>(reply.ReadInt32());
    if (ret == WMError::WM_OK) {
        sptr<CallingWindowInfo> info = reply.ReadParcelable<CallingWindowInfo>();
        if (info == nullptr) {
            TLOGE(WmsLogTag::WMS_KEYBOARD, "Read callingWindowInfo failed");
            return WMError::WM_ERROR_IPC_FAILED;
        }
        callingWindowInfo = *info;
    }
    return ret;
}

WSError SceneSessionManagerLiteProxy::RegisterIAbilityManagerCollaborator(int32_t type,
    const sptr<AAFwk::IAbilityManagerCollaborator>& impl)
{
    TLOGI(WmsLogTag::WMS_MAIN, "type:%{public}d", type);
    if (!impl) {
        TLOGE(WmsLogTag::WMS_MAIN, "impl is nullptr");
        return WSError::WS_ERROR_INVALID_PARAM;
    }
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::WMS_MAIN, "WriteInterfaceToken failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    if (!data.WriteInt32(type)) {
        TLOGE(WmsLogTag::WMS_MAIN, "Write type failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    if (!data.WriteRemoteObject(impl->AsObject())) {
        TLOGE(WmsLogTag::WMS_MAIN, "Write impl failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }

    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::WMS_MAIN, "remote is null");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    if (remote->SendRequest(static_cast<uint32_t>(
        SceneSessionManagerLiteMessage::TRANS_ID_REGISTER_COLLABORATOR),
        data, reply, option) != ERR_NONE) {
        TLOGE(WmsLogTag::WMS_MAIN, "SendRequest failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    return static_cast<WSError>(reply.ReadInt32());
}

WSError SceneSessionManagerLiteProxy::UnregisterIAbilityManagerCollaborator(int32_t type)
{
    TLOGI(WmsLogTag::WMS_MAIN, "type:%{public}d", type);
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::WMS_MAIN, "WriteInterfaceToken failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    if (!data.WriteInt32(type)) {
        TLOGE(WmsLogTag::WMS_MAIN, "Write type failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }

    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::WMS_MAIN, "remote is null");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    if (remote->SendRequest(static_cast<uint32_t>(
        SceneSessionManagerLiteMessage::TRANS_ID_UNREGISTER_COLLABORATOR),
        data, reply, option) != ERR_NONE) {
        TLOGE(WmsLogTag::WMS_MAIN, "SendRequest failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    return static_cast<WSError>(reply.ReadInt32());
}

WMError SceneSessionManagerLiteProxy::GetAllMainWindowInfos(std::vector<MainWindowInfo>& infos)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::WMS_MAIN, "WriteInterfaceToken failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::WMS_MAIN, "remote is null");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    if (remote->SendRequest(static_cast<int32_t>(
        SceneSessionManagerLiteMessage::TRANS_ID_GET_ALL_MAIN_WINDOW_INFO), data, reply, option) != ERR_NONE) {
        TLOGE(WmsLogTag::WMS_MAIN, "send request fail");
        return WMError::WM_ERROR_IPC_FAILED;
    }

    WMError error = static_cast<WMError>(GetParcelableInfos(reply, infos));
    if (error != WMError::WM_OK) {
        TLOGE(WmsLogTag::WMS_MAIN, "get info error");
        return error;
    }

    return static_cast<WMError>(reply.ReadInt32());
}

WMError SceneSessionManagerLiteProxy::ClearMainSessions(const std::vector<int32_t>& persistentIds,
    std::vector<int32_t>& clearFailedIds)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::WMS_MAIN, "WriteInterfaceToken failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    if (!data.WriteInt32Vector(persistentIds)) {
        TLOGE(WmsLogTag::WMS_MAIN, "Write persistentIds failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::WMS_MAIN, "remote is null");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    if (remote->SendRequest(static_cast<int32_t>(
        SceneSessionManagerLiteMessage::TRANS_ID_CLEAR_MAIN_SESSIONS), data, reply, option) != ERR_NONE) {
        TLOGE(WmsLogTag::WMS_MAIN, "send request fail");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    reply.ReadInt32Vector(&clearFailedIds);
    return static_cast<WMError>(reply.ReadInt32());
}

WSError SceneSessionManagerLiteProxy::RaiseWindowToTop(int32_t persistentId)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::WMS_MAIN, "WriteInterfaceToken failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    if (!data.WriteInt32(persistentId)) {
        TLOGE(WmsLogTag::WMS_MAIN, "Write persistentId failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }

    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::WMS_MAIN, "remote is null");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    if (remote->SendRequest(static_cast<uint32_t>(
        SceneSessionManagerLiteMessage::TRANS_ID_RAISE_WINDOW_TO_TOP),
        data, reply, option) != ERR_NONE) {
        TLOGE(WmsLogTag::WMS_MAIN, "SendRequest failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    int32_t ret = reply.ReadInt32();
    return static_cast<WSError>(ret);
}

WMError SceneSessionManagerLiteProxy::GetWindowStyleType(WindowStyleType& windowStyleType)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::WMS_MAIN, "GetwindowStyleType Write interfaceToken failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::WMS_MAIN, "remote is null");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    if (remote->SendRequest(static_cast<uint32_t>(
        SceneSessionManagerLiteMessage::TRANS_ID_GET_WINDOW_STYLE_TYPE), data, reply, option) != ERR_NONE) {
        return WMError::WM_ERROR_IPC_FAILED;
    }
    windowStyleType = static_cast<WindowStyleType>(reply.ReadUint32());
    return static_cast<WMError>(reply.ReadInt32());
}

WMError SceneSessionManagerLiteProxy::TerminateSessionByPersistentId(int32_t persistentId)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::WMS_MAIN, "WriteInterfaceToken failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    if (!data.WriteInt32(persistentId)) {
        TLOGE(WmsLogTag::WMS_MAIN, "Write persistentId failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::WMS_MAIN, "remote is null");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    if (remote->SendRequest(static_cast<int32_t>(
        SceneSessionManagerLiteMessage::TRANS_ID_TERMINATE_SESSION_BY_PERSISTENT_ID),
        data, reply, option) != ERR_NONE) {
        TLOGE(WmsLogTag::WMS_MAIN, "send request fail");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    return static_cast<WMError>(reply.ReadInt32());
}

WMError SceneSessionManagerLiteProxy::CloseTargetFloatWindow(const std::string& bundleName)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::WMS_MULTI_WINDOW, "WriteInterfaceToken failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    if (!data.WriteString(bundleName)) {
        TLOGE(WmsLogTag::WMS_MULTI_WINDOW, "Write bundleName failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::WMS_MULTI_WINDOW, "remote is null");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    if (remote->SendRequest(static_cast<int32_t>(
        SceneSessionManagerLiteMessage::TRANS_ID_CLOSE_TARGET_FLOAT_WINDOW),
        data, reply, option) != ERR_NONE) {
        TLOGE(WmsLogTag::WMS_MULTI_WINDOW, "send request fail");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    return WMError::WM_OK;
}

WMError SceneSessionManagerLiteProxy::CloseTargetPiPWindow(const std::string& bundleName)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::WMS_PIP, "WriteInterfaceToken failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    if (!data.WriteString(bundleName)) {
        TLOGE(WmsLogTag::WMS_PIP, "Write bundleName failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::WMS_PIP, "remote is null");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    if (remote->SendRequest(static_cast<int32_t>(
        SceneSessionManagerLiteMessage::TRANS_ID_CLOSE_TARGET_PIP_WINDOW),
        data, reply, option) != ERR_NONE) {
        TLOGE(WmsLogTag::WMS_PIP, "send request fail");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    return static_cast<WMError>(reply.ReadInt32());
}

WMError SceneSessionManagerLiteProxy::GetCurrentPiPWindowInfo(std::string& bundleName)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::WMS_PIP, "WriteInterfaceToken failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::WMS_PIP, "remote is null");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    if (remote->SendRequest(static_cast<int32_t>(
        SceneSessionManagerLiteMessage::TRANS_ID_GET_CURRENT_PIP_WINDOW_INFO),
        data, reply, option) != ERR_NONE) {
        TLOGE(WmsLogTag::WMS_PIP, "send request fail");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    WMError errorCode = static_cast<WMError>(reply.ReadInt32());
    bundleName = reply.ReadString();
    return errorCode;
}

WMError SceneSessionManagerLiteProxy::GetRootMainWindowId(int32_t persistentId, int32_t& hostWindowId)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::WMS_MAIN, "WriteInterfaceToken failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    if (!data.WriteInt32(persistentId)) {
        TLOGE(WmsLogTag::WMS_MAIN, "Failed to write persistentId");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::WMS_MAIN, "remote is nullptr");
        return WMError::WM_ERROR_NULLPTR;
    }
    int32_t ret = remote->SendRequest(static_cast<uint32_t>(
        SceneSessionManagerLiteMessage::TRANS_ID_GET_ROOT_MAIN_WINDOW_ID), data, reply, option);
    if (ret != ERR_NONE) {
        TLOGE(WmsLogTag::WMS_MAIN, "Send request failed, ret:%{public}d", ret);
        return WMError::WM_ERROR_IPC_FAILED;
    }
    if (!reply.ReadInt32(hostWindowId)) {
        TLOGE(WmsLogTag::WMS_MAIN, "Failed to read hostWindowId");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    return WMError::WM_OK;
}

WMError SceneSessionManagerLiteProxy::GetAccessibilityWindowInfo(std::vector<sptr<AccessibilityWindowInfo>>& infos)
{
    MessageOption option;
    MessageParcel reply;
    MessageParcel data;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        WLOGFE("Write InterfaceToken failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }

    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        WLOGFE("remote is null");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    if (remote->SendRequest(static_cast<uint32_t>(
        SceneSessionManagerLiteMessage::TRANS_ID_GET_WINDOW_INFO),
        data, reply, option) != ERR_NONE) {
        WLOGFE("SendRequest failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }

    if (!MarshallingHelper::UnmarshallingVectorParcelableObj<AccessibilityWindowInfo>(reply, infos)) {
        WLOGFE("read window info failed.");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    return static_cast<WMError>(reply.ReadInt32());
}

WSError SceneSessionManagerLiteProxy::NotifyAppUseControlList(
    ControlAppType type, int32_t userId, const std::vector<AppUseControlInfo>& controlList)
{
    TLOGD(WmsLogTag::WMS_LIFE, "in");
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::WMS_LIFE, "WriteInterfaceToken failed");
        return WSError::WS_ERROR_INVALID_PARAM;
    }

    if (!data.WriteUint8(static_cast<uint8_t>(type))) {
        TLOGE(WmsLogTag::WMS_LIFE, "Write type failed");
        return WSError::WS_ERROR_INVALID_PARAM;
    }

    if (!data.WriteInt32(userId)) {
        TLOGE(WmsLogTag::WMS_LIFE, "Write userId failed");
        return WSError::WS_ERROR_INVALID_PARAM;
    }

    if (!data.WriteInt32(static_cast<int32_t>(controlList.size()))) {
        TLOGE(WmsLogTag::WMS_LIFE, "Write controlList size failed");
        return WSError::WS_ERROR_INVALID_PARAM;
    }

    for (const auto& control : controlList) {
        if (!data.WriteString(control.bundleName_) || !data.WriteInt32(control.appIndex_) ||
            !data.WriteBool(control.isNeedControl_) || !data.WriteBool(control.isControlRecentOnly_)) {
            TLOGE(WmsLogTag::WMS_LIFE, "Write controlList failed");
            return WSError::WS_ERROR_INVALID_PARAM;
        }
    }

    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::WMS_LIFE, "remote is null");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    if (remote->SendRequest(static_cast<uint32_t>(
        SceneSessionManagerLiteMessage::TRANS_ID_NOTIFY_APP_USE_CONTROL_LIST),
        data, reply, option) != ERR_NONE) {
        TLOGE(WmsLogTag::WMS_LIFE, "SendRequest failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    return static_cast<WSError>(reply.ReadInt32());
}

WMError SceneSessionManagerLiteProxy::MinimizeMainSession(
    const std::string& bundleName, int32_t appIndex, int32_t userId)
{
    TLOGD(WmsLogTag::WMS_LIFE, "in");
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::WMS_LIFE, "Write interfaceToken failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    if (!data.WriteString(bundleName)) {
        TLOGE(WmsLogTag::WMS_LIFE, "write bundleName failed.");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    if (!data.WriteInt32(appIndex)) {
        TLOGE(WmsLogTag::WMS_LIFE, "write appIndex failed.");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    if (!data.WriteInt32(userId)) {
        TLOGE(WmsLogTag::WMS_LIFE, "write userId failed.");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::WMS_LIFE, "remote is null");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    if (remote->SendRequest(static_cast<uint32_t>(SceneSessionManagerLiteMessage::TRANS_ID_MINIMIZE_MAIN_SESSION),
        data, reply, option) != ERR_NONE) {
        TLOGE(WmsLogTag::WMS_LIFE, "SendRequest failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    int32_t ret = 0;
    if (!reply.ReadInt32(ret)) {
        TLOGE(WmsLogTag::WMS_LIFE, "Read ret failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    return static_cast<WMError>(ret);
}

WMError SceneSessionManagerLiteProxy::LockSessionByAbilityInfo(const AbilityInfoBase& abilityInfo, bool isLock)
{
    TLOGD(WmsLogTag::WMS_LIFE, "in");
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::WMS_LIFE, "Write interfaceToken failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    if (!data.WriteString(abilityInfo.bundleName)) {
        TLOGE(WmsLogTag::WMS_LIFE, "write bundleName failed.");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    if (!data.WriteString(abilityInfo.moduleName)) {
        TLOGE(WmsLogTag::WMS_LIFE, "write moduleName failed.");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    if (!data.WriteString(abilityInfo.abilityName)) {
        TLOGE(WmsLogTag::WMS_LIFE, "write abilityName failed.");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    if (!data.WriteInt32(abilityInfo.appIndex)) {
        TLOGE(WmsLogTag::WMS_LIFE, "write appIndex failed.");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    if (!data.WriteBool(isLock)) {
        TLOGE(WmsLogTag::WMS_LIFE, "write isLock failed.");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::WMS_LIFE, "remote is null");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    if (remote->SendRequest(
        static_cast<uint32_t>(SceneSessionManagerLiteMessage::TRANS_ID_LOCK_SESSION_BY_ABILITY_INFO),
        data, reply, option) != ERR_NONE) {
        TLOGE(WmsLogTag::WMS_LIFE, "SendRequest failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    int32_t ret = 0;
    if (!reply.ReadInt32(ret)) {
        TLOGE(WmsLogTag::WMS_LIFE, "Read ret failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    return static_cast<WMError>(ret);
}

WMError SceneSessionManagerLiteProxy::HasFloatingWindowForeground(const sptr<IRemoteObject>& abilityToken,
    bool& hasOrNot)
{
    if (!abilityToken) {
        TLOGE(WmsLogTag::WMS_SYSTEM, "AbilityToken is null");
        return WMError::WM_ERROR_INVALID_PARAM;
    }
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::WMS_SYSTEM, "Write interfaceToken failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    if (!data.WriteRemoteObject(abilityToken)) {
        TLOGE(WmsLogTag::WMS_SYSTEM, "Write abilityToken failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::WMS_SYSTEM, "Remote is null");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    if (remote->SendRequest(static_cast<uint32_t>(SceneSessionManagerLiteMessage::TRANS_ID_HAS_FLOAT_FOREGROUND),
        data, reply, option) != ERR_NONE) {
        TLOGE(WmsLogTag::WMS_SYSTEM, "SendRequest failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    if (!reply.ReadBool(hasOrNot)) {
        TLOGE(WmsLogTag::WMS_SYSTEM, "Read result failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    uint32_t ret = 0;
    if (!reply.ReadUint32(ret)) {
        TLOGE(WmsLogTag::WMS_SYSTEM, "Read ret failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    return static_cast<WMError>(ret);
}

WMError SceneSessionManagerLiteProxy::RegisterSessionLifecycleListenerByIds(
    const sptr<ISessionLifecycleListener>& listener, const std::vector<int32_t>& persistentIdList)
{
    TLOGD(WmsLogTag::WMS_LIFE, "in");
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::WMS_LIFE, "Write interfaceToken failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    if (listener == nullptr || listener->AsObject() == nullptr) {
        TLOGE(WmsLogTag::WMS_LIFE, "listener is null");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    if (!data.WriteRemoteObject(listener->AsObject())) {
        TLOGE(WmsLogTag::WMS_LIFE, "Write lifecycle listener failed.");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    if (!data.WriteInt32Vector(persistentIdList)) {
        TLOGE(WmsLogTag::WMS_LIFE, "Write persistentIdList failed.");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::WMS_LIFE, "remote is null");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    if (remote->SendRequest(
        static_cast<uint32_t>(SceneSessionManagerLiteMessage::TRANS_ID_REGISTER_SESSION_LIFECYCLE_LISTENER_BY_IDS),
        data, reply, option) != ERR_NONE) {
        TLOGE(WmsLogTag::WMS_LIFE, "SendRequest failed.");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    int32_t ret = 0;
    if (!reply.ReadInt32(ret)) {
        TLOGE(WmsLogTag::WMS_LIFE, "Read ret failed.");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    return static_cast<WMError>(ret);
}

WMError SceneSessionManagerLiteProxy::RegisterSessionLifecycleListenerByBundles(
    const sptr<ISessionLifecycleListener>& listener, const std::vector<std::string>& bundleNameList)
{
    TLOGD(WmsLogTag::WMS_LIFE, "in");
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::WMS_LIFE, "Write interfaceToken failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    if (listener == nullptr || listener->AsObject() == nullptr) {
        TLOGE(WmsLogTag::WMS_LIFE, "listener is null");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    if (!data.WriteRemoteObject(listener->AsObject())) {
        TLOGE(WmsLogTag::WMS_LIFE, "Write lifecycle listener failed.");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    if (!data.WriteStringVector(bundleNameList)) {
        TLOGE(WmsLogTag::WMS_LIFE, "Write bundleNameList failed.");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::WMS_LIFE, "remote is null");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    if (remote->SendRequest(
        static_cast<uint32_t>(SceneSessionManagerLiteMessage::TRANS_ID_REGISTER_SESSION_LIFECYCLE_LISTENER_BY_BUNDLES),
        data, reply, option) != ERR_NONE) {
        TLOGE(WmsLogTag::WMS_LIFE, "SendRequest failed.");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    int32_t ret = 0;
    if (!reply.ReadInt32(ret)) {
        TLOGE(WmsLogTag::WMS_LIFE, "Read ret failed.");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    return static_cast<WMError>(ret);
}

WMError SceneSessionManagerLiteProxy::UnregisterSessionLifecycleListener(
    const sptr<ISessionLifecycleListener>& listener)
{
    TLOGD(WmsLogTag::WMS_LIFE, "in");
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::WMS_LIFE, "Write interfaceToken failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    if (listener == nullptr || listener->AsObject() == nullptr) {
        TLOGE(WmsLogTag::WMS_LIFE, "listener is null");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    if (!data.WriteRemoteObject(listener->AsObject())) {
        TLOGE(WmsLogTag::WMS_LIFE, "Write lifecycle listener failed.");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::WMS_LIFE, "remote is null");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    if (remote->SendRequest(
        static_cast<uint32_t>(SceneSessionManagerLiteMessage::TRANS_ID_UNREGISTER_SESSION_LIFECYCLE_LISTENER),
        data, reply, option) != ERR_NONE) {
        TLOGE(WmsLogTag::WMS_LIFE, "SendRequest failed.");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    int32_t ret = 0;
    if (!reply.ReadInt32(ret)) {
        TLOGE(WmsLogTag::WMS_LIFE, "Read ret failed.");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    return static_cast<WMError>(ret);
}

WMError SceneSessionManagerLiteProxy::ListWindowInfo(const WindowInfoOption& windowInfoOption,
    std::vector<sptr<WindowInfo>>& infos)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "Write interfaceToken failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    if (!data.WriteUint8(static_cast<WindowInfoFilterOptionDataType>(windowInfoOption.windowInfoFilterOption))) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "write windowInfoFilterOption failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    if (!data.WriteUint8(static_cast<WindowInfoTypeOptionDataType>(windowInfoOption.windowInfoTypeOption))) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "write windowInfoTypeOption failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    if (!data.WriteUint64(windowInfoOption.displayId)) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "write displayId failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    if (!data.WriteInt32(windowInfoOption.windowId)) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "write windowId failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "remote is null");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    if (remote->SendRequest(static_cast<uint32_t>(
        SceneSessionManagerLiteMessage::TRANS_ID_LIST_WINDOW_INFO), data, reply, option) != ERR_NONE) {
        return WMError::WM_ERROR_IPC_FAILED;
    }
    if (!MarshallingHelper::UnmarshallingVectorParcelableObj<WindowInfo>(reply, infos)) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "read window info failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    int32_t errCode = 0;
    if (!reply.ReadInt32(errCode)) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "read errcode failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    return static_cast<WMError>(errCode);
}
} // namespace OHOS::Rosen
