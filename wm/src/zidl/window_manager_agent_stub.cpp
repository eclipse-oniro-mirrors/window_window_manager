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

#include "zidl/window_manager_agent_stub.h"
#include "ipc_skeleton.h"
#include "marshalling_helper.h"
#include "window_manager_hilog.h"
#include "wm_common.h"

namespace OHOS {
namespace Rosen {
namespace {
constexpr HiviewDFX::HiLogLabel LABEL = {LOG_CORE, HILOG_DOMAIN_WINDOW, "WindowManagerAgentStub"};
constexpr uint32_t MAX_VECTOR_SIZE = 10000;
}

int WindowManagerAgentStub::OnRemoteRequest(uint32_t code, MessageParcel& data,
    MessageParcel& reply, MessageOption& option)
{
    WLOGFD("code is %{public}u", code);
    if (data.ReadInterfaceToken() != GetDescriptor()) {
        WLOGFE("InterfaceToken check failed");
        return ERR_TRANSACTION_FAILED;
    }
    WindowManagerAgentMsg msgId = static_cast<WindowManagerAgentMsg>(code);
    switch (msgId) {
        case WindowManagerAgentMsg::TRANS_ID_UPDATE_FOCUS: {
            sptr<FocusChangeInfo> info = data.ReadParcelable<FocusChangeInfo>();
            if (info == nullptr) {
                WLOGFE("FocusChangeInfo is null");
                return ERR_INVALID_DATA;
            }
            bool focused = false;
            if (!data.ReadBool(focused)) {
                TLOGE(WmsLogTag::WMS_FOCUS, "read focused failed");
                return ERR_INVALID_DATA;
            }
            UpdateFocusChangeInfo(info, focused);
            break;
        }
        case WindowManagerAgentMsg::TRANS_ID_NOTIFY_DISPLAY_GROUP_INFO_CHANGE: {
            DisplayGroupId displayGroupId = DISPLAY_GROUP_ID_INVALID;
            if (!data.ReadUint64(displayGroupId)) {
                TLOGE(WmsLogTag::WMS_FOCUS, "read displayGroupId failed");
                return ERR_INVALID_DATA;
            }
            DisplayId displayId = DISPLAY_ID_INVALID;
            if (!data.ReadUint64(displayId)) {
                TLOGE(WmsLogTag::WMS_FOCUS, "read displayId failed");
                return ERR_INVALID_DATA;
            }
            bool isAdd = true;
            if (!data.ReadBool(isAdd)) {
                TLOGE(WmsLogTag::WMS_FOCUS, "read isAdd failed");
                return ERR_INVALID_DATA;
            }
            UpdateDisplayGroupInfo(displayGroupId, displayId, isAdd);
            break;
        }
        case WindowManagerAgentMsg::TRANS_ID_NOTIFY_WINDOW_SYSTEM_BAR_PROPERTY_CHANGE: {
            // LCOV_EXCL_START
            uint32_t type = 0;
            if (!data.ReadUint32(type) ||
                type < static_cast<uint32_t>(WindowType::ABOVE_APP_SYSTEM_WINDOW_BASE) ||
                type > static_cast<uint32_t>(WindowType::ABOVE_APP_SYSTEM_WINDOW_END)) {
                TLOGE(WmsLogTag::WMS_IMMS, "read type failed");
                return ERR_INVALID_DATA;
            }
            bool enable = false;
            if (!data.ReadBool(enable)) {
                TLOGE(WmsLogTag::WMS_IMMS, "read enable failed");
                return ERR_INVALID_DATA;
            }
            uint32_t backgroundColor = 0;
            if (!data.ReadUint32(backgroundColor)) {
                TLOGE(WmsLogTag::WMS_IMMS, "read backgroundColor failed");
                return ERR_INVALID_DATA;
            }
            uint32_t contentColor = 0;
            if (!data.ReadUint32(contentColor)) {
                TLOGE(WmsLogTag::WMS_IMMS, "read contentColor failed");
                return ERR_INVALID_DATA;
            }
            bool enableAnimation = false;
            if (!data.ReadBool(enableAnimation)) {
                TLOGE(WmsLogTag::WMS_IMMS, "read enableAnimation failed");
                return ERR_INVALID_DATA;
            }
            uint32_t settingFlag = 0;
            uint32_t MAX_SETTINGFLAG = 7;
            if (!data.ReadUint32(settingFlag) ||
                settingFlag < static_cast<uint32_t>(SystemBarSettingFlag::DEFAULT_SETTING) ||
                settingFlag > MAX_SETTINGFLAG) {
                TLOGE(WmsLogTag::WMS_IMMS, "read settingFlag failed");
                return ERR_INVALID_DATA;
            }
            SystemBarProperty systemBarProperty = { enable, backgroundColor,
                contentColor, enableAnimation, static_cast<SystemBarSettingFlag>(settingFlag) };
            NotifyWindowSystemBarPropertyChange(static_cast<WindowType>(type), systemBarProperty);
            break;
            // LCOV_EXCL_STOP
        }
        case WindowManagerAgentMsg::TRANS_ID_UPDATE_WINDOW_MODE_TYPE: {
            uint8_t typeId = 0;
            if (!data.ReadUint8(typeId) ||
                typeId < static_cast<uint8_t>(WindowModeType::WINDOW_MODE_SPLIT_FLOATING) ||
                typeId > static_cast<uint8_t>(WindowModeType::WINDOW_MODE_OTHER)) {
                TLOGE(WmsLogTag::WMS_LIFE, "read WindowModeType failed");
                return ERR_INVALID_DATA;
            }
            WindowModeType type = static_cast<WindowModeType>(typeId);
            UpdateWindowModeTypeInfo(type);
            break;
        }
        case WindowManagerAgentMsg::TRANS_ID_UPDATE_SYSTEM_BAR_PROPS: {
            DisplayId displayId = 0;
            if (!data.ReadUint64(displayId)) {
                return ERR_INVALID_DATA;
            }
            SystemBarRegionTints tints;
            bool res = MarshallingHelper::UnmarshallingVectorObj<SystemBarRegionTint>(data, tints,
                [](Parcel& parcel, SystemBarRegionTint& tint) {
                    uint32_t type;
                    SystemBarProperty prop;
                    Rect region;
                    bool res = parcel.ReadUint32(type) && parcel.ReadBool(prop.enable_) &&
                        parcel.ReadUint32(prop.backgroundColor_) && parcel.ReadUint32(prop.contentColor_) &&
                        parcel.ReadInt32(region.posX_) && parcel.ReadInt32(region.posY_) &&
                        parcel.ReadUint32(region.width_) && parcel.ReadUint32(region.height_);
                    tint.type_ = static_cast<WindowType>(type);
                    tint.prop_ = prop;
                    tint.region_ = region;
                    return res;
                }
            );
            if (!res) {
                WLOGFE("fail to read SystemBarRegionTints.");
                break;
            }
            UpdateSystemBarRegionTints(displayId, tints);
            break;
        }
        case WindowManagerAgentMsg::TRANS_ID_UPDATE_WINDOW_STATUS: {
            std::vector<sptr<AccessibilityWindowInfo>> infos;
            if (!MarshallingHelper::UnmarshallingVectorParcelableObj<AccessibilityWindowInfo>(data, infos)) {
                WLOGFE("read accessibility window infos failed");
                return ERR_INVALID_DATA;
            }
            int32_t typeId = 0;
            if (!data.ReadInt32(typeId) ||
                typeId < static_cast<int32_t>(WindowUpdateType::WINDOW_UPDATE_ADDED) ||
                typeId > static_cast<int32_t>(WindowUpdateType::WINDOW_UPDATE_ALL)) {
                TLOGE(WmsLogTag::WMS_LIFE, "read WindowUpdateType failed");
                return ERR_INVALID_DATA;
            }
            WindowUpdateType type = static_cast<WindowUpdateType>(typeId);
            NotifyAccessibilityWindowInfo(infos, type);
            break;
        }
        case WindowManagerAgentMsg::TRANS_ID_UPDATE_WINDOW_VISIBILITY: {
            std::vector<sptr<WindowVisibilityInfo>> infos;
            if (!MarshallingHelper::UnmarshallingVectorParcelableObj<WindowVisibilityInfo>(data, infos)) {
                WLOGFE("fail to read WindowVisibilityInfo.");
                break;
            }
            UpdateWindowVisibilityInfo(infos);
            break;
        }
        case WindowManagerAgentMsg::TRANS_ID_UPDATE_WINDOW_DRAWING_STATE: {
            std::vector<sptr<WindowDrawingContentInfo>> infos;
            if (!MarshallingHelper::UnmarshallingVectorParcelableObj<WindowDrawingContentInfo>(data, infos)) {
                WLOGFE("fail to read WindowDrawingContentInfo.");
                break;
            }
            UpdateWindowDrawingContentInfo(infos);
            break;
        }
        case WindowManagerAgentMsg::TRANS_ID_UPDATE_CAMERA_FLOAT: {
            uint32_t accessTokenId = data.ReadUint32();
            bool isShowing = data.ReadBool();
            UpdateCameraFloatWindowStatus(accessTokenId, isShowing);
            break;
        }
        case WindowManagerAgentMsg::TRANS_ID_UPDATE_WATER_MARK_FLAG: {
            bool showWaterMark = data.ReadBool();
            NotifyWaterMarkFlagChangedResult(showWaterMark);
            break;
        }
        case WindowManagerAgentMsg::TRANS_ID_UPDATE_VISIBLE_WINDOW_NUM: {
            std::vector<VisibleWindowNumInfo> visibleWindowNumInfo;
            bool res = MarshallingHelper::UnmarshallingVectorObj<VisibleWindowNumInfo>(
                data, visibleWindowNumInfo, [](Parcel& parcel, VisibleWindowNumInfo& num) {
                    uint32_t displayId = -1;
                    uint32_t visibleWindowNum = -1;
                    bool res = parcel.ReadUint32(displayId) && parcel.ReadUint32(visibleWindowNum);
                    num.displayId = displayId;
                    num.visibleWindowNum = visibleWindowNum;
                    return res;
                }
            );
            if (!res) {
                WLOGFE("fail to read VisibleWindowNumInfo.");
                break;
            }
            UpdateVisibleWindowNum(visibleWindowNumInfo);
            break;
        }
        case WindowManagerAgentMsg::TRANS_ID_UPDATE_GESTURE_NAVIGATION_ENABLED: {
            bool enbale = data.ReadBool();
            NotifyGestureNavigationEnabledResult(enbale);
            break;
        }
        case WindowManagerAgentMsg::TRANS_ID_UPDATE_CAMERA_WINDOW_STATUS: {
            uint32_t accessTokenId = data.ReadUint32();
            bool isShowing = data.ReadBool();
            UpdateCameraWindowStatus(accessTokenId, isShowing);
            break;
        }
        case WindowManagerAgentMsg::TRANS_ID_UPDATE_WINDOW_STYLE_TYPE: {
            uint8_t typeId = 0;
            if (!data.ReadUint8(typeId) ||
                typeId < static_cast<uint8_t>(WindowStyleType::WINDOW_STYLE_DEFAULT) ||
                typeId > static_cast<uint8_t>(WindowStyleType::WINDOW_STYLE_FREE_MULTI_WINDOW)) {
                TLOGE(WmsLogTag::WMS_LIFE, "read WindowStyleType failed");
                return ERR_INVALID_DATA;
            }
            WindowStyleType type = static_cast<WindowStyleType>(typeId);
            NotifyWindowStyleChange(type);
            break;
        }
        case WindowManagerAgentMsg::TRANS_ID_NOTIFY_WINDOW_PID_VISIBILITY: {
            sptr<WindowPidVisibilityInfo> info = data.ReadParcelable<WindowPidVisibilityInfo>();
            if (info == nullptr) {
                TLOGE(WmsLogTag::WMS_LIFE, "windowPidVisibilityInfo is null.");
                return ERR_INVALID_DATA;
            }
            NotifyWindowPidVisibilityChanged(info);
            break;
        }
        case WindowManagerAgentMsg::TRANS_ID_UPDATE_PIP_WINDOW_STATE_CHANGED: {
            std::string bundleName = data.ReadString();
            bool isForeground = data.ReadBool();
            UpdatePiPWindowStateChanged(bundleName, isForeground);
            break;
        }
        case WindowManagerAgentMsg::TRANS_ID_NOTIFY_CALLING_DISPLAY_CHANGE: {
            sptr<CallingWindowInfo> callingWindowInfo = data.ReadParcelable<CallingWindowInfo>();
            if (callingWindowInfo == nullptr) {
                TLOGE(WmsLogTag::WMS_KEYBOARD, "callingWindowInfo is nullptr!");
                return ERR_INVALID_VALUE;
            }
            NotifyCallingWindowDisplayChanged(*callingWindowInfo);
            break;
        }
        case WindowManagerAgentMsg::TRANS_ID_NOTIFY_WINDOW_PROPERTY_CHANGE: {
            uint32_t propertyDirtyFlags = 0;
            if (!data.ReadUint32(propertyDirtyFlags)) {
                TLOGE(WmsLogTag::WMS_ATTRIBUTE, "read propertyDirtyFlags failed");
                return ERR_INVALID_DATA;
            }

            WindowInfoList windowInfoList;
            if (!ReadWindowInfoList(data, windowInfoList)) {
                TLOGE(WmsLogTag::WMS_ATTRIBUTE, "fail to read windowInfoList.");
                return ERR_INVALID_DATA;
            }
            NotifyWindowPropertyChange(propertyDirtyFlags, windowInfoList);
            break;
        }
        case WindowManagerAgentMsg::TRANS_ID_NOTIFY_WINDOW_SUPPORT_ROTATION_CHANGE: {
            sptr<SupportRotationInfo> supportRotationInfo = data.ReadParcelable<SupportRotationInfo>();
            if (supportRotationInfo == nullptr) {
                TLOGE(WmsLogTag::WMS_ROTATION, "fail to read supportRotationInfo.");
                return ERR_INVALID_DATA;
            }
            NotifySupportRotationChange(*supportRotationInfo);
            break;
        }
        default:
            WLOGFW("unknown transaction code %{public}d", code);
            return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
    }
    return ERR_NONE;
}

// LCOV_EXCL_START
bool WindowManagerAgentStub::ReadWindowInfoList(MessageParcel& data, WindowInfoList& windowInfoList)
{
    uint32_t windowInfoListLength = 0;
    if (!data.ReadUint32(windowInfoListLength)) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "read windowInfoListLength failed");
        return false;
    }
    size_t windowInfoListSize = static_cast<size_t>(windowInfoListLength);

