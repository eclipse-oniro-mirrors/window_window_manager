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

#include "interfaces/include/ws_common.h"
#include "session/container/include/zidl/session_stage_interface.h"
#include "session/host/include/session.h"
#include <gmock/gmock.h>

namespace OHOS {
namespace Rosen {
class SessionMocker : public Session {
public:
    SessionMocker(const SessionInfo& info) : Session(info) {}
    ~SessionMocker() {}
    MOCK_METHOD7(Connect, WSError(const sptr<ISessionStage>& sessionStage,
        const sptr<IWindowEventChannel>& eventChannel, const std::shared_ptr<RSSurfaceNode>& surfaceNode,
        SystemSessionConfig& systemConfig, sptr<WindowSessionProperty> property, sptr<IRemoteObject> token,
        const std::string& identityToken));
    MOCK_METHOD7(CreateAndConnectSpecificSession, WSError(const sptr<ISessionStage>& sessionStage,
        const sptr<IWindowEventChannel>& eventChannel, const std::shared_ptr<RSSurfaceNode>& surfaceNode,
        sptr<WindowSessionProperty> property, int32_t& persistentId, sptr<ISession>& session,
        sptr<IRemoteObject> token));

    MOCK_METHOD3(
        Foreground, WSError(sptr<WindowSessionProperty> property, bool isFromClient, const std::string& identityToken));
    MOCK_METHOD2(Background, WSError(bool isFromClient, const std::string& identityToken));
    MOCK_METHOD2(Disconnect, WSError(bool isFromClient, const std::string& identityToken));

    MOCK_METHOD6(UpdateSessionRect, WSError(const WSRect& rect, SizeChangeReason reason,
        bool isGlobal, bool isFromMoveToGlobal, const MoveConfiguration& moveConfiguration,
        const RectAnimationConfig& rectAnimationConfig));
    MOCK_METHOD1(UpdateClientRect, WSError(const WSRect& rect));
    MOCK_METHOD(WSError, UpdateGlobalDisplayRectFromClient, (const WSRect& rect, SizeChangeReason reason), (override));

    MOCK_METHOD0(Recover, WSError(void));
    MOCK_METHOD0(Maximize, WSError(void));
    MOCK_METHOD1(PendingSessionActivation, WSError(const sptr<AAFwk::SessionInfo> info));
    MOCK_METHOD1(UpdateActiveStatus, WSError(bool isActive));
    MOCK_METHOD(WSError, OnSessionEvent, (SessionEvent event, const SessionEventParam& param), (override));
    MOCK_METHOD1(RequestSessionBack, WSError(bool needMoveToBackground));
    MOCK_METHOD0(RaiseToAppTop, WSError(void));
    MOCK_METHOD3(GetAvoidAreaByType, AvoidArea(AvoidAreaType type, const WSRect& rect, int32_t apiVersion));
    MOCK_METHOD1(SetAspectRatio, WSError(float ratio));
    MOCK_METHOD1(ResetAspectRatio, WSError(float ratio));
    MOCK_METHOD1(OnNeedAvoid, WSError(bool status));
    MOCK_METHOD1(SetGlobalMaximizeMode, WSError(MaximizeMode mode));
    MOCK_METHOD1(NotifyExtensionTimeout, void(int32_t errorCode));
    MOCK_METHOD1(GetAppForceLandscapeConfig, WMError(AppForceLandscapeConfig& config));
    MOCK_METHOD1(GetAppHookWindowInfoFromServer, WMError(HookWindowInfo& hookWindowInfo));
    MOCK_METHOD1(SetDialogSessionBackGestureEnabled, WSError(bool isEnabled));
    MOCK_METHOD1(SetActive, WSError(bool active));
    MOCK_METHOD1(SyncSessionEvent, WSError(SessionEvent event));
    MOCK_METHOD0(RemoveStartingWindow, WSError(void));
    MOCK_METHOD1(GetGlobalMaximizeMode, WSError(MaximizeMode& mode));
    MOCK_METHOD2(UpdateSessionPropertyByAction, WMError(const sptr<WindowSessionProperty>& property,
        WSPropertyChangeAction action));
    MOCK_METHOD1(TransferExtensionData, int32_t(const AAFwk::WantParams& wantParams));
    MOCK_METHOD1(RaiseMainWindowAboveTarget, WSError(int32_t targetId));
    MOCK_METHOD(WSError, ProcessPointDownSession, (int32_t x, int32_t y), (override));
    MOCK_CONST_METHOD2(ConvertGlobalRectToRelative, WSRect(const WSRect& globalRect, DisplayId targetDisplayId));
    MOCK_METHOD1(SetIsShowDecorInFreeMultiWindow, WSError(bool isShow));
    MOCK_METHOD(WSError, SetContentAspectRatio, (float ratio, bool isPersistent, bool needUpdateRect), (override));
    MOCK_METHOD1(GetGlobalScaledRect, WMError(Rect& globalScaledRect));
};
} // namespace Rosen
} // namespace OHOS
