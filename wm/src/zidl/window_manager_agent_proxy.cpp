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

void WindowManagerAgentProxy::UpdateDisplayGroupInfo(DisplayGroupId displayGroupId, DisplayId displayId, bool isAdd)
{
    MessageParcel data;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::WMS_FOCUS, "WriteInterfaceToken failed");
        return;
    }
    if (!data.WriteUint64(displayGroupId)) {
        TLOGE(WmsLogTag::WMS_FOCUS, "Write displayGroupId failed");
        return;
    }
    if (!data.WriteUint64(displayId)) {
        TLOGE(WmsLogTag::WMS_FOCUS, "Write displayId failed");
        return;
    }
    if (!data.WriteBool(isAdd)) {
        TLOGE(WmsLogTag::WMS_FOCUS, "Write isAdd failed");
        return;
    }
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::WMS_FOCUS, "remote is null");
        return;
    }
    if (remote->SendRequest(static_cast<uint32_t>(WindowManagerAgentMsg::TRANS_ID_NOTIFY_DISPLAY_GROUP_INFO_CHANGE),
        data, reply, option) != ERR_NONE) {
        TLOGE(WmsLogTag::WMS_FOCUS, "SendRequest failed");
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

    if (!data.WriteInt32(static_cast<int32_t>(type))) {
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

void WindowManagerAgentProxy::NotifyWindowSystemBarPropertyChange(
    WindowType type, const SystemBarProperty& systemBarProperty)
{
    MessageParcel data;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::WMS_IMMS, "WriteInterfaceToken failed");
        return;
    }
    if (!data.WriteInt32(static_cast<int32_t>(type))) {
        TLOGE(WmsLogTag::WMS_IMMS, "Write type failed");
        return;
    }
    if (!data.WriteBool(systemBarProperty.enable_)) {
        TLOGE(WmsLogTag::WMS_IMMS, "Write enable failed");
        return;
    }
    if (!data.WriteUint32(systemBarProperty.backgroundColor_)) {
        TLOGE(WmsLogTag::WMS_IMMS, "Write backgroundColor failed");
        return;
    }
    if (!data.WriteUint32(systemBarProperty.contentColor_)) {
        TLOGE(WmsLogTag::WMS_IMMS, "Write contentColor failed");
        return;
    }
    if (!data.WriteBool(systemBarProperty.enableAnimation_)) {
        TLOGE(WmsLogTag::WMS_IMMS, "Write enableAnimation failed");
        return;
    }
    if (!data.WriteUint32(static_cast<uint32_t>(systemBarProperty.settingFlag_))) {
        TLOGE(WmsLogTag::WMS_IMMS, "Write settingFlag failed");
        return;
    }
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::WMS_IMMS, "remote is null");
        return;
    }
    if (remote->SendRequest(static_cast<uint32_t>(
        WindowManagerAgentMsg::TRANS_ID_NOTIFY_WINDOW_SYSTEM_BAR_PROPERTY_CHANGE), data, reply, option) != ERR_NONE) {
        TLOGE(WmsLogTag::WMS_IMMS, "sendRequest failed");
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

void WindowManagerAgentProxy::NotifyCallingWindowDisplayChanged(const CallingWindowInfo& callingWindowInfo)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::WMS_KEYBOARD, "WriteInterfaceToken failed");
        return;
    }
    if (!data.WriteParcelable(&callingWindowInfo)) {
        TLOGE(WmsLogTag::WMS_KEYBOARD, "Write callingWindowInfo failed");
        return;
    }

    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::WMS_KEYBOARD, "remote is null");
        return;
    }
    int sendCode = remote->SendRequest(
        static_cast<uint32_t>(WindowManagerAgentMsg::TRANS_ID_NOTIFY_CALLING_DISPLAY_CHANGE),
        data, reply, option);
    if (sendCode != ERR_NONE) {
        TLOGE(WmsLogTag::WMS_KEYBOARD, "SendRequest calling display info change failed, code: %{public}d", sendCode);
    }
}

