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

#include "zidl/window_manager_stub.h"
#include <ipc_skeleton.h>
#include <key_event.h>
#include <rs_iwindow_animation_controller.h>
#include <rs_window_animation_target.h>

#include "marshalling_helper.h"
#include "memory_guard.h"
#include "window_manager_hilog.h"

namespace OHOS {
namespace Rosen {
namespace {
constexpr HiviewDFX::HiLogLabel LABEL = {LOG_CORE, HILOG_DOMAIN_WINDOW, "WindowManagerStub"};
}

int32_t WindowManagerStub::OnRemoteRequest(uint32_t code, MessageParcel& data, MessageParcel& reply,
    MessageOption& option)
{
    MemoryGuard cacheGuard;
    if (data.ReadInterfaceToken() != GetDescriptor()) {
        WLOGFE("InterfaceToken check failed");
        return ERR_TRANSACTION_FAILED;
    }
    auto msgId = static_cast<WindowManagerMessage>(code);
    switch (msgId) {
        case WindowManagerMessage::TRANS_ID_CREATE_WINDOW: {
            sptr<IRemoteObject> windowObject = data.ReadRemoteObject();
            sptr<IWindow> windowProxy = iface_cast<IWindow>(windowObject);
            sptr<WindowProperty> windowProperty = data.ReadStrongParcelable<WindowProperty>();
            std::shared_ptr<RSSurfaceNode> surfaceNode = RSSurfaceNode::Unmarshalling(data);
            uint32_t windowId;
            sptr<IRemoteObject> token = nullptr;
            if (windowProperty && windowProperty->GetTokenState()) {
                token = data.ReadRemoteObject();
            }
            WMError errCode = CreateWindow(windowProxy, windowProperty, surfaceNode, windowId, token);
            reply.WriteUint32(windowId);
            reply.WriteInt32(static_cast<int32_t>(errCode));
            if (windowProperty) {
                reply.WriteUint32(windowProperty->GetWindowFlags());
                reply.WriteUint32(windowProperty->GetApiCompatibleVersion());
            }
            break;
        }
        case WindowManagerMessage::TRANS_ID_ADD_WINDOW: {
            sptr<WindowProperty> windowProperty = data.ReadStrongParcelable<WindowProperty>();
            WMError errCode = AddWindow(windowProperty);
            reply.WriteInt32(static_cast<int32_t>(errCode));
            break;
        }
        case WindowManagerMessage::TRANS_ID_REMOVE_WINDOW: {
            uint32_t windowId = 0;
            if (!data.ReadUint32(windowId)) {
                TLOGE(WmsLogTag::WMS_LIFE, "TRANS_ID_REMOVE_WINDOW Read windowId failed.");
                return ERR_INVALID_DATA;
            }
            bool isFromInnerkits = false;
            if (!data.ReadBool(isFromInnerkits)) {
                TLOGE(WmsLogTag::WMS_LIFE, "TRANS_ID_REMOVE_WINDOW Read isFromInnerkits failed.");
                return ERR_INVALID_DATA;
            }
            WMError errCode = RemoveWindow(windowId, isFromInnerkits);
            reply.WriteInt32(static_cast<int32_t>(errCode));
            break;
        }
        case WindowManagerMessage::TRANS_ID_DESTROY_WINDOW: {
            uint32_t windowId = 0;
            if (!data.ReadUint32(windowId)) {
                TLOGE(WmsLogTag::WMS_LIFE, "TRANS_ID_DESTROY_WINDOW Read windowId failed.");
                return ERR_INVALID_DATA;
            }
            WMError errCode = DestroyWindow(windowId);
            reply.WriteInt32(static_cast<int32_t>(errCode));
            break;
        }
        case WindowManagerMessage::TRANS_ID_REQUEST_FOCUS: {
            uint32_t windowId = 0;
            if (!data.ReadUint32(windowId)) {
                TLOGE(WmsLogTag::WMS_FOCUS, "read focus failed");
                return ERR_INVALID_DATA;
            }
            WMError errCode = RequestFocus(windowId);
            reply.WriteInt32(static_cast<int32_t>(errCode));
            break;
        }
        case WindowManagerMessage::TRANS_ID_GET_AVOID_AREA: {
            uint32_t windowId = data.ReadUint32();
            uint32_t avoidAreaTypeId = 0;
            if (!data.ReadUint32(avoidAreaTypeId) ||
                avoidAreaTypeId >= static_cast<uint32_t>(AvoidAreaType::TYPE_END)) {
                return ERR_INVALID_DATA;
            }
            auto avoidAreaType = static_cast<AvoidAreaType>(avoidAreaTypeId);
            Rect rect = {};
            if (!data.ReadInt32(rect.posX_) || !data.ReadInt32(rect.posY_) ||
                !data.ReadUint32(rect.width_) || !data.ReadUint32(rect.height_)) {
                TLOGE(WmsLogTag::WMS_IMMS, "read rect error");
                return ERR_INVALID_DATA;
            }
            AvoidArea avoidArea = GetAvoidAreaByType(windowId, avoidAreaType, rect);
            reply.WriteParcelable(&avoidArea);
            break;
        }
        case WindowManagerMessage::TRANS_ID_REGISTER_WINDOW_MANAGER_AGENT: {
            uint32_t windowType = 0;
            if (!data.ReadUint32(windowType) ||
                windowType >= static_cast<uint32_t>(WindowManagerAgentType::WINDOW_MANAGER_AGENT_TYPE_END)) {
                return ERR_INVALID_DATA;
            }
            auto type = static_cast<WindowManagerAgentType>(windowType);
            sptr<IRemoteObject> windowManagerAgentObject = data.ReadRemoteObject();
            if (windowManagerAgentObject == nullptr) {
                return ERR_INVALID_DATA;
            }
            sptr<IWindowManagerAgent> windowManagerAgentProxy =
                iface_cast<IWindowManagerAgent>(windowManagerAgentObject);
            WMError errCode = RegisterWindowManagerAgent(type, windowManagerAgentProxy);
            reply.WriteInt32(static_cast<int32_t>(errCode));
            break;
        }
        case WindowManagerMessage::TRANS_ID_UNREGISTER_WINDOW_MANAGER_AGENT: {
            uint32_t windowType = 0;
            if (!data.ReadUint32(windowType) ||
                windowType >= static_cast<uint32_t>(WindowManagerAgentType::WINDOW_MANAGER_AGENT_TYPE_END)) {
                return ERR_INVALID_DATA;
            }
            auto type = static_cast<WindowManagerAgentType>(windowType);
            sptr<IRemoteObject> windowManagerAgentObject = data.ReadRemoteObject();
            if (windowManagerAgentObject == nullptr) {
                return ERR_INVALID_DATA;
            }
            sptr<IWindowManagerAgent> windowManagerAgentProxy =
                iface_cast<IWindowManagerAgent>(windowManagerAgentObject);
            WMError errCode = UnregisterWindowManagerAgent(type, windowManagerAgentProxy);
            reply.WriteInt32(static_cast<int32_t>(errCode));
            break;
        }
        case WindowManagerMessage::TRANS_ID_NOTIFY_READY_MOVE_OR_DRAG: {
            uint32_t windowId = 0;
            if (!data.ReadUint32(windowId)) {
                return ERR_INVALID_DATA;
            }
            sptr<WindowProperty> windowProperty = data.ReadStrongParcelable<WindowProperty>();
            if (windowProperty == nullptr) {
                return ERR_INVALID_DATA;
            }
            sptr<MoveDragProperty> moveDragProperty = data.ReadStrongParcelable<MoveDragProperty>();
            if (moveDragProperty == nullptr) {
                return ERR_INVALID_DATA;
            }
            NotifyServerReadyToMoveOrDrag(windowId, windowProperty, moveDragProperty);
            break;
        }
        case WindowManagerMessage::TRANS_ID_PROCESS_POINT_DOWN: {
            uint32_t windowId = 0;
            if (!data.ReadUint32(windowId)) {
                return ERR_INVALID_DATA;
            }
            bool isPointDown = false;
            if (!data.ReadBool(isPointDown)) {
                return ERR_INVALID_DATA;
            }
            ProcessPointDown(windowId, isPointDown);
            break;
        }
        case WindowManagerMessage::TRANS_ID_PROCESS_POINT_UP: {
            uint32_t windowId = 0;
            if (!data.ReadUint32(windowId)) {
                return ERR_INVALID_DATA;
            }
            ProcessPointUp(windowId);
            break;
        }
        case WindowManagerMessage::TRANS_ID_GET_TOP_WINDOW_ID: {
            uint32_t mainWinId = 0;
            if (!data.ReadUint32(mainWinId)) {
                TLOGE(WmsLogTag::WMS_HIERARCHY, "read mainWinId failed");
                return ERR_INVALID_DATA;
            }
            uint32_t topWinId = 0;
            WMError errCode = GetTopWindowId(mainWinId, topWinId);
            reply.WriteUint32(topWinId);
            reply.WriteInt32(static_cast<int32_t>(errCode));
            break;
        }
        case WindowManagerMessage::TRANS_ID_MINIMIZE_ALL_APP_WINDOWS: {
            uint64_t displayId = 0;
            if (!data.ReadUint64(displayId)) {
                TLOGE(WmsLogTag::WMS_LIFE, "Read displayID failed.");
                return ERR_INVALID_DATA;
            }
            int32_t excludeWindowId = 0;
            if (!data.ReadInt32(excludeWindowId)) {
                TLOGE(WmsLogTag::WMS_LIFE, "Read excludeWindowId failed.");
                return ERR_INVALID_DATA;
            }
            WMError errCode = MinimizeAllAppWindows(displayId, excludeWindowId);
            reply.WriteInt32(static_cast<int32_t>(errCode));
            break;
        }
        case WindowManagerMessage::TRANS_ID_TOGGLE_SHOWN_STATE_FOR_ALL_APP_WINDOWS: {
            WMError errCode = ToggleShownStateForAllAppWindows();
            reply.WriteInt32(static_cast<int32_t>(errCode));
            break;
        }
        case WindowManagerMessage::TRANS_ID_UPDATE_LAYOUT_MODE: {
            uint32_t layoutMode = 0;
            if (!data.ReadUint32(layoutMode)) {
                TLOGE(WmsLogTag::WMS_LAYOUT, "read layoutMode failed");
                return ERR_INVALID_DATA;
            }
            auto mode = static_cast<WindowLayoutMode>(layoutMode);
            WMError errCode = SetWindowLayoutMode(mode);
            reply.WriteInt32(static_cast<int32_t>(errCode));
            break;
        }
        case WindowManagerMessage::TRANS_ID_NOTIFY_SCREEN_SHOT_EVENT: {
            int32_t screenshotEventType = 0;
            if (!data.ReadInt32(screenshotEventType)) {
                TLOGE(WmsLogTag::WMS_ATTRIBUTE, "read screenshotEventType failed");
                return ERR_INVALID_DATA;
            }
            WMError errCode = NotifyScreenshotEvent(static_cast<ScreenshotEventType>(screenshotEventType));
            reply.WriteInt32(static_cast<int32_t>(errCode));
            break;
        }
        case WindowManagerMessage::TRANS_ID_UPDATE_PROPERTY: {
            uint32_t actionValue = 0;
            if (!data.ReadUint32(actionValue)) {
                TLOGE(WmsLogTag::DEFAULT, "read action error");
                return ERR_INVALID_DATA;
            }
            auto action = static_cast<PropertyChangeAction>(actionValue);
            sptr<WindowProperty> windowProperty = new WindowProperty();
            windowProperty->Read(data, action);
            WMError errCode = UpdateProperty(windowProperty, action);
            reply.WriteInt32(static_cast<int32_t>(errCode));
            break;
        }
        case WindowManagerMessage::TRANS_ID_GET_ACCESSIBILITY_WINDOW_INFO_ID: {
            std::vector<sptr<AccessibilityWindowInfo>> infos;
            WMError errCode = GetAccessibilityWindowInfo(infos);
            if (!MarshallingHelper::MarshallingVectorParcelableObj<AccessibilityWindowInfo>(reply, infos)) {
                WLOGFE("Write accessibility window infos failed");
                return -1;
            }
            reply.WriteInt32(static_cast<int32_t>(errCode));
            break;
        }
        case WindowManagerMessage::TRANS_ID_GET_UNRELIABLE_WINDOW_INFO_ID: {
            int32_t windowId = 0;
            if (!data.ReadInt32(windowId)) {
                WLOGFE("Failed to readInt32 windowId");
                return ERR_INVALID_DATA;
            }
            std::vector<sptr<UnreliableWindowInfo>> infos;
            WMError errCode = GetUnreliableWindowInfo(windowId, infos);
            if (!MarshallingHelper::MarshallingVectorParcelableObj<UnreliableWindowInfo>(reply, infos)) {
                WLOGFE("Write unreliable window infos failed");
                return ERR_INVALID_DATA;
            }
            reply.WriteInt32(static_cast<int32_t>(errCode));
            break;
        }
        case WindowManagerMessage::TRANS_ID_GET_VISIBILITY_WINDOW_INFO_ID: {
            std::vector<sptr<WindowVisibilityInfo>> infos;
            WMError errCode = GetVisibilityWindowInfo(infos);
            if (!MarshallingHelper::MarshallingVectorParcelableObj<WindowVisibilityInfo>(reply, infos)) {
                WLOGFE("Write visibility window infos failed");
                return -1;
            }
            reply.WriteInt32(static_cast<int32_t>(errCode));
            break;
        }
        case WindowManagerMessage::TRANS_ID_ANIMATION_SET_CONTROLLER: {
            sptr<IRemoteObject> controllerObject = data.ReadRemoteObject();
            if (controllerObject == nullptr) {
                TLOGE(WmsLogTag::DEFAULT, "Read animation controller object failed");
                return ERR_INVALID_DATA;
            }
            sptr<RSIWindowAnimationController> controller = iface_cast<RSIWindowAnimationController>(controllerObject);
            WMError errCode = SetWindowAnimationController(controller);
            reply.WriteInt32(static_cast<int32_t>(errCode));
            break;
        }
        case WindowManagerMessage::TRANS_ID_GET_SYSTEM_CONFIG: {
            SystemConfig config;
            WMError errCode = GetSystemConfig(config);
            reply.WriteParcelable(&config);
            reply.WriteInt32(static_cast<int32_t>(errCode));
            break;
        }
        case WindowManagerMessage::TRANS_ID_NOTIFY_WINDOW_TRANSITION: {
            sptr<WindowTransitionInfo> from = data.ReadParcelable<WindowTransitionInfo>();
            sptr<WindowTransitionInfo> to = data.ReadParcelable<WindowTransitionInfo>();
            bool isFromClient = false;
            if (!data.ReadBool(isFromClient)) {
                return ERR_INVALID_DATA;
            }
            WMError errCode = NotifyWindowTransition(from, to, isFromClient);
            reply.WriteInt32(static_cast<int32_t>(errCode));
            break;
        }
        case WindowManagerMessage::TRANS_ID_GET_FULLSCREEN_AND_SPLIT_HOT_ZONE: {
            DisplayId displayId = 0;
            if (!data.ReadUint64(displayId)) {
                TLOGE(WmsLogTag::WMS_FOCUS, "read displayId failed");
                return ERR_INVALID_DATA;
            }
            ModeChangeHotZones hotZones = { { 0, 0, 0, 0 }, { 0, 0, 0, 0 }, { 0, 0, 0, 0 } };
            WMError errCode = GetModeChangeHotZones(displayId, hotZones);
            reply.WriteInt32(static_cast<int32_t>(errCode));

            reply.WriteInt32(hotZones.fullscreen_.posX_);
            reply.WriteInt32(hotZones.fullscreen_.posY_);
            reply.WriteUint32(hotZones.fullscreen_.width_);
            reply.WriteUint32(hotZones.fullscreen_.height_);

            reply.WriteInt32(hotZones.primary_.posX_);
            reply.WriteInt32(hotZones.primary_.posY_);
            reply.WriteUint32(hotZones.primary_.width_);
            reply.WriteUint32(hotZones.primary_.height_);

            reply.WriteInt32(hotZones.secondary_.posX_);
            reply.WriteInt32(hotZones.secondary_.posY_);
            reply.WriteUint32(hotZones.secondary_.width_);
            reply.WriteUint32(hotZones.secondary_.height_);
            break;
        }
        case WindowManagerMessage::TRANS_ID_GET_ANIMATION_CALLBACK: {
            std::vector<uint32_t> windowIds;
            data.ReadUInt32Vector(&windowIds);
            bool isAnimated = data.ReadBool();
            sptr<RSIWindowAnimationFinishedCallback> finishedCallback = nullptr;
            MinimizeWindowsByLauncher(windowIds, isAnimated, finishedCallback);
            if (finishedCallback == nullptr) {
                if (!reply.WriteBool(false)) {
                    WLOGFE("finishedCallback is nullptr and failed to write!");
                    return ERR_INVALID_DATA;
                }
            } else {
                if (!reply.WriteBool(true) || !reply.WriteRemoteObject(finishedCallback->AsObject())) {
                    WLOGFE("finishedCallback is not nullptr and failed to write!");
                    return ERR_INVALID_DATA;
                }
            }
            break;
        }
        case WindowManagerMessage::TRANS_ID_UPDATE_AVOIDAREA_LISTENER: {
            uint32_t windowId = 0;
            if (!data.ReadUint32(windowId)) {
                return ERR_INVALID_DATA;
            }
            bool haveAvoidAreaListener = false;
            if (!data.ReadBool(haveAvoidAreaListener)) {
                return ERR_INVALID_DATA;
            }
            WMError errCode = UpdateAvoidAreaListener(windowId, haveAvoidAreaListener);
            reply.WriteInt32(static_cast<int32_t>(errCode));
            break;
        }
        case WindowManagerMessage::TRANS_ID_UPDATE_RS_TREE: {
            uint32_t windowId = 0;
            if (!data.ReadUint32(windowId)) {
                return ERR_INVALID_DATA;
            }
            bool isAdd = false;
            if (!data.ReadBool(isAdd)) {
                return ERR_INVALID_DATA;
            }
            WMError errCode = UpdateRsTree(windowId, isAdd);
            reply.WriteInt32(static_cast<int32_t>(errCode));
            break;
        }
        case WindowManagerMessage::TRANS_ID_BIND_DIALOG_TARGET: {
            uint32_t windowId = 0;
            if (!data.ReadUint32(windowId)) {
                TLOGE(WmsLogTag::DEFAULT, "Failed to read windowId");
                return ERR_INVALID_DATA;
            }
            sptr<IRemoteObject> targetToken = data.ReadRemoteObject();
            if (targetToken == nullptr) {
                TLOGE(WmsLogTag::WMS_DIALOG, "Read targetToken object failed!");
                return ERR_INVALID_DATA;
            }
            WMError errCode = BindDialogTarget(windowId, targetToken);
            reply.WriteInt32(static_cast<int32_t>(errCode));
            break;
        }
        case WindowManagerMessage::TRANS_ID_SET_ANCHOR_AND_SCALE : {
            int32_t x = 0;
            int32_t y = 0;
            if (!data.ReadInt32(x) || !data.ReadInt32(y)) {
                return ERR_INVALID_DATA;
            }
            float scale = 0.0f;
            if (!data.ReadFloat(scale)) {
                return ERR_INVALID_DATA;
            }
            SetAnchorAndScale(x, y, scale);
            break;
        }
        case WindowManagerMessage::TRANS_ID_SET_ANCHOR_OFFSET: {
            int32_t deltaX = 0;
            int32_t deltaY = 0;
            if (!data.ReadInt32(deltaX) || !data.ReadInt32(deltaY)) {
                return ERR_INVALID_DATA;
            }
            SetAnchorOffset(deltaX, deltaY);
            break;
        }
        case WindowManagerMessage::TRANS_ID_OFF_WINDOW_ZOOM: {
            OffWindowZoom();
            break;
        }
        case WindowManagerMessage::TRANS_ID_RAISE_WINDOW_Z_ORDER: {
            uint32_t windowId = 0;
            if (!data.ReadUint32(windowId)) {
                TLOGE(WmsLogTag::WMS_HIERARCHY, "read windowId failed");
                return ERR_INVALID_DATA;
            }
            WMError errCode = RaiseToAppTop(windowId);
            reply.WriteInt32(static_cast<int32_t>(errCode));
            break;
        }
        case WindowManagerMessage::TRANS_ID_GET_SNAPSHOT: {
            uint32_t windowId = 0;
            if (!data.ReadUint32(windowId)) {
                TLOGE(WmsLogTag::WMS_ATTRIBUTE, "read windowId error");
                return ERR_INVALID_DATA;
            }
            std::shared_ptr<Media::PixelMap> pixelMap = GetSnapshot(windowId);
            reply.WriteParcelable(pixelMap.get());
            break;
        }
        case WindowManagerMessage::TRANS_ID_GESTURE_NAVIGATION_ENABLED: {
            bool enable = data.ReadBool();
            WMError errCode = SetGestureNavigationEnabled(enable);
            reply.WriteInt32(static_cast<int32_t>(errCode));
            break;
        }
        case WindowManagerMessage::TRANS_ID_SET_WINDOW_GRAVITY: {
            uint32_t windowId = INVALID_WINDOW_ID;
            if (!data.ReadUint32(windowId)) {
                TLOGE(WmsLogTag::WMS_KEYBOARD, "Read windowId failed.");
                return ERR_INVALID_DATA;
            }
            uint32_t gravityValue = 0;
            if (!data.ReadUint32(gravityValue) ||
                gravityValue < static_cast<uint32_t>(WindowGravity::WINDOW_GRAVITY_FLOAT) ||
                gravityValue > static_cast<uint32_t>(WindowGravity::WINDOW_GRAVITY_BOTTOM)) {
                TLOGE(WmsLogTag::WMS_KEYBOARD, "Window gravity value read failed, gravityValue: %{public}d",
                    gravityValue);
                return ERR_INVALID_DATA;
            }
            WindowGravity gravity = static_cast<WindowGravity>(gravityValue);
            uint32_t percent = 0;
            if (!data.ReadUint32(percent)) {
                TLOGE(WmsLogTag::WMS_KEYBOARD, "Percent read failed.");
                return ERR_INVALID_DATA;
            }
            WMError errCode = SetWindowGravity(windowId, gravity, percent);
            reply.WriteInt32(static_cast<int32_t>(errCode));
            break;
        }
        case WindowManagerMessage::TRANS_ID_DISPATCH_KEY_EVENT: {
            uint32_t windowId = 0;
            if (!data.ReadUint32(windowId)) {
                TLOGE(WmsLogTag::WMS_EVENT, "Read failed!");
                return ERR_INVALID_DATA;
            }
            std::shared_ptr<MMI::KeyEvent> event = MMI::KeyEvent::Create();
            if (event == nullptr) {
                TLOGE(WmsLogTag::WMS_EVENT, "event is null");
                return ERR_INVALID_DATA;
            }
            event->ReadFromParcel(data);
            DispatchKeyEvent(windowId, event);
            break;
        }
        case WindowManagerMessage::TRANS_ID_NOTIFY_DUMP_INFO_RESULT: {
            std::vector<std::string> info;
            if (!data.ReadStringVector(&info)) {
                return ERR_INVALID_DATA;
            }
            NotifyDumpInfoResult(info);
            break;
        }
        case WindowManagerMessage::TRANS_ID_GET_WINDOW_ANIMATION_TARGETS: {
            std::vector<uint32_t> missionIds;
            if (!data.ReadUInt32Vector(&missionIds)) {
                TLOGE(WmsLogTag::DEFAULT, "Read animation target mission ids failed");
                return ERR_INVALID_DATA;
            }
            std::vector<sptr<RSWindowAnimationTarget>> targets;
            WMError errCode = GetWindowAnimationTargets(missionIds, targets);
            if (!MarshallingHelper::MarshallingVectorParcelableObj<RSWindowAnimationTarget>(reply, targets)) {
                WLOGFE("Write window animation targets failed");
                return ERR_INVALID_DATA;
            }
            reply.WriteInt32(static_cast<int32_t>(errCode));
            break;
        }
        case WindowManagerMessage::TRANS_ID_SET_MAXIMIZE_MODE: {
            uint32_t modeId = 0;
            if (!data.ReadUint32(modeId) || modeId >= static_cast<uint32_t>(MaximizeMode::MODE_END)) {
                return ERR_INVALID_DATA;
            }
            MaximizeMode maximizeMode = static_cast<MaximizeMode>(modeId);
            SetMaximizeMode(maximizeMode);
            break;
        }
        case WindowManagerMessage::TRANS_ID_GET_MAXIMIZE_MODE: {
            MaximizeMode maximizeMode = GetMaximizeMode();
            reply.WriteInt32(static_cast<int32_t>(maximizeMode));
            break;
        }
        case WindowManagerMessage::TRANS_ID_GET_FOCUS_WINDOW_INFO: {
            FocusChangeInfo focusInfo;
            uint64_t displayId = 0;
            if (!data.ReadUint64(displayId)) {
                TLOGE(WmsLogTag::WMS_FOCUS, "Failed to read displayId");
                return ERR_INVALID_DATA;
            }
            GetFocusWindowInfo(focusInfo, displayId);
            reply.WriteParcelable(&focusInfo);
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
