/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#ifndef MOCK_KEYBOARD_SESSION_H
#define MOCK_KEYBOARD_SESSION_H

#include "session/host/include/keyboard_session.h"
#include "interfaces/include/ws_common.h"
#include "session/container/include/zidl/session_stage_interface.h"
#include "session/host/include/session.h"
#include <gmock/gmock.h>

namespace OHOS {
namespace Rosen {
class KeyboardSessionMocker : public KeyboardSession {
public:
    KeyboardSessionMocker(const SessionInfo& info, const sptr<SpecificSessionCallback>& specificCallback,
        const sptr<KeyboardSessionCallback>& keyboardCallback)
        : KeyboardSession(info, specificCallback, keyboardCallback) {}
    ~KeyboardSessionMocker() {}

    sptr<SceneSession> GetSceneSession(uint32_t persistentId)
    {
        return callingSession_;
    };
    sptr<SceneSession> callingSession_;
};

using UpdateSessionRectCallBack = std::function<void(const WSRect& rect, SizeChangeReason reason)>;
class KSSceneSessionMocker : public SceneSession {
public:
    KSSceneSessionMocker(const SessionInfo& info, const sptr<SpecificSessionCallback>& specificCallback)
        : SceneSession(info, specificCallback) {}
    ~KSSceneSessionMocker() {}

    WSError UpdateSessionRect(const WSRect& rect, SizeChangeReason reason, bool isGlobal = false,
        bool isFromMoveToGlobal = false, MoveConfiguration moveConfiguration = {}) override
    {
        updateRectCallback_(rect, reason);
        return WSError::WS_OK;
    };

    int32_t GetStatusBarHeight() override
    {
        constexpr int32_t statusBarHeight = 128;
        return statusBarHeight;
    }

    UpdateSessionRectCallBack updateRectCallback_ = [](const WSRect& rect, SizeChangeReason reason) {};
};
} // namespace Rosen
} // namespace OHOS
#endif