void WindowManagerAgentProxy::NotifyWindowPidVisibilityChanged(const sptr<WindowPidVisibilityInfo>& info)
{
    MessageParcel data;
    if (info == nullptr) {
        TLOGE(WmsLogTag::WMS_LIFE, "Invalid window pid visibility info");
        return;
    }

    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::WMS_LIFE, "WriteInterfaceToken failed");
        return;
    }

    if (!data.WriteParcelable(info)) {
        TLOGE(WmsLogTag::WMS_LIFE, "Write windowPidVisibilityInfo failed");
        return;
    }

    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::WMS_LIFE, "remote is null");
        return;
    }
    if (remote->SendRequest(static_cast<uint32_t>(WindowManagerAgentMsg::TRANS_ID_NOTIFY_WINDOW_PID_VISIBILITY),
        data, reply, option) != ERR_NONE) {
        TLOGE(WmsLogTag::WMS_LIFE, "SendRequest failed");
    }
}

void WindowManagerAgentProxy::UpdatePiPWindowStateChanged(const std::string& bundleName, bool isForeground)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::WMS_PIP, "WriteInterfaceToken failed");
        return;
    }
    if (!data.WriteString(bundleName)) {
        TLOGE(WmsLogTag::WMS_PIP, "Write bundleName failed");
        return;
    }
    if (!data.WriteBool(isForeground)) {
        TLOGE(WmsLogTag::WMS_PIP, "Write state failed");
        return;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::WMS_PIP, "remote is null");
        return;
    }
    if (remote->SendRequest(static_cast<uint32_t>(WindowManagerAgentMsg::TRANS_ID_UPDATE_PIP_WINDOW_STATE_CHANGED),
        data, reply, option) != ERR_NONE) {
        TLOGE(WmsLogTag::WMS_PIP, "SendRequest failed");
    }
}

void WindowManagerAgentProxy::NotifyWindowPropertyChange(
    uint32_t propertyDirtyFlags, const WindowInfoList& windowInfoList)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "WriteInterfaceToken failed");
        return;
    }
    if (!data.WriteUint32(propertyDirtyFlags)) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "Write propertyDirtyFlags failed");
        return;
    }
    if (!data.WriteUint32(static_cast<uint32_t>(windowInfoList.size()))) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "Write windowInfoList failed");
        return;
    }

    for (const auto& windowInfo : windowInfoList) {
        if (!data.WriteUint32(static_cast<uint32_t>(windowInfo.size()))) {
            TLOGE(WmsLogTag::WMS_ATTRIBUTE, "Write windowInfo failed");
            return;
        }

        for (const auto& pair : windowInfo) {
            if (!WriteWindowChangeInfoValue(data, pair)) {
                TLOGE(WmsLogTag::WMS_ATTRIBUTE, "Write window change info value failed");
                return;
            }
        }
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "remote is null");
        return;
    }
    if (remote->SendRequest(static_cast<uint32_t>(WindowManagerAgentMsg::TRANS_ID_NOTIFY_WINDOW_PROPERTY_CHANGE),
        data, reply, option) != ERR_NONE) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "SendRequest failed");
    }
}

void WindowManagerAgentProxy::NotifySupportRotationChange(const SupportRotationInfo& supportRotationInfo)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TLOGE(WmsLogTag::WMS_ROTATION, "WriteInterfaceToken failed");
        return;
    }
    if (!data.WriteParcelable(&supportRotationInfo)) {
        TLOGE(WmsLogTag::WMS_ROTATION, "supportRotationInfo marshalling failed");
        return;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TLOGE(WmsLogTag::WMS_ROTATION, "remote is null");
        return;
    }
    int sendCode = remote->SendRequest(
        static_cast<uint32_t>(WindowManagerAgentMsg::TRANS_ID_NOTIFY_WINDOW_SUPPORT_ROTATION_CHANGE),
        data, reply, option);
    if (sendCode != ERR_NONE) {
        TLOGE(WmsLogTag::WMS_ROTATION, "SendRequest failed, code: %{public}d", sendCode);
    }
}