    if (windowInfoListSize > MAX_VECTOR_SIZE) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "windowInfoListSize is too large, size: %{public}zu", windowInfoListSize);
        return false;
    }

    for (size_t i = 0; i < windowInfoListSize; i++) {
        uint32_t windowInfoLength = 0;
        if (!data.ReadUint32(windowInfoLength)) {
            TLOGE(WmsLogTag::WMS_ATTRIBUTE, "read windowInfoLength failed");
            return false;
        }
        size_t windowInfoSize = static_cast<size_t>(windowInfoLength);
        std::unordered_map<WindowInfoKey, WindowChangeInfoType> windowInfo;
        for (size_t j = 0; j < windowInfoSize; j++) {
            if (!ReadWindowInfo(data, windowInfo)) {
                TLOGE(WmsLogTag::WMS_ATTRIBUTE, "fail to read windowInfo.");
                return false;
            }
        }
        windowInfoList.emplace_back(windowInfo);
    }
    return true;
}

bool WindowManagerAgentStub::ReadWindowInfo(MessageParcel& data,
    std::unordered_map<WindowInfoKey, WindowChangeInfoType>& windowInfo)
{
    int32_t windowInfoKeyValue = 0;
    if (!data.ReadInt32(windowInfoKeyValue)) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "read windowInfoKeyValue failed");
        return false;
    }

    WindowInfoKey windowInfoKey = static_cast<WindowInfoKey>(windowInfoKeyValue);
    switch (windowInfoKey) {
        case WindowInfoKey::WINDOW_ID : {
            uint32_t value = 0;
            if (!data.ReadUint32(value)) {
                TLOGE(WmsLogTag::WMS_ATTRIBUTE, "read uint32_t failed");
                return false;
            }
            windowInfo[windowInfoKey] = value;
            break;
        }
        case WindowInfoKey::BUNDLE_NAME :
        case WindowInfoKey::ABILITY_NAME : {
            std::string value;
            if (!data.ReadString(value)) {
                TLOGE(WmsLogTag::WMS_ATTRIBUTE, "read string failed");
                return false;
            }
            windowInfo[windowInfoKey] = value;
            break;
        }
        case WindowInfoKey::APP_INDEX : {
            int32_t value = 0;
            if (!data.ReadInt32(value)) {
                TLOGE(WmsLogTag::WMS_ATTRIBUTE, "read int32_t failed");
                return false;
            }
            windowInfo[windowInfoKey] = value;
            break;
        }
        case WindowInfoKey::VISIBILITY_STATE : {
            uint32_t value = 0;
            if (!data.ReadUint32(value)) {
                TLOGE(WmsLogTag::WMS_ATTRIBUTE, "read WindowVisibilityState failed");
                return false;
            }
            windowInfo[windowInfoKey] = static_cast<WindowVisibilityState>(value);
            break;
        }
        case WindowInfoKey::WINDOW_MODE : {
            uint32_t value = 0;
            if (!data.ReadUint32(value)) {
                TLOGE(WmsLogTag::WMS_ATTRIBUTE, "read WindowMode failed");
                return false;
            }
            windowInfo[windowInfoKey] = static_cast<WindowMode>(value);
            break;
        }
        case WindowInfoKey::DISPLAY_ID : {
            uint64_t value = 0;
            if (!data.ReadUint64(value)) {
                TLOGE(WmsLogTag::WMS_ATTRIBUTE, "read uint64_t failed");
                return false;
            }
            windowInfo[windowInfoKey] = value;
            break;
        }
        case WindowInfoKey::WINDOW_RECT : {
            Rect rect = Rect::EMPTY_RECT;
            if (!data.ReadInt32(rect.posX_) || !data.ReadInt32(rect.posY_) ||
                !data.ReadUint32(rect.width_) || !data.ReadUint32(rect.height_)) {
                TLOGE(WmsLogTag::WMS_ATTRIBUTE, "read Rect failed");
                return false;
            }
            windowInfo[windowInfoKey] = rect;
            break;
        }
        case WindowInfoKey::FLOATING_SCALE : {
            float value = 0.f;
            if (!data.ReadFloat(value)) {
                TLOGE(WmsLogTag::WMS_ATTRIBUTE, "read float failed");
                return false;
            }
            windowInfo[windowInfoKey] = value;
            break;
        }
        case WindowInfoKey::MID_SCENE : {
            bool value = false;
            if (!data.ReadBool(value)) {
                TLOGE(WmsLogTag::WMS_ATTRIBUTE, "read bool failed");
                return false;
            }
            windowInfo[windowInfoKey] = value;
            break;
        }
        case WindowInfoKey::WINDOW_GLOBAL_RECT : {
            Rect globalRect = Rect::EMPTY_RECT;
            if (!data.ReadInt32(globalRect.posX_) || !data.ReadInt32(globalRect.posY_) ||
                !data.ReadUint32(globalRect.width_) || !data.ReadUint32(globalRect.height_)) {
                TLOGE(WmsLogTag::WMS_ATTRIBUTE, "read globalRect failed");
                return false;
            }
            windowInfo[windowInfoKey] = globalRect;
            break;
        }
        default : {
            TLOGE(WmsLogTag::WMS_ATTRIBUTE, "unknown WindowInfoKey");
            return false;
        }
    }
    return true;
}
// LCOV_EXCL_STOP
} // namespace Rosen
} // namespace OHOS
