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

#ifndef OHOS_WINDOW_ADAPTER_H
#define OHOS_WINDOW_ADAPTER_H

#include <refbase.h>
#include <zidl/window_manager_agent_interface.h>
#include "common/include/window_session_property.h"
#include "singleton_delegator.h"
#include "window.h"
#include "window_property.h"
#include "wm_single_instance.h"
#include "zidl/window_interface.h"
#include "zidl/window_manager_interface.h"

namespace OHOS {
namespace Rosen {
class WMSDeathRecipient : public IRemoteObject::DeathRecipient {
public:
    WMSDeathRecipient(const int32_t userId = INVALID_USER_ID);
    virtual void OnRemoteDied(const wptr<IRemoteObject>& wptrDeath) override;

private:
    int32_t userId_;
};

class WindowAdapter : public RefBase {
WM_DECLARE_SINGLE_INSTANCE_BASE(WindowAdapter);
public:
    static WindowAdapter& GetInstance(const int32_t userId);

    using SessionRecoverCallbackFunc = std::function<WMError()>;
    using UIEffectRecoverCallbackFunc = std::function<WMError()>;
    using WMSConnectionChangedCallbackFunc = std::function<void(int32_t, int32_t, bool)>;
    using OutlineRecoverCallbackFunc = std::function<WMError()>;
    virtual WMError CreateWindow(sptr<IWindow>& window, sptr<WindowProperty>& windowProperty,
        std::shared_ptr<RSSurfaceNode> surfaceNode, uint32_t& windowId, const sptr<IRemoteObject>& token);
    virtual WMError AddWindow(sptr<WindowProperty>& windowProperty);
    virtual WMError RemoveWindow(uint32_t windowId, bool isFromInnerkits);
    virtual WMError DestroyWindow(uint32_t windowId);
    virtual WMError RequestFocus(uint32_t windowId);
    virtual WMError GetAvoidAreaByType(uint32_t windowId, AvoidAreaType type, AvoidArea& avoidRect,
        const Rect& rect = Rect::EMPTY_RECT);
    virtual WMError GetTopWindowId(uint32_t mainWinId, uint32_t& topWinId);
    virtual WMError GetParentMainWindowId(int32_t windowId, int32_t& mainWindowId);
    virtual void NotifyServerReadyToMoveOrDrag(uint32_t windowId, sptr<WindowProperty>& windowProperty,
        sptr<MoveDragProperty>& moveDragProperty);
    virtual void ProcessPointDown(uint32_t windowId, bool isPointDown = true);
    virtual void ProcessPointUp(uint32_t windowId);
    virtual WMError MinimizeAllAppWindows(DisplayId displayId, int32_t excludeWindowId = 0);
    virtual WMError ToggleShownStateForAllAppWindows();
    virtual WMError SetWindowLayoutMode(WindowLayoutMode mode);
    virtual WMError UpdateProperty(sptr<WindowProperty>& windowProperty, PropertyChangeAction action);
    virtual WMError SetWindowGravity(uint32_t windowId, WindowGravity gravity, uint32_t percent);
    virtual WMError GetSystemConfig(SystemConfig& systemConfig);
    virtual WMError GetModeChangeHotZones(DisplayId displayId, ModeChangeHotZones& hotZones);
    virtual WMError UpdateRsTree(uint32_t windowId, bool isAdd);
    virtual WMError BindDialogTarget(uint32_t& windowId, sptr<IRemoteObject> targetToken);
    virtual WMError RegisterWindowManagerAgent(WindowManagerAgentType type,
        const sptr<IWindowManagerAgent>& windowManagerAgent);
    void RegisterWindowManagerAgentWhenSCBFault(WindowManagerAgentType type,
        const sptr<IWindowManagerAgent>& windowManagerAgent);
    virtual WMError UnregisterWindowManagerAgent(WindowManagerAgentType type,
        const sptr<IWindowManagerAgent>& windowManagerAgent);
    virtual WMError RegisterWindowPropertyChangeAgent(WindowInfoKey windowInfoKey, uint32_t interestInfo,
        const sptr<IWindowManagerAgent>& windowManagerAgent);
    virtual WMError UnregisterWindowPropertyChangeAgent(WindowInfoKey windowInfoKey, uint32_t interestInfo,
        const sptr<IWindowManagerAgent>& windowManagerAgent);
    virtual WMError CheckWindowId(int32_t windowId, int32_t& pid);

