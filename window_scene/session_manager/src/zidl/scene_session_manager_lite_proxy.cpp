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
#include "pointer_event.h"
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

WSError SceneSessionManagerLiteProxy::SetSessionIconForThirdParty(const sptr<IRemoteObject>& token,
    const std::shared_ptr<Media::PixelMap>& icon)
{
    TLOGD(WmsLogTag::WMS_MAIN, "in");
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::WMS_MAIN, "WriteInterfaceToken failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    if (!data.WriteRemoteObject(token)) {
        TLOGE(WmsLogTag::WMS_MAIN, "Write token failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    if (icon == nullptr) {
        TLOGE(WmsLogTag::WMS_MAIN, "Icon is null");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    if (!data.WriteParcelable(icon.get())) {
        TLOGE(WmsLogTag::WMS_MAIN, "Write icon failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::WMS_MAIN, "Remote is null");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    if (remote->SendRequest(
        static_cast<uint32_t>(SceneSessionManagerLiteMessage::TRANS_ID_SET_SESSION_ICON_FOR_THIRD_PARTY),
        data, reply, option) != ERR_NONE) {
        TLOGE(WmsLogTag::WMS_MAIN, "SendRequest failed");
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

WSError SceneSessionManagerLiteProxy::PendingSessionToForeground(const sptr<IRemoteObject>& token, int32_t windowMode)
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

    if (!data.WriteInt32(windowMode)) {
        TLOGE(WmsLogTag::WMS_LIFE, "Write windowMode failed");
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

WSError SceneSessionManagerLiteProxy::PendingSessionToBackground(const sptr<IRemoteObject>& token,
    const BackgroundParams& params)
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
    if (!data.WriteInt32(params.persistentId)) {
        TLOGE(WmsLogTag::WMS_LIFE, "Write persistentId failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    if (!data.WriteBool(params.shouldBackToCaller)) {
        TLOGE(WmsLogTag::WMS_LIFE, "Write shouldBackToCaller failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    if (!data.WriteParcelable(&params.wantParams)) {
        TLOGE(WmsLogTag::WMS_LIFE, "Write wantParams failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::WMS_LIFE, "remote is null");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    if (remote->SendRequest(static_cast<uint32_t>(
        SceneSessionManagerLiteMessage::TRANS_ID_PENDING_SESSION_TO_BACKGROUND),
        data, reply, option) != ERR_NONE) {
        TLOGE(WmsLogTag::WMS_LIFE, "SendRequest failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    int32_t ret = 0;
    if (!reply.ReadInt32(ret)) {
        TLOGE(WmsLogTag::WMS_LIFE, "Read ret failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    return static_cast<WSError>(ret);
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

WSError SceneSessionManagerLiteProxy::IsFocusWindowParent(const sptr<IRemoteObject>& token, bool& isParent)
{
    TLOGD(WmsLogTag::WMS_FOCUS, "run");
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (token == nullptr) {
        TLOGE(WmsLogTag::WMS_FOCUS, "Token is nullptr");
        return WSError::WS_ERROR_INVALID_PARAM;
    }
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::WMS_FOCUS, "Write interfaceToken failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    if (!data.WriteRemoteObject(token)) {
        TLOGE(WmsLogTag::WMS_FOCUS, "Write token failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::WMS_FOCUS, "Remote is null");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    if (remote->SendRequest(static_cast<uint32_t>(SceneSessionManagerLiteMessage::TRANS_ID_IS_FOCUS_WINDOW_PARENT),
        data, reply, option) != ERR_NONE) {
        TLOGE(WmsLogTag::WMS_FOCUS, "SendRequest failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    bool value = false;
    if (!reply.ReadBool(value)) {
        TLOGE(WmsLogTag::WMS_FOCUS, "Read result failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    int32_t ret = 0;
    if (!reply.ReadInt32(ret)) {
        TLOGE(WmsLogTag::WMS_FOCUS, "Read ret failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    isParent = value;
    TLOGD(WmsLogTag::WMS_FOCUS, "isParent: %{public}d", isParent);
    return static_cast<WSError>(ret);
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

WMError SceneSessionManagerLiteProxy::GetParentMainWindowId(int32_t windowId, int32_t& mainWindowId)
{
    TLOGI(WmsLogTag::WMS_LIFE, "in");
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::WMS_LIFE, "write interfaceToken failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    if (!data.WriteInt32(windowId)) {
        TLOGE(WmsLogTag::WMS_LIFE, "write windowId failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::WMS_LIFE, "remote is nullptr");
        return WMError::WM_ERROR_NULLPTR;
    }
    int32_t ret = remote->SendRequest(
        static_cast<uint32_t>(SceneSessionManagerLiteMessage::TRANS_ID_GET_PARENT_WINDOW_ID),
        data, reply, option);
    if (ret != ERR_NONE) {
        TLOGE(WmsLogTag::WMS_LIFE, "Send request failed, ret:%{public}d", ret);
        return WMError::WM_ERROR_IPC_FAILED;
    }
    if (!reply.ReadInt32(mainWindowId)) {
        TLOGE(WmsLogTag::WMS_LIFE, "Read mainWindowId failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    return static_cast<WMError>(reply.ReadInt32());
}

WMError SceneSessionManagerLiteProxy::UpdateWindowModeByIdForUITest(int32_t windowId, int32_t updateMode)
{
    TLOGD(WmsLogTag::WMS_LAYOUT, "in");
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "write interfaceToken failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    if (!data.WriteInt32(windowId)) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "write windowId failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    if (!data.WriteInt32(updateMode)) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "write updateMode failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "remote is nullptr");
        return WMError::WM_ERROR_NULLPTR;
    }
    int32_t ret = remote->SendRequest(
        static_cast<uint32_t>(SceneSessionManagerLiteMessage::TRANS_ID_UPDATE_WINDOW_MODE_BY_ID_FOR_UI_TEST),
        data, reply, option);
    if (ret != ERR_NONE) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "Send request failed, ret:%{public}d", ret);
        return WMError::WM_ERROR_IPC_FAILED;
    }
    return WMError::WM_OK;
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

void SceneSessionManagerLiteProxy::GetAllGroupInfo(std::unordered_map<DisplayId, DisplayGroupId>& displayId2GroupIdMap,
                                                   std::vector<sptr<FocusChangeInfo>>& allFocusInfoList)
{
    TLOGD(WmsLogTag::WMS_FOCUS, "request on lite proxy");
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::WMS_FOCUS, "WriteInterfaceToken failed");
        return;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::WMS_FOCUS, "remote is null");
        return;
    }
    if (remote->SendRequest(static_cast<uint32_t>(SceneSessionManagerLiteMessage::TRANS_ID_GET_ALL_GROUP_INFO),
        data, reply, option) != ERR_NONE) {
        TLOGE(WmsLogTag::WMS_FOCUS, "SendRequest failed");
        return;
    }
    if (!MarshallingHelper::UnmarshallingVectorParcelableObj<FocusChangeInfo>(reply, allFocusInfoList)) {
        TLOGE(WmsLogTag::WMS_FOCUS, "read window info failed");
        return;
    }
    int32_t mapSize;
    if (!reply.ReadInt32(mapSize)) {
        TLOGE(WmsLogTag::WMS_FOCUS, "Failed to read mapSize");
        return;
    }
    for (int32_t i = 0; i < mapSize; i++) {
        DisplayId displayId = DISPLAY_ID_INVALID;
        if (!reply.ReadUint64(displayId)) {
            TLOGE(WmsLogTag::WMS_FOCUS, "Failed to read displayId");
            return;
        }
        DisplayGroupId displayGroupId = DISPLAY_GROUP_ID_INVALID;
        if (!reply.ReadUint64(displayGroupId)) {
            TLOGE(WmsLogTag::WMS_FOCUS, "Failed to read displayGroupId");
            return;
        }
        displayId2GroupIdMap[displayId] = displayGroupId;
    }
    if (displayId2GroupIdMap.empty() || allFocusInfoList.empty()) {
        TLOGE(WmsLogTag::WMS_FOCUS, "ipc reply is empty");
        return;
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

    if (windowManagerAgent == nullptr) {
        TLOGE(WmsLogTag::DEFAULT, "windowManagerAgent is null");
        return WMError::WM_ERROR_NULLPTR;
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

    if (windowManagerAgent == nullptr) {
        TLOGE(WmsLogTag::DEFAULT, "windowManagerAgent is null");
        return WMError::WM_ERROR_NULLPTR;
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

WMError SceneSessionManagerLiteProxy::UpdateScreenLockStatusForApp(const std::string& bundleName, bool isRelease)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "WriteInterfaceToken failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    if (!data.WriteString(bundleName)) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "Write bundleName failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    if (!data.WriteBool(isRelease)) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "Write isRelease failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "remote is null");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    if (remote->SendRequest(static_cast<uint32_t>(SceneSessionManagerLiteMessage::TRANS_ID_UPDATE_SESSION_SCREEN_LOCK),
        data, reply, option) != ERR_NONE) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "SendRequest failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    int32_t errCode = 0;
    if (!reply.ReadInt32(errCode)) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "read errcode failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    return static_cast<WMError>(errCode);
}

WMError SceneSessionManagerLiteProxy::GetWindowModeType(WindowModeType& windowModeType)
{
    TLOGD(WmsLogTag::DEFAULT, "get Window mode type proxy");
    MessageParcel data;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::DEFAULT, "WriteInterfaceToken failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    MessageParcel reply;
    MessageOption option;
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::DEFAULT, "remote is null");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    if (remote->SendRequest(static_cast<uint32_t>(
        SceneSessionManagerLiteMessage::TRANS_ID_GET_WINDOW_MODE_TYPE), data, reply, option) != ERR_NONE) {
        TLOGE(WmsLogTag::DEFAULT, "SendRequest failed");
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

WMError SceneSessionManagerLiteProxy::UpdateAnimationSpeedWithPid(pid_t pid, float speed)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::WMS_ANIMATION, "Write interfaceToken failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    if (!data.WriteInt32(pid)) {
        TLOGE(WmsLogTag::WMS_ANIMATION, "Write pid failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    if (!data.WriteFloat(speed)) {
        TLOGE(WmsLogTag::WMS_ANIMATION, "Write speed failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::WMS_ANIMATION, "Remote is null");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    if (remote->SendRequest(static_cast<uint32_t>(
        SceneSessionManagerLiteMessage::TRANS_ID_UPDATE_ANIMATION_SPEED_WITH_PID),
        data, reply, option) != ERR_NONE) {
        TLOGE(WmsLogTag::WMS_ANIMATION, "SendRequest failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    int32_t ret = 0;
    if (!reply.ReadInt32(ret)) {
        TLOGE(WmsLogTag::WMS_ANIMATION, "Read ret failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    return static_cast<WMError>(ret);
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
    int sendCode = remote->SendRequest(
        static_cast<uint32_t>(SceneSessionManagerLiteMessage::TRANS_ID_GET_CALLING_WINDOW_INFO),
        data, reply, option);
    if (sendCode != ERR_NONE) {
        TLOGE(WmsLogTag::WMS_KEYBOARD, "SendRequest failed, code: %{public}d", sendCode);
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

WMError SceneSessionManagerLiteProxy::GetMainWindowInfoByToken(const sptr<IRemoteObject>& abilityToken,
    MainWindowInfo& windowInfo)
{
    if (!abilityToken) {
        TLOGE(WmsLogTag::WMS_MAIN, "AbilityToken is null");
        return WMError::WM_ERROR_INVALID_PARAM;
    }
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::WMS_MAIN, "Write interfaceToken failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    if (!data.WriteRemoteObject(abilityToken)) {
        TLOGE(WmsLogTag::WMS_MAIN, "Write abilityToken failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }

    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::WMS_MAIN, "Remote is null");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    if (remote->SendRequest(static_cast<uint32_t>(
        SceneSessionManagerLiteMessage::TRANS_ID_GET_MAIN_WINDOW_INFO_BY_TOKEN), data, reply, option) != ERR_NONE) {
        TLOGE(WmsLogTag::WMS_MAIN, "SendRequest failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    sptr<MainWindowInfo> info = reply.ReadParcelable<MainWindowInfo>();
    if (info) {
        windowInfo = *info;
    } else {
        TLOGE(WmsLogTag::WMS_MAIN, "get info error");
        return WMError::WM_ERROR_IPC_FAILED;
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
        if (!data.WriteParcelable(&control)) {
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
    MessageOption option(MessageOption::TF_SYNC);
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
    MessageOption option(MessageOption::TF_SYNC);
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
    MessageOption option(MessageOption::TF_SYNC);
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

WMError SceneSessionManagerLiteProxy::SetGlobalDragResizeType(DragResizeType dragResizeType)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "Write interfaceToken failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    if (!data.WriteUint32(static_cast<uint32_t>(dragResizeType))) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "Write dragResizeType failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "remote is null");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    if (remote->SendRequest(static_cast<uint32_t>(SceneSessionManagerLiteMessage::TRANS_ID_SET_GLOBAL_DRAG_RESIZE_TYPE),
        data, reply, option) != ERR_NONE) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "SendRequest failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    int32_t ret = 0;
    if (!reply.ReadInt32(ret)) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "Read ret failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    return static_cast<WMError>(ret);
}

WMError SceneSessionManagerLiteProxy::GetGlobalDragResizeType(DragResizeType& dragResizeType)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "Write interfaceToken failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "remote is null");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    if (remote->SendRequest(static_cast<uint32_t>(SceneSessionManagerLiteMessage::TRANS_ID_GET_GLOBAL_DRAG_RESIZE_TYPE),
        data, reply, option) != ERR_NONE) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "SendRequest failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    uint32_t obtainedDragResizeType = 0;
    if (!reply.ReadUint32(obtainedDragResizeType)) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "Read dragResizeType failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    if (obtainedDragResizeType >= static_cast<uint32_t>(DragResizeType::RESIZE_MAX_VALUE)) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "bad dragResizeType value");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    dragResizeType = static_cast<DragResizeType>(obtainedDragResizeType);
    int32_t ret = 0;
    if (!reply.ReadInt32(ret)) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "Read ret failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    return static_cast<WMError>(ret);
}

WMError SceneSessionManagerLiteProxy::SetAppDragResizeType(const std::string& bundleName, DragResizeType dragResizeType)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "Write interfaceToken failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    if (!data.WriteString(bundleName)) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "Write bundleName failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    if (!data.WriteUint32(static_cast<uint32_t>(dragResizeType))) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "Write dragResizeType failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "remote is null");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    if (remote->SendRequest(static_cast<uint32_t>(SceneSessionManagerLiteMessage::TRANS_ID_SET_APP_DRAG_RESIZE_TYPE),
        data, reply, option) != ERR_NONE) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "SendRequest failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    int32_t ret = 0;
    if (!reply.ReadInt32(ret)) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "Read ret failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    return static_cast<WMError>(ret);
}

WMError SceneSessionManagerLiteProxy::GetAppDragResizeType(const std::string& bundleName,
    DragResizeType& dragResizeType)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "Write interfaceToken failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    if (!data.WriteString(bundleName)) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "Write bundleName failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "remote is null");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    if (remote->SendRequest(static_cast<uint32_t>(SceneSessionManagerLiteMessage::TRANS_ID_GET_APP_DRAG_RESIZE_TYPE),
        data, reply, option) != ERR_NONE) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "SendRequest failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    uint32_t obtainedDragResizeType = 0;
    if (!reply.ReadUint32(obtainedDragResizeType)) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "Read dragResizeType failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    if (obtainedDragResizeType >= static_cast<uint32_t>(DragResizeType::RESIZE_MAX_VALUE)) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "bad dragResizeType value");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    dragResizeType = static_cast<DragResizeType>(obtainedDragResizeType);
    int32_t ret = 0;
    if (!reply.ReadInt32(ret)) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "Read ret failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    return static_cast<WMError>(ret);
}

WMError SceneSessionManagerLiteProxy::SetAppKeyFramePolicy(
    const std::string& bundleName, const KeyFramePolicy& keyFramePolicy)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "Write interfaceToken failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    if (!data.WriteString(bundleName)) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "Write bundleName failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    if (!data.WriteParcelable(&keyFramePolicy)) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "Write keyFramePolicy failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "remote is null");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    if (remote->SendRequest(static_cast<uint32_t>(SceneSessionManagerLiteMessage::TRANS_ID_SET_APP_KEY_FRAME_POLICY),
        data, reply, option) != ERR_NONE) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "SendRequest failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    int32_t ret = 0;
    if (!reply.ReadInt32(ret)) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "Read ret failed");
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

WMError SceneSessionManagerLiteProxy::RegisterWindowPropertyChangeAgent(WindowInfoKey windowInfoKey,
    uint32_t interestInfo, const sptr<IWindowManagerAgent>& windowManagerAgent)
{
    MessageOption option;
    MessageParcel reply;
    MessageParcel data;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "Write InterfaceToken failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }

    if (!data.WriteInt32(static_cast<int32_t>(windowInfoKey))) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "Write windowInfoKey failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }

    if (!data.WriteUint32(interestInfo)) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "Write interestInfo failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }

    if (!windowManagerAgent) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "windowManagerAgent is nullptr");
        return WMError::WM_ERROR_IPC_FAILED;
    }

    if (!data.WriteRemoteObject(windowManagerAgent->AsObject())) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "Write IWindowManagerAgent failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }

    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "remote is null");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    if (remote->SendRequest(static_cast<uint32_t>(
        SceneSessionManagerLiteMessage::TRANS_ID_REGISTER_WINDOW_PROPERTY_CHANGE_AGENT),
        data, reply, option) != ERR_NONE) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "SendRequest failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }

    return static_cast<WMError>(reply.ReadInt32());
}

WMError SceneSessionManagerLiteProxy::UnregisterWindowPropertyChangeAgent(WindowInfoKey windowInfoKey,
    uint32_t interestInfo, const sptr<IWindowManagerAgent>& windowManagerAgent)
{
    MessageOption option;
    MessageParcel reply;
    MessageParcel data;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "Write InterfaceToken failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }

    if (!data.WriteInt32(static_cast<int32_t>(windowInfoKey))) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "Write windowInfoKey failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }

    if (!data.WriteUint32(interestInfo)) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "Write interestInfo failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }

    if (!windowManagerAgent) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "windowManagerAgent is nullptr");
        return WMError::WM_ERROR_IPC_FAILED;
    }

    if (!data.WriteRemoteObject(windowManagerAgent->AsObject())) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "Write IWindowManagerAgent failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }

    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "remote is null");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    if (remote->SendRequest(static_cast<uint32_t>(
        SceneSessionManagerLiteMessage::TRANS_ID_UNREGISTER_WINDOW_PROPERTY_CHANGE_AGENT),
        data, reply, option) != ERR_NONE) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "SendRequest failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }

    return static_cast<WMError>(reply.ReadInt32());
}

WMError SceneSessionManagerLiteProxy::RecoverWindowPropertyChangeFlag(uint32_t observedFlags, uint32_t interestedFlags)
{
    MessageOption option;
    MessageParcel reply;
    MessageParcel data;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "Write InterfaceToken failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }

    if (!data.WriteUint32(observedFlags)) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "Write observedFlags failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }

    if (!data.WriteUint32(interestedFlags)) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "Write interestedFlags failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }

    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "remote is null");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    if (remote->SendRequest(static_cast<uint32_t>(
        SceneSessionManagerLiteMessage::TRANS_ID_RECOVER_WINDOW_PROPERTY_CHANGE_FLAG),
        data, reply, option) != ERR_NONE) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "SendRequest failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }

    int32_t ret = 0;
    if (!reply.ReadInt32(ret)) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "Read ret failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    return static_cast<WMError>(ret);
}

WSError SceneSessionManagerLiteProxy::GetRecentMainSessionInfoList(
    std::vector<RecentSessionInfo>& recentSessionInfoList)
{
    TLOGD(WmsLogTag::WMS_LIFE, "in");
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::WMS_LIFE, "Write interfaceToken failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::WMS_LIFE, "remote is null");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    if (remote->SendRequest(
        static_cast<uint32_t>(SceneSessionManagerLiteMessage::TRANS_ID_GET_RECENT_MAIN_SESSION_INFO_LIST),
        data, reply, option) != ERR_NONE) {
        TLOGE(WmsLogTag::WMS_LIFE, "SendRequest failed.");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    WSError error = static_cast<WSError>(GetParcelableInfos(reply, recentSessionInfoList));
    if (error != WSError::WS_OK) {
        TLOGE(WmsLogTag::WMS_LIFE, "get info error");
        return error;
    }
    int32_t errCode = 0;
    if (!reply.ReadInt32(errCode)) {
        TLOGE(WmsLogTag::WMS_LIFE, "read errcode failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    return static_cast<WSError>(errCode);
}

WSError SceneSessionManagerLiteProxy::PendingSessionToBackgroundByPersistentId(const int32_t persistentId,
    bool shouldBackToCaller)
{
    TLOGD(WmsLogTag::WMS_LIFE, "in");
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::WMS_MAIN, "WriteInterfaceToken failed");
        return WSError::WS_ERROR_INVALID_PARAM;
    }
    if (!data.WriteInt32(persistentId)) {
        TLOGE(WmsLogTag::WMS_MAIN, "Failed to write persistentId");
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
        SceneSessionManagerLiteMessage::TRANS_ID_PENDING_SESSION_TO_BACKGROUND_BY_PERSISTENTID),
        data, reply, option) != ERR_NONE) {
        TLOGE(WmsLogTag::WMS_LIFE, "SendRequest failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    int32_t ret = 0;
    if (!reply.ReadInt32(ret)) {
        TLOGE(WmsLogTag::WMS_LIFE, "Read ret failed.");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    return static_cast<WSError>(ret);
}

WMError SceneSessionManagerLiteProxy::CreateNewInstanceKey(const std::string& bundleName, std::string& instanceKey)
{
    TLOGD(WmsLogTag::WMS_LIFE, "in");
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::WMS_LIFE, "Write interfaceToken failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    if (!data.WriteString(bundleName)) {
        TLOGE(WmsLogTag::WMS_LIFE, "write bundleName failed.");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::WMS_LIFE, "remote is null");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    if (remote->SendRequest(
        static_cast<uint32_t>(SceneSessionManagerLiteMessage::TRANS_ID_CREATE_NEW_INSTANCE_KEY),
        data, reply, option) != ERR_NONE) {
        TLOGE(WmsLogTag::WMS_LIFE, "SendRequest failed.");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    int32_t ret = 0;
    if (!reply.ReadInt32(ret)) {
        TLOGE(WmsLogTag::WMS_LIFE, "read ret failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    if (!reply.ReadString(instanceKey)) {
        TLOGE(WmsLogTag::WMS_LIFE, "read instanceKey failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    return static_cast<WMError>(ret);
}

WMError SceneSessionManagerLiteProxy::GetRouterStackInfo(
    int32_t persistentId, const sptr<ISessionRouterStackListener>& listener)
{
    TLOGD(WmsLogTag::WMS_LIFE, "in");
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::WMS_LIFE, "Write interfaceToken failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    if (!data.WriteInt32(persistentId)) {
        TLOGE(WmsLogTag::WMS_LIFE, "write persistentId failed.");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    if (listener == nullptr || listener->AsObject() == nullptr) {
        TLOGE(WmsLogTag::WMS_LIFE, "listener is null");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    if (!data.WriteRemoteObject(listener->AsObject())) {
        TLOGE(WmsLogTag::WMS_LIFE, "Write session router stack listener failed.");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::WMS_LIFE, "remote is null");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    if (remote->SendRequest(
        static_cast<uint32_t>(SceneSessionManagerLiteMessage::TRANS_ID_GET_ROUTER_STACK_INFO),
        data, reply, option) != ERR_NONE) {
        TLOGE(WmsLogTag::WMS_LIFE, "SendRequest failed.");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    return WMError::WM_OK;
}

WMError SceneSessionManagerLiteProxy::RemoveInstanceKey(const std::string& bundleName, const std::string& instanceKey)
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
    if (!data.WriteString(instanceKey)) {
        TLOGE(WmsLogTag::WMS_LIFE, "write instanceKey failed.");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::WMS_LIFE, "remote is null");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    if (remote->SendRequest(
        static_cast<uint32_t>(SceneSessionManagerLiteMessage::TRANS_ID_REMOVE_INSTANCE_KEY),
        data, reply, option) != ERR_NONE) {
        TLOGE(WmsLogTag::WMS_LIFE, "SendRequest failed.");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    int32_t ret = 0;
    if (!reply.ReadInt32(ret)) {
        TLOGE(WmsLogTag::WMS_LIFE, "read ret failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    return static_cast<WMError>(ret);
}

WMError SceneSessionManagerLiteProxy::TransferSessionToTargetScreen(
    const TransferSessionInfo& info)
{
    TLOGD(WmsLogTag::WMS_LIFE, "in");
    if (info.persistentId < 0 || info.toScreenId < 0) {
        TLOGE(WmsLogTag::WMS_LIFE, "Invalid param");
        return WMError::WM_ERROR_INVALID_PARAM;
    }
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::WMS_LIFE, "Write interfaceToken failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    if (!data.WriteInt32(info.persistentId)) {
        TLOGE(WmsLogTag::WMS_LIFE, "Write persistentId failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    if (!data.WriteInt32(info.toScreenId)) {
        TLOGE(WmsLogTag::WMS_LIFE, "Write toScreenId failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    if (!data.WriteParcelable(&info.wantParams)) {
        TLOGE(WmsLogTag::WMS_LIFE, "Write wantParams failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::WMS_LIFE, "remote is null");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    if (remote->SendRequest(
        static_cast<uint32_t>(SceneSessionManagerLiteMessage::TRANS_ID_TRANSFER_SESSION_TO_TARGET_SCREEN),
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

WMError SceneSessionManagerLiteProxy::UpdateKioskAppList(const std::vector<std::string>& kioskAppList)
{
    TLOGD(WmsLogTag::WMS_LIFE, "in");
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::WMS_LIFE, "Write interfaceToken failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    if (!data.WriteStringVector(kioskAppList)) {
        TLOGE(WmsLogTag::WMS_LIFE, "Write kioskAppList failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::WMS_LIFE, "remote is null");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    if (remote->SendRequest(
        static_cast<uint32_t>(SceneSessionManagerLiteMessage::TRANS_ID_UPDATE_KIOSK_APP_LIST),
        data, reply, option) != ERR_NONE) {
        TLOGE(WmsLogTag::WMS_LIFE, "SendRequest failed.");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    int32_t ret = 0;
    if (!reply.ReadInt32(ret)) {
        TLOGE(WmsLogTag::WMS_LIFE, "read ret failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    return static_cast<WMError>(ret);
}

WMError SceneSessionManagerLiteProxy::EnterKioskMode(const sptr<IRemoteObject>& token)
{
    TLOGD(WmsLogTag::WMS_LIFE, "in");
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::WMS_LIFE, "Write interfaceToken failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    if (!data.WriteRemoteObject(token)) {
        TLOGE(WmsLogTag::WMS_LIFE, "Write token failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::WMS_LIFE, "remote is null");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    if (remote->SendRequest(
        static_cast<uint32_t>(SceneSessionManagerLiteMessage::TRANS_ID_ENTER_KIOSK_MODE),
        data, reply, option) != ERR_NONE) {
        TLOGE(WmsLogTag::WMS_LIFE, "SendRequest failed.");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    int32_t ret = 0;
    if (!reply.ReadInt32(ret)) {
        TLOGE(WmsLogTag::WMS_LIFE, "read ret failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    return static_cast<WMError>(ret);
}

WMError SceneSessionManagerLiteProxy::ExitKioskMode(const sptr<IRemoteObject>& token)
{
    TLOGD(WmsLogTag::WMS_LIFE, "in");
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::WMS_LIFE, "Write interfaceToken failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::WMS_LIFE, "remote is null");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    if (remote->SendRequest(
        static_cast<uint32_t>(SceneSessionManagerLiteMessage::TRANS_ID_EXIT_KIOSK_MODE),
        data, reply, option) != ERR_NONE) {
        TLOGE(WmsLogTag::WMS_LIFE, "SendRequest failed.");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    int32_t ret = 0;
    if (!reply.ReadInt32(ret)) {
        TLOGE(WmsLogTag::WMS_LIFE, "read ret failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    return static_cast<WMError>(ret);
}

WSError SceneSessionManagerLiteProxy::SendPointerEventForHover(const std::shared_ptr<MMI::PointerEvent>& pointerEvent)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::WMS_EVENT, "Write interfaceToken failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    if (!pointerEvent->WriteToParcel(data)) {
        TLOGE(WmsLogTag::WMS_EVENT, "Write pointer event failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::WMS_EVENT, "Remote is null");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    if (remote->SendRequest(
        static_cast<uint32_t>(SceneSessionManagerLiteMessage::TRANS_ID_SEND_POINTER_EVENT_FOR_HOVER),
        data, reply, option) != ERR_NONE) {
        TLOGE(WmsLogTag::WMS_EVENT, "SendRequest failed.");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    int32_t ret = 0;
    if (!reply.ReadInt32(ret)) {
        TLOGE(WmsLogTag::WMS_EVENT, "Read ret failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    return static_cast<WSError>(ret);
}

WMError SceneSessionManagerLiteProxy::SetPipEnableByScreenId(int32_t screenId, bool isEnabled)
{
    TLOGD(WmsLogTag::WMS_PIP, "SetPipEnableByScreenId");
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::WMS_PIP, "Write interfaceToken failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    if (!data.WriteInt32(screenId)) {
        TLOGE(WmsLogTag::WMS_PIP, "Failed to write screenId");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    if (!data.WriteBool(isEnabled)) {
        TLOGE(WmsLogTag::WMS_PIP, "Write isEnabled failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::WMS_PIP, "remote is null");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    if (remote->SendRequest(
        static_cast<uint32_t>(SceneSessionManagerLiteMessage::TRANS_ID_SET_PIP_ENABLED_BY_SCREENID),
        data, reply, option) != ERR_NONE) {
        TLOGE(WmsLogTag::WMS_PIP, "SendRequest failed.");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    int32_t ret = 0;
    if (!reply.ReadInt32(ret)) {
        TLOGE(WmsLogTag::WMS_PIP, "read ret failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    return static_cast<WMError>(ret);
}

WMError SceneSessionManagerLiteProxy::UnsetPipEnableByScreenId(int32_t screenId)
{
    TLOGD(WmsLogTag::WMS_PIP, "UnsetPipEnableByScreenId");
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::WMS_PIP, "Write interfaceToken failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    if (!data.WriteInt32(screenId)) {
        TLOGE(WmsLogTag::WMS_PIP, "Failed to write screenId");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::WMS_PIP, "remote is null");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    if (remote->SendRequest(
        static_cast<uint32_t>(SceneSessionManagerLiteMessage::TRANS_ID_UNSET_PIP_ENABLED_BY_SCREENID),
        data, reply, option) != ERR_NONE) {
        TLOGE(WmsLogTag::WMS_PIP, "SendRequest failed.");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    int32_t ret = 0;
    if (!reply.ReadInt32(ret)) {
        TLOGE(WmsLogTag::WMS_PIP, "read ret failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    return static_cast<WMError>(ret);
}

WMError SceneSessionManagerLiteProxy::RegisterPipChgListenerByScreenId(int32_t screenId,
    const sptr<IPipChangeListener>& listener)
{
    TLOGD(WmsLogTag::WMS_PIP, "in");
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::WMS_PIP, "Write interfaceToken failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    if (!data.WriteInt32(screenId)) {
        TLOGE(WmsLogTag::WMS_PIP, "Failed to write screenId");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    if (listener == nullptr || listener->AsObject() == nullptr) {
        TLOGE(WmsLogTag::WMS_PIP, "listener is null");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    if (!data.WriteRemoteObject(listener->AsObject())) {
        TLOGE(WmsLogTag::WMS_PIP, "Write listener failed.");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::WMS_PIP, "remote is null");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    if (remote->SendRequest(static_cast<uint32_t>(SceneSessionManagerLiteMessage::TRANS_ID_REGISTER_PIP_CHG_LISTENER),
        data, reply, option) != ERR_NONE) {
        TLOGE(WmsLogTag::WMS_PIP, "SendRequest failed.");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    int32_t ret = 0;
    if (!reply.ReadInt32(ret)) {
        TLOGE(WmsLogTag::WMS_PIP, "Read ret failed.");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    return static_cast<WMError>(ret);
}

WMError SceneSessionManagerLiteProxy::UnregisterPipChgListenerByScreenId(int32_t screenId)
{
    TLOGD(WmsLogTag::WMS_PIP, "in");
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::WMS_PIP, "Write interfaceToken failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    if (!data.WriteInt32(screenId)) {
        TLOGE(WmsLogTag::WMS_PIP, "Failed to write screenId");
        return WMError::WM_ERROR_IPC_FAILED;
    }

    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::WMS_PIP, "remote is null");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    if (remote->SendRequest(
        static_cast<uint32_t>(SceneSessionManagerLiteMessage::TRANS_ID_UNREGISTER_PIP_CHG_LISTENER),
        data, reply, option) != ERR_NONE) {
        TLOGE(WmsLogTag::WMS_PIP, "SendRequest failed.");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    int32_t ret = 0;
    if (!reply.ReadInt32(ret)) {
        TLOGE(WmsLogTag::WMS_PIP, "Read ret failed.");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    return static_cast<WMError>(ret);
}

WMError SceneSessionManagerLiteProxy::GetDisplayIdByWindowId(const std::vector<uint64_t>& windowIds,
    std::unordered_map<uint64_t, DisplayId>& windowDisplayIdMap)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "WriteInterfaceToken failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    if (!data.WriteUInt64Vector(windowIds)) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "Write windowIds failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    if (Remote()->SendRequest(static_cast<uint32_t>(SceneSessionManagerLiteMessage::TRANS_ID_GET_DISPLAYID_BY_WINDOWID),
        data, reply, option) != ERR_NONE) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "SendRequest failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    int32_t mapSize;
    if (!reply.ReadInt32(mapSize)) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "Fail to read mapSize");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    for (int32_t i = 0; i < mapSize; i++) {
        uint64_t windowId;
        if (!reply.ReadUint64(windowId)) {
            TLOGE(WmsLogTag::WMS_ATTRIBUTE, "Fail to read windowId");
            return WMError::WM_ERROR_IPC_FAILED;
        }
        uint64_t displayId;
        if (!reply.ReadUint64(displayId)) {
            TLOGE(WmsLogTag::WMS_ATTRIBUTE, "Fail to read displayId");
            return WMError::WM_ERROR_IPC_FAILED;
        }
        windowDisplayIdMap[windowId] = displayId;
    }
    return static_cast<WMError>(reply.ReadInt32());
}
} // namespace OHOS::Rosen
