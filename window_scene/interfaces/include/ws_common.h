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

#ifndef OHOS_ROSEN_WINDOW_SCENE_WS_COMMON_H
#define OHOS_ROSEN_WINDOW_SCENE_WS_COMMON_H

#include <inttypes.h>
#include <map>
#include <string>
#include <want.h>

#include "iremote_broker.h"

namespace OHOS::Rosen {
constexpr uint64_t INVALID_SESSION_ID = 0;

enum class WSError : int32_t {
    WS_OK = 0,
    WS_DO_NOTHING,
    WS_ERROR_NO_MEM,
    WS_ERROR_DESTROYED_OBJECT,
    WS_ERROR_INVALID_WINDOW,
    WS_ERROR_INVALID_WINDOW_MODE_OR_SIZE,
    WS_ERROR_INVALID_OPERATION,
    WS_ERROR_INVALID_PERMISSION,
    WS_ERROR_NOT_SYSTEM_APP,
    WS_ERROR_NO_REMOTE_ANIMATION,
    WS_ERROR_INVALID_DISPLAY,
    WS_ERROR_INVALID_PARENT,
    WS_ERROR_OPER_FULLSCREEN_FAILED,
    WS_ERROR_REPEAT_OPERATION,
    WS_ERROR_INVALID_SESSION,

    WS_ERROR_DEVICE_NOT_SUPPORT = 801, // the value do not change.It is defined on all system

    WS_ERROR_NEED_REPORT_BASE = 1000, // error code > 1000 means need report
    WS_ERROR_NULLPTR,
    WS_ERROR_INVALID_TYPE,
    WS_ERROR_INVALID_PARAM,
    WS_ERROR_SAMGR,
    WS_ERROR_IPC_FAILED,
    WS_ERROR_NEED_REPORT_END,
    WS_ERROR_START_ABILITY_FAILED,
};

enum class WSErrorCode : int32_t {
    WS_OK = 0,
    WS_ERROR_NO_PERMISSION = 201,
    WS_ERROR_INVALID_PARAM = 401,
    WS_ERROR_DEVICE_NOT_SUPPORT = 801,
    WS_ERROR_REPEAT_OPERATION = 1300001,
    WS_ERROR_STATE_ABNORMALLY = 1300002,
    WS_ERROR_SYSTEM_ABNORMALLY = 1300003,
    WS_ERROR_INVALID_CALLING = 1300004,
    WS_ERROR_STAGE_ABNORMALLY = 1300005,
    WS_ERROR_CONTEXT_ABNORMALLY = 1300006,
};

const std::map<WSError, WSErrorCode> WS_JS_TO_ERROR_CODE_MAP {
    { WSError::WS_OK,                    WSErrorCode::WS_OK },
    { WSError::WS_DO_NOTHING,            WSErrorCode::WS_ERROR_STATE_ABNORMALLY },
    { WSError::WS_ERROR_INVALID_SESSION, WSErrorCode::WS_ERROR_STATE_ABNORMALLY },
    { WSError::WS_ERROR_IPC_FAILED,      WSErrorCode::WS_ERROR_SYSTEM_ABNORMALLY },
    { WSError::WS_ERROR_NULLPTR,         WSErrorCode::WS_ERROR_STATE_ABNORMALLY },
};

enum class SessionState : uint32_t {
    STATE_DISCONNECT = 0,
    STATE_CONNECT,
    STATE_FOREGROUND,
    STATE_ACTIVE,
    STATE_INACTIVE,
    STATE_BACKGROUND,
    STATE_END,
};

struct SessionInfo {
    std::string bundleName_ = "";
    std::string moduleName_ = "";
    std::string abilityName_ = "";
    sptr<IRemoteObject> callerToken_ = nullptr;
    bool isSystem_ = false;

    sptr<AAFwk::Want> want;
    int32_t resultCode;
    int32_t requestCode;
    int32_t errorCode;
    std::string errorReason;

    uint64_t persistentId_ = INVALID_SESSION_ID;
    uint64_t callerPersistentId_ = INVALID_SESSION_ID;
    uint32_t callState_ = 0;
    int64_t uiAbilityId_ = 0;
};

enum class SizeChangeReason : uint32_t {
    UNDEFINED = 0,
    MAXIMIZE,
    RECOVER,
    ROTATION,
    DRAG,
    DRAG_START,
    DRAG_END,
    RESIZE,
    MOVE,
    HIDE,
    TRANSFORM,
    CUSTOM_ANIMATION_SHOW,
    FULL_TO_SPLIT,
    SPLIT_TO_FULL,
    END,
};

enum class SessionEvent : uint32_t {
    EVENT_MAXIMIZE = 100,
    EVENT_RECOVER,
    EVENT_MINIMIZE,
    EVENT_CLOSE,
    EVENT_START_MOVE,
    EVENT_TERMINATE,
    EVENT_EXCEPTION,
};

struct WSRect {
    int32_t posX_ = 0;
    int32_t posY_ = 0;
    uint32_t width_ = 0;
    uint32_t height_ = 0;

    bool operator==(const WSRect& a) const
    {
        return (posX_ == a.posX_ && posY_ == a.posY_ && width_ == a.width_ && height_ == a.height_);
    }

    bool operator!=(const WSRect& a) const
    {
        return !this->operator==(a);
    }
};

struct WindowShadowConfig {
    float offsetX_ = 0.0f;
    float offsetY_ = 0.0f;
    float alpha_ = 0.0f;
    float radius_ = 0.0f;
    std::string color_ = "#000000";
};

struct KeyboardSceneAnimationConfig {
    std::string curveType_ = "easeOut";
    float ctrlX1_ = 0.2f;
    float ctrlY1_ = 0.0f;
    float ctrlX2_ = 0.2f;
    float ctrlY2_ = 1.0f;
    uint32_t durationIn_ = 150; // default durationIn time
    uint32_t durationOut_ = 150; // default durationOut time
};

struct AppWindowSceneConfig {
    float floatCornerRadius_ = 0.0f;

    WindowShadowConfig focusedShadow_;
    WindowShadowConfig unfocusedShadow_;
    KeyboardSceneAnimationConfig keyboardAnimation_;
};

} // namespace OHOS::Rosen
#endif // OHOS_ROSEN_WINDOW_SCENE_WS_COMMON_H