    virtual WMError SetWindowAnimationController(const sptr<RSIWindowAnimationController>& controller);
    virtual WMError NotifyWindowTransition(sptr<WindowTransitionInfo> from, sptr<WindowTransitionInfo> to);
    virtual WMError UpdateAvoidAreaListener(uint32_t windowId, bool haveListener);
    virtual void ClearWindowAdapter();

    virtual WMError GetAccessibilityWindowInfo(std::vector<sptr<AccessibilityWindowInfo>>& infos);
    virtual WMError ConvertToRelativeCoordinateExtended(const Rect& rect, Rect& newRect, DisplayId& newDisplayId);
    virtual WMError GetUnreliableWindowInfo(int32_t windowId, std::vector<sptr<UnreliableWindowInfo>>& infos);
    virtual WMError ListWindowInfo(const WindowInfoOption& windowInfoOption, std::vector<sptr<WindowInfo>>& infos);
    virtual WMError GetAllWindowLayoutInfo(DisplayId displayId, std::vector<sptr<WindowLayoutInfo>>& infos);
    virtual WMError GetAllMainWindowInfo(std::vector<sptr<MainWindowInfo>>& infos);
    virtual WMError GetMainWindowSnapshot(const std::vector<int32_t>& windowIds,
        const WindowSnapshotConfiguration& config, const sptr<IRemoteObject>& callback);
    virtual WMError GetGlobalWindowMode(DisplayId displayId, GlobalWindowMode& globalWinMode);
    virtual WMError GetTopNavDestinationName(int32_t windowId, std::string& topNavDestName);
    virtual WMError GetVisibilityWindowInfo(std::vector<sptr<WindowVisibilityInfo>>& infos);
    virtual void MinimizeWindowsByLauncher(std::vector<uint32_t> windowIds, bool isAnimated,
        sptr<RSIWindowAnimationFinishedCallback>& finishCallback);
    virtual void SetAnchorAndScale(int32_t x, int32_t y, float scale);
    virtual void SetAnchorOffset(int32_t deltaX, int32_t deltaY);
    virtual void OffWindowZoom();
    virtual WMError RaiseToAppTop(uint32_t windowId);
    virtual std::shared_ptr<Media::PixelMap> GetSnapshot(int32_t windowId);
    virtual WMError SetGestureNavigationEnabled(bool enable);
    virtual void DispatchKeyEvent(uint32_t windowId, std::shared_ptr<MMI::KeyEvent> event);
    virtual void NotifyDumpInfoResult(const std::vector<std::string>& info);
    virtual WMError DumpSessionAll(std::vector<std::string>& infos);
    virtual WMError DumpSessionWithId(int32_t persistentId, std::vector<std::string>& infos);
    virtual WMError GetUIContentRemoteObj(int32_t persistentId, sptr<IRemoteObject>& uiContentRemoteObj);
    virtual WMError GetRootUIContentRemoteObj(DisplayId displayId, sptr<IRemoteObject>& uiContentRemoteObj);
    virtual WMError GetWindowAnimationTargets(std::vector<uint32_t> missionIds,
        std::vector<sptr<RSWindowAnimationTarget>>& targets);
    virtual void SetMaximizeMode(MaximizeMode maximizeMode);
    virtual MaximizeMode GetMaximizeMode();
    virtual void GetFocusWindowInfo(FocusChangeInfo& focusInfo, DisplayId displayId = DEFAULT_DISPLAY_ID);
    virtual void GetFocusWindowInfoByAbilityToken(FocusChangeInfo& focusInfo, const sptr<IRemoteObject>& abilityToken);
    virtual WMError UpdateSessionAvoidAreaListener(int32_t persistentId, bool haveListener);
    virtual WMError UpdateSessionTouchOutsideListener(int32_t& persistentId, bool haveListener);
    virtual WMError NotifyWindowExtensionVisibilityChange(int32_t pid, int32_t uid, bool visible);
    virtual WMError UpdateSessionWindowVisibilityListener(int32_t persistentId, bool haveListener);
    virtual WMError UpdateSessionOcclusionStateListener(int32_t persistentId, bool haveListener);
    virtual WMError RaiseWindowToTop(int32_t persistentId);
    virtual WMError ShiftAppWindowFocus(int32_t sourcePersistentId, int32_t targetPersistentId);
    virtual WMError SetSpecificWindowZIndex(WindowType windowType, int32_t zIndex, bool updateMap = true);
    virtual void CreateAndConnectSpecificSession(const sptr<ISessionStage>& sessionStage,
        const sptr<IWindowEventChannel>& eventChannel, const std::shared_ptr<RSSurfaceNode>& surfaceNode,
        sptr<WindowSessionProperty> property, int32_t& persistentId, sptr<ISession>& session,
        SystemSessionConfig& systemConfig, sptr<IRemoteObject> token = nullptr);
    virtual WMError DestroyAndDisconnectSpecificSession(const int32_t persistentId);
    virtual WMError DestroyAndDisconnectSpecificSessionWithDetachCallback(const int32_t persistentId,
        const sptr<IRemoteObject>& callback);
    WMError GetSnapshotByWindowId(int32_t windowId, std::shared_ptr<Media::PixelMap>& pixelMap);
    WMError RegisterWMSConnectionChangedListener(const WMSConnectionChangedCallbackFunc& callbackFunc);
    WMError UnregisterWMSConnectionChangedListener();
    virtual WMError SetSessionGravity(int32_t persistentId, SessionGravity gravity, uint32_t percent);
    virtual WMError BindDialogSessionTarget(uint64_t persistentId, sptr<IRemoteObject> targetToken);
    virtual WMError RequestFocusStatus(int32_t persistentId, bool isFocused);
    virtual WMError RequestFocusStatusBySA(int32_t persistentId, bool isFocused = true,
        bool byForeground = true, FocusChangeReason reason = FocusChangeReason::SA_REQUEST);
    virtual void AddExtensionWindowStageToSCB(const sptr<ISessionStage>& sessionStage,
        const sptr<IRemoteObject>& token, uint64_t surfaceNodeId, int64_t startModalExtensionTimeStamp,
        bool isConstrainedModal = false);
    virtual void RemoveExtensionWindowStageFromSCB(const sptr<ISessionStage>& sessionStage,
        const sptr<IRemoteObject>& token, bool isConstrainedModal = false);
    virtual void UpdateModalExtensionRect(const sptr<IRemoteObject>& token, Rect rect);
    virtual void ProcessModalExtensionPointDown(const sptr<IRemoteObject>& token, int32_t posX, int32_t posY);
    virtual WMError AddOrRemoveSecureSession(int32_t persistentId, bool shouldHide);
    virtual WMError UpdateExtWindowFlags(const sptr<IRemoteObject>& token, uint32_t extWindowFlags,
        uint32_t extWindowActions);
    virtual WMError GetHostWindowRect(int32_t hostWindowId, Rect& rect);
    virtual WMError GetHostGlobalScaledRect(int32_t hostWindowId, Rect& globalScaledRect);
    virtual WMError GetFreeMultiWindowEnableState(bool& enable);
    virtual WMError GetCallingWindowWindowStatus(uint32_t callingWindowId, WindowStatus& windowStatus);
    virtual WMError GetCallingWindowRect(uint32_t callingWindowId, Rect& rect);
    virtual WMError GetWindowModeType(WindowModeType& windowModeType);
    virtual WMError GetWindowStyleType(WindowStyleType& windowStyleType);
    virtual WMError SkipSnapshotForAppProcess(int32_t pid, bool skip);
    virtual WMError SetProcessWatermark(int32_t pid, const std::string& watermarkName, bool isEnabled);
    virtual WMError GetWindowIdsByCoordinate(DisplayId displayId, int32_t windowNumber,
        int32_t x, int32_t y, std::vector<int32_t>& windowIds);
    virtual WMError UpdateScreenLockStatusForApp(const std::string& bundleName, bool isRelease);
    virtual WMError NotifyWatchGestureConsumeResult(int32_t keyCode, bool isConsumed);
    virtual WMError NotifyWatchFocusActiveChange(bool isActive);
    virtual WMError MinimizeByWindowId(const std::vector<int32_t>& windowIds);
    virtual WMError SetForegroundWindowNum(uint32_t windowNum);
    virtual WMError SetStartWindowBackgroundColor(
        const std::string& moduleName, const std::string& abilityName, uint32_t color, int32_t uid);

