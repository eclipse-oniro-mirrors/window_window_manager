/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

#include "zidl/window_manager_proxy.h"
#include <ipc_types.h>
#include <key_event.h>
#include <rs_iwindow_animation_controller.h>
#include <rs_window_animation_target.h>

#include "marshalling_helper.h"
#include "window_manager_hilog.h"

namespace OHOS {
namespace Rosen {
namespace {
constexpr HiviewDFX::HiLogLabel LABEL = {LOG_CORE, HILOG_DOMAIN_WINDOW, "WindowManagerProxy"};
}


WMError WindowManagerProxy::CreateWindow(sptr<IWindow>& window, sptr<WindowProperty>& property,
    const std::shared_ptr<RSSurfaceNode>& surfaceNode, uint32_t& windowId, sptr<IRemoteObject> token)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        WLOGFE("WriteInterfaceToken failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }

    if (!data.WriteRemoteObject(window->AsObject())) {
        WLOGFE("Write IWindow failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }

    if (!data.WriteParcelable(property.GetRefPtr())) {
        WLOGFE("Write windowProperty failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }

    if (surfaceNode == nullptr || !surfaceNode->Marshalling(data)) {
        WLOGFE("Write windowProperty failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    if (token != nullptr) {
        if (!data.WriteRemoteObject(token)) {
            WLOGFE("Write abilityToken failed");
            return WMError::WM_ERROR_IPC_FAILED;
        }
    }

    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        WLOGFE("remote is null");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    if (remote->SendRequest(static_cast<uint32_t>(WindowManagerMessage::TRANS_ID_CREATE_WINDOW),
        data, reply, option) != ERR_NONE) {
        return WMError::WM_ERROR_IPC_FAILED;
    }
    windowId = reply.ReadUint32();
    int32_t ret = reply.ReadInt32();
    property->SetWindowFlags(reply.ReadUint32());
    property->SetApiCompatibleVersion(reply.ReadUint32());
    return static_cast<WMError>(ret);
}

WMError WindowManagerProxy::AddWindow(sptr<WindowProperty>& property)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        WLOGFE("WriteInterfaceToken failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }

    if (!data.WriteParcelable(property.GetRefPtr())) {
        WLOGFE("Write windowProperty failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }

    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        WLOGFE("remote is null");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    if (remote->SendRequest(static_cast<uint32_t>(WindowManagerMessage::TRANS_ID_ADD_WINDOW),
        data, reply, option) != ERR_NONE) {
        return WMError::WM_ERROR_IPC_FAILED;
    }

    int32_t ret = reply.ReadInt32();
    return static_cast<WMError>(ret);
}

WMError WindowManagerProxy::RemoveWindow(uint32_t windowId, bool isFromInnerkits)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        WLOGFE("WriteInterfaceToken failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }

    if (!data.WriteUint32(windowId)) {
        WLOGFE("Write windowId failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }

    if (!data.WriteBool(isFromInnerkits)) {
        WLOGFE("Write isFromInnerkits failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }

    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        WLOGFE("remote is null");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    if (remote->SendRequest(static_cast<uint32_t>(WindowManagerMessage::TRANS_ID_REMOVE_WINDOW),
        data, reply, option) != ERR_NONE) {
        return WMError::WM_ERROR_IPC_FAILED;
    }

    int32_t ret = reply.ReadInt32();
    return static_cast<WMError>(ret);
}

WMError WindowManagerProxy::DestroyWindow(uint32_t windowId, bool /* onlySelf */)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        WLOGFE("WriteInterfaceToken failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }

    if (!data.WriteUint32(windowId)) {
        WLOGFE("Write windowId failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }

    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        WLOGFE("remote is null");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    if (remote->SendRequest(static_cast<uint32_t>(WindowManagerMessage::TRANS_ID_DESTROY_WINDOW),
        data, reply, option) != ERR_NONE) {
        return WMError::WM_ERROR_IPC_FAILED;
    }

    int32_t ret = reply.ReadInt32();
    return static_cast<WMError>(ret);
}

WMError WindowManagerProxy::RequestFocus(uint32_t windowId)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        WLOGFE("WriteInterfaceToken failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }

    if (!data.WriteUint32(windowId)) {
        WLOGFE("Write windowId failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        WLOGFE("remote is null");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    if (remote->SendRequest(static_cast<uint32_t>(WindowManagerMessage::TRANS_ID_REQUEST_FOCUS),
        data, reply, option) != ERR_NONE) {
        return WMError::WM_ERROR_IPC_FAILED;
    }

    int32_t ret = reply.ReadInt32();
    return static_cast<WMError>(ret);
}

AvoidArea WindowManagerProxy::GetAvoidAreaByType(uint32_t windowId, AvoidAreaType type, const Rect& rect)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    AvoidArea avoidArea;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        WLOGFE("WriteInterfaceToken failed");
        return avoidArea;
    }

    if (!data.WriteUint32(windowId)) {
        WLOGFE("Write windowId failed");
        return avoidArea;
    }

    if (!data.WriteUint32(static_cast<uint32_t>(type))) {
        WLOGFE("Write AvoidAreaType failed");
        return avoidArea;
    }

    if (!data.WriteInt32(rect.posX_) || !data.WriteInt32(rect.posY_) ||
        !data.WriteUint32(rect.width_) || !data.WriteUint32(rect.height_)) {
        TLOGE(WmsLogTag::WMS_IMMS, "write rect error");
        return avoidArea;
    }

    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        WLOGFE("remote is null");
        return avoidArea;
    }
    if (remote->SendRequest(static_cast<uint32_t>(WindowManagerMessage::TRANS_ID_GET_AVOID_AREA),
        data, reply, option) != ERR_NONE) {
        return avoidArea;
    }
    sptr<AvoidArea> area = reply.ReadParcelable<AvoidArea>();
    if (area == nullptr) {
        return avoidArea;
    }
    return *area;
}

WMError WindowManagerProxy::RegisterWindowManagerAgent(WindowManagerAgentType type,
    const sptr<IWindowManagerAgent>& windowManagerAgent)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        WLOGFE("WriteInterfaceToken failed");
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
    if (remote->SendRequest(static_cast<uint32_t>(WindowManagerMessage::TRANS_ID_REGISTER_WINDOW_MANAGER_AGENT),
        data, reply, option) != ERR_NONE) {
        WLOGFE("SendRequest failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }

    return static_cast<WMError>(reply.ReadInt32());
}

WMError WindowManagerProxy::UnregisterWindowManagerAgent(WindowManagerAgentType type,
    const sptr<IWindowManagerAgent>& windowManagerAgent)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        WLOGFE("WriteInterfaceToken failed");
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
    if (remote->SendRequest(static_cast<uint32_t>(WindowManagerMessage::TRANS_ID_UNREGISTER_WINDOW_MANAGER_AGENT),
        data, reply, option) != ERR_NONE) {
        WLOGFE("SendRequest failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }

    return static_cast<WMError>(reply.ReadInt32());
}

WMError WindowManagerProxy::SetWindowAnimationController(const sptr<RSIWindowAnimationController>& controller)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (controller == nullptr) {
        WLOGFE("RSWindowAnimation Failed to set window animation controller, controller is null!");
        return WMError::WM_ERROR_IPC_FAILED;
    }

    if (!data.WriteInterfaceToken(GetDescriptor())) {
        WLOGFE("RSWindowAnimation Failed to WriteInterfaceToken!");
        return WMError::WM_ERROR_IPC_FAILED;
    }

    if (!data.WriteRemoteObject(controller->AsObject())) {
        WLOGFE("RSWindowAnimation Failed to write controller!");
        return WMError::WM_ERROR_IPC_FAILED;
    }

    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        WLOGFE("remote is null");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    auto error = remote->SendRequest(static_cast<uint32_t>(WindowManagerMessage::TRANS_ID_ANIMATION_SET_CONTROLLER),
        data, reply, option);
    if (error != ERR_NONE) {
        WLOGFE("RSWindowAnimation Send request error: %{public}d", error);
        return WMError::WM_ERROR_IPC_FAILED;
    }

    int32_t ret = reply.ReadInt32();
    return static_cast<WMError>(ret);
}

void WindowManagerProxy::NotifyServerReadyToMoveOrDrag(uint32_t windowId, sptr<WindowProperty>& windowProperty,
    sptr<MoveDragProperty>& moveDragProperty)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        WLOGFE("WriteInterfaceToken failed");
        return;
    }
    if (!data.WriteUint32(windowId)) {
        WLOGFE("Write windowId failed");
        return;
    }
    if (!data.WriteParcelable(windowProperty.GetRefPtr())) {
        WLOGFE("Failed to write windowProperty!");
        return;
    }
    if (!data.WriteParcelable(moveDragProperty.GetRefPtr())) {
        WLOGFE("Failed to write moveDragProperty!");
        return;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        WLOGFE("remote is null");
        return;
    }
    if (remote->SendRequest(static_cast<uint32_t>(WindowManagerMessage::TRANS_ID_NOTIFY_READY_MOVE_OR_DRAG),
        data, reply, option) != ERR_NONE) {
        WLOGFE("SendRequest failed");
    }
}

void WindowManagerProxy::ProcessPointDown(uint32_t windowId, bool isPointDown)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        WLOGFE("WriteInterfaceToken failed");
        return;
    }
    if (!data.WriteUint32(windowId)) {
        WLOGFE("Write windowId failed");
        return;
    }
    if (!data.WriteBool(isPointDown)) {
        WLOGFE("Write isPointDown failed");
        return;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        WLOGFE("remote is null");
        return;
    }
    if (remote->SendRequest(static_cast<uint32_t>(WindowManagerMessage::TRANS_ID_PROCESS_POINT_DOWN),
        data, reply, option) != ERR_NONE) {
        WLOGFE("SendRequest failed");
    }
}

void WindowManagerProxy::ProcessPointUp(uint32_t windowId)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        WLOGFE("WriteInterfaceToken failed");
        return;
    }
    if (!data.WriteUint32(windowId)) {
        WLOGFE("Write windowId failed");
        return;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        WLOGFE("remote is null");
        return;
    }
    if (remote->SendRequest(static_cast<uint32_t>(WindowManagerMessage::TRANS_ID_PROCESS_POINT_UP),
        data, reply, option) != ERR_NONE) {
        WLOGFE("SendRequest failed");
    }
}

WMError WindowManagerProxy::MinimizeAllAppWindows(DisplayId displayId, int32_t excludeWindowId)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        WLOGFE("WriteInterfaceToken failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    if (!data.WriteUint64(displayId)) {
        WLOGFE("Write displayId failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    if (!data.WriteInt32(excludeWindowId)) {
        WLOGFE("Write excludeWindowId failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        WLOGFE("remote is null");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    if (remote->SendRequest(static_cast<uint32_t>(WindowManagerMessage::TRANS_ID_MINIMIZE_ALL_APP_WINDOWS),
        data, reply, option) != ERR_NONE) {
        WLOGFE("SendRequest failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }

    int32_t ret;
    if (!reply.ReadInt32(ret)) {
        return WMError::WM_ERROR_IPC_FAILED;
    }

    WLOGFE("MinimizeAllAppWindows: %{public}u", ret);
    return static_cast<WMError>(ret);
}

WMError WindowManagerProxy::ToggleShownStateForAllAppWindows()
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        WLOGFE("WriteInterfaceToken failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        WLOGFE("remote is null");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    if (remote->SendRequest(
        static_cast<uint32_t>(WindowManagerMessage::TRANS_ID_TOGGLE_SHOWN_STATE_FOR_ALL_APP_WINDOWS),
        data, reply, option) != ERR_NONE) {
        WLOGFE("SendRequest failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    int32_t ret;
    if (!reply.ReadInt32(ret)) {
        return WMError::WM_ERROR_IPC_FAILED;
    }
    return static_cast<WMError>(ret);
}

WMError WindowManagerProxy::SetWindowLayoutMode(WindowLayoutMode mode)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        WLOGFE("WriteInterfaceToken failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    if (!data.WriteUint32(static_cast<uint32_t>(mode))) {
        WLOGFE("Write mode failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        WLOGFE("remote is null");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    if (remote->SendRequest(static_cast<uint32_t>(WindowManagerMessage::TRANS_ID_UPDATE_LAYOUT_MODE),
        data, reply, option) != ERR_NONE) {
        return WMError::WM_ERROR_IPC_FAILED;
    }

    int32_t ret = reply.ReadInt32();
    return static_cast<WMError>(ret);
}

WMError WindowManagerProxy::NotifyScreenshotEvent(ScreenshotEventType type)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "WriteInterfaceToken failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    if (!data.WriteInt32(static_cast<int32_t>(type))) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "Write screen shot event type failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "remote is null");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    if (remote->SendRequest(static_cast<uint32_t>(WindowManagerMessage::TRANS_ID_NOTIFY_SCREEN_SHOT_EVENT),
        data, reply, option) != ERR_NONE) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "SendRequest failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    int32_t ret = reply.ReadInt32();
    return static_cast<WMError>(ret);
}

WMError WindowManagerProxy::UpdateProperty(sptr<WindowProperty>& windowProperty, PropertyChangeAction action,
    bool isAsyncTask)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        WLOGFE("WriteInterfaceToken failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }

    if (!data.WriteUint32(static_cast<uint32_t>(action))) {
        WLOGFE("Write PropertyChangeAction failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }

    if (!windowProperty || !windowProperty->Write(data, action)) {
        WLOGFE("Write windowProperty failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }

    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        WLOGFE("remote is null");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    if (remote->SendRequest(static_cast<uint32_t>(WindowManagerMessage::TRANS_ID_UPDATE_PROPERTY),
        data, reply, option) != ERR_NONE) {
        return WMError::WM_ERROR_IPC_FAILED;
    }

    int32_t ret = reply.ReadInt32();
    return static_cast<WMError>(ret);
}

WMError WindowManagerProxy::SetWindowGravity(uint32_t windowId, WindowGravity gravity, uint32_t percent)
{
    MessageParcel data;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        WLOGFE("WriteInterfaceToken failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    if (!data.WriteUint32(windowId)) {
        WLOGFE("Write mainWinId failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    if (!data.WriteUint32(static_cast<uint32_t>(gravity))) {
        WLOGFE("Write mainWinId failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    if (!data.WriteUint32(percent)) {
        WLOGFE("Write mainWinId failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }

    MessageParcel reply;
    MessageOption option;
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        WLOGFE("remote is null");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    if (remote->SendRequest(static_cast<uint32_t>(WindowManagerMessage::TRANS_ID_SET_WINDOW_GRAVITY),
        data, reply, option) != ERR_NONE) {
        return WMError::WM_ERROR_IPC_FAILED;
    }

    int32_t ret = reply.ReadInt32();
    return static_cast<WMError>(ret);
}

__attribute__((no_sanitize("cfi"))) WMError WindowManagerProxy::GetTopWindowId(
    uint32_t mainWinId, uint32_t& topWinId)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        WLOGFE("WriteInterfaceToken failed");
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
    if (remote->SendRequest(static_cast<uint32_t>(WindowManagerMessage::TRANS_ID_GET_TOP_WINDOW_ID),
        data, reply, option) != ERR_NONE) {
        return WMError::WM_ERROR_IPC_FAILED;
    }
    topWinId = reply.ReadUint32();
    int32_t ret = reply.ReadInt32();
    return static_cast<WMError>(ret);
}

WMError WindowManagerProxy::GetAccessibilityWindowInfo(std::vector<sptr<AccessibilityWindowInfo>>& infos)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        WLOGFE("WriteInterfaceToken failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        WLOGFE("remote is null");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    if (remote->SendRequest(static_cast<uint32_t>(WindowManagerMessage::TRANS_ID_GET_ACCESSIBILITY_WINDOW_INFO_ID),
        data, reply, option) != ERR_NONE) {
        return WMError::WM_ERROR_IPC_FAILED;
    }
    if (!MarshallingHelper::UnmarshallingVectorParcelableObj<AccessibilityWindowInfo>(reply, infos)) {
        WLOGFE("read accessibility window infos failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    return static_cast<WMError>(reply.ReadInt32());
}

WMError WindowManagerProxy::GetUnreliableWindowInfo(int32_t windowId,
    std::vector<sptr<UnreliableWindowInfo>>& infos)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        WLOGFE("WriteInterfaceToken failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    if (!data.WriteInt32(windowId)) {
        WLOGFE("Write windowId failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        WLOGFE("remote is null");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    if (remote->SendRequest(
        static_cast<uint32_t>(WindowManagerMessage::TRANS_ID_GET_UNRELIABLE_WINDOW_INFO_ID),
        data, reply, option) != ERR_NONE) {
        return WMError::WM_ERROR_IPC_FAILED;
    }
    if (!MarshallingHelper::UnmarshallingVectorParcelableObj<UnreliableWindowInfo>(reply, infos)) {
        WLOGFE("read unreliable window infos failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    return static_cast<WMError>(reply.ReadInt32());
}

WMError WindowManagerProxy::GetVisibilityWindowInfo(std::vector<sptr<WindowVisibilityInfo>>& infos)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        WLOGFE("WriteInterfaceToken failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        WLOGFE("remote is null");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    if (remote->SendRequest(static_cast<uint32_t>(WindowManagerMessage::TRANS_ID_GET_VISIBILITY_WINDOW_INFO_ID),
        data, reply, option) != ERR_NONE) {
        return WMError::WM_ERROR_IPC_FAILED;
    }
    if (!MarshallingHelper::UnmarshallingVectorParcelableObj<WindowVisibilityInfo>(reply, infos)) {
        WLOGFE("read visibility window infos failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    return static_cast<WMError>(reply.ReadInt32());
}

WMError WindowManagerProxy::GetSystemConfig(SystemConfig& systemConfig)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        WLOGFE("WriteInterfaceToken failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        WLOGFE("remote is null");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    if (remote->SendRequest(static_cast<uint32_t>(WindowManagerMessage::TRANS_ID_GET_SYSTEM_CONFIG),
        data, reply, option) != ERR_NONE) {
        return WMError::WM_ERROR_IPC_FAILED;
    }
    sptr<SystemConfig> config = reply.ReadParcelable<SystemConfig>();
    if (config == nullptr) {
        WLOGFE("Read SystemConfig failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    systemConfig = *config;
    int32_t ret = reply.ReadInt32();
    return static_cast<WMError>(ret);
}

WMError WindowManagerProxy::NotifyWindowTransition(sptr<WindowTransitionInfo>& from, sptr<WindowTransitionInfo>& to,
    bool isFromClient)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!data.WriteInterfaceToken(GetDescriptor())) {
        WLOGFE("Failed to WriteInterfaceToken!");
        return WMError::WM_ERROR_IPC_FAILED;
    }

    if (!data.WriteParcelable(from)) {
        WLOGFE("Failed to write from ability window info!");
        return WMError::WM_ERROR_IPC_FAILED;
    }

    if (!data.WriteParcelable(to)) {
        WLOGFE("Failed to write to ability window info!");
        return WMError::WM_ERROR_IPC_FAILED;
    }

    if (!data.WriteBool(isFromClient)) {
        WLOGFE("Failed to write to isFromClient!");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        WLOGFE("remote is null");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    auto error = remote->SendRequest(static_cast<uint32_t>(WindowManagerMessage::TRANS_ID_NOTIFY_WINDOW_TRANSITION),
        data, reply, option);
    if (error != ERR_NONE) {
        WLOGFE("Send request error: %{public}d", static_cast<uint32_t>(error));
        return WMError::WM_ERROR_IPC_FAILED;
    }
    auto ret = static_cast<WMError>(reply.ReadInt32());
    return ret;
}

WMError WindowManagerProxy::GetModeChangeHotZones(DisplayId displayId, ModeChangeHotZones& hotZones)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!data.WriteInterfaceToken(GetDescriptor())) {
        WLOGFE("WriteInterfaceToken failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    if (!data.WriteUint64(displayId)) {
        WLOGFE("Write displayId failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        WLOGFE("remote is null");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    if (remote->SendRequest(static_cast<uint32_t>(WindowManagerMessage::TRANS_ID_GET_FULLSCREEN_AND_SPLIT_HOT_ZONE),
        data, reply, option) != ERR_NONE) {
        return WMError::WM_ERROR_IPC_FAILED;
    }

    auto ret = static_cast<WMError>(reply.ReadInt32());
    if (ret == WMError::WM_OK) {
        hotZones.fullscreen_.posX_ = reply.ReadInt32();
        hotZones.fullscreen_.posY_ = reply.ReadInt32();
        hotZones.fullscreen_.width_ = reply.ReadUint32();
        hotZones.fullscreen_.height_ = reply.ReadUint32();

        hotZones.primary_.posX_ = reply.ReadInt32();
        hotZones.primary_.posY_ = reply.ReadInt32();
        hotZones.primary_.width_ = reply.ReadUint32();
        hotZones.primary_.height_ = reply.ReadUint32();

        hotZones.secondary_.posX_ = reply.ReadInt32();
        hotZones.secondary_.posY_ = reply.ReadInt32();
        hotZones.secondary_.width_ = reply.ReadUint32();
        hotZones.secondary_.height_ = reply.ReadUint32();
    }
    return ret;
}

void WindowManagerProxy::MinimizeWindowsByLauncher(std::vector<uint32_t> windowIds, bool isAnimated,
    sptr<RSIWindowAnimationFinishedCallback>& finishCallback)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        WLOGFE("WriteInterfaceToken failed");
        return;
    }

    if (!data.WriteUInt32Vector(windowIds)) {
        WLOGFE("Write windowIds failed");
        return;
    }

    if (!data.WriteBool(isAnimated)) {
        WLOGFE("Write isAnimated failed");
        return;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        WLOGFE("remote is null");
        return;
    }
    if (remote->SendRequest(static_cast<uint32_t>(WindowManagerMessage::TRANS_ID_GET_ANIMATION_CALLBACK),
        data, reply, option) != ERR_NONE) {
        WLOGFE("Send request error");
        return;
    }
    if (reply.ReadBool()) {
        sptr<IRemoteObject> finishCallbackObject = reply.ReadRemoteObject();
        finishCallback = iface_cast<RSIWindowAnimationFinishedCallback>(finishCallbackObject);
    } else {
        finishCallback = nullptr;
    }
    return;
}

WMError WindowManagerProxy::UpdateAvoidAreaListener(uint32_t windowId, bool haveListener)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!data.WriteInterfaceToken(GetDescriptor())) {
        WLOGFE("WriteInterfaceToken failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    if (!data.WriteUint32(windowId)) {
        WLOGFE("Write windowId failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    if (!data.WriteBool(haveListener)) {
        WLOGFE("Write avoid area listener failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        WLOGFE("remote is null");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    if (remote->SendRequest(static_cast<uint32_t>(WindowManagerMessage::TRANS_ID_UPDATE_AVOIDAREA_LISTENER),
        data, reply, option) != ERR_NONE) {
        return WMError::WM_ERROR_IPC_FAILED;
    }
    return static_cast<WMError>(reply.ReadInt32());
}

WMError WindowManagerProxy::UpdateRsTree(uint32_t windowId, bool isAdd)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);

    if (!data.WriteInterfaceToken(GetDescriptor())) {
        WLOGFE("WriteInterfaceToken failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    if (!data.WriteUint32(windowId)) {
        WLOGFE("Write windowId failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    if (!data.WriteBool(isAdd)) {
        WLOGFE("Write avoid area listener failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        WLOGFE("remote is null");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    if (remote->SendRequest(static_cast<uint32_t>(WindowManagerMessage::TRANS_ID_UPDATE_RS_TREE),
        data, reply, option) != ERR_NONE) {
        return WMError::WM_ERROR_IPC_FAILED;
    }
    return static_cast<WMError>(reply.ReadInt32());
}

WMError WindowManagerProxy::BindDialogTarget(uint32_t& windowId, sptr<IRemoteObject> targetToken)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        WLOGFE("WriteInterfaceToken failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    if (!data.WriteUint32(windowId)) {
        WLOGFE("Write windowId failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    if (targetToken != nullptr) {
        if (!data.WriteRemoteObject(targetToken)) {
            WLOGFE("Write targetToken failed");
            return WMError::WM_ERROR_IPC_FAILED;
        }
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        WLOGFE("remote is null");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    if (remote->SendRequest(static_cast<uint32_t>(WindowManagerMessage::TRANS_ID_BIND_DIALOG_TARGET),
        data, reply, option) != ERR_NONE) {
        return WMError::WM_ERROR_IPC_FAILED;
    }

    int32_t ret = reply.ReadInt32();
    return static_cast<WMError>(ret);
}

void WindowManagerProxy::SetAnchorAndScale(int32_t x, int32_t y, float scale)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        WLOGFE("WriteInterfaceToken failed");
        return;
    }
    if (!data.WriteInt32(x)) {
        WLOGFE("Write anchor x failed");
        return;
    }
    if (!data.WriteInt32(y)) {
        WLOGFE("Write anchor y failed");
        return;
    }
    if (!data.WriteFloat(scale)) {
        WLOGFE("Write scale failed");
        return;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        WLOGFE("remote is null");
        return;
    }
    if (remote->SendRequest(static_cast<uint32_t>(WindowManagerMessage::TRANS_ID_SET_ANCHOR_AND_SCALE),
        data, reply, option) != ERR_NONE) {
        WLOGFE("SendRequest failed");
    }
}

void WindowManagerProxy::SetAnchorOffset(int32_t deltaX, int32_t deltaY)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        WLOGFE("WriteInterfaceToken failed");
        return;
    }
    if (!data.WriteInt32(deltaX)) {
        WLOGFE("Write anchor delatX failed");
        return;
    }
    if (!data.WriteInt32(deltaY)) {
        WLOGFE("Write anchor deltaY failed");
        return;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        WLOGFE("remote is null");
        return;
    }
    if (remote->SendRequest(static_cast<uint32_t>(WindowManagerMessage::TRANS_ID_SET_ANCHOR_OFFSET),
        data, reply, option) != ERR_NONE) {
        WLOGFE("SendRequest failed");
    }
}

void WindowManagerProxy::OffWindowZoom()
{
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
    if (remote->SendRequest(static_cast<uint32_t>(WindowManagerMessage::TRANS_ID_OFF_WINDOW_ZOOM),
        data, reply, option) != ERR_NONE) {
        WLOGFE("SendRequest failed");
    }
}

/** @note @window.hierarchy */
WMError WindowManagerProxy::RaiseToAppTop(uint32_t windowId)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        WLOGFE("WriteInterfaceToken failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }

    if (!data.WriteUint32(windowId)) {
        WLOGFE("Write anchor delatX failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }

    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        WLOGFE("remote is null");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    if (remote->SendRequest(static_cast<uint32_t>(WindowManagerMessage::TRANS_ID_RAISE_WINDOW_Z_ORDER),
        data, reply, option) != ERR_NONE) {
        WLOGFE("SendRequest failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    return WMError::WM_OK;
}

std::shared_ptr<Media::PixelMap> WindowManagerProxy::GetSnapshot(int32_t windowId)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!data.WriteInterfaceToken(GetDescriptor())) {
        WLOGFE("WriteInterfaceToken failed");
        return nullptr;
    }
    if (!data.WriteUint32(windowId)) {
        WLOGFE("Write windowId failed");
        return nullptr;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        WLOGFE("remote is null");
        return nullptr;
    }
    if (remote->SendRequest(static_cast<uint32_t>(WindowManagerMessage::TRANS_ID_GET_SNAPSHOT),
        data, reply, option) != ERR_NONE) {
        WLOGFE("SendRequest failed");
        return nullptr;
    }

    std::shared_ptr<Media::PixelMap> map(reply.ReadParcelable<Media::PixelMap>());
    if (map == nullptr) {
        WLOGFE("Read pixelMap is null");
        return nullptr;
    }
    return map;
}

WMError WindowManagerProxy::GetSnapshotByWindowId(int32_t persistentId, std::shared_ptr<Media::PixelMap>& pixelMap)
{
    pixelMap = GetSnapshot(persistentId);
    if (pixelMap == nullptr) {
        WLOGFE("Get snapshot is nullptr");
        return WMError::WM_ERROR_NULLPTR;
    }
    return WMError::WM_OK;
}

WMError WindowManagerProxy::SetGestureNavigationEnabled(bool enable)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        WLOGFE("WriteInterfaceToken failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }

    if (!data.WriteBool(enable)) {
        WLOGFE("Write anchor delatX failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }

    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        WLOGFE("remote is null");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    if (remote->SendRequest(static_cast<uint32_t>(WindowManagerMessage::TRANS_ID_GESTURE_NAVIGATION_ENABLED),
        data, reply, option) != ERR_NONE) {
        WLOGFE("SendRequest failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    int32_t ret = reply.ReadInt32();
    return static_cast<WMError>(ret);
}

void WindowManagerProxy::DispatchKeyEvent(uint32_t windowId, std::shared_ptr<MMI::KeyEvent> event)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        WLOGFE("WriteInterfaceToken failed");
        return;
    }
    if (!data.WriteUint32(windowId)) {
        WLOGFE("Write anchor delatX failed");
        return;
    }
    if (!event || !event->WriteToParcel(data)) {
        WLOGFE("Write event faild");
        return;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        WLOGFE("remote is null");
        return;
    }
    if (remote->SendRequest(static_cast<uint32_t>(WindowManagerMessage::TRANS_ID_DISPATCH_KEY_EVENT),
        data, reply, option) != ERR_NONE) {
        WLOGFE("SendRequest failed");
        return;
    }
}

void WindowManagerProxy::NotifyDumpInfoResult(const std::vector<std::string>& info)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        WLOGFE("WriteInterfaceToken pfailed");
        return;
    }
    if (!data.WriteStringVector(info)) {
        WLOGFE("Write info failed");
        return;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        WLOGFE("remote is null");
        return;
    }
    if (remote->SendRequest(static_cast<uint32_t>(WindowManagerMessage::TRANS_ID_NOTIFY_DUMP_INFO_RESULT),
        data, reply, option) != ERR_NONE) {
        WLOGFE("SendRequest failed");
        return;
    }
}

WMError WindowManagerProxy::GetWindowAnimationTargets(std::vector<uint32_t> missionIds,
    std::vector<sptr<RSWindowAnimationTarget>>& targets)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        WLOGFE("write interfaceToken failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    if (!data.WriteUInt32Vector(missionIds)) {
        WLOGFE("Write missionIds failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        WLOGFE("remote is null");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    if (remote->SendRequest(static_cast<uint32_t>(WindowManagerMessage::TRANS_ID_GET_WINDOW_ANIMATION_TARGETS),
        data, reply, option) != ERR_NONE) {
        return WMError::WM_ERROR_IPC_FAILED;
    }
    if (!MarshallingHelper::UnmarshallingVectorParcelableObj<RSWindowAnimationTarget>(reply, targets)) {
        WLOGFE("read window animation targets failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    return static_cast<WMError>(reply.ReadInt32());
}

void WindowManagerProxy::SetMaximizeMode(MaximizeMode maximizeMode)
{
    MessageParcel data;
    MessageOption option;
    MessageParcel reply;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        WLOGFE("WriteInterfaceToken failed!");
        return;
    }
    if (!data.WriteInt32(static_cast<uint32_t>(maximizeMode))) {
        WLOGFE("Write maximizeMode failed");
        return;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        WLOGFE("remote is null");
        return;
    }
    if (remote->SendRequest(static_cast<uint32_t>(WindowManagerMessage::TRANS_ID_SET_MAXIMIZE_MODE),
        data, reply, option) != ERR_NONE) {
        WLOGFE("SendRequest failed");
    }
}

MaximizeMode WindowManagerProxy::GetMaximizeMode()
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        WLOGFE("WriteInterfaceToken failed");
        return MaximizeMode::MODE_FULL_FILL;
    }

    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        WLOGFE("remote is null");
        return MaximizeMode::MODE_FULL_FILL;
    }
    if (remote->SendRequest(static_cast<uint32_t>(WindowManagerMessage::TRANS_ID_GET_MAXIMIZE_MODE),
        data, reply, option) != ERR_NONE) {
        WLOGFE("SendRequest failed");
        return MaximizeMode::MODE_FULL_FILL;
    }
    int32_t ret = reply.ReadInt32();
    return static_cast<MaximizeMode>(ret);
}

void WindowManagerProxy::GetFocusWindowInfo(FocusChangeInfo& focusInfo, DisplayId displayId)
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
    if (remote->SendRequest(static_cast<uint32_t>(WindowManagerMessage::TRANS_ID_GET_FOCUS_WINDOW_INFO),
        data, reply, option) != ERR_NONE) {
        WLOGFE("SendRequest failed");
        return;
    }
    sptr<FocusChangeInfo> info = reply.ReadParcelable<FocusChangeInfo>();
    if (info != nullptr) {
        focusInfo = *info;
    }
}
} // namespace Rosen
} // namespace OHOS
