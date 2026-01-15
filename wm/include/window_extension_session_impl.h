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

#ifndef OHOS_ROSEN_WINDOW_EXTENSION_SESSION_IMPL_H
#define OHOS_ROSEN_WINDOW_EXTENSION_SESSION_IMPL_H

#include "window_session_impl.h"

#include <optional>

#include "accessibility_element_info.h"
#include "extension_data_handler.h"
#include "extension/extension_business_info.h"

namespace OHOS {
namespace Rosen {
struct AccessibilityChildTreeInfo {
    uint32_t windowId = 0;
    int32_t treeId = -1;
    int64_t accessibilityId = -1;
};

enum class EnvironmentType {
    NAPI = 0,
    ANI = 1
};

class WindowExtensionSessionImpl : public WindowSessionImpl {
public:
    explicit WindowExtensionSessionImpl(const sptr<WindowOption>& option);
    ~WindowExtensionSessionImpl();

    WMError Create(const std::shared_ptr<AbilityRuntime::Context>& context,
        const sptr<Rosen::ISession>& iSession, const std::string& identityToken = "",
        bool isModuleAbilityHookEnd = false) override;
    WMError MoveTo(int32_t x, int32_t y, bool isMoveToGlobal = false,
        MoveConfiguration moveConfiguration = {}) override;
    WMError Resize(uint32_t width, uint32_t height, const RectAnimationConfig& rectAnimationConfig = {}) override;
    WMError TransferAbilityResult(uint32_t resultCode, const AAFwk::Want& want) override;
    WMError TransferExtensionData(const AAFwk::WantParams& wantParams) override;
    WSError NotifyTransferComponentData(const AAFwk::WantParams& wantParams) override;
    void RegisterTransferComponentDataListener(const NotifyTransferComponentDataFunc& func) override;
    WSErrorCode NotifyTransferComponentDataSync(
        const AAFwk::WantParams& wantParams, AAFwk::WantParams& reWantParams) override;
    void RegisterTransferComponentDataForResultListener(
        const NotifyTransferComponentDataForResultFunc& func) override;
    WMError RegisterHostWindowRectChangeListener(const sptr<IWindowRectChangeListener>& listener) override;
    WMError UnregisterHostWindowRectChangeListener(const sptr<IWindowRectChangeListener>& listener) override;
    WMError RegisterRectChangeInGlobalDisplayListener(
        const sptr<IRectChangeInGlobalDisplayListener>& listener) override;
    WMError UnregisterRectChangeInGlobalDisplayListener(
        const sptr<IRectChangeInGlobalDisplayListener>& listener) override;
    WMError RegisterKeyboardDidShowListener(const sptr<IKeyboardDidShowListener>& listener) override;
    WMError UnregisterKeyboardDidShowListener(const sptr<IKeyboardDidShowListener>& listener) override;
    WMError RegisterKeyboardDidHideListener(const sptr<IKeyboardDidHideListener>& listener) override;
    WMError UnregisterKeyboardDidHideListener(const sptr<IKeyboardDidHideListener>& listener) override;
    void TriggerBindModalUIExtension() override;
    std::shared_ptr<IDataHandler> GetExtensionDataHandler() const override;
    WSError SendExtensionData(MessageParcel& data, MessageParcel& reply, MessageOption& option) override;
    WindowMode GetWindowMode() const override;
    WMError SetWindowMode(WindowMode mode) override;

    /*
     * Window Privacy
     */
    WMError SetPrivacyMode(bool isPrivacyMode) override;
    WMError HidePrivacyContentForHost(bool needHide) override;