    /*
     * Window Recover
     */
    void RegisterSessionRecoverCallbackFunc(int32_t persistentId, const SessionRecoverCallbackFunc& callbackFunc);
    void UnregisterSessionRecoverCallbackFunc(int32_t persistentId);
    virtual WMError RecoverAndReconnectSceneSession(const sptr<ISessionStage>& sessionStage,
        const sptr<IWindowEventChannel>& eventChannel, const std::shared_ptr<RSSurfaceNode>& surfaceNode,
        sptr<ISession>& session, sptr<WindowSessionProperty> property, sptr<IRemoteObject> token = nullptr);
    virtual void RecoverAndConnectSpecificSession(const sptr<ISessionStage>& sessionStage,
        const sptr<IWindowEventChannel>& eventChannel, const std::shared_ptr<RSSurfaceNode>& surfaceNode,
        sptr<WindowSessionProperty> property, sptr<ISession>& session, sptr<IRemoteObject> token = nullptr);
    virtual void RegisterUIEffectRecoverCallbackFunc(int32_t id,
        const UIEffectRecoverCallbackFunc& callbackFunc);
    virtual void UnregisterUIEffectRecoverCallbackFunc(int32_t id);

    /*
     * PC Window
     */
    virtual WMError IsPcWindow(bool& isPcWindow);
    virtual WMError IsFreeMultiWindowMode(bool& isFreeMultiWindow);
    virtual WMError IsPcOrPadFreeMultiWindowMode(bool& isPcOrPadFreeMultiWindowMode);
    virtual WMError IsWindowRectAutoSave(const std::string& key, bool& enabled, int persistentId);
    virtual WMError ShiftAppWindowPointerEvent(int32_t sourceWindowId, int32_t targetWindowId, int32_t fingerId);
    virtual WMError UseImplicitAnimation(int32_t hostWindowId, bool useImplicit);

