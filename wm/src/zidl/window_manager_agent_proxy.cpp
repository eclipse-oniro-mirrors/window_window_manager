/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#include "zidl/window_manager_agent_proxy.h"
#include <ipc_types.h>
#include "marshalling_helper.h"
#include "window_manager_hilog.h"
#include "wm_common.h"

namespace OHOS {
namespace Rosen {
namespace {
    constexpr HiviewDFX::HiLogLabel LABEL = {LOG_CORE, HILOG_DOMAIN_WINDOW, "WindowManagerAgentProxy"};
}

void WindowManagerAgentProxy::UpdateFocusChangeInfo(const sptr<FocusChangeInfo>& focusChangeInfo, bool focused)
{
    MessageParcel data;
    if (focusChangeInfo == nullptr) {
        WLOGFE("Invalid focus change info");
        return;
    }

    if (!data.WriteInterfaceToken(GetDescriptor())) {
        WLOGFE("WriteInterfaceToken failed");
        return;
    }

    if (!data.WriteParcelable(focusChangeInfo)) {
        WLOGFE("Write displayId failed");
        return;
    }

    if (!data.WriteBool(focused)) {
        WLOGFE("Write Focus failed");
        return;
    }
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        WLOGFE("remote is null");
        return;
    }
    if (remote->SendRequest(static_cast<uint32_t>(WindowManagerAgentMsg::TRANS_ID_UPDATE_FOCUS),
        data, reply, option) != ERR_NONE) {
        WLOGFE("SendRequest failed");
    }
}

void WindowManagerAgentProxy::UpdateWindowModeTypeInfo(WindowModeType type)
{
    MessageParcel data;
    
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::WMS_MAIN, "WriteInterfaceToken failed");
        return;
    }

    if (!data.WriteUint8(static_cast<uint8_t>(type))) {
        TLOGE(WmsLogTag::WMS_MAIN, "Write displayId failed");
        return;
    }

    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        WLOGFE("remote is null");
        return;
    }
    if (remote->SendRequest(static_cast<uint32_t>(WindowManagerAgentMsg::TRANS_ID_UPDATE_WINDOW_MODE_TYPE),
        data, reply, option) != ERR_NONE) {
        TLOGE(WmsLogTag::WMS_MAIN, "SendRequest failed");
    }
}

void WindowManagerAgentProxy::UpdateSystemBarRegionTints(DisplayId displayId, const SystemBarRegionTints& tints)
{
    MessageParcel data;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        WLOGFE("WriteInterfaceToken failed");
        return;
    }

    if (!data.WriteUint64(displayId)) {
        WLOGFE("Write displayId failed");
        return;
    }
    bool res = MarshallingHelper::MarshallingVectorObj<SystemBarRegionTint>(data, tints,
        [](Parcel& parcel, const SystemBarRegionTint& tint) {
            return parcel.WriteUint32(static_cast<uint32_t>(tint.type_)) && parcel.WriteBool(tint.prop_.enable_) &&
                parcel.WriteUint32(tint.prop_.backgroundColor_) && parcel.WriteUint32(tint.prop_.contentColor_) &&
                parcel.WriteInt32(tint.region_.posX_) && parcel.WriteInt32(tint.region_.posY_) &&
                parcel.WriteInt32(tint.region_.width_) && parcel.WriteInt32(tint.region_.height_);
        }
    );
    if (!res) {
        WLOGFE("Write SystemBarRegionTint failed");
        return;
    }
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        WLOGFE("remote is null");
        return;
    }
    if (remote->SendRequest(static_cast<uint32_t>(WindowManagerAgentMsg::TRANS_ID_UPDATE_SYSTEM_BAR_PROPS),
        data, reply, option) != ERR_NONE) {
        WLOGFE("SendRequest failed");
    }
}

void WindowManagerAgentProxy::NotifyAccessibilityWindowInfo(const std::vector<sptr<AccessibilityWindowInfo>>& infos,
    WindowUpdateType type)
{
    MessageParcel data;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        WLOGFE("WriteInterfaceToken failed");
        return;
    }

    if (!MarshallingHelper::MarshallingVectorParcelableObj<AccessibilityWindowInfo>(data, infos)) {
        WLOGFE("Write accessibility window infos failed");
        return;
    }

    if (!data.WriteUint32(static_cast<uint32_t>(type))) {
        WLOGFE("Write windowUpdateType failed");
        return;
    }
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        WLOGFE("remote is null");
        return;
    }
    if (remote->SendRequest(static_cast<uint32_t>(WindowManagerAgentMsg::TRANS_ID_UPDATE_WINDOW_STATUS),
        data, reply, option) != ERR_NONE) {
        WLOGFE("SendRequest failed");
    }
}

void WindowManagerAgentProxy::UpdateWindowVisibilityInfo(
    const std::vector<sptr<WindowVisibilityInfo>>& visibilityInfos)
{
    MessageParcel data;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        WLOGFE("WriteInterfaceToken failed");
        return;
    }
    if (!data.WriteUint32(static_cast<uint32_t>(visibilityInfos.size()))) {
        WLOGFE("write windowVisibilityInfos size failed");
        return;
    }
    for (auto& info : visibilityInfos) {
        if (!data.WriteParcelable(info)) {
            WLOGFE("Write windowVisibilityInfo failed");
            return;
        }
    }
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        WLOGFE("remote is null");
        return;
    }
    if (remote->SendRequest(static_cast<uint32_t>(WindowManagerAgentMsg::TRANS_ID_UPDATE_WINDOW_VISIBILITY),
        data, reply, option) != ERR_NONE) {
        WLOGFE("SendRequest failed");
    }
}

