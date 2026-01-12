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
#include "session/container/include/zidl/session_stage_stub.h"
#include "session/host/include/session.h"
#include "wm_common.h"
#include <gmock/gmock.h>

namespace OHOS {
namespace Rosen {
class SessionStageMocker : public SessionStageStub {
public:
    SessionStageMocker() {};
    ~SessionStageMocker() {};

    MOCK_METHOD1(SetActive, WSError(bool active));
    MOCK_METHOD4(UpdateRect, WSError(const WSRect& rect, SizeChangeReason reason,
        const SceneAnimationConfig& config, const std::map<AvoidAreaType, AvoidArea>& avoidAreas));
    MOCK_METHOD(WSError, UpdateGlobalDisplayRectFromServer, (const WSRect& rect, SizeChangeReason reason), (override));
    MOCK_METHOD0(UpdateDensity, void(void));
    MOCK_METHOD0(UpdateOrientation, WSError(void));
    MOCK_METHOD1(UpdateSessionViewportConfig, WSError(const SessionViewportConfig& config));
    MOCK_METHOD0(HandleBackEvent, WSError(void));
    MOCK_METHOD0(NotifyDestroy, WSError(void));
    MOCK_METHOD2(UpdateFocus, WSError(const sptr<FocusNotifyInfo>& focusNotifyInfo, bool isFocused));
    MOCK_METHOD1(NotifyTransferComponentData, WSError(const AAFwk::WantParams& wantParams));
    MOCK_METHOD2(NotifyTransferComponentDataSync, WSErrorCode(const AAFwk::WantParams& wantParams,
        AAFwk::WantParams& reWantParams));
    MOCK_METHOD1(MarkProcessed, WSError(int32_t eventId));
    MOCK_METHOD4(NotifyOccupiedAreaChangeInfo, void(sptr<OccupiedAreaChangeInfo> info,
        const std::shared_ptr<RSTransaction>& rsTransaction, const Rect& callingWindowRect,
        const std::map<AvoidAreaType, AvoidArea>& avoidAreas));
    MOCK_METHOD2(UpdateAvoidArea, WSError(const sptr<AvoidArea>& avoidArea, AvoidAreaType type));
    MOCK_METHOD1(DumpSessionElementInfo, void(const std::vector<std::string>& params));
    MOCK_METHOD0(NotifyScreenshot, void(void));
    MOCK_METHOD1(NotifyScreenshotAppEvent, WSError(ScreenshotEventType type));
    MOCK_METHOD0(NotifyTouchOutside, WSError(void));
    MOCK_METHOD1(UpdateWindowMode, WSError(WindowMode mode));
    MOCK_METHOD1(GetTopNavDestinationName, WSError(std::string& topNavDestName));
    MOCK_METHOD1(NotifyLayoutFinishAfterWindowModeChange, WSError(WindowMode mode));
    MOCK_METHOD1(NotifyForegroundInteractiveStatus, void(bool interactive));
    MOCK_METHOD1(UpdateMaximizeMode, WSError(MaximizeMode mode));
    MOCK_METHOD0(NotifyCloseExistPipWindow, WSError(void));
    MOCK_METHOD2(NotifySessionForeground, void(uint32_t reason, bool withAnimation));
    MOCK_METHOD3(NotifySessionBackground, void(uint32_t reason, bool withAnimation, bool isFromInnerkits));
    MOCK_METHOD2(UpdateTitleInTargetPos, WSError(bool isShow, int32_t height));
    MOCK_METHOD2(NotifyDensityFollowHost, WSError(bool isFollowHost, float densityValue));
    MOCK_METHOD1(NotifyWindowVisibility, WSError(bool isVisible));
    MOCK_METHOD1(NotifyWindowOcclusionState, WSError(const WindowVisibilityState state));
    MOCK_METHOD1(NotifyTransformChange, void(const Transform& transform));
    MOCK_METHOD1(NotifySingleHandTransformChange, void(const SingleHandTransform& singleHandTransform));
    MOCK_METHOD1(NotifyDialogStateChange, WSError(bool isForeground));
    MOCK_METHOD1(UpdateDisplayId, WSError(uint64_t displayId));
    MOCK_METHOD2(SetPipActionEvent, WSError(const std::string& action, int32_t status));
    MOCK_METHOD2(SetPiPControlEvent, WSError(WsPiPControlType controlType, WsPiPControlStatus status));
    MOCK_METHOD2(NotifyDisplayMove, void(DisplayId from, DisplayId to));
    MOCK_METHOD1(SwitchFreeMultiWindow, WSError(bool enable));
    MOCK_METHOD2(SetUniqueVirtualPixelRatio, void(bool useUniqueDensity, float virtualPixelRatio));
    MOCK_METHOD1(UpdateAnimationSpeed, void(float speed));
    MOCK_METHOD0(PcAppInPadNormalClose, WSError(void));
    MOCK_METHOD1(NotifyCompatibleModePropertyChange, WSError(const sptr<CompatibleModeProperty> property));
    MOCK_METHOD1(NotifySessionFullScreen, void(bool fullScreen));
    MOCK_METHOD1(GetUIContentRemoteObj, WSError(sptr<IRemoteObject>& uiContentRemoteObj));
    MOCK_METHOD2(NotifyDumpInfo, WSError(const std::vector<std::string>& params,
        std::vector<std::string>& info));
    MOCK_METHOD1(SetSplitButtonVisible, WSError(bool isVisible));
    MOCK_METHOD1(SetEnableDragBySystem, WSError(bool enableDrag));
    MOCK_METHOD2(SendContainerModalEvent, WSError(const std::string& eventName, const std::string& eventValue));
    MOCK_METHOD2(UpdateWindowModeByIdForUITest, WMError(int32_t windowId, int32_t updateMode));
    MOCK_METHOD2(NotifyHighlightChange, WSError(const sptr<HighlightNotifyInfo>& highlightNotifyInfo,
        bool isHighlight));
    MOCK_METHOD1(SetDragActivated, WSError(bool dragActivated));
    MOCK_METHOD3(NotifyPipWindowSizeChange, WSError(double width, double height, double scale));
    MOCK_METHOD1(NotifyPiPActiveStatusChange, WSError(bool status));
    MOCK_METHOD1(NotifyWindowCrossAxisChange, void(CrossAxisState state));
    MOCK_METHOD0(LinkKeyFrameNode, WSError(void));
    MOCK_METHOD1(SetStageKeyFramePolicy, WSError(const KeyFramePolicy& keyFramePolicy));
    MOCK_METHOD1(SetCurrentRotation, WSError(int32_t currentRotation));
    MOCK_METHOD0(NotifyLifecyclePausedStatus, void(void));
    MOCK_METHOD1(NotifyAppUseControlStatus, void(bool isUseControl));
    MOCK_METHOD1(NotifyExtensionSecureLimitChange, WSError(bool isLimit));
    MOCK_METHOD0(NotifyAppForceLandscapeConfigUpdated, WSError(void));
    MOCK_METHOD0(NotifyAppForceLandscapeConfigEnableUpdated, WSError(void));
    MOCK_METHOD0(NotifyAppHookWindowInfoUpdated, WSError(void));
    MOCK_METHOD1(GetRouterStackInfo, WMError(std::string& routerStackInfo));
    MOCK_METHOD1(SendFbActionEvent, WSError(const std::string& action));
    MOCK_METHOD1(UpdateIsShowDecorInFreeMultiWindow, WSError(bool isShow));
    MOCK_METHOD1(UpdateBrightness, WSError(float brightness));
    MOCK_METHOD0(AddSidebarBlur, WSError(void));
    MOCK_METHOD1(SetSidebarBlurStyleWithType, WSError(SidebarBlurType type));
};
} // namespace Rosen
} // namespace OHOS
