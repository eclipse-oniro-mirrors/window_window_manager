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

#ifndef SUPPORT_SCREEN
#define SUPPORT_SCREEN
#endif
#include "session_manager/include/zidl/scene_session_manager_proxy.h"

#include <ipc_types.h>
#include <message_option.h>
#include <message_parcel.h>
#include <ui/rs_surface_node.h>

#include "marshalling_helper.h"
#include "permission.h"
#include "window_manager.h"
#include "window_manager_hilog.h"
#include "ui_effect_controller_interface.h"
#include "ui_effect_controller_client_interface.h"

namespace OHOS::Rosen {
namespace {
constexpr int32_t CYCLE_LIMIT = 1000;
constexpr HiviewDFX::HiLogLabel LABEL = {LOG_CORE, HILOG_DOMAIN_WINDOW, "SceneSessionManagerProxy"};
constexpr DisplayId VIRTUAL_DISPLAY_ID = 999;
}
WSError SceneSessionManagerProxy::CreateAndConnectSpecificSession(const sptr<ISessionStage>& sessionStage,
    const sptr<IWindowEventChannel>& eventChannel, const std::shared_ptr<RSSurfaceNode>& surfaceNode,
    sptr<WindowSessionProperty> property, int32_t& persistentId, sptr<ISession>& session,
    SystemSessionConfig& systemConfig, sptr<IRemoteObject> token)
{
    MessageOption option(MessageOption::TF_SYNC);
    MessageParcel data;
    MessageParcel reply;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::WMS_LIFE, "Write InterfaceToken failed!");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    if (!data.WriteRemoteObject(sessionStage->AsObject())) {
        TLOGE(WmsLogTag::WMS_LIFE, "Write ISessionStage failed!");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    if (!data.WriteRemoteObject(eventChannel->AsObject())) {
        TLOGE(WmsLogTag::WMS_LIFE, "Write IWindowEventChannel failed!");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    if (!surfaceNode || !surfaceNode->Marshalling(data)) {
        TLOGE(WmsLogTag::WMS_LIFE, "Write surfaceNode failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    if (!property || !data.WriteStrongParcelable(property)) {
        TLOGE(WmsLogTag::WMS_LIFE, "Write property failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    if (token != nullptr) {
        if (!data.WriteRemoteObject(token)) {
            return WSError::WS_ERROR_IPC_FAILED;
        }
    }

    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::WMS_LIFE, "remote is null");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    if (remote->SendRequest(static_cast<uint32_t>(
        SceneSessionManagerMessage::TRANS_ID_CREATE_AND_CONNECT_SPECIFIC_SESSION), data, reply, option) != ERR_NONE) {
        TLOGE(WmsLogTag::WMS_LIFE, "SendRequest failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    persistentId = reply.ReadInt32();
    sptr<IRemoteObject> sessionObject = reply.ReadRemoteObject();
    if (sessionObject == nullptr) {
        TLOGE(WmsLogTag::WMS_LIFE, "ReadRemoteObject failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    session = iface_cast<ISession>(sessionObject);
    sptr<SystemSessionConfig> config = reply.ReadParcelable<SystemSessionConfig>();
    if (config) {
        systemConfig = *config;
    }
    uint32_t level = 0;
    if (!reply.ReadUint32(level)) {
        TLOGE(WmsLogTag::WMS_LIFE, "Read level failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    property->SetSubWindowLevel(level);
    uint64_t displayId = 0;
    if (!reply.ReadUint64(displayId)) {
        TLOGE(WmsLogTag::WMS_LIFE, "Read displayId failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    if (property->GetDisplayId() != VIRTUAL_DISPLAY_ID) {
        property->SetDisplayId(displayId);
    }
    uint32_t windowType = 0;
    if (!reply.ReadUint32(windowType)) {
        TLOGE(WmsLogTag::WMS_LIFE, "Read windowType failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    property->SetWindowType(static_cast<WindowType>(windowType));
    int32_t ret = reply.ReadInt32();
    return static_cast<WSError>(ret);
}

WSError SceneSessionManagerProxy::RecoverAndConnectSpecificSession(const sptr<ISessionStage>& sessionStage,
    const sptr<IWindowEventChannel>& eventChannel, const std::shared_ptr<RSSurfaceNode>& surfaceNode,
    sptr<WindowSessionProperty> property, sptr<ISession>& session, sptr<IRemoteObject> token)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        WLOGFE("Write InterfaceToken failed!");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    if (!sessionStage || !data.WriteRemoteObject(sessionStage->AsObject())) {
        WLOGFE("Write ISessionStage failed!");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    if (!eventChannel || !data.WriteRemoteObject(eventChannel->AsObject())) {
        WLOGFE("Write IWindowEventChannel failed!");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    if (!surfaceNode || !surfaceNode->Marshalling(data)) {
        WLOGFE("Write surfaceNode failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }

    if (property) {
        if (!data.WriteBool(true) || !data.WriteParcelable(property.GetRefPtr())) {
            return WSError::WS_ERROR_IPC_FAILED;
        }
    } else {
        if (!data.WriteBool(false)) {
            return WSError::WS_ERROR_IPC_FAILED;
        }
    }
    if (token != nullptr && !data.WriteRemoteObject(token)) {
        return WSError::WS_ERROR_IPC_FAILED;
    }

    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        WLOGFE("remote is null");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    if (remote->SendRequest(static_cast<uint32_t>(
        SceneSessionManagerMessage::TRANS_ID_RECOVER_AND_CONNECT_SPECIFIC_SESSION),
        data, reply, option) != ERR_NONE) {
        WLOGFE("SendRequest failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    sptr<IRemoteObject> sessionObject = reply.ReadRemoteObject();
    if (sessionObject == nullptr) {
        WLOGFE("ReadRemoteObject failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    session = iface_cast<ISession>(sessionObject);
    int32_t ret = reply.ReadInt32();
    return static_cast<WSError>(ret);
}
WSError SceneSessionManagerProxy::RecoverAndReconnectSceneSession(const sptr<ISessionStage>& sessionStage,
    const sptr<IWindowEventChannel>& eventChannel, const std::shared_ptr<RSSurfaceNode>& surfaceNode,
    sptr<ISession>& session, sptr<WindowSessionProperty> property, sptr<IRemoteObject> token)
{
    MessageOption option(MessageOption::TF_SYNC);
    MessageParcel data;
    MessageParcel reply;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        WLOGFE("Write InterfaceToken failed!");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    if (!sessionStage || !data.WriteRemoteObject(sessionStage->AsObject())) {
        WLOGFE("Write ISessionStage failed!");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    if (!eventChannel || !data.WriteRemoteObject(eventChannel->AsObject())) {
        WLOGFE("Write IWindowEventChannel failed!");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    if (!surfaceNode || !surfaceNode->Marshalling(data)) {
        WLOGFE("Write surfaceNode failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }

    if (property) {
        if (!data.WriteBool(true) || !data.WriteParcelable(property.GetRefPtr())) {
            return WSError::WS_ERROR_IPC_FAILED;
        }
    } else {
        if (!data.WriteBool(false)) {
            return WSError::WS_ERROR_IPC_FAILED;
        }
    }
    if (token != nullptr && !data.WriteRemoteObject(token)) {
        return WSError::WS_ERROR_IPC_FAILED;
    }

    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        WLOGFE("remote is null");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    if (remote->SendRequest(
        static_cast<uint32_t>(SceneSessionManagerMessage::TRANS_ID_RECOVER_AND_RECONNECT_SCENE_SESSION), data, reply,
        option) != ERR_NONE) {
        WLOGFE("SendRequest failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    sptr<IRemoteObject> sessionObject = reply.ReadRemoteObject();
    if (sessionObject == nullptr) {
        WLOGFE("ReadRemoteObject failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    session = iface_cast<ISession>(sessionObject);
    int32_t ret = reply.ReadInt32();
    return static_cast<WSError>(ret);
}

WMError SceneSessionManagerProxy::SetParentWindow(int32_t subWindowId, int32_t newParentWindowId)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::WMS_SUB, "Write interfaceToken failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    if (!data.WriteInt32(subWindowId)) {
        TLOGE(WmsLogTag::WMS_SUB, "Write subWindowId failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    if (!data.WriteInt32(newParentWindowId)) {
        TLOGE(WmsLogTag::WMS_SUB, "Write newParentWindowId failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::WMS_SUB, "remote is null");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    if (remote->SendRequest(static_cast<uint32_t>(SceneSessionManagerMessage::TRANS_ID_SET_PARENT_WINDOW),
        data, reply, option) != ERR_NONE) {
        TLOGE(WmsLogTag::WMS_SUB, "SendRequest failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    uint32_t ret = 0;
    if (!reply.ReadUint32(ret)) {
        TLOGE(WmsLogTag::WMS_SUB, "Read ret failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    return static_cast<WMError>(ret);
}

WMError SceneSessionManagerProxy::GetSessionSnapshotById(int32_t persistentId, SessionSnapshot& snapshot)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::WMS_SYSTEM, "WriteInterfaceToken failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    if (!data.WriteInt32(persistentId)) {
        TLOGE(WmsLogTag::WMS_SYSTEM, "Write persistentId failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }

    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::WMS_SYSTEM, "remote is null");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    if (remote->SendRequest(static_cast<uint32_t>(SceneSessionManagerMessage::TRANS_ID_GET_SESSION_SNAPSHOT_BY_ID),
        data, reply, option) != ERR_NONE) {
        TLOGE(WmsLogTag::WMS_SYSTEM, "SendRequest failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    sptr<SessionSnapshot> info(reply.ReadParcelable<SessionSnapshot>());
    if (info) {
        snapshot = *info;
    } else {
        TLOGE(WmsLogTag::WMS_SYSTEM, "Read snapshot is null.");
    }
    return static_cast<WMError>(reply.ReadInt32());
}

WMError SceneSessionManagerProxy::GetSnapshotByWindowId(int32_t persistentId,
    std::shared_ptr<Media::PixelMap>& pixelMap)
{
    SessionSnapshot snapshot;
    WMError ret = GetSessionSnapshotById(persistentId, snapshot);
    if (ret == WMError::WM_OK) {
        pixelMap = snapshot.snapshot;
    }
    return ret;
}

WSError SceneSessionManagerProxy::DestroyAndDisconnectSpecificSession(const int32_t persistentId)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::WMS_LIFE, "WriteInterfaceToken failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    if (!data.WriteInt32(persistentId)) {
        TLOGE(WmsLogTag::WMS_LIFE, "Write persistentId failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::WMS_LIFE, "remote is null");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    if (remote->SendRequest(static_cast<uint32_t>(
        SceneSessionManagerMessage::TRANS_ID_DESTROY_AND_DISCONNECT_SPECIFIC_SESSION),
        data, reply, option) != ERR_NONE) {
        TLOGE(WmsLogTag::WMS_LIFE, "SendRequest failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    int32_t ret = reply.ReadInt32();
    return static_cast<WSError>(ret);
}

WSError SceneSessionManagerProxy::DestroyAndDisconnectSpecificSessionWithDetachCallback(const int32_t persistentId,
    const sptr<IRemoteObject>& callback)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::WMS_LIFE, "WriteInterfaceToken failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    if (!data.WriteInt32(persistentId)) {
        TLOGE(WmsLogTag::WMS_LIFE, "Write persistentId failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    if (callback == nullptr || !data.WriteRemoteObject(callback)) {
        TLOGE(WmsLogTag::WMS_LIFE, "Write callback failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::WMS_LIFE, "remote is null");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    if (remote->SendRequest(static_cast<uint32_t>(
        SceneSessionManagerMessage::TRANS_ID_DESTROY_AND_DISCONNECT_SPECIFIC_SESSION_WITH_DETACH_CALLBACK),
        data, reply, option) != ERR_NONE) {
        TLOGE(WmsLogTag::WMS_LIFE, "SendRequest failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    int32_t ret = reply.ReadInt32();
    return static_cast<WSError>(ret);
}

WMError SceneSessionManagerProxy::RequestFocusStatus(int32_t persistentId, bool isFocused, bool byForeground,
    FocusChangeReason reason)
{
    TLOGI(WmsLogTag::WMS_FOCUS, "SceneSessionManagerProxy::RequestFocusStatus id: %{public}d, focusState:\
        %{public}u, byForeground: %{public}d, reason: %{public}d", persistentId, isFocused, byForeground, reason);
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        WLOGFE("WriteInterfaceToken failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    if (!data.WriteInt32(persistentId)) {
        WLOGFE("Write persistentId failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    if (!data.WriteBool(isFocused)) {
        WLOGFE("Write isFocused failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }

    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        WLOGFE("remote is null");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    if (remote->SendRequest(static_cast<uint32_t>(
        SceneSessionManagerMessage::TRANS_ID_REQUEST_FOCUS),
        data, reply, option) != ERR_NONE) {
        WLOGFE("SendRequest failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    int32_t ret = reply.ReadInt32();
    return static_cast<WMError>(ret);
}

WMError SceneSessionManagerProxy::RequestFocusStatusBySA(int32_t persistentId, bool isFocused,
    bool byForeground, FocusChangeReason reason)
{
    TLOGI(WmsLogTag::WMS_FOCUS,
        "id: %{public}d, focusState: %{public}d, byForeground: %{public}d, reason: %{public}d",
        persistentId, isFocused, byForeground, static_cast<int32_t>(reason));
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::WMS_FOCUS, "WriteInterfaceToken failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    if (!data.WriteInt32(persistentId)) {
        TLOGE(WmsLogTag::WMS_FOCUS, "Write persistentId failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    if (!data.WriteBool(isFocused)) {
        TLOGE(WmsLogTag::WMS_FOCUS, "Write isFocused failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    if (!data.WriteBool(byForeground)) {
        TLOGE(WmsLogTag::WMS_FOCUS, "Write byForeground failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    if (!data.WriteInt32(static_cast<int32_t>(reason))) {
        TLOGE(WmsLogTag::WMS_FOCUS, "Write reason failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }

    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::WMS_FOCUS, "remote is null");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    if (remote->SendRequest(static_cast<uint32_t>(
        SceneSessionManagerMessage::TRANS_ID_REQUEST_FOCUS_STATUS_BY_SA),
        data, reply, option) != ERR_NONE) {
        TLOGE(WmsLogTag::WMS_FOCUS, "SendRequest failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    int32_t ret = reply.ReadInt32();
    return static_cast<WMError>(ret);
}

WSError SceneSessionManagerProxy::RaiseWindowToTop(int32_t persistentId)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        WLOGFE("WriteInterfaceToken failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    if (!data.WriteInt32(persistentId)) {
        WLOGFE("Write persistentId failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }

    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        WLOGFE("remote is null");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    if (remote->SendRequest(static_cast<uint32_t>(
        SceneSessionManagerMessage::TRANS_ID_RAISE_WINDOW_TO_TOP),
        data, reply, option) != ERR_NONE) {
        WLOGFE("SendRequest failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    int32_t ret = reply.ReadInt32();
    return static_cast<WSError>(ret);
}

WSError SceneSessionManagerProxy::BindDialogSessionTarget(uint64_t persistentId, sptr<IRemoteObject> targetToken)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        WLOGFE("WriteInterfaceToken failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    if (!data.WriteUint64(persistentId)) {
        WLOGFE("Write PropertyChangeAction failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    if (targetToken != nullptr) {
        if (!data.WriteRemoteObject(targetToken)) {
            WLOGFE("Write targetToken failed");
            return WSError::WS_ERROR_IPC_FAILED;
        }
    }

    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        WLOGFE("remote is null");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    if (remote->SendRequest(static_cast<uint32_t>(
        SceneSessionManagerMessage::TRANS_ID_BIND_DIALOG_TARGET),
        data, reply, option) != ERR_NONE) {
        WLOGFE("SendRequest failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    int32_t ret = reply.ReadInt32();
    return static_cast<WSError>(ret);
}

WSError SceneSessionManagerProxy::UpdateSessionAvoidAreaListener(int32_t persistentId, bool haveListener)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);

    if (!data.WriteInterfaceToken(GetDescriptor())) {
        WLOGFE("WriteInterfaceToken failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    if (!data.WriteInt32(persistentId)) {
        WLOGFE("Write persistentId failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    if (!data.WriteBool(haveListener)) {
        WLOGFE("Write avoid area listener failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        WLOGFE("remote is null");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    if (remote->SendRequest(static_cast<uint32_t>(
        SceneSessionManagerMessage::TRANS_ID_UPDATE_AVOIDAREA_LISTENER),
        data, reply, option) != ERR_NONE) {
        return WSError::WS_ERROR_IPC_FAILED;
    }
    return static_cast<WSError>(reply.ReadInt32());
}

WMError SceneSessionManagerProxy::NotifyScreenshotEvent(ScreenshotEventType type)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "Write interfaceToken failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    if (!data.WriteInt32(static_cast<int32_t>(type))) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "write screenshot event type failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "remote is null");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    if (remote->SendRequest(static_cast<uint32_t>(
        SceneSessionManagerMessage::TRANS_ID_NOTIFY_SCREEN_SHOT_EVENT), data, reply, option) != ERR_NONE) {
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

WSError SceneSessionManagerProxy::UpdateSessionTouchOutsideListener(int32_t& persistentId, bool haveListener)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);

    if (!data.WriteInterfaceToken(GetDescriptor())) {
        WLOGFE("WriteInterfaceToken failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    if (!data.WriteInt32(persistentId)) {
        WLOGFE("Write persistentId failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    if (!data.WriteBool(haveListener)) {
        WLOGFE("Write avoid area listener failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        WLOGFE("remote is null");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    if (remote->SendRequest(static_cast<uint32_t>(
        SceneSessionManagerMessage::TRANS_ID_UPDATE_TOUCHOUTSIDE_LISTENER),
        data, reply, option) != ERR_NONE) {
        return WSError::WS_ERROR_IPC_FAILED;
    }
    return static_cast<WSError>(reply.ReadInt32());
}

WSError SceneSessionManagerProxy::UpdateSessionWindowVisibilityListener(int32_t persistentId, bool haveListener)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);

    if (!data.WriteInterfaceToken(GetDescriptor())) {
        WLOGFE("WriteInterfaceToken failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    if (!data.WriteInt32(persistentId)) {
        WLOGFE("Write persistentId failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    if (!data.WriteBool(haveListener)) {
        WLOGFE("Write avoid area listener failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        WLOGFE("remote is null");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    if (remote->SendRequest(static_cast<uint32_t>(
        SceneSessionManagerMessage::TRANS_ID_UPDATE_WINDOW_VISIBILITY_LISTENER),
        data, reply, option) != ERR_NONE) {
        return WSError::WS_ERROR_IPC_FAILED;
    }
    return static_cast<WSError>(reply.ReadInt32());
}

WMError SceneSessionManagerProxy::UpdateSessionOcclusionStateListener(int32_t persistentId, bool haveListener)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);

    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "write interfaceToken failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    if (!data.WriteInt32(persistentId)) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "write persistentId failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    if (!data.WriteBool(haveListener)) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "write haveListener failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "remote is null");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    auto reqErrCode = remote->SendRequest(
        static_cast<uint32_t>(SceneSessionManagerMessage::TRANS_ID_UPDATE_SESSION_OCCLUSION_STATE_LISTENER),
        data, reply, option);
    if (reqErrCode != ERR_NONE) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "send request failed, errCode: %{public}d", reqErrCode);
        return WMError::WM_ERROR_IPC_FAILED;
    }
    int32_t errCode = 0;
    if (!reply.ReadInt32(errCode)) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "read errcode failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    return static_cast<WMError>(errCode);
}

WMError SceneSessionManagerProxy::RecoverWindowPropertyChangeFlag(uint32_t observedFlags, uint32_t interestedFlags)
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
        SceneSessionManagerMessage::TRANS_ID_RECOVER_WINDOW_PROPERTY_CHANGE_FLAG),
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

WMError SceneSessionManagerProxy::RegisterWindowManagerAgent(WindowManagerAgentType type,
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
        WLOGFE("remote is null");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    if (remote->SendRequest(static_cast<uint32_t>(
        SceneSessionManagerMessage::TRANS_ID_REGISTER_WINDOW_MANAGER_AGENT),
        data, reply, option) != ERR_NONE) {
        WLOGFE("SendRequest failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }

    return static_cast<WMError>(reply.ReadInt32());
}

WMError SceneSessionManagerProxy::UnregisterWindowManagerAgent(WindowManagerAgentType type,
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
        WLOGFE("remote is null");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    if (remote->SendRequest(static_cast<uint32_t>(
        SceneSessionManagerMessage::TRANS_ID_UNREGISTER_WINDOW_MANAGER_AGENT),
        data, reply, option) != ERR_NONE) {
        WLOGFE("SendRequest failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }

    return static_cast<WMError>(reply.ReadInt32());
}

WMError SceneSessionManagerProxy::RegisterWindowPropertyChangeAgent(WindowInfoKey windowInfoKey, uint32_t interestInfo,
    const sptr<IWindowManagerAgent>& windowManagerAgent)
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
        return WMError::WM_ERROR_NULLPTR;
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
        SceneSessionManagerMessage::TRANS_ID_REGISTER_WINDOW_PROPERTY_CHANGE_AGENT),
        data, reply, option) != ERR_NONE) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "SendRequest failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }

    return static_cast<WMError>(reply.ReadInt32());
}

WMError SceneSessionManagerProxy::UnregisterWindowPropertyChangeAgent(WindowInfoKey windowInfoKey,
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
        return WMError::WM_ERROR_NULLPTR;
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
        SceneSessionManagerMessage::TRANS_ID_UNREGISTER_WINDOW_PROPERTY_CHANGE_AGENT),
        data, reply, option) != ERR_NONE) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "SendRequest failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }

    return static_cast<WMError>(reply.ReadInt32());
}

WMError SceneSessionManagerProxy::SetGestureNavigationEnabled(bool enable)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        WLOGFE("Write InterfaceToken failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }

    if (!data.WriteBool(enable)) {
        WLOGFE("Write enable failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }

    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        WLOGFE("remote is null");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    if (remote->SendRequest(static_cast<uint32_t>(
        SceneSessionManagerMessage::TRANS_ID_SET_GESTURE_NAVIGATION_ENABLED), data, reply, option) != ERR_NONE) {
        WLOGFE("SendRequest failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    int32_t ret = reply.ReadInt32();
    return static_cast<WMError>(ret);
}

void SceneSessionManagerProxy::GetFocusWindowInfo(FocusChangeInfo& focusInfo, DisplayId displayId)
{
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
    if (remote->SendRequest(static_cast<uint32_t>(SceneSessionManagerMessage::TRANS_ID_GET_FOCUS_SESSION_INFO),
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

void SceneSessionManagerProxy::GetFocusWindowInfoByAbilityToken(FocusChangeInfo& focusInfo,
    const sptr<IRemoteObject>& abilityToken)
{
    if (!abilityToken) {
        TLOGE(WmsLogTag::WMS_FOCUS, "AbilityToken is null");
        return;
    }
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::WMS_FOCUS, "WriteInterfaceToken failed");
        return;
    }
    if (!data.WriteRemoteObject(abilityToken)) {
        TLOGE(WmsLogTag::WMS_FOCUS, "Write abilityToken failed");
        return;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::WMS_FOCUS, "Remote is null");
        return;
    }
    if (remote->SendRequest(static_cast<uint32_t>(
        SceneSessionManagerMessage::TRANS_ID_GET_FOCUS_SESSION_INFO_BY_ABILITY_TOKEN), data, reply, option) !=
        ERR_NONE) {
        TLOGE(WmsLogTag::WMS_FOCUS, "SendRequest failed");
        return;
    }
    sptr<FocusChangeInfo> info = reply.ReadParcelable<FocusChangeInfo>();
    if (info) {
        focusInfo = *info;
    } else {
        TLOGE(WmsLogTag::WMS_FOCUS, "Info is null");
    }
}

WSError SceneSessionManagerProxy::SetSessionLabel(const sptr<IRemoteObject>& token, const std::string& label)
{
    WLOGFI("run SceneSessionManagerProxy::SetSessionLabel");
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
    if (remote->SendRequest(static_cast<uint32_t>(SceneSessionManagerMessage::TRANS_ID_SET_SESSION_LABEL),
        data, reply, option) != ERR_NONE) {
        WLOGFE("SendRequest failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    return static_cast<WSError>(reply.ReadInt32());
}

WSError SceneSessionManagerProxy::SetSessionIcon(const sptr<IRemoteObject>& token,
    const std::shared_ptr<Media::PixelMap>& icon)
{
    WLOGFI("run SceneSessionManagerProxy::SetSessionIcon");
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
    if (remote->SendRequest(static_cast<uint32_t>(SceneSessionManagerMessage::TRANS_ID_SET_SESSION_ICON),
        data, reply, option) != ERR_NONE) {
        WLOGFE("SendRequest failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    return static_cast<WSError>(reply.ReadInt32());
}

WSError SceneSessionManagerProxy::IsValidSessionIds(
    const std::vector<int32_t>& sessionIds, std::vector<bool>& results)
{
    WLOGFI("run SceneSessionManagerProxy::IsValidSessionIds");
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
    if (remote->SendRequest(static_cast<uint32_t>(SceneSessionManagerMessage::TRANS_ID_IS_VALID_SESSION_IDS),
        data, reply, option) != ERR_NONE) {
        WLOGFE("SendRequest failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }

    reply.ReadBoolVector(&results);
    return static_cast<WSError>(reply.ReadInt32());
}

WMError SceneSessionManagerProxy::GetAccessibilityWindowInfo(std::vector<sptr<AccessibilityWindowInfo>>& infos)
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
        SceneSessionManagerMessage::TRANS_ID_GET_WINDOW_INFO),
        data, reply, option) != ERR_NONE) {
        WLOGFE("SendRequest failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }

    if (!MarshallingHelper::UnmarshallingVectorParcelableObj<AccessibilityWindowInfo>(reply, infos)) {
        WLOGFE("read window info failed.");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    return static_cast<WMError>(reply.ReadUint32());
}

WMError SceneSessionManagerProxy::GetUnreliableWindowInfo(int32_t windowId,
    std::vector<sptr<UnreliableWindowInfo>>& infos)
{
    TLOGD(WmsLogTag::DEFAULT, "run!");
    MessageOption option;
    MessageParcel reply;
    MessageParcel data;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::DEFAULT, "Write InterfaceToken failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }

    if (!data.WriteInt32(windowId)) {
        TLOGE(WmsLogTag::DEFAULT, "Write windowId failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }

    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::DEFAULT, "remote is null");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    if (remote->SendRequest(static_cast<uint32_t>(
        SceneSessionManagerMessage::TRANS_ID_GET_UNRELIABLE_WINDOW_INFO),
        data, reply, option) != ERR_NONE) {
        TLOGE(WmsLogTag::DEFAULT, "SendRequest failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }

    if (!MarshallingHelper::UnmarshallingVectorParcelableObj<UnreliableWindowInfo>(reply, infos)) {
        TLOGE(WmsLogTag::DEFAULT, "read window info failed.");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    return static_cast<WMError>(reply.ReadInt32());
}

WSError SceneSessionManagerProxy::PendingSessionToForeground(const sptr<IRemoteObject>& token, int32_t windowMode)
{
    WLOGFI("run SceneSessionManagerProxy::PendingSessionToForeground");
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
    if (remote->SendRequest(static_cast<uint32_t>(SceneSessionManagerMessage::TRANS_ID_PENDING_SESSION_TO_FOREGROUND),
        data, reply, option) != ERR_NONE) {
        WLOGFE("SendRequest failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    return static_cast<WSError>(reply.ReadInt32());
}

WSError SceneSessionManagerProxy::PendingSessionToBackgroundForDelegator(const sptr<IRemoteObject>& token,
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
        SceneSessionManagerMessage::TRANS_ID_PENDING_SESSION_TO_BACKGROUND_FOR_DELEGATOR),
        data, reply, option) != ERR_NONE) {
        TLOGE(WmsLogTag::WMS_LIFE, "SendRequest failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    return static_cast<WSError>(reply.ReadInt32());
}

WSError SceneSessionManagerProxy::RegisterSessionListener(const sptr<ISessionListener>& listener)
{
    WLOGFI("run SceneSessionManagerProxy::RegisterSessionListener");
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
    if (remote->SendRequest(static_cast<uint32_t>(SceneSessionManagerMessage::TRANS_ID_REGISTER_SESSION_LISTENER),
        data, reply, option) != ERR_NONE) {
        WLOGFE("SendRequest failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    return static_cast<WSError>(reply.ReadInt32());
}

WSError SceneSessionManagerProxy::UnRegisterSessionListener(const sptr<ISessionListener>& listener)
{
    WLOGFI("run SceneSessionManagerProxy::UnRegisterSessionListener");
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
        static_cast<uint32_t>(SceneSessionManagerMessage::TRANS_ID_UNREGISTER_SESSION_LISTENER),
        data, reply, option) != ERR_NONE) {
        WLOGFE("SendRequest failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    return static_cast<WSError>(reply.ReadInt32());
}

WSError SceneSessionManagerProxy::GetSessionInfos(const std::string& deviceId, int32_t numMax,
                                                  std::vector<SessionInfoBean>& sessionInfos)
{
    WLOGFI("run SceneSessionManagerProxy::GetSessionInfos");
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
    if (remote->SendRequest(static_cast<uint32_t>(SceneSessionManagerMessage::TRANS_ID_GET_MISSION_INFOS),
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

WSError SceneSessionManagerProxy::GetSessionInfo(const std::string& deviceId, int32_t persistentId,
                                                 SessionInfoBean& sessionInfo)
{
    WLOGFI("run SceneSessionManagerProxy::GetSessionInfo");
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
    if (remote->SendRequest(static_cast<uint32_t>(SceneSessionManagerMessage::TRANS_ID_GET_MISSION_INFO_BY_ID),
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

WSError SceneSessionManagerProxy::GetSessionInfoByContinueSessionId(
    const std::string& continueSessionId, SessionInfoBean& sessionInfo)
{
    TLOGD(WmsLogTag::WMS_LIFE, "run query session info");
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
        SceneSessionManagerMessage::TRANS_ID_GET_SESSION_INFO_BY_CONTINUE_SESSION_ID),
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

WSError SceneSessionManagerProxy::DumpSessionAll(std::vector<std::string>& infos)
{
    WLOGFI("run SceneSessionManagerProxy::DumpSessionAll");
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        WLOGFE("DumpSessionAll write interface token failed.");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        WLOGFE("remote is null");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    if (remote->SendRequest(static_cast<uint32_t>(SceneSessionManagerMessage::TRANS_ID_DUMP_SESSION_ALL),
        data, reply, option) != ERR_NONE) {
        WLOGFE("DumpSessionAll sendRequest failed.");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    if (!reply.ReadStringVector(&infos)) {
        WLOGFE("DumpSessionAll read session info failed.");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    return static_cast<WSError>(reply.ReadInt32());
}

WSError SceneSessionManagerProxy::DumpSessionWithId(int32_t persistentId, std::vector<std::string>& infos)
{
    WLOGFI("run SceneSessionManagerProxy::DumpSessionWithId");
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        WLOGFE("DumpSessionWithId write interface token failed.");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    if (!data.WriteInt32(persistentId)) {
        WLOGFE("DumpSessionWithId write persistentId failed.");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        WLOGFE("remote is null");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    if (remote->SendRequest(static_cast<uint32_t>(SceneSessionManagerMessage::TRANS_ID_DUMP_SESSION_WITH_ID),
        data, reply, option) != ERR_NONE) {
        WLOGFE("DumpSessionWithId sendRequest failed.");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    if (!reply.ReadStringVector(&infos)) {
        WLOGFE("DumpSessionWithId read session info failed.");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    return static_cast<WSError>(reply.ReadInt32());
}

template<typename T>
WSError SceneSessionManagerProxy::GetParcelableInfos(MessageParcel& reply, std::vector<T>& parcelableInfos)
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

WSError SceneSessionManagerProxy::TerminateSessionNew(const sptr<AAFwk::SessionInfo> abilitySessionInfo,
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
    if (remote->SendRequest(static_cast<uint32_t>(SceneSessionManagerMessage::TRANS_ID_TERMINATE_SESSION_NEW),
        data, reply, option) != ERR_NONE) {
        WLOGFE("SendRequest failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    return static_cast<WSError>(reply.ReadInt32());
}

WSError SceneSessionManagerProxy::GetFocusSessionToken(sptr<IRemoteObject>& token, DisplayId displayId)
{
    WLOGFD("run SceneSessionManagerProxy::GetFocusSessionToken");
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        WLOGFE("Write interfaceToken failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    if (!data.WriteUint64(displayId)) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "write displayId failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        WLOGFE("remote is null");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    if (remote->SendRequest(static_cast<uint32_t>(SceneSessionManagerMessage::TRANS_ID_GET_FOCUS_SESSION_TOKEN),
        data, reply, option) != ERR_NONE) {
        WLOGFE("SendRequest failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    token = reply.ReadRemoteObject();
    if (token == nullptr) {
        WLOGFD("get token nullptr.");
    }
    return static_cast<WSError>(reply.ReadInt32());
}

WSError SceneSessionManagerProxy::GetFocusSessionElement(AppExecFwk::ElementName& element, DisplayId displayId)
{
    WLOGFD("run SceneSessionManagerProxy::GetFocusSessionElement");
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        WLOGFE("Write interfaceToken failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    if (!data.WriteUint64(displayId)) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "write displayId failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        WLOGFE("remote is null");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    if (remote->SendRequest(static_cast<uint32_t>(SceneSessionManagerMessage::TRANS_ID_GET_FOCUS_SESSION_ELEMENT),
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

WMError SceneSessionManagerProxy::CheckWindowId(int32_t windowId, int32_t& pid)
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
    int32_t ret = remote->SendRequest(static_cast<uint32_t>(SceneSessionManagerMessage::TRANS_ID_CHECK_WINDOW_ID),
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

WSError SceneSessionManagerProxy::GetSessionDumpInfo(const std::vector<std::string>& params, std::string& info)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        WLOGFE("WriteInterfaceToken failed");
        return WSError::WS_ERROR_INVALID_PARAM;
    }
    if (!data.WriteStringVector(params)) {
        WLOGFE("Write params failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        WLOGFE("remote is null");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    if (remote->SendRequest(static_cast<uint32_t>(SceneSessionManagerMessage::TRANS_ID_GET_SESSION_DUMP_INFO),
        data, reply, option) != ERR_NONE) {
        WLOGFE("SendRequest failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }

    auto infoSize = reply.ReadUint32();
    if (infoSize != 0) {
        const char* infoPtr = nullptr;
        infoPtr = reinterpret_cast<const char*>(reply.ReadRawData(infoSize));
        info = (infoPtr) ? std::string(infoPtr, infoSize) : "";
    }
    WLOGFD("GetSessionDumpInfo, infoSize: %{public}d", infoSize);
    return static_cast<WSError>(reply.ReadInt32());
}

WSError SceneSessionManagerProxy::GetSessionSnapshot(const std::string& deviceId, int32_t persistentId,
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
    if (remote->SendRequest(static_cast<uint32_t>(SceneSessionManagerMessage::TRANS_ID_GET_SESSION_SNAPSHOT),
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

WSError SceneSessionManagerProxy::GetUIContentRemoteObj(int32_t persistentId, sptr<IRemoteObject>& uiContentRemoteObj)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "WriteInterfaceToken failed");
        return WSError::WS_ERROR_INVALID_PARAM;
    }
    if (!data.WriteInt32(persistentId)) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "Write persistentId failed");
        return WSError::WS_ERROR_INVALID_PARAM;
    }

    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "remote is null");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    if (remote->SendRequest(static_cast<uint32_t>(SceneSessionManagerMessage::TRANS_ID_GET_UI_CONTENT_REMOTE_OBJ),
        data, reply, option) != ERR_NONE) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "SendRequest failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    sptr<IRemoteObject> remoteObj = reply.ReadRemoteObject();
    if (remoteObj == nullptr) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "ReadRemoteObject failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    uiContentRemoteObj = remoteObj;
    return static_cast<WSError>(reply.ReadUint32());
}

WMError SceneSessionManagerProxy::GetRootUIContentRemoteObj(DisplayId displayId,
    sptr<IRemoteObject>& uiContentRemoteObj)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "write interfaceToken failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    if (!data.WriteUint64(displayId)) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "write displayId failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "remote is null");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    auto reqErrCode = remote->SendRequest(static_cast<uint32_t>(
        SceneSessionManagerMessage::TRANS_ID_GET_ROOT_UI_CONTENT_REMOTE_OBJ), data, reply, option);
    if (reqErrCode != ERR_NONE) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "send request failed, errCode: %{public}d", reqErrCode);
        return WMError::WM_ERROR_IPC_FAILED;
    }
    sptr<IRemoteObject> remoteObj = reply.ReadRemoteObject();
    if (remoteObj == nullptr) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "read remote object failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    uiContentRemoteObj = remoteObj;
    int32_t errCode = 0;
    if (!reply.ReadInt32(errCode)) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "read errcode failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    return static_cast<WMError>(errCode);
}

WSError SceneSessionManagerProxy::SetSessionContinueState(const sptr<IRemoteObject>& token,
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
    if (remote->SendRequest(static_cast<uint32_t>(SceneSessionManagerMessage::TRANS_ID_SET_SESSION_CONTINUE_STATE),
        data, reply, option) != ERR_NONE) {
        WLOGFE("SendRequest failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    return static_cast<WSError>(reply.ReadInt32());
}

void SceneSessionManagerProxy::NotifyDumpInfoResult(const std::vector<std::string>& info)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        WLOGFE("WriteInterfaceToken pfailed");
        return;
    }
    uint32_t vectorSize = static_cast<uint32_t>(info.size());
    if (!data.WriteUint32(vectorSize)) {
        WLOGFE("Write vector size failed");
        return;
    }
    for (const auto& elem : info) {
        uint32_t curSize = static_cast<uint32_t>(elem.length());
        WLOGFD("NotifyDumpInfoResult infoSize: %{public}u", curSize);
        if (!data.WriteUint32(curSize)) {
            WLOGFE("Write info size failed");
            return;
        }
        if (curSize != 0) {
            if (!data.WriteRawData(elem.c_str(), curSize)) {
                WLOGFE("Write info failed");
                return;
            }
        }
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        WLOGFE("remote is null");
        return;
    }
    if (remote->SendRequest(static_cast<uint32_t>(SceneSessionManagerMessage::TRANS_ID_NOTIFY_DUMP_INFO_RESULT),
        data, reply, option) != ERR_NONE) {
        WLOGFE("SendRequest failed");
        return;
    }
}

WSError SceneSessionManagerProxy::LockSession(int32_t sessionId)
{
    WLOGFI("run SceneSessionManagerProxy::LockSession");
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
    if (remote->SendRequest(static_cast<uint32_t>(SceneSessionManagerMessage::TRANS_ID_LOCK_SESSION),
        data, reply, option) != ERR_NONE) {
        WLOGFE("SendRequest failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    return static_cast<WSError>(reply.ReadInt32());
}

WSError SceneSessionManagerProxy::UnlockSession(int32_t sessionId)
{
    WLOGFI("run SceneSessionManagerProxy::UnlockSession");
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
    if (remote->SendRequest(static_cast<uint32_t>(SceneSessionManagerMessage::TRANS_ID_UNLOCK_SESSION),
        data, reply, option) != ERR_NONE) {
        WLOGFE("SendRequest failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    return static_cast<WSError>(reply.ReadInt32());
}

WSError SceneSessionManagerProxy::MoveSessionsToForeground(const std::vector<std::int32_t>& sessionIds,
    int32_t topSessionId)
{
    WLOGFI("run SceneSessionManagerProxy::MoveSessionsToForeground");
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
    if (remote->SendRequest(static_cast<uint32_t>(SceneSessionManagerMessage::TRANS_ID_MOVE_MISSIONS_TO_FOREGROUND),
        data, reply, option) != ERR_NONE) {
        WLOGFE("SendRequest failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    return static_cast<WSError>(reply.ReadInt32());
}

WSError SceneSessionManagerProxy::MoveSessionsToBackground(const std::vector<std::int32_t>& sessionIds,
    std::vector<int32_t>& result)
{
    WLOGFI("run SceneSessionManagerProxy::MoveSessionsToBackground");
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
    if (remote->SendRequest(static_cast<uint32_t>(SceneSessionManagerMessage::TRANS_ID_MOVE_MISSIONS_TO_BACKGROUND),
        data, reply, option) != ERR_NONE) {
        WLOGFE("SendRequest failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    reply.ReadInt32Vector(&result);
    return static_cast<WSError>(reply.ReadInt32());
}

WSError SceneSessionManagerProxy::ClearSession(int32_t persistentId)
{
    WLOGFI("run SceneSessionManagerProxy::ClearSession");
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
    if (remote->SendRequest(static_cast<uint32_t>(SceneSessionManagerMessage::TRANS_ID_CLEAR_SESSION),
        data, reply, option) != ERR_NONE) {
        WLOGFE("SendRequest failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    return static_cast<WSError>(reply.ReadInt32());
}

WSError SceneSessionManagerProxy::ClearAllSessions()
{
    WLOGFI("run SceneSessionManagerProxy::ClearSession");
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
    if (remote->SendRequest(static_cast<uint32_t>(SceneSessionManagerMessage::TRANS_ID_CLEAR_ALL_SESSIONS),
        data, reply, option) != ERR_NONE) {
        WLOGFE("SendRequest failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    return static_cast<WSError>(reply.ReadInt32());
}

WSError SceneSessionManagerProxy::RegisterIAbilityManagerCollaborator(int32_t type,
    const sptr<AAFwk::IAbilityManagerCollaborator>& impl)
{
    WLOGFI("run SceneSessionManagerProxy::RegisterIAbilityManagerCollaborator");
    if (!impl) {
        WLOGFE("impl is nullptr");
        return WSError::WS_ERROR_INVALID_PARAM;
    }
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!data.WriteInterfaceToken(GetDescriptor())) {
        WLOGFE("Write interface token failed.");
        return WSError::WS_ERROR_INVALID_PARAM;
    }
    if (!data.WriteInt32(type)) {
        WLOGFE("type write failed.");
        return WSError::WS_ERROR_INVALID_PARAM;
    }
    if (!data.WriteRemoteObject(impl->AsObject())) {
        WLOGFE("impl write failed.");
        return WSError::WS_ERROR_INVALID_PARAM;
    }

    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        WLOGFE("remote is null");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    if (remote->SendRequest(static_cast<uint32_t>(SceneSessionManagerMessage::TRANS_ID_REGISTER_COLLABORATOR),
        data, reply, option) != ERR_NONE) {
        WLOGFE("SendRequest failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    return static_cast<WSError>(reply.ReadInt32());
}

WSError SceneSessionManagerProxy::UnregisterIAbilityManagerCollaborator(int32_t type)
{
    WLOGFI("run SceneSessionManagerProxy::UnregisterIAbilityManagerCollaborator");
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!data.WriteInterfaceToken(GetDescriptor())) {
        WLOGFE("Write interface token failed.");
        return WSError::WS_ERROR_INVALID_PARAM;
    }
    if (!data.WriteInt32(type)) {
        WLOGFE("type write failed.");
        return WSError::WS_ERROR_INVALID_PARAM;
    }

    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        WLOGFE("remote is null");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    if (remote->SendRequest(static_cast<uint32_t>(SceneSessionManagerMessage::TRANS_ID_UNREGISTER_COLLABORATOR),
        data, reply, option) != ERR_NONE) {
        WLOGFE("SendRequest failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    return static_cast<WSError>(reply.ReadInt32());
}

WSError SceneSessionManagerProxy::NotifyWindowExtensionVisibilityChange(int32_t pid, int32_t uid, bool visible)
{
    WLOGFI("run SceneSessionManagerProxy::NotifyWindowExtensionVisibilityChange");
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);

    if (!data.WriteInterfaceToken(GetDescriptor())) {
        WLOGFE("Write interface token failed.");
        return WSError::WS_ERROR_INVALID_PARAM;
    }

    if (!data.WriteInt32(pid)) {
        WLOGFE("pid write failed.");
        return WSError::WS_ERROR_INVALID_PARAM;
    }

    if (!data.WriteInt32(uid)) {
        WLOGFE("uid write failed.");
        return WSError::WS_ERROR_INVALID_PARAM;
    }
    if (!data.WriteBool(visible)) {
        WLOGFE("pid write failed.");
        return WSError::WS_ERROR_INVALID_PARAM;
    }

    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        WLOGFE("remote is null");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    if (remote->SendRequest(static_cast<uint32_t>(
        SceneSessionManagerMessage::TRANS_ID_NOTIFY_WINDOW_EXTENSION_VISIBILITY_CHANGE),
        data, reply, option) != ERR_NONE) {
        WLOGFE("SendRequest failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    return static_cast<WSError>(reply.ReadInt32());
}

WMError SceneSessionManagerProxy::GetTopWindowId(uint32_t mainWinId, uint32_t& topWinId)
{
    WLOGFD("[GetTopWin] mainId: %{public}d", mainWinId);
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        WLOGFE("Write interface token failed.");
        return WMError::WM_ERROR_IPC_FAILED;
    }

    if (!data.WriteUint32(mainWinId)) {
        WLOGFE("Write mainWinId failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }

    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        WLOGFE("remote is null");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    if (remote->SendRequest(static_cast<uint32_t>(SceneSessionManagerMessage::TRANS_ID_GET_TOP_WINDOW_ID),
        data, reply, option) != ERR_NONE) {
        WLOGFE("SendRequest failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }

    topWinId = reply.ReadUint32();
    int32_t ret = reply.ReadInt32();
    return static_cast<WMError>(ret);
}

WMError SceneSessionManagerProxy::NotifyWatchGestureConsumeResult(int32_t keyCode, bool isConsumed)
{
    TLOGD(WmsLogTag::WMS_EVENT, "isConsumed:%{public}d", isConsumed);
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::WMS_EVENT, "Write interface token failed.");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    if (!data.WriteInt32(keyCode)) {
        TLOGE(WmsLogTag::WMS_EVENT, "Write keyCode failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    if (!data.WriteBool(isConsumed)) {
        TLOGE(WmsLogTag::WMS_EVENT, "Write isConsumed failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }

    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::WMS_EVENT, "remote is null");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    if (remote->SendRequest(static_cast<uint32_t>(SceneSessionManagerMessage::TRANS_ID_WATCH_GESTURE_CONSUME_RESULT),
        data, reply, option) != ERR_NONE) {
        TLOGE(WmsLogTag::WMS_EVENT, "SendRequest failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    int32_t ret = 0;
    if (!reply.ReadInt32(ret)) {
        return WMError::WM_ERROR_IPC_FAILED;
    }
    return static_cast<WMError>(ret);
}

WMError SceneSessionManagerProxy::NotifyWatchFocusActiveChange(bool isActive)
{
    TLOGD(WmsLogTag::WMS_EVENT, "isActive:%{public}d", isActive);
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::WMS_EVENT, "Write interface token failed.");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    if (!data.WriteBool(isActive)) {
        TLOGE(WmsLogTag::WMS_EVENT, "Write isActive failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }

    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::WMS_EVENT, "remote is null");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    if (remote->SendRequest(static_cast<uint32_t>(SceneSessionManagerMessage::TRANS_ID_WATCH_FOCUS_ACTIVE_CHANGE),
        data, reply, option) != ERR_NONE) {
        TLOGE(WmsLogTag::WMS_EVENT, "SendRequest failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    int32_t ret = 0;
    if (!reply.ReadInt32(ret)) {
        return WMError::WM_ERROR_IPC_FAILED;
    }
    return static_cast<WMError>(ret);
}

WMError SceneSessionManagerProxy::GetParentMainWindowId(int32_t windowId, int32_t& mainWindowId)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::WMS_SUB, "WriteInterfaceToken failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    if (!data.WriteInt32(windowId)) {
        TLOGE(WmsLogTag::WMS_SUB, "Write windowId failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::WMS_SUB, "Remote is null");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    if (remote->SendRequest(static_cast<uint32_t>(SceneSessionManagerMessage::TRANS_ID_GET_PARENT_MAIN_WINDOW_ID),
        data, reply, option) != ERR_NONE) {
        TLOGE(WmsLogTag::WMS_SUB, "SendRequest failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    int32_t replyMainWindowId = INVALID_SESSION_ID;
    if (!reply.ReadInt32(replyMainWindowId)) {
        return WMError::WM_ERROR_IPC_FAILED;
    }
    int32_t ret = 0;
    if (!reply.ReadInt32(ret)) {
        return WMError::WM_ERROR_IPC_FAILED;
    }
    mainWindowId = replyMainWindowId;
    return static_cast<WMError>(ret);
}

WMError SceneSessionManagerProxy::ListWindowInfo(const WindowInfoOption& windowInfoOption,
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
        SceneSessionManagerMessage::TRANS_ID_LIST_WINDOW_INFO), data, reply, option) != ERR_NONE) {
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

WMError SceneSessionManagerProxy::GetAllWindowLayoutInfo(DisplayId displayId,
    std::vector<sptr<WindowLayoutInfo>>& infos)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "Write interfaceToken failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    if (!data.WriteUint64(displayId)) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "write displayId failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "remote is null");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    if (remote->SendRequest(static_cast<uint32_t>(
        SceneSessionManagerMessage::TRANS_ID_GET_WINDOW_LAYOUT_INFO), data, reply, option) != ERR_NONE) {
        return WMError::WM_ERROR_IPC_FAILED;
    }
    if (!MarshallingHelper::UnmarshallingVectorParcelableObj<WindowLayoutInfo>(reply, infos)) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "read window layout info failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    int32_t errCode = 0;
    if (!reply.ReadInt32(errCode)) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "read errcode failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    return static_cast<WMError>(errCode);
}

WMError SceneSessionManagerProxy::GetAllMainWindowInfo(std::vector<sptr<MainWindowInfo>>& infos)
{
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
    if (remote->SendRequest(static_cast<uint32_t>(
        SceneSessionManagerMessage::TRANS_ID_GET_ALL_MAIN_WINDOW_INFO), data, reply, option) != ERR_NONE) {
        return WMError::WM_ERROR_IPC_FAILED;
    }
    if (!MarshallingHelper::UnmarshallingVectorParcelableObj<MainWindowInfo>(reply, infos)) {
        TLOGE(WmsLogTag::WMS_LIFE, "read main window info failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    int32_t errCode = 0;
    if (!reply.ReadInt32(errCode)) {
        TLOGE(WmsLogTag::WMS_LIFE, "read errcode failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    return static_cast<WMError>(errCode);
}
 
WMError SceneSessionManagerProxy::GetMainWindowSnapshot(const std::vector<int32_t>& windowIds,
    const WindowSnapshotConfiguration& config, const sptr<IRemoteObject>& callback)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::WMS_LIFE, "Write interfaceToken failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    if (!data.WriteInt32Vector(windowIds)) {
        TLOGE(WmsLogTag::WMS_LIFE, "Write windowIds failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    if (!data.WriteBool(config.useCache)) {
        TLOGE(WmsLogTag::WMS_LIFE, "Write useCache failed!");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    if (!data.WriteRemoteObject(callback)) {
        TLOGE(WmsLogTag::WMS_LIFE, "Write callback failed!");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::WMS_LIFE, "remote is null");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    if (remote->SendRequest(static_cast<uint32_t>(
        SceneSessionManagerMessage::TRANS_ID_GET_MAIN_WINDOW_SNAPSHOT), data, reply, option) != ERR_NONE) {
        return WMError::WM_ERROR_IPC_FAILED;
    }
    int32_t errCode = 0;
    if (!reply.ReadInt32(errCode)) {
        TLOGE(WmsLogTag::WMS_LIFE, "read errcode failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    return static_cast<WMError>(errCode);
}

WMError SceneSessionManagerProxy::GetGlobalWindowMode(DisplayId displayId, GlobalWindowMode& globalWinMode)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "Write interfaceToken failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    if (!data.WriteUint64(displayId)) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "write displayId failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "remote is null");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    auto reqErrCode = remote->SendRequest(
        static_cast<uint32_t>(SceneSessionManagerMessage::TRANS_ID_GET_GLOBAL_WINDOW_MODE), data, reply, option);
    if (reqErrCode != ERR_NONE) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "send request failed, errCode: %{public}d", reqErrCode);
        return WMError::WM_ERROR_IPC_FAILED;
    }
    int32_t errCode = 0;
    if (!reply.ReadInt32(errCode)) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "read errcode failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    uint32_t mode = 0;
    if (!reply.ReadUint32(mode)) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "read global window mode failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    globalWinMode = static_cast<GlobalWindowMode>(mode);
    return static_cast<WMError>(errCode);
}

WMError SceneSessionManagerProxy::GetTopNavDestinationName(int32_t windowId, std::string& topNavDestName)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "write interfaceToken failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    if (!data.WriteInt32(windowId)) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "write windowId failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "remote is null");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    auto reqErrCode = remote->SendRequest(
        static_cast<uint32_t>(SceneSessionManagerMessage::TRANS_ID_GET_TOP_NAV_DEST_NAME), data, reply, option);
    if (reqErrCode != ERR_NONE) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "send request failed, errCode: %{public}d", reqErrCode);
        return WMError::WM_ERROR_IPC_FAILED;
    }
    const char* namePtr = nullptr;
    auto size = reply.ReadUint32();
    if (size != 0) {
        namePtr = reinterpret_cast<const char*>(reply.ReadRawData(size));
    }
    topNavDestName = (namePtr != nullptr) ? std::string(namePtr, size) : "";
    int32_t errCode = 0;
    if (!reply.ReadInt32(errCode)) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "read errcode failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    return static_cast<WMError>(errCode);
}

WMError SceneSessionManagerProxy::SetWatermarkImageForApp(const std::shared_ptr<Media::PixelMap>& pixelMap,
    std::string& watermarkName)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "write interfaceToken failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    if (!data.WriteParcelable(pixelMap.get())) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "write pixelMap failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "remote is null");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    auto reqErrCode = remote->SendRequest(
        static_cast<uint32_t>(SceneSessionManagerMessage::TRANS_ID_SET_APP_WATERMARK_IMAGE), data, reply, option);
    if (reqErrCode != ERR_NONE) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "send request failed, errCode: %{public}d", reqErrCode);
        return WMError::WM_ERROR_IPC_FAILED;
    }
    const char* namePtr = nullptr;
    auto size = reply.ReadUint32();
    if (size != 0) {
        auto nameData = reply.ReadRawData(size);
        if (!nameData) {
            TLOGE(WmsLogTag::WMS_ATTRIBUTE, "read watermark name failed");
            return WMError::WM_ERROR_IPC_FAILED;
        }
        namePtr = reinterpret_cast<const char*>(nameData);
    }
    watermarkName = (namePtr != nullptr) ? std::string(namePtr, size) : "";
    int32_t errCode = 0;
    if (!reply.ReadInt32(errCode)) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "read errcode failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    return static_cast<WMError>(errCode);
}

WMError SceneSessionManagerProxy::RecoverWatermarkImageForApp(const std::string& watermarkName)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "write interfaceToken failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    if (!data.WriteString(watermarkName)) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "write watermarkName failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "remote is null");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    auto reqErrCode = remote->SendRequest(
        static_cast<uint32_t>(SceneSessionManagerMessage::TRANS_ID_RECOVER_APP_WATERMARK_IMAGE), data, reply, option);
    if (reqErrCode != ERR_NONE) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "send request failed, errCode: %{public}d", reqErrCode);
        return WMError::WM_ERROR_IPC_FAILED;
    }
    int32_t errCode = 0;
    if (!reply.ReadInt32(errCode)) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "read errcode failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    return static_cast<WMError>(errCode);
}

WMError SceneSessionManagerProxy::GetVisibilityWindowInfo(std::vector<sptr<WindowVisibilityInfo>>& infos)
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
        WLOGFE("remote is null");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    if (remote->SendRequest(static_cast<uint32_t>(
        SceneSessionManagerMessage::TRANS_ID_GET_VISIBILITY_WINDOW_INFO_ID), data, reply, option) != ERR_NONE) {
        return WMError::WM_ERROR_IPC_FAILED;
    }
    if (!MarshallingHelper::UnmarshallingVectorParcelableObj<WindowVisibilityInfo>(reply, infos)) {
        WLOGFE("read visibility window infos failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    return static_cast<WMError>(reply.ReadInt32());
}

WSError SceneSessionManagerProxy::ShiftAppWindowFocus(int32_t sourcePersistentId, int32_t targetPersistentId)
{
    WLOGFD("run SceneSessionManagerProxy::ShiftAppWindowFocus");
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        WLOGFE("Write interface token failed.");
        return WSError::WS_ERROR_INVALID_PARAM;
    }

    if (!data.WriteUint32(sourcePersistentId)) {
        WLOGFE("Write sourcePersistentId failed");
        return WSError::WS_ERROR_INVALID_PARAM;
    }

    if (!data.WriteUint32(targetPersistentId)) {
        WLOGFE("Write targetPersistentId failed");
        return WSError::WS_ERROR_INVALID_PARAM;
    }

    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        WLOGFE("remote is null");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    if (remote->SendRequest(static_cast<uint32_t>(
        SceneSessionManagerMessage::TRANS_ID_SHIFT_APP_WINDOW_FOCUS),
        data, reply, option) != ERR_NONE) {
        WLOGFE("SendRequest failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    return static_cast<WSError>(reply.ReadInt32());
}

WSError SceneSessionManagerProxy::SetSpecificWindowZIndex(WindowType windowType, int32_t zIndex)
{
    TLOGI(WmsLogTag::WMS_FOCUS, "run SetSpecificSystemWindowZIndex");
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::WMS_FOCUS, "Write interface token failed.");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    if (!data.WriteUint32(static_cast<uint32_t>(windowType))) {
        TLOGE(WmsLogTag::WMS_FOCUS, "Write windowType failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    if (!data.WriteInt32(zIndex)) {
        TLOGE(WmsLogTag::WMS_FOCUS, "Write zIndex failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::WMS_FOCUS, "Remote is null");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    if (remote->SendRequest(static_cast<uint32_t>(
        SceneSessionManagerMessage::TRANS_ID_SET_SPECIFIC_WINDOW_ZINDEX),
        data, reply, option) != ERR_NONE) {
        TLOGE(WmsLogTag::WMS_FOCUS, "SendRequest failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    int32_t result = 0;
    if (!reply.ReadInt32(result)) {
        TLOGE(WmsLogTag::WMS_FOCUS, "Failed to read result");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    return static_cast<WSError>(result);
}

WSError SceneSessionManagerProxy::ResetSpecificWindowZIndex(int32_t pid)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::WMS_FOCUS, "Write interfaceToken failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    if (!data.WriteInt32(pid)) {
        TLOGE(WmsLogTag::WMS_FOCUS, "Write pid failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::WMS_FOCUS, "Remote is null");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    if (remote->SendRequest(static_cast<uint32_t>(
        SceneSessionManagerMessage::TRANS_ID_RESET_SPECIFIC_WINDOW_ZINDEX),
        data, reply, option) != ERR_NONE) {
        TLOGE(WmsLogTag::WMS_FOCUS, "SendRequest failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    int32_t result = 0;
    if (!reply.ReadInt32(result)) {
        TLOGE(WmsLogTag::WMS_FOCUS, "Failed to read result");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    return static_cast<WSError>(result);
}

void SceneSessionManagerProxy::UpdateModalExtensionRect(const sptr<IRemoteObject>& token, Rect rect)
{
    MessageOption option(MessageOption::TF_SYNC);
    MessageParcel data;
    MessageParcel reply;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::WMS_UIEXT, "Write interface token failed.");
        return;
    }
    if (!data.WriteRemoteObject(token)) {
        TLOGE(WmsLogTag::WMS_UIEXT, "Write token failed");
        return;
    }
    if (!data.WriteInt32(rect.posX_)) {
        TLOGE(WmsLogTag::WMS_UIEXT, "Write posX_ failed");
        return;
    }
    if (!data.WriteInt32(rect.posY_)) {
        TLOGE(WmsLogTag::WMS_UIEXT, "Write posY_ failed");
        return;
    }
    if (!data.WriteInt32(rect.width_)) {
        TLOGE(WmsLogTag::WMS_UIEXT, "Write width_ failed");
        return;
    }
    if (!data.WriteInt32(rect.height_)) {
        TLOGE(WmsLogTag::WMS_UIEXT, "Write height_ failed");
        return;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::WMS_UIEXT, "remote is null");
        return;
    }
    if (remote->SendRequest(static_cast<uint32_t>(
                            SceneSessionManagerMessage::TRANS_ID_UPDATE_MODALEXTENSION_RECT_TO_SCB),
                            data, reply, option) != ERR_NONE) {
        TLOGE(WmsLogTag::WMS_UIEXT, "SendRequest failed");
    }
}

void SceneSessionManagerProxy::ProcessModalExtensionPointDown(const sptr<IRemoteObject>& token, int32_t posX,
    int32_t posY)
{
    MessageOption option(MessageOption::TF_SYNC);
    MessageParcel data;
    MessageParcel reply;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::WMS_UIEXT, "Write interface token failed.");
        return;
    }
    if (!data.WriteRemoteObject(token)) {
        TLOGE(WmsLogTag::WMS_UIEXT, "Write token failed");
        return;
    }
    if (!data.WriteInt32(posX)) {
        TLOGE(WmsLogTag::WMS_UIEXT, "Write posX failed");
        return;
    }
    if (!data.WriteInt32(posY)) {
        TLOGE(WmsLogTag::WMS_UIEXT, "Write posY failed");
        return;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::WMS_UIEXT, "remote is null");
        return;
    }
    if (remote->SendRequest(static_cast<uint32_t>(
                            SceneSessionManagerMessage::TRANS_ID_PROCESS_MODALEXTENSION_POINTDOWN_TO_SCB),
                            data, reply, option) != ERR_NONE) {
        TLOGE(WmsLogTag::WMS_UIEXT, "SendRequest failed");
    }
}

void SceneSessionManagerProxy::AddExtensionWindowStageToSCB(const sptr<ISessionStage>& sessionStage,
    const sptr<IRemoteObject>& token, uint64_t surfaceNodeId, int64_t startModalExtensionTimeStamp,
    bool isConstrainedModal)
{
    if (sessionStage == nullptr || token == nullptr) {
        TLOGE(WmsLogTag::WMS_UIEXT, "input is nullptr");
        return;
    }
    MessageOption option(MessageOption::TF_SYNC);
    MessageParcel data;
    MessageParcel reply;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::WMS_UIEXT, "Write InterfaceToken failed!");
        return;
    }
    if (!data.WriteRemoteObject(sessionStage->AsObject())) {
        TLOGE(WmsLogTag::WMS_UIEXT, "Write ISessionStage failed!");
        return;
    }
    if (!data.WriteRemoteObject(token)) {
        TLOGE(WmsLogTag::WMS_UIEXT, "Write token failed");
        return;
    }
    if (!data.WriteUint64(static_cast<uint64_t>(surfaceNodeId))) {
        TLOGE(WmsLogTag::WMS_UIEXT, "Write surfaceNodeId failed");
        return;
    }
    if (!data.WriteInt64(startModalExtensionTimeStamp)) {
        TLOGE(WmsLogTag::WMS_UIEXT, "Write startModalExtensionTimeStamp failed");
        return;
    }
    if (!data.WriteBool(isConstrainedModal)) {
        TLOGE(WmsLogTag::WMS_UIEXT, "Write isConstrainedModal failed");
        return;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::WMS_UIEXT, "remote is null");
        return;
    }
    if (remote->SendRequest(static_cast<uint32_t>(
                            SceneSessionManagerMessage::TRANS_ID_ADD_EXTENSION_WINDOW_STAGE_TO_SCB),
                            data, reply, option) != ERR_NONE) {
        TLOGE(WmsLogTag::WMS_UIEXT, "SendRequest failed");
    }
}

void SceneSessionManagerProxy::RemoveExtensionWindowStageFromSCB(const sptr<ISessionStage>& sessionStage,
    const sptr<IRemoteObject>& token, bool isConstrainedModal)
{
    if (sessionStage == nullptr || token == nullptr) {
        TLOGE(WmsLogTag::WMS_UIEXT, "input is nullptr");
        return;
    }
    MessageOption option(MessageOption::TF_SYNC);
    MessageParcel data;
    MessageParcel reply;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::WMS_UIEXT, "Write InterfaceToken failed!");
        return;
    }
    if (!data.WriteRemoteObject(sessionStage->AsObject())) {
        TLOGE(WmsLogTag::WMS_UIEXT, "Write ISessionStage failed!");
        return;
    }
    if (!data.WriteRemoteObject(token)) {
        TLOGE(WmsLogTag::WMS_UIEXT, "Write token failed");
        return;
    }
    if (!data.WriteBool(isConstrainedModal)) {
        TLOGE(WmsLogTag::WMS_UIEXT, "Write isConstrainedModal failed");
        return;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::WMS_UIEXT, "remote is null");
        return;
    }
    if (remote->SendRequest(static_cast<uint32_t>(
                            SceneSessionManagerMessage::TRANS_ID_REMOVE_EXTENSION_WINDOW_STAGE_FROM_SCB),
                            data, reply, option) != ERR_NONE) {
        TLOGE(WmsLogTag::WMS_UIEXT, "SendRequest failed");
    }
}

WSError SceneSessionManagerProxy::AddOrRemoveSecureSession(int32_t persistentId, bool shouldHide)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::WMS_UIEXT, "WriteInterfaceToken failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    if (!data.WriteInt32(persistentId)) {
        TLOGE(WmsLogTag::WMS_UIEXT, "Write persistentId failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    if (!data.WriteBool(shouldHide)) {
        TLOGE(WmsLogTag::WMS_UIEXT, "Write shouldHide failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::WMS_UIEXT, "remote is null");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    if (remote->SendRequest(static_cast<uint32_t>(
                            SceneSessionManagerMessage::TRANS_ID_ADD_OR_REMOVE_SECURE_SESSION),
                            data, reply, option) != ERR_NONE) {
        TLOGE(WmsLogTag::WMS_UIEXT, "SendRequest failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    return static_cast<WSError>(reply.ReadInt32());
}

WSError SceneSessionManagerProxy::UpdateExtWindowFlags(const sptr<IRemoteObject>& token, uint32_t extWindowFlags,
    uint32_t extWindowActions)
{
    TLOGD(WmsLogTag::WMS_UIEXT, "run SceneSessionManagerProxy::UpdateExtWindowFlags");
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::WMS_UIEXT, "Write interface token failed.");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    if (!data.WriteRemoteObject(token)) {
        TLOGE(WmsLogTag::WMS_UIEXT, "Write token failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    if (!data.WriteUint32(extWindowFlags)) {
        TLOGE(WmsLogTag::WMS_UIEXT, "Write extWindowFlags failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    if (!data.WriteUint32(extWindowActions)) {
        TLOGE(WmsLogTag::WMS_UIEXT, "Write extWindowActions failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::WMS_UIEXT, "remote is null");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    if (remote->SendRequest(
        static_cast<uint32_t>(SceneSessionManagerMessage::TRANS_ID_UPDATE_EXTENSION_WINDOW_FLAGS),
        data, reply, option) != ERR_NONE) {
        TLOGE(WmsLogTag::WMS_UIEXT, "SendRequest AddExtensionSessionInfo failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    return static_cast<WSError>(reply.ReadInt32());
}

WSError SceneSessionManagerProxy::GetHostWindowRect(int32_t hostWindowId, Rect& rect)
{
    TLOGD(WmsLogTag::WMS_UIEXT, "run SceneSessionManagerProxy::GetHostWindowRect");
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::WMS_UIEXT, "Write interface token failed.");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    if (!data.WriteInt32(hostWindowId)) {
        TLOGE(WmsLogTag::WMS_UIEXT, "Write hostWindowId failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::WMS_UIEXT, "remote is null");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    if (remote->SendRequest(
        static_cast<uint32_t>(SceneSessionManagerMessage::TRANS_ID_GET_HOST_WINDOW_RECT),
        data, reply, option) != ERR_NONE) {
        TLOGE(WmsLogTag::WMS_UIEXT, "SendRequest GetHostWindowRect failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    auto posX = reply.ReadInt32();
    auto posY = reply.ReadInt32();
    auto width = reply.ReadUint32();
    auto height = reply.ReadUint32();
    rect = {posX, posY, width, height};
    return static_cast<WSError>(reply.ReadInt32());
}

WSError SceneSessionManagerProxy::GetHostGlobalScaledRect(int32_t hostWindowId, Rect& globalScaledRect)
{
    TLOGD(WmsLogTag::WMS_UIEXT, "in");
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::WMS_UIEXT, "Write interface token failed.");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    if (!data.WriteInt32(hostWindowId)) {
        TLOGE(WmsLogTag::WMS_UIEXT, "Write hostWindowId failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::WMS_UIEXT, "remote is null");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    auto errorCode = remote->SendRequest(
        static_cast<uint32_t>(SceneSessionManagerMessage::TRANS_ID_GET_HOST_GLOBAL_SCALE_RECT), data, reply, option);
    if (errorCode != ERR_NONE) {
        TLOGE(WmsLogTag::WMS_UIEXT, "SendRequest failed code: %{public}d", errorCode);
        return WSError::WS_ERROR_IPC_FAILED;
    }
    auto posX = reply.ReadInt32();
    auto posY = reply.ReadInt32();
    auto width = reply.ReadUint32();
    auto height = reply.ReadUint32();
    globalScaledRect = {posX, posY, width, height};
    return static_cast<WSError>(reply.ReadInt32());
}

WMError SceneSessionManagerProxy::ConvertToRelativeCoordinateExtended(
    const Rect& rect, Rect& newRect, DisplayId& newDisplayId)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "Write interface token failed.");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    if (!data.WriteInt32(rect.posX_) || !data.WriteInt32(rect.posY_) ||
        !data.WriteUint32(rect.width_) || !data.WriteUint32(rect.height_) ||
        !data.WriteUint64(newDisplayId)) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "Failed to write rect");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::WMS_MULTI_WINDOW, "remote is nullptr");
        return WMError::WM_ERROR_NULLPTR;
    }
    auto sendRet = remote->SendRequest(
        static_cast<uint32_t>(SceneSessionManagerMessage::TRANS_ID_GLOBAL_COORDINATE_TO_RELATIVE_COORDINATE),
        data, reply, option);
    if (sendRet != ERR_NONE) {
        TLOGE(WmsLogTag::WMS_MULTI_WINDOW,
            "SendRequest ConvertToRelativeCoordinateExtended failed, code: %{public}d", sendRet);
        return WMError::WM_ERROR_IPC_FAILED;
    }
    int32_t posX = 0;
    int32_t posY = 0;
    uint32_t width = 0;
    uint32_t height = 0;
    if (!reply.ReadInt32(posX) || !reply.ReadInt32(posY) || !reply.ReadUint32(width) || !reply.ReadUint32(height)) {
        TLOGE(WmsLogTag::WMS_MULTI_WINDOW, "Failed to read rect");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    newRect = {posX, posY, width, height};
    uint64_t displayId = 0;
    if (!reply.ReadUint64(displayId)) {
        TLOGE(WmsLogTag::WMS_MULTI_WINDOW, "Failed to read displayId");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    newDisplayId = displayId;
    int32_t result = 0;
    if (!reply.ReadInt32(result)) {
        TLOGE(WmsLogTag::WMS_MULTI_WINDOW, "Failed to read result");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    return static_cast<WMError>(result);
}

WSError SceneSessionManagerProxy::GetFreeMultiWindowEnableState(bool& enable)
{
    TLOGD(WmsLogTag::WMS_MULTI_WINDOW, "in");
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::WMS_MULTI_WINDOW, "Write interface token failed.");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::WMS_MULTI_WINDOW, "remote is nullptr");
        return WSError::WS_ERROR_NULLPTR;
    }
    if (remote->SendRequest(
        static_cast<uint32_t>(SceneSessionManagerMessage::TRANS_ID_GET_FREE_MULTI_WINDOW_ENABLE_STATE),
        data, reply, option) != ERR_NONE) {
        TLOGE(WmsLogTag::WMS_MULTI_WINDOW, "SendRequest GetFreeMultiWindowEnableState failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    auto isEnable = reply.ReadBool();
    enable = isEnable;
    return static_cast<WSError>(reply.ReadInt32());
}

WMError SceneSessionManagerProxy::GetCallingWindowWindowStatus(uint32_t callingWindowId, WindowStatus& windowStatus)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::WMS_KEYBOARD, "WriteInterfaceToken failed, callingWindowId: %{public}u", callingWindowId);
        return WMError::WM_ERROR_IPC_FAILED;
    }
    if (!data.WriteUint32(callingWindowId)) {
        TLOGE(WmsLogTag::WMS_KEYBOARD, "Write callingWindowId failed, callingWindowId: %{public}u", callingWindowId);
        return WMError::WM_ERROR_IPC_FAILED;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::WMS_KEYBOARD, "remote is null, callingWindowId: %{public}u", callingWindowId);
        return WMError::WM_ERROR_IPC_FAILED;
    }
    int sendCode = remote->SendRequest(
        static_cast<uint32_t>(SceneSessionManagerMessage::TRANS_ID_GET_WINDOW_STATUS),
        data, reply, option);
    if (sendCode != ERR_NONE) {
        TLOGE(WmsLogTag::WMS_KEYBOARD, "SendRequest failed, code: %{public}d, callingWindowId: %{public}u",
            sendCode, callingWindowId);
        return WMError::WM_ERROR_IPC_FAILED;
    }
    auto ret = static_cast<WMError>(reply.ReadInt32());
    if (ret == WMError::WM_OK) {
        windowStatus = static_cast<WindowStatus>(reply.ReadUint32());
    }
    return ret;
}

WMError SceneSessionManagerProxy::GetCallingWindowRect(uint32_t callingWindowId, Rect& rect)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::WMS_KEYBOARD, "WriteInterfaceToken failed, callingWindowId: %{public}u", callingWindowId);
        return WMError::WM_ERROR_IPC_FAILED;
    }
    if (!data.WriteUint32(callingWindowId)) {
        TLOGE(WmsLogTag::WMS_KEYBOARD, "Write callingWindowId failed, callingWindowId: %{public}u", callingWindowId);
        return WMError::WM_ERROR_IPC_FAILED;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::WMS_KEYBOARD, "remote is null, callingWindowId: %{public}u", callingWindowId);
        return WMError::WM_ERROR_IPC_FAILED;
    }
    int sendCode = remote->SendRequest(
        static_cast<uint32_t>(SceneSessionManagerMessage::TRANS_ID_GET_WINDOW_RECT),
        data, reply, option);
    if (sendCode != ERR_NONE) {
        TLOGE(WmsLogTag::WMS_KEYBOARD, "SendRequest failed, code: %{public}d, callingWindowId: %{public}u",
            sendCode, callingWindowId);
        return WMError::WM_ERROR_IPC_FAILED;
    }
    auto ret = static_cast<WMError>(reply.ReadInt32());
    if (ret == WMError::WM_OK) {
        rect.posX_ = reply.ReadInt32();
        rect.posY_ = reply.ReadInt32();
        rect.width_ = reply.ReadUint32();
        rect.height_ = reply.ReadUint32();
    }
    return ret;
}

WMError SceneSessionManagerProxy::GetWindowModeType(WindowModeType& windowModeType)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGD(WmsLogTag::DEFAULT, "Write interfaceToken failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::DEFAULT, "Remote is null");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    if (remote->SendRequest(static_cast<uint32_t>(
        SceneSessionManagerMessage::TRANS_ID_GET_WINDOW_MODE_TYPE), data, reply, option) != ERR_NONE) {
        return WMError::WM_ERROR_IPC_FAILED;
    }
    windowModeType = static_cast<WindowModeType>(reply.ReadUint32());
    return static_cast<WMError>(reply.ReadInt32());
}

WMError SceneSessionManagerProxy::MinimizeAllAppWindows(DisplayId displayId, int32_t excludeWindowId)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::WMS_LIFE, "Write interfaceToken failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    if (!data.WriteUint64(displayId)) {
        TLOGE(WmsLogTag::WMS_LIFE, "Write displayId failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    if (!data.WriteInt32(excludeWindowId)) {
        TLOGE(WmsLogTag::WMS_LIFE, "Write excludeWindowId failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::WMS_LIFE, "Remote is null");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    if (remote->SendRequest(static_cast<uint32_t>(
        SceneSessionManagerMessage::TRANS_ID_MINIMIZE_ALL_WINDOW), data, reply, option) != ERR_NONE) {
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

WMError SceneSessionManagerProxy::ToggleShownStateForAllAppWindows()
{
    if (!Permission::IsSystemCallingOrStartByHdcd(true)) {
        TLOGE(WmsLogTag::WMS_LIFE, "Not system app, no right");
        return WMError::WM_ERROR_NOT_SYSTEM_APP;
    }
    TLOGE(WmsLogTag::WMS_LIFE, "Not support call toggleShownState");
    return WMError::WM_ERROR_DEVICE_NOT_SUPPORT;
}

WMError SceneSessionManagerProxy::GetWindowStyleType(WindowStyleType& windowStyleType)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::WMS_LIFE, "GetwindowStyleType Write interfaceToken failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::WMS_LIFE, "remote is null");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    if (remote->SendRequest(static_cast<uint32_t>(
        SceneSessionManagerMessage::TRANS_ID_GET_WINDOW_STYLE_TYPE), data, reply, option) != ERR_NONE) {
        return WMError::WM_ERROR_IPC_FAILED;
    }
    windowStyleType = static_cast<WindowStyleType>(reply.ReadUint32());
    return static_cast<WMError>(reply.ReadInt32());
}

WMError SceneSessionManagerProxy::GetProcessSurfaceNodeIdByPersistentId(const int32_t pid,
    const std::vector<int32_t>& persistentIds, std::vector<uint64_t>& surfaceNodeIds)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::DEFAULT, "Write interfaceToken failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    if (!data.WriteInt32(pid)) {
        TLOGE(WmsLogTag::DEFAULT, "Write pid failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    if (!data.WriteInt32Vector(persistentIds)) {
        TLOGE(WmsLogTag::DEFAULT, "Write persistentIds failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::WMS_LIFE, "remote is null");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    if (remote->SendRequest(static_cast<uint32_t>(
        SceneSessionManagerMessage::TRANS_ID_GET_PROCESS_SURFACENODEID_BY_PERSISTENTID),
        data, reply, option) != ERR_NONE) {
        TLOGE(WmsLogTag::DEFAULT, "SendRequest failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    reply.ReadUInt64Vector(&surfaceNodeIds);
    return static_cast<WMError>(reply.ReadInt32());
}

WMError SceneSessionManagerProxy::SkipSnapshotForAppProcess(int32_t pid, bool skip)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "WriteInterfaceToken failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    if (!data.WriteInt32(pid)) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "Write pid failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    if (!data.WriteBool(skip)) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "Write skip failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::WMS_LIFE, "remote is null");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    if (remote->SendRequest(static_cast<uint32_t>(SceneSessionManagerMessage::TRANS_ID_SET_PROCESS_SNAPSHOT_SKIP),
        data, reply, option) != ERR_NONE) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "SendRequest failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }

    return static_cast<WMError>(reply.ReadInt32());
}

WMError SceneSessionManagerProxy::SkipSnapshotByUserIdAndBundleNames(int32_t userId,
    const std::vector<std::string>& bundleNameList)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "Write interfaceToken failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    if (!data.WriteInt32(userId)) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "Write userId failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    if (!data.WriteStringVector(bundleNameList)) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "Write bundleNameList failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::WMS_LIFE, "remote is null");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    if (remote->SendRequest(static_cast<uint32_t>(
        SceneSessionManagerMessage::TRANS_ID_SET_SNAPSHOT_SKIP_BY_USERID_AND_BUNDLENAMES),
        data, reply, option) != ERR_NONE) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "SendRequest failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    return static_cast<WMError>(reply.ReadInt32());
}

WMError SceneSessionManagerProxy::SetProcessWatermark(int32_t pid, const std::string& watermarkName, bool isEnabled)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "WriteInterfaceToken failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    if (!data.WriteInt32(pid)) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "Write pid failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    if (!data.WriteString(watermarkName)) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "Write watermarkName failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    if (!data.WriteBool(isEnabled)) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "Write isEnabled failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    if (Remote()->SendRequest(static_cast<uint32_t>(SceneSessionManagerMessage::TRANS_ID_SET_PROCESS_WATERMARK),
        data, reply, option) != ERR_NONE) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "SendRequest failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }

    return static_cast<WMError>(reply.ReadInt32());
}

WMError SceneSessionManagerProxy::GetWindowIdsByCoordinate(DisplayId displayId, int32_t windowNumber,
    int32_t x, int32_t y, std::vector<int32_t>& windowIds)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::DEFAULT, "Write interfaceToken failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    if (!data.WriteUint64(displayId)) {
        TLOGE(WmsLogTag::DEFAULT, "Write displayId failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    if (!data.WriteInt32(windowNumber)) {
        TLOGE(WmsLogTag::DEFAULT, "Write windowNumber failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    if (!data.WriteInt32(x)) {
        TLOGE(WmsLogTag::DEFAULT, "Write x failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    if (!data.WriteInt32(y)) {
        TLOGE(WmsLogTag::DEFAULT, "Write y failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::DEFAULT, "remote is null");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    if (remote->SendRequest(static_cast<uint32_t>(SceneSessionManagerMessage::TRANS_ID_GET_WINDOW_IDS_BY_COORDINATE),
        data, reply, option) != ERR_NONE) {
        TLOGE(WmsLogTag::DEFAULT, "SendRequest failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    auto ret = static_cast<WMError>(reply.ReadInt32());
    if (ret == WMError::WM_OK) {
        reply.ReadInt32Vector(&windowIds);
    }
    return ret;
}

WMError SceneSessionManagerProxy::UpdateScreenLockStatusForApp(const std::string& bundleName, bool isRelease)
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
    if (Remote()->SendRequest(static_cast<uint32_t>(SceneSessionManagerMessage::TRANS_ID_UPDATE_SESSION_SCREEN_LOCK),
        data, reply, option) != ERR_NONE) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "SendRequest failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    return static_cast<WMError>(reply.ReadInt32());
}

WMError SceneSessionManagerProxy::AddSkipSelfWhenShowOnVirtualScreenList(const std::vector<int32_t>& persistentIds)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "write token failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    if (!data.WriteUint64(persistentIds.size())) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "write size failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    for (const auto persistentId: persistentIds) {
        if (!data.WriteInt32(persistentId)) {
            TLOGE(WmsLogTag::WMS_ATTRIBUTE, "write persistentId failed");
            return WMError::WM_ERROR_IPC_FAILED;
        }
    }
    if (Remote()->SendRequest(
        static_cast<uint32_t>(SceneSessionManagerMessage::TRANS_ID_ADD_SKIP_SELF_ON_VIRTUAL_SCREEN),
        data, reply, option) != ERR_NONE) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "SendRequest failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    return static_cast<WMError>(reply.ReadInt32());
}

WMError SceneSessionManagerProxy::RemoveSkipSelfWhenShowOnVirtualScreenList(const std::vector<int32_t>& persistentIds)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "write token failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    if (!data.WriteUint64(persistentIds.size())) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "write size failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    for (const auto persistentId: persistentIds) {
        if (!data.WriteInt32(persistentId)) {
            TLOGE(WmsLogTag::WMS_ATTRIBUTE, "write persistentId failed");
            return WMError::WM_ERROR_IPC_FAILED;
        }
    }
    if (Remote()->SendRequest(
        static_cast<uint32_t>(SceneSessionManagerMessage::TRANS_ID_REMOVE_SKIP_SELF_ON_VIRTUAL_SCREEN),
        data, reply, option) != ERR_NONE) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "SendRequest failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    return static_cast<WMError>(reply.ReadInt32());
}

WMError SceneSessionManagerProxy::SetScreenPrivacyWindowTagSwitch(
    uint64_t screenId, const std::vector<std::string>& privacyWindowTags, bool enable)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "write token failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    if (!data.WriteUint64(screenId)) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "write screenId failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    if (!data.WriteUint64(privacyWindowTags.size())) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "write size failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    for (const auto privacyWidnowTag: privacyWindowTags) {
        if (!data.WriteString(privacyWidnowTag)) {
            TLOGE(WmsLogTag::WMS_ATTRIBUTE, "write privacyWidnowTag failed");
            return WMError::WM_ERROR_IPC_FAILED;
        }
    }
    if (!data.WriteBool(enable)) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "write enable failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }

    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "Remote is null");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    if (remote->SendRequest(
        static_cast<uint32_t>(SceneSessionManagerMessage::TRANS_ID_SET_SCREEN_PRIVACY_WINDOW_TAG_SWITCH),
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

WMError SceneSessionManagerProxy::IsPcOrPadFreeMultiWindowMode(bool& isPcOrPadFreeMultiWindowMode)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::WMS_SUB, "Write interfaceToken failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::WMS_SUB, "Remote is null");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    if (remote->SendRequest(static_cast<uint32_t>(
        SceneSessionManagerMessage::TRANS_ID_IS_PC_OR_PAD_FREE_MULTI_WINDOW_MODE),
        data, reply, option) != ERR_NONE) {
        TLOGE(WmsLogTag::WMS_SUB, "SendRequest failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    bool repliedValue = false;
    if (!reply.ReadBool(repliedValue)) {
        TLOGE(WmsLogTag::WMS_SUB, "Read isPcOrPadFreeMultiWindowMode failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    int32_t ret = 0;
    if (!reply.ReadInt32(ret)) {
        TLOGE(WmsLogTag::WMS_SUB, "Read ret failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    isPcOrPadFreeMultiWindowMode = repliedValue;
    return static_cast<WMError>(ret);
}

WMError SceneSessionManagerProxy::IsPcWindow(bool& isPcWindow)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::WMS_UIEXT, "Write interfaceToken failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::WMS_UIEXT, "Remote is null");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    if (remote->SendRequest(static_cast<uint32_t>(
        SceneSessionManagerMessage::TRANS_ID_IS_PC_WINDOW),
        data, reply, option) != ERR_NONE) {
        TLOGE(WmsLogTag::WMS_UIEXT, "SendRequest failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    bool result = false;
    if (!reply.ReadBool(result)) {
        TLOGE(WmsLogTag::WMS_UIEXT, "Read isPcWindow failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    int32_t ret = 0;
    if (!reply.ReadInt32(ret)) {
        TLOGE(WmsLogTag::WMS_UIEXT, "Read ret failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    isPcWindow = result;
    return static_cast<WMError>(ret);
}

WMError SceneSessionManagerProxy::IsFreeMultiWindow(bool& isFreeMultiWindow)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::WMS_PC, "Write interfaceToken failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::WMS_PC, "Remote is null");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    if (remote->SendRequest(static_cast<uint32_t>(
        SceneSessionManagerMessage::TRANS_ID_IS_FREE_MULTI_WINDOW),
        data, reply, option) != ERR_NONE) {
        TLOGE(WmsLogTag::WMS_PC, "SendRequest failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    int32_t ret = 0;
    if (!reply.ReadInt32(ret)) {
        TLOGE(WmsLogTag::WMS_PC, "Read ret failed ret:%{public}d", ret);
        return WMError::WM_ERROR_IPC_FAILED;
    }
    auto ipcRes = static_cast<WMError>(ret);
    bool result = false;
    if (ipcRes == WMError::WM_OK && !reply.ReadBool(result)) {
        TLOGE(WmsLogTag::WMS_PC, "Read isFreeMultiWindow failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }

    isFreeMultiWindow = result;
    return ipcRes;
}

WMError SceneSessionManagerProxy::GetDisplayIdByWindowId(const std::vector<uint64_t>& windowIds,
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
    if (Remote()->SendRequest(static_cast<uint32_t>(SceneSessionManagerMessage::TRANS_ID_GET_DISPLAYID_BY_WINDOWID),
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

WMError SceneSessionManagerProxy::IsWindowRectAutoSave(const std::string& key, bool& enabled, int persistentId)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::WMS_MAIN, "Write interfaceToken failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    if (!data.WriteString(key)) {
        TLOGE(WmsLogTag::WMS_MAIN, "Write key failed");
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
    if (remote->SendRequest(static_cast<uint32_t>(SceneSessionManagerMessage::TRANS_ID_IS_WINDOW_RECT_AUTO_SAVE),
        data, reply, option) != ERR_NONE) {
        TLOGE(WmsLogTag::WMS_MAIN, "SendRequest failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    if (!reply.ReadBool(enabled)) {
        TLOGE(WmsLogTag::WMS_MAIN, "Read enable failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    uint32_t ret = 0;
    if (!reply.ReadUint32(ret)) {
        TLOGE(WmsLogTag::WMS_MAIN, "Read ret failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    return static_cast<WMError>(ret);
}

WMError SceneSessionManagerProxy::SetImageForRecent(uint32_t imgResourceId, ImageFit imageFit, int32_t persistentId)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::WMS_PATTERN, "Write interfaceToken failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    if (!data.WriteUint32(imgResourceId)) {
        TLOGE(WmsLogTag::WMS_PATTERN, "Write imgResourceId failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    if (!data.WriteUint32(static_cast<uint32_t>(imageFit))) {
        TLOGE(WmsLogTag::WMS_PATTERN, "Write ImageFit failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    if (!data.WriteInt32(persistentId)) {
        TLOGE(WmsLogTag::WMS_PATTERN, "Write persistentId failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::WMS_PATTERN, "remote is null");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    int sendCode = remote->SendRequest(
        static_cast<uint32_t>(SceneSessionManagerMessage::TRANS_ID_SET_IMAGE_FOR_RECENT),
        data, reply, option);
    if (sendCode != ERR_NONE) {
        TLOGE(WmsLogTag::WMS_PATTERN, "SendRequest failed, code: %{public}d", sendCode);
        return WMError::WM_ERROR_IPC_FAILED;
    }
    uint32_t ret = 0;
    if (!reply.ReadUint32(ret)) {
        TLOGE(WmsLogTag::WMS_PATTERN, "Read ret failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    return static_cast<WMError>(ret);
}

WMError SceneSessionManagerProxy::SetImageForRecentPixelMap(const std::shared_ptr<Media::PixelMap>& pixelMap,
    ImageFit imageFit, int32_t persistentId)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::WMS_PATTERN, "Write interfaceToken failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    if (!data.WriteParcelable(pixelMap.get())) {
        TLOGE(WmsLogTag::WMS_PATTERN, "Write pixelMap failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    if (!data.WriteUint32(static_cast<uint32_t>(imageFit))) {
        TLOGE(WmsLogTag::WMS_PATTERN, "Write ImageFit failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    if (!data.WriteInt32(persistentId)) {
        TLOGE(WmsLogTag::WMS_PATTERN, "Write persistentId failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::WMS_PATTERN, "remote is null");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    int sendCode = remote->SendRequest(
        static_cast<uint32_t>(SceneSessionManagerMessage::TRANS_ID_SET_IMAGE_FOR_RECENT_PIXELMAP),
        data, reply, option);
    if (sendCode != ERR_NONE) {
        TLOGE(WmsLogTag::WMS_PATTERN, "SendRequest failed, code: %{public}d", sendCode);
        return WMError::WM_ERROR_IPC_FAILED;
    }
    uint32_t ret = 0;
    if (!reply.ReadUint32(ret)) {
        TLOGE(WmsLogTag::WMS_PATTERN, "Read ret failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    return static_cast<WMError>(ret);
}

WMError SceneSessionManagerProxy::RemoveImageForRecent(int32_t persistentId)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::WMS_PATTERN, "Write interfaceToken failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    if (!data.WriteInt32(persistentId)) {
        TLOGE(WmsLogTag::WMS_PATTERN, "Write persistentId failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::WMS_PATTERN, "remote is null");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    int sendCode = remote->SendRequest(
        static_cast<uint32_t>(SceneSessionManagerMessage::TRANS_ID_REMOVE_IMAGE_FOR_RECENT),
        data, reply, option);
    if (sendCode != ERR_NONE) {
        TLOGE(WmsLogTag::WMS_PATTERN, "SendRequest failed, code: %{public}d", sendCode);
        return WMError::WM_ERROR_IPC_FAILED;
    }
    uint32_t ret = 0;
    if (!reply.ReadUint32(ret)) {
        TLOGE(WmsLogTag::WMS_PATTERN, "Read ret failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    return static_cast<WMError>(ret);
}

WMError SceneSessionManagerProxy::SetGlobalDragResizeType(DragResizeType dragResizeType)
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
    if (remote->SendRequest(static_cast<uint32_t>(SceneSessionManagerMessage::TRANS_ID_SET_GLOBAL_DRAG_RESIZE_TYPE),
        data, reply, option) != ERR_NONE) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "SendRequest failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    uint32_t ret = 0;
    if (!reply.ReadUint32(ret)) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "Read ret failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    return static_cast<WMError>(ret);
}

WMError SceneSessionManagerProxy::GetGlobalDragResizeType(DragResizeType& dragResizeType)
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
    if (remote->SendRequest(static_cast<uint32_t>(SceneSessionManagerMessage::TRANS_ID_GET_GLOBAL_DRAG_RESIZE_TYPE),
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
    uint32_t ret = 0;
    if (!reply.ReadUint32(ret)) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "Read ret failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    return static_cast<WMError>(ret);
}

WMError SceneSessionManagerProxy::SetAppDragResizeType(const std::string& bundleName, DragResizeType dragResizeType)
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
    if (remote->SendRequest(static_cast<uint32_t>(SceneSessionManagerMessage::TRANS_ID_SET_APP_DRAG_RESIZE_TYPE),
        data, reply, option) != ERR_NONE) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "SendRequest failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    uint32_t ret = 0;
    if (!reply.ReadUint32(ret)) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "Read ret failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    return static_cast<WMError>(ret);
}

WMError SceneSessionManagerProxy::GetAppDragResizeType(const std::string& bundleName, DragResizeType& dragResizeType)
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
    if (remote->SendRequest(static_cast<uint32_t>(SceneSessionManagerMessage::TRANS_ID_GET_APP_DRAG_RESIZE_TYPE),
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
    uint32_t ret = 0;
    if (!reply.ReadUint32(ret)) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "Read ret failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    return static_cast<WMError>(ret);
}

WMError SceneSessionManagerProxy::SetAppKeyFramePolicy(
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
    if (remote->SendRequest(static_cast<uint32_t>(SceneSessionManagerMessage::TRANS_ID_SET_APP_KEY_FRAME_POLICY),
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

WMError SceneSessionManagerProxy::ShiftAppWindowPointerEvent(int32_t sourcePersistentId, int32_t targetPersistentId,
                                                             int32_t fingerId)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::WMS_PC, "Write interfaceToken failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    if (!data.WriteInt32(sourcePersistentId)) {
        TLOGE(WmsLogTag::WMS_PC, "Write sourcePersistentId failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    if (!data.WriteInt32(targetPersistentId)) {
        TLOGE(WmsLogTag::WMS_PC, "Write targetPersistentId failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    if (!data.WriteInt32(fingerId)) {
        TLOGE(WmsLogTag::WMS_PC, "Write fingerId failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::WMS_PC, "remote is null");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    if (remote->SendRequest(static_cast<uint32_t>(SceneSessionManagerMessage::TRANS_ID_SHIFT_APP_WINDOW_POINTER_EVENT),
        data, reply, option) != ERR_NONE) {
        TLOGE(WmsLogTag::WMS_PC, "SendRequest failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    return static_cast<WMError>(reply.ReadInt32());
}

WMError SceneSessionManagerProxy::SetStartWindowBackgroundColor(
    const std::string& moduleName, const std::string& abilityName, uint32_t color, int32_t uid)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::WMS_PATTERN, "Write interfaceToken failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    if (!data.WriteString(moduleName)) {
        TLOGE(WmsLogTag::WMS_PATTERN, "Write moduleName failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    if (!data.WriteString(abilityName)) {
        TLOGE(WmsLogTag::WMS_PATTERN, "Write abilityName failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    if (!data.WriteUint32(color)) {
        TLOGE(WmsLogTag::WMS_PATTERN, "Write color failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    if (!data.WriteInt32(uid)) {
        TLOGE(WmsLogTag::WMS_PATTERN, "Write uid failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::WMS_PATTERN, "Remote is null");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    int sendCode = remote->SendRequest(
        static_cast<uint32_t>(SceneSessionManagerMessage::TRANS_ID_SET_START_WINDOW_BACKGROUND_COLOR),
        data, reply, option);
    if (sendCode != ERR_NONE) {
        TLOGE(WmsLogTag::WMS_PATTERN, "SendRequest failed, code: %{public}d", sendCode);
        return WMError::WM_ERROR_IPC_FAILED;
    }
    int32_t ret = 0;
    if (!reply.ReadInt32(ret)) {
        return WMError::WM_ERROR_IPC_FAILED;
    }
    return static_cast<WMError>(ret);
}

WMError SceneSessionManagerProxy::MinimizeByWindowId(const std::vector<int32_t>& windowIds)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::WMS_LIFE, "Write interfaceToken failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    if (!data.WriteInt32Vector(windowIds)) {
        TLOGE(WmsLogTag::WMS_LIFE, "Write int32Vector failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::WMS_LIFE, "Remote is null");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    if (remote->SendRequest(static_cast<uint32_t>(SceneSessionManagerMessage::TRANS_ID_MINIMIZE_BY_WINDOW_ID),
        data, reply, option) != ERR_NONE) {
        TLOGE(WmsLogTag::WMS_LIFE, "SendRequest failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    int32_t ret = 0;
    if (!reply.ReadInt32(ret)) {
        return WMError::WM_ERROR_IPC_FAILED;
    }
    return static_cast<WMError>(ret);
}

WMError SceneSessionManagerProxy::SetForegroundWindowNum(uint32_t windowNum)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::WMS_PC, "Write interfaceToken failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    if (!data.WriteUint32(windowNum)) {
        TLOGE(WmsLogTag::WMS_PC, "Write windowNum failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::WMS_PC, "Remote is null");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    if (remote->SendRequest(static_cast<uint32_t>(SceneSessionManagerMessage::TRANS_ID_SET_FOREGROUND_WINDOW_NUM),
        data, reply, option) != ERR_NONE) {
        TLOGE(WmsLogTag::WMS_PC, "SendRequest failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    return static_cast<WMError>(reply.ReadInt32());
}

WSError SceneSessionManagerProxy::UseImplicitAnimation(int32_t hostWindowId, bool useImplicit)
{
    TLOGD(WmsLogTag::WMS_UIEXT, "Run SceneSessionManagerProxy::UseImplicitAnimation");
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::WMS_UIEXT, "Write interface token failed.");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    if (!data.WriteInt32(hostWindowId)) {
        TLOGE(WmsLogTag::WMS_UIEXT, "Write hostWindowId failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    if (!data.WriteBool(useImplicit)) {
        TLOGE(WmsLogTag::WMS_UIEXT, "Write useImplicit failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::WMS_UIEXT, "remote is null");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    if (remote->SendRequest(static_cast<uint32_t>(SceneSessionManagerMessage::TRANS_ID_USE_IMPLICIT_ANIMATION),
        data, reply, option) != ERR_NONE) {
        TLOGE(WmsLogTag::WMS_UIEXT, "SendRequest UseImplicitAnimation failed");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    int32_t ret = 0;
    if (!reply.ReadInt32(ret)) {
        return WSError::WS_ERROR_IPC_FAILED;
    }
    return static_cast<WSError>(ret);
}

WMError SceneSessionManagerProxy::AnimateTo(int32_t windowId, const WindowAnimationProperty& animationProperty,
    const WindowAnimationOption& animationOption)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::WMS_ANIMATION, "Write interfaceToken failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    if (!data.WriteInt32(windowId)) {
        TLOGE(WmsLogTag::WMS_ANIMATION, "Write windowId failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    if (!data.WriteParcelable(&animationProperty)) {
        TLOGE(WmsLogTag::WMS_ANIMATION, "Write animationProperty failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    if (!data.WriteParcelable(&animationOption)) {
        TLOGE(WmsLogTag::WMS_ANIMATION, "Write animationOption failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::WMS_ANIMATION, "Remote is null");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    if (remote->SendRequest(static_cast<uint32_t>(SceneSessionManagerMessage::TRANS_ID_ANIMATE_TO_WINDOW),
        data, reply, option) != ERR_NONE) {
        TLOGE(WmsLogTag::WMS_ANIMATION, "SendRequest failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    return static_cast<WMError>(reply.ReadInt32());
}

WMError SceneSessionManagerProxy::CreateUIEffectController(const sptr<IUIEffectControllerClient>& controllerClient,
    sptr<IUIEffectController>& controller, int32_t& controllerId)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::WMS_ANIMATION, "Write interfaceToken failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    if (!data.WriteRemoteObject(controllerClient->AsObject())) {
        TLOGE(WmsLogTag::WMS_ANIMATION, "Write controller client failed!");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::WMS_ANIMATION, "Remote is null");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    if (remote->SendRequest(static_cast<uint32_t>(SceneSessionManagerMessage::TRANS_ID_CREATE_UI_EFFECT_CONTROLLER),
        data, reply, option) != ERR_NONE) {
        TLOGE(WmsLogTag::WMS_ANIMATION, "SendRequest failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    WMError err = static_cast<WMError>(reply.ReadInt32());
    if (err != WMError::WM_OK) {
        return err;
    }
    controllerId = reply.ReadInt32();
    sptr<IRemoteObject> controllerObject = reply.ReadRemoteObject();
    if (controllerObject == nullptr) {
        TLOGE(WmsLogTag::WMS_ANIMATION, "receive controller object failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    controller = iface_cast<IUIEffectController>(controllerObject);
    return err;
}

WMError SceneSessionManagerProxy::AddSessionBlackList(
    const std::unordered_set<std::string>& bundleNames, const std::unordered_set<std::string>& privacyWindowTags)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "Write interfaceToken failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    if (!data.WriteUint64(bundleNames.size())) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "Write size failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    for (auto bundleName : bundleNames) {
        if (!data.WriteString(bundleName)) {
            TLOGE(WmsLogTag::WMS_ATTRIBUTE, "Write bundleName failed");
            return WMError::WM_ERROR_IPC_FAILED;
        }
    }
    if (!data.WriteUint64(privacyWindowTags.size())) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "Write size failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    for (auto privacyWindowTag : privacyWindowTags) {
        if (!data.WriteString(privacyWindowTag)) {
            TLOGE(WmsLogTag::WMS_ATTRIBUTE, "Write privacyWindowTag failed");
            return WMError::WM_ERROR_IPC_FAILED;
        }
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::WMS_ANIMATION, "Remote is null");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    if (remote->SendRequest(
        static_cast<uint32_t>(SceneSessionManagerMessage::TRANS_ID_ADD_SESSION_BLACK_LIST),
        data, reply, option) != ERR_NONE) {
        TLOGE(WmsLogTag::WMS_ANIMATION, "SendRequest failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    int32_t ret = 0;
    if (!reply.ReadInt32(ret)) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "Read ret failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    return static_cast<WMError>(ret);
}

WMError SceneSessionManagerProxy::RemoveSessionBlackList(
    const std::unordered_set<std::string>& bundleNames, const std::unordered_set<std::string>& privacyWindowTags)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "Write interfaceToken failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    if (!data.WriteUint64(bundleNames.size())) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "Write size failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    for (auto bundleName : bundleNames) {
        if (!data.WriteString(bundleName)) {
            TLOGE(WmsLogTag::WMS_ATTRIBUTE, "Write bundleName failed");
            return WMError::WM_ERROR_IPC_FAILED;
        }
    }
    if (!data.WriteUint64(privacyWindowTags.size())) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "Write size failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    for (auto privacyWindowTag : privacyWindowTags) {
        if (!data.WriteString(privacyWindowTag)) {
            TLOGE(WmsLogTag::WMS_ATTRIBUTE, "Write privacyWindowTag failed");
            return WMError::WM_ERROR_IPC_FAILED;
        }
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::WMS_ANIMATION, "Remote is null");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    if (remote->SendRequest(
        static_cast<uint32_t>(SceneSessionManagerMessage::TRANS_ID_REMOVE_SESSION_BLACK_LIST),
        data, reply, option) != ERR_NONE) {
        TLOGE(WmsLogTag::WMS_ANIMATION, "SendRequest failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    int32_t ret = 0;
    if (!reply.ReadInt32(ret)) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "Read ret failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    return static_cast<WMError>(ret);
}

WMError SceneSessionManagerProxy::GetPiPSettingSwitchStatus(bool& switchStatus)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::WMS_PIP, "Write interfaceToken failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    sptr remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::WMS_PIP, "Remote is null");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    if (remote->SendRequest(static_cast<uint32_t>(SceneSessionManagerMessage::TRANS_ID_GET_PIP_SWITCH_STATUS),
        data, reply, option) != ERR_NONE) {
        TLOGE(WmsLogTag::WMS_PIP, "SendRequest failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    bool status = false;
    if (!reply.ReadBool(status)) {
        TLOGE(WmsLogTag::WMS_PIP, "Read status failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    int32_t ret = 0;
    if (!reply.ReadInt32(ret)) {
        TLOGE(WmsLogTag::WMS_PIP, "Read ret failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    switchStatus = status;
    return static_cast<WMError>(ret);
}

WMError SceneSessionManagerProxy::GetIsPipEnabled(bool& isPipEnabled)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::WMS_PIP, "Write interfaceToken failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    sptr remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::WMS_PIP, "Remote is null");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    if (remote->SendRequest(static_cast<uint32_t>(SceneSessionManagerMessage::TRANS_ID_GET_PIP_IS_PIP_ENABLED),
        data, reply, option) != ERR_NONE) {
        TLOGE(WmsLogTag::WMS_PIP, "SendRequest failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    bool status = false;
    if (!reply.ReadBool(status)) {
        TLOGE(WmsLogTag::WMS_PIP, "Read status failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    int32_t ret = 0;
    if (!reply.ReadInt32(ret)) {
        TLOGE(WmsLogTag::WMS_PIP, "Read ret failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    isPipEnabled = status;
    return static_cast<WMError>(ret);
}

WMError SceneSessionManagerProxy::UpdateOutline(const sptr<IRemoteObject>& remoteObject,
                                                const OutlineParams& outlineParams)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);

    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::WMS_ANIMATION, "Write interface token failed.");
        return WMError::WM_ERROR_IPC_FAILED;
    }

    if (remoteObject == nullptr || !data.WriteRemoteObject(remoteObject)) {
        TLOGE(WmsLogTag::WMS_ANIMATION, "Write remote object failed.");
        return WMError::WM_ERROR_IPC_FAILED;
    }

    if (!data.WriteParcelable(&outlineParams)) {
        TLOGE(WmsLogTag::WMS_ANIMATION, "Write outline params failed.");
        return WMError::WM_ERROR_IPC_FAILED;
    }

    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::WMS_ANIMATION, "Remote is null.");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    if (remote->SendRequest(static_cast<uint32_t>(SceneSessionManagerMessage::TRANS_ID_UPDATE_OUTLINE),
        data, reply, option) != ERR_NONE) {
        TLOGE(WmsLogTag::WMS_ANIMATION, "Send request failed.");
        return WMError::WM_ERROR_IPC_FAILED;
    }

    int32_t ret = 0;
    if (!reply.ReadInt32(ret)) {
        TLOGE(WmsLogTag::WMS_ANIMATION, "Read reply failed.");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    return static_cast<WMError>(ret);
}

WMError SceneSessionManagerProxy::NotifySupportRotationRegistered()
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::WMS_ROTATION, "Write interfaceToken failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    sptr remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::WMS_ROTATION, "Remote is null");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    if (remote->SendRequest(static_cast<uint32_t>(SceneSessionManagerMessage::TRANS_ID_SUPPORT_ROTATION_REGISTERED),
        data, reply, option) != ERR_NONE) {
        TLOGE(WmsLogTag::WMS_ROTATION, "SendRequest failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    return WMError::WM_OK;
}
} // namespace OHOS::Rosen