bool WindowManagerAgentProxy::WriteWindowChangeInfoValue(MessageParcel& data,
    const std::pair<WindowInfoKey, WindowChangeInfoType>& windowInfoPair)
{
    if (!data.WriteInt32(static_cast<int32_t>(windowInfoPair.first))) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "Write windowInfoKey failed");
        return false;
    }
    switch (windowInfoPair.first) {
        case WindowInfoKey::WINDOW_ID: {
            if (!data.WriteUint32(static_cast<uint32_t>(std::get<int32_t>(windowInfoPair.second))  )) {
                TLOGE(WmsLogTag::WMS_ATTRIBUTE, "Write uint32_t failed");
                return false;
            }
            break;
        }
        case WindowInfoKey::BUNDLE_NAME :
        case WindowInfoKey::ABILITY_NAME: {
            if (!data.WriteString(std::get<std::string>(windowInfoPair.second))) {
                TLOGE(WmsLogTag::WMS_ATTRIBUTE, "Write string failed");
                return false;
            }
            break;
        }
        case WindowInfoKey::APP_INDEX : {
            if (!data.WriteInt32(std::get<int32_t>(windowInfoPair.second))) {
                TLOGE(WmsLogTag::WMS_ATTRIBUTE, "Write int32_t failed");
                return false;
            }
            break;
        }
        case WindowInfoKey::VISIBILITY_STATE : {
            if (!data.WriteUint32(static_cast<uint32_t>(std::get<WindowVisibilityState>(windowInfoPair.second)))) {
                TLOGE(WmsLogTag::WMS_ATTRIBUTE, "Write WindowVisibilityState failed");
                return false;
            }
            break;
        }
        case WindowInfoKey::WINDOW_MODE : {
            if (!data.WriteUint32(static_cast<uint32_t>(std::get<WindowMode>(windowInfoPair.second)))) {
                TLOGE(WmsLogTag::WMS_ATTRIBUTE, "Write WindowMode failed");
                return false;
            }
            break;
        }
        case WindowInfoKey::DISPLAY_ID : {
            if (!data.WriteUint64(std::get<uint64_t>(windowInfoPair.second))) {
                TLOGE(WmsLogTag::WMS_ATTRIBUTE, "Write uint64_t failed");
                return false;
            }
            break;
        }
        case WindowInfoKey::WINDOW_RECT : {
            Rect rect = std::get<Rect>(windowInfoPair.second);
            if (!data.WriteInt32(rect.posX_)) {
                TLOGE(WmsLogTag::WMS_ATTRIBUTE, "Write posX failed");
                return false;
            }
            if (!data.WriteInt32(rect.posY_)) {
                TLOGE(WmsLogTag::WMS_ATTRIBUTE, "Write posY failed");
                return false;
            }
            if (!data.WriteUint32(rect.width_)) {
                TLOGE(WmsLogTag::WMS_ATTRIBUTE, "Write Width failed");
                return false;
            }
            if (!data.WriteUint32(rect.height_)) {
                TLOGE(WmsLogTag::WMS_ATTRIBUTE, "Write Height failed");
                return false;
            }
            break;
        }
        case WindowInfoKey::FLOATING_SCALE : {
            if (!data.WriteFloat(std::get<float>(windowInfoPair.second))) {
                TLOGE(WmsLogTag::WMS_ATTRIBUTE, "Write float failed");
                return false;
            }
            break;
        }
        case WindowInfoKey::MID_SCENE : {
            if (!data.WriteBool(std::get<bool>(windowInfoPair.second))) {
                TLOGE(WmsLogTag::WMS_ATTRIBUTE, "Write bool failed");
                return false;
            }
            break;
        }
        case WindowInfoKey::WINDOW_GLOBAL_RECT : {
            Rect globalRect = std::get<Rect>(windowInfoPair.second);
            if (!data.WriteInt32(globalRect.posX_)) {
                TLOGE(WmsLogTag::WMS_ATTRIBUTE, "Write posX failed");
                return false;
            }
            if (!data.WriteInt32(globalRect.posY_)) {
                TLOGE(WmsLogTag::WMS_ATTRIBUTE, "Write posY failed");
                return false;
            }
            if (!data.WriteUint32(globalRect.width_)) {
                TLOGE(WmsLogTag::WMS_ATTRIBUTE, "Write Width failed");
                return false;
            }
            if (!data.WriteUint32(globalRect.height_)) {
                TLOGE(WmsLogTag::WMS_ATTRIBUTE, "Write Height failed");
                return false;
            }
            break;
        }
        default : {
            TLOGE(WmsLogTag::WMS_ATTRIBUTE, "Unknown WindowInfoKey: %{public}d",
                static_cast<int32_t>(windowInfoPair.first));
            return false;
        }
    }
    return true;
}
} // namespace Rosen
} // namespace OHOS