void WindowManagerAgentProxy::UpdateWindowDrawingContentInfo(
    const std::vector<sptr<WindowDrawingContentInfo>>& windowDrawingContentInfos)
{
    MessageParcel data;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        WLOGFE("WriteInterfaceToken failed");
        return;
    }
    if (!data.WriteUint32(static_cast<uint32_t>(windowDrawingContentInfos.size()))) {
        WLOGFE("write windowDrawingContentInfos size failed");
        return;
    }
    for (auto& info : windowDrawingContentInfos) {
        if (!data.WriteParcelable(info)) {
            WLOGFE("Write windowDrawingContentInfos failed");
            return;
        }
    }
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        WLOGFE("remote is null");
        return;
    }
    if (remote->SendRequest(
        static_cast<uint32_t>(WindowManagerAgentMsg::TRANS_ID_UPDATE_WINDOW_DRAWING_STATE), data, reply,
        option) != ERR_NONE) {
        WLOGFE("SendRequest failed");
    }
}

void WindowManagerAgentProxy::UpdateCameraFloatWindowStatus(uint32_t accessTokenId, bool isShowing)
{
    MessageParcel data;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        WLOGFE("WriteInterfaceToken failed");
        return;
    }

    if (!data.WriteUint32(accessTokenId)) {
        WLOGFE("Write accessTokenId failed");
        return;
    }

    if (!data.WriteBool(isShowing)) {
        WLOGFE("Write is showing status failed");
        return;
    }
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        WLOGFE("remote is null");
        return;
    }
    if (remote->SendRequest(static_cast<uint32_t>(WindowManagerAgentMsg::TRANS_ID_UPDATE_CAMERA_FLOAT),
        data, reply, option) != ERR_NONE) {
        WLOGFE("SendRequest failed");
    }
}

void WindowManagerAgentProxy::NotifyWaterMarkFlagChangedResult(bool showWaterMark)
{
    MessageParcel data;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        WLOGFE("WriteInterfaceToken failed");
        return;
    }

    if (!data.WriteBool(showWaterMark)) {
        WLOGFE("Write is showing status failed");
        return;
    }
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        WLOGFE("remote is null");
        return;
    }
    if (remote->SendRequest(static_cast<uint32_t>(WindowManagerAgentMsg::TRANS_ID_UPDATE_WATER_MARK_FLAG),
        data, reply, option) != ERR_NONE) {
        WLOGFE("SendRequest failed");
    }
}

void WindowManagerAgentProxy::UpdateVisibleWindowNum(
    const std::vector<VisibleWindowNumInfo>& visibleWindowNumInfo)
{
    MessageParcel data;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        WLOGFE("WriteInterfaceToken failed");
        return;
    }

    bool res = MarshallingHelper::MarshallingVectorObj<VisibleWindowNumInfo>(data, visibleWindowNumInfo,
        [](Parcel& parcel, const VisibleWindowNumInfo& num) {
            return parcel.WriteUint32(num.displayId) && parcel.WriteUint32(num.visibleWindowNum);
        }
    );
    if (!res) {
        WLOGFE("Write VisibleWindowNumInfo failed");
        return;
    }
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        WLOGFE("remote is null");
        return;
    }
    if (remote->SendRequest(static_cast<uint32_t>(WindowManagerAgentMsg::TRANS_ID_UPDATE_VISIBLE_WINDOW_NUM),
        data, reply, option) != ERR_NONE) {
        WLOGFE("SendRequest failed");
    }
}

void WindowManagerAgentProxy::NotifyGestureNavigationEnabledResult(bool enable)
{
    MessageParcel data;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        WLOGFE("WriteInterfaceToken failed");
        return;
    }

    if (!data.WriteBool(enable)) {
        WLOGFE("Write is showing status failed");
        return;
    }
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        WLOGFE("remote is null");
        return;
    }
    if (remote->SendRequest(static_cast<uint32_t>(WindowManagerAgentMsg::TRANS_ID_UPDATE_GESTURE_NAVIGATION_ENABLED),
        data, reply, option) != ERR_NONE) {
        WLOGFE("SendRequest failed");
    }
}

void WindowManagerAgentProxy::UpdateCameraWindowStatus(uint32_t accessTokenId, bool isShowing)
{
    MessageParcel data;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::WMS_MAIN, "WriteInterfaceToken failed");
        return;
    }
    if (!data.WriteUint32(accessTokenId)) {
        TLOGE(WmsLogTag::WMS_MAIN, "Write accessTokenId failed");
        return;
    }
    if (!data.WriteBool(isShowing)) {
        TLOGE(WmsLogTag::WMS_MAIN, "Write isShowing status failed");
        return;
    }
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::WMS_MAIN, "remote is null");
        return;
    }
    if (remote->SendRequest(static_cast<uint32_t>(WindowManagerAgentMsg::TRANS_ID_UPDATE_CAMERA_WINDOW_STATUS),
        data, reply, option) != ERR_NONE) {
        TLOGE(WmsLogTag::WMS_MAIN, "SendRequest failed");
    }
}

void WindowManagerAgentProxy::NotifyWindowStyleChange(WindowStyleType type)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::WMS_MAIN, "WriteInterfaceToken failed");
        return;
    }
    if (!data.WriteUint8(static_cast<uint8_t>(type))) {
        TLOGE(WmsLogTag::WMS_MAIN, "Write displayId failed");
        return;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::WMS_MAIN, "remote is null");
        return;
    }
    if (remote->SendRequest(static_cast<uint32_t>(WindowManagerAgentMsg::TRANS_ID_UPDATE_WINDOW_STYLE_TYPE),
        data, reply, option) != ERR_NONE) {
        TLOGE(WmsLogTag::WMS_MAIN, "SendRequest failed");
    }
}

} // namespace Rosen
} // namespace OHOS

