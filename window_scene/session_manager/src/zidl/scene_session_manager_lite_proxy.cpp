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
        WLOGFE("remote is null");
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

WSError SceneSessionManagerLiteProxy::RegisterSessionListener(const sptr<ISessionListener>& listener)
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
    if (infoSize > CYCLE_LIMIT) {
        WLOGFE("infoSize is too large");
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

WSError SceneSessionManagerLiteProxy::GetFocusSessionToken(sptr<IRemoteObject>& token)
{
    WLOGFD("run SceneSessionManagerLiteProxy::GetFocusSessionToken");
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        WLOGFE("Write interfaceToken failed");
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

WSError SceneSessionManagerLiteProxy::GetFocusSessionElement(AppExecFwk::ElementName& element)
{
    WLOGFD("run SceneSessionManagerLiteProxy::GetFocusSessionElement");
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        WLOGFE("Write interfaceToken failed");
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

void SceneSessionManagerLiteProxy::GetFocusWindowInfo(FocusChangeInfo& focusInfo)
{
    WLOGFI("get focus Winow info lite proxy");
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        WLOGFE("WriteInterfaceToken failed");
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
    if (Remote()->SendRequest(static_cast<int32_t>(
        SceneSessionManagerLiteMessage::TRANS_ID_TERMINATE_SESSION_BY_PERSISTENT_ID),
        data, reply, option) != ERR_NONE) {
        TLOGE(WmsLogTag::WMS_MAIN, "send request fail");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    return static_cast<WMError>(reply.ReadInt32());
}
} // namespace OHOS::Rosen