    /*
     * Sub Window
     */
    virtual WMError SetParentWindow(int32_t subWindowId, int32_t newParentWindowId);

    virtual WMError GetDisplayIdByWindowId(const std::vector<uint64_t>& windowIds,
        std::unordered_map<uint64_t, DisplayId>& windowDisplayIdMap);
    virtual WMError SetGlobalDragResizeType(DragResizeType dragResizeType);
    virtual WMError GetGlobalDragResizeType(DragResizeType& dragResizeType);
    virtual WMError SetAppDragResizeType(const std::string& bundleName, DragResizeType dragResizeType);
    virtual WMError GetAppDragResizeType(const std::string& bundleName, DragResizeType& dragResizeType);
    virtual WMError SetAppKeyFramePolicy(const std::string& bundleName, const KeyFramePolicy& keyFramePolicy);
    /*
     * Window Pattern
     */
    virtual WMError SetImageForRecent(uint32_t imgResourceId, ImageFit imageFit, int32_t persistentId);
    virtual WMError SetImageForRecentPixelMap(const std::shared_ptr<Media::PixelMap>& pixelMap, ImageFit imageFit,
        int32_t persistentId);
    virtual WMError RemoveImageForRecent(int32_t persistentId);

    /*
     * Window Animation
     */
    virtual WMError AnimateTo(int32_t windowId, const WindowAnimationProperty& animationProperty,
        const WindowAnimationOption& animationOption);
    WMError NotifySupportRotationRegistered();
    
