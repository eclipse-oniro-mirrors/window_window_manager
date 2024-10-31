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

#ifndef OHOS_ROSEN_SESSION_IPC_INTERFACE_CODE_H
#define OHOS_ROSEN_SESSION_IPC_INTERFACE_CODE_H

namespace OHOS {
namespace Rosen {
enum class SessionInterfaceCode {
    // Base
    TRANS_ID_CONNECT = 0,
    TRANS_ID_FOREGROUND,
    TRANS_ID_BACKGROUND,
    TRANS_ID_DISCONNECT,
    TRANS_ID_SHOW,
    TRANS_ID_HIDE,
    TRANS_ID_CHANGE_SESSION_VISIBILITY_WITH_STATUS_BAR,
    TRANS_ID_ACTIVE_PENDING_SESSION,
    TRANS_ID_TERMINATE,
    TRANS_ID_EXCEPTION,
    TRANS_ID_DRAWING_COMPLETED,

    // Scene
    TRANS_ID_SESSION_EVENT = 100,
    TRANS_ID_UPDATE_SESSION_RECT,
    TRANS_ID_RAISE_TO_APP_TOP,
    TRANS_ID_BACKPRESSED,
    TRANS_ID_MARK_PROCESSED,
    TRANS_ID_SET_MAXIMIZE_MODE,
    TRANS_ID_GET_MAXIMIZE_MODE,
    TRANS_ID_NEED_AVOID,
    TRANS_ID_GET_AVOID_AREA,
    TRANS_ID_SET_ASPECT_RATIO,
    TRANS_ID_UPDATE_WINDOW_ANIMATION_FLAG,
    TRANS_ID_UPDATE_CUSTOM_ANIMATION,
    TRANS_ID_RAISE_ABOVE_TARGET,
    TRANS_ID_RAISE_APP_MAIN_WINDOW,
    TRANS_ID_PROCESS_POINT_DOWN_SESSION,
    TRANS_ID_SEND_POINTEREVENT_FOR_MOVE_DRAG,
    TRANS_ID_SET_LANDSCAPE_MULTI_WINDOW,
    TRANS_ID_UPDATE_RECTCHANGE_LISTENER_REGISTERED,
    TRANS_ID_SET_CALLING_SESSION_ID,
    TRANS_ID_SET_CUSTOM_DECOR_HEIGHT,
    TRANS_ID_UPDATE_SESSION_PROPERTY,
    TRANS_ID_ADJUST_KEYBOARD_LAYOUT,
    TRANS_ID_LAYOUT_FULL_SCREEN_CHANGE,
    TRANS_ID_GET_FORCE_LANDSCAPE_CONFIG,
    TRANS_ID_GET_STATUSBAR_HEIGHT,
    TRANS_ID_SET_DIALOG_SESSION_BACKGESTURE_ENABLE,
    TRANS_ID_SYSTEM_SESSION_EVENT,
    TRANS_ID_FRAME_LAYOUT_FINISH,
    TRANS_ID_SET_SYSTEM_DRAG_ENABLE,
    TRANS_ID_REQUEST_FOCUS,
    TRANS_ID_GET_START_MOVE_FLAG,
    TRANS_ID_GET_ALL_AVOID_AREAS,
    TRANS_ID_UPDATE_CLIENT_RECT,
    TRANS_ID_RESTORE_MAIN_WINDOW,
    TRANS_ID_TITLE_AND_DOCK_HOVER_SHOW_CHANGE,
    TRANS_ID_SET_GESTURE_BACK_ENABLE,
    TRANS_ID_MODAL_TYPE_CHANGE,
    TRANS_ID_GET_GLOBAL_SCALED_RECT,

    // Extension
    TRANS_ID_TRANSFER_ABILITY_RESULT = 500,
    TRANS_ID_TRANSFER_EXTENSION_DATA = 501,
    TRANS_ID_NOTIFY_EXTENSION_DIED,
    TRANS_ID_NOTIFY_ASYNC_ON,
    TRANS_ID_NOTIFY_SYNC_ON,
    TRANS_ID_NOTIFY_REPORT_ACCESSIBILITY_EVENT,
    TRANS_ID_TRIGGER_BIND_MODAL_UI_EXTENSION,
    TRANS_ID_NOTIFY_EXTENSION_TIMEOUT,
    TRANS_ID_NOTIFY_EXTENSION_EVENT_ASYNC,

    // PictureInPicture
    TRANS_ID_NOTIFY_PIP_WINDOW_PREPARE_CLOSE = 800,
    TRANS_ID_UPDATE_PIP_RECT,
    TRANS_ID_UPDATE_PIP_CONTROL_STATUS,
    TRANS_ID_SET_AUTOSTART_PIP,
};
} // namespace Rosen
} // namespace OHOS

#endif  // OHOS_ROSEN_SESSION_IPC_INTERFACE_CODE_H
