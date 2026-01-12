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

#ifndef OHOS_WINDOW_SCENE_SESSION_STAGE_RPOXY_H
#define OHOS_WINDOW_SCENE_SESSION_STAGE_RPOXY_H

#include <iremote_proxy.h>
#include <transaction/rs_transaction.h>
#include <feature/window_keyframe/rs_window_keyframe_node.h>

#include "interfaces/include/ws_common.h"
#include "session/container/include/zidl/session_stage_interface.h"
#include "ws_common.h"

namespace OHOS::Rosen {
class SessionStageProxy : public IRemoteProxy<ISessionStage> {
public:
    explicit SessionStageProxy(const sptr<IRemoteObject>& impl) : IRemoteProxy<ISessionStage>(impl) {};

    ~SessionStageProxy() {};

    WSError SetActive(bool active) override;
    WSError UpdateRect(const WSRect& rect, SizeChangeReason reason,
        const SceneAnimationConfig& config = { nullptr, ROTATE_ANIMATION_DURATION,
            0, WindowAnimationCurve::LINEAR, {0.0f, 0.0f, 0.0f, 0.0f} },
        const std::map<AvoidAreaType, AvoidArea>& avoidAreas = {}) override;
    WSError UpdateGlobalDisplayRectFromServer(const WSRect& rect, SizeChangeReason reason) override;
    WSError UpdateOrientation() override;
    WSError UpdateSessionViewportConfig(const SessionViewportConfig& config) override;
    WSError HandleBackEvent() override;
    WSError MarkProcessed(int32_t eventId) override;
    WSError UpdateFocus(const sptr<FocusNotifyInfo>& focusNotifyInfo, bool isFocused) override;
    WSError NotifyDestroy() override;
    WSError NotifyCloseExistPipWindow() override;
    WSError NotifyTransferComponentData(const AAFwk::WantParams& wantParams) override;
    WSErrorCode NotifyTransferComponentDataSync(const AAFwk::WantParams& wantParams,
                                                AAFwk::WantParams& reWantParams) override;
    void NotifyOccupiedAreaChangeInfo(sptr<OccupiedAreaChangeInfo> info,
        const std::shared_ptr<RSTransaction>& rsTransaction, const Rect& callingSessionRect,
        const std::map<AvoidAreaType, AvoidArea>& avoidAreas) override;
    WSError UpdateAvoidArea(const sptr<AvoidArea>& avoidArea, AvoidAreaType type) override;
    void NotifyScreenshot() override;
    WSError NotifyScreenshotAppEvent(ScreenshotEventType type) override;
    void DumpSessionElementInfo(const std::vector<std::string>& params)  override;
    WSError NotifyTouchOutside() override;
    WSError NotifyWindowVisibility(bool isVisible) override;
    WSError NotifyWindowOcclusionState(const WindowVisibilityState state) override;
    WSError UpdateWindowMode(WindowMode mode) override;
    WSError GetTopNavDestinationName(std::string& topNavDestName) override;
    WSError NotifyLayoutFinishAfterWindowModeChange(WindowMode mode) override;
    WMError UpdateWindowModeForUITest(int32_t updateMode) override;
    void NotifyForegroundInteractiveStatus(bool interactive) override;
    WSError UpdateMaximizeMode(MaximizeMode mode) override;
    void NotifySessionForeground(uint32_t reason, bool withAnimation) override;
    void NotifySessionBackground(uint32_t reason, bool withAnimation, bool isFromInnerkits) override;
    WSError NotifyDensityFollowHost(bool isFollowHost, float densityValue) override;
    WSError UpdateTitleInTargetPos(bool isShow, int32_t height) override;
    void NotifyTransformChange(const Transform& transform) override;
    void NotifySingleHandTransformChange(const SingleHandTransform& singleHandTransform) override;
    WSError NotifyDialogStateChange(bool isForeground) override;
    WSError SetPipActionEvent(const std::string& action, int32_t status) override;
    WSError SetPiPControlEvent(WsPiPControlType controlType, WsPiPControlStatus status) override;
    WSError NotifyPipWindowSizeChange(double width, double height, double scale) override;
    WSError NotifyPiPActiveStatusChange(bool status) override;
    WSError UpdateDisplayId(uint64_t displayId) override;
    void NotifyDisplayMove(DisplayId from, DisplayId to) override;
    WSError SwitchFreeMultiWindow(bool enable) override;
    WSError GetUIContentRemoteObj(sptr<IRemoteObject>& uiContentRemoteObj) override;
    void NotifyKeyboardPanelInfoChange(const KeyboardPanelInfo& keyboardPanelInfo) override;
    WSError PcAppInPadNormalClose() override;
    WSError NotifyCompatibleModePropertyChange(const sptr<CompatibleModeProperty> property) override;
    void SetUniqueVirtualPixelRatio(bool useUniqueDensity, float virtualPixelRatio) override;
    void UpdateAnimationSpeed(float speed) override;
    void NotifySessionFullScreen(bool fullScreen) override;
    WSError NotifyTargetRotationInfo(OrientationInfo& info, OrientationInfo& currentInfo) override;
    WSError NotifyPageRotationIsIgnored() override;
    RotationChangeResult NotifyRotationChange(const RotationChangeInfo& rotationChangeInfo) override;

