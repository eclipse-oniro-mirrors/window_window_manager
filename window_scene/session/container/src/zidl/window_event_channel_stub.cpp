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

#include "session/container/include/zidl/window_event_channel_stub.h"
#include "session/container/include/zidl/window_event_ipc_interface_code.h"

#include <axis_event.h>
#include <ipc_types.h>
#include <key_event.h>
#include <pointer_event.h>

#include "accessibility_element_info.h"
#include "parcel/accessibility_element_info_parcel.h"
#include "window_manager_hilog.h"

namespace OHOS::Rosen {
namespace {
constexpr HiviewDFX::HiLogLabel LABEL = {LOG_CORE, HILOG_DOMAIN_WINDOW, "WindowEventChannelStub"};
}

int WindowEventChannelStub::OnRemoteRequest(uint32_t code, MessageParcel& data,
    MessageParcel& reply, MessageOption& option)
{
    WLOGFD("Window event channel on remote request!, code: %{public}u", code);
    if (data.ReadInterfaceToken() != GetDescriptor()) {
        WLOGFE("Failed to check interface token!");
        return ERR_TRANSACTION_FAILED;
    }

    switch (code) {
        case static_cast<uint32_t>(WindowEventInterfaceCode::TRANS_ID_TRANSFER_KEY_EVENT):
            return HandleTransferKeyEvent(data, reply);
        case static_cast<uint32_t>(WindowEventInterfaceCode::TRANS_ID_TRANSFER_KEY_EVENT_ASYNC):
            return HandleTransferKeyEventAsync(data, reply);
        case static_cast<uint32_t>(WindowEventInterfaceCode::TRANS_ID_TRANSFER_POINTER_EVENT):
            return HandleTransferPointerEvent(data, reply);
        case static_cast<uint32_t>(WindowEventInterfaceCode::TRANS_ID_TRANSFER_FOCUS_ACTIVE_EVENT):
            return HandleTransferFocusActiveEvent(data, reply);
        case static_cast<uint32_t>(WindowEventInterfaceCode::TRANS_ID_TRANSFER_FOCUS_STATE_EVENT):
            return HandleTransferFocusStateEvent(data, reply);
        case static_cast<uint32_t>(WindowEventInterfaceCode::TRANS_ID_TRANSFER_BACKPRESSED_EVENT):
            return HandleTransferBackpressedEvent(data, reply);
        case static_cast<uint32_t>(WindowEventInterfaceCode::TRANS_ID_TRANSFER_ACCESSIBILITY_HOVER_EVENT):
            return HandleTransferAccessibilityHoverEvent(data, reply);
        case static_cast<uint32_t>(WindowEventInterfaceCode::TRANS_ID_TRANSFER_ACCESSIBILITY_CHILD_TREE_REGISTER):
            return HandleTransferAccessibilityChildTreeRegister(data, reply);
        case static_cast<uint32_t>(WindowEventInterfaceCode::TRANS_ID_TRANSFER_ACCESSIBILITY_CHILD_TREE_UNREGISTER):
            return HandleTransferAccessibilityChildTreeUnregister(data, reply);
        case static_cast<uint32_t>(WindowEventInterfaceCode::TRANS_ID_TRANSFER_ACCESSIBILITY_DUMP_CHILD_INFO):
            return HandleTransferAccessibilityDumpChildInfo(data, reply);
        default:
            WLOGFE("Failed to find function handler!");
            return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
    }
}

int WindowEventChannelStub::HandleTransferBackpressedEvent(MessageParcel& data, MessageParcel& reply)
{
    WLOGFD("TransferBackpressedEvent!");
    bool isConsumed = false;
    WSError errCode = TransferBackpressedEventForConsumed(isConsumed);

    reply.WriteBool(isConsumed);
    reply.WriteInt32(static_cast<int32_t>(errCode));
    return ERR_NONE;
}

int WindowEventChannelStub::HandleTransferKeyEvent(MessageParcel& data, MessageParcel& reply)
{
    WLOGFD("TransferKeyEvent!");
    auto keyEvent = MMI::KeyEvent::Create();
    if (keyEvent == nullptr) {
        WLOGFE("Failed to create key event!");
        return ERR_INVALID_DATA;
    }
    if (!keyEvent->ReadFromParcel(data)) {
        WLOGFE("Read Key Event failed");
        return ERR_INVALID_DATA;
    }
    bool isPreImeEvent = data.ReadBool();
    bool isConsumed = false;
    WSError errCode = TransferKeyEventForConsumed(keyEvent, isConsumed, isPreImeEvent);

    reply.WriteBool(isConsumed);
    reply.WriteInt32(static_cast<int32_t>(errCode));
    return ERR_NONE;
}

int WindowEventChannelStub::HandleTransferKeyEventAsync(MessageParcel& data, MessageParcel& reply)
{
    auto keyEvent = MMI::KeyEvent::Create();
    if (keyEvent == nullptr) {
        TLOGE(WmsLogTag::WMS_EVENT, "Failed to create key event!");
        return ERR_INVALID_DATA;
    }
    if (!keyEvent->ReadFromParcel(data)) {
        TLOGE(WmsLogTag::WMS_EVENT, "Read Key Event failed");
        return ERR_INVALID_DATA;
    }
    bool isPreImeEvent = false;
    if (!data.ReadBool(isPreImeEvent)) {
        TLOGE(WmsLogTag::WMS_EVENT, "Read Key Event failed");
        return ERR_INVALID_DATA;
    }
    sptr<IRemoteObject> listener = data.ReadRemoteObject();
    if (listener == nullptr) {
        TLOGE(WmsLogTag::WMS_EVENT, "ReadRemoteObject failed");
        return ERR_INVALID_DATA;
    }

    WSError errCode = TransferKeyEventForConsumedAsync(keyEvent, isPreImeEvent, listener);
    reply.WriteUint32(static_cast<uint32_t>(errCode));
    return ERR_NONE;
}

int WindowEventChannelStub::HandleTransferPointerEvent(MessageParcel& data, MessageParcel& reply)
{
    auto pointerEvent = MMI::PointerEvent::Create();
    if (pointerEvent == nullptr) {
        WLOGFE("Failed to create pointer event!");
        return ERR_INVALID_DATA;
    }
    if (!pointerEvent->ReadFromParcel(data)) {
        WLOGFE("Read Pointer Event failed");
        return ERR_INVALID_DATA;
    }
    TLOGD(WmsLogTag::WMS_EVENT, "id:%{public}d", pointerEvent->GetId());
    WSError errCode = TransferPointerEvent(pointerEvent);
    reply.WriteInt32(static_cast<int32_t>(errCode));
    return ERR_NONE;
}

int WindowEventChannelStub::HandleTransferFocusActiveEvent(MessageParcel& data, MessageParcel& reply)
{
    bool isFocusActive = data.ReadBool();
    WSError errCode = TransferFocusActiveEvent(isFocusActive);
    reply.WriteInt32(static_cast<int32_t>(errCode));
    return ERR_NONE;
}

int WindowEventChannelStub::HandleTransferFocusStateEvent(MessageParcel& data, MessageParcel& reply)
{
    bool focusState = data.ReadBool();
    WSError errCode = TransferFocusState(focusState);
    reply.WriteInt32(static_cast<int32_t>(errCode));
    return ERR_NONE;
}

int WindowEventChannelStub::HandleTransferAccessibilityHoverEvent(MessageParcel& data, MessageParcel& reply)
{
    float pointX = 0;
    float pointY = 0;
    int32_t sourceType = 0;
    int32_t eventType = 0;
    int64_t timeMs = 0;
    if (!data.ReadFloat(pointX) ||
        !data.ReadFloat(pointY) ||
        !data.ReadInt32(sourceType) ||
        !data.ReadInt32(eventType) ||
        !data.ReadInt64(timeMs)) {
        WLOGFE("Read HandleTransferAccessibilityHoverEvent data failed!");
        return ERR_INVALID_DATA;
    };
    WSError errCode = TransferAccessibilityHoverEvent(pointX, pointY, sourceType, eventType, timeMs);
    reply.WriteInt32(static_cast<int32_t>(errCode));
    return ERR_NONE;
}

int32_t WindowEventChannelStub::HandleTransferAccessibilityChildTreeRegister(MessageParcel& data, MessageParcel& reply)
{
    uint32_t windowId = 0;
    int32_t treeId = -1;
    int64_t accessibilityId = -1;
    if (!data.ReadUint32(windowId) ||
        !data.ReadInt32(treeId) ||
        !data.ReadInt64(accessibilityId)) {
        TLOGE(WmsLogTag::WMS_UIEXT, "Read HandleTransferAccessibilityChildTreeRegister data failed!");
        return ERR_INVALID_DATA;
    }
    WSError errCode = TransferAccessibilityChildTreeRegister(windowId, treeId, accessibilityId);
    reply.WriteInt32(static_cast<int32_t>(errCode));
    return ERR_NONE;
}

int32_t WindowEventChannelStub::HandleTransferAccessibilityChildTreeUnregister(
    MessageParcel& data, MessageParcel& reply)
{
    WSError errCode = TransferAccessibilityChildTreeUnregister();
    reply.WriteInt32(static_cast<int32_t>(errCode));
    return ERR_NONE;
}

int32_t WindowEventChannelStub::HandleTransferAccessibilityDumpChildInfo(MessageParcel& data, MessageParcel& reply)
{
    std::vector<std::string> params;
    if (!data.ReadStringVector(&params)) {
        TLOGE(WmsLogTag::WMS_UIEXT, "Read HandleTransferAccessibilityDumpChildInfo data failed!");
        return ERR_INVALID_VALUE;
    }
    std::vector<std::string> info;
    TransferAccessibilityDumpChildInfo(params, info);
    reply.WriteStringVector(info);
    return ERR_NONE;
}
}