    /*
     * Window Property
     */
    virtual WMError SetWatermarkImageForApp(const std::shared_ptr<Media::PixelMap>& pixelMap);
    virtual WMError RecoverWatermarkImageForApp();
    virtual WMError NotifyScreenshotEvent(ScreenshotEventType type);
    virtual WMError CreateUIEffectController(const sptr<IUIEffectControllerClient>& controllerClient,
        sptr<IUIEffectController>& controller, int32_t& controllerId);
    virtual WMError AddSessionBlackList(
        const std::unordered_set<std::string>& bundleNames, const std::unordered_set<std::string>& privacyWindowTags);
    virtual WMError RemoveSessionBlackList(
        const std::unordered_set<std::string>& bundleNames, const std::unordered_set<std::string>& privacyWindowTags);

     /*
     * PiP Window
     */
    WMError GetPiPSettingSwitchStatus(bool& switchStatus);
    WMError GetIsPipEnabled(bool& isPipEnabled);

    /*
     * Window outline
     */
    void RegisterOutlineRecoverCallbackFunc(const OutlineRecoverCallbackFunc& callback);
    void UnregisterOutlineRecoverCallbackFunc();
    virtual WMError UpdateOutline(const sptr<IRemoteObject>& remoteObject, const OutlineParams& outlineParams);

    sptr<IWindowManager> GetWindowManagerServiceProxy() const;

private:
    friend class sptr<WindowAdapter>;
    ~WindowAdapter() override;
    WindowAdapter(const int32_t userId = INVALID_USER_ID);

    static inline SingletonDelegator<WindowAdapter> delegator;
    bool InitWMSProxy();
    bool InitSSMProxy();

    /*
     * Multi user and multi screen
     */
    void OnUserSwitch();
    int32_t userId_;
    static std::unordered_map<int32_t, sptr<WindowAdapter>> windowAdapterMap_;
    static std::mutex windowAdapterMapMutex_;

    /*
     * Window Recover
     */
    void ReregisterWindowManagerAgent();
    void ReregisterWindowManagerFaultAgent(const sptr<IWindowManager>& proxy);
    void WindowManagerAndSessionRecover();
    void RecoverSpecificZIndexSetByApp();
    WMError RecoverWindowPropertyChangeFlag();
    uint32_t observedFlags_ = 0;
    uint32_t interestedFlags_ = 0;
    std::string appWatermarkName_;
    std::unordered_map<WindowType, int32_t> specificZIndexMap_;

    mutable std::mutex mutex_;
    sptr<IWindowManager> windowManagerServiceProxy_ = nullptr;
    sptr<WMSDeathRecipient> wmsDeath_ = nullptr;
    bool isProxyValid_ = false;
    bool isRegisteredUserSwitchListener_ = false;
    bool recoverInitialized_ = false;
    std::map<int32_t, SessionRecoverCallbackFunc> sessionRecoverCallbackFuncMap_;
    // above guarded by mutex_

    std::mutex effectMutex_;
    std::map<int32_t, UIEffectRecoverCallbackFunc> uiEffectRecoverCallbackFuncMap_;

    // Note: Currently, sptr does not support unordered_map<T, unordered_set<sptr<T>>>.
    std::map<WindowManagerAgentType, std::set<sptr<IWindowManagerAgent>>> windowManagerAgentMap_;
    std::map<WindowManagerAgentType, std::set<sptr<IWindowManagerAgent>>> windowManagerAgentFaultMap_;
    std::mutex wmAgentMapMutex_;
    // Agent map both locked by wmAgentMapMutex_

    std::mutex outlineMutex_;
    OutlineRecoverCallbackFunc outlineRecoverCallbackFunc_;
};
} // namespace Rosen
} // namespace OHOS
#endif // OHOS_WINDOW_ADAPTER_H
