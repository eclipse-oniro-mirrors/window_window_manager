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

#include "session/host/include/zidl/session_stub.h"

#include "ability_start_setting.h"
#include <ipc_types.h>
#include "want.h"

#include "session/host/include/zidl/session_ipc_interface_code.h"
#include <gmock/gmock.h>

namespace OHOS {
namespace Rosen {
class SessionStubMocker : public SessionStub {
public:
    SessionStubMocker() {};
    ~SessionStubMocker() {};

    MOCK_METHOD7(Connect, WSError(const sptr<ISessionStage>& sessionStage,
        const sptr<IWindowEventChannel>& eventChannel, const std::shared_ptr<RSSurfaceNode>& surfaceNode,
        SystemSessionConfig& systemConfig, sptr<WindowSessionProperty> property, sptr<IRemoteObject> token,
        const std::string& identityToken));

    MOCK_METHOD3(
        Foreground, WSError(sptr<WindowSessionProperty> property, bool isFromClient, const std::string& identityToken));
    MOCK_METHOD2(Background, WSError(bool isFromClient, const std::string& identityToken));
    MOCK_METHOD2(Disconnect, WSError(bool isFromClient, const std::string& identityToken));
    MOCK_METHOD1(Show, WSError(sptr<WindowSessionProperty> property));
    MOCK_METHOD0(Hide, WSError(void));
    MOCK_METHOD0(DrawingCompleted, WSError(void));
    MOCK_METHOD0(RemoveStartingWindow, WSError(void));
    MOCK_METHOD(WSError, OnSessionEvent, (SessionEvent event, const SessionEventParam& param), (override));

    MOCK_METHOD4(OnRemoteRequest, int(uint32_t code, MessageParcel& data, MessageParcel& reply, MessageOption& option));

    MOCK_METHOD2(HandleConnect, int(MessageParcel& data, MessageParcel& reply));
    MOCK_METHOD2(HandleForeground, int(MessageParcel& data, MessageParcel& reply));
    MOCK_METHOD2(HandleBackground, int(MessageParcel& data, MessageParcel& reply));
    MOCK_METHOD2(HandleDisconnect, int(MessageParcel& data, MessageParcel& reply));
    MOCK_METHOD2(HandleSessionEvent, int(MessageParcel& data, MessageParcel& reply));
    MOCK_METHOD2(HandleUpdateSessionRect, int(MessageParcel& data, MessageParcel& reply));
    MOCK_METHOD2(HandleRaiseToAppTop, int(MessageParcel& data, MessageParcel& reply));
    MOCK_METHOD2(HandleBackPressed, int(MessageParcel& data, MessageParcel& reply));
    MOCK_METHOD2(HandleMarkProcessed, int(MessageParcel& data, MessageParcel& reply));
    MOCK_METHOD2(HandleSetGlobalMaximizeMode, int(MessageParcel& data, MessageParcel& reply));
    MOCK_METHOD2(HandleGetGlobalMaximizeMode, int(MessageParcel& data, MessageParcel& reply));
    MOCK_METHOD2(HandleNeedAvoid, int(MessageParcel& data, MessageParcel& reply));
    MOCK_METHOD2(HandleGetAvoidAreaByType, int(MessageParcel& data, MessageParcel& reply));
    MOCK_METHOD2(HandleSetSessionProperty, int(MessageParcel& data, MessageParcel& reply));
    MOCK_METHOD2(HandleSetAspectRatio, int(MessageParcel& data, MessageParcel& reply));
    MOCK_METHOD2(HandleSetWindowAnimationFlag, int(MessageParcel& data, MessageParcel& reply));
    MOCK_METHOD2(HandleUpdateWindowSceneAfterCustomAnimation, int(MessageParcel& data, MessageParcel& reply));
    MOCK_METHOD2(HandleRaiseAboveTarget, int(MessageParcel& data, MessageParcel& reply));
    MOCK_METHOD2(HandleRaiseMainWindowAboveTarget, int(MessageParcel& data, MessageParcel& reply));
    MOCK_METHOD2(HandleRaiseAppMainWindowToTop, int(MessageParcel& data, MessageParcel& reply));
    MOCK_METHOD2(HandlePendingSessionActivation, int(MessageParcel& data, MessageParcel& reply));
    MOCK_METHOD2(HandleTerminateSession, int(MessageParcel& data, MessageParcel& reply));
    MOCK_METHOD2(HandleSessionException, int(MessageParcel& data, MessageParcel& reply));
    MOCK_METHOD2(HandleTransferAbilityResult, int(MessageParcel& data, MessageParcel& reply));
    MOCK_METHOD2(HandleTransferExtensionData, int(MessageParcel& data, MessageParcel& reply));
    MOCK_METHOD2(HandleNotifyExtensionDied, int(MessageParcel& data, MessageParcel& reply));
    MOCK_METHOD2(HandleNotifyExtensionTimeout, int(MessageParcel& data, MessageParcel& reply));
    MOCK_METHOD2(HandleGetStatusBarHeight, int(MessageParcel& data, MessageParcel& reply));
    MOCK_METHOD2(HandleGetAppForceLandscapeConfig, int(MessageParcel& data, MessageParcel& reply));
    MOCK_METHOD2(HandleNotifySecureLimitChange, int(MessageParcel& data, MessageParcel& reply));
    MOCK_METHOD2(HandleGetAllAvoidAreas, int(MessageParcel& data, MessageParcel& reply));
    MOCK_METHOD3(GetAvoidAreaByType, AvoidArea(AvoidAreaType type, const WSRect& rect, int32_t apiVersion));
    MOCK_METHOD1(GetAllAvoidAreas, WSError(std::map<AvoidAreaType, AvoidArea>& avoidAreas));
    MOCK_METHOD2(UpdateSessionPropertyByAction, WMError(const sptr<WindowSessionProperty>& property,
        WSPropertyChangeAction action));
    MOCK_METHOD1(GetCrossAxisState, WSError(CrossAxisState& state));
    MOCK_METHOD1(GetWaterfallMode, WSError(bool& isWaterfallMode));
    MOCK_METHOD1(IsMainWindowFullScreenAcrossDisplays, WMError(bool& isAcrossDisplays));
    MOCK_METHOD1(GetFloatingBallWindowId, WMError(uint32_t& windowId));
    MOCK_METHOD1(SendFbActionEvent, WSError(const std::string& action));
    MOCK_METHOD1(RestoreFbMainWindow, WMError(const std::shared_ptr<AAFwk::Want>& want));
    MOCK_METHOD0(NotifyFloatViewPrepareClose, void(void));
    MOCK_METHOD1(UpdateFloatView, WMError(const FloatViewTemplateInfo& fvTemplateInfo));
    MOCK_METHOD1(RestoreFloatViewMainWindow, WMError(const std::shared_ptr<AAFwk::WantParams>& wantParams));
    MOCK_METHOD1(UpdateIsShowDecorInFreeMultiWindow, WSError(bool& isShow));
    MOCK_METHOD(WSError, SetContentAspectRatio, (float ratio, bool isPersistent, bool needUpdateRect), (override));
    MOCK_METHOD(WSError, SetDecorVisible, (bool isVisible), (override));
    MOCK_METHOD(WMError, SetFloatNavigationEnabled, (bool enable), (override));
};
} // namespace Rosen
} // namespace OHOS