    // UIExtension
    WSError NotifyDumpInfo(const std::vector<std::string>& params, std::vector<std::string>& info) override;
    WSError SendExtensionData(MessageParcel& data, MessageParcel& reply, MessageOption& option) override;

    WSError LinkKeyFrameNode() override;
    WSError SetStageKeyFramePolicy(const KeyFramePolicy& keyFramePolicy) override;

    WSError SetDragActivated(bool dragActivated) override;
    WSError SetSplitButtonVisible(bool isVisible) override;
    WSError SetEnableDragBySystem(bool dragEnable) override;
    WSError SetFullScreenWaterfallMode(bool isWaterfallMode) override;
    WSError SetSupportEnterWaterfallMode(bool isSupportEnter) override;
    WSError SendContainerModalEvent(const std::string& eventName, const std::string& eventValue) override;
    WSError NotifyExtensionSecureLimitChange(bool isLimit) override;
    WSError NotifyHighlightChange(const sptr<HighlightNotifyInfo>& highlightNotifyInfo, bool isHighlight) override;
    void NotifyWindowCrossAxisChange(CrossAxisState state) override;
    WSError NotifyWindowAttachStateChange(bool isAttach) override;
    void NotifyKeyboardAnimationCompleted(const KeyboardPanelInfo& keyboardPanelInfo) override;
    void NotifyKeyboardAnimationWillBegin(const KeyboardAnimationInfo& keyboardAnimationInfo,
        const std::shared_ptr<RSTransaction>& rsTransaction) override;
    WSError SetCurrentRotation(int32_t currentRotation) override;
    WSError NotifyAppForceLandscapeConfigUpdated() override;
    WSError NotifyAppForceLandscapeConfigEnableUpdated() override;
    WSError NotifyAppHookWindowInfoUpdated() override;
    WSError CloseSpecificScene() override;

    // Window LifeCycle
    void NotifyLifecyclePausedStatus() override;
    void NotifyAppUseControlStatus(bool isUseControl) override;
    WMError GetRouterStackInfo(std::string& routerStackInfo) override;
    WSError SendFbActionEvent(const std::string& action) override;

    WSError UpdateIsShowDecorInFreeMultiWindow(bool isShow) override;

    // Window Property
    WSError UpdateBrightness(float brightness) override;
    void UpdateDensity() override;
    WSError AddSidebarBlur() override;
    WSError SetSidebarBlurStyleWithType(SidebarBlurType type) override;

private:
    static inline BrokerDelegator<SessionStageProxy> delegator_;
};
} // namespace OHOS::Rosen
#endif // OHOS_WINDOW_SCENE_SESSION_STAGE_RPOXY_H
