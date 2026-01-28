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

#include "zidl/window_proxy.h"
#include <ipc_types.h>
#include <key_event.h>
#include "pointer_event.h"
#include "message_option.h"
#include "window_manager_hilog.h"
#include "wm_common.h"

namespace OHOS {
namespace Rosen {
namespace {
constexpr HiviewDFX::HiLogLabel LABEL = {LOG_CORE, HILOG_DOMAIN_WINDOW, "WindowProxy"};
}

WMError WindowProxy::UpdateWindowRect(const struct Rect& rect, bool decoStatus, WindowSizeChangeReason reason,
    const std::shared_ptr<RSTransaction>& rsTransaction)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        WLOGFE("WriteInterfaceToken failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    if (!(data.WriteInt32(rect.posX_) && data.WriteInt32(rect.posY_) &&
        data.WriteUint32(rect.width_) && data.WriteUint32(rect.height_))) {
        WLOGFE("Write WindowRect failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    if (!data.WriteBool(decoStatus)) {
        WLOGFE("Write deco status failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    if (!data.WriteUint32(static_cast<uint32_t>(reason))) {
        WLOGFE("Write WindowSizeChangeReason failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }

    bool hasRSTransaction = rsTransaction != nullptr;
    if (!data.WriteBool(hasRSTransaction)) {
        WLOGFE("Write transaction sync Id failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    if (hasRSTransaction) {
        if (!data.WriteParcelable(rsTransaction.get())) {
            WLOGFE("Write transaction sync Id failed");
            return WMError::WM_ERROR_IPC_FAILED;
        }
    }

    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        WLOGFE("remote is null");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    if (remote->SendRequest(static_cast<uint32_t>(WindowMessage::TRANS_ID_UPDATE_WINDOW_RECT),
        data, reply, option) != ERR_NONE) {
        WLOGFE("SendRequest failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    return WMError::WM_OK;
}

WMError WindowProxy::UpdateWindowMode(WindowMode mode)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        WLOGFE("WriteInterfaceToken failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    if (!data.WriteUint32(static_cast<uint32_t>(mode))) {
        WLOGFE("Write WindowMode failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }

    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        WLOGFE("remote is null");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    if (remote->SendRequest(static_cast<uint32_t>(WindowMessage::TRANS_ID_UPDATE_WINDOW_MODE),
        data, reply, option) != ERR_NONE) {
        WLOGFE("SendRequest failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    return WMError::WM_OK;
}

WMError WindowProxy::UpdateWindowModeSupportType(uint32_t windowModeSupportType)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        WLOGFE("WriteInterfaceToken failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    if (!data.WriteUint32(windowModeSupportType)) {
        WLOGFE("Write WindowMode failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }

    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        WLOGFE("remote is null");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    if (remote->SendRequest(static_cast<uint32_t>(WindowMessage::TRANS_ID_UPDATE_MODE_SUPPORT_INFO),
        data, reply, option) != ERR_NONE) {
        WLOGFE("SendRequest failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    return WMError::WM_OK;
}

WMError WindowProxy::UpdateFocusStatus(bool focused)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        WLOGFE("WriteInterfaceToken failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    if (!data.WriteBool(focused)) {
        WLOGFE("Write Focus failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }

    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        WLOGFE("remote is null");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    if (remote->SendRequest(static_cast<uint32_t>(WindowMessage::TRANS_ID_UPDATE_FOCUS_STATUS),
        data, reply, option) != ERR_NONE) {
        WLOGFE("SendRequest failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    return WMError::WM_OK;
}

WMError WindowProxy::UpdateAvoidArea(const sptr<AvoidArea>& avoidArea, AvoidAreaType type)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        WLOGFE("WriteInterfaceToken failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    if (!data.WriteStrongParcelable(avoidArea)) {
        WLOGFE("Write WindowRect failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    if (!data.WriteUint32(static_cast<uint32_t>(type))) {
        WLOGFE("Write AvoidAreaType failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        WLOGFE("remote is null");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    if (remote->SendRequest(static_cast<uint32_t>(WindowMessage::TRANS_ID_UPDATE_AVOID_AREA),
        data, reply, option) != ERR_NONE) {
        WLOGFE("SendRequest failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    return WMError::WM_OK;
}

WMError WindowProxy::UpdateWindowState(WindowState state)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        WLOGFE("WriteInterfaceToken failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }

    if (!data.WriteUint32(static_cast<uint32_t>(state))) {
        WLOGFE("Write isStopped");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        WLOGFE("remote is null");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    if (remote->SendRequest(static_cast<uint32_t>(WindowMessage::TRANS_ID_UPDATE_WINDOW_STATE),
        data, reply, option) != ERR_NONE) {
        WLOGFE("SendRequest failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    return WMError::WM_OK;
}

WMError WindowProxy::UpdateWindowDragInfo(const PointInfo& point, DragEvent event)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        WLOGFE("WriteInterfaceToken failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    if (!(data.WriteInt32(point.x) and data.WriteInt32(point.y))) {
        WLOGFE("Write pos failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    if (!data.WriteInt32(static_cast<uint32_t>(event))) {
        WLOGFE("Write event failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }

    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        WLOGFE("remote is null");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    if (remote->SendRequest(static_cast<uint32_t>(WindowMessage::TRANS_ID_UPDATE_DRAG_EVENT),
        data, reply, option) != ERR_NONE) {
        WLOGFE("SendRequest TRANS_ID_UPDATE_DRAG_EVENT failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    return WMError::WM_OK;
}

WMError WindowProxy::UpdateDisplayId(DisplayId from, DisplayId to)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        WLOGFE("WriteInterfaceToken failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    if (!(data.WriteUint64(from) and data.WriteUint64(to))) {
        WLOGFE("Write displayid failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        WLOGFE("remote is null");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    if (remote->SendRequest(static_cast<uint32_t>(WindowMessage::TRANS_ID_UPDATE_DISPLAY_ID),
        data, reply, option) != ERR_NONE) {
        WLOGFE("SendRequest TRANS_ID_UPDATE_DISPLAY_ID failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    return WMError::WM_OK;
}

WMError WindowProxy::UpdateOccupiedAreaChangeInfo(const sptr<OccupiedAreaChangeInfo>& info,
    const std::map<AvoidAreaType, AvoidArea>& avoidAreas,
    const std::shared_ptr<RSTransaction>& rsTransaction)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        WLOGFE("WriteInterfaceToken failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    if (!data.WriteParcelable(info)) {
        WLOGFE("Write OccupiedAreaChangeInfo failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }

    if (!data.WriteUint32(avoidAreas.size())) {
        TLOGE(WmsLogTag::WMS_KEYBOARD, "Write avoid area size failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    for (const auto& [type, avoidArea] : avoidAreas) {
        if (!data.WriteUint32(static_cast<uint32_t>(type))) {
            TLOGE(WmsLogTag::WMS_KEYBOARD, "Write avoid area type failed");
            return WMError::WM_ERROR_IPC_FAILED;
        }
        if (!data.WriteParcelable(&avoidArea)) {
            TLOGE(WmsLogTag::WMS_KEYBOARD, "Write avoid area failed");
            return WMError::WM_ERROR_IPC_FAILED;
        }
    }
    
    bool hasRSTransaction = rsTransaction != nullptr;
    if (!data.WriteBool(hasRSTransaction)) {
        WLOGFE("Write transaction sync Id failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    if (hasRSTransaction) {
        if (!data.WriteParcelable(rsTransaction.get())) {
            WLOGFE("Write transaction sync Id failed");
            return WMError::WM_ERROR_IPC_FAILED;
        }
    }

    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        WLOGFE("remote is null");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    if (remote->SendRequest(static_cast<uint32_t>(WindowMessage::TRANS_ID_UPDATE_OCCUPIED_AREA),
        data, reply, option) != ERR_NONE) {
        WLOGFE("SendRequest failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    return WMError::WM_OK;
}

WMError WindowProxy::UpdateOccupiedAreaAndRect(const sptr<OccupiedAreaChangeInfo>& info, const Rect& rect,
    const std::map<AvoidAreaType, AvoidArea>& avoidAreas,
    const std::shared_ptr<RSTransaction>& rsTransaction)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        WLOGFE("WriteInterfaceToken failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    if (!data.WriteParcelable(info)) {
        WLOGFE("Write OccupiedAreaChangeInfo failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }

    if (!(data.WriteInt32(rect.posX_) && data.WriteInt32(rect.posY_) &&
        data.WriteUint32(rect.width_) && data.WriteUint32(rect.height_))) {
        WLOGFE("Write WindowRect failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }

    if (!(data.WriteUint32(avoidAreas.size()))) {
        TLOGE(WmsLogTag::WMS_KEYBOARD, "Write avoid area size failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    
    for (const auto& [type, avoidArea] : avoidAreas) {
        if (!data.WriteUint32(static_cast<uint32_t>(type))) {
            TLOGE(WmsLogTag::WMS_KEYBOARD, "Write avoid area type failed");
            return WMError::WM_ERROR_IPC_FAILED;
        }
        if (!data.WriteParcelable(&avoidArea)) {
            TLOGE(WmsLogTag::WMS_KEYBOARD, "Write avoid area failed");
            return WMError::WM_ERROR_IPC_FAILED;
        }
    }

    bool hasRSTransaction = rsTransaction != nullptr;
    if (!data.WriteBool(hasRSTransaction)) {
        WLOGFE("Write transaction sync Id failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    if (hasRSTransaction) {
        if (!data.WriteParcelable(rsTransaction.get())) {
            WLOGFE("Write transaction sync Id failed");
            return WMError::WM_ERROR_IPC_FAILED;
        }
    }

    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        WLOGFE("remote is null");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    if (remote->SendRequest(static_cast<uint32_t>(WindowMessage::TRANS_ID_UPDATE_OCCUPIED_AREA_AND_RECT),
        data, reply, option) != ERR_NONE) {
        WLOGFE("SendRequest failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    return WMError::WM_OK;
}

WMError WindowProxy::UpdateActiveStatus(bool isActive)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        WLOGFE("WriteInterfaceToken failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    if (!data.WriteBool(isActive)) {
        WLOGFE("Write Focus failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }

    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        WLOGFE("remote is null");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    if (remote->SendRequest(static_cast<uint32_t>(WindowMessage::TRANS_ID_UPDATE_ACTIVE_STATUS),
        data, reply, option) != ERR_NONE) {
        WLOGFE("SendRequest failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    return WMError::WM_OK;
}

sptr<WindowProperty> WindowProxy::GetWindowProperty()
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        WLOGFE("WriteInterfaceToken failed");
        return nullptr;
    }
    uint32_t requestCode = static_cast<uint32_t>(WindowMessage::TRANS_ID_GET_WINDOW_PROPERTY);
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        WLOGFE("remote is null");
        return nullptr;
    }
    if (remote->SendRequest(requestCode, data, reply, option) != ERR_NONE) {
        WLOGFE("SendRequest failed");
        return nullptr;
    }
    sptr<WindowProperty> property = reply.ReadParcelable<WindowProperty>();
    return property;
}

WMError WindowProxy::NotifyTouchOutside()
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        WLOGFE("WriteInterfaceToken failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }

    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        WLOGFE("remote is null");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    if (remote->SendRequest(static_cast<uint32_t>(WindowMessage::TRANS_ID_NOTIFY_OUTSIDE_PRESSED),
        data, reply, option) != ERR_NONE) {
        WLOGFE("SendRequest failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    return WMError::WM_OK;
}

WMError WindowProxy::NotifyScreenshot()
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        WLOGFE("WriteInterfaceToken failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }

    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        WLOGFE("remote is null");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    if (remote->SendRequest(static_cast<uint32_t>(WindowMessage::TRANS_ID_NOTIFY_SCREEN_SHOT),
        data, reply, option) != ERR_NONE) {
        WLOGFE("SendRequest failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    return WMError::WM_OK;
}

WMError WindowProxy::NotifyScreenshotAppEvent(ScreenshotEventType type)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "WriteInterfaceToken failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    if (!data.WriteInt32(static_cast<int32_t>(type))) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "Write screenshot event type failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "remote is null");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    if (remote->SendRequest(static_cast<uint32_t>(WindowMessage::TRANS_ID_NOTIFY_SCREEN_SHOT_APP_EVENT),
        data, reply, option) != ERR_NONE) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "SendRequest failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    return WMError::WM_OK;
}

WMError WindowProxy::DumpInfo(const std::vector<std::string>& params)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        WLOGFE("WriteInterfaceToken failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    if (!data.WriteStringVector(params)) {
        WLOGFE("Write params failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        WLOGFE("remote is null");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    if (remote->SendRequest(static_cast<uint32_t>(WindowMessage::TRANS_ID_DUMP_INFO),
        data, reply, option) != ERR_NONE) {
        WLOGFE("SendRequest failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    return WMError::WM_OK;
}

WMError WindowProxy::UpdateZoomTransform(const Transform& trans, bool isDisplayZoomOn)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        WLOGFE("WriteInterfaceToken failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    if (!trans.Marshalling(data)) {
        WLOGFE("Write params failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    if (!data.WriteBool(isDisplayZoomOn)) {
        WLOGFE("Write params failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        WLOGFE("remote is null");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    if (remote->SendRequest(static_cast<uint32_t>(WindowMessage::TRANS_ID_UPDATE_ZOOM_TRANSFORM),
        data, reply, option) != ERR_NONE) {
        WLOGFE("SendRequest failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    return WMError::WM_OK;
}

WMError WindowProxy::NotifyDestroy(void)
{
    MessageParcel data;
    MessageParcel replay;
    MessageOption option(MessageOption::TF_ASYNC);
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        WLOGFE("WriteInterfaceToken failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }

    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        WLOGFE("remote is null");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    if (remote->SendRequest(static_cast<uint32_t>(WindowMessage::TRANS_ID_NOTIFY_DESTROY),
        data, replay, option) != ERR_NONE) {
        WLOGFE("SendRequest failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    return WMError::WM_OK;
}

WMError WindowProxy::NotifyForeground(void)
{
    MessageParcel data;
    MessageParcel replay;
    MessageOption option(MessageOption::TF_ASYNC);
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        WLOGFE("WriteInterfaceToken failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }

    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        WLOGFE("remote is null");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    if (remote->SendRequest(static_cast<uint32_t>(WindowMessage::TRANS_ID_NOTIFY_FOREGROUND),
        data, replay, option) != ERR_NONE) {
        WLOGFE("SendRequest failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    return WMError::WM_OK;
}

WMError WindowProxy::NotifyBackground(void)
{
    MessageParcel data;
    MessageParcel replay;
    MessageOption option(MessageOption::TF_ASYNC);
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        WLOGFE("WriteInterfaceToken failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }

    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        WLOGFE("remote is null");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    if (remote->SendRequest(static_cast<uint32_t>(WindowMessage::TRANS_ID_NOTIFY_BACKGROUND),
        data, replay, option) != ERR_NONE) {
        WLOGFE("SendRequest failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    return WMError::WM_OK;
}

WMError WindowProxy::NotifyWindowClientPointUp(const std::shared_ptr<MMI::PointerEvent>& pointerEvent)
{
    if (!pointerEvent) {
        WLOGFE("pointerEvent is nullptr");
        return WMError::WM_ERROR_NULLPTR;
    }
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        WLOGFE("WriteInterfaceToken failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    if (!pointerEvent->WriteToParcel(data)) {
        WLOGFE("Failed to write point event");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        WLOGFE("remote is null");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    if (remote->SendRequest(static_cast<uint32_t>(WindowMessage::TRANS_ID_NOTIFY_CLIENT_POINT_UP),
        data, reply, option) != ERR_NONE) {
        WLOGFE("SendRequest failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    return WMError::WM_OK;
}

WMError WindowProxy::RestoreSplitWindowMode(uint32_t mode)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        WLOGFE("WriteInterfaceToken failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    if (!data.WriteUint32(mode)) {
        WLOGFE("mode failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    uint32_t requestCode = static_cast<uint32_t>(WindowMessage::TRANS_ID_RESTORE_SPLIT_WINDOW_MODE);
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        WLOGFE("remote is null");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    if (remote->SendRequest(requestCode, data, reply, option) != ERR_NONE) {
        WLOGFE("SendRequest failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }
    return WMError::WM_OK;
}

void WindowProxy::ConsumeKeyEvent(std::shared_ptr<MMI::KeyEvent> event)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        WLOGFE("WriteInterfaceToken failed");
        return;
    }
    if (!event->WriteToParcel(data)) {
        WLOGFE("Write point event failed");
        return;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        WLOGFE("remote is null");
        return;
    }
    if (remote->SendRequest(static_cast<uint32_t>(WindowMessage::TRANS_ID_CONSUME_KEY_EVENT),
        data, reply, option) != ERR_NONE) {
        WLOGFE("SendRequest failed");
        return;
    }
}

void WindowProxy::NotifyForegroundInteractiveStatus(bool interactive)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        WLOGFE("WriteInterfaceToken failed");
        return;
    }
    if (!data.WriteBool(interactive)) {
        WLOGFE("Write Focus failed");
        return;
    }

    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        WLOGFE("remote is null");
        return;
    }
    if (remote->SendRequest(static_cast<uint32_t>(WindowMessage::TRANS_ID_NOTIFY_FOREGROUND_INTERACTIVE_STATUS),
        data, reply, option) != ERR_NONE) {
        WLOGFE("SendRequest failed");
    }
}

void WindowProxy::NotifyMMIServiceOnline(uint32_t winId)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        WLOGFE("WriteInterfaceToken failed");
        return;
    }
    if (!data.WriteUint32(winId)) {
        WLOGFE("Write windowId failed");
        return;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        WLOGFE("remote is null");
        return;
    }
    if (remote->SendRequest(static_cast<uint32_t>(WindowMessage::TRANS_ID_NOTIFY_MMI_SERVICE_ONLINE),
        data, reply, option) != ERR_NONE) {
        WLOGFE("SendRequest failed");
        return;
    }
}

} // namespace Rosen
} // namespace OHOS