    WMError NapiSetUIContent(const std::string& contentInfo, napi_env env, napi_value storage,
        BackupAndRestoreType type, sptr<IRemoteObject> token, AppExecFwk::Ability* ability) override;
    WMError AniSetUIContent(const std::string& contentInfo, ani_env* env, ani_object storage,
        BackupAndRestoreType type, sptr<IRemoteObject> token, AppExecFwk::Ability* ability) override;
    WMError NapiSetUIContentByName(const std::string& contentName, napi_env env, napi_value storage,
        BackupAndRestoreType type, sptr<IRemoteObject> token, AppExecFwk::Ability* ability) override;
    WMError AniSetUIContentByName(const std::string& contentName, ani_env* env, ani_object storage,
        BackupAndRestoreType type, sptr<IRemoteObject> token, AppExecFwk::Ability* ability) override;
    void SetUniqueVirtualPixelRatio(bool useUniqueDensity, float virtualPixelRatio) override {}
    WSError UpdateRect(const WSRect& rect, SizeChangeReason reason, const SceneAnimationConfig& config = {
        nullptr, ROTATE_ANIMATION_DURATION, 0, WindowAnimationCurve::LINEAR, {0.0f, 0.0f, 0.0f, 0.0f} },
        const std::map<AvoidAreaType, AvoidArea>& avoidAreas = {}) override;

    WMError GetAvoidAreaByType(AvoidAreaType type, AvoidArea& avoidArea, const Rect& rect = Rect::EMPTY_RECT,
        int32_t apiVersion = API_VERSION_INVALID) override;
    WSError NotifyAccessibilityHoverEvent(float pointX, float pointY, int32_t sourceType, int32_t eventType,
        int64_t timeMs) override;
    WSError NotifyAccessibilityChildTreeRegister(
        uint32_t windowId, int32_t treeId, int64_t accessibilityId) override;
    WSError NotifyAccessibilityChildTreeUnregister() override;
    WSError NotifyAccessibilityDumpChildInfo(
        const std::vector<std::string>& params, std::vector<std::string>& info) override;
    WMError TransferAccessibilityEvent(const Accessibility::AccessibilityEventInfo& info,
        int64_t uiExtensionIdLevel) override;
    WMError Destroy(bool needNotifyServer, bool needClearListener = true,
        uint32_t reason = static_cast<uint32_t>(WindowStateChangeReason::NORMAL)) override;

    WMError RegisterAvoidAreaChangeListener(const sptr<IAvoidAreaChangedListener>& listener) override;
    WMError UnregisterAvoidAreaChangeListener(const sptr<IAvoidAreaChangedListener>& listener) override;
    void ConsumePointerEvent(const std::shared_ptr<MMI::PointerEvent>& pointerEvent) override;
    void NotifyPointerEvent(const std::shared_ptr<MMI::PointerEvent>& pointerEvent) override;
    void ProcessPointerEventWithHostWindowDelayRaise(const std::shared_ptr<MMI::PointerEvent>& pointerEvent,
        bool isHitTargetDraggable) const;

