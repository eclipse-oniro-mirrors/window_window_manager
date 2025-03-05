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

#include "zidl/window_stub.h"
#include <vector>
#include "ipc_skeleton.h"
#include <key_event.h>
#include "pointer_event.h"
#include "window_manager_hilog.h"
#include <transaction/rs_transaction.h>

namespace OHOS {
namespace Rosen {
namespace {
constexpr HiviewDFX::HiLogLabel LABEL = {LOG_CORE, HILOG_DOMAIN_WINDOW, "WindowStub"};
}

int WindowStub::OnRemoteRequest(uint32_t code, MessageParcel& data, MessageParcel& reply, MessageOption& option)
{
    if (staticDestroyMonitor_.IsDestroyed()) {
        WLOGFE("Main thread finished, static data has been destroyed");
        return ERR_INVALID_STATE;
    }
    if (data.ReadInterfaceToken() != GetDescriptor()) {
        WLOGFE("InterfaceToken check failed");
        return ERR_TRANSACTION_FAILED;
    }
    WindowMessage msgId = static_cast<WindowMessage>(code);
    switch (msgId) {
        case WindowMessage::TRANS_ID_UPDATE_WINDOW_RECT: {
            struct Rect rect { data.ReadInt32(), data.ReadInt32(), data.ReadUint32(), data.ReadUint32() };
            bool decoStatus = data.ReadBool();
            WindowSizeChangeReason reason = static_cast<WindowSizeChangeReason>(data.ReadUint32());
            bool hasRSTransaction = data.ReadBool();
            if (hasRSTransaction) {
                auto rsTransaction = data.ReadParcelable<RSTransaction>();
                if (!rsTransaction) {
                    WLOGFE("RSTransaction unMarsh failed");
                    return -1;
                }
                std::shared_ptr<RSTransaction> transaction(rsTransaction);
                UpdateWindowRect(rect, decoStatus, reason, transaction);
            } else {
                UpdateWindowRect(rect, decoStatus, reason);
            }
            break;
        }
        case WindowMessage::TRANS_ID_UPDATE_WINDOW_MODE: {
            WindowMode mode = static_cast<WindowMode>(data.ReadUint32());
            UpdateWindowMode(mode);
            break;
        }
        case WindowMessage::TRANS_ID_UPDATE_MODE_SUPPORT_INFO: {
            uint32_t windowModeSupportType = 0;
            if (!data.ReadUint32(windowModeSupportType)) {
                TLOGE(WmsLogTag::WMS_LAYOUT, "read windowModeSupportType failed");
                return ERR_INVALID_DATA;
            }
            UpdateWindowModeSupportType(windowModeSupportType);
            break;
        }
        case WindowMessage::TRANS_ID_UPDATE_FOCUS_STATUS: {
            bool focused = data.ReadBool();
            UpdateFocusStatus(focused);
            break;
        }
        case WindowMessage::TRANS_ID_UPDATE_AVOID_AREA: {
            sptr<AvoidArea> avoidArea = data.ReadStrongParcelable<AvoidArea>();
            if (avoidArea == nullptr) {
                return ERR_INVALID_DATA;
            }
            uint32_t type;
            if (!data.ReadUint32(type)) {
                return ERR_INVALID_DATA;
            }
            UpdateAvoidArea(avoidArea, static_cast<AvoidAreaType>(type));
            break;
        }
        case WindowMessage::TRANS_ID_UPDATE_WINDOW_STATE: {
            UpdateWindowState(static_cast<WindowState>(data.ReadUint32()));
            break;
        }
        case WindowMessage::TRANS_ID_UPDATE_DRAG_EVENT: {
            PointInfo point;
            point.x = data.ReadInt32();
            point.y = data.ReadInt32();
            DragEvent event = static_cast<DragEvent>(data.ReadUint32());
            UpdateWindowDragInfo(point, event);
            break;
        }
        case WindowMessage::TRANS_ID_UPDATE_DISPLAY_ID: {
            UpdateDisplayId(data.ReadUint64(), data.ReadUint64());
            break;
        }
        case WindowMessage::TRANS_ID_UPDATE_OCCUPIED_AREA: {
            sptr<OccupiedAreaChangeInfo> info = data.ReadParcelable<OccupiedAreaChangeInfo>();
            if (info == nullptr) {
                WLOGFE("OccupiedAreaChangeInfo is null");
                return ERR_INVALID_DATA;
            }
            bool hasRSTransaction = data.ReadBool();
            if (hasRSTransaction) {
                auto rsTransaction = data.ReadParcelable<RSTransaction>();
                if (!rsTransaction) {
                    WLOGFE("RSTransaction unMarsh failed");
                    return ERR_INVALID_DATA;
                }
                std::shared_ptr<RSTransaction> transaction(rsTransaction);
                UpdateOccupiedAreaChangeInfo(info, transaction);
            } else {
                UpdateOccupiedAreaChangeInfo(info);
            }

            break;
        }
        case WindowMessage::TRANS_ID_UPDATE_OCCUPIED_AREA_AND_RECT: {
            sptr<OccupiedAreaChangeInfo> info = data.ReadParcelable<OccupiedAreaChangeInfo>();
            if (info == nullptr) {
                WLOGFE("OccupiedAreaChangeInfo is null");
                return ERR_INVALID_DATA;
            }
            struct Rect rect { data.ReadInt32(), data.ReadInt32(), data.ReadUint32(), data.ReadUint32() };
            bool hasRSTransaction = data.ReadBool();
            if (hasRSTransaction) {
                auto rsTransaction = data.ReadParcelable<RSTransaction>();
                if (!rsTransaction) {
                    WLOGFE("RSTransaction unMarsh failed");
                    return ERR_INVALID_DATA;
                }
                std::shared_ptr<RSTransaction> transaction(rsTransaction);
                UpdateOccupiedAreaAndRect(info, rect, transaction);
            } else {
                UpdateOccupiedAreaAndRect(info, rect);
            }

            break;
        }
        case WindowMessage::TRANS_ID_UPDATE_ACTIVE_STATUS: {
            bool isActive = data.ReadBool();
            UpdateActiveStatus(isActive);
            break;
        }
        case WindowMessage::TRANS_ID_GET_WINDOW_PROPERTY: {
            auto property = GetWindowProperty();
            reply.WriteParcelable(property.GetRefPtr());
            break;
        }
        case WindowMessage::TRANS_ID_NOTIFY_OUTSIDE_PRESSED: {
            NotifyTouchOutside();
            break;
        }
        case WindowMessage::TRANS_ID_NOTIFY_SCREEN_SHOT: {
            NotifyScreenshot();
            break;
        }
        case WindowMessage::TRANS_ID_NOTIFY_DESTROY: {
            NotifyDestroy();
            break;
        }
        case WindowMessage::TRANS_ID_NOTIFY_FOREGROUND: {
            NotifyForeground();
            break;
        }
        case WindowMessage::TRANS_ID_NOTIFY_BACKGROUND: {
            NotifyBackground();
            break;
        }
        case WindowMessage::TRANS_ID_DUMP_INFO: {
            std::vector<std::string> params;
            if (!data.ReadStringVector(&params)) {
                WLOGFE("Fail to read params");
                return ERR_INVALID_DATA;
            }
            DumpInfo(params);
            break;
        }
        case WindowMessage::TRANS_ID_NOTIFY_CLIENT_POINT_UP: {
            auto pointerEvent = MMI::PointerEvent::Create();
            if (!pointerEvent || !pointerEvent->ReadFromParcel(data)) {
                WLOGFE("Read Pointer Event failed");
                return ERR_INVALID_DATA;
            }
            NotifyWindowClientPointUp(pointerEvent);
            break;
        }
        case WindowMessage::TRANS_ID_UPDATE_ZOOM_TRANSFORM: {
            Transform trans;
            trans.Unmarshalling(data);
            bool isDisplayZoomOn = data.ReadBool();
            UpdateZoomTransform(trans, isDisplayZoomOn);
            break;
        }
        case WindowMessage::TRANS_ID_RESTORE_SPLIT_WINDOW_MODE: {
            RestoreSplitWindowMode(data.ReadUint32());
            break;
        }
        case WindowMessage::TRANS_ID_CONSUME_KEY_EVENT: {
            auto event = MMI::KeyEvent::Create();
            if (!event || !event->ReadFromParcel(data)) {
                WLOGFE("Read Pointer Event failed");
                return ERR_INVALID_DATA;
            }
            ConsumeKeyEvent(event);
            break;
        }
        case WindowMessage::TRANS_ID_NOTIFY_FOREGROUND_INTERACTIVE_STATUS: {
            bool interactive = data.ReadBool();
            NotifyForegroundInteractiveStatus(interactive);
            break;
        }
        case WindowMessage::TRANS_ID_NOTIFY_MMI_SERVICE_ONLINE: {
            uint32_t winId = data.ReadUint32();
            NotifyMMIServiceOnline(winId);
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
