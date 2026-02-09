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

#include "zidl/display_manager_agent_stub.h"

#include <vector>

#include "display_info.h"
#include "display_change_info.h"
#include "dm_common.h"
#include "marshalling_helper.h"
#include "screen_info.h"
#include "window_manager_hilog.h"

namespace OHOS::Rosen {

int32_t DisplayManagerAgentStub::OnRemoteRequest(uint32_t code, MessageParcel& data,
    MessageParcel& reply, MessageOption& option)
{
    TLOGD(WmsLogTag::DMS, "code:%{public}u", code);
    if (data.ReadInterfaceToken() != GetDescriptor()) {
        TLOGE(WmsLogTag::DMS, "InterfaceToken check failed");
        return -1;
    }
    switch (code) {
        case TRANS_ID_NOTIFY_DISPLAY_POWER_EVENT: {
            DisplayPowerEvent event = static_cast<DisplayPowerEvent>(data.ReadUint32());
            EventStatus status = static_cast<EventStatus>(data.ReadUint32());
            NotifyDisplayPowerEvent(event, status);
            break;
        }
        case TRANS_ID_NOTIFY_DISPLAY_STATE_CHANGED: {
            DisplayState state = static_cast<DisplayState>(data.ReadUint32());
            DisplayId id = static_cast<DisplayId>(data.ReadUint64());
            NotifyDisplayStateChanged(id, state);
            break;
        }
        case TRANS_ID_ON_SCREEN_CONNECT: {
            sptr<ScreenInfo> screenInfo = data.ReadParcelable<ScreenInfo>();
            OnScreenConnect(screenInfo);
            break;
        }
        case TRANS_ID_ON_SCREEN_DISCONNECT: {
            ScreenId screenId;
            if (!data.ReadUint64(screenId)) {
                TLOGE(WmsLogTag::DMS, "Read ScreenId failed");
                return -1;
            }
            OnScreenDisconnect(screenId);
            break;
        }
        case TRANS_ID_ON_SCREEN_CHANGED: {
            sptr<ScreenInfo> screenInfo = data.ReadParcelable<ScreenInfo>();
            uint32_t event;
            if (!data.ReadUint32(event)) {
                TLOGE(WmsLogTag::DMS, "Read ScreenChangeEvent failed");
                return -1;
            }
            OnScreenChange(screenInfo, static_cast<ScreenChangeEvent>(event));
            break;
        }
        case TRANS_ID_ON_SCREENGROUP_CHANGED: {
            std::string trigger;
            if (!data.ReadString(trigger)) {
                TLOGE(WmsLogTag::DMS, "Read trigger failed");
                return -1;
            }
            std::vector<sptr<ScreenInfo>> screenInfos;
            if (!MarshallingHelper::UnmarshallingVectorParcelableObj<ScreenInfo>(data, screenInfos)) {
                TLOGE(WmsLogTag::DMS, "Read ScreenInfo failed");
                return -1;
            }
            uint32_t event;
            if (!data.ReadUint32(event)) {
                TLOGE(WmsLogTag::DMS, "Read ScreenChangeEvent failed");
                return -1;
            }
            OnScreenGroupChange(trigger, screenInfos, static_cast<ScreenGroupChangeEvent>(event));
            break;
        }
        case TRANS_ID_ON_DISPLAY_CONNECT: {
            sptr<DisplayInfo> displayInfo = data.ReadParcelable<DisplayInfo>();
            OnDisplayCreate(displayInfo);
            break;
        }
        case TRANS_ID_ON_DISPLAY_DISCONNECT: {
            DisplayId displayId;
            if (!data.ReadUint64(displayId)) {
                TLOGE(WmsLogTag::DMS, "Read DisplayId failed");
                return -1;
            }
            OnDisplayDestroy(displayId);
            break;
        }
        case TRANS_ID_ON_DISPLAY_CHANGED: {
            sptr<DisplayInfo> displayInfo = data.ReadParcelable<DisplayInfo>();
            uint32_t event;
            if (!data.ReadUint32(event)) {
                TLOGE(WmsLogTag::DMS, "Read DisplayChangeEvent failed");
                return -1;
            }
            OnDisplayChange(displayInfo, static_cast<DisplayChangeEvent>(event));
            break;
        }
        case TRANS_ID_ON_SCREEN_SHOT: {
            sptr<ScreenshotInfo> snapshotInfo = data.ReadParcelable<ScreenshotInfo>();
            OnScreenshot(snapshotInfo);
            break;
        }
        case TRANS_ID_ON_PRIVATE_WINDOW: {
            bool hasPrivate = data.ReadBool();
            NotifyPrivateWindowStateChanged(hasPrivate);
            break;
        }
        case TRANS_ID_ON_PRIVATE_WINDOW_LIST: {
            return ProcPrivateWindowList(data);
        }
        case TRANS_ID_ON_FOLD_STATUS_CHANGED: {
            uint32_t foldStatus;
            if (!data.ReadUint32(foldStatus)) {
                TLOGE(WmsLogTag::DMS, "Read FoldStatus failed");
                return -1;
            }
            NotifyFoldStatusChanged(static_cast<FoldStatus>(foldStatus));
            break;
        }
        case TRANS_ID_ON_DISPLAY_CHANGE_INFO_CHANGED: {
            sptr<DisplayChangeInfo> info;
            info = DisplayChangeInfo::Unmarshalling(data);
            if (!info) {
                TLOGE(WmsLogTag::DMS, "Read DisplayChangeInfo failed");
                return -1;
            }
            NotifyDisplayChangeInfoChanged(info);
            break;
        }
        case TRANS_ID_ON_DISPLAY_MODE_CHANGED: {
            uint32_t displayMode;
            if (!data.ReadUint32(displayMode)) {
                TLOGE(WmsLogTag::DMS, "Read FoldDisplayMode failed");
                return -1;
            }
            NotifyDisplayModeChanged(static_cast<FoldDisplayMode>(displayMode));
            break;
        }
        case TRANS_ID_ON_AVAILABLE_AREA_CHANGED: {
            DMRect rect;
            DisplayId displayId = static_cast<DisplayId>(data.ReadUint64());
            rect.posX_ = data.ReadInt32();
            rect.posY_ = data.ReadInt32();
            rect.width_ = data.ReadUint32();
            rect.height_ = data.ReadUint32();
            NotifyAvailableAreaChanged(rect, displayId);
            break;
        }
        case TRANS_ID_ON_FOLD_ANGLE_CHANGED: {
            return ProcFoldAngleChanged(data);
        }
        case TRANS_ID_ON_CAPTURE_STATUS_CHANGED: {
            return ProcCaptureStatusChanged(data);
        }
        case TRANS_ID_ON_SCREEN_MAGNETIC_STATE_CHANGED: {
            bool isMagneticState = data.ReadBool();
            NotifyScreenMagneticStateChanged(isMagneticState);
            break;
        }
        case TRANS_ID_ON_BRIGHTNESS_INFO_CHANGED: {
            DisplayId id;
            ScreenBrightnessInfo info;
            if (!data.ReadUint64(id) || !data.ReadFloat(info.currentHeadroom) ||
                !data.ReadFloat(info.maxHeadroom) || !data.ReadFloat(info.sdrNits)) {
                TLOGE(WmsLogTag::DMS, "Read brightnessInfo failed");
                return -1;
            }
            NotifyBrightnessInfoChanged(id, info);
            break;
        }
        case TRANS_ID_ON_SCREEN_MODE_CHANGED: {
            std::vector<sptr<ScreenInfo>> screenInfos;
            if (!MarshallingHelper::UnmarshallingVectorParcelableObj<ScreenInfo>(data, screenInfos)) {
                TLOGE(WmsLogTag::DMS, "Read ScreenInfo failed");
                return -1;
            }
            NotifyScreenModeChange(screenInfos);
            break;
        }
        case TRANS_ID_NOTIFY_ABNORMAL_SCREEN_CONNECT_CHANGED: {
            ScreenId screenId;
            if (!data.ReadUint64(screenId)) {
                TLOGE(WmsLogTag::DMS, "Read screenId failed");
                return -1;
            }
            NotifyAbnormalScreenConnectChange(screenId);
            break;
        }
        case TRANS_ID_ON_DISPLAY_ATTRIBUTE_CHANGED: {
            sptr<DisplayInfo> displayInfo = data.ReadParcelable<DisplayInfo>();
            std::vector<std::string> attributes;
            if (!data.ReadStringVector(&attributes)) {
                TLOGE(WmsLogTag::DMS, "Read attributes failed");
                return -1;
            }
            OnDisplayAttributeChange(displayInfo, attributes);
            break;
        }
        case TRANS_ID_ON_RECORDING_DISPLAY_CHANGED: {
            std::vector<DisplayId> displayIds;
            if (!data.ReadUInt64Vector(&displayIds)) {
                TLOGE(WmsLogTag::DMS, "Read screenId failed");
                return -1;
            }
            NotifyRecordingDisplayChanged(displayIds);
            break;
        }
        default: {
            TLOGW(WmsLogTag::DMS, "unknown transaction code %{public}d", code);
            return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
        }
    }
    return 0;
}

int32_t DisplayManagerAgentStub::ProcFoldAngleChanged(MessageParcel& data)
{
    std::vector<float> foldAngles;
    if (!data.ReadFloatVector(&foldAngles)) {
        TLOGE(WmsLogTag::DMS, "Read foldAngles failed");
        return -1;
    }
    NotifyFoldAngleChanged(foldAngles);
    return 0;
}

int32_t DisplayManagerAgentStub::ProcCaptureStatusChanged(MessageParcel& data)
{
    bool isCapture = data.ReadBool();
    NotifyCaptureStatusChanged(isCapture);
    return 0;
}

int32_t DisplayManagerAgentStub::ProcPrivateWindowList(MessageParcel& data)
{
    DisplayId displayId = static_cast<DisplayId>(data.ReadUint64());
    std::vector<std::string> privacyWindowList;
    data.ReadStringVector(&privacyWindowList);
    NotifyPrivateStateWindowListChanged(displayId, privacyWindowList);
    return 0;
}
} // namespace OHOS::Rosen