    void NotifyFocusActiveEvent(bool isFocusActive) override;
    void NotifyFocusStateEvent(bool focusState) override;
    void NotifyBackpressedEvent(bool& isConsumed) override;
    void NotifyKeyEvent(const std::shared_ptr<MMI::KeyEvent>& keyEvent, bool& isConsumed,
        bool notifyInputMethod = true) override;
    void NotifySessionForeground(uint32_t reason, bool withAnimation) override;
    void NotifySessionBackground(uint32_t reason, bool withAnimation, bool isFromInnerkits) override;
    void NotifyOccupiedAreaChangeInfo(sptr<OccupiedAreaChangeInfo> info,
        const std::shared_ptr<RSTransaction>& rsTransaction, const Rect& callingSessionRect,
        const std::map<AvoidAreaType, AvoidArea>& avoidAreas) override;
    void NotifyKeyboardDidShow(const KeyboardPanelInfo& keyboardPanelInfo) override;
    void NotifyKeyboardDidHide(const KeyboardPanelInfo& keyboardPanelInfo) override;
    void NotifyOccupiedAreaChange(sptr<OccupiedAreaChangeInfo> info);
    WMError RegisterOccupiedAreaChangeListener(const sptr<IOccupiedAreaChangeListener>& listener) override;
    WMError UnregisterOccupiedAreaChangeListener(const sptr<IOccupiedAreaChangeListener>& listener) override;
    void UpdateConfiguration(const std::shared_ptr<AppExecFwk::Configuration>& configuration) override;
    void UpdateConfigurationForSpecified(const std::shared_ptr<AppExecFwk::Configuration>& configuration,
        const std::shared_ptr<Global::Resource::ResourceManager>& resourceManager) override;
    static void UpdateConfigurationForAll(const std::shared_ptr<AppExecFwk::Configuration>& configuration,
        const std::vector<std::shared_ptr<AbilityRuntime::Context>>& ignoreWindowContexts = {});
    WMError Show(uint32_t reason = 0, bool withAnimation = false, bool withFocus = true) override;
    WMError Show(uint32_t reason, bool withAnimation, bool withFocus, bool waitAttach) override;
    WMError Hide(uint32_t reason, bool withAnimation, bool isFromInnerkits) override;
    WMError Hide(uint32_t reason, bool withAnimation, bool isFromInnerkits, bool waitDetach) override;
    WSError NotifyDensityFollowHost(bool isFollowHost, float densityValue) override;
    WMError SetUIExtCustomDensity(const float density) override;
    float GetVirtualPixelRatio(const sptr<DisplayInfo>& displayInfo) override;
    float GetDefaultDensity(const sptr<DisplayInfo>& displayInfo);
    WMError HideNonSecureWindows(bool shouldHide) override;
    WMError SetWaterMarkFlag(bool isEnable) override;
    Rect GetHostWindowRect(int32_t hostWindowId) override;
    WMError GetGlobalScaledRect(Rect& globalScaledRect) override;
    bool IsComponentFocused() const override;

    /*
     * Gesture Back
     */
    WMError GetGestureBackEnabled(bool& enable) const override;
    WMError SetGestureBackEnabled(bool enable) override;

    /*
     * Immersive
     */
    WMError SetLayoutFullScreen(bool status) override;
    WMError SetImmersiveModeEnabledState(bool enable) override;
    bool GetImmersiveModeEnabledState() const override;
    WMError UpdateSystemBarProperties(const std::unordered_map<WindowType, SystemBarProperty>& systemBarProperties,
        const std::unordered_map<WindowType, SystemBarPropertyFlag>& systemBarPropertyFlags) override;
    WMError UpdateHostSpecificSystemBarEnabled(const std::string& name, bool enable, bool enableAnimation) override;

    /*
     * Free Multi Window
     */
    bool GetFreeMultiWindowModeEnabledState() override;

    bool PreNotifyKeyEvent(const std::shared_ptr<MMI::KeyEvent>& keyEvent) override;
    void NotifyExtensionTimeout(int32_t errorCode) override;
    int32_t GetRealParentId() const override;
    WindowType GetParentWindowType() const override;
    void NotifyModalUIExtensionMayBeCovered(bool byLoadContent) override;
    WSError UpdateSessionViewportConfig(const SessionViewportConfig& config) override;
    void NotifyExtensionEventAsync(uint32_t notifyEvent) override;
    WSError NotifyDumpInfo(const std::vector<std::string>& params, std::vector<std::string>& info) override;

    /*
     * PC Window
     */
    bool IsPcWindow() const override;
    bool IsPcOrPadFreeMultiWindowMode() const override;
    WMError UseImplicitAnimation(bool useImplicit) override;

    /*
     * Window Immersive
     */
    void UpdateDefaultStatusBarColor() override;

