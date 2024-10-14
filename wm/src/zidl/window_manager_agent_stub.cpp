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
            bool focused = data.ReadBool();
            UpdateFocusChangeInfo(info, focused);
            break;
        }
        case WindowManagerAgentMsg::TRANS_ID_UPDATE_WINDOW_MODE_TYPE: {
            WindowModeType type = static_cast<WindowModeType>(data.ReadUint8());
            UpdateWindowModeTypeInfo(type);
            break;
        }
        case WindowManagerAgentMsg::TRANS_ID_UPDATE_SYSTEM_BAR_PROPS: {
            DisplayId displayId = data.ReadUint64();
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
            WindowUpdateType type = static_cast<WindowUpdateType>(data.ReadUint32());
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
            WindowStyleType type = static_cast<WindowStyleType>(data.ReadUint8());
            NotifyWindowStyleChange(type);
            break;
        }
        default:
            WLOGFW("unknown transaction code %{public}d", code);
            return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
    }
    return ERR_NONE;
}
} // namespace Rosen
} // namespace OHOS