    /*
     * Window Property
     */
    static void UpdateConfigurationSyncForAll(const std::shared_ptr<AppExecFwk::Configuration>& configuration);
    void UpdateConfigurationSync(const std::shared_ptr<AppExecFwk::Configuration>& configuration) override;
    CrossAxisState GetCrossAxisState() override;
    WMError ExtensionSetKeepScreenOn(bool keepScreenOn) override;
    WMError ExtensionSetBrightness(float brightness) override;
    void UpdateExtensionConfig(const std::shared_ptr<AAFwk::Want>& want) override;
    WMError SendExtensionMessageToHost(uint32_t code, const AAFwk::Want& data) const;
    WMError OnExtensionMessage(uint32_t code, int32_t persistentId, const AAFwk::Want& data) override;
    WMError HandleHostWindowRaise(uint32_t code, int32_t persistentId, const AAFwk::Want& data) override;
    WMError HandleRegisterHostWindowRectChangeListener(uint32_t code, int32_t persistentId,
        const AAFwk::Want& data) override;
    WMError HandleUnregisterHostWindowRectChangeListener(uint32_t code, int32_t persistentId,
        const AAFwk::Want& data) override;
    WMError HandleRegisterHostRectChangeInGlobalDisplayListener(uint32_t code, int32_t persistentId,
        const AAFwk::Want& data) override;
    WMError HandleUnregisterHostRectChangeInGlobalDisplayListener(uint32_t code, int32_t persistentId,
        const AAFwk::Want& data) override;
    uint32_t GetHostStatusBarContentColor() const override;
    WMError GetWindowStateSnapshot(std::string& winStateSnapshotJsonStr) override;
    WMError SetStatusBarColorForExtension(uint32_t color) override;
    WMError SetStatusBarColorForExtensionInner(uint32_t color);

protected:
    NotifyTransferComponentDataFunc notifyTransferComponentDataFunc_;
    NotifyTransferComponentDataForResultFunc notifyTransferComponentDataForResultFunc_;

private:
    void InputMethodKeyEventResultCallback(const std::shared_ptr<MMI::KeyEvent>& keyEvent, bool consumed,
        std::shared_ptr<std::promise<bool>> isConsumedPromise, std::shared_ptr<bool> isTimeout);
    void AddExtensionWindowStageToSCB(bool isConstrainedModal = false);
    void RemoveExtensionWindowStageFromSCB(bool isConstrainedModal = false);
    void CheckAndAddExtWindowFlags();
    void CheckAndRemoveExtWindowFlags();
    WMError UpdateExtWindowFlags(const ExtensionWindowFlags& flags, const ExtensionWindowFlags& actions);
    void UpdateRectForRotation(const Rect& wmRect, const Rect& preRect, WindowSizeChangeReason wmReason,
        const std::shared_ptr<RSTransaction>& rsTransaction = nullptr,
        const std::map<AvoidAreaType, AvoidArea>& avoidAreas = {});
    void UpdateRectForOtherReason(const Rect& wmRect, WindowSizeChangeReason wmReason,
        const std::map<AvoidAreaType, AvoidArea>& avoidAreas = {});
    WMError GetSystemViewportConfig(SessionViewportConfig& config);
    void UpdateSystemViewportConfig();
    void UpdateExtensionDensity(SessionViewportConfig& config);
    void NotifyDisplayInfoChange(const SessionViewportConfig& config);
    WSError UpdateSessionViewportConfigInner(const SessionViewportConfig& config);
    void UpdateAccessibilityTreeInfo();
    void ArkUIFrameworkSupport();
    WMError CheckHideNonSecureWindowsPermission(bool shouldHide);
    void ReportModalUIExtensionMayBeCovered(bool byLoadContent) const;
    std::unique_ptr<Ace::UIContent> CreateUIContent(AppExecFwk::Ability* ability, void* env, EnvironmentType envType);
    WMError SetUIContentInner(const std::string& contentInfo, void* env, void* storage,
        sptr<IRemoteObject> token, AppExecFwk::Ability* ability, bool initByName = false,
        EnvironmentType envType = EnvironmentType::NAPI);
    void RegisterDataConsumer();
    void RegisterConsumer(Extension::Businesscode code,
        const std::function<WMError(AAFwk::Want&& data, std::optional<AAFwk::Want>& reply)>& func);
    WMError OnCrossAxisStateChange(AAFwk::Want&& data, std::optional<AAFwk::Want>& reply);
    WMError OnResyncExtensionConfig(AAFwk::Want&& data, std::optional<AAFwk::Want>& reply);
    WMError OnGestureBackEnabledChange(AAFwk::Want&& data, std::optional<AAFwk::Want>& reply);
    WMError OnHostWindowDelayRaiseStateChange(AAFwk::Want&& data, std::optional<AAFwk::Want>& reply);
    WMError OnHostWindowRectChange(AAFwk::Want&& data, std::optional<AAFwk::Want>& reply);
    WMError OnScreenshot(AAFwk::Want&& data, std::optional<AAFwk::Want>& reply);
    WMError OnExtensionSecureLimitChange(AAFwk::Want&& data, std::optional<AAFwk::Want>& reply);
    WMError OnKeyboardDidShow(AAFwk::Want&& data, std::optional<AAFwk::Want>& reply);
    WMError OnKeyboardDidHide(AAFwk::Want&& data, std::optional<AAFwk::Want>& reply);
    WMError OnHostStatusBarContentColorChange(AAFwk::Want&& data, std::optional<AAFwk::Want>& reply);
    WMError OnHostRectChangeInGlobalDisplay(AAFwk::Want&& data, std::optional<AAFwk::Want>& reply);

    /*
     * Compatible Mode
     */
    WMError OnHostWindowCompatInfoChange(AAFwk::Want&& data, std::optional<AAFwk::Want>& reply);
    WMError SetCompatInfo(const AAFwk::WantParams& configParam);

    std::shared_ptr<Extension::DataHandler> dataHandler_;
    std::unordered_map<uint32_t, DataConsumeCallback> dataConsumers_;  // Read only after init
    sptr<IRemoteObject> abilityToken_ { nullptr };
    std::atomic<bool> isDensityFollowHost_ { false };
    std::optional<std::atomic<float>> hostDensityValue_ = std::nullopt;
    std::optional<std::atomic<bool>> focusState_ = std::nullopt;
    std::optional<AccessibilityChildTreeInfo> accessibilityChildTreeInfo_ = std::nullopt;
    ExtensionWindowFlags extensionWindowFlags_ { 0 };
    bool modalUIExtensionMayBeCovered_ { false };
    bool modalUIExtensionSelfLoadContent_ { false };
    bool isDensityCustomized_ { false };
    float customizedDensity_ { 0.0f };
    float lastDensity_ { 0.0f };
    int32_t lastOrientation_ { 0 };
    uint64_t lastDisplayId_ { 0 };
    uint32_t lastTransform_ { 0 };
    AAFwk::WantParams extensionConfig_ {};
    bool hostGestureBackEnabled_ { true };
    bool immersiveModeEnabled_ { false };
    std::mutex hostWindowRectChangeListenerMutex_;
    std::mutex keyboardDidShowListenerMutex_;
    std::mutex keyboardDidHideListenerMutex_;
    std::mutex occupiedAreaChangeListenerMutex_;
    std::mutex hostRectChangeInGlobalDisplayListenerMutex_;
    std::vector<sptr<IWindowRectChangeListener>> hostWindowRectChangeListener_;
    std::vector<sptr<IKeyboardDidShowListener>> keyboardDidShowListenerList_;
    std::vector<sptr<IKeyboardDidHideListener>> keyboardDidHideListenerList_;
    std::vector<sptr<IOccupiedAreaChangeListener>> occupiedAreaChangeListenerList_;
    std::vector<sptr<IRectChangeInGlobalDisplayListener>> hostRectChangeInGlobalDisplayListenerList_;
    uint32_t hostStatusBarContentColor_ { 0 };
    int64_t startModalExtensionTimeStamp_ = -1;

    /*
     * PC Fold Screen
     */
    WMError OnWaterfallModeChange(AAFwk::Want&& data, std::optional<AAFwk::Want>& reply);

    /*
     * Window Rotation
     */
    void UpdateRotateDuration(WindowSizeChangeReason& reason, int32_t& duration,
        const std::shared_ptr<RSTransaction>& rsTransaction);
};
} // namespace Rosen
} // namespace OHOS

#endif // OHOS_ROSEN_WINDOW_EXTENSION_SESSION_IMPL_H
